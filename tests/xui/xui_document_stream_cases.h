static void prepare_stream_splits(void)
{
    const char* text = "# \xe4\xb8\xad\xf0\x9f\x99\x82\r\n\r\n**bold** &amp; [ref]\n\n[ref]: /target\n"
        "\xc2\x80\xdf\xbf\xe0\xa0\x80\xed\x9f\xbf\xee\x80\x80\xef\xbf\xbf\xf0\x90\x80\x80\xf4\x8f\xbf\xbf\n";
    uint64_t split, length = strlen(text); char expected[512];
    snprintf(expected, sizeof(expected), "seed\n\n%s", text);
    for (split = 0; split <= length; split++) {
        xui_document d = test_markdown_open("seed\n\n"); xui_document_prepare a, b, invalid = NULL;
        xui_doc_txn_desc_t desc = {0}; xui_doc_prepare_info_t info; xui_doc_change_info_t change_info = {0};
        xui_document_change_set change; uint64_t complete = split; char projection[512];
        while (complete && complete < length && ((unsigned char)text[complete] & 0xc0) == 0x80) complete--;
        CHECK(xuiDocumentClearHistory(d) == XUI_OK);
        desc.iSize = sizeof(desc); desc.iDomain = XUI_DOC_SOURCE; desc.iOrigin = 81; desc.iGroup = 29;
        CHECK(xuiDocumentPrepareStreamSource(d, NULL, &desc, text, split, 0, &a) == XUI_OK);
        info = prepare_info(a); CHECK(info.iBufferedUtf8Bytes == split - complete && info.iSourceBytes == complete + 6);
        memcpy(projection, expected, (size_t)complete + 6); projection[complete + 6] = 0; prepare_source(a, projection);
        if (info.iBufferedUtf8Bytes) {
            CHECK(xuiDocumentPrepareRun(a) == XUI_DOC_ERROR_BUSY && prepare_info(a).iState == XUI_DOC_PREPARE_QUEUED);
            CHECK(xuiDocumentPreparePublish(d, a, NULL) == XUI_DOC_ERROR_BUSY && xuiDocumentHasPrepare(d));
            CHECK(xuiDocumentPrepareContinueSource(d, a, NULL, 0, &invalid) == XUI_DOC_ERROR_BUSY && !invalid);
            CHECK(xuiDocumentPrepareStreamSource(d, a, NULL, NULL, 0, 1, &invalid) == XUI_DOC_ERROR_UTF8 && !invalid);
            CHECK(xuiDocumentSaveFile(d, "build/document/unfinished-stream.md", XUI_DOC_FILE_MARKDOWN) == XUI_DOC_ERROR_BUSY);
            CHECK(!prepare_info(a).bCancellationRequested);
        }
        CHECK(xuiDocumentPrepareStreamSource(d, a, NULL, text + split, length - split, 1, &b) == XUI_OK);
        CHECK(!prepare_info(b).iBufferedUtf8Bytes && prepare_info(b).iSourceBytes == length + 6);
        prepare_source(b, expected); test_source(d, "seed\n\n", 1);
        CHECK(xuiDocumentPrepareRun(b) == XUI_OK && xuiDocumentPreparePublish(d, b, &change) == XUI_OK);
        change_info.iSize = sizeof(change_info); CHECK(xuiDocumentChangeSetGetInfo(change, &change_info) == XUI_OK && change_info.iOrigin == 81);
        xuiDocumentChangeSetRelease(change); test_source(d, expected, 1);
        CHECK(xuiDocumentUndo(d, NULL) == XUI_OK && !xuiDocumentCanUndo(d)); test_source(d, "seed\n\n", 1);
        CHECK(xuiDocumentRedo(d, NULL) == XUI_OK); test_source(d, expected, 1);
        xuiDocumentPrepareRelease(a); xuiDocumentPrepareRelease(b); xuiDocumentRelease(d);
    }
    printf("Document UTF-8 stream: all %llu two-chunk splits, pending-byte gates, complete Unicode range and one shared Undo passed\n", (unsigned long long)(length + 1));
}

static void prepare_stream_invalid(void)
{
    const char* invalid[] = {"\x80", "\xc0", "\xc1", "\xf5", "\xff", "\xe0\x80", "\xed\xa0", "\xf0\x80", "\xf4\x90", "\xe2" "A", "\xf0\x9f\x7f"};
    xui_document d = test_markdown_open("base"); xui_document_prepare a, b, bad = NULL;
    unsigned i; xui_doc_txn_desc_t desc = {0}; uint64_t revision = xuiDocumentGetRevision(d);
    CHECK(xuiDocumentPrepareStreamSource(d, NULL, NULL, "ok", 2, 0, &a) == XUI_OK);
    for (i = 0; i < sizeof(invalid) / sizeof(*invalid); i++) {
        CHECK(xuiDocumentPrepareStreamSource(d, a, NULL, invalid[i], strlen(invalid[i]), 0, &bad) == XUI_DOC_ERROR_UTF8 && !bad);
        CHECK(!prepare_info(a).bCancellationRequested && prepare_info(a).iSourceBytes == 6 && xuiDocumentGetRevision(d) == revision);
    }
    desc.iSize = sizeof(desc); desc.iDomain = XUI_DOC_SOURCE;
    CHECK(xuiDocumentPrepareStreamSource(d, a, &desc, "x", 1, 0, &bad) == XUI_ERROR_INVALID_ARGUMENT && !bad);
    CHECK(xuiDocumentPrepareStreamSource(d, a, NULL, NULL, 1, 0, &bad) == XUI_ERROR_INVALID_ARGUMENT && !bad);
    CHECK(xuiDocumentPrepareStreamSource(d, a, NULL, "\xe4", 1, 0, &b) == XUI_OK);
    xuiDocumentPrepareRelease(a); a = b; CHECK(prepare_info(a).iBufferedUtf8Bytes == 1);
    CHECK(xuiDocumentPrepareStreamSource(d, a, NULL, "A", 1, 0, &bad) == XUI_DOC_ERROR_UTF8 && !bad);
    CHECK(xuiDocumentPrepareStreamSource(d, a, NULL, "\xb8\xad\x80", 3, 0, &bad) == XUI_DOC_ERROR_UTF8 && !bad);
    CHECK(prepare_info(a).iBufferedUtf8Bytes == 1 && !prepare_info(a).bCancellationRequested); prepare_source(a, "baseok");
    CHECK(xuiDocumentPrepareStreamSource(d, a, NULL, "\xb8", 1, 0, &b) == XUI_OK);
    xuiDocumentPrepareRelease(a); a = b; CHECK(prepare_info(a).iBufferedUtf8Bytes == 2);
    CHECK(xuiDocumentPrepareStreamSource(d, a, NULL, "\xad", 1, 1, &b) == XUI_OK);
    xuiDocumentPrepareRelease(a); a = b;
    CHECK(xuiDocumentPrepareRun(a) == XUI_OK && xuiDocumentPreparePublish(d, a, NULL) == XUI_OK); test_source(d, "baseok\xe4\xb8\xad", 1);
    CHECK(xuiDocumentPrepareStreamSource(d, a, NULL, "x", 1, 0, &bad) == XUI_DOC_ERROR_STALE && !bad); xuiDocumentPrepareRelease(a);
    CHECK(xuiDocumentPrepareStreamSource(d, NULL, NULL, "\xf0\x9f", 2, 0, &a) == XUI_OK);
    xuiDocumentPrepareCancel(a);
    CHECK(xuiDocumentPrepareStreamSource(d, a, NULL, "\x99\x82", 2, 1, &bad) == XUI_DOC_ERROR_CANCELLED && !bad);
    CHECK(xuiDocumentPrepareRun(a) == XUI_DOC_ERROR_CANCELLED);
    CHECK(xuiDocumentPreparePublish(d, a, NULL) == XUI_DOC_ERROR_CANCELLED && !xuiDocumentHasPrepare(d));
    xuiDocumentPrepareRelease(a); xuiDocumentRelease(d);
    {
        xui_doc_desc_t create = {0}; create.iSize = sizeof(create); create.iProfile = XUI_DOCUMENT_MARKDOWN; create.iMaxTextBytes = 5;
        CHECK(xuiDocumentCreate(&create, &d) == XUI_OK && xuiDocumentLoadMarkdown(d, "aaa", 3) == XUI_OK);
        CHECK(xuiDocumentPrepareStreamSource(d, NULL, NULL, "\xe4\xb8", 2, 0, &a) == XUI_OK);
        CHECK(xuiDocumentPrepareStreamSource(d, a, NULL, "\xad", 1, 1, &bad) == XUI_DOC_ERROR_LIMIT && !bad);
        CHECK(prepare_info(a).iBufferedUtf8Bytes == 2 && !prepare_info(a).bCancellationRequested);
        CHECK(xuiDocumentCancelPrepare(d) == XUI_OK); xuiDocumentPrepareRelease(a); xuiDocumentRelease(d);
    }
    puts("Document UTF-8 stream: invalid partial/complete sequences, chunk atomicity, final truncation, cancellation and byte budgets passed");
}

static void prepare_stream_failures(void)
{
    long budget; int success = 0;
    for (budget = 0; budget < 1000 && !success; budget++) {
        fail_allocator a = {-1, 0}; xui_doc_desc_t desc = {0}; xui_document d; xui_document_prepare first, next = NULL;
        uint64_t revision; int result;
        desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN;
        desc.onAlloc = failing_alloc; desc.onFree = failing_free; desc.pAllocatorUser = &a;
        CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK);
        CHECK(xuiDocumentPrepareStreamSource(d, NULL, NULL, "**\xe4\xb8", 4, 0, &first) == XUI_OK);
        revision = xuiDocumentGetRevision(d); a.remaining = budget;
        result = xuiDocumentPrepareStreamSource(d, first, NULL, "\xad** &amp; tail", 14, 1, &next);
        if (!next) CHECK(prepare_info(first).iBufferedUtf8Bytes == 2 && !prepare_info(first).bCancellationRequested);
        if (result == XUI_OK) result = xuiDocumentPrepareRun(next);
        if (result == XUI_OK) result = xuiDocumentPreparePublish(d, next, NULL);
        a.remaining = -1;
        if (result != XUI_OK) {
            CHECK(result == XUI_ERROR_OUT_OF_MEMORY && xuiDocumentGetRevision(d) == revision && !xuiDocumentCanUndo(d)); test_source(d, "", 1);
            if (!next) {
                CHECK(xuiDocumentPrepareStreamSource(d, first, NULL, "\xad** &amp; tail", 14, 1, &next) == XUI_OK);
                CHECK(xuiDocumentPrepareRun(next) == XUI_OK && xuiDocumentPreparePublish(d, next, NULL) == XUI_OK);
            }
        } else { success = 1; test_source(d, "**\xe4\xb8\xad** &amp; tail", 1); }
        xuiDocumentPrepareRelease(first); xuiDocumentPrepareRelease(next); xuiDocumentRelease(d); CHECK(!a.live);
    }
    CHECK(success); printf("Document UTF-8 stream allocation-failure sweep: %ld points passed\n", budget);
}

static void prepare_stream_frames(void)
{
    const char* text = "# \xe4\xb8\xad\xf0\x9f\x99\x82\n\n**bold**\n\n[ref]\n\n[ref]: /target\n";
    xui_document d = test_markdown_open(""); xui_document_prepare p = NULL, next;
    xui_doc_txn_desc_t desc = {0}; xui_doc_stats_t stats = {0}; uint64_t i, frames = 0, token; unsigned notice = notifications;
    desc.iSize = sizeof(desc); desc.iDomain = XUI_DOC_SOURCE; desc.iOrigin = 98; desc.iGroup = 72;
    CHECK(xuiDocumentSubscribe(d, changed, NULL, &token) == XUI_OK);
    for (i = 0; i < strlen(text); i++) {
        CHECK(xuiDocumentPrepareStreamSource(d, p, p ? NULL : &desc, text + i, 1, 0, &next) == XUI_OK);
        xuiDocumentPrepareRelease(p); p = next;
        if (!(i % 7) && !prepare_info(p).iBufferedUtf8Bytes) {
            CHECK(prepare_finish(prepare_start(p)) == XUI_OK && xuiDocumentPreparePublish(d, p, NULL) == XUI_OK);
            xuiDocumentPrepareRelease(p); p = NULL; frames++;
        }
    }
    if (p) {
        CHECK(xuiDocumentPrepareStreamSource(d, p, NULL, NULL, 0, 1, &next) == XUI_OK); xuiDocumentPrepareRelease(p); p = next;
        CHECK(prepare_finish(prepare_start(p)) == XUI_OK && xuiDocumentPreparePublish(d, p, NULL) == XUI_OK);
        xuiDocumentPrepareRelease(p); frames++;
    }
    test_source(d, text, 1); stats.iSize = sizeof(stats); CHECK(xuiDocumentGetStats(d, &stats) == XUI_OK);
    CHECK(stats.iMarkdownParses == frames && stats.iUndoCount == 1 && notifications == notice + frames);
    CHECK(xuiDocumentUndo(d, NULL) == XUI_OK && !xuiDocumentCanUndo(d)); test_source(d, "", 1);
    CHECK(xuiDocumentRedo(d, NULL) == XUI_OK); test_source(d, text, 1);
    xuiDocumentUnsubscribe(d, token); xuiDocumentRelease(d);
    puts("Document UTF-8 stream: one-byte network chunks, host-batched worker parsing and grouped Undo across publications passed");
}
