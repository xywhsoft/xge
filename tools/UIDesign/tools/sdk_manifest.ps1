param([switch]$Verify, [switch]$Capture)
$ErrorActionPreference = 'Stop'
$toolRoot = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..'))
$projectRoot = [IO.Path]::GetFullPath((Join-Path $toolRoot '../..'))
$manifestPath = Join-Path $toolRoot 'release/sdk-manifest.json'
$capturePath = Join-Path $toolRoot 'release/.sdk-inputs.json'
function Get-SdkHash([string]$path) {
    $stream = [IO.File]::OpenRead($path)
    $sha = [Security.Cryptography.SHA256]::Create()
    try { return ([BitConverter]::ToString($sha.ComputeHash($stream))).Replace('-', '') }
    finally { $sha.Dispose(); $stream.Dispose() }
}
$inputFiles = @('xge.h', 'xui.h', 'lib/xrt/xrt.h', 'lib/xrt/xrt_config.h', 'src/xui_xrt_port.h')
$inputHashes = [ordered]@{}
foreach ($file in $inputFiles) { $inputHashes[$file] = Get-SdkHash (Join-Path $projectRoot $file) }
if ($Capture) {
    $inputHashes | ConvertTo-Json | Set-Content -LiteralPath $capturePath -Encoding UTF8
    exit 0
}
if (-not $Verify) {
    $captured = Get-Content -LiteralPath $capturePath -Raw | ConvertFrom-Json
    foreach ($file in $inputFiles) {
        if ($captured.$file -ne $inputHashes[$file]) { throw "SDK header changed during build; rebuild required: $file" }
    }
}
$requiredExports = @(
    'xuiProxyXgePumpInput', 'xuiHotKeyRegister', 'xuiDockPanelSaveState',
    'xuiDockPanelLoadState', 'xuiTabsCreate', 'xgeRenderRequest',
    'xgeRenderRequestAfter', 'xgePlatformNativeHandle', 'xrtFileWriteAtomic'
)
$files = @('xge.h', 'xui.h', 'lib/xrt/xrt.h', 'lib/xrt/xrt_config.h',
    'src/xui_xrt_port.h', 'build/xge.lib', 'build/xge.dll',
    'tools/UIDesign/release/xge.dll', 'tools/UIDesign/release/xui_uidesign.exe')
$hashes = [ordered]@{}
foreach ($file in $files) {
    $hashes[$file] = Get-SdkHash (Join-Path $projectRoot $file)
}
if ($hashes['build/xge.dll'] -ne $hashes['tools/UIDesign/release/xge.dll']) {
    throw 'Release DLL differs from the SDK DLL used by the linker.'
}
$exports = (& objdump -p (Join-Path $toolRoot 'release/xge.dll')) -join "`n"
if ($LASTEXITCODE -ne 0) { throw 'Cannot inspect SDK exports.' }
foreach ($export in $requiredExports) {
    if ($exports -notmatch ('(?m)\b' + [regex]::Escape($export) + '\s*$')) {
        throw "Required SDK export is missing: $export"
    }
}
if ($Verify) {
    $manifest = Get-Content -LiteralPath $manifestPath -Raw | ConvertFrom-Json
    if ($manifest.format -ne 'xui.uidesign.sdk' -or $manifest.version -ne 1) { throw 'Invalid SDK manifest.' }
    foreach ($file in $files) {
        if ($manifest.sha256.$file -ne $hashes[$file]) { throw "SDK input changed since build: $file" }
    }
    Write-Output '[UIDESIGN-SDK] Header, DLL, import library and executable hashes verified.'
} else {
    # Generated build metadata, never credentials or machine-specific absolute paths.
    [ordered]@{format='xui.uidesign.sdk'; version=1; requiredExports=$requiredExports; sha256=$hashes} |
        ConvertTo-Json -Depth 5 | Set-Content -LiteralPath $manifestPath -Encoding UTF8
    Write-Output '[UIDESIGN-SDK] Matched SDK manifest generated.'
}
