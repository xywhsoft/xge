param(
    [ValidateSet('combobox', 'cascader', 'color_picker', 'date_picker', 'icon_picker')]
    [string[]]$Family = @('combobox'),
    [switch]$Legacy
)
$ErrorActionPreference = 'Stop'
Push-Location (Split-Path $PSScriptRoot -Parent)
try {
    $out = 'build/style_pickers'
    New-Item -ItemType Directory -Force $out | Out-Null
    $sources = @(
        'test_xui/xui_test_proxy.c', 'test_xui/xui_test_xrt_impl.c',
        'lib/xlayout/xlayout.c', 'src/xui_core.c', 'src/xui_widget.c',
        'src/xui_layout.c', 'src/xui_input.c', 'src/xui_edit.c',
        'src/xui_drag_drop.c', 'src/xui_unicode.c', 'src/xui_text.c',
        'src/xui_assets.c', 'src/xui_builtin_atlas.c', 'src/xui_icon.c',
        'src/xui_scroll_model.c', 'src/xui_scrollbar.c', 'src/xui_scroll_frame.c',
        'src/xui_scroll_view.c', 'src/xui_popup.c', 'src/xui_menu.c',
        'src/xui_window_frame.c', 'src/xui_window.c', 'src/xui_button.c',
        'src/xui_input_widget.c', 'src/xui_accessibility.c'
    )
    $flags = @('-O2', '-g', '-Wall', '-Wextra', '-Wno-unused-parameter',
        '-Wno-unused-function', '-Wno-cast-function-type', '-DXGE_DEBUGMODE=0',
        '-ffunction-sections', '-fdata-sections', '-I.', '-Wl,--gc-sections')
    $libs = @('-lm', '-lws2_32', '-liphlpapi', '-lgdi32', '-luser32', '-lshell32',
        '-lole32', '-loleaut32', '-luuid', '-limm32', '-lwinmm', '-lavrt')
    $objects = @()
    $headers = Get-ChildItem -Path '.', 'src', 'lib/xrt', 'lib/xlayout' -Filter '*.h' -File
    $headers += Get-Item 'test_xui/xui_test_proxy.h'
    $latestHeader = ($headers | Measure-Object -Property LastWriteTimeUtc -Maximum).Maximum
    foreach ($source in $sources) {
        $object = "$out/$([IO.Path]::GetFileNameWithoutExtension($source)).o"
        if (!(Test-Path $object) -or
            (Get-Item $source).LastWriteTimeUtc -gt (Get-Item $object).LastWriteTimeUtc -or
            $latestHeader -gt (Get-Item $object).LastWriteTimeUtc) {
            & gcc @flags -c $source -o $object
            if ($LASTEXITCODE -ne 0) { throw "Core build failed: $source" }
        }
        $objects += $object
    }
    foreach ($name in $Family) {
        $test = "test_xui/xui_style_pickers_${name}_test.c"
        $extra = @()
        $suffix = ''
        if ($Legacy) {
            $test = "test_xui/xui_${name}_test.c"
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
