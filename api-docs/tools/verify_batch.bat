@echo off
rem XGE api-docs batch syntax verify (SPEC section 6/8, gate 4)
setlocal
where gcc >nul 2>nul
if errorlevel 1 (
    echo [FAIL] gcc not found in PATH
    exit /b 1
)
set "REPO=%~dp0..\.."
gcc -fsyntax-only -Wall -I "%REPO%" "%~dp0verify_tu.c"
if errorlevel 1 (
    echo [FAIL] xge.h/xui.h syntax check failed
    exit /b 1
)
echo [OK] xge.h + xui.h syntax check passed
exit /b 0
