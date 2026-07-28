@echo off
setlocal
set TOOL_NAME=ParticleLab
set TOOL_DIR=%~dp0
set ROOT=%TOOL_DIR%..\..
set OUT=%TOOL_DIR%release\xge_particlelab.exe

set FLAGS=-O2 -Wall -Wextra -Wno-unused-parameter -Wno-unused-function -DXGE_DLL -DXUI_DLL -DXGE_DEBUGMODE=0
set SRC="%TOOL_DIR%src\main.c"
set XGE_LIB="%ROOT%\build\xge.lib"
set LIBS=%XGE_LIB% -lm -lws2_32 -liphlpapi -lgdi32 -luser32 -lshell32 -lcomdlg32 -lole32 -lwinmm -lavrt

if not exist "%TOOL_DIR%release" mkdir "%TOOL_DIR%release"
pushd "%ROOT%" >nul
gcc %FLAGS% -I"%ROOT%" -I"%TOOL_DIR%src" -o "%OUT%" %SRC% %LIBS%
if %ERRORLEVEL% neq 0 ( popd >nul & echo BUILD FAILED & exit /b 1 )
popd >nul
echo BUILD OK: %OUT%
