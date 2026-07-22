param(
	[int]$Repeat = 200,
	[int]$EffectsRepeat = 20,
	[int]$Runs = 5,
	[double]$MaxLinearMedianMs = 1.5,
	[double]$MaxRadialMedianMs = 2.0,
	[double]$MaxMaskMedianMs = 1.0,
	[double]$MaxBlendMedianMs = 3.0,
	[double]$MaxFilterMedianMs = 0.75,
	[double]$MaxEffectsMedianMs = 9.5,
	[string]$OutputDir = "artifacts\shape_ex_svg_performance_gate",
	[switch]$SkipBuild
)

$ErrorActionPreference = "Stop"
$root = (Resolve-Path (Join-Path $PSScriptRoot "..\..")).Path
$outputFull = if ([System.IO.Path]::IsPathRooted($OutputDir)) {
	[System.IO.Path]::GetFullPath($OutputDir)
} else {
	[System.IO.Path]::GetFullPath((Join-Path $root $OutputDir))
}

function Invoke-PerformanceCase {
	param(
		[Parameter(Mandatory = $true)][string]$Name,
		[Parameter(Mandatory = $true)][string]$Script,
		[Parameter(Mandatory = $true)][double]$Limit,
		[int]$CaseRepeat = $Repeat
	)

	$caseOutput = Join-Path $outputFull $Name
	$manifestPath = Join-Path $caseOutput "manifest.json"
	$invocationError = $null
	$params = @{
		Repeat = $CaseRepeat
		Runs = $Runs
		MaxMedianMs = $Limit
		OutputDir = $caseOutput
		SkipBuild = $true
	}
	try {
		& $Script @params
	} catch {
		$invocationError = $_.Exception.Message
	}
	if (-not (Test-Path -LiteralPath $manifestPath -PathType Leaf)) {
		return [ordered]@{
			name = $Name
			manifest = $manifestPath
			missing = $true
			limit_ms = $Limit
			invocation_error = $invocationError
			passed = $false
		}
	}
	$data = Get-Content -LiteralPath $manifestPath -Raw | ConvertFrom-Json
	return [ordered]@{
		name = $Name
		manifest = $manifestPath
		manifest_sha256 = (Get-FileHash -LiteralPath $manifestPath -Algorithm SHA256).Hash
		missing = $false
		median_ms = [double]$data.summary.median_average_ms
		limit_ms = $Limit
		invocation_error = $invocationError
		passed = ($invocationError -eq $null) -and ($data.summary.passed -eq $true)
	}
}

if (($Repeat -le 0) -or ($EffectsRepeat -le 0) -or ($Runs -le 0) -or
    ($MaxLinearMedianMs -le 0.0) -or ($MaxRadialMedianMs -le 0.0) -or
    ($MaxMaskMedianMs -le 0.0) -or ($MaxBlendMedianMs -le 0.0) -or
	($MaxFilterMedianMs -le 0.0) -or
    ($MaxEffectsMedianMs -le 0.0)) {
	throw "Repeat, Runs, and performance limits must be positive."
}
New-Item -ItemType Directory -Force -Path $outputFull | Out-Null
$buildError = $null
if (-not $SkipBuild) {
	try {
		& (Join-Path $root "build_dll.bat")
		if ($LASTEXITCODE -ne 0) { throw "Failed to build xge.dll." }
		& (Join-Path $root "examples\xge_svg\build.bat")
		if ($LASTEXITCODE -ne 0) { throw "Failed to build xge_svg.exe." }
	} catch {
		$buildError = $_.Exception.Message
	}
}

$results = @()
if ($buildError -eq $null) {
	$results = @(
		Invoke-PerformanceCase "linear" (Join-Path $PSScriptRoot "run_svg_linear_gpu_perf.ps1") $MaxLinearMedianMs
		Invoke-PerformanceCase "radial" (Join-Path $PSScriptRoot "run_svg_radial_gpu_perf.ps1") $MaxRadialMedianMs
		Invoke-PerformanceCase "mask" (Join-Path $PSScriptRoot "run_svg_mask_gpu_perf.ps1") $MaxMaskMedianMs
		Invoke-PerformanceCase "blend" (Join-Path $PSScriptRoot "run_svg_blend8_perf.ps1") $MaxBlendMedianMs
		Invoke-PerformanceCase "filter" (Join-Path $PSScriptRoot "run_svg_filter_gpu_perf.ps1") $MaxFilterMedianMs
		Invoke-PerformanceCase "effects" (Join-Path $root "dev\shape_ex_compare\run_effects_perf.ps1") $MaxEffectsMedianMs $EffectsRepeat
	)
}
$failures = @()
if ($buildError -ne $null) {
	$failures += "build:$buildError"
}
foreach ($result in $results) {
	if (-not $result.passed) {
		$failures += "performance:$($result.name)"
	}
}

$sourceFiles = @(
	"src\xge_shape_ex.c",
	"src\xge_svg.c",
	"tools\svg_compare\run_svg_linear_gpu_perf.ps1",
	"tools\svg_compare\run_svg_radial_gpu_perf.ps1",
	"tools\svg_compare\run_svg_mask_gpu_perf.ps1",
	"tools\svg_compare\run_svg_blend8_perf.ps1",
	"tools\svg_compare\run_svg_filter_gpu_perf.ps1",
	"dev\shape_ex_compare\run_effects_perf.ps1",
	"examples\xge_shape_ex_effects\main.c",
	"examples\xge_svg\assets\gradient_repeat_seam_batch_39.svg",
	"examples\xge_svg\assets\radial_gradient_gpu_batch_41.svg",
	"examples\xge_svg\assets\mask_gradient_batch_33.svg",
	"examples\xge_svg\assets\blend_fill_stroke_gradients.svg",
	"examples\xge_svg\assets\filter_gaussian_sigma_rounding.svg"
)
$sourceHashes = @()
foreach ($source in $sourceFiles) {
	$path = Join-Path $root $source
	$sourceHashes += [ordered]@{
		file = $source
		sha256 = (Get-FileHash -LiteralPath $path -Algorithm SHA256).Hash
	}
}

$manifest = [ordered]@{
	generated_at = (Get-Date).ToString("o")
	policy = [ordered]@{
		measurement = "warm repeated ShapeEx and SVG draw"
		repeat_per_run = $Repeat
		effects_repeat_per_run = $EffectsRepeat
		runs = $Runs
		max_linear_median_ms = $MaxLinearMedianMs
		max_radial_median_ms = $MaxRadialMedianMs
		max_mask_median_ms = $MaxMaskMedianMs
		max_blend_median_ms = $MaxBlendMedianMs
		max_filter_median_ms = $MaxFilterMedianMs
		max_effects_median_ms = $MaxEffectsMedianMs
	}
	build_error = $buildError
	results = $results
	source_hashes = $sourceHashes
	failures = $failures
	summary = [ordered]@{
		case_count = $results.Count
		passed_case_count = @($results | Where-Object { $_.passed }).Count
		failure_count = $failures.Count
		passed = ($buildError -eq $null) -and ($results.Count -eq 6) -and ($failures.Count -eq 0)
	}
}
$manifestPath = Join-Path $outputFull "manifest_performance.json"
$manifest | ConvertTo-Json -Depth 10 | Set-Content -LiteralPath $manifestPath -Encoding UTF8
Write-Host ("ShapeEx/SVG performance gate: cases={0}/{1}, failures={2}, passed={3}" -f $manifest.summary.passed_case_count, $manifest.summary.case_count, $failures.Count, $manifest.summary.passed)
Write-Host "ShapeEx/SVG performance gate manifest: $manifestPath"
if (-not $manifest.summary.passed) {
	throw "ShapeEx/SVG performance gate failed: $($failures -join '; ')"
}
