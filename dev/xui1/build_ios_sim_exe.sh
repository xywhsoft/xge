#!/bin/sh
set -e

SDK="${IOS_SDK:-iphonesimulator}"
ARCH="${IOS_ARCH:-arm64}"
DEPLOY="${IOS_DEPLOYMENT_TARGET:-12.0}"
OUT_DIR="build/ios_sim"
APP_DIR="$OUT_DIR/XGE.app"
BIN="$APP_DIR/XGE"
SRC="xge.c platform/ios/xge_ios_main.m"
INC="-I."
FLAGS="-O2 -Wall -Wextra -Wno-unused-parameter -Wno-unused-function -Wno-cast-function-type -x objective-c -DXGE_NO_AUDIO -DSOKOL_GLES3 -mios-simulator-version-min=$DEPLOY"
LIBS="-framework UIKit -framework Foundation -framework QuartzCore -framework OpenGLES -lm"

SDK_PATH="$(xcrun --sdk "$SDK" --show-sdk-path)"

mkdir -p "$APP_DIR"
echo "[XGE] Building iOS simulator OpenGL ES MVP app..."
xcrun --sdk "$SDK" clang $FLAGS -isysroot "$SDK_PATH" -arch "$ARCH" $INC -o "$BIN" $SRC $LIBS
cp platform/ios/Info.plist "$APP_DIR/Info.plist"
codesign --force --sign - "$APP_DIR" >/dev/null 2>&1 || true
echo "[XGE] Build successful: $APP_DIR"
