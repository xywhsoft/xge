@echo off
setlocal

set ROOT=%~dp0..\..
for %%I in ("%ROOT%") do set ROOT=%%~fI
set TOOL_DIR=%~dp0.
for %%I in ("%TOOL_DIR%") do set TOOL_DIR=%%~fI
set OUT_DIR=%TOOL_DIR%\release
set OUT=%OUT_DIR%\xui_asset_atlas.exe

where gcc >nul 2>nul
if errorlevel 1 (
	echo [XUI ATLAS] gcc not found in PATH
	exit /b 1
)
if not exist "%OUT_DIR%" mkdir "%OUT_DIR%" || exit /b 1

echo [XUI ATLAS] Building generator...
gcc -std=c11 -O2 -Wall -Wextra -Wno-unused-function -Wno-unused-parameter -I"%TOOL_DIR%" ^
	-o "%OUT%" "%TOOL_DIR%\xui_asset_atlas.c" "%TOOL_DIR%\zstd.c"
if errorlevel 1 (
	echo [XUI ATLAS] Build failed
	exit /b 1
)
echo [XUI ATLAS] Build successful: %OUT%
endlocal
