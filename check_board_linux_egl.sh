#!/bin/sh
set -e

echo "[XGE] Building board Linux EGL pbuffer smoke..."
./build_board_linux_egl_exe.sh

echo "[XGE] Running board Linux EGL pbuffer smoke..."
./build/xge_board_linux_egl

echo "[XGE] Building EGL surfaceless smoke..."
./build_egl_surfaceless_exe.sh

echo "[XGE] Running EGL surfaceless smoke..."
./build/xge_egl_surfaceless

echo "[XGE] Board Linux EGL smoke checks finished."
