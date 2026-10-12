static void text_format_literal_cases(void)
{
    static const char source[] = "aa **bb** cc\n\nend\n";
    static const uint64_t rich_edges[] = {0, 4, 7, 10};
    static const uint64_t visual_edges[] = {0, 3, 5, 13};
    static const uint64_t source_edges[] = {0, 18};
    static const uint64_t live_edges[] = {0, 3, 5, 7, 9, 12, 14, 18};
    xui_test_proxy_state_t proxy; xui_context context; xui_font font;
    xui_document document; xui_document_transaction transaction; xui_doc_desc_t desc = {0};
    xui_doc_attributes_t attrs = {0}; xui_widget view; uint64_t paragraph, node; uint32_t mode;
    xuiTestProxyInit(&proxy); CHECK(xuiCreate(&context) == XUI_OK);
    CHECK(xuiSetProxy(context, &proxy.tProxy) == XUI_OK);
    CHECK(proxy.tProxy.fontLoadFile(&proxy.tProxy, &font, "test.ttf", 14, 0) == XUI_OK);
    CHECK(xuiSetDefaultFont(context, font) == XUI_OK);
    CHECK(xuiDocumentCreate(NULL, &document) == XUI_OK);
    CHECK(xuiDocumentBeginTransaction(document, NULL, &transaction) == XUI_OK);
    paragraph = line_add(transaction, 1, XUI_DOC_PARAGRAPH, NULL);
    line_add(transaction, paragraph, XUI_DOC_TEXT, "aa");
    node = line_add(transaction, paragraph, XUI_DOC_TEXT, "bb"); attrs.fFontSize = 14;
    CHECK(xuiDocumentTxnSetAttributes(transaction, node, &attrs) == XUI_OK);
    node = line_add(transaction, paragraph, XUI_DOC_TEXT, "\xe4\xb8\xad");
    memset(&attrs, 0, sizeof(attrs)); attrs.iMarks = XUI_DOC_BOLD;
    CHECK(xuiDocumentTxnSetAttributes(transaction, node, &attrs) == XUI_OK);
    line_add(transaction, paragraph, XUI_DOC_TEXT, "dd");
    CHECK(xuiDocumentTxnCommit(transaction, NULL) == XUI_OK); xuiDocumentTxnRelease(transaction);
    view = text_line_view(context, document, XUI_DOC_VISUAL, 600);
    text_unit_partition(view, "aabb\xe4\xb8\xad" "dd\n", rich_edges, 4, XUI_INTERNAL_TEXT_FORMAT);
    xuiWidgetDestroy(view); xuiDocumentRelease(document);
    desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN;
    CHECK(xuiDocumentCreate(&desc, &document) == XUI_OK);
    CHECK(xuiDocumentLoadMarkdown(document, source, strlen(source)) == XUI_OK);
    for (mode = XUI_DOC_VISUAL; mode <= XUI_DOC_LIVE_MARKDOWN; mode++) {
        view = text_line_view(context, document, mode, 600);
        if (mode == XUI_DOC_LIVE_MARKDOWN) {
            xui_doc_range_t active; active.tAnchor = active.tCaret = line_position(document, 1, XUI_DOC_POSITION_SOURCE, 14, XUI_DOC_AFTER);
            CHECK(xuiDocumentViewSetSelection(view, &active) == XUI_OK);
        }
        text_unit_partition(view, mode == XUI_DOC_VISUAL ? "aa bb cc\nend\n" : source,
            mode == XUI_DOC_VISUAL ? visual_edges : mode == XUI_DOC_SOURCE_TEXT ? source_edges : live_edges,
            mode == XUI_DOC_VISUAL ? 4 : mode == XUI_DOC_SOURCE_TEXT ? 2 : 8, XUI_INTERNAL_TEXT_FORMAT);
        xuiWidgetDestroy(view);
    }
    xuiDestroy(context); xuiDocumentRelease(document); proxy.tProxy.fontDestroy(&proxy.tProxy, font);
    puts("Document format literal units: adjacent equal-style runs, UTF8, bold, raw source and LIVE hidden/active regions passed.");
}

static void text_format_attributes(void)
{
    static const uint64_t edges[] = {0, 2, 3, 5, 6, 10, 12, 14, 19};
    const uint32_t red = XUI_COLOR_RGBA(200, 20, 30, 255), blue = XUI_COLOR_RGBA(10, 40, 220, 255);
    xui_test_proxy_state_t proxy; xui_context context; xui_font font;
    xui_document document; xui_document_transaction transaction;
    xui_doc_attributes_t attrs = {0}; xui_widget view; uint64_t paragraph, node;
    xuiTestProxyInit(&proxy); CHECK(xuiCreate(&context) == XUI_OK);
    CHECK(xuiSetProxy(context, &proxy.tProxy) == XUI_OK);
    CHECK(proxy.tProxy.fontLoadFile(&proxy.tProxy, &font, "test.ttf", 14, 0) == XUI_OK);
    CHECK(xuiSetDefaultFont(context, font) == XUI_OK);
    CHECK(xuiDocumentCreate(NULL, &document) == XUI_OK);
    CHECK(xuiDocumentBeginTransaction(document, NULL, &transaction) == XUI_OK);
    attrs.sLanguage = "en"; CHECK(xuiDocumentTxnSetAttributes(transaction, 1, &attrs) == XUI_OK);
    paragraph = line_add(transaction, 1, XUI_DOC_PARAGRAPH, NULL);
    memset(&attrs, 0, sizeof(attrs)); attrs.iTextColor = red;
    CHECK(xuiDocumentTxnSetAttributes(transaction, paragraph, &attrs) == XUI_OK);
    line_add(transaction, paragraph, XUI_DOC_TEXT, "A");
    node = line_add(transaction, paragraph, XUI_DOC_TEXT, "B");
    CHECK(xuiDocumentTxnSetAttributes(transaction, node, &attrs) == XUI_OK);
    node = line_add(transaction, paragraph, XUI_DOC_TEXT, "C"); attrs.iTextColor = blue;
    CHECK(xuiDocumentTxnSetAttributes(transaction, node, &attrs) == XUI_OK);
    node = line_add(transaction, paragraph, XUI_DOC_TEXT, "D"); attrs.iFlags = XUI_DOC_BACKGROUND_COLOR_CURRENT;
    CHECK(xuiDocumentTxnSetAttributes(transaction, node, &attrs) == XUI_OK);
    node = line_add(transaction, paragraph, XUI_DOC_TEXT, "E"); attrs.iFlags = 0; attrs.iBackgroundColor = blue;
    CHECK(xuiDocumentTxnSetAttributes(transaction, node, &attrs) == XUI_OK);
    node = line_add(transaction, paragraph, XUI_DOC_TEXT, "F"); attrs.sLanguage = "zh";
    CHECK(xuiDocumentTxnSetAttributes(transaction, node, &attrs) == XUI_OK);
    node = line_add(transaction, paragraph, XUI_DOC_TEXT, "G"); attrs.iMarks = XUI_DOC_UNDERLINE;
    CHECK(xuiDocumentTxnSetAttributes(transaction, node, &attrs) == XUI_OK);
    node = line_add(transaction, paragraph, XUI_DOC_HARD_BREAK, NULL);
    CHECK(xuiDocumentTxnSetAttributes(transaction, node, &attrs) == XUI_OK);
    node = line_add(transaction, paragraph, XUI_DOC_TEXT, "H");
    CHECK(xuiDocumentTxnSetAttributes(transaction, node, &attrs) == XUI_OK);
    paragraph = line_add(transaction, 1, XUI_DOC_PARAGRAPH, NULL);
    { xui_doc_attributes_t alignment = {0}; alignment.iAlignment = 1;
      CHECK(xuiDocumentTxnSetAttributes(transaction, paragraph, &alignment) == XUI_OK); }
    node = line_add(transaction, paragraph, XUI_DOC_TEXT, "I");
    CHECK(xuiDocumentTxnSetAttributes(transaction, node, &attrs) == XUI_OK);
    paragraph = line_add(transaction, 1, XUI_DOC_HEADING, NULL);
    { xui_doc_attributes_t heading = {0}; heading.iHeadingLevel = 2;
      CHECK(xuiDocumentTxnSetAttributes(transaction, paragraph, &heading) == XUI_OK); }
    node = line_add(transaction, paragraph, XUI_DOC_TEXT, "J");
    CHECK(xuiDocumentTxnSetAttributes(transaction, node, &attrs) == XUI_OK);
    line_add(transaction, 1, XUI_DOC_CODE_BLOCK, "code");
    CHECK(xuiDocumentTxnCommit(transaction, NULL) == XUI_OK); xuiDocumentTxnRelease(transaction);
    view = text_line_view(context, document, XUI_DOC_VISUAL, 600);
    text_unit_partition(view, "ABCDEFG\nH\nI\nJ\ncode\n", edges, sizeof(edges) / sizeof(*edges), XUI_INTERNAL_TEXT_FORMAT);
    xuiWidgetDestroy(view); xuiDestroy(context); xuiDocumentRelease(document);
    proxy.tProxy.fontDestroy(&proxy.tProxy, font);
    puts("Document format effective attributes: inheritance/currentColor, language, underline/break, alignment, heading and code passed.");
}

static uint64_t text_format_make_rich(xui_document document)
{
    xui_document_transaction transaction; xui_doc_attributes_t attrs = {0}; uint64_t paragraph, node;
    CHECK(xuiDocumentBeginTransaction(document, NULL, &transaction) == XUI_OK);
    paragraph = line_add(transaction, 1, XUI_DOC_PARAGRAPH, NULL);
    line_add(transaction, paragraph, XUI_DOC_TEXT, "aa");
    node = line_add(transaction, paragraph, XUI_DOC_TEXT, "bb"); attrs.iMarks = XUI_DOC_BOLD;
    CHECK(xuiDocumentTxnSetAttributes(transaction, node, &attrs) == XUI_OK);
    line_add(transaction, paragraph, XUI_DOC_TEXT, "cc");
    CHECK(xuiDocumentTxnCommit(transaction, NULL) == XUI_OK); xuiDocumentTxnRelease(transaction);
    return node;
}

static void text_format_live_tail(xui_widget view, xui_document document)
{
    xui_doc_range_t active;
    active.tAnchor = active.tCaret = line_position(document, 1, XUI_DOC_POSITION_SOURCE, 14, XUI_DOC_AFTER);
    CHECK(xuiDocumentViewSetSelection(view, &active) == XUI_OK);
}

static void text_format_oom(int profile, uint32_t mode)
{
    text_paragraph_allocator allocator = {0}; xui_test_proxy_state_t proxy;
    xui_context context; xui_font font; xui_document document; xui_widget view;
    xui_doc_desc_t desc = {0}; xui_internal_accessible_text_range* range;
    uint64_t revision, calls, live, first_end; int action, fault, actual, i;
    static const char source[] = "aa **bb** cc\n\nend\n";
    xuiTestProxyInit(&proxy); CHECK(xuiCreate(&context) == XUI_OK);
    CHECK(xuiSetProxy(context, &proxy.tProxy) == XUI_OK);
    CHECK(proxy.tProxy.fontLoadFile(&proxy.tProxy, &font, "test.ttf", 14, 0) == XUI_OK);
    CHECK(xuiSetDefaultFont(context, font) == XUI_OK);
    desc.iSize = sizeof(desc); desc.iProfile = (uint32_t)profile;
    desc.onAlloc = text_paragraph_alloc; desc.onFree = text_paragraph_free; desc.pAllocatorUser = &allocator;
    CHECK(xuiDocumentCreate(&desc, &document) == XUI_OK);
    if (profile == XUI_DOCUMENT_MARKDOWN) CHECK(xuiDocumentLoadMarkdown(document, source, strlen(source)) == XUI_OK);
    else text_format_make_rich(document);
    revision = xuiDocumentGetRevision(document);
    first_end = profile == XUI_DOCUMENT_RICH ? 2 : mode == XUI_DOC_SOURCE_TEXT ? 18 : 3;
    for (action = 0; action < 3; action++) for (fault = 1; fault <= 4; fault++) {
        view = text_line_view(context, document, mode, 600);
        if (mode == XUI_DOC_LIVE_MARKDOWN) text_format_live_tail(view, document);
        range = create(view, XUI_INTERNAL_TEXT_DOCUMENT, 0, 0, 0);
        CHECK(range->ops->setOffsets(range, action ? 0 : 1, action ? 0 : 1) == XUI_OK);
        calls = allocator.calls; live = allocator.live; allocator.fail_call = calls + fault; actual = 77;
        if (!action) CHECK(range->ops->expand(range, XUI_INTERNAL_TEXT_FORMAT) == XUI_ERROR_OUT_OF_MEMORY);
        else if (action == 1) CHECK(range->ops->move(range, XUI_INTERNAL_TEXT_FORMAT, 1, &actual) == XUI_ERROR_OUT_OF_MEMORY && !actual);
        else CHECK(range->ops->moveEndpoint(range, XUI_INTERNAL_TEXT_START, XUI_INTERNAL_TEXT_FORMAT, 1, &actual) == XUI_ERROR_OUT_OF_MEMORY && !actual);
        CHECK(allocator.live == live && xuiDocumentGetRevision(document) == revision);
        offsets(range, action ? 0 : 1, action ? 0 : 1); allocator.fail_call = 0;
        if (!action) { CHECK(range->ops->expand(range, XUI_INTERNAL_TEXT_FORMAT) == XUI_OK); offsets(range, 0, first_end); }
        else if (action == 1) { CHECK(range->ops->move(range, XUI_INTERNAL_TEXT_FORMAT, 1, &actual) == XUI_OK && actual == 1); offsets(range, first_end, first_end); }
        else { CHECK(range->ops->moveEndpoint(range, XUI_INTERNAL_TEXT_START, XUI_INTERNAL_TEXT_FORMAT, 1, &actual) == XUI_OK && actual == 1); offsets(range, first_end, first_end); }
        calls = allocator.calls;
        for (i = 0; i < 512; i++) {
            CHECK(range->ops->setOffsets(range, 0, 0) == XUI_OK);
            CHECK(range->ops->move(range, XUI_INTERNAL_TEXT_FORMAT, INT_MAX, &actual) == XUI_OK);
            CHECK(range->ops->move(range, XUI_INTERNAL_TEXT_FORMAT, INT_MIN, &actual) == XUI_OK);
            CHECK(range->ops->expand(range, XUI_INTERNAL_TEXT_FORMAT) == XUI_OK);
        }
        CHECK(allocator.calls == calls); range->ops->release(range); xuiWidgetDestroy(view);
    }
    xuiDestroy(context); xuiDocumentRelease(document); CHECK(!allocator.live);
    proxy.tProxy.fontDestroy(&proxy.tProxy, font);
    printf("Document format allocation faults: 3 operations x4 allocations, retry, warm queries and zero leaks: profile=%d mode=%u\n", profile, mode);
}

static void text_format_failed_live_rebuild(void)
{
    text_paragraph_allocator allocator = {0}; xui_test_proxy_state_t proxy;
    xui_context context; xui_font font; xui_document document; xui_widget view;
    xui_doc_desc_t desc = {0}; xui_doc_range_t active;
    xui_internal_accessible_text_range* range; uint64_t calls, live;
    xuiTestProxyInit(&proxy); CHECK(xuiCreate(&context) == XUI_OK);
    CHECK(xuiSetProxy(context, &proxy.tProxy) == XUI_OK);
    CHECK(proxy.tProxy.fontLoadFile(&proxy.tProxy, &font, "test.ttf", 14, 0) == XUI_OK);
    CHECK(xuiSetDefaultFont(context, font) == XUI_OK);
    desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN;
    desc.onAlloc = text_paragraph_alloc; desc.onFree = text_paragraph_free; desc.pAllocatorUser = &allocator;
    CHECK(xuiDocumentCreate(&desc, &document) == XUI_OK);
    CHECK(xuiDocumentLoadMarkdown(document, "aa **bb** cc\n\nend\n", 18) == XUI_OK);
    view = text_line_view(context, document, XUI_DOC_LIVE_MARKDOWN, 600); text_format_live_tail(view, document);
    range = create(view, XUI_INTERNAL_TEXT_DOCUMENT, 0, 0, 0);
    CHECK(range->ops->setOffsets(range, 0, 0) == XUI_OK);
    CHECK(range->ops->expand(range, XUI_INTERNAL_TEXT_FORMAT) == XUI_OK); offsets(range, 0, 3);
    active.tAnchor = active.tCaret = line_position(document, 1, XUI_DOC_POSITION_SOURCE, 0, XUI_DOC_AFTER);
    CHECK(xuiDocumentViewSetSelection(view, &active) == XUI_OK);
    CHECK(range->ops->setOffsets(range, 0, 0) == XUI_OK);
    calls = allocator.calls; live = allocator.live; allocator.fail_call = calls + 1;
    CHECK(range->ops->expand(range, XUI_INTERNAL_TEXT_FORMAT) == XUI_ERROR_OUT_OF_MEMORY);
    CHECK(allocator.live == live); offsets(range, 0, 0); allocator.fail_call = 0;
    text_format_live_tail(view, document); calls = allocator.calls;
    CHECK(range->ops->expand(range, XUI_INTERNAL_TEXT_FORMAT) == XUI_OK); offsets(range, 0, 3);
    CHECK(allocator.calls == calls);
    active.tAnchor = active.tCaret = line_position(document, 1, XUI_DOC_POSITION_SOURCE, 0, XUI_DOC_AFTER);
    CHECK(xuiDocumentViewSetSelection(view, &active) == XUI_OK);
    CHECK(range->ops->expand(range, XUI_INTERNAL_TEXT_FORMAT) == XUI_OK); offsets(range, 0, 14);
    range->ops->release(range); xuiWidgetDestroy(view); xuiDestroy(context); xuiDocumentRelease(document);
    CHECK(!allocator.live); proxy.tProxy.fontDestroy(&proxy.tProxy, font);
    puts("Document format failed LIVE cache rebuild retains old cache and atomic range; restoring/rebuilding active block passed.");
}

static void text_format_theme_refresh(void)
{
    static const uint64_t split[] = {0, 1, 3}, merged[] = {0, 3};
    const uint32_t red = XUI_COLOR_RGBA(200, 20, 30, 255);
    xui_test_proxy_state_t proxy; xui_context context; xui_font font, large;
    xui_document document; xui_document_transaction transaction;
    xui_doc_attributes_t attrs = {0}; xui_style_property_t property = {0};
    xui_widget view; uint64_t paragraph, node;
    xuiTestProxyInit(&proxy); CHECK(xuiCreate(&context) == XUI_OK);
    CHECK(xuiSetProxy(context, &proxy.tProxy) == XUI_OK);
    CHECK(proxy.tProxy.fontLoadFile(&proxy.tProxy, &font, "test.ttf", 14, 0) == XUI_OK);
    CHECK(proxy.tProxy.fontCreateSized(&proxy.tProxy, &large, font, 28) == XUI_OK);
    CHECK(xuiSetDefaultFont(context, font) == XUI_OK);
    CHECK(xuiDocumentCreate(NULL, &document) == XUI_OK);
    CHECK(xuiDocumentBeginTransaction(document, NULL, &transaction) == XUI_OK);
    paragraph = line_add(transaction, 1, XUI_DOC_PARAGRAPH, NULL);
    line_add(transaction, paragraph, XUI_DOC_TEXT, "a");
    node = line_add(transaction, paragraph, XUI_DOC_TEXT, "b"); attrs.iTextColor = red;
    CHECK(xuiDocumentTxnSetAttributes(transaction, node, &attrs) == XUI_OK);
    CHECK(xuiDocumentTxnCommit(transaction, NULL) == XUI_OK); xuiDocumentTxnRelease(transaction);
    view = text_line_view(context, document, XUI_DOC_VISUAL, 600);
    text_unit_partition(view, "ab\n", split, 3, XUI_INTERNAL_TEXT_FORMAT);
    property.iSize = sizeof(property); property.sName = "document.text.color";
    property.tValue.iSize = sizeof(property.tValue); property.tValue.iType = XUI_STYLE_VALUE_COLOR;
    property.tValue.iColor = red; CHECK(xuiWidgetSetInlineStyle(view, &property, 1) == XUI_OK);
    /* No layout/render pump: range queries must resolve the current theme. */
    text_unit_partition(view, "ab\n", merged, 2, XUI_INTERNAL_TEXT_FORMAT);
    CHECK(xuiWidgetSetInlineStyle(view, NULL, 0) == XUI_OK);
    text_unit_partition(view, "ab\n", split, 3, XUI_INTERNAL_TEXT_FORMAT);
    CHECK(xuiDocumentBeginTransaction(document, NULL, &transaction) == XUI_OK);
    memset(&attrs, 0, sizeof(attrs)); attrs.fFontSize = 14;
    CHECK(xuiDocumentTxnSetAttributes(transaction, node, &attrs) == XUI_OK);
    CHECK(xuiDocumentTxnCommit(transaction, NULL) == XUI_OK); xuiDocumentTxnRelease(transaction);
    text_unit_partition(view, "ab\n", merged, 2, XUI_INTERNAL_TEXT_FORMAT);
    CHECK(xuiSetDefaultFont(context, large) == XUI_OK);
    text_unit_partition(view, "ab\n", split, 3, XUI_INTERNAL_TEXT_FORMAT);
    CHECK(xuiSetDefaultFont(context, font) == XUI_OK);
    text_unit_partition(view, "ab\n", merged, 2, XUI_INTERNAL_TEXT_FORMAT);
    xuiWidgetDestroy(view); xuiDestroy(context); xuiDocumentRelease(document);
    proxy.tProxy.fontDestroy(&proxy.tProxy, large); proxy.tProxy.fontDestroy(&proxy.tProxy, font);
    puts("Document format queries resolve theme/default-font changes before the layout/render pump passed.");
}

typedef struct text_format_font_selector { xui_font small, large; int split; } text_format_font_selector;
static xui_font text_format_font(xui_context context, const char* family, uint32_t marks, float size, void* user)
{
    text_format_font_selector* selector = user; (void)context; (void)marks; (void)size;
    return selector->split && !strcmp(family, "B") ? selector->large : selector->small;
}
static void text_format_font_invalidation(void)
{
    static const uint64_t split[] = {0, 1, 3}, merged[] = {0, 3};
    xui_test_proxy_state_t proxy; xui_context context; text_format_font_selector selector = {0};
    xui_document document; xui_document_transaction transaction;
    xui_doc_attributes_t attrs = {0}; xui_doc_view_desc_t desc = {0}; xui_widget view;
    uint64_t paragraph, node;
    xuiTestProxyInit(&proxy); CHECK(xuiCreate(&context) == XUI_OK);
    CHECK(xuiSetProxy(context, &proxy.tProxy) == XUI_OK);
    CHECK(proxy.tProxy.fontLoadFile(&proxy.tProxy, &selector.small, "test.ttf", 14, 0) == XUI_OK);
    CHECK(proxy.tProxy.fontCreateSized(&proxy.tProxy, &selector.large, selector.small, 28) == XUI_OK);
    CHECK(xuiSetDefaultFont(context, selector.small) == XUI_OK);
    CHECK(xuiDocumentCreate(NULL, &document) == XUI_OK);
    CHECK(xuiDocumentBeginTransaction(document, NULL, &transaction) == XUI_OK);
    paragraph = line_add(transaction, 1, XUI_DOC_PARAGRAPH, NULL);
    node = line_add(transaction, paragraph, XUI_DOC_TEXT, "a"); strcpy(attrs.sFontFamily, "A");
    CHECK(xuiDocumentTxnSetAttributes(transaction, node, &attrs) == XUI_OK);
    node = line_add(transaction, paragraph, XUI_DOC_TEXT, "b"); strcpy(attrs.sFontFamily, "B");
    CHECK(xuiDocumentTxnSetAttributes(transaction, node, &attrs) == XUI_OK);
    CHECK(xuiDocumentTxnCommit(transaction, NULL) == XUI_OK); xuiDocumentTxnRelease(transaction);
    desc.iSize = sizeof(desc); desc.pDocument = document;
    desc.tRenderer.iSize = sizeof(desc.tRenderer); desc.tRenderer.onFont = text_format_font; desc.tRenderer.pUser = &selector;
    CHECK(xuiDocumentViewCreate(context, &desc, &view) == XUI_OK);
    CHECK(xuiSetRootWidget(context, view) == XUI_OK);
    CHECK(xuiWidgetSetRect(view, (xui_rect_t){0, 0, 600, 100}) == XUI_OK); CHECK(xuiLayout(context) == XUI_OK);
    text_unit_partition(view, "ab\n", merged, 2, XUI_INTERNAL_TEXT_FORMAT);
    selector.split = 1; CHECK(xuiDocumentViewInvalidateFonts(view) == XUI_OK);
    text_unit_partition(view, "ab\n", split, 3, XUI_INTERNAL_TEXT_FORMAT);
    selector.split = 0; CHECK(xuiDocumentViewInvalidateFonts(view) == XUI_OK);
    text_unit_partition(view, "ab\n", merged, 2, XUI_INTERNAL_TEXT_FORMAT);
    xuiWidgetDestroy(view); xuiDestroy(context); xuiDocumentRelease(document);
    proxy.tProxy.fontDestroy(&proxy.tProxy, selector.large); proxy.tProxy.fontDestroy(&proxy.tProxy, selector.small);
    puts("Document format font callback selection and explicit font invalidation refresh unchanged-descriptor cache passed.");
}

static void text_format_history(void)
{
    static const uint64_t split[] = {0, 2, 4, 7}, merged[] = {0, 7};
    text_paragraph_allocator allocator = {0}; xui_test_proxy_state_t proxy;
    xui_context context; xui_font font; xui_document document, replacement;
    xui_document_transaction transaction; xui_doc_attributes_t attrs = {0}; xui_doc_desc_t desc = {0};
    xui_widget first, second; xui_internal_accessible_text_range* retained;
    uint64_t node, bytes = 77;
    xuiTestProxyInit(&proxy); CHECK(xuiCreate(&context) == XUI_OK);
    CHECK(xuiSetProxy(context, &proxy.tProxy) == XUI_OK);
    CHECK(proxy.tProxy.fontLoadFile(&proxy.tProxy, &font, "test.ttf", 14, 0) == XUI_OK);
    CHECK(xuiSetDefaultFont(context, font) == XUI_OK);
    desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_RICH;
    desc.onAlloc = text_paragraph_alloc; desc.onFree = text_paragraph_free; desc.pAllocatorUser = &allocator;
    CHECK(xuiDocumentCreate(&desc, &document) == XUI_OK); node = text_format_make_rich(document);
    CHECK(xuiDocumentClearHistory(document) == XUI_OK);
    first = text_line_view(context, document, XUI_DOC_VISUAL, 600);
    second = text_line_view(context, document, XUI_DOC_VISUAL, 600);
    text_unit_partition(first, "aabbcc\n", split, 4, XUI_INTERNAL_TEXT_FORMAT);
    text_unit_partition(second, "aabbcc\n", split, 4, XUI_INTERNAL_TEXT_FORMAT);
    retained = create(first, XUI_INTERNAL_TEXT_DOCUMENT, 0, 0, 0);
    CHECK(retained->ops->setOffsets(retained, 1, 1) == XUI_OK);
    CHECK(retained->ops->expand(retained, XUI_INTERNAL_TEXT_FORMAT) == XUI_OK); offsets(retained, 0, 2);
    CHECK(xuiDocumentBeginTransaction(document, NULL, &transaction) == XUI_OK);
    CHECK(xuiDocumentTxnSetAttributes(transaction, node, &attrs) == XUI_OK);
    CHECK(xuiDocumentTxnCommit(transaction, NULL) == XUI_OK); xuiDocumentTxnRelease(transaction);
    text_unit_partition(first, "aabbcc\n", merged, 2, XUI_INTERNAL_TEXT_FORMAT);
    text_unit_partition(second, "aabbcc\n", merged, 2, XUI_INTERNAL_TEXT_FORMAT);
    CHECK(retained->ops->expand(retained, XUI_INTERNAL_TEXT_FORMAT) == XUI_OK); offsets(retained, 0, 7);
    CHECK(xuiDocumentUndo(document, NULL) == XUI_OK);
    text_unit_partition(first, "aabbcc\n", split, 4, XUI_INTERNAL_TEXT_FORMAT);
    text_unit_partition(second, "aabbcc\n", split, 4, XUI_INTERNAL_TEXT_FORMAT);
    CHECK(retained->ops->expand(retained, XUI_INTERNAL_TEXT_FORMAT) == XUI_OK); offsets(retained, 0, 2);
    CHECK(xuiDocumentRedo(document, NULL) == XUI_OK);
    text_unit_partition(second, "aabbcc\n", merged, 2, XUI_INTERNAL_TEXT_FORMAT);
    CHECK(xuiDocumentCreate(&desc, &replacement) == XUI_OK); text_format_make_rich(replacement);
    CHECK(xuiDocumentViewSetDocument(first, replacement) == XUI_OK);
    CHECK(retained->ops->expand(retained, XUI_INTERNAL_TEXT_FORMAT) == XUI_DOC_ERROR_STALE);
    CHECK(retained->ops->getText(retained, NULL, 0, &bytes) == XUI_DOC_ERROR_STALE && !bytes);
    text_unit_partition(first, "aabbcc\n", split, 4, XUI_INTERNAL_TEXT_FORMAT);
    xuiWidgetDestroy(first); CHECK(retained->ops->expand(retained, XUI_INTERNAL_TEXT_FORMAT) == XUI_DOC_ERROR_STALE);
    retained->ops->release(retained); xuiWidgetDestroy(second); xuiDestroy(context);
    xuiDocumentRelease(document); xuiDocumentRelease(replacement); CHECK(!allocator.live);
    proxy.tProxy.fontDestroy(&proxy.tProxy, font);
    puts("Document format two views, attribute-only edit, shared Undo/Redo, retained range, replacement/destroy and zero leaks passed.");
}

static void text_format_cases(void)
{
    text_format_literal_cases(); text_format_attributes(); text_format_theme_refresh();
    text_format_font_invalidation(); text_format_history();
    text_format_oom(XUI_DOCUMENT_RICH, XUI_DOC_VISUAL);
    text_format_oom(XUI_DOCUMENT_MARKDOWN, XUI_DOC_VISUAL);
    text_format_oom(XUI_DOCUMENT_MARKDOWN, XUI_DOC_SOURCE_TEXT);
    text_format_oom(XUI_DOCUMENT_MARKDOWN, XUI_DOC_LIVE_MARKDOWN);
    text_format_failed_live_rebuild();
}

static void text_format_live_syntax(void)
{
    static const char* sources[] = {"```c\nab\n```\n\nend\n", "aa [bb](url) cc\n\nend\n", "> aa\n> bb\n\nend\n"};
    static const uint64_t fenced[] = {0, 5, 8, 13, 17};
    static const uint64_t link[] = {0, 3, 4, 6, 12, 15, 17, 21};
    static const uint64_t quote[] = {0, 2, 5, 7, 9, 11, 15};
    const uint64_t* edges[] = {fenced, link, quote}; const size_t counts[] = {5, 8, 7};
    const uint64_t active_at[] = {13, 17, 11};
    xui_test_proxy_state_t proxy; xui_context context; xui_font font;
    xui_document document; xui_doc_desc_t desc = {0}; xui_widget view; unsigned i;
    xuiTestProxyInit(&proxy); CHECK(xuiCreate(&context) == XUI_OK);
    CHECK(xuiSetProxy(context, &proxy.tProxy) == XUI_OK);
    CHECK(proxy.tProxy.fontLoadFile(&proxy.tProxy, &font, "test.ttf", 14, 0) == XUI_OK);
    CHECK(xuiSetDefaultFont(context, font) == XUI_OK);
    desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN;
    CHECK(xuiDocumentCreate(&desc, &document) == XUI_OK);
    for (i = 0; i < 3; i++) {
        xui_doc_range_t active;
        CHECK(xuiDocumentLoadMarkdown(document, sources[i], strlen(sources[i])) == XUI_OK);
        view = text_line_view(context, document, XUI_DOC_LIVE_MARKDOWN, 600);
        active.tAnchor = active.tCaret = line_position(document, 1, XUI_DOC_POSITION_SOURCE, active_at[i], XUI_DOC_AFTER);
        CHECK(xuiDocumentViewSetSelection(view, &active) == XUI_OK);
        printf("Document LIVE format literal hidden syntax fixture=%u\n", i);
        text_unit_partition(view, sources[i], edges[i], counts[i], XUI_INTERNAL_TEXT_FORMAT);
        xuiWidgetDestroy(view);
    }
    xuiDestroy(context); xuiDocumentRelease(document); proxy.tProxy.fontDestroy(&proxy.tProxy, font);
    puts("Document LIVE format fenced code, link delimiters and quote continuation prefixes passed.");
}
