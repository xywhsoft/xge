#!/usr/bin/env sh
set -eu

ROOT="$(CDPATH= cd -- "$(dirname "$0")/../.." && pwd)"
cd "$ROOT"

OUT_DIR="build"
OUT="$OUT_DIR/xge_xui_text_edit_lab"
SRC="xge.c examples/xui_text_edit_lab/main.c"
CFLAGS="-O2 -Wall -Wextra -Wno-unused-parameter -Wno-unused-function -Wno-cast-function-type"
LIBS="-lm -ldl -lpthread -lX11 -lXi -lXcursor -lGL"

mkdir -p "$OUT_DIR"

echo "[XGE] Building xui text edit lab..."
cc $CFLAGS -I. -o "$OUT" $SRC $LIBS
echo "[XGE] Build successful: $OUT"
