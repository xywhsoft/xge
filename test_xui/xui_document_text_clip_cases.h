/* Literal viewport/container limits are independent of renderer geometry. */
static void text_clip_overflow(xui_widget widget, int overflow)
{
    xui_layout_t layout = xuiWidgetGetLayout(widget);
    layout.iOverflow = overflow;
    CHECK(xuiWidgetSetLayout(widget, &layout) == XUI_OK);
}
static void text_clip_visible(xui_internal_accessible_text_range* range,
    float left, float top, float right, float bottom)
{
    xui_rect_t rectangles[32], small[2]; uint64_t count, small_count, total, i;
    CHECK(range->ops->getRects(range, NULL, 0, &total) == XUI_OK && total > 0 && total <= 32);
    CHECK(range->ops->getRects(range, rectangles, 32, &count) == XUI_OK && count == total);
    for (i = 0; i < count; i++) {
        if (rectangles[i].fX < left - .01f || rectangles[i].fY < top - .01f ||
            rectangles[i].fX + rectangles[i].fW > right + .01f ||
            rectangles[i].fY + rectangles[i].fH > bottom + .01f)
            fprintf(stderr, "Visible rect [%g,%g,%g,%g] exceeds literal clip [%g,%g..%g,%g]\n",
                (double)rectangles[i].fX, (double)rectangles[i].fY, (double)rectangles[i].fW, (double)rectangles[i].fH,
                left, top, right, bottom);
        CHECK(rectangles[i].fX >= left - .01f && rectangles[i].fY >= top - .01f);
        CHECK(rectangles[i].fW > 0 && rectangles[i].fH > 0);
        CHECK(rectangles[i].fX + rectangles[i].fW <= right + .01f);
        CHECK(rectangles[i].fY + rectangles[i].fH <= bottom + .01f);
    }
    CHECK(fabs(rectangles[0].fX + rectangles[0].fW - right) < .02f);
    small[0] = small[1] = (xui_rect_t){-1001, -1002, -1003, -1004};
    CHECK(range->ops->getRects(range, small, 1, &small_count) == XUI_OK && small_count == count);
    CHECK(!memcmp(&small[0], &rectangles[0], sizeof(small[0])));
    CHECK(small[1].fX == -1001 && small[1].fY == -1002 && small[1].fW == -1003 && small[1].fH == -1004);
}
static void text_clip_hidden(xui_internal_accessible_text_range* range)
{
    xui_rect_t rectangle = {-1001, -1002, -1003, -1004}; uint64_t total = 123;
    CHECK(range->ops->getRects(range, &rectangle, 1, &total) == XUI_OK && !total);
    CHECK(rectangle.fX == -1001 && rectangle.fY == -1002 && rectangle.fW == -1003 && rectangle.fH == -1004);
}
static void text_clip_case(int profile, uint32_t mode)
{
    static const char source[] = "abcdefghijklmnopqrstuvwx\nabcdefghijklmnopqrstuvwx\nabcdefghijklmnopqrstuvwx\n";
    xui_test_proxy_state_t proxy; xui_context context; xui_font font;
    xui_document document; xui_widget root, outer, inner, editor;
    xui_doc_desc_t desc = {0}; xui_doc_editor_desc_t ed = {0};
    xui_internal_accessible_text_range* whole;
    xuiTestProxyInit(&proxy); CHECK(xuiCreate(&context) == XUI_OK);
    CHECK(xuiSetProxy(context, &proxy.tProxy) == XUI_OK);
    CHECK(proxy.tProxy.fontLoadFile(&proxy.tProxy, &font, "test.ttf", 14, 0) == XUI_OK);
    CHECK(xuiSetDefaultFont(context, font) == XUI_OK);
    CHECK(xuiSetViewportSize(context, 160, 120) == XUI_OK);
    CHECK(xuiWidgetCreate(context, &root) == XUI_OK);
    CHECK(xuiWidgetCreate(context, &outer) == XUI_OK);
    CHECK(xuiWidgetCreate(context, &inner) == XUI_OK);
    CHECK(xuiSetRootWidget(context, root) == XUI_OK);
    CHECK(xuiWidgetAddChild(root, outer) == XUI_OK && xuiWidgetAddChild(outer, inner) == XUI_OK);
    CHECK(xuiWidgetSetRect(root, (xui_rect_t){0, 0, 160, 120}) == XUI_OK);
    CHECK(xuiWidgetSetRect(outer, (xui_rect_t){10, 10, 90, 90}) == XUI_OK);
    CHECK(xuiWidgetSetRect(inner, (xui_rect_t){10, 5, 50, 60}) == XUI_OK);
    text_clip_overflow(root, XUI_OVERFLOW_VISIBLE);
    text_clip_overflow(outer, XUI_OVERFLOW_HIDDEN);
    text_clip_overflow(inner, XUI_OVERFLOW_CLIP);
    desc.iSize = sizeof(desc); desc.iProfile = (uint32_t)profile;
    CHECK(xuiDocumentCreate(&desc, &document) == XUI_OK);
    if (profile == XUI_DOCUMENT_MARKDOWN) CHECK(xuiDocumentLoadMarkdown(document, source, strlen(source)) == XUI_OK);
    ed.iSize = sizeof(ed); ed.iMode = mode; ed.tView.iSize = sizeof(ed.tView); ed.tView.pDocument = document;
    CHECK(xuiDocumentEditorCreate(context, &ed, &editor) == XUI_OK);
    CHECK(xuiWidgetAddChild(inner, editor) == XUI_OK);
    CHECK(xuiWidgetSetRect(editor, (xui_rect_t){20, 4, 200, 80}) == XUI_OK);
    if (profile == XUI_DOCUMENT_RICH) CHECK(xuiDocumentEditorInsertText(editor, source, strlen(source) - 1) == XUI_OK);
    CHECK(xuiDocumentEditorFlush(editor) == XUI_OK);
    whole = create(editor, XUI_INTERNAL_TEXT_DOCUMENT, 0, 0, 0);
    text(whole, source);
    text_clip_visible(whole, 20, 15, 70, 75);
    text_clip_overflow(inner, XUI_OVERFLOW_VISIBLE);
    text_clip_visible(whole, 10, 10, 100, 100);
    text_clip_overflow(outer, XUI_OVERFLOW_VISIBLE);
    text_clip_visible(whole, 0, 0, 160, 120);
    CHECK(xuiSetViewportSize(context, 55, 38) == XUI_OK);
    text_clip_visible(whole, 0, 0, 55, 38);
    CHECK(xuiSetViewportSize(context, 160, 120) == XUI_OK);
    CHECK(xuiWidgetSetVisible(outer, 0) == XUI_OK); text_clip_hidden(whole); text(whole, source);
    CHECK(xuiWidgetSetVisible(outer, 1) == XUI_OK);
    CHECK(xuiWidgetSetVisible(editor, 0) == XUI_OK); text_clip_hidden(whole);
    CHECK(xuiWidgetSetVisible(editor, 1) == XUI_OK);
    CHECK(xuiWidgetSetEnabled(editor, 0) == XUI_OK); text_clip_visible(whole, 0, 0, 160, 120);
    CHECK(xuiWidgetSetEnabled(editor, 1) == XUI_OK);
    CHECK(xuiWidgetRemoveFromParent(inner) == XUI_OK); text_clip_hidden(whole); text(whole, source);
    CHECK(xuiWidgetAddChild(outer, inner) == XUI_OK); text_clip_visible(whole, 0, 0, 160, 120);
    text_clip_overflow(inner, XUI_OVERFLOW_HIDDEN);
    CHECK(xuiDocumentViewSetScroll(editor, 0, 10) == XUI_OK);
    text_clip_visible(whole, 20, 15, 70, 75);
    CHECK(xuiSetViewportSize(context, 0, 0) == XUI_OK); text_clip_hidden(whole);
    CHECK(xuiSetViewportSize(context, 160, 120) == XUI_OK);
    CHECK(xuiWidgetSetRect(inner, (xui_rect_t){10, 5, 0, 60}) == XUI_OK); text_clip_hidden(whole);
    whole->ops->release(whole); xuiDestroy(context); xuiDocumentRelease(document);
    proxy.tProxy.fontDestroy(&proxy.tProxy, font);
    printf("Document text ancestor/viewport clipping passed: profile=%d mode=%u\n", profile, mode);
}
static void text_clip_cases(void)
{
    text_clip_case(XUI_DOCUMENT_RICH, XUI_DOC_VISUAL);
    text_clip_case(XUI_DOCUMENT_MARKDOWN, XUI_DOC_VISUAL);
    text_clip_case(XUI_DOCUMENT_MARKDOWN, XUI_DOC_SOURCE_TEXT);
    text_clip_case(XUI_DOCUMENT_MARKDOWN, XUI_DOC_LIVE_MARKDOWN);
}
