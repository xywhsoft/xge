@echo off
setlocal
cd /d "%~dp0\..\.."

rem Verify the public WebView API remains available when the optional WebView2
rem backend is not compiled. The internal provider channel is linked from the
rem WebView object because production DLLs intentionally do not export it.
set "XUI_ENABLE_WEBVIEW2="
set "XUI_WEBVIEW_TEST_EXPORTS="
call tools\build\build_dll.bat default build\webview-disabled
if errorlevel 1 exit /b 1

set "WEBVIEW_OBJECT=build\webview-disabled\obj\src_xui_document_xui_webview.c.o"
if not exist "%WEBVIEW_OBJECT%" (
    echo Missing WebView object: %WEBVIEW_OBJECT% 1>&2
    exit /b 1
)

gcc -std=c11 -O2 -Wall -Wextra -Werror -DXGE_DLL @config/includes.rsp ^
    -o build\webview-disabled\xui_webview_disabled_test.exe ^
    tests\xui\xui_webview_disabled_test.c "%WEBVIEW_OBJECT%" ^
    build\webview-disabled\xge.lib -lole32 -luuid -lgdi32 -luser32
if errorlevel 1 exit /b 1

build\webview-disabled\xui_webview_disabled_test.exe
exit /b %errorlevel%
