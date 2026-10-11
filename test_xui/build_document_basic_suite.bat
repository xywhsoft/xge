@echo off
setlocal
cd /d "%~dp0\.."
python test_xui\build_document_basic_suite.py %*
exit /b %errorlevel%
