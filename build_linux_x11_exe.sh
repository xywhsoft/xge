#!/bin/sh
set -e

OUT_DIR=build
OUT="$OUT_DIR/xge_linux_x11"
SRC="xge.c examples/platform_smoke/main.c"
INC="-I."
FLAGS="-O2 -Wall -Wextra -Wno-unused-parameter -Wno-unused-function -Wno-cast-function-type -DSOKOL_GLCORE"
LIBS="-lX11 -lXi -lXcursor -lGL -ldl -lpthread -lm"

mkdir -p "$OUT_DIR"
echo "[XGE] Building Linux X11/OpenGL platform smoke EXE..."
cc $FLAGS $INC -o "$OUT" $SRC $LIBS
echo "[XGE] Build successful: $OUT"
