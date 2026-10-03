#include "../xui_config.h"
#if XUI_ENABLE_DOCUMENT
#include "xui_document_internal.h"
#include <assert.h>

/* Reachability is separate from lifetime refcounts. A shared subtree is
 * charged once per owner class, and changing a root visits only paths whose
 * class count crosses zero. Metadata lives outside immutable object payloads. */
static void doc_memory_lock(doc_allocator* a)
{
    while (atomic_exchange_explicit(&a->memory_lock, 1, memory_order_acquire)) { }
}
static void doc_memory_unlock(doc_allocator* a) { atomic_store_explicit(&a->memory_lock, 0, memory_order_release); }
/* Detached prepare builders retain their roots but cannot read mutable owner
 * counters without this lock. A changed live root invalidates their plan at
 * publication; snapshot diagnostics only touch the third owner class. */
void doc_memory_owner_counts(const doc_allocation* h, uint64_t* current, uint64_t* history)
{
    doc_allocator* a = h->value.allocator;
    doc_memory_lock(a); *current = h->value.owners[0]; *history = h->value.owners[1]; doc_memory_unlock(a);
}
uint64_t doc_memory_storage_history_bytes(doc_allocator* a)
{
    uint64_t bytes;
    doc_memory_lock(a); bytes = a->memory_buckets[2] + a->memory_buckets[6]; doc_memory_unlock(a);
    return bytes;
}
static doc_allocation* doc_memory_header(const void* p) { return (doc_allocation*)p - 1; }
static uint64_t doc_memory_bytes(const void* p) { return p ? doc_memory_header(p)->value.bytes : 0; }
void doc_memory_tag(void* p, unsigned kind) { doc_memory_header(p)->value.kind = kind; }
static unsigned doc_memory_mask(const doc_allocation* h)
{
    return (unsigned)!!h->value.owners[0] | ((unsigned)!!h->value.owners[1] << 1) | ((unsigned)!!h->value.owners[2] << 2);
}
static void doc_memory_visit(void* pointer, unsigned owner, int add)
{
    doc_allocation* h; doc_allocator* a; unsigned before, after, i;
    if (!pointer) return;
    h = doc_memory_header(pointer); a = h->value.allocator;
    assert(h->value.kind >= DOC_MEMORY_BLOB && h->value.kind <= DOC_MEMORY_ATTRIBUTE);
    before = doc_memory_mask(h);
    if (add) { assert(h->value.owners[owner] != UINT64_MAX); if (h->value.owners[owner]++) return; }
    else { assert(h->value.owners[owner]); if (--h->value.owners[owner]) return; }
    after = doc_memory_mask(h);
    if (before) { assert(a->memory_buckets[before] >= h->value.bytes); a->memory_buckets[before] -= h->value.bytes; }
    if (after) a->memory_buckets[after] += h->value.bytes;
    switch (h->value.kind) {
    case DOC_MEMORY_SEQUENCE: {
        doc_sequence* p = pointer;
        doc_memory_visit(p->left, owner, add); doc_memory_visit(p->right, owner, add); doc_memory_visit(p->blob, owner, add);
        break;
    }
    case DOC_MEMORY_NODE: {
        doc_node* p = pointer;
        doc_memory_visit(doc_attribute_owner(p->attrs), owner, add);
        doc_memory_visit(p->text, owner, add); doc_memory_visit(p->children, owner, add); doc_memory_visit(p->provenance, owner, add);
        doc_memory_visit(p->resource, owner, add); doc_memory_visit(p->info, owner, add); doc_memory_visit(p->title, owner, add);
        doc_memory_visit(p->link_target, owner, add); doc_memory_visit(p->link_title, owner, add);
        doc_memory_visit(p->column_widths, owner, add);
        doc_memory_visit(p->extension_payload, owner, add);
        doc_memory_visit(p->quote_prefixes, owner, add);
        doc_memory_visit(p->syntax_aux, owner, add);
        doc_memory_visit(p->list_indents, owner, add);
        break;
    }
    case DOC_MEMORY_INDEX: {
        doc_index* p = pointer;
        for (i = 0; i < DOC_INDEX_SIZE; i++) doc_memory_visit(p->slots[i], owner, add);
        break;
    }
    case DOC_MEMORY_STATE: {
        doc_state* p = pointer;
        doc_memory_visit(p->index, owner, add); doc_memory_visit(p->source, owner, add);
        doc_memory_visit(p->references, owner, add); doc_memory_visit(p->reference_values, owner, add); doc_memory_visit(p->inline_syntax, owner, add);
        doc_memory_visit(p->reference_candidates, owner, add);
        break;
    }
    default: break; /* Blob and interned attributes have no owned edges. */
    }
}
void doc_memory_current(doc_state* before, doc_state* after)
{
    doc_allocator* a = before ? before->allocator : after ? after->allocator : NULL;
    if (!a || before == after) return;
    assert(!before || !after || before->allocator == after->allocator);
    doc_memory_lock(a); doc_memory_visit(after, 0, 1); doc_memory_visit(before, 0, 0); doc_memory_unlock(a);
}
void doc_memory_history(doc_history* h, int add)
{
    doc_allocator* a = h->before->allocator; uint64_t bytes = doc_memory_bytes(h) + doc_memory_bytes(h->ops);
    assert(!!h->accounted != !!add);
    doc_memory_lock(a);
    doc_memory_visit(h->before, 1, add); doc_memory_visit(h->after, 1, add);
    if (add) a->history_record_bytes += bytes;
    else { assert(a->history_record_bytes >= bytes); a->history_record_bytes -= bytes; }
    h->accounted = add; doc_memory_unlock(a);
}
/* Phantom history owner on the current root. It does not own a lifetime ref or
 * count toward history bytes while the same objects are current. */
void doc_memory_standby(doc_state* state, int add)
{
    doc_allocator* a = state->allocator;
    doc_memory_lock(a); doc_memory_visit(state, 1, add); doc_memory_unlock(a);
}
void doc_memory_apply_prepared(doc_allocator* a, const doc_memory_delta* changes, uint64_t count,
    const uint64_t* added, const uint64_t* removed, uint64_t record_bytes)
{
    uint64_t i;
    doc_memory_lock(a);
    /* The plan captured the exact live roots and history configuration. It
     * owns every referenced allocation. No tree walk, allocation or callback
     * occurs here. Snapshot diagnostics use this same lock and leave no marks. */
    for (i = 0; i < count; i++) {
        doc_allocation* h = changes[i].allocation;
        assert(!h->value.owners[2]);
        h->value.owners[0] = changes[i].current; h->value.owners[1] = changes[i].history;
    }
    for (i = 1; i < 4; i++) {
        assert(a->memory_buckets[i] >= removed[i]);
        a->memory_buckets[i] = a->memory_buckets[i] - removed[i] + added[i];
    }
    a->history_record_bytes = record_bytes;
    atomic_fetch_add(&a->prepared_publishes, 1); atomic_fetch_add(&a->prepared_storage_updates, count);
    doc_memory_unlock(a);
}
static uint64_t doc_memory_current_bytes(doc_allocator* a)
{
    return a->memory_buckets[1] + a->memory_buckets[3] + a->memory_buckets[5] + a->memory_buckets[7];
}
static uint64_t doc_memory_extra_history(doc_allocator* a)
{
    return a->memory_buckets[2] + a->memory_buckets[6] + a->history_record_bytes;
}
uint64_t doc_memory_history_bytes(doc_allocator* a)
{
    uint64_t bytes; doc_memory_lock(a); bytes = doc_memory_extra_history(a); doc_memory_unlock(a); return bytes;
}
void doc_memory_stats(doc_allocator* a, xui_doc_stats_t* out)
{
    doc_memory_lock(a); out->iCurrentBytes = doc_memory_current_bytes(a);
    out->iHistoryBytes = doc_memory_extra_history(a); out->iSnapshotCount = a->snapshot_count; doc_memory_unlock(a);
}
int doc_snapshot_create(doc_state* state, uint64_t identity, uint64_t revision, xui_document_snapshot* out)
{
    doc_allocator* a = state->allocator; xui_document_snapshot s = doc_alloc(a, sizeof(*s));
    *out = NULL;
    if (!s) return XUI_ERROR_OUT_OF_MEMORY;
    atomic_init(&s->refs, 1); s->state = state; doc_state_retain(state); s->identity = identity; s->revision = revision;
    doc_memory_lock(a); s->memory_next = a->snapshots;
    if (a->snapshots) a->snapshots->memory_previous = s;
    a->snapshots = s; a->snapshot_count++; a->snapshot_handle_bytes += doc_memory_bytes(s); doc_memory_unlock(a);
    *out = s; return XUI_OK;
}
void doc_snapshot_unregister(xui_document_snapshot s)
{
    doc_allocator* a = s->state->allocator;
    doc_memory_lock(a);
    if (s->memory_previous) s->memory_previous->memory_next = s->memory_next;
    else { assert(a->snapshots == s); a->snapshots = s->memory_next; }
    if (s->memory_next) s->memory_next->memory_previous = s->memory_previous;
    assert(a->snapshot_count && a->snapshot_handle_bytes >= doc_memory_bytes(s));
    a->snapshot_count--; a->snapshot_handle_bytes -= doc_memory_bytes(s); doc_memory_unlock(a);
}
static int doc_memory_read(doc_allocator* a, xui_doc_memory_stats_t* out)
{
    xui_document_snapshot s; uint64_t accounted = 0; unsigned i;
    if (!a || !out || out->iSize != sizeof(*out)) return XUI_ERROR_INVALID_ARGUMENT;
    memset(out, 0, sizeof(*out)); out->iSize = sizeof(*out);
    doc_memory_lock(a);
    /* Snapshots are registered in O(1). Only this explicit diagnostic walks
     * their union. Accounting adds no full scan to acquire/release; the final
     * lifetime release may still reclaim unshared persistent storage. */
    for (s = a->snapshots; s; s = s->memory_next) doc_memory_visit(s->state, 2, 1);
    out->iLiveBytes = atomic_load(&a->live); out->iCurrentBytes = doc_memory_current_bytes(a);
    out->iHistoryBytes = doc_memory_extra_history(a); out->iSnapshotCount = a->snapshot_count;
    out->iSnapshotHandleBytes = a->snapshot_handle_bytes;
    out->iSnapshotBytes = a->memory_buckets[4] + a->memory_buckets[5] + a->memory_buckets[6] + a->memory_buckets[7] + a->snapshot_handle_bytes;
    out->iSnapshotAdditionalBytes = a->memory_buckets[4] + a->snapshot_handle_bytes;
    for (i = 1; i < 8; i++) accounted += a->memory_buckets[i];
    accounted += a->snapshot_handle_bytes + a->history_record_bytes;
    assert(out->iLiveBytes >= accounted); out->iOtherBytes = out->iLiveBytes - accounted;
    for (s = a->snapshots; s; s = s->memory_next) doc_memory_visit(s->state, 2, 0);
    doc_memory_unlock(a); return XUI_OK;
}
XUI_API int xuiDocumentGetMemoryStats(xui_document d, xui_doc_memory_stats_t* out)
{
    return doc_memory_read(d ? d->allocator : NULL, out);
}
XUI_API int xuiDocumentSnapshotGetMemoryStats(xui_document_snapshot s, xui_doc_memory_stats_t* out)
{
    return doc_memory_read(s ? s->state->allocator : NULL, out);
}

#endif
