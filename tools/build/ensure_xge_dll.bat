@echo off
setlocal
pushd "%~dp0\..\.." >nul || exit /b 1

call build.bat
if errorlevel 1 (
	popd
	exit /b 1
)

rem A local DLL beside a Document test takes precedence over PATH. Refresh
rem existing copies so these tests cannot silently load an earlier build.
if exist build\document\xge.dll (
	copy /Y build\xge.dll build\document\xge.dll >nul
	if errorlevel 1 (
		popd
		exit /b 1
	)
)

popd
exit /b 0
