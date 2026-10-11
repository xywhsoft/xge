@echo off
setlocal

set OUT_DIR=build
set OUT=%OUT_DIR%\xui_text_pixel_alignment_test.exe

if not exist %OUT_DIR% mkdir %OUT_DIR% || exit /b 1
call ensure_xge_dll.bat
if errorlevel 1 exit /b %errorlevel%

gcc -O2 -Wall -Wextra -Wno-missing-field-initializers -DXGE_DLL -I. ^
    -o %OUT% test_xui\xui_text_pixel_alignment_test.c %OUT_DIR%\xge.lib ^
    -lws2_32 -liphlpapi -lgdi32 -luser32 -lshell32 -lopengl32 -lole32 -lwinmm -lavrt
if errorlevel 1 exit /b %errorlevel%

%OUT%
if errorlevel 1 exit /b %errorlevel%

endlocal
