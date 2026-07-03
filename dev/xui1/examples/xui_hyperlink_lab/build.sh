#!/usr/bin/env sh
set -eu

ROOT="$(CDPATH= cd -- "$(dirname "$0")/../.." && pwd)"
OUT_DIR="$ROOT/build"
OUT="$OUT_DIR/xge_xui_hyperlink_lab"
SRC="$ROOT/xge.c $ROOT/examples/xui_hyperlink_lab/main.c"
CFLAGS="-O2 -Wall -Wextra -Wno-unused-parameter -Wno-unused-function -Wno-cast-function-type"
LIBS="-lm -ldl -lpthread"

mkdir -p "$OUT_DIR"

echo "[XGE] Building xui hyperlink lab..."
cc $CFLAGS -I"$ROOT" -o "$OUT" $SRC $LIBS
echo "[XGE] Build successful: $OUT"
