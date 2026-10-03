/* Mandatory controls use paragraph-wide Unicode decisions, including CRLF. */
static const char* const document_mandatory_breaks[] = {
    "\n", "\r", "\r\n", "\v", "\f", "\xc2\x85", "\xe2\x80\xa8", "\xe2\x80\xa9"
};
static unsigned mandatory_break_draws;
static int mandatory_break_text_clean(const char* text, int bytes)
{
    int i;
    for (i = 0; i < bytes; i++) if ((unsigned char)text[i] < 32 ||
        (i + 1 < bytes && (unsigned char)text[i] == 0xc2 && (unsigned char)text[i + 1] == 0x85) ||
        (i + 2 < bytes && !memcmp(text + i, "\xe2\x80", 2) &&
            ((unsigned char)text[i + 2] == 0xa8 || (unsigned char)text[i + 2] == 0xa9))) return 0;
    return 1;
}
static int mandatory_break_draw_text(xui_proxy proxy, xui_draw_context draw, const xui_text_item_t* pTextItem, xui_rect_t rect, uint32_t color, uint32_t flags)
{
    const char* text = pTextItem && pTextItem->iSize >= sizeof(*pTextItem) ? pTextItem->sText : NULL;

    CHECK(mandatory_break_text_clean(text, (int)strlen(text)));
    mandatory_break_draws++;
    return decoration_base_text(proxy, draw, pTextItem, rect, color, flags);
}
static int mandatory_break_draw_spans(xui_proxy proxy, xui_draw_context draw, const xui_text_item_t* pTextItem, xui_rect_t rect, uint32_t color, uint32_t flags, const xui_text_paint_span_t* spans, int count)
{
    const char* text = pTextItem && pTextItem->iSize >= sizeof(*pTextItem) ? pTextItem->sText : NULL;
    int bytes = pTextItem && pTextItem->iSize >= sizeof(*pTextItem) ? pTextItem->iTextSize : 0;

    CHECK(mandatory_break_text_clean(text, bytes));
    mandatory_break_draws++;
    return decoration_spans(proxy, draw, pTextItem, rect, color, flags, spans, count);
}
static void document_unicode_mandatory_breaks(xui_surface target, xui_test_proxy_state_t* proxy)
{
    xui_context context; xui_font font; unsigned sample, variant, pass;
    const uint32_t color = XUI_COLOR_RGBA(20, 40, 180, 255);
    decoration_base_shape = proxy->tProxy.textShape; decoration_base_metrics = proxy->tProxy.fontGetMetrics;
    decoration_base_line = proxy->tProxy.drawLine; decoration_base_text = proxy->tProxy.drawText;
    decoration_base_spans = proxy->tProxy.drawTextSpans;
    proxy->tProxy.textShape = decoration_shape; proxy->tProxy.fontGetMetrics = decoration_metrics;
    proxy->tProxy.drawLine = decoration_line; proxy->tProxy.drawText = mandatory_break_draw_text;
    proxy->tProxy.drawTextSpans = mandatory_break_draw_spans;
    decoration_invalid_metrics = decoration_metrics_error = decoration_line_error = 0;
    CHECK(proxy->tProxy.fontLoadFile(&proxy->tProxy, &font, "mandatory.ttf", 20, 0) == XUI_OK &&
        xuiCreate(&context) == XUI_OK && xuiSetProxy(context, &proxy->tProxy) == XUI_OK &&
        xuiSetDefaultFont(context, font) == XUI_OK);
    for (sample = 0; sample < sizeof(document_mandatory_breaks) / sizeof(*document_mandatory_breaks); sample++) {
        xui_document document[2]; xui_document_renderer renderer[2]; uint64_t first[2], last[2];
        xui_doc_renderer_desc_t desc = {0}; char joined[24]; size_t length = strlen(document_mandatory_breaks[sample]);
        snprintf(joined, sizeof(joined), "AV%sAV", document_mandatory_breaks[sample]);
        desc.iSize = sizeof(desc); desc.fLineGap = 4;
        for (variant = 0; variant < 2; variant++) {
            xui_document_transaction transaction; xui_document_snapshot snapshot; uint64_t paragraph;
            CHECK(xuiDocumentCreate(NULL, &document[variant]) == XUI_OK &&
                xuiDocumentBeginTransaction(document[variant], NULL, &transaction) == XUI_OK);
            paragraph = decoration_insert(transaction, 1, NULL, 0, 0);
            first[variant] = decoration_insert(transaction, paragraph, variant ? "A" : joined, XUI_DOC_UNDERLINE, color);
            if (variant) {
                decoration_insert(transaction, paragraph, "V", XUI_DOC_UNDERLINE, color);
                if (sample == 2) {
                    decoration_insert(transaction, paragraph, "\r", XUI_DOC_UNDERLINE, color);
                    decoration_insert(transaction, paragraph, "\n", XUI_DOC_UNDERLINE, color);
                } else decoration_insert(transaction, paragraph, document_mandatory_breaks[sample], XUI_DOC_UNDERLINE, color);
                decoration_insert(transaction, paragraph, "A", XUI_DOC_UNDERLINE, color);
                last[variant] = decoration_insert(transaction, paragraph, "V", XUI_DOC_UNDERLINE, color);
            } else last[variant] = first[variant];
            CHECK(xuiDocumentTxnCommit(transaction, NULL) == XUI_OK); xuiDocumentTxnRelease(transaction);
            CHECK(xuiDocumentAcquireSnapshot(document[variant], &snapshot) == XUI_OK &&
                xuiDocumentRendererCreate(context, &desc, &renderer[variant]) == XUI_OK &&
                xuiDocumentRendererSetSnapshot(renderer[variant], snapshot, NULL) == XUI_OK);
            xuiDocumentSnapshotRelease(snapshot);
        }
        for (pass = 0; pass < 4; pass++) {
            xui_doc_rect_t caret[2], sizes[2], ranges[2][16]; uint64_t counts[2]; int exact;
            double width = pass == 1 ? 40 : pass == 2 ? 12 : 200; unsigned i;
            for (variant = 0; variant < 2; variant++) {
                xui_doc_range_t range;
                range.tAnchor = decoration_position(document[variant], first[variant], 0);
                range.tCaret = decoration_position(document[variant], last[variant], variant ? 1 : 4 + length);
                CHECK(xuiDocumentRendererLayout(renderer[variant], width, 0, 200) == XUI_OK &&
                    xuiDocumentRendererGetCaretRect(renderer[variant], &range.tCaret, &caret[variant]) == XUI_OK &&
                    xuiDocumentRendererGetSize(renderer[variant], &sizes[variant], &exact) == XUI_OK && exact &&
                    xuiDocumentRendererGetRangeRects(renderer[variant], &range, ranges[variant], 16, &counts[variant]) == XUI_OK);
                if (pass != 2) CHECK(fabs(caret[variant].x - 17) < .01 && fabs(caret[variant].y - 24) < .01);
            }
            CHECK(fabs(caret[0].x - caret[1].x) < .01 && fabs(caret[0].y - caret[1].y) < .01 &&
                fabs(sizes[0].width - sizes[1].width) < .01 && fabs(sizes[0].height - sizes[1].height) < .01 && counts[0] == counts[1]);
            for (i = 0; i < counts[0]; i++) CHECK(fabs(ranges[0][i].x - ranges[1][i].x) < .01 &&
                fabs(ranges[0][i].y - ranges[1][i].y) < .01 && fabs(ranges[0][i].width - ranges[1][i].width) < .01 &&
                fabs(ranges[0][i].height - ranges[1][i].height) < .01);
        }
        for (variant = 0; variant < 2; variant++) {
            xui_draw_context draw; decoration_line_count = decoration_span_count = 0;
            CHECK(proxy->tProxy.drawBegin(&proxy->tProxy, &draw, target) == XUI_OK &&
                xuiDocumentRendererDraw(renderer[variant], draw, 0, 0, (xui_rect_t){0, 0, 200, 200}, NULL, 0) == XUI_OK &&
                proxy->tProxy.drawEnd(&proxy->tProxy, draw) == XUI_OK && decoration_span_count == 2 &&
                decoration_line_count == 2 && decoration_has(0, 17, 18.5f, 1.5f, color) &&
                decoration_has(0, 17, 42.5f, 1.5f, color));
            xuiDocumentRendererRelease(renderer[variant]); xuiDocumentRelease(document[variant]);
        }
    }
    for (sample = 0; sample < sizeof(document_mandatory_breaks) / sizeof(*document_mandatory_breaks); sample++) {
        xui_document document; xui_document_transaction transaction; xui_document_snapshot snapshot;
        xui_document_renderer renderer; xui_doc_renderer_desc_t desc = {0}; xui_doc_node_desc_t node = {0};
        uint64_t paragraph, text_node; xui_doc_rect_t size; xui_draw_context draw; int exact; char controls[8];
        snprintf(controls, sizeof(controls), "%s%s", document_mandatory_breaks[sample], document_mandatory_breaks[sample]);
        CHECK(xuiDocumentCreate(NULL, &document) == XUI_OK &&
            xuiDocumentBeginTransaction(document, NULL, &transaction) == XUI_OK);
        node.iSize = sizeof(node); node.iKind = XUI_DOC_PARAGRAPH; node.tAttributes.iFlags = XUI_DOC_SPACING_EXPLICIT;
        CHECK(xuiDocumentTxnInsertNode(transaction, 1, XUI_DOCUMENT_APPEND, &node, &paragraph) == XUI_OK);
        text_node = decoration_insert(transaction, paragraph, controls, XUI_DOC_UNDERLINE | XUI_DOC_STRIKE, color);
        CHECK(text_node && xuiDocumentTxnCommit(transaction, NULL) == XUI_OK); xuiDocumentTxnRelease(transaction);
        desc.iSize = sizeof(desc); desc.fLineGap = 4;
        CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK &&
            xuiDocumentRendererCreate(context, &desc, &renderer) == XUI_OK &&
            xuiDocumentRendererSetSnapshot(renderer, snapshot, NULL) == XUI_OK &&
            xuiDocumentRendererLayout(renderer, 200, 0, 200) == XUI_OK &&
            xuiDocumentRendererGetSize(renderer, &size, &exact) == XUI_OK && exact && fabs(size.height - 68) < .01);
        xuiDocumentSnapshotRelease(snapshot); mandatory_break_draws = decoration_line_count = 0;
        {
            xui_doc_position_t at = decoration_position(document, text_node, strlen(controls)), hit;
            xui_doc_rect_t caret;
            CHECK(xuiDocumentRendererGetCaretRect(renderer, &at, &caret) == XUI_OK &&
                fabs(caret.x) < .01 && fabs(caret.y - 48) < .01 && fabs(caret.height - 20) < .01 &&
                xuiDocumentRendererHitTest(renderer, 5, 58, &hit) == XUI_OK &&
                hit.iNodeId == text_node && hit.iOffset == strlen(controls));
            at.iAffinity = XUI_DOC_AFTER;
            CHECK(xuiDocumentRendererGetCaretRect(renderer, &at, &caret) == XUI_OK && fabs(caret.y - 48) < .01);
        }
        CHECK(proxy->tProxy.drawBegin(&proxy->tProxy, &draw, target) == XUI_OK &&
            xuiDocumentRendererDraw(renderer, draw, 0, 0, (xui_rect_t){0, 0, 200, 200}, NULL, 0) == XUI_OK &&
            proxy->tProxy.drawEnd(&proxy->tProxy, draw) == XUI_OK && !mandatory_break_draws && !decoration_line_count);
        {
            xui_document_change_set changes; xui_doc_position_t at; xui_doc_rect_t caret;
            CHECK(xuiDocumentBeginTransaction(document, NULL, &transaction) == XUI_OK &&
                xuiDocumentTxnReplaceText(transaction, text_node, strlen(controls), strlen(controls), "AV", 2) == XUI_OK &&
                xuiDocumentTxnCommit(transaction, &changes) == XUI_OK); xuiDocumentTxnRelease(transaction);
            CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK &&
                xuiDocumentRendererSetSnapshot(renderer, snapshot, changes) == XUI_OK &&
                xuiDocumentRendererLayout(renderer, 200, 0, 200) == XUI_OK &&
                xuiDocumentRendererGetSize(renderer, &size, &exact) == XUI_OK && exact && fabs(size.height - 68) < .01);
            xuiDocumentSnapshotRelease(snapshot); xuiDocumentChangeSetRelease(changes);
            at = decoration_position(document, text_node, strlen(controls) + 2);
            CHECK(xuiDocumentRendererGetCaretRect(renderer, &at, &caret) == XUI_OK &&
                fabs(caret.x - 17) < .01 && fabs(caret.y - 48) < .01);
            mandatory_break_draws = decoration_span_count = decoration_line_count = 0;
            CHECK(proxy->tProxy.drawBegin(&proxy->tProxy, &draw, target) == XUI_OK &&
                xuiDocumentRendererDraw(renderer, draw, 0, 0, (xui_rect_t){0, 0, 200, 200}, NULL, 0) == XUI_OK &&
                proxy->tProxy.drawEnd(&proxy->tProxy, draw) == XUI_OK && mandatory_break_draws == 1 &&
                decoration_span_count == 1 && decoration_line_count == 2 &&
                decoration_has(0, 17, 66.5f, 1.5f, color));
        }
        xuiDocumentRendererRelease(renderer); xuiDocumentRelease(document);
    }
    for (sample = 0; sample < sizeof(document_mandatory_breaks) / sizeof(*document_mandatory_breaks); sample++)
    for (variant = 0; variant < 2; variant++) for (pass = 0; pass < 2; pass++) {
        xui_document document; xui_document_snapshot snapshot; xui_document_renderer renderer;
        xui_doc_renderer_desc_t desc = {0}; xui_doc_desc_t profile = {0}; uint64_t node = 1;
        xui_doc_position_t at, hit; xui_doc_rect_t caret; xui_draw_context draw;
        size_t control = strlen(document_mandatory_breaks[sample]), count = pass ? 4095 : 2;
        size_t bytes = count * 2 + control * 2 + 1; char* text = malloc(bytes + 1);
        CHECK(text); memset(text, 'A', count); memcpy(text + count, document_mandatory_breaks[sample], control);
        memset(text + count + control, 'A', count); memcpy(text + count * 2 + control, document_mandatory_breaks[sample], control);
        text[bytes - 1] = 'V'; text[bytes] = 0;
        profile.iSize = sizeof(profile); profile.iProfile = variant ? XUI_DOCUMENT_MARKDOWN : XUI_DOCUMENT_RICH;
        CHECK(xuiDocumentCreate(&profile, &document) == XUI_OK);
        if (variant) CHECK(xuiDocumentLoadMarkdown(document, text, bytes) == XUI_OK);
        else {
            xui_document_transaction transaction; xui_doc_node_desc_t code = {0};
            code.iSize = sizeof(code); code.iKind = XUI_DOC_CODE_BLOCK; code.sText = text; code.iTextBytes = bytes;
            CHECK(xuiDocumentBeginTransaction(document, NULL, &transaction) == XUI_OK &&
                xuiDocumentTxnInsertNode(transaction, 1, XUI_DOCUMENT_APPEND, &code, &node) == XUI_OK &&
                xuiDocumentTxnCommit(transaction, NULL) == XUI_OK); xuiDocumentTxnRelease(transaction);
        }
        desc.iSize = sizeof(desc); desc.fLineGap = 4;
        CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK &&
            xuiDocumentRendererCreate(context, &desc, &renderer) == XUI_OK &&
            xuiDocumentRendererSetSnapshot(renderer, snapshot, NULL) == XUI_OK &&
            (!variant || xuiDocumentRendererSetMode(renderer, XUI_DOC_SOURCE_TEXT) == XUI_OK) &&
            xuiDocumentRendererLayout(renderer, 200, 0, 200) == XUI_OK);
        xuiDocumentSnapshotRelease(snapshot);
        at = decoration_position(document, node, bytes); if (variant) at.iKind = XUI_DOC_POSITION_SOURCE;
        CHECK(xuiDocumentRendererGetCaretRect(renderer, &at, &caret) == XUI_OK &&
            fabs(caret.x - (variant ? 10 : 18)) < .01 && fabs(caret.y - (variant ? 40 : 48)) < .01 &&
            xuiDocumentRendererHitTest(renderer, caret.x - 1, caret.y + 10, &hit) == XUI_OK &&
            hit.iNodeId == at.iNodeId && hit.iOffset == bytes);
        CHECK(proxy->tProxy.drawBegin(&proxy->tProxy, &draw, target) == XUI_OK &&
            xuiDocumentRendererDraw(renderer, draw, 0, 0, (xui_rect_t){0, 0, 200, 200}, NULL, 0) == XUI_OK &&
            proxy->tProxy.drawEnd(&proxy->tProxy, draw) == XUI_OK);
        xuiDocumentRendererRelease(renderer); xuiDocumentRelease(document); free(text);
    }
    /* A logical EOF after a mandatory control is an editable blank row.
     * A code slice ending at 8 KiB must not add another row at interior cuts. */
    for (sample = 0; sample < sizeof(document_mandatory_breaks) / sizeof(*document_mandatory_breaks); sample++)
    for (variant = 0; variant < 4; variant++) for (pass = 0; pass < 2; pass++) {
        xui_document document; xui_document_snapshot snapshot; xui_document_renderer renderer;
        xui_doc_desc_t profile = {0}; xui_doc_renderer_desc_t desc = {0};
        xui_doc_position_t at, hit; xui_doc_rect_t caret, size; int exact; unsigned resize, affinity;
        uint64_t node_id = 1; size_t length = strlen(document_mandatory_breaks[sample]);
        size_t prefix = variant >= 2 && pass ? 8191 : 2, bytes = prefix + length;
        char* text = malloc(bytes + 1); CHECK(text);
        memset(text, 'A', prefix); text[prefix - 1] = 'V';
        memcpy(text + prefix, document_mandatory_breaks[sample], length + 1);
        profile.iSize = sizeof(profile); profile.iProfile = variant == 3 ? XUI_DOCUMENT_MARKDOWN : XUI_DOCUMENT_RICH;
        CHECK(xuiDocumentCreate(&profile, &document) == XUI_OK);
        if (variant == 3) CHECK(xuiDocumentLoadMarkdown(document, text, bytes) == XUI_OK);
        else {
            xui_document_transaction transaction; xui_doc_node_desc_t node = {0}; uint64_t paragraph;
            CHECK(xuiDocumentBeginTransaction(document, NULL, &transaction) == XUI_OK);
            node.iSize = sizeof(node); node.iKind = variant == 2 ? XUI_DOC_CODE_BLOCK : XUI_DOC_PARAGRAPH;
            node.tAttributes.iFlags = XUI_DOC_SPACING_EXPLICIT;
            if (variant == 2) { node.sText = text; node.iTextBytes = bytes; }
            CHECK(xuiDocumentTxnInsertNode(transaction, 1, XUI_DOCUMENT_APPEND, &node, &paragraph) == XUI_OK);
            if (variant == 2) node_id = paragraph;
            else {
                if (!variant) node_id = decoration_insert(transaction, paragraph, text, XUI_DOC_UNDERLINE, color);
                else {
                    decoration_insert(transaction, paragraph, "AV", XUI_DOC_UNDERLINE, color);
                    if (sample == 2) decoration_insert(transaction, paragraph, "\r", XUI_DOC_UNDERLINE, color);
                    node_id = decoration_insert(transaction, paragraph, sample == 2 ? "\n" :
                        document_mandatory_breaks[sample], XUI_DOC_STRIKE, color);
                    bytes = sample == 2 ? 1 : length;
                }
            }
            CHECK(xuiDocumentTxnCommit(transaction, NULL) == XUI_OK); xuiDocumentTxnRelease(transaction);
        }
        desc.iSize = sizeof(desc); desc.fLineGap = 4;
        CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK &&
            xuiDocumentRendererCreate(context, &desc, &renderer) == XUI_OK &&
            xuiDocumentRendererSetSnapshot(renderer, snapshot, NULL) == XUI_OK &&
            (variant != 3 || xuiDocumentRendererSetMode(renderer, XUI_DOC_SOURCE_TEXT) == XUI_OK));
        xuiDocumentSnapshotRelease(snapshot);
        at = decoration_position(document, node_id, bytes);
        if (variant == 3) at.iKind = XUI_DOC_POSITION_SOURCE;
        for (resize = 0; resize < 3; resize++) {
            double width = resize == 1 ? 12 : 200;
            double rows = variant < 2 && resize == 1 ? 3 : 2;
            double gap = variant == 3 ? 0 : 4;
            CHECK(xuiDocumentRendererLayout(renderer, width, 0, 200) == XUI_OK &&
                xuiDocumentRendererGetSize(renderer, &size, &exact) == XUI_OK && exact &&
                fabs(size.height - (rows * 20 + (rows - 1) * gap)) < .01);
            for (affinity = XUI_DOC_BEFORE; affinity <= XUI_DOC_AFTER; affinity++) {
                at.iAffinity = affinity;
                CHECK(xuiDocumentRendererGetCaretRect(renderer, &at, &caret) == XUI_OK &&
                    fabs(caret.x - (variant == 2 ? 8 : 0)) < .01 &&
                    fabs(caret.y - ((rows - 1) * (20 + gap))) < .01 && fabs(caret.height - 20) < .01 &&
                    xuiDocumentRendererHitTest(renderer, caret.x + 5, caret.y + 10, &hit) == XUI_OK &&
                    hit.iNodeId == at.iNodeId && hit.iOffset == at.iOffset);
            }
        }
        xuiDocumentRendererRelease(renderer); xuiDocumentRelease(document); free(text);
    }
    /* Structural HardBreak's blank row must hit a parent gap where input can
     * create a Text sibling, rather than a childless break node. */
    {
        xui_document document; xui_document_transaction transaction; xui_document_snapshot snapshot;
        xui_document_renderer renderer; xui_doc_node_desc_t node = {0}; xui_doc_renderer_desc_t desc = {0};
        uint64_t paragraph, hard; xui_doc_position_t at, hit; xui_doc_rect_t caret, size; int exact;
        CHECK(xuiDocumentCreate(NULL, &document) == XUI_OK &&
            xuiDocumentBeginTransaction(document, NULL, &transaction) == XUI_OK);
        node.iSize = sizeof(node); node.iKind = XUI_DOC_PARAGRAPH; node.tAttributes.iFlags = XUI_DOC_SPACING_EXPLICIT;
        CHECK(xuiDocumentTxnInsertNode(transaction, 1, XUI_DOCUMENT_APPEND, &node, &paragraph) == XUI_OK);
        decoration_insert(transaction, paragraph, "AV", XUI_DOC_UNDERLINE, color);
        node.iKind = XUI_DOC_HARD_BREAK;
        CHECK(xuiDocumentTxnInsertNode(transaction, paragraph, XUI_DOCUMENT_APPEND, &node, &hard) == XUI_OK &&
            xuiDocumentTxnCommit(transaction, NULL) == XUI_OK); xuiDocumentTxnRelease(transaction);
        desc.iSize = sizeof(desc); desc.fLineGap = 4;
        CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK &&
            xuiDocumentRendererCreate(context, &desc, &renderer) == XUI_OK &&
            xuiDocumentRendererSetSnapshot(renderer, snapshot, NULL) == XUI_OK &&
            xuiDocumentRendererLayout(renderer, 200, 0, 200) == XUI_OK &&
            xuiDocumentRendererGetSize(renderer, &size, &exact) == XUI_OK && exact && fabs(size.height - 44) < .01);
        xuiDocumentSnapshotRelease(snapshot);
        at = decoration_position(document, paragraph, 2); at.iKind = XUI_DOC_POSITION_GAP;
        CHECK(xuiDocumentRendererGetCaretRect(renderer, &at, &caret) == XUI_OK &&
            fabs(caret.x) < .01 && fabs(caret.y - 24) < .01 &&
            xuiDocumentRendererHitTest(renderer, 5, 34, &hit) == XUI_OK &&
            hit.iKind == XUI_DOC_POSITION_GAP && hit.iNodeId == paragraph && hit.iOffset == 2);
        at.iOffset = 1;
        CHECK(xuiDocumentRendererGetCaretRect(renderer, &at, &caret) == XUI_OK && fabs(caret.y) < .01 && fabs(caret.x - 17) < .01);
        xuiDocumentRendererRelease(renderer); xuiDocumentRelease(document);
    }
    xuiDestroy(context); proxy->tProxy.fontDestroy(&proxy->tProxy, font);
    proxy->tProxy.textShape = decoration_base_shape; proxy->tProxy.fontGetMetrics = decoration_base_metrics;
    proxy->tProxy.drawLine = decoration_base_line; proxy->tProxy.drawText = decoration_base_text;
    proxy->tProxy.drawTextSpans = decoration_base_spans;
    puts("Unicode mandatory Document controls: LF/CR/CRLF/VT/FF/NEL/LS/PS, color splits, wraps, ranges, clean draw, code/SOURCE and UTF-8/CRLF chunk edges passed");
    puts("Document trailing empty lines: eight controls, affinities, split CRLF, Rich/code/SOURCE reflow, code slice EOF and editable HardBreak gap passed");
}
