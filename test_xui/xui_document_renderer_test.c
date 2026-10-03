#include "../xui_document_ui.h"
#include "xui_test_proxy.h"
#include "../xge.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#ifdef _WIN32
#include <windows.h>
#endif

#define CHECK(c) do { if (!(c)) { fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #c); exit(1); } } while (0)
#include "xui_document_shape_paint_cases.h"
#include "xui_document_shared_span_cases.h"
#include "xui_document_line_paint_cases.h"
static uint64_t add(xui_document_transaction t, uint64_t parent,
    unsigned kind, const char* text);
static xui_doc_position_t position(xui_document d, uint64_t id, uint64_t at);
static xui_text_shape_proc paragraph_base_shape;
static xui_draw_text_spans_proc paragraph_base_spans;
static int paragraph_spans_seen;
static uint32_t paragraph_expected_color;
static int paragraph_wrapped_spans_seen;
static int paragraph_wrapped_mode;
static int paragraph_kerning_shape(xui_proxy proxy, const xui_text_item_t* pTextItem, xui_text_shape_t* shape)
{
    const char* text = pTextItem && pTextItem->iSize >= sizeof(*pTextItem) ? pTextItem->sText : NULL;

    int result = paragraph_base_shape(proxy, pTextItem, shape);
    if (result == XUI_OK && shape->pClusters) {
        int i;
        for (i = 0; i + 1 < shape->iClusterCount; i++) {
            if (text[shape->pClusters[i].iTextStart] == 'A' &&
                text[shape->pClusters[i + 1].iTextStart] == 'V') {
                shape->pClusters[i].fAdvance -= 3;
                shape->fWidth -= 3;
            }
        }
    }
    return result;
}
static int paragraph_kerning_spans(xui_proxy proxy, xui_draw_context draw, const xui_text_item_t* pTextItem, xui_rect_t rect, uint32_t color, uint32_t flags, const xui_text_paint_span_t* spans, int count)
{
    const char* text = pTextItem && pTextItem->iSize >= sizeof(*pTextItem) ? pTextItem->sText : NULL;
    int bytes = pTextItem && pTextItem->iSize >= sizeof(*pTextItem) ? pTextItem->iTextSize : 0;

    if (paragraph_wrapped_mode && ((bytes == 3 && memcmp(text, "AV ", 3) == 0) ||
        (bytes == 2 && memcmp(text, "AV", 2) == 0))) {
        CHECK(count == 2 && spans[0].iStart == 0 && spans[0].iEnd == 1 &&
            spans[1].iStart == 1 && spans[1].iEnd == bytes);
        paragraph_wrapped_spans_seen++;
    } else if (bytes == 2 && memcmp(text, "AV", 2) == 0) {
        CHECK(count == 2 && spans[0].iStart == 0 && spans[0].iEnd == 1 &&
            spans[1].iStart == 1 && spans[1].iEnd == 2 &&
            spans[1].iColor == paragraph_expected_color);
        paragraph_spans_seen++;
    }
    return paragraph_base_spans(proxy, draw, pTextItem, rect, color, flags, spans, count);
}
static void paragraph_paint_boundary_kerning(xui_context context,
    xui_surface target, xui_test_proxy_state_t* proxy)
{
    xui_context kerning_context;
    xui_font font = xuiGetDefaultFont(context);
    xui_document document[2]; xui_document_renderer renderer[2];
    xui_doc_rect_t end[2], boundary[2];
    uint64_t last[2]; unsigned variant;
    xui_draw_context draw;
    paragraph_base_shape = proxy->tProxy.textShape;
    paragraph_base_spans = proxy->tProxy.drawTextSpans;
    paragraph_expected_color = XUI_COLOR_RGBA(180, 20, 40, 255);
    proxy->tProxy.textShape = paragraph_kerning_shape;
    proxy->tProxy.drawTextSpans = paragraph_kerning_spans;
    CHECK(xuiCreate(&kerning_context) == XUI_OK &&
        xuiSetProxy(kerning_context, &proxy->tProxy) == XUI_OK &&
        xuiSetDefaultFont(kerning_context, font) == XUI_OK);
    for (variant = 0; variant < 2; variant++) {
        xui_document_transaction transaction; xui_document_snapshot snapshot;
        xui_doc_node_desc_t node = {0}; xui_doc_position_t at;
        uint64_t paragraph;
        CHECK(xuiDocumentCreate(NULL, &document[variant]) == XUI_OK &&
            xuiDocumentBeginTransaction(document[variant], NULL, &transaction) == XUI_OK);
        paragraph = add(transaction, 1, XUI_DOC_PARAGRAPH, NULL);
        node.iSize = sizeof(node); node.iKind = XUI_DOC_TEXT;
        node.sText = variant ? "A" : "AV";
        node.iTextBytes = variant ? 1 : 2;
        CHECK(xuiDocumentTxnInsertNode(transaction, paragraph,
            XUI_DOCUMENT_APPEND, &node, &last[variant]) == XUI_OK);
        if (variant) {
            node.sText = "V"; node.iTextBytes = 1;
            node.tAttributes.iTextColor = paragraph_expected_color;
            CHECK(xuiDocumentTxnInsertNode(transaction, paragraph,
                XUI_DOCUMENT_APPEND, &node, &last[variant]) == XUI_OK);
        }
        CHECK(xuiDocumentTxnCommit(transaction, NULL) == XUI_OK);
        xuiDocumentTxnRelease(transaction);
        CHECK(xuiDocumentAcquireSnapshot(document[variant], &snapshot) == XUI_OK &&
            xuiDocumentRendererCreate(kerning_context, NULL, &renderer[variant]) == XUI_OK &&
            xuiDocumentRendererSetSnapshot(renderer[variant], snapshot, NULL) == XUI_OK &&
            xuiDocumentRendererLayout(renderer[variant], 300, 0, 100) == XUI_OK);
        xuiDocumentSnapshotRelease(snapshot);
        at = position(document[variant], last[variant], variant ? 1 : 2);
        CHECK(xuiDocumentRendererGetCaretRect(renderer[variant], &at,
            &end[variant]) == XUI_OK);
        if (variant) {
            at.iOffset = 0;
            CHECK(xuiDocumentRendererGetCaretRect(renderer[variant], &at,
                &boundary[variant]) == XUI_OK);
        }
    }
    CHECK(end[0].x < 14 && fabs(end[0].x - end[1].x) < .01 &&
        fabs(boundary[1].x - (end[1].x - 7)) < .01);
    paragraph_spans_seen = 0;
    CHECK(proxy->tProxy.drawBegin(&proxy->tProxy, &draw, target) == XUI_OK &&
        xuiDocumentRendererDraw(renderer[1], draw, 0, 0,
            (xui_rect_t){0, 0, 300, 100}, NULL, 0) == XUI_OK &&
        proxy->tProxy.drawEnd(&proxy->tProxy, draw) == XUI_OK &&
        paragraph_spans_seen == 1);
    {
        xui_document_transaction transaction;
        xui_document_change_set changes;
        xui_document_snapshot snapshot;
        xui_doc_text_style_t style = {0};
        xui_doc_range_t range = {0};
        xui_doc_renderer_stats_t before = {0}, after = {0};
        xui_doc_position_t at;
        before.iSize = after.iSize = sizeof(before);
        CHECK(xuiDocumentRendererGetStats(renderer[1], &before) == XUI_OK);
        range.tAnchor = position(document[1], last[1], 0);
        range.tCaret = position(document[1], last[1], 1);
        style.iSize = sizeof(style);
        style.iTextColor = XUI_COLOR_RGBA(20, 40, 180, 255);
        CHECK(xuiDocumentBeginTransaction(document[1], NULL, &transaction) == XUI_OK &&
            xuiDocumentTxnSetTextStyle(transaction, &range,
                XUI_DOC_TEXT_STYLE_COLOR, &style) == XUI_OK &&
            xuiDocumentTxnCommit(transaction, &changes) == XUI_OK);
        xuiDocumentTxnRelease(transaction);
        CHECK(xuiDocumentAcquireSnapshot(document[1], &snapshot) == XUI_OK &&
            xuiDocumentRendererSetSnapshot(renderer[1], snapshot, changes) == XUI_OK &&
            xuiDocumentRendererLayout(renderer[1], 300, 0, 100) == XUI_OK);
        xuiDocumentSnapshotRelease(snapshot);
        xuiDocumentChangeSetRelease(changes);
        at = position(document[1], last[1], 1);
        CHECK(xuiDocumentRendererGetCaretRect(renderer[1], &at, &boundary[1]) == XUI_OK &&
            fabs(boundary[1].x - end[1].x) < .01 &&
            xuiDocumentRendererGetStats(renderer[1], &after) == XUI_OK &&
            before.iShapedBytes == after.iShapedBytes);
        paragraph_expected_color = style.iTextColor;
        paragraph_spans_seen = 0;
        CHECK(proxy->tProxy.drawBegin(&proxy->tProxy, &draw, target) == XUI_OK &&
            xuiDocumentRendererDraw(renderer[1], draw, 0, 0,
                (xui_rect_t){0, 0, 300, 100}, NULL, 0) == XUI_OK &&
            proxy->tProxy.drawEnd(&proxy->tProxy, draw) == XUI_OK &&
            paragraph_spans_seen == 1);
    }
    {
        xui_document documents[2]; xui_document_renderer renderers[2];
        uint64_t nodes[2][4] = {{0}}; xui_doc_rect_t first[2], next[2], end_rect[2];
        unsigned sample;
        for (sample = 0; sample < 2; sample++) {
            xui_document_transaction transaction; xui_document_snapshot snapshot;
            xui_doc_node_desc_t node = {0}; uint64_t paragraph;
            unsigned part, parts = sample ? 4 : 1;
            static const char* const split[] = {"A", "V ", "A", "V"};
            CHECK(xuiDocumentCreate(NULL, &documents[sample]) == XUI_OK &&
                xuiDocumentBeginTransaction(documents[sample], NULL, &transaction) == XUI_OK);
            paragraph = add(transaction, 1, XUI_DOC_PARAGRAPH, NULL);
            for (part = 0; part < parts; part++) {
                node.iSize = sizeof(node); node.iKind = XUI_DOC_TEXT;
                node.sText = sample ? split[part] : "AV AV";
                node.iTextBytes = strlen(node.sText);
                node.tAttributes.iTextColor = sample && (part & 1) ?
                    XUI_COLOR_RGBA(180, 20, 40, 255) : 0;
                CHECK(xuiDocumentTxnInsertNode(transaction, paragraph,
                    XUI_DOCUMENT_APPEND, &node, &nodes[sample][part]) == XUI_OK);
            }
            CHECK(xuiDocumentTxnCommit(transaction, NULL) == XUI_OK);
            xuiDocumentTxnRelease(transaction);
            CHECK(xuiDocumentAcquireSnapshot(documents[sample], &snapshot) == XUI_OK &&
                xuiDocumentRendererCreate(kerning_context, NULL, &renderers[sample]) == XUI_OK &&
                xuiDocumentRendererSetSnapshot(renderers[sample], snapshot, NULL) == XUI_OK &&
                xuiDocumentRendererLayout(renderers[sample], 20, 0, 100) == XUI_OK);
            xuiDocumentSnapshotRelease(snapshot);
            {
                xui_doc_position_t at = position(documents[sample],
                    sample ? nodes[sample][1] : nodes[sample][0], sample ? 1 : 2);
                CHECK(xuiDocumentRendererGetCaretRect(renderers[sample], &at,
                    &first[sample]) == XUI_OK);
                at = position(documents[sample], sample ? nodes[sample][2] : nodes[sample][0],
                    sample ? 1 : 4);
                CHECK(xuiDocumentRendererGetCaretRect(renderers[sample], &at,
                    &next[sample]) == XUI_OK);
                at = position(documents[sample], sample ? nodes[sample][3] : nodes[sample][0],
                    sample ? 1 : 5);
                CHECK(xuiDocumentRendererGetCaretRect(renderers[sample], &at,
                    &end_rect[sample]) == XUI_OK);
            }
        }
        CHECK(fabs(first[0].x - first[1].x) < .01 &&
            fabs(first[0].y - first[1].y) < .01 &&
            fabs(next[0].x - next[1].x) < .01 &&
            fabs(next[0].y - next[1].y) < .01 &&
            fabs(end_rect[0].x - end_rect[1].x) < .01 &&
            fabs(end_rect[0].y - end_rect[1].y) < .01 &&
            next[1].y > first[1].y);
        paragraph_wrapped_mode = 1;
        paragraph_wrapped_spans_seen = 0;
        CHECK(proxy->tProxy.drawBegin(&proxy->tProxy, &draw, target) == XUI_OK &&
            xuiDocumentRendererDraw(renderers[1], draw, 0, 0,
                (xui_rect_t){0, 0, 100, 100}, NULL, 0) == XUI_OK &&
            proxy->tProxy.drawEnd(&proxy->tProxy, draw) == XUI_OK &&
            paragraph_wrapped_spans_seen == 2);
        paragraph_wrapped_mode = 0;
        for (sample = 0; sample < 2; sample++) {
            xuiDocumentRendererRelease(renderers[sample]);
            xuiDocumentRelease(documents[sample]);
        }
    }
    for (variant = 0; variant < 2; variant++) {
        xuiDocumentRendererRelease(renderer[variant]);
        xuiDocumentRelease(document[variant]);
    }
    xuiDestroy(kerning_context);
    proxy->tProxy.textShape = paragraph_base_shape;
    proxy->tProxy.drawTextSpans = paragraph_base_spans;
    puts("Paint-only Text boundaries retain whole-line kerning, caret geometry and color spans");
}
static uint64_t add(xui_document_transaction t, uint64_t parent, unsigned kind, const char* text)
{
    xui_doc_node_desc_t desc = {0}; uint64_t id;
    desc.iSize = sizeof(desc); desc.iKind = kind; desc.sText = text; desc.iTextBytes = text ? strlen(text) : 0;
    CHECK(xuiDocumentTxnInsertNode(t, parent, XUI_DOCUMENT_APPEND, &desc, &id) == XUI_OK); return id;
}
static xui_doc_position_t position(xui_document d, uint64_t id, uint64_t at)
{
    xui_doc_position_t p = {0}; p.iSize = sizeof(p); p.iKind = XUI_DOC_POSITION_TEXT;
    p.iDocumentId = xuiDocumentGetIdentity(d); p.iRevision = xuiDocumentGetRevision(d); p.iNodeId = id; p.iOffset = at; return p;
}
#include "xui_document_source_projection_cases.h"
#include "xui_document_grapheme_renderer_cases.h"
#include "xui_document_decoration_renderer_cases.h"
#include "xui_document_mixed_font_cases.h"
#include "xui_document_vertical_font_cases.h"
#include "xui_document_object_shaping_cases.h"
#include "xui_document_break_font_cases.h"
#include "xui_document_unicode_break_cases.h"
#include "xui_document_break_boundary_cases.h"
#include "xui_document_soft_wrap_cases.h"
#include "xui_document_format_cases.h"
#include "xui_document_joint_scale_cases.h"
#include "xui_document_joint_convergence_cases.h"
#include "xui_document_ligature_renderer_cases.h"
#include "xui_document_draw_only_cases.h"
#include "xui_document_grapheme_font_cases.h"
#include "xui_document_context_renderer_cases.h"
#include "xui_document_shy_context_cases.h"
#include "xui_document_language_renderer_cases.h"
#include "xui_text_input_caps_cases.h"
#ifdef _WIN32
static void native_grapheme_font_pixels(void)
{
    static const uint32_t marks[] = {0, XUI_DOC_BOLD, XUI_DOC_SUPERSCRIPT, XUI_DOC_SUBSCRIPT};
    xui_proxy_t proxy = xuiProxyXge(); unsigned backend, sample, style, variant, pass;
    unsigned char* pixels[2];
    CHECK(proxy.fontLoadFile(&proxy, &unit_fonts[0], "C:\\Windows\\Fonts\\arial.ttf", 18, 0) == XUI_OK &&
        proxy.fontLoadFile(&proxy, &unit_fonts[1], "C:\\Windows\\Fonts\\arialbd.ttf", 30, 0) == XUI_OK);
    pixels[0] = malloc(360 * 180 * 4); pixels[1] = malloc(360 * 180 * 4); CHECK(pixels[0] && pixels[1]);
    for (backend = 0; backend < 2; backend++) {
        xui_context context; xui_doc_renderer_desc_t desc = {0};
        if (backend) proxy.drawTextSpans = NULL;
        CHECK(xuiCreate(&context) == XUI_OK && xuiSetProxy(context, &proxy) == XUI_OK &&
            xuiSetDefaultFont(context, unit_fonts[0]) == XUI_OK);
        desc.iSize = sizeof(desc); desc.tFonts = (xui_doc_font_set_t){unit_fonts[0], unit_fonts[1], unit_fonts[0], unit_fonts[1], unit_fonts[0]};
        desc.onFont = unit_font; desc.fLineGap = 4; desc.iTextColor = XUI_COLOR_RGBA(20, 40, 180, 255);
        for (sample = 0; sample < 3; sample++) for (style = 0; style < 4; style++) {
            xui_document document[2]; xui_document_renderer renderer[2]; xui_surface surfaces[2];
            uint64_t first[2], tail[2]; unsigned length = (unsigned)strlen(unit_stems[sample]), split = unit_prefix[sample];
            for (variant = 0; variant < 2; variant++) {
                xui_document_transaction transaction; xui_document_snapshot snapshot; uint64_t paragraph;
                xui_surface_desc_t surface = {0}; char prefix[16], suffix[24];
                CHECK(xuiDocumentCreate(NULL, &document[variant]) == XUI_OK &&
                    xuiDocumentBeginTransaction(document[variant], NULL, &transaction) == XUI_OK);
                paragraph = decoration_insert(transaction, 1, NULL, 0, 0);
                memcpy(prefix, unit_stems[sample], split); prefix[split] = 0;
                first[variant] = decoration_insert(transaction, paragraph, variant ? prefix : unit_stems[sample], marks[style], 0);
                if (variant) decoration_insert(transaction, paragraph, "", XUI_DOC_BOLD | XUI_DOC_SUPERSCRIPT, 0);
                snprintf(suffix, sizeof(suffix), "%sXY", variant ? unit_stems[sample] + split : "");
                tail[variant] = decoration_insert(transaction, paragraph, suffix,
                    style == 0 ? XUI_DOC_BOLD : style == 1 ? 0 : marks[style == 2 ? 3 : 2], 0);
                CHECK(xuiDocumentTxnCommit(transaction, NULL) == XUI_OK); xuiDocumentTxnRelease(transaction);
                CHECK(xuiDocumentAcquireSnapshot(document[variant], &snapshot) == XUI_OK &&
                    xuiDocumentRendererCreate(context, &desc, &renderer[variant]) == XUI_OK &&
                    xuiDocumentRendererSetSnapshot(renderer[variant], snapshot, NULL) == XUI_OK); xuiDocumentSnapshotRelease(snapshot);
                surface.iKind = XUI_SURFACE_KIND_TEXTURE; surface.iFormat = XUI_SURFACE_FORMAT_RGBA8;
                surface.iWidth = 360; surface.iHeight = 180; surface.iFlags = XUI_SURFACE_USAGE_TARGET | XUI_SURFACE_ALPHA_PREMULTIPLIED;
                CHECK(proxy.surfaceCreate(&proxy, &surfaces[variant], &surface) == XUI_OK);
            }
            for (pass = 0; pass < 3; pass++) {
                xui_doc_rect_t ends[2], after[2]; unsigned colored = 0; size_t pixel;
                for (variant = 0; variant < 2; variant++) {
                    xui_draw_context draw;
                    xui_doc_position_t at = decoration_position(document[variant], variant ? tail[variant] : first[variant], variant ? length - split : length);
                    at.iAffinity = XUI_DOC_BEFORE;
                    CHECK(xuiDocumentRendererLayout(renderer[variant], pass == 1 ? 7 : 120, 0, 180) == XUI_OK &&
                        xuiDocumentRendererGetCaretRect(renderer[variant], &at, &ends[variant]) == XUI_OK);
                    at = decoration_position(document[variant], tail[variant], (variant ? length - split : 0) + 1);
                    {
                        int result = xuiDocumentRendererGetCaretRect(renderer[variant], &at, &after[variant]);
                        if (result != XUI_OK) fprintf(stderr, "Native grapheme font backend=%u sample=%u style=%u variant=%u pass=%u at=%llu:%llu caret=%d\n",
                            backend, sample, style, variant, pass, (unsigned long long)at.iNodeId, (unsigned long long)at.iOffset, result);
                        CHECK(result == XUI_OK);
                    }
                    CHECK(proxy.surfaceClear(&proxy, surfaces[variant], 0) == XUI_OK);
                    CHECK(proxy.drawBegin(&proxy, &draw, surfaces[variant]) == XUI_OK);
                    {
                        int result = xuiDocumentRendererDraw(renderer[variant], draw, 0, 0, (xui_rect_t){0, 0, 360, 180}, NULL, 0);
                        if (result != XUI_OK) fprintf(stderr, "Native grapheme font backend=%u sample=%u style=%u variant=%u pass=%u draw=%d\n",
                            backend, sample, style, variant, pass, result);
                        CHECK(result == XUI_OK && proxy.drawEnd(&proxy, draw) == XUI_OK &&
                            proxy.surfaceReadRGBA(&proxy, surfaces[variant], pixels[variant], 360 * 4) == XUI_OK);
                    }
                }
                unit_rect_equal(ends[0], ends[1]); unit_rect_equal(after[0], after[1]);
                CHECK(!memcmp(pixels[0], pixels[1], 360 * 180 * 4));
                for (pixel = 0; pixel < 360 * 180; pixel++) if (pixels[0][pixel * 4 + 3] >= 64) colored++;
                CHECK(colored > 10);
            }
            for (variant = 0; variant < 2; variant++) {
                proxy.surfaceDestroy(&proxy, surfaces[variant]); xuiDocumentRendererRelease(renderer[variant]); xuiDocumentRelease(document[variant]);
            }
        }
        xuiDestroy(context);
    }
    free(pixels[0]); free(pixels[1]); proxy.fontDestroy(&proxy, unit_fonts[0]); proxy.fontDestroy(&proxy, unit_fonts[1]);
    puts("Native XGE Arial cross-font combining/ZWJ/RI and script owners: spans/plain, wide/narrow/reflow, caret and identical nonempty full RGBA pixels passed");
}
static void native_object_font_pixels(xui_proxy proxy, xui_context context, xui_font font,
    unsigned char* first_pixels, unsigned char* second_pixels)
{
    xui_document document[2]; xui_document_renderer renderer[2]; xui_surface surfaces[2];
    xui_doc_renderer_desc_t desc = {0}; object_shape_probe probe = {0};
    uint64_t last[2], object[2]; unsigned variant, pass; xui_doc_rect_t bounds[2], caret[2];
    unsigned blue = 0, red = 0, purple = 0; size_t at;
    const uint32_t text_blue = XUI_COLOR_RGBA(20, 40, 180, 255), text_red = XUI_COLOR_RGBA(180, 20, 40, 255);
    probe.width = 13; probe.height = 36; probe.baseline = 24;
    desc.iSize = sizeof(desc); desc.tFonts = (xui_doc_font_set_t){font, font, font, font, font};
    desc.onObjectMeasure = object_shape_measure; desc.onObjectDraw = object_shape_draw; desc.pUser = &probe;
    for (variant = 0; variant < 2; variant++) {
        xui_document_transaction transaction; xui_document_snapshot snapshot; uint64_t paragraph;
        xui_doc_node_desc_t image = {0}; xui_surface_desc_t surface = {0};
        CHECK(xuiDocumentCreate(NULL, &document[variant]) == XUI_OK &&
            xuiDocumentBeginTransaction(document[variant], NULL, &transaction) == XUI_OK);
        paragraph = decoration_insert(transaction, 1, NULL, 0, 0);
        if (variant) decoration_insert(transaction, paragraph, "A", XUI_DOC_UNDERLINE, text_blue);
        decoration_insert(transaction, paragraph, variant ? "V " : "AV ", XUI_DOC_UNDERLINE, text_blue);
        image.iSize = sizeof(image); image.iKind = XUI_DOC_IMAGE;
        CHECK(xuiDocumentTxnInsertNode(transaction, paragraph, XUI_DOCUMENT_APPEND, &image, &object[variant]) == XUI_OK);
        if (variant) {
            decoration_insert(transaction, paragraph, "A", XUI_DOC_SUPERSCRIPT | XUI_DOC_UNDERLINE, text_red);
            decoration_insert(transaction, paragraph, "V\nA", XUI_DOC_SUPERSCRIPT | XUI_DOC_UNDERLINE, text_red);
        }
        last[variant] = decoration_insert(transaction, paragraph, variant ? "V" : "AV\nAV",
            XUI_DOC_SUPERSCRIPT | XUI_DOC_UNDERLINE, text_red);
        CHECK(xuiDocumentTxnCommit(transaction, NULL) == XUI_OK); xuiDocumentTxnRelease(transaction);
        CHECK(xuiDocumentAcquireSnapshot(document[variant], &snapshot) == XUI_OK &&
            xuiDocumentRendererCreate(context, &desc, &renderer[variant]) == XUI_OK &&
            xuiDocumentRendererSetSnapshot(renderer[variant], snapshot, NULL) == XUI_OK);
        xuiDocumentSnapshotRelease(snapshot);
        surface.iKind = XUI_SURFACE_KIND_TEXTURE; surface.iFormat = XUI_SURFACE_FORMAT_RGBA8;
        surface.iWidth = 520; surface.iHeight = 160;
        surface.iFlags = XUI_SURFACE_USAGE_TARGET | XUI_SURFACE_ALPHA_PREMULTIPLIED;
        CHECK(proxy->surfaceCreate(proxy, &surfaces[variant], &surface) == XUI_OK);
    }
    for (pass = 0; pass < 3; pass++) {
        for (variant = 0; variant < 2; variant++) {
            xui_doc_position_t end = decoration_position(document[variant], last[variant], variant ? 1 : 5);
            CHECK(xuiDocumentRendererLayout(renderer[variant], pass == 1 ? 30 : 120, 0, 160) == XUI_OK &&
                xuiDocumentRendererGetNodeRect(renderer[variant], object[variant], &bounds[variant]) == XUI_OK &&
                xuiDocumentRendererGetCaretRect(renderer[variant], &end, &caret[variant]) == XUI_OK);
        }
        CHECK(fabs(bounds[0].x - bounds[1].x) < .01 && fabs(bounds[0].y - bounds[1].y) < .01 &&
            fabs(bounds[0].width - bounds[1].width) < .01 && fabs(bounds[0].height - bounds[1].height) < .01 &&
            fabs(caret[0].x - caret[1].x) < .01 && fabs(caret[0].y - caret[1].y) < .01);
    }
    for (variant = 0; variant < 2; variant++) {
        xui_draw_context draw; probe.draws = 0;
        CHECK(proxy->surfaceClear(proxy, surfaces[variant], 0) == XUI_OK &&
            proxy->drawBegin(proxy, &draw, surfaces[variant]) == XUI_OK &&
            xuiDocumentRendererDraw(renderer[variant], draw, 0, 0, (xui_rect_t){0, 0, 520, 160}, NULL, 0) == XUI_OK &&
            proxy->drawEnd(proxy, draw) == XUI_OK && probe.draws == 1 &&
            proxy->surfaceReadRGBA(proxy, surfaces[variant], variant ? second_pixels : first_pixels, 520 * 4) == XUI_OK);
    }
    CHECK(!memcmp(first_pixels, second_pixels, 520 * 160 * 4));
    for (at = 0; at < 520 * 160; at++) {
        const unsigned char* p = first_pixels + at * 4;
        if (p[3] < 64) continue;
        if (p[2] > p[0] + 40 && p[2] > p[1] + 40) blue++;
        else if (p[0] > p[1] + 40 && p[0] > p[2] + 40) red++;
        else if (p[0] > p[1] + 40 && p[2] > p[1] + 40) purple++;
    }
    CHECK(blue > 20 && red > 20 && purple > 400);
    for (variant = 0; variant < 2; variant++) {
        proxy->surfaceDestroy(proxy, surfaces[variant]);
        xuiDocumentRendererRelease(renderer[variant]); xuiDocumentRelease(document[variant]);
    }
    puts("Native XGE Arial object/script/raw-break split/join RGBA pixels, glyph/object presence, wrapping and object geometry passed");
}
static void native_break_font_pixels(xui_proxy proxy, xui_context context, xui_font font,
    unsigned char* first_pixels, unsigned char* second_pixels)
{
    unsigned sample, variant, pass;
    const uint32_t color = XUI_COLOR_RGBA(20, 40, 180, 255);
    for (sample = 0; sample < 4; sample++) {
        xui_document document[2]; xui_document_renderer renderer[2]; xui_surface surfaces[2];
        xui_doc_renderer_desc_t desc = {0}; uint64_t last[2];
        uint32_t marks = XUI_DOC_UNDERLINE | XUI_DOC_STRIKE | XUI_DOC_HIGHLIGHT |
            (sample & 1 ? XUI_DOC_SUPERSCRIPT : 0);
        desc.iSize = sizeof(desc); desc.tFonts = (xui_doc_font_set_t){font, font, font, font, font};
        desc.iHighlightColor = XUI_COLOR_RGBA(200, 220, 240, 255);
        for (variant = 0; variant < 2; variant++) {
            xui_document_transaction transaction; xui_document_snapshot snapshot;
            xui_doc_node_desc_t node = {0}; xui_surface_desc_t surface = {0}; uint64_t paragraph, separator;
            CHECK(xuiDocumentCreate(NULL, &document[variant]) == XUI_OK &&
                xuiDocumentBeginTransaction(document[variant], NULL, &transaction) == XUI_OK);
            paragraph = decoration_insert(transaction, 1, NULL, 0, 0);
            decoration_insert(transaction, paragraph, variant ? "AV" : sample >= 2 ? "AV\n" : "AV ", marks, color);
            if (variant) {
                node.iSize = sizeof(node); node.iKind = sample >= 2 ? XUI_DOC_HARD_BREAK : XUI_DOC_SOFT_BREAK;
                node.tAttributes.iMarks = marks; node.tAttributes.iTextColor = color;
                CHECK(xuiDocumentTxnInsertNode(transaction, paragraph, XUI_DOCUMENT_APPEND, &node, &separator) == XUI_OK);
            }
            last[variant] = decoration_insert(transaction, paragraph, "AV", marks, color);
            CHECK(xuiDocumentTxnCommit(transaction, NULL) == XUI_OK); xuiDocumentTxnRelease(transaction);
            CHECK(xuiDocumentAcquireSnapshot(document[variant], &snapshot) == XUI_OK &&
                xuiDocumentRendererCreate(context, &desc, &renderer[variant]) == XUI_OK &&
                xuiDocumentRendererSetSnapshot(renderer[variant], snapshot, NULL) == XUI_OK);
            xuiDocumentSnapshotRelease(snapshot);
            surface.iKind = XUI_SURFACE_KIND_TEXTURE; surface.iFormat = XUI_SURFACE_FORMAT_RGBA8;
            surface.iWidth = 520; surface.iHeight = 160;
            surface.iFlags = XUI_SURFACE_USAGE_TARGET | XUI_SURFACE_ALPHA_PREMULTIPLIED;
            CHECK(proxy->surfaceCreate(proxy, &surfaces[variant], &surface) == XUI_OK);
        }
        for (pass = 0; pass < 3; pass++) {
            xui_doc_rect_t caret[2]; size_t i; unsigned glyph_pixels = 0;
            for (variant = 0; variant < 2; variant++) {
                xui_draw_context draw; xui_doc_position_t at = decoration_position(document[variant], last[variant], 2);
                CHECK(xuiDocumentRendererLayout(renderer[variant], pass == 1 ? 18 : 120, 0, 160) == XUI_OK &&
                    xuiDocumentRendererGetCaretRect(renderer[variant], &at, &caret[variant]) == XUI_OK &&
                    proxy->surfaceClear(proxy, surfaces[variant], 0) == XUI_OK &&
                    proxy->drawBegin(proxy, &draw, surfaces[variant]) == XUI_OK &&
                    xuiDocumentRendererDraw(renderer[variant], draw, 0, 0, (xui_rect_t){0, 0, 520, 160}, NULL, 0) == XUI_OK &&
                    proxy->drawEnd(proxy, draw) == XUI_OK &&
                    proxy->surfaceReadRGBA(proxy, surfaces[variant], variant ? second_pixels : first_pixels, 520 * 4) == XUI_OK);
            }
            if (memcmp(first_pixels, second_pixels, 520 * 160 * 4)) {
                unsigned differences = 0; size_t first_difference = 520 * 160;
                for (i = 0; i < 520 * 160; i++) if (memcmp(first_pixels + i * 4, second_pixels + i * 4, 4)) {
                    if (!differences) first_difference = i;
                    differences++;
                }
                fprintf(stderr, "Native break sample=%u pass=%u caret=(%.3f,%.3f)/(%.3f,%.3f), %u differing pixels, first=(%zu,%zu)\n",
                    sample, pass, caret[0].x, caret[0].y, caret[1].x, caret[1].y,
                    differences, first_difference % 520, first_difference / 520);
            }
            CHECK(fabs(caret[0].x - caret[1].x) < .01 && fabs(caret[0].y - caret[1].y) < .01 &&
                fabs(caret[0].height - caret[1].height) < .01 && !memcmp(first_pixels, second_pixels, 520 * 160 * 4));
            for (i = 0; i < 520 * 160; i++) {
                const unsigned char* p = first_pixels + i * 4;
                if (p[3] >= 64 && p[2] > p[0] + 40 && p[2] > p[1] + 40) glyph_pixels++;
            }
            CHECK(glyph_pixels > 20);
        }
        for (variant = 0; variant < 2; variant++) {
            proxy->surfaceDestroy(proxy, surfaces[variant]);
            xuiDocumentRendererRelease(renderer[variant]); xuiDocumentRelease(document[variant]);
        }
    }
    puts("Native XGE Arial explicit soft/hard breaks match literal spaces/newlines in script, decoration, background, wrapped geometry and full RGBA pixels");
}
static void native_mandatory_break_pixels(xui_proxy proxy, xui_context context, xui_font font,
    unsigned char* first_pixels, unsigned char* second_pixels)
{
    unsigned sample, variant, pass, trailing;
    for (sample = 0; sample < sizeof(document_mandatory_breaks) / sizeof(*document_mandatory_breaks); sample++)
    for (trailing = 0; trailing < 2; trailing++) {
        xui_document document[2]; xui_document_renderer renderer[2]; xui_surface surfaces[2]; uint64_t text_node[2];
        xui_doc_renderer_desc_t desc = {0}; char text[24];
        desc.iSize = sizeof(desc); desc.tFonts = (xui_doc_font_set_t){font, font, font, font, font};
        desc.iHighlightColor = XUI_COLOR_RGBA(200, 220, 240, 255);
        for (variant = 0; variant < 2; variant++) {
            xui_document_transaction transaction; xui_document_snapshot snapshot;
            xui_surface_desc_t surface = {0}; uint64_t paragraph;
            snprintf(text, sizeof(text), "AV%s%s", variant ? document_mandatory_breaks[sample] : "\n", trailing ? "" : "AV");
            CHECK(xuiDocumentCreate(NULL, &document[variant]) == XUI_OK &&
                xuiDocumentBeginTransaction(document[variant], NULL, &transaction) == XUI_OK);
            paragraph = decoration_insert(transaction, 1, NULL, 0, 0);
            text_node[variant] = decoration_insert(transaction, paragraph, text,
                XUI_DOC_UNDERLINE | XUI_DOC_STRIKE | XUI_DOC_HIGHLIGHT, XUI_COLOR_RGBA(20, 40, 180, 255));
            CHECK(xuiDocumentTxnCommit(transaction, NULL) == XUI_OK); xuiDocumentTxnRelease(transaction);
            CHECK(xuiDocumentAcquireSnapshot(document[variant], &snapshot) == XUI_OK &&
                xuiDocumentRendererCreate(context, &desc, &renderer[variant]) == XUI_OK &&
                xuiDocumentRendererSetSnapshot(renderer[variant], snapshot, NULL) == XUI_OK);
            xuiDocumentSnapshotRelease(snapshot);
            surface.iKind = XUI_SURFACE_KIND_TEXTURE; surface.iFormat = XUI_SURFACE_FORMAT_RGBA8;
            surface.iWidth = 520; surface.iHeight = 160;
            surface.iFlags = XUI_SURFACE_USAGE_TARGET | XUI_SURFACE_ALPHA_PREMULTIPLIED;
            CHECK(proxy->surfaceCreate(proxy, &surfaces[variant], &surface) == XUI_OK);
        }
        for (pass = 0; pass < 3; pass++) {
            xui_doc_rect_t caret[2]; size_t pixel; unsigned colored = 0;
            for (variant = 0; variant < 2; variant++) {
                xui_draw_context draw; xui_doc_position_t at = decoration_position(document[variant], text_node[variant],
                    (trailing ? 2 : 4) + (variant ? strlen(document_mandatory_breaks[sample]) : 1));
                CHECK(xuiDocumentRendererLayout(renderer[variant], pass == 1 ? 18 : 120, 0, 160) == XUI_OK &&
                    xuiDocumentRendererGetCaretRect(renderer[variant], &at, &caret[variant]) == XUI_OK &&
                    proxy->surfaceClear(proxy, surfaces[variant], 0) == XUI_OK &&
                    proxy->drawBegin(proxy, &draw, surfaces[variant]) == XUI_OK &&
                    xuiDocumentRendererDraw(renderer[variant], draw, 0, 0, (xui_rect_t){0, 0, 520, 160}, NULL, 0) == XUI_OK &&
                    proxy->drawEnd(proxy, draw) == XUI_OK &&
                    proxy->surfaceReadRGBA(proxy, surfaces[variant], variant ? second_pixels : first_pixels, 520 * 4) == XUI_OK);
                if (trailing) CHECK(fabs(caret[variant].x) < .01 && caret[variant].y > 0);
            }
            if (fabs(caret[0].x-caret[1].x)>=.01 || fabs(caret[0].y-caret[1].y)>=.01 ||
                fabs(caret[0].height-caret[1].height)>=.01 || memcmp(first_pixels,second_pixels,520*160*4)) {
                size_t differences=0,first=520*160;
                for(pixel=0;pixel<520*160;pixel++)if(memcmp(first_pixels+pixel*4,second_pixels+pixel*4,4)){
                    if(!differences)first=pixel;
                    differences++;
                }
                fprintf(stderr,"Native mandatory sample=%u trailing=%u pass=%u caret=(%.6f,%.6f,%.6f)/(%.6f,%.6f,%.6f), %zu differing pixels first=(%zu,%zu)\n",
                    sample,trailing,pass,caret[0].x,caret[0].y,caret[0].height,caret[1].x,caret[1].y,caret[1].height,differences,first%520,first/520);
            }
            CHECK(fabs(caret[0].x - caret[1].x) < .01 && fabs(caret[0].y - caret[1].y) < .01 &&
                fabs(caret[0].height - caret[1].height) < .01 && !memcmp(first_pixels, second_pixels, 520 * 160 * 4));
            for (pixel = 0; pixel < 520 * 160; pixel++) {
                const unsigned char* p = first_pixels + pixel * 4;
                if (p[3] >= 64 && p[2] > p[0] + 40 && p[2] > p[1] + 40) colored++;
            }
            CHECK(colored > 20);
        }
        for (variant = 0; variant < 2; variant++) {
            proxy->surfaceDestroy(proxy, surfaces[variant]);
            xuiDocumentRendererRelease(renderer[variant]); xuiDocumentRelease(document[variant]);
        }
    }
    puts("Native XGE Arial eight mandatory controls and trailing empty rows match LF in reflow, caret, decorations/background and full RGBA pixels");
}
static void native_boundary_affinity_pixels(xui_proxy proxy, xui_context context, xui_font font,
    unsigned char* first_pixels, unsigned char* second_pixels)
{
    unsigned sample, styled, variant, pass;
    for (sample = 0; sample < sizeof(document_mandatory_breaks) / sizeof(*document_mandatory_breaks); sample++)
    for (styled = 0; styled < 2; styled++) {
        xui_document document[2]; xui_document_renderer renderer[2]; xui_surface surfaces[2];
        uint64_t controls[2], carriers[2]; size_t ends[2]; xui_doc_renderer_desc_t desc = {0};
        desc.iSize = sizeof(desc); desc.fLineGap = 4; desc.tFonts = (xui_doc_font_set_t){font, font, font, font, font};
        for (variant = 0; variant < 2; variant++) {
            xui_document_transaction transaction; xui_document_snapshot snapshot; xui_doc_node_desc_t node = {0};
            xui_surface_desc_t surface = {0}; uint64_t paragraph; char text[16];
            CHECK(xuiDocumentCreate(NULL, &document[variant]) == XUI_OK &&
                xuiDocumentBeginTransaction(document[variant], NULL, &transaction) == XUI_OK);
            paragraph = decoration_insert(transaction, 1, NULL, 0, 0);
            if (!variant) {
                snprintf(text, sizeof(text), "AV%s", document_mandatory_breaks[sample]);
                controls[variant] = decoration_insert(transaction, paragraph, text, XUI_DOC_UNDERLINE, XUI_COLOR_RGBA(20, 40, 180, 255));
                ends[variant] = strlen(text);
            } else {
                decoration_insert(transaction, paragraph, "A", XUI_DOC_UNDERLINE, XUI_COLOR_RGBA(20, 40, 180, 255));
                decoration_insert(transaction, paragraph, "V", XUI_DOC_UNDERLINE, XUI_COLOR_RGBA(20, 40, 180, 255));
                if (sample == 2) decoration_insert(transaction, paragraph, "\r", 0, 0);
                controls[variant] = decoration_insert(transaction, paragraph, sample == 2 ? "\n" : document_mandatory_breaks[sample], 0, 0);
                ends[variant] = sample == 2 ? 1 : strlen(document_mandatory_breaks[sample]);
            }
            if (styled) {
                node.iSize = sizeof(node); node.iKind = XUI_DOC_TEXT; node.sText = ""; node.tAttributes.fFontSize = 40;
                CHECK(xuiDocumentTxnInsertNode(transaction, paragraph, XUI_DOCUMENT_APPEND, &node, &carriers[variant]) == XUI_OK);
            }
            {
                uint64_t next = decoration_insert(transaction, paragraph, variant ? "A" : "AV", XUI_DOC_UNDERLINE, XUI_COLOR_RGBA(20, 40, 180, 255));
                if (!styled) carriers[variant] = next;
                if (variant) decoration_insert(transaction, paragraph, "V", XUI_DOC_UNDERLINE, XUI_COLOR_RGBA(20, 40, 180, 255));
            }
            CHECK(xuiDocumentTxnCommit(transaction, NULL) == XUI_OK); xuiDocumentTxnRelease(transaction);
            CHECK(xuiDocumentAcquireSnapshot(document[variant], &snapshot) == XUI_OK &&
                xuiDocumentRendererCreate(context, &desc, &renderer[variant]) == XUI_OK &&
                xuiDocumentRendererSetSnapshot(renderer[variant], snapshot, NULL) == XUI_OK); xuiDocumentSnapshotRelease(snapshot);
            surface.iKind = XUI_SURFACE_KIND_TEXTURE; surface.iFormat = XUI_SURFACE_FORMAT_RGBA8;
            surface.iWidth = 520; surface.iHeight = 160; surface.iFlags = XUI_SURFACE_USAGE_TARGET | XUI_SURFACE_ALPHA_PREMULTIPLIED;
            CHECK(proxy->surfaceCreate(proxy, &surfaces[variant], &surface) == XUI_OK);
        }
        for (pass = 0; pass < 3; pass++) {
            xui_doc_rect_t before[2], after[2], expected; size_t pixel; unsigned colored = 0;
            for (variant = 0; variant < 2; variant++) {
                xui_draw_context draw; xui_doc_position_t at = decoration_position(document[variant], controls[variant], ends[variant]);
                xui_doc_position_t next = decoration_position(document[variant], carriers[variant], 0);
                CHECK(xuiDocumentRendererLayout(renderer[variant], pass == 1 ? 18 : 120, 0, 160) == XUI_OK &&
                    xuiDocumentRendererGetCaretRect(renderer[variant], &at, &before[variant]) == XUI_OK);
                at.iAffinity = XUI_DOC_AFTER;
                CHECK(xuiDocumentRendererGetCaretRect(renderer[variant], &at, &after[variant]) == XUI_OK &&
                    xuiDocumentRendererGetCaretRect(renderer[variant], &next, &expected) == XUI_OK &&
                    fabs(after[variant].x - expected.x) < .01 && fabs(after[variant].y - expected.y) < .01 &&
                    fabs(after[variant].height - expected.height) < .01 && after[variant].y > before[variant].y &&
                    proxy->surfaceClear(proxy, surfaces[variant], 0) == XUI_OK &&
                    proxy->drawBegin(proxy, &draw, surfaces[variant]) == XUI_OK &&
                    xuiDocumentRendererDraw(renderer[variant], draw, 0, 0, (xui_rect_t){0, 0, 520, 160}, NULL, 0) == XUI_OK &&
                    proxy->drawEnd(proxy, draw) == XUI_OK &&
                    proxy->surfaceReadRGBA(proxy, surfaces[variant], variant ? second_pixels : first_pixels, 520 * 4) == XUI_OK);
            }
            CHECK(fabs(before[0].x - before[1].x) < .01 && fabs(before[0].y - before[1].y) < .01 &&
                fabs(after[0].x - after[1].x) < .01 && fabs(after[0].y - after[1].y) < .01 &&
                fabs(after[0].height - after[1].height) < .01 && !memcmp(first_pixels, second_pixels, 520 * 160 * 4));
            for (pixel = 0; pixel < 520 * 160; pixel++) {
                const unsigned char* p = first_pixels + pixel * 4;
                if (p[3] >= 64 && p[2] > p[0] + 40 && p[2] > p[1] + 40) colored++;
            }
            CHECK(colored > 20);
        }
        for (variant = 0; variant < 2; variant++) {
            proxy->surfaceDestroy(proxy, surfaces[variant]); xuiDocumentRendererRelease(renderer[variant]); xuiDocumentRelease(document[variant]);
        }
    }
    puts("Native XGE Arial mandatory boundary affinity, split CRLF/empty 40-font carriers, reflow and identical full RGBA pixels passed");
}
static void native_soft_wrap_pixels(xui_proxy proxy, xui_context context, xui_font font,
    unsigned char* first_pixels, unsigned char* second_pixels)
{
    unsigned styled, variant, pass;
    for (styled = 0; styled < 3; styled++) {
        xui_document document[2]; xui_document_renderer renderer[2]; xui_surface surfaces[2];
        uint64_t first[2], next[2], empty[2] = {0}; xui_doc_renderer_desc_t desc = {0};
        desc.iSize = sizeof(desc); desc.fLineGap = 4; desc.tFonts = (xui_doc_font_set_t){font, font, font, font, font};
        for (variant = 0; variant < 2; variant++) {
            xui_document_transaction transaction; xui_document_snapshot snapshot; xui_doc_node_desc_t node = {0};
            xui_surface_desc_t surface = {0}; uint64_t paragraph, unused; unsigned part;
            CHECK(xuiDocumentCreate(NULL, &document[variant]) == XUI_OK &&
                xuiDocumentBeginTransaction(document[variant], NULL, &transaction) == XUI_OK);
            paragraph = decoration_insert(transaction, 1, NULL, 0, 0);
            if (variant) decoration_insert(transaction, paragraph, "A", XUI_DOC_UNDERLINE, XUI_COLOR_RGBA(20, 40, 180, 255));
            first[variant] = decoration_insert(transaction, paragraph, variant ? "V " : "AV ", XUI_DOC_UNDERLINE, XUI_COLOR_RGBA(20, 40, 180, 255));
            node.iSize = sizeof(node); node.iKind = XUI_DOC_TEXT;
            if (styled == 1) {
                node.sText = ""; node.tAttributes.fFontSize = 40;
                CHECK(xuiDocumentTxnInsertNode(transaction, paragraph, XUI_DOCUMENT_APPEND, &node, &empty[variant]) == XUI_OK);
            }
            node.tAttributes.fFontSize = styled == 2 ? 40 : 0;
            node.tAttributes.iFlags = XUI_DOC_UNDERLINE; node.tAttributes.iTextColor = XUI_COLOR_RGBA(180, 20, 40, 255);
            for (part = 0; part < (variant ? 2u : 1u); part++) {
                node.sText = variant ? (part ? "V" : "A") : "AV"; node.iTextBytes = strlen(node.sText);
                CHECK(xuiDocumentTxnInsertNode(transaction, paragraph, XUI_DOCUMENT_APPEND, &node, &unused) == XUI_OK);
                if (!part) next[variant] = unused;
            }
            CHECK(xuiDocumentTxnCommit(transaction, NULL) == XUI_OK); xuiDocumentTxnRelease(transaction);
            CHECK(xuiDocumentAcquireSnapshot(document[variant], &snapshot) == XUI_OK &&
                xuiDocumentRendererCreate(context, &desc, &renderer[variant]) == XUI_OK &&
                xuiDocumentRendererSetSnapshot(renderer[variant], snapshot, NULL) == XUI_OK); xuiDocumentSnapshotRelease(snapshot);
            surface.iKind = XUI_SURFACE_KIND_TEXTURE; surface.iFormat = XUI_SURFACE_FORMAT_RGBA8;
            surface.iWidth = 520; surface.iHeight = 160; surface.iFlags = XUI_SURFACE_USAGE_TARGET | XUI_SURFACE_ALPHA_PREMULTIPLIED;
            CHECK(proxy->surfaceCreate(proxy, &surfaces[variant], &surface) == XUI_OK);
        }
        for (pass = 0; pass < 4; pass++) {
            xui_doc_rect_t before[2], after[2]; size_t pixel; unsigned blue = 0, red = 0;
            for (variant = 0; variant < 2; variant++) {
                xui_draw_context draw; xui_doc_rect_t reference, clicked;
                xui_doc_position_t at = decoration_position(document[variant], first[variant], variant ? 2 : 3), hit;
                xui_doc_position_t target = decoration_position(document[variant], next[variant], 0); target.iAffinity = XUI_DOC_AFTER;
                CHECK(xuiDocumentRendererLayout(renderer[variant], pass == 1 ? 35 : pass == 2 ? 18 : 120, 0, 160) == XUI_OK &&
                    xuiDocumentRendererGetCaretRect(renderer[variant], &at, &before[variant]) == XUI_OK);
                at.iAffinity = XUI_DOC_AFTER;
                CHECK(xuiDocumentRendererGetCaretRect(renderer[variant], &at, &after[variant]) == XUI_OK &&
                    xuiDocumentRendererGetCaretRect(renderer[variant], &target, &reference) == XUI_OK);
                if (empty[variant]) {
                    xui_doc_rect_t carrier; xui_doc_position_t zero = decoration_position(document[variant], empty[variant], 0);
                    CHECK(xuiDocumentRendererGetCaretRect(renderer[variant], &zero, &carrier) == XUI_OK);
                    if (carrier.y > before[variant].y) reference = carrier;
                }
                if (pass == 1 || pass == 2) {
                    CHECK(after[variant].y > before[variant].y && fabs(after[variant].x - reference.x) < .01 &&
                        fabs(after[variant].y - reference.y) < .01 && fabs(after[variant].height - reference.height) < .01 &&
                        xuiDocumentRendererHitTest(renderer[variant], before[variant].x + 100, before[variant].y + before[variant].height * .5, &hit) == XUI_OK &&
                        xuiDocumentRendererGetCaretRect(renderer[variant], &hit, &clicked) == XUI_OK && fabs(clicked.y - before[variant].y) < .01);
                } else CHECK(fabs(after[variant].x - before[variant].x) < .01 && fabs(after[variant].y - before[variant].y) < .01);
                CHECK(proxy->surfaceClear(proxy, surfaces[variant], 0) == XUI_OK && proxy->drawBegin(proxy, &draw, surfaces[variant]) == XUI_OK &&
                    xuiDocumentRendererDraw(renderer[variant], draw, 0, 0, (xui_rect_t){0, 0, 520, 160}, NULL, 0) == XUI_OK &&
                    proxy->drawEnd(proxy, draw) == XUI_OK &&
                    proxy->surfaceReadRGBA(proxy, surfaces[variant], variant ? second_pixels : first_pixels, 520 * 4) == XUI_OK);
            }
            CHECK(fabs(before[0].x - before[1].x) < .01 && fabs(before[0].y - before[1].y) < .01 &&
                fabs(after[0].x - after[1].x) < .01 && fabs(after[0].y - after[1].y) < .01 &&
                fabs(after[0].height - after[1].height) < .01 && !memcmp(first_pixels, second_pixels, 520 * 160 * 4));
            for (pixel = 0; pixel < 520 * 160; pixel++) {
                const unsigned char* p = first_pixels + pixel * 4;
                if (p[3] >= 64 && p[2] > p[0] + 40 && p[2] > p[1] + 40) blue++;
                if (p[3] >= 64 && p[0] > p[1] + 40 && p[0] > p[2] + 40) red++;
            }
            CHECK(blue > 10 && red > 10);
        }
        for (variant = 0; variant < 2; variant++) {
            proxy->surfaceDestroy(proxy, surfaces[variant]); xuiDocumentRendererRelease(renderer[variant]); xuiDocumentRelease(document[variant]);
        }
    }
    puts("Native XGE Arial soft-wrap affinity, split colors, mixed/empty 40-font nodes, row-end hit, reflow and identical full RGBA pixels passed");
}
static void native_format_pixels(xui_proxy proxy, xui_context context, xui_font font,
    unsigned char* first_pixels, unsigned char* second_pixels)
{
    unsigned sample, variant;
    for (sample = 0; sample < 8; sample++) {
        xui_document document[2]; xui_document_renderer renderer[2]; xui_surface surface[2];
        xui_doc_renderer_desc_t desc = {0}; uint64_t control[2], next[2]; size_t offsets[2]; double width = 120;
        xui_doc_rect_t before[2], after, reference;
        desc.iSize = sizeof(desc); desc.fLineGap = 4; desc.tFonts = (xui_doc_font_set_t){font, font, font, font, font};
        if (sample >= 4) {
            xui_font sized = font; xui_text_shape_t prefix = {0}, hyphen = {0};
            if (sample == 5 || sample == 7) CHECK(proxy->fontCreateSized(proxy, &sized, font, 40) == XUI_OK);
            CHECK(xuiTextShape(context, &(xui_text_item_t){.iSize=sizeof(xui_text_item_t), .pFont=font, .sText="AV", .iTextSize=2, .iFlags=0}, &prefix) == XUI_OK && xuiTextShape(context, &(xui_text_item_t){.iSize=sizeof(xui_text_item_t), .pFont=sized, .sText="-", .iTextSize=1, .iFlags=0}, &hyphen) == XUI_OK);
            width = prefix.fWidth + hyphen.fWidth + .5;
            xuiTextShapeFree(&prefix); xuiTextShapeFree(&hyphen);
            if (sized != font) proxy->fontDestroy(proxy, sized);
        }
        for (variant = 0; variant < 2; variant++) {
            xui_document_transaction transaction; xui_document_snapshot snapshot; xui_surface_desc_t target = {0};
            uint64_t paragraph; char text[16];
            CHECK(xuiDocumentCreate(NULL, &document[variant]) == XUI_OK && xuiDocumentBeginTransaction(document[variant], NULL, &transaction) == XUI_OK);
            paragraph = decoration_insert(transaction, 1, NULL, 0, 0);
            if (sample < 4) {
                snprintf(text, sizeof(text), "A%sV", variant ? "" : document_formats[sample]);
                control[variant] = decoration_insert(transaction, paragraph, text, XUI_DOC_UNDERLINE, XUI_COLOR_RGBA(20, 40, 180, 255));
                offsets[variant] = strlen(text); next[variant] = control[variant];
            } else {
                xui_doc_node_desc_t node = {0}; uint64_t hard;
                decoration_insert(transaction, paragraph, "AV", XUI_DOC_UNDERLINE, XUI_COLOR_RGBA(20, 40, 180, 255));
                node.iSize = sizeof(node); node.iKind = XUI_DOC_TEXT; node.sText = variant ? "-" : "\xc2\xad";
                node.iTextBytes = offsets[variant] = strlen(node.sText); node.tAttributes.fFontSize = sample == 5 || sample == 7 ? 40 : 0;
                node.tAttributes.iMarks = XUI_DOC_UNDERLINE; node.tAttributes.iTextColor = XUI_COLOR_RGBA(20, 40, 180, 255);
                CHECK(xuiDocumentTxnInsertNode(transaction, paragraph, XUI_DOCUMENT_APPEND, &node, &control[variant]) == XUI_OK);
                if (variant) {
                    node.iKind = XUI_DOC_HARD_BREAK; node.sText = NULL; node.iTextBytes = 0;
                    CHECK(xuiDocumentTxnInsertNode(transaction, paragraph, XUI_DOCUMENT_APPEND, &node, &hard) == XUI_OK);
                }
                next[variant] = decoration_insert(transaction, paragraph, sample >= 6 ? "AV\xce\xbb" : "AV", XUI_DOC_UNDERLINE, XUI_COLOR_RGBA(20, 40, 180, 255));
            }
            CHECK(xuiDocumentTxnCommit(transaction, NULL) == XUI_OK); xuiDocumentTxnRelease(transaction);
            CHECK(xuiDocumentAcquireSnapshot(document[variant], &snapshot) == XUI_OK && xuiDocumentRendererCreate(context, &desc, &renderer[variant]) == XUI_OK &&
                xuiDocumentRendererSetSnapshot(renderer[variant], snapshot, NULL) == XUI_OK && xuiDocumentRendererLayout(renderer[variant], width, 0, 160) == XUI_OK);
            xuiDocumentSnapshotRelease(snapshot);
            if (sample >= 6 && !variant) {
                size_t g; unsigned recipes = 0;
                CHECK(renderer[variant]->blocks[0].context_count == 1 && renderer[variant]->blocks[0].contexts[0]->hyphen_text);
                for (g = 0; g < renderer[variant]->blocks[0].paint_group_count; g++) recipes += renderer[variant]->blocks[0].paint_groups[g].has_hyphen_context != 0;
                if(renderer[variant]->blocks[0].variants){
                    const doc_render_paint_variant* full=renderer[variant]->blocks[0].variants;
                    CHECK(full->bytes==7 && !memcmp(full->text,"AV-AV\xce\xbb",8) && full->span_count);
                }else CHECK(recipes > 0);
            }
            {
                xui_doc_position_t at = decoration_position(document[variant], control[variant], offsets[variant]);
                CHECK(xuiDocumentRendererGetCaretRect(renderer[variant], &at, &before[variant]) == XUI_OK);
            }
            target.iKind = XUI_SURFACE_KIND_TEXTURE; target.iFormat = XUI_SURFACE_FORMAT_RGBA8; target.iWidth = 520; target.iHeight = 160;
            target.iFlags = XUI_SURFACE_USAGE_TARGET | XUI_SURFACE_ALPHA_PREMULTIPLIED;
            CHECK(proxy->surfaceCreate(proxy, &surface[variant], &target) == XUI_OK && proxy->surfaceClear(proxy, surface[variant], 0) == XUI_OK);
            {
                xui_draw_context draw;
                CHECK(proxy->drawBegin(proxy, &draw, surface[variant]) == XUI_OK &&
                    xuiDocumentRendererDraw(renderer[variant], draw, 0, 0, (xui_rect_t){0, 0, 520, 160}, NULL, 0) == XUI_OK && proxy->drawEnd(proxy, draw) == XUI_OK &&
                    proxy->surfaceReadRGBA(proxy, surface[variant], variant ? second_pixels : first_pixels, 520 * 4) == XUI_OK);
            }
        }
        CHECK(fabs(before[0].x - before[1].x) < .01 && fabs(before[0].y - before[1].y) < .01 &&
            fabs(before[0].height - before[1].height) < .01 && !memcmp(first_pixels, second_pixels, 520 * 160 * 4));
        if (sample >= 4) {
            xui_doc_position_t at = decoration_position(document[0], control[0], 2);
            xui_doc_position_t target = decoration_position(document[1], next[1], 0);
            at.iAffinity = XUI_DOC_AFTER;
            CHECK(xuiDocumentRendererGetCaretRect(renderer[0], &at, &after) == XUI_OK && xuiDocumentRendererGetCaretRect(renderer[1], &target, &reference) == XUI_OK &&
                fabs(after.x - reference.x) < .01 && fabs(after.y - reference.y) < .01 && fabs(after.height - reference.height) < .01 && after.y > before[0].y);
        }
        {
            size_t pixel; unsigned colored = 0;
            for (pixel = 0; pixel < 520 * 160; pixel++) {
                const unsigned char* p = first_pixels + pixel * 4;
                if (p[3] >= 64 && p[2] > p[0] + 40 && p[2] > p[1] + 40) colored++;
            }
            CHECK(colored > 20);
        }
        for (variant = 0; variant < 2; variant++) {
            proxy->surfaceDestroy(proxy, surface[variant]); xuiDocumentRendererRelease(renderer[variant]); xuiDocumentRelease(document[variant]);
        }
    }
    puts("Native XGE Arial invisible-format deletion and 20/40-font ASCII/Greek-context discretionary hyphens match explicit text/hard rows in caret and full RGBA pixels passed");
}
static void native_joint_scale_pixels(xui_proxy proxy, xui_context context, xui_font font,
    unsigned char* first_pixels, unsigned char* second_pixels)
{
    static const size_t lengths[] = {9000, 105, 90000}; unsigned sample, variant, pass;
    const uint32_t blue = XUI_COLOR_RGBA(20, 40, 180, 255), red = XUI_COLOR_RGBA(180, 20, 40, 255);
    for (sample = 0; sample < 3; sample++) {
        char* text = malloc(lengths[sample] + 1); size_t i;
        xui_document documents[2]; xui_document_renderer renderers[2]; xui_surface surfaces[2];
        uint64_t first[2], tail[2]; xui_doc_renderer_desc_t desc = {0};
        CHECK(text); for (i = 0; i < lengths[sample]; i += 3) memcpy(text + i, "AV ", 3); text[lengths[sample]] = 0;
        desc.iSize = sizeof(desc); desc.tFonts = (xui_doc_font_set_t){font, font, font, font, font}; desc.fLineGap = 4;
        for (variant = 0; variant < 2; variant++) {
            xui_document_transaction transaction; xui_document_snapshot snapshot; uint64_t paragraph;
            xui_surface_desc_t target = {0};
            CHECK(xuiDocumentCreate(NULL, &documents[variant]) == XUI_OK &&
                xuiDocumentBeginTransaction(documents[variant], NULL, &transaction) == XUI_OK);
            paragraph = decoration_insert(transaction, 1, NULL, 0, 0);
            if (sample == 1) {
                for (i = 0; i < lengths[sample]; i += variant ? 1 : 3) {
                    char word[4]; size_t bytes = variant ? 1 : 3; uint64_t id;
                    memcpy(word, text + i, bytes); word[bytes] = 0;
                    id = decoration_insert(transaction, paragraph, word, 0, (i / 3) & 1 ? red : blue);
                    if ((!variant && !i) || (variant && i == 1)) first[variant] = id;
                    tail[variant] = id;
                }
            } else if (variant) {
                first[variant] = decoration_insert(transaction, paragraph, "A", 0, blue);
                tail[variant] = decoration_insert(transaction, paragraph, text + 1, 0, blue);
            } else first[variant] = tail[variant] = decoration_insert(transaction, paragraph, text, 0, blue);
            CHECK(xuiDocumentTxnCommit(transaction, NULL) == XUI_OK); xuiDocumentTxnRelease(transaction);
            CHECK(xuiDocumentAcquireSnapshot(documents[variant], &snapshot) == XUI_OK &&
                xuiDocumentRendererCreate(context, &desc, &renderers[variant]) == XUI_OK &&
                xuiDocumentRendererSetSnapshot(renderers[variant], snapshot, NULL) == XUI_OK); xuiDocumentSnapshotRelease(snapshot);
            target.iKind = XUI_SURFACE_KIND_TEXTURE; target.iFormat = XUI_SURFACE_FORMAT_RGBA8; target.iWidth = 520; target.iHeight = 160;
            target.iFlags = XUI_SURFACE_USAGE_TARGET | XUI_SURFACE_ALPHA_PREMULTIPLIED;
            CHECK(proxy->surfaceCreate(proxy, &surfaces[variant], &target) == XUI_OK);
        }
        for (pass = 0; pass < 3; pass++) {
            xui_doc_rect_t caret[2], deep[2]; double width = pass == 1 ? 50 : sample == 1 ? 2000 : 150;
            for (variant = 0; variant < 2; variant++) {
                xui_doc_position_t at; xui_draw_context draw;
                uint64_t node = sample == 1 ? first[variant] : tail[variant];
                CHECK(xuiDocumentRendererLayout(renderers[variant], width, 0, 160) == XUI_OK);
                at = decoration_position(documents[variant], node, variant ? 1 : 2);
                CHECK(xuiDocumentRendererGetCaretRect(renderers[variant], &at, &caret[variant]) == XUI_OK);
                if (sample == 2) {
                    at = decoration_position(documents[variant], tail[variant], 8192 - variant);
                    CHECK(xuiDocumentRendererGetCaretRect(renderers[variant], &at, &deep[variant]) == XUI_OK);
                }
                CHECK(proxy->surfaceClear(proxy, surfaces[variant], 0) == XUI_OK && proxy->drawBegin(proxy, &draw, surfaces[variant]) == XUI_OK &&
                    xuiDocumentRendererDraw(renderers[variant], draw, 0, 0, (xui_rect_t){0, 0, 520, 160}, NULL, 0) == XUI_OK &&
                    proxy->drawEnd(proxy, draw) == XUI_OK && proxy->surfaceReadRGBA(proxy, surfaces[variant], variant ? second_pixels : first_pixels, 520 * 4) == XUI_OK);
            }
            CHECK(fabs(caret[0].x - caret[1].x) < .01 && fabs(caret[0].y - caret[1].y) < .01 &&
                !memcmp(first_pixels, second_pixels, 520 * 160 * 4));
            if (sample == 2) CHECK(fabs(deep[0].x - deep[1].x) < .01 && fabs(deep[0].y - deep[1].y) < .01 &&
                fabs(deep[0].height - deep[1].height) < .01);
            {
                size_t pixel; unsigned blue_pixels = 0, red_pixels = 0;
                for (pixel = 0; pixel < 520 * 160; pixel++) {
                    const unsigned char* p = first_pixels + pixel * 4;
                    if (p[3] < 64) continue;
                    if (p[2] > p[0] + 40 && p[2] > p[1] + 40) blue_pixels++;
                    if (p[0] > p[2] + 40 && p[0] > p[1] + 40) red_pixels++;
                }
                CHECK(blue_pixels > 20 && (sample != 1 || red_pixels > 20));
            }
        }
        for (variant = 0; variant < 2; variant++) {
            proxy->surfaceDestroy(proxy, surfaces[variant]); xuiDocumentRendererRelease(renderers[variant]); xuiDocumentRelease(documents[variant]);
        }
        free(text);
    }
    puts("Native XGE Arial joint scale: 9 KiB, 105 color nodes, 90 KiB visible prefix/deep continuation and reflow match caret/full RGBA with real blue/red glyphs");
}
static int native_vertical_font_frame(void* user)
{
    xui_proxy_t proxy = xuiProxyXge(); xui_context context; xui_font font;
    xui_document document[2]; xui_document_renderer renderer[2]; uint64_t nodes[2][8] = {{0}};
    xui_surface surfaces[2]; xui_doc_renderer_desc_t desc = {0};
    unsigned char pixels[2][520 * 160 * 4]; xui_doc_rect_t carets[2][4];
    unsigned variant, part, pass; int y, x, top[4] = {160, 160, 160, 160}, bottom[4] = {0};
    unsigned counts[4] = {0};
    static const char* const joined[] = {"AV ", "AV ", "AV ", "AV"};
    static const char* const split[] = {"A", "V ", "A", "V ", "A", "V ", "A", "V"};
    const uint32_t colors[] = {XUI_COLOR_RGBA(20, 40, 180, 255), XUI_COLOR_RGBA(180, 20, 40, 255),
        XUI_COLOR_RGBA(20, 180, 40, 255), XUI_COLOR_RGBA(160, 40, 180, 255)};
    (void)user;
    CHECK(proxy.fontLoadFile(&proxy, &font, "C:\\Windows\\Fonts\\arial.ttf", 20, 0) == XUI_OK &&
        xuiCreate(&context) == XUI_OK && xuiSetProxy(context, &proxy) == XUI_OK &&
        xuiSetDefaultFont(context, font) == XUI_OK);
    desc.iSize = sizeof(desc); desc.tFonts = (xui_doc_font_set_t){font, font, font, font, font};
    for (variant = 0; variant < 2; variant++) {
        xui_document_transaction transaction; xui_document_snapshot snapshot; uint64_t paragraph;
        xui_surface_desc_t surface = {0};
        CHECK(xuiDocumentCreate(NULL, &document[variant]) == XUI_OK &&
            xuiDocumentBeginTransaction(document[variant], NULL, &transaction) == XUI_OK);
        paragraph = decoration_insert(transaction, 1, NULL, 0, 0);
        for (part = 0; part < (variant ? 8 : 4); part++) {
            unsigned zone = variant ? part / 2 : part;
            nodes[variant][part] = decoration_insert(transaction, paragraph,
                variant ? split[part] : joined[part], XUI_DOC_UNDERLINE |
                (zone == 1 ? XUI_DOC_SUPERSCRIPT : zone == 2 ? XUI_DOC_SUBSCRIPT : 0), colors[zone]);
        }
        CHECK(xuiDocumentTxnCommit(transaction, NULL) == XUI_OK); xuiDocumentTxnRelease(transaction);
        CHECK(xuiDocumentAcquireSnapshot(document[variant], &snapshot) == XUI_OK &&
            xuiDocumentRendererCreate(context, &desc, &renderer[variant]) == XUI_OK &&
            xuiDocumentRendererSetSnapshot(renderer[variant], snapshot, NULL) == XUI_OK);
        xuiDocumentSnapshotRelease(snapshot);
        surface.iKind = XUI_SURFACE_KIND_TEXTURE; surface.iFormat = XUI_SURFACE_FORMAT_RGBA8;
        surface.iWidth = 520; surface.iHeight = 160;
        surface.iFlags = XUI_SURFACE_USAGE_TARGET | XUI_SURFACE_ALPHA_PREMULTIPLIED;
        CHECK(proxy.surfaceCreate(&proxy, &surfaces[variant], &surface) == XUI_OK);
    }
    for (pass = 0; pass < 3; pass++)
        vertical_compare(document, renderer, nodes, pass == 1 ? 28 : 500, carets);
    CHECK(carets[1][1].y < carets[1][0].y && carets[1][2].y > carets[1][0].y &&
        carets[1][1].height < carets[1][0].height);
    for (variant = 0; variant < 2; variant++) {
        xui_draw_context draw;
        CHECK(proxy.surfaceClear(&proxy, surfaces[variant], 0) == XUI_OK &&
            proxy.drawBegin(&proxy, &draw, surfaces[variant]) == XUI_OK &&
            xuiDocumentRendererDraw(renderer[variant], draw, 0, 0, (xui_rect_t){0, 0, 520, 160}, NULL, 0) == XUI_OK &&
            proxy.drawEnd(&proxy, draw) == XUI_OK &&
            proxy.surfaceReadRGBA(&proxy, surfaces[variant], pixels[variant], 520 * 4) == XUI_OK);
    }
    CHECK(!memcmp(pixels[0], pixels[1], sizeof(pixels[0])));
    for (y = 0; y < 160; y++) for (x = 0; x < 520; x++) {
        const unsigned char* p = pixels[0] + (y * 520 + x) * 4;
        int zone = -1;
        if (p[3] < 64) continue;
        if (p[0] > p[1] + 40 && p[0] > p[2] + 40) zone = 1;
        else if (p[1] > p[0] + 40 && p[1] > p[2] + 40) zone = 2;
        else if (p[2] > p[1] + 40) zone = p[0] > p[1] + 40 ? 3 : 0;
        if (zone < 0) continue;
        counts[zone]++; if (y < top[zone]) top[zone] = y; if (y > bottom[zone]) bottom[zone] = y;
    }
    for (part = 0; part < 4; part++) CHECK(counts[part] > 5);
    CHECK(top[1] < top[0] && top[0] < top[2] && bottom[1] < bottom[0] && bottom[0] < bottom[2]);
    for (variant = 0; variant < 2; variant++) {
        proxy.surfaceDestroy(&proxy, surfaces[variant]);
        xuiDocumentRendererRelease(renderer[variant]); xuiDocumentRelease(document[variant]);
    }
    native_object_font_pixels(&proxy, context, font, pixels[0], pixels[1]);
    native_break_font_pixels(&proxy, context, font, pixels[0], pixels[1]);
    native_mandatory_break_pixels(&proxy, context, font, pixels[0], pixels[1]);
    native_boundary_affinity_pixels(&proxy, context, font, pixels[0], pixels[1]);
    native_soft_wrap_pixels(&proxy, context, font, pixels[0], pixels[1]);
    native_format_pixels(&proxy, context, font, pixels[0], pixels[1]);
    native_joint_scale_pixels(&proxy, context, font, pixels[0], pixels[1]);
    {
        xui_proxy_t plain = proxy; xui_context plain_context;
        plain.drawTextSpans = NULL;
        CHECK(xuiCreate(&plain_context) == XUI_OK && xuiSetProxy(plain_context, &plain) == XUI_OK &&
            xuiSetDefaultFont(plain_context, font) == XUI_OK);
        native_mandatory_break_pixels(&plain, plain_context, font, pixels[0], pixels[1]);
        native_boundary_affinity_pixels(&plain, plain_context, font, pixels[0], pixels[1]);
        native_format_pixels(&plain, plain_context, font, pixels[0], pixels[1]);
        native_joint_scale_pixels(&plain, plain_context, font, pixels[0], pixels[1]);
        xuiDestroy(plain_context);
        puts("DrawText-only native Arial: mandatory/trailing controls, boundary affinity, same-color and 105 color-node full-frame equivalence, reflow and 90 KiB continuation passed");
    }
    native_grapheme_font_pixels();
    xuiDestroy(context); proxy.fontDestroy(&proxy, font);
    puts("Native XGE Arial script sizing/kerning/reflow and identical split/join RGBA pixels with raised/lowered colors passed");
    xgeQuit();
    return XGE_OK;
}
static void native_vertical_font_geometry(void)
{
    xge_desc_t engine = {0};
    engine.iWidth = 520; engine.iHeight = 160; engine.sTitle = "Document script pixels";
    engine.iFlags = XGE_INIT_OFFSCREEN; engine.iRunMode = XGE_RUN_GAME_LOOP;
    CHECK(xgeInit(&engine) == XGE_OK && xgeRun(native_vertical_font_frame, NULL) == XGE_OK);
    xgeUnit();
}
static void native_mixed_font_geometry(void)
{
    xui_proxy_t proxy = xuiProxyXge(); xui_context context; xui_font normal, bold;
    xui_document document[2]; xui_document_renderer renderer[2]; uint64_t nodes[2][6] = {{0}};
    xui_doc_renderer_desc_t desc = {0}; unsigned variant, pass, part;
    static const char* const joined[] = {"AV ", "AV ", "AV"};
    static const char* const split[] = {"A", "V ", "A", "V ", "A", "V"};
    CHECK(proxy.fontLoadFile(&proxy, &normal, "C:\\Windows\\Fonts\\arial.ttf", 18, 0) == XUI_OK &&
        proxy.fontLoadFile(&proxy, &bold, "C:\\Windows\\Fonts\\arialbd.ttf", 30, 0) == XUI_OK &&
        xuiCreate(&context) == XUI_OK && xuiSetProxy(context, &proxy) == XUI_OK &&
        xuiSetDefaultFont(context, normal) == XUI_OK);
    desc.iSize = sizeof(desc); desc.tFonts = (xui_doc_font_set_t){normal, bold, normal, bold, normal};
    for (variant = 0; variant < 2; variant++) {
        xui_document_transaction transaction; xui_document_snapshot snapshot; uint64_t paragraph;
        CHECK(xuiDocumentCreate(NULL, &document[variant]) == XUI_OK &&
            xuiDocumentBeginTransaction(document[variant], NULL, &transaction) == XUI_OK);
        paragraph = add(transaction, 1, XUI_DOC_PARAGRAPH, NULL);
        for (part = 0; part < (variant ? 6 : 3); part++) {
            xui_doc_node_desc_t node = {0}; unsigned zone = variant ? part / 2 : part;
            node.iSize = sizeof(node); node.iKind = XUI_DOC_TEXT;
            node.sText = variant ? split[part] : joined[part]; node.iTextBytes = strlen(node.sText);
            node.tAttributes.iMarks = zone == 1 ? XUI_DOC_BOLD : 0;
            node.tAttributes.iTextColor = part & 1 ? XUI_COLOR_RGBA(180, 20, 40, 255) : 0;
            CHECK(xuiDocumentTxnInsertNode(transaction, paragraph, XUI_DOCUMENT_APPEND, &node, &nodes[variant][part]) == XUI_OK);
        }
        CHECK(xuiDocumentTxnCommit(transaction, NULL) == XUI_OK); xuiDocumentTxnRelease(transaction);
        CHECK(xuiDocumentAcquireSnapshot(document[variant], &snapshot) == XUI_OK &&
            xuiDocumentRendererCreate(context, &desc, &renderer[variant]) == XUI_OK &&
            xuiDocumentRendererSetSnapshot(renderer[variant], snapshot, NULL) == XUI_OK);
        xuiDocumentSnapshotRelease(snapshot);
    }
    for (pass = 0; pass < 3; pass++) {
        xui_doc_rect_t carets[2][3];
        for (variant = 0; variant < 2; variant++) {
            CHECK(xuiDocumentRendererLayout(renderer[variant], pass == 1 ? 28 : 500, 0, 400) == XUI_OK);
            for (part = 0; part < 3; part++) {
                xui_doc_position_t at = position(document[variant], nodes[variant][variant ? part * 2 + 1 : part],
                    part == 2 ? (variant ? 1 : 2) : (variant ? 2 : 3));
                CHECK(xuiDocumentRendererGetCaretRect(renderer[variant], &at, &carets[variant][part]) == XUI_OK);
            }
        }
        for (part = 0; part < 3; part++) CHECK(fabs(carets[0][part].x - carets[1][part].x) < .01 &&
            fabs(carets[0][part].y - carets[1][part].y) < .01 &&
            fabs(carets[0][part].height - carets[1][part].height) < .01);
    }
    for (variant = 0; variant < 2; variant++) {
        xuiDocumentRendererRelease(renderer[variant]); xuiDocumentRelease(document[variant]);
    }
    xuiDestroy(context); proxy.fontDestroy(&proxy, normal); proxy.fontDestroy(&proxy, bold);
    puts("Native XGE Arial regular/bold mixed font kerning, baseline and reflow geometry passed");
}
static void native_cross_node_emoji(void)
{
    static const char svg[] = "<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 36 36\"><path fill=\"#7dd3fc\" d=\"M0 0h36v36H0z\"/></svg>";
    xui_proxy_t proxy = xuiProxyXge();
    xui_context context; xui_font font;
    xge_emoji_pack previous = NULL, pack = NULL;
    xge_emoji_metrics_t metrics = {0}; uint32_t emoji_id = 0;
    xui_text_shape_t shape = {0};
    xui_document document[2]; xui_document_renderer renderer[2];
    uint64_t nodes[2][3] = {{0}}; unsigned variant, pass;
    double expected;
    metrics.iSize = sizeof(metrics); metrics.fAdvanceEm = 1.1f;
    metrics.fWidthEm = metrics.fHeightEm = 1; metrics.fBaselineRatio = .82f;
    CHECK(xgeEmojiPackGetDefault(&previous) == XGE_OK &&
        xgeEmojiPackCreate(&pack) == XGE_OK &&
        xgeEmojiPackAddSvgMemory(pack, grapheme_emoji, svg, sizeof(svg) - 1,
            &metrics, &emoji_id) == XGE_OK && emoji_id &&
        xgeEmojiPackSetDefault(pack) == XGE_OK);
    CHECK(proxy.fontLoadFile(&proxy, &font, "C:\\Windows\\Fonts\\arial.ttf", 18, 0) == XUI_OK &&
        xuiCreate(&context) == XUI_OK && xuiSetProxy(context, &proxy) == XUI_OK &&
        xuiSetDefaultFont(context, font) == XUI_OK &&
        xuiTextShape(context, &(xui_text_item_t){.iSize=sizeof(xui_text_item_t), .pFont=font, .sText=grapheme_emoji, .iTextSize=11, .iFlags=XUI_TEXT_SHAPE_DEFAULT}, &shape) == XUI_OK &&
        shape.iClusterCount == 1 && shape.pClusters[0].iTextStart == 0 &&
        shape.pClusters[0].iTextEnd == 11);
    {
        xge_font_t raw_font = {0}; xge_font_metrics_t raw_metrics = {0};
        xui_font_metrics_t font_metrics = {0};
        CHECK(xgeFontLoad(&raw_font, "C:\\Windows\\Fonts\\arial.ttf", 18) == XGE_OK &&
            xgeFontGetMetrics(&raw_font, &raw_metrics) == XGE_OK &&
            proxy.fontGetMetrics(&proxy, font, &font_metrics) == XUI_OK);
        CHECK(font_metrics.fUnderlinePosition > 0 && font_metrics.fUnderlineThickness > 0 &&
            font_metrics.fStrikePosition < 0 && font_metrics.fStrikeThickness > 0 &&
            fabsf(font_metrics.fUnderlinePosition - raw_metrics.fUnderlinePosition) < .00001f &&
            fabsf(font_metrics.fUnderlineThickness - raw_metrics.fUnderlineThickness) < .00001f &&
            fabsf(font_metrics.fStrikePosition - raw_metrics.fStrikePosition) < .00001f &&
            fabsf(font_metrics.fStrikeThickness - raw_metrics.fStrikeThickness) < .00001f);
        xgeFontFree(&raw_font);
        puts("Native XGE Arial font decoration metrics match the XUI proxy");
    }
    expected = shape.pClusters[0].fAdvance;
    xuiTextShapeFree(&shape);
    for (variant = 0; variant < 2; variant++) {
        xui_document_transaction transaction; xui_document_snapshot snapshot;
        xui_doc_node_desc_t node = {0}; uint64_t paragraph; unsigned part;
        static const char* const split[] = {"\xf0\x9f\x91\xa9", "\xe2\x80\x8d", "\xf0\x9f\x92\xbbX"};
        CHECK(xuiDocumentCreate(NULL, &document[variant]) == XUI_OK &&
            xuiDocumentBeginTransaction(document[variant], NULL, &transaction) == XUI_OK);
        paragraph = add(transaction, 1, XUI_DOC_PARAGRAPH, NULL);
        for (part = 0; part < (variant ? 3 : 1); part++) {
            node.iSize = sizeof(node); node.iKind = XUI_DOC_TEXT;
            node.sText = variant ? split[part] : "\xf0\x9f\x91\xa9\xe2\x80\x8d\xf0\x9f\x92\xbbX";
            node.iTextBytes = strlen(node.sText);
            node.tAttributes.iTextColor = part ? XUI_COLOR_RGBA(180, 20, 40, 255) : 0;
            CHECK(xuiDocumentTxnInsertNode(transaction, paragraph, XUI_DOCUMENT_APPEND,
                &node, &nodes[variant][part]) == XUI_OK);
        }
        CHECK(xuiDocumentTxnCommit(transaction, NULL) == XUI_OK);
        xuiDocumentTxnRelease(transaction);
        CHECK(xuiDocumentAcquireSnapshot(document[variant], &snapshot) == XUI_OK &&
            xuiDocumentRendererCreate(context, NULL, &renderer[variant]) == XUI_OK &&
            xuiDocumentRendererSetSnapshot(renderer[variant], snapshot, NULL) == XUI_OK);
        xuiDocumentSnapshotRelease(snapshot);
    }
    for (pass = 0; pass < 3; pass++) {
        xui_doc_rect_t end[2], next[2];
        for (variant = 0; variant < 2; variant++) {
            xui_doc_position_t at = position(document[variant], nodes[variant][variant ? 2 : 0], variant ? 4 : 11);
            CHECK(xuiDocumentRendererLayout(renderer[variant], pass == 1 ? 1 : 300, 0, 100) == XUI_OK &&
                xuiDocumentRendererGetCaretRect(renderer[variant], &at, &end[variant]) == XUI_OK);
            at.iOffset++;
            CHECK(xuiDocumentRendererGetCaretRect(renderer[variant], &at, &next[variant]) == XUI_OK);
        }
        CHECK(fabs(end[0].x - expected) < .01 && fabs(end[0].x - end[1].x) < .01 &&
            fabs(end[0].y - end[1].y) < .01 && fabs(next[0].x - next[1].x) < .01 &&
            fabs(next[0].y - next[1].y) < .01);
    }
    for (variant = 0; variant < 2; variant++) {
        xuiDocumentRendererRelease(renderer[variant]); xuiDocumentRelease(document[variant]);
    }
    xuiDestroy(context); proxy.fontDestroy(&proxy, font);
    if (previous) CHECK(xgeEmojiPackSetDefault(previous) == XGE_OK);
    else xgeEmojiPackClearDefault();
    xgeEmojiPackFree(previous); xgeEmojiPackFree(pack);
    puts("Native XGE emoji pack: one merged ZWJ cluster retains cross-node geometry and reflow");
}
#endif
static void block_style_layout(xui_context context)
{
    xui_document d; xui_document_transaction t; xui_document_snapshot s;
    xui_document_change_set change;
    xui_document_renderer r; xui_doc_renderer_desc_t desc = {0};
    xui_doc_block_style_t style = {0}; xui_doc_range_t range, after;
    xui_doc_rect_t before, changed, caret_before, caret_changed;
    uint64_t paragraph, text; int exact;
    CHECK(xuiDocumentCreate(NULL, &d) == XUI_OK && xuiDocumentBeginTransaction(d, NULL, &t) == XUI_OK);
    paragraph = add(t, 1, XUI_DOC_PARAGRAPH, NULL);
    text = add(t, paragraph, XUI_DOC_TEXT, "short");
    CHECK(xuiDocumentTxnCommit(t, NULL) == XUI_OK); xuiDocumentTxnRelease(t);
    desc.iSize = sizeof(desc); desc.fParagraphGap = 18;
    CHECK(xuiDocumentRendererCreate(context, &desc, &r) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(d, &s) == XUI_OK && xuiDocumentRendererSetSnapshot(r, s, NULL) == XUI_OK);
    xuiDocumentSnapshotRelease(s);
    CHECK(xuiDocumentRendererLayout(r, 400, 0, 1000) == XUI_OK);
    CHECK(xuiDocumentRendererGetSize(r, &before, &exact) == XUI_OK && exact);
    range.tAnchor = range.tCaret = position(d, text, 0);
    CHECK(xuiDocumentRendererGetCaretRect(r, &range.tCaret, &caret_before) == XUI_OK);
    style.iSize = sizeof(style); style.iAlignment = 1; style.bSpacingExplicit = 1;
    CHECK(xuiDocumentBeginTransaction(d, NULL, &t) == XUI_OK);
    CHECK(xuiDocumentTxnSetBlockStyleRange(t, &range,
        XUI_DOC_BLOCK_STYLE_ALIGNMENT | XUI_DOC_BLOCK_STYLE_SPACING, &style, &after) == XUI_OK);
    CHECK(xuiDocumentTxnCommit(t, &change) == XUI_OK); xuiDocumentTxnRelease(t);
    CHECK(xuiDocumentAcquireSnapshot(d, &s) == XUI_OK && xuiDocumentRendererSetSnapshot(r, s, change) == XUI_OK);
    xuiDocumentSnapshotRelease(s); xuiDocumentChangeSetRelease(change);
    CHECK(xuiDocumentRendererLayout(r, 400, 0, 1000) == XUI_OK);
    CHECK(xuiDocumentRendererGetSize(r, &changed, &exact) == XUI_OK && exact);
    range.tCaret.iRevision = xuiDocumentGetRevision(d);
    CHECK(xuiDocumentRendererGetCaretRect(r, &range.tCaret, &caret_changed) == XUI_OK);
    CHECK(fabs((before.height - changed.height) - 18) < .01 && caret_changed.x > caret_before.x + 10);
    xuiDocumentRendererRelease(r); xuiDocumentRelease(d);
    puts("Block style layout: explicit zero spacing removes default gap; center alignment moves caret passed");
}
static void extended_inline_layout(xui_context context, xui_surface target, xui_test_proxy_state_t* proxy)
{
    const char* source = "normal ==mark== ~sub~ ^sup^\n";
    const uint32_t highlight = XUI_COLOR_RGBA(123, 231, 45, 255);
    xui_doc_desc_t md = {0}; xui_doc_renderer_desc_t desc = {0};
    xui_document d; xui_document_snapshot s; xui_document_renderer r;
    xui_doc_range_t range; xui_doc_rect_t normal, sub, sup;
    xui_draw_context draw; uint64_t count;
    md.iSize = sizeof(md); md.iProfile = XUI_DOCUMENT_MARKDOWN;
    CHECK(xuiDocumentCreate(&md, &d) == XUI_OK && xuiDocumentLoadMarkdown(d, source, strlen(source)) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(d, &s) == XUI_OK);
    desc.iSize = sizeof(desc); desc.iHighlightColor = highlight;
    CHECK(xuiDocumentRendererCreate(context, &desc, &r) == XUI_OK &&
        xuiDocumentRendererSetSnapshot(r, s, NULL) == XUI_OK);
    CHECK(xuiDocumentRendererLayout(r, 500, 0, 1000) == XUI_OK);
    CHECK(xuiDocumentSnapshotFind(s, XUI_DOC_SEMANTIC, "normal", 6, NULL, &range, 1, &count) == XUI_OK && count == 1);
    CHECK(xuiDocumentRendererGetCaretRect(r, &range.tAnchor, &normal) == XUI_OK);
    CHECK(xuiDocumentSnapshotFind(s, XUI_DOC_SEMANTIC, "sub", 3, NULL, &range, 1, &count) == XUI_OK && count == 1);
    CHECK(xuiDocumentRendererGetCaretRect(r, &range.tAnchor, &sub) == XUI_OK);
    CHECK(xuiDocumentSnapshotFind(s, XUI_DOC_SEMANTIC, "sup", 3, NULL, &range, 1, &count) == XUI_OK && count == 1);
    CHECK(xuiDocumentRendererGetCaretRect(r, &range.tAnchor, &sup) == XUI_OK);
    CHECK(sub.y > normal.y && sup.y < normal.y);
    CHECK(proxy->tProxy.drawBegin(&proxy->tProxy, &draw, target) == XUI_OK);
    CHECK(xuiDocumentRendererDraw(r, draw, 0, 0, (xui_rect_t){0, 0, 500, 200}, NULL, 0) == XUI_OK);
    CHECK(proxy->tProxy.drawEnd(&proxy->tProxy, draw) == XUI_OK &&
        xuiTestSurfaceGetRectFillColorCount(target, highlight) > 0);
    xuiDocumentRendererRelease(r); xuiDocumentSnapshotRelease(s); xuiDocumentRelease(d);
    puts("Extended inline rendering: raised/lowered script carets and themed highlight background passed");
}
static void admonition_title_layout(xui_context context, xui_surface target,
    xui_test_proxy_state_t* proxy)
{
    const char* source[] = {"> Alert\n", "> [!NOTE]\n> Alert\n"};
    double plain_height = 0, plain_caret_y = 0;
    int plain_draws = 0, mode;
    for (mode = 0; mode < 2; mode++) {
        xui_doc_desc_t md = {0}; xui_doc_renderer_desc_t desc = {0};
        xui_document document; xui_document_snapshot snapshot;
        xui_document_renderer renderer; xui_doc_range_t range;
        xui_doc_rect_t size, caret; xui_doc_position_t hit;
        xui_draw_context draw; uint64_t count;
        int exact;
        md.iSize = sizeof(md); md.iProfile = XUI_DOCUMENT_MARKDOWN;
        CHECK(xuiDocumentCreate(&md, &document) == XUI_OK &&
            xuiDocumentLoadMarkdown(document, source[mode], strlen(source[mode])) == XUI_OK);
        CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
        desc.iSize = sizeof(desc);
        desc.iQuoteBorderColor = XUI_COLOR_RGBA(30, 80, 170, 255);
        CHECK(xuiDocumentRendererCreate(context, &desc, &renderer) == XUI_OK &&
            xuiDocumentRendererSetSnapshot(renderer, snapshot, NULL) == XUI_OK);
        CHECK(xuiDocumentRendererLayout(renderer, 300, 0, 300) == XUI_OK &&
            xuiDocumentRendererGetSize(renderer, &size, &exact) == XUI_OK && exact);
        CHECK(xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC, "Alert", 5,
            NULL, &range, 1, &count) == XUI_OK && count == 1 &&
            xuiDocumentRendererGetCaretRect(renderer, &range.tAnchor, &caret) == XUI_OK);
        xuiTestSurfaceReset(target);
        CHECK(proxy->tProxy.drawBegin(&proxy->tProxy, &draw, target) == XUI_OK);
        CHECK(xuiDocumentRendererDraw(renderer, draw, 0, 0,
            (xui_rect_t){0, 0, 300, 300}, NULL, 0) == XUI_OK);
        CHECK(proxy->tProxy.drawEnd(&proxy->tProxy, draw) == XUI_OK);
        if (!mode) {
            plain_height = size.height; plain_caret_y = caret.y;
            plain_draws = xuiTestSurfaceGetTextDrawCount(target);
        } else {
            xui_doc_range_t whole = {0}; uint64_t rectangles = 0;
            CHECK(size.height > plain_height + 10 && caret.y > plain_caret_y + 10 &&
                xuiTestSurfaceGetTextDrawCount(target) == plain_draws + 1);
            CHECK(xuiDocumentRendererHitTest(renderer, 25, 5, &hit) == XUI_OK &&
                hit.iKind == XUI_DOC_POSITION_GAP && hit.iOffset == 0);
            whole.tAnchor = position(document, 1, 0);
            whole.tAnchor.iKind = XUI_DOC_POSITION_GAP;
            whole.tCaret = whole.tAnchor; whole.tCaret.iOffset = 1;
            CHECK(xuiDocumentRendererGetRangeRects(renderer, &whole,
                NULL, 0, &rectangles) == XUI_OK && rectangles > 0);
            CHECK(xuiDocumentRendererSetMode(renderer, XUI_DOC_SOURCE_TEXT) == XUI_OK &&
                xuiDocumentRendererLayout(renderer, 300, 0, 300) == XUI_OK &&
                xuiDocumentRendererHitTest(renderer, 25, 5, &hit) == XUI_OK &&
                hit.iKind == XUI_DOC_POSITION_SOURCE);
            CHECK(xuiDocumentRendererSetMode(renderer, XUI_DOC_VISUAL) == XUI_OK &&
                xuiDocumentRendererLayout(renderer, 300, 0, 300) == XUI_OK &&
                xuiDocumentRendererHitTest(renderer, 25, 5, &hit) == XUI_OK &&
                hit.iKind == XUI_DOC_POSITION_GAP);
        }
        xuiDocumentRendererRelease(renderer);
        xuiDocumentSnapshotRelease(snapshot);
        xuiDocumentRelease(document);
    }
    {
        xui_document document; xui_document_transaction transaction;
        xui_document_snapshot snapshot; xui_document_renderer renderer;
        xui_doc_node_desc_t node = {0}; xui_doc_renderer_desc_t desc = {0};
        xui_doc_position_t at, hit; xui_doc_rect_t caret, size;
        xui_draw_context draw; uint64_t quote; int exact;
        CHECK(xuiDocumentCreate(NULL, &document) == XUI_OK &&
            xuiDocumentBeginTransaction(document, NULL, &transaction) == XUI_OK);
        node.iSize = sizeof(node); node.iKind = XUI_DOC_QUOTE; node.sInfo = "warning";
        CHECK(xuiDocumentTxnInsertNode(transaction, 1, XUI_DOCUMENT_APPEND,
            &node, &quote) == XUI_OK &&
            xuiDocumentTxnCommit(transaction, NULL) == XUI_OK);
        xuiDocumentTxnRelease(transaction);
        CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
        desc.iSize = sizeof(desc);
        CHECK(xuiDocumentRendererCreate(context, &desc, &renderer) == XUI_OK &&
            xuiDocumentRendererSetSnapshot(renderer, snapshot, NULL) == XUI_OK &&
            xuiDocumentRendererLayout(renderer, 300, 0, 300) == XUI_OK &&
            xuiDocumentRendererGetSize(renderer, &size, &exact) == XUI_OK && exact &&
            size.height > 20);
        at = position(document, quote, 0); at.iKind = XUI_DOC_POSITION_GAP;
        CHECK(xuiDocumentRendererGetCaretRect(renderer, &at, &caret) == XUI_OK &&
            xuiDocumentRendererHitTest(renderer, 25, 5, &hit) == XUI_OK &&
            hit.iNodeId == quote && hit.iKind == XUI_DOC_POSITION_GAP &&
            hit.iOffset == 0);
        xuiTestSurfaceReset(target);
        CHECK(proxy->tProxy.drawBegin(&proxy->tProxy, &draw, target) == XUI_OK &&
            xuiDocumentRendererDraw(renderer, draw, 0, 0,
                (xui_rect_t){0, 0, 300, 300}, NULL, 0) == XUI_OK &&
            proxy->tProxy.drawEnd(&proxy->tProxy, draw) == XUI_OK &&
            xuiTestSurfaceGetTextDrawCount(target) == 1);
        xuiDocumentRendererRelease(renderer);
        xuiDocumentSnapshotRelease(snapshot);
        xuiDocumentRelease(document);
    }
    puts("Admonition title: dedicated visual row, quote hit, body geometry and empty quote caret passed");
}
static void link_inline_render(xui_context context, xui_surface target, xui_test_proxy_state_t* proxy)
{
    const uint32_t link_color = XUI_COLOR_RGBA(89, 34, 201, 255);
    xui_doc_desc_t md = {0}; xui_doc_renderer_desc_t desc = {0};
    xui_document d; xui_document_snapshot s; xui_document_renderer r;
    xui_draw_context draw;
    md.iSize = sizeof(md); md.iProfile = XUI_DOCUMENT_MARKDOWN;
    CHECK(xuiDocumentCreate(&md, &d) == XUI_OK &&
        xuiDocumentLoadMarkdown(d, "[link](/target)\n", 16) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(d, &s) == XUI_OK);
    desc.iSize = sizeof(desc); desc.iLinkColor = link_color;
    CHECK(xuiDocumentRendererCreate(context, &desc, &r) == XUI_OK &&
        xuiDocumentRendererSetSnapshot(r, s, NULL) == XUI_OK);
    CHECK(xuiDocumentRendererLayout(r, 300, 0, 300) == XUI_OK);
    CHECK(proxy->tProxy.drawBegin(&proxy->tProxy, &draw, target) == XUI_OK);
    CHECK(xuiDocumentRendererDraw(r, draw, 0, 0, (xui_rect_t){0, 0, 300, 100}, NULL, 0) == XUI_OK);
    CHECK(proxy->tProxy.drawEnd(&proxy->tProxy, draw) == XUI_OK &&
        xuiTestSurfaceGetLastTextColor(target) == link_color);
    xuiDocumentRendererRelease(r); xuiDocumentSnapshotRelease(s); xuiDocumentRelease(d);
    puts("Link rendering: configurable visual text color passed");
}
static void image_surface_destroy(xui_context context, void* handle, void* user)
{
    xui_test_proxy_state_t* proxy = user; (void)context;
    proxy->tProxy.surfaceDestroy(&proxy->tProxy, (xui_surface)handle);
}
static void image_resource_set(xui_context context, xui_test_proxy_state_t* proxy,
    int width, int height, xui_resource* resource)
{
    xui_resource_desc_t desc = {0}; xui_surface surface;
    CHECK(xuiTestSurfaceCreate(proxy, &surface, width, height, 0) == XUI_OK);
    desc.iSize = sizeof(desc); desc.sName = "doc.image";
    desc.iKind = XUI_RESOURCE_SURFACE; desc.pHandle = surface;
    desc.pUser = proxy; desc.onDestroy = image_surface_destroy;
    CHECK(xuiResourceSet(context, resource, &desc) == XUI_OK);
}
static void image_resource_load(xui_context context, xui_test_proxy_state_t* proxy)
{
    const unsigned char encoded[] = {1, 2};
    xui_doc_image_load_limits_t limits = {0}; xui_resource resource = NULL;
    xui_surface old; uint64_t generation; int loads, destroyed;
    limits.iSize = sizeof(limits); limits.iMaxEncodedBytes = 2; limits.iMaxPixels = 64;
    loads = xuiTestProxyGetSurfaceLoadCount(proxy);
    CHECK(xuiDocumentImageResourceLoadMemory(context, "doc.loaded", encoded, 1, &limits, &resource) == XUI_OK);
    CHECK(resource && xuiResourceFind(context, "doc.loaded") == resource &&
        xuiTestProxyGetSurfaceLoadCount(proxy) == loads + 1);
    old = (xui_surface)xuiResourceGetHandle(resource);
    generation = xuiResourceGetRegistryGeneration(context);
    destroyed = xuiTestProxyGetSurfaceDestroyCount(proxy);
    limits.iMaxEncodedBytes = 1; resource = (xui_resource)1;
    CHECK(xuiDocumentImageResourceLoadMemory(context, "doc.loaded", encoded, 2, &limits, &resource) == XUI_DOC_ERROR_LIMIT && !resource);
    CHECK(xuiResourceGetRegistryGeneration(context) == generation &&
        xuiResourceGetHandle(xuiResourceFind(context, "doc.loaded")) == old);
    limits.iMaxEncodedBytes = 2; limits.iMaxPixels = 63;
    CHECK(xuiDocumentImageResourceLoadMemory(context, "doc.loaded", encoded, 2, &limits, &resource) == XUI_DOC_ERROR_LIMIT && !resource);
    CHECK(xuiTestProxyGetSurfaceDestroyCount(proxy) == destroyed + 1 &&
        xuiResourceGetRegistryGeneration(context) == generation &&
        xuiResourceGetHandle(xuiResourceFind(context, "doc.loaded")) == old);
    limits.iMaxPixels = 64;
    CHECK(xuiDocumentImageResourceLoadFile(context, "doc.loaded", "missing-doc-image.png", &limits, &resource) == XUI_DOC_ERROR_IO && !resource);
    CHECK(xuiResourceGetHandle(xuiResourceFind(context, "doc.loaded")) == old);
    limits.iMaxEncodedBytes = 1;
    CHECK(xuiDocumentImageResourceLoadFile(context, "doc.loaded", "res/msgbox_info.png", &limits, &resource) == XUI_DOC_ERROR_LIMIT && !resource);
    CHECK(xuiResourceGetHandle(xuiResourceFind(context, "doc.loaded")) == old);
    limits.iMaxEncodedBytes = 0;
    CHECK(xuiDocumentImageResourceLoadFile(context, "doc.loaded", "res/msgbox_info.png", &limits, &resource) == XUI_OK && resource);
    CHECK(xuiResourceGetRegistryGeneration(context) > generation &&
        xuiTestProxyGetSurfaceDestroyCount(proxy) == destroyed + 2 &&
        xuiResourceGetHandle(resource) != old);
    CHECK(xuiResourceRemove(resource) == XUI_OK &&
        xuiTestProxyGetSurfaceDestroyCount(proxy) == destroyed + 3);
    puts("Document image resource loader: explicit file/memory input, limits, atomic replacement and surface ownership passed");
}
static int image_resource_await(xui_doc_image_request request, xui_resource* resource)
{
    unsigned i; int result;
    for (i = 0; i < 5000; i++) {
        result = xuiDocumentImageResourcePoll(request, resource);
        if (result != XUI_DOC_ERROR_BUSY) return result;
#ifdef _WIN32
        Sleep(1);
#endif
    }
    CHECK(0 && "image worker did not finish");
    return XUI_DOC_ERROR_BUSY;
}
static void image_resource_async(xui_context context)
{
    xui_doc_image_request request = NULL, older = NULL;
    xui_doc_image_load_limits_t limits = {0}; xui_resource resource = NULL, replacement = NULL;
    uint64_t generation = xuiResourceGetRegistryGeneration(context);
    CHECK(xuiDocumentImageResourceLoadFileAsync(context, "doc.async", "res/msgbox_info.png", NULL, &request) == XUI_OK && request);
    CHECK(image_resource_await(request, &resource) == XUI_OK && resource &&
        xuiResourceGetRegistryGeneration(context) > generation);
    CHECK(xuiDocumentImageResourcePoll(request, &replacement) == XUI_OK && replacement == resource);
    xuiDocumentImageResourceRelease(request);
    CHECK(xuiResourceRemove(resource) == XUI_OK);

    {
        unsigned char encoded[4096]; FILE* file = fopen("res/msgbox_info.png", "rb"); size_t bytes;
        CHECK(file != NULL);
        bytes = fread(encoded, 1, sizeof(encoded), file);
        CHECK(bytes > 0 && bytes < sizeof(encoded) && fgetc(file) == EOF);
        CHECK(fclose(file) == 0);
        CHECK(xuiDocumentImageResourceLoadMemoryAsync(context, "doc.async", encoded, bytes, NULL, &request) == XUI_OK);
        memset(encoded, 0, bytes);
        CHECK(image_resource_await(request, &resource) == XUI_OK && resource);
        xuiDocumentImageResourceRelease(request);
        CHECK(xuiResourceRemove(resource) == XUI_OK);
    }

    limits.iSize = sizeof(limits); limits.iMaxEncodedBytes = 1;
    CHECK(xuiDocumentImageResourceLoadFileAsync(context, "doc.async", "res/msgbox_info.png", &limits, &request) == XUI_OK);
    CHECK(image_resource_await(request, &resource) == XUI_DOC_ERROR_LIMIT && !resource);
    xuiDocumentImageResourceRelease(request);
    limits.iMaxEncodedBytes = 0; limits.iMaxPixels = 1;
    CHECK(xuiDocumentImageResourceLoadFileAsync(context, "doc.async", "res/msgbox_info.png", &limits, &request) == XUI_OK);
    CHECK(image_resource_await(request, &resource) == XUI_DOC_ERROR_LIMIT && !resource);
    xuiDocumentImageResourceRelease(request);
    { const unsigned char invalid[] = {1, 2, 3}; int width = -1, height = -1;
      CHECK(xgeImageInfoMemory(invalid, sizeof(invalid), &width, &height) == XGE_ERROR_RESOURCE_FAILED && !width && !height);
      CHECK(xuiDocumentImageResourceLoadMemoryAsync(context, "doc.async", invalid, sizeof(invalid), NULL, &request) == XUI_OK); }
    CHECK(image_resource_await(request, &resource) == XUI_DOC_ERROR_FORMAT && !resource);
    xuiDocumentImageResourceRelease(request);
    CHECK(xuiDocumentImageResourceLoadFileAsync(context, "doc.async", "res/msgbox_info.png", NULL, &request) == XUI_OK);
    xuiDocumentImageResourceRelease(request);
    CHECK(!xuiResourceFind(context, "doc.async"));

    CHECK(xuiDocumentImageResourceLoadFileAsync(context, "doc.async", "res/msgbox_info.png", NULL, &request) == XUI_OK);
    CHECK(xuiDocumentImageResourceCancel(request) == XUI_OK);
    CHECK(image_resource_await(request, &resource) == XUI_DOC_ERROR_CANCELLED && !resource);
    xuiDocumentImageResourceRelease(request);

    CHECK(xuiDocumentImageResourceLoadFileAsync(context, "doc.async", "res/msgbox_info.png", NULL, &request) == XUI_OK);
    { const unsigned char encoded = 1;
      CHECK(xuiDocumentImageResourceLoadMemory(context, "doc.async", &encoded, 1, NULL, &replacement) == XUI_OK); }
    CHECK(image_resource_await(request, &resource) == XUI_DOC_ERROR_STALE && !resource &&
        xuiResourceFind(context, "doc.async") == replacement);
    xuiDocumentImageResourceRelease(request);
    CHECK(xuiResourceRemove(replacement) == XUI_OK);

    CHECK(xuiDocumentImageResourceLoadFileAsync(context, "doc.async", "res/msgbox_info.png", NULL, &older) == XUI_OK);
    CHECK(xuiDocumentImageResourceLoadFileAsync(context, "doc.async", "res/msgbox_info.png", NULL, &request) == XUI_OK);
    CHECK(image_resource_await(older, &resource) == XUI_DOC_ERROR_CANCELLED && !resource);
    CHECK(image_resource_await(request, &resource) == XUI_OK && resource);
    xuiDocumentImageResourceRelease(older); xuiDocumentImageResourceRelease(request);
    CHECK(xuiResourceRemove(resource) == XUI_OK);
    {
        xui_doc_image_request queued[5] = {0}; xui_doc_image_async_stats_t stats = {0};
        char name[32]; unsigned i;
        for (i = 0; i < 5; i++) {
            snprintf(name, sizeof(name), "doc.queue.%u", i);
            CHECK(xuiDocumentImageResourceLoadFileAsync(context, name, "res/msgbox_info.png", NULL, &queued[i]) == XUI_OK);
        }
        stats.iSize = sizeof(stats);
        CHECK(xuiDocumentImageResourceGetAsyncStats(context, &stats) == XUI_OK &&
            stats.iRunning == 4 && stats.iQueued == 1 && stats.iOutstanding == 5);
        CHECK(image_resource_await(queued[4], &resource) == XUI_OK && resource);
        for (i = 0; i < 5; i++) {
            CHECK(image_resource_await(queued[i], &resource) == XUI_OK && resource);
            CHECK(xuiResourceRemove(resource) == XUI_OK);
            xuiDocumentImageResourceRelease(queued[i]);
        }
        stats.iSize = sizeof(stats);
        CHECK(xuiDocumentImageResourceGetAsyncStats(context, &stats) == XUI_OK &&
            !stats.iRunning && !stats.iQueued && !stats.iOutstanding && !stats.iCopiedBytes);
    }
    {
        xui_doc_image_request queued[64] = {0}; xui_doc_image_async_stats_t stats = {0};
        char name[32]; unsigned i;
        for (i = 0; i < 64; i++) {
            snprintf(name, sizeof(name), "doc.cap.%u", i);
            CHECK(xuiDocumentImageResourceLoadFileAsync(context, name, "missing-doc-image.png", NULL, &queued[i]) == XUI_OK);
        }
        stats.iSize = sizeof(stats);
        CHECK(xuiDocumentImageResourceGetAsyncStats(context, &stats) == XUI_OK &&
            stats.iRunning == 4 && stats.iQueued == 60 && stats.iOutstanding == 64);
        CHECK(xuiDocumentImageResourceLoadFileAsync(context, "doc.cap.extra", "missing-doc-image.png", NULL, &request) == XUI_DOC_ERROR_LIMIT && !request);
        for (i = 64; i > 0; i--) xuiDocumentImageResourceRelease(queued[i - 1]);
    }
    {
        xui_doc_image_request versions[80] = {0}; xui_doc_image_async_stats_t stats = {0}; unsigned i;
        for (i = 0; i < 80; i++)
            CHECK(xuiDocumentImageResourceLoadFileAsync(context, "doc.churn", "res/msgbox_info.png", NULL, &versions[i]) == XUI_OK);
        stats.iSize = sizeof(stats);
        CHECK(xuiDocumentImageResourceGetAsyncStats(context, &stats) == XUI_OK &&
            stats.iRunning == 4 && stats.iQueued == 1 && stats.iOutstanding == 5);
        CHECK(image_resource_await(versions[79], &resource) == XUI_OK && resource);
        CHECK(xuiDocumentImageResourcePoll(versions[4], NULL) == XUI_DOC_ERROR_CANCELLED);
        for (i = 0; i < 80; i++) xuiDocumentImageResourceRelease(versions[i]);
        CHECK(xuiResourceRemove(resource) == XUI_OK);
    }
    {
        const size_t bytes = 16u * 1024u * 1024u;
        unsigned char* encoded = (unsigned char*)calloc(1, bytes);
        xui_doc_image_request requests[4] = {0}; xui_doc_image_async_stats_t stats = {0};
        char name[32]; unsigned i;
        CHECK(encoded != NULL);
        for (i = 0; i < 4; i++) {
            snprintf(name, sizeof(name), "doc.bytes.%u", i);
            CHECK(xuiDocumentImageResourceLoadMemoryAsync(context, name, encoded, bytes, NULL, &requests[i]) == XUI_OK);
        }
        stats.iSize = sizeof(stats);
        CHECK(xuiDocumentImageResourceGetAsyncStats(context, &stats) == XUI_OK &&
            stats.iRunning == 4 && stats.iCopiedBytes == 64u * 1024u * 1024u);
        CHECK(xuiDocumentImageResourceLoadMemoryAsync(context, "doc.bytes.extra", encoded, bytes, NULL, &request) == XUI_DOC_ERROR_LIMIT && !request);
        for (i = 4; i > 0; i--) xuiDocumentImageResourceRelease(requests[i - 1]);
        free(encoded);
    }
    puts("Document async images: file/memory decode, bounded queue, owner publication, limits, cancel, stale result and supersession passed");
}
static void image_resource_render(xui_context context, xui_surface target, xui_test_proxy_state_t* proxy)
{
    unsigned profile;
    for (profile = 0; profile < 2; profile++) {
        xui_doc_desc_t doc_desc = {0}; xui_document document; xui_document_transaction txn;
        xui_document_snapshot snapshot; xui_document_renderer renderer; xui_resource resource, dependent;
        xui_doc_rect_t bounds, view_bounds; xui_rect_t src, dst; xui_draw_context draw;
        xui_doc_view_desc_t view_desc = {0}; xui_widget view;
        xui_doc_node_desc_t image = {0}; xui_resource_desc_t dependent_desc = {0};
        uint64_t para, image_id, generation, revision; int exact, destroyed;
        doc_desc.iSize = sizeof(doc_desc); doc_desc.iProfile = profile ? XUI_DOCUMENT_MARKDOWN : XUI_DOCUMENT_RICH;
        CHECK(xuiDocumentCreate(&doc_desc, &document) == XUI_OK);
        if (profile) CHECK(xuiDocumentLoadMarkdown(document, "![alt](doc.image)\n", 18) == XUI_OK);
        else {
            CHECK(xuiDocumentBeginTransaction(document, NULL, &txn) == XUI_OK);
            para = add(txn, 1, XUI_DOC_PARAGRAPH, NULL);
            image.iSize = sizeof(image); image.iKind = XUI_DOC_IMAGE;
            image.sText = "alt"; image.iTextBytes = 3; image.sResource = "doc.image";
            CHECK(xuiDocumentTxnInsertNode(txn, para, XUI_DOCUMENT_APPEND, &image, &image_id) == XUI_OK);
            CHECK(xuiDocumentTxnCommit(txn, NULL) == XUI_OK); xuiDocumentTxnRelease(txn);
        }
        CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
        CHECK(xuiDocumentRendererCreate(context, NULL, &renderer) == XUI_OK);
        CHECK(xuiDocumentRendererSetSnapshot(renderer, snapshot, NULL) == XUI_OK);
        xuiDocumentSnapshotRelease(snapshot);
        CHECK(xuiDocumentRendererLayout(renderer, 150, 0, 400) == XUI_OK);
        CHECK(xuiDocumentRendererGetSize(renderer, &bounds, &exact) == XUI_OK && exact && bounds.height < 60);
        generation = xuiResourceGetRegistryGeneration(context); revision = xuiDocumentGetRevision(document);
        image_resource_set(context, proxy, 240, 120, &resource);
        CHECK(xuiResourceGetRegistryGeneration(context) > generation);
        dependent_desc.iSize = sizeof(dependent_desc); dependent_desc.sName = "doc.image.dependent";
        dependent_desc.iKind = XUI_RESOURCE_SURFACE;
        CHECK(xuiResourceSet(context, &dependent, &dependent_desc) == XUI_OK);
        CHECK(xuiResourceAddDependency(dependent, resource) == XUI_OK &&
            xuiResourceGetDependencyCount(dependent) == 1);
        generation = xuiResourceGetRegistryGeneration(context);
        CHECK(xuiDocumentRendererLayout(renderer, 150, 0, 400) == XUI_OK);
        CHECK(xuiDocumentRendererGetSize(renderer, &bounds, &exact) == XUI_OK && exact &&
            bounds.height > 75 && bounds.height < 95);
        view_desc.iSize = sizeof(view_desc); view_desc.pDocument = document; view_desc.bAutoHeight = 1;
        CHECK(xuiDocumentViewCreate(context, &view_desc, &view) == XUI_OK);
        CHECK(xuiSetRootWidget(context, view) == XUI_OK);
        CHECK(xuiWidgetSetRect(view, (xui_rect_t){0, 0, 150, 300}) == XUI_OK);
        CHECK(xuiDocumentViewGetContentSize(view, &view_bounds, &exact) == XUI_OK && exact &&
            view_bounds.height > 75 && view_bounds.height < 95);
        CHECK(xuiInputViewport(context, 150, 300) == XUI_OK && xuiUpdate(context, .016f) == XUI_OK);
        xuiWidgetClearDirty(view, XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
        xuiTestSurfaceReset(target);
        CHECK(proxy->tProxy.drawBegin(&proxy->tProxy, &draw, target) == XUI_OK);
        CHECK(xuiDocumentRendererDraw(renderer, draw, 0, 0, (xui_rect_t){0, 0, 150, 300}, NULL, 0) == XUI_OK);
        CHECK(proxy->tProxy.drawEnd(&proxy->tProxy, draw) == XUI_OK);
        src = xuiTestSurfaceGetLastSrc(target); dst = xuiTestSurfaceGetLastDst(target);
        CHECK(src.fW == 240 && src.fH == 120 && fabsf(dst.fW - 150.0f) < .01f && fabsf(dst.fH - 75.0f) < .01f);
        destroyed = xuiTestProxyGetSurfaceDestroyCount(proxy);
        image_resource_set(context, proxy, 80, 200, &resource);
        CHECK(xuiTestProxyGetSurfaceDestroyCount(proxy) == destroyed + 1 &&
            xuiResourceGetRegistryGeneration(context) > generation &&
            xuiResourceGetDependencyCount(dependent) == 0);
        generation = xuiResourceGetRegistryGeneration(context);
        CHECK(xuiUpdate(context, .016f) == XUI_OK &&
            (xuiWidgetGetDirtyFlags(view) & (XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER)));
        CHECK(xuiDocumentRendererLayout(renderer, 150, 0, 400) == XUI_OK);
        CHECK(xuiDocumentRendererGetSize(renderer, &bounds, &exact) == XUI_OK && exact &&
            bounds.height > 200 && bounds.height < 220);
        CHECK(xuiDocumentViewGetContentSize(view, &view_bounds, &exact) == XUI_OK && exact &&
            view_bounds.height > 200 && view_bounds.height < 220);
        xuiTestSurfaceReset(target);
        CHECK(proxy->tProxy.drawBegin(&proxy->tProxy, &draw, target) == XUI_OK);
        CHECK(xuiDocumentRendererDraw(renderer, draw, 0, 0, (xui_rect_t){0, 0, 150, 300}, NULL, 0) == XUI_OK);
        CHECK(proxy->tProxy.drawEnd(&proxy->tProxy, draw) == XUI_OK);
        src = xuiTestSurfaceGetLastSrc(target); dst = xuiTestSurfaceGetLastDst(target);
        CHECK(src.fW == 80 && src.fH == 200 && fabsf(dst.fW - 80.0f) < .01f && fabsf(dst.fH - 200.0f) < .01f);
        CHECK(xuiResourceTouch(resource) == XUI_OK && xuiResourceGetRegistryGeneration(context) > generation);
        generation = xuiResourceGetRegistryGeneration(context);
        CHECK(xuiDocumentRendererLayout(renderer, 150, 0, 400) == XUI_OK);
        CHECK(xuiResourceRemove(resource) == XUI_OK);
        CHECK(xuiResourceGetRegistryGeneration(context) > generation);
        CHECK(xuiDocumentRendererLayout(renderer, 150, 0, 400) == XUI_OK);
        xuiTestSurfaceReset(target);
        CHECK(proxy->tProxy.drawBegin(&proxy->tProxy, &draw, target) == XUI_OK);
        CHECK(xuiDocumentRendererDraw(renderer, draw, 0, 0, (xui_rect_t){0, 0, 150, 300}, NULL, 0) == XUI_OK);
        CHECK(proxy->tProxy.drawEnd(&proxy->tProxy, draw) == XUI_OK);
        CHECK(xuiTestSurfaceGetLastSrc(target).fW == 0 && xuiTestSurfaceGetRectFillCount(target) > 0);
        CHECK(xuiDocumentViewGetContentSize(view, &view_bounds, &exact) == XUI_OK && exact &&
            view_bounds.height < 60);
        CHECK(xuiDocumentGetRevision(document) == revision);
        CHECK(xuiResourceRemove(dependent) == XUI_OK);
        xuiWidgetDestroy(view); xuiDocumentRendererRelease(renderer); xuiDocumentRelease(document);
    }
    puts("Named image surfaces: registry auto-invalidation, rich/Markdown layout, draw, replacement, removal and fallback passed");
}
static void image_resource_scroll_anchor(xui_context context, xui_test_proxy_state_t* proxy)
{
    unsigned profile;
    for (profile = 0; profile < 2; profile++) {
        xui_doc_desc_t document_desc = {0}; xui_doc_view_desc_t view_desc = {0};
        xui_document document; xui_document_transaction txn;
        xui_doc_node_desc_t image = {0}; xui_resource resource = NULL;
        xui_widget view; xui_doc_position_t before, after;
        uint64_t paragraph, image_id, revision; double x, old_y, new_y;
        char markdown[4096]; size_t used = 0; unsigned i;
        document_desc.iSize = sizeof(document_desc);
        document_desc.iProfile = profile ? XUI_DOCUMENT_MARKDOWN : XUI_DOCUMENT_RICH;
        CHECK(xuiDocumentCreate(&document_desc, &document) == XUI_OK);
        image_resource_set(context, proxy, 50, 40, &resource);
        if (profile) {
            int written = snprintf(markdown, sizeof(markdown), "![alt](doc.image)\n\n");
            CHECK(written > 0 && (size_t)written < sizeof(markdown)); used = (size_t)written;
            for (i = 0; i < 40; i++) {
                written = snprintf(markdown + used, sizeof(markdown) - used,
                    "Paragraph %u remains visible.\n\n", i);
                CHECK(written > 0 && (size_t)written < sizeof(markdown) - used);
                used += (size_t)written;
            }
            CHECK(xuiDocumentLoadMarkdown(document, markdown, used) == XUI_OK);
        } else {
            CHECK(xuiDocumentBeginTransaction(document, NULL, &txn) == XUI_OK);
            paragraph = add(txn, 1, XUI_DOC_PARAGRAPH, NULL);
            image.iSize = sizeof(image); image.iKind = XUI_DOC_IMAGE;
            image.sText = "alt"; image.iTextBytes = 3; image.sResource = "doc.image";
            CHECK(xuiDocumentTxnInsertNode(txn, paragraph, XUI_DOCUMENT_APPEND, &image, &image_id) == XUI_OK);
            for (i = 0; i < 40; i++) {
                char text[64];
                CHECK(snprintf(text, sizeof(text), "Paragraph %u remains visible.", i) > 0);
                paragraph = add(txn, 1, XUI_DOC_PARAGRAPH, NULL);
                add(txn, paragraph, XUI_DOC_TEXT, text);
            }
            CHECK(xuiDocumentTxnCommit(txn, NULL) == XUI_OK); xuiDocumentTxnRelease(txn);
        }
        revision = xuiDocumentGetRevision(document);
        view_desc.iSize = sizeof(view_desc); view_desc.pDocument = document;
        CHECK(xuiDocumentViewCreate(context, &view_desc, &view) == XUI_OK);
        CHECK(xuiSetRootWidget(context, view) == XUI_OK);
        CHECK(xuiWidgetSetRect(view, (xui_rect_t){0, 0, 160, 100}) == XUI_OK);
        CHECK(xuiInputViewport(context, 160, 100) == XUI_OK && xuiUpdate(context, .016f) == XUI_OK);
        CHECK(xuiDocumentViewSetScroll(view, 0, 220) == XUI_OK);
        CHECK(xuiDocumentViewGetScroll(view, &x, &old_y) == XUI_OK && fabs(old_y - 220) < .01);
        CHECK(xuiDocumentViewHitTest(view, 20, 30, &before) == XUI_OK);

        image_resource_set(context, proxy, 50, 130, &resource);
        CHECK(xuiUpdate(context, .016f) == XUI_OK);
        CHECK(xuiDocumentViewGetScroll(view, &x, &new_y) == XUI_OK && fabs(new_y - old_y - 90) < .01);
        CHECK(xuiDocumentViewHitTest(view, 20, 30, &after) == XUI_OK &&
            after.iKind == before.iKind && after.iNodeId == before.iNodeId &&
            after.iOffset == before.iOffset);
        old_y = new_y;

        image_resource_set(context, proxy, 50, 20, &resource);
        CHECK(xuiUpdate(context, .016f) == XUI_OK);
        CHECK(xuiDocumentViewGetScroll(view, &x, &new_y) == XUI_OK && fabs(new_y - old_y + 110) < .01);
        CHECK(xuiDocumentViewHitTest(view, 20, 30, &after) == XUI_OK &&
            after.iKind == before.iKind && after.iNodeId == before.iNodeId &&
            after.iOffset == before.iOffset && xuiDocumentGetRevision(document) == revision);
        xuiWidgetDestroy(view); xuiDocumentRelease(document);
        CHECK(xuiResourceRemove(resource) == XUI_OK);
    }
    puts("Resource resize preserves the visual reading anchor in fixed-height rich and Markdown views");
}
static void image_resource_unmeasured_anchor(xui_context context, xui_test_proxy_state_t* proxy)
{
    xui_document document; xui_document_transaction txn; xui_widget view;
    xui_doc_view_desc_t view_desc = {0}; xui_doc_node_desc_t image = {0};
    xui_resource resource = NULL; xui_doc_position_t before, after;
    xui_doc_renderer_stats_t stats = {0};
    uint64_t paragraph, image_id; double x, old_y, new_y; unsigned i;
    CHECK(xuiDocumentCreate(NULL, &document) == XUI_OK);
    CHECK(xuiDocumentBeginTransaction(document, NULL, &txn) == XUI_OK);
    for (i = 0; i < 45; i++) {
        char text[64];
        paragraph = add(txn, 1, XUI_DOC_PARAGRAPH, NULL);
        if (i == 15) {
            image.iSize = sizeof(image); image.iKind = XUI_DOC_IMAGE;
            image.sText = "alt"; image.iTextBytes = 3; image.sResource = "doc.image";
            CHECK(xuiDocumentTxnInsertNode(txn, paragraph, XUI_DOCUMENT_APPEND, &image, &image_id) == XUI_OK);
        } else {
            CHECK(snprintf(text, sizeof(text), "Paragraph %u remains visible.", i) > 0);
            add(txn, paragraph, XUI_DOC_TEXT, text);
        }
    }
    CHECK(xuiDocumentTxnCommit(txn, NULL) == XUI_OK); xuiDocumentTxnRelease(txn);
    image_resource_set(context, proxy, 50, 40, &resource);
    view_desc.iSize = sizeof(view_desc); view_desc.pDocument = document;
    CHECK(xuiDocumentViewCreate(context, &view_desc, &view) == XUI_OK);
    CHECK(xuiSetRootWidget(context, view) == XUI_OK);
    CHECK(xuiWidgetSetRect(view, (xui_rect_t){0, 0, 160, 100}) == XUI_OK);
    CHECK(xuiInputViewport(context, 160, 100) == XUI_OK && xuiUpdate(context, .016f) == XUI_OK);
    stats.iSize = sizeof(stats);
    CHECK(xuiDocumentViewGetRenderStats(view, &stats) == XUI_OK && stats.iMeasuredBlocks < 15);
    CHECK(xuiDocumentViewSetScroll(view, 0, 800) == XUI_OK);
    CHECK(xuiDocumentViewHitTest(view, 20, 30, &before) == XUI_OK);
    CHECK(xuiDocumentViewGetScroll(view, &x, &old_y) == XUI_OK && fabs(old_y - 800) < .01);
    image_resource_set(context, proxy, 50, 130, &resource);
    CHECK(xuiUpdate(context, .016f) == XUI_OK);
    CHECK(xuiDocumentViewGetScroll(view, &x, &new_y) == XUI_OK && new_y > old_y + 50);
    CHECK(xuiDocumentViewHitTest(view, 20, 30, &after) == XUI_OK &&
        after.iKind == before.iKind && after.iNodeId == before.iNodeId &&
        after.iOffset == before.iOffset);
    xuiWidgetDestroy(view); xuiDocumentRelease(document);
    CHECK(xuiResourceRemove(resource) == XUI_OK);
    puts("Offscreen, unmeasured image resource changes preserve the visible text anchor");
}
static int mutable_object_measure(xui_document_snapshot snapshot, xui_doc_node_id node,
    float width, float zoom, xui_vec2_t* size, float* baseline, void* user)
{
    int* height = user; (void)snapshot; (void)node;
    size->fX = fminf(width, 50 * zoom);
    size->fY = *height * zoom;
    *baseline = size->fY;
    return XUI_OK;
}
static void object_provider_scroll_anchor(xui_context context)
{
    xui_document document; xui_document_transaction txn; xui_widget view;
    xui_doc_view_desc_t view_desc = {0}; xui_doc_node_desc_t object = {0};
    xui_doc_position_t before, after; xui_doc_renderer_stats_t stats_before = {0}, stats_after = {0};
    uint64_t paragraph, object_id, revision; double x, old_y, new_y; unsigned i; int height = 40;
    CHECK(xuiDocumentCreate(NULL, &document) == XUI_OK);
    CHECK(xuiDocumentBeginTransaction(document, NULL, &txn) == XUI_OK);
    paragraph = add(txn, 1, XUI_DOC_PARAGRAPH, NULL);
    object.iSize = sizeof(object); object.iKind = XUI_DOC_MATH;
    object.sText = "x^2"; object.iTextBytes = 3;
    CHECK(xuiDocumentTxnInsertNode(txn, paragraph, XUI_DOCUMENT_APPEND, &object, &object_id) == XUI_OK);
    for (i = 0; i < 40; i++) {
        char text[64];
        CHECK(snprintf(text, sizeof(text), "Paragraph %u remains visible.", i) > 0);
        paragraph = add(txn, 1, XUI_DOC_PARAGRAPH, NULL);
        add(txn, paragraph, XUI_DOC_TEXT, text);
    }
    CHECK(xuiDocumentTxnCommit(txn, NULL) == XUI_OK); xuiDocumentTxnRelease(txn);
    revision = xuiDocumentGetRevision(document);
    view_desc.iSize = sizeof(view_desc); view_desc.pDocument = document;
    view_desc.tRenderer.iSize = sizeof(view_desc.tRenderer);
    view_desc.tRenderer.onObjectMeasure = mutable_object_measure;
    view_desc.tRenderer.pUser = &height;
    CHECK(xuiDocumentViewCreate(context, &view_desc, &view) == XUI_OK);
    CHECK(xuiSetRootWidget(context, view) == XUI_OK);
    CHECK(xuiWidgetSetRect(view, (xui_rect_t){0, 0, 160, 100}) == XUI_OK);
    CHECK(xuiInputViewport(context, 160, 100) == XUI_OK && xuiUpdate(context, .016f) == XUI_OK);
    CHECK(xuiDocumentViewSetScroll(view, 0, 220) == XUI_OK);
    CHECK(xuiDocumentViewHitTest(view, 20, 30, &before) == XUI_OK);
    CHECK(xuiDocumentViewGetScroll(view, &x, &old_y) == XUI_OK && fabs(old_y - 220) < .01);
    stats_before.iSize = stats_after.iSize = sizeof(stats_before);
    CHECK(xuiDocumentViewGetRenderStats(view, &stats_before) == XUI_OK);
    height = 130;
    CHECK(xuiDocumentViewInvalidateObjects(view) == XUI_OK);
    CHECK(xuiDocumentViewGetScroll(view, &x, &new_y) == XUI_OK && fabs(new_y - old_y - 90) < .01);
    CHECK(xuiDocumentViewGetRenderStats(view, &stats_after) == XUI_OK &&
        stats_after.iShapedBytes == stats_before.iShapedBytes);
    CHECK(xuiDocumentViewHitTest(view, 20, 30, &after) == XUI_OK &&
        after.iKind == before.iKind && after.iNodeId == before.iNodeId &&
        after.iOffset == before.iOffset);
    CHECK(xuiDocumentGetRevision(document) == revision);
    xuiWidgetDestroy(view); xuiDocumentRelease(document);
    puts("Explicit object-provider invalidation preserves the reading anchor and unrelated text shaping");
}
static void object_provider_same_block_anchor(xui_context context)
{
    xui_document document; xui_document_transaction txn; xui_widget view;
    xui_doc_view_desc_t view_desc = {0}; xui_doc_node_desc_t object = {0};
    xui_doc_position_t before, after; uint64_t paragraph, object_id, text_id;
    double x, old_y, new_y; unsigned i; int height = 40;
    CHECK(xuiDocumentCreate(NULL, &document) == XUI_OK);
    CHECK(xuiDocumentBeginTransaction(document, NULL, &txn) == XUI_OK);
    paragraph = add(txn, 1, XUI_DOC_PARAGRAPH, NULL);
    object.iSize = sizeof(object); object.iKind = XUI_DOC_MATH;
    object.sText = "x^2"; object.iTextBytes = 3;
    CHECK(xuiDocumentTxnInsertNode(txn, paragraph, XUI_DOCUMENT_APPEND, &object, &object_id) == XUI_OK);
    add(txn, paragraph, XUI_DOC_HARD_BREAK, NULL);
    text_id = add(txn, paragraph, XUI_DOC_TEXT, "Text after formula");
    for (i = 0; i < 40; i++) {
        char text[64];
        CHECK(snprintf(text, sizeof(text), "Paragraph %u remains visible.", i) > 0);
        paragraph = add(txn, 1, XUI_DOC_PARAGRAPH, NULL);
        add(txn, paragraph, XUI_DOC_TEXT, text);
    }
    CHECK(xuiDocumentTxnCommit(txn, NULL) == XUI_OK); xuiDocumentTxnRelease(txn);
    view_desc.iSize = sizeof(view_desc); view_desc.pDocument = document;
    view_desc.tRenderer.iSize = sizeof(view_desc.tRenderer);
    view_desc.tRenderer.onObjectMeasure = mutable_object_measure;
    view_desc.tRenderer.pUser = &height;
    CHECK(xuiDocumentViewCreate(context, &view_desc, &view) == XUI_OK);
    CHECK(xuiSetRootWidget(context, view) == XUI_OK);
    CHECK(xuiWidgetSetRect(view, (xui_rect_t){0, 0, 160, 100}) == XUI_OK);
    CHECK(xuiInputViewport(context, 160, 100) == XUI_OK && xuiUpdate(context, .016f) == XUI_OK);
    CHECK(xuiDocumentViewSetScroll(view, 0, 50) == XUI_OK);
    CHECK(xuiDocumentViewHitTest(view, 20, 5, &before) == XUI_OK && before.iNodeId == text_id);
    CHECK(xuiDocumentViewGetScroll(view, &x, &old_y) == XUI_OK && fabs(old_y - 50) < .01);
    height = 130;
    CHECK(xuiDocumentViewInvalidateObjects(view) == XUI_OK);
    CHECK(xuiDocumentViewGetScroll(view, &x, &new_y) == XUI_OK && fabs(new_y - old_y - 90) < .01);
    CHECK(xuiDocumentViewHitTest(view, 20, 5, &after) == XUI_OK &&
        after.iNodeId == text_id && after.iOffset == before.iOffset);
    xuiWidgetDestroy(view); xuiDocumentRelease(document);
    puts("An object resizing above text in the same paragraph preserves the text-line anchor");
}
static void view_zoom_scroll_anchor(xui_context context)
{
    unsigned profile;
    for (profile = 0; profile < 2; profile++) {
        xui_doc_desc_t document_desc = {0}; xui_doc_view_desc_t view_desc = {0};
        xui_document document; xui_document_transaction txn; xui_widget view;
        xui_doc_position_t before, after; uint64_t paragraph; double x, old_y, new_y;
        char markdown[4096]; size_t used = 0; unsigned i;
        document_desc.iSize = sizeof(document_desc);
        document_desc.iProfile = profile ? XUI_DOCUMENT_MARKDOWN : XUI_DOCUMENT_RICH;
        CHECK(xuiDocumentCreate(&document_desc, &document) == XUI_OK);
        if (profile) {
            int written;
            for (i = 0; i < 40; i++) {
                written = snprintf(markdown + used, sizeof(markdown) - used,
                    "Paragraph %u remains visible during zoom.\n\n", i);
                CHECK(written > 0 && (size_t)written < sizeof(markdown) - used);
                used += (size_t)written;
            }
            CHECK(xuiDocumentLoadMarkdown(document, markdown, used) == XUI_OK);
        } else {
            CHECK(xuiDocumentBeginTransaction(document, NULL, &txn) == XUI_OK);
            for (i = 0; i < 40; i++) {
                char body[64];
                CHECK(snprintf(body, sizeof(body), "Paragraph %u remains visible during zoom.", i) > 0);
                paragraph = add(txn, 1, XUI_DOC_PARAGRAPH, NULL);
                add(txn, paragraph, XUI_DOC_TEXT, body);
            }
            CHECK(xuiDocumentTxnCommit(txn, NULL) == XUI_OK); xuiDocumentTxnRelease(txn);
        }
        view_desc.iSize = sizeof(view_desc); view_desc.pDocument = document;
        CHECK(xuiDocumentViewCreate(context, &view_desc, &view) == XUI_OK);
        CHECK(xuiSetRootWidget(context, view) == XUI_OK);
        CHECK(xuiWidgetSetRect(view, (xui_rect_t){0, 0, 180, 100}) == XUI_OK);
        CHECK(xuiInputViewport(context, 180, 100) == XUI_OK && xuiUpdate(context, .016f) == XUI_OK);
        CHECK(xuiDocumentViewSetScroll(view, 0, 220) == XUI_OK);
        CHECK(xuiDocumentViewHitTest(view, 45, 20, &before) == XUI_OK);
        CHECK(xuiDocumentViewGetScroll(view, &x, &old_y) == XUI_OK && fabs(old_y - 220) < .01);
        CHECK(xuiDocumentViewSetZoom(view, 1.5f) == XUI_OK);
        CHECK(xuiDocumentViewGetScroll(view, &x, &new_y) == XUI_OK && new_y > old_y + 40);
        CHECK(xuiDocumentViewHitTest(view, 45, 20, &after) == XUI_OK &&
            after.iKind == before.iKind && after.iNodeId == before.iNodeId);
        CHECK(xuiDocumentViewSetZoom(view, 1.0f) == XUI_OK);
        CHECK(xuiDocumentViewGetScroll(view, &x, &new_y) == XUI_OK && fabs(new_y - old_y) < 20);
        CHECK(xuiDocumentViewHitTest(view, 45, 20, &before) == XUI_OK);
        CHECK(xuiSetVirtualDpi(context, 1.5f) == XUI_OK && xuiUpdate(context, .016f) == XUI_OK);
        CHECK(xuiDocumentViewHitTest(view, 45, 20, &after) == XUI_OK &&
            after.iKind == before.iKind && after.iNodeId == before.iNodeId);
        CHECK(xuiSetVirtualDpi(context, 1.0f) == XUI_OK && xuiUpdate(context, .016f) == XUI_OK);
        CHECK(xuiDocumentViewHitTest(view, 45, 20, &after) == XUI_OK &&
            after.iKind == before.iKind && after.iNodeId == before.iNodeId);
        xuiWidgetDestroy(view); xuiDocumentRelease(document);
    }
    puts("Rich/Markdown View zoom/DPI preserve the visible paragraph and restore scroll on zoom back");
}
typedef struct style_anchor_family_probe { xui_font font; unsigned used; } style_anchor_family_probe;
static xui_font style_anchor_family_resolve(xui_context context, const char* family,
    uint32_t marks, float size, void* user)
{
    style_anchor_family_probe* probe = user;
    (void)context; (void)marks; (void)size;
    if (strcmp(family, "Anchor family")) return NULL;
    probe->used++;
    return probe->font;
}
static void view_style_scroll_anchor(xui_context context, xui_test_proxy_state_t* proxy)
{
    unsigned mode;
    for (mode = 0; mode < 4; mode++) {
        xui_document document; xui_document_transaction txn; xui_widget view;
        xui_doc_view_desc_t view_desc = {0}; xui_doc_text_style_t style = {0};
        xui_doc_range_t range; xui_doc_position_t before, after;
        style_anchor_family_probe family = {0};
        uint64_t paragraph, target_text = 0, target_bytes = 0;
        double x, old_y, new_y, styled_y, scroll = mode & 1 ? 1000 : 220;
        unsigned i, family_change = mode >= 2;
        CHECK(xuiDocumentCreate(NULL, &document) == XUI_OK &&
            xuiDocumentBeginTransaction(document, NULL, &txn) == XUI_OK);
        for (i = 0; i < 60; i++) {
            char body[64]; int written;
            written = snprintf(body, sizeof(body), "Style anchor paragraph %u stays in place.", i);
            CHECK(written > 0 && (size_t)written < sizeof(body));
            paragraph = add(txn, 1, XUI_DOC_PARAGRAPH, NULL);
            if (i == (mode & 1 ? 10u : 0u)) {
                target_text = add(txn, paragraph, XUI_DOC_TEXT, body);
                target_bytes = (uint64_t)written;
            } else add(txn, paragraph, XUI_DOC_TEXT, body);
        }
        CHECK(xuiDocumentTxnCommit(txn, NULL) == XUI_OK); xuiDocumentTxnRelease(txn);
        view_desc.iSize = sizeof(view_desc); view_desc.pDocument = document;
        if (family_change) {
            CHECK(proxy->tProxy.fontLoadFile(&proxy->tProxy, &family.font,
                "test.ttf", 48, 0) == XUI_OK);
            view_desc.tRenderer.iSize = sizeof(view_desc.tRenderer);
            view_desc.tRenderer.onFont = style_anchor_family_resolve;
            view_desc.tRenderer.pUser = &family;
        }
        CHECK(xuiDocumentViewCreate(context, &view_desc, &view) == XUI_OK &&
            xuiSetRootWidget(context, view) == XUI_OK);
        CHECK(xuiWidgetSetRect(view, (xui_rect_t){0, 0, 180, 100}) == XUI_OK);
        CHECK(xuiInputViewport(context, 180, 100) == XUI_OK &&
            xuiUpdate(context, .016f) == XUI_OK);
        CHECK(xuiDocumentViewSetScroll(view, 0, scroll) == XUI_OK);
        CHECK(xuiDocumentViewHitTest(view, 45, 20, &before) == XUI_OK);
        CHECK(xuiDocumentViewGetScroll(view, &x, &old_y) == XUI_OK && fabs(old_y - scroll) < .01);
        range.tAnchor = position(document, target_text, 0);
        range.tCaret = position(document, target_text, target_bytes);
        style.iSize = sizeof(style); style.fFontSize = 48;
        if (family_change) strcpy(style.sFontFamily, "Anchor family");
        CHECK(xuiDocumentBeginTransaction(document, NULL, &txn) == XUI_OK);
        CHECK(xuiDocumentTxnSetTextStyle(txn, &range,
            family_change ? XUI_DOC_TEXT_STYLE_FONT_FAMILY : XUI_DOC_TEXT_STYLE_FONT_SIZE,
            &style) == XUI_OK);
        CHECK(xuiDocumentTxnCommit(txn, NULL) == XUI_OK); xuiDocumentTxnRelease(txn);
        CHECK(!family_change || family.used > 0);
        CHECK(xuiDocumentViewGetScroll(view, &x, &new_y) == XUI_OK && new_y > old_y + 30);
        styled_y = new_y;
        CHECK(xuiDocumentViewHitTest(view, 45, 20, &after) == XUI_OK &&
            after.iNodeId == before.iNodeId);
        CHECK(xuiDocumentUndo(document, NULL) == XUI_OK);
        CHECK(xuiDocumentViewGetScroll(view, &x, &new_y) == XUI_OK);
        /* An unmeasured predecessor can acquire its actual normal-size height
         * during the edit, so Undo preserves the visible text instead of the
         * earlier estimate-based scroll coordinate. */
        CHECK(mode & 1 ? new_y < old_y + 100 && new_y < styled_y - 200 :
            fabs(new_y - old_y) < 20);
        CHECK(xuiDocumentViewHitTest(view, 45, 20, &after) == XUI_OK &&
            after.iNodeId == before.iNodeId);
        if (mode & 1) {
            xui_doc_renderer_stats_t stats_before = {0}, stats_after = {0};
            double color_y = new_y;
            stats_before.iSize = stats_after.iSize = sizeof(stats_before);
            CHECK(xuiDocumentViewGetRenderStats(view, &stats_before) == XUI_OK);
            range.tAnchor = position(document, target_text, 0);
            range.tCaret = position(document, target_text, target_bytes);
            style.iTextColor = XUI_COLOR_RGBA(40, 90, 170, 255);
            CHECK(xuiDocumentBeginTransaction(document, NULL, &txn) == XUI_OK);
            CHECK(xuiDocumentTxnSetTextStyle(txn, &range,
                XUI_DOC_TEXT_STYLE_COLOR, &style) == XUI_OK);
            CHECK(xuiDocumentTxnCommit(txn, NULL) == XUI_OK); xuiDocumentTxnRelease(txn);
            CHECK(xuiDocumentViewGetScroll(view, &x, &new_y) == XUI_OK &&
                fabs(new_y - color_y) < .01);
            CHECK(xuiDocumentViewGetRenderStats(view, &stats_after) == XUI_OK &&
                stats_after.iShapedBytes == stats_before.iShapedBytes);
            CHECK(xuiDocumentViewHitTest(view, 45, 20, &after) == XUI_OK &&
                after.iNodeId == before.iNodeId);
        }
        xuiWidgetDestroy(view); xuiDocumentRelease(document);
        if (family.font) proxy->tProxy.fontDestroy(&proxy->tProxy, family.font);
    }
    puts("Rich View measured/unmeasured font size and resolved font family mutations preserve the reading anchor and Undo position");
}
static void view_same_paragraph_style_anchor(xui_context context)
{
    char body[2501];
    xui_document document; xui_document_transaction txn; xui_document_change_set changes;
    xui_document_snapshot snapshot; xui_document_renderer renderer; xui_widget view;
    xui_doc_view_desc_t view_desc = {0}; xui_doc_text_style_t style = {0};
    xui_doc_range_t range; xui_doc_position_t before, mapped, after;
    xui_doc_rect_t before_rect, after_rect;
    uint64_t paragraph, text_node; double x, old_y, new_y;
    size_t i; int mapping;
    for (i = 0; i < 500; i++) memcpy(body + 5 * i, "word ", 5);
    body[2500] = 0;
    CHECK(xuiDocumentCreate(NULL, &document) == XUI_OK &&
        xuiDocumentBeginTransaction(document, NULL, &txn) == XUI_OK);
    paragraph = add(txn, 1, XUI_DOC_PARAGRAPH, NULL);
    text_node = add(txn, paragraph, XUI_DOC_TEXT, body);
    CHECK(xuiDocumentTxnCommit(txn, NULL) == XUI_OK); xuiDocumentTxnRelease(txn);
    view_desc.iSize = sizeof(view_desc); view_desc.pDocument = document;
    CHECK(xuiDocumentViewCreate(context, &view_desc, &view) == XUI_OK &&
        xuiSetRootWidget(context, view) == XUI_OK &&
        xuiWidgetSetRect(view, (xui_rect_t){0, 0, 160, 100}) == XUI_OK &&
        xuiInputViewport(context, 160, 100) == XUI_OK &&
        xuiUpdate(context, .016f) == XUI_OK &&
        xuiDocumentViewSetScroll(view, 0, 580) == XUI_OK &&
        xuiDocumentViewHitTest(view, 45, 20, &before) == XUI_OK &&
        before.iNodeId == text_node && before.iOffset > 500);
    CHECK(xuiDocumentViewGetScroll(view, &x, &old_y) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK &&
        xuiDocumentRendererCreate(context, NULL, &renderer) == XUI_OK &&
        xuiDocumentRendererSetSnapshot(renderer, snapshot, NULL) == XUI_OK &&
        xuiDocumentRendererLayout(renderer, 160, old_y, 100) == XUI_OK &&
        xuiDocumentRendererGetCaretRect(renderer, &before, &before_rect) == XUI_OK);
    xuiDocumentRendererRelease(renderer); xuiDocumentSnapshotRelease(snapshot);
    range.tAnchor = position(document, text_node, 0);
    range.tCaret = position(document, text_node, 500);
    style.iSize = sizeof(style); style.fFontSize = 42;
    CHECK(xuiDocumentBeginTransaction(document, NULL, &txn) == XUI_OK &&
        xuiDocumentTxnSetTextStyle(txn, &range, XUI_DOC_TEXT_STYLE_FONT_SIZE, &style) == XUI_OK &&
        xuiDocumentTxnCommit(txn, &changes) == XUI_OK);
    xuiDocumentTxnRelease(txn);
    CHECK(xuiDocumentMapPosition(changes, &before, &mapped, &mapping) == XUI_OK &&
        mapping != XUI_DOC_MAP_DELETED);
    xuiDocumentChangeSetRelease(changes);
    CHECK(xuiDocumentViewGetScroll(view, &x, &new_y) == XUI_OK &&
        new_y > old_y + 100 &&
        xuiDocumentViewHitTest(view, 45, 20, &after) == XUI_OK &&
        after.iKind == XUI_DOC_POSITION_TEXT);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK &&
        xuiDocumentRendererCreate(context, NULL, &renderer) == XUI_OK &&
        xuiDocumentRendererSetSnapshot(renderer, snapshot, NULL) == XUI_OK &&
        xuiDocumentRendererLayout(renderer, 160, new_y, 100) == XUI_OK &&
        xuiDocumentRendererGetCaretRect(renderer, &mapped, &after_rect) == XUI_OK &&
        fabs((after_rect.y - new_y) - (before_rect.y - old_y)) < 2);
    xuiDocumentRendererRelease(renderer); xuiDocumentSnapshotRelease(snapshot);
    xuiWidgetDestroy(view); xuiDocumentRelease(document);
    puts("Rich View same-paragraph font-size reflow preserves the mapped visible text-line anchor");
}
static void renderer_inline_style_split_cache(xui_context context)
{
    xui_document document; xui_document_transaction txn; xui_document_change_set changes;
    xui_document_snapshot snapshot; xui_document_renderer renderer;
    xui_doc_renderer_stats_t before = {0}, after = {0};
    xui_doc_text_style_t style = {0}; xui_doc_range_t range;
    xui_doc_rect_t rect; uint64_t paragraph, texts[3], new_text;
    unsigned i;
    CHECK(xuiDocumentCreate(NULL, &document) == XUI_OK &&
        xuiDocumentBeginTransaction(document, NULL, &txn) == XUI_OK);
    for (i = 0; i < 3; i++) {
        paragraph = add(txn, 1, XUI_DOC_PARAGRAPH, NULL);
        texts[i] = add(txn, paragraph, XUI_DOC_TEXT,
            i == 1 ? "middle words" : "untouched words");
    }
    CHECK(xuiDocumentTxnCommit(txn, NULL) == XUI_OK); xuiDocumentTxnRelease(txn);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK &&
        xuiDocumentRendererCreate(context, NULL, &renderer) == XUI_OK &&
        xuiDocumentRendererSetSnapshot(renderer, snapshot, NULL) == XUI_OK &&
        xuiDocumentRendererLayout(renderer, 200, 0, 1000) == XUI_OK);
    xuiDocumentSnapshotRelease(snapshot);
    before.iSize = after.iSize = sizeof(before);
    CHECK(xuiDocumentRendererGetStats(renderer, &before) == XUI_OK &&
        before.iMeasuredBlocks >= 3);
    range.tAnchor = position(document, texts[1], 2);
    range.tCaret = position(document, texts[1], 7);
    style.iSize = sizeof(style); style.fFontSize = 32;
    CHECK(xuiDocumentBeginTransaction(document, NULL, &txn) == XUI_OK &&
        xuiDocumentTxnSetTextStyle(txn, &range, XUI_DOC_TEXT_STYLE_FONT_SIZE, &style) == XUI_OK &&
        xuiDocumentTxnCommit(txn, &changes) == XUI_OK);
    xuiDocumentTxnRelease(txn);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK &&
        xuiDocumentRendererSetSnapshot(renderer, snapshot, changes) == XUI_OK);
    xuiDocumentSnapshotRelease(snapshot); xuiDocumentChangeSetRelease(changes);
    {
        xui_doc_position_t first = position(document, texts[0], 2);
        xui_doc_position_t last = position(document, texts[2], 2);
        CHECK(xuiDocumentRendererGetCaretRect(renderer, &first, &rect) == XUI_OK &&
            xuiDocumentRendererGetCaretRect(renderer, &last, &rect) == XUI_OK &&
            xuiDocumentRendererGetStats(renderer, &after) == XUI_OK &&
            after.iShapedBytes == before.iShapedBytes);
    }
    range.tAnchor = position(document, texts[2], 1);
    range.tCaret = position(document, texts[2], 4);
    CHECK(xuiDocumentBeginTransaction(document, NULL, &txn) == XUI_OK &&
        xuiDocumentTxnSetTextStyle(txn, &range, XUI_DOC_TEXT_STYLE_FONT_SIZE, &style) == XUI_OK);
    paragraph = add(txn, 1, XUI_DOC_PARAGRAPH, NULL);
    new_text = add(txn, paragraph, XUI_DOC_TEXT, "new block");
    CHECK(xuiDocumentTxnCommit(txn, &changes) == XUI_OK); xuiDocumentTxnRelease(txn);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK &&
        xuiDocumentRendererSetSnapshot(renderer, snapshot, changes) == XUI_OK &&
        xuiDocumentRendererLayout(renderer, 200, 0, 1000) == XUI_OK);
    xuiDocumentSnapshotRelease(snapshot); xuiDocumentChangeSetRelease(changes);
    {
        xui_doc_position_t added = position(document, new_text, 1);
        CHECK(xuiDocumentRendererGetCaretRect(renderer, &added, &rect) == XUI_OK);
    }
    xuiDocumentRendererRelease(renderer); xuiDocumentRelease(document);
    puts("Inline style splits retain unrelated measured blocks; mixed block insertion rebuilds the directory");
}
static uint64_t source_line_at(const char* source, uint64_t offset)
{
    uint64_t i, line = 0;
    for (i = 0; i < offset; i++) if (source[i] == '\n') line++;
    return line;
}
static void view_zoom_source_anchor(xui_context context)
{
    unsigned variant;
    for (variant = 0; variant < 2; variant++) {
        xui_doc_desc_t document_desc = {0}; xui_doc_view_desc_t view_desc = {0};
        xui_document document; xui_widget view; xui_doc_position_t before, after;
        xui_proxy_t proxy = {0}; xui_font normal = xuiGetDefaultFont(context), larger = NULL;
        char markdown[4096]; size_t used = 0; double x, old_y, new_y; unsigned i;
        document_desc.iSize = sizeof(document_desc); document_desc.iProfile = XUI_DOCUMENT_MARKDOWN;
        for (i = 0; i < 40; i++) {
            int written = snprintf(markdown + used, sizeof(markdown) - used,
                "Paragraph %u remains visible during zoom.\n\n", i);
            CHECK(written > 0 && (size_t)written < sizeof(markdown) - used);
            used += (size_t)written;
        }
        CHECK(xuiDocumentCreate(&document_desc, &document) == XUI_OK &&
            xuiDocumentLoadMarkdown(document, markdown, used) == XUI_OK);
        view_desc.iSize = sizeof(view_desc); view_desc.pDocument = document;
        CHECK(xuiDocumentViewCreate(context, &view_desc, &view) == XUI_OK);
        CHECK(xuiSetRootWidget(context, view) == XUI_OK);
        CHECK(xuiWidgetSetRect(view, (xui_rect_t){0, 0, 180, 100}) == XUI_OK);
        CHECK(xuiDocumentViewSetMode(view, variant ? XUI_DOC_LIVE_MARKDOWN : XUI_DOC_SOURCE_TEXT) == XUI_OK);
        CHECK(xuiInputViewport(context, 180, 100) == XUI_OK && xuiUpdate(context, .016f) == XUI_OK);
        CHECK(xuiDocumentViewSetScroll(view, 0, 220) == XUI_OK);
        CHECK(xuiDocumentViewHitTest(view, 45, 20, &before) == XUI_OK &&
            before.iKind == XUI_DOC_POSITION_SOURCE && before.iOffset <= used);
        CHECK(xuiDocumentViewGetScroll(view, &x, &old_y) == XUI_OK && fabs(old_y - 220) < .01);
        CHECK(xuiDocumentViewSetZoom(view, 1.5f) == XUI_OK);
        CHECK(xuiDocumentViewGetScroll(view, &x, &new_y) == XUI_OK && new_y > old_y + 40);
        CHECK(xuiDocumentViewHitTest(view, 45, 20, &after) == XUI_OK &&
            after.iKind == XUI_DOC_POSITION_SOURCE && after.iOffset <= used &&
            source_line_at(markdown, after.iOffset) == source_line_at(markdown, before.iOffset));
        CHECK(xuiDocumentViewSetZoom(view, 1.0f) == XUI_OK);
        CHECK(xuiDocumentViewHitTest(view, 45, 20, &after) == XUI_OK &&
            after.iKind == XUI_DOC_POSITION_SOURCE && after.iOffset <= used &&
            source_line_at(markdown, after.iOffset) == source_line_at(markdown, before.iOffset));
        CHECK(xuiSetVirtualDpi(context, 1.5f) == XUI_OK && xuiUpdate(context, .016f) == XUI_OK);
        CHECK(xuiDocumentViewHitTest(view, 45, 20, &after) == XUI_OK &&
            after.iKind == XUI_DOC_POSITION_SOURCE && after.iOffset <= used &&
            source_line_at(markdown, after.iOffset) == source_line_at(markdown, before.iOffset));
        CHECK(xuiSetVirtualDpi(context, 1.0f) == XUI_OK && xuiUpdate(context, .016f) == XUI_OK);
        CHECK(xuiDocumentViewHitTest(view, 45, 20, &after) == XUI_OK &&
            after.iKind == XUI_DOC_POSITION_SOURCE && after.iOffset <= used &&
            source_line_at(markdown, after.iOffset) == source_line_at(markdown, before.iOffset));
        CHECK(xuiGetProxy(context, &proxy) == XUI_OK &&
            proxy.fontLoadFile(&proxy, &larger, "test.ttf", 22, 0) == XUI_OK);
        CHECK(xuiSetDefaultFont(context, larger) == XUI_OK && xuiUpdate(context, .016f) == XUI_OK);
        CHECK(xuiDocumentViewHitTest(view, 45, 20, &after) == XUI_OK &&
            after.iKind == XUI_DOC_POSITION_SOURCE && after.iOffset <= used &&
            source_line_at(markdown, after.iOffset) == source_line_at(markdown, before.iOffset));
        CHECK(xuiSetDefaultFont(context, normal) == XUI_OK && xuiUpdate(context, .016f) == XUI_OK);
        CHECK(xuiDocumentViewHitTest(view, 45, 20, &after) == XUI_OK &&
            after.iKind == XUI_DOC_POSITION_SOURCE && after.iOffset <= used &&
            source_line_at(markdown, after.iOffset) == source_line_at(markdown, before.iOffset));
        xuiWidgetDestroy(view); xuiDocumentRelease(document);
        proxy.fontDestroy(&proxy, larger);
    }
    puts("SOURCE and LIVE Markdown zoom/DPI/default-font changes preserve the visible source line");
}
typedef struct dpi_object_probe { xui_context context; unsigned measures; } dpi_object_probe;
static int dpi_object_measure(xui_document_snapshot snapshot, xui_doc_node_id node,
    float width, float zoom, xui_vec2_t* size, float* baseline, void* user)
{
    dpi_object_probe* probe = user; (void)snapshot; (void)node;
    probe->measures++;
    size->fX = fminf(width, 50 * zoom);
    size->fY = 40 * zoom * xuiGetVirtualDpi(probe->context);
    *baseline = size->fY;
    return XUI_OK;
}
static void dpi_layout_and_view_anchor(xui_context context)
{
    xui_document document; xui_document_transaction txn; xui_document_snapshot snapshot;
    xui_doc_view_desc_t view_desc = {0}; xui_doc_renderer_desc_t renderer_desc = {0};
    xui_doc_node_desc_t object = {0}; xui_widget view; xui_document_renderer renderer;
    xui_doc_position_t before, after; xui_doc_rect_t old_size, new_size;
    xui_doc_renderer_stats_t stats = {0}; dpi_object_probe probe = {0};
    uint64_t paragraph, object_id, shaped_before; double x, old_y, new_y;
    unsigned i; int exact;
    CHECK(xuiSetVirtualDpi(context, 1.0f) == XUI_OK);
    CHECK(xuiDocumentCreate(NULL, &document) == XUI_OK);
    CHECK(xuiDocumentBeginTransaction(document, NULL, &txn) == XUI_OK);
    paragraph = add(txn, 1, XUI_DOC_PARAGRAPH, NULL);
    object.iSize = sizeof(object); object.iKind = XUI_DOC_MATH;
    object.sText = "x^2"; object.iTextBytes = 3;
    CHECK(xuiDocumentTxnInsertNode(txn, paragraph, XUI_DOCUMENT_APPEND, &object, &object_id) == XUI_OK);
    for (i = 0; i < 40; i++) {
        char body[64];
        CHECK(snprintf(body, sizeof(body), "Paragraph %u remains visible.", i) > 0);
        paragraph = add(txn, 1, XUI_DOC_PARAGRAPH, NULL);
        add(txn, paragraph, XUI_DOC_TEXT, body);
    }
    CHECK(xuiDocumentTxnCommit(txn, NULL) == XUI_OK); xuiDocumentTxnRelease(txn);
    probe.context = context;
    view_desc.iSize = sizeof(view_desc); view_desc.pDocument = document;
    view_desc.tRenderer.iSize = sizeof(view_desc.tRenderer);
    view_desc.tRenderer.onObjectMeasure = dpi_object_measure;
    view_desc.tRenderer.pUser = &probe;
    CHECK(xuiDocumentViewCreate(context, &view_desc, &view) == XUI_OK);
    CHECK(xuiSetRootWidget(context, view) == XUI_OK);
    CHECK(xuiWidgetSetRect(view, (xui_rect_t){0, 0, 160, 100}) == XUI_OK);
    CHECK(xuiInputViewport(context, 160, 100) == XUI_OK && xuiUpdate(context, .016f) == XUI_OK);
    CHECK(xuiDocumentViewSetScroll(view, 0, 220) == XUI_OK);
    CHECK(xuiDocumentViewHitTest(view, 20, 20, &before) == XUI_OK);
    CHECK(xuiDocumentViewGetScroll(view, &x, &old_y) == XUI_OK && fabs(old_y - 220) < .01);
    CHECK(xuiSetVirtualDpi(context, 1.5f) == XUI_OK && xuiUpdate(context, .016f) == XUI_OK);
    CHECK(xuiDocumentViewGetScroll(view, &x, &new_y) == XUI_OK && new_y > old_y + 15);
    CHECK(xuiDocumentViewHitTest(view, 20, 20, &after) == XUI_OK &&
        after.iNodeId == before.iNodeId && after.iOffset == before.iOffset);
    xuiWidgetDestroy(view);

    CHECK(xuiSetVirtualDpi(context, 1.0f) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
    renderer_desc.iSize = sizeof(renderer_desc);
    renderer_desc.onObjectMeasure = dpi_object_measure; renderer_desc.pUser = &probe;
    CHECK(xuiDocumentRendererCreate(context, &renderer_desc, &renderer) == XUI_OK &&
        xuiDocumentRendererSetSnapshot(renderer, snapshot, NULL) == XUI_OK);
    xuiDocumentSnapshotRelease(snapshot);
    CHECK(xuiDocumentRendererLayout(renderer, 160, 0, 2000) == XUI_OK);
    CHECK(xuiDocumentRendererGetSize(renderer, &old_size, &exact) == XUI_OK && exact);
    stats.iSize = sizeof(stats);
    CHECK(xuiDocumentRendererGetStats(renderer, &stats) == XUI_OK);
    shaped_before = stats.iShapedBytes;
    CHECK(xuiSetVirtualDpi(context, 1.5f) == XUI_OK);
    CHECK(xuiDocumentRendererGetSize(renderer, &new_size, &exact) == XUI_OK && !exact);
    CHECK(xuiDocumentRendererLayout(renderer, 160, 0, 2000) == XUI_OK);
    CHECK(xuiDocumentRendererGetSize(renderer, &new_size, &exact) == XUI_OK && exact &&
        new_size.height > old_size.height + 15);
    CHECK(xuiDocumentRendererGetStats(renderer, &stats) == XUI_OK && stats.iShapedBytes > shaped_before);
    xuiDocumentRendererRelease(renderer); xuiDocumentRelease(document);
    CHECK(xuiSetVirtualDpi(context, 1.0f) == XUI_OK);

    {
        xui_doc_desc_t markdown_desc = {0};
        markdown_desc.iSize = sizeof(markdown_desc); markdown_desc.iProfile = XUI_DOCUMENT_MARKDOWN;
        CHECK(xuiDocumentCreate(&markdown_desc, &document) == XUI_OK &&
            xuiDocumentLoadMarkdown(document, "first\n\nsecond\n\nthird\n", 21) == XUI_OK);
        CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
        CHECK(xuiDocumentRendererCreate(context, NULL, &renderer) == XUI_OK &&
            xuiDocumentRendererSetMode(renderer, XUI_DOC_SOURCE_TEXT) == XUI_OK &&
            xuiDocumentRendererSetSnapshot(renderer, snapshot, NULL) == XUI_OK);
        xuiDocumentSnapshotRelease(snapshot);
        CHECK(xuiDocumentRendererLayout(renderer, 160, 0, 2000) == XUI_OK);
        CHECK(xuiDocumentRendererGetSize(renderer, &old_size, &exact) == XUI_OK && exact);
        stats.iSize = sizeof(stats);
        CHECK(xuiDocumentRendererGetStats(renderer, &stats) == XUI_OK);
        shaped_before = stats.iShapedBytes;
        CHECK(xuiSetVirtualDpi(context, 1.5f) == XUI_OK);
        CHECK(xuiDocumentRendererGetSize(renderer, &new_size, &exact) == XUI_OK && !exact);
        CHECK(xuiDocumentRendererLayout(renderer, 160, 0, 2000) == XUI_OK);
        CHECK(xuiDocumentRendererGetStats(renderer, &stats) == XUI_OK && stats.iShapedBytes > shaped_before);
        xuiDocumentRendererRelease(renderer); xuiDocumentRelease(document);
        CHECK(xuiSetVirtualDpi(context, 1.0f) == XUI_OK);
    }
    puts("Virtual DPI change remeasures standalone Renderer and preserves fixed-height View reading position");
}
static void default_font_theme_anchor(xui_context context)
{
    xui_proxy_t proxy = {0}; xui_theme_t original = {0}, changed;
    xui_font larger = NULL; xui_document document; xui_document_transaction txn;
    xui_document_snapshot snapshot; xui_document_renderer renderer; xui_widget view;
    xui_doc_view_desc_t view_desc = {0}; xui_doc_rect_t old_size, new_size;
    xui_doc_position_t before, after; uint64_t paragraph; double x, old_y, new_y;
    xui_vec2_t measured_before, measured_after;
    unsigned i; int exact;
    CHECK(xuiGetProxy(context, &proxy) == XUI_OK && xuiGetTheme(context, &original) == XUI_OK);
    CHECK(proxy.fontLoadFile(&proxy, &larger, "test.ttf", 22, 0) == XUI_OK);
    CHECK(xuiDocumentCreate(NULL, &document) == XUI_OK &&
        xuiDocumentBeginTransaction(document, NULL, &txn) == XUI_OK);
    for (i = 0; i < 40; i++) {
        char body[64];
        CHECK(snprintf(body, sizeof(body), "Theme font paragraph %u remains visible.", i) > 0);
        paragraph = add(txn, 1, XUI_DOC_PARAGRAPH, NULL);
        add(txn, paragraph, XUI_DOC_TEXT, body);
    }
    CHECK(xuiDocumentTxnCommit(txn, NULL) == XUI_OK); xuiDocumentTxnRelease(txn);
    view_desc.iSize = sizeof(view_desc); view_desc.pDocument = document;
    CHECK(xuiDocumentViewCreate(context, &view_desc, &view) == XUI_OK);
    CHECK(xuiSetRootWidget(context, view) == XUI_OK);
    CHECK(xuiWidgetSetRect(view, (xui_rect_t){0, 0, 180, 100}) == XUI_OK);
    CHECK(xuiInputViewport(context, 180, 100) == XUI_OK && xuiUpdate(context, .016f) == XUI_OK);
    CHECK(xuiDocumentViewSetScroll(view, 0, 220) == XUI_OK);
    CHECK(xuiDocumentViewHitTest(view, 45, 20, &before) == XUI_OK);
    CHECK(xuiDocumentViewGetScroll(view, &x, &old_y) == XUI_OK && fabs(old_y - 220) < .01);
    changed = original; changed.pFont = larger;
    CHECK(xuiSetTheme(context, &changed) == XUI_OK && xuiUpdate(context, .016f) == XUI_OK);
    CHECK(xuiDocumentViewGetScroll(view, &x, &new_y) == XUI_OK && new_y > old_y + 30);
    CHECK(xuiDocumentViewHitTest(view, 45, 20, &after) == XUI_OK &&
        after.iNodeId == before.iNodeId);
    CHECK(xuiSetTheme(context, &original) == XUI_OK && xuiUpdate(context, .016f) == XUI_OK);
    CHECK(xuiDocumentViewHitTest(view, 45, 20, &after) == XUI_OK &&
        after.iNodeId == before.iNodeId);
    xuiWidgetDestroy(view);

    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
    CHECK(xuiDocumentRendererCreate(context, NULL, &renderer) == XUI_OK &&
        xuiDocumentRendererSetSnapshot(renderer, snapshot, NULL) == XUI_OK);
    xuiDocumentSnapshotRelease(snapshot);
    CHECK(xuiDocumentRendererLayout(renderer, 180, 0, 5000) == XUI_OK);
    CHECK(xuiDocumentRendererGetSize(renderer, &old_size, &exact) == XUI_OK && exact);
    CHECK(xuiSetTheme(context, &changed) == XUI_OK);
    CHECK(xuiDocumentRendererGetSize(renderer, &new_size, &exact) == XUI_OK && !exact);
    CHECK(xuiDocumentRendererLayout(renderer, 180, 0, 5000) == XUI_OK);
    CHECK(xuiDocumentRendererGetSize(renderer, &new_size, &exact) == XUI_OK && exact &&
        new_size.height > old_size.height + 30);
    xuiDocumentRendererRelease(renderer);
    CHECK(xuiSetTheme(context, &original) == XUI_OK);

    view_desc.bAutoHeight = 1;
    CHECK(xuiDocumentViewCreate(context, &view_desc, &view) == XUI_OK);
    CHECK(xuiWidgetMeasureContent(view, (xui_vec2_t){180, 10000}, &measured_before) == XUI_OK);
    CHECK(xuiSetTheme(context, &changed) == XUI_OK);
    CHECK(xuiWidgetMeasureContent(view, (xui_vec2_t){180, 10000}, &measured_after) == XUI_OK &&
        measured_after.fY > measured_before.fY + 30);
    xuiWidgetDestroy(view);
    CHECK(xuiSetTheme(context, &original) == XUI_OK);

    view_desc.tRenderer.iSize = sizeof(view_desc.tRenderer);
    view_desc.tRenderer.tFonts.normal = original.pFont;
    CHECK(xuiDocumentViewCreate(context, &view_desc, &view) == XUI_OK);
    CHECK(xuiWidgetMeasureContent(view, (xui_vec2_t){180, 10000}, &measured_before) == XUI_OK);
    CHECK(xuiSetTheme(context, &changed) == XUI_OK);
    CHECK(xuiWidgetMeasureContent(view, (xui_vec2_t){180, 10000}, &measured_after) == XUI_OK &&
        fabsf(measured_after.fY - measured_before.fY) < .01f);
    xuiWidgetDestroy(view); xuiDocumentRelease(document);
    CHECK(xuiSetTheme(context, &original) == XUI_OK);
    proxy.fontDestroy(&proxy, larger);
    puts("Theme default-font change reflows Renderer, fixed/auto View and preserves reading position; explicit font stays fixed");
}
static void unrelated_resource_preserves_layout(xui_context context)
{
    xui_document document; xui_document_transaction txn; xui_document_snapshot snapshot;
    xui_document_renderer renderer; xui_doc_renderer_stats_t before = {0}, after = {0};
    xui_resource_desc_t desc = {0}; xui_resource resource; uint64_t paragraph;
    CHECK(xuiDocumentCreate(NULL, &document) == XUI_OK);
    CHECK(xuiDocumentBeginTransaction(document, NULL, &txn) == XUI_OK);
    paragraph = add(txn, 1, XUI_DOC_PARAGRAPH, NULL);
    add(txn, paragraph, XUI_DOC_TEXT, "ordinary text");
    CHECK(xuiDocumentTxnCommit(txn, NULL) == XUI_OK); xuiDocumentTxnRelease(txn);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
    CHECK(xuiDocumentRendererCreate(context, NULL, &renderer) == XUI_OK &&
        xuiDocumentRendererSetSnapshot(renderer, snapshot, NULL) == XUI_OK);
    xuiDocumentSnapshotRelease(snapshot);
    CHECK(xuiDocumentRendererLayout(renderer, 200, 0, 200) == XUI_OK);
    before.iSize = after.iSize = sizeof(before);
    CHECK(xuiDocumentRendererGetStats(renderer, &before) == XUI_OK && before.iLayoutPasses > 0);
    desc.iSize = sizeof(desc); desc.sName = "doc.unrelated"; desc.iKind = XUI_RESOURCE_SURFACE;
    CHECK(xuiResourceSet(context, &resource, &desc) == XUI_OK);
    CHECK(xuiDocumentRendererLayout(renderer, 200, 0, 200) == XUI_OK &&
        xuiDocumentRendererGetStats(renderer, &after) == XUI_OK && after.iLayoutPasses == before.iLayoutPasses);
    CHECK(xuiResourceRemove(resource) == XUI_OK);
    CHECK(xuiDocumentRendererLayout(renderer, 200, 0, 200) == XUI_OK &&
        xuiDocumentRendererGetStats(renderer, &after) == XUI_OK && after.iLayoutPasses == before.iLayoutPasses);
    xuiDocumentRendererRelease(renderer); xuiDocumentRelease(document);
    puts("Unrelated resource generations leave image-free layout cache intact");
}
typedef struct style_font_probe { int seen; float size; } style_font_probe;
static xui_font style_font_resolve(xui_context context, const char* family,
    uint32_t marks, float size, void* user)
{
    style_font_probe* probe = user; (void)context; (void)marks;
    if (!strcmp(family, "Test family")) { probe->seen++; probe->size = size; }
    return NULL;
}
static void renderer_paint_only_color_refresh(xui_context context, xui_surface target,
    xui_test_proxy_state_t* proxy)
{
    const uint32_t foreground = XUI_COLOR_RGBA(43, 89, 166, 255);
    const uint32_t background = XUI_COLOR_RGBA(221, 192, 68, 255);
    const uint32_t paragraph_background = XUI_COLOR_RGBA(93, 180, 122, 255);
    const uint32_t inherited_foreground = XUI_COLOR_RGBA(138, 47, 153, 255);
    xui_document document; xui_document_transaction txn; xui_document_change_set changes;
    xui_document_snapshot snapshot; xui_document_renderer renderer;
    xui_doc_renderer_stats_t before = {0}, after = {0};
    xui_doc_text_style_t style = {0}; xui_doc_range_t range;
    xui_doc_node_info_t paragraph_info = {0};
    xui_doc_rect_t caret_before, caret_after; xui_doc_position_t at;
    xui_draw_context draw; uint64_t paragraph, text_node;
    const char* body = "Color-only updates keep the existing glyph layout.";
    CHECK(xuiDocumentCreate(NULL, &document) == XUI_OK &&
        xuiDocumentBeginTransaction(document, NULL, &txn) == XUI_OK);
    paragraph = add(txn, 1, XUI_DOC_PARAGRAPH, NULL);
    text_node = add(txn, paragraph, XUI_DOC_TEXT, body);
    CHECK(xuiDocumentTxnCommit(txn, NULL) == XUI_OK); xuiDocumentTxnRelease(txn);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK &&
        xuiDocumentRendererCreate(context, NULL, &renderer) == XUI_OK &&
        xuiDocumentRendererSetSnapshot(renderer, snapshot, NULL) == XUI_OK &&
        xuiDocumentRendererLayout(renderer, 200, 0, 100) == XUI_OK);
    xuiDocumentSnapshotRelease(snapshot);
    before.iSize = after.iSize = sizeof(before);
    at = position(document, text_node, 12);
    CHECK(xuiDocumentRendererGetCaretRect(renderer, &at, &caret_before) == XUI_OK &&
        xuiDocumentRendererGetStats(renderer, &before) == XUI_OK &&
        before.iShapedBytes > 0);
    range.tAnchor = position(document, text_node, 0);
    range.tCaret = position(document, text_node, strlen(body));
    style.iSize = sizeof(style); style.iTextColor = foreground;
    style.iBackgroundColor = background;
    CHECK(xuiDocumentBeginTransaction(document, NULL, &txn) == XUI_OK &&
        xuiDocumentTxnSetTextStyle(txn, &range,
            XUI_DOC_TEXT_STYLE_COLOR | XUI_DOC_TEXT_STYLE_BACKGROUND, &style) == XUI_OK &&
        xuiDocumentTxnCommit(txn, &changes) == XUI_OK);
    xuiDocumentTxnRelease(txn);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK &&
        xuiDocumentRendererSetSnapshot(renderer, snapshot, changes) == XUI_OK &&
        xuiDocumentRendererLayout(renderer, 200, 0, 100) == XUI_OK);
    xuiDocumentSnapshotRelease(snapshot); xuiDocumentChangeSetRelease(changes);
    at = position(document, text_node, 12);
    CHECK(xuiDocumentRendererGetCaretRect(renderer, &at, &caret_after) == XUI_OK &&
        xuiDocumentRendererGetStats(renderer, &after) == XUI_OK &&
        after.iShapedBytes == before.iShapedBytes &&
        fabs(caret_after.x - caret_before.x) < .01 &&
        fabs(caret_after.y - caret_before.y) < .01);
    xuiTestSurfaceReset(target);
    CHECK(proxy->tProxy.drawBegin(&proxy->tProxy, &draw, target) == XUI_OK &&
        xuiDocumentRendererDraw(renderer, draw, 0, 0,
            (xui_rect_t){0, 0, 200, 100}, NULL, 0) == XUI_OK &&
        proxy->tProxy.drawEnd(&proxy->tProxy, draw) == XUI_OK &&
        xuiTestSurfaceGetLastTextColor(target) == foreground &&
        xuiTestSurfaceGetRectFillColorCount(target, background) > 0);
    paragraph_info.iSize = sizeof(paragraph_info);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK &&
        xuiDocumentSnapshotGetNode(snapshot, paragraph, &paragraph_info) == XUI_OK);
    xuiDocumentSnapshotRelease(snapshot);
    paragraph_info.tAttributes.iBackgroundColor = paragraph_background;
    CHECK(xuiDocumentBeginTransaction(document, NULL, &txn) == XUI_OK &&
        xuiDocumentTxnSetAttributes(txn, paragraph,
            &paragraph_info.tAttributes) == XUI_OK &&
        xuiDocumentTxnCommit(txn, &changes) == XUI_OK);
    xuiDocumentTxnRelease(txn);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK &&
        xuiDocumentRendererSetSnapshot(renderer, snapshot, changes) == XUI_OK &&
        xuiDocumentRendererLayout(renderer, 200, 0, 100) == XUI_OK &&
        xuiDocumentRendererGetStats(renderer, &after) == XUI_OK &&
        after.iShapedBytes == before.iShapedBytes);
    xuiDocumentSnapshotRelease(snapshot); xuiDocumentChangeSetRelease(changes);
    xuiTestSurfaceReset(target);
    CHECK(proxy->tProxy.drawBegin(&proxy->tProxy, &draw, target) == XUI_OK &&
        xuiDocumentRendererDraw(renderer, draw, 0, 0,
            (xui_rect_t){0, 0, 200, 100}, NULL, 0) == XUI_OK &&
        proxy->tProxy.drawEnd(&proxy->tProxy, draw) == XUI_OK &&
        xuiTestSurfaceGetRectFillColorCount(target, paragraph_background) > 0);
    range.tAnchor = position(document, text_node, 0);
    range.tCaret = position(document, text_node, strlen(body));
    style.iTextColor = 0;
    CHECK(xuiDocumentBeginTransaction(document, NULL, &txn) == XUI_OK &&
        xuiDocumentTxnSetTextStyle(txn, &range,
            XUI_DOC_TEXT_STYLE_COLOR, &style) == XUI_OK &&
        xuiDocumentTxnCommit(txn, &changes) == XUI_OK);
    xuiDocumentTxnRelease(txn);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK &&
        xuiDocumentRendererSetSnapshot(renderer, snapshot, changes) == XUI_OK &&
        xuiDocumentRendererLayout(renderer, 200, 0, 100) == XUI_OK &&
        xuiDocumentRendererGetStats(renderer, &after) == XUI_OK &&
        after.iShapedBytes == before.iShapedBytes);
    xuiDocumentSnapshotRelease(snapshot); xuiDocumentChangeSetRelease(changes);
    paragraph_info.tAttributes.iTextColor = inherited_foreground;
    CHECK(xuiDocumentBeginTransaction(document, NULL, &txn) == XUI_OK &&
        xuiDocumentTxnSetAttributes(txn, paragraph,
            &paragraph_info.tAttributes) == XUI_OK &&
        xuiDocumentTxnCommit(txn, &changes) == XUI_OK);
    xuiDocumentTxnRelease(txn);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK &&
        xuiDocumentRendererSetSnapshot(renderer, snapshot, changes) == XUI_OK &&
        xuiDocumentRendererLayout(renderer, 200, 0, 100) == XUI_OK &&
        xuiDocumentRendererGetStats(renderer, &after) == XUI_OK &&
        after.iShapedBytes == before.iShapedBytes);
    xuiDocumentSnapshotRelease(snapshot); xuiDocumentChangeSetRelease(changes);
    xuiTestSurfaceReset(target);
    CHECK(proxy->tProxy.drawBegin(&proxy->tProxy, &draw, target) == XUI_OK &&
        xuiDocumentRendererDraw(renderer, draw, 0, 0,
            (xui_rect_t){0, 0, 200, 100}, NULL, 0) == XUI_OK &&
        proxy->tProxy.drawEnd(&proxy->tProxy, draw) == XUI_OK &&
        xuiTestSurfaceGetLastTextColor(target) == inherited_foreground &&
        xuiTestSurfaceGetRectFillColorCount(target, paragraph_background) > 0);
    xuiDocumentRendererRelease(renderer); xuiDocumentRelease(document);
    puts("Renderer paint-only text, paragraph and inherited colors retain shaping and refresh pixels");
}
static void renderer_paint_only_cell_refresh(xui_context context, xui_surface target,
    xui_test_proxy_state_t* proxy)
{
    const uint32_t background = XUI_COLOR_RGBA(137, 202, 112, 255);
    const uint32_t foreground = XUI_COLOR_RGBA(57, 71, 188, 255);
    xui_document document; xui_document_transaction txn; xui_document_change_set changes;
    xui_document_snapshot snapshot; xui_document_renderer renderer;
    xui_doc_node_info_t cell_info = {0};
    xui_doc_renderer_stats_t before = {0}, after = {0};
    xui_draw_context draw; uint64_t table, row, cell, paragraph;
    CHECK(xuiDocumentCreate(NULL, &document) == XUI_OK &&
        xuiDocumentBeginTransaction(document, NULL, &txn) == XUI_OK);
    table = add(txn, 1, XUI_DOC_TABLE, NULL);
    row = add(txn, table, XUI_DOC_ROW, NULL);
    cell = add(txn, row, XUI_DOC_CELL, NULL);
    paragraph = add(txn, cell, XUI_DOC_PARAGRAPH, NULL);
    add(txn, paragraph, XUI_DOC_TEXT, "Cell paint");
    CHECK(xuiDocumentTxnCommit(txn, NULL) == XUI_OK); xuiDocumentTxnRelease(txn);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK &&
        xuiDocumentRendererCreate(context, NULL, &renderer) == XUI_OK &&
        xuiDocumentRendererSetSnapshot(renderer, snapshot, NULL) == XUI_OK &&
        xuiDocumentRendererLayout(renderer, 240, 0, 120) == XUI_OK);
    before.iSize = after.iSize = sizeof(before);
    cell_info.iSize = sizeof(cell_info);
    CHECK(xuiDocumentSnapshotGetNode(snapshot, cell, &cell_info) == XUI_OK &&
        xuiDocumentRendererGetStats(renderer, &before) == XUI_OK);
    xuiDocumentSnapshotRelease(snapshot);
    cell_info.tAttributes.iBackgroundColor = background;
    cell_info.tAttributes.iTextColor = foreground;
    CHECK(xuiDocumentBeginTransaction(document, NULL, &txn) == XUI_OK &&
        xuiDocumentTxnSetAttributes(txn, cell, &cell_info.tAttributes) == XUI_OK &&
        xuiDocumentTxnCommit(txn, &changes) == XUI_OK);
    xuiDocumentTxnRelease(txn);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK &&
        xuiDocumentRendererSetSnapshot(renderer, snapshot, changes) == XUI_OK &&
        xuiDocumentRendererLayout(renderer, 240, 0, 120) == XUI_OK &&
        xuiDocumentRendererGetStats(renderer, &after) == XUI_OK &&
        after.iShapedBytes == before.iShapedBytes);
    xuiDocumentSnapshotRelease(snapshot); xuiDocumentChangeSetRelease(changes);
    xuiTestSurfaceReset(target);
    CHECK(proxy->tProxy.drawBegin(&proxy->tProxy, &draw, target) == XUI_OK &&
        xuiDocumentRendererDraw(renderer, draw, 0, 0,
            (xui_rect_t){0, 0, 240, 120}, NULL, 0) == XUI_OK &&
        proxy->tProxy.drawEnd(&proxy->tProxy, draw) == XUI_OK &&
        xuiTestSurfaceGetRectFillColorCount(target, background) > 0 &&
        xuiTestSurfaceGetLastTextColor(target) == foreground);
    xuiDocumentRendererRelease(renderer); xuiDocumentRelease(document);
    puts("Renderer paint-only cell background and inherited text color retain shaping");
}
static void rich_text_style_render(xui_context context, xui_surface target, xui_test_proxy_state_t* proxy)
{
    const uint32_t foreground = XUI_COLOR_RGBA(42, 75, 180, 255);
    const uint32_t background = XUI_COLOR_RGBA(245, 216, 93, 255);
    xui_document d; xui_document_transaction t; xui_document_snapshot s;
    xui_document_renderer r; xui_doc_renderer_desc_t renderer_desc = {0};
    xui_doc_text_style_t style = {0}; xui_doc_range_t range; style_font_probe font_probe = {0};
    xui_doc_rect_t a0, a1, b0, b1; xui_doc_position_t at, hit; xui_draw_context draw;
    uint64_t paragraph, body, count;
    CHECK(xuiDocumentCreate(NULL, &d) == XUI_OK && xuiDocumentBeginTransaction(d, NULL, &t) == XUI_OK);
    paragraph = add(t, 1, XUI_DOC_PARAGRAPH, NULL); body = add(t, paragraph, XUI_DOC_TEXT, "ab");
    CHECK(xuiDocumentTxnCommit(t, NULL) == XUI_OK); xuiDocumentTxnRelease(t);
    range.tAnchor = position(d, body, 1); range.tCaret = position(d, body, 2);
    style.iSize = sizeof(style); style.iTextColor = foreground;
    style.iBackgroundColor = background; style.fFontSize = 28;
    strcpy(style.sFontFamily, "Test family");
    CHECK(xuiDocumentBeginTransaction(d, NULL, &t) == XUI_OK);
    CHECK(xuiDocumentTxnSetTextStyle(t, &range,
        XUI_DOC_TEXT_STYLE_COLOR | XUI_DOC_TEXT_STYLE_BACKGROUND |
            XUI_DOC_TEXT_STYLE_FONT_SIZE | XUI_DOC_TEXT_STYLE_FONT_FAMILY,
        &style) == XUI_OK);
    CHECK(xuiDocumentTxnCommit(t, NULL) == XUI_OK); xuiDocumentTxnRelease(t);
    CHECK(xuiDocumentAcquireSnapshot(d, &s) == XUI_OK);
    renderer_desc.iSize = sizeof(renderer_desc); renderer_desc.onFont = style_font_resolve;
    renderer_desc.pUser = &font_probe;
    CHECK(xuiDocumentRendererCreate(context, &renderer_desc, &r) == XUI_OK &&
        xuiDocumentRendererSetSnapshot(r, s, NULL) == XUI_OK);
    CHECK(xuiDocumentRendererLayout(r, 300, 0, 300) == XUI_OK);
    CHECK(font_probe.seen > 0 && fabsf(font_probe.size - 28) < .01f);
    at = position(d, body, 0); CHECK(xuiDocumentRendererGetCaretRect(r, &at, &a0) == XUI_OK);
    at = position(d, body, 1); CHECK(xuiDocumentRendererGetCaretRect(r, &at, &a1) == XUI_OK);
    CHECK(xuiDocumentSnapshotFind(s, XUI_DOC_SEMANTIC, "b", 1, NULL, &range, 1, &count) == XUI_OK && count == 1);
    CHECK(xuiDocumentRendererGetCaretRect(r, &range.tAnchor, &b0) == XUI_OK);
    CHECK(xuiDocumentRendererGetCaretRect(r, &range.tCaret, &b1) == XUI_OK);
    CHECK(b1.x - b0.x > a1.x - a0.x + 2);
    CHECK(a0.y > b0.y + 1);
    CHECK(xuiDocumentRendererHitTest(r, (a0.x + a1.x) / 2, b0.y + 1, &hit) == XUI_OK &&
        hit.iNodeId == body && hit.iOffset <= 1);
    xuiTestSurfaceReset(target);
    CHECK(proxy->tProxy.drawBegin(&proxy->tProxy, &draw, target) == XUI_OK);
    CHECK(xuiDocumentRendererDraw(r, draw, 0, 0, (xui_rect_t){0, 0, 300, 100}, NULL, 0) == XUI_OK);
    CHECK(proxy->tProxy.drawEnd(&proxy->tProxy, draw) == XUI_OK &&
        xuiTestSurfaceGetLastTextColor(target) == foreground &&
        xuiTestSurfaceGetRectFillColorCount(target, background) > 0);
    xuiDocumentRendererRelease(r); xuiDocumentSnapshotRelease(s); xuiDocumentRelease(d);
    puts("Rich text style rendering: font metrics, foreground and background passed");
}
static void html_block_style_render(xui_context context, xui_surface target,
    xui_test_proxy_state_t* proxy)
{
    static const char html[] =
        "<body style=\"color:#112233;font-size:24px;font-family:'Test family'\">"
        "<p style=\"background-color:#556677\">visible</p></body>";
    const uint32_t foreground = XUI_COLOR_RGBA(17, 34, 51, 255);
    const uint32_t background = XUI_COLOR_RGBA(85, 102, 119, 255);
    xui_document_fragment fragment = NULL;
    xui_document document = NULL; xui_document_transaction transaction = NULL;
    xui_document_snapshot snapshot = NULL; xui_document_renderer renderer = NULL;
    xui_doc_renderer_desc_t renderer_desc = {0};
    xui_doc_node_info_t block = {0}, leaf = {0};
    xui_doc_range_t caret; xui_doc_text_style_query_t query = {0};
    style_font_probe probe = {0}; xui_draw_context draw;
    uint64_t first = 0, count = 0, paragraph, text_node;
    CHECK(xuiDocumentFragmentImportHtml(html, sizeof(html) - 1, &fragment) == XUI_OK);
    CHECK(xuiDocumentCreate(NULL, &document) == XUI_OK);
    CHECK(xuiDocumentBeginTransaction(document, NULL, &transaction) == XUI_OK);
    CHECK(xuiDocumentTxnInsertFragment(transaction, fragment, 1, 0,
        &first, &count) == XUI_OK && count == 1);
    CHECK(xuiDocumentTxnCommit(transaction, NULL) == XUI_OK);
    xuiDocumentTxnRelease(transaction); xuiDocumentFragmentRelease(fragment);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
    CHECK(xuiDocumentSnapshotGetChild(snapshot, 1, 0, &paragraph) == XUI_OK);
    CHECK(xuiDocumentSnapshotGetChild(snapshot, paragraph, 0, &text_node) == XUI_OK);
    block.iSize = leaf.iSize = sizeof(block);
    CHECK(xuiDocumentSnapshotGetNode(snapshot, paragraph, &block) == XUI_OK &&
        block.tAttributes.iTextColor == foreground &&
        block.tAttributes.iBackgroundColor == background &&
        block.tAttributes.fFontSize == 24);
    CHECK(xuiDocumentSnapshotGetNode(snapshot, text_node, &leaf) == XUI_OK &&
        !leaf.tAttributes.iTextColor && !leaf.tAttributes.iBackgroundColor &&
        !leaf.tAttributes.fFontSize);
    caret.tAnchor = caret.tCaret = position(document, text_node, 0);
    query.iSize = sizeof(query);
    CHECK(xuiDocumentSnapshotQueryTextStyle(snapshot, &caret, &query) == XUI_OK &&
        query.bHasText && query.tStyle.iTextColor == foreground &&
        !query.tStyle.iBackgroundColor && query.tStyle.fFontSize == 24 &&
        !strcmp(query.tStyle.sFontFamily, "Test family"));
    renderer_desc.iSize = sizeof(renderer_desc);
    renderer_desc.onFont = style_font_resolve; renderer_desc.pUser = &probe;
    CHECK(xuiDocumentRendererCreate(context, &renderer_desc, &renderer) == XUI_OK &&
        xuiDocumentRendererSetSnapshot(renderer, snapshot, NULL) == XUI_OK);
    CHECK(xuiDocumentRendererLayout(renderer, 300, 0, 300) == XUI_OK &&
        probe.seen > 0 && fabsf(probe.size - 24) < .01f);
    xuiTestSurfaceReset(target);
    CHECK(proxy->tProxy.drawBegin(&proxy->tProxy, &draw, target) == XUI_OK);
    CHECK(xuiDocumentRendererDraw(renderer, draw, 0, 0,
        (xui_rect_t){0, 0, 300, 100}, NULL, 0) == XUI_OK);
    CHECK(proxy->tProxy.drawEnd(&proxy->tProxy, draw) == XUI_OK &&
        xuiTestSurfaceGetLastTextColor(target) == foreground &&
        xuiTestSurfaceGetRectFillColorCount(target, background) > 0);
    xuiDocumentRendererRelease(renderer); xuiDocumentSnapshotRelease(snapshot);
    xuiDocumentRelease(document);
    puts("HTML block styles: inherited color, background and font size reach native text rendering");
}
static void explicit_zero_color_render(xui_context context, xui_surface target,
    xui_test_proxy_state_t* proxy)
{
    const uint32_t theme_text = XUI_COLOR_RGBA(34, 67, 101, 255);
    const uint32_t theme_highlight = XUI_COLOR_RGBA(211, 53, 89, 255);
    xui_document document = NULL; xui_document_transaction transaction = NULL;
    xui_document_snapshot snapshot = NULL; xui_document_renderer renderer = NULL;
    xui_doc_renderer_desc_t renderer_desc = {0}; xui_doc_attributes_t attrs = {0};
    xui_draw_context draw; uint64_t paragraph, leaf;
    CHECK(xuiDocumentCreate(NULL, &document) == XUI_OK &&
        xuiDocumentBeginTransaction(document, NULL, &transaction) == XUI_OK);
    paragraph = add(transaction, 1, XUI_DOC_PARAGRAPH, NULL);
    leaf = add(transaction, paragraph, XUI_DOC_TEXT, "transparent");
    attrs.iFlags = XUI_DOC_TEXT_COLOR_EXPLICIT_ZERO;
    CHECK(xuiDocumentTxnSetAttributes(transaction, paragraph, &attrs) == XUI_OK);
    attrs.iFlags = XUI_DOC_BACKGROUND_COLOR_EXPLICIT_ZERO;
    attrs.iMarks = XUI_DOC_HIGHLIGHT;
    CHECK(xuiDocumentTxnSetAttributes(transaction, leaf, &attrs) == XUI_OK &&
        xuiDocumentTxnCommit(transaction, NULL) == XUI_OK);
    xuiDocumentTxnRelease(transaction);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
    renderer_desc.iSize = sizeof(renderer_desc);
    renderer_desc.iTextColor = theme_text;
    renderer_desc.iHighlightColor = theme_highlight;
    CHECK(xuiDocumentRendererCreate(context, &renderer_desc, &renderer) == XUI_OK &&
        xuiDocumentRendererSetSnapshot(renderer, snapshot, NULL) == XUI_OK &&
        xuiDocumentRendererLayout(renderer, 300, 0, 300) == XUI_OK);
    xuiTestSurfaceReset(target);
    CHECK(proxy->tProxy.drawBegin(&proxy->tProxy, &draw, target) == XUI_OK);
    CHECK(xuiDocumentRendererDraw(renderer, draw, 0, 0,
        (xui_rect_t){0, 0, 300, 100}, NULL, 0) == XUI_OK);
    CHECK(proxy->tProxy.drawEnd(&proxy->tProxy, draw) == XUI_OK &&
        xuiTestSurfaceGetTextDrawCount(target) > 0 &&
        xuiTestSurfaceGetLastTextColor(target) == 0 &&
        xuiTestSurfaceGetRectFillColorCount(target, theme_highlight) == 0);
    xuiDocumentRendererRelease(renderer); xuiDocumentSnapshotRelease(snapshot);
    xuiDocumentRelease(document);
    puts("Explicit transparent black foreground and background reach the native renderer");
}
static void current_color_render(xui_context context, xui_surface target,
    xui_test_proxy_state_t* proxy)
{
    const uint32_t theme_a = XUI_COLOR_RGBA(31, 70, 111, 255);
    const uint32_t theme_b = XUI_COLOR_RGBA(147, 60, 29, 255);
    const uint32_t link_color = XUI_COLOR_RGBA(190, 22, 170, 255);
    const uint32_t inherited = XUI_COLOR_RGBA(46, 135, 77, 255);
    xui_document document = NULL; xui_document_transaction transaction = NULL;
    xui_document_snapshot snapshot = NULL; xui_document_change_set changes = NULL;
    xui_document_renderer renderer = NULL;
    xui_doc_renderer_desc_t desc = {0}; xui_doc_attributes_t attrs = {0};
    xui_doc_renderer_stats_t before = {0}, after = {0};
    xui_draw_context draw; uint64_t paragraph, leaf;
    CHECK(xuiDocumentCreate(NULL, &document) == XUI_OK &&
        xuiDocumentBeginTransaction(document, NULL, &transaction) == XUI_OK);
    paragraph = add(transaction, 1, XUI_DOC_PARAGRAPH, NULL);
    leaf = add(transaction, paragraph, XUI_DOC_TEXT, "current");
    attrs.iFlags = XUI_DOC_BACKGROUND_COLOR_CURRENT;
    CHECK(xuiDocumentTxnSetAttributes(transaction, paragraph, &attrs) == XUI_OK);
    attrs.iMarks = XUI_DOC_LINK;
    attrs.iFlags = XUI_DOC_TEXT_COLOR_CURRENT | XUI_DOC_BACKGROUND_COLOR_CURRENT;
    CHECK(xuiDocumentTxnSetAttributes(transaction, leaf, &attrs) == XUI_OK &&
        xuiDocumentTxnSetResource(transaction, leaf, "https://example.test/", "", "") == XUI_OK &&
        xuiDocumentTxnCommit(transaction, NULL) == XUI_OK);
    xuiDocumentTxnRelease(transaction); transaction = NULL;
    desc.iSize = sizeof(desc); desc.iTextColor = theme_a; desc.iLinkColor = link_color;
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK &&
        xuiDocumentRendererCreate(context, &desc, &renderer) == XUI_OK &&
        xuiDocumentRendererSetSnapshot(renderer, snapshot, NULL) == XUI_OK &&
        xuiDocumentRendererLayout(renderer, 300, 0, 300) == XUI_OK);
    xuiDocumentSnapshotRelease(snapshot); snapshot = NULL;
    before.iSize = after.iSize = sizeof(before);
    CHECK(xuiDocumentRendererGetStats(renderer, &before) == XUI_OK);
    xuiTestSurfaceReset(target);
    CHECK(proxy->tProxy.drawBegin(&proxy->tProxy, &draw, target) == XUI_OK &&
        xuiDocumentRendererDraw(renderer, draw, 0, 0,
            (xui_rect_t){0, 0, 300, 100}, NULL, 0) == XUI_OK &&
        proxy->tProxy.drawEnd(&proxy->tProxy, draw) == XUI_OK &&
        xuiTestSurfaceGetLastTextColor(target) == theme_a &&
        xuiTestSurfaceGetRectFillColorCount(target, theme_a) > 0 &&
        !xuiTestSurfaceGetRectFillColorCount(target, link_color));
    attrs.iMarks = 0; attrs.iFlags = XUI_DOC_BACKGROUND_COLOR_CURRENT;
    attrs.iTextColor = inherited;
    CHECK(xuiDocumentBeginTransaction(document, NULL, &transaction) == XUI_OK &&
        xuiDocumentTxnSetAttributes(transaction, paragraph, &attrs) == XUI_OK &&
        xuiDocumentTxnCommit(transaction, &changes) == XUI_OK);
    xuiDocumentTxnRelease(transaction); transaction = NULL;
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK &&
        xuiDocumentRendererSetSnapshot(renderer, snapshot, changes) == XUI_OK &&
        xuiDocumentRendererLayout(renderer, 300, 0, 300) == XUI_OK &&
        xuiDocumentRendererGetStats(renderer, &after) == XUI_OK &&
        after.iShapedBytes == before.iShapedBytes);
    xuiDocumentSnapshotRelease(snapshot); snapshot = NULL;
    xuiDocumentChangeSetRelease(changes); changes = NULL;
    xuiTestSurfaceReset(target);
    CHECK(proxy->tProxy.drawBegin(&proxy->tProxy, &draw, target) == XUI_OK &&
        xuiDocumentRendererDraw(renderer, draw, 0, 0,
            (xui_rect_t){0, 0, 300, 100}, NULL, 0) == XUI_OK &&
        proxy->tProxy.drawEnd(&proxy->tProxy, draw) == XUI_OK &&
        xuiTestSurfaceGetLastTextColor(target) == inherited &&
        xuiTestSurfaceGetRectFillColorCount(target, inherited) > 0);
    xuiDocumentRendererRelease(renderer); renderer = NULL;
    attrs.iTextColor = 0;
    CHECK(xuiDocumentBeginTransaction(document, NULL, &transaction) == XUI_OK &&
        xuiDocumentTxnSetAttributes(transaction, paragraph, &attrs) == XUI_OK &&
        xuiDocumentTxnCommit(transaction, NULL) == XUI_OK);
    xuiDocumentTxnRelease(transaction); transaction = NULL;
    desc.iTextColor = theme_b;
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK &&
        xuiDocumentRendererCreate(context, &desc, &renderer) == XUI_OK &&
        xuiDocumentRendererSetSnapshot(renderer, snapshot, NULL) == XUI_OK &&
        xuiDocumentRendererLayout(renderer, 300, 0, 300) == XUI_OK);
    xuiDocumentSnapshotRelease(snapshot); snapshot = NULL;
    xuiTestSurfaceReset(target);
    CHECK(proxy->tProxy.drawBegin(&proxy->tProxy, &draw, target) == XUI_OK &&
        xuiDocumentRendererDraw(renderer, draw, 0, 0,
            (xui_rect_t){0, 0, 300, 100}, NULL, 0) == XUI_OK &&
        proxy->tProxy.drawEnd(&proxy->tProxy, draw) == XUI_OK &&
        xuiTestSurfaceGetLastTextColor(target) == theme_b &&
        xuiTestSurfaceGetRectFillColorCount(target, theme_b) > 0 &&
        !xuiTestSurfaceGetRectFillColorCount(target, theme_a));
    xuiDocumentRendererRelease(renderer); xuiDocumentRelease(document);
    puts("currentColor rendering: link override, inherited text and theme repaint passed");
}
static void html_table_cell_style_render(xui_context context, xui_surface target,
    xui_test_proxy_state_t* proxy)
{
    static const char html[] =
        "<table style=\"color:#224466\"><tr>"
        "<th style=\"color:#123456;background-color:currentColor\">"
        "<p style=\"color:#abcdef\">A</p></th>"
        "<td style=\"background-color:transparent\">B</td>"
        "<td style=\"background-color:currentColor\">C</td>"
        "</tr></table>";
    const uint32_t first = XUI_COLOR_RGBA(18, 52, 86, 255);
    const uint32_t third = XUI_COLOR_RGBA(34, 68, 102, 255);
    const uint32_t default_cell = XUI_COLOR_RGBA(221, 220, 219, 255);
    const uint32_t default_header = XUI_COLOR_RGBA(203, 202, 201, 255);
    xui_document_fragment fragment = NULL;
    xui_document document = NULL; xui_document_transaction transaction = NULL;
    xui_document_snapshot snapshot = NULL; xui_document_renderer renderer = NULL;
    xui_doc_renderer_desc_t desc = {0};
    xui_draw_context draw; uint64_t inserted, count;
    CHECK(xuiDocumentFragmentImportHtml(html, sizeof(html) - 1, &fragment) == XUI_OK &&
        xuiDocumentCreate(NULL, &document) == XUI_OK &&
        xuiDocumentBeginTransaction(document, NULL, &transaction) == XUI_OK &&
        xuiDocumentTxnInsertFragment(transaction, fragment, 1, 0,
            &inserted, &count) == XUI_OK && count == 1 &&
        xuiDocumentTxnCommit(transaction, NULL) == XUI_OK);
    xuiDocumentTxnRelease(transaction); xuiDocumentFragmentRelease(fragment);
    desc.iSize = sizeof(desc);
    desc.iTableCellColor = default_cell; desc.iTableHeaderColor = default_header;
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK &&
        xuiDocumentRendererCreate(context, &desc, &renderer) == XUI_OK &&
        xuiDocumentRendererSetSnapshot(renderer, snapshot, NULL) == XUI_OK &&
        xuiDocumentRendererLayout(renderer, 390, 0, 390) == XUI_OK);
    xuiDocumentSnapshotRelease(snapshot);
    xuiTestSurfaceReset(target);
    CHECK(proxy->tProxy.drawBegin(&proxy->tProxy, &draw, target) == XUI_OK &&
        xuiDocumentRendererDraw(renderer, draw, 0, 0,
            (xui_rect_t){0, 0, 390, 140}, NULL, 0) == XUI_OK &&
        proxy->tProxy.drawEnd(&proxy->tProxy, draw) == XUI_OK &&
        xuiTestSurfaceGetRectFillColorCount(target, first) > 0 &&
        xuiTestSurfaceGetRectFillColorCount(target, third) > 0 &&
        !xuiTestSurfaceGetRectFillColorCount(target, default_cell) &&
        !xuiTestSurfaceGetRectFillColorCount(target, default_header));
    xuiDocumentRendererRelease(renderer); xuiDocumentRelease(document);
    puts("HTML table cell backgrounds resolve against cell text color and suppress defaults");
}
static void table_cell_alignment_render(xui_context context)
{
    static const char gfm[] = "| H | C |\n| ---: | :---: |\n| x | y |\n";
    xui_document document = NULL; xui_document_transaction transaction = NULL;
    xui_document_snapshot snapshot = NULL; xui_document_renderer renderer = NULL;
    xui_document_change_set changes = NULL;
    xui_doc_node_info_t info = {0}; xui_doc_cell_hit_t hit = {0};
    xui_doc_rect_t caret; xui_doc_range_t range, after;
    xui_doc_block_style_t style = {0}; xui_doc_desc_t md = {0};
    uint64_t table, row, cell[3], paragraph[3], leaf[3]; unsigned i;
    info.iSize = sizeof(info); hit.iSize = sizeof(hit);
    CHECK(xuiDocumentCreate(NULL, &document) == XUI_OK &&
        xuiDocumentBeginTransaction(document, NULL, &transaction) == XUI_OK);
    table = add(transaction, 1, XUI_DOC_TABLE, NULL);
    row = add(transaction, table, XUI_DOC_ROW, NULL);
    for (i = 0; i < 3; i++) {
        cell[i] = add(transaction, row, XUI_DOC_CELL, NULL);
        paragraph[i] = add(transaction, cell[i], XUI_DOC_PARAGRAPH, NULL);
        leaf[i] = add(transaction, paragraph[i], XUI_DOC_TEXT, "x");
        CHECK(xuiDocumentTxnGetNode(transaction, cell[i], &info) == XUI_OK);
        info.tAttributes.iAlignment = i == 2 ? 1 : 2;
        CHECK(xuiDocumentTxnSetAttributes(transaction, cell[i],
            &info.tAttributes) == XUI_OK);
    }
    CHECK(xuiDocumentTxnGetNode(transaction, paragraph[1], &info) == XUI_OK);
    info.tAttributes.iFlags |= XUI_DOC_ALIGNMENT_EXPLICIT_LEFT;
    CHECK(xuiDocumentTxnSetAttributes(transaction, paragraph[1],
        &info.tAttributes) == XUI_OK &&
        xuiDocumentTxnCommit(transaction, NULL) == XUI_OK);
    xuiDocumentTxnRelease(transaction); transaction = NULL;
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK &&
        xuiDocumentRendererCreate(context, NULL, &renderer) == XUI_OK &&
        xuiDocumentRendererSetSnapshot(renderer, snapshot, NULL) == XUI_OK &&
        xuiDocumentRendererLayout(renderer, 390, 0, 390) == XUI_OK);
    xuiDocumentSnapshotRelease(snapshot); snapshot = NULL;
    for (i = 0; i < 3; i++) {
        hit.iSize = sizeof(hit);
        CHECK(xuiDocumentRendererGetCellRect(renderer, cell[i], &hit) == XUI_OK &&
            xuiDocumentRendererGetCaretRect(renderer,
                &(xui_doc_position_t){.iSize = sizeof(xui_doc_position_t),
                    .iDocumentId = xuiDocumentGetIdentity(document),
                    .iRevision = xuiDocumentGetRevision(document),
                    .iNodeId = leaf[i], .iKind = XUI_DOC_POSITION_TEXT},
                &caret) == XUI_OK);
        if (i == 0) CHECK(caret.x > hit.tBounds.x + hit.tBounds.width * .5);
        if (i == 1) CHECK(caret.x < hit.tBounds.x + hit.tBounds.width * .5);
        if (i == 2) CHECK(fabs(caret.x -
            (hit.tBounds.x + hit.tBounds.width * .5)) < 30);
    }
    range.tAnchor = range.tCaret = position(document, leaf[1], 0);
    style.iSize = sizeof(style); style.bAlignmentInherited = 1;
    CHECK(xuiDocumentBeginTransaction(document, NULL, &transaction) == XUI_OK &&
        xuiDocumentTxnSetBlockStyleRange(transaction, &range,
            XUI_DOC_BLOCK_STYLE_ALIGNMENT, &style, &after) == XUI_OK &&
        xuiDocumentTxnCommit(transaction, &changes) == XUI_OK);
    xuiDocumentTxnRelease(transaction); transaction = NULL;
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK &&
        xuiDocumentRendererSetSnapshot(renderer, snapshot, changes) == XUI_OK &&
        xuiDocumentRendererLayout(renderer, 390, 0, 390) == XUI_OK);
    xuiDocumentSnapshotRelease(snapshot); snapshot = NULL;
    xuiDocumentChangeSetRelease(changes); changes = NULL;
    CHECK(xuiDocumentRendererGetCellRect(renderer, cell[1], &hit) == XUI_OK &&
        xuiDocumentRendererGetCaretRect(renderer,
            &(xui_doc_position_t){.iSize = sizeof(xui_doc_position_t),
                .iDocumentId = xuiDocumentGetIdentity(document),
                .iRevision = xuiDocumentGetRevision(document),
                .iNodeId = leaf[1], .iKind = XUI_DOC_POSITION_TEXT},
            &caret) == XUI_OK &&
        caret.x > hit.tBounds.x + hit.tBounds.width * .5);
    xuiDocumentRendererRelease(renderer); xuiDocumentRelease(document);

    md.iSize = sizeof(md); md.iProfile = XUI_DOCUMENT_MARKDOWN;
    md.iMarkdownDialect = XUI_MD_GFM;
    CHECK(xuiDocumentCreate(&md, &document) == XUI_OK &&
        xuiDocumentLoadMarkdown(document, gfm, sizeof(gfm) - 1) == XUI_OK &&
        xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK &&
        xuiDocumentSnapshotGetChild(snapshot, 1, 0, &table) == XUI_OK &&
        xuiDocumentSnapshotGetChild(snapshot, table, 1, &row) == XUI_OK);
    for (i = 0; i < 2; i++) {
        CHECK(xuiDocumentSnapshotGetChild(snapshot, row, i, &cell[i]) == XUI_OK &&
            xuiDocumentSnapshotGetChild(snapshot, cell[i], 0, &paragraph[i]) == XUI_OK &&
            xuiDocumentSnapshotGetChild(snapshot, paragraph[i], 0, &leaf[i]) == XUI_OK);
    }
    CHECK(xuiDocumentRendererCreate(context, NULL, &renderer) == XUI_OK &&
        xuiDocumentRendererSetSnapshot(renderer, snapshot, NULL) == XUI_OK &&
        xuiDocumentRendererLayout(renderer, 390, 0, 390) == XUI_OK);
    for (i = 0; i < 2; i++) {
        hit.iSize = sizeof(hit);
        CHECK(xuiDocumentRendererGetCellRect(renderer, cell[i], &hit) == XUI_OK &&
            xuiDocumentRendererGetCaretRect(renderer,
                &(xui_doc_position_t){.iSize = sizeof(xui_doc_position_t),
                    .iDocumentId = xuiDocumentGetIdentity(document),
                    .iRevision = xuiDocumentGetRevision(document),
                    .iNodeId = leaf[i], .iKind = XUI_DOC_POSITION_TEXT},
                &caret) == XUI_OK);
        if (!i) CHECK(caret.x > hit.tBounds.x + hit.tBounds.width * .5);
        else CHECK(fabs(caret.x -
            (hit.tBounds.x + hit.tBounds.width * .5)) < 30);
    }
    xuiDocumentRendererRelease(renderer); xuiDocumentSnapshotRelease(snapshot);
    xuiDocumentRelease(document);
    puts("Table alignment: cell inheritance, explicit left override, reset and GFM layout passed");
}
static void paragraph_background_prefix(xui_context context, xui_surface target,
    xui_test_proxy_state_t* proxy)
{
    const uint32_t background = XUI_COLOR_RGBA(121, 52, 203, 255);
    const size_t bytes = 100000; char* content = malloc(bytes + 1);
    xui_document document = NULL; xui_document_transaction transaction = NULL;
    xui_document_snapshot snapshot = NULL; xui_document_renderer renderer = NULL;
    xui_doc_attributes_t attrs = {0}; xui_doc_renderer_stats_t stats = {0};
    xui_draw_context draw; uint64_t paragraph; size_t i;
    CHECK(content != NULL);
    for (i = 0; i < bytes; i++) content[i] = "word "[i % 5];
    content[bytes] = 0;
    CHECK(xuiDocumentCreate(NULL, &document) == XUI_OK &&
        xuiDocumentBeginTransaction(document, NULL, &transaction) == XUI_OK);
    paragraph = add(transaction, 1, XUI_DOC_PARAGRAPH, NULL);
    add(transaction, paragraph, XUI_DOC_TEXT, content);
    attrs.iBackgroundColor = background;
    CHECK(xuiDocumentTxnSetAttributes(transaction, paragraph, &attrs) == XUI_OK &&
        xuiDocumentTxnCommit(transaction, NULL) == XUI_OK);
    xuiDocumentTxnRelease(transaction); free(content);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK &&
        xuiDocumentRendererCreate(context, NULL, &renderer) == XUI_OK &&
        xuiDocumentRendererSetSnapshot(renderer, snapshot, NULL) == XUI_OK);
    stats.iSize = sizeof(stats);
    CHECK(xuiDocumentRendererLayout(renderer, 300, 0, 100) == XUI_OK &&
        xuiDocumentRendererGetStats(renderer, &stats) == XUI_OK &&
        stats.iShapedBytes > 0 && stats.iShapedBytes < bytes / 2);
    xuiTestSurfaceReset(target);
    CHECK(proxy->tProxy.drawBegin(&proxy->tProxy, &draw, target) == XUI_OK);
    CHECK(xuiDocumentRendererDraw(renderer, draw, 0, 0,
        (xui_rect_t){0, 0, 300, 100}, NULL, 0) == XUI_OK);
    CHECK(proxy->tProxy.drawEnd(&proxy->tProxy, draw) == XUI_OK &&
        xuiTestSurfaceGetRectFillColorCount(target, background) > 0);
    xuiTestSurfaceReset(target);
    CHECK(proxy->tProxy.drawBegin(&proxy->tProxy, &draw, target) == XUI_OK);
    CHECK(xuiDocumentRendererDraw(renderer, draw, 0, 0,
        (xui_rect_t){0, 700, 300, 100}, NULL, 0) == XUI_OK);
    CHECK(proxy->tProxy.drawEnd(&proxy->tProxy, draw) == XUI_OK &&
        xuiTestSurfaceGetRectFillColorCount(target, background) > 0 &&
        xuiDocumentRendererGetStats(renderer, &stats) == XUI_OK &&
        stats.iShapedBytes < bytes / 2);
    xuiDocumentRendererRelease(renderer); xuiDocumentSnapshotRelease(snapshot);
    xuiDocumentRelease(document);
    puts("Long paragraph prefix paints its block background without full shaping");
}
typedef struct link_activation { unsigned calls; char uri[64]; } link_activation;
static void link_activated(xui_widget view, xui_doc_node_id node, const char* resource, void* user)
{
    link_activation* state = user; (void)view; (void)node;
    state->calls++; snprintf(state->uri, sizeof(state->uri), "%s", resource ? resource : "");
}
static void link_view_activation(xui_context context)
{
    xui_doc_desc_t md = {0}; xui_doc_view_desc_t desc = {0};
    xui_document d; xui_widget view; xui_event_t event = {0};
    xui_doc_position_t hit; xui_document_snapshot s; xui_doc_node_info_t info = {0};
    link_activation state = {0};
    md.iSize = sizeof(md); md.iProfile = XUI_DOCUMENT_MARKDOWN;
    CHECK(xuiDocumentCreate(&md, &d) == XUI_OK &&
        xuiDocumentLoadMarkdown(d, "[go](/target)\n", 14) == XUI_OK);
    desc.iSize = sizeof(desc); desc.pDocument = d; desc.bDisableSelection = 1;
    desc.onActivate = link_activated; desc.pUser = &state;
    CHECK(xuiDocumentViewCreate(context, &desc, &view) == XUI_OK && xuiSetRootWidget(context, view) == XUI_OK);
    CHECK(xuiWidgetSetRect(view, (xui_rect_t){0, 0, 300, 100}) == XUI_OK);
    CHECK(xuiDocumentViewHitTest(view, 5, 10, &hit) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(d, &s) == XUI_OK);
    info.iSize = sizeof(info);
    CHECK(xuiDocumentSnapshotGetNode(s, hit.iNodeId, &info) == XUI_OK &&
        (info.tAttributes.iMarks & XUI_DOC_LINK));
    xuiDocumentSnapshotRelease(s);
    event.iSize = sizeof(event); event.pTarget = view; event.fX = 5; event.fY = 10;
    event.iType = XUI_EVENT_POINTER_DOWN; CHECK(xuiDispatchEvent(context, &event) == XUI_OK);
    event.iType = XUI_EVENT_POINTER_UP; CHECK(xuiDispatchEvent(context, &event) == XUI_OK);
    CHECK(state.calls == 1 && !strcmp(state.uri, "/target"));
    xuiWidgetDestroy(view); xuiDocumentRelease(d);
    memset(&state, 0, sizeof(state));
    CHECK(xuiDocumentCreate(&md, &d) == XUI_OK &&
        xuiDocumentLoadMarkdown(d, "[![alt](/asset)](/image-target)\n",
            strlen("[![alt](/asset)](/image-target)\n")) == XUI_OK);
    desc.pDocument = d;
    CHECK(xuiDocumentViewCreate(context, &desc, &view) == XUI_OK &&
        xuiSetRootWidget(context, view) == XUI_OK);
    CHECK(xuiWidgetSetRect(view, (xui_rect_t){0, 0, 300, 100}) == XUI_OK);
    CHECK(xuiDocumentViewHitTest(view, 5, 10, &hit) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(d, &s) == XUI_OK);
    CHECK(xuiDocumentSnapshotGetNode(s, hit.iNodeId, &info) == XUI_OK &&
        info.iKind == XUI_DOC_IMAGE && (info.tAttributes.iMarks & XUI_DOC_LINK));
    xuiDocumentSnapshotRelease(s);
    event.pTarget = view; event.fX = 5; event.fY = 10;
    event.iType = XUI_EVENT_POINTER_DOWN; CHECK(xuiDispatchEvent(context, &event) == XUI_OK);
    event.iType = XUI_EVENT_POINTER_UP; CHECK(xuiDispatchEvent(context, &event) == XUI_OK);
    CHECK(state.calls == 1 && !strcmp(state.uri, "/image-target"));
    xuiWidgetDestroy(view); xuiDocumentRelease(d);
    puts("Link View activation: text and linked image return their hyperlink destinations");
}
static void live_markdown(xui_context context)
{
    const char* source = "# Heading\r\n\r\nBody **strong** text.\r\n\r\n> - First\r\n> - Second\r\n\r\n| A | B |\r\n| - | - |\r\n| C | D |\r\n\r\n```c\r\nint value = 1;\r\n```\r\n\r\nTail\r\n";
    const char* needles[] = {"Heading", "strong", "Second", "| C", "int value", "Tail"};
    const char* starts[] = {"# Heading", "Body", "> - First", "| A", "```c", "Tail"};
    xui_doc_desc_t desc = {0}; xui_document d; xui_document_snapshot snapshot;
    xui_document_renderer r; xui_doc_renderer_stats_t stats = {0}; unsigned i;
    xui_doc_position_t p, hit; xui_doc_rect_t caret; uint64_t start, end, revision;
    desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN;
    CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK); CHECK(xuiDocumentLoadMarkdown(d, source, strlen(source)) == XUI_OK);
    revision = xuiDocumentGetRevision(d); CHECK(xuiDocumentAcquireSnapshot(d, &snapshot) == XUI_OK);
    CHECK(xuiDocumentRendererCreate(context, NULL, &r) == XUI_OK); CHECK(xuiDocumentRendererSetSnapshot(r, snapshot, NULL) == XUI_OK);
    CHECK(xuiDocumentRendererSetMode(r, XUI_DOC_LIVE_MARKDOWN) == XUI_OK);
    for (i = 0; i < sizeof(needles) / sizeof(*needles); i++) {
        p = position(d, 1, (uint64_t)(strstr(source, needles[i]) - source)); p.iKind = XUI_DOC_POSITION_SOURCE;
        CHECK(xuiDocumentRendererSetActivePosition(r, &p) == XUI_OK);
        CHECK(xuiDocumentRendererGetActiveSourceRange(r, &start, &end) == XUI_OK);
        CHECK(start == (uint64_t)(strstr(source, starts[i]) - source) && p.iOffset >= start && p.iOffset < end);
        CHECK(xuiDocumentRendererLayout(r, 320, 0, 10000) == XUI_OK);
        stats.iSize = sizeof(stats); CHECK(xuiDocumentRendererGetStats(r, &stats) == XUI_OK);
        CHECK(stats.iSourceBlocks > 0 && stats.iSourceBlocks < stats.iBlocks && !stats.bLiveSourceFallback);
        CHECK(xuiDocumentRendererGetCaretRect(r, &p, &caret) == XUI_OK);
        CHECK(xuiDocumentRendererHitTest(r, caret.x + .1, caret.y + caret.height / 2, &hit) == XUI_OK);
        CHECK(hit.iKind == XUI_DOC_POSITION_SOURCE && hit.iOffset == p.iOffset);
        /* Every raw byte boundary in the active CRLF block has caret geometry,
         * including closing fences and syntax that has no semantic leaf. */
        for (p.iOffset = start; p.iOffset < end; p.iOffset++)
            CHECK(xuiDocumentRendererGetCaretRect(r, &p, &caret) == XUI_OK);
    }
    p = position(d, 1, 2); p.iKind = XUI_DOC_POSITION_SOURCE;
    CHECK(xuiDocumentRendererGetCaretRect(r, &p, &caret) == XUI_OK); /* Inactive heading. */
    CHECK(xuiDocumentGetRevision(d) == revision);
    xuiDocumentRendererRelease(r); xuiDocumentSnapshotRelease(snapshot); xuiDocumentRelease(d);
    CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK); CHECK(xuiDocumentLoadMarkdown(d, "---\n\nbody\n", 10) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(d, &snapshot) == XUI_OK); CHECK(xuiDocumentRendererCreate(context, NULL, &r) == XUI_OK);
    CHECK(xuiDocumentRendererSetMode(r, XUI_DOC_LIVE_MARKDOWN) == XUI_OK); CHECK(xuiDocumentRendererSetSnapshot(r, snapshot, NULL) == XUI_OK);
    CHECK(xuiDocumentRendererGetStats(r, &stats) == XUI_OK && !stats.bLiveSourceFallback && stats.iSourceBlocks < stats.iBlocks);
    CHECK(xuiDocumentRendererGetActiveSourceRange(r, &start, &end) == XUI_OK && !start && end == 5);
    xuiDocumentRendererRelease(r); xuiDocumentSnapshotRelease(snapshot); xuiDocumentRelease(d);
    {
        const char* empty = "#\r\n\r\n~~~\r\n~~~\r\n\r\n>\r\n\r\n-\r\n\r\nTail\r\n";
        const char* note = "[^z]: Zed\n\nBody[^a] and [^z].\n\n[^a]: Alpha\n";
        const char* inputs[] = {empty, note}; unsigned j;
        for (j = 0; j < 2; j++) {
            CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK); CHECK(xuiDocumentLoadMarkdown(d, inputs[j], strlen(inputs[j])) == XUI_OK);
            CHECK(xuiDocumentAcquireSnapshot(d, &snapshot) == XUI_OK); CHECK(xuiDocumentRendererCreate(context, NULL, &r) == XUI_OK);
            CHECK(xuiDocumentRendererSetMode(r, XUI_DOC_LIVE_MARKDOWN) == XUI_OK); CHECK(xuiDocumentRendererSetSnapshot(r, snapshot, NULL) == XUI_OK);
            CHECK(xuiDocumentRendererGetStats(r, &stats) == XUI_OK && !stats.bLiveSourceFallback);
            p = position(d, 1, 0); p.iKind = XUI_DOC_POSITION_SOURCE;
            for (; p.iOffset <= strlen(inputs[j]); p.iOffset++) {
                CHECK(xuiDocumentRendererSetActivePosition(r, &p) == XUI_OK);
                CHECK(xuiDocumentRendererGetCaretRect(r, &p, &caret) == XUI_OK);
            }
            xuiDocumentRendererRelease(r); xuiDocumentSnapshotRelease(snapshot); xuiDocumentRelease(d);
        }
    }
}
static void live_pending_source(xui_context context)
{
    const char* source = "# Header\n\nmiddle\n\n**tail**\n";
    xui_doc_desc_t desc = {0}; xui_doc_txn_desc_t txn = {0};
    xui_doc_source_patch_t patch = {0};
    xui_document d; xui_document_snapshot snapshot, published;
    xui_document_prepare first = NULL, second = NULL, third = NULL, fourth = NULL;
    xui_document_renderer r; xui_doc_renderer_stats_t stats = {0};
    xui_doc_range_t head, tail; xui_doc_position_t active, candidate, hit;
    xui_doc_rect_t caret; uint64_t start, end, count, middle, tail_source;
    middle = (uint64_t)(strstr(source, "middle") - source);
    tail_source = (uint64_t)(strstr(source, "tail") - source);
    desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN;
    CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK &&
        xuiDocumentLoadMarkdown(d, source, strlen(source)) == XUI_OK &&
        xuiDocumentAcquireSnapshot(d, &snapshot) == XUI_OK);
    CHECK(xuiDocumentRendererCreate(context, NULL, &r) == XUI_OK &&
        xuiDocumentRendererSetMode(r, XUI_DOC_LIVE_MARKDOWN) == XUI_OK &&
        xuiDocumentRendererSetSnapshot(r, snapshot, NULL) == XUI_OK);
    active = position(d, 1, middle + 3); active.iKind = XUI_DOC_POSITION_SOURCE;
    CHECK(xuiDocumentRendererSetActivePosition(r, &active) == XUI_OK &&
        xuiDocumentRendererGetActiveSourceRange(r, &start, &end) == XUI_OK &&
        start <= middle && middle < end && end <= tail_source);
    txn.iSize = sizeof(txn); txn.iDomain = XUI_DOC_SOURCE;
    patch.iSize = sizeof(patch); patch.iStart = patch.iEnd = middle + 3;
    patch.sText = "XY"; patch.iTextBytes = 2;
    CHECK(xuiDocumentPrepareSource(d, &txn, &patch, 1, &first) == XUI_OK &&
        xuiDocumentRendererSetSourceInput(r, first) == XUI_OK);
    CHECK(xuiDocumentRendererGetActiveSourceRange(r, &start, &count) == XUI_OK && count == end + 2);
    stats.iSize = sizeof(stats);
    CHECK(xuiDocumentRendererGetStats(r, &stats) == XUI_OK &&
        stats.iSourceBlocks > 0 && stats.iSourceBlocks < stats.iBlocks && !stats.bLiveSourceFallback);
    CHECK(xuiDocumentPrepareSourcePosition(first, middle + 5, XUI_DOC_AFTER, &candidate) == XUI_OK &&
        xuiDocumentRendererLayout(r, 320, 0, 2000) == XUI_OK &&
        xuiDocumentRendererGetCaretRect(r, &candidate, &caret) == XUI_OK &&
        xuiDocumentRendererHitTest(r, caret.x + .1, caret.y + caret.height / 2, &hit) == XUI_OK &&
        hit.iKind == XUI_DOC_POSITION_SOURCE && hit.iInputGeneration == candidate.iInputGeneration &&
        hit.iOffset == candidate.iOffset);
    CHECK(xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC, "Header", 6, NULL, &head, 1, &count) == XUI_OK && count == 1);
    CHECK(xuiDocumentRendererGetCaretRect(r, &head.tAnchor, &caret) == XUI_OK &&
        xuiDocumentRendererHitTest(r, caret.x + .1, caret.y + caret.height / 2, &hit) == XUI_OK &&
        hit.iKind == XUI_DOC_POSITION_SOURCE && hit.iOffset == 2 &&
        hit.iInputGeneration == candidate.iInputGeneration);
    CHECK(xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC, "tail", 4, NULL, &tail, 1, &count) == XUI_OK && count == 1);
    CHECK(xuiDocumentRendererGetCaretRect(r, &tail.tAnchor, &caret) == XUI_OK &&
        xuiDocumentRendererHitTest(r, caret.x + .1, caret.y + caret.height / 2, &hit) == XUI_OK &&
        hit.iKind == XUI_DOC_POSITION_SOURCE && hit.iOffset == tail_source + 2 &&
        hit.iInputGeneration == candidate.iInputGeneration);
    patch.iStart = patch.iEnd = middle + 5; patch.sText = "Z"; patch.iTextBytes = 1;
    CHECK(xuiDocumentPrepareContinueSource(d, first, &patch, 1, &second) == XUI_OK &&
        xuiDocumentRendererSetSourceInput(r, second) == XUI_OK &&
        xuiDocumentRendererGetActiveSourceRange(r, &start, &count) == XUI_OK && count == end + 3);
    patch.iStart = patch.iEnd = tail_source + 3; patch.sText = "Q"; patch.iTextBytes = 1;
    CHECK(xuiDocumentPrepareContinueSource(d, second, &patch, 1, &third) == XUI_OK &&
        xuiDocumentRendererSetSourceInput(r, third) == XUI_OK &&
        xuiDocumentRendererGetStats(r, &stats) == XUI_OK &&
        stats.bLiveSourceFallback && stats.iSourceBlocks == stats.iBlocks);
    CHECK(xuiDocumentPrepareSourcePosition(third, tail_source + 4, XUI_DOC_AFTER, &candidate) == XUI_OK &&
        xuiDocumentRendererLayout(r, 320, 0, 2000) == XUI_OK &&
        xuiDocumentRendererGetCaretRect(r, &candidate, &caret) == XUI_OK &&
        xuiDocumentRendererHitTest(r, caret.x + .1, caret.y + caret.height / 2, &hit) == XUI_OK &&
        hit.iInputGeneration == candidate.iInputGeneration && hit.iOffset == candidate.iOffset);
    CHECK(xuiDocumentPrepareRun(third) == XUI_OK &&
        xuiDocumentPreparePublish(d, third, NULL) == XUI_OK &&
        xuiDocumentAcquireSnapshot(d, &published) == XUI_OK &&
        xuiDocumentRendererSetSnapshot(r, published, NULL) == XUI_OK &&
        xuiDocumentRendererGetStats(r, &stats) == XUI_OK && !stats.bLiveSourceFallback);
    {
        char actual[128]; uint64_t actual_bytes, old_tail;
        CHECK(xuiDocumentSnapshotCopySource(published, actual, sizeof(actual), &actual_bytes) == XUI_OK &&
            actual_bytes == strlen(actual));
        old_tail = (uint64_t)(strstr(actual, "tail") - actual);
        active = position(d, 1, middle + 4); active.iKind = XUI_DOC_POSITION_SOURCE;
        CHECK(xuiDocumentRendererSetActivePosition(r, &active) == XUI_OK);
        patch.iStart = middle + 3; patch.iEnd = middle + 6;
        patch.sText = ""; patch.iTextBytes = 0;
        CHECK(xuiDocumentPrepareSource(d, &txn, &patch, 1, &fourth) == XUI_OK &&
            xuiDocumentRendererSetSourceInput(r, fourth) == XUI_OK &&
            xuiDocumentRendererGetStats(r, &stats) == XUI_OK &&
            stats.iSourceBlocks < stats.iBlocks && !stats.bLiveSourceFallback);
        CHECK(xuiDocumentSnapshotFind(published, XUI_DOC_SEMANTIC, "tail", 4, NULL, &tail, 1, &count) == XUI_OK && count == 1);
        CHECK(xuiDocumentRendererLayout(r, 320, 0, 2000) == XUI_OK &&
            xuiDocumentRendererGetCaretRect(r, &tail.tAnchor, &caret) == XUI_OK &&
            xuiDocumentRendererHitTest(r, caret.x + .1, caret.y + caret.height / 2, &hit) == XUI_OK &&
            hit.iKind == XUI_DOC_POSITION_SOURCE && hit.iOffset == old_tail - 3);
        CHECK(xuiDocumentCancelPrepare(d) == XUI_OK &&
            xuiDocumentRendererSetSourceInput(r, NULL) == XUI_OK);
    }
    xuiDocumentSnapshotRelease(published); xuiDocumentRendererRelease(r);
    xuiDocumentPrepareRelease(fourth);
    xuiDocumentPrepareRelease(third); xuiDocumentPrepareRelease(second); xuiDocumentPrepareRelease(first);
    xuiDocumentSnapshotRelease(snapshot); xuiDocumentRelease(d);
    puts("LIVE pending source: hybrid active block, positive/negative inactive shifts, continued candidates, full-source fallback, publication and cancel passed");
}
static void live_pending_source_scale(xui_context context)
{
    const char* prefix = "# Header\n\n```\n";
    const char* suffix = "```\n\n**tail**\n";
    size_t head = strlen(prefix), tail = strlen(suffix), at, bytes = head + 1024 * 81 + tail;
    char* source = malloc(bytes + 1);
    xui_doc_desc_t desc = {0}; xui_doc_txn_desc_t txn = {0};
    xui_doc_source_patch_t patch = {0};
    xui_document d; xui_document_snapshot snapshot;
    xui_document_prepare previous = NULL, next = NULL;
    xui_document_renderer r; xui_doc_renderer_stats_t before = {0}, after = {0};
    xui_doc_position_t active, candidate; xui_doc_rect_t rect;
    uint64_t edit = head + 512 * 81 + 12; unsigned i;
    CHECK(source);
    memcpy(source, prefix, head);
    for (at = head; at < head + 1024 * 81; at += 81) {
        memset(source + at, 'a', 80); source[at + 80] = '\n';
    }
    memcpy(source + at, suffix, tail); source[bytes] = 0;
    desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN;
    CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK &&
        xuiDocumentLoadMarkdown(d, source, bytes) == XUI_OK &&
        xuiDocumentAcquireSnapshot(d, &snapshot) == XUI_OK);
    CHECK(xuiDocumentRendererCreate(context, NULL, &r) == XUI_OK &&
        xuiDocumentRendererSetMode(r, XUI_DOC_LIVE_MARKDOWN) == XUI_OK &&
        xuiDocumentRendererSetSnapshot(r, snapshot, NULL) == XUI_OK);
    active = position(d, 1, edit); active.iKind = XUI_DOC_POSITION_SOURCE;
    CHECK(xuiDocumentRendererSetActivePosition(r, &active) == XUI_OK);
    before.iSize = after.iSize = sizeof(before);
    CHECK(xuiDocumentRendererGetStats(r, &before) == XUI_OK);
    txn.iSize = sizeof(txn); txn.iDomain = XUI_DOC_SOURCE;
    patch.iSize = sizeof(patch); patch.iStart = edit; patch.iEnd = edit + 1;
    patch.iTextBytes = 1;
    for (i = 0; i < 16; i++) {
        patch.sText = i & 1 ? "b" : "c";
        CHECK((previous ? xuiDocumentPrepareContinueSource(d, previous, &patch, 1, &next) :
            xuiDocumentPrepareSource(d, &txn, &patch, 1, &next)) == XUI_OK &&
            xuiDocumentRendererSetSourceInput(r, next) == XUI_OK &&
            xuiDocumentPrepareSourcePosition(next, edit, XUI_DOC_AFTER, &candidate) == XUI_OK &&
            xuiDocumentRendererGetCaretRect(r, &candidate, &rect) == XUI_OK);
        xuiDocumentPrepareRelease(previous); previous = next; next = NULL;
    }
    CHECK(xuiDocumentRendererGetStats(r, &after) == XUI_OK &&
        !after.bLiveSourceFallback && after.iSourceBlocks > 1000 &&
        after.iSourceBytesScanned - before.iSourceBytesScanned < 16000 &&
        after.iSourceIncrementalUpdates - before.iSourceIncrementalUpdates == 16);
    {
        uint64_t rows = after.iSourceBlocks, scanned = after.iSourceBytesScanned;
        patch.iStart = patch.iEnd = edit + 1;
        patch.sText = "\r\n"; patch.iTextBytes = 2;
        CHECK(xuiDocumentPrepareContinueSource(d, previous, &patch, 1, &next) == XUI_OK &&
            xuiDocumentRendererSetSourceInput(r, next) == XUI_OK);
        xuiDocumentPrepareRelease(previous); previous = next; next = NULL;
        CHECK(xuiDocumentRendererGetStats(r, &after) == XUI_OK &&
            !after.bLiveSourceFallback && after.iSourceBlocks == rows + 1 &&
            after.iSourceBytesScanned - scanned < 2000);
        patch.iStart = edit + 1; patch.iEnd = edit + 3;
        patch.sText = ""; patch.iTextBytes = 0;
        CHECK(xuiDocumentPrepareContinueSource(d, previous, &patch, 1, &next) == XUI_OK &&
            xuiDocumentRendererSetSourceInput(r, next) == XUI_OK);
        xuiDocumentPrepareRelease(previous); previous = next; next = NULL;
        CHECK(xuiDocumentRendererGetStats(r, &after) == XUI_OK &&
            !after.bLiveSourceFallback && after.iSourceBlocks == rows);
    }
    printf("LIVE pending source %llu bytes, 16 local edits: %llu source bytes scanned, %llu reused rows.\n",
        (unsigned long long)bytes,
        (unsigned long long)(after.iSourceBytesScanned - before.iSourceBytesScanned),
        (unsigned long long)(after.iSourceReusedBlocks - before.iSourceReusedBlocks));
    CHECK(xuiDocumentCancelPrepare(d) == XUI_OK);
    xuiDocumentPrepareRelease(previous); xuiDocumentRendererRelease(r);
    xuiDocumentSnapshotRelease(snapshot); xuiDocumentRelease(d); free(source);
}
static void live_source_only_document(xui_context context)
{
    const char* source = "[foo]: /url\n";
    xui_doc_desc_t desc = {0}; xui_doc_txn_desc_t txn = {0};
    xui_doc_source_patch_t patch = {0};
    xui_document d; xui_document_snapshot snapshot;
    xui_document_prepare input; xui_document_renderer r;
    xui_doc_position_t p, hit; xui_doc_rect_t caret;
    desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN;
    CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK &&
        xuiDocumentLoadMarkdown(d, source, strlen(source)) == XUI_OK &&
        xuiDocumentAcquireSnapshot(d, &snapshot) == XUI_OK &&
        xuiDocumentRendererCreate(context, NULL, &r) == XUI_OK &&
        xuiDocumentRendererSetMode(r, XUI_DOC_LIVE_MARKDOWN) == XUI_OK &&
        xuiDocumentRendererSetSnapshot(r, snapshot, NULL) == XUI_OK);
    p = position(d, 1, 0); p.iKind = XUI_DOC_POSITION_SOURCE;
    CHECK(xuiDocumentRendererGetCaretRect(r, &p, &caret) == XUI_OK &&
        xuiDocumentRendererHitTest(r, caret.x + .1, caret.y + caret.height / 2, &hit) == XUI_OK &&
        hit.iKind == XUI_DOC_POSITION_SOURCE && hit.iOffset == 0);
    txn.iSize = sizeof(txn); txn.iDomain = XUI_DOC_SOURCE;
    patch.iSize = sizeof(patch); patch.iStart = patch.iEnd = 0;
    patch.sText = "x"; patch.iTextBytes = 1;
    CHECK(xuiDocumentPrepareSource(d, &txn, &patch, 1, &input) == XUI_OK &&
        xuiDocumentRendererSetSourceInput(r, input) == XUI_OK &&
        xuiDocumentPrepareSourcePosition(input, 0, XUI_DOC_AFTER, &p) == XUI_OK &&
        xuiDocumentRendererGetCaretRect(r, &p, &caret) == XUI_OK &&
        xuiDocumentRendererHitTest(r, caret.x + .1, caret.y + caret.height / 2, &hit) == XUI_OK &&
        hit.iKind == XUI_DOC_POSITION_SOURCE && hit.iOffset == 0);
    CHECK(xuiDocumentCancelPrepare(d) == XUI_OK);
    xuiDocumentPrepareRelease(input); xuiDocumentRendererRelease(r);
    xuiDocumentSnapshotRelease(snapshot); xuiDocumentRelease(d);
    puts("LIVE source-only document: caret and hit work before and during a candidate");
}
static void live_pending_source_differential(xui_context context)
{
    const char* source = "# Header\n\n```\nalpha\nbeta\r\ngamma\n```\n\n**tail**\n";
    xui_doc_desc_t desc = {0}; xui_doc_txn_desc_t txn = {0};
    xui_doc_source_patch_t patch = {0}, reference_patch = {0};
    xui_document d, reference; xui_document_snapshot snapshot, reference_snapshot;
    xui_document_prepare previous = NULL, next = NULL, reference_input = NULL;
    xui_document_renderer r, oracle = NULL;
    xui_doc_range_t header, tail, reference_header, reference_tail;
    xui_doc_position_t active, a, b, hit_a, hit_b;
    xui_doc_rect_t rect_a, rect_b, size_a, size_b;
    xui_doc_renderer_stats_t stats_a = {0}, stats_b = {0};
    uint64_t start, end, base_start, base_end, source_bytes, count;
    uint32_t seed = 0x711d2a5u; unsigned step, probe;
    int exact_a, exact_b; char buffer[4096];
    desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN;
    CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK &&
        xuiDocumentCreate(&desc, &reference) == XUI_OK &&
        xuiDocumentLoadMarkdown(d, source, strlen(source)) == XUI_OK &&
        xuiDocumentLoadMarkdown(reference, source, strlen(source)) == XUI_OK &&
        xuiDocumentAcquireSnapshot(d, &snapshot) == XUI_OK &&
        xuiDocumentAcquireSnapshot(reference, &reference_snapshot) == XUI_OK);
    CHECK(xuiDocumentRendererCreate(context, NULL, &r) == XUI_OK &&
        xuiDocumentRendererSetMode(r, XUI_DOC_LIVE_MARKDOWN) == XUI_OK &&
        xuiDocumentRendererSetSnapshot(r, snapshot, NULL) == XUI_OK);
    active = position(d, 1, (uint64_t)(strstr(source, "beta") - source));
    active.iKind = XUI_DOC_POSITION_SOURCE;
    CHECK(xuiDocumentRendererSetActivePosition(r, &active) == XUI_OK &&
        xuiDocumentRendererGetActiveSourceRange(r, &base_start, &base_end) == XUI_OK);
    txn.iSize = sizeof(txn); txn.iDomain = XUI_DOC_SOURCE;
    patch.iSize = reference_patch.iSize = sizeof(patch);
    stats_a.iSize = stats_b.iSize = sizeof(stats_a);
    CHECK(xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC, "Header", 6, NULL, &header, 1, &count) == XUI_OK && count == 1 &&
        xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC, "tail", 4, NULL, &tail, 1, &count) == XUI_OK && count == 1 &&
        xuiDocumentSnapshotFind(reference_snapshot, XUI_DOC_SEMANTIC, "Header", 6, NULL, &reference_header, 1, &count) == XUI_OK && count == 1 &&
        xuiDocumentSnapshotFind(reference_snapshot, XUI_DOC_SEMANTIC, "tail", 4, NULL, &reference_tail, 1, &count) == XUI_OK && count == 1);
    for (step = 0; step < 96; step++) {
        unsigned operation; uint64_t edit;
        seed = seed * 1664525u + 1013904223u;
        CHECK(xuiDocumentRendererGetActiveSourceRange(r, &start, &end) == XUI_OK && end > start + 12);
        edit = start + 5 + (seed % (uint32_t)(end - start - 10));
        operation = (seed >> 16) % 6;
        patch.iStart = edit; patch.iEnd = edit + (operation == 3 || operation == 4);
        patch.sText = operation == 0 ? "Z" : operation == 1 ? "\n" :
            operation == 2 ? "\r\n" : operation == 3 ? "Q" :
            operation == 4 ? "" : "\r";
        patch.iTextBytes = strlen(patch.sText);
        CHECK((previous ? xuiDocumentPrepareContinueSource(d, previous, &patch, 1, &next) :
            xuiDocumentPrepareSource(d, &txn, &patch, 1, &next)) == XUI_OK &&
            xuiDocumentRendererSetSourceInput(r, next) == XUI_OK);
        xuiDocumentPrepareRelease(previous); previous = next; next = NULL;
        CHECK(xuiDocumentPrepareCopySource(previous, buffer, sizeof(buffer), &source_bytes) == XUI_OK &&
            xuiDocumentRendererGetActiveSourceRange(r, &start, &end) == XUI_OK &&
            start == base_start && end <= source_bytes && source_bytes < sizeof(buffer));
        if (reference_input) {
            xuiDocumentRendererRelease(oracle); oracle = NULL;
            CHECK(xuiDocumentCancelPrepare(reference) == XUI_OK);
            xuiDocumentPrepareRelease(reference_input); reference_input = NULL;
        }
        reference_patch.iStart = base_start; reference_patch.iEnd = base_end;
        reference_patch.sText = buffer + base_start;
        reference_patch.iTextBytes = end - base_start;
        CHECK(xuiDocumentPrepareSource(reference, &txn, &reference_patch, 1, &reference_input) == XUI_OK &&
            xuiDocumentRendererCreate(context, NULL, &oracle) == XUI_OK &&
            xuiDocumentRendererSetMode(oracle, XUI_DOC_LIVE_MARKDOWN) == XUI_OK &&
            xuiDocumentRendererSetSnapshot(oracle, reference_snapshot, NULL) == XUI_OK);
        active = position(reference, 1, base_start + 5); active.iKind = XUI_DOC_POSITION_SOURCE;
        CHECK(xuiDocumentRendererSetActivePosition(oracle, &active) == XUI_OK &&
            xuiDocumentRendererSetSourceInput(oracle, reference_input) == XUI_OK &&
            xuiDocumentRendererLayout(r, 320, 0, 3000) == XUI_OK &&
            xuiDocumentRendererLayout(oracle, 320, 0, 3000) == XUI_OK &&
            xuiDocumentRendererGetStats(r, &stats_a) == XUI_OK &&
            xuiDocumentRendererGetStats(oracle, &stats_b) == XUI_OK &&
            !stats_a.bLiveSourceFallback && !stats_b.bLiveSourceFallback &&
            stats_a.iBlocks == stats_b.iBlocks && stats_a.iSourceBlocks == stats_b.iSourceBlocks &&
            xuiDocumentRendererGetSize(r, &size_a, &exact_a) == XUI_OK &&
            xuiDocumentRendererGetSize(oracle, &size_b, &exact_b) == XUI_OK &&
            fabs(size_a.width - size_b.width) < .001 && fabs(size_a.height - size_b.height) < .001 && exact_a == exact_b);
        CHECK(xuiDocumentRendererGetCaretRect(r, &header.tAnchor, &rect_a) == XUI_OK &&
            xuiDocumentRendererGetCaretRect(oracle, &reference_header.tAnchor, &rect_b) == XUI_OK &&
            fabs(rect_a.x - rect_b.x) < .001 && fabs(rect_a.y - rect_b.y) < .001 &&
            xuiDocumentRendererGetCaretRect(r, &tail.tAnchor, &rect_a) == XUI_OK &&
            xuiDocumentRendererGetCaretRect(oracle, &reference_tail.tAnchor, &rect_b) == XUI_OK &&
            fabs(rect_a.x - rect_b.x) < .001 && fabs(rect_a.y - rect_b.y) < .001);
        for (probe = 0; probe < 4; probe++) {
            uint64_t offset = probe == 0 ? start : probe == 1 ? edit : probe == 2 ? end - 1 : end;
            CHECK(xuiDocumentPrepareSourcePosition(previous, offset, XUI_DOC_AFTER, &a) == XUI_OK &&
                xuiDocumentPrepareSourcePosition(reference_input, offset, XUI_DOC_AFTER, &b) == XUI_OK &&
                xuiDocumentRendererGetCaretRect(r, &a, &rect_a) == XUI_OK &&
                xuiDocumentRendererGetCaretRect(oracle, &b, &rect_b) == XUI_OK &&
                fabs(rect_a.x - rect_b.x) < .001 && fabs(rect_a.y - rect_b.y) < .001 &&
                fabs(rect_a.height - rect_b.height) < .001 &&
                xuiDocumentRendererHitTest(r, rect_a.x + .1, rect_a.y + rect_a.height / 2, &hit_a) == XUI_OK &&
                xuiDocumentRendererHitTest(oracle, rect_b.x + .1, rect_b.y + rect_b.height / 2, &hit_b) == XUI_OK &&
                hit_a.iKind == hit_b.iKind && hit_a.iOffset == hit_b.iOffset);
        }
    }
    xuiDocumentRendererRelease(oracle); xuiDocumentPrepareRelease(reference_input);
    CHECK(xuiDocumentCancelPrepare(d) == XUI_OK && xuiDocumentCancelPrepare(reference) == XUI_OK);
    xuiDocumentPrepareRelease(previous); xuiDocumentRendererRelease(r);
    xuiDocumentSnapshotRelease(snapshot); xuiDocumentSnapshotRelease(reference_snapshot);
    xuiDocumentRelease(d); xuiDocumentRelease(reference);
    puts("LIVE pending source: 96 mixed edit candidates match rebuilt hybrid layout, semantic anchors, carets and hits");
}
static void live_corpus(xui_context context, const char* path)
{
    FILE* file = fopen(path, "rb"); long length; char* json; xvalue* cases; size_t i, count, fallbacks = 0;
    uint64_t boundaries = 0;
    CHECK(file); CHECK(fseek(file, 0, SEEK_END) == 0); length = ftell(file); CHECK(length >= 0);
    rewind(file); json = malloc((size_t)length + 1); CHECK(json);
    CHECK(fread(json, 1, (size_t)length, file) == (size_t)length); fclose(file); json[length] = 0;
    cases = xrtJsonParse((xstrview){json, (size_t)length}); free(json); CHECK(cases && xrtValueIs(cases, XVALUE_ARRAY)); count = xrtValueCount(cases);
    for (i = 0; i < count; i++) {
        xstrview md; xui_doc_desc_t desc = {0}; xui_document d; xui_document_snapshot s; xui_document_renderer r;
        xui_doc_renderer_stats_t stats = {0}; xui_doc_position_t p; xui_doc_rect_t caret; int result;
        CHECK(xrtValueGetString(xrtValueObjectGet(xrtValueArrayGet(cases, i), (xstrview){"markdown", 8}), &md));
        desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN; desc.iMarkdownDialect = XUI_MD_COMMONMARK;
        CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK); CHECK(xuiDocumentLoadMarkdown(d, md.Data, md.Size) == XUI_OK);
        CHECK(xuiDocumentAcquireSnapshot(d, &s) == XUI_OK); CHECK(xuiDocumentRendererCreate(context, NULL, &r) == XUI_OK);
        CHECK(xuiDocumentRendererSetMode(r, XUI_DOC_LIVE_MARKDOWN) == XUI_OK); CHECK(xuiDocumentRendererSetSnapshot(r, s, NULL) == XUI_OK);
        stats.iSize = sizeof(stats); CHECK(xuiDocumentRendererGetStats(r, &stats) == XUI_OK); fallbacks += !!stats.bLiveSourceFallback;
        p = position(d, 1, 0); p.iKind = XUI_DOC_POSITION_SOURCE;
        for (; p.iOffset <= md.Size; p.iOffset++) {
            if (p.iOffset < md.Size && ((unsigned char)md.Data[p.iOffset] & 0xc0) == 0x80) continue;
            CHECK(xuiDocumentRendererSetActivePosition(r, &p) == XUI_OK);
            result = xuiDocumentRendererGetCaretRect(r, &p, &caret);
            if (result != XUI_OK) fprintf(stderr, "Live corpus case %llu, byte %llu returned %d\n", (unsigned long long)i + 1, (unsigned long long)p.iOffset, result);
            CHECK(result == XUI_OK); boundaries++;
        }
        xuiDocumentRendererRelease(r); xuiDocumentSnapshotRelease(s); xuiDocumentRelease(d);
    }
    xrtValueRelease(cases);
    CHECK(!fallbacks);
    printf("Live Markdown: %llu corpus cases, %llu UTF-8 caret boundaries passed; %llu cases use the declared source fallback (not HTML/display conformance).\n",
        (unsigned long long)count, (unsigned long long)boundaries, (unsigned long long)fallbacks);
}
static int outer_wheel(xui_widget widget, const xui_event_t* event, void* user)
{
    (void)widget;
    if (event->iType == XUI_EVENT_POINTER_WHEEL && event->iPhase == XUI_EVENT_PHASE_BUBBLE) (*(unsigned*)user)++;
    return XUI_OK;
}
static void hit_test_row_precedence(xui_context context)
{
    xui_document document; xui_document_transaction txn; xui_document_snapshot snapshot;
    xui_document_renderer renderer; xui_doc_renderer_desc_t desc = {0};
    xui_doc_position_t first, hit; xui_doc_rect_t caret, size;
    uint64_t paragraph, short_text, long_text; int exact;
    CHECK(xuiDocumentCreate(NULL, &document) == XUI_OK &&
        xuiDocumentBeginTransaction(document, NULL, &txn) == XUI_OK);
    paragraph = add(txn, 1, XUI_DOC_PARAGRAPH, NULL);
    short_text = add(txn, paragraph, XUI_DOC_TEXT, "a");
    add(txn, paragraph, XUI_DOC_HARD_BREAK, NULL);
    long_text = add(txn, paragraph, XUI_DOC_TEXT, "long long long long long");
    CHECK(xuiDocumentTxnCommit(txn, NULL) == XUI_OK); xuiDocumentTxnRelease(txn);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
    desc.iSize = sizeof(desc);
    CHECK(xuiDocumentRendererCreate(context, &desc, &renderer) == XUI_OK &&
        xuiDocumentRendererSetSnapshot(renderer, snapshot, NULL) == XUI_OK);
    xuiDocumentSnapshotRelease(snapshot);
    CHECK(xuiDocumentRendererLayout(renderer, 240, 0, 200) == XUI_OK);
    first = position(document, short_text, 0);
    CHECK(xuiDocumentRendererGetCaretRect(renderer, &first, &caret) == XUI_OK);
    CHECK(xuiDocumentRendererHitTest(renderer, 230, caret.y + caret.height / 2, &hit) == XUI_OK &&
        hit.iNodeId == short_text && hit.iOffset == 1);
    CHECK(xuiDocumentRendererGetSize(renderer, &size, &exact) == XUI_OK && exact);
    CHECK(xuiDocumentRendererHitTest(renderer, 230, size.height + 50, &hit) == XUI_OK &&
        hit.iNodeId == long_text && hit.iOffset == strlen("long long long long long"));
    CHECK(xuiDocumentRendererHitTest(renderer, 0, size.height + 50, &hit) == XUI_OK &&
        hit.iNodeId == long_text && hit.iOffset == 0);
    xuiDocumentRendererRelease(renderer); xuiDocumentRelease(document);
    puts("Document hit-test row precedence: short-line end and below-document endpoints passed");
}
static void hit_test_cell_precedence(xui_context context)
{
    xui_document document; xui_document_transaction txn; xui_document_snapshot snapshot;
    xui_document_renderer renderer; xui_doc_renderer_desc_t desc = {0};
    xui_doc_position_t bottom, hit; xui_doc_rect_t caret;
    uint64_t table, row, cell, paragraph, left_text, bottom_text;
    CHECK(xuiDocumentCreate(NULL, &document) == XUI_OK &&
        xuiDocumentBeginTransaction(document, NULL, &txn) == XUI_OK);
    table = add(txn, 1, XUI_DOC_TABLE, NULL); row = add(txn, table, XUI_DOC_ROW, NULL);
    cell = add(txn, row, XUI_DOC_CELL, NULL); paragraph = add(txn, cell, XUI_DOC_PARAGRAPH, NULL);
    left_text = add(txn, paragraph, XUI_DOC_TEXT, "a");
    cell = add(txn, row, XUI_DOC_CELL, NULL); paragraph = add(txn, cell, XUI_DOC_PARAGRAPH, NULL);
    add(txn, paragraph, XUI_DOC_TEXT, "top"); add(txn, paragraph, XUI_DOC_HARD_BREAK, NULL);
    bottom_text = add(txn, paragraph, XUI_DOC_TEXT, "bottom");
    CHECK(xuiDocumentTxnCommit(txn, NULL) == XUI_OK); xuiDocumentTxnRelease(txn);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
    desc.iSize = sizeof(desc);
    CHECK(xuiDocumentRendererCreate(context, &desc, &renderer) == XUI_OK &&
        xuiDocumentRendererSetSnapshot(renderer, snapshot, NULL) == XUI_OK);
    xuiDocumentSnapshotRelease(snapshot);
    CHECK(xuiDocumentRendererLayout(renderer, 240, 0, 200) == XUI_OK);
    bottom = position(document, bottom_text, 0);
    CHECK(xuiDocumentRendererGetCaretRect(renderer, &bottom, &caret) == XUI_OK);
    CHECK(xuiDocumentRendererHitTest(renderer, 110, caret.y + caret.height / 2, &hit) == XUI_OK &&
        hit.iNodeId == left_text && hit.iOffset == 1);
    xuiDocumentRendererRelease(renderer); xuiDocumentRelease(document);
    puts("Document hit-test cell precedence: click inside a short cell stays in that cell");
}
static void embedded_view(xui_context context)
{
    xui_document d; xui_document_transaction t; xui_doc_view_desc_t desc = {0};
    xui_widget host, view; xui_event_t event = {0}; xui_vec2_t before, after; unsigned wheels = 0; uint64_t paragraph;
    CHECK(xuiDocumentCreate(NULL, &d) == XUI_OK); CHECK(xuiDocumentBeginTransaction(d, NULL, &t) == XUI_OK);
    paragraph = add(t, 1, XUI_DOC_PARAGRAPH, NULL); add(t, paragraph, XUI_DOC_TEXT, "Embedded text");
    CHECK(xuiDocumentTxnCommit(t, NULL) == XUI_OK); xuiDocumentTxnRelease(t);
    CHECK(xuiWidgetCreate(context, &host) == XUI_OK); CHECK(xuiSetRootWidget(context, host) == XUI_OK);
    CHECK(xuiWidgetSetRect(host, (xui_rect_t){0, 0, 320, 180}) == XUI_OK);
    CHECK(xuiWidgetSetEventCallback(host, outer_wheel, &wheels) == XUI_OK);
    desc.iSize = sizeof(desc); desc.pDocument = d; desc.bAutoHeight = 1;
    CHECK(xuiDocumentViewCreate(context, &desc, &view) == XUI_OK); CHECK(xuiWidgetAddChild(host, view) == XUI_OK);
    CHECK(xuiWidgetSetRect(view, (xui_rect_t){0, 0, 300, 150}) == XUI_OK);
    CHECK(xuiWidgetMeasureContent(view, (xui_vec2_t){300, 1000}, &before) == XUI_OK);
    event.iSize = sizeof(event); event.iType = XUI_EVENT_POINTER_WHEEL; event.pTarget = view; event.fWheelY = -1;
    CHECK(xuiDispatchEvent(context, &event) == XUI_OK && wheels == 1);
    CHECK(xuiDocumentBeginTransaction(d, NULL, &t) == XUI_OK);
    paragraph = add(t, 1, XUI_DOC_PARAGRAPH, NULL); add(t, paragraph, XUI_DOC_TEXT, "Appended by the owner while this view remains read-only.");
    CHECK(xuiDocumentTxnCommit(t, NULL) == XUI_OK); xuiDocumentTxnRelease(t);
    CHECK(xuiWidgetMeasureContent(view, (xui_vec2_t){300, 1000}, &after) == XUI_OK && after.fY > before.fY);
    CHECK(xuiDocumentUndo(d, NULL) == XUI_OK);
    CHECK(xuiWidgetMeasureContent(view, (xui_vec2_t){300, 1000}, &after) == XUI_OK && after.fY == before.fY);
    xuiWidgetDestroy(host); xuiDocumentRelease(d);
}
static void range_rect_geometry(xui_context context)
{
    xui_document document; xui_document_transaction txn;
    xui_document_snapshot snapshot; xui_document_renderer renderer;
    xui_doc_range_t range; xui_doc_rect_t rects[32], first_rect[1], object_rect;
    xui_doc_rect_t caret, object_before, object_after;
    xui_doc_renderer_stats_t before = {0}, after = {0};
    uint64_t paragraph, text_a, text_b, image, diagram, count, queried;
    unsigned i;
    CHECK(xuiDocumentCreate(NULL, &document) == XUI_OK &&
        xuiDocumentBeginTransaction(document, NULL, &txn) == XUI_OK);
    for (i = 0; i < 64; i++) {
        uint64_t filler = add(txn, 1, XUI_DOC_PARAGRAPH, NULL);
        add(txn, filler, XUI_DOC_TEXT, "filler");
    }
    paragraph = add(txn, 1, XUI_DOC_PARAGRAPH, NULL);
    text_a = add(txn, paragraph, XUI_DOC_TEXT,
        "first styled group has several words ");
    text_b = add(txn, paragraph, XUI_DOC_TEXT,
        "and second group also wraps around");
    image = add(txn, paragraph, XUI_DOC_IMAGE, "");
    diagram = add(txn, 1, XUI_DOC_DIAGRAM, "graph TD;A-->B");
    CHECK(xuiDocumentTxnCommit(txn, NULL) == XUI_OK);
    xuiDocumentTxnRelease(txn);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK &&
        xuiDocumentRendererCreate(context, NULL, &renderer) == XUI_OK &&
        xuiDocumentRendererSetSnapshot(renderer, snapshot, NULL) == XUI_OK);
    xuiDocumentSnapshotRelease(snapshot);
    CHECK(xuiDocumentRendererLayout(renderer, 96, 0, 28) == XUI_OK);
    before.iSize = after.iSize = sizeof(before);
    CHECK(xuiDocumentRendererGetStats(renderer, &before) == XUI_OK);
    range.tAnchor = position(document, text_a, 2);
    range.tCaret = position(document, text_b, 15);
    CHECK(xuiDocumentRendererGetRangeRects(renderer, &range,
        NULL, 0, &count) == XUI_OK && count > 1 && count <= 32);
    CHECK(xuiDocumentRendererGetStats(renderer, &after) == XUI_OK &&
        after.iMeasuredBlocks == before.iMeasuredBlocks + 1);
    CHECK(xuiDocumentRendererGetRangeRects(renderer, &range,
        first_rect, 1, &queried) == XUI_OK && queried == count);
    CHECK(xuiDocumentRendererGetRangeRects(renderer, &range,
        rects, 32, &queried) == XUI_OK && queried == count);
    CHECK(rects[0].x == first_rect[0].x && rects[0].y == first_rect[0].y);
    CHECK(xuiDocumentRendererGetCaretRect(renderer, &range.tAnchor,
        &caret) == XUI_OK && fabs(caret.x - rects[0].x) < .01 &&
        fabs(caret.y - rects[0].y) < .01);
    for (i = 0; i < count; i++) {
        CHECK(rects[i].width > 0 && rects[i].height > 0);
        if (i) CHECK(rects[i].y >= rects[i - 1].y);
    }
    { xui_doc_position_t swap = range.tAnchor;
      range.tAnchor = range.tCaret; range.tCaret = swap; }
    CHECK(xuiDocumentRendererGetRangeRects(renderer, &range,
        first_rect, 1, &queried) == XUI_OK && queried == count &&
        first_rect[0].x == rects[0].x && first_rect[0].y == rects[0].y);
    range.tAnchor = range.tCaret = position(document, paragraph, 2);
    range.tAnchor.iKind = range.tCaret.iKind = XUI_DOC_POSITION_GAP;
    range.tCaret.iOffset = 3;
    CHECK(xuiDocumentRendererGetRangeRects(renderer, &range,
        rects, 32, &count) == XUI_OK && count == 1);
    CHECK(xuiDocumentRendererGetNodeRect(renderer, image, &object_rect) == XUI_OK &&
        fabs(rects[0].x - object_rect.x) < .01 &&
        fabs(rects[0].y - object_rect.y) < .01 &&
        fabs(rects[0].width - object_rect.width) < .01);
    range.tCaret = range.tAnchor;
    CHECK(xuiDocumentRendererGetCaretRect(renderer, &range.tAnchor,
        &object_before) == XUI_OK);
    range.tCaret.iOffset = 3;
    CHECK(xuiDocumentRendererGetCaretRect(renderer, &range.tCaret,
        &object_after) == XUI_OK &&
        fabs(object_before.x - object_rect.x) < .01 &&
        fabs(object_after.x - object_rect.x - object_rect.width) < .01 &&
        fabs(object_before.y - object_after.y) < .01);
    range.tAnchor = range.tCaret = position(document, 1, 65);
    range.tAnchor.iKind = range.tCaret.iKind = XUI_DOC_POSITION_GAP;
    range.tCaret.iOffset = 66;
    CHECK(xuiDocumentRendererGetStats(renderer, &before) == XUI_OK &&
        xuiDocumentRendererGetRangeRects(renderer, &range,
            rects, 32, &count) == XUI_OK && count == 1);
    CHECK(xuiDocumentRendererGetStats(renderer, &after) == XUI_OK &&
        after.iMeasuredBlocks == before.iMeasuredBlocks + 1);
    CHECK(xuiDocumentRendererGetNodeRect(renderer, diagram, &object_rect) == XUI_OK &&
        rects[0].y >= object_rect.y);
    range.tCaret = range.tAnchor;
    CHECK(xuiDocumentRendererGetRangeRects(renderer, &range,
        NULL, 0, &count) == XUI_OK && !count);
    range.tCaret.iOffset = 3;
    range.tAnchor.iRevision--;
    CHECK(xuiDocumentRendererGetRangeRects(renderer, &range,
        NULL, 0, &count) == XUI_DOC_ERROR_STALE);
    CHECK(xuiDocumentRendererGetRangeRects(renderer, &range,
        NULL, 1, &count) == XUI_ERROR_INVALID_ARGUMENT);
    xuiDocumentRendererRelease(renderer); xuiDocumentRelease(document);

    {
        xui_doc_desc_t md = {0};
        md.iSize = sizeof(md); md.iProfile = XUI_DOCUMENT_MARKDOWN;
        CHECK(xuiDocumentCreate(&md, &document) == XUI_OK &&
            xuiDocumentLoadMarkdown(document, "alpha\nbeta\n", 11) == XUI_OK);
    }
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK &&
        xuiDocumentRendererCreate(context, NULL, &renderer) == XUI_OK &&
        xuiDocumentRendererSetMode(renderer, XUI_DOC_SOURCE_TEXT) == XUI_OK &&
        xuiDocumentRendererSetSnapshot(renderer, snapshot, NULL) == XUI_OK);
    xuiDocumentSnapshotRelease(snapshot);
    CHECK(xuiDocumentRendererLayout(renderer, 100, 0, 18) == XUI_OK);
    range.tAnchor = range.tCaret = position(document, 1, 1);
    range.tAnchor.iKind = range.tCaret.iKind = XUI_DOC_POSITION_SOURCE;
    range.tCaret.iOffset = 8;
    CHECK(xuiDocumentRendererGetRangeRects(renderer, &range,
        rects, 32, &count) == XUI_OK && count >= 2 &&
        rects[1].y > rects[0].y);
    xuiDocumentRendererRelease(renderer); xuiDocumentRelease(document);
    puts("Renderer range geometry: wrapped runs, empty-alt object, offscreen lazy block, reverse/truncated query and SOURCE rows passed");
}
static void paragraph_projection_run_boundary(xui_context context)
{
    const char* full_text = "alpha beta gamma delta epsilon";
    const uint64_t split = 8, length = 30;
    xui_document documents[2]; xui_document_renderer renderers[2];
    xui_doc_node_id runs[2][2], images[2];
    xui_doc_rect_t sizes[2], image_rects[2], carets[2];
    uint64_t i; int version, exact;
    for (version = 0; version < 2; version++) {
        xui_document_transaction txn; xui_document_snapshot snapshot;
        xui_doc_node_id paragraph;
        xui_doc_node_desc_t desc = {0};
        CHECK(xuiDocumentCreate(NULL, &documents[version]) == XUI_OK &&
            xuiDocumentBeginTransaction(documents[version], NULL, &txn) == XUI_OK);
        paragraph = add(txn, 1, XUI_DOC_PARAGRAPH, NULL);
        desc.iSize = sizeof(desc); desc.iKind = XUI_DOC_TEXT;
        desc.sText = full_text; desc.iTextBytes = version ? split : length;
        CHECK(xuiDocumentTxnInsertNode(txn, paragraph, XUI_DOCUMENT_APPEND,
            &desc, &runs[version][0]) == XUI_OK);
        if (version) {
            desc.sText = full_text + split; desc.iTextBytes = length - split;
            CHECK(xuiDocumentTxnInsertNode(txn, paragraph, XUI_DOCUMENT_APPEND,
                &desc, &runs[version][1]) == XUI_OK);
        }
        images[version] = add(txn, paragraph, XUI_DOC_IMAGE, "");
        CHECK(xuiDocumentTxnCommit(txn, NULL) == XUI_OK);
        xuiDocumentTxnRelease(txn);
        CHECK(xuiDocumentAcquireSnapshot(documents[version], &snapshot) == XUI_OK &&
            xuiDocumentRendererCreate(context, NULL, &renderers[version]) == XUI_OK &&
            xuiDocumentRendererSetSnapshot(renderers[version], snapshot, NULL) == XUI_OK);
        xuiDocumentSnapshotRelease(snapshot);
        CHECK(xuiDocumentRendererLayout(renderers[version], 82, 0, 1000) == XUI_OK &&
            xuiDocumentRendererGetSize(renderers[version], &sizes[version], &exact) == XUI_OK && exact &&
            xuiDocumentRendererGetNodeRect(renderers[version], images[version],
                &image_rects[version]) == XUI_OK);
    }
    CHECK(fabs(sizes[0].height - sizes[1].height) < .01 &&
        fabs(image_rects[0].x - image_rects[1].x) < .01 &&
        fabs(image_rects[0].y - image_rects[1].y) < .01);
    for (i = 0; i <= length; i++) {
        xui_doc_position_t plain = position(documents[0], runs[0][0], i);
        xui_doc_position_t styled = position(documents[1],
            i <= split ? runs[1][0] : runs[1][1],
            i <= split ? i : i - split);
        CHECK(xuiDocumentRendererGetCaretRect(renderers[0], &plain, &carets[0]) == XUI_OK &&
            xuiDocumentRendererGetCaretRect(renderers[1], &styled, &carets[1]) == XUI_OK &&
            fabs(carets[0].x - carets[1].x) < .01 &&
            fabs(carets[0].y - carets[1].y) < .01);
    }
    for (version = 0; version < 2; version++) {
        xuiDocumentRendererRelease(renderers[version]);
        xuiDocumentRelease(documents[version]);
    }
    puts("Paragraph projection: split text run and following object match unsplit layout at every caret");
}
static void paragraph_projection_crlf(xui_context context)
{
    xui_doc_rect_t sizes[2], carets[2];
    int version;
    for (version = 0; version < 2; version++) {
        xui_document document; xui_document_transaction txn;
        xui_document_snapshot snapshot; xui_document_renderer renderer;
        xui_doc_node_id paragraph, last_text;
        xui_doc_position_t after_break; int exact;
        CHECK(xuiDocumentCreate(NULL, &document) == XUI_OK &&
            xuiDocumentBeginTransaction(document, NULL, &txn) == XUI_OK);
        paragraph = add(txn, 1, XUI_DOC_PARAGRAPH, NULL);
        if (version) {
            add(txn, paragraph, XUI_DOC_TEXT, "a\r");
            last_text = add(txn, paragraph, XUI_DOC_TEXT, "\nb");
        } else last_text = add(txn, paragraph, XUI_DOC_TEXT, "a\nb");
        CHECK(xuiDocumentTxnCommit(txn, NULL) == XUI_OK);
        xuiDocumentTxnRelease(txn);
        after_break = position(document, last_text, version ? 1 : 2);
        CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK &&
            xuiDocumentRendererCreate(context, NULL, &renderer) == XUI_OK &&
            xuiDocumentRendererSetSnapshot(renderer, snapshot, NULL) == XUI_OK);
        xuiDocumentSnapshotRelease(snapshot);
        CHECK(xuiDocumentRendererLayout(renderer, 200, 0, 200) == XUI_OK &&
            xuiDocumentRendererGetSize(renderer, &sizes[version], &exact) == XUI_OK && exact &&
            xuiDocumentRendererGetCaretRect(renderer, &after_break, &carets[version]) == XUI_OK);
        xuiDocumentRendererRelease(renderer); xuiDocumentRelease(document);
    }
    CHECK(fabs(sizes[0].height - sizes[1].height) < .01 &&
        fabs(carets[0].y - carets[1].y) < .01);
    for (version = 0; version < 2; version++) {
        xui_document document; xui_document_transaction txn;
        xui_document_snapshot snapshot; xui_document_renderer renderer;
        xui_doc_node_id code; xui_doc_position_t after_break; int exact;
        CHECK(xuiDocumentCreate(NULL, &document) == XUI_OK &&
            xuiDocumentBeginTransaction(document, NULL, &txn) == XUI_OK);
        code = add(txn, 1, XUI_DOC_CODE_BLOCK, version ? "a\r\nb" : "a\nb");
        CHECK(xuiDocumentTxnCommit(txn, NULL) == XUI_OK);
        xuiDocumentTxnRelease(txn);
        after_break = position(document, code, version ? 3 : 2);
        CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK &&
            xuiDocumentRendererCreate(context, NULL, &renderer) == XUI_OK &&
            xuiDocumentRendererSetSnapshot(renderer, snapshot, NULL) == XUI_OK);
        xuiDocumentSnapshotRelease(snapshot);
        CHECK(xuiDocumentRendererLayout(renderer, 200, 0, 200) == XUI_OK &&
            xuiDocumentRendererGetSize(renderer, &sizes[version], &exact) == XUI_OK && exact &&
            xuiDocumentRendererGetCaretRect(renderer, &after_break, &carets[version]) == XUI_OK);
        xuiDocumentRendererRelease(renderer); xuiDocumentRelease(document);
    }
    CHECK(fabs(sizes[0].height - sizes[1].height) < .01 &&
        fabs(carets[0].y - carets[1].y) < .01);
    puts("Paragraph projection: CRLF across runs and in code creates one hard line break");
}
int main(int argc, char** argv)
{
    xui_test_proxy_state_t proxy; xui_context context; xui_font font; xui_surface target; xui_draw_context draw;
    xui_document document; xui_document_transaction t; xui_document_snapshot s; xui_document_change_set changes;
    xui_document_renderer wide, narrow; xui_doc_rect_t wsize, nsize, caret, second_line; xui_doc_position_t p, hit;
    uint64_t para, body, table, row, cell, text[4], i; int exact;
    xui_doc_renderer_stats_t stats = {0}; xui_doc_renderer_desc_t desc = {0};
    xuiTestProxyInit(&proxy); CHECK(xuiCreate(&context) == XUI_OK); CHECK(xuiSetProxy(context, &proxy.tProxy) == XUI_OK);
    CHECK(proxy.tProxy.fontLoadFile(&proxy.tProxy, &font, "test.ttf", 14, 0) == XUI_OK);
    CHECK(xuiSetDefaultFont(context, font) == XUI_OK); CHECK(xuiTestSurfaceCreate(&proxy, &target, 640, 800, XUI_SURFACE_USAGE_TARGET) == XUI_OK);
    paragraph_paint_boundary_kerning(context, target, &proxy);
    document_cross_node_graphemes(context, target, &proxy);
    document_decorated_shaping(context, target, &proxy);
    document_mixed_font_shaping(target, &proxy);
    document_vertical_shaping(target, &proxy);
    document_object_joint_shaping(target, &proxy);
    document_break_font_context(target, &proxy);
    document_unicode_mandatory_breaks(target, &proxy);
    document_break_boundary_carets(&proxy);
    document_soft_wrap_affinity(&proxy);
    document_format_controls(&proxy);
    document_joint_scale(&proxy);
    document_joint_convergence(&proxy);
    document_ligature_carets(&proxy);
    document_draw_only(&proxy);
    document_grapheme_fonts(target, &proxy);
    document_retained_context(&proxy);
    document_shy_context(&proxy);
    document_language_lifetime(&proxy);
    text_input_capability_cases(&proxy);
    document_input_capability_cases(&proxy);
    document_retained_paint_contract(&proxy);
    document_shared_span_contract(&proxy);
    document_shared_shy_convergence(&proxy);
    document_line_paint_contract(&proxy);
#ifdef _WIN32
    native_cross_node_emoji();
    native_mixed_font_geometry();
    native_vertical_font_geometry();
#endif
    range_rect_geometry(context);
    paragraph_projection_run_boundary(context);
    paragraph_projection_crlf(context);
    block_style_layout(context);
    extended_inline_layout(context, target, &proxy);
    admonition_title_layout(context, target, &proxy);
    link_inline_render(context, target, &proxy);
    image_resource_load(context, &proxy);
    image_resource_async(context);
    image_resource_render(context, target, &proxy);
    image_resource_scroll_anchor(context, &proxy);
    image_resource_unmeasured_anchor(context, &proxy);
    object_provider_scroll_anchor(context);
    object_provider_same_block_anchor(context);
    view_zoom_scroll_anchor(context);
    view_style_scroll_anchor(context, &proxy);
    view_same_paragraph_style_anchor(context);
    renderer_inline_style_split_cache(context);
    view_zoom_source_anchor(context);
    dpi_layout_and_view_anchor(context);
    default_font_theme_anchor(context);
    unrelated_resource_preserves_layout(context);
    rich_text_style_render(context, target, &proxy);
    renderer_paint_only_color_refresh(context, target, &proxy);
    renderer_paint_only_cell_refresh(context, target, &proxy);
    html_block_style_render(context, target, &proxy);
    explicit_zero_color_render(context, target, &proxy);
    current_color_render(context, target, &proxy);
    html_table_cell_style_render(context, target, &proxy);
    table_cell_alignment_render(context);
    paragraph_background_prefix(context, target, &proxy);
    link_view_activation(context);
    live_markdown(context);
    live_pending_source(context);
    live_pending_source_scale(context);
    live_source_only_document(context);
    live_pending_source_differential(context);
    source_projection_cases();
    if (argc > 1) live_corpus(context, argv[1]);
    hit_test_row_precedence(context);
    hit_test_cell_precedence(context);
    embedded_view(context);
    CHECK(xuiDocumentCreate(NULL, &document) == XUI_OK); CHECK(xuiDocumentBeginTransaction(document, NULL, &t) == XUI_OK);
    para = add(t, 1, XUI_DOC_PARAGRAPH, NULL); body = add(t, para, XUI_DOC_TEXT, "Body before a table.");
    table = add(t, 1, XUI_DOC_TABLE, NULL);
    for (i = 0; i < 2; i++) {
        unsigned j; row = add(t, table, XUI_DOC_ROW, NULL);
        for (j = 0; j < 2; j++) {
            cell = add(t, row, XUI_DOC_CELL, NULL); para = add(t, cell, XUI_DOC_PARAGRAPH, NULL);
            text[i * 2 + j] = add(t, para, XUI_DOC_TEXT, "A cell with enough text to wrap over multiple lines.");
        }
    }
    CHECK(xuiDocumentTxnCommit(t, NULL) == XUI_OK); xuiDocumentTxnRelease(t); CHECK(xuiDocumentAcquireSnapshot(document, &s) == XUI_OK);
    desc.iSize = sizeof(desc); desc.fZoom = 1.25f;
    CHECK(xuiDocumentRendererCreate(context, &desc, &wide) == XUI_OK); CHECK(xuiDocumentRendererCreate(context, &desc, &narrow) == XUI_OK);
    CHECK(xuiDocumentRendererSetSnapshot(wide, s, NULL) == XUI_OK); CHECK(xuiDocumentRendererSetSnapshot(narrow, s, NULL) == XUI_OK);
    xuiDocumentSnapshotRelease(s);
    CHECK(xuiDocumentRendererLayout(wide, 600, 0, 800) == XUI_OK); CHECK(xuiDocumentRendererLayout(narrow, 240, 0, 800) == XUI_OK);
    CHECK(xuiDocumentRendererGetSize(wide, &wsize, &exact) == XUI_OK && exact);
    CHECK(xuiDocumentRendererGetSize(narrow, &nsize, &exact) == XUI_OK && exact && nsize.height > wsize.height);
    {
        xui_doc_range_t selection;
        xui_doc_rect_t rects[16], cell_caret;
        uint64_t rectangles;
        selection.tAnchor = position(document, text[2], 0);
        selection.tCaret = position(document, text[2], 8);
        CHECK(xuiDocumentRendererGetRangeRects(narrow, &selection,
            rects, 16, &rectangles) == XUI_OK && rectangles > 0 && rectangles <= 16);
        CHECK(xuiDocumentRendererGetCaretRect(narrow, &selection.tAnchor,
            &cell_caret) == XUI_OK && fabs(rects[0].y - cell_caret.y) < .01);
    }
    for (i = 0; i < 4; i++) {
        p = position(document, text[i], 0); CHECK(xuiDocumentRendererGetCaretRect(narrow, &p, &caret) == XUI_OK);
        CHECK(xuiDocumentRendererHitTest(narrow, caret.x + 1, caret.y + caret.height / 2, &hit) == XUI_OK && hit.iNodeId == text[i]);
    }
    CHECK(proxy.tProxy.drawBegin(&proxy.tProxy, &draw, target) == XUI_OK);
    CHECK(xuiDocumentRendererDraw(narrow, draw, 0, 0, (xui_rect_t){0, 0, 240, 800}, NULL, 0) == XUI_OK);
    CHECK(proxy.tProxy.drawEnd(&proxy.tProxy, draw) == XUI_OK && xuiTestSurfaceGetTextDrawCount(target) > 10);
    CHECK(xuiDocumentBeginTransaction(document, NULL, &t) == XUI_OK);
    CHECK(xuiDocumentTxnReplaceText(t, text[0], 0, 1, "Edited cell", 11) == XUI_OK);
    CHECK(xuiDocumentTxnCommit(t, &changes) == XUI_OK); xuiDocumentTxnRelease(t);
    CHECK(xuiDocumentAcquireSnapshot(document, &s) == XUI_OK);
    CHECK(xuiDocumentRendererSetSnapshot(narrow, s, changes) == XUI_OK); xuiDocumentSnapshotRelease(s); xuiDocumentChangeSetRelease(changes);
    CHECK(xuiDocumentRendererLayout(narrow, 240, 0, 800) == XUI_OK);
    p = position(document, body, 0); CHECK(xuiDocumentRendererGetCaretRect(narrow, &p, &caret) == XUI_OK);
    CHECK(xuiDocumentBeginTransaction(document, NULL, &t) == XUI_OK);
    CHECK(xuiDocumentTxnPasteTableMatrix(t, table, 0, 0,
        "\"first\nsecond\"\tother", strlen("\"first\nsecond\"\tother"), &cell) == XUI_OK);
    CHECK(xuiDocumentTxnCommit(t, &changes) == XUI_OK); xuiDocumentTxnRelease(t);
    CHECK(xuiDocumentAcquireSnapshot(document, &s) == XUI_OK);
    CHECK(xuiDocumentSnapshotGetChild(s, table, 0, &row) == XUI_OK);
    CHECK(xuiDocumentSnapshotGetChild(s, row, 0, &cell) == XUI_OK);
    CHECK(xuiDocumentSnapshotGetChild(s, cell, 0, &para) == XUI_OK);
    CHECK(xuiDocumentSnapshotGetChild(s, para, 0, &text[0]) == XUI_OK);
    CHECK(xuiDocumentRendererSetSnapshot(narrow, s, changes) == XUI_OK);
    xuiDocumentSnapshotRelease(s); xuiDocumentChangeSetRelease(changes);
    CHECK(xuiDocumentRendererLayout(narrow, 240, 0, 800) == XUI_OK);
    p = position(document, text[0], 1);
    CHECK(xuiDocumentRendererGetCaretRect(narrow, &p, &caret) == XUI_OK);
    p.iOffset = 6;
    p.iAffinity = XUI_DOC_AFTER;
    CHECK(xuiDocumentRendererGetCaretRect(narrow, &p, &second_line) == XUI_OK &&
        second_line.y > caret.y);
    xuiDocumentRendererRelease(wide); xuiDocumentRendererRelease(narrow); xuiDocumentRelease(document);
    {
        xui_doc_desc_t md = {0}; const char* source = "# Heading\n\n> - **bold** and ordinary text\n> - more text\n\n| A | B |\n| - | - |\n| cell | second |\n\n```c\nint value = 1;\n```\n";
        md.iSize = sizeof(md); md.iProfile = XUI_DOCUMENT_MARKDOWN; CHECK(xuiDocumentCreate(&md, &document) == XUI_OK);
        CHECK(xuiDocumentLoadMarkdown(document, source, strlen(source)) == XUI_OK); CHECK(xuiDocumentAcquireSnapshot(document, &s) == XUI_OK);
        CHECK(xuiDocumentRendererCreate(context, NULL, &wide) == XUI_OK); CHECK(xuiDocumentRendererSetSnapshot(wide, s, NULL) == XUI_OK);
        xuiDocumentSnapshotRelease(s); CHECK(xuiDocumentRendererLayout(wide, 300, 0, 800) == XUI_OK);
        CHECK(proxy.tProxy.drawBegin(&proxy.tProxy, &draw, target) == XUI_OK);
        CHECK(xuiDocumentRendererDraw(wide, draw, 0, 0, (xui_rect_t){0, 0, 300, 800}, NULL, 0) == XUI_OK);
        CHECK(proxy.tProxy.drawEnd(&proxy.tProxy, draw) == XUI_OK);
        stats.iSize = sizeof(stats); CHECK(xuiDocumentRendererGetStats(wide, &stats) == XUI_OK && stats.iMeasuredBlocks == 5);
        CHECK(xuiDocumentRendererSetMode(wide, XUI_DOC_SOURCE_TEXT) == XUI_OK);
        CHECK(xuiDocumentRendererLayout(wide, 300, 0, 800) == XUI_OK);
        p = position(document, 1, 3); p.iKind = XUI_DOC_POSITION_SOURCE;
        CHECK(xuiDocumentRendererGetCaretRect(wide, &p, &caret) == XUI_OK);
        CHECK(xuiDocumentRendererHitTest(wide, caret.x + 1, caret.y + caret.height / 2, &hit) == XUI_OK && hit.iKind == XUI_DOC_POSITION_SOURCE && hit.iOffset == 3);
        CHECK(proxy.tProxy.drawBegin(&proxy.tProxy, &draw, target) == XUI_OK);
        CHECK(xuiDocumentRendererDraw(wide, draw, 0, 0, (xui_rect_t){0, 0, 300, 800}, NULL, 0) == XUI_OK);
        CHECK(proxy.tProxy.drawEnd(&proxy.tProxy, draw) == XUI_OK);
        CHECK(xuiDocumentRendererSetMode(wide, XUI_DOC_VISUAL) == XUI_OK);
        CHECK(xuiDocumentRendererLayout(wide, 300, 0, 800) == XUI_OK);
        xuiDocumentRendererRelease(wide); xuiDocumentRelease(document);
    }
    CHECK(xuiDocumentCreate(NULL, &document) == XUI_OK); CHECK(xuiDocumentBeginTransaction(document, NULL, &t) == XUI_OK);
    for (i = 0; i < 10000; i++) { para = add(t, 1, XUI_DOC_PARAGRAPH, NULL); add(t, para, XUI_DOC_TEXT, "A paragraph for lazy layout."); }
    CHECK(xuiDocumentTxnCommit(t, NULL) == XUI_OK); xuiDocumentTxnRelease(t); CHECK(xuiDocumentAcquireSnapshot(document, &s) == XUI_OK);
    CHECK(xuiDocumentRendererCreate(context, NULL, &wide) == XUI_OK); CHECK(xuiDocumentRendererSetSnapshot(wide, s, NULL) == XUI_OK); xuiDocumentSnapshotRelease(s);
    CHECK(xuiDocumentRendererLayout(wide, 300, 0, 400) == XUI_OK);
    CHECK(xuiDocumentRendererGetStats(wide, &stats) == XUI_OK && stats.iBlocks == 10000 && stats.iMeasuredBlocks < 50);
    CHECK(xuiDocumentRendererLayout(wide, 300, 200000, 400) == XUI_OK);
    CHECK(xuiDocumentRendererGetStats(wide, &stats) == XUI_OK && stats.iMeasuredBlocks < 100);
    xuiDocumentRendererRelease(wide); xuiDocumentRelease(document);
    {
        xui_widget view_widget; xui_doc_view_desc_t vd = {0}; xui_doc_range_t selection;
        xui_event_t event = {0}; xui_rect_i_t damage = {0, 0, 640, 800}; xui_surface cache;
        CHECK(xuiDocumentCreate(NULL, &document) == XUI_OK); CHECK(xuiDocumentBeginTransaction(document, NULL, &t) == XUI_OK);
        para = add(t, 1, XUI_DOC_PARAGRAPH, NULL); body = add(t, para, XUI_DOC_TEXT, "View text");
        CHECK(xuiDocumentTxnCommit(t, NULL) == XUI_OK); xuiDocumentTxnRelease(t);
        vd.iSize = sizeof(vd); vd.pDocument = document;
        CHECK(xuiDocumentViewCreate(context, &vd, &view_widget) == XUI_OK);
        CHECK(xuiSetRootWidget(context, view_widget) == XUI_OK); CHECK(xuiWidgetSetRect(view_widget, (xui_rect_t){0, 0, 320, 240}) == XUI_OK);
        CHECK(xuiInputViewport(context, 320, 240) == XUI_OK); CHECK(xuiUpdate(context, .016f) == XUI_OK);
        CHECK(xuiRender(context, target, &damage, 1) == XUI_OK);
        cache = xuiWidgetGetCacheSurface(view_widget, xuiWidgetGetStateId(view_widget));
        CHECK(cache && xuiTestSurfaceGetTextDrawCount(cache) > 0);
        selection.tAnchor = selection.tCaret = position(document, body, 9);
        selection.tAnchor.iAffinity = selection.tCaret.iAffinity = XUI_DOC_AFTER;
        CHECK(xuiDocumentViewSetSelection(view_widget, &selection) == XUI_OK);
        CHECK(xuiDocumentBeginTransaction(document, NULL, &t) == XUI_OK); CHECK(xuiDocumentTxnReplaceText(t, body, 9, 9, " changed", 8) == XUI_OK);
        CHECK(xuiDocumentTxnCommit(t, NULL) == XUI_OK); xuiDocumentTxnRelease(t);
        CHECK(xuiDocumentViewSetSelection(view_widget, &selection) == XUI_DOC_ERROR_STALE);
        CHECK(xuiDocumentViewGetSelection(view_widget, &selection) == XUI_OK && selection.tCaret.iOffset == 17);
        CHECK(xuiDocumentViewSetZoom(view_widget, 1.5f) == XUI_OK);
        CHECK(xuiUpdate(context, .016f) == XUI_OK); CHECK(xuiRender(context, target, &damage, 1) == XUI_OK);
        event.iSize = sizeof(event); event.iType = XUI_EVENT_POINTER_DOWN; event.pTarget = view_widget;
        event.iButton = XUI_POINTER_BUTTON_LEFT; event.fX = 2; event.fY = 2;
        CHECK(xuiDispatchEvent(context, &event) == XUI_OK);
        event.iType = XUI_EVENT_POINTER_UP; CHECK(xuiDispatchEvent(context, &event) == XUI_OK);
        CHECK(xuiDocumentViewGetSelection(view_widget, &selection) == XUI_OK && selection.tCaret.iNodeId == body);
        xuiDocumentRelease(document); xuiWidgetDestroy(view_widget);
    }
    {
        const unsigned char encoded = 1; int destroyed;
        CHECK(xuiDocumentImageResourceLoadMemory(context, "doc.teardown", &encoded, 1, NULL, NULL) == XUI_OK);
        proxy.tProxy.surfaceDestroy(&proxy.tProxy, target);
        destroyed = xuiTestProxyGetSurfaceDestroyCount(&proxy);
        xuiDestroy(context);
        CHECK(xuiTestProxyGetSurfaceDestroyCount(&proxy) == destroyed + 1);
    }
    proxy.tProxy.fontDestroy(&proxy.tProxy, font);
    puts("Unified Document renderer: rich/Markdown, nested cells, shared views, hit testing, drawing and lazy layout passed."); return 0;
}
