@echo off
setlocal
cd /d "%~dp0..\.."
if not exist build mkdir build || exit /b 1
set SRC=tests\xui\xui_text_edit_display_test.c tests\xui\xui_test_proxy.c tests\xui\xui_test_xrt_impl.c modules\xlayout\xlayout.c
set SRC=%SRC% src\xui\core\xui_core.c src\xui\accessibility\xui_native_accessibility.c src\xui\core\xui_widget.c src\xui\core\xui_layout.c src\xui\core\xui_input.c src\xui\core\xui_edit.c src\xui\core\xui_drag_drop.c src\xui\accessibility\xui_accessibility.c
set SRC=%SRC% src\xui\core\xui_text.c src\xui\core\xui_assets.c src\xui\core\xui_builtin_atlas.c src\xui\core\xui_unicode.c src\xui\core\xui_icon.c src\xui\controls\xui_label.c src\xui\controls\xui_button.c src\xui\controls\xui_checkbox.c src\xui\core\xui_input_widget.c
set SRC=%SRC% src\xui\controls\xui_find.c src\xui\controls\xui_window_frame.c src\xui\controls\xui_window.c src\xui\controls\xui_popup.c src\xui\controls\xui_menu.c src\xui\core\xui_scroll_model.c src\xui\controls\xui_scrollbar.c src\xui\controls\xui_scroll_frame.c src\xui\controls\xui_scroll_view.c
gcc -O1 -g -Wall -Wextra -Wno-unused-parameter -Wno-unused-function -Wno-cast-function-type -DXGE_DEBUGMODE=0 @config/includes.rsp -o build\xui_text_edit_display_test.exe %SRC% -Wl,--wrap=xrtMalloc -lm -lws2_32 -liphlpapi -lgdi32 -luser32 -lshell32 -lopengl32 -lole32 -loleaut32 -lwinmm -lavrt
if errorlevel 1 exit /b %errorlevel%
build\xui_text_edit_display_test.exe %*
exit /b %errorlevel%
