/* Width estimates must be replaceable even after their fragments have been evicted. */
static int lazyWidthTest(int glyphs)
{
    const int count = 10000, bytesPerLine = glyphs * 3, length = count * (bytesPerLine + 1) - 1;
    xui_test_proxy_state_t proxy;
    xui_context context = NULL;
    xui_widget edit = NULL;
    xui_font font = NULL;
    xui_rich_document doc = NULL;
    xui_rich_edit_data_t* p;
    xui_rich_edit_desc_t desc = {0};
    xui_rich_text_style_t style = {0};
    float estimate, expected, maxX;
    unsigned updates = 0, layouts = 0;
    char* text = NULL;
    int failed = 0, i, j;
    xuiTestProxyInit(&proxy);
    g_shape = proxy.tProxy.textShape; proxy.tProxy.textShape = auditShape;
    text = (char*)malloc((size_t)length + 1);
    REQUIRE(text != NULL);
    for (i = 0; i < count; i++) {
        for (j = 0; j < glyphs; j++) memcpy(text + i * (bytesPerLine + 1) + j * 3, "\xe4\xb8\xad", 3);
        if (i + 1 < count) text[i * (bytesPerLine + 1) + bytesPerLine] = '\n';
    }
    text[length] = 0;
    REQUIRE(xuiRichDocumentCreate(&doc) == XUI_OK);
    REQUIRE(xuiRichDocumentReplace(doc, 0, 0, text, &style) == XUI_OK);
    REQUIRE(xuiCreate(&context) == XUI_OK);
    REQUIRE(xuiSetProxy(context, &proxy.tProxy) == XUI_OK);
    REQUIRE(proxy.tProxy.fontLoadFile(&proxy.tProxy, &font, "width.ttf", 14, 0) == XUI_OK);
    REQUIRE(xuiSetDefaultFont(context, font) == XUI_OK);
    REQUIRE(xuiInputViewport(context, 960, 640) == XUI_OK);
    desc.iSize = sizeof(desc); desc.pDocument = doc; desc.pFont = font; desc.bWordWrap = 0;
    REQUIRE(xuiRichEditCreate(context, &edit, &desc) == XUI_OK);
    REQUIRE(xuiSetRootWidget(context, edit) == XUI_OK);
    REQUIRE(xuiLayout(context) == XUI_OK);
    p = __xuiRichEditData(edit);
    REQUIRE(__xuiRichEditPrepareLayout(edit, p) == XUI_OK);
    estimate = p->fContentWidth;
    expected = fmaxf(p->fLayoutWidth, glyphs * 7.0f); /* The deterministic proxy's shared shape advance. */
    REQUIRE(estimate > expected);
    REQUIRE(!p->bExactLayout && p->iBlockCount < 100);
    /* These are ordinary scroll/prepare operations, never global fragment enumeration. */
    for (i = 0; i < count; i += 20) {
        resetWork();
        REQUIRE(xuiRichEditSetScroll(edit, 100000.0f, (float)__xuiRichEditHeightBefore(p, i)) == XUI_OK);
        REQUIRE(__xuiRichEditPrepareLayout(edit, p) == XUI_OK);
        REQUIRE(g_work.ParagraphIndex == 0 && g_work.LayoutBlock < 100);
        REQUIRE(g_work.WidthStep <= g_work.LayoutBlock * 15);
        REQUIRE(!p->bExactLayout && p->iBlockCount < 100);
        layouts += g_work.LayoutBlock; updates += g_work.WidthStep;
    }
    /* Test-side full inspection verifies every estimate was replaced, not just the viewport. */
    for (i = 0; i < count; i++) REQUIRE(p->pParagraphs[i].fWidth == expected);
    REQUIRE(xuiScrollModelGetMaxOffset(&p->tScrollModel, &maxX, NULL) == XUI_OK);
    printf("RichEdit nowrap UTF8 N=%d glyphs=%d estimate=%.2f measured_max=%.2f content=%.2f max_x=%.2f point_steps=%u layout=%u\n",
        count, glyphs, estimate, expected, p->fContentWidth, maxX, updates, layouts);
    REQUIRE(p->fContentWidth == expected);
    REQUIRE(maxX == fmaxf(0.0f, expected - p->fLayoutWidth));
    REQUIRE(p->fScrollX <= maxX);
    REQUIRE(p->bShowHScrollBar == (maxX > 0.0f));
    resetWork();
    REQUIRE(__xuiRichEditPrepareLayout(edit, p) == XUI_OK);
    REQUIRE(g_work.LayoutBlock == 0 && g_work.ParagraphIndex == 0 && g_work.WidthStep == 0);
    REQUIRE(privateBytes(p) < 3 * 1048576);
cleanup:
    if (context != NULL) xuiDestroy(context);
    if (doc != NULL) xuiRichDocumentDestroy(doc);
    if (font != NULL) proxy.tProxy.fontDestroy(&proxy.tProxy, font);
    free(text);
    return failed;
}
