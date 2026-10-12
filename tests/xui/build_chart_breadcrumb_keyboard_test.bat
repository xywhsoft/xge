@echo off
setlocal

set OUT_DIR=build
set OUT=%OUT_DIR%\xui_chart_breadcrumb_keyboard_test.exe
set SRC=tests\xui\xui_chart_breadcrumb_keyboard_test.c tests\xui\xui_test_proxy.c
set FLAGS=-O2 -Wall -Wextra -Wno-unused-parameter -Wno-unused-function -Wno-cast-function-type -DXGE_DLL -DXGE_DEBUGMODE=0
set LIBS=%OUT_DIR%\xge.lib -lm -lws2_32 -liphlpapi -lgdi32 -luser32 -lshell32 -lopengl32 -lole32 -lwinmm -lavrt

where gcc >nul 2>nul
if errorlevel 1 exit /b 1
if not exist %OUT_DIR% mkdir %OUT_DIR%
call tools\build\ensure_xge_dll.bat
if errorlevel 1 exit /b 1

gcc %FLAGS% @config/includes.rsp -o %OUT% %SRC% %LIBS%
if errorlevel 1 exit /b 1
%OUT%
exit /b %errorlevel%
