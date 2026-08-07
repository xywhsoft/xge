@echo off
setlocal
set ROOT=%~dp0..\..
pushd "%ROOT%" || exit /b 1
set OUT=build\xge_emoji.exe
set FLAGS=-O2 -Wall -Wextra -Wno-unused-parameter -Wno-unused-function -DXGE_DLL -DXGE_DEBUGMODE=0
set LIBS=build\xge.lib -lm -lws2_32 -liphlpapi -lgdi32 -luser32 -lshell32 -lopengl32 -lole32 -lwinmm -lavrt
if not exist build\xge.lib call build_dll.bat
if errorlevel 1 ( popd & exit /b 1 )
gcc %FLAGS% -I. -o "%OUT%" examples\xge_emoji\main.c %LIBS%
if errorlevel 1 ( popd & exit /b 1 )
echo [XGE] Built %OUT%
popd
endlocal
