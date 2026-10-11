@echo off
setlocal
call ensure_xge_dll.bat
if errorlevel 1 exit /b 1
python test_xui\build_document_cst_test.py
exit /b %errorlevel%
