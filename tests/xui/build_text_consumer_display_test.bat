@echo off
setlocal
pushd "%~dp0..\.." || exit /b 1
set OUT=build\xui_text_consumer_display_test.exe
if not exist build (mkdir build || (popd & exit /b 1))
rem The five consumers are included in the test to count only their allocations.
set SRC=tests\xui\xui_text_consumer_display_test.c tests\xui\xui_test_proxy.c tests\xui\xui_test_xrt_impl.c modules\xlayout\xlayout.c src\xui\core\xui_core.c src\xui\accessibility\xui_native_accessibility.c src\xui\core\xui_widget.c src\xui\core\xui_layout.c src\xui\core\xui_input.c src\xui\core\xui_edit.c src\xui\core\xui_drag_drop.c src\xui\core\xui_unicode.c src\xui\core\xui_text.c src\xui\core\xui_assets.c src\xui\core\xui_builtin_atlas.c src\xui\core\xui_icon.c src\xui\core\xui_scroll_model.c src\xui\controls\xui_scrollbar.c src\xui\controls\xui_scroll_frame.c src\xui\controls\xui_scroll_view.c src\xui\controls\xui_popup.c src\xui\controls\xui_menu.c src\xui\controls\xui_window_frame.c src\xui\controls\xui_window.c src\xui\controls\xui_button.c
set SRC=%SRC% src\xui\accessibility\xui_accessibility.c
set FLAGS=-O2 -g -Wall -Wextra -Wno-unused-parameter -Wno-unused-function -Wno-cast-function-type -DXGE_DEBUGMODE=0 -ffunction-sections -fdata-sections
set LIBS=-lm -lws2_32 -liphlpapi -lgdi32 -luser32 -lshell32 -lole32 -loleaut32 -luuid -limm32 -lwinmm -lavrt
gcc %FLAGS% @config/includes.rsp -Wl,--gc-sections -o %OUT% %SRC% %LIBS%
if errorlevel 1 (popd & exit /b 1)
%OUT%
set RESULT=%errorlevel%
popd
exit /b %RESULT%
