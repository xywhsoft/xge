@echo off
setlocal
cd /d "%~dp0..\..\.."
python tests\xge\test_3d_motion.py
if errorlevel 1 exit /b 1
python tests\xge\test_3d_retarget.py
if errorlevel 1 exit /b 1
gcc -O2 -Wall -Wextra -Werror -Wno-missing-field-initializers -DXGE_DLL @config/includes.rsp -include build/3d/xge_build_config.h examples/xge/xge_3d_animation/main.c build/3d/xge.lib -lm -o build/3d/xge_3d_animation.exe
exit /b %errorlevel%
