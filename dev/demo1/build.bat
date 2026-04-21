@echo off
setlocal

REM ============================================================================
REM XGE 引擎编译脚本 - Windows
REM ============================================================================

set SRC=main.c
set OUT=xge.exe
set INC=-I.
set LIBS=-lgdi32 -luser32 -lshell32 -lopengl32
set FLAGS=-O2 -Wall

REM 检查编译器
where gcc >nul 2>nul
if %errorlevel% neq 0 (
    echo [ERROR] gcc not found in PATH
    exit /b 1
)

echo [XGE] Compiling %SRC%...
gcc %FLAGS% %INC% -o %OUT% %SRC% %LIBS%

if %errorlevel% equ 0 (
    echo [XGE] Build successful: %OUT%
) else (
    echo [XGE] Build failed
    exit /b 1
)

endlocal
