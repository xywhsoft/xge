@echo off
setlocal
set OUT_DIR=build
set OUT=%OUT_DIR%\xui_rich_edit_test.exe
set SRC=test_xui\xui_rich_edit_test.c test_xui\xui_test_proxy.c
set FLAGS=-O2 -Wall -Wextra -Wno-unused-parameter -Wno-unused-function -Wno-cast-function-type -DXGE_DLL
if not exist %OUT_DIR% mkdir %OUT_DIR%
call ensure_xge_dll.bat
if %errorlevel% neq 0 exit /b %errorlevel%
gcc %FLAGS% -I. -o %OUT% %SRC% build\xge.lib -lm -lws2_32 -liphlpapi -lgdi32 -luser32 -lshell32 -lopengl32 -lole32 -lwinmm -lavrt
if %errorlevel% neq 0 exit /b %errorlevel%
%OUT%
exit /b %errorlevel%
