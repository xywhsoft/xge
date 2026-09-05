@echo off
setlocal
pushd "%~dp0.." || exit /b 1
if not exist build mkdir build
set FAMILY=%~1
if "%FAMILY%"=="" set FAMILY=choices
set SRC=test_xui\xui_style_basic_%FAMILY%_test.c test_xui\xui_test_proxy.c test_xui\xui_test_xrt_impl.c lib\xlayout\xlayout.c
set SRC=%SRC% src\xui_core.c src\xui_widget.c src\xui_layout.c src\xui_input.c src\xui_edit.c src\xui_drag_drop.c src\xui_accessibility.c
set SRC=%SRC% src\xui_text.c src\xui_assets.c src\xui_builtin_atlas.c src\xui_unicode.c src\xui_icon.c src\xui_image.c src\xui_label.c src\xui_button.c src\xui_check_card.c src\xui_hyperlink.c src\xui_checkbox.c src\xui_radio.c src\xui_toggle.c src\xui_input_widget.c src\xui_numeric_input.c src\xui_canvas.c
set SRC=%SRC% src\xui_find.c src\xui_window_frame.c src\xui_window.c src\xui_popup.c src\xui_menu.c src\xui_scroll_model.c src\xui_scrollbar.c src\xui_scroll_frame.c src\xui_scroll_view.c
gcc -O1 -g -Wall -Wextra -Werror -Wno-unused-parameter -Wno-unused-function -Wno-cast-function-type -DXGE_DEBUGMODE=0 -I. -o build\xui_style_basic_%FAMILY%_test.exe %SRC% -lm -lws2_32 -liphlpapi -lgdi32 -luser32 -lshell32 -lopengl32 -lole32 -lwinmm -lavrt
if errorlevel 1 (popd & exit /b 1)
build\xui_style_basic_%FAMILY%_test.exe
set RESULT=%errorlevel%
popd
exit /b %RESULT%
