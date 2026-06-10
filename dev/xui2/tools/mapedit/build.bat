@echo off
setlocal

set ROOT=%~dp0..\..
for %%I in ("%ROOT%") do set ROOT=%%~fI
set TOOL_DIR=%~dp0.
for %%I in ("%TOOL_DIR%") do set TOOL_DIR=%%~fI
set OUT_DIR=%TOOL_DIR%\release
set OUT=%OUT_DIR%\xge_mapedit.exe
set RES_OBJ=%OUT_DIR%\mapedit_res.o
set XGE_LIB=%ROOT%\build\xge.lib
set XGE_DLL=%ROOT%\build\xge.dll
set OPTION_DIR=%TOOL_DIR%\option
set RES_DIR=%TOOL_DIR%\res
set ASSETS_DIR=%TOOL_DIR%\assets
set RC_FILE=%TOOL_DIR%\mapedit.rc
set SRC="%TOOL_DIR%\src\main.c" "%TOOL_DIR%\map_sdk\xge_map.c"
set INC=-I"%ROOT%" -I"%TOOL_DIR%"
set FLAGS=-O2 -Wall -Wextra -Wno-unused-parameter -Wno-unused-function -Wno-cast-function-type -DXGE_DLL -DXUI_DLL -DXGE_DEBUGMODE=0
set LIBS=%XGE_LIB% -lm -lws2_32 -liphlpapi -lgdi32 -luser32 -lshell32 -lcomdlg32 -lopengl32 -lole32 -lwinmm -lavrt

where gcc >nul 2>nul
if %errorlevel% neq 0 (
	echo [MAPEDIT:XUI2] gcc not found in PATH
	exit /b 1
)

if not exist "%OUT_DIR%" mkdir "%OUT_DIR%" || exit /b 1
where windres >nul 2>nul
if %errorlevel% neq 0 (
	echo [MAPEDIT:XUI2] windres not found in PATH
	exit /b 1
)
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

echo [MAPEDIT:XUI2] Building resources...
pushd "%TOOL_DIR%" >nul
windres -O coff -i "%RC_FILE%" -o "%RES_OBJ%"
set WINDRES_RET=%ERRORLEVEL%
popd >nul
if not "%WINDRES_RET%"=="0" (
	echo [MAPEDIT:XUI2] Resource build failed
	exit /b %WINDRES_RET%
)

echo [MAPEDIT:XUI2] Building map editor...
pushd "%ROOT%" >nul
gcc %FLAGS% %INC% -o "%OUT%" %SRC% "%RES_OBJ%" %LIBS%
set GCC_RET=%ERRORLEVEL%
popd >nul
if not "%GCC_RET%"=="0" (
	echo [MAPEDIT:XUI2] Build failed
	exit /b %GCC_RET%
)

copy /Y "%XGE_DLL%" "%OUT_DIR%\xge.dll" >nul
if errorlevel 1 (
	echo [MAPEDIT:XUI2] Failed to copy xge.dll
	exit /b 1
)
if not exist "%OUT_DIR%\option" mkdir "%OUT_DIR%\option" || exit /b 1
del /Q "%OUT_DIR%\option\layout_*.xson" >nul 2>nul
copy /Y "%OPTION_DIR%\*.xson" "%OUT_DIR%\option\" >nul
if errorlevel 1 (
	echo [MAPEDIT:XUI2] Failed to copy option xson files
	exit /b 1
)
if not exist "%OUT_DIR%\res" mkdir "%OUT_DIR%\res" || exit /b 1
copy /Y "%RES_DIR%\*.png" "%OUT_DIR%\res\" >nul
if errorlevel 1 (
	echo [MAPEDIT:XUI2] Failed to copy toolbar icons
	exit /b 1
)
for /D %%R in ("%RES_DIR%\passage_*") do (
	if not exist "%OUT_DIR%\res\%%~nxR" mkdir "%OUT_DIR%\res\%%~nxR" || exit /b 1
	copy /Y "%%R\*.png" "%OUT_DIR%\res\%%~nxR\" >nul
	if errorlevel 1 (
		echo [MAPEDIT:XUI2] Failed to copy passage overlay resources
		exit /b 1
	)
)
if not exist "%OUT_DIR%\res\fonts" mkdir "%OUT_DIR%\res\fonts" || exit /b 1
copy /Y "%RES_DIR%\fonts\*.xrf" "%OUT_DIR%\res\fonts\" >nul
if errorlevel 1 (
	echo [MAPEDIT:XUI2] Failed to copy font resources
	exit /b 1
)
if not exist "%OUT_DIR%\assets" mkdir "%OUT_DIR%\assets" || exit /b 1
xcopy /E /I /Y "%ASSETS_DIR%" "%OUT_DIR%\assets" >nul
if errorlevel 1 (
	echo [MAPEDIT:XUI2] Failed to copy asset resources
	exit /b 1
)

echo [MAPEDIT:XUI2] Build successful: %OUT%
endlocal
