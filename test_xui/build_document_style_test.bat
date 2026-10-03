@echo off
setlocal
call ensure_xge_dll.bat
if %errorlevel% neq 0 exit /b 1
if not exist build\document mkdir build\document
gcc -std=c11 -O2 -g -Wall -Wextra -Werror -Wno-unused-function -I. -DXGE_DLL -DXUI_DLL -o build\document\xui_document_style_test.exe test_xui\xui_document_style_test.c test_xui\xui_test_proxy.c build\xge.lib -lm -lws2_32 -liphlpapi -lgdi32 -luser32 -lshell32 -lole32 -lwinmm -lavrt
if %errorlevel% neq 0 exit /b 1
set PATH=%CD%\build;%PATH%
build\document\xui_document_style_test.exe
exit /b %errorlevel%
