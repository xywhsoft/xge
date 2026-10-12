@echo off
setlocal
pushd "%~dp0..\.." || exit /b 1
if not exist build mkdir build
set FAMILY=%~1
if "%FAMILY%"=="" set FAMILY=choices
set SRC=tests\xui\xui_style_basic_%FAMILY%_test.c tests\xui\xui_test_proxy.c tests\xui\xui_test_xrt_impl.c modules\xlayout\xlayout.c
set SRC=%SRC% src\xui\core\xui_core.c src\xui\accessibility\xui_native_accessibility.c src\xui\core\xui_widget.c src\xui\core\xui_layout.c src\xui\core\xui_input.c src\xui\core\xui_edit.c src\xui\core\xui_drag_drop.c src\xui\accessibility\xui_accessibility.c
set SRC=%SRC% src\xui\core\xui_text.c src\xui\core\xui_assets.c src\xui\core\xui_builtin_atlas.c src\xui\core\xui_unicode.c src\xui\core\xui_icon.c src\xui\controls\xui_image.c src\xui\controls\xui_label.c src\xui\controls\xui_button.c src\xui\controls\xui_check_card.c src\xui\controls\xui_hyperlink.c src\xui\controls\xui_checkbox.c src\xui\controls\xui_radio.c src\xui\controls\xui_toggle.c src\xui\core\xui_input_widget.c src\xui\controls\xui_numeric_input.c src\xui\controls\xui_canvas.c
set SRC=%SRC% src\xui\controls\xui_find.c src\xui\controls\xui_window_frame.c src\xui\controls\xui_window.c src\xui\controls\xui_popup.c src\xui\controls\xui_menu.c src\xui\core\xui_scroll_model.c src\xui\controls\xui_scrollbar.c src\xui\controls\xui_scroll_frame.c src\xui\controls\xui_scroll_view.c
gcc -O1 -g -Wall -Wextra -Werror -Wno-unused-parameter -Wno-unused-function -Wno-cast-function-type -DXGE_DEBUGMODE=0 @config/includes.rsp -o build\xui_style_basic_%FAMILY%_test.exe %SRC% -lm -lws2_32 -liphlpapi -lgdi32 -luser32 -lshell32 -lopengl32 -lole32 -loleaut32 -lwinmm -lavrt
if errorlevel 1 (popd & exit /b 1)
build\xui_style_basic_%FAMILY%_test.exe
set RESULT=%errorlevel%
popd
exit /b %RESULT%
