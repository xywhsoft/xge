@echo off
setlocal
cd /d "%~dp0.."
python test\check_feature_profiles.py %*
exit /b %errorlevel%
