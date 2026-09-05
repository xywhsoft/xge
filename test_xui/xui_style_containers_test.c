#include "src/xui_internal.h"
#include "xui_test_proxy.h"
#include <stdio.h>
#include <string.h>

static int checks, failures;
#define CHECK(e) do { ++checks; if (!(e)) { printf("FAIL %d: %s\n", __LINE__, #e); ++failures; } } while (0)

static uint32_t colors[4096];
static int color_count;
static xui_draw_rect_fill_proc fill_original;
static int capture_fill(xui_proxy p, xui_draw_context draw, xui_rect_t r, uint32_t color)
{
    if (color_count < 4096) colors[color_count++] = color;
    return fill_original(p, draw, r, color);
}
static int seen(uint32_t color)
{
    int i;
    for (i = 0; i < color_count; ++i) if (colors[i] == color) return 1;
    return 0;
}
static xui_style_property_t prop(const char* name, uint32_t color)
{
    xui_style_property_t p;
    memset(&p, 0, sizeof(p));
    p.iSize = sizeof(p); p.sName = name;
    p.tValue.iSize = sizeof(p.tValue); p.tValue.iType = XUI_STYLE_VALUE_COLOR;
    p.tValue.iColor = color;
    return p;
}
static xui_style_desc_t style(xui_style_property_t* props, int count)
{
    xui_style_desc_t s;
    memset(&s, 0, sizeof(s)); s.iSize = sizeof(s);
    s.pProperties = props; s.iPropertyCount = count;
    return s;
}
static xui_context setup(xui_test_proxy_state_t* proxy)
{
    xui_context c = NULL;
    xuiTestProxyInit(proxy);
    fill_original = proxy->tProxy.drawRectFill;
    proxy->tProxy.drawRectFill = capture_fill;
    CHECK(xuiCreate(&c) == XUI_OK);
    CHECK(xuiSetProxy(c, &proxy->tProxy) == XUI_OK);
    CHECK(xuiSetViewportSize(c, 640, 420) == XUI_OK);
    return c;
}
static void paint(xui_context c)
{
    color_count = 0;
    CHECK(xuiRenderPrepare(c) == XUI_OK);
}
static void paint_only(xui_widget widget)
{
    CHECK((xuiWidgetGetDirtyFlags(widget) & (XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_TREE)) == 0);
}
static void scroll_styles(void)
{
    xui_test_proxy_state_t proxy;
    xui_context c = setup(&proxy);
    xui_widget view, frame, bar;
    xui_scroll_view_desc_t desc;
    xui_style_property_t p[3];
    xui_style_desc_t s;
    xui_rect_t before;
    uint32_t base_bg = 0x123456ff, base_corner = 0x234567ff, base_grip = 0x345678ff;
    memset(&desc, 0, sizeof(desc)); desc.iSize = sizeof(desc);
    desc.fContentWidth = 1200; desc.fContentHeight = 900;
    desc.iScrollbarMode = XUI_SCROLLBAR_MODE_FULL; desc.fScrollbarSize = 16;
    desc.iCornerMode = XUI_SCROLL_FRAME_CORNER_GRIP;
    CHECK(xuiScrollViewCreate(c, &view, &desc) == XUI_OK);
    CHECK(xuiSetRootWidget(c, view) == XUI_OK);
    frame = xuiScrollViewGetFrameWidget(view);
    bar = xuiScrollFrameGetVScrollBarWidget(frame);
    CHECK(xuiScrollViewSetBackgroundColor(view, base_bg) == XUI_OK);
    CHECK(xuiScrollViewSetCornerColors(view, base_corner, base_grip) == XUI_OK);
    paint(c);
    CHECK(seen(base_bg) && seen(base_corner) && seen(base_grip));
    before = xuiScrollFrameGetViewportRect(frame);

    p[0] = prop("scrollframe.background.color", 0x445566ff);
    p[1] = prop("scrollframe.corner.color", 0x556677ff);
    p[2] = prop("scrollframe.grip.color", 0x667788ff);
    CHECK(xuiStyleSetDefault(c, p, 3) == XUI_OK);
    paint_only(frame); paint(c);
    CHECK(seen(p[0].tValue.iColor) && seen(p[1].tValue.iColor) && seen(p[2].tValue.iColor));
    CHECK(xuiScrollViewGetBackgroundColor(view) == base_bg);

    p[0] = prop("scrollview.background.color", 0x778899ff);
    p[1] = prop("scrollview.corner.color", 0x8899aaff);
    p[2] = prop("scrollview.grip.color", 0x99aabbff);
    s = style(p, 3);
    CHECK(xuiStyleSetType(c, xuiScrollViewGetType(c), &s) == XUI_OK);
    paint_only(view); paint(c);
    CHECK(seen(p[0].tValue.iColor) && seen(p[1].tValue.iColor) && seen(p[2].tValue.iColor));
    p[0].tValue.iType = XUI_STYLE_VALUE_TOKEN; p[0].tValue.sText = "containers.bg";
    {
        xui_style_value_t token = prop("unused", 0xabcdefff).tValue;
        CHECK(xuiStyleSetToken(c, "containers.bg", &token) == XUI_OK);
        CHECK(xuiStyleSetClass(c, "containers", &s) == XUI_OK);
        CHECK(xuiWidgetAddStyleClass(view, "containers") == XUI_OK);
        paint(c); CHECK(seen(token.iColor));
        token.iColor = 0x987654ff;
        CHECK(xuiStyleSetToken(c, "containers.bg", &token) == XUI_OK);
        paint_only(view); paint(c); CHECK(seen(token.iColor));
    }
    p[0] = prop("scrollview.background.color", 0);
    p[1] = prop("scrollview.corner.color", 0);
    p[2] = prop("scrollview.grip.color", 0);
    CHECK(xuiWidgetSetInlineStyle(view, p, 3) == XUI_OK);
    paint_only(view); paint(c);
    CHECK(!seen(base_bg) && !seen(base_corner) && !seen(base_grip));
    CHECK(!seen(0x987654ff) && !seen(0x8899aaff) && !seen(0x99aabbff));
    CHECK(xuiScrollFrameGetBackgroundColor(frame) == base_bg);
    CHECK(xuiWidgetSetInlineStyle(view, NULL, 0) == XUI_OK);
    xuiWidgetClearStyleClasses(view);
    CHECK(xuiStyleRemoveType(c, xuiScrollViewGetType(c)) == XUI_OK);
    CHECK(xuiStyleClearDefault(c) == XUI_OK);
    paint(c); CHECK(seen(base_bg) && seen(base_corner) && seen(base_grip));
    {
        xui_rect_t after = xuiScrollFrameGetViewportRect(frame);
        CHECK(memcmp(&before, &after, sizeof(before)) == 0);
    }

    /* The actual nested ScrollBar owns all thumb/track/button styling. */
    p[0] = prop("scrollbar.track.color", 0xa1b2c3ff);
    CHECK(xuiWidgetSetInlineStyle(bar, p, 1) == XUI_OK);
    paint_only(bar); paint(c); CHECK(seen(p[0].tValue.iColor));
    p[0].tValue.iColor = 0;
    CHECK(xuiWidgetSetInlineStyle(bar, p, 1) == XUI_OK);
    paint(c); CHECK(!seen(0xa1b2c3ff));
    CHECK(xuiWidgetSetInlineStyle(bar, NULL, 0) == XUI_OK);
    paint(c); CHECK(seen(0xdee8f3ff));
    p[0] = prop("scrollframe.background.color", 0);
    p[1] = prop("scrollframe.corner.color", 0);
    p[2] = prop("scrollframe.grip.color", 0);
    CHECK(xuiWidgetSetInlineStyle(frame, p, 3) == XUI_OK);
    paint_only(frame); paint(c);
    CHECK(!seen(base_bg) && !seen(base_corner) && !seen(base_grip));
    base_bg = 0x456789ff;
    CHECK(xuiScrollFrameSetBackgroundColor(frame, base_bg) == XUI_OK);
    paint(c); CHECK(!seen(base_bg));
    CHECK(xuiWidgetSetInlineStyle(frame, NULL, 0) == XUI_OK);
    paint(c); CHECK(seen(base_bg) && seen(base_corner) && seen(base_grip));
    xuiDestroy(c);
}
int main(void)
{
    scroll_styles();
    printf("style_containers: %d checks, %d failures\n", checks, failures);
    return failures ? 1 : 0;
}
