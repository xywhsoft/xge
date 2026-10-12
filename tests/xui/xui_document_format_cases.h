static xui_draw_text_proc document_format_base_draw;
static xui_draw_text_spans_proc document_format_base_spans;
static unsigned document_format_hyphens;
static unsigned document_format_fail_hyphen;
static int document_format_hyphen_kerning;
static int document_format_hyphen_narrow;
static unsigned document_format_fail_terminal, document_format_terminal_shapes;
static const char* const document_formats[] = {"\xc2\xad", "\xe2\x80\x8b", "\xe2\x81\xa0", "\xef\xbb\xbf"};
static int document_format_shape(xui_proxy proxy, const xui_text_item_t* pTextItem, xui_text_shape_t* shape)
{
    const char* text = pTextItem && pTextItem->iSize >= sizeof(*pTextItem) ? pTextItem->sText : NULL;
    int bytes = pTextItem && pTextItem->iSize >= sizeof(*pTextItem) ? pTextItem->iTextSize : 0;

    if (document_format_fail_hyphen && bytes == 1 && text[0] == '-') {
        document_format_fail_hyphen--; return XUI_ERROR_OUT_OF_MEMORY;
    }
    if (bytes > 1 && text[bytes - 1] == '-') {
        document_format_terminal_shapes++;
        if (document_format_fail_terminal && !--document_format_fail_terminal)
            return XUI_ERROR_OUT_OF_MEMORY;
    }
    {
        int result = decoration_shape(proxy, pTextItem, shape);
        if (result == XUI_OK && (document_format_hyphen_kerning || document_format_hyphen_narrow)) {
            int i;
            if (document_format_hyphen_narrow) for (i = 0; i < shape->iClusterCount; i++) {
                if (text[shape->pClusters[i].iTextStart] == '-') {
                    shape->fWidth += 2 - shape->pClusters[i].fAdvance;
                    shape->pClusters[i].fAdvance = 2;
                }
            }
            for (i = 0; i + 1 < shape->iClusterCount; i++) {
                if (text[shape->pClusters[i].iTextStart] == 'V' &&
                    text[shape->pClusters[i + 1].iTextStart] == '-') {
                    shape->pClusters[i].fAdvance += document_format_hyphen_kerning;
                    shape->fWidth += document_format_hyphen_kerning;
                }
            }
        }
        return result;
    }
}
static void document_format_paint_check(const char* text, int bytes)
{
    unsigned sample; int i;
    if (bytes < 0) bytes = (int)strlen(text);
    for (sample = 0; sample < 4; sample++) {
        int length = (int)strlen(document_formats[sample]);
        for (i = 0; i + length <= bytes; i++) CHECK(memcmp(text + i, document_formats[sample], (size_t)length));
    }
    for (i = 0; i < bytes; i++) if (text[i] == '-') document_format_hyphens++;
}
static int document_format_draw(xui_proxy proxy, xui_draw_context draw, const xui_text_item_t* pTextItem, xui_rect_t rect, uint32_t color, uint32_t flags)
{
    const char* text = pTextItem && pTextItem->iSize >= sizeof(*pTextItem) ? pTextItem->sText : NULL;

    document_format_paint_check(text, -1);
    return document_format_base_draw(proxy, draw, pTextItem, rect, color, flags);
}
static int document_format_spans(xui_proxy proxy, xui_draw_context draw, const xui_text_item_t* pTextItem, xui_rect_t rect, uint32_t color, uint32_t flags, const xui_text_paint_span_t* spans, int count)
{
    const char* text = pTextItem && pTextItem->iSize >= sizeof(*pTextItem) ? pTextItem->sText : NULL;
    int bytes = pTextItem && pTextItem->iSize >= sizeof(*pTextItem) ? pTextItem->iTextSize : 0;

    document_format_paint_check(text, bytes);
    return document_format_base_spans(proxy, draw, pTextItem, rect, color, flags, spans, count);
}
static void document_format_controls(xui_test_proxy_state_t* proxy)
{
    xui_context context; xui_font font; xui_surface surface; xui_text_shape_proc saved = proxy->tProxy.textShape;
    unsigned sample, mode, split;
    decoration_base_shape = saved; proxy->tProxy.textShape = document_format_shape;
    document_format_base_draw = proxy->tProxy.drawText; proxy->tProxy.drawText = document_format_draw;
    document_format_base_spans = proxy->tProxy.drawTextSpans; proxy->tProxy.drawTextSpans = document_format_spans;
    CHECK(proxy->tProxy.fontLoadFile(&proxy->tProxy, &font, "format.ttf", 20, 0) == XUI_OK &&
        xuiCreate(&context) == XUI_OK && xuiSetProxy(context, &proxy->tProxy) == XUI_OK && xuiSetDefaultFont(context, font) == XUI_OK &&
        xuiTestSurfaceCreate(proxy, &surface, 320, 200, XUI_SURFACE_USAGE_TARGET) == XUI_OK);
    for (sample = 0; sample < 4; sample++) for (mode = 0; mode < 4; mode++) for (split = 0; split < (mode ? 1u : 2u); split++) {
        xui_document document; xui_document_snapshot snapshot; xui_document_renderer renderer;
        xui_doc_desc_t profile = {0}; xui_doc_renderer_desc_t desc = {0}; uint64_t last = 1; char text[16];
        xui_doc_position_t at; xui_doc_rect_t caret; xui_draw_context draw;
        snprintf(text, sizeof(text), "A%sV", document_formats[sample]);
        profile.iSize = sizeof(profile); profile.iProfile = mode >= 2 ? XUI_DOCUMENT_MARKDOWN : XUI_DOCUMENT_RICH;
        CHECK(xuiDocumentCreate(&profile, &document) == XUI_OK);
        if (mode >= 2) CHECK(xuiDocumentLoadMarkdown(document, text, strlen(text)) == XUI_OK);
        else {
            xui_document_transaction transaction; xui_doc_node_desc_t node = {0}; uint64_t paragraph;
            CHECK(xuiDocumentBeginTransaction(document, NULL, &transaction) == XUI_OK);
            node.iSize = sizeof(node); node.iKind = mode ? XUI_DOC_CODE_BLOCK : XUI_DOC_PARAGRAPH;
            if (mode) { node.sText = text; node.iTextBytes = strlen(text); }
            CHECK(xuiDocumentTxnInsertNode(transaction, 1, XUI_DOCUMENT_APPEND, &node, &paragraph) == XUI_OK);
            if (mode) last = paragraph;
            else {
                if (split) {
                    decoration_insert(transaction, paragraph, "A", 0, 0);
                    decoration_insert(transaction, paragraph, document_formats[sample], 0, XUI_COLOR_RGBA(180, 20, 40, 255));
                }
                last = decoration_insert(transaction, paragraph, split ? "V" : text, XUI_DOC_UNDERLINE, 0);
            }
            CHECK(xuiDocumentTxnCommit(transaction, NULL) == XUI_OK); xuiDocumentTxnRelease(transaction);
        }
        desc.iSize = sizeof(desc); desc.fLineGap = 4;
        CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
        if (mode == 3) {
            uint64_t paragraph;
            CHECK(xuiDocumentSnapshotGetChild(snapshot, 1, 0, &paragraph) == XUI_OK && xuiDocumentSnapshotGetChild(snapshot, paragraph, 0, &last) == XUI_OK);
        }
        CHECK(xuiDocumentRendererCreate(context, &desc, &renderer) == XUI_OK &&
            xuiDocumentRendererSetSnapshot(renderer, snapshot, NULL) == XUI_OK &&
            (mode != 2 || xuiDocumentRendererSetMode(renderer, XUI_DOC_SOURCE_TEXT) == XUI_OK) &&
            xuiDocumentRendererLayout(renderer, 200, 0, 200) == XUI_OK); xuiDocumentSnapshotRelease(snapshot);
        at = decoration_position(document, last, split ? 1 : strlen(text));
        if (mode == 2) at.iKind = XUI_DOC_POSITION_SOURCE;
        CHECK(xuiDocumentRendererGetCaretRect(renderer, &at, &caret) == XUI_OK && fabs(caret.x - (mode == 1 ? 25 : 17)) < .01 && fabs(caret.y) < .01);
        CHECK(proxy->tProxy.drawBegin(&proxy->tProxy, &draw, surface) == XUI_OK &&
            xuiDocumentRendererDraw(renderer, draw, 0, 0, (xui_rect_t){0, 0, 320, 200}, NULL, 0) == XUI_OK && proxy->tProxy.drawEnd(&proxy->tProxy, draw) == XUI_OK);
        {
            char copied[32]; uint64_t bytes;
            CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
            if (mode >= 2) CHECK(xuiDocumentSnapshotCopySource(snapshot, copied, sizeof(copied), &bytes) == XUI_OK &&
                bytes == strlen(text) && !memcmp(copied, text, (size_t)bytes));
            else CHECK(xuiDocumentSnapshotCopyText(snapshot, last, copied, sizeof(copied), &bytes) == XUI_OK &&
                bytes == strlen(split ? "V" : text) && !memcmp(copied, split ? "V" : text, (size_t)bytes));
            xuiDocumentSnapshotRelease(snapshot);
        }
        xuiDocumentRendererRelease(renderer); xuiDocumentRelease(document);
    }
    for (split = 0; split < 2; split++) {
        xui_document document; xui_document_transaction transaction; xui_document_snapshot snapshot;
        xui_document_renderer renderer; xui_doc_renderer_desc_t desc = {0}; uint64_t paragraph, control; unsigned pass;
        CHECK(xuiDocumentCreate(NULL, &document) == XUI_OK && xuiDocumentBeginTransaction(document, NULL, &transaction) == XUI_OK);
        paragraph = decoration_insert(transaction, 1, NULL, 0, 0);
        if (split) {
            decoration_insert(transaction, paragraph, "AV", XUI_DOC_UNDERLINE, 0);
            control = decoration_insert(transaction, paragraph, "\xc2\xad", XUI_DOC_UNDERLINE, XUI_COLOR_RGBA(180, 20, 40, 255));
            decoration_insert(transaction, paragraph, "AV", XUI_DOC_UNDERLINE, 0);
        } else control = decoration_insert(transaction, paragraph, "AV\xc2\xad" "AV", XUI_DOC_UNDERLINE, 0);
        CHECK(xuiDocumentTxnCommit(transaction, NULL) == XUI_OK); xuiDocumentTxnRelease(transaction);
        desc.iSize = sizeof(desc); desc.fLineGap = 4;
        CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK && xuiDocumentRendererCreate(context, &desc, &renderer) == XUI_OK &&
            xuiDocumentRendererSetSnapshot(renderer, snapshot, NULL) == XUI_OK); xuiDocumentSnapshotRelease(snapshot);
        for (pass = 0; pass < 3; pass++) {
            xui_doc_position_t at = decoration_position(document, control, split ? 2 : 4), hit;
            xui_doc_rect_t before, after; xui_draw_context draw;
            CHECK(xuiDocumentRendererLayout(renderer, pass == 1 ? 27 : 100, 0, 200) == XUI_OK &&
                xuiDocumentRendererGetCaretRect(renderer, &at, &before) == XUI_OK);
            at.iAffinity = XUI_DOC_AFTER;
            CHECK(xuiDocumentRendererGetCaretRect(renderer, &at, &after) == XUI_OK);
            if (pass == 1) CHECK(fabs(before.x - 27) < .01 && fabs(before.y) < .01 && fabs(after.x) < .01 && fabs(after.y - 24) < .01 &&
                xuiDocumentRendererHitTest(renderer, 22, 10, &hit) == XUI_OK && hit.iNodeId == control && hit.iOffset == (split ? 2u : 4u) && hit.iAffinity == XUI_DOC_BEFORE);
            else CHECK(fabs(before.x - 17) < .01 && fabs(after.x - 17) < .01 && fabs(after.y) < .01);
            document_format_hyphens = 0;
            CHECK(proxy->tProxy.drawBegin(&proxy->tProxy, &draw, surface) == XUI_OK &&
                xuiDocumentRendererDraw(renderer, draw, 0, 0, (xui_rect_t){0, 0, 320, 200}, NULL, 0) == XUI_OK &&
                proxy->tProxy.drawEnd(&proxy->tProxy, draw) == XUI_OK && document_format_hyphens == (pass == 1 ? 1u : 0u));
        }
        xuiDocumentRendererRelease(renderer); xuiDocumentRelease(document);
    }
    /* A rejected isolated hyphen can fit in the actual terminal context.
     * The marker's source-node/color boundary must not lose that candidate. */
    for (sample = 0; sample < 6; sample++) for (split = 0; split < 3; split++) {
        xui_document document; xui_document_transaction transaction; xui_document_snapshot snapshot;
        xui_document_renderer renderer; xui_doc_renderer_desc_t desc = {0}; uint64_t paragraph, controls[2];
        xui_doc_desc_t profile = {0};
        xui_doc_position_t at, hit; xui_doc_rect_t before, after; xui_draw_context draw;
        unsigned part, count = sample >= 3 ? 2u : 1u;
        const double available[] = {25, 30, 14, 46, 42, 100};
        const char* source = count == 1 ? "AV\xc2\xad" "AV" : "AV\xc2\xad" "AV\xc2\xad" "AVAV";
        document_format_hyphen_kerning = sample == 1 || sample == 3 ? 3 : sample == 2 ? -5 : -2;
        document_format_hyphen_narrow = sample == 2;
        profile.iSize = sizeof(profile); profile.iProfile = split == 2 ? XUI_DOCUMENT_MARKDOWN : XUI_DOCUMENT_RICH;
        CHECK(xuiDocumentCreate(&profile, &document) == XUI_OK);
        if (split == 2) CHECK(xuiDocumentLoadMarkdown(document, source, strlen(source)) == XUI_OK);
        else {
            CHECK(xuiDocumentBeginTransaction(document, NULL, &transaction) == XUI_OK);
            paragraph = decoration_insert(transaction, 1, NULL, 0, 0);
            if (split == 1) {
                for (part = 0; part < count; part++) {
                    decoration_insert(transaction, paragraph, "AV", XUI_DOC_UNDERLINE, 0);
                    controls[part] = decoration_insert(transaction, paragraph, "\xc2\xad", XUI_DOC_UNDERLINE, XUI_COLOR_RGBA(180, 20, 40, 255));
                }
                decoration_insert(transaction, paragraph, count == 1 ? "AV" : "AVAV", XUI_DOC_UNDERLINE, 0);
            } else controls[0] = controls[1] = decoration_insert(transaction, paragraph, source, XUI_DOC_UNDERLINE, 0);
            CHECK(xuiDocumentTxnCommit(transaction, NULL) == XUI_OK); xuiDocumentTxnRelease(transaction);
        }
        desc.iSize = sizeof(desc); desc.fLineGap = 4;
        CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
        if (split == 2) {
            CHECK(xuiDocumentSnapshotGetChild(snapshot, 1, 0, &paragraph) == XUI_OK &&
                xuiDocumentSnapshotGetChild(snapshot, paragraph, 0, &controls[0]) == XUI_OK);
            controls[1] = controls[0];
        }
        CHECK(xuiDocumentRendererCreate(context, &desc, &renderer) == XUI_OK &&
            xuiDocumentRendererSetSnapshot(renderer, snapshot, NULL) == XUI_OK);
        xuiDocumentSnapshotRelease(snapshot); document_format_terminal_shapes = 0;
        CHECK(xuiDocumentRendererLayout(renderer, available[sample], 0, 200) == XUI_OK);
        for (part = 0; part < count; part++) {
            unsigned used = sample < 4 || (sample == 4 && part == 1);
            double x = sample < 3 ? available[sample] : sample == 3 ? 30 : sample == 4 && part == 1 ? 42 : (part + 1) * 17;
            double y = sample == 3 ? part * 24 : 0;
            at = decoration_position(document, controls[part], split == 1 ? 2 : (part + 1) * 4);
            CHECK(xuiDocumentRendererGetCaretRect(renderer, &at, &before) == XUI_OK);
            if (fabs(before.x - x) >= .01 || fabs(before.y - y) >= .01)
                fprintf(stderr, "Contextual hyphen sample=%u split=%u marker=%u: caret %.3f/%.3f, expected %.3f/%.3f\n", sample, split, part, before.x, before.y, x, y);
            CHECK(fabs(before.x - x) < .01 && fabs(before.y - y) < .01);
            at.iAffinity = XUI_DOC_AFTER;
            CHECK(xuiDocumentRendererGetCaretRect(renderer, &at, &after) == XUI_OK &&
                fabs(after.x - (used ? 0 : x)) < .01 && fabs(after.y - (used ? y + 24 : y)) < .01);
            if (used) CHECK(xuiDocumentRendererHitTest(renderer, x - 1, y + 10, &hit) == XUI_OK &&
                hit.iNodeId == controls[part] && hit.iOffset == (split == 1 ? 2u : (part + 1) * 4u) && hit.iAffinity == XUI_DOC_BEFORE);
        }
        if (sample == 5) CHECK(!document_format_terminal_shapes);
        document_format_hyphens = 0;
        CHECK(proxy->tProxy.drawBegin(&proxy->tProxy, &draw, surface) == XUI_OK &&
            xuiDocumentRendererDraw(renderer, draw, 0, 0, (xui_rect_t){0, 0, 320, 200}, NULL, 0) == XUI_OK &&
            proxy->tProxy.drawEnd(&proxy->tProxy, draw) == XUI_OK && document_format_hyphens == (sample == 5 ? 0u : sample == 3 ? 2u : 1u));
        if (split == 2) {
            char copied[24]; uint64_t bytes;
            CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK &&
                xuiDocumentSnapshotCopySource(snapshot, copied, sizeof(copied), &bytes) == XUI_OK &&
                bytes == strlen(source) && !memcmp(copied, source, (size_t)bytes));
            xuiDocumentSnapshotRelease(snapshot);
        }
        xuiDocumentRendererRelease(renderer); xuiDocumentRelease(document);
    }
    document_format_hyphen_kerning = 0;
    document_format_hyphen_narrow = 0;
    /* Hidden 40-font markers do not split the visible seed span, but each
     * selected marker adds its own 40-font hyphen span on a different row. */
    {
        xui_document document; xui_document_transaction transaction; xui_document_snapshot snapshot;
        xui_document_renderer renderer; xui_doc_renderer_desc_t desc = {0}; xui_doc_node_desc_t node = {0};
        uint64_t paragraph, controls[4], last; unsigned part; xui_draw_context draw;
        xui_doc_position_t at; xui_doc_rect_t caret;
        CHECK(xuiDocumentCreate(NULL, &document) == XUI_OK && xuiDocumentBeginTransaction(document, NULL, &transaction) == XUI_OK);
        paragraph = decoration_insert(transaction, 1, NULL, 0, 0);
        node.iSize = sizeof(node); node.iKind = XUI_DOC_TEXT; node.sText = "\xc2\xad"; node.iTextBytes = 2;
        node.tAttributes.fFontSize = 40; node.tAttributes.iMarks = XUI_DOC_UNDERLINE; node.tAttributes.iTextColor = XUI_COLOR_RGBA(180, 20, 40, 255);
        for (part = 0; part < 4; part++) {
            decoration_insert(transaction, paragraph, "AV", XUI_DOC_UNDERLINE, 0);
            CHECK(xuiDocumentTxnInsertNode(transaction, paragraph, XUI_DOCUMENT_APPEND, &node, &controls[part]) == XUI_OK);
        }
        last = decoration_insert(transaction, paragraph, "AVAV", XUI_DOC_UNDERLINE, 0);
        CHECK(xuiDocumentTxnCommit(transaction, NULL) == XUI_OK); xuiDocumentTxnRelease(transaction);
        desc.iSize = sizeof(desc); desc.fLineGap = 4;
        CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK && xuiDocumentRendererCreate(context, &desc, &renderer) == XUI_OK &&
            xuiDocumentRendererSetSnapshot(renderer, snapshot, NULL) == XUI_OK && xuiDocumentRendererLayout(renderer, 37, 0, 200) == XUI_OK);
        xuiDocumentSnapshotRelease(snapshot);
        for (part = 0; part < 4; part++) {
            at = decoration_position(document, controls[part], 2);
            CHECK(xuiDocumentRendererGetCaretRect(renderer, &at, &caret) == XUI_OK && fabs(caret.x - 37) < .01 &&
                fabs(caret.y - part * 44) < .01 && fabs(caret.height - 40) < .01);
        }
        at = decoration_position(document, last, 4);
        CHECK(xuiDocumentRendererGetCaretRect(renderer, &at, &caret) == XUI_OK && fabs(caret.x - 34) < .01 && fabs(caret.y - 176) < .01);
        document_format_hyphens = 0;
        CHECK(proxy->tProxy.drawBegin(&proxy->tProxy, &draw, surface) == XUI_OK &&
            xuiDocumentRendererDraw(renderer, draw, 0, 0, (xui_rect_t){0, 0, 320, 200}, NULL, 0) == XUI_OK &&
            proxy->tProxy.drawEnd(&proxy->tProxy, draw) == XUI_OK && document_format_hyphens == 4);
        xuiDocumentRendererRelease(renderer); xuiDocumentRelease(document);
    }
    /* The deletion projection can create a displayed grapheme. Split source
     * nodes must preserve its wrapping, range and interior caret geometry. */
    for (sample = 0; sample < 4; sample++) for (split = 0; split < 2; split++) {
        xui_document document; xui_document_transaction transaction; xui_document_snapshot snapshot;
        xui_document_renderer renderer; xui_doc_renderer_desc_t desc = {0}; uint64_t paragraph, interior, last; char text[24];
        xui_doc_position_t at; xui_doc_rect_t head, tail; unsigned pass;
        CHECK(xuiDocumentCreate(NULL, &document) == XUI_OK && xuiDocumentBeginTransaction(document, NULL, &transaction) == XUI_OK);
        paragraph = decoration_insert(transaction, 1, NULL, 0, 0);
        if (split) {
            decoration_insert(transaction, paragraph, "e", 0, 0);
            interior = decoration_insert(transaction, paragraph, document_formats[sample], 0, XUI_COLOR_RGBA(180, 20, 40, 255));
            last = decoration_insert(transaction, paragraph, "\xcc\x81", 0, 0);
        } else {
            snprintf(text, sizeof(text), "e%s\xcc\x81", document_formats[sample]);
            interior = last = decoration_insert(transaction, paragraph, text, 0, 0);
        }
        CHECK(xuiDocumentTxnCommit(transaction, NULL) == XUI_OK); xuiDocumentTxnRelease(transaction);
        desc.iSize = sizeof(desc); desc.fLineGap = 4;
        CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK && xuiDocumentRendererCreate(context, &desc, &renderer) == XUI_OK &&
            xuiDocumentRendererSetSnapshot(renderer, snapshot, NULL) == XUI_OK); xuiDocumentSnapshotRelease(snapshot);
        for (pass = 0; pass < 2; pass++) {
            CHECK(xuiDocumentRendererLayout(renderer, pass ? 7 : 100, 0, 200) == XUI_OK);
            at = decoration_position(document, interior, split ? 0 : 1);
            CHECK(xuiDocumentRendererGetCaretRect(renderer, &at, &head) == XUI_OK); at.iAffinity = XUI_DOC_AFTER;
            CHECK(xuiDocumentRendererGetCaretRect(renderer, &at, &tail) == XUI_OK && fabs(head.x) < .01 && fabs(head.y) < .01 &&
                fabs(tail.x - 20) < .01 && fabs(tail.y) < .01);
        }
        xuiDocumentRendererRelease(renderer); xuiDocumentRelease(document);
    }
    for (sample = 1; sample < 4; sample++) {
        xui_document document; xui_document_transaction transaction; xui_document_snapshot snapshot;
        xui_document_renderer renderer; xui_doc_renderer_desc_t desc = {0}; uint64_t paragraph, last; char text[24];
        xui_doc_position_t at; xui_doc_rect_t caret;
        snprintf(text, sizeof(text), "\xe4\xb8\xad%s\xe6\x96\x87", document_formats[sample]);
        CHECK(xuiDocumentCreate(NULL, &document) == XUI_OK && xuiDocumentBeginTransaction(document, NULL, &transaction) == XUI_OK);
        paragraph = decoration_insert(transaction, 1, NULL, 0, 0); last = decoration_insert(transaction, paragraph, text, 0, 0);
        CHECK(xuiDocumentTxnCommit(transaction, NULL) == XUI_OK); xuiDocumentTxnRelease(transaction);
        desc.iSize = sizeof(desc); desc.fLineGap = 4;
        CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK && xuiDocumentRendererCreate(context, &desc, &renderer) == XUI_OK &&
            xuiDocumentRendererSetSnapshot(renderer, snapshot, NULL) == XUI_OK && xuiDocumentRendererLayout(renderer, 10, 0, 200) == XUI_OK);
        xuiDocumentSnapshotRelease(snapshot); at = decoration_position(document, last, strlen(text));
        CHECK(xuiDocumentRendererGetCaretRect(renderer, &at, &caret) == XUI_OK &&
            fabs(caret.x - (sample == 1 ? 10 : 20)) < .01 && fabs(caret.y - (sample == 1 ? 24 : 0)) < .01);
        xuiDocumentRendererRelease(renderer); xuiDocumentRelease(document);
    }
    /* DrawText-only hosts use the same deletion projection inside a run. */
    {
        xui_context fallback; xui_proxy_t callbacks = proxy->tProxy;
        xui_document document; xui_document_transaction transaction; xui_document_snapshot snapshot;
        xui_document_renderer renderer; xui_doc_renderer_desc_t desc = {0}; uint64_t paragraph, last; xui_draw_context draw;
        xui_doc_position_t at; xui_doc_rect_t caret;
        callbacks.drawTextSpans = NULL;
        CHECK(xuiCreate(&fallback) == XUI_OK && xuiSetProxy(fallback, &callbacks) == XUI_OK && xuiSetDefaultFont(fallback, font) == XUI_OK);
        CHECK(xuiDocumentCreate(NULL, &document) == XUI_OK && xuiDocumentBeginTransaction(document, NULL, &transaction) == XUI_OK);
        paragraph = decoration_insert(transaction, 1, NULL, 0, 0); last = decoration_insert(transaction, paragraph, "A\xc2\xadV", XUI_DOC_UNDERLINE, 0);
        CHECK(xuiDocumentTxnCommit(transaction, NULL) == XUI_OK); xuiDocumentTxnRelease(transaction);
        desc.iSize = sizeof(desc); desc.fLineGap = 4;
        CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK && xuiDocumentRendererCreate(fallback, &desc, &renderer) == XUI_OK &&
            xuiDocumentRendererSetSnapshot(renderer, snapshot, NULL) == XUI_OK && xuiDocumentRendererLayout(renderer, 100, 0, 200) == XUI_OK);
        xuiDocumentSnapshotRelease(snapshot); at = decoration_position(document, last, 4);
        CHECK(xuiDocumentRendererGetCaretRect(renderer, &at, &caret) == XUI_OK && fabs(caret.x - 17) < .01 &&
            proxy->tProxy.drawBegin(&proxy->tProxy, &draw, surface) == XUI_OK &&
            xuiDocumentRendererDraw(renderer, draw, 0, 0, (xui_rect_t){0, 0, 320, 200}, NULL, 0) == XUI_OK && proxy->tProxy.drawEnd(&proxy->tProxy, draw) == XUI_OK);
        xuiDocumentRendererRelease(renderer); xuiDocumentRelease(document);
        xuiDestroy(fallback);
    }
    {
        xui_document document; xui_document_transaction transaction; xui_document_snapshot snapshot;
        xui_document_renderer renderer; xui_doc_renderer_desc_t desc = {0}; uint64_t paragraph, last;
        xui_doc_position_t at; xui_doc_rect_t caret;
        CHECK(xuiDocumentCreate(NULL, &document) == XUI_OK && xuiDocumentBeginTransaction(document, NULL, &transaction) == XUI_OK);
        paragraph = decoration_insert(transaction, 1, NULL, 0, 0); last = decoration_insert(transaction, paragraph, "AV\xc2\xad" "AV", 0, 0);
        CHECK(xuiDocumentTxnCommit(transaction, NULL) == XUI_OK); xuiDocumentTxnRelease(transaction);
        desc.iSize = sizeof(desc); desc.fLineGap = 4;
        CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK && xuiDocumentRendererCreate(context, &desc, &renderer) == XUI_OK &&
            xuiDocumentRendererSetSnapshot(renderer, snapshot, NULL) == XUI_OK); xuiDocumentSnapshotRelease(snapshot);
        document_format_fail_hyphen = 1;
        CHECK(xuiDocumentRendererLayout(renderer, 27, 0, 200) == XUI_ERROR_OUT_OF_MEMORY && !document_format_fail_hyphen &&
            xuiDocumentRendererLayout(renderer, 27, 0, 200) == XUI_OK);
        at = decoration_position(document, last, 4);
        CHECK(xuiDocumentRendererGetCaretRect(renderer, &at, &caret) == XUI_OK && fabs(caret.x - 27) < .01 && fabs(caret.y) < .01);
        xuiDocumentRendererRelease(renderer); xuiDocumentRelease(document);
    }
    /* Candidate, pending paint and convergence checks are all fallible. None
     * may publish partial widths or retain a failed temporary paint string. */
    document_format_hyphen_kerning = -2;
    for (sample = 1; sample <= 3; sample++) {
        xui_document document; xui_document_transaction transaction; xui_document_snapshot snapshot;
        xui_document_renderer renderer; xui_doc_renderer_desc_t desc = {0}; uint64_t paragraph, last;
        xui_doc_position_t at; xui_doc_rect_t caret;
        CHECK(xuiDocumentCreate(NULL, &document) == XUI_OK && xuiDocumentBeginTransaction(document, NULL, &transaction) == XUI_OK);
        paragraph = decoration_insert(transaction, 1, NULL, 0, 0); last = decoration_insert(transaction, paragraph, "AV\xc2\xad" "AV", 0, 0);
        CHECK(xuiDocumentTxnCommit(transaction, NULL) == XUI_OK); xuiDocumentTxnRelease(transaction);
        desc.iSize = sizeof(desc); desc.fLineGap = 4;
        CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK && xuiDocumentRendererCreate(context, &desc, &renderer) == XUI_OK &&
            xuiDocumentRendererSetSnapshot(renderer, snapshot, NULL) == XUI_OK && xuiDocumentRendererLayout(renderer, 100, 0, 200) == XUI_OK);
        xuiDocumentSnapshotRelease(snapshot); document_format_fail_terminal = sample;
        CHECK(xuiDocumentRendererLayout(renderer, 25, 0, 200) == XUI_ERROR_OUT_OF_MEMORY && !document_format_fail_terminal &&
            xuiDocumentRendererLayout(renderer, 25, 0, 200) == XUI_OK);
        at = decoration_position(document, last, 4);
        CHECK(xuiDocumentRendererGetCaretRect(renderer, &at, &caret) == XUI_OK && fabs(caret.x - 25) < .01 && fabs(caret.y) < .01);
        xuiDocumentRendererRelease(renderer); xuiDocumentRelease(document);
    }
    document_format_hyphen_kerning = 0;
    proxy->tProxy.surfaceDestroy(&proxy->tProxy, surface); xuiDestroy(context); proxy->tProxy.fontDestroy(&proxy->tProxy, font);
    proxy->tProxy.textShape = saved; proxy->tProxy.drawText = document_format_base_draw; proxy->tProxy.drawTextSpans = document_format_base_spans;
    puts("Document invisible formats: SHY/ZWSP/WJ/FEFF in Rich joined/split, code/SOURCE, projected draw, contextual discretionary candidates, narrow hyphens, previous/latest breaks and 3 failure retries passed");
}
