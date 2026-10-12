@echo off
setlocal
cd /d "%~dp0\..\.."
python tests\xui\build_document_basic_suite.py %*
exit /b %errorlevel%
