/* Exercise source retention through public edits and independent reloads. */
#define main xui_document_core_uninvoked
#include "xui_document_test.c"
#undef main
#include "xui_document_reference_sharing_cases.h"

#ifndef XUI_DLL
static uint64_t source_compaction_payload(doc_sequence* source)
{
    memory_oracle oracle = {0}; uint64_t bytes = 0; size_t i;
    memory_oracle_visit(&oracle, source, 1);
    for (i = 0; i < oracle.count; i++) {
        doc_allocation* allocation = (doc_allocation*)oracle.entries[i].pointer - 1;
        if (allocation->value.kind == DOC_MEMORY_BLOB) bytes += ((doc_blob*)oracle.entries[i].pointer)->size;
    }
    free(oracle.entries); return bytes;
}
static void source_compaction_accounting(xui_document d, int exact)
{
    uint64_t payload = source_compaction_payload(d->state->source);
    CHECK(d->state->source_storage_bytes >= payload);
    CHECK(d->state->source_storage_bytes <= 4096 ||
        doc_seq_size(d->state->source) > (d->state->source_storage_bytes - 1) / 4);
    if (exact) CHECK(d->state->source_storage_bytes == payload);
    memory_oracle_check(d);
}
#else
static void source_compaction_accounting(xui_document d, int exact) { (void)d; (void)exact; }
#endif

static void source_compaction_snapshot_matches(xui_document_snapshot snapshot, const char* source)
{
    xui_document oracle = test_markdown_open(source); xui_document_snapshot expected;
    CHECK(xuiDocumentAcquireSnapshot(oracle, &expected) == XUI_OK);
    inc_snapshot_equal(snapshot, expected); xuiDocumentSnapshotRelease(expected); xuiDocumentRelease(oracle);
}

static void source_compaction_shrink(void)
{
    static const size_t sizes[] = {4096, 131072, 1048576}; unsigned si, used, mode, cases = 0;
    for (si = 0; si < sizeof(sizes) / sizeof(*sizes); si++) for (used = 0; used < 2; used++) for (mode = 0; mode < 7; mode++) {
        size_t offset, n = sizes[si]; char* source = reference_sharing_source(n, (int)used, &offset), *edited;
        xui_document d = test_markdown_open(source); xui_document_snapshot before, after, restored;
        xui_doc_txn_desc_t td = {0}; xui_document_transaction t = NULL; xui_document_prepare p = NULL, next;
        xui_doc_source_patch_t patch; xui_doc_memory_stats_t memory;
        if (mode == 4) {
            char* wire; uint64_t bytes; xui_document loaded;
            CHECK(xuiDocumentAcquireSnapshot(d, &restored) == XUI_OK && xuiDocumentSerialize(restored, &wire, &bytes) == XUI_OK);
            CHECK(xuiDocumentDeserialize(NULL, wire, bytes, &loaded) == XUI_OK);
            xuiDocumentFreeBuffer(wire); xuiDocumentSnapshotRelease(restored); xuiDocumentRelease(d); d = loaded;
        }
        edited = malloc(strlen(source) + 1); CHECK(edited); strcpy(edited, source);
        if (mode == 5) edited[0] = 0;
        else memmove(edited + offset + 64, edited + offset + n, strlen(edited + offset + n) + 1);
        CHECK(xuiDocumentClearHistory(d) == XUI_OK && xuiDocumentAcquireSnapshot(d, &before) == XUI_OK);
        td.iSize = sizeof(td); td.iDomain = XUI_DOC_SOURCE;
        if (mode >= 1 && mode <= 3) {
            patch = prepare_patch(offset + 64, offset + (mode >= 2 ? n / 2 : n), "");
            CHECK(xuiDocumentPrepareSource(d, &td, &patch, 1, &p) == XUI_OK);
            if (mode >= 2) {
                if (mode == 3) CHECK(xuiDocumentPrepareRun(p) == XUI_OK);
                patch = prepare_patch(offset + 64, offset + 64 + n / 2, "");
                CHECK(xuiDocumentPrepareContinueSource(d, p, &patch, 1, &next) == XUI_OK);
                xuiDocumentPrepareRelease(p); p = next;
            }
            CHECK(xuiDocumentPrepareRun(p) == XUI_OK && xuiDocumentPreparePublish(d, p, NULL) == XUI_OK);
            xuiDocumentPrepareRelease(p);
        } else {
            CHECK(xuiDocumentBeginTransaction(d, &td, &t) == XUI_OK);
            if (mode == 5) CHECK(xuiDocumentTxnReplaceSource(t, 0, strlen(source), "", 0) == XUI_OK);
            else if (mode == 6) {
                size_t retained = n;
                while (retained > 64) {
                    size_t next_size = retained / 2;
                    CHECK(xuiDocumentTxnReplaceSource(t, offset + next_size, offset + retained, "", 0) == XUI_OK);
                    retained = next_size;
                }
            } else CHECK(xuiDocumentTxnReplaceSource(t, offset + 64, offset + n, "", 0) == XUI_OK);
            CHECK(xuiDocumentTxnCommit(t, NULL) == XUI_OK); xuiDocumentTxnRelease(t);
        }
        reference_sharing_compare(d, edited); CHECK(xuiDocumentAcquireSnapshot(d, &after) == XUI_OK);
        CHECK(xuiDocumentUndo(d, NULL) == XUI_OK && xuiDocumentAcquireSnapshot(d, &restored) == XUI_OK);
        inc_snapshot_equal(before, restored); xuiDocumentSnapshotRelease(restored);
        CHECK(xuiDocumentRedo(d, NULL) == XUI_OK && xuiDocumentAcquireSnapshot(d, &restored) == XUI_OK);
        inc_snapshot_equal(after, restored); xuiDocumentSnapshotRelease(restored);
        CHECK(xuiDocumentClearHistory(d) == XUI_OK); source_compaction_accounting(d, mode != 6);
        memory = document_memory(d); CHECK(memory.iCurrentBytes < 65536 && !memory.iHistoryBytes);
        printf("Source compaction body=%zu used=%u mode=%u CurrentBytes=%llu retained-old-snapshot=%llu\n", n, used, mode,
            (unsigned long long)memory.iCurrentBytes, (unsigned long long)memory.iSnapshotAdditionalBytes);
        xuiDocumentRelease(d); memory.iSize = sizeof(memory);
        CHECK(xuiDocumentSnapshotGetMemoryStats(after, &memory) == XUI_OK &&
            !memory.iCurrentBytes && !memory.iHistoryBytes && !memory.iOtherBytes &&
            memory.iLiveBytes == memory.iSnapshotBytes && memory.iSnapshotCount == 2);
        source_compaction_snapshot_matches(before, source); source_compaction_snapshot_matches(after, edited);
        xuiDocumentSnapshotRelease(before);
        CHECK(xuiDocumentSnapshotGetMemoryStats(after, &memory) == XUI_OK &&
            memory.iLiveBytes < 131072 && memory.iSnapshotCount == 1);
        xuiDocumentSnapshotRelease(after); free(source); free(edited); cases++;
    }
    printf("Source compaction shrink: %u public SOURCE/Prepare/queued and ready Continue/native reload/empty/cumulative deletion, full oracle, Undo/Redo and owner accounting passed\n", cases);
}

static void source_compaction_many_blobs(void)
{
    unsigned si, pending, i; static const size_t sizes[] = {3072, 8192};
    for (si = 0; si < 2; si++) for (pending = 0; pending < 2; pending++) {
        size_t size = sizes[si], at = 6; char *chunk = malloc(size + 1), expected[6 + 96 * 32 + 1];
        xui_doc_source_patch_t patches[192]; xui_document d = test_markdown_open("start\n");
        xui_document_transaction t = NULL; xui_doc_txn_desc_t td = {0}; xui_document_prepare p;
        CHECK(chunk); memset(chunk, 'a', size); chunk[size] = 0; strcpy(expected, "start\n");
        for (i = 0; i < 96; i++) {
            patches[i * 2] = prepare_patch(at, at, chunk);
            patches[i * 2 + 1] = prepare_patch(at + 32, at + size, "");
            memset(expected + at, 'a', 32); at += 32; expected[at] = 0;
        }
        CHECK(xuiDocumentClearHistory(d) == XUI_OK); td.iSize = sizeof(td); td.iDomain = XUI_DOC_SOURCE;
        if (pending) {
            CHECK(xuiDocumentPrepareSource(d, &td, patches, 192, &p) == XUI_OK);
            CHECK(xuiDocumentPrepareRun(p) == XUI_OK && xuiDocumentPreparePublish(d, p, NULL) == XUI_OK);
            xuiDocumentPrepareRelease(p);
        } else {
            CHECK(xuiDocumentBeginTransaction(d, &td, &t) == XUI_OK);
            for (i = 0; i < 192; i++) CHECK(xuiDocumentTxnReplaceSource(t, patches[i].iStart, patches[i].iEnd, patches[i].sText, patches[i].iTextBytes) == XUI_OK);
            CHECK(xuiDocumentTxnCommit(t, NULL) == XUI_OK); xuiDocumentTxnRelease(t);
        }
        reference_sharing_compare(d, expected); CHECK(xuiDocumentClearHistory(d) == XUI_OK); source_compaction_accounting(d, pending != 0);
        printf("Source compaction 96 distinct %zu-byte allocations prepare=%u: source=%zu CurrentBytes=%llu\n", size, pending, strlen(expected),
            (unsigned long long)document_memory(d).iCurrentBytes);
#ifndef XUI_DLL
        {
            memory_oracle text = {0};
            reference_sharing_text_memory(&text, d->state, DOC_ROOT);
            printf("Fragmentation independent source-payload=%llu semantic-text-storage=%llu\n",
                (unsigned long long)source_compaction_payload(d->state->source),
                (unsigned long long)reference_sharing_allocation_bytes(&text));
            free(text.entries);
        }
#endif
        CHECK(document_memory(d).iCurrentBytes < 65536);
        xuiDocumentRelease(d); free(chunk);
    }
    puts("Source compaction fragmented input: large and sub-4KiB backing Blobs, 192 sequential patches, SOURCE and worker Prepare passed");
}

static void source_compaction_continue_running(void)
{
    unsigned used;
    for (used = 0; used < 2; used++) {
        prepare_gate_allocator a; xui_doc_desc_t desc = {0}; xui_document d; xui_document_prepare p, next;
        size_t offset; char* source = reference_sharing_source(131072, (int)used, &offset), *edited = malloc(strlen(source) + 2);
        xui_doc_source_patch_t patch = prepare_patch(offset + 64, offset + 131072, ""); xthread* worker;
        CHECK(edited); strcpy(edited, source);
        memmove(edited + offset + 64, edited + offset + 131072, strlen(edited + offset + 131072) + 1);
        atomic_init(&a.live, 0); atomic_init(&a.gate, 0); a.owner = xrtThreadCurrentId();
        atomic_init(&a.entered, 0); atomic_init(&a.resume, 0);
        desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN; desc.onAlloc = prepare_gate_alloc; desc.onFree = prepare_gate_free; desc.pAllocatorUser = &a;
        CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK && xuiDocumentLoadMarkdown(d, source, strlen(source)) == XUI_OK && xuiDocumentClearHistory(d) == XUI_OK);
        CHECK(xuiDocumentPrepareSource(d, NULL, &patch, 1, &p) == XUI_OK);
        atomic_store(&a.gate, 1); worker = prepare_start(p); prepare_gate_wait(&a.entered);
        CHECK(prepare_info(p).iState == XUI_DOC_PREPARE_RUNNING); prepare_source(p, edited);
        patch = prepare_patch(offset + 1, offset + 1, "Y");
        CHECK(xuiDocumentPrepareContinueSource(d, p, &patch, 1, &next) == XUI_OK);
        memmove(edited + offset + 2, edited + offset + 1, strlen(edited + offset + 1) + 1); edited[offset + 1] = 'Y';
        atomic_store(&a.resume, 1); CHECK(prepare_finish(worker) == XUI_DOC_ERROR_STALE);
        xuiDocumentPrepareRelease(p);
        CHECK(xuiDocumentPrepareRun(next) == XUI_OK && xuiDocumentPreparePublish(d, next, NULL) == XUI_OK);
        xuiDocumentPrepareRelease(next); reference_sharing_compare(d, edited);
        CHECK(xuiDocumentClearHistory(d) == XUI_OK); source_compaction_accounting(d, 1);
        CHECK(document_memory(d).iCurrentBytes < 65536); xuiDocumentRelease(d); CHECK(!atomic_load(&a.live));
        free(source); free(edited);
    }
    puts("Source compaction Continue while Run is paused: immutable source/accounting input, stale predecessor, full oracle and no leaks passed");
}

static void source_compaction_prepare_faults(void)
{
    unsigned cancel, used; long totals[2] = {0};
    for (cancel = 0; cancel < 2; cancel++) for (used = 0; used < 2; used++) {
        size_t offset; char* source = reference_sharing_source(8192, (int)used, &offset); long point; int success = 0;
        xui_doc_source_patch_t patch = prepare_patch(offset + 64, offset + 8192, "");
        for (point = 0; point < 3000 && !success; point++) {
            fail_allocator failing = {-1, 0}; prepare_cancel_allocator cancelling = {0, -1, 0, NULL};
            xui_doc_desc_t desc = {0}; xui_document d; xui_document_prepare p = NULL;
            xui_document_snapshot before, after; uint64_t revision; int result;
            desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN;
            desc.onAlloc = cancel ? prepare_cancel_alloc : failing_alloc; desc.onFree = cancel ? prepare_cancel_free : failing_free;
            desc.pAllocatorUser = cancel ? (void*)&cancelling : (void*)&failing;
            CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK && xuiDocumentLoadMarkdown(d, source, strlen(source)) == XUI_OK &&
                xuiDocumentClearHistory(d) == XUI_OK && xuiDocumentAcquireSnapshot(d, &before) == XUI_OK);
            CHECK(xuiDocumentMarkSaved(d, before) == XUI_OK); revision = xuiDocumentGetRevision(d);
            if (!cancel) failing.remaining = point;
            result = xuiDocumentPrepareSource(d, NULL, &patch, 1, &p);
            if (cancel) { cancelling.prepare = p; cancelling.remaining = point; }
            if (result == XUI_OK) result = xuiDocumentPrepareRun(p);
            cancelling.prepare = NULL; cancelling.remaining = failing.remaining = -1;
            if (result == XUI_OK) result = xuiDocumentPreparePublish(d, p, NULL);
            CHECK(xuiDocumentAcquireSnapshot(d, &after) == XUI_OK);
            if (result != XUI_OK) {
                CHECK(result == (cancel ? XUI_DOC_ERROR_CANCELLED : XUI_ERROR_OUT_OF_MEMORY));
                CHECK(xuiDocumentGetRevision(d) == revision && !xuiDocumentCanUndo(d) && !xuiDocumentIsDirty(d));
                expect_identity_tree(before, after, XUI_DOCUMENT_ROOT); expect_same_syntax(before, after); inc_snapshot_equal(before, after);
                test_source(d, source, 1);
            } else { success = 1; CHECK(xuiDocumentUndo(d, NULL) == XUI_OK && !xuiDocumentIsDirty(d)); }
            xuiDocumentSnapshotRelease(before); xuiDocumentSnapshotRelease(after); xuiDocumentPrepareRelease(p);
            CHECK(xuiDocumentCancelPrepare(d) == XUI_OK); source_compaction_accounting(d, 0); xuiDocumentRelease(d);
            CHECK(!failing.live && !cancelling.live);
        }
        CHECK(success); totals[cancel] += point - 1;
        printf("Source compaction Prepare %s used=%u: %ld fault checkpoints atomic and leak-free\n", cancel ? "cancel" : "OOM", used, point - 1);
        free(source);
    }
    printf("Source compaction Prepare faults: %ld OOM and %ld cancellation checkpoints passed\n", totals[0], totals[1]);
}

#ifndef XUI_DLL
static void source_compaction_coverage(void)
{
    unsigned sparse;
    for (sparse = 0; sparse < 2; sparse++) {
        xui_document d = test_markdown_open("seed"); doc_allocator* allocator = d->allocator;
        char original[32768], expected[512 * 20], actual[512 * 20]; doc_blob* blob; doc_sequence *source = NULL, *old;
        uint64_t retained = UINT64_MAX, i, bytes = sparse ? 8 : 20;
        for (i = 0; i < sizeof(original); i++) original[i] = (char)('a' + i % 26);
        blob = doc_blob_new(allocator, original, sizeof(original)); CHECK(blob);
        for (i = 0; i < 512; i++) {
            doc_sequence *piece = doc_seq_blob_range(allocator, blob, i * 64, bytes), *next = NULL;
            CHECK(piece && doc_seq_replace(allocator, source, doc_seq_size(source), doc_seq_size(source), piece, &next) == XUI_OK);
            doc_seq_release(piece); doc_seq_release(source); source = next;
            memcpy(expected + i * bytes, original + i * 64, (size_t)bytes);
        }
        old = source; doc_seq_retain(old);
        CHECK(doc_seq_compact_bytes(allocator, &source, NULL, &retained) == XUI_OK);
        CHECK(doc_seq_read(source, 0, actual, bytes * 512) == XUI_OK && !memcmp(expected, actual, (size_t)bytes * 512));
        CHECK(retained == source_compaction_payload(source));
        if (sparse) CHECK(retained == bytes * 512 && source != old);
        else CHECK(retained == sizeof(original) && source == old);
        CHECK(doc_seq_read(old, 0, actual, bytes * 512) == XUI_OK && !memcmp(expected, actual, (size_t)bytes * 512));
        doc_seq_release(old); doc_seq_release(source); doc_blob_release(blob); xuiDocumentRelease(d);
    }
    puts("Source compaction combined coverage: 512 individually short ranges retain a collectively dense Blob, sparse ranges compact, old immutable roots unchanged");
}
#else
static void source_compaction_coverage(void) {}
#endif

int main(void)
{
    source_compaction_shrink();
    source_compaction_many_blobs();
    source_compaction_continue_running();
    source_compaction_prepare_faults();
    source_compaction_coverage();
    return 0;
}
