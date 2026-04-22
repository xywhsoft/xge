@echo off
setlocal

set OUT_DIR=build
set OUT=%OUT_DIR%\xge_debug_overlay.exe
set SRC=xge.c examples\debug_overlay\main.c
set INC=-I.
set FLAGS=-O2 -Wall -Wextra -Wno-unused-parameter -Wno-unused-function -Wno-cast-function-type
set LIBS=-lws2_32 -liphlpapi -lgdi32 -luser32 -lshell32 -lopengl32 -lole32 -lwinmm -lavrt

where gcc >nul 2>nul
if %errorlevel% neq 0 (
	echo [ERROR] gcc not found in PATH
	exit /b 1
)

if not exist %OUT_DIR% (
	mkdir %OUT_DIR% || exit /b 1
)

echo [XGE] Building debug overlay EXE...
gcc %FLAGS% %INC% -o %OUT% %SRC% %LIBS%
if %errorlevel% neq 0 (
	echo [XGE] Build failed
	exit /b 1
)

echo [XGE] Build successful: %OUT%
endlocal
