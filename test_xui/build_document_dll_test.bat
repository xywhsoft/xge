@echo off
setlocal
call ensure_xge_dll.bat
if %errorlevel% neq 0 exit /b 1
if not exist build\document mkdir build\document || exit /b 1
gcc -std=c11 -O2 -g -Wall -Wextra -Werror -I. -DXGE_DLL -DXUI_DLL -o build\document\xui_document_dll_test.exe test_xui\xui_document_test.c build\xge.lib -lm -lws2_32 -liphlpapi
if %errorlevel% neq 0 exit /b 1
set PATH=%CD%\build;%PATH%
build\document\xui_document_dll_test.exe
exit /b %errorlevel%
