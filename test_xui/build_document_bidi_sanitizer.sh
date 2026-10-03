#!/bin/sh
set -eu
cd "$(dirname "$0")/.."
export XUI_DOCUMENT_SANITIZER_TEST_SOURCE=test_xui/xui_document_bidi_portability_test.c
export XUI_DOCUMENT_SANITIZER_BINARY=xui_document_bidi_sanitizer
export XUI_DOCUMENT_SANITIZER_EXTRA_FLAGS=-DXUI_BIDI_TEST_ALLOCATOR
exec sh test_xui/build_document_renderer_sanitizer.sh
