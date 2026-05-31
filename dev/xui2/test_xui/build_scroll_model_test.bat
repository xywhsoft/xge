@echo off
setlocal

set OUT_DIR=build
set OUT=%OUT_DIR%\xui_scroll_model_test.exe
set SRC=test_xui\xui_scroll_model_test.c src\xui_scroll_model.c
set INC=-I.
set FLAGS=-O2 -Wall -Wextra -Wno-unused-parameter -Wno-unused-function -Wno-cast-function-type -DXGE_DLL -DXGE_DEBUGMODE=0

where gcc >nul 2>nul
if %errorlevel% neq 0 (
	echo [ERROR] gcc not found in PATH
	exit /b 1
)

if not exist %OUT_DIR% (
	mkdir %OUT_DIR% || exit /b 1
)

echo [XUI] Building xui_scroll_model_test...
gcc %FLAGS% %INC% -o %OUT% %SRC% -lm
if %errorlevel% neq 0 (
	echo [XUI] Build failed
	exit /b 1
)

%OUT%
if %errorlevel% neq 0 (
	echo [XUI] Test failed
	exit /b %errorlevel%
)

echo [XUI] Tests passed: %OUT%
endlocal
