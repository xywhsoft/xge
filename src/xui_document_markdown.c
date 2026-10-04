#include "../xui_config.h"
#if XUI_ENABLE_DOCUMENT
#include "xui_document_internal.h"
#include "xui_document_md4c.h"
#include "../lib/md4c/entity.h"
#include <ctype.h>
#include <stdio.h>

typedef struct doc_md_block {
    MD_BLOCKTYPE type;
    uint64_t id, paragraph;
    int fenced_code;
} doc_md_block;
typedef struct doc_md_span {
    uint32_t marks;
    uint64_t object, syntax_index;
    uint32_t syntax_flags;
    char *href, *title;
    int owns;
} doc_md_span;
typedef struct doc_md_context {
    struct xui_doc_transaction_t build;
    const char* source;
    uint64_t size, cursor, parser_offset, last_break_id, last_break_text_offset;
    MD_TEXTTYPE last_break_type;
    MD_TEXTTYPE pending_text_type, pending_content_type;
    uint64_t pending_text_start, pending_text_end;
    MD_SIZE pending_text_size;
    int pending_text_source;
    int html_scope;
    uint64_t html_scope_start, html_scope_end, html_node, html_parent;
    doc_md_block blocks[DOC_MAX_DEPTH];
    doc_md_span spans[DOC_MAX_DEPTH];
    xui_doc_inline_syntax_t* syntax;
    uint64_t syntax_count, syntax_capacity;
    xui_doc_inline_syntax_t* candidates;
    uint64_t candidate_count, candidate_capacity;
    doc_table_token_relative* table_tokens;
    uint64_t table_count, table_capacity, table_id;
    uint64_t ordinary_syntax_count;
    int footnote_section;
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
    out = doc_alloc(c->build.draft->allocator, (size_t)attr->size * 3 + 1);
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
static int doc_md_segment(doc_md_context* c, uint64_t id, uint64_t offset, uint64_t bytes,
    uint64_t start, uint64_t end, unsigned kind)
{
    doc_node* p = doc_index_get(c->build.draft->index, id); xui_doc_source_segment_t segment = {0};
    if (!p || !doc_text_kind(p->kind) || !bytes) return XUI_OK;
    segment.iSize = sizeof(segment); segment.iKind = start == DOC_NONE ? XUI_DOC_SOURCE_SYNTHETIC : kind;
    segment.iTextStart = offset; segment.iTextEnd = offset + bytes; segment.iSourceStart = start; segment.iSourceEnd = end;
    return c->result = doc_source_append(c->build.draft->allocator, &p->provenance, &segment);
}
static int doc_md_enter_block(MD_BLOCKTYPE type, void* detail, void* user)
{
    doc_md_context* c = user;
    xui_doc_node_desc_t d = {0};
    uint64_t id = DOC_ROOT;
    char* info = NULL;
    int result = XUI_OK;
    if (c->block_count >= DOC_MAX_DEPTH) return c->result = XUI_DOC_ERROR_LIMIT;
    if (type == MD_BLOCK_FOOTNOTE_DEF_SECTION && !c->footnote_section) {
        c->ordinary_syntax_count = c->syntax_count; c->footnote_section = 1;
    }
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
    if (type == MD_BLOCK_TABLE) {
        if (c->table_id) return c->result = XUI_DOC_ERROR_SCHEMA;
        c->table_id = id; c->table_count = 0;
    }
    c->blocks[c->block_count].type = type;
    c->blocks[c->block_count].id = id;
    c->blocks[c->block_count].paragraph = 0;
    c->blocks[c->block_count].fenced_code = type == MD_BLOCK_CODE && ((MD_BLOCK_CODE_DETAIL*)detail)->fence_char != 0;
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
    if (frame.type == MD_BLOCK_TABLE) {
        doc_blob* tokens;
        if (c->table_id != frame.id || !c->table_count)
            return c->result = XUI_DOC_ERROR_FORMAT;
        if (c->table_count > SIZE_MAX / sizeof(*c->table_tokens))
            return c->result = XUI_DOC_ERROR_LIMIT;
        tokens = doc_blob_new(c->build.draft->allocator, (const char*)c->table_tokens,
            c->table_count * sizeof(*c->table_tokens));
        if (!tokens) return c->result = XUI_ERROR_OUT_OF_MEMORY;
        p = doc_index_get(c->build.draft->index, frame.id);
        doc_blob_release(p->syntax_aux); p->syntax_aux = tokens;
        c->table_id = 0; c->table_count = 0;
    }
    if (frame.paragraph && frame.paragraph != frame.id) {
        p = doc_index_get(c->build.draft->index, frame.paragraph);
        doc_md_range(c, frame.id, p->source_start, p->source_end, 0);
    }
    p = doc_index_get(c->build.draft->index, frame.id);
    if (c->block_count && frame.id != doc_md_parent(c)) doc_md_range(c, doc_md_parent(c), p->source_start, p->source_end, 0);
    return 0;
}
static void doc_md_block_source(MD_BLOCKTYPE type, MD_OFFSET start, MD_OFFSET end, int enter, void* user)
{
    doc_md_context* c = user;
    doc_node* n;
    if (!c->block_count || type == MD_BLOCK_BLANK) return;
    n = doc_index_get(c->build.draft->index, doc_md_parent(c));
    if (enter) n->syntax_start = c->parser_offset + start;
    n->syntax_end = c->parser_offset + end;
    if (type == MD_BLOCK_DOC) n->syntax_start = 0;
}
static void doc_md_block_markers(int kind, MD_OFFSET first_start, MD_OFFSET first_end,
    MD_OFFSET second_start, MD_OFFSET second_end, MD_OFFSET fence_tail_end,
    MD_OFFSET list_gap_end, MD_OFFSET task_gap_end, void* user)
{
    doc_md_context* c = user;
    doc_node* n;
    uint64_t base, start, end;
    int valid_kind;
    if (c->result || !c->block_count) return;
    n = doc_index_get(c->build.draft->index, doc_md_parent(c));
    if (!n || n->syntax_start == DOC_NONE) { c->result = XUI_DOC_ERROR_FORMAT; return; }
    base = n->syntax_start;
    start = c->parser_offset + first_start; end = c->parser_offset + first_end;
    valid_kind = (kind == XUI_DOC_BLOCK_SYNTAX_ATX_HEADING || kind == XUI_DOC_BLOCK_SYNTAX_SETEXT_HEADING) ?
        n->kind == XUI_DOC_HEADING : kind == XUI_DOC_BLOCK_SYNTAX_FENCED_CODE ?
        n->kind == XUI_DOC_CODE_BLOCK || n->kind == XUI_DOC_DIAGRAM :
        kind == XUI_DOC_BLOCK_SYNTAX_THEMATIC_BREAK ? n->kind == XUI_DOC_RULE :
        kind == XUI_DOC_BLOCK_SYNTAX_LIST_ITEM ? n->kind == XUI_DOC_LIST_ITEM :
        kind == XUI_DOC_BLOCK_SYNTAX_QUOTE_OPEN ? n->kind == XUI_DOC_QUOTE :
        kind == XUI_DOC_BLOCK_SYNTAX_TABLE_UNDERLINE && n->kind == XUI_DOC_TABLE;
    if (start < base || end <= start || end > n->syntax_end || end - base > UINT32_MAX ||
        !valid_kind) {
        c->result = XUI_DOC_ERROR_FORMAT; return;
    }
    n->marker_kind = (uint32_t)kind;
    n->marker_primary_start = (uint32_t)(start - base); n->marker_primary_end = (uint32_t)(end - base);
    if (kind == XUI_DOC_BLOCK_SYNTAX_FENCED_CODE) {
        uint64_t tail = c->parser_offset + fence_tail_end;
        if (tail < end || tail > n->syntax_end || tail - base > UINT32_MAX) {
            c->result = XUI_DOC_ERROR_FORMAT; return;
        }
        n->marker_tail_end = (uint32_t)(tail - base);
    }
    if (second_start < second_end) {
        start = c->parser_offset + second_start; end = c->parser_offset + second_end;
        if (start < base || end > n->syntax_end || start >= end || end - base > UINT32_MAX) {
            c->result = XUI_DOC_ERROR_FORMAT; return;
        }
        n->marker_secondary_start = (uint32_t)(start - base);
        n->marker_secondary_end = (uint32_t)(end - base);
    }
    if (kind == XUI_DOC_BLOCK_SYNTAX_LIST_ITEM) {
        uint64_t gap = c->parser_offset + list_gap_end;
        uint64_t task_gap = task_gap_end ? c->parser_offset + task_gap_end : DOC_NONE;
        uint64_t at;
        if (gap < c->parser_offset + first_end || gap > n->syntax_end ||
            gap - base > UINT32_MAX ||
            (n->marker_secondary_end ?
                (gap > base + n->marker_secondary_start || task_gap == DOC_NONE ||
                 task_gap < base + n->marker_secondary_end || task_gap > n->syntax_end ||
                 task_gap - base > UINT32_MAX) : task_gap != DOC_NONE)) {
            c->result = XUI_DOC_ERROR_FORMAT; return;
        }
        for (at = c->parser_offset + first_end;
             at < (n->marker_secondary_end ? base + n->marker_secondary_start : gap); at++) {
            if (c->source[at] != ' ' && c->source[at] != '\t') {
                c->result = XUI_DOC_ERROR_FORMAT; return;
            }
        }
        if (task_gap != DOC_NONE) {
            for (at = base + n->marker_secondary_end; at < task_gap; at++) {
                if (c->source[at] != ' ' && c->source[at] != '\t') {
                    c->result = XUI_DOC_ERROR_FORMAT; return;
                }
            }
            n->marker_tail_end = (uint32_t)(task_gap - base);
        } else {
            n->marker_tail_end = (uint32_t)(gap - base);
        }
    }
    if (kind == XUI_DOC_BLOCK_SYNTAX_QUOTE_OPEN && n->info && n->info->size) {
        uint64_t first = base + n->marker_secondary_start, last = base + n->marker_secondary_end;
        uint64_t tail = c->parser_offset + fence_tail_end, at;
        if (first < base + n->marker_primary_end || last - first != n->info->size + 3 ||
            tail < last || tail > n->syntax_end || tail - base > UINT32_MAX ||
            c->source[first] != '[' || c->source[first + 1] != '!' || c->source[last - 1] != ']') {
            c->result = XUI_DOC_ERROR_FORMAT; return;
        }
        for (at = base + n->marker_primary_end; at < first; at++)
            if (c->source[at] != ' ' && c->source[at] != '\t') { c->result = XUI_DOC_ERROR_FORMAT; return; }
        for (at = 0; at < n->info->size; at++) {
            unsigned char ch = (unsigned char)c->source[first + 2 + at];
            if (ch >= 'A' && ch <= 'Z') ch += 'a' - 'A';
            if (ch != (unsigned char)n->info->data[at]) { c->result = XUI_DOC_ERROR_FORMAT; return; }
        }
        for (at = last; at < tail && c->source[at] != '\r' && c->source[at] != '\n'; at++)
            if (c->source[at] != ' ' && c->source[at] != '\t') { c->result = XUI_DOC_ERROR_FORMAT; return; }
        if (at < tail && c->source[at] == '\r') at++;
        if (at < tail && c->source[at] == '\n') at++;
        if (at != tail) { c->result = XUI_DOC_ERROR_FORMAT; return; }
        n->marker_tail_end = (uint32_t)(tail - base);
    }
}
static int doc_md_heading_content(MD_OFFSET start, MD_OFFSET end, void* user)
{
    doc_md_context* c = user;
    doc_node* n;
    doc_heading_content_relative relative;
    uint64_t base, beg, finish;
    if (c->result) return c->result;
    n = c->block_count ? doc_index_get(c->build.draft->index, doc_md_parent(c)) : NULL;
    if (!n || n->kind != XUI_DOC_HEADING || n->syntax_aux ||
        (n->marker_kind != XUI_DOC_BLOCK_SYNTAX_ATX_HEADING &&
         n->marker_kind != XUI_DOC_BLOCK_SYNTAX_SETEXT_HEADING))
        return c->result = XUI_DOC_ERROR_FORMAT;
    base = n->syntax_start;
    beg = c->parser_offset + start; finish = c->parser_offset + end;
    if (base == DOC_NONE || beg < base || finish < beg || finish > n->syntax_end ||
        finish - base > UINT32_MAX ||
        (n->marker_kind == XUI_DOC_BLOCK_SYNTAX_ATX_HEADING && beg < base + n->marker_primary_end) ||
        (n->marker_kind == XUI_DOC_BLOCK_SYNTAX_SETEXT_HEADING && finish > base + n->marker_primary_start))
        return c->result = XUI_DOC_ERROR_FORMAT;
    relative.start = (uint32_t)(beg - base); relative.end = (uint32_t)(finish - base);
    n->syntax_aux = doc_blob_new(c->build.draft->allocator, (const char*)&relative, sizeof(relative));
    return n->syntax_aux ? XUI_OK : (c->result = XUI_ERROR_OUT_OF_MEMORY);
}
static int doc_md_fence_info(MD_OFFSET info_beg, MD_OFFSET info_end, MD_OFFSET lang_end, void* user)
{
    doc_md_context* c = user;
    doc_node* n;
    doc_fence_info_relative relative;
    doc_blob* blob;
    uint64_t base, start, end, language_end, tail, at;
    if (c->result) return c->result;
    if (!c->block_count) return c->result = XUI_DOC_ERROR_FORMAT;
    n = doc_index_get(c->build.draft->index, doc_md_parent(c));
    if (!n || (n->kind != XUI_DOC_CODE_BLOCK && n->kind != XUI_DOC_DIAGRAM) ||
        n->marker_kind != XUI_DOC_BLOCK_SYNTAX_FENCED_CODE || n->syntax_aux)
        return c->result = XUI_DOC_ERROR_FORMAT;
    base = n->syntax_start;
    start = c->parser_offset + info_beg;
    end = c->parser_offset + info_end;
    language_end = c->parser_offset + lang_end;
    tail = base + n->marker_tail_end;
    if (start < base + n->marker_primary_end || start > language_end ||
        language_end > end || end > tail || tail > c->size || end - base > UINT32_MAX)
        return c->result = XUI_DOC_ERROR_FORMAT;
    for (at = base + n->marker_primary_end; at < start; at++)
        if (c->source[at] != ' ') return c->result = XUI_DOC_ERROR_FORMAT;
    for (at = end; at < tail; at++)
        if (c->source[at] != ' ') return c->result = XUI_DOC_ERROR_FORMAT;
    relative.info_start = (uint32_t)(start - base);
    relative.info_end = (uint32_t)(end - base);
    relative.language_end = (uint32_t)(language_end - base);
    blob = doc_blob_new(c->build.draft->allocator, (const char*)&relative, sizeof(relative));
    if (!blob) return c->result = XUI_ERROR_OUT_OF_MEMORY;
    n->syntax_aux = blob;
    return XUI_OK;
}
static int doc_md_break_source(int kind, MD_OFFSET trailing, MD_OFFSET marker_beg,
    MD_OFFSET marker_end, MD_OFFSET newline_beg, MD_OFFSET newline_end, void* user)
{
    doc_md_context* c = user;
    doc_node* n;
    uint64_t base, at, end, line_end, text_offset = c->last_break_text_offset;
    uint64_t marker_start = DOC_NONE, marker_finish = DOC_NONE;
    int projected;
    if (c->result) return c->result;
    if (!c->last_break_id) return XUI_OK;
    n = doc_index_get(c->build.draft->index, c->last_break_id);
    c->last_break_id = 0;
    projected = n && doc_text_kind(n->kind);
    at = c->parser_offset + trailing;
    end = c->parser_offset + newline_beg;
    line_end = c->parser_offset + newline_end;
    if (marker_beg != UINT_MAX || marker_end != UINT_MAX) {
        if (marker_beg == UINT_MAX || marker_end == UINT_MAX)
            return c->result = XUI_DOC_ERROR_FORMAT;
        marker_start = c->parser_offset + marker_beg;
        marker_finish = c->parser_offset + marker_end;
    }
    base = marker_start != DOC_NONE && marker_start < at ? marker_start : at;
    if (!n || (!projected && (n->kind != XUI_DOC_SOFT_BREAK && n->kind != XUI_DOC_HARD_BREAK)) ||
        (!projected && n->marker_kind) || kind < XUI_DOC_BREAK_SOFT || kind > XUI_DOC_BREAK_HARD_FORCED ||
        (c->last_break_type == MD_TEXT_SOFTBR) != (kind == XUI_DOC_BREAK_SOFT) ||
        (projected && !doc_seq_equal_bytes(n->text, text_offset, "\n", 1)) ||
        at > end || end >= line_end || line_end > c->size || line_end - base > UINT32_MAX)
        return c->result = XUI_DOC_ERROR_FORMAT;
    while (at < end) {
        if (c->source[at] != ' ' && c->source[at] != '\t')
            return c->result = XUI_DOC_ERROR_FORMAT;
        at++;
    }
    if (c->source[end] != '\r' && c->source[end] != '\n')
        return c->result = XUI_DOC_ERROR_FORMAT;
    if (marker_start != DOC_NONE) {
        if (marker_start < base || marker_start >= marker_finish || marker_finish > end)
            return c->result = XUI_DOC_ERROR_FORMAT;
    }
    if (kind == XUI_DOC_BREAK_HARD_BACKSLASH) {
        if (marker_finish != end || marker_start + 1 != end || c->source[marker_start] != '\\')
            return c->result = XUI_DOC_ERROR_FORMAT;
    } else if (kind == XUI_DOC_BREAK_HARD_SPACES) {
        if (marker_start != c->parser_offset + trailing || marker_finish != end ||
            marker_finish - marker_start < 2 || c->source[end - 1] != ' ' || c->source[end - 2] != ' ')
            return c->result = XUI_DOC_ERROR_FORMAT;
    } else if (marker_start != DOC_NONE || marker_finish != DOC_NONE)
        return c->result = XUI_DOC_ERROR_FORMAT;
    c->cursor = line_end;
    if (projected) {
        /* Image alt and other literal inline objects own text rather than a
         * separate break node. Attach its normalized newline to the same
         * parser-confirmed origin used by ordinary breaks. */
        doc_md_range(c, n->id, base, line_end, 0);
        doc_md_range(c, n->parent, base, line_end, 0);
        return doc_md_segment(c, n->id, text_offset, 1, base, line_end, XUI_DOC_SOURCE_NORMALIZED);
    }
    /* The parser's synthetic newline callback can precede every text callback
     * in an empty task first line. The previous text cursor belongs to another
     * block then; only this parser-confirmed hook owns the break coordinates. */
    n->source_start = base; n->source_end = line_end; n->source_exact = 0;
    doc_md_range(c, n->parent, base, line_end, 0);
    n->marker_kind = (uint32_t)kind;
    n->marker_primary_start = (uint32_t)(c->parser_offset + trailing - base);
    n->marker_primary_end = (uint32_t)(end - base);
    n->marker_tail_end = (uint32_t)(line_end - base);
    if (marker_start != DOC_NONE) {
        n->marker_secondary_start = (uint32_t)(marker_start - base);
        n->marker_secondary_end = (uint32_t)(marker_finish - base);
    }
    return XUI_OK;
}
static int doc_md_quote_prefixes(const MD_OFFSET* offsets, MD_SIZE count, void* user)
{
    doc_md_context* c = user;
    doc_node* n;
    uint64_t base, previous = DOC_NONE;
    uint32_t* relative = NULL;
    doc_quote_indent_relative* indents = NULL;
    doc_blob *prefix_blob = NULL, *indent_blob = NULL;
    MD_SIZE i;
    int result = XUI_OK;
    if (c->result) return c->result;
    if (!offsets || !count || !c->block_count) return c->result = XUI_DOC_ERROR_FORMAT;
    n = doc_index_get(c->build.draft->index, doc_md_parent(c));
    if (!n || n->kind != XUI_DOC_QUOTE ||
        n->marker_kind != XUI_DOC_BLOCK_SYNTAX_QUOTE_OPEN ||
        n->syntax_start == DOC_NONE) return c->result = XUI_DOC_ERROR_FORMAT;
    base = n->syntax_start;
#if SIZE_MAX <= UINT_MAX
    if (count > SIZE_MAX / sizeof(*indents))
        return c->result = XUI_DOC_ERROR_LIMIT;
#endif
    if (count > 1) {
        relative = doc_alloc(c->build.draft->allocator, (size_t)(count - 1) * sizeof(*relative));
        if (!relative) return c->result = XUI_ERROR_OUT_OF_MEMORY;
    }
    indents = doc_alloc(c->build.draft->allocator, (size_t)count * sizeof(*indents));
    if (!indents) { doc_free(relative); return c->result = XUI_ERROR_OUT_OF_MEMORY; }
    for (i = 0; i < count; i++) {
        size_t slot = (size_t)i * 6;
        uint64_t at = c->parser_offset + offsets[slot];
        uint64_t end = c->parser_offset + offsets[slot + 1];
        if (!(i & 4095u)) {
            result = doc_txn_check(&c->build, 0); if (result != XUI_OK) goto done;
        }
        if (at < base || at >= c->size || at - base > UINT32_MAX ||
            c->source[at] != '>' || (previous != DOC_NONE && at <= previous) ||
            (i == 0 && at - base != n->marker_primary_start) ||
            end <= at || end > c->size || end - base > UINT32_MAX ||
            offsets[slot + 2] > offsets[slot + 3] || offsets[slot + 3] >= offsets[slot + 4] ||
            offsets[slot + 4] > offsets[slot + 5]) {
            result = XUI_DOC_ERROR_FORMAT; goto done;
        }
        if (i) relative[i - 1] = (uint32_t)(at - base);
        indents[i] = (doc_quote_indent_relative){(uint32_t)(at - base), (uint32_t)(end - base),
            offsets[slot + 2], offsets[slot + 3], offsets[slot + 4], offsets[slot + 5]};
        previous = at;
    }
    if (count > 1) {
        prefix_blob = doc_blob_new(c->build.draft->allocator, (const char*)relative,
            (uint64_t)(count - 1) * sizeof(*relative));
        if (!prefix_blob) { result = XUI_ERROR_OUT_OF_MEMORY; goto done; }
    }
    indent_blob = doc_blob_new(c->build.draft->allocator, (const char*)indents,
        (uint64_t)count * sizeof(*indents));
    if (!indent_blob) { result = XUI_ERROR_OUT_OF_MEMORY; goto done; }
    doc_blob_release(n->quote_prefixes); n->quote_prefixes = prefix_blob; prefix_blob = NULL;
    doc_blob_release(n->syntax_aux); n->syntax_aux = indent_blob; indent_blob = NULL;
done:
    doc_free(relative); doc_free(indents); doc_blob_release(prefix_blob); doc_blob_release(indent_blob);
    return c->result = result;
}
static int doc_md_list_indents(const MD_OFFSET* records, MD_SIZE count, void* user)
{
    doc_md_context* c = user;
    doc_node* n;
    uint64_t base, previous = DOC_NONE;
    doc_list_indent_relative* relative;
    MD_SIZE i;
    if (c->result) return c->result;
    if (!records || !count || !c->block_count) return c->result = XUI_DOC_ERROR_FORMAT;
    n = doc_index_get(c->build.draft->index, doc_md_parent(c));
    if (!n || n->kind != XUI_DOC_LIST_ITEM ||
        n->marker_kind != XUI_DOC_BLOCK_SYNTAX_LIST_ITEM ||
        n->syntax_start == DOC_NONE) return c->result = XUI_DOC_ERROR_FORMAT;
    base = n->syntax_start;
    {
        doc_list_indent_relative first;
        doc_blob* blob;
        uint64_t beg = c->parser_offset + records[0], end = c->parser_offset + records[1];
        if (beg < base || beg - base != n->marker_primary_start ||
            end < beg || end > c->size || end - base != n->marker_tail_end ||
            records[2] >= records[3] || records[3] > records[4])
            return c->result = XUI_DOC_ERROR_FORMAT;
        first = (doc_list_indent_relative){(uint32_t)(beg - base), (uint32_t)(end - base),
            records[2], records[3], records[4]};
        blob = doc_blob_new(c->build.draft->allocator, (const char*)&first, sizeof(first));
        if (!blob) return c->result = XUI_ERROR_OUT_OF_MEMORY;
        doc_blob_release(n->syntax_aux); n->syntax_aux = blob;
        records += 5; count--;
        if (!count) { doc_blob_release(n->list_indents); n->list_indents = NULL; return XUI_OK; }
    }
#if SIZE_MAX <= UINT_MAX
    if (count > SIZE_MAX / sizeof(*relative))
        return c->result = XUI_DOC_ERROR_LIMIT;
#endif
    relative = doc_alloc(c->build.draft->allocator, (size_t)count * sizeof(*relative));
    if (!relative) return c->result = XUI_ERROR_OUT_OF_MEMORY;
    for (i = 0; i < count; i++) {
        size_t slot = (size_t)i * 5;
        uint64_t start = c->parser_offset + records[slot];
        uint64_t end = c->parser_offset + records[slot + 1], at;
        if (start < base || start >= end || end > c->size ||
            end - base > UINT32_MAX || (previous != DOC_NONE && start < previous) ||
            start - base <= n->marker_primary_end ||
            records[slot + 2] >= records[slot + 3] ||
            records[slot + 3] > records[slot + 4]) {
            doc_free(relative); return c->result = XUI_DOC_ERROR_FORMAT;
        }
        for (at = start; at < end; at++) {
            if (c->source[at] != ' ' && c->source[at] != '\t') {
                doc_free(relative); return c->result = XUI_DOC_ERROR_FORMAT;
            }
        }
        relative[i].start = (uint32_t)(start - base);
        relative[i].end = (uint32_t)(end - base);
        relative[i].start_column = records[slot + 2];
        relative[i].content_column = records[slot + 3];
        relative[i].end_column = records[slot + 4];
        previous = end;
    }
    {
        doc_blob* blob = doc_blob_new(c->build.draft->allocator, (const char*)relative,
            (uint64_t)count * sizeof(*relative));
        doc_free(relative);
        if (!blob) return c->result = XUI_ERROR_OUT_OF_MEMORY;
        doc_blob_release(n->list_indents); n->list_indents = blob;
    }
    return XUI_OK;
}
static int doc_md_code_indents(const MD_OFFSET* records, MD_SIZE count, void* user)
{
    doc_md_context* c = user;
    doc_node* n;
    doc_code_indent_relative* relative;
    doc_blob* blob;
    uint64_t base, previous = DOC_NONE;
    MD_SIZE i;
    if (c->result) return c->result;
    if (!records || !count || !c->block_count) return c->result = XUI_DOC_ERROR_FORMAT;
    n = doc_index_get(c->build.draft->index, doc_md_parent(c));
    if (!n || n->kind != XUI_DOC_CODE_BLOCK ||
        c->blocks[c->block_count - 1].fenced_code || n->marker_kind || n->syntax_aux ||
        n->syntax_start == DOC_NONE || n->syntax_end == DOC_NONE)
        return c->result = XUI_DOC_ERROR_FORMAT;
    base = n->syntax_start;
#if SIZE_MAX <= UINT_MAX
    if (count > SIZE_MAX / sizeof(*relative)) return c->result = XUI_DOC_ERROR_LIMIT;
#endif
    relative = doc_alloc(c->build.draft->allocator, (size_t)count * sizeof(*relative));
    if (!relative) return c->result = XUI_ERROR_OUT_OF_MEMORY;
    for (i = 0; i < count; i++) {
        size_t slot = (size_t)i * 6;
        uint64_t start = c->parser_offset + records[slot];
        uint64_t end = c->parser_offset + records[slot + 1], at;
        uint32_t flags = records[slot + 5];
        if (start < base || start > end || end > n->syntax_end ||
            end - base > UINT32_MAX || (previous != DOC_NONE && start <= previous) ||
            flags > XUI_DOC_CODE_INDENT_BLANK ||
            (flags ? records[slot + 3] != UINT_MAX ||
                records[slot + 2] != records[slot + 4] :
                start == end || records[slot + 2] >= records[slot + 3] ||
                records[slot + 3] > records[slot + 4] ||
                records[slot + 3] - records[slot + 2] != 4)) {
            doc_free(relative); return c->result = XUI_DOC_ERROR_FORMAT;
        }
        for (at = start; at < end; at++) {
            if (c->source[at] != ' ' && c->source[at] != '\t') {
                doc_free(relative); return c->result = XUI_DOC_ERROR_FORMAT;
            }
        }
        relative[i].start = (uint32_t)(start - base);
        relative[i].end = (uint32_t)(end - base);
        relative[i].start_column = records[slot + 2];
        relative[i].content_column = records[slot + 3];
        relative[i].end_column = records[slot + 4];
        relative[i].flags = flags;
        previous = end;
    }
    if (relative[0].flags || relative[0].start == relative[0].end) {
        doc_free(relative); return c->result = XUI_DOC_ERROR_FORMAT;
    }
    blob = doc_blob_new(c->build.draft->allocator, (const char*)relative,
        (uint64_t)count * sizeof(*relative));
    if (!blob) { doc_free(relative); return c->result = XUI_ERROR_OUT_OF_MEMORY; }
    n->syntax_aux = blob;
    n->marker_kind = XUI_DOC_BLOCK_SYNTAX_INDENTED_CODE;
    n->marker_primary_start = relative[0].start;
    n->marker_primary_end = relative[0].end;
    doc_free(relative);
    return XUI_OK;
}
static int doc_md_table_token(int kind, MD_SIZE row, MD_SIZE ordinal,
    MD_OFFSET from, MD_OFFSET to, unsigned flags, void* user)
{
    doc_md_context* c = user;
    doc_node* table;
    doc_table_token_relative* token;
    uint64_t start = c->parser_offset + from, end = c->parser_offset + to, base;
    uint64_t i;
    if (c->result) return c->result;
    table = doc_index_get(c->build.draft->index, c->table_id);
    if (!table || table->kind != XUI_DOC_TABLE ||
        table->syntax_start == DOC_NONE || table->marker_kind != XUI_DOC_BLOCK_SYNTAX_TABLE_UNDERLINE)
        return c->result = XUI_DOC_ERROR_FORMAT;
    base = table->syntax_start;
    if (start < base || end <= start || end > table->syntax_end || end > c->size ||
        end - base > UINT32_MAX ||
        (kind != XUI_DOC_TABLE_TOKEN_PIPE && kind != XUI_DOC_TABLE_TOKEN_UNDERLINE) ||
        (kind == XUI_DOC_TABLE_TOKEN_PIPE && (end != start + 1 || c->source[start] != '|' || flags)) ||
        (kind == XUI_DOC_TABLE_TOKEN_UNDERLINE && (row != 1 || flags > 3)))
        return c->result = XUI_DOC_ERROR_FORMAT;
    if (kind == XUI_DOC_TABLE_TOKEN_UNDERLINE) {
        int hyphen = 0;
        if (((flags & 1) != 0) != (c->source[start] == ':') ||
            ((flags & 2) != 0) != (c->source[end - 1] == ':'))
            return c->result = XUI_DOC_ERROR_FORMAT;
        for (i = start; i < end; i++) {
            if (c->source[i] == '-') hyphen = 1;
            else if (c->source[i] != ':') return c->result = XUI_DOC_ERROR_FORMAT;
        }
        if (!hyphen) return c->result = XUI_DOC_ERROR_FORMAT;
    }
    if (c->table_count == c->table_capacity) {
        uint64_t capacity = c->table_capacity ? c->table_capacity + c->table_capacity / 2 : 16;
        doc_table_token_relative* next;
        if (capacity <= c->table_capacity || capacity > SIZE_MAX / sizeof(*next))
            return c->result = XUI_DOC_ERROR_LIMIT;
        next = doc_realloc(c->build.draft->allocator, c->table_tokens,
            (size_t)capacity * sizeof(*next));
        if (!next) return c->result = XUI_ERROR_OUT_OF_MEMORY;
        c->table_tokens = next; c->table_capacity = capacity;
    }
    token = &c->table_tokens[c->table_count++];
    token->start = (uint32_t)(start - base); token->end = (uint32_t)(end - base);
    token->row = (uint32_t)row; token->ordinal = (uint32_t)ordinal;
    token->kind = (uint32_t)kind; token->flags = flags;
    return XUI_OK;
}
static int doc_md_store_definition(doc_md_context* c, const xui_doc_reference_definition_t* range)
{
    doc_state* state = c->build.draft;
    doc_sequence *insert, *next = NULL; uint64_t size = doc_seq_size(state->references);
    if (c->result) return c->result;
    insert = doc_seq_text(state->allocator, (const char*)range, sizeof(*range));
    if (!insert) return c->result = XUI_ERROR_OUT_OF_MEMORY;
    c->result = doc_seq_replace(state->allocator, state->references, size, size, insert, &next); doc_seq_release(insert);
    if (!c->result) { doc_seq_release(state->references); state->references = next; }
    return c->result;
}
static int doc_md_reference_source(MD_OFFSET start, MD_OFFSET end, MD_OFFSET label_start, MD_OFFSET label_end,
    MD_OFFSET destination_start, MD_OFFSET destination_end, MD_OFFSET title_start, MD_OFFSET title_end, int has_title, void* user)
{
    doc_md_context* c = user; xui_doc_reference_definition_t range = {0};
    if (c->result) return c->result;
    range.iSize = sizeof(range); range.iKind = XUI_DOC_REFERENCE_LINK;
    range.iSourceStart = c->parser_offset + start; range.iSourceEnd = c->parser_offset + end;
    range.iLabelStart = c->parser_offset + label_start; range.iLabelEnd = c->parser_offset + label_end;
    range.iDestinationStart = c->parser_offset + destination_start; range.iDestinationEnd = c->parser_offset + destination_end;
    range.iTitleStart = has_title ? c->parser_offset + title_start : DOC_NONE;
    range.iTitleEnd = has_title ? c->parser_offset + title_end : DOC_NONE;
    while (range.iSourceStart > c->parser_offset && c->source[range.iSourceStart - 1] != '\n' && c->source[range.iSourceStart - 1] != '\r') range.iSourceStart--;
    while (range.iSourceEnd < c->size && c->source[range.iSourceEnd] != '\n' && c->source[range.iSourceEnd] != '\r') range.iSourceEnd++;
    if (range.iSourceEnd < c->size && c->source[range.iSourceEnd] == '\r') range.iSourceEnd++;
    if (range.iSourceEnd < c->size && c->source[range.iSourceEnd] == '\n') range.iSourceEnd++;
    return doc_md_store_definition(c, &range);
}
static int doc_md_reference_values(const char* label, MD_SIZE label_bytes, const char* destination, MD_SIZE destination_bytes,
    const char* title, MD_SIZE title_bytes, int has_title, void* user)
{
    doc_md_context* c = user; doc_state* state = c->build.draft;
    xui_doc_reference_definition_t last;
    uint64_t bytes = doc_seq_size(state->references);
    if (c->result) return c->result;
    if (bytes < sizeof(last) || bytes % sizeof(last) ||
        doc_seq_size(state->reference_values) + 1 != bytes / sizeof(last) ||
        doc_seq_read(state->references, bytes - sizeof(last), &last, sizeof(last)) != XUI_OK ||
        last.iKind != XUI_DOC_REFERENCE_LINK) return c->result = XUI_DOC_ERROR_SCHEMA;
    return c->result = doc_reference_value_append(state, XUI_DOC_REFERENCE_LINK,
        label, label_bytes, destination, destination_bytes, title, title_bytes, has_title, c->build.cancellation);
}
static int doc_md_footnote_source(MD_OFFSET start, MD_OFFSET end, MD_OFFSET label_start, MD_OFFSET label_end,
    const char* body, MD_SIZE body_bytes, void* user)
{
    doc_md_context* c = user; xui_doc_reference_definition_t range = {0};
    if (c->result) return c->result;
    range.iSize = sizeof(range); range.iKind = XUI_DOC_REFERENCE_FOOTNOTE;
    range.iSourceStart = c->parser_offset + start; range.iSourceEnd = c->parser_offset + end;
    range.iLabelStart = c->parser_offset + label_start; range.iLabelEnd = c->parser_offset + label_end;
    range.iDestinationStart = range.iDestinationEnd = DOC_NONE;
    range.iTitleStart = range.iTitleEnd = DOC_NONE;
    if (doc_md_store_definition(c, &range) != XUI_OK) return c->result;
    return c->result = doc_reference_value_append(c->build.draft, XUI_DOC_REFERENCE_FOOTNOTE,
        c->source + range.iLabelStart, label_end - label_start,
        body, body_bytes, NULL, 0, 0, c->build.cancellation);
}
static int doc_md_candidate_source(int kind, MD_OFFSET start, MD_OFFSET end,
    MD_OFFSET label_start, MD_OFFSET label_end, void* user)
{
    doc_md_context* c = user; xui_doc_inline_syntax_t* candidate;
    uint64_t capacity;
    if (c->result) return c->result;
    if (kind < 1 || kind > 3 || start > label_start || label_start > label_end ||
        label_end > end || c->parser_offset + end > c->size) return c->result = XUI_DOC_ERROR_FORMAT;
    if (c->candidate_count == c->candidate_capacity) {
        capacity = c->candidate_capacity ? c->candidate_capacity * 2 : 16;
        if (capacity > c->build.draft->allocator->max_nodes) capacity = c->build.draft->allocator->max_nodes;
        if (capacity <= c->candidate_count || capacity > SIZE_MAX / sizeof(*candidate))
            return c->result = XUI_DOC_ERROR_LIMIT;
        candidate = doc_realloc(c->build.draft->allocator, c->candidates, (size_t)capacity * sizeof(*candidate));
        if (!candidate) return c->result = XUI_ERROR_OUT_OF_MEMORY;
        c->candidates = candidate; c->candidate_capacity = capacity;
    }
    candidate = &c->candidates[c->candidate_count++]; memset(candidate, 0, sizeof(*candidate));
    candidate->iSize = sizeof(*candidate); candidate->iKind = XUI_DOC_SYNTAX_CANDIDATE_LINK + (uint32_t)kind - 1;
    candidate->iParentIndex = candidate->iDefinitionIndex = DOC_NONE;
    candidate->iSourceStart = c->parser_offset + start; candidate->iSourceEnd = c->parser_offset + end;
    candidate->iContentStart = c->parser_offset + label_start; candidate->iContentEnd = c->parser_offset + label_end;
    return XUI_OK;
}
static int doc_md_candidate_order(const void* left, const void* right)
{
    const xui_doc_inline_syntax_t* a = left; const xui_doc_inline_syntax_t* b = right;
    if (a->iSourceStart != b->iSourceStart) return a->iSourceStart < b->iSourceStart ? -1 : 1;
    if (a->iSourceEnd != b->iSourceEnd) return a->iSourceEnd < b->iSourceEnd ? -1 : 1;
    if (a->iKind != b->iKind) return a->iKind < b->iKind ? -1 : 1;
    if (a->iContentStart != b->iContentStart) return a->iContentStart < b->iContentStart ? -1 : 1;
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
    span->syntax_index = DOC_NONE; span->syntax_flags = 0;
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
        if (((MD_SPAN_A_DETAIL*)detail)->is_autolink) span->syntax_flags |= XUI_DOC_SYNTAX_AUTOLINK;
        span->href = doc_md_attribute(c, &((MD_SPAN_A_DETAIL*)detail)->href);
        span->title = doc_md_attribute(c, &((MD_SPAN_A_DETAIL*)detail)->title);
        break;
    case MD_SPAN_IMG:
        d.iKind = XUI_DOC_IMAGE; span->owns = 1;
        if (span->marks & XUI_DOC_LINK) {
            d.tAttributes.iMarks |= XUI_DOC_LINK;
            d.sLinkTarget = span->href ? span->href : "";
            d.sLinkTitle = span->title;
        }
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
static uint32_t doc_md_syntax_kind(MD_SPANTYPE type)
{
    switch (type) {
    case MD_SPAN_EM: return XUI_DOC_SYNTAX_EMPHASIS;
    case MD_SPAN_STRONG: return XUI_DOC_SYNTAX_STRONG;
    case MD_SPAN_U: case MD_SPAN_INS: return XUI_DOC_SYNTAX_UNDERLINE;
    case MD_SPAN_DEL: return XUI_DOC_SYNTAX_STRIKE;
    case MD_SPAN_CODE: return XUI_DOC_SYNTAX_CODE;
    case MD_SPAN_A: return XUI_DOC_SYNTAX_LINK;
    case MD_SPAN_IMG: return XUI_DOC_SYNTAX_IMAGE;
    case MD_SPAN_LATEXMATH: return XUI_DOC_SYNTAX_MATH;
    case MD_SPAN_LATEXMATH_DISPLAY: return XUI_DOC_SYNTAX_DISPLAY_MATH;
    case MD_SPAN_FOOTNOTE_REF: return XUI_DOC_SYNTAX_FOOTNOTE_REF;
    case MD_SPAN_MARK: return XUI_DOC_SYNTAX_HIGHLIGHT;
    case MD_SPAN_SUBSCRIPT: return XUI_DOC_SYNTAX_SUBSCRIPT;
    case MD_SPAN_SUPERSCRIPT: return XUI_DOC_SYNTAX_SUPERSCRIPT;
    default: return 0;
    }
}
static int doc_md_span_source(MD_SPANTYPE type, MD_OFFSET from, MD_OFFSET to, int enter,
    MD_OFFSET destination_start, MD_OFFSET destination_end, void* user)
{
    doc_md_context* c = user; doc_md_span* span = &c->spans[c->span_count];
    xui_doc_inline_syntax_t* syntax; uint64_t start = c->parser_offset + from, end = c->parser_offset + to, i;
    if (c->result) return c->result;
    if (!c->span_count || start > end || end > c->size) return c->result = XUI_DOC_ERROR_FORMAT;
    if (enter) {
        if (c->syntax_count == c->syntax_capacity) {
            uint64_t capacity = c->syntax_capacity ? c->syntax_capacity * 2 : 16;
            if (capacity > c->build.draft->allocator->max_nodes) capacity = c->build.draft->allocator->max_nodes;
            if (capacity <= c->syntax_count || capacity > SIZE_MAX / sizeof(*syntax)) return c->result = XUI_DOC_ERROR_LIMIT;
            syntax = doc_realloc(c->build.draft->allocator, c->syntax, (size_t)capacity * sizeof(*syntax));
            if (!syntax) return c->result = XUI_ERROR_OUT_OF_MEMORY;
            c->syntax = syntax; c->syntax_capacity = capacity;
        }
        span->syntax_index = c->syntax_count++; syntax = &c->syntax[span->syntax_index];
        memset(syntax, 0, sizeof(*syntax)); syntax->iSize = sizeof(*syntax); syntax->iKind = doc_md_syntax_kind(type);
        syntax->iParentIndex = c->spans[c->span_count - 1].syntax_index; syntax->iDefinitionIndex = DOC_NONE;
        syntax->iSourceStart = start; syntax->iContentStart = end;
        syntax->iContentEnd = syntax->iSourceEnd = DOC_NONE; syntax->iFlags = span->syntax_flags;
        /* The parser's resolved destination identifies the chosen definition,
         * including Unicode-folded labels and duplicate first-definition wins. */
        if ((type == MD_SPAN_A || type == MD_SPAN_IMG || type == MD_SPAN_FOOTNOTE_REF) && destination_start != UINT_MAX) {
            for (i = 0; i < doc_seq_size(c->build.draft->references); i += sizeof(xui_doc_reference_definition_t)) {
                xui_doc_reference_definition_t definition;
                doc_seq_read(c->build.draft->references, i, &definition, sizeof(definition));
                if ((type == MD_SPAN_FOOTNOTE_REF ?
                        definition.iKind == XUI_DOC_REFERENCE_FOOTNOTE &&
                        definition.iSourceStart == c->parser_offset + destination_start &&
                        definition.iSourceEnd == c->parser_offset + destination_end :
                        definition.iKind == XUI_DOC_REFERENCE_LINK &&
                        definition.iDestinationStart == c->parser_offset + destination_start &&
                        definition.iDestinationEnd == c->parser_offset + destination_end)) {
                    syntax->iDefinitionIndex = i / sizeof(definition); break;
                }
            }
        }
    } else {
        if (span->syntax_index >= c->syntax_count) return c->result = XUI_DOC_ERROR_FORMAT;
        syntax = &c->syntax[span->syntax_index];
        if (syntax->iKind != doc_md_syntax_kind(type) || syntax->iContentStart > start) return c->result = XUI_DOC_ERROR_FORMAT;
        syntax->iContentEnd = start; syntax->iSourceEnd = end;
        if (type == MD_SPAN_FOOTNOTE_REF && span->object) {
            doc_node* node = doc_index_get(c->build.draft->index, span->object); uint64_t bytes = doc_seq_size(node->text);
            int exact = bytes == start - syntax->iContentStart && doc_seq_equal_bytes(node->text, 0, c->source + syntax->iContentStart, bytes);
            doc_md_range(c, node->id, syntax->iContentStart, start, exact);
            return doc_md_segment(c, node->id, 0, bytes, syntax->iContentStart, start,
                exact ? XUI_DOC_SOURCE_DIRECT : XUI_DOC_SOURCE_NORMALIZED);
        }
        /* ![] has no text callback. Keep its zero-width alt source position so
         * inserting alt text does not serialize the surrounding paragraph. */
        if (type == MD_SPAN_IMG && span->object && syntax->iContentStart == start) {
            doc_node* node = doc_index_get(c->build.draft->index, span->object);
            if (node && node->source_start == DOC_NONE && !doc_seq_size(node->text))
                doc_md_range(c, node->id, start, start, 1);
        }
    }
    return XUI_OK;
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
static int doc_md_text_scope(MD_TEXTTYPE type, MD_OFFSET beg, MD_OFFSET end, int enter, void* user)
{
    doc_md_context* c = user;
    uint64_t start = c->parser_offset + beg, finish = c->parser_offset + end;
    if (c->result) return c->result;
    if (type != MD_TEXT_HTML || beg >= end || c->parser_offset > c->size || end > c->size - c->parser_offset ||
        (enter && c->html_scope) || (!enter && (!c->html_scope || c->html_scope_start != start || c->html_scope_end != finish)))
        return c->result = XUI_DOC_ERROR_FORMAT;
    c->html_scope = enter;
    if (enter) { c->html_scope_start = start; c->html_scope_end = finish; }
    c->html_node = c->html_parent = 0;
    return XUI_OK;
}
static int doc_md_text_source(MD_TEXTTYPE type, MD_TEXTTYPE content_type, MD_OFFSET beg, MD_OFFSET end, MD_SIZE size, void* user)
{
    doc_md_context* c = user;
    if (c->result) return c->result;
    if (c->pending_text_source || !size || beg > end || c->parser_offset > c->size ||
        end > c->size - c->parser_offset) return c->result = XUI_DOC_ERROR_FORMAT;
    c->pending_text_type = type; c->pending_text_size = size;
    c->pending_content_type = content_type;
    c->pending_text_start = c->parser_offset + beg; c->pending_text_end = c->parser_offset + end;
    c->pending_text_source = 1;
    return XUI_OK;
}
static int doc_md_text(MD_TEXTTYPE type, const MD_CHAR* text, MD_SIZE size, void* user)
{
    doc_md_context* c = user;
    doc_md_span* span = &c->spans[c->span_count];
    xui_doc_node_desc_t d = {0};
    uint64_t id = span->object, parent, start = DOC_NONE, end = DOC_NONE;
    uint64_t bytes = size, text_start = 0;
    char decoded[8];
    unsigned n, segment_kind = XUI_DOC_SOURCE_DIRECT;
    int exact = 1, result;
    int projected_break = (type == MD_TEXT_BR || type == MD_TEXT_SOFTBR) && span->object;
    MD_TEXTTYPE content_type = type;
    uintptr_t ptr = (uintptr_t)text, base = (uintptr_t)c->source;
    if (type == MD_TEXT_BR || type == MD_TEXT_SOFTBR) {
        c->last_break_id = 0;
        c->last_break_type = type;
    }
    if (c->pending_text_source) {
        if (type != c->pending_text_type || size != c->pending_text_size)
            return c->result = XUI_DOC_ERROR_FORMAT;
        c->pending_text_source = 0;
        content_type = c->pending_content_type;
        start = c->pending_text_start; end = c->pending_text_end;
        c->cursor = end; exact = 0; segment_kind = XUI_DOC_SOURCE_NORMALIZED;
    } else if (ptr >= base && ptr - base <= c->size && size <= c->size - (ptr - base)) {
        start = ptr - base; end = start + size;
        if (start && type == MD_TEXT_NORMAL && size && ispunct((unsigned char)text[0])) {
            uint64_t p = start;
            while (p && c->source[p - 1] == '\\') p--;
            if ((start - p) & 1) { start--; exact = 0; segment_kind = XUI_DOC_SOURCE_ESCAPE; }
        }
        c->cursor = end;
    } else {
        exact = 0; segment_kind = XUI_DOC_SOURCE_NORMALIZED;
    }
    if (type == MD_TEXT_ENTITY) {
        n = doc_md_entity(text, size, decoded);
        if (n) { text = decoded; bytes = n; exact = 0; segment_kind = XUI_DOC_SOURCE_ENTITY; }
    } else if (type == MD_TEXT_NULLCHAR) {
        bytes = doc_md_utf8(0xfffd, decoded); text = decoded; exact = 0; segment_kind = XUI_DOC_SOURCE_NORMALIZED;
    }
    if (projected_break && (bytes != 1 || text[0] != '\n')) return c->result = XUI_DOC_ERROR_FORMAT;
    parent = doc_md_inline_parent(c);
    if (!parent) return c->result;
    if (c->html_scope && content_type == MD_TEXT_HTML && !id && c->html_node) {
        if (parent != c->html_parent) return c->result = XUI_DOC_ERROR_FORMAT;
        id = c->html_node;
    }
    if (!id) {
        doc_node* p = doc_index_get(c->build.draft->index, parent);
        if (doc_text_kind(p->kind)) id = parent;
    }
    if (id) {
        doc_node* p = doc_index_get(c->build.draft->index, id);
        uint64_t len = doc_seq_size(p->text);
        text_start = len;
        result = doc_txn_text(&c->build, id, len, len, text, bytes);
        if (result != XUI_OK) return c->result = result;
    } else {
        d.iSize = sizeof(d); d.iKind = XUI_DOC_TEXT;
        if (type == MD_TEXT_BR) d.iKind = XUI_DOC_HARD_BREAK;
        else if (type == MD_TEXT_SOFTBR) d.iKind = XUI_DOC_SOFT_BREAK;
        else if (content_type == MD_TEXT_HTML) d.iKind = XUI_DOC_HTML;
        if (doc_text_kind(d.iKind)) { d.sText = text; d.iTextBytes = bytes; }
        d.tAttributes.iMarks = span->marks;
        if (span->marks & XUI_DOC_LINK) { d.sResource = span->href; d.sTitle = span->title; }
        if (doc_md_add(c, parent, &d, &id) != XUI_OK) return c->result;
    }
    if (c->html_scope && content_type == MD_TEXT_HTML && !c->html_node) {
        doc_node* html = doc_index_get(c->build.draft->index, id);
        if (html && html->kind == XUI_DOC_HTML) { c->html_node = id; c->html_parent = parent; }
    }
    if ((type == MD_TEXT_BR || type == MD_TEXT_SOFTBR) && id != parent) {
        doc_node* break_node = doc_index_get(c->build.draft->index, id);
        if (break_node && (break_node->kind == XUI_DOC_SOFT_BREAK ||
            break_node->kind == XUI_DOC_HARD_BREAK)) c->last_break_id = id;
    }
    if (projected_break) {
        c->last_break_id = id;
        c->last_break_text_offset = text_start;
        return XUI_OK;
    }
    doc_md_range(c, id, start, end, exact);
    if (id != parent) doc_md_range(c, parent, start, end, 0);
    if (segment_kind == XUI_DOC_SOURCE_ESCAPE && bytes > 1 && end - start == bytes + 1) {
        result = doc_md_segment(c, id, text_start, 1, start, start + 2, XUI_DOC_SOURCE_ESCAPE);
        if (result != XUI_OK) return result;
        return doc_md_segment(c, id, text_start + 1, bytes - 1, start + 2, end, XUI_DOC_SOURCE_DIRECT);
    }
    return doc_md_segment(c, id, text_start, bytes, start, end, segment_kind);
}
static int doc_md_raw_line_end(const char* source, uint64_t bytes, uint64_t start,
    const atomic_int* cancellation, uint64_t* content_end, uint64_t* next)
{
    uint64_t end = start;
    int result = cancellation ? atomic_load(cancellation) : XUI_OK;
    if (result != XUI_OK) return result;
    while (end < bytes && source[end] != '\r' && source[end] != '\n') {
        if ((end & 4095) == 0 && cancellation && (result = atomic_load(cancellation)) != XUI_OK) return result;
        end++;
    }
    *content_end = end;
    if (end < bytes && source[end] == '\r') end++;
    if (end < bytes && source[end] == '\n') end++;
    *next = end; return XUI_OK;
}
int doc_markdown_front_matter_range(const char* source, uint64_t bytes, uint64_t offset,
    const atomic_int* cancellation, doc_front_matter_range* out)
{
    uint64_t first_end, content_start, line, end, next;
    int result;
    out->content_start = out->content_end = DOC_NONE; out->syntax_end = offset;
    result = cancellation ? atomic_load(cancellation) : XUI_OK;
    if (result != XUI_OK) return result;
    if (offset > bytes || bytes - offset < 4 || memcmp(source + offset, "---", 3) ||
        (source[offset + 3] != '\r' && source[offset + 3] != '\n')) return XUI_OK;
    result = doc_md_raw_line_end(source, bytes, offset, cancellation, &first_end, &content_start);
    if (result != XUI_OK) return result;
    if (first_end - offset != 3 || memcmp(source + offset, "---", 3) != 0 ||
        content_start == first_end) return XUI_OK;
    for (line = content_start; line < bytes; line = next) {
        result = doc_md_raw_line_end(source, bytes, line, cancellation, &end, &next);
        if (result != XUI_OK) return result;
        if (end - line == 3 && (!memcmp(source + line, "---", 3) || !memcmp(source + line, "...", 3))) {
            out->content_start = content_start; out->content_end = line; out->syntax_end = next;
            break;
        }
    }
    return XUI_OK;
}
static uint64_t doc_md_front_matter(doc_md_context* c, uint64_t offset)
{
    doc_front_matter_range range;
    xui_doc_node_desc_t d = {0};
    uint64_t id;
    c->result = doc_markdown_front_matter_range(c->source, c->size, offset, c->build.cancellation, &range);
    if (c->result != XUI_OK || range.content_start == DOC_NONE) return offset;
    d.iSize = sizeof(d); d.iKind = XUI_DOC_FRONT_MATTER;
    d.sText = c->source + range.content_start; d.iTextBytes = range.content_end - range.content_start;
    if (doc_md_add(c, DOC_ROOT, &d, &id) != XUI_OK) return offset;
    doc_md_range(c, id, range.content_start, range.content_end, 1);
    if (doc_md_segment(c, id, 0, d.iTextBytes, range.content_start, range.content_end, XUI_DOC_SOURCE_DIRECT) != XUI_OK) return offset;
    doc_index_get(c->build.draft->index, id)->syntax_start = offset;
    doc_index_get(c->build.draft->index, id)->syntax_end = range.syntax_end;
    {
        doc_node* node = doc_index_get(c->build.draft->index, id);
        node->marker_kind = XUI_DOC_BLOCK_SYNTAX_FRONT_MATTER;
        node->marker_primary_start = 0; node->marker_primary_end = 3;
        node->marker_secondary_start = (uint32_t)(range.content_end - offset);
        node->marker_secondary_end = node->marker_secondary_start + 3;
    }
    return range.syntax_end;
}
static int doc_md_anchor_subtree(doc_state* state, uint64_t id, uint64_t ordinal,
    const atomic_int* cancellation)
{
    doc_node* node = doc_index_get(state->index, id);
    uint64_t i;
    int cancelled = cancellation ? atomic_load(cancellation) : XUI_OK;
    if (cancelled != XUI_OK) return cancelled;
    node->block_ordinal = ordinal; node->block_shift_base = 0;
    for (i = 0; i < doc_seq_size(node->children); i++) {
        int result = doc_md_anchor_subtree(state, doc_seq_get_id(node->children, i), ordinal, cancellation);
        if (result != XUI_OK) return result;
    }
    return XUI_OK;
}
/* An ordered top-level syntax range may bound source-position search only if
 * every hit in its subtree, including resolved provenance, stays inside it.
 * References and other cross-block constructs deliberately disable the index. */
static int doc_md_source_candidates_bounded(doc_state* state, uint64_t id, uint64_t start, uint64_t end)
{
    doc_node* node = doc_index_get(state->index, id);
    doc_node_source_range range;
    uint64_t i, count;
    if (!node) return 0;
    doc_node_source_range_get(state, node, &range);
    count = doc_seq_size(node->children);
    if (doc_text_kind(node->kind)) {
        if (range.source_start != DOC_NONE || range.source_end != DOC_NONE) {
            if (range.source_start == DOC_NONE || range.source_end == DOC_NONE ||
                range.source_start < start || range.source_end > end || range.source_start > range.source_end) return 0;
        }
        for (i = 0; i < doc_seq_size(node->provenance); i += sizeof(xui_doc_source_segment_t)) {
            xui_doc_source_segment_t segment;
            if (doc_seq_read(node->provenance, i, &segment, sizeof(segment)) != XUI_OK) return 0;
            doc_source_resolve_segment(state, node, &segment);
            if (segment.iSourceStart == DOC_NONE && segment.iSourceEnd == DOC_NONE) continue;
            if (segment.iSourceStart == DOC_NONE || segment.iSourceEnd == DOC_NONE ||
                segment.iSourceStart < start || segment.iSourceEnd > end ||
                segment.iSourceStart > segment.iSourceEnd) return 0;
        }
    }
    if (!count && !doc_seq_size(node->text) && range.syntax_start != DOC_NONE &&
        (range.syntax_start < start || range.syntax_end > end || range.syntax_start > range.syntax_end)) return 0;
    for (i = 0; i < count; i++)
        if (!doc_md_source_candidates_bounded(state, doc_seq_get_id(node->children, i), start, end)) return 0;
    return 1;
}
int doc_markdown_source_block_valid(doc_state* state, uint64_t index, uint64_t limit)
{
    doc_node* root = doc_index_get(state->index, DOC_ROOT);
    doc_node* block;
    doc_node_source_range range, neighbor;
    uint64_t count = root ? doc_seq_size(root->children) : 0;
    if (index >= count) return 0;
    block = doc_index_get(state->index, doc_seq_get_id(root->children, index));
    if (!block) return 0;
    doc_node_source_range_get(state, block, &range);
    if (range.syntax_start == DOC_NONE || range.syntax_end == DOC_NONE ||
        range.syntax_start > range.syntax_end || range.syntax_end > limit) return 0;
    if (index) {
        doc_node* previous = doc_index_get(state->index, doc_seq_get_id(root->children, index - 1));
        if (!previous) return 0;
        doc_node_source_range_get(state, previous, &neighbor);
        if (neighbor.syntax_end > range.syntax_start) return 0;
    }
    if (index + 1 < count) {
        doc_node* next = doc_index_get(state->index, doc_seq_get_id(root->children, index + 1));
        if (!next) return 0;
        doc_node_source_range_get(state, next, &neighbor);
        if (range.syntax_end > neighbor.syntax_start) return 0;
    }
    return doc_md_source_candidates_bounded(state, block->id, range.syntax_start, range.syntax_end);
}
int doc_markdown_source_blocks_ordered(doc_state* state, uint64_t limit)
{
    doc_node* root = doc_index_get(state->index, DOC_ROOT);
    uint64_t i, count = root ? doc_seq_size(root->children) : 0;
    for (i = 0; i < count; i++)
        if (!doc_markdown_source_block_valid(state, i, limit)) return 0;
    return 1;
}
/* Offsets in the returned tree are relative to the window. Only a real
 * document start recognizes BOM/front matter; the caller relocates fragments. */
int doc_markdown_parse_input(xui_document_transaction t, uint64_t start,
    const char* source, uint64_t parse_size, doc_state** out, uint64_t* ordinary_syntax)
{
    doc_md_context c;
    MD_PARSER parser;
    doc_state* state;
    uint64_t offset = 0;
    unsigned i;
    int result = doc_txn_check(t, 0);
    if (result != XUI_OK) return result;
    *out = NULL;
    if (!source) return XUI_ERROR_INVALID_ARGUMENT;
    if (parse_size > UINT_MAX || parse_size >= SIZE_MAX) return XUI_DOC_ERROR_LIMIT;
    atomic_fetch_add(&t->draft->allocator->markdown_parsed_bytes, parse_size);
    state = doc_state_new(t->draft->allocator, XUI_DOCUMENT_MARKDOWN);
    if (!state) return XUI_ERROR_OUT_OF_MEMORY;
    {
        doc_node* root = doc_index_get(state->index, DOC_ROOT);
        xui_doc_attributes_t attrs = *root->attrs;
        attrs.sLanguage = doc_index_get(t->draft->index, DOC_ROOT)->attrs->sLanguage;
        result = doc_node_set_attrs(state->allocator, root, &attrs);
        if (result != XUI_OK) { doc_state_release(state); return result; }
    }
    state->source = t->draft->source; doc_seq_retain(state->source);
    state->source_open_brackets = t->draft->source_open_brackets;
    state->source_storage_bytes = t->draft->source_storage_bytes;
    state->dialect = t->draft->dialect;
    memset(&c, 0, sizeof(c)); c.source = source; c.size = parse_size; c.spans[0].syntax_index = DOC_NONE;
    c.build.document = t->document; c.build.draft = state; c.build.domain = XUI_DOC_SEMANTIC; c.build.parsing = 1;
    c.build.cancellation = t->cancellation;
    if (!start && parse_size >= 3 && memcmp(source, "\xef\xbb\xbf", 3) == 0) offset = 3;
    if (!start && state->dialect == XUI_MD_EXTENDED) offset = doc_md_front_matter(&c, offset);
    c.cursor = c.parser_offset = offset;
    memset(&parser, 0, sizeof(parser));
    parser.flags = doc_md4c_dialect_flags(state->dialect);
    parser.enter_block = doc_md_enter_block; parser.leave_block = doc_md_leave_block;
    parser.enter_span = doc_md_enter_span; parser.leave_span = doc_md_leave_span; parser.text = doc_md_text;
    result = c.result ? c.result : doc_md4c_parse(t->draft->allocator, source + offset, (MD_SIZE)(parse_size - offset), &parser,
        doc_md_block_source, doc_md_block_markers, doc_md_fence_info, doc_md_heading_content, doc_md_break_source,
        doc_md_quote_prefixes, doc_md_list_indents, doc_md_code_indents,
        doc_md_table_token, doc_md_reference_source, doc_md_reference_values, doc_md_footnote_source, doc_md_candidate_source, doc_md_span_source,
        doc_md_text_source, doc_md_text_scope, t->cancellation, &state->markdown_footnotes, &c);
    if (!result && (c.pending_text_source || c.html_scope)) result = XUI_DOC_ERROR_FORMAT;
    if (result || c.result || c.build.error) result = c.result ? c.result : (c.build.error ? c.build.error :
        (result == XUI_ERROR_OUT_OF_MEMORY || result == XUI_DOC_ERROR_CANCELLED || result == XUI_DOC_ERROR_STALE ? result : XUI_DOC_ERROR_FORMAT));
    if (result == XUI_OK && (doc_seq_size(state->references) % sizeof(xui_doc_reference_definition_t) ||
        doc_seq_size(state->reference_values) != doc_seq_size(state->references) / sizeof(xui_doc_reference_definition_t)))
        result = XUI_DOC_ERROR_SCHEMA;
    for (i = 1; i <= c.span_count; i++) if (c.spans[i].owns) { doc_free(c.spans[i].href); doc_free(c.spans[i].title); }
    if (result == XUI_OK && c.syntax_count) {
        state->inline_syntax = doc_seq_text(state->allocator, (const char*)c.syntax, c.syntax_count * sizeof(*c.syntax));
        if (!state->inline_syntax) result = XUI_ERROR_OUT_OF_MEMORY;
    }
    if (result == XUI_OK && c.candidate_count) {
        uint64_t read, unique = 0;
        qsort(c.candidates, (size_t)c.candidate_count, sizeof(*c.candidates), doc_md_candidate_order);
        for (read = 0; read < c.candidate_count; read++) {
            if (unique && !memcmp(&c.candidates[unique - 1], &c.candidates[read], sizeof(*c.candidates))) continue;
            c.candidates[unique++] = c.candidates[read];
        }
        c.candidate_count = unique;
        state->reference_candidates = doc_seq_text(state->allocator, (const char*)c.candidates,
            c.candidate_count * sizeof(*c.candidates));
        if (!state->reference_candidates) result = XUI_ERROR_OUT_OF_MEMORY;
    }
    if (result == XUI_OK) {
        doc_node* root = doc_index_get(state->index, DOC_ROOT);
        uint64_t at;
        for (at = 0; at < doc_seq_size(root->children) && result == XUI_OK; at++)
            result = doc_md_anchor_subtree(state, doc_seq_get_id(root->children, at), at, t->cancellation);
        if (result == XUI_OK) state->source_blocks_indexed = doc_markdown_source_blocks_ordered(state, parse_size);
    }
    if (ordinary_syntax) *ordinary_syntax = c.footnote_section ? c.ordinary_syntax_count : c.syntax_count;
    doc_free(c.syntax); doc_free(c.candidates); doc_free(c.table_tokens);
    if (result != XUI_OK) { doc_state_release(state); return result; }
    *out = state; return XUI_OK;
}
int doc_markdown_parse_window_with_suffix(xui_document_transaction t,
    uint64_t start, uint64_t end, const char* suffix, uint64_t suffix_bytes,
    doc_state** out)
{
    uint64_t size, parse_size;
    char* source; int result = doc_txn_check(t, 0);
    if (result != XUI_OK) return result;
    *out = NULL;
    if ((suffix_bytes && !suffix) || end < start) return XUI_ERROR_INVALID_ARGUMENT;
    size = end - start;
    if (size > UINT_MAX || suffix_bytes > UINT_MAX - size || size + suffix_bytes >= SIZE_MAX)
        return XUI_DOC_ERROR_LIMIT;
    parse_size = size + suffix_bytes;
    source = doc_alloc(t->draft->allocator, (size_t)parse_size + 1);
    if (!source) return XUI_ERROR_OUT_OF_MEMORY;
    result = doc_seq_read(t->draft->source, start, source, size);
    if (result == XUI_OK) {
        if (suffix_bytes) memcpy(source + size, suffix, (size_t)suffix_bytes);
        source[parse_size] = 0;
        result = doc_markdown_parse_input(t, start, source, parse_size, out, NULL);
    }
    doc_free(source); return result;
}
int doc_markdown_parse_window(xui_document_transaction t, uint64_t start, uint64_t end, doc_state** out)
{
    return doc_markdown_parse_window_with_suffix(t, start, end, NULL, 0, out);
}
int doc_markdown_parse(xui_document_transaction t)
{
    doc_state* state = NULL;
    uint64_t size = doc_seq_size(t->draft->source);
    int result = doc_txn_check(t, 0);
    if (result != XUI_OK) return result;
    if (size > UINT_MAX) return doc_txn_fail(t, XUI_DOC_ERROR_LIMIT);
    atomic_fetch_add(&t->draft->allocator->markdown_parses, 1);
    result = doc_markdown_incremental(t, &state);
    if (result == XUI_ERROR_UNSUPPORTED) {
        result = doc_markdown_parse_window(t, 0, size, &state);
        if (result == XUI_OK) result = doc_reference_values_reuse(t->draft, state, t->base, t->cancellation);
        if (result == XUI_OK && t->domain != XUI_DOC_SEMANTIC) result = doc_markdown_reconcile(t, &state);
    } else if (result == XUI_OK && t->base &&
        t->base->reference_values != t->draft->reference_values &&
        state->reference_values != t->draft->reference_values) {
        /* Incremental value replacement also needs the immutable origin:
         * earlier SOURCE patches can temporarily remove a later moved value. */
        result = doc_reference_values_reuse(t->draft, state, t->base, t->cancellation);
    }
    /* Owner-side Prepare only builds immutable source paths. Coverage scans
     * and copies run here, on the parsing worker, before any publication. The
     * retained-payload bound accumulates new insertions and never subtracts
     * removed bytes until a scan proves what storage is still reachable. */
    if (result == XUI_OK && state->source_storage_bytes > 4096 &&
        size <= (state->source_storage_bytes - 1) / 4)
        result = doc_seq_compact_bytes(state->allocator, &state->source,
            t->cancellation, &state->source_storage_bytes);
    if (result == XUI_OK) result = doc_txn_check(t, 0);
    if (result == XUI_OK && (state->node_count > t->draft->allocator->max_nodes || state->text_bytes > t->draft->allocator->max_bytes)) result = XUI_DOC_ERROR_LIMIT;
    if (result != XUI_OK) { doc_state_release(state); return doc_txn_fail(t, result); }
    doc_state_release(t->draft); t->draft = state; t->parse_op_start = t->count; return XUI_OK;
}

static int doc_source_removed_brackets(xui_document_transaction t,
    uint64_t start, uint64_t end, uint64_t* count)
{
    char buffer[4096]; uint64_t at = start;
    *count = 0;
    while (at < end) {
        uint64_t size = end - at < sizeof(buffer) ? end - at : sizeof(buffer);
        int result = doc_txn_check(t, 0);
        if (result != XUI_OK) return result;
        result = doc_seq_read(t->draft->source, at, buffer, size);
        if (result != XUI_OK) return result;
        *count += doc_source_open_bracket_count(buffer, size);
        at += size;
    }
    return XUI_OK;
}
int doc_txn_source_patch(xui_document_transaction t, uint64_t start, uint64_t end, const char* text, uint64_t bytes, int parse)
{
    doc_sequence *insert, *source = NULL, *before_source = NULL;
    uint64_t size, removed_brackets, added_brackets, next_brackets;
    xui_doc_operation_t op = {0};
    int result = doc_txn_check(t, 0);
    if (result != XUI_OK) return result;
    if (t->draft->profile != XUI_DOCUMENT_MARKDOWN) return doc_txn_fail(t, XUI_ERROR_UNSUPPORTED);
    size = doc_seq_size(t->draft->source);
    if (start > end || end > size) return doc_txn_fail(t, XUI_ERROR_INVALID_ARGUMENT);
    if (!doc_utf8(text, bytes) || !doc_seq_boundary(t->draft->source, start) || !doc_seq_boundary(t->draft->source, end)) return doc_txn_fail(t, XUI_DOC_ERROR_UTF8);
    if (bytes > t->draft->allocator->max_bytes || size - (end - start) > t->draft->allocator->max_bytes - bytes) return doc_txn_fail(t, XUI_DOC_ERROR_LIMIT);
    if (end - start == bytes && doc_seq_equal_bytes(t->draft->source, start, text, bytes)) return XUI_OK;
    result = doc_source_removed_brackets(t, start, end, &removed_brackets);
    if (result != XUI_OK) return doc_txn_fail(t, result);
    added_brackets = doc_source_open_bracket_count(text, bytes);
    if (removed_brackets > t->draft->source_open_brackets ||
        added_brackets > UINT64_MAX - (t->draft->source_open_brackets - removed_brackets))
        return doc_txn_fail(t, XUI_DOC_ERROR_LIMIT);
    next_brackets = t->draft->source_open_brackets - removed_brackets + added_brackets;
    if (bytes > UINT64_MAX - t->draft->source_storage_bytes)
        return doc_txn_fail(t, XUI_DOC_ERROR_LIMIT);
    insert = doc_seq_text(t->draft->allocator, text, bytes);
    if (bytes && !insert) return doc_txn_fail(t, XUI_ERROR_OUT_OF_MEMORY);
    result = doc_seq_replace(t->draft->allocator, t->draft->source, start, end, insert, &source);
    doc_seq_release(insert);
    if (result != XUI_OK) return doc_txn_fail(t, result);
    if (parse) { before_source = t->draft->source; doc_seq_retain(before_source); }
    doc_seq_release(t->draft->source); t->draft->source = source;
    t->draft->source_open_brackets = next_brackets;
    t->draft->source_storage_bytes += bytes;
    op.iKind = XUI_DOC_OP_SOURCE; op.iFlags = XUI_DOC_CHANGE_SOURCE | XUI_DOC_CHANGE_TEXT | XUI_DOC_CHANGE_STRUCTURE;
    op.iOffset = start; op.iOldLength = end - start; op.iNewLength = bytes;
    result = doc_txn_op(t, &op);
    if (result == XUI_OK && parse) {
        t->source_before_patch = before_source;
        result = doc_markdown_parse(t);
        t->source_before_patch = NULL;
    }
    doc_seq_release(before_source);
    return result;
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
static int doc_md_open_fence(doc_state* state, doc_node* node,
    unsigned char* marker, uint64_t* needed)
{
    doc_node_source_range range;
    char chunk[1024]; uint64_t offset, size = doc_seq_size(state->source), run = 0;
    unsigned spaces = 0;
    *marker = 0; *needed = 3;
    doc_node_source_range_get(state, node, &range);
    offset = range.syntax_start;
    if (offset == DOC_NONE || offset >= size) return XUI_OK;
    while (offset < size) {
        uint64_t count = size - offset < sizeof(chunk) ? size - offset : sizeof(chunk), i;
        int result = doc_seq_read(state->source, offset, chunk, count);
        if (result != XUI_OK) return result;
        for (i = 0; i < count; i++) {
            unsigned char c = (unsigned char)chunk[i];
            if (c == '\n' || c == '\r') goto done;
            if (!*marker) {
                if (c == ' ' && spaces < 3) { spaces++; continue; }
                if (c != '~' && c != '`') return XUI_OK;
                *marker = c; run = 1;
            } else if (c == *marker) run++;
            else goto done;
        }
        offset += count;
    }
done:
    if (run >= 3) *needed = run;
    else *marker = 0;
    return XUI_OK;
}
static int doc_md_has_fence_closer(doc_sequence* source, unsigned char marker,
    uint64_t needed, int* found)
{
    char chunk[1024]; uint64_t offset = 0, size = doc_seq_size(source);
    unsigned state = 0, spaces = 0; uint64_t run = 0; unsigned char line_marker = 0;
    *found = 0;
    while (offset < size) {
        uint64_t count = size - offset < sizeof(chunk) ? size - offset : sizeof(chunk), i;
        int result = doc_seq_read(source, offset, chunk, count);
        if (result != XUI_OK) return result;
        for (i = 0; i < count; i++) {
            unsigned char c = (unsigned char)chunk[i];
            if (c == '\n' || c == '\r') {
                if ((state == 1 && run >= needed) || state == 2) { *found = 1; return XUI_OK; }
                state = spaces = run = line_marker = 0; continue;
            }
            if (!state) {
                if (c == ' ' && spaces < 3) spaces++;
                else if ((c == '~' || c == '`') && (!marker || c == marker)) {
                    state = 1; line_marker = c; run = 1;
                }
                else state = 3;
            } else if (state == 1) {
                if (c == line_marker) run++;
                else if ((c == ' ' || c == '\t') && run >= needed) state = 2;
                else state = 3;
            } else if (state == 2 && c != ' ' && c != '\t') state = 3;
        }
        offset += count;
    }
    if ((state == 1 && run >= needed) || state == 2) *found = 1;
    return XUI_OK;
}
static uint64_t doc_md_edit_scalar(const char* text, uint64_t bytes,
    uint64_t at, unsigned* scalar)
{
    unsigned char first = (unsigned char)text[at];
    uint64_t width = first < 0x80 ? 1 : first < 0xe0 ? 2 : first < 0xf0 ? 3 : 4;
    uint64_t j; unsigned value;
    if (width > bytes - at) return 0;
    value = width == 1 ? first : first & (0x7fu >> width);
    for (j = 1; j < width; j++)
        value = (value << 6) | ((unsigned char)text[at + j] & 0x3fu);
    *scalar = value;
    return width;
}
int doc_markdown_text(xui_document_transaction t, uint64_t id, uint64_t start, uint64_t end, const char* text, uint64_t bytes, xui_doc_position_t* caret)
{
    doc_node* p = doc_index_get(t->draft->index, id);
    doc_node_source_range range;
    struct xui_doc_transaction_t expected = {0};
    char *semantic = NULL, *escaped = NULL;
    uint64_t source_start, source_end, n = 0, i, last_scalar = bytes, inserted = bytes;
    xui_doc_position_t target = {0};
    int literal, protect_leading, protect_trailing, result;
    result = doc_markdown_shadow_begin(t, &expected); if (result != XUI_OK) return result;
    result = doc_txn_text(&expected, id, start, end, text, bytes);
    if (result != XUI_OK) goto done;
    target.iSize = sizeof(target); target.iDocumentId = t->document->identity; target.iRevision = t->base_revision;
    target.iNodeId = id; target.iKind = XUI_DOC_POSITION_TEXT; target.iOffset = start + inserted; target.iAffinity = XUI_DOC_AFTER;
    if (!expected.count) goto done;
    /* Empty text runs have no Markdown token. Removing only their content
     * leaves mark/link/code delimiters behind, which can become literal text
     * or bind to a neighbor. Rewrite the changed block against the complete
     * desired tree, retaining the existing caret and semantic checks. */
    if (p->kind == XUI_DOC_TEXT && !doc_seq_size(doc_index_get(expected.draft->index, id)->text)) {
        result = doc_markdown_apply_tree(t, expected.draft, NULL, NULL); goto accept;
    }
    if (p->kind == XUI_DOC_DIAGRAM || p->kind == XUI_DOC_CODE_BLOCK) {
        doc_node* changed = doc_index_get(expected.draft->index, id);
        unsigned char marker; uint64_t needed; int has_fence;
        result = doc_md_open_fence(t->draft, p, &marker, &needed);
        if (result != XUI_OK) goto done;
        result = doc_md_has_fence_closer(changed->text, marker, needed, &has_fence);
        if (result != XUI_OK) goto done;
        if (has_fence) { result = doc_markdown_apply_tree(t, expected.draft, NULL, NULL); goto accept; }
    }
    doc_node_source_range_get(t->draft, p, &range);
    if (range.source_start == DOC_NONE || range.source_end == DOC_NONE) {
        result = doc_markdown_apply_tree(t, expected.draft, NULL, NULL); goto accept;
    }
    source_start = range.source_start; source_end = range.source_end;
    /* Image text is Markdown alt content, not a literal code/HTML body. Its
     * punctuation must be escaped inside ![...], just like ordinary text. */
    literal = (p->kind != XUI_DOC_TEXT && p->kind != XUI_DOC_IMAGE) ||
        (p->attrs->iMarks & XUI_DOC_CODE);
    {
        uint64_t from, to; int left, right;
        if (doc_source_map_text(t->draft, p, start, XUI_DOC_AFTER, &from, &left) == XUI_OK && left == XUI_DOC_MAP_EXACT &&
            doc_source_map_text(t->draft, p, end, XUI_DOC_BEFORE, &to, &right) == XUI_OK && right == XUI_DOC_MAP_EXACT && (start == end || from <= to)) {
            source_start = from; source_end = start == end ? from : to;
            goto escape;
        }
    }
    if (p->source_exact && source_end - source_start == doc_seq_size(p->text)) {
        source_end = source_start + end; source_start += start;
    } else {
        doc_node* changed = doc_index_get(expected.draft->index, id);
        semantic = doc_seq_string(t->draft->allocator, changed->text); bytes = doc_seq_size(changed->text);
        if (!semantic) { result = XUI_ERROR_OUT_OF_MEMORY; goto done; }
        text = semantic;
    }
 escape:
    /* Horizontal Unicode whitespace next to a block or inline delimiter can
     * change the parsed structure or stop the delimiter from binding. Spell
     * only edited edge whitespace as an entity; CR/LF remain structural. */
    protect_leading = !literal && (semantic != NULL || start == 0);
    protect_trailing = !literal && (semantic != NULL || end == doc_seq_size(p->text));
    if (bytes > (SIZE_MAX - 32) / 2) { result = XUI_DOC_ERROR_LIMIT; goto done; }
    if (bytes) {
        last_scalar = bytes - 1;
        while (last_scalar && ((unsigned char)text[last_scalar] & 0xc0u) == 0x80u)
            last_scalar--;
    }
    escaped = doc_alloc(t->draft->allocator, (size_t)bytes * 2 + 32);
    if (!escaped) { result = XUI_ERROR_OUT_OF_MEMORY; goto done; }
    for (i = 0; i < bytes; i++) {
        if ((i == 0 && protect_leading) || (i == last_scalar && protect_trailing)) {
            unsigned scalar; uint64_t width = doc_md_edit_scalar(text, bytes, i, &scalar);
            if (!width) { result = XUI_DOC_ERROR_UTF8; goto done; }
            if (scalar != '\r' && scalar != '\n' &&
                doc_md4c_unicode_whitespace(scalar)) {
                char entity[16]; int length = snprintf(entity, sizeof(entity), "&#%u;", scalar);
                if (length <= 0 || (size_t)length >= sizeof(entity) ||
                    n > bytes * 2 + 32 - (uint64_t)length) {
                    result = XUI_DOC_ERROR_LIMIT; goto done;
                }
                memcpy(escaped + n, entity, (size_t)length);
                n += (uint64_t)length; i += width - 1; continue;
            }
        }
        if (!literal && doc_md_escapable((unsigned char)text[i])) escaped[n++] = '\\';
        escaped[n++] = text[i];
    }
    result = doc_txn_source(t, source_start, source_end, escaped, n);
accept:
    if (result == XUI_OK) result = doc_markdown_accept(t, &expected);
done:
    if (result == XUI_OK && caret) *caret = doc_markdown_resolve_caret(t, expected.draft, target);
    doc_state_release(expected.draft); doc_free(expected.ops); doc_free(escaped); doc_free(semantic);
    return result == XUI_OK ? result : doc_txn_fail(t, result);
}
/* A task marker belongs to the item's opening syntax line, which can be
 * earlier than its first semantic source byte when the item starts empty.
 * Inspect bounded chunks of that line, never a flattened document copy. */
static int doc_md_task_marker_line(doc_sequence* source,
    uint64_t start, uint64_t* marker)
{
    char chunk[4096];
    uint64_t at, size = doc_seq_size(source);
    char previous[2] = {0, 0};
    unsigned seen = 0;
    int result;
    if (start >= size) return XUI_ERROR_NOT_FOUND;
    for (at = start; at < size;) {
        uint64_t remaining = size - at;
        size_t bytes = remaining > sizeof(chunk) ? sizeof(chunk) : (size_t)remaining;
        size_t i;
        result = doc_seq_read(source, at, chunk, bytes);
        if (result != XUI_OK) return result;
        for (i = 0; i < bytes; i++) {
            char current = chunk[i];
            if (current == '\n' || current == '\r') return XUI_ERROR_NOT_FOUND;
            if (seen == 2 && previous[0] == '[' &&
                (previous[1] == ' ' || previous[1] == 'x' ||
                 previous[1] == 'X') && current == ']')
                { *marker = at + i - 2; return XUI_OK; }
            previous[0] = previous[1];
            previous[1] = current;
            if (seen < 2) seen++;
        }
        at += bytes;
    }
    return XUI_ERROR_NOT_FOUND;
}
static int doc_md_task_marker_offset(doc_sequence* source,
    uint64_t syntax_start, uint64_t source_start, uint64_t* marker)
{
    char chunk[4096];
    uint64_t line;
    int result;
    if (!source || !marker) return XUI_DOC_ERROR_SCHEMA;
    if (syntax_start != DOC_NONE) {
        result = doc_md_task_marker_line(source, syntax_start, marker);
        if (result != XUI_ERROR_NOT_FOUND) return result;
    }
    if (source_start == DOC_NONE || source_start > doc_seq_size(source))
        return XUI_ERROR_NOT_FOUND;
    line = source_start;
    while (line) {
        uint64_t begin = line > sizeof(chunk) ? line - sizeof(chunk) : 0;
        size_t bytes = (size_t)(line - begin), i;
        result = doc_seq_read(source, begin, chunk, bytes);
        if (result != XUI_OK) return result;
        for (i = bytes; i > 0; i--) {
            if (chunk[i - 1] == '\n' || chunk[i - 1] == '\r')
                return doc_md_task_marker_line(source, begin + i, marker);
        }
        line = begin;
    }
    return doc_md_task_marker_line(source, 0, marker);
}
int doc_markdown_attributes(xui_document_transaction t, uint64_t id, const xui_doc_attributes_t* attrs)
{
    doc_node* p = doc_index_get(t->draft->index, id);
    doc_node_source_range range;
    xui_doc_attributes_t expected;
    uint64_t start;
    int result;
    if (!p) return doc_txn_fail(t, XUI_ERROR_NOT_FOUND);
    doc_node_source_range_get(t->draft, p, &range);
    expected = *p->attrs;
    if (p->kind == XUI_DOC_TEXT) {
        xui_doc_range_t range;
        expected.iMarks = attrs->iMarks;
        if (!doc_attributes_equal(&expected, attrs)) return doc_txn_fail(t, XUI_ERROR_UNSUPPORTED);
        memset(&range, 0, sizeof(range)); range.tAnchor.iSize = range.tCaret.iSize = sizeof(range.tAnchor);
        range.tAnchor.iKind = range.tCaret.iKind = XUI_DOC_POSITION_TEXT;
        range.tAnchor.iDocumentId = range.tCaret.iDocumentId = t->document->identity;
        range.tAnchor.iRevision = range.tCaret.iRevision = t->base_revision;
        range.tAnchor.iNodeId = range.tCaret.iNodeId = id; range.tCaret.iOffset = doc_seq_size(p->text);
        return doc_markdown_marks(t, &range, attrs->iMarks & ~p->attrs->iMarks, p->attrs->iMarks & ~attrs->iMarks);
    }
    if (p->kind != XUI_DOC_LIST_ITEM || !(p->attrs->iFlags & XUI_DOC_TASK) ||
        (range.syntax_start == DOC_NONE && range.source_start == DOC_NONE)) {
        struct xui_doc_transaction_t shadow;
        result = doc_markdown_shadow_begin(t, &shadow); if (result != XUI_OK) return result;
        result = xuiDocumentTxnSetAttributes(&shadow, id, attrs);
        return doc_markdown_shadow_end(t, &shadow, result, NULL, NULL);
    }
    expected.iFlags = (expected.iFlags & ~XUI_DOC_CHECKED) | (attrs->iFlags & XUI_DOC_CHECKED);
    if (!doc_attributes_equal(&expected, attrs)) return doc_txn_fail(t, XUI_ERROR_UNSUPPORTED);
    result = doc_md_task_marker_offset(t->draft->source,
        range.syntax_start, range.source_start, &start);
    if (result != XUI_OK) return doc_txn_fail(t,
        result == XUI_ERROR_NOT_FOUND ? XUI_ERROR_UNSUPPORTED : result);
    {
        struct xui_doc_transaction_t shadow;
        result = doc_markdown_shadow_begin(t, &shadow); if (result != XUI_OK) return result;
        result = xuiDocumentTxnSetAttributes(&shadow, id, attrs);
        if (result == XUI_OK && shadow.count) result = doc_txn_source(t, start + 1, start + 2, attrs->iFlags & XUI_DOC_CHECKED ? "x" : " ", 1);
        if (result == XUI_OK && shadow.count) result = doc_markdown_accept(t, &shadow);
        doc_state_release(shadow.draft); doc_free(shadow.ops);
    }
    return result;
}
static int doc_markdown_marks_patch(xui_document_transaction t, const xui_doc_range_t* range, uint32_t set)
{
    doc_node* p;
    doc_node_source_range source_range;
    uint64_t start, end, a, b;
    const char* marker;
    unsigned marker_size;
    char *source, *replacement;
    int result;
    uint32_t change = set;
    if (!range || range->tAnchor.iNodeId != range->tCaret.iNodeId || !change || (change & (change - 1))) return doc_txn_fail(t, XUI_ERROR_UNSUPPORTED);
    p = doc_index_get(t->draft->index, range->tAnchor.iNodeId);
    if (!p || p->kind != XUI_DOC_TEXT || !p->source_exact) return doc_txn_fail(t, XUI_ERROR_UNSUPPORTED);
    doc_node_source_range_get(t->draft, p, &source_range);
    if (change == XUI_DOC_BOLD) { marker = "**"; marker_size = 2; }
    else if (change == XUI_DOC_ITALIC) { marker = "*"; marker_size = 1; }
    else if (change == XUI_DOC_STRIKE) { marker = "~~"; marker_size = 2; }
    else if (change == XUI_DOC_CODE) { marker = "`"; marker_size = 1; }
    else if (t->draft->dialect == XUI_MD_EXTENDED && change == XUI_DOC_HIGHLIGHT) { marker = "=="; marker_size = 2; }
    else if (t->draft->dialect == XUI_MD_EXTENDED && change == XUI_DOC_SUBSCRIPT) { marker = "~"; marker_size = 1; }
    else if (t->draft->dialect == XUI_MD_EXTENDED && change == XUI_DOC_SUPERSCRIPT) { marker = "^"; marker_size = 1; }
    else return doc_txn_fail(t, XUI_ERROR_UNSUPPORTED);
    a = range->tAnchor.iOffset; b = range->tCaret.iOffset;
    if (a > b) { uint64_t tmp = a; a = b; b = tmp; }
    if (b > doc_seq_size(p->text) || !doc_seq_boundary(p->text, a) || !doc_seq_boundary(p->text, b)) return doc_txn_fail(t, XUI_DOC_ERROR_UTF8);
    if (a == b || (p->attrs->iMarks & set)) return XUI_OK;
    source = doc_seq_string(t->draft->allocator, t->draft->source);
    if (!source) return doc_txn_fail(t, XUI_ERROR_OUT_OF_MEMORY);
    start = source_range.source_start + a; end = source_range.source_start + b;
    if (isspace((unsigned char)source[start]) || isspace((unsigned char)source[end - 1])) goto unsupported;
    replacement = doc_alloc(t->draft->allocator, (size_t)(end - start) + 2 * marker_size + 1);
    if (!replacement) { doc_free(source); return doc_txn_fail(t, XUI_ERROR_OUT_OF_MEMORY); }
    memcpy(replacement, marker, marker_size); memcpy(replacement + marker_size, source + start, (size_t)(end - start));
    memcpy(replacement + marker_size + end - start, marker, marker_size);
    result = doc_txn_source(t, start, end, replacement, end - start + 2 * marker_size);
    doc_free(replacement); doc_free(source); return result;
unsupported:
    doc_free(source); return doc_txn_fail(t, XUI_ERROR_UNSUPPORTED);
}
typedef struct doc_md_delimiter_patch { uint64_t start, end; } doc_md_delimiter_patch;
static int doc_md_delimiter_order(const void* left, const void* right)
{
    const doc_md_delimiter_patch *a = left, *b = right;
    return a->start < b->start ? -1 : a->start != b->start;
}
/* Attempt lossless delimiter removal on an unpublished candidate. Validation
 * against the desired semantic tree is mandatory: adjacent delimiters can
 * rebind, and removing code ticks may expose new Markdown syntax. */
static int doc_markdown_clear_delimiters(xui_document_transaction t, const xui_doc_range_t* range,
    uint32_t clear, doc_state* desired)
{
    struct xui_doc_snapshot_t snapshot = {0}; struct xui_doc_transaction_t candidate = {0};
    doc_md_delimiter_patch* patches; xui_doc_position_t first = range->tAnchor, last = range->tCaret;
    uint64_t start, end, i, count = 0, capacity = doc_seq_size(t->draft->inline_syntax) / sizeof(xui_doc_inline_syntax_t);
    uint32_t kind; int result, order, quality;
    if (clear == XUI_DOC_BOLD) kind = XUI_DOC_SYNTAX_STRONG;
    else if (clear == XUI_DOC_ITALIC) kind = XUI_DOC_SYNTAX_EMPHASIS;
    else if (clear == XUI_DOC_STRIKE) kind = XUI_DOC_SYNTAX_STRIKE;
    else if (clear == XUI_DOC_CODE) kind = XUI_DOC_SYNTAX_CODE;
    else if (clear == XUI_DOC_HIGHLIGHT) kind = XUI_DOC_SYNTAX_HIGHLIGHT;
    else if (clear == XUI_DOC_SUBSCRIPT) kind = XUI_DOC_SYNTAX_SUBSCRIPT;
    else if (clear == XUI_DOC_SUPERSCRIPT) kind = XUI_DOC_SYNTAX_SUPERSCRIPT;
    else if (clear == XUI_DOC_LINK) kind = XUI_DOC_SYNTAX_LINK;
    else return XUI_ERROR_NOT_FOUND;
    if (!capacity) return XUI_ERROR_NOT_FOUND;
    if (doc_position_compare(t->draft, &first, &last, &order) != XUI_OK || !order) return XUI_ERROR_NOT_FOUND;
    if (order > 0) { xui_doc_position_t temp = first; first = last; last = temp; }
    first.iAffinity = XUI_DOC_AFTER; last.iAffinity = XUI_DOC_BEFORE;
    snapshot.state = t->draft; snapshot.identity = t->document->identity; snapshot.revision = t->base_revision;
    if (xuiDocumentPositionToSource(&snapshot, &first, &start, &quality) != XUI_OK ||
        xuiDocumentPositionToSource(&snapshot, &last, &end, &quality) != XUI_OK || start >= end) return XUI_ERROR_NOT_FOUND;
    if (capacity > SIZE_MAX / (2 * sizeof(*patches))) return XUI_DOC_ERROR_LIMIT;
    patches = doc_alloc(t->draft->allocator, (size_t)capacity * 2 * sizeof(*patches));
    if (!patches) return XUI_ERROR_OUT_OF_MEMORY;
    for (i = 0; i < capacity; i++) {
        xui_doc_inline_syntax_t syntax;
        doc_seq_read_syntax(t->draft->inline_syntax, i, &syntax);
        if (syntax.iKind != kind || syntax.iContentStart >= end || syntax.iContentEnd <= start) continue;
        patches[count].start = syntax.iSourceStart; patches[count++].end = syntax.iContentStart;
        patches[count].start = syntax.iContentEnd; patches[count++].end = syntax.iSourceEnd;
    }
    if (!count) { doc_free(patches); return XUI_ERROR_NOT_FOUND; }
    qsort(patches, (size_t)count, sizeof(*patches), doc_md_delimiter_order);
    for (i = 1; i < count; i++) if (patches[i].start < patches[i - 1].end) { doc_free(patches); return XUI_ERROR_NOT_FOUND; }
    result = doc_markdown_shadow_begin(t, &candidate);
    for (i = count; i && result == XUI_OK; i--)
        result = doc_txn_source_patch(&candidate, patches[i - 1].start, patches[i - 1].end, "", 0, 0);
    if (result == XUI_OK) result = doc_markdown_parse(&candidate);
    if (result == XUI_OK && !doc_semantic_equal(desired, candidate.draft)) result = XUI_ERROR_NOT_FOUND;
    if (result == XUI_OK) {
        for (i = 0; i < candidate.count && result == XUI_OK; i++) result = doc_txn_op(t, &candidate.ops[i]);
        if (result == XUI_OK) { doc_state_release(t->draft); t->draft = candidate.draft; candidate.draft = NULL; t->parse_op_start = t->count; }
    }
    doc_free(patches); doc_state_release(candidate.draft); doc_free(candidate.ops); return result;
}
static uint32_t doc_md_mark_syntax_kind(uint32_t mark)
{
    switch (mark) {
    case XUI_DOC_BOLD: return XUI_DOC_SYNTAX_STRONG;
    case XUI_DOC_ITALIC: return XUI_DOC_SYNTAX_EMPHASIS;
    case XUI_DOC_STRIKE: return XUI_DOC_SYNTAX_STRIKE;
    case XUI_DOC_HIGHLIGHT: return XUI_DOC_SYNTAX_HIGHLIGHT;
    case XUI_DOC_SUBSCRIPT: return XUI_DOC_SYNTAX_SUBSCRIPT;
    case XUI_DOC_SUPERSCRIPT: return XUI_DOC_SYNTAX_SUPERSCRIPT;
    default: return 0;
    }
}
static int doc_md_wrapped_child_edge(doc_sequence* syntaxes, uint64_t parent_index,
    const xui_doc_inline_syntax_t* parent, uint64_t edge, int opening)
{
    uint64_t i, count = doc_seq_size(syntaxes) / sizeof(xui_doc_inline_syntax_t);
    for (i = 0; i < count; i++) {
        xui_doc_inline_syntax_t child;
        doc_seq_read_syntax(syntaxes, i, &child);
        if (child.iParentIndex == parent_index && child.iSourceStart == parent->iContentStart &&
            child.iSourceEnd == parent->iContentEnd &&
            (opening ? child.iContentStart : child.iContentEnd) == edge) return 1;
    }
    return 0;
}
/* Move the opening delimiter past a cleared prefix. A remaining run
 * beginning with a space needs an entity spelling for that space; an entity
 * spelling for the preceding letter keeps the delimiter left-flanking. Keep
 * this source-only rewrite private until a full semantic reparse accepts it. */
static int doc_markdown_clear_mark_prefix(xui_document_transaction t, const xui_doc_range_t* range,
    uint32_t mark, doc_state* desired)
{
    struct xui_doc_snapshot_t snapshot = {0}; struct xui_doc_transaction_t candidate = {0};
    xui_doc_position_t first = range->tAnchor, last = range->tCaret;
    xui_doc_inline_syntax_t syntax = {0};
    uint64_t start, end, size, count, i, base, span_bytes, scalar_start, leading_bytes, prefix_bytes, marker_bytes, suffix_bytes, bytes, used = 0;
    uint32_t scalar = 0; unsigned scalar_bytes, k;
    char entity[16], *source = NULL, *replacement = NULL;
    int order, quality, result, entity_bytes = 0;
    uint32_t syntax_kind = doc_md_mark_syntax_kind(mark);
    if (!syntax_kind) return XUI_ERROR_NOT_FOUND;
    if (doc_position_compare(t->draft, &first, &last, &order) != XUI_OK || !order) return XUI_ERROR_NOT_FOUND;
    if (order > 0) { xui_doc_position_t temp = first; first = last; last = temp; }
    first.iAffinity = XUI_DOC_AFTER; last.iAffinity = XUI_DOC_BEFORE;
    snapshot.state = t->draft; snapshot.identity = t->document->identity; snapshot.revision = t->base_revision;
    if (xuiDocumentPositionToSource(&snapshot, &first, &start, &quality) != XUI_OK || quality != XUI_DOC_MAP_EXACT ||
        xuiDocumentPositionToSource(&snapshot, &last, &end, &quality) != XUI_OK || quality != XUI_DOC_MAP_EXACT ||
        start >= end) return XUI_ERROR_NOT_FOUND;
    count = doc_seq_size(t->draft->inline_syntax) / sizeof(syntax);
    for (i = 0; i < count; i++) {
        doc_seq_read_syntax(t->draft->inline_syntax, i, &syntax);
        if (syntax.iKind == syntax_kind && syntax.iContentStart <= start &&
            syntax.iContentStart < end && end < syntax.iContentEnd &&
            syntax.iSourceStart < syntax.iContentStart && syntax.iContentEnd < syntax.iSourceEnd &&
            (syntax.iContentStart == start ||
                doc_md_wrapped_child_edge(t->draft->inline_syntax, i, &syntax, start, 1))) break;
    }
    if (i == count) return XUI_ERROR_NOT_FOUND;
    size = doc_seq_size(t->draft->source);
    if (syntax.iSourceEnd > size || end >= size) return XUI_ERROR_NOT_FOUND;
    base = syntax.iSourceStart; span_bytes = syntax.iSourceEnd - base;
    if (span_bytes > SIZE_MAX - 16) return XUI_DOC_ERROR_LIMIT;
    source = doc_alloc(t->draft->allocator, (size_t)span_bytes + 1);
    if (!source) return XUI_ERROR_OUT_OF_MEMORY;
    result = doc_seq_read(t->draft->source, base, source, span_bytes);
    if (result != XUI_OK) goto done;
    if (source[end - base] != ' ') { result = XUI_ERROR_NOT_FOUND; goto done; }
    scalar_start = end - 1;
    while (scalar_start > start && ((unsigned char)source[scalar_start - base] & 0xc0) == 0x80) scalar_start--;
    scalar_bytes = (unsigned)(end - scalar_start);
    if (scalar_bytes > 4 || !scalar_bytes) { result = XUI_ERROR_NOT_FOUND; goto done; }
    if (scalar_bytes == 1) scalar = (unsigned char)source[scalar_start - base];
    else {
        scalar = (unsigned char)source[scalar_start - base] & (0x7f >> scalar_bytes);
        for (k = 1; k < scalar_bytes; k++) scalar = (scalar << 6) | ((unsigned char)source[scalar_start + k - base] & 0x3f);
    }
    if (scalar_bytes > 1 || isalnum((unsigned char)scalar) || scalar == ' ' || scalar == '\t') {
        entity_bytes = snprintf(entity, sizeof(entity), "&#%u;", scalar);
        if (entity_bytes <= 0 || (size_t)entity_bytes >= sizeof(entity)) { result = XUI_DOC_ERROR_LIMIT; goto done; }
    }
    prefix_bytes = scalar_start - start;
    marker_bytes = syntax.iContentStart - syntax.iSourceStart;
    leading_bytes = start - syntax.iContentStart;
    suffix_bytes = syntax.iSourceEnd - end - 1;
    bytes = leading_bytes + prefix_bytes + (entity_bytes ? (uint64_t)entity_bytes : scalar_bytes) +
        marker_bytes + 5 + suffix_bytes;
    if (bytes >= SIZE_MAX || bytes > t->draft->allocator->max_bytes) { result = XUI_DOC_ERROR_LIMIT; goto done; }
    replacement = doc_alloc(t->draft->allocator, (size_t)bytes + 1);
    if (!replacement) { result = XUI_ERROR_OUT_OF_MEMORY; goto done; }
    memcpy(replacement + used, source + marker_bytes, (size_t)leading_bytes); used += leading_bytes;
    memcpy(replacement + used, source + (start - base), (size_t)prefix_bytes); used += prefix_bytes;
    if (entity_bytes) { memcpy(replacement + used, entity, (size_t)entity_bytes); used += (uint64_t)entity_bytes; }
    else { memcpy(replacement + used, source + (scalar_start - base), scalar_bytes); used += scalar_bytes; }
    memcpy(replacement + used, source, (size_t)marker_bytes); used += marker_bytes;
    memcpy(replacement + used, "&#32;", 5); used += 5;
    memcpy(replacement + used, source + (end + 1 - base), (size_t)suffix_bytes); used += suffix_bytes;
    result = doc_markdown_shadow_begin(t, &candidate);
    if (result == XUI_OK) result = doc_txn_source_patch(&candidate, syntax.iSourceStart, syntax.iSourceEnd, replacement, used, 0);
    if (result == XUI_OK) result = doc_markdown_parse(&candidate);
    if (result == XUI_OK && !doc_semantic_equal(desired, candidate.draft)) result = XUI_ERROR_NOT_FOUND;
    if (result == XUI_OK) {
        for (i = 0; i < candidate.count && result == XUI_OK; i++) result = doc_txn_op(t, &candidate.ops[i]);
        if (result == XUI_OK) { doc_state_release(t->draft); t->draft = candidate.draft; candidate.draft = NULL; t->parse_op_start = t->count; }
    }
done:
    doc_free(source); doc_free(replacement); doc_state_release(candidate.draft); doc_free(candidate.ops); return result;
}
static int doc_md_boundary_scalar(const char* source, uint64_t start, uint64_t end, int trailing,
    uint64_t* from, uint64_t* to, char entity[16], unsigned* entity_bytes)
{
    uint64_t at; uint32_t scalar; unsigned bytes, i; int length;
    if (start >= end) return XUI_ERROR_NOT_FOUND;
    at = trailing ? end - 1 : start;
    if (trailing) while (at > start && ((unsigned char)source[at] & 0xc0) == 0x80) at--;
    bytes = trailing ? (unsigned)(end - at) :
        (unsigned char)source[at] < 0x80 ? 1 : (unsigned char)source[at] < 0xe0 ? 2 :
        (unsigned char)source[at] < 0xf0 ? 3 : 4;
    if (!bytes || bytes > 4 || bytes > end - at) return XUI_ERROR_NOT_FOUND;
    *from = at; *to = at + bytes;
    scalar = bytes == 1 ? (unsigned char)source[at] : (unsigned char)source[at] & (0x7f >> bytes);
    for (i = 1; i < bytes; i++) scalar = (scalar << 6) | ((unsigned char)source[at + i] & 0x3f);
    *entity_bytes = 0;
    if (bytes > 1 || isalnum((unsigned char)scalar) || scalar == ' ' || scalar == '\t') {
        length = snprintf(entity, 16, "&#%u;", scalar);
        if (length <= 0 || length >= 16) return XUI_DOC_ERROR_LIMIT;
        *entity_bytes = (unsigned)length;
    }
    return XUI_OK;
}
static void doc_md_piece(char* target, uint64_t* used, const char* source, uint64_t bytes)
{
    if (bytes) memcpy(target + *used, source, (size_t)bytes);
    *used += bytes;
}
/* Move the closing delimiter before a cleared suffix. A middle selection
 * also needs a new opening delimiter for its remaining right-hand run. */
static int doc_markdown_clear_mark_tail(xui_document_transaction t, const xui_doc_range_t* range,
    uint32_t mark, doc_state* desired)
{
    struct xui_doc_snapshot_t snapshot = {0}; struct xui_doc_transaction_t candidate = {0};
    xui_doc_position_t first = range->tAnchor, last = range->tCaret;
    xui_doc_inline_syntax_t syntax = {0};
    uint64_t start, end, size, count, i, base, span_bytes, used = 0, marker_start, selected_start, selected_end;
    uint64_t first_from, first_to, last_from = 0, last_to = 0, content_end, marker_bytes;
    char first_entity[16], last_entity[16], *source = NULL, *replacement = NULL;
    const char* alternate = NULL; unsigned first_entity_bytes, last_entity_bytes = 0;
    int order, quality, result, middle;
    uint32_t syntax_kind = doc_md_mark_syntax_kind(mark);
    if (!syntax_kind) return XUI_ERROR_NOT_FOUND;
    if (doc_position_compare(t->draft, &first, &last, &order) != XUI_OK || !order) return XUI_ERROR_NOT_FOUND;
    if (order > 0) { xui_doc_position_t temp = first; first = last; last = temp; }
    first.iAffinity = XUI_DOC_AFTER; last.iAffinity = XUI_DOC_BEFORE;
    snapshot.state = t->draft; snapshot.identity = t->document->identity; snapshot.revision = t->base_revision;
    if (xuiDocumentPositionToSource(&snapshot, &first, &start, &quality) != XUI_OK || quality != XUI_DOC_MAP_EXACT ||
        xuiDocumentPositionToSource(&snapshot, &last, &end, &quality) != XUI_OK || quality != XUI_DOC_MAP_EXACT ||
        start >= end) return XUI_ERROR_NOT_FOUND;
    count = doc_seq_size(t->draft->inline_syntax) / sizeof(syntax);
    for (i = 0; i < count; i++) {
        doc_seq_read_syntax(t->draft->inline_syntax, i, &syntax);
        if (syntax.iKind == syntax_kind && syntax.iContentStart < start &&
            start < end && end <= syntax.iContentEnd &&
            syntax.iContentStart > syntax.iSourceStart &&
            syntax.iContentStart - syntax.iSourceStart == syntax.iSourceEnd - syntax.iContentEnd) break;
    }
    if (i == count) return XUI_ERROR_NOT_FOUND;
    size = doc_seq_size(t->draft->source);
    if (syntax.iSourceEnd > size) return XUI_ERROR_NOT_FOUND;
    base = syntax.iSourceStart; span_bytes = syntax.iSourceEnd - base;
    if (span_bytes > SIZE_MAX - 64) return XUI_DOC_ERROR_LIMIT;
    source = doc_alloc(t->draft->allocator, (size_t)span_bytes + 1);
    if (!source) return XUI_ERROR_OUT_OF_MEMORY;
    result = doc_seq_read(t->draft->source, base, source, span_bytes);
    if (result != XUI_OK) goto done;
    marker_bytes = syntax.iContentStart - syntax.iSourceStart;
    marker_start = syntax.iContentEnd - base; content_end = marker_start;
    selected_start = start - base; selected_end = end - base;
    middle = end < syntax.iContentEnd &&
        !doc_md_wrapped_child_edge(t->draft->inline_syntax, i, &syntax, end, 0);
    if (source[selected_start - 1] != ' ' || (middle && source[selected_end] != ' ')) { result = XUI_ERROR_NOT_FOUND; goto done; }
    if (mark == XUI_DOC_BOLD) {
        if (marker_bytes != 2) { result = XUI_ERROR_NOT_FOUND; goto done; }
        if (source[0] == '*' && source[1] == '*' && source[marker_start] == '*' && source[marker_start + 1] == '*') alternate = "__";
        else if (source[0] == '_' && source[1] == '_' && source[marker_start] == '_' && source[marker_start + 1] == '_') alternate = "**";
    } else {
        char marker = mark == XUI_DOC_ITALIC ? source[0] :
            mark == XUI_DOC_HIGHLIGHT ? '=' : mark == XUI_DOC_SUPERSCRIPT ? '^' : '~';
        uint64_t j, expected = (mark == XUI_DOC_STRIKE || mark == XUI_DOC_HIGHLIGHT) ? 2 : 1;
        if (mark == XUI_DOC_ITALIC && marker != '*' && marker != '_') { result = XUI_ERROR_NOT_FOUND; goto done; }
        if (mark == XUI_DOC_STRIKE && marker_bytes == 1) expected = 1;
        if (marker_bytes != expected) { result = XUI_ERROR_NOT_FOUND; goto done; }
        for (j = 0; j < marker_bytes; j++)
            if (source[j] != marker || source[marker_start + j] != marker) { result = XUI_ERROR_NOT_FOUND; goto done; }
        alternate = source;
    }
    if (!alternate) { result = XUI_ERROR_NOT_FOUND; goto done; }
    result = doc_md_boundary_scalar(source, selected_start, selected_end, 0,
        &first_from, &first_to, first_entity, &first_entity_bytes);
    if (result != XUI_OK) goto done;
    if (middle) {
        result = doc_md_boundary_scalar(source, selected_start, selected_end, 1,
            &last_from, &last_to, last_entity, &last_entity_bytes);
        if (result != XUI_OK) goto done;
    }
    replacement = doc_alloc(t->draft->allocator, (size_t)span_bytes + 64);
    if (!replacement) { result = XUI_ERROR_OUT_OF_MEMORY; goto done; }
    doc_md_piece(replacement, &used, source, marker_bytes);
    doc_md_piece(replacement, &used, source + (syntax.iContentStart - base), selected_start - 1 - (syntax.iContentStart - base));
    doc_md_piece(replacement, &used, "&#32;", 5);
    doc_md_piece(replacement, &used, source + marker_start, marker_bytes);
    doc_md_piece(replacement, &used, first_entity_bytes ? first_entity : source + first_from,
        first_entity_bytes ? first_entity_bytes : first_to - first_from);
    if (middle) {
        if (last_from > first_from) {
            doc_md_piece(replacement, &used, source + first_to, last_from - first_to);
            doc_md_piece(replacement, &used, last_entity_bytes ? last_entity : source + last_from,
                last_entity_bytes ? last_entity_bytes : last_to - last_from);
        }
        doc_md_piece(replacement, &used, alternate, marker_bytes);
        doc_md_piece(replacement, &used, "&#32;", 5);
        doc_md_piece(replacement, &used, source + selected_end + 1, content_end - selected_end - 1);
        doc_md_piece(replacement, &used, alternate, marker_bytes);
    } else {
        doc_md_piece(replacement, &used, source + first_to, selected_end - first_to);
        doc_md_piece(replacement, &used, source + selected_end, content_end - selected_end);
    }
    result = doc_markdown_shadow_begin(t, &candidate);
    if (result == XUI_OK) result = doc_txn_source_patch(&candidate, syntax.iSourceStart, syntax.iSourceEnd, replacement, used, 0);
    if (result == XUI_OK) result = doc_markdown_parse(&candidate);
    if (result == XUI_OK && !doc_semantic_equal(desired, candidate.draft)) result = XUI_ERROR_NOT_FOUND;
    if (result == XUI_OK) {
        for (i = 0; i < candidate.count && result == XUI_OK; i++) result = doc_txn_op(t, &candidate.ops[i]);
        if (result == XUI_OK) { doc_state_release(t->draft); t->draft = candidate.draft; candidate.draft = NULL; t->parse_op_start = t->count; }
    }
done:
    doc_free(source); doc_free(replacement); doc_state_release(candidate.draft); doc_free(candidate.ops); return result;
}
/* A complete strong child can sit inside one emphasis span (***...***).
 * Clearing emphasis in the middle must move the strong delimiter outside
 * two emphasis runs. Copy the selected source bytes verbatim so an entity or
 * escape inside the selection keeps its original spelling. */
static int doc_markdown_clear_wrapped_middle(xui_document_transaction t,
    const xui_doc_range_t* range, uint32_t mark, doc_state* desired)
{
    struct xui_doc_snapshot_t snapshot = {0}; struct xui_doc_transaction_t candidate = {0};
    xui_doc_position_t first = range->tAnchor, last = range->tCaret;
    xui_doc_inline_syntax_t outer = {0}, strong = {0};
    uint64_t start, end, count, i, j, base, span_bytes, selected_start, selected_end, used = 0;
    char *source = NULL, *replacement = NULL; const char* emphasis;
    int order, quality, result;
    if (mark != XUI_DOC_ITALIC) return XUI_ERROR_NOT_FOUND;
    if (doc_position_compare(t->draft, &first, &last, &order) != XUI_OK || !order) return XUI_ERROR_NOT_FOUND;
    if (order > 0) { xui_doc_position_t temp = first; first = last; last = temp; }
    first.iAffinity = XUI_DOC_AFTER; last.iAffinity = XUI_DOC_BEFORE;
    snapshot.state = t->draft; snapshot.identity = t->document->identity; snapshot.revision = t->base_revision;
    if (xuiDocumentPositionToSource(&snapshot, &first, &start, &quality) != XUI_OK || quality != XUI_DOC_MAP_EXACT ||
        xuiDocumentPositionToSource(&snapshot, &last, &end, &quality) != XUI_OK || quality != XUI_DOC_MAP_EXACT ||
        start >= end) return XUI_ERROR_NOT_FOUND;
    count = doc_seq_size(t->draft->inline_syntax) / sizeof(outer);
    for (j = 0; j < count; j++) {
        doc_seq_read_syntax(t->draft->inline_syntax, j, &strong);
        if (strong.iKind != XUI_DOC_SYNTAX_STRONG || strong.iParentIndex >= count ||
            strong.iContentStart - strong.iSourceStart != 2 ||
            strong.iSourceEnd - strong.iContentEnd != 2 ||
            strong.iContentStart >= start || end >= strong.iContentEnd) continue;
        doc_seq_read_syntax(t->draft->inline_syntax, strong.iParentIndex, &outer);
        if (outer.iKind == XUI_DOC_SYNTAX_EMPHASIS &&
            outer.iContentStart - outer.iSourceStart == 1 &&
            outer.iSourceEnd - outer.iContentEnd == 1 &&
            strong.iSourceStart == outer.iContentStart && strong.iSourceEnd == outer.iContentEnd) break;
    }
    if (j == count || outer.iSourceEnd > doc_seq_size(t->draft->source)) return XUI_ERROR_NOT_FOUND;
    base = outer.iSourceStart; span_bytes = outer.iSourceEnd - base;
    if (span_bytes > SIZE_MAX - 64) return XUI_DOC_ERROR_LIMIT;
    source = doc_alloc(t->draft->allocator, (size_t)span_bytes + 1);
    if (!source) return XUI_ERROR_OUT_OF_MEMORY;
    result = doc_seq_read(t->draft->source, base, source, span_bytes);
    if (result != XUI_OK) goto done;
    selected_start = start - base; selected_end = end - base;
    if (source[selected_start - 1] != ' ' || source[selected_end] != ' ' ||
        (source[0] != '*' && source[0] != '_') ||
        source[0] != source[1] || source[0] != source[2] ||
        source[0] != source[span_bytes - 1] ||
        source[0] != source[span_bytes - 2] || source[0] != source[span_bytes - 3]) {
        result = XUI_ERROR_NOT_FOUND; goto done;
    }
    emphasis = source[0] == '*' ? "_" : "*";
    replacement = doc_alloc(t->draft->allocator, (size_t)span_bytes + 64);
    if (!replacement) { result = XUI_ERROR_OUT_OF_MEMORY; goto done; }
    doc_md_piece(replacement, &used, source + (strong.iSourceStart - base), 2);
    doc_md_piece(replacement, &used, emphasis, 1);
    doc_md_piece(replacement, &used, source + (strong.iContentStart - base),
        selected_start - 1 - (strong.iContentStart - base));
    doc_md_piece(replacement, &used, "&#32;", 5);
    doc_md_piece(replacement, &used, emphasis, 1);
    doc_md_piece(replacement, &used, source + selected_start, selected_end - selected_start);
    doc_md_piece(replacement, &used, emphasis, 1);
    doc_md_piece(replacement, &used, "&#32;", 5);
    doc_md_piece(replacement, &used, source + selected_end + 1,
        strong.iContentEnd - end - 1);
    doc_md_piece(replacement, &used, emphasis, 1);
    doc_md_piece(replacement, &used, source + (strong.iContentEnd - base), 2);
    result = doc_markdown_shadow_begin(t, &candidate);
    if (result == XUI_OK) result = doc_txn_source_patch(&candidate, outer.iSourceStart, outer.iSourceEnd, replacement, used, 0);
    if (result == XUI_OK) result = doc_markdown_parse(&candidate);
    if (result == XUI_OK && !doc_semantic_equal(desired, candidate.draft)) result = XUI_ERROR_NOT_FOUND;
    if (result == XUI_OK) {
        for (i = 0; i < candidate.count && result == XUI_OK; i++) result = doc_txn_op(t, &candidate.ops[i]);
        if (result == XUI_OK) { doc_state_release(t->draft); t->draft = candidate.draft; candidate.draft = NULL; t->parse_op_start = t->count; }
    }
done:
    doc_free(source); doc_free(replacement); doc_state_release(candidate.draft); doc_free(candidate.ops); return result;
}
int doc_markdown_marks(xui_document_transaction t, const xui_doc_range_t* range, uint32_t set, uint32_t clear)
{
    struct xui_doc_transaction_t expected = {0}; int result;
    if ((set & XUI_DOC_UNDERLINE) ||
        (t->draft->dialect == XUI_MD_COMMONMARK && (set & XUI_DOC_STRIKE)) ||
        (t->draft->dialect != XUI_MD_EXTENDED &&
            (set & (XUI_DOC_HIGHLIGHT | XUI_DOC_SUBSCRIPT | XUI_DOC_SUPERSCRIPT))))
        return doc_txn_fail(t, XUI_DOC_ERROR_UNREPRESENTABLE);
    result = doc_markdown_shadow_begin(t, &expected); if (result != XUI_OK) return result;
    result = xuiDocumentTxnSetMarks(&expected, range, set, clear);
    if (result == XUI_OK && expected.count) {
        uint32_t change = set | clear;
        doc_node* n = doc_index_get(t->draft->index, range->tAnchor.iNodeId);
        result = !set && doc_md_mark_syntax_kind(clear) ?
            doc_markdown_clear_mark_prefix(t, range, clear, expected.draft) : XUI_ERROR_NOT_FOUND;
        if (result == XUI_ERROR_NOT_FOUND && !set && clear == XUI_DOC_ITALIC)
            result = doc_markdown_clear_wrapped_middle(t, range, clear, expected.draft);
        if (result == XUI_ERROR_NOT_FOUND && !set && doc_md_mark_syntax_kind(clear))
            result = doc_markdown_clear_mark_tail(t, range, clear, expected.draft);
        if (result == XUI_ERROR_NOT_FOUND && !set && clear)
            result = doc_markdown_clear_delimiters(t, range, clear, expected.draft);
        if (result == XUI_ERROR_NOT_FOUND) {
            if (range->tAnchor.iNodeId == range->tCaret.iNodeId && n && n->source_exact && change && !(change & (change - 1)) &&
                !(n->attrs->iMarks & ~change) && (change & (XUI_DOC_BOLD | XUI_DOC_ITALIC |
                    XUI_DOC_STRIKE | XUI_DOC_CODE | XUI_DOC_HIGHLIGHT | XUI_DOC_SUBSCRIPT | XUI_DOC_SUPERSCRIPT)) && !clear)
                result = doc_markdown_marks_patch(t, range, set);
            else result = doc_markdown_apply_tree(t, expected.draft, NULL, NULL);
        }
    }
    if (result == XUI_OK && expected.count) result = doc_markdown_accept(t, &expected);
    doc_state_release(expected.draft); doc_free(expected.ops);
    return result == XUI_OK ? result : doc_txn_fail(t, result);
}
int doc_markdown_link(xui_document_transaction t, const xui_doc_range_t* range,
    const char* uri, const char* title)
{
    struct xui_doc_transaction_t expected = {0}; int result;
    result = doc_markdown_shadow_begin(t, &expected);
    if (result != XUI_OK) return result;
    result = xuiDocumentTxnSetLink(&expected, range, uri, title);
    if (result == XUI_OK && expected.count) {
        result = !*uri ? doc_markdown_clear_delimiters(t, range, XUI_DOC_LINK, expected.draft) :
            doc_markdown_link_patch(t, range, uri, title, expected.draft);
        if (result == XUI_ERROR_NOT_FOUND) result = doc_markdown_apply_tree(t, expected.draft, NULL, NULL);
    }
    if (result == XUI_OK && expected.count) result = doc_markdown_accept(t, &expected);
    doc_state_release(expected.draft); doc_free(expected.ops);
    return result == XUI_OK ? result : doc_txn_fail(t, result);
}

#endif
