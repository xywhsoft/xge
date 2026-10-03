#!/bin/sh
set -eu
cd "$(dirname "$0")/.."
contour_out=${XGE_FONT_CONTOUR_OUT_DIR:-build/font-contour-sanitizer}
mkdir -p "$contour_out"
${CC:-gcc} -std=c11 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie \
    -Wall -Wextra -Werror -I. -o "$contour_out/contours-test" \
    test/test_font_contours_portability.c -lm
ASAN_OPTIONS=${ASAN_OPTIONS:-detect_leaks=1:halt_on_error=1}
UBSAN_OPTIONS=${UBSAN_OPTIONS:-halt_on_error=1}
export ASAN_OPTIONS UBSAN_OPTIONS
"$contour_out/contours-test"
