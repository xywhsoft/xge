#!/usr/bin/env sh
set -eu

ROOT="$(CDPATH= cd -- "$(dirname "$0")/../.." && pwd)"
cd "$ROOT"

OUT_DIR="build"
OUT="$OUT_DIR/xge_miniprogram_bridge_lab"
SRC="xge.c examples/miniprogram_bridge_lab/main.c"
INC="-I."
FLAGS="-O2 -Wall -Wextra -Wno-unused-parameter -Wno-unused-function"
LIBS="-lm -ldl -pthread"

mkdir -p "$OUT_DIR"

echo "[XGE] Building miniprogram bridge lab..."
cc $FLAGS $INC -o "$OUT" $SRC $LIBS
echo "[XGE] Build successful: $OUT"
