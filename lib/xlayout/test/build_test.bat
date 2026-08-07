@echo off
setlocal

set ROOT=%~dp0..\..\..
set OUT=%~dp0xlayout_test.exe
set TCC=D:\GIT\tcc_xlang\win32\dist\x86_64\tcc.exe
set TCC_BASE=D:\GIT\x-lang\demo6\res\tcc\win64\x86_64

if not exist "%TCC%" (
	echo [ERROR] TCC not found: %TCC%
	exit /b 1
)

"%TCC%" -m64 -I"%ROOT%" -I"%TCC_BASE%\include" -I"%TCC_BASE%\include\winapi" -L"%TCC_BASE%\lib" ^
	-o "%OUT%" "%~dp0test_xlayout.c" "%~dp0xlayout_test_xrt.c" "%ROOT%\lib\xlayout\xlayout.c" ^
	-lkernel32 -luser32 -ladvapi32 -lshell32 -lole32 -lws2_32 -liphlpapi
if errorlevel 1 exit /b %errorlevel%

"%OUT%"
exit /b %errorlevel%
