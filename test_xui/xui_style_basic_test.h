#ifndef XUI_STYLE_BASIC_TEST_H
#define XUI_STYLE_BASIC_TEST_H

#include "xui_control_pixel_test.h"

#undef PIXEL_CHECK
#define PIXEL_CHECK(expr) do { ++g_checks; if (!(expr)) { \
	++g_failures; printf("%s:%d: %s\n", __FILE__, __LINE__, #expr); \
} } while (0)

enum { BASIC_FILL, BASIC_STROKE, BASIC_LINE, BASIC_CIRCLE, BASIC_RING, BASIC_TEXT, BASIC_SURFACE, BASIC_TRIANGLE, BASIC_SVG_FILL, BASIC_SVG_STROKE };
typedef struct basic_draw_t { int kind; uint32_t color; } basic_draw_t;
static basic_draw_t basic_draws[8192];
static int basic_count;
static xui_draw_context basic_filter_draw;
static xui_proxy_t basic_proxy;

static void basic_record(xui_draw_context draw, int kind, uint32_t color)
{
	if (basic_filter_draw != NULL && draw != basic_filter_draw) return;
	if (basic_count < (int)(sizeof(basic_draws) / sizeof(basic_draws[0]))) {
		basic_draws[basic_count++] = (basic_draw_t){kind, color};
	}
}

static int basic_fill(xui_proxy p, xui_draw_context d, xui_rect_t r, uint32_t c)
{ basic_record(d, BASIC_FILL, c); return basic_proxy.drawRectFill(p, d, r, c); }
static int basic_stroke(xui_proxy p, xui_draw_context d, xui_rect_t r, float w, uint32_t c)
{ basic_record(d, BASIC_STROKE, c); return basic_proxy.drawRectStroke(p, d, r, w, c); }
static int basic_line(xui_proxy p, xui_draw_context d, float x, float y, float u, float v, float w, uint32_t c)
{ basic_record(d, BASIC_LINE, c); return basic_proxy.drawLine(p, d, x, y, u, v, w, c); }
static int basic_circle(xui_proxy p, xui_draw_context d, float x, float y, float r, uint32_t c)
{ basic_record(d, BASIC_CIRCLE, c); return basic_proxy.drawCircleFill(p, d, x, y, r, c); }
static int basic_ring(xui_proxy p, xui_draw_context d, float x, float y, float r, float w, uint32_t c)
{ basic_record(d, BASIC_RING, c); return basic_proxy.drawCircleStroke(p, d, x, y, r, w, c); }
static int basic_text(xui_proxy p, xui_draw_context d, xui_font f, const char* s, xui_rect_t r, uint32_t c, uint32_t flags)
{ basic_record(d, BASIC_TEXT, c); return basic_proxy.drawText(p, d, f, s, r, c, flags); }
static int basic_surface(xui_proxy p, xui_draw_context d, xui_surface s, xui_rect_t a, xui_rect_t b, uint32_t c, uint32_t flags)
{ basic_record(d, BASIC_SURFACE, c); return basic_proxy.drawSurface(p, d, s, a, b, c, flags); }
static int basic_triangle(xui_proxy p, xui_draw_context d, xui_vec2_t a, xui_vec2_t b, xui_vec2_t c, uint32_t color)
{ basic_record(d, BASIC_TRIANGLE, color); return basic_proxy.drawTriangleFill(p, d, a, b, c, color); }
static int basic_svg(xui_proxy p, xui_draw_context d, const char* path, xui_rect_t view, xui_rect_t target, const xui_path_style_t* style, float tolerance)
{
	basic_record(d, BASIC_SVG_FILL, style->iFillColor);
	basic_record(d, BASIC_SVG_STROKE, style->iStrokeColor);
	return basic_proxy.drawSvgPath(p, d, path, view, target, style, tolerance);
}

static void basic_configure(xui_proxy p)
{
	basic_proxy = *p;
	p->drawRectFill = basic_fill; p->drawRectStroke = basic_stroke;
	p->drawLine = basic_line; p->drawCircleFill = basic_circle; p->drawCircleStroke = basic_ring;
	p->drawText = basic_text; p->drawSurface = basic_surface;
	p->drawTriangleFill = basic_triangle;
	p->drawSvgPath = basic_svg;
}

static int basic_seen(int kind, uint32_t color)
{
	int i, n = 0;
	for (i = 0; i < basic_count; ++i) if (basic_draws[i].kind == kind && basic_draws[i].color == color) ++n;
	return n;
}

static int basic_init(pixel_fixture_t* f)
{
	if (!pixel_init_proxy(f, basic_configure)) return 0;
	f->proxy.tProxy.surfaceDestroy(&f->proxy.tProxy, f->target);
	f->target = NULL;
	return xuiTestSurfaceCreate(&f->proxy, &f->target, 640, 480,
		XUI_SURFACE_USAGE_TARGET | XUI_SURFACE_ALPHA_PREMULTIPLIED) == XUI_OK;
}

static int basic_kind_count(int kind)
{
	int i, n = 0;
	for (i = 0; i < basic_count; ++i) if (basic_draws[i].kind == kind) ++n;
	return n;
}

static xui_style_property_t basic_color(const char* key, uint32_t color)
{
	xui_style_property_t p = {0};
	p.iSize = sizeof(p); p.sName = key;
	p.tValue.iSize = sizeof(p.tValue); p.tValue.iType = XUI_STYLE_VALUE_COLOR; p.tValue.iColor = color;
	return p;
}

static xui_style_desc_t basic_style(const xui_style_property_t* p, int count)
{
	xui_style_desc_t s = {0};
	s.iSize = sizeof(s); s.pProperties = p; s.iPropertyCount = count;
	return s;
}

static void basic_paint(pixel_fixture_t* f, xui_widget w, uint32_t state)
{
	xui_widget_cache_render_proc render = NULL;
	xui_draw_context draw = NULL;
	void* user = NULL;
	basic_count = 0;
	PIXEL_CHECK(xuiWidgetGetCacheRenderCallback(w, &render, &user) == XUI_OK && render != NULL);
	PIXEL_CHECK(f->proxy.tProxy.drawBegin(&f->proxy.tProxy, &draw, f->target) == XUI_OK);
	basic_filter_draw = draw;
	if (render && draw) PIXEL_CHECK(render(w, draw, state, user) == XUI_OK);
	basic_filter_draw = NULL;
	if (draw) PIXEL_CHECK(f->proxy.tProxy.drawEnd(&f->proxy.tProxy, draw) == XUI_OK);
}

static void basic_render(pixel_fixture_t* f)
{ basic_count = 0; PIXEL_CHECK(xuiRender(f->context, f->target, NULL, 0) == XUI_OK); }

static void basic_attach(pixel_fixture_t* f, xui_widget w)
{
	PIXEL_CHECK(xuiWidgetAddChild(f->root, w) == XUI_OK);
	PIXEL_CHECK(xuiWidgetSetRect(w, (xui_rect_t){10, 10, 240, 40}) == XUI_OK);
}

/* Test the installed painter, not resolved-property getters. Each key walks the
 * cascade, a live token update, both alpha-zero encodings, and exact restoration. */
static void basic_case_on(pixel_fixture_t* f, xui_widget w, xui_widget painted, const char* key, uint32_t state, int kind)
{
	const uint32_t colors[] = {0x19283791u, 0x28473683u, 0x37684575u, 0x46895467u, 0x57a06359u};
	basic_draw_t base[256];
	xui_style_property_t p = basic_color(key, colors[0]);
	xui_style_property_info_t info = {0};
	xui_style_desc_t s;
	int count, kind_count, failures = g_failures, i;
	info.iSize = sizeof(info);
	PIXEL_CHECK(xuiStyleGetPropertyInfo(f->context, xuiStyleFindProperty(f->context, key), &info) == XUI_OK);
	PIXEL_CHECK(info.iValueType == XUI_STYLE_VALUE_COLOR);
	PIXEL_CHECK(info.iDirtyFlags == (XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER));
	basic_paint(f, painted, state);
	count = 0; kind_count = basic_kind_count(kind);
	PIXEL_CHECK(kind_count <= 256 && kind_count > 0);
	if (kind_count > 256) return;
	for (i = 0; i < basic_count; ++i) if (basic_draws[i].kind == kind) base[count++] = basic_draws[i];
	PIXEL_CHECK(xuiStyleSetDefault(f->context, &p, 1) == XUI_OK);
	basic_paint(f, painted, state); PIXEL_CHECK(basic_seen(kind, colors[0]) > 0);
	p.tValue.iColor = colors[1]; s = basic_style(&p, 1);
	PIXEL_CHECK(xuiStyleSetType(f->context, xuiWidgetGetType(w), &s) == XUI_OK);
	basic_paint(f, painted, state); PIXEL_CHECK(basic_seen(kind, colors[1]) > 0);
	p.tValue.iColor = colors[2];
	PIXEL_CHECK(xuiStyleSetClass(f->context, "basic", &s) == XUI_OK);
	PIXEL_CHECK(xuiWidgetAddStyleClass(w, "basic") == XUI_OK);
	basic_paint(f, painted, state); PIXEL_CHECK(basic_seen(kind, colors[2]) > 0);
	p.tValue.iColor = colors[3];
	PIXEL_CHECK(xuiStyleSetToken(f->context, "basic.color", &p.tValue) == XUI_OK);
	p.tValue.iType = XUI_STYLE_VALUE_TOKEN; p.tValue.sText = "basic.color";
	PIXEL_CHECK(xuiWidgetSetInlineStyle(w, &p, 1) == XUI_OK);
	basic_paint(f, painted, state); PIXEL_CHECK(basic_seen(kind, colors[3]) > 0);
	p.tValue.iType = XUI_STYLE_VALUE_COLOR; p.tValue.iColor = colors[4];
	xuiWidgetClearDirty(w, 0);
	PIXEL_CHECK(xuiStyleSetToken(f->context, "basic.color", &p.tValue) == XUI_OK);
	basic_paint(f, painted, state); PIXEL_CHECK(basic_seen(kind, colors[4]) > 0);
	PIXEL_CHECK((xuiWidgetGetDirtyFlags(w) & XUI_WIDGET_DIRTY_LAYOUT) == 0);
	for (i = 0; i < 2; ++i) {
		p.tValue.iColor = i ? 0 : 0x12345600u;
		PIXEL_CHECK(xuiStyleSetToken(f->context, "basic.color", &p.tValue) == XUI_OK);
		basic_paint(f, painted, state);
		PIXEL_CHECK(basic_kind_count(kind) < kind_count || basic_seen(kind, p.tValue.iColor) > 0);
		PIXEL_CHECK(basic_seen(kind, colors[2]) == 0);
	}
	PIXEL_CHECK(xuiWidgetSetInlineStyle(w, NULL, 0) == XUI_OK);
	basic_paint(f, painted, state); PIXEL_CHECK(basic_seen(kind, colors[2]) > 0);
	PIXEL_CHECK(xuiWidgetRemoveStyleClass(w, "basic") == XUI_OK);
	PIXEL_CHECK(xuiStyleRemoveClass(f->context, "basic") == XUI_OK);
	PIXEL_CHECK(xuiStyleRemoveType(f->context, xuiWidgetGetType(w)) == XUI_OK);
	PIXEL_CHECK(xuiStyleClearDefault(f->context) == XUI_OK);
	basic_paint(f, painted, state);
	PIXEL_CHECK(basic_kind_count(kind) == count);
	count = 0;
	for (i = 0; i < basic_count; ++i) if (basic_draws[i].kind == kind) {
		PIXEL_CHECK(count < kind_count && basic_draws[i].color == base[count].color);
		++count;
	}
	if (g_failures != failures) printf("color case failed: %s state %u\n", key, state);
}

static void basic_case(pixel_fixture_t* f, xui_widget w, const char* key, uint32_t state, int kind)
{ basic_case_on(f, w, w, key, state, kind); }

static void basic_cached_case(pixel_fixture_t* f, xui_widget w, const char* key, int kind)
{
	xui_style_property_t p = basic_color(key, 0x21436587u);
	xui_style_desc_t s = basic_style(&p, 1);
	PIXEL_CHECK(xuiStyleSetToken(f->context, "basic.live", &p.tValue) == XUI_OK);
	p.tValue.iType = XUI_STYLE_VALUE_TOKEN; p.tValue.sText = "basic.live";
	PIXEL_CHECK(xuiStyleSetType(f->context, xuiWidgetGetType(w), &s) == XUI_OK);
	basic_render(f); PIXEL_CHECK(basic_seen(kind, 0x21436587u) > 0);
	basic_render(f); PIXEL_CHECK(basic_seen(kind, 0x21436587u) == 0);
	p.tValue.iType = XUI_STYLE_VALUE_COLOR; p.tValue.iColor = 0x32547698u;
	PIXEL_CHECK(xuiStyleSetToken(f->context, "basic.live", &p.tValue) == XUI_OK);
	basic_render(f); PIXEL_CHECK(basic_seen(kind, 0x32547698u) > 0);
	PIXEL_CHECK(xuiStyleRemoveType(f->context, xuiWidgetGetType(w)) == XUI_OK);
	basic_render(f); PIXEL_CHECK(basic_seen(kind, 0x32547698u) == 0);
}

#endif
