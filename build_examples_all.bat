@echo off
setlocal

set ROOT=%~dp0

if "%~1"=="/?" goto help
if /i "%~1"=="--help" goto help
if /i "%~1"=="help" goto help

set MODE=all
if /i "%~1"=="--xui" set MODE=xui
if /i "%~1"=="xui" set MODE=xui

if /i "%MODE%"=="xui" goto build_xui
if not "%~1"=="" (
	echo [XGE] Unknown option: %~1
	goto help
)

echo [XGE] Building all Windows examples...
call :build "core lifecycle" "examples\core_lifecycle\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :build "diagnostics caps/log" "examples\diagnostics_caps_log\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :build "viewport clip camera" "examples\viewport_clip_camera\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :build "image texture lab" "examples\image_texture_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :build "font text lab" "examples\font_text_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :build "resource provider lab" "examples\resource_provider_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :build "input state lab" "examples\input_state_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :build "audio lab" "examples\audio_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :build "scene lifecycle lab" "examples\scene_lifecycle_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :build "auto pause lab" "examples\auto_pause_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :build "render target lab" "examples\render_target_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :build "shader variant lab" "examples\shader_variant_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :build "material mesh buffer lab" "examples\material_mesh_buffer_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :build "blend depth color lab" "examples\blend_depth_color_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :build "shape full gallery" "examples\shape_full_gallery\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :build "async assets lab" "examples\async_assets_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :build "platform runtime lab" "examples\platform_runtime_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :build "offscreen EGL lab" "examples\offscreen_egl_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :build "render thread lab" "examples\render_thread_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :build "miniprogram bridge lab" "examples\miniprogram_bridge_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :build "XUI widget tree style lab" "examples\xui_widget_tree_style_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :build "XUI layout gallery" "examples\xui_layout_gallery\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :build "XUI focus capture event lab" "examples\xui_focus_capture_event_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :build "XUI paint host lab" "examples\xui_paint_host_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :build "XUI theme lab" "examples\xui_theme_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :build "XUI basic controls lab" "examples\xui_basic_controls_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :build "XUI choice controls lab" "examples\xui_choice_controls_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :build "XUI choice standard lab" "examples\xui_choice_standard_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :build "XUI value controls lab" "examples\xui_value_controls_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :build "XUI list scroll lab" "examples\xui_list_scroll_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :build "XUI list standard lab" "examples\xui_list_standard_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :build "XUI scroll standard lab" "examples\xui_scroll_standard_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :build "XUI virtual list standard lab" "examples\xui_virtual_list_standard_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :build "XUI overlay menu lab" "examples\xui_overlay_menu_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :build "XUI hyperlink lab" "examples\xui_hyperlink_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :build "MVP" "examples\mvp\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :build "app mode" "examples\app_mode\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :build "async" "examples\async\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :build "audio" "examples\audio\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :build "debug overlay" "examples\debug_overlay\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :build "gamepad" "examples\gamepad\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :build "input" "examples\input\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :build "isometric depth" "examples\isometric_depth\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :build "manual validation" "examples\manual_validation\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :build "perspective quad" "examples\perspective_quad\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :build "platform smoke" "examples\platform_smoke\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :build "render target" "examples\render_target\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :build "scene" "examples\scene\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :build "shader distortion" "examples\shader_distortion\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :build "shader material" "examples\shader_material\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :build "shape" "examples\shape\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :build "sprite batch" "examples\sprite_batch\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :build "text" "examples\text\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :build "texture" "examples\texture\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :build "text input" "examples\text_input\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :build "touch" "examples\touch\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :build "XUI bridge" "examples\xui_bridge\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :build "XUI incubation" "examples\xui_incubation\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :build "XUI layout validation" "examples\xui_layout_validation\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :build "XUI input validation" "examples\xui_input_validation\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :build "XUI TextEdit validation" "examples\xui_text_edit_validation\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :build "XUI widget tree style lab" "examples\xui_widget_tree_style_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :build "XUI layout gallery" "examples\xui_layout_gallery\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :build "XUI text edit lab" "examples\xui_text_edit_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :build "Game login lab" "examples\game_login_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%

echo.
echo [XGE] All Windows examples built successfully.
exit /b 0

:build_xui
echo [XGE] Building XUI validation examples...
call :build "XUI bridge" "examples\xui_bridge\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :build "XUI incubation" "examples\xui_incubation\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :build "XUI layout validation" "examples\xui_layout_validation\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :build "XUI input validation" "examples\xui_input_validation\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :build "XUI TextEdit validation" "examples\xui_text_edit_validation\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :build "XUI widget tree style lab" "examples\xui_widget_tree_style_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :build "XUI layout gallery" "examples\xui_layout_gallery\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :build "XUI focus capture event lab" "examples\xui_focus_capture_event_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :build "XUI paint host lab" "examples\xui_paint_host_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :build "XUI theme lab" "examples\xui_theme_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :build "XUI basic controls lab" "examples\xui_basic_controls_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :build "XUI choice controls lab" "examples\xui_choice_controls_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :build "XUI choice standard lab" "examples\xui_choice_standard_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :build "XUI value controls lab" "examples\xui_value_controls_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :build "XUI list scroll lab" "examples\xui_list_scroll_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :build "XUI list standard lab" "examples\xui_list_standard_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :build "XUI scroll standard lab" "examples\xui_scroll_standard_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :build "XUI virtual list standard lab" "examples\xui_virtual_list_standard_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :build "XUI overlay menu lab" "examples\xui_overlay_menu_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :build "XUI hyperlink lab" "examples\xui_hyperlink_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :build "XUI tabs dirty rect lab" "examples\xui_tabs_dirty_rect_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :build "XUI tabs standard lab" "examples\xui_tabs_standard_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :build "XUI text buffer lab" "examples\xui_text_buffer_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :build "XUI visual proc lab" "examples\xui_visual_proc_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :build "XUI control proc lab" "examples\xui_control_proc_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :build "XUI overlay proc lab" "examples\xui_overlay_proc_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :build "XUI list view proc lab" "examples\xui_list_view_proc_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :build "XUI scroll view proc lab" "examples\xui_scroll_view_proc_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :build "XUI input proc lab" "examples\xui_input_proc_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :build "XUI input standard lab" "examples\xui_input_standard_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :build "XUI input policy lab" "examples\xui_input_policy_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :build "XUI input IME policy lab" "examples\xui_input_ime_policy_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :build "XUI input nav policy lab" "examples\xui_input_nav_policy_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :build "XUI input context policy lab" "examples\xui_input_context_policy_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :build "XUI input blink focus lab" "examples\xui_input_blink_focus_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :build "XUI input pointer select lab" "examples\xui_input_pointer_select_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :build "XUI input replace policy lab" "examples\xui_input_replace_policy_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :build "XUI input delete policy lab" "examples\xui_input_delete_policy_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :build "XUI input clipboard policy lab" "examples\xui_input_clipboard_policy_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :build "XUI input password policy lab" "examples\xui_input_password_policy_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :build "XUI button state lab" "examples\xui_button_state_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :build "XUI button standard lab" "examples\xui_button_standard_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :build "XUI text select lab" "examples\xui_text_select_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :build "XUI text delete lab" "examples\xui_text_delete_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :build "XUI text wrap scroll lab" "examples\xui_text_wrap_scroll_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :build "XUI menu owner lab" "examples\xui_menu_owner_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :build "XUI popup owner lab" "examples\xui_popup_owner_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :build "XUI tooltip owner lab" "examples\xui_tooltip_owner_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :build "XUI dialog modal lab" "examples\xui_dialog_modal_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :build "XUI text undo stack lab" "examples\xui_text_undo_stack_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :build "XUI text edit standard lab" "examples\xui_text_edit_standard_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :build "XUI text history limit lab" "examples\xui_text_history_limit_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :build "XUI text menu detail lab" "examples\xui_text_menu_detail_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :build "XUI combo detail lab" "examples\xui_combo_detail_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :build "XUI combo policy lab" "examples\xui_combo_policy_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :build "XUI list view detail lab" "examples\xui_list_view_detail_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :build "XUI menu policy lab" "examples\xui_menu_policy_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :build "XUI popup policy lab" "examples\xui_popup_policy_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :build "XUI tooltip policy lab" "examples\xui_tooltip_policy_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :build "XUI dialog policy lab" "examples\xui_dialog_policy_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :build "XUI overlay policy lab" "examples\xui_overlay_policy_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :build "XUI split layout lab" "examples\xui_split_layout_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :build "XUI window lab" "examples\xui_window_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :build "XUI text edit lab" "examples\xui_text_edit_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :build "XUI standard controls lab" "examples\xui_standard_controls_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :build "XUI all controls gallery" "examples\xui_all_controls_gallery\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :build "XUI data controls lab" "examples\xui_data_controls_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :build "XUI numeric input lab" "examples\xui_numeric_input_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :build "XUI message box lab" "examples\xui_message_box_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :build "XUI toolbar lab" "examples\xui_toolbar_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :build "XUI status bar lab" "examples\xui_status_bar_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :build "XUI tree view lab" "examples\xui_tree_view_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :build "XUI table view lab" "examples\xui_table_view_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :build "XUI property grid lab" "examples\xui_property_grid_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :build "XUI breadcrumb lab" "examples\xui_breadcrumb_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :build "XUI accordion lab" "examples\xui_accordion_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :build "XUI search box lab" "examples\xui_search_box_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :build "XUI color picker lab" "examples\xui_color_picker_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :build "XUI date picker lab" "examples\xui_date_picker_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :build "XUI toast lab" "examples\xui_toast_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :build "XUI XSON page lab" "examples\xui_xson_page_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :build "XUI XSON style lab" "examples\xui_xson_style_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :build "XUI XSON layout gallery" "examples\xui_xson_layout_gallery_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :build "XUI XSON app layout lab" "examples\xui_xson_app_layout_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :build "XUI XSON virtual list lab" "examples\xui_xson_virtual_list_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :build "XUI XSON breadcrumb lab" "examples\xui_xson_breadcrumb_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :build "XUI XSON accordion lab" "examples\xui_xson_accordion_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :build "XUI XSON toast lab" "examples\xui_xson_toast_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :build "XUI XSON property grid lab" "examples\xui_xson_property_grid_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :build "XUI XSON table view lab" "examples\xui_xson_table_view_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :build "XUI XSON controls lab" "examples\xui_xson_controls_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :build "Game login lab" "examples\game_login_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%

echo.
echo [XGE] XUI validation examples built successfully.
exit /b 0

:build
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
