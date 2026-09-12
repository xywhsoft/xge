#include "../src/xui_internal.h"
#include "xui_test_proxy.h"
#include <stdio.h>
#include <string.h>

#define CHECK(e) do { ++checks; if (!(e)) { printf("style_drag: %s (%d)\n", #e, __LINE__); return 1; } } while (0)
static int checks, count;
static uint32_t colors[4096];
static int (*base_fill)(xui_proxy, xui_draw_context, xui_rect_t, uint32_t);
static int fill(xui_proxy p, xui_draw_context d, xui_rect_t r, uint32_t c)
{ if (count < 4096) colors[count++] = c; return base_fill(p, d, r, c); }
static int seen(uint32_t c)
{ int i; for (i = 0; i < count; ++i) if (colors[i] == c) return 1; return 0; }
static xui_style_property_t color(const char* name, uint32_t value)
{
    xui_style_property_t p = {0};
    p.iSize = sizeof(p); p.sName = name; p.tValue.iSize = sizeof(p.tValue);
    p.tValue.iType = XUI_STYLE_VALUE_COLOR; p.tValue.iColor = value; return p;
}
static int paint(xui_context c)
{ count = 0; return xuiRenderPrepare(c); }
static int fixture(xui_test_proxy_state_t* proxy, xui_context* context)
{
    xui_widget root;
    xuiTestProxyInit(proxy); base_fill = proxy->tProxy.drawRectFill; proxy->tProxy.drawRectFill = fill;
    CHECK(xuiCreate(context) == XUI_OK);
    CHECK(xuiSetProxy(*context, &proxy->tProxy) == XUI_OK);
    CHECK(xuiInputViewport(*context, 640, 480) == XUI_OK);
    CHECK(xuiWidgetCreate(*context, &root) == XUI_OK);
    CHECK(xuiWidgetSetLayoutType(root, XUI_LAYOUT_MANUAL) == XUI_OK);
    CHECK(xuiSetRootWidget(*context, root) == XUI_OK);
    return 0;
}
static int split_colors(int orientation)
{
    xui_test_proxy_state_t proxy;
    xui_context c;
    xui_widget w, divider;
    xui_split_layout_desc_t desc = {0};
    xui_style_property_t p;
    xui_rect_t r, before, pane, shadow;
    uint32_t layout_version, base;
    int x, y;
    CHECK(fixture(&proxy, &c) == 0);
    desc.iSize = sizeof(desc); desc.iOrientation = orientation; desc.iPaneCount = 2;
    desc.fDividerSize = 3; desc.fDividerVisualSize = 3;
    CHECK(xuiSplitLayoutCreate(c, &w, &desc) == XUI_OK);
    CHECK(xuiWidgetAddChild(xuiGetRootWidget(c), w) == XUI_OK);
    CHECK(xuiWidgetSetRect(w, (xui_rect_t){30, 40, 400, 240}) == XUI_OK);
    CHECK(paint(c) == XUI_OK);
    CHECK(xuiSplitLayoutGetColors(w, &base, NULL, NULL, NULL) == XUI_OK);
    for (divider = xuiWidgetGetFirstChild(w); divider != NULL; divider = xuiWidgetGetNextSibling(divider))
        if (divider != xuiSplitLayoutGetPaneWidget(w, 0) && divider != xuiSplitLayoutGetPaneWidget(w, 1)) break;
    CHECK(divider != NULL);
    before = xuiWidgetGetRect(divider); layout_version = w->iLayoutVersion;
    p = color("splitlayout.divider.color", 0x974521ff);
    CHECK(xuiWidgetSetInlineStyle(w, &p, 1) == XUI_OK);
    CHECK(paint(c) == XUI_OK && seen(p.tValue.iColor));
    CHECK(paint(c) == XUI_OK && count == 0);
    p.tValue.iColor = 0;
    CHECK(xuiWidgetSetInlineStyle(w, &p, 1) == XUI_OK);
    CHECK(paint(c) == XUI_OK && !seen(0x974521ff) && !seen(base));
    CHECK(xuiWidgetSetInlineStyle(w, NULL, 0) == XUI_OK);
    CHECK(paint(c) == XUI_OK && seen(base));
    CHECK(xuiSplitLayoutSetColors(w, 0x348672ff, 0x762134ff, 0x571934ff, 0x319752ff) == XUI_OK);
    CHECK(paint(c) == XUI_OK && seen(0x348672ff));
    r = xuiWidgetGetWorldRect(divider); x = r.fX + r.fW / 2; y = r.fY + r.fH / 2;
    CHECK(xuiInputPointerMove(c, x, y, 0) == XUI_OK && xuiDispatchPendingEvents(c) == XUI_OK);
    CHECK(paint(c) == XUI_OK);
    p = color("splitlayout.divider.hover_color", 0x649723ff);
    CHECK(xuiWidgetSetInlineStyle(w, &p, 1) == XUI_OK);
    CHECK(paint(c) == XUI_OK && seen(p.tValue.iColor));
    CHECK(xuiInputPointerDown(c, x, y, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT) == XUI_OK);
    CHECK(xuiDispatchPendingEvents(c) == XUI_OK && paint(c) == XUI_OK);
    CHECK(c->pDragAdornerOwner == w && c->iDragAdornerPrimitiveCount == 1);
    pane = xuiSplitLayoutGetPaneRect(w, 0); shadow = c->arrDragAdornerPrimitives[0].tRect;
    p = color("splitlayout.shadow.color", 0x829364ff);
    CHECK(xuiWidgetSetInlineStyle(w, &p, 1) == XUI_OK);
    CHECK(paint(c) == XUI_OK && c->arrDragAdornerPrimitives[0].iColor == p.tValue.iColor && seen(p.tValue.iColor));
    r = c->arrDragAdornerPrimitives[0].tRect;
    CHECK(memcmp(&r, &shadow, sizeof(r)) == 0);
    p.tValue.iColor = 0;
    CHECK(xuiWidgetSetInlineStyle(w, &p, 1) == XUI_OK);
    CHECK(paint(c) == XUI_OK && c->arrDragAdornerPrimitives[0].iColor == 0);
    CHECK(xuiWidgetSetInlineStyle(w, NULL, 0) == XUI_OK);
    CHECK(paint(c) == XUI_OK && c->arrDragAdornerPrimitives[0].iColor == 0x319752ff);
    p = color("splitlayout.divider.active_color", 0x934671ff);
    CHECK(xuiWidgetSetInlineStyle(w, &p, 1) == XUI_OK);
    CHECK(paint(c) == XUI_OK && seen(p.tValue.iColor));
    r = xuiWidgetGetRect(divider); CHECK(memcmp(&r, &before, sizeof(r)) == 0);
    r = xuiSplitLayoutGetPaneRect(w, 0); CHECK(memcmp(&r, &pane, sizeof(r)) == 0);
    CHECK(w->iLayoutVersion == layout_version);
    CHECK(xuiInputPointerUp(c, x, y, XUI_POINTER_BUTTON_LEFT, 0) == XUI_OK);
    CHECK(xuiDispatchPendingEvents(c) == XUI_OK);
    xuiDestroy(c); return 0;
}
static int window_colors(int resize)
{
    xui_test_proxy_state_t proxy;
    xui_context c;
    xui_widget w;
    xui_window_desc_t desc = {0};
    xui_style_property_t p = color("window.border.active_color", 0x984521ff);
    xui_rect_t before, shadow, r;
    uint32_t layout_version, base;
    int x, y;
    CHECK(fixture(&proxy, &c) == 0);
    desc.iSize = sizeof(desc); desc.sTitle = "Theme";
    CHECK(xuiWindowCreate(c, &w, &desc) == XUI_OK);
    CHECK(xuiWidgetSetRect(w, (xui_rect_t){80, 70, 280, 200}) == XUI_OK);
    CHECK(paint(c) == XUI_OK);
    before = xuiWidgetGetRect(w); layout_version = w->iLayoutVersion;
    x = resize ? 359 : 110; y = resize ? 269 : 80;
    CHECK(xuiInputPointerDown(c, x, y, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT) == XUI_OK);
    CHECK(xuiDispatchPendingEvents(c) == XUI_OK);
    CHECK(xuiInputPointerMove(c, x + 20, y + 10, XUI_POINTER_BUTTON_LEFT) == XUI_OK);
    CHECK(xuiDispatchPendingEvents(c) == XUI_OK && paint(c) == XUI_OK);
    CHECK(c->pDragAdornerOwner == w && c->iDragAdornerPrimitiveCount == 1);
    shadow = c->arrDragAdornerPrimitives[0].tRect; base = c->arrDragAdornerPrimitives[0].iColor;
    CHECK(xuiWidgetSetInlineStyle(w, &p, 1) == XUI_OK);
    CHECK(paint(c) == XUI_OK && c->arrDragAdornerPrimitives[0].iColor == p.tValue.iColor);
    CHECK(paint(c) == XUI_OK && count == 0);
    p.tValue.iColor = 0;
    CHECK(xuiWidgetSetInlineStyle(w, &p, 1) == XUI_OK);
    CHECK(paint(c) == XUI_OK && c->arrDragAdornerPrimitives[0].iColor == 0);
    CHECK(xuiWidgetSetInlineStyle(w, NULL, 0) == XUI_OK);
    CHECK(paint(c) == XUI_OK && c->arrDragAdornerPrimitives[0].iColor == base);
    r = c->arrDragAdornerPrimitives[0].tRect; CHECK(memcmp(&r, &shadow, sizeof(r)) == 0);
    r = xuiWidgetGetRect(w); CHECK(memcmp(&r, &before, sizeof(r)) == 0);
    CHECK(w->iLayoutVersion == layout_version);
    CHECK(xuiInputPointerUp(c, x + 20, y + 10, XUI_POINTER_BUTTON_LEFT, 0) == XUI_OK);
    CHECK(xuiDispatchPendingEvents(c) == XUI_OK);
    xuiDestroy(c); return 0;
}
int main(void)
{
    int result = split_colors(XUI_ORIENTATION_VERTICAL) | split_colors(XUI_ORIENTATION_HORIZONTAL);
    result |= window_colors(0) | window_colors(1);
    printf("style_drag: %d checks, result=%d\n", checks, result);
    return result;
}
