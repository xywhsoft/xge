/* Geometry oracle for the fractional fixture. It constructs a separate full flow layout,
 * reads raw xLayout output, and never calls the production paragraph/fragment/rect builders. */
typedef struct fractional_item_t {
    xui_rich_node node;
    xui_font font;
    xui_document_node_id_t id;
    int paragraph, line, start, end, nodeStart, nodeEnd, type;
    double left, top, right, bottom;
    uint32_t color, background, flags;
} fractional_item_t;
typedef struct fractional_reference_t {
    fractional_item_t items[48 * 97];
    int count, starts[48], lengths[48], documentStarts[48];
    double prefix[48];
    float heights[48], advances[48];
    double height;
} fractional_reference_t;

static xui_rect_t fractionalRect(const fractional_item_t* f, double prefix, double x, double y)
{
    xui_rect_t r;
    r.fX = (int)floor(f->left + x + 0.5);
    r.fY = (int)floor((prefix + f->top) + y + 0.5);
    r.fW = (int)floor(f->right + x + 0.5) - r.fX;
    r.fH = (int)floor((prefix + f->bottom) + y + 0.5) - r.fY;
    return r;
}

static int fractionalBuildReference(xui_widget edit, fractional_reference_t* ref)
{
    xui_rich_edit_data_t* p = __xuiRichEditData(edit);
    xui_proxy proxy = xuiInternalContextGetProxy(xuiWidgetGetContext(edit));
    xlayout_context_t* layout = xLayoutContextCreate(NULL);
    xlayout_node_t root = XLAYOUT_NODE_INVALID;
    xui_rich_node node;
    int failed = 0, paragraph = 0, at = 0, line = 0;
    float width = (float)__xuiRichEditContentRect(edit, p).fW;
    memset(ref, 0, sizeof(*ref));
    REQUIRE(layout != NULL);
    for (node = xuiRichNodeGetFirstChild(xuiRichDocumentGetRoot(p->pDocument));
         node != NULL; node = xuiRichNodeGetNextSibling(node), paragraph++) {
        xui_rich_node_info_t info = {0};
        float before, after, height;
        int start = at;
        REQUIRE(paragraph < 48);
        REQUIRE(xuiRichNodeGetInfo(node, &info) == XUI_OK);
        REQUIRE(info.iType == XUI_RICH_NODE_PARAGRAPH);
        REQUIRE(info.tParagraphStyle.iAlign == XUI_RICH_ALIGN_LEFT && info.tParagraphStyle.fIndentLeft == 0 &&
            info.tParagraphStyle.fIndentRight == 0 && info.tParagraphStyle.fFirstLineIndent == 0);
        before = info.tParagraphStyle.fSpaceBefore * p->fZoom;
        after = info.tParagraphStyle.fSpaceAfter * p->fZoom;
        ref->prefix[paragraph] = ref->height;
        ref->starts[paragraph] = ref->count;
        ref->documentStarts[paragraph] = start;
        /* Only shaping/cluster metrics are shared. No lazy geometry is read here. */
        REQUIRE(__xuiRichEditBuildAtoms(edit, p, node, &at) == XUI_OK);
        if (p->iAtomCount == 0) {
            xui_font_metrics_t metrics = {0};
            fractional_item_t* f = &ref->items[ref->count++];
            float lineHeight;
            REQUIRE(proxy->fontGetMetrics(proxy, p->pFont, &metrics) == XUI_OK);
            lineHeight = fmaxf(info.tParagraphStyle.fLineHeight * p->fZoom, metrics.fLineHeight);
            f->node = node; f->id = info.iId; f->paragraph = paragraph; f->line = line++;
            f->start = f->end = start; f->type = info.iType;
            f->left = f->right = info.tParagraphStyle.fIndentLeft * p->fZoom;
            f->top = before; f->bottom = before + lineHeight;
            height = before; height += lineHeight; height += after;
        } else {
            xlayout_node_t source = xLayoutNodeCreate(layout, XLAYOUT_ROLE_LEAF);
            xlayout_style_t style = xLayoutStyleDefault();
            xlayout_constraints_t constraints = xLayoutConstraints(width, XLAYOUT_UNBOUNDED);
            xlayout_measure_t measure;
            xui_rich_flow_t flow = {p->pAtoms, p->iAtomCount};
            float lastY = -1.0f;
            uint32_t j;
            root = xLayoutNodeCreate(layout, XLAYOUT_ROLE_CONTAINER);
            REQUIRE(root != XLAYOUT_NODE_INVALID && source != XLAYOUT_NODE_INVALID);
            style.container.format = XLAYOUT_FORMAT_FLOW;
            style.container.axis = XLAYOUT_HORIZONTAL;
            style.container.align_items = XLAYOUT_ALIGN_BASELINE;
            style.container.row_gap = p->fLineGap * p->fZoom;
            REQUIRE(xLayoutNodeSetStyle(layout, root, &style));
            REQUIRE(xLayoutNodeSetUserData(layout, source, &flow));
            REQUIRE(xLayoutNodeSetFragmentSource(layout, source, __xuiRichFlowCount, __xuiRichFlowMeasure));
            REQUIRE(xLayoutNodeAppend(layout, root, source));
            REQUIRE(xLayoutMeasure(layout, root, &constraints, &measure));
            REQUIRE(xLayoutArrange(layout, root, (xlayout_rect_t){0, 0, width, measure.height}));
            for (j = 0; j < xLayoutNodeFragmentCount(layout, source); j++) {
                xlayout_fragment_t raw;
                const xui_rich_atom_t* atom;
                fractional_item_t* f;
                xui_rich_node_info_t child = {0};
                REQUIRE(xLayoutNodeGetFragment(layout, source, j, &raw));
                REQUIRE(ref->count < (int)(sizeof(ref->items) / sizeof(ref->items[0])));
                f = &ref->items[ref->count++];
                atom = &flow.pAtoms[raw.tag];
                REQUIRE(xuiRichNodeGetInfo(atom->pNode, &child) == XUI_OK);
                if (lastY >= 0 && raw.rect.y > lastY + 0.1f) line++;
                lastY = raw.rect.y;
                f->node = atom->pNode; f->id = child.iId; f->font = atom->pFont;
                f->paragraph = paragraph; f->line = line; f->type = atom->iNodeType;
                f->start = atom->iDocumentStart; f->end = atom->iDocumentEnd;
                f->nodeStart = atom->iNodeStart; f->nodeEnd = atom->iNodeEnd;
                f->color = atom->iTextColor; f->background = atom->iBackgroundColor; f->flags = atom->iStyleFlags;
                f->left = raw.rect.x; f->right = raw.rect.x + raw.rect.width;
                f->top = (double)raw.rect.y + before;
                f->bottom = (double)(raw.rect.y + raw.rect.height) + before;
            }
            line++;
            height = before + after; height += measure.height;
            xLayoutNodeDestroy(layout, root); root = XLAYOUT_NODE_INVALID;
        }
        ref->lengths[paragraph] = ref->count - ref->starts[paragraph];
        ref->heights[paragraph] = height;
        if (xuiRichNodeGetNextSibling(node) != NULL) { at++; height += p->fParagraphGap * p->fZoom; }
        ref->advances[paragraph] = fmaxf(1.0f, height);
        ref->height += ref->advances[paragraph];
    }
    REQUIRE(paragraph == 48);
cleanup:
    if (layout != NULL) xLayoutContextDestroy(layout);
    return failed;
}

static xui_rect_t fractionalScreenRect(const fractional_reference_t* ref, int i, xui_rect_t content, float x, float y)
{
    return fractionalRect(&ref->items[i], ref->prefix[ref->items[i].paragraph],
        (double)content.fX - x, (double)content.fY - y);
}

static void fractionalDrawReference(xui_widget edit, xui_draw_context draw, const fractional_reference_t* ref)
{
    xui_rich_edit_data_t* p = __xuiRichEditData(edit);
    xui_proxy proxy = xuiInternalContextGetProxy(xuiWidgetGetContext(edit));
    xui_rect_t content = __xuiRichEditContentRect(edit, p);
    int i, start = p->iAnchor < p->iCaret ? p->iAnchor : p->iCaret;
    int end = p->iAnchor > p->iCaret ? p->iAnchor : p->iCaret;
    if ((p->iBackgroundColor & 255) != 0)
        proxy->drawRectFill(proxy, draw, xuiWidgetGetContentRect(edit), p->iBackgroundColor);
    for (i = 0; i < 48; i++) {
        fractional_item_t block = {0};
        xui_rect_t r;
        block.right = content.fW; block.bottom = ref->heights[i];
        r = fractionalRect(&block, ref->prefix[i], content.fX - (double)p->fScrollX, content.fY - (double)p->fScrollY);
        if (r.fY + r.fH < content.fY || r.fY > content.fY + content.fH) continue;
        /* Every fixture paragraph has the same independently specified background. */
        proxy->drawRectFill(proxy, draw, r, XUI_COLOR_RGBA(247,248,249,255));
    }
    for (i = 0; i < ref->count; i++) {
        const fractional_item_t* f = &ref->items[i];
        xui_rect_t r = fractionalScreenRect(ref, i, content, p->fScrollX, p->fScrollY);
        if (r.fY + r.fH < content.fY || r.fY > content.fY + content.fH ||
            r.fX + r.fW < content.fX || r.fX > content.fX + content.fW) continue;
        if ((f->background & 255) != 0) proxy->drawRectFill(proxy, draw, r, f->background);
        if (end > f->start && start < f->end) proxy->drawRectFill(proxy, draw, r, p->iSelectionColor);
    }
    for (i = 0; i < ref->count;) {
        const fractional_item_t* f = &ref->items[i];
        xui_rich_node_info_t info = {0};
        xui_rect_t r = fractionalScreenRect(ref, i, content, p->fScrollX, p->fScrollY);
        int j = i + 1, textEnd = f->nodeEnd;
        char text[97];
        if (f->type != XUI_RICH_NODE_TEXT && f->type != XUI_RICH_NODE_LINK) { i++; continue; }
        while (j < ref->count) {
            const fractional_item_t* next = &ref->items[j];
            xui_rect_t nr = fractionalScreenRect(ref, j, content, p->fScrollX, p->fScrollY);
            if (next->node != f->node || next->font != f->font || next->color != f->color ||
                next->flags != f->flags || next->line != f->line || next->nodeStart != textEnd ||
                nr.fX != r.fX + r.fW) break;
            textEnd = next->nodeEnd; r.fW = nr.fX + nr.fW - r.fX;
            if (nr.fH > r.fH) r.fH = nr.fH;
            j++;
        }
        if (r.fY + r.fH >= content.fY && r.fY <= content.fY + content.fH) {
            xuiRichNodeGetInfo(f->node, &info);
            memcpy(text, info.sText + f->nodeStart, (size_t)(textEnd - f->nodeStart));
            text[textEnd - f->nodeStart] = 0;
            proxy->drawText(proxy, draw, f->font, text, r, f->color,
                XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_TOP | XUI_TEXT_CLIP);
        }
        i = j;
    }
}

static xui_rect_t fractionalCaret(const fractional_reference_t* ref, int offset, xui_rect_t content, float x, float y)
{
    int i = 0;
    xui_rect_t r;
    while (i + 1 < ref->count && ref->items[i].end < offset) i++;
    r = fractionalScreenRect(ref, i, content, x, y);
    if (offset > ref->items[i].start) r.fX += r.fW;
    r.fW = 1;
    return r;
}

static int fractionalHit(const fractional_reference_t* ref, xui_rect_t content, float sx, float sy, float x, float y)
{
    int i = 0, end, best;
    double distance = DBL_MAX;
    xui_rect_t r = fractionalScreenRect(ref, 0, content, sx, sy);
    if (y < r.fY) return 0;
    r = fractionalScreenRect(ref, ref->count - 1, content, sx, sy);
    if (y > r.fY + r.fH) return ref->items[ref->count - 1].end;
    for (;;) {
        int bottom, nextTop;
        r = fractionalScreenRect(ref, i, content, sx, sy); bottom = r.fY + r.fH;
        end = i + 1;
        while (end < ref->count && ref->items[end].line == ref->items[i].line) {
            r = fractionalScreenRect(ref, end++, content, sx, sy);
            if (r.fY + r.fH > bottom) bottom = r.fY + r.fH;
        }
        if (end == ref->count) break;
        r = fractionalScreenRect(ref, end, content, sx, sy); nextTop = r.fY;
        if (y < (bottom + nextTop) * 0.5) break;
        i = end;
    }
    best = ref->items[i].start;
    for (; i < end; i++) {
        double d;
        r = fractionalScreenRect(ref, i, content, sx, sy);
        d = fabs((double)x - r.fX);
        if (d < distance) { distance = d; best = ref->items[i].start; }
        d = fabs((double)x - (r.fX + r.fW));
        if (d <= distance) { distance = d; best = ref->items[i].end; }
    }
    return best;
}

/* Keep the review probe's decimal spacing, empty paragraphs, zoom and three scroll phases. */
static int fractionalGeometryTest(void)
{
    enum { paragraphs = 48, columns = 96, views = 3 };
    const float scrolls[views] = {0.0f, 600.25f, 1200.5f};
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
    xui_rich_paragraph_style_t para = {0};
    fractional_reference_t* ref = NULL;
    xui_rich_layout_fragment_t fragment;
    xui_rich_node node;
    xui_rect_t content, actual, expected;
    uint64_t trace;
    char text[columns + 1];
    int failed = 0, i, j, k, phase, splitRound = 0, splitTruncate = 0, caretChecks = 0, hitChecks = 0;
    int changedHeight = 0, geometryChecks = 0;
    xuiTestProxyInit(&proxy);
    g_drawText = proxy.tProxy.drawText; proxy.tProxy.drawText = auditText;
    g_fill = proxy.tProxy.drawRectFill; proxy.tProxy.drawRectFill = auditFill;
    REQUIRE(xuiRichDocumentCreate(&doc) == XUI_OK);
    REQUIRE(xuiRichDocumentBeginTransaction(doc) == XUI_OK);
    para.iSize = sizeof(para); para.fSpaceBefore = 0.25f; para.fSpaceAfter = 0.35f;
    para.iBackgroundColor = XUI_COLOR_RGBA(247,248,249,255);
    for (i = 0; i < columns; i++) text[i] = "Rich fractional "[i % 16];
    text[columns] = 0;
    for (i = 0; i < paragraphs; i++) {
        node = i == 0 ? xuiRichNodeGetFirstChild(xuiRichDocumentGetRoot(doc)) : xuiRichDocumentAppendParagraph(doc);
        REQUIRE(node != NULL);
        if (i % 7 != 3) REQUIRE(xuiRichDocumentAppendText(doc, node, text, &style) != NULL);
        REQUIRE(xuiRichNodeSetParagraphStyle(doc, node, &para) == XUI_OK);
    }
    REQUIRE(xuiRichDocumentEndTransaction(doc) == XUI_OK);
    REQUIRE(xuiCreate(&context) == XUI_OK);
    REQUIRE(xuiSetProxy(context, &proxy.tProxy) == XUI_OK);
    REQUIRE(proxy.tProxy.fontLoadFile(&proxy.tProxy, &font, "fractional.ttf", 14, 0) == XUI_OK);
    REQUIRE(xuiSetDefaultFont(context, font) == XUI_OK);
    REQUIRE(xuiInputViewport(context, 360, 240) == XUI_OK);
    desc.iSize = sizeof(desc); desc.pDocument = doc; desc.pFont = font; desc.bWordWrap = 1;
    desc.fParagraphGap = 0.45f; desc.fLineGap = 0.35f;
    REQUIRE(xuiRichEditCreate(context, &edit, &desc) == XUI_OK);
    REQUIRE(xuiSetRootWidget(context, edit) == XUI_OK);
    REQUIRE(xuiRichEditSetZoom(edit, 1.25f) == XUI_OK);
    REQUIRE(xuiTestSurfaceCreate(&proxy, &target, 360, 240, XUI_SURFACE_USAGE_TARGET) == XUI_OK);
    REQUIRE(proxy.tProxy.drawBegin(&proxy.tProxy, &draw, target) == XUI_OK);
    REQUIRE(xuiLayout(context) == XUI_OK);
    p = __xuiRichEditData(edit);
    ref = (fractional_reference_t*)malloc(sizeof(*ref));
    REQUIRE(ref != NULL);
    REQUIRE(fractionalBuildReference(edit, ref) == 0);
    REQUIRE(!p->bExactLayout && p->iBlockCount < paragraphs);

    /* First frame, before visiting the rest of the document. */
    g_trace = 0; g_recordCount = 0;
    REQUIRE(__xuiRichEditRender(edit, draw, 0, NULL) == XUI_OK);
    trace = g_trace; g_trace = 0;
    fractionalDrawReference(edit, draw, ref);
    REQUIRE(trace == g_trace);

    /* Visit exact targets but leave the ordinary lazy eviction path active. */
    for (i = 0; i < paragraphs; i++)
        REQUIRE(__xuiRichEditEnsureOffset(edit, p, ref->documentStarts[i]) == XUI_OK);
    REQUIRE(!p->bExactLayout && p->iBlockCount < paragraphs);
    REQUIRE(p->fContentHeight == (float)ref->height);
    for (i = 0; i < paragraphs; i++) {
        REQUIRE(p->pParagraphs[i].fAdvance == ref->advances[i]);
        REQUIRE(__xuiRichEditHeightBefore(p, i) == ref->prefix[i]);
    }
    p->iAnchor = 0; p->iCaret = xuiRichDocumentGetLength(doc);
    for (phase = 0; phase < 2; phase++) {
        if (phase) {
            REQUIRE(__xuiRichEditEnsureAllLayout(edit, p) == XUI_OK);
            REQUIRE(p->iTotalFragmentCount == ref->count);
        }
        for (j = 0; j < views; j++) {
            REQUIRE(xuiRichEditSetScroll(edit, 0, scrolls[j]) == XUI_OK);
            REQUIRE(p->fScrollY == scrolls[j]);
            REQUIRE(xuiLayout(context) == XUI_OK);
            content = __xuiRichEditContentRect(edit, p);
            REQUIRE(p->bExactLayout == phase);
            g_trace = 0; g_recordCount = 0;
            REQUIRE(__xuiRichEditRender(edit, draw, 0, NULL) == XUI_OK);
            trace = g_trace; g_trace = 0;
            fractionalDrawReference(edit, draw, ref);
            REQUIRE(trace == g_trace);
            for (i = 0; i < p->iBlockCount; i++) {
                xui_rich_block_layout_t* block = &p->pBlocks[i];
                int first = ref->starts[block->iParagraph];
                REQUIRE(block->iFragmentCount == ref->lengths[block->iParagraph]);
                for (k = 0; k < block->iFragmentCount; k++) {
                    const fractional_item_t* f = &ref->items[first + k];
                    __xuiRichEditGetBlockFragment(block, k, &fragment);
                    expected = fractionalRect(f, ref->prefix[f->paragraph], 0, 0);
                    if (memcmp(&fragment.tPublic.tRect, &expected, sizeof(expected)) != 0)
                        printf("geometry mismatch view=%d block=%d para=%d fragment=%d actual=(%d,%d,%d,%d) expected=(%d,%d,%d,%d) prefix=%.9f/%.9f\n",
                            j, i, block->iParagraph, k, fragment.tPublic.tRect.fX, fragment.tPublic.tRect.fY,
                            fragment.tPublic.tRect.fW, fragment.tPublic.tRect.fH, expected.fX, expected.fY, expected.fW, expected.fH,
                            block->fParagraphY, ref->prefix[f->paragraph]);
                    REQUIRE(memcmp(&fragment.tPublic.tRect, &expected, sizeof(expected)) == 0);
                    actual = __xuiRichEditFragmentRect(edit, p, &fragment);
                    expected = fractionalScreenRect(ref, first + k, content, p->fScrollX, p->fScrollY);
                    REQUIRE(memcmp(&actual, &expected, sizeof(actual)) == 0);
                    REQUIRE(fragment.tPublic.iNodeId == f->id && fragment.tPublic.iDocumentStart == f->start &&
                        fragment.tPublic.iDocumentEnd == f->end);
                    geometryChecks++;
                }
            }
            /* Caret requests may evict/reorder blocks; do not hold lazy cache pointers across them. */
            for (i = 0; i < ref->count; i++) {
                const fractional_item_t* f = &ref->items[i];
                actual = fractionalScreenRect(ref, i, content, p->fScrollX, p->fScrollY);
                if (actual.fY < content.fY || actual.fY + actual.fH > content.fY + content.fH) continue;
                for (k = 0; k < 2; k++) {
                    int offset = k ? f->end : f->start;
                    xui_rect_t caret = __xuiRichEditCaret(edit, p, offset);
                    xui_rect_t want = fractionalCaret(ref, offset, content, p->fScrollX, p->fScrollY);
                    REQUIRE(memcmp(&caret, &want, sizeof(caret)) == 0);
                    caretChecks++;
                }
            }
            /* Probe both sides of each final pixel and each inter-line midpoint in the viewport. */
            for (i = content.fY - 1; i <= content.fY + content.fH + 1; i++) for (k = 0; k < 3; k++) {
                float x = content.fX + 8.25f + 71.5f * k, y = i + 0.25f * k;
                REQUIRE(__xuiRichEditHit(edit, p, x, y) ==
                    fractionalHit(ref, content, p->fScrollX, p->fScrollY, x, y));
                hitChecks++;
            }
            printf("RichEdit fractional %s scroll=%.2f draw/caret/hit=equal cached=%d\n",
                phase ? "full" : "lazy", scrolls[j], p->iBlockCount);
        }
    }
    for (i = 0; i < ref->count; i++) {
        const fractional_item_t* f = &ref->items[i];
        double prefix = ref->prefix[f->paragraph];
        expected = fractionalRect(f, prefix, 0, 0);
        splitRound += expected.fY != (int)floor(prefix + 0.5) + (int)floor(f->top + 0.5);
        splitTruncate += expected.fY != (int)prefix + (int)f->top;
        changedHeight += expected.fH != (int)(f->bottom - f->top);
    }
    REQUIRE(splitRound > 0 && splitTruncate > 0 && changedHeight > 0);
    printf("RichEdit fractional raw-flow oracle fragments=%d geometry=%d caret=%d hit=%d "
        "split_round_rejected=%d split_truncate_rejected=%d edge_height_changes=%d height=%.6f\n",
        ref->count, geometryChecks, caretChecks, hitChecks, splitRound, splitTruncate, changedHeight, ref->height);
cleanup:
    if (draw != NULL) proxy.tProxy.drawEnd(&proxy.tProxy, draw);
    if (context != NULL) xuiDestroy(context);
    if (target != NULL) proxy.tProxy.surfaceDestroy(&proxy.tProxy, target);
    if (doc != NULL) xuiRichDocumentDestroy(doc);
    if (font != NULL) proxy.tProxy.fontDestroy(&proxy.tProxy, font);
    free(ref);
    return failed;
}

static int fractionalRecordedRect(xui_rect_t expected, const char* text)
{
    int i;
    for (i = 0; i < g_recordCount; i++)
        if (strcmp(g_records[i].text, text) == 0 && memcmp(&g_records[i].rect, &expected, sizeof(expected)) == 0) return 1;
    return 0;
}

/* Source dimensions and shared font metrics are the oracle here, not cached block geometry. */
static int fractionalObjectTest(void)
{
    xui_test_proxy_state_t proxy;
    xui_context context = NULL;
    xui_widget edit = NULL, child = NULL, cellEditor = NULL;
    xui_font font = NULL, sized;
    xui_surface target = NULL;
    xui_draw_context draw = NULL;
    xui_rich_document doc = NULL;
    xui_rich_edit_data_t* p;
    xui_rich_edit_desc_t desc = {0};
    xui_rich_table_desc_t tableDesc = {0};
    xui_rich_paragraph_style_t para = {0};
    xui_rich_node node, widgetNode, table, hitNode, link;
    xui_rich_node_info_t info = {0};
    xui_text_shape_t shape = {0};
    xui_font_metrics_t metrics = {0};
    xui_vec2_t measured = {0};
    fractional_item_t widgetRaw = {0}, tableRaw = {0}, linkRaw = {0};
    xui_rect_t content, expected, actual, cells[3][3];
    float before, after, gap, advance, cellHeight, tableWidth, widgetWidth, widgetHeight;
    double widgetPrefix, tablePrefix;
    int failed = 0, i, j, row, col, hitRow, hitColumn, start, end, cellChecks = 0;
    xuiTestProxyInit(&proxy);
    g_drawText = proxy.tProxy.drawText; proxy.tProxy.drawText = auditText;
    g_fill = proxy.tProxy.drawRectFill; proxy.tProxy.drawRectFill = auditFill;
    REQUIRE(xuiCreate(&context) == XUI_OK);
    REQUIRE(xuiSetProxy(context, &proxy.tProxy) == XUI_OK);
    REQUIRE(proxy.tProxy.fontLoadFile(&proxy.tProxy, &font, "fractional-object.ttf", 14, 0) == XUI_OK);
    REQUIRE(xuiSetDefaultFont(context, font) == XUI_OK);
    REQUIRE(xuiInputViewport(context, 360, 240) == XUI_OK);
    REQUIRE(xuiWidgetCreate(context, &child) == XUI_OK);
    REQUIRE(xuiRichDocumentCreate(&doc) == XUI_OK);
    REQUIRE(xuiRichDocumentBeginTransaction(doc) == XUI_OK);
    para.iSize = sizeof(para); para.fSpaceBefore = 0.25f; para.fSpaceAfter = 0.35f;
    node = xuiRichNodeGetFirstChild(xuiRichDocumentGetRoot(doc));
    link = xuiRichDocumentAppendLink(doc, node, "x", "https://example.test", NULL);
    REQUIRE(link != NULL);
    REQUIRE(xuiRichNodeSetParagraphStyle(doc, node, &para) == XUI_OK);
    node = xuiRichDocumentAppendParagraph(doc);
    widgetNode = xuiRichDocumentAppendWidget(doc, node, child, 20.25f, 18.25f, 18.25f);
    REQUIRE(widgetNode != NULL);
    para.iAlign = XUI_RICH_ALIGN_CENTER;
    REQUIRE(xuiRichNodeSetParagraphStyle(doc, node, &para) == XUI_OK);
    tableDesc.iSize = sizeof(tableDesc); tableDesc.iRows = 3; tableDesc.iColumns = 3;
    tableDesc.fWidth = 101.3f; tableDesc.fCellPadding = 0.3f;
    tableDesc.iHeaderColor = XUI_COLOR_RGBA(221,230,239,255);
    tableDesc.iCellColor = XUI_COLOR_RGBA(244,247,250,255);
    table = xuiRichDocumentAppendTable(doc, &tableDesc);
    REQUIRE(table != NULL);
    REQUIRE(xuiRichNodeSetParagraphStyle(doc, table, &para) == XUI_OK);
    for (row = 0; row < 3; row++) for (col = 0; col < 3; col++)
        REQUIRE(xuiRichTableSetCellText(doc, table, row, col, "x", NULL) == XUI_OK);
    for (i = 0; i < 24; i++) REQUIRE(xuiRichDocumentAppendParagraph(doc) != NULL);
    REQUIRE(xuiRichDocumentEndTransaction(doc) == XUI_OK);
    desc.iSize = sizeof(desc); desc.pDocument = doc; desc.pFont = font; desc.bWordWrap = 1;
    desc.fParagraphGap = 0.45f; desc.fLineGap = 0.35f;
    REQUIRE(xuiRichEditCreate(context, &edit, &desc) == XUI_OK);
    REQUIRE(xuiSetRootWidget(context, edit) == XUI_OK);
    REQUIRE(xuiRichEditSetZoom(edit, 1.25f) == XUI_OK);
    REQUIRE(xuiTestSurfaceCreate(&proxy, &target, 360, 240, XUI_SURFACE_USAGE_TARGET) == XUI_OK);
    REQUIRE(proxy.tProxy.drawBegin(&proxy.tProxy, &draw, target) == XUI_OK);
    REQUIRE(xuiLayout(context) == XUI_OK);
    p = __xuiRichEditData(edit);
    content = __xuiRichEditContentRect(edit, p);
    REQUIRE(!p->bExactLayout);
    sized = __xuiRichEditSizedFont(edit, p, font, 0);
    REQUIRE(xuiTextShape(context, sized, "x", 1, 0, &shape) == XUI_OK);
    before = para.fSpaceBefore * p->fZoom; after = para.fSpaceAfter * p->fZoom;
    gap = desc.fParagraphGap * p->fZoom;
    linkRaw.top = before; linkRaw.bottom = (double)shape.fLineHeight + before;
    linkRaw.right = shape.pClusters[0].fAdvance;
    advance = before + after; advance += shape.fLineHeight; advance += gap;
    xuiTextShapeFree(&shape);
    widgetPrefix = advance;
    widgetWidth = 20.25f * p->fZoom; widgetHeight = 18.25f * p->fZoom;
    widgetRaw.left = ((float)content.fW - widgetWidth) * 0.5f;
    widgetRaw.right = widgetRaw.left + widgetWidth;
    widgetRaw.top = before; widgetRaw.bottom = (double)widgetHeight + before;
    advance = before + after; advance += widgetHeight; advance += gap;
    tablePrefix = widgetPrefix + advance;
    REQUIRE(xuiRichNodeGetInfo(table, &info) == XUI_OK);
    REQUIRE(proxy.tProxy.fontGetMetrics(&proxy.tProxy, font, &metrics) == XUI_OK);
    REQUIRE(proxy.tProxy.textMeasure(&proxy.tProxy, sized, "x", &measured) == XUI_OK);
    cellHeight = fmaxf(metrics.fLineHeight, measured.fY) + info.fCellPadding * p->fZoom * 2;
    tableWidth = info.fWidth * p->fZoom;
    tableRaw.left = ((float)content.fW - tableWidth) * 0.5f;
    tableRaw.right = (float)tableRaw.left + tableWidth;
    tableRaw.top = before; tableRaw.bottom = before + cellHeight * 3;
    REQUIRE(p->pBlocks[1].fParagraphY == widgetPrefix && p->pBlocks[2].fParagraphY == tablePrefix);
    p->iAnchor = 0; p->iCaret = xuiRichDocumentGetLength(doc);
    for (j = 0; j < 3; j++) {
        REQUIRE(xuiRichEditSetScroll(edit, 0, j * 0.25f) == XUI_OK);
        REQUIRE(xuiLayout(context) == XUI_OK);
        REQUIRE(p->fScrollY == j * 0.25f);
        expected = fractionalRect(&linkRaw, 0, content.fX, content.fY - (double)p->fScrollY);
        REQUIRE(__xuiRichEditLinkAt(edit, p, expected.fX, expected.fY) == link);
        REQUIRE(__xuiRichEditLinkAt(edit, p, expected.fX, expected.fY - 0.25f) == NULL);
        REQUIRE(__xuiRichEditLinkAt(edit, p, expected.fX + expected.fW, expected.fY + expected.fH) == link);
        expected = fractionalRect(&widgetRaw, widgetPrefix, content.fX, content.fY - (double)p->fScrollY);
        actual = xuiWidgetGetRect(child);
        REQUIRE(memcmp(&actual, &expected, sizeof(actual)) == 0);
        REQUIRE(__xuiRichEditAtomicAt(edit, p, expected.fX + 1, expected.fY + 1, &start, &end, &hitNode));
        REQUIRE(hitNode == widgetNode && end - start == 3);
        actual = __xuiRichEditCaret(edit, p, start);
        REQUIRE(actual.fX == expected.fX && actual.fY == expected.fY && actual.fH == expected.fH);
        actual = __xuiRichEditCaret(edit, p, end);
        REQUIRE(actual.fX == expected.fX + expected.fW && actual.fY == expected.fY && actual.fH == expected.fH);
        REQUIRE(__xuiRichEditHit(edit, p, expected.fX + 1, expected.fY + 1) == start);
        REQUIRE(__xuiRichEditHit(edit, p, expected.fX + expected.fW - 1, expected.fY + 1) == end);
        g_trace = 0; g_recordCount = 0;
        REQUIRE(__xuiRichEditRender(edit, draw, 0, NULL) == XUI_OK);
        REQUIRE(fractionalRecordedRect(expected, "<fill>"));
        for (row = 0; row < 3; row++) for (col = 0; col < 3; col++) {
            fractional_item_t cellRaw = tableRaw;
            double w = tableRaw.right - tableRaw.left, h = tableRaw.bottom - tableRaw.top;
            cellRaw.left = tableRaw.left + w * col / 3;
            cellRaw.right = tableRaw.left + w * (col + 1) / 3;
            cellRaw.top = tableRaw.top + h * row / 3;
            cellRaw.bottom = tableRaw.top + h * (row + 1) / 3;
            expected = fractionalRect(&cellRaw, tablePrefix, content.fX, content.fY - (double)p->fScrollY);
            cells[row][col] = expected;
            REQUIRE(fractionalRecordedRect(expected, "<fill>"));
            for (i = 0; i < 2; i++) {
                REQUIRE(__xuiRichEditTableCellAt(edit, p, expected.fX + (i ? expected.fW - 1 : 0),
                    expected.fY + (i ? expected.fH - 1 : 0), &hitNode, &hitRow, &hitColumn, &actual));
                REQUIRE(hitNode == table && hitRow == row && hitColumn == col);
                REQUIRE(memcmp(&actual, &expected, sizeof(actual)) == 0);
                cellChecks++;
            }
            if (col) REQUIRE(cells[row][col - 1].fX + cells[row][col - 1].fW == expected.fX);
            if (row) REQUIRE(cells[row - 1][col].fY + cells[row - 1][col].fH == expected.fY);
            cellRaw.left += info.fCellPadding * p->fZoom; cellRaw.right -= info.fCellPadding * p->fZoom;
            cellRaw.top += info.fCellPadding * p->fZoom; cellRaw.bottom -= info.fCellPadding * p->fZoom;
            expected = fractionalRect(&cellRaw, tablePrefix, content.fX, content.fY - (double)p->fScrollY);
            REQUIRE(fractionalRecordedRect(expected, "x"));
        }
        REQUIRE(xuiRichEditBeginTableCellEdit(edit, info.iId, 1, 1) == XUI_OK);
        cellEditor = xuiRichEditGetTableCellEditor(edit, NULL, NULL, NULL);
        REQUIRE(cellEditor != NULL && xuiLayout(context) == XUI_OK);
        actual = xuiWidgetGetRect(cellEditor);
        REQUIRE(memcmp(&actual, &cells[1][1], sizeof(actual)) == 0);
        for (i = 0; i < xuiWidgetGetAccessibleNodeCount(edit); i++) {
            xui_accessible_node_t accessible = {0};
            REQUIRE(xuiWidgetGetAccessibleNode(edit, i, &accessible) == XUI_OK);
            if (accessible.iRole == XUI_ACCESSIBLE_ROLE_CELL) {
                expected = cells[accessible.iRow][accessible.iColumn];
                REQUIRE(memcmp(&accessible.tBounds, &expected, sizeof(expected)) == 0);
                cellChecks++;
            }
        }
        REQUIRE(__xuiRichEditEndCellEdit(edit, p, 0) == XUI_OK);
        cellEditor = NULL;
    }
    printf("RichEdit fractional objects zoom=1.25 scroll=0/0.25/0.5 widget_draw/caret/hit/arrange=equal "
        "table_shared_edges/hit/editor/a11y=%d checks\n", cellChecks);
cleanup:
    if (shape.pClusters != NULL) xuiTextShapeFree(&shape);
    if (draw != NULL) proxy.tProxy.drawEnd(&proxy.tProxy, draw);
    if (context != NULL) xuiDestroy(context);
    if (target != NULL) proxy.tProxy.surfaceDestroy(&proxy.tProxy, target);
    if (doc != NULL) xuiRichDocumentDestroy(doc);
    if (font != NULL) proxy.tProxy.fontDestroy(&proxy.tProxy, font);
    return failed;
}
