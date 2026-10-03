/* Continuous text shaping must survive object and physical-line boundaries. */
typedef struct object_shape_probe {
    xui_font fonts[2]; float width, height, baseline;
    unsigned draws; xui_rect_t bounds;
} object_shape_probe;
static xui_font object_shape_font(xui_context context, const char* family,
    uint32_t marks, float size, void* user)
{
    object_shape_probe* probe = user;
    (void)context; (void)family; (void)size;
    return probe->fonts[!!(marks & XUI_DOC_SUPERSCRIPT)];
}
static int object_shape_measure(xui_document_snapshot snapshot, uint64_t node,
    float width, float zoom, xui_vec2_t* size, float* baseline, void* user)
{
    object_shape_probe* probe = user; xui_doc_node_info_t info = {0};
    (void)width; info.iSize = sizeof(info);
    CHECK(xuiDocumentSnapshotGetNode(snapshot, node, &info) == XUI_OK &&
        (info.iKind == XUI_DOC_IMAGE || info.iKind == XUI_DOC_MATH));
    *size = (xui_vec2_t){probe->width * zoom, probe->height * zoom};
    *baseline = probe->baseline * zoom; return XUI_OK;
}
static int object_shape_draw(xui_document_snapshot snapshot, uint64_t node,
    xui_proxy proxy, xui_draw_context draw, xui_rect_t rect, void* user)
{
    object_shape_probe* probe = user; (void)snapshot; (void)node;
    probe->draws++; probe->bounds = rect;
    return proxy->drawRectFill(proxy, draw, rect, XUI_COLOR_RGBA(160, 40, 180, 255));
}
static int object_shape_failure_at, object_shape_calls;
static xui_font object_shape_failure_font;
static int object_shape_shape(xui_proxy proxy, const xui_text_item_t* pTextItem, xui_text_shape_t* shape)
{
    xui_font font = pTextItem && pTextItem->iSize >= sizeof(*pTextItem) ? pTextItem->pFont : NULL;
    const char* text = pTextItem && pTextItem->iSize >= sizeof(*pTextItem) ? pTextItem->sText : NULL;
    int bytes = pTextItem && pTextItem->iSize >= sizeof(*pTextItem) ? pTextItem->iTextSize : 0;

    if (font == object_shape_failure_font && bytes == 2 && !memcmp(text, "AV", 2) &&
        ++object_shape_calls == object_shape_failure_at) return XUI_ERROR_INVALID_STATE;
    return decoration_shape(proxy, pTextItem, shape);
}
static int object_shape_spans(xui_proxy proxy, xui_draw_context draw, const xui_text_item_t* pTextItem, xui_rect_t rect, uint32_t color, uint32_t flags, const xui_text_paint_span_t* spans, int count)
{
    const char* text = pTextItem && pTextItem->iSize >= sizeof(*pTextItem) ? pTextItem->sText : NULL;
    int bytes = pTextItem && pTextItem->iSize >= sizeof(*pTextItem) ? pTextItem->iTextSize : 0;

    CHECK(!memchr(text, '\r', (size_t)bytes) && !memchr(text, '\n', (size_t)bytes));
    return decoration_spans(proxy, draw, pTextItem, rect, color, flags, spans, count);
}
static int object_shape_text(xui_proxy proxy, xui_draw_context draw, const xui_text_item_t* pTextItem, xui_rect_t rect, uint32_t color, uint32_t flags)
{
    const char* text = pTextItem && pTextItem->iSize >= sizeof(*pTextItem) ? pTextItem->sText : NULL;

    CHECK(!strchr(text, '\r') && !strchr(text, '\n'));
    return decoration_base_text(proxy, draw, pTextItem, rect, color, flags);
}
static void object_shape_compare(xui_document* document, xui_document_renderer* renderer,
    uint64_t endpoints[2][3], double width, xui_doc_rect_t carets[2][2])
{
    xui_doc_rect_t rects[2][64], sizes[2]; uint64_t counts[2]; unsigned variant, part; int exact;
    for (variant = 0; variant < 2; variant++) {
        xui_doc_range_t range;
        CHECK(xuiDocumentRendererLayout(renderer[variant], width, 0, 400) == XUI_OK &&
            xuiDocumentRendererGetSize(renderer[variant], &sizes[variant], &exact) == XUI_OK && exact);
        range.tAnchor = decoration_position(document[variant], endpoints[variant][0], 0);
        for (part = 0; part < 2; part++) {
            xui_doc_position_t at = decoration_position(document[variant], endpoints[variant][part + 1],
                part ? (variant ? 1 : 2) : (variant ? 2 : 3));
            CHECK(xuiDocumentRendererGetCaretRect(renderer[variant], &at, &carets[variant][part]) == XUI_OK);
            if (part) range.tCaret = at;
        }
        CHECK(xuiDocumentRendererGetRangeRects(renderer[variant], &range, rects[variant], 64, &counts[variant]) == XUI_OK &&
            counts[variant] <= 64);
    }
    CHECK(fabs(sizes[0].width - sizes[1].width) < .01 && fabs(sizes[0].height - sizes[1].height) < .01);
    for (part = 0; part < 2; part++) CHECK(fabs(carets[0][part].x - carets[1][part].x) < .01 &&
        fabs(carets[0][part].y - carets[1][part].y) < .01 && fabs(carets[0][part].height - carets[1][part].height) < .01);
    CHECK(counts[0] == counts[1]);
    for (part = 0; part < counts[0]; part++) CHECK(fabs(rects[0][part].x - rects[1][part].x) < .01 &&
        fabs(rects[0][part].y - rects[1][part].y) < .01 && fabs(rects[0][part].width - rects[1][part].width) < .01 &&
        fabs(rects[0][part].height - rects[1][part].height) < .01);
}
static void document_object_joint_shaping(xui_surface target, xui_test_proxy_state_t* proxy)
{
    object_shape_probe probe = {0}; xui_context context; xui_doc_renderer_desc_t desc = {0};
    unsigned sample, variant, pass;
    const uint32_t blue = XUI_COLOR_RGBA(20, 40, 180, 255), red = XUI_COLOR_RGBA(180, 20, 40, 255);
    CHECK(proxy->tProxy.fontLoadFile(&proxy->tProxy, &probe.fonts[0], "normal.ttf", 20, 0) == XUI_OK &&
        proxy->tProxy.fontLoadFile(&proxy->tProxy, &probe.fonts[1], "small.ttf", 15, 0) == XUI_OK);
    decoration_base_shape = proxy->tProxy.textShape; decoration_base_metrics = proxy->tProxy.fontGetMetrics;
    decoration_base_line = proxy->tProxy.drawLine; decoration_base_spans = proxy->tProxy.drawTextSpans;
    decoration_base_text = proxy->tProxy.drawText;
    proxy->tProxy.textShape = object_shape_shape; proxy->tProxy.fontGetMetrics = decoration_metrics;
    proxy->tProxy.drawLine = decoration_line; proxy->tProxy.drawTextSpans = object_shape_spans;
    proxy->tProxy.drawText = object_shape_text;
    decoration_invalid_metrics = decoration_metrics_error = decoration_line_error = 0;
    object_shape_failure_font = probe.fonts[1]; object_shape_calls = object_shape_failure_at = 0;
    CHECK(xuiCreate(&context) == XUI_OK && xuiSetProxy(context, &proxy->tProxy) == XUI_OK &&
        xuiSetDefaultFont(context, probe.fonts[0]) == XUI_OK);
    desc.iSize = sizeof(desc); desc.onFont = object_shape_font; desc.pUser = &probe;
    desc.onObjectMeasure = object_shape_measure; desc.onObjectDraw = object_shape_draw; desc.fLineGap = 4;
    for (sample = 0; sample < 7; sample++) {
        xui_document document[2]; xui_document_renderer renderer[2]; uint64_t endpoints[2][3], separators[2] = {0};
        xui_doc_rect_t carets[2][2];
        probe.width = 13; probe.height = 36; probe.baseline = 24;
        for (variant = 0; variant < 2; variant++) {
            xui_document_transaction transaction; xui_document_snapshot snapshot; uint64_t paragraph;
            const char* prefix = sample == 4 ? "AV \n" : sample >= 5 ? "AV \r\n" : "AV ";
            const char* split_prefix = sample == 4 ? "V \n" : sample == 5 ? "V \r" : sample == 6 ? "V \r\n" : "V ";
            CHECK(xuiDocumentCreate(NULL, &document[variant]) == XUI_OK &&
                xuiDocumentBeginTransaction(document[variant], NULL, &transaction) == XUI_OK);
            paragraph = decoration_insert(transaction, 1, NULL, 0, 0);
            endpoints[variant][0] = decoration_insert(transaction, paragraph, variant ? "A" : prefix, XUI_DOC_UNDERLINE, blue);
            endpoints[variant][1] = variant ? decoration_insert(transaction, paragraph, split_prefix, XUI_DOC_UNDERLINE, red) :
                endpoints[variant][0];
            if (sample == 5 && variant) decoration_insert(transaction, paragraph, "\n", XUI_DOC_UNDERLINE, red);
            if (sample < 4) {
                xui_doc_node_desc_t node = {0};
                node.iSize = sizeof(node); node.iKind = sample == 0 ? XUI_DOC_IMAGE : sample == 1 ? XUI_DOC_MATH :
                    sample == 2 ? XUI_DOC_SOFT_BREAK : XUI_DOC_HARD_BREAK;
                if (sample < 2) { node.sText = sample ? "x" : ""; node.iTextBytes = strlen(node.sText); }
                CHECK(xuiDocumentTxnInsertNode(transaction, paragraph, XUI_DOCUMENT_APPEND, &node, &separators[variant]) == XUI_OK);
            }
            if (variant) decoration_insert(transaction, paragraph, "A", XUI_DOC_SUPERSCRIPT | XUI_DOC_UNDERLINE | XUI_DOC_STRIKE, blue);
            endpoints[variant][2] = decoration_insert(transaction, paragraph, variant ? "V" : "AV",
                XUI_DOC_SUPERSCRIPT | XUI_DOC_UNDERLINE | XUI_DOC_STRIKE, variant ? red : blue);
            CHECK(xuiDocumentTxnCommit(transaction, NULL) == XUI_OK); xuiDocumentTxnRelease(transaction);
            CHECK(xuiDocumentAcquireSnapshot(document[variant], &snapshot) == XUI_OK &&
                xuiDocumentRendererCreate(context, &desc, &renderer[variant]) == XUI_OK &&
                xuiDocumentRendererSetSnapshot(renderer[variant], snapshot, NULL) == XUI_OK);
            xuiDocumentSnapshotRelease(snapshot);
        }
        for (pass = 0; pass < 4; pass++) {
            double width = pass == 1 ? 35 : pass == 2 ? 12 : 120;
            object_shape_compare(document, renderer, endpoints, width, carets);
            if (width == 120) CHECK(fabs(carets[1][0].x - 27) < .01 &&
                fabs(carets[1][1].x - (sample < 2 ? 52 : sample == 2 ? 49 : 12)) < .01 &&
                fabs(carets[1][0].y - (sample < 2 ? 8 : sample == 2 ? .5 : 0)) < .01 &&
                fabs(carets[1][1].y - (sample < 2 ? 7.5 : sample == 2 ? 0 : 24)) < .01);
        }
        {
            xui_draw_context draw; float normal_base = sample < 2 ? 24 : sample == 2 ? 17 : 16;
            float script_base = sample < 2 ? 20 : sample == 2 ? 12 : 36;
            float script_x = sample < 2 ? 40 : sample == 2 ? 37 : 0;
            float script_mid = sample < 2 ? 45 : sample == 2 ? 42 : 5;
            probe.draws = decoration_span_count = decoration_line_count = 0;
            CHECK(proxy->tProxy.drawBegin(&proxy->tProxy, &draw, target) == XUI_OK &&
                xuiDocumentRendererDraw(renderer[1], draw, 0, 0, (xui_rect_t){0, 0, 120, 200}, NULL, 0) == XUI_OK &&
                proxy->tProxy.drawEnd(&proxy->tProxy, draw) == XUI_OK && decoration_span_count == 2 &&
                decoration_line_count == 6 && probe.draws == (sample < 2 ? 1u : 0u));
            CHECK(decoration_has(0, 7, normal_base + 2.5f, 1.5f, blue) &&
                decoration_has(7, 27, normal_base + 2.5f, 1.5f, red) &&
                decoration_has(script_x, script_mid, script_base + 2.5f, 1.5f, blue) &&
                decoration_has(script_mid, script_x + 12, script_base + 2.5f, 1.5f, red) &&
                decoration_has(script_x, script_mid, script_base - 4.5f, 2, blue) &&
                decoration_has(script_mid, script_x + 12, script_base - 4.5f, 2, red));
        }
        if (sample < 2) {
            xui_doc_rect_t bounds[2]; xui_doc_position_t hit;
            for (variant = 0; variant < 2; variant++) CHECK(
                xuiDocumentRendererGetNodeRect(renderer[variant], separators[variant], &bounds[variant]) == XUI_OK &&
                fabs(bounds[variant].x - 27) < .01 && fabs(bounds[variant].y) < .01 &&
                fabs(bounds[variant].width - 13) < .01 && fabs(bounds[variant].height - 36) < .01);
            CHECK(xuiDocumentRendererHitTest(renderer[1], 30, 18, &hit) == XUI_OK && hit.iNodeId == separators[1]);
            probe.width = 19; probe.height = 44; probe.baseline = 30;
            for (variant = 0; variant < 2; variant++) CHECK(xuiDocumentRendererInvalidateObjects(renderer[variant]) == XUI_OK);
            object_shape_compare(document, renderer, endpoints, 120, carets);
            CHECK(fabs(carets[1][1].x - 58) < .01 && fabs(carets[1][0].y - 14) < .01 &&
                fabs(carets[1][1].y - 13.5) < .01);
        }
        for (pass = 1; pass <= 2; pass++) {
            xui_document_renderer retry; xui_document_snapshot snapshot; xui_doc_rect_t caret; xui_doc_position_t at;
            CHECK(xuiDocumentAcquireSnapshot(document[1], &snapshot) == XUI_OK &&
                xuiDocumentRendererCreate(context, &desc, &retry) == XUI_OK &&
                xuiDocumentRendererSetSnapshot(retry, snapshot, NULL) == XUI_OK);
            xuiDocumentSnapshotRelease(snapshot); object_shape_calls = 0; object_shape_failure_at = (int)pass;
            CHECK(xuiDocumentRendererLayout(retry, 120, 0, 400) == XUI_ERROR_INVALID_STATE && object_shape_calls == (int)pass);
            object_shape_calls = object_shape_failure_at = 0;
            CHECK(xuiDocumentRendererLayout(retry, 120, 0, 400) == XUI_OK);
            at = decoration_position(document[1], endpoints[1][2], 1);
            CHECK(xuiDocumentRendererGetCaretRect(retry, &at, &caret) == XUI_OK &&
                fabs(caret.x - (sample < 2 ? 58 : sample == 2 ? 49 : 12)) < .01);
            xuiDocumentRendererRelease(retry);
        }
        for (variant = 0; variant < 2; variant++) {
            xuiDocumentRendererRelease(renderer[variant]); xuiDocumentRelease(document[variant]);
        }
    }
    /* One run may contain CRLF plus blank physical lines. Never draw those
     * controls, or shape the glyphs on different lines as one string. */
    {
        xui_document document[2]; xui_document_renderer renderer[2]; uint64_t nodes[2][6] = {{0}};
        static const char* const split[] = {"A", "V\r", "\nA", "V\n", "\nA", "V"};
        for (variant = 0; variant < 2; variant++) {
            xui_document_transaction transaction; xui_document_snapshot snapshot; uint64_t paragraph; unsigned part;
            CHECK(xuiDocumentCreate(NULL, &document[variant]) == XUI_OK &&
                xuiDocumentBeginTransaction(document[variant], NULL, &transaction) == XUI_OK);
            paragraph = decoration_insert(transaction, 1, NULL, 0, 0);
            for (part = 0; part < (variant ? 6 : 1); part++) nodes[variant][part] = decoration_insert(transaction,
                paragraph, variant ? split[part] : "AV\r\nAV\n\nAV", XUI_DOC_UNDERLINE, blue);
            CHECK(xuiDocumentTxnCommit(transaction, NULL) == XUI_OK); xuiDocumentTxnRelease(transaction);
            CHECK(xuiDocumentAcquireSnapshot(document[variant], &snapshot) == XUI_OK &&
                xuiDocumentRendererCreate(context, &desc, &renderer[variant]) == XUI_OK &&
                xuiDocumentRendererSetSnapshot(renderer[variant], snapshot, NULL) == XUI_OK);
            xuiDocumentSnapshotRelease(snapshot);
        }
        for (pass = 0; pass < 3; pass++) {
            xui_doc_rect_t carets[2][3]; unsigned part;
            for (variant = 0; variant < 2; variant++) {
                CHECK(xuiDocumentRendererLayout(renderer[variant], pass == 1 ? 12 : 120, 0, 400) == XUI_OK);
                for (part = 0; part < 3; part++) {
                    xui_doc_position_t at = decoration_position(document[variant], nodes[variant][variant ? part * 2 + 1 : 0],
                        variant ? 1 : part == 0 ? 2 : part == 1 ? 6 : 10);
                    CHECK(xuiDocumentRendererGetCaretRect(renderer[variant], &at, &carets[variant][part]) == XUI_OK);
                }
            }
            for (part = 0; part < 3; part++) CHECK(fabs(carets[0][part].x - carets[1][part].x) < .01 &&
                fabs(carets[0][part].y - carets[1][part].y) < .01);
            if (pass != 1) CHECK(fabs(carets[1][0].x - 17) < .01 && fabs(carets[1][0].y) < .01 &&
                fabs(carets[1][1].y - 24) < .01 && fabs(carets[1][2].y - 72) < .01);
        }
        for (variant = 0; variant < 2; variant++) {
            xui_draw_context draw; decoration_span_count = decoration_line_count = 0;
            CHECK(proxy->tProxy.drawBegin(&proxy->tProxy, &draw, target) == XUI_OK &&
                xuiDocumentRendererDraw(renderer[variant], draw, 0, 0, (xui_rect_t){0, 0, 120, 200}, NULL, 0) == XUI_OK &&
                proxy->tProxy.drawEnd(&proxy->tProxy, draw) == XUI_OK && decoration_span_count == 3 &&
                decoration_line_count == 3 && decoration_has(0, 17, 18.5f, 1.5f, blue) &&
                decoration_has(0, 17, 42.5f, 1.5f, blue) && decoration_has(0, 17, 90.5f, 1.5f, blue));
            xuiDocumentRendererRelease(renderer[variant]); xuiDocumentRelease(document[variant]);
        }
    }
    for (pass = 0; pass < 4; pass++) {
        xui_document document; xui_document_transaction transaction; xui_document_snapshot snapshot;
        xui_document_renderer renderer; xui_draw_context draw; xui_doc_rect_t size; int exact;
        uint64_t paragraph; const char* text = pass == 1 ? "A\r\n" : pass == 2 ? "\r\nA" : "\r\n\n";
        CHECK(xuiDocumentCreate(NULL, &document) == XUI_OK &&
            xuiDocumentBeginTransaction(document, NULL, &transaction) == XUI_OK);
        paragraph = decoration_insert(transaction, 1, NULL, 0, 0);
        decoration_insert(transaction, paragraph, text, XUI_DOC_UNDERLINE, blue);
        if (pass == 3) {
            xui_doc_node_desc_t image = {0}; uint64_t object;
            image.iSize = sizeof(image); image.iKind = XUI_DOC_IMAGE;
            CHECK(xuiDocumentTxnInsertNode(transaction, paragraph, XUI_DOCUMENT_APPEND, &image, &object) == XUI_OK);
        }
        CHECK(xuiDocumentTxnCommit(transaction, NULL) == XUI_OK); xuiDocumentTxnRelease(transaction);
        CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK &&
            xuiDocumentRendererCreate(context, &desc, &renderer) == XUI_OK &&
            xuiDocumentRendererSetSnapshot(renderer, snapshot, NULL) == XUI_OK &&
            xuiDocumentRendererLayout(renderer, 120, 0, 200) == XUI_OK &&
            xuiDocumentRendererGetSize(renderer, &size, &exact) == XUI_OK && exact && size.height > 0);
        xuiDocumentSnapshotRelease(snapshot); probe.draws = decoration_span_count = decoration_line_count = 0;
        CHECK(proxy->tProxy.drawBegin(&proxy->tProxy, &draw, target) == XUI_OK &&
            xuiDocumentRendererDraw(renderer, draw, 0, 0, (xui_rect_t){0, 0, 120, 200}, NULL, 0) == XUI_OK &&
            proxy->tProxy.drawEnd(&proxy->tProxy, draw) == XUI_OK &&
            decoration_span_count == (pass == 2 ? 1 : 0) &&
            decoration_line_count == (pass == 1 || pass == 2 ? 1 : 0) && probe.draws == (pass == 3 ? 1u : 0u));
        if (pass == 1 || pass == 2) CHECK(decoration_has(0, 10, pass == 1 ? 18.5f : 42.5f, 1.5f, blue));
        xuiDocumentRendererRelease(renderer); xuiDocumentRelease(document);
    }
    xuiDestroy(context); proxy->tProxy.textShape = decoration_base_shape; proxy->tProxy.fontGetMetrics = decoration_base_metrics;
    proxy->tProxy.drawLine = decoration_base_line; proxy->tProxy.drawTextSpans = decoration_base_spans;
    proxy->tProxy.drawText = decoration_base_text;
    proxy->tProxy.fontDestroy(&proxy->tProxy, probe.fonts[0]); proxy->tProxy.fontDestroy(&proxy->tProxy, probe.fonts[1]);
    puts("Object/control joint shaping: image/math, resize, soft/hard/raw LF/CRLF, blank lines, scripts, colors, ranges, hit, draw and failures passed");
}
