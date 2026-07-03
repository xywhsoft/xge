#!/bin/sh
set -e

ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)
cd "$ROOT"

if ! command -v emcc >/dev/null 2>&1; then
	echo "[XGE] emcc not found. Please activate the Emscripten SDK first."
	exit 1
fi

OUT_DIR="build/web"
OUT="$OUT_DIR/xge_web.html"
SRC="xge.c examples/platform_smoke/main.c"
INC="-I."
FLAGS="-O2 -Wall -Wextra -Wno-unused-parameter -Wno-unused-function -Wno-cast-function-type -DSOKOL_GLES3 -DXGE_NO_AUDIO"
EMFLAGS="-sMIN_WEBGL_VERSION=2 -sMAX_WEBGL_VERSION=2 -sFULL_ES3=1 -sALLOW_MEMORY_GROWTH=1 -sEXIT_RUNTIME=0 --shell-file platform/web/shell.html"

mkdir -p "$OUT_DIR"
echo "[XGE] Building Web/Emscripten MVP..."
emcc $FLAGS $INC -o "$OUT" $SRC $EMFLAGS
echo "[XGE] Build successful: $OUT"
echo "[XGE] Serve build/web with a local HTTP server before opening the page."