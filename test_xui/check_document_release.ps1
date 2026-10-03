param([string]$DllPath)

$ErrorActionPreference = 'Stop'
$root = (Resolve-Path -LiteralPath (Join-Path $PSScriptRoot '..')).Path
if (-not $DllPath) { $DllPath = Join-Path $root 'build\xge.dll' }
$DllPath = (Resolve-Path -LiteralPath $DllPath).Path

# Historical API comments and archived packets are intentionally outside this
# release gate. These are the headers, implementation, examples and tests that
# are compiled or shown to users with the current Document release.
$active = New-Object 'System.Collections.Generic.List[string]'
foreach ($name in @('xui.h', 'xui_document.h', 'xui_document_ui.h',
        'xui_sources.bat', 'xui_document_sources.bat')) {
    $active.Add((Join-Path $root $name))
}
foreach ($folder in @('src', 'test_xui', 'examples\xui_document',
        'examples\xui_richedit')) {
    Get-ChildItem -LiteralPath (Join-Path $root $folder) -Recurse -File |
        Where-Object { $_.Extension -in @('.c', '.h', '.bat', '.md') } |
        ForEach-Object { $active.Add($_.FullName) }
}
$legacy = '\b(?:xuiRich(?:Document|Edit)[A-Za-z0-9_]*|xui_rich_(?:document|node|edit)[A-Za-z0-9_]*)\b'
foreach ($path in $active) {
    if ([regex]::IsMatch([IO.File]::ReadAllText($path), $legacy)) {
        throw "Legacy RichDocument/RichEdit API remains active: $path"
    }
}
foreach ($name in @('src\xui_rich_document.c', 'src\xui_rich_edit.c')) {
    if (Test-Path -LiteralPath (Join-Path $root $name)) {
        throw "Legacy implementation remains in the release tree: $name"
    }
}

$declared = New-Object 'System.Collections.Generic.HashSet[string]'
foreach ($name in @('xui_document.h', 'xui_document_ui.h')) {
    $source = [IO.File]::ReadAllText((Join-Path $root $name))
    $source = [regex]::Replace($source, '(?s)/\*.*?\*/|//[^\r\n]*', '')
    foreach ($match in [regex]::Matches($source, '(?s)\bXUI_API\b[^;{}]*;')) {
        $function = [regex]::Match($match.Value, '\b(xui[A-Za-z0-9_]+)\s*\(')
        if (-not $function.Success) {
            throw "Cannot read a public function declaration in ${name}: $($match.Value)"
        }
        [void]$declared.Add($function.Groups[1].Value)
    }
}
if ($declared.Count -lt 200) { throw "Public Document declaration count is unexpectedly small: $($declared.Count)" }

$dump = & objdump -p $DllPath
if ($LASTEXITCODE -ne 0) { throw "objdump failed for $DllPath" }
$exported = New-Object 'System.Collections.Generic.HashSet[string]'
foreach ($line in $dump) {
    $entry = [regex]::Match($line,
        '^\s*\[\d+\]\s+\+base\[\d+\]\s+[0-9A-Fa-f]+\s+([A-Za-z_]\w*)\s*$')
    if ($entry.Success) { [void]$exported.Add($entry.Groups[1].Value) }
}
if ($exported.Count -lt $declared.Count) {
    throw "DLL export table has too few entries or could not be parsed: $($exported.Count)"
}
$missing = @($declared | Where-Object { -not $exported.Contains($_) } | Sort-Object)
if ($missing.Count) { throw "Public Document declarations missing from DLL: $($missing -join ', ')" }
$removed = @($exported | Where-Object { $_ -match $legacy } | Sort-Object)
if ($removed.Count) { throw "Legacy APIs exported by DLL: $($removed -join ', ')" }
Write-Output "Document release API audit: $($declared.Count) public functions exported; no legacy RichDocument/RichEdit API."
