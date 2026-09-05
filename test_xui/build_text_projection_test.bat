@echo off
setlocal
set OUT=build\xui_text_projection_test.exe
set FLAGS=-std=c99 -O2 -Wall -Wextra -Wno-unused-parameter -Wno-unused-function -Wno-cast-function-type -DXGE_DLL -DXGE_DEBUGMODE=0
where gcc >nul 2>nul
if errorlevel 1 exit /b 1
if not exist build mkdir build
call ensure_xge_dll.bat
if errorlevel 1 exit /b %errorlevel%
rem Shared text is included in the test; only XRT is obtained from the DLL.
gcc %FLAGS% -I. -o %OUT% test_xui\xui_text_projection_test.c src\xui_unicode.c build\xge.lib -lm
if errorlevel 1 exit /b %errorlevel%
%OUT%
exit /b %errorlevel%
