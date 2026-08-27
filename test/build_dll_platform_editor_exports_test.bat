@echo off
setlocal
if not exist build\xge.dll call build_dll.bat
if %errorlevel% neq 0 exit /b %errorlevel%
gcc -O2 -Wall -Wextra -o build\test_dll_platform_editor_exports.exe test\test_dll_platform_editor_exports.c
if %errorlevel% neq 0 exit /b 1
build\test_dll_platform_editor_exports.exe
if %errorlevel% neq 0 exit /b 1
endlocal
