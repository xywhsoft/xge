@echo off
setlocal
pushd "%~dp0.." || exit /b 1
if not exist build mkdir build
call xui_sources.bat
set XUI_SRC=%XUI_SRC:src\xui_proxy_xge.c=%
gcc -O0 -g -Wall -Wextra -Wno-unused-parameter -Wno-unused-function -Wno-cast-function-type -DXGE_DEBUGMODE=0 -I. -o build\xui_style_drag_test.exe test_xui\xui_style_drag_test.c test_xui\xui_test_proxy.c test_xui\xui_test_xrt_impl.c %XUI_SRC% -lm -lws2_32 -liphlpapi -lgdi32 -luser32 -lshell32 -lopengl32 -lole32 -lwinmm -lavrt
if errorlevel 1 (popd & exit /b 1)
build\xui_style_drag_test.exe
set RESULT=%errorlevel%
popd
exit /b %RESULT%
