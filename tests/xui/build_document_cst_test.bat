@echo off
setlocal
call tools\build\ensure_xge_dll.bat
if errorlevel 1 exit /b 1
python tests\xui\build_document_cst_test.py
exit /b %errorlevel%
