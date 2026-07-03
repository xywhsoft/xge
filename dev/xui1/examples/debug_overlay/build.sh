#!/bin/sh
set -e

ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)
cd "$ROOT"

OUT_DIR="build"
OUT="$OUT_DIR/xge_debug_overlay"
SRC="xge.c examples/debug_overlay/main.c"
INC="-I."
FLAGS="-O2 -Wall -Wextra -Wno-unused-parameter -Wno-unused-function -Wno-cast-function-type"
LIBS="-lX11 -lXi -lXcursor -lGL -ldl -lpthread -lm"
CC="${CC:-cc}"

mkdir -p "$OUT_DIR"
echo "[XGE] Building debug overlay EXE..."
$CC $FLAGS $INC -o "$OUT" $SRC $LIBS
echo "[XGE] Build successful: $OUT"