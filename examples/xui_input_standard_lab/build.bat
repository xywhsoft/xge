@echo off
setlocal

set ROOT=%~dp0..\..
pushd "%ROOT%" || exit /b 1

set OUT_DIR=build
set OUT=%OUT_DIR%\xge_xui_input_standard_lab.exe
set SRC=examples\xui_input_standard_lab\main.c
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

echo [XGE] Building xui input standard lab EXE...
gcc %FLAGS% %INC% -o "%OUT%" %SRC% %LIBS%
if errorlevel 1 (
	echo [XGE] Build failed
	popd
	exit /b 1
)

echo [XGE] Build successful: %OUT%
popd
endlocal
