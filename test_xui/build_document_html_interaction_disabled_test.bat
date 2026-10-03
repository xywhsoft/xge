@echo off
setlocal
cd /d "%~dp0\.."
if not exist build\xge.lib (
    echo Build the normal DLL first. 1>&2
    exit /b 1
)
if not exist build\document mkdir build\document || exit /b 1
gcc -std=c11 -O2 -Wall -Wextra -Werror -DXGE_DLL -DXUI_DLL -I. -o build\document\xui_document_html_interaction_disabled_test.exe test_xui\xui_document_html_interaction_disabled_test.c build\xge.lib -lole32 -luuid -lgdi32 -luser32
if %errorlevel% neq 0 exit /b 1
set PATH=%CD%\build;%PATH%
build\document\xui_document_html_interaction_disabled_test.exe
exit /b %errorlevel%
