@echo off
setlocal
cd /d "%~dp0..\.."
python tests\xge\build_3d_suite.py %*
exit /b %errorlevel%
