@echo off
setlocal enabledelayedexpansion
cd /d "%~dp0\..\.."

set OUT_DIR=build
set INC=-I.
set FLAGS=-O2 -Wall -Wextra -Wno-unused-parameter -Wno-unused-function -Wno-cast-function-type -DXGE_DLL -DXGE_DEBUGMODE=0
set XGE_LIB=%OUT_DIR%\xge.lib
set LIBS=%XGE_LIB% -lm -lws2_32 -liphlpapi -lgdi32 -luser32 -lshell32 -lopengl32 -lole32 -lwinmm -lavrt

REM XUI source files needed for XUI chapters (ch121+)
set XUI_SRC=src\xui_core.c src\xui_widget.c src\xui_input.c src\xui_text.c src\xui_proxy_xge.c src\xui_label.c src\xui_button.c src\xui_checkbox.c src\xui_radio.c src\xui_toggle.c src\xui_check_card.c src\xui_input_widget.c src\xui_numeric_input.c src\xui_text_edit.c src\xui_image.c src\xui_separator.c src\xui_hyperlink.c src\xui_panel.c src\xui_scroll_frame.c src\xui_scroll_model.c src\xui_scroll_view.c src\xui_scrollbar.c src\xui_list_view.c src\xui_tree_view.c src\xui_table_view.c src\xui_table_grid.c src\xui_window.c src\xui_tabs.c src\xui_accordion.c src\xui_breadcrumb.c src\xui_popup.c src\xui_menu.c src\xui_menubar.c src\xui_toolbar.c src\xui_msgbox.c src\xui_msgtip.c src\xui_toast.c src\xui_statusbar.c src\xui_carousel.c src\xui_combobox.c src\xui_cascader.c src\xui_color_picker.c src\xui_date_picker.c src\xui_tag_input.c src\xui_slider.c src\xui_range_slider.c src\xui_progress.c src\xui_step_bar.c src\xui_canvas.c src\xui_chart.c src\xui_dock_panel.c src\xui_flow_graph.c src\xui_flow_graph_widget.c src\xui_timeline_view.c src\xui_property_grid.c src\xui_message_list.c src\xui_inventory_grid.c src\xui_virtual_joystick.c src\xui_qrcode.c src\xui_terminal.c src\xui_page.c src\xui_split_layout.c src\xui_find.c src\xui_assets.c src\xui_code_edit.c src\xui_code_document.c src\xui_code_editing.c src\xui_code_lexer.c src\xui_code_lexer_c.c src\xui_code_language.c src\xui_code_layout.c src\xui_code_fold.c src\xui_code_fold_state.c src\xui_code_margin.c src\xui_code_search.c src\xui_code_selection.c src\xui_code_theme.c src\xui_code_token_buffer.c src\xui_code_annotation.c src\xui_code_command.c src\xui_code_provider.c src\xui_file_dialog.c src\xui_workflow.c src\xui_workflow_widget.c

where gcc >nul 2>nul
if errorlevel 1 (
	echo [ERROR] gcc not found in PATH
	exit /b 1
)

if not exist "%OUT_DIR%" mkdir "%OUT_DIR%"

if not exist "%XGE_LIB%" (
	echo [TUT] Building xge.lib first...
	call build_dll.bat
	if errorlevel 1 exit /b 1
)

set /a OK=0
set /a FAIL=0

echo [TUT] Building tutorial capture demos...
echo.

for %%F in (examples\tutorial_capture\ch*_main*.c) do (
	set "SRC=%%F"
	set "BASE=%%~nF"
	set "OUT=%OUT_DIR%\!BASE!.exe"

	REM Determine if XUI chapter (ch121+)
	set "IS_XUI=0"
	set "CHNUM=!BASE:~2,3!"
	REM Remove leading zeros for comparison
	for /f "tokens=* delims=0" %%N in ("!CHNUM!") do set "CHNUM=%%N"
	if not defined CHNUM set "CHNUM=0"
	if !CHNUM! GEQ 121 set "IS_XUI=1"

	if "!IS_XUI!"=="1" (
		gcc %FLAGS% %INC% -o "!OUT!" "%%F" %XUI_SRC% %LIBS% 2>nul
	) else (
		gcc %FLAGS% %INC% -o "!OUT!" "%%F" %LIBS% 2>nul
	)

	if errorlevel 1 (
		echo [FAIL] !BASE!
		set /a FAIL+=1
	) else (
		echo [ OK ] !BASE!
		set /a OK+=1
	)
)

echo.
echo [TUT] Build complete: %OK% succeeded, %FAIL% failed.
endlocal
