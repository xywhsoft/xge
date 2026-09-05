param([ValidateSet('list', 'table', 'tree', 'message', 'timeline')][string]$Family = 'list')
$ErrorActionPreference = 'Stop'
Push-Location (Join-Path $PSScriptRoot '..')
try {
    $outDir = Join-Path 'build' 'style_collections'
    New-Item -ItemType Directory -Force -Path $outDir | Out-Null
    $sources = @(
        "test_xui/xui_style_collections_${Family}_test.c",
        'test_xui/xui_test_proxy.c', 'test_xui/xui_test_xrt_impl.c',
        'lib/xlayout/xlayout.c', 'src/xui_core.c', 'src/xui_widget.c',
        'src/xui_accessibility.c', 'src/xui_layout.c', 'src/xui_input.c',
        'src/xui_edit.c', 'src/xui_drag_drop.c', 'src/xui_unicode.c',
        'src/xui_text.c', 'src/xui_assets.c', 'src/xui_builtin_atlas.c',
        'src/xui_icon.c', 'src/xui_scroll_model.c', 'src/xui_scrollbar.c',
        'src/xui_scroll_frame.c', 'src/xui_scroll_view.c', 'src/xui_popup.c',
        'src/xui_menu.c'
    )
    $out = Join-Path $outDir "xui_style_collections_${Family}_test.exe"
    & gcc -O2 -g -Wall -Wextra -Wno-unused-parameter -Wno-unused-function -Wno-cast-function-type -DXGE_DEBUGMODE=0 -I. -o $out @sources -lm -lws2_32 -liphlpapi -lgdi32 -luser32 -lshell32 -lole32 -loleaut32 -luuid -limm32 -lwinmm -lavrt
    if ($LASTEXITCODE -ne 0) { throw "Build failed: $Family" }
    & $out
    if ($LASTEXITCODE -ne 0) { throw "Test failed: $Family" }
} finally {
    Pop-Location
}
