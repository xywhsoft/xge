@echo off
setlocal
chcp 65001 >nul

echo [XGE] Running platform scaffold check...
call check_platform_scaffold.bat
if errorlevel 1 exit /b 1

echo.
echo [XGE] Running script documentation check...
call check_script_docs.bat
if errorlevel 1 exit /b 1

echo.
echo [XGE] Running platform toolchain check...
call check_platform_backends.bat

echo.
echo [XGE] Running platform result status check...
call check_platform_results.bat
if errorlevel 1 (
	echo [XGE] Platform result check reported blocked platforms. This is expected until real target smoke tests pass.
)

echo.
echo [XGE] Platform preparation checks finished.
endlocal
exit /b 0
