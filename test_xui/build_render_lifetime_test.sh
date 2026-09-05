#!/bin/sh
set -eu
cd "$(dirname "$0")/.."
CC="${CC:-gcc}"
OUT="${XUI_RENDER_LIFETIME_TEST_OUT:-build/xui_render_lifetime_asan}"
mkdir -p build
"$CC" -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie \
    -Wall -Wextra -Wno-unused-parameter -Wno-unused-function -Wno-cast-function-type \
    -DXGE_DEBUGMODE=0 -I. -o "$OUT" \
    test_xui/xui_render_lifetime_test.c test_xui/xui_test_proxy.c test_xui/xui_test_xrt_impl.c \
    lib/xlayout/xlayout.c src/xui_core.c src/xui_widget.c src/xui_layout.c \
    src/xui_input.c src/xui_edit.c src/xui_drag_drop.c src/xui_accessibility.c \
    src/xui_text.c src/xui_assets.c src/xui_builtin_atlas.c src/xui_unicode.c src/xui_icon.c \
    -lm -ldl -lpthread
ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 UBSAN_OPTIONS=halt_on_error=1 "$OUT" "$@"
