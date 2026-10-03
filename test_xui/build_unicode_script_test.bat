@echo off
setlocal
cd /d "%~dp0\.."
if not exist build\document mkdir build\document
gcc -std=c11 -O2 -Wall -Wextra -Werror -I. -o build\document\xui_unicode_script_test.exe test_xui\xui_unicode_script_test.c lib\libunibreak\src\unibreakdef.c
if errorlevel 1 exit /b 1
build\document\xui_unicode_script_test.exe
exit /b %errorlevel%
