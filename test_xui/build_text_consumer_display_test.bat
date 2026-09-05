@echo off
setlocal
pushd "%~dp0.." || exit /b 1
set OUT=build\xui_text_consumer_display_test.exe
if not exist build (mkdir build || (popd & exit /b 1))
rem The five consumers are included in the test to count only their allocations.
set SRC=test_xui\xui_text_consumer_display_test.c test_xui\xui_test_proxy.c test_xui\xui_test_xrt_impl.c lib\xlayout\xlayout.c src\xui_core.c src\xui_widget.c src\xui_layout.c src\xui_input.c src\xui_edit.c src\xui_drag_drop.c src\xui_unicode.c src\xui_text.c src\xui_assets.c src\xui_builtin_atlas.c src\xui_icon.c src\xui_scroll_model.c src\xui_scrollbar.c src\xui_scroll_frame.c src\xui_scroll_view.c src\xui_popup.c src\xui_menu.c src\xui_window_frame.c src\xui_window.c src\xui_button.c
set SRC=%SRC% src\xui_accessibility.c
set FLAGS=-O2 -g -Wall -Wextra -Wno-unused-parameter -Wno-unused-function -Wno-cast-function-type -DXGE_DEBUGMODE=0 -ffunction-sections -fdata-sections
set LIBS=-lm -lws2_32 -liphlpapi -lgdi32 -luser32 -lshell32 -lole32 -loleaut32 -luuid -limm32 -lwinmm -lavrt
gcc %FLAGS% -I. -Wl,--gc-sections -o %OUT% %SRC% %LIBS%
if errorlevel 1 (popd & exit /b 1)
%OUT%
set RESULT=%errorlevel%
popd
exit /b %RESULT%
