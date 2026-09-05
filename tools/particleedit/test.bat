@echo off
setlocal EnableExtensions
set "TOOL=%~dp0"
call "%TOOL%build.bat"
if errorlevel 1 exit /b 1
pushd "%TOOL%..\.." || exit /b 1
gcc -std=c11 -O2 -Wall -Wextra -Werror -DXGE_DLL -DXGE_DEBUGMODE=0 -I. "%TOOL%tests\test_document.c" "%TOOL%src\pe_document.c" "%TOOL%src\pe_fields.c" -o "%TOOL%release\test_document.exe" build\xge.lib -lm
if errorlevel 1 (
  popd
  exit /b 1
)
"%TOOL%release\test_document.exe" "%TOOL%presets"
if errorlevel 1 (
  popd
  exit /b 1
)
"%TOOL%release\particleedit.exe" --preset 1 --exercise --frames 120
set "RESULT=%errorlevel%"
popd
exit /b %RESULT%
