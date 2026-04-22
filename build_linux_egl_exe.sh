#!/bin/sh
set -e

OUT_DIR=build
OUT="$OUT_DIR/xge_linux_egl"
SRC="xge.c examples/platform_smoke/main.c"
INC="-I."
FLAGS="-O2 -Wall -Wextra -Wno-unused-parameter -Wno-unused-function -Wno-cast-function-type -DSOKOL_GLES3 -DSOKOL_FORCE_EGL"
LIBS="-lX11 -lXi -lXcursor -lEGL -lGLESv2 -ldl -lpthread -lm"

mkdir -p "$OUT_DIR"
echo "[XGE] Building Linux X11/EGL MVP EXE..."
cc $FLAGS $INC -o "$OUT" $SRC $LIBS
echo "[XGE] Build successful: $OUT"
