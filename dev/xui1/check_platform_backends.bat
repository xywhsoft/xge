@echo off
setlocal

echo [XGE] Platform backend toolchain check

call :check gcc "Windows MinGW build"
call :check cmake "Android NDK CMake build"
call :check gradle "Android APK build"
call :check adb "Android device install/run"
call :check emcc "Web/Emscripten build"
call :check python "Web local HTTP smoke server"

if "%ANDROID_HOME%"=="" if "%ANDROID_SDK_ROOT%"=="" (
	echo [MISS] ANDROID_HOME or ANDROID_SDK_ROOT is not set
) else (
	if not "%ANDROID_HOME%"=="" echo [ OK ] ANDROID_HOME=%ANDROID_HOME%
	if not "%ANDROID_SDK_ROOT%"=="" echo [ OK ] ANDROID_SDK_ROOT=%ANDROID_SDK_ROOT%
)

if "%ANDROID_NDK_HOME%"=="" (
	echo [MISS] ANDROID_NDK_HOME is not set
) else (
	echo [ OK ] ANDROID_NDK_HOME=%ANDROID_NDK_HOME%
)

if "%JAVA_HOME%"=="" (
	echo [MISS] JAVA_HOME is not set
) else (
	echo [ OK ] JAVA_HOME=%JAVA_HOME%
)

echo [XGE] Done.
exit /b 0

:check
where %~1 >nul 2>nul
if errorlevel 1 (
	echo [MISS] %~1 - %~2
) else (
	echo [ OK ] %~1 - %~2
)
exit /b 0
