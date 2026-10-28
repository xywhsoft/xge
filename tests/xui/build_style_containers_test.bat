@echo off
setlocal
pushd "%~dp0..\.." || exit /b 1
if not exist build mkdir build
call tools\build\ensure_xge_dll.bat
if errorlevel 1 (popd & exit /b 1)
call tools/build/xui_sources.bat
set XUI_SRC=%XUI_SRC:src\xui\core\xui_proxy_xge.c=%
set TEST_SRC=tests\xui\xui_style_containers_test.c
set TEST_OUT=build\xui_style_containers_test.exe
if "%~1"=="--regression" (
    set TEST_SRC=tests\xui\xui_%~2_test.c
    set TEST_OUT=build\xui_style_containers_%~2_test.exe
    if "%~2"=="table_view_index" set XUI_SRC=%XUI_SRC:src\xui\controls\xui_table_view.c=%
    goto build
)
set XUI_SRC=%XUI_SRC:src\xui\controls\xui_dock_panel.c=%
set XUI_SRC=%XUI_SRC:src\xui\controls\xui_property_grid.c=%
:build
gcc -O0 -g -Wall -Wextra -Wno-unused-parameter -Wno-unused-function -Wno-cast-function-type -DXGE_DEBUGMODE=0 @config/includes.rsp -o %TEST_OUT% %TEST_SRC% tests\xui\xui_test_proxy.c tests\xui\xui_test_xrt_impl.c %XUI_SRC% build\xge.lib -lm -lws2_32 -liphlpapi -lgdi32 -luser32 -lshell32 -lopengl32 -lole32 -loleaut32 -lwinmm -lavrt -luuid
if errorlevel 1 (popd & exit /b 1)
%TEST_OUT% %*
set RESULT=%errorlevel%
popd
exit /b %RESULT%
