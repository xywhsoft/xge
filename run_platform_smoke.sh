#!/bin/sh
set -e

EXE="${XGE_SMOKE_EXE:-build/xge_platform_smoke}"
LOG="${XGE_SMOKE_LOG:-build/platform_smoke.log}"
FRAMES="${XGE_SMOKE_FRAMES:-180}"

if [ ! -x "$EXE" ]; then
	echo "[XGE] $EXE not found or not executable. Run ./build_platform_smoke_exe.sh first."
	exit 1
fi

mkdir -p build
echo "[XGE] Running $EXE --frames $FRAMES"
"$EXE" --frames "$FRAMES" > "$LOG"
./check_platform_smoke_log.sh "$LOG" --require-final
