@echo off
setlocal
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0build_style_pickers_test.ps1" %*
exit /b %errorlevel%
