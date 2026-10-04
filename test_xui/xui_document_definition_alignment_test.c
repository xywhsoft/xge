/* Public edit/Prepare coverage with an independent complete-load oracle. */
#define main xui_document_core_uninvoked
#include "xui_document_test.c"
#undef main
#include "xui_document_reference_sharing_cases.h"

typedef struct alignment_fixture {
    char *source, *edited;
    size_t capacity, body;
    char needle[33];
    xui_doc_source_patch_t patches[24];
    unsigned count;
} alignment_fixture;

static void alignment_patch(alignment_fixture* f, size_t begin, size_t end, const char* text)
{
    size_t bytes = strlen(text), size = strlen(f->edited);
    char* copy = malloc(bytes + 1);
    CHECK(copy && begin <= end && end <= size && f->count < 24 &&
        size - (end - begin) + bytes < f->capacity);
    memcpy(copy, text, bytes + 1);
    f->patches[f->count++] = prepare_patch(begin, end, copy);
    memmove(f->edited + begin + bytes, f->edited + end, size - end + 1);
    memcpy(f->edited + begin, copy, bytes);
}
static void alignment_replace(alignment_fixture* f, const char* old, const char* text)
{
    char* at = strstr(f->edited, old);
    CHECK(at); alignment_patch(f, (size_t)(at - f->edited),
        (size_t)(at - f->edited) + strlen(old), text);
}
static size_t alignment_body(alignment_fixture* f)
{
    char* at = strstr(f->edited, f->needle); CHECK(at); return (size_t)(at - f->edited);
}
static alignment_fixture alignment_make(size_t body, unsigned used, unsigned operation)
{
    const char* prefix = used ? "use [^n] and [r] and [n]\n\n[^a]: first\n\n[^n]: " :
        "use [r] and [n]\n\n[^a]: first\n\n[^n]: ";
    const char* suffix = "\n\n[^n]: tiny duplicate\n\n[^b]: last\n\n[r]: /old 'Title'\n\n[n]: /typed 'LinkTitle'\n\n[t]: /title \"\"\n";
    alignment_fixture f = {0}; size_t i, offset = strlen(prefix); uint32_t random = 734;
    f.capacity = offset + body + strlen(suffix) + 2048; f.body = body;
    f.source = malloc(f.capacity); f.edited = malloc(f.capacity); CHECK(f.source && f.edited);
    memcpy(f.source, prefix, offset);
    for (i = 0; i < body; i++) {
        random = random * 1664525u + 1013904223u;
        f.source[offset + i] = (char)('a' + (random >> 16) % 26);
    }
    memcpy(f.needle, f.source + offset, 32); f.needle[32] = 0;
    strcpy(f.source + offset + body, suffix); strcpy(f.edited, f.source);
    alignment_replace(&f, "[^a]: first", "[^a]: FIRST");
    alignment_replace(&f, "[^b]: last", "[^b]: LAST");
    alignment_replace(&f, "/old", "/new");
    switch (operation) {
    case 0: alignment_patch(&f, 0, 0, "[^extra]: inserted\n\n"); break;
    case 1: alignment_replace(&f, "[^a]: FIRST\n\n", ""); break;
    case 2:
        alignment_replace(&f, "[^a]:", "[^z]:"); alignment_replace(&f, "'Title'", "'Other'");
        alignment_replace(&f, "[t]: /title \"\"", "[t]: /title"); break;
    case 3: case 4:
        alignment_replace(&f, "[^n]: tiny duplicate\n\n", "");
        alignment_patch(&f, 0, 0, "[^n]: tiny duplicate\n\n");
        if (operation == 4) {
            size_t at = alignment_body(&f) + body / 2; alignment_patch(&f, at, at + 1, "X");
        }
        break;
    case 5: {
        size_t at = alignment_body(&f) + body / 2;
        alignment_patch(&f, at, at + 1, "XY");
        alignment_patch(&f, 0, 0, "[^n]: new tiny duplicate\n\n");
        alignment_replace(&f, "'Title'", "\"\""); break;
    }
    case 6:
        alignment_replace(&f, "[n]: /typed 'LinkTitle'\n\n", "");
        alignment_patch(&f, 0, 0, "[^n]: typed appended\n\n"); break;
    case 7: {
        size_t at;
        alignment_replace(&f, "[^a]: FIRST\n\n", "");
        alignment_replace(&f, "[^n]: tiny duplicate\n\n", "");
        alignment_replace(&f, "[^b]: LAST\n\n", "");
        alignment_replace(&f, "[r]: /new 'Title'\n\n", "");
        alignment_replace(&f, "[n]: /typed 'LinkTitle'\n\n", "");
        alignment_replace(&f, "[t]: /title \"\"\n", "");
        at = alignment_body(&f) + body / 2; alignment_patch(&f, at, at + 2, "YZ"); break;
    }
    default: CHECK(0);
    }
    return f;
}
static void alignment_free(alignment_fixture* f)
{
    unsigned i; for (i = 0; i < f->count; i++) free((void*)f->patches[i].sText);
    free(f->source); free(f->edited);
}
static void alignment_snapshot(xui_document_snapshot actual, const char* text)
{
    xui_document oracle = test_markdown_open(text); xui_document_snapshot expected;
    uint64_t bytes; char* copied = malloc(strlen(text) + 1); CHECK(copied);
    CHECK(xuiDocumentSnapshotCopySource(actual, copied, strlen(text) + 1, &bytes) == XUI_OK &&
        bytes == strlen(text) && !memcmp(copied, text, (size_t)bytes));
    CHECK(xuiDocumentAcquireSnapshot(oracle, &expected) == XUI_OK);
    inc_snapshot_equal(actual, expected);
    free(copied); xuiDocumentSnapshotRelease(expected); xuiDocumentRelease(oracle);
}
static void alignment_execute(xui_document d, alignment_fixture* f, unsigned mode)
{
    xui_doc_txn_desc_t td = {0}; unsigned i;
    td.iSize = sizeof(td); td.iDomain = XUI_DOC_SOURCE;
    if (!mode) {
        xui_document_transaction t;
        CHECK(xuiDocumentBeginTransaction(d, &td, &t) == XUI_OK);
        for (i = 0; i < f->count; i++)
            CHECK(xuiDocumentTxnReplaceSource(t, f->patches[i].iStart, f->patches[i].iEnd,
                f->patches[i].sText, f->patches[i].iTextBytes) == XUI_OK);
        CHECK(xuiDocumentTxnCommit(t, NULL) == XUI_OK); xuiDocumentTxnRelease(t);
    } else {
        xui_document_prepare p, next;
        CHECK(xuiDocumentPrepareSource(d, &td, f->patches, mode == 1 ? f->count : 1, &p) == XUI_OK);
        if (mode >= 2) {
            if (mode == 3) CHECK(xuiDocumentPrepareRun(p) == XUI_OK);
            CHECK(xuiDocumentPrepareContinueSource(d, p, f->patches + 1, f->count - 1, &next) == XUI_OK);
            xuiDocumentPrepareRelease(p); p = next;
        }
        CHECK(xuiDocumentPrepareRun(p) == XUI_OK && xuiDocumentPreparePublish(d, p, NULL) == XUI_OK);
        xuiDocumentPrepareRelease(p);
    }
}
static void alignment_matrix(void)
{
    static const size_t sizes[] = {4096, 131072, 1048576};
    unsigned size, used, operation, mode, cases = 0;
    for (size = 0; size < 3; size++) for (used = 0; used < 2; used++)
    for (operation = 0; operation < 8; operation++) for (mode = 0; mode < 4; mode++) {
        alignment_fixture f = alignment_make(sizes[size], used, operation);
        xui_document d = test_markdown_open(f.source); xui_document_snapshot before, after, restored;
        xui_doc_memory_stats_t memory;
        CHECK(xuiDocumentClearHistory(d) == XUI_OK && xuiDocumentAcquireSnapshot(d, &before) == XUI_OK &&
            xuiDocumentMarkSaved(d, before) == XUI_OK);
        alignment_execute(d, &f, mode); CHECK(xuiDocumentIsDirty(d));
        CHECK(xuiDocumentAcquireSnapshot(d, &after) == XUI_OK); alignment_snapshot(after, f.edited);
        memory = document_memory(d);
        /* A newly preceding duplicate becomes the visible footnote. Undo
         * necessarily owns the removed semantic body, but not a second cache
         * copy of that same body. Source payload remains shared in these edits. */
        CHECK(memory.iHistoryBytes < 65536 +
            ((used && operation >= 3 && operation <= 6) ? sizes[size] : 0));
#ifndef XUI_DLL
        CHECK(reference_sharing_cache_history(before, after) < 32768); memory_oracle_check(d);
#endif
        printf("Definition alignment body=%zu used=%u operation=%u mode=%u HistoryBytes=%llu\n",
            sizes[size], used, operation, mode, (unsigned long long)memory.iHistoryBytes);
        CHECK(xuiDocumentUndo(d, NULL) == XUI_OK && !xuiDocumentIsDirty(d) &&
            xuiDocumentAcquireSnapshot(d, &restored) == XUI_OK);
        inc_snapshot_equal(before, restored); xuiDocumentSnapshotRelease(restored);
        CHECK(xuiDocumentRedo(d, NULL) == XUI_OK && xuiDocumentIsDirty(d) &&
            xuiDocumentAcquireSnapshot(d, &restored) == XUI_OK);
        inc_snapshot_equal(after, restored); xuiDocumentSnapshotRelease(restored);
        CHECK(xuiDocumentClearHistory(d) == XUI_OK);
#ifndef XUI_DLL
        memory_oracle_check(d);
#endif
        xuiDocumentRelease(d);
        alignment_snapshot(before, f.source); alignment_snapshot(after, f.edited);
        memory.iSize = sizeof(memory);
        CHECK(xuiDocumentSnapshotGetMemoryStats(after, &memory) == XUI_OK && !memory.iCurrentBytes &&
            !memory.iHistoryBytes && !memory.iOtherBytes && memory.iLiveBytes == memory.iSnapshotBytes);
        xuiDocumentSnapshotRelease(before); xuiDocumentSnapshotRelease(after); alignment_free(&f); cases++;
    }
    printf("Definition alignment: %u public SOURCE/Prepare/queued and ready Continue cases, count changes, reorder, changed large duplicate, typed labels, title presence, full-source/tree/CST/ordered definitions, Undo/Redo and retained snapshots passed\n", cases);
}
static void alignment_continue_running(void)
{
    unsigned used;
    for (used = 0; used < 2; used++) {
        alignment_fixture f = alignment_make(131072, used, 4);
        prepare_gate_allocator a; xui_doc_desc_t desc = {0}; xui_document d; xui_document_prepare p, next;
        xthread* worker;
        atomic_init(&a.live, 0); atomic_init(&a.gate, 0); a.owner = xrtThreadCurrentId();
        atomic_init(&a.entered, 0); atomic_init(&a.resume, 0);
        desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN;
        desc.onAlloc = prepare_gate_alloc; desc.onFree = prepare_gate_free; desc.pAllocatorUser = &a;
        CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK && xuiDocumentLoadMarkdown(d, f.source, strlen(f.source)) == XUI_OK &&
            xuiDocumentClearHistory(d) == XUI_OK);
        CHECK(xuiDocumentPrepareSource(d, NULL, f.patches, 3, &p) == XUI_OK);
        atomic_store(&a.gate, 1); worker = prepare_start(p); prepare_gate_wait(&a.entered);
        CHECK(prepare_info(p).iState == XUI_DOC_PREPARE_RUNNING);
        CHECK(xuiDocumentPrepareContinueSource(d, p, f.patches + 3, f.count - 3, &next) == XUI_OK);
        atomic_store(&a.resume, 1); CHECK(prepare_finish(worker) == XUI_DOC_ERROR_STALE);
        xuiDocumentPrepareRelease(p);
        CHECK(xuiDocumentPrepareRun(next) == XUI_OK && xuiDocumentPreparePublish(d, next, NULL) == XUI_OK);
        xuiDocumentPrepareRelease(next); reference_sharing_compare(d, f.edited);
        CHECK(document_memory(d).iHistoryBytes < 65536 + (used ? 131072 : 0)); xuiDocumentRelease(d);
        CHECK(!atomic_load(&a.live)); alignment_free(&f);
    }
    puts("Definition alignment RUNNING Continue: immutable predecessor input, stale worker, duplicate reorder plus body change, full oracle and no leaks passed");
}
static void alignment_faults(void)
{
    static const unsigned operations[] = {0, 4, 6, 7};
    unsigned cancel, sample; long totals[2] = {0};
    for (cancel = 0; cancel < 2; cancel++) for (sample = 0; sample < 4; sample++) {
        alignment_fixture f = alignment_make(4096, sample & 1, operations[sample]);
        long point; int success = 0;
        for (point = 0; point < 6000 && !success; point++) {
            fail_allocator failing = {-1, 0}; prepare_cancel_allocator cancelling = {0, -1, 0, NULL};
            xui_doc_desc_t desc = {0}; xui_document d; xui_document_prepare p = NULL;
            xui_document_snapshot before, after; uint64_t revision; int result;
            desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN;
            desc.onAlloc = cancel ? prepare_cancel_alloc : failing_alloc;
            desc.onFree = cancel ? prepare_cancel_free : failing_free;
            desc.pAllocatorUser = cancel ? (void*)&cancelling : (void*)&failing;
            CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK &&
                xuiDocumentLoadMarkdown(d, f.source, strlen(f.source)) == XUI_OK &&
                xuiDocumentClearHistory(d) == XUI_OK && xuiDocumentAcquireSnapshot(d, &before) == XUI_OK &&
                xuiDocumentMarkSaved(d, before) == XUI_OK);
            revision = xuiDocumentGetRevision(d); if (!cancel) failing.remaining = point;
            result = xuiDocumentPrepareSource(d, NULL, f.patches, f.count, &p);
            if (cancel) { cancelling.prepare = p; cancelling.remaining = point; }
            if (result == XUI_OK) result = xuiDocumentPrepareRun(p);
            cancelling.prepare = NULL; cancelling.remaining = failing.remaining = -1;
            if (result == XUI_OK) result = xuiDocumentPreparePublish(d, p, NULL);
            CHECK(xuiDocumentAcquireSnapshot(d, &after) == XUI_OK);
            if (result != XUI_OK) {
                CHECK(result == (cancel ? XUI_DOC_ERROR_CANCELLED : XUI_ERROR_OUT_OF_MEMORY) &&
                    xuiDocumentGetRevision(d) == revision && !xuiDocumentCanUndo(d) && !xuiDocumentIsDirty(d));
                expect_identity_tree(before, after, XUI_DOCUMENT_ROOT); expect_same_syntax(before, after);
                inc_snapshot_equal(before, after); test_source(d, f.source, 1);
            } else {
                success = 1; alignment_snapshot(after, f.edited);
                CHECK(xuiDocumentUndo(d, NULL) == XUI_OK && !xuiDocumentIsDirty(d));
            }
            xuiDocumentSnapshotRelease(before); xuiDocumentSnapshotRelease(after); xuiDocumentPrepareRelease(p);
            CHECK(xuiDocumentCancelPrepare(d) == XUI_OK);
#ifndef XUI_DLL
            memory_oracle_check(d);
#endif
            xuiDocumentRelease(d); CHECK(!failing.live && !cancelling.live);
        }
        CHECK(success); totals[cancel] += point - 1;
        printf("Definition alignment Prepare %s operation=%u: %ld atomic and leak-free fault checkpoints\n",
            cancel ? "cancel" : "OOM", operations[sample], point - 1);
        alignment_free(&f);
    }
    printf("Definition alignment faults: %ld OOM and %ld cancellation checkpoints passed\n", totals[0], totals[1]);
}
static void alignment_empty_middle(void)
{
    static const char* sources[] = {"text\n\n[r]: /keep\n", "text\n\n[^a]: keep\n\n[^b]: other\n"};
    unsigned i, mode;
    for (i = 0; i < 2; i++) for (mode = 0; mode < 2; mode++) {
        alignment_fixture f = {0}; xui_document d; xui_document_snapshot snapshot;
        f.capacity = 256; f.source = malloc(256); f.edited = malloc(256); CHECK(f.source && f.edited);
        strcpy(f.source, sources[i]); strcpy(f.edited, f.source); d = test_markdown_open(f.source);
        CHECK(xuiDocumentClearHistory(d) == XUI_OK);
        alignment_patch(&f, 6, 6, "[^new]: inserted\n\n");
        alignment_execute(d, &f, mode); CHECK(xuiDocumentAcquireSnapshot(d, &snapshot) == XUI_OK);
        alignment_snapshot(snapshot, f.edited); xuiDocumentSnapshotRelease(snapshot);
        alignment_free(&f); f = (alignment_fixture){0};
        f.capacity = 256; f.source = malloc(256); f.edited = malloc(256); CHECK(f.source && f.edited);
        strcpy(f.source, "text\n\n[^new]: inserted\n\n"); strcat(f.source, sources[i] + 6); strcpy(f.edited, f.source);
        alignment_replace(&f, "[^new]: inserted\n\n", ""); alignment_execute(d, &f, mode);
        CHECK(xuiDocumentAcquireSnapshot(d, &snapshot) == XUI_OK); alignment_snapshot(snapshot, sources[i]);
        xuiDocumentSnapshotRelease(snapshot); alignment_free(&f); xuiDocumentRelease(d);
    }
    puts("Definition alignment empty middle: insert/delete definition runs preserve exact surrounding values passed");
}
static xui_document alignment_reload(xui_document d)
{
    xui_document_snapshot s; xui_document loaded; char* wire; uint64_t bytes;
    CHECK(xuiDocumentAcquireSnapshot(d, &s) == XUI_OK && xuiDocumentSerialize(s, &wire, &bytes) == XUI_OK &&
        xuiDocumentDeserialize(NULL, wire, bytes, &loaded) == XUI_OK);
    xuiDocumentFreeBuffer(wire); xuiDocumentSnapshotRelease(s); xuiDocumentRelease(d); return loaded;
}
static void alignment_duplicate_groups(void)
{
    unsigned reload, mode, i;
    for (reload = 0; reload < 2; reload++) for (mode = 1; mode <= 3; mode += 2) {
        alignment_fixture f = {0}; xui_document d; xui_document_snapshot before, after;
        f.capacity = 20000; f.source = malloc(f.capacity); f.edited = malloc(f.capacity);
        CHECK(f.source && f.edited); strcpy(f.source, "begin\n\n[^start]: first\n\n");
        for (i = 0; i < 128; i++) strcat(f.source, i & 1 ?
            "[same]: /shared 'same title'\n\n" : "[^same]: shared foot value\n\n");
        strcat(f.source, "[^end]: last\n\n"); strcpy(f.edited, f.source);
        alignment_replace(&f, "[^start]: first", "[^start]: FIRST");
        alignment_replace(&f, "[^end]: last", "[^end]: LAST");
        alignment_patch(&f, 0, 0, "[same]: /shared 'same title'\n\n[^same]: shared foot value\n\n");
        d = test_markdown_open(f.source); if (reload) d = alignment_reload(d);
        CHECK(xuiDocumentClearHistory(d) == XUI_OK && xuiDocumentAcquireSnapshot(d, &before) == XUI_OK);
        alignment_execute(d, &f, mode); CHECK(xuiDocumentAcquireSnapshot(d, &after) == XUI_OK);
        alignment_snapshot(after, f.edited); CHECK(document_memory(d).iHistoryBytes < 65536);
#ifndef XUI_DLL
        CHECK(reference_sharing_cache_history(before, after) < 32768); memory_oracle_check(d);
#endif
        CHECK(xuiDocumentUndo(d, NULL) == XUI_OK); reference_sharing_compare(d, f.source);
        CHECK(xuiDocumentRedo(d, NULL) == XUI_OK); reference_sharing_compare(d, f.edited);
        xuiDocumentRelease(d); alignment_snapshot(before, f.source); alignment_snapshot(after, f.edited);
        xuiDocumentSnapshotRelease(before); xuiDocumentSnapshotRelease(after); alignment_free(&f);
    }
    for (reload = 0; reload < 2; reload++) {
        alignment_fixture f = alignment_make(1048576, reload, reload ? 0 : 4);
        xui_document d = alignment_reload(test_markdown_open(f.source));
        xui_document_snapshot before, after;
        CHECK(xuiDocumentClearHistory(d) == XUI_OK && xuiDocumentAcquireSnapshot(d, &before) == XUI_OK);
        alignment_execute(d, &f, 1); CHECK(xuiDocumentAcquireSnapshot(d, &after) == XUI_OK);
        alignment_snapshot(after, f.edited); CHECK(document_memory(d).iHistoryBytes < 65536);
#ifndef XUI_DLL
        CHECK(reference_sharing_cache_history(before, after) < 32768); memory_oracle_check(d);
#endif
        CHECK(xuiDocumentUndo(d, NULL) == XUI_OK); reference_sharing_compare(d, f.source);
        CHECK(xuiDocumentRedo(d, NULL) == XUI_OK); reference_sharing_compare(d, f.edited);
        xuiDocumentRelease(d); alignment_snapshot(before, f.source); alignment_snapshot(after, f.edited);
        xuiDocumentSnapshotRelease(before); xuiDocumentSnapshotRelease(after); alignment_free(&f);
    }
    puts("Definition alignment duplicate groups: 128 interleaved equal link/footnote values, typed duplicate runs, ready Continue, four group cases and two 1MiB native reload edits passed");
}
static alignment_fixture alignment_swap_make(size_t n, unsigned used)
{
    const char* prefix = used ? "use [^n] and [r]\n\n[^a]: first\n\n[^n]: " :
        "use [r]\n\n[^a]: first\n\n[^n]: ";
    const char* between = "\n\n[^n]: ", *suffix = "\n\n[^b]: last\n\n[r]: /old\n";
    alignment_fixture f = {0}; size_t i, first = strlen(prefix), second = first + n + strlen(between);
    char *a = malloc(n + 1), *b = malloc(n + 1); uint32_t random = 778;
    CHECK(a && b); f.capacity = n * 2 + 2048; f.body = n;
    f.source = malloc(f.capacity); f.edited = malloc(f.capacity); CHECK(f.source && f.edited);
    for (i = 0; i < n; i++) {
        random = random * 1664525u + 1013904223u; a[i] = (char)('a' + (random >> 16) % 26);
        random = random * 1664525u + 1013904223u; b[i] = (char)('a' + (random >> 16) % 26);
    }
    a[n] = b[n] = 0; strcpy(f.source, prefix); strcat(f.source, a);
    strcat(f.source, between); strcat(f.source, b); strcat(f.source, suffix); strcpy(f.edited, f.source);
    alignment_replace(&f, "[^a]: first", "[^a]: FIRST"); alignment_replace(&f, "[^b]: last", "[^b]: LAST");
    a[n / 2] = 'X'; b[n / 2] = 'Y';
    alignment_patch(&f, first, first + n, b); alignment_patch(&f, second, second + n, a);
    alignment_patch(&f, 0, 0, "[^extra]: inserted\n\n"); free(a); free(b); return f;
}
static void alignment_equal_size_swaps(void)
{
    static const size_t sizes[] = {4096, 131072, 1048576}; unsigned size, used, mode, cases = 0;
    for (size = 0; size < 3; size++) for (used = 0; used < 2; used++) for (mode = 0; mode < 4; mode++) {
        alignment_fixture f = alignment_swap_make(sizes[size], used);
        xui_document d = test_markdown_open(f.source); xui_document_snapshot before, after, restored;
        CHECK(xuiDocumentClearHistory(d) == XUI_OK && xuiDocumentAcquireSnapshot(d, &before) == XUI_OK &&
            xuiDocumentMarkSaved(d, before) == XUI_OK);
        alignment_execute(d, &f, mode); CHECK(xuiDocumentAcquireSnapshot(d, &after) == XUI_OK);
        alignment_snapshot(after, f.edited);
        CHECK(document_memory(d).iHistoryBytes < 65536 + (used ? sizes[size] : 0));
#ifndef XUI_DLL
        CHECK(reference_sharing_cache_history(before, after) < 32768); memory_oracle_check(d);
        printf("Definition equal-size swapped bodies bytes=%zu used=%u mode=%u cache-exclusive=%llu\n", sizes[size],
            used, mode, (unsigned long long)reference_sharing_cache_history(before, after));
#endif
        CHECK(xuiDocumentUndo(d, NULL) == XUI_OK && !xuiDocumentIsDirty(d) &&
            xuiDocumentAcquireSnapshot(d, &restored) == XUI_OK);
        inc_snapshot_equal(before, restored); xuiDocumentSnapshotRelease(restored);
        CHECK(xuiDocumentRedo(d, NULL) == XUI_OK && xuiDocumentAcquireSnapshot(d, &restored) == XUI_OK);
        inc_snapshot_equal(after, restored); xuiDocumentSnapshotRelease(restored);
        xuiDocumentRelease(d); alignment_snapshot(before, f.source); alignment_snapshot(after, f.edited);
        xuiDocumentSnapshotRelease(before); xuiDocumentSnapshotRelease(after); alignment_free(&f); cases++;
    }
    printf("Definition equal-size swaps: %u cases, both distinct bodies edited/reordered, added definition, immutable transaction origin, full oracle and Undo/Redo passed\n", cases);
}
typedef struct alignment_sync_allocator {
    fail_allocator allocator;
    long remaining;
    atomic_int cancellation;
} alignment_sync_allocator;
static void* alignment_sync_alloc(void* user, size_t bytes)
{
    alignment_sync_allocator* a = user;
    if (a->remaining >= 0) {
        if (!a->remaining) atomic_store(&a->cancellation, XUI_DOC_ERROR_CANCELLED);
        else a->remaining--;
    }
    return failing_alloc(&a->allocator, bytes);
}
static void alignment_sync_free(void* user, void* pointer)
{ alignment_sync_allocator* a = user; failing_free(&a->allocator, pointer); }
static void alignment_swap_faults(void)
{
    unsigned cancel, used; long totals[2] = {0};
    for (cancel = 0; cancel < 2; cancel++) for (used = 0; used < 2; used++) {
#ifdef XUI_DLL
        if (cancel) continue; /* Synchronous SOURCE has no public cancellation token. */
#endif
        alignment_fixture f = alignment_swap_make(4096, used); long point; int success = 0;
        for (point = 0; point < 6000 && !success; point++) {
            alignment_sync_allocator allocator = {{-1, 0}, -1, ATOMIC_VAR_INIT(0)};
            xui_doc_desc_t desc = {0}; xui_doc_txn_desc_t td = {0}; xui_document d; xui_document_transaction t = NULL;
            xui_document_snapshot before, after; uint64_t revision; unsigned i; int result;
            desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN;
            desc.onAlloc = alignment_sync_alloc; desc.onFree = alignment_sync_free; desc.pAllocatorUser = &allocator;
            CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK && xuiDocumentLoadMarkdown(d, f.source, strlen(f.source)) == XUI_OK &&
                xuiDocumentClearHistory(d) == XUI_OK && xuiDocumentAcquireSnapshot(d, &before) == XUI_OK &&
                xuiDocumentMarkSaved(d, before) == XUI_OK);
            revision = xuiDocumentGetRevision(d); td.iSize = sizeof(td); td.iDomain = XUI_DOC_SOURCE;
            if (!cancel) allocator.allocator.remaining = point;
            result = xuiDocumentBeginTransaction(d, &td, &t);
#ifndef XUI_DLL
            if (cancel && result == XUI_OK) { t->cancellation = &allocator.cancellation; allocator.remaining = point; }
#endif
            for (i = 0; i < f.count && result == XUI_OK; i++) result = xuiDocumentTxnReplaceSource(t,
                f.patches[i].iStart, f.patches[i].iEnd, f.patches[i].sText, f.patches[i].iTextBytes);
            if (result == XUI_OK) result = xuiDocumentTxnCommit(t, NULL);
            allocator.remaining = allocator.allocator.remaining = -1; atomic_store(&allocator.cancellation, 0);
            xuiDocumentTxnRelease(t); CHECK(xuiDocumentAcquireSnapshot(d, &after) == XUI_OK);
            if (result == XUI_OK) {
                success = 1; alignment_snapshot(after, f.edited);
                CHECK(xuiDocumentUndo(d, NULL) == XUI_OK && !xuiDocumentIsDirty(d));
            } else {
                CHECK(result == (cancel ? XUI_DOC_ERROR_CANCELLED : XUI_ERROR_OUT_OF_MEMORY) &&
                    xuiDocumentGetRevision(d) == revision && !xuiDocumentCanUndo(d) && !xuiDocumentIsDirty(d));
                expect_identity_tree(before, after, XUI_DOCUMENT_ROOT); expect_same_syntax(before, after);
                inc_snapshot_equal(before, after); test_source(d, f.source, 1);
            }
#ifndef XUI_DLL
            memory_oracle_check(d);
#endif
            xuiDocumentSnapshotRelease(before); xuiDocumentSnapshotRelease(after); xuiDocumentRelease(d);
            CHECK(!allocator.allocator.live);
        }
        CHECK(success); totals[cancel] += point - 1; alignment_free(&f);
    }
    printf("Definition swapped-body SOURCE faults: %ld OOM and %ld private cancellation checkpoints atomic and leak-free\n", totals[0], totals[1]);
}
int main(void)
{
#ifdef XUI_DOC_REFERENCE_HASH_COLLISION_TEST
    puts("Definition alignment collision build: every full-value and label hash forced to zero; complete typed byte equality remains required");
#endif
    alignment_matrix(); alignment_continue_running(); alignment_faults(); alignment_empty_middle();
    alignment_duplicate_groups();
    alignment_equal_size_swaps(); alignment_swap_faults();
    return 0;
}
