param(
    [ValidateSet('list', 'table', 'tree', 'message', 'timeline')][string]$Family = 'list',
    [ValidateSet('style', 'regression', 'audit')][string]$Mode = 'style'
)
$ErrorActionPreference = 'Stop'
$previousPath = $env:PATH
Push-Location (Join-Path $PSScriptRoot '..\..')
try {
    $outDir = Join-Path 'build' 'style_collections'
    New-Item -ItemType Directory -Force -Path $outDir | Out-Null
    $control = @{ list = 'list_view'; table = 'table_view'; tree = 'tree_view'; message = 'message_list'; timeline = 'timeline_view' }[$Family]
    $test = "xui_style_collections_${Family}_test"
    $controlSources = @()
    $extraFlags = @()
    if ($Mode -eq 'regression') {
        $test = "xui_${control}_test"
        $controlSources = @("src/xui/controls/xui_${control}.c")
    } elseif ($Mode -eq 'audit') {
        $suffix = @{ table = 'index'; tree = 'scale'; message = 'audit' }[$Family]
        if (-not $suffix) { throw "No audit suite for $Family" }
        $test = "xui_${control}_${suffix}_test"
        if ($Family -eq 'tree') { $extraFlags = @('-Wl,--stack,262144') }
    }
    $sources = @("tests/xui/$test.c") + $controlSources + @(
        'tests/xui/xui_test_proxy.c', 'tests/xui/xui_test_xrt_impl.c',
        'modules/xlayout/xlayout.c', 'src/xui/core/xui_core.c', 'src/xui/core/xui_widget.c',
        'src/xui/accessibility/xui_accessibility.c', 'src/xui/accessibility/xui_native_accessibility.c',
        'src/xui/core/xui_layout.c', 'src/xui/core/xui_input.c',
        'src/xui/core/xui_edit.c', 'src/xui/core/xui_drag_drop.c', 'src/xui/core/xui_unicode.c',
        'src/xui/core/xui_text.c', 'src/xui/core/xui_assets.c', 'src/xui/core/xui_builtin_atlas.c',
        'src/xui/core/xui_icon.c', 'src/xui/core/xui_scroll_model.c', 'src/xui/controls/xui_scrollbar.c',
        'src/xui/controls/xui_scroll_frame.c', 'src/xui/controls/xui_scroll_view.c', 'src/xui/controls/xui_popup.c',
        'src/xui/controls/xui_menu.c'
    )
    $out = Join-Path $outDir "$test.exe"
    $engineLibraries = @()
    if ($Family -eq 'message') {
        & cmd /c tools\build\ensure_xge_dll.bat
        if ($LASTEXITCODE -ne 0) { throw 'Engine build failed' }
        # Consume the authoritative generated source manifest. The legacy batch
        # files now delegate to build.py and no longer contain expandable lists.
        $sourceOutput = & python tools/build/build.py --sources-only
        if ($LASTEXITCODE -ne 0) { throw 'Source manifest generation failed' }
        $allSources = @(($sourceOutput -join ' ') -split '\s+' | Where-Object { $_ })
        $sources = @("tests/xui/$test.c") + $controlSources + @(
            'tests/xui/xui_test_proxy.c', 'tests/xui/xui_test_xrt_impl.c'
        ) + @($allSources | Where-Object {
            $_ -notin @('src/xui/controls/xui_message_list.c','src/xui/core/xui_proxy_xge.c')
        })
        $engineLibraries = @('build/xge.lib')
        $env:PATH = (Join-Path (Get-Location) 'build') + ';' + $env:PATH
    }
    & gcc -O2 -g -Wall -Wextra -Wno-unused-parameter -Wno-unused-function -Wno-cast-function-type -DXGE_DEBUGMODE=0 @config/includes.rsp @extraFlags -o $out @sources @engineLibraries -lm -lws2_32 -liphlpapi -lgdi32 -luser32 -lshell32 -lole32 -loleaut32 -luuid -limm32 -lwinmm -lavrt
    if ($LASTEXITCODE -ne 0) { throw "Build failed: $Family" }
    & $out
    if ($LASTEXITCODE -ne 0) { throw "Test failed: $Family" }
} finally {
    $env:PATH = $previousPath
    Pop-Location
}
