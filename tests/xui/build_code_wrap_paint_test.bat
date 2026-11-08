@echo off
setlocal
call tools\build\ensure_xge_dll.bat
if errorlevel 1 exit /b 1
gcc -std=c11 -O2 -Wall -Wextra -Werror -Wno-unused-parameter -Wno-unused-function -Wno-cast-function-type -DXGE_DLL -DXUI_DLL @config/includes.rsp tests\xui\xui_code_wrap_paint_test.c tests\xui\xui_test_proxy.c build\xge.lib -lm -lws2_32 -liphlpapi -lgdi32 -luser32 -lshell32 -lopengl32 -lole32 -lwinmm -lavrt -o build\xui_code_wrap_paint_test.exe
if errorlevel 1 exit /b 1
build\xui_code_wrap_paint_test.exe
exit /b %errorlevel%
