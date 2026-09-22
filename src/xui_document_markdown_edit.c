#include "xui_document_internal.h"
#include <ctype.h>

typedef struct doc_md_output { doc_allocator* allocator; char* data; uint64_t size, capacity; int error; } doc_md_output;
static void doc_md_write(doc_md_output* b, const char* text, uint64_t bytes)
{
    uint64_t required, capacity; char* next;
    if (b->error) return;
    if (bytes >= SIZE_MAX || b->size >= SIZE_MAX - bytes) { b->error = XUI_DOC_ERROR_LIMIT; return; }
    required = b->size + bytes + 1;
    if (required > b->capacity) {
        capacity = b->capacity ? b->capacity : 128;
        while (capacity < required) { if (capacity > SIZE_MAX / 2) { capacity = required; break; } capacity *= 2; }
        next = doc_realloc(b->allocator, b->data, (size_t)capacity);
        if (!next) { b->error = XUI_ERROR_OUT_OF_MEMORY; return; }
        b->data = next; b->capacity = capacity;
    }
    if (bytes) memcpy(b->data + b->size, text, (size_t)bytes);
    b->size += bytes; b->data[b->size] = 0;
}
static void doc_md_escape(doc_md_output* b, const char* text, uint64_t bytes)
{
    uint64_t i;
    for (i = 0; i < bytes && !b->error; i++) {
        unsigned char c = (unsigned char)text[i];
        if ((c >= 33 && c <= 47) || (c >= 58 && c <= 64) || (c >= 91 && c <= 96) || (c >= 123 && c <= 126)) doc_md_write(b, "\\", 1);
        doc_md_write(b, text + i, 1);
    }
}
static void doc_md_inline(doc_state* s, doc_node* n, doc_md_output* b)
{
    char* text; uint64_t length = doc_seq_size(n->text), i, fence = 1, run = 0; uint32_t marks = n->attrs.iMarks;
    if (n->kind == XUI_DOC_SOFT_BREAK) { doc_md_write(b, "\n", 1); return; }
    if (n->kind == XUI_DOC_HARD_BREAK) { doc_md_write(b, "\\\n", 2); return; }
    if (n->kind != XUI_DOC_TEXT || marks & ~(XUI_DOC_BOLD | XUI_DOC_ITALIC | XUI_DOC_STRIKE | XUI_DOC_CODE | XUI_DOC_LINK)) { b->error = XUI_ERROR_UNSUPPORTED; return; }
    text = doc_seq_string(s->allocator, n->text); if (!text) { b->error = XUI_ERROR_OUT_OF_MEMORY; return; }
    if (marks & XUI_DOC_LINK) doc_md_write(b, "[", 1);
    if (marks & XUI_DOC_BOLD) doc_md_write(b, "**", 2);
    if (marks & XUI_DOC_ITALIC) doc_md_write(b, "*", 1);
    if (marks & XUI_DOC_STRIKE) doc_md_write(b, "~~", 2);
    if (marks & XUI_DOC_CODE) {
        int pad;
        for (i = 0; i < length; i++) { if (text[i] == '`') { if (++run >= fence) fence = run + 1; } else run = 0; }
        for (i = 0; i < fence; i++) doc_md_write(b, "`", 1);
        pad = length && (text[0] == '`' || text[length - 1] == '`' || (text[0] == ' ' && text[length - 1] == ' '));
        if (pad) doc_md_write(b, " ", 1);
        doc_md_write(b, text, length);
        if (pad) doc_md_write(b, " ", 1);
        for (i = 0; i < fence; i++) doc_md_write(b, "`", 1);
    } else doc_md_escape(b, text, length);
    if (marks & XUI_DOC_STRIKE) doc_md_write(b, "~~", 2);
    if (marks & XUI_DOC_ITALIC) doc_md_write(b, "*", 1);
    if (marks & XUI_DOC_BOLD) doc_md_write(b, "**", 2);
    if (marks & XUI_DOC_LINK) {
        const char* uri = doc_string(n->resource); const char* title = doc_string(n->title);
        doc_md_write(b, "](<", 3); doc_md_escape(b, uri, strlen(uri)); doc_md_write(b, ">", 1);
        if (*title) { doc_md_write(b, " \"", 2); doc_md_escape(b, title, strlen(title)); doc_md_write(b, "\"", 1); }
        doc_md_write(b, ")", 1);
    }
    doc_free(text);
}
static int doc_md_paragraph(doc_state* s, uint64_t id, doc_md_output* b)
{
    doc_node* n = doc_index_get(s->index, id); uint64_t i;
    if (n->kind != XUI_DOC_PARAGRAPH && n->kind != XUI_DOC_HEADING) return XUI_ERROR_UNSUPPORTED;
    if (n->kind == XUI_DOC_HEADING) { for (i = 0; i < n->attrs.iHeadingLevel; i++) doc_md_write(b, "#", 1); doc_md_write(b, " ", 1); }
    for (i = 0; i < doc_seq_size(n->children) && !b->error; i++) {
        doc_node* child = doc_index_get(s->index, doc_seq_get_id(n->children, i));
        doc_node run = {0};
        if (child->kind != XUI_DOC_TEXT) { doc_md_inline(s, child, b); continue; }
        run.kind = child->kind; run.attrs = child->attrs; run.text = child->text;
        run.resource = child->resource; run.info = child->info; run.title = child->title;
        doc_seq_retain(run.text);
        while (i + 1 < doc_seq_size(n->children)) {
            doc_node* next = doc_index_get(s->index, doc_seq_get_id(n->children, i + 1)); doc_sequence* merged;
            uint64_t size = doc_seq_size(run.text);
            if (next->kind != XUI_DOC_TEXT || !doc_attributes_equal(&run.attrs, &next->attrs) ||
                strcmp(doc_string(run.resource), doc_string(next->resource)) ||
                strcmp(doc_string(run.info), doc_string(next->info)) || strcmp(doc_string(run.title), doc_string(next->title))) break;
            b->error = doc_seq_replace(s->allocator, run.text, size, size, next->text, &merged);
            if (b->error) break;
            doc_seq_release(run.text); run.text = merged; i++;
        }
        if (!b->error && doc_seq_size(run.text)) doc_md_inline(s, &run, b);
        doc_seq_release(run.text);
    }
    doc_md_write(b, "\n\n", 2); return b->error;
}
static uint64_t doc_md_line_start(const char* source, uint64_t p)
{
    while (p && source[p - 1] != '\n' && source[p - 1] != '\r') p--;
    if (!p && (unsigned char)source[0] == 0xef && (unsigned char)source[1] == 0xbb && (unsigned char)source[2] == 0xbf) p = 3;
    return p;
}
static uint64_t doc_md_line_end(const char* source, uint64_t size, uint64_t p)
{
    while (p < size && source[p] != '\n' && source[p] != '\r') p++;
    if (p < size && source[p] == '\r') p++;
    if (p < size && source[p] == '\n') p++;
    return p;
}
static int doc_md_bounds(doc_state* s, uint64_t id, const char* source, uint64_t* start, uint64_t* end)
{
    doc_node* n = doc_index_get(s->index, id); uint64_t size = doc_seq_size(s->source), at;
    if ((n->kind != XUI_DOC_PARAGRAPH && n->kind != XUI_DOC_HEADING) || n->source_start == DOC_NONE || n->source_end == DOC_NONE) return XUI_ERROR_UNSUPPORTED;
    *start = doc_md_line_start(source, n->source_start); *end = doc_md_line_end(source, size, n->source_end);
    at = *start; while (at < size && source[at] == ' ') at++;
    if (n->kind == XUI_DOC_HEADING && at < size && source[at] != '#') *end = doc_md_line_end(source, size, *end);
    return XUI_OK;
}
static xui_doc_position_t doc_md_rewrite_caret(xui_document_transaction t, doc_state* expected, xui_doc_position_t caret)
{
    doc_node* n = doc_index_get(expected->index, caret.iNodeId); doc_node* root = doc_index_get(expected->index, DOC_ROOT);
    doc_node* actual_root = doc_index_get(t->draft->index, DOC_ROOT); uint64_t index = 0, offset = caret.iOffset, i;
    if (n && n->kind == XUI_DOC_TEXT) {
        doc_node* p = doc_index_get(expected->index, n->parent);
        for (i = 0; i < doc_seq_size(p->children); i++) {
            doc_node* previous = doc_index_get(expected->index, doc_seq_get_id(p->children, i));
            if (previous->id == n->id) break;
            offset += doc_seq_size(previous->text) + (previous->kind == XUI_DOC_SOFT_BREAK || previous->kind == XUI_DOC_HARD_BREAK);
        }
        n = p;
    }
    if (n && n->parent == DOC_ROOT) {
        for (i = 0; i < doc_seq_size(root->children); i++) {
            doc_node* previous = doc_index_get(expected->index, doc_seq_get_id(root->children, i));
            if (previous->id == n->id) break;
            if (!doc_semantic_empty_paragraph(expected, previous)) index++;
        }
        if (doc_semantic_empty_paragraph(expected, n)) {
            caret.iNodeId = DOC_ROOT; caret.iKind = XUI_DOC_POSITION_GAP; caret.iOffset = index; return caret;
        }
        n = doc_index_get(t->draft->index, doc_seq_get_id(actual_root->children, index));
        if (n) for (i = 0; i < doc_seq_size(n->children); i++) {
            doc_node* child = doc_index_get(t->draft->index, doc_seq_get_id(n->children, i)); uint64_t length = doc_seq_size(child->text);
            if (doc_text_kind(child->kind) && offset <= length) { caret.iNodeId = child->id; caret.iOffset = offset; caret.iKind = XUI_DOC_POSITION_TEXT; return caret; }
            length += child->kind == XUI_DOC_SOFT_BREAK || child->kind == XUI_DOC_HARD_BREAK;
            if (offset >= length) offset -= length;
        }
    }
    caret.iNodeId = DOC_ROOT; caret.iKind = XUI_DOC_POSITION_GAP; caret.iOffset = doc_seq_size(actual_root->children); return caret;
}
int doc_markdown_apply_paragraphs(xui_document_transaction t, doc_state* desired, const xui_doc_position_t* target, xui_doc_position_t* caret)
{
    doc_state* old = t->draft; doc_node *before, *after;
    doc_md_output output = {0}; char* source = NULL; uint64_t prefix = 0, suffix = 0, ac, bc, i, start, end, unused;
    int result = XUI_OK;
    before = doc_index_get(old->index, DOC_ROOT); after = doc_index_get(desired->index, DOC_ROOT);
    ac = doc_seq_size(before->children); bc = doc_seq_size(after->children);
    while (prefix < ac && prefix < bc && doc_semantic_subtree_equal(old, doc_seq_get_id(before->children, prefix), desired, doc_seq_get_id(after->children, prefix))) prefix++;
    while (suffix < ac - prefix && suffix < bc - prefix && doc_semantic_subtree_equal(old, doc_seq_get_id(before->children, ac - suffix - 1), desired, doc_seq_get_id(after->children, bc - suffix - 1))) suffix++;
    if (prefix == ac && prefix == bc) { if (caret && target) *caret = *target; goto done; }
    source = doc_seq_string(t->document->allocator, old->source); if (!source) { result = XUI_ERROR_OUT_OF_MEMORY; goto done; }
    start = end = doc_seq_size(old->source);
    if (ac > prefix + suffix) {
        result = doc_md_bounds(old, doc_seq_get_id(before->children, prefix), source, &start, &unused); if (result != XUI_OK) goto done;
        result = doc_md_bounds(old, doc_seq_get_id(before->children, ac - suffix - 1), source, &unused, &end); if (result != XUI_OK) goto done;
        for (i = prefix; i < ac - suffix; i++) {
            doc_node* n = doc_index_get(old->index, doc_seq_get_id(before->children, i));
            if (n->kind != XUI_DOC_PARAGRAPH && n->kind != XUI_DOC_HEADING) { result = XUI_ERROR_UNSUPPORTED; goto done; }
        }
    } else if (prefix < ac) {
        result = doc_md_bounds(old, doc_seq_get_id(before->children, prefix), source, &start, &unused); if (result != XUI_OK) goto done; end = start;
    }
    output.allocator = t->document->allocator;
    if (start && source[start - 1] != '\n' && source[start - 1] != '\r') doc_md_write(&output, "\n\n", 2);
    for (i = prefix; i < bc - suffix; i++) { result = doc_md_paragraph(desired, doc_seq_get_id(after->children, i), &output); if (result != XUI_OK) goto done; }
    result = doc_txn_source(t, start, end, output.data ? output.data : "", output.size);
    if (result == XUI_OK && !doc_semantic_equal(desired, t->draft)) result = XUI_DOC_ERROR_UNREPRESENTABLE;
    if (result == XUI_OK && target && caret) *caret = doc_md_rewrite_caret(t, desired, *target);
done:
    doc_free(source); doc_free(output.data); return result == XUI_OK ? result : doc_txn_fail(t, result);
}
int doc_markdown_range(xui_document_transaction t, const xui_doc_range_t* range, const char* text, uint64_t bytes, xui_doc_position_t* caret)
{
    struct xui_doc_transaction_t expected = {0}; xui_doc_position_t target = {0}; int result;
    expected.document = t->document; expected.domain = XUI_DOC_SEMANTIC; expected.parsing = 1; expected.base_revision = t->base_revision;
    expected.draft = doc_state_clone(t->draft); if (!expected.draft) return doc_txn_fail(t, XUI_ERROR_OUT_OF_MEMORY);
    result = xuiDocumentTxnReplaceRange(&expected, range, text, bytes, &target);
    if (result == XUI_OK) result = doc_markdown_apply_paragraphs(t, expected.draft, &target, caret);
    doc_state_release(expected.draft); return result == XUI_OK ? result : doc_txn_fail(t, result);
}
