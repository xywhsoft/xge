@echo off
setlocal
set OUT=build\xui_text_linebreak_test.exe
set FLAGS=-O2 -Wall -Wextra -Wno-unused-parameter -Wno-unused-function -Wno-cast-function-type -DXGE_DLL -DXGE_DEBUGMODE=0
where gcc >nul 2>nul
if errorlevel 1 exit /b 1
if not exist build mkdir build
call ensure_xge_dll.bat
if errorlevel 1 exit /b %errorlevel%
gcc %FLAGS% -I. -o %OUT% test_xui\xui_text_linebreak_test.c test_xui\xui_test_proxy.c build\xge.lib -lm -lws2_32 -liphlpapi -lgdi32 -luser32 -lshell32 -lole32 -lwinmm -lavrt
if errorlevel 1 exit /b %errorlevel%
%OUT%
exit /b %errorlevel%
