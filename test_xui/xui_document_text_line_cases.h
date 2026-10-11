/* Literal partitions, not expectations calculated by the implementation. */
static void text_unit_partition(xui_widget view, const char* expected,
    const uint64_t* boundaries, size_t count, int unit)
{
    xui_internal_accessible_text_range *whole, *range;
    size_t i; uint64_t at, length = strlen(expected); int actual;
    whole = create(view, XUI_INTERNAL_TEXT_DOCUMENT, 0, 0, 0); text(whole, expected);
    CHECK(whole->ops->clone(whole, &range) == XUI_OK);
    for (i = 0; i + 1 < count; i++) {
        for (at = boundaries[i]; at < boundaries[i + 1]; at++) {
            if (((unsigned char)expected[at] & 0xc0) == 0x80) continue;
            CHECK(range->ops->setOffsets(range, at, at) == XUI_OK);
            {
                int result = range->ops->expand(range, unit);
                if (result != XUI_OK) fprintf(stderr, "Unit=%d expand at %llu: result=%d\n", unit, (unsigned long long)at, result);
                CHECK(result == XUI_OK);
            }
            offsets(range, boundaries[i], boundaries[i + 1]);
        }
        CHECK(range->ops->move(range, unit, 0, &actual) == XUI_OK && !actual);
        offsets(range, boundaries[i], boundaries[i + 1]);
        if (i + 2 < count) {
            int result = range->ops->move(range, unit, 1, &actual);
            if (result != XUI_OK || actual != 1) fprintf(stderr,
                "Line move fixture=[%s] unit=%zu start=%llu result=%d actual=%d\n",
                expected, i, (unsigned long long)boundaries[i], result, actual);
            CHECK(result == XUI_OK && actual == 1);
            offsets(range, boundaries[i + 1], boundaries[i + 2]);
            CHECK(range->ops->move(range, unit, -1, &actual) == XUI_OK && actual == -1);
            offsets(range, boundaries[i], boundaries[i + 1]);
        }
    }
    CHECK(range->ops->setOffsets(range, length, length) == XUI_OK);
    CHECK(range->ops->expand(range, unit) == XUI_OK);
    offsets(range, boundaries[count - 2], length);
    CHECK(range->ops->setOffsets(range, 0, 0) == XUI_OK);
    CHECK(range->ops->move(range, unit, INT_MAX, &actual) == XUI_OK && actual == (int)count - 1);
    offsets(range, length, length);
    CHECK(range->ops->move(range, unit, INT_MIN, &actual) == XUI_OK && actual == 1 - (int)count);
    offsets(range, 0, 0);
    CHECK(range->ops->setOffsets(range, 0, boundaries[1]) == XUI_OK);
    CHECK(range->ops->moveEndpoint(range, XUI_INTERNAL_TEXT_START, unit, 2, &actual) == XUI_OK);
    CHECK(actual == (count > 2 ? 2 : 1)); offsets(range, boundaries[count > 2 ? 2 : 1], boundaries[count > 2 ? 2 : 1]);
    range->ops->release(range); whole->ops->release(whole);
}
static void text_line_partition(xui_widget view, const char* expected,
    const uint64_t* boundaries, size_t count)
{ text_unit_partition(view, expected, boundaries, count, XUI_INTERNAL_TEXT_LINE); }
static xui_widget text_line_view(xui_context context, xui_document document, uint32_t mode, float width)
{
    xui_doc_view_desc_t desc = {0}; xui_widget view;
    desc.iSize = sizeof(desc); desc.pDocument = document;
    CHECK(xuiDocumentViewCreate(context, &desc, &view) == XUI_OK);
    CHECK(xuiSetRootWidget(context, view) == XUI_OK);
    CHECK(xuiWidgetSetRect(view, (xui_rect_t){0, 0, width, 100}) == XUI_OK);
    CHECK(xuiDocumentViewSetMode(view, mode) == XUI_OK);
    CHECK(xuiLayout(context) == XUI_OK);
    CHECK(xuiWidgetGetContentRect(view).fW == width);
    return view;
}
static void text_line_cases(void)
{
    xui_test_proxy_state_t proxy; xui_context context; xui_font font;
    xui_document document; xui_document_transaction transaction; xui_widget view;
    xui_doc_desc_t desc = {0}; uint64_t paragraph, table, row, cell;
    static const uint64_t wrap[] = {0, 3, 7, 8, 11};
    static const uint64_t hard[] = {0, 4, 7, 8};
    static const uint64_t cells[] = {0, 5, 11};
    static const uint64_t source[] = {0, 8, 10, 18};
    static const uint64_t visual[] = {0, 5, 12};
    static const uint64_t inactive[] = {0, 10, 18};
    static const uint64_t fenced[] = {0, 4, 5, 15, 18, 23, 28};
    static const uint64_t definitions[] = {0, 14};
    xuiTestProxyInit(&proxy); CHECK(xuiCreate(&context) == XUI_OK);
    CHECK(xuiSetProxy(context, &proxy.tProxy) == XUI_OK);
    CHECK(proxy.tProxy.fontLoadFile(&proxy.tProxy, &font, "test.ttf", 14, 0) == XUI_OK);
    CHECK(xuiSetDefaultFont(context, font) == XUI_OK);
    CHECK(xuiDocumentCreate(NULL, &document) == XUI_OK);
    CHECK(xuiDocumentBeginTransaction(document, NULL, &transaction) == XUI_OK);
    paragraph = line_add(transaction, 1, XUI_DOC_PARAGRAPH, NULL);
    line_add(transaction, paragraph, XUI_DOC_TEXT, "abcdef");
    line_add(transaction, 1, XUI_DOC_PARAGRAPH, NULL);
    paragraph = line_add(transaction, 1, XUI_DOC_PARAGRAPH, NULL);
    line_add(transaction, paragraph, XUI_DOC_TEXT, "xy");
    CHECK(xuiDocumentTxnCommit(transaction, NULL) == XUI_OK); xuiDocumentTxnRelease(transaction);
    view = text_line_view(context, document, XUI_DOC_VISUAL, 21);
    text_line_partition(view, "abcdef\n\nxy\n", wrap, sizeof(wrap) / sizeof(*wrap));
    xuiWidgetDestroy(view); xuiDocumentRelease(document);

    CHECK(xuiDocumentCreate(NULL, &document) == XUI_OK);
    CHECK(xuiDocumentBeginTransaction(document, NULL, &transaction) == XUI_OK);
    paragraph = line_add(transaction, 1, XUI_DOC_PARAGRAPH, NULL);
    line_add(transaction, paragraph, XUI_DOC_TEXT, "abc");
    line_add(transaction, paragraph, XUI_DOC_HARD_BREAK, NULL);
    line_add(transaction, paragraph, XUI_DOC_TEXT, "de");
    line_add(transaction, paragraph, XUI_DOC_HARD_BREAK, NULL);
    CHECK(xuiDocumentTxnCommit(transaction, NULL) == XUI_OK); xuiDocumentTxnRelease(transaction);
    view = text_line_view(context, document, XUI_DOC_VISUAL, 100);
    text_line_partition(view, "abc\nde\n\n", hard, sizeof(hard) / sizeof(*hard));
    xuiWidgetDestroy(view); xuiDocumentRelease(document);

    CHECK(xuiDocumentCreate(NULL, &document) == XUI_OK);
    CHECK(xuiDocumentBeginTransaction(document, NULL, &transaction) == XUI_OK);
    table = line_add(transaction, 1, XUI_DOC_TABLE, NULL); row = line_add(transaction, table, XUI_DOC_ROW, NULL);
    cell = line_add(transaction, row, XUI_DOC_CELL, NULL); paragraph = line_add(transaction, cell, XUI_DOC_PARAGRAPH, NULL);
    line_add(transaction, paragraph, XUI_DOC_TEXT, "left");
    cell = line_add(transaction, row, XUI_DOC_CELL, NULL); paragraph = line_add(transaction, cell, XUI_DOC_PARAGRAPH, NULL);
    line_add(transaction, paragraph, XUI_DOC_TEXT, "right");
    CHECK(xuiDocumentTxnCommit(transaction, NULL) == XUI_OK); xuiDocumentTxnRelease(transaction);
    view = text_line_view(context, document, XUI_DOC_VISUAL, 600);
    text_line_partition(view, "left\tright\n", cells, sizeof(cells) / sizeof(*cells));
    xuiWidgetDestroy(view); xuiDocumentRelease(document);

    desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN;
    CHECK(xuiDocumentCreate(&desc, &document) == XUI_OK);
    CHECK(xuiDocumentLoadMarkdown(document, "# head\r\n\r\nabcdef\r\n", 18) == XUI_OK);
    view = text_line_view(context, document, XUI_DOC_SOURCE_TEXT, 21);
    text_line_partition(view, "# head\r\n\r\nabcdef\r\n", source, sizeof(source) / sizeof(*source));
    CHECK(xuiDocumentViewSetMode(view, XUI_DOC_VISUAL) == XUI_OK);
    CHECK(xuiWidgetSetRect(view, (xui_rect_t){0, 0, 600, 100}) == XUI_OK);
    text_line_partition(view, "head\nabcdef\n", visual, sizeof(visual) / sizeof(*visual));
    CHECK(xuiDocumentViewSetMode(view, XUI_DOC_LIVE_MARKDOWN) == XUI_OK);
    text_line_partition(view, "# head\r\n\r\nabcdef\r\n", source, sizeof(source) / sizeof(*source));
    {
        xui_doc_range_t selected;
        selected.tAnchor = selected.tCaret = line_position(document, 1,
            XUI_DOC_POSITION_SOURCE, 10, XUI_DOC_AFTER);
        CHECK(xuiDocumentViewSetSelection(view, &selected) == XUI_OK);
        text_line_partition(view, "# head\r\n\r\nabcdef\r\n", inactive, sizeof(inactive) / sizeof(*inactive));
    }
    xuiWidgetDestroy(view); xuiDocumentRelease(document);

    CHECK(xuiDocumentCreate(&desc, &document) == XUI_OK);
    CHECK(xuiDocumentLoadMarkdown(document, "# h\n\n```c\nabcd\nxy\n```\n\nlast\n", 28) == XUI_OK);
    view = text_line_view(context, document, XUI_DOC_LIVE_MARKDOWN, 600);
    text_line_partition(view, "# h\n\n```c\nabcd\nxy\n```\n\nlast\n", fenced, sizeof(fenced) / sizeof(*fenced));
    xuiWidgetDestroy(view); xuiDocumentRelease(document);

    CHECK(xuiDocumentCreate(&desc, &document) == XUI_OK);
    CHECK(xuiDocumentLoadMarkdown(document, "[id]: /target\n", 14) == XUI_OK);
    view = text_line_view(context, document, XUI_DOC_LIVE_MARKDOWN, 100);
    text_line_partition(view, "[id]: /target\n", definitions, sizeof(definitions) / sizeof(*definitions));
    xuiWidgetDestroy(view); xuiDocumentRelease(document);
    xuiDestroy(context); proxy.tProxy.fontDestroy(&proxy.tProxy, font);
    puts("Document line units: wrapped/empty/hard-break/table/SOURCE/VISUAL/LIVE/CRLF/count/endpoints passed");
}
