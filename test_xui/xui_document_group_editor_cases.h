static unsigned group_undo_count(xui_document d)
{
    xui_doc_stats_t stats = {0}; stats.iSize = sizeof(stats);
    CHECK(xuiDocumentGetStats(d, &stats) == XUI_OK); return (unsigned)stats.iUndoCount;
}
static void group_content(xui_document d, const char* text)
{
    if (xuiDocumentGetProfile(d) == XUI_DOCUMENT_MARKDOWN) source(d, text);
    else { char expected[256]; snprintf(expected, sizeof(expected), "%s\n", text); plain(d, expected); }
}
static void group_selection(xui_widget editor, int first, int last)
{
    int a, b; CHECK(xuiEditGetSelection(editor, &a, &b) == XUI_OK && a == first && b == last);
}
static void group_reset(xui_context context, xui_widget editor, xui_document d)
{
    CHECK(xuiDocumentEditorCancelInput(editor) == XUI_OK);
    if (xuiDocumentGetProfile(d) == XUI_DOCUMENT_MARKDOWN) CHECK(xuiDocumentLoadMarkdown(d, "base", 4) == XUI_OK);
    else CHECK(xuiEditSetText(editor, "base") == XUI_OK);
    async_editor_flush(context, editor);
    CHECK(xuiDocumentClearHistory(d) == XUI_OK && xuiEditSetSelection(editor, 4, 4) == XUI_OK);
}
static void group_typed(xui_context context, xui_widget editor, const char* text)
{
    send_text(context, editor, text, 0, 0); async_editor_flush(context, editor);
}
static void group_fail_after_publish(xui_document d, xui_document_change_set change, void* user)
{
    async_editor_allocator* allocator = user; (void)d; (void)change;
    atomic_store(&allocator->fail_owner, 1);
}
static void group_editor_cases(void)
{
    unsigned mode;
    for (mode = 0; mode < 5; mode++) {
        xui_test_proxy_state_t proxy; xui_context context; xui_font font; xui_widget editor; xui_document d;
        xui_doc_desc_t desc = {0}; xui_doc_editor_desc_t ed = {0}; xui_document_snapshot saved;
        async_editor_allocator allocator; unsigned i;
        atomic_init(&allocator.gate, 0); atomic_init(&allocator.entered, 0); atomic_init(&allocator.resume, 0);
        atomic_init(&allocator.fail, 0); atomic_init(&allocator.fail_owner, 0); atomic_init(&allocator.live, 0);
        atomic_init(&allocator.owner_calls, 0); atomic_init(&allocator.fail_owner_at, UINT64_MAX);
        allocator.owner = xrtThreadCurrentId(); allocator.errors = 0;
        xuiTestProxyInit(&proxy); CHECK(xuiCreate(&context) == XUI_OK && xuiSetProxy(context, &proxy.tProxy) == XUI_OK);
        CHECK(proxy.tProxy.fontLoadFile(&proxy.tProxy, &font, "test.ttf", 14, 0) == XUI_OK && xuiSetDefaultFont(context, font) == XUI_OK);
        desc.iSize = sizeof(desc); desc.iProfile = mode ? XUI_DOCUMENT_MARKDOWN : XUI_DOCUMENT_RICH;
        desc.onAlloc = async_editor_alloc; desc.onFree = async_editor_free; desc.pAllocatorUser = &allocator;
        CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK);
        if (mode) CHECK(xuiDocumentLoadMarkdown(d, "base", 4) == XUI_OK);
        ed.iSize = sizeof(ed); ed.tView.iSize = sizeof(ed.tView); ed.tView.pDocument = d;
        ed.iMode = mode == 1 || mode == 4 ? XUI_DOC_SOURCE_TEXT : mode == 2 ? XUI_DOC_LIVE_MARKDOWN : XUI_DOC_VISUAL;
        ed.iAsyncSourceThresholdBytes = mode == 4 ? 1 : UINT64_MAX; ed.onError = async_editor_error; ed.pUser = &allocator;
        CHECK(xuiDocumentEditorCreate(context, &ed, &editor) == XUI_OK && xuiSetRootWidget(context, editor) == XUI_OK);
        CHECK(xuiWidgetSetRect(editor, (xui_rect_t){0, 0, 400, 260}) == XUI_OK && xuiInputViewport(context, 400, 260) == XUI_OK);
        if (!mode) CHECK(xuiDocumentEditorInsertText(editor, "base", 4) == XUI_OK);
        CHECK(xuiDocumentClearHistory(d) == XUI_OK && xuiEditSetSelection(editor, 4, 4) == XUI_OK);

        group_typed(context, editor, "a"); group_typed(context, editor, "b"); group_typed(context, editor, "c");
        group_content(d, "baseabc"); CHECK(group_undo_count(d) == 1);
        CHECK(xuiEditUndo(editor) == XUI_OK); group_content(d, "base"); group_selection(editor, 4, 4);
        CHECK(xuiEditRedo(editor) == XUI_OK); group_content(d, "baseabc"); group_selection(editor, 7, 7);
        for (i = 0; i < 3; i++) { ordered_key(context, editor, XUI_KEY_BACKSPACE, 0); async_editor_flush(context, editor); }
        group_content(d, "base"); CHECK(group_undo_count(d) == 2);
        CHECK(xuiEditUndo(editor) == XUI_OK); group_content(d, "baseabc"); group_selection(editor, 7, 7);
        group_typed(context, editor, "X"); CHECK(group_undo_count(d) == 2 && !xuiDocumentCanRedo(d));
        CHECK(xuiEditUndo(editor) == XUI_OK); group_content(d, "baseabc");
        CHECK(xuiEditSetSelection(editor, 3, 4) == XUI_OK);
        group_typed(context, editor, "Q"); group_typed(context, editor, "Y"); group_content(d, "basQYabc");
        CHECK(xuiEditUndo(editor) == XUI_OK); group_content(d, "baseabc"); group_selection(editor, 3, 4);
        CHECK(xuiEditRedo(editor) == XUI_OK); group_content(d, "basQYabc"); group_selection(editor, 5, 5);

        /* Pending navigation keeps the displayed caret during publication;
         * Redo of the old group restores its last insertion caret instead. */
        group_reset(context, editor, d);
        if (mode == 4) { atomic_store(&allocator.entered, 0); atomic_store(&allocator.resume, 0); atomic_store(&allocator.gate, 1); }
        send_text(context, editor, "A", 0, 0);
        if (mode == 4) { CHECK(xuiDocumentEditorFlush(editor) == XUI_DOC_ERROR_BUSY); async_editor_gate(&allocator.entered); }
        send_text(context, editor, "B", 0, 0); ordered_key(context, editor, XUI_KEY_LEFT, 0); send_text(context, editor, "Y", 0, 0);
        if (mode == 4) {
            CHECK(ordered_work(editor).iQueuedEvents == 1 && !strcmp(xuiEditGetText(editor), "baseAB"));
            group_selection(editor, 5, 5); atomic_store(&allocator.resume, 1);
        }
        async_editor_flush(context, editor); group_content(d, "baseAYB"); CHECK(group_undo_count(d) == 2);
        CHECK(xuiEditUndo(editor) == XUI_OK); group_content(d, "baseAB"); group_selection(editor, 5, 5);
        CHECK(xuiEditUndo(editor) == XUI_OK); group_content(d, "base"); group_selection(editor, 4, 4);
        CHECK(xuiEditRedo(editor) == XUI_OK); group_content(d, "baseAB"); group_selection(editor, 6, 6);
        CHECK(xuiEditRedo(editor) == XUI_OK); group_content(d, "baseAYB"); group_selection(editor, 6, 6);

        group_reset(context, editor, d); group_typed(context, editor, "A"); group_typed(context, editor, "B");
        CHECK(xuiTestProxySetClipboardText(&proxy, "paste") == XUI_OK);
        ordered_key(context, editor, 'V', XUI_MOD_CTRL); async_editor_flush(context, editor); group_typed(context, editor, "C");
        group_content(d, "baseABpasteC"); CHECK(group_undo_count(d) == 3);
        CHECK(xuiEditUndo(editor) == XUI_OK); group_content(d, "baseABpaste");
        CHECK(xuiEditUndo(editor) == XUI_OK); group_content(d, "baseAB");
        CHECK(xuiEditUndo(editor) == XUI_OK); group_content(d, "base");

        group_reset(context, editor, d); group_typed(context, editor, "a");
        ordered_key(context, editor, XUI_KEY_ENTER, 0); async_editor_flush(context, editor); group_typed(context, editor, "b");
        CHECK(group_undo_count(d) == 3 && xuiEditUndo(editor) == XUI_OK && xuiEditUndo(editor) == XUI_OK);
        group_content(d, "basea"); CHECK(xuiEditUndo(editor) == XUI_OK); group_content(d, "base");
        group_reset(context, editor, d); group_typed(context, editor, "a");
        ordered_key(context, editor, 'A', XUI_MOD_CTRL); ordered_key(context, editor, 'X', XUI_MOD_CTRL);
        async_editor_flush(context, editor); group_typed(context, editor, "b");
        CHECK(group_undo_count(d) == 3 && xuiEditUndo(editor) == XUI_OK && xuiEditUndo(editor) == XUI_OK);
        group_content(d, "basea"); CHECK(xuiEditUndo(editor) == XUI_OK); group_content(d, "base");

        /* A saved state stays reachable inside the coalescing interval. */
        group_reset(context, editor, d); group_typed(context, editor, "a");
        CHECK(xuiDocumentAcquireSnapshot(d, &saved) == XUI_OK && xuiDocumentMarkSaved(d, saved) == XUI_OK);
        xuiDocumentSnapshotRelease(saved); group_typed(context, editor, "b"); CHECK(group_undo_count(d) == 2);
        CHECK(xuiEditUndo(editor) == XUI_OK && !xuiDocumentIsDirty(d)); group_content(d, "basea");

        group_reset(context, editor, d); group_typed(context, editor, "a");
        send_text(context, editor, "\xe4\xb8\xad", 1, 1); send_text(context, editor, "\xe4\xb8\xad", 1, 0);
        async_editor_flush(context, editor); group_typed(context, editor, "b");
        group_content(d, "basea\xe4\xb8\xad" "b"); CHECK(group_undo_count(d) == 3);
        CHECK(xuiEditUndo(editor) == XUI_OK); group_content(d, "basea\xe4\xb8\xad");
        CHECK(xuiEditUndo(editor) == XUI_OK); group_content(d, "basea");
        CHECK(xuiEditUndo(editor) == XUI_OK); group_content(d, "base");

        group_reset(context, editor, d);
        group_typed(context, editor, "e\xcc\x81"); group_typed(context, editor, "\xf0\x9f\x98\x80");
        CHECK(group_undo_count(d) == 1); group_selection(editor, 11, 11);
        ordered_key(context, editor, XUI_KEY_BACKSPACE, 0); async_editor_flush(context, editor);
        ordered_key(context, editor, XUI_KEY_BACKSPACE, 0); async_editor_flush(context, editor);
        group_content(d, "base"); CHECK(group_undo_count(d) == 2);
        CHECK(xuiEditUndo(editor) == XUI_OK); group_selection(editor, 11, 11);
        CHECK(xuiEditUndo(editor) == XUI_OK); group_content(d, "base");

        group_reset(context, editor, d); CHECK(xuiEditSetSelection(editor, 1, 1) == XUI_OK);
        ordered_key(context, editor, XUI_KEY_DELETE, 0); async_editor_flush(context, editor);
        ordered_key(context, editor, XUI_KEY_DELETE, 0); async_editor_flush(context, editor); group_content(d, "be");
        ordered_key(context, editor, XUI_KEY_BACKSPACE, 0); async_editor_flush(context, editor); group_content(d, "e");
        CHECK(group_undo_count(d) == 2 && xuiEditUndo(editor) == XUI_OK); group_content(d, "be"); group_selection(editor, 1, 1);
        CHECK(xuiEditUndo(editor) == XUI_OK); group_content(d, "base"); group_selection(editor, 1, 1);

        group_reset(context, editor, d); CHECK(xuiSetFocusWidget(context, editor) == XUI_OK); group_typed(context, editor, "a");
        CHECK(xuiSetFocusWidget(context, NULL) == XUI_OK && xuiSetFocusWidget(context, editor) == XUI_OK);
        CHECK(xuiDispatchPendingEvents(context) == XUI_OK); /* Deliver queued Blur before directly dispatched text. */
        group_typed(context, editor, "b");
        if (group_undo_count(d) != 2) fprintf(stderr, "group blur mode=%u undo=%u focus=%d\n", mode, group_undo_count(d), xuiGetFocusWidget(context) == editor);
        CHECK(group_undo_count(d) == 2);
        CHECK(xuiEditUndo(editor) == XUI_OK); group_content(d, "basea");
        if (mode) {
            group_reset(context, editor, d); group_typed(context, editor, "a");
            CHECK(xuiDocumentViewSetMode(editor, ed.iMode == XUI_DOC_SOURCE_TEXT ? XUI_DOC_LIVE_MARKDOWN : XUI_DOC_SOURCE_TEXT) == XUI_OK);
            group_typed(context, editor, "b"); CHECK(group_undo_count(d) == 2 && xuiEditUndo(editor) == XUI_OK);
            group_content(d, "basea"); CHECK(xuiDocumentViewSetMode(editor, ed.iMode) == XUI_OK);
        }
        if (!mode || mode == 3) {
            group_reset(context, editor, d); group_typed(context, editor, "a");
            ordered_key(context, editor, 'B', XUI_MOD_CTRL); group_typed(context, editor, "b");
            plain(d, "baseab\n"); CHECK(group_undo_count(d) == 2 && xuiEditUndo(editor) == XUI_OK); group_content(d, "basea");
        }

        /* Explicit API calls keep their programmatic transaction semantics. */
        group_reset(context, editor, d);
        CHECK(xuiDocumentEditorInsertText(editor, "a", 1) == XUI_OK); async_editor_flush(context, editor);
        CHECK(xuiDocumentEditorInsertText(editor, "b", 1) == XUI_OK); async_editor_flush(context, editor);
        CHECK(group_undo_count(d) == 2); group_content(d, "baseab");
        group_reset(context, editor, d); CHECK(xuiDocumentSetHistoryLimits(d, 1, UINT64_C(64) * 1024 * 1024) == XUI_OK);
        for (i = 0; i < 16; i++) group_typed(context, editor, "x");
        CHECK(group_undo_count(d) == 1 && xuiEditUndo(editor) == XUI_OK); group_content(d, "base"); group_selection(editor, 4, 4);
        xuiWidgetDestroy(editor); xuiDocumentRelease(d); CHECK(!atomic_load(&allocator.live) && !allocator.errors);
        xuiDestroy(context); proxy.tProxy.fontDestroy(&proxy.tProxy, font);
    }
    puts("Document Editor input groups: rich/SOURCE/LIVE/VISUAL, cross-publication typing/deletion, navigation/paste/IME/save boundaries, programmatic batches and Undo/Redo selections passed.");
}
static void group_editor_boundary_cases(void)
{
    xui_test_proxy_state_t proxy; xui_context context; xui_font font; xui_widget editor; xui_document d;
    xui_doc_desc_t desc = {0}; xui_doc_editor_desc_t ed = {0}; async_editor_allocator allocator; uint64_t token, started;
    atomic_init(&allocator.gate, 0); atomic_init(&allocator.entered, 0); atomic_init(&allocator.resume, 0);
    atomic_init(&allocator.fail, 0); atomic_init(&allocator.fail_owner, 0); atomic_init(&allocator.live, 0);
    atomic_init(&allocator.owner_calls, 0); atomic_init(&allocator.fail_owner_at, UINT64_MAX);
    allocator.owner = xrtThreadCurrentId(); allocator.errors = 0;
    xuiTestProxyInit(&proxy); CHECK(xuiCreate(&context) == XUI_OK && xuiSetProxy(context, &proxy.tProxy) == XUI_OK);
    CHECK(proxy.tProxy.fontLoadFile(&proxy.tProxy, &font, "test.ttf", 14, 0) == XUI_OK && xuiSetDefaultFont(context, font) == XUI_OK);
    desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN;
    desc.onAlloc = async_editor_alloc; desc.onFree = async_editor_free; desc.pAllocatorUser = &allocator;
    CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK && xuiDocumentLoadMarkdown(d, "base", 4) == XUI_OK);
    ed.iSize = sizeof(ed); ed.tView.iSize = sizeof(ed.tView); ed.tView.pDocument = d;
    ed.iMode = XUI_DOC_SOURCE_TEXT; ed.iAsyncSourceThresholdBytes = 1; ed.iUndoGroupTimeoutMs = 20;
    ed.onError = async_editor_error; ed.pUser = &allocator;
    CHECK(xuiDocumentEditorCreate(context, &ed, &editor) == XUI_OK && xuiSetRootWidget(context, editor) == XUI_OK);
    CHECK(xuiWidgetSetRect(editor, (xui_rect_t){0, 0, 400, 260}) == XUI_OK && xuiInputViewport(context, 400, 260) == XUI_OK);
    group_reset(context, editor, d); group_typed(context, editor, "a");
    started = xrtClock(); while (xrtClock() - started < 25000) xrtThreadYield();
    group_typed(context, editor, "b"); CHECK(group_undo_count(d) == 2);
    CHECK(xuiEditUndo(editor) == XUI_OK); source(d, "basea");

    /* Replaying three old events in one frame must retain the timeout gap
     * between their arrivals. B/C still belong to one undo unit. */
    group_reset(context, editor, d); ordered_pause(&allocator, editor); ordered_key(context, editor, 'Z', XUI_MOD_CTRL);
    send_text(context, editor, "A", 0, 0);
    started = xrtClock(); while (xrtClock() - started < 25000) xrtThreadYield();
    send_text(context, editor, "B", 0, 0); send_text(context, editor, "C", 0, 0);
    atomic_store(&allocator.resume, 1); async_editor_flush(context, editor); source(d, "baseABC");
    CHECK(group_undo_count(d) == 2 && xuiEditUndo(editor) == XUI_OK); source(d, "baseA");
    CHECK(xuiEditRedo(editor) == XUI_OK); source(d, "baseABC");

    /* Publish the old group, then fail creation of a distinct IME group.
     * Retry must keep the IME replacement range, not the old input generation
     * or the current caret. No accepted character may be lost. */
    group_reset(context, editor, d);
    atomic_store(&allocator.entered, 0); atomic_store(&allocator.resume, 0); atomic_store(&allocator.gate, 1);
    send_text(context, editor, "A", 0, 0); CHECK(xuiDocumentEditorFlush(editor) == XUI_DOC_ERROR_BUSY); async_editor_gate(&allocator.entered);
    {
        xui_event_t event = {0}; event.iSize = sizeof(event); event.pTarget = editor; event.iType = XUI_EVENT_IME_COMPOSITION;
        strcpy(event.sText, "\xe6\x96\x87"); event.iTextSize = event.iCompositionCursor = 3; event.bCompositionActive = 1;
        event.bCompositionReplacementRange = 1; event.iCompositionReplacementStart = 1; event.iCompositionReplacementEnd = 3;
        CHECK(xuiDispatchEvent(context, &event) == XUI_OK); send_text(context, editor, "\xe6\x96\x87", 1, 0);
    }
    CHECK(ordered_work(editor).iQueuedEvents == 1 && xuiDocumentEditorIsComposing(editor));
    CHECK(xuiDocumentSubscribe(d, group_fail_after_publish, &allocator, &token) == XUI_OK);
    atomic_store(&allocator.resume, 1); ordered_wait_ready(editor);
    { int result = xuiDocumentEditorFlush(editor);
      if (result != XUI_ERROR_OUT_OF_MEMORY) fprintf(stderr, "IME group boundary Flush=%d queued=%llu input=%d composing=%d errors=%u\n", result,
          (unsigned long long)ordered_work(editor).iQueuedEvents, ordered_work(editor).bHasInput, xuiDocumentEditorIsComposing(editor), allocator.errors);
      CHECK(result == XUI_ERROR_OUT_OF_MEMORY); }
    CHECK(ordered_work(editor).iQueuedEvents == 1 && !ordered_work(editor).bHasInput && xuiDocumentEditorIsComposing(editor));
    atomic_store(&allocator.fail_owner, 0); xuiDocumentUnsubscribe(d, token);
    source(d, "baseA");
    CHECK(xuiDocumentEditorRetryInput(editor) == XUI_OK); async_editor_flush(context, editor); source(d, "b\xe6\x96\x87" "eA");
    CHECK(!xuiDocumentEditorIsComposing(editor) && group_undo_count(d) == 2);
    CHECK(xuiEditUndo(editor) == XUI_OK); source(d, "baseA"); group_selection(editor, 5, 5);
    CHECK(xuiEditUndo(editor) == XUI_OK); source(d, "base");
    xuiWidgetDestroy(editor); ed.iUndoGroupTimeoutMs = UINT32_MAX;
    CHECK(xuiDocumentEditorCreate(context, &ed, &editor) == XUI_OK && xuiSetRootWidget(context, editor) == XUI_OK);
    CHECK(xuiWidgetSetRect(editor, (xui_rect_t){0, 0, 400, 260}) == XUI_OK); group_reset(context, editor, d);
    send_text(context, editor, "a", 0, 0); send_text(context, editor, "b", 0, 0); async_editor_flush(context, editor);
    CHECK(group_undo_count(d) == 2 && xuiEditUndo(editor) == XUI_OK); source(d, "basea");
    xuiWidgetDestroy(editor); xuiDocumentRelease(d); CHECK(!atomic_load(&allocator.live) && !allocator.errors);
    xuiDestroy(context); proxy.tProxy.fontDestroy(&proxy.tProxy, font);
    puts("Document Editor group boundaries: arrival-time timeout during replay and IME replacement-range retry after group publication passed.");
}
