@echo off
setlocal
cd /d "%~dp0\..\.."
set "SDK=%WEBVIEW2_SDK_DIR%"
if "%SDK%"=="" set "SDK=artifacts\xui-webview-probe\sdk"
if not exist "%SDK%\build\native\include\WebView2.h" (
    echo Set WEBVIEW2_SDK_DIR to the extracted WebView2 SDK. 1>&2
    exit /b 1
)
set "WEBVIEW2_SDK_DIR=%SDK%"
set "XUI_ENABLE_WEBVIEW2=1"
set "XUI_WEBVIEW_TEST_EXPORTS=0"
call tools\build\build_dll.bat default build\webview-basic
if errorlevel 1 exit /b 1
rem The normal builder must package the loader itself; do not mask a missing dependency.
if not exist build\webview-basic\WebView2Loader.dll exit /b 1
gcc -std=c11 -O2 -Wall -Wextra -Werror -DXGE_DLL -DXUI_DLL @config/includes.rsp -o build\webview-basic\xui_webview_basic_test.exe tests\xui\xui_webview_basic_test.c build\webview-basic\xge.lib -lole32 -luuid -lgdi32 -luser32
if errorlevel 1 exit /b 1
gcc -std=c11 -O2 -Wall -Wextra -Werror -o build\webview-basic\xui_webview_exports_test.exe tests\xui\xui_webview_exports_test.c
if errorlevel 1 exit /b 1
build\webview-basic\xui_webview_exports_test.exe "build\webview-basic\xge.dll"
if errorlevel 1 exit /b 1
build\webview-basic\xui_webview_basic_test.exe
exit /b %errorlevel%
