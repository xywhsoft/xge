#include "xui_document_internal.h"
#include "xui_document_md4c.h"
#include "../lib/md4c/entity.h"
#include <ctype.h>

typedef struct doc_md_block {
    MD_BLOCKTYPE type;
    uint64_t id, paragraph;
} doc_md_block;
typedef struct doc_md_span {
    uint32_t marks;
    uint64_t object;
    char *href, *title;
    int owns;
} doc_md_span;
typedef struct doc_md_context {
    struct xui_doc_transaction_t build;
    const char* source;
    uint64_t size, cursor;
    doc_md_block blocks[DOC_MAX_DEPTH];
    doc_md_span spans[DOC_MAX_DEPTH];
    unsigned block_count, span_count;
    int result;
} doc_md_context;

static unsigned doc_md_utf8(uint32_t c, char out[4])
{
    if (c == 0 || c > 0x10ffff || (c >= 0xd800 && c <= 0xdfff)) c = 0xfffd;
    if (c < 0x80) { out[0] = (char)c; return 1; }
    if (c < 0x800) { out[0] = (char)(0xc0 | (c >> 6)); out[1] = (char)(0x80 | (c & 63)); return 2; }
    if (c < 0x10000) { out[0] = (char)(0xe0 | (c >> 12)); out[1] = (char)(0x80 | ((c >> 6) & 63)); out[2] = (char)(0x80 | (c & 63)); return 3; }
    out[0] = (char)(0xf0 | (c >> 18)); out[1] = (char)(0x80 | ((c >> 12) & 63));
    out[2] = (char)(0x80 | ((c >> 6) & 63)); out[3] = (char)(0x80 | (c & 63)); return 4;
}
static unsigned doc_md_entity(const char* s, uint64_t n, char out[8])
{
    const ENTITY* e;
    unsigned count;
    if (n > 3 && s[0] == '&' && s[1] == '#') {
        uint64_t i = 2;
        uint32_t c = 0;
        unsigned base = 10;
        if (s[i] == 'x' || s[i] == 'X') { base = 16; i++; }
        for (; i < n && s[i] != ';'; i++) {
            unsigned digit;
            if (s[i] >= '0' && s[i] <= '9') digit = (unsigned)(s[i] - '0');
            else if (s[i] >= 'a' && s[i] <= 'f') digit = (unsigned)(s[i] - 'a' + 10);
            else if (s[i] >= 'A' && s[i] <= 'F') digit = (unsigned)(s[i] - 'A' + 10);
            else return 0;
            if (digit >= base) return 0;
            if (c <= 0x110000) c = c * base + digit;
        }
        return doc_md_utf8(c, out);
    }
    e = entity_lookup(s, (size_t)n);
    if (!e) return 0;
    count = doc_md_utf8(e->codepoints[0], out);
    if (e->codepoints[1]) count += doc_md_utf8(e->codepoints[1], out + count);
    return count;
}
static char* doc_md_attribute(doc_md_context* c, const MD_ATTRIBUTE* attr)
{
    char* out;
    uint64_t used = 0;
    unsigned i = 0;
    if (!attr || !attr->size) return NULL;
    out = doc_alloc(c->build.document->allocator, (size_t)attr->size * 3 + 1);
    if (!out) { c->result = XUI_ERROR_OUT_OF_MEMORY; return NULL; }
    if (!attr->substr_types || !attr->substr_offsets) {
        memcpy(out, attr->text, attr->size); out[attr->size] = 0; return out;
    }
    while (attr->substr_offsets[i] < attr->size) {
        uint64_t start = attr->substr_offsets[i], end = attr->substr_offsets[i + 1];
        char decoded[8]; unsigned n = 0;
        if (attr->substr_types[i] == MD_TEXT_ENTITY) n = doc_md_entity(attr->text + start, end - start, decoded);
        else if (attr->substr_types[i] == MD_TEXT_NULLCHAR) n = doc_md_utf8(0xfffd, decoded);
        if (n) { memcpy(out + used, decoded, n); used += n; }
        else { memcpy(out + used, attr->text + start, (size_t)(end - start)); used += end - start; }
        i++;
    }
    out[used] = 0; return out;
}
static uint64_t doc_md_parent(doc_md_context* c)
{
    return c->blocks[c->block_count - 1].id;
}
static int doc_md_add(doc_md_context* c, uint64_t parent, const xui_doc_node_desc_t* d, uint64_t* id)
{
    int r = doc_txn_insert(&c->build, parent, DOC_NONE, d, id);
    if (r != XUI_OK) c->result = r;
    return r;
}
static uint64_t doc_md_inline_parent(doc_md_context* c)
{
    doc_md_block* block = &c->blocks[c->block_count - 1];
    doc_node* node = doc_index_get(c->build.draft->index, block->id);
    xui_doc_node_desc_t desc = {0};
    if (node->kind == XUI_DOC_PARAGRAPH || node->kind == XUI_DOC_HEADING) return node->id;
    if (doc_text_kind(node->kind)) return node->id;
    if (!block->paragraph) {
        desc.iSize = sizeof(desc); desc.iKind = XUI_DOC_PARAGRAPH;
        if (doc_md_add(c, node->id, &desc, &block->paragraph) != XUI_OK) return 0;
    }
    return block->paragraph;
}
static void doc_md_range(doc_md_context* c, uint64_t id, uint64_t start, uint64_t end, int exact)
{
    doc_node* p = doc_index_get(c->build.draft->index, id);
    if (start == DOC_NONE || !p) return;
    if (p->source_start == DOC_NONE) { p->source_start = start; p->source_end = end; p->source_exact = exact; }
    else {
        if (p->source_end != start) p->source_exact = 0;
        if (start < p->source_start) p->source_start = start;
        if (end > p->source_end) p->source_end = end;
        p->source_exact = p->source_exact && exact;
    }
}
static int doc_md_enter_block(MD_BLOCKTYPE type, void* detail, void* user)
{
    doc_md_context* c = user;
    xui_doc_node_desc_t d = {0};
    uint64_t id = DOC_ROOT;
    char* info = NULL;
    int result = XUI_OK;
    if (c->block_count >= DOC_MAX_DEPTH) return c->result = XUI_DOC_ERROR_LIMIT;
    d.iSize = sizeof(d);
    switch (type) {
    case MD_BLOCK_DOC: break;
    case MD_BLOCK_QUOTE: d.iKind = XUI_DOC_QUOTE; break;
    case MD_BLOCK_UL: d.iKind = XUI_DOC_LIST; if (((MD_BLOCK_UL_DETAIL*)detail)->is_tight) d.tAttributes.iFlags |= XUI_DOC_TIGHT; break;
    case MD_BLOCK_OL:
        d.iKind = XUI_DOC_LIST; d.tAttributes.iFlags = XUI_DOC_ORDERED;
        d.tAttributes.iListStart = ((MD_BLOCK_OL_DETAIL*)detail)->start;
        if (((MD_BLOCK_OL_DETAIL*)detail)->is_tight) d.tAttributes.iFlags |= XUI_DOC_TIGHT;
        break;
    case MD_BLOCK_LI:
        d.iKind = XUI_DOC_LIST_ITEM;
        if (((MD_BLOCK_LI_DETAIL*)detail)->is_task) {
            d.tAttributes.iFlags |= XUI_DOC_TASK;
            if (((MD_BLOCK_LI_DETAIL*)detail)->task_mark != ' ') d.tAttributes.iFlags |= XUI_DOC_CHECKED;
        }
        break;
    case MD_BLOCK_HR: d.iKind = XUI_DOC_RULE; break;
    case MD_BLOCK_H: d.iKind = XUI_DOC_HEADING; d.tAttributes.iHeadingLevel = ((MD_BLOCK_H_DETAIL*)detail)->level; break;
    case MD_BLOCK_P: d.iKind = XUI_DOC_PARAGRAPH; break;
    case MD_BLOCK_CODE:
        info = doc_md_attribute(c, &((MD_BLOCK_CODE_DETAIL*)detail)->lang);
        d.sInfo = info; d.iKind = c->build.draft->dialect == XUI_MD_EXTENDED && info && strcmp(info, "mermaid") == 0 ? XUI_DOC_DIAGRAM : XUI_DOC_CODE_BLOCK;
        break;
    case MD_BLOCK_HTML: d.iKind = XUI_DOC_HTML; d.tAttributes.iFlags = XUI_DOC_BLOCK; break;
    case MD_BLOCK_TABLE: d.iKind = XUI_DOC_TABLE; break;
    case MD_BLOCK_TR: d.iKind = XUI_DOC_ROW; break;
    case MD_BLOCK_THEAD: case MD_BLOCK_TBODY: case MD_BLOCK_FOOTNOTE_DEF_SECTION:
    case MD_BLOCK_BLANK: id = doc_md_parent(c); break;
    case MD_BLOCK_TH: case MD_BLOCK_TD:
        d.iKind = XUI_DOC_CELL;
        if (type == MD_BLOCK_TH) d.tAttributes.iFlags |= XUI_DOC_HEADER;
        switch (((MD_BLOCK_TD_DETAIL*)detail)->align) {
        case MD_ALIGN_CENTER: d.tAttributes.iAlignment = 1; break;
        case MD_ALIGN_RIGHT: d.tAttributes.iAlignment = 2; break;
        default: break;
        }
        break;
    case MD_BLOCK_FOOTNOTE_DEF:
        d.iKind = XUI_DOC_FOOTNOTE;
        info = doc_md_attribute(c, &((MD_BLOCK_FOOTNOTE_DEF_DETAIL*)detail)->label); d.sInfo = info;
        break;
    case MD_BLOCK_ADMONITION:
        d.iKind = XUI_DOC_QUOTE;
        info = doc_md_attribute(c, &((MD_BLOCK_ADMONITION_DETAIL*)detail)->type); d.sInfo = info;
        break;
    default: c->result = XUI_ERROR_UNSUPPORTED; break;
    }
    if (c->result == XUI_OK && d.iKind) result = doc_md_add(c, doc_md_parent(c), &d, &id);
    if (info) doc_free(info);
    if (c->result != XUI_OK || result != XUI_OK) return c->result;
    c->blocks[c->block_count].type = type;
    c->blocks[c->block_count].id = id;
    c->blocks[c->block_count].paragraph = 0;
    c->block_count++;
    return 0;
}
static int doc_md_leave_block(MD_BLOCKTYPE type, void* detail, void* user)
{
    doc_md_context* c = user;
    doc_md_block frame;
    doc_node* p;
    (void)type; (void)detail;
    if (!c->block_count) return c->result = XUI_DOC_ERROR_SCHEMA;
    frame = c->blocks[--c->block_count];
    if (frame.paragraph && frame.paragraph != frame.id) {
        p = doc_index_get(c->build.draft->index, frame.paragraph);
        doc_md_range(c, frame.id, p->source_start, p->source_end, 0);
    }
    p = doc_index_get(c->build.draft->index, frame.id);
    if (c->block_count && frame.id != doc_md_parent(c)) doc_md_range(c, doc_md_parent(c), p->source_start, p->source_end, 0);
    return 0;
}
static int doc_md_enter_span(MD_SPANTYPE type, void* detail, void* user)
{
    doc_md_context* c = user;
    doc_md_span* span;
    xui_doc_node_desc_t d = {0};
    uint64_t parent;
    if (c->span_count + 1 >= DOC_MAX_DEPTH) return c->result = XUI_DOC_ERROR_LIMIT;
    c->spans[c->span_count + 1] = c->spans[c->span_count];
    span = &c->spans[++c->span_count]; span->owns = 0;
    d.iSize = sizeof(d);
    switch (type) {
    case MD_SPAN_EM: span->marks |= XUI_DOC_ITALIC; break;
    case MD_SPAN_STRONG: span->marks |= XUI_DOC_BOLD; break;
    case MD_SPAN_DEL: span->marks |= XUI_DOC_STRIKE; break;
    case MD_SPAN_U: case MD_SPAN_INS: span->marks |= XUI_DOC_UNDERLINE; break;
    case MD_SPAN_CODE: span->marks |= XUI_DOC_CODE; break;
    case MD_SPAN_MARK: span->marks |= XUI_DOC_HIGHLIGHT; break;
    case MD_SPAN_SUBSCRIPT: span->marks |= XUI_DOC_SUBSCRIPT; break;
    case MD_SPAN_SUPERSCRIPT: span->marks |= XUI_DOC_SUPERSCRIPT; break;
    case MD_SPAN_A:
        span->marks |= XUI_DOC_LINK; span->owns = 1;
        span->href = doc_md_attribute(c, &((MD_SPAN_A_DETAIL*)detail)->href);
        span->title = doc_md_attribute(c, &((MD_SPAN_A_DETAIL*)detail)->title);
        break;
    case MD_SPAN_IMG:
        d.iKind = XUI_DOC_IMAGE; span->owns = 1;
        span->href = doc_md_attribute(c, &((MD_SPAN_IMG_DETAIL*)detail)->src);
        span->title = doc_md_attribute(c, &((MD_SPAN_IMG_DETAIL*)detail)->title);
        d.sResource = span->href; d.sTitle = span->title;
        break;
    case MD_SPAN_LATEXMATH: case MD_SPAN_LATEXMATH_DISPLAY:
        d.iKind = XUI_DOC_MATH;
        if (type == MD_SPAN_LATEXMATH_DISPLAY) d.tAttributes.iFlags = XUI_DOC_BLOCK;
        break;
    case MD_SPAN_FOOTNOTE_REF:
        d.iKind = XUI_DOC_FOOTNOTE_REF; span->owns = 1;
        span->href = doc_md_attribute(c, &((MD_SPAN_FOOTNOTE_REF_DETAIL*)detail)->label);
        span->title = NULL; d.sInfo = span->href;
        d.sText = span->href; d.iTextBytes = span->href ? strlen(span->href) : 0;
        break;
    default: return c->result = XUI_ERROR_UNSUPPORTED;
    }
    if (c->result != XUI_OK) return c->result;
    if (d.iKind) {
        parent = doc_md_inline_parent(c);
        if (!parent) return c->result;
        if (doc_md_add(c, parent, &d, &span->object) != XUI_OK) return c->result;
    }
    return 0;
}
static int doc_md_leave_span(MD_SPANTYPE type, void* detail, void* user)
{
    doc_md_context* c = user;
    doc_md_span* span;
    (void)type; (void)detail;
    if (!c->span_count) return c->result = XUI_DOC_ERROR_SCHEMA;
    span = &c->spans[c->span_count--];
    if (span->owns) { doc_free(span->href); doc_free(span->title); }
    return 0;
}
static int doc_md_text(MD_TEXTTYPE type, const MD_CHAR* text, MD_SIZE size, void* user)
{
    doc_md_context* c = user;
    doc_md_span* span = &c->spans[c->span_count];
    xui_doc_node_desc_t d = {0};
    uint64_t id = span->object, parent, start = DOC_NONE, end = DOC_NONE;
    uint64_t bytes = size;
    char decoded[8];
    unsigned n;
    int exact = 1, result;
    uintptr_t ptr = (uintptr_t)text, base = (uintptr_t)c->source;
    if (ptr >= base && ptr - base <= c->size && size <= c->size - (ptr - base)) {
        start = ptr - base; end = start + size;
        if (start && type == MD_TEXT_NORMAL && size && ispunct((unsigned char)text[0])) {
            uint64_t p = start;
            while (p && c->source[p - 1] == '\\') p--;
            if ((start - p) & 1) { start--; exact = 0; }
        }
        c->cursor = end;
    } else {
        exact = 0;
        if ((type == MD_TEXT_CODE || type == MD_TEXT_BR || type == MD_TEXT_SOFTBR) && size == 1 && text[0] == '\n') {
            start = c->cursor;
            while (c->cursor < c->size && c->source[c->cursor] != '\r' && c->source[c->cursor] != '\n') c->cursor++;
            if (c->cursor < c->size && c->source[c->cursor] == '\r') c->cursor++;
            if (c->cursor < c->size && c->source[c->cursor] == '\n') c->cursor++;
            end = c->cursor;
        }
    }
    if (type == MD_TEXT_ENTITY) {
        n = doc_md_entity(text, size, decoded);
        if (n) { text = decoded; bytes = n; exact = 0; }
    } else if (type == MD_TEXT_NULLCHAR) {
        bytes = doc_md_utf8(0xfffd, decoded); text = decoded; exact = 0;
    }
    parent = doc_md_inline_parent(c);
    if (!parent) return c->result;
    if (!id) {
        doc_node* p = doc_index_get(c->build.draft->index, parent);
        if (doc_text_kind(p->kind)) id = parent;
    }
    if (id) {
        doc_node* p = doc_index_get(c->build.draft->index, id);
        uint64_t len = doc_seq_size(p->text);
        result = doc_txn_text(&c->build, id, len, len, text, bytes);
        if (result != XUI_OK) return c->result = result;
    } else {
        d.iSize = sizeof(d); d.iKind = XUI_DOC_TEXT;
        if (type == MD_TEXT_BR) d.iKind = XUI_DOC_HARD_BREAK;
        else if (type == MD_TEXT_SOFTBR) d.iKind = XUI_DOC_SOFT_BREAK;
        else if (type == MD_TEXT_HTML) d.iKind = XUI_DOC_HTML;
        if (doc_text_kind(d.iKind)) { d.sText = text; d.iTextBytes = bytes; }
        d.tAttributes.iMarks = span->marks;
        if (span->marks & XUI_DOC_LINK) { d.sResource = span->href; d.sTitle = span->title; }
        if (doc_md_add(c, parent, &d, &id) != XUI_OK) return c->result;
    }
    doc_md_range(c, id, start, end, exact);
    if (id != parent) doc_md_range(c, parent, start, end, 0);
    return 0;
}
static uint64_t doc_md_front_matter(doc_md_context* c, uint64_t offset)
{
    uint64_t first_end = offset, line, end;
    xui_doc_node_desc_t d = {0};
    uint64_t id;
    while (first_end < c->size && c->source[first_end] != '\n') first_end++;
    end = first_end;
    if (end > offset && c->source[end - 1] == '\r') end--;
    if (end - offset != 3 || memcmp(c->source + offset, "---", 3) != 0 || first_end == c->size) return offset;
    line = first_end + 1;
    while (line < c->size) {
        end = line;
        while (end < c->size && c->source[end] != '\n') end++;
        { uint64_t trim = end; if (trim > line && c->source[trim - 1] == '\r') trim--;
          if (trim - line == 3 && (memcmp(c->source + line, "---", 3) == 0 || memcmp(c->source + line, "...", 3) == 0)) {
              d.iSize = sizeof(d); d.iKind = XUI_DOC_FRONT_MATTER;
              d.sText = c->source + first_end + 1; d.iTextBytes = line - first_end - 1;
              if (doc_md_add(c, DOC_ROOT, &d, &id) != XUI_OK) return offset;
              doc_md_range(c, id, first_end + 1, line, 1);
              return end < c->size ? end + 1 : end;
          }
        }
        line = end < c->size ? end + 1 : end;
    }
    return offset;
}
int doc_markdown_parse(xui_document_transaction t)
{
    doc_md_context c;
    MD_PARSER parser;
    doc_state* state;
    char* source;
    uint64_t size = doc_seq_size(t->draft->source), offset = 0;
    unsigned i;
    int result;
    if (size > UINT_MAX) return doc_txn_fail(t, XUI_DOC_ERROR_LIMIT);
    atomic_fetch_add(&t->document->allocator->markdown_parses, 1);
    atomic_fetch_add(&t->document->allocator->markdown_parsed_bytes, size);
    source = doc_seq_string(t->document->allocator, t->draft->source);
    if (!source) return doc_txn_fail(t, XUI_ERROR_OUT_OF_MEMORY);
    state = doc_state_new(t->document->allocator, XUI_DOCUMENT_MARKDOWN);
    if (!state) { doc_free(source); return doc_txn_fail(t, XUI_ERROR_OUT_OF_MEMORY); }
    state->source = t->draft->source; doc_seq_retain(state->source);
    state->dialect = t->draft->dialect;
    memset(&c, 0, sizeof(c)); c.source = source; c.size = size;
    c.build.document = t->document; c.build.draft = state; c.build.domain = XUI_DOC_SEMANTIC; c.build.parsing = 1;
    if (size >= 3 && memcmp(source, "\xef\xbb\xbf", 3) == 0) offset = 3;
    if (state->dialect == XUI_MD_EXTENDED) offset = doc_md_front_matter(&c, offset);
    c.cursor = offset;
    memset(&parser, 0, sizeof(parser));
    parser.flags = state->dialect == XUI_MD_COMMONMARK ? 0 : MD_FLAG_TABLES | MD_FLAG_STRIKETHROUGH | MD_FLAG_TASKLISTS | MD_FLAG_PERMISSIVEAUTOLINKS;
    if (state->dialect == XUI_MD_EXTENDED) parser.flags |= MD_FLAG_LATEXMATHSPANS | MD_FLAG_FOOTNOTES | MD_FLAG_ADMONITIONS;
    parser.enter_block = doc_md_enter_block; parser.leave_block = doc_md_leave_block;
    parser.enter_span = doc_md_enter_span; parser.leave_span = doc_md_leave_span; parser.text = doc_md_text;
    result = c.result ? c.result : doc_md4c_parse(t->document->allocator, source + offset, (MD_SIZE)(size - offset), &parser, &c);
    if (result || c.result || c.build.error) result = c.result ? c.result : (c.build.error ? c.build.error : (result == XUI_ERROR_OUT_OF_MEMORY ? result : XUI_DOC_ERROR_FORMAT));
    for (i = 1; i <= c.span_count; i++) if (c.spans[i].owns) { doc_free(c.spans[i].href); doc_free(c.spans[i].title); }
    doc_free(source);
    if (result == XUI_OK) result = doc_markdown_reconcile(t, &state);
    if (result != XUI_OK) { doc_state_release(state); return doc_txn_fail(t, result); }
    doc_state_release(t->draft); t->draft = state; t->parse_op_start = t->count; return XUI_OK;
}

int doc_txn_source_patch(xui_document_transaction t, uint64_t start, uint64_t end, const char* text, uint64_t bytes, int parse)
{
    doc_sequence *insert, *source = NULL;
    uint64_t size;
    xui_doc_operation_t op = {0};
    int result = doc_txn_check(t, 0);
    if (result != XUI_OK) return result;
    if (t->draft->profile != XUI_DOCUMENT_MARKDOWN) return doc_txn_fail(t, XUI_ERROR_UNSUPPORTED);
    size = doc_seq_size(t->draft->source);
    if (start > end || end > size) return doc_txn_fail(t, XUI_ERROR_INVALID_ARGUMENT);
    if (!doc_utf8(text, bytes) || !doc_seq_boundary(t->draft->source, start) || !doc_seq_boundary(t->draft->source, end)) return doc_txn_fail(t, XUI_DOC_ERROR_UTF8);
    if (bytes > t->document->max_bytes || size - (end - start) > t->document->max_bytes - bytes) return doc_txn_fail(t, XUI_DOC_ERROR_LIMIT);
    if (end - start == bytes && doc_seq_equal_bytes(t->draft->source, start, text, bytes)) return XUI_OK;
    insert = doc_seq_text(t->document->allocator, text, bytes);
    if (bytes && !insert) return doc_txn_fail(t, XUI_ERROR_OUT_OF_MEMORY);
    result = doc_seq_replace(t->document->allocator, t->draft->source, start, end, insert, &source);
    doc_seq_release(insert);
    if (result != XUI_OK) return doc_txn_fail(t, result);
    doc_seq_release(t->draft->source); t->draft->source = source;
    op.iKind = XUI_DOC_OP_SOURCE; op.iFlags = XUI_DOC_CHANGE_SOURCE | XUI_DOC_CHANGE_TEXT | XUI_DOC_CHANGE_STRUCTURE;
    op.iOffset = start; op.iOldLength = end - start; op.iNewLength = bytes;
    result = doc_txn_op(t, &op);
    return result == XUI_OK && parse ? doc_markdown_parse(t) : result;
}
int doc_txn_source(xui_document_transaction t, uint64_t start, uint64_t end, const char* text, uint64_t bytes)
{
    return doc_txn_source_patch(t, start, end, text, bytes, 1);
}
XUI_API int xuiDocumentTxnReplaceSource(xui_document_transaction t, uint64_t start, uint64_t end, const char* text, uint64_t bytes)
{
    int result = doc_txn_check(t, XUI_DOC_SOURCE);
    return result == XUI_OK ? doc_txn_source(t, start, end, text, bytes) : result;
}
XUI_API int xuiDocumentLoadMarkdown(xui_document d, const char* text, uint64_t bytes)
{
    xui_document_transaction t = NULL;
    xui_doc_txn_desc_t desc = {0};
    int result;
    if (!d) return XUI_ERROR_INVALID_ARGUMENT;
    desc.iSize = sizeof(desc); desc.iDomain = XUI_DOC_SOURCE;
    result = xuiDocumentBeginTransaction(d, &desc, &t);
    if (result != XUI_OK) return result;
    result = xuiDocumentTxnReplaceSource(t, 0, doc_seq_size(t->draft->source), text, bytes);
    if (result == XUI_OK) result = xuiDocumentTxnCommit(t, NULL);
    xuiDocumentTxnRelease(t); return result;
}
static int doc_md_escapable(unsigned char c)
{
    return (c >= 33 && c <= 47) || (c >= 58 && c <= 64) || (c >= 91 && c <= 96) || (c >= 123 && c <= 126);
}
int doc_markdown_text(xui_document_transaction t, uint64_t id, uint64_t start, uint64_t end, const char* text, uint64_t bytes)
{
    doc_node* p = doc_index_get(t->draft->index, id);
    struct xui_doc_transaction_t expected = {0};
    char *semantic = NULL, *escaped;
    uint64_t source_start, source_end, n = 0, i;
    int literal, result;
    if (!p || p->source_start == DOC_NONE || p->source_end == DOC_NONE) return doc_txn_fail(t, XUI_ERROR_UNSUPPORTED);
    expected.document = t->document; expected.domain = XUI_DOC_SEMANTIC; expected.parsing = 1;
    expected.draft = doc_state_clone(t->draft);
    if (!expected.draft) return doc_txn_fail(t, XUI_ERROR_OUT_OF_MEMORY);
    result = doc_txn_text(&expected, id, start, end, text, bytes);
    if (result != XUI_OK) { doc_state_release(expected.draft); return doc_txn_fail(t, result); }
    source_start = p->source_start; source_end = p->source_end;
    literal = p->kind != XUI_DOC_TEXT || (p->attrs.iMarks & XUI_DOC_CODE);
    if (p->source_exact && source_end - source_start == doc_seq_size(p->text)) {
        source_end = source_start + end; source_start += start;
    } else {
        doc_sequence *insert = doc_seq_text(t->document->allocator, text, bytes), *combined = NULL;
        if (bytes && !insert) { doc_state_release(expected.draft); return doc_txn_fail(t, XUI_ERROR_OUT_OF_MEMORY); }
        result = doc_seq_replace(t->document->allocator, p->text, start, end, insert, &combined);
        doc_seq_release(insert);
        if (result != XUI_OK) { doc_state_release(expected.draft); return doc_txn_fail(t, result); }
        semantic = doc_seq_string(t->document->allocator, combined); bytes = doc_seq_size(combined);
        doc_seq_release(combined);
        if (!semantic) { doc_state_release(expected.draft); return doc_txn_fail(t, XUI_ERROR_OUT_OF_MEMORY); }
        text = semantic;
    }
    if (bytes > (SIZE_MAX - 1) / 2) { doc_free(semantic); doc_state_release(expected.draft); return doc_txn_fail(t, XUI_DOC_ERROR_LIMIT); }
    escaped = doc_alloc(t->document->allocator, (size_t)bytes * 2 + 1);
    if (!escaped) { doc_free(semantic); doc_state_release(expected.draft); return doc_txn_fail(t, XUI_ERROR_OUT_OF_MEMORY); }
    for (i = 0; i < bytes; i++) {
        if (!literal && doc_md_escapable((unsigned char)text[i])) escaped[n++] = '\\';
        escaped[n++] = text[i];
    }
    result = doc_txn_source(t, source_start, source_end, escaped, n);
    if (result == XUI_OK && !doc_semantic_equal(expected.draft, t->draft)) result = doc_txn_fail(t, XUI_DOC_ERROR_UNREPRESENTABLE);
    doc_state_release(expected.draft); doc_free(escaped); doc_free(semantic); return result;
}
int doc_markdown_attributes(xui_document_transaction t, uint64_t id, const xui_doc_attributes_t* attrs)
{
    doc_node* p = doc_index_get(t->draft->index, id);
    xui_doc_attributes_t expected;
    char* source;
    uint64_t start, end;
    int result;
    if (!p || p->source_start == DOC_NONE) return doc_txn_fail(t, XUI_ERROR_UNSUPPORTED);
    expected = p->attrs;
    if (p->kind == XUI_DOC_TEXT) {
        xui_doc_range_t range;
        expected.iMarks = attrs->iMarks;
        if (!doc_attributes_equal(&expected, attrs)) return doc_txn_fail(t, XUI_ERROR_UNSUPPORTED);
        memset(&range, 0, sizeof(range)); range.tAnchor.iSize = range.tCaret.iSize = sizeof(range.tAnchor);
        range.tAnchor.iKind = range.tCaret.iKind = XUI_DOC_POSITION_TEXT;
        range.tAnchor.iDocumentId = range.tCaret.iDocumentId = t->document->identity;
        range.tAnchor.iRevision = range.tCaret.iRevision = t->base_revision;
        range.tAnchor.iNodeId = range.tCaret.iNodeId = id; range.tCaret.iOffset = doc_seq_size(p->text);
        return doc_markdown_marks(t, &range, attrs->iMarks & ~p->attrs.iMarks, p->attrs.iMarks & ~attrs->iMarks);
    }
    if (p->kind != XUI_DOC_LIST_ITEM || !(p->attrs.iFlags & XUI_DOC_TASK)) return doc_txn_fail(t, XUI_ERROR_UNSUPPORTED);
    expected.iFlags = (expected.iFlags & ~XUI_DOC_CHECKED) | (attrs->iFlags & XUI_DOC_CHECKED);
    if (!doc_attributes_equal(&expected, attrs)) return doc_txn_fail(t, XUI_ERROR_UNSUPPORTED);
    source = doc_seq_string(t->document->allocator, t->draft->source);
    if (!source) return doc_txn_fail(t, XUI_ERROR_OUT_OF_MEMORY);
    start = p->source_start;
    while (start && source[start - 1] != '\n' && source[start - 1] != '\r') start--;
    end = p->source_start;
    while (start + 2 < end && !(source[start] == '[' && source[start + 2] == ']' &&
        (source[start + 1] == ' ' || source[start + 1] == 'x' || source[start + 1] == 'X'))) start++;
    if (start + 2 >= end) { doc_free(source); return doc_txn_fail(t, XUI_ERROR_UNSUPPORTED); }
    doc_free(source);
    result = doc_txn_source(t, start + 1, start + 2, attrs->iFlags & XUI_DOC_CHECKED ? "x" : " ", 1);
    return result;
}
static int doc_markdown_marks_patch(xui_document_transaction t, const xui_doc_range_t* range, uint32_t set, uint32_t clear)
{
    doc_node* p;
    uint64_t start, end, a, b;
    const char* marker;
    unsigned marker_size;
    char *source, *replacement;
    int result;
    uint32_t change = set | clear;
    if (!range || range->tAnchor.iNodeId != range->tCaret.iNodeId || !change || (change & (change - 1))) return doc_txn_fail(t, XUI_ERROR_UNSUPPORTED);
    p = doc_index_get(t->draft->index, range->tAnchor.iNodeId);
    if (!p || p->kind != XUI_DOC_TEXT || !p->source_exact) return doc_txn_fail(t, XUI_ERROR_UNSUPPORTED);
    if (change == XUI_DOC_BOLD) { marker = "**"; marker_size = 2; }
    else if (change == XUI_DOC_ITALIC) { marker = "*"; marker_size = 1; }
    else if (change == XUI_DOC_STRIKE) { marker = "~~"; marker_size = 2; }
    else if (change == XUI_DOC_CODE) { marker = "`"; marker_size = 1; }
    else return doc_txn_fail(t, XUI_ERROR_UNSUPPORTED);
    a = range->tAnchor.iOffset; b = range->tCaret.iOffset;
    if (a > b) { uint64_t tmp = a; a = b; b = tmp; }
    if (b > doc_seq_size(p->text) || !doc_seq_boundary(p->text, a) || !doc_seq_boundary(p->text, b)) return doc_txn_fail(t, XUI_DOC_ERROR_UTF8);
    if (a == b || (set && (p->attrs.iMarks & set)) || (clear && !(p->attrs.iMarks & clear))) return XUI_OK;
    source = doc_seq_string(t->document->allocator, t->draft->source);
    if (!source) return doc_txn_fail(t, XUI_ERROR_OUT_OF_MEMORY);
    start = p->source_start + a; end = p->source_start + b;
    if (clear) {
        if (a || b != doc_seq_size(p->text) || start < marker_size || end + marker_size > doc_seq_size(t->draft->source)) goto unsupported;
        if (memcmp(source + start - marker_size, marker, marker_size) || memcmp(source + end, marker, marker_size)) {
            if (change == XUI_DOC_BOLD) marker = "__";
            else if (change == XUI_DOC_ITALIC) marker = "_";
            else goto unsupported;
            if (memcmp(source + start - marker_size, marker, marker_size) || memcmp(source + end, marker, marker_size)) goto unsupported;
        }
        replacement = doc_alloc(t->document->allocator, (size_t)(end - start) + 1);
        if (!replacement) { doc_free(source); return doc_txn_fail(t, XUI_ERROR_OUT_OF_MEMORY); }
        memcpy(replacement, source + start, (size_t)(end - start));
        result = doc_txn_source(t, start - marker_size, end + marker_size, replacement, end - start);
    } else {
        if (isspace((unsigned char)source[start]) || isspace((unsigned char)source[end - 1])) goto unsupported;
        replacement = doc_alloc(t->document->allocator, (size_t)(end - start) + 2 * marker_size + 1);
        if (!replacement) { doc_free(source); return doc_txn_fail(t, XUI_ERROR_OUT_OF_MEMORY); }
        memcpy(replacement, marker, marker_size); memcpy(replacement + marker_size, source + start, (size_t)(end - start));
        memcpy(replacement + marker_size + end - start, marker, marker_size);
        result = doc_txn_source(t, start, end, replacement, end - start + 2 * marker_size);
    }
    doc_free(replacement); doc_free(source); return result;
unsupported:
    doc_free(source); return doc_txn_fail(t, XUI_ERROR_UNSUPPORTED);
}
int doc_markdown_marks(xui_document_transaction t, const xui_doc_range_t* range, uint32_t set, uint32_t clear)
{
    struct xui_doc_transaction_t expected = {0}; int result;
    expected.document = t->document; expected.domain = XUI_DOC_SEMANTIC; expected.parsing = 1; expected.base_revision = t->base_revision;
    expected.draft = doc_state_clone(t->draft);
    if (!expected.draft) return doc_txn_fail(t, XUI_ERROR_OUT_OF_MEMORY);
    result = xuiDocumentTxnSetMarks(&expected, range, set, clear);
    if (result == XUI_OK) {
        uint32_t change = set | clear;
        doc_node* n = doc_index_get(t->draft->index, range->tAnchor.iNodeId);
        int whole = n && ((range->tAnchor.iOffset == 0 && range->tCaret.iOffset == doc_seq_size(n->text)) ||
            (range->tCaret.iOffset == 0 && range->tAnchor.iOffset == doc_seq_size(n->text)));
        if (range->tAnchor.iNodeId == range->tCaret.iNodeId && n && n->source_exact && (!clear || whole) && change && !(change & (change - 1)) &&
            !(n->attrs.iMarks & ~change) && (change & (XUI_DOC_BOLD | XUI_DOC_ITALIC | XUI_DOC_STRIKE | XUI_DOC_CODE)))
            result = doc_markdown_marks_patch(t, range, set, clear);
        else result = doc_markdown_apply_paragraphs(t, expected.draft, NULL, NULL);
    }
    if (result == XUI_OK && !doc_semantic_equal(expected.draft, t->draft)) result = XUI_DOC_ERROR_UNREPRESENTABLE;
    doc_state_release(expected.draft);
    return result == XUI_OK ? result : doc_txn_fail(t, result);
}
