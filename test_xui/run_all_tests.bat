@echo off
setlocal enabledelayedexpansion

set ROOT=%~dp0..
pushd "%ROOT%" || exit /b 1

set OUT_DIR=build
set PASS_COUNT=0
set FAIL_COUNT=0
set FAIL_LIST=

echo ============================================
echo  XUI Logic Test Runner (99 tests)
echo ============================================
echo.

REM Ensure xge.dll is built
if not exist "%OUT_DIR%\xge.lib" (
	echo [SETUP] Building xge.dll...
	call build_dll.bat
	if errorlevel 1 ( popd & exit /b 1 )
)

REM Build and run each test
for %%F in (test_xui\build_*_test.bat) do (
	set "TNAME=%%~nF"
	set "TNAME=!TNAME:build_=!"
	echo   [RUN] !TNAME!...
	call %%F >nul 2>&1
	if errorlevel 1 (
		echo   [FAIL] !TNAME!
		set /a FAIL_COUNT+=1
		set "FAIL_LIST=!FAIL_LIST! !TNAME!"
	) else (
		set /a PASS_COUNT+=1
	)
)

echo.
echo ============================================
echo  RESULTS: %PASS_COUNT% passed, %FAIL_COUNT% failed
if defined FAIL_LIST (
	echo  FAILURES:!FAIL_LIST!
)
echo ============================================

popd
if %FAIL_COUNT% gtr 0 (endlocal & exit /b 1)
endlocal
exit /b 0
