@echo off
setlocal EnableDelayedExpansion

set ROOT=%~dp0..\..
for %%I in ("%ROOT%") do set ROOT=%%~fI
set TOOL_DIR=%~dp0.
for %%I in ("%TOOL_DIR%") do set TOOL_DIR=%%~fI
set TEST_DIR=%TEMP%\xui_uidesign_test_%RANDOM%_%RANDOM%
for %%I in ("%TEST_DIR%") do set TEST_DIR=%%~fI
set TEST_EXE=%TEST_DIR%\test_uidesign.exe
set GENERATED_TEST_EXE=%TEST_DIR%\test_generated_view.exe
set SESSION_TEST_EXE=%TEST_DIR%\test_sessions.exe
set FIXTURE=%TEST_DIR%\roundtrip.json
set GENERATED=%TEST_DIR%\milestone_view.c
set GENERATED_CLI=%TEST_DIR%\cli_view.c
set XGE_LIB=%ROOT%\build\xge.lib
set INC=-I"%ROOT%" -I"%TOOL_DIR%\src"
set FLAGS=-std=c11 -O2 -Wall -Wextra -Wformat=2 -Werror=format -Wno-unused-parameter -DXGE_DLL -DXUI_DLL
set LIBS="%XGE_LIB%" -lm -lws2_32 -liphlpapi -lgdi32 -luser32 -lshell32 -lole32 -lwinmm -lavrt
set RESULT=0

where gcc >nul 2>nul
if errorlevel 1 (
	echo [UIDESIGN-TEST] gcc not found in PATH
	exit /b 1
)
call "%TOOL_DIR%\build.bat"
if errorlevel 1 exit /b 1
mkdir "%TEST_DIR%" || exit /b 1

echo [UIDESIGN-TEST] Building unit tests...
pushd "%ROOT%" >nul
gcc %FLAGS% %INC% -o "%TEST_EXE%" "%TOOL_DIR%\tests\test_uidesign.c" "%TOOL_DIR%\src\ui_design_model.c" "%TOOL_DIR%\src\ui_design_document.c" "%TOOL_DIR%\src\ui_design_codegen.c" %LIBS%
set RESULT=!ERRORLEVEL!
popd >nul
if not "!RESULT!"=="0" goto cleanup
copy /Y "%ROOT%\build\xge.dll" "%TEST_DIR%\xge.dll" >nul
if errorlevel 1 (
	set RESULT=1
	goto cleanup
)

"%TEST_EXE%" "%FIXTURE%" "%GENERATED%"
set RESULT=!ERRORLEVEL!
if not "!RESULT!"=="0" goto cleanup

echo [UIDESIGN-TEST] Checking session state and memory budgets...
gcc %FLAGS% %INC% -o "%SESSION_TEST_EXE%" "%TOOL_DIR%\tests\test_sessions.c" "%TOOL_DIR%\src\ui_design_model.c" "%TOOL_DIR%\src\ui_design_document.c" "%TOOL_DIR%\src\ui_design_session.c" %LIBS%
set RESULT=!ERRORLEVEL!
if not "!RESULT!"=="0" goto cleanup
"%SESSION_TEST_EXE%" "%TEST_DIR%"
set RESULT=!ERRORLEVEL!
if not "!RESULT!"=="0" goto cleanup

echo [UIDESIGN-TEST] Checking generated source...
pushd "%TEST_DIR%" >nul
gcc -std=c11 -O2 -Wall -Wextra -Werror -DXGE_DLL -DXUI_DLL -I"%ROOT%" -I"%TEST_DIR%" -o "%GENERATED_TEST_EXE%" "%TOOL_DIR%\tests\test_generated_view.c" "%GENERATED%" %LIBS%
set RESULT=!ERRORLEVEL!
popd >nul
if not "!RESULT!"=="0" goto cleanup
"%GENERATED_TEST_EXE%"
set RESULT=!ERRORLEVEL!
if not "!RESULT!"=="0" goto cleanup

echo [UIDESIGN-TEST] Checking CLI generation and runtime smoke tests...
"%TOOL_DIR%\release\xui_uidesign.exe" --generate "%FIXTURE%" "%GENERATED_CLI%"
set RESULT=!ERRORLEVEL!
if not "!RESULT!"=="0" goto cleanup
pushd "%TEST_DIR%" >nul
gcc -std=c11 -Wall -Wextra -Werror -DXGE_DLL -DXUI_DLL -I"%ROOT%" -fsyntax-only "%GENERATED_CLI%"
set RESULT=!ERRORLEVEL!
popd >nul
if not "!RESULT!"=="0" goto cleanup
"%TOOL_DIR%\release\xui_uidesign.exe" --frames 2
set RESULT=!ERRORLEVEL!
if not "!RESULT!"=="0" goto cleanup
"%TOOL_DIR%\release\xui_uidesign.exe" --preview "%FIXTURE%" --frames 2
set RESULT=!ERRORLEVEL!
if not "!RESULT!"=="0" goto cleanup
"%TOOL_DIR%\release\xui_uidesign.exe" --exercise --frames 12
set RESULT=!ERRORLEVEL!
if not "!RESULT!"=="0" goto cleanup
"%TOOL_DIR%\release\xui_uidesign.exe" --workbench-exercise --workspace "%TEST_DIR%\workspace.xson" --frames 3
set RESULT=!ERRORLEVEL!
if not "!RESULT!"=="0" goto cleanup
"%TOOL_DIR%\release\xui_uidesign.exe" --workspace "%TEST_DIR%\workspace.xson" --frames 2
set RESULT=!ERRORLEVEL!
if not "!RESULT!"=="0" goto cleanup
"%TOOL_DIR%\release\xui_uidesign.exe" --seconds 2
set RESULT=!ERRORLEVEL!
if not "!RESULT!"=="0" goto cleanup
powershell -NoProfile -ExecutionPolicy Bypass -File "%TOOL_DIR%\tools\sdk_manifest.ps1" -Verify
set RESULT=!ERRORLEVEL!

:cleanup
powershell -NoProfile -ExecutionPolicy Bypass -Command "$p=[IO.Path]::GetFullPath('%TEST_DIR%'); $prefix=[IO.Path]::GetFullPath([IO.Path]::Combine($env:TEMP,'xui_uidesign_test_')); if (-not $p.StartsWith($prefix,[StringComparison]::OrdinalIgnoreCase)) { exit 2 }; if (Test-Path -LiteralPath $p) { Remove-Item -LiteralPath $p -Recurse -Force }"
if not "%RESULT%"=="0" (
	echo [UIDESIGN-TEST] Failed with code %RESULT%
	exit /b %RESULT%
)
echo [UIDESIGN-TEST] All checks passed.
endlocal
