@echo off
setlocal
cd /d "%~dp0\..\.."

set OUT_DIR=build
set OUT=%OUT_DIR%\xui_tablegrid.exe
set SRC=examples\xui_tablegrid\main.c src\xui_core.c src\xui_widget.c src\xui_input.c src\xui_text.c src\xui_label.c src\xui_assets.c src\xui_button.c src\xui_scroll_model.c src\xui_scrollbar.c src\xui_scroll_frame.c src\xui_scroll_view.c src\xui_popup.c src\xui_menu.c src\xui_input_widget.c src\xui_numeric_input.c src\xui_combobox.c src\xui_color_picker.c src\xui_date_picker.c src\xui_text_edit.c src\xui_table_view.c src\xui_table_grid.c src\xui_proxy_xge.c
set INC=-I.
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
	call build_dll.bat
	if %errorlevel% neq 0 exit /b %errorlevel%
)

echo [XUI] Building xui_tablegrid...
gcc %FLAGS% %INC% -o %OUT% %SRC% %LIBS%
if %errorlevel% neq 0 (
	echo [XUI] Build failed
	exit /b 1
)

echo [XUI] Build successful: %OUT%
endlocal
