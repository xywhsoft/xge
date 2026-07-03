@echo off
setlocal

echo [XGE] Verifying xge/xgedbg build split...

call build_dll.bat
if errorlevel 1 exit /b 1

call build_dbg_dll.bat
if errorlevel 1 exit /b 1

call build_debug_api_compile_test.bat
if errorlevel 1 exit /b 1

call examples\app_mode\build.bat
if errorlevel 1 exit /b 1

call examples\async\build.bat
if errorlevel 1 exit /b 1

call examples\platform_smoke\build.bat
if errorlevel 1 exit /b 1

call examples\xui_layout_gallery\build.bat
if errorlevel 1 exit /b 1

call examples\xui_xson_page_lab\build.bat
if errorlevel 1 exit /b 1

call examples\xui_xson_style_lab\build.bat
if errorlevel 1 exit /b 1

call examples\xui_xson_app_layout_lab\build.bat
if errorlevel 1 exit /b 1

call examples\xui_xson_virtual_list_lab\build.bat
if errorlevel 1 exit /b 1

call examples\xui_xson_layout_gallery_lab\build.bat
if errorlevel 1 exit /b 1

call examples\debug_overlay\build.bat
if errorlevel 1 exit /b 1

call examples\diagnostics_caps_log\build.bat
if errorlevel 1 exit /b 1

call examples\xui_tabs_dirty_rect_lab\build.bat
if errorlevel 1 exit /b 1

echo [OK] xge/xgedbg build split verified.
endlocal
