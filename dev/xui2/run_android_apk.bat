@echo off
setlocal

if "%XGE_ANDROID_APK%"=="" (
	set APK=platform\android\gradle\app\build\outputs\apk\debug\app-debug.apk
) else (
	set APK=%XGE_ANDROID_APK%
)
if "%XGE_ANDROID_PACKAGE%"=="" (
	set PKG=dev.xge.smoke
) else (
	set PKG=%XGE_ANDROID_PACKAGE%
)
if "%XGE_ANDROID_ACTIVITY%"=="" (
	set ACTIVITY=android.app.NativeActivity
) else (
	set ACTIVITY=%XGE_ANDROID_ACTIVITY%
)

where adb >nul 2>nul
if errorlevel 1 (
	echo [XGE] adb not found. Install Android platform-tools.
	exit /b 1
)

if not exist "%APK%" (
	echo [XGE] %APK% not found. Run build_android_apk.bat first.
	exit /b 1
)

echo [XGE] Installing %APK%...
adb install -r "%APK%"
if errorlevel 1 exit /b 1

echo [XGE] Starting %PKG%/%ACTIVITY%...
adb shell am start -n "%PKG%/%ACTIVITY%"
if errorlevel 1 exit /b 1
echo [XGE] Android smoke app started. Use adb logcat for device logs.
endlocal
