@echo off
setlocal

set OUT_DIR=build
set OUT=%OUT_DIR%\xui_file_dialog_test.exe
call tools\build\xui_sources.bat
set "XUI_SRC=%XUI_SRC:src\xui\core\xui_proxy_xge.c=%"
set SRC=tests\xui\xui_file_dialog_test.c tests\xui\xui_test_proxy.c tests\xui\xui_test_xrt_impl.c %XUI_SRC%
set INC=@config/includes.rsp
set FLAGS=-O2 -Wall -Wextra -Wno-unused-parameter -Wno-unused-function -Wno-cast-function-type -DXGE_DEBUGMODE=0 -DXUI_FILE_DIALOG_TEST=1
set LIBS=build\xge.lib -lm -lws2_32 -liphlpapi -lgdi32 -luser32 -lshell32 -lopengl32 -lole32 -loleaut32 -luuid -limm32 -lwinmm -lavrt

where gcc >nul 2>nul
if %errorlevel% neq 0 (
	echo [ERROR] gcc not found in PATH
	exit /b 1
)

if not exist %OUT_DIR% (
	mkdir %OUT_DIR% || exit /b 1
)

call tools\build\ensure_xge_dll.bat
if %errorlevel% neq 0 exit /b %errorlevel%

echo [XUI] Building xui_file_dialog_test...
gcc %FLAGS% %INC% -o %OUT% %SRC% %LIBS%
if %errorlevel% neq 0 (
	echo [XUI] Build failed
	exit /b 1
)

%OUT%
if %errorlevel% neq 0 (
	echo [XUI] Tests failed
	exit /b 1
)

echo [XUI] Tests passed: %OUT%
endlocal
