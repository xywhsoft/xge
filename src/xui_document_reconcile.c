#include "../xui_config.h"
#if XUI_ENABLE_DOCUMENT
#include "xui_document_internal.h"

typedef struct doc_candidate {
    doc_node* node;
    uint64_t start, end;
    int used;
} doc_candidate;
typedef struct doc_reconcile {
    xui_document document;
    doc_state *old, *parsed, *result;
    doc_candidate* candidates;
    uint64_t count;
    const xui_doc_operation_t* edit;
    uint64_t edit_count;
    const atomic_int* cancellation;
    uint64_t block_ordinal;
    int64_t block_shift_base;
    int local;
} doc_reconcile;
static int doc_reconcile_preserve(doc_reconcile*, uint64_t, uint64_t);
static int doc_candidate_order(const void* a, const void* b)
{
    const doc_candidate *x = a, *y = b;
    if (x->node->kind != y->node->kind) return x->node->kind < y->node->kind ? -1 : 1;
    if (x->start != y->start) return x->start < y->start ? -1 : 1;
    if (x->end != y->end) return x->end < y->end ? -1 : 1;
    return x->node->id < y->node->id ? -1 : x->node->id != y->node->id;
}
static int doc_candidates_collect(doc_reconcile* c, uint64_t id)
{
    doc_node* n = doc_index_get(c->old->index, id);
    doc_candidate* item = &c->candidates[c->count++];
    doc_node_source_range range;
    uint64_t i;
    int result = c->cancellation ? atomic_load(c->cancellation) : XUI_OK;
    if (result != XUI_OK) return result;
    item->node = n;
    doc_node_source_range_get(c->old, n, &range);
    item->start = range.syntax_start != DOC_NONE ? range.syntax_start : range.source_start;
    item->end = range.syntax_end != DOC_NONE ? range.syntax_end : range.source_end;
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
    for (i = 0; i < doc_seq_size(n->children); i++) {
        result = doc_candidates_collect(c, doc_seq_get_id(n->children, i));
        if (result != XUI_OK) return result;
    }
    return XUI_OK;
}
static uint64_t doc_candidate_match(doc_reconcile* c, doc_node* n, uint64_t parent)
{
    uint64_t low = 0, high = c->count, i, best = DOC_NONE, distance = DOC_NONE;
    doc_node_source_range range;
    uint64_t start, end;
    doc_node_source_range_get(c->parsed, n, &range);
    start = range.syntax_start != DOC_NONE ? range.syntax_start : range.source_start;
    end = range.syntax_end != DOC_NONE ? range.syntax_end : range.source_end;
    if (n->id == DOC_ROOT) return DOC_ROOT;
    if (start == DOC_NONE) return n->id;
    while (low < high) {
        uint64_t mid = low + (high - low) / 2;
        doc_candidate* v = &c->candidates[mid];
        if (v->node->kind < n->kind || (v->node->kind == n->kind && v->start < start)) low = mid + 1;
        else high = mid;
    }
    for (i = low; i < c->count; i++) {
        doc_candidate* v = &c->candidates[i];
        uint64_t d;
        if (v->node->kind != n->kind || v->start != start) break;
        if (v->used || v->node->parent != parent) continue;
        d = v->end > end ? v->end - end : end - v->end;
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
    uint64_t i, id = doc_candidate_match(c, n, parent);
    int result = XUI_OK;
    if (c->cancellation && (result = atomic_load(c->cancellation)) != 0) return result;
    if (id != fresh_id || id == DOC_ROOT) {
        if (doc_semantic_subtree_equal(c->old, id, c->parsed, fresh_id)) {
            *out = id; return doc_reconcile_preserve(c, id, fresh_id);
        }
    }
    copy = doc_node_clone(a, n);
    if (!copy) return XUI_ERROR_OUT_OF_MEMORY;
    copy->id = id; copy->parent = parent;
    /* Source edits already establish this logical node's identity. Preserve
     * matching semantic text bytes too, including disjoint unchanged runs. */
    {
        doc_node* old = doc_index_get(c->old->index, id);
        if (old && old->kind == copy->kind && (old->text || copy->text)) {
            doc_sequence* shared = NULL;
            result = doc_seq_reuse_bytes(a, old->text, copy->text, c->cancellation, &shared);
            if (result != XUI_OK) { doc_node_release(copy); return result; }
            doc_seq_release(copy->text); copy->text = shared;
        }
    }
    if (c->local) { copy->block_ordinal = c->block_ordinal; copy->block_shift_base = c->block_shift_base; }
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
    c.document = t->document; c.old = t->draft; c.parsed = *parsed;
    c.cancellation = t->cancellation;
    c.edit = t->count > t->parse_op_start ? t->ops + t->parse_op_start : NULL;
    c.edit_count = t->count - t->parse_op_start;
    if (c.old->node_count > SIZE_MAX / sizeof(*c.candidates)) return XUI_DOC_ERROR_LIMIT;
    c.candidates = doc_alloc(c.old->allocator, (size_t)c.old->node_count * sizeof(*c.candidates));
    if (!c.candidates) return XUI_ERROR_OUT_OF_MEMORY;
    result = doc_candidates_collect(&c, DOC_ROOT);
    if (result != XUI_OK) { doc_free(c.candidates); return result; }
    qsort(c.candidates, (size_t)c.count, sizeof(*c.candidates), doc_candidate_order);
    c.result = doc_state_new(c.old->allocator, XUI_DOCUMENT_MARKDOWN);
    if (!c.result) { doc_free(c.candidates); return XUI_ERROR_OUT_OF_MEMORY; }
    c.result->source = c.parsed->source; doc_seq_retain(c.result->source);
    c.result->source_open_brackets = c.parsed->source_open_brackets;
    c.result->source_storage_bytes = c.parsed->source_storage_bytes;
    c.result->references = c.parsed->references; doc_seq_retain(c.result->references);
    c.result->reference_values = c.parsed->reference_values; doc_seq_retain(c.result->reference_values);
    c.result->inline_syntax = c.parsed->inline_syntax; doc_seq_retain(c.result->inline_syntax);
    c.result->reference_candidates = c.parsed->reference_candidates; doc_seq_retain(c.result->reference_candidates);
    c.result->dialect = c.parsed->dialect;
    c.result->markdown_footnotes = c.parsed->markdown_footnotes;
    result = doc_reconcile_node(&c, DOC_ROOT, 0, &root);
    if (result == XUI_OK) c.result->source_blocks_indexed =
        doc_markdown_source_blocks_ordered(c.result, doc_seq_size(c.result->source));
    doc_free(c.candidates);
    if (result == XUI_OK) { doc_state_release(*parsed); *parsed = c.result; }
    else doc_state_release(c.result);
    return result;
}

static uint64_t doc_reconcile_count(doc_state* s, uint64_t id)
{
    doc_node* n = doc_index_get(s->index, id); uint64_t count = 1, i;
    for (i = 0; i < doc_seq_size(n->children); i++) count += doc_reconcile_count(s, doc_seq_get_id(n->children, i));
    return count;
}
/* Reuse the full parser's identity and logical-text-run rules, but only for
 * the replaced block. The result already retains every unaffected subtree. */
int doc_markdown_reconcile_block(xui_document_transaction t, doc_state* parsed, uint64_t old_id,
    uint64_t fresh_id, doc_state* result_state, uint64_t ordinal, int64_t shift_base, uint64_t* out)
{
    doc_reconcile c = {0}; uint64_t count = doc_reconcile_count(t->draft, old_id); int result;
    c.document = t->document; c.old = t->draft; c.parsed = parsed; c.result = result_state;
    c.local = 1; c.block_ordinal = ordinal; c.block_shift_base = shift_base;
    c.cancellation = t->cancellation; c.edit = t->ops + t->parse_op_start; c.edit_count = t->count - t->parse_op_start;
    if (count > SIZE_MAX / sizeof(*c.candidates)) return XUI_DOC_ERROR_LIMIT;
    c.candidates = doc_alloc(c.old->allocator, (size_t)count * sizeof(*c.candidates));
    if (!c.candidates) return XUI_ERROR_OUT_OF_MEMORY;
    result = doc_candidates_collect(&c, old_id);
    if (result == XUI_OK) {
        qsort(c.candidates, (size_t)c.count, sizeof(*c.candidates), doc_candidate_order);
        result = doc_reconcile_node(&c, fresh_id, DOC_ROOT, out);
    }
    doc_free(c.candidates); return result;
}

/* Semantic commands already know which nodes survived. Keep that tree's IDs
 * and text boundaries; the parser supplies verified source metadata only.
 * In particular, equal siblings and relocated subtrees must never be matched
 * by their old source offsets. Parser text callbacks may split entities and
 * escapes differently, so align their bytes rather than their child ordinals. */
typedef struct doc_semantic_projection {
    doc_state *desired, *parsed;
    struct xui_doc_transaction_t normalized;
    uint64_t block_ordinal;
    int64_t block_shift_base;
    int local;
    int reuse_shared;
    int retain_root_shifts;
} doc_semantic_projection;

static int doc_projection_empty(doc_state* s, doc_node* n)
{
    return n && ((n->kind == XUI_DOC_TEXT && !doc_seq_size(n->text)) || doc_semantic_empty_paragraph(s, n));
}

static int doc_projection_drop(doc_semantic_projection* c, doc_node* n)
{
    doc_state* state = c->normalized.draft;
    doc_node *parent, *copy; doc_sequence* children = NULL; uint64_t index; int result;
    if (doc_index_get(state->index, n->id)) return doc_txn_delete(&c->normalized, n->id);
    /* Source reconciliation builds a fresh index. The parent's child list is
     * already copied, but this omitted empty node has not been inserted. */
    parent = doc_index_get(state->index, n->parent); index = doc_child_index(parent, n->id);
    copy = doc_node_clone(state->allocator, parent);
    if (!copy) return XUI_ERROR_OUT_OF_MEMORY;
    result = doc_seq_replace(state->allocator, copy->children, index, index + 1, NULL, &children);
    if (result == XUI_OK) {
        doc_seq_release(copy->children); copy->children = children; result = doc_state_set(state, copy);
    }
    doc_node_release(copy); return result;
}

/* A shared subtree already contains the parser's metadata when the normalized
 * root also retains the parser's source shifts. Otherwise only unshifted
 * subtrees can be reused. Verify every indexed descendant:
 * persistent indexes can replace a child without replacing its parent node.
 * Empty nodes still need the normal projection's canonicalization. */
static int doc_projection_reuse(doc_semantic_projection* c, doc_node* a, doc_node* b,
    int* reusable)
{
    uint64_t i;
    int result;
    *reusable = 0;
    if (c->normalized.cancellation &&
        (result = atomic_load(c->normalized.cancellation)) != XUI_OK) return result;
    if (!a || !b || a != b || doc_projection_empty(c->desired, a) ||
        (!c->retain_root_shifts && (a->block_shift_base ||
            doc_node_block_shift(c->desired, a) ||
            doc_node_block_shift(c->parsed, b)))) return XUI_OK;
    for (i = 0; i < doc_seq_size(a->children); i++) {
        doc_node* left = doc_index_get(c->desired->index, doc_seq_get_id(a->children, i));
        doc_node* right = doc_index_get(c->parsed->index, doc_seq_get_id(b->children, i));
        int child_reusable;
        result = doc_projection_reuse(c, left, right, &child_reusable);
        if (result != XUI_OK || !child_reusable) return result;
    }
    *reusable = 1;
    return XUI_OK;
}

/* The parser's persistent root sequence can be transplanted only when it
 * names exactly the desired root blocks. Semantic equivalence alone allows
 * different IDs and ignorable empty paragraphs, neither of which is safe. */
static int doc_projection_same_root_ids(doc_semantic_projection* c, int* same)
{
    doc_node* a = doc_index_get(c->desired->index, DOC_ROOT);
    doc_node* b = doc_index_get(c->parsed->index, DOC_ROOT);
    uint64_t i, count = doc_seq_size(a->children);
    *same = 0;
    if (count != doc_seq_size(b->children)) return XUI_OK;
    for (i = 0; i < count; i++) {
        uint64_t id = doc_seq_get_id(a->children, i);
        doc_node* left = doc_index_get(c->desired->index, id);
        doc_node* right = doc_index_get(c->parsed->index, id);
        int result;
        if (c->normalized.cancellation &&
            (result = atomic_load(c->normalized.cancellation)) != XUI_OK) return result;
        if (id != doc_seq_get_id(b->children, i) || !left || !right ||
            doc_projection_empty(c->desired, left) ||
            doc_projection_empty(c->parsed, right)) return XUI_OK;
    }
    *same = 1;
    return XUI_OK;
}

static int doc_projection_node(doc_semantic_projection* c, uint64_t wanted, uint64_t parsed)
{
    doc_node *a = doc_index_get(c->desired->index, wanted), *b = doc_index_get(c->parsed->index, parsed), *copy;
    uint64_t ai, bi = 0, bo = 0, ac = doc_seq_size(a->children), bc = doc_seq_size(b->children);
    int result;
    if (c->normalized.cancellation && (result = atomic_load(c->normalized.cancellation)) != 0) return result;
    if (c->reuse_shared && a->id != DOC_ROOT) {
        int reusable;
        result = doc_projection_reuse(c, a, b, &reusable);
        if (result != XUI_OK) return result;
        if (reusable) return XUI_OK;
    }
    copy = doc_node_clone(c->desired->allocator, a);
    if (!copy) return XUI_ERROR_OUT_OF_MEMORY;
    copy->source_start = b->source_start; copy->source_end = b->source_end; copy->source_exact = b->source_exact;
    copy->syntax_start = b->syntax_start; copy->syntax_end = b->syntax_end;
    copy->marker_kind = b->marker_kind;
    copy->marker_primary_start = b->marker_primary_start; copy->marker_primary_end = b->marker_primary_end;
    copy->marker_secondary_start = b->marker_secondary_start; copy->marker_secondary_end = b->marker_secondary_end;
    copy->marker_tail_end = b->marker_tail_end;
    doc_blob_release(copy->quote_prefixes); copy->quote_prefixes = b->quote_prefixes;
    doc_blob_retain(copy->quote_prefixes);
    doc_blob_release(copy->syntax_aux); copy->syntax_aux = b->syntax_aux;
    doc_blob_retain(copy->syntax_aux);
    doc_blob_release(copy->list_indents); copy->list_indents = b->list_indents;
    doc_blob_retain(copy->list_indents);
    copy->block_ordinal = c->local ? c->block_ordinal : b->block_ordinal;
    /* When root IDs agree, retain the parser's lazy source shifts and raw
     * block bases. The fallback clears the desired root's shifts, so store
     * each parsed node's effective shift in its base instead. */
    copy->block_shift_base = c->local ? c->block_shift_base :
        c->retain_root_shifts ? b->block_shift_base : -doc_node_block_shift(c->parsed, b);
    copy->provenance_base = b->provenance_base; copy->provenance_current = b->provenance_current;
    doc_seq_release(copy->provenance); copy->provenance = b->provenance; doc_seq_retain(copy->provenance);
    if (a->id == DOC_ROOT && c->retain_root_shifts) {
        doc_seq_release(copy->children); copy->children = b->children; doc_seq_retain(copy->children);
        c->normalized.draft->block_shifts = c->parsed->block_shifts;
    } else if (a->id == DOC_ROOT && c->desired->block_shifts) {
        doc_sequence* clean = NULL;
        result = doc_seq_clear_source_shifts(c->desired->allocator, copy->children, &clean);
        if (result != XUI_OK) { doc_node_release(copy); return result; }
        doc_seq_release(copy->children); copy->children = clean;
        c->normalized.draft->block_shifts = 0;
    }
    result = doc_state_set(c->normalized.draft, copy); doc_node_release(copy);
    if (result != XUI_OK) return result;
    for (ai = 0; ai < ac; ai++) {
        doc_node *left = doc_index_get(c->desired->index, doc_seq_get_id(a->children, ai)), *right;
        uint64_t bytes = doc_seq_size(left->text), used = 0;
        right = bi < bc ? doc_index_get(c->parsed->index, doc_seq_get_id(b->children, bi)) : NULL;
        if (doc_projection_empty(c->desired, left)) {
            if (right && left->kind == XUI_DOC_PARAGRAPH && doc_semantic_empty_paragraph(c->parsed, right)) {
                result = doc_projection_node(c, left->id, right->id); bi++;
            } else result = doc_projection_drop(c, left);
            if (result != XUI_OK) return result;
            continue;
        }
        while (doc_projection_empty(c->parsed, right)) {
            bi++; right = bi < bc ? doc_index_get(c->parsed->index, doc_seq_get_id(b->children, bi)) : NULL;
        }
        if (!right) return XUI_DOC_ERROR_UNREPRESENTABLE;
        if (left->kind != XUI_DOC_TEXT) {
            if (bo || left->kind != right->kind) return XUI_DOC_ERROR_UNREPRESENTABLE;
            result = doc_projection_node(c, left->id, right->id); bi++;
            if (result != XUI_OK) return result;
            continue;
        }
        copy = doc_node_clone(c->desired->allocator, left);
        if (!copy) return XUI_ERROR_OUT_OF_MEMORY;
        copy->source_start = copy->source_end = copy->syntax_start = copy->syntax_end = DOC_NONE;
        copy->block_ordinal = c->local ? c->block_ordinal : b->block_ordinal;
        /* Slices below use absolute coordinates. Cancel the retained root
         * shift for this newly projected text node, or use zero after the
         * fallback has cleared the root shifts. */
        copy->block_shift_base = c->local ? c->block_shift_base :
            c->retain_root_shifts && copy->block_ordinal != DOC_NONE ?
                doc_seq_source_shift(doc_index_get(c->parsed->index, DOC_ROOT)->children,
                    copy->block_ordinal) : 0;
        doc_seq_release(copy->provenance); copy->provenance = NULL;
        copy->provenance_base = DOC_NONE; copy->provenance_current = 0;
        copy->source_exact = 1;
        while (used < bytes) {
            uint64_t available, take, start, end;
            int exact;
            right = bi < bc ? doc_index_get(c->parsed->index, doc_seq_get_id(b->children, bi)) : NULL;
            if (!right || right->kind != XUI_DOC_TEXT || bo >= doc_seq_size(right->text)) {
                result = XUI_DOC_ERROR_UNREPRESENTABLE; break;
            }
            available = doc_seq_size(right->text) - bo; take = bytes - used < available ? bytes - used : available;
            result = doc_source_slice(c->parsed, right, bo, bo + take, used, &copy->provenance);
            if (result != XUI_OK) break;
            {
                doc_node_source_range range;
                doc_node_source_range_get(c->parsed, right, &range);
                start = range.source_start; end = range.source_end;
            }
            exact = right->source_exact && start != DOC_NONE && end - start == doc_seq_size(right->text);
            if (exact) { start += bo; end = start + take; }
            else if (bo || take != doc_seq_size(right->text)) start = end = DOC_NONE;
            if (!used) copy->source_start = start;
            else if (copy->source_end != start) copy->source_exact = 0;
            if (start == DOC_NONE || end == DOC_NONE || copy->source_start == DOC_NONE) {
                copy->source_start = DOC_NONE; copy->source_exact = 0;
            }
            copy->source_end = end;
            if (!exact) copy->source_exact = 0;
            used += take; bo += take;
            if (bo == doc_seq_size(right->text)) { bi++; bo = 0; }
        }
        if (copy->source_start == DOC_NONE) copy->source_end = DOC_NONE;
        if (result == XUI_OK) result = doc_state_set(c->normalized.draft, copy);
        doc_node_release(copy);
        if (result != XUI_OK) return result;
    }
    return XUI_OK;
}

static int doc_reconcile_preserve(doc_reconcile* source, uint64_t wanted, uint64_t parsed)
{
    doc_semantic_projection c = {0};
    c.desired = source->old; c.parsed = source->parsed;
    c.local = source->local; c.block_ordinal = source->block_ordinal;
    c.block_shift_base = source->block_shift_base;
    c.normalized.draft = source->result; c.normalized.parsing = DOC_BUILD_PARSE;
    c.normalized.document = source->document;
    c.normalized.cancellation = source->cancellation;
    c.normalized.domain = XUI_DOC_SEMANTIC;
    return doc_projection_node(&c, wanted, parsed);
}

int doc_markdown_accept(xui_document_transaction t, xui_document_transaction desired)
{
    doc_semantic_projection c = {0};
    uint64_t i;
    int result;
    if (!doc_semantic_equal(desired->draft, t->draft)) return doc_txn_fail(t, XUI_DOC_ERROR_UNREPRESENTABLE);
    c.desired = desired->draft; c.parsed = t->draft;
    c.reuse_shared = 1;
    c.normalized.cancellation = t->cancellation;
    result = doc_projection_same_root_ids(&c, &c.retain_root_shifts);
    if (result != XUI_OK) return doc_txn_fail(t, result);
    c.normalized.document = t->document; c.normalized.domain = XUI_DOC_SEMANTIC; c.normalized.parsing = DOC_BUILD_SEMANTIC;
    c.normalized.draft = doc_state_clone(c.desired);
    if (!c.normalized.draft) return doc_txn_fail(t, XUI_ERROR_OUT_OF_MEMORY);
    doc_seq_release(c.normalized.draft->source); c.normalized.draft->source = c.parsed->source; doc_seq_retain(c.parsed->source);
    c.normalized.draft->source_open_brackets = c.parsed->source_open_brackets;
    c.normalized.draft->source_storage_bytes = c.parsed->source_storage_bytes;
    doc_seq_release(c.normalized.draft->references); c.normalized.draft->references = c.parsed->references; doc_seq_retain(c.parsed->references);
    doc_seq_release(c.normalized.draft->reference_values); c.normalized.draft->reference_values = c.parsed->reference_values; doc_seq_retain(c.parsed->reference_values);
    doc_seq_release(c.normalized.draft->inline_syntax); c.normalized.draft->inline_syntax = c.parsed->inline_syntax; doc_seq_retain(c.parsed->inline_syntax);
    doc_seq_release(c.normalized.draft->reference_candidates); c.normalized.draft->reference_candidates = c.parsed->reference_candidates;
    doc_seq_retain(c.parsed->reference_candidates);
    c.normalized.draft->markdown_footnotes = c.parsed->markdown_footnotes;
    result = doc_projection_node(&c, DOC_ROOT, DOC_ROOT);
    if (result == XUI_OK) result = doc_schema_validate(c.normalized.draft);
    if (result == XUI_OK) c.normalized.draft->source_blocks_indexed =
        doc_markdown_source_blocks_ordered(c.normalized.draft, doc_seq_size(c.normalized.draft->source));
    for (i = 0; i < desired->count && result == XUI_OK; i++) result = doc_txn_op(t, &desired->ops[i]);
    for (i = 0; i < c.normalized.count && result == XUI_OK; i++) result = doc_txn_op(t, &c.normalized.ops[i]);
    if (result == XUI_OK) {
        doc_state_release(t->draft); t->draft = c.normalized.draft; c.normalized.draft = NULL;
        t->parse_op_start = t->count;
    }
    doc_state_release(c.normalized.draft); doc_free(c.normalized.ops);
    return result == XUI_OK ? result : doc_txn_fail(t, result);
}

#endif
