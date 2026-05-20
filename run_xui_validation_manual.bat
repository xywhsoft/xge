@echo off
setlocal

set ROOT=%~dp0
set BUILD=%ROOT%build

if "%~1"=="/?" goto help
if /i "%~1"=="--help" goto help
if /i "%~1"=="help" goto help

if not exist "%BUILD%\xge_xui_bridge.exe" (
	echo [XGE] XUI validation EXEs not found, building first...
	call "%ROOT%build_examples_all.bat" --xui
	if %errorlevel% neq 0 exit /b 1
)

echo [XGE] XUI manual validation runner
echo [XGE] Close each validation window to continue to the next one.
echo [XGE] Record results in the XUI Windows manual validation markdown file under dev\docs.

call :run "XUI Bridge" "build\xge_xui_bridge.exe"
if %errorlevel% neq 0 exit /b 1

call :run "XUI Layout Validation" "build\xge_xui_layout_validation.exe"
if %errorlevel% neq 0 exit /b 1

call :run "XUI Input Validation" "build\xge_xui_input_validation.exe"
if %errorlevel% neq 0 exit /b 1

call :run "XUI TextEdit Validation" "build\xge_xui_text_edit_validation.exe"
if %errorlevel% neq 0 exit /b 1

echo [XGE] XUI manual validation sequence finished.
echo [XGE] Please copy each summary line into:
echo [XGE]   the XUI Windows manual validation markdown file under dev\docs.
exit /b 0

:run
echo.
echo [XGE] Running %~1...
if not exist "%ROOT%%~2" (
	echo [XGE] Missing executable: %~2
	exit /b 1
)
start "%~1" /wait "%ROOT%%~2"
set CODE=%errorlevel%
echo [XGE] %~1 exited with code %CODE%
if not "%CODE%"=="0" exit /b %CODE%
exit /b 0

:help
echo [XGE] Usage: run_xui_validation_manual.bat
echo [XGE] Builds missing XUI validation EXEs through build_examples_all.bat --xui, then runs each validation window in sequence.
echo [XGE] Record results in the XUI Windows manual validation markdown file under dev\docs.
exit /b 0
