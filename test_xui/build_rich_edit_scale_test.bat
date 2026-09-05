@echo off
setlocal
set OUT_DIR=build
set OUT=%OUT_DIR%\xui_rich_edit_scale_test.exe
set XUI_SRC=lib\xlayout\xlayout.c src\xui_core.c src\xui_widget.c src\xui_layout.c src\xui_input.c src\xui_edit.c src\xui_drag_drop.c src\xui_unicode.c src\xui_text.c src\xui_assets.c src\xui_builtin_atlas.c src\xui_icon.c src\xui_scroll_model.c src\xui_scrollbar.c src\xui_scroll_frame.c src\xui_scroll_view.c src\xui_popup.c src\xui_menu.c src\xui_rich_document.c src\xui_find.c src\xui_label.c src\xui_button.c src\xui_checkbox.c src\xui_toolbar.c src\xui_input_widget.c src\xui_window.c src\xui_window_frame.c src\xui_panel.c src\xui_text_edit.c
set SUPPORT=%OUT_DIR%\xui_rich_edit_scale_support.o
set SRC=test_xui\xui_test_proxy.c test_xui\xui_test_xrt_impl.c src\xui_image.c %XUI_SRC%
set FLAGS=-O2 -g -Wall -Wextra -Werror -Wno-unused-parameter -Wno-unused-function -Wno-cast-function-type -DXGE_DEBUGMODE=0
set LIBS=-lm -lws2_32 -liphlpapi -lgdi32 -luser32 -lshell32 -lole32 -loleaut32 -luuid -limm32 -lwinmm -lavrt
if not exist %OUT_DIR% mkdir %OUT_DIR% || exit /b 1
set NEED_BUILD=1
if exist %SUPPORT% (
    powershell -NoProfile -Command "$out=(Get-Item $env:SUPPORT).LastWriteTimeUtc; $paths=@('xui.h','xge.h','src/xui_internal.h','lib','test_xui/xui_test_proxy.h','test_xui/build_rich_edit_scale_test.bat') + ($env:SRC -split ' '); $newer=Get-ChildItem -Path $paths -Recurse -File | Where-Object { $_.LastWriteTimeUtc -gt $out } | Select-Object -First 1; if ($null -ne $newer) { exit 1 }"
    if not errorlevel 1 set NEED_BUILD=0
)
if %NEED_BUILD% equ 1 (
    gcc %FLAGS% -I. -r -o %SUPPORT% %SRC%
    if errorlevel 1 exit /b 1
)
gcc %FLAGS% -I. -o %OUT% test_xui\xui_rich_edit_scale_test.c %SUPPORT% %LIBS%
if errorlevel 1 exit /b 1
%OUT% %*
exit /b %errorlevel%
