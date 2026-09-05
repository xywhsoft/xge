#include "../src/xui_internal.h"
#include "xui_test_proxy.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static struct {
    unsigned LayoutBlock, IndexBlock, SearchBlock, SearchLine, SearchFragment;
    unsigned FragmentRead, RenderBlock, ArrangeWidget, WidgetNode;
} g_work;
static int g_failAllocation = -1;
static void* auditMalloc(size_t n)
{
    if (g_failAllocation >= 0 && g_failAllocation-- == 0) return NULL;
    return xrtMalloc(n);
}
#define XUI_RICH_EDIT_AUDIT_STEP(name) (++g_work.name)
#define xrtMalloc auditMalloc
#include "../src/xui_rich_edit.c"
#undef xrtMalloc
#include "xui_rich_edit_scale_reference.h"

static xui_text_shape_proc g_shape;
static xui_draw_text_proc g_drawText;
static xui_draw_rect_fill_proc g_fill;
static unsigned g_shapeCalls, g_shapeBytes;
static uint64_t g_trace;
static struct trace_record { uint64_t hash; xui_rect_t rect; char text[80]; } g_records[16384], g_expected[16384];
static int g_recordCount;

static void recordDraw(xui_rect_t rect, const char* text)
{
    if (g_recordCount >= 16384) return;
    g_records[g_recordCount].hash = g_trace;
    g_records[g_recordCount].rect = rect;
    snprintf(g_records[g_recordCount].text, 80, "%s", text);
    g_recordCount++;
}

static void hashBytes(const void* bytes, size_t n)
{
    const unsigned char* p = (const unsigned char*)bytes;
    while (n--) g_trace = (g_trace ^ *p++) * UINT64_C(1099511628211);
}
static int auditShape(xui_proxy p, xui_font f, const char* text, int n, uint32_t flags, xui_text_shape_t* shape)
{
    g_shapeCalls++;
    g_shapeBytes += n < 0 ? (unsigned)strlen(text) : (unsigned)n;
    return g_shape(p, f, text, n, flags, shape);
}
static int auditText(xui_proxy p, xui_draw_context d, xui_font font, const char* text, xui_rect_t r, uint32_t color, uint32_t flags)
{
    hashBytes(text, strlen(text) + 1);
    hashBytes(&font, sizeof(font)); hashBytes(&r, sizeof(r));
    hashBytes(&color, sizeof(color)); hashBytes(&flags, sizeof(flags));
    recordDraw(r, text);
    return g_drawText(p, d, font, text, r, color, flags);
}
static int auditFill(xui_proxy p, xui_draw_context d, xui_rect_t r, uint32_t color)
{
    hashBytes(&r, sizeof(r)); hashBytes(&color, sizeof(color));
    recordDraw(r, "<fill>");
    return g_fill(p, d, r, color);
}
static void resetWork(void)
{
    memset(&g_work, 0, sizeof(g_work));
    g_shapeCalls = g_shapeBytes = 0;
}
static size_t privateBytes(xui_rich_edit_data_t* p)
{
    size_t bytes = sizeof(*p) + (size_t)p->iBlockCapacity * sizeof(*p->pBlocks) +
        (size_t)p->iFragmentCapacity * sizeof(*p->pFragments) +
        (size_t)p->iAtomCapacity * sizeof(*p->pAtoms) +
        (size_t)p->iWidgetCapacity * sizeof(*p->pWidgets) + (size_t)p->iScratchCapacity;
    int i;
    for (i = 0; i < p->iBlockCount; i++)
        bytes += (size_t)p->pBlocks[i].iFragmentCount * sizeof(*p->pFragments) +
            (size_t)p->pBlocks[i].iVisualLineCount * sizeof(xui_rich_line_index_t) +
            (size_t)p->pBlocks[i].iWidgetCount * sizeof(int);
    return bytes;
}

#define REQUIRE(e) do { if (!(e)) { printf("RichEdit scale failed at %d: %s\n", __LINE__, #e); failed = 1; goto cleanup; } } while (0)

static int compareView(xui_widget edit, xui_draw_context draw)
{
    xui_rich_edit_data_t* p = __xuiRichEditData(edit);
    xui_rect_t content = __xuiRichEditContentRect(edit, p);
    uint64_t hash;
    int failed = 0, i, j;
    g_trace = 0; g_recordCount = 0;
    REQUIRE(__xuiRichEditRender(edit, draw, 0, NULL) == XUI_OK);
    memcpy(g_expected, g_records, sizeof(g_records));
    j = g_recordCount; g_recordCount = 0;
    hash = g_trace; g_trace = 0;
    REQUIRE(auditReferenceRender(edit, draw, 0, NULL) == XUI_OK);
    if (hash != g_trace) {
        printf("draw mismatch: indexed=%d reference=%d scroll=%.4f\n", j, g_recordCount, p->fScrollY);
        for (i = 0; i < j && i < g_recordCount; i++) if (g_expected[i].hash != g_records[i].hash) {
            printf("draw %d indexed=(%d %d %d %d) %s\n", i,
                g_expected[i].rect.fX, g_expected[i].rect.fY, g_expected[i].rect.fW, g_expected[i].rect.fH, g_expected[i].text);
            printf("draw %d reference=(%d %d %d %d) %s\n", i,
                g_records[i].rect.fX, g_records[i].rect.fY, g_records[i].rect.fW, g_records[i].rect.fH, g_records[i].text);
            break;
        }
    }
    REQUIRE(hash == g_trace);
    for (i = 0; i < 4; i++) for (j = 0; j < 3; j++) {
        float x = content.fX - 10 + content.fW * j * 0.6f;
        float y = content.fY - 3 + content.fH * i * 0.34f;
        int s, e, rs, re;
        xui_rich_node node, other;
        REQUIRE(__xuiRichEditHit(edit, p, x, y) == auditReferenceHit(edit, p, x, y));
        REQUIRE(__xuiRichEditLinkAt(edit, p, x, y) == auditReferenceLinkAt(edit, p, x, y));
        s = e = rs = re = -1; node = other = NULL;
        REQUIRE(__xuiRichEditAtomicAt(edit, p, x, y, &s, &e, &node) ==
            auditReferenceAtomicAt(edit, p, x, y, &rs, &re, &other));
        REQUIRE(s == rs && e == re && node == other);
    }
cleanup:
    return failed;
}

static int scaleTest(int count, int columns)
{
    xui_test_proxy_state_t proxy;
    xui_context context = NULL;
    xui_widget edit = NULL, child = NULL;
    xui_font font = NULL;
    xui_surface target = NULL;
    xui_draw_context draw = NULL;
    xui_rich_document doc = NULL;
    xui_rich_edit_data_t* p;
    xui_rich_edit_desc_t desc = {0};
    xui_rich_text_style_t style = {0};
    xui_rich_node node;
    xui_rect_t content, r, rr;
    char* text = NULL;
    int failed = 0, i, j, length = count * (columns + 1) - 1;
    unsigned coldBlocks, coldShapes, widthBlocks, reads, blocks;
    size_t bytes;
    uint64_t hash;
    xuiTestProxyInit(&proxy);
    g_shape = proxy.tProxy.textShape; proxy.tProxy.textShape = auditShape;
    g_drawText = proxy.tProxy.drawText; proxy.tProxy.drawText = auditText;
    g_fill = proxy.tProxy.drawRectFill; proxy.tProxy.drawRectFill = auditFill;
    text = (char*)malloc((size_t)length + 1);
    REQUIRE(text != NULL);
    for (i = 0; i < count; i++) {
        for (j = 0; j < columns; j++) text[i * (columns + 1) + j] = "Rich content abc "[j % 17];
        if (i + 1 < count) text[i * (columns + 1) + columns] = '\n';
    }
    text[length] = 0;
    style.iSize = sizeof(style);
    REQUIRE(xuiRichDocumentCreate(&doc) == XUI_OK);
    REQUIRE(xuiRichDocumentReplace(doc, 0, 0, text, &style) == XUI_OK);
    REQUIRE(xuiCreate(&context) == XUI_OK);
    REQUIRE(xuiSetProxy(context, &proxy.tProxy) == XUI_OK);
    REQUIRE(proxy.tProxy.fontLoadFile(&proxy.tProxy, &font, "scale.ttf", 14, 0) == XUI_OK);
    REQUIRE(xuiSetDefaultFont(context, font) == XUI_OK);
    REQUIRE(xuiInputViewport(context, 960, 640) == XUI_OK);
    desc.iSize = sizeof(desc); desc.pDocument = doc; desc.pFont = font; desc.bWordWrap = 1;
    REQUIRE(xuiRichEditCreate(context, &edit, &desc) == XUI_OK);
    REQUIRE(xuiSetRootWidget(context, edit) == XUI_OK);
    REQUIRE(xuiWidgetSetRect(edit, (xui_rect_t){0, 0, 936, 616}) == XUI_OK);
    REQUIRE(xuiTestSurfaceCreate(&proxy, &target, 960, 640, XUI_SURFACE_USAGE_TARGET) == XUI_OK);
    REQUIRE(proxy.tProxy.drawBegin(&proxy.tProxy, &draw, target) == XUI_OK);
    p = __xuiRichEditData(edit);
    resetWork();
    REQUIRE(xuiLayout(context) == XUI_OK);
    REQUIRE(xuiUpdate(context, 0.016f) == XUI_OK);
    REQUIRE(__xuiRichEditRender(edit, draw, 0, NULL) == XUI_OK);
    coldBlocks = g_work.LayoutBlock; coldShapes = g_shapeCalls;
    REQUIRE(coldBlocks >= (unsigned)count && coldBlocks <= (unsigned)count * 3);
    REQUIRE(g_shapeBytes <= (unsigned)length * 3);
    REQUIRE(xuiRichEditGetFragmentCount(edit) == count * columns);
    bytes = privateBytes(p);
    /* One huge paragraph also retains power-of-two atom/fragment staging buffers. */
    REQUIRE(bytes <= (size_t)length * (count == 1 ? 384 : 160) + 1048576);
    for (i = 0; i < 3; i++) {
        REQUIRE(xuiRichEditSetScroll(edit, 0, p->fContentHeight * i * 0.45f) == XUI_OK);
        resetWork();
        REQUIRE(__xuiRichEditPrepareLayout(edit, p) == XUI_OK);
        REQUIRE(__xuiRichEditArrangeChildren(edit, (xui_rect_t){0}, NULL) == XUI_OK);
        REQUIRE(__xuiRichEditRender(edit, draw, 0, NULL) == XUI_OK);
        REQUIRE(g_work.LayoutBlock == 0 && g_work.IndexBlock == 0 && g_work.WidgetNode == 0);
        REQUIRE(g_work.SearchBlock <= 80 && g_work.SearchLine <= 64);
        /* 936x616 viewport: at most 46 lines of 134 proxy glyphs, plus run lookahead. */
        REQUIRE(g_shapeCalls == 0 && g_work.FragmentRead < 14000 && g_work.RenderBlock < 50);
        reads = g_work.FragmentRead; blocks = g_work.RenderBlock;
        if (i == 2) printf("RichEdit indexed N=%d cols=%d cold_layout=%u cold_shape=%u private_bytes=%zu warm_reads=%u warm_blocks=%u\n",
            count, columns, coldBlocks, coldShapes, bytes, reads, blocks);
        if (i == 2 && count == 10000) {
            resetWork();
            REQUIRE(auditReferenceRender(edit, draw, 0, NULL) == XUI_OK);
            REQUIRE(g_work.FragmentRead >= (unsigned)p->iTotalFragmentCount * 2);
            printf("RichEdit full-scan oracle N=10000 render_fragment_reads=%u\n", g_work.FragmentRead);
        }
        REQUIRE(compareView(edit, draw) == 0);
    }
    for (i = 0; i < 3; i++) {
        int offset = length * i / 2;
        r = __xuiRichEditCaret(edit, p, offset);
        rr = auditReferenceCaret(edit, p, offset);
        REQUIRE(memcmp(&r, &rr, sizeof(r)) == 0);
    }
    content = __xuiRichEditContentRect(edit, p);
    resetWork();
    REQUIRE(__xuiRichEditHit(edit, p, content.fX + 25, content.fY + 17) >= 0);
    REQUIRE(g_work.FragmentRead < 256 && g_work.SearchBlock < 32);
    REQUIRE(xuiRichEditSetSelection(edit, 5, 20) == XUI_OK);
    REQUIRE(xuiRichEditCopy(edit) == XUI_OK);
    REQUIRE(strlen(xuiTestProxyGetClipboardText(&proxy)) == 15);
    resetWork();
    REQUIRE(xuiWidgetSetRect(edit, (xui_rect_t){0, 0, 520, 616}) == XUI_OK);
    REQUIRE(xuiLayout(context) == XUI_OK);
    REQUIRE(__xuiRichEditRender(edit, draw, 0, NULL) == XUI_OK);
    widthBlocks = g_work.LayoutBlock;
    REQUIRE(widthBlocks >= (unsigned)count && widthBlocks <= (unsigned)count * 3);
    printf("RichEdit indexed N=%d width_switch_layout=%u shape=%u private_bytes=%zu\n",
        count, widthBlocks, g_shapeCalls, privateBytes(p));
    REQUIRE(compareView(edit, draw) == 0);
    REQUIRE(xuiRichEditSetSelection(edit, length / 2, length / 2) == XUI_OK);
    REQUIRE(xuiRichEditInsertText(edit, "change") == XUI_OK);
    REQUIRE(compareView(edit, draw) == 0);
    REQUIRE(xuiRichEditUndo(edit) == XUI_OK);
    REQUIRE(xuiRichEditRedo(edit) == XUI_OK);
    REQUIRE(compareView(edit, draw) == 0);
    REQUIRE(xuiRichEditSetSelection(edit, columns - 2, columns + 3) == XUI_OK);
    REQUIRE(xuiRichEditInsertText(edit, "\nsplit\n") == XUI_OK);
    REQUIRE(compareView(edit, draw) == 0);
    if (count == 100) {
        xui_rich_paragraph_style_t para = {0};
        REQUIRE(xuiWidgetCreate(context, &child) == XUI_OK);
        node = xuiRichNodeGetFirstChild(xuiRichDocumentGetRoot(doc));
        REQUIRE(xuiRichDocumentAppendWidget(doc, node, child, 80, 130, 17) != NULL);
        style.fFontSize = 35; style.iFlags = XUI_RICH_STYLE_SUPERSCRIPT;
        REQUIRE(xuiRichDocumentAppendLink(doc, node, "varied", "https://example.test", &style) != NULL);
        para.iSize = sizeof(para); para.fSpaceBefore = 8; para.fSpaceAfter = 3;
        REQUIRE(xuiRichNodeSetParagraphStyle(doc, node, &para) == XUI_OK);
        REQUIRE(xuiLayout(context) == XUI_OK);
        REQUIRE(xuiWidgetGetParent(child) == edit);
        REQUIRE(xuiRichEditSetScroll(edit, 0, 0) == XUI_OK);
        REQUIRE(compareView(edit, draw) == 0);
        REQUIRE(xuiRichEditSetScroll(edit, 0, p->fContentHeight * 0.8f) == XUI_OK);
        resetWork();
        REQUIRE(xuiLayout(context) == XUI_OK);
        REQUIRE(g_work.ArrangeWidget == 1 && g_work.FragmentRead == 1);
        REQUIRE(xuiWidgetGetParent(child) == edit);
        REQUIRE(xuiWidgetRemoveFromParent(child) == XUI_OK);
        REQUIRE(__xuiRichEditPrepareLayout(edit, p) == XUI_OK && xuiWidgetGetParent(child) == edit);
        g_failAllocation = 0; p->bLayoutDirty = 1; p->bIncrementalLayout = 0;
        REQUIRE(__xuiRichEditEnsureLayout(edit, p) == XUI_ERROR_OUT_OF_MEMORY);
        REQUIRE(p->bLayoutDirty);
        g_failAllocation = -1;
        REQUIRE(__xuiRichEditEnsureLayout(edit, p) == XUI_OK);
        REQUIRE(compareView(edit, draw) == 0);
    }
    g_trace = 0; REQUIRE(__xuiRichEditRender(edit, draw, 0, NULL) == XUI_OK); hash = g_trace;
    g_trace = 0; REQUIRE(auditReferenceRender(edit, draw, 0, NULL) == XUI_OK); REQUIRE(hash == g_trace);
cleanup:
    g_failAllocation = -1;
    if (draw != NULL) proxy.tProxy.drawEnd(&proxy.tProxy, draw);
    if (context != NULL) xuiDestroy(context);
    if (target != NULL) proxy.tProxy.surfaceDestroy(&proxy.tProxy, target);
    if (doc != NULL) xuiRichDocumentDestroy(doc);
    if (font != NULL) proxy.tProxy.fontDestroy(&proxy.tProxy, font);
    free(text);
    return failed;
}

#define main richLegacyMain
#include "xui_rich_edit_test.c"
#undef main

int main(int argc, char** argv)
{
    if (argc > 1) return scaleTest(atoi(argv[1]), argc > 2 ? atoi(argv[2]) : 96);
    if (scaleTest(100, 96) || scaleTest(1000, 96) || scaleTest(10000, 96) || scaleTest(1, 100000)) return 1;
    if (richLegacyMain()) return 1;
    puts("RichEdit visible-range scale regression passed");
    return 0;
}
