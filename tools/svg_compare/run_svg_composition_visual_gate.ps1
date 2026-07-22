param(
    [string]$ReferenceManifest = "artifacts\svg_reference_thorvg_batch111_all_461\manifest_thorvg_512x512.json",
    [string]$OutputDir = "artifacts\svg_composition_visual_gate",
    [int]$Width = 512,
    [int]$Height = 512,
    [switch]$XgeClone,
    [switch]$SkipBuild,
    [switch]$WriteDiffImages
)

$ErrorActionPreference = "Stop"
$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$root = (Resolve-Path (Join-Path $scriptDir "..\..")).Path
$outputFull = [System.IO.Path]::GetFullPath((Join-Path $root $OutputDir))
. (Join-Path $scriptDir "svg_compare_cases.ps1")

$featureTags = @("clip", "mask", "blend", "filter")
$selected = @()
foreach ($case in (Get-XgeSvgCompareCases -IncludeExperimental)) {
    $tags = @(Get-XgeSvgCompareCaseTags -Case $case)
    if (@($tags | Where-Object { $_ -in $featureTags }).Count -eq 0) { continue }
    $svgPath = Join-Path $root ([string]$case.svg)
    if (-not (Test-Path -LiteralPath $svgPath)) { throw "Composition gate SVG is missing: $svgPath" }
    $svgText = Get-Content -LiteralPath $svgPath -Raw
    if ($svgText -match "(?i)<(?:text|image|feImage)\b") { continue }
    $selected += $case
}
if ($selected.Count -eq 0) { throw "Composition visual gate selected no vector-only cases." }

$selectedNames = @($selected | ForEach-Object { [string]$_.name } | Sort-Object -Unique)
if ($selectedNames.Count -ne $selected.Count) { throw "Composition visual gate selected duplicate case names." }
$featureCounts = [ordered]@{}
foreach ($tag in $featureTags) {
    $featureCounts[$tag] = @(
        $selected | Where-Object { @(Get-XgeSvgCompareCaseTags -Case $_) -contains $tag }
    ).Count
}

$gateParams = @{
    ReferenceManifest = $ReferenceManifest
    OutputDir = $OutputDir
    Width = $Width
    Height = $Height
    CaseName = $selectedNames
}
if ($XgeClone) { $gateParams["XgeClone"] = $true }
if ($SkipBuild) { $gateParams["SkipBuild"] = $true }
if ($WriteDiffImages) { $gateParams["WriteDiffImages"] = $true }

Write-Host ("SVG composition visual gate cases: {0} (clip={1}, mask={2}, blend={3}, filter={4})" -f `
    $selectedNames.Count, $featureCounts.clip, $featureCounts.mask, $featureCounts.blend, $featureCounts.filter)
& (Join-Path $scriptDir "run_svg_visual_gate.ps1") @gateParams
if ($LASTEXITCODE -ne 0) { throw "SVG composition child visual gate failed." }

$childManifestPath = Join-Path $outputFull ("manifest_{0}x{1}.json" -f $Width, $Height)
if (-not (Test-Path -LiteralPath $childManifestPath)) { throw "Composition child manifest is missing: $childManifestPath" }
$child = Get-Content -LiteralPath $childManifestPath -Raw | ConvertFrom-Json
$childNames = @($child.cases | ForEach-Object { [string]$_.name } | Sort-Object -Unique)
$failures = @()
if (-not $child.summary.passed) { $failures += "child-failed" }
if ([int]$child.summary.total_cases -ne $selectedNames.Count) { $failures += "child-count" }
if ([string]$child.xge.mode -ne $(if ($XgeClone) { "clone" } else { "normal" })) { $failures += "child-mode" }
if (@(Compare-Object -ReferenceObject $selectedNames -DifferenceObject $childNames).Count -ne 0) {
    $failures += "child-case-set"
}

$manifest = [ordered]@{
    generated_at = (Get-Date).ToString("o")
    child_manifest = $childManifestPath
    reference_manifest = $ReferenceManifest
    xge_mode = if ($XgeClone) { "clone" } else { "normal" }
    selected_case_count = $selectedNames.Count
    feature_counts = $featureCounts
    selected_cases = $selectedNames
    failures = $failures
    passed = $failures.Count -eq 0
}
$manifestPath = Join-Path $outputFull ("manifest_composition_gate_{0}x{1}.json" -f $Width, $Height)
$manifest | ConvertTo-Json -Depth 7 | Set-Content -LiteralPath $manifestPath -Encoding UTF8
Write-Host ("SVG composition visual gate: cases={0}, failures={1}, passed={2}" -f `
    $selectedNames.Count, $failures.Count, $manifest.passed)
Write-Host "SVG composition visual gate manifest: $manifestPath"
if (-not $manifest.passed) { throw "SVG composition visual gate audit failed: $($failures -join ', ')" }
