@echo off
setlocal

if "%XGE_WEB_PORT%"=="" (
	set PORT=8000
) else (
	set PORT=%XGE_WEB_PORT%
)

set ROOT=build\web
set HTML=%ROOT%\xge_web.html

if not exist "%HTML%" (
	echo [XGE] %HTML% not found. Run build_web_exe.bat first.
	exit /b 1
)

where python >nul 2>nul
if errorlevel 1 (
	echo [XGE] python not found.
	exit /b 1
)

echo [XGE] Serving http://localhost:%PORT%/xge_web.html
python -m http.server %PORT% -d "%ROOT%"
endlocal
