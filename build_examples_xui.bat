@echo off
setlocal

set ROOT=%~dp0

echo [XGE] Building XUI validation examples...
call :do_build "XUI bridge" "examples\xui_bridge\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :do_build "XUI layout validation" "examples\xui_layout_validation\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :do_build "XUI widget tree style lab" "examples\xui_widget_tree_style_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :do_build "XUI layout gallery" "examples\xui_layout_gallery\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :do_build "XUI focus capture event lab" "examples\xui_focus_capture_event_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :do_build "XUI paint host lab" "examples\xui_paint_host_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :do_build "XUI theme lab" "examples\xui_theme_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :do_build "XUI label" "examples\xui_label\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :do_build "XUI label XSON" "examples\xui_label_xson\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :do_build "XUI tooltip" "examples\xui_tooltip\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :do_build "XUI tooltip XSON" "examples\xui_tooltip_xson\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :do_build "XUI image" "examples\xui_image\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :do_build "XUI image XSON" "examples\xui_image_xson\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :do_build "XUI separator" "examples\xui_separator\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :do_build "XUI separator XSON" "examples\xui_separator_xson\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :do_build "XUI progress" "examples\xui_progress\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :do_build "XUI progress XSON" "examples\xui_progress_xson\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :do_build "XUI button" "examples\xui_button\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :do_build "XUI button XSON" "examples\xui_button_xson\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :do_build "XUI choice" "examples\xui_choice\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :do_build "XUI choice XSON" "examples\xui_choice_xson\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :do_build "XUI input" "examples\xui_input\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :do_build "XUI input XSON" "examples\xui_input_xson\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :do_build "XUI TextEdit" "examples\xui_textedit\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :do_build "XUI TextEdit XSON" "examples\xui_textedit_xson\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :do_build "XUI NumericInput" "examples\xui_numericinput\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :do_build "XUI NumericInput XSON" "examples\xui_numericinput_xson\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :do_build "XUI slider" "examples\xui_slider\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :do_build "XUI slider XSON" "examples\xui_slider_xson\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :do_build "XUI scrollbar" "examples\xui_scrollbar\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :do_build "XUI scrollbar XSON" "examples\xui_scrollbar_xson\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :do_build "XUI ScrollView" "examples\xui_scrollview\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :do_build "XUI ScrollView XSON" "examples\xui_scrollview_xson\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :do_build "XUI panel" "examples\xui_panel\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :do_build "XUI panel XSON" "examples\xui_panel_xson\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :do_build "XUI split layout" "examples\xui_split_layout\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :do_build "XUI split layout XSON" "examples\xui_split_layout_xson\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :do_build "XUI tabs" "examples\xui_tabs\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :do_build "XUI tabs XSON" "examples\xui_tabs_xson\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :do_build "XUI Window" "examples\xui_window\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :do_build "XUI Window XSON" "examples\xui_window_xson\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :do_build "XUI Popup" "examples\xui_popup\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :do_build "XUI Popup XSON" "examples\xui_popup_xson\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :do_build "XUI Menu" "examples\xui_menu\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :do_build "XUI Menu XSON" "examples\xui_menu_xson\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :do_build "XUI MenuBar" "examples\xui_menubar\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :do_build "XUI MenuBar XSON" "examples\xui_menubar_xson\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :do_build "XUI ComboBox" "examples\xui_combobox\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :do_build "XUI ComboBox XSON" "examples\xui_combobox_xson\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :do_build "XUI color picker" "examples\xui_colorpicker\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :do_build "XUI color picker XSON" "examples\xui_colorpicker_xson\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :do_build "XUI DatePicker" "examples\xui_datepicker\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :do_build "XUI DatePicker XSON" "examples\xui_datepicker_xson\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :do_build "XUI MsgBox InputBox" "examples\xui_msgbox_inputbox\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :do_build "XUI MsgBox InputBox XSON" "examples\xui_msgbox_inputbox_xson\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :do_build "XUI ListView" "examples\xui_listview\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :do_build "XUI ListView XSON" "examples\xui_listview_xson\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :do_build "XUI TreeView" "examples\xui_treeview\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :do_build "XUI TreeView XSON" "examples\xui_treeview_xson\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :do_build "XUI accordion" "examples\xui_accordion\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :do_build "XUI accordion XSON" "examples\xui_accordion_xson\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :do_build "XUI popup owner lab" "examples\xui_popup_owner_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :do_build "XUI popup policy lab" "examples\xui_popup_policy_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :do_build "XUI input proc lab" "examples\xui_input_proc_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :do_build "XUI input policy lab" "examples\xui_input_policy_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :do_build "XUI input IME policy lab" "examples\xui_input_ime_policy_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :do_build "XUI input nav policy lab" "examples\xui_input_nav_policy_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :do_build "XUI input context policy lab" "examples\xui_input_context_policy_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :do_build "XUI input blink focus lab" "examples\xui_input_blink_focus_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :do_build "XUI input pointer select lab" "examples\xui_input_pointer_select_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :do_build "XUI input replace policy lab" "examples\xui_input_replace_policy_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :do_build "XUI input delete policy lab" "examples\xui_input_delete_policy_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :do_build "XUI input clipboard policy lab" "examples\xui_input_clipboard_policy_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :do_build "XUI input password policy lab" "examples\xui_input_password_policy_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :do_build "XUI text buffer lab" "examples\xui_text_buffer_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :do_build "XUI text select lab" "examples\xui_text_select_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :do_build "XUI text delete lab" "examples\xui_text_delete_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :do_build "XUI text wrap scroll lab" "examples\xui_text_wrap_scroll_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :do_build "XUI text undo stack lab" "examples\xui_text_undo_stack_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :do_build "XUI text history limit lab" "examples\xui_text_history_limit_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :do_build "XUI text menu detail lab" "examples\xui_text_menu_detail_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :do_build "XUI text edit lab" "examples\xui_text_edit_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :do_build "XUI visual proc lab" "examples\xui_visual_proc_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :do_build "XUI tabs dirty rect lab" "examples\xui_tabs_dirty_rect_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :do_build "XUI toolbar lab" "examples\xui_toolbar_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :do_build "XUI status bar lab" "examples\xui_status_bar_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :do_build "XUI toast lab" "examples\xui_toast_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :do_build "XUI hyperlink lab" "examples\xui_hyperlink_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :do_build "XUI XSON app layout lab" "examples\xui_xson_app_layout_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :do_build "XUI XSON toast lab" "examples\xui_xson_toast_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%

echo.
echo [XGE] XUI validation examples built successfully.
exit /b 0

:do_build
echo.
echo [XGE] Building %~1...
if not exist "%ROOT%%~2" (
	echo [XGE] Missing build script: %~2
	exit /b 1
)
call "%ROOT%%~2"
set CODE=%ERRORLEVEL%
if not "%CODE%"=="0" (
	echo [XGE] Failed: %~1
	exit /b %CODE%
)
exit /b 0

:help
echo [XGE] Usage:
echo [XGE]   build_examples_all.bat
echo [XGE]   build_examples_all.bat --xui
echo [XGE]
echo [XGE] Default builds every Windows/GCC example under examples.
echo [XGE] Platform-specific variants live beside their examples, for example:
echo [XGE]   examples\platform_smoke\build_web.bat
echo [XGE]   examples\platform_smoke\build_linux_x11.sh
exit /b 0
