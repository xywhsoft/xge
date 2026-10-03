@echo off
setlocal
cd /d "%~dp0\.."
set "OUT_DIR=build\shape-paint-oom"
if not exist "%OUT_DIR%" mkdir "%OUT_DIR%"
call xui_sources.bat
call build_text_shaping.bat
if errorlevel 1 exit /b 1
gcc -shared -O2 -Wall -Wextra -Wno-unused-parameter -Wno-unused-function -Wno-cast-function-type -DXGE_DLL -DXGE_BUILD_DLL -DXUI_DLL -DXUI_BUILD_DLL -DBUILD_DLL -DXGE_DEBUGMODE=0 -DXRT_MODULE_MEMORY_DEBUG -DXGE_ENABLE_HARFBUZZ -I. "-Wl,--out-implib,%OUT_DIR%\xge.lib" -o "%OUT_DIR%\xge.dll" xge.c %XUI_SRC% build\text\harfbuzz.o -lm -lws2_32 -liphlpapi -lgdi32 -luser32 -lshell32 -lole32 -loleaut32 -luuid -limm32 -lwinmm -lavrt
if errorlevel 1 exit /b 1
gcc -std=c11 -O2 -Wall -Wextra -Werror -DXGE_DLL -DXUI_DLL -DXRT_MODULE_MEMORY_DEBUG -I. -o "%OUT_DIR%\xui_text_shape_paint_oom_test.exe" test_xui\xui_text_shape_paint_oom_test.c "%OUT_DIR%\xge.lib" -lm
if errorlevel 1 exit /b 1
"%OUT_DIR%\xui_text_shape_paint_oom_test.exe"
exit /b %errorlevel%
