@echo off
setlocal

set ROOT=%~dp0..\..
pushd "%ROOT%" || exit /b 1

where emcc >nul 2>nul
if errorlevel 1 (
	echo [XGE] emcc not found. Please activate the Emscripten SDK first.
	popd
	exit /b 1
)

set OUT_DIR=build\web
set OUT=%OUT_DIR%\xge_web.html
set SRC=xge.c examples\platform_smoke\main.c
set INC=-I.
set FLAGS=-O2 -Wall -Wextra -Wno-unused-parameter -Wno-unused-function -Wno-cast-function-type -DSOKOL_GLES3 -DXGE_NO_AUDIO
set EMFLAGS=-sMIN_WEBGL_VERSION=2 -sMAX_WEBGL_VERSION=2 -sFULL_ES3=1 -sALLOW_MEMORY_GROWTH=1 -sEXIT_RUNTIME=0 --shell-file platform/web/shell.html

if not exist "%OUT_DIR%" mkdir "%OUT_DIR%"
echo [XGE] Building Web/Emscripten MVP...
emcc %FLAGS% %INC% -o "%OUT%" %SRC% %EMFLAGS%
if errorlevel 1 (
	popd
	exit /b 1
)
echo [XGE] Build successful: %OUT%
echo [XGE] Serve build\web with a local HTTP server before opening the page.
popd
endlocal