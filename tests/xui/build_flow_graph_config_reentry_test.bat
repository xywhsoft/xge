@echo off
setlocal
set OUT_DIR=build
set OUT=%OUT_DIR%\xui_flow_graph_config_reentry_test.exe
set SRC=tests\xui\xui_flow_graph_config_reentry_test.c src\xui\workflow\xui_flow_graph.c
set INC=@config/includes.rsp
set FLAGS=-O2 -Wall -Wextra -Wno-unused-parameter -Wno-unused-function -Wno-cast-function-type -DXGE_DLL -DXGE_DEBUGMODE=0
set LIBS=%OUT_DIR%\xge.lib -lws2_32 -liphlpapi -lgdi32 -luser32 -lshell32 -lopengl32 -lole32 -lwinmm -lavrt
where gcc >nul 2>nul
if errorlevel 1 exit /b 1
if not exist %OUT_DIR% mkdir %OUT_DIR%
if errorlevel 1 exit /b 1
if not exist %OUT_DIR%\xge.lib (
 call tools\build\build_dll.bat
 if errorlevel 1 exit /b 1
)
gcc %FLAGS% %INC% -o %OUT% %SRC% %LIBS%
if errorlevel 1 exit /b 1
%OUT%
if errorlevel 1 exit /b 1
echo [XUI] Config reentry tests passed: %OUT%
endlocal
