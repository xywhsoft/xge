#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
cd "$ROOT"

OUT_DIR="build"
OUT="$OUT_DIR/xge_xui_visual_proc_lab"

mkdir -p "$OUT_DIR"

echo "[XGE] Building xui visual proc lab..."
cc -O2 -Wall -Wextra -Wno-unused-parameter -Wno-unused-function -I. \
	-o "$OUT" \
	xge.c \
	examples/xui_visual_proc_lab/main.c \
	-lm -ldl -lpthread

echo "[XGE] Build successful: $OUT"
