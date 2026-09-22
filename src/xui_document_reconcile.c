#include "xui_document_internal.h"

typedef struct doc_candidate {
    doc_node* node;
    uint64_t start, end;
    int used;
} doc_candidate;
typedef struct doc_reconcile {
    doc_state *old, *parsed, *result;
    doc_candidate* candidates;
    uint64_t count;
    const xui_doc_operation_t* edit;
    uint64_t edit_count;
} doc_reconcile;
static int doc_candidate_order(const void* a, const void* b)
{
    const doc_candidate *x = a, *y = b;
    if (x->node->kind != y->node->kind) return x->node->kind < y->node->kind ? -1 : 1;
    if (x->start != y->start) return x->start < y->start ? -1 : 1;
    if (x->end != y->end) return x->end < y->end ? -1 : 1;
    return x->node->id < y->node->id ? -1 : x->node->id != y->node->id;
}
static void doc_candidates_collect(doc_reconcile* c, uint64_t id)
{
    doc_node* n = doc_index_get(c->old->index, id);
    doc_candidate* item = &c->candidates[c->count++];
    uint64_t i;
    item->node = n; item->start = n->source_start; item->end = n->source_end;
    for (i = 0; i < c->edit_count && item->start != DOC_NONE; i++) {
        const xui_doc_operation_t* edit = &c->edit[i];
        uint64_t start = edit->iOffset, end = start + edit->iOldLength, added = edit->iNewLength;
        if (edit->iKind != XUI_DOC_OP_SOURCE) continue;
        if (item->end <= start) { /* Before the patch. */ }
        else if (item->start >= end) {
            item->start = item->start - edit->iOldLength + added;
            item->end = item->end - edit->iOldLength + added;
        } else {
            if (item->start > start) item->start = start;
            item->end = item->end >= end ? item->end - edit->iOldLength + added : start + added;
        }
    }
    for (i = 0; i < doc_seq_size(n->children); i++) doc_candidates_collect(c, doc_seq_get_id(n->children, i));
}
static uint64_t doc_candidate_match(doc_reconcile* c, doc_node* n, uint64_t parent)
{
    uint64_t low = 0, high = c->count, i, best = DOC_NONE, distance = DOC_NONE;
    if (n->id == DOC_ROOT) return DOC_ROOT;
    if (n->source_start == DOC_NONE) return n->id;
    while (low < high) {
        uint64_t mid = low + (high - low) / 2;
        doc_candidate* v = &c->candidates[mid];
        if (v->node->kind < n->kind || (v->node->kind == n->kind && v->start < n->source_start)) low = mid + 1;
        else high = mid;
    }
    for (i = low; i < c->count; i++) {
        doc_candidate* v = &c->candidates[i];
        uint64_t d;
        if (v->node->kind != n->kind || v->start != n->source_start) break;
        if (v->used || v->node->parent != parent) continue;
        d = v->end > n->source_end ? v->end - n->source_end : n->source_end - v->end;
        if (d < distance) { distance = d; best = i; }
    }
    if (best == DOC_NONE) return n->id;
    c->candidates[best].used = 1;
    return c->candidates[best].node->id;
}
static int doc_reconcile_node(doc_reconcile* c, uint64_t fresh_id, uint64_t parent, uint64_t* out)
{
    doc_node *n = doc_index_get(c->parsed->index, fresh_id), *copy;
    doc_allocator* a = c->result->allocator;
    uint64_t i;
    int result = XUI_OK;
    copy = doc_node_clone(a, n);
    if (!copy) return XUI_ERROR_OUT_OF_MEMORY;
    copy->id = doc_candidate_match(c, n, parent); copy->parent = parent;
    doc_seq_release(copy->children); copy->children = NULL;
    for (i = 0; i < doc_seq_size(n->children); i++) {
        uint64_t child;
        doc_sequence *item, *next = NULL;
        result = doc_reconcile_node(c, doc_seq_get_id(n->children, i), copy->id, &child);
        if (result != XUI_OK) break;
        item = doc_seq_id(a, child);
        if (!item) { result = XUI_ERROR_OUT_OF_MEMORY; break; }
        result = doc_seq_replace(a, copy->children, i, i, item, &next);
        doc_seq_release(item);
        if (result != XUI_OK) break;
        doc_seq_release(copy->children); copy->children = next;
    }
    if (result == XUI_OK) result = doc_state_set(c->result, copy);
    *out = copy->id; doc_node_release(copy); return result;
}
int doc_markdown_reconcile(xui_document_transaction t, doc_state** parsed)
{
    doc_reconcile c = {0};
    uint64_t root;
    int result;
    if (t->draft->node_count == 1) return XUI_OK;
    c.old = t->draft; c.parsed = *parsed;
    c.edit = t->count > t->parse_op_start ? t->ops + t->parse_op_start : NULL;
    c.edit_count = t->count - t->parse_op_start;
    if (c.old->node_count > SIZE_MAX / sizeof(*c.candidates)) return XUI_DOC_ERROR_LIMIT;
    c.candidates = doc_alloc(c.old->allocator, (size_t)c.old->node_count * sizeof(*c.candidates));
    if (!c.candidates) return XUI_ERROR_OUT_OF_MEMORY;
    doc_candidates_collect(&c, DOC_ROOT);
    qsort(c.candidates, (size_t)c.count, sizeof(*c.candidates), doc_candidate_order);
    c.result = doc_state_new(c.old->allocator, XUI_DOCUMENT_MARKDOWN);
    if (!c.result) { doc_free(c.candidates); return XUI_ERROR_OUT_OF_MEMORY; }
    c.result->source = c.parsed->source; doc_seq_retain(c.result->source);
    c.result->dialect = c.parsed->dialect;
    result = doc_reconcile_node(&c, DOC_ROOT, 0, &root);
    doc_free(c.candidates);
    if (result == XUI_OK) { doc_state_release(*parsed); *parsed = c.result; }
    else doc_state_release(c.result);
    return result;
}
