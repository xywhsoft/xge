static xui_clipboard_set_text_proc ordered_clipboard_set;
static xui_clipboard_get_text_proc ordered_clipboard_get;
static int ordered_slow_clipboard;
static unsigned ordered_slow_calls;
static xui_widget ordered_reentry_editor;
static xui_widget ordered_callback_editor;
static unsigned ordered_callback_action;
static unsigned ordered_read_call, ordered_read_trigger;
static void ordered_clipboard_callback(void)
{
    xui_widget editor = ordered_callback_editor; unsigned action = ordered_callback_action;
    if (!editor || !action) return;
    ordered_callback_action = 0;
    if (action == 1) CHECK(xuiDocumentEditorCancelInput(editor) == XUI_OK);
    else if (action == 2) CHECK(xuiDocumentLoadMarkdown(xuiDocumentViewGetDocument(editor), "external", 8) == XUI_OK);
    else if (action == 3) xuiWidgetDestroy(editor);
    else if (action == 4) {
        CHECK(xuiDocumentEditorCancelInput(editor) == XUI_OK);
        send_text(xuiWidgetGetContext(editor), editor, "new", 0, 0);
    }
}
static int ordered_clipboard(xui_proxy proxy, const char* text)
{
    if (ordered_slow_clipboard) {
        if (ordered_reentry_editor) {
            xui_widget editor = ordered_reentry_editor; xui_doc_range_t range;
            xui_doc_command_state_t state = {0}; uint64_t replaced = 0; state.iSize = sizeof(state);
            CHECK(xuiDocumentEditorInsertText(editor, "overtake", 8) == XUI_DOC_ERROR_BUSY);
            CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_DOC_ERROR_BUSY);
            CHECK(xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_COPY, &state) == XUI_OK &&
                !state.bEnabled && state.iDisabledReason == XUI_DOC_ERROR_BUSY);
            CHECK(xuiDocumentEditorSetMarks(editor, XUI_DOC_BOLD, 0) == XUI_DOC_ERROR_BUSY);
            CHECK(xuiDocumentEditorReplaceAll(editor, "base", 4, "wrong", 5, NULL, &replaced) == XUI_DOC_ERROR_BUSY && !replaced);
            CHECK(xuiDocumentEditorFlush(editor) == XUI_DOC_ERROR_BUSY);
            CHECK(xuiDocumentEditorRetryInput(editor) == XUI_DOC_ERROR_BUSY);
            CHECK(xuiDocumentEditorCancelComposition(editor) == XUI_DOC_ERROR_BUSY);
            CHECK(xuiDocumentViewGetSelection(editor, &range) == XUI_OK && xuiDocumentViewSetSelection(editor, &range) == XUI_DOC_ERROR_BUSY);
            CHECK(xuiDocumentViewSetScroll(editor, 0, 0) == XUI_DOC_ERROR_BUSY);
            CHECK(xuiDocumentViewSetZoom(editor, 1.25f) == XUI_DOC_ERROR_BUSY);
            CHECK(xuiDocumentViewSetMode(editor, XUI_DOC_VISUAL) == XUI_DOC_ERROR_BUSY);
            CHECK(xuiDocumentViewFind(editor, "base", 4, 0, 0, &range) == XUI_DOC_ERROR_BUSY);
            CHECK(xuiDocumentEditorSetReadOnly(editor, 1) == XUI_DOC_ERROR_BUSY);
            CHECK(xuiEditCopy(editor) == XUI_DOC_ERROR_BUSY && xuiEditSetSelection(editor, 0, 0) == XUI_DOC_ERROR_BUSY);
        }
        uint64_t started = xrtClock(); ordered_slow_calls++;
        while (xrtClock() - started < 8000) xrtThreadYield();
    }
    ordered_clipboard_callback(); return ordered_clipboard_set(proxy, text);
}
static int ordered_clipboard_read(xui_proxy proxy, char* text, int capacity)
{
    int result = ordered_clipboard_get(proxy, text, capacity);
    if (ordered_read_trigger && ++ordered_read_call == ordered_read_trigger) ordered_clipboard_callback();
    return result;
}
static void ordered_key(xui_context context, xui_widget editor, int key, unsigned modifiers)
{
    xui_event_t event = {0}; event.iSize = sizeof(event); event.iType = XUI_EVENT_KEY_DOWN;
    event.iKey = key; event.iModifiers = modifiers; event.pTarget = editor;
    CHECK(xuiDispatchEvent(context, &event) == XUI_OK);
}
static xui_doc_editor_pending_work_t ordered_work(xui_widget editor)
{
    xui_doc_editor_pending_work_t work = {0}; work.iSize = sizeof(work);
    CHECK(xuiDocumentEditorGetPendingWork(editor, &work) == XUI_OK); return work;
}
static int ordered_parent_event(xui_widget w, const xui_event_t* event, void* data)
{
    (void)w;
    if (event->iType == XUI_EVENT_KEY_DOWN && event->iPhase == XUI_EVENT_PHASE_BUBBLE && event->iKey == 'P' && event->iModifiers & XUI_MOD_CTRL)
        (*(unsigned*)data)++;
    return XUI_OK;
}
static void ordered_wait_ready(xui_widget editor)
{
    xdeadline deadline = xrtDeadlineAfter(10000000); xui_doc_prepare_info_t info;
    do { info = async_editor_info(editor); CHECK(!xrtDeadlineExpired(deadline)); xrtThreadYield(); }
    while (info.iState != XUI_DOC_PREPARE_READY);
}
static void ordered_pause(async_editor_allocator* a, xui_widget editor)
{
    atomic_store(&a->entered, 0); atomic_store(&a->resume, 0); atomic_store(&a->gate, 1);
    CHECK(xuiDocumentEditorInsertText(editor, "u", 1) == XUI_OK && xuiDocumentEditorFlush(editor) == XUI_DOC_ERROR_BUSY);
    async_editor_gate(&a->entered);
}
typedef struct ordered_observer {
    async_editor_allocator* allocator;
    xui_widget editor;
    unsigned mode, calls;
} ordered_observer;
static void ordered_changed(xui_document d, xui_document_change_set change, void* data)
{
    ordered_observer* observer = data; xui_doc_change_info_t info = {0}; info.iSize = sizeof(info);
    CHECK(xuiDocumentChangeSetGetInfo(change, &info) == XUI_OK);
    if (!info.bUndo && observer->mode != 3 && observer->mode != 4) return;
    observer->calls++;
    if (ordered_work(observer->editor).iQueuedEvents)
        CHECK(xuiDocumentSaveFile(d, "build/document/ordered-callback.md", XUI_DOC_FILE_MARKDOWN) == XUI_DOC_ERROR_BUSY);
    if (observer->mode == 1) atomic_store(&observer->allocator->fail_owner, 1);
    else if (observer->mode == 4) {
        send_text(xuiWidgetGetContext(observer->editor), observer->editor, "next", 0, 0);
        CHECK(xuiDocumentSaveFile(d, "build/document/ordered-callback.md", XUI_DOC_FILE_MARKDOWN) == XUI_DOC_ERROR_BUSY);
    }
    else if (observer->mode >= 2) xuiWidgetDestroy(observer->editor);
}
static void ordered_editor_cases(void)
{
    xui_context context; xui_test_proxy_state_t proxy; xui_font font; xui_document d; xui_widget editor, other, host;
    xui_doc_desc_t desc = {0}; xui_doc_editor_desc_t ed = {0}; xui_doc_range_t range;
    async_editor_allocator a; xui_doc_editor_pending_work_t work; xui_doc_stats_t stats = {0};
    uint64_t revision, token; unsigned i, errors, parent_keys = 0; ordered_observer observer = {0};
    atomic_init(&a.gate, 0); atomic_init(&a.entered, 0); atomic_init(&a.resume, 0); atomic_init(&a.fail, 0);
    atomic_init(&a.fail_owner, 0); atomic_init(&a.live, 0);
    atomic_init(&a.owner_calls, 0); atomic_init(&a.fail_owner_at, UINT64_MAX);
    a.owner = xrtThreadCurrentId(); a.errors = 0;
    xuiTestProxyInit(&proxy); ordered_clipboard_set = proxy.tProxy.clipboardSetText; proxy.tProxy.clipboardSetText = ordered_clipboard;
    ordered_clipboard_get = proxy.tProxy.clipboardGetText; proxy.tProxy.clipboardGetText = ordered_clipboard_read;
    CHECK(xuiCreate(&context) == XUI_OK && xuiSetProxy(context, &proxy.tProxy) == XUI_OK);
    CHECK(proxy.tProxy.fontLoadFile(&proxy.tProxy, &font, "test.ttf", 14, 0) == XUI_OK && xuiSetDefaultFont(context, font) == XUI_OK);
    desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN;
    desc.onAlloc = async_editor_alloc; desc.onFree = async_editor_free; desc.pAllocatorUser = &a;
    CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK && xuiDocumentLoadMarkdown(d, "base", 4) == XUI_OK);
    CHECK(xuiDocumentClearHistory(d) == XUI_OK);
    ed.iSize = sizeof(ed); ed.tView.iSize = sizeof(ed.tView); ed.tView.pDocument = d;
    ed.iMode = XUI_DOC_SOURCE_TEXT; ed.iAsyncSourceThresholdBytes = 1; ed.onError = async_editor_error; ed.pUser = &a;
    CHECK(xuiDocumentEditorCreate(context, &ed, &editor) == XUI_OK && xuiDocumentEditorCreate(context, &ed, &other) == XUI_OK);
    CHECK(xuiWidgetCreate(context, &host) == XUI_OK && xuiSetRootWidget(context, host) == XUI_OK);
    CHECK(xuiWidgetSetEventCallback(host, ordered_parent_event, &parent_keys) == XUI_OK);
    CHECK(xuiWidgetSetRect(host, (xui_rect_t){0, 0, 800, 260}) == XUI_OK);
    CHECK(xuiWidgetAddChild(host, editor) == XUI_OK && xuiWidgetAddChild(host, other) == XUI_OK);
    CHECK(xuiWidgetSetRect(editor, (xui_rect_t){0, 0, 400, 260}) == XUI_OK && xuiWidgetSetRect(other, (xui_rect_t){400, 0, 400, 260}) == XUI_OK);
    CHECK(xuiInputViewport(context, 800, 260) == XUI_OK && xuiUpdate(context, .016f) == XUI_OK);
    CHECK(xuiEditSetSelection(editor, 4, 4) == XUI_OK && xuiDocumentEditorInsertText(editor, "A", 1) == XUI_OK);
    async_editor_flush(context, editor); source(d, "baseA"); revision = xuiDocumentGetRevision(d);
    ordered_pause(&a, editor);
    ordered_key(context, editor, 'Z', XUI_MOD_CTRL); ordered_key(context, editor, 'Z', XUI_MOD_CTRL);
    ordered_key(context, editor, 'Y', XUI_MOD_CTRL); send_text(context, editor, "X", 0, 0);
    ordered_key(context, editor, XUI_KEY_LEFT, 0); send_text(context, editor, "Y", 0, 0);
    work = ordered_work(editor); CHECK(work.bHasInput && work.iQueuedEvents == 6 && work.iResult == XUI_DOC_ERROR_BUSY);
    ordered_key(context, editor, 'P', XUI_MOD_CTRL); CHECK(parent_keys == 1 && ordered_work(editor).iQueuedEvents == 6);
    {
        xui_event_t move = {0}; move.iSize = sizeof(move); move.pTarget = editor; move.iType = XUI_EVENT_POINTER_MOVE;
        for (i = 0; i < 1000; i++) CHECK(xuiDispatchEvent(context, &move) == XUI_OK);
        CHECK(ordered_work(editor).iQueuedEvents == 6); /* Hover never backlogs input. */
    }
    CHECK(xuiDocumentEditorInsertText(editor, "overtake", 8) == XUI_DOC_ERROR_BUSY);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_DOC_ERROR_BUSY && xuiEditCopy(editor) == XUI_DOC_ERROR_BUSY);
    CHECK(xuiDocumentViewGetSelection(editor, &range) == XUI_OK && xuiDocumentViewSetSelection(editor, &range) == XUI_DOC_ERROR_BUSY);
    CHECK(xuiEditSetSelection(editor, 0, 0) == XUI_DOC_ERROR_BUSY && !xuiDocumentEditorCanExecute(editor, XUI_DOC_EDIT_PASTE));
    CHECK(xuiDocumentEditorInsertText(other, "other", 5) == XUI_DOC_ERROR_BUSY);
    CHECK(xuiEditSetSelection(other, 0, 4) == XUI_OK && xuiEditCopy(other) == XUI_OK && !strcmp(proxy.sClipboard, "base"));
    CHECK(xuiDocumentViewSetMode(editor, XUI_DOC_VISUAL) == XUI_DOC_ERROR_BUSY);
    CHECK(xuiDocumentEditorSetReadOnly(editor, 1) == XUI_DOC_ERROR_BUSY && !xuiDocumentEditorGetReadOnly(editor));
    CHECK(xuiDocumentSaveFile(d, "build/document/ordered.md", XUI_DOC_FILE_MARKDOWN) == XUI_DOC_ERROR_BUSY);
    CHECK(xuiUpdate(context, .016f) == XUI_OK && ordered_work(editor).iQueuedEvents == 6);
    CHECK(xuiDocumentGetRevision(d) == revision && !strcmp(xuiEditGetText(editor), "baseAu"));
    atomic_store(&a.resume, 1); async_editor_flush(context, editor); source(d, "baseAYX");
    CHECK(!ordered_work(editor).bHasInput && !ordered_work(editor).iQueuedEvents && ordered_work(editor).iResult == XUI_OK);
    CHECK(xuiDocumentViewGetSelection(editor, &range) == XUI_OK && range.tCaret.iOffset == 6);
    CHECK(xuiEditUndo(editor) == XUI_OK); source(d, "baseAX");
    CHECK(xuiDocumentViewGetSelection(editor, &range) == XUI_OK && range.tCaret.iOffset == 5);
    CHECK(xuiEditUndo(editor) == XUI_OK); source(d, "baseA");
    CHECK(xuiDocumentViewGetSelection(editor, &range) == XUI_OK && range.tCaret.iOffset == 5);
    CHECK(xuiEditRedo(editor) == XUI_OK); source(d, "baseAX");
    CHECK(xuiEditRedo(editor) == XUI_OK); source(d, "baseAYX"); CHECK(!a.errors);

    /* Allocation failure after Undo has removed the prepare: accepted text
     * remains queued, and SaveFile/other writers must still be blocked. */
    CHECK(xuiDocumentLoadMarkdown(d, "base", 4) == XUI_OK && xuiDocumentClearHistory(d) == XUI_OK);
    CHECK(xuiEditSetSelection(editor, 4, 4) == XUI_OK); ordered_pause(&a, editor);
    ordered_key(context, editor, 'Z', XUI_MOD_CTRL); send_text(context, editor, "R", 0, 0);
    observer.allocator = &a; observer.editor = editor; observer.mode = 1;
    CHECK(xuiDocumentSubscribe(d, ordered_changed, &observer, &token) == XUI_OK);
    atomic_store(&a.resume, 1); ordered_wait_ready(editor);
    CHECK(xuiDocumentEditorFlush(editor) == XUI_ERROR_OUT_OF_MEMORY && observer.calls == 1);
    work = ordered_work(editor); CHECK(!work.bHasInput && work.iQueuedEvents == 1 && work.iResult == XUI_ERROR_OUT_OF_MEMORY);
    CHECK(!xuiDocumentHasPrepare(d) && xuiDocumentSaveFile(d, "build/document/ordered.md", XUI_DOC_FILE_MARKDOWN) == XUI_DOC_ERROR_BUSY);
    CHECK(xuiDocumentEditorInsertText(other, "other", 5) == XUI_DOC_ERROR_BUSY);
    errors = a.errors; CHECK(xuiUpdate(context, .016f) == XUI_OK && a.errors == errors + 1);
    CHECK(xuiUpdate(context, .016f) == XUI_OK && a.errors == errors + 1);
    atomic_store(&a.fail_owner, 0); xuiDocumentUnsubscribe(d, token);
    CHECK(xuiDocumentEditorRetryInput(editor) == XUI_OK); async_editor_flush(context, editor); source(d, "baseR");
    stats.iSize = sizeof(stats); CHECK(xuiDocumentGetStats(d, &stats) == XUI_OK && stats.iUndoCount == 1);
    CHECK(xuiDocumentSaveFile(d, "build/document/ordered.md", XUI_DOC_FILE_MARKDOWN) == XUI_OK);

    /* Explicit cancellation and an external authoritative edit discard both
     * the original candidate and its queued dependent events. */
    ordered_pause(&a, editor); ordered_key(context, editor, 'Z', XUI_MOD_CTRL); send_text(context, editor, "discard", 0, 0);
    CHECK(xuiDocumentEditorCancelInput(editor) == XUI_OK && !ordered_work(editor).iQueuedEvents);
    atomic_store(&a.resume, 1); CHECK(xuiDocumentSaveFile(d, "build/document/ordered.md", XUI_DOC_FILE_MARKDOWN) == XUI_OK); source(d, "baseR");
    /* Join the cancelled worker before reusing the same deterministic gate. */
    xuiWidgetDestroy(editor); CHECK(xuiDocumentEditorCreate(context, &ed, &editor) == XUI_OK && xuiWidgetAddChild(host, editor) == XUI_OK);
    CHECK(xuiWidgetSetRect(editor, (xui_rect_t){0, 0, 400, 260}) == XUI_OK && xuiEditSetSelection(editor, 5, 5) == XUI_OK);
    ordered_pause(&a, editor); ordered_key(context, editor, 'Z', XUI_MOD_CTRL); send_text(context, editor, "stale", 0, 0);
    CHECK(xuiDocumentLoadMarkdown(d, "external", 8) == XUI_OK && !ordered_work(editor).iQueuedEvents);
    atomic_store(&a.resume, 1); source(d, "external");
    xuiWidgetDestroy(editor); CHECK(xuiDocumentEditorCreate(context, &ed, &editor) == XUI_OK && xuiWidgetAddChild(host, editor) == XUI_OK);
    CHECK(xuiWidgetSetRect(editor, (xui_rect_t){0, 0, 400, 260}) == XUI_OK);

    /* More than one frame of events keeps its save barrier between batches. */
    CHECK(xuiDocumentLoadMarkdown(d, "base", 4) == XUI_OK && xuiDocumentClearHistory(d) == XUI_OK);
    CHECK(xuiEditSetSelection(editor, 4, 4) == XUI_OK); ordered_pause(&a, editor);
    ordered_key(context, editor, 'Z', XUI_MOD_CTRL);
    for (i = 0; i < 160; i++) send_text(context, editor, "x", 0, 0);
    CHECK(ordered_work(editor).iQueuedEvents == 161); atomic_store(&a.resume, 1); ordered_wait_ready(editor);
    CHECK(xuiDocumentEditorFlush(editor) == XUI_DOC_ERROR_BUSY && ordered_work(editor).iQueuedEvents >= 97 && ordered_work(editor).iQueuedEvents < 161);
    async_editor_flush(context, editor);
    { char expected[165]; memcpy(expected, "base", 4); memset(expected + 4, 'x', 160); expected[164] = 0; source(d, expected); }
    CHECK(xuiEditUndo(editor) == XUI_OK); source(d, "base");

    /* Preedit/cancel/confirm stays in input order behind Undo and forms no
     * independent content history. */
    CHECK(xuiDocumentClearHistory(d) == XUI_OK && xuiEditSetSelection(editor, 4, 4) == XUI_OK); ordered_pause(&a, editor);
    ordered_key(context, editor, 'Z', XUI_MOD_CTRL);
    send_text(context, editor, "\xe4\xb8\xad", 1, 1); send_text(context, editor, "", 1, 0);
    send_text(context, editor, "\xe6\x96\x87", 1, 1); send_text(context, editor, "\xe6\x96\x87", 1, 0); send_text(context, editor, "Z", 0, 0);
    atomic_store(&a.resume, 1); async_editor_flush(context, editor); source(d, "base\xe6\x96\x87Z");
    CHECK(!xuiDocumentEditorIsComposing(editor) && xuiEditUndo(editor) == XUI_OK); source(d, "base\xe6\x96\x87");
    CHECK(xuiEditUndo(editor) == XUI_OK); source(d, "base");

    /* CancelInput also discards a last replayed preedit with no source
     * candidate left after Undo. It must not depend on bHasInput. */
    CHECK(xuiDocumentClearHistory(d) == XUI_OK && xuiEditSetSelection(editor, 4, 4) == XUI_OK); ordered_pause(&a, editor);
    ordered_key(context, editor, 'Z', XUI_MOD_CTRL); send_text(context, editor, "\xe4\xb8\xad", 1, 1);
    atomic_store(&a.resume, 1); async_editor_flush(context, editor);
    CHECK(xuiDocumentEditorIsComposing(editor) && !ordered_work(editor).bHasInput && !ordered_work(editor).iQueuedEvents);
    CHECK(xuiDocumentEditorCancelInput(editor) == XUI_OK && !xuiDocumentEditorIsComposing(editor)); source(d, "base");
    CHECK(xuiEditSetSelection(editor, 0, 4) == XUI_OK); send_text(context, editor, "\xe4\xb8\xad", 1, 1);
    CHECK(xuiEditCopy(editor) == XUI_OK && xuiDocumentEditorIsComposing(editor) && !strcmp(proxy.sClipboard, "base"));
    CHECK(xuiDocumentEditorCancelInput(editor) == XUI_OK && !xuiDocumentEditorIsComposing(editor));

    /* A slow host callback consumes the frame budget. Work after it must
     * wait for the next Flush, even when fewer than 64 events were processed. */
    CHECK(xuiDocumentClearHistory(d) == XUI_OK && xuiEditSetSelection(editor, 4, 4) == XUI_OK); ordered_pause(&a, editor);
    ordered_key(context, editor, 'Z', XUI_MOD_CTRL); ordered_key(context, editor, 'A', XUI_MOD_CTRL);
    ordered_key(context, editor, 'C', XUI_MOD_CTRL); send_text(context, editor, "J", 0, 0);
    atomic_store(&a.resume, 1); ordered_wait_ready(editor); ordered_slow_clipboard = 1; ordered_slow_calls = 0; ordered_reentry_editor = editor;
    for (i = 0; !ordered_slow_calls; i++) { CHECK(i < 100); CHECK(xuiDocumentEditorFlush(editor) == XUI_DOC_ERROR_BUSY); }
    ordered_slow_clipboard = 0; ordered_reentry_editor = NULL;
    CHECK(ordered_work(editor).iQueuedEvents == 1 && !ordered_work(editor).bHasInput);
    CHECK(xuiDocumentSaveFile(d, "build/document/ordered.md", XUI_DOC_FILE_MARKDOWN) == XUI_DOC_ERROR_BUSY);
    CHECK(!strcmp(proxy.sClipboard, "base")); async_editor_flush(context, editor); source(d, "J");
    CHECK(xuiEditUndo(editor) == XUI_OK); source(d, "base");
    CHECK(xuiDocumentViewGetSelection(editor, &range) == XUI_OK && range.tAnchor.iOffset == 0 && range.tCaret.iOffset == 4);

    /* The edit protocol uses the same guard even for an immediate Copy,
     * where no pending input or event queue existed before the callback. */
    ordered_slow_clipboard = 1; ordered_reentry_editor = editor;
    CHECK(xuiEditCopy(editor) == XUI_OK);
    ordered_slow_clipboard = 0; ordered_reentry_editor = NULL;

    /* Cancellation during Cut's clipboard callback aborts the pending
     * deletion too, so explicit cancellation cannot remove committed text. */
    CHECK(xuiDocumentClearHistory(d) == XUI_OK && xuiEditSetSelection(editor, 4, 4) == XUI_OK); ordered_pause(&a, editor);
    ordered_key(context, editor, 'Z', XUI_MOD_CTRL); ordered_key(context, editor, 'A', XUI_MOD_CTRL);
    ordered_key(context, editor, 'X', XUI_MOD_CTRL); send_text(context, editor, "discard", 0, 0);
    ordered_callback_editor = editor; ordered_callback_action = 1;
    atomic_store(&a.resume, 1); async_editor_flush(context, editor);
    CHECK(!ordered_callback_action && !ordered_work(editor).bHasInput && !ordered_work(editor).iQueuedEvents);
    ordered_callback_editor = NULL; source(d, "base");

    /* Both paste callbacks and Cut may cancel, replace the Document, destroy
     * the Editor, or cancel then enqueue fresh text. None may resume the old
     * edit on a changed lifetime; fresh events must survive queue teardown. */
    {
        unsigned boundary, action;
        for (boundary = 0; boundary < 3; boundary++) for (action = 1; action <= 4; action++) {
            int result;
            CHECK(xuiDocumentLoadMarkdown(d, "base", 4) == XUI_OK && xuiDocumentClearHistory(d) == XUI_OK);
            CHECK(xuiEditSetSelection(editor, 4, 4) == XUI_OK); ordered_pause(&a, editor);
            ordered_key(context, editor, 'Z', XUI_MOD_CTRL); ordered_key(context, editor, 'A', XUI_MOD_CTRL);
            ordered_key(context, editor, boundary ? 'V' : 'X', XUI_MOD_CTRL); send_text(context, editor, "discard", 0, 0);
            CHECK(ordered_clipboard_set(&proxy.tProxy, "paste") == XUI_OK);
            ordered_callback_editor = editor; ordered_callback_action = action;
            ordered_read_trigger = boundary; ordered_read_call = 0;
            atomic_store(&a.resume, 1); ordered_wait_ready(editor);
            for (i = 0; ordered_callback_action; i++) {
                CHECK(i < 100); result = xuiDocumentEditorFlush(editor);
                CHECK(result == XUI_OK || result == XUI_DOC_ERROR_BUSY || (action == 3 && result == XUI_DOC_ERROR_STALE));
            }
            ordered_callback_editor = NULL; ordered_read_trigger = 0;
            if (action == 3) {
                CHECK(xuiDocumentEditorCreate(context, &ed, &editor) == XUI_OK && xuiWidgetAddChild(host, editor) == XUI_OK);
                CHECK(xuiWidgetSetRect(editor, (xui_rect_t){0, 0, 400, 260}) == XUI_OK);
            } else async_editor_flush(context, editor);
            source(d, action == 2 ? "external" : action == 4 ? "new" : "base");
            CHECK(xuiDocumentSaveFile(d, "build/document/ordered.md", XUI_DOC_FILE_MARKDOWN) == XUI_OK);
        }
        CHECK(xuiDocumentLoadMarkdown(d, "base", 4) == XUI_OK);
    }

    /* An observer can deliver another event during the final publication.
     * Flush must not report idle while that newly queued event still exists. */
    CHECK(xuiDocumentClearHistory(d) == XUI_OK && xuiEditSetSelection(editor, 4, 4) == XUI_OK); ordered_pause(&a, editor);
    observer.mode = 4; observer.calls = 0; observer.editor = editor;
    CHECK(xuiDocumentSubscribe(d, ordered_changed, &observer, &token) == XUI_OK);
    atomic_store(&a.resume, 1); ordered_wait_ready(editor);
    CHECK(xuiDocumentEditorFlush(editor) == XUI_DOC_ERROR_BUSY && observer.calls == 1);
    xuiDocumentUnsubscribe(d, token);
    CHECK(ordered_work(editor).iQueuedEvents == 1 && !ordered_work(editor).bHasInput);
    async_editor_flush(context, editor); source(d, "baseunext");
    CHECK(xuiDocumentLoadMarkdown(d, "base", 4) == XUI_OK);

    /* Pointer capture and focus follow delivery, never delayed replay. */
    CHECK(xuiDocumentClearHistory(d) == XUI_OK && xuiEditSetSelection(editor, 4, 4) == XUI_OK); ordered_pause(&a, editor);
    ordered_key(context, editor, 'Z', XUI_MOD_CTRL);
    {
        double sx, sy;
        CHECK(xuiDocumentViewGetScroll(editor, &sx, &sy) == XUI_OK);
        CHECK(sx == 0 && sy == 0);
        xui_event_t event = {0}; event.iSize = sizeof(event); event.pTarget = editor; event.iButton = XUI_POINTER_BUTTON_LEFT;
        event.fX = event.fY = 1; event.iType = XUI_EVENT_POINTER_DOWN; CHECK(xuiDispatchEvent(context, &event) == XUI_OK);
        event.iType = XUI_EVENT_POINTER_MOVE;
        for (i = 0; i < 1000; i++) CHECK(xuiDispatchEvent(context, &event) == XUI_OK);
        CHECK(ordered_work(editor).iQueuedEvents < 8); /* Consecutive drag moves coalesce. */
        event.iType = XUI_EVENT_POINTER_UP; CHECK(xuiDispatchEvent(context, &event) == XUI_OK); send_text(context, editor, "Q", 0, 0);
    }
    CHECK(xuiSetFocusWidget(context, other) == XUI_OK); atomic_store(&a.resume, 1); async_editor_flush(context, editor);
    source(d, "Qbase"); CHECK(xuiGetFocusWidget(context) == other);

    /* Refused enqueue does not overwrite earlier accepted events. */
    CHECK(xuiDocumentLoadMarkdown(d, "base", 4) == XUI_OK && xuiDocumentClearHistory(d) == XUI_OK);
    CHECK(xuiEditSetSelection(editor, 4, 4) == XUI_OK); ordered_pause(&a, editor);
    ordered_key(context, editor, 'Z', XUI_MOD_CTRL); errors = a.errors;
    atomic_store(&a.fail_owner, 1); send_text(context, editor, "rejected", 0, 0); atomic_store(&a.fail_owner, 0);
    CHECK(a.errors == errors + 1 && ordered_work(editor).iQueuedEvents == 1);
    ordered_key(context, editor, 'B', XUI_MOD_CTRL); /* SOURCE has no bold command: report it, then continue. */
    send_text(context, editor, "kept", 0, 0); atomic_store(&a.resume, 1); async_editor_flush(context, editor); source(d, "basekept");
    CHECK(a.errors == errors + 2);

    CHECK(xuiDocumentLoadMarkdown(d, "base", 4) == XUI_OK && xuiDocumentClearHistory(d) == XUI_OK);
    CHECK(xuiEditSetSelection(editor, 4, 4) == XUI_OK); ordered_pause(&a, editor);
    ordered_key(context, editor, 'Z', XUI_MOD_CTRL);
    for (i = 0; i < 4095; i++) send_text(context, editor, "x", 0, 0);
    errors = a.errors; send_text(context, editor, "overflow", 0, 0);
    CHECK(a.errors == errors + 1 && ordered_work(editor).iQueuedEvents == 4096);
    CHECK(xuiDocumentEditorCancelInput(editor) == XUI_OK && ordered_work(editor).iResult == XUI_OK);
    atomic_store(&a.resume, 1); xuiWidgetDestroy(editor); source(d, "base");
    CHECK(xuiDocumentEditorCreate(context, &ed, &editor) == XUI_OK && xuiWidgetAddChild(host, editor) == XUI_OK);
    CHECK(xuiWidgetSetRect(editor, (xui_rect_t){0, 0, 400, 260}) == XUI_OK);

    /* Observer teardown while replaying Undo must free the remaining queue
     * and avoid touching subtype memory after the callback. */
    CHECK(xuiDocumentLoadMarkdown(d, "base", 4) == XUI_OK && xuiDocumentClearHistory(d) == XUI_OK);
    CHECK(xuiEditSetSelection(editor, 4, 4) == XUI_OK); ordered_pause(&a, editor);
    ordered_key(context, editor, 'Z', XUI_MOD_CTRL); send_text(context, editor, "must not replay", 0, 0);
    observer.mode = 2; observer.calls = 0; observer.editor = editor;
    CHECK(xuiDocumentSubscribe(d, ordered_changed, &observer, &token) == XUI_OK);
    atomic_store(&a.resume, 1); ordered_wait_ready(editor); CHECK(xuiDocumentEditorFlush(editor) == XUI_OK && observer.calls == 1);
    source(d, "base"); xuiDocumentUnsubscribe(d, token);
    CHECK(xuiDocumentSaveFile(d, "build/document/ordered.md", XUI_DOC_FILE_MARKDOWN) == XUI_OK);

    /* Teardown during the initial pending publication, before its queued
     * Undo can run, is a separate callback boundary. */
    CHECK(xuiDocumentEditorCreate(context, &ed, &editor) == XUI_OK && xuiWidgetAddChild(host, editor) == XUI_OK);
    CHECK(xuiWidgetSetRect(editor, (xui_rect_t){0, 0, 400, 260}) == XUI_OK && xuiEditSetSelection(editor, 4, 4) == XUI_OK);
    ordered_pause(&a, editor); ordered_key(context, editor, 'Z', XUI_MOD_CTRL); send_text(context, editor, "later", 0, 0);
    observer.mode = 3; observer.calls = 0; observer.editor = editor;
    CHECK(xuiDocumentSubscribe(d, ordered_changed, &observer, &token) == XUI_OK);
    atomic_store(&a.resume, 1); ordered_wait_ready(editor); CHECK(xuiDocumentEditorFlush(editor) == XUI_OK && observer.calls == 1);
    source(d, "baseu"); xuiDocumentUnsubscribe(d, token);
    CHECK(xuiDocumentSaveFile(d, "build/document/ordered.md", XUI_DOC_FILE_MARKDOWN) == XUI_OK);

    /* Mode/find/document transitions resume after Flush only if its observer
     * left the original widget alive. The committed root still survives. */
    for (i = 0; i < 3; i++) {
        xui_document replacement = NULL; int result;
        CHECK(xuiDocumentLoadMarkdown(d, "base", 4) == XUI_OK && xuiDocumentClearHistory(d) == XUI_OK);
        CHECK(xuiDocumentEditorCreate(context, &ed, &editor) == XUI_OK && xuiWidgetAddChild(host, editor) == XUI_OK);
        CHECK(xuiWidgetSetRect(editor, (xui_rect_t){0, 0, 400, 260}) == XUI_OK && xuiEditSetSelection(editor, 4, 4) == XUI_OK);
        ordered_pause(&a, editor); observer.mode = 3; observer.calls = 0; observer.editor = editor;
        CHECK(xuiDocumentSubscribe(d, ordered_changed, &observer, &token) == XUI_OK);
        atomic_store(&a.resume, 1); ordered_wait_ready(editor);
        if (i == 0) result = xuiDocumentViewSetMode(editor, XUI_DOC_VISUAL);
        else if (i == 1) result = xuiDocumentViewFind(editor, "base", 4, 0, 0, &range);
        else {
            CHECK(xuiDocumentCreate(&desc, &replacement) == XUI_OK);
            result = xuiDocumentViewSetDocument(editor, replacement); xuiDocumentRelease(replacement);
        }
        CHECK(result == XUI_ERROR_INVALID_STATE && observer.calls == 1);
        source(d, "baseu"); xuiDocumentUnsubscribe(d, token);
        CHECK(xuiDocumentSaveFile(d, "build/document/ordered.md", XUI_DOC_FILE_MARKDOWN) == XUI_OK);
    }
    xuiWidgetDestroy(other); xuiWidgetDestroy(host); xuiDocumentRelease(d); xuiDestroy(context); proxy.tProxy.fontDestroy(&proxy.tProxy, font);
    CHECK(!atomic_load(&a.live));
    puts("Document ordered Editor events: repeated Undo/Redo, subsequent input/navigation/IME/pointers, bounded FIFO, API/save barriers, OOM retry, clipboard reentry/cancel/replacement/lifetime, and observer/transition teardown passed.");
}
