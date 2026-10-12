@echo off
setlocal

where gradle >nul 2>nul
if errorlevel 1 (
	echo [XGE] gradle not found. Install Gradle or use Android Studio.
	exit /b 1
)

if "%ANDROID_HOME%"=="" if "%ANDROID_SDK_ROOT%"=="" (
	echo [XGE] ANDROID_HOME or ANDROID_SDK_ROOT is not set.
	exit /b 1
)

echo [XGE] Building Android debug APK...
pushd platform\android\gradle || exit /b 1
gradle assembleDebug
set ERR=%ERRORLEVEL%
popd
if not "%ERR%"=="0" exit /b %ERR%
echo [XGE] Build successful: platform\android\gradle\app\build\outputs\apk\debug\app-debug.apk
endlocal
