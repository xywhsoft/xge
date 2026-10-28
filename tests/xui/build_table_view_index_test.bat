@echo off
setlocal
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0build_style_collections_test.ps1" -Family table -Mode audit
exit /b %errorlevel%
