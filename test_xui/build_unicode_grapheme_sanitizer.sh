#!/bin/sh
set -eu
cd "$(dirname "$0")/.."
grapheme_out=${XUI_GRAPHEME_OUT_DIR:-build/unicode-grapheme-sanitizer}
mkdir -p "$grapheme_out"
${CC:-gcc} -std=c11 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie \
    -Wall -Wextra -Werror -I. -o "$grapheme_out/grapheme-test" \
    test_xui/xui_unicode_grapheme_test.c src/xui_unicode.c
ASAN_OPTIONS=${ASAN_OPTIONS:-detect_leaks=1:halt_on_error=1}
UBSAN_OPTIONS=${UBSAN_OPTIONS:-halt_on_error=1}
export ASAN_OPTIONS UBSAN_OPTIONS
"$grapheme_out/grapheme-test"
