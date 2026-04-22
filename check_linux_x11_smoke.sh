#!/bin/sh
set -e

./build_linux_x11_exe.sh
XGE_SMOKE_EXE=build/xge_linux_x11 XGE_SMOKE_LOG=build/linux_x11_smoke.log ./run_platform_smoke.sh
