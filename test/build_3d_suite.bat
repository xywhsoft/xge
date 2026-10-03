@echo off
setlocal
cd /d "%~dp0.."
python test\build_3d_suite.py %*
exit /b %errorlevel%
