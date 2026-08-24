@echo off
setlocal

set ROOT=%~dp0..\..
for %%I in ("%ROOT%") do set ROOT=%%~fI
call "%~dp0build.bat"
if errorlevel 1 exit /b 1

"%~dp0release\xui_asset_atlas.exe" ^
	"%ROOT%\res\xui_builtin_atlas.png" ^
	"%ROOT%\src\xui_builtin_atlas.c"
exit /b %errorlevel%
