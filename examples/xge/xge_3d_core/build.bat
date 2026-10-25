@echo off
setlocal
cd /d "%~dp0..\..\.."
python examples\xge\xge_3d_core\build.py %*
exit /b %errorlevel%
