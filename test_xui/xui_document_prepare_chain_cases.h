/* Continued source input uses only immutable candidate data on the owner. */
static void prepare_chain_contract(void)
{
    const char* initial = "alpha\n\nkeep\n";
    const char* pending = "alpha!\n\nkeep\n";
    const char* expected = "**alpha!** \xe4\xb8\xad\xf0\x9f\x99\x82\n\nkeep\n";
    xui_document d = test_markdown_open(initial), other;
    xui_document_prepare a, b, c, invalid = NULL;
    xui_doc_source_patch_t patch = prepare_patch(5, 5, "!"), edits[3];
    xui_doc_txn_desc_t desc = {0}; xui_doc_stats_t before = {0}, after = {0};
    xui_doc_change_info_t info = {0}; xui_document_change_set change;
    xui_doc_position_t at, mapped; xui_doc_range_t keep = test_find(d, "keep");
    uint64_t revision = xuiDocumentGetRevision(d), token; int mapping; char bytes[8];
    prepare_notice notice = {xrtThreadCurrentId(), 0, 0}; xui_document_transaction writer;
    CHECK(xuiDocumentClearHistory(d) == XUI_OK);
    CHECK(xuiDocumentSubscribe(d, prepare_changed, &notice, &token) == XUI_OK);
    desc.iSize = sizeof(desc); desc.iDomain = XUI_DOC_SOURCE; desc.iOrigin = 71; desc.iGroup = 19;
    before.iSize = after.iSize = sizeof(before); CHECK(xuiDocumentGetStats(d, &before) == XUI_OK);
    at = test_position(d, XUI_DOCUMENT_ROOT, XUI_DOC_POSITION_SOURCE, 7, XUI_DOC_AFTER);
    CHECK(xuiDocumentPrepareSource(d, &desc, &patch, 1, &a) == XUI_OK);
    CHECK(xuiDocumentPrepareContinueSource(d, NULL, NULL, 0, &invalid) == XUI_ERROR_INVALID_ARGUMENT && !invalid);
    CHECK(xuiDocumentCreate(NULL, &other) == XUI_OK);
    CHECK(xuiDocumentPrepareContinueSource(other, a, NULL, 0, &invalid) != XUI_OK && !invalid); xuiDocumentRelease(other);
    CHECK(xuiDocumentBeginTransaction(d, NULL, &writer) == XUI_OK);
    CHECK(xuiDocumentPrepareContinueSource(d, a, NULL, 0, &invalid) == XUI_DOC_ERROR_BUSY && !invalid); xuiDocumentTxnRelease(writer);
    edits[0] = prepare_patch(0, 0, "**");
    edits[1] = prepare_patch(8, 8, "**");
    edits[2] = prepare_patch(10, 10, " \xe4\xb8\xad\xf0\x9f\x99\x82");
    CHECK(xuiDocumentPrepareContinueSource(d, a, edits, 3, &b) == XUI_OK);
    CHECK(prepare_info(a).iResult == XUI_DOC_ERROR_STALE && prepare_info(b).iPatchCount == 4);
    CHECK(prepare_info(b).iBaseRevision == revision && prepare_info(b).iGeneration > prepare_info(a).iGeneration);
    prepare_source(a, pending); prepare_source(b, expected); test_source(d, initial, 1);
    CHECK(xuiDocumentPrepareReadSource(b, 12, bytes, 6) == XUI_OK && !memcmp(bytes, expected + 12, 6));
    CHECK(xuiDocumentPrepareReadSource(b, strlen(expected), NULL, 0) == XUI_OK);
    CHECK(xuiDocumentPrepareReadSource(b, UINT64_MAX, bytes, 1) == XUI_ERROR_INVALID_ARGUMENT);
    CHECK(xuiDocumentPrepareReadSource(b, strlen(expected), bytes, 1) == XUI_ERROR_INVALID_ARGUMENT);
    CHECK(xuiDocumentPrepareReadSource(b, 0, NULL, 1) == XUI_ERROR_INVALID_ARGUMENT);
    CHECK(xuiDocumentPrepareReadSource(NULL, 0, NULL, 0) == XUI_ERROR_INVALID_ARGUMENT);
    patch = prepare_patch(12, 12, "x"); /* Middle of the Chinese code point. */
    CHECK(xuiDocumentPrepareContinueSource(d, b, &patch, 1, &invalid) == XUI_DOC_ERROR_UTF8 && !invalid);
    patch = prepare_patch(0, 0, "\xe4");
    CHECK(xuiDocumentPrepareContinueSource(d, b, &patch, 1, &invalid) == XUI_DOC_ERROR_UTF8 && !invalid);
    CHECK(!prepare_info(b).bCancellationRequested && !notice.calls);
    CHECK(xuiDocumentGetStats(d, &after) == XUI_OK && after.iMarkdownParses == before.iMarkdownParses);
    CHECK(xuiDocumentPrepareRun(b) == XUI_OK);
    /* A ready candidate can be retried or extended without reading its tree. */
    CHECK(xuiDocumentPrepareContinueSource(d, b, NULL, 0, &c) == XUI_OK && prepare_info(c).iPatchCount == 4);
    CHECK(xuiDocumentPrepareContinueSource(d, b, NULL, 0, &invalid) == XUI_DOC_ERROR_STALE && !invalid);
    CHECK(xuiDocumentPreparePublish(d, b, NULL) == XUI_DOC_ERROR_STALE);
    CHECK(xuiDocumentPrepareRun(c) == XUI_OK && xuiDocumentPreparePublish(d, c, &change) == XUI_OK);
    CHECK(notice.calls == 1 && xuiDocumentGetRevision(d) == revision + 1);
    info.iSize = sizeof(info); CHECK(xuiDocumentChangeSetGetInfo(change, &info) == XUI_OK);
    CHECK(info.iOperationCount == 4 && info.iOrigin == 71 && info.iDomain == XUI_DOC_SOURCE);
    CHECK(xuiDocumentMapPosition(change, &at, &mapped, &mapping) == XUI_OK && mapping == XUI_DOC_MAP_EXACT && mapped.iOffset == 20);
    CHECK(test_find(d, "keep").tAnchor.iNodeId == keep.tAnchor.iNodeId); test_source(d, expected, 1);
    xuiDocumentChangeSetRelease(change);
    CHECK(xuiDocumentUndo(d, NULL) == XUI_OK && !xuiDocumentCanUndo(d)); test_source(d, initial, 1);
    CHECK(xuiDocumentRedo(d, NULL) == XUI_OK); test_source(d, expected, 1);
    xuiDocumentPrepareRelease(a); xuiDocumentPrepareRelease(b); xuiDocumentPrepareRelease(c);
    CHECK(xuiDocumentPrepareSource(d, NULL, NULL, 0, &a) == XUI_OK);
    CHECK(xuiDocumentPrepareContinueSource(d, a, NULL, 0, &b) == XUI_OK);
    revision = xuiDocumentGetRevision(d);
    CHECK(xuiDocumentPrepareRun(b) == XUI_OK && xuiDocumentPreparePublish(d, b, NULL) == XUI_OK && xuiDocumentGetRevision(d) == revision);
    xuiDocumentPrepareRelease(a); xuiDocumentPrepareRelease(b);
    CHECK(xuiDocumentPrepareSource(d, NULL, NULL, 0, &a) == XUI_OK); xuiDocumentPrepareCancel(a);
    CHECK(xuiDocumentPrepareContinueSource(d, a, NULL, 0, &invalid) == XUI_DOC_ERROR_CANCELLED && !invalid);
    CHECK(xuiDocumentCancelPrepare(d) == XUI_OK); xuiDocumentPrepareRelease(a);
    xuiDocumentUnsubscribe(d, token); xuiDocumentRelease(d);
    puts("Document continued input: sequential offsets, UTF-8 slices, immutable projections, one publish/history, node identity and source mappings passed");
}

static void prepare_chain_concurrent(void)
{
    prepare_gate_allocator a; xui_doc_desc_t desc = {0}; xui_document d;
    xui_document_prepare first, next; xui_doc_source_patch_t patch = prepare_patch(0, 3, "new"); xthread* thread;
    atomic_init(&a.live, 0); atomic_init(&a.gate, 0); a.owner = xrtThreadCurrentId();
    atomic_init(&a.entered, 0); atomic_init(&a.resume, 0);
    desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN;
    desc.onAlloc = prepare_gate_alloc; desc.onFree = prepare_gate_free; desc.pAllocatorUser = &a;
    CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK && xuiDocumentLoadMarkdown(d, "old", 3) == XUI_OK);
    CHECK(xuiDocumentClearHistory(d) == XUI_OK && xuiDocumentPrepareSource(d, NULL, &patch, 1, &first) == XUI_OK);
    atomic_store(&a.gate, 1); thread = prepare_start(first); prepare_gate_wait(&a.entered);
    CHECK(prepare_info(first).iState == XUI_DOC_PREPARE_RUNNING);
    patch = prepare_patch(3, 3, " \xe4\xb8\xad");
    CHECK(xuiDocumentPrepareContinueSource(d, first, &patch, 1, &next) == XUI_OK);
    prepare_source(first, "new"); prepare_source(next, "new \xe4\xb8\xad");
    /* Publish the newer generation while the original worker still owns its
     * builder, then release the live document before that worker resumes. */
    CHECK(xuiDocumentPrepareRun(next) == XUI_OK && xuiDocumentPreparePublish(d, next, NULL) == XUI_OK);
    test_source(d, "new \xe4\xb8\xad", 1);
    CHECK(xuiDocumentUndo(d, NULL) == XUI_OK && !xuiDocumentCanUndo(d)); test_source(d, "old", 1);
    xuiDocumentRelease(d); xuiDocumentPrepareRelease(next); xuiDocumentPrepareRelease(first);
    atomic_store(&a.resume, 1); CHECK(prepare_finish(thread) == XUI_DOC_ERROR_STALE);
    CHECK(!atomic_load(&a.live));
    puts("Document continued input: newer generation publishes while its predecessor runs; owner destruction and worker reclamation passed");
}

static void prepare_chain_failures(void)
{
    long budget; int success = 0;
    for (budget = 0; budget < 2000 && !success; budget++) {
        fail_allocator a = {-1, 0}; xui_doc_desc_t desc = {0}; xui_document d;
        xui_document_prepare first, next = NULL; xui_doc_source_patch_t patch = prepare_patch(0, 3, "**new**");
        xui_doc_stats_t before = {0}, after = {0}; unsigned notices = notifications; uint64_t revision, token; int result;
        desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN;
        desc.onAlloc = failing_alloc; desc.onFree = failing_free; desc.pAllocatorUser = &a;
        CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK && xuiDocumentLoadMarkdown(d, "old", 3) == XUI_OK);
        CHECK(xuiDocumentClearHistory(d) == XUI_OK && xuiDocumentSubscribe(d, changed, NULL, &token) == XUI_OK);
        CHECK(xuiDocumentPrepareSource(d, NULL, &patch, 1, &first) == XUI_OK && xuiDocumentPrepareRun(first) == XUI_OK);
        patch = prepare_patch(7, 7, " &amp; [ref]\n\n[ref]: /target\n");
        before.iSize = after.iSize = sizeof(before); CHECK(xuiDocumentGetStats(d, &before) == XUI_OK);
        revision = xuiDocumentGetRevision(d); a.remaining = budget;
        result = xuiDocumentPrepareContinueSource(d, first, &patch, 1, &next);
        if (!next) CHECK(!prepare_info(first).bCancellationRequested && prepare_info(first).iState == XUI_DOC_PREPARE_READY);
        if (result == XUI_OK) result = xuiDocumentPrepareRun(next);
        if (result == XUI_OK) result = xuiDocumentPreparePublish(d, next, NULL);
        a.remaining = -1; CHECK(xuiDocumentGetStats(d, &after) == XUI_OK);
        if (result != XUI_OK) {
            CHECK(result == XUI_ERROR_OUT_OF_MEMORY && notifications == notices && xuiDocumentGetRevision(d) == revision);
            CHECK(before.iCurrentBytes == after.iCurrentBytes && before.iHistoryBytes == after.iHistoryBytes && !xuiDocumentCanUndo(d));
            test_source(d, "old", 1);
            if (!next) { CHECK(xuiDocumentPreparePublish(d, first, NULL) == XUI_OK); test_source(d, "**new**", 1); }
            else prepare_source(next, "**new** &amp; [ref]\n\n[ref]: /target\n");
        } else {
            success = 1; CHECK(notifications == notices + 1); test_source(d, "**new** &amp; [ref]\n\n[ref]: /target\n", 1);
            CHECK(xuiDocumentUndo(d, NULL) == XUI_OK && !xuiDocumentCanUndo(d)); test_source(d, "old", 1);
        }
        xuiDocumentPrepareRelease(first); xuiDocumentPrepareRelease(next); xuiDocumentRelease(d); CHECK(!a.live);
    }
    CHECK(success); printf("Document continued input allocation-failure sweep: %ld points passed\n", budget);
    {
        fail_allocator a = {-1, 0}; xui_doc_desc_t desc = {0}; xui_document d; xui_document_prepare first, next;
        xui_doc_source_patch_t patch = prepare_patch(0, 0, "typed");
        desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN;
        desc.onAlloc = failing_alloc; desc.onFree = failing_free; desc.pAllocatorUser = &a;
        CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK && xuiDocumentPrepareSource(d, NULL, &patch, 1, &first) == XUI_OK);
        a.remaining = 0; CHECK(xuiDocumentPrepareRun(first) == XUI_ERROR_OUT_OF_MEMORY); a.remaining = -1;
        CHECK(xuiDocumentPrepareContinueSource(d, first, NULL, 0, &next) == XUI_OK);
        CHECK(xuiDocumentPrepareRun(next) == XUI_OK && xuiDocumentPreparePublish(d, next, NULL) == XUI_OK); test_source(d, "typed", 1);
        xuiDocumentPrepareRelease(first); xuiDocumentPrepareRelease(next); xuiDocumentRelease(d); CHECK(!a.live);
    }
}

static void prepare_chain_storage(void)
{
    const uint64_t size = UINT64_C(10) * 1024 * 1024;
    char* source = malloc((size_t)size); xui_document d; xui_doc_desc_t desc = {0};
    xui_document_prepare p, next; xui_doc_source_patch_t patch = {0};
    xui_doc_stats_t before = {0}, after = {0}; uint64_t i, previous_live; char slice[5];
    CHECK(source); memset(source, 'a', (size_t)size);
    desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN;
    CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK);
    patch.iSize = sizeof(patch); patch.sText = source; patch.iTextBytes = size;
    CHECK(xuiDocumentPrepareSource(d, NULL, &patch, 1, &p) == XUI_OK); free(source);
    before.iSize = after.iSize = sizeof(before); CHECK(xuiDocumentGetStats(d, &before) == XUI_OK);
    previous_live = before.iLiveBytes;
    for (i = 0; i < 2048; i++) {
        patch = prepare_patch(size + i, size + i, "b");
        CHECK(xuiDocumentPrepareContinueSource(d, p, &patch, 1, &next) == XUI_OK);
        xuiDocumentPrepareRelease(p); p = next;
        CHECK(xuiDocumentGetStats(d, &after) == XUI_OK);
        CHECK(after.iLiveBytes < previous_live + 65536); previous_live = after.iLiveBytes;
    }
    CHECK(after.iMarkdownParses == before.iMarkdownParses && after.iLiveBytes < before.iLiveBytes + 2048 * 4096);
    CHECK(prepare_info(p).iPatchCount == 2049 && prepare_info(p).iSourceBytes == size + 2048);
    CHECK(xuiDocumentPrepareReadSource(p, size - 1, slice, sizeof(slice)) == XUI_OK && !memcmp(slice, "abbbb", sizeof(slice)));
    CHECK(xuiDocumentCancelPrepare(d) == XUI_OK); xuiDocumentPrepareRelease(p); xuiDocumentRelease(d);
    printf("Document continued input: 2048 edits on a 10 MiB pending source, no parsing/full-source copies; %llu retained bytes for source/log paths\n",
        (unsigned long long)(after.iLiveBytes - before.iLiveBytes));
}
