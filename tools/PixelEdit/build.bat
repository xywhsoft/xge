@echo off
setlocal

set ROOT=%~dp0..\..
for %%I in ("%ROOT%") do set ROOT=%%~fI
set TOOL_DIR=%~dp0.
for %%I in ("%TOOL_DIR%") do set TOOL_DIR=%%~fI
set OUT_DIR=%TOOL_DIR%\release
set OUT=%OUT_DIR%\xge_pixeledit.exe
set XGE_LIB=%ROOT%\build\xge.lib
set XGE_DLL=%ROOT%\build\xge.dll
set SRC="%TOOL_DIR%\src\main.c" "%TOOL_DIR%\src\pxe_canvas.c"
set FLAGS=-O2 -Wall -Wextra -Wno-unused-parameter -Wno-unused-function -DXGE_DLL -DXUI_DLL -DXGE_DEBUGMODE=0
set LIBS=%XGE_LIB% -lm -lws2_32 -liphlpapi -lgdi32 -luser32 -lshell32 -lcomdlg32 -lole32 -lwinmm -lavrt

where gcc >nul 2>nul
if %errorlevel% neq 0 (
	echo [PIXELEDIT:XUI2] gcc not found in PATH
	exit /b 1
)

if not exist "%OUT_DIR%" mkdir "%OUT_DIR%" || exit /b 1
if not exist "%XGE_LIB%" (
	pushd "%ROOT%" >nul
	call build_dll.bat
	set BUILD_RET=%ERRORLEVEL%
	popd >nul
	if not "%BUILD_RET%"=="0" exit /b %BUILD_RET%
)
if not exist "%XGE_DLL%" (
	pushd "%ROOT%" >nul
	call build_dll.bat
	set BUILD_RET=%ERRORLEVEL%
	popd >nul
	if not "%BUILD_RET%"=="0" exit /b %BUILD_RET%
)

echo [PIXELEDIT:XUI2] Building PixelEdit...
pushd "%ROOT%" >nul
gcc %FLAGS% -I"%ROOT%" -I"%TOOL_DIR%\src" -o "%OUT%" %SRC% %LIBS%
set GCC_RET=%ERRORLEVEL%
popd >nul
if not "%GCC_RET%"=="0" (
	echo [PIXELEDIT:XUI2] Build failed
	exit /b 1
)

copy /Y "%XGE_DLL%" "%OUT_DIR%\xge.dll" >nul
if not exist "%OUT_DIR%\option" mkdir "%OUT_DIR%\option" || exit /b 1
if exist "%TOOL_DIR%\option\setup.xson" (
	copy /Y "%TOOL_DIR%\option\setup.xson" "%OUT_DIR%\option\" >nul
)
if not exist "%OUT_DIR%\output" mkdir "%OUT_DIR%\output" || exit /b 1

echo [PIXELEDIT:XUI2] Build successful: %OUT%
endlocal
