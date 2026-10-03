@echo off
setlocal
call ensure_xge_dll.bat
if errorlevel 1 exit /b 1
gcc -std=c11 -O2 -Wall -Wextra -Werror -DXGE_DLL -DXUI_DLL -I. test\test_opentype_shaping.c build\xge.lib -o build\xge_opentype_shaping_test.exe
if errorlevel 1 exit /b 1
build\xge_opentype_shaping_test.exe
exit /b %errorlevel%
