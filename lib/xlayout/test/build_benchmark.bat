@echo off
setlocal

set ROOT=%~dp0..\..\..
set OUT=%~dp0xlayout_benchmark.exe
set GCC=E:\software\w64devkit\bin\gcc.exe

"%GCC%" -std=c11 -O2 -Wall -Wextra -Wno-unused-function -Wno-cast-function-type -I"%ROOT%" ^
	-o "%OUT%" "%~dp0benchmark_xlayout.c" "%~dp0xlayout_test_xrt.c" "%ROOT%\lib\xlayout\xlayout.c" ^
	-lws2_32 -liphlpapi -lgdi32 -luser32 -lshell32 -lole32 -lwinmm -lavrt
if errorlevel 1 exit /b %errorlevel%

"%OUT%"
exit /b %errorlevel%
