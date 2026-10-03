@echo off
setlocal
cd /d "%~dp0\..\.."
if not exist artifacts\xui-document-rebuild mkdir artifacts\xui-document-rebuild || exit /b 1
call ensure_xge_dll.bat
if %errorlevel% neq 0 exit /b 1
gcc -std=c11 -O2 -Wall -Wextra -Werror -DXGE_DLL -DXUI_DLL -I. -o build\xui_document.exe examples\xui_document\main.c build\xge.lib -lm -lws2_32 -liphlpapi -lgdi32 -luser32 -lshell32 -lole32 -lwinmm -lavrt
exit /b %errorlevel%
