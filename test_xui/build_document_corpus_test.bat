@echo off
setlocal
call test_xui\build_document_test.bat
if %errorlevel% neq 0 exit /b 1
call xui_document_sources.bat
gcc -std=c11 -O2 -Wall -Wextra -Werror -I. -o build\document\xui_document_corpus_test.exe test_xui\xui_document_corpus_test.c %XUI_DOCUMENT_SRC% build\document\xrt.o -lm -lws2_32 -liphlpapi
if %errorlevel% neq 0 exit /b 1
build\document\xui_document_corpus_test.exe %*
exit /b %errorlevel%
