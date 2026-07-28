@echo off
setlocal
set ROOT=%~dp0..\..
pushd "%ROOT%" || exit /b 1
set OUT_DIR=build
set OUT=%OUT_DIR%\audit_xui_basic.exe
set SRC=examples\audit_xui_basic\main.c
set INC=-I.
set FLAGS=-O2 -Wall -Wextra -Wno-unused-parameter -Wno-unused-function -Wno-cast-function-type -DXGE_DLL -DXGE_DEBUGMODE=0
set XGE_LIB=%OUT_DIR%\xge.lib
set LIBS=%XGE_LIB% -lm -lws2_32 -liphlpapi -lgdi32 -luser32 -lshell32 -lopengl32 -lole32 -lwinmm -lavrt
where gcc >nul 2>nul
if errorlevel 1 ( echo [ERROR] gcc not found & popd & exit /b 1 )
if not exist "%OUT_DIR%" mkdir "%OUT_DIR%"
if not exist "%XGE_LIB%" ( call build_dll.bat & if errorlevel 1 ( popd & exit /b 1 ) )
echo [AUDIT] Building audit_xui_basic...
gcc %FLAGS% %INC% -o "%OUT%" %SRC% %LIBS%
if errorlevel 1 ( echo [AUDIT] Build failed & popd & exit /b 1 )
echo [AUDIT] Build successful: %OUT%
popd
endlocal
