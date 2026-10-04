#ifndef XUI_DOCUMENT_MEMORY_ORACLE_H
#define XUI_DOCUMENT_MEMORY_ORACLE_H
#include "../src/xui_document_internal.h"
/* Compute allocation unions from immutable roots without production memory
 * visitors, prepared deltas, buckets or mutable allocation owner counters. */
typedef struct memory_oracle_entry { void* pointer; unsigned mask; } memory_oracle_entry;
typedef struct memory_oracle { memory_oracle_entry* entries; size_t count, capacity; } memory_oracle;
static void memory_oracle_visit(memory_oracle* oracle, void* pointer, unsigned mask)
{
    size_t i; doc_allocation* header; unsigned j;
    if (!pointer) return;
    for (i = 0; i < oracle->count && oracle->entries[i].pointer != pointer; i++) {}
    if (i == oracle->count) {
        if (i == oracle->capacity) {
            size_t capacity = oracle->capacity ? oracle->capacity * 2 : 256;
            memory_oracle_entry* next = realloc(oracle->entries, capacity * sizeof(*next));
            CHECK(next); oracle->entries = next; oracle->capacity = capacity;
        }
        oracle->entries[i].pointer = pointer; oracle->entries[i].mask = 0; oracle->count++;
    }
    if (oracle->entries[i].mask & mask) return;
    oracle->entries[i].mask |= mask;
    header = (doc_allocation*)pointer - 1;
#define WALK(p) memory_oracle_visit(oracle, (p), mask)
    switch (header->value.kind) {
    case DOC_MEMORY_SEQUENCE: {
        doc_sequence* p = pointer; WALK(p->left); WALK(p->right); WALK(p->blob); WALK(p->value); break;
    }
    case DOC_MEMORY_NODE: {
        doc_node* p = pointer; WALK(doc_attribute_owner(p->attrs)); WALK(p->text); WALK(p->children); WALK(p->provenance);
        WALK(p->resource); WALK(p->info); WALK(p->title); WALK(p->link_target); WALK(p->link_title);
        WALK(p->column_widths); WALK(p->extension_payload); WALK(p->quote_prefixes); WALK(p->syntax_aux); WALK(p->list_indents); break;
    }
    case DOC_MEMORY_INDEX: {
        doc_index* p = pointer; for (j = 0; j < DOC_INDEX_SIZE; j++) WALK(p->slots[j]); break;
    }
    case DOC_MEMORY_STATE: {
        doc_state* p = pointer; WALK(p->index); WALK(p->source); WALK(p->references); WALK(p->reference_values);
        WALK(p->inline_syntax); WALK(p->reference_candidates); break;
    }
    default: CHECK(header->value.kind == DOC_MEMORY_BLOB || header->value.kind == DOC_MEMORY_ATTRIBUTE); break;
    }
#undef WALK
}
static void memory_oracle_check(xui_document d)
{
    memory_oracle oracle = {0}; xui_doc_memory_stats_t actual = document_memory(d), expected = {0};
    xui_document_snapshot s; doc_history* h; uint64_t records = 0, union_bytes = 0; size_t i; unsigned list;
    expected.iSize = sizeof(expected); expected.iLiveBytes = atomic_load(&d->allocator->live);
    memory_oracle_visit(&oracle, d->state, 1);
    if (d->standby_history) memory_oracle_visit(&oracle, d->state, 2);
    for (list = 0; list < 2; list++) for (h = list ? d->redo : d->undo; h; h = h->next) {
        records += ((doc_allocation*)h - 1)->value.bytes;
        if (h->ops) records += ((doc_allocation*)h->ops - 1)->value.bytes;
        memory_oracle_visit(&oracle, h->before, 2); memory_oracle_visit(&oracle, h->after, 2);
    }
    for (s = d->allocator->snapshots; s; s = s->memory_next) {
        expected.iSnapshotCount++; expected.iSnapshotHandleBytes += ((doc_allocation*)s - 1)->value.bytes;
        memory_oracle_visit(&oracle, s->state, 4);
    }
    for (i = 0; i < oracle.count; i++) {
        doc_allocation* allocation = (doc_allocation*)oracle.entries[i].pointer - 1;
        uint64_t bytes = allocation->value.bytes; unsigned mask = oracle.entries[i].mask;
        union_bytes += bytes;
        if (mask & 1) expected.iCurrentBytes += bytes;
        if ((mask & 3) == 2) expected.iHistoryBytes += bytes;
        if (mask & 4) expected.iSnapshotBytes += bytes;
        if (mask == 4) expected.iSnapshotAdditionalBytes += bytes;
        CHECK(!!allocation->value.owners[0] == !!(mask & 1) &&
            !!allocation->value.owners[1] == !!(mask & 2) && !allocation->value.owners[2]);
    }
    expected.iHistoryBytes += records;
    expected.iSnapshotBytes += expected.iSnapshotHandleBytes;
    expected.iSnapshotAdditionalBytes += expected.iSnapshotHandleBytes;
    CHECK(expected.iLiveBytes >= union_bytes + records + expected.iSnapshotHandleBytes);
    expected.iOtherBytes = expected.iLiveBytes - union_bytes - records - expected.iSnapshotHandleBytes;
    if (memcmp(&actual, &expected, sizeof(actual))) fprintf(stderr, "Independent memory graph mismatch current=%llu/%llu history=%llu/%llu\n",
        (unsigned long long)actual.iCurrentBytes, (unsigned long long)expected.iCurrentBytes,
        (unsigned long long)actual.iHistoryBytes, (unsigned long long)expected.iHistoryBytes);
    CHECK(!memcmp(&actual, &expected, sizeof(actual))); free(oracle.entries);
}
#endif
