param(
	[string]$ReferenceManifest = "artifacts\svg_reference_thorvg_batch111_all_461\manifest_thorvg_512x512.json",
	[string]$OutputDir = "artifacts\svg_compare_visual_images",
	[int]$Width = 512,
	[int]$Height = 512,
	[double]$VisualScale = 0.25,
	[int]$VisualChannelThreshold = 8,
	[double]$MaxVisualDifferentPixelRatio = 0.011,
	[double]$MaxVisualRmseChannelDiff = 1.1,
	[int]$MaxVisualChannelDiff = 12,
	[int]$MaxPixelBoundsDelta = 0,
	[switch]$XgeClone,
	[switch]$SkipBuild,
	[switch]$WriteDiffImages
)

$ErrorActionPreference = "Stop"

$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$root = (Resolve-Path (Join-Path $scriptDir "..\..")).Path
. (Join-Path $scriptDir "svg_compare_cases.ps1")

# The ThorVG reference build intentionally has no PNG/JPEG loader. Keep these
# cases as render/reference reports without treating decoder output as a failure.
$decoderSensitiveNames = @(
	"filter_gaussian_raster",
	"raster_image",
	"image_jpg_data_uri"
)
$structuralCases = @()
$decoderCases = @()

foreach ($case in (Get-XgeSvgCompareCases -IncludeExperimental)) {
	$tags = @(Get-XgeSvgCompareCaseTags -Case $case)
	if ($tags -contains "svg-batch156-gate") { continue }
	$svgPath = Join-Path $root ([string]$case.svg)
	if (-not (Test-Path -LiteralPath $svgPath)) {
		throw "SVG image gate asset is missing: $svgPath"
	}
	$svgText = Get-Content -LiteralPath $svgPath -Raw
	if (($svgText -notmatch "(?i)<(?:image|feImage)\b") -or
	    ($svgText -match "(?i)<text\b")) {
		continue
	}
	if ($decoderSensitiveNames -contains [string]$case.name) {
		$decoderCases += $case
	} else {
		$structuralCases += $case
	}
}

if ($structuralCases.Count -eq 0) {
	throw "SVG image gate selected no structural image cases."
}
if ($decoderCases.Count -ne $decoderSensitiveNames.Count) {
	throw "SVG image gate decoder-sensitive case list is incomplete."
}

$outDirFull = if ([System.IO.Path]::IsPathRooted($OutputDir)) {
	[System.IO.Path]::GetFullPath($OutputDir)
} else {
	[System.IO.Path]::GetFullPath((Join-Path $root $OutputDir))
}
$structuralDir = Join-Path $outDirFull "structural"
$decoderDir = Join-Path $outDirFull "decoder_report"
New-Item -ItemType Directory -Force -Path $outDirFull | Out-Null

Write-Host "SVG structural image gate cases: $($structuralCases.Count)"
$structuralParams = @{
	OutputDir = $structuralDir
	Width = $Width
	Height = $Height
	ReferenceManifest = $ReferenceManifest
	ReferenceTag = "thorvg-image"
	FailOnReferenceMissing = $true
	IncludeExperimental = $true
	CaseName = @($structuralCases | ForEach-Object { [string]$_.name })
	XgePreserveAspectRatio = "none"
	VisualScale = $VisualScale
	VisualChannelThreshold = $VisualChannelThreshold
	MaxVisualDifferentPixelRatio = $MaxVisualDifferentPixelRatio
	MaxVisualRmseChannelDiff = $MaxVisualRmseChannelDiff
	MaxVisualChannelDiff = $MaxVisualChannelDiff
	MaxPixelBoundsDelta = $MaxPixelBoundsDelta
}
if ($SkipBuild) { $structuralParams["SkipBuild"] = $true }
if ($XgeClone) { $structuralParams["XgeClone"] = $true }
if ($WriteDiffImages) { $structuralParams["WriteDiffImages"] = $true }

& (Join-Path $scriptDir "run_svg_compare.ps1") @structuralParams
if ($LASTEXITCODE -ne 0) {
	throw "XGE SVG structural image gate failed."
}

Write-Host "SVG decoder-sensitive report cases: $($decoderCases.Count)"
$decoderParams = @{
	OutputDir = $decoderDir
	Width = $Width
	Height = $Height
	ReferenceManifest = $ReferenceManifest
	ReferenceTag = "thorvg-decoder"
	FailOnReferenceMissing = $true
	IncludeExperimental = $true
	CaseName = @($decoderCases | ForEach-Object { [string]$_.name })
	XgePreserveAspectRatio = "none"
	VisualScale = $VisualScale
	VisualChannelThreshold = $VisualChannelThreshold
	SkipBuild = $true
}
if ($XgeClone) { $decoderParams["XgeClone"] = $true }
if ($WriteDiffImages) { $decoderParams["WriteDiffImages"] = $true }

& (Join-Path $scriptDir "run_svg_compare.ps1") @decoderParams
if ($LASTEXITCODE -ne 0) {
	throw "XGE SVG decoder-sensitive report failed."
}

$structuralManifestPath = Join-Path $structuralDir ("manifest_{0}x{1}.json" -f $Width, $Height)
$decoderManifestPath = Join-Path $decoderDir ("manifest_{0}x{1}.json" -f $Width, $Height)
$structuralManifest = Get-Content -LiteralPath $structuralManifestPath -Raw | ConvertFrom-Json
$decoderManifest = Get-Content -LiteralPath $decoderManifestPath -Raw | ConvertFrom-Json
$decoderDimensionFailures = @(
	$decoderManifest.cases | Where-Object {
		($_.pixel_diff.width_equal -ne $true) -or
		($_.pixel_diff.height_equal -ne $true)
	}
)
if ($decoderDimensionFailures.Count -gt 0) {
	throw "XGE SVG decoder-sensitive report contains dimension mismatches."
}

$combined = [ordered]@{
	generated_at = (Get-Date).ToString("o")
	mode = if ($XgeClone) { "clone" } else { "normal" }
	reference_manifest = [System.IO.Path]::GetFullPath((Join-Path $root $ReferenceManifest))
	structural_manifest = $structuralManifestPath
	decoder_report_manifest = $decoderManifestPath
	thresholds = [ordered]@{
		visual_scale = $VisualScale
		visual_channel_threshold = $VisualChannelThreshold
		max_visual_different_pixel_ratio = $MaxVisualDifferentPixelRatio
		max_visual_rmse_channel_diff = $MaxVisualRmseChannelDiff
		max_visual_channel_diff = $MaxVisualChannelDiff
		max_pixel_bounds_delta = $MaxPixelBoundsDelta
	}
	summary = [ordered]@{
		total_cases = $structuralCases.Count + $decoderCases.Count
		visual_gate_cases = $structuralCases.Count
		decoder_report_cases = $decoderCases.Count
		failed_visual_cases = [int]$structuralManifest.summary.failed_compares
		decoder_dimension_failures = $decoderDimensionFailures.Count
		passed = ([int]$structuralManifest.summary.failed_compares -eq 0) -and
			($decoderDimensionFailures.Count -eq 0)
	}
}
$combinedPath = Join-Path $outDirFull ("manifest_image_gate_{0}x{1}.json" -f $Width, $Height)
$combined | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $combinedPath -Encoding UTF8
Write-Host "SVG image visual gate manifest saved: $combinedPath"
