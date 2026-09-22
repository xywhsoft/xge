#include "xui_document_internal.h"

int doc_position_valid(doc_state* s, const xui_doc_position_t* p)
{
    doc_node* n;
    if (!p || p->iSize != sizeof(*p) || p->iAffinity > XUI_DOC_AFTER) return 0;
    if (p->iKind == XUI_DOC_POSITION_SOURCE) return s->profile == XUI_DOCUMENT_MARKDOWN && p->iNodeId == DOC_ROOT && doc_seq_boundary(s->source, p->iOffset);
    n = doc_index_get(s->index, p->iNodeId);
    if (!n) return 0;
    if (p->iKind == XUI_DOC_POSITION_TEXT) return doc_text_kind(n->kind) && doc_seq_boundary(n->text, p->iOffset);
    return p->iKind == XUI_DOC_POSITION_GAP && !doc_text_kind(n->kind) && p->iOffset <= doc_seq_size(n->children);
}
int doc_position_compare(doc_state* s, const xui_doc_position_t* a, const xui_doc_position_t* b, int* order)
{
    uint64_t pa[DOC_MAX_DEPTH], pb[DOC_MAX_DEPTH]; unsigned na = 0, nb = 0;
    doc_node* n;
    if (!order || !doc_position_valid(s, a) || !doc_position_valid(s, b)) return XUI_ERROR_INVALID_ARGUMENT;
    if (a->iKind == XUI_DOC_POSITION_SOURCE || b->iKind == XUI_DOC_POSITION_SOURCE) {
        if (a->iKind != b->iKind) return XUI_DOC_ERROR_DOMAIN;
        *order = a->iOffset < b->iOffset ? -1 : a->iOffset != b->iOffset; return XUI_OK;
    }
    if (a->iNodeId == b->iNodeId) { *order = a->iOffset < b->iOffset ? -1 : a->iOffset != b->iOffset; return XUI_OK; }
    for (n = doc_index_get(s->index, a->iNodeId); n && na < DOC_MAX_DEPTH; n = doc_index_get(s->index, n->parent)) pa[na++] = n->id;
    for (n = doc_index_get(s->index, b->iNodeId); n && nb < DOC_MAX_DEPTH; n = doc_index_get(s->index, n->parent)) pb[nb++] = n->id;
    while (na && nb && pa[na - 1] == pb[nb - 1]) { na--; nb--; }
    if (!na) { n = doc_index_get(s->index, a->iNodeId); *order = a->iOffset <= doc_child_index(n, pb[nb - 1]) ? -1 : 1; }
    else if (!nb) { n = doc_index_get(s->index, b->iNodeId); *order = b->iOffset <= doc_child_index(n, pa[na - 1]) ? 1 : -1; }
    else {
        n = doc_index_get(s->index, doc_index_get(s->index, pa[na - 1])->parent);
        *order = doc_child_index(n, pa[na - 1]) < doc_child_index(n, pb[nb - 1]) ? -1 : 1;
    }
    return XUI_OK;
}
XUI_API int xuiDocumentSnapshotComparePositions(xui_document_snapshot s, const xui_doc_position_t* a, const xui_doc_position_t* b, int* order)
{
    if (!s || !a || !b || !order) return XUI_ERROR_INVALID_ARGUMENT;
    if (a->iDocumentId != s->identity || b->iDocumentId != s->identity || a->iRevision != s->revision || b->iRevision != s->revision) return XUI_DOC_ERROR_STALE;
    return doc_position_compare(s->state, a, b, order);
}

static uint64_t doc_map_offset(uint64_t position, uint32_t affinity, uint64_t start,
    uint64_t old_length, uint64_t new_length, int* deleted)
{
    if (position < start) return position;
    if (position > start + old_length) return position - old_length + new_length;
    if (old_length && position > start && position < start + old_length) *deleted = 1;
    if (position == start + old_length && old_length) return start + new_length;
    return affinity == XUI_DOC_AFTER ? start + new_length : start;
}
typedef struct doc_source_search {
    doc_node* best;
    uint64_t offset, distance, span;
} doc_source_search;
static void doc_find_source(doc_state* state, uint64_t id, doc_source_search* search)
{
    doc_node* p = doc_index_get(state->index, id);
    uint64_t i, count;
    if (!p) return;
    if (doc_text_kind(p->kind) && p->source_start != DOC_NONE && p->source_end != DOC_NONE) {
        uint64_t distance = search->offset < p->source_start ? p->source_start - search->offset :
            (search->offset > p->source_end ? search->offset - p->source_end : 0);
        uint64_t span = p->source_end - p->source_start;
        if (!search->best || distance < search->distance || (distance == search->distance && span < search->span)) {
            search->best = p; search->distance = distance; search->span = span;
        }
    }
    count = doc_seq_size(p->children);
    for (i = 0; i < count; i++) doc_find_source(state, doc_seq_get_id(p->children, i), search);
}
XUI_API int xuiDocumentSourceToPosition(xui_document_snapshot s, uint64_t offset, xui_doc_position_t* out, int* mapping)
{
    doc_source_search search = {0};
    doc_node* p;
    uint64_t n;
    if (!s || !out || !mapping || s->state->profile != XUI_DOCUMENT_MARKDOWN || offset > doc_seq_size(s->state->source)) return XUI_ERROR_INVALID_ARGUMENT;
    memset(out, 0, sizeof(*out)); out->iSize = sizeof(*out);
    out->iDocumentId = s->identity; out->iRevision = s->revision; out->iAffinity = XUI_DOC_AFTER;
    search.offset = offset; doc_find_source(s->state, DOC_ROOT, &search); p = search.best;
    if (p && offset == doc_seq_size(s->state->source) && offset > p->source_end) {
        uint64_t at = offset; unsigned breaks = 0; char previous = 0;
        while (at > p->source_end && breaks < 2) {
            char c; doc_seq_read(s->state->source, --at, &c, 1);
            if (c == '\n' || c == '\r') { if (c != '\r' || previous != '\n') breaks++; }
            else if (c != ' ' && c != '\t') break;
            previous = c;
        }
        if (breaks >= 2) {
            out->iNodeId = DOC_ROOT; out->iKind = XUI_DOC_POSITION_GAP;
            out->iOffset = doc_seq_size(doc_index_get(s->state->index, DOC_ROOT)->children); *mapping = XUI_DOC_MAP_APPROXIMATE; return XUI_OK;
        }
    }
    if (!p) {
        out->iNodeId = DOC_ROOT; out->iKind = XUI_DOC_POSITION_GAP; *mapping = XUI_DOC_MAP_APPROXIMATE;
        return XUI_OK;
    }
    out->iKind = XUI_DOC_POSITION_TEXT; out->iNodeId = p->id; n = doc_seq_size(p->text);
    if (offset <= p->source_start) out->iOffset = 0;
    else if (offset >= p->source_end) out->iOffset = n;
    else if (p->source_exact && p->source_end - p->source_start == n) out->iOffset = offset - p->source_start;
    else out->iOffset = 0;
    while (out->iOffset && !doc_seq_boundary(p->text, out->iOffset)) out->iOffset--;
    *mapping = p->source_exact && !search.distance && out->iOffset == offset - p->source_start ? XUI_DOC_MAP_EXACT : XUI_DOC_MAP_APPROXIMATE;
    return XUI_OK;
}
XUI_API int xuiDocumentPositionToSource(xui_document_snapshot s, const xui_doc_position_t* p, uint64_t* out, int* mapping)
{
    doc_node* node;
    if (!s || !p || !out || !mapping || p->iSize != sizeof(*p) || s->state->profile != XUI_DOCUMENT_MARKDOWN) return XUI_ERROR_INVALID_ARGUMENT;
    if (p->iDocumentId != s->identity || p->iRevision != s->revision) return XUI_DOC_ERROR_STALE;
    if (!doc_position_valid(s->state, p)) return XUI_ERROR_INVALID_ARGUMENT;
    if (p->iKind == XUI_DOC_POSITION_SOURCE) { *out = p->iOffset; *mapping = XUI_DOC_MAP_EXACT; return XUI_OK; }
    node = doc_index_get(s->state->index, p->iNodeId);
    if (p->iKind == XUI_DOC_POSITION_GAP) {
        if (p->iNodeId == DOC_ROOT && (!p->iOffset || p->iOffset == doc_seq_size(node->children))) {
            *out = p->iOffset ? doc_seq_size(s->state->source) : 0; *mapping = XUI_DOC_MAP_EXACT; return XUI_OK;
        }
        if (p->iOffset < doc_seq_size(node->children)) node = doc_index_get(s->state->index, doc_seq_get_id(node->children, p->iOffset));
        if (!node || node->source_start == DOC_NONE) return XUI_ERROR_NOT_FOUND;
        *out = node->source_start; *mapping = XUI_DOC_MAP_APPROXIMATE; return XUI_OK;
    }
    if (!node || node->source_start == DOC_NONE) return XUI_ERROR_NOT_FOUND;
    if (p->iKind != XUI_DOC_POSITION_TEXT || p->iOffset > doc_seq_size(node->text) || !doc_seq_boundary(node->text, p->iOffset)) return XUI_ERROR_INVALID_ARGUMENT;
    if (node->source_exact && node->source_end - node->source_start == doc_seq_size(node->text)) {
        *out = node->source_start + p->iOffset; *mapping = XUI_DOC_MAP_EXACT;
    } else {
        *out = p->iOffset == doc_seq_size(node->text) && p->iOffset ? node->source_end : node->source_start;
        *mapping = XUI_DOC_MAP_APPROXIMATE;
    }
    return XUI_OK;
}
/* Resolve ancestry at an operation boundary, including subtrees moved earlier
 * in the same transaction. Looking only at the before/after trees loses the
 * deletion boundary when siblings or ancestors have also been edited. */
static uint64_t doc_map_parent_at(xui_document_change_set c, uint64_t id, uint64_t applied)
{
    doc_node* n = doc_index_get(c->before->index, id); uint64_t parent, i;
    if (!n) n = doc_index_get(c->after->index, id);
    parent = n ? n->parent : 0;
    for (i = 0; i < applied; i++) {
        const xui_doc_operation_t* op = &c->ops[c->undo ? c->count - i - 1 : i];
        if (op->iNodeId != id) continue;
        if (op->iKind == XUI_DOC_OP_MOVE) parent = c->undo ? op->iParentId : op->iOtherNodeId;
        else if (op->iKind == (uint32_t)(c->undo ? XUI_DOC_OP_DELETE : XUI_DOC_OP_INSERT)) parent = op->iParentId;
    }
    return parent;
}
static int doc_map_descendant_at(xui_document_change_set c, uint64_t node, uint64_t ancestor, uint64_t applied)
{
    unsigned depth;
    for (depth = 0; node && depth < DOC_MAX_DEPTH; depth++) {
        if (node == ancestor) return 1;
        if (node == DOC_ROOT) return 0;
        node = doc_map_parent_at(c, node, applied);
    }
    return 0;
}
XUI_API int xuiDocumentMapPosition(xui_document_change_set c, const xui_doc_position_t* before, xui_doc_position_t* after, int* mapping)
{
    doc_node* node;
    uint64_t i;
    int deleted = 0;
    if (!c || !before || !after || !mapping || before->iSize != sizeof(*before)) return XUI_ERROR_INVALID_ARGUMENT;
    if (before->iDocumentId != c->identity || before->iRevision != c->before_revision) return XUI_DOC_ERROR_STALE;
    if (!doc_position_valid(c->before, before)) return XUI_ERROR_INVALID_ARGUMENT;
    *after = *before; *mapping = XUI_DOC_MAP_EXACT;
    if (c->before->profile == XUI_DOCUMENT_MARKDOWN && c->after->profile == XUI_DOCUMENT_MARKDOWN) {
        struct xui_doc_snapshot_t old = {0}, next = {0};
        uint64_t source;
        int previous_mapping, result;
        old.state = c->before; old.identity = c->identity; old.revision = c->before_revision;
        next.state = c->after; next.identity = c->identity; next.revision = c->after_revision;
        result = xuiDocumentPositionToSource(&old, before, &source, &previous_mapping);
        if (result == XUI_OK) {
            for (i = 0; i < c->count; i++) {
                const xui_doc_operation_t* op = &c->ops[c->undo ? c->count - i - 1 : i];
                if (op->iKind == XUI_DOC_OP_SOURCE) source = doc_map_offset(source, before->iAffinity, op->iOffset,
                    c->undo ? op->iNewLength : op->iOldLength, c->undo ? op->iOldLength : op->iNewLength, &deleted);
            }
            if (before->iKind == XUI_DOC_POSITION_SOURCE) {
                after->iOffset = source; after->iRevision = c->after_revision; *mapping = deleted ? XUI_DOC_MAP_DELETED : XUI_DOC_MAP_EXACT; return XUI_OK;
            }
            result = xuiDocumentSourceToPosition(&next, source, after, mapping);
            if (result == XUI_OK) {
                after->iAffinity = before->iAffinity;
                if (deleted) *mapping = XUI_DOC_MAP_DELETED;
                else if (previous_mapping != XUI_DOC_MAP_EXACT) *mapping = XUI_DOC_MAP_APPROXIMATE;
            }
            return result;
        }
    }
    for (i = 0; i < c->count; i++) {
        const xui_doc_operation_t* op = &c->ops[c->undo ? c->count - i - 1 : i];
        if (op->iKind == (uint32_t)(c->undo ? XUI_DOC_OP_INSERT : XUI_DOC_OP_DELETE) &&
            doc_map_descendant_at(c, after->iNodeId, op->iNodeId, i)) {
            after->iNodeId = op->iParentId; after->iKind = XUI_DOC_POSITION_GAP; after->iOffset = op->iOffset;
            deleted = 1; continue;
        }
        if (op->iKind == XUI_DOC_OP_TEXT && after->iKind == XUI_DOC_POSITION_TEXT && after->iNodeId == op->iNodeId) {
            after->iOffset = doc_map_offset(after->iOffset, after->iAffinity, op->iOffset,
                c->undo ? op->iNewLength : op->iOldLength, c->undo ? op->iOldLength : op->iNewLength, &deleted);
        } else if (op->iKind == XUI_DOC_OP_SPLIT && after->iKind == XUI_DOC_POSITION_TEXT) {
            if (c->undo && after->iNodeId == op->iOtherNodeId) { after->iNodeId = op->iNodeId; after->iOffset += op->iOffset; }
            else if (!c->undo && after->iNodeId == op->iNodeId &&
                (after->iOffset > op->iOffset || (after->iOffset == op->iOffset && after->iAffinity == XUI_DOC_AFTER))) {
                after->iNodeId = op->iOtherNodeId; after->iOffset -= op->iOffset;
            }
        } else if (op->iKind == XUI_DOC_OP_MOVE && after->iKind == XUI_DOC_POSITION_GAP) {
            uint64_t from = c->undo ? op->iOtherNodeId : op->iParentId, to = c->undo ? op->iParentId : op->iOtherNodeId;
            uint64_t old_index = c->undo ? op->iNewLength : op->iOffset, new_index = c->undo ? op->iOffset : op->iNewLength;
            if (after->iNodeId == from) after->iOffset = doc_map_offset(after->iOffset, after->iAffinity, old_index, 1, 0, &deleted);
            if (after->iNodeId == to) after->iOffset = doc_map_offset(after->iOffset, after->iAffinity, new_index, 0, 1, &deleted);
        } else if ((op->iKind == XUI_DOC_OP_INSERT || op->iKind == XUI_DOC_OP_DELETE) &&
            after->iKind == XUI_DOC_POSITION_GAP && after->iNodeId == op->iParentId) {
            after->iOffset = doc_map_offset(after->iOffset, after->iAffinity, op->iOffset,
                c->undo ? op->iNewLength : op->iOldLength, c->undo ? op->iOldLength : op->iNewLength, &deleted);
        }
    }
    node = doc_index_get(c->after->index, after->iNodeId);
    if (!node) {
        doc_node* previous = doc_index_get(c->before->index, before->iNodeId);
        deleted = 1;
        while (previous && !doc_index_get(c->after->index, previous->id)) {
            doc_node* parent = doc_index_get(c->before->index, previous->parent);
            if (!parent) break;
            after->iOffset = doc_child_index(parent, previous->id);
            previous = parent;
        }
        after->iNodeId = previous ? previous->id : DOC_ROOT;
        after->iKind = XUI_DOC_POSITION_GAP;
        node = doc_index_get(c->after->index, after->iNodeId);
    }
    if (!node) return XUI_ERROR_NOT_FOUND;
    { uint64_t length = after->iKind == XUI_DOC_POSITION_TEXT ? doc_seq_size(node->text) : doc_seq_size(node->children);
      if (after->iOffset > length) after->iOffset = length;
    }
    after->iRevision = c->after_revision;
    if (deleted) *mapping = XUI_DOC_MAP_DELETED;
    return XUI_OK;
}
