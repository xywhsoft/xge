@echo off
setlocal
cd /d "%~dp0\.."
if not exist build\webview\xge.lib (
    echo Build the optional WebView2 DLL first with build_webview_widget_test.bat. 1>&2
    exit /b 1
)
gcc -std=c11 -O2 -Wall -Wextra -Werror -DXGE_DLL -DXUI_DLL -DXUI_WEBVIEW_TEST_EXPORTS -I. -o build\webview\xui_webview_script_test.exe test_xui\xui_webview_script_test.c build\webview\xge.lib -lole32 -luuid -lgdi32 -luser32
if %errorlevel% neq 0 exit /b 1
build\webview\xui_webview_script_test.exe
exit /b %errorlevel%
