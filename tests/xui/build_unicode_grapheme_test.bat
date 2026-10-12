@echo off
setlocal
cd /d "%~dp0\..\.."
if not exist build\document mkdir build\document
gcc -std=c11 -O2 -Wall -Wextra -Werror @config/includes.rsp -o build\document\xui_unicode_grapheme_test.exe tests\xui\xui_unicode_grapheme_test.c src\xui\core\xui_unicode.c
if errorlevel 1 exit /b 1
build\document\xui_unicode_grapheme_test.exe
exit /b %errorlevel%
