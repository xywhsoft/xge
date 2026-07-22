param(
	[string]$RendererExe = "artifacts\thorvg_reference_build\tools\svg2png\tvg-svg2png.exe",
	[string]$OutputDir = "artifacts\svg_compare_visual_nontext",
	[int]$Width = 512,
	[int]$Height = 512,
	[double]$VisualScale = 0.25,
	[int]$VisualChannelThreshold = 8,
	[double]$MaxVisualDifferentPixelRatio = 0.0025,
	[double]$MaxVisualRmseChannelDiff = 1.5,
	[int]$MaxVisualChannelDiff = 50,
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

	$parentFull = [System.IO.Path]::GetFullPath($Parent).TrimEnd([System.IO.Path]::DirectorySeparatorChar, [System.IO.Path]::AltDirectorySeparatorChar)
	$childFull = [System.IO.Path]::GetFullPath($Child)
	$prefix = $parentFull + [System.IO.Path]::DirectorySeparatorChar
	if (-not $childFull.StartsWith($prefix, [System.StringComparison]::OrdinalIgnoreCase)) {
		throw "Refusing to modify path outside output directory: $childFull"
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
$sourceAssetDir = Join-Path $root "examples\xge_svg\assets"
$sanitizedRoot = Join-Path $outDirFull "sanitized_source"
$sanitizedAssetDir = Join-Path $sanitizedRoot "examples\xge_svg\assets"
$referenceDir = Join-Path $outDirFull "reference"
$normalDir = Join-Path $outDirFull "normal"
$cloneDir = Join-Path $outDirFull "clone"

New-Item -ItemType Directory -Force -Path $outDirFull | Out-Null
foreach ($path in @($sanitizedRoot, $referenceDir, $normalDir, $cloneDir)) {
	Assert-ChildPath -Parent $outDirFull -Child $path
	if (Test-Path -LiteralPath $path) {
		Remove-Item -LiteralPath $path -Recurse -Force
	}
}
New-Item -ItemType Directory -Force -Path (Split-Path -Parent $sanitizedAssetDir) | Out-Null
Copy-Item -LiteralPath $sourceAssetDir -Destination $sanitizedAssetDir -Recurse

$selected = @()
$utf8NoBom = [System.Text.UTF8Encoding]::new($false)
foreach ($case in (Get-XgeSvgCompareCases -IncludeExperimental)) {
	$tags = @(Get-XgeSvgCompareCaseTags -Case $case)
	if (($tags -contains "text") -or ($tags -contains "font")) { continue }

	$sourcePath = Join-Path $root ([string]$case.svg)
	if (-not (Test-Path -LiteralPath $sourcePath -PathType Leaf)) {
		throw "SVG non-text gate asset is missing: $sourcePath"
	}
	$sourceText = Get-Content -LiteralPath $sourcePath -Raw
	if ($sourceText -notmatch "(?i)<text\b") { continue }
	if ($sourceText -notmatch "(?i)<(?:path|rect|circle|ellipse|line|polygon|polyline|use|image)\b") { continue }

	$sanitizedText = [System.Text.RegularExpressions.Regex]::Replace(
		$sourceText,
		"(?is)<text\b[^>]*>.*?</text\s*>|<text\b[^>]*/\s*>",
		""
	)
	if ($sanitizedText -match "(?i)<text\b") {
		throw "SVG non-text gate could not remove every text element: $($case.name)"
	}

	$sanitizedPath = Join-Path $sanitizedRoot ([string]$case.svg)
	[System.IO.File]::WriteAllText($sanitizedPath, $sanitizedText, $utf8NoBom)
	$selected += $case
}

if ($selected.Count -eq 0) {
	throw "SVG non-text gate selected no mixed-purpose cases."
}

$caseNames = @($selected | ForEach-Object { [string]$_.name })
Write-Host "SVG sanitized non-text gate cases: $($caseNames.Count)"

$referenceParams = @{
	RendererExe = $RendererExe
	OutputDir = $referenceDir
	SourceRoot = $sanitizedRoot
	Width = $Width
	Height = $Height
	ReferenceTag = "thorvg-nontext"
	IncludeExperimental = $true
	CaseName = $caseNames
}
& (Join-Path $scriptDir "render_svg_references.ps1") @referenceParams
if ($LASTEXITCODE -ne 0) {
	throw "ThorVG sanitized non-text reference rendering failed."
}

$referenceManifest = Join-Path $referenceDir ("manifest_thorvg-nontext_{0}x{1}.json" -f $Width, $Height)
if (-not (Test-Path -LiteralPath $referenceManifest -PathType Leaf)) {
	throw "ThorVG sanitized non-text reference manifest is missing: $referenceManifest"
}

$compareParams = @{
	OutputDir = $normalDir
	SourceRoot = $sanitizedRoot
	Width = $Width
	Height = $Height
	ReferenceManifest = $referenceManifest
	ReferenceTag = "thorvg-nontext"
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
	throw "XGE sanitized non-text visual gate failed."
}

$cloneParams = @{} + $compareParams
$cloneParams["OutputDir"] = $cloneDir
$cloneParams["SkipBuild"] = $true
$cloneParams["XgeClone"] = $true
& (Join-Path $scriptDir "run_svg_compare.ps1") @cloneParams
if ($LASTEXITCODE -ne 0) {
	throw "XGE sanitized non-text clone visual gate failed."
}

$normalManifestPath = Join-Path $normalDir ("manifest_{0}x{1}.json" -f $Width, $Height)
$cloneManifestPath = Join-Path $cloneDir ("manifest_{0}x{1}.json" -f $Width, $Height)
$normalManifest = Get-Content -LiteralPath $normalManifestPath -Raw | ConvertFrom-Json
$cloneManifest = Get-Content -LiteralPath $cloneManifestPath -Raw | ConvertFrom-Json
$passed = ([int]$normalManifest.summary.failed_compares -eq 0) -and
	([int]$cloneManifest.summary.failed_compares -eq 0)

$combined = [ordered]@{
	generated_at = (Get-Date).ToString("o")
	source_root = $sanitizedRoot
	reference_manifest = $referenceManifest
	normal_manifest = $normalManifestPath
	clone_manifest = $cloneManifestPath
	case_names = $caseNames
	thresholds = [ordered]@{
		visual_scale = $VisualScale
		visual_channel_threshold = $VisualChannelThreshold
		max_visual_different_pixel_ratio = $MaxVisualDifferentPixelRatio
		max_visual_rmse_channel_diff = $MaxVisualRmseChannelDiff
		max_visual_channel_diff = $MaxVisualChannelDiff
		max_pixel_bounds_delta = $MaxPixelBoundsDelta
	}
	summary = [ordered]@{
		total_cases = $caseNames.Count
		normal_failed_cases = [int]$normalManifest.summary.failed_compares
		clone_failed_cases = [int]$cloneManifest.summary.failed_compares
		passed = $passed
	}
}
$combinedPath = Join-Path $outDirFull ("manifest_nontext_gate_{0}x{1}.json" -f $Width, $Height)
$combined | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $combinedPath -Encoding UTF8
Write-Host "SVG sanitized non-text visual gate manifest saved: $combinedPath"

if (-not $passed) {
	throw "SVG sanitized non-text visual gate failed."
}
