#!/usr/bin/env sh
set -eu

OUT_DIR=build
OUT="$OUT_DIR/xge_xui_text_history_limit_lab"
SRC="xge.c examples/xui_text_history_limit_lab/main.c"
INC="-I."
CFLAGS="-std=c99 -Wall -Wextra"
LIBS="-lm -ldl -lpthread"

mkdir -p "$OUT_DIR"

echo "[XGE] Building xui text history limit lab..."
cc $CFLAGS $INC $SRC -o "$OUT" $LIBS
echo "[XGE] Build successful: $OUT"
