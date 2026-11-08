@echo off
setlocal
set OUT=build\xui_table_view_pixel_test.exe
set EVIDENCE=build\table-view-pixels
call tools\build\ensure_xge_dll.bat
if errorlevel 1 exit /b %errorlevel%
gcc -O2 -Wall -Wextra -Wno-unused-parameter -DXGE_DLL -DXUI_DLL -DXGE_DEBUGMODE=0 @config/includes.rsp tests\xui\xui_table_view_pixel_test.c build\xge.lib -o "%OUT%"
if errorlevel 1 exit /b %errorlevel%
if not exist "%EVIDENCE%" mkdir "%EVIDENCE%"
if errorlevel 1 exit /b %errorlevel%
"%OUT%" "%EVIDENCE%" thin
exit /b %errorlevel%
