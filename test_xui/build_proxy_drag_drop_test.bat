@echo off
setlocal
set OUT=build\xui_proxy_drag_drop_test.exe
set LIBS=build\xge.lib -lws2_32 -liphlpapi -lgdi32 -luser32 -lshell32 -lopengl32 -lole32 -lwinmm -lavrt
if not exist build\xge.dll call ensure_xge_dll.bat
if errorlevel 1 exit /b %errorlevel%
gcc -O2 -Wall -Wextra -Wno-unused-parameter -DXGE_DLL -DXGE_DEBUGMODE=0 -I. -o %OUT% test_xui\xui_proxy_drag_drop_test.c %LIBS%
if errorlevel 1 exit /b %errorlevel%
%OUT%
if errorlevel 1 exit /b %errorlevel%
echo [XUI] Proxy drag/drop test passed: %OUT%
endlocal
