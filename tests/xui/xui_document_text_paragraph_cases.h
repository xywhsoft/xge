static void text_paragraph_partition(xui_widget view, const char* expected,
    const uint64_t* edges, size_t count)
{ text_unit_partition(view, expected, edges, count, XUI_INTERNAL_TEXT_PARAGRAPH); }
typedef struct text_paragraph_allocator {
    uint64_t calls, fail_call, live;
} text_paragraph_allocator;
static void* text_paragraph_alloc(void* user, size_t bytes)
{
    text_paragraph_allocator* state = user; void* memory;
    state->calls++;
    if (state->calls == state->fail_call) return NULL;
    memory = malloc(bytes); if (memory) state->live++; return memory;
}
static void text_paragraph_free(void* user, void* memory)
{
    text_paragraph_allocator* state = user;
    if (memory) { CHECK(state->live); state->live--; free(memory); }
}
static void text_paragraph_oom(int profile, uint32_t mode)
{
    static const char source[] = "aa\n\nbb\n";
    text_paragraph_allocator allocator = {0}; xui_test_proxy_state_t proxy;
    xui_context context; xui_font font; xui_document document; xui_widget editor, view;
    xui_doc_desc_t desc = {0}; xui_doc_editor_desc_t ed = {0}; xui_doc_view_desc_t vd = {0};
    xui_internal_accessible_text_range* range;
    uint64_t revision, calls, live, first_end = mode == XUI_DOC_VISUAL ? 3 : 4; int action, actual;
    xuiTestProxyInit(&proxy); CHECK(xuiCreate(&context) == XUI_OK);
    CHECK(xuiSetProxy(context, &proxy.tProxy) == XUI_OK);
    CHECK(proxy.tProxy.fontLoadFile(&proxy.tProxy, &font, "test.ttf", 14, 0) == XUI_OK);
    CHECK(xuiSetDefaultFont(context, font) == XUI_OK);
    desc.iSize = sizeof(desc); desc.iProfile = (uint32_t)profile;
    desc.onAlloc = text_paragraph_alloc; desc.onFree = text_paragraph_free; desc.pAllocatorUser = &allocator;
    CHECK(xuiDocumentCreate(&desc, &document) == XUI_OK);
    if (profile == XUI_DOCUMENT_MARKDOWN) CHECK(xuiDocumentLoadMarkdown(document, source, strlen(source)) == XUI_OK);
    else {
        ed.iSize = sizeof(ed); ed.iMode = mode; ed.tView.iSize = sizeof(ed.tView); ed.tView.pDocument = document;
        CHECK(xuiDocumentEditorCreate(context, &ed, &editor) == XUI_OK);
        CHECK(xuiDocumentEditorInsertText(editor, source, strlen(source) - 1) == XUI_OK);
        CHECK(xuiDocumentEditorFlush(editor) == XUI_OK); xuiWidgetDestroy(editor);
    }
    revision = xuiDocumentGetRevision(document);
    for (action = 0; action < 3; action++) {
        vd.iSize = sizeof(vd); vd.pDocument = document;
        CHECK(xuiDocumentViewCreate(context, &vd, &view) == XUI_OK);
        CHECK(xuiDocumentViewSetMode(view, mode) == XUI_OK);
        range = create(view, XUI_INTERNAL_TEXT_DOCUMENT, 0, 0, 0);
        CHECK(range->ops->setOffsets(range, action ? 0 : 1, action ? 0 : 1) == XUI_OK);
        calls = allocator.calls; live = allocator.live; allocator.fail_call = calls + 1; actual = 77;
        if (!action) CHECK(range->ops->expand(range, XUI_INTERNAL_TEXT_PARAGRAPH) == XUI_ERROR_OUT_OF_MEMORY);
        else if (action == 1) CHECK(range->ops->move(range, XUI_INTERNAL_TEXT_PARAGRAPH, 1, &actual) == XUI_ERROR_OUT_OF_MEMORY && !actual);
        else CHECK(range->ops->moveEndpoint(range, XUI_INTERNAL_TEXT_START, XUI_INTERNAL_TEXT_PARAGRAPH, 1, &actual) == XUI_ERROR_OUT_OF_MEMORY && !actual);
        CHECK(allocator.calls == calls + 1 && allocator.live == live);
        CHECK(xuiDocumentGetRevision(document) == revision);
        offsets(range, action ? 0 : 1, action ? 0 : 1);
        allocator.fail_call = 0;
        if (!action) { CHECK(range->ops->expand(range, XUI_INTERNAL_TEXT_PARAGRAPH) == XUI_OK); offsets(range, 0, first_end); }
        else if (action == 1) { CHECK(range->ops->move(range, XUI_INTERNAL_TEXT_PARAGRAPH, 1, &actual) == XUI_OK && actual == 1); offsets(range, first_end, first_end); }
        else { CHECK(range->ops->moveEndpoint(range, XUI_INTERNAL_TEXT_START, XUI_INTERNAL_TEXT_PARAGRAPH, 1, &actual) == XUI_OK && actual == 1); offsets(range, first_end, first_end); }
        CHECK(allocator.calls == calls + 2); calls = allocator.calls;
        CHECK(range->ops->setOffsets(range, 0, 0) == XUI_OK);
        CHECK(range->ops->move(range, XUI_INTERNAL_TEXT_PARAGRAPH, INT_MAX, &actual) == XUI_OK);
        CHECK(range->ops->move(range, XUI_INTERNAL_TEXT_PARAGRAPH, INT_MIN, &actual) == XUI_OK);
        CHECK(range->ops->expand(range, XUI_INTERNAL_TEXT_PARAGRAPH) == XUI_OK);
        CHECK(allocator.calls == calls);
        range->ops->release(range); xuiWidgetDestroy(view);
    }
    xuiDestroy(context); xuiDocumentRelease(document); CHECK(!allocator.live);
    proxy.tProxy.fontDestroy(&proxy.tProxy, font);
    printf("Document paragraph cache OOM/atomic/retry/no-query-alloc/leaks passed: profile=%d mode=%u\n", profile, mode);
}
static void text_paragraph_history(int profile, uint32_t mode)
{
    static const uint64_t semantic[] = {0, 3, 6}, source[] = {0, 4, 7};
    static const uint64_t added_semantic[] = {0, 3, 6, 9}, added_source[] = {0, 4, 8, 11};
    static const uint64_t replacement_semantic[] = {0, 2, 4}, replacement_source[] = {0, 3, 5};
    xui_test_proxy_state_t proxy; xui_context context; xui_font font;
    xui_document document, replacement; xui_document_transaction transaction;
    xui_doc_desc_t desc = {0}; xui_widget views[2];
    xui_doc_txn_desc_t txn_desc = {0};
    xui_internal_accessible_text_range* ranges[2]; uint64_t paragraph, a, b, length;
    int i, stage, visual = mode == XUI_DOC_VISUAL;
    xuiTestProxyInit(&proxy); CHECK(xuiCreate(&context) == XUI_OK);
    CHECK(xuiSetProxy(context, &proxy.tProxy) == XUI_OK);
    CHECK(proxy.tProxy.fontLoadFile(&proxy.tProxy, &font, "test.ttf", 14, 0) == XUI_OK);
    CHECK(xuiSetDefaultFont(context, font) == XUI_OK);
    desc.iSize = sizeof(desc); desc.iProfile = (uint32_t)profile;
    CHECK(xuiDocumentCreate(&desc, &document) == XUI_OK);
    if (profile == XUI_DOCUMENT_MARKDOWN) CHECK(xuiDocumentLoadMarkdown(document, "aa\n\nbb\n", 7) == XUI_OK);
    else {
        CHECK(xuiDocumentBeginTransaction(document, NULL, &transaction) == XUI_OK);
        paragraph = line_add(transaction, 1, XUI_DOC_PARAGRAPH, NULL); line_add(transaction, paragraph, XUI_DOC_TEXT, "aa");
        paragraph = line_add(transaction, 1, XUI_DOC_PARAGRAPH, NULL); line_add(transaction, paragraph, XUI_DOC_TEXT, "bb");
        CHECK(xuiDocumentTxnCommit(transaction, NULL) == XUI_OK); xuiDocumentTxnRelease(transaction);
    }
    for (i = 0; i < 2; i++) {
        views[i] = text_line_view(context, document, mode, 600);
        text_paragraph_partition(views[i], visual ? "aa\nbb\n" : "aa\n\nbb\n", visual ? semantic : source, 3);
        ranges[i] = create(views[i], XUI_INTERNAL_TEXT_DOCUMENT, 0, 0, 0);
        CHECK(ranges[i]->ops->setOffsets(ranges[i], 1, 1) == XUI_OK);
    }
    txn_desc.iSize = sizeof(txn_desc);
    txn_desc.iDomain = profile == XUI_DOCUMENT_MARKDOWN ? XUI_DOC_SOURCE : XUI_DOC_SEMANTIC;
    CHECK(xuiDocumentBeginTransaction(document, &txn_desc, &transaction) == XUI_OK);
    if (profile == XUI_DOCUMENT_MARKDOWN) CHECK(xuiDocumentTxnReplaceSource(transaction, 0, 0, "cc\n\n", 4) == XUI_OK);
    else {
        xui_doc_node_desc_t node = {0}; node.iSize = sizeof(node); node.iKind = XUI_DOC_PARAGRAPH;
        CHECK(xuiDocumentTxnInsertNode(transaction, 1, 0, &node, &paragraph) == XUI_OK);
        line_add(transaction, paragraph, XUI_DOC_TEXT, "cc");
    }
    CHECK(xuiDocumentTxnCommit(transaction, NULL) == XUI_OK); xuiDocumentTxnRelease(transaction);
    for (stage = 0; stage < 3; stage++) {
        int added = stage != 1;
        for (i = 0; i < 2; i++) {
            text_paragraph_partition(views[i], added ? (visual ? "cc\naa\nbb\n" : "cc\n\naa\n\nbb\n") :
                (visual ? "aa\nbb\n" : "aa\n\nbb\n"), added ? (visual ? added_semantic : added_source) :
                (visual ? semantic : source), added ? 4 : 3);
            CHECK(ranges[i]->ops->expand(ranges[i], XUI_INTERNAL_TEXT_PARAGRAPH) == XUI_OK);
            offsets(ranges[i], added ? (visual ? 3 : 4) : 0, added ? (visual ? 6 : 8) : (visual ? 3 : 4));
        }
        if (!stage) CHECK(xuiDocumentUndo(document, NULL) == XUI_OK);
        if (stage == 1) CHECK(xuiDocumentRedo(document, NULL) == XUI_OK);
    }
    CHECK(xuiDocumentViewSetDocument(views[0], document) == XUI_OK);
    CHECK(ranges[0]->ops->expand(ranges[0], XUI_INTERNAL_TEXT_PARAGRAPH) == XUI_OK);
    offsets(ranges[0], visual ? 3 : 4, visual ? 6 : 8);
    CHECK(xuiDocumentCreate(&desc, &replacement) == XUI_OK);
    if (profile == XUI_DOCUMENT_MARKDOWN) CHECK(xuiDocumentLoadMarkdown(replacement, "q\n\nr\n", 5) == XUI_OK);
    else {
        CHECK(xuiDocumentBeginTransaction(replacement, NULL, &transaction) == XUI_OK);
        paragraph = line_add(transaction, 1, XUI_DOC_PARAGRAPH, NULL); line_add(transaction, paragraph, XUI_DOC_TEXT, "q");
        paragraph = line_add(transaction, 1, XUI_DOC_PARAGRAPH, NULL); line_add(transaction, paragraph, XUI_DOC_TEXT, "r");
        CHECK(xuiDocumentTxnCommit(transaction, NULL) == XUI_OK); xuiDocumentTxnRelease(transaction);
    }
    CHECK(xuiDocumentViewSetDocument(views[0], replacement) == XUI_OK);
    CHECK(ranges[0]->ops->getOffsets(ranges[0], &a, &b, &length) == XUI_DOC_ERROR_STALE);
    text_paragraph_partition(views[0], visual ? "q\nr\n" : "q\n\nr\n", visual ? replacement_semantic : replacement_source, 3);
    CHECK(ranges[1]->ops->expand(ranges[1], XUI_INTERNAL_TEXT_PARAGRAPH) == XUI_OK);
    offsets(ranges[1], visual ? 3 : 4, visual ? 6 : 8);
    for (i = 0; i < 2; i++) {
        xuiWidgetDestroy(views[i]);
        CHECK(ranges[i]->ops->expand(ranges[i], XUI_INTERNAL_TEXT_PARAGRAPH) == XUI_DOC_ERROR_STALE);
        ranges[i]->ops->release(ranges[i]);
    }
    xuiDestroy(context); xuiDocumentRelease(document); xuiDocumentRelease(replacement);
    proxy.tProxy.fontDestroy(&proxy.tProxy, font);
    printf("Document paragraph two-view/edit/Undo/Redo/replacement/destroy passed: profile=%d mode=%u\n", profile, mode);
}
static void text_paragraph_source_scale(uint64_t bytes)
{
    text_paragraph_allocator allocator = {0}; xui_test_proxy_state_t proxy;
    xui_context context; xui_font font; xui_document document; xui_widget view;
    xui_doc_desc_t desc = {0}; xui_doc_view_desc_t vd = {0}; xui_internal_accessible_text_range* range;
    uint64_t calls, first_end = bytes / 2 + 2, at; int i, actual;
    uint64_t points[6] = {0, bytes / 2 - 1, bytes / 2, bytes / 2 + 1, bytes / 2 + 2, bytes};
    char* input = malloc((size_t)bytes + 1); CHECK(input);
    memset(input, 'a', (size_t)bytes); input[bytes] = 0;
    input[bytes / 2] = input[bytes / 2 + 1] = '\n';
    xuiTestProxyInit(&proxy); CHECK(xuiCreate(&context) == XUI_OK);
    CHECK(xuiSetProxy(context, &proxy.tProxy) == XUI_OK);
    CHECK(proxy.tProxy.fontLoadFile(&proxy.tProxy, &font, "test.ttf", 14, 0) == XUI_OK);
    CHECK(xuiSetDefaultFont(context, font) == XUI_OK);
    desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN;
    desc.onAlloc = text_paragraph_alloc; desc.onFree = text_paragraph_free; desc.pAllocatorUser = &allocator;
    CHECK(xuiDocumentCreate(&desc, &document) == XUI_OK);
    CHECK(xuiDocumentLoadMarkdown(document, input, bytes) == XUI_OK); free(input);
    vd.iSize = sizeof(vd); vd.pDocument = document; CHECK(xuiDocumentViewCreate(context, &vd, &view) == XUI_OK);
    CHECK(xuiDocumentViewSetMode(view, XUI_DOC_SOURCE_TEXT) == XUI_OK);
    range = create(view, XUI_INTERNAL_TEXT_DOCUMENT, 0, 0, 0);
    CHECK(range->ops->setOffsets(range, 0, 0) == XUI_OK); calls = allocator.calls;
    CHECK(range->ops->expand(range, XUI_INTERNAL_TEXT_PARAGRAPH) == XUI_OK); offsets(range, 0, first_end);
    CHECK(allocator.calls == calls + 1); calls = allocator.calls;
    for (i = 0; i < 6; i++) {
        at = points[i]; CHECK(range->ops->setOffsets(range, at, at) == XUI_OK);
        CHECK(range->ops->expand(range, XUI_INTERNAL_TEXT_PARAGRAPH) == XUI_OK);
        offsets(range, at < first_end ? 0 : first_end, at < first_end ? first_end : bytes);
    }
    CHECK(range->ops->setOffsets(range, 0, 0) == XUI_OK);
    CHECK(range->ops->move(range, XUI_INTERNAL_TEXT_PARAGRAPH, INT_MAX, &actual) == XUI_OK && actual == 2);
    CHECK(range->ops->move(range, XUI_INTERNAL_TEXT_PARAGRAPH, INT_MIN, &actual) == XUI_OK && actual == -2);
    CHECK(allocator.calls == calls);
    range->ops->release(range); xuiWidgetDestroy(view); xuiDestroy(context); xuiDocumentRelease(document);
    CHECK(!allocator.live); proxy.tProxy.fontDestroy(&proxy.tProxy, font);
    printf("Document source paragraph %llu-byte start/separator/next/EOF cached no-query-alloc/leaks passed\n", (unsigned long long)bytes);
}
static void text_paragraph_cases(void)
{
    xui_test_proxy_state_t proxy; xui_context context; xui_font font;
    xui_document document; xui_document_transaction transaction; xui_widget view;
    xui_doc_desc_t desc = {0}; uint64_t paragraph, table, row, cell;
    static const uint64_t wrap[] = {0, 7, 8, 11}, hard[] = {0, 5, 6, 8};
    static const uint64_t cells[] = {0, 2, 4, 6}, code[] = {0, 12, 14};
    static const uint64_t raw[] = {0, 10, 18}, visual[] = {0, 5, 12};
    static const uint64_t leading[] = {0, 3, 10, 11}, unicode_edges[] = {0, 4, 9, 13};
    static const uint64_t blank[] = {0, 4}, definitions[] = {0, 14};
    xuiTestProxyInit(&proxy); CHECK(xuiCreate(&context) == XUI_OK);
    CHECK(xuiSetProxy(context, &proxy.tProxy) == XUI_OK);
    CHECK(proxy.tProxy.fontLoadFile(&proxy.tProxy, &font, "test.ttf", 14, 0) == XUI_OK);
    CHECK(xuiSetDefaultFont(context, font) == XUI_OK);
    CHECK(xuiDocumentCreate(NULL, &document) == XUI_OK);
    CHECK(xuiDocumentBeginTransaction(document, NULL, &transaction) == XUI_OK);
    paragraph = line_add(transaction, 1, XUI_DOC_PARAGRAPH, NULL); line_add(transaction, paragraph, XUI_DOC_TEXT, "abcdef");
    line_add(transaction, 1, XUI_DOC_PARAGRAPH, NULL);
    paragraph = line_add(transaction, 1, XUI_DOC_PARAGRAPH, NULL); line_add(transaction, paragraph, XUI_DOC_TEXT, "xy");
    CHECK(xuiDocumentTxnCommit(transaction, NULL) == XUI_OK); xuiDocumentTxnRelease(transaction);
    view = text_line_view(context, document, XUI_DOC_VISUAL, 21);
    text_paragraph_partition(view, "abcdef\n\nxy\n", wrap, sizeof(wrap) / sizeof(*wrap));
    xuiWidgetDestroy(view); xuiDocumentRelease(document);
    CHECK(xuiDocumentCreate(NULL, &document) == XUI_OK);
    CHECK(xuiDocumentBeginTransaction(document, NULL, &transaction) == XUI_OK);
    paragraph = line_add(transaction, 1, XUI_DOC_PARAGRAPH, NULL);
    line_add(transaction, paragraph, XUI_DOC_TEXT, "a"); line_add(transaction, paragraph, XUI_DOC_SOFT_BREAK, NULL);
    line_add(transaction, paragraph, XUI_DOC_TEXT, "b"); line_add(transaction, paragraph, XUI_DOC_HARD_BREAK, NULL);
    line_add(transaction, 1, XUI_DOC_PARAGRAPH, NULL);
    paragraph = line_add(transaction, 1, XUI_DOC_PARAGRAPH, NULL); line_add(transaction, paragraph, XUI_DOC_TEXT, "z");
    CHECK(xuiDocumentTxnCommit(transaction, NULL) == XUI_OK); xuiDocumentTxnRelease(transaction);
    view = text_line_view(context, document, XUI_DOC_VISUAL, 600);
    text_paragraph_partition(view, "a\nb\n\n\nz\n", hard, sizeof(hard) / sizeof(*hard));
    xuiWidgetDestroy(view); xuiDocumentRelease(document);
    CHECK(xuiDocumentCreate(NULL, &document) == XUI_OK);
    CHECK(xuiDocumentBeginTransaction(document, NULL, &transaction) == XUI_OK);
    table = line_add(transaction, 1, XUI_DOC_TABLE, NULL); row = line_add(transaction, table, XUI_DOC_ROW, NULL);
    cell = line_add(transaction, row, XUI_DOC_CELL, NULL);
    paragraph = line_add(transaction, cell, XUI_DOC_PARAGRAPH, NULL); line_add(transaction, paragraph, XUI_DOC_TEXT, "a");
    paragraph = line_add(transaction, cell, XUI_DOC_PARAGRAPH, NULL); line_add(transaction, paragraph, XUI_DOC_TEXT, "b");
    cell = line_add(transaction, row, XUI_DOC_CELL, NULL);
    paragraph = line_add(transaction, cell, XUI_DOC_PARAGRAPH, NULL); line_add(transaction, paragraph, XUI_DOC_TEXT, "c");
    CHECK(xuiDocumentTxnCommit(transaction, NULL) == XUI_OK); xuiDocumentTxnRelease(transaction);
    view = text_line_view(context, document, XUI_DOC_VISUAL, 600);
    text_paragraph_partition(view, "a\nb\tc\n", cells, sizeof(cells) / sizeof(*cells));
    xuiWidgetDestroy(view); xuiDocumentRelease(document);
    CHECK(xuiDocumentCreate(NULL, &document) == XUI_OK);
    CHECK(xuiDocumentBeginTransaction(document, NULL, &transaction) == XUI_OK);
    line_add(transaction, 1, XUI_DOC_CODE_BLOCK, "one\n\nsecond");
    paragraph = line_add(transaction, 1, XUI_DOC_PARAGRAPH, NULL); line_add(transaction, paragraph, XUI_DOC_TEXT, "x");
    CHECK(xuiDocumentTxnCommit(transaction, NULL) == XUI_OK); xuiDocumentTxnRelease(transaction);
    view = text_line_view(context, document, XUI_DOC_VISUAL, 600);
    text_paragraph_partition(view, "one\n\nsecond\nx\n", code, sizeof(code) / sizeof(*code));
    xuiWidgetDestroy(view); xuiDocumentRelease(document);
    desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN;
    CHECK(xuiDocumentCreate(&desc, &document) == XUI_OK);
    CHECK(xuiDocumentLoadMarkdown(document, "# head\r\n\r\nabcdef\r\n", 18) == XUI_OK);
    view = text_line_view(context, document, XUI_DOC_SOURCE_TEXT, 21);
    text_paragraph_partition(view, "# head\r\n\r\nabcdef\r\n", raw, sizeof(raw) / sizeof(*raw));
    CHECK(xuiDocumentViewSetMode(view, XUI_DOC_VISUAL) == XUI_OK);
    text_paragraph_partition(view, "head\nabcdef\n", visual, sizeof(visual) / sizeof(*visual));
    CHECK(xuiDocumentViewSetMode(view, XUI_DOC_LIVE_MARKDOWN) == XUI_OK);
    text_paragraph_partition(view, "# head\r\n\r\nabcdef\r\n", raw, sizeof(raw) / sizeof(*raw));
    CHECK(xuiDocumentViewSetMode(view, XUI_DOC_SOURCE_TEXT) == XUI_OK);
    CHECK(xuiDocumentLoadMarkdown(document, "\n\r\nx\r\n \t\r\nz", 11) == XUI_OK);
    text_paragraph_partition(view, "\n\r\nx\r\n \t\r\nz", leading, sizeof(leading) / sizeof(*leading));
    CHECK(xuiDocumentLoadMarkdown(document, "\xce\xb1\n\n\xe4\xb8\xad\n\n\xf0\x9f\x98\x80", 13) == XUI_OK);
    text_paragraph_partition(view, "\xce\xb1\n\n\xe4\xb8\xad\n\n\xf0\x9f\x98\x80", unicode_edges, sizeof(unicode_edges) / sizeof(*unicode_edges));
    CHECK(xuiDocumentLoadMarkdown(document, " \t\n\n", 4) == XUI_OK);
    text_paragraph_partition(view, " \t\n\n", blank, sizeof(blank) / sizeof(*blank));
    CHECK(xuiDocumentLoadMarkdown(document, "[id]: /target\n", 14) == XUI_OK);
    text_paragraph_partition(view, "[id]: /target\n", definitions, sizeof(definitions) / sizeof(*definitions));
    CHECK(xuiDocumentLoadMarkdown(document, "", 0) == XUI_OK);
    {
        xui_internal_accessible_text_range* range = create(view, XUI_INTERNAL_TEXT_DOCUMENT, 0, 0, 0); int actual;
        CHECK(range->ops->expand(range, XUI_INTERNAL_TEXT_PARAGRAPH) == XUI_OK); offsets(range, 0, 0);
        CHECK(range->ops->move(range, XUI_INTERNAL_TEXT_PARAGRAPH, INT_MAX, &actual) == XUI_OK && !actual);
        CHECK(range->ops->moveEndpoint(range, XUI_INTERNAL_TEXT_START, XUI_INTERNAL_TEXT_PARAGRAPH, INT_MIN, &actual) == XUI_OK && !actual);
        range->ops->release(range);
    }
    xuiWidgetDestroy(view); xuiDocumentRelease(document);
    xuiDestroy(context); proxy.tProxy.fontDestroy(&proxy.tProxy, font);
    text_paragraph_oom(XUI_DOCUMENT_RICH, XUI_DOC_VISUAL);
    text_paragraph_oom(XUI_DOCUMENT_MARKDOWN, XUI_DOC_VISUAL);
    text_paragraph_oom(XUI_DOCUMENT_MARKDOWN, XUI_DOC_SOURCE_TEXT);
    text_paragraph_oom(XUI_DOCUMENT_MARKDOWN, XUI_DOC_LIVE_MARKDOWN);
    text_paragraph_history(XUI_DOCUMENT_RICH, XUI_DOC_VISUAL);
    text_paragraph_history(XUI_DOCUMENT_MARKDOWN, XUI_DOC_VISUAL);
    text_paragraph_history(XUI_DOCUMENT_MARKDOWN, XUI_DOC_SOURCE_TEXT);
    text_paragraph_history(XUI_DOCUMENT_MARKDOWN, XUI_DOC_LIVE_MARKDOWN);
    text_paragraph_source_scale(1024 * 1024);
    text_paragraph_source_scale(10 * 1024 * 1024);
    puts("Document paragraph units: semantic/explicit-empty/breaks/code/cells/raw-blank/CRLF/UTF8/mode/cache/EOF/count/endpoints passed");
}
