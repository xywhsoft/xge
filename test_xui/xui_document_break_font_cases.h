/* Explicit line breaks retain their own effective inline font and marks. */
static int break_font_fail_space;
static int break_font_shape(xui_proxy proxy, const xui_text_item_t* pTextItem, xui_text_shape_t* shape)
{
    const char* text = pTextItem && pTextItem->iSize >= sizeof(*pTextItem) ? pTextItem->sText : NULL;
    int bytes = pTextItem && pTextItem->iSize >= sizeof(*pTextItem) ? pTextItem->iTextSize : 0;

    if (break_font_fail_space && bytes == 1 && text[0] == ' ')
        return XUI_ERROR_INVALID_STATE;
    return decoration_base_shape(proxy, pTextItem, shape);
}
static void document_break_font_context(xui_surface target, xui_test_proxy_state_t* proxy)
{
    xui_context context; xui_font font; unsigned sample, variant, pass;
    const uint32_t color = XUI_COLOR_RGBA(20, 40, 180, 255);
    decoration_base_shape = proxy->tProxy.textShape;
    decoration_base_metrics = proxy->tProxy.fontGetMetrics;
    decoration_base_line = proxy->tProxy.drawLine;
    proxy->tProxy.textShape = break_font_shape;
    proxy->tProxy.fontGetMetrics = decoration_metrics;
    proxy->tProxy.drawLine = decoration_line;
    decoration_invalid_metrics = decoration_metrics_error = decoration_line_error = 0;
    CHECK(proxy->tProxy.fontLoadFile(&proxy->tProxy, &font, "break.ttf", 20, 0) == XUI_OK &&
        xuiCreate(&context) == XUI_OK && xuiSetProxy(context, &proxy->tProxy) == XUI_OK &&
        xuiSetDefaultFont(context, font) == XUI_OK);
    for (sample = 0; sample < 12; sample++) {
        xui_document document[2]; xui_document_renderer renderer[2];
        uint64_t last[2], separator = 0; xui_doc_renderer_desc_t desc = {0};
        unsigned kind = sample % 6; double zoom = sample >= 6 ? 1.25 : 1;
        uint32_t marks = XUI_DOC_UNDERLINE | XUI_DOC_STRIKE |
            (kind == 2 || kind == 5 ? XUI_DOC_SUPERSCRIPT : 0);
        int hard = kind >= 4;
        desc.iSize = sizeof(desc); desc.fLineGap = 4; desc.fZoom = (float)zoom;
        for (variant = 0; variant < 2; variant++) {
            xui_document_transaction transaction; xui_document_snapshot snapshot;
            xui_doc_node_desc_t node = {0}; uint64_t paragraph, unused;
            CHECK(xuiDocumentCreate(NULL, &document[variant]) == XUI_OK &&
                xuiDocumentBeginTransaction(document[variant], NULL, &transaction) == XUI_OK);
            node.iSize = sizeof(node); node.iKind = kind == 3 ? XUI_DOC_HEADING : XUI_DOC_PARAGRAPH;
            node.tAttributes.iHeadingLevel = kind == 3 ? 1 : 0;
            node.tAttributes.fFontSize = kind == 1 || kind == 4 ? 40 : 0;
            node.tAttributes.iTextColor = color;
            CHECK(xuiDocumentTxnInsertNode(transaction, 1, XUI_DOCUMENT_APPEND, &node, &paragraph) == XUI_OK);
            decoration_insert(transaction, paragraph, "A", marks, 0);
            if (variant) {
                memset(&node, 0, sizeof(node)); node.iSize = sizeof(node);
                node.iKind = hard ? XUI_DOC_HARD_BREAK : XUI_DOC_SOFT_BREAK;
                node.tAttributes.iMarks = marks;
                CHECK(xuiDocumentTxnInsertNode(transaction, paragraph, XUI_DOCUMENT_APPEND, &node, &separator) == XUI_OK);
            }
            unused = decoration_insert(transaction, paragraph, variant ? "V" : hard ? "\nV" : " V", marks, 0);
            last[variant] = unused;
            CHECK(xuiDocumentTxnCommit(transaction, NULL) == XUI_OK); xuiDocumentTxnRelease(transaction);
            CHECK(xuiDocumentAcquireSnapshot(document[variant], &snapshot) == XUI_OK &&
                xuiDocumentRendererCreate(context, &desc, &renderer[variant]) == XUI_OK &&
                xuiDocumentRendererSetSnapshot(renderer[variant], snapshot, NULL) == XUI_OK);
            xuiDocumentSnapshotRelease(snapshot);
        }
        for (pass = 0; pass < 4; pass++) {
            xui_doc_rect_t caret[2], size[2], gap; int exact;
            double font_size = kind == 1 || kind == 4 ? 40 : kind == 3 ? 36 :
                kind == 2 || kind == 5 ? 15 : 20;
            double width = pass == 1 ? font_size * .8 : pass == 2 ? 42 : 200;
            for (variant = 0; variant < 2; variant++) {
                xui_doc_position_t at = decoration_position(document[variant], last[variant], variant ? 1 : 2);
                CHECK(xuiDocumentRendererLayout(renderer[variant], width, 0, 300) == XUI_OK &&
                    xuiDocumentRendererGetCaretRect(renderer[variant], &at, &caret[variant]) == XUI_OK &&
                    xuiDocumentRendererGetSize(renderer[variant], &size[variant], &exact) == XUI_OK && exact);
            }
            CHECK(fabs(caret[0].x - caret[1].x) < .01 && fabs(caret[0].y - caret[1].y) < .01 &&
                fabs(caret[0].height - caret[1].height) < .01 &&
                fabs(size[0].width - size[1].width) < .01 && fabs(size[0].height - size[1].height) < .01);
            CHECK(xuiDocumentRendererGetNodeRect(renderer[1], separator, &gap) == XUI_OK &&
                fabs(gap.width - (hard ? 1 : font_size * zoom * .5)) < .01 &&
                fabs(gap.height - font_size * zoom) < .01);
            if ((pass == 0 || pass == 3) && !hard) {
                xui_draw_context draw; double middle = floor(gap.x + gap.width * .5 + .5);
                unsigned type; decoration_line_count = 0;
                CHECK(proxy->tProxy.drawBegin(&proxy->tProxy, &draw, target) == XUI_OK &&
                    xuiDocumentRendererDraw(renderer[1], draw, 0, 0, (xui_rect_t){0, 0, 200, 300}, NULL, 0) == XUI_OK &&
                    proxy->tProxy.drawEnd(&proxy->tProxy, draw) == XUI_OK);
                for (type = 0; type < 2; type++) {
                    double y = floor(gap.y + .5) + font_size * zoom * .8 + (type ? -4.5 : 2.5);
                    int i, found = 0;
                    for (i = 0; i < decoration_line_count; i++) if (decoration_lines[i].left <= middle &&
                        decoration_lines[i].right >= middle && fabs(decoration_lines[i].y - y) < .01 &&
                        decoration_lines[i].color == color) found = 1;
                    CHECK(found);
                }
            }
        }
        {
            xui_document_renderer retry; xui_document_snapshot snapshot;
            CHECK(xuiDocumentAcquireSnapshot(document[1], &snapshot) == XUI_OK &&
                xuiDocumentRendererCreate(context, &desc, &retry) == XUI_OK &&
                xuiDocumentRendererSetSnapshot(retry, snapshot, NULL) == XUI_OK);
            xuiDocumentSnapshotRelease(snapshot); break_font_fail_space = 1;
            CHECK(xuiDocumentRendererLayout(retry, 200, 0, 300) == XUI_ERROR_INVALID_STATE);
            break_font_fail_space = 0;
            CHECK(xuiDocumentRendererLayout(retry, 200, 0, 300) == XUI_OK);
            xuiDocumentRendererRelease(retry);
        }
        if (!sample) for (pass = 0; pass < 2; pass++) {
            xui_document_transaction transaction; xui_document_snapshot snapshot;
            xui_document_change_set changes; xui_doc_attributes_t attrs = {0};
            xui_doc_rect_t caret, gap; xui_doc_position_t at;
            attrs.iMarks = marks; attrs.fFontSize = 40;
            if (!pass) {
                CHECK(xuiDocumentBeginTransaction(document[1], NULL, &transaction) == XUI_OK &&
                    xuiDocumentTxnSetAttributes(transaction, separator, &attrs) == XUI_OK &&
                    xuiDocumentTxnCommit(transaction, &changes) == XUI_OK);
                xuiDocumentTxnRelease(transaction);
            } else CHECK(xuiDocumentUndo(document[1], &changes) == XUI_OK);
            CHECK(xuiDocumentAcquireSnapshot(document[1], &snapshot) == XUI_OK &&
                xuiDocumentRendererSetSnapshot(renderer[1], snapshot, changes) == XUI_OK &&
                xuiDocumentRendererLayout(renderer[1], 200, 0, 300) == XUI_OK);
            xuiDocumentSnapshotRelease(snapshot); xuiDocumentChangeSetRelease(changes);
            at = decoration_position(document[1], last[1], 1);
            CHECK(xuiDocumentRendererGetCaretRect(renderer[1], &at, &caret) == XUI_OK &&
                xuiDocumentRendererGetNodeRect(renderer[1], separator, &gap) == XUI_OK &&
                fabs(gap.width - (pass ? 10 : 20)) < .01 && fabs(gap.height - (pass ? 20 : 40)) < .01 &&
                fabs(caret.x - (pass ? 30 : 40)) < .01 && fabs(caret.y - (pass ? 0 : 16)) < .01);
        }
        for (variant = 0; variant < 2; variant++) {
            xuiDocumentRendererRelease(renderer[variant]); xuiDocumentRelease(document[variant]);
        }
    }
    {
        static const char source[] = "[A\nV](https://example.test/)\n";
        xui_document document; xui_document_snapshot snapshot; xui_document_renderer renderer;
        xui_doc_renderer_desc_t desc = {0}; xui_doc_node_info_t info = {0};
        xui_doc_desc_t markdown = {0};
        xui_doc_rect_t gap, caret; xui_doc_position_t at; xui_draw_context draw;
        uint64_t paragraph, separator, last, length; char saved[sizeof(source)]; int i, found = 0;
        markdown.iSize = sizeof(markdown); markdown.iProfile = XUI_DOCUMENT_MARKDOWN;
        CHECK(xuiDocumentCreate(&markdown, &document) == XUI_OK &&
            xuiDocumentLoadMarkdown(document, source, strlen(source)) == XUI_OK &&
            xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK &&
            xuiDocumentSnapshotGetChild(snapshot, 1, 0, &paragraph) == XUI_OK &&
            xuiDocumentSnapshotGetChild(snapshot, paragraph, 1, &separator) == XUI_OK &&
            xuiDocumentSnapshotGetChild(snapshot, paragraph, 2, &last) == XUI_OK);
        info.iSize = sizeof(info);
        CHECK(xuiDocumentSnapshotGetNode(snapshot, separator, &info) == XUI_OK &&
            info.iKind == XUI_DOC_SOFT_BREAK && (info.tAttributes.iMarks & XUI_DOC_LINK) &&
            info.iTextBytes == 0 && xuiDocumentSnapshotCopySource(snapshot, saved, sizeof(saved), &length) == XUI_OK &&
            length == strlen(source) && !memcmp(saved, source, (size_t)length));
        desc.iSize = sizeof(desc); desc.iLinkColor = color;
        CHECK(xuiDocumentRendererCreate(context, &desc, &renderer) == XUI_OK &&
            xuiDocumentRendererSetSnapshot(renderer, snapshot, NULL) == XUI_OK &&
            xuiDocumentRendererLayout(renderer, 200, 0, 300) == XUI_OK);
        xuiDocumentSnapshotRelease(snapshot); at = decoration_position(document, last, 1);
        CHECK(xuiDocumentRendererGetNodeRect(renderer, separator, &gap) == XUI_OK &&
            xuiDocumentRendererGetCaretRect(renderer, &at, &caret) == XUI_OK &&
            fabs(gap.x - 10) < .01 && fabs(gap.width - 10) < .01 && fabs(caret.x - 30) < .01);
        decoration_line_count = 0;
        CHECK(proxy->tProxy.drawBegin(&proxy->tProxy, &draw, target) == XUI_OK &&
            xuiDocumentRendererDraw(renderer, draw, 0, 0, (xui_rect_t){0, 0, 200, 300}, NULL, 0) == XUI_OK &&
            proxy->tProxy.drawEnd(&proxy->tProxy, draw) == XUI_OK);
        for (i = 0; i < decoration_line_count; i++) if (decoration_lines[i].left <= 15 &&
            decoration_lines[i].right >= 15 && fabs(decoration_lines[i].y - 18.5) < .01 &&
            decoration_lines[i].color == color) found = 1;
        CHECK(found);
        xuiDocumentRendererRelease(renderer); xuiDocumentRelease(document);
    }
    for (variant = 0; variant < 2; variant++) for (pass = 0; pass < 2; pass++) {
        xui_proxy_t callbacks = proxy->tProxy; xui_context blank_context;
        xui_document document; xui_document_transaction transaction; xui_document_snapshot snapshot;
        xui_document_renderer renderer; xui_doc_renderer_desc_t desc = {0}; xui_doc_node_desc_t node = {0};
        xui_doc_rect_t first, last_rect; uint64_t paragraph, soft[2], unused; xui_draw_context draw;
        if (variant) callbacks.drawTextSpans = NULL;
        CHECK(xuiCreate(&blank_context) == XUI_OK && xuiSetProxy(blank_context, &callbacks) == XUI_OK &&
            xuiSetDefaultFont(blank_context, font) == XUI_OK && xuiDocumentCreate(NULL, &document) == XUI_OK &&
            xuiDocumentBeginTransaction(document, NULL, &transaction) == XUI_OK);
        node.iSize = sizeof(node); node.iKind = XUI_DOC_PARAGRAPH; node.tAttributes.fFontSize = 40;
        node.tAttributes.iTextColor = color;
        CHECK(xuiDocumentTxnInsertNode(transaction, 1, XUI_DOCUMENT_APPEND, &node, &paragraph) == XUI_OK);
        memset(&node, 0, sizeof(node)); node.iSize = sizeof(node); node.iKind = XUI_DOC_SOFT_BREAK;
        node.tAttributes.iMarks = XUI_DOC_UNDERLINE | XUI_DOC_STRIKE;
        CHECK(xuiDocumentTxnInsertNode(transaction, paragraph, XUI_DOCUMENT_APPEND, &node, &soft[0]) == XUI_OK);
        if (pass) {
            node.iKind = XUI_DOC_HARD_BREAK;
            CHECK(xuiDocumentTxnInsertNode(transaction, paragraph, XUI_DOCUMENT_APPEND, &node, &unused) == XUI_OK);
            node.iKind = XUI_DOC_SOFT_BREAK;
        }
        CHECK(xuiDocumentTxnInsertNode(transaction, paragraph, XUI_DOCUMENT_APPEND, &node, &soft[1]) == XUI_OK &&
            xuiDocumentTxnCommit(transaction, NULL) == XUI_OK); xuiDocumentTxnRelease(transaction);
        desc.iSize = sizeof(desc); desc.fLineGap = 4;
        CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK &&
            xuiDocumentRendererCreate(blank_context, &desc, &renderer) == XUI_OK &&
            xuiDocumentRendererSetSnapshot(renderer, snapshot, NULL) == XUI_OK &&
            xuiDocumentRendererLayout(renderer, 120, 0, 200) == XUI_OK &&
            xuiDocumentRendererGetNodeRect(renderer, soft[0], &first) == XUI_OK &&
            xuiDocumentRendererGetNodeRect(renderer, soft[1], &last_rect) == XUI_OK &&
            fabs(first.width - 20) < .01 && fabs(first.height - 40) < .01 &&
            fabs(last_rect.x - (pass ? 0 : 20)) < .01 && fabs(last_rect.y - (pass ? 44 : 0)) < .01);
        xuiDocumentSnapshotRelease(snapshot); decoration_line_count = 0;
        CHECK(callbacks.drawBegin(&callbacks, &draw, target) == XUI_OK &&
            xuiDocumentRendererDraw(renderer, draw, 0, 0, (xui_rect_t){0, 0, 120, 200}, NULL, 0) == XUI_OK &&
            callbacks.drawEnd(&callbacks, draw) == XUI_OK);
        {
            unsigned part, type;
            for (part = 0; part < 2; part++) for (type = 0; type < 2; type++) {
                double middle = part && !pass ? 30 : 10;
                double y = (part && pass ? 44 : 0) + 32 + (type ? -4.5 : 2.5);
                int found = 0, i;
                for (i = 0; i < decoration_line_count; i++) if (decoration_lines[i].left <= middle &&
                    decoration_lines[i].right >= middle && fabs(decoration_lines[i].y - y) < .01 &&
                    decoration_lines[i].color == color) found = 1;
                CHECK(found);
            }
        }
        xuiDocumentRendererRelease(renderer); xuiDocumentRelease(document); xuiDestroy(blank_context);
    }
    xuiDestroy(context); proxy->tProxy.fontDestroy(&proxy->tProxy, font);
    proxy->tProxy.textShape = decoration_base_shape;
    proxy->tProxy.fontGetMetrics = decoration_base_metrics;
    proxy->tProxy.drawLine = decoration_base_line;
    puts("Explicit soft/hard breaks: real space advance, inherited font/color, headings, scripts, zoom, wrapping, decorations, failure retry, font mutation/undo, lossless Markdown links and blank/fallback spans passed");
}
