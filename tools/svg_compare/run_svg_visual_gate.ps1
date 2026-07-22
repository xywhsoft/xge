param(
	[string]$ReferenceManifest = "artifacts\svg_reference_thorvg_batch111_all_461\manifest_thorvg_512x512.json",
	[string]$OutputDir = "artifacts\svg_compare_visual_stable",
	[int]$Width = 512,
	[int]$Height = 512,
	[string[]]$CaseName = @(),
	[double]$VisualScale = 0.25,
	[int]$VisualChannelThreshold = 8,
	[double]$MaxVisualDifferentPixelRatio = 0.0,
	[double]$MaxVisualRmseChannelDiff = 0.75,
	[int]$MaxVisualChannelDiff = 8,
	[int]$MaxPixelBoundsDelta = 0,
	[switch]$StableOnly,
	[switch]$XgeClone,
	[switch]$SkipBuild,
	[switch]$WriteDiffImages
)

$ErrorActionPreference = "Stop"

$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$root = (Resolve-Path (Join-Path $scriptDir "..\..")).Path
. (Join-Path $scriptDir "svg_compare_cases.ps1")

$requested = @{}
foreach ($name in $CaseName) {
	foreach ($part in ($name -split ",")) {
		$normalized = $part.Trim()
		if ($normalized -ne "") { $requested[$normalized] = $true }
	}
}

$selected = @()
foreach ($case in (Get-XgeSvgCompareCases -IncludeExperimental)) {
	$tags = @(Get-XgeSvgCompareCaseTags -Case $case)
	if ($StableOnly -and ($tags -notcontains "stable")) { continue }
	if ($tags -contains "root-aspect-batch-147") { continue }
	if ($tags -contains "svg-batch156-gate") { continue }
	if (($requested.Count -gt 0) -and (-not $requested.ContainsKey([string]$case.name))) { continue }

	$svgPath = Join-Path $root ([string]$case.svg)
	if (-not (Test-Path -LiteralPath $svgPath)) {
		throw "Visual gate SVG is missing: $svgPath"
	}
	$svgText = Get-Content -LiteralPath $svgPath -Raw
	if ($svgText -match "(?i)<(?:text|image|feImage)\b") { continue }
	$selected += $case
}

if ($selected.Count -eq 0) {
	throw "Visual gate selected no vector-only SVG cases."
}
if ($requested.Count -gt 0) {
	$selectedNames = @($selected | ForEach-Object { [string]$_.name })
	$missingNames = @($requested.Keys | Where-Object { $selectedNames -notcontains $_ })
	if ($missingNames.Count -gt 0) {
		throw "Requested cases are not eligible vector-only visual cases: $($missingNames -join ', ')"
	}
}

Write-Host "SVG visual gate cases: $($selected.Count)"
$compareParams = @{
	OutputDir = $OutputDir
	Width = $Width
	Height = $Height
	ReferenceManifest = $ReferenceManifest
	ReferenceTag = "thorvg-visual"
	FailOnReferenceMissing = $true
	IncludeExperimental = $true
	CaseName = @($selected | ForEach-Object { [string]$_.name })
	XgePreserveAspectRatio = "none"
	VisualScale = $VisualScale
	VisualChannelThreshold = $VisualChannelThreshold
	MaxVisualDifferentPixelRatio = $MaxVisualDifferentPixelRatio
	MaxVisualRmseChannelDiff = $MaxVisualRmseChannelDiff
	MaxVisualChannelDiff = $MaxVisualChannelDiff
	MaxPixelBoundsDelta = $MaxPixelBoundsDelta
}
if ($SkipBuild) { $compareParams["SkipBuild"] = $true }
if ($XgeClone) { $compareParams["XgeClone"] = $true }
if ($WriteDiffImages) { $compareParams["WriteDiffImages"] = $true }

& (Join-Path $scriptDir "run_svg_compare.ps1") @compareParams
if ($LASTEXITCODE -ne 0) {
	throw "XGE SVG visual gate failed."
}
