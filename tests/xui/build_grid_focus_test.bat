@echo off
setlocal
where gcc >nul 2>nul
if errorlevel 1 exit /b 1
call tools\build\ensure_xge_dll.bat
if errorlevel 1 exit /b %errorlevel%
gcc -O2 -Wall -Wextra -Wno-unused-parameter -Wno-unused-function -Wno-cast-function-type -DXGE_DLL -DXGE_DEBUGMODE=0 @config/includes.rsp -o build\xui_grid_focus_test.exe tests\xui\xui_grid_focus_test.c tests\xui\xui_test_proxy.c build\xge.lib -lm -lws2_32 -liphlpapi -lgdi32 -luser32 -lshell32 -lopengl32 -lole32 -lwinmm -lavrt
if errorlevel 1 exit /b %errorlevel%
build\xui_grid_focus_test.exe
exit /b %errorlevel%
