@echo off
setlocal
cd /d "%~dp0\.."
set "OUT_DIR=build\webview"
if not "%~1"=="" set "OUT_DIR=%~1"
if not exist "%OUT_DIR%\xge.lib" (
    echo Build the optional WebView2 DLL first with build_webview_widget_test.bat. 1>&2
    exit /b 1
)
gcc -std=c11 -O2 -Wall -Wextra -Werror -DXGE_DLL -DXUI_DLL -DXUI_WEBVIEW_TEST_EXPORTS -I. -o "%OUT_DIR%\xui_document_html_interaction_test.exe" test_xui\xui_document_html_interaction_test.c "%OUT_DIR%\xge.lib" -lole32 -luuid -lgdi32 -luser32 -lws2_32
if %errorlevel% neq 0 exit /b 1
"%OUT_DIR%\xui_document_html_interaction_test.exe"
exit /b %errorlevel%
