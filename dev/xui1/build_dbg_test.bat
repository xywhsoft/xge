@echo off
setlocal

set OUT_DIR=build
set OUT=%OUT_DIR%\xge_dbg_test.exe
set SRC=test\test_main.c
set INC=-I.
set FLAGS=-O2 -Wall -Wextra -Wno-unused-parameter -Wno-unused-function -Wno-cast-function-type -DXGE_DLL -DXGE_DEBUGMODE=1
set XGE_LIB=%OUT_DIR%\xgedbg.lib
set LIBS=%XGE_LIB% -lws2_32 -liphlpapi -lgdi32 -luser32 -lshell32 -lopengl32 -lole32 -lwinmm -lavrt

where gcc >nul 2>nul
if %errorlevel% neq 0 (
	echo [ERROR] gcc not found in PATH
	exit /b 1
)

if not exist %OUT_DIR% (
	mkdir %OUT_DIR% || exit /b 1
)

if not exist %XGE_LIB% (
	call build_dbg_dll.bat
	if %errorlevel% neq 0 exit /b %errorlevel%
)

echo [XGEDBG] Building debug tests...
gcc %FLAGS% %INC% -o %OUT% %SRC% %LIBS%
if %errorlevel% neq 0 (
	echo [XGEDBG] Build failed
	exit /b 1
)

%OUT%
if %errorlevel% neq 0 (
	echo [XGEDBG] Tests failed
	exit /b 1
)

echo [XGEDBG] Tests passed: %OUT%
endlocal
