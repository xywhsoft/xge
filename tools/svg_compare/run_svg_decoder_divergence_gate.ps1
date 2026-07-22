param(
	[string]$RendererExe = "artifacts\thorvg_reference_build\tools\svg2png\tvg-svg2png.exe",
	[string]$OutputDir = "artifacts\svg_decoder_divergence_gate",
	[int]$Width = 512,
	[int]$Height = 512,
	[switch]$SkipBuild
)

$ErrorActionPreference = "Stop"

function Assert-ChildPath {
	param(
		[Parameter(Mandatory = $true)][string]$Parent,
		[Parameter(Mandatory = $true)][string]$Child
	)

	$parentFull = [System.IO.Path]::GetFullPath($Parent).TrimEnd(
		[System.IO.Path]::DirectorySeparatorChar,
		[System.IO.Path]::AltDirectorySeparatorChar
	)
	$childFull = [System.IO.Path]::GetFullPath($Child)
	$prefix = $parentFull + [System.IO.Path]::DirectorySeparatorChar
	if (-not $childFull.StartsWith($prefix, [System.StringComparison]::OrdinalIgnoreCase)) {
		throw "Refusing to modify path outside output directory: $childFull"
	}
}

function Get-CaseNames {
	param([Parameter(Mandatory = $true)][object]$Manifest)

	return @($Manifest.cases | ForEach-Object { [string]$_.name })
}

function Add-CaseSetFailures {
	param(
		[Parameter(Mandatory = $true)][string]$Label,
		[Parameter(Mandatory = $true)][string[]]$Expected,
		[Parameter(Mandatory = $true)][string[]]$Actual,
		[Parameter(Mandatory = $true)][ref]$Failures
	)

	$expectedSet = @{}
	$actualSet = @{}
	foreach ($name in $Expected) {
		if ($expectedSet.ContainsKey($name)) { $Failures.Value += "duplicate:expected:$name" }
		else { $expectedSet[$name] = $true }
	}
	foreach ($name in $Actual) {
		if ($actualSet.ContainsKey($name)) { $Failures.Value += "duplicate:$Label`:$name" }
		else { $actualSet[$name] = $true }
	}
	foreach ($name in $expectedSet.Keys) {
		if (-not $actualSet.ContainsKey($name)) { $Failures.Value += "missing:$Label`:$name" }
	}
	foreach ($name in $actualSet.Keys) {
		if (-not $expectedSet.ContainsKey($name)) { $Failures.Value += "extra:$Label`:$name" }
	}
}

if (($Width -le 0) -or ($Height -le 0)) {
	throw "Width and Height must be positive."
}

$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$root = (Resolve-Path (Join-Path $scriptDir "..\..")).Path
. (Join-Path $scriptDir "svg_compare_cases.ps1")

$outDirFull = if ([System.IO.Path]::IsPathRooted($OutputDir)) {
	[System.IO.Path]::GetFullPath($OutputDir)
} else {
	[System.IO.Path]::GetFullPath((Join-Path $root $OutputDir))
}
$referenceDir = Join-Path $outDirFull "reference"
$normalDir = Join-Path $outDirFull "normal"
$cloneDir = Join-Path $outDirFull "clone"
New-Item -ItemType Directory -Force -Path $outDirFull | Out-Null
foreach ($path in @($referenceDir, $normalDir, $cloneDir)) {
	Assert-ChildPath -Parent $outDirFull -Child $path
	if (Test-Path -LiteralPath $path) { Remove-Item -LiteralPath $path -Recurse -Force }
}

$caseNames = @("mask_external_scene_batch_31", "mask_luminance_batch_32")
$catalogByName = @{}
foreach ($case in (Get-XgeSvgCompareCases -IncludeExperimental)) {
	$catalogByName[[string]$case.name] = $case
}
$failures = @()
foreach ($name in $caseNames) {
	if (-not $catalogByName.ContainsKey($name)) {
		$failures += "missing:catalog:$name"
		continue
	}
	$tags = @(Get-XgeSvgCompareCaseTags -Case $catalogByName[$name])
	if ($tags -notcontains "decode-divergence") { $failures += "missing-tag:decode-divergence:$name" }
	if ($tags -notcontains "image") { $failures += "missing-tag:image:$name" }
}
if ($failures.Count -gt 0) {
	throw "SVG decoder divergence catalog validation failed: $($failures -join ', ')"
}

$referenceParams = @{
	RendererExe = $RendererExe
	OutputDir = $referenceDir
	Width = $Width
	Height = $Height
	ReferenceTag = "thorvg-decoder-divergence"
	IncludeExperimental = $true
	CaseName = $caseNames
}
& (Join-Path $scriptDir "render_svg_references.ps1") @referenceParams
if ($LASTEXITCODE -ne 0) { throw "ThorVG decoder divergence reference rendering failed." }
$referenceManifestPath = Join-Path $referenceDir ("manifest_thorvg-decoder-divergence_{0}x{1}.json" -f $Width, $Height)

$compareParams = @{
	OutputDir = $normalDir
	Width = $Width
	Height = $Height
	ReferenceManifest = $referenceManifestPath
	ReferenceTag = "thorvg-decoder-divergence"
	FailOnReferenceMissing = $true
	IncludeExperimental = $true
	CaseName = $caseNames
	XgePreserveAspectRatio = "none"
}
if ($SkipBuild) { $compareParams["SkipBuild"] = $true }
& (Join-Path $scriptDir "run_svg_compare.ps1") @compareParams
if ($LASTEXITCODE -ne 0) { throw "XGE decoder divergence normal rendering failed." }

$cloneParams = @{} + $compareParams
$cloneParams["OutputDir"] = $cloneDir
$cloneParams["SkipBuild"] = $true
$cloneParams["XgeClone"] = $true
& (Join-Path $scriptDir "run_svg_compare.ps1") @cloneParams
if ($LASTEXITCODE -ne 0) { throw "XGE decoder divergence clone rendering failed." }

$normalManifestPath = Join-Path $normalDir ("manifest_{0}x{1}.json" -f $Width, $Height)
$cloneManifestPath = Join-Path $cloneDir ("manifest_{0}x{1}.json" -f $Width, $Height)
$referenceManifest = Get-Content -LiteralPath $referenceManifestPath -Raw | ConvertFrom-Json
$normalManifest = Get-Content -LiteralPath $normalManifestPath -Raw | ConvertFrom-Json
$cloneManifest = Get-Content -LiteralPath $cloneManifestPath -Raw | ConvertFrom-Json
$failures = @()
Add-CaseSetFailures "reference" $caseNames @(Get-CaseNames $referenceManifest) ([ref]$failures)
Add-CaseSetFailures "normal" $caseNames @(Get-CaseNames $normalManifest) ([ref]$failures)
Add-CaseSetFailures "clone" $caseNames @(Get-CaseNames $cloneManifest) ([ref]$failures)
if (($normalManifest.summary.passed -ne $true) -or ([int]$normalManifest.summary.missing_references -ne 0)) {
	$failures += "failed:normal"
}
if (($cloneManifest.summary.passed -ne $true) -or ([int]$cloneManifest.summary.missing_references -ne 0)) {
	$failures += "failed:clone"
}

$normalByName = @{}
foreach ($entry in $normalManifest.cases) { $normalByName[[string]$entry.name] = $entry }
$audits = @()
$hashMismatchCount = 0
foreach ($entry in $cloneManifest.cases) {
	$name = [string]$entry.name
	if (-not $normalByName.ContainsKey($name)) { continue }
	$normalEntry = $normalByName[$name]
	$hashEqual = [string]$normalEntry.xge_sha256 -ceq [string]$entry.xge_sha256
	if (-not $hashEqual) {
		$failures += "hash-mismatch:normal-clone:$name"
		$hashMismatchCount++
	}
	$referenceDiffPresent = (-not [bool]$normalEntry.hash_equal) -and
		([double]$normalEntry.visual_diff.different_pixel_ratio_above_threshold -gt 0.0)
	if (-not $referenceDiffPresent) {
		$failures += "decoder-divergence-resolved:$name"
	}
	if ((-not [bool]$normalEntry.pixel_diff.width_equal) -or (-not [bool]$normalEntry.pixel_diff.height_equal)) {
		$failures += "dimension-mismatch:$name"
	}
	$audits += [ordered]@{
		name = $name
		normal_clone_hash_equal = $hashEqual
		reference_divergence_present = $referenceDiffPresent
		visual_different_pixel_ratio = [double]$normalEntry.visual_diff.different_pixel_ratio_above_threshold
		visual_rmse_channel_diff = [double]$normalEntry.visual_diff.rmse_channel_diff
		visual_max_channel_diff = [int]$normalEntry.visual_diff.max_channel_diff
	}
}

$result = [ordered]@{
	generated_at = (Get-Date).ToString("o")
	policy = [ordered]@{
		classification = "allowed-image-decoder-divergence"
		reason = "The project explicitly permits image decoder differences while requiring deterministic XGE normal and clone output."
		migration = "Move a case to the regular image visual gate when its ThorVG and XGE decoder output converges."
	}
	selected_cases = $caseNames
	reference_manifest = $referenceManifestPath
	normal_manifest = $normalManifestPath
	clone_manifest = $cloneManifestPath
	audits = $audits
	failures = $failures
	summary = [ordered]@{
		total_cases = $caseNames.Count
		audited_cases = $audits.Count
		normal_clone_hash_mismatches = $hashMismatchCount
		failure_count = $failures.Count
		passed = $failures.Count -eq 0
	}
}
$manifestPath = Join-Path $outDirFull ("manifest_decoder_divergence_gate_{0}x{1}.json" -f $Width, $Height)
$result | ConvertTo-Json -Depth 10 | Set-Content -LiteralPath $manifestPath -Encoding UTF8
Write-Host "SVG decoder divergence gate manifest saved: $manifestPath"
if ($failures.Count -gt 0) {
	throw "SVG decoder divergence gate failed: $($failures -join ', ')"
}
