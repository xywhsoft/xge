static xui_text_shape_proc decoration_base_shape;
static xui_font_get_metrics_proc decoration_base_metrics;
static xui_draw_line_proc decoration_base_line;
static xui_draw_text_spans_proc decoration_base_spans;
static xui_draw_text_proc decoration_base_text;
static int decoration_invalid_metrics, decoration_metrics_error, decoration_line_error;
static int decoration_span_count, decoration_line_count;
static int decoration_text_count, decoration_underlined_text_count;
static uint32_t decoration_text_flags;
static struct { float left, right, y, width; uint32_t color; } decoration_lines[32];
static const char decoration_emoji[] = "\xf0\x9f\x91\xa9\xe2\x80\x8d\xf0\x9f\x92\xbb";
static int decoration_shape(xui_proxy proxy, const xui_text_item_t* pTextItem, xui_text_shape_t* shape)
{
    const char* text = pTextItem && pTextItem->iSize >= sizeof(*pTextItem) ? pTextItem->sText : NULL;
    int bytes = pTextItem && pTextItem->iSize >= sizeof(*pTextItem) ? pTextItem->iTextSize : 0;

    int result = decoration_base_shape(proxy, pTextItem, shape);
    if (result == XUI_OK) {
        int i, count = 0; double width = 0;
        for (i = 0; i < shape->iClusterCount; i++) {
            xui_text_cluster_t cluster = shape->pClusters[i];
            if (bytes - cluster.iTextStart >= 11 &&
                !memcmp(text + cluster.iTextStart, decoration_emoji, 11)) {
                CHECK(i + 2 < shape->iClusterCount);
                cluster.iTextEnd += 7; cluster.fAdvance = 19; i += 2;
            } else if (i + 1 < shape->iClusterCount &&
                text[cluster.iTextStart] == 'A' && text[shape->pClusters[i + 1].iTextStart] == 'V')
                cluster.fAdvance -= 3;
            shape->pClusters[count++] = cluster; width += cluster.fAdvance;
        }
        shape->iClusterCount = count; shape->fWidth = (float)width;
    }
    return result;
}
static int decoration_metrics(xui_proxy proxy, xui_font font, xui_font_metrics_t* metrics)
{
    int result = decoration_base_metrics(proxy, font, metrics);
    if (result == XUI_OK) {
        metrics->fUnderlinePosition = decoration_invalid_metrics ? NAN : 2.5f;
        metrics->fUnderlineThickness = decoration_invalid_metrics ? NAN : 1.5f;
        metrics->fStrikePosition = decoration_invalid_metrics ? INFINITY : -4.5f;
        metrics->fStrikeThickness = decoration_invalid_metrics ? -1 : 2;
        if (decoration_metrics_error) {
            metrics->fUnderlinePosition = metrics->fStrikePosition = 9;
            metrics->fUnderlineThickness = metrics->fStrikeThickness = 9;
            return XUI_ERROR_INVALID_STATE;
        }
    }
    return result;
}
static int decoration_line(xui_proxy proxy, xui_draw_context draw,
    float left, float y, float right, float y2, float width, uint32_t color)
{
    CHECK(decoration_line_count < 32 && fabsf(y - y2) < .01f);
    decoration_lines[decoration_line_count].left = left;
    decoration_lines[decoration_line_count].right = right;
    decoration_lines[decoration_line_count].y = y;
    decoration_lines[decoration_line_count].width = width;
    decoration_lines[decoration_line_count++].color = color;
    return decoration_line_error ? XUI_ERROR_INVALID_STATE :
        decoration_base_line(proxy, draw, left, y, right, y2, width, color);
}
static int decoration_spans(xui_proxy proxy, xui_draw_context draw, const xui_text_item_t* pTextItem, xui_rect_t rect, uint32_t color, uint32_t flags, const xui_text_paint_span_t* spans, int count)
{
    int bytes = pTextItem && pTextItem->iSize >= sizeof(*pTextItem) ? pTextItem->iTextSize : 0;

    CHECK(!(flags & XUI_TEXT_UNDERLINE) && count > 0 &&
        spans[0].iStart == 0 && spans[count - 1].iEnd == bytes);
    decoration_span_count++;
    return decoration_base_spans(proxy, draw, pTextItem, rect, color, flags, spans, count);
}
static int decoration_text(xui_proxy proxy, xui_draw_context draw, const xui_text_item_t* pTextItem, xui_rect_t rect, uint32_t color, uint32_t flags)
{

    decoration_text_flags = flags;
    decoration_text_count++;
    if (flags & XUI_TEXT_UNDERLINE) decoration_underlined_text_count++;
    return decoration_base_text(proxy, draw, pTextItem, rect, color, flags);
}
static int decoration_has(float left, float right, float y, float width, uint32_t color)
{
    int i;
    for (i = 0; i < decoration_line_count; i++)
        if (fabsf(decoration_lines[i].left - left) < .01f &&
            fabsf(decoration_lines[i].right - right) < .01f &&
            fabsf(decoration_lines[i].y - y) < .01f &&
            fabsf(decoration_lines[i].width - width) < .01f &&
            decoration_lines[i].color == color) return 1;
    return 0;
}
static xui_doc_position_t decoration_position(xui_document document, uint64_t node, uint64_t offset)
{
    xui_doc_position_t position = {0};
    position.iSize = sizeof(position); position.iKind = XUI_DOC_POSITION_TEXT;
    position.iDocumentId = xuiDocumentGetIdentity(document);
    position.iRevision = xuiDocumentGetRevision(document);
    position.iNodeId = node; position.iOffset = offset;
    return position;
}
static uint64_t decoration_insert(xui_document_transaction transaction, uint64_t parent,
    const char* text, uint32_t marks, uint32_t color)
{
    xui_doc_node_desc_t node = {0}; uint64_t id;
    node.iSize = sizeof(node); node.iKind = text ? XUI_DOC_TEXT : XUI_DOC_PARAGRAPH;
    node.sText = text; node.iTextBytes = text ? strlen(text) : 0;
    node.tAttributes.iMarks = marks; node.tAttributes.iTextColor = color;
    if (marks & XUI_DOC_LINK) node.sResource = "https://example.test/link";
    CHECK(xuiDocumentTxnInsertNode(transaction, parent, XUI_DOCUMENT_APPEND, &node, &id) == XUI_OK);
    return id;
}
static void document_decorated_shaping(xui_context parent, xui_surface target,
    xui_test_proxy_state_t* proxy)
{
    xui_context context; xui_font font = xuiGetDefaultFont(parent);
    xui_document document[2]; xui_document_renderer renderer[2]; uint64_t nodes[2][4] = {{0}};
    xui_doc_renderer_desc_t desc = {0}; unsigned variant, pass;
    const uint32_t blue = XUI_COLOR_RGBA(20, 40, 180, 255), red = XUI_COLOR_RGBA(180, 20, 40, 255);
    const uint32_t highlight = XUI_COLOR_RGBA(40, 180, 20, 128);
    decoration_base_shape = proxy->tProxy.textShape; decoration_base_metrics = proxy->tProxy.fontGetMetrics;
    decoration_base_line = proxy->tProxy.drawLine; decoration_base_spans = proxy->tProxy.drawTextSpans;
    decoration_base_text = proxy->tProxy.drawText;
    proxy->tProxy.textShape = decoration_shape; proxy->tProxy.fontGetMetrics = decoration_metrics;
    proxy->tProxy.drawLine = decoration_line; proxy->tProxy.drawTextSpans = decoration_spans;
    proxy->tProxy.drawText = decoration_text;
    CHECK(xuiCreate(&context) == XUI_OK && xuiSetProxy(context, &proxy->tProxy) == XUI_OK &&
        xuiSetDefaultFont(context, font) == XUI_OK);
    desc.iSize = sizeof(desc); desc.tFonts = (xui_doc_font_set_t){font, font, font, font, font};
    desc.fLineGap = 4; desc.iHighlightColor = highlight;
    for (variant = 0; variant < 2; variant++) {
        xui_document_transaction transaction; xui_document_snapshot snapshot;
        uint64_t paragraph; unsigned part;
        static const char* const split[] = {"A", "V ", "A", "V"};
        static const uint32_t marks[] = {XUI_DOC_BOLD | XUI_DOC_UNDERLINE,
            XUI_DOC_ITALIC | XUI_DOC_LINK | XUI_DOC_STRIKE,
            XUI_DOC_CODE | XUI_DOC_HIGHLIGHT | XUI_DOC_UNDERLINE, XUI_DOC_LINK | XUI_DOC_STRIKE};
        CHECK(xuiDocumentCreate(NULL, &document[variant]) == XUI_OK &&
            xuiDocumentBeginTransaction(document[variant], NULL, &transaction) == XUI_OK);
        paragraph = decoration_insert(transaction, 1, NULL, 0, 0);
        for (part = 0; part < (variant ? 4 : 1); part++)
            nodes[variant][part] = decoration_insert(transaction, paragraph, variant ? split[part] : "AV AV",
                variant ? marks[part] : 0, part & 1 ? red : blue);
        CHECK(xuiDocumentTxnCommit(transaction, NULL) == XUI_OK); xuiDocumentTxnRelease(transaction);
        CHECK(xuiDocumentAcquireSnapshot(document[variant], &snapshot) == XUI_OK &&
            xuiDocumentRendererCreate(context, &desc, &renderer[variant]) == XUI_OK &&
            xuiDocumentRendererSetSnapshot(renderer[variant], snapshot, NULL) == XUI_OK);
        xuiDocumentSnapshotRelease(snapshot);
    }
    for (pass = 0; pass < 3; pass++) {
        xui_doc_rect_t first[2], next[2], end[2];
        double width = pass == 1 ? 300 : 20;
        for (variant = 0; variant < 2; variant++) {
            xui_doc_position_t at;
            CHECK(xuiDocumentRendererLayout(renderer[variant], width, 0, 100) == XUI_OK);
            at = decoration_position(document[variant], nodes[variant][variant ? 1 : 0], variant ? 1 : 2);
            CHECK(xuiDocumentRendererGetCaretRect(renderer[variant], &at, &first[variant]) == XUI_OK);
            at = decoration_position(document[variant], nodes[variant][variant ? 2 : 0], variant ? 1 : 4);
            CHECK(xuiDocumentRendererGetCaretRect(renderer[variant], &at, &next[variant]) == XUI_OK);
            at = decoration_position(document[variant], nodes[variant][variant ? 3 : 0], variant ? 1 : 5);
            CHECK(xuiDocumentRendererGetCaretRect(renderer[variant], &at, &end[variant]) == XUI_OK);
        }
        CHECK(fabs(first[0].x - first[1].x) < .01 && fabs(first[0].y - first[1].y) < .01 &&
            fabs(next[0].x - next[1].x) < .01 && fabs(next[0].y - next[1].y) < .01 &&
            fabs(end[0].x - end[1].x) < .01 && fabs(end[0].y - end[1].y) < .01);
    }
    for (pass = 0; pass < 3; pass++) {
        xui_draw_context draw; float underline = pass ? 12.32f : 13.7f;
        float strike = pass ? 7.28f : 6.7f;
        decoration_invalid_metrics = pass == 1; decoration_metrics_error = pass == 2;
        decoration_line_count = decoration_span_count = 0;
        xuiTestSurfaceReset(target);
        CHECK(proxy->tProxy.drawBegin(&proxy->tProxy, &draw, target) == XUI_OK &&
            xuiDocumentRendererDraw(renderer[1], draw, 0, 0, (xui_rect_t){0, 0, 300, 100}, NULL, 0) == XUI_OK &&
            proxy->tProxy.drawEnd(&proxy->tProxy, draw) == XUI_OK &&
            decoration_span_count == 2 && decoration_line_count == 6);
        CHECK(decoration_has(0, 4, underline, pass ? 1 : 1.5f, blue) &&
            decoration_has(4, 18, underline, pass ? 1 : 1.5f, red) &&
            decoration_has(4, 18, strike, pass ? 1 : 2, red) &&
            decoration_has(0, 4, underline + 18, pass ? 1 : 1.5f, blue) &&
            decoration_has(4, 11, underline + 18, pass ? 1 : 1.5f, red) &&
            decoration_has(4, 11, strike + 18, pass ? 1 : 2, red) &&
            xuiTestSurfaceGetRectFillColorCount(target, highlight) == 1);
    }
    decoration_invalid_metrics = decoration_metrics_error = 0;
    {
        xui_document_transaction transaction; xui_document_snapshot snapshot;
        xui_document_change_set changes; xui_doc_range_t range;
        xui_draw_context draw;
        range.tAnchor = decoration_position(document[1], nodes[1][0], 0);
        range.tCaret = decoration_position(document[1], nodes[1][0], 1);
        CHECK(xuiDocumentBeginTransaction(document[1], NULL, &transaction) == XUI_OK &&
            xuiDocumentTxnSetMarks(transaction, &range, 0, XUI_DOC_UNDERLINE) == XUI_OK &&
            xuiDocumentTxnCommit(transaction, &changes) == XUI_OK);
        xuiDocumentTxnRelease(transaction);
        CHECK(xuiDocumentAcquireSnapshot(document[1], &snapshot) == XUI_OK &&
            xuiDocumentRendererSetSnapshot(renderer[1], snapshot, changes) == XUI_OK &&
            xuiDocumentRendererLayout(renderer[1], 20, 0, 100) == XUI_OK);
        xuiDocumentSnapshotRelease(snapshot); xuiDocumentChangeSetRelease(changes);
        decoration_line_count = decoration_span_count = 0;
        CHECK(proxy->tProxy.drawBegin(&proxy->tProxy, &draw, target) == XUI_OK &&
            xuiDocumentRendererDraw(renderer[1], draw, 0, 0, (xui_rect_t){0, 0, 300, 100}, NULL, 0) == XUI_OK &&
            proxy->tProxy.drawEnd(&proxy->tProxy, draw) == XUI_OK && decoration_span_count == 2 &&
            decoration_line_count == 5 && !decoration_has(0, 4, 13.7f, 1.5f, blue));
    }
    for (variant = 0; variant < 2; variant++) {
        xuiDocumentRendererRelease(renderer[variant]); xuiDocumentRelease(document[variant]);
    }
    {
        xui_document_transaction transaction; xui_document_snapshot snapshot;
        xui_doc_position_t at; xui_doc_rect_t caret; xui_draw_context draw;
        uint64_t paragraph, node;
        CHECK(xuiDocumentCreate(NULL, &document[0]) == XUI_OK &&
            xuiDocumentBeginTransaction(document[0], NULL, &transaction) == XUI_OK);
        paragraph = decoration_insert(transaction, 1, NULL, 0, 0);
        decoration_insert(transaction, paragraph, "\xf0\x9f\x91\xa9", XUI_DOC_BOLD, blue);
        decoration_insert(transaction, paragraph, "", XUI_DOC_UNDERLINE, blue);
        decoration_insert(transaction, paragraph, "\xe2\x80\x8d", XUI_DOC_UNDERLINE, blue);
        node = decoration_insert(transaction, paragraph, "\xf0\x9f\x92\xbb", XUI_DOC_STRIKE, red);
        CHECK(xuiDocumentTxnCommit(transaction, NULL) == XUI_OK); xuiDocumentTxnRelease(transaction);
        CHECK(xuiDocumentAcquireSnapshot(document[0], &snapshot) == XUI_OK &&
            xuiDocumentRendererCreate(context, &desc, &renderer[0]) == XUI_OK &&
            xuiDocumentRendererSetSnapshot(renderer[0], snapshot, NULL) == XUI_OK &&
            xuiDocumentRendererLayout(renderer[0], 300, 0, 100) == XUI_OK);
        xuiDocumentSnapshotRelease(snapshot);
        at = decoration_position(document[0], node, 4);
        CHECK(xuiDocumentRendererGetCaretRect(renderer[0], &at, &caret) == XUI_OK && fabs(caret.x - 19) < .01);
        decoration_line_count = decoration_span_count = 0;
        CHECK(proxy->tProxy.drawBegin(&proxy->tProxy, &draw, target) == XUI_OK &&
            xuiDocumentRendererDraw(renderer[0], draw, 0, 0, (xui_rect_t){0, 0, 300, 100}, NULL, 0) == XUI_OK &&
            decoration_line_count == 2 && decoration_span_count == 1 &&
            decoration_has(0, 19, 13.7f, 1.5f, blue) && decoration_has(0, 19, 6.7f, 2, red));
        {
            xui_rect_t saved = {2, 3, 40, 50}, restored; int active;
            CHECK(proxy->tProxy.drawClipSet(&proxy->tProxy, draw, saved) == XUI_OK);
            decoration_line_error = 1; decoration_line_count = 0;
            CHECK(xuiDocumentRendererDraw(renderer[0], draw, 0, 0,
                (xui_rect_t){0, 0, 300, 100}, NULL, 0) == XUI_ERROR_INVALID_STATE);
            decoration_line_error = 0;
            CHECK(proxy->tProxy.drawClipGet(&proxy->tProxy, draw, &restored, &active) == XUI_OK && active &&
                !memcmp(&saved, &restored, sizeof(saved)));
        }
        CHECK(proxy->tProxy.drawEnd(&proxy->tProxy, draw) == XUI_OK);
        xuiDocumentRendererRelease(renderer[0]); xuiDocumentRelease(document[0]);
    }
    {
        xui_document_transaction transaction; xui_document_snapshot snapshot;
        xui_draw_context draw; uint64_t paragraph;
        CHECK(xuiDocumentCreate(NULL, &document[0]) == XUI_OK &&
            xuiDocumentBeginTransaction(document[0], NULL, &transaction) == XUI_OK);
        paragraph = decoration_insert(transaction, 1, NULL, 0, 0);
        decoration_insert(transaction, paragraph, "AV", XUI_DOC_UNDERLINE | XUI_DOC_STRIKE, blue);
        CHECK(xuiDocumentTxnCommit(transaction, NULL) == XUI_OK); xuiDocumentTxnRelease(transaction);
        CHECK(xuiDocumentAcquireSnapshot(document[0], &snapshot) == XUI_OK &&
            xuiDocumentRendererCreate(context, &desc, &renderer[0]) == XUI_OK &&
            xuiDocumentRendererSetSnapshot(renderer[0], snapshot, NULL) == XUI_OK &&
            xuiDocumentRendererLayout(renderer[0], 300, 0, 100) == XUI_OK);
        xuiDocumentSnapshotRelease(snapshot);
        decoration_line_count = decoration_span_count = 0; decoration_text_flags = 0;
        decoration_text_count = decoration_underlined_text_count = 0;
        CHECK(proxy->tProxy.drawBegin(&proxy->tProxy, &draw, target) == XUI_OK &&
            xuiDocumentRendererDraw(renderer[0], draw, 0, 0, (xui_rect_t){0, 0, 300, 100}, NULL, 0) == XUI_OK &&
            proxy->tProxy.drawEnd(&proxy->tProxy, draw) == XUI_OK && decoration_span_count == 0 &&
            decoration_line_count == 2 && decoration_text_count == 1 && decoration_underlined_text_count == 0 &&
            !(decoration_text_flags & XUI_TEXT_UNDERLINE) &&
            decoration_has(0, 11, 13.7f, 1.5f, blue) && decoration_has(0, 11, 6.7f, 2, blue));
        xuiDocumentRendererRelease(renderer[0]); xuiDocumentRelease(document[0]);
    }
    xuiDestroy(context);
    proxy->tProxy.textShape = decoration_base_shape; proxy->tProxy.fontGetMetrics = decoration_base_metrics;
    proxy->tProxy.drawLine = decoration_base_line; proxy->tProxy.drawTextSpans = decoration_base_spans;
    proxy->tProxy.drawText = decoration_base_text;
    puts("Decorated joint shaping: wrapped marks, merged emoji, supplied/fallback metrics, revision and clip-error cleanup passed");
}
