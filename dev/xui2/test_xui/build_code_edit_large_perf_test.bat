@echo off
setlocal

set OUT_DIR=build
set OUT=%OUT_DIR%\xui_code_edit_large_perf_test.exe
set SRC=test_xui\xui_code_edit_large_perf_test.c test_xui\xui_test_proxy.c src\xui_core.c src\xui_widget.c src\xui_input.c src\xui_text.c src\xui_label.c src\xui_button.c src\xui_checkbox.c src\xui_image.c src\xui_assets.c src\xui_panel.c src\xui_tabs.c src\xui_scroll_model.c src\xui_scrollbar.c src\xui_scroll_frame.c src\xui_scroll_view.c src\xui_popup.c src\xui_menu.c src\xui_window.c src\xui_input_widget.c src\xui_list_view.c src\xui_dock_panel.c src\xui_find.c src\xui_code_edit.c src\xui_code_document.c src\xui_code_selection.c src\xui_code_theme.c src\xui_code_fold_state.c src\xui_code_annotation.c src\xui_code_token_buffer.c src\xui_code_provider.c src\xui_code_margin.c src\xui_code_command.c src\xui_code_fold.c src\xui_code_editing.c src\xui_code_search.c src\xui_code_lexer_c.c src\xui_code_lexer.c src\xui_code_language.c
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

echo [XUI] Building xui_code_edit_large_perf_test...
gcc %FLAGS% %INC% -o %OUT% %SRC% %LIBS%
if %errorlevel% neq 0 (
	echo [XUI] Build failed
	exit /b 1
)

%OUT% %*
if %errorlevel% neq 0 (
	echo [XUI] Test failed
	exit /b %errorlevel%
)

echo [XUI] Tests passed: %OUT%
endlocal
