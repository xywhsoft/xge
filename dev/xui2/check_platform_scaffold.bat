@echo off
setlocal
chcp 65001 >nul
set PYTHONUTF8=1

where python >nul 2>nul
if errorlevel 1 (
	echo [XGE] python not found.
	exit /b 1
)

python tools\check_platform_scaffold.py
exit /b %ERRORLEVEL%
