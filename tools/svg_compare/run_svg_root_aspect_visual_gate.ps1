param(
    [string]$ReferenceManifest = "artifacts\svg_reference_thorvg_batch147_root_aspect_symbol\manifest_thorvg_512x512.json",
    [string]$OutputDir = "artifacts\svg_root_aspect_visual_gate",
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
$referenceFull = if ([System.IO.Path]::IsPathRooted($ReferenceManifest)) {
    [System.IO.Path]::GetFullPath($ReferenceManifest)
} else {
    [System.IO.Path]::GetFullPath((Join-Path $root $ReferenceManifest))
}
. (Join-Path $scriptDir "svg_compare_cases.ps1")

$selected = @(
    Get-XgeSvgCompareCases -IncludeExperimental |
        Where-Object { @(Get-XgeSvgCompareCaseTags -Case $_) -contains "root-aspect-batch-147" }
)
if ($selected.Count -eq 0) { throw "Root-aspect visual gate selected no cases." }

$selectedNames = @($selected | ForEach-Object { [string]$_.name } | Sort-Object -Unique)
if ($selectedNames.Count -ne $selected.Count) { throw "Root-aspect visual gate selected duplicate case names." }
foreach ($case in $selected) {
    if ((-not $case.Contains("xge_preserve_aspect_ratio")) -or
        ([string]$case.xge_preserve_aspect_ratio -cne "document")) {
        throw "Root-aspect case does not preserve its document policy: $($case.name)"
    }
    if ((-not $case.Contains("thorvg_reference_wrap_root")) -or
        (-not [bool]$case.thorvg_reference_wrap_root)) {
        throw "Root-aspect case does not request a ThorVG symbol wrapper: $($case.name)"
    }
}
if (-not (Test-Path -LiteralPath $referenceFull -PathType Leaf)) {
    throw "Root-aspect reference manifest is missing: $referenceFull"
}
$reference = Get-Content -LiteralPath $referenceFull -Raw | ConvertFrom-Json
$referenceNames = @($reference.cases | ForEach-Object { [string]$_.name } | Sort-Object -Unique)
if (@(Compare-Object -ReferenceObject $selectedNames -DifferenceObject $referenceNames).Count -ne 0) {
    throw "Root-aspect reference manifest case set does not match the catalog."
}
foreach ($entry in $reference.cases) {
    if (-not [bool]$entry.thorvg_reference_wrap_root) {
        throw "Root-aspect reference was not rendered through the ThorVG symbol wrapper: $($entry.name)"
    }
}

$gateParams = @{
    ReferenceManifest = $referenceFull
    OutputDir = $OutputDir
    Width = $Width
    Height = $Height
    IncludeExperimental = $true
    CaseName = $selectedNames
    FailOnReferenceMissing = $true
    XgePreserveAspectRatio = "none"
    VisualScale = 0.25
    VisualChannelThreshold = 8
    MaxVisualDifferentPixelRatio = 0.0
    MaxVisualRmseChannelDiff = 0.75
    MaxVisualChannelDiff = 8
    MaxPixelBoundsDelta = 0
}
if ($XgeClone) { $gateParams["XgeClone"] = $true }
if ($SkipBuild) { $gateParams["SkipBuild"] = $true }
if ($WriteDiffImages) { $gateParams["WriteDiffImages"] = $true }

Write-Host ("SVG root-aspect visual gate cases: {0}, mode={1}" -f `
    $selectedNames.Count, $(if ($XgeClone) { "clone" } else { "normal" }))
& (Join-Path $scriptDir "run_svg_compare.ps1") @gateParams
if ($LASTEXITCODE -ne 0) { throw "SVG root-aspect child visual gate failed." }

$childManifestPath = Join-Path $outputFull ("manifest_{0}x{1}.json" -f $Width, $Height)
if (-not (Test-Path -LiteralPath $childManifestPath)) {
    throw "Root-aspect child manifest is missing: $childManifestPath"
}
$child = Get-Content -LiteralPath $childManifestPath -Raw | ConvertFrom-Json
$childNames = @($child.cases | ForEach-Object { [string]$_.name } | Sort-Object -Unique)
$failures = @()
$expectedMode = if ($XgeClone) { "clone" } else { "normal" }
if (-not $child.summary.passed) { $failures += "child-failed" }
if ([int]$child.summary.total_cases -ne $selectedNames.Count) { $failures += "child-count" }
if ([string]$child.xge.mode -cne $expectedMode) { $failures += "child-mode" }
if (@(Compare-Object -ReferenceObject $selectedNames -DifferenceObject $childNames).Count -ne 0) {
    $failures += "child-case-set"
}
foreach ($entry in $child.cases) {
    if ([string]$entry.xge_preserve_aspect_ratio_source -cne "document") {
        $failures += "child-document-policy:$($entry.name)"
    }
    if ($null -ne $entry.xge_preserve_aspect_ratio) {
        $failures += "child-unexpected-override:$($entry.name)"
    }
}

$featureCounts = [ordered]@{}
foreach ($tag in @("none", "meet", "slice", "default", "grammar")) {
    $featureCounts[$tag] = @(
        $selected | Where-Object { @(Get-XgeSvgCompareCaseTags -Case $_) -contains $tag }
    ).Count
}
$manifest = [ordered]@{
    generated_at = (Get-Date).ToString("o")
    child_manifest = $childManifestPath
    reference_manifest = $referenceFull
    xge_mode = $expectedMode
    selected_case_count = $selectedNames.Count
    feature_counts = $featureCounts
    selected_cases = $selectedNames
    failures = $failures
    passed = $failures.Count -eq 0
}
$manifestPath = Join-Path $outputFull ("manifest_root_aspect_gate_{0}x{1}.json" -f $Width, $Height)
$manifest | ConvertTo-Json -Depth 7 | Set-Content -LiteralPath $manifestPath -Encoding UTF8
Write-Host ("SVG root-aspect visual gate: cases={0}, failures={1}, passed={2}" -f `
    $selectedNames.Count, $failures.Count, $manifest.passed)
Write-Host "SVG root-aspect visual gate manifest: $manifestPath"
if (-not $manifest.passed) { throw "SVG root-aspect visual gate audit failed: $($failures -join ', ')" }
