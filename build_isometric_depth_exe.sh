#!/usr/bin/env sh
set -eu

OUT_DIR=build
OUT="$OUT_DIR/xge_isometric_depth"
SRC="xge.c examples/isometric_depth/main.c"
INC="-I."
FLAGS="-O2 -Wall -Wextra -Wno-unused-parameter -Wno-unused-function -Wno-cast-function-type"
LIBS="-lm -ldl -pthread"

mkdir -p "$OUT_DIR"
echo "[XGE] Building isometric depth EXE..."
cc $FLAGS $INC -o "$OUT" $SRC $LIBS
echo "[XGE] Build successful: $OUT"
