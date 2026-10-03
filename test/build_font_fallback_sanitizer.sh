#!/bin/sh
set -eu
cd "$(dirname "$0")/.."
fallback_out=${XGE_FONT_FALLBACK_OUT_DIR:-build/font-fallback-sanitizer}
hb_out=${XGE_OPENTYPE_HB_DIR:-build/opentype-linux}
mkdir -p "$fallback_out"
if [ ! -s "$hb_out/harfbuzz.o" ] || [ -n "$(find lib/harfbuzz/src build_text_shaping.bat -type f -newer "$hb_out/harfbuzz.o" -print -quit)" ]; then
    XGE_OPENTYPE_OUT_DIR="$hb_out" sh test/build_opentype_dependency_test.sh
fi
${CC:-gcc} -std=c11 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie \
    -Wall -Wextra -Werror -I. -o "$fallback_out/fallback-test" \
    test/test_font_fallback_portability.c lib/libunibreak/src/unibreakdef.c \
    "$hb_out/harfbuzz.o" -lm -lpthread
ASAN_OPTIONS=${ASAN_OPTIONS:-detect_leaks=1:halt_on_error=1}
UBSAN_OPTIONS=${UBSAN_OPTIONS:-halt_on_error=1}
export ASAN_OPTIONS UBSAN_OPTIONS
"$fallback_out/fallback-test"
