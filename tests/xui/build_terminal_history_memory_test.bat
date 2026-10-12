@echo off
setlocal
pushd "%~dp0..\.." || exit /b 1

call tools\build\ensure_xge_dll.bat
if errorlevel 1 (popd & exit /b 1)

rem Inspect private history storage without adding a public testing API.
call tools/build/xui_sources.bat
set XUI_SRC=%XUI_SRC:src\xui\controls\xui_terminal.c=%
set OUT=build\xui_terminal_history_memory_test.exe
set FLAGS=-O2 -Wall -Wextra -Wno-unused-parameter -Wno-unused-function -Wno-cast-function-type -DXGE_DLL -DXGE_DEBUGMODE=0
gcc %FLAGS% @config/includes.rsp -c -o build\xui_terminal_history_memory_test.o tests\xui\xui_terminal_history_memory_test.c
if errorlevel 1 (popd & exit /b 1)
gcc %FLAGS% @config/includes.rsp -o %OUT% build\xui_terminal_history_memory_test.o tests\xui\xui_test_proxy.c %XUI_SRC% build\xge.lib -lm -lws2_32 -liphlpapi -lgdi32 -luser32 -lshell32 -lopengl32 -lole32 -loleaut32 -lwinmm -lavrt
if errorlevel 1 (popd & exit /b 1)
%OUT% %*
set RESULT=%errorlevel%
popd
exit /b %RESULT%
