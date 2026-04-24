#!/usr/bin/env sh
set -eu

ROOT="$(CDPATH= cd -- "$(dirname "$0")/../.." && pwd)"
OUT_DIR="$ROOT/build"
OUT="$OUT_DIR/xge_auto_pause_lab"
SRC="$ROOT/xge.c $ROOT/examples/auto_pause_lab/main.c"
CFLAGS="-O2 -Wall -Wextra -Wno-unused-parameter -Wno-unused-function -Wno-cast-function-type"
LIBS="-lm -ldl -lpthread"

mkdir -p "$OUT_DIR"

echo "[XGE] Building auto pause lab..."
cc $CFLAGS -I"$ROOT" -o "$OUT" $SRC $LIBS
echo "[XGE] Build successful: $OUT"
