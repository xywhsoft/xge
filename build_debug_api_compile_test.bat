@echo off
setlocal

set OUT_DIR=build
set RELEASE_LOG=%OUT_DIR%\debug_api_release_fail.log
set RELEASE_OBJ=%OUT_DIR%\debug_api_release_fail.o
set LINK_XGE_LOG=%OUT_DIR%\debug_api_link_xge_fail.log
set LINK_XGE_OUT=%OUT_DIR%\debug_api_link_xge_fail.exe
set DEBUG_OUT=%OUT_DIR%\debug_api_debug_ok.exe
set XUI_SNAPSHOT_OUT=%OUT_DIR%\debug_xui_snapshot.exe
set INC=-I.
set COMMON_FLAGS=-O2 -Wall -Wextra -Werror=implicit-function-declaration -Wno-unused-parameter -Wno-unused-function -Wno-cast-function-type
set XGE_LIB=%OUT_DIR%\xge.lib
set XGEDBG_LIB=%OUT_DIR%\xgedbg.lib
set SYS_LIBS=-lws2_32 -liphlpapi -lgdi32 -luser32 -lshell32 -lopengl32 -lole32 -lwinmm -lavrt
set LIBS=%XGEDBG_LIB% %SYS_LIBS%

where gcc >nul 2>nul
if %errorlevel% neq 0 (
	echo [ERROR] gcc not found in PATH
	exit /b 1
)

if not exist %OUT_DIR% (
	mkdir %OUT_DIR% || exit /b 1
)

echo [XGE] Verifying debug API is unavailable when XGE_DEBUGMODE=0...
gcc %COMMON_FLAGS% %INC% -DXGE_DLL -DXGE_DEBUGMODE=0 -c test\debug_api_release_fail.c -o %RELEASE_OBJ% > %RELEASE_LOG% 2>&1
if %errorlevel% equ 0 (
	echo [ERROR] Debug API compiled in XGE_DEBUGMODE=0, expected failure.
	type %RELEASE_LOG%
	exit /b 1
)
echo [XGE] Expected compile failure observed.

if not exist %XGE_LIB% (
	call build_dll.bat
	if %errorlevel% neq 0 exit /b %errorlevel%
)

echo [XGE] Verifying debug API does not link against xge.lib...
gcc %COMMON_FLAGS% %INC% -DXGE_DLL -DXGE_DEBUGMODE=1 -o %LINK_XGE_OUT% test\debug_api_debug_ok.c %XGE_LIB% %SYS_LIBS% > %LINK_XGE_LOG% 2>&1
if %errorlevel% equ 0 (
	echo [ERROR] Debug API linked against xge.lib, expected failure.
	type %LINK_XGE_LOG%
	exit /b 1
)
echo [XGE] Expected link failure observed.

if not exist %XGEDBG_LIB% (
	call build_dbg_dll.bat
	if %errorlevel% neq 0 exit /b %errorlevel%
)

echo [XGEDBG] Verifying debug API is available when XGE_DEBUGMODE=1...
gcc %COMMON_FLAGS% %INC% -DXGE_DLL -DXGE_DEBUGMODE=1 -o %DEBUG_OUT% test\debug_api_debug_ok.c %LIBS%
if %errorlevel% neq 0 (
	echo [ERROR] Debug API failed to compile or link in XGE_DEBUGMODE=1.
	exit /b 1
)

echo [XGEDBG] Verifying XUI snapshot debug API at runtime...
gcc %COMMON_FLAGS% %INC% -DXGE_DLL -DXGE_DEBUGMODE=1 -o %XUI_SNAPSHOT_OUT% test\debug_xui_snapshot.c %LIBS%
if %errorlevel% neq 0 (
	echo [ERROR] XUI snapshot debug test failed to compile or link.
	exit /b 1
)
%XUI_SNAPSHOT_OUT%
if %errorlevel% neq 0 (
	echo [ERROR] XUI snapshot debug test failed.
	exit /b 1
)

echo [OK] Debug API compile boundary verified.
endlocal
