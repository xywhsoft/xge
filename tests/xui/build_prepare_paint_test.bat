@echo off
setlocal
pushd "%~dp0..\.." || exit /b 1
if not exist build mkdir build
call tools\build\ensure_xge_dll.bat
if errorlevel 1 (popd & exit /b 1)
call tools/build/xui_sources.bat
set XUI_SRC=%XUI_SRC:src\xui\core\xui_proxy_xge.c=%
gcc -O0 -g -Wall -Wextra -Wno-unused-parameter -Wno-unused-function -Wno-cast-function-type -DXGE_DEBUGMODE=0 @config/includes.rsp -o build\xui_prepare_paint_test.exe tests\xui\xui_prepare_paint_test.c tests\xui\xui_test_proxy.c tests\xui\xui_test_xrt_impl.c %XUI_SRC% build\xge.lib -lm -lws2_32 -liphlpapi -lgdi32 -luser32 -lshell32 -lopengl32 -lole32 -loleaut32 -lwinmm -lavrt
if errorlevel 1 (popd & exit /b 1)
build\xui_prepare_paint_test.exe %*
set RESULT=%errorlevel%
popd
exit /b %RESULT%
