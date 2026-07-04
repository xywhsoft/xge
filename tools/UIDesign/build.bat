@echo off
setlocal EnableDelayedExpansion

set ROOT=%~dp0..\..
for %%I in ("%ROOT%") do set ROOT=%%~fI
set TOOL_DIR=%~dp0.
for %%I in ("%TOOL_DIR%") do set TOOL_DIR=%%~fI

set OUT_DIR=%TOOL_DIR%\release
set OUT=%OUT_DIR%\xui_uidesign.exe
set XGE_LIB=%ROOT%\build\xge.lib
set XGE_DLL=%ROOT%\build\xge.dll
set INC=-I"%ROOT%" -I"%TOOL_DIR%\src"
set FLAGS=-O2 -Wall -Wextra -Wno-unused-parameter -Wno-unused-function -Wno-cast-function-type -DXGE_DLL -DXUI_DLL -DXGE_DEBUGMODE=0
set SRC="%TOOL_DIR%\src\main.c" "%TOOL_DIR%\src\ui_design_model.c" "%TOOL_DIR%\src\ui_design_registry.c" "%TOOL_DIR%\src\ui_design_toolbox.c" "%TOOL_DIR%\src\ui_design_canvas.c" "%TOOL_DIR%\src\ui_design_inspector.c"
set LIBS=%XGE_LIB% -lm -lws2_32 -liphlpapi -lgdi32 -luser32 -lshell32 -lole32 -lwinmm -lavrt
set NEED_XGE_BUILD=0

where gcc >nul 2>nul
if %errorlevel% neq 0 (
	echo [UIDESIGN] gcc not found in PATH
	exit /b 1
)

if not exist "%OUT_DIR%" mkdir "%OUT_DIR%" || exit /b 1

if not exist "%XGE_LIB%" set NEED_XGE_BUILD=1
if not exist "%XGE_DLL%" set NEED_XGE_BUILD=1
if "%NEED_XGE_BUILD%"=="0" (
	for /f %%I in ('powershell -NoProfile -ExecutionPolicy Bypass -Command "$dll=(Get-Item -LiteralPath '%XGE_DLL%').LastWriteTimeUtc; $inputs=@('%ROOT%\xge.c','%ROOT%\xge.rc','%ROOT%\xui.h','%ROOT%\xui_sources.bat') + (Get-ChildItem -LiteralPath '%ROOT%\src' -Include '*.c','*.h' -File -Recurse | ForEach-Object FullName); if ($inputs | Where-Object { (Test-Path -LiteralPath $_) -and ((Get-Item -LiteralPath $_).LastWriteTimeUtc -gt $dll) }) { '1' } else { '0' }"') do set NEED_XGE_BUILD=%%I
)

if "%NEED_XGE_BUILD%"=="1" (
	pushd "%ROOT%" >nul
	call build_dll.bat
	set BUILD_RET=!ERRORLEVEL!
	popd >nul
	if not "!BUILD_RET!"=="0" exit /b !BUILD_RET!
)

echo [UIDESIGN] Building xui_uidesign...
pushd "%ROOT%" >nul
gcc %FLAGS% %INC% -o "%OUT%" %SRC% %LIBS%
set GCC_RET=!ERRORLEVEL!
popd >nul
if not "!GCC_RET!"=="0" (
	echo [UIDESIGN] Build failed
	exit /b !GCC_RET!
)

copy /Y "%XGE_DLL%" "%OUT_DIR%\xge.dll" >nul
if errorlevel 1 (
	echo [UIDESIGN] Failed to copy xge.dll
	exit /b 1
)

echo [UIDESIGN] Build successful: %OUT%
endlocal
