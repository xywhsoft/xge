@echo off
setlocal
cd /d "%~dp0.."
if not exist build mkdir build || exit /b 1
set SRC=test_xui\xui_text_edit_lifetime_test.c test_xui\xui_test_proxy.c test_xui\xui_test_xrt_impl.c lib\xlayout\xlayout.c
set SRC=%SRC% src\xui_core.c src\xui_widget.c src\xui_layout.c src\xui_input.c src\xui_edit.c src\xui_drag_drop.c src\xui_accessibility.c
set SRC=%SRC% src\xui_text.c src\xui_assets.c src\xui_builtin_atlas.c src\xui_unicode.c src\xui_icon.c src\xui_label.c src\xui_button.c src\xui_checkbox.c src\xui_input_widget.c
set SRC=%SRC% src\xui_text_edit.c src\xui_find.c src\xui_window_frame.c src\xui_window.c src\xui_popup.c src\xui_menu.c src\xui_scroll_model.c src\xui_scrollbar.c src\xui_scroll_frame.c src\xui_scroll_view.c
gcc -O1 -g -Wall -Wextra -Wno-unused-parameter -Wno-unused-function -Wno-cast-function-type -DXGE_DEBUGMODE=0 -I. -o build\xui_text_edit_lifetime_test.exe %SRC% -Wl,--wrap=xuiWidgetDestroy -Wl,--wrap=xrtFree -lm -lws2_32 -liphlpapi -lgdi32 -luser32 -lshell32 -lopengl32 -lole32 -lwinmm -lavrt
if errorlevel 1 exit /b %errorlevel%
build\xui_text_edit_lifetime_test.exe %*
exit /b %errorlevel%
