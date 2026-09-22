#include "xui_document_internal.h"

static int doc_command_position(xui_document_transaction t, const xui_doc_position_t* p)
{
    if (!p || p->iDocumentId != t->document->identity || p->iRevision != t->base_revision) return XUI_DOC_ERROR_STALE;
    return doc_position_valid(t->draft, p) ? XUI_OK : XUI_ERROR_INVALID_ARGUMENT;
}
static xui_doc_position_t doc_command_caret(xui_document_transaction t, uint64_t id, uint64_t offset, unsigned kind)
{
    xui_doc_position_t p = {0}; p.iSize = sizeof(p); p.iDocumentId = t->document->identity; p.iRevision = t->base_revision;
    p.iNodeId = id; p.iOffset = offset; p.iKind = kind; p.iAffinity = XUI_DOC_AFTER; return p;
}
static int doc_paragraph(doc_node* n) { return n && (n->kind == XUI_DOC_PARAGRAPH || n->kind == XUI_DOC_HEADING); }
static int doc_insert_empty(xui_document_transaction t, uint64_t parent, uint64_t index, unsigned kind, uint64_t* id)
{
    xui_doc_node_desc_t desc = {0}; desc.iSize = sizeof(desc); desc.iKind = kind;
    return doc_txn_insert(t, parent, index, &desc, id);
}
/* Convert a structural insertion gap to a text caret without flattening styles. */
static int doc_text_caret(xui_document_transaction t, xui_doc_position_t* p)
{
    doc_node* n = doc_index_get(t->draft->index, p->iNodeId); uint64_t id; int result;
    if (p->iKind == XUI_DOC_POSITION_TEXT) return XUI_OK;
    if (p->iKind != XUI_DOC_POSITION_GAP) return XUI_DOC_ERROR_DOMAIN;
    if (!doc_paragraph(n)) {
        result = doc_insert_empty(t, n->id, p->iOffset, XUI_DOC_PARAGRAPH, &id);
        if (result != XUI_OK) return result;
        *p = doc_command_caret(t, id, 0, XUI_DOC_POSITION_GAP); n = doc_index_get(t->draft->index, id);
    }
    if (p->iOffset) {
        doc_node* previous = doc_index_get(t->draft->index, doc_seq_get_id(n->children, p->iOffset - 1));
        if (previous->kind == XUI_DOC_TEXT) { *p = doc_command_caret(t, previous->id, doc_seq_size(previous->text), XUI_DOC_POSITION_TEXT); return XUI_OK; }
    }
    result = doc_insert_empty(t, n->id, p->iOffset, XUI_DOC_TEXT, &id);
    if (result == XUI_OK) *p = doc_command_caret(t, id, 0, XUI_DOC_POSITION_TEXT);
    return result;
}
XUI_API int xuiDocumentTxnSplitBlock(xui_document_transaction t, const xui_doc_position_t* at, xui_doc_position_t* caret)
{
    doc_node *n, *block; xui_doc_position_t p; uint64_t index, block_id, parent, next, tail = 0, new_text = 0; int result;
    xui_doc_node_desc_t desc = {0};
    result = doc_txn_check(t, XUI_DOC_SEMANTIC); if (result != XUI_OK) return result;
    if (!caret || (result = doc_command_position(t, at)) != XUI_OK) return doc_txn_fail(t, caret ? result : XUI_ERROR_INVALID_ARGUMENT);
    if (t->draft->profile == XUI_DOCUMENT_MARKDOWN && !t->parsing) return doc_txn_fail(t, XUI_ERROR_UNSUPPORTED);
    p = *at; n = doc_index_get(t->draft->index, p.iNodeId);
    if (p.iKind == XUI_DOC_POSITION_TEXT) {
        block = doc_index_get(t->draft->index, n->parent);
        if (n->kind != XUI_DOC_TEXT || !doc_paragraph(block)) return doc_txn_fail(t, XUI_ERROR_UNSUPPORTED);
        block_id = block->id; parent = block->parent; index = doc_child_index(block, n->id);
        if (p.iOffset == 0) tail = n->id;
        else if (p.iOffset < doc_seq_size(n->text)) {
            result = doc_split_text(t, n->id, p.iOffset, &tail); if (result != XUI_OK) return result;
            index++;
        } else index++;
    } else if (p.iKind == XUI_DOC_POSITION_GAP && doc_paragraph(n)) {
        block_id = n->id; parent = n->parent; index = p.iOffset;
    } else return doc_txn_fail(t, XUI_ERROR_UNSUPPORTED);
    block = doc_index_get(t->draft->index, block_id);
    desc.iSize = sizeof(desc); desc.iKind = XUI_DOC_PARAGRAPH; desc.tAttributes = block->attrs; desc.tAttributes.iHeadingLevel = 0;
    result = doc_txn_insert(t, parent, doc_child_index(doc_index_get(t->draft->index, parent), block_id) + 1, &desc, &next);
    if (result != XUI_OK) return result;
    while (index < doc_seq_size((block = doc_index_get(t->draft->index, block_id))->children)) {
        uint64_t child = doc_seq_get_id(block->children, index);
        if (!tail) tail = child;
        result = xuiDocumentTxnMoveNode(t, child, next, DOC_NONE); if (result != XUI_OK) return result;
    }
    n = doc_index_get(t->draft->index, tail);
    if (!n || n->kind != XUI_DOC_TEXT) {
        result = doc_insert_empty(t, next, 0, XUI_DOC_TEXT, &new_text); if (result != XUI_OK) return result;
    }
    *caret = doc_command_caret(t, new_text ? new_text : tail, 0, XUI_DOC_POSITION_TEXT); return XUI_OK;
}
XUI_API int xuiDocumentTxnJoinBlocks(xui_document_transaction t, uint64_t left, uint64_t right, xui_doc_position_t* caret)
{
    doc_node *a, *b, *parent; uint64_t offset, child; int result = doc_txn_check(t, XUI_DOC_SEMANTIC);
    if (result != XUI_OK) return result;
    if (!caret) return doc_txn_fail(t, XUI_ERROR_INVALID_ARGUMENT);
    if (t->draft->profile == XUI_DOCUMENT_MARKDOWN && !t->parsing) return doc_txn_fail(t, XUI_ERROR_UNSUPPORTED);
    a = doc_index_get(t->draft->index, left); b = doc_index_get(t->draft->index, right);
    if (!doc_paragraph(a) || !doc_paragraph(b) || a->parent != b->parent) return doc_txn_fail(t, XUI_DOC_ERROR_SCHEMA);
    parent = doc_index_get(t->draft->index, a->parent);
    if (doc_child_index(parent, right) != doc_child_index(parent, left) + 1) return doc_txn_fail(t, XUI_DOC_ERROR_SCHEMA);
    offset = doc_seq_size(a->children); *caret = doc_command_caret(t, left, offset, XUI_DOC_POSITION_GAP);
    if (offset) {
        doc_node* last = doc_index_get(t->draft->index, doc_seq_get_id(a->children, offset - 1));
        if (last->kind == XUI_DOC_TEXT) *caret = doc_command_caret(t, last->id, doc_seq_size(last->text), XUI_DOC_POSITION_TEXT);
    }
    while (doc_seq_size((b = doc_index_get(t->draft->index, right))->children)) {
        child = doc_seq_get_id(b->children, 0);
        result = xuiDocumentTxnMoveNode(t, child, left, DOC_NONE); if (result != XUI_OK) return result;
    }
    return doc_txn_delete(t, right);
}
static int doc_delete_range(xui_document_transaction t, xui_doc_position_t a, xui_doc_position_t b, xui_doc_position_t* caret)
{
    doc_node *na = doc_index_get(t->draft->index, a.iNodeId), *nb = doc_index_get(t->draft->index, b.iNodeId), *pa, *pb;
    uint64_t ai, bi, i, ap, bp, parent; int result;
    *caret = a;
    if (a.iNodeId == b.iNodeId) {
        if (a.iKind == XUI_DOC_POSITION_TEXT) return doc_txn_text(t, a.iNodeId, a.iOffset, b.iOffset, "", 0);
        for (i = b.iOffset; i > a.iOffset; i--) {
            na = doc_index_get(t->draft->index, a.iNodeId);
            result = doc_txn_delete(t, doc_seq_get_id(na->children, i - 1)); if (result != XUI_OK) return result;
        }
        return XUI_OK;
    }
    pa = a.iKind == XUI_DOC_POSITION_TEXT ? doc_index_get(t->draft->index, na->parent) : na;
    pb = b.iKind == XUI_DOC_POSITION_TEXT ? doc_index_get(t->draft->index, nb->parent) : nb;
    if (!doc_paragraph(pa) || !doc_paragraph(pb) || (pa->id != pb->id && pa->parent != pb->parent)) return doc_txn_fail(t, XUI_ERROR_UNSUPPORTED);
    ap = pa->id; bp = pb->id; parent = pa->parent;
    ai = a.iKind == XUI_DOC_POSITION_TEXT ? doc_child_index(pa, na->id) + 1 : a.iOffset;
    bi = b.iKind == XUI_DOC_POSITION_TEXT ? doc_child_index(pb, nb->id) : b.iOffset;
    if (a.iKind == XUI_DOC_POSITION_TEXT) {
        result = doc_txn_text(t, a.iNodeId, a.iOffset, doc_seq_size(na->text), "", 0); if (result != XUI_OK) return result;
    }
    if (b.iKind == XUI_DOC_POSITION_TEXT) {
        result = doc_txn_text(t, b.iNodeId, 0, b.iOffset, "", 0); if (result != XUI_OK) return result;
    }
    if (ap == bp) {
        for (i = bi; i > ai; i--) {
            pa = doc_index_get(t->draft->index, ap);
            result = doc_txn_delete(t, doc_seq_get_id(pa->children, i - 1)); if (result != XUI_OK) return result;
        }
        return XUI_OK;
    }
    for (i = doc_seq_size(doc_index_get(t->draft->index, ap)->children); i > ai; i--) {
        pa = doc_index_get(t->draft->index, ap);
        result = doc_txn_delete(t, doc_seq_get_id(pa->children, i - 1)); if (result != XUI_OK) return result;
    }
    for (i = bi; i > 0; i--) {
        pb = doc_index_get(t->draft->index, bp);
        result = doc_txn_delete(t, doc_seq_get_id(pb->children, i - 1)); if (result != XUI_OK) return result;
    }
    pa = doc_index_get(t->draft->index, parent); ai = doc_child_index(pa, ap); bi = doc_child_index(pa, bp);
    for (i = bi; i > ai + 1; i--) {
        pa = doc_index_get(t->draft->index, parent);
        result = doc_txn_delete(t, doc_seq_get_id(pa->children, i - 1)); if (result != XUI_OK) return result;
    }
    { xui_doc_position_t unused; return xuiDocumentTxnJoinBlocks(t, ap, bp, &unused); }
}
XUI_API int xuiDocumentTxnReplaceRange(xui_document_transaction t, const xui_doc_range_t* range,
    const char* text, uint64_t bytes, xui_doc_position_t* caret)
{
    xui_doc_position_t a, b, p; uint64_t start, i; int order, result = doc_txn_check(t, 0);
    if (result != XUI_OK) return result;
    if (!range || !caret || !doc_utf8(text, bytes)) return doc_txn_fail(t, XUI_ERROR_INVALID_ARGUMENT);
    result = doc_command_position(t, &range->tAnchor);
    if (result == XUI_OK) result = doc_command_position(t, &range->tCaret);
    if (result == XUI_OK) result = doc_position_compare(t->draft, &range->tAnchor, &range->tCaret, &order);
    if (result != XUI_OK) return doc_txn_fail(t, result);
    a = order <= 0 ? range->tAnchor : range->tCaret; b = order <= 0 ? range->tCaret : range->tAnchor;
    if (a.iKind == XUI_DOC_POSITION_SOURCE) {
        result = xuiDocumentTxnReplaceSource(t, a.iOffset, b.iOffset, text, bytes);
        if (result == XUI_OK) { *caret = a; caret->iOffset += bytes; caret->iAffinity = XUI_DOC_AFTER; }
        return result;
    }
    result = doc_txn_check(t, XUI_DOC_SEMANTIC); if (result != XUI_OK) return result;
    if (t->draft->profile == XUI_DOCUMENT_MARKDOWN && !t->parsing) {
        if (a.iKind != XUI_DOC_POSITION_TEXT || b.iKind != XUI_DOC_POSITION_TEXT || a.iNodeId != b.iNodeId ||
            (bytes && doc_index_get(t->draft->index, a.iNodeId)->kind == XUI_DOC_TEXT && (memchr(text, '\n', (size_t)bytes) || memchr(text, '\r', (size_t)bytes))))
            return doc_markdown_range(t, range, text, bytes, caret);
        result = doc_txn_text(t, a.iNodeId, a.iOffset, b.iOffset, text, bytes);
        if (result == XUI_OK) {
            *caret = a; caret->iOffset += bytes; caret->iAffinity = XUI_DOC_AFTER;
            if (!doc_position_valid(t->draft, caret)) *caret = doc_command_caret(t, DOC_ROOT,
                doc_seq_size(doc_index_get(t->draft->index, DOC_ROOT)->children), XUI_DOC_POSITION_GAP);
        }
        return result;
    }
    result = order ? doc_delete_range(t, a, b, &p) : XUI_OK;
    if (!order) p = a;
    if (result != XUI_OK) return result;
    if (!bytes) { *caret = p; return XUI_OK; }
    result = doc_text_caret(t, &p); if (result != XUI_OK) return doc_txn_fail(t, result);
    if (doc_index_get(t->draft->index, p.iNodeId)->kind != XUI_DOC_TEXT) {
        result = doc_txn_text(t, p.iNodeId, p.iOffset, p.iOffset, text, bytes);
        if (result == XUI_OK) { p.iOffset += bytes; *caret = p; } return result;
    }
    for (start = i = 0; i <= bytes; i++) {
        if (i != bytes && text[i] != '\r' && text[i] != '\n') continue;
        result = doc_txn_text(t, p.iNodeId, p.iOffset, p.iOffset, text + start, i - start);
        if (result != XUI_OK) return result;
        p.iOffset += i - start;
        if (i < bytes) {
            result = xuiDocumentTxnSplitBlock(t, &p, &p); if (result != XUI_OK) return result;
            if (text[i] == '\r' && i + 1 < bytes && text[i + 1] == '\n') i++;
        }
        start = i + 1;
    }
    p.iAffinity = XUI_DOC_AFTER; *caret = p; return XUI_OK;
}

typedef struct doc_range_copy { doc_state* state; xui_doc_position_t a, b; char* text; uint64_t length, capacity; } doc_range_copy;
static void doc_copy_append(doc_range_copy* c, const char* s, uint64_t bytes) { if (bytes) memcpy(c->text + c->length, s, (size_t)bytes); c->length += bytes; }
static void doc_copy_range_node(doc_range_copy* c, uint64_t id)
{
    doc_node* n = doc_index_get(c->state->index, id); xui_doc_position_t first = c->a, last = c->a;
    int ca, cb; uint64_t i, size = doc_text_kind(n->kind) ? doc_seq_size(n->text) : doc_seq_size(n->children);
    first.iNodeId = last.iNodeId = id; first.iOffset = 0; last.iOffset = size;
    first.iKind = last.iKind = doc_text_kind(n->kind) ? XUI_DOC_POSITION_TEXT : XUI_DOC_POSITION_GAP;
    doc_position_compare(c->state, &last, &c->a, &ca); doc_position_compare(c->state, &first, &c->b, &cb);
    if (ca <= 0 || cb >= 0) return;
    if (doc_text_kind(n->kind)) {
        uint64_t from = c->a.iNodeId == id ? c->a.iOffset : 0, to = c->b.iNodeId == id ? c->b.iOffset : size;
        doc_seq_read(n->text, from, c->text + c->length, to - from); c->length += to - from;
    }
    for (i = 0; i < doc_seq_size(n->children); i++) doc_copy_range_node(c, doc_seq_get_id(n->children, i));
    if (n->kind == XUI_DOC_SOFT_BREAK || n->kind == XUI_DOC_HARD_BREAK) doc_copy_append(c, "\n", 1);
    doc_position_compare(c->state, &last, &c->b, &cb);
    if (cb < 0) {
        doc_node* parent = doc_index_get(c->state->index, n->parent);
        int last_child = parent && doc_seq_get_id(parent->children, doc_seq_size(parent->children) - 1) == id;
        if (n->kind == XUI_DOC_CELL) { if (!last_child) doc_copy_append(c, "\t", 1); }
        else if ((doc_paragraph(n) || n->kind == XUI_DOC_CODE_BLOCK || n->kind == XUI_DOC_ROW) &&
            !(last_child && parent->kind == XUI_DOC_CELL)) doc_copy_append(c, "\n", 1);
    }
}
XUI_API int xuiDocumentSnapshotCopyRange(xui_document_snapshot s, const xui_doc_range_t* range, char** out, uint64_t* bytes)
{
    doc_range_copy c = {0}; int order, result;
    if (out) *out = NULL;
    if (bytes) *bytes = 0;
    if (!s || !range || !out || !bytes) return XUI_ERROR_INVALID_ARGUMENT;
    result = xuiDocumentSnapshotComparePositions(s, &range->tAnchor, &range->tCaret, &order); if (result != XUI_OK) return result;
    c.state = s->state; c.a = order <= 0 ? range->tAnchor : range->tCaret; c.b = order <= 0 ? range->tCaret : range->tAnchor;
    if (s->state->node_count > (UINT64_MAX - s->state->text_bytes - 1) / 2) return XUI_DOC_ERROR_LIMIT;
    c.capacity = c.a.iKind == XUI_DOC_POSITION_SOURCE ? c.b.iOffset - c.a.iOffset : s->state->text_bytes + s->state->node_count * 2;
    if (c.capacity >= SIZE_MAX) return XUI_DOC_ERROR_LIMIT;
    c.text = malloc((size_t)c.capacity + 1); if (!c.text) return XUI_ERROR_OUT_OF_MEMORY;
    if (c.a.iKind == XUI_DOC_POSITION_SOURCE) { doc_seq_read(s->state->source, c.a.iOffset, c.text, c.capacity); c.length = c.capacity; }
    else if (order) doc_copy_range_node(&c, DOC_ROOT);
    c.text[c.length] = 0; *out = c.text; *bytes = c.length; return XUI_OK;
}
