/* Included after the scale harness so the private operation counters are shared. */
static xui_font_create_sized_proc g_sizedFont;
static xui_font_destroy_proc g_destroyFont;
static unsigned g_sizedCalls, g_destroyCalls;
static float g_lastSize;
static int lazySizedFont(xui_proxy proxy, xui_font* font, xui_font source, float size)
{
    g_sizedCalls++; g_lastSize = size;
    return g_sizedFont(proxy, font, source, size);
}
static void lazyDestroyFont(xui_proxy proxy, xui_font font)
{
    g_destroyCalls++;
    g_destroyFont(proxy, font);
}

static int lazyTextTest(void)
{
    const int count = 10000, columns = 96, length = count * (columns + 1) - 1;
    xui_test_proxy_state_t proxy;
    xui_context context = NULL;
    xui_widget edit = NULL;
    xui_font font = NULL;
    xui_surface target = NULL;
    xui_draw_context draw = NULL;
    xui_rich_document doc = NULL;
    xui_rich_edit_data_t* p;
    xui_rich_edit_desc_t desc = {0};
    xui_rich_text_style_t style = {0};
    xui_rect_t r, content;
    xui_document_node_id_t anchor;
    unsigned cold, resized, destroyed, created;
    float local, measured;
    size_t bytes;
    int failed = 0, i, j, k, state;
    char* text = NULL;
    xuiTestProxyInit(&proxy);
    g_shape = proxy.tProxy.textShape; proxy.tProxy.textShape = auditShape;
    g_sizedFont = proxy.tProxy.fontCreateSized; proxy.tProxy.fontCreateSized = lazySizedFont;
    g_destroyFont = proxy.tProxy.fontDestroy; proxy.tProxy.fontDestroy = lazyDestroyFont;
    text = (char*)malloc((size_t)length + 1);
    REQUIRE(text != NULL);
    for (i = 0; i < count; i++) {
        for (j = 0; j < columns; j++) text[i * (columns + 1) + j] = "Rich content abc "[j % 17];
        if (i + 1 < count) text[i * (columns + 1) + columns] = '\n';
    }
    text[length] = 0;
    style.iSize = sizeof(style); style.fFontSize = 18;
    REQUIRE(xuiRichDocumentCreate(&doc) == XUI_OK);
    REQUIRE(xuiRichDocumentReplace(doc, 0, 0, text, &style) == XUI_OK);
    REQUIRE(xuiCreate(&context) == XUI_OK);
    REQUIRE(xuiSetProxy(context, &proxy.tProxy) == XUI_OK);
    REQUIRE(proxy.tProxy.fontLoadFile(&proxy.tProxy, &font, "lazy.ttf", 14, 0) == XUI_OK);
    REQUIRE(xuiSetDefaultFont(context, font) == XUI_OK);
    REQUIRE(xuiInputViewport(context, 960, 640) == XUI_OK);
    desc.iSize = sizeof(desc); desc.pDocument = doc; desc.pFont = font; desc.bWordWrap = 1;
    REQUIRE(xuiRichEditCreate(context, &edit, &desc) == XUI_OK);
    REQUIRE(xuiSetRootWidget(context, edit) == XUI_OK);
    REQUIRE(xuiWidgetSetRect(edit, (xui_rect_t){0, 0, 936, 616}) == XUI_OK);
    REQUIRE(xuiTestSurfaceCreate(&proxy, &target, 960, 640, XUI_SURFACE_USAGE_TARGET) == XUI_OK);
    REQUIRE(proxy.tProxy.drawBegin(&proxy.tProxy, &draw, target) == XUI_OK);
    p = __xuiRichEditData(edit);
    resetWork();
    REQUIRE(xuiLayout(context) == XUI_OK);
    REQUIRE(__xuiRichEditRender(edit, draw, 0, NULL) == XUI_OK);
    cold = g_work.LayoutBlock; bytes = privateBytes(p);
    REQUIRE(!p->bExactLayout && p->iParagraphCount == count);
    REQUIRE(cold > 0 && cold <= 180 && g_shapeBytes <= 180 * columns);
    REQUIRE(g_work.ParagraphIndex <= 6 * (unsigned)count);
    REQUIRE(p->iBlockCount <= 80 && p->iTotalFragmentCount <= 80 * columns);
    REQUIRE(bytes < 3 * 1048576);
    REQUIRE(p->iLayoutDpiGeneration == context->iDpiGeneration);

    /* Distant jumps correct only the destination neighbourhood. Eviction keeps heights. */
    measured = p->pParagraphs[0].fAdvance;
    for (j = 1; j <= 20; j++) {
        int paragraph = 300 + (j * 433) % 9000;
        resetWork();
        REQUIRE(xuiRichEditSetScroll(edit, 0, (float)__xuiRichEditHeightBefore(p, paragraph) + 2.0f) == XUI_OK);
        REQUIRE(xuiLayout(context) == XUI_OK);
        REQUIRE(__xuiRichEditRender(edit, draw, 0, NULL) == XUI_OK);
        REQUIRE(g_work.LayoutBlock <= 180 && g_shapeCalls <= 180);
        REQUIRE(g_work.ParagraphIndex == 0 && !p->bExactLayout);
        REQUIRE(p->iBlockCount <= 80 && privateBytes(p) < 3 * 1048576);
        REQUIRE(__xuiRichEditParagraphByY(p, p->fScrollY) == paragraph);
        REQUIRE(p->pParagraphs[0].fAdvance == measured);
        resetWork();
        for (i = 0; i < 3; i++) {
            REQUIRE(__xuiRichEditPrepareLayout(edit, p) == XUI_OK);
            REQUIRE(__xuiRichEditRender(edit, draw, 0, NULL) == XUI_OK);
        }
        REQUIRE(g_work.LayoutBlock == 0 && g_shapeCalls == 0);
        REQUIRE(g_work.ParagraphIndex == 0 && g_work.IndexBlock == 0);
        REQUIRE(g_work.HeightStep < 30000 && g_work.RenderBlock < 150);
    }

    k = __xuiRichEditParagraphByY(p, p->fScrollY);
    anchor = p->pParagraphs[k].iId;
    local = p->fScrollY - (float)__xuiRichEditHeightBefore(p, k);
    resetWork();
    REQUIRE(xuiWidgetSetRect(edit, (xui_rect_t){0, 0, 520, 616}) == XUI_OK);
    REQUIRE(xuiLayout(context) == XUI_OK);
    REQUIRE(__xuiRichEditRender(edit, draw, 0, NULL) == XUI_OK);
    resized = g_work.LayoutBlock;
    REQUIRE(resized > 0 && resized <= 180 && g_shapeCalls <= 180);
    REQUIRE(g_work.ParagraphIndex <= 4 * (unsigned)count && !p->bExactLayout);
    k = __xuiRichEditParagraphByY(p, p->fScrollY);
    REQUIRE(p->pParagraphs[k].iId == anchor);
    REQUIRE(fabsf(p->fScrollY - (float)__xuiRichEditHeightBefore(p, k) - local) < 0.1f);

    created = g_sizedCalls; destroyed = g_destroyCalls;
    resetWork();
    REQUIRE(xuiSetVirtualDpi(context, 2.0f) == XUI_OK);
    REQUIRE(xuiLayout(context) == XUI_OK);
    REQUIRE(__xuiRichEditRender(edit, draw, 0, NULL) == XUI_OK);
    REQUIRE(p->iLayoutDpiGeneration == context->iDpiGeneration);
    REQUIRE(g_sizedCalls > created && g_destroyCalls > destroyed && g_lastSize == 18.0f);
    REQUIRE(g_work.LayoutBlock <= 180 && !p->bExactLayout);
    k = __xuiRichEditParagraphByY(p, p->fScrollY);
    REQUIRE(p->pParagraphs[k].iId == anchor);
    REQUIRE(xuiRichEditSetZoom(edit, 1.5f) == XUI_OK);
    REQUIRE(xuiLayout(context) == XUI_OK);
    REQUIRE(g_lastSize == 27.0f);
    REQUIRE(xuiRichEditSetZoom(edit, 1.0f) == XUI_OK);
    REQUIRE(xuiLayout(context) == XUI_OK);

    resetWork();
    REQUIRE(xuiRichEditSetSelection(edit, length - 30, length - 12) == XUI_OK);
    REQUIRE(xuiRichEditCopy(edit) == XUI_OK);
    REQUIRE(strncmp(xuiTestProxyGetClipboardText(&proxy), text + length - 30, 18) == 0);
    REQUIRE(xuiLayout(context) == XUI_OK);
    REQUIRE(__xuiRichEditRender(edit, draw, 0, NULL) == XUI_OK);
    REQUIRE(g_work.LayoutBlock <= 180 && !p->bExactLayout);
    r = __xuiRichEditCaret(edit, p, length - 12);
    content = __xuiRichEditContentRect(edit, p);
    REQUIRE(r.fY >= content.fY - 1 && r.fY + r.fH <= content.fY + content.fH + 1);
    i = __xuiRichEditHit(edit, p, r.fX, r.fY + r.fH * 0.5f);
    REQUIRE(i >= length - columns && i <= length);
    REQUIRE(__xuiRichEditLineMove(edit, p, -1, 0) == XUI_OK);
    REQUIRE(p->iCaret < length - 12);
    REQUIRE(__xuiRichEditLineMove(edit, p, 1, 1) == XUI_OK);
    REQUIRE(p->iCaret >= length - columns);
    REQUIRE(xuiRichEditSetSelection(edit, 0, length) == XUI_OK);
    REQUIRE(xuiRichEditQueryCommand(edit, XUI_RICH_COMMAND_BOLD, &state) == XUI_OK);
    REQUIRE(state == XUI_RICH_COMMAND_STATE_OFF && !p->bExactLayout);

    /* Edits above the viewport rebuild only lightweight offsets and preserve node identity. */
    REQUIRE(xuiRichEditSetScroll(edit, 0, (float)__xuiRichEditHeightBefore(p, 5000) + 2.0f) == XUI_OK);
    REQUIRE(xuiLayout(context) == XUI_OK);
    k = __xuiRichEditParagraphByY(p, p->fScrollY); anchor = p->pParagraphs[k].iId;
    resetWork();
    REQUIRE(xuiRichDocumentReplace(doc, 3, 3, "inserted", &style) == XUI_OK);
    REQUIRE(xuiLayout(context) == XUI_OK);
    REQUIRE(__xuiRichEditRender(edit, draw, 0, NULL) == XUI_OK);
    k = __xuiRichEditParagraphByY(p, p->fScrollY);
    REQUIRE(p->pParagraphs[k].iId == anchor && g_work.LayoutBlock <= 180);
    REQUIRE(xuiRichDocumentUndo(doc) == XUI_OK);
    REQUIRE(xuiLayout(context) == XUI_OK);
    REQUIRE(xuiRichDocumentRedo(doc) == XUI_OK);
    REQUIRE(xuiLayout(context) == XUI_OK);
    REQUIRE(xuiRichDocumentReplace(doc, xuiRichDocumentGetLength(doc), xuiRichDocumentGetLength(doc),
        "\nlast a\xcc\x81 \xf0\x9f\x91\xa8\xe2\x80\x8d\xf0\x9f\x91\xa9", &style) == XUI_OK);
    REQUIRE(xuiLayout(context) == XUI_OK);
    REQUIRE(p->iParagraphCount == count + 1);
    REQUIRE(xuiRichEditSetSelection(edit, xuiRichDocumentGetLength(doc), xuiRichDocumentGetLength(doc)) == XUI_OK);
    REQUIRE(xuiLayout(context) == XUI_OK);
    REQUIRE(__xuiRichEditRender(edit, draw, 0, NULL) == XUI_OK);
    REQUIRE(p->pBlocks[p->iBlockCount - 1].iDocumentEnd == xuiRichDocumentGetLength(doc));
    REQUIRE(!p->bExactLayout && privateBytes(p) < 3 * 1048576);

    /* A failed materialization is reported and retry recreates its missing fragments. */
    g_failAllocation = 0; p->bLayoutDirty = 1;
    REQUIRE(__xuiRichEditEnsureLayout(edit, p) == XUI_ERROR_OUT_OF_MEMORY);
    g_failAllocation = -1;
    REQUIRE(__xuiRichEditEnsureLayout(edit, p) == XUI_OK);
    REQUIRE(__xuiRichEditRender(edit, draw, 0, NULL) == XUI_OK);
    printf("RichEdit lazy N=10000 first_layout=%u resize_layout=%u first_private_bytes=%zu cached=%d warm_index=0 warm_shape=0 dpi_pixel_size=18 zoom_pixel_size=27\n",
        cold, resized, bytes, p->iBlockCount);
cleanup:
    g_failAllocation = -1;
    if (draw != NULL) proxy.tProxy.drawEnd(&proxy.tProxy, draw);
    if (context != NULL) xuiDestroy(context);
    if (target != NULL) proxy.tProxy.surfaceDestroy(&proxy.tProxy, target);
    if (doc != NULL) xuiRichDocumentDestroy(doc);
    if (font != NULL) proxy.tProxy.fontDestroy(&proxy.tProxy, font);
    free(text);
    return failed;
}

static int lazyWidgetTest(void)
{
    const int count = 10000;
    xui_test_proxy_state_t proxy;
    xui_context context = NULL;
    xui_widget edit = NULL, input = NULL, cell = NULL;
    xui_widget* widgets = NULL;
    xui_font font = NULL;
    xui_rich_document doc = NULL;
    xui_rich_edit_data_t* p;
    xui_rich_edit_desc_t desc = {0};
    xui_rich_table_desc_t tableDesc = {0};
    xui_rich_node table, paragraph;
    xui_rich_node_info_t info = {0};
    unsigned cold, arranged;
    int failed = 0, i;
    xuiTestProxyInit(&proxy);
    REQUIRE(xuiCreate(&context) == XUI_OK);
    REQUIRE(xuiSetProxy(context, &proxy.tProxy) == XUI_OK);
    REQUIRE(proxy.tProxy.fontLoadFile(&proxy.tProxy, &font, "widgets.ttf", 14, 0) == XUI_OK);
    REQUIRE(xuiSetDefaultFont(context, font) == XUI_OK);
    REQUIRE(xuiInputViewport(context, 960, 640) == XUI_OK);
    REQUIRE(xuiRichDocumentCreate(&doc) == XUI_OK);
    widgets = (xui_widget*)calloc(count, sizeof(*widgets));
    REQUIRE(widgets != NULL);
    desc.iSize = sizeof(desc); desc.pFont = font; desc.bWordWrap = 1;
    REQUIRE(xuiRichEditCreate(context, &input, &desc) == XUI_OK);
    REQUIRE(xuiRichDocumentBeginTransaction(doc) == XUI_OK);
    for (i = 0; i < count; i++) {
        paragraph = i == 0 ? xuiRichNodeGetFirstChild(xuiRichDocumentGetRoot(doc)) : xuiRichDocumentAppendParagraph(doc);
        REQUIRE(paragraph != NULL);
        if (i == 0) widgets[i] = input;
        else REQUIRE(xuiWidgetCreate(context, &widgets[i]) == XUI_OK);
        REQUIRE(xuiRichDocumentAppendWidget(doc, paragraph, widgets[i], 100, 64, 64) != NULL);
    }
    tableDesc.iSize = sizeof(tableDesc); tableDesc.iRows = 2; tableDesc.iColumns = 2;
    table = xuiRichDocumentAppendTable(doc, &tableDesc);
    REQUIRE(table != NULL);
    REQUIRE(xuiRichTableSetCellText(doc, table, 0, 0, "cell", NULL) == XUI_OK);
    REQUIRE(xuiRichDocumentEndTransaction(doc) == XUI_OK);
    desc.pDocument = doc;
    REQUIRE(xuiRichEditCreate(context, &edit, &desc) == XUI_OK);
    REQUIRE(xuiSetRootWidget(context, edit) == XUI_OK);
    REQUIRE(xuiWidgetSetRect(edit, (xui_rect_t){0, 0, 936, 616}) == XUI_OK);
    p = __xuiRichEditData(edit);
    resetWork();
    REQUIRE(xuiLayout(context) == XUI_OK);
    REQUIRE(__xuiRichEditPrepareLayout(edit, p) == XUI_OK);
    REQUIRE(xuiLayout(context) == XUI_OK);
    cold = g_work.LayoutBlock; arranged = g_work.ArrangeWidget;
    REQUIRE(cold < 100 && arranged < 80 && !p->bExactLayout);
    REQUIRE(xuiWidgetGetParent(widgets[0]) == edit && xuiWidgetGetParent(widgets[count - 1]) == NULL);
    REQUIRE(privateBytes(p) < 4 * 1048576);
    REQUIRE(xuiSetFocusWidget(context, input) == XUI_OK);
    REQUIRE(xuiInputImeComposition(context, "a\xcc\x81", -1, 0, 3) == XUI_OK);
    REQUIRE(xuiDispatchPendingEvents(context) == XUI_OK);
    REQUIRE(__xuiRichEditData(input)->bImeActive);
    REQUIRE(xuiSetPointerCaptureEx(context, 7, XUI_POINTER_TYPE_PEN, widgets[1]) == XUI_OK);
    REQUIRE(xuiSetPointerCaptureEx(context, 42, XUI_POINTER_TYPE_TOUCH, widgets[2]) == XUI_OK);
    resetWork();
    REQUIRE(xuiRichEditSetScroll(edit, 0, (float)__xuiRichEditHeightBefore(p, 9900)) == XUI_OK);
    REQUIRE(xuiLayout(context) == XUI_OK);
    REQUIRE(g_work.LayoutBlock < 100 && g_work.ArrangeWidget < 80 && g_work.ParagraphIndex == 0);
    REQUIRE(xuiWidgetGetParent(input) == edit && xuiGetFocusWidget(context) == input);
    REQUIRE(xuiWidgetGetParent(widgets[1]) == edit && xuiGetPointerCaptureEx(context, 7, XUI_POINTER_TYPE_PEN) == widgets[1]);
    REQUIRE(xuiWidgetGetParent(widgets[2]) == edit && xuiGetPointerCaptureEx(context, 42, XUI_POINTER_TYPE_TOUCH) == widgets[2]);
    REQUIRE(xuiWidgetGetParent(widgets[3]) == NULL && xuiWidgetGetParent(widgets[9900]) == edit);
    REQUIRE(__xuiRichEditData(input)->bImeActive && context->bImeEnabled);
    REQUIRE(xuiReleasePointerCaptureEx(context, 7, XUI_POINTER_TYPE_PEN, widgets[1]) == XUI_OK);
    REQUIRE(xuiReleasePointerCaptureEx(context, 42, XUI_POINTER_TYPE_TOUCH, widgets[2]) == XUI_OK);
    REQUIRE(xuiSetFocusWidget(context, edit) == XUI_OK);
    REQUIRE(xuiRichEditSetScroll(edit, 0, (float)__xuiRichEditHeightBefore(p, 9900)) == XUI_OK);
    REQUIRE(xuiLayout(context) == XUI_OK);
    REQUIRE(xuiWidgetGetParent(input) == NULL && xuiWidgetGetParent(widgets[1]) == NULL && xuiWidgetGetParent(widgets[2]) == NULL);

    /* Accessibility lays out an offscreen object but does not attach/arrange it. */
    paragraph = p->pParagraphs[4000].pNode;
    REQUIRE(xuiRichNodeGetInfo(xuiRichNodeGetFirstChild(paragraph), &info) == XUI_OK);
    {
        xui_rect_t bounds = __xuiRichEditAccessibleNodeBounds(edit, p, xuiRichNodeGetFirstChild(paragraph));
        REQUIRE(bounds.fW == 100 && bounds.fH == 64);
        REQUIRE(xuiWidgetGetParent(widgets[4000]) == NULL);
    }
    REQUIRE(xuiRichNodeGetInfo(table, &info) == XUI_OK);
    REQUIRE(xuiRichEditBeginTableCellEdit(edit, info.iId, 0, 0) == XUI_OK);
    cell = xuiRichEditGetTableCellEditor(edit, NULL, NULL, NULL);
    REQUIRE(cell != NULL);
    REQUIRE(xuiLayout(context) == XUI_OK);
    REQUIRE(xuiRichEditSetScroll(edit, 0, 0) == XUI_OK);
    REQUIRE(xuiLayout(context) == XUI_OK);
    REQUIRE(xuiWidgetGetParent(cell) == edit && xuiGetFocusWidget(context) == cell);
    REQUIRE(xuiWidgetGetRect(cell).fW > 0 && xuiWidgetGetRect(cell).fH > 0);
    REQUIRE(xuiRichEditInsertText(cell, " changed") == XUI_OK);
    REQUIRE(xuiRichEditEndTableCellEdit(edit) == XUI_OK);
    cell = NULL;
    REQUIRE(strstr(xuiRichTableGetCellText(table, 0, 0), "changed") != NULL);
    REQUIRE(xuiLayout(context) == XUI_OK);
    REQUIRE(xuiWidgetGetParent(input) == edit);
    printf("RichEdit lazy widgets N=10000 first_layout=%u first_arrange=%u cached=%d private_bytes=%zu focus_ime_multi_capture=preserved\n",
        cold, arranged, p->iBlockCount, privateBytes(p));
cleanup:
    if (context != NULL) xuiDestroy(context);
    if (doc != NULL) xuiRichDocumentDestroy(doc);
    if (font != NULL) proxy.tProxy.fontDestroy(&proxy.tProxy, font);
    free(widgets);
    return failed;
}
