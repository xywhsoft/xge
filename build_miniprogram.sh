#!/bin/sh
set -e

OUT_DIR="dist/miniprogram"

mkdir -p "$OUT_DIR/platform" "$OUT_DIR/examples"
cp platform/miniprogram/xge_miniprogram.js "$OUT_DIR/platform/xge_miniprogram.js"
cp platform/miniprogram/xge_miniprogram_types.d.ts "$OUT_DIR/platform/xge_miniprogram_types.d.ts"
mkdir -p "$OUT_DIR/examples/miniprogram_hello"
cp -R examples/miniprogram_hello/. "$OUT_DIR/examples/miniprogram_hello/"

echo "[XGE] Mini program scaffold generated: $OUT_DIR"
echo "[XGE] Native WASM build is not implemented yet."
