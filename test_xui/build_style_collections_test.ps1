param(
    [ValidateSet('list', 'table', 'tree', 'message', 'timeline')][string]$Family = 'list',
    [ValidateSet('style', 'regression', 'audit')][string]$Mode = 'style'
)
$ErrorActionPreference = 'Stop'
$previousPath = $env:PATH
Push-Location (Join-Path $PSScriptRoot '..')
try {
    $outDir = Join-Path 'build' 'style_collections'
    New-Item -ItemType Directory -Force -Path $outDir | Out-Null
    $control = @{ list = 'list_view'; table = 'table_view'; tree = 'tree_view'; message = 'message_list'; timeline = 'timeline_view' }[$Family]
    $test = "xui_style_collections_${Family}_test"
    $controlSources = @()
    $extraFlags = @()
    if ($Mode -eq 'regression') {
        $test = "xui_${control}_test"
        $controlSources = @("src/xui_${control}.c")
    } elseif ($Mode -eq 'audit') {
        $suffix = @{ table = 'index'; tree = 'scale'; message = 'audit' }[$Family]
        if (-not $suffix) { throw "No audit suite for $Family" }
        $test = "xui_${control}_${suffix}_test"
        if ($Family -eq 'tree') { $extraFlags = @('-Wl,--stack,262144') }
    }
    $sources = @("test_xui/$test.c") + $controlSources + @(
        'test_xui/xui_test_proxy.c', 'test_xui/xui_test_xrt_impl.c',
        'lib/xlayout/xlayout.c', 'src/xui_core.c', 'src/xui_widget.c',
        'src/xui_accessibility.c', 'src/xui_layout.c', 'src/xui_input.c',
        'src/xui_edit.c', 'src/xui_drag_drop.c', 'src/xui_unicode.c',
        'src/xui_text.c', 'src/xui_assets.c', 'src/xui_builtin_atlas.c',
        'src/xui_icon.c', 'src/xui_scroll_model.c', 'src/xui_scrollbar.c',
        'src/xui_scroll_frame.c', 'src/xui_scroll_view.c', 'src/xui_popup.c',
        'src/xui_menu.c'
    )
    $out = Join-Path $outDir "$test.exe"
    $engineLibraries = @()
    if ($Family -eq 'message') {
        & cmd /c ensure_xge_dll.bat
        if ($LASTEXITCODE -ne 0) { throw 'Engine build failed' }
        # Expand the same ordered manifests as the release build. MessageList
        # now needs private Document helpers and the browser's local stubs.
        $manifest = @{}
        foreach ($file in @('xui_document_sources.bat', 'xui_sources.bat')) {
            foreach ($line in Get-Content -LiteralPath $file) {
                if ($line -match '^set (XUI_DOCUMENT_SRC|XUI_SRC)=(.*)$') {
                    $name=$Matches[1]; $value=$Matches[2]
                    foreach ($key in @($manifest.Keys)) { $value=$value.Replace("%$key%",$manifest[$key]) }
                    $manifest[$name]=$value
                }
            }
        }
        $sources = @("test_xui/$test.c") + $controlSources + @(
            'test_xui/xui_test_proxy.c', 'test_xui/xui_test_xrt_impl.c'
        ) + @($manifest['XUI_SRC'] -split '\s+' | Where-Object {
            $_ -and $_ -notin @('src\xui_message_list.c','src\xui_proxy_xge.c')
        })
        $engineLibraries = @('build/xge.lib')
        $env:PATH = (Join-Path (Get-Location) 'build') + ';' + $env:PATH
    }
    & gcc -O2 -g -Wall -Wextra -Wno-unused-parameter -Wno-unused-function -Wno-cast-function-type -DXGE_DEBUGMODE=0 -I. @extraFlags -o $out @sources @engineLibraries -lm -lws2_32 -liphlpapi -lgdi32 -luser32 -lshell32 -lole32 -loleaut32 -luuid -limm32 -lwinmm -lavrt
    if ($LASTEXITCODE -ne 0) { throw "Build failed: $Family" }
    & $out
    if ($LASTEXITCODE -ne 0) { throw "Test failed: $Family" }
} finally {
    $env:PATH = $previousPath
    Pop-Location
}
