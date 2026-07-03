#!/bin/sh
set -e

ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)
cd "$ROOT"

OUT_DIR="dist/miniprogram"

mkdir -p "$OUT_DIR/platform" "$OUT_DIR/examples/miniprogram_hello"
cp platform/miniprogram/xge_miniprogram.js "$OUT_DIR/platform/xge_miniprogram.js"
cp platform/miniprogram/xge_miniprogram_types.d.ts "$OUT_DIR/platform/xge_miniprogram_types.d.ts"
cp -R examples/miniprogram_hello/. "$OUT_DIR/examples/miniprogram_hello/"
rm -f "$OUT_DIR/examples/miniprogram_hello/build.bat" "$OUT_DIR/examples/miniprogram_hello/build.sh"

echo "[XGE] Mini program scaffold generated: $OUT_DIR"
echo "[XGE] Native WASM build is not implemented yet."
