@echo off
setlocal

set ROOT=%~dp0..\..\..
for %%I in ("%ROOT%") do set ROOT=%%~fI
set TEST_DIR=%~dp0.
for %%I in ("%TEST_DIR%") do set TEST_DIR=%%~fI
set TEST_OUT=%TEST_DIR%\xge_map_test.exe

if not exist "%ROOT%\build\xge.lib" call "%ROOT%\build_dll.bat" || exit /b 1

pushd "%TEST_DIR%" >nul
gcc -std=c11 -O2 -Wall -Wextra -Wconversion -Wsign-conversion -DXGE_DLL ^
  -I"%ROOT%" -I"%ROOT%\tools\mapedit" ^
  -o "%TEST_OUT%" "%TEST_DIR%\test_xge_map.c" "%ROOT%\tools\mapedit\map_sdk\xge_map.c" ^
  "%ROOT%\build\xge.lib"
if errorlevel 1 goto fail
copy /Y "%ROOT%\build\xge.dll" "%TEST_DIR%\xge.dll" >nul || goto fail
"%TEST_OUT%"
if errorlevel 1 goto fail
gcc -std=c11 -O2 -Wall -Wextra -Wconversion -Wsign-conversion -DXGE_DLL -DXUI_DLL ^
  -I"%ROOT%" -I"%ROOT%\tools\mapedit" ^
  -o "%TEST_OUT%" "%TEST_DIR%\test_editor_core.c" ^
  "%ROOT%\tools\mapedit\src\mapedit_history.c" "%ROOT%\tools\mapedit\src\mapedit_document.c" ^
  "%ROOT%\tools\mapedit\src\mapedit_files.c" "%ROOT%\tools\mapedit\src\mapedit_view.c" ^
  "%ROOT%\tools\mapedit\map_sdk\xge_map.c" "%ROOT%\build\xge.lib"
if errorlevel 1 goto fail
"%TEST_OUT%"
if errorlevel 1 goto fail
del /Q "%TEST_OUT%" "%TEST_DIR%\xge.dll" >nul 2>nul
del /Q "%TEST_DIR%\map_sdk_test_*.xson" >nul 2>nul
popd >nul
exit /b 0

:fail
set RET=%ERRORLEVEL%
del /Q "%TEST_OUT%" "%TEST_DIR%\xge.dll" >nul 2>nul
del /Q "%TEST_DIR%\map_sdk_test_*.xson" >nul 2>nul
popd >nul
exit /b %RET%
