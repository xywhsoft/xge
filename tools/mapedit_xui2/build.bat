@echo off
setlocal

set ROOT=%~dp0..\..
for %%I in ("%ROOT%") do set ROOT=%%~fI
set TOOL_DIR=%~dp0.
for %%I in ("%TOOL_DIR%") do set TOOL_DIR=%%~fI
set XUI2_DIR=%ROOT%\dev\xui2
set OUT_DIR=%TOOL_DIR%\release
set OUT=%OUT_DIR%\xge_mapedit_xui2.exe
set XGE_LIB=%XUI2_DIR%\build\xge.lib
set XGE_DLL=%XUI2_DIR%\build\xge.dll

set SRC="%TOOL_DIR%\src\main.c" "%TOOL_DIR%\src\mapedit_xui2_mapdoc.c" "%ROOT%\tools\mapedit\map_sdk\xge_map.c" "%XUI2_DIR%\src\xui_core.c" "%XUI2_DIR%\src\xui_widget.c" "%XUI2_DIR%\src\xui_input.c" "%XUI2_DIR%\src\xui_text.c" "%XUI2_DIR%\src\xui_label.c" "%XUI2_DIR%\src\xui_assets.c" "%XUI2_DIR%\src\xui_scroll_model.c" "%XUI2_DIR%\src\xui_scrollbar.c" "%XUI2_DIR%\src\xui_scroll_frame.c" "%XUI2_DIR%\src\xui_scroll_view.c" "%XUI2_DIR%\src\xui_canvas.c" "%XUI2_DIR%\src\xui_popup.c" "%XUI2_DIR%\src\xui_menu.c" "%XUI2_DIR%\src\xui_toolbar.c" "%XUI2_DIR%\src\xui_statusbar.c" "%XUI2_DIR%\src\xui_dock_panel.c" "%XUI2_DIR%\src\xui_proxy_xge.c"
set FLAGS=-O2 -Wall -Wextra -Wno-unused-parameter -Wno-unused-function -Wno-cast-function-type -DXGE_DLL -DXGE_DEBUGMODE=0
set LIBS=%XGE_LIB% -lm -lws2_32 -liphlpapi -lgdi32 -luser32 -lshell32 -lopengl32 -lole32 -lwinmm -lavrt

where gcc >nul 2>nul
if %errorlevel% neq 0 (
	echo [MAPEDIT_XUI2] gcc not found in PATH
	exit /b 1
)

if not exist "%OUT_DIR%" mkdir "%OUT_DIR%" || exit /b 1
if not exist "%XGE_LIB%" (
	call "%XUI2_DIR%\build_dll.bat"
	if errorlevel 1 exit /b %ERRORLEVEL%
)
if not exist "%XGE_DLL%" (
	call "%XUI2_DIR%\build_dll.bat"
	if errorlevel 1 exit /b %ERRORLEVEL%
)

echo [MAPEDIT_XUI2] Building XUI2 map editor...
gcc %FLAGS% -I"%XUI2_DIR%" -I"%ROOT%" -I"%ROOT%\tools\mapedit" -o "%OUT%" %SRC% %LIBS%
if errorlevel 1 (
	echo [MAPEDIT_XUI2] Build failed
	exit /b 1
)

copy /Y "%XGE_DLL%" "%OUT_DIR%\xge.dll" >nul
if errorlevel 1 (
	echo [MAPEDIT_XUI2] Failed to copy xge.dll
	exit /b 1
)

echo [MAPEDIT_XUI2] Build successful: %OUT%
endlocal
