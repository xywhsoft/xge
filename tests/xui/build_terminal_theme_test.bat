@echo off
setlocal
pushd "%~dp0..\.." || exit /b 1
call tools\build\ensure_xge_dll.bat
if errorlevel 1 goto failed
gcc -O2 -Wall -Wextra -DXGE_DLL -DXGE_DEBUGMODE=0 @config/includes.rsp -o build\xui_terminal_theme_test.exe tests\xui\xui_terminal_theme_test.c tests\xui\xui_test_proxy.c build\xge.lib -lm -lws2_32 -liphlpapi -lgdi32 -luser32 -lshell32 -lopengl32 -lole32 -lwinmm -lavrt
if errorlevel 1 goto failed
build\xui_terminal_theme_test.exe
if errorlevel 1 goto failed
popd
exit /b 0
:failed
popd
exit /b 1
