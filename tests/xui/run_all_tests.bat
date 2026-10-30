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

REM WebView2 is an optional backend. Build its shared test profile before the
REM alphabetically earlier document tests, or report those tests as skipped.
set "WEBVIEW_SDK=%WEBVIEW2_SDK_DIR%"
if not defined WEBVIEW_SDK set "WEBVIEW_SDK=artifacts\xui-webview-probe\sdk"
set "WEBVIEW_AVAILABLE=0"
if exist "%WEBVIEW_SDK%\build\native\include\WebView2.h" set "WEBVIEW_AVAILABLE=1"
if "!WEBVIEW_AVAILABLE!"=="1" (
	echo [SETUP] Building WebView2 test profile...
	call tests\xui\build_webview_widget_test.bat >nul 2>&1
	if errorlevel 1 (
		echo [FAIL] WebView2 test profile setup
		popd
		exit /b 1
	)
)

REM Build and run each test
for %%F in (tests\xui\build_*_test.bat) do (
	set "TNAME=%%~nF"
	set "TNAME=!TNAME:build_=!"
	set "SKIP_REASON="
	if "!QUICK!"=="1" if /I "!TNAME!"=="document_cst_test" set "SKIP_REASON=extended"
	if "!WEBVIEW_AVAILABLE!"=="0" (
		for %%W in (
			document_html_interaction_test document_web_provider_test document_web_render_test
			webview_basic_test webview_capture_test webview_exports_test webview_local_test
			webview_script_test webview_scroll_test webview_widget_test
		) do if /I "!TNAME!"=="%%W" set "SKIP_REASON=WebView2 SDK unavailable"
	)
	if defined SKIP_REASON (
		echo   [SKIP] !TNAME! ^(!SKIP_REASON!^)
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
