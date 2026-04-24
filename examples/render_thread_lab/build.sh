#!/usr/bin/env sh
set -eu

ROOT="$(CDPATH= cd -- "$(dirname "$0")/../.." && pwd)"
cd "$ROOT"

OUT_DIR="build"
OUT="$OUT_DIR/xge_render_thread_lab"
SRC="xge.c examples/render_thread_lab/main.c"
INC="-I."
FLAGS="-O2 -Wall -Wextra -Wno-unused-parameter -Wno-unused-function"
LIBS="-lm -ldl -pthread"

mkdir -p "$OUT_DIR"

echo "[XGE] Building render thread lab..."
cc $FLAGS $INC -o "$OUT" $SRC $LIBS
echo "[XGE] Build successful: $OUT"
