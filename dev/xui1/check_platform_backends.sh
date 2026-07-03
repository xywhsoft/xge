#!/bin/sh

echo "[XGE] Platform backend toolchain check"

check_tool() {
	if command -v "$1" >/dev/null 2>&1; then
		echo "[ OK ] $1 - $2"
	else
		echo "[MISS] $1 - $2"
	fi
}

check_tool cc "Linux/macOS native build"
check_tool gcc "Linux native build"
check_tool clang "macOS/iOS native build"
check_tool cmake "Android NDK CMake build"
check_tool gradle "Android APK build"
check_tool adb "Android device install/run"
check_tool emcc "Web/Emscripten build"
check_tool python3 "Web local HTTP smoke server"
check_tool xcrun "iOS simulator build"

if [ -n "${ANDROID_HOME:-}" ]; then
	echo "[ OK ] ANDROID_HOME=$ANDROID_HOME"
elif [ -n "${ANDROID_SDK_ROOT:-}" ]; then
	echo "[ OK ] ANDROID_SDK_ROOT=$ANDROID_SDK_ROOT"
else
	echo "[MISS] ANDROID_HOME or ANDROID_SDK_ROOT is not set"
fi

if [ -n "${ANDROID_NDK_HOME:-}" ]; then
	echo "[ OK ] ANDROID_NDK_HOME=$ANDROID_NDK_HOME"
else
	echo "[MISS] ANDROID_NDK_HOME is not set"
fi

if [ -n "${JAVA_HOME:-}" ]; then
	echo "[ OK ] JAVA_HOME=$JAVA_HOME"
else
	echo "[MISS] JAVA_HOME is not set"
fi

echo "[XGE] Done."
