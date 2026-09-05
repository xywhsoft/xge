@echo off
setlocal
pushd "%~dp0.." || exit /b 1
if not exist build mkdir build
set FLAGS=-std=c11 -O1 -g -Wall -Wextra -Werror -DXRT_MODULE_MEMORY_DEBUG -I.
gcc %FLAGS% test\test_particle.c test\particle_xrt_impl.c src\xge_particle.c src\xge_particle_io.c -o build\xge_particle_test.exe -lm -lws2_32 -liphlpapi
if errorlevel 1 (popd & exit /b 1)
build\xge_particle_test.exe
set RESULT=%errorlevel%
popd
exit /b %RESULT%
