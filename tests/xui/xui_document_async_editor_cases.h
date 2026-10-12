#include <stdatomic.h>
typedef struct async_editor_allocator {
    uint64_t owner;
    atomic_int gate, entered, resume, fail, fail_owner;
    atomic_uint live;
    atomic_uint_fast64_t owner_calls, fail_owner_at;
    unsigned errors;
} async_editor_allocator;
static void async_editor_gate(atomic_int* value)
{
    xdeadline deadline = xrtDeadlineAfter(10000000);
    while (!atomic_load(value)) { CHECK(!xrtDeadlineExpired(deadline)); xrtThreadYield(); }
}
static void* async_editor_alloc(void* data, size_t bytes)
{
    async_editor_allocator* a = data; void* p;
    if (xrtThreadCurrentId() == a->owner) {
        uint64_t call = atomic_fetch_add(&a->owner_calls, 1);
        if (atomic_load(&a->fail_owner) || call == atomic_load(&a->fail_owner_at)) return NULL;
    } else {
        if (atomic_exchange(&a->gate, 0)) { atomic_store(&a->entered, 1); async_editor_gate(&a->resume); }
        if (atomic_load(&a->fail)) return NULL;
    }
    p = malloc(bytes); if (p) atomic_fetch_add(&a->live, 1); return p;
}
static void async_editor_free(void* data, void* p)
{
    async_editor_allocator* a = data; CHECK(atomic_fetch_sub(&a->live, 1)); free(p);
}
static void async_editor_error(xui_widget w, int error, void* data)
{
    async_editor_allocator* a = data; CHECK(w && error != XUI_OK && xrtThreadCurrentId() == a->owner); a->errors++;
}
static xui_doc_prepare_info_t async_editor_info(xui_widget editor)
{
    xui_doc_prepare_info_t info = {0}; info.iSize = sizeof(info);
    CHECK(xuiDocumentEditorGetPendingInput(editor, &info) == XUI_OK); return info;
}
static void async_editor_flush(xui_context context, xui_widget editor)
{
    xdeadline deadline = xrtDeadlineAfter(10000000); int result;
    do {
        CHECK(xuiUpdate(context, .016f) == XUI_OK); result = xuiDocumentEditorFlush(editor);
        CHECK(result == XUI_OK || result == XUI_DOC_ERROR_BUSY);
        CHECK(!xrtDeadlineExpired(deadline)); xrtThreadYield();
    } while (result != XUI_OK);
}
static void async_editor_cases(void)
{
    xui_test_proxy_state_t proxy; xui_context context; xui_font font; xui_surface surface;
    xui_document d; xui_widget editor, other, view; xui_doc_desc_t create = {0};
    xui_doc_editor_desc_t ed = {0}; xui_doc_view_desc_t vd = {0}; async_editor_allocator a;
    xui_doc_range_t stale, range; xui_doc_prepare_info_t info; uint64_t revision;
    xui_rect_i_t damage = {0, 0, 400, 260}; xui_doc_stats_t stats = {0};
    atomic_init(&a.gate, 0); atomic_init(&a.entered, 0); atomic_init(&a.resume, 0); atomic_init(&a.fail, 0); atomic_init(&a.live, 0);
    atomic_init(&a.fail_owner, 0);
    atomic_init(&a.owner_calls, 0); atomic_init(&a.fail_owner_at, UINT64_MAX); a.owner = xrtThreadCurrentId(); a.errors = 0;
    xuiTestProxyInit(&proxy); CHECK(xuiCreate(&context) == XUI_OK && xuiSetProxy(context, &proxy.tProxy) == XUI_OK);
    CHECK(proxy.tProxy.fontLoadFile(&proxy.tProxy, &font, "test.ttf", 14, 0) == XUI_OK && xuiSetDefaultFont(context, font) == XUI_OK);
    CHECK(xuiTestSurfaceCreate(&proxy, &surface, 400, 260, XUI_SURFACE_USAGE_TARGET) == XUI_OK);
    create.iSize = sizeof(create); create.iProfile = XUI_DOCUMENT_MARKDOWN;
    create.onAlloc = async_editor_alloc; create.onFree = async_editor_free; create.pAllocatorUser = &a;
    CHECK(xuiDocumentCreate(&create, &d) == XUI_OK && xuiDocumentLoadMarkdown(d, "abc\n", 4) == XUI_OK);
    CHECK(xuiDocumentClearHistory(d) == XUI_OK);
    ed.iSize = sizeof(ed); ed.tView.iSize = sizeof(ed.tView); ed.tView.pDocument = d;
    ed.iMode = XUI_DOC_SOURCE_TEXT; ed.iAsyncSourceThresholdBytes = 1; ed.onError = async_editor_error; ed.pUser = &a;
    CHECK(xuiDocumentEditorCreate(context, &ed, &editor) == XUI_OK && xuiDocumentEditorCreate(context, &ed, &other) == XUI_OK);
    vd.iSize = sizeof(vd); vd.pDocument = d; CHECK(xuiDocumentViewCreate(context, &vd, &view) == XUI_OK);
    CHECK(xuiDocumentViewSetMode(view, XUI_DOC_SOURCE_TEXT) == XUI_OK);
    CHECK(xuiSetRootWidget(context, editor) == XUI_OK && xuiWidgetSetRect(editor, (xui_rect_t){0, 0, 400, 260}) == XUI_OK);
    CHECK(xuiInputViewport(context, 400, 260) == XUI_OK && xuiSetFocusWidget(context, editor) == XUI_OK);
    CHECK(xuiEditSetSelection(editor, 3, 3) == XUI_OK); revision = xuiDocumentGetRevision(d);
    CHECK(xuiDocumentEditorInsertText(editor, "!", 1) == XUI_OK);
    CHECK(xuiDocumentViewGetSelection(editor, &stale) == XUI_OK && stale.tCaret.iInputGeneration && stale.tCaret.iOffset == 4);
    CHECK(!strcmp(xuiEditGetText(editor), "abc!\n") && !strcmp(xuiEditGetText(view), "abc\n")); source(d, "abc\n");
    CHECK(xuiDocumentEditorInsertText(other, "wrong", 5) == XUI_DOC_ERROR_BUSY);
    atomic_store(&a.gate, 1); CHECK(xuiDocumentEditorFlush(editor) == XUI_DOC_ERROR_BUSY); async_editor_gate(&a.entered);
    CHECK(async_editor_info(editor).iState == XUI_DOC_PREPARE_RUNNING);
    CHECK(xuiDocumentEditorInsertText(editor, "\xe4\xb8\xad", 3) == XUI_OK);
    CHECK(xuiDocumentEditorInsertText(editor, "e\xcc\x81", 3) == XUI_OK);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_BACKSPACE) == XUI_OK);
    CHECK(xuiDocumentViewSetSelection(editor, &stale) == XUI_DOC_ERROR_STALE);
    CHECK(!strcmp(xuiEditGetText(editor), "abc!\xe4\xb8\xad\n"));
    CHECK(xuiEditSelectAll(editor) == XUI_OK && xuiEditCopy(editor) == XUI_OK && !strcmp(proxy.sClipboard, "abc!\xe4\xb8\xad\n"));
    CHECK(xuiEditSetSelection(editor, 7, 7) == XUI_OK);
    CHECK(xuiUpdate(context, .016f) == XUI_OK);
    CHECK(xuiRender(context, surface, &damage, 1) == XUI_OK);
    /* Cached widgets draw text into their cache surfaces, then composite those
     * surfaces onto this target; its direct text-call count is not meaningful. */
    CHECK(xuiTestSurfaceGetDrawCount(surface) > 0);
    CHECK(xuiDocumentViewHitTest(editor, 10, 10, &range.tCaret) == XUI_OK && range.tCaret.iInputGeneration == async_editor_info(editor).iGeneration);
    {
        xui_document_snapshot snapshot; int order;
        CHECK(xuiDocumentViewGetSelection(editor, &range) == XUI_OK && xuiDocumentAcquireSnapshot(d, &snapshot) == XUI_OK);
        CHECK(xuiDocumentSnapshotComparePositions(snapshot, &range.tAnchor, &range.tCaret, &order) != XUI_OK); xuiDocumentSnapshotRelease(snapshot);
    }
    CHECK(xuiDocumentViewSetMode(editor, XUI_DOC_VISUAL) == XUI_DOC_ERROR_BUSY && xuiDocumentViewGetMode(editor) == XUI_DOC_SOURCE_TEXT);
    CHECK(xuiDocumentSaveFile(d, "build/document/pending-editor.md", XUI_DOC_FILE_MARKDOWN) == XUI_DOC_ERROR_BUSY);
    /* Preedit is a separate immutable display candidate. It must neither
     * supersede accepted input nor let the worker publish under the IME. */
    info = async_editor_info(editor);
    send_text(context, editor, "\xe6\x96\x87", 1, 1);
    CHECK(xuiDocumentEditorIsComposing(editor) && async_editor_info(editor).iGeneration == info.iGeneration);
    CHECK(xuiRender(context, surface, &damage, 1) == XUI_OK && xuiDocumentEditorFlush(editor) == XUI_DOC_ERROR_BUSY);
    CHECK(xuiDocumentGetRevision(d) == revision && !strcmp(xuiEditGetText(editor), "abc!\xe4\xb8\xad\n"));
    send_text(context, editor, "", 1, 0); CHECK(!xuiDocumentEditorIsComposing(editor));
    send_text(context, editor, "\xe6\x96\x87", 1, 1); send_text(context, editor, "\xe6\x96\x87", 1, 0);
    CHECK(!strcmp(xuiEditGetText(editor), "abc!\xe4\xb8\xad\n") && xuiDocumentEditorIsComposing(editor));
    CHECK(xuiSetFocusWidget(context, NULL) == XUI_OK && xuiDocumentHasPrepare(d));
    atomic_store(&a.resume, 1); async_editor_flush(context, editor);
    CHECK(xuiDocumentGetRevision(d) == revision + 2 && !xuiDocumentHasPrepare(d)); source(d, "abc!\xe4\xb8\xad\xe6\x96\x87\n");
    CHECK(!strcmp(xuiEditGetText(view), "abc!\xe4\xb8\xad\xe6\x96\x87\n"));
    CHECK(xuiDocumentViewGetSelection(editor, &range) == XUI_OK && !range.tCaret.iInputGeneration && range.tCaret.iOffset == 10);
    stats.iSize = sizeof(stats); CHECK(xuiDocumentGetStats(d, &stats) == XUI_OK && stats.iUndoCount == 2);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK); source(d, "abc!\xe4\xb8\xad\n");
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK); source(d, "abc\n");
    CHECK(xuiDocumentViewGetSelection(editor, &range) == XUI_OK && range.tCaret.iOffset == 3 && !range.tCaret.iInputGeneration);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_REDO) == XUI_OK); source(d, "abc!\xe4\xb8\xad\n");
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_REDO) == XUI_OK); source(d, "abc!\xe4\xb8\xad\xe6\x96\x87\n");
    CHECK(a.errors == 0);
    /* Worker OOM preserves visible input and blocks saving. Explicit retry
     * gets a new generation and publishes after allocation recovers. */
    CHECK(xuiDocumentEditorInsertText(editor, "?", 1) == XUI_OK); atomic_store(&a.fail, 1);
    CHECK(xuiDocumentEditorFlush(editor) == XUI_DOC_ERROR_BUSY);
    {
        xdeadline deadline = xrtDeadlineAfter(10000000);
        do { info = async_editor_info(editor); CHECK(!xrtDeadlineExpired(deadline)); xrtThreadYield(); } while (info.iState != XUI_DOC_PREPARE_FAILED);
    }
    CHECK(xuiUpdate(context, .016f) == XUI_OK && a.errors == 1);
    CHECK(xuiDocumentEditorFlush(editor) == XUI_ERROR_OUT_OF_MEMORY && xuiDocumentHasPrepare(d));
    CHECK(strstr(xuiEditGetText(editor), "?") && !strstr(xuiEditGetText(view), "?"));
    atomic_store(&a.fail, 0); CHECK(xuiDocumentEditorRetryInput(editor) == XUI_OK && async_editor_info(editor).iGeneration > info.iGeneration);
    async_editor_flush(context, editor); source(d, "abc!\xe4\xb8\xad\xe6\x96\x87?\n");
    CHECK(xuiDocumentEditorInsertText(editor, "discard", 7) == XUI_OK && xuiDocumentEditorCancelInput(editor) == XUI_OK);
    CHECK(!xuiDocumentHasPrepare(d) && !strstr(xuiEditGetText(editor), "discard"));
    /* An explicit external write invalidates pending input; other editors
     * cannot silently replace it, but Document commands have explicit scope. */
    CHECK(xuiDocumentEditorInsertText(editor, "stale", 5) == XUI_OK);
    CHECK(xuiDocumentLoadMarkdown(d, "external", 8) == XUI_OK && !xuiDocumentHasPrepare(d));
    CHECK(!strcmp(xuiEditGetText(editor), "external"));
    CHECK(xuiDocumentViewSetMode(editor, XUI_DOC_VISUAL) == XUI_OK && xuiDocumentViewSetMode(editor, XUI_DOC_SOURCE_TEXT) == XUI_OK);
    /* Stress queue replacement/retirement without waiting between input and
     * cancellation, then mix completed publications with external writes. */
    {
        unsigned i;
        for (i = 0; i < 64; i++) {
            CHECK(xuiDocumentEditorInsertText(editor, "x", 1) == XUI_OK);
            CHECK(xuiDocumentEditorFlush(editor) == XUI_DOC_ERROR_BUSY);
            if (i % 4 == 0) CHECK(xuiDocumentEditorCancelInput(editor) == XUI_OK);
            else if (i % 4 == 1) {
                CHECK(xuiDocumentEditorInsertText(editor, "y", 1) == XUI_OK && xuiDocumentEditorCancelInput(editor) == XUI_OK);
            } else if (i % 4 == 2) {
                async_editor_flush(context, editor); CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
            } else CHECK(xuiDocumentLoadMarkdown(d, "external", 8) == XUI_OK);
            /* A no-op external write does not cancel an input candidate. */
            if (xuiDocumentHasPrepare(d)) CHECK(xuiDocumentEditorCancelInput(editor) == XUI_OK);
        }
    }
    CHECK(xuiDocumentClearHistory(d) == XUI_OK);
    atomic_store(&a.entered, 0); atomic_store(&a.resume, 0); atomic_store(&a.gate, 1);
    CHECK(xuiDocumentEditorInsertText(editor, "undo this batch", 15) == XUI_OK);
    {
        xui_event_t event = {0}; event.iSize = sizeof(event); event.iType = XUI_EVENT_KEY_DOWN;
        event.iKey = 'Z'; event.iModifiers = XUI_MOD_CTRL; event.pTarget = editor;
        CHECK(xuiDispatchEvent(context, &event) == XUI_OK); async_editor_gate(&a.entered);
        CHECK(xuiDocumentHasPrepare(d)); atomic_store(&a.resume, 1); async_editor_flush(context, editor);
        source(d, "external"); CHECK(!xuiDocumentCanUndo(d) && xuiDocumentCanRedo(d));
    }
    /* A history change invalidates the precomputed plan. Failure in the owner
     * fallback must retain the candidate just like a worker preparation OOM. */
    CHECK(xuiDocumentLoadMarkdown(d, "history base", 12) == XUI_OK);
    CHECK(xuiDocumentEditorInsertText(editor, "?", 1) == XUI_OK && xuiDocumentEditorFlush(editor) == XUI_DOC_ERROR_BUSY);
    {
        xdeadline deadline = xrtDeadlineAfter(10000000);
        do { info = async_editor_info(editor); CHECK(!xrtDeadlineExpired(deadline)); xrtThreadYield(); } while (info.iState != XUI_DOC_PREPARE_READY);
    }
    CHECK(xuiDocumentClearHistory(d) == XUI_OK); atomic_store(&a.fail_owner, 1);
    CHECK(xuiDocumentEditorFlush(editor) == XUI_ERROR_OUT_OF_MEMORY && xuiDocumentHasPrepare(d)); atomic_store(&a.fail_owner, 0);
    CHECK(async_editor_info(editor).iState == XUI_DOC_PREPARE_FAILED);
    CHECK(xuiDocumentEditorRetryInput(editor) == XUI_OK); async_editor_flush(context, editor);
    CHECK(xuiDocumentGetStats(d, &stats) == XUI_OK && stats.iUndoCount == 1);
    CHECK(xuiDocumentEditorInsertText(editor, "teardown", 8) == XUI_OK);
    CHECK(xuiDocumentEditorFlush(editor) == XUI_DOC_ERROR_BUSY);
    xuiWidgetDestroy(editor); CHECK(!xuiDocumentHasPrepare(d));
    xuiWidgetDestroy(other); xuiWidgetDestroy(view); xuiDocumentRelease(d);
    proxy.tProxy.surfaceDestroy(&proxy.tProxy, surface); xuiDestroy(context); proxy.tProxy.fontDestroy(&proxy.tProxy, font);
    CHECK(!atomic_load(&a.live));
    puts("Document async SOURCE editor: frozen worker input/selection/clipboard/rendering, committed preview, separate IME undo unit, OOM retry, cancel/external writes and joined teardown passed");
}
static void async_editor_stream_cases(void)
{
    xui_test_proxy_state_t proxy; xui_context context; xui_font font;
    xui_document d; xui_widget editor; xui_doc_desc_t create = {0};
    xui_doc_editor_desc_t ed = {0}; xui_doc_prepare_info_t pending;
    xui_doc_editor_pending_work_t work = {0}; xui_doc_range_t range;
    xdeadline deadline; int result;
    xuiTestProxyInit(&proxy);
    CHECK(xuiCreate(&context) == XUI_OK && xuiSetProxy(context, &proxy.tProxy) == XUI_OK);
    CHECK(proxy.tProxy.fontLoadFile(&proxy.tProxy, &font, "test.ttf", 14, 0) == XUI_OK &&
        xuiSetDefaultFont(context, font) == XUI_OK);
    create.iSize = sizeof(create); create.iProfile = XUI_DOCUMENT_MARKDOWN;
    CHECK(xuiDocumentCreate(&create, &d) == XUI_OK &&
        xuiDocumentLoadMarkdown(d, "seed\n\n", 6) == XUI_OK && xuiDocumentClearHistory(d) == XUI_OK);
    ed.iSize = sizeof(ed); ed.tView.iSize = sizeof(ed.tView); ed.tView.pDocument = d;
    ed.iMode = XUI_DOC_SOURCE_TEXT;
    CHECK(xuiDocumentEditorCreate(context, &ed, &editor) == XUI_OK);
    CHECK(xuiSetRootWidget(context, editor) == XUI_OK &&
        xuiWidgetSetRect(editor, (xui_rect_t){0, 0, 400, 260}) == XUI_OK &&
        xuiInputViewport(context, 400, 260) == XUI_OK);
    CHECK(xuiDocumentEditorAppendStreamSource(editor, "# \xe4", 3, 0) == XUI_OK);
    pending = async_editor_info(editor);
    CHECK(pending.iBufferedUtf8Bytes == 1 && pending.iSourceBytes == 8 &&
        !strcmp(xuiEditGetText(editor), "seed\n\n# "));
    CHECK(xuiDocumentViewGetSelection(editor, &range) == XUI_OK &&
        range.tCaret.iInputGeneration == pending.iGeneration && range.tCaret.iOffset == 8);
    CHECK(xuiDocumentEditorFlush(editor) == XUI_DOC_ERROR_BUSY);
    CHECK(xuiDocumentSaveFile(d, "build/document/stream-editor-pending.md", XUI_DOC_FILE_MARKDOWN) == XUI_DOC_ERROR_BUSY);
    CHECK(xuiDocumentViewSetMode(editor, XUI_DOC_VISUAL) == XUI_DOC_ERROR_BUSY);
    CHECK(xuiDocumentEditorInsertText(editor, "wrong", 5) == XUI_DOC_ERROR_BUSY);
    CHECK(xuiDocumentEditorAppendStreamSource(editor, "A", 1, 0) == XUI_DOC_ERROR_UTF8);
    CHECK(async_editor_info(editor).iBufferedUtf8Bytes == 1 &&
        !strcmp(xuiEditGetText(editor), "seed\n\n# "));
    CHECK(xuiDocumentEditorAppendStreamSource(editor, "\xb8\xad", 2, 0) == XUI_OK);
    CHECK(!strcmp(xuiEditGetText(editor), "seed\n\n# \xe4\xb8\xad"));
    deadline = xrtDeadlineAfter(10000000);
    do {
        CHECK(xuiUpdate(context, .016f) == XUI_OK);
        result = xuiDocumentEditorFlush(editor);
        CHECK(result == XUI_DOC_ERROR_BUSY && !xrtDeadlineExpired(deadline));
        xrtThreadYield();
    } while (xuiDocumentHasPrepare(d));
    source(d, "seed\n\n# \xe4\xb8\xad");
    work.iSize = sizeof(work);
    CHECK(xuiDocumentEditorGetPendingWork(editor, &work) == XUI_OK &&
        work.iResult == XUI_DOC_ERROR_BUSY && !work.bHasInput);
    CHECK(xuiDocumentSaveFile(d, "build/document/stream-editor-between-frames.md", XUI_DOC_FILE_MARKDOWN) == XUI_DOC_ERROR_BUSY);
    CHECK(xuiDocumentEditorAppendStreamSource(editor, "\n\n**bold**", 10, 0) == XUI_OK);
    CHECK(xuiDocumentEditorAppendStreamSource(editor, " [ref]", 6, 1) == XUI_OK);
    async_editor_flush(context, editor);
    source(d, "seed\n\n# \xe4\xb8\xad\n\n**bold** [ref]");
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
    source(d, "seed\n\n");
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_REDO) == XUI_OK);
    source(d, "seed\n\n# \xe4\xb8\xad\n\n**bold** [ref]");
    CHECK(xuiDocumentEditorAppendStreamSource(editor, "\xe4", 1, 0) == XUI_OK);
    CHECK(xuiDocumentEditorAppendStreamSource(editor, NULL, 0, 1) == XUI_DOC_ERROR_UTF8);
    CHECK(xuiDocumentEditorCancelInput(editor) == XUI_OK && !xuiDocumentHasPrepare(d));
    CHECK(xuiDocumentEditorGetPendingWork(editor, &work) == XUI_OK && work.iResult == XUI_OK);
    CHECK(xuiDocumentEditorAppendStreamSource(editor, "!", 1, 0) == XUI_OK);
    send_text(context, editor, "q", 0, 0);
    CHECK(xuiDocumentEditorGetPendingWork(editor, &work) == XUI_OK &&
        work.iQueuedEvents == 1 && work.iResult == XUI_DOC_ERROR_BUSY);
    CHECK(xuiDocumentEditorAppendStreamSource(editor, "?", 1, 1) == XUI_OK);
    async_editor_flush(context, editor);
    source(d, "seed\n\n# \xe4\xb8\xad\n\n**bold** [ref]!?q");
    CHECK(xuiDocumentEditorAppendStreamSource(editor, "z", 1, 0) == XUI_OK);
    deadline = xrtDeadlineAfter(10000000);
    do {
        CHECK(xuiUpdate(context, .016f) == XUI_OK);
        CHECK(xuiDocumentEditorFlush(editor) == XUI_DOC_ERROR_BUSY && !xrtDeadlineExpired(deadline));
        xrtThreadYield();
    } while (xuiDocumentHasPrepare(d));
    CHECK(xuiDocumentEditorCancelInput(editor) == XUI_OK);
    CHECK(xuiDocumentEditorGetPendingWork(editor, &work) == XUI_OK && work.iResult == XUI_OK);
    CHECK(xuiDocumentSaveFile(d, "build/document/stream-editor-cancelled.md", XUI_DOC_FILE_MARKDOWN) == XUI_OK);
    source(d, "seed\n\n# \xe4\xb8\xad\n\n**bold** [ref]!?qz");
    CHECK(xuiDocumentViewSetMode(editor, XUI_DOC_VISUAL) == XUI_OK);
    CHECK(xuiDocumentEditorAppendStreamSource(editor, "x", 1, 1) == XUI_ERROR_UNSUPPORTED);
    xuiWidgetDestroy(editor); xuiDocumentRelease(d);
    xuiDestroy(context); proxy.tProxy.fontDestroy(&proxy.tProxy, font);
    puts("Document SOURCE Editor stream: split UTF-8, pending projection, frame publication, save/mode gates, grouped Undo, queued input, between-frame cancel and profile policy passed");
}
static void async_editor_live_cases(void)
{
    const char* original = "# Header\n\nmiddle\n\n**tail**\n";
    const char* edited = "# Header\n\nmidXYZdle\n\n**tail**\n";
    xui_test_proxy_state_t proxy; xui_context context; xui_font font;
    xui_doc_desc_t create = {0}; xui_doc_editor_desc_t ed = {0};
    xui_document d; xui_widget editor; xui_doc_range_t range;
    xui_doc_renderer_stats_t stats = {0}; xui_doc_prepare_info_t pending;
    xdeadline deadline;
    uint64_t middle = (uint64_t)(strstr(original, "middle") - original);
    xuiTestProxyInit(&proxy);
    CHECK(xuiCreate(&context) == XUI_OK && xuiSetProxy(context, &proxy.tProxy) == XUI_OK);
    CHECK(proxy.tProxy.fontLoadFile(&proxy.tProxy, &font, "test.ttf", 14, 0) == XUI_OK &&
        xuiSetDefaultFont(context, font) == XUI_OK);
    create.iSize = sizeof(create); create.iProfile = XUI_DOCUMENT_MARKDOWN;
    CHECK(xuiDocumentCreate(&create, &d) == XUI_OK &&
        xuiDocumentLoadMarkdown(d, original, strlen(original)) == XUI_OK &&
        xuiDocumentClearHistory(d) == XUI_OK);
    ed.iSize = sizeof(ed); ed.tView.iSize = sizeof(ed.tView); ed.tView.pDocument = d;
    ed.iMode = XUI_DOC_LIVE_MARKDOWN; ed.iAsyncSourceThresholdBytes = 1;
    CHECK(xuiDocumentEditorCreate(context, &ed, &editor) == XUI_OK &&
        xuiSetRootWidget(context, editor) == XUI_OK &&
        xuiWidgetSetRect(editor, (xui_rect_t){0, 0, 400, 260}) == XUI_OK &&
        xuiInputViewport(context, 400, 260) == XUI_OK);
    CHECK(xuiEditSetSelection(editor, (int)middle + 3, (int)middle + 3) == XUI_OK &&
        xuiDocumentEditorInsertText(editor, "XY", 2) == XUI_OK);
    pending = async_editor_info(editor);
    CHECK(!strcmp(xuiEditGetText(editor), "# Header\n\nmidXYdle\n\n**tail**\n") &&
        xuiDocumentViewGetSelection(editor, &range) == XUI_OK &&
        range.tCaret.iInputGeneration == pending.iGeneration && range.tCaret.iOffset == middle + 5);
    stats.iSize = sizeof(stats);
    CHECK(xuiDocumentViewGetRenderStats(editor, &stats) == XUI_OK &&
        stats.iSourceBlocks > 0 && stats.iSourceBlocks < stats.iBlocks && !stats.bLiveSourceFallback);
    CHECK(xuiDocumentEditorInsertText(editor, "Z", 1) == XUI_OK &&
        !strcmp(xuiEditGetText(editor), edited));
    source(d, original);
    CHECK(xuiDocumentSaveFile(d, "build/document/live-editor-pending.md", XUI_DOC_FILE_MARKDOWN) == XUI_DOC_ERROR_BUSY &&
        xuiDocumentViewSetMode(editor, XUI_DOC_SOURCE_TEXT) == XUI_DOC_ERROR_BUSY);
    async_editor_flush(context, editor);
    source(d, edited);
    CHECK(xuiDocumentViewGetMode(editor) == XUI_DOC_LIVE_MARKDOWN &&
        xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
    source(d, original);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_REDO) == XUI_OK);
    source(d, edited);
    CHECK(xuiEditSetSelection(editor, (int)middle, (int)strlen(edited)) == XUI_OK &&
        xuiDocumentEditorInsertText(editor, "replacement\n", 12) == XUI_OK &&
        xuiDocumentViewGetRenderStats(editor, &stats) == XUI_OK && stats.bLiveSourceFallback);
    CHECK(!strcmp(xuiEditGetText(editor), "# Header\n\nreplacement\n"));
    async_editor_flush(context, editor);
    source(d, "# Header\n\nreplacement\n");
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
    source(d, edited);
    CHECK(xuiDocumentEditorAppendStreamSource(editor, "z", 1, 0) == XUI_OK &&
        xuiDocumentViewGetRenderStats(editor, &stats) == XUI_OK &&
        stats.iSourceBlocks > 0 && stats.iSourceBlocks < stats.iBlocks && !stats.bLiveSourceFallback);
    deadline = xrtDeadlineAfter(10000000);
    do {
        CHECK(xuiUpdate(context, .016f) == XUI_OK &&
            xuiDocumentEditorFlush(editor) == XUI_DOC_ERROR_BUSY && !xrtDeadlineExpired(deadline));
        xrtThreadYield();
    } while (xuiDocumentHasPrepare(d));
    source(d, "# Header\n\nmidXYZdle\n\n**tail**\nz");
    CHECK(xuiDocumentSaveFile(d, "build/document/live-editor-stream-open.md", XUI_DOC_FILE_MARKDOWN) == XUI_DOC_ERROR_BUSY);
    CHECK(xuiDocumentEditorAppendStreamSource(editor, "!", 1, 1) == XUI_OK);
    async_editor_flush(context, editor);
    source(d, "# Header\n\nmidXYZdle\n\n**tail**\nz!");
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
    source(d, edited);
    xuiWidgetDestroy(editor); xuiDocumentRelease(d);
    xuiDestroy(context); proxy.tProxy.fontDestroy(&proxy.tProxy, font);
    puts("Document LIVE Editor pending source: hybrid projection, continued input, stream frames, shared history, save/mode gates and cross-block fallback passed");
}
static void async_editor_visual_cases(void)
{
    const char* original = "# Header\n\nhello world\n";
    const char* edited = "# Header\n\nhello\\*ZQ\\!\xe4\xb8\xad world\n";
    xui_test_proxy_state_t proxy; xui_context context; xui_font font;
    xui_doc_desc_t create = {0}; xui_doc_editor_desc_t ed = {0};
    xui_doc_view_desc_t vd = {0}; async_editor_allocator a;
    xui_document d; xui_document_snapshot snapshot;
    xui_widget editor, sibling; xui_doc_range_t selected, pending_selection;
    xui_doc_prepare_info_t pending; xui_doc_stats_t stats = {0};
    xdeadline deadline; uint64_t found, revision, matches;
    atomic_init(&a.gate, 0); atomic_init(&a.entered, 0); atomic_init(&a.resume, 0);
    atomic_init(&a.fail, 0); atomic_init(&a.fail_owner, 0); atomic_init(&a.live, 0);
    atomic_init(&a.owner_calls, 0); atomic_init(&a.fail_owner_at, UINT64_MAX); a.owner = xrtThreadCurrentId(); a.errors = 0;
    xuiTestProxyInit(&proxy);
    CHECK(xuiCreate(&context) == XUI_OK && xuiSetProxy(context, &proxy.tProxy) == XUI_OK);
    CHECK(proxy.tProxy.fontLoadFile(&proxy.tProxy, &font, "test.ttf", 14, 0) == XUI_OK &&
        xuiSetDefaultFont(context, font) == XUI_OK);
    create.iSize = sizeof(create); create.iProfile = XUI_DOCUMENT_MARKDOWN;
    create.onAlloc = async_editor_alloc; create.onFree = async_editor_free; create.pAllocatorUser = &a;
    CHECK(xuiDocumentCreate(&create, &d) == XUI_OK &&
        xuiDocumentLoadMarkdown(d, original, strlen(original)) == XUI_OK &&
        xuiDocumentClearHistory(d) == XUI_OK);
    ed.iSize = sizeof(ed); ed.tView.iSize = sizeof(ed.tView); ed.tView.pDocument = d;
    ed.iMode = XUI_DOC_VISUAL; ed.iAsyncSourceThresholdBytes = 1;
    ed.onError = async_editor_error; ed.pUser = &a;
    vd.iSize = sizeof(vd); vd.pDocument = d;
    CHECK(xuiDocumentEditorCreate(context, &ed, &editor) == XUI_OK &&
        xuiDocumentViewCreate(context, &vd, &sibling) == XUI_OK &&
        xuiSetRootWidget(context, editor) == XUI_OK &&
        xuiWidgetSetRect(editor, (xui_rect_t){0, 0, 400, 260}) == XUI_OK &&
        xuiInputViewport(context, 400, 260) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(d, &snapshot) == XUI_OK);
    CHECK(xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC, "hello", 5,
        NULL, &selected, 1, &found) == XUI_OK && found == 1);
    xuiDocumentSnapshotRelease(snapshot);
    selected.tAnchor.iOffset = selected.tCaret.iOffset = 5;
    CHECK(xuiDocumentViewSetSelection(editor, &selected) == XUI_OK);
    CHECK(xuiDocumentViewSetFindQuery(editor, "*Z", 2) == XUI_OK &&
        xuiDocumentViewGetFindResultCount(editor, &matches) == XUI_OK && !matches);
    revision = xuiDocumentGetRevision(d);
    CHECK(xuiDocumentEditorInsertText(editor, "*Z", 2) == XUI_OK);
    pending = async_editor_info(editor);
    CHECK(xuiDocumentGetRevision(d) == revision &&
        xuiDocumentViewGetSelection(editor, &pending_selection) == XUI_OK &&
        pending_selection.tCaret.iKind == XUI_DOC_POSITION_TEXT &&
        pending_selection.tCaret.iOffset == 7 &&
        strstr(xuiEditGetText(editor), "hello*Z world") &&
        strstr(xuiEditGetText(sibling), "hello world"));
    CHECK(xuiDocumentViewGetFindResultCount(editor, &matches) == XUI_OK && matches == 1);
    source(d, original);
    CHECK(xuiDocumentSaveFile(d, "build/document/visual-editor-pending.md", XUI_DOC_FILE_MARKDOWN) == XUI_DOC_ERROR_BUSY &&
        xuiDocumentViewSetMode(editor, XUI_DOC_SOURCE_TEXT) == XUI_DOC_ERROR_BUSY);
    atomic_store(&a.gate, 1);
    CHECK(xuiDocumentEditorFlush(editor) == XUI_DOC_ERROR_BUSY);
    async_editor_gate(&a.entered);
    atomic_store(&a.fail_owner, 1);
    CHECK(xuiDocumentEditorInsertText(editor, "E", 1) == XUI_ERROR_OUT_OF_MEMORY &&
        async_editor_info(editor).iGeneration == pending.iGeneration);
    atomic_store(&a.fail_owner, 0);
    CHECK(strstr(xuiEditGetText(editor), "hello*Z world"));
    CHECK(async_editor_info(editor).iState == XUI_DOC_PREPARE_RUNNING &&
        xuiDocumentEditorInsertText(editor, "Q", 1) == XUI_OK &&
        xuiDocumentEditorInsertText(editor, "!", 1) == XUI_OK &&
        xuiDocumentEditorInsertText(editor, "\xe4\xb8\xad", 3) == XUI_OK);
    CHECK(strstr(xuiEditGetText(editor), "hello*ZQ!\xe4\xb8\xad world") &&
        async_editor_info(editor).iPatchCount == 4 && xuiDocumentGetRevision(d) == revision);
    atomic_store(&a.resume, 1); async_editor_flush(context, editor);
    source(d, edited);
    CHECK(xuiDocumentGetRevision(d) == revision + 1 &&
        xuiDocumentViewGetSelection(editor, &pending_selection) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(d, &snapshot) == XUI_OK);
    { int order; CHECK(xuiDocumentSnapshotComparePositions(snapshot,
        &pending_selection.tAnchor, &pending_selection.tCaret, &order) == XUI_OK); }
    xuiDocumentSnapshotRelease(snapshot);
    stats.iSize = sizeof(stats);
    CHECK(xuiDocumentGetStats(d, &stats) == XUI_OK && stats.iUndoCount == 1);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK); source(d, original);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_REDO) == XUI_OK);
    source(d, edited);
    CHECK(xuiDocumentAcquireSnapshot(d, &snapshot) == XUI_OK);
    CHECK(xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC, "hello", 5,
        NULL, &selected, 1, &found) == XUI_OK && found == 1);
    xuiDocumentSnapshotRelease(snapshot);
    selected.tAnchor.iOffset = selected.tCaret.iOffset = 2;
    CHECK(xuiDocumentViewSetSelection(editor, &selected) == XUI_OK);
    CHECK(xuiDocumentViewSetFindQuery(editor, "~", 1) == XUI_OK &&
        xuiDocumentViewGetFindResultCount(editor, &matches) == XUI_OK && !matches);
    CHECK(xuiDocumentEditorInsertText(editor, "~", 1) == XUI_OK);
    CHECK(xuiDocumentViewGetFindResultCount(editor, &matches) == XUI_OK && matches == 1);
    CHECK(
        xuiDocumentEditorCancelInput(editor) == XUI_OK && !xuiDocumentHasPrepare(d));
    CHECK(xuiDocumentViewGetFindResultCount(editor, &matches) == XUI_OK && !matches);
    source(d, edited);
    CHECK(xuiDocumentEditorInsertText(editor, "?", 1) == XUI_OK);
    atomic_store(&a.fail, 1);
    CHECK(xuiDocumentEditorFlush(editor) == XUI_DOC_ERROR_BUSY);
    deadline = xrtDeadlineAfter(10000000);
    do { pending = async_editor_info(editor); CHECK(!xrtDeadlineExpired(deadline)); xrtThreadYield(); }
    while (pending.iState != XUI_DOC_PREPARE_FAILED);
    CHECK(xuiDocumentEditorFlush(editor) == XUI_ERROR_OUT_OF_MEMORY &&
        strstr(xuiEditGetText(editor), "he?llo*ZQ!\xe4\xb8\xad") && a.errors == 0);
    atomic_store(&a.fail, 0);
    CHECK(xuiDocumentEditorRetryInput(editor) == XUI_OK);
    async_editor_flush(context, editor);
    source(d, "# Header\n\nhe\\?llo\\*ZQ\\!\xe4\xb8\xad world\n");
    xuiWidgetDestroy(editor); xuiWidgetDestroy(sibling); xuiDocumentRelease(d);
    CHECK(xuiDocumentCreate(&create, &d) == XUI_OK &&
        xuiDocumentLoadMarkdown(d, "abcdef\n", 7) == XUI_OK &&
        xuiDocumentClearHistory(d) == XUI_OK);
    ed.tView.pDocument = d;
    CHECK(xuiDocumentEditorCreate(context, &ed, &editor) == XUI_OK &&
        xuiSetRootWidget(context, editor) == XUI_OK &&
        xuiWidgetSetRect(editor, (xui_rect_t){0, 0, 400, 260}) == XUI_OK &&
        xuiSetFocusWidget(context, editor) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(d, &snapshot) == XUI_OK &&
        xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC, "abcdef", 6,
            NULL, &selected, 1, &found) == XUI_OK && found == 1);
    xuiDocumentSnapshotRelease(snapshot);
    selected.tAnchor.iOffset = selected.tCaret.iOffset = 3;
    CHECK(xuiDocumentViewSetSelection(editor, &selected) == XUI_OK);
    atomic_store(&a.entered, 0); atomic_store(&a.resume, 0); atomic_store(&a.gate, 1);
    send_text(context, editor, "A", 0, 0);
    CHECK(xuiDocumentEditorFlush(editor) == XUI_DOC_ERROR_BUSY);
    async_editor_gate(&a.entered);
    send_text(context, editor, "B", 0, 0);
    CHECK(async_editor_info(editor).iPatchCount == 2 &&
        strstr(xuiEditGetText(editor), "abcABdef"));
    atomic_store(&a.resume, 1); async_editor_flush(context, editor);
    source(d, "abcABdef\n");
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
    source(d, "abcdef\n");
    xuiWidgetDestroy(editor); xuiDocumentRelease(d);
    { const char* original = "**abcdef**\n\n[ref]: /keep\n";
      const char* changed = "**abcX\\!def**\n\n[ref]: /keep\n";
      CHECK(xuiDocumentCreate(&create, &d) == XUI_OK &&
          xuiDocumentLoadMarkdown(d, original, strlen(original)) == XUI_OK &&
          xuiDocumentClearHistory(d) == XUI_OK);
      ed.tView.pDocument = d;
      CHECK(xuiDocumentEditorCreate(context, &ed, &editor) == XUI_OK &&
          xuiSetRootWidget(context, editor) == XUI_OK &&
          xuiWidgetSetRect(editor, (xui_rect_t){0, 0, 400, 260}) == XUI_OK);
      CHECK(xuiDocumentAcquireSnapshot(d, &snapshot) == XUI_OK &&
          xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC, "abcdef", 6,
              NULL, &selected, 1, &found) == XUI_OK && found == 1);
      xuiDocumentSnapshotRelease(snapshot);
      selected.tAnchor.iOffset = selected.tCaret.iOffset = 3;
      CHECK(xuiDocumentViewSetSelection(editor, &selected) == XUI_OK);
      revision = xuiDocumentGetRevision(d);
      atomic_store(&a.entered, 0); atomic_store(&a.resume, 0); atomic_store(&a.gate, 1);
      CHECK(xuiDocumentEditorInsertText(editor, "X", 1) == XUI_OK &&
          xuiDocumentGetRevision(d) == revision && xuiDocumentHasPrepare(d));
      CHECK(xuiDocumentEditorFlush(editor) == XUI_DOC_ERROR_BUSY); async_editor_gate(&a.entered);
      CHECK(xuiDocumentEditorInsertText(editor, "!", 1) == XUI_OK &&
          async_editor_info(editor).iPatchCount == 2);
      source(d, original);
      atomic_store(&a.resume, 1); async_editor_flush(context, editor);
      source(d, changed);
      CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK); source(d, original);
      xuiWidgetDestroy(editor); xuiDocumentRelease(d);
    }
    xuiDestroy(context); proxy.tProxy.fontDestroy(&proxy.tProxy, font);
    CHECK(!atomic_load(&a.live));
    puts("Document VISUAL text Prepare: continued API/event UTF-8/escaped input, committed sibling, find invalidation, cancel, worker OOM retry and Undo/Redo passed");
}
static void async_editor_visual_backspace_cases(void)
{
    xui_test_proxy_state_t proxy; xui_context context; xui_font font;
    xui_doc_desc_t create = {0}; xui_doc_editor_desc_t ed = {0};
    async_editor_allocator a; xui_document d; xui_widget editor;
    xui_document_snapshot snapshot; xui_doc_range_t selected;
    xui_doc_prepare_info_t pending; xui_doc_stats_t stats = {0};
    xui_event_t key = {0}; uint64_t found, revision;
    atomic_init(&a.gate, 0); atomic_init(&a.entered, 0); atomic_init(&a.resume, 0);
    atomic_init(&a.fail, 0); atomic_init(&a.fail_owner, 0); atomic_init(&a.live, 0);
    atomic_init(&a.owner_calls, 0); atomic_init(&a.fail_owner_at, UINT64_MAX); a.owner = xrtThreadCurrentId(); a.errors = 0;
    xuiTestProxyInit(&proxy);
    CHECK(xuiCreate(&context) == XUI_OK && xuiSetProxy(context, &proxy.tProxy) == XUI_OK);
    CHECK(proxy.tProxy.fontLoadFile(&proxy.tProxy, &font, "test.ttf", 14, 0) == XUI_OK &&
        xuiSetDefaultFont(context, font) == XUI_OK);
    create.iSize = sizeof(create); create.iProfile = XUI_DOCUMENT_MARKDOWN;
    create.onAlloc = async_editor_alloc; create.onFree = async_editor_free; create.pAllocatorUser = &a;
    ed.iSize = sizeof(ed); ed.tView.iSize = sizeof(ed.tView);
    ed.iMode = XUI_DOC_VISUAL; ed.iAsyncSourceThresholdBytes = 1;
    ed.onError = async_editor_error; ed.pUser = &a;
    CHECK(xuiDocumentCreate(&create, &d) == XUI_OK &&
        xuiDocumentLoadMarkdown(d, "abcdef\n", 7) == XUI_OK &&
        xuiDocumentClearHistory(d) == XUI_OK);
    ed.tView.pDocument = d;
    CHECK(xuiDocumentEditorCreate(context, &ed, &editor) == XUI_OK &&
        xuiSetRootWidget(context, editor) == XUI_OK &&
        xuiWidgetSetRect(editor, (xui_rect_t){0, 0, 400, 260}) == XUI_OK &&
        xuiSetFocusWidget(context, editor) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(d, &snapshot) == XUI_OK &&
        xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC, "abcdef", 6,
            NULL, &selected, 1, &found) == XUI_OK && found == 1);
    xuiDocumentSnapshotRelease(snapshot);
    selected.tAnchor.iOffset = selected.tCaret.iOffset = 4;
    CHECK(xuiDocumentViewSetSelection(editor, &selected) == XUI_OK);
    revision = xuiDocumentGetRevision(d);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_BACKSPACE) == XUI_OK &&
        !strcmp(xuiEditGetText(editor), "abcef\n"));
    pending = async_editor_info(editor);
    atomic_store(&a.gate, 1);
    CHECK(xuiDocumentEditorFlush(editor) == XUI_DOC_ERROR_BUSY); async_editor_gate(&a.entered);
    atomic_store(&a.fail_owner, 1);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_BACKSPACE) == XUI_ERROR_OUT_OF_MEMORY &&
        async_editor_info(editor).iGeneration == pending.iGeneration);
    atomic_store(&a.fail_owner, 0);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_BACKSPACE) == XUI_OK &&
        xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_BACKSPACE) == XUI_OK &&
        !strcmp(xuiEditGetText(editor), "aef\n") &&
        async_editor_info(editor).iPatchCount == 3 && xuiDocumentGetRevision(d) == revision);
    source(d, "abcdef\n");
    atomic_store(&a.resume, 1); async_editor_flush(context, editor);
    source(d, "aef\n");
    stats.iSize = sizeof(stats);
    CHECK(xuiDocumentGetStats(d, &stats) == XUI_OK && stats.iUndoCount == 1 &&
        xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
    source(d, "abcdef\n");
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_REDO) == XUI_OK); source(d, "aef\n");
    xuiWidgetDestroy(editor); xuiDocumentRelease(d);

    CHECK(xuiDocumentCreate(&create, &d) == XUI_OK &&
        xuiDocumentLoadMarkdown(d, "ab\xe4\xb8\xad" "def\n", 9) == XUI_OK &&
        xuiDocumentClearHistory(d) == XUI_OK);
    ed.tView.pDocument = d;
    CHECK(xuiDocumentEditorCreate(context, &ed, &editor) == XUI_OK &&
        xuiSetRootWidget(context, editor) == XUI_OK &&
        xuiWidgetSetRect(editor, (xui_rect_t){0, 0, 400, 260}) == XUI_OK &&
        xuiSetFocusWidget(context, editor) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(d, &snapshot) == XUI_OK &&
        xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC, "ab\xe4\xb8\xad" "def", 8,
            NULL, &selected, 1, &found) == XUI_OK && found == 1);
    xuiDocumentSnapshotRelease(snapshot);
    selected.tAnchor.iOffset = selected.tCaret.iOffset = 5;
    CHECK(xuiDocumentViewSetSelection(editor, &selected) == XUI_OK);
    key.iSize = sizeof(key); key.iType = XUI_EVENT_KEY_DOWN;
    key.iKey = XUI_KEY_BACKSPACE; key.pTarget = editor;
    revision = xuiDocumentGetRevision(d);
    atomic_store(&a.entered, 0); atomic_store(&a.resume, 0); atomic_store(&a.gate, 1);
    CHECK(xuiDispatchEvent(context, &key) == XUI_OK &&
        !strcmp(xuiEditGetText(editor), "abdef\n") &&
        xuiDocumentEditorFlush(editor) == XUI_DOC_ERROR_BUSY);
    async_editor_gate(&a.entered);
    CHECK(xuiDispatchEvent(context, &key) == XUI_OK &&
        !strcmp(xuiEditGetText(editor), "adef\n") &&
        async_editor_info(editor).iPatchCount == 2 && xuiDocumentGetRevision(d) == revision);
    atomic_store(&a.resume, 1); async_editor_flush(context, editor);
    source(d, "adef\n");
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
    source(d, "ab\xe4\xb8\xad" "def\n");
    CHECK(xuiDocumentClearHistory(d) == XUI_OK &&
        xuiDocumentAcquireSnapshot(d, &snapshot) == XUI_OK &&
        xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC, "ab\xe4\xb8\xad" "def", 8,
            NULL, &selected, 1, &found) == XUI_OK && found == 1);
    xuiDocumentSnapshotRelease(snapshot);
    selected.tAnchor.iOffset = selected.tCaret.iOffset = 2;
    CHECK(xuiDocumentViewSetSelection(editor, &selected) == XUI_OK);
    key.iKey = XUI_KEY_DELETE;
    revision = xuiDocumentGetRevision(d);
    atomic_store(&a.entered, 0); atomic_store(&a.resume, 0); atomic_store(&a.gate, 1);
    CHECK(xuiDispatchEvent(context, &key) == XUI_OK &&
        !strcmp(xuiEditGetText(editor), "abdef\n") &&
        xuiDocumentEditorFlush(editor) == XUI_DOC_ERROR_BUSY);
    async_editor_gate(&a.entered);
    CHECK(xuiDispatchEvent(context, &key) == XUI_OK &&
        xuiDispatchEvent(context, &key) == XUI_OK &&
        !strcmp(xuiEditGetText(editor), "abf\n") &&
        async_editor_info(editor).iPatchCount == 3 && xuiDocumentGetRevision(d) == revision);
    atomic_store(&a.resume, 1); async_editor_flush(context, editor);
    source(d, "abf\n");
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
    source(d, "ab\xe4\xb8\xad" "def\n");
    CHECK(xuiDocumentClearHistory(d) == XUI_OK &&
        xuiDocumentAcquireSnapshot(d, &snapshot) == XUI_OK &&
        xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC, "ab\xe4\xb8\xad" "def", 8,
            NULL, &selected, 1, &found) == XUI_OK && found == 1);
    xuiDocumentSnapshotRelease(snapshot);
    selected.tAnchor.iOffset = selected.tCaret.iOffset = 6;
    CHECK(xuiDocumentViewSetSelection(editor, &selected) == XUI_OK);
    revision = xuiDocumentGetRevision(d);
    atomic_store(&a.entered, 0); atomic_store(&a.resume, 0); atomic_store(&a.gate, 1);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_BACKSPACE) == XUI_OK &&
        !strcmp(xuiEditGetText(editor), "ab\xe4\xb8\xad" "ef\n") &&
        xuiDocumentEditorFlush(editor) == XUI_DOC_ERROR_BUSY);
    async_editor_gate(&a.entered);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_DELETE) == XUI_OK &&
        xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_BACKSPACE) == XUI_OK &&
        !strcmp(xuiEditGetText(editor), "abf\n") &&
        async_editor_info(editor).iPatchCount == 3 && xuiDocumentGetRevision(d) == revision);
    atomic_store(&a.resume, 1); async_editor_flush(context, editor);
    source(d, "abf\n");
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
    source(d, "ab\xe4\xb8\xad" "def\n");
    xuiWidgetDestroy(editor); xuiDocumentRelease(d);
    xuiDestroy(context); proxy.tProxy.fontDestroy(&proxy.tProxy, font);
    CHECK(!atomic_load(&a.live));
    puts("Document VISUAL pending Backspace/Delete: original-prefix/suffix source mapping, mixed API/key UTF-8, OOM retention and one Undo passed");
}
static void async_editor_visual_selection_cases(void)
{
    xui_test_proxy_state_t proxy; xui_context context; xui_font font;
    xui_doc_desc_t create = {0}; xui_doc_editor_desc_t ed = {0};
    async_editor_allocator a; xui_document d; xui_widget editor;
    xui_document_snapshot snapshot; xui_doc_range_t selected;
    xui_doc_stats_t stats = {0}; xui_doc_prepare_info_t pending;
    uint64_t found, revision;
    atomic_init(&a.gate, 0); atomic_init(&a.entered, 0); atomic_init(&a.resume, 0);
    atomic_init(&a.fail, 0); atomic_init(&a.fail_owner, 0); atomic_init(&a.live, 0);
    atomic_init(&a.owner_calls, 0); atomic_init(&a.fail_owner_at, UINT64_MAX); a.owner = xrtThreadCurrentId(); a.errors = 0;
    xuiTestProxyInit(&proxy);
    CHECK(xuiCreate(&context) == XUI_OK && xuiSetProxy(context, &proxy.tProxy) == XUI_OK &&
        proxy.tProxy.fontLoadFile(&proxy.tProxy, &font, "test.ttf", 14, 0) == XUI_OK &&
        xuiSetDefaultFont(context, font) == XUI_OK);
    create.iSize = sizeof(create); create.iProfile = XUI_DOCUMENT_MARKDOWN;
    create.onAlloc = async_editor_alloc; create.onFree = async_editor_free; create.pAllocatorUser = &a;
    CHECK(xuiDocumentCreate(&create, &d) == XUI_OK &&
        xuiDocumentLoadMarkdown(d, "abcdef\n", 7) == XUI_OK &&
        xuiDocumentClearHistory(d) == XUI_OK);
    ed.iSize = sizeof(ed); ed.tView.iSize = sizeof(ed.tView); ed.tView.pDocument = d;
    ed.iMode = XUI_DOC_VISUAL; ed.iAsyncSourceThresholdBytes = 1;
    ed.onError = async_editor_error; ed.pUser = &a;
    CHECK(xuiDocumentEditorCreate(context, &ed, &editor) == XUI_OK &&
        xuiSetRootWidget(context, editor) == XUI_OK &&
        xuiWidgetSetRect(editor, (xui_rect_t){0, 0, 400, 260}) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(d, &snapshot) == XUI_OK &&
        xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC, "abcdef", 6,
            NULL, &selected, 1, &found) == XUI_OK && found == 1);
    xuiDocumentSnapshotRelease(snapshot);
    selected.tAnchor.iOffset = selected.tCaret.iOffset = 3;
    CHECK(xuiDocumentViewSetSelection(editor, &selected) == XUI_OK);
    revision = xuiDocumentGetRevision(d);
    CHECK(xuiDocumentEditorInsertText(editor, "*Z", 2) == XUI_OK);
    atomic_store(&a.gate, 1);
    CHECK(xuiDocumentEditorFlush(editor) == XUI_DOC_ERROR_BUSY); async_editor_gate(&a.entered);
    CHECK(xuiDocumentViewGetSelection(editor, &selected) == XUI_OK);
    selected.tAnchor.iOffset = 3; selected.tCaret.iOffset = 4;
    CHECK(xuiDocumentViewSetSelection(editor, &selected) == XUI_OK);
    pending = async_editor_info(editor);
    atomic_store(&a.fail_owner, 1);
    CHECK(xuiDocumentEditorInsertText(editor, "?", 1) == XUI_ERROR_OUT_OF_MEMORY &&
        async_editor_info(editor).iGeneration == pending.iGeneration);
    atomic_store(&a.fail_owner, 0);
    CHECK(!strcmp(xuiEditGetText(editor), "abc*Zdef\n"));
    CHECK(xuiDocumentEditorInsertText(editor, "?", 1) == XUI_OK &&
        !strcmp(xuiEditGetText(editor), "abc?Zdef\n"));
    CHECK(xuiDocumentViewGetSelection(editor, &selected) == XUI_OK);
    selected.tAnchor.iOffset = selected.tCaret.iOffset = 6;
    CHECK(xuiDocumentViewSetSelection(editor, &selected) == XUI_OK &&
        xuiDocumentEditorInsertText(editor, "!", 1) == XUI_OK &&
        !strcmp(xuiEditGetText(editor), "abc?Zd!ef\n") &&
        async_editor_info(editor).iPatchCount == 3 && xuiDocumentGetRevision(d) == revision);
    CHECK(xuiDocumentViewGetSelection(editor, &selected) == XUI_OK);
    selected.tAnchor.iOffset = 3; selected.tCaret.iOffset = 5;
    CHECK(xuiDocumentViewSetSelection(editor, &selected) == XUI_OK);
    source(d, "abcdef\n");
    atomic_store(&a.resume, 1); async_editor_flush(context, editor);
    source(d, "abc\\?Zd\\!ef\n");
    CHECK(xuiEditCopy(editor) == XUI_OK && !strcmp(proxy.sClipboard, "?Z"));
    stats.iSize = sizeof(stats);
    CHECK(xuiDocumentGetStats(d, &stats) == XUI_OK && stats.iUndoCount == 1 &&
        xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
    source(d, "abcdef\n");
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_REDO) == XUI_OK &&
        xuiDocumentViewGetSelection(editor, &selected) == XUI_OK &&
        xuiDocumentAcquireSnapshot(d, &snapshot) == XUI_OK);
    { uint64_t caret_source; int mapping;
        CHECK(xuiDocumentPositionToSource(snapshot, &selected.tCaret,
            &caret_source, &mapping) == XUI_OK &&
            mapping == XUI_DOC_MAP_EXACT && caret_source == 9); }
    xuiDocumentSnapshotRelease(snapshot);
    xuiWidgetDestroy(editor); xuiDocumentRelease(d);

    { const char* original = "abcdef\n\nsecond paragraph\n";
      const char* changed = "abc\\*def\n\nsecond paragraph\n";
      CHECK(xuiDocumentCreate(&create, &d) == XUI_OK &&
          xuiDocumentLoadMarkdown(d, original, strlen(original)) == XUI_OK &&
          xuiDocumentClearHistory(d) == XUI_OK);
      ed.tView.pDocument = d;
      CHECK(xuiDocumentEditorCreate(context, &ed, &editor) == XUI_OK &&
          xuiSetRootWidget(context, editor) == XUI_OK &&
          xuiWidgetSetRect(editor, (xui_rect_t){0, 0, 400, 260}) == XUI_OK);
      CHECK(xuiDocumentAcquireSnapshot(d, &snapshot) == XUI_OK &&
          xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC, "abcdef", 6,
              NULL, &selected, 1, &found) == XUI_OK && found == 1);
      xuiDocumentSnapshotRelease(snapshot);
      selected.tAnchor.iOffset = selected.tCaret.iOffset = 3;
      CHECK(xuiDocumentViewSetSelection(editor, &selected) == XUI_OK &&
          xuiDocumentEditorInsertText(editor, "*", 1) == XUI_OK);
      atomic_store(&a.entered, 0); atomic_store(&a.resume, 0); atomic_store(&a.gate, 1);
      CHECK(xuiDocumentEditorFlush(editor) == XUI_DOC_ERROR_BUSY); async_editor_gate(&a.entered);
      CHECK(xuiDocumentAcquireSnapshot(d, &snapshot) == XUI_OK &&
          xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC, "second", 6,
              NULL, &selected, 1, &found) == XUI_OK && found == 1);
      xuiDocumentSnapshotRelease(snapshot);
      CHECK(xuiDocumentViewSetSelection(editor, &selected) == XUI_OK);
      source(d, original);
      atomic_store(&a.resume, 1); async_editor_flush(context, editor);
      source(d, changed);
      CHECK(xuiEditCopy(editor) == XUI_OK && !strcmp(proxy.sClipboard, "second") &&
          xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
      source(d, original);
      xuiWidgetDestroy(editor); xuiDocumentRelease(d);
    }
    { const char* original = "abcde\nfghij\n\ntail paragraph\n";
      const char* changed = "abc\\*Z\\!hij\n\ntail paragraph\n";
      xui_doc_range_t first, last;
      CHECK(xuiDocumentCreate(&create, &d) == XUI_OK &&
          xuiDocumentLoadMarkdown(d, original, strlen(original)) == XUI_OK &&
          xuiDocumentClearHistory(d) == XUI_OK);
      ed.tView.pDocument = d;
      CHECK(xuiDocumentEditorCreate(context, &ed, &editor) == XUI_OK &&
          xuiSetRootWidget(context, editor) == XUI_OK &&
          xuiWidgetSetRect(editor, (xui_rect_t){0, 0, 400, 260}) == XUI_OK);
      CHECK(xuiDocumentAcquireSnapshot(d, &snapshot) == XUI_OK &&
          xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC, "abcde", 5,
              NULL, &first, 1, &found) == XUI_OK && found == 1 &&
          xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC, "fghij", 5,
              NULL, &last, 1, &found) == XUI_OK && found == 1);
      xuiDocumentSnapshotRelease(snapshot);
      CHECK(first.tAnchor.iNodeId != last.tAnchor.iNodeId);
      first.tAnchor.iOffset = 3; first.tCaret = last.tCaret; first.tCaret.iOffset = 2;
      CHECK(xuiDocumentViewSetSelection(editor, &first) == XUI_OK);
      revision = xuiDocumentGetRevision(d);
      CHECK(xuiDocumentEditorInsertText(editor, "*Z", 2) == XUI_OK &&
          xuiDocumentGetRevision(d) == revision && xuiDocumentHasPrepare(d) &&
          !strcmp(xuiEditGetText(editor), "abc*Zhij\ntail paragraph\n"));
      CHECK(xuiDocumentEditorInsertText(editor, "!", 1) == XUI_OK &&
          async_editor_info(editor).iPatchCount == 2 &&
          !strcmp(xuiEditGetText(editor), "abc*Z!hij\ntail paragraph\n"));
      atomic_store(&a.entered, 0); atomic_store(&a.resume, 0); atomic_store(&a.gate, 1);
      CHECK(xuiDocumentEditorFlush(editor) == XUI_DOC_ERROR_BUSY); async_editor_gate(&a.entered);
      CHECK(xuiDocumentAcquireSnapshot(d, &snapshot) == XUI_OK &&
          xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC, "tail", 4,
              NULL, &selected, 1, &found) == XUI_OK && found == 1);
      xuiDocumentSnapshotRelease(snapshot);
      CHECK(xuiDocumentViewSetSelection(editor, &selected) == XUI_OK);
      source(d, original);
      atomic_store(&a.resume, 1); async_editor_flush(context, editor);
      source(d, changed);
      CHECK(xuiEditCopy(editor) == XUI_OK && !strcmp(proxy.sClipboard, "tail") &&
          xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
      source(d, original);
      CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_REDO) == XUI_OK);
      source(d, changed);
      CHECK(xuiDocumentViewGetSelection(editor, &selected) == XUI_OK &&
          xuiDocumentAcquireSnapshot(d, &snapshot) == XUI_OK);
      { uint64_t caret_source; int mapping;
        CHECK(xuiDocumentPositionToSource(snapshot, &selected.tCaret,
            &caret_source, &mapping) == XUI_OK &&
            mapping == XUI_DOC_MAP_EXACT && caret_source == 8); }
      xuiDocumentSnapshotRelease(snapshot);
      xuiWidgetDestroy(editor); xuiDocumentRelease(d);
    }
    { const char* original = "**a &amp; b &amp; c**\n\n[ref]: /keep\n";
      const char* changed = "**a &amp; bX\\!c**\n\n[ref]: /keep\n";
      xui_doc_range_t left, right, range;
      xui_doc_node_info_t node = {0};
      CHECK(xuiDocumentCreate(&create, &d) == XUI_OK &&
          xuiDocumentLoadMarkdown(d, original, strlen(original)) == XUI_OK &&
          xuiDocumentClearHistory(d) == XUI_OK);
      ed.tView.pDocument = d;
      CHECK(xuiDocumentEditorCreate(context, &ed, &editor) == XUI_OK &&
          xuiSetRootWidget(context, editor) == XUI_OK &&
          xuiWidgetSetRect(editor, (xui_rect_t){0, 0, 400, 260}) == XUI_OK);
      CHECK(xuiDocumentAcquireSnapshot(d, &snapshot) == XUI_OK &&
          xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC, "b", 1,
              NULL, &left, 1, &found) == XUI_OK && found == 1 &&
          xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC, "c", 1,
              NULL, &right, 1, &found) == XUI_OK && found == 1);
      xuiDocumentSnapshotRelease(snapshot);
      range.tAnchor = left.tCaret; range.tCaret = right.tAnchor;
      CHECK(range.tAnchor.iNodeId != range.tCaret.iNodeId &&
          xuiDocumentViewSetSelection(editor, &range) == XUI_OK);
      revision = xuiDocumentGetRevision(d);
      atomic_store(&a.entered, 0); atomic_store(&a.resume, 0); atomic_store(&a.gate, 1);
      CHECK(xuiDocumentEditorInsertText(editor, "X", 1) == XUI_OK &&
          xuiDocumentGetRevision(d) == revision && xuiDocumentHasPrepare(d));
      CHECK(xuiDocumentEditorFlush(editor) == XUI_DOC_ERROR_BUSY); async_editor_gate(&a.entered);
      CHECK(xuiDocumentEditorInsertText(editor, "!", 1) == XUI_OK &&
          async_editor_info(editor).iPatchCount == 2 &&
          xuiDocumentGetRevision(d) == revision);
      source(d, original);
      atomic_store(&a.resume, 1); async_editor_flush(context, editor);
      source(d, changed);
      CHECK(xuiDocumentAcquireSnapshot(d, &snapshot) == XUI_OK &&
          xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC, "X", 1,
              NULL, &range, 1, &found) == XUI_OK && found == 1);
      node.iSize = sizeof(node);
      CHECK(xuiDocumentSnapshotGetNode(snapshot, range.tAnchor.iNodeId, &node) == XUI_OK &&
          (node.tAttributes.iMarks & XUI_DOC_BOLD));
      xuiDocumentSnapshotRelease(snapshot);
      CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK); source(d, original);
      CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_REDO) == XUI_OK); source(d, changed);
      xuiWidgetDestroy(editor); xuiDocumentRelease(d);
    }
    { const char* original = "~~a &amp; b &amp; c~~\r\n\r\n[ref]: /keep\r\n";
      const char* changed = "~~a &amp; bZc~~\r\n\r\n[ref]: /keep\r\n";
      xui_doc_range_t left, right, range;
      xui_doc_node_info_t node = {0};
      create.iMarkdownDialect = XUI_MD_GFM;
      CHECK(xuiDocumentCreate(&create, &d) == XUI_OK &&
          xuiDocumentLoadMarkdown(d, original, strlen(original)) == XUI_OK &&
          xuiDocumentClearHistory(d) == XUI_OK);
      ed.tView.pDocument = d;
      CHECK(xuiDocumentEditorCreate(context, &ed, &editor) == XUI_OK &&
          xuiSetRootWidget(context, editor) == XUI_OK &&
          xuiWidgetSetRect(editor, (xui_rect_t){0, 0, 400, 260}) == XUI_OK);
      CHECK(xuiDocumentAcquireSnapshot(d, &snapshot) == XUI_OK &&
          xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC, "b", 1,
              NULL, &left, 1, &found) == XUI_OK && found == 1 &&
          xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC, "c", 1,
              NULL, &right, 1, &found) == XUI_OK && found == 1);
      xuiDocumentSnapshotRelease(snapshot);
      range.tAnchor = right.tAnchor; range.tCaret = left.tCaret;
      CHECK(xuiDocumentViewSetSelection(editor, &range) == XUI_OK);
      revision = xuiDocumentGetRevision(d);
      atomic_store(&a.entered, 0); atomic_store(&a.resume, 0); atomic_store(&a.gate, 1);
      CHECK(xuiDocumentEditorInsertText(editor, "Z", 1) == XUI_OK &&
          xuiDocumentGetRevision(d) == revision && xuiDocumentHasPrepare(d));
      CHECK(xuiDocumentEditorFlush(editor) == XUI_DOC_ERROR_BUSY); async_editor_gate(&a.entered);
      source(d, original);
      atomic_store(&a.resume, 1); async_editor_flush(context, editor);
      source(d, changed);
      CHECK(xuiDocumentAcquireSnapshot(d, &snapshot) == XUI_OK &&
          xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC, "Z", 1,
              NULL, &range, 1, &found) == XUI_OK && found == 1);
      node.iSize = sizeof(node);
      CHECK(xuiDocumentSnapshotGetNode(snapshot, range.tAnchor.iNodeId, &node) == XUI_OK &&
          (node.tAttributes.iMarks & XUI_DOC_STRIKE));
      xuiDocumentSnapshotRelease(snapshot);
      CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK); source(d, original);
      xuiWidgetDestroy(editor); xuiDocumentRelease(d);
      create.iMarkdownDialect = XUI_MD_COMMONMARK;
    }
    { const char* originals[] = {
          "*a &amp; b &amp; c*\n", "==a &amp; b &amp; c==\n",
          "~a &amp; b &amp; c~\n", "^a &amp; b &amp; c^\n",
          "[a &amp; b &amp; c](/dest)\n",
          "[a &amp; b &amp; c][ref]\n\n[ref]: /dest\n" };
      const char* changed[] = {
          "*a &amp; bZc*\n", "==a &amp; bZc==\n",
          "~a &amp; bZc~\n", "^a &amp; bZc^\n",
          "[a &amp; bZc](/dest)\n",
          "[a &amp; bZc][ref]\n\n[ref]: /dest\n" };
      const uint32_t marks[] = {
          XUI_DOC_ITALIC, XUI_DOC_HIGHLIGHT, XUI_DOC_SUBSCRIPT,
          XUI_DOC_SUPERSCRIPT, XUI_DOC_LINK, XUI_DOC_LINK };
      unsigned variant;
      create.iMarkdownDialect = XUI_MD_EXTENDED;
      for (variant = 0; variant < sizeof(marks) / sizeof(marks[0]); variant++) {
          xui_doc_range_t left, right, range;
          xui_doc_node_info_t node = {0};
          CHECK(xuiDocumentCreate(&create, &d) == XUI_OK &&
              xuiDocumentLoadMarkdown(d, originals[variant],
                  strlen(originals[variant])) == XUI_OK &&
              xuiDocumentClearHistory(d) == XUI_OK);
          ed.tView.pDocument = d;
          CHECK(xuiDocumentEditorCreate(context, &ed, &editor) == XUI_OK &&
              xuiSetRootWidget(context, editor) == XUI_OK &&
              xuiWidgetSetRect(editor, (xui_rect_t){0, 0, 400, 260}) == XUI_OK);
          CHECK(xuiDocumentAcquireSnapshot(d, &snapshot) == XUI_OK &&
              xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC, "b", 1,
                  NULL, &left, 1, &found) == XUI_OK && found == 1 &&
              xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC, "c", 1,
                  NULL, &right, 1, &found) == XUI_OK && found == 1);
          xuiDocumentSnapshotRelease(snapshot);
          range.tAnchor = left.tCaret; range.tCaret = right.tAnchor;
          CHECK(range.tAnchor.iNodeId != range.tCaret.iNodeId &&
              xuiDocumentViewSetSelection(editor, &range) == XUI_OK);
          revision = xuiDocumentGetRevision(d);
          atomic_store(&a.entered, 0); atomic_store(&a.resume, 0); atomic_store(&a.gate, 1);
          CHECK(xuiDocumentEditorInsertText(editor, "Z", 1) == XUI_OK &&
              xuiDocumentGetRevision(d) == revision && xuiDocumentHasPrepare(d));
          CHECK(xuiDocumentEditorFlush(editor) == XUI_DOC_ERROR_BUSY);
          async_editor_gate(&a.entered); source(d, originals[variant]);
          atomic_store(&a.resume, 1); async_editor_flush(context, editor);
          source(d, changed[variant]);
          CHECK(xuiDocumentAcquireSnapshot(d, &snapshot) == XUI_OK &&
              xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC, "Z", 1,
                  NULL, &range, 1, &found) == XUI_OK && found == 1);
          node.iSize = sizeof(node);
          CHECK(xuiDocumentSnapshotGetNode(snapshot, range.tAnchor.iNodeId, &node) == XUI_OK &&
              (node.tAttributes.iMarks & marks[variant]));
          xuiDocumentSnapshotRelease(snapshot);
          CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
          source(d, originals[variant]);
          xuiWidgetDestroy(editor); xuiDocumentRelease(d);
      }
      create.iMarkdownDialect = XUI_MD_COMMONMARK;
    }
    { const char* original = "**ab** **cd**\n";
      xui_doc_range_t left, right, range;
      int result;
      CHECK(xuiDocumentCreate(&create, &d) == XUI_OK &&
          xuiDocumentLoadMarkdown(d, original, strlen(original)) == XUI_OK &&
          xuiDocumentClearHistory(d) == XUI_OK);
      ed.tView.pDocument = d;
      CHECK(xuiDocumentEditorCreate(context, &ed, &editor) == XUI_OK &&
          xuiSetRootWidget(context, editor) == XUI_OK &&
          xuiWidgetSetRect(editor, (xui_rect_t){0, 0, 400, 260}) == XUI_OK);
      CHECK(xuiDocumentAcquireSnapshot(d, &snapshot) == XUI_OK &&
          xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC, "ab", 2,
              NULL, &left, 1, &found) == XUI_OK && found == 1 &&
          xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC, "cd", 2,
              NULL, &right, 1, &found) == XUI_OK && found == 1);
      xuiDocumentSnapshotRelease(snapshot);
      range.tAnchor = left.tAnchor; range.tAnchor.iOffset++;
      range.tCaret = right.tAnchor; range.tCaret.iOffset++;
      CHECK(xuiDocumentViewSetSelection(editor, &range) == XUI_OK);
      result = xuiDocumentEditorInsertText(editor, "X", 1);
      CHECK(result == XUI_OK && !xuiDocumentHasPrepare(d));
      CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK); source(d, original);
      xuiWidgetDestroy(editor); xuiDocumentRelease(d);
    }
    { const char* original = "abcde\nfghij\n";
      xui_doc_range_t first, last;
      CHECK(xuiDocumentCreate(&create, &d) == XUI_OK &&
          xuiDocumentLoadMarkdown(d, original, strlen(original)) == XUI_OK &&
          xuiDocumentClearHistory(d) == XUI_OK);
      ed.tView.pDocument = d;
      CHECK(xuiDocumentEditorCreate(context, &ed, &editor) == XUI_OK &&
          xuiSetRootWidget(context, editor) == XUI_OK &&
          xuiWidgetSetRect(editor, (xui_rect_t){0, 0, 400, 260}) == XUI_OK);
      CHECK(xuiDocumentAcquireSnapshot(d, &snapshot) == XUI_OK &&
          xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC, "abcde", 5,
              NULL, &first, 1, &found) == XUI_OK && found == 1 &&
          xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC, "fghij", 5,
              NULL, &last, 1, &found) == XUI_OK && found == 1);
      xuiDocumentSnapshotRelease(snapshot);
      selected.tAnchor = last.tAnchor; selected.tAnchor.iOffset = 2;
      selected.tCaret = first.tCaret; selected.tCaret.iOffset = 3;
      CHECK(xuiDocumentViewSetSelection(editor, &selected) == XUI_OK);
      revision = xuiDocumentGetRevision(d);
      CHECK(xuiDocumentEditorInsertText(editor, "", 0) == XUI_OK &&
          xuiDocumentGetRevision(d) == revision && xuiDocumentHasPrepare(d) &&
          !strcmp(xuiEditGetText(editor), "abchij\n"));
      atomic_store(&a.entered, 0); atomic_store(&a.resume, 0); atomic_store(&a.gate, 1);
      CHECK(xuiDocumentEditorFlush(editor) == XUI_DOC_ERROR_BUSY); async_editor_gate(&a.entered);
      source(d, original);
      atomic_store(&a.resume, 1); async_editor_flush(context, editor);
      source(d, "abchij\n");
      CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
      source(d, original);
      xuiWidgetDestroy(editor); xuiDocumentRelease(d);
    }
    { const char* original = "abcde\nfghij\n";
      xui_doc_range_t first, last;
      CHECK(xuiDocumentCreate(&create, &d) == XUI_OK &&
          xuiDocumentLoadMarkdown(d, original, strlen(original)) == XUI_OK &&
          xuiDocumentClearHistory(d) == XUI_OK);
      ed.tView.pDocument = d;
      CHECK(xuiDocumentEditorCreate(context, &ed, &editor) == XUI_OK &&
          xuiSetRootWidget(context, editor) == XUI_OK &&
          xuiWidgetSetRect(editor, (xui_rect_t){0, 0, 400, 260}) == XUI_OK);
      CHECK(xuiDocumentAcquireSnapshot(d, &snapshot) == XUI_OK &&
          xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC, "abcde", 5,
              NULL, &first, 1, &found) == XUI_OK && found == 1 &&
          xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC, "fghij", 5,
              NULL, &last, 1, &found) == XUI_OK && found == 1);
      xuiDocumentSnapshotRelease(snapshot);
      first.tAnchor.iOffset = 3; first.tCaret = last.tCaret; first.tCaret.iOffset = 2;
      CHECK(xuiDocumentViewSetSelection(editor, &first) == XUI_OK);
      revision = xuiDocumentGetRevision(d);
      send_text(context, editor, "*", 0, 0);
      CHECK(xuiDocumentGetRevision(d) == revision && xuiDocumentHasPrepare(d));
      atomic_store(&a.entered, 0); atomic_store(&a.resume, 0); atomic_store(&a.gate, 1);
      CHECK(xuiDocumentEditorFlush(editor) == XUI_DOC_ERROR_BUSY); async_editor_gate(&a.entered);
      send_text(context, editor, "Z", 0, 0);
      CHECK(async_editor_info(editor).iPatchCount == 2 &&
          !strcmp(xuiEditGetText(editor), "abc*Zhij\n"));
      source(d, original);
      atomic_store(&a.resume, 1); async_editor_flush(context, editor);
      source(d, "abc\\*Zhij\n");
      stats.iSize = sizeof(stats);
      CHECK(xuiDocumentGetStats(d, &stats) == XUI_OK && stats.iUndoCount == 1 &&
          xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
      source(d, original);
      xuiWidgetDestroy(editor); xuiDocumentRelease(d);
    }
    { const char* original = "abcde\n\nfghij\n\nlater\n";
      const char* changed = "abc\\*Z\\!hij\n\nlater\n";
      xui_doc_range_t first, last;
      CHECK(xuiDocumentCreate(&create, &d) == XUI_OK &&
          xuiDocumentLoadMarkdown(d, original, strlen(original)) == XUI_OK &&
          xuiDocumentClearHistory(d) == XUI_OK);
      ed.tView.pDocument = d;
      CHECK(xuiDocumentEditorCreate(context, &ed, &editor) == XUI_OK &&
          xuiSetRootWidget(context, editor) == XUI_OK &&
          xuiWidgetSetRect(editor, (xui_rect_t){0, 0, 400, 260}) == XUI_OK);
      CHECK(xuiDocumentAcquireSnapshot(d, &snapshot) == XUI_OK &&
          xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC, "abcde", 5,
              NULL, &first, 1, &found) == XUI_OK && found == 1 &&
          xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC, "fghij", 5,
              NULL, &last, 1, &found) == XUI_OK && found == 1);
      xuiDocumentSnapshotRelease(snapshot);
      first.tAnchor.iOffset = 3; first.tCaret = last.tCaret; first.tCaret.iOffset = 2;
      CHECK(xuiDocumentViewSetSelection(editor, &first) == XUI_OK);
      revision = xuiDocumentGetRevision(d);
      atomic_store(&a.entered, 0); atomic_store(&a.resume, 0); atomic_store(&a.gate, 1);
      CHECK(xuiDocumentEditorInsertText(editor, "*Z", 2) == XUI_OK &&
          xuiDocumentGetRevision(d) == revision && xuiDocumentHasPrepare(d) &&
          !strcmp(xuiEditGetText(editor), "abc*Zhij\nlater\n"));
      CHECK(xuiDocumentEditorFlush(editor) == XUI_DOC_ERROR_BUSY); async_editor_gate(&a.entered);
      CHECK(xuiDocumentEditorInsertText(editor, "!", 1) == XUI_OK &&
          async_editor_info(editor).iPatchCount == 2 &&
          !strcmp(xuiEditGetText(editor), "abc*Z!hij\nlater\n"));
      CHECK(xuiDocumentAcquireSnapshot(d, &snapshot) == XUI_OK &&
          xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC, "later", 5,
              NULL, &selected, 1, &found) == XUI_OK && found == 1);
      xuiDocumentSnapshotRelease(snapshot);
      CHECK(xuiDocumentViewSetSelection(editor, &selected) == XUI_OK);
      source(d, original);
      atomic_store(&a.resume, 1); async_editor_flush(context, editor);
      source(d, changed);
      CHECK(xuiEditCopy(editor) == XUI_OK && !strcmp(proxy.sClipboard, "later"));
      CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
      source(d, original);
      CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_REDO) == XUI_OK);
      source(d, changed);
      xuiWidgetDestroy(editor); xuiDocumentRelease(d);
    }
    { const char* original = "abcde\r\n\r\nfghij\r\n";
      xui_doc_range_t first, last;
      CHECK(xuiDocumentCreate(&create, &d) == XUI_OK &&
          xuiDocumentLoadMarkdown(d, original, strlen(original)) == XUI_OK &&
          xuiDocumentClearHistory(d) == XUI_OK);
      ed.tView.pDocument = d;
      CHECK(xuiDocumentEditorCreate(context, &ed, &editor) == XUI_OK &&
          xuiSetRootWidget(context, editor) == XUI_OK &&
          xuiWidgetSetRect(editor, (xui_rect_t){0, 0, 400, 260}) == XUI_OK);
      CHECK(xuiDocumentAcquireSnapshot(d, &snapshot) == XUI_OK &&
          xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC, "abcde", 5,
              NULL, &first, 1, &found) == XUI_OK && found == 1 &&
          xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC, "fghij", 5,
              NULL, &last, 1, &found) == XUI_OK && found == 1);
      xuiDocumentSnapshotRelease(snapshot);
      first.tAnchor.iOffset = 3; first.tCaret = last.tCaret; first.tCaret.iOffset = 2;
      CHECK(xuiDocumentViewSetSelection(editor, &first) == XUI_OK);
      revision = xuiDocumentGetRevision(d);
      CHECK(xuiDocumentEditorInsertText(editor, "Z", 1) == XUI_OK &&
          xuiDocumentGetRevision(d) == revision && xuiDocumentHasPrepare(d));
      async_editor_flush(context, editor);
      source(d, "abcZhij\r\n");
      CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
      source(d, original);
      xuiWidgetDestroy(editor); xuiDocumentRelease(d);
    }
    { const char* original = "abcde\n\n[ref]: /keep\n\nfghij\n";
      xui_doc_range_t first, last;
      CHECK(xuiDocumentCreate(&create, &d) == XUI_OK &&
          xuiDocumentLoadMarkdown(d, original, strlen(original)) == XUI_OK &&
          xuiDocumentClearHistory(d) == XUI_OK);
      ed.tView.pDocument = d;
      CHECK(xuiDocumentEditorCreate(context, &ed, &editor) == XUI_OK &&
          xuiSetRootWidget(context, editor) == XUI_OK &&
          xuiWidgetSetRect(editor, (xui_rect_t){0, 0, 400, 260}) == XUI_OK);
      CHECK(xuiDocumentAcquireSnapshot(d, &snapshot) == XUI_OK &&
          xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC, "abcde", 5,
              NULL, &first, 1, &found) == XUI_OK && found == 1 &&
          xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC, "fghij", 5,
              NULL, &last, 1, &found) == XUI_OK && found == 1);
      xuiDocumentSnapshotRelease(snapshot);
      first.tAnchor.iOffset = 3; first.tCaret = last.tCaret; first.tCaret.iOffset = 2;
      CHECK(xuiDocumentViewSetSelection(editor, &first) == XUI_OK);
      revision = xuiDocumentGetRevision(d);
      CHECK(xuiDocumentEditorInsertText(editor, "Z", 1) == XUI_OK &&
          xuiDocumentGetRevision(d) == revision + 1 && !xuiDocumentHasPrepare(d));
      CHECK(strstr(xuiEditGetText(editor), "abcZhij") != NULL);
      CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
      source(d, original);
      xuiWidgetDestroy(editor); xuiDocumentRelease(d);
    }
    { const char* original = "abcdefghijklmnopqrstuvwxyz\n";
      char plain[512] = "abcdefghijklmnopqrstuvwxyz", expected_source[1024];
      size_t length = 26, i, j, output;
      CHECK(xuiDocumentCreate(&create, &d) == XUI_OK &&
          xuiDocumentLoadMarkdown(d, original, strlen(original)) == XUI_OK &&
          xuiDocumentClearHistory(d) == XUI_OK);
      ed.tView.pDocument = d;
      CHECK(xuiDocumentEditorCreate(context, &ed, &editor) == XUI_OK &&
          xuiSetRootWidget(context, editor) == XUI_OK &&
          xuiWidgetSetRect(editor, (xui_rect_t){0, 0, 400, 260}) == XUI_OK);
      CHECK(xuiDocumentAcquireSnapshot(d, &snapshot) == XUI_OK &&
          xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC,
              "abcdefghijklmnopqrstuvwxyz", 26, NULL, &selected, 1, &found) == XUI_OK && found == 1);
      xuiDocumentSnapshotRelease(snapshot);
      selected.tAnchor.iOffset = selected.tCaret.iOffset = 13;
      CHECK(xuiDocumentViewSetSelection(editor, &selected) == XUI_OK &&
          xuiDocumentEditorInsertText(editor, "*", 1) == XUI_OK);
      memmove(plain + 14, plain + 13, length - 13 + 1);
      plain[13] = '*'; length++;
      atomic_store(&a.entered, 0); atomic_store(&a.resume, 0); atomic_store(&a.gate, 1);
      CHECK(xuiDocumentEditorFlush(editor) == XUI_DOC_ERROR_BUSY); async_editor_gate(&a.entered);
      for (i = 0; i < 128; i++) {
          const char* inserted = i % 4 == 0 ? "?" : i % 4 == 1 ? "*" :
              i % 4 == 2 ? "" : "\xe4\xb8\xad";
          uint64_t bytes_inserted = i % 4 == 2 ? 0 : i % 4 == 3 ? 3 : 1;
          uint64_t start = 1 + (i * 17) % (length - 2);
          uint64_t removed = i % 4 == 0 ? 0 : 1;
          while (start < length - 1 &&
              ((unsigned char)plain[start] & 0xc0) == 0x80) start++;
          if (removed && (unsigned char)plain[start] >= 0xe0) removed = 3;
          if (start + removed >= length) {
              start = 1;
              while (start < length - 1 && (unsigned char)plain[start] >= 0x80) start++;
              CHECK(start < length - 1);
              removed = i % 4 == 0 ? 0 : 1;
          }
          CHECK(xuiDocumentViewGetSelection(editor, &selected) == XUI_OK);
          selected.tAnchor.iOffset = i & 1 ? start + removed : start;
          selected.tCaret.iOffset = i & 1 ? start : start + removed;
          CHECK(xuiDocumentViewSetSelection(editor, &selected) == XUI_OK &&
              xuiDocumentEditorInsertText(editor, inserted, bytes_inserted) == XUI_OK);
          memmove(plain + start + bytes_inserted, plain + start + removed,
              length - start - removed + 1);
          memcpy(plain + start, inserted, (size_t)bytes_inserted);
          length = length - removed + bytes_inserted;
          plain[length] = '\n'; plain[length + 1] = 0;
          CHECK(!strcmp(xuiEditGetText(editor), plain) &&
              async_editor_info(editor).iPatchCount == i + 2);
          plain[length] = 0;
      }
      output = 0;
      for (j = 0; j < length; j++) {
          unsigned char c = (unsigned char)plain[j];
          if ((c >= 33 && c <= 47) || (c >= 58 && c <= 64) ||
              (c >= 91 && c <= 96) || (c >= 123 && c <= 126)) expected_source[output++] = '\\';
          expected_source[output++] = plain[j];
      }
      expected_source[output++] = '\n'; expected_source[output] = 0;
      source(d, original);
      atomic_store(&a.resume, 1); async_editor_flush(context, editor);
      source(d, expected_source);
      CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
      source(d, original);
      xuiWidgetDestroy(editor); xuiDocumentRelease(d);
    }
    xuiDestroy(context); proxy.tProxy.fontDestroy(&proxy.tProxy, font);
    CHECK(!atomic_load(&a.live) && !a.errors);
    puts("Document VISUAL pending same-leaf, cross-soft-break and cross-paragraph edits, shifted selection/caret, 128 mixed edits and Undo/Redo passed");
}
static void async_editor_visual_oom_sweep(void)
{
    xui_test_proxy_state_t proxy; xui_context context; xui_font font;
    xui_doc_desc_t create = {0}; xui_doc_editor_desc_t ed = {0};
    async_editor_allocator a; uint64_t allocations = 0, span_allocations = 0,
        block_allocations = 0,
        marked_allocations = 0, marked_leaf_allocations = 0, failure;
    atomic_init(&a.gate, 0); atomic_init(&a.entered, 0); atomic_init(&a.resume, 0);
    atomic_init(&a.fail, 0); atomic_init(&a.fail_owner, 0); atomic_init(&a.live, 0);
    atomic_init(&a.owner_calls, 0); atomic_init(&a.fail_owner_at, UINT64_MAX);
    a.owner = xrtThreadCurrentId(); a.errors = 0;
    xuiTestProxyInit(&proxy);
    CHECK(xuiCreate(&context) == XUI_OK && xuiSetProxy(context, &proxy.tProxy) == XUI_OK &&
        proxy.tProxy.fontLoadFile(&proxy.tProxy, &font, "test.ttf", 14, 0) == XUI_OK &&
        xuiSetDefaultFont(context, font) == XUI_OK);
    create.iSize = sizeof(create); create.iProfile = XUI_DOCUMENT_MARKDOWN;
    create.onAlloc = async_editor_alloc; create.onFree = async_editor_free; create.pAllocatorUser = &a;
    ed.iSize = sizeof(ed); ed.tView.iSize = sizeof(ed.tView);
    ed.iMode = XUI_DOC_VISUAL; ed.iAsyncSourceThresholdBytes = 1;
    for (failure = 0; failure <= allocations + 1 || failure < 2; failure++) {
        xui_document d; xui_widget editor; xui_document_snapshot snapshot;
        xui_doc_range_t selected; xui_doc_prepare_info_t pending;
        uint64_t found, before, calls; int result;
        CHECK(xuiDocumentCreate(&create, &d) == XUI_OK &&
            xuiDocumentLoadMarkdown(d, "abcdef\n", 7) == XUI_OK &&
            xuiDocumentClearHistory(d) == XUI_OK);
        ed.tView.pDocument = d;
        CHECK(xuiDocumentEditorCreate(context, &ed, &editor) == XUI_OK &&
            xuiSetRootWidget(context, editor) == XUI_OK &&
            xuiWidgetSetRect(editor, (xui_rect_t){0, 0, 400, 260}) == XUI_OK);
        CHECK(xuiDocumentAcquireSnapshot(d, &snapshot) == XUI_OK &&
            xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC, "abcdef", 6,
                NULL, &selected, 1, &found) == XUI_OK && found == 1);
        xuiDocumentSnapshotRelease(snapshot);
        selected.tAnchor.iOffset = selected.tCaret.iOffset = 3;
        CHECK(xuiDocumentViewSetSelection(editor, &selected) == XUI_OK &&
            xuiDocumentEditorInsertText(editor, "*Z", 2) == XUI_OK);
        atomic_store(&a.gate, 1); atomic_store(&a.entered, 0); atomic_store(&a.resume, 0);
        CHECK(xuiDocumentEditorFlush(editor) == XUI_DOC_ERROR_BUSY); async_editor_gate(&a.entered);
        CHECK(xuiDocumentViewGetSelection(editor, &selected) == XUI_OK);
        selected.tAnchor.iOffset = 3; selected.tCaret.iOffset = 4;
        CHECK(xuiDocumentViewSetSelection(editor, &selected) == XUI_OK);
        pending = async_editor_info(editor);
        before = atomic_load(&a.owner_calls);
        if (failure >= 2) atomic_store(&a.fail_owner_at, before + failure - 2);
        result = xuiDocumentEditorInsertText(editor, "?", 1);
        atomic_store(&a.fail_owner_at, UINT64_MAX);
        calls = atomic_load(&a.owner_calls) - before;
        if (failure < 2) {
            CHECK(result == XUI_OK && calls && calls < 512);
            if (failure == 1) allocations = calls;
        } else {
            CHECK(result == XUI_ERROR_OUT_OF_MEMORY && calls == failure - 1 &&
                async_editor_info(editor).iGeneration == pending.iGeneration &&
                !strcmp(xuiEditGetText(editor), "abc*Zdef\n"));
        }
        atomic_store(&a.resume, 1); async_editor_flush(context, editor);
        source(d, failure < 2 ? "abc\\?Zdef\n" : "abc\\*Zdef\n");
        xuiWidgetDestroy(editor); xuiDocumentRelease(d);
    }
    for (failure = 0; failure <= span_allocations + 1 || failure < 2; failure++) {
        const char* original = "abcde\nfghij\n";
        xui_document d; xui_widget editor; xui_document_snapshot snapshot;
        xui_doc_range_t first, last; uint64_t found, before, calls; int result;
        CHECK(xuiDocumentCreate(&create, &d) == XUI_OK &&
            xuiDocumentLoadMarkdown(d, original, strlen(original)) == XUI_OK &&
            xuiDocumentClearHistory(d) == XUI_OK);
        ed.tView.pDocument = d;
        CHECK(xuiDocumentEditorCreate(context, &ed, &editor) == XUI_OK &&
            xuiSetRootWidget(context, editor) == XUI_OK &&
            xuiWidgetSetRect(editor, (xui_rect_t){0, 0, 400, 260}) == XUI_OK);
        CHECK(xuiDocumentAcquireSnapshot(d, &snapshot) == XUI_OK &&
            xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC, "abcde", 5,
                NULL, &first, 1, &found) == XUI_OK && found == 1 &&
            xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC, "fghij", 5,
                NULL, &last, 1, &found) == XUI_OK && found == 1);
        xuiDocumentSnapshotRelease(snapshot);
        first.tAnchor.iOffset = 3; first.tCaret = last.tCaret; first.tCaret.iOffset = 2;
        CHECK(xuiDocumentViewSetSelection(editor, &first) == XUI_OK);
        before = atomic_load(&a.owner_calls);
        if (failure >= 2) atomic_store(&a.fail_owner_at, before + failure - 2);
        result = xuiDocumentEditorInsertText(editor, "*Z", 2);
        atomic_store(&a.fail_owner_at, UINT64_MAX);
        calls = atomic_load(&a.owner_calls) - before;
        if (failure < 2) {
            CHECK(result == XUI_OK && xuiDocumentHasPrepare(d) && calls && calls < 512);
            if (failure == 1) span_allocations = calls;
            CHECK(xuiDocumentEditorCancelInput(editor) == XUI_OK);
        } else {
            CHECK(result == XUI_ERROR_OUT_OF_MEMORY && calls == failure - 1 &&
                !xuiDocumentHasPrepare(d) &&
                !strcmp(xuiEditGetText(editor), "abcde\nfghij\n"));
        }
        source(d, original);
        xuiWidgetDestroy(editor); xuiDocumentRelease(d);
    }
    for (failure = 0; failure <= block_allocations + 1 || failure < 2; failure++) {
        const char* original = "abcde\n\nfghij\n\nlater\n";
        xui_document d; xui_widget editor; xui_document_snapshot snapshot;
        xui_doc_range_t first, last; uint64_t found, before, calls; int result;
        CHECK(xuiDocumentCreate(&create, &d) == XUI_OK &&
            xuiDocumentLoadMarkdown(d, original, strlen(original)) == XUI_OK &&
            xuiDocumentClearHistory(d) == XUI_OK);
        ed.tView.pDocument = d;
        CHECK(xuiDocumentEditorCreate(context, &ed, &editor) == XUI_OK &&
            xuiSetRootWidget(context, editor) == XUI_OK &&
            xuiWidgetSetRect(editor, (xui_rect_t){0, 0, 400, 260}) == XUI_OK);
        CHECK(xuiDocumentAcquireSnapshot(d, &snapshot) == XUI_OK &&
            xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC, "abcde", 5,
                NULL, &first, 1, &found) == XUI_OK && found == 1 &&
            xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC, "fghij", 5,
                NULL, &last, 1, &found) == XUI_OK && found == 1);
        xuiDocumentSnapshotRelease(snapshot);
        first.tAnchor.iOffset = 3; first.tCaret = last.tCaret; first.tCaret.iOffset = 2;
        CHECK(xuiDocumentViewSetSelection(editor, &first) == XUI_OK);
        before = atomic_load(&a.owner_calls);
        if (failure >= 2) atomic_store(&a.fail_owner_at, before + failure - 2);
        result = xuiDocumentEditorInsertText(editor, "*Z", 2);
        atomic_store(&a.fail_owner_at, UINT64_MAX);
        calls = atomic_load(&a.owner_calls) - before;
        if (failure < 2) {
            CHECK(result == XUI_OK && xuiDocumentHasPrepare(d) && calls && calls < 512);
            if (failure == 1) block_allocations = calls;
            CHECK(xuiDocumentEditorCancelInput(editor) == XUI_OK);
        } else {
            CHECK(result == XUI_ERROR_OUT_OF_MEMORY && calls == failure - 1 &&
                !xuiDocumentHasPrepare(d) &&
                !strcmp(xuiEditGetText(editor), "abcde\nfghij\nlater\n"));
        }
        source(d, original);
        xuiWidgetDestroy(editor); xuiDocumentRelease(d);
    }
    for (failure = 0; failure <= marked_allocations + 1 || failure < 2; failure++) {
        const char* original = "**a &amp; b &amp; c**\n\n[ref]: /keep\n";
        xui_document d; xui_widget editor; xui_document_snapshot snapshot;
        xui_doc_range_t left, right, range; uint64_t found, before, calls; int result;
        CHECK(xuiDocumentCreate(&create, &d) == XUI_OK &&
            xuiDocumentLoadMarkdown(d, original, strlen(original)) == XUI_OK &&
            xuiDocumentClearHistory(d) == XUI_OK);
        ed.tView.pDocument = d;
        CHECK(xuiDocumentEditorCreate(context, &ed, &editor) == XUI_OK &&
            xuiSetRootWidget(context, editor) == XUI_OK &&
            xuiWidgetSetRect(editor, (xui_rect_t){0, 0, 400, 260}) == XUI_OK);
        CHECK(xuiDocumentAcquireSnapshot(d, &snapshot) == XUI_OK &&
            xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC, "b", 1,
                NULL, &left, 1, &found) == XUI_OK && found == 1 &&
            xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC, "c", 1,
                NULL, &right, 1, &found) == XUI_OK && found == 1);
        xuiDocumentSnapshotRelease(snapshot);
        range.tAnchor = left.tCaret; range.tCaret = right.tAnchor;
        CHECK(xuiDocumentViewSetSelection(editor, &range) == XUI_OK);
        before = atomic_load(&a.owner_calls);
        if (failure >= 2) atomic_store(&a.fail_owner_at, before + failure - 2);
        result = xuiDocumentEditorInsertText(editor, "X", 1);
        atomic_store(&a.fail_owner_at, UINT64_MAX);
        calls = atomic_load(&a.owner_calls) - before;
        if (failure < 2) {
            CHECK(result == XUI_OK && xuiDocumentHasPrepare(d) && calls && calls < 512);
            if (failure == 1) marked_allocations = calls;
            CHECK(xuiDocumentEditorCancelInput(editor) == XUI_OK);
        } else {
            CHECK(result == XUI_ERROR_OUT_OF_MEMORY && calls == failure - 1 &&
                !xuiDocumentHasPrepare(d));
        }
        source(d, original);
        xuiWidgetDestroy(editor); xuiDocumentRelease(d);
    }
    for (failure = 0; failure <= marked_leaf_allocations + 1 || failure < 2; failure++) {
        const char* original = "**abcdef**\n\n[ref]: /keep\n";
        xui_document d; xui_widget editor; xui_document_snapshot snapshot;
        xui_doc_range_t selected; uint64_t found, before, calls; int result;
        CHECK(xuiDocumentCreate(&create, &d) == XUI_OK &&
            xuiDocumentLoadMarkdown(d, original, strlen(original)) == XUI_OK &&
            xuiDocumentClearHistory(d) == XUI_OK);
        ed.tView.pDocument = d;
        CHECK(xuiDocumentEditorCreate(context, &ed, &editor) == XUI_OK &&
            xuiSetRootWidget(context, editor) == XUI_OK &&
            xuiWidgetSetRect(editor, (xui_rect_t){0, 0, 400, 260}) == XUI_OK);
        CHECK(xuiDocumentAcquireSnapshot(d, &snapshot) == XUI_OK &&
            xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC, "abcdef", 6,
                NULL, &selected, 1, &found) == XUI_OK && found == 1);
        xuiDocumentSnapshotRelease(snapshot);
        selected.tAnchor.iOffset = selected.tCaret.iOffset = 3;
        CHECK(xuiDocumentViewSetSelection(editor, &selected) == XUI_OK);
        before = atomic_load(&a.owner_calls);
        if (failure >= 2) atomic_store(&a.fail_owner_at, before + failure - 2);
        result = xuiDocumentEditorInsertText(editor, "X", 1);
        atomic_store(&a.fail_owner_at, UINT64_MAX);
        calls = atomic_load(&a.owner_calls) - before;
        if (failure < 2) {
            CHECK(result == XUI_OK && xuiDocumentHasPrepare(d) && calls && calls < 512);
            if (failure == 1) marked_leaf_allocations = calls;
            CHECK(xuiDocumentEditorCancelInput(editor) == XUI_OK);
        } else {
            CHECK(result == XUI_ERROR_OUT_OF_MEMORY && calls == failure - 1 &&
                !xuiDocumentHasPrepare(d));
        }
        source(d, original);
        xuiWidgetDestroy(editor); xuiDocumentRelease(d);
    }
    xuiDestroy(context); proxy.tProxy.fontDestroy(&proxy.tProxy, font);
    CHECK(!atomic_load(&a.live) && !a.errors);
    printf("Document VISUAL same-leaf continuation and cross-soft-break/cross-paragraph/marked creation: %llu/%llu/%llu/%llu/%llu owner allocation-failure points preserved the old state.\n",
        (unsigned long long)allocations, (unsigned long long)span_allocations,
        (unsigned long long)block_allocations,
        (unsigned long long)marked_allocations,
        (unsigned long long)marked_leaf_allocations);
}
static int async_editor_order_time(const void* a, const void* b)
{
    double x = *(const double*)a, y = *(const double*)b; return x < y ? -1 : x != y;
}
/* VISUAL candidate creation and repeated input must stay on the owner thread
 * while parsing is blocked. This measures a real renderer draw, excluding GPU
 * execution and the later worker parse/publication. */
static void async_editor_visual_scale_cases(void)
{
    unsigned mib;
    for (mib = 1; mib <= 10; mib += 9) {
        xui_test_proxy_state_t proxy; xui_context context; xui_font font; xui_surface surface;
        xui_doc_desc_t desc = {0}; xui_doc_editor_desc_t ed = {0}; async_editor_allocator a;
        xui_document d; xui_document_snapshot snapshot; xui_widget editor;
        xui_doc_range_t selection = {0}; xui_doc_node_id paragraph, body;
        xui_doc_node_info_t info = {0}; xui_rect_i_t damage = {0, 0, 640, 400};
        size_t bytes = (size_t)mib * 1024 * 1024, blocks = bytes / 1024, at;
        char *original = malloc(bytes + 1), expected[69];
        double samples[64], first_ms; uint64_t revision, begin; unsigned i;
        CHECK(original);
        for (at = 0; at < bytes; at += 1024) {
            memset(original + at, 'a', 1022);
            memcpy(original + at + 500, "0123456789", 10);
            original[at + 1022] = original[at + 1023] = '\n';
        }
        original[bytes] = 0; memset(expected, 'x', sizeof(expected));
        atomic_init(&a.gate, 0); atomic_init(&a.entered, 0); atomic_init(&a.resume, 0);
        atomic_init(&a.fail, 0); atomic_init(&a.fail_owner, 0); atomic_init(&a.live, 0);
        atomic_init(&a.owner_calls, 0); atomic_init(&a.fail_owner_at, UINT64_MAX); a.owner = xrtThreadCurrentId(); a.errors = 0;
        xuiTestProxyInit(&proxy);
        CHECK(xuiCreate(&context) == XUI_OK && xuiSetProxy(context, &proxy.tProxy) == XUI_OK &&
            proxy.tProxy.fontLoadFile(&proxy.tProxy, &font, "test.ttf", 14, 0) == XUI_OK &&
            xuiSetDefaultFont(context, font) == XUI_OK &&
            xuiTestSurfaceCreate(&proxy, &surface, 640, 400, XUI_SURFACE_USAGE_TARGET) == XUI_OK);
        desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN;
        desc.onAlloc = async_editor_alloc; desc.onFree = async_editor_free; desc.pAllocatorUser = &a;
        desc.iHistoryMaxBytes = UINT64_C(256) * 1024 * 1024;
        CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK &&
            xuiDocumentLoadMarkdown(d, original, bytes) == XUI_OK &&
            xuiDocumentClearHistory(d) == XUI_OK);
        revision = xuiDocumentGetRevision(d);
        ed.iSize = sizeof(ed); ed.tView.iSize = sizeof(ed.tView); ed.tView.pDocument = d;
        ed.iMode = XUI_DOC_VISUAL; ed.onError = async_editor_error; ed.pUser = &a;
        CHECK(xuiDocumentEditorCreate(context, &ed, &editor) == XUI_OK &&
            xuiSetRootWidget(context, editor) == XUI_OK &&
            xuiWidgetSetRect(editor, (xui_rect_t){0, 0, 640, 400}) == XUI_OK &&
            xuiInputViewport(context, 640, 400) == XUI_OK &&
            xuiUpdate(context, .016f) == XUI_OK);
        CHECK(xuiDocumentAcquireSnapshot(d, &snapshot) == XUI_OK &&
            xuiDocumentSnapshotGetChild(snapshot, 1, blocks / 2, &paragraph) == XUI_OK &&
            xuiDocumentSnapshotGetChild(snapshot, paragraph, 0, &body) == XUI_OK);
        info.iSize = sizeof(info);
        CHECK(xuiDocumentSnapshotGetNode(snapshot, body, &info) == XUI_OK &&
            info.iKind == XUI_DOC_TEXT && info.iTextBytes == 1022);
        xuiDocumentSnapshotRelease(snapshot);
        selection.tAnchor.iSize = sizeof(selection.tAnchor);
        selection.tAnchor.iDocumentId = xuiDocumentGetIdentity(d);
        selection.tAnchor.iRevision = revision; selection.tAnchor.iNodeId = body;
        selection.tAnchor.iKind = XUI_DOC_POSITION_TEXT;
        selection.tAnchor.iOffset = 511; selection.tAnchor.iAffinity = XUI_DOC_AFTER;
        selection.tCaret = selection.tAnchor;
        CHECK(xuiDocumentViewSetSelection(editor, &selection) == XUI_OK);
        begin = xrtClock();
        CHECK(xuiDocumentEditorInsertText(editor, "x", 1) == XUI_OK);
        first_ms = (double)(xrtClock() - begin) / 1000.0;
        CHECK(async_editor_info(editor).iPatchCount == 1);
        atomic_store(&a.gate, 1);
        CHECK(xuiDocumentEditorFlush(editor) == XUI_DOC_ERROR_BUSY);
        async_editor_gate(&a.entered);
        for (i = 0; i < 68; i++) {
            begin = xrtClock();
            CHECK(xuiDocumentEditorInsertText(editor, "x", 1) == XUI_OK &&
                xuiUpdate(context, .016f) == XUI_OK &&
                xuiRender(context, surface, &damage, 1) == XUI_OK);
            if (i >= 4) samples[i - 4] = (double)(xrtClock() - begin) / 1000.0;
            CHECK(xuiDocumentGetRevision(d) == revision &&
                async_editor_info(editor).iSourceBytes == bytes + i + 2);
        }
        qsort(samples, 64, sizeof(*samples), async_editor_order_time);
        printf("VISUAL Editor %u MiB, frozen worker: initial input %.3f ms; 64 continued edits (input/update/proxy draw) P50 %.3f ms, P95 %.3f ms, max %.3f ms.\n",
            mib, first_ms, samples[32], samples[60], samples[63]);
        atomic_store(&a.resume, 1); async_editor_flush(context, editor);
        CHECK(xuiDocumentGetRevision(d) == revision + 1 &&
            xuiDocumentAcquireSnapshot(d, &snapshot) == XUI_OK);
        { char actual[69]; CHECK(xuiDocumentSnapshotReadSource(snapshot,
            (uint64_t)(blocks / 2) * 1024 + 511, actual, sizeof(actual)) == XUI_OK &&
            !memcmp(actual, expected, sizeof(actual))); }
        xuiDocumentSnapshotRelease(snapshot);
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK &&
            xuiDocumentAcquireSnapshot(d, &snapshot) == XUI_OK);
        { uint64_t length; CHECK(xuiDocumentSnapshotCopySource(snapshot, NULL, 0, &length) == XUI_OK &&
            length == bytes); }
        xuiDocumentSnapshotRelease(snapshot);
        CHECK(xuiDocumentClearHistory(d) == XUI_OK &&
            xuiDocumentAcquireSnapshot(d, &snapshot) == XUI_OK &&
            xuiDocumentSnapshotGetChild(snapshot, 1, blocks / 2, &paragraph) == XUI_OK &&
            xuiDocumentSnapshotGetChild(snapshot, paragraph, 0, &body) == XUI_OK);
        xuiDocumentSnapshotRelease(snapshot);
        revision = xuiDocumentGetRevision(d);
        selection.tAnchor.iRevision = selection.tCaret.iRevision = revision;
        selection.tAnchor.iNodeId = selection.tCaret.iNodeId = body;
        selection.tAnchor.iOffset = selection.tCaret.iOffset = 511;
        CHECK(xuiDocumentViewSetSelection(editor, &selection) == XUI_OK);
        begin = xrtClock();
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_BACKSPACE) == XUI_OK);
        first_ms = (double)(xrtClock() - begin) / 1000.0;
        atomic_store(&a.entered, 0); atomic_store(&a.resume, 0); atomic_store(&a.gate, 1);
        CHECK(xuiDocumentEditorFlush(editor) == XUI_DOC_ERROR_BUSY);
        async_editor_gate(&a.entered);
        for (i = 0; i < 68; i++) {
            begin = xrtClock();
            CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_BACKSPACE) == XUI_OK &&
                xuiUpdate(context, .016f) == XUI_OK &&
                xuiRender(context, surface, &damage, 1) == XUI_OK);
            if (i >= 4) samples[i - 4] = (double)(xrtClock() - begin) / 1000.0;
            CHECK(xuiDocumentGetRevision(d) == revision &&
                async_editor_info(editor).iSourceBytes == bytes - i - 2);
        }
        qsort(samples, 64, sizeof(*samples), async_editor_order_time);
        printf("VISUAL Editor %u MiB, frozen worker: first Backspace %.3f ms; 64 continued Backspaces (input/update/proxy draw) P50 %.3f ms, P95 %.3f ms, max %.3f ms.\n",
            mib, first_ms, samples[32], samples[60], samples[63]);
        atomic_store(&a.resume, 1); async_editor_flush(context, editor);
        CHECK(xuiDocumentAcquireSnapshot(d, &snapshot) == XUI_OK);
        { uint64_t length; char removed[10];
            CHECK(xuiDocumentSnapshotCopySource(snapshot, NULL, 0, &length) == XUI_OK &&
                length == bytes - 69 &&
                xuiDocumentSnapshotReadSource(snapshot, (uint64_t)(blocks / 2) * 1024 + 500,
                    removed, sizeof(removed)) == XUI_OK &&
                !memcmp(removed, "aaaaaaaaaa", sizeof(removed))); }
        xuiDocumentSnapshotRelease(snapshot);
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK &&
            xuiDocumentAcquireSnapshot(d, &snapshot) == XUI_OK);
        { char restored[10]; CHECK(xuiDocumentSnapshotReadSource(snapshot,
            (uint64_t)(blocks / 2) * 1024 + 500, restored, sizeof(restored)) == XUI_OK &&
            !memcmp(restored, "0123456789", sizeof(restored))); }
        xuiDocumentSnapshotRelease(snapshot);
        xuiWidgetDestroy(editor); xuiDocumentRelease(d);
        CHECK(!atomic_load(&a.live) && !a.errors);
        proxy.tProxy.surfaceDestroy(&proxy.tProxy, surface);
        xuiDestroy(context); proxy.tProxy.fontDestroy(&proxy.tProxy, font); free(original);
    }
}
/* Measures the LIVE main-thread path while the Markdown worker is held. It
 * includes input, view update and proxy draw, but not publication or GPU/OS
 * delivery latency. The result is diagnostic rather than a machine gate. */
static void async_editor_live_scale_cases(void)
{
    unsigned mib;
    for (mib = 1; mib <= 10; mib += 9) {
        xui_test_proxy_state_t proxy; xui_context context; xui_font font; xui_surface surface; xui_widget editor;
        xui_doc_desc_t desc = {0}; xui_doc_editor_desc_t ed = {0}; xui_document d; async_editor_allocator a;
        xui_doc_range_t range; xui_doc_renderer_stats_t before = {0}, after = {0};
        xui_rect_i_t damage = {0, 0, 640, 400};
        const char* prefix = "## Heading\n\nParagraph with **bold**, &amp; and [link](/target). ";
        char block[512], *original; size_t bytes = (size_t)mib * 1024 * 1024, at;
        double samples[64]; uint64_t begin, revision; unsigned i;
        atomic_init(&a.gate, 0); atomic_init(&a.entered, 0); atomic_init(&a.resume, 0);
        atomic_init(&a.fail, 0); atomic_init(&a.fail_owner, 0); atomic_init(&a.live, 0);
        atomic_init(&a.owner_calls, 0); atomic_init(&a.fail_owner_at, UINT64_MAX); a.owner = xrtThreadCurrentId(); a.errors = 0;
        original = malloc(bytes + 1); CHECK(original);
        memset(block, 'a', sizeof(block)); memcpy(block, prefix, strlen(prefix));
        block[510] = block[511] = '\n';
        for (at = 0; at < bytes; at += sizeof(block)) memcpy(original + at, block, sizeof(block));
        original[bytes] = 0;
        xuiTestProxyInit(&proxy);
        CHECK(xuiCreate(&context) == XUI_OK && xuiSetProxy(context, &proxy.tProxy) == XUI_OK &&
            proxy.tProxy.fontLoadFile(&proxy.tProxy, &font, "test.ttf", 14, 0) == XUI_OK &&
            xuiSetDefaultFont(context, font) == XUI_OK &&
            xuiTestSurfaceCreate(&proxy, &surface, 640, 400, XUI_SURFACE_USAGE_TARGET) == XUI_OK);
        desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN;
        desc.onAlloc = async_editor_alloc; desc.onFree = async_editor_free; desc.pAllocatorUser = &a;
        desc.iHistoryMaxBytes = UINT64_C(256) * 1024 * 1024;
        CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK &&
            xuiDocumentLoadMarkdown(d, original, bytes) == XUI_OK &&
            xuiDocumentClearHistory(d) == XUI_OK);
        revision = xuiDocumentGetRevision(d);
        ed.iSize = sizeof(ed); ed.tView.iSize = sizeof(ed.tView); ed.tView.pDocument = d;
        ed.iMode = XUI_DOC_LIVE_MARKDOWN; ed.iAsyncSourceThresholdBytes = 1;
        ed.onError = async_editor_error; ed.pUser = &a;
        CHECK(xuiDocumentEditorCreate(context, &ed, &editor) == XUI_OK &&
            xuiSetRootWidget(context, editor) == XUI_OK &&
            xuiWidgetSetRect(editor, (xui_rect_t){0, 0, 640, 400}) == XUI_OK &&
            xuiInputViewport(context, 640, 400) == XUI_OK &&
            xuiUpdate(context, .016f) == XUI_OK &&
            xuiRender(context, surface, &damage, 1) == XUI_OK);
        before.iSize = after.iSize = sizeof(before);
        CHECK(xuiDocumentViewGetRenderStats(editor, &before) == XUI_OK &&
            xuiEditSetSelection(editor, 3, 3) == XUI_OK &&
            xuiDocumentEditorInsertText(editor, "!", 1) == XUI_OK);
        atomic_store(&a.gate, 1);
        CHECK(xuiDocumentEditorFlush(editor) == XUI_DOC_ERROR_BUSY); async_editor_gate(&a.entered);
        for (i = 0; i < 68; i++) {
            CHECK(xuiDocumentViewGetSelection(editor, &range) == XUI_OK);
            range.tAnchor.iOffset = 3; range.tCaret.iOffset = 4;
            CHECK(xuiDocumentViewSetSelection(editor, &range) == XUI_OK);
            begin = xrtClock();
            CHECK(xuiDocumentEditorInsertText(editor, i & 1 ? "a" : "b", 1) == XUI_OK &&
                xuiUpdate(context, .016f) == XUI_OK &&
                xuiRender(context, surface, &damage, 1) == XUI_OK);
            if (i >= 4) samples[i - 4] = (double)(xrtClock() - begin) / 1000.0;
            CHECK(xuiDocumentGetRevision(d) == revision &&
                async_editor_info(editor).iSourceBytes == bytes + 1);
        }
        CHECK(xuiDocumentViewGetRenderStats(editor, &after) == XUI_OK &&
            !after.bLiveSourceFallback && after.iSourceBlocks < after.iBlocks &&
            after.iSourceIncrementalUpdates > before.iSourceIncrementalUpdates &&
            after.iSourceBytesScanned - before.iSourceBytesScanned < 100000);
        qsort(samples, 64, sizeof(*samples), async_editor_order_time);
        printf("LIVE Editor %u MiB, frozen worker, 64 pending edits (input/update/proxy draw): P50 %.3f ms, P95 %.3f ms, max %.3f ms; scanned %llu bytes.\n",
            mib, samples[32], samples[60], samples[63],
            (unsigned long long)(after.iSourceBytesScanned - before.iSourceBytesScanned));
        atomic_store(&a.resume, 1); async_editor_flush(context, editor);
        CHECK(xuiDocumentGetRevision(d) == revision + 1 &&
            xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
        {
            xui_document_snapshot snapshot; char* actual = malloc(bytes + 2); uint64_t length;
            CHECK(actual && xuiDocumentAcquireSnapshot(d, &snapshot) == XUI_OK &&
                xuiDocumentSnapshotCopySource(snapshot, actual, bytes + 2, &length) == XUI_OK &&
                length == bytes && !memcmp(actual, original, bytes));
            xuiDocumentSnapshotRelease(snapshot); free(actual);
        }
        xuiWidgetDestroy(editor); xuiDocumentRelease(d);
        CHECK(!atomic_load(&a.live) && !a.errors);
        proxy.tProxy.surfaceDestroy(&proxy.tProxy, surface);
        xuiDestroy(context); proxy.tProxy.fontDestroy(&proxy.tProxy, font); free(original);
    }
}
/* Timed work includes accepted input, layout/update, accessibility dispatch
 * and proxy rendering. A deliberately frozen parser makes this a repeatable
 * SOURCE pending-input sample, not a complete end-to-end/GPU latency claim. */
static void async_editor_scale_cases(void)
{
    unsigned mib;
    for (mib = 1; mib <= 10; mib += 9) {
        xui_test_proxy_state_t proxy; xui_context context; xui_font font; xui_surface surface; xui_widget editor;
        xui_doc_desc_t desc = {0}; xui_doc_editor_desc_t ed = {0}; xui_document d; async_editor_allocator a;
        xui_doc_range_t range; xui_doc_stats_t before = {0}, after = {0}; xui_rect_i_t damage = {0, 0, 640, 400};
        const char* prefix = "## Heading\n\nParagraph with **bold**, &amp; and [link](/target). ";
        char block[512], *original; size_t bytes = (size_t)mib * 1024 * 1024, at; unsigned i;
        double samples[64], navigation_samples[64]; uint64_t begin, revision;
        atomic_init(&a.gate, 0); atomic_init(&a.entered, 0); atomic_init(&a.resume, 0); atomic_init(&a.fail, 0);
        atomic_init(&a.fail_owner, 0); atomic_init(&a.live, 0); atomic_init(&a.owner_calls, 0); atomic_init(&a.fail_owner_at, UINT64_MAX); a.owner = xrtThreadCurrentId(); a.errors = 0;
        original = malloc(bytes + 1); CHECK(original); memset(block, 'a', sizeof(block));
        memcpy(block, prefix, strlen(prefix));
        block[510] = block[511] = '\n';
        for (at = 0; at < bytes; at += sizeof(block)) memcpy(original + at, block, sizeof(block));
        original[bytes] = 0;
        xuiTestProxyInit(&proxy); CHECK(xuiCreate(&context) == XUI_OK && xuiSetProxy(context, &proxy.tProxy) == XUI_OK);
        CHECK(proxy.tProxy.fontLoadFile(&proxy.tProxy, &font, "test.ttf", 14, 0) == XUI_OK && xuiSetDefaultFont(context, font) == XUI_OK);
        CHECK(xuiTestSurfaceCreate(&proxy, &surface, 640, 400, XUI_SURFACE_USAGE_TARGET) == XUI_OK);
        desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN;
        desc.onAlloc = async_editor_alloc; desc.onFree = async_editor_free; desc.pAllocatorUser = &a;
        /* A leading insertion shifts retained Markdown source metadata. The
         * 10 MiB history step exceeds the default 64 MiB cap; this scenario
         * explicitly reserves enough history to verify one shared Undo. */
        desc.iHistoryMaxBytes = UINT64_C(256) * 1024 * 1024;
        CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK && xuiDocumentLoadMarkdown(d, original, bytes) == XUI_OK);
        CHECK(xuiDocumentClearHistory(d) == XUI_OK); revision = xuiDocumentGetRevision(d);
        ed.iSize = sizeof(ed); ed.tView.iSize = sizeof(ed.tView); ed.tView.pDocument = d;
        ed.iMode = XUI_DOC_SOURCE_TEXT; ed.onError = async_editor_error; ed.pUser = &a;
        CHECK(xuiDocumentEditorCreate(context, &ed, &editor) == XUI_OK);
        CHECK(xuiSetRootWidget(context, editor) == XUI_OK && xuiWidgetSetRect(editor, (xui_rect_t){0, 0, 640, 400}) == XUI_OK);
        CHECK(xuiInputViewport(context, 640, 400) == XUI_OK && xuiUpdate(context, .016f) == XUI_OK);
        CHECK(xuiRender(context, surface, &damage, 1) == XUI_OK);
        {
            xui_doc_stats_t selection_before = {0}, selection_after = {0}; int first, last;
            selection_before.iSize = selection_after.iSize = sizeof(selection_before);
            CHECK(xuiDocumentGetStats(d, &selection_before) == XUI_OK);
            for (i = 0; i < 64; i++) {
                int at = (int)(bytes / 2 + 20 + i % 16);
                CHECK(xuiEditSetSelection(editor, at + 1, at) == XUI_OK);
                CHECK(xuiEditGetSelection(editor, &first, &last) == XUI_OK && first == at + 1 && last == at);
            }
            CHECK(xuiDocumentGetStats(d, &selection_after) == XUI_OK);
            CHECK(selection_after.iAllocations == selection_before.iAllocations &&
                selection_after.iLiveBytes == selection_before.iLiveBytes);
            printf("SOURCE Editor %u MiB, 64 compatibility selection round trips: no Document allocation or source projection.\n", mib);
        }
        CHECK(xuiDocumentViewGetSelection(editor, &range) == XUI_OK); range.tAnchor.iOffset = range.tCaret.iOffset = 3;
        CHECK(xuiDocumentViewSetSelection(editor, &range) == XUI_OK && xuiDocumentEditorInsertText(editor, "!", 1) == XUI_OK);
        atomic_store(&a.gate, 1); CHECK(xuiDocumentEditorFlush(editor) == XUI_DOC_ERROR_BUSY); async_editor_gate(&a.entered);
        {
            xui_doc_stats_t selection_before = {0}, selection_after = {0}; int first, last;
            selection_before.iSize = selection_after.iSize = sizeof(selection_before);
            CHECK(xuiDocumentGetStats(d, &selection_before) == XUI_OK);
            CHECK(xuiEditSetSelection(editor, (int)(bytes / 2 + 21), (int)(bytes / 2 + 20)) == XUI_OK);
            CHECK(xuiEditGetSelection(editor, &first, &last) == XUI_OK && first == (int)(bytes / 2 + 21) && last == (int)(bytes / 2 + 20));
            CHECK(xuiDocumentGetStats(d, &selection_after) == XUI_OK);
            CHECK(selection_after.iAllocations == selection_before.iAllocations &&
                selection_after.iLiveBytes == selection_before.iLiveBytes);
        }
        before.iSize = after.iSize = sizeof(before); CHECK(xuiDocumentGetStats(d, &before) == XUI_OK);
        for (i = 0; i < 68; i++) {
            CHECK(xuiDocumentViewGetSelection(editor, &range) == XUI_OK); range.tAnchor.iOffset = 3; range.tCaret.iOffset = 4;
            CHECK(xuiDocumentViewSetSelection(editor, &range) == XUI_OK);
            begin = xrtClock();
            CHECK(xuiDocumentEditorInsertText(editor, i & 1 ? "a" : "b", 1) == XUI_OK);
            CHECK(xuiUpdate(context, .016f) == XUI_OK && xuiRender(context, surface, &damage, 1) == XUI_OK);
            if (i >= 4) samples[i - 4] = (double)(xrtClock() - begin) / 1000.0;
            CHECK(xuiDocumentGetRevision(d) == revision && async_editor_info(editor).iSourceBytes == bytes + 1);
        }
        CHECK(xuiDocumentGetStats(d, &after) == XUI_OK && after.iMarkdownParses == before.iMarkdownParses);
        qsort(samples, 64, sizeof(*samples), async_editor_order_time);
        atomic_store(&a.resume, 1); async_editor_flush(context, editor);
        CHECK(xuiDocumentGetStats(d, &after) == XUI_OK);
        printf("SOURCE Editor %u MiB publication: revision %llu -> %llu, %u Undo, %llu history bytes / %llu cap.\n", mib,
            (unsigned long long)revision, (unsigned long long)xuiDocumentGetRevision(d), (unsigned)after.iUndoCount,
            (unsigned long long)after.iHistoryBytes, (unsigned long long)after.iHistoryMaxBytes);
        CHECK(xuiDocumentGetRevision(d) == revision + 1 && after.iUndoCount == 1);
        CHECK(xuiDocumentViewGetSelection(editor, &range) == XUI_OK && range.tCaret.iOffset == 4 && !range.tCaret.iInputGeneration);
        {
            xui_document_snapshot snapshot; char* actual = malloc(bytes + 2); uint64_t length; CHECK(actual);
            CHECK(xuiDocumentAcquireSnapshot(d, &snapshot) == XUI_OK);
            CHECK(xuiDocumentSnapshotCopySource(snapshot, actual, bytes + 2, &length) == XUI_OK && length == bytes + 1);
            CHECK(!memcmp(actual, original, 3) && actual[3] == 'a' && !memcmp(actual + 4, original + 3, bytes - 3));
            xuiDocumentSnapshotRelease(snapshot);
            {
                xui_event_t event = {0}; size_t caret = bytes / 2 + 21;
                event.iSize = sizeof(event); event.iType = XUI_EVENT_KEY_DOWN; event.pTarget = editor;
                CHECK(xuiDocumentViewGetSelection(editor, &range) == XUI_OK);
                range.tAnchor.iOffset = range.tCaret.iOffset = caret;
                CHECK(xuiDocumentViewSetSelection(editor, &range) == XUI_OK);
                for (i = 0; i < 68; i++) {
                    begin = xrtClock();
                    event.iKey = XUI_KEY_LEFT; CHECK(xuiDispatchEvent(context, &event) == XUI_OK);
                    event.iKey = XUI_KEY_RIGHT; CHECK(xuiDispatchEvent(context, &event) == XUI_OK);
                    if (i >= 4) navigation_samples[i - 4] = (double)(xrtClock() - begin) / 1000.0;
                    CHECK(xuiDocumentViewGetSelection(editor, &range) == XUI_OK && range.tCaret.iOffset == caret);
                }
                qsort(navigation_samples, 64, sizeof(*navigation_samples), async_editor_order_time);
                printf("SOURCE Editor %u MiB, 64 Left+Right pairs near midpoint: P50 %.3f ms, P95 %.3f ms, max %.3f ms.\n",
                    mib, navigation_samples[32], navigation_samples[60], navigation_samples[63]);
            }
            CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK && xuiDocumentAcquireSnapshot(d, &snapshot) == XUI_OK);
            CHECK(xuiDocumentSnapshotCopySource(snapshot, actual, bytes + 2, &length) == XUI_OK && length == bytes && !memcmp(actual, original, bytes));
            xuiDocumentSnapshotRelease(snapshot); free(actual);
        }
        xuiWidgetDestroy(editor); xuiDocumentRelease(d); CHECK(!atomic_load(&a.live) && !a.errors);
        proxy.tProxy.surfaceDestroy(&proxy.tProxy, surface); xuiDestroy(context); proxy.tProxy.fontDestroy(&proxy.tProxy, font); free(original);
        printf("SOURCE Editor %u MiB, 64 programmatic pending edits after warmup with worker frozen (input/update/proxy draw): P50 %.3f ms, P95 %.3f ms, max %.3f ms; one publication/Undo and allocation balance passed.\n",
            mib, samples[32], samples[60], samples[63]);
    }
}
