@echo off
setlocal
if not exist build\document mkdir build\document
call xui_document_sources.bat
gcc -std=c11 -O2 -g -Wall -Wextra -Werror -I. -c test_xui\xui_test_xrt_impl.c -o build\document\xrt.o
if %errorlevel% neq 0 exit /b 1
gcc -std=c11 -O2 -g -Wall -Wextra -Werror -I. -o build\document\xui_document_attribute_pool_scale_test.exe test_xui\xui_document_attribute_pool_scale_test.c %XUI_DOCUMENT_SRC% build\document\xrt.o -lm -lws2_32 -liphlpapi
if %errorlevel% neq 0 exit /b 1
build\document\xui_document_attribute_pool_scale_test.exe
exit /b %errorlevel%
