@echo off
setlocal
cd /d "%~dp0\.."
call ensure_xge_dll.bat
if errorlevel 1 exit /b 1
gcc -std=c11 -O2 -Wall -Wextra -Werror -DXGE_DLL -DXUI_DLL -I. -o build\document\xui_document_script_test.exe test_xui\xui_document_script_native_test.c build\xge.lib -lm
if errorlevel 1 exit /b 1
build\document\xui_document_script_test.exe
if errorlevel 1 exit /b 1
gcc -std=c11 -O2 -Wall -Wextra -Werror -DXGE_DLL -DXUI_DLL -DTEST_DRAW_TEXT_ONLY -I. -o build\document\xui_document_script_draw_only_test.exe test_xui\xui_document_script_native_test.c build\xge.lib -lm
if errorlevel 1 exit /b 1
build\document\xui_document_script_draw_only_test.exe
exit /b %errorlevel%
