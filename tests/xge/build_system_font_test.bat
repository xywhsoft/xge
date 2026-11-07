@echo off
setlocal

call tools\build\ensure_xge_dll.bat
if %errorlevel% neq 0 exit /b %errorlevel%

gcc -std=c11 -O2 -Wall -Wextra -Werror -DXGE_DLL @config/includes.rsp ^
    -o build\xge_system_font_test.exe tests\xge\xge_system_font_test.c ^
    build\xge.lib -lm -lws2_32 -liphlpapi -lgdi32 -luser32 -lshell32 -lole32 -lwinmm -lavrt
if %errorlevel% neq 0 exit /b %errorlevel%

build\xge_system_font_test.exe
exit /b %errorlevel%
