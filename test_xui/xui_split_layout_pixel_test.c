#include "src/xui_internal.h"
#include "xui_test_proxy.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

static int checks, failures;
#define CHECK(e) do { ++checks; if (!(e)) { if (failures < 12) { printf("FAIL %d: %s\n", __LINE__, #e); } ++failures; } } while (0)
static xui_rect_t pixels(float x, float y, float w, float h)
{
    int l = (int)floorf(x + .5f), t = (int)floorf(y + .5f);
    return (xui_rect_t){l, t, (int)floorf(x + w + .5f) - l, (int)floorf(y + h + .5f) - t};
}
static int same(xui_rect_t a, xui_rect_t b)
{
    return a.fX == b.fX && a.fY == b.fY && a.fW == b.fW && a.fH == b.fH;
}
static xui_rect_t centered(xui_rect_t r, int vertical, float size)
{
    return vertical ? pixels(r.fX + (r.fW - size) * .5f, r.fY, size, r.fH) :
        pixels(r.fX, r.fY + (r.fH - size) * .5f, r.fW, size);
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
static void geometry(int vertical, int odd, int offset)
{
    xui_test_proxy_state_t p;
    xui_context c = NULL;
    xui_widget root, parent, split, divider;
    xui_surface target = NULL;
    xui_split_layout_desc_t d;
    xui_rect_t layout, visual, hit, world, clipped, shadow, final;
    int x, y, dx = vertical ? 14 : 0, dy = vertical ? 0 : -12;
    xuiTestProxyInit(&p);
    copyOriginal = p.tProxy.surfaceDrawTo;
    p.tProxy.surfaceDrawTo = copy_capture;
    CHECK(xuiCreate(&c) == XUI_OK);
    CHECK(xuiSetProxy(c, &p.tProxy) == XUI_OK);
    CHECK(xuiSetViewportSize(c, 400, 300) == XUI_OK);
    CHECK(xuiWidgetCreate(c, &root) == XUI_OK);
    CHECK(xuiSetRootWidget(c, root) == XUI_OK);
    CHECK(xuiWidgetSetLayoutType(root, XUI_LAYOUT_MANUAL) == XUI_OK);
    CHECK(xuiWidgetCreate(c, &parent) == XUI_OK);
    CHECK(xuiWidgetAddChild(root, parent) == XUI_OK);
    CHECK(xuiWidgetSetLayoutType(parent, XUI_LAYOUT_MANUAL) == XUI_OK);
    CHECK(xuiWidgetSetOverflow(parent, XUI_OVERFLOW_HIDDEN) == XUI_OK);
    CHECK(xuiWidgetSetRect(parent, (xui_rect_t){offset, offset, 380, 260}) == XUI_OK);
    memset(&d, 0, sizeof(d));
    d.iSize = sizeof(d);
    d.iOrientation = vertical ? XUI_ORIENTATION_VERTICAL : XUI_ORIENTATION_HORIZONTAL;
    d.iPaneCount = 2;
    d.fDividerSize = 5.75f;
    d.fDividerVisualSize = 2.5f;
    d.fDividerHitSize = 11.75f;
    CHECK(xuiSplitLayoutCreate(c, &split, &d) == XUI_OK);
    CHECK(xuiWidgetAddChild(parent, split) == XUI_OK);
    CHECK(xuiWidgetSetRect(split, (xui_rect_t){5, 7, 320 + odd, 190 + odd}) == XUI_OK);
    CHECK(xuiSplitLayoutSetPaneMode(split, 0, XUI_SPLIT_PANE_FIXED) == XUI_OK);
    CHECK(xuiSplitLayoutSetPaneFixedSize(split, 0, 89.25f) == XUI_OK);
    CHECK(xuiSplitLayoutSetPaneMinSize(split, 0, 80.75f) == XUI_OK);
    CHECK(xuiLayout(c) == XUI_OK);
    layout = xuiSplitLayoutGetDividerLayoutRect(split, 0);
    visual = centered(layout, vertical, 2.5f);
    hit = centered(layout, vertical, 11.75f);
    CHECK(same(xuiSplitLayoutGetDividerVisualRect(split, 0), visual));
    CHECK(same(xuiSplitLayoutGetDividerHitRect(split, 0), hit));
    divider = xuiWidgetGetLastChild(split);
    CHECK(same(xuiWidgetGetRect(divider), layout));
    CHECK(xuiTestSurfaceCreate(&p, &target, 400, 300, XUI_SURFACE_USAGE_TARGET) == XUI_OK);
    CHECK(xuiRenderPrepare(c) == XUI_OK);
    tracked = xuiWidgetGetCacheSurface(divider, xuiWidgetGetInputState(divider));
    CHECK(tracked != NULL);
    CHECK(same(xuiTestSurfaceGetLastRect(tracked),
        (xui_rect_t){visual.fX - layout.fX, visual.fY - layout.fY, visual.fW, visual.fH}));
    world = xuiWidgetGetWorldRect(divider);
    clipped = world;
    if (clipped.fX < 0) { clipped.fW += clipped.fX; clipped.fX = 0; }
    if (clipped.fY < 0) { clipped.fH += clipped.fY; clipped.fY = 0; }
    copies = 0;
    CHECK(xuiRender(c, target, NULL, 0) == XUI_OK);
    CHECK(copies > 0 && same(dstSeen, clipped));
    CHECK(same(srcSeen, (xui_rect_t){clipped.fX - world.fX, clipped.fY - world.fY, clipped.fW, clipped.fH}));
    CHECK(xuiHitTest(c, -1, 100, XUI_WIDGET_HIT_DEFAULT) == NULL);
    x = world.fX + world.fW / 2;
    y = world.fY + world.fH / 2;
    CHECK(xuiInputPointerDown(c, x, y, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT) == XUI_OK);
    CHECK(xuiDispatchPendingEvents(c) == XUI_OK);
    CHECK(xuiSplitLayoutGetActiveDivider(split) == 0);
    CHECK(xuiInputPointerMove(c, x + dx, y + dy, XUI_POINTER_BUTTON_LEFT) == XUI_OK);
    CHECK(xuiDispatchPendingEvents(c) == XUI_OK);
    shadow = xuiSplitLayoutGetShadowRect(split);
    CHECK(same(layout, xuiSplitLayoutGetDividerLayoutRect(split, 0)));
    CHECK(xuiInputPointerUp(c, x + dx, y + dy, XUI_POINTER_BUTTON_LEFT, 0) == XUI_OK);
    CHECK(xuiDispatchPendingEvents(c) == XUI_OK);
    CHECK(xuiLayout(c) == XUI_OK);
    final = xuiSplitLayoutGetDividerVisualRect(split, 0);
    world = xuiWidgetGetWorldRect(split);
    final.fX += world.fX; final.fY += world.fY;
    CHECK(same(shadow, final));
    CHECK(fabsf(xuiSplitLayoutGetPaneFixedSize(split, 0) -
        fmaxf(80.75f, (vertical ? layout.fX + dx : layout.fY + dy))) < .001f);
    tracked = NULL;
    p.tProxy.surfaceDestroy(&p.tProxy, target);
    xuiDestroy(c);
}
int main(void)
{
    int vertical, odd, offset;
    for (vertical = 0; vertical <= 1; ++vertical)
    for (odd = 0; odd <= 1; ++odd)
    for (offset = -17; offset <= 17; offset += 34) geometry(vertical, odd, offset);
    printf("xui_split_layout_pixel_test: %d checks, %d failures\n", checks, failures);
    return failures != 0;
}
