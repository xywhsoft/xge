@echo off
setlocal
call tests\xui\build_document_test.bat
if %errorlevel% neq 0 exit /b 1
call tools/build/xui_document_sources.bat
gcc -std=c11 -O2 -Wall -Wextra -Werror @config/includes.rsp -o build\document\xui_document_corpus_test.exe tests\xui\xui_document_corpus_test.c %XUI_DOCUMENT_SRC% build\document\xrt.o -lm -lws2_32 -liphlpapi
if %errorlevel% neq 0 exit /b 1
build\document\xui_document_corpus_test.exe %*
exit /b %errorlevel%
