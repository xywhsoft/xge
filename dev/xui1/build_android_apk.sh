#!/bin/sh
set -e

if ! command -v gradle >/dev/null 2>&1; then
	echo "[XGE] gradle not found. Install Gradle or use Android Studio."
	exit 1
fi

if [ -z "${ANDROID_HOME:-}" ] && [ -z "${ANDROID_SDK_ROOT:-}" ]; then
	echo "[XGE] ANDROID_HOME or ANDROID_SDK_ROOT is not set."
	exit 1
fi

echo "[XGE] Building Android debug APK..."
cd platform/android/gradle
gradle assembleDebug
echo "[XGE] Build successful: platform/android/gradle/app/build/outputs/apk/debug/app-debug.apk"
