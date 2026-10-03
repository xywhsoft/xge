@echo off
setlocal
cd /d "%~dp0\.."
call ensure_xge_dll.bat
if %errorlevel% neq 0 exit /b 1
if not exist build\document mkdir build\document
gcc -std=c11 -O2 -Wall -Wextra -Werror -DXGE_DLL -DXUI_DLL -I. -o build\document\xui_document_projected_span_spans_test.exe test_xui\xui_document_projected_span_native_test.c build\xge.lib -lm
if %errorlevel% neq 0 exit /b 1
set PATH=%CD%\build;%PATH%
build\document\xui_document_projected_span_spans_test.exe
if %errorlevel% neq 0 exit /b 1
gcc -std=c11 -O2 -Wall -Wextra -Werror -DXGE_DLL -DXUI_DLL -DTEST_DRAW_TEXT_ONLY -I. -o build\document\xui_document_projected_span_plain_test.exe test_xui\xui_document_projected_span_native_test.c build\xge.lib -lm
if %errorlevel% neq 0 exit /b 1
build\document\xui_document_projected_span_plain_test.exe
if %errorlevel% neq 0 exit /b 1
exit /b 0
