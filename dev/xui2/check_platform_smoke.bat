@echo off
setlocal

call examples\platform_smoke\build.bat
if errorlevel 1 exit /b 1

call run_platform_smoke.bat
exit /b %ERRORLEVEL%
