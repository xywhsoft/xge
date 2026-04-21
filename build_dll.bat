@echo off
setlocal

set OUT_DIR=build
set OUT=%OUT_DIR%\xge.dll
set SRC=xge.c
set INC=-I.
set FLAGS=-O2 -Wall -Wextra -Wno-unused-parameter -Wno-unused-function -Wno-cast-function-type -DXGE_DLL -DXGE_BUILD_DLL
set LIBS=-lws2_32 -liphlpapi -lgdi32 -luser32 -lshell32 -lopengl32 -lole32 -lwinmm -lavrt

where gcc >nul 2>nul
if %errorlevel% neq 0 (
	echo [ERROR] gcc not found in PATH
	exit /b 1
)

if not exist %OUT_DIR% (
	mkdir %OUT_DIR% || exit /b 1
)

echo [XGE] Building DLL...
gcc -shared %FLAGS% %INC% -o %OUT% %SRC% %LIBS%
if %errorlevel% neq 0 (
	echo [XGE] Build failed
	exit /b 1
)

echo [XGE] Build successful: %OUT%
endlocal
