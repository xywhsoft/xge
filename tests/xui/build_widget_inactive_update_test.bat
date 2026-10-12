@echo off
setlocal
cd /d "%~dp0\..\.."
call tools\build\ensure_xge_dll.bat
if %errorlevel% neq 0 exit /b 1
gcc -std=c11 -O2 -Wall -Wextra -Werror -DXGE_DLL -DXUI_DLL @config/includes.rsp -o build\xui_widget_inactive_update_test.exe tests\xui\xui_widget_inactive_update_test.c build\xge.lib
if %errorlevel% neq 0 exit /b 1
build\xui_widget_inactive_update_test.exe
exit /b %errorlevel%
