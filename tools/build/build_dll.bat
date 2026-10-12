@echo off
setlocal
cd /d "%~dp0\..\.."
python tools\build\build.py %*
exit /b %errorlevel%
