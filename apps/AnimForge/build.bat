@echo off
setlocal
cd /d "%~dp0\..\.."

set OUT_DIR=build
set OUT=%OUT_DIR%\AnimForge.exe
set AF_DIR=apps\AnimForge

REM --- AnimForge source files ---
set AF_SRC=%AF_DIR%\src\main.c %AF_DIR%\src\core\af_app.c %AF_DIR%\src\core\af_doc.c %AF_DIR%\src\core\af_undo.c %AF_DIR%\src\core\af_file.c %AF_DIR%\src\core\af_import.c %AF_DIR%\src\core\af_export.c %AF_DIR%\src\editor\af_tools.c %AF_DIR%\src\editor\af_tool_shape.c %AF_DIR%\src\editor\af_tool_pen.c %AF_DIR%\src\editor\af_tool_select.c %AF_DIR%\src\editor\af_tool_fill.c %AF_DIR%\src\editor\af_tool_text.c %AF_DIR%\src\editor\af_tool_brush.c %AF_DIR%\src\editor\af_timeline_ui.c %AF_DIR%\src\editor\af_stage.c %AF_DIR%\src\editor\af_curve_editor.c %AF_DIR%\src\editor\af_library.c %AF_DIR%\src\editor\af_property.c %AF_DIR%\src\editor\af_onion.c %AF_DIR%\src\editor\af_shell.c %AF_DIR%\src\editor\af_dialogs.c %AF_DIR%\src\anim\af_easing.c %AF_DIR%\src\anim\af_tween.c %AF_DIR%\src\anim\af_playback.c %AF_DIR%\src\anim\af_symbol.c %AF_DIR%\src\anim\af_timeline.c %AF_DIR%\src\anim\af_shape_morph.c

set INC=@config/includes.rsp -I%AF_DIR%\src -I%AF_DIR%\lib\xanim
set FLAGS=-O2 -Wall -Wextra -Wno-unused-parameter -Wno-unused-function -Wno-cast-function-type -DXGE_DLL -DXUI_DLL -DXGE_DEBUGMODE=0
set XGE_LIB=%OUT_DIR%\xge.lib
set LIBS=%XGE_LIB% -lm -lws2_32 -liphlpapi -lgdi32 -luser32 -lshell32 -lopengl32 -lole32 -lwinmm -lavrt

where gcc >nul 2>nul
if %errorlevel% neq 0 (
	echo [ERROR] gcc not found in PATH
	exit /b 1
)

if not exist %OUT_DIR% (
	mkdir %OUT_DIR% || exit /b 1
)

if not exist %XGE_LIB% (
	echo [AnimForge] Building xge.dll first...
	call tools\build\build_dll.bat
	if %errorlevel% neq 0 exit /b %errorlevel%
)

echo [AnimForge] Building AnimForge...
gcc %FLAGS% %INC% -o %OUT% %AF_SRC% %LIBS%
if %errorlevel% neq 0 (
	echo [AnimForge] Build FAILED
	exit /b 1
)

echo [AnimForge] Build successful: %OUT%
endlocal
