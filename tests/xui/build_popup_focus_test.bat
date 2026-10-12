@echo off
setlocal
cd /d "%~dp0..\.."
set OUT=build\xui_popup_focus_test.exe
set SRC=tests\xui\xui_popup_focus_test.c tests\xui\xui_test_xrt_impl.c modules\xlayout\xlayout.c src\xui\core\xui_core.c src\xui\accessibility\xui_native_accessibility.c src\xui\core\xui_widget.c src\xui\core\xui_layout.c src\xui\core\xui_input.c src\xui\core\xui_edit.c src\xui\core\xui_drag_drop.c src\xui\core\xui_text.c src\xui\core\xui_assets.c src\xui\core\xui_builtin_atlas.c src\xui\core\xui_unicode.c src\xui\core\xui_scroll_model.c src\xui\controls\xui_scrollbar.c src\xui\controls\xui_scroll_frame.c src\xui\controls\xui_scroll_view.c src\xui\controls\xui_popup.c
set SRC=src\xui\accessibility\xui_accessibility.c %SRC%
if not exist build mkdir build || exit /b 1
gcc -O1 -g -Wall -Wextra -Wno-unused-parameter -Wno-unused-function -Wno-cast-function-type -DXGE_DEBUGMODE=0 @config/includes.rsp -o %OUT% %SRC% -lm -lws2_32 -liphlpapi -lgdi32 -luser32 -lshell32 -lopengl32 -lole32 -loleaut32 -lwinmm -lavrt
if errorlevel 1 exit /b %errorlevel%
%OUT% %*
exit /b %errorlevel%
