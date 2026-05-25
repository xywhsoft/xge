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
set SRC="%TOOL_DIR%\src\main.c" "%TOOL_DIR%\src\core\mapedit_app.c" "%TOOL_DIR%\src\core\mapedit_layout.c" "%TOOL_DIR%\src\workspace\mapedit_workspace.c" "%TOOL_DIR%\src\ui\mapedit_tile_grid.c" "%TOOL_DIR%\src\workspaces\tileset\tileset_workspace.c" "%TOOL_DIR%\src\workspaces\tileset\tabs\tileset_materials.c" "%TOOL_DIR%\src\workspaces\tileset\tabs\tileset_sets.c" "%TOOL_DIR%\src\workspaces\tileset\tabs\tileset_arrange.c" "%TOOL_DIR%\src\workspaces\tileset\tabs\tileset_passage.c" "%TOOL_DIR%\src\workspaces\tileset\tabs\tileset_actor_overlay.c" "%TOOL_DIR%\src\workspaces\tileset\tabs\tileset_tags.c" "%TOOL_DIR%\src\workspaces\map\map_workspace.c" "%TOOL_DIR%\src\workspaces\map\tabs\map_maps.c" "%TOOL_DIR%\src\workspaces\map\tabs\map_tile_select.c" "%TOOL_DIR%\src\workspaces\map\tabs\map_edit.c" "%TOOL_DIR%\src\workspaces\map\tabs\map_passage.c" "%TOOL_DIR%\src\workspaces\map\tabs\map_tags.c" "%TOOL_DIR%\map_sdk\xge_map.c"
set FLAGS=-O2 -Wall -Wextra -Wno-unused-parameter -Wno-unused-function -DXGE_DLL -DXGE_DEBUGMODE=0
set LIBS=%XGE_LIB% -lws2_32 -liphlpapi -lgdi32 -luser32 -lshell32 -lcomdlg32 -lopengl32 -lole32 -lwinmm -lavrt

where gcc >nul 2>nul
if %errorlevel% neq 0 (
	echo [MAPEDIT] gcc not found in PATH
	exit /b 1
)

if not exist "%OUT_DIR%" mkdir "%OUT_DIR%" || exit /b 1
where windres >nul 2>nul
if %errorlevel% neq 0 (
	echo [MAPEDIT] windres not found in PATH
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

echo [MAPEDIT] Building resources...
pushd "%TOOL_DIR%" >nul
windres -O coff -i "%RC_FILE%" -o "%RES_OBJ%"
set WINDRES_RET=%ERRORLEVEL%
popd >nul
if not "%WINDRES_RET%"=="0" (
	echo [MAPEDIT] Resource build failed
	exit /b %WINDRES_RET%
)

echo [MAPEDIT] Building map editor...
gcc %FLAGS% -I"%ROOT%" -I"%TOOL_DIR%" -I"%TOOL_DIR%\src\core" -I"%TOOL_DIR%\src\workspace" -I"%TOOL_DIR%\src\ui" -I"%TOOL_DIR%\src\workspaces\tileset" -I"%TOOL_DIR%\src\workspaces\map" -o "%OUT%" %SRC% "%RES_OBJ%" %LIBS%
if errorlevel 1 (
	echo [MAPEDIT] Build failed
	exit /b 1
)

copy /Y "%XGE_DLL%" "%OUT_DIR%\xge.dll" >nul
if errorlevel 1 (
	echo [MAPEDIT] Failed to copy xge.dll
	exit /b 1
)
if not exist "%OUT_DIR%\option" mkdir "%OUT_DIR%\option" || exit /b 1
copy /Y "%OPTION_DIR%\*.xson" "%OUT_DIR%\option\" >nul
if errorlevel 1 (
	echo [MAPEDIT] Failed to copy option xson files
	exit /b 1
)
if not exist "%OUT_DIR%\res" mkdir "%OUT_DIR%\res" || exit /b 1
copy /Y "%RES_DIR%\*.png" "%OUT_DIR%\res\" >nul
if errorlevel 1 (
	echo [MAPEDIT] Failed to copy toolbar icons
	exit /b 1
)
for /D %%R in ("%RES_DIR%\passage_*") do (
	if not exist "%OUT_DIR%\res\%%~nxR" mkdir "%OUT_DIR%\res\%%~nxR" || exit /b 1
	copy /Y "%%R\*.png" "%OUT_DIR%\res\%%~nxR\" >nul
	if errorlevel 1 (
		echo [MAPEDIT] Failed to copy passage overlay resources
		exit /b 1
	)
)
if not exist "%OUT_DIR%\res\fonts" mkdir "%OUT_DIR%\res\fonts" || exit /b 1
copy /Y "%RES_DIR%\fonts\*.xrf" "%OUT_DIR%\res\fonts\" >nul
if errorlevel 1 (
	echo [MAPEDIT] Failed to copy font resources
	exit /b 1
)
if not exist "%OUT_DIR%\assets" mkdir "%OUT_DIR%\assets" || exit /b 1
for %%D in (tilesheets animated_tiles autotiles state_tiles state_autotiles objects maps) do (
	if not exist "%OUT_DIR%\assets\%%D" mkdir "%OUT_DIR%\assets\%%D" || exit /b 1
)

echo [MAPEDIT] Build successful: %OUT%
endlocal
