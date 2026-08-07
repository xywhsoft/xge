@echo off
setlocal
cd /d "%~dp0.."

set OUT=build\test_tsf_candidate_ui.exe
set SRC=test\test_tsf_candidate_ui.c
set INC=-I.
set FLAGS=-O0 -Wall -Wextra -Wno-unused-parameter -Wno-unused-function -Wno-cast-function-type -DXGE_DEBUGMODE=0
set LIBS=-lm -lws2_32 -liphlpapi -lgdi32 -luser32 -lshell32 -lopengl32 -lole32 -loleaut32 -luuid -limm32 -lwinmm -lavrt

where gcc >nul 2>nul
if %errorlevel% neq 0 exit /b 1

echo [XGE] Building TSF candidate UI test...
gcc %FLAGS% %INC% -o %OUT% %SRC% %LIBS%
if %errorlevel% neq 0 exit /b 1

%OUT%
if %errorlevel% neq 0 exit /b 1
echo [XGE] TSF candidate UI test passed: %OUT%
endlocal
