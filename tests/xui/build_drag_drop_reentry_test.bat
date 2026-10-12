@echo off
setlocal
cd /d "%~dp0..\.."

set OUT=build\xui_drag_drop_reentry_test.exe
set SRC=tests\xui\xui_drag_drop_reentry_test.c tests\xui\xui_test_xrt_impl.c modules\xlayout\xlayout.c src\xui\core\xui_core.c src\xui\accessibility\xui_native_accessibility.c src\xui\core\xui_widget.c src\xui\core\xui_layout.c src\xui\core\xui_input.c src\xui\core\xui_edit.c src\xui\core\xui_drag_drop.c src\xui\core\xui_text.c src\xui\core\xui_assets.c src\xui\core\xui_builtin_atlas.c src\xui\core\xui_unicode.c
set SRC=src\xui\accessibility\xui_accessibility.c %SRC%
set FLAGS=-O1 -g -Wall -Wextra -Wno-unused-parameter -Wno-unused-function -Wno-cast-function-type -DXGE_DEBUGMODE=0 @config/includes.rsp
set LIBS=-lm -lws2_32 -liphlpapi -lgdi32 -luser32 -lshell32 -lopengl32 -lole32 -loleaut32 -lwinmm -lavrt

if not exist build mkdir build || exit /b 1
gcc %FLAGS% -o %OUT% %SRC% %LIBS%
if errorlevel 1 exit /b %errorlevel%
%OUT% %*
exit /b %errorlevel%
