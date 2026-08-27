@echo off
setlocal
set OUT_DIR=build
set OUT=%OUT_DIR%\test_drag_data_object.exe
set SRC=test\test_drag_data_object.c
set FLAGS=-O1 -Wall -Wextra -Wno-unused-parameter -Wno-unused-function -Wno-cast-function-type -DXGE_DEBUGMODE=0
set LIBS=-lm -lws2_32 -liphlpapi -lgdi32 -luser32 -lshell32 -lopengl32 -lole32 -loleaut32 -luuid -limm32 -lwinmm -lavrt
if not exist %OUT_DIR% mkdir %OUT_DIR% || exit /b 1
gcc %FLAGS% -I. -o %OUT% %SRC% %LIBS%
if %errorlevel% neq 0 exit /b 1
%OUT%
if %errorlevel% neq 0 exit /b 1
echo [XGE] Drag data object test passed: %OUT%
endlocal
