@echo off
setlocal

set OUT_DIR=build
set OUT=%OUT_DIR%\test_ime_mode_contract.exe
set SRC=test\test_ime_mode_contract.c
set INC=-I.
set FLAGS=-O0 -Wall -Wextra -Wno-unused-parameter -Wno-unused-function -Wno-cast-function-type -DXGE_DEBUGMODE=0
set LIBS=-lm -lws2_32 -liphlpapi -lgdi32 -luser32 -lshell32 -lopengl32 -lole32 -loleaut32 -luuid -limm32 -lwinmm -lavrt

where gcc >nul 2>nul
if %errorlevel% neq 0 exit /b 1
if not exist %OUT_DIR% mkdir %OUT_DIR% || exit /b 1

echo [XGE] Building IME mode-contract test...
gcc %FLAGS% %INC% -o %OUT% %SRC% %LIBS%
if %errorlevel% neq 0 exit /b 1

%OUT%
if %errorlevel% neq 0 exit /b 1

echo [XGE] IME mode-contract test passed: %OUT%
endlocal
