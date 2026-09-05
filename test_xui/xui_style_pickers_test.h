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
static xui_surface painted_targets[32768];
static int painted_count;
static struct { xui_draw_context draw; xui_surface surface; } draws[32];
static int begin_draw(xui_proxy p, xui_draw_context* d, xui_surface surface)
{
    int i, result = original.drawBegin(p, d, surface);
    if (result != XUI_OK) return result;
    for (i = 0; i < 32; ++i) if (draws[i].draw == NULL) {
        draws[i].draw = *d; draws[i].surface = surface;
        return result;
    }
    CHECK(0 && "draw recorder capacity");
    return result;
}
static int end_draw(xui_proxy p, xui_draw_context d)
{
    int i;
    for (i = 0; i < 32; ++i) if (draws[i].draw == d) draws[i].draw = NULL;
    return original.drawEnd(p, d);
}
static void record_color(xui_draw_context d, uint32_t color)
{
    int i;
    xui_surface surface = NULL;
    for (i = 0; i < 32; ++i) if (draws[i].draw == d) surface = draws[i].surface;
    if (painted_count >= (int)(sizeof(painted) / sizeof(painted[0]))) {
        CHECK(0 && "paint recorder capacity");
        return;
    }
    painted_targets[painted_count] = surface;
    painted[painted_count++] = color;
}
static int has_color(uint32_t color)
{
    int i;
    for (i = 0; i < painted_count; ++i) if (painted[i] == color) return 1;
    return 0;
}
static int cache_has_color(xui_widget w, uint32_t color)
{
    xui_surface surface = xuiWidgetGetCacheSurface(w, xuiWidgetGetStateId(w));
    int i;
    if (surface == NULL) return 0;
    for (i = 0; i < painted_count; ++i)
        if (painted[i] == color && painted_targets[i] == surface) return 1;
    return 0;
}
static int fill(xui_proxy p, xui_draw_context d, xui_rect_t r, uint32_t c)
{ record_color(d, c); return original.drawRectFill(p, d, r, c); }
static int stroke(xui_proxy p, xui_draw_context d, xui_rect_t r, float w, uint32_t c)
{ record_color(d, c); return original.drawRectStroke(p, d, r, w, c); }
static int line(xui_proxy p, xui_draw_context d, float x1, float y1, float x2, float y2, float w, uint32_t c)
{ record_color(d, c); return original.drawLine(p, d, x1, y1, x2, y2, w, c); }
static int circle_fill(xui_proxy p, xui_draw_context d, float x, float y, float r, uint32_t c)
{ record_color(d, c); return original.drawCircleFill(p, d, x, y, r, c); }
static int circle_stroke(xui_proxy p, xui_draw_context d, float x, float y, float r, float w, uint32_t c)
{ record_color(d, c); return original.drawCircleStroke(p, d, x, y, r, w, c); }
static int triangle(xui_proxy p, xui_draw_context d, xui_vec2_t a, xui_vec2_t b, xui_vec2_t c, uint32_t color)
{ record_color(d, color); return original.drawTriangleFill(p, d, a, b, c, color); }
static int draw_text(xui_proxy p, xui_draw_context d, xui_font f, const char* s, xui_rect_t r, uint32_t c, uint32_t flags)
{ record_color(d, c); return original.drawText(p, d, f, s, r, c, flags); }
static int draw_svg(xui_proxy p, xui_draw_context d, const char* path, xui_rect_t view, xui_rect_t rect, const xui_path_style_t* style, float tolerance)
{
    record_color(d, style->iFillColor);
    record_color(d, style->iStrokeColor);
    return original.drawSvgPath(p, d, path, view, rect, style, tolerance);
}
static void setup(void)
{
    xuiTestProxyInit(&proxy);
    original = proxy.tProxy;
    proxy.tProxy.drawBegin = begin_draw;
    proxy.tProxy.drawEnd = end_draw;
    proxy.tProxy.drawRectFill = fill;
    proxy.tProxy.drawRectStroke = stroke;
    proxy.tProxy.drawLine = line;
    proxy.tProxy.drawCircleFill = circle_fill;
    proxy.tProxy.drawCircleStroke = circle_stroke;
    proxy.tProxy.drawTriangleFill = triangle;
    proxy.tProxy.drawText = draw_text;
    proxy.tProxy.drawSvgPath = draw_svg;
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
static void test_live_token(xui_widget w, const char* key, uint32_t base)
{
    xui_style_property_t p = property(key, 0), resolved_property = {0};
    xui_style_desc_t style = {0};
    xui_style_value_t token = property(key, 0xb6789affu).tValue;
    phase = key;
    style.iSize = sizeof(style); style.pProperties = &p; style.iPropertyCount = 1;
    p.tValue.iType = XUI_STYLE_VALUE_TOKEN; p.tValue.sText = "picker-live-ink";
    CHECK(xuiStyleSetToken(context, "picker-live-ink", &token) == XUI_OK);
    CHECK(xuiStyleSetClass(context, "picker-live", &style) == XUI_OK);
    CHECK(xuiWidgetAddStyleClass(w, "picker-live") == XUI_OK);
    paint(); CHECK(has_color(token.iColor));
    token.iColor = 0;
    CHECK(xuiStyleSetToken(context, "picker-live-ink", &token) == XUI_OK);
    paint(); CHECK(!has_color(0xb6789affu));
    CHECK(xuiWidgetGetResolvedStyleProperty(w, key, &resolved_property) == XUI_OK);
    CHECK(resolved_property.tValue.iColor == 0);
    token.iColor = 0xb789abffu;
    CHECK(xuiStyleSetToken(context, "picker-live-ink", &token) == XUI_OK);
    paint(); CHECK(has_color(token.iColor));
    CHECK(xuiStyleRemoveClass(context, "picker-live") == XUI_OK);
    paint(); CHECK(has_color(base));
    CHECK(xuiWidgetRemoveStyleClass(w, "picker-live") == XUI_OK);
    CHECK(xuiStyleRemoveToken(context, "picker-live-ink") == XUI_OK);
}
static void prepare_only(void)
{
    painted_count = 0;
    CHECK(xuiRenderPrepare(context) == XUI_OK);
}
typedef int (*picker_open_close_proc)(xui_widget);
static void test_owner_colors(xui_widget w, const void* base, const color_case* cases, int count,
    picker_open_close_proc open_picker, picker_open_close_proc close_picker)
{
    int i;
    for (i = 0; i < count; ++i) {
        uint32_t value = 0xf12345ffu + (uint32_t)i * 0x00010000u;
        uint32_t before = *(const uint32_t*)((const char*)base + cases[i].offset);
        const char* key = cases[i].key;
        if (strstr(key, "placeholder") != NULL) continue;
        phase = key;
        CHECK(close_picker(w) == XUI_OK);
        CHECK(xuiWidgetSetEnabled(w, 1) == XUI_OK);
        CHECK(xuiSetFocusWidget(context, NULL) == XUI_OK);
        CHECK(xuiInputPointerMove(context, 900, 700, 0) == XUI_OK);
        CHECK(xuiDispatchPendingEvents(context) == XUI_OK);
        if (strstr(key, "disabled") != NULL) CHECK(xuiWidgetSetEnabled(w, 0) == XUI_OK);
        if (strstr(key, "hover") != NULL) {
            CHECK(xuiInputPointerMove(context, 40, 30, 0) == XUI_OK);
            CHECK(xuiDispatchPendingEvents(context) == XUI_OK);
        }
        if (strstr(key, "focus") != NULL) CHECK(xuiSetFocusWidget(context, w) == XUI_OK);
        if (strstr(key, "open") != NULL) CHECK(open_picker(w) == XUI_OK);
        inline_color(w, key, value);
        prepare_only(); CHECK(cache_has_color(w, value));
        inline_color(w, key, 0);
        prepare_only(); CHECK(!cache_has_color(w, value));
        inline_color(w, key, 0xabcdef00u);
        prepare_only(); CHECK(!cache_has_color(w, value) && !cache_has_color(w, 0xabcdefffu));
        CHECK(xuiWidgetSetInlineStyle(w, NULL, 0) == XUI_OK);
        prepare_only(); CHECK(cache_has_color(w, before));
    }
    CHECK(close_picker(w) == XUI_OK);
    CHECK(xuiWidgetSetEnabled(w, 1) == XUI_OK);
    CHECK(xuiSetFocusWidget(context, NULL) == XUI_OK);
    CHECK(xuiInputPointerMove(context, 900, 700, 0) == XUI_OK);
    CHECK(xuiDispatchPendingEvents(context) == XUI_OK);
}
static void test_prepare_only(xui_widget w, xui_widget dependent, const char* key, uint32_t base)
{
    xui_style_property_t p = property(key, 0xa98765ffu);
    xui_style_desc_t style = {0};
    xui_style_value_t token = property(key, 0xd98765ffu).tValue;
    xui_widget_type menu_type = xuiMenuGetType(context);
    int modern_menu = xuiStyleFindProperty(context, "menu.panel.color") != 0;
    phase = key;
    CHECK(xuiWidgetGetType(w)->onPreparePaint != NULL);
    CHECK(xuiWidgetSetInlineStyle(w, NULL, 0) == XUI_OK);
    settle();
    style.iSize = sizeof(style); style.pProperties = &p; style.iPropertyCount = 1;
    CHECK(xuiStyleSetDefault(context, &p, 1) == XUI_OK);
    prepare_only(); CHECK(cache_has_color(dependent, p.tValue.iColor));
    p.tValue.iColor = 0xb98765ffu;
    CHECK(xuiStyleSetType(context, xuiWidgetGetType(w), &style) == XUI_OK);
    prepare_only(); CHECK(cache_has_color(dependent, p.tValue.iColor));
    p.tValue.iColor = 0xc98765ffu;
    CHECK(xuiStyleSetClass(context, "picker-prepare", &style) == XUI_OK);
    CHECK(xuiWidgetAddStyleClass(w, "picker-prepare") == XUI_OK);
    prepare_only(); CHECK(cache_has_color(dependent, p.tValue.iColor));
    p.tValue.iType = XUI_STYLE_VALUE_TOKEN; p.tValue.sText = "picker-prepare-ink";
    CHECK(xuiStyleSetToken(context, "picker-prepare-ink", &token) == XUI_OK);
    CHECK(xuiWidgetSetInlineStyle(w, &p, 1) == XUI_OK);
    prepare_only(); CHECK(cache_has_color(dependent, token.iColor));
    token.iColor = 0;
    CHECK(xuiStyleSetToken(context, "picker-prepare-ink", &token) == XUI_OK);
    prepare_only(); CHECK(!cache_has_color(dependent, 0xd98765ffu));
    token.iColor = 0xabcdef00u;
    CHECK(xuiStyleSetToken(context, "picker-prepare-ink", &token) == XUI_OK);
    prepare_only(); CHECK(!cache_has_color(dependent, 0xd98765ffu) && !cache_has_color(dependent, 0xabcdefffu));
    CHECK(xuiWidgetSetInlineStyle(w, NULL, 0) == XUI_OK);
    prepare_only(); CHECK(cache_has_color(dependent, 0xc98765ffu));
    CHECK(xuiStyleRemoveClass(context, "picker-prepare") == XUI_OK);
    prepare_only(); CHECK(cache_has_color(dependent, 0xb98765ffu));
    CHECK(xuiStyleRemoveType(context, xuiWidgetGetType(w)) == XUI_OK);
    prepare_only(); CHECK(cache_has_color(dependent, 0xa98765ffu));
    CHECK(xuiStyleClearDefault(context) == XUI_OK);
    prepare_only(); CHECK(cache_has_color(dependent, base));
    CHECK((w->iDirtyFlags & XUI_WIDGET_DIRTY_LAYOUT) == 0);
    /* The legacy menu setter requests layout; cd8278c makes it paint-only. */
    if (modern_menu || xuiWidgetGetType(dependent) != menu_type)
        CHECK((dependent->iDirtyFlags & XUI_WIDGET_DIRTY_LAYOUT) == 0);
    if (modern_menu) {
        prepare_only(); CHECK(painted_count == 0);
    }
    CHECK(xuiWidgetRemoveStyleClass(w, "picker-prepare") == XUI_OK);
    CHECK(xuiStyleRemoveToken(context, "picker-prepare-ink") == XUI_OK);
}
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
