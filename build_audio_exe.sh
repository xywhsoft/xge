#!/bin/sh
set -e

OUT_DIR=build
OUT="$OUT_DIR/xge_audio"
SRC="xge.c examples/audio/main.c"
INC="-I."
FLAGS="-O2 -Wall -Wextra -Wno-unused-parameter -Wno-unused-function -Wno-cast-function-type"
LIBS="-lX11 -lXi -lXcursor -lGL -ldl -lpthread -lm"

mkdir -p "$OUT_DIR"
echo "[XGE] Building audio EXE..."
gcc $FLAGS $INC -o "$OUT" $SRC $LIBS
echo "[XGE] Build successful: $OUT"
