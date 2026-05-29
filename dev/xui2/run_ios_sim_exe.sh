#!/bin/sh
set -e

DEVICE="${IOS_SIMULATOR:-booted}"
APP_DIR="${IOS_APP_DIR:-build/ios_sim/XGE.app}"
BUNDLE_ID="${IOS_BUNDLE_ID:-dev.xge.mvp}"

if ! command -v xcrun >/dev/null 2>&1; then
	echo "[XGE] xcrun not found. Install Xcode command line tools."
	exit 1
fi

if [ ! -d "$APP_DIR" ]; then
	echo "[XGE] $APP_DIR not found. Run ./build_ios_sim_exe.sh first."
	exit 1
fi

echo "[XGE] Installing $APP_DIR to simulator: $DEVICE"
xcrun simctl install "$DEVICE" "$APP_DIR"

echo "[XGE] Launching $BUNDLE_ID on simulator: $DEVICE"
xcrun simctl launch "$DEVICE" "$BUNDLE_ID"
echo "[XGE] iOS simulator smoke app started."
