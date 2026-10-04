#ifndef XUI_DOCUMENT_REFERENCE_CACHE_ORACLE_H
#define XUI_DOCUMENT_REFERENCE_CACHE_ORACLE_H
#include "../src/xui_document_internal.h"
/* Inspect every stored payload independently of the production comparator. */
static void reference_cache_valid(doc_state* state)
{
    uint64_t i, count = doc_seq_size(state->reference_values);
    CHECK(doc_seq_size(state->references) == count * sizeof(xui_doc_reference_definition_t));
    for (i = 0; i < count; i++) {
        xui_doc_reference_definition_t ref; doc_sequence* value = doc_seq_get_value_item(state->reference_values, i);
        CHECK(doc_seq_read(state->references, i * sizeof(ref), &ref, sizeof(ref)) == XUI_OK &&
            ref.iKind == doc_seq_get_id(state->reference_values, i));
        {
            uint64_t header[4]; CHECK((ref.iKind == XUI_DOC_REFERENCE_LINK || ref.iKind == XUI_DOC_REFERENCE_FOOTNOTE) &&
                value && doc_seq_size(value) >= sizeof(header));
            CHECK(doc_seq_read(value, 0, header, sizeof(header)) == XUI_OK);
            CHECK(header[0] <= doc_seq_size(value) - sizeof(header) && header[1] <= doc_seq_size(value) - sizeof(header) - header[0] &&
                header[2] == doc_seq_size(value) - sizeof(header) - header[0] - header[1] && header[3] <= 1 &&
                header[3] == (uint64_t)(ref.iTitleStart != DOC_NONE));
            if (ref.iKind == XUI_DOC_REFERENCE_FOOTNOTE) CHECK(header[0] && !header[2] && !header[3]);
        }
    }
}
static void reference_cache_snapshot_equal(xui_document_snapshot a, xui_document_snapshot b)
{
    uint64_t i, count = doc_seq_size(a->state->reference_values);
    reference_cache_valid(a->state); reference_cache_valid(b->state);
    CHECK(count == doc_seq_size(b->state->reference_values));
    for (i = 0; i < count; i++) {
        doc_sequence *left = doc_seq_get_value_item(a->state->reference_values, i), *right = doc_seq_get_value_item(b->state->reference_values, i);
        CHECK(doc_seq_get_id(a->state->reference_values, i) == doc_seq_get_id(b->state->reference_values, i));
        if (left || right) {
            uint64_t at = 0; CHECK(left && right && doc_seq_size(left) == doc_seq_size(right));
            while (at < doc_seq_size(left)) {
                char a_bytes[1024], b_bytes[1024];
                uint64_t n = doc_seq_size(left) - at > sizeof(a_bytes) ? sizeof(a_bytes) : doc_seq_size(left) - at;
                CHECK(doc_seq_read(left, at, a_bytes, n) == XUI_OK && doc_seq_read(right, at, b_bytes, n) == XUI_OK &&
                    !memcmp(a_bytes, b_bytes, (size_t)n));
                at += n;
            }
        }
    }
}
#endif
