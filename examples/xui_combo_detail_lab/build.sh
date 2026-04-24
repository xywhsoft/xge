#!/usr/bin/env sh
set -eu

OUT_DIR=build
OUT="$OUT_DIR/xge_xui_combo_detail_lab"
SRC="xge.c examples/xui_combo_detail_lab/main.c"
INC="-I."
CFLAGS="-O2 -Wall -Wextra -Wno-unused-parameter -Wno-unused-function"
LIBS="-lm -ldl -lpthread"

mkdir -p "$OUT_DIR"

echo "[XGE] Building xui combo detail lab..."
cc $CFLAGS $INC -o "$OUT" $SRC $LIBS
echo "[XGE] Build successful: $OUT"
