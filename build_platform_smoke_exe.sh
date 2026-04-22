#!/bin/sh
set -e

OUT_DIR=build
OUT="$OUT_DIR/xge_platform_smoke"
SRC="xge.c examples/platform_smoke/main.c"
INC="-I."
FLAGS="-O2 -Wall -Wextra -Wno-unused-parameter -Wno-unused-function -Wno-cast-function-type"
LIBS="-lm -ldl -lpthread -lX11 -lXi -lXcursor -lGL"

mkdir -p "$OUT_DIR"
echo "[XGE] Building platform smoke EXE..."
cc $FLAGS $INC -o "$OUT" $SRC $LIBS
echo "[XGE] Build successful: $OUT"
