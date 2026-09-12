@echo off
setlocal
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0build_style_overlays_test.ps1" %*
exit /b %errorlevel%
