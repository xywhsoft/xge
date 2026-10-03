@echo off
setlocal
cd /d "%~dp0\.."
call test_xui\build_unicode_grapheme_test.bat
if errorlevel 1 exit /b 1
call test_xui\build_unicode_script_test.bat
if errorlevel 1 exit /b 1
if "%~1"=="" (
    call test_xui\build_document_test.bat
) else (
    call test_xui\build_document_corpus_test.bat "%~1"
)
if %errorlevel% neq 0 exit /b 1
call test_xui\build_document_quote_nested_test.bat
if errorlevel 1 exit /b 1
call test_xui\build_document_quote_source_test.bat
if errorlevel 1 exit /b 1
call test_xui\build_document_quote_prefix_test.bat
if errorlevel 1 exit /b 1
call test_xui\build_document_attribute_pool_scale_test.bat
if %errorlevel% neq 0 exit /b 1
call test_xui\build_document_footnote_perf_test.bat
if %errorlevel% neq 0 exit /b 1
call test_xui\build_document_dll_test.bat
if %errorlevel% neq 0 exit /b 1
call test\build_dll_platform_editor_exports_test.bat
if %errorlevel% neq 0 exit /b 1
powershell.exe -NoProfile -ExecutionPolicy Bypass -File test_xui\check_document_release.ps1
if %errorlevel% neq 0 exit /b 1
call test_xui\build_document_html_interaction_disabled_test.bat
if %errorlevel% neq 0 exit /b 1
call test_xui\build_document_renderer_test.bat %1
if %errorlevel% neq 0 exit /b 1
call test_xui\build_document_shape_context_test.bat
if errorlevel 1 exit /b 1
call test_xui\build_text_break_index_test.bat
if errorlevel 1 exit /b 1
call test_xui\build_document_line17_test.bat
if errorlevel 1 exit /b 1
call test\build_opentype_shaping_test.bat
if %errorlevel% neq 0 exit /b 1
call test\build_font_fallback_shaping_test.bat
if %errorlevel% neq 0 exit /b 1
call test\build_script_shaping_test.bat
if errorlevel 1 exit /b 1
call test\build_context_shaping_test.bat
if errorlevel 1 exit /b 1
call test\build_joining_context_test.bat
if errorlevel 1 exit /b 1
call test_xui\build_text_item_test.bat
if errorlevel 1 exit /b 1
call test_xui\build_text_bidi_test.bat
if %errorlevel% neq 0 exit /b 1
call test\build_rtl_shaping_test.bat
if %errorlevel% neq 0 exit /b 1
call test_xui\build_document_opentype_test.bat
if %errorlevel% neq 0 exit /b 1
call test_xui\build_document_bidi_test.bat
if %errorlevel% neq 0 exit /b 1
call test_xui\build_document_grapheme17_test.bat
if errorlevel 1 exit /b 1
call test_xui\build_document_script_test.bat
if errorlevel 1 exit /b 1
call test_xui\build_document_context_test.bat
if errorlevel 1 exit /b 1
call test_xui\build_document_language_test.bat
if errorlevel 1 exit /b 1
call test_xui\build_document_input_caps_test.bat
if errorlevel 1 exit /b 1
call test_xui\build_document_ascii_context_test.bat
if errorlevel 1 exit /b 1
call test_xui\build_document_ascii_script_test.bat
if errorlevel 1 exit /b 1
call test_xui\build_document_word_context_test.bat
if errorlevel 1 exit /b 1
call test_xui\build_document_shared_span_test.bat
if errorlevel 1 exit /b 1
call test_xui\build_document_line_paint_test.bat
if errorlevel 1 exit /b 1
call test_xui\build_document_unicode_span_test.bat
if errorlevel 1 exit /b 1
call test_xui\build_document_rtl_span_test.bat
if errorlevel 1 exit /b 1
call test_xui\build_document_projected_span_test.bat
if errorlevel 1 exit /b 1
call test_xui\build_document_shy_span_test.bat
if %errorlevel% neq 0 exit /b 1
call test_xui\build_document_contour_points_test.bat
if %errorlevel% neq 0 exit /b 1
call test_xui\build_document_style_test.bat
if %errorlevel% neq 0 exit /b 1
call test_xui\build_document_scale_test.bat
if %errorlevel% neq 0 exit /b 1
call test_xui\build_document_fractional_test.bat
if %errorlevel% neq 0 exit /b 1
call test_xui\build_document_table_geometry_test.bat
if %errorlevel% neq 0 exit /b 1
call test_xui\build_document_editor_test.bat
if %errorlevel% neq 0 exit /b 1
call test\build_clipboard_win32_test.bat
if %errorlevel% neq 0 exit /b 1
call test_xui\build_document_dib_paste_test.bat
if %errorlevel% neq 0 exit /b 1
call test_xui\build_document_object_source_test.bat
if %errorlevel% neq 0 exit /b 1
call test_xui\build_message_document_test.bat
if %errorlevel% neq 0 exit /b 1
call examples\xui_document\build.bat
if %errorlevel% neq 0 exit /b 1
build\xui_document.exe --verify
if %errorlevel% neq 0 exit /b 1
build\xui_document.exe --verify-async
if %errorlevel% neq 0 exit /b 1
build\xui_document.exe --verify-image-placeholder
if %errorlevel% neq 0 exit /b 1
echo Document core, DLL, renderer, editor and native rendering checks passed.
if "%~1"=="" echo CommonMark corpus was not supplied; its check was skipped.
exit /b 0
