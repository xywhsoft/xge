@echo off
call "%~dp0\..\xui_ime_common\build.bat" composition
exit /b %errorlevel%
