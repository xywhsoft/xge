@echo off
setlocal
cd /d "%~dp0\.."
call ensure_xge_dll.bat
if errorlevel 1 exit /b 1
if not exist build\document mkdir build\document
gcc -std=c11 -O2 -Wall -Wextra -Werror -DXGE_DLL -DXUI_DLL -I. test_xui\xui_document_ascii_script_public_test.c build\xge.lib -o build\document\xui_document_ascii_script_public_test.exe -lm
if errorlevel 1 exit /b 1
build\document\xui_document_ascii_script_public_test.exe
if errorlevel 1 exit /b 1
gcc -std=c11 -O2 -Wall -Wextra -Werror -DXGE_DLL -DXUI_DLL -I. test_xui\xui_document_ascii_script_native_test.c build\xge.lib -o build\document\xui_document_ascii_script_native_test.exe -lm
if errorlevel 1 exit /b 1
build\document\xui_document_ascii_script_native_test.exe
exit /b %errorlevel%
