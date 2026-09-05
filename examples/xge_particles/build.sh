#!/bin/sh
set -eu
cd "$(dirname "$0")/../.."
test -f build/libxge.so || sh build_dll.sh
for scene in impact fire_smoke explosion weather dust magic confetti; do
  ${CC:-cc} -std=c11 -O2 -Wall -Wextra -Werror -DXGE_DLL -I. \
    examples/xge_particles/common.c "examples/xge_particles/$scene.c" \
    -o "build/xge_particles_$scene" -Lbuild -lxge -lm -Wl,-rpath,'$ORIGIN'
done
