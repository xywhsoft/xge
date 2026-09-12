@echo off
setlocal
pushd "%~dp0.." || exit /b 1
if not exist build mkdir build
gcc -std=c11 -O1 -Wall -Wextra -Wno-unused-parameter -Wno-unused-function -Wno-cast-function-type -DXGE_DEBUGMODE=0 -I. test\test_texture_storage.c -o build\xge_texture_storage_test.exe -lm -lws2_32 -liphlpapi -lgdi32 -luser32 -lshell32 -lole32 -loleaut32 -luuid -limm32 -lwinmm -lavrt
if errorlevel 1 (popd & exit /b 1)
build\xge_texture_storage_test.exe
set RESULT=%errorlevel%
popd
exit /b %RESULT%
