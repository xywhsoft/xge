/* Included after xui_document_decoration_renderer_cases.h to share its
 * deterministic AV kerning and decoration callback probes. */
static xui_font mixed_failure_font;
static int mixed_failure_at, mixed_shape_calls;
static int mixed_font_shape(xui_proxy proxy, const xui_text_item_t* pTextItem, xui_text_shape_t* shape)
{
    xui_font font = pTextItem && pTextItem->iSize >= sizeof(*pTextItem) ? pTextItem->pFont : NULL;
    const char* text = pTextItem && pTextItem->iSize >= sizeof(*pTextItem) ? pTextItem->sText : NULL;
    int bytes = pTextItem && pTextItem->iSize >= sizeof(*pTextItem) ? pTextItem->iTextSize : 0;

    if (font == mixed_failure_font && bytes == 3 && !memcmp(text, "AV ", 3) &&
        ++mixed_shape_calls == mixed_failure_at) return XUI_ERROR_INVALID_STATE;
    return decoration_shape(proxy, pTextItem, shape);
}
static void document_mixed_font_shaping(xui_surface target, xui_test_proxy_state_t* proxy)
{
    xui_context context; xui_font normal, large;
    xui_document document[2]; xui_document_renderer renderer[2]; uint64_t nodes[2][6] = {{0}};
    xui_doc_renderer_desc_t desc = {0}; unsigned variant, pass, part;
    const uint32_t blue = XUI_COLOR_RGBA(20, 40, 180, 255), red = XUI_COLOR_RGBA(180, 20, 40, 255);
    static const char* const joined[] = {"AV ", "AV ", "AV"};
    static const char* const split[] = {"A", "V ", "A", "V ", "A", "V"};
    CHECK(proxy->tProxy.fontLoadFile(&proxy->tProxy, &normal, "normal.ttf", 20, 0) == XUI_OK &&
        proxy->tProxy.fontLoadFile(&proxy->tProxy, &large, "large.ttf", 40, 0) == XUI_OK);
    decoration_base_shape = proxy->tProxy.textShape; decoration_base_metrics = proxy->tProxy.fontGetMetrics;
    decoration_base_line = proxy->tProxy.drawLine; decoration_base_spans = proxy->tProxy.drawTextSpans;
    decoration_base_text = proxy->tProxy.drawText;
    mixed_failure_font = large; mixed_failure_at = mixed_shape_calls = 0;
    proxy->tProxy.textShape = mixed_font_shape; proxy->tProxy.fontGetMetrics = decoration_metrics;
    proxy->tProxy.drawLine = decoration_line; proxy->tProxy.drawTextSpans = decoration_spans;
    decoration_invalid_metrics = decoration_metrics_error = decoration_line_error = 0;
    CHECK(xuiCreate(&context) == XUI_OK && xuiSetProxy(context, &proxy->tProxy) == XUI_OK &&
        xuiSetDefaultFont(context, normal) == XUI_OK);
    desc.iSize = sizeof(desc); desc.tFonts = (xui_doc_font_set_t){normal, large, normal, large, normal};
    desc.fLineGap = 4;
    for (variant = 0; variant < 2; variant++) {
        xui_document_transaction transaction; xui_document_snapshot snapshot; uint64_t paragraph;
        CHECK(xuiDocumentCreate(NULL, &document[variant]) == XUI_OK &&
            xuiDocumentBeginTransaction(document[variant], NULL, &transaction) == XUI_OK);
        paragraph = decoration_insert(transaction, 1, NULL, 0, 0);
        for (part = 0; part < (variant ? 6 : 3); part++) {
            unsigned zone = variant ? part / 2 : part;
            nodes[variant][part] = decoration_insert(transaction, paragraph,
                variant ? split[part] : joined[part], XUI_DOC_UNDERLINE |
                (zone == 1 ? XUI_DOC_BOLD | XUI_DOC_STRIKE : 0), variant && (part & 1) ? red : blue);
        }
        CHECK(xuiDocumentTxnCommit(transaction, NULL) == XUI_OK); xuiDocumentTxnRelease(transaction);
        CHECK(xuiDocumentAcquireSnapshot(document[variant], &snapshot) == XUI_OK &&
            xuiDocumentRendererCreate(context, &desc, &renderer[variant]) == XUI_OK &&
            xuiDocumentRendererSetSnapshot(renderer[variant], snapshot, NULL) == XUI_OK);
        xuiDocumentSnapshotRelease(snapshot);
    }
    for (pass = 0; pass < 4; pass++) {
        double width = pass == 1 ? 45 : pass == 2 ? 12 : 120;
        xui_doc_rect_t carets[2][3], rects[2][32]; uint64_t count[2];
        for (variant = 0; variant < 2; variant++) {
            xui_doc_range_t range;
            CHECK(xuiDocumentRendererLayout(renderer[variant], width, 0, 400) == XUI_OK);
            for (part = 0; part < 3; part++) {
                xui_doc_position_t at = decoration_position(document[variant],
                    nodes[variant][variant ? part * 2 + 1 : part], part == 2 ? (variant ? 1 : 2) : (variant ? 2 : 3));
                CHECK(xuiDocumentRendererGetCaretRect(renderer[variant], &at, &carets[variant][part]) == XUI_OK);
            }
            range.tAnchor = decoration_position(document[variant], nodes[variant][0], 0);
            range.tCaret = decoration_position(document[variant], nodes[variant][variant ? 5 : 2], variant ? 1 : 2);
            CHECK(xuiDocumentRendererGetRangeRects(renderer[variant], &range, rects[variant], 32, &count[variant]) == XUI_OK &&
                count[variant] <= 32);
        }
        for (part = 0; part < 3; part++) CHECK(
            fabs(carets[0][part].x - carets[1][part].x) < .01 &&
            fabs(carets[0][part].y - carets[1][part].y) < .01 &&
            fabs(carets[0][part].height - carets[1][part].height) < .01);
        CHECK(count[0] == count[1]);
        for (part = 0; part < count[0]; part++) CHECK(
            fabs(rects[0][part].x - rects[1][part].x) < .01 &&
            fabs(rects[0][part].y - rects[1][part].y) < .01 &&
            fabs(rects[0][part].width - rects[1][part].width) < .01 &&
            fabs(rects[0][part].height - rects[1][part].height) < .01);
        if (width == 120) CHECK(fabs(carets[1][0].x - 27) < .01 && fabs(carets[1][1].x - 84) < .01 &&
            fabs(carets[1][2].x - 101) < .01 && fabs(carets[1][0].y - 16) < .01 &&
            fabs(carets[1][1].y) < .01 && fabs(carets[1][2].y - 16) < .01);
    }
    {
        xui_draw_context draw;
        decoration_span_count = decoration_line_count = 0;
        CHECK(proxy->tProxy.drawBegin(&proxy->tProxy, &draw, target) == XUI_OK &&
            xuiDocumentRendererDraw(renderer[1], draw, 0, 0, (xui_rect_t){0, 0, 120, 100}, NULL, 0) == XUI_OK &&
            proxy->tProxy.drawEnd(&proxy->tProxy, draw) == XUI_OK && decoration_span_count == 3 &&
            decoration_line_count == 8 && decoration_has(0, 7, 34.5f, 1.5f, blue) &&
            decoration_has(7, 27, 34.5f, 1.5f, red) && decoration_has(27, 44, 34.5f, 1.5f, blue) &&
            decoration_has(44, 84, 34.5f, 1.5f, red) && decoration_has(27, 44, 27.5f, 2, blue) &&
            decoration_has(44, 84, 27.5f, 2, red) && decoration_has(84, 91, 34.5f, 1.5f, blue) &&
            decoration_has(91, 101, 34.5f, 1.5f, red));
    }
    for (pass = 0; pass < 2; pass++) for (variant = 0; variant < 2; variant++) {
        xui_document_change_set changes; xui_document_snapshot snapshot; xui_doc_position_t at;
        xui_doc_rect_t caret;
        if (!pass) {
            xui_document_transaction transaction; xui_doc_range_t range;
            range.tAnchor = decoration_position(document[variant], nodes[variant][variant ? 2 : 1], 0);
            range.tCaret = decoration_position(document[variant], nodes[variant][variant ? 3 : 1], variant ? 2 : 3);
            CHECK(xuiDocumentBeginTransaction(document[variant], NULL, &transaction) == XUI_OK &&
                xuiDocumentTxnSetMarks(transaction, &range, 0, XUI_DOC_BOLD) == XUI_OK &&
                xuiDocumentTxnCommit(transaction, &changes) == XUI_OK);
            xuiDocumentTxnRelease(transaction);
        } else CHECK(xuiDocumentUndo(document[variant], &changes) == XUI_OK);
        CHECK(xuiDocumentAcquireSnapshot(document[variant], &snapshot) == XUI_OK &&
            xuiDocumentRendererSetSnapshot(renderer[variant], snapshot, changes) == XUI_OK &&
            xuiDocumentRendererLayout(renderer[variant], 120, 0, 400) == XUI_OK);
        xuiDocumentSnapshotRelease(snapshot); xuiDocumentChangeSetRelease(changes);
        at = decoration_position(document[variant], nodes[variant][variant ? 5 : 2], variant ? 1 : 2);
        CHECK(xuiDocumentRendererGetCaretRect(renderer[variant], &at, &caret) == XUI_OK &&
            fabs(caret.x - (pass ? 101 : 71)) < .01 && fabs(caret.height - 20) < .01);
    }
    for (pass = 1; pass <= 2; pass++) {
        xui_document_renderer retry; xui_document_snapshot snapshot; xui_doc_position_t at;
        xui_doc_rect_t caret;
        CHECK(xuiDocumentAcquireSnapshot(document[1], &snapshot) == XUI_OK &&
            xuiDocumentRendererCreate(context, &desc, &retry) == XUI_OK &&
            xuiDocumentRendererSetSnapshot(retry, snapshot, NULL) == XUI_OK);
        xuiDocumentSnapshotRelease(snapshot);
        mixed_shape_calls = 0; mixed_failure_at = (int)pass;
        CHECK(xuiDocumentRendererLayout(retry, 120, 0, 400) == XUI_ERROR_INVALID_STATE &&
            mixed_shape_calls == (int)pass);
        mixed_failure_at = mixed_shape_calls = 0;
        CHECK(xuiDocumentRendererLayout(retry, 120, 0, 400) == XUI_OK);
        at = decoration_position(document[1], nodes[1][5], 1);
        CHECK(xuiDocumentRendererGetCaretRect(retry, &at, &caret) == XUI_OK && fabs(caret.x - 101) < .01);
        xuiDocumentRendererRelease(retry);
    }
    for (pass = 0; pass < 2; pass++) {
        xui_document empty_document; xui_document_transaction transaction; xui_document_snapshot snapshot;
        xui_document_renderer empty_renderer; xui_doc_position_t at; xui_doc_rect_t caret;
        xui_draw_context draw; uint64_t paragraph, empty, last;
        CHECK(xuiDocumentCreate(NULL, &empty_document) == XUI_OK &&
            xuiDocumentBeginTransaction(empty_document, NULL, &transaction) == XUI_OK);
        paragraph = decoration_insert(transaction, 1, NULL, 0, 0);
        if (pass) decoration_insert(transaction, paragraph, "A", XUI_DOC_UNDERLINE, blue);
        empty = decoration_insert(transaction, paragraph, "", XUI_DOC_BOLD | XUI_DOC_UNDERLINE, blue);
        if (!pass) decoration_insert(transaction, paragraph, "A", XUI_DOC_UNDERLINE, blue);
        last = decoration_insert(transaction, paragraph, "V", XUI_DOC_UNDERLINE, red);
        CHECK(xuiDocumentTxnCommit(transaction, NULL) == XUI_OK); xuiDocumentTxnRelease(transaction);
        CHECK(xuiDocumentAcquireSnapshot(empty_document, &snapshot) == XUI_OK &&
            xuiDocumentRendererCreate(context, &desc, &empty_renderer) == XUI_OK &&
            xuiDocumentRendererSetSnapshot(empty_renderer, snapshot, NULL) == XUI_OK &&
            xuiDocumentRendererLayout(empty_renderer, 120, 0, 400) == XUI_OK);
        xuiDocumentSnapshotRelease(snapshot);
        at = decoration_position(empty_document, last, 1);
        CHECK(xuiDocumentRendererGetCaretRect(empty_renderer, &at, &caret) == XUI_OK);
        CHECK(
            fabs(caret.x - 17) < .01 && fabs(caret.y - 16) < .01);
        at = decoration_position(empty_document, empty, 0);
        CHECK(xuiDocumentRendererGetCaretRect(empty_renderer, &at, &caret) == XUI_OK && fabs(caret.height - 40) < .01);
        decoration_span_count = decoration_line_count = 0;
        CHECK(proxy->tProxy.drawBegin(&proxy->tProxy, &draw, target) == XUI_OK &&
            xuiDocumentRendererDraw(empty_renderer, draw, 0, 0, (xui_rect_t){0, 0, 120, 100}, NULL, 0) == XUI_OK &&
            proxy->tProxy.drawEnd(&proxy->tProxy, draw) == XUI_OK && decoration_span_count == 1 &&
            decoration_line_count == 2 && decoration_has(0, 7, 34.5f, 1.5f, blue) &&
            decoration_has(7, 17, 34.5f, 1.5f, red));
        xuiDocumentRendererRelease(empty_renderer); xuiDocumentRelease(empty_document);
    }
    for (variant = 0; variant < 2; variant++) {
        xuiDocumentRendererRelease(renderer[variant]); xuiDocumentRelease(document[variant]);
    }
    xuiDestroy(context); proxy->tProxy.textShape = decoration_base_shape;
    proxy->tProxy.fontGetMetrics = decoration_base_metrics; proxy->tProxy.drawLine = decoration_base_line;
    proxy->tProxy.drawTextSpans = decoration_base_spans;
    proxy->tProxy.fontDestroy(&proxy->tProxy, normal); proxy->tProxy.fontDestroy(&proxy->tProxy, large);
    puts("Mixed font joint shaping: color boundaries, four widths, shared baseline, ranges, decorations, font mutation/undo and seed/line failures passed");
}
