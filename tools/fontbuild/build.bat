@echo off
setlocal

set ROOT=%~dp0..\..
for %%I in ("%ROOT%") do set ROOT=%%~fI
set TOOL_DIR=%~dp0.
for %%I in ("%TOOL_DIR%") do set TOOL_DIR=%%~fI
set OUT_DIR=%TOOL_DIR%\release
set OUT=%OUT_DIR%\xge_fontbuild.exe
set SRC=%TOOL_DIR%\fontbuild.c
set FLAGS=-O2 -Wall -Wextra -Wno-unused-parameter -Wno-unused-function
set LIBS=-lgdi32

where gcc >nul 2>nul
if %errorlevel% neq 0 (
	echo [FONTBUILD] gcc not found in PATH
	exit /b 1
)

if not exist "%OUT_DIR%" mkdir "%OUT_DIR%" || exit /b 1

echo [FONTBUILD] Building font generator...
gcc %FLAGS% -o "%OUT%" "%SRC%" %LIBS%
if errorlevel 1 (
	echo [FONTBUILD] Build failed
	exit /b 1
)

echo [FONTBUILD] Build successful: %OUT%
endlocal
