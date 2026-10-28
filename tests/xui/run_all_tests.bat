@echo off
setlocal enabledelayedexpansion

set ROOT=%~dp0..\..
pushd "%ROOT%" || exit /b 1

set OUT_DIR=build
set PASS_COUNT=0
set FAIL_COUNT=0
set SKIP_COUNT=0
set FAIL_LIST=
set TEST_COUNT=0
set QUICK=0
if /I "%~1"=="--quick" set QUICK=1
for %%F in (tests\xui\build_*_test.bat) do set /a TEST_COUNT+=1

echo ============================================
echo  XUI Logic Test Runner (!TEST_COUNT! tests)
echo ============================================
echo.

REM Ensure xge.dll is built
if not exist "%OUT_DIR%\xge.lib" (
	echo [SETUP] Building xge.dll...
	call tools\build\build_dll.bat
	if errorlevel 1 ( popd & exit /b 1 )
)

REM Build and run each test
for %%F in (tests\xui\build_*_test.bat) do (
	set "TNAME=%%~nF"
	set "TNAME=!TNAME:build_=!"
	if "!QUICK!"=="1" if /I "!TNAME!"=="document_cst_test" (
		echo   [SKIP] !TNAME! ^(extended^)
		set /a SKIP_COUNT+=1
	) else (
		echo   [RUN] !TNAME!...
		call %%F >nul 2>&1
		if errorlevel 1 (
			echo   [RETRY] !TNAME!...
			ping 127.0.0.1 -n 2 >nul
			call %%F >nul 2>&1
			if errorlevel 1 (
				echo   [FAIL] !TNAME!
				set /a FAIL_COUNT+=1
				set "FAIL_LIST=!FAIL_LIST! !TNAME!"
			) else (
				set /a PASS_COUNT+=1
			)
		) else (
			set /a PASS_COUNT+=1
		)
	)
)

echo.
echo ============================================
echo  RESULTS: %PASS_COUNT% passed, %FAIL_COUNT% failed, %SKIP_COUNT% skipped
if defined FAIL_LIST (
	echo  FAILURES:!FAIL_LIST!
)
echo ============================================

popd
if %FAIL_COUNT% gtr 0 (endlocal & exit /b 1)
endlocal
exit /b 0
