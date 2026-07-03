#!/bin/sh
set -e

ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)
cd "$ROOT"

OUT_DIR="build"
OUT="$OUT_DIR/xge_board_linux_egl"
SRC="xge.c examples/offscreen/main.c"
INC="-I."
FLAGS="-O2 -Wall -Wextra -Wno-unused-parameter -Wno-unused-function -Wno-cast-function-type -DSOKOL_DUMMY_BACKEND -DXGE_HAS_EGL -DXGE_HAS_EGL_PBUFFER -DXGE_HAS_EGL_BOARD_LINUX"
LIBS="-lEGL -lGLESv2 -ldl -lpthread -lm"
CC="${CC:-gcc}"

mkdir -p "$OUT_DIR"
echo "[XGE] Building board Linux EGL EXE..."
$CC $FLAGS $INC -o "$OUT" $SRC $LIBS
echo "[XGE] Build successful: $OUT"
echo "[XGE] Run on target board: $OUT"