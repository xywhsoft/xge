@echo off
setlocal

if "%ANDROID_NDK_HOME%"=="" (
	echo [XGE] ANDROID_NDK_HOME is not set.
	exit /b 1
)

if "%ANDROID_ABI%"=="" (
	set ABI=arm64-v8a
) else (
	set ABI=%ANDROID_ABI%
)

if "%ANDROID_PLATFORM%"=="" (
	set API=android-18
) else (
	set API=%ANDROID_PLATFORM%
)

set OUT_DIR=build\android\%ABI%

echo [XGE] Configuring Android NativeActivity scaffold...
cmake -S platform\android -B "%OUT_DIR%" ^
	-DCMAKE_TOOLCHAIN_FILE="%ANDROID_NDK_HOME%\build\cmake\android.toolchain.cmake" ^
	-DANDROID_ABI="%ABI%" ^
	-DANDROID_PLATFORM="%API%"
if errorlevel 1 exit /b 1

echo [XGE] Building Android NativeActivity scaffold...
cmake --build "%OUT_DIR%"
if errorlevel 1 exit /b 1

echo [XGE] Build successful: %OUT_DIR%
endlocal
