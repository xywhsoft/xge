@echo off
setlocal
cd /d "%~dp0"
python tools\build_profile.py %*
exit /b %errorlevel%
