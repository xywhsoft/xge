@echo off
setlocal
cd /d "%~dp0.."
call ensure_xge_dll.bat
if errorlevel 1 exit /b %errorlevel%
if not exist build\document mkdir build\document
gcc -std=c11 -O2 -g -Wall -Wextra -Werror -Wno-unused-function -Wno-unused-parameter -I. -DXGE_DLL -DXUI_DLL -o build\document\xui_document_visual_prepared_text_test.exe test_xui\xui_document_visual_prepared_text_test.c test_xui\xui_test_proxy.c src\xui_unicode.c build\xge.lib -lm -lws2_32 -liphlpapi -lgdi32 -luser32 -lshell32 -lole32 -loleaut32 -luuid -lwinmm -lavrt
if errorlevel 1 exit /b %errorlevel%
set PATH=%CD%\build;%PATH%
build\document\xui_document_visual_prepared_text_test.exe
exit /b %errorlevel%
