#include "xui_style_basic_test.h"

static void text_cases(pixel_fixture_t* f)
{
	xui_widget label = NULL, link = NULL;
	xui_label_desc_t ld = {0}; xui_hyperlink_desc_t hd = {0};
	xui_style_property_t p[2];
	ld.iSize = sizeof(ld); ld.sText = "Label"; ld.iTextColor = 0x246813ffu;
	hd.iSize = sizeof(hd); hd.sText = "Link"; hd.iTextColor = 0x135724ffu;
	PIXEL_CHECK(xuiLabelCreate(f->context, &label, &ld) == XUI_OK); basic_attach(f, label);
	PIXEL_CHECK(xuiHyperlinkCreate(f->context, &link, &hd) == XUI_OK); basic_attach(f, link);
	basic_case(f, label, "text.color", 0, BASIC_TEXT);
	PIXEL_CHECK(xuiWidgetSetEnabled(label, 0) == XUI_OK);
	basic_case(f, label, "text.disabled_color", 0, BASIC_TEXT);
	PIXEL_CHECK(xuiWidgetSetEnabled(label, 1) == XUI_OK);
	basic_cached_case(f, label, "text.color", BASIC_TEXT);
	basic_case(f, link, "text.color", 0, BASIC_TEXT);
	basic_case(f, link, "hyperlink.text.color", 0, BASIC_TEXT);
	basic_case(f, link, "hyperlink.text.hover_color", XUI_WIDGET_STATE_HOVER, BASIC_TEXT);
	basic_case(f, link, "hyperlink.text.active_color", XUI_WIDGET_STATE_ACTIVE, BASIC_TEXT);
	PIXEL_CHECK(xuiWidgetSetEnabled(link, 0) == XUI_OK);
	basic_case(f, link, "text.disabled_color", 0, BASIC_TEXT);
	basic_case(f, link, "hyperlink.text.disabled_color", 0, BASIC_TEXT);
	PIXEL_CHECK(xuiWidgetSetEnabled(link, 1) == XUI_OK);
	basic_cached_case(f, link, "hyperlink.text.color", BASIC_TEXT);
	p[0] = basic_color("text.color", 0x10203040u); p[1] = basic_color("hyperlink.text.color", 0x50607080u);
	PIXEL_CHECK(xuiWidgetSetInlineStyle(link, p, 2) == XUI_OK);
	basic_paint(f, link, 0); PIXEL_CHECK(basic_seen(BASIC_TEXT, p[1].tValue.iColor) > 0);
	PIXEL_CHECK(xuiWidgetSetInlineStyle(link, NULL, 0) == XUI_OK);
	PIXEL_CHECK(xuiLabelGetTextColor(label) == ld.iTextColor && xuiHyperlinkGetTextColor(link) == hd.iTextColor);
	xuiWidgetDestroy(label); xuiWidgetDestroy(link);
}

static void image_icon_cases(pixel_fixture_t* f)
{
	xui_widget w = NULL;
	xui_surface surface = NULL;
	xui_image_desc_t d = {0};
	xui_style_property_t p[2];
	xui_icon_category category = NULL;
	xui_icon icon = NULL, alias = NULL;
	xui_icon_draw_desc_t draw;
	xui_painter painter = NULL;
	PIXEL_CHECK(xuiTestSurfaceCreate(&f->proxy, &surface, 16, 16, 0) == XUI_OK);
	d.iSize = sizeof(d); d.pSurface = surface; d.iColor = 0x246813ffu;
	PIXEL_CHECK(xuiImageCreate(f->context, &w, &d) == XUI_OK); basic_attach(f, w);
	basic_case(f, w, "image.color", 0, BASIC_SURFACE);
	basic_case(f, w, "image.tint", 0, BASIC_SURFACE);
	PIXEL_CHECK(xuiWidgetSetEnabled(w, 0) == XUI_OK);
	basic_case(f, w, "image.tint", 0, BASIC_SURFACE);
	PIXEL_CHECK(xuiWidgetSetEnabled(w, 1) == XUI_OK);
	basic_cached_case(f, w, "image.tint", BASIC_SURFACE);
	p[0] = basic_color("image.color", 0x10203040u); p[1] = basic_color("image.tint", 0x50607080u);
	PIXEL_CHECK(xuiWidgetSetInlineStyle(w, p, 2) == XUI_OK);
	basic_paint(f, w, 0); PIXEL_CHECK(basic_seen(BASIC_SURFACE, p[1].tValue.iColor) > 0);
	PIXEL_CHECK(xuiWidgetSetInlineStyle(w, NULL, 0) == XUI_OK);
	PIXEL_CHECK(xuiImageGetColor(w) == d.iColor);
	PIXEL_CHECK(xuiIconCategoryCreate(f->context, "basic", NULL, &category) == XUI_OK);
	PIXEL_CHECK(xuiIconAddSurface(category, "surface", surface, (xui_rect_t){0, 0, 16, 16}, 0, NULL, &icon) == XUI_OK);
	PIXEL_CHECK(xuiIconAddAlias(category, "alias", icon, NULL, &alias) == XUI_OK);
	/* Icon is a drawing resource, not a widget. A global image style cannot
	 * override explicit tint passed by the caller, including aliases and zero. */
	PIXEL_CHECK(xuiStyleSetDefault(f->context, p, 2) == XUI_OK);
	xuiIconDrawDescDefault(&draw); draw.iColor = 0xabcdef67u;
	PIXEL_CHECK(xuiPainterBegin(f->context, f->target, &painter) == XUI_OK);
	basic_count = 0;
	PIXEL_CHECK(xuiIconDraw(painter, alias, (xui_rect_t){0, 0, 16, 16}, &draw) == XUI_OK);
	PIXEL_CHECK(basic_seen(BASIC_SURFACE, draw.iColor) > 0);
	draw.iColor = 0; basic_count = 0;
	PIXEL_CHECK(xuiIconDraw(painter, alias, (xui_rect_t){0, 0, 16, 16}, &draw) == XUI_OK);
	PIXEL_CHECK(basic_kind_count(BASIC_SURFACE) == 0 || basic_seen(BASIC_SURFACE, 0) > 0);
	PIXEL_CHECK(xuiPainterEnd(painter) == XUI_OK);
	PIXEL_CHECK(xuiIconRemove(category, "alias") == XUI_OK);
	PIXEL_CHECK(xuiIconRemove(category, "surface") == XUI_OK);
	xuiWidgetDestroy(w);
	f->proxy.tProxy.surfaceDestroy(&f->proxy.tProxy, surface);
}

int main(void)
{
	pixel_fixture_t f;
	if (!basic_init(&f)) return 1;
	text_cases(&f); image_icon_cases(&f);
	pixel_cleanup(&f);
	return pixel_result("xui_style_basic_content_test");
}
