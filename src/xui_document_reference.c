#include "../xui_config.h"
#if XUI_ENABLE_DOCUMENT
#include "xui_document_internal.h"

static int doc_reference_cancelled(const atomic_int* cancellation)
{ return cancellation ? atomic_load(cancellation) : XUI_OK; }
/* Values are derived metadata. Each ordinal has one item, so source-offset
 * shifts share the entire tree and a field edit copies only its search path. */
int doc_reference_value_append(doc_state* state, uint32_t kind,
    const char* label, uint64_t label_bytes, const char* destination, uint64_t destination_bytes,
    const char* title, uint64_t title_bytes, int has_title, const atomic_int* cancellation)
{
    doc_blob* blob = NULL; doc_sequence *item = NULL, *next = NULL;
    uint64_t header[4] = {label_bytes, destination_bytes, title_bytes, !!has_title};
    uint64_t size = sizeof(header), offset, i; int result = doc_reference_cancelled(cancellation);
    if (result != XUI_OK) return result;
    if (kind == XUI_DOC_REFERENCE_LINK) {
        const char* fields[3] = {label, destination, title};
        for (i = 0; i < 3; i++) {
            if ((!fields[i] && header[i]) || header[i] > UINT64_MAX - size) return XUI_DOC_ERROR_FORMAT;
            size += header[i];
        }
        blob = doc_blob_allocate(state->allocator, size);
        if (!blob) return XUI_ERROR_OUT_OF_MEMORY;
        memcpy(blob->data, header, sizeof(header)); offset = sizeof(header);
        for (i = 0; i < 3 && result == XUI_OK; i++) {
            uint64_t at = 0;
            while (at < header[i] && (result = doc_reference_cancelled(cancellation)) == XUI_OK) {
                uint64_t bytes = header[i] - at > 16384 ? 16384 : header[i] - at;
                memcpy(blob->data + offset + at, fields[i] + at, (size_t)bytes); at += bytes;
            }
            offset += header[i];
        }
    } else if (kind != XUI_DOC_REFERENCE_FOOTNOTE) return XUI_DOC_ERROR_FORMAT;
    if (result == XUI_OK) result = doc_reference_cancelled(cancellation);
    if (result == XUI_OK) {
        item = doc_seq_blob_item(state->allocator, kind, blob);
        if (!item) result = XUI_ERROR_OUT_OF_MEMORY;
    }
    if (result == XUI_OK) result = doc_reference_cancelled(cancellation);
    if (result == XUI_OK) result = doc_seq_replace(state->allocator, state->reference_values,
        doc_seq_size(state->reference_values), doc_seq_size(state->reference_values), item, &next);
    if (result == XUI_OK) { doc_seq_release(state->reference_values); state->reference_values = next; next = NULL; }
    doc_seq_release(item); doc_seq_release(next); doc_blob_release(blob); return result;
}
int doc_reference_value_replace(doc_state* state, uint64_t index, doc_state* parsed, uint64_t parsed_index)
{
    doc_sequence *item, *next = NULL; uint64_t kind; int result;
    if (index >= doc_seq_size(state->reference_values) || parsed_index >= doc_seq_size(parsed->reference_values))
        return XUI_DOC_ERROR_SCHEMA;
    kind = doc_seq_get_id(parsed->reference_values, parsed_index);
    item = doc_seq_blob_item(state->allocator, kind, doc_seq_get_blob_item(parsed->reference_values, parsed_index));
    if (!item) return XUI_ERROR_OUT_OF_MEMORY;
    result = doc_seq_replace(state->allocator, state->reference_values, index, index + 1, item, &next);
    if (result == XUI_OK) { doc_seq_release(state->reference_values); state->reference_values = next; next = NULL; }
    doc_seq_release(item); doc_seq_release(next); return result;
}
int doc_reference_link_values_equal(doc_state* before, doc_state* after, const atomic_int* cancellation, int* equal)
{
    uint64_t count = doc_seq_size(before->reference_values), i; int result;
    *equal = 0;
    result = doc_reference_cancelled(cancellation); if (result != XUI_OK) return result;
    if (doc_seq_size(before->references) % sizeof(xui_doc_reference_definition_t) ||
        doc_seq_size(after->references) % sizeof(xui_doc_reference_definition_t) ||
        count != doc_seq_size(before->references) / sizeof(xui_doc_reference_definition_t) ||
        doc_seq_size(after->reference_values) != doc_seq_size(after->references) / sizeof(xui_doc_reference_definition_t))
        return XUI_DOC_ERROR_SCHEMA;
    if (count != doc_seq_size(after->reference_values)) return XUI_OK;
    if (before->reference_values == after->reference_values) { *equal = 1; return XUI_OK; }
    for (i = 0; i < count; i++) {
        uint64_t kind = doc_seq_get_id(before->reference_values, i), at;
        doc_blob *left = doc_seq_get_blob_item(before->reference_values, i), *right = doc_seq_get_blob_item(after->reference_values, i);
        result = doc_reference_cancelled(cancellation); if (result != XUI_OK) return result;
        if (kind != doc_seq_get_id(after->reference_values, i)) return XUI_OK;
        if (kind == XUI_DOC_REFERENCE_FOOTNOTE) { if (left || right) return XUI_DOC_ERROR_SCHEMA; continue; }
        if (kind != XUI_DOC_REFERENCE_LINK || !left || !right || left->size < 4 * sizeof(uint64_t) || right->size < 4 * sizeof(uint64_t))
            return XUI_DOC_ERROR_SCHEMA;
        if (left == right) continue;
        if (left->size != right->size) return XUI_OK;
        for (at = 0; at < left->size; ) {
            uint64_t bytes = left->size - at > 16384 ? 16384 : left->size - at;
            result = doc_reference_cancelled(cancellation); if (result != XUI_OK) return result;
            if (memcmp(left->data + at, right->data + at, (size_t)bytes)) return XUI_OK;
            at += bytes;
        }
    }
    *equal = 1; return XUI_OK;
}

#endif
