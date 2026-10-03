#include "../xui_config.h"
#if XUI_ENABLE_DOCUMENT
#include "xui_document_md4c.h"

/* Route the parser's complete allocation lifetime
 * through the document allocator. TLS isolates independent parser workers;
 * save/restore also makes nested calls safe. */
static _Thread_local doc_allocator* doc_md4c_allocator;
static _Thread_local int doc_md4c_oom;
static _Thread_local doc_md4c_block_source_proc doc_md4c_block_source;
static _Thread_local doc_md4c_block_markers_proc doc_md4c_block_markers;
static _Thread_local doc_md4c_fence_info_proc doc_md4c_fence_info;
static _Thread_local doc_md4c_heading_content_proc doc_md4c_heading_content;
static _Thread_local doc_md4c_break_source_proc doc_md4c_break_source;
static _Thread_local doc_md4c_quote_prefixes_proc doc_md4c_quote_prefixes;
static _Thread_local doc_md4c_list_indents_proc doc_md4c_list_indents;
static _Thread_local doc_md4c_code_indents_proc doc_md4c_code_indents;
static _Thread_local doc_md4c_table_token_proc doc_md4c_table_token;
static _Thread_local doc_md4c_reference_source_proc doc_md4c_reference_source;
static _Thread_local doc_md4c_reference_values_proc doc_md4c_reference_values;
static _Thread_local doc_md4c_footnote_source_proc doc_md4c_footnote_source;
static _Thread_local doc_md4c_candidate_source_proc doc_md4c_candidate_source;
static _Thread_local doc_md4c_span_source_proc doc_md4c_span_source;
static _Thread_local const atomic_int* doc_md4c_cancellation;
static _Thread_local int* doc_md4c_footnotes;
static int doc_md4c_note_footnote(MD_OFFSET beg, MD_OFFSET end, MD_OFFSET lb, MD_OFFSET le, void* user)
{
    if (doc_md4c_footnotes) *doc_md4c_footnotes = 1;
    return doc_md4c_footnote_source ? doc_md4c_footnote_source(beg, end, lb, le, user) : 0;
}
static int doc_md4c_cancelled(void)
{
    return doc_md4c_cancellation ? atomic_load(doc_md4c_cancellation) : 0;
}
static void* doc_md4c_malloc(size_t bytes)
{
    if (doc_md4c_cancelled()) return NULL;
    void* p = doc_alloc(doc_md4c_allocator, bytes); if (!p) doc_md4c_oom = 1; return p;
}
static void* doc_md4c_realloc(void* previous, size_t bytes)
{
    if (doc_md4c_cancelled()) return NULL;
    void* p = doc_realloc(doc_md4c_allocator, previous, bytes); if (!p && bytes) doc_md4c_oom = 1; return p;
}
#define malloc doc_md4c_malloc
#define realloc doc_md4c_realloc
#define free doc_free
#define md_parse doc_md4c_upstream_parse
#define MD_XUI_CANCEL() doc_md4c_cancelled()
#define MD_XUI_FOOTNOTE_DEFINED(beg, end, lb, le) doc_md4c_note_footnote(beg, end, lb, le, ctx->userdata)
#define MD_XUI_REFERENCE_CANDIDATE(kind, beg, end, lb, le) \
    (doc_md4c_candidate_source ? doc_md4c_candidate_source(kind, beg, end, lb, le, ctx->userdata) : 0)
#define MD_XUI_SOURCE_BLOCK(type, beg, end, enter) do { if (doc_md4c_block_source) doc_md4c_block_source(type, beg, end, enter, ctx->userdata); } while (0)
#define MD_XUI_SOURCE_MARKERS(kind, a, b, c, d, tail, list_gap, task_gap) do { \
    if (doc_md4c_block_markers) doc_md4c_block_markers(kind, a, b, c, d, tail, list_gap, task_gap, ctx->userdata); \
} while (0)
#define MD_XUI_SOURCE_FENCE_INFO(info_beg, info_end, lang_end) \
    (doc_md4c_fence_info ? doc_md4c_fence_info(info_beg, info_end, lang_end, ctx->userdata) : 0)
#define MD_XUI_SOURCE_HEADING_CONTENT(beg, end) \
    (doc_md4c_heading_content ? doc_md4c_heading_content(beg, end, ctx->userdata) : 0)
#define MD_XUI_SOURCE_BREAK(kind, trailing, marker_beg, marker_end, newline_beg, newline_end) \
    (doc_md4c_break_source ? doc_md4c_break_source(kind, trailing, marker_beg, marker_end, \
        newline_beg, newline_end, ctx->userdata) : 0)
#define MD_XUI_SOURCE_QUOTE_PREFIXES(offsets, count) \
    (doc_md4c_quote_prefixes ? doc_md4c_quote_prefixes(offsets, count, ctx->userdata) : 0)
#define MD_XUI_SOURCE_LIST_INDENTS(pairs, count) \
    (doc_md4c_list_indents ? doc_md4c_list_indents(pairs, count, ctx->userdata) : 0)
#define MD_XUI_SOURCE_CODE_INDENTS(records, count) \
    (doc_md4c_code_indents ? doc_md4c_code_indents(records, count, ctx->userdata) : 0)
#define MD_XUI_SOURCE_TABLE_TOKEN(kind, row, ordinal, beg, end, flags) \
    (doc_md4c_table_token ? doc_md4c_table_token(kind, row, ordinal, beg, end, flags, ctx->userdata) : 0)
#define MD_XUI_SOURCE_REFERENCE(beg, end, lb, le, db, de, tb, te, has_title) \
    (doc_md4c_reference_source ? doc_md4c_reference_source(beg, end, lb, le, db, de, tb, te, has_title, ctx->userdata) : 0)
#define MD_XUI_REFERENCE_VALUES(label, label_size, dest, dest_size, title, title_size, has_title) \
    (doc_md4c_reference_values ? doc_md4c_reference_values(label, label_size, dest, dest_size, title, title_size, has_title, ctx->userdata) : 0)
#define MD_XUI_SOURCE_SPAN(type, beg, end, enter, db, de) \
    (doc_md4c_span_source ? doc_md4c_span_source(type, beg, end, enter, db, de, ctx->userdata) : 0)
#include "../lib/md4c/md4c.c"
#undef MD_XUI_CANCEL
#undef MD_XUI_FOOTNOTE_DEFINED
#undef MD_XUI_REFERENCE_CANDIDATE
#undef MD_XUI_SOURCE_SPAN
#undef MD_XUI_SOURCE_REFERENCE
#undef MD_XUI_REFERENCE_VALUES
#undef MD_XUI_SOURCE_BLOCK
#undef MD_XUI_SOURCE_MARKERS
#undef MD_XUI_SOURCE_FENCE_INFO
#undef MD_XUI_SOURCE_HEADING_CONTENT
#undef MD_XUI_SOURCE_BREAK
#undef MD_XUI_SOURCE_QUOTE_PREFIXES
#undef MD_XUI_SOURCE_LIST_INDENTS
#undef MD_XUI_SOURCE_CODE_INDENTS
#undef MD_XUI_SOURCE_TABLE_TOKEN
#undef md_parse
#undef free
#undef realloc
#undef malloc

int doc_md4c_label_equal(const char* a, const char* b)
{
    size_t a_bytes, b_bytes;
    if (!a || !b) return 0;
    a_bytes = strlen(a); b_bytes = strlen(b);
    if (a_bytes > UINT_MAX || b_bytes > UINT_MAX) return 0;
    return md_label_cmp(a, (MD_SIZE)a_bytes, b, (MD_SIZE)b_bytes) == 0;
}
int doc_md4c_footnote_label_valid(const char* label)
{
    MD_CTX ctx = {0};
    CHAR framed[78];
    OFF end = 0;
    size_t bytes;
    if (!label) return 0;
    bytes = strlen(label);
    if (!bytes || bytes > 76) return 0;
    memcpy(framed, label, bytes);
    framed[bytes] = _T(']');
    ctx.text = framed; ctx.size = (SZ)(bytes + 1);
    return md_is_footnote_label(&ctx, 0, &end) && end == bytes;
}

int doc_md4c_parse(doc_allocator* allocator, const char* text, MD_SIZE size, const MD_PARSER* parser,
    doc_md4c_block_source_proc block_source, doc_md4c_block_markers_proc block_markers,
    doc_md4c_fence_info_proc fence_info, doc_md4c_heading_content_proc heading_content,
    doc_md4c_break_source_proc break_source,
    doc_md4c_quote_prefixes_proc quote_prefixes, doc_md4c_list_indents_proc list_indents,
    doc_md4c_code_indents_proc code_indents,
    doc_md4c_table_token_proc table_token,
    doc_md4c_reference_source_proc reference_source,
    doc_md4c_reference_values_proc reference_values,
    doc_md4c_footnote_source_proc footnote_source, doc_md4c_candidate_source_proc candidate_source,
    doc_md4c_span_source_proc span_source,
    const atomic_int* cancellation, int* footnotes, void* user)
{
    doc_allocator* previous = doc_md4c_allocator;
    doc_md4c_block_source_proc previous_source = doc_md4c_block_source;
    doc_md4c_block_markers_proc previous_markers = doc_md4c_block_markers;
    doc_md4c_fence_info_proc previous_fence_info = doc_md4c_fence_info;
    doc_md4c_heading_content_proc previous_heading_content = doc_md4c_heading_content;
    doc_md4c_break_source_proc previous_break_source = doc_md4c_break_source;
    doc_md4c_quote_prefixes_proc previous_quote_prefixes = doc_md4c_quote_prefixes;
    doc_md4c_list_indents_proc previous_list_indents = doc_md4c_list_indents;
    doc_md4c_code_indents_proc previous_code_indents = doc_md4c_code_indents;
    doc_md4c_table_token_proc previous_table_token = doc_md4c_table_token;
    doc_md4c_reference_source_proc previous_reference = doc_md4c_reference_source;
    doc_md4c_reference_values_proc previous_values = doc_md4c_reference_values;
    doc_md4c_footnote_source_proc previous_footnote_source = doc_md4c_footnote_source;
    doc_md4c_candidate_source_proc previous_candidate_source = doc_md4c_candidate_source;
    doc_md4c_span_source_proc previous_span = doc_md4c_span_source;
    const atomic_int* previous_cancellation = doc_md4c_cancellation;
    int* previous_footnotes = doc_md4c_footnotes;
    int previous_oom = doc_md4c_oom, result;
    doc_md4c_allocator = allocator; doc_md4c_oom = 0;
    doc_md4c_block_source = block_source;
    doc_md4c_block_markers = block_markers;
    doc_md4c_fence_info = fence_info;
    doc_md4c_heading_content = heading_content;
    doc_md4c_break_source = break_source;
    doc_md4c_quote_prefixes = quote_prefixes;
    doc_md4c_list_indents = list_indents;
    doc_md4c_code_indents = code_indents;
    doc_md4c_table_token = table_token;
    doc_md4c_reference_source = reference_source;
    doc_md4c_reference_values = reference_values;
    doc_md4c_footnote_source = footnote_source;
    doc_md4c_candidate_source = candidate_source;
    doc_md4c_span_source = span_source;
    doc_md4c_cancellation = cancellation;
    doc_md4c_footnotes = footnotes;
    result = doc_md4c_upstream_parse(text, size, parser, user);
    if (doc_md4c_oom) result = XUI_ERROR_OUT_OF_MEMORY;
    if (doc_md4c_cancelled()) result = doc_md4c_cancelled();
    doc_md4c_allocator = previous; doc_md4c_oom = previous_oom;
    doc_md4c_block_source = previous_source;
    doc_md4c_block_markers = previous_markers;
    doc_md4c_fence_info = previous_fence_info;
    doc_md4c_heading_content = previous_heading_content;
    doc_md4c_break_source = previous_break_source;
    doc_md4c_quote_prefixes = previous_quote_prefixes;
    doc_md4c_list_indents = previous_list_indents;
    doc_md4c_code_indents = previous_code_indents;
    doc_md4c_table_token = previous_table_token;
    doc_md4c_reference_source = previous_reference;
    doc_md4c_reference_values = previous_values;
    doc_md4c_footnote_source = previous_footnote_source;
    doc_md4c_candidate_source = previous_candidate_source;
    doc_md4c_span_source = previous_span;
    doc_md4c_cancellation = previous_cancellation;
    doc_md4c_footnotes = previous_footnotes;
    return result;
}

unsigned doc_md4c_dialect_flags(uint32_t dialect)
{
    unsigned flags = dialect == XUI_MD_COMMONMARK ? 0 : MD_FLAG_TABLES | MD_FLAG_STRIKETHROUGH |
        MD_FLAG_TASKLISTS | MD_FLAG_PERMISSIVEAUTOLINKS;
    if (dialect == XUI_MD_EXTENDED) flags |= MD_FLAG_LATEXMATHSPANS | MD_FLAG_FOOTNOTES |
        MD_FLAG_ADMONITIONS | MD_FLAG_SUPERSCRIPTS | MD_FLAG_SUBSCRIPTS | MD_FLAG_HIGHLIGHT;
    return flags;
}
typedef struct doc_md4c_values {
    doc_allocator* allocator;
    char* bytes;
    size_t size, capacity, offset;
    int comparing, equal, error;
} doc_md4c_values;
static int doc_md4c_values_bytes(doc_md4c_values* values, const void* data, size_t bytes)
{
    size_t i;
    if (values->comparing) {
        if (!values->equal) return 0;
        if (values->offset > values->size || bytes > values->size - values->offset) { values->equal = 0; return 0; }
    } else {
        size_t required, capacity; char* allocation;
        if (bytes > SIZE_MAX - values->size) { values->error = XUI_ERROR_OUT_OF_MEMORY; return -1; }
        required = values->size + bytes; capacity = values->capacity;
        if (required > capacity) {
            if (!capacity) capacity = 256;
            while (capacity < required) {
                if (capacity > SIZE_MAX / 2) { capacity = required; break; }
                capacity *= 2;
            }
            allocation = doc_realloc(values->allocator, values->bytes, capacity);
            if (!allocation) { values->error = XUI_ERROR_OUT_OF_MEMORY; return -1; }
            values->bytes = allocation; values->capacity = capacity;
        }
    }
    for (i = 0; i < bytes; ) {
        size_t chunk = bytes - i > 16384 ? 16384 : bytes - i;
        if (doc_md4c_cancelled()) { values->error = doc_md4c_cancelled(); return -1; }
        if (values->comparing) {
            if (memcmp(values->bytes + values->offset + i, (const char*)data + i, chunk)) { values->equal = 0; return 0; }
        } else memcpy(values->bytes + values->size + i, (const char*)data + i, chunk);
        i += chunk;
    }
    if (values->comparing) values->offset += bytes; else values->size += bytes;
    return 0;
}
static int doc_md4c_values_reference(const char* label, MD_SIZE label_bytes,
    const char* destination, MD_SIZE destination_bytes, const char* title, MD_SIZE title_bytes, int has_title, void* user)
{
    doc_md4c_values* values = user;
    uint64_t header[4] = {label_bytes, destination_bytes, title_bytes, !!has_title};
    if (doc_md4c_values_bytes(values, header, sizeof(header)) ||
        doc_md4c_values_bytes(values, label, label_bytes) ||
        doc_md4c_values_bytes(values, destination, destination_bytes) ||
        doc_md4c_values_bytes(values, title, title_bytes)) return -1;
    return 0;
}
static int doc_md4c_values_block(MD_BLOCKTYPE type, void* detail, void* user)
{ (void)type; (void)detail; (void)user; return 0; }
static int doc_md4c_values_span(MD_SPANTYPE type, void* detail, void* user)
{ (void)type; (void)detail; (void)user; return 0; }
static int doc_md4c_values_text(MD_TEXTTYPE type, const MD_CHAR* text, MD_SIZE size, void* user)
{ (void)type; (void)text; (void)size; (void)user; return 0; }
int doc_md4c_reference_values_equal(doc_allocator* allocator, const char* before, MD_SIZE before_size,
    const char* after, MD_SIZE after_size, unsigned flags, const atomic_int* cancellation, int* equal)
{
    MD_PARSER parser = {0}; doc_md4c_values values = {0}; int result, footnotes;
    values.allocator = allocator; values.equal = 1; *equal = 0;
    parser.flags = flags; parser.enter_block = parser.leave_block = doc_md4c_values_block;
    parser.enter_span = parser.leave_span = doc_md4c_values_span; parser.text = doc_md4c_values_text;
    atomic_fetch_add(&allocator->markdown_parses, 1);
    atomic_fetch_add(&allocator->markdown_parsed_bytes, before_size);
    result = doc_md4c_parse(allocator, before, before_size, &parser, NULL, NULL, NULL, NULL, NULL,
        NULL, NULL, NULL, NULL, NULL, doc_md4c_values_reference, NULL, NULL, NULL, cancellation, &footnotes, &values);
    if (!result) {
        values.comparing = 1;
        atomic_fetch_add(&allocator->markdown_parses, 1);
        atomic_fetch_add(&allocator->markdown_parsed_bytes, after_size);
        result = doc_md4c_parse(allocator, after, after_size, &parser, NULL, NULL, NULL, NULL, NULL,
            NULL, NULL, NULL, NULL, NULL, doc_md4c_values_reference, NULL, NULL, NULL, cancellation, &footnotes, &values);
    }
    if (values.error) result = values.error;
    if (!result) *equal = values.equal && values.offset == values.size;
    doc_free(values.bytes);
    return result == -1 ? XUI_DOC_ERROR_FORMAT : result;
}

#endif
