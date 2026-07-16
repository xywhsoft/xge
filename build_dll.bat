@echo off
setlocal

set OUT_DIR=build
set OUT=%OUT_DIR%\xge.dll
set OUT_LIB=%OUT_DIR%\xge.lib
set RES_OBJ=%OUT_DIR%\xge_res.o
call xui_sources.bat
set SRC=xge.c %XUI_SRC%
set RC=xge.rc
set INC=-I.
set FLAGS=-O2 -Wall -Wextra -Wno-unused-parameter -Wno-unused-function -Wno-cast-function-type -DXGE_DLL -DXGE_BUILD_DLL -DXUI_DLL -DXUI_BUILD_DLL -DBUILD_DLL -DXGE_DEBUGMODE=0
set LIBS=-lm -lws2_32 -liphlpapi -lgdi32 -luser32 -lshell32 -lole32 -lwinmm -lavrt

where gcc >nul 2>nul
if %errorlevel% neq 0 (
	echo [ERROR] gcc not found in PATH
	exit /b 1
)

if not exist %OUT_DIR% (
	mkdir %OUT_DIR% || exit /b 1
)

where windres >nul 2>nul
if %errorlevel% neq 0 (
	echo [ERROR] windres not found in PATH
	exit /b 1
)

echo [XGE] Building DLL resources...
windres -O coff -i %RC% -o %RES_OBJ%
if %errorlevel% neq 0 (
	echo [XGE] Resource build failed
	exit /b 1
)

echo [XGE] Building DLL...
gcc -shared %FLAGS% %INC% -Wl,--out-implib,%OUT_LIB% -o %OUT% %SRC% %RES_OBJ% %LIBS%
if %errorlevel% neq 0 (
	echo [XGE] Build failed
	exit /b 1
)

echo [XGE] Build successful: %OUT%
echo [XGE] Import library: %OUT_LIB%
endlocal
