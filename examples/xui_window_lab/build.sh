#!/usr/bin/env sh
set -eu

ROOT="$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)"
cd "$ROOT"

OUT_DIR="build"
OUT="$OUT_DIR/xge_xui_window_lab"
SRC="xge.c examples/xui_window_lab/main.c"
CFLAGS="-O2 -Wall -Wextra -Wno-unused-parameter -Wno-unused-function"

mkdir -p "$OUT_DIR"

echo "[XGE] Building xui window lab..."
cc $CFLAGS -I. -o "$OUT" $SRC -lm -ldl -lpthread
echo "[XGE] Build successful: $OUT"
