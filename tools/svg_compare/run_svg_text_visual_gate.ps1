param(
	[string]$RendererExe = "artifacts\thorvg_reference_build\tools\svg2png\tvg-svg2png.exe",
	[string]$OutputDir = "artifacts\svg_compare_visual_text",
	[int]$Width = 512,
	[int]$Height = 512,
	[double]$VisualScale = 0.125,
	[int]$VisualChannelThreshold = 8,
	[double]$MaxVisualDifferentPixelRatio = 0.06,
	[double]$MaxVisualRmseChannelDiff = 3.5,
	[int]$MaxVisualChannelDiff = 60,
	[int]$MaxPixelBoundsDelta = 0,
	[switch]$SkipBuild,
	[switch]$WriteDiffImages
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

function Assert-SameCaseSet {
	param(
		[Parameter(Mandatory = $true)][string]$Label,
		[Parameter(Mandatory = $true)][string[]]$Expected,
		[Parameter(Mandatory = $true)][string[]]$Actual,
		[Parameter(Mandatory = $true)][ref]$Failures
	)

	$expectedSet = @{}
	$actualSet = @{}
	foreach ($name in $Expected) {
		if ($expectedSet.ContainsKey($name)) {
			$Failures.Value += "duplicate:expected:$name"
		} else {
			$expectedSet[$name] = $true
		}
	}
	foreach ($name in $Actual) {
		if ($actualSet.ContainsKey($name)) {
			$Failures.Value += "duplicate:$Label`:$name"
		} else {
			$actualSet[$name] = $true
		}
	}
	foreach ($name in $expectedSet.Keys) {
		if (-not $actualSet.ContainsKey($name)) {
			$Failures.Value += "missing:$Label`:$name"
		}
	}
	foreach ($name in $actualSet.Keys) {
		if (-not $expectedSet.ContainsKey($name)) {
			$Failures.Value += "extra:$Label`:$name"
		}
	}
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
	if (Test-Path -LiteralPath $path) {
		Remove-Item -LiteralPath $path -Recurse -Force
	}
}

# Most cases carry their own font data. The final two exercise host-font text at a
# lower perceptual resolution so glyph edge phase does not dominate the result.
$caseNames = @(
	"font_face_ttf_basic",
	"font_face_otf_marker",
	"font_face_inherited",
	"font_face_css_class",
	"font_face_inline_override",
	"font_face_tspan_inherit",
	"font_face_tspan_override",
	"font_face_multiple",
	"font_face_malformed_src",
	"font_face_missing_family",
	"text_tspan_font_family_inherit",
	"text_font_size_style_ignored",
	"text_font_family_style_ignored",
	"text_font_size_keywords_zero",
	"text_font_size_negative_tspan_fallback",
	"text_nontext_font_size_ignored",
	"text_xml_space_style_ignored",
	"text_length_spacing_ignored",
	"text_length_glyphs_ignored",
	"text_decoration_ignored",
	"font_size_style_ignored"
)
$catalogByName = @{}
foreach ($case in (Get-XgeSvgCompareCases -IncludeExperimental)) {
	$catalogByName[[string]$case.name] = $case
}
foreach ($name in $caseNames) {
	if (-not $catalogByName.ContainsKey($name)) {
		throw "SVG text gate case is missing from the catalog: $name"
	}
	$assetPath = Join-Path $root ([string]$catalogByName[$name].svg)
	if (-not (Test-Path -LiteralPath $assetPath -PathType Leaf)) {
		throw "SVG text gate asset is missing: $assetPath"
	}
}

Write-Host "SVG text visual gate cases: $($caseNames.Count)"
$referenceParams = @{
	RendererExe = $RendererExe
	OutputDir = $referenceDir
	Width = $Width
	Height = $Height
	ReferenceTag = "thorvg-text"
	IncludeExperimental = $true
	CaseName = $caseNames
}
& (Join-Path $scriptDir "render_svg_references.ps1") @referenceParams
if ($LASTEXITCODE -ne 0) {
	throw "ThorVG text reference rendering failed."
}

$referenceManifest = Join-Path $referenceDir ("manifest_thorvg-text_{0}x{1}.json" -f $Width, $Height)
if (-not (Test-Path -LiteralPath $referenceManifest -PathType Leaf)) {
	throw "ThorVG text reference manifest is missing: $referenceManifest"
}

$compareParams = @{
	OutputDir = $normalDir
	Width = $Width
	Height = $Height
	ReferenceManifest = $referenceManifest
	ReferenceTag = "thorvg-text"
	FailOnReferenceMissing = $true
	IncludeExperimental = $true
	CaseName = $caseNames
	XgePreserveAspectRatio = "none"
	VisualScale = $VisualScale
	VisualChannelThreshold = $VisualChannelThreshold
	MaxVisualDifferentPixelRatio = $MaxVisualDifferentPixelRatio
	MaxVisualRmseChannelDiff = $MaxVisualRmseChannelDiff
	MaxVisualChannelDiff = $MaxVisualChannelDiff
	MaxPixelBoundsDelta = $MaxPixelBoundsDelta
}
if ($SkipBuild) { $compareParams["SkipBuild"] = $true }
if ($WriteDiffImages) { $compareParams["WriteDiffImages"] = $true }

& (Join-Path $scriptDir "run_svg_compare.ps1") @compareParams
if ($LASTEXITCODE -ne 0) {
	throw "XGE text visual gate failed."
}

$cloneParams = @{} + $compareParams
$cloneParams["OutputDir"] = $cloneDir
$cloneParams["SkipBuild"] = $true
$cloneParams["XgeClone"] = $true
& (Join-Path $scriptDir "run_svg_compare.ps1") @cloneParams
if ($LASTEXITCODE -ne 0) {
	throw "XGE text clone visual gate failed."
}

$normalManifestPath = Join-Path $normalDir ("manifest_{0}x{1}.json" -f $Width, $Height)
$cloneManifestPath = Join-Path $cloneDir ("manifest_{0}x{1}.json" -f $Width, $Height)
$normalManifest = Get-Content -LiteralPath $normalManifestPath -Raw | ConvertFrom-Json
$cloneManifest = Get-Content -LiteralPath $cloneManifestPath -Raw | ConvertFrom-Json
$normalNames = @(Get-CaseNames $normalManifest)
$cloneNames = @(Get-CaseNames $cloneManifest)
$failures = @()

Assert-SameCaseSet "normal" $caseNames $normalNames ([ref]$failures)
Assert-SameCaseSet "clone" $caseNames $cloneNames ([ref]$failures)
if (($normalManifest.summary.passed -ne $true) -or
	([int]$normalManifest.summary.failed_compares -ne 0) -or
	([int]$normalManifest.summary.missing_references -ne 0)) {
	$failures += "failed:normal"
}
if (($cloneManifest.summary.passed -ne $true) -or
	([int]$cloneManifest.summary.failed_compares -ne 0) -or
	([int]$cloneManifest.summary.missing_references -ne 0)) {
	$failures += "failed:clone"
}

$normalHashes = @{}
foreach ($entry in $normalManifest.cases) {
	$normalHashes[[string]$entry.name] = [string]$entry.xge_sha256
}
$hashMismatchCount = 0
foreach ($entry in $cloneManifest.cases) {
	$name = [string]$entry.name
	if ((-not $normalHashes.ContainsKey($name)) -or
		($normalHashes[$name] -cne [string]$entry.xge_sha256)) {
		$failures += "hash-mismatch:clone:$name"
		$hashMismatchCount++
	}
}

$passed = $failures.Count -eq 0
$combined = [ordered]@{
	generated_at = (Get-Date).ToString("o")
	source_root = $root
	reference_manifest = $referenceManifest
	normal_manifest = $normalManifestPath
	clone_manifest = $cloneManifestPath
	selected_cases = $caseNames
	case_names = $caseNames
	thresholds = [ordered]@{
		visual_scale = $VisualScale
		visual_channel_threshold = $VisualChannelThreshold
		max_visual_different_pixel_ratio = $MaxVisualDifferentPixelRatio
		max_visual_rmse_channel_diff = $MaxVisualRmseChannelDiff
		max_visual_channel_diff = $MaxVisualChannelDiff
		max_pixel_bounds_delta = $MaxPixelBoundsDelta
	}
	failures = $failures
	summary = [ordered]@{
		total_cases = $caseNames.Count
		normal_failed_cases = [int]$normalManifest.summary.failed_compares
		clone_failed_cases = [int]$cloneManifest.summary.failed_compares
		normal_clone_hash_mismatches = $hashMismatchCount
		failure_count = $failures.Count
		passed = $passed
	}
}
$combinedPath = Join-Path $outDirFull ("manifest_text_gate_{0}x{1}.json" -f $Width, $Height)
$combined | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $combinedPath -Encoding UTF8
Write-Host "SVG text visual gate manifest saved: $combinedPath"

if (-not $passed) {
	throw "SVG text visual gate failed: $($failures -join ', ')"
}
