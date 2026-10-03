#!/bin/sh
set -eu
cd "$(dirname "$0")/.."
out=${XGE_OPENTYPE_OUT_DIR:-build/opentype-linux}
mkdir -p "$out"
${CXX:-g++} -std=c++17 -O2 -fno-exceptions -fno-rtti -ffunction-sections -fdata-sections -DNDEBUG \
    -DHB_NO_BUFFER_SERIALIZE -DHB_NO_BUFFER_VERIFY -DHB_NO_DRAW -DHB_NO_OT_FETCH -DHB_NO_MMAP -DHB_NO_COLOR -DHAVE_PTHREAD \
    -c lib/harfbuzz/src/harfbuzz.cc -o "$out/harfbuzz.o"
${CC:-gcc} -std=c11 -O2 -Wall -Wextra -Werror -o "$out/dependency-test" \
    test/test_opentype_dependency.c "$out/harfbuzz.o" -lm -lpthread
"$out/dependency-test"
