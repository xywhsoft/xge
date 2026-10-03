@echo off
setlocal
cd /d "%~dp0..\.."
set "PROFILE=%~1"
if not defined PROFILE set "PROFILE=3d"
python tools\build_profile.py %PROFILE%
if errorlevel 1 exit /b 1
gcc -O2 -Wall -Wextra -Wno-missing-field-initializers -DXGE_DLL -I. -include build/%PROFILE%/xge_build_config.h examples/xge_3d/main.c build/%PROFILE%/xge.lib -lm -o build/%PROFILE%/xge_3d.exe
exit /b %errorlevel%
