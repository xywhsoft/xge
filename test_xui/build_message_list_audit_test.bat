@echo off
setlocal
set OUT_DIR=build
set OUT=%OUT_DIR%\xui_message_list_audit_test.exe
rem Shared text and Document private helpers must be in-process. Use the
rem release manifest rather than a partial list that drifts with Document.
call xui_sources.bat
rem The audited MessageList is included by the test. Only the native rendering
rem proxy stays in the DLL. Keep browser stubs local for Document private calls.
set "XUI_SRC=%XUI_SRC:src\xui_message_list.c=%"
set "XUI_SRC=%XUI_SRC:src\xui_proxy_xge.c=%"
set SRC=test_xui\xui_message_list_audit_test.c test_xui\xui_test_proxy.c test_xui\xui_test_xrt_impl.c %XUI_SRC%
set FLAGS=-O2 -g -Wall -Wextra -Wno-unused-parameter -Wno-unused-function -Wno-cast-function-type -DXGE_DEBUGMODE=0 %MESSAGE_LIST_TEST_FLAGS%
set LIBS=build\xge.lib -lm -lws2_32 -liphlpapi -lgdi32 -luser32 -lshell32 -lole32 -loleaut32 -luuid -limm32 -lwinmm -lavrt
call ensure_xge_dll.bat
if errorlevel 1 exit /b 1
if not exist %OUT_DIR% mkdir %OUT_DIR% || exit /b 1
gcc %FLAGS% -I. -o %OUT% %SRC% %LIBS%
if errorlevel 1 exit /b 1
set PATH=%CD%\build;%PATH%
%OUT% %*
exit /b %errorlevel%
