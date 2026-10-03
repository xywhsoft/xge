@echo off
setlocal
cd /d "%~dp0\.."
call ensure_xge_dll.bat
if errorlevel 1 exit /b 1
gcc -std=c11 -O2 -Wall -Wextra -Werror -DXGE_DLL -I. test\test_font_fallback_shaping.c build\xge.lib -o build\xge_font_fallback_test.exe -lm
if errorlevel 1 exit /b 1
build\xge_font_fallback_test.exe
exit /b %errorlevel%
