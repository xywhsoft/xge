#!/bin/sh
set -eu
cd "$(dirname "$0")/.."
mkdir -p build
"${CC:-cc}" -std=gnu11 -O1 -Wall -Wextra -Wno-unused-parameter -Wno-unused-function -Wno-cast-function-type -DXGE_DEBUGMODE=0 -I. test/test_texture_storage.c -o build/xge_texture_storage_test -lX11 -lXi -lXcursor -lGL -ldl -lpthread -lm
build/xge_texture_storage_test
