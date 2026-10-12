#!/bin/sh
set -eu
cd "$(dirname "$0")/../.."
CC="${CC:-gcc}"
OUT="${XUI_RENDER_LIFETIME_TEST_OUT:-build/xui_render_lifetime_asan}"
mkdir -p build
"$CC" -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie \
    -Wall -Wextra -Wno-unused-parameter -Wno-unused-function -Wno-cast-function-type \
    -DXGE_DEBUGMODE=0 @config/includes.rsp -o "$OUT" \
    tests/xui/xui_render_lifetime_test.c tests/xui/xui_test_proxy.c tests/xui/xui_test_xrt_impl.c \
    modules/xlayout/xlayout.c src/xui/core/xui_core.c src/xui/accessibility/xui_native_accessibility.c src/xui/core/xui_widget.c src/xui/core/xui_layout.c \
    src/xui/core/xui_input.c src/xui/core/xui_edit.c src/xui/core/xui_drag_drop.c src/xui/accessibility/xui_accessibility.c \
    src/xui/core/xui_text.c src/xui/core/xui_assets.c src/xui/core/xui_builtin_atlas.c src/xui/core/xui_unicode.c src/xui/core/xui_icon.c \
    -lm -ldl -lpthread
ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 UBSAN_OPTIONS=halt_on_error=1 "$OUT" "$@"
