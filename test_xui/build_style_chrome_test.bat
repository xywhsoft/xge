@echo off
setlocal
pushd "%~dp0.." || exit /b 1
set OUT=build\xui_style_chrome_test.exe
set SRC=test_xui\xui_style_chrome_test.c test_xui\xui_test_proxy.c test_xui\xui_test_xrt_impl.c lib\xlayout\xlayout.c
set SRC=%SRC% src\xui_core.c src\xui_widget.c src\xui_layout.c src\xui_input.c src\xui_edit.c src\xui_drag_drop.c src\xui_text.c src\xui_assets.c src\xui_builtin_atlas.c src\xui_unicode.c src\xui_accessibility.c
set SRC=%SRC% src\xui_menu.c src\xui_popup.c src\xui_scroll_view.c src\xui_scroll_model.c src\xui_scroll_frame.c src\xui_window_frame.c
set FLAGS=-O1 -g -Wall -Wextra -Werror -Wno-unused-parameter -Wno-unused-function -Wno-cast-function-type -DXGE_DEBUGMODE=0 -I.
set SRC=%SRC% src\xui_numeric_input.c src\xui_input_widget.c src\xui_label.c src\xui_button.c src\xui_icon.c src\xui_image.c
set LIBS=-lm -lws2_32 -liphlpapi -lgdi32 -luser32 -lshell32 -lopengl32 -lole32 -lwinmm -lavrt
if not exist build (
	mkdir build
	if errorlevel 1 (
		popd
		exit /b 1
	)
)
gcc %FLAGS% -o "%OUT%" %SRC% %LIBS%
set RESULT=%errorlevel%
if not "%RESULT%"=="0" goto done
"%OUT%" %*
set RESULT=%errorlevel%
:done
popd
exit /b %RESULT%
