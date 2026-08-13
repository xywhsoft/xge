@echo off
setlocal

set OUT_DIR=build
set OUT=%OUT_DIR%\test_native_input_coordinates.exe
set SRC=test\test_native_input_coordinates.c
set INC=-I.
set FLAGS=-O1 -Wall -Wextra -Wno-unused-parameter -Wno-unused-function -Wno-cast-function-type -DXGE_DEBUGMODE=0
set LIBS=-lm -lws2_32 -liphlpapi -lgdi32 -luser32 -lshell32 -lopengl32 -lole32 -loleaut32 -luuid -limm32 -lwinmm -lavrt

where gcc >nul 2>nul
if %errorlevel% neq 0 (
	echo [ERROR] gcc not found in PATH
	exit /b 1
)

if not exist %OUT_DIR% mkdir %OUT_DIR% || exit /b 1

echo [XGE] Building native input coordinate test...
gcc %FLAGS% %INC% -o %OUT% %SRC% %LIBS%
if %errorlevel% neq 0 exit /b 1

%OUT%
if %errorlevel% neq 0 exit /b 1

echo [XGE] Native input coordinate test passed: %OUT%
endlocal
