#!/bin/sh
set -eu
cd "$(dirname "$0")/../.."
CC=${CC:-gcc}
OUT_DIR=${XUI_BIDI_SANITIZER_OUT_DIR:-build/document}
CORPUS=${1:-tests/xui/data/unicode-17/BidiCharacterTest.txt}
CLASS_CORPUS=${2:-tests/xui/data/unicode-17/BidiTest.txt}
mkdir -p "$OUT_DIR"
"$CC" -std=c11 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie \
    -Wall -Wextra -Werror -DXUI_BIDI_TEST_ALLOCATOR @config/includes.rsp \
    tests/xui/xui_text_bidi_test.c src/xui/core/xui_text_bidi.c -o "$OUT_DIR/xui_text_bidi_sanitizer"
ASAN_OPTIONS=${ASAN_OPTIONS:-detect_leaks=1:halt_on_error=1}
UBSAN_OPTIONS=${UBSAN_OPTIONS:-halt_on_error=1}
export ASAN_OPTIONS UBSAN_OPTIONS
"$OUT_DIR/xui_text_bidi_sanitizer" "$CORPUS" "$CLASS_CORPUS"
