#!/bin/sh
set -eu

cd "$(dirname "$0")/.."
CC=${CC:-gcc}
OUT_DIR=${XUI_DOCUMENT_SANITIZER_OUT_DIR:-build/document}
mkdir -p "$OUT_DIR"
sources=$(awk '
    /^set XUI_DOCUMENT_SRC=/ {
        sub(/^set XUI_DOCUMENT_SRC=/, "")
        gsub(/%XUI_DOCUMENT_SRC%/, "")
        gsub(/\r/, "")
        gsub(/\\/, "/")
        printf "%s ", $0
    }
' xui_document_sources.bat)
set -- $sources

"$CC" -std=c11 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie \
    -Wall -Wextra -Werror -Wno-unused-function -I. \
    -o "$OUT_DIR/xui_document_attribute_pool_scale_sanitizer" \
    test_xui/xui_document_attribute_pool_scale_test.c "$@" test_xui/xui_test_xrt_impl.c \
    -lm -ldl -lpthread
ASAN_OPTIONS=${ASAN_OPTIONS:-detect_leaks=1:halt_on_error=1}
UBSAN_OPTIONS=${UBSAN_OPTIONS:-halt_on_error=1}
export ASAN_OPTIONS UBSAN_OPTIONS
"$OUT_DIR/xui_document_attribute_pool_scale_sanitizer"
