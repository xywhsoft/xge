#include "xui_test_proxy.h"
#include "../src/xui_text_edit.c"
#include <math.h>

static int failures, checks, shapes, draws, failAllocation = -1;
static char drawn[16][128];
static xui_text_shape_proc originalShape;
static xui_draw_text_proc originalDraw;
void* __real_xrtMalloc(size_t size);
void* __wrap_xrtMalloc(size_t size)
{
    if (failAllocation >= 0 && failAllocation-- == 0) return NULL;
    return __real_xrtMalloc(size);
}
#define CHECK(e) do { checks++; if (!(e)) { failures++; printf("FAIL %d: %s\n", __LINE__, #e); } } while (0)

static int shape(xui_proxy proxy, xui_font font, const char* text, int size, uint32_t flags, xui_text_shape_t* out)
{
    int ret, i, count = 0;
    shapes++;
    ret = originalShape(proxy, font, text, size, flags, out);
    if (ret != XUI_OK) return ret;
    for (i = 0; i < out->iClusterCount; i++) {
        xui_text_cluster_t cluster = out->pClusters[i];
        if (i + 2 < out->iClusterCount && cluster.iTextStart + 3 <= size &&
            memcmp(text + cluster.iTextStart, "ffi", 3) == 0) {
            cluster.iTextEnd = out->pClusters[i + 2].iTextEnd;
            cluster.fAdvance = 15;
            i += 2;
        }
        out->pClusters[count++] = cluster;
    }
    out->iClusterCount = count;
    out->fWidth = 0;
    for (i = 0; i < count; i++) out->fWidth += out->pClusters[i].fAdvance;
    return XUI_OK;
}
static int draw_text(xui_proxy proxy, xui_draw_context draw, xui_font font,
    const char* text, xui_rect_t rect, uint32_t color, uint32_t flags)
{
    if (draws < 16) snprintf(drawn[draws], sizeof(drawn[draws]), "%s", text);
    draws++;
    return originalDraw(proxy, draw, font, text, rect, color, flags);
}

static void run_case(const char* raw, int width, int wrap, const char* first,
    const char* second, float firstWidth, int joined)
{
    xui_test_proxy_state_t proxy;
    xui_context context = NULL;
    xui_widget edit = NULL;
    xui_font font = NULL;
    xui_surface surface = NULL;
    xui_draw_context draw = NULL;
    xui_text_edit_desc_t desc = {0};
    xui_text_edit_data_t* data;
    xui_text_layout normal;
    xui_rect_t content;
    int i, before, next, length = (int)strlen(raw);
    xui_vec2_t measured;
    xuiTestProxyInit(&proxy);
    originalShape = proxy.tProxy.textShape; proxy.tProxy.textShape = shape;
    originalDraw = proxy.tProxy.drawText; proxy.tProxy.drawText = draw_text;
    CHECK(xuiCreate(&context) == XUI_OK);
    CHECK(xuiSetProxy(context, &proxy.tProxy) == XUI_OK);
    CHECK(proxy.tProxy.fontLoadFile(&proxy.tProxy, &font, "display.ttf", 20, 0) == XUI_OK);
    CHECK(xuiSetDefaultFont(context, font) == XUI_OK);
    CHECK(xuiSetViewportSize(context, width, 160) == XUI_OK);
    desc.iSize = sizeof(desc); desc.pFont = font; desc.sText = raw; desc.bWordWrap = wrap;
    CHECK(xuiTextEditCreate(context, &edit, &desc) == XUI_OK);
    CHECK(xuiWidgetSetPadding(edit, (xui_thickness_t){0, 0, 0, 0}) == XUI_OK);
    CHECK(xuiSetRootWidget(context, edit) == XUI_OK);
    CHECK(xuiLayout(context) == XUI_OK);
    CHECK(xuiTextEditGetLineCount(edit) == (second != NULL ? 2 : 1));
    data = __xuiTextEditGetData(edit);
    CHECK(data != NULL && data->tTextLayout.pLayout != NULL);
    if (data == NULL || data->pLines == NULL) goto cleanup;
    CHECK(data->iLineCount == (second != NULL ? 2 : 1));
    CHECK(fabsf(data->pLines[0].fW - firstWidth) < 0.001f);
    CHECK(strcmp(xuiTextEditGetText(edit), raw) == 0);
    CHECK(xuiTestSurfaceCreate(&proxy, &surface, width, 160, XUI_SURFACE_USAGE_TARGET) == XUI_OK);
    CHECK(proxy.tProxy.drawBegin(&proxy.tProxy, &draw, surface) == XUI_OK);
    draws = 0;
    CHECK(__xuiTextEditCacheRender(edit, draw, 0, NULL) == XUI_OK);
    CHECK(draws == (second != NULL ? 2 : 1));
    CHECK(strcmp(drawn[0], first) == 0);
    if (second) CHECK(strcmp(drawn[1], second) == 0);
    measured = __xuiTextEditMeasureRange(edit, data, font, 0, data->pLines[0].iEnd);
    CHECK(measured.fX == firstWidth);
    content = __xuiTextEditTextContentRect(edit, data);
    before = shapes;
    for (i = 0; i < 32; i++) {
        CHECK(__xuiTextEditCursorFromPoint(edit, data, content.fX + firstWidth + 1,
            content.fY + data->fLineHeight * 0.5f) == data->pLines[0].iEnd);
    }
    CHECK(shapes == before);
    if (joined == 1) {
        CHECK(xuiInternalTextLayoutNextCaret(data->tTextLayout.pLayout, 1, length, &next) == XUI_OK);
        CHECK(next == length - 1);
        for (i = 0; i < 35; i++) {
            int offset = __xuiTextEditCursorFromPoint(edit, data, content.fX + i,
                content.fY + data->fLineHeight * 0.5f);
            CHECK(offset == 0 || offset == 1 || offset == length - 1 || offset == length);
        }
    }
    if (joined == 2) {
        for (i = 1; i < 20; i += 8) {
            int start, end;
            int offset = __xuiTextEditOffsetFromPoint(edit, data, content.fX + i,
                content.fY + data->fLineHeight * 0.5f, 1);
            CHECK(__xuiTextEditSelectWordAt(edit, data, offset) == XUI_OK);
            CHECK(xuiTextEditGetSelection(edit, &start, &end) == XUI_OK);
            CHECK(start == (i < 10 ? 0 : 3) && end == (i < 10 ? 3 : 6));
        }
    }
    if (!second && strcmp(raw, "ab\xC2\xAD" "cd") == 0) {
        measured = __xuiTextEditMeasureRange(edit, data, font, 2, 4);
        CHECK(measured.fX == 0.0f);
        CHECK(xuiTextEditSetSelection(edit, 2, 4) == XUI_OK);
        CHECK(xuiTextEditCopy(edit) == XUI_OK);
        CHECK(strcmp(xuiTestProxyGetClipboardText(&proxy), "\xC2\xAD") == 0);
    }
    CHECK(xuiTextEditSetSelection(edit, 0, length) == XUI_OK);
    CHECK(xuiTextEditCopy(edit) == XUI_OK);
    CHECK(strcmp(xuiTestProxyGetClipboardText(&proxy), raw) == 0);
    CHECK(xuiTextEditSetSelection(edit, length, length) == XUI_OK);
    CHECK(__xuiTextEditInsertText(edit, data, "!", 1) == XUI_OK);
    CHECK(xuiTextEditUndo(edit) == XUI_OK);
    CHECK(strcmp(xuiTextEditGetText(edit), raw) == 0);
    CHECK(xuiLayout(context) == XUI_OK);
    normal = data->tTextLayout.pLayout;
    failAllocation = 0;
    CHECK(__xuiTextEditBuildLines(edit, data, font, (float)width + 1) == XUI_ERROR_OUT_OF_MEMORY);
    failAllocation = -1;
    CHECK(data->tTextLayout.pLayout == normal && data->bLinesDirty);
    CHECK(__xuiTextEditBuildLines(edit, data, font, (float)width) == XUI_OK);
    CHECK(xuiSetVirtualDpi(context, 1.5f) == XUI_OK);
    CHECK(xuiLayout(context) == XUI_OK);
    CHECK(xuiTextEditGetLineCount(edit) > 0);
    CHECK(data->tTextLayout.iDpiGeneration == context->iDpiGeneration);
    CHECK(xuiSetFocusWidget(context, edit) == XUI_OK);
    CHECK(xuiTextEditSetSelection(edit, length, length) == XUI_OK);
    normal = data->tTextLayout.pLayout;
    CHECK(xuiInputImeComposition(context, "x\xC2\xAD" "y", -1, 0, 4) == XUI_OK);
    CHECK(xuiDispatchPendingEvents(context) == XUI_OK);
    CHECK(data->bImeActive);
    draws = 0;
    CHECK(__xuiTextEditCacheRender(edit, draw, 0, NULL) == XUI_OK);
    CHECK(data->tTextLayout.pLayout == normal);
    CHECK(data->tImeTextLayout.pLayout != NULL && data->tImeTextLayout.pLayout != normal);
    CHECK(strcmp(xuiTextLayoutGetText(normal), raw) == 0);
    CHECK(strcmp(xuiTextEditGetText(edit), raw) == 0);
    CHECK(__xuiTextEditImeRect(edit, NULL).fH > 0);
    CHECK(data->tTextLayout.pLayout == normal);
    CHECK(proxy.tProxy.drawEnd(&proxy.tProxy, draw) == XUI_OK);
cleanup:
    xuiDestroy(context);
    proxy.tProxy.surfaceDestroy(&proxy.tProxy, surface);
    proxy.tProxy.fontDestroy(&proxy.tProxy, font);
}
static void fontless_case(void)
{
    static const char raw[] = "ab\xC2\xAD" "cd\r\nE\rF\nG\vH\fI\xC2\x85" "J\xE2\x80\xA8" "K\xE2\x80\xA9";
    xui_test_proxy_state_t proxy;
    xui_context context = NULL;
    xui_widget edit = NULL;
    xui_font font = NULL;
    xui_text_edit_desc_t desc = {0};
    xui_text_edit_data_t* data;
    int start, end, i, offsets[9][2];
    xuiTestProxyInit(&proxy);
    CHECK(xuiCreate(&context) == XUI_OK);
    CHECK(xuiSetProxy(context, &proxy.tProxy) == XUI_OK);
    CHECK(xuiSetViewportSize(context, 100, 300) == XUI_OK);
    desc.iSize = sizeof(desc); desc.sText = raw; desc.bWordWrap = 1;
    CHECK(xuiTextEditCreate(context, &edit, &desc) == XUI_OK);
    CHECK(xuiSetRootWidget(context, edit) == XUI_OK);
    CHECK(xuiLayout(context) == XUI_OK);
    CHECK(xuiTextEditGetLineCount(edit) == 9);
    CHECK(xuiTextEditSetSelection(edit, 2, 4) == XUI_OK);
    CHECK(xuiTextEditGetSelection(edit, &start, &end) == XUI_OK && start == 2 && end == 4);
    CHECK(xuiTextEditCopy(edit) == XUI_OK);
    CHECK(strcmp(xuiTestProxyGetClipboardText(&proxy), "\xC2\xAD") == 0);
    data = __xuiTextEditGetData(edit);
    CHECK(data->tTextLayout.pLayout == NULL && data->fContentWidth == 0);
    for (i = 0; i < 9; i++) {
        offsets[i][0] = data->pLines[i].iStart;
        offsets[i][1] = data->pLines[i].iEnd;
    }
    CHECK(proxy.tProxy.fontLoadFile(&proxy.tProxy, &font, "display.ttf", 20, 0) == XUI_OK);
    CHECK(xuiTextEditSetFont(edit, font) == XUI_OK);
    CHECK(xuiTextEditGetLineCount(edit) == 9);
    CHECK(data->tTextLayout.pLayout != NULL && data->pLines[0].fW == 40);
    for (i = 0; i < 9; i++) {
        CHECK(data->pLines[i].iStart == offsets[i][0] && data->pLines[i].iEnd == offsets[i][1]);
    }
    CHECK(xuiTextEditSetFont(edit, NULL) == XUI_OK);
    CHECK(xuiTextEditGetLineCount(edit) == 9);
    CHECK(data->tTextLayout.pLayout == NULL && data->fContentWidth == 0);
    CHECK(xuiTextEditGetSelection(edit, &start, &end) == XUI_OK && start == 2 && end == 4);
    CHECK(strcmp(xuiTextEditGetText(edit), raw) == 0);
    xuiDestroy(context);
    proxy.tProxy.fontDestroy(&proxy.tProxy, font);
}
int main(void)
{
    run_case("ab\xC2\xAD" "cd", 30, 1, "ab-", "cd", 30, 0);
    run_case("ab\xC2\xAD" "cd", 100, 1, "abcd", NULL, 40, 0);
    run_case("A\xE2\x80\x8B" "B\xE2\x81\xA0" "C\xEF\xBB\xBF" "D", 100, 1, "ABCD", NULL, 40, 0);
    run_case("A\xE2\x80\x8B" "B\xC2\xAD" "C", 100, 0, "ABC", NULL, 30, 0);
    run_case("Af\xC2\xAD" "fiB", 100, 1, "AffiB", NULL, 35, 1);
    run_case("Af\xE2\x81\xA0" "fiB", 100, 0, "AffiB", NULL, 35, 1);
    run_case("a\r\nb", 100, 1, "a", "b", 10, 0);
    run_case("a\n", 100, 1, "a", "", 10, 0);
    run_case("\xE4\xBD\xA0\xE5\xA5\xBD", 100, 1, "\xE4\xBD\xA0\xE5\xA5\xBD", NULL, 20, 2);
    fontless_case();
    printf("TextEdit display: %d checks, %d failures\n", checks, failures);
    return failures != 0;
}
