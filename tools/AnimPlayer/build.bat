@echo off
setlocal
cd /d "%~dp0\..\.."

set OUT_DIR=build
set OUT=%OUT_DIR%\AnimPlayer.exe
set AP_DIR=tools\AnimPlayer
set XANIM_DIR=tools\AnimForge\lib\xanim

REM --- Source files ---
set SRC=%AP_DIR%\src\main.c %AP_DIR%\src\player_app.c %AP_DIR%\src\player_shell.c %XANIM_DIR%\xanim_player.c

set INC=-I. -I%XANIM_DIR%
set FLAGS=-O2 -Wall -Wextra -Wno-unused-parameter -Wno-unused-function -DXGE_DLL -DXGE_DEBUGMODE=0
set XGE_LIB=%OUT_DIR%\xge.lib
set LIBS=%XGE_LIB% -lm -lws2_32 -liphlpapi -lgdi32 -luser32 -lshell32 -lopengl32 -lole32 -lwinmm -lavrt

where gcc >nul 2>nul
if %errorlevel% neq 0 (
	echo [ERROR] gcc not found in PATH
	exit /b 1
)

if not exist %OUT_DIR% (
	mkdir %OUT_DIR% || exit /b 1
)

if not exist %XGE_LIB% (
	echo [AnimPlayer] Building xge.dll first...
	call build_dll.bat
	if %errorlevel% neq 0 exit /b %errorlevel%
)

echo [AnimPlayer] Building AnimPlayer...
gcc %FLAGS% %INC% -o %OUT% %SRC% %LIBS%
if %errorlevel% neq 0 (
	echo [AnimPlayer] Build FAILED
	exit /b 1
)

echo [AnimPlayer] Build successful: %OUT%
endlocal
