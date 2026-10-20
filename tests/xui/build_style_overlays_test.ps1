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

$root = (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path
$flags = @('-O2', '-Wall', '-Wextra', '-Wno-unused-parameter', '-Wno-unused-function', '-Wno-cast-function-type', '-DXGE_DEBUGMODE=0', '@config/includes.rsp')
$libs = @('-lm', '-lws2_32', '-liphlpapi', '-lgdi32', '-luser32', '-lshell32', '-lopengl32', '-lole32', '-loleaut32', '-luuid', '-lwinmm', '-lavrt')
$sources = @(
    'tests/xui/xui_test_proxy.c', 'tests/xui/xui_test_xrt_impl.c', 'modules/xlayout/xlayout.c',
    'src/xui/core/xui_unicode.c', 'src/xui/accessibility/xui_accessibility.c', 'src/xui/accessibility/xui_native_accessibility.c',
    'src/xui/core/xui_core.c', 'src/xui/core/xui_widget.c',
    'src/xui/core/xui_layout.c', 'src/xui/core/xui_input.c', 'src/xui/core/xui_edit.c', 'src/xui/core/xui_drag_drop.c',
    'src/xui/core/xui_text.c', 'src/xui/controls/xui_label.c', 'src/xui/core/xui_assets.c', 'src/xui/core/xui_builtin_atlas.c',
    'src/xui/controls/xui_breadcrumb.c', 'src/xui/controls/xui_button.c', 'src/xui/core/xui_scroll_model.c', 'src/xui/controls/xui_scrollbar.c',
    'src/xui/controls/xui_scroll_frame.c', 'src/xui/controls/xui_scroll_view.c', 'src/xui/controls/xui_popup.c', 'src/xui/controls/xui_menu.c',
    'src/xui/controls/xui_list_view.c', 'src/xui/controls/xui_tree_view.c', 'src/xui/core/xui_input_widget.c',
    'src/xui/controls/xui_combobox.c', 'src/xui/controls/xui_window_frame.c',
    'src/xui/controls/xui_window.c', 'src/xui/controls/xui_msgtip.c', 'src/xui/controls/xui_toast.c', 'src/xui/controls/xui_msgbox.c', 'src/xui/controls/xui_file_dialog.c'
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
        Get-ChildItem -LiteralPath $root, (Join-Path $root 'src'), $PSScriptRoot, (Join-Path $root 'modules/xlayout') -Filter '*.h' -File
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
        $source = "tests/xui/xui_${test}_test.c"
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
