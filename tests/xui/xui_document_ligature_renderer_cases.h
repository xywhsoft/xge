#include "xui_document_ligature_model.h"
static xui_draw_text_proc ligature_base_text;
static xui_draw_text_spans_proc ligature_base_spans;
static const char* ligature_draw_expected;
static double ligature_draw_left, ligature_draw_top, ligature_draw_width;
static unsigned ligature_draw_count, ligature_draw_wrap;
static void ligature_draw_check(const char* text, int bytes, xui_rect_t rect)
{
    int split_script = ligature_sample == 3 && !ligature_draw_wrap;
    if (!ligature_draw_expected) return;
    if (ligature_draw_wrap == 2) {
        CHECK(bytes == 1 && text[0] == ligature_sources[ligature_sample][ligature_draw_count]);
    } else {
        const char* expected = ligature_draw_wrap || split_script ? (ligature_draw_count ? "X" : ligature_patterns[ligature_sample]) : ligature_draw_expected;
        if (bytes != (int)strlen(expected) || memcmp(text, expected, (size_t)bytes))
            fprintf(stderr, "Ligature draw sample=%u wrap=%u count=%u text='%.*s' expected='%s' rect=%g,%g,%g\n",
                ligature_sample, ligature_draw_wrap, ligature_draw_count, bytes, text, expected,
                (double)rect.fX, (double)rect.fY, (double)rect.fW);
        CHECK(bytes == (int)strlen(expected) && !memcmp(text, expected, (size_t)bytes));
    }
    CHECK(fabs((double)rect.fX - ligature_draw_left - (split_script && ligature_draw_count ? 12 : 0)) < .01 &&
        fabs((double)rect.fY - ligature_draw_top - (ligature_draw_wrap ? ligature_draw_count * 24 : 0)) < .01 &&
        fabs((double)rect.fW - (split_script ? (ligature_draw_count ? 10 : 12) :
            ligature_draw_wrap == 2 || (ligature_draw_wrap && ligature_draw_count) ? 10 : ligature_draw_width)) < .01);
    ligature_draw_count++;
}
static int ligature_text(xui_proxy proxy, xui_draw_context draw, const xui_text_item_t* pTextItem, xui_rect_t rect, uint32_t color, uint32_t flags)
{
    const char* text = pTextItem && pTextItem->iSize >= sizeof(*pTextItem) ? pTextItem->sText : NULL;

    ligature_draw_check(text, (int)strlen(text), rect);
    return ligature_base_text(proxy, draw, pTextItem, rect, color, flags);
}
static int ligature_spans(xui_proxy proxy, xui_draw_context draw, const xui_text_item_t* pTextItem, xui_rect_t rect, uint32_t color, uint32_t flags, const xui_text_paint_span_t* spans, int count)
{
    const char* text = pTextItem && pTextItem->iSize >= sizeof(*pTextItem) ? pTextItem->sText : NULL;
    int bytes = pTextItem && pTextItem->iSize >= sizeof(*pTextItem) ? pTextItem->iTextSize : 0;

    const char* expected = ligature_draw_expected; int result;
    ligature_draw_check(text, bytes, rect); ligature_draw_expected = NULL;
    result = ligature_base_spans(proxy, draw, pTextItem, rect, color, flags, spans, count);
    ligature_draw_expected = expected; return result;
}
static void document_ligature_carets(xui_test_proxy_state_t* proxy)
{
    xui_context context; xui_font font; xui_surface surface; unsigned variant, pass;
    ligature_base_shape = proxy->tProxy.textShape; proxy->tProxy.textShape = ligature_shape;
    ligature_base_text = proxy->tProxy.drawText; proxy->tProxy.drawText = ligature_text;
    ligature_base_spans = proxy->tProxy.drawTextSpans; proxy->tProxy.drawTextSpans = ligature_spans;
    CHECK(proxy->tProxy.fontLoadFile(&proxy->tProxy, &font, "ligature.ttf", 20, 0) == XUI_OK &&
        xuiCreate(&context) == XUI_OK && xuiSetProxy(context, &proxy->tProxy) == XUI_OK &&
        xuiSetDefaultFont(context, font) == XUI_OK && xuiTestSurfaceCreate(proxy, &surface, 320, 200, XUI_SURFACE_USAGE_TARGET) == XUI_OK);
    for (ligature_caret_mode = 0; ligature_caret_mode < 2; ligature_caret_mode++)
    for (ligature_sample = 0; ligature_sample < 5; ligature_sample++) for (variant = 0; variant < 6; variant++) {
        xui_document document; xui_document_snapshot snapshot;
        xui_document_renderer renderer; xui_doc_renderer_desc_t desc = {0};
        uint64_t first, tail, paragraph; unsigned middle = ligature_middle[ligature_sample];
        const char* source = ligature_sources[ligature_sample]; size_t bytes = strlen(source);
        double first_advance = ligature_caret_mode ? (ligature_sample == 1 ? 4 : 8) : 6;
        xui_doc_position_t head, at, end, hit; xui_doc_rect_t left, caret, eof;
        if (variant == 2 || variant >= 4) {
            char markdown[40]; xui_doc_desc_t profile = {0};
            profile.iSize = sizeof(profile); profile.iProfile = XUI_DOCUMENT_MARKDOWN;
            if (variant == 2) snprintf(markdown, sizeof(markdown), "**%.*s**%s", (int)middle, source, source + middle);
            CHECK(xuiDocumentCreate(&profile, &document) == XUI_OK && xuiDocumentLoadMarkdown(document,
                variant == 2 ? markdown : source, variant == 2 ? strlen(markdown) : bytes) == XUI_OK);
            CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK &&
                xuiDocumentSnapshotGetChild(snapshot, 1, 0, &paragraph) == XUI_OK &&
                xuiDocumentSnapshotGetChild(snapshot, paragraph, 0, &first) == XUI_OK);
            if (variant == 2) CHECK(xuiDocumentSnapshotGetChild(snapshot, paragraph, 1, &tail) == XUI_OK);
            else tail = first;
            xuiDocumentSnapshotRelease(snapshot);
        } else {
            xui_document_transaction transaction; xui_doc_node_desc_t node = {0};
            CHECK(xuiDocumentCreate(NULL, &document) == XUI_OK &&
                xuiDocumentBeginTransaction(document, NULL, &transaction) == XUI_OK);
            node.iSize = sizeof(node); node.iKind = variant == 3 ? XUI_DOC_CODE_BLOCK : XUI_DOC_PARAGRAPH;
            if (variant == 3) { node.sText = source; node.iTextBytes = bytes; }
            CHECK(xuiDocumentTxnInsertNode(transaction, 1, XUI_DOCUMENT_APPEND, &node, &paragraph) == XUI_OK);
            if (variant == 3) first = tail = paragraph;
            else {
                char prefix[16]; memcpy(prefix, source, middle); prefix[middle] = 0;
                first = decoration_insert(transaction, paragraph, variant == 1 ? prefix : source, 0, 0xff3366ffu);
                tail = variant == 1 ? decoration_insert(transaction, paragraph, source + middle, 0, 0xffdd4433u) : first;
            }
            CHECK(xuiDocumentTxnCommit(transaction, NULL) == XUI_OK); xuiDocumentTxnRelease(transaction);
        }
        head = decoration_position(document, first, 0);
        at = decoration_position(document, first, middle);
        end = decoration_position(document, tail, bytes - ((variant == 1 || variant == 2) ? middle : 0));
        if (variant >= 4) {
            head.iKind = at.iKind = end.iKind = XUI_DOC_POSITION_SOURCE;
            head.iNodeId = at.iNodeId = end.iNodeId = 1; head.iOffset = 0; end.iOffset = bytes;
        }
        desc.iSize = sizeof(desc); desc.fLineGap = 4; desc.tFonts = (xui_doc_font_set_t){font, font, font, font, font};
        CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK &&
            xuiDocumentRendererCreate(context, &desc, &renderer) == XUI_OK &&
            xuiDocumentRendererSetSnapshot(renderer, snapshot, NULL) == XUI_OK);
        xuiDocumentSnapshotRelease(snapshot);
        if (variant >= 4) CHECK(xuiDocumentRendererSetMode(renderer, variant == 4 ? XUI_DOC_SOURCE_TEXT : XUI_DOC_LIVE_MARKDOWN) == XUI_OK);
        if (variant == 5) CHECK(xuiDocumentRendererSetActivePosition(renderer, &at) == XUI_OK);
        for (pass = 0; pass < 3; pass++) {
            int layout_result = xuiDocumentRendererLayout(renderer, pass == 1 ? 120 : 200, 0, 200);
            int caret_result = layout_result == XUI_OK ? xuiDocumentRendererGetCaretRect(renderer, &head, &left) : layout_result;
            if (layout_result != XUI_OK || caret_result != XUI_OK)
                fprintf(stderr, "Ligature mode=%u sample=%u variant=%u pass=%u layout=%d caret=%d\n",
                    ligature_caret_mode, ligature_sample, variant, pass, layout_result, caret_result);
            CHECK(layout_result == XUI_OK && caret_result == XUI_OK);
            at.iAffinity = XUI_DOC_BEFORE;
            CHECK(xuiDocumentRendererGetCaretRect(renderer, &at, &caret) == XUI_OK &&
                fabs(caret.x - left.x - first_advance) < .01 && fabs(caret.y - left.y) < .01);
            at.iAffinity = XUI_DOC_AFTER;
            CHECK(xuiDocumentRendererGetCaretRect(renderer, &at, &caret) == XUI_OK && fabs(caret.x - left.x - first_advance) < .01);
            CHECK(xuiDocumentRendererGetCaretRect(renderer, &end, &eof) == XUI_OK &&
                fabs(eof.x - left.x - (ligature_sample == 1 ? 28 : 22)) < .01);
            if (!ligature_sample) CHECK(xuiDocumentRendererHitTest(renderer, left.x + first_advance - 1, left.y + 10, &hit) == XUI_OK &&
                (variant >= 4 || hit.iNodeId == first) && hit.iOffset == 1);
            if (ligature_sample == 1) {
                xui_doc_position_t second = decoration_position(document, tail, (variant == 1 || variant == 2) ? 1 : 2);
                if (variant >= 4) { second.iKind = XUI_DOC_POSITION_SOURCE; second.iNodeId = 1; }
                CHECK(xuiDocumentRendererGetCaretRect(renderer, &second, &caret) == XUI_OK &&
                    fabs(caret.x - left.x - (ligature_caret_mode ? 11 : 12)) < .01);
            }
            {
                xui_doc_range_t range = {head, at}; xui_doc_rect_t rects[8]; uint64_t count;
                xui_draw_context draw; char displayed[32];
                CHECK(xuiDocumentRendererGetRangeRects(renderer, &range, rects, 8, &count) == XUI_OK && count == 1 &&
                    fabs(rects[0].width - first_advance) < .01);
                snprintf(displayed, sizeof(displayed), "%sX", ligature_patterns[ligature_sample]);
                ligature_draw_expected = displayed; ligature_draw_left = left.x; ligature_draw_top = left.y;
                ligature_draw_width = ligature_sample == 1 ? 28 : 22; ligature_draw_count = 0;
                CHECK(proxy->tProxy.drawBegin(&proxy->tProxy, &draw, surface) == XUI_OK &&
                    xuiDocumentRendererDraw(renderer, draw, 0, 0, (xui_rect_t){0, 0, 320, 200}, NULL, 0) == XUI_OK &&
                    proxy->tProxy.drawEnd(&proxy->tProxy, draw) == XUI_OK && ligature_draw_count == (ligature_sample == 3 ? 2u : 1u));
                ligature_draw_expected = NULL;
            }
        }
        if (variant < 3) {
            unsigned wrap;
            for (wrap = 1; wrap <= 2; wrap++) {
                xui_doc_position_t boundary = end; xui_doc_rect_t before, after;
                xui_draw_context draw; unsigned rows = wrap == 1 ? 2 : (unsigned)bytes;
                if (wrap == 2 && ligature_sample > 1) continue;
                boundary.iOffset = wrap == 1 ? bytes - 1 - ((variant == 1 || variant == 2) ? middle : 0) : middle;
                if (wrap == 2) boundary.iNodeId = first;
                boundary.iAffinity = XUI_DOC_BEFORE;
                CHECK(xuiDocumentRendererLayout(renderer, wrap == 1 ? (ligature_sample == 1 ? 20 : 15) : 8, 0, 200) == XUI_OK &&
                    xuiDocumentRendererGetCaretRect(renderer, &boundary, &before) == XUI_OK &&
                    fabs(before.x - (wrap == 1 ? (ligature_sample == 1 ? 18 : 12) : 10)) < .01 && fabs(before.y) < .01);
                boundary.iAffinity = XUI_DOC_AFTER;
                CHECK(xuiDocumentRendererGetCaretRect(renderer, &boundary, &after) == XUI_OK && fabs(after.x) < .01 && fabs(after.y - 24) < .01 &&
                    xuiDocumentRendererGetCaretRect(renderer, &end, &eof) == XUI_OK && fabs(eof.x - 10) < .01 && fabs(eof.y - (rows - 1) * 24) < .01);
                ligature_draw_expected = source; ligature_draw_wrap = wrap; ligature_draw_count = 0;
                ligature_draw_left = ligature_draw_top = 0; ligature_draw_width = ligature_sample == 1 ? 18 : 12;
                CHECK(proxy->tProxy.drawBegin(&proxy->tProxy, &draw, surface) == XUI_OK &&
                    xuiDocumentRendererDraw(renderer, draw, 0, 0, (xui_rect_t){0, 0, 320, 200}, NULL, 0) == XUI_OK &&
                    proxy->tProxy.drawEnd(&proxy->tProxy, draw) == XUI_OK && ligature_draw_count == rows);
                ligature_draw_expected = NULL; ligature_draw_wrap = 0;
            }
            CHECK(xuiDocumentRendererLayout(renderer, 200, 0, 200) == XUI_OK);
        }
        if (ligature_caret_mode && !ligature_sample && !variant) {
            xui_text_shape_t raw = {0};
            CHECK(xuiTextShape(context, &(xui_text_item_t){.iSize=sizeof(xui_text_item_t), .pFont=font, .sText="fiX", .iTextSize=3, .iFlags=XUI_TEXT_SHAPE_DEFAULT}, &raw) == XUI_OK &&
                raw.iCaretCount == 1 && raw.pCarets && raw.pCarets[0].iTextOffset == 1 && raw.pCarets[0].fAdvance == 8);
            xuiTextShapeFree(&raw); CHECK(!raw.pCarets && !raw.pClusters && !raw.iCaretCount);
        }
        if (ligature_caret_mode && !variant && (ligature_sample == 1 || ligature_sample == 3)) {
            uint64_t revision = xuiDocumentGetRevision(document); unsigned bad;
            for (bad = 1; bad <= 7; bad++) {
                if ((bad == 4) != (ligature_sample == 3)) continue;
                ligature_bad_stop = bad;
                CHECK(xuiDocumentRendererInvalidateFonts(renderer) == XUI_OK &&
                    xuiDocumentRendererLayout(renderer, 200, 0, 200) == XUI_ERROR_INVALID_STATE &&
                    xuiDocumentGetRevision(document) == revision);
                ligature_bad_stop = 0;
                CHECK(xuiDocumentRendererLayout(renderer, 200, 0, 200) == XUI_OK &&
                    xuiDocumentRendererGetCaretRect(renderer, &at, &caret) == XUI_OK && fabs(caret.x - left.x - first_advance) < .01);
            }
            ligature_oom_shape = 1;
            CHECK(xuiDocumentRendererInvalidateFonts(renderer) == XUI_OK &&
                xuiDocumentRendererLayout(renderer, 200, 0, 200) == XUI_ERROR_OUT_OF_MEMORY && !ligature_oom_shape);
            CHECK(xuiDocumentRendererLayout(renderer, 200, 0, 200) == XUI_OK);
        }
        xuiDocumentRendererRelease(renderer); xuiDocumentRelease(document);
    }
    proxy->tProxy.textShape = ligature_base_shape;
    proxy->tProxy.drawText = ligature_base_text; proxy->tProxy.drawTextSpans = ligature_base_spans;
    proxy->tProxy.surfaceDestroy(&proxy->tProxy, surface);
    xuiDestroy(context); proxy->tProxy.fontDestroy(&proxy->tProxy, font);
    puts("Document ligature carets: Unicode grapheme stops inside merged clusters, Rich joined/split, Markdown visual/source/live and code reflow passed");
}
