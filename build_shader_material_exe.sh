#!/usr/bin/env sh
set -eu

OUT_DIR=build
OUT="$OUT_DIR/xge_shader_material"
SRC="xge.c examples/shader_material/main.c"
INC="-I."
FLAGS="-O2 -Wall -Wextra -Wno-unused-parameter -Wno-unused-function -Wno-cast-function-type"
LIBS="-lm -ldl -pthread"

mkdir -p "$OUT_DIR"
echo "[XGE] Building shader material EXE..."
cc $FLAGS $INC -o "$OUT" $SRC $LIBS
echo "[XGE] Build successful: $OUT"
