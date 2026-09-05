@echo off
setlocal
call ensure_xge_dll.bat
if errorlevel 1 exit /b %errorlevel%
gcc -O2 -Wall -Wextra -Werror -Wno-unused-parameter -Wno-unused-function -Wno-cast-function-type -DXGE_DLL -DXGE_DEBUGMODE=0 -I. -o build\xui_date_picker_pixel_test.exe test_xui\xui_date_picker_pixel_test.c test_xui\xui_test_proxy.c build\xge.lib -lm -lws2_32 -liphlpapi -lgdi32 -luser32 -lshell32 -lopengl32 -lole32 -lwinmm -lavrt
if errorlevel 1 exit /b %errorlevel%
build\xui_date_picker_pixel_test.exe
exit /b %errorlevel%
