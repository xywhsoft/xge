@echo off
setlocal
pushd "%~dp0..\.." || exit /b 1
call tools\build\ensure_xge_dll.bat
if errorlevel 1 (popd & exit /b 1)
gcc -O2 -Wall -Wextra -Wno-unused-parameter -Wno-unused-function -Wno-cast-function-type -DXGE_DLL -DXGE_DEBUGMODE=0 @config/includes.rsp -o build\xui_tree_adapter_lifetime_test.exe tests\xui\xui_tree_adapter_lifetime_test.c tests\xui\xui_test_proxy.c build\xge.lib -lm -lws2_32 -liphlpapi -lgdi32 -luser32 -lshell32 -lopengl32 -lole32 -lwinmm -lavrt
if errorlevel 1 (popd & exit /b 1)
build\xui_tree_adapter_lifetime_test.exe
set RESULT=%errorlevel%
popd
exit /b %RESULT%
