#include "xui.h"
#include "xui_test_proxy.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

static int checks, failures, selections, lastIndex;
#define CHECK(e) do { ++checks; if (!(e)) { if (failures < 16) { printf("FAIL %d: %s\n", __LINE__, #e); } ++failures; } } while (0)
static xui_rect_t pixels(float x, float y, float w, float h)
{
    int l = (int)floorf(x + .5f), t = (int)floorf(y + .5f);
    int r = (int)floorf(x + w + .5f), b = (int)floorf(y + h + .5f);
    return (xui_rect_t){l, t, r > l ? r - l : 0, b > t ? b - t : 0};
}
static int same(xui_rect_t a, xui_rect_t b)
{
    return a.fX == b.fX && a.fY == b.fY && a.fW == b.fW && a.fH == b.fH;
}
static xui_rect_t intersect(xui_rect_t a, xui_rect_t b)
{
    int l = a.fX > b.fX ? a.fX : b.fX, t = a.fY > b.fY ? a.fY : b.fY;
    int r = a.fX + a.fW < b.fX + b.fW ? a.fX + a.fW : b.fX + b.fW;
    int bottom = a.fY + a.fH < b.fY + b.fH ? a.fY + a.fH : b.fY + b.fH;
    return (xui_rect_t){l, t, r > l ? r - l : 0, bottom > t ? bottom - t : 0};
}
static const char *display[] = {"File", "E&xit", "View", "Tools"};
static xui_font wideFont;
static xui_rect_t textSeen[4], srcSeen, dstSeen;
static int textCalls, copies;
static xui_surface tracked;
static xui_draw_text_proc textOriginal;
static xui_surface_draw_to_proc copyOriginal;
static float text_width(xui_font font, const char *s)
{
    return strlen(s) * (font == wideFont ? 7.25f : 5.25f) + .25f;
}
static int measure(xui_proxy p, xui_font font, const char *s, xui_vec2_t *size)
{
    (void)p; size->fX = text_width(font, s); size->fY = 13.5f; return XUI_OK;
}
static int text_capture(xui_proxy p, xui_draw_context draw, xui_font font,
    const char *s, xui_rect_t r, uint32_t color, uint32_t flags)
{
    int i;
    for (i = 0; i < 4; ++i) if (strcmp(s, display[i]) == 0) { textSeen[i] = r; ++textCalls; }
    return textOriginal(p, draw, font, s, r, color, flags);
}
static int copy_capture(xui_proxy p, xui_surface target, xui_surface source,
    xui_rect_t src, xui_rect_t dst, uint32_t color, uint32_t flags)
{
    if (source == tracked) { srcSeen = src; dstSeen = dst; ++copies; }
    return copyOriginal(p, target, source, src, dst, color, flags);
}
static void selected(xui_widget bar, int index, int value, void *user)
{
    (void)bar; (void)user; ++selections; lastIndex = index; CHECK(value == 100 + index);
}
static void verify(xui_context c, xui_widget bar, const xui_menubar_metrics_t *m,
    xui_font font, int h)
{
    xui_rect_t expected[4];
    xui_vec2_t size;
    float x = m->fPaddingX;
    int i, before = textCalls;
    CHECK(xuiLayout(c) == XUI_OK);
    CHECK(xuiRenderPrepare(c) == XUI_OK);
    CHECK(textCalls >= before + 4);
    for (i = 0; i < 4; ++i) {
        float w = fmaxf(20, text_width(font, display[i]) + 2 * m->fItemPaddingX);
        expected[i] = pixels(x, m->fPaddingY, w, fmaxf(1, h - 2 * m->fPaddingY));
        CHECK(same(xuiMenuBarGetItemRect(bar, i), expected[i]));
        CHECK(same(textSeen[i], pixels(expected[i].fX + m->fItemPaddingX, expected[i].fY,
            expected[i].fW - 2 * m->fItemPaddingX, expected[i].fH)));
        if (i > 0 && m->fItemGap == 0) CHECK(expected[i-1].fX + expected[i-1].fW == expected[i].fX);
        x += w + m->fItemGap;
    }
    CHECK(xuiWidgetMeasureContent(bar, (xui_vec2_t){XUI_LAYOUT_UNBOUNDED, XUI_LAYOUT_UNBOUNDED}, &size) == XUI_OK);
    CHECK(size.fX == floorf(x + m->fPaddingX - m->fItemGap + .5f));
    CHECK(size.fY == floorf(m->fHeight + .5f));
}
static void geometry(int odd, int offset, float gap)
{
    xui_test_proxy_state_t p;
    xui_context c = NULL;
    xui_widget root, parent, bar;
    xui_font font = NULL;
    xui_surface target = NULL;
    xui_menubar_metrics_t m, got;
    xui_menubar_item_t items[4];
    xui_rect_t r, world, clipped;
    int i, x, y, before;
    xuiTestProxyInit(&p);
    textOriginal = p.tProxy.drawText; p.tProxy.drawText = text_capture;
    copyOriginal = p.tProxy.surfaceDrawTo; p.tProxy.surfaceDrawTo = copy_capture;
    p.tProxy.textMeasure = measure;
    textCalls = 0; memset(textSeen, 0, sizeof(textSeen));
    CHECK(xuiCreate(&c) == XUI_OK);
    CHECK(xuiSetProxy(c, &p.tProxy) == XUI_OK);
    CHECK(xuiSetViewportSize(c, 400, 300) == XUI_OK);
    CHECK(p.tProxy.fontLoadMemory(&p.tProxy, &font, "body", 4, 14, XUI_FONT_FORMAT_TTF) == XUI_OK);
    CHECK(p.tProxy.fontLoadMemory(&p.tProxy, &wideFont, "body", 4, 18, XUI_FONT_FORMAT_TTF) == XUI_OK);
    CHECK(xuiWidgetCreate(c, &root) == XUI_OK);
    CHECK(xuiSetRootWidget(c, root) == XUI_OK);
    CHECK(xuiWidgetSetLayoutType(root, XUI_LAYOUT_MANUAL) == XUI_OK);
    CHECK(xuiWidgetCreate(c, &parent) == XUI_OK);
    CHECK(xuiWidgetAddChild(root, parent) == XUI_OK);
    CHECK(xuiWidgetSetLayoutType(parent, XUI_LAYOUT_MANUAL) == XUI_OK);
    CHECK(xuiWidgetSetOverflow(parent, XUI_OVERFLOW_HIDDEN) == XUI_OK);
    CHECK(xuiWidgetSetRect(parent, (xui_rect_t){offset, offset, 119, 48}) == XUI_OK);
    CHECK(xuiMenuBarCreate(c, &bar, NULL) == XUI_OK);
    CHECK(xuiWidgetAddChild(parent, bar) == XUI_OK);
    CHECK(xuiWidgetSetRect(bar, (xui_rect_t){5, 7, 120 + odd, 30 + odd}) == XUI_OK);
    CHECK(xuiMenuBarSetFont(bar, font) == XUI_OK);
    memset(items, 0, sizeof(items));
    for (i = 0; i < 4; ++i) { items[i].sText = display[i]; items[i].iState = XUI_MENUBAR_ITEM_ENABLED; items[i].iValue = 100 + i; }
    items[0].sText = "&File"; items[1].sText = "E&&xit"; items[2].sText = "&View"; items[2].iState = 0;
    CHECK(xuiMenuBarSetItems(bar, items, 4) == XUI_OK);
    CHECK(xuiMenuBarSetSelect(bar, selected, NULL) == XUI_OK);
    memset(&m, 0, sizeof(m)); m.iSize = sizeof(m);
    CHECK(xuiMenuBarGetMetrics(bar, &m) == XUI_OK);
    m.fHeight = 29.5f; m.fPaddingX = 2.5f; m.fPaddingY = 3.75f;
    m.fItemPaddingX = 6.75f; m.fItemGap = gap;
    CHECK(xuiMenuBarSetMetrics(bar, &m) == XUI_OK);
    verify(c, bar, &m, font, 30 + odd);
    world = xuiWidgetGetWorldRect(bar);
    r = xuiMenuBarGetItemRect(bar, 1);
    x = world.fX + r.fX; y = world.fY + r.fY + r.fH / 2;
    CHECK(xuiInputPointerMove(c, x, y, 0) == XUI_OK);
    CHECK(xuiDispatchPendingEvents(c) == XUI_OK);
    CHECK(xuiMenuBarGetHoverIndex(bar) == 1);
    before = selections;
    CHECK(xuiInputPointerDown(c, x, y, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT) == XUI_OK);
    CHECK(xuiDispatchPendingEvents(c) == XUI_OK);
    CHECK(xuiInputPointerUp(c, x, y, XUI_POINTER_BUTTON_LEFT, 0) == XUI_OK);
    CHECK(xuiDispatchPendingEvents(c) == XUI_OK);
    CHECK(selections == before + 1 && lastIndex == 1);
    r = xuiMenuBarGetItemRect(bar, 2);
    CHECK(xuiInputPointerMove(c, world.fX + r.fX, y, 0) == XUI_OK);
    CHECK(xuiDispatchPendingEvents(c) == XUI_OK);
    CHECK(xuiMenuBarGetHoverIndex(bar) == -1);
    CHECK(selections == before + 1);
    CHECK(xuiHitTest(c, world.fX + world.fW - 1, y, XUI_WIDGET_HIT_DEFAULT) != bar);
    CHECK(xuiTestSurfaceCreate(&p, &target, 400, 300, XUI_SURFACE_USAGE_TARGET) == XUI_OK);
    CHECK(xuiRenderPrepare(c) == XUI_OK);
    tracked = xuiWidgetGetCacheSurface(bar, xuiWidgetGetStateId(bar));
    CHECK(tracked != NULL);
    clipped = intersect(intersect(world, xuiWidgetGetWorldRect(parent)), (xui_rect_t){0, 0, 400, 300});
    copies = 0;
    CHECK(xuiRender(c, target, NULL, 0) == XUI_OK);
    CHECK(copies > 0 && same(dstSeen, clipped));
    CHECK(same(srcSeen, (xui_rect_t){clipped.fX - world.fX, clipped.fY - world.fY, clipped.fW, clipped.fH}));
    m.fPaddingX = 3.75f; m.fPaddingY = 2.5f; m.fItemPaddingX = 7.5f;
    CHECK(xuiMenuBarSetMetrics(bar, &m) == XUI_OK);
    memset(&got, 0, sizeof(got)); got.iSize = sizeof(got);
    CHECK(xuiMenuBarGetMetrics(bar, &got) == XUI_OK);
    CHECK(got.fPaddingX == 3.75f && got.fPaddingY == 2.5f && got.fItemPaddingX == 7.5f);
    verify(c, bar, &m, font, 30 + odd);
    CHECK(xuiMenuBarSetFont(bar, wideFont) == XUI_OK);
    verify(c, bar, &m, wideFont, 30 + odd);
    items[0].sText = "&Tools"; display[0] = "Tools"; items[3].sText = "&File"; display[3] = "File";
    CHECK(xuiMenuBarSetItems(bar, items, 4) == XUI_OK);
    verify(c, bar, &m, wideFont, 30 + odd);
    CHECK(xuiMenuBarSetItemEnabled(bar, 2, 1) == XUI_OK);
    CHECK(xuiMenuBarIsItemEnabled(bar, 2));
    CHECK(xuiMenuBarClear(bar) == XUI_OK);
    CHECK(xuiMenuBarGetItemCount(bar) == 0);
    CHECK(same(xuiMenuBarGetItemRect(bar, 0), (xui_rect_t){0, 0, 0, 0}));
    tracked = NULL;
    xuiDestroy(c);
    p.tProxy.surfaceDestroy(&p.tProxy, target);
    p.tProxy.fontDestroy(&p.tProxy, font); p.tProxy.fontDestroy(&p.tProxy, wideFont);
    wideFont = NULL; display[0] = "File"; display[3] = "Tools";
}
int main(void)
{
    int odd, offset, gap;
    for (odd = 0; odd <= 1; ++odd)
    for (offset = -17; offset <= 17; offset += 34)
    for (gap = 0; gap < 2; ++gap) geometry(odd, offset, gap ? .75f : 0);
    printf("xui_menubar_pixel_test: %d checks, %d failures, %d selection callbacks\n", checks, failures, selections);
    return failures != 0;
}
