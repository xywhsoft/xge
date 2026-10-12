#!/bin/sh
set -e

OUT_DIR=build
OUT="$OUT_DIR/xge_test"
SRC="single/xge.c tests/xge/test_main.c"
INC="@config/includes.rsp"
FLAGS="-O2 -Wall -Wextra -Wno-unused-parameter -Wno-unused-function -Wno-cast-function-type"
LIBS="-lX11 -lXi -lXcursor -lGL -ldl -lpthread -lm"

mkdir -p "$OUT_DIR"
echo "[XGE] Building tests..."
gcc $FLAGS $INC -o "$OUT" $SRC $LIBS
"$OUT"
echo "[XGE] Tests passed: $OUT"
