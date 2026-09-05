@echo off
setlocal
cd /d "%~dp0.."

set OUT=build\xui_drag_drop_reentry_test.exe
set SRC=test_xui\xui_drag_drop_reentry_test.c test_xui\xui_test_xrt_impl.c lib\xlayout\xlayout.c src\xui_core.c src\xui_widget.c src\xui_layout.c src\xui_input.c src\xui_edit.c src\xui_drag_drop.c src\xui_text.c src\xui_assets.c src\xui_builtin_atlas.c src\xui_unicode.c
set FLAGS=-O1 -g -Wall -Wextra -Wno-unused-parameter -Wno-unused-function -Wno-cast-function-type -DXGE_DEBUGMODE=0 -I.
set LIBS=-lm -lws2_32 -liphlpapi -lgdi32 -luser32 -lshell32 -lopengl32 -lole32 -lwinmm -lavrt

if not exist build mkdir build || exit /b 1
gcc %FLAGS% -o %OUT% %SRC% %LIBS%
if errorlevel 1 exit /b %errorlevel%
%OUT% %*
exit /b %errorlevel%
