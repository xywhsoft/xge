@echo off
setlocal enabledelayedexpansion
cd /d "%~dp0\..\.."

set OUT_DIR=build
set IMG_DIR=D:\GIT\home\host\xge\wwwroot\tutorial\img

if not exist "%IMG_DIR%" mkdir "%IMG_DIR%"

set /a OK=0
set /a FAIL=0
set /a SKIP=0

echo [TUT] Running tutorial capture demos...
echo [TUT] Output: %IMG_DIR%
echo.

for %%F in (%OUT_DIR%\ch*_main*.exe) do (
	set "BASE=%%~nF"
	REM Extract chapter and example number from filename: chXX_mainY
	REM Build output png name: chXX_Y.png
	set "PNGNAME=!BASE:_main=_!"
	set "PNGPATH=%IMG_DIR%\!PNGNAME!.png"

	if exist "!PNGPATH!" (
		echo [SKIP] !BASE! ^(already exists^)
		set /a SKIP+=1
	) else (
		"%%F" --capture "!PNGPATH!" >nul 2>nul
		if errorlevel 1 (
			echo [FAIL] !BASE!
			set /a FAIL+=1
		) else (
			if exist "!PNGPATH!" (
				echo [ OK ] !BASE! -^> !PNGNAME!.png
				set /a OK+=1
			) else (
				echo [FAIL] !BASE! ^(no output file^)
				set /a FAIL+=1
			)
		)
	)
)

echo.
echo [TUT] Capture complete: %OK% new, %SKIP% skipped, %FAIL% failed.
endlocal
