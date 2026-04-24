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
call :build "XUI value controls lab" "examples\xui_value_controls_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :build "XUI list scroll lab" "examples\xui_list_scroll_lab\build.bat"
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
call :build "XUI value controls lab" "examples\xui_value_controls_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :build "XUI list scroll lab" "examples\xui_list_scroll_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :build "XUI overlay menu lab" "examples\xui_overlay_menu_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :build "XUI hyperlink lab" "examples\xui_hyperlink_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :build "XUI tabs dirty rect lab" "examples\xui_tabs_dirty_rect_lab\build.bat"
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
call :build "XUI text edit lab" "examples\xui_text_edit_lab\build.bat"
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
