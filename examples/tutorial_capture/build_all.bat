@echo off
setlocal enabledelayedexpansion
cd /d "%~dp0\..\.."

set OUT_DIR=build
set INC=-I.
set FLAGS=-O2 -Wall -Wextra -Wno-unused-parameter -Wno-unused-function -Wno-cast-function-type -DXGE_DLL -DXGE_DEBUGMODE=0
set XGE_LIB=%OUT_DIR%\xge.lib
set LIBS=%XGE_LIB% -lm -lws2_32 -liphlpapi -lgdi32 -luser32 -lshell32 -lopengl32 -lole32 -lwinmm -lavrt

where gcc >nul 2>nul
if errorlevel 1 (
	echo [ERROR] gcc not found in PATH
	exit /b 1
)

if not exist "%OUT_DIR%" mkdir "%OUT_DIR%"

call ensure_xge_dll.bat
if errorlevel 1 exit /b 1

set /a OK=0
set /a FAIL=0

echo [TUT] Building tutorial capture demos...
echo.

for %%F in (examples\tutorial_capture\ch*_main*.c) do (
	set "SRC=%%F"
	set "BASE=%%~nF"
	set "OUT=%OUT_DIR%\!BASE!.exe"

	gcc %FLAGS% %INC% -o "!OUT!" "%%F" %LIBS% 2>nul

	if errorlevel 1 (
		echo [FAIL] !BASE!
		set /a FAIL+=1
	) else (
		echo [ OK ] !BASE!
		set /a OK+=1
	)
)

echo.
echo [TUT] Build complete: %OK% succeeded, %FAIL% failed.
endlocal
