#!/bin/sh
set -e

PORT="${XGE_WEB_PORT:-8000}"
ROOT="build/web"
HTML="$ROOT/xge_web.html"

if [ ! -f "$HTML" ]; then
	echo "[XGE] $HTML not found. Run ./examples/platform_smoke/build_web.sh first."
	exit 1
fi

if command -v python3 >/dev/null 2>&1; then
	echo "[XGE] Serving http://localhost:$PORT/xge_web.html"
	python3 -m http.server "$PORT" -d "$ROOT"
elif command -v python >/dev/null 2>&1; then
	echo "[XGE] Serving http://localhost:$PORT/xge_web.html"
	python -m http.server "$PORT" -d "$ROOT"
else
	echo "[XGE] python3/python not found."
	exit 1
fi
