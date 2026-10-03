#include "../xui_config.h"
#if XUI_ENABLE_DOCUMENT
#include "xui_document_internal.h"

int doc_position_valid(doc_state* s, const xui_doc_position_t* p)
{
    doc_node* n;
    if (!p || p->iSize != sizeof(*p) || p->iAffinity > XUI_DOC_AFTER || p->iInputGeneration) return 0;
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
    uint64_t offset;
    unsigned affinity;
    doc_source_hit hit;
} doc_source_search;
static void doc_find_source(doc_state* state, uint64_t id, doc_source_search* search)
{
    doc_node* p = doc_index_get(state->index, id);
    doc_node_source_range range;
    uint64_t i, count;
    if (!p) return;
    doc_node_source_range_get(state, p, &range);
    if (doc_text_kind(p->kind)) {
        doc_source_hit hit = {0}; int found = doc_source_map_source(state, p, search->offset, search->affinity, &hit);
        if (!found && range.source_start != DOC_NONE && range.source_end != DOC_NONE) {
            uint64_t bytes = doc_seq_size(p->text);
            hit.distance = search->offset < range.source_start ? range.source_start - search->offset :
                (search->offset > range.source_end ? search->offset - range.source_end : 0);
            hit.span = range.source_end - range.source_start; hit.mapping = XUI_DOC_MAP_APPROXIMATE;
            hit.offset = search->offset >= range.source_end ? bytes : 0;
            if (!hit.distance && p->source_exact && hit.span == bytes) {
                hit.offset = search->offset - range.source_start; hit.mapping = XUI_DOC_MAP_EXACT;
            }
            found = 1;
        }
        if (found && (!search->best || doc_source_better(&hit, &search->hit, search->affinity))) {
            search->best = p; search->hit = hit;
        }
    }
    if (!doc_seq_size(p->children) && !doc_seq_size(p->text) && range.syntax_start != DOC_NONE &&
        search->offset >= range.syntax_start && search->offset <= range.syntax_end) {
        doc_source_hit hit = {0}; hit.span = range.syntax_end - range.syntax_start; hit.mapping = XUI_DOC_MAP_SYNTAX;
        if (!search->best || doc_source_better(&hit, &search->hit, search->affinity)) {
            search->best = p; search->hit = hit;
        }
    }
    count = doc_seq_size(p->children);
    for (i = 0; i < count; i++) doc_find_source(state, doc_seq_get_id(p->children, i), search);
}
static int doc_source_block_bound(doc_state* state, doc_node* root, uint64_t index,
    uint64_t offset, uint64_t* distance)
{
    doc_node* block = doc_index_get(state->index, doc_seq_get_id(root->children, index));
    doc_node_source_range range;
    if (!block) return 0;
    doc_node_source_range_get(state, block, &range);
    if (range.syntax_start == DOC_NONE || range.syntax_end == DOC_NONE ||
        range.syntax_start > range.syntax_end) return 0;
    *distance = offset < range.syntax_start ? range.syntax_start - offset :
        offset > range.syntax_end ? offset - range.syntax_end : 0;
    return 1;
}
/* Probe outward to establish a distance bound, then visit the eligible blocks
 * in document order. The second pass preserves the full DFS tie semantics. */
static int doc_find_source_indexed(doc_state* state, doc_source_search* search)
{
    doc_node* root = doc_index_get(state->index, DOC_ROOT);
    doc_source_search probe = *search;
    uint64_t count, lo, hi, left, right, first, last, i;
    if (!root || !state->source_blocks_indexed || !(count = doc_seq_size(root->children))) return 0;
    lo = 0; hi = count;
    while (lo < hi) {
        doc_node* block;
        doc_node_source_range range;
        uint64_t mid = lo + (hi - lo) / 2;
        block = doc_index_get(state->index, doc_seq_get_id(root->children, mid));
        if (!block) return 0;
        doc_node_source_range_get(state, block, &range);
        if (range.syntax_end == DOC_NONE) return 0;
        if (range.syntax_end < search->offset) lo = mid + 1;
        else hi = mid;
    }
    left = right = lo; first = count; last = 0;
    while (left || right < count) {
        uint64_t left_distance = UINT64_MAX, right_distance = UINT64_MAX, at;
        if (left && !doc_source_block_bound(state, root, left - 1, search->offset, &left_distance)) return 0;
        if (right < count && !doc_source_block_bound(state, root, right, search->offset, &right_distance)) return 0;
        if (probe.best && left_distance > probe.hit.distance && right_distance > probe.hit.distance) break;
        at = left_distance <= right_distance ? --left : right++;
        doc_find_source(state, doc_seq_get_id(root->children, at), &probe);
        if (at < first) first = at;
        if (at >= last) last = at + 1;
    }
    if (first == count) return 0;
    for (i = first; i < last; i++)
        doc_find_source(state, doc_seq_get_id(root->children, i), search);
    return 1;
}
/* The full scan remains an internal correctness oracle and a fallback for
 * documents whose source hits cannot be confined to ordered block ranges. */
int doc_source_find_position(doc_state* state, uint64_t offset, unsigned affinity,
    int full_scan, uint64_t* node_id, doc_source_hit* hit)
{
    doc_source_search search = {0};
    if (!state || !node_id || !hit || affinity > XUI_DOC_AFTER) return XUI_ERROR_INVALID_ARGUMENT;
    search.offset = offset; search.affinity = affinity;
    if (full_scan || !doc_find_source_indexed(state, &search))
        doc_find_source(state, DOC_ROOT, &search);
    *node_id = search.best ? search.best->id : 0;
    *hit = search.hit;
    return XUI_OK;
}
XUI_API int xuiDocumentSourceToPosition(xui_document_snapshot s, uint64_t offset, xui_doc_position_t* out, int* mapping)
{
    return xuiDocumentSourceToPositionEx(s, offset, XUI_DOC_AFTER, out, mapping);
}
XUI_API int xuiDocumentSourceToPositionEx(xui_document_snapshot s, uint64_t offset, uint32_t affinity, xui_doc_position_t* out, int* mapping)
{
    doc_source_hit hit = {0};
    doc_node* p;
    uint64_t node_id = 0;
    doc_node_source_range range;
    if (!s || !out || !mapping || affinity > XUI_DOC_AFTER || s->state->profile != XUI_DOCUMENT_MARKDOWN || offset > doc_seq_size(s->state->source)) return XUI_ERROR_INVALID_ARGUMENT;
    if (!doc_seq_boundary(s->state->source, offset)) return XUI_DOC_ERROR_UTF8;
    memset(out, 0, sizeof(*out)); out->iSize = sizeof(*out);
    out->iDocumentId = s->identity; out->iRevision = s->revision; out->iAffinity = affinity;
    doc_source_find_position(s->state, offset, affinity, 0, &node_id, &hit);
    p = node_id ? doc_index_get(s->state->index, node_id) : NULL;
    if (p) doc_node_source_range_get(s->state, p, &range);
    if (p && offset == doc_seq_size(s->state->source) && offset > range.source_end) {
        uint64_t at = offset; unsigned breaks = 0; char previous = 0;
        while (at > range.source_end && breaks < 2) {
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
    out->iKind = doc_text_kind(p->kind) ? XUI_DOC_POSITION_TEXT : XUI_DOC_POSITION_GAP;
    out->iNodeId = p->id; out->iOffset = hit.offset; *mapping = hit.mapping;
    if (out->iKind == XUI_DOC_POSITION_TEXT) while (out->iOffset && !doc_seq_boundary(p->text, out->iOffset)) {
        out->iOffset--; *mapping = XUI_DOC_MAP_APPROXIMATE;
    }
    if (*mapping == XUI_DOC_MAP_EXACT && p->provenance) {
        uint64_t reverse; int quality;
        if (doc_source_map_text(s->state, p, out->iOffset, affinity, &reverse, &quality) == XUI_OK &&
            (reverse != offset || quality != XUI_DOC_MAP_EXACT)) *mapping = quality == XUI_DOC_MAP_COLLAPSED ? quality : XUI_DOC_MAP_SYNTAX;
    }
    return XUI_OK;
}
static int doc_source_node_boundary(doc_state* state, uint64_t id, int end, uint64_t* offset)
{
    doc_node* node = doc_index_get(state->index, id);
    doc_node_source_range range; uint64_t count;
    if (!node) return 0;
    doc_node_source_range_get(state, node, &range);
    if (range.syntax_start != DOC_NONE) {
        *offset = end ? range.syntax_end : range.syntax_start; return 1;
    }
    if (range.source_start != DOC_NONE) {
        *offset = end ? range.source_end : range.source_start; return 1;
    }
    count = doc_seq_size(node->children);
    if (end) {
        while (count) if (doc_source_node_boundary(state,
            doc_seq_get_id(node->children, --count), 1, offset)) return 1;
    } else {
        uint64_t i;
        for (i = 0; i < count; i++) if (doc_source_node_boundary(state,
            doc_seq_get_id(node->children, i), 0, offset)) return 1;
    }
    return 0;
}
static int doc_source_gap_approximate(doc_state* state, uint64_t id, uint64_t gap,
    uint32_t affinity, uint64_t* offset)
{
    doc_node* node = doc_index_get(state->index, id);
    while (node) {
        uint64_t count = doc_seq_size(node->children), i;
        if (affinity == XUI_DOC_AFTER) {
            for (i = gap; i; i--) if (doc_source_node_boundary(state,
                doc_seq_get_id(node->children, i - 1), 1, offset)) return 1;
            for (i = gap; i < count; i++) if (doc_source_node_boundary(state,
                doc_seq_get_id(node->children, i), 0, offset)) return 1;
        } else {
            for (i = gap; i < count; i++) if (doc_source_node_boundary(state,
                doc_seq_get_id(node->children, i), 0, offset)) return 1;
            for (i = gap; i; i--) if (doc_source_node_boundary(state,
                doc_seq_get_id(node->children, i - 1), 1, offset)) return 1;
        }
        if (!node->parent) break;
        {
            doc_node* parent = doc_index_get(state->index, node->parent);
            if (!parent) break;
            gap = doc_child_index(parent, node->id);
            if (gap == DOC_NONE) break;
            node = parent;
        }
    }
    if (node) {
        doc_node_source_range range;
        doc_node_source_range_get(state, node, &range);
        if (range.syntax_start != DOC_NONE) {
            *offset = affinity == XUI_DOC_AFTER ? range.syntax_end : range.syntax_start; return 1;
        }
        if (range.source_start != DOC_NONE) {
            *offset = affinity == XUI_DOC_AFTER ? range.source_end : range.source_start; return 1;
        }
    }
    return 0;
}
XUI_API int xuiDocumentPositionToSource(xui_document_snapshot s, const xui_doc_position_t* p, uint64_t* out, int* mapping)
{
    doc_node* node;
    doc_node_source_range range;
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
        if (node) doc_node_source_range_get(s->state, node, &range);
        if (node && range.syntax_start != DOC_NONE) {
            *out = p->iNodeId == node->id && p->iOffset ? range.syntax_end : range.syntax_start;
            *mapping = p->iNodeId == node->id && node->kind == XUI_DOC_CELL &&
                !doc_seq_size(node->children) && range.syntax_start == range.syntax_end ?
                XUI_DOC_MAP_SYNTAX : XUI_DOC_MAP_APPROXIMATE;
            return XUI_OK;
        }
        if (!node || range.source_start == DOC_NONE) {
            if (doc_source_gap_approximate(s->state, p->iNodeId, p->iOffset,
                p->iAffinity, out) && *out <= doc_seq_size(s->state->source) &&
                doc_seq_boundary(s->state->source, *out)) {
                *mapping = XUI_DOC_MAP_APPROXIMATE; return XUI_OK;
            }
            return XUI_ERROR_NOT_FOUND;
        }
        *out = range.source_start; *mapping = XUI_DOC_MAP_APPROXIMATE; return XUI_OK;
    }
    if (doc_source_map_text(s->state, node, p->iOffset, p->iAffinity, out, mapping) == XUI_OK) return XUI_OK;
    if (node) doc_node_source_range_get(s->state, node, &range);
    if (node && range.source_start == DOC_NONE && !doc_seq_size(node->text) && range.syntax_start != DOC_NONE) {
        *out = range.syntax_start; *mapping = XUI_DOC_MAP_APPROXIMATE; return XUI_OK;
    }
    if (!node || range.source_start == DOC_NONE) return XUI_ERROR_NOT_FOUND;
    if (p->iKind != XUI_DOC_POSITION_TEXT || p->iOffset > doc_seq_size(node->text) || !doc_seq_boundary(node->text, p->iOffset)) return XUI_ERROR_INVALID_ARGUMENT;
    if (node->source_exact && range.source_end - range.source_start == doc_seq_size(node->text)) {
        *out = range.source_start + p->iOffset; *mapping = XUI_DOC_MAP_EXACT;
    } else {
        *out = p->iOffset == doc_seq_size(node->text) && p->iOffset ? range.source_end : range.source_start;
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
    xui_doc_position_t input;
    doc_node* node;
    uint64_t i;
    int deleted = 0;
    if (!c || !before || !after || !mapping || before->iSize != sizeof(*before)) return XUI_ERROR_INVALID_ARGUMENT;
    if (before->iDocumentId != c->identity || before->iRevision != c->before_revision) return XUI_DOC_ERROR_STALE;
    if (!doc_position_valid(c->before, before)) return XUI_ERROR_INVALID_ARGUMENT;
    input = *before; before = &input; /* The caller may map a position in place. */
    *after = *before; *mapping = XUI_DOC_MAP_EXACT;
    if (c->domain == XUI_DOC_SOURCE && before->iKind == XUI_DOC_POSITION_TEXT &&
        doc_semantic_subtree_equal(c->before, before->iNodeId, c->after, before->iNodeId)) {
        after->iRevision = c->after_revision; return XUI_OK;
    }
    if (c->before->profile == XUI_DOCUMENT_MARKDOWN && c->after->profile == XUI_DOCUMENT_MARKDOWN &&
        (c->domain == XUI_DOC_SOURCE || before->iKind == XUI_DOC_POSITION_SOURCE)) {
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
            result = xuiDocumentSourceToPositionEx(&next, source, before->iAffinity, after, mapping);
            if (result == XUI_OK) {
                after->iAffinity = before->iAffinity;
                if (deleted) *mapping = XUI_DOC_MAP_DELETED;
                else if (previous_mapping != XUI_DOC_MAP_EXACT) {
                    if (*mapping == XUI_DOC_MAP_EXACT) *mapping = previous_mapping;
                    else if (*mapping == XUI_DOC_MAP_APPROXIMATE || previous_mapping == XUI_DOC_MAP_APPROXIMATE) *mapping = XUI_DOC_MAP_APPROXIMATE;
                    else if (previous_mapping == XUI_DOC_MAP_COLLAPSED) *mapping = XUI_DOC_MAP_COLLAPSED;
                }
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
        } else if (op->iKind == XUI_DOC_OP_SPLIT &&
            after->iKind == (uint32_t)(op->iParentId ? XUI_DOC_POSITION_GAP : XUI_DOC_POSITION_TEXT)) {
            if (c->undo && after->iNodeId == op->iOtherNodeId) { after->iNodeId = op->iNodeId; after->iOffset += op->iOffset; }
            else if (!c->undo && after->iNodeId == op->iNodeId &&
                (after->iOffset > op->iOffset || (after->iOffset == op->iOffset && after->iAffinity == XUI_DOC_AFTER))) {
                after->iNodeId = op->iOtherNodeId; after->iOffset -= op->iOffset;
            }
        } else if (op->iKind == XUI_DOC_OP_MERGE && after->iKind == XUI_DOC_POSITION_GAP) {
            if (!c->undo && after->iNodeId == op->iOtherNodeId) { after->iNodeId = op->iNodeId; after->iOffset += op->iOffset; }
            else if (c->undo && after->iNodeId == op->iNodeId &&
                (after->iOffset > op->iOffset || (after->iOffset == op->iOffset && after->iAffinity == XUI_DOC_AFTER))) {
                after->iNodeId = op->iOtherNodeId; after->iOffset -= op->iOffset;
            } else if (!c->undo && after->iNodeId == op->iNodeId) {
                after->iOffset = doc_map_offset(after->iOffset, after->iAffinity, op->iOffset, 0, op->iOldLength, &deleted);
            }
        } else if (op->iKind == XUI_DOC_OP_MOVE && !(op->iMappingFlags & XUI_DOC_OP_ANCESTRY_ONLY) && after->iKind == XUI_DOC_POSITION_GAP) {
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
    return doc_position_valid(c->after, after) ? XUI_OK : XUI_DOC_ERROR_SCHEMA;
}

#endif
