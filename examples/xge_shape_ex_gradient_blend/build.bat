@echo off
setlocal

set ROOT=%~dp0..\..
pushd "%ROOT%" || exit /b 1

set OUT_DIR=build
set OUT=%OUT_DIR%\xge_shape_ex_gradient_blend.exe
set SRC=examples\xge_shape_ex_gradient_blend\main.c
set FLAGS=-O2 -Wall -Wextra -Wno-unused-parameter -Wno-cast-function-type -DXGE_DLL -DXGE_DEBUGMODE=0
set LIBS=%OUT_DIR%\xge.lib -lm -lws2_32 -liphlpapi -lgdi32 -luser32 -lshell32 -lopengl32 -lole32 -lwinmm -lavrt

where gcc >nul 2>nul
if errorlevel 1 (
	echo [ERROR] gcc not found in PATH
	popd
	exit /b 1
)

if not exist "%OUT_DIR%\xge.lib" (
	call build_dll.bat
	if errorlevel 1 (
		popd
		exit /b 1
	)
)

echo [XGE] Building ShapeEx gradient blend comparison example...
gcc %FLAGS% -I. -o "%OUT%" %SRC% %LIBS%
if errorlevel 1 (
	echo [XGE] Build failed
	popd
	exit /b 1
)

echo [XGE] Build successful: %OUT%
popd
endlocal
