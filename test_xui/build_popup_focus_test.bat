@echo off
setlocal
cd /d "%~dp0.."
set OUT=build\xui_popup_focus_test.exe
set SRC=test_xui\xui_popup_focus_test.c test_xui\xui_test_xrt_impl.c lib\xlayout\xlayout.c src\xui_core.c src\xui_widget.c src\xui_layout.c src\xui_input.c src\xui_edit.c src\xui_drag_drop.c src\xui_text.c src\xui_assets.c src\xui_builtin_atlas.c src\xui_unicode.c src\xui_scroll_model.c src\xui_scrollbar.c src\xui_scroll_frame.c src\xui_scroll_view.c src\xui_popup.c
set SRC=src\xui_accessibility.c %SRC%
if not exist build mkdir build || exit /b 1
gcc -O1 -g -Wall -Wextra -Wno-unused-parameter -Wno-unused-function -Wno-cast-function-type -DXGE_DEBUGMODE=0 -I. -o %OUT% %SRC% -lm -lws2_32 -liphlpapi -lgdi32 -luser32 -lshell32 -lopengl32 -lole32 -lwinmm -lavrt
if errorlevel 1 exit /b %errorlevel%
%OUT% %*
exit /b %errorlevel%
