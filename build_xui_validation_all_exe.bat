@echo off
setlocal

if "%~1"=="/?" goto help
if /i "%~1"=="--help" goto help
if /i "%~1"=="help" goto help

echo [XGE] Building all XUI validation EXEs...

call "%~dp0build_xui_bridge_exe.bat"
if %errorlevel% neq 0 exit /b 1

call "%~dp0build_xui_incubation_exe.bat"
if %errorlevel% neq 0 exit /b 1

call "%~dp0build_xui_layout_validation_exe.bat"
if %errorlevel% neq 0 exit /b 1

call "%~dp0build_xui_input_validation_exe.bat"
if %errorlevel% neq 0 exit /b 1

call "%~dp0build_xui_text_edit_validation_exe.bat"
if %errorlevel% neq 0 exit /b 1

echo [XGE] All XUI validation EXEs built successfully.
echo [XGE] Run:
echo [XGE]   build\xge_xui_bridge.exe
echo [XGE]   build\xge_xui_incubation.exe
echo [XGE]   build\xge_xui_layout_validation.exe
echo [XGE]   build\xge_xui_input_validation.exe
echo [XGE]   build\xge_xui_text_edit_validation.exe

endlocal
exit /b 0

:help
echo [XGE] Usage: build_xui_validation_all_exe.bat
echo [XGE] Builds all XUI validation EXEs sequentially.
echo [XGE] Use build_xui_validation_all_parallel.bat for faster parallel builds.
exit /b 0
