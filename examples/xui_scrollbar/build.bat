@echo off
setlocal
cd /d "%~dp0\..\.."

set OUT_DIR=build
set OUT=%OUT_DIR%\xui_scrollbar.exe
set SRC=examples\xui_scrollbar\main.c src\xui_core.c src\xui_widget.c src\xui_input.c src\xui_text.c src\xui_label.c src\xui_scrollbar.c src\xui_proxy_xge.c
set INC=-I.
set FLAGS=-O2 -Wall -Wextra -Wno-unused-parameter -Wno-unused-function -Wno-cast-function-type -DXGE_DLL -DXGE_DEBUGMODE=0
set XGE_LIB=%OUT_DIR%\xge.lib
set LIBS=%XGE_LIB% -lm -lws2_32 -liphlpapi -lgdi32 -luser32 -lshell32 -lopengl32 -lole32 -lwinmm -lavrt

where gcc >nul 2>nul
if %errorlevel% neq 0 (
	echo [ERROR] gcc not found in PATH
	exit /b 1
)

if not exist %OUT_DIR% (
	mkdir %OUT_DIR% || exit /b 1
)

if not exist %XGE_LIB% (
	call build_dll.bat
	if %errorlevel% neq 0 exit /b %errorlevel%
)

echo [XUI] Building xui_scrollbar...
gcc %FLAGS% %INC% -o %OUT% %SRC% %LIBS%
if %errorlevel% neq 0 (
	echo [XUI] Build failed
	exit /b 1
)

echo [XUI] Build successful: %OUT%
endlocal
