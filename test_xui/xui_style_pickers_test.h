#ifndef XUI_STYLE_PICKERS_TEST_H
#define XUI_STYLE_PICKERS_TEST_H
#include "src/xui_internal.h"
#include "xui_test_proxy.h"
#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <stdlib.h>

static int checks, failures;
static const char* phase = "setup";
#define CHECK(e) do { ++checks; if (!(e)) { ++failures; \
    printf("FAIL %s:%d: %s\n", phase, __LINE__, #e); } } while (0)
static xui_context context;
static xui_widget root;
static xui_surface target;
static xui_font font;
static xui_test_proxy_state_t proxy;
static xui_proxy_t original;
static uint32_t painted[32768];
static int painted_count;
static void record_color(uint32_t color)
{
    if (painted_count < (int)(sizeof(painted) / sizeof(painted[0]))) painted[painted_count++] = color;
}
static int has_color(uint32_t color)
{
    int i;
    for (i = 0; i < painted_count; ++i) if (painted[i] == color) return 1;
    return 0;
}
static int fill(xui_proxy p, xui_draw_context d, xui_rect_t r, uint32_t c)
{ record_color(c); return original.drawRectFill(p, d, r, c); }
static int stroke(xui_proxy p, xui_draw_context d, xui_rect_t r, float w, uint32_t c)
{ record_color(c); return original.drawRectStroke(p, d, r, w, c); }
static int line(xui_proxy p, xui_draw_context d, float x1, float y1, float x2, float y2, float w, uint32_t c)
{ record_color(c); return original.drawLine(p, d, x1, y1, x2, y2, w, c); }
static int circle_fill(xui_proxy p, xui_draw_context d, float x, float y, float r, uint32_t c)
{ record_color(c); return original.drawCircleFill(p, d, x, y, r, c); }
static int circle_stroke(xui_proxy p, xui_draw_context d, float x, float y, float r, float w, uint32_t c)
{ record_color(c); return original.drawCircleStroke(p, d, x, y, r, w, c); }
static int triangle(xui_proxy p, xui_draw_context d, xui_vec2_t a, xui_vec2_t b, xui_vec2_t c, uint32_t color)
{ record_color(color); return original.drawTriangleFill(p, d, a, b, c, color); }
static int draw_text(xui_proxy p, xui_draw_context d, xui_font f, const char* s, xui_rect_t r, uint32_t c, uint32_t flags)
{ record_color(c); return original.drawText(p, d, f, s, r, c, flags); }
static void setup(void)
{
    xuiTestProxyInit(&proxy);
    original = proxy.tProxy;
    proxy.tProxy.drawRectFill = fill;
    proxy.tProxy.drawRectStroke = stroke;
    proxy.tProxy.drawLine = line;
    proxy.tProxy.drawCircleFill = circle_fill;
    proxy.tProxy.drawCircleStroke = circle_stroke;
    proxy.tProxy.drawTriangleFill = triangle;
    proxy.tProxy.drawText = draw_text;
    CHECK(xuiCreate(&context) == XUI_OK);
    CHECK(xuiSetProxy(context, &proxy.tProxy) == XUI_OK);
    CHECK(proxy.tProxy.fontLoadMemory(&proxy.tProxy, &font, "test", 4, 14.0f, XUI_FONT_FORMAT_TTF) == XUI_OK);
    CHECK(xuiSetDefaultFont(context, font) == XUI_OK);
    CHECK(xuiInputViewport(context, 1000.0f, 800.0f) == XUI_OK);
    CHECK(xuiWidgetCreate(context, &root) == XUI_OK);
    CHECK(xuiWidgetSetRect(root, (xui_rect_t){0, 0, 1000, 800}) == XUI_OK);
    CHECK(xuiSetRootWidget(context, root) == XUI_OK);
    CHECK(xuiTestSurfaceCreate(&proxy, &target, 1000, 800, XUI_SURFACE_USAGE_TARGET) == XUI_OK);
}
static void attach(xui_widget w)
{
    CHECK(xuiWidgetSetRect(w, (xui_rect_t){30, 20, 240, 32}) == XUI_OK);
    CHECK(xuiWidgetAddChild(root, w) == XUI_OK);
    CHECK(xuiLayout(context) == XUI_OK);
}
static void paint(void)
{
    xui_rect_i_t r = {0, 0, 1000, 800};
    CHECK(xuiUpdate(context, 0.0f) == XUI_OK);
    painted_count = 0;
    CHECK(xuiRender(context, target, &r, 1) == XUI_OK);
}
static void settle(void)
{
    CHECK(xuiUpdate(context, 0.016f) == XUI_OK);
    CHECK(xuiLayout(context) == XUI_OK);
    paint();
    CHECK(xuiLayout(context) == XUI_OK);
    paint();
}
static int finish(void)
{
    xuiDestroy(context);
    proxy.tProxy.surfaceDestroy(&proxy.tProxy, target);
    proxy.tProxy.fontDestroy(&proxy.tProxy, font);
    printf("style_pickers: %d checks, %d failures\n", checks, failures);
    return failures ? 1 : 0;
}
static xui_style_property_t property(const char* key, uint32_t color)
{
    xui_style_property_t p;
    memset(&p, 0, sizeof(p));
    p.iSize = sizeof(p);
    p.sName = key;
    p.tValue.iSize = sizeof(p.tValue);
    p.tValue.iType = XUI_STYLE_VALUE_COLOR;
    p.tValue.iColor = color;
    return p;
}
static void inline_color(xui_widget w, const char* key, uint32_t color)
{
    xui_style_property_t p = property(key, color);
    CHECK(xuiWidgetSetInlineStyle(w, &p, 1) == XUI_OK);
}
typedef struct color_case { const char* key; size_t offset; } color_case;
typedef uint32_t (*resolve_color_proc)(xui_widget, size_t);
static void test_colors(xui_widget w, const void* base, const color_case* cases, int count, resolve_color_proc resolve)
{
    int i;
    xui_widget_type type = xuiWidgetGetType(w);
    for (i = 0; i < count; ++i) {
        const color_case* row = &cases[i];
        uint32_t before = *(const uint32_t*)((const char*)base + row->offset);
        xui_style_property_info_t info = {0};
        xui_style_property_t p = property(row->key, 0x123456ffu);
        xui_style_desc_t style = {0};
        xui_style_value_t token = {0};
        phase = row->key;
        style.iSize = sizeof(style);
        style.pProperties = &p;
        style.iPropertyCount = 1;
        CHECK(xuiStyleGetPropertyInfo(context, xuiStyleFindProperty(context, row->key), &info) == XUI_OK);
        CHECK(info.iValueType == XUI_STYLE_VALUE_COLOR);
        CHECK(info.iDirtyFlags == (XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER));
        settle();
        CHECK(xuiStyleSetDefault(context, &p, 1) == XUI_OK);
        CHECK(resolve(w, row->offset) == 0x123456ffu);
        CHECK((w->iDirtyFlags & XUI_WIDGET_DIRTY_LAYOUT) == 0);
        p.tValue.iColor = 0x234567ffu;
        CHECK(xuiStyleSetType(context, type, &style) == XUI_OK);
        CHECK(resolve(w, row->offset) == 0x234567ffu);
        p.tValue.iColor = 0x345678ffu;
        CHECK(xuiStyleSetClass(context, "picker-test", &style) == XUI_OK);
        CHECK(xuiWidgetAddStyleClass(w, "picker-test") == XUI_OK);
        CHECK(resolve(w, row->offset) == 0x345678ffu);
        p.tValue.iType = XUI_STYLE_VALUE_TOKEN;
        p.tValue.sText = "picker-ink";
        token.iType = XUI_STYLE_VALUE_COLOR;
        token.iSize = sizeof(token);
        token.iColor = 0x456789ffu;
        CHECK(xuiStyleSetToken(context, "picker-ink", &token) == XUI_OK);
        CHECK(xuiWidgetSetInlineStyle(w, &p, 1) == XUI_OK);
        CHECK(resolve(w, row->offset) == token.iColor);
        token.iColor = 0;
        CHECK(xuiStyleSetToken(context, "picker-ink", &token) == XUI_OK);
        CHECK(resolve(w, row->offset) == 0);
        token.iColor = 0xabcdef00u;
        CHECK(xuiStyleSetToken(context, "picker-ink", &token) == XUI_OK);
        CHECK(resolve(w, row->offset) == 0xabcdef00u);
        CHECK(xuiWidgetSetInlineStyle(w, NULL, 0) == XUI_OK);
        CHECK(resolve(w, row->offset) == 0x345678ffu);
        CHECK(xuiStyleRemoveClass(context, "picker-test") == XUI_OK);
        CHECK(resolve(w, row->offset) == 0x234567ffu);
        CHECK(xuiStyleRemoveType(context, type) == XUI_OK);
        CHECK(resolve(w, row->offset) == 0x123456ffu);
        CHECK(xuiStyleClearDefault(context) == XUI_OK);
        CHECK(resolve(w, row->offset) == before);
        CHECK(*(const uint32_t*)((const char*)base + row->offset) == before);
        CHECK(xuiStyleRemoveToken(context, "picker-ink") == XUI_OK);
        xuiWidgetClearStyleClasses(w);
    }
}
#endif
