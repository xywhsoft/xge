/* Shared native-DLL/headless checks for paragraph shaping around script spans. */
static xui_font vertical_fonts[2];
static int vertical_same_font;
static xui_font vertical_font(xui_context context, const char* family,
    uint32_t marks, float size, void* user)
{
    (void)context; (void)family; (void)size; (void)user;
    return vertical_fonts[!vertical_same_font &&
        !!(marks & (XUI_DOC_SUPERSCRIPT | XUI_DOC_SUBSCRIPT))];
}
static void vertical_compare(xui_document* document, xui_document_renderer* renderer,
    uint64_t nodes[2][8], double width, xui_doc_rect_t carets[2][4])
{
    unsigned variant, part; xui_doc_rect_t rects[2][32]; uint64_t count[2];
    for (variant = 0; variant < 2; variant++) {
        xui_doc_range_t range;
        CHECK(xuiDocumentRendererLayout(renderer[variant], width, 0, 400) == XUI_OK);
        for (part = 0; part < 4; part++) {
            xui_doc_position_t at = decoration_position(document[variant],
                nodes[variant][variant ? part * 2 + 1 : part],
                part == 3 ? (variant ? 1 : 2) : (variant ? 2 : 3));
            CHECK(xuiDocumentRendererGetCaretRect(renderer[variant], &at, &carets[variant][part]) == XUI_OK);
            if (width >= 120) {
                xui_doc_rect_t a; xui_doc_position_t hit;
                at = decoration_position(document[variant], nodes[variant][variant ? part * 2 : part], 1);
                CHECK(xuiDocumentRendererGetCaretRect(renderer[variant], &at, &a) == XUI_OK &&
                    xuiDocumentRendererHitTest(renderer[variant], a.x - .1,
                        a.y + a.height * .5, &hit) == XUI_OK &&
                    hit.iNodeId == at.iNodeId && hit.iOffset == at.iOffset);
            }
        }
        range.tAnchor = decoration_position(document[variant], nodes[variant][0], 0);
        range.tCaret = decoration_position(document[variant], nodes[variant][variant ? 7 : 3], variant ? 1 : 2);
        CHECK(xuiDocumentRendererGetRangeRects(renderer[variant], &range, rects[variant], 32, &count[variant]) == XUI_OK &&
            count[variant] <= 32);
    }
    for (part = 0; part < 4; part++) CHECK(
        fabs(carets[0][part].x - carets[1][part].x) < .01 &&
        fabs(carets[0][part].y - carets[1][part].y) < .01 &&
        fabs(carets[0][part].height - carets[1][part].height) < .01);
    CHECK(count[0] == count[1]);
    for (part = 0; part < count[0]; part++) CHECK(
        fabs(rects[0][part].x - rects[1][part].x) < .01 &&
        fabs(rects[0][part].y - rects[1][part].y) < .01 &&
        fabs(rects[0][part].width - rects[1][part].width) < .01 &&
        fabs(rects[0][part].height - rects[1][part].height) < .01);
}
static void document_vertical_shaping(xui_surface target, xui_test_proxy_state_t* proxy)
{
    xui_context context; xui_doc_renderer_desc_t desc = {0}; unsigned mode, variant, pass, part;
    const uint32_t blue = XUI_COLOR_RGBA(20, 40, 180, 255), red = XUI_COLOR_RGBA(180, 20, 40, 255);
    static const char* const joined[] = {"AV ", "AV ", "AV ", "AV"};
    static const char* const split[] = {"A", "V ", "A", "V ", "A", "V ", "A", "V"};
    static const uint32_t marks[] = {XUI_DOC_UNDERLINE,
        XUI_DOC_UNDERLINE | XUI_DOC_STRIKE | XUI_DOC_SUPERSCRIPT,
        XUI_DOC_UNDERLINE | XUI_DOC_STRIKE | XUI_DOC_SUBSCRIPT, XUI_DOC_UNDERLINE};
    CHECK(proxy->tProxy.fontLoadFile(&proxy->tProxy, &vertical_fonts[0], "normal.ttf", 20, 0) == XUI_OK &&
        proxy->tProxy.fontLoadFile(&proxy->tProxy, &vertical_fonts[1], "small.ttf", 15, 0) == XUI_OK);
    decoration_base_shape = proxy->tProxy.textShape; decoration_base_metrics = proxy->tProxy.fontGetMetrics;
    decoration_base_line = proxy->tProxy.drawLine; decoration_base_spans = proxy->tProxy.drawTextSpans;
    proxy->tProxy.textShape = mixed_font_shape; proxy->tProxy.fontGetMetrics = decoration_metrics;
    proxy->tProxy.drawLine = decoration_line; proxy->tProxy.drawTextSpans = decoration_spans;
    mixed_failure_font = vertical_fonts[1]; mixed_failure_at = mixed_shape_calls = 0;
    decoration_invalid_metrics = decoration_metrics_error = decoration_line_error = 0;
    CHECK(xuiCreate(&context) == XUI_OK && xuiSetProxy(context, &proxy->tProxy) == XUI_OK &&
        xuiSetDefaultFont(context, vertical_fonts[0]) == XUI_OK);
    desc.iSize = sizeof(desc); desc.tFonts = (xui_doc_font_set_t){vertical_fonts[0], vertical_fonts[0],
        vertical_fonts[0], vertical_fonts[0], vertical_fonts[0]};
    desc.onFont = vertical_font; desc.fLineGap = 4;
    for (mode = 0; mode < 2; mode++) {
        xui_document document[2]; xui_document_renderer renderer[2]; uint64_t nodes[2][8] = {{0}};
        xui_doc_rect_t carets[2][4];
        vertical_same_font = (int)mode;
        for (variant = 0; variant < 2; variant++) {
            xui_document_transaction transaction; xui_document_snapshot snapshot; uint64_t paragraph;
            CHECK(xuiDocumentCreate(NULL, &document[variant]) == XUI_OK &&
                xuiDocumentBeginTransaction(document[variant], NULL, &transaction) == XUI_OK);
            paragraph = decoration_insert(transaction, 1, NULL, 0, 0);
            for (part = 0; part < (variant ? 8 : 4); part++)
                nodes[variant][part] = decoration_insert(transaction, paragraph,
                    variant ? split[part] : joined[part], marks[variant ? part / 2 : part],
                    variant && (part & 1) ? red : blue);
            CHECK(xuiDocumentTxnCommit(transaction, NULL) == XUI_OK); xuiDocumentTxnRelease(transaction);
            CHECK(xuiDocumentAcquireSnapshot(document[variant], &snapshot) == XUI_OK &&
                xuiDocumentRendererCreate(context, &desc, &renderer[variant]) == XUI_OK &&
                xuiDocumentRendererSetSnapshot(renderer[variant], snapshot, NULL) == XUI_OK);
            xuiDocumentSnapshotRelease(snapshot);
        }
        for (pass = 0; pass < 4; pass++) {
            double width = pass == 1 ? 33 : pass == 2 ? 12 : 120;
            vertical_compare(document, renderer, nodes, width, carets);
            if (width == 120) CHECK(fabs(carets[1][0].x - 27) < .01 &&
                fabs(carets[1][1].x - (mode ? 54 : 46.5)) < .01 &&
                fabs(carets[1][2].x - (mode ? 81 : 66)) < .01 &&
                fabs(carets[1][3].x - (mode ? 98 : 83)) < .01 &&
                fabs(carets[1][0].y - (mode ? 6 : .5)) < .01 &&
                fabs(carets[1][1].y) < .01 &&
                fabs(carets[1][2].y - (mode ? 12 : 9)) < .01 &&
                fabs(carets[1][1].height - (mode ? 20 : 15)) < .01);
        }
        {
            xui_draw_context draw;
            float super_mid = mode ? 34 : 32, super_end = mode ? 54 : 47;
            float sub_mid = mode ? 61 : 51, sub_end = mode ? 81 : 66;
            float super_base = mode ? 16 : 12, sub_base = mode ? 28 : 21, normal_base = mode ? 22 : 17;
            decoration_span_count = decoration_line_count = 0;
            CHECK(proxy->tProxy.drawBegin(&proxy->tProxy, &draw, target) == XUI_OK &&
                xuiDocumentRendererDraw(renderer[1], draw, 0, 0, (xui_rect_t){0, 0, 120, 100}, NULL, 0) == XUI_OK &&
                proxy->tProxy.drawEnd(&proxy->tProxy, draw) == XUI_OK &&
                decoration_span_count == 4 && decoration_line_count == 12);
            CHECK(decoration_has(0, 7, normal_base + 2.5f, 1.5f, blue) &&
                decoration_has(7, 27, normal_base + 2.5f, 1.5f, red) &&
                decoration_has(27, super_mid, super_base + 2.5f, 1.5f, blue) &&
                decoration_has(super_mid, super_end, super_base + 2.5f, 1.5f, red) &&
                decoration_has(27, super_mid, super_base - 4.5f, 2, blue) &&
                decoration_has(super_mid, super_end, super_base - 4.5f, 2, red) &&
                decoration_has(super_end, sub_mid, sub_base + 2.5f, 1.5f, blue) &&
                decoration_has(sub_mid, sub_end, sub_base + 2.5f, 1.5f, red) &&
                decoration_has(super_end, sub_mid, sub_base - 4.5f, 2, blue) &&
                decoration_has(sub_mid, sub_end, sub_base - 4.5f, 2, red) &&
                decoration_has(sub_end, sub_end + 7, normal_base + 2.5f, 1.5f, blue) &&
                decoration_has(sub_end + 7, sub_end + 17, normal_base + 2.5f, 1.5f, red));
        }
        for (pass = 0; pass < 4; pass++) {
            for (variant = 0; variant < 2; variant++) {
                xui_document_change_set changes; xui_document_snapshot snapshot;
                if (!(pass & 1)) {
                    xui_document_transaction transaction; xui_doc_range_t range;
                    unsigned zone = pass ? 2 : 1;
                    range.tAnchor = decoration_position(document[variant], nodes[variant][variant ? zone * 2 : zone], 0);
                    range.tCaret = decoration_position(document[variant], nodes[variant][variant ? zone * 2 + 1 : zone], variant ? 2 : 3);
                    CHECK(xuiDocumentBeginTransaction(document[variant], NULL, &transaction) == XUI_OK &&
                        xuiDocumentTxnSetMarks(transaction, &range, pass ? XUI_DOC_SUPERSCRIPT : 0,
                            pass ? XUI_DOC_SUBSCRIPT : XUI_DOC_SUPERSCRIPT) == XUI_OK &&
                        xuiDocumentTxnCommit(transaction, &changes) == XUI_OK);
                    xuiDocumentTxnRelease(transaction);
                } else CHECK(xuiDocumentUndo(document[variant], &changes) == XUI_OK);
                CHECK(xuiDocumentAcquireSnapshot(document[variant], &snapshot) == XUI_OK &&
                    xuiDocumentRendererSetSnapshot(renderer[variant], snapshot, changes) == XUI_OK);
                xuiDocumentSnapshotRelease(snapshot); xuiDocumentChangeSetRelease(changes);
            }
            vertical_compare(document, renderer, nodes, 120, carets);
            CHECK(fabs(carets[1][3].x - (mode ? 98 : pass ? 83 : 90.5)) < .01 &&
                fabs(carets[1][1].y - (pass ? 0 : carets[1][0].y)) < .01);
            if (pass == 2) CHECK(fabs(carets[1][2].y) < .01);
        }
        if (!mode) for (pass = 0; pass < 2; pass++) {
            xui_document_renderer retry; xui_document_snapshot snapshot;
            xui_doc_position_t at; xui_doc_rect_t caret;
            CHECK(xuiDocumentAcquireSnapshot(document[1], &snapshot) == XUI_OK &&
                xuiDocumentRendererCreate(context, &desc, &retry) == XUI_OK &&
                xuiDocumentRendererSetSnapshot(retry, snapshot, NULL) == XUI_OK);
            xuiDocumentSnapshotRelease(snapshot);
            mixed_shape_calls = 0; mixed_failure_at = pass ? 3 : 1;
            CHECK(xuiDocumentRendererLayout(retry, 120, 0, 400) == XUI_ERROR_INVALID_STATE &&
                mixed_shape_calls == mixed_failure_at);
            mixed_failure_at = mixed_shape_calls = 0;
            CHECK(xuiDocumentRendererLayout(retry, 120, 0, 400) == XUI_OK);
            at = decoration_position(document[1], nodes[1][7], 1);
            CHECK(xuiDocumentRendererGetCaretRect(retry, &at, &caret) == XUI_OK && fabs(caret.x - 83) < .01);
            xuiDocumentRendererRelease(retry);
        }
        for (variant = 0; variant < 2; variant++) {
            xuiDocumentRendererRelease(renderer[variant]); xuiDocumentRelease(document[variant]);
        }
    }
    /* Empty script nodes preserve their caret shift without splitting AV. */
    vertical_same_font = 1;
    for (pass = 0; pass < 4; pass++) {
        xui_document document; xui_document_transaction transaction; xui_document_snapshot snapshot;
        xui_document_renderer renderer; xui_doc_position_t at; xui_doc_rect_t caret; xui_draw_context draw;
        uint64_t paragraph, empty, last; int super = pass >= 2;
        CHECK(xuiDocumentCreate(NULL, &document) == XUI_OK &&
            xuiDocumentBeginTransaction(document, NULL, &transaction) == XUI_OK);
        paragraph = decoration_insert(transaction, 1, NULL, 0, 0);
        if (pass & 1) decoration_insert(transaction, paragraph, "A", XUI_DOC_UNDERLINE, blue);
        empty = decoration_insert(transaction, paragraph, "", XUI_DOC_UNDERLINE |
            (super ? XUI_DOC_SUPERSCRIPT : XUI_DOC_SUBSCRIPT), blue);
        if (!(pass & 1)) decoration_insert(transaction, paragraph, "A", XUI_DOC_UNDERLINE, blue);
        last = decoration_insert(transaction, paragraph, "V", XUI_DOC_UNDERLINE, red);
        CHECK(xuiDocumentTxnCommit(transaction, NULL) == XUI_OK); xuiDocumentTxnRelease(transaction);
        CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK &&
            xuiDocumentRendererCreate(context, &desc, &renderer) == XUI_OK &&
            xuiDocumentRendererSetSnapshot(renderer, snapshot, NULL) == XUI_OK &&
            xuiDocumentRendererLayout(renderer, 120, 0, 400) == XUI_OK);
        xuiDocumentSnapshotRelease(snapshot);
        at = decoration_position(document, last, 1);
        CHECK(xuiDocumentRendererGetCaretRect(renderer, &at, &caret) == XUI_OK &&
            fabs(caret.x - 17) < .01 && fabs(caret.y - (super ? 6 : 0)) < .01);
        at = decoration_position(document, empty, 0);
        CHECK(xuiDocumentRendererGetCaretRect(renderer, &at, &caret) == XUI_OK &&
            fabs(caret.height - 20) < .01 && fabs(caret.y - (super ? 0 : 6)) < .01);
        decoration_span_count = decoration_line_count = 0;
        CHECK(proxy->tProxy.drawBegin(&proxy->tProxy, &draw, target) == XUI_OK &&
            xuiDocumentRendererDraw(renderer, draw, 0, 0, (xui_rect_t){0, 0, 120, 100}, NULL, 0) == XUI_OK &&
            proxy->tProxy.drawEnd(&proxy->tProxy, draw) == XUI_OK &&
            decoration_span_count == 1 && decoration_line_count == 2 &&
            decoration_has(0, 7, super ? 24.5f : 18.5f, 1.5f, blue) &&
            decoration_has(7, 17, super ? 24.5f : 18.5f, 1.5f, red));
        xuiDocumentRendererRelease(renderer); xuiDocumentRelease(document);
    }
    xuiDestroy(context); proxy->tProxy.textShape = decoration_base_shape;
    proxy->tProxy.fontGetMetrics = decoration_base_metrics; proxy->tProxy.drawLine = decoration_base_line;
    proxy->tProxy.drawTextSpans = decoration_base_spans;
    proxy->tProxy.fontDestroy(&proxy->tProxy, vertical_fonts[0]);
    proxy->tProxy.fontDestroy(&proxy->tProxy, vertical_fonts[1]);
    puts("Script span joint shaping: scaled/same-font baselines, colors, wraps, ranges, decorations, empty scripts, mutation/undo and failures passed");
}
