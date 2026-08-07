@echo off
setlocal
cd /d "%~dp0\..\.."

if "%~1"=="" exit /b 2
set MODE=%~1
set MAIN=examples\xui_ime_%MODE%\main.c
set OUT=build\xui_ime_%MODE%.exe
if not exist "%MAIN%" exit /b 2

set SRC=%MAIN%
set FLAGS=-O2 -Wall -Wextra -Wno-unused-parameter -Wno-unused-function -Wno-cast-function-type -DXGE_DLL -DXGE_DEBUGMODE=0 -I.
set LIBS=build\xge.lib -lm -lws2_32 -liphlpapi -lgdi32 -luser32 -lshell32 -lopengl32 -lole32 -loleaut32 -lwinmm -lavrt

where gcc >nul 2>nul
if %errorlevel% neq 0 exit /b 1
if not exist build mkdir build
call ensure_xge_dll.bat
if %errorlevel% neq 0 exit /b %errorlevel%

echo [XUI] Building xui_ime_%MODE%...
gcc %FLAGS% -o "%OUT%" %SRC% %LIBS%
if %errorlevel% neq 0 exit /b %errorlevel%
echo [XUI] Build successful: %OUT%
endlocal
