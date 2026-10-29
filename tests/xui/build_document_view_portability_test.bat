@echo off
setlocal
cd /d "%~dp0\..\.."

call tools\build\ensure_xge_dll.bat
if errorlevel 1 exit /b 1
if not exist build\document mkdir build\document

rem Do not define XGE_DLL/XUI_DLL: this headless test supplies local image
rem stubs while the remaining public symbols resolve through the import library.
gcc -std=c11 -O2 -g -Wall -Wextra -Werror -Wno-unused-function ^
    @config/includes.rsp ^
    -o build\document\xui_document_view_portability_test.exe ^
    tests\xui\xui_document_view_portability_test.c ^
    tests\xui\xui_test_proxy.c build\xge.lib ^
    -lm -lws2_32 -liphlpapi -lgdi32 -luser32 -lshell32 -lole32 -lwinmm -lavrt
if errorlevel 1 exit /b 1

set "PATH=%CD%\build;%PATH%"
build\document\xui_document_view_portability_test.exe
exit /b %errorlevel%
