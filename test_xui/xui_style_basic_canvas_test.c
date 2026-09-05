#include "xui_style_basic_test.h"

static void stroke(xui_widget viewport)
{
	const int types[] = {XUI_EVENT_POINTER_DOWN, XUI_EVENT_POINTER_MOVE, XUI_EVENT_POINTER_UP};
	xui_rect_t world = xuiWidgetGetWorldRect(viewport);
	int i;
	basic_count = 0;
	for (i = 0; i < 3; ++i) {
		xui_widget_event_proc proc = NULL;
		void* user = NULL;
		xui_event_t e = {0};
		e.iType = types[i]; e.iButton = XUI_POINTER_BUTTON_LEFT;
		e.fX = world.fX + 10 + i * 2; e.fY = world.fY + 10 + i * 2;
		PIXEL_CHECK(xuiWidgetGetEventHandler(viewport, e.iType, &proc, &user) == XUI_OK && proc != NULL);
		if (proc) (void)proc(viewport, &e, user);
	}
}

int main(void)
{
	pixel_fixture_t f;
	xui_widget w = NULL, viewport;
	xui_canvas_desc_t d = {0};
	xui_style_property_t p;
	xui_style_desc_t s;
	uint32_t base;
	int saved_draw_count, i;
	if (!basic_init(&f)) return 1;
	d.iSize = sizeof(d); d.fCanvasWidth = 50; d.fCanvasHeight = 30;
	d.fViewportWidth = 180; d.fViewportHeight = 100; d.bPenEnabled = 1;
	d.iBackgroundColor = 0x246813ffu; d.iBorderColor = 0x135724ffu;
	PIXEL_CHECK(xuiCanvasCreate(f.context, &w, &d) == XUI_OK); basic_attach(&f, w);
	PIXEL_CHECK(xuiWidgetSetRect(w, (xui_rect_t){10, 10, 180, 100}) == XUI_OK);
	viewport = xuiCanvasGetViewportWidget(w); PIXEL_CHECK(viewport != NULL);
	basic_render(&f);
	basic_case_on(&f, w, viewport, "canvas.background.color", 0, BASIC_FILL);
	basic_case_on(&f, w, viewport, "canvas.border.color", 0, BASIC_STROKE);
	basic_cached_case(&f, w, "canvas.background.color", BASIC_FILL);
	PIXEL_CHECK(xuiCanvasDrawRectFill(w, (xui_rect_t){0, 0, 12, 12}, 0x654321ffu) == XUI_OK);
	saved_draw_count = xuiTestSurfaceGetDrawCount(xuiCanvasGetSurface(w));
	p = basic_color("canvas.background.color", 0x31415987u);
	PIXEL_CHECK(xuiWidgetSetInlineStyle(w, &p, 1) == XUI_OK);
	basic_render(&f); PIXEL_CHECK(basic_seen(BASIC_FILL, 0x31415987u) > 0);
	p.tValue.iColor = 0;
	PIXEL_CHECK(xuiWidgetSetInlineStyle(w, &p, 1) == XUI_OK);
	basic_render(&f); PIXEL_CHECK(basic_seen(BASIC_FILL, d.iBackgroundColor) == 0);
	PIXEL_CHECK(xuiWidgetSetInlineStyle(w, NULL, 0) == XUI_OK);
	basic_render(&f); PIXEL_CHECK(basic_seen(BASIC_FILL, d.iBackgroundColor) > 0);
	PIXEL_CHECK(xuiTestSurfaceGetDrawCount(xuiCanvasGetSurface(w)) == saved_draw_count);
	PIXEL_CHECK(xuiCanvasGetPen(w, NULL, NULL, &base) == XUI_OK);
	p = basic_color("canvas.pen.color", 0x12345678u); s = basic_style(&p, 1);
	PIXEL_CHECK(xuiStyleSetDefault(f.context, &p, 1) == XUI_OK);
	stroke(viewport); PIXEL_CHECK(basic_seen(BASIC_LINE, p.tValue.iColor) > 0);
	p.tValue.iColor = 0x23456789u;
	PIXEL_CHECK(xuiStyleSetType(f.context, xuiWidgetGetType(w), &s) == XUI_OK);
	stroke(viewport); PIXEL_CHECK(basic_seen(BASIC_LINE, p.tValue.iColor) > 0);
	p.tValue.iColor = 0x3456789au;
	PIXEL_CHECK(xuiStyleSetClass(f.context, "pen", &s) == XUI_OK);
	PIXEL_CHECK(xuiWidgetAddStyleClass(w, "pen") == XUI_OK);
	stroke(viewport); PIXEL_CHECK(basic_seen(BASIC_LINE, p.tValue.iColor) > 0);
	p.tValue.iColor = 0x456789abu;
	PIXEL_CHECK(xuiStyleSetToken(f.context, "pen", &p.tValue) == XUI_OK);
	p.tValue.iType = XUI_STYLE_VALUE_TOKEN; p.tValue.sText = "pen";
	PIXEL_CHECK(xuiWidgetSetInlineStyle(w, &p, 1) == XUI_OK);
	stroke(viewport); PIXEL_CHECK(basic_seen(BASIC_LINE, 0x456789abu) > 0);
	p.tValue.iType = XUI_STYLE_VALUE_COLOR;
	for (i = 0; i < 2; ++i) {
		p.tValue.iColor = i ? 0 : 0x12345600u;
		PIXEL_CHECK(xuiStyleSetToken(f.context, "pen", &p.tValue) == XUI_OK);
		stroke(viewport); PIXEL_CHECK(basic_kind_count(BASIC_LINE) == 0);
	}
	PIXEL_CHECK(xuiWidgetSetInlineStyle(w, NULL, 0) == XUI_OK);
	PIXEL_CHECK(xuiWidgetRemoveStyleClass(w, "pen") == XUI_OK);
	PIXEL_CHECK(xuiStyleRemoveClass(f.context, "pen") == XUI_OK);
	PIXEL_CHECK(xuiStyleRemoveType(f.context, xuiWidgetGetType(w)) == XUI_OK);
	PIXEL_CHECK(xuiStyleClearDefault(f.context) == XUI_OK);
	stroke(viewport); PIXEL_CHECK(basic_seen(BASIC_LINE, base) > 0);
	p = basic_color("canvas.pen.color", 0xabcdef99u);
	PIXEL_CHECK(xuiStyleSetDefault(f.context, &p, 1) == XUI_OK);
	PIXEL_CHECK(xuiCanvasSetPen(w, 1, 2, 0xabcdef44u) == XUI_OK);
	stroke(viewport); PIXEL_CHECK(basic_seen(BASIC_LINE, 0xabcdef44u) > 0);
	PIXEL_CHECK(xuiCanvasSetPen(w, 1, 2, 0) == XUI_OK);
	stroke(viewport); PIXEL_CHECK(basic_kind_count(BASIC_LINE) == 0);
	basic_count = 0;
	PIXEL_CHECK(xuiCanvasDrawRectFill(w, (xui_rect_t){0, 0, 10, 10}, 0xdeadbeefu) == XUI_OK);
	PIXEL_CHECK(basic_seen(BASIC_FILL, 0xdeadbeefu) > 0);
	pixel_cleanup(&f);
	return pixel_result("xui_style_basic_canvas_test");
}
