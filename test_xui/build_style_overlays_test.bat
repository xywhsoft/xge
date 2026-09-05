@echo off
setlocal
set OUT_DIR=build
set SRC=test_xui\xui_style_overlays_test.c test_xui\xui_test_proxy.c test_xui\xui_test_xrt_impl.c lib\xlayout\xlayout.c
set SRC=%SRC% src\xui_unicode.c src\xui_accessibility.c src\xui_core.c src\xui_widget.c src\xui_layout.c src\xui_input.c src\xui_edit.c src\xui_drag_drop.c src\xui_text.c src\xui_label.c src\xui_assets.c src\xui_builtin_atlas.c src\xui_breadcrumb.c src\xui_button.c src\xui_scroll_model.c src\xui_scrollbar.c src\xui_scroll_frame.c src\xui_scroll_view.c src\xui_popup.c src\xui_menu.c src\xui_list_view.c src\xui_input_widget.c src\xui_combobox.c src\xui_window_frame.c src\xui_window.c src\xui_msgtip.c src\xui_toast.c src\xui_msgbox.c src\xui_file_dialog.c
set FLAGS=-O2 -Wall -Wextra -Wno-unused-parameter -Wno-unused-function -Wno-cast-function-type -DXGE_DEBUGMODE=0
set LIBS=-lm -lws2_32 -liphlpapi -lgdi32 -luser32 -lshell32 -lopengl32 -lole32 -lwinmm -lavrt
where gcc >nul 2>nul
if errorlevel 1 exit /b 1
if not exist %OUT_DIR% mkdir %OUT_DIR%
gcc %FLAGS% -I. -o %OUT_DIR%\xui_style_overlays_test.exe %SRC% %LIBS%
if errorlevel 1 exit /b 1
%OUT_DIR%\xui_style_overlays_test.exe
exit /b %errorlevel%
