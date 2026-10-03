@echo off
setlocal
cd /d "%~dp0\.."
set OUT_DIR=build\document
if not exist %OUT_DIR% mkdir %OUT_DIR% || exit /b 1
call xui_document_sources.bat
gcc -std=c11 -O2 -g -Wall -Wextra -Werror -I. -c test_xui\xui_test_xrt_impl.c -o %OUT_DIR%\xrt_footnote_perf.o
if %errorlevel% neq 0 exit /b 1
gcc -std=c11 -O2 -g -Wall -Wextra -Werror -I. -o %OUT_DIR%\xui_document_footnote_perf_test.exe test_xui\xui_document_footnote_perf_test.c %XUI_DOCUMENT_SRC% %OUT_DIR%\xrt_footnote_perf.o -lm -lws2_32 -liphlpapi
if %errorlevel% neq 0 exit /b 1
%OUT_DIR%\xui_document_footnote_perf_test.exe 100000
if %errorlevel% neq 0 exit /b 1
%OUT_DIR%\xui_document_footnote_perf_test.exe 100000 chain
if %errorlevel% neq 0 exit /b 1
%OUT_DIR%\xui_document_footnote_perf_test.exe 100000 prepare-chain
exit /b %errorlevel%
