param(
	[string]$OutputDir = "artifacts\svg_batch156_visual_gate",
	[string]$RendererExe = "",
	[int]$Width = 512,
	[int]$Height = 512,
	[switch]$SkipBuild,
	[switch]$WriteDiffImages
)

$ErrorActionPreference = "Stop"
$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$root = (Resolve-Path (Join-Path $scriptDir "..\..")).Path
$outputFull = [System.IO.Path]::GetFullPath((Join-Path $root $OutputDir))
. (Join-Path $scriptDir "svg_compare_cases.ps1")

$caseNames = @(
	"thorvg_resource_test0",
	"thorvg_resource_test1",
	"thorvg_resource_test3",
	"use_symbol_forward_gradient",
	"missing_clip_mask_reference",
	"long_path_attributes",
	"symbol_intrinsic_matrix",
	"mask_forward_style_content",
	"use_invalid_stroke_width",
	"opacity_nonfinite",
	"use_paint_opacity_inherit",
	"axis_rect_half_open"
)
$selected = @(Select-XgeSvgCompareCases `
	-Cases (Get-XgeSvgCompareCases -IncludeExperimental) -CaseName $caseNames)
$selectedNames = @($selected | ForEach-Object { [string]$_.name } | Sort-Object -Unique)
if ($selectedNames.Count -ne $caseNames.Count) {
	throw "Batch156 visual gate case catalog is incomplete or contains duplicates."
}
if (@(Compare-Object -ReferenceObject ($caseNames | Sort-Object) -DifferenceObject $selectedNames).Count -ne 0) {
	throw "Batch156 visual gate case set does not match its declared cases."
}

if ($RendererExe -eq "") {
	$rendererCandidate = Join-Path $root "artifacts\thorvg_reference_build\tools\svg2png\tvg-svg2png.exe"
	if (Test-Path -LiteralPath $rendererCandidate -PathType Leaf) {
		$RendererExe = $rendererCandidate
	} else {
		$buildOutput = & (Join-Path $scriptDir "build_thorvg_svg2png.ps1") -PrintPath
		if ($LASTEXITCODE -ne 0) { throw "Failed to build the ThorVG SVG reference renderer." }
		$RendererExe = [string](@($buildOutput) | Select-Object -Last 1)
	}
}
$rendererFull = if ([System.IO.Path]::IsPathRooted($RendererExe)) {
	[System.IO.Path]::GetFullPath($RendererExe)
} else {
	[System.IO.Path]::GetFullPath((Join-Path $root $RendererExe))
}
if (-not (Test-Path -LiteralPath $rendererFull -PathType Leaf)) {
	throw "ThorVG SVG reference renderer is missing: $rendererFull"
}

New-Item -ItemType Directory -Force -Path $outputFull | Out-Null
$referenceOutput = Join-Path $outputFull "reference"
$referenceParams = @{
	RendererExe = $rendererFull
	OutputDir = $referenceOutput
	Width = $Width
	Height = $Height
	ReferenceTag = "thorvg-batch156"
	IncludeExperimental = $true
	CaseName = $selectedNames
}
& (Join-Path $scriptDir "render_svg_references.ps1") @referenceParams
if ($LASTEXITCODE -ne 0) { throw "Batch156 ThorVG reference rendering failed." }

$referenceManifest = Join-Path $referenceOutput ("manifest_thorvg-batch156_{0}x{1}.json" -f $Width, $Height)
if (-not (Test-Path -LiteralPath $referenceManifest -PathType Leaf)) {
	throw "Batch156 reference manifest is missing: $referenceManifest"
}

$modeManifests = @()
foreach ($mode in @("normal", "clone")) {
	$modeOutput = Join-Path $outputFull $mode
	$compareParams = @{
		OutputDir = $modeOutput
		ReferenceManifest = $referenceManifest
		ReferenceTag = "thorvg-batch156"
		Width = $Width
		Height = $Height
		IncludeExperimental = $true
		CaseName = $selectedNames
		FailOnReferenceMissing = $true
		XgePreserveAspectRatio = "none"
		VisualScale = 0.25
		VisualChannelThreshold = 8
	}
	if ($mode -eq "clone") { $compareParams["XgeClone"] = $true }
	if ($SkipBuild -or ($mode -eq "clone")) { $compareParams["SkipBuild"] = $true }
	if ($WriteDiffImages) { $compareParams["WriteDiffImages"] = $true }
	& (Join-Path $scriptDir "run_svg_compare.ps1") @compareParams
	if ($LASTEXITCODE -ne 0) { throw "Batch156 $mode comparison failed." }
	$modeManifests += [ordered]@{
		mode = $mode
		path = Join-Path $modeOutput ("manifest_{0}x{1}.json" -f $Width, $Height)
	}
}

$defaultLimit = [ordered]@{
	visual_rmse = 0.80
	visual_different_pixel_ratio_above_threshold = 0.0025
	visual_max_channel_diff = 50
	pixel_bounds_delta = 0
}
$caseLimits = @{
	thorvg_resource_test0 = [ordered]@{ visual_rmse = 0.30; visual_different_pixel_ratio_above_threshold = 0.0; visual_max_channel_diff = 4; pixel_bounds_delta = 0 }
	thorvg_resource_test1 = [ordered]@{ visual_rmse = 0.70; visual_different_pixel_ratio_above_threshold = 0.0; visual_max_channel_diff = 4; pixel_bounds_delta = 0 }
	thorvg_resource_test3 = [ordered]@{ visual_rmse = 2.25; visual_different_pixel_ratio_above_threshold = 0.011; visual_max_channel_diff = 70; pixel_bounds_delta = 0 }
	use_symbol_forward_gradient = [ordered]@{ visual_rmse = 0.30; visual_different_pixel_ratio_above_threshold = 0.0; visual_max_channel_diff = 4; pixel_bounds_delta = 0 }
	missing_clip_mask_reference = [ordered]@{ visual_rmse = 0.05; visual_different_pixel_ratio_above_threshold = 0.0; visual_max_channel_diff = 1; pixel_bounds_delta = 0 }
	long_path_attributes = [ordered]@{ visual_rmse = 0.30; visual_different_pixel_ratio_above_threshold = 0.0; visual_max_channel_diff = 4; pixel_bounds_delta = 0 }
	symbol_intrinsic_matrix = [ordered]@{ visual_rmse = 0.65; visual_different_pixel_ratio_above_threshold = 0.0025; visual_max_channel_diff = 50; pixel_bounds_delta = 0 }
	mask_forward_style_content = [ordered]@{ visual_rmse = 0.55; visual_different_pixel_ratio_above_threshold = 0.0; visual_max_channel_diff = 2; pixel_bounds_delta = 0 }
	use_invalid_stroke_width = [ordered]@{ visual_rmse = 0.15; visual_different_pixel_ratio_above_threshold = 0.0; visual_max_channel_diff = 2; pixel_bounds_delta = 0 }
	opacity_nonfinite = [ordered]@{ visual_rmse = 0.10; visual_different_pixel_ratio_above_threshold = 0.0; visual_max_channel_diff = 2; pixel_bounds_delta = 0 }
	use_paint_opacity_inherit = [ordered]@{ visual_rmse = 0.45; visual_different_pixel_ratio_above_threshold = 0.0; visual_max_channel_diff = 4; pixel_bounds_delta = 0 }
	axis_rect_half_open = [ordered]@{ visual_rmse = 0.30; visual_different_pixel_ratio_above_threshold = 0.0; visual_max_channel_diff = 4; pixel_bounds_delta = 0 }
}

$failures = @()
$audits = @()
foreach ($modeManifest in $modeManifests) {
	if (-not (Test-Path -LiteralPath $modeManifest.path -PathType Leaf)) {
		$failures += "missing-manifest:$($modeManifest.mode)"
		continue
	}
	$data = Get-Content -LiteralPath $modeManifest.path -Raw | ConvertFrom-Json
	$actualNames = @($data.cases | ForEach-Object { [string]$_.name } | Sort-Object -Unique)
	if (@(Compare-Object -ReferenceObject $selectedNames -DifferenceObject $actualNames).Count -ne 0) {
		$failures += "case-set:$($modeManifest.mode)"
	}
	if ([string]$data.xge.mode -cne [string]$modeManifest.mode) {
		$failures += "mode:$($modeManifest.mode)"
	}
	foreach ($entry in $data.cases) {
		$limit = if ($caseLimits.ContainsKey([string]$entry.name)) { $caseLimits[[string]$entry.name] } else { $defaultLimit }
		$caseFailures = @()
		if ([double]$entry.visual_diff.rmse_channel_diff -gt [double]$limit.visual_rmse) { $caseFailures += "visual-rmse" }
		if ([double]$entry.visual_diff.different_pixel_ratio_above_threshold -gt [double]$limit.visual_different_pixel_ratio_above_threshold) { $caseFailures += "visual-ratio" }
		if ([int]$entry.visual_diff.max_channel_diff -gt [int]$limit.visual_max_channel_diff) { $caseFailures += "visual-max" }
		if ([int]$entry.pixel_bounds_diff.max_abs_delta -gt [int]$limit.pixel_bounds_delta) { $caseFailures += "bounds" }
		foreach ($failure in $caseFailures) { $failures += "$($modeManifest.mode):$($entry.name):$failure" }
		$audits += [ordered]@{
			mode = $modeManifest.mode
			name = [string]$entry.name
			visual_rmse = [double]$entry.visual_diff.rmse_channel_diff
			visual_different_pixel_ratio_above_threshold = [double]$entry.visual_diff.different_pixel_ratio_above_threshold
			visual_max_channel_diff = [int]$entry.visual_diff.max_channel_diff
			pixel_bounds_delta = [int]$entry.pixel_bounds_diff.max_abs_delta
			limits = $limit
			failures = $caseFailures
			passed = $caseFailures.Count -eq 0
		}
	}
}

$manifest = [ordered]@{
	generated_at = (Get-Date).ToString("o")
	renderer = $rendererFull
	renderer_sha256 = (Get-FileHash -LiteralPath $rendererFull -Algorithm SHA256).Hash
	reference_manifest = $referenceManifest
	reference_manifest_sha256 = (Get-FileHash -LiteralPath $referenceManifest -Algorithm SHA256).Hash
	width = $Width
	height = $Height
	selected_cases = $selectedNames
	default_limits = $defaultLimit
	case_limits = $caseLimits
	mode_manifests = $modeManifests
	audits = $audits
	failures = $failures
	summary = [ordered]@{
		selected_case_count = $selectedNames.Count
		audit_count = $audits.Count
		passed_audit_count = @($audits | Where-Object { $_.passed }).Count
		failure_count = $failures.Count
		passed = $failures.Count -eq 0
	}
}
$manifestPath = Join-Path $outputFull ("manifest_batch156_visual_gate_{0}x{1}.json" -f $Width, $Height)
$manifest | ConvertTo-Json -Depth 10 | Set-Content -LiteralPath $manifestPath -Encoding UTF8
Write-Host ("SVG Batch156 visual gate: audits={0}/{1}, failures={2}, passed={3}" -f `
	$manifest.summary.passed_audit_count, $manifest.summary.audit_count, $failures.Count, $manifest.summary.passed)
Write-Host "SVG Batch156 visual gate manifest: $manifestPath"
if (-not $manifest.summary.passed) { throw "SVG Batch156 visual gate failed: $($failures -join '; ')" }
