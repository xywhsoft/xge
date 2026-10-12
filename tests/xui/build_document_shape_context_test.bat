@echo off
setlocal
cd /d "%~dp0\..\.."
if not exist build\document mkdir build\document
gcc -std=c11 -O2 -Wall -Wextra -Werror -Wno-unused-function @config/includes.rsp -o build\document\xui_document_shape_context_test.exe tests\xui\xui_document_shape_context_test.c -lm
if errorlevel 1 exit /b 1
build\document\xui_document_shape_context_test.exe
exit /b %errorlevel%
