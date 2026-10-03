@echo off
setlocal
cd /d "%~dp0\.."
call ensure_xge_dll.bat
if errorlevel 1 exit /b 1
gcc -std=c11 -O2 -Wall -Wextra -Werror -I. test\test_unicode_joining.c -o build\unicode_joining_test.exe
if errorlevel 1 exit /b 1
build\unicode_joining_test.exe
if errorlevel 1 exit /b 1
python test\verify_unicode_joining.py build\unicode_joining_test.exe
if errorlevel 1 exit /b 1
g++ -std=c++17 -O2 -Wall -Wextra -Werror test\test_hb_joining_transparency.cc build\text\harfbuzz.o -o build\hb_joining_transparency_test.exe -lm
if errorlevel 1 exit /b 1
build\hb_joining_transparency_test.exe
if errorlevel 1 exit /b 1
gcc -std=c11 -O2 -Wall -Wextra -Werror -I. test\test_joining_context_portability.c build\text\harfbuzz.o -o build\joining_context_portable_test.exe -lm
if errorlevel 1 exit /b 1
build\joining_context_portable_test.exe
if errorlevel 1 exit /b 1
gcc -std=c11 -O2 -Wall -Wextra -Werror -DXGE_DLL -I. test\test_joining_context.c build\xge.lib build\text\harfbuzz.o -o build\xge_joining_context_test.exe -lm
if errorlevel 1 exit /b 1
build\xge_joining_context_test.exe
exit /b %errorlevel%
