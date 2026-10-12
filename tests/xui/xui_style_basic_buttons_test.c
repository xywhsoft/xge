#include "xui_style_basic_test.h"

static void button_cases(pixel_fixture_t* f, xui_surface surface)
{
	xui_widget w = NULL;
	xui_button_desc_t d = {0};
	xui_style_property_t p[2];
	const char* fills[] = {"button.normal_color", "button.hover_color", "button.active_color", "button.disabled_color", "button.checked_color"};
	const char* borders[] = {"button.border_color", "button.hover_border_color", "button.active_border_color", "button.disabled_border_color", "button.checked_border_color"};
	const uint32_t states[] = {0, XUI_WIDGET_STATE_HOVER, XUI_WIDGET_STATE_ACTIVE, XUI_WIDGET_STATE_DISABLED, XUI_BUTTON_STATE_CHECKED};
	size_t i;
	d.iSize = sizeof(d); d.sText = "Button"; d.iTextColor = 0x248631ffu;
	PIXEL_CHECK(xuiButtonCreate(f->context, &w, &d) == XUI_OK); basic_attach(f, w);
	for (i = 0; i < sizeof(states) / sizeof(*states); ++i) {
		basic_case(f, w, fills[i], states[i], BASIC_FILL);
		basic_case(f, w, borders[i], states[i], BASIC_STROKE);
	}
	basic_case(f, w, "button.focus_color", XUI_WIDGET_STATE_FOCUS, BASIC_STROKE);
	basic_case(f, w, "button.text_color", 0, BASIC_TEXT);
	basic_case(f, w, "text.color", 0, BASIC_TEXT);
	basic_case(f, w, "button.disabled_text_color", XUI_WIDGET_STATE_DISABLED, BASIC_TEXT);
	basic_case(f, w, "text.disabled_color", XUI_WIDGET_STATE_DISABLED, BASIC_TEXT);
	PIXEL_CHECK(xuiButtonSetIcon(w, surface, (xui_rect_t){0, 0, 16, 16}) == XUI_OK);
	PIXEL_CHECK(xuiButtonSetIconColor(w, 0x87654321u) == XUI_OK);
	basic_case(f, w, "button.icon_color", 0, BASIC_SURFACE);
	basic_case(f, w, "button.icon_color", XUI_WIDGET_STATE_DISABLED, BASIC_SURFACE);
	PIXEL_CHECK(xuiButtonGetIconColor(w) == 0x87654321u && xuiButtonGetTextColor(w) == d.iTextColor);
	PIXEL_CHECK(xuiButtonSetBadgeVisible(w, 1) == XUI_OK);
	basic_case(f, w, "button.badge.color", 0, BASIC_CIRCLE);
	basic_case(f, w, "button.badge.border_color", 0, BASIC_RING);
	basic_case(f, w, "button.badge.color", XUI_WIDGET_STATE_DISABLED, BASIC_CIRCLE);
	basic_cached_case(f, w, "button.badge.color", BASIC_CIRCLE);
	p[0] = basic_color("button.text_color", 0x135724ffu); p[1] = basic_color("text.color", 0x246813ffu);
	PIXEL_CHECK(xuiWidgetSetInlineStyle(w, p, 2) == XUI_OK);
	basic_paint(f, w, 0); PIXEL_CHECK(basic_seen(BASIC_TEXT, p[1].tValue.iColor) > 0);
	PIXEL_CHECK(xuiWidgetSetInlineStyle(w, NULL, 0) == XUI_OK);
	PIXEL_CHECK(xuiButtonSetBadgeSurface(w, surface, (xui_rect_t){0, 0, 16, 16}) == XUI_OK);
	p[0] = basic_color("button.badge.color", 0);
	PIXEL_CHECK(xuiWidgetSetInlineStyle(w, p, 1) == XUI_OK);
	basic_paint(f, w, 0); PIXEL_CHECK(basic_seen(BASIC_SURFACE, XUI_COLOR_WHITE) > 0);
	xuiWidgetDestroy(w);
}

static void card_cases(pixel_fixture_t* f)
{
	xui_widget w = NULL;
	xui_check_card_desc_t d = {0};
	const char* keys[] = {"checkcard.background.color", "checkcard.background.hover_color", "checkcard.background.active_color",
		"checkcard.background.checked_color", "checkcard.border.color", "checkcard.border.hover_color", "checkcard.border.checked_color",
		"checkcard.border.disabled_color", "checkcard.corner.color", "checkcard.check.color", "checkcard.focus.color"};
	const uint32_t states[] = {0, XUI_WIDGET_STATE_HOVER, XUI_WIDGET_STATE_ACTIVE, XUI_CHECKCARD_STATE_CHECKED,
		0, XUI_WIDGET_STATE_HOVER, XUI_CHECKCARD_STATE_CHECKED, XUI_WIDGET_STATE_DISABLED,
		XUI_CHECKCARD_STATE_CHECKED, XUI_CHECKCARD_STATE_CHECKED, XUI_WIDGET_STATE_FOCUS};
	const int kinds[] = {BASIC_FILL, BASIC_FILL, BASIC_FILL, BASIC_FILL, BASIC_STROKE, BASIC_STROKE, BASIC_STROKE,
		BASIC_STROKE, BASIC_TRIANGLE, BASIC_LINE, BASIC_STROKE};
	size_t i;
	d.iSize = sizeof(d); d.iBackgroundColor = 0x357913ffu; d.iCheckColor = 0x246802ffu; d.fFocusWidth = 2;
	PIXEL_CHECK(xuiCheckCardCreate(f->context, &w, &d) == XUI_OK); basic_attach(f, w);
	for (i = 0; i < sizeof(keys) / sizeof(*keys); ++i) basic_case(f, w, keys[i], states[i], kinds[i]);
	basic_case(f, w, "checkcard.check.color", XUI_CHECKCARD_STATE_CHECKED | XUI_WIDGET_STATE_DISABLED, BASIC_LINE);
	basic_case(f, w, "checkcard.background.color", XUI_WIDGET_STATE_DISABLED, BASIC_FILL);
	basic_cached_case(f, w, "checkcard.background.color", BASIC_FILL);
	PIXEL_CHECK(xuiCheckCardSetStateColors(w, 0, 0) == XUI_OK);
	basic_paint(f, w, XUI_WIDGET_STATE_FOCUS); PIXEL_CHECK(basic_seen(BASIC_STROKE, 150) == 0);
	xuiWidgetDestroy(w);
}

int main(void)
{
	pixel_fixture_t f;
	xui_surface surface = NULL;
	if (!basic_init(&f)) return 1;
	PIXEL_CHECK(xuiTestSurfaceCreate(&f.proxy, &surface, 16, 16, 0) == XUI_OK);
	button_cases(&f, surface); card_cases(&f);
	f.proxy.tProxy.surfaceDestroy(&f.proxy.tProxy, surface);
	pixel_cleanup(&f);
	return pixel_result("xui_style_basic_buttons_test");
}
