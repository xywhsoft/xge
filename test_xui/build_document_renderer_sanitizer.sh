#!/bin/sh
set -eu

cd "$(dirname "$0")/.."
CC=${CC:-gcc}
OUT_DIR=${XUI_DOCUMENT_SANITIZER_OUT_DIR:-build/document}
TEST_SOURCE=${XUI_DOCUMENT_SANITIZER_TEST_SOURCE:-test_xui/xui_document_renderer_portability_test.c}
TEST_BINARY=${XUI_DOCUMENT_SANITIZER_BINARY:-xui_document_renderer_sanitizer}
mkdir -p "$OUT_DIR"

# Reuse the release core manifest. Renderer dependencies below are the small
# headless XUI subset needed by the test proxy, without a platform WebView.
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
    -ffunction-sections -fdata-sections -Wl,--gc-sections \
    -Wall -Wextra -Werror -Wno-unused-function -Wno-unused-parameter -I. \
    ${XUI_DOCUMENT_SANITIZER_EXTRA_FLAGS:-} \
    -o "$OUT_DIR/$TEST_BINARY" \
    "$TEST_SOURCE" \
    test_xui/xui_test_proxy.c test_xui/xui_test_xrt_impl.c "$@" \
    lib/xlayout/xlayout.c src/xui_core.c src/xui_widget.c src/xui_layout.c \
    src/xui_input.c src/xui_assets.c src/xui_builtin_atlas.c \
    src/xui_text.c src/xui_text_bidi.c src/xui_document_layout.c src/xui_document_renderer.c \
    src/xui_scroll_model.c src/xui_accessibility.c src/xui_drag_drop.c \
    src/xui_edit.c -lm -ldl -lpthread

ASAN_OPTIONS=${ASAN_OPTIONS:-detect_leaks=1:halt_on_error=1}
UBSAN_OPTIONS=${UBSAN_OPTIONS:-halt_on_error=1}
export ASAN_OPTIONS UBSAN_OPTIONS
"$OUT_DIR/$TEST_BINARY"
