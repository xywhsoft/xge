/* Included by both the standalone and actual-DLL core test builds. */
#include <stdatomic.h>

static xui_doc_source_patch_t prepare_patch(uint64_t start, uint64_t end, const char* text)
{
    xui_doc_source_patch_t p = {0}; p.iSize = sizeof(p); p.iStart = start; p.iEnd = end;
    p.sText = text; p.iTextBytes = strlen(text); return p;
}
static xui_doc_prepare_info_t prepare_info(xui_document_prepare p)
{
    xui_doc_prepare_info_t info = {0}; info.iSize = sizeof(info);
    CHECK(xuiDocumentPrepareGetInfo(p, &info) == XUI_OK); return info;
}
static void prepare_source(xui_document_prepare p, const char* expected)
{
    uint64_t n; char buffer[4096];
    CHECK(xuiDocumentPrepareCopySource(p, NULL, 0, &n) == XUI_OK && n == strlen(expected));
    CHECK(xuiDocumentPrepareCopySource(p, buffer, n, &n) == XUI_ERROR_BUFFER_TOO_SMALL);
    CHECK(xuiDocumentPrepareCopySource(p, buffer, sizeof(buffer), &n) == XUI_OK && !strcmp(buffer, expected));
}
static int32 prepare_worker(void* data)
{
    int result = xuiDocumentPrepareRun(data); xuiDocumentPrepareRelease(data); return result;
}
static xthread* prepare_start(xui_document_prepare p)
{
    xthread* thread; xuiDocumentPrepareRetain(p); thread = xrtThreadCreate(prepare_worker, p, 0); CHECK(thread); return thread;
}
static int prepare_finish(xthread* thread)
{
    int result; CHECK(xrtThreadWaitFor(thread, 10000000) == XWAIT_OK);
    result = xrtThreadExitCode(thread); xrtThreadDestroy(thread); return result;
}
typedef struct prepare_notice { uint64_t owner; unsigned calls; int drop_document; } prepare_notice;
static void prepare_changed(xui_document d, xui_document_change_set change, void* data)
{
    prepare_notice* n = data; xui_document_prepare p = NULL;
    CHECK(xrtThreadCurrentId() == n->owner); CHECK(change); n->calls++;
    CHECK(xuiDocumentPrepareSource(d, NULL, NULL, 0, &p) == XUI_DOC_ERROR_BUSY && !p);
    CHECK(xuiDocumentCancelPrepare(d) == XUI_DOC_ERROR_BUSY);
    CHECK(!xuiDocumentHasPrepare(d));
    if (n->drop_document) xuiDocumentRelease(d);
}
static void prepare_contract(void)
{
    const char* initial = "alpha &amp; **beta**\n\nkeep\n";
    const char* expected = "alpha! &amp; **gamma**\n\nkeep\n";
    xui_document d = test_markdown_open(initial), other;
    xui_document_prepare p; xui_document_transaction t; xui_document_snapshot saved, s;
    xui_document_change_set change = NULL; xui_doc_change_info_t ci = {0};
    xui_doc_txn_desc_t desc = {0}; xui_doc_source_patch_t patches[2];
    xui_doc_stats_t before = {0}, after = {0}; xui_doc_prepare_info_t info;
    prepare_notice notice = {xrtThreadCurrentId(), 0, 0}; uint64_t token, revision, kept;
    xui_doc_position_t at, mapped; int mapping; xui_doc_range_t keep = test_find(d, "keep");
    kept = keep.tAnchor.iNodeId; revision = xuiDocumentGetRevision(d);
    before.iSize = after.iSize = sizeof(before); ci.iSize = sizeof(ci);
    CHECK(xuiDocumentAcquireSnapshot(d, &saved) == XUI_OK && xuiDocumentMarkSaved(d, saved) == XUI_OK);
    CHECK(xuiDocumentSubscribe(d, prepare_changed, &notice, &token) == XUI_OK);
    desc.iSize = sizeof(desc); desc.iDomain = XUI_DOC_SOURCE; desc.iBaseRevision = revision; desc.iOrigin = 57; desc.iGroup = 123;
    patches[0] = prepare_patch(5, 5, "!");
    patches[1] = prepare_patch((uint64_t)(strstr(initial, "beta") - initial) + 1, (uint64_t)(strstr(initial, "beta") - initial) + 5, "gamma");
    CHECK(xuiDocumentGetStats(d, &before) == XUI_OK);
    CHECK(xuiDocumentPrepareSource(d, &desc, patches, 2, &p) == XUI_OK && xuiDocumentHasPrepare(d));
    prepare_source(p, expected); test_source(d, initial, 1); CHECK(!notice.calls && !xuiDocumentIsDirty(d));
    CHECK(xuiDocumentGetStats(d, &after) == XUI_OK && after.iMarkdownParses == before.iMarkdownParses);
    info = prepare_info(p); CHECK(info.iState == XUI_DOC_PREPARE_QUEUED && info.iResult == XUI_DOC_ERROR_BUSY);
    CHECK(info.iPatchCount == 2 && info.iGeneration && info.iDocumentId == xuiDocumentGetIdentity(d) && info.iBaseRevision == revision);
    CHECK(xuiDocumentPreparePublish(d, p, &change) == XUI_DOC_ERROR_BUSY && !change);
    CHECK(xuiDocumentSaveFile(d, "build/document/should-not-be-written.md", XUI_DOC_FILE_MARKDOWN) == XUI_DOC_ERROR_BUSY);
    CHECK(prepare_finish(prepare_start(p)) == XUI_OK); CHECK(!notice.calls);
    CHECK(xuiDocumentGetStats(d, &after) == XUI_OK && after.iMarkdownParses == before.iMarkdownParses + 1);
    CHECK(after.iCurrentBytes == before.iCurrentBytes && after.iHistoryBytes == before.iHistoryBytes);
    CHECK(xuiDocumentPrepareRun(p) == XUI_ERROR_INVALID_STATE);
    info = prepare_info(p); CHECK(info.iState == XUI_DOC_PREPARE_READY && info.iResult == XUI_OK && !info.iCommittedRevision);
    CHECK(xuiDocumentBeginTransaction(d, NULL, &t) == XUI_OK);
    CHECK(xuiDocumentPreparePublish(d, p, NULL) == XUI_DOC_ERROR_BUSY); xuiDocumentTxnRelease(t);
    CHECK(xuiDocumentCreate(NULL, &other) == XUI_OK);
    CHECK(xuiDocumentPreparePublish(other, p, NULL) == XUI_DOC_ERROR_STALE); xuiDocumentRelease(other);
    at = test_position(d, XUI_DOCUMENT_ROOT, XUI_DOC_POSITION_SOURCE, (uint64_t)(strstr(initial, "keep") - initial), XUI_DOC_AFTER);
    CHECK(xuiDocumentPreparePublish(d, p, &change) == XUI_OK && change && notice.calls == 1 && !xuiDocumentHasPrepare(d));
    CHECK(xuiDocumentGetRevision(d) == revision + 1 && xuiDocumentIsDirty(d)); test_source(d, expected, 1);
    CHECK(xuiDocumentChangeSetGetInfo(change, &ci) == XUI_OK && ci.iDomain == XUI_DOC_SOURCE && ci.iOrigin == 57 && ci.iOperationCount == 2);
    CHECK(xuiDocumentMapPosition(change, &at, &mapped, &mapping) == XUI_OK && mapping == XUI_DOC_MAP_EXACT && mapped.iOffset == at.iOffset + 2);
    xuiDocumentChangeSetRelease(change);
    CHECK(xuiDocumentAcquireSnapshot(d, &s) == XUI_OK); expect_text(s, kept, "keep"); xuiDocumentSnapshotRelease(s);
    info = prepare_info(p); CHECK(info.iState == XUI_DOC_PREPARE_COMMITTED && info.iResult == XUI_OK && info.iCommittedRevision == revision + 1);
    CHECK(xuiDocumentPreparePublish(d, p, NULL) == XUI_ERROR_INVALID_STATE); xuiDocumentPrepareRelease(p);
    CHECK(xuiDocumentUndo(d, NULL) == XUI_OK && !xuiDocumentIsDirty(d)); test_source(d, initial, 1);
    CHECK(xuiDocumentRedo(d, NULL) == XUI_OK); test_source(d, expected, 1);
    CHECK(notice.calls == 3); xuiDocumentSnapshotRelease(saved);
    revision = xuiDocumentGetRevision(d);
    CHECK(xuiDocumentPrepareSource(d, NULL, NULL, 0, &p) == XUI_OK);
    CHECK(xuiDocumentPrepareRun(p) == XUI_OK && xuiDocumentPreparePublish(d, p, NULL) == XUI_OK);
    CHECK(xuiDocumentGetRevision(d) == revision && notice.calls == 3 && !xuiDocumentHasPrepare(d));
    xuiDocumentPrepareRelease(p); xuiDocumentUnsubscribe(d, token); xuiDocumentRelease(d);
    /* A notification may release the caller's live Document reference. */
    d = test_markdown_open("old"); patches[0] = prepare_patch(0, 3, "new"); notice.calls = 0; notice.drop_document = 1;
    CHECK(xuiDocumentSubscribe(d, prepare_changed, &notice, &token) == XUI_OK);
    CHECK(xuiDocumentPrepareSource(d, NULL, patches, 1, &p) == XUI_OK && xuiDocumentPrepareRun(p) == XUI_OK);
    CHECK(xuiDocumentPreparePublish(d, p, NULL) == XUI_OK && notice.calls == 1); /* d is released here. */
    prepare_source(p, "new"); xuiDocumentPrepareRelease(p);
    puts("Document prepare: detached worker, source projection, one publish/notification/history, mappings and callback lifetime passed");
}
static void prepare_invalidation(void)
{
    xui_document d = test_markdown_open("old"), rich_doc; xui_document_prepare a = NULL, b = NULL, bad = NULL;
    xui_doc_source_patch_t patch = prepare_patch(0, 3, "first"), invalid = prepare_patch(0, 0, "\xc0");
    xui_doc_txn_desc_t desc = {0}; xui_document_transaction t; uint64_t revision = xuiDocumentGetRevision(d), gen;
    CHECK(xuiDocumentCreate(NULL, &rich_doc) == XUI_OK);
    CHECK(xuiDocumentPrepareSource(rich_doc, NULL, &patch, 1, &a) == XUI_ERROR_UNSUPPORTED && !a); xuiDocumentRelease(rich_doc);
    CHECK(xuiDocumentPrepareSource(d, NULL, &patch, 1, &a) == XUI_OK);
    xuiDocumentPrepareCancel(a);
    CHECK(xuiDocumentPreparePublish(d, a, NULL) == XUI_DOC_ERROR_CANCELLED && !xuiDocumentHasPrepare(d));
    CHECK(xuiDocumentPrepareRun(a) == XUI_DOC_ERROR_CANCELLED);
    CHECK(xuiDocumentPreparePublish(d, a, NULL) == XUI_DOC_ERROR_CANCELLED && !xuiDocumentHasPrepare(d)); xuiDocumentPrepareRelease(a);
    CHECK(xuiDocumentPrepareSource(d, NULL, &patch, 1, &a) == XUI_OK && xuiDocumentPrepareRun(a) == XUI_OK);
    xuiDocumentPrepareCancel(a); CHECK(xuiDocumentPreparePublish(d, a, NULL) == XUI_DOC_ERROR_CANCELLED); xuiDocumentPrepareRelease(a);
    CHECK(xuiDocumentPrepareSource(d, NULL, &patch, 1, &a) == XUI_OK && xuiDocumentPrepareRun(a) == XUI_OK); gen = prepare_info(a).iGeneration;
    patch = prepare_patch(0, 3, "second");
    CHECK(xuiDocumentPrepareSource(d, NULL, &patch, 1, &b) == XUI_OK && prepare_info(b).iGeneration > gen);
    CHECK(xuiDocumentPreparePublish(d, a, NULL) == XUI_DOC_ERROR_STALE); xuiDocumentPrepareRelease(a);
    CHECK(xuiDocumentPrepareSource(d, NULL, &invalid, 1, &bad) == XUI_DOC_ERROR_UTF8 && !bad);
    CHECK(xuiDocumentHasPrepare(d) && !prepare_info(b).bCancellationRequested);
    CHECK(xuiDocumentPrepareRun(b) == XUI_OK && xuiDocumentPreparePublish(d, b, NULL) == XUI_OK); xuiDocumentPrepareRelease(b);
    CHECK(xuiDocumentGetRevision(d) == revision + 1); test_source(d, "second", 1);
    patch = prepare_patch(0, 6, "third");
    CHECK(xuiDocumentPrepareSource(d, NULL, &patch, 1, &a) == XUI_OK);
    CHECK(xuiDocumentLoadMarkdown(d, "external", 8) == XUI_OK && !xuiDocumentHasPrepare(d));
    CHECK(xuiDocumentPrepareRun(a) == XUI_DOC_ERROR_STALE); xuiDocumentPrepareRelease(a); test_source(d, "external", 1);
    patch = prepare_patch(0, 8, "aborted");
    CHECK(xuiDocumentPrepareSource(d, NULL, &patch, 1, &a) == XUI_OK && xuiDocumentPrepareRun(a) == XUI_OK);
    CHECK(xuiDocumentUndo(d, NULL) == XUI_OK && !xuiDocumentHasPrepare(d));
    CHECK(xuiDocumentPreparePublish(d, a, NULL) == XUI_DOC_ERROR_STALE); xuiDocumentPrepareRelease(a);
    patch = prepare_patch(0, 6, "aborted");
    CHECK(xuiDocumentPrepareSource(d, NULL, &patch, 1, &a) == XUI_OK);
    CHECK(xuiDocumentRedo(d, NULL) == XUI_OK && xuiDocumentPrepareRun(a) == XUI_DOC_ERROR_STALE); xuiDocumentPrepareRelease(a);
    patch = prepare_patch(0, 8, "kept");
    CHECK(xuiDocumentPrepareSource(d, NULL, &patch, 1, &a) == XUI_OK);
    desc.iSize = sizeof(desc); desc.iDomain = XUI_DOC_SOURCE;
    CHECK(xuiDocumentBeginTransaction(d, &desc, &t) == XUI_OK);
    CHECK(xuiDocumentTxnReplaceSource(t, 9, 9, "x", 1) == XUI_ERROR_INVALID_ARGUMENT);
    CHECK(xuiDocumentTxnCommit(t, NULL) == XUI_ERROR_INVALID_ARGUMENT); xuiDocumentTxnRelease(t);
    CHECK(xuiDocumentHasPrepare(d) && xuiDocumentPrepareRun(a) == XUI_OK && xuiDocumentPreparePublish(d, a, NULL) == XUI_OK);
    xuiDocumentPrepareRelease(a); test_source(d, "kept", 1);
    patch = prepare_patch(0, 4, "cancelled");
    CHECK(xuiDocumentPrepareSource(d, NULL, &patch, 1, &a) == XUI_OK && xuiDocumentCancelPrepare(d) == XUI_OK);
    CHECK(!xuiDocumentHasPrepare(d) && xuiDocumentPrepareRun(a) == XUI_DOC_ERROR_CANCELLED); xuiDocumentPrepareRelease(a);
    /* Two prepared commits with an explicit group share the normal Undo. */
    CHECK(xuiDocumentClearHistory(d) == XUI_OK); desc.iOrigin = 9; desc.iGroup = 11;
    patch = prepare_patch(0, 4, "one");
    CHECK(xuiDocumentPrepareSource(d, &desc, &patch, 1, &a) == XUI_OK && xuiDocumentPrepareRun(a) == XUI_OK && xuiDocumentPreparePublish(d, a, NULL) == XUI_OK);
    xuiDocumentPrepareRelease(a); patch = prepare_patch(0, 3, "two");
    CHECK(xuiDocumentPrepareSource(d, &desc, &patch, 1, &a) == XUI_OK && xuiDocumentPrepareRun(a) == XUI_OK && xuiDocumentPreparePublish(d, a, NULL) == XUI_OK);
    xuiDocumentPrepareRelease(a); CHECK(xuiDocumentUndo(d, NULL) == XUI_OK && !xuiDocumentCanUndo(d)); test_source(d, "kept", 1);
    xuiDocumentRelease(d);
    puts("Document prepare invalidation: cancellation, supersession, failed creation/edit, external writes, Undo/Redo and grouping passed");
}
static void prepare_failures(void)
{
    long budget; int success = 0;
    for (budget = 0; budget < 1600 && !success; budget++) {
        fail_allocator allocator = {-1, 0}; xui_doc_desc_t desc = {0}; xui_document d;
        xui_document_prepare p = NULL, previous; xui_document_snapshot before, after;
        xui_doc_source_patch_t patch = prepare_patch(0, 3, "# new &amp; **value**\n\n[label]: /target\n\n[label]\n");
        xui_doc_stats_t stats = {0}, current = {0}; uint64_t revision, token; unsigned notice = notifications; int result;
        desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN;
        desc.onAlloc = failing_alloc; desc.onFree = failing_free; desc.pAllocatorUser = &allocator;
        CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK && xuiDocumentLoadMarkdown(d, "old", 3) == XUI_OK);
        CHECK(xuiDocumentSubscribe(d, changed, NULL, &token) == XUI_OK);
        CHECK(xuiDocumentAcquireSnapshot(d, &before) == XUI_OK && xuiDocumentMarkSaved(d, before) == XUI_OK);
        CHECK(xuiDocumentPrepareSource(d, NULL, NULL, 0, &previous) == XUI_OK);
        revision = xuiDocumentGetRevision(d); stats.iSize = current.iSize = sizeof(stats); CHECK(xuiDocumentGetStats(d, &stats) == XUI_OK);
        allocator.remaining = budget;
        result = xuiDocumentPrepareSource(d, NULL, &patch, 1, &p);
        if (!p) CHECK(!prepare_info(previous).bCancellationRequested && xuiDocumentHasPrepare(d));
        if (result == XUI_OK) result = xuiDocumentPrepareRun(p);
        if (result == XUI_OK) result = xuiDocumentPreparePublish(d, p, NULL);
        allocator.remaining = -1;
        CHECK(xuiDocumentAcquireSnapshot(d, &after) == XUI_OK);
        if (result != XUI_OK) {
            CHECK(result == XUI_ERROR_OUT_OF_MEMORY && notifications == notice && xuiDocumentGetRevision(d) == revision && !xuiDocumentIsDirty(d));
            CHECK(xuiDocumentGetStats(d, &current) == XUI_OK && current.iUndoCount == stats.iUndoCount && current.iRedoCount == stats.iRedoCount);
            expect_identity_tree(before, after, 1); test_source(d, "old", 1);
        } else {
            success = 1; CHECK(notifications == notice + 1); test_source(d, patch.sText, 1);
            CHECK(xuiDocumentUndo(d, NULL) == XUI_OK && !xuiDocumentIsDirty(d)); test_source(d, "old", 1);
        }
        xuiDocumentSnapshotRelease(before); xuiDocumentSnapshotRelease(after);
        xuiDocumentPrepareRelease(previous); xuiDocumentPrepareRelease(p); xuiDocumentRelease(d); CHECK(!allocator.live);
    }
    CHECK(success); printf("Document prepare allocation-failure sweep: %ld points passed\n", budget);
}
typedef struct prepare_gate_allocator {
    atomic_uint live;
    atomic_int gate;
    uint64_t owner;
    atomic_int entered, resume;
} prepare_gate_allocator;
static void prepare_gate_wait(atomic_int* gate)
{
    xdeadline deadline = xrtDeadlineAfter(10000000);
    while (!atomic_load(gate)) { CHECK(!xrtDeadlineExpired(deadline)); xrtThreadYield(); }
}
static void* prepare_gate_alloc(void* data, size_t bytes)
{
    prepare_gate_allocator* a = data; void* p;
    if (xrtThreadCurrentId() != a->owner && atomic_exchange(&a->gate, 0)) {
        atomic_store(&a->entered, 1); prepare_gate_wait(&a->resume);
    }
    p = malloc(bytes); if (p) atomic_fetch_add(&a->live, 1); return p;
}
static void prepare_gate_free(void* data, void* p)
{
    prepare_gate_allocator* a = data; CHECK(atomic_fetch_sub(&a->live, 1) > 0); free(p);
}
static void prepare_concurrent_lifetime(void)
{
    unsigned mode;
    for (mode = 0; mode < 6; mode++) {
        prepare_gate_allocator a; xui_doc_desc_t desc = {0}; xui_document d; xui_document_prepare p, replacement = NULL;
        xui_doc_source_patch_t patch = prepare_patch(0, 3, "new &amp; **body**"); xthread* thread;
        xui_doc_prepare_info_t info; xui_document_snapshot kept; xui_doc_memory_stats_t memory = {0};
        atomic_init(&a.live, 0); atomic_init(&a.gate, 0); a.owner = xrtThreadCurrentId();
        atomic_init(&a.entered, 0); atomic_init(&a.resume, 0);
        desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN;
        desc.onAlloc = prepare_gate_alloc; desc.onFree = prepare_gate_free; desc.pAllocatorUser = &a;
        CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK && xuiDocumentLoadMarkdown(d, "old", 3) == XUI_OK);
        CHECK(xuiDocumentAcquireSnapshot(d, &kept) == XUI_OK);
        CHECK(xuiDocumentPrepareSource(d, NULL, &patch, 1, &p) == XUI_OK);
        atomic_store(&a.gate, 1); thread = prepare_start(p); prepare_gate_wait(&a.entered);
        info = prepare_info(p); CHECK(info.iState == XUI_DOC_PREPARE_RUNNING && info.iResult == XUI_DOC_ERROR_BUSY);
        prepare_source(p, patch.sText); test_source(d, "old", 1);
        CHECK(xuiDocumentPreparePublish(d, p, NULL) == XUI_DOC_ERROR_BUSY);
        if (mode == 0) xuiDocumentPrepareCancel(p);
        else if (mode == 1) {
            patch = prepare_patch(0, 3, "newest"); CHECK(xuiDocumentPrepareSource(d, NULL, &patch, 1, &replacement) == XUI_OK);
            CHECK(xuiDocumentPrepareRun(replacement) == XUI_OK && xuiDocumentPreparePublish(d, replacement, NULL) == XUI_OK);
        } else if (mode == 2) CHECK(xuiDocumentLoadMarkdown(d, "external", 8) == XUI_OK);
        else if (mode == 3) { xuiDocumentRelease(d); d = NULL; }
        else if (mode == 4) CHECK(xuiDocumentClearHistory(d) == XUI_OK);
        else CHECK(xuiDocumentSetHistoryLimits(d, 1, 512) == XUI_OK);
        atomic_store(&a.resume, 1);
        CHECK(prepare_finish(thread) == (mode >= 4 ? XUI_OK : mode == 0 || mode == 3 ? XUI_DOC_ERROR_CANCELLED : XUI_DOC_ERROR_STALE));
        if (mode >= 4) { CHECK(xuiDocumentPreparePublish(d, p, NULL) == XUI_OK); test_source(d, patch.sText, 1); }
        else if (d) { CHECK(xuiDocumentCancelPrepare(d) == XUI_OK); test_source(d, mode == 1 ? "newest" : mode == 2 ? "external" : "old", 1); }
        xuiDocumentPrepareRelease(replacement); xuiDocumentPrepareRelease(p); xuiDocumentRelease(d);
        memory.iSize = sizeof(memory); CHECK(xuiDocumentSnapshotGetMemoryStats(kept, &memory) == XUI_OK && memory.iSnapshotBytes == memory.iLiveBytes && !memory.iOtherBytes);
        xuiDocumentSnapshotRelease(kept); CHECK(!atomic_load(&a.live));
    }
    puts("Document prepare concurrent lifetime: running cancellation, replacement, external commit, history changes and owner destruction passed");
}
typedef struct prepare_cancel_allocator {
    unsigned live; long remaining; int fired;
    xui_document_prepare prepare;
} prepare_cancel_allocator;
static void* prepare_cancel_alloc(void* data, size_t bytes)
{
    prepare_cancel_allocator* a = data; void* p;
    if (a->prepare && a->remaining >= 0 && !a->remaining--) { a->fired = 1; xuiDocumentPrepareCancel(a->prepare); }
    p = malloc(bytes); if (p) a->live++; return p;
}
static void prepare_cancel_free(void* data, void* p)
{
    prepare_cancel_allocator* a = data; CHECK(a->live); a->live--; free(p);
}
static void prepare_cancellation_points(void)
{
    const char* input = "# old\n\n**value** &amp; [link]\n\n[link]: /old\n\n- a\n- b\n";
    long point; int success = 0;
    for (point = 0; point < 1000 && !success; point++) {
        prepare_cancel_allocator a = {0, -1, 0, NULL}; xui_doc_desc_t desc = {0}; xui_document d;
        xui_document_prepare p; xui_doc_source_patch_t patch = prepare_patch(2, 5, "new"); int result;
        uint64_t revision; xui_document_snapshot before, after;
        desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN;
        desc.onAlloc = prepare_cancel_alloc; desc.onFree = prepare_cancel_free; desc.pAllocatorUser = &a;
        CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK && xuiDocumentLoadMarkdown(d, input, strlen(input)) == XUI_OK);
        CHECK(xuiDocumentAcquireSnapshot(d, &before) == XUI_OK); revision = xuiDocumentGetRevision(d);
        CHECK(xuiDocumentPrepareSource(d, NULL, &patch, 1, &p) == XUI_OK); a.prepare = p; a.remaining = point;
        result = xuiDocumentPrepareRun(p); a.prepare = NULL;
        if (a.fired) {
            CHECK(result == XUI_DOC_ERROR_CANCELLED && xuiDocumentPreparePublish(d, p, NULL) == XUI_DOC_ERROR_CANCELLED);
            CHECK(xuiDocumentGetRevision(d) == revision); test_source(d, input, 1);
            CHECK(xuiDocumentAcquireSnapshot(d, &after) == XUI_OK); expect_identity_tree(before, after, 1); xuiDocumentSnapshotRelease(after);
        } else { CHECK(result == XUI_OK && xuiDocumentPreparePublish(d, p, NULL) == XUI_OK); success = 1; }
        xuiDocumentSnapshotRelease(before); xuiDocumentPrepareRelease(p); xuiDocumentRelease(d); CHECK(!a.live);
    }
    CHECK(success); printf("Document prepare cancellation sweep: %ld checkpoints passed\n", point);
    {
        xui_doc_desc_t desc = {0}; xui_document d; xui_document_prepare p = NULL;
        xui_doc_source_patch_t patch = prepare_patch(0, 3, "123456789");
        desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN; desc.iMaxTextBytes = 8; desc.iMaxNodes = 3;
        CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK && xuiDocumentLoadMarkdown(d, "old", 3) == XUI_OK);
        CHECK(xuiDocumentPrepareSource(d, NULL, &patch, 1, &p) == XUI_DOC_ERROR_LIMIT && !p && !xuiDocumentHasPrepare(d));
        patch = prepare_patch(0, 3, "**a**b");
        CHECK(xuiDocumentPrepareSource(d, NULL, &patch, 1, &p) == XUI_OK && xuiDocumentPrepareRun(p) == XUI_DOC_ERROR_LIMIT);
        CHECK(xuiDocumentPreparePublish(d, p, NULL) == XUI_DOC_ERROR_LIMIT && xuiDocumentHasPrepare(d)); test_source(d, "old", 1);
        xuiDocumentPrepareRelease(p); xuiDocumentRelease(d);
    }
}
static uint64_t prepared_source_bytes(xui_document d)
{
    xui_document_snapshot s; uint64_t bytes;
    CHECK(xuiDocumentAcquireSnapshot(d, &s) == XUI_OK && xuiDocumentSnapshotCopySource(s, NULL, 0, &bytes) == XUI_OK);
    xuiDocumentSnapshotRelease(s); return bytes;
}
static void prepared_expect_accounting(xui_document d, xui_document oracle)
{
    xui_doc_memory_stats_t a, b; xui_doc_stats_t x = {0}, y = {0}; xui_document_snapshot s, t;
    char source[1024]; uint64_t n;
    CHECK(xuiDocumentAcquireSnapshot(d, &s) == XUI_OK && xuiDocumentAcquireSnapshot(oracle, &t) == XUI_OK);
    CHECK(xuiDocumentSnapshotCopySource(s, source, sizeof(source), &n) == XUI_OK); test_source(oracle, source, 1);
    expect_identity_tree(s, t, XUI_DOCUMENT_ROOT);
    xuiDocumentSnapshotRelease(s); xuiDocumentSnapshotRelease(t);
    a = document_memory(d); b = document_memory(oracle);
    if (memcmp(&a, &b, sizeof(a))) fprintf(stderr, "prepared memory mismatch: live %llu/%llu, current %llu/%llu, history %llu/%llu, other %llu/%llu\n",
        (unsigned long long)a.iLiveBytes, (unsigned long long)b.iLiveBytes, (unsigned long long)a.iCurrentBytes, (unsigned long long)b.iCurrentBytes,
        (unsigned long long)a.iHistoryBytes, (unsigned long long)b.iHistoryBytes, (unsigned long long)a.iOtherBytes, (unsigned long long)b.iOtherBytes);
    CHECK(!memcmp(&a, &b, sizeof(a)));
    x.iSize = y.iSize = sizeof(x); CHECK(xuiDocumentGetStats(d, &x) == XUI_OK && xuiDocumentGetStats(oracle, &y) == XUI_OK);
    CHECK(x.iUndoCount == y.iUndoCount && x.iRedoCount == y.iRedoCount && x.iHistoryBytes <= x.iHistoryMaxBytes);
    CHECK(xuiDocumentGetRevision(d) == xuiDocumentGetRevision(oracle) && xuiDocumentIsDirty(d) == xuiDocumentIsDirty(oracle));
}
static void prepared_accounting_model(void)
{
    unsigned disabled, i;
    for (disabled = 0; disabled < 2; disabled++) {
        xui_doc_desc_t options = {0}; xui_document d, oracle; xui_document_snapshot kept[4] = {0}, expected[4] = {0};
        options.iSize = sizeof(options); options.iProfile = XUI_DOCUMENT_MARKDOWN; options.bDisableHistory = (int)disabled;
        CHECK(xuiDocumentCreate(&options, &d) == XUI_OK && xuiDocumentCreate(&options, &oracle) == XUI_OK);
        CHECK(xuiDocumentLoadMarkdown(d, "seed", 4) == XUI_OK && xuiDocumentLoadMarkdown(oracle, "seed", 4) == XUI_OK);
        for (i = 0; i < 120; i++) {
            char source[512]; xui_doc_source_patch_t patch; xui_document_prepare p; xui_document_transaction t;
            xui_doc_txn_desc_t desc = {0}; xui_doc_stats_t before = {0}, after = {0}; int fast = 1;
            if (!(i % 9)) {
                uint64_t limits[] = {UINT64_MAX, 8192, 1}; uint64_t cap = limits[(i / 9) % 3];
                CHECK(xuiDocumentSetHistoryLimits(d, 4, cap) == XUI_OK && xuiDocumentSetHistoryLimits(oracle, 4, cap) == XUI_OK);
            }
            (void)snprintf(source, sizeof(source), "## doc_%u\n\nvalue_%u **mark_%u** &amp; [link]\n\n[link]: /v_%u\n", i, i / 3, i % 5, i % 7);
            patch = prepare_patch(0, prepared_source_bytes(d), source); desc.iSize = sizeof(desc); desc.iDomain = XUI_DOC_SOURCE;
            desc.iOrigin = 11; desc.iGroup = i / 4 + 1;
            CHECK(xuiDocumentPrepareSource(d, &desc, &patch, 1, &p) == XUI_OK && xuiDocumentPrepareRun(p) == XUI_OK);
            before.iSize = after.iSize = sizeof(before); CHECK(xuiDocumentGetStats(d, &before) == XUI_OK);
            if (i % 11 == 1) {
                CHECK(xuiDocumentSetHistoryLimits(d, 4, UINT64_MAX - i) == XUI_OK && xuiDocumentSetHistoryLimits(oracle, 4, UINT64_MAX - i) == XUI_OK); fast = 0;
            } else if (i % 11 == 2) {
                fast = !(before.iUndoCount || before.iRedoCount);
                CHECK(xuiDocumentClearHistory(d) == XUI_OK && xuiDocumentClearHistory(oracle) == XUI_OK);
            }
            CHECK(xuiDocumentBeginTransaction(oracle, &desc, &t) == XUI_OK);
            CHECK(xuiDocumentTxnReplaceSource(t, patch.iStart, patch.iEnd, patch.sText, patch.iTextBytes) == XUI_OK);
            CHECK(xuiDocumentTxnCommit(t, NULL) == XUI_OK); xuiDocumentTxnRelease(t);
            CHECK(xuiDocumentPreparePublish(d, p, NULL) == XUI_OK);
            CHECK(xuiDocumentGetStats(d, &after) == XUI_OK && after.iPreparedPublishes == before.iPreparedPublishes + fast);
            if (fast) CHECK(after.iAllocations == before.iAllocations); /* Publish allocates nothing. */
            xuiDocumentPrepareRelease(p); prepared_expect_accounting(d, oracle);
            if (i % 7 == 5 && xuiDocumentCanUndo(d)) {
                CHECK(xuiDocumentUndo(d, NULL) == XUI_OK && xuiDocumentUndo(oracle, NULL) == XUI_OK); prepared_expect_accounting(d, oracle);
                if (i % 14 == 12) { CHECK(xuiDocumentRedo(d, NULL) == XUI_OK && xuiDocumentRedo(oracle, NULL) == XUI_OK); prepared_expect_accounting(d, oracle); }
            }
            xuiDocumentSnapshotRelease(kept[i % 4]); xuiDocumentSnapshotRelease(expected[i % 4]);
            CHECK(xuiDocumentAcquireSnapshot(d, &kept[i % 4]) == XUI_OK && xuiDocumentAcquireSnapshot(oracle, &expected[i % 4]) == XUI_OK);
            if (!(i % 12)) { CHECK(xuiDocumentMarkSaved(d, kept[i % 4]) == XUI_OK && xuiDocumentMarkSaved(oracle, expected[i % 4]) == XUI_OK); }
            prepared_expect_accounting(d, oracle);
        }
        for (i = 0; i < 4; i++) { xuiDocumentSnapshotRelease(kept[i]); xuiDocumentSnapshotRelease(expected[i]); }
        prepared_expect_accounting(d, oracle); xuiDocumentRelease(d); xuiDocumentRelease(oracle);
    }
    puts("Document prepared accounting: 240 commits match synchronous physical memory/history, including grouping, caps, branches, snapshots and config fallback");
}
static void prepared_publication_failure(void)
{
    unsigned changed_config;
    for (changed_config = 0; changed_config < 2; changed_config++) {
        fail_allocator allocator = {-1, 0}; xui_doc_desc_t desc = {0}; xui_document d; xui_document_prepare p;
        xui_document_snapshot saved; xui_doc_source_patch_t patch = prepare_patch(0, 3, "# replacement\n\nvalue");
        uint64_t revision; xui_doc_stats_t before = {0}, after = {0}; int result;
        desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN;
        desc.onAlloc = failing_alloc; desc.onFree = failing_free; desc.pAllocatorUser = &allocator;
        CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK && xuiDocumentLoadMarkdown(d, "old", 3) == XUI_OK);
        CHECK(xuiDocumentAcquireSnapshot(d, &saved) == XUI_OK && xuiDocumentMarkSaved(d, saved) == XUI_OK); revision = xuiDocumentGetRevision(d);
        CHECK(xuiDocumentPrepareSource(d, NULL, &patch, 1, &p) == XUI_OK && xuiDocumentPrepareRun(p) == XUI_OK);
        if (changed_config) CHECK(xuiDocumentSetHistoryLimits(d, 2, 4096) == XUI_OK);
        before.iSize = after.iSize = sizeof(before); CHECK(xuiDocumentGetStats(d, &before) == XUI_OK);
        allocator.remaining = 0; result = xuiDocumentPreparePublish(d, p, NULL); allocator.remaining = -1;
        CHECK(result == (changed_config ? XUI_ERROR_OUT_OF_MEMORY : XUI_OK));
        CHECK(xuiDocumentGetStats(d, &after) == XUI_OK && after.iAllocations == before.iAllocations);
        if (changed_config) {
            CHECK(xuiDocumentGetRevision(d) == revision && !xuiDocumentIsDirty(d)); test_source(d, "old", 1);
            CHECK(after.iCurrentBytes == before.iCurrentBytes && after.iHistoryBytes == before.iHistoryBytes);
        } else { CHECK(after.iPreparedPublishes == before.iPreparedPublishes + 1); test_source(d, patch.sText, 1); }
        xuiDocumentSnapshotRelease(saved); xuiDocumentPrepareRelease(p); xuiDocumentRelease(d); CHECK(!allocator.live);
    }
    puts("Document prepared publication: allocation-free commit and atomic fallback OOM passed");
}
typedef struct prepared_diag_context { reader_test reader; atomic_int done; } prepared_diag_context;
static int32 prepared_diag_reader(void* data)
{
    prepared_diag_context* c = data;
    do { CHECK(memory_reader(&c->reader) == 0); } while (!atomic_load(&c->done));
    return 0;
}
static void prepared_concurrent_accounting(void)
{
    xui_document d = test_markdown_open("000"); prepared_diag_context context; unsigned i; xthread* readers[4];
    xui_doc_memory_stats_t memory = {0}; xui_doc_range_t original = test_find(d, "000");
    context.reader.text = original.tAnchor.iNodeId; atomic_init(&context.done, 0);
    CHECK(xuiDocumentAcquireSnapshot(d, &context.reader.snapshot) == XUI_OK && xuiDocumentSetHistoryLimits(d, 4, 8192) == XUI_OK);
    for (i = 0; i < 4; i++) { readers[i] = xrtThreadCreate(prepared_diag_reader, &context, 0); CHECK(readers[i]); }
    for (i = 0; i < 120; i++) {
        char text[64]; xui_doc_source_patch_t patch; xui_document_prepare p;
        (void)snprintf(text, sizeof(text), "**next_%u** &amp;", i); patch = prepare_patch(0, prepared_source_bytes(d), text);
        CHECK(xuiDocumentPrepareSource(d, NULL, &patch, 1, &p) == XUI_OK && xuiDocumentPrepareRun(p) == XUI_OK && xuiDocumentPreparePublish(d, p, NULL) == XUI_OK);
        xuiDocumentPrepareRelease(p);
        if (!(i % 17)) CHECK(xuiDocumentClearHistory(d) == XUI_OK);
    }
    xuiDocumentRelease(d); atomic_store(&context.done, 1);
    for (i = 0; i < 4; i++) { CHECK(xrtThreadWaitFor(readers[i], 10000000) == XWAIT_OK && xrtThreadExitCode(readers[i]) == 0); xrtThreadDestroy(readers[i]); }
    memory.iSize = sizeof(memory); CHECK(xuiDocumentSnapshotGetMemoryStats(context.reader.snapshot, &memory) == XUI_OK && !memory.iOtherBytes && memory.iLiveBytes == memory.iSnapshotBytes);
    xuiDocumentSnapshotRelease(context.reader.snapshot);
    puts("Document prepared accounting: four concurrent snapshot diagnostics across 120 prepared commits, eviction and owner release passed");
}
