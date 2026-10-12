#!/bin/sh
set -eu
cd "$(dirname "$0")/../.."
caret_out=${XGE_OPENTYPE_CARET_OUT_DIR:-build/opentype-caret-sanitizer}
hb_out=${XGE_OPENTYPE_HB_DIR:-build/opentype-linux}
mkdir -p "$caret_out"
if [ ! -s "$hb_out/harfbuzz.o" ] || [ -n "$(find extlibs/harfbuzz/src tools\build\build_text_shaping.bat -type f -newer "$hb_out/harfbuzz.o" -print -quit)" ]; then
    XGE_OPENTYPE_OUT_DIR="$hb_out" sh tests/xge/build_opentype_dependency_test.sh
fi
${CC:-gcc} -std=c11 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie \
    -Wall -Wextra -Werror @config/includes.rsp -o "$caret_out/carets-test" \
    tests/xge/test_opentype_carets_portability.c extlibs/libunibreak/src/unibreakdef.c \
    "$hb_out/harfbuzz.o" -lm -lpthread
ASAN_OPTIONS=${ASAN_OPTIONS:-detect_leaks=1:halt_on_error=1}
UBSAN_OPTIONS=${UBSAN_OPTIONS:-halt_on_error=1}
export ASAN_OPTIONS UBSAN_OPTIONS
"$caret_out/carets-test"
${CC:-gcc} -std=c11 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie \
    -Wall -Wextra -Werror -DXGE_TEST_CONTOUR_POINTS @config/includes.rsp -o "$caret_out/contour-carets-test" \
    tests/xge/test_opentype_carets_portability.c extlibs/libunibreak/src/unibreakdef.c \
    "$hb_out/harfbuzz.o" -lm -lpthread
"$caret_out/contour-carets-test"
