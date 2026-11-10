@echo off
setlocal
call tools\build\ensure_xge_dll.bat
if %errorlevel% neq 0 exit /b %errorlevel%
gcc -O2 -Wall -Wextra -Werror -Wno-unused-parameter -DXGE_DLL @config/includes.rsp -o build\xui_token_style_resolver_test.exe tests\xui\xui_token_style_resolver_test.c tests\xui\xui_test_proxy.c build\xge.lib -lm -lws2_32 -liphlpapi -lgdi32 -luser32 -lshell32 -lopengl32 -lole32 -lwinmm -lavrt
if %errorlevel% neq 0 exit /b %errorlevel%
build\xui_token_style_resolver_test.exe
exit /b %errorlevel%
