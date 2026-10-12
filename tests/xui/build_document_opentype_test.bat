@echo off
setlocal
call tools\build\ensure_xge_dll.bat
if errorlevel 1 exit /b 1
if not exist build\document mkdir build\document
gcc -std=c11 -O2 -g -Wall -Wextra -Werror @config/includes.rsp -DXGE_DLL -DXUI_DLL -o build\document\xui_document_opentype_test.exe tests\xui\xui_document_opentype_test.c build\xge.lib -lm
if errorlevel 1 exit /b 1
set PATH=%CD%\build;%PATH%
build\document\xui_document_opentype_test.exe
exit /b %errorlevel%
