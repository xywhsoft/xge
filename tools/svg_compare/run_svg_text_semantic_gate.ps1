param(
	[string]$RendererExe = "artifacts\thorvg_reference_build\tools\svg2png\tvg-svg2png.exe",
	[string]$OutputDir = "artifacts\svg_compare_text_semantic",
	[int]$Width = 512,
	[int]$Height = 512,
	[double]$VisualScale = 0.25,
	[int]$VisualChannelThreshold = 8,
	[double]$MaxVisualDifferentPixelRatio = 0.002,
	[double]$MaxVisualRmseChannelDiff = 0.5,
	[int]$MaxVisualChannelDiff = 30,
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

function ConvertTo-EmbeddedFontSource {
	param(
		[Parameter(Mandatory = $true)][string]$Source,
		[Parameter(Mandatory = $true)][string]$FontFace,
		[Parameter(Mandatory = $true)][string]$CaseName
	)

	$normalized = [System.Text.RegularExpressions.Regex]::Replace(
		$Source,
		'(?is)font-family\s*=\s*([''"])(.*?)\1',
		'font-family="XgeTiny"'
	)
	$normalized = [System.Text.RegularExpressions.Regex]::Replace(
		$normalized,
		'(?i)font-family\s*:\s*[^;}\r\n]+',
		'font-family:XgeTiny'
	)
	$rootMatch = [System.Text.RegularExpressions.Regex]::Match(
		$normalized,
		'(?is)<svg\b[^>]*>'
	)
	if (-not $rootMatch.Success) {
		throw "SVG text semantic gate root element is missing: $CaseName"
	}
	$rootTag = $rootMatch.Value.Substring(0, $rootMatch.Value.Length - 1) +
		' font-family="XgeTiny">'
	$replacement = $rootTag + '<style>' + $FontFace + '</style>'
	return $normalized.Substring(0, $rootMatch.Index) + $replacement +
		$normalized.Substring($rootMatch.Index + $rootMatch.Length)
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
$normalizedRoot = Join-Path $outDirFull "normalized_source"
$normalizedAssetDir = Join-Path $normalizedRoot "examples\xge_svg\assets"
$referenceDir = Join-Path $outDirFull "reference"
$normalDir = Join-Path $outDirFull "normal"
$cloneDir = Join-Path $outDirFull "clone"

New-Item -ItemType Directory -Force -Path $outDirFull | Out-Null
foreach ($path in @($normalizedRoot, $referenceDir, $normalDir, $cloneDir)) {
	Assert-ChildPath -Parent $outDirFull -Child $path
	if (Test-Path -LiteralPath $path) {
		Remove-Item -LiteralPath $path -Recurse -Force
	}
}
New-Item -ItemType Directory -Force -Path (Split-Path -Parent $normalizedAssetDir) | Out-Null
Copy-Item -LiteralPath $sourceAssetDir -Destination $normalizedAssetDir -Recurse

$caseNames = @(
	"text_style_duplicate_invalid",
	"font_size_invalid",
	"text_baseline_ignored",
	"text_xml_space_default",
	"text_xml_space_preserve",
	"text_tspan_position",
	"css_important_text_anchor_first_value",
	"mask_text_image",
	"mask_text_image_vector"
)
$catalogByName = @{}
foreach ($case in (Get-XgeSvgCompareCases -IncludeExperimental)) {
	$catalogByName[[string]$case.name] = $case
}

$fontSourcePath = Join-Path $sourceAssetDir "font_face_ttf_basic.svg"
$fontSource = Get-Content -LiteralPath $fontSourcePath -Raw
$fontFace = [System.Text.RegularExpressions.Regex]::Match(
	$fontSource,
	'(?is)@font-face\s*\{.*?\}'
).Value
if ([string]::IsNullOrEmpty($fontFace)) {
	throw "SVG text semantic gate embedded font fixture is missing."
}

$utf8NoBom = [System.Text.UTF8Encoding]::new($false)
$normalizedAssets = @()
foreach ($name in $caseNames) {
	if (-not $catalogByName.ContainsKey($name)) {
		throw "SVG text semantic gate case is missing from the catalog: $name"
	}
	$relativePath = [string]$catalogByName[$name].svg
	$sourcePath = Join-Path $root $relativePath
	$normalizedPath = Join-Path $normalizedRoot $relativePath
	if (-not (Test-Path -LiteralPath $sourcePath -PathType Leaf)) {
		throw "SVG text semantic gate asset is missing: $sourcePath"
	}
	$source = Get-Content -LiteralPath $sourcePath -Raw
	$normalized = ConvertTo-EmbeddedFontSource $source $fontFace $name
	[System.IO.File]::WriteAllText($normalizedPath, $normalized, $utf8NoBom)
	$normalizedAssets += [ordered]@{
		name = $name
		source = $sourcePath
		normalized = $normalizedPath
		normalized_sha256 = (Get-FileHash -LiteralPath $normalizedPath -Algorithm SHA256).Hash.ToLowerInvariant()
	}
}

Write-Host "SVG normalized-font text semantic cases: $($caseNames.Count)"
$referenceParams = @{
	RendererExe = $RendererExe
	OutputDir = $referenceDir
	SourceRoot = $normalizedRoot
	Width = $Width
	Height = $Height
	ReferenceTag = "thorvg-text-semantic"
	IncludeExperimental = $true
	CaseName = $caseNames
}
& (Join-Path $scriptDir "render_svg_references.ps1") @referenceParams
if ($LASTEXITCODE -ne 0) {
	throw "ThorVG normalized-font text semantic reference rendering failed."
}

$referenceManifest = Join-Path $referenceDir ("manifest_thorvg-text-semantic_{0}x{1}.json" -f $Width, $Height)
if (-not (Test-Path -LiteralPath $referenceManifest -PathType Leaf)) {
	throw "ThorVG normalized-font text semantic manifest is missing: $referenceManifest"
}

$compareParams = @{
	OutputDir = $normalDir
	SourceRoot = $normalizedRoot
	Width = $Width
	Height = $Height
	ReferenceManifest = $referenceManifest
	ReferenceTag = "thorvg-text-semantic"
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
	throw "XGE normalized-font text semantic gate failed."
}
$cloneParams = @{} + $compareParams
$cloneParams["OutputDir"] = $cloneDir
$cloneParams["SkipBuild"] = $true
$cloneParams["XgeClone"] = $true
& (Join-Path $scriptDir "run_svg_compare.ps1") @cloneParams
if ($LASTEXITCODE -ne 0) {
	throw "XGE normalized-font text semantic clone gate failed."
}

$normalManifestPath = Join-Path $normalDir ("manifest_{0}x{1}.json" -f $Width, $Height)
$cloneManifestPath = Join-Path $cloneDir ("manifest_{0}x{1}.json" -f $Width, $Height)
$normalManifest = Get-Content -LiteralPath $normalManifestPath -Raw | ConvertFrom-Json
$cloneManifest = Get-Content -LiteralPath $cloneManifestPath -Raw | ConvertFrom-Json
$failures = @()
Add-CaseSetFailures "normal" $caseNames @(Get-CaseNames $normalManifest) ([ref]$failures)
Add-CaseSetFailures "clone" $caseNames @(Get-CaseNames $cloneManifest) ([ref]$failures)
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

$combined = [ordered]@{
	generated_at = (Get-Date).ToString("o")
	source_root = $normalizedRoot
	font_fixture = $fontSourcePath
	font_fixture_sha256 = (Get-FileHash -LiteralPath $fontSourcePath -Algorithm SHA256).Hash.ToLowerInvariant()
	reference_manifest = $referenceManifest
	normal_manifest = $normalManifestPath
	clone_manifest = $cloneManifestPath
	selected_cases = $caseNames
	normalized_assets = $normalizedAssets
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
		passed = $failures.Count -eq 0
	}
}
$combinedPath = Join-Path $outDirFull ("manifest_text_semantic_gate_{0}x{1}.json" -f $Width, $Height)
$combined | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $combinedPath -Encoding UTF8
Write-Host "SVG normalized-font text semantic gate manifest saved: $combinedPath"
if ($failures.Count -gt 0) {
	throw "SVG normalized-font text semantic gate failed: $($failures -join ', ')"
}
