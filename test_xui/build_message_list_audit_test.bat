@echo off
setlocal
set OUT_DIR=build
set OUT=%OUT_DIR%\xui_message_list_audit_test.exe
set XUI_SRC=lib\xlayout\xlayout.c src\xui_core.c src\xui_widget.c src\xui_layout.c src\xui_input.c src\xui_edit.c src\xui_drag_drop.c src\xui_unicode.c src\xui_text.c src\xui_assets.c src\xui_builtin_atlas.c src\xui_icon.c src\xui_scroll_model.c src\xui_scrollbar.c src\xui_scroll_frame.c src\xui_scroll_view.c src\xui_popup.c src\xui_menu.c
set SRC=test_xui\xui_message_list_audit_test.c test_xui\xui_test_proxy.c test_xui\xui_test_xrt_impl.c src\xui_accessibility.c %XUI_SRC%
set FLAGS=-O2 -g -Wall -Wextra -Wno-unused-parameter -Wno-unused-function -Wno-cast-function-type -DXGE_DEBUGMODE=0 %MESSAGE_LIST_TEST_FLAGS%
set LIBS=-lm -lws2_32 -liphlpapi -lgdi32 -luser32 -lshell32 -lole32 -loleaut32 -luuid -limm32 -lwinmm -lavrt
if not exist %OUT_DIR% mkdir %OUT_DIR% || exit /b 1
gcc %FLAGS% -I. -o %OUT% %SRC% %LIBS%
if errorlevel 1 exit /b 1
%OUT% %*
exit /b %errorlevel%
