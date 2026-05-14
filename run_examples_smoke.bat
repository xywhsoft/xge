@echo off
setlocal

set ROOT=%~dp0

if "%~1"=="/?" goto help
if /i "%~1"=="--help" goto help
if /i "%~1"=="help" goto help

set XUI_ONLY=0
if /i "%~1"=="--xui" (
	set XUI_ONLY=1
	shift
)

set FRAMES=120
if not "%~1"=="" set FRAMES=%~1

if "%XUI_ONLY%"=="1" (
	call "%ROOT%build_examples_all.bat" --xui
	if errorlevel 1 exit /b 1
	goto run_xui_smoke
)

call "%ROOT%examples\core_lifecycle\build.bat"
if errorlevel 1 exit /b 1
call "%ROOT%examples\diagnostics_caps_log\build.bat"
if errorlevel 1 exit /b 1
call "%ROOT%examples\viewport_clip_camera\build.bat"
if errorlevel 1 exit /b 1
call "%ROOT%examples\image_texture_lab\build.bat"
if errorlevel 1 exit /b 1
call "%ROOT%examples\font_text_lab\build.bat"
if errorlevel 1 exit /b 1
call "%ROOT%examples\resource_provider_lab\build.bat"
if errorlevel 1 exit /b 1
call "%ROOT%examples\input_state_lab\build.bat"
if errorlevel 1 exit /b 1
call "%ROOT%examples\audio_lab\build.bat"
if errorlevel 1 exit /b 1
call "%ROOT%examples\scene_lifecycle_lab\build.bat"
if errorlevel 1 exit /b 1
call "%ROOT%examples\auto_pause_lab\build.bat"
if errorlevel 1 exit /b 1
call "%ROOT%examples\render_target_lab\build.bat"
if errorlevel 1 exit /b 1
call "%ROOT%examples\shader_variant_lab\build.bat"
if errorlevel 1 exit /b 1
call "%ROOT%examples\material_mesh_buffer_lab\build.bat"
if errorlevel 1 exit /b 1
call "%ROOT%examples\blend_depth_color_lab\build.bat"
if errorlevel 1 exit /b 1
call "%ROOT%examples\shape_full_gallery\build.bat"
if errorlevel 1 exit /b 1
call "%ROOT%examples\async_assets_lab\build.bat"
if errorlevel 1 exit /b 1
call "%ROOT%examples\platform_runtime_lab\build.bat"
if errorlevel 1 exit /b 1
call "%ROOT%examples\offscreen_egl_lab\build.bat"
if errorlevel 1 exit /b 1
call "%ROOT%examples\render_thread_lab\build.bat"
if errorlevel 1 exit /b 1
call "%ROOT%examples\miniprogram_bridge_lab\build.bat"
if errorlevel 1 exit /b 1
call "%ROOT%examples\xui_widget_tree_style_lab\build.bat"
if errorlevel 1 exit /b 1
call "%ROOT%examples\xui_layout_gallery\build.bat"
if errorlevel 1 exit /b 1
call "%ROOT%examples\xui_focus_capture_event_lab\build.bat"
if errorlevel 1 exit /b 1
call "%ROOT%examples\xui_paint_host_lab\build.bat"
if errorlevel 1 exit /b 1
call "%ROOT%examples\xui_theme_lab\build.bat"
if errorlevel 1 exit /b 1
call "%ROOT%examples\xui_basic_controls_lab\build.bat"
if errorlevel 1 exit /b 1
call "%ROOT%examples\xui_choice_controls_lab\build.bat"
if errorlevel 1 exit /b 1
call "%ROOT%examples\xui_choice_standard_lab\build.bat"
if errorlevel 1 exit /b 1
call "%ROOT%examples\xui_value_controls_lab\build.bat"
if errorlevel 1 exit /b 1
call "%ROOT%examples\xui_list_standard_lab\build.bat"
if errorlevel 1 exit /b 1
call "%ROOT%examples\xui_scroll_standard_lab\build.bat"
if errorlevel 1 exit /b 1
call "%ROOT%examples\xui_virtual_list_standard_lab\build.bat"
if errorlevel 1 exit /b 1
call "%ROOT%examples\xui_list_scroll_lab\build.bat"
if errorlevel 1 exit /b 1
call "%ROOT%examples\xui_overlay_menu_lab\build.bat"
if errorlevel 1 exit /b 1
call "%ROOT%examples\xui_hyperlink_lab\build.bat"
if errorlevel 1 exit /b 1
call "%ROOT%examples\xui_tabs_dirty_rect_lab\build.bat"
if errorlevel 1 exit /b 1
call "%ROOT%examples\xui_tabs_standard_lab\build.bat"
if errorlevel 1 exit /b 1
call "%ROOT%examples\xui_text_buffer_lab\build.bat"
if errorlevel 1 exit /b 1
call "%ROOT%examples\xui_visual_proc_lab\build.bat"
if errorlevel 1 exit /b 1
call "%ROOT%examples\xui_control_proc_lab\build.bat"
if errorlevel 1 exit /b 1
call "%ROOT%examples\xui_overlay_proc_lab\build.bat"
if errorlevel 1 exit /b 1
call "%ROOT%examples\xui_list_view_proc_lab\build.bat"
if errorlevel 1 exit /b 1
call "%ROOT%examples\xui_scroll_view_proc_lab\build.bat"
if errorlevel 1 exit /b 1
call "%ROOT%examples\xui_input_proc_lab\build.bat"
if errorlevel 1 exit /b 1
call "%ROOT%examples\xui_input_standard_lab\build.bat"
if errorlevel 1 exit /b 1
call "%ROOT%examples\xui_input_policy_lab\build.bat"
if errorlevel 1 exit /b 1
call "%ROOT%examples\xui_input_ime_policy_lab\build.bat"
if errorlevel 1 exit /b 1
call "%ROOT%examples\xui_input_nav_policy_lab\build.bat"
if errorlevel 1 exit /b 1
call "%ROOT%examples\xui_input_context_policy_lab\build.bat"
if errorlevel 1 exit /b 1
call "%ROOT%examples\xui_input_blink_focus_lab\build.bat"
if errorlevel 1 exit /b 1
call "%ROOT%examples\xui_input_pointer_select_lab\build.bat"
if errorlevel 1 exit /b 1
call "%ROOT%examples\xui_input_replace_policy_lab\build.bat"
if errorlevel 1 exit /b 1
call "%ROOT%examples\xui_input_delete_policy_lab\build.bat"
if errorlevel 1 exit /b 1
call "%ROOT%examples\xui_input_clipboard_policy_lab\build.bat"
if errorlevel 1 exit /b 1
call "%ROOT%examples\xui_input_password_policy_lab\build.bat"
if errorlevel 1 exit /b 1
call "%ROOT%examples\xui_button_state_lab\build.bat"
if errorlevel 1 exit /b 1
call "%ROOT%examples\xui_button_standard_lab\build.bat"
if errorlevel 1 exit /b 1
call "%ROOT%examples\xui_text_select_lab\build.bat"
if errorlevel 1 exit /b 1
call "%ROOT%examples\xui_text_delete_lab\build.bat"
if errorlevel 1 exit /b 1
call "%ROOT%examples\xui_text_wrap_scroll_lab\build.bat"
if errorlevel 1 exit /b 1
call "%ROOT%examples\xui_menu_owner_lab\build.bat"
if errorlevel 1 exit /b 1
call "%ROOT%examples\xui_popup_owner_lab\build.bat"
if errorlevel 1 exit /b 1
call "%ROOT%examples\xui_tooltip_owner_lab\build.bat"
if errorlevel 1 exit /b 1
call "%ROOT%examples\xui_dialog_modal_lab\build.bat"
if errorlevel 1 exit /b 1
call "%ROOT%examples\xui_text_undo_stack_lab\build.bat"
if errorlevel 1 exit /b 1
call "%ROOT%examples\xui_text_edit_standard_lab\build.bat"
if errorlevel 1 exit /b 1
call "%ROOT%examples\xui_text_history_limit_lab\build.bat"
if errorlevel 1 exit /b 1
call "%ROOT%examples\xui_text_menu_detail_lab\build.bat"
if errorlevel 1 exit /b 1
call "%ROOT%examples\xui_combo_detail_lab\build.bat"
if errorlevel 1 exit /b 1
call "%ROOT%examples\xui_combo_policy_lab\build.bat"
if errorlevel 1 exit /b 1
call "%ROOT%examples\xui_list_view_detail_lab\build.bat"
if errorlevel 1 exit /b 1
call "%ROOT%examples\xui_menu_policy_lab\build.bat"
if errorlevel 1 exit /b 1
call "%ROOT%examples\xui_popup_policy_lab\build.bat"
if errorlevel 1 exit /b 1
call "%ROOT%examples\xui_tooltip_policy_lab\build.bat"
if errorlevel 1 exit /b 1
call "%ROOT%examples\xui_dialog_policy_lab\build.bat"
if errorlevel 1 exit /b 1
call "%ROOT%examples\xui_overlay_policy_lab\build.bat"
if errorlevel 1 exit /b 1
call "%ROOT%examples\xui_split_layout_lab\build.bat"
if errorlevel 1 exit /b 1
call "%ROOT%examples\xui_window_lab\build.bat"
if errorlevel 1 exit /b 1
call "%ROOT%examples\xui_text_edit_lab\build.bat"
if errorlevel 1 exit /b 1
call "%ROOT%examples\xui_standard_controls_lab\build.bat"
if errorlevel 1 exit /b 1
call "%ROOT%examples\xui_all_controls_gallery\build.bat"
if errorlevel 1 exit /b 1
call "%ROOT%examples\xui_data_controls_lab\build.bat"
if errorlevel 1 exit /b 1
call "%ROOT%examples\xui_numericinput\build.bat"
if errorlevel 1 exit /b 1
call "%ROOT%examples\xui_numericinput_xson\build.bat"
if errorlevel 1 exit /b 1
call "%ROOT%examples\xui_combobox\build.bat"
if errorlevel 1 exit /b 1
call "%ROOT%examples\xui_combobox_xson\build.bat"
if errorlevel 1 exit /b 1
call "%ROOT%examples\xui_message_box_lab\build.bat"
if errorlevel 1 exit /b 1
call "%ROOT%examples\xui_toolbar_lab\build.bat"
if errorlevel 1 exit /b 1
call "%ROOT%examples\xui_status_bar_lab\build.bat"
if errorlevel 1 exit /b 1
call "%ROOT%examples\xui_tree_view_lab\build.bat"
if errorlevel 1 exit /b 1
call "%ROOT%examples\xui_table_view_lab\build.bat"
if errorlevel 1 exit /b 1
call "%ROOT%examples\xui_property_grid_lab\build.bat"
if errorlevel 1 exit /b 1
call "%ROOT%examples\xui_breadcrumb_lab\build.bat"
if errorlevel 1 exit /b 1
call "%ROOT%examples\xui_accordion_lab\build.bat"
if errorlevel 1 exit /b 1
if errorlevel 1 exit /b 1
call "%ROOT%examples\xui_color_picker_lab\build.bat"
if errorlevel 1 exit /b 1
call "%ROOT%examples\xui_toast_lab\build.bat"
if errorlevel 1 exit /b 1
call "%ROOT%examples\xui_label_xson\build.bat"
if errorlevel 1 exit /b 1
call "%ROOT%examples\xui_image\build.bat"
if errorlevel 1 exit /b 1
call "%ROOT%examples\xui_image_xson\build.bat"
if errorlevel 1 exit /b 1
call "%ROOT%examples\xui_separator\build.bat"
if errorlevel 1 exit /b 1
call "%ROOT%examples\xui_separator_xson\build.bat"
if errorlevel 1 exit /b 1
call "%ROOT%examples\xui_progress\build.bat"
if errorlevel 1 exit /b 1
call "%ROOT%examples\xui_progress_xson\build.bat"
if errorlevel 1 exit /b 1
call "%ROOT%examples\xui_xson_page_lab\build.bat"
if errorlevel 1 exit /b 1
call "%ROOT%examples\xui_xson_style_lab\build.bat"
if errorlevel 1 exit /b 1
call "%ROOT%examples\xui_xson_layout_gallery_lab\build.bat"
if errorlevel 1 exit /b 1
call "%ROOT%examples\xui_xson_app_layout_lab\build.bat"
if errorlevel 1 exit /b 1
call "%ROOT%examples\xui_xson_virtual_list_lab\build.bat"
if errorlevel 1 exit /b 1
call "%ROOT%examples\xui_xson_breadcrumb_lab\build.bat"
if errorlevel 1 exit /b 1
call "%ROOT%examples\xui_xson_accordion_lab\build.bat"
if errorlevel 1 exit /b 1
call "%ROOT%examples\xui_xson_toast_lab\build.bat"
if errorlevel 1 exit /b 1
call "%ROOT%examples\xui_xson_property_grid_lab\build.bat"
if errorlevel 1 exit /b 1
call "%ROOT%examples\xui_xson_table_view_lab\build.bat"
if errorlevel 1 exit /b 1
call "%ROOT%examples\xui_xson_controls_lab\build.bat"
if errorlevel 1 exit /b 1

echo [XGE] Running core lifecycle smoke...
"%ROOT%build\xge_core_lifecycle.exe" --frames %FRAMES%
if errorlevel 1 exit /b 1

echo [XGE] Running diagnostics caps/log smoke...
"%ROOT%build\xge_diagnostics_caps_log.exe" --frames %FRAMES%
if errorlevel 1 exit /b 1

echo [XGE] Running viewport clip camera smoke...
"%ROOT%build\xge_viewport_clip_camera.exe" --frames %FRAMES%
if errorlevel 1 exit /b 1

echo [XGE] Running image texture lab smoke...
"%ROOT%build\xge_image_texture_lab.exe" --frames %FRAMES%
if errorlevel 1 exit /b 1

echo [XGE] Running font text lab smoke...
"%ROOT%build\xge_font_text_lab.exe" --frames %FRAMES%
if errorlevel 1 exit /b 1

echo [XGE] Running resource provider lab smoke...
"%ROOT%build\xge_resource_provider_lab.exe" --frames %FRAMES%
if errorlevel 1 exit /b 1

echo [XGE] Running input state lab smoke...
"%ROOT%build\xge_input_state_lab.exe" --frames %FRAMES%
if errorlevel 1 exit /b 1

echo [XGE] Running audio lab smoke...
"%ROOT%build\xge_audio_lab.exe" --frames %FRAMES%
if errorlevel 1 exit /b 1

echo [XGE] Running scene lifecycle lab smoke...
"%ROOT%build\xge_scene_lifecycle_lab.exe" --frames %FRAMES%
if errorlevel 1 exit /b 1

echo [XGE] Running auto pause lab smoke...
"%ROOT%build\xge_auto_pause_lab.exe" --frames %FRAMES%
if errorlevel 1 exit /b 1

echo [XGE] Running render target lab smoke...
"%ROOT%build\xge_render_target_lab.exe" --frames %FRAMES%
if errorlevel 1 exit /b 1

echo [XGE] Running shader variant lab smoke...
"%ROOT%build\xge_shader_variant_lab.exe" --frames %FRAMES%
if errorlevel 1 exit /b 1

echo [XGE] Running material mesh buffer lab smoke...
"%ROOT%build\xge_material_mesh_buffer_lab.exe" --frames %FRAMES%
if errorlevel 1 exit /b 1

echo [XGE] Running blend depth color lab smoke...
"%ROOT%build\xge_blend_depth_color_lab.exe" --frames %FRAMES%
if errorlevel 1 exit /b 1

echo [XGE] Running shape full gallery smoke...
"%ROOT%build\xge_shape_full_gallery.exe" --frames %FRAMES%
if errorlevel 1 exit /b 1

echo [XGE] Running async assets lab smoke...
"%ROOT%build\xge_async_assets_lab.exe" --frames %FRAMES%
if errorlevel 1 exit /b 1

echo [XGE] Running platform runtime lab smoke...
"%ROOT%build\xge_platform_runtime_lab.exe" --frames %FRAMES%
if errorlevel 1 exit /b 1

echo [XGE] Running offscreen EGL lab smoke...
"%ROOT%build\xge_offscreen_egl_lab.exe" --frames %FRAMES%
if errorlevel 1 exit /b 1

echo [XGE] Running render thread lab smoke...
"%ROOT%build\xge_render_thread_lab.exe" --frames %FRAMES%
if errorlevel 1 exit /b 1

echo [XGE] Running miniprogram bridge lab smoke...
"%ROOT%build\xge_miniprogram_bridge_lab.exe" --frames %FRAMES%
if errorlevel 1 exit /b 1

:run_xui_smoke

echo [XGE] Running XUI widget tree style lab smoke...
"%ROOT%build\xge_xui_widget_tree_style_lab.exe" --frames %FRAMES%
if errorlevel 1 exit /b 1

echo [XGE] Running XUI layout gallery smoke...
"%ROOT%build\xge_xui_layout_gallery.exe" --frames %FRAMES%
if errorlevel 1 exit /b 1

echo [XGE] Running XUI focus capture event lab smoke...
"%ROOT%build\xge_xui_focus_capture_event_lab.exe" --frames %FRAMES%
if errorlevel 1 exit /b 1

echo [XGE] Running XUI paint host lab smoke...
"%ROOT%build\xge_xui_paint_host_lab.exe" --frames %FRAMES%
if errorlevel 1 exit /b 1

echo [XGE] Running XUI theme lab smoke...
"%ROOT%build\xge_xui_theme_lab.exe" --frames %FRAMES%
if errorlevel 1 exit /b 1

echo [XGE] Running XUI basic controls lab smoke...
"%ROOT%build\xge_xui_basic_controls_lab.exe" --frames %FRAMES%
if errorlevel 1 exit /b 1

echo [XGE] Running XUI choice controls lab smoke...
"%ROOT%build\xge_xui_choice_controls_lab.exe" --frames %FRAMES%
if errorlevel 1 exit /b 1

echo [XGE] Running XUI choice standard lab smoke...
"%ROOT%build\xge_xui_choice_standard_lab.exe"
if errorlevel 1 exit /b 1

echo [XGE] Running XUI value controls lab smoke...
"%ROOT%build\xge_xui_value_controls_lab.exe" --frames %FRAMES%
if errorlevel 1 exit /b 1

echo [XGE] Running XUI list scroll lab smoke...
"%ROOT%build\xge_xui_list_scroll_lab.exe" --frames %FRAMES%
if errorlevel 1 exit /b 1

echo [XGE] Running XUI list standard lab smoke...
"%ROOT%build\xge_xui_list_standard_lab.exe"
if errorlevel 1 exit /b 1

echo [XGE] Running XUI scroll standard lab smoke...
"%ROOT%build\xge_xui_scroll_standard_lab.exe"
if errorlevel 1 exit /b 1

echo [XGE] Running XUI virtual list standard lab smoke...
"%ROOT%build\xge_xui_virtual_list_standard_lab.exe"
if errorlevel 1 exit /b 1

echo [XGE] Running XUI overlay menu lab smoke...
"%ROOT%build\xge_xui_overlay_menu_lab.exe" --frames %FRAMES%
if errorlevel 1 exit /b 1

echo [XGE] Running XUI hyperlink lab smoke...
"%ROOT%build\xge_xui_hyperlink_lab.exe" --frames %FRAMES%
if errorlevel 1 exit /b 1

echo [XGE] Running XUI tabs dirty rect lab smoke...
"%ROOT%build\xge_xui_tabs_dirty_rect_lab.exe" --frames %FRAMES%
if errorlevel 1 exit /b 1

echo [XGE] Running XUI tabs standard lab smoke...
"%ROOT%build\xge_xui_tabs_standard_lab.exe"
if errorlevel 1 exit /b 1

echo [XGE] Running XUI text buffer lab smoke...
"%ROOT%build\xge_xui_text_buffer_lab.exe" --frames %FRAMES%
if errorlevel 1 exit /b 1

echo [XGE] Running XUI visual proc lab smoke...
"%ROOT%build\xge_xui_visual_proc_lab.exe" --frames %FRAMES%
if errorlevel 1 exit /b 1

echo [XGE] Running XUI control proc lab smoke...
"%ROOT%build\xge_xui_control_proc_lab.exe" --frames %FRAMES%
if errorlevel 1 exit /b 1

echo [XGE] Running XUI overlay proc lab smoke...
"%ROOT%build\xge_xui_overlay_proc_lab.exe" --frames %FRAMES%
if errorlevel 1 exit /b 1

echo [XGE] Running XUI list view proc lab smoke...
"%ROOT%build\xge_xui_list_view_proc_lab.exe" --frames %FRAMES%
if errorlevel 1 exit /b 1

echo [XGE] Running XUI scroll view proc lab smoke...
"%ROOT%build\xge_xui_scroll_view_proc_lab.exe" --frames %FRAMES%
if errorlevel 1 exit /b 1

echo [XGE] Running XUI input proc lab smoke...
"%ROOT%build\xge_xui_input_proc_lab.exe" --frames %FRAMES%
if errorlevel 1 exit /b 1

echo [XGE] Running XUI input standard lab smoke...
"%ROOT%build\xge_xui_input_standard_lab.exe"
if errorlevel 1 exit /b 1

echo [XGE] Running XUI input policy lab smoke...
"%ROOT%build\xge_xui_input_policy_lab.exe" --frames %FRAMES%
if errorlevel 1 exit /b 1

echo [XGE] Running XUI input IME policy lab smoke...
"%ROOT%build\xge_xui_input_ime_policy_lab.exe" --frames %FRAMES%
if errorlevel 1 exit /b 1

echo [XGE] Running XUI input nav policy lab smoke...
"%ROOT%build\xge_xui_input_nav_policy_lab.exe" --frames %FRAMES%
if errorlevel 1 exit /b 1

echo [XGE] Running XUI input context policy lab smoke...
"%ROOT%build\xge_xui_input_context_policy_lab.exe" --frames %FRAMES%
if errorlevel 1 exit /b 1

echo [XGE] Running XUI input blink focus lab smoke...
"%ROOT%build\xge_xui_input_blink_focus_lab.exe" --frames %FRAMES%
if errorlevel 1 exit /b 1

echo [XGE] Running XUI input pointer select lab smoke...
"%ROOT%build\xge_xui_input_pointer_select_lab.exe" --frames %FRAMES%
if errorlevel 1 exit /b 1

echo [XGE] Running XUI input replace policy lab smoke...
"%ROOT%build\xge_xui_input_replace_policy_lab.exe" --frames %FRAMES%
if errorlevel 1 exit /b 1

echo [XGE] Running XUI input delete policy lab smoke...
"%ROOT%build\xge_xui_input_delete_policy_lab.exe" --frames %FRAMES%
if errorlevel 1 exit /b 1

echo [XGE] Running XUI input clipboard policy lab smoke...
"%ROOT%build\xge_xui_input_clipboard_policy_lab.exe" --frames %FRAMES%
if errorlevel 1 exit /b 1

echo [XGE] Running XUI input password policy lab smoke...
"%ROOT%build\xge_xui_input_password_policy_lab.exe" --frames %FRAMES%
if errorlevel 1 exit /b 1

echo [XGE] Running XUI button state lab smoke...
"%ROOT%build\xge_xui_button_state_lab.exe" --frames %FRAMES%
if errorlevel 1 exit /b 1

echo [XGE] Running XUI button standard lab smoke...
"%ROOT%build\xge_xui_button_standard_lab.exe"
if errorlevel 1 exit /b 1

echo [XGE] Running XUI text select lab smoke...
"%ROOT%build\xge_xui_text_select_lab.exe" --frames %FRAMES%
if errorlevel 1 exit /b 1

echo [XGE] Running XUI text delete lab smoke...
"%ROOT%build\xge_xui_text_delete_lab.exe" --frames %FRAMES%
if errorlevel 1 exit /b 1

echo [XGE] Running XUI text wrap scroll lab smoke...
"%ROOT%build\xge_xui_text_wrap_scroll_lab.exe" --frames %FRAMES%
if errorlevel 1 exit /b 1

echo [XGE] Running XUI text edit standard lab smoke...
"%ROOT%build\xge_xui_text_edit_standard_lab.exe"
if errorlevel 1 exit /b 1

echo [XGE] Running XUI standard controls lab smoke...
"%ROOT%build\xge_xui_standard_controls_lab.exe" --frames %FRAMES%
if errorlevel 1 exit /b 1

echo [XGE] Running XUI all controls gallery smoke...
"%ROOT%build\xge_xui_all_controls_gallery.exe" --frames %FRAMES%
if errorlevel 1 exit /b 1

echo [XGE] Running XUI data controls lab smoke...
"%ROOT%build\xge_xui_data_controls_lab.exe" --frames %FRAMES%
if errorlevel 1 exit /b 1

echo [XGE] Running XUI NumericInput smoke...
"%ROOT%build\xui_numericinput.exe" --frames %FRAMES%
if errorlevel 1 exit /b 1

echo [XGE] Running XUI NumericInput XSON smoke...
"%ROOT%build\xui_numericinput_xson.exe" --frames %FRAMES%
if errorlevel 1 exit /b 1

echo [XGE] Running XUI ComboBox smoke...
"%ROOT%build\xui_combobox.exe" --frames %FRAMES%
if errorlevel 1 exit /b 1

echo [XGE] Running XUI ComboBox XSON smoke...
"%ROOT%build\xui_combobox_xson.exe" --frames %FRAMES%
if errorlevel 1 exit /b 1

echo [XGE] Running XUI message box lab smoke...
"%ROOT%build\xge_xui_message_box_lab.exe" --frames %FRAMES%
if errorlevel 1 exit /b 1

echo [XGE] Running XUI toolbar lab smoke...
"%ROOT%build\xge_xui_toolbar_lab.exe" --frames %FRAMES%
if errorlevel 1 exit /b 1

echo [XGE] Running XUI status bar lab smoke...
"%ROOT%build\xge_xui_status_bar_lab.exe" --frames %FRAMES%
if errorlevel 1 exit /b 1

echo [XGE] Running XUI tree view lab smoke...
"%ROOT%build\xge_xui_tree_view_lab.exe" --frames %FRAMES%
if errorlevel 1 exit /b 1

echo [XGE] Running XUI table view lab smoke...
"%ROOT%build\xge_xui_table_view_lab.exe" --frames %FRAMES%
if errorlevel 1 exit /b 1

echo [XGE] Running XUI property grid lab smoke...
"%ROOT%build\xge_xui_property_grid_lab.exe" --frames %FRAMES%
if errorlevel 1 exit /b 1

echo [XGE] Running XUI breadcrumb lab smoke...
"%ROOT%build\xge_xui_breadcrumb_lab.exe" --frames %FRAMES%
if errorlevel 1 exit /b 1

echo [XGE] Running XUI accordion lab smoke...
"%ROOT%build\xge_xui_accordion_lab.exe" --frames %FRAMES%
if errorlevel 1 exit /b 1

echo [XGE] Running XUI search box lab smoke...
if errorlevel 1 exit /b 1

echo [XGE] Running XUI color picker lab smoke...
"%ROOT%build\xge_xui_color_picker_lab.exe" --frames %FRAMES%
if errorlevel 1 exit /b 1

echo [XGE] Running XUI toast lab smoke...
"%ROOT%build\xge_xui_toast_lab.exe" --frames %FRAMES%
if errorlevel 1 exit /b 1

echo [XGE] Running XUI label XSON smoke...
"%ROOT%build\xui_label_xson.exe" --frames %FRAMES%
if errorlevel 1 exit /b 1

echo [XGE] Running XUI image smoke...
"%ROOT%build\xui_image.exe" --frames %FRAMES%
if errorlevel 1 exit /b 1

echo [XGE] Running XUI image XSON smoke...
"%ROOT%build\xui_image_xson.exe" --frames %FRAMES%
if errorlevel 1 exit /b 1

echo [XGE] Running XUI separator smoke...
"%ROOT%build\xui_separator.exe" --frames %FRAMES%
if errorlevel 1 exit /b 1

echo [XGE] Running XUI separator XSON smoke...
"%ROOT%build\xui_separator_xson.exe" --frames %FRAMES%
if errorlevel 1 exit /b 1

echo [XGE] Running XUI progress smoke...
"%ROOT%build\xui_progress.exe" --frames %FRAMES%
if errorlevel 1 exit /b 1

echo [XGE] Running XUI progress XSON smoke...
"%ROOT%build\xui_progress_xson.exe" --frames %FRAMES%
if errorlevel 1 exit /b 1

echo [XGE] Running XUI XSON page lab smoke...
"%ROOT%build\xge_xui_xson_page_lab.exe" --frames %FRAMES%
if errorlevel 1 exit /b 1

echo [XGE] Running XUI XSON style lab smoke...
"%ROOT%build\xge_xui_xson_style_lab.exe" --frames %FRAMES%
if errorlevel 1 exit /b 1

echo [XGE] Running XUI XSON layout gallery lab smoke...
"%ROOT%build\xge_xui_xson_layout_gallery_lab.exe" --frames %FRAMES%
if errorlevel 1 exit /b 1

echo [XGE] Running XUI XSON app layout lab smoke...
"%ROOT%build\xge_xui_xson_app_layout_lab.exe" --frames %FRAMES%
if errorlevel 1 exit /b 1

echo [XGE] Running XUI XSON virtual list lab smoke...
"%ROOT%build\xge_xui_xson_virtual_list_lab.exe" --frames %FRAMES%
if errorlevel 1 exit /b 1

echo [XGE] Running XUI XSON breadcrumb lab smoke...
"%ROOT%build\xge_xui_xson_breadcrumb_lab.exe" --frames %FRAMES%
if errorlevel 1 exit /b 1

echo [XGE] Running XUI XSON accordion lab smoke...
"%ROOT%build\xge_xui_xson_accordion_lab.exe" --frames %FRAMES%
if errorlevel 1 exit /b 1

echo [XGE] Running XUI XSON toast lab smoke...
"%ROOT%build\xge_xui_xson_toast_lab.exe" --frames %FRAMES%
if errorlevel 1 exit /b 1

echo [XGE] Running XUI XSON property grid lab smoke...
"%ROOT%build\xge_xui_xson_property_grid_lab.exe" --frames %FRAMES%
if errorlevel 1 exit /b 1

echo [XGE] Running XUI XSON table view lab smoke...
"%ROOT%build\xge_xui_xson_table_view_lab.exe" --frames %FRAMES%
if errorlevel 1 exit /b 1

echo [XGE] Running XUI XSON controls lab smoke...
"%ROOT%build\xge_xui_xson_controls_lab.exe"
if errorlevel 1 exit /b 1

echo [XGE] Running XUI menu owner lab smoke...
"%ROOT%build\xge_xui_menu_owner_lab.exe" --frames %FRAMES%
if errorlevel 1 exit /b 1

echo [XGE] Running XUI popup owner lab smoke...
"%ROOT%build\xge_xui_popup_owner_lab.exe" --frames %FRAMES%
if errorlevel 1 exit /b 1

echo [XGE] Running XUI tooltip owner lab smoke...
"%ROOT%build\xge_xui_tooltip_owner_lab.exe" --frames %FRAMES%
if errorlevel 1 exit /b 1

echo [XGE] Running XUI dialog modal lab smoke...
"%ROOT%build\xge_xui_dialog_modal_lab.exe" --frames %FRAMES%
if errorlevel 1 exit /b 1

echo [XGE] Running XUI text undo stack lab smoke...
"%ROOT%build\xge_xui_text_undo_stack_lab.exe" --frames %FRAMES%
if errorlevel 1 exit /b 1

echo [XGE] Running XUI text history limit lab smoke...
"%ROOT%build\xge_xui_text_history_limit_lab.exe" --frames %FRAMES%
if errorlevel 1 exit /b 1

echo [XGE] Running XUI text menu detail lab smoke...
"%ROOT%build\xge_xui_text_menu_detail_lab.exe" --frames %FRAMES%
if errorlevel 1 exit /b 1

echo [XGE] Running XUI combo detail lab smoke...
"%ROOT%build\xge_xui_combo_detail_lab.exe" --frames %FRAMES%
if errorlevel 1 exit /b 1

echo [XGE] Running XUI combo policy lab smoke...
"%ROOT%build\xge_xui_combo_policy_lab.exe" --frames %FRAMES%
if errorlevel 1 exit /b 1

echo [XGE] Running XUI list view detail lab smoke...
"%ROOT%build\xge_xui_list_view_detail_lab.exe" --frames %FRAMES%
if errorlevel 1 exit /b 1

echo [XGE] Running XUI menu policy lab smoke...
"%ROOT%build\xge_xui_menu_policy_lab.exe" --frames %FRAMES%
if errorlevel 1 exit /b 1

echo [XGE] Running XUI popup policy lab smoke...
"%ROOT%build\xge_xui_popup_policy_lab.exe" --frames %FRAMES%
if errorlevel 1 exit /b 1

echo [XGE] Running XUI tooltip policy lab smoke...
"%ROOT%build\xge_xui_tooltip_policy_lab.exe" --frames %FRAMES%
if errorlevel 1 exit /b 1

echo [XGE] Running XUI dialog policy lab smoke...
"%ROOT%build\xge_xui_dialog_policy_lab.exe" --frames %FRAMES%
if errorlevel 1 exit /b 1

echo [XGE] Running XUI overlay policy lab smoke...
"%ROOT%build\xge_xui_overlay_policy_lab.exe"
if errorlevel 1 exit /b 1

echo [XGE] Running XUI split layout lab smoke...
"%ROOT%build\xge_xui_split_layout_lab.exe" --frames %FRAMES%
if errorlevel 1 exit /b 1

echo [XGE] Running XUI window lab smoke...
"%ROOT%build\xge_xui_window_lab.exe" --frames %FRAMES%
if errorlevel 1 exit /b 1

echo [XGE] Running XUI text edit lab smoke...
"%ROOT%build\xge_xui_text_edit_lab.exe" --frames %FRAMES%
if errorlevel 1 exit /b 1

echo [XGE] Running game login lab smoke...
"%ROOT%build\xge_game_login_lab.exe" --frames %FRAMES%
if errorlevel 1 exit /b 1

echo [XGE] Example smoke checks finished.
exit /b 0

:help
echo [XGE] Usage: run_examples_smoke.bat [--xui] [frames]
echo [XGE] Builds and runs examples that support automatic exit.
echo [XGE]   --xui  Build and run only XUI smoke examples.
exit /b 0
