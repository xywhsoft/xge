#include "../xui_document_ui.h"
#include "xui_test_proxy.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHECK(e) do { if (!(e)) { fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #e); exit(1); } } while (0)
#define PARAGRAPHS 48
static xui_draw_text_proc base_text;
static xui_draw_text_spans_proc base_spans;
static struct draw_record { char first; xui_rect_t rect; } draws[128];
static size_t draw_count;
static unsigned span_draw_count;
static void record_glyph(const char* text, xui_rect_t rect)
{
    CHECK(text[0] && !strchr(text, '\r') && !strchr(text, '\n') &&
        draw_count < sizeof(draws) / sizeof(draws[0]));
    draws[draw_count].first = text[0]; draws[draw_count++].rect = rect;
}
static int record_text(xui_proxy p, xui_draw_context d, const xui_text_item_t* pTextItem, xui_rect_t rect, uint32_t color, uint32_t flags)
{
    const char* text = pTextItem && pTextItem->iSize >= sizeof(*pTextItem) ? pTextItem->sText : NULL;

    record_glyph(text, rect);
    return base_text(p, d, pTextItem, rect, color, flags);
}
static int record_spans(xui_proxy p, xui_draw_context d, const xui_text_item_t* pTextItem, xui_rect_t rect, uint32_t color, uint32_t flags, const xui_text_paint_span_t* spans, int count)
{
    const char* text = pTextItem && pTextItem->iSize >= sizeof(*pTextItem) ? pTextItem->sText : NULL;
    int bytes = pTextItem && pTextItem->iSize >= sizeof(*pTextItem) ? pTextItem->iTextSize : 0;

    CHECK(bytes > 0 && count > 0 && spans[0].iStart == 0 && spans[count - 1].iEnd == bytes);
    record_glyph(text, rect); span_draw_count++;
    return base_spans(p, d, pTextItem, rect, color, flags, spans, count);
}
static uint64_t insert(xui_document_transaction txn, uint64_t parent, uint32_t kind,
    const char* text, float spacing)
{
    xui_doc_node_desc_t desc = {0}; uint64_t id = 0;
    desc.iSize = sizeof(desc); desc.iKind = kind; desc.sText = text;
    desc.iTextBytes = text ? strlen(text) : 0;
    if (kind == XUI_DOC_PARAGRAPH) {
        desc.tAttributes.iFlags = XUI_DOC_SPACING_EXPLICIT;
        desc.tAttributes.fParagraphSpacing = spacing;
    }
    CHECK(xuiDocumentTxnInsertNode(txn, parent, XUI_DOCUMENT_APPEND, &desc, &id) == XUI_OK);
    return id;
}
static xui_doc_position_t position(xui_document doc, uint64_t node, uint64_t at, uint32_t kind)
{
    xui_doc_position_t p = {0}; p.iSize = sizeof(p); p.iKind = kind;
    p.iDocumentId = xuiDocumentGetIdentity(doc); p.iRevision = xuiDocumentGetRevision(doc);
    p.iNodeId = node; p.iOffset = at; p.iAffinity = XUI_DOC_AFTER; return p;
}
static int near(double a, double b) { return fabs(a - b) < .0005; }
static int pixel(double x) { return (int)floor(x + .5); }
int main(void)
{
    static const double scrolls[] = {0, 600.25, 900.5};
    const float zoom = 1.25f, gap = .35f, spacing = .35f;
    xui_test_proxy_state_t proxy; xui_context context; xui_font font, sized;
    xui_surface target; xui_draw_context draw; xui_text_shape_t shape = {0};
    xui_document document; xui_document_transaction txn; xui_document_snapshot snapshot;
    xui_document_renderer renderer; xui_doc_renderer_desc_t desc = {0};
    xui_doc_rect_t size, caret; xui_doc_position_t hit;
    uint64_t paragraphs[PARAGRAPHS], text_nodes[PARAGRAPHS];
    double tops[PARAGRAPHS], height, line_height, glyph_width, expected_height = 0;
    int empty[PARAGRAPHS], exact, i, phase, split_round_cases = 0;
    xuiTestProxyInit(&proxy);
    base_text = proxy.tProxy.drawText; proxy.tProxy.drawText = record_text;
    base_spans = proxy.tProxy.drawTextSpans; proxy.tProxy.drawTextSpans = record_spans;
    CHECK(xuiCreate(&context) == XUI_OK && xuiSetProxy(context, &proxy.tProxy) == XUI_OK);
    CHECK(proxy.tProxy.fontLoadFile(&proxy.tProxy, &font, "fractional.ttf", 14, 0) == XUI_OK);
    CHECK(proxy.tProxy.fontCreateSized(&proxy.tProxy, &sized, font, 14 * zoom) == XUI_OK);
    CHECK(xuiSetDefaultFont(context, font) == XUI_OK);
    CHECK(xuiTestSurfaceCreate(&proxy, &target, 360, 240, XUI_SURFACE_USAGE_TARGET) == XUI_OK);
    CHECK(xuiTextShape(context, &(xui_text_item_t){.iSize=sizeof(xui_text_item_t), .pFont=sized, .sText="A", .iTextSize=1, .iFlags=0}, &shape) == XUI_OK);
    line_height = shape.fLineHeight; glyph_width = shape.fWidth; xuiTextShapeFree(&shape);
    CHECK(line_height > 0);
    CHECK(xuiDocumentCreate(NULL, &document) == XUI_OK &&
        xuiDocumentBeginTransaction(document, NULL, &txn) == XUI_OK);
    for (i = 0; i < PARAGRAPHS; i++) {
        char text[2] = {(char)('A' + i % 26), 0};
        const char* content = i == 5 ? "F\nf" : text;
        empty[i] = i % 7 == 3;
        paragraphs[i] = insert(txn, 1, XUI_DOC_PARAGRAPH, NULL, spacing);
        text_nodes[i] = empty[i] ? 0 : insert(txn, paragraphs[i], XUI_DOC_TEXT, content, 0);
        tops[i] = expected_height;
        height = empty[i] ? 20 * zoom : line_height * (i == 5 ? 2 : 1) + (i == 5 ? gap * zoom : 0);
        expected_height += height + spacing * zoom;
    }
    CHECK(xuiDocumentTxnCommit(txn, NULL) == XUI_OK); xuiDocumentTxnRelease(txn);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
    desc.iSize = sizeof(desc); desc.fZoom = zoom; desc.fLineGap = gap; desc.fParagraphGap = .45f;
    CHECK(xuiDocumentRendererCreate(context, &desc, &renderer) == XUI_OK &&
        xuiDocumentRendererSetSnapshot(renderer, snapshot, NULL) == XUI_OK);
    xuiDocumentSnapshotRelease(snapshot);
    CHECK(xuiDocumentRendererLayout(renderer, 320, 0, 240) == XUI_OK);
    for (i = 0; i < PARAGRAPHS; i++) {
        xui_doc_position_t p = empty[i] ? position(document, paragraphs[i], 0, XUI_DOC_POSITION_GAP) :
            position(document, text_nodes[i], 0, XUI_DOC_POSITION_TEXT);
        CHECK(xuiDocumentRendererGetCaretRect(renderer, &p, &caret) == XUI_OK);
        if (!near(caret.y, tops[i])) fprintf(stderr, "paragraph=%d caret_y=%.8f expected=%.8f line_height=%.8f\n", i, caret.y, tops[i], line_height);
        CHECK(near(caret.y, tops[i]));
        CHECK(near(caret.height, empty[i] ? 20 * zoom : line_height));
        CHECK(xuiDocumentRendererHitTest(renderer, 0, tops[i] + caret.height / 2, &hit) == XUI_OK &&
            hit.iNodeId == (empty[i] ? paragraphs[i] : text_nodes[i]));
    }
    {
        xui_doc_position_t p = position(document, text_nodes[5], 2, XUI_DOC_POSITION_TEXT);
        CHECK(xuiDocumentRendererGetCaretRect(renderer, &p, &caret) == XUI_OK);
        CHECK(near(caret.y, tops[5] + line_height + gap * zoom));
        CHECK(xuiDocumentRendererHitTest(renderer, 0,
            tops[5] + line_height + gap * zoom + line_height / 2, &hit) == XUI_OK &&
            hit.iNodeId == text_nodes[5] && hit.iOffset == 2);
        p.iAffinity = XUI_DOC_BEFORE;
        CHECK(xuiDocumentRendererGetCaretRect(renderer, &p, &caret) == XUI_OK);
        CHECK(near(caret.y, tops[5]));
    }
    CHECK(xuiDocumentRendererGetSize(renderer, &size, &exact) == XUI_OK && exact);
    CHECK(near(size.height, expected_height));
    for (phase = 0; phase < 3; phase++) {
        double scroll = scrolls[phase]; size_t expected_count = 0;
        struct draw_record expected[128];
        CHECK(xuiDocumentRendererLayout(renderer, 320, scroll, 240) == XUI_OK);
        for (i = 0; i < PARAGRAPHS; i++) {
            int line;
            if (empty[i]) continue;
            if (pixel(tops[i] - scroll) != pixel(tops[i]) + pixel(-scroll)) split_round_cases++;
            for (line = 0; line < (i == 5 ? 2 : 1); line++) {
                double y = tops[i] + line * (line_height + gap * zoom) - scroll;
                xui_rect_t rect = {0, (float)pixel(y), (float)pixel(glyph_width),
                    (float)(pixel(y + line_height) - pixel(y))};
                if (rect.fY > 240 || rect.fY + rect.fH < 0) continue;
                expected[expected_count].first = line ? 'f' : (char)('A' + i % 26);
                expected[expected_count++].rect = rect;
            }
        }
        draw_count = 0;
        CHECK(proxy.tProxy.drawBegin(&proxy.tProxy, &draw, target) == XUI_OK);
        CHECK(xuiDocumentRendererDraw(renderer, draw, 0, -scroll,
            (xui_rect_t){0, 0, 320, 240}, NULL, 0) == XUI_OK);
        CHECK(proxy.tProxy.drawEnd(&proxy.tProxy, draw) == XUI_OK);
        CHECK(draw_count == expected_count);
        for (i = 0; i < (int)draw_count; i++) {
            CHECK(draws[i].first == expected[i].first);
            CHECK(draws[i].rect.fX == expected[i].rect.fX && draws[i].rect.fW == expected[i].rect.fW &&
                draws[i].rect.fY == expected[i].rect.fY &&
                draws[i].rect.fH == expected[i].rect.fH);
        }
    }
    CHECK(split_round_cases > 0);
    CHECK(span_draw_count >= 2);
    xuiDocumentRendererRelease(renderer); xuiDocumentRelease(document); xuiDestroy(context);
    proxy.tProxy.surfaceDestroy(&proxy.tProxy, target);
    proxy.tProxy.fontDestroy(&proxy.tProxy, sized); proxy.tProxy.fontDestroy(&proxy.tProxy, font);
    puts("Unified Document fractional geometry: 48 paragraphs, decimal spacing/zoom/scroll, line caret/hit and final-pixel rounding passed");
    return 0;
}
