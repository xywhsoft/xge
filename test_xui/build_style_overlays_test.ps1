param(
    [string[]]$Tests = @('style_overlays'),
    [switch]$Regression,
    [string]$Compiler = 'gcc'
)

$ErrorActionPreference = 'Stop'
function Get-SourceHash([string]$Path) {
    $sha = [Security.Cryptography.SHA256]::Create()
    $stream = [IO.File]::OpenRead((Resolve-Path -LiteralPath $Path).Path)
    try { return [BitConverter]::ToString($sha.ComputeHash($stream)).Replace('-', '') }
    finally { $stream.Dispose(); $sha.Dispose() }
}

$root = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path
$flags = @('-O2', '-Wall', '-Wextra', '-Wno-unused-parameter', '-Wno-unused-function', '-Wno-cast-function-type', '-DXGE_DEBUGMODE=0', '-I.')
$libs = @('-lm', '-lws2_32', '-liphlpapi', '-lgdi32', '-luser32', '-lshell32', '-lopengl32', '-lole32', '-lwinmm', '-lavrt')
$sources = @(
    'test_xui/xui_test_proxy.c', 'test_xui/xui_test_xrt_impl.c', 'lib/xlayout/xlayout.c',
    'src/xui_unicode.c', 'src/xui_accessibility.c', 'src/xui_core.c', 'src/xui_widget.c',
    'src/xui_layout.c', 'src/xui_input.c', 'src/xui_edit.c', 'src/xui_drag_drop.c',
    'src/xui_text.c', 'src/xui_label.c', 'src/xui_assets.c', 'src/xui_builtin_atlas.c',
    'src/xui_breadcrumb.c', 'src/xui_button.c', 'src/xui_scroll_model.c', 'src/xui_scrollbar.c',
    'src/xui_scroll_frame.c', 'src/xui_scroll_view.c', 'src/xui_popup.c', 'src/xui_menu.c',
    'src/xui_list_view.c', 'src/xui_input_widget.c', 'src/xui_combobox.c', 'src/xui_window_frame.c',
    'src/xui_window.c', 'src/xui_msgtip.c', 'src/xui_toast.c', 'src/xui_msgbox.c', 'src/xui_file_dialog.c'
)
if ($Regression) {
    $Tests = @('style_overlays', 'prepare_paint', 'popup', 'popup_focus', 'menu', 'msgtip', 'toast', 'msgbox', 'file_dialog')
}

Push-Location $root
try {
    $cc = (Get-Command $Compiler -ErrorAction Stop).Source
    $out = Join-Path $root 'build/style_overlays'
    $null = New-Item -ItemType Directory -Path $out -Force
    $headers = @(
        Get-ChildItem -LiteralPath $root, (Join-Path $root 'src'), $PSScriptRoot, (Join-Path $root 'lib/xlayout') -Filter '*.h' -File
    ) | Sort-Object FullName
    $headerHash = ($headers | ForEach-Object { Get-SourceHash $_.FullName }) -join ':'
    $compilerVersion = (& $cc --version | Select-Object -First 1)
    $objects = @()
    foreach ($source in $sources) {
        $object = Join-Path $out ([IO.Path]::GetFileNameWithoutExtension($source) + '.o')
        $stamp = $object + '.json'
        $inputHash = Get-SourceHash $source
        $signature = [ordered]@{ source = $inputHash; headers = $headerHash; compiler = $cc; version = $compilerVersion; flags = $flags } | ConvertTo-Json -Compress
        # Only generated build metadata is written here; every input change rebuilds the object.
        if (!(Test-Path -LiteralPath $object) -or !(Test-Path -LiteralPath $stamp) -or (Get-Content -LiteralPath $stamp -Raw).Trim() -ne $signature) {
            & $cc @flags '-c' $source '-o' $object
            if ($LASTEXITCODE -ne 0) { throw "Compile failed: $source" }
            Set-Content -LiteralPath $stamp -Value $signature -Encoding ASCII
        }
        $objects += $object
    }
    foreach ($test in $Tests) {
        if ($test -notmatch '^[a-z0-9_]+$') { throw "Invalid test name: $test" }
        $source = "test_xui/xui_${test}_test.c"
        $exe = Join-Path $out "xui_${test}_test.exe"
        if (!(Test-Path -LiteralPath $source -PathType Leaf)) { throw "Missing test: $source" }
        Write-Host "[XUI] Build and run: $test"
        & $cc @flags $source @objects @libs '-o' $exe
        if ($LASTEXITCODE -ne 0) { throw "Link failed: $test" }
        & $exe
        if ($LASTEXITCODE -ne 0) { throw "Test failed: $test (exit $LASTEXITCODE)" }
    }
    Write-Host "[XUI] PASS $($Tests.Count) overlay test programs"
} finally {
    Pop-Location
}
