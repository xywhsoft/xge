@echo off
setlocal
cd /d "%~dp0\.."
set "SDK=%WEBVIEW2_SDK_DIR%"
if "%SDK%"=="" set "SDK=artifacts\xui-webview-probe\sdk"
if not exist "%SDK%\build\native\include\WebView2.h" (
    echo Set WEBVIEW2_SDK_DIR to the extracted Microsoft.Web.WebView2 NuGet package. 1>&2
    exit /b 1
)
if not exist "%SDK%\build\native\x64\WebView2Loader.dll.lib" exit /b 1
if not exist build\webview mkdir build\webview
if not exist artifacts\xui-webview-probe mkdir artifacts\xui-webview-probe
call ensure_xge_dll.bat
if %errorlevel% neq 0 exit /b 1
gcc -std=c11 -O2 -Wall -Wextra -Werror -Wno-unknown-pragmas -isystem "%SDK%\build\native\include" -o build\webview\xui_webview_win32_probe.exe test_xui\xui_webview_win32_probe.c "%SDK%\build\native\x64\WebView2Loader.dll.lib" -lole32 -luuid -lgdi32 -luser32
if %errorlevel% neq 0 exit /b 1
gcc -std=c11 -O2 -Wall -Wextra -Werror -Wno-unknown-pragmas -DXGE_DLL -DXUI_DLL -DXUI_WEBVIEW_PROBE_XGE -I. -isystem "%SDK%\build\native\include" -o build\webview\xui_webview_xge_probe.exe test_xui\xui_webview_win32_probe.c "%SDK%\build\native\x64\WebView2Loader.dll.lib" build\xge.lib -lole32 -luuid -lgdi32 -luser32 -lws2_32 -liphlpapi -lshell32 -lwinmm -lavrt
if %errorlevel% neq 0 exit /b 1
copy /y "%SDK%\build\native\x64\WebView2Loader.dll" build\webview\WebView2Loader.dll >nul
if %errorlevel% neq 0 exit /b 1
copy /y build\xge.dll build\webview\xge.dll >nul
if %errorlevel% neq 0 exit /b 1
build\webview\xui_webview_win32_probe.exe
if %errorlevel% neq 0 exit /b 1
build\webview\xui_webview_xge_probe.exe
exit /b %errorlevel%
