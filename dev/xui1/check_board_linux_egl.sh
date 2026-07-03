#!/bin/sh
set -e

echo "[XGE] Building board Linux EGL pbuffer smoke..."
./examples/offscreen/build_board_linux_egl.sh

echo "[XGE] Running board Linux EGL pbuffer smoke..."
./build/xge_board_linux_egl

echo "[XGE] Building EGL surfaceless smoke..."
./examples/egl_surfaceless/build.sh

echo "[XGE] Running EGL surfaceless smoke..."
./build/xge_egl_surfaceless

echo "[XGE] Board Linux EGL smoke checks finished."
