@echo off
setlocal
set OUT=build\xui_style_edit_disabled_test.exe
call tools\build\ensure_xge_dll.bat
if %errorlevel% neq 0 exit /b %errorlevel%
gcc -O2 -Wall -Wextra -DXGE_DLL @config/includes.rsp -o %OUT% tests\xui\xui_style_edit_disabled_test.c tests\xui\xui_test_proxy.c build\xge.lib -lm -lws2_32 -liphlpapi -lgdi32 -luser32 -lshell32 -lopengl32 -lole32 -lwinmm -lavrt
if %errorlevel% neq 0 exit /b %errorlevel%
%OUT%
exit /b %errorlevel%
