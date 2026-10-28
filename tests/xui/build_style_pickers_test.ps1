param(
    [ValidateSet('combobox', 'cascader', 'color_picker', 'date_picker', 'icon_picker')]
    [string[]]$Family = @('combobox', 'cascader', 'color_picker', 'date_picker', 'icon_picker'),
    [switch]$Legacy,
    [string]$OverlayRef
)
$ErrorActionPreference = 'Stop'
Push-Location (Join-Path $PSScriptRoot '..\..')
try {
    $out = 'build/style_pickers'
    New-Item -ItemType Directory -Force $out | Out-Null
    $sources = @(
        'tests/xui/xui_test_proxy.c', 'tests/xui/xui_test_xrt_impl.c',
        'modules/xlayout/xlayout.c', 'src/xui/core/xui_core.c', 'src/xui/accessibility/xui_native_accessibility.c', 'src/xui/core/xui_widget.c',
        'src/xui/core/xui_layout.c', 'src/xui/core/xui_input.c', 'src/xui/core/xui_edit.c',
        'src/xui/core/xui_drag_drop.c', 'src/xui/core/xui_unicode.c', 'src/xui/core/xui_text.c',
        'src/xui/core/xui_assets.c', 'src/xui/core/xui_builtin_atlas.c', 'src/xui/core/xui_icon.c',
        'src/xui/core/xui_scroll_model.c', 'src/xui/controls/xui_scrollbar.c', 'src/xui/controls/xui_scroll_frame.c',
        'src/xui/controls/xui_scroll_view.c', 'src/xui/controls/xui_popup.c', 'src/xui/controls/xui_menu.c',
        'src/xui/controls/xui_window_frame.c', 'src/xui/controls/xui_window.c', 'src/xui/controls/xui_button.c',
        'src/xui/core/xui_input_widget.c', 'src/xui/accessibility/xui_accessibility.c'
    )
    $flags = @('-O2', '-g', '-Wall', '-Wextra', '-Wno-unused-parameter',
        '-Wno-unused-function', '-Wno-cast-function-type', '-DXGE_DEBUGMODE=0',
        '-ffunction-sections', '-fdata-sections', '@config/includes.rsp', '-Wl,--gc-sections')
    $libs = @('-lm', '-lws2_32', '-liphlpapi', '-lgdi32', '-luser32', '-lshell32',
        '-lole32', '-loleaut32', '-luuid', '-limm32', '-lwinmm', '-lavrt')
    $objects = @()
    $headers = Get-ChildItem -Path '.', 'src', 'extlibs/xrt', 'modules/xlayout' -Filter '*.h' -File
    $headers += Get-Item 'tests/xui/xui_test_proxy.h'
    $latestHeader = ($headers | Measure-Object -Property LastWriteTimeUtc -Maximum).Maximum
    foreach ($source in $sources) {
        $object = "$out/$([IO.Path]::GetFileNameWithoutExtension($source)).o"
        if ($OverlayRef -and $source -in @('src/xui/controls/xui_popup.c', 'src/xui/controls/xui_menu.c')) {
            $overlaySource = & git show "${OverlayRef}:$source"
            if ($LASTEXITCODE -ne 0) { throw "Cannot read ${OverlayRef}:$source" }
            $object = "$out/$([IO.Path]::GetFileNameWithoutExtension($source))_overlay.o"
            $overlaySource | & gcc @flags -Isrc -x c -c - -o $object
            if ($LASTEXITCODE -ne 0) { throw "Overlay build failed: $source" }
            $objects += $object
            continue
        }
        if (!(Test-Path $object) -or
            (Get-Item $source).LastWriteTimeUtc -gt (Get-Item $object).LastWriteTimeUtc -or
            $latestHeader -gt (Get-Item $object).LastWriteTimeUtc) {
            & gcc @flags -c $source -o $object
            if ($LASTEXITCODE -ne 0) { throw "Core build failed: $source" }
        }
        $objects += $object
    }
    foreach ($name in $Family) {
        Write-Host "[XUI] style_pickers: $name (legacy=$Legacy, overlay=$OverlayRef)"
        $test = "tests/xui/xui_style_pickers_${name}_test.c"
        $extra = @()
        $suffix = ''
        if ($Legacy) {
            $test = "tests/xui/xui_${name}_test.c"
            $extra = @("src/xui_${name}.c")
            $suffix = '_legacy'
        }
        $exe = "$out/${name}${suffix}.exe"
        & gcc @flags -o $exe $test @objects @extra @libs
        if ($LASTEXITCODE -ne 0) { throw "Build failed: $name" }
        & ".\$exe"
        if ($LASTEXITCODE -ne 0) { throw "Tests failed: $name" }
    }
} finally {
    Pop-Location
}
