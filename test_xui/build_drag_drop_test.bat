@echo off
setlocal

set OUT_DIR=build
set OUT=%OUT_DIR%\xui_drag_drop_test.exe
set SRC=test_xui\xui_drag_drop_test.c
set FLAGS=-O2 -Wall -Wextra -Wno-unused-parameter -DXGE_DLL -DXGE_DEBUGMODE=0 -I.
set LIBS=%OUT_DIR%\xge.lib -lws2_32 -liphlpapi -lgdi32 -luser32 -lshell32 -lopengl32 -lole32 -lwinmm -lavrt

if not exist %OUT_DIR% mkdir %OUT_DIR% || exit /b 1
if not exist %OUT_DIR%\xge.dll call ensure_xge_dll.bat
if errorlevel 1 exit /b %errorlevel%

gcc %FLAGS% -o %OUT% %SRC% %LIBS%
if errorlevel 1 exit /b %errorlevel%
%OUT%
if errorlevel 1 exit /b %errorlevel%

call test_xui\build_drag_drop_reentry_test.bat
if errorlevel 1 exit /b %errorlevel%

echo [XUI] Drag/drop test passed: %OUT%
endlocal
