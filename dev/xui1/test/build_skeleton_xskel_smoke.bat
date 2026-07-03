@echo off
setlocal

set ROOT=%~dp0..
pushd "%ROOT%" || exit /b 1

set OUT_DIR=build
set OUT=%OUT_DIR%\xge_skeleton_xskel_smoke.exe
set SRC=test\skeleton_xskel_load_smoke.c
set INC=-I.
set FLAGS=-O2 -Wall -Wextra -Wno-unused-parameter -Wno-unused-function -Wno-cast-function-type -DXGE_DLL -DXGE_DEBUGMODE=0
set XGE_LIB=%OUT_DIR%\xge.lib
set LIBS=%XGE_LIB% -lws2_32 -liphlpapi -lgdi32 -luser32 -lshell32 -lopengl32 -lole32 -lwinmm -lavrt

where gcc >nul 2>nul
if errorlevel 1 (
	echo [ERROR] gcc not found in PATH
	popd
	exit /b 1
)

if not exist "%OUT_DIR%" (
	mkdir "%OUT_DIR%" || (popd && exit /b 1)
)

if not exist "%XGE_LIB%" (
	call build_dll.bat
	if errorlevel 1 (
		popd
		exit /b 1
	)
)

echo [XGE] Building skeleton xskel smoke...
gcc %FLAGS% %INC% -o "%OUT%" %SRC% %LIBS%
if errorlevel 1 (
	echo [XGE] Build failed
	popd
	exit /b 1
)

"%OUT%"
if errorlevel 1 (
	echo [XGE] Skeleton xskel smoke failed
	popd
	exit /b 1
)

echo [XGE] Skeleton xskel smoke passed: %OUT%
popd
endlocal
