#!/bin/sh
set -eu
cd "$(dirname "$0")/.."
context_out=${XGE_TEXT_CONTEXT_OUT_DIR:-build/context-sanitizer}
mkdir -p "$context_out"
${CC:-gcc} -std=c11 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie \
    -Wall -Wextra -Werror -I. test/test_text_context.c -o "$context_out/context-contract"
ASAN_OPTIONS=${ASAN_OPTIONS:-detect_leaks=1:halt_on_error=1}
UBSAN_OPTIONS=${UBSAN_OPTIONS:-halt_on_error=1}
export ASAN_OPTIONS UBSAN_OPTIONS
"$context_out/context-contract"
sh test/build_font_fallback_sanitizer.sh
