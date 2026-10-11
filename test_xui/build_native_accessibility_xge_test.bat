@echo off
setlocal
cd /d "%~dp0.."
call ensure_xge_dll.bat
if errorlevel 1 exit /b %errorlevel%
if not exist build\document mkdir build\document
gcc -std=c11 -O2 -g -Wall -Wextra -Werror -Wno-unused-function -Wno-unused-parameter -I. -DXGE_DLL -DXUI_DLL -o build\document\xui_native_accessibility_xge_test.exe test_xui\xui_native_accessibility_xge_test.c build\xge.lib -lm -lopengl32 -lws2_32 -liphlpapi -lgdi32 -luser32 -lshell32 -lole32 -loleaut32 -luuid -lwinmm -lavrt
if errorlevel 1 exit /b %errorlevel%
set PATH=%CD%\build;%PATH%
build\document\xui_native_accessibility_xge_test.exe build\document\native-accessibility-xge.png
exit /b %errorlevel%
