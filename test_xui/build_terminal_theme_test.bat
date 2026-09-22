@echo off
setlocal
pushd "%~dp0.." || exit /b 1
call ensure_xge_dll.bat
if errorlevel 1 goto failed
gcc -O2 -Wall -Wextra -DXGE_DLL -DXGE_DEBUGMODE=0 -I. -o build\xui_terminal_theme_test.exe test_xui\xui_terminal_theme_test.c test_xui\xui_test_proxy.c build\xge.lib -lm -lws2_32 -liphlpapi -lgdi32 -luser32 -lshell32 -lopengl32 -lole32 -lwinmm -lavrt
if errorlevel 1 goto failed
build\xui_terminal_theme_test.exe
if errorlevel 1 goto failed
popd
exit /b 0
:failed
popd
exit /b 1
