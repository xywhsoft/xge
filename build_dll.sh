#!/bin/sh
set -e

OUT_DIR=build
OUT="$OUT_DIR/libxge.so"
SRC="xge.c"
INC="-I."
FLAGS="-O2 -Wall -Wextra -Wno-unused-parameter -Wno-unused-function -Wno-cast-function-type -fPIC -DXGE_DLL -DXGE_BUILD_DLL"
LIBS="-lX11 -lXi -lXcursor -lGL -ldl -lpthread -lm"

mkdir -p "$OUT_DIR"
echo "[XGE] Building shared library..."
gcc -shared $FLAGS $INC -o "$OUT" $SRC $LIBS
echo "[XGE] Build successful: $OUT"
