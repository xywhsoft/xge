#!/bin/sh
set -e

APK="${XGE_ANDROID_APK:-platform/android/gradle/app/build/outputs/apk/debug/app-debug.apk}"
PKG="${XGE_ANDROID_PACKAGE:-dev.xge.smoke}"
ACTIVITY="${XGE_ANDROID_ACTIVITY:-android.app.NativeActivity}"

if ! command -v adb >/dev/null 2>&1; then
	echo "[XGE] adb not found. Install Android platform-tools."
	exit 1
fi

if [ ! -f "$APK" ]; then
	echo "[XGE] $APK not found. Run ./build_android_apk.sh first."
	exit 1
fi

echo "[XGE] Installing $APK..."
adb install -r "$APK"

echo "[XGE] Starting $PKG/$ACTIVITY..."
adb shell am start -n "$PKG/$ACTIVITY"
echo "[XGE] Android smoke app started. Use adb logcat for device logs."
