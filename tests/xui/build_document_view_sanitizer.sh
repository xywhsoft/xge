#!/bin/sh
set -eu

cd "$(dirname "$0")/../.."
CC=${CC:-gcc}
PYTHON=${PYTHON:-python3}
OUT_DIR=${XUI_DOCUMENT_SANITIZER_OUT_DIR:-build/document}
TEST_SOURCE=${XUI_DOCUMENT_SANITIZER_TEST_SOURCE:-tests/xui/xui_document_view_portability_test.c}
TEST_BINARY=${XUI_DOCUMENT_SANITIZER_BINARY:-xui_document_view_sanitizer}
mkdir -p "$OUT_DIR"

# Expand the same authoritative source manifest used by the release build.
# Exclude only the platform XGE proxy and WebView modules; this headless test
# supplies a test proxy and fails if its out-of-scope PNG clipboard path is
# entered.
ui_sources=$("$PYTHON" tools/build/build.py --sources-only)
source_args=
for source in $ui_sources; do
    case "$source" in
        src/xui/core/xui_proxy_xge.c|src/xui/document/xui_webview.c|src/xui/document/xui_document_web_provider.c) ;;
        *) source_args="$source_args $source" ;;
    esac
done
# Source paths in both manifests have no whitespace; splitting is intentional.
set -- $source_args

"$CC" -std=c11 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie \
    -ffunction-sections -fdata-sections -Wl,--gc-sections \
    -Wall -Wextra -Werror -Wno-error=maybe-uninitialized \
    -Wno-unused-function -Wno-unused-parameter \
    -Wno-format-truncation -DXUI_DOCUMENT_HEADLESS_IMAGE_STUBS @config/includes.rsp \
    -o "$OUT_DIR/$TEST_BINARY" \
    "$TEST_SOURCE" \
    tests/xui/xui_test_proxy.c tests/xui/xui_test_xrt_impl.c "$@" \
    -lm -ldl -lpthread

ASAN_OPTIONS=${ASAN_OPTIONS:-detect_leaks=1:halt_on_error=1}
UBSAN_OPTIONS=${UBSAN_OPTIONS:-halt_on_error=1}
export ASAN_OPTIONS UBSAN_OPTIONS
"$OUT_DIR/$TEST_BINARY"
