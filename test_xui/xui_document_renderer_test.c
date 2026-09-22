#include "../xui_document_ui.h"
#include "xui_test_proxy.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define CHECK(c) do { if (!(c)) { fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #c); exit(1); } } while (0)
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
    CHECK(xuiDocumentRendererGetStats(r, &stats) == XUI_OK && stats.bLiveSourceFallback && stats.iSourceBlocks == stats.iBlocks);
    CHECK(xuiDocumentRendererGetActiveSourceRange(r, &start, &end) == XUI_OK && !start && end == 10);
    xuiDocumentRendererRelease(r); xuiDocumentSnapshotRelease(snapshot); xuiDocumentRelease(d);
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
    printf("Live Markdown: %llu corpus cases, %llu UTF-8 caret boundaries passed; %llu cases use the declared source fallback (not HTML/display conformance).\n",
        (unsigned long long)count, (unsigned long long)boundaries, (unsigned long long)fallbacks);
}
static int outer_wheel(xui_widget widget, const xui_event_t* event, void* user)
{
    (void)widget;
    if (event->iType == XUI_EVENT_POINTER_WHEEL && event->iPhase == XUI_EVENT_PHASE_BUBBLE) (*(unsigned*)user)++;
    return XUI_OK;
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
int main(int argc, char** argv)
{
    xui_test_proxy_state_t proxy; xui_context context; xui_font font; xui_surface target; xui_draw_context draw;
    xui_document document; xui_document_transaction t; xui_document_snapshot s; xui_document_change_set changes;
    xui_document_renderer wide, narrow; xui_doc_rect_t wsize, nsize, caret; xui_doc_position_t p, hit;
    uint64_t para, body, table, row, cell, text[4], i; int exact;
    xui_doc_renderer_stats_t stats = {0}; xui_doc_renderer_desc_t desc = {0};
    xuiTestProxyInit(&proxy); CHECK(xuiCreate(&context) == XUI_OK); CHECK(xuiSetProxy(context, &proxy.tProxy) == XUI_OK);
    CHECK(proxy.tProxy.fontLoadFile(&proxy.tProxy, &font, "test.ttf", 14, 0) == XUI_OK);
    CHECK(xuiSetDefaultFont(context, font) == XUI_OK); CHECK(xuiTestSurfaceCreate(&proxy, &target, 640, 800, XUI_SURFACE_USAGE_TARGET) == XUI_OK);
    live_markdown(context);
    if (argc > 1) live_corpus(context, argv[1]);
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
    proxy.tProxy.surfaceDestroy(&proxy.tProxy, target); xuiDestroy(context); proxy.tProxy.fontDestroy(&proxy.tProxy, font);
    puts("Unified Document renderer: rich/Markdown, nested cells, shared views, hit testing, drawing and lazy layout passed."); return 0;
}
