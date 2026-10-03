#include "../xui_document_ui.h"
#include "xui_test_proxy.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define CHECK(expr) do { if (!(expr)) { \
    fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #expr); exit(1); \
} } while (0)

static void check_document(xui_context context, xui_surface target,
    xui_test_proxy_state_t* proxy, xui_document document, uint64_t text_id)
{
    xui_document_snapshot snapshot;
    xui_document_renderer renderer;
    xui_doc_rect_t size, first, later;
    xui_doc_position_t at = {0}, hit = {0};
    xui_draw_context draw;
    int exact;
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
    CHECK(xuiDocumentRendererCreate(context, NULL, &renderer) == XUI_OK);
    CHECK(xuiDocumentRendererSetSnapshot(renderer, snapshot, NULL) == XUI_OK);
    xuiDocumentSnapshotRelease(snapshot);
    CHECK(xuiDocumentRendererLayout(renderer, 320, 0, 200) == XUI_OK);
    CHECK(xuiDocumentRendererGetSize(renderer, &size, &exact) == XUI_OK &&
        exact && size.height > 0);
    at.iSize = sizeof(at); at.iKind = XUI_DOC_POSITION_TEXT;
    at.iDocumentId = xuiDocumentGetIdentity(document);
    at.iRevision = xuiDocumentGetRevision(document);
    at.iNodeId = text_id;
    CHECK(xuiDocumentRendererGetCaretRect(renderer, &at, &first) == XUI_OK);
    at.iOffset = 5;
    CHECK(xuiDocumentRendererGetCaretRect(renderer, &at, &later) == XUI_OK &&
        later.x > first.x);
    CHECK(xuiDocumentRendererHitTest(renderer, later.x, later.y + later.height / 2,
        &hit) == XUI_OK && hit.iNodeId == text_id);
    xuiTestSurfaceReset(target);
    CHECK(proxy->tProxy.drawBegin(&proxy->tProxy, &draw, target) == XUI_OK);
    CHECK(xuiDocumentRendererDraw(renderer, draw, 0, 0,
        (xui_rect_t){0, 0, 320, 200}, NULL, 0) == XUI_OK);
    CHECK(proxy->tProxy.drawEnd(&proxy->tProxy, draw) == XUI_OK &&
        xuiTestSurfaceGetTextDrawCount(target) > 0);
    xuiDocumentRendererRelease(renderer);
}
static void check_admonition(xui_context context, xui_surface target,
    xui_test_proxy_state_t* proxy)
{
    const char* source = "> [!NOTE]\n> Alert\n";
    xui_doc_desc_t md = {0}; xui_doc_renderer_desc_t desc = {0};
    xui_document document; xui_document_snapshot snapshot;
    xui_document_renderer renderer; xui_doc_range_t range;
    xui_doc_range_t whole = {0};
    xui_doc_node_id quote; xui_doc_position_t hit;
    xui_doc_rect_t caret; xui_draw_context draw;
    uint64_t count, rectangles = 0;
    md.iSize = sizeof(md); md.iProfile = XUI_DOCUMENT_MARKDOWN;
    CHECK(xuiDocumentCreate(&md, &document) == XUI_OK &&
        xuiDocumentLoadMarkdown(document, source, strlen(source)) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK &&
        xuiDocumentSnapshotGetChild(snapshot, 1, 0, &quote) == XUI_OK);
    desc.iSize = sizeof(desc);
    CHECK(xuiDocumentRendererCreate(context, &desc, &renderer) == XUI_OK &&
        xuiDocumentRendererSetSnapshot(renderer, snapshot, NULL) == XUI_OK &&
        xuiDocumentRendererLayout(renderer, 320, 0, 200) == XUI_OK);
    CHECK(xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC, "Alert", 5,
        NULL, &range, 1, &count) == XUI_OK && count == 1 &&
        xuiDocumentRendererGetCaretRect(renderer, &range.tAnchor, &caret) == XUI_OK &&
        caret.y >= 20);
    CHECK(xuiDocumentRendererHitTest(renderer, 25, 5, &hit) == XUI_OK &&
        hit.iNodeId == quote && hit.iKind == XUI_DOC_POSITION_GAP && hit.iOffset == 0);
    whole.tAnchor = hit; whole.tAnchor.iNodeId = 1;
    whole.tCaret = whole.tAnchor; whole.tCaret.iOffset = 1;
    CHECK(xuiDocumentRendererGetRangeRects(renderer, &whole,
        NULL, 0, &rectangles) == XUI_OK && rectangles > 0);
    xuiTestSurfaceReset(target);
    CHECK(proxy->tProxy.drawBegin(&proxy->tProxy, &draw, target) == XUI_OK &&
        xuiDocumentRendererDraw(renderer, draw, 0, 0,
            (xui_rect_t){0, 0, 320, 200}, NULL, 0) == XUI_OK &&
        proxy->tProxy.drawEnd(&proxy->tProxy, draw) == XUI_OK &&
        xuiTestSurfaceGetTextDrawCount(target) == 2);
    xuiDocumentRendererRelease(renderer);
    xuiDocumentSnapshotRelease(snapshot);
    xuiDocumentRelease(document);
}

static void check_long_word(xui_context context)
{
    const size_t bytes = 48 * 1024;
    char* text = malloc(bytes + 1);
    xui_document document;
    xui_document_transaction txn;
    xui_document_snapshot snapshot;
    xui_document_renderer renderer;
    xui_doc_node_desc_t node = {0};
    xui_doc_renderer_stats_t stats = {0};
    xui_doc_position_t at = {0}, hit = {0};
    xui_doc_rect_t caret, size;
    uint64_t paragraph, text_id;
    int exact;
    CHECK(text != NULL);
    memset(text, 'A', bytes); text[bytes] = 0;
    CHECK(xuiDocumentCreate(NULL, &document) == XUI_OK &&
        xuiDocumentBeginTransaction(document, NULL, &txn) == XUI_OK);
    node.iSize = sizeof(node); node.iKind = XUI_DOC_PARAGRAPH;
    CHECK(xuiDocumentTxnInsertNode(txn, 1, XUI_DOCUMENT_APPEND,
        &node, &paragraph) == XUI_OK);
    node.iKind = XUI_DOC_TEXT; node.sText = text; node.iTextBytes = bytes;
    CHECK(xuiDocumentTxnInsertNode(txn, paragraph, XUI_DOCUMENT_APPEND,
        &node, &text_id) == XUI_OK &&
        xuiDocumentTxnCommit(txn, NULL) == XUI_OK);
    xuiDocumentTxnRelease(txn);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK &&
        xuiDocumentRendererCreate(context, NULL, &renderer) == XUI_OK &&
        xuiDocumentRendererSetSnapshot(renderer, snapshot, NULL) == XUI_OK);
    xuiDocumentSnapshotRelease(snapshot);
    CHECK(xuiDocumentRendererLayout(renderer, 320, 0, 200) == XUI_OK);
    stats.iSize = sizeof(stats);
    CHECK(xuiDocumentRendererGetStats(renderer, &stats) == XUI_OK &&
        stats.iShapedBytes > 0 && stats.iShapedBytes < bytes / 2 &&
        xuiDocumentRendererGetSize(renderer, &size, &exact) == XUI_OK && !exact);
    at.iSize = sizeof(at); at.iKind = XUI_DOC_POSITION_TEXT;
    at.iDocumentId = xuiDocumentGetIdentity(document);
    at.iRevision = xuiDocumentGetRevision(document);
    at.iNodeId = text_id; at.iOffset = bytes - 128;
    CHECK(xuiDocumentRendererGetCaretRect(renderer, &at, &caret) == XUI_OK &&
        xuiDocumentRendererHitTest(renderer, caret.x + .1,
            caret.y + caret.height / 2, &hit) == XUI_OK &&
        hit.iNodeId == text_id && hit.iOffset == at.iOffset &&
        xuiDocumentRendererGetSize(renderer, &size, &exact) == XUI_OK && exact);
    xuiDocumentRendererRelease(renderer);
    xuiDocumentRelease(document);
    free(text);
}
static void check_long_code(xui_context context, xui_surface target,
    xui_test_proxy_state_t* proxy)
{
    const size_t bytes = 48 * 1024;
    char* text = malloc(bytes + 1);
    xui_document document;
    xui_document_transaction txn;
    xui_document_snapshot snapshot;
    xui_document_renderer renderer;
    xui_doc_node_desc_t node = {0};
    xui_doc_renderer_stats_t stats = {0};
    xui_doc_position_t at = {0}, hit = {0};
    xui_doc_rect_t caret, size;
    xui_draw_context draw;
    uint64_t code;
    int exact;
    CHECK(text != NULL);
    memset(text, 'A', bytes); text[bytes] = 0;
    CHECK(xuiDocumentCreate(NULL, &document) == XUI_OK &&
        xuiDocumentBeginTransaction(document, NULL, &txn) == XUI_OK);
    node.iSize = sizeof(node); node.iKind = XUI_DOC_CODE_BLOCK;
    node.sText = text; node.iTextBytes = bytes;
    CHECK(xuiDocumentTxnInsertNode(txn, 1, XUI_DOCUMENT_APPEND,
        &node, &code) == XUI_OK &&
        xuiDocumentTxnCommit(txn, NULL) == XUI_OK);
    xuiDocumentTxnRelease(txn);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK &&
        xuiDocumentRendererCreate(context, NULL, &renderer) == XUI_OK &&
        xuiDocumentRendererSetSnapshot(renderer, snapshot, NULL) == XUI_OK);
    xuiDocumentSnapshotRelease(snapshot);
    CHECK(xuiDocumentRendererLayout(renderer, 320, 0, 200) == XUI_OK);
    stats.iSize = sizeof(stats);
    CHECK(xuiDocumentRendererGetStats(renderer, &stats) == XUI_OK &&
        stats.iShapedBytes == 8192 &&
        xuiDocumentRendererGetSize(renderer, &size, &exact) == XUI_OK &&
        !exact && size.width > 320);
    {
        xui_doc_range_t range = {0};
        xui_doc_rect_t rect;
        uint64_t count = 0;
        range.tAnchor.iSize = range.tCaret.iSize = sizeof(range.tAnchor);
        range.tAnchor.iKind = range.tCaret.iKind = XUI_DOC_POSITION_TEXT;
        range.tAnchor.iDocumentId = range.tCaret.iDocumentId =
            xuiDocumentGetIdentity(document);
        range.tAnchor.iRevision = range.tCaret.iRevision =
            xuiDocumentGetRevision(document);
        range.tAnchor.iNodeId = range.tCaret.iNodeId = code;
        range.tAnchor.iOffset = 1000; range.tCaret.iOffset = 16000;
        CHECK(xuiDocumentRendererGetRangeRects(renderer, &range,
            &rect, 1, &count) == XUI_OK && count == 1 &&
            rect.width > 0);
        CHECK(xuiDocumentRendererGetStats(renderer, &stats) == XUI_OK &&
            stats.iShapedBytes < bytes / 2 &&
            xuiDocumentRendererGetSize(renderer, &size, &exact) == XUI_OK &&
            !exact);
    }
    at.iSize = sizeof(at); at.iKind = XUI_DOC_POSITION_TEXT;
    at.iDocumentId = xuiDocumentGetIdentity(document);
    at.iRevision = xuiDocumentGetRevision(document);
    at.iNodeId = code; at.iOffset = bytes / 2;
    CHECK(xuiDocumentRendererGetCaretRect(renderer, &at, &caret) == XUI_OK &&
        xuiDocumentRendererHitTest(renderer, caret.x + .1,
            caret.y + caret.height / 2, &hit) == XUI_OK &&
        hit.iNodeId == code && hit.iOffset == at.iOffset);
    xuiTestSurfaceReset(target);
    CHECK(proxy->tProxy.drawBegin(&proxy->tProxy, &draw, target) == XUI_OK);
    CHECK(xuiDocumentRendererDraw(renderer, draw, 100 - caret.x, 0,
        (xui_rect_t){0, 0, 320, 200}, NULL, 0) == XUI_OK);
    CHECK(proxy->tProxy.drawEnd(&proxy->tProxy, draw) == XUI_OK &&
        xuiTestSurfaceGetTextDrawCount(target) > 0);
    xuiDocumentRendererRelease(renderer);
    xuiDocumentRelease(document);
    free(text);
}
static void check_long_source(xui_context context, xui_surface target,
    xui_test_proxy_state_t* proxy)
{
    const size_t bytes = 48 * 1024;
    char* text = malloc(bytes + 1);
    xui_doc_desc_t desc = {0};
    xui_document document;
    xui_document_snapshot snapshot;
    xui_document_renderer renderer;
    xui_doc_renderer_stats_t stats = {0};
    xui_doc_range_t range = {0};
    xui_doc_position_t at = {0}, hit = {0};
    xui_doc_rect_t caret, rect, size;
    xui_draw_context draw;
    uint64_t count = 0;
    int exact;
    CHECK(text != NULL);
    memset(text, 'A', bytes); text[bytes] = 0;
    desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN;
    CHECK(xuiDocumentCreate(&desc, &document) == XUI_OK);
    CHECK(xuiDocumentLoadMarkdown(document, text, bytes) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
    CHECK(xuiDocumentRendererCreate(context, NULL, &renderer) == XUI_OK);
    CHECK(xuiDocumentRendererSetMode(renderer, XUI_DOC_SOURCE_TEXT) == XUI_OK);
    CHECK(xuiDocumentRendererSetSnapshot(renderer, snapshot, NULL) == XUI_OK);
    xuiDocumentSnapshotRelease(snapshot);
    CHECK(xuiDocumentRendererLayout(renderer, 320, 0, 200) == XUI_OK);
    stats.iSize = sizeof(stats);
    CHECK(xuiDocumentRendererGetStats(renderer, &stats) == XUI_OK &&
        stats.iShapedBytes == 8192 &&
        xuiDocumentRendererGetSize(renderer, &size, &exact) == XUI_OK &&
        !exact && size.width > 320);
    at.iSize = sizeof(at); at.iKind = XUI_DOC_POSITION_SOURCE;
    at.iDocumentId = xuiDocumentGetIdentity(document);
    at.iRevision = xuiDocumentGetRevision(document);
    at.iNodeId = 1; at.iOffset = 1000;
    range.tAnchor = at;
    at.iOffset = 16000; range.tCaret = at;
    CHECK(xuiDocumentRendererGetRangeRects(renderer, &range,
        &rect, 1, &count) == XUI_OK && count == 1 && rect.width > 0);
    CHECK(xuiDocumentRendererGetStats(renderer, &stats) == XUI_OK &&
        stats.iShapedBytes < bytes / 2 &&
        xuiDocumentRendererGetSize(renderer, &size, &exact) == XUI_OK &&
        !exact);
    at.iOffset = bytes / 2;
    CHECK(xuiDocumentRendererGetCaretRect(renderer, &at, &caret) == XUI_OK &&
        xuiDocumentRendererHitTest(renderer, caret.x + .1,
            caret.y + caret.height / 2, &hit) == XUI_OK &&
        hit.iKind == XUI_DOC_POSITION_SOURCE &&
        hit.iOffset == at.iOffset);
    xuiTestSurfaceReset(target);
    CHECK(proxy->tProxy.drawBegin(&proxy->tProxy, &draw, target) == XUI_OK);
    CHECK(xuiDocumentRendererDraw(renderer, draw, 100 - caret.x, 0,
        (xui_rect_t){0, 0, 320, 200}, NULL, 0) == XUI_OK);
    CHECK(proxy->tProxy.drawEnd(&proxy->tProxy, draw) == XUI_OK &&
        xuiTestSurfaceGetTextDrawCount(target) > 0);
    xuiDocumentRendererRelease(renderer);
    xuiDocumentRelease(document);
    free(text);
}

static xui_text_shape_proc portability_base_shape;
static int portability_tall_shape(xui_proxy proxy, const xui_text_item_t* pTextItem, xui_text_shape_t* shape)
{
    const char* text = pTextItem && pTextItem->iSize >= sizeof(*pTextItem) ? pTextItem->sText : NULL;
    int bytes = pTextItem && pTextItem->iSize >= sizeof(*pTextItem) ? pTextItem->iTextSize : 0;

    int i, result = portability_base_shape(proxy, pTextItem, shape);
    if (result != XUI_OK) return result;
    for (i = 0; i + 2 < bytes; i++) {
        if ((unsigned char)text[i] != 0xe4 ||
            (unsigned char)text[i + 1] != 0xb8 ||
            (unsigned char)text[i + 2] != 0xad) continue;
        shape->fAscent *= 3;
        shape->fDescent *= 3;
        shape->fLineHeight *= 3;
        shape->fHeight = shape->fLineHeight;
        break;
    }
    return XUI_OK;
}
static void check_unicode_source_height(void)
{
    static const char source[] = "\xe4\xb8\xad\none\ntwo\nthree\n";
    xui_test_proxy_state_t proxy;
    xui_context context;
    xui_font font;
    xui_doc_desc_t desc = {0};
    xui_document document;
    xui_document_snapshot snapshot;
    xui_document_renderer cold, warm, live_cold, live_warm;
    xui_doc_position_t p = {0};
    xui_doc_rect_t actual, expected;
    xuiTestProxyInit(&proxy);
    portability_base_shape = proxy.tProxy.textShape;
    proxy.tProxy.textShape = portability_tall_shape;
    CHECK(xuiCreate(&context) == XUI_OK &&
        xuiSetProxy(context, &proxy.tProxy) == XUI_OK);
    CHECK(proxy.tProxy.fontLoadFile(&proxy.tProxy, &font,
        "test.ttf", 14, 0) == XUI_OK);
    CHECK(xuiSetDefaultFont(context, font) == XUI_OK);
    desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN;
    CHECK(xuiDocumentCreate(&desc, &document) == XUI_OK);
    CHECK(xuiDocumentLoadMarkdown(document, source,
        sizeof(source) - 1) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
    p.iSize = sizeof(p); p.iKind = XUI_DOC_POSITION_SOURCE;
    p.iDocumentId = xuiDocumentGetIdentity(document);
    p.iRevision = xuiDocumentGetRevision(document);
    p.iNodeId = 1; p.iOffset = sizeof(source) - 3;
    CHECK(xuiDocumentRendererCreate(context, NULL, &cold) == XUI_OK);
    CHECK(xuiDocumentRendererCreate(context, NULL, &warm) == XUI_OK);
    CHECK(xuiDocumentRendererSetMode(cold, XUI_DOC_SOURCE_TEXT) == XUI_OK);
    CHECK(xuiDocumentRendererSetMode(warm, XUI_DOC_SOURCE_TEXT) == XUI_OK);
    CHECK(xuiDocumentRendererSetSnapshot(cold, snapshot, NULL) == XUI_OK);
    CHECK(xuiDocumentRendererSetSnapshot(warm, snapshot, NULL) == XUI_OK);
    CHECK(xuiDocumentRendererLayout(cold, 320, 0, 0) == XUI_OK);
    CHECK(xuiDocumentRendererLayout(warm, 320, 0, 120) == XUI_OK);
    CHECK(xuiDocumentRendererGetCaretRect(cold, &p, &actual) == XUI_OK);
    CHECK(xuiDocumentRendererGetCaretRect(warm, &p, &expected) == XUI_OK);
    CHECK(fabs(actual.y - expected.y) < .01);
    CHECK(xuiDocumentRendererCreate(context, NULL, &live_cold) == XUI_OK);
    CHECK(xuiDocumentRendererCreate(context, NULL, &live_warm) == XUI_OK);
    CHECK(xuiDocumentRendererSetMode(live_cold, XUI_DOC_LIVE_MARKDOWN) == XUI_OK);
    CHECK(xuiDocumentRendererSetMode(live_warm, XUI_DOC_LIVE_MARKDOWN) == XUI_OK);
    CHECK(xuiDocumentRendererSetSnapshot(live_cold, snapshot, NULL) == XUI_OK);
    CHECK(xuiDocumentRendererSetSnapshot(live_warm, snapshot, NULL) == XUI_OK);
    CHECK(xuiDocumentRendererSetActivePosition(live_cold, &p) == XUI_OK);
    CHECK(xuiDocumentRendererSetActivePosition(live_warm, &p) == XUI_OK);
    CHECK(xuiDocumentRendererLayout(live_cold, 320, 0, 0) == XUI_OK);
    CHECK(xuiDocumentRendererLayout(live_warm, 320, 0, 120) == XUI_OK);
    CHECK(xuiDocumentRendererGetCaretRect(live_cold, &p, &actual) == XUI_OK);
    CHECK(xuiDocumentRendererGetCaretRect(live_warm, &p, &expected) == XUI_OK);
    CHECK(fabs(actual.y - expected.y) < .01);
    xuiDocumentRendererRelease(cold);
    xuiDocumentRendererRelease(warm);
    xuiDocumentRendererRelease(live_cold);
    xuiDocumentRendererRelease(live_warm);
    xuiDocumentSnapshotRelease(snapshot);
    xuiDocumentRelease(document);
    xuiDestroy(context);
    proxy.tProxy.fontDestroy(&proxy.tProxy, font);
}

static void check_table_alignment(xui_context context)
{
    static const char source[] = "| H | C |\n| ---: | :---: |\n| x | y |\n";
    xui_doc_desc_t desc = {0};
    xui_document document = NULL;
    xui_document_snapshot snapshot = NULL;
    xui_document_renderer renderer = NULL;
    xui_doc_cell_hit_t cell_hit = {0};
    xui_doc_position_t position = {0};
    xui_doc_rect_t caret;
    uint64_t table, row, cell, paragraph, text_id;
    unsigned i;

    desc.iSize = sizeof(desc);
    desc.iProfile = XUI_DOCUMENT_MARKDOWN;
    desc.iMarkdownDialect = XUI_MD_GFM;
    CHECK(xuiDocumentCreate(&desc, &document) == XUI_OK &&
        xuiDocumentLoadMarkdown(document, source, sizeof(source) - 1) == XUI_OK &&
        xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK &&
        xuiDocumentSnapshotGetChild(snapshot, 1, 0, &table) == XUI_OK &&
        xuiDocumentSnapshotGetChild(snapshot, table, 1, &row) == XUI_OK &&
        xuiDocumentRendererCreate(context, NULL, &renderer) == XUI_OK &&
        xuiDocumentRendererSetSnapshot(renderer, snapshot, NULL) == XUI_OK &&
        xuiDocumentRendererLayout(renderer, 320, 0, 200) == XUI_OK);
    position.iSize = sizeof(position);
    position.iDocumentId = xuiDocumentGetIdentity(document);
    position.iRevision = xuiDocumentGetRevision(document);
    position.iKind = XUI_DOC_POSITION_TEXT;
    cell_hit.iSize = sizeof(cell_hit);
    for (i = 0; i < 2; i++) {
        CHECK(xuiDocumentSnapshotGetChild(snapshot, row, i, &cell) == XUI_OK &&
            xuiDocumentSnapshotGetChild(snapshot, cell, 0, &paragraph) == XUI_OK &&
            xuiDocumentSnapshotGetChild(snapshot, paragraph, 0, &text_id) == XUI_OK);
        position.iNodeId = text_id;
        CHECK(xuiDocumentRendererGetCellRect(renderer, cell, &cell_hit) == XUI_OK &&
            xuiDocumentRendererGetCaretRect(renderer, &position, &caret) == XUI_OK);
        if (!i)
            CHECK(caret.x > cell_hit.tBounds.x + cell_hit.tBounds.width * .5);
        else
            CHECK(fabs(caret.x -
                (cell_hit.tBounds.x + cell_hit.tBounds.width * .5)) < 30);
    }
    xuiDocumentRendererRelease(renderer);
    xuiDocumentSnapshotRelease(snapshot);
    xuiDocumentRelease(document);
}

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
#include "xui_document_shape_paint_cases.h"
#include "xui_document_shared_span_cases.h"
#include "xui_document_line_paint_cases.h"
static void check_paint_group(xui_context context, xui_surface target,
    xui_test_proxy_state_t* proxy)
{
    xui_document document;
    xui_document_transaction transaction;
    xui_document_snapshot snapshot;
    xui_document_renderer renderer;
    xui_doc_node_desc_t node = {0};
    xui_draw_context draw;
    uint64_t paragraph, text_id;
    int pass;
    CHECK(xuiDocumentCreate(NULL, &document) == XUI_OK &&
        xuiDocumentBeginTransaction(document, NULL, &transaction) == XUI_OK);
    node.iSize = sizeof(node); node.iKind = XUI_DOC_PARAGRAPH;
    CHECK(xuiDocumentTxnInsertNode(transaction, 1, XUI_DOCUMENT_APPEND,
        &node, &paragraph) == XUI_OK);
    node.iKind = XUI_DOC_TEXT; node.sText = "A"; node.iTextBytes = 1;
    CHECK(xuiDocumentTxnInsertNode(transaction, paragraph,
        XUI_DOCUMENT_APPEND, &node, &text_id) == XUI_OK);
    node.sText = "V";
    node.tAttributes.iTextColor = XUI_COLOR_RGBA(180, 20, 40, 255);
    CHECK(xuiDocumentTxnInsertNode(transaction, paragraph,
        XUI_DOCUMENT_APPEND, &node, &text_id) == XUI_OK);
    CHECK(xuiDocumentTxnCommit(transaction, NULL) == XUI_OK);
    xuiDocumentTxnRelease(transaction);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK &&
        xuiDocumentRendererCreate(context, NULL, &renderer) == XUI_OK &&
        xuiDocumentRendererSetSnapshot(renderer, snapshot, NULL) == XUI_OK);
    xuiDocumentSnapshotRelease(snapshot);
    for (pass = 0; pass < 3; pass++) {
        double width = pass == 1 ? 7 : 320;
        CHECK(xuiDocumentRendererLayout(renderer, width, 0, 100) == XUI_OK &&
            proxy->tProxy.drawBegin(&proxy->tProxy, &draw, target) == XUI_OK &&
            xuiDocumentRendererDraw(renderer, draw, 0, 0,
                (xui_rect_t){0, 0, 320, 100}, NULL, 0) == XUI_OK &&
            proxy->tProxy.drawEnd(&proxy->tProxy, draw) == XUI_OK);
    }
    xuiDocumentRendererRelease(renderer);
    xuiDocumentRelease(document);
}

int main(void)
{
    xui_test_proxy_state_t proxy;
    xui_context context;
    xui_font font;
    xui_surface target;
    xui_document document;
    xui_document_transaction txn;
    xui_doc_node_desc_t node = {0};
    xui_doc_desc_t desc = {0};
    uint64_t paragraph, text_id;

    xuiTestProxyInit(&proxy);
    CHECK(xuiCreate(&context) == XUI_OK);
    CHECK(xuiSetProxy(context, &proxy.tProxy) == XUI_OK);
    CHECK(proxy.tProxy.fontLoadFile(&proxy.tProxy, &font, "test.ttf", 14, 0) == XUI_OK);
    CHECK(xuiSetDefaultFont(context, font) == XUI_OK);
    CHECK(xuiTestSurfaceCreate(&proxy, &target, 320, 200,
        XUI_SURFACE_USAGE_TARGET) == XUI_OK);

    CHECK(xuiDocumentCreate(NULL, &document) == XUI_OK);
    CHECK(xuiDocumentBeginTransaction(document, NULL, &txn) == XUI_OK);
    node.iSize = sizeof(node); node.iKind = XUI_DOC_PARAGRAPH;
    CHECK(xuiDocumentTxnInsertNode(txn, 1, XUI_DOCUMENT_APPEND,
        &node, &paragraph) == XUI_OK);
    node.iKind = XUI_DOC_TEXT; node.sText = "Hello Linux renderer";
    node.iTextBytes = strlen(node.sText);
    CHECK(xuiDocumentTxnInsertNode(txn, paragraph, XUI_DOCUMENT_APPEND,
        &node, &text_id) == XUI_OK);
    CHECK(xuiDocumentTxnCommit(txn, NULL) == XUI_OK);
    xuiDocumentTxnRelease(txn);
    check_document(context, target, &proxy, document, text_id);
    xuiDocumentRelease(document);
    check_paint_group(context, target, &proxy);
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
    check_long_word(context);
    check_long_code(context, target, &proxy);
    check_long_source(context, target, &proxy);
    check_unicode_source_height();

    desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN;
    CHECK(xuiDocumentCreate(&desc, &document) == XUI_OK);
    CHECK(xuiDocumentLoadMarkdown(document, "Hello Linux renderer\n", 21) == XUI_OK);
    {
        xui_document_snapshot snapshot;
        CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
        CHECK(xuiDocumentSnapshotGetChild(snapshot, 1, 0,
            &paragraph) == XUI_OK);
        CHECK(xuiDocumentSnapshotGetChild(snapshot, paragraph, 0,
            &text_id) == XUI_OK);
        xuiDocumentSnapshotRelease(snapshot);
    }
    check_document(context, target, &proxy, document, text_id);
    xuiDocumentRelease(document);
    check_admonition(context, target, &proxy);
    check_table_alignment(context);

    proxy.tProxy.surfaceDestroy(&proxy.tProxy, target);
    xuiDestroy(context);
    proxy.tProxy.fontDestroy(&proxy.tProxy, font);
    puts("Linux headless Rich/Markdown Document renderer layout, caret, hit and draw passed");
    return 0;
}
