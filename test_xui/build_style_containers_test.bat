@echo off
setlocal
pushd "%~dp0.." || exit /b 1
if not exist build mkdir build
call xui_sources.bat
set XUI_SRC=%XUI_SRC:src\xui_proxy_xge.c=%
set TEST_SRC=test_xui\xui_style_containers_test.c
set TEST_OUT=build\xui_style_containers_test.exe
if "%~1"=="--regression" (
    set TEST_SRC=test_xui\xui_%~2_test.c
    set TEST_OUT=build\xui_style_containers_%~2_test.exe
    goto build
)
set XUI_SRC=%XUI_SRC:src\xui_dock_panel.c=%
:build
gcc -O0 -g -Wall -Wextra -Wno-unused-parameter -Wno-unused-function -Wno-cast-function-type -DXGE_DEBUGMODE=0 -I. -o %TEST_OUT% %TEST_SRC% test_xui\xui_test_proxy.c test_xui\xui_test_xrt_impl.c %XUI_SRC% -lm -lws2_32 -liphlpapi -lgdi32 -luser32 -lshell32 -lopengl32 -lole32 -lwinmm -lavrt
if errorlevel 1 (popd & exit /b 1)
%TEST_OUT% %*
set RESULT=%errorlevel%
popd
exit /b %RESULT%
