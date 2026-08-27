@echo off
setlocal

set OUT_DIR=build
set OUT=%OUT_DIR%\test_platform_native_handle.exe
set SRC=test\test_platform_native_handle.c
set INC=-I.
set FLAGS=-O1 -Wall -Wextra -Wno-unused-parameter -Wno-unused-function -Wno-cast-function-type -DXGE_DEBUGMODE=0
set LIBS=-lm -lws2_32 -liphlpapi -lgdi32 -luser32 -lshell32 -lopengl32 -lole32 -loleaut32 -luuid -limm32 -lwinmm -lavrt

where gcc >nul 2>nul
if %errorlevel% neq 0 exit /b 1
if not exist %OUT_DIR% mkdir %OUT_DIR% || exit /b 1

gcc %FLAGS% %INC% -o %OUT% %SRC% %LIBS%
if %errorlevel% neq 0 exit /b 1
%OUT%
if %errorlevel% neq 0 exit /b 1

echo [XGE] Platform native handle test passed: %OUT%
endlocal
