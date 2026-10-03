@echo off
setlocal
call ensure_xge_dll.bat
if %errorlevel% neq 0 exit /b 1
gcc -std=c11 -O2 -Wall -Wextra -Werror -I. -DXGE_DLL -o build\xge_png_memory_test.exe test\test_png_memory.c build\xge.lib -lm -lws2_32 -liphlpapi -lgdi32 -luser32 -lshell32 -lole32 -lwinmm -lavrt
if %errorlevel% neq 0 exit /b 1
set PATH=%CD%\build;%PATH%
build\xge_png_memory_test.exe
exit /b %errorlevel%
