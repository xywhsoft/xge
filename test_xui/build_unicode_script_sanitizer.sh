#!/bin/sh
set -eu
cd "$(dirname "$0")/.."
script_out=${XUI_SCRIPT_OUT_DIR:-build/unicode-script-sanitizer}
mkdir -p "$script_out"
${CC:-gcc} -std=c11 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie \
    -Wall -Wextra -Werror -I. -o "$script_out/script-test" \
    test_xui/xui_unicode_script_test.c lib/libunibreak/src/unibreakdef.c
ASAN_OPTIONS=${ASAN_OPTIONS:-detect_leaks=1:halt_on_error=1}
UBSAN_OPTIONS=${UBSAN_OPTIONS:-halt_on_error=1}
export ASAN_OPTIONS UBSAN_OPTIONS
"$script_out/script-test"
