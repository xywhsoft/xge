#!/bin/sh
set -e

OUT_DIR=build
OUT="$OUT_DIR/libxgedbg.so"
SRC="xge.c"
INC="-I."
FLAGS="-O2 -Wall -Wextra -Wno-unused-parameter -Wno-unused-function -Wno-cast-function-type -fPIC -DXGE_DLL -DXGE_BUILD_DLL -DXGE_DEBUGMODE=1"
LIBS="-lX11 -lXi -lXcursor -lGL -ldl -lpthread -lm"
CC="${CC:-gcc}"

mkdir -p "$OUT_DIR"
echo "[XGEDBG] Building shared library..."
$CC -shared $FLAGS $INC -o "$OUT" $SRC $LIBS
echo "[XGEDBG] Build successful: $OUT"
