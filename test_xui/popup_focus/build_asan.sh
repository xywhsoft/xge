#!/bin/sh
set -eu
cd "$(dirname "$0")/../.."

CC="${CC:-gcc}"
OUT="${XUI_POPUP_TEST_OUT:-build/xui_popup_focus_asan}"
INPUT=src/xui_input.c
POPUP=src/xui_popup.c
HEADER=
if [ "${XUI_POPUP_TEST_BASELINE:-0}" = 1 ]; then
    mkdir -p build
    git archive --format=tar -o build/popup-focus-baseline.tar \
        "${XUI_POPUP_TEST_BASELINE_REF:-HEAD}" src/xui_input.c src/xui_popup.c src/xui_internal.h
    tar -xf build/popup-focus-baseline.tar -C build --strip-components=1
    INPUT=build/xui_input.c
    POPUP=build/xui_popup.c
    HEADER="-include build/xui_internal.h"
fi
mkdir -p build
"$CC" -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie \
    -Wall -Wextra -Wno-unused-parameter -Wno-unused-function -Wno-cast-function-type \
    -DXGE_DEBUGMODE=0 -I. -Isrc $HEADER -o "$OUT" \
    test_xui/xui_popup_focus_test.c test_xui/xui_test_xrt_impl.c \
    lib/xlayout/xlayout.c src/xui_core.c src/xui_widget.c src/xui_layout.c \
    "$INPUT" src/xui_edit.c src/xui_drag_drop.c src/xui_text.c \
    src/xui_assets.c src/xui_builtin_atlas.c src/xui_unicode.c \
    src/xui_scroll_model.c src/xui_scrollbar.c src/xui_scroll_frame.c \
    src/xui_scroll_view.c "$POPUP" -lm -ldl -lpthread
ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 UBSAN_OPTIONS=halt_on_error=1 "$OUT" "$@"
