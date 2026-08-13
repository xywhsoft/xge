@echo off
setlocal
cd /d "%~dp0\..\.."
set OUT_DIR=build
set OUT=%OUT_DIR%\xui_richedit.exe
if not exist %OUT_DIR% mkdir %OUT_DIR%
call ensure_xge_dll.bat
if %errorlevel% neq 0 exit /b %errorlevel%
gcc -O2 -Wall -Wextra -Wno-unused-parameter -Wno-unused-function -Wno-cast-function-type -DXGE_DLL -I. -o %OUT% examples\xui_richedit\main.c build\xge.lib -lm -lws2_32 -liphlpapi -lgdi32 -luser32 -lshell32 -lopengl32 -lole32 -lwinmm -lavrt
if %errorlevel% neq 0 exit /b %errorlevel%
echo [XUI] Build successful: %OUT%
