@echo off
setlocal
where gcc >nul 2>nul
if errorlevel 1 exit /b 1
call ensure_xge_dll.bat
if errorlevel 1 exit /b %errorlevel%
gcc -O2 -Wall -Wextra -Wno-unused-parameter -Wno-unused-function -Wno-cast-function-type -DXGE_DLL -DXGE_DEBUGMODE=0 -I. -o build\xui_grid_focus_test.exe test_xui\xui_grid_focus_test.c test_xui\xui_test_proxy.c build\xge.lib -lm -lws2_32 -liphlpapi -lgdi32 -luser32 -lshell32 -lopengl32 -lole32 -lwinmm -lavrt
if errorlevel 1 exit /b %errorlevel%
build\xui_grid_focus_test.exe
exit /b %errorlevel%
