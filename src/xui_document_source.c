#include "../xui_config.h"
#if XUI_ENABLE_DOCUMENT
#include "xui_document_internal.h"
#include "xui_document_md4c.h"

static int doc_cst_cancel(const atomic_int* cancellation)
{ return cancellation ? atomic_load(cancellation) : XUI_OK; }
static int doc_cst_definition_limit(doc_state* state, uint64_t ordinal, uint64_t limit, uint64_t* bytes)
{
    xui_doc_reference_definition_t definition;
    uint64_t size = doc_seq_size(state->references);
    int result;
    if (size % sizeof(definition)) return XUI_DOC_ERROR_SCHEMA;
    if (ordinal >= size / sizeof(definition)) return XUI_ERROR_NOT_FOUND;
    result = doc_seq_read(state->references, ordinal * sizeof(definition), &definition, sizeof(definition));
    if (result != XUI_OK) return result;
    if (definition.iSourceStart > definition.iSourceEnd || definition.iSourceEnd > limit)
        return XUI_DOC_ERROR_SCHEMA;
    *bytes = definition.iSourceEnd - definition.iSourceStart; return XUI_OK;
}
static int doc_cst_definition(doc_state* state, uint64_t ordinal, uint64_t* bytes)
{ return doc_cst_definition_limit(state, ordinal, doc_seq_size(state->source), bytes); }
static uint64_t doc_cst_find(doc_sequence* forest, uint64_t key)
{
    uint64_t start = 0, end = doc_seq_size(forest);
    while (start < end) {
        uint64_t middle = start + (end - start) / 2;
        if (doc_seq_get_id(forest, middle) < key) start = middle + 1;
        else end = middle;
    }
    return start;
}
static int doc_cst_value(doc_state* state, uint64_t definition, doc_sequence** value,
    doc_source_cst_header* header)
{
    uint64_t at, bytes; int result;
    if (!state || definition == UINT64_MAX) return XUI_ERROR_INVALID_ARGUMENT;
    result = doc_cst_definition(state, definition, &bytes); if (result != XUI_OK) return result;
    at = doc_cst_find(state->reference_syntax, definition + 1);
    if (at == doc_seq_size(state->reference_syntax) || doc_seq_get_id(state->reference_syntax, at) != definition + 1)
        return XUI_ERROR_NOT_FOUND;
    *value = doc_seq_get_value_item(state->reference_syntax, at);
    if (!*value || doc_seq_size(*value) < sizeof(*header) ||
        doc_seq_read(*value, 0, header, sizeof(*header)) != XUI_OK || !header->count ||
        header->count > (UINT64_MAX - sizeof(*header)) / sizeof(doc_source_cst_record) ||
        sizeof(*header) + header->count * sizeof(doc_source_cst_record) != doc_seq_size(*value) ||
        header->source_bytes != bytes) return XUI_DOC_ERROR_SCHEMA;
    return XUI_OK;
}
int doc_source_cst_info(doc_state* state, uint64_t definition, doc_source_cst_header* out)
{
    doc_sequence* value; doc_source_cst_header header; int result;
    if (!out) return XUI_ERROR_INVALID_ARGUMENT;
    memset(out, 0, sizeof(*out)); result = doc_cst_value(state, definition, &value, &header);
    if (result == XUI_OK) *out = header;
    return result;
}
int doc_source_cst_read(doc_state* state, uint64_t definition, uint64_t ordinal, doc_source_cst_record* out)
{
    doc_source_cst_header header; doc_sequence* value; int result;
    if (!out) return XUI_ERROR_INVALID_ARGUMENT;
    memset(out, 0, sizeof(*out)); result = doc_cst_value(state, definition, &value, &header);
    if (result != XUI_OK) return result;
    if (ordinal >= header.count) return XUI_ERROR_NOT_FOUND;
    return doc_seq_read(value, sizeof(header) + ordinal * sizeof(*out), out, sizeof(*out));
}
/* Temporary dependency windows append synthetic reference definitions. Their
 * parser-confirmed coordinates belong to that input, while state->source still
 * retains the real document. The caller supplies the actual parse extent;
 * normal storage/read APIs continue to enforce the real source extent. */
int doc_source_cst_set_parsed(doc_state* state, uint64_t definition, const doc_source_cst_record* records,
    uint64_t count, uint64_t input_bytes, const atomic_int* cancellation)
{
    doc_source_cst_header header; doc_blob* blob = NULL;
    doc_sequence *value = NULL, *item = NULL, *next = NULL;
    uint64_t at, end, bytes, i; int result = doc_cst_cancel(cancellation);
    if (result != XUI_OK) return result;
    if (!state || !records || !count || definition == UINT64_MAX) return XUI_ERROR_INVALID_ARGUMENT;
    if (count > state->allocator->max_syntax_records || count > (UINT64_MAX - sizeof(header)) / sizeof(*records))
        return XUI_DOC_ERROR_LIMIT;
    result = doc_cst_definition_limit(state, definition, input_bytes, &header.source_bytes); if (result != XUI_OK) return result;
    header.count = count; bytes = sizeof(header) + count * sizeof(*records);
    if (bytes > SIZE_MAX) return XUI_DOC_ERROR_LIMIT;
    for (i = 0; i < count; i++) {
        const doc_source_cst_record* record = records + i;
        if (!(i & 255u) && (result = doc_cst_cancel(cancellation)) != XUI_OK) return result;
        if (record->kind < DOC_CST_SCOPE || record->kind > DOC_CST_OPAQUE || record->reserved ||
            record->start > record->end || record->end > header.source_bytes) return XUI_DOC_ERROR_SCHEMA;
        if (!i) {
            if (record->kind != DOC_CST_SCOPE || record->parent != DOC_NONE || record->start ||
                record->end != header.source_bytes) return XUI_DOC_ERROR_SCHEMA;
        } else {
            if (record->kind == DOC_CST_SCOPE || record->parent >= i ||
                records[record->parent].start > record->start || records[record->parent].end < record->end)
                return XUI_DOC_ERROR_SCHEMA;
        }
    }
    /* The caller has collected authoritative syntax for this definition.
     * Exact record equality can retain the arena, including on a source
     * shift; dedented text/value equality alone is never used here. */
    {
        doc_source_cst_header previous_header; doc_sequence* previous;
        if (doc_cst_value(state, definition, &previous, &previous_header) == XUI_OK &&
            previous_header.count == count) {
            doc_source_cst_record buffer[64]; int equal = 1;
            for (i = 0; i < count && equal; ) {
                uint64_t n = count - i > 64 ? 64 : count - i;
                result = doc_cst_cancel(cancellation); if (result != XUI_OK) return result;
                result = doc_seq_read(previous, sizeof(header) + i * sizeof(*records), buffer, n * sizeof(*records));
                if (result != XUI_OK) return result;
                equal = !memcmp(buffer, records + i, (size_t)(n * sizeof(*records))); i += n;
            }
            if (equal) return doc_cst_cancel(cancellation);
        }
    }
    blob = doc_blob_allocate(state->allocator, bytes); if (!blob) return XUI_ERROR_OUT_OF_MEMORY;
    memcpy(blob->data, &header, sizeof(header));
    for (i = 0; i < count; ) {
        uint64_t n = count - i > 256 ? 256 : count - i;
        result = doc_cst_cancel(cancellation); if (result != XUI_OK) goto done;
        memcpy(blob->data + sizeof(header) + i * sizeof(*records), records + i, (size_t)(n * sizeof(*records))); i += n;
    }
    value = doc_seq_blob_range(state->allocator, blob, 0, bytes);
    if (!value) { result = XUI_ERROR_OUT_OF_MEMORY; goto done; }
    item = doc_seq_value_item(state->allocator, definition + 1, value);
    if (!item) { result = XUI_ERROR_OUT_OF_MEMORY; goto done; }
    at = doc_cst_find(state->reference_syntax, definition + 1); end = at;
    if (at < doc_seq_size(state->reference_syntax) && doc_seq_get_id(state->reference_syntax, at) == definition + 1) end++;
    result = doc_seq_replace(state->allocator, state->reference_syntax, at, end, item, &next);
    if (result == XUI_OK) result = doc_cst_cancel(cancellation);
    if (result == XUI_OK) { doc_seq_release(state->reference_syntax); state->reference_syntax = next; next = NULL; }
done:
    doc_seq_release(value); doc_seq_release(item); doc_seq_release(next); doc_blob_release(blob); return result;
}
int doc_source_cst_set(doc_state* state, uint64_t definition, const doc_source_cst_record* records,
    uint64_t count, const atomic_int* cancellation)
{
    if (!state) return XUI_ERROR_INVALID_ARGUMENT;
    return doc_source_cst_set_parsed(state, definition, records, count, doc_seq_size(state->source), cancellation);
}

/* Transfer an already validated authoritative grammar arena after the caller
 * reconciles definition ordinals. Relative raw ranges require no rebasing. */
int doc_source_cst_replace(doc_state* state, uint64_t index, doc_state* parsed,
    uint64_t parsed_index, const atomic_int* cancellation)
{
    doc_source_cst_header header;doc_sequence *source=NULL,*value=NULL,*item=NULL,*next=NULL,*previous;
    uint64_t bytes,parsed_bytes,at,end;int result=doc_cst_cancel(cancellation),missing;
    if(result!=XUI_OK)return result;
    if(!state || !parsed || index==UINT64_MAX || parsed_index==UINT64_MAX || state->allocator!=parsed->allocator)
        return XUI_ERROR_INVALID_ARGUMENT;
    result=doc_cst_definition(state,index,&bytes);if(result!=XUI_OK)return result;
    result=doc_cst_definition(parsed,parsed_index,&parsed_bytes);if(result!=XUI_OK)return result;
    if(bytes!=parsed_bytes)return XUI_DOC_ERROR_SCHEMA;
    result=doc_cst_value(parsed,parsed_index,&source,&header);missing=result==XUI_ERROR_NOT_FOUND;
    if(result!=XUI_OK && !missing)return result;
    at=doc_cst_find(state->reference_syntax,index+1);end=at;
    if(at<doc_seq_size(state->reference_syntax) && doc_seq_get_id(state->reference_syntax,at)==index+1)end++;
    if(missing && at==end)return XUI_OK;
    previous=at<end?doc_seq_get_value_item(state->reference_syntax,at):NULL;
    if(!missing){
        if(previous)result=doc_seq_reuse_bytes(state->allocator,previous,source,cancellation,&value);
        else{value=source;doc_seq_retain(value);result=XUI_OK;}
        if(result!=XUI_OK)goto done;
        if(value==previous){result=doc_cst_cancel(cancellation);goto done;}
        item=doc_seq_value_item(state->allocator,index+1,value);if(!item){result=XUI_ERROR_OUT_OF_MEMORY;goto done;}
    }
    result=doc_seq_replace(state->allocator,state->reference_syntax,at,end,item,&next);
    if(result==XUI_OK)result=doc_cst_cancel(cancellation);
    if(result==XUI_OK){doc_seq_release(state->reference_syntax);state->reference_syntax=next;next=NULL;}
done:
    doc_seq_release(value);doc_seq_release(item);doc_seq_release(next);return result;
}
/* Reuse parser-authoritative relative syntax bytes independently of semantic
 * definition identities. Equal text values alone never prove equal grammar. */
typedef struct doc_cst_share_entry { doc_sequence* value; uint64_t hash; } doc_cst_share_entry;
typedef struct doc_cst_share_index {
    doc_cst_share_entry* entries;
    uint64_t* order;
    uint64_t count;
    const atomic_int* cancellation;
} doc_cst_share_index;
static int doc_cst_share_key(doc_sequence* value, const atomic_int* cancellation, doc_cst_share_entry* entry)
{
    doc_source_cst_header header; unsigned char bytes[2048]; uint64_t at, size = doc_seq_size(value);
    int result;
    if (!value || size < sizeof(header) || doc_seq_read(value, 0, &header, sizeof(header)) != XUI_OK ||
        !header.count || header.count > (UINT64_MAX - sizeof(header)) / sizeof(doc_source_cst_record) ||
        size != sizeof(header) + header.count * sizeof(doc_source_cst_record)) return XUI_DOC_ERROR_SCHEMA;
    entry->value = value; entry->hash = UINT64_C(14695981039346656037);
    for (at = 0; at < size;) {
        uint64_t n = size - at > sizeof(bytes) ? sizeof(bytes) : size - at, i;
        result = doc_cst_cancel(cancellation); if (result != XUI_OK) return result;
        result = doc_seq_read(value, at, bytes, n); if (result != XUI_OK) return result;
        for (i = 0; i < n; i++) entry->hash = (entry->hash ^ bytes[i]) * UINT64_C(1099511628211);
        at += n;
    }
#ifdef XUI_DOC_REFERENCE_HASH_COLLISION_TEST
    entry->hash = 0;
#endif
    return doc_cst_cancel(cancellation);
}
static int doc_cst_share_compare(const doc_cst_share_entry* a, const doc_cst_share_entry* b,
    const atomic_int* cancellation, int* order)
{
    uint64_t size = doc_seq_size(a->value), other = doc_seq_size(b->value), at;
    unsigned char left[2048], right[2048]; int result = doc_cst_cancel(cancellation);
    *order = 0; if (result != XUI_OK) return result;
    if (a->hash != b->hash) { *order = a->hash < b->hash ? -1 : 1; return XUI_OK; }
    if (size != other) { *order = size < other ? -1 : 1; return XUI_OK; }
    if (a->value == b->value) return XUI_OK;
    for (at = 0; at < size;) {
        uint64_t n = size - at > sizeof(left) ? sizeof(left) : size - at;
        result = doc_cst_cancel(cancellation); if (result != XUI_OK) return result;
        if (doc_seq_read(a->value, at, left, n) != XUI_OK || doc_seq_read(b->value, at, right, n) != XUI_OK)
            return XUI_DOC_ERROR_SCHEMA;
        *order = memcmp(left, right, (size_t)n); if (*order) return XUI_OK;
        at += n;
    }
    return XUI_OK;
}
static int doc_cst_share_order(doc_cst_share_index* c, uint64_t a, uint64_t b, int* order)
{
    int result = doc_cst_share_compare(c->entries + a, c->entries + b, c->cancellation, order);
    if (result == XUI_OK && !*order) *order = a < b ? -1 : a != b;
    return result;
}
static int doc_cst_share_sift(doc_cst_share_index* c, uint64_t root, uint64_t count)
{
    int result, order;
    while (root < count / 2) {
        uint64_t child = root * 2 + 1, temporary;
        if (child + 1 < count) {
            result = doc_cst_share_order(c, c->order[child], c->order[child + 1], &order);
            if (result != XUI_OK) return result;
            if (order < 0) child++;
        }
        result = doc_cst_share_order(c, c->order[root], c->order[child], &order);
        if (result != XUI_OK) return result;
        if (order >= 0) break;
        temporary = c->order[root]; c->order[root] = c->order[child]; c->order[child] = temporary; root = child;
    }
    return XUI_OK;
}
static int doc_cst_share_find(doc_cst_share_index* c, const doc_cst_share_entry* key,
    uint64_t preferred, uint64_t* found)
{
    uint64_t low = 0, high = c->count, first, last; int result, order;
    *found = DOC_NONE;
    if (preferred < c->count) {
        result = doc_cst_share_compare(c->entries + preferred, key, c->cancellation, &order);
        if (result != XUI_OK) return result;
        if (!order) { *found = preferred; return XUI_OK; }
    }
    while (low < high) {
        uint64_t mid = low + (high - low) / 2;
        result = doc_cst_share_compare(c->entries + c->order[mid], key, c->cancellation, &order);
        if (result != XUI_OK) return result;
        if (order < 0) low = mid + 1; else high = mid;
    }
    if (low == c->count) return XUI_OK;
    result = doc_cst_share_compare(c->entries + c->order[low], key, c->cancellation, &order);
    if (result != XUI_OK || order) return result;
    first = low; high = c->count;
    while (low < high) {
        uint64_t mid = low + (high - low) / 2;
        result = doc_cst_share_compare(c->entries + c->order[mid], key, c->cancellation, &order);
        if (result != XUI_OK) return result;
        if (order <= 0) low = mid + 1; else high = mid;
    }
    last = low; low = first; high = last;
    while (low < high) {
        uint64_t mid = low + (high - low) / 2;
        if (c->order[mid] < preferred) low = mid + 1; else high = mid;
    }
    if (low == last) *found = c->order[last - 1];
    else if (low == first || c->order[low] - preferred <= preferred - c->order[low - 1]) *found = c->order[low];
    else *found = c->order[low - 1];
    return doc_cst_cancel(c->cancellation);
}
int doc_source_cst_reuse(doc_state* before, doc_state* after, doc_state* base, const atomic_int* cancellation)
{
    doc_cst_share_index c = {0}; doc_sequence *forest = NULL, *value = NULL, *item = NULL, *next = NULL;
    uint64_t old_count, base_count, new_count, i, j, preferred = 0; int result = doc_cst_cancel(cancellation);
    if (result != XUI_OK) return result;
    if (!before || !after || before->allocator != after->allocator || (base && base->allocator != after->allocator))
        return XUI_ERROR_INVALID_ARGUMENT;
    if (before->reference_syntax == after->reference_syntax || !after->reference_syntax) return XUI_OK;
    if (base && base->reference_syntax == before->reference_syntax) base = NULL;
    old_count = doc_seq_size(before->reference_syntax); base_count = base ? doc_seq_size(base->reference_syntax) : 0;
    new_count = doc_seq_size(after->reference_syntax);
    if (!old_count && !base_count) return XUI_OK;
    if (base_count > UINT64_MAX - old_count) return XUI_DOC_ERROR_LIMIT;
    c.count = old_count + base_count; c.cancellation = cancellation;
    if (c.count > SIZE_MAX / sizeof(*c.entries) || c.count > SIZE_MAX / sizeof(*c.order)) return XUI_DOC_ERROR_LIMIT;
    c.entries = doc_alloc(after->allocator, (size_t)c.count * sizeof(*c.entries));
    c.order = doc_alloc(after->allocator, (size_t)c.count * sizeof(*c.order));
    if (!c.entries || !c.order) { result = XUI_ERROR_OUT_OF_MEMORY; goto done; }
    for (i = 0; i < c.count; i++) {
        doc_sequence* input = i < old_count ? before->reference_syntax : base->reference_syntax;
        uint64_t ordinal = i < old_count ? i : i - old_count;
        result = doc_cst_share_key(doc_seq_get_value_item(input, ordinal), cancellation, c.entries + i);
        if (result != XUI_OK) goto done;
        c.order[i] = i;
    }
    for (i = c.count / 2; i; ) { result = doc_cst_share_sift(&c, --i, c.count); if (result != XUI_OK) goto done; }
    for (i = c.count; i > 1; ) {
        uint64_t temporary = c.order[0]; c.order[0] = c.order[--i]; c.order[i] = temporary;
        result = doc_cst_share_sift(&c, 0, i); if (result != XUI_OK) goto done;
    }
    forest = before->reference_syntax; doc_seq_retain(forest);
    for (i = 0; i < new_count; i++) {
        doc_cst_share_entry key; doc_source_cst_header header; doc_sequence* fresh;
        uint64_t id = doc_seq_get_id(after->reference_syntax, i), found, at, end;
        if (!id || (i && doc_seq_get_id(after->reference_syntax, i - 1) >= id)) { result = XUI_DOC_ERROR_SCHEMA; goto done; }
        result = doc_cst_value(after, id - 1, &fresh, &header);
        if (result == XUI_OK) result = doc_cst_share_key(fresh, cancellation, &key);
        if (result == XUI_OK) result = doc_cst_share_find(&c, &key, preferred, &found);
        if (result != XUI_OK) goto done;
        at = doc_cst_find(forest, id);
        if (at < i) { result = XUI_DOC_ERROR_SCHEMA; goto done; }
        if (at != i) {
            result = doc_seq_replace(after->allocator, forest, i, at, NULL, &next);
            if (result != XUI_OK) goto done;
            doc_seq_release(forest); forest = next; next = NULL;
        }
        end = i < doc_seq_size(forest) && doc_seq_get_id(forest, i) == id ? i + 1 : i;
        if (found != DOC_NONE) { value = c.entries[found].value; doc_seq_retain(value); preferred = found + 1; }
        else {
            doc_sequence* previous = end > i ? doc_seq_get_value_item(forest, i) :
                c.entries[preferred < c.count ? preferred : c.count - 1].value;
            result = doc_seq_reuse_bytes(after->allocator, previous, fresh, cancellation, &value);
            if (result != XUI_OK) goto done;
        }
        if (end == i || value != doc_seq_get_value_item(forest, i)) {
            item = doc_seq_value_item(after->allocator, id, value);
            if (!item) { result = XUI_ERROR_OUT_OF_MEMORY; goto done; }
            result = doc_seq_replace(after->allocator, forest, i, end, item, &next);
            if (result != XUI_OK) goto done;
            doc_seq_release(forest); forest = next; next = NULL;
        }
        doc_seq_release(value); value = NULL; doc_seq_release(item); item = NULL;
    }
    j = doc_seq_size(forest);
    if (j > new_count) {
        result = doc_seq_replace(after->allocator, forest, new_count, j, NULL, &next);
        if (result != XUI_OK) goto done;
        doc_seq_release(forest); forest = next; next = NULL;
    }
    result = doc_cst_cancel(cancellation);
    if (result == XUI_OK) { doc_seq_release(after->reference_syntax); after->reference_syntax = forest; forest = NULL; }
done:
    doc_seq_release(forest); doc_seq_release(value); doc_seq_release(item); doc_seq_release(next);
    doc_free(c.entries); doc_free(c.order); return result;
}
#include "xui_document_cst_forest.h"
int doc_source_cursor_read(const doc_source_cursor* cursor,uint64_t ordinal,doc_source_cst_record* out)
{
    if(!cursor || !cursor->value || !out)return XUI_ERROR_INVALID_ARGUMENT;
    if(ordinal>=cursor->header.count)return XUI_ERROR_NOT_FOUND;
    return doc_seq_read(cursor->value,sizeof(cursor->header)+ordinal*sizeof(*out),out,sizeof(*out));
}
static int doc_source_cursor_match(doc_source_cursor* cursor,uint32_t kind,uint64_t start,
    uint64_t end,const atomic_int* cancellation)
{
    uint64_t i;int result;
    if(start<cursor->base || end<start || end-cursor->base>cursor->header.source_bytes)return XUI_ERROR_NOT_FOUND;
    for(i=1;i<cursor->header.count;i++){
        doc_source_cst_record r;
        if(!(i&255u) && (result=doc_cst_cancel(cancellation))!=XUI_OK)return result;
        result=doc_source_cursor_read(cursor,i,&r);if(result!=XUI_OK)return result;
        if(r.kind==DOC_CST_BLOCK && r.detail==kind && r.start==start-cursor->base && r.end==end-cursor->base){
            cursor->root=i;return XUI_OK;
        }
    }
    result=doc_cst_cancel(cancellation);return result==XUI_OK?XUI_ERROR_NOT_FOUND:result;
}
int doc_source_cursor_block(doc_state* state,uint32_t kind,uint64_t start,uint64_t end,
    const atomic_int* cancellation,doc_source_cursor* out)
{
    doc_source_cursor cursor={0};uint64_t lo=0,hi,i,count;int result;
    if(!state || !out || start>end || end>doc_seq_size(state->source))return XUI_ERROR_INVALID_ARGUMENT;
    memset(out,0,sizeof(*out));result=doc_cst_cancel(cancellation);if(result!=XUI_OK)return result;
    hi=doc_seq_size(state->source_syntax);
    while(lo<hi){
        uint64_t middle=lo+(hi-lo)/2;
        result=doc_forest_value(state->source_syntax,middle,doc_seq_size(state->source),&cursor.base,&cursor.header,&cursor.value);
        if(result!=XUI_OK)return result;
        if(cursor.base<=start)lo=middle+1;else hi=middle;
    }
    if(lo){
        result=doc_forest_value(state->source_syntax,lo-1,doc_seq_size(state->source),&cursor.base,&cursor.header,&cursor.value);
        if(result!=XUI_OK)return result;
        result=doc_source_cursor_match(&cursor,kind,start,end,cancellation);
        if(result==XUI_OK){*out=cursor;return result;}
        if(result!=XUI_ERROR_NOT_FOUND)return result;
    }
    if(doc_seq_size(state->references)%sizeof(xui_doc_reference_definition_t))return XUI_DOC_ERROR_SCHEMA;
    count=doc_seq_size(state->references)/sizeof(xui_doc_reference_definition_t);
    for(i=0;i<count;i++){
        xui_doc_reference_definition_t definition;
        if(!(i&255u) && (result=doc_cst_cancel(cancellation))!=XUI_OK)return result;
        result=doc_seq_read(state->references,i*sizeof(definition),&definition,sizeof(definition));if(result!=XUI_OK)return result;
        if(definition.iSourceStart>start || definition.iSourceEnd<end)continue;
        cursor.base=definition.iSourceStart;
        result=doc_cst_value(state,i,&cursor.value,&cursor.header);
        if(result==XUI_ERROR_NOT_FOUND)continue;
        if(result!=XUI_OK)return result;
        result=doc_source_cursor_match(&cursor,kind,start,end,cancellation);
        if(result==XUI_OK){*out=cursor;return result;}
        if(result!=XUI_ERROR_NOT_FOUND)return result;
    }
    result=doc_cst_cancel(cancellation);return result==XUI_OK?XUI_ERROR_NOT_FOUND:result;
}
int doc_source_cst_has_lookups(doc_state* state,uint64_t index,const atomic_int* cancellation,int* out)
{
    doc_source_cst_header header;doc_sequence* value;doc_source_cst_record record;uint64_t i;int result;
    if(!out)return XUI_ERROR_INVALID_ARGUMENT;
    *out=0;
    result=doc_cst_value(state,index,&value,&header);if(result==XUI_ERROR_NOT_FOUND)return XUI_OK;
    if(result!=XUI_OK)return result;
    for(i=0;i<header.count;i++){
        if(!(i&255u) && (result=doc_cst_cancel(cancellation))!=XUI_OK)return result;
        result=doc_seq_read(value,sizeof(header)+i*sizeof(record),&record,sizeof(record));if(result!=XUI_OK)return result;
        if(record.kind==DOC_CST_INLINE && (record.flags&(DOC_CST_INLINE_CANDIDATE|DOC_CST_INLINE_REFERENCE))){*out=1;break;}
    }
    return XUI_OK;
}
XUI_API int xuiDocumentSnapshotGetSourceInfo(xui_document_snapshot snapshot, xui_doc_source_info_t* out)
{
    if (!snapshot || !out || out->iSize != sizeof(*out)) return XUI_ERROR_INVALID_ARGUMENT;
    memset(out, 0, sizeof(*out)); out->iSize = sizeof(*out);
    out->iInlineSyntaxCount = doc_seq_size(snapshot->state->inline_syntax) / sizeof(xui_doc_inline_syntax_t);
    out->iReferenceDefinitionCount = doc_seq_size(snapshot->state->references) / sizeof(xui_doc_reference_definition_t);
    out->iReferenceCandidateCount = doc_seq_size(snapshot->state->reference_candidates) / sizeof(xui_doc_inline_syntax_t);
    return XUI_OK;
}
XUI_API int xuiDocumentSnapshotGetInlineSyntax(xui_document_snapshot snapshot, uint64_t index, xui_doc_inline_syntax_t* out)
{
    if (!snapshot || !out || out->iSize != sizeof(*out)) return XUI_ERROR_INVALID_ARGUMENT;
    memset(out, 0, sizeof(*out)); out->iSize = sizeof(*out);
    if (index >= doc_seq_size(snapshot->state->inline_syntax) / sizeof(*out)) return XUI_ERROR_NOT_FOUND;
    return doc_seq_read_syntax(snapshot->state->inline_syntax, index, out);
}
XUI_API int xuiDocumentSnapshotGetReferenceDefinition(xui_document_snapshot snapshot, uint64_t index, xui_doc_reference_definition_t* out)
{
    if (!snapshot || !out || out->iSize != sizeof(*out)) return XUI_ERROR_INVALID_ARGUMENT;
    memset(out, 0, sizeof(*out)); out->iSize = sizeof(*out);
    if (index >= doc_seq_size(snapshot->state->references) / sizeof(*out)) return XUI_ERROR_NOT_FOUND;
    return doc_seq_read(snapshot->state->references, index * sizeof(*out), out, sizeof(*out));
}
XUI_API int xuiDocumentSnapshotGetReferenceCandidate(xui_document_snapshot snapshot, uint64_t index, xui_doc_inline_syntax_t* out)
{
    if (!snapshot || !out || out->iSize != sizeof(*out)) return XUI_ERROR_INVALID_ARGUMENT;
    memset(out, 0, sizeof(*out)); out->iSize = sizeof(*out);
    if (index >= doc_seq_size(snapshot->state->reference_candidates) / sizeof(*out)) return XUI_ERROR_NOT_FOUND;
    return doc_seq_read_syntax(snapshot->state->reference_candidates, index, out);
}

int doc_source_append(doc_allocator* a, doc_sequence** target, const xui_doc_source_segment_t* segment)
{
    doc_sequence *item, *next = NULL; uint64_t size = doc_seq_size(*target); int result;
    item = doc_seq_text(a, (const char*)segment, sizeof(*segment));
    if (!item) return XUI_ERROR_OUT_OF_MEMORY;
    result = doc_seq_replace(a, *target, size, size, item, &next); doc_seq_release(item);
    if (result == XUI_OK) { doc_seq_release(*target); *target = next; }
    return result;
}

/* Rebase the packed source spans only when they are read. Unchanged suffix
 * nodes can retain the exact same persistent provenance sequence across edits. */
void doc_source_resolve_segment(const doc_state* state, const doc_node* node,
    xui_doc_source_segment_t* segment)
{
    int64_t shift = doc_node_block_shift(state, node);
    if (node->provenance_base != DOC_NONE)
        shift += (int64_t)node->provenance_current - (int64_t)node->provenance_base;
    if (segment->iSourceStart != DOC_NONE) segment->iSourceStart = (uint64_t)((int64_t)segment->iSourceStart + shift);
    if (segment->iSourceEnd != DOC_NONE) segment->iSourceEnd = (uint64_t)((int64_t)segment->iSourceEnd + shift);
}

int doc_source_shift_provenance(doc_node* node, uint64_t removed, uint64_t added)
{
    uint64_t i, base = DOC_NONE;
    if (!node->provenance || removed == added) return XUI_OK;
    if (node->provenance_base == DOC_NONE) {
        for (i = 0; i < doc_seq_size(node->provenance); i += sizeof(xui_doc_source_segment_t)) {
            xui_doc_source_segment_t segment;
            int result = doc_seq_read(node->provenance, i, &segment, sizeof(segment));
            if (result != XUI_OK) return result;
            if (segment.iSourceStart != DOC_NONE && segment.iSourceStart < base) base = segment.iSourceStart;
            if (segment.iSourceEnd != DOC_NONE && segment.iSourceEnd < base) base = segment.iSourceEnd;
        }
        if (base == DOC_NONE) return XUI_OK;
        node->provenance_base = node->provenance_current = base;
    }
    if (node->provenance_current < removed || node->provenance_current - removed > UINT64_MAX - added)
        return XUI_ERROR_UNSUPPORTED;
    node->provenance_current = node->provenance_current - removed + added;
    return XUI_OK;
}

int doc_source_slice(doc_state* state, const doc_node* node, uint64_t start, uint64_t end,
    uint64_t target_offset, doc_sequence** out)
{
    uint64_t i;
    for (i = 0; i < doc_seq_size(node->provenance); i += sizeof(xui_doc_source_segment_t)) {
        xui_doc_source_segment_t s; uint64_t from, to; int result;
        doc_seq_read(node->provenance, i, &s, sizeof(s));
        doc_source_resolve_segment(state, node, &s);
        if (s.iTextEnd <= start || s.iTextStart >= end) continue;
        from = s.iTextStart > start ? s.iTextStart : start; to = s.iTextEnd < end ? s.iTextEnd : end;
        if (s.iKind == XUI_DOC_SOURCE_DIRECT) {
            s.iSourceEnd = s.iSourceStart + to - s.iTextStart; s.iSourceStart += from - s.iTextStart;
        } else {
            if (from != s.iTextStart) s.iFlags |= XUI_DOC_SOURCE_PARTIAL_START;
            if (to != s.iTextEnd) s.iFlags |= XUI_DOC_SOURCE_PARTIAL_END;
        }
        s.iTextStart = target_offset + from - start; s.iTextEnd = target_offset + to - start;
        result = doc_source_append(state->allocator, out, &s); if (result != XUI_OK) return result;
    }
    return XUI_OK;
}

XUI_API int xuiDocumentSnapshotGetSourceSegment(xui_document_snapshot snapshot, uint64_t id,
    uint64_t index, xui_doc_source_segment_t* out)
{
    doc_node* node;
    if (!snapshot || !out || out->iSize != sizeof(*out)) return XUI_ERROR_INVALID_ARGUMENT;
    memset(out, 0, sizeof(*out)); out->iSize = sizeof(*out);
    node = doc_index_get(snapshot->state->index, id);
    if (!node || index >= doc_seq_size(node->provenance) / sizeof(*out)) return XUI_ERROR_NOT_FOUND;
    { int result = doc_seq_read(node->provenance, index * sizeof(*out), out, sizeof(*out));
      if (result == XUI_OK) doc_source_resolve_segment(snapshot->state, node, out);
      return result; }
}

/* A source line is a raw byte partition, independent of Markdown semantics.
 * Read bounded chunks so a long code/HTML line never needs a flat copy. */
XUI_API int xuiDocumentSnapshotGetSourceLine(xui_document_snapshot snapshot,
    uint64_t offset, xui_doc_source_line_t* out)
{
    doc_sequence* source;
    uint64_t size, at, start, indent_end, trailing_start;
    char buffer[4096], current;
    int body = 0, result;
    if (!snapshot || !out || out->iSize != sizeof(*out)) return XUI_ERROR_INVALID_ARGUMENT;
    memset(out, 0, sizeof(*out)); out->iSize = sizeof(*out);
    if (snapshot->state->profile != XUI_DOCUMENT_MARKDOWN) return XUI_ERROR_UNSUPPORTED;
    source = snapshot->state->source; size = doc_seq_size(source);
    if (offset > size) return XUI_ERROR_INVALID_ARGUMENT;
    if (offset == size) return XUI_ERROR_NOT_FOUND;
    at = offset;
    result = doc_seq_read(source, offset, &current, 1);
    if (result != XUI_OK) return result;
    if (current == '\n' && offset) {
        result = doc_seq_read(source, offset - 1, &current, 1);
        if (result != XUI_OK) return result;
        if (current == '\r') at--;
    }
    while (at) {
        uint64_t count = at < sizeof(buffer) ? at : sizeof(buffer), i;
        start = at - count;
        result = doc_seq_read(source, start, buffer, count);
        if (result != XUI_OK) return result;
        for (i = count; i; i--) if (buffer[i - 1] == '\r' || buffer[i - 1] == '\n') {
            at = start + i;
            goto found_start;
        }
        at = start;
    }
found_start:
    start = at; indent_end = trailing_start = start;
    out->iLineStart = start;
    while (at < size) {
        uint64_t count = size - at < sizeof(buffer) ? size - at : sizeof(buffer), i;
        result = doc_seq_read(source, at, buffer, count);
        if (result != XUI_OK) return result;
        for (i = 0; i < count; i++) {
            uint64_t position = at + i;
            current = buffer[i];
            if (current == '\r' || current == '\n') {
                out->iContentEnd = position;
                out->iLineEnd = position + 1;
                if (current == '\r' && out->iLineEnd < size) {
                    char next;
                    result = doc_seq_read(source, out->iLineEnd, &next, 1);
                    if (result != XUI_OK) return result;
                    if (next == '\n') out->iLineEnd++;
                }
                goto found_end;
            }
            if (current == ' ' || current == '\t') {
                if (!body) indent_end = position + 1;
            } else {
                body = 1; trailing_start = position + 1;
            }
        }
        at += count;
    }
    out->iContentEnd = out->iLineEnd = size;
found_end:
    out->iIndentEnd = indent_end;
    out->iTrailingStart = body ? trailing_start : indent_end;
    return XUI_OK;
}

XUI_API int xuiDocumentSnapshotGetBlockSyntax(xui_document_snapshot snapshot,
    xui_doc_node_id id, xui_doc_block_syntax_t* out)
{
    doc_node* node;
    doc_node_source_range range;
    uint64_t span;
    if (!snapshot || !out || out->iSize != sizeof(*out)) return XUI_ERROR_INVALID_ARGUMENT;
    memset(out, 0, sizeof(*out)); out->iSize = sizeof(*out);
    if (snapshot->state->profile != XUI_DOCUMENT_MARKDOWN) return XUI_ERROR_UNSUPPORTED;
    node = doc_index_get(snapshot->state->index, id);
    if (!node || !node->marker_kind || node->kind == XUI_DOC_SOFT_BREAK ||
        node->kind == XUI_DOC_HARD_BREAK) return XUI_ERROR_NOT_FOUND;
    doc_node_source_range_get(snapshot->state, node, &range);
    if (range.syntax_start == DOC_NONE || range.syntax_end == DOC_NONE ||
        range.syntax_end < range.syntax_start || range.syntax_end > doc_seq_size(snapshot->state->source))
        return XUI_DOC_ERROR_FORMAT;
    span = range.syntax_end - range.syntax_start;
    if (node->marker_primary_start > node->marker_primary_end || node->marker_primary_end > span ||
        node->marker_primary_start == node->marker_primary_end) return XUI_DOC_ERROR_FORMAT;
    out->iKind = node->marker_kind;
    out->iPrimaryStart = range.syntax_start + node->marker_primary_start;
    out->iPrimaryEnd = range.syntax_start + node->marker_primary_end;
    out->iSecondaryStart = out->iSecondaryEnd = DOC_NONE;
    out->iFenceTailStart = out->iFenceTailEnd = DOC_NONE;
    out->iFenceInfoStart = out->iFenceInfoEnd = DOC_NONE;
    out->iFenceLanguageStart = out->iFenceLanguageEnd = DOC_NONE;
    out->iListMarkerGapStart = out->iListMarkerGapEnd = DOC_NONE;
    out->iTaskMarkerGapStart = out->iTaskMarkerGapEnd = DOC_NONE;
    out->iHeadingContentStart = out->iHeadingContentEnd = DOC_NONE;
    if (node->marker_secondary_start || node->marker_secondary_end) {
        if (node->marker_secondary_start >= node->marker_secondary_end ||
            node->marker_secondary_end > span) return XUI_DOC_ERROR_FORMAT;
        out->iSecondaryStart = range.syntax_start + node->marker_secondary_start;
        out->iSecondaryEnd = range.syntax_start + node->marker_secondary_end;
    }
    if (node->marker_kind == XUI_DOC_BLOCK_SYNTAX_ATX_HEADING ||
        node->marker_kind == XUI_DOC_BLOCK_SYNTAX_SETEXT_HEADING) {
        doc_heading_content_relative content;
        if (node->kind != XUI_DOC_HEADING || !node->syntax_aux ||
            node->syntax_aux->size != sizeof(content)) return XUI_DOC_ERROR_FORMAT;
        memcpy(&content, node->syntax_aux->data, sizeof(content));
        if (content.start > content.end || content.end > span ||
            (node->marker_kind == XUI_DOC_BLOCK_SYNTAX_ATX_HEADING && content.start < node->marker_primary_end) ||
            (node->marker_kind == XUI_DOC_BLOCK_SYNTAX_SETEXT_HEADING && content.end > node->marker_primary_start))
            return XUI_DOC_ERROR_FORMAT;
        out->iHeadingContentStart = range.syntax_start + content.start;
        out->iHeadingContentEnd = range.syntax_start + content.end;
    }
    if (node->marker_kind == XUI_DOC_BLOCK_SYNTAX_FENCED_CODE) {
        doc_fence_info_relative info;
        if (node->marker_tail_end < node->marker_primary_end ||
            node->marker_tail_end > span || !node->syntax_aux ||
            node->syntax_aux->size != sizeof(info))
            return XUI_DOC_ERROR_FORMAT;
        out->iFenceTailStart = out->iPrimaryEnd;
        out->iFenceTailEnd = range.syntax_start + node->marker_tail_end;
        memcpy(&info, node->syntax_aux->data, sizeof(info));
        if (info.info_start < node->marker_primary_end ||
            info.info_start > info.language_end ||
            info.language_end > info.info_end ||
            info.info_end > node->marker_tail_end) return XUI_DOC_ERROR_FORMAT;
        out->iFenceInfoStart = range.syntax_start + info.info_start;
        out->iFenceInfoEnd = range.syntax_start + info.info_end;
        out->iFenceLanguageStart = out->iFenceInfoStart;
        out->iFenceLanguageEnd = range.syntax_start + info.language_end;
    }
    if (node->marker_kind == XUI_DOC_BLOCK_SYNTAX_QUOTE_OPEN) {
        if (node->quote_prefixes && node->quote_prefixes->size % sizeof(uint32_t))
            return XUI_DOC_ERROR_FORMAT;
        out->iQuotePrefixCount = 1 + (node->quote_prefixes ?
            node->quote_prefixes->size / sizeof(uint32_t) : 0);
    }
    if (node->marker_kind == XUI_DOC_BLOCK_SYNTAX_TABLE_UNDERLINE) {
        if (!node->syntax_aux || node->syntax_aux->size % sizeof(doc_table_token_relative))
            return XUI_DOC_ERROR_FORMAT;
        out->iTableTokenCount = node->syntax_aux->size / sizeof(doc_table_token_relative);
    }
    if (node->marker_kind == XUI_DOC_BLOCK_SYNTAX_INDENTED_CODE) {
        if (node->kind != XUI_DOC_CODE_BLOCK || !node->syntax_aux ||
            !node->syntax_aux->size ||
            node->syntax_aux->size % sizeof(doc_code_indent_relative))
            return XUI_DOC_ERROR_FORMAT;
        out->iCodeIndentCount = node->syntax_aux->size / sizeof(doc_code_indent_relative);
    }
    if (node->marker_kind == XUI_DOC_BLOCK_SYNTAX_LIST_ITEM && node->list_indents) {
        if (node->list_indents->size % sizeof(doc_list_indent_relative))
            return XUI_DOC_ERROR_FORMAT;
        out->iListIndentCount = node->list_indents->size / sizeof(doc_list_indent_relative);
    }
    if (node->marker_kind == XUI_DOC_BLOCK_SYNTAX_LIST_ITEM) {
        out->iListMarkerGapStart = out->iPrimaryEnd;
        if (node->marker_secondary_end) {
            if (node->marker_secondary_start < node->marker_primary_end ||
                node->marker_tail_end < node->marker_secondary_end ||
                node->marker_tail_end > span) return XUI_DOC_ERROR_FORMAT;
            out->iListMarkerGapEnd = out->iSecondaryStart;
            out->iTaskMarkerGapStart = out->iSecondaryEnd;
            out->iTaskMarkerGapEnd = range.syntax_start + node->marker_tail_end;
        } else {
            if (node->marker_tail_end < node->marker_primary_end ||
                node->marker_tail_end > span) return XUI_DOC_ERROR_FORMAT;
            out->iListMarkerGapEnd = range.syntax_start + node->marker_tail_end;
        }
    }
    return XUI_OK;
}
XUI_API int xuiDocumentSnapshotGetBreakSyntax(xui_document_snapshot snapshot,
    xui_doc_node_id id, xui_doc_break_syntax_t* out)
{
    doc_node* node;
    doc_node_source_range range;
    uint64_t span;
    if (!snapshot || !out || out->iSize != sizeof(*out)) return XUI_ERROR_INVALID_ARGUMENT;
    memset(out, 0, sizeof(*out)); out->iSize = sizeof(*out);
    if (snapshot->state->profile != XUI_DOCUMENT_MARKDOWN) return XUI_ERROR_UNSUPPORTED;
    node = doc_index_get(snapshot->state->index, id);
    if (!node || (node->kind != XUI_DOC_SOFT_BREAK && node->kind != XUI_DOC_HARD_BREAK) ||
        !node->marker_kind) return XUI_ERROR_NOT_FOUND;
    doc_node_source_range_get(snapshot->state, node, &range);
    if (range.source_start == DOC_NONE || range.source_end == DOC_NONE ||
        range.source_end < range.source_start ||
        range.source_end > doc_seq_size(snapshot->state->source)) return XUI_DOC_ERROR_FORMAT;
    span = range.source_end - range.source_start;
    if (node->marker_kind < XUI_DOC_BREAK_SOFT ||
        node->marker_kind > XUI_DOC_BREAK_HARD_FORCED ||
        (node->kind == XUI_DOC_SOFT_BREAK) != (node->marker_kind == XUI_DOC_BREAK_SOFT) ||
        node->marker_primary_start > node->marker_primary_end ||
        node->marker_primary_end >= node->marker_tail_end ||
        node->marker_tail_end != span) return XUI_DOC_ERROR_FORMAT;
    if (node->marker_secondary_start || node->marker_secondary_end) {
        if (node->marker_kind != XUI_DOC_BREAK_HARD_BACKSLASH &&
            node->marker_kind != XUI_DOC_BREAK_HARD_SPACES) return XUI_DOC_ERROR_FORMAT;
        if (node->marker_secondary_start >= node->marker_secondary_end ||
            node->marker_secondary_end > node->marker_primary_end)
            return XUI_DOC_ERROR_FORMAT;
    } else if (node->marker_kind == XUI_DOC_BREAK_HARD_BACKSLASH ||
        node->marker_kind == XUI_DOC_BREAK_HARD_SPACES) return XUI_DOC_ERROR_FORMAT;
    out->iKind = node->marker_kind;
    out->iTrailingStart = range.source_start + node->marker_primary_start;
    out->iTrailingEnd = range.source_start + node->marker_primary_end;
    out->iLineEndingStart = out->iTrailingEnd;
    out->iLineEndingEnd = range.source_start + node->marker_tail_end;
    out->iHardMarkerStart = out->iHardMarkerEnd = DOC_NONE;
    if (node->marker_secondary_end) {
        out->iHardMarkerStart = range.source_start + node->marker_secondary_start;
        out->iHardMarkerEnd = range.source_start + node->marker_secondary_end;
    }
    return XUI_OK;
}
XUI_API int xuiDocumentSnapshotGetQuotePrefix(xui_document_snapshot snapshot,
    xui_doc_node_id id, uint64_t index, uint64_t* start, uint64_t* end)
{
    xui_doc_block_syntax_t syntax = {0};
    doc_node* node;
    doc_node_source_range range;
    uint32_t relative;
    int result;
    if (!start || !end) return XUI_ERROR_INVALID_ARGUMENT;
    *start = *end = DOC_NONE;
    syntax.iSize = sizeof(syntax);
    result = xuiDocumentSnapshotGetBlockSyntax(snapshot, id, &syntax);
    if (result != XUI_OK) return result;
    if (syntax.iKind != XUI_DOC_BLOCK_SYNTAX_QUOTE_OPEN ||
        index >= syntax.iQuotePrefixCount) return XUI_ERROR_NOT_FOUND;
    if (!index) {
        *start = syntax.iPrimaryStart; *end = syntax.iPrimaryEnd;
        return XUI_OK;
    }
    node = doc_index_get(snapshot->state->index, id);
    memcpy(&relative, node->quote_prefixes->data + (index - 1) * sizeof(relative),
        sizeof(relative));
    doc_node_source_range_get(snapshot->state, node, &range);
    if (relative <= node->marker_primary_start ||
        relative >= range.syntax_end - range.syntax_start)
        return XUI_DOC_ERROR_FORMAT;
    *start = range.syntax_start + relative; *end = *start + 1;
    return XUI_OK;
}
XUI_API int xuiDocumentSnapshotGetListContinuationIndent(xui_document_snapshot snapshot,
    xui_doc_node_id id, uint64_t index, xui_doc_list_indent_t* out)
{
    xui_doc_block_syntax_t syntax = {0};
    doc_node_source_range range;
    doc_node* node;
    doc_list_indent_relative relative;
    int result;
    if (!out || out->iSize != sizeof(*out)) return XUI_ERROR_INVALID_ARGUMENT;
    memset(out, 0, sizeof(*out)); out->iSize = sizeof(*out);
    syntax.iSize = sizeof(syntax);
    result = xuiDocumentSnapshotGetBlockSyntax(snapshot, id, &syntax);
    if (result != XUI_OK) return result;
    if (syntax.iKind != XUI_DOC_BLOCK_SYNTAX_LIST_ITEM ||
        index >= syntax.iListIndentCount) return XUI_ERROR_NOT_FOUND;
    node = doc_index_get(snapshot->state->index, id);
    memcpy(&relative, node->list_indents->data + index * sizeof(relative), sizeof(relative));
    doc_node_source_range_get(snapshot->state, node, &range);
    if (relative.start >= relative.end ||
        relative.start <= node->marker_primary_end ||
        relative.end > range.syntax_end - range.syntax_start ||
        relative.start_column >= relative.content_column ||
        relative.content_column > relative.end_column) return XUI_DOC_ERROR_FORMAT;
    out->iSourceStart = range.syntax_start + relative.start;
    out->iSourceEnd = range.syntax_start + relative.end;
    out->iIndentStartColumn = relative.start_column;
    out->iContentColumn = relative.content_column;
    out->iIndentEndColumn = relative.end_column;
    return XUI_OK;
}
XUI_API int xuiDocumentSnapshotGetCodeIndent(xui_document_snapshot snapshot,
    xui_doc_node_id id, uint64_t index, xui_doc_code_indent_t* out)
{
    xui_doc_block_syntax_t syntax = {0};
    doc_code_indent_relative relative;
    doc_node_source_range range;
    doc_node* node;
    int result;
    if (!out || out->iSize != sizeof(*out)) return XUI_ERROR_INVALID_ARGUMENT;
    memset(out, 0, sizeof(*out)); out->iSize = sizeof(*out);
    syntax.iSize = sizeof(syntax);
    result = xuiDocumentSnapshotGetBlockSyntax(snapshot, id, &syntax);
    if (result != XUI_OK) return result;
    if (syntax.iKind != XUI_DOC_BLOCK_SYNTAX_INDENTED_CODE ||
        index >= syntax.iCodeIndentCount) return XUI_ERROR_NOT_FOUND;
    node = doc_index_get(snapshot->state->index, id);
    memcpy(&relative, node->syntax_aux->data + index * sizeof(relative), sizeof(relative));
    doc_node_source_range_get(snapshot->state, node, &range);
    if (relative.start > relative.end ||
        relative.end > range.syntax_end - range.syntax_start ||
        relative.flags > XUI_DOC_CODE_INDENT_BLANK ||
        (relative.flags ? relative.content_column != UINT32_MAX ||
            relative.start_column != relative.end_column :
            relative.start == relative.end ||
            relative.start_column >= relative.content_column ||
            relative.content_column > relative.end_column ||
            relative.content_column - relative.start_column != 4))
        return XUI_DOC_ERROR_FORMAT;
    out->iFlags = relative.flags;
    out->iSourceStart = range.syntax_start + relative.start;
    out->iSourceEnd = range.syntax_start + relative.end;
    out->iIndentStartColumn = relative.start_column;
    out->iContentColumn = relative.content_column;
    out->iIndentEndColumn = relative.end_column;
    return XUI_OK;
}
XUI_API int xuiDocumentSnapshotGetTableToken(xui_document_snapshot snapshot,
    xui_doc_node_id id, uint64_t index, xui_doc_table_token_t* out)
{
    xui_doc_block_syntax_t syntax = {0};
    doc_table_token_relative relative;
    doc_node_source_range range;
    doc_node* node;
    int result;
    if (!out || out->iSize != sizeof(*out)) return XUI_ERROR_INVALID_ARGUMENT;
    memset(out, 0, sizeof(*out)); out->iSize = sizeof(*out);
    syntax.iSize = sizeof(syntax);
    result = xuiDocumentSnapshotGetBlockSyntax(snapshot, id, &syntax);
    if (result != XUI_OK) return result;
    if (syntax.iKind != XUI_DOC_BLOCK_SYNTAX_TABLE_UNDERLINE ||
        index >= syntax.iTableTokenCount) return XUI_ERROR_NOT_FOUND;
    node = doc_index_get(snapshot->state->index, id);
    memcpy(&relative, node->syntax_aux->data + index * sizeof(relative), sizeof(relative));
    doc_node_source_range_get(snapshot->state, node, &range);
    if (relative.start >= relative.end ||
        relative.end > range.syntax_end - range.syntax_start ||
        (relative.kind != XUI_DOC_TABLE_TOKEN_PIPE &&
         relative.kind != XUI_DOC_TABLE_TOKEN_UNDERLINE) ||
        (relative.kind == XUI_DOC_TABLE_TOKEN_UNDERLINE && relative.row != 1) ||
        relative.flags > 3) return XUI_DOC_ERROR_FORMAT;
    out->iKind = relative.kind; out->iRow = relative.row;
    out->iOrdinal = relative.ordinal; out->iFlags = relative.flags;
    out->iSourceStart = range.syntax_start + relative.start;
    out->iSourceEnd = range.syntax_start + relative.end;
    return XUI_OK;
}

static unsigned doc_source_quality(int mapping)
{
    return mapping == XUI_DOC_MAP_EXACT ? 0 : mapping == XUI_DOC_MAP_COLLAPSED ? 1 :
        mapping == XUI_DOC_MAP_SYNTAX ? 2 : 3;
}
int doc_source_better(const doc_source_hit* a, const doc_source_hit* b, unsigned affinity)
{
    if (a->distance != b->distance) return a->distance < b->distance;
    if (a->mapping != b->mapping) return doc_source_quality(a->mapping) < doc_source_quality(b->mapping);
    if (a->rank != b->rank) return a->rank < b->rank;
    if (a->span != b->span) return a->span < b->span;
    return affinity == XUI_DOC_AFTER;
}
static int doc_source_segment_map(const xui_doc_source_segment_t* s, uint64_t offset,
    unsigned affinity, int from_source, doc_source_hit* hit)
{
    uint64_t begin = from_source ? s->iSourceStart : s->iTextStart;
    uint64_t end = from_source ? s->iSourceEnd : s->iTextEnd;
    uint64_t other_begin = from_source ? s->iTextStart : s->iSourceStart;
    uint64_t other_end = from_source ? s->iTextEnd : s->iSourceEnd;
    if (s->iSourceStart == DOC_NONE || s->iSourceEnd == DOC_NONE) return 0;
    hit->distance = offset < begin ? begin - offset : offset > end ? offset - end : 0;
    hit->span = end - begin;
    hit->rank = (offset == begin && affinity == XUI_DOC_BEFORE) || (offset == end && affinity == XUI_DOC_AFTER);
    if (hit->distance) {
        hit->offset = offset < begin ? other_begin : other_end;
        hit->mapping = from_source ? XUI_DOC_MAP_SYNTAX : XUI_DOC_MAP_APPROXIMATE;
    } else if (s->iKind == XUI_DOC_SOURCE_DIRECT && end - begin == other_end - other_begin) {
        hit->offset = other_begin + offset - begin; hit->mapping = XUI_DOC_MAP_EXACT;
    } else if (begin != end && offset == begin && !(s->iFlags & XUI_DOC_SOURCE_PARTIAL_START)) {
        hit->offset = other_begin; hit->mapping = XUI_DOC_MAP_EXACT;
    } else if (begin != end && offset == end && !(s->iFlags & XUI_DOC_SOURCE_PARTIAL_END)) {
        hit->offset = other_end; hit->mapping = XUI_DOC_MAP_EXACT;
    } else {
        hit->offset = affinity == XUI_DOC_AFTER ? other_end : other_begin; hit->mapping = XUI_DOC_MAP_COLLAPSED;
    }
    return 1;
}
static int doc_source_map(const doc_state* state, const doc_node* node,
    uint64_t offset, unsigned affinity, int from_source, doc_source_hit* out)
{
    uint64_t i; int found = 0;
    for (i = 0; i < doc_seq_size(node->provenance); i += sizeof(xui_doc_source_segment_t)) {
        xui_doc_source_segment_t segment; doc_source_hit hit;
        doc_seq_read(node->provenance, i, &segment, sizeof(segment));
        doc_source_resolve_segment(state, node, &segment);
        if (doc_source_segment_map(&segment, offset, affinity, from_source, &hit) &&
            (!found || doc_source_better(&hit, out, affinity))) { *out = hit; found = 1; }
    }
    return found;
}
int doc_source_map_source(const doc_state* state, const doc_node* node,
    uint64_t offset, unsigned affinity, doc_source_hit* out)
{
    return doc_source_map(state, node, offset, affinity, 1, out);
}
int doc_source_map_text(const doc_state* state, const doc_node* node,
    uint64_t offset, unsigned affinity, uint64_t* out, int* mapping)
{
    doc_source_hit hit;
    if (!doc_source_map(state, node, offset, affinity, 0, &hit)) return XUI_ERROR_NOT_FOUND;
    *out = hit.offset; *mapping = hit.mapping; return XUI_OK;
}

/* Derived read-only source ownership. The index retains the existing snapshot;
 * it owns neither another source store nor a content/undo root. Construction
 * uses parser-confirmed annotations, then a sorted sweep partitions residual
 * bytes. Unknown and opaque payloads remain explicit rather than being given
 * an invented Markdown interpretation. */
/* Public records include decoding metadata for convenience. Store only a
 * snapshot-local provenance ordinal in each token; the immutable snapshot
 * already owns all original mapping fields. Avoid duplicating eight wide
 * values for every few-byte syntax/trivia token. */
typedef struct doc_si_record_value {
    uint32_t iKindDetail, iFlags;
    uint64_t iParentIndex, iNodeId, iOriginIndex;
    uint64_t iSourceStart, iSourceEnd, iContentStart, iContentEnd;
} doc_si_record_value;
typedef struct doc_si_token_value {
    uint64_t iSourceStart, iSourceEnd, iRecordIndex, iProvenanceIndex;
    uint32_t iKind, iDetail, iFlags, iReserved;
} doc_si_token_value;
/* Sweep priority and final subtree maximum have disjoint lifetimes. The
 * temporary annotations become the final bindings in place, using maximum
 * for priority until the token sweep finishes. No duplicate origin vector. */
#define DOC_SI_MAXIMUM_ANCHOR UINT32_C(0x80000000)
typedef struct doc_si_binding_value {
    uint64_t iSourceStart, iSourceEnd, iRecordIndex, iOriginIndex, maximum;
    uint32_t iKindDetail, iFlagsOrigin;
} doc_si_binding_value;
typedef doc_si_binding_value doc_si_annotation;
/* There are at most UINT64_MAX/2 annotations before events are created.
 * Keep the start/end bit beside the ordinal instead of a padded third field;
 * original offsets and annotation ordinals retain their full required range. */
typedef struct doc_si_event { uint64_t offset, annotation_phase; } doc_si_event;
/* A bounded source and ordinal fit without sacrificing the wide fallback. */
typedef struct doc_si_event32 { uint32_t offset, annotation_phase; } doc_si_event32;
typedef struct doc_si_interval { uint64_t start, end, record, maximum; } doc_si_interval;
struct xui_doc_source_index_t {
    atomic_uint refs;
    xui_document_snapshot snapshot;
    xui_doc_source_index_info_t info;
    doc_si_record_value* records;
    doc_si_token_value* tokens;
    doc_si_binding_value* bindings;
};
typedef struct doc_si_builder {
    xui_document_source_index index;
    doc_si_annotation* annotations;
    doc_si_interval* intervals;
    doc_si_event* events;
    uint64_t* heap;
    uint64_t record_capacity, token_capacity, binding_capacity, annotation_count, annotation_capacity;
    uint64_t interval_count, interval_capacity, event_capacity, heap_capacity, heap_count;
    uint64_t used, peak, limit, steps, reads;
    xui_doc_source_cancel_proc cancel;
    void* cancel_user;
    int error, count_tokens;
    size_t event_unit;
    uint64_t expected_records;
    doc_si_token_value previous_token;
} doc_si_builder;

static int doc_si_step(doc_si_builder* b)
{
    b->steps++;
    if (!b->error && !(b->steps & 127) && b->cancel && b->cancel(b->cancel_user))
        b->error = XUI_DOC_ERROR_CANCELLED;
    return b->error;
}
static int doc_si_reserve(doc_si_builder* b, void** pointer, uint64_t* capacity,
    uint64_t count, size_t unit)
{
    uint64_t next, old_bytes, new_bytes; void* memory;
    if (b->error || count <= *capacity) return b->error;
    if (doc_si_step(b)) return b->error;
    next = *capacity ? *capacity : 16;
    while (next < count) {
        if (next > UINT64_MAX / 2) { b->error = XUI_DOC_ERROR_LIMIT; return b->error; }
        next *= 2;
    }
    if (next > (SIZE_MAX - sizeof(doc_allocation)) / unit) {
        b->error = XUI_DOC_ERROR_LIMIT; return b->error;
    }
    new_bytes = next * unit + sizeof(doc_allocation);
    old_bytes = *capacity ? *capacity * unit + sizeof(doc_allocation) : 0;
    /* doc_realloc allocates before freeing the previous buffer. Charge that
     * actual peak, not merely the final vector sizes. */
    if (new_bytes > b->limit || b->used > b->limit - new_bytes) {
        b->error = XUI_DOC_ERROR_LIMIT; return b->error;
    }
    memory = doc_realloc(b->index->snapshot->state->allocator, *pointer, (size_t)(next * unit));
    if (!memory) { b->error = XUI_ERROR_OUT_OF_MEMORY; return b->error; }
    if (b->peak < b->used + new_bytes) b->peak = b->used + new_bytes;
    b->used = b->used - old_bytes + new_bytes; *pointer = memory; *capacity = next;
    return XUI_OK;
}
/* This phase knows its complete count. Do not round an immutable array
 * up to the next power of two or grow it through a duplicate old buffer. */
static int doc_si_exact(doc_si_builder* b, void** pointer, uint64_t* capacity,
    uint64_t count, size_t unit)
{
    uint64_t bytes;
    if (b->error || !count) return b->error;
    if (*pointer || *capacity) return b->error = XUI_DOC_ERROR_FORMAT;
    if (doc_si_step(b)) return b->error;
    if (count > (SIZE_MAX - sizeof(doc_allocation)) / unit) return b->error = XUI_DOC_ERROR_LIMIT;
    bytes = count * unit + sizeof(doc_allocation);
    if (bytes > b->limit || b->used > b->limit - bytes) return b->error = XUI_DOC_ERROR_LIMIT;
    *pointer = doc_alloc(b->index->snapshot->state->allocator, (size_t)(count * unit));
    if (!*pointer) return b->error = XUI_ERROR_OUT_OF_MEMORY;
    *capacity = count; b->used += bytes; if (b->used > b->peak) b->peak = b->used;
    return XUI_OK;
}
static int doc_si_count_records(doc_si_builder* b, const xui_doc_source_info_t* info)
{
    doc_state* state = b->index->snapshot->state;
    uint64_t count = state->node_count, i, extra[3] = {
        info->iInlineSyntaxCount, info->iReferenceDefinitionCount, info->iReferenceCandidateCount };
    for (i = 0; i < 3; i++) {
        if (extra[i] > UINT64_MAX - count) return b->error = XUI_DOC_ERROR_LIMIT;
        count += extra[i];
    }
    for (i = 0; i < info->iReferenceDefinitionCount; i++) {
        doc_source_cst_header header; int result;
        if (doc_si_step(b)) return b->error;
        result = doc_source_cst_info(state, i, &header);
        if (result == XUI_ERROR_NOT_FOUND) continue;
        if (result != XUI_OK) return b->error = result;
        if (!header.count || header.count - 1 > UINT64_MAX - count) return b->error = XUI_DOC_ERROR_LIMIT;
        count += header.count - 1;
    }
    for (i = 0; i < doc_source_forest_count(state); i++) {
        doc_source_cst_header header; uint64_t base;
        if (doc_si_step(b)) return b->error;
        b->error = doc_source_forest_info(state, i, &base, &header);
        if (b->error) return b->error;
        if (header.count > UINT64_MAX - count) return b->error = XUI_DOC_ERROR_LIMIT;
        count += header.count;
    }
    b->expected_records = count;
    return doc_si_exact(b, (void**)&b->index->records, &b->record_capacity,
        count, sizeof(*b->index->records));
}
static int doc_si_read(doc_si_builder* b, uint64_t at, void* bytes, uint64_t count)
{
    if (b->error || doc_si_step(b)) return b->error;
    b->error = doc_seq_read(b->index->snapshot->state->source, at, bytes, count);
    if (!b->error) b->reads += count;
    return b->error;
}
static int doc_si_span(doc_si_builder* b, uint64_t start, uint64_t end)
{
    return start != DOC_NONE && end != DOC_NONE && start <= end && end <= b->index->info.iSourceBytes;
}
static uint64_t doc_si_record(doc_si_builder* b, uint32_t kind, uint32_t detail,
    uint32_t flags, uint64_t parent, uint64_t node, uint64_t origin,
    uint64_t start, uint64_t end, uint64_t content_start, uint64_t content_end)
{
    doc_si_record_value* r; uint64_t ordinal = b->index->info.iRecordCount;
    if (doc_si_reserve(b, (void**)&b->index->records, &b->record_capacity,
        ordinal + 1, sizeof(*r))) return DOC_NONE;
    if (kind > UINT16_MAX || detail > UINT16_MAX) { b->error = XUI_DOC_ERROR_LIMIT; return DOC_NONE; }
    r = b->index->records + ordinal; memset(r, 0, sizeof(*r));
    r->iKindDetail = kind | (detail << 16); r->iFlags = flags;
    r->iParentIndex = parent; r->iNodeId = node; r->iOriginIndex = origin;
    r->iSourceStart = start; r->iSourceEnd = end;
    r->iContentStart = content_start; r->iContentEnd = content_end;
    b->index->info.iRecordCount++; return ordinal;
}
static int doc_si_add_origin(doc_si_builder* b, uint64_t record, uint32_t kind,
    uint32_t detail, uint32_t flags, uint64_t start, uint64_t end, unsigned priority,
    uint32_t origin_kind, uint64_t origin_index)
{
    doc_si_annotation* a;
    if (b->error || doc_si_step(b)) return b->error;
    if (start == DOC_NONE && end == DOC_NONE) return XUI_OK;
    if (record >= b->index->info.iRecordCount || !doc_si_span(b, start, end))
        return b->error = XUI_DOC_ERROR_FORMAT;
    if (kind > UINT16_MAX || detail > UINT16_MAX || flags > UINT16_MAX ||
        origin_kind > UINT16_MAX / 2) return b->error = XUI_DOC_ERROR_LIMIT;
    if (doc_si_reserve(b, (void**)&b->annotations, &b->annotation_capacity,
        b->annotation_count + 1, sizeof(*a))) return b->error;
    a = b->annotations + b->annotation_count++; memset(a, 0, sizeof(*a));
    a->maximum = priority; a->iOriginIndex = origin_index;
    a->iKindDetail = kind | (detail << 16); a->iFlagsOrigin = flags | (origin_kind << 16);
    a->iSourceStart = start; a->iSourceEnd = end; a->iRecordIndex = record;
    return XUI_OK;
}
static int doc_si_add(doc_si_builder* b, uint64_t record, uint32_t kind,
    uint32_t detail, uint32_t flags, uint64_t start, uint64_t end, unsigned priority)
{
    return doc_si_add_origin(b, record, kind, detail, flags, start, end, priority,
        XUI_DOC_SOURCE_BINDING_RANGE, DOC_NONE);
}
static int doc_si_group(doc_si_builder* b, uint64_t record)
{
    doc_si_record_value* r = b->index->records + record;
    return doc_si_add(b, record, 0, 0, 0, r->iSourceStart, r->iSourceEnd, 0);
}
static int doc_si_blocks(doc_si_builder* b, uint64_t record, uint64_t node)
{
    xui_doc_block_syntax_t s = {0}; uint64_t i; int result;
    const uint32_t confirmed = XUI_DOC_SOURCE_TOKEN_PARSER_CONFIRMED;
    s.iSize = sizeof(s); result = xuiDocumentSnapshotGetBlockSyntax(b->index->snapshot, node, &s);
    if (result == XUI_ERROR_NOT_FOUND) return XUI_OK;
    if (result != XUI_OK) return b->error = result;
    /* The entire table separator is a coarse marker; its indexed underline
     * and pipe records take precedence. */
    doc_si_add_origin(b, record, XUI_DOC_SOURCE_TOKEN_BLOCK_MARKER, s.iKind, confirmed,
        s.iPrimaryStart, s.iPrimaryEnd, s.iKind == XUI_DOC_BLOCK_SYNTAX_TABLE_UNDERLINE ? 90 : 100,
        XUI_DOC_SOURCE_BINDING_BLOCK_PRIMARY, DOC_NONE);
    doc_si_add_origin(b, record, XUI_DOC_SOURCE_TOKEN_BLOCK_MARKER, s.iKind, confirmed,
        s.iSecondaryStart, s.iSecondaryEnd, 100, XUI_DOC_SOURCE_BINDING_BLOCK_SECONDARY, DOC_NONE);
    doc_si_add_origin(b, record, 0, 0, 0, s.iFenceTailStart, s.iFenceTailEnd, 1,
        XUI_DOC_SOURCE_BINDING_FENCE_TAIL, DOC_NONE);
    doc_si_add_origin(b, record, XUI_DOC_SOURCE_TOKEN_FENCE_INFO, s.iKind, confirmed,
        s.iFenceInfoStart, s.iFenceInfoEnd, 110, XUI_DOC_SOURCE_BINDING_FENCE_INFO, DOC_NONE);
    doc_si_add_origin(b, record, XUI_DOC_SOURCE_TOKEN_FENCE_INFO, s.iKind, confirmed,
        s.iFenceLanguageStart, s.iFenceLanguageEnd, 70, XUI_DOC_SOURCE_BINDING_FENCE_LANGUAGE, DOC_NONE);
    doc_si_add_origin(b, record, XUI_DOC_SOURCE_TOKEN_LIST_GAP, s.iKind, confirmed,
        s.iListMarkerGapStart, s.iListMarkerGapEnd, 100, XUI_DOC_SOURCE_BINDING_LIST_MARKER_GAP, DOC_NONE);
    doc_si_add_origin(b, record, XUI_DOC_SOURCE_TOKEN_LIST_GAP, s.iKind, confirmed,
        s.iTaskMarkerGapStart, s.iTaskMarkerGapEnd, 100, XUI_DOC_SOURCE_BINDING_TASK_MARKER_GAP, DOC_NONE);
    for (i = 0; !b->error && i < s.iQuotePrefixCount; i++) {
        uint64_t start, end;
        result = xuiDocumentSnapshotGetQuotePrefix(b->index->snapshot, node, i, &start, &end);
        if (result != XUI_OK) return b->error = result;
        doc_si_add_origin(b, record, XUI_DOC_SOURCE_TOKEN_BLOCK_MARKER, s.iKind, confirmed, start, end, 120,
            XUI_DOC_SOURCE_BINDING_QUOTE_PREFIX, i);
    }
    if (!b->error && s.iKind == XUI_DOC_BLOCK_SYNTAX_QUOTE_OPEN) {
        doc_node* n = doc_index_get(b->index->snapshot->state->index, node);
        doc_node_source_range range;
        if (!n || !n->syntax_aux || n->syntax_aux->size != s.iQuotePrefixCount * sizeof(doc_quote_indent_relative))
            return b->error = XUI_DOC_ERROR_FORMAT;
        doc_node_source_range_get(b->index->snapshot->state, n, &range);
        for (i = 0; !b->error && i < s.iQuotePrefixCount; i++) {
            doc_quote_indent_relative v;
            memcpy(&v, n->syntax_aux->data + i * sizeof(v), sizeof(v));
            if (v.indent_start > v.marker || v.marker >= v.end || v.end > range.syntax_end - range.syntax_start ||
                v.start_column > v.marker_column || v.marker_column >= v.content_column || v.content_column > v.end_column)
                return b->error = XUI_DOC_ERROR_FORMAT;
            doc_si_add_origin(b, record, XUI_DOC_SOURCE_TOKEN_CONTAINER_INDENT, s.iKind, confirmed,
                range.syntax_start + v.indent_start, range.syntax_start + v.end, 70,
                XUI_DOC_SOURCE_BINDING_QUOTE_INDENT, i);
        }
    }
    if (!b->error && s.iKind == XUI_DOC_BLOCK_SYNTAX_LIST_ITEM) {
        doc_node* n = doc_index_get(b->index->snapshot->state->index, node);
        doc_node_source_range range; doc_list_first_indent_relative v;
        if (!n || !n->syntax_aux || n->syntax_aux->size != sizeof(v)) return b->error = XUI_DOC_ERROR_FORMAT;
        memcpy(&v, n->syntax_aux->data, sizeof(v));
        doc_node_source_range_get(b->index->snapshot->state, n, &range);
        if (v.indent_start > v.start || v.start > v.end || v.end > range.syntax_end - range.syntax_start ||
            v.start_column >= v.content_column || v.content_column > v.end_column) return b->error = XUI_DOC_ERROR_FORMAT;
        doc_si_add_origin(b, record, XUI_DOC_SOURCE_TOKEN_CONTAINER_INDENT, s.iKind, confirmed,
            range.syntax_start + v.indent_start, range.syntax_start + v.end, 70,
            XUI_DOC_SOURCE_BINDING_LIST_FIRST_INDENT, 0);
    }
    for (i = 0; !b->error && i < s.iListIndentCount; i++) {
        xui_doc_list_indent_t v = {0}; v.iSize = sizeof(v);
        result = xuiDocumentSnapshotGetListContinuationIndent(b->index->snapshot, node, i, &v);
        if (result != XUI_OK) return b->error = result;
        doc_si_add_origin(b, record, XUI_DOC_SOURCE_TOKEN_CONTAINER_INDENT, s.iKind, confirmed,
            v.iSourceStart, v.iSourceEnd, 100, XUI_DOC_SOURCE_BINDING_LIST_CONTINUATION_INDENT, i);
    }
    for (i = 0; !b->error && i < s.iCodeIndentCount; i++) {
        xui_doc_code_indent_t v = {0}; v.iSize = sizeof(v);
        result = xuiDocumentSnapshotGetCodeIndent(b->index->snapshot, node, i, &v);
        if (result != XUI_OK) return b->error = result;
        doc_si_add_origin(b, record, XUI_DOC_SOURCE_TOKEN_CODE_INDENT, s.iKind, confirmed,
            v.iSourceStart, v.iSourceEnd, 110, XUI_DOC_SOURCE_BINDING_CODE_INDENT, i);
    }
    for (i = 0; !b->error && i < s.iTableTokenCount; i++) {
        xui_doc_table_token_t v = {0}; v.iSize = sizeof(v);
        result = xuiDocumentSnapshotGetTableToken(b->index->snapshot, node, i, &v);
        if (result != XUI_OK) return b->error = result;
        doc_si_add_origin(b, record, XUI_DOC_SOURCE_TOKEN_TABLE, v.iKind, confirmed,
            v.iSourceStart, v.iSourceEnd, 120, XUI_DOC_SOURCE_BINDING_TABLE_TOKEN, i);
    }
    return b->error;
}
static int doc_si_nodes(doc_si_builder* b, uint64_t id, uint64_t parent, unsigned depth)
{
    xui_doc_node_info_t n = {0}; uint64_t record, start, end, i; uint32_t flags = 0, html_kind = 0;
    int result; const uint32_t confirmed = XUI_DOC_SOURCE_TOKEN_PARSER_CONFIRMED;
    if (b->error || doc_si_step(b)) return b->error;
    if (depth > DOC_MAX_DEPTH) return b->error = XUI_DOC_ERROR_LIMIT;
    n.iSize = sizeof(n); result = xuiDocumentSnapshotGetNode(b->index->snapshot, id, &n);
    if (result != XUI_OK) return b->error = result;
    start = n.iSyntaxStart != DOC_NONE ? n.iSyntaxStart : n.iSourceStart;
    end = n.iSyntaxEnd != DOC_NONE ? n.iSyntaxEnd : n.iSourceEnd;
    if (!doc_si_span(b, start, end)) {
        if (start != DOC_NONE || end != DOC_NONE) return b->error = XUI_DOC_ERROR_FORMAT;
        flags |= XUI_DOC_SOURCE_RECORD_UNMAPPED;
    }
    if (n.iKind == XUI_DOC_HTML || n.iKind == XUI_DOC_FRONT_MATTER || n.iKind == XUI_DOC_EXTENSION)
        flags |= XUI_DOC_SOURCE_RECORD_OPAQUE;
    while (parent && (!(flags & XUI_DOC_SOURCE_RECORD_UNMAPPED)) &&
        ((b->index->records[parent].iFlags & XUI_DOC_SOURCE_RECORD_UNMAPPED) ||
         b->index->records[parent].iSourceStart > start || b->index->records[parent].iSourceEnd < end))
        parent = b->index->records[parent].iParentIndex;
    record = doc_si_record(b, XUI_DOC_SOURCE_RECORD_NODE, n.iKind, flags, parent,
        id, DOC_NONE, start, end, n.iSourceStart, n.iSourceEnd);
    if (record == DOC_NONE || doc_si_group(b, record)) return b->error;
    if (!(flags & XUI_DOC_SOURCE_RECORD_UNMAPPED) && start < end) {
        doc_si_interval* interval;
        if (doc_si_reserve(b, (void**)&b->intervals, &b->interval_capacity,
            b->interval_count + 1, sizeof(*interval))) return b->error;
        interval = b->intervals + b->interval_count++;
        interval->start = start; interval->end = end; interval->record = record; interval->maximum = end;
    }
    if (n.iKind == XUI_DOC_HTML && doc_si_span(b, n.iSourceStart, n.iSourceEnd)) {
        char prefix[4]; html_kind = XUI_DOC_SOURCE_TOKEN_HTML;
        if (n.iSourceEnd - n.iSourceStart >= sizeof(prefix)) {
            if (doc_si_read(b, n.iSourceStart, prefix, sizeof(prefix))) return b->error;
            if (!memcmp(prefix, "<!--", 4)) html_kind = XUI_DOC_SOURCE_TOKEN_HTML_COMMENT;
        }
        doc_si_add(b, record, html_kind, n.iKind, confirmed | XUI_DOC_SOURCE_TOKEN_OPAQUE_PAYLOAD,
            n.iSourceStart, n.iSourceEnd, 30);
    }
    for (i = 0; !b->error && i < n.iSourceSegmentCount; i++) {
        xui_doc_source_segment_t s = {0};
        s.iSize = sizeof(s); result = xuiDocumentSnapshotGetSourceSegment(b->index->snapshot, id, i, &s);
        if (result != XUI_OK) return b->error = result;
        if (s.iSourceStart == DOC_NONE && s.iSourceEnd == DOC_NONE) continue;
        if (!doc_si_span(b, s.iSourceStart, s.iSourceEnd) || s.iTextStart > s.iTextEnd ||
            s.iTextEnd > n.iTextBytes || s.iKind < XUI_DOC_SOURCE_DIRECT || s.iKind > XUI_DOC_SOURCE_SYNTHETIC ||
            (s.iKind == XUI_DOC_SOURCE_DIRECT && s.iTextEnd - s.iTextStart != s.iSourceEnd - s.iSourceStart))
            return b->error = XUI_DOC_ERROR_FORMAT;
        doc_si_add_origin(b, record, html_kind ? html_kind : XUI_DOC_SOURCE_TOKEN_CONTENT, n.iKind,
            confirmed | (flags & XUI_DOC_SOURCE_RECORD_OPAQUE ? XUI_DOC_SOURCE_TOKEN_OPAQUE_PAYLOAD : 0),
            s.iSourceStart, s.iSourceEnd, 80, XUI_DOC_SOURCE_BINDING_PROVENANCE, i);
        if (b->error) return b->error;
    }
    if (n.iKind == XUI_DOC_SOFT_BREAK || n.iKind == XUI_DOC_HARD_BREAK) {
        xui_doc_break_syntax_t v = {0}; v.iSize = sizeof(v);
        result = xuiDocumentSnapshotGetBreakSyntax(b->index->snapshot, id, &v);
        if (result != XUI_OK && result != XUI_ERROR_NOT_FOUND) return b->error = result;
        if (result == XUI_OK) {
            doc_si_add_origin(b, record, XUI_DOC_SOURCE_TOKEN_HORIZONTAL_SPACE, v.iKind, confirmed,
                v.iTrailingStart, v.iTrailingEnd, 100, XUI_DOC_SOURCE_BINDING_BREAK_TRAILING, DOC_NONE);
            doc_si_add_origin(b, record, XUI_DOC_SOURCE_TOKEN_BREAK_MARKER, v.iKind, confirmed,
                v.iHardMarkerStart, v.iHardMarkerEnd, 120, XUI_DOC_SOURCE_BINDING_BREAK_HARD_MARKER, DOC_NONE);
            doc_si_add_origin(b, record, XUI_DOC_SOURCE_TOKEN_LINE_ENDING, v.iKind, confirmed,
                v.iLineEndingStart, v.iLineEndingEnd, 110, XUI_DOC_SOURCE_BINDING_BREAK_LINE_ENDING, DOC_NONE);
        }
    } else if (doc_si_blocks(b, record, id)) return b->error;
    for (i = 0; !b->error && i < n.iChildCount; i++) {
        uint64_t child;
        result = xuiDocumentSnapshotGetChild(b->index->snapshot, id, i, &child);
        if (result != XUI_OK) return b->error = result;
        doc_si_nodes(b, child, record, depth + 1);
    }
    return b->error;
}
static int doc_si_interval_compare(const void* left, const void* right)
{
    const doc_si_interval *a = left, *b = right;
    if (a->start != b->start) return a->start < b->start ? -1 : 1;
    if (a->end != b->end) return a->end > b->end ? -1 : 1;
    return a->record < b->record ? -1 : a->record != b->record;
}
static int doc_si_event_compare(const void* left, const void* right)
{
    const doc_si_event *a = left, *b = right;
    if (a->offset != b->offset) return a->offset < b->offset ? -1 : 1;
    if ((a->annotation_phase & 1) != (b->annotation_phase & 1))
        return (a->annotation_phase & 1) < (b->annotation_phase & 1) ? -1 : 1;
    return a->annotation_phase < b->annotation_phase ? -1 : a->annotation_phase != b->annotation_phase;
}
static void doc_si_swap(char* a, char* c, size_t unit)
{
    char temp[64]; memcpy(temp, a, unit); memcpy(a, c, unit); memcpy(c, temp, unit);
}
static int doc_si_sort_sift(doc_si_builder* b, char* data, uint64_t root, uint64_t count,
    size_t unit, int (*compare)(const void*, const void*))
{
    while (count > 1 && root <= (count - 2) / 2) {
        uint64_t child = root * 2 + 1;
        if (doc_si_step(b)) return b->error;
        if (child + 1 < count && compare(data + child * unit, data + (child + 1) * unit) < 0) child++;
        if (compare(data + root * unit, data + child * unit) >= 0) break;
        doc_si_swap(data + root * unit, data + child * unit, unit); root = child;
    }
    return XUI_OK;
}
static int doc_si_sort(doc_si_builder* b, void* data, uint64_t count, size_t unit,
    int (*compare)(const void*, const void*))
{
    uint64_t at; char* bytes = data;
    if (count < 2) return b->error;
    if (unit > 64) return b->error = XUI_DOC_ERROR_FORMAT;
    for (at = count / 2; at && !b->error; at--) doc_si_sort_sift(b, bytes, at - 1, count, unit, compare);
    for (at = count; at > 1 && !b->error; at--) {
        doc_si_swap(bytes, bytes + (at - 1) * unit, unit);
        doc_si_sort_sift(b, bytes, 0, at - 1, unit, compare);
    }
    return b->error;
}
static uint64_t doc_si_interval_maximum(doc_si_builder* b, uint64_t start, uint64_t end)
{
    uint64_t middle, maximum, left, right;
    if (start == end || b->error) return 0;
    if (doc_si_step(b)) return 0;
    middle = start + (end - start) / 2; maximum = b->intervals[middle].end;
    left = doc_si_interval_maximum(b, start, middle); right = doc_si_interval_maximum(b, middle + 1, end);
    if (maximum < left) maximum = left;
    if (maximum < right) maximum = right;
    b->intervals[middle].maximum = maximum; return maximum;
}
static void doc_si_owner_search(doc_si_builder* b, uint64_t lo, uint64_t hi,
    uint64_t start, uint64_t end, uint64_t* best)
{
    uint64_t mid; doc_si_interval* v; doc_si_record_value* old;
    if (lo == hi || b->error || doc_si_step(b)) return;
    mid = lo + (hi - lo) / 2; v = b->intervals + mid;
    if (v->maximum < end || b->intervals[lo].start > start) return;
    if (v->start <= start && v->end >= end) {
        old = b->index->records + *best;
        if (v->end - v->start < old->iSourceEnd - old->iSourceStart ||
            (v->end - v->start == old->iSourceEnd - old->iSourceStart && v->record > *best))
            *best = v->record;
    }
    doc_si_owner_search(b, lo, mid, start, end, best);
    if (v->start <= start) doc_si_owner_search(b, mid + 1, hi, start, end, best);
}
static uint64_t doc_si_owner(doc_si_builder* b, uint64_t start, uint64_t end)
{
    uint64_t best = 0;
    doc_si_owner_search(b, 0, b->interval_count, start, end, &best); return best;
}
static int doc_si_inline(doc_si_builder* b, const xui_doc_source_info_t* info)
{
    uint64_t base = b->index->info.iRecordCount, i;
    const uint32_t confirmed = XUI_DOC_SOURCE_TOKEN_PARSER_CONFIRMED;
    for (i = 0; !b->error && i < info->iInlineSyntaxCount; i++) {
        xui_doc_inline_syntax_t s = {0}; uint64_t parent, record; int result;
        s.iSize = sizeof(s); result = xuiDocumentSnapshotGetInlineSyntax(b->index->snapshot, i, &s);
        if (result != XUI_OK) return b->error = result;
        if (!doc_si_span(b, s.iSourceStart, s.iSourceEnd) || s.iContentStart < s.iSourceStart ||
            s.iContentStart > s.iContentEnd || s.iContentEnd > s.iSourceEnd ||
            (s.iParentIndex != DOC_NONE && s.iParentIndex >= i)) return b->error = XUI_DOC_ERROR_FORMAT;
        parent = s.iParentIndex == DOC_NONE ? doc_si_owner(b, s.iSourceStart, s.iSourceEnd) : base + s.iParentIndex;
        if (b->error) return b->error;
        if (b->index->records[parent].iSourceStart > s.iSourceStart ||
            b->index->records[parent].iSourceEnd < s.iSourceEnd) return b->error = XUI_DOC_ERROR_FORMAT;
        record = doc_si_record(b, XUI_DOC_SOURCE_RECORD_INLINE, s.iKind, 0, parent,
            0, i, s.iSourceStart, s.iSourceEnd, s.iContentStart, s.iContentEnd);
        if (record == DOC_NONE || doc_si_group(b, record)) return b->error;
        doc_si_add_origin(b, record, XUI_DOC_SOURCE_TOKEN_INLINE_MARKER, s.iKind, confirmed,
            s.iSourceStart, s.iContentStart, 90, XUI_DOC_SOURCE_BINDING_INLINE_OPEN, DOC_NONE);
        doc_si_add_origin(b, record, XUI_DOC_SOURCE_TOKEN_INLINE_MARKER, s.iKind, confirmed,
            s.iContentEnd, s.iSourceEnd, 90, XUI_DOC_SOURCE_BINDING_INLINE_CLOSE, DOC_NONE);
        if (s.iKind == XUI_DOC_SYNTAX_DESTINATION || s.iKind == XUI_DOC_SYNTAX_TITLE) {
            doc_si_add_origin(b, record, XUI_DOC_SOURCE_TOKEN_FIELD_CONTENT, s.iKind, confirmed,
                s.iContentStart, s.iContentEnd, 110, XUI_DOC_SOURCE_BINDING_INLINE_FIELD, DOC_NONE);
        }
    }
    return b->error;
}
#include "xui_document_cst_index.h"
static int doc_si_references(doc_si_builder* b, const xui_doc_source_info_t* info)
{
    uint64_t i; const uint32_t confirmed = XUI_DOC_SOURCE_TOKEN_PARSER_CONFIRMED;
    for (i = 0; !b->error && i < info->iReferenceDefinitionCount; i++) {
        xui_doc_reference_definition_t s = {0}; doc_source_cst_header syntax;
        uint64_t record, parent; int result, has_syntax;
        s.iSize = sizeof(s); result = xuiDocumentSnapshotGetReferenceDefinition(b->index->snapshot, i, &s);
        if (result != XUI_OK) return b->error = result;
        if (!doc_si_span(b, s.iSourceStart, s.iSourceEnd)) return b->error = XUI_DOC_ERROR_FORMAT;
        result = doc_source_cst_info(b->index->snapshot->state, i, &syntax);
        if (result != XUI_OK && result != XUI_ERROR_NOT_FOUND) return b->error = result;
        has_syntax = result == XUI_OK;
        parent = doc_si_owner(b, s.iSourceStart, s.iSourceEnd);
        record = doc_si_record(b, XUI_DOC_SOURCE_RECORD_DEFINITION, s.iKind,
            !has_syntax && s.iKind == XUI_DOC_REFERENCE_FOOTNOTE ? XUI_DOC_SOURCE_RECORD_OPAQUE : 0,
            parent, 0, i, s.iSourceStart, s.iSourceEnd, DOC_NONE, DOC_NONE);
        if (record == DOC_NONE) return b->error;
        if (has_syntax) doc_si_cst_records(b, i, record, s.iSourceStart, &syntax, 0);
        else doc_si_add(b, record, s.iKind == XUI_DOC_REFERENCE_FOOTNOTE ? XUI_DOC_SOURCE_TOKEN_OPAQUE : XUI_DOC_SOURCE_TOKEN_REFERENCE_SYNTAX,
            s.iKind, confirmed | (s.iKind == XUI_DOC_REFERENCE_FOOTNOTE ? XUI_DOC_SOURCE_TOKEN_OPAQUE_PAYLOAD : 0),
            s.iSourceStart, s.iSourceEnd, 40);
        doc_si_add_origin(b, record, XUI_DOC_SOURCE_TOKEN_REFERENCE_LABEL, s.iKind, confirmed,
            s.iLabelStart, s.iLabelEnd, 110, XUI_DOC_SOURCE_BINDING_REFERENCE_LABEL, DOC_NONE);
        doc_si_add_origin(b, record, XUI_DOC_SOURCE_TOKEN_REFERENCE_DESTINATION, s.iKind, confirmed,
            s.iDestinationStart, s.iDestinationEnd, 110, XUI_DOC_SOURCE_BINDING_REFERENCE_DESTINATION, DOC_NONE);
        doc_si_add_origin(b, record, XUI_DOC_SOURCE_TOKEN_REFERENCE_TITLE, s.iKind, confirmed,
            s.iTitleStart, s.iTitleEnd, 110, XUI_DOC_SOURCE_BINDING_REFERENCE_TITLE, DOC_NONE);
    }
    for (i = 0; !b->error && i < info->iReferenceCandidateCount; i++) {
        xui_doc_inline_syntax_t s = {0}; uint64_t record, parent; int result;
        s.iSize = sizeof(s); result = xuiDocumentSnapshotGetReferenceCandidate(b->index->snapshot, i, &s);
        if (result != XUI_OK) return b->error = result;
        if (!doc_si_span(b, s.iSourceStart, s.iSourceEnd) || s.iContentStart < s.iSourceStart ||
            s.iContentStart > s.iContentEnd || s.iContentEnd > s.iSourceEnd) return b->error = XUI_DOC_ERROR_FORMAT;
        parent = doc_si_owner(b, s.iSourceStart, s.iSourceEnd);
        record = doc_si_record(b, XUI_DOC_SOURCE_RECORD_CANDIDATE, s.iKind, 0, parent,
            0, i, s.iSourceStart, s.iSourceEnd, s.iContentStart, s.iContentEnd);
        if (record == DOC_NONE || doc_si_group(b, record)) return b->error;
    }
    return b->error;
}
static int doc_si_body(doc_si_builder* b)
{
    uint64_t i, count = doc_source_forest_count(b->index->snapshot->state);
    for (i = 0; !b->error && i < count; i++) {
        uint64_t base, record; doc_source_cst_header header;
        if (doc_si_step(b)) return b->error;
        b->error = doc_source_forest_info(b->index->snapshot->state, i, &base, &header);
        if (b->error) return b->error;
        record = doc_si_record(b, XUI_DOC_SOURCE_RECORD_SYNTAX, XUI_DOC_SOURCE_SYNTAX_SCOPE,
            XUI_DOC_SOURCE_RECORD_SYNTAX_DOCUMENT, 0, 0, i, base, base + header.source_bytes,
            base, base + header.source_bytes);
        if (record == DOC_NONE) return b->error;
        doc_si_cst_records(b, i, record, base, &header, 1);
    }
    return b->error;
}
static int doc_si_preferred(const doc_si_builder* b, uint64_t left, uint64_t right)
{
    const doc_si_annotation *a = b->annotations + left, *c = b->annotations + right;
    uint64_t a_span = a->iSourceEnd - a->iSourceStart, c_span = c->iSourceEnd - c->iSourceStart;
    if (a->maximum != c->maximum) return a->maximum > c->maximum;
    if (a_span != c_span) return a_span < c_span;
    if (a->iRecordIndex != c->iRecordIndex) return a->iRecordIndex > c->iRecordIndex;
    return left > right;
}
static void doc_si_heap_push(doc_si_builder* b, uint64_t value)
{
    uint64_t at = b->heap_count++;
    while (at) {
        uint64_t parent = (at - 1) / 2;
        if (doc_si_step(b) || !doc_si_preferred(b, value, b->heap[parent])) break;
        b->heap[at] = b->heap[parent]; at = parent;
    }
    b->heap[at] = value;
}
static void doc_si_heap_pop(doc_si_builder* b)
{
    uint64_t at = 0, value = b->heap[--b->heap_count];
    while (b->heap_count > 1 && at <= (b->heap_count - 2) / 2) {
        uint64_t child = at * 2 + 1;
        if (doc_si_step(b)) break;
        if (child + 1 < b->heap_count && doc_si_preferred(b, b->heap[child + 1], b->heap[child])) child++;
        if (!doc_si_preferred(b, b->heap[child], value)) break;
        b->heap[at] = b->heap[child]; at = child;
    }
    if (b->heap_count) b->heap[at] = value;
}
static int doc_si_same_token(const doc_si_token_value* a, const doc_si_token_value* c)
{
    return a->iSourceEnd == c->iSourceStart && a->iKind == c->iKind && a->iDetail == c->iDetail &&
        a->iFlags == c->iFlags && a->iRecordIndex == c->iRecordIndex && a->iProvenanceIndex == c->iProvenanceIndex;
}
static int doc_si_emit(doc_si_builder* b, const doc_si_annotation* origin,
    uint64_t start, uint64_t end, uint32_t kind)
{
    doc_si_token_value token = {0}, *previous;
    if (start == end) return b->error;
    if (b->error || doc_si_step(b)) return b->error;
    token.iSourceStart = start; token.iSourceEnd = end; token.iKind = kind;
    token.iDetail = origin->iKindDetail >> 16; token.iFlags = origin->iFlagsOrigin & UINT16_MAX;
    token.iRecordIndex = origin->iRecordIndex;
    token.iProvenanceIndex = (origin->iFlagsOrigin >> 16) == XUI_DOC_SOURCE_BINDING_PROVENANCE ||
        (origin->iFlagsOrigin >> 16) == XUI_DOC_SOURCE_BINDING_DEFINITION_SYNTAX ||
        (origin->iFlagsOrigin >> 16) == XUI_DOC_SOURCE_BINDING_DOCUMENT_SYNTAX ? origin->iOriginIndex : DOC_NONE;
    if (kind == XUI_DOC_SOURCE_TOKEN_UNCLASSIFIED) b->index->info.iUnclassifiedBytes += end - start;
    if (token.iFlags & XUI_DOC_SOURCE_TOKEN_OPAQUE_PAYLOAD) b->index->info.iOpaqueBytes += end - start;
    previous = b->index->info.iTokenCount ? (b->count_tokens ? &b->previous_token :
        b->index->tokens + b->index->info.iTokenCount - 1) : NULL;
    if (previous && doc_si_same_token(previous, &token)) { previous->iSourceEnd = end; return XUI_OK; }
    if (b->count_tokens) {
        b->previous_token = token; b->index->info.iTokenCount++; return XUI_OK;
    }
    if (doc_si_reserve(b, (void**)&b->index->tokens, &b->token_capacity,
        b->index->info.iTokenCount + 1, sizeof(token))) return b->error;
    b->index->tokens[b->index->info.iTokenCount++] = token; return XUI_OK;
}
static unsigned doc_si_raw_kind(unsigned char c, unsigned fallback)
{
    return c == ' ' || c == '\t' ? XUI_DOC_SOURCE_TOKEN_HORIZONTAL_SPACE :
        c == '\r' || c == '\n' ? XUI_DOC_SOURCE_TOKEN_LINE_ENDING : fallback;
}
static int doc_si_residual(doc_si_builder* b, const doc_si_annotation* origin,
    uint64_t start, uint64_t end, unsigned fallback)
{
    unsigned char buffer[4096]; uint64_t at = start;
    while (at < end && !b->error) {
        uint64_t count = end - at < sizeof(buffer) ? end - at : sizeof(buffer), i = 0;
        if (doc_si_read(b, at, buffer, count)) return b->error;
        while (i < count) {
            uint64_t first = i; unsigned kind = doc_si_raw_kind(buffer[i++], fallback);
            while (i < count && doc_si_raw_kind(buffer[i], fallback) == kind) i++;
            if (doc_si_emit(b, origin, at + first, at + i, kind)) return b->error;
        }
        at += count;
    }
    return b->error;
}
static doc_si_event doc_si_event_get(const doc_si_builder* b, uint64_t ordinal)
{
    doc_si_event result;
    if (b->event_unit == sizeof(doc_si_event32)) {
        const doc_si_event32* events = (const doc_si_event32*)b->events;
        result.offset = events[ordinal].offset; result.annotation_phase = events[ordinal].annotation_phase;
    } else result = b->events[ordinal];
    return result;
}
static void doc_si_event_set(doc_si_builder* b, uint64_t ordinal, uint64_t offset, uint64_t phase)
{
    if (b->event_unit == sizeof(doc_si_event32)) {
        doc_si_event32* events = (doc_si_event32*)b->events;
        events[ordinal].offset = (uint32_t)offset; events[ordinal].annotation_phase = (uint32_t)phase;
    } else { b->events[ordinal].offset = offset; b->events[ordinal].annotation_phase = phase; }
}
static int doc_si_event32_compare(const void* left, const void* right)
{
    const doc_si_event32 *a = left, *c = right;
    if (a->offset != c->offset) return a->offset < c->offset ? -1 : 1;
    if ((a->annotation_phase & 1) != (c->annotation_phase & 1))
        return (a->annotation_phase & 1) < (c->annotation_phase & 1) ? -1 : 1;
    return a->annotation_phase < c->annotation_phase ? -1 : a->annotation_phase != c->annotation_phase;
}
static int doc_si_sweep(doc_si_builder* b, uint64_t count)
{
    uint64_t next, at = 0, event = 0, size = b->index->info.iSourceBytes;
    while (at < size && !b->error) {
        const doc_si_annotation* annotation; uint32_t kind;
        if (doc_si_step(b)) return b->error;
        while (event < count && doc_si_event_get(b, event).offset == at && !b->error) {
            uint64_t phase = doc_si_event_get(b, event).annotation_phase;
            if (doc_si_step(b)) return b->error;
            if (phase & 1) doc_si_heap_push(b, phase >> 1);
            event++;
        }
        while (b->heap_count && !b->error && b->annotations[b->heap[0]].iSourceEnd <= at)
            doc_si_heap_pop(b);
        if (b->error) return b->error;
        if (!b->heap_count) return b->error = XUI_DOC_ERROR_FORMAT;
        next = event < count ? doc_si_event_get(b, event).offset : size;
        if (next <= at || next > size) return b->error = XUI_DOC_ERROR_FORMAT;
        annotation = b->annotations + b->heap[0]; kind = annotation->iKindDetail & UINT16_MAX;
        if (!kind || kind == XUI_DOC_SOURCE_TOKEN_INLINE_MARKER ||
            kind == XUI_DOC_SOURCE_TOKEN_BLOCK_MARKER ||
            kind == XUI_DOC_SOURCE_TOKEN_REFERENCE_SYNTAX || kind == XUI_DOC_SOURCE_TOKEN_TABLE)
            doc_si_residual(b, annotation, at, next, kind ? kind : XUI_DOC_SOURCE_TOKEN_UNCLASSIFIED);
        else doc_si_emit(b, annotation, at, next, kind);
        at = next;
    }
    return b->error;
}
static int doc_si_partition(doc_si_builder* b)
{
    uint64_t i, count = 0, next = 0, tokens;
    if (b->annotation_count > UINT64_MAX / 2) return b->error = XUI_DOC_ERROR_LIMIT;
    for (i = 0; i < b->annotation_count; i++) {
        if (doc_si_step(b)) return b->error;
        if (b->annotations[i].iSourceStart < b->annotations[i].iSourceEnd) count += 2;
    }
    b->event_unit = b->index->info.iSourceBytes <= UINT32_MAX && b->annotation_count <= UINT32_MAX / 2 ?
        sizeof(doc_si_event32) : sizeof(doc_si_event);
    if (doc_si_exact(b, (void**)&b->events, &b->event_capacity, count, b->event_unit) ||
        doc_si_exact(b, (void**)&b->heap, &b->heap_capacity, b->annotation_count, sizeof(*b->heap))) return b->error;
    for (i = 0; !b->error && i < b->annotation_count; i++) {
        if (doc_si_step(b)) return b->error;
        if (b->annotations[i].iSourceStart == b->annotations[i].iSourceEnd) continue;
        doc_si_event_set(b, next++, b->annotations[i].iSourceStart, (i << 1) | 1);
        doc_si_event_set(b, next++, b->annotations[i].iSourceEnd, i << 1);
    }
    if (doc_si_sort(b, b->events, count, b->event_unit,
        b->event_unit == sizeof(doc_si_event32) ? doc_si_event32_compare : doc_si_event_compare)) return b->error;
    /* Count the same deterministic partition before allocating its immutable
     * tokens. This is another sweep of existing annotations, not a parse or a
     * second document. Neither growing tokens nor duplicating old storage is
     * needed while the event workspace is live. */
    b->count_tokens = 1;
    if (doc_si_sweep(b, count)) return b->error;
    tokens = b->index->info.iTokenCount;
    b->index->info.iTokenCount = b->index->info.iUnclassifiedBytes = b->index->info.iOpaqueBytes = 0;
    b->heap_count = 0; b->count_tokens = 0;
    if (doc_si_exact(b, (void**)&b->index->tokens, &b->token_capacity, tokens, sizeof(*b->index->tokens))) return b->error;
    if (doc_si_sweep(b, count)) return b->error;
    if (tokens != b->index->info.iTokenCount) return b->error = XUI_DOC_ERROR_FORMAT;
    return XUI_OK;
}
static int doc_si_binding_compare(const void* left, const void* right)
{
    const doc_si_binding_value *a = left, *c = right;
    uint32_t a_origin = a->iFlagsOrigin >> 16, c_origin = c->iFlagsOrigin >> 16;
    if (a->iSourceStart != c->iSourceStart) return a->iSourceStart < c->iSourceStart ? -1 : 1;
    if (a->iSourceEnd != c->iSourceEnd) return a->iSourceEnd > c->iSourceEnd ? -1 : 1;
    if (a->iRecordIndex != c->iRecordIndex) return a->iRecordIndex < c->iRecordIndex ? -1 : 1;
    if (a_origin != c_origin) return a_origin < c_origin ? -1 : 1;
    if (a->iOriginIndex != c->iOriginIndex) return a->iOriginIndex < c->iOriginIndex ? -1 : 1;
    if (a->iKindDetail != c->iKindDetail) return a->iKindDetail < c->iKindDetail ? -1 : 1;
    return a->iFlagsOrigin < c->iFlagsOrigin ? -1 : a->iFlagsOrigin != c->iFlagsOrigin;
}
static uint64_t doc_si_binding_maximum(doc_si_builder* b, uint64_t lo, uint64_t hi, int* anchored)
{
    uint64_t mid, maximum, child; int anchor, child_anchor;
    doc_si_binding_value* v;
    *anchored = 0;
    if (lo == hi || b->error || doc_si_step(b)) return 0;
    mid = lo + (hi - lo) / 2; v = b->index->bindings + mid;
    maximum = v->iSourceEnd; anchor = v->iSourceStart == maximum;
    child = doc_si_binding_maximum(b, lo, mid, &child_anchor);
    if (child > maximum) { maximum = child; anchor = child_anchor; }
    else if (child == maximum) anchor |= child_anchor;
    child = doc_si_binding_maximum(b, mid + 1, hi, &child_anchor);
    if (child > maximum) { maximum = child; anchor = child_anchor; }
    else if (child == maximum) anchor |= child_anchor;
    v->maximum = maximum;
    if (anchor) v->iFlagsOrigin |= DOC_SI_MAXIMUM_ANCHOR;
    *anchored = anchor; return maximum;
}
static int doc_si_bindings(doc_si_builder* b)
{
    uint64_t i; int anchored;
    if (doc_si_step(b)) return b->error;
    b->index->bindings = b->annotations;
    b->binding_capacity = b->annotation_capacity;
    b->annotations = NULL; b->annotation_capacity = 0;
    b->index->info.iBindingCount = b->annotation_count;
    for (i = 0; !b->error && i < b->annotation_count; i++) {
        doc_si_binding_value* v = b->index->bindings + i;
        if (doc_si_step(b)) return b->error;
        v->maximum = 0;
        if (!(v->iKindDetail & UINT16_MAX)) v->iKindDetail |= XUI_DOC_SOURCE_TOKEN_SCOPE;
    }
    if (!b->error) doc_si_sort(b, b->index->bindings, b->annotation_count,
        sizeof(*b->index->bindings), doc_si_binding_compare);
    if (!b->error) doc_si_binding_maximum(b, 0, b->annotation_count, &anchored);
    /* Validate snapshot-backed metadata before publishing. Queries then cannot
     * expose a partially valid source index or defer malformed indents. */
    for (i = 0; !b->error && i < b->annotation_count; i++) {
        xui_doc_source_binding_t binding = {0}; binding.iSize = sizeof(binding);
        if (doc_si_step(b)) return b->error;
        b->error = xuiDocumentSourceIndexGetBinding(b->index, i, &binding);
    }
    return b->error;
}
static void doc_si_work_free(doc_si_builder* b, void** memory, uint64_t* capacity, size_t unit)
{
    if (*memory) { doc_free(*memory); b->used -= *capacity * unit + sizeof(doc_allocation); }
    *memory = NULL; *capacity = 0;
}
XUI_API void xuiDocumentSourceIndexRetain(xui_document_source_index index)
{ if (index) atomic_fetch_add(&index->refs, 1); }
XUI_API void xuiDocumentSourceIndexRelease(xui_document_source_index index)
{
    xui_document_snapshot snapshot;
    if (!index || atomic_fetch_sub(&index->refs, 1) != 1) return;
    snapshot = index->snapshot; doc_free(index->records); doc_free(index->tokens);
    doc_free(index->bindings); doc_free(index);
    xuiDocumentSnapshotRelease(snapshot);
}
XUI_API int xuiDocumentSnapshotCreateSourceIndex(xui_document_snapshot snapshot,
    const xui_doc_source_index_desc_t* desc, xui_document_source_index* out)
{
    doc_si_builder b = {0}; xui_doc_source_info_t info = {0}; uint64_t i, child;
    doc_node* root; int result;
    if (!out) return XUI_ERROR_INVALID_ARGUMENT;
    *out = NULL;
    if (!snapshot || (desc && (desc->iSize != sizeof(*desc) || desc->iReserved))) return XUI_ERROR_INVALID_ARGUMENT;
    if (snapshot->state->profile != XUI_DOCUMENT_MARKDOWN) return XUI_ERROR_UNSUPPORTED;
    b.limit = desc && desc->iMaxWorkingBytes ? desc->iMaxWorkingBytes : UINT64_C(64) * 1024 * 1024;
    b.cancel = desc ? desc->onCancel : NULL; b.cancel_user = desc ? desc->pCancelUser : NULL;
    if (b.cancel && b.cancel(b.cancel_user)) return XUI_DOC_ERROR_CANCELLED;
    b.used = b.peak = sizeof(struct xui_doc_source_index_t) + sizeof(doc_allocation);
    if (b.used > b.limit) return XUI_DOC_ERROR_LIMIT;
    b.index = doc_alloc(snapshot->state->allocator, sizeof(*b.index));
    if (!b.index) return XUI_ERROR_OUT_OF_MEMORY;
    atomic_init(&b.index->refs, 1); b.index->snapshot = snapshot; xuiDocumentSnapshotRetain(snapshot);
    b.index->info.iSize = sizeof(b.index->info);
    b.index->info.iDocumentId = snapshot->identity; b.index->info.iRevision = snapshot->revision;
    b.index->info.iSourceBytes = doc_seq_size(snapshot->state->source);
    info.iSize = sizeof(info); b.error = xuiDocumentSnapshotGetSourceInfo(snapshot, &info);
    if (!b.error) doc_si_count_records(&b, &info);
    if (b.error) { xuiDocumentSourceIndexRelease(b.index); return b.error; }
    doc_si_record(&b, XUI_DOC_SOURCE_RECORD_ROOT, 0, 0, DOC_NONE, DOC_ROOT, DOC_NONE,
        0, b.index->info.iSourceBytes, 0, b.index->info.iSourceBytes);
    if (!b.error) doc_si_group(&b, 0);
    root = doc_index_get(snapshot->state->index, DOC_ROOT);
    if (!root) b.error = XUI_DOC_ERROR_SCHEMA;
    for (i = 0; !b.error && i < doc_seq_size(root->children); i++) {
        result = xuiDocumentSnapshotGetChild(snapshot, DOC_ROOT, i, &child);
        if (result != XUI_OK) { b.error = result; break; }
        doc_si_nodes(&b, child, 0, 1);
    }
    if (!b.error) doc_si_sort(&b, b.intervals, b.interval_count, sizeof(*b.intervals), doc_si_interval_compare);
    if (!b.error) doc_si_interval_maximum(&b, 0, b.interval_count);
    if (!b.error) doc_si_inline(&b, &info);
    if (!b.error) doc_si_references(&b, &info);
    if (!b.error) doc_si_body(&b);
    if (!b.error && b.index->info.iSourceBytes >= 3) {
        unsigned char bom[3];
        if (!doc_si_read(&b, 0, bom, sizeof(bom)) && !memcmp(bom, "\xef\xbb\xbf", 3))
            doc_si_add(&b, 0, XUI_DOC_SOURCE_TOKEN_BOM, 0, 0, 0, 3, 200);
    }
    /* Ownership intervals are only needed while attaching inline/definition
     * annotations. End their lifetime before the event/token sweep grows its
     * buffers, rather than charging both independent phases simultaneously. */
    doc_si_work_free(&b, (void**)&b.intervals, &b.interval_capacity, sizeof(*b.intervals));
    if (!b.error && b.index->info.iRecordCount != b.expected_records) b.error = XUI_DOC_ERROR_FORMAT;
    if (!b.error) doc_si_partition(&b);
    doc_si_work_free(&b, (void**)&b.events, &b.event_capacity, b.event_unit);
    doc_si_work_free(&b, (void**)&b.heap, &b.heap_capacity, sizeof(*b.heap));
    if (!b.error) doc_si_bindings(&b);
    if (!b.error && b.cancel && b.cancel(b.cancel_user)) b.error = XUI_DOC_ERROR_CANCELLED;
    doc_free(b.annotations);
    if (b.error) { xuiDocumentSourceIndexRelease(b.index); return b.error; }
    b.index->info.iPeakWorkingBytes = b.peak; b.index->info.iBuildSteps = b.steps; b.index->info.iSourceBytesRead = b.reads;
    b.index->info.iAnnotationCount = b.annotation_count;
    b.index->info.iAllocatedBytes = sizeof(*b.index) + sizeof(doc_allocation) +
        b.record_capacity * sizeof(*b.index->records) + sizeof(doc_allocation) +
        (b.token_capacity ? b.token_capacity * sizeof(*b.index->tokens) + sizeof(doc_allocation) : 0) +
        (b.binding_capacity ? b.binding_capacity * sizeof(*b.index->bindings) + sizeof(doc_allocation) : 0);
    *out = b.index; return XUI_OK;
}
XUI_API int xuiDocumentSourceIndexGetInfo(xui_document_source_index index, xui_doc_source_index_info_t* out)
{
    if (!out || out->iSize != sizeof(*out)) return XUI_ERROR_INVALID_ARGUMENT;
    memset(out, 0, sizeof(*out)); out->iSize = sizeof(*out);
    if (!index) return XUI_ERROR_INVALID_ARGUMENT;
    *out = index->info; return XUI_OK;
}
XUI_API int xuiDocumentSourceIndexGetRecord(xui_document_source_index index,
    uint64_t ordinal, xui_doc_source_record_t* out)
{
    if (!out || out->iSize != sizeof(*out)) return XUI_ERROR_INVALID_ARGUMENT;
    memset(out, 0, sizeof(*out)); out->iSize = sizeof(*out);
    if (!index) return XUI_ERROR_INVALID_ARGUMENT;
    if (ordinal >= index->info.iRecordCount) return XUI_ERROR_NOT_FOUND;
    {
        const doc_si_record_value* r = index->records + ordinal;
        out->iKind = r->iKindDetail & UINT16_MAX; out->iDetail = r->iKindDetail >> 16; out->iFlags = r->iFlags;
        out->iParentIndex = r->iParentIndex; out->iNodeId = r->iNodeId; out->iOriginIndex = r->iOriginIndex;
        out->iSourceStart = r->iSourceStart; out->iSourceEnd = r->iSourceEnd;
        out->iContentStart = r->iContentStart; out->iContentEnd = r->iContentEnd;
    }
    return XUI_OK;
}
static int doc_si_decode(xui_document_source_index index, const doc_si_token_value* t,
    uint64_t provenance, xui_doc_source_token_t* out)
{
    {
        xui_doc_source_token_t result = {0}; result.iSize = sizeof(result);
        result.iKind = t->iKind; result.iDetail = t->iDetail; result.iFlags = t->iFlags;
        result.iSourceStart = t->iSourceStart; result.iSourceEnd = t->iSourceEnd; result.iRecordIndex = t->iRecordIndex;
        result.iNodeId = index->records[t->iRecordIndex].iNodeId;
        result.iTextStart = result.iTextEnd = result.iOriginalSourceStart = result.iOriginalSourceEnd = DOC_NONE;
        if (provenance != DOC_NONE && ((index->records[t->iRecordIndex].iKindDetail & UINT16_MAX) == XUI_DOC_SOURCE_RECORD_SYNTAX ||
            (index->records[t->iRecordIndex].iKindDetail & UINT16_MAX) == XUI_DOC_SOURCE_RECORD_DEFINITION)) {
            doc_source_cst_record s; uint64_t base; int code = doc_si_cst_origin(index, t->iRecordIndex, provenance, &s, &base);
            if (code != XUI_OK) return code;
            result.iOriginalSourceStart = base + s.start; result.iOriginalSourceEnd = base + s.end;
            if (result.iOriginalSourceStart > t->iSourceStart || result.iOriginalSourceEnd < t->iSourceEnd) return XUI_DOC_ERROR_FORMAT;
            if (s.kind == DOC_CST_TEXT) result.iSegmentKind = s.flags;
            if (t->iSourceStart != result.iOriginalSourceStart) result.iSegmentFlags |= XUI_DOC_SOURCE_PARTIAL_START;
            if (t->iSourceEnd != result.iOriginalSourceEnd) result.iSegmentFlags |= XUI_DOC_SOURCE_PARTIAL_END;
        } else if (provenance != DOC_NONE) {
            xui_doc_source_segment_t s = {0}; int code; s.iSize = sizeof(s);
            code = xuiDocumentSnapshotGetSourceSegment(index->snapshot, result.iNodeId, provenance, &s);
            if (code != XUI_OK) return code;
            if (s.iSourceStart > t->iSourceStart || s.iSourceEnd < t->iSourceEnd || s.iTextStart > s.iTextEnd)
                return XUI_DOC_ERROR_FORMAT;
            result.iTextStart = s.iTextStart; result.iTextEnd = s.iTextEnd;
            result.iOriginalSourceStart = s.iSourceStart; result.iOriginalSourceEnd = s.iSourceEnd;
            result.iSegmentKind = s.iKind; result.iSegmentFlags = s.iFlags;
            if (s.iKind == XUI_DOC_SOURCE_DIRECT) {
                if (s.iTextEnd - s.iTextStart != s.iSourceEnd - s.iSourceStart) return XUI_DOC_ERROR_FORMAT;
                result.iTextEnd = s.iTextStart + t->iSourceEnd - s.iSourceStart;
                result.iTextStart = s.iTextStart + t->iSourceStart - s.iSourceStart;
            } else {
                if (t->iSourceStart != s.iSourceStart) result.iSegmentFlags |= XUI_DOC_SOURCE_PARTIAL_START;
                if (t->iSourceEnd != s.iSourceEnd) result.iSegmentFlags |= XUI_DOC_SOURCE_PARTIAL_END;
            }
        }
        *out = result;
    }
    return XUI_OK;
}
XUI_API int xuiDocumentSourceIndexGetToken(xui_document_source_index index,
    uint64_t ordinal, xui_doc_source_token_t* out)
{
    if (!out || out->iSize != sizeof(*out)) return XUI_ERROR_INVALID_ARGUMENT;
    memset(out, 0, sizeof(*out)); out->iSize = sizeof(*out);
    if (!index) return XUI_ERROR_INVALID_ARGUMENT;
    if (ordinal >= index->info.iTokenCount) return XUI_ERROR_NOT_FOUND;
    return doc_si_decode(index, index->tokens + ordinal, index->tokens[ordinal].iProvenanceIndex, out);
}
XUI_API int xuiDocumentSourceIndexGetBinding(xui_document_source_index index,
    uint64_t ordinal, xui_doc_source_binding_t* out)
{
    const doc_si_binding_value* v; doc_si_token_value token = {0};
    xui_doc_source_binding_t result = {0}; int code;
    uint64_t node, start = DOC_NONE, end = DOC_NONE;
    if (!out || out->iSize != sizeof(*out)) return XUI_ERROR_INVALID_ARGUMENT;
    memset(out, 0, sizeof(*out)); out->iSize = sizeof(*out);
    if (!index) return XUI_ERROR_INVALID_ARGUMENT;
    if (ordinal >= index->info.iBindingCount) return XUI_ERROR_NOT_FOUND;
    v = index->bindings + ordinal;
    result.iSize = sizeof(result); result.iOriginKind = (v->iFlagsOrigin & ~DOC_SI_MAXIMUM_ANCHOR) >> 16;
    result.iOriginIndex = v->iOriginIndex;
    token.iSourceStart = v->iSourceStart; token.iSourceEnd = v->iSourceEnd; token.iRecordIndex = v->iRecordIndex;
    token.iKind = v->iKindDetail & UINT16_MAX; token.iDetail = v->iKindDetail >> 16;
    token.iFlags = v->iFlagsOrigin & UINT16_MAX;
    result.iStartColumn = result.iMarkerColumn = result.iContentColumn = result.iEndColumn = DOC_NONE;
    code = doc_si_decode(index, &token, result.iOriginKind == XUI_DOC_SOURCE_BINDING_PROVENANCE ||
        result.iOriginKind == XUI_DOC_SOURCE_BINDING_DEFINITION_SYNTAX ||
        result.iOriginKind == XUI_DOC_SOURCE_BINDING_DOCUMENT_SYNTAX ?
        v->iOriginIndex : DOC_NONE, &result.tRange);
    if (code != XUI_OK) return code;
    node = result.tRange.iNodeId;
    switch (result.iOriginKind) {
    case XUI_DOC_SOURCE_BINDING_DOCUMENT_SYNTAX:
    case XUI_DOC_SOURCE_BINDING_DEFINITION_SYNTAX:
        code = doc_si_cst_metadata(index, v->iRecordIndex, result.iOriginIndex, &result);
        if (code != XUI_OK) return code;
        break;
    case XUI_DOC_SOURCE_BINDING_LIST_CONTINUATION_INDENT: {
        xui_doc_list_indent_t s = {0}; s.iSize = sizeof(s);
        code = xuiDocumentSnapshotGetListContinuationIndent(index->snapshot, node, result.iOriginIndex, &s);
        if (code != XUI_OK) return code;
        start = s.iSourceStart; end = s.iSourceEnd; result.iMetadata = XUI_DOC_SOURCE_BINDING_COLUMNS;
        result.iStartColumn = s.iIndentStartColumn; result.iContentColumn = s.iContentColumn; result.iEndColumn = s.iIndentEndColumn;
        break;
    }
    case XUI_DOC_SOURCE_BINDING_CODE_INDENT: {
        xui_doc_code_indent_t s = {0}; s.iSize = sizeof(s);
        code = xuiDocumentSnapshotGetCodeIndent(index->snapshot, node, result.iOriginIndex, &s);
        if (code != XUI_OK) return code;
        start = s.iSourceStart; end = s.iSourceEnd;
        result.iMetadata = XUI_DOC_SOURCE_BINDING_COLUMNS; result.iMarkerFlags = s.iFlags;
        result.iStartColumn = s.iIndentStartColumn; result.iEndColumn = s.iIndentEndColumn;
        result.iContentColumn = s.iContentColumn == UINT32_MAX ? DOC_NONE : s.iContentColumn;
        break;
    }
    case XUI_DOC_SOURCE_BINDING_TABLE_TOKEN: {
        xui_doc_table_token_t s = {0}; s.iSize = sizeof(s);
        code = xuiDocumentSnapshotGetTableToken(index->snapshot, node, result.iOriginIndex, &s);
        if (code != XUI_OK) return code;
        start = s.iSourceStart; end = s.iSourceEnd; result.iMetadata = XUI_DOC_SOURCE_BINDING_TABLE;
        result.iMarkerFlags = s.iFlags; result.iRow = s.iRow; result.iOrdinal = s.iOrdinal;
        break;
    }
    case XUI_DOC_SOURCE_BINDING_QUOTE_INDENT:
    case XUI_DOC_SOURCE_BINDING_LIST_FIRST_INDENT: {
        doc_node* n = doc_index_get(index->snapshot->state->index, node);
        doc_node_source_range range;
        if (!n || !n->syntax_aux) return XUI_DOC_ERROR_FORMAT;
        doc_node_source_range_get(index->snapshot->state, n, &range);
        if (result.iOriginKind == XUI_DOC_SOURCE_BINDING_QUOTE_INDENT) {
            doc_quote_indent_relative s;
            if (n->marker_kind != XUI_DOC_BLOCK_SYNTAX_QUOTE_OPEN || n->syntax_aux->size % sizeof(s) ||
                result.iOriginIndex >= n->syntax_aux->size / sizeof(s)) return XUI_DOC_ERROR_FORMAT;
            memcpy(&s, n->syntax_aux->data + result.iOriginIndex * sizeof(s), sizeof(s));
            start = range.syntax_start + s.indent_start; end = range.syntax_start + s.end;
            result.iStartColumn = s.start_column; result.iMarkerColumn = s.marker_column;
            result.iContentColumn = s.content_column; result.iEndColumn = s.end_column;
        } else {
            doc_list_first_indent_relative s;
            if (n->marker_kind != XUI_DOC_BLOCK_SYNTAX_LIST_ITEM || n->syntax_aux->size != sizeof(s) ||
                result.iOriginIndex) return XUI_DOC_ERROR_FORMAT;
            memcpy(&s, n->syntax_aux->data, sizeof(s));
            start = range.syntax_start + s.indent_start; end = range.syntax_start + s.end;
            result.iStartColumn = s.start_column; result.iContentColumn = s.content_column; result.iEndColumn = s.end_column;
        }
        result.iMetadata = XUI_DOC_SOURCE_BINDING_COLUMNS; break;
    }
    default: break;
    }
    if (start != DOC_NONE && (start != v->iSourceStart || end != v->iSourceEnd)) return XUI_DOC_ERROR_FORMAT;
    *out = result; return XUI_OK;
}
static int doc_si_visit(xui_document_source_index index, uint64_t lo, uint64_t hi,
    uint64_t start, uint64_t end, xui_doc_source_binding_proc callback, void* user, uint64_t* count)
{
    uint64_t mid; const doc_si_binding_value* v; int result, hit;
    if (lo == hi) return XUI_OK;
    mid = lo + (hi - lo) / 2; v = index->bindings + mid;
    if (v->maximum < start || (v->maximum == start && !(v->iFlagsOrigin & DOC_SI_MAXIMUM_ANCHOR)) ||
        (start == end ? index->bindings[lo].iSourceStart > start : index->bindings[lo].iSourceStart >= end))
        return XUI_OK;
    result = doc_si_visit(index, lo, mid, start, end, callback, user, count);
    if (result != XUI_OK) return result;
    if (v->iSourceStart == v->iSourceEnd)
        hit = start == end ? v->iSourceStart == start : v->iSourceStart >= start && v->iSourceStart < end;
    else hit = start == end ? v->iSourceStart <= start && start < v->iSourceEnd :
        v->iSourceStart < end && v->iSourceEnd > start;
    if (hit) {
        xui_doc_source_binding_t binding = {0}; binding.iSize = sizeof(binding);
        result = xuiDocumentSourceIndexGetBinding(index, mid, &binding);
        if (result != XUI_OK) return result;
        (*count)++;
        if (callback(user, mid, &binding)) return XUI_DOC_ERROR_CANCELLED;
    }
    return doc_si_visit(index, mid + 1, hi, start, end, callback, user, count);
}
XUI_API int xuiDocumentSourceIndexVisitBindings(xui_document_source_index index,
    uint64_t start, uint64_t end, xui_doc_source_binding_proc onBinding, void* user, uint64_t* visited)
{
    uint64_t count = 0; int result;
    if (visited) *visited = 0;
    if (!index || !onBinding || start > end || end > index->info.iSourceBytes) return XUI_ERROR_INVALID_ARGUMENT;
    xuiDocumentSourceIndexRetain(index);
    result = doc_si_visit(index, 0, index->info.iBindingCount, start, end, onBinding, user, &count);
    if (visited) *visited = count;
    xuiDocumentSourceIndexRelease(index); return result;
}
XUI_API int xuiDocumentSourceIndexFindToken(xui_document_source_index index, uint64_t offset, uint64_t* ordinal)
{
    uint64_t first = 0, end, mid;
    if (!ordinal) return XUI_ERROR_INVALID_ARGUMENT;
    *ordinal = DOC_NONE;
    if (!index || offset > index->info.iSourceBytes) return XUI_ERROR_INVALID_ARGUMENT;
    if (offset == index->info.iSourceBytes) return XUI_ERROR_NOT_FOUND;
    end = index->info.iTokenCount;
    while (first < end) {
        mid = first + (end - first) / 2;
        if (index->tokens[mid].iSourceEnd <= offset) first = mid + 1;
        else end = mid;
    }
    if (first >= index->info.iTokenCount || index->tokens[first].iSourceStart > offset) return XUI_DOC_ERROR_FORMAT;
    *ordinal = first; return XUI_OK;
}
XUI_API int xuiDocumentSourceIndexReadSource(xui_document_source_index index,
    uint64_t offset, void* buffer, uint64_t bytes)
{
    if (!index) return XUI_ERROR_INVALID_ARGUMENT;
    return xuiDocumentSnapshotReadSource(index->snapshot, offset, buffer, bytes);
}

#endif
