@echo off
setlocal

set OUT_DIR=build
set OUT=%OUT_DIR%\xge_emoji_render_all_test.exe
set SRC=test\test_emoji_render_all.c
set FLAGS=-O2 -Wall -Wextra -Wno-unused-parameter -DXGE_DLL -DXGE_DEBUGMODE=0
set LIBS=%OUT_DIR%\xge.lib -lm -lws2_32 -liphlpapi -lgdi32 -luser32 -lshell32 -lopengl32 -lole32 -lwinmm -lavrt

if not exist %OUT_DIR% mkdir %OUT_DIR%
gcc %FLAGS% -I. -o %OUT% %SRC% %LIBS%
if %errorlevel% neq 0 exit /b %errorlevel%

%OUT%
exit /b %errorlevel%
