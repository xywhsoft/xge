@echo off
setlocal
cd /d "%~dp0..\.."
python tests\xge\check_feature_profiles.py %*
exit /b %errorlevel%
