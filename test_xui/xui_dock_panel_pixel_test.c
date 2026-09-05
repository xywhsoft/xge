#include "src/xui_internal.h"
#include "xui_test_proxy.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
static int checks, failures;
#define CHECK(e) do { ++checks; if (!(e)) { if (failures < 16) { printf("FAIL %d: %s\n", __LINE__, #e); } ++failures; } } while (0)
static xui_rect_t pixels(float x, float y, float w, float h)
{
    int l = (int)floorf(x + .5f), t = (int)floorf(y + .5f);
    return (xui_rect_t){l, t, (int)floorf(x + w + .5f) - l, (int)floorf(y + h + .5f) - t};
}
static int same(xui_rect_t a, xui_rect_t b)
{
    return a.fX == b.fX && a.fY == b.fY && a.fW == b.fW && a.fH == b.fH;
}
static xui_dock_pane_info_t pane(xui_widget dock, int id)
{
    xui_dock_pane_info_t info;
    memset(&info, 0, sizeof(info)); info.iSize = sizeof(info);
    CHECK(xuiDockPanelGetPaneInfo(dock, id, &info) == XUI_OK);
    return info;
}
static xui_dock_window_info_t window(xui_widget dock, int id)
{
    xui_dock_window_info_t info;
    memset(&info, 0, sizeof(info)); info.iSize = sizeof(info);
    CHECK(xuiDockPanelGetWindowInfo(dock, id, &info) == XUI_OK);
    return info;
}
static xui_surface tracked;
static xui_rect_t srcSeen, dstSeen;
static int copies;
static xui_surface_draw_to_proc copyOriginal;
static int copy_capture(xui_proxy p, xui_surface target, xui_surface source,
    xui_rect_t src, xui_rect_t dst, uint32_t color, uint32_t flags)
{
    if (source == tracked) { srcSeen = src; dstSeen = dst; ++copies; }
    return copyOriginal(p, target, source, src, dst, color, flags);
}
static void drag_split(xui_context c, xui_widget dock, xui_rect_t r, int dx, int dy)
{
    xui_rect_t world = xuiWidgetGetWorldRect(dock), shadow, final;
    xui_dock_hit_t hit;
    int x = world.fX + r.fX + r.fW / 2, y = world.fY + r.fY + r.fH / 2;
    CHECK(xuiInputPointerDown(c, x, y, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT) == XUI_OK);
    CHECK(xuiDispatchPendingEvents(c) == XUI_OK);
    CHECK(xuiGetPointerCapture(c) == dock);
    CHECK(xuiInputPointerMove(c, x + dx, y + dy, XUI_POINTER_BUTTON_LEFT) == XUI_OK);
    CHECK(xuiDispatchPendingEvents(c) == XUI_OK);
    CHECK(c->iDragAdornerPrimitiveCount == 1);
    shadow = c->arrDragAdornerPrimitives[0].tRect;
    CHECK(xuiInputPointerUp(c, x + dx, y + dy, XUI_POINTER_BUTTON_LEFT, 0) == XUI_OK);
    CHECK(xuiDispatchPendingEvents(c) == XUI_OK);
    CHECK(xuiLayout(c) == XUI_OK);
    memset(&hit, 0, sizeof(hit)); hit.iSize = sizeof(hit);
    CHECK(xuiDockPanelHitTest(dock, shadow.fX - world.fX + shadow.fW / 2,
        shadow.fY - world.fY + shadow.fH / 2, &hit) == XUI_OK);
    CHECK(hit.iType == XUI_DOCK_PANEL_HIT_SPLITTER);
    final = hit.tRect; final.fX += world.fX; final.fY += world.fY;
    CHECK(same(shadow, final));
}
static void resize_auto_hide(xui_context c, xui_widget dock, int id, int region)
{
    xui_dock_window_info_t wi = window(dock, id);
    xui_rect_t world = xuiWidgetGetWorldRect(wi.pHostWidget), shadow, r, expected;
    int x = world.fX + world.fW / 2, y = world.fY + world.fH / 2, dx = 0, dy = 0;
    if (region == XUI_DOCK_PANEL_REGION_LEFT) { x = world.fX + world.fW - 1; dx = -300; }
    if (region == XUI_DOCK_PANEL_REGION_RIGHT) { x = world.fX + 1; dx = 300; }
    if (region == XUI_DOCK_PANEL_REGION_TOP) { y = world.fY + world.fH - 1; dy = -300; }
    if (region == XUI_DOCK_PANEL_REGION_BOTTOM) { y = world.fY + 1; dy = 300; }
    CHECK(xuiInputPointerDown(c, x, y, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT) == XUI_OK);
    CHECK(xuiDispatchPendingEvents(c) == XUI_OK);
    CHECK(xuiGetPointerCapture(c) == wi.pHostWidget);
    CHECK(xuiInputPointerMove(c, x + dx, y + dy, XUI_POINTER_BUTTON_LEFT) == XUI_OK);
    CHECK(xuiDispatchPendingEvents(c) == XUI_OK);
    CHECK(c->iDragAdornerPrimitiveCount == 1);
    shadow = c->arrDragAdornerPrimitives[0].tRect;
    CHECK(xuiInputPointerUp(c, x + dx, y + dy, XUI_POINTER_BUTTON_LEFT, 0) == XUI_OK);
    CHECK(xuiDispatchPendingEvents(c) == XUI_OK);
    CHECK(xuiLayout(c) == XUI_OK);
    r = xuiDockPanelGetAutoHideExpandRect(dock);
    if (region == XUI_DOCK_PANEL_REGION_LEFT || region == XUI_DOCK_PANEL_REGION_RIGHT) {
        CHECK(r.fW == 81);
        expected = pixels((region == XUI_DOCK_PANEL_REGION_LEFT ? r.fX + r.fW : r.fX) - 2.875f,
            r.fY, 5.75f, r.fH);
    } else {
        CHECK(r.fH == 61);
        expected = pixels(r.fX, (region == XUI_DOCK_PANEL_REGION_TOP ? r.fY + r.fH : r.fY) - 2.875f,
            r.fW, 5.75f);
    }
    world = xuiWidgetGetWorldRect(dock);
    expected.fX += world.fX; expected.fY += world.fY;
    CHECK(same(shadow, expected));
}
static void geometry(int odd, int offset, int region)
{
    xui_test_proxy_state_t p;
    xui_context c = NULL;
    xui_widget root, parent, dock;
    xui_surface target = NULL;
    xui_dock_panel_metrics_t m;
    xui_dock_pane_info_t a, b;
    xui_dock_window_info_t wi;
    xui_dock_hit_t hit;
    xui_rect_t r, split, world, clipped, expected;
    int doc, side, extra, pa, pb, pc, mode, w = 640 + odd, h = 420 + odd;
    int vertical = region == XUI_DOCK_PANEL_REGION_LEFT || region == XUI_DOCK_PANEL_REGION_RIGHT;
    float value;
    xuiTestProxyInit(&p);
    copyOriginal = p.tProxy.surfaceDrawTo; p.tProxy.surfaceDrawTo = copy_capture;
    CHECK(xuiCreate(&c) == XUI_OK);
    CHECK(xuiSetProxy(c, &p.tProxy) == XUI_OK);
    CHECK(xuiSetViewportSize(c, 800, 600) == XUI_OK);
    CHECK(xuiWidgetCreate(c, &root) == XUI_OK);
    CHECK(xuiSetRootWidget(c, root) == XUI_OK);
    CHECK(xuiWidgetSetLayoutType(root, XUI_LAYOUT_MANUAL) == XUI_OK);
    CHECK(xuiWidgetCreate(c, &parent) == XUI_OK);
    CHECK(xuiWidgetAddChild(root, parent) == XUI_OK);
    CHECK(xuiWidgetSetLayoutType(parent, XUI_LAYOUT_MANUAL) == XUI_OK);
    CHECK(xuiWidgetSetOverflow(parent, XUI_OVERFLOW_HIDDEN) == XUI_OK);
    CHECK(xuiWidgetSetRect(parent, (xui_rect_t){offset, offset, 760, 550}) == XUI_OK);
    CHECK(xuiDockPanelCreate(c, &dock, NULL) == XUI_OK);
    CHECK(xuiWidgetAddChild(parent, dock) == XUI_OK);
    CHECK(xuiWidgetSetRect(dock, (xui_rect_t){5, 7, w, h}) == XUI_OK);
    memset(&m, 0, sizeof(m)); m.iSize = sizeof(m);
    CHECK(xuiDockPanelGetMetrics(dock, &m) == XUI_OK);
    m.fBorderWidth = 1.75f; m.fCaptionHeight = 23.5f; m.fTabStripHeight = 25.75f;
    m.fButtonSize = 13.75f; m.fSplitterSize = 5.75f; m.fSplitterHitSize = 11.5f;
    m.fMinPaneWidth = 80.75f; m.fMinPaneHeight = 60.5f; m.fAutoHideStripSize = 19.5f;
    CHECK(xuiDockPanelSetMetrics(dock, &m) == XUI_OK);
    CHECK(xuiDockPanelAddWindow(dock, "Document", NULL, &doc) == XUI_OK);
    CHECK(xuiDockPanelDockWindow(dock, doc, XUI_DOCK_PANEL_REGION_DOCUMENT, XUI_DOCK_PANEL_SIDE_FILL, .5f, &pa) == XUI_OK);
    CHECK(xuiDockPanelAddWindow(dock, "Side", NULL, &side) == XUI_OK);
    CHECK(xuiDockPanelDockWindow(dock, side, region, XUI_DOCK_PANEL_SIDE_FILL, .5f, &pb) == XUI_OK);
    CHECK(xuiDockPanelSetRegionSize(dock, region, XUI_DOCK_PANEL_SIZE_PIXEL, 128.25f) == XUI_OK);
    CHECK(xuiLayout(c) == XUI_OK);
    if (region == XUI_DOCK_PANEL_REGION_LEFT) {
        r = pixels(0, 0, 128.25f, h); split = pixels(128.25f, 0, 5.75f, h);
    } else if (region == XUI_DOCK_PANEL_REGION_RIGHT) {
        r = pixels(w - 128.25f, 0, 128.25f, h); split = pixels(w - 134, 0, 5.75f, h);
    } else if (region == XUI_DOCK_PANEL_REGION_TOP) {
        r = pixels(0, 0, w, 128.25f); split = pixels(0, 128.25f, w, 5.75f);
    } else {
        r = pixels(0, h - 128.25f, w, 128.25f); split = pixels(0, h - 134, w, 5.75f);
    }
    b = pane(dock, pb);
    CHECK(same(b.tRect, r));
    CHECK(same(b.tCaptionRect, pixels(r.fX + 1.75f, r.fY + 1.75f, r.fW - 3.5f, 23.5f)));
    CHECK(same(b.tClientRect, pixels(r.fX + 1.75f, r.fY + 25.25f, r.fW - 3.5f, r.fH - 27)));
    CHECK(b.tCaptionRect.fY + b.tCaptionRect.fH == b.tClientRect.fY);
    memset(&hit, 0, sizeof(hit)); hit.iSize = sizeof(hit);
    CHECK(xuiDockPanelHitTest(dock, split.fX + split.fW / 2, split.fY + split.fH / 2, &hit) == XUI_OK);
    CHECK(hit.iType == XUI_DOCK_PANEL_HIT_SPLITTER && same(hit.tRect, split));
    expected = vertical ? pixels(split.fX - (11.5f - split.fW) * .5f, split.fY, 11.5f, split.fH) :
        pixels(split.fX, split.fY - (11.5f - split.fH) * .5f, split.fW, 11.5f);
    CHECK(xuiDockPanelHitTest(dock, vertical ? expected.fX : expected.fX + expected.fW / 2,
        vertical ? expected.fY + expected.fH / 2 : expected.fY, &hit) == XUI_OK);
    CHECK(hit.iType == XUI_DOCK_PANEL_HIT_SPLITTER);
    CHECK(xuiDockPanelHitTest(dock, vertical ? expected.fX - 1 : expected.fX + expected.fW / 2,
        vertical ? expected.fY + expected.fH / 2 : expected.fY - 1, &hit) == XUI_OK);
    CHECK(hit.iType != XUI_DOCK_PANEL_HIT_SPLITTER);
    drag_split(c, dock, split, vertical ? 13 : 0, vertical ? 0 : 13);
    CHECK(xuiDockPanelGetRegionSize(dock, region, &mode, &value) == XUI_OK);
    CHECK(value == (region == XUI_DOCK_PANEL_REGION_LEFT || region == XUI_DOCK_PANEL_REGION_TOP ? 141.25f : 115.25f));
    CHECK(xuiDockPanelAddWindow(dock, "Nested", NULL, &extra) == XUI_OK);
    CHECK(xuiDockPanelDockWindowToPaneSide(dock, extra, pa, XUI_DOCK_PANEL_SIDE_RIGHT, .37f, &pc) == XUI_OK);
    CHECK(xuiLayout(c) == XUI_OK);
    a = pane(dock, pa); b = pane(dock, pc);
    CHECK(a.tRect.fX + a.tRect.fW < b.tRect.fX);
    split = (xui_rect_t){a.tRect.fX + a.tRect.fW, a.tRect.fY,
        b.tRect.fX - a.tRect.fX - a.tRect.fW, a.tRect.fH};
    CHECK(xuiDockPanelHitTest(dock, split.fX + split.fW / 2, split.fY + split.fH / 2, &hit) == XUI_OK);
    CHECK(hit.iType == XUI_DOCK_PANEL_HIT_SPLITTER && same(hit.tRect, split));
    drag_split(c, dock, split, -21, 0);
    CHECK(xuiDockPanelAutoHideWindow(dock, side) == XUI_OK);
    CHECK(xuiLayout(c) == XUI_OK);
    wi = window(dock, side);
    expected = vertical ? pixels(region == XUI_DOCK_PANEL_REGION_LEFT ? 0 : w - 19.5f, 0, 19.5f, wi.tAutoHideRect.fH) :
        pixels(0, region == XUI_DOCK_PANEL_REGION_TOP ? 0 : h - 19.5f, wi.tAutoHideRect.fW, 19.5f);
    CHECK(same(wi.tAutoHideRect, expected));
    a = pane(dock, pa); b = pane(dock, pc);
    if (region == XUI_DOCK_PANEL_REGION_LEFT) CHECK(a.tRect.fX == expected.fX + expected.fW);
    if (region == XUI_DOCK_PANEL_REGION_RIGHT) CHECK(b.tRect.fX + b.tRect.fW == expected.fX);
    if (region == XUI_DOCK_PANEL_REGION_TOP) CHECK(a.tRect.fY == expected.fY + expected.fH);
    if (region == XUI_DOCK_PANEL_REGION_BOTTOM) CHECK(a.tRect.fY + a.tRect.fH == expected.fY);
    CHECK(xuiDockPanelSetWindowMinSize(dock, side, 80.75f, 60.75f) == XUI_OK);
    CHECK(xuiDockPanelExpandAutoHideWindow(dock, side) == XUI_OK);
    CHECK(xuiLayout(c) == XUI_OK);
    wi = window(dock, side);
    CHECK(same(wi.tRect, xuiDockPanelGetAutoHideExpandRect(dock)));
    CHECK(same(xuiWidgetGetRect(wi.pHostWidget), wi.tRect));
    resize_auto_hide(c, dock, side, region);
    CHECK(xuiDockPanelCollapseAutoHide(dock) == XUI_OK);
    CHECK(xuiLayout(c) == XUI_OK);
    CHECK(xuiTestSurfaceCreate(&p, &target, 800, 600, XUI_SURFACE_USAGE_TARGET) == XUI_OK);
    CHECK(xuiRenderPrepare(c) == XUI_OK);
    tracked = xuiWidgetGetCacheSurface(dock, xuiWidgetGetStateId(dock));
    CHECK(tracked != NULL);
    world = xuiWidgetGetWorldRect(dock); clipped = world;
    if (clipped.fX < 0) { clipped.fW += clipped.fX; clipped.fX = 0; }
    if (clipped.fY < 0) { clipped.fH += clipped.fY; clipped.fY = 0; }
    copies = 0;
    CHECK(xuiRender(c, target, NULL, 0) == XUI_OK);
    CHECK(copies > 0 && same(dstSeen, clipped));
    CHECK(same(srcSeen, (xui_rect_t){clipped.fX - world.fX, clipped.fY - world.fY, clipped.fW, clipped.fH}));
    CHECK(xuiHitTest(c, -1, 100, XUI_WIDGET_HIT_DEFAULT) == NULL);
    if (region == XUI_DOCK_PANEL_REGION_LEFT) {
        float basis = w - pixels(0, 0, 19.5f, h).fW;
        CHECK(xuiDockPanelDockWindow(dock, extra, XUI_DOCK_PANEL_REGION_RIGHT,
            XUI_DOCK_PANEL_SIDE_FILL, .5f, &pc) == XUI_OK);
        CHECK(xuiDockPanelSetRegionSize(dock, XUI_DOCK_PANEL_REGION_RIGHT,
            XUI_DOCK_PANEL_SIZE_PORTION, .4f) == XUI_OK);
        CHECK(xuiLayout(c) == XUI_OK);
        split = pixels(w - basis * .4f - 5.75f, 0, 5.75f, h);
        drag_split(c, dock, split, 13, 0);
        CHECK(xuiDockPanelGetRegionSize(dock, XUI_DOCK_PANEL_REGION_RIGHT, &mode, &value) == XUI_OK);
        CHECK(mode == XUI_DOCK_PANEL_SIZE_PORTION);
        CHECK(fabsf(value - (.4f - 13 / basis)) < .000001f);
    }
    tracked = NULL;
    p.tProxy.surfaceDestroy(&p.tProxy, target);
    xuiDestroy(c);
}
int main(void)
{
    int odd, offset, region;
    for (odd = 0; odd <= 1; ++odd)
    for (offset = -17; offset <= 17; offset += 34)
    for (region = XUI_DOCK_PANEL_REGION_LEFT; region <= XUI_DOCK_PANEL_REGION_BOTTOM; ++region)
        geometry(odd, offset, region);
    printf("xui_dock_panel_pixel_test: %d checks, %d failures\n", checks, failures);
    return failures != 0;
}
