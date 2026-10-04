#include "../xui_config.h"
#if XUI_ENABLE_DOCUMENT
#include "xui_document_internal.h"
#include <assert.h>

/* Count incoming edges of each reachable immutable allocation, independently
 * of the live allocator's mutable accounting. Root multiplicities matter;
 * descendants are visited only on a 0<->1 transition, exactly as in memory.c. */
typedef struct doc_charge_entry {
    doc_allocation* allocation;
    uint64_t counts[4]; /* Old current/history, proposed current/history. */
} doc_charge_entry;
typedef struct doc_charge_map {
    doc_allocator* allocator;
    doc_charge_entry* entries;
    uint64_t size, capacity, extra_history, visits;
    const atomic_int* cancellation;
} doc_charge_map;
struct doc_publish_plan {
    doc_allocator* allocator;
    doc_history** history; /* Captured Undo followed by Redo; ignore mutable next. */
    uint64_t undo_count, redo_count, revision, max_bytes, keep_undo;
    unsigned limit;
    int disabled, standby, ready;
    doc_history* next;
    xui_document_change_set change;
    doc_memory_delta* deltas;
    uint64_t count, added[4], removed[4], record_bytes;
};
static uint64_t doc_charge_hash(const void* pointer)
{
    uint64_t x = (uint64_t)(uintptr_t)pointer;
    x ^= x >> 30; x *= UINT64_C(0xbf58476d1ce4e5b9);
    x ^= x >> 27; x *= UINT64_C(0x94d049bb133111eb); return x ^ (x >> 31);
}
static int doc_charge_grow(doc_charge_map* m)
{
    uint64_t i, capacity = m->capacity ? m->capacity * 2 : 128;
    doc_charge_entry* entries;
    if (capacity < m->capacity || capacity > SIZE_MAX / sizeof(*entries)) return XUI_DOC_ERROR_LIMIT;
    entries = doc_alloc(m->allocator, (size_t)capacity * sizeof(*entries));
    if (!entries) return XUI_ERROR_OUT_OF_MEMORY;
    for (i = 0; i < m->capacity; i++) if (m->entries[i].allocation) {
        uint64_t slot = doc_charge_hash(m->entries[i].allocation) & (capacity - 1);
        while (entries[slot].allocation) slot = (slot + 1) & (capacity - 1);
        entries[slot] = m->entries[i];
    }
    doc_free(m->entries); m->entries = entries; m->capacity = capacity; return XUI_OK;
}
static int doc_charge_visit(doc_charge_map* m, void* pointer, unsigned owner, int add)
{
    doc_allocation* h; doc_charge_entry* entry; uint64_t slot; unsigned i; int result;
    if (!pointer) return XUI_OK;
    if (!(m->visits++ & 255) && m->cancellation && (result = atomic_load(m->cancellation)) != 0) return result;
    h = (doc_allocation*)pointer - 1;
    if (!m->capacity || (add && m->size >= m->capacity - m->capacity / 4)) {
        result = doc_charge_grow(m); if (result != XUI_OK) return result;
    }
    slot = doc_charge_hash(h) & (m->capacity - 1);
    while (m->entries[slot].allocation && m->entries[slot].allocation != h) slot = (slot + 1) & (m->capacity - 1);
    entry = &m->entries[slot];
    if (!entry->allocation) { assert(add); entry->allocation = h; m->size++; }
    if (add) { if (entry->counts[owner] == UINT64_MAX) return XUI_DOC_ERROR_LIMIT; if (entry->counts[owner]++) return XUI_OK; }
    else { assert(entry->counts[owner]); if (--entry->counts[owner]) return XUI_OK; }
    if (owner == 3 && !entry->counts[2]) {
        if (add) m->extra_history += h->value.bytes;
        else { assert(m->extra_history >= h->value.bytes); m->extra_history -= h->value.bytes; }
    }
#define VISIT(p) do { result = doc_charge_visit(m, (p), owner, add); if (result != XUI_OK) return result; } while (0)
    switch (h->value.kind) {
    case DOC_MEMORY_SEQUENCE: {
        doc_sequence* p = pointer; VISIT(p->left); VISIT(p->right); VISIT(p->blob); VISIT(p->value); break;
    }
    case DOC_MEMORY_NODE: {
        doc_node* p = pointer; VISIT(doc_attribute_owner(p->attrs)); VISIT(p->text); VISIT(p->children); VISIT(p->provenance);
        VISIT(p->resource); VISIT(p->info); VISIT(p->title);
        VISIT(p->link_target); VISIT(p->link_title);
        VISIT(p->column_widths); VISIT(p->extension_payload); VISIT(p->quote_prefixes); VISIT(p->syntax_aux); VISIT(p->list_indents); break;
    }
    case DOC_MEMORY_INDEX: {
        doc_index* p = pointer; for (i = 0; i < DOC_INDEX_SIZE; i++) VISIT(p->slots[i]); break;
    }
    case DOC_MEMORY_STATE: {
        doc_state* p = pointer; VISIT(p->index); VISIT(p->source); VISIT(p->references); VISIT(p->reference_values); VISIT(p->inline_syntax);
        VISIT(p->reference_candidates); break;
    }
    default: assert(h->value.kind == DOC_MEMORY_BLOB || h->value.kind == DOC_MEMORY_ATTRIBUTE); break;
    }
#undef VISIT
    return XUI_OK;
}
static uint64_t doc_charge_record_bytes(doc_history* h)
{
    return ((doc_allocation*)h - 1)->value.bytes + (h->ops ? ((doc_allocation*)h->ops - 1)->value.bytes : 0);
}
static int doc_charge_history(doc_charge_map* m, doc_history* h, unsigned owner, int add)
{
    int result = doc_charge_visit(m, h->before, owner, add);
    return result == XUI_OK ? doc_charge_visit(m, h->after, owner, add) : result;
}

/* Start with the live owner counters and simulate only 0<->1 transitions.
 * The current root is sufficient even when history was just cleared or is
 * disabled; adding a new history record may still visit that root once.
 * Shared persistent branches stop the walk immediately. A concurrent live
 * write invalidates the captured revision/history at publication. */
static int doc_charge_visit_delta(doc_charge_map* m, void* pointer, unsigned owner, int add)
{
    doc_allocation* h; doc_charge_entry* entry; uint64_t slot, before_count; unsigned i;
    int result, was_extra, is_extra;
    if (!pointer) return XUI_OK;
    if (!(m->visits++ & 255) && m->cancellation && (result = atomic_load(m->cancellation)) != 0) return result;
    h = (doc_allocation*)pointer - 1;
    if (!m->capacity || (m->size >= m->capacity - m->capacity / 4)) {
        result = doc_charge_grow(m); if (result != XUI_OK) return result;
    }
    slot = doc_charge_hash(h) & (m->capacity - 1);
    while (m->entries[slot].allocation && m->entries[slot].allocation != h) slot = (slot + 1) & (m->capacity - 1);
    entry = &m->entries[slot];
    if (!entry->allocation) {
        entry->allocation = h; m->size++;
        doc_memory_owner_counts(h, &entry->counts[0], &entry->counts[1]);
        entry->counts[2] = entry->counts[0]; entry->counts[3] = entry->counts[1];
    }
    before_count = entry->counts[owner];
    was_extra = !entry->counts[2] && !!entry->counts[3];
    if (add) {
        if (before_count == UINT64_MAX) return XUI_DOC_ERROR_LIMIT;
        entry->counts[owner]++;
    } else {
        if (!before_count) return XUI_DOC_ERROR_STALE;
        entry->counts[owner]--;
    }
    is_extra = !entry->counts[2] && !!entry->counts[3];
    if (was_extra != is_extra) {
        if (is_extra) {
            if (m->extra_history > UINT64_MAX - h->value.bytes) return XUI_DOC_ERROR_LIMIT;
            m->extra_history += h->value.bytes;
        } else {
            if (m->extra_history < h->value.bytes) return XUI_DOC_ERROR_STALE;
            m->extra_history -= h->value.bytes;
        }
    }
    if ((add && before_count) || (!add && before_count != 1)) return XUI_OK;
#define VISIT_DELTA(p) do { result = doc_charge_visit_delta(m, (p), owner, add); if (result != XUI_OK) return result; } while (0)
    switch (h->value.kind) {
    case DOC_MEMORY_SEQUENCE: {
        doc_sequence* p = pointer; VISIT_DELTA(p->left); VISIT_DELTA(p->right); VISIT_DELTA(p->blob); VISIT_DELTA(p->value); break;
    }
    case DOC_MEMORY_NODE: {
        doc_node* p = pointer; VISIT_DELTA(doc_attribute_owner(p->attrs)); VISIT_DELTA(p->text); VISIT_DELTA(p->children); VISIT_DELTA(p->provenance);
        VISIT_DELTA(p->resource); VISIT_DELTA(p->info); VISIT_DELTA(p->title);
        VISIT_DELTA(p->link_target); VISIT_DELTA(p->link_title);
        VISIT_DELTA(p->column_widths); VISIT_DELTA(p->extension_payload); VISIT_DELTA(p->quote_prefixes); VISIT_DELTA(p->syntax_aux); VISIT_DELTA(p->list_indents); break;
    }
    case DOC_MEMORY_INDEX: {
        doc_index* p = pointer; for (i = 0; i < DOC_INDEX_SIZE; i++) VISIT_DELTA(p->slots[i]); break;
    }
    case DOC_MEMORY_STATE: {
        doc_state* p = pointer; VISIT_DELTA(p->index); VISIT_DELTA(p->source);
        VISIT_DELTA(p->references); VISIT_DELTA(p->reference_values); VISIT_DELTA(p->inline_syntax); VISIT_DELTA(p->reference_candidates); break;
    }
    default: assert(h->value.kind == DOC_MEMORY_BLOB || h->value.kind == DOC_MEMORY_ATTRIBUTE); break;
    }
#undef VISIT_DELTA
    return XUI_OK;
}
static int doc_charge_history_delta(doc_charge_map* m, doc_history* h, int add)
{
    int result = doc_charge_visit_delta(m, h->before, 3, add);
    return result == XUI_OK ? doc_charge_visit_delta(m, h->after, 3, add) : result;
}
doc_publish_plan* doc_publish_plan_capture(xui_document d)
{
    doc_publish_plan* p = doc_alloc(d->allocator, sizeof(*p)); doc_history* h; uint64_t count, i = 0;
    if (!p) return NULL;
    p->allocator = d->allocator; p->revision = d->revision; p->limit = d->history_limit;
    p->max_bytes = d->history_max_bytes; p->disabled = d->disable_history;
    p->standby = d->standby_history;
    count = (uint64_t)d->undo_count + d->redo_count;
    if (count) {
        if (count > SIZE_MAX / sizeof(*p->history)) { doc_free(p); return NULL; }
        p->history = doc_alloc(d->allocator, (size_t)count * sizeof(*p->history));
        if (!p->history) { doc_free(p); return NULL; }
        for (h = d->undo; h; h = h->next) { doc_history_retain(h); p->history[i++] = h; }
        for (h = d->redo; h; h = h->next) { doc_history_retain(h); p->history[i++] = h; }
        assert(i == count);
    }
    p->undo_count = d->undo_count; p->redo_count = d->redo_count; return p;
}
static doc_history* doc_proposed_history(doc_publish_plan* p, uint64_t index, unsigned skip)
{
    if (p->next) { if (!index) return p->next; index--; }
    return p->history[index + skip];
}
static int doc_delta_order(const void* a, const void* b)
{
    uintptr_t x = (uintptr_t)((const doc_memory_delta*)a)->allocation, y = (uintptr_t)((const doc_memory_delta*)b)->allocation;
    return x < y ? -1 : x != y;
}
static int doc_charge_delta_plan(doc_publish_plan* p, doc_charge_map* map)
{
    uint64_t i, changed = 0;
    for (i = 0; i < map->capacity; i++) {
        doc_charge_entry* e = &map->entries[i];
        if (e->allocation && (e->counts[0] != e->counts[2] || e->counts[1] != e->counts[3])) changed++;
    }
    if (changed > SIZE_MAX / sizeof(*p->deltas)) return XUI_DOC_ERROR_LIMIT;
    if (changed) {
        p->deltas = doc_alloc(p->allocator, (size_t)changed * sizeof(*p->deltas));
        if (!p->deltas) return XUI_ERROR_OUT_OF_MEMORY;
    }
    for (i = 0; i < map->capacity; i++) {
        doc_charge_entry* e = &map->entries[i]; unsigned before, after;
        if (!e->allocation || (e->counts[0] == e->counts[2] && e->counts[1] == e->counts[3])) continue;
        p->deltas[p->count].allocation = e->allocation; p->deltas[p->count].current = e->counts[2];
        p->deltas[p->count++].history = e->counts[3];
        before = (unsigned)!!e->counts[0] | ((unsigned)!!e->counts[1] << 1);
        after = (unsigned)!!e->counts[2] | ((unsigned)!!e->counts[3] << 1);
        if (before != after) { p->removed[before] += e->allocation->value.bytes; p->added[after] += e->allocation->value.bytes; }
    }
    if (p->count > 1) qsort(p->deltas, (size_t)p->count, sizeof(*p->deltas), doc_delta_order);
    p->ready = 1;
    atomic_fetch_add(&p->allocator->prepared_accounting_visits, map->visits);
    return XUI_OK;
}
static int doc_charge_plan_fast(doc_publish_plan* p, doc_charge_map* map,
    xui_document_transaction t, unsigned skip)
{
    uint64_t i, current, history; int found = 0, result;
    for (i = 0; i < p->undo_count + p->redo_count; i++)
        if (p->history[i]->before == t->base || p->history[i]->after == t->base) { found = 1; break; }
    doc_memory_owner_counts((const doc_allocation*)t->base - 1, &current, &history);
    if (!current || (found && !history)) return XUI_ERROR_UNSUPPORTED;
    map->extra_history = doc_memory_storage_history_bytes(p->allocator);
    result = doc_charge_visit_delta(map, t->draft, 2, 1);
    if (result == XUI_OK) result = doc_charge_visit_delta(map, t->base, 2, 0);
    if (result == XUI_OK && p->next) result = doc_charge_history_delta(map, p->next, 1);
    if (result == XUI_OK && p->standby) result = doc_charge_visit_delta(map, t->base, 3, 0);
    if (result == XUI_OK && skip) result = doc_charge_history_delta(map, p->history[0], 0);
    for (i = p->undo_count; result == XUI_OK && i < p->undo_count + p->redo_count; i++)
        result = doc_charge_history_delta(map, p->history[i], 0);
    for (i = 0; result == XUI_OK && i < p->keep_undo; i++)
        p->record_bytes += doc_charge_record_bytes(doc_proposed_history(p, i, skip));
    while (result == XUI_OK && p->keep_undo && (p->keep_undo > p->limit ||
        p->record_bytes > p->max_bytes || map->extra_history > p->max_bytes - p->record_bytes)) {
        doc_history* h;
        if (p->keep_undo == 1 && !p->disabled) {
            result = doc_charge_visit_delta(map, t->draft, 3, 1);
            if (result != XUI_OK) break;
        }
        h = doc_proposed_history(p, --p->keep_undo, skip);
        result = doc_charge_history_delta(map, h, 0); p->record_bytes -= doc_charge_record_bytes(h);
    }
    return result;
}
int doc_publish_plan_build(doc_publish_plan* p, xui_document_transaction t, uint64_t identity)
{
    doc_charge_map map = {0}; uint64_t i; int result; unsigned skip = 0;
    map.allocator = p->allocator; map.cancellation = t->cancellation;
    if (!p->disabled) {
        p->next = doc_history_prepare(t, p->undo_count ? p->history[0] : NULL);
        if (!p->next) return t->error ? t->error : XUI_ERROR_OUT_OF_MEMORY;
        if (p->undo_count && p->next->before == p->history[0]->before && p->next->group &&
            p->next->group == p->history[0]->group && p->next->origin == p->history[0]->origin) skip = 1;
    }
    p->change = doc_change_create(p->allocator, identity, p->revision, t->base, t->draft,
        t->ops, t->count, t->flags, t->domain, t->origin, 0);
    if (!p->change) return XUI_ERROR_OUT_OF_MEMORY;
    p->keep_undo = p->undo_count - skip + !!p->next;
    result = doc_charge_plan_fast(p, &map, t, skip);
    if (result == XUI_DOC_ERROR_STALE) {
        uint64_t attempted = map.visits;
        /* A history-only change may happen while Run is building the plan.
         * Keep the candidate usable: the root-based calculation is detached
         * from mutable owner counters, and Publish will revalidate anyway. */
        doc_free(map.entries); memset(&map, 0, sizeof(map));
        map.allocator = p->allocator; map.cancellation = t->cancellation; map.visits = attempted;
        p->record_bytes = 0; p->keep_undo = p->undo_count - skip + !!p->next;
        result = XUI_ERROR_UNSUPPORTED;
    }
    if (result != XUI_ERROR_UNSUPPORTED) goto finish;
    result = doc_charge_visit(&map, t->base, 0, 1);
    if (result == XUI_OK && p->standby) result = doc_charge_visit(&map, t->base, 1, 1);
    for (i = 0; result == XUI_OK && i < p->undo_count + p->redo_count; i++) result = doc_charge_history(&map, p->history[i], 1, 1);
    if (result == XUI_OK) result = doc_charge_visit(&map, t->draft, 2, 1);
    for (i = 0; result == XUI_OK && i < p->keep_undo; i++) {
        doc_history* h = doc_proposed_history(p, i, skip);
        result = doc_charge_history(&map, h, 3, 1); p->record_bytes += doc_charge_record_bytes(h);
    }
    /* A new root commit clears Redo. As in the synchronous path, evict the
     * oldest Undo records until both limits hold, even the new record itself. */
    while (result == XUI_OK && p->keep_undo && (p->keep_undo > p->limit ||
        p->record_bytes > p->max_bytes || map.extra_history > p->max_bytes - p->record_bytes)) {
        doc_history* h;
        if (p->keep_undo == 1 && !p->disabled) {
            result = doc_charge_visit(&map, t->draft, 3, 1);
            if (result != XUI_OK) break;
        }
        h = doc_proposed_history(p, --p->keep_undo, skip);
        result = doc_charge_history(&map, h, 3, 0); p->record_bytes -= doc_charge_record_bytes(h);
    }
finish:
    if (result == XUI_OK) result = doc_charge_delta_plan(p, &map);
    doc_free(map.entries); return result;
}
int doc_publish_plan_valid(doc_publish_plan* p, xui_document d)
{
    return p && p->ready && p->revision == d->revision && p->limit == d->history_limit && p->max_bytes == d->history_max_bytes &&
        p->disabled == d->disable_history && p->standby == d->standby_history &&
        p->undo_count == d->undo_count && p->redo_count == d->redo_count &&
        (p->undo_count ? p->history[0] : NULL) == d->undo && (p->redo_count ? p->history[p->undo_count] : NULL) == d->redo;
}
void doc_publish_plan_apply(doc_publish_plan* p)
{
    uint64_t i;
    doc_memory_apply_prepared(p->allocator, p->deltas, p->count, p->added, p->removed, p->record_bytes);
    /* Commit updates the live lists after this; removed records are already
     * unaccounted. Retained records are marked again once the lists are final. */
    for (i = 0; i < p->undo_count + p->redo_count; i++) p->history[i]->accounted = 0;
}
uint64_t doc_publish_plan_undo_count(doc_publish_plan* p) { return p->keep_undo; }
doc_history* doc_publish_plan_take_history(doc_publish_plan* p)
{
    doc_history* h = p->next; p->next = NULL; return h;
}
xui_document_change_set doc_publish_plan_change(doc_publish_plan* p)
{
    xuiDocumentChangeSetRetain(p->change); return p->change;
}
void doc_publish_plan_release(doc_publish_plan* p)
{
    uint64_t i;
    if (!p) return;
    for (i = 0; i < p->undo_count + p->redo_count; i++) doc_history_release(p->history[i]);
    doc_history_release(p->next); xuiDocumentChangeSetRelease(p->change);
    doc_free(p->history); doc_free(p->deltas); doc_free(p);
}

#endif
