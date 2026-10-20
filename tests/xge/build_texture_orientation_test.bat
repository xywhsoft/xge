@echo off
setlocal
pushd "%~dp0..\.." || exit /b 1
if not defined CC set "CC=gcc"
python tools\build\build.py full-dev
if errorlevel 1 (popd & exit /b 1)
set FLAGS=-std=c11 -O2 -Wall -Wextra -Werror -Wno-misleading-indentation -Wno-missing-field-initializers -DXGE_DLL -DXUI_DLL -include build/full-dev/xge_build_config.h @config/includes.rsp
"%CC%" %FLAGS% tests\xge\test_texture_orientation.c build\full-dev\xge.lib -lm -o build\full-dev\test_texture_orientation.exe
if errorlevel 1 (popd & exit /b 1)
build\full-dev\test_texture_orientation.exe
if errorlevel 1 (popd & exit /b 1)
"%CC%" %FLAGS% -DXGE_ORIENTATION_NEGATIVE_CONTROL tests\xge\test_texture_orientation.c build\full-dev\xge.lib -lm -o build\full-dev\test_texture_orientation_negative.exe
if errorlevel 1 (popd & exit /b 1)
build\full-dev\test_texture_orientation_negative.exe > build\full-dev\orientation-negative.log 2>&1
if not errorlevel 1 (echo ERROR: negative control unexpectedly passed & popd & exit /b 1)
findstr /c:"kind=1 mode=0 crop=0 flip=0" build\full-dev\orientation-negative.log >nul
if errorlevel 1 (echo ERROR: negative control failed for an unrelated reason & popd & exit /b 1)
echo PASS negative control: missing texture origin detected by rendered pixels
popd
exit /b 0
