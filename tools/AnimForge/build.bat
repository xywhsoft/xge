@echo off
setlocal
cd /d "%~dp0\..\.."

set OUT_DIR=build
set OUT=%OUT_DIR%\AnimForge.exe
set AF_DIR=tools\AnimForge

REM --- AnimForge source files ---
set AF_SRC=%AF_DIR%\src\main.c %AF_DIR%\src\core\af_app.c %AF_DIR%\src\core\af_doc.c %AF_DIR%\src\core\af_undo.c %AF_DIR%\src\core\af_file.c %AF_DIR%\src\core\af_import.c %AF_DIR%\src\core\af_export.c %AF_DIR%\src\editor\af_tools.c %AF_DIR%\src\editor\af_tool_shape.c %AF_DIR%\src\editor\af_tool_pen.c %AF_DIR%\src\editor\af_tool_select.c %AF_DIR%\src\editor\af_tool_fill.c %AF_DIR%\src\editor\af_tool_text.c %AF_DIR%\src\editor\af_tool_brush.c %AF_DIR%\src\editor\af_timeline_ui.c %AF_DIR%\src\editor\af_stage.c %AF_DIR%\src\editor\af_curve_editor.c %AF_DIR%\src\editor\af_library.c %AF_DIR%\src\editor\af_property.c %AF_DIR%\src\editor\af_onion.c %AF_DIR%\src\editor\af_shell.c %AF_DIR%\src\editor\af_dialogs.c %AF_DIR%\src\anim\af_easing.c %AF_DIR%\src\anim\af_tween.c %AF_DIR%\src\anim\af_playback.c %AF_DIR%\src\anim\af_symbol.c %AF_DIR%\src\anim\af_timeline.c %AF_DIR%\src\anim\af_shape_morph.c

REM --- XUI source files (compiled directly into the exe) ---
set XUI_SRC=src\xui_core.c src\xui_widget.c src\xui_input.c src\xui_icon.c src\xui_assets.c src\xui_text.c src\xui_label.c src\xui_scroll_model.c src\xui_scrollbar.c src\xui_scroll_frame.c src\xui_scroll_view.c src\xui_canvas.c src\xui_popup.c src\xui_menu.c src\xui_menubar.c src\xui_toolbar.c src\xui_statusbar.c src\xui_dock_panel.c src\xui_proxy_xge.c

set INC=-I. -I%AF_DIR%\src -I%AF_DIR%\lib\xanim
set FLAGS=-O2 -Wall -Wextra -Wno-unused-parameter -Wno-unused-function -Wno-cast-function-type -DXGE_DLL -DXGE_DEBUGMODE=0
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
	call build_dll.bat
	if %errorlevel% neq 0 exit /b %errorlevel%
)

echo [AnimForge] Building AnimForge...
gcc %FLAGS% %INC% -o %OUT% %AF_SRC% %XUI_SRC% %LIBS%
if %errorlevel% neq 0 (
	echo [AnimForge] Build FAILED
	exit /b 1
)

echo [AnimForge] Build successful: %OUT%
endlocal
