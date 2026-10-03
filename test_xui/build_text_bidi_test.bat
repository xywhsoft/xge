@echo off
setlocal
if not exist build\document mkdir build\document
gcc -std=c11 -O2 -Wall -Wextra -Werror -DXUI_BIDI_TEST_ALLOCATOR -I. test_xui\xui_text_bidi_test.c src\xui_text_bidi.c -o build\document\xui_text_bidi_test.exe
if errorlevel 1 exit /b 1
set CORPUS=%~1
if not defined CORPUS set CORPUS=artifacts\xui-document-rebuild\bidi\BidiCharacterTest-17.0.0.txt
set CLASS_CORPUS=%~2
if not defined CLASS_CORPUS set CLASS_CORPUS=artifacts\xui-document-rebuild\bidi\BidiTest-17.0.0.txt
if not exist "%CORPUS%" goto contracts
if not exist "%CLASS_CORPUS%" goto characters
build\document\xui_text_bidi_test.exe "%CORPUS%" "%CLASS_CORPUS%"
exit /b %errorlevel%
:characters
build\document\xui_text_bidi_test.exe "%CORPUS%"
exit /b %errorlevel%
:contracts
build\document\xui_text_bidi_test.exe
exit /b %errorlevel%
