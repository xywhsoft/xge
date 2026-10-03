#!/bin/sh
set -eu
cd "$(dirname "$0")/.."
joining_out=${XGE_JOINING_OUT_DIR:-build/joining-context-sanitizer}
hb_out=${XGE_OPENTYPE_HB_DIR:-build/opentype-linux}
mkdir -p "$joining_out"
if [ ! -s "$hb_out/harfbuzz.o" ] || [ -n "$(find lib/harfbuzz/src build_text_shaping.bat -type f -newer "$hb_out/harfbuzz.o" -print -quit)" ]; then
    XGE_OPENTYPE_OUT_DIR="$hb_out" sh test/build_opentype_dependency_test.sh
fi
${CC:-gcc} -std=c11 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie \
    -Wall -Wextra -Werror -I. -o "$joining_out/context-test" \
    test/test_joining_context_portability.c "$hb_out/harfbuzz.o" -lm -lpthread
ASAN_OPTIONS=${ASAN_OPTIONS:-detect_leaks=1:halt_on_error=1}
UBSAN_OPTIONS=${UBSAN_OPTIONS:-halt_on_error=1}
export ASAN_OPTIONS UBSAN_OPTIONS
"$joining_out/context-test"
${CC:-gcc} -std=c11 -O2 -Wall -Wextra -Werror test/test_unicode_joining.c -o "$joining_out/unicode-test"
python3 test/verify_unicode_joining.py "$joining_out/unicode-test"
${CXX:-g++} -std=c++17 -O2 -Wall -Wextra -Werror test/test_hb_joining_transparency.cc \
    "$hb_out/harfbuzz.o" -o "$joining_out/hb-transparency-test" -lm -lpthread
"$joining_out/hb-transparency-test"
