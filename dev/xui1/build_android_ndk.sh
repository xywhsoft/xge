#!/bin/sh
set -e

if [ -z "${ANDROID_NDK_HOME:-}" ]; then
	echo "[XGE] ANDROID_NDK_HOME is not set."
	exit 1
fi

ABI="${ANDROID_ABI:-arm64-v8a}"
API="${ANDROID_PLATFORM:-android-18}"
OUT_DIR="build/android/$ABI"

echo "[XGE] Configuring Android NativeActivity scaffold..."
cmake -S platform/android -B "$OUT_DIR" \
	-DCMAKE_TOOLCHAIN_FILE="$ANDROID_NDK_HOME/build/cmake/android.toolchain.cmake" \
	-DANDROID_ABI="$ABI" \
	-DANDROID_PLATFORM="$API"

echo "[XGE] Building Android NativeActivity scaffold..."
cmake --build "$OUT_DIR"
