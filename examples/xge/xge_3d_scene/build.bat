@echo off
setlocal
cd /d "%~dp0..\..\.."
set "PROFILE=%~1"
if not defined PROFILE set "PROFILE=3d"
python tools\build\build.py %PROFILE%
if errorlevel 1 exit /b 1
python tests\xge\make_3d_fixtures.py
if errorlevel 1 exit /b 1
gcc -O2 -Wall -Wextra -Werror -Wno-missing-field-initializers -DXGE_DLL @config/includes.rsp -include build/%PROFILE%/xge_build_config.h examples/xge/xge_3d_scene/main.c build/%PROFILE%/xge.lib -lm -o build/%PROFILE%/xge_3d_scene.exe
exit /b %errorlevel%
