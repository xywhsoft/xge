/* Retained ranges follow the editor's unpublished SOURCE generation. The
 * committed peer, owner callbacks and history still use the same Document. */
#include "../extlibs/xrt/xrt.h"
#include <stdatomic.h>
typedef struct prepared_text_allocator {
    uint64_t owner;
    atomic_uint_fast64_t calls, live, fail_at;
    atomic_int gate, entered, resume, fail_worker;
} prepared_text_allocator;
static void prepared_text_wait(atomic_int* flag)
{
    xdeadline deadline = xrtDeadlineAfter(30000000);
    while (!atomic_load(flag)) { CHECK(!xrtDeadlineExpired(deadline)); xrtThreadYield(); }
}
static void* prepared_text_alloc(void* user, size_t bytes)
{
    prepared_text_allocator* a = user; void* p;
    if (xrtThreadCurrentId() == a->owner) {
        uint64_t call = atomic_fetch_add(&a->calls, 1) + 1;
        if (call == atomic_load(&a->fail_at)) return NULL;
    } else {
        if (atomic_exchange(&a->gate, 0)) {
            atomic_store(&a->entered, 1); prepared_text_wait(&a->resume);
        }
        if (atomic_load(&a->fail_worker)) return NULL;
    }
    p = malloc(bytes); if (p) atomic_fetch_add(&a->live, 1); return p;
}
static void prepared_text_free(void* user, void* p)
{
    prepared_text_allocator* a = user;
    if (p) { CHECK(atomic_fetch_sub(&a->live, 1)); free(p); }
}
static void prepared_text_init(prepared_text_allocator* a)
{
    a->owner = xrtThreadCurrentId(); atomic_init(&a->calls, 0); atomic_init(&a->live, 0);
    atomic_init(&a->fail_at, UINT64_MAX); atomic_init(&a->gate, 0);
    atomic_init(&a->entered, 0); atomic_init(&a->resume, 0); atomic_init(&a->fail_worker, 0);
}
static xui_doc_prepare_info_t prepared_text_info(xui_widget editor)
{
    xui_doc_prepare_info_t info = {0}; info.iSize = sizeof(info);
    CHECK(xuiDocumentEditorGetPendingInput(editor, &info) == XUI_OK); return info;
}
static xui_internal_accessible_text_range* prepared_text_document(xui_widget editor)
{
    xui_accessible_node_t root = {0}; xui_internal_accessible_text_range* range = NULL;
    xui_internal_accessible_text_request request = {XUI_INTERNAL_TEXT_DOCUMENT, 0, 0, 0}; int result;
    root.iSize = sizeof(root); CHECK(xuiWidgetGetAccessibleNode(editor, 0, &root) == XUI_OK);
    result = xuiInternalAccessibilityCreateTextRange(editor, root.iId, &request, &range);
    if (result != XUI_OK) fprintf(stderr, "Prepared document range result=%d\n", result);
    CHECK(result == XUI_OK && range); return range;
}
static int prepared_text_flush(xui_widget editor)
{
    xdeadline deadline = xrtDeadlineAfter(30000000); int result;
    do {
        result = xuiDocumentEditorFlush(editor);
        CHECK(!xrtDeadlineExpired(deadline)); xrtThreadYield();
    } while (result == XUI_DOC_ERROR_BUSY);
    return result;
}
static void prepared_text_source(xui_document document, const char* expected)
{
    xui_document_snapshot snapshot; char buffer[256]; uint64_t bytes;
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
    CHECK(xuiDocumentSnapshotCopySource(snapshot, buffer, sizeof(buffer), &bytes) == XUI_OK);
    CHECK(bytes == strlen(expected) && !strcmp(buffer, expected)); xuiDocumentSnapshotRelease(snapshot);
}
static void prepared_text_external_insert(xui_document document, uint64_t at, const char* inserted)
{
    xui_doc_txn_desc_t desc = {0}; xui_doc_source_patch_t patch = {0}; xui_document_transaction transaction;
    desc.iSize = sizeof(desc); desc.iDomain = XUI_DOC_SOURCE;
    patch.iStart = patch.iEnd = at; patch.sText = inserted; patch.iTextBytes = strlen(inserted);
    CHECK(xuiDocumentBeginTransaction(document, &desc, &transaction) == XUI_OK);
    CHECK(xuiDocumentTxnReplaceSource(transaction, patch.iStart, patch.iEnd, patch.sText, patch.iTextBytes) == XUI_OK);
    CHECK(xuiDocumentTxnCommit(transaction, NULL) == XUI_OK); xuiDocumentTxnRelease(transaction);
}
typedef struct prepared_text_observer {
    xui_widget editor;
    xui_internal_accessible_text_range* range;
    uint64_t generation, first, last;
    int enabled, called;
} prepared_text_observer;
static void prepared_text_changed(xui_document document, xui_document_change_set change, void* user)
{
    prepared_text_observer* observer = user; xui_doc_change_info_t info = {0}; int result;
    char buffer[32]; uint64_t bytes = 77, first, last, length;
    (void)document;
    if (!observer->enabled) return;
    info.iSize = sizeof(info); CHECK(xuiDocumentChangeSetGetInfo(change, &info) == XUI_OK);
    CHECK(info.iInputGeneration == observer->generation);
    CHECK(prepared_text_info(observer->editor).iState == XUI_DOC_PREPARE_PUBLISHING);
    result = observer->range->ops->getText(observer->range, buffer, sizeof(buffer), &bytes);
    if (result != XUI_OK) fprintf(stderr, "Published range in owner callback result=%d\n", result);
    CHECK(result == XUI_OK && bytes == 4 && !strcmp(buffer, "tail"));
    CHECK(observer->range->ops->getOffsets(observer->range, &first, &last, &length) == XUI_OK);
    CHECK(first == observer->first && last == observer->last); observer->called++;
}
static void prepared_text_case(uint32_t mode)
{
    static const char original[] = "abc\n\n# tail\n\nlast\n";
    static const char first[] = "a\xe4\xb8\xad" "bc\n\n# tail\n\nlast\n";
    static const char continued[] = "a\xe4\xb8\xad" "_\n_bc\n\n# tail\n\nlast\n";
    static const uint64_t source_lines[] = {0, 7, 8, 15, 16, 21};
    static const uint64_t live_lines[] = {0, 7, 8, 16, 21};
    static const uint64_t source_continued[] = {0, 6, 10, 11, 18, 19, 24};
    static const uint64_t live_continued[] = {0, 6, 10, 11, 19, 24};
    static const uint64_t paragraphs[] = {0, 8, 16, 21};
    static const uint64_t source_format[] = {0, 21}, live_format[] = {0, 8, 10, 14, 16, 20, 21};
    static const uint64_t source_format_continued[] = {0, 24}, live_format_continued[] = {0, 11, 13, 17, 19, 23, 24};
    prepared_text_allocator allocator; xui_test_proxy_state_t proxy; xui_context context; xui_font font;
    xui_document document, replacement; xui_doc_desc_t desc = {0}; xui_doc_editor_desc_t ed = {0};
    xui_doc_view_desc_t vd = {0}; xui_widget editor, view;
    xui_internal_accessible_text_range *base, *retained, *pending, *found, *caret, *selected, *reader, *clone, *semantic, *format;
    xui_doc_range_t selection; uint64_t revision, generation, calls, length = 88, i, token; int actual;
    prepared_text_observer observer = {0};
    prepared_text_init(&allocator); xuiTestProxyInit(&proxy);
    CHECK(xuiCreate(&context) == XUI_OK && xuiSetProxy(context, &proxy.tProxy) == XUI_OK);
    CHECK(xuiSetViewportSize(context, 800, 600) == XUI_OK);
    CHECK(proxy.tProxy.fontLoadFile(&proxy.tProxy, &font, "test.ttf", 14, 0) == XUI_OK);
    CHECK(xuiSetDefaultFont(context, font) == XUI_OK);
    desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN;
    desc.onAlloc = prepared_text_alloc; desc.onFree = prepared_text_free; desc.pAllocatorUser = &allocator;
    CHECK(xuiDocumentCreate(&desc, &document) == XUI_OK);
    CHECK(xuiDocumentLoadMarkdown(document, original, sizeof(original) - 1) == XUI_OK);
    CHECK(xuiDocumentClearHistory(document) == XUI_OK); revision = xuiDocumentGetRevision(document);
    /* Subscribe before the Views: this observer runs after both View change
     * handlers, while the owner is still inside Prepare publication. */
    CHECK(xuiDocumentSubscribe(document, prepared_text_changed, &observer, &token) == XUI_OK);
    ed.iSize = sizeof(ed); ed.tView.iSize = sizeof(ed.tView); ed.tView.pDocument = document;
    ed.iMode = mode; ed.iAsyncSourceThresholdBytes = 1;
    CHECK(xuiDocumentEditorCreate(context, &ed, &editor) == XUI_OK);
    CHECK(xuiSetRootWidget(context, editor) == XUI_OK);
    CHECK(xuiWidgetSetRect(editor, (xui_rect_t){0, 0, 600, 400}) == XUI_OK);
    CHECK(xuiLayout(context) == XUI_OK);
    CHECK(xuiDocumentViewSetMode(editor, XUI_DOC_VISUAL) == XUI_OK);
    {
        xui_internal_accessible_text_range* visual = create(editor, XUI_INTERNAL_TEXT_DOCUMENT, 0, 0, 0);
        CHECK(visual->ops->findText(visual, "tail", 4, 0, 0, &semantic) == XUI_OK && semantic);
        text(semantic, "tail"); visual->ops->release(visual);
    }
    CHECK(xuiDocumentViewSetMode(editor, mode) == XUI_OK);
    vd.iSize = sizeof(vd); vd.pDocument = document;
    CHECK(xuiDocumentViewCreate(context, &vd, &view) == XUI_OK);
    CHECK(xuiDocumentViewSetMode(view, XUI_DOC_SOURCE_TEXT) == XUI_OK);
    base = create(editor, XUI_INTERNAL_TEXT_DOCUMENT, 0, 0, 0);
    CHECK(base->ops->clone(base, &retained) == XUI_OK);
    CHECK(retained->ops->setOffsets(retained, 7, 11) == XUI_OK); text(retained, "tail");
    reader = create(view, XUI_INTERNAL_TEXT_DOCUMENT, 0, 0, 0);
    CHECK(xuiEditSetSelection(editor, 1, 1) == XUI_OK);
    CHECK(xuiDocumentEditorInsertText(editor, "\xe4\xb8\xad", 3) == XUI_OK);
    CHECK(xuiDocumentHasPrepare(document) && xuiDocumentGetRevision(document) == revision);
    generation = prepared_text_info(editor).iGeneration; CHECK(generation);
    /* The old formal DLL must fail here: pending document ranges used to be
     * rejected with UNSUPPORTED, before any worker has started. */
    pending = prepared_text_document(editor);
    text(pending, first); text(base, first); offsets(retained, 10, 14); text(retained, "tail");
    offsets(semantic, 10, 14); text(semantic, "tail");
    text(reader, original); prepared_text_source(document, original);
    text_line_partition(editor, first, mode == XUI_DOC_SOURCE_TEXT ? source_lines : live_lines,
        mode == XUI_DOC_SOURCE_TEXT ? sizeof(source_lines) / sizeof(*source_lines) : sizeof(live_lines) / sizeof(*live_lines));
    text_unit_partition(editor, first, paragraphs, sizeof(paragraphs) / sizeof(*paragraphs), XUI_INTERNAL_TEXT_PARAGRAPH);
    text_unit_partition(editor, first, mode == XUI_DOC_SOURCE_TEXT ? source_format : live_format,
        mode == XUI_DOC_SOURCE_TEXT ? 2 : 7, XUI_INTERNAL_TEXT_FORMAT);
    CHECK(pending->ops->clone(pending, &format) == XUI_OK);
    CHECK(format->ops->setOffsets(format, 0, 0) == XUI_OK);
    CHECK(format->ops->expand(format, XUI_INTERNAL_TEXT_FORMAT) == XUI_OK);
    caret = create(editor, XUI_INTERNAL_TEXT_CARET, 0, 0, 0); offsets(caret, 4, 4);
    selected = create(editor, XUI_INTERNAL_TEXT_SELECTION, 0, 0, 0); offsets(selected, 4, 4);
    CHECK(pending->ops->findText(pending, "tail", 4, 0, 0, &found) == XUI_OK && found);
    offsets(found, 10, 14); CHECK(found->ops->clone(found, &clone) == XUI_OK);
    {
        xui_rect_t rectangles[16]; uint64_t count, first_offset, last_offset, total;
        xui_internal_accessible_text_range* point;
        CHECK(found->ops->getRects(found, rectangles, 16, &count) == XUI_OK && count && count <= 16);
        point = create(editor, XUI_INTERNAL_TEXT_POINT, 0, rectangles[0].fX,
            rectangles[0].fY + rectangles[0].fH / 2);
        CHECK(point->ops->getOffsets(point, &first_offset, &last_offset, &total) == XUI_OK);
        CHECK(first_offset == last_offset && first_offset >= 10 && first_offset <= 14);
        point->ops->release(point);
        CHECK(caret->ops->setOffsets(caret, 2, 2) == XUI_DOC_ERROR_UTF8); offsets(caret, 4, 4);
        CHECK(caret->ops->setOffsets(caret, 1, 1) == XUI_OK);
        CHECK(caret->ops->expand(caret, XUI_INTERNAL_TEXT_GRAPHEME) == XUI_OK);
        offsets(caret, 1, 4); text(caret, "\xe4\xb8\xad");
        CHECK(caret->ops->setOffsets(caret, 4, 4) == XUI_OK);
    }
    /* Real RUNNING state, not a timing assumption. Queries use warmed caches. */
    atomic_store(&allocator.gate, 1);
    CHECK(xuiDocumentEditorFlush(editor) == XUI_DOC_ERROR_BUSY); prepared_text_wait(&allocator.entered);
    CHECK(prepared_text_info(editor).iState == XUI_DOC_PREPARE_RUNNING);
    calls = atomic_load(&allocator.calls);
    for (i = 0; i < 5000; i++) {
        offsets(found, 10, 14); text(found, "tail");
        CHECK(format->ops->setOffsets(format, 0, 0) == XUI_OK);
        CHECK(format->ops->expand(format, XUI_INTERNAL_TEXT_FORMAT) == XUI_OK);
        offsets(format, 0, mode == XUI_DOC_SOURCE_TEXT ? 21 : 8);
    }
    CHECK(atomic_load(&allocator.calls) == calls);
    format->ops->release(format);
    CHECK(xuiDocumentEditorInsertText(editor, "_\n_", 3) == XUI_OK);
    CHECK(prepared_text_info(editor).iGeneration != generation);
    text(pending, continued); text(base, continued); offsets(found, 13, 17); offsets(clone, 13, 17);
    offsets(retained, 13, 17); offsets(caret, 7, 7); offsets(selected, 7, 7);
    offsets(semantic, 13, 17);
    text_line_partition(editor, continued, mode == XUI_DOC_SOURCE_TEXT ? source_continued : live_continued,
        mode == XUI_DOC_SOURCE_TEXT ? sizeof(source_continued) / sizeof(*source_continued) : sizeof(live_continued) / sizeof(*live_continued));
    text_unit_partition(editor, continued, mode == XUI_DOC_SOURCE_TEXT ? source_format_continued : live_format_continued,
        mode == XUI_DOC_SOURCE_TEXT ? 2 : 7, XUI_INTERNAL_TEXT_FORMAT);
    CHECK(found->ops->select(found) == XUI_OK);
    CHECK(xuiDocumentViewGetSelection(editor, &selection) == XUI_OK);
    CHECK(selection.tAnchor.iInputGeneration == prepared_text_info(editor).iGeneration &&
        selection.tAnchor.iOffset == 13 && selection.tCaret.iOffset == 17);
    atomic_store(&allocator.resume, 1);
    CHECK(xuiDocumentEditorCancelInput(editor) == XUI_OK);
    CHECK(xuiDocumentGetRevision(document) == revision && !xuiDocumentHasPrepare(document));
    offsets(found, 7, 11); offsets(retained, 7, 11); text(found, "tail"); text(pending, original);
    offsets(semantic, 7, 11); text(semantic, "tail");
    offsets(caret, 1, 1); text(reader, original);
    /* New generation, publication, peer notification and common history. */
    CHECK(xuiEditSetSelection(editor, 1, 1) == XUI_OK);
    CHECK(xuiDocumentEditorInsertText(editor, "Z", 1) == XUI_OK); offsets(found, 8, 12);
    observer.editor = editor; observer.range = found; observer.generation = prepared_text_info(editor).iGeneration;
    observer.first = 8; observer.last = 12; observer.enabled = 1;
    CHECK(prepared_text_flush(editor) == XUI_OK && xuiDocumentGetRevision(document) == revision + 1);
    observer.enabled = 0; CHECK(observer.called == 1);
    offsets(found, 8, 12); text(found, "tail"); text(reader, "aZbc\n\n# tail\n\nlast\n");
    CHECK(xuiDocumentUndo(document, NULL) == XUI_OK); offsets(found, 7, 11);
    CHECK(xuiDocumentRedo(document, NULL) == XUI_OK); offsets(found, 8, 12);
    CHECK(xuiDocumentViewSetMode(editor, XUI_DOC_VISUAL) == XUI_OK); text(found, "tail");
    CHECK(xuiDocumentViewSetMode(editor, mode) == XUI_OK); offsets(found, 8, 12);
    CHECK(xuiEditSetSelection(editor, 1, 1) == XUI_OK);
    CHECK(xuiDocumentEditorInsertText(editor, "Y", 1) == XUI_OK); offsets(found, 9, 13);
    prepared_text_external_insert(document, 0, "!");
    CHECK(!xuiDocumentHasPrepare(document)); offsets(found, 9, 13); text(found, "tail");
    prepared_text_source(document, "!aZbc\n\n# tail\n\nlast\n");
    CHECK(xuiDocumentViewSetDocument(editor, document) == XUI_OK); text(found, "tail");
    /* Allocation failure of a fresh candidate projection is atomic and retryable. */
    CHECK(xuiEditSetSelection(editor, 2, 2) == XUI_OK);
    CHECK(xuiDocumentEditorInsertText(editor, "Q", 1) == XUI_OK);
    calls = atomic_load(&allocator.calls); atomic_store(&allocator.fail_at, calls + 1);
    CHECK(found->ops->getText(found, NULL, 0, &length) == XUI_ERROR_OUT_OF_MEMORY);
    CHECK(length == 0); atomic_store(&allocator.fail_at, UINT64_MAX);
    offsets(found, 10, 14); text(found, "tail");
    CHECK(xuiDocumentEditorCancelInput(editor) == XUI_OK); offsets(found, 9, 13);
    /* A failed worker retains usable ranges; a zero-operation retry changes
     * generation without moving endpoints, then publishes only once. */
    CHECK(xuiEditSetSelection(editor, 2, 2) == XUI_OK);
    CHECK(xuiDocumentEditorInsertText(editor, "R", 1) == XUI_OK);
    atomic_store(&allocator.fail_worker, 1);
    CHECK(prepared_text_flush(editor) == XUI_ERROR_OUT_OF_MEMORY);
    CHECK(prepared_text_info(editor).iState == XUI_DOC_PREPARE_FAILED);
    generation = prepared_text_info(editor).iGeneration;
    offsets(found, 10, 14); text(found, "tail");
    atomic_store(&allocator.fail_worker, 0);
    CHECK(xuiDocumentEditorRetryInput(editor) == XUI_OK && prepared_text_info(editor).iGeneration != generation);
    offsets(found, 10, 14); text(found, "tail");
    CHECK(prepared_text_flush(editor) == XUI_OK); offsets(found, 10, 14);
    CHECK(xuiDocumentUndo(document, NULL) == XUI_OK); offsets(found, 9, 13);
    revision = xuiDocumentGetRevision(document);
    CHECK(xuiDocumentEditorAppendStreamSource(editor, "", 0, 1) == XUI_OK);
    CHECK(!xuiDocumentHasPrepare(document)); offsets(found, 9, 13);
    CHECK(xuiEditSetSelection(editor, 2, 2) == XUI_OK);
    CHECK(xuiDocumentEditorInsertText(editor, "N", 1) == XUI_OK);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_BACKSPACE) == XUI_OK);
    CHECK(xuiDocumentHasPrepare(document)); offsets(found, 9, 13);
    CHECK(prepared_text_flush(editor) == XUI_OK);
    CHECK(xuiDocumentGetRevision(document) == revision + 1 && !xuiDocumentHasPrepare(document));
    prepared_text_source(document, "!aZbc\n\n# tail\n\nlast\n");
    /* Empty final input while idle is skipped; an open, empty stream creates
     * real zero-operation generations and publishes without a ChangeSet. */
    revision = xuiDocumentGetRevision(document);
    CHECK(xuiDocumentEditorAppendStreamSource(editor, "", 0, 0) == XUI_OK);
    CHECK(xuiDocumentHasPrepare(document));
    {
        xui_internal_accessible_text_range* empty_stream = prepared_text_document(editor);
        text(empty_stream, "!aZbc\n\n# tail\n\nlast\n");
        CHECK(xuiDocumentEditorAppendStreamSource(editor, "", 0, 1) == XUI_OK);
        CHECK(prepared_text_flush(editor) == XUI_OK);
        CHECK(xuiDocumentGetRevision(document) == revision && !xuiDocumentHasPrepare(document));
        text(empty_stream, "!aZbc\n\n# tail\n\nlast\n"); empty_stream->ops->release(empty_stream);
    }
    offsets(found, 9, 13); text(found, "tail");
    /* Split UTF8 stream input exposes complete code points only, and ranges
     * made in its successive generations survive cancellation. */
    CHECK(xuiDocumentEditorAppendStreamSource(editor, "\xe4\xb8", 2, 0) == XUI_OK);
    {
        xui_internal_accessible_text_range *stream = prepared_text_document(editor), *rune;
        text(stream, "!aZbc\n\n# tail\n\nlast\n");
        CHECK(xuiDocumentEditorAppendStreamSource(editor, "\xad", 1, 1) == XUI_OK);
        rune = prepared_text_document(editor); text(rune, "!aZbc\n\n# tail\n\nlast\n\xe4\xb8\xad");
        CHECK(rune->ops->setOffsets(rune, 20, 23) == XUI_OK); text(rune, "\xe4\xb8\xad");
        CHECK(xuiDocumentEditorCancelInput(editor) == XUI_OK);
        text(stream, "!aZbc\n\n# tail\n\nlast\n"); offsets(rune, 20, 20); text(rune, "");
        CHECK(xuiDocumentGetRevision(document) == revision);
        stream->ops->release(stream); rune->ops->release(rune);
    }
    CHECK(xuiDocumentCreate(&desc, &replacement) == XUI_OK);
    CHECK(xuiDocumentLoadMarkdown(replacement, "other\n", 6) == XUI_OK);
    CHECK(xuiDocumentViewSetDocument(editor, replacement) == XUI_OK);
    CHECK(found->ops->move(found, XUI_INTERNAL_TEXT_WORD, 1, &actual) == XUI_DOC_ERROR_STALE && !actual);
    base->ops->release(base); retained->ops->release(retained); pending->ops->release(pending);
    caret->ops->release(caret); selected->ops->release(selected); clone->ops->release(clone); semantic->ops->release(semantic);
    found->ops->release(found); xuiDocumentUnsubscribe(document, token);
    xuiWidgetDestroy(view); xuiDestroy(context);
    CHECK(reader->ops->getText(reader, NULL, 0, &length) == XUI_DOC_ERROR_STALE);
    reader->ops->release(reader); xuiDocumentRelease(document); xuiDocumentRelease(replacement);
    CHECK(!atomic_load(&allocator.live)); proxy.tProxy.fontDestroy(&proxy.tProxy, font);
    printf("Prepared text ranges: mode=%u continuation/cancel/publish/history/external/OOM/RUNNING passed\n", mode);
}
static void text_prepared_cases(void)
{
    prepared_text_case(XUI_DOC_SOURCE_TEXT);
    prepared_text_case(XUI_DOC_LIVE_MARKDOWN);
}
