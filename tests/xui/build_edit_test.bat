@echo off
setlocal

set OUT_DIR=build
set OUT=%OUT_DIR%\xui_edit_test.exe
set SRC=tests\xui\xui_edit_test.c
set INC=@config/includes.rsp
set FLAGS=-O2 -Wall -Wextra -Wno-unused-parameter -Wno-unused-function -Wno-cast-function-type -DXGE_DEBUGMODE=0

where gcc >nul 2>nul
if %errorlevel% neq 0 (
	echo [ERROR] gcc not found in PATH
	exit /b 1
)

if not exist %OUT_DIR% (
	mkdir %OUT_DIR% || exit /b 1
)

echo [XUI] Building xui_edit_test...
gcc %FLAGS% %INC% -o %OUT% %SRC%
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
