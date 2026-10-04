#include "../xui_config.h"
#if XUI_ENABLE_DOCUMENT
#include "xui_document_internal.h"

static int doc_reference_cancelled(const atomic_int* cancellation)
{ return cancellation ? atomic_load(cancellation) : XUI_OK; }
/* Values are derived metadata. Each ordinal has one item, so source-offset
 * shifts share the entire tree and field edits share unchanged interior byte ranges and copy ordinal paths. */
int doc_reference_value_append(doc_state* state, uint32_t kind,
    const char* label, uint64_t label_bytes, const char* destination, uint64_t destination_bytes,
    const char* title, uint64_t title_bytes, int has_title, const atomic_int* cancellation)
{
    doc_blob* blob = NULL; doc_sequence *value = NULL, *item = NULL, *next = NULL;
    uint64_t header[4] = {label_bytes, destination_bytes, title_bytes, !!has_title};
    uint64_t size = sizeof(header), offset, i; int result = doc_reference_cancelled(cancellation);
    if (result != XUI_OK) return result;
    if (kind == XUI_DOC_REFERENCE_LINK || kind == XUI_DOC_REFERENCE_FOOTNOTE) {
        const char* fields[3] = {label, destination, title};
        if (kind == XUI_DOC_REFERENCE_FOOTNOTE && (title_bytes || has_title)) return XUI_DOC_ERROR_FORMAT;
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
    } else return XUI_DOC_ERROR_FORMAT;
    if (result == XUI_OK) result = doc_reference_cancelled(cancellation);
    if (result == XUI_OK) {
        value = doc_seq_blob_range(state->allocator, blob, 0, size);
        if (!value) result = XUI_ERROR_OUT_OF_MEMORY;
        if (result == XUI_OK) item = doc_seq_value_item(state->allocator, kind, value);
        if (!item) result = XUI_ERROR_OUT_OF_MEMORY;
    }
    if (result == XUI_OK) result = doc_reference_cancelled(cancellation);
    if (result == XUI_OK) result = doc_seq_replace(state->allocator, state->reference_values,
        doc_seq_size(state->reference_values), doc_seq_size(state->reference_values), item, &next);
    if (result == XUI_OK) { doc_seq_release(state->reference_values); state->reference_values = next; next = NULL; }
    doc_seq_release(value); doc_seq_release(item); doc_seq_release(next); doc_blob_release(blob); return result;
}
int doc_reference_value_replace(doc_state* state, uint64_t index, doc_state* parsed, uint64_t parsed_index,
    const atomic_int* cancellation)
{
    doc_sequence *item = NULL, *value = NULL, *next = NULL; uint64_t kind; int result;
    if (index >= doc_seq_size(state->reference_values) || parsed_index >= doc_seq_size(parsed->reference_values))
        return XUI_DOC_ERROR_SCHEMA;
    kind = doc_seq_get_id(parsed->reference_values, parsed_index);
    result = doc_reference_cancelled(cancellation);
    if (result == XUI_OK && kind == doc_seq_get_id(state->reference_values, index))
        result = doc_seq_reuse_bytes(state->allocator, doc_seq_get_value_item(state->reference_values, index),
            doc_seq_get_value_item(parsed->reference_values, parsed_index), cancellation, &value);
    else if (result == XUI_OK) { value = doc_seq_get_value_item(parsed->reference_values, parsed_index); doc_seq_retain(value); }
    if (result == XUI_OK) {
        item = doc_seq_value_item(state->allocator, kind, value);
        if (!item) result = XUI_ERROR_OUT_OF_MEMORY;
    }
    if (result == XUI_OK) result = doc_seq_replace(state->allocator, state->reference_values, index, index + 1, item, &next);
    if (result == XUI_OK) result = doc_reference_cancelled(cancellation);
    if (result == XUI_OK) { doc_seq_release(state->reference_values); state->reference_values = next; next = NULL; }
    doc_seq_release(value); doc_seq_release(item); doc_seq_release(next); return result;
}
int doc_reference_values_equal(doc_state* before, doc_state* after, const atomic_int* cancellation, int* equal)
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
        doc_sequence *left = doc_seq_get_value_item(before->reference_values, i), *right = doc_seq_get_value_item(after->reference_values, i);
        result = doc_reference_cancelled(cancellation); if (result != XUI_OK) return result;
        if (kind != doc_seq_get_id(after->reference_values, i)) return XUI_OK;
        if ((kind != XUI_DOC_REFERENCE_LINK && kind != XUI_DOC_REFERENCE_FOOTNOTE) ||
            !left || !right || doc_seq_size(left) < 4 * sizeof(uint64_t) || doc_seq_size(right) < 4 * sizeof(uint64_t))
            return XUI_DOC_ERROR_SCHEMA;
        if (left == right) continue;
        if (doc_seq_size(left) != doc_seq_size(right)) return XUI_OK;
        for (at = 0; at < doc_seq_size(left); ) {
            uint64_t bytes = doc_seq_size(left) - at > 16384 ? 16384 : doc_seq_size(left) - at;
            result = doc_reference_cancelled(cancellation); if (result != XUI_OK) return result;
            {
                char a[16384], b[16384];
                if (doc_seq_read(left, at, a, bytes) != XUI_OK || doc_seq_read(right, at, b, bytes) != XUI_OK)
                    return XUI_DOC_ERROR_SCHEMA;
                if (memcmp(a, b, (size_t)bytes)) return XUI_OK;
            }
            at += bytes;
        }
    }
    *equal = 1; return XUI_OK;
}

static int doc_reference_item_equal(doc_sequence* before, uint64_t left_index,
    doc_sequence* after, uint64_t right_index, const atomic_int* cancellation, int* equal)
{
    uint64_t kind = doc_seq_get_id(before, left_index), at;
    doc_sequence *left = doc_seq_get_value_item(before, left_index),
        *right = doc_seq_get_value_item(after, right_index);
    int result = doc_reference_cancelled(cancellation);
    *equal = 0;
    if (result != XUI_OK) return result;
    if (!left || !right || doc_seq_size(left) < 4 * sizeof(uint64_t) || doc_seq_size(right) < 4 * sizeof(uint64_t) ||
        (kind != XUI_DOC_REFERENCE_LINK && kind != XUI_DOC_REFERENCE_FOOTNOTE)) return XUI_DOC_ERROR_SCHEMA;
    if (kind != doc_seq_get_id(after, right_index) || doc_seq_size(left) != doc_seq_size(right)) return XUI_OK;
    if (left != right) for (at = 0; at < doc_seq_size(left); ) {
        uint64_t bytes = doc_seq_size(left) - at > 16384 ? 16384 : doc_seq_size(left) - at;
        result = doc_reference_cancelled(cancellation); if (result != XUI_OK) return result;
        {
                char a[16384], b[16384];
                if (doc_seq_read(left, at, a, bytes) != XUI_OK || doc_seq_read(right, at, b, bytes) != XUI_OK)
                    return XUI_DOC_ERROR_SCHEMA;
                if (memcmp(a, b, (size_t)bytes)) return XUI_OK;
            }
        at += bytes;
    }
    *equal = 1; return XUI_OK;
}
/* Alignment reuses immutable contents, not definition identities. The new
 * parser remains authoritative for ordinal order and reference coordinates.
 * Exact typed contents are byte-proven even when hashes collide. Label keys
 * only choose a candidate for the common byte-range sharing algorithm. */
typedef struct doc_reference_key {
    doc_sequence* value; /* Borrowed from the retained input state. */
    uint64_t kind, hash, label_hash, label_bytes;
} doc_reference_key;
typedef struct doc_reference_alignment {
    doc_reference_key* keys;
    uint64_t* order;
    doc_sequence** pools; /* Lazy immutable byte ropes for ambiguous typed labels. */
    doc_allocator* allocator;
    doc_state* base; /* Borrowed immutable origin for sequential SOURCE edits. */
    uint64_t count;
    const atomic_int* cancellation;
} doc_reference_alignment;
static uint64_t doc_reference_hash_bytes(uint64_t hash, const unsigned char* bytes, uint64_t count)
{
    uint64_t i;
    for (i = 0; i < count; i++) hash = (hash ^ bytes[i]) * UINT64_C(1099511628211);
    return hash;
}
static int doc_reference_key_read(doc_sequence* values, uint64_t index,
    const atomic_int* cancellation, doc_reference_key* key, int label_only)
{
    uint64_t header[4], size, sum = sizeof(header), i, at;
    unsigned char buffer[16384]; int result;
    key->value = doc_seq_get_value_item(values, index); key->kind = doc_seq_get_id(values, index);
    if (!key->value || (key->kind != XUI_DOC_REFERENCE_LINK && key->kind != XUI_DOC_REFERENCE_FOOTNOTE)) return XUI_DOC_ERROR_SCHEMA;
    size = doc_seq_size(key->value);
    result = doc_seq_read(key->value, 0, header, sizeof(header)); if (result != XUI_OK) return XUI_DOC_ERROR_SCHEMA;
    if (header[3] > 1 || (key->kind == XUI_DOC_REFERENCE_FOOTNOTE && (header[2] || header[3]))) return XUI_DOC_ERROR_SCHEMA;
    for (i = 0; i < 3; i++) {
        if (header[i] > UINT64_MAX - sum) return XUI_DOC_ERROR_SCHEMA;
        sum += header[i];
    }
    if (sum != size) return XUI_DOC_ERROR_SCHEMA;
    key->label_bytes = header[0]; key->hash = key->label_hash = UINT64_C(14695981039346656037) ^ key->kind;
    if (label_only) size = sizeof(header) + header[0];
    for (at = 0; at < size;) {
        uint64_t n = size - at > sizeof(buffer) ? sizeof(buffer) : size - at;
        uint64_t begin = at > sizeof(header) ? at : sizeof(header);
        uint64_t end = at + n < sizeof(header) + header[0] ? at + n : sizeof(header) + header[0];
        result = doc_reference_cancelled(cancellation); if (result != XUI_OK) return result;
        if (doc_seq_read(key->value, at, buffer, n) != XUI_OK) return XUI_DOC_ERROR_SCHEMA;
        key->hash = doc_reference_hash_bytes(key->hash, buffer, n);
        if (begin < end) key->label_hash = doc_reference_hash_bytes(key->label_hash, buffer + begin - at, end - begin);
        at += n;
    }
#ifdef XUI_DOC_REFERENCE_HASH_COLLISION_TEST
    key->hash = key->label_hash = 0; /* Test builds still require complete byte proofs. */
#endif
    return doc_reference_cancelled(cancellation);
}
static int doc_reference_key_compare(const doc_reference_key* a, const doc_reference_key* b,
    int label, const atomic_int* cancellation, int* order)
{
    uint64_t x = label ? a->label_hash : a->hash, y = label ? b->label_hash : b->hash;
    uint64_t left = label ? a->label_bytes : doc_seq_size(a->value);
    uint64_t right = label ? b->label_bytes : doc_seq_size(b->value);
    uint64_t at = 0, limit = left < right ? left : right, offset = label ? 4 * sizeof(uint64_t) : 0;
    int result = doc_reference_cancelled(cancellation);
    *order = 0; if (result != XUI_OK) return result;
    if (x != y) { *order = x < y ? -1 : 1; return XUI_OK; }
    if (a->kind != b->kind) { *order = a->kind < b->kind ? -1 : 1; return XUI_OK; }
    if (a->value != b->value) while (at < limit) {
        unsigned char p[16384], q[16384]; uint64_t bytes = limit - at > sizeof(p) ? sizeof(p) : limit - at;
        result = doc_reference_cancelled(cancellation); if (result != XUI_OK) return result;
        if (doc_seq_read(a->value, offset + at, p, bytes) != XUI_OK ||
            doc_seq_read(b->value, offset + at, q, bytes) != XUI_OK) return XUI_DOC_ERROR_SCHEMA;
        *order = memcmp(p, q, (size_t)bytes); if (*order) return XUI_OK;
        at += bytes;
    }
    *order = left < right ? -1 : left != right;
    if (!*order && label == 2) {
        left = doc_seq_size(a->value); right = doc_seq_size(b->value);
        *order = left < right ? -1 : left != right;
    }
    return XUI_OK;
}
static int doc_reference_order_compare(doc_reference_alignment* c, uint64_t a, uint64_t b, int label, int* order)
{
    int result = doc_reference_key_compare(c->keys + a, c->keys + b, label ? 2 : 0, c->cancellation, order);
    if (result == XUI_OK && !*order) *order = a < b ? -1 : a != b;
    return result;
}
static int doc_reference_order_sift(doc_reference_alignment* c, uint64_t* order, uint64_t root, uint64_t count, int label)
{
    int result, comparison;
    while (root < count / 2) {
        uint64_t child = root * 2 + 1, swap;
        if (child + 1 < count) {
            result = doc_reference_order_compare(c, order[child], order[child + 1], label, &comparison);
            if (result != XUI_OK) return result;
            if (comparison < 0) child++;
        }
        result = doc_reference_order_compare(c, order[root], order[child], label, &comparison);
        if (result != XUI_OK) return result;
        if (comparison >= 0) break;
        swap = order[root]; order[root] = order[child]; order[child] = swap; root = child;
    }
    return XUI_OK;
}
static int doc_reference_order_sort(doc_reference_alignment* c, int label)
{
    uint64_t* order = c->order + (label ? c->count : 0), i; int result;
    for (i = c->count / 2; i;) {
        result = doc_reference_order_sift(c, order, --i, c->count, label); if (result != XUI_OK) return result;
    }
    for (i = c->count; i > 1;) {
        uint64_t swap = order[0]; order[0] = order[--i]; order[i] = swap;
        result = doc_reference_cancelled(c->cancellation); if (result != XUI_OK) return result;
        result = doc_reference_order_sift(c, order, 0, i, label); if (result != XUI_OK) return result;
    }
    return doc_reference_cancelled(c->cancellation);
}
static int doc_reference_key_find(doc_reference_alignment* c, const doc_reference_key* key,
    uint64_t preferred, int label, uint64_t* found)
{
    uint64_t low = 0, high = c->count, first, last, *order = c->order + (label ? c->count : 0);
    int result, comparison;
    *found = DOC_NONE;
    if (preferred < c->count) {
        result = doc_reference_key_compare(c->keys + preferred, key, label, c->cancellation, &comparison);
        if (result != XUI_OK) return result;
        if (!comparison && (!label || doc_seq_size(c->keys[preferred].value) == doc_seq_size(key->value))) {
            *found = preferred; return XUI_OK;
        }
    }
    while (low < high) {
        uint64_t mid = low + (high - low) / 2;
        result = doc_reference_key_compare(c->keys + order[mid], key, label, c->cancellation, &comparison);
        if (result != XUI_OK) return result;
        if (comparison < 0) low = mid + 1; else high = mid;
    }
    if (low == c->count) return XUI_OK;
    result = doc_reference_key_compare(c->keys + order[low], key, label, c->cancellation, &comparison);
    if (result != XUI_OK || comparison) return result;
    first = low; high = c->count;
    while (low < high) {
        uint64_t mid = low + (high - low) / 2;
        result = doc_reference_key_compare(c->keys + order[mid], key, label, c->cancellation, &comparison);
        if (result != XUI_OK) return result;
        if (comparison <= 0) low = mid + 1; else high = mid;
    }
    last = low;
    if (label) {
        uint64_t bytes = doc_seq_size(key->value), chosen, begin, end;
        low = first; high = last;
        while (low < high) {
            uint64_t mid = low + (high - low) / 2;
            if (doc_seq_size(c->keys[order[mid]].value) < bytes) low = mid + 1; else high = mid;
        }
        if (low == last) chosen = doc_seq_size(c->keys[order[last - 1]].value);
        else if (low == first || doc_seq_size(c->keys[order[low]].value) - bytes <=
            bytes - doc_seq_size(c->keys[order[low - 1]].value)) chosen = doc_seq_size(c->keys[order[low]].value);
        else chosen = doc_seq_size(c->keys[order[low - 1]].value);
        low = first; high = last;
        while (low < high) {
            uint64_t mid = low + (high - low) / 2;
            if (doc_seq_size(c->keys[order[mid]].value) < chosen) low = mid + 1; else high = mid;
        }
        begin = low; high = last;
        while (low < high) {
            uint64_t mid = low + (high - low) / 2;
            if (doc_seq_size(c->keys[order[mid]].value) <= chosen) low = mid + 1; else high = mid;
        }
        end = low; first = begin; last = end;
    }
    low = first; high = last;
    /* Equal contents/labels are ordinal ordered. Prefer the preceding run's
     * next item, otherwise the nearest ordinal, without scanning duplicates. */
    while (low < high) {
        uint64_t mid = low + (high - low) / 2;
        if (order[mid] < preferred) low = mid + 1; else high = mid;
    }
    if (low == last) *found = order[last - 1];
    else if (low == first || order[low] - preferred <= preferred - order[low - 1]) *found = order[low];
    else *found = order[low - 1];
    return doc_reference_cancelled(c->cancellation);
}
static int doc_reference_label_pool(doc_reference_alignment* c, const doc_reference_key* key,
    doc_sequence** pool)
{
    uint64_t low = 0, high = c->count, first, last, i, *order = c->order + c->count;
    doc_sequence *value = NULL, *next = NULL; int result, comparison;
    *pool = NULL;
    while (low < high) {
        uint64_t mid = low + (high - low) / 2;
        result = doc_reference_key_compare(c->keys + order[mid], key, 1, c->cancellation, &comparison);
        if (result != XUI_OK) return result;
        if (comparison < 0) low = mid + 1; else high = mid;
    }
    if (low == c->count) return XUI_OK;
    result = doc_reference_key_compare(c->keys + order[low], key, 1, c->cancellation, &comparison);
    if (result != XUI_OK || comparison) return result;
    first = low; high = c->count;
    while (low < high) {
        uint64_t mid = low + (high - low) / 2;
        result = doc_reference_key_compare(c->keys + order[mid], key, 1, c->cancellation, &comparison);
        if (result != XUI_OK) return result;
        if (comparison <= 0) low = mid + 1; else high = mid;
    }
    last = low;
    if (last - first < 2 && !c->base) return XUI_OK;
    if (!c->pools) {
        if (c->count > SIZE_MAX / sizeof(*c->pools)) return XUI_DOC_ERROR_LIMIT;
        c->pools = doc_alloc(c->allocator, (size_t)c->count * sizeof(*c->pools));
        if (!c->pools) return XUI_ERROR_OUT_OF_MEMORY;
        memset(c->pools, 0, (size_t)c->count * sizeof(*c->pools));
    }
    if (c->pools[first]) { *pool = c->pools[first]; return doc_reference_cancelled(c->cancellation); }
    for (i = first; i < last; i++) {
        result = doc_reference_cancelled(c->cancellation); if (result != XUI_OK) break;
        if (doc_seq_size(c->keys[order[i]].value) > UINT64_MAX - doc_seq_size(value)) {
            result = XUI_DOC_ERROR_LIMIT; break;
        }
        result = doc_seq_replace(c->allocator, value, doc_seq_size(value), doc_seq_size(value),
            c->keys[order[i]].value, &next);
        if (result != XUI_OK) break;
        doc_seq_release(value); value = next; next = NULL;
    }
    /* A previous synchronous patch can remove a value that a later patch
     * moves/reinserts. The immutable transaction origin still owns it. */
    if (result == XUI_OK && c->base) for (i = 0; i < doc_seq_size(c->base->reference_values); i++) {
        doc_reference_key base_key;
        result = doc_reference_key_read(c->base->reference_values, i, c->cancellation, &base_key, 1);
        if (result == XUI_OK) result = doc_reference_key_compare(&base_key, key, 1, c->cancellation, &comparison);
        if (result != XUI_OK) break;
        if (comparison) continue;
        if (doc_seq_size(base_key.value) > UINT64_MAX - doc_seq_size(value)) {
            result = XUI_DOC_ERROR_LIMIT; break;
        }
        result = doc_seq_replace(c->allocator, value, doc_seq_size(value), doc_seq_size(value), base_key.value, &next);
        if (result != XUI_OK) break;
        doc_seq_release(value); value = next; next = NULL;
    }
    if (result == XUI_OK) result = doc_reference_cancelled(c->cancellation);
    if (result == XUI_OK) { c->pools[first] = value; *pool = value; value = NULL; }
    doc_seq_release(value); doc_seq_release(next); return result;
}
static int doc_reference_append_run(doc_allocator* allocator, doc_sequence** output,
    doc_sequence* source, uint64_t start, uint64_t count, const atomic_int* cancellation)
{
    doc_sequence *tail = NULL, *run = NULL, *next = NULL; uint64_t end = doc_seq_size(*output); int result;
    if (!count) return XUI_OK;
    result = doc_reference_cancelled(cancellation); if (result != XUI_OK) return result;
    result = doc_seq_replace(allocator, source, start + count, doc_seq_size(source), NULL, &tail);
    if (result == XUI_OK) result = doc_seq_replace(allocator, tail, 0, start, NULL, &run);
    if (result == XUI_OK) result = doc_seq_replace(allocator, *output, end, end, run, &next);
    if (result == XUI_OK) { doc_seq_release(*output); *output = next; next = NULL; }
    doc_seq_release(tail); doc_seq_release(run); doc_seq_release(next); return result;
}
static int doc_reference_append_changed(doc_allocator* allocator, doc_sequence** middle,
    uint64_t kind, doc_sequence* old, doc_sequence* fresh, const atomic_int* cancellation)
{
    doc_sequence *value = NULL, *item = NULL, *next = NULL; uint64_t end = doc_seq_size(*middle); int result;
    if (old) result = doc_seq_reuse_bytes(allocator, old, fresh, cancellation, &value);
    else { value = fresh; doc_seq_retain(value); result = doc_reference_cancelled(cancellation); }
    if (result == XUI_OK) {
        item = doc_seq_value_item(allocator, kind, value); if (!item) result = XUI_ERROR_OUT_OF_MEMORY;
    }
    if (result == XUI_OK) result = doc_seq_replace(allocator, *middle, end, end, item, &next);
    if (result == XUI_OK) { doc_seq_release(*middle); *middle = next; next = NULL; }
    doc_seq_release(value); doc_seq_release(item); doc_seq_release(next); return result;
}
int doc_reference_values_reuse(doc_state* before, doc_state* after, doc_state* base, const atomic_int* cancellation)
{
    doc_sequence *next = NULL, *middle = NULL;
    doc_reference_alignment c = {0}; doc_reference_key key;
    uint64_t old_count = doc_seq_size(before->reference_values), new_count = doc_seq_size(after->reference_values);
    uint64_t i, prefix = 0, suffix = 0, old_middle, new_middle, preferred = 0, run = DOC_NONE, run_count = 0;
    int result = doc_reference_cancelled(cancellation), equal;
    if (result != XUI_OK) return result;
    if (before->allocator != after->allocator || (base && base->allocator != after->allocator) ||
        doc_seq_size(before->references) % sizeof(xui_doc_reference_definition_t) ||
        doc_seq_size(after->references) % sizeof(xui_doc_reference_definition_t) ||
        old_count != doc_seq_size(before->references) / sizeof(xui_doc_reference_definition_t) ||
        new_count != doc_seq_size(after->references) / sizeof(xui_doc_reference_definition_t)) return XUI_DOC_ERROR_SCHEMA;
    if (base && base->reference_values == before->reference_values) base = NULL;
    if (before->reference_values == after->reference_values || !new_count) return XUI_OK;
    if (!old_count) return base ? doc_reference_values_reuse(base, after, NULL, cancellation) : XUI_OK;
    while (prefix < old_count && prefix < new_count) {
        result = doc_reference_item_equal(before->reference_values, prefix, after->reference_values, prefix, cancellation, &equal);
        if (result != XUI_OK || !equal) break;
        prefix++;
    }
    while (result == XUI_OK && suffix < old_count - prefix && suffix < new_count - prefix) {
        result = doc_reference_item_equal(before->reference_values, old_count - suffix - 1,
            after->reference_values, new_count - suffix - 1, cancellation, &equal);
        if (result != XUI_OK || !equal) break;
        suffix++;
    }
    if (result != XUI_OK) goto done;
    old_middle = old_count - prefix - suffix; new_middle = new_count - prefix - suffix;
    if (old_count == new_count && !old_middle) {
        next = before->reference_values; doc_seq_retain(next); goto published;
    }
    if (!new_middle) goto splice;
    if (!old_middle && base) return doc_reference_values_reuse(base, after, NULL, cancellation);
    if (!old_middle) {
        result = doc_reference_append_run(after->allocator, &middle, after->reference_values,
            prefix, new_middle, cancellation);
        goto splice;
    }
    /* One changed ordinal remains a direct persistent replacement. */
    if (old_middle == 1 && new_middle == 1 && !base) {
        uint64_t kind = doc_seq_get_id(after->reference_values, prefix);
        doc_sequence* old = kind == doc_seq_get_id(before->reference_values, prefix) ? doc_seq_get_value_item(before->reference_values, prefix) : NULL;
        result = doc_reference_append_changed(after->allocator, &middle, kind, old,
            doc_seq_get_value_item(after->reference_values, prefix), cancellation);
        goto splice;
    }
    c.count = old_middle; c.cancellation = cancellation; c.allocator = after->allocator; c.base = base;
    if (c.count) {
        if (c.count > SIZE_MAX / sizeof(*c.keys) || c.count > SIZE_MAX / (2 * sizeof(*c.order))) { result = XUI_DOC_ERROR_LIMIT; goto done; }
        c.keys = doc_alloc(after->allocator, (size_t)c.count * sizeof(*c.keys));
        c.order = doc_alloc(after->allocator, (size_t)c.count * 2 * sizeof(*c.order));
        if (!c.keys || !c.order) { result = XUI_ERROR_OUT_OF_MEMORY; goto done; }
        for (i = 0; i < c.count && result == XUI_OK; i++) {
            result = doc_reference_key_read(before->reference_values, prefix + i, cancellation, c.keys + i, 0);
            c.order[i] = c.order[c.count + i] = i;
        }
        if (result == XUI_OK) result = doc_reference_order_sort(&c, 0);
        if (result == XUI_OK) result = doc_reference_order_sort(&c, 1);
    }
    for (i = 0; i < new_middle && result == XUI_OK; i++) {
        uint64_t exact = DOC_NONE, candidate = DOC_NONE;
        doc_sequence* pool = NULL;
        result = doc_reference_key_read(after->reference_values, prefix + i, cancellation, &key, 0);
        if (result == XUI_OK && c.count) result = doc_reference_key_find(&c, &key, preferred, 0, &exact);
        if (result != XUI_OK) break;
        if (exact != DOC_NONE) {
            if (run != DOC_NONE && exact != run + run_count) {
                result = doc_reference_append_run(after->allocator, &middle, before->reference_values, prefix + run, run_count, cancellation);
                run = DOC_NONE; run_count = 0;
            }
            if (run == DOC_NONE) run = exact;
            run_count++; preferred = exact + 1; continue;
        }
        if (run != DOC_NONE) {
            result = doc_reference_append_run(after->allocator, &middle, before->reference_values, prefix + run, run_count, cancellation);
            run = DOC_NONE; run_count = 0;
        }
        if (result == XUI_OK && c.count) result = doc_reference_key_find(&c, &key, preferred, 1, &candidate);
        /* Multiple values of the same typed label can have equal sizes and
         * different bodies. A shared input rope lets the common byte matcher
         * reuse proven ranges from any of them without guessing an identity. */
        if (result == XUI_OK && candidate != DOC_NONE)
            result = doc_reference_label_pool(&c, &key, &pool);
        if (result == XUI_OK && candidate == DOC_NONE && c.count) {
            uint64_t hint = preferred < c.count ? preferred : c.count - 1;
            if (c.keys[hint].kind == key.kind) candidate = hint;
        }
        if (result == XUI_OK) result = doc_reference_append_changed(after->allocator, &middle, key.kind,
            pool ? pool : (candidate == DOC_NONE ? NULL : c.keys[candidate].value), key.value, cancellation);
        if (candidate != DOC_NONE) preferred = candidate + 1;
    }
    if (result == XUI_OK && run != DOC_NONE) result = doc_reference_append_run(after->allocator, &middle,
        before->reference_values, prefix + run, run_count, cancellation);
splice:
    if (result == XUI_OK) result = doc_seq_replace(after->allocator, before->reference_values,
        prefix, old_count - suffix, middle, &next);
published:
    if (result == XUI_OK) result = doc_reference_cancelled(cancellation);
    if (result == XUI_OK && doc_seq_size(next) != new_count) result = XUI_DOC_ERROR_SCHEMA;
    if (result == XUI_OK) { doc_seq_release(after->reference_values); after->reference_values = next; next = NULL; }
done:
    doc_seq_release(next); doc_seq_release(middle);
    if (c.pools) for (i = 0; i < c.count; i++) doc_seq_release(c.pools[i]);
    doc_free(c.pools); doc_free(c.keys); doc_free(c.order); return result;
}

#endif
