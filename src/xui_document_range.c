#include "xui_document_internal.h"

typedef struct doc_mark_target { uint64_t id, start, end; } doc_mark_target;
typedef struct doc_mark_collect {
    doc_mark_target* targets;
    uint64_t count, capacity;
    const xui_doc_position_t *first, *last;
    int error;
} doc_mark_collect;
static void doc_collect_marks(doc_state* s, uint64_t id, doc_mark_collect* c)
{
    doc_node* n = doc_index_get(s->index, id);
    xui_doc_position_t first = *c->first, last = *c->first; uint64_t i; int before, after;
    if (c->error) return;
    first.iNodeId = last.iNodeId = id; first.iOffset = 0;
    first.iKind = last.iKind = doc_text_kind(n->kind) ? XUI_DOC_POSITION_TEXT : XUI_DOC_POSITION_GAP;
    last.iOffset = doc_text_kind(n->kind) ? doc_seq_size(n->text) : doc_seq_size(n->children);
    doc_position_compare(s, &last, c->first, &before); doc_position_compare(s, &first, c->last, &after);
    if (before <= 0 || after >= 0) return;
    if (n->kind == XUI_DOC_TEXT) {
        if (c->count == c->capacity) {
            uint64_t capacity = c->capacity ? c->capacity * 2 : 16; doc_mark_target* targets;
            if (capacity < c->capacity || capacity > SIZE_MAX / sizeof(*targets)) { c->error = XUI_DOC_ERROR_LIMIT; return; }
            targets = doc_realloc(s->allocator, c->targets, (size_t)capacity * sizeof(*targets));
            if (!targets) { c->error = XUI_ERROR_OUT_OF_MEMORY; return; }
            c->targets = targets; c->capacity = capacity;
        }
        doc_mark_target* target = &c->targets[c->count++];
        target->id = id; target->start = id == c->first->iNodeId ? c->first->iOffset : 0;
        target->end = id == c->last->iNodeId ? c->last->iOffset : doc_seq_size(n->text);
    }
    for (i = 0; i < doc_seq_size(n->children) && !c->error; i++) doc_collect_marks(s, doc_seq_get_id(n->children, i), c);
}
int doc_marks_range(xui_document_transaction t, const xui_doc_range_t* range, uint32_t set, uint32_t clear)
{
    doc_mark_collect c = {0}; xui_doc_range_t part = *range;
    uint64_t i; int order, result;
    result = doc_position_compare(t->draft, &range->tAnchor, &range->tCaret, &order);
    if (result != XUI_OK) return doc_txn_fail(t, result);
    c.first = order < 0 ? &range->tAnchor : &range->tCaret; c.last = order < 0 ? &range->tCaret : &range->tAnchor;
    doc_collect_marks(t->draft, DOC_ROOT, &c);
    result = c.error;
    part.tAnchor.iKind = part.tCaret.iKind = XUI_DOC_POSITION_TEXT;
    for (i = c.count; i > 0 && result == XUI_OK; i--) {
        doc_mark_target* target = &c.targets[i - 1];
        part.tAnchor.iNodeId = part.tCaret.iNodeId = target->id;
        part.tAnchor.iOffset = target->start; part.tCaret.iOffset = target->end;
        result = xuiDocumentTxnSetMarks(t, &part, set, clear);
        if (result != XUI_OK) break;
    }
    doc_free(c.targets); return result == XUI_OK ? result : doc_txn_fail(t, result);
}
XUI_API int xuiDocumentSnapshotQueryMarks(xui_document_snapshot snapshot, const xui_doc_range_t* range, uint32_t* common, uint32_t* mixed)
{
    doc_mark_collect c = {0}; int order, result; uint64_t i; uint32_t any = 0;
    if (!snapshot || !range || !common || !mixed) return XUI_ERROR_INVALID_ARGUMENT;
    *common = *mixed = 0;
    result = xuiDocumentSnapshotComparePositions(snapshot, &range->tAnchor, &range->tCaret, &order);
    if (result != XUI_OK) return result;
    if (range->tAnchor.iKind == XUI_DOC_POSITION_SOURCE) return XUI_DOC_ERROR_DOMAIN;
    if (!order) {
        doc_node* n = doc_index_get(snapshot->state->index, range->tCaret.iNodeId);
        if (range->tCaret.iKind == XUI_DOC_POSITION_GAP && doc_seq_size(n->children)) {
            uint64_t index = range->tCaret.iOffset ? range->tCaret.iOffset - 1 : 0;
            n = doc_index_get(snapshot->state->index, doc_seq_get_id(n->children, index));
        }
        if (n && n->kind == XUI_DOC_TEXT) *common = n->attrs.iMarks;
        return XUI_OK;
    }
    c.first = order < 0 ? &range->tAnchor : &range->tCaret; c.last = order < 0 ? &range->tCaret : &range->tAnchor;
    doc_collect_marks(snapshot->state, DOC_ROOT, &c);
    if (!c.error && c.count) {
        *common = UINT32_MAX;
        for (i = 0; i < c.count; i++) {
            uint32_t marks = doc_index_get(snapshot->state->index, c.targets[i].id)->attrs.iMarks;
            any |= marks; *common &= marks;
        }
        *mixed = any & ~*common;
    }
    doc_free(c.targets); return c.error;
}
typedef struct doc_plain { char* text; uint64_t size, capacity; int error; } doc_plain;
static void doc_plain_append(doc_plain* b, const char* s, uint64_t size)
{
    if (b->error || !size) return;
    if (size > UINT64_MAX - b->size || size > b->capacity - b->size) { b->error = XUI_DOC_ERROR_LIMIT; return; }
    memcpy(b->text + b->size, s, (size_t)size); b->size += size;
}
static void doc_plain_node(doc_state* s, uint64_t id, doc_plain* b)
{
    doc_node* n = doc_index_get(s->index, id); uint64_t i;
    if (n->text && !b->error) {
        uint64_t size = doc_seq_size(n->text);
        if (size > b->capacity - b->size) { b->error = XUI_DOC_ERROR_LIMIT; return; }
        doc_seq_read(n->text, 0, b->text + b->size, size); b->size += size;
    }
    if (n->kind == XUI_DOC_SOFT_BREAK || n->kind == XUI_DOC_HARD_BREAK) doc_plain_append(b, "\n", 1);
    for (i = 0; i < doc_seq_size(n->children); i++) {
        if (i && n->kind == XUI_DOC_ROW) doc_plain_append(b, "\t", 1);
        doc_plain_node(s, doc_seq_get_id(n->children, i), b);
    }
    if (n->kind == XUI_DOC_PARAGRAPH || n->kind == XUI_DOC_HEADING || n->kind == XUI_DOC_CODE_BLOCK || n->kind == XUI_DOC_ROW) {
        doc_node* parent = doc_index_get(s->index, n->parent);
        if (parent && parent->kind == XUI_DOC_CELL && doc_seq_get_id(parent->children, doc_seq_size(parent->children) - 1) == id) return;
        doc_plain_append(b, "\n", 1);
    }
}
XUI_API int xuiDocumentSnapshotCopyPlainText(xui_document_snapshot s, char** out, uint64_t* bytes)
{
    doc_plain b = {0};
    if (out) *out = NULL;
    if (bytes) *bytes = 0;
    if (!s || !out || !bytes) return XUI_ERROR_INVALID_ARGUMENT;
    if (s->state->node_count > (UINT64_MAX - s->state->text_bytes - 1) / 2) return XUI_DOC_ERROR_LIMIT;
    b.capacity = s->state->text_bytes + 2 * s->state->node_count;
    if (b.capacity >= SIZE_MAX) return XUI_DOC_ERROR_LIMIT;
    b.text = malloc((size_t)b.capacity + 1); if (!b.text) return XUI_ERROR_OUT_OF_MEMORY;
    doc_plain_node(s->state, DOC_ROOT, &b);
    if (b.error) { free(b.text); return b.error; }
    b.text[b.size] = 0; *out = b.text; *bytes = b.size; return XUI_OK;
}
