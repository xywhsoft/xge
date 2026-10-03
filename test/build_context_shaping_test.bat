@echo off
setlocal
cd /d "%~dp0\.."
call ensure_xge_dll.bat
if errorlevel 1 exit /b 1
gcc -std=c11 -O2 -Wall -Wextra -Werror -I. test\test_text_context.c -o build\text_context_contract.exe
if errorlevel 1 exit /b 1
build\text_context_contract.exe
if errorlevel 1 exit /b 1
gcc -std=c11 -O2 -Wall -Wextra -Werror -DXGE_DLL -I. test\test_context_shaping.c build\xge.lib build\text\harfbuzz.o -o build\xge_context_shaping_test.exe -lm
if errorlevel 1 exit /b 1
build\xge_context_shaping_test.exe
if errorlevel 1 exit /b 1
rem Explicit language must not be guessed from the process locale.
set LANG=tr_TR.UTF-8
set LC_ALL=tr_TR.UTF-8
build\xge_context_shaping_test.exe
exit /b %errorlevel%
