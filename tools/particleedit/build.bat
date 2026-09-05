@echo off
setlocal EnableExtensions
set "TOOL=%~dp0"
pushd "%TOOL%..\.." || exit /b 1
where gcc >nul 2>nul
if errorlevel 1 (
  echo [particleedit] gcc is required in PATH.
  popd
  exit /b 1
)
call ensure_xge_dll.bat
if errorlevel 1 (
  popd
  exit /b 1
)
if not exist "%TOOL%release" mkdir "%TOOL%release"
gcc -std=c11 -O2 -Wall -Wextra -Werror -DXGE_DLL -DXUI_DLL -DXGE_DEBUGMODE=0 -I. "%TOOL%src\main.c" "%TOOL%src\pe_document.c" "%TOOL%src\pe_fields.c" "%TOOL%src\pe_preview.c" "%TOOL%src\pe_ui.c" "%TOOL%src\pe_graph.c" "%TOOL%src\pe_exercise.c" -o "%TOOL%release\particleedit.exe" build\xge.lib -lm -lcomdlg32 -lshell32 -luser32
if errorlevel 1 (
  popd
  exit /b 1
)
copy /Y build\xge.dll "%TOOL%release\xge.dll" >nul
if errorlevel 1 (
  popd
  exit /b 1
)
echo [particleedit] Built %TOOL%release\particleedit.exe
popd
exit /b 0
