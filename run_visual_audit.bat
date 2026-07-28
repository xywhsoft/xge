@echo off
setlocal enabledelayedexpansion

set ROOT=%~dp0
pushd "%ROOT%" || exit /b 1

set OUT_DIR=build
set CAPTURE_DIR=artifacts\audit
set PASS_COUNT=0
set FAIL_COUNT=0
set FAIL_LIST=

if not exist "%CAPTURE_DIR%" mkdir "%CAPTURE_DIR%"

echo ============================================
echo  XGE Visual Audit Batch Runner
echo ============================================
echo.

REM --- Build phase ---
echo [PHASE 1] Building audit programs...
echo.

set AUDIT_LIST=audit_shape_basic audit_shape_ex_fill audit_shape_ex_stroke audit_shape_ex_clip_mask audit_shape_ex_blend audit_shape_ex_effect audit_shape_ex_transform audit_svg_render audit_text_render audit_ninepatch_sprite audit_camera_viewport audit_xui_basic audit_xui_text audit_xui_container audit_xui_data audit_xui_overlay audit_xui_feedback audit_xui_complex audit_xui_picker audit_xui_style audit_xui_cache

for %%A in (%AUDIT_LIST%) do (
	if exist "examples\%%A\build.bat" (
		echo   Building %%A...
		call examples\%%A\build.bat >nul 2>&1
		if errorlevel 1 (
			echo   [FAIL] Build %%A failed
			set /a FAIL_COUNT+=1
			set "FAIL_LIST=!FAIL_LIST! %%A(build)"
		) else (
			echo   [OK] %%A built
		)
	) else (
		echo   [SKIP] %%A - no build.bat
	)
)

echo.
echo [PHASE 2] Running captures...
echo.

for %%A in (%AUDIT_LIST%) do (
	if exist "%OUT_DIR%\%%A.exe" (
		echo   Running %%A --capture...
		"%OUT_DIR%\%%A.exe" --frames 2 --capture "%CAPTURE_DIR%\%%A.png" >nul 2>&1
		if errorlevel 1 (
			echo   [FAIL] Capture %%A failed
			set /a FAIL_COUNT+=1
			set "FAIL_LIST=!FAIL_LIST! %%A(capture)"
		) else (
			echo   [OK] %%A captured
			set /a PASS_COUNT+=1
		)
	) else (
		echo   [SKIP] %%A - no exe
	)
)

echo.
echo [PHASE 3] Pixel assertions...
echo.

for %%A in (%AUDIT_LIST%) do (
	if exist "examples\%%A\check_capture.ps1" (
		if exist "%CAPTURE_DIR%\%%A.png" (
			echo   Checking %%A...
			powershell -ExecutionPolicy Bypass -File "examples\%%A\check_capture.ps1" -Path "%CAPTURE_DIR%\%%A.png" >nul 2>&1
			if errorlevel 1 (
				echo   [FAIL] Pixel check %%A failed
				set /a FAIL_COUNT+=1
				set "FAIL_LIST=!FAIL_LIST! %%A(pixels)"
			) else (
				echo   [OK] %%A pixels verified
				set /a PASS_COUNT+=1
			)
		) else (
			echo   [SKIP] %%A - no capture PNG
		)
	) else (
		echo   [SKIP] %%A - no check_capture.ps1
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
