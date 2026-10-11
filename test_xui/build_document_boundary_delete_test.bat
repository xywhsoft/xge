@echo off
setlocal
cd /d "%~dp0\.."
call ensure_xge_dll.bat
if errorlevel 1 exit /b 1
if not exist build\document mkdir build\document || exit /b 1
gcc -std=c11 -O2 -g -Wall -Wextra -Werror -DXGE_DLL -DXUI_DLL -I. -o build\document\xui_document_boundary_delete_test.exe test_xui\xui_document_boundary_delete_test.c test_xui\xui_test_proxy.c build\xge.lib -lm -lws2_32 -liphlpapi -lgdi32 -luser32 -lshell32 -lole32 -lwinmm -lavrt
if errorlevel 1 exit /b 1
set PATH=%CD%\build;%PATH%
build\document\xui_document_boundary_delete_test.exe
exit /b %errorlevel%
