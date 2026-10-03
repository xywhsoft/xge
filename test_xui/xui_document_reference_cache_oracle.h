#ifndef XUI_DOCUMENT_REFERENCE_CACHE_ORACLE_H
#define XUI_DOCUMENT_REFERENCE_CACHE_ORACLE_H
#include "../src/xui_document_internal.h"
/* Inspect every stored payload independently of the production comparator. */
static void reference_cache_valid(doc_state* state)
{
    uint64_t i, count = doc_seq_size(state->reference_values);
    CHECK(doc_seq_size(state->references) == count * sizeof(xui_doc_reference_definition_t));
    for (i = 0; i < count; i++) {
        xui_doc_reference_definition_t ref; doc_blob* value = doc_seq_get_blob_item(state->reference_values, i);
        CHECK(doc_seq_read(state->references, i * sizeof(ref), &ref, sizeof(ref)) == XUI_OK &&
            ref.iKind == doc_seq_get_id(state->reference_values, i));
        if (ref.iKind == XUI_DOC_REFERENCE_FOOTNOTE) CHECK(!value);
        else {
            uint64_t header[4]; CHECK(ref.iKind == XUI_DOC_REFERENCE_LINK && value && value->size >= sizeof(header));
            memcpy(header, value->data, sizeof(header));
            CHECK(header[0] <= value->size - sizeof(header) && header[1] <= value->size - sizeof(header) - header[0] &&
                header[2] == value->size - sizeof(header) - header[0] - header[1] && header[3] <= 1 &&
                header[3] == (uint64_t)(ref.iTitleStart != DOC_NONE));
        }
    }
}
static void reference_cache_snapshot_equal(xui_document_snapshot a, xui_document_snapshot b)
{
    uint64_t i, count = doc_seq_size(a->state->reference_values);
    reference_cache_valid(a->state); reference_cache_valid(b->state);
    CHECK(count == doc_seq_size(b->state->reference_values));
    for (i = 0; i < count; i++) {
        doc_blob *left = doc_seq_get_blob_item(a->state->reference_values, i), *right = doc_seq_get_blob_item(b->state->reference_values, i);
        CHECK(doc_seq_get_id(a->state->reference_values, i) == doc_seq_get_id(b->state->reference_values, i));
        if (left || right) CHECK(left && right && left->size == right->size && !memcmp(left->data, right->data, (size_t)left->size));
    }
}
#endif
