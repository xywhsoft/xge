@echo off
setlocal

set ROOT=%~dp0..\..
for %%I in ("%ROOT%") do set ROOT=%%~fI
set TOOL_DIR=%~dp0.
for %%I in ("%TOOL_DIR%") do set TOOL_DIR=%%~fI
set OUT_DIR=%TOOL_DIR%\release
set OUT=%OUT_DIR%\skeleton_edit.exe
set XGE_LIB=%ROOT%\build\xge.lib
set XGE_DLL=%ROOT%\build\xge.dll
set SRC="%TOOL_DIR%\src\main.c" "%TOOL_DIR%\src\core\ske_app.c" "%TOOL_DIR%\src\editor\ske_shell.c"
set FLAGS=-O2 -Wall -Wextra -Wno-unused-parameter -Wno-unused-function -DXGE_DLL -DXGE_DEBUGMODE=0
set LIBS=%XGE_LIB% -lws2_32 -liphlpapi -lgdi32 -luser32 -lshell32 -lcomdlg32 -lopengl32 -lole32 -lwinmm -lavrt

where gcc >nul 2>nul
if %errorlevel% neq 0 (
	echo [SKELETONEDIT] gcc not found in PATH
	exit /b 1
)

if not exist "%OUT_DIR%" mkdir "%OUT_DIR%" || exit /b 1
if not exist "%XGE_LIB%" (
	call "%ROOT%\build_dll.bat"
	if errorlevel 1 exit /b %ERRORLEVEL%
)
if not exist "%XGE_DLL%" (
	call "%ROOT%\build_dll.bat"
	if errorlevel 1 exit /b %ERRORLEVEL%
)

echo [SKELETONEDIT] Building SkeletonEdit...
gcc %FLAGS% -I"%ROOT%" -I"%TOOL_DIR%\src\core" -I"%TOOL_DIR%\src\editor" -o "%OUT%" %SRC% %LIBS%
if errorlevel 1 (
	echo [SKELETONEDIT] Build failed
	exit /b 1
)

copy /Y "%XGE_DLL%" "%OUT_DIR%\xge.dll" >nul
if errorlevel 1 (
	echo [SKELETONEDIT] Failed to copy xge.dll
	exit /b 1
)
if not exist "%OUT_DIR%\projects" mkdir "%OUT_DIR%\projects" || exit /b 1
if not exist "%OUT_DIR%\assets" mkdir "%OUT_DIR%\assets" || exit /b 1

echo [SKELETONEDIT] Build successful: %OUT%
endlocal
