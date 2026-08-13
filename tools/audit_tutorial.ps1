<#
 .SYNOPSIS
   Checks the static publication contract for the XGE + XUI tutorial.

 .DESCRIPTION
   This is intentionally a fast, GUI-free gate. It verifies that every
   published chapter has one authoritative C example, one screenshot, the
   minimum teaching sections, and no broken local page resources. Runtime and
   pixel checks remain a separate gate because they need a graphics context.
#>
[CmdletBinding()]
param(
    [string]$SiteRoot = "D:\GIT\home\host\xge\wwwroot",
    [string]$SourceRoot,
    [int]$FirstChapter = 1,
    [int]$LastChapter = 211,
    [switch]$Strict,
    [switch]$PassThru
)

$ErrorActionPreference = "Stop"

if ([string]::IsNullOrWhiteSpace($SourceRoot)) {
    $SourceRoot = Join-Path (Split-Path -Parent $PSScriptRoot) "examples\tutorial_capture"
}

function Add-Issue([System.Collections.Generic.List[object]]$Issues, [int]$Chapter, [string]$Kind, [string]$Detail) {
    $Issues.Add([pscustomobject]@{ Chapter = $Chapter; Kind = $Kind; Detail = $Detail })
}

if (-not (Test-Path -LiteralPath $SiteRoot)) { throw "Site root not found: $SiteRoot" }
if (-not (Test-Path -LiteralPath $SourceRoot)) { throw "Tutorial source root not found: $SourceRoot" }
if ($FirstChapter -lt 1 -or $LastChapter -lt $FirstChapter) { throw "Invalid chapter range" }

$issues = [System.Collections.Generic.List[object]]::new()
$summary = [System.Collections.Generic.List[object]]::new()

for ($chapter = $FirstChapter; $chapter -le $LastChapter; $chapter++) {
    $id = "{0:D2}" -f $chapter
    $page = Join-Path $SiteRoot "tutorial\ch$id.html"
    $source = Join-Path $SourceRoot "ch${id}_main1.c"
    $image = Join-Path $SiteRoot "tutorial\img\ch${id}_1.png"
    $hasPage = Test-Path -LiteralPath $page
    $hasSource = Test-Path -LiteralPath $source
    $hasImage = Test-Path -LiteralPath $image
    $codeBlocks = 0
    $hasBuild = $false
    $hasRun = $false
    $hasFullCode = $false
    $hasCaption = $false
    $commentLines = 0

    if (-not $hasPage) {
        Add-Issue $issues $chapter "page" "Missing tutorial/ch$id.html"
    } else {
        $html = Get-Content -Raw -LiteralPath $page -Encoding utf8
        $codeBlocks = [regex]::Matches($html, '<pre\b', 'IgnoreCase').Count
        $hasBuild = $html -match 'gcc\b|build[^<\s]*\.bat'
        $hasRun = $html -match 'xgeRun\b|xgeFrame\b|--frames\b|--capture\b|\.\\ch\d+\.exe'
        # Existing pages use a uniform "完整代码" heading but may carry inline
        # markup or whitespace. The text marker is deliberately tolerant.
        $hasFullCode = ($html.IndexOf([string][char]0x5B8C) -ge 0 -and $html.IndexOf([string][char]0x4EE3) -ge 0)
        $hasCaption = $html -match '<figcaption>|执行效果'
        if ($codeBlocks -eq 0) { Add-Issue $issues $chapter "code" "No code block in page" }
        if (-not $hasBuild) { Add-Issue $issues $chapter "build" "No build instruction in page" }
        if (-not $hasRun) { Add-Issue $issues $chapter "run" "No runnable entry point or finite-frame instruction in page" }
        if (-not $hasFullCode) { Add-Issue $issues $chapter "teaching" "No complete-code section" }
        if (-not $hasCaption) { Add-Issue $issues $chapter "screenshot" "No screenshot caption or execution-effect description" }

        foreach ($match in [regex]::Matches($html, '(?:href|src)\s*=\s*["'']([^"''#?]+)', 'IgnoreCase')) {
            $url = $match.Groups[1].Value.Trim()
            if ($url -match '^(https?:|//|mailto:|tel:|data:|javascript:)') { continue }
            if ($url.StartsWith('/')) { $target = Join-Path $SiteRoot $url.TrimStart('/') }
            else { $target = Join-Path (Split-Path -Parent $page) ($url -replace '/', '\\') }
            if (-not (Test-Path -LiteralPath $target)) { Add-Issue $issues $chapter "link" "Missing local resource: $url" }
        }
    }

    if (-not $hasSource) {
        Add-Issue $issues $chapter "source" "Missing authoritative example ch${id}_main1.c"
    } else {
        $lines = Get-Content -LiteralPath $source -Encoding utf8
        $commentLines = ($lines | Where-Object { $_ -match '^\s*(//|/\*|\*)' } | Measure-Object).Count
        if ($commentLines -eq 0) { Add-Issue $issues $chapter "comments" "Example has no source comments" }
    }

    if (-not $hasImage) { Add-Issue $issues $chapter "screenshot" "Missing tutorial/img/ch${id}_1.png" }
    $summary.Add([pscustomobject]@{
        Chapter = $chapter; Page = $hasPage; Source = $hasSource; Screenshot = $hasImage
        CodeBlocks = $codeBlocks; BuildInstruction = $hasBuild; RunInstruction = $hasRun
        FullCode = $hasFullCode; Caption = $hasCaption; SourceCommentLines = $commentLines
    })
}

$passCount = ($summary | Where-Object { $_.Page -and $_.Source -and $_.Screenshot -and $_.CodeBlocks -gt 0 -and $_.BuildInstruction -and $_.RunInstruction -and $_.FullCode -and $_.Caption -and $_.SourceCommentLines -gt 0 } | Measure-Object).Count
Write-Host "XGE tutorial static audit: $passCount / $($summary.Count) chapters satisfy the baseline."
if ($issues.Count -gt 0) {
    $byKind = ($issues | Group-Object Kind | Sort-Object Name | ForEach-Object { "$($_.Name)=$($_.Count)" }) -join ", "
    Write-Host "Issues: $($issues.Count) ($byKind)"
    if ($PassThru) { $issues | Sort-Object Chapter, Kind }
    if ($Strict) { exit 1 }
} else {
    Write-Host "No static publication issues found."
}

if ($PassThru) { $summary }
