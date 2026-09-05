#include "xui.h"
#include "xui_test_proxy.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

static int checks, failures, selections, overflowSelections, lastFirst, lastCount;
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
static xui_surface icons[2], tracked;
static xui_rect_t iconSeen[2], textSeen, separatorSeen, dotSeen[3], srcSeen, dstSeen;
static int iconCalls, dotCalls, copies;
static uint32_t separatorColor, dotColor;
static xui_draw_surface_proc iconOriginal;
static xui_draw_rect_fill_proc fillOriginal;
static xui_draw_text_proc textOriginal;
static xui_surface_draw_to_proc copyOriginal;
static int measure(xui_proxy p, xui_font font, const char *s, xui_vec2_t *size)
{
    (void)p; (void)font; size->fX = strlen(s) * 5.25f + .25f; size->fY = 13.5f; return XUI_OK;
}
static int icon_capture(xui_proxy p, xui_draw_context draw, xui_surface source,
    xui_rect_t src, xui_rect_t dst, uint32_t color, uint32_t flags)
{
    int i;
    for (i = 0; i < 2; ++i) if (source == icons[i]) { iconSeen[i] = dst; ++iconCalls; }
    return iconOriginal(p, draw, source, src, dst, color, flags);
}
static int text_capture(xui_proxy p, xui_draw_context draw, xui_font font,
    const char *s, xui_rect_t r, uint32_t color, uint32_t flags)
{
    if (strcmp(s, "Go") == 0) textSeen = r;
    return textOriginal(p, draw, font, s, r, color, flags);
}
static int fill_capture(xui_proxy p, xui_draw_context draw, xui_rect_t r, uint32_t color)
{
    if (color == separatorColor) separatorSeen = r;
    if (color == dotColor && r.fW == 2 && r.fH == 2) { dotSeen[dotCalls % 3] = r; ++dotCalls; }
    return fillOriginal(p, draw, r, color);
}
static int copy_capture(xui_proxy p, xui_surface target, xui_surface source,
    xui_rect_t src, xui_rect_t dst, uint32_t color, uint32_t flags)
{
    if (source == tracked) { srcSeen = src; dstSeen = dst; ++copies; }
    return copyOriginal(p, target, source, src, dst, color, flags);
}
static void selected(xui_widget bar, int index, int value, void *user)
{
    (void)bar; (void)user; ++selections; CHECK(index == 1 && value == 101);
}
static void overflow_selected(xui_widget bar, int first, int count, void *user)
{
    (void)bar; (void)user; ++overflowSelections; lastFirst = first; lastCount = count;
}
static void verify_layout(xui_widget bar, const xui_toolbar_metrics_t *m, int w, int h, int overflow)
{
    float x = m->fPaddingX, y = m->fPaddingY;
    float iw = fmaxf(0, w - 2 * x), ih = fmaxf(0, h - 2 * y);
    int vertical = m->iOrientation == XUI_ORIENTATION_VERTICAL;
    float cursor = vertical ? y : x, end = vertical ? y + ih : x + iw;
    float axis = 2 * m->fPaddingX, cross = 2 * m->fPaddingY + (vertical ? m->fItemWidth : m->fItemHeight);
    int i, prev = 0, first = -1;
    xui_rect_t expected, actual;
    xui_vec2_t size;
    for (i = 0; i < 6; ++i) {
        const xui_toolbar_item_t *item = xuiToolbarGetItem(bar, i);
        float extent = item->iType == XUI_TOOLBAR_ITEM_SEPARATOR ? m->fSeparatorSize :
            (vertical ? m->fItemHeight : m->fItemWidth);
        if (i > 0 && item->iType != XUI_TOOLBAR_ITEM_SEPARATOR && item->iGroup != prev) {
            cursor += m->fGroupGap; axis += m->fGroupGap;
        }
        axis += extent;
        if (overflow && first < 0 && cursor + extent > end - m->fOverflowSize) first = i;
        if (first < 0) {
            float visible = fminf(extent, fmaxf(0, end - cursor));
            expected = vertical ? pixels(x, cursor, iw, visible) : pixels(cursor, y, visible, ih);
        } else expected = (xui_rect_t){0, 0, 0, 0};
        actual = xuiToolbarGetItemRect(bar, i);
        CHECK(same(actual, expected));
        CHECK(actual.fW >= 0 && actual.fH >= 0);
        if (expected.fW > 0 && expected.fH > 0) {
            int hit = item->iType != XUI_TOOLBAR_ITEM_SEPARATOR && (item->iState & XUI_TOOLBAR_ITEM_ENABLED) ? i : -1;
            CHECK(xuiToolbarGetItemAt(bar, expected.fX, expected.fY) == hit);
            CHECK(xuiToolbarGetItemAt(bar, expected.fX + expected.fW - 1, expected.fY + expected.fH - 1) == hit);
        }
        cursor += extent;
        if (item->iType != XUI_TOOLBAR_ITEM_SEPARATOR) prev = item->iGroup;
    }
    CHECK(xuiToolbarGetOverflowFirst(bar) == first);
    CHECK(xuiToolbarGetOverflowCount(bar) == (first < 0 ? 0 : 6 - first));
    expected = first < 0 ? (xui_rect_t){0, 0, 0, 0} : (vertical ?
        pixels(x, y + ih - m->fOverflowSize, iw, m->fOverflowSize) :
        pixels(x + iw - m->fOverflowSize, y, m->fOverflowSize, ih));
    CHECK(same(xuiToolbarGetOverflowRect(bar), expected));
    CHECK(xuiWidgetMeasureContent(bar, (xui_vec2_t){XUI_LAYOUT_UNBOUNDED, XUI_LAYOUT_UNBOUNDED}, &size) == XUI_OK);
    CHECK(size.fX == floorf((vertical ? cross : axis) + .5f));
    CHECK(size.fY == floorf((vertical ? axis : cross) + .5f));
}
static void verify_draw(xui_context c, xui_widget bar, const xui_toolbar_metrics_t *m, int pressed)
{
    int i, before = iconCalls, vertical = m->iOrientation == XUI_ORIENTATION_VERTICAL;
    xui_rect_t r, expected;
    CHECK(xuiLayout(c) == XUI_OK);
    CHECK(xuiRenderPrepare(c) == XUI_OK);
    CHECK(iconCalls >= before + 2);
    for (i = 0; i < 2; ++i) {
        float ix, iy, is, tw, group, tx;
        r = xuiToolbarGetItemRect(bar, i);
        is = fminf(m->fIconSize, r.fH); tw = fmaxf(0, r.fW - 12);
        group = i == 0 ? fminf(tw, is + m->fIconGap + 10.75f) : is;
        ix = r.fX + 6 + (tw - group) * .5f;
        iy = r.fY + (r.fH - is) * .5f + (pressed == i);
        CHECK(same(iconSeen[i], pixels(ix, iy, is, is)));
        if (i == 0) {
            tx = ix + is + m->fIconGap;
            CHECK(same(textSeen, pixels(tx, r.fY + (pressed == i), r.fX + r.fW - 6 - tx, r.fH)));
        }
    }
    r = xuiToolbarGetItemRect(bar, 2);
    expected = vertical ? pixels(r.fX + 6, r.fY + (r.fH - 1) * .5f, r.fW - 12, 1) :
        pixels(r.fX + (r.fW - 1) * .5f, r.fY + 5, 1, r.fH - 10);
    CHECK(same(separatorSeen, expected));
    if (xuiToolbarGetOverflowCount(bar) > 0) {
        r = xuiToolbarGetOverflowRect(bar);
        r.fX += vertical ? 3 : 2; r.fY += vertical ? 2 : 3;
        r.fW -= vertical ? 6 : 4; r.fH -= vertical ? 4 : 6;
        expected = pixels(r.fX + (r.fW - 10) * .5f, r.fY + (r.fH - 2) * .5f, 2, 2);
        CHECK(dotCalls >= 3);
        for (i = 0; i < 3; ++i) { CHECK(same(dotSeen[i], expected)); expected.fX += 4; }
    }
}
static void geometry(int odd, int offset, int vertical)
{
    xui_test_proxy_state_t p;
    xui_context c = NULL;
    xui_widget root, parent, bar;
    xui_font font = NULL;
    xui_surface target = NULL;
    xui_toolbar_metrics_t m, got;
    xui_toolbar_colors_t colors;
    xui_toolbar_item_t items[6];
    xui_rect_t r, world, clipped;
    int i, x, y, before, w = vertical ? 55 + odd : 156 + odd, h = vertical ? 114 + odd : 35 + odd;
    xuiTestProxyInit(&p);
    iconOriginal = p.tProxy.drawSurface; p.tProxy.drawSurface = icon_capture;
    textOriginal = p.tProxy.drawText; p.tProxy.drawText = text_capture;
    fillOriginal = p.tProxy.drawRectFill; p.tProxy.drawRectFill = fill_capture;
    copyOriginal = p.tProxy.surfaceDrawTo; p.tProxy.surfaceDrawTo = copy_capture;
    p.tProxy.textMeasure = measure; iconCalls = dotCalls = 0;
    CHECK(xuiCreate(&c) == XUI_OK);
    CHECK(xuiSetProxy(c, &p.tProxy) == XUI_OK);
    CHECK(xuiSetViewportSize(c, 400, 300) == XUI_OK);
    CHECK(p.tProxy.fontLoadMemory(&p.tProxy, &font, "body", 4, 14, XUI_FONT_FORMAT_TTF) == XUI_OK);
    for (i = 0; i < 2; ++i) CHECK(xuiTestSurfaceCreate(&p, &icons[i], 16, 16, XUI_SURFACE_USAGE_TARGET) == XUI_OK);
    CHECK(xuiWidgetCreate(c, &root) == XUI_OK);
    CHECK(xuiSetRootWidget(c, root) == XUI_OK);
    CHECK(xuiWidgetSetLayoutType(root, XUI_LAYOUT_MANUAL) == XUI_OK);
    CHECK(xuiWidgetCreate(c, &parent) == XUI_OK);
    CHECK(xuiWidgetAddChild(root, parent) == XUI_OK);
    CHECK(xuiWidgetSetLayoutType(parent, XUI_LAYOUT_MANUAL) == XUI_OK);
    CHECK(xuiWidgetSetOverflow(parent, XUI_OVERFLOW_HIDDEN) == XUI_OK);
    CHECK(xuiWidgetSetRect(parent, (xui_rect_t){offset, offset, 140, 95}) == XUI_OK);
    CHECK(xuiToolbarCreate(c, &bar, NULL) == XUI_OK);
    CHECK(xuiWidgetAddChild(parent, bar) == XUI_OK);
    CHECK(xuiWidgetSetRect(bar, (xui_rect_t){5, 7, w, h}) == XUI_OK);
    CHECK(xuiToolbarSetFont(bar, font) == XUI_OK);
    memset(items, 0, sizeof(items));
    for (i = 0; i < 6; ++i) { items[i].iType = XUI_TOOLBAR_ITEM_BUTTON; items[i].iState = XUI_TOOLBAR_ITEM_ENABLED; items[i].iValue = 100 + i; items[i].iGroup = i < 3 ? 0 : (i == 5 ? 2 : 1); }
    items[0].sText = "Go"; items[0].pIcon = icons[0]; items[0].tIconSrc = (xui_rect_t){0, 0, 16, 16};
    items[1].iType = XUI_TOOLBAR_ITEM_TOGGLE; items[1].pIcon = icons[1]; items[1].tIconSrc = items[0].tIconSrc;
    items[2].iType = XUI_TOOLBAR_ITEM_SEPARATOR; items[2].iState = 0;
    items[3].sText = "Off"; items[3].iState = 0; items[4].sText = "Run"; items[5].sText = "End";
    CHECK(xuiToolbarSetItems(bar, items, 6) == XUI_OK);
    CHECK(xuiToolbarSetSelect(bar, selected, NULL) == XUI_OK);
    memset(&m, 0, sizeof(m)); m.iSize = sizeof(m);
    CHECK(xuiToolbarGetMetrics(bar, &m) == XUI_OK);
    m.iOrientation = vertical ? XUI_ORIENTATION_VERTICAL : XUI_ORIENTATION_HORIZONTAL;
    m.fItemWidth = 40.75f; m.fItemHeight = 26.5f; m.fSeparatorSize = 7.75f;
    m.fPaddingX = m.fPaddingY = 2.5f; m.fGroupGap = 3.25f; m.fOverflowSize = 19.75f;
    m.fIconSize = 13.5f; m.fIconGap = 3.25f;
    CHECK(xuiToolbarSetMetrics(bar, &m) == XUI_OK);
    memset(&colors, 0, sizeof(colors)); colors.iSize = sizeof(colors);
    CHECK(xuiToolbarGetColors(bar, &colors) == XUI_OK);
    colors.iSeparatorColor = separatorColor = XUI_COLOR_RGBA(17, 97, 53, 255);
    colors.iTextColor = dotColor = XUI_COLOR_RGBA(89, 31, 103, 255);
    CHECK(xuiToolbarSetColors(bar, &colors) == XUI_OK);
    CHECK(xuiToolbarSetOverflow(bar, 0, m.fOverflowSize, overflow_selected, NULL) == XUI_OK);
    CHECK(xuiLayout(c) == XUI_OK);
    verify_layout(bar, &m, w, h, 0);
    verify_draw(c, bar, &m, -1);
    r = xuiToolbarGetItemRect(bar, 0);
    CHECK(xuiToolbarGetItemAt(bar, vertical ? r.fX : r.fX + r.fW, vertical ? r.fY + r.fH : r.fY) == 1);
    world = xuiWidgetGetWorldRect(bar); r = xuiToolbarGetItemRect(bar, 1);
    x = world.fX + r.fX + r.fW / 2; y = world.fY + r.fY + r.fH / 2;
    before = selections;
    CHECK(xuiInputPointerDown(c, x, y, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT) == XUI_OK);
    CHECK(xuiDispatchPendingEvents(c) == XUI_OK);
    CHECK(xuiToolbarGetActiveIndex(bar) == 1);
    verify_draw(c, bar, &m, 1);
    CHECK(xuiInputPointerUp(c, x, y, XUI_POINTER_BUTTON_LEFT, 0) == XUI_OK);
    CHECK(xuiDispatchPendingEvents(c) == XUI_OK);
    CHECK(selections == before + 1 && xuiToolbarGetItemChecked(bar, 1));
    CHECK(xuiTestSurfaceCreate(&p, &target, 400, 300, XUI_SURFACE_USAGE_TARGET) == XUI_OK);
    CHECK(xuiRenderPrepare(c) == XUI_OK);
    tracked = xuiWidgetGetCacheSurface(bar, xuiWidgetGetStateId(bar));
    CHECK(tracked != NULL);
    clipped = intersect(intersect(world, xuiWidgetGetWorldRect(parent)), (xui_rect_t){0, 0, 400, 300});
    copies = 0;
    CHECK(xuiRender(c, target, NULL, 0) == XUI_OK);
    CHECK(copies > 0 && same(dstSeen, clipped));
    CHECK(same(srcSeen, (xui_rect_t){clipped.fX - world.fX, clipped.fY - world.fY, clipped.fW, clipped.fH}));
    CHECK(xuiHitTest(c, vertical ? x : world.fX + w - 1, vertical ? world.fY + h - 1 : y, XUI_WIDGET_HIT_DEFAULT) != bar);
    CHECK(xuiToolbarSetOverflow(bar, 1, m.fOverflowSize, overflow_selected, NULL) == XUI_OK);
    verify_layout(bar, &m, w, h, 1);
    CHECK(xuiToolbarGetOverflowFirst(bar) == 3);
    verify_draw(c, bar, &m, -1);
    /* Enlarge the clipping ancestor only for clicking the overflow button. */
    CHECK(xuiWidgetSetRect(parent, (xui_rect_t){17, 17, 350, 250}) == XUI_OK);
    CHECK(xuiLayout(c) == XUI_OK);
    world = xuiWidgetGetWorldRect(bar); r = xuiToolbarGetOverflowRect(bar);
    x = world.fX + r.fX + r.fW / 2; y = world.fY + r.fY + r.fH / 2;
    before = overflowSelections;
    CHECK(xuiInputPointerDown(c, x, y, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT) == XUI_OK);
    CHECK(xuiDispatchPendingEvents(c) == XUI_OK);
    CHECK(xuiInputPointerUp(c, x, y, XUI_POINTER_BUTTON_LEFT, 0) == XUI_OK);
    CHECK(xuiDispatchPendingEvents(c) == XUI_OK);
    CHECK(overflowSelections == before + 1 && lastFirst == 3 && lastCount == 3);
    m.fPaddingX = m.fPaddingY = 3.75f; m.fIconSize = 14.25f; m.fIconGap = 2.75f;
    CHECK(xuiToolbarSetMetrics(bar, &m) == XUI_OK);
    memset(&got, 0, sizeof(got)); got.iSize = sizeof(got);
    CHECK(xuiToolbarGetMetrics(bar, &got) == XUI_OK && got.fPaddingX == 3.75f && got.fIconSize == 14.25f);
    verify_layout(bar, &m, w, h, 1); verify_draw(c, bar, &m, -1);
    m.fItemWidth = 41.5f; m.fItemHeight = 27.75f; m.fSeparatorSize = 8.5f;
    CHECK(xuiToolbarSetItemSize(bar, m.fItemWidth, m.fItemHeight, m.fSeparatorSize) == XUI_OK);
    verify_layout(bar, &m, w, h, 1); verify_draw(c, bar, &m, -1);
    m.iOrientation = vertical ? XUI_ORIENTATION_HORIZONTAL : XUI_ORIENTATION_VERTICAL;
    w = vertical ? 300 : 56; h = vertical ? 36 : 210;
    CHECK(xuiToolbarSetOrientation(bar, m.iOrientation) == XUI_OK);
    CHECK(xuiWidgetSetRect(bar, (xui_rect_t){5, 7, w, h}) == XUI_OK);
    CHECK(xuiToolbarSetOverflow(bar, 0, m.fOverflowSize, overflow_selected, NULL) == XUI_OK);
    verify_layout(bar, &m, w, h, 0); verify_draw(c, bar, &m, -1);
    CHECK(xuiToolbarSetItemGroup(bar, 5, 1) == XUI_OK);
    verify_layout(bar, &m, w, h, 0); verify_draw(c, bar, &m, -1);
    tracked = NULL; xuiDestroy(c);
    p.tProxy.surfaceDestroy(&p.tProxy, target);
    for (i = 0; i < 2; ++i) { p.tProxy.surfaceDestroy(&p.tProxy, icons[i]); icons[i] = NULL; }
    p.tProxy.fontDestroy(&p.tProxy, font);
}
int main(void)
{
    int odd, offset, vertical;
    for (odd = 0; odd <= 1; ++odd)
    for (offset = -17; offset <= 17; offset += 34)
    for (vertical = 0; vertical < 2; ++vertical) geometry(odd, offset, vertical);
    printf("xui_toolbar_pixel_test: %d checks, %d failures, %d selection / %d overflow callbacks\n",
        checks, failures, selections, overflowSelections);
    return failures != 0;
}
