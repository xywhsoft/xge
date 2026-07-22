param(
    [string]$OutputPath = "artifacts\shape_ex_svg_api_coverage\manifest.json"
)

$ErrorActionPreference = "Stop"
$root = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path
$headerPath = Join-Path $root "xge.h"
$outputFull = [System.IO.Path]::GetFullPath((Join-Path $root $OutputPath))
$outputDir = Split-Path -Parent $outputFull

$header = Get-Content -LiteralPath $headerPath -Raw
$declarationPattern = [regex]'XGE_API\s+[^;\r\n]*?\b(?<name>xge(?:ShapeEx|Svg)[A-Za-z0-9_]*)\s*\('
$apiNames = @(
    $declarationPattern.Matches($header) |
        ForEach-Object { $_.Groups['name'].Value } |
        Sort-Object -Unique
)

$sourceFiles = @()
foreach ($scope in @("test", "examples", "dev\shape_ex_compare")) {
    $scopePath = Join-Path $root $scope
    if (Test-Path -LiteralPath $scopePath) {
        $sourceFiles += Get-ChildItem -LiteralPath $scopePath -Recurse -File |
            Where-Object { $_.Extension -in @(".c", ".cc", ".cpp") }
    }
}

$evidence = @{}
foreach ($name in $apiNames) { $evidence[$name] = [System.Collections.Generic.List[string]]::new() }
foreach ($file in $sourceFiles) {
    $text = Get-Content -LiteralPath $file.FullName -Raw
    foreach ($name in $apiNames) {
        if ([regex]::IsMatch($text, "\b$([regex]::Escape($name))\s*\(")) {
            $relative = $file.FullName.Substring($root.Length).TrimStart('\')
            $evidence[$name].Add($relative)
        }
    }
}

$covered = @()
$missing = @()
foreach ($name in $apiNames) {
    if ($evidence[$name].Count -gt 0) {
        $covered += [ordered]@{ name = $name; files = @($evidence[$name] | Sort-Object -Unique) }
    } else {
        $missing += $name
    }
}

$manifest = [ordered]@{
    generated_at = (Get-Date).ToString("o")
    header = $headerPath
    evidence_scopes = @("test", "examples", "dev\shape_ex_compare")
    declaration_count = $apiNames.Count
    covered_count = $covered.Count
    missing_count = $missing.Count
    covered = $covered
    missing = $missing
    passed = $missing.Count -eq 0
}
New-Item -ItemType Directory -Force -Path $outputDir | Out-Null
$manifest | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $outputFull -Encoding UTF8
Write-Host ("ShapeEx/SVG API coverage: declarations={0}, covered={1}, missing={2}, passed={3}" -f `
    $apiNames.Count, $covered.Count, $missing.Count, $manifest.passed)
Write-Host "ShapeEx/SVG API coverage manifest: $outputFull"
if (-not $manifest.passed) {
    throw "ShapeEx/SVG APIs without direct C/C++ call evidence: $($missing -join ', ')"
}
