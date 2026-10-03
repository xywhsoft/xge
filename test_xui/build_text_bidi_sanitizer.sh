#!/bin/sh
set -eu
cd "$(dirname "$0")/.."
CC=${CC:-gcc}
OUT_DIR=${XUI_BIDI_SANITIZER_OUT_DIR:-build/document}
CORPUS=${1:-artifacts/xui-document-rebuild/bidi/BidiCharacterTest-17.0.0.txt}
CLASS_CORPUS=${2:-artifacts/xui-document-rebuild/bidi/BidiTest-17.0.0.txt}
mkdir -p "$OUT_DIR"
"$CC" -std=c11 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie \
    -Wall -Wextra -Werror -DXUI_BIDI_TEST_ALLOCATOR -I. \
    test_xui/xui_text_bidi_test.c src/xui_text_bidi.c -o "$OUT_DIR/xui_text_bidi_sanitizer"
ASAN_OPTIONS=${ASAN_OPTIONS:-detect_leaks=1:halt_on_error=1}
UBSAN_OPTIONS=${UBSAN_OPTIONS:-halt_on_error=1}
export ASAN_OPTIONS UBSAN_OPTIONS
"$OUT_DIR/xui_text_bidi_sanitizer" "$CORPUS" "$CLASS_CORPUS"
