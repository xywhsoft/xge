@echo off
setlocal
pushd "%~dp0.." || exit /b 1
if not exist build\xge.lib (call build_dll.bat || (popd & exit /b 1))
gcc -std=c11 -O2 -Wall -Wextra -Werror -DXGE_DLL -I. test\particle_bench.c -o build\xge_particle_bench.exe build\xge.lib -lm
if errorlevel 1 (popd & exit /b 1)
build\xge_particle_bench.exe
set RESULT=%errorlevel%
popd
exit /b %RESULT%
