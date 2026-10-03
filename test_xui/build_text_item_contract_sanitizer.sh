#!/bin/sh
set -eu
cd "$(dirname "$0")/.."
CC=${CC:-gcc}
OUT_DIR=${XUI_DOCUMENT_SANITIZER_OUT_DIR:-build/document}
mkdir -p "$OUT_DIR"
"$CC" -std=c11 -O1 -g -Wall -Wextra -Werror -I. \
    -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie \
    test_xui/xui_text_item_contract_test.c -o "$OUT_DIR/xui_text_item_contract_sanitizer"
ASAN_OPTIONS=${ASAN_OPTIONS:-detect_leaks=1:halt_on_error=1}
UBSAN_OPTIONS=${UBSAN_OPTIONS:-halt_on_error=1}
export ASAN_OPTIONS UBSAN_OPTIONS
"$OUT_DIR/xui_text_item_contract_sanitizer"
