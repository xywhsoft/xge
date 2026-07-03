@echo off
setlocal
cd /d "%~dp0\..\.."

if not exist build\xui_chart.exe (
	call examples\xui_chart\build.bat
	if %errorlevel% neq 0 exit /b %errorlevel%
)

echo [XUI] Running xui_chart interactively. Close the window or press Esc to exit.
build\xui_chart.exe
endlocal
