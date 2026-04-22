#!/usr/bin/env sh
set -eu

OUT_DIR=build
OUT="$OUT_DIR/xge_xui_incubation"
SRC="xge.c examples/xui_incubation/main.c"
INC="-I."
FLAGS="-O2 -Wall -Wextra -Wno-unused-parameter -Wno-unused-function -Wno-cast-function-type"
LIBS="-lm -ldl -pthread"

mkdir -p "$OUT_DIR"
echo "[XGE] Building XUI incubation EXE..."
cc $FLAGS $INC -o "$OUT" $SRC $LIBS
echo "[XGE] Build successful: $OUT"
