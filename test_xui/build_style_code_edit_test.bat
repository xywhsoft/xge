@echo off
setlocal
set OUT=build\xui_style_code_edit_test.exe
call ensure_xge_dll.bat
if %errorlevel% neq 0 exit /b %errorlevel%
gcc -O2 -Wall -Wextra -DXGE_DLL -I. -o %OUT% test_xui\xui_style_code_edit_test.c test_xui\xui_test_proxy.c build\xge.lib -lm -lws2_32 -liphlpapi -lgdi32 -luser32 -lshell32 -lopengl32 -lole32 -lwinmm -lavrt
if %errorlevel% neq 0 exit /b %errorlevel%
%OUT%
exit /b %errorlevel%
