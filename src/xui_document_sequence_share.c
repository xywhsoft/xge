#include "../xui_config.h"
#if XUI_ENABLE_DOCUMENT
#include "xui_document_internal.h"
static int doc_seq_share_cancelled(const atomic_int* cancellation)
{ return cancellation ? atomic_load(cancellation) : XUI_OK; }
/* Private derived-value storage. Every retained range is byte-proven; hashes
 * only locate candidates and never establish content equality. */
static int doc_seq_share_common(doc_sequence* old, uint64_t x, doc_sequence* fresh,
    uint64_t y, uint64_t limit, int backwards, const atomic_int* cancellation, uint64_t* common)
{
    unsigned char a[16384], b[16384]; int result;
    *common = 0;
    while (*common < limit) {
        uint64_t n = limit - *common > sizeof(a) ? sizeof(a) : limit - *common, i;
        uint64_t at = backwards ? x - *common - n : x + *common;
        uint64_t bt = backwards ? y - *common - n : y + *common;
        result = doc_seq_share_cancelled(cancellation); if (result != XUI_OK) return result;
        if (doc_seq_read(old, at, a, n) != XUI_OK || doc_seq_read(fresh, bt, b, n) != XUI_OK)
            return XUI_DOC_ERROR_SCHEMA;
        for (i = 0; i < n; i++) {
            uint64_t j = backwards ? n - i - 1 : i;
            if (a[j] != b[j]) { *common += i; return XUI_OK; }
        }
        *common += n;
    }
    return doc_seq_share_cancelled(cancellation);
}
static int doc_seq_share_value_append_range(doc_allocator* allocator, doc_sequence** value,
    doc_sequence* source, uint64_t at, uint64_t bytes, int shared, const atomic_int* cancellation)
{
    doc_sequence *tail = NULL, *piece = NULL, *next = NULL; doc_blob* blob = NULL;
    uint64_t copied = 0, end = doc_seq_size(*value); int result;
    if (!bytes) return XUI_OK;
    result = doc_seq_share_cancelled(cancellation); if (result != XUI_OK) return result;
    if (shared) {
        if (!at && bytes == doc_seq_size(source)) { piece = source; doc_seq_retain(piece); }
        else {
            result = doc_seq_replace(allocator, source, at + bytes, doc_seq_size(source), NULL, &tail);
            if (result == XUI_OK) result = doc_seq_replace(allocator, tail, 0, at, NULL, &piece);
        }
    } else {
        blob = doc_blob_allocate(allocator, bytes); if (!blob) result = XUI_ERROR_OUT_OF_MEMORY;
        while (copied < bytes && result == XUI_OK) {
            uint64_t n = bytes - copied > 16384 ? 16384 : bytes - copied;
            result = doc_seq_share_cancelled(cancellation);
            if (result == XUI_OK && doc_seq_read(source, at + copied, blob->data + copied, n) != XUI_OK)
                result = XUI_DOC_ERROR_SCHEMA;
            copied += n;
        }
        if (result == XUI_OK) {
            piece = doc_seq_blob_range(allocator, blob, 0, bytes);
            if (!piece) result = XUI_ERROR_OUT_OF_MEMORY;
        }
    }
    if (result == XUI_OK) result = doc_seq_replace(allocator, *value, end, end, piece, &next);
    if (result == XUI_OK) { doc_seq_release(*value); *value = next; next = NULL; }
    doc_seq_release(tail); doc_seq_release(piece); doc_seq_release(next); doc_blob_release(blob); return result;
}
#define DOC_SEQ_SHARE_MATCH_BYTES 64
typedef struct doc_seq_share_match { uint64_t hash, next; } doc_seq_share_match;
typedef struct doc_seq_share_reader {
    doc_sequence* value;
    uint64_t at, bytes;
    unsigned char buffer[16384];
} doc_seq_share_reader;
static int doc_seq_share_byte(doc_seq_share_reader* r, uint64_t at, unsigned char* byte)
{
    if (at < r->at || at - r->at >= r->bytes) {
        uint64_t size = doc_seq_size(r->value);
        if (at >= size) return XUI_DOC_ERROR_SCHEMA;
        r->at = at; r->bytes = size - at > sizeof(r->buffer) ? sizeof(r->buffer) : size - at;
        if (doc_seq_read(r->value, at, r->buffer, r->bytes) != XUI_OK) return XUI_DOC_ERROR_SCHEMA;
    }
    *byte = r->buffer[at - r->at]; return XUI_OK;
}
static uint64_t doc_seq_share_hash(const unsigned char bytes[DOC_SEQ_SHARE_MATCH_BYTES])
{
    unsigned i; uint64_t hash = 0;
    for (i = 0; i < DOC_SEQ_SHARE_MATCH_BYTES; i++) hash = hash * 257 + bytes[i] + 1;
    return hash;
}
static uint64_t doc_seq_share_slot(uint64_t hash, uint64_t capacity)
{ return (hash ^ (hash >> 32)) & (capacity - 1); }
typedef struct doc_seq_share_coverage { doc_blob* blob; uint64_t bytes; } doc_seq_share_coverage;
typedef struct doc_seq_share_compaction {
    doc_allocator* allocator;
    doc_sequence *source, *output;
    doc_seq_share_coverage* entries;
    uint64_t count, last, visits;
    const atomic_int* cancellation;
} doc_seq_share_compaction;
static int doc_seq_share_collect(doc_sequence* p, doc_seq_share_coverage* entries,
    uint64_t* count, uint64_t* visits, const atomic_int* cancellation)
{
    int result;
    if (!p) return XUI_OK;
    if (!((*visits)++ & 255) && (result = doc_seq_share_cancelled(cancellation)) != XUI_OK) return result;
    result = doc_seq_share_collect(p->left, entries, count, visits, cancellation);
    if (result != XUI_OK) return result;
    if (!p->blob || p->value || p->id || p->offset > p->blob->size ||
        p->length > p->blob->size - p->offset) return XUI_DOC_ERROR_SCHEMA;
    if (*count == UINT64_MAX) return XUI_DOC_ERROR_LIMIT;
    if (entries) { entries[*count].blob = p->blob; entries[*count].bytes = p->length; }
    (*count)++;
    return doc_seq_share_collect(p->right, entries, count, visits, cancellation);
}
/* In-place heap sort bounds both temporary storage and cancellation latency;
 * qsort cannot observe cancellation during a large fragmented-source scan. */
static int doc_seq_share_coverage_sift(doc_seq_share_coverage* entries,
    uint64_t root, uint64_t count, uint64_t* visits, const atomic_int* cancellation)
{
    while (root < count / 2) {
        uint64_t child = root * 2 + 1; doc_seq_share_coverage swap; int result;
        if (!((*visits)++ & 255) && (result = doc_seq_share_cancelled(cancellation)) != XUI_OK) return result;
        if (child + 1 < count && (uintptr_t)entries[child].blob < (uintptr_t)entries[child + 1].blob) child++;
        if ((uintptr_t)entries[root].blob >= (uintptr_t)entries[child].blob) break;
        swap = entries[root]; entries[root] = entries[child]; entries[child] = swap; root = child;
    }
    return XUI_OK;
}
static int doc_seq_share_coverage_sort(doc_seq_share_coverage* entries,
    uint64_t count, uint64_t* visits, const atomic_int* cancellation)
{
    uint64_t i; int result;
    for (i = count / 2; i;) {
        result = doc_seq_share_coverage_sift(entries, --i, count, visits, cancellation);
        if (result != XUI_OK) return result;
    }
    for (i = count; i > 1;) {
        doc_seq_share_coverage swap = entries[0];
        if (!((*visits)++ & 255) && (result = doc_seq_share_cancelled(cancellation)) != XUI_OK) return result;
        entries[0] = entries[--i]; entries[i] = swap;
        result = doc_seq_share_coverage_sift(entries, 0, i, visits, cancellation);
        if (result != XUI_OK) return result;
    }
    return doc_seq_share_cancelled(cancellation);
}
static int doc_seq_share_sparse(const doc_seq_share_coverage* entry)
{
    return entry->blob->size &&
        entry->bytes <= (entry->blob->size - 1) / 4;
}
static int doc_seq_share_compact_walk(doc_seq_share_compaction* c, doc_sequence* p, uint64_t base)
{
    uint64_t low = 0, high = c->count, at; int result;
    if (!p) return XUI_OK;
    if (!(c->visits++ & 255) && (result = doc_seq_share_cancelled(c->cancellation)) != XUI_OK) return result;
    result = doc_seq_share_compact_walk(c, p->left, base); if (result != XUI_OK) return result;
    at = base + doc_seq_size(p->left);
    while (low < high) {
        uint64_t mid = low + (high - low) / 2;
        if ((uintptr_t)c->entries[mid].blob < (uintptr_t)p->blob) low = mid + 1; else high = mid;
    }
    if (low == c->count || c->entries[low].blob != p->blob) return XUI_DOC_ERROR_SCHEMA;
    if (doc_seq_share_sparse(c->entries + low)) {
        result = doc_seq_share_value_append_range(c->allocator, &c->output, c->source, c->last, at - c->last, 1, c->cancellation);
        if (result == XUI_OK) result = doc_seq_share_value_append_range(c->allocator, &c->output, c->source, at, p->length, 0, c->cancellation);
        if (result != XUI_OK) return result;
        c->last = at + p->length;
    }
    return doc_seq_share_compact_walk(c, p->right, at + p->length);
}
/* Account combined slice coverage for every backing Blob. On success the
 * returned payload count is exact, including compact copies; on failure the
 * caller's immutable root and accounting scalar are unchanged. */
int doc_seq_compact_bytes(doc_allocator* allocator, doc_sequence** value,
    const atomic_int* cancellation, uint64_t* backing_bytes)
{
    doc_seq_share_compaction c = {0}; uint64_t count = 0, used = 0, visits = 0, i, retained = 0; int result;
    result = doc_seq_share_collect(*value, NULL, &count, &visits, cancellation);
    if (result != XUI_OK) return result;
    if (!count) {
        result = doc_seq_share_cancelled(cancellation);
        if (result == XUI_OK && backing_bytes) *backing_bytes = 0;
        return result;
    }
    if (count > SIZE_MAX / sizeof(*c.entries)) return XUI_DOC_ERROR_LIMIT;
    c.entries = doc_alloc(allocator, (size_t)count * sizeof(*c.entries)); if (!c.entries) return XUI_ERROR_OUT_OF_MEMORY;
    c.allocator = allocator; c.source = *value; c.cancellation = cancellation;
    result = doc_seq_share_collect(*value, c.entries, &used, &visits, cancellation);
    if (result == XUI_OK) result = doc_seq_share_coverage_sort(c.entries, used, &visits, cancellation);
    for (i = 0; i < used && result == XUI_OK; i++) {
        if (!(i & 255)) result = doc_seq_share_cancelled(cancellation);
        if (result != XUI_OK) break;
        if (c.count && c.entries[c.count - 1].blob == c.entries[i].blob) {
            if (c.entries[i].bytes > UINT64_MAX - c.entries[c.count - 1].bytes) result = XUI_DOC_ERROR_LIMIT;
            else c.entries[c.count - 1].bytes += c.entries[i].bytes;
        } else c.entries[c.count++] = c.entries[i];
    }
    for (i = 0; i < c.count && result == XUI_OK; i++) {
        uint64_t bytes = doc_seq_share_sparse(c.entries + i) ? c.entries[i].bytes : c.entries[i].blob->size;
        if (!(i & 255)) result = doc_seq_share_cancelled(cancellation);
        if (bytes > UINT64_MAX - retained) result = XUI_DOC_ERROR_LIMIT;
        else retained += bytes;
    }
    if (result == XUI_OK) result = doc_seq_share_compact_walk(&c, *value, 0);
    if (result == XUI_OK && c.last) result = doc_seq_share_value_append_range(allocator, &c.output, *value,
        c.last, doc_seq_size(*value) - c.last, 1, cancellation);
    if (result == XUI_OK) result = doc_seq_share_cancelled(cancellation);
    if (result == XUI_OK) {
        if (c.last) { doc_seq_release(*value); *value = c.output; c.output = NULL; }
        if (backing_bytes) *backing_bytes = retained;
    }
    doc_seq_release(c.output); doc_free(c.entries); return result;
}
int doc_seq_reuse_bytes(doc_allocator* allocator, doc_sequence* old, doc_sequence* fresh,
    const atomic_int* cancellation, doc_sequence** out)
{
    uint64_t old_size = doc_seq_size(old), new_size = doc_seq_size(fresh), prefix = 0, suffix = 0;
    uint64_t old_end, new_end, limit, count = 0, capacity = 1, i, at, literal, floor, hash = 0, power = 1;
    doc_seq_share_match* entries = NULL; uint64_t* buckets = NULL;
    doc_sequence* value = NULL; doc_seq_share_reader reader = {0}; unsigned char window[DOC_SEQ_SHARE_MATCH_BYTES];
    int result = doc_seq_share_cancelled(cancellation);
    *out = NULL;
    if (result != XUI_OK) return result;
    if (!old_size || !new_size) { *out = fresh; doc_seq_retain(fresh); return XUI_OK; }
    limit = old_size < new_size ? old_size : new_size;
    result = doc_seq_share_common(old, 0, fresh, 0, limit, 0, cancellation, &prefix);
    if (result == XUI_OK && old_size == new_size && prefix == old_size) {
        *out = old; doc_seq_retain(old); return XUI_OK;
    }
    if (result == XUI_OK) result = doc_seq_share_common(old, old_size, fresh, new_size,
        limit - prefix, 1, cancellation, &suffix);
    if (result != XUI_OK) goto done;
    old_end = old_size - suffix; new_end = new_size - suffix;
    result = doc_seq_share_value_append_range(allocator, &value, old, 0, prefix, 1, cancellation);
    if (result != XUI_OK) goto done;
    if (old_end - prefix >= DOC_SEQ_SHARE_MATCH_BYTES && new_end - prefix >= DOC_SEQ_SHARE_MATCH_BYTES) {
        count = (old_end - prefix) / DOC_SEQ_SHARE_MATCH_BYTES;
        while (capacity < count && capacity <= UINT64_MAX / 2) capacity *= 2;
        if (capacity < count || count > SIZE_MAX / sizeof(*entries) || capacity > SIZE_MAX / sizeof(*buckets)) {
            result = XUI_DOC_ERROR_LIMIT; goto done;
        }
        entries = doc_alloc(allocator, (size_t)count * sizeof(*entries));
        buckets = doc_alloc(allocator, (size_t)capacity * sizeof(*buckets));
        if (!entries || !buckets) { result = XUI_ERROR_OUT_OF_MEMORY; goto done; }
        /* Earliest-first chains keep repeated bytes from selecting the tail. */
        for (i = count; i; ) {
            uint64_t slot;
            --i;
            if (!(i & 255) && (result = doc_seq_share_cancelled(cancellation)) != XUI_OK) goto done;
            if (doc_seq_read(old, prefix + i * DOC_SEQ_SHARE_MATCH_BYTES, window, sizeof(window)) != XUI_OK) {
                result = XUI_DOC_ERROR_SCHEMA; goto done;
            }
            entries[i].hash = doc_seq_share_hash(window); slot = doc_seq_share_slot(entries[i].hash, capacity);
            entries[i].next = buckets[slot]; buckets[slot] = i + 1;
        }
        for (i = 1; i < DOC_SEQ_SHARE_MATCH_BYTES; i++) power *= 257;
    }
    reader.value = fresh; literal = at = prefix; floor = prefix;
    while (count && new_end - at >= DOC_SEQ_SHARE_MATCH_BYTES) {
        uint64_t slot, entry, visited = 0, match = DOC_NONE, backward, forward, old_at;
        if (!(at & 16383) && (result = doc_seq_share_cancelled(cancellation)) != XUI_OK) goto done;
        if (at == literal) {
            if (doc_seq_read(fresh, at, window, sizeof(window)) != XUI_OK) { result = XUI_DOC_ERROR_SCHEMA; goto done; }
            hash = doc_seq_share_hash(window);
        }
        slot = doc_seq_share_slot(hash, capacity);
        while (buckets[slot] && prefix + (buckets[slot] - 1) * DOC_SEQ_SHARE_MATCH_BYTES < floor)
            buckets[slot] = entries[buckets[slot] - 1].next;
        for (entry = buckets[slot]; entry; entry = entries[entry - 1].next) {
            uint64_t common;
            if (!(visited++ & 255) && (result = doc_seq_share_cancelled(cancellation)) != XUI_OK) goto done;
            old_at = prefix + (entry - 1) * DOC_SEQ_SHARE_MATCH_BYTES;
            if (old_at < floor || entries[entry - 1].hash != hash) continue;
            result = doc_seq_share_common(old, old_at, fresh, at, DOC_SEQ_SHARE_MATCH_BYTES, 0, cancellation, &common);
            if (result != XUI_OK) goto done;
            if (common == DOC_SEQ_SHARE_MATCH_BYTES) { match = old_at; break; }
        }
        if (match != DOC_NONE) {
            limit = match - floor < at - literal ? match - floor : at - literal;
            result = doc_seq_share_common(old, match, fresh, at, limit, 1, cancellation, &backward);
            if (result != XUI_OK) goto done;
            limit = old_end - match < new_end - at ? old_end - match : new_end - at;
            result = doc_seq_share_common(old, match, fresh, at, limit, 0, cancellation, &forward);
            if (result == XUI_OK) result = doc_seq_share_value_append_range(allocator, &value, fresh,
                literal, at - backward - literal, 0, cancellation);
            if (result == XUI_OK) result = doc_seq_share_value_append_range(allocator, &value, old,
                match - backward, backward + forward, 1, cancellation);
            if (result != XUI_OK) goto done;
            floor = match + forward; at += forward; literal = at;
        } else {
            unsigned char removed, added;
            if (new_end - at == DOC_SEQ_SHARE_MATCH_BYTES) { at++; break; }
            result = doc_seq_share_byte(&reader, at, &removed);
            if (result == XUI_OK) result = doc_seq_share_byte(&reader, at + DOC_SEQ_SHARE_MATCH_BYTES, &added);
            if (result != XUI_OK) goto done;
            hash = (hash - ((uint64_t)removed + 1) * power) * 257 + added + 1; at++;
        }
    }
    result = doc_seq_share_value_append_range(allocator, &value, fresh, literal, new_end - literal, 0, cancellation);
    if (result == XUI_OK) result = doc_seq_share_value_append_range(allocator, &value, old, old_end, suffix, 1, cancellation);
    if (result == XUI_OK) result = doc_seq_compact_bytes(allocator, &value, cancellation, NULL);
    if (result == XUI_OK) result = doc_seq_share_cancelled(cancellation);
    if (result == XUI_OK) { *out = value; value = NULL; }
done:
    doc_seq_release(value); doc_free(entries); doc_free(buckets); return result;
}

#endif
