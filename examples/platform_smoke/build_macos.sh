#!/bin/sh
set -e

ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)
cd "$ROOT"

OUT_DIR="build"
OUT="$OUT_DIR/xge_macos"
SRC="xge.c examples/platform_smoke/main.c"
INC="-I."
FLAGS="-O2 -Wall -Wextra -Wno-unused-parameter -Wno-unused-function -Wno-cast-function-type -x objective-c -DXGE_NO_AUDIO"
LIBS="-framework Cocoa -framework QuartzCore -framework OpenGL -lm"
CC="${CC:-clang}"

mkdir -p "$OUT_DIR"
echo "[XGE] Building macOS OpenGL MVP EXE..."
$CC $FLAGS $INC -o "$OUT" $SRC $LIBS
echo "[XGE] Build successful: $OUT"