@echo off
setlocal
set OUT_DIR=build\document
if not exist %OUT_DIR% mkdir %OUT_DIR% || exit /b 1
call tools/build/xui_document_sources.bat
set SRC=%XUI_DOCUMENT_SRC%
gcc -std=c11 -O2 -g -Wall -Wextra -Werror @config/includes.rsp -c tests\xui\xui_test_xrt_impl.c -o %OUT_DIR%\xrt.o
if %errorlevel% neq 0 exit /b 1
gcc -std=c11 -O2 -g -Wall -Wextra -Werror @config/includes.rsp -o %OUT_DIR%\xui_document_test.exe tests\xui\xui_document_test.c %SRC% %OUT_DIR%\xrt.o -lm -lws2_32 -liphlpapi
if %errorlevel% neq 0 exit /b 1
%OUT_DIR%\xui_document_test.exe
exit /b %errorlevel%
