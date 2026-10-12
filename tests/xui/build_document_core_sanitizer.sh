#!/bin/sh
set -eu

cd "$(dirname "$0")/../.."
CC=${CC:-gcc}
OUT_DIR=${XUI_DOCUMENT_SANITIZER_OUT_DIR:-build/document}
if [ "$#" -gt 1 ]; then
    echo "Usage: $0 [commonmark-0.31.2.json]" >&2
    exit 2
fi
CORPUS=${1:-}
mkdir -p "$OUT_DIR"

# The core source manifest is shared with Windows builds. Each entry is a
# project-owned path without whitespace, so splitting the resulting list is
# intentional here.
sources=$(awk '
    /^set XUI_DOCUMENT_SRC=/ {
        sub(/^set XUI_DOCUMENT_SRC=/, "")
        gsub(/%XUI_DOCUMENT_SRC%/, "")
        gsub(/\r/, "")
        gsub(/\\/, "/")
        printf "%s ", $0
    }
' tools/build/xui_document_sources.bat)
set -- $sources

"$CC" -std=c11 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie \
    -Wall -Wextra -Werror -Wno-unused-function @config/includes.rsp \
    -o "$OUT_DIR/xui_document_test_sanitizer" \
    tests/xui/xui_document_test.c "$@" tests/xui/xui_test_xrt_impl.c \
    -lm -ldl -lpthread

ASAN_OPTIONS=${ASAN_OPTIONS:-detect_leaks=1:halt_on_error=1}
UBSAN_OPTIONS=${UBSAN_OPTIONS:-halt_on_error=1}
export ASAN_OPTIONS UBSAN_OPTIONS
"$OUT_DIR/xui_document_test_sanitizer"

"$CC" -std=c11 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie \
    -Wall -Wextra -Werror -Wno-unused-function @config/includes.rsp \
    -o "$OUT_DIR/xui_document_quote_nested_sanitizer" \
    tests/xui/xui_document_quote_nested_test.c "$@" tests/xui/xui_test_xrt_impl.c \
    -lm -ldl -lpthread
"$OUT_DIR/xui_document_quote_nested_sanitizer"

"$CC" -std=c11 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie \
    -Wall -Wextra -Werror -Wno-unused-function @config/includes.rsp \
    -o "$OUT_DIR/xui_document_quote_source_sanitizer" \
    tests/xui/xui_document_quote_source_test.c "$@" tests/xui/xui_test_xrt_impl.c \
    -lm -ldl -lpthread
"$OUT_DIR/xui_document_quote_source_sanitizer"

"$CC" -std=c11 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie \
    -Wall -Wextra -Werror -Wno-unused-function @config/includes.rsp \
    -o "$OUT_DIR/xui_document_quote_prefix_sanitizer" \
    tests/xui/xui_document_quote_prefix_test.c "$@" tests/xui/xui_test_xrt_impl.c \
    -lm -ldl -lpthread
"$OUT_DIR/xui_document_quote_prefix_sanitizer"

"$CC" -std=c11 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie \
    -Wall -Wextra -Werror -Wno-unused-function @config/includes.rsp \
    -o "$OUT_DIR/xui_document_footnote_prefix_sanitizer" \
    tests/xui/xui_document_footnote_prefix_test.c "$@" tests/xui/xui_test_xrt_impl.c \
    -lm -ldl -lpthread
"$OUT_DIR/xui_document_footnote_prefix_sanitizer"

"$CC" -std=c11 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie \
    -Wall -Wextra -Werror -Wno-unused-function @config/includes.rsp \
    -o "$OUT_DIR/xui_document_tab_prefix_sanitizer" \
    tests/xui/xui_document_tab_prefix_test.c "$@" tests/xui/xui_test_xrt_impl.c \
    -lm -ldl -lpthread
"$OUT_DIR/xui_document_tab_prefix_sanitizer"

"$CC" -std=c11 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie \
    -Wall -Wextra -Werror -Wno-unused-function @config/includes.rsp \
    -o "$OUT_DIR/xui_document_source_sharing_sanitizer" \
    tests/xui/xui_document_source_sharing_test.c "$@" tests/xui/xui_test_xrt_impl.c \
    -lm -ldl -lpthread
"$OUT_DIR/xui_document_source_sharing_sanitizer"

"$CC" -std=c11 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie \
    -Wall -Wextra -Werror -Wno-unused-function @config/includes.rsp \
    -o "$OUT_DIR/xui_document_reference_sharing_sanitizer" \
    tests/xui/xui_document_reference_sharing_test.c "$@" tests/xui/xui_test_xrt_impl.c \
    -lm -ldl -lpthread
"$OUT_DIR/xui_document_reference_sharing_sanitizer"

"$CC" -std=c11 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie \
    -Wall -Wextra -Werror -Wno-unused-function @config/includes.rsp \
    -o "$OUT_DIR/xui_document_source_compaction_sanitizer" \
    tests/xui/xui_document_source_compaction_test.c "$@" tests/xui/xui_test_xrt_impl.c \
    -lm -ldl -lpthread
"$OUT_DIR/xui_document_source_compaction_sanitizer"

"$CC" -std=c11 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie \
    -Wall -Wextra -Werror -Wno-unused-function @config/includes.rsp \
    -o "$OUT_DIR/xui_document_definition_alignment_sanitizer" \
    tests/xui/xui_document_definition_alignment_test.c "$@" tests/xui/xui_test_xrt_impl.c \
    -lm -ldl -lpthread
"$OUT_DIR/xui_document_definition_alignment_sanitizer"

"$CC" -std=c11 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie \
    -Wall -Wextra -Werror -Wno-unused-function -DXUI_DOC_REFERENCE_HASH_COLLISION_TEST @config/includes.rsp \
    -o "$OUT_DIR/xui_document_definition_alignment_collision_sanitizer" \
    tests/xui/xui_document_definition_alignment_test.c "$@" tests/xui/xui_test_xrt_impl.c \
    -lm -ldl -lpthread
"$OUT_DIR/xui_document_definition_alignment_collision_sanitizer"

"$CC" -std=c11 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie \
    -Wall -Wextra -Werror -Wno-unused-function @config/includes.rsp \
    -o "$OUT_DIR/xui_document_dependency_sanitizer" \
    tests/xui/xui_document_dependency_test.c "$@" tests/xui/xui_test_xrt_impl.c \
    -lm -ldl -lpthread
"$OUT_DIR/xui_document_dependency_sanitizer"

"$CC" -std=c11 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie \
    -Wall -Wextra -Werror -Wno-unused-function -DXUI_DOC_REFERENCE_HASH_COLLISION_TEST @config/includes.rsp \
    -o "$OUT_DIR/xui_document_dependency_collision_sanitizer" \
    tests/xui/xui_document_dependency_test.c "$@" tests/xui/xui_test_xrt_impl.c \
    -lm -ldl -lpthread
"$OUT_DIR/xui_document_dependency_collision_sanitizer"

"$CC" -std=c11 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie \
    -Wall -Wextra -Werror -Wno-unused-function @config/includes.rsp \
    -o "$OUT_DIR/xui_document_break_origin_sanitizer" \
    tests/xui/xui_document_break_origin_test.c "$@" tests/xui/xui_test_xrt_impl.c \
    -lm -ldl -lpthread
"$OUT_DIR/xui_document_break_origin_sanitizer"

"$CC" -std=c11 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie \
    -Wall -Wextra -Werror -Wno-unused-function -DXUI_DOC_REFERENCE_HASH_COLLISION_TEST @config/includes.rsp \
    -o "$OUT_DIR/xui_document_break_origin_collision_sanitizer" \
    tests/xui/xui_document_break_origin_test.c "$@" tests/xui/xui_test_xrt_impl.c \
    -lm -ldl -lpthread
"$OUT_DIR/xui_document_break_origin_collision_sanitizer"

"$CC" -std=c11 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie \
    -Wall -Wextra -Werror -Wno-unused-function @config/includes.rsp \
    -o "$OUT_DIR/xui_document_front_matter_lines_sanitizer" \
    tests/xui/xui_document_front_matter_lines_test.c "$@" tests/xui/xui_test_xrt_impl.c \
    -lm -ldl -lpthread
"$OUT_DIR/xui_document_front_matter_lines_sanitizer"

"$CC" -std=c11 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie \
    -Wall -Wextra -Werror -Wno-unused-function -DXUI_DOC_REFERENCE_HASH_COLLISION_TEST @config/includes.rsp \
    -o "$OUT_DIR/xui_document_front_matter_lines_collision_sanitizer" \
    tests/xui/xui_document_front_matter_lines_test.c "$@" tests/xui/xui_test_xrt_impl.c \
    -lm -ldl -lpthread
"$OUT_DIR/xui_document_front_matter_lines_collision_sanitizer"

"$CC" -std=c11 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie \
    -Wall -Wextra -Werror -Wno-unused-function @config/includes.rsp \
    -o "$OUT_DIR/xui_document_object_break_origin_sanitizer" \
    tests/xui/xui_document_object_break_origin_test.c "$@" tests/xui/xui_test_xrt_impl.c \
    -lm -ldl -lpthread
"$OUT_DIR/xui_document_object_break_origin_sanitizer"

"$CC" -std=c11 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie \
    -Wall -Wextra -Werror -Wno-unused-function -DXUI_DOC_REFERENCE_HASH_COLLISION_TEST @config/includes.rsp \
    -o "$OUT_DIR/xui_document_object_break_origin_collision_sanitizer" \
    tests/xui/xui_document_object_break_origin_test.c "$@" tests/xui/xui_test_xrt_impl.c \
    -lm -ldl -lpthread
"$OUT_DIR/xui_document_object_break_origin_collision_sanitizer"

"$CC" -std=c11 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie \
    -Wall -Wextra -Werror -Wno-unused-function @config/includes.rsp \
    -o "$OUT_DIR/xui_document_text_origin_sanitizer" \
    tests/xui/xui_document_text_origin_test.c "$@" tests/xui/xui_test_xrt_impl.c \
    -lm -ldl -lpthread
"$OUT_DIR/xui_document_text_origin_sanitizer"

"$CC" -std=c11 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie \
    -Wall -Wextra -Werror -Wno-unused-function -DXUI_DOC_REFERENCE_HASH_COLLISION_TEST @config/includes.rsp \
    -o "$OUT_DIR/xui_document_text_origin_collision_sanitizer" \
    tests/xui/xui_document_text_origin_test.c "$@" tests/xui/xui_test_xrt_impl.c \
    -lm -ldl -lpthread
"$OUT_DIR/xui_document_text_origin_collision_sanitizer"

"$CC" -std=c11 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie \
    -Wall -Wextra -Werror -Wno-unused-function @config/includes.rsp \
    -o "$OUT_DIR/xui_document_fence_boundary_sanitizer" \
    tests/xui/xui_document_fence_boundary_test.c "$@" tests/xui/xui_test_xrt_impl.c \
    -lm -ldl -lpthread
"$OUT_DIR/xui_document_fence_boundary_sanitizer"

"$CC" -std=c11 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie \
    -Wall -Wextra -Werror -Wno-unused-function -DXUI_DOC_REFERENCE_HASH_COLLISION_TEST @config/includes.rsp \
    -o "$OUT_DIR/xui_document_fence_boundary_collision_sanitizer" \
    tests/xui/xui_document_fence_boundary_test.c "$@" tests/xui/xui_test_xrt_impl.c \
    -lm -ldl -lpthread
"$OUT_DIR/xui_document_fence_boundary_collision_sanitizer"

"$CC" -std=c11 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie \
    -Wall -Wextra -Werror -Wno-unused-function @config/includes.rsp \
    -o "$OUT_DIR/xui_document_inline_whitespace_sanitizer" \
    tests/xui/xui_document_inline_whitespace_test.c "$@" tests/xui/xui_test_xrt_impl.c \
    -lm -ldl -lpthread
"$OUT_DIR/xui_document_inline_whitespace_sanitizer"

"$CC" -std=c11 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie \
    -Wall -Wextra -Werror -Wno-unused-function -DXUI_DOC_REFERENCE_HASH_COLLISION_TEST @config/includes.rsp \
    -o "$OUT_DIR/xui_document_inline_whitespace_collision_sanitizer" \
    tests/xui/xui_document_inline_whitespace_test.c "$@" tests/xui/xui_test_xrt_impl.c \
    -lm -ldl -lpthread
"$OUT_DIR/xui_document_inline_whitespace_collision_sanitizer"

"$CC" -std=c11 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie \
    -Wall -Wextra -Werror -Wno-unused-function @config/includes.rsp \
    -o "$OUT_DIR/xui_document_nested_image_sanitizer" \
    tests/xui/xui_document_nested_image_test.c "$@" tests/xui/xui_test_xrt_impl.c \
    -lm -ldl -lpthread
"$OUT_DIR/xui_document_nested_image_sanitizer"

"$CC" -std=c11 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie \
    -Wall -Wextra -Werror -Wno-unused-function -DXUI_DOC_REFERENCE_HASH_COLLISION_TEST @config/includes.rsp \
    -o "$OUT_DIR/xui_document_nested_image_collision_sanitizer" \
    tests/xui/xui_document_nested_image_test.c "$@" tests/xui/xui_test_xrt_impl.c \
    -lm -ldl -lpthread
"$OUT_DIR/xui_document_nested_image_collision_sanitizer"

"$CC" -std=c11 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie \
    -Wall -Wextra -Werror -Wno-unused-function @config/includes.rsp \
    -o "$OUT_DIR/xui_document_quote_unwrap_source_sanitizer" \
    tests/xui/xui_document_quote_unwrap_source_test.c "$@" tests/xui/xui_test_xrt_impl.c \
    -lm -ldl -lpthread
"$OUT_DIR/xui_document_quote_unwrap_source_sanitizer"

"$CC" -std=c11 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie \
    -Wall -Wextra -Werror -Wno-unused-function @config/includes.rsp \
    -o "$OUT_DIR/xui_document_quote_boundary_sanitizer" \
    tests/xui/xui_document_quote_boundary_test.c "$@" tests/xui/xui_test_xrt_impl.c \
    -lm -ldl -lpthread
"$OUT_DIR/xui_document_quote_boundary_sanitizer"

"$CC" -std=c11 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie \
    -Wall -Wextra -Werror -Wno-unused-function @config/includes.rsp \
    -o "$OUT_DIR/xui_document_admonition_source_sanitizer" \
    tests/xui/xui_document_admonition_source_test.c "$@" tests/xui/xui_test_xrt_impl.c \
    -lm -ldl -lpthread
"$OUT_DIR/xui_document_admonition_source_sanitizer"

"$CC" -std=c11 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie \
    -Wall -Wextra -Werror -Wno-unused-function @config/includes.rsp \
    -o "$OUT_DIR/xui_document_footnote_unwrap_source_sanitizer" \
    tests/xui/xui_document_footnote_unwrap_source_test.c "$@" tests/xui/xui_test_xrt_impl.c \
    -lm -ldl -lpthread
"$OUT_DIR/xui_document_footnote_unwrap_source_sanitizer"

"$CC" -std=c11 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie \
    -Wall -Wextra -Werror -Wno-unused-function @config/includes.rsp \
    -o "$OUT_DIR/xui_document_unlist_source_sanitizer" \
    tests/xui/xui_document_unlist_source_test.c "$@" tests/xui/xui_test_xrt_impl.c \
    -lm -ldl -lpthread
"$OUT_DIR/xui_document_unlist_source_sanitizer"

"$CC" -std=c11 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie \
    -Wall -Wextra -Werror -Wno-unused-function @config/includes.rsp \
    -o "$OUT_DIR/xui_document_image_field_source_sanitizer" \
    tests/xui/xui_document_image_field_source_test.c "$@" tests/xui/xui_test_xrt_impl.c \
    -lm -ldl -lpthread
"$OUT_DIR/xui_document_image_field_source_sanitizer"

"$CC" -std=c11 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie \
    -Wall -Wextra -Werror -Wno-unused-function -DXUI_DOC_REFERENCE_HASH_COLLISION_TEST @config/includes.rsp \
    -o "$OUT_DIR/xui_document_image_field_source_collision_sanitizer" \
    tests/xui/xui_document_image_field_source_test.c "$@" tests/xui/xui_test_xrt_impl.c \
    -lm -ldl -lpthread
"$OUT_DIR/xui_document_image_field_source_collision_sanitizer"

"$CC" -std=c11 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie \
    -Wall -Wextra -Werror -Wno-unused-function @config/includes.rsp \
    -o "$OUT_DIR/xui_document_link_field_cst_sanitizer" \
    tests/xui/xui_document_link_field_cst_test.c "$@" tests/xui/xui_test_xrt_impl.c \
    -lm -ldl -lpthread
"$OUT_DIR/xui_document_link_field_cst_sanitizer"

"$CC" -std=c11 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie \
    -Wall -Wextra -Werror -Wno-unused-function -DXUI_DOC_REFERENCE_HASH_COLLISION_TEST @config/includes.rsp \
    -o "$OUT_DIR/xui_document_link_field_cst_collision_sanitizer" \
    tests/xui/xui_document_link_field_cst_test.c "$@" tests/xui/xui_test_xrt_impl.c \
    -lm -ldl -lpthread
"$OUT_DIR/xui_document_link_field_cst_collision_sanitizer"

"$CC" -std=c11 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie \
    -Wall -Wextra -Werror -Wno-unused-function @config/includes.rsp \
    -o "$OUT_DIR/xui_document_link_command_fields_sanitizer" \
    tests/xui/xui_document_link_command_fields_test.c "$@" tests/xui/xui_test_xrt_impl.c \
    -lm -ldl -lpthread
"$OUT_DIR/xui_document_link_command_fields_sanitizer"

"$CC" -std=c11 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie \
    -Wall -Wextra -Werror -Wno-unused-function -DXUI_DOC_REFERENCE_HASH_COLLISION_TEST @config/includes.rsp \
    -o "$OUT_DIR/xui_document_link_command_fields_collision_sanitizer" \
    tests/xui/xui_document_link_command_fields_test.c "$@" tests/xui/xui_test_xrt_impl.c \
    -lm -ldl -lpthread
"$OUT_DIR/xui_document_link_command_fields_collision_sanitizer"

"$CC" -std=c11 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie \
    -Wall -Wextra -Werror -Wno-unused-function @config/includes.rsp \
    -o "$OUT_DIR/xui_document_link_owner_fields_sanitizer" \
    tests/xui/xui_document_link_owner_fields_test.c "$@" tests/xui/xui_test_xrt_impl.c \
    -lm -ldl -lpthread
"$OUT_DIR/xui_document_link_owner_fields_sanitizer"

"$CC" -std=c11 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie \
    -Wall -Wextra -Werror -Wno-unused-function -DXUI_DOC_REFERENCE_HASH_COLLISION_TEST @config/includes.rsp \
    -o "$OUT_DIR/xui_document_link_owner_fields_collision_sanitizer" \
    tests/xui/xui_document_link_owner_fields_test.c "$@" tests/xui/xui_test_xrt_impl.c \
    -lm -ldl -lpthread
"$OUT_DIR/xui_document_link_owner_fields_collision_sanitizer"

"$CC" -std=c11 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie \
    -Wall -Wextra -Werror -Wno-unused-function @config/includes.rsp \
    -o "$OUT_DIR/xui_document_unlink_fields_sanitizer" \
    tests/xui/xui_document_unlink_fields_test.c "$@" tests/xui/xui_test_xrt_impl.c \
    -lm -ldl -lpthread
"$OUT_DIR/xui_document_unlink_fields_sanitizer"

"$CC" -std=c11 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie \
    -Wall -Wextra -Werror -Wno-unused-function -DXUI_DOC_REFERENCE_HASH_COLLISION_TEST @config/includes.rsp \
    -o "$OUT_DIR/xui_document_unlink_fields_collision_sanitizer" \
    tests/xui/xui_document_unlink_fields_test.c "$@" tests/xui/xui_test_xrt_impl.c \
    -lm -ldl -lpthread
"$OUT_DIR/xui_document_unlink_fields_collision_sanitizer"

"$CC" -std=c11 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie \
    -Wall -Wextra -Werror -Wno-unused-function @config/includes.rsp \
    -o "$OUT_DIR/xui_document_source_index_sanitizer" \
    tests/xui/xui_document_source_index_test.c "$@" tests/xui/xui_test_xrt_impl.c \
    -lm -ldl -lpthread
"$OUT_DIR/xui_document_source_index_sanitizer"

"$CC" -std=c11 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie \
    -Wall -Wextra -Werror -Wno-unused-function -DXUI_DOC_REFERENCE_HASH_COLLISION_TEST @config/includes.rsp \
    -o "$OUT_DIR/xui_document_source_index_collision_sanitizer" \
    tests/xui/xui_document_source_index_test.c "$@" tests/xui/xui_test_xrt_impl.c \
    -lm -ldl -lpthread
"$OUT_DIR/xui_document_source_index_collision_sanitizer"

"$CC" -std=c11 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie \
    -Wall -Wextra -Werror -Wno-unused-function @config/includes.rsp \
    -o "$OUT_DIR/xui_document_prefix_binding_sanitizer" \
    tests/xui/xui_document_prefix_binding_test.c "$@" tests/xui/xui_test_xrt_impl.c \
    -lm -ldl -lpthread
"$OUT_DIR/xui_document_prefix_binding_sanitizer"

"$CC" -std=c11 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie \
    -Wall -Wextra -Werror -Wno-unused-function -DXUI_DOC_REFERENCE_HASH_COLLISION_TEST @config/includes.rsp \
    -o "$OUT_DIR/xui_document_prefix_binding_collision_sanitizer" \
    tests/xui/xui_document_prefix_binding_test.c "$@" tests/xui/xui_test_xrt_impl.c \
    -lm -ldl -lpthread
"$OUT_DIR/xui_document_prefix_binding_collision_sanitizer"

"$CC" -std=c11 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie \
    -Wall -Wextra -Werror -Wno-unused-function @config/includes.rsp \
    -o "$OUT_DIR/xui_document_source_working_sanitizer" \
    tests/xui/xui_document_source_working_test.c "$@" tests/xui/xui_test_xrt_impl.c \
    -lm -ldl -lpthread
"$OUT_DIR/xui_document_source_working_sanitizer"

"$CC" -std=c11 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie \
    -Wall -Wextra -Werror -Wno-unused-function -DXUI_DOC_REFERENCE_HASH_COLLISION_TEST @config/includes.rsp \
    -o "$OUT_DIR/xui_document_source_working_collision_sanitizer" \
    tests/xui/xui_document_source_working_test.c "$@" tests/xui/xui_test_xrt_impl.c \
    -lm -ldl -lpthread
"$OUT_DIR/xui_document_source_working_collision_sanitizer"

python3 tests/xui/build_document_cst_test.py --sanitizer --out-dir "$OUT_DIR/cst"

if [ -n "$CORPUS" ]; then
    "$CC" -std=c11 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie \
        -Wall -Wextra -Werror -Wno-unused-function @config/includes.rsp \
        -o "$OUT_DIR/xui_document_corpus_sanitizer" \
        tests/xui/xui_document_corpus_test.c "$@" tests/xui/xui_test_xrt_impl.c \
        -lm -ldl -lpthread
    "$OUT_DIR/xui_document_corpus_sanitizer" "$CORPUS"
fi

# Run the permanent lossless move matrix on fresh normal/collision kernels.
for move_variant in normal collision; do
    python3 tests/xui/build_document_move_test.py --sanitize --variant "$move_variant" \
        --out-dir "$OUT_DIR/moves-block-$move_variant"
    python3 tests/xui/build_document_container_move_test.py --sanitize --variant "$move_variant" \
        --out-dir "$OUT_DIR/moves-container-$move_variant"
done
