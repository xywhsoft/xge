/* Shared DLL/headless regression: Unicode boundaries are independent of a
 * proxy's cluster granularity and the Document's paint-only Text splits. */
static xui_text_shape_proc grapheme_base_shape;
static xui_draw_text_spans_proc grapheme_base_spans;
static xui_draw_rect_fill_proc grapheme_base_fill;
static const char grapheme_emoji[] = "\xf0\x9f\x91\xa9\xe2\x80\x8d\xf0\x9f\x92\xbb";
static int grapheme_merged_draws, grapheme_selection_fills;
static uint32_t grapheme_selection_color;
static xui_rect_t grapheme_selection_rect;

static int grapheme_merge_shape(xui_proxy proxy, const xui_text_item_t* pTextItem, xui_text_shape_t* shape)
{
    const char* text = pTextItem && pTextItem->iSize >= sizeof(*pTextItem) ? pTextItem->sText : NULL;
    int bytes = pTextItem && pTextItem->iSize >= sizeof(*pTextItem) ? pTextItem->iTextSize : 0;

    int result = grapheme_base_shape(proxy, pTextItem, shape);
    if (result == XUI_OK) {
        int i, count = 0;
        double width = 0;
        for (i = 0; i < shape->iClusterCount; i++) {
            xui_text_cluster_t cluster = shape->pClusters[i];
            if (bytes - cluster.iTextStart >= 11 &&
                memcmp(text + cluster.iTextStart, grapheme_emoji, 11) == 0) {
                CHECK(i + 2 < shape->iClusterCount &&
                    shape->pClusters[i + 2].iTextEnd == cluster.iTextStart + 11);
                cluster.iTextEnd += 7; cluster.fAdvance = 19;
                i += 2;
            }
            shape->pClusters[count++] = cluster;
            width += cluster.fAdvance;
        }
        shape->iClusterCount = count; shape->fWidth = (float)width;
    }
    return result;
}
static int grapheme_draw_spans(xui_proxy proxy, xui_draw_context draw, const xui_text_item_t* pTextItem, xui_rect_t rect, uint32_t color, uint32_t flags, const xui_text_paint_span_t* spans, int count)
{
    const char* text = pTextItem && pTextItem->iSize >= sizeof(*pTextItem) ? pTextItem->sText : NULL;
    int bytes = pTextItem && pTextItem->iSize >= sizeof(*pTextItem) ? pTextItem->iTextSize : 0;

    int i;
    CHECK(count > 0 && spans[0].iStart == 0 && spans[count - 1].iEnd == bytes);
    for (i = 1; i < count; i++) CHECK(spans[i].iStart == spans[i - 1].iEnd);
    for (i = 0; i + 11 <= bytes; i++)
        if (memcmp(text + i, grapheme_emoji, 11) == 0) grapheme_merged_draws++;
    return grapheme_base_spans(proxy, draw, pTextItem, rect, color, flags, spans, count);
}
static int grapheme_draw_fill(xui_proxy proxy, xui_draw_context draw,
    xui_rect_t rect, uint32_t color)
{
    if (color == grapheme_selection_color) {
        grapheme_selection_fills++;
        grapheme_selection_rect = rect;
    }
    return grapheme_base_fill(proxy, draw, rect, color);
}
static xui_doc_position_t grapheme_position(xui_document document,
    uint64_t node, uint64_t offset, unsigned affinity)
{
    xui_doc_position_t position = {0};
    position.iSize = sizeof(position); position.iKind = XUI_DOC_POSITION_TEXT;
    position.iDocumentId = xuiDocumentGetIdentity(document);
    position.iRevision = xuiDocumentGetRevision(document);
    position.iNodeId = node; position.iOffset = offset; position.iAffinity = affinity;
    return position;
}
static void document_cross_node_graphemes(xui_context parent,
    xui_surface target, xui_test_proxy_state_t* proxy)
{
    xui_context context;
    unsigned sample;
    grapheme_base_shape = proxy->tProxy.textShape;
    grapheme_base_spans = proxy->tProxy.drawTextSpans;
    grapheme_base_fill = proxy->tProxy.drawRectFill;
    proxy->tProxy.textShape = grapheme_merge_shape;
    proxy->tProxy.drawTextSpans = grapheme_draw_spans;
    proxy->tProxy.drawRectFill = grapheme_draw_fill;
    CHECK(xuiCreate(&context) == XUI_OK && xuiSetProxy(context, &proxy->tProxy) == XUI_OK &&
        xuiSetDefaultFont(context, xuiGetDefaultFont(parent)) == XUI_OK);
    grapheme_selection_color = XUI_COLOR_RGBA(1, 2, 3, 128);
    for (sample = 0; sample < 3; sample++) {
        xui_document document[2]; xui_document_renderer renderer[2];
        uint64_t nodes[2][4] = {{0}};
        unsigned variant, pass, parts = sample ? 3 : 2;
        const char* full = sample == 0 ? "e\xcc\x81X" : sample == 1 ?
            "\xf0\x9f\x91\xa9\xe2\x80\x8d\xf0\x9f\x92\xbbX" :
            "a\xf0\x9f\x91\xa9\xe2\x80\x8d\xf0\x9f\x92\xbb" "b";
        const char* split[3] = {sample == 0 ? "e" : sample == 1 ?
            "\xf0\x9f\x91\xa9" : "a\xf0\x9f\x91\xa9",
            sample == 0 ? "\xcc\x81X" : "\xe2\x80\x8d",
            sample == 1 ? "\xf0\x9f\x92\xbbX" : "\xf0\x9f\x92\xbb" "b"};
        uint64_t begin = sample == 2 ? 1 : 0, length = sample ? 11 : 3;
        for (variant = 0; variant < 2; variant++) {
            xui_document_transaction transaction; xui_document_snapshot snapshot;
            xui_doc_node_desc_t node = {0}; uint64_t paragraph; unsigned part;
            CHECK(xuiDocumentCreate(NULL, &document[variant]) == XUI_OK &&
                xuiDocumentBeginTransaction(document[variant], NULL, &transaction) == XUI_OK);
            node.iSize = sizeof(node); node.iKind = XUI_DOC_PARAGRAPH;
            CHECK(xuiDocumentTxnInsertNode(transaction, 1, XUI_DOCUMENT_APPEND,
                &node, &paragraph) == XUI_OK);
            for (part = 0; part < (variant ? parts : 1); part++) {
                node.iKind = XUI_DOC_TEXT; node.sText = variant ? split[part] : full;
                node.iTextBytes = strlen(node.sText);
                node.tAttributes.iTextColor = variant && part ? XUI_COLOR_RGBA(180, 20, 40, 255) : 0;
                CHECK(xuiDocumentTxnInsertNode(transaction, paragraph,
                    XUI_DOCUMENT_APPEND, &node, &nodes[variant][part]) == XUI_OK);
                if (variant && !part) {
                    node.sText = ""; node.iTextBytes = 0;
                    CHECK(xuiDocumentTxnInsertNode(transaction, paragraph,
                        XUI_DOCUMENT_APPEND, &node, &nodes[variant][3]) == XUI_OK);
                }
            }
            CHECK(xuiDocumentTxnCommit(transaction, NULL) == XUI_OK);
            xuiDocumentTxnRelease(transaction);
            CHECK(xuiDocumentAcquireSnapshot(document[variant], &snapshot) == XUI_OK &&
                xuiDocumentRendererCreate(context, NULL, &renderer[variant]) == XUI_OK &&
                xuiDocumentRendererSetSnapshot(renderer[variant], snapshot, NULL) == XUI_OK);
            xuiDocumentSnapshotRelease(snapshot);
        }
        for (pass = 0; pass < 3; pass++) {
            xui_doc_rect_t head[2], tail[2], next[2];
            double width = pass == 1 ? 7 : 300;
            for (variant = 0; variant < 2; variant++) {
                xui_doc_position_t at, hit;
                uint64_t tail_node = variant ? nodes[variant][parts - 1] : nodes[variant][0];
                uint64_t tail_offset = variant ? (sample ? 4 : 2) : begin + length;
                CHECK(xuiDocumentRendererLayout(renderer[variant], width, 0, 200) == XUI_OK);
                at = grapheme_position(document[variant], nodes[variant][0], begin, XUI_DOC_AFTER);
                CHECK(xuiDocumentRendererGetCaretRect(renderer[variant], &at, &head[variant]) == XUI_OK);
                at = grapheme_position(document[variant], tail_node, tail_offset, XUI_DOC_BEFORE);
                CHECK(xuiDocumentRendererGetCaretRect(renderer[variant], &at, &tail[variant]) == XUI_OK);
                at.iOffset++;
                CHECK(xuiDocumentRendererGetCaretRect(renderer[variant], &at, &next[variant]) == XUI_OK);
                CHECK(tail[variant].x > head[variant].x && fabs(tail[variant].y - head[variant].y) < .01);
                CHECK(xuiDocumentRendererHitTest(renderer[variant],
                    head[variant].x + (tail[variant].x - head[variant].x) * .25,
                    head[variant].y + head[variant].height * .5, &hit) == XUI_OK &&
                    hit.iNodeId == nodes[variant][0] && hit.iOffset == begin);
                CHECK(xuiDocumentRendererHitTest(renderer[variant],
                    head[variant].x + (tail[variant].x - head[variant].x) * .75,
                    head[variant].y + head[variant].height * .5, &hit) == XUI_OK &&
                    hit.iNodeId == tail_node && hit.iOffset == tail_offset);
                /* Every raw UTF-8 boundary inside the grapheme obeys affinity. */
                at = grapheme_position(document[variant], nodes[variant][0],
                    begin + (sample ? 4 : 1), XUI_DOC_BEFORE);
                {
                    xui_doc_rect_t before, after;
                    CHECK(xuiDocumentRendererGetCaretRect(renderer[variant], &at, &before) == XUI_OK);
                    at.iAffinity = XUI_DOC_AFTER;
                    CHECK(xuiDocumentRendererGetCaretRect(renderer[variant], &at, &after) == XUI_OK &&
                        fabs(before.x - head[variant].x) < .01 &&
                        fabs(after.x - tail[variant].x) < .01 &&
                        fabs(before.y - after.y) < .01);
                }
            }
            CHECK(fabs(head[0].x - head[1].x) < .01 && fabs(head[0].y - head[1].y) < .01 &&
                fabs(tail[0].x - tail[1].x) < .01 && fabs(tail[0].y - tail[1].y) < .01 &&
                fabs(next[0].x - next[1].x) < .01 && fabs(next[0].y - next[1].y) < .01);
            if (sample) CHECK(fabs(tail[1].x - head[1].x - 19) < .01);
            {
                xui_doc_range_t selection = {0}; xui_doc_rect_t rect;
                uint64_t total; xui_draw_context draw;
                selection.tAnchor = grapheme_position(document[1], nodes[1][1], 0, XUI_DOC_BEFORE);
                selection.tCaret = grapheme_position(document[1], nodes[1][1], sample ? 3 : 2, XUI_DOC_AFTER);
                CHECK(xuiDocumentRendererGetRangeRects(renderer[1], &selection, &rect, 1, &total) == XUI_OK &&
                    total == 1 && fabs(rect.x - head[1].x) < .01 &&
                    fabs(rect.width - (tail[1].x - head[1].x)) < .01 &&
                    fabs(rect.y - head[1].y) < .01);
                if (sample) CHECK(xuiDocumentRendererGetNodeRect(renderer[1], nodes[1][1], &rect) == XUI_OK &&
                    fabs(rect.width - 19) < .01);
                grapheme_selection_fills = grapheme_merged_draws = 0;
                CHECK(proxy->tProxy.drawBegin(&proxy->tProxy, &draw, target) == XUI_OK &&
                    xuiDocumentRendererDraw(renderer[1], draw, 0, 0,
                        (xui_rect_t){0, 0, 300, 200}, &selection, grapheme_selection_color) == XUI_OK &&
                    proxy->tProxy.drawEnd(&proxy->tProxy, draw) == XUI_OK &&
                    grapheme_selection_fills == 1 &&
                    fabs(grapheme_selection_rect.fW - (tail[1].x - head[1].x)) < .01);
                if (sample) CHECK(grapheme_merged_draws == 1);
            }
        }
        for (variant = 0; variant < 2; variant++) {
            xuiDocumentRendererRelease(renderer[variant]); xuiDocumentRelease(document[variant]);
        }
    }
    /* SOURCE rows and code blocks use the same Unicode boundary flags even
     * when the proxy emits a separate cluster for the combining scalar. */
    for (sample = 0; sample < 2; sample++) {
        xui_document document; xui_document_snapshot snapshot;
        xui_document_renderer renderer; xui_doc_desc_t desc = {0};
        xui_doc_position_t at, hit; xui_doc_range_t selection;
        xui_doc_rect_t head, tail, before, after, rect;
        uint64_t node_id = 1, total; xui_draw_context draw;
        desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN;
        CHECK(xuiDocumentCreate(sample ? &desc : NULL, &document) == XUI_OK);
        if (sample) CHECK(xuiDocumentLoadMarkdown(document, "e\xcc\x81X", 4) == XUI_OK);
        else {
            xui_document_transaction transaction; xui_doc_node_desc_t node = {0};
            node.iSize = sizeof(node); node.iKind = XUI_DOC_CODE_BLOCK;
            node.sText = "e\xcc\x81X\nZ"; node.iTextBytes = 6;
            CHECK(xuiDocumentBeginTransaction(document, NULL, &transaction) == XUI_OK &&
                xuiDocumentTxnInsertNode(transaction, 1, XUI_DOCUMENT_APPEND, &node, &node_id) == XUI_OK &&
                xuiDocumentTxnCommit(transaction, NULL) == XUI_OK);
            xuiDocumentTxnRelease(transaction);
        }
        CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK &&
            xuiDocumentRendererCreate(context, NULL, &renderer) == XUI_OK &&
            xuiDocumentRendererSetSnapshot(renderer, snapshot, NULL) == XUI_OK);
        xuiDocumentSnapshotRelease(snapshot);
        if (sample) CHECK(xuiDocumentRendererSetMode(renderer, XUI_DOC_SOURCE_TEXT) == XUI_OK);
        CHECK(xuiDocumentRendererLayout(renderer, 300, 0, 100) == XUI_OK);
        at = grapheme_position(document, node_id, 0, XUI_DOC_BEFORE);
        if (sample) at.iKind = XUI_DOC_POSITION_SOURCE;
        CHECK(xuiDocumentRendererGetCaretRect(renderer, &at, &head) == XUI_OK);
        at.iOffset = 3;
        CHECK(xuiDocumentRendererGetCaretRect(renderer, &at, &tail) == XUI_OK);
        at.iOffset = 1;
        CHECK(xuiDocumentRendererGetCaretRect(renderer, &at, &before) == XUI_OK);
        at.iAffinity = XUI_DOC_AFTER;
        CHECK(xuiDocumentRendererGetCaretRect(renderer, &at, &after) == XUI_OK);
        if (fabs(before.x - head.x) >= .01 || fabs(after.x - tail.x) >= .01)
            fprintf(stderr, "grapheme source/code %u: head %.3f tail %.3f before %.3f after %.3f\n",
                sample, head.x, tail.x, before.x, after.x);
        CHECK(
            fabs(before.x - head.x) < .01 && fabs(after.x - tail.x) < .01);
        CHECK(xuiDocumentRendererHitTest(renderer, head.x + (tail.x - head.x) * .75,
            head.y + head.height * .5, &hit) == XUI_OK && hit.iOffset == 3);
        selection.tAnchor = at; selection.tAnchor.iAffinity = XUI_DOC_BEFORE;
        selection.tCaret = at; selection.tCaret.iOffset = 3;
        CHECK(xuiDocumentRendererGetRangeRects(renderer, &selection, &rect, 1, &total) == XUI_OK &&
            total == 1 && fabs(rect.x - head.x) < .01 && fabs(rect.width - (tail.x - head.x)) < .01);
        grapheme_selection_fills = 0;
        CHECK(proxy->tProxy.drawBegin(&proxy->tProxy, &draw, target) == XUI_OK &&
            xuiDocumentRendererDraw(renderer, draw, 0, 0, (xui_rect_t){0, 0, 300, 100},
                &selection, grapheme_selection_color) == XUI_OK &&
            proxy->tProxy.drawEnd(&proxy->tProxy, draw) == XUI_OK && grapheme_selection_fills == 1);
        xuiDocumentRendererRelease(renderer); xuiDocumentRelease(document);
    }
    {
        /* One grapheme may contain an arbitrarily long combining sequence.
         * Audit its actual scans rather than counting only the binary lookup. */
        char text[2002]; unsigned i;
        xui_document document; xui_document_transaction transaction;
        xui_document_snapshot snapshot; xui_document_renderer renderer;
        xui_doc_node_desc_t node = {0}; uint64_t node_id;
        xui_doc_position_t at; xui_doc_rect_t rect;
        xui_doc_renderer_stats_t before = {0}, after = {0};
        text[0] = 'e';
        for (i = 0; i < 1000; i++) memcpy(text + 1 + i * 2, "\xcc\x81", 2);
        text[2001] = 0;
        node.iSize = sizeof(node); node.iKind = XUI_DOC_CODE_BLOCK;
        node.sText = text; node.iTextBytes = 2001;
        CHECK(xuiDocumentCreate(NULL, &document) == XUI_OK &&
            xuiDocumentBeginTransaction(document, NULL, &transaction) == XUI_OK &&
            xuiDocumentTxnInsertNode(transaction, 1, XUI_DOCUMENT_APPEND, &node, &node_id) == XUI_OK &&
            xuiDocumentTxnCommit(transaction, NULL) == XUI_OK);
        xuiDocumentTxnRelease(transaction);
        CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK &&
            xuiDocumentRendererCreate(context, NULL, &renderer) == XUI_OK &&
            xuiDocumentRendererSetSnapshot(renderer, snapshot, NULL) == XUI_OK &&
            xuiDocumentRendererLayout(renderer, 300, 0, 100) == XUI_OK);
        xuiDocumentSnapshotRelease(snapshot);
        before.iSize = after.iSize = sizeof(before);
        at = grapheme_position(document, node_id, 1001, XUI_DOC_AFTER);
        CHECK(xuiDocumentRendererGetStats(renderer, &before) == XUI_OK &&
            xuiDocumentRendererGetCaretRect(renderer, &at, &rect) == XUI_OK &&
            xuiDocumentRendererGetStats(renderer, &after) == XUI_OK &&
            after.iCaretFragmentsExamined - before.iCaretFragmentsExamined >= 1000);
        xuiDocumentRendererRelease(renderer); xuiDocumentRelease(document);
    }
    xuiDestroy(context);
    proxy->tProxy.textShape = grapheme_base_shape;
    proxy->tProxy.drawTextSpans = grapheme_base_spans;
    proxy->tProxy.drawRectFill = grapheme_base_fill;
    puts("Cross-node combining/merged emoji and SOURCE/code retain wrapping, affinity, hit and whole-grapheme selection");
}
