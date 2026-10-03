@echo off
setlocal
cd /d "%~dp0..\.."
python test\build_3d_suite.py ibl 3d
if errorlevel 1 exit /b 1
gcc -O2 -Wall -Wextra -Werror -Wno-missing-field-initializers -DXGE_DLL -I. -include build/3d/xge_build_config.h examples/xge_3d_lighting/main.c build/3d/xge.lib -lm -o build/3d/xge_3d_lighting.exe
exit /b %errorlevel%
