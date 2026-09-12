@echo off
setlocal
pushd "%~dp0.." || exit /b 1
if not exist build\xge.lib (call build_dll.bat || (popd & exit /b 1))
gcc -std=c11 -O2 -Wall -Wextra -Werror -DXGE_DLL -DXUI_DLL -I. test\test_texture_storage_render.c -o build\xge_texture_storage_render_test.exe build\xge.lib -lm
if errorlevel 1 (popd & exit /b 1)
build\xge_texture_storage_render_test.exe
set RESULT=%errorlevel%
popd
exit /b %RESULT%
