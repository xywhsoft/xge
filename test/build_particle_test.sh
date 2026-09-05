#!/bin/sh
set -eu
cd "$(dirname "$0")/.."
mkdir -p build
${CC:-cc} -std=c11 -O1 -g -Wall -Wextra -Werror -DXRT_MODULE_MEMORY_DEBUG -I. \
  test/test_particle.c test/particle_xrt_impl.c src/xge_particle.c src/xge_particle_io.c \
  -o build/xge_particle_test -lm -lpthread -ldl
./build/xge_particle_test
