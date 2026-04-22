@echo off
setlocal

if "%XGE_SMOKE_EXE%"=="" (
	set EXE=build\xge_platform_smoke.exe
) else (
	set EXE=%XGE_SMOKE_EXE%
)

if "%XGE_SMOKE_LOG%"=="" (
	set LOG=build\platform_smoke.log
) else (
	set LOG=%XGE_SMOKE_LOG%
)

if "%XGE_SMOKE_FRAMES%"=="" (
	set FRAMES=180
) else (
	set FRAMES=%XGE_SMOKE_FRAMES%
)

if not exist "%EXE%" (
	echo [XGE] %EXE% not found. Run build_platform_smoke_exe.bat first.
	exit /b 1
)

if not exist build (
	mkdir build || exit /b 1
)

echo [XGE] Running %EXE% --frames %FRAMES%
"%EXE%" --frames %FRAMES% > "%LOG%"
if errorlevel 1 (
	echo [XGE] Platform smoke run failed.
	exit /b 1
)

call check_platform_smoke_log.bat "%LOG%" --require-final
exit /b %ERRORLEVEL%
