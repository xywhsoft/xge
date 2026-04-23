@echo off
setlocal

powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0build_xui_validation_all_parallel.ps1" %*
exit /b %errorlevel%
