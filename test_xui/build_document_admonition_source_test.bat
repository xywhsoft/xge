@echo off
setlocal
call ensure_xge_dll.bat
if errorlevel 1 exit /b 1
set OUT_DIR=build\document
if not exist %OUT_DIR% mkdir %OUT_DIR% || exit /b 1
call xui_document_sources.bat
gcc -std=c11 -O2 -g -Wall -Wextra -Werror -I. -c test_xui\xui_test_xrt_impl.c -o %OUT_DIR%\admonition-source-xrt.o
if errorlevel 1 exit /b 1
gcc -std=c11 -O2 -g -Wall -Wextra -Werror -Wno-unused-function -I. test_xui\xui_document_admonition_source_test.c %XUI_DOCUMENT_SRC% %OUT_DIR%\admonition-source-xrt.o -lm -lws2_32 -liphlpapi -o %OUT_DIR%\xui_document_admonition_source_test.exe
if errorlevel 1 exit /b 1
%OUT_DIR%\xui_document_admonition_source_test.exe
if errorlevel 1 exit /b 1
gcc -std=c11 -O2 -g -Wall -Wextra -Werror -Wno-unused-function -I. -DXGE_DLL -DXUI_DLL -DXUI_QUOTE_TEST_EDITOR test_xui\xui_document_admonition_source_test.c test_xui\xui_test_proxy.c src\xui_unicode.c build\xge.lib -lm -lws2_32 -liphlpapi -lgdi32 -luser32 -lshell32 -lole32 -lwinmm -lavrt -o %OUT_DIR%\xui_document_admonition_source_dll_test.exe
if errorlevel 1 exit /b 1
set PATH=%CD%\build;%PATH%
%OUT_DIR%\xui_document_admonition_source_dll_test.exe
exit /b %errorlevel%
