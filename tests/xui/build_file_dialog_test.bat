@echo off
setlocal

set OUT_DIR=build
set OUT=%OUT_DIR%\xui_file_dialog_test.exe
set SRC=tests\xui\xui_file_dialog_test.c tests\xui\xui_test_proxy.c tests\xui\xui_test_xrt_impl.c modules\xlayout\xlayout.c src\xui\core\xui_core.c src\xui\accessibility\xui_native_accessibility.c src\xui\core\xui_widget.c src\xui\core\xui_layout.c src\xui\core\xui_input.c src\xui\core\xui_edit.c src\xui\core\xui_drag_drop.c src\xui\core\xui_text.c src\xui\controls\xui_label.c src\xui\core\xui_assets.c src\xui\core\xui_builtin_atlas.c src\xui\controls\xui_breadcrumb.c src\xui\controls\xui_button.c src\xui\core\xui_scroll_model.c src\xui\controls\xui_scrollbar.c src\xui\controls\xui_scroll_frame.c src\xui\controls\xui_scroll_view.c src\xui\controls\xui_popup.c src\xui\controls\xui_menu.c src\xui\controls\xui_list_view.c src\xui\controls\xui_tree_view.c src\xui\core\xui_input_widget.c src\xui\controls\xui_combobox.c src\xui\controls\xui_window_frame.c src\xui\controls\xui_window.c src\xui\controls\xui_msgtip.c src\xui\controls\xui_msgbox.c src\xui\controls\xui_file_dialog.c
set SRC=src\xui\core\xui_unicode.c %SRC%
set SRC=src\xui\accessibility\xui_accessibility.c %SRC%
set INC=@config/includes.rsp
set FLAGS=-O2 -Wall -Wextra -Wno-unused-parameter -Wno-unused-function -Wno-cast-function-type -DXGE_DEBUGMODE=0 -DXUI_FILE_DIALOG_TEST=1
set LIBS=-lm -lws2_32 -liphlpapi -lgdi32 -luser32 -lshell32 -lopengl32 -lole32 -loleaut32 -luuid -lwinmm -lavrt

where gcc >nul 2>nul
if %errorlevel% neq 0 (
	echo [ERROR] gcc not found in PATH
	exit /b 1
)

if not exist %OUT_DIR% (
	mkdir %OUT_DIR% || exit /b 1
)

echo [XUI] Building xui_file_dialog_test...
gcc %FLAGS% %INC% -o %OUT% %SRC% %LIBS%
if %errorlevel% neq 0 (
	echo [XUI] Build failed
	exit /b 1
)

%OUT%
if %errorlevel% neq 0 (
	echo [XUI] Tests failed
	exit /b 1
)

echo [XUI] Tests passed: %OUT%
endlocal
