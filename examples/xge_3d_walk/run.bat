@echo off
setlocal
cd /d "%~dp0..\.."
python examples\xge_3d_walk\build.py --run %*
exit /b %errorlevel%
