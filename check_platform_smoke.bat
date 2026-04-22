@echo off
setlocal

call build_platform_smoke_exe.bat
if errorlevel 1 exit /b 1

call run_platform_smoke.bat
exit /b %ERRORLEVEL%
