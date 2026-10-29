@echo off
setlocal
cd /d "%~dp0\..\.."
call tests\xui\build_unicode_grapheme_test.bat
if errorlevel 1 exit /b 1
call tests\xui\build_unicode_word_corpus_test.bat
if errorlevel 1 exit /b 1
call tests\xui\build_unicode_script_test.bat
if errorlevel 1 exit /b 1
if "%~1"=="" (
    call tests\xui\build_document_test.bat
) else (
    call tests\xui\build_document_corpus_test.bat "%~1"
)
if %errorlevel% neq 0 exit /b 1
call tests\xui\build_document_quote_nested_test.bat
if errorlevel 1 exit /b 1
call tests\xui\build_document_quote_source_test.bat
if errorlevel 1 exit /b 1
call tests\xui\build_document_quote_prefix_test.bat
if errorlevel 1 exit /b 1
call tests\xui\build_document_footnote_prefix_test.bat
if errorlevel 1 exit /b 1
call tests\xui\build_document_tab_prefix_test.bat
if errorlevel 1 exit /b 1
call tests\xui\build_document_source_sharing_test.bat
if errorlevel 1 exit /b 1
call tests\xui\build_document_reference_sharing_test.bat
if errorlevel 1 exit /b 1
call tests\xui\build_document_source_compaction_test.bat
if errorlevel 1 exit /b 1
call tests\xui\build_document_definition_alignment_test.bat
if errorlevel 1 exit /b 1
call tests\xui\build_document_dependency_test.bat
if errorlevel 1 exit /b 1
call tests\xui\build_document_break_origin_test.bat
if errorlevel 1 exit /b 1
call tests\xui\build_document_front_matter_lines_test.bat
if errorlevel 1 exit /b 1
call tests\xui\build_document_object_break_origin_test.bat
if errorlevel 1 exit /b 1
call tests\xui\build_document_text_origin_test.bat
if errorlevel 1 exit /b 1
call tests\xui\build_document_fence_boundary_test.bat
if errorlevel 1 exit /b 1
call tests\xui\build_document_inline_whitespace_test.bat
if errorlevel 1 exit /b 1
call tests\xui\build_document_nested_image_test.bat
if errorlevel 1 exit /b 1
call tests\xui\build_document_image_field_source_test.bat
if errorlevel 1 exit /b 1
call tests\xui\build_document_link_field_cst_test.bat
if errorlevel 1 exit /b 1
call tests\xui\build_document_link_command_fields_test.bat
if errorlevel 1 exit /b 1
call tests\xui\build_document_link_owner_fields_test.bat
if errorlevel 1 exit /b 1
call tests\xui\build_document_unlink_fields_test.bat
if errorlevel 1 exit /b 1
call tests\xui\build_document_source_index_test.bat
if errorlevel 1 exit /b 1
call tests\xui\build_document_cst_test.bat
if errorlevel 1 exit /b 1
call tests\xui\build_document_prefix_binding_test.bat
if errorlevel 1 exit /b 1
call tests\xui\build_document_source_working_test.bat
if errorlevel 1 exit /b 1
call tests\xui\build_document_quote_unwrap_source_test.bat
if errorlevel 1 exit /b 1
call tests\xui\build_document_quote_boundary_test.bat
if errorlevel 1 exit /b 1
call tests\xui\build_document_admonition_source_test.bat
if errorlevel 1 exit /b 1
call tests\xui\build_document_footnote_unwrap_source_test.bat
if errorlevel 1 exit /b 1
call tests\xui\build_document_unlist_source_test.bat
if errorlevel 1 exit /b 1
call tests\xui\build_document_attribute_pool_scale_test.bat
if %errorlevel% neq 0 exit /b 1
call tests\xui\build_document_footnote_perf_test.bat
if %errorlevel% neq 0 exit /b 1
call tests\xui\build_document_dll_test.bat
if %errorlevel% neq 0 exit /b 1
call tests\xge\build_dll_platform_editor_exports_test.bat
if %errorlevel% neq 0 exit /b 1
powershell.exe -NoProfile -ExecutionPolicy Bypass -File tests\xui\check_document_release.ps1
if %errorlevel% neq 0 exit /b 1
call tests\xui\build_document_html_interaction_disabled_test.bat
if %errorlevel% neq 0 exit /b 1
call tests\xui\build_document_renderer_test.bat %1
if %errorlevel% neq 0 exit /b 1
call tests\xui\build_document_shape_context_test.bat
if errorlevel 1 exit /b 1
call tests\xui\build_text_break_index_test.bat
if errorlevel 1 exit /b 1
call tests\xui\build_document_line17_test.bat
if errorlevel 1 exit /b 1
call tests\xge\build_opentype_shaping_test.bat
if %errorlevel% neq 0 exit /b 1
call tests\xge\build_font_fallback_shaping_test.bat
if %errorlevel% neq 0 exit /b 1
call tests\xge\build_script_shaping_test.bat
if errorlevel 1 exit /b 1
call tests\xge\build_context_shaping_test.bat
if errorlevel 1 exit /b 1
call tests\xge\build_joining_context_test.bat
if errorlevel 1 exit /b 1
call tests\xui\build_text_item_test.bat
if errorlevel 1 exit /b 1
call tests\xui\build_text_bidi_test.bat
if %errorlevel% neq 0 exit /b 1
call tests\xge\build_rtl_shaping_test.bat
if %errorlevel% neq 0 exit /b 1
call tests\xui\build_document_opentype_test.bat
if %errorlevel% neq 0 exit /b 1
call tests\xui\build_document_bidi_test.bat
if %errorlevel% neq 0 exit /b 1
call tests\xui\build_document_grapheme17_test.bat
if errorlevel 1 exit /b 1
call tests\xui\build_document_script_test.bat
if errorlevel 1 exit /b 1
call tests\xui\build_document_context_test.bat
if errorlevel 1 exit /b 1
call tests\xui\build_document_language_test.bat
if errorlevel 1 exit /b 1
call tests\xui\build_document_input_caps_test.bat
if errorlevel 1 exit /b 1
call tests\xui\build_document_ascii_context_test.bat
if errorlevel 1 exit /b 1
call tests\xui\build_document_ascii_script_test.bat
if errorlevel 1 exit /b 1
call tests\xui\build_document_word_context_test.bat
if errorlevel 1 exit /b 1
call tests\xui\build_document_shared_span_test.bat
if errorlevel 1 exit /b 1
call tests\xui\build_document_line_paint_test.bat
if errorlevel 1 exit /b 1
call tests\xui\build_document_unicode_span_test.bat
if errorlevel 1 exit /b 1
call tests\xui\build_document_rtl_span_test.bat
if errorlevel 1 exit /b 1
call tests\xui\build_document_projected_span_test.bat
if errorlevel 1 exit /b 1
call tests\xui\build_document_shy_span_test.bat
if %errorlevel% neq 0 exit /b 1
call tests\xui\build_document_contour_points_test.bat
if %errorlevel% neq 0 exit /b 1
call tests\xui\build_document_style_test.bat
if %errorlevel% neq 0 exit /b 1
call tests\xui\build_document_scale_test.bat
if %errorlevel% neq 0 exit /b 1
call tests\xui\build_document_fractional_test.bat
if %errorlevel% neq 0 exit /b 1
call tests\xui\build_document_table_geometry_test.bat
if %errorlevel% neq 0 exit /b 1
call tests\xui\build_document_editor_test.bat
if %errorlevel% neq 0 exit /b 1
call tests\xui\build_document_accessible_text_test.bat
if errorlevel 1 exit /b 1
call tests\xui\build_document_visual_prepared_text_test.bat
if errorlevel 1 exit /b 1
call tests\xui\build_native_accessibility_test.bat
if %errorlevel% neq 0 exit /b 1
call tests\xui\build_native_accessibility_xge_test.bat
if errorlevel 1 exit /b 1
call tests\xui\build_document_join_editor_test.bat
if errorlevel 1 exit /b 1
call tests\xge\build_clipboard_win32_test.bat
if %errorlevel% neq 0 exit /b 1
call tests\xui\build_document_dib_paste_test.bat
if %errorlevel% neq 0 exit /b 1
call tests\xui\build_document_object_source_test.bat
if %errorlevel% neq 0 exit /b 1
call tests\xui\build_message_document_test.bat
if %errorlevel% neq 0 exit /b 1
call examples\xui\xui_document\build.bat
if %errorlevel% neq 0 exit /b 1
build\xui_document.exe --verify
if %errorlevel% neq 0 exit /b 1
build\xui_document.exe --verify-async
if %errorlevel% neq 0 exit /b 1
build\xui_document.exe --verify-image-placeholder
if %errorlevel% neq 0 exit /b 1
rem The permanent move tests use fresh output directories on every suite run.
if not defined XUI_DOCUMENT_MOVE_OUT_DIR set "XUI_DOCUMENT_MOVE_OUT_DIR=build\document\moves-%RANDOM%-%RANDOM%"
python tests\xui\build_document_move_test.py --variant normal --out-dir "%XUI_DOCUMENT_MOVE_OUT_DIR%\block-normal"
if errorlevel 1 exit /b 1
python tests\xui\build_document_move_test.py --variant collision --out-dir "%XUI_DOCUMENT_MOVE_OUT_DIR%\block-collision"
if errorlevel 1 exit /b 1
python tests\xui\build_document_container_move_test.py --variant normal --out-dir "%XUI_DOCUMENT_MOVE_OUT_DIR%\container-normal"
if errorlevel 1 exit /b 1
python tests\xui\build_document_container_move_test.py --variant collision --out-dir "%XUI_DOCUMENT_MOVE_OUT_DIR%\container-collision"
if errorlevel 1 exit /b 1
python tests\xui\build_document_editor_suite.py --build-dir build --out-dir "%XUI_DOCUMENT_MOVE_OUT_DIR%\editor-suite"
if errorlevel 1 exit /b 1

echo Document core, DLL, renderer, editor and native rendering checks passed.
if "%~1"=="" echo CommonMark corpus was not supplied; its check was skipped.
exit /b 0
