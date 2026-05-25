@echo off
setlocal

set ROOT=%~dp0..\..
for %%I in ("%ROOT%") do set ROOT=%%~fI
set TOOL_DIR=%~dp0.
for %%I in ("%TOOL_DIR%") do set TOOL_DIR=%%~fI
set OUT_DIR=%TOOL_DIR%\release
set OUT=%OUT_DIR%\xui_designer.exe
set RES_OBJ=%OUT_DIR%\uidesign_res.o
set XGE_LIB=%ROOT%\build\xge.lib
set XGE_DLL=%ROOT%\build\xge.dll
set OPTION_DIR=%TOOL_DIR%\option
set RC_FILE=%TOOL_DIR%\UIDesign.rc
set SRC="%TOOL_DIR%\src\main.c" "%TOOL_DIR%\src\core\uid_app.c" "%TOOL_DIR%\src\core\uid_document.c" "%TOOL_DIR%\src\core\uid_project.c" "%TOOL_DIR%\src\core\uid_registry.c" "%TOOL_DIR%\src\core\uid_undo.c" "%TOOL_DIR%\src\core\uid_refresh.c" "%TOOL_DIR%\src\designer\uid_shell.c" "%TOOL_DIR%\src\platform\uid_backend_events.c"
set FLAGS=-O2 -Wall -Wextra -Wno-unused-parameter -Wno-unused-function -DXGE_DLL -DXGE_DEBUGMODE=0
set LIBS=%XGE_LIB% -lws2_32 -liphlpapi -lgdi32 -luser32 -lshell32 -lcomdlg32 -lopengl32 -lole32 -lwinmm -lavrt

where gcc >nul 2>nul
if %errorlevel% neq 0 (
	echo [UIDESIGN] gcc not found in PATH
	exit /b 1
)

if not exist "%OUT_DIR%" mkdir "%OUT_DIR%" || exit /b 1
where windres >nul 2>nul
if %errorlevel% neq 0 (
	echo [UIDESIGN] windres not found in PATH
	exit /b 1
)
if not exist "%XGE_LIB%" (
	call "%ROOT%\build_dll.bat"
	if errorlevel 1 exit /b %ERRORLEVEL%
)
if not exist "%XGE_DLL%" (
	call "%ROOT%\build_dll.bat"
	if errorlevel 1 exit /b %ERRORLEVEL%
)

echo [UIDESIGN] Building resources...
pushd "%TOOL_DIR%" >nul
windres -O coff -i "%RC_FILE%" -o "%RES_OBJ%"
set WINDRES_RET=%ERRORLEVEL%
popd >nul
if not "%WINDRES_RET%"=="0" (
	echo [UIDESIGN] Resource build failed
	exit /b %WINDRES_RET%
)

echo [UIDESIGN] Building XUI Designer...
gcc %FLAGS% -I"%ROOT%" -I"%TOOL_DIR%" -I"%TOOL_DIR%\src\core" -I"%TOOL_DIR%\src\designer" -I"%TOOL_DIR%\src\platform" -o "%OUT%" %SRC% "%RES_OBJ%" %LIBS%
if errorlevel 1 (
	echo [UIDESIGN] Build failed
	exit /b 1
)

copy /Y "%XGE_DLL%" "%OUT_DIR%\xge.dll" >nul
if errorlevel 1 (
	echo [UIDESIGN] Failed to copy xge.dll
	exit /b 1
)
if not exist "%OUT_DIR%\option" mkdir "%OUT_DIR%\option" || exit /b 1
copy /Y "%OPTION_DIR%\*.xson" "%OUT_DIR%\option\" >nul
if errorlevel 1 (
	echo [UIDESIGN] Failed to copy option xson files
	exit /b 1
)
if not exist "%OUT_DIR%\res" mkdir "%OUT_DIR%\res" || exit /b 1
if not exist "%OUT_DIR%\assets" mkdir "%OUT_DIR%\assets" || exit /b 1

echo [UIDESIGN] Build successful: %OUT%
endlocal
