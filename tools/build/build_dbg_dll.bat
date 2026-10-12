@echo off
setlocal
cd /d "%~dp0\..\.."
python tools\build\build.py default build --debug %*
exit /b %errorlevel%
