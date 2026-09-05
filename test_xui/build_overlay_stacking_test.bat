@echo off
setlocal
pushd "%~dp0.." || exit /b 1
call ensure_xge_dll.bat
if errorlevel 1 (popd & exit /b 1)
gcc -O2 -Wall -Wextra -Wno-unused-parameter -Wno-unused-function -Wno-cast-function-type -DXGE_DLL -DXGE_DEBUGMODE=0 -I. -o build\xui_overlay_stacking_test.exe test_xui\xui_overlay_stacking_test.c build\xge.lib -lws2_32 -liphlpapi -lgdi32 -luser32 -lshell32 -lopengl32 -lole32 -lwinmm -lavrt
if errorlevel 1 (popd & exit /b 1)
build\xui_overlay_stacking_test.exe
set RESULT=%errorlevel%
popd
exit /b %RESULT%
