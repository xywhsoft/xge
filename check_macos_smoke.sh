#!/bin/sh
set -e

./build_macos_exe.sh
XGE_SMOKE_EXE=build/xge_macos XGE_SMOKE_LOG=build/macos_smoke.log ./run_platform_smoke.sh
