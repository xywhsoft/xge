#!/bin/sh
set -e

./examples/platform_smoke/build_linux_egl.sh
XGE_SMOKE_EXE=build/xge_linux_egl XGE_SMOKE_LOG=build/linux_egl_smoke.log ./run_platform_smoke.sh
