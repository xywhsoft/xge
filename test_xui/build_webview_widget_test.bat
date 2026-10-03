@echo off
setlocal
cd /d "%~dp0\.."
set "SDK=%WEBVIEW2_SDK_DIR%"
if "%SDK%"=="" set "SDK=artifacts\xui-webview-probe\sdk"
if not exist "%SDK%\build\native\include\WebView2.h" (
    echo Set WEBVIEW2_SDK_DIR to the extracted Microsoft.Web.WebView2 NuGet package. 1>&2
    exit /b 1
)
set "WEBVIEW2_SDK_DIR=%SDK%"
set "XUI_ENABLE_WEBVIEW2=1"
set "XUI_WEBVIEW_TEST_EXPORTS=1"
call build_dll.bat default build\webview
if %errorlevel% neq 0 exit /b 1
copy /y "%SDK%\build\native\x64\WebView2Loader.dll" build\webview\WebView2Loader.dll >nul
if %errorlevel% neq 0 exit /b 1
gcc -std=c11 -O2 -Wall -Wextra -Werror -DXGE_DLL -DXUI_DLL -DXUI_WEBVIEW_TEST_EXPORTS -I. -o build\webview\xui_webview_widget_test.exe test_xui\xui_webview_widget_test.c build\webview\xge.lib -lole32 -luuid -lgdi32 -luser32
if %errorlevel% neq 0 exit /b 1
build\webview\xui_webview_widget_test.exe
exit /b %errorlevel%
