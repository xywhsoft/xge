param(
	[string]$ReferenceManifest = "artifacts\svg_reference_thorvg_batch111_all_461\manifest_thorvg_512x512.json",
	[string]$OutputDir = "artifacts\svg_convenience_api_gate",
	[int]$Width = 512,
	[int]$Height = 512,
	[string[]]$CaseName = @(),
	[double]$VisualScale = 0.25,
	[int]$VisualChannelThreshold = 8,
	[double]$MaxVisualDifferentPixelRatio = 0.0,
	[double]$MaxVisualRmseChannelDiff = 0.75,
	[int]$MaxVisualChannelDiff = 8,
	[int]$MaxPixelBoundsDelta = 0,
	[switch]$SkipBuild,
	[switch]$WriteDiffImages
)

$ErrorActionPreference = "Stop"

$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$root = (Resolve-Path (Join-Path $scriptDir "..\..")).Path
. (Join-Path $scriptDir "svg_compare_cases.ps1")

$defaultCases = @(
	"compat",
	"clip_vector",
	"mask_alpha_a8_overlap",
	"gradient_spread_repeat",
	"gradient_radial_fr",
	"stroke_curve_dash_batch_84",
	"pattern_units",
	"use_forward_paint_order",
	"nested_svg_viewport_clip",
	"blend_group_mask_isolation",
	"filter_drop_shadow_primitive",
	"filter_color_graph",
	"fill_self_intersect_evenodd",
	"filter_morphology_source_erode",
	"css_url_escape",
	"aa_coverage_primitives_batch_69",
	"image_svg_data_fragment"
)
$caseNames = if ($CaseName.Count -gt 0) { @($CaseName) } else { $defaultCases }
$fileResourceCaseNames = if ($CaseName.Count -gt 0) { @() } else { @("image_source_batch_36") }
$catalog = @{}
foreach ($case in (Get-XgeSvgCompareCases -IncludeExperimental)) {
	$catalog[[string]$case.name] = $case
}
foreach ($name in $caseNames) {
	if (-not $catalog.ContainsKey($name)) {
		throw "SVG convenience API gate case is not registered: $name"
	}
	$svgPath = Join-Path $root ([string]$catalog[$name].svg)
	$svgText = Get-Content -LiteralPath $svgPath -Raw
	if (($svgText -match "(?i)<(?:text|feImage)\b") -or
	    (($svgText -match "(?i)<image\b") -and ($name -ne "image_svg_data_fragment"))) {
		throw "SVG convenience API gate requires a self-contained non-text vector case: $name"
	}
}
foreach ($name in $fileResourceCaseNames) {
	if (-not $catalog.ContainsKey($name)) {
		throw "SVG convenience API file-resource case is not registered: $name"
	}
	$svgPath = Join-Path $root ([string]$catalog[$name].svg)
	$svgText = Get-Content -LiteralPath $svgPath -Raw
	if ($svgText -match "(?i)<(?:text|feImage)\b") {
		throw "SVG convenience API file-resource case must not contain text: $name"
	}
}

$outDirFull = if ([System.IO.Path]::IsPathRooted($OutputDir)) {
	[System.IO.Path]::GetFullPath($OutputDir)
} else {
	[System.IO.Path]::GetFullPath((Join-Path $root $OutputDir))
}
New-Item -ItemType Directory -Force -Path $outDirFull | Out-Null

$modes = @(
	[ordered]@{ name = "direct"; aspect = "none"; file_resources = $true },
	[ordered]@{ name = "raster-file"; aspect = ""; file_resources = $true },
	[ordered]@{ name = "raster-memory"; aspect = ""; file_resources = $false },
	[ordered]@{ name = "texture-file"; aspect = ""; file_resources = $true },
	[ordered]@{ name = "texture-memory"; aspect = ""; file_resources = $false }
)
$children = @()
$buildPending = -not $SkipBuild

foreach ($mode in $modes) {
	$modeDir = Join-Path $outDirFull ([string]$mode.name)
	$modeCases = @($caseNames)
	if ([bool]$mode.file_resources) { $modeCases += $fileResourceCaseNames }
	Write-Host "SVG convenience API gate mode: $($mode.name), cases: $($modeCases.Count)"
	$params = @{
		OutputDir = $modeDir
		Width = $Width
		Height = $Height
		ReferenceManifest = $ReferenceManifest
		ReferenceTag = "thorvg-convenience-api"
		FailOnReferenceMissing = $true
		IncludeExperimental = $true
		CaseName = $modeCases
		XgeRenderApi = [string]$mode.name
		VisualScale = $VisualScale
		VisualChannelThreshold = $VisualChannelThreshold
		MaxVisualDifferentPixelRatio = $MaxVisualDifferentPixelRatio
		MaxVisualRmseChannelDiff = $MaxVisualRmseChannelDiff
		MaxVisualChannelDiff = $MaxVisualChannelDiff
		MaxPixelBoundsDelta = $MaxPixelBoundsDelta
	}
	if ([string]$mode.aspect -ne "") { $params["XgePreserveAspectRatio"] = [string]$mode.aspect }
	if (-not $buildPending) { $params["SkipBuild"] = $true }
	if ($WriteDiffImages) { $params["WriteDiffImages"] = $true }

	& (Join-Path $scriptDir "run_svg_compare.ps1") @params
	$buildPending = $false
	$manifestPath = Join-Path $modeDir ("manifest_{0}x{1}.json" -f $Width, $Height)
	if (-not (Test-Path -LiteralPath $manifestPath -PathType Leaf)) {
		throw "SVG convenience API child manifest is missing: $manifestPath"
	}
	$manifest = Get-Content -LiteralPath $manifestPath -Raw | ConvertFrom-Json
	$expectedMode = if ([string]$mode.name -eq "direct") { "normal" } else { [string]$mode.name }
	if (([string]$manifest.xge.mode -ne $expectedMode) -or
	    ([string]$manifest.xge.render_api -ne [string]$mode.name) -or
	    ([bool]$manifest.xge.clone)) {
		throw "SVG convenience API child manifest mode mismatch: $($mode.name)"
	}
	if (([int]$manifest.summary.total_cases -ne $modeCases.Count) -or
	    ([int]$manifest.summary.compared_cases -ne $modeCases.Count) -or
	    ([int]$manifest.summary.missing_references -ne 0) -or
	    ([int]$manifest.summary.failed_compares -ne 0) -or
	    (-not [bool]$manifest.summary.passed)) {
		throw "SVG convenience API child manifest failed: $($mode.name)"
	}
	$children += [ordered]@{
		mode = [string]$mode.name
		manifest = $manifestPath
		cases = $modeCases
		data = $manifest
	}
}

$baseline = $children | Where-Object { $_.mode -eq "direct" } | Select-Object -First 1
$hashFailures = @()
foreach ($child in ($children | Where-Object { $_.mode -ne "direct" })) {
	$hashByName = @{}
	foreach ($entry in $child.data.cases) { $hashByName[[string]$entry.name] = [string]$entry.xge_sha256 }
	foreach ($entry in $baseline.data.cases) {
		$name = [string]$entry.name
		if ($child.cases -notcontains $name) { continue }
		if ((-not $hashByName.ContainsKey($name)) -or ($hashByName[$name] -ne [string]$entry.xge_sha256)) {
			$hashFailures += [ordered]@{ mode = [string]$child.mode; case = $name }
		}
	}
}
if ($hashFailures.Count -gt 0) {
	throw "SVG convenience API output differs from direct rendering in $($hashFailures.Count) mode/case pair(s)."
}

$combined = [ordered]@{
	generated_at = (Get-Date).ToString("o")
	reference_manifest = [System.IO.Path]::GetFullPath((Join-Path $root $ReferenceManifest))
	width = $Width
	height = $Height
	self_contained_cases = $caseNames
	file_resource_cases = $fileResourceCaseNames
	modes = @($children | ForEach-Object {
		[ordered]@{ name = $_.mode; manifest = $_.manifest }
	})
	summary = [ordered]@{
		self_contained_case_count = $caseNames.Count
		file_resource_case_count = $fileResourceCaseNames.Count
		mode_count = $children.Count
		comparison_count = ($caseNames.Count * $children.Count) + ($fileResourceCaseNames.Count * 3)
		cross_mode_hash_failures = $hashFailures.Count
		passed = ($hashFailures.Count -eq 0)
	}
}
$combinedPath = Join-Path $outDirFull ("manifest_svg_convenience_api_gate_{0}x{1}.json" -f $Width, $Height)
$combined | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $combinedPath -Encoding UTF8
Write-Host "SVG convenience API gate manifest saved: $combinedPath"
