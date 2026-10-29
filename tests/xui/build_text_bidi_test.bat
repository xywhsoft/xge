@echo off
setlocal
if not exist build\document mkdir build\document
gcc -std=c11 -O2 -Wall -Wextra -Werror -DXUI_BIDI_TEST_ALLOCATOR @config/includes.rsp tests\xui\xui_text_bidi_test.c src\xui\core\xui_text_bidi.c -o build\document\xui_text_bidi_test.exe
if errorlevel 1 exit /b 1
set CORPUS=%~1
if not defined CORPUS set CORPUS=tests\xui\data\unicode-17\BidiCharacterTest.txt
set CLASS_CORPUS=%~2
if not defined CLASS_CORPUS set CLASS_CORPUS=tests\xui\data\unicode-17\BidiTest.txt
if not exist "%CORPUS%" exit /b 1
if not exist "%CLASS_CORPUS%" exit /b 1
build\document\xui_text_bidi_test.exe "%CORPUS%" "%CLASS_CORPUS%"
exit /b %errorlevel%
