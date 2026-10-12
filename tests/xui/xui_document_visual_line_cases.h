/* Public renderer row queries with independent logical endpoint goldens. */
static uint64_t line_add(xui_document_transaction transaction, uint64_t parent,
    unsigned kind, const char* value)
{
    xui_doc_node_desc_t desc = {0}; uint64_t id;
    desc.iSize = sizeof(desc); desc.iKind = kind; desc.sText = value;
    desc.iTextBytes = value ? strlen(value) : 0;
    CHECK(xuiDocumentTxnInsertNode(transaction, parent, XUI_DOCUMENT_APPEND, &desc, &id) == XUI_OK);
    return id;
}
static xui_doc_position_t line_position(xui_document document, uint64_t node,
    unsigned kind, uint64_t offset, unsigned affinity)
{
    xui_doc_position_t p = {0}; p.iSize = sizeof(p);
    p.iDocumentId = xuiDocumentGetIdentity(document); p.iRevision = xuiDocumentGetRevision(document);
    p.iNodeId = node; p.iKind = kind; p.iOffset = offset; p.iAffinity = affinity;
    return p;
}
static void line_expect(xui_document_renderer renderer, xui_doc_position_t p,
    uint64_t first_node, unsigned first_kind, uint64_t first,
    uint64_t last_node, unsigned last_kind, uint64_t last)
{
    xui_doc_range_t row;
    int result = xuiDocumentRendererGetLineRange(renderer, &p, &row);
    if (result != XUI_OK) fprintf(stderr, "Visual row query node=%llu offset=%llu result=%d\n",
        (unsigned long long)p.iNodeId, (unsigned long long)p.iOffset, result);
    CHECK(result == XUI_OK);
    if (row.tAnchor.iNodeId != first_node || row.tAnchor.iOffset != first ||
        row.tCaret.iNodeId != last_node || row.tCaret.iOffset != last)
        fprintf(stderr, "Visual row expected %llu:%llu..%llu:%llu; got %llu:%llu..%llu:%llu\n",
            (unsigned long long)first_node, (unsigned long long)first,
            (unsigned long long)last_node, (unsigned long long)last,
            (unsigned long long)row.tAnchor.iNodeId, (unsigned long long)row.tAnchor.iOffset,
            (unsigned long long)row.tCaret.iNodeId, (unsigned long long)row.tCaret.iOffset);
    CHECK(row.tAnchor.iNodeId == first_node && row.tAnchor.iKind == first_kind && row.tAnchor.iOffset == first);
    CHECK(row.tCaret.iNodeId == last_node && row.tCaret.iKind == last_kind && row.tCaret.iOffset == last);
    CHECK(row.tAnchor.iDocumentId == p.iDocumentId && row.tCaret.iRevision == p.iRevision);
    CHECK(row.tAnchor.iInputGeneration == p.iInputGeneration && row.tCaret.iInputGeneration == p.iInputGeneration);
}
static void line_snapshot(xui_document document, xui_document_renderer renderer, double width)
{
    xui_document_snapshot snapshot;
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
    CHECK(xuiDocumentRendererSetSnapshot(renderer, snapshot, NULL) == XUI_OK);
    xuiDocumentSnapshotRelease(snapshot);
    CHECK(xuiDocumentRendererLayout(renderer, width, 0, 100) == XUI_OK);
}
static void visual_line_cases(void)
{
    xui_test_proxy_state_t proxy; xui_context context; xui_font font;
    xui_document document; xui_document_transaction transaction; xui_document_renderer renderer;
    xui_doc_attributes_t attrs = {0}; uint64_t paragraph, a, b, c, table, row, cell, nested, last, code;
    xui_doc_position_t p; xui_doc_range_t unchanged, output; int sample;
    xuiTestProxyInit(&proxy); CHECK(xuiCreate(&context) == XUI_OK);
    CHECK(xuiSetProxy(context, &proxy.tProxy) == XUI_OK);
    CHECK(proxy.tProxy.fontLoadFile(&proxy.tProxy, &font, "test.ttf", 14, 0) == XUI_OK);
    CHECK(xuiSetDefaultFont(context, font) == XUI_OK);
    CHECK(xuiDocumentRendererCreate(context, NULL, &renderer) == XUI_OK);
    CHECK(xuiDocumentCreate(NULL, &document) == XUI_OK);
    CHECK(xuiDocumentBeginTransaction(document, NULL, &transaction) == XUI_OK);
    paragraph = line_add(transaction, 1, XUI_DOC_PARAGRAPH, NULL);
    a = line_add(transaction, paragraph, XUI_DOC_TEXT, "ab");
    b = line_add(transaction, paragraph, XUI_DOC_TEXT, "CD");
    c = line_add(transaction, paragraph, XUI_DOC_TEXT, "efgh");
    attrs.fFontSize = 28; CHECK(xuiDocumentTxnSetAttributes(transaction, b, &attrs) == XUI_OK);
    CHECK(xuiDocumentTxnCommit(transaction, NULL) == XUI_OK); xuiDocumentTxnRelease(transaction);
    line_snapshot(document, renderer, 49);
    p = line_position(document, b, XUI_DOC_POSITION_TEXT, 1, XUI_DOC_AFTER);
    line_expect(renderer, p, a, XUI_DOC_POSITION_TEXT, 0, c, XUI_DOC_POSITION_TEXT, 1);
    p = line_position(document, c, XUI_DOC_POSITION_TEXT, 1, XUI_DOC_BEFORE);
    line_expect(renderer, p, a, XUI_DOC_POSITION_TEXT, 0, c, XUI_DOC_POSITION_TEXT, 1);
    p.iAffinity = XUI_DOC_AFTER;
    line_expect(renderer, p, c, XUI_DOC_POSITION_TEXT, 1, c, XUI_DOC_POSITION_TEXT, 4);
    CHECK(xuiDocumentRendererLayout(renderer, 100, 0, 100) == XUI_OK);
    line_expect(renderer, p, a, XUI_DOC_POSITION_TEXT, 0, c, XUI_DOC_POSITION_TEXT, 4);
    memset(&unchanged, 0xa5, sizeof(unchanged)); output = unchanged;
    p.iRevision--; CHECK(xuiDocumentRendererGetLineRange(renderer, &p, &output) == XUI_DOC_ERROR_STALE);
    CHECK(!memcmp(&output, &unchanged, sizeof(output)));
    CHECK(xuiDocumentRendererGetLineRange(renderer, NULL, &output) == XUI_ERROR_INVALID_ARGUMENT);
    CHECK(!memcmp(&output, &unchanged, sizeof(output)));
    xuiDocumentRelease(document);

    CHECK(xuiDocumentCreate(NULL, &document) == XUI_OK);
    CHECK(xuiDocumentBeginTransaction(document, NULL, &transaction) == XUI_OK);
    paragraph = line_add(transaction, 1, XUI_DOC_PARAGRAPH, NULL);
    a = line_add(transaction, paragraph, XUI_DOC_TEXT, "abc");
    line_add(transaction, paragraph, XUI_DOC_HARD_BREAK, NULL);
    b = line_add(transaction, paragraph, XUI_DOC_TEXT, "de");
    line_add(transaction, paragraph, XUI_DOC_HARD_BREAK, NULL);
    CHECK(xuiDocumentTxnCommit(transaction, NULL) == XUI_OK); xuiDocumentTxnRelease(transaction);
    line_snapshot(document, renderer, 100);
    p = line_position(document, a, XUI_DOC_POSITION_TEXT, 1, XUI_DOC_AFTER);
    line_expect(renderer, p, a, XUI_DOC_POSITION_TEXT, 0, paragraph, XUI_DOC_POSITION_GAP, 2);
    p = line_position(document, b, XUI_DOC_POSITION_TEXT, 1, XUI_DOC_AFTER);
    line_expect(renderer, p, b, XUI_DOC_POSITION_TEXT, 0, paragraph, XUI_DOC_POSITION_GAP, 4);
    p = line_position(document, paragraph, XUI_DOC_POSITION_GAP, 4, XUI_DOC_AFTER);
    line_expect(renderer, p, paragraph, XUI_DOC_POSITION_GAP, 4, paragraph, XUI_DOC_POSITION_GAP, 4);
    xuiDocumentRelease(document);

    CHECK(xuiDocumentCreate(NULL, &document) == XUI_OK);
    CHECK(xuiDocumentBeginTransaction(document, NULL, &transaction) == XUI_OK);
    table = line_add(transaction, 1, XUI_DOC_TABLE, NULL); row = line_add(transaction, table, XUI_DOC_ROW, NULL);
    cell = line_add(transaction, row, XUI_DOC_CELL, NULL); paragraph = line_add(transaction, cell, XUI_DOC_PARAGRAPH, NULL);
    a = line_add(transaction, paragraph, XUI_DOC_TEXT, "left");
    cell = line_add(transaction, row, XUI_DOC_CELL, NULL); paragraph = line_add(transaction, cell, XUI_DOC_PARAGRAPH, NULL);
    b = line_add(transaction, paragraph, XUI_DOC_TEXT, "right");
    nested = line_add(transaction, cell, XUI_DOC_TABLE, NULL); last = line_add(transaction, nested, XUI_DOC_ROW, NULL);
    cell = line_add(transaction, last, XUI_DOC_CELL, NULL); paragraph = line_add(transaction, cell, XUI_DOC_PARAGRAPH, NULL);
    c = line_add(transaction, paragraph, XUI_DOC_TEXT, "\xd7\x90\xd7\x91\xd7\x92");
    cell = line_add(transaction, last, XUI_DOC_CELL, NULL);
    CHECK(xuiDocumentTxnCommit(transaction, NULL) == XUI_OK); xuiDocumentTxnRelease(transaction);
    line_snapshot(document, renderer, 600);
    p = line_position(document, a, XUI_DOC_POSITION_TEXT, 1, XUI_DOC_AFTER);
    line_expect(renderer, p, a, XUI_DOC_POSITION_TEXT, 0, a, XUI_DOC_POSITION_TEXT, 4);
    p = line_position(document, b, XUI_DOC_POSITION_TEXT, 1, XUI_DOC_AFTER);
    line_expect(renderer, p, b, XUI_DOC_POSITION_TEXT, 0, b, XUI_DOC_POSITION_TEXT, 5);
    p = line_position(document, c, XUI_DOC_POSITION_TEXT, 2, XUI_DOC_AFTER);
    line_expect(renderer, p, c, XUI_DOC_POSITION_TEXT, 0, c, XUI_DOC_POSITION_TEXT, 6);
    p = line_position(document, cell, XUI_DOC_POSITION_GAP, 0, XUI_DOC_AFTER);
    line_expect(renderer, p, cell, XUI_DOC_POSITION_GAP, 0, cell, XUI_DOC_POSITION_GAP, 0);
    xuiDocumentRelease(document);

    CHECK(xuiDocumentCreate(NULL, &document) == XUI_OK); line_snapshot(document, renderer, 100);
    p = line_position(document, 1, XUI_DOC_POSITION_GAP, 0, XUI_DOC_AFTER);
    line_expect(renderer, p, 1, XUI_DOC_POSITION_GAP, 0, 1, XUI_DOC_POSITION_GAP, 0);
    xuiDocumentRelease(document);
    for (sample = 1; sample <= 10; sample += 9) {
        size_t length = (size_t)sample * 1024 * 1024; char* value = malloc(length + 1);
        xui_doc_renderer_stats_t before = {0}, stats = {0}; CHECK(value); memset(value, 'x', length); value[length] = 0;
        before.iSize = sizeof(before); stats.iSize = sizeof(stats);
        CHECK(xuiDocumentCreate(NULL, &document) == XUI_OK);
        CHECK(xuiDocumentBeginTransaction(document, NULL, &transaction) == XUI_OK);
        code = line_add(transaction, 1, XUI_DOC_CODE_BLOCK, value); free(value);
        CHECK(xuiDocumentTxnCommit(transaction, NULL) == XUI_OK); xuiDocumentTxnRelease(transaction);
        CHECK(xuiDocumentRendererGetStats(renderer, &before) == XUI_OK);
        line_snapshot(document, renderer, 100);
        p = line_position(document, code, XUI_DOC_POSITION_TEXT, 0, XUI_DOC_AFTER);
        line_expect(renderer, p, code, XUI_DOC_POSITION_TEXT, 0, code, XUI_DOC_POSITION_TEXT, length);
        p.iOffset = length / 2;
        line_expect(renderer, p, code, XUI_DOC_POSITION_TEXT, 0, code, XUI_DOC_POSITION_TEXT, length);
        p.iOffset = length;
        line_expect(renderer, p, code, XUI_DOC_POSITION_TEXT, 0, code, XUI_DOC_POSITION_TEXT, length);
        CHECK(xuiDocumentRendererGetStats(renderer, &stats) == XUI_OK && stats.iShapedBytes - before.iShapedBytes <= 16384);
        xuiDocumentRelease(document);
    }
    {
        const char source[] = "# head\r\n\r\nabcdef\r\n"; xui_doc_desc_t desc = {0};
        desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN;
        CHECK(xuiDocumentCreate(&desc, &document) == XUI_OK);
        CHECK(xuiDocumentLoadMarkdown(document, source, sizeof(source) - 1) == XUI_OK);
        line_snapshot(document, renderer, 100);
        CHECK(xuiDocumentRendererSetMode(renderer, XUI_DOC_SOURCE_TEXT) == XUI_OK);
        line_snapshot(document, renderer, 14);
        p = line_position(document, 1, XUI_DOC_POSITION_SOURCE, 0, XUI_DOC_AFTER);
        line_expect(renderer, p, 1, XUI_DOC_POSITION_SOURCE, 0, 1, XUI_DOC_POSITION_SOURCE, 8);
        p.iOffset = 7; line_expect(renderer, p, 1, XUI_DOC_POSITION_SOURCE, 0, 1, XUI_DOC_POSITION_SOURCE, 8);
        p.iOffset = 8; line_expect(renderer, p, 1, XUI_DOC_POSITION_SOURCE, 8, 1, XUI_DOC_POSITION_SOURCE, 10);
        p.iOffset = 10; line_expect(renderer, p, 1, XUI_DOC_POSITION_SOURCE, 10, 1, XUI_DOC_POSITION_SOURCE, 18);
        p.iOffset = 18; line_expect(renderer, p, 1, XUI_DOC_POSITION_SOURCE, 18, 1, XUI_DOC_POSITION_SOURCE, 18);
        CHECK(xuiDocumentRendererSetMode(renderer, XUI_DOC_LIVE_MARKDOWN) == XUI_OK);
        p.iOffset = 0; CHECK(xuiDocumentRendererSetActivePosition(renderer, &p) == XUI_OK);
        CHECK(xuiDocumentRendererLayout(renderer, 100, 0, 100) == XUI_OK);
        line_expect(renderer, p, 1, XUI_DOC_POSITION_SOURCE, 0, 1, XUI_DOC_POSITION_SOURCE, 8);
        p.iOffset = 11; line_expect(renderer, p, 1, XUI_DOC_POSITION_SOURCE, 10, 1, XUI_DOC_POSITION_SOURCE, 16);
        xuiDocumentRelease(document);
    }
    xuiDocumentRendererRelease(renderer); xuiDestroy(context); proxy.tProxy.fontDestroy(&proxy.tProxy, font);
    puts("Document visual rows: wrapping/affinity/mixed-font/break/table/nested-cell/Bidi/empty/CRLF/LIVE/1MiB/10MiB passed");
}
