@echo off
setlocal
chcp 65001 >nul

echo [XGE] Running C language boundary check...
call tools\check\check_c_language_boundary.bat
if errorlevel 1 exit /b 1

echo.
echo [XGE] Running script documentation check...
call tools\check\check_script_docs.bat
if errorlevel 1 exit /b 1

echo.
echo [XGE] Running release documentation check...
python tools\check\check_docs.py
if errorlevel 1 exit /b 1

echo.
echo [XGE] Running platform toolchain check...
call tools\check\check_platform_backends.bat

echo.
echo [XGE] Platform preparation checks finished. Run target smoke tests before release.
endlocal
exit /b 0
