#include "xui_control_pixel_test.h"
#include "../src/xui_slider.c"
#include "../src/xui_range_slider.c"
#include "../src/xui_scrollbar.c"
#include "../src/xui_virtual_joystick.c"
#include "../src/xui_toolbar.c"
#include "../src/xui_menubar.c"
#include "../src/xui_statusbar.c"

/* Record the colors actually submitted to the renderer, not just resolution. */
static uint32_t g_colors[32768];
static int g_color_count;
static const char* g_key;

static int chrome_record(uint32_t color)
{
	if ((color & 255u) && g_color_count < (int)(sizeof(g_colors) / sizeof(g_colors[0])))
		g_colors[g_color_count++] = color;
	return XUI_OK;
}

static int chrome_fill(xui_proxy p, xui_draw_context d, xui_rect_t r, uint32_t c)
{ return chrome_record(c); }
static int chrome_stroke(xui_proxy p, xui_draw_context d, xui_rect_t r, float w, uint32_t c)
{ return chrome_record(c); }
static int chrome_circle(xui_proxy p, xui_draw_context d, float x, float y, float r, uint32_t c)
{ return chrome_record(c); }
static int chrome_ring(xui_proxy p, xui_draw_context d, float x, float y, float r, float w, uint32_t c)
{ return chrome_record(c); }
static int chrome_line(xui_proxy p, xui_draw_context d, float x, float y, float xx, float yy, float w, uint32_t c)
{ return chrome_record(c); }
static int chrome_text(xui_proxy p, xui_draw_context d, xui_font f, const char* s, xui_rect_t r, uint32_t c, uint32_t flags)
{ return chrome_record(c); }
static int chrome_surface(xui_proxy p, xui_draw_context d, xui_surface s, xui_rect_t src, xui_rect_t dst, uint32_t c, uint32_t flags)
{ return chrome_record(c); }

static void chrome_configure(xui_proxy p)
{
	p->drawRectFill = chrome_fill;
	p->drawRectStroke = chrome_stroke;
	p->drawCircleFill = chrome_circle;
	p->drawCircleStroke = chrome_ring;
	p->drawLine = chrome_line;
	p->drawText = chrome_text;
	p->drawSurface = chrome_surface;
}

static int chrome_count(uint32_t color)
{
	int i, n = 0;
	for (i = 0; i < g_color_count; i++) if (g_colors[i] == color) n++;
	return n;
}

static void chrome_paint(pixel_fixture_t* f, xui_widget w, uint32_t state)
{
	g_color_count = 0;
	pixel_paint(f, w, state);
}

static xui_style_property_t chrome_prop(const char* key, uint32_t color)
{
	xui_style_property_t p = {0};
	p.iSize = sizeof(p);
	p.sName = key;
	p.tValue.iSize = sizeof(p.tValue);
	p.tValue.iType = XUI_STYLE_VALUE_COLOR;
	p.tValue.iColor = color;
	return p;
}

static void chrome_inline(xui_widget w, const char* key, uint32_t color)
{
	xui_style_property_t p = chrome_prop(key, color);
	PIXEL_CHECK(xuiWidgetSetInlineStyle(w, &p, 1) == XUI_OK);
}

static void chrome_expect(pixel_fixture_t* f, xui_widget w, uint32_t state, uint32_t color)
{
	chrome_paint(f, w, state);
	if (!chrome_count(color)) printf("missing painted color %08x: %s\n", (unsigned)color, g_key);
	PIXEL_CHECK(chrome_count(color) > 0);
}

static void chrome_key(pixel_fixture_t* f, xui_widget w, const char* key, uint32_t base, uint32_t state)
{
	xui_style_property_t p = chrome_prop(key, 0x17395bffu);
	xui_style_desc_t s = {0};
	xui_style_property_info_t info = {0};
	xui_style_value_t token = p.tValue;
	int painted;
	g_key = key;
	s.iSize = sizeof(s); s.pProperties = &p; s.iPropertyCount = 1;
	PIXEL_CHECK(xuiStyleGetPropertyInfo(f->context, xuiStyleFindProperty(f->context, key), &info) == XUI_OK);
	PIXEL_CHECK(info.iValueType == XUI_STYLE_VALUE_COLOR);
	PIXEL_CHECK(info.iDirtyFlags == (XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER));
	chrome_expect(f, w, state, base);
	PIXEL_CHECK(xuiStyleSetDefault(f->context, &p, 1) == XUI_OK);
	chrome_expect(f, w, state, p.tValue.iColor);
	p.tValue.iColor = 0x294b6dffu;
	PIXEL_CHECK(xuiStyleSetType(f->context, xuiWidgetGetType(w), &s) == XUI_OK);
	chrome_expect(f, w, state, p.tValue.iColor);
	p.tValue.iColor = 0x3b5d7fffu;
	PIXEL_CHECK(xuiStyleSetClass(f->context, "chrome-test", &s) == XUI_OK);
	PIXEL_CHECK(xuiWidgetAddStyleClass(w, "chrome-test") == XUI_OK);
	chrome_expect(f, w, state, p.tValue.iColor);
	xuiWidgetClearDirty(w, ~0u);
	chrome_inline(w, key, 0x4d6f91ffu);
	chrome_expect(f, w, state, 0x4d6f91ffu);
	PIXEL_CHECK((xuiWidgetGetDirtyFlags(w) & XUI_WIDGET_DIRTY_LAYOUT) == 0);
	PIXEL_CHECK((xuiWidgetGetDirtyFlags(w) & XUI_WIDGET_DIRTY_CACHE) != 0);
	token.iColor = 0x5f81a3ffu;
	PIXEL_CHECK(xuiStyleSetToken(f->context, "chrome-color", &token) == XUI_OK);
	p.tValue.iType = XUI_STYLE_VALUE_TOKEN; p.tValue.sText = "chrome-color";
	PIXEL_CHECK(xuiWidgetSetInlineStyle(w, &p, 1) == XUI_OK);
	chrome_expect(f, w, state, token.iColor);
	token.iColor = 0x7193b5ffu;
	PIXEL_CHECK(xuiStyleSetToken(f->context, "chrome-color", &token) == XUI_OK);
	chrome_expect(f, w, state, token.iColor);
	painted = g_color_count;
	token.iColor = 0;
	PIXEL_CHECK(xuiStyleSetToken(f->context, "chrome-color", &token) == XUI_OK);
	chrome_paint(f, w, state);
	if (g_color_count >= painted) printf("transparent color remained visible: %s\n", key);
	PIXEL_CHECK(g_color_count < painted);
	chrome_inline(w, key, 0x7193b500u);
	chrome_paint(f, w, state);
	PIXEL_CHECK(g_color_count < painted);
	PIXEL_CHECK(xuiWidgetSetInlineStyle(w, NULL, 0) == XUI_OK);
	chrome_expect(f, w, state, 0x3b5d7fffu);
	PIXEL_CHECK(xuiStyleRemoveClass(f->context, "chrome-test") == XUI_OK);
	PIXEL_CHECK(xuiWidgetRemoveStyleClass(w, "chrome-test") == XUI_OK);
	chrome_expect(f, w, state, 0x294b6dffu);
	PIXEL_CHECK(xuiStyleRemoveType(f->context, xuiWidgetGetType(w)) == XUI_OK);
	chrome_expect(f, w, state, 0x17395bffu);
	PIXEL_CHECK(xuiStyleClearDefault(f->context) == XUI_OK);
	PIXEL_CHECK(xuiStyleRemoveToken(f->context, "chrome-color") == XUI_OK);
	chrome_expect(f, w, state, base);
}

static void chrome_attach(pixel_fixture_t* f, xui_widget w, int width, int height)
{
	PIXEL_CHECK(w != NULL);
	PIXEL_CHECK(xuiWidgetAddChild(f->root, w) == XUI_OK);
	PIXEL_CHECK(xuiWidgetSetRect(w, (xui_rect_t){0, 0, width, height}) == XUI_OK);
}

static void chrome_sliders(pixel_fixture_t* f)
{
	xui_widget slider = NULL, range = NULL, scroll = NULL, joystick = NULL;
	const uint32_t disabled = XUI_WIDGET_STATE_DISABLED;
	PIXEL_CHECK(xuiSliderCreate(f->context, &slider, NULL) == XUI_OK);
	PIXEL_CHECK(xuiRangeSliderCreate(f->context, &range, NULL) == XUI_OK);
	PIXEL_CHECK(xuiScrollBarCreate(f->context, &scroll, NULL) == XUI_OK);
	PIXEL_CHECK(xuiVirtualJoystickCreate(f->context, &joystick, NULL) == XUI_OK);
	chrome_attach(f, slider, 180, 30);
	chrome_attach(f, range, 180, 30);
	chrome_attach(f, scroll, 180, 30);
	chrome_attach(f, joystick, 120, 120);
	__xuiVirtualJoystickGetData(joystick)->bUseBuiltinAtlas = 0;
	chrome_key(f, slider, "slider.knob.disabled_color", XUI_COLOR_RGBA(232, 237, 244, 255), disabled);
	chrome_key(f, range, "rangeslider.knob.disabled_color", XUI_COLOR_RGBA(232, 237, 244, 255), disabled);
	chrome_key(f, joystick, "virtual_joystick.knob.border_color", XUI_COLOR_RGBA(255, 255, 255, 170), disabled);
	chrome_key(f, slider, "slider.knob.color", __xuiSliderGetData(slider)->iKnobColor, 0);
	chrome_inline(slider, "slider.disabled.color", 0);
	chrome_paint(f, slider, disabled);
	PIXEL_CHECK(chrome_count(72) == 0);
	chrome_inline(slider, "slider.knob.border_color", 0);
	chrome_paint(f, slider, disabled);
	PIXEL_CHECK(chrome_count(105) == 0);
	chrome_inline(range, "rangeslider.disabled.color", 0);
	chrome_paint(f, range, disabled);
	PIXEL_CHECK(chrome_count(72) == 0);
	chrome_inline(range, "rangeslider.knob.border_color", 0);
	chrome_paint(f, range, disabled);
	PIXEL_CHECK(chrome_count(105) == 0);
	chrome_inline(scroll, "scrollbar.disabled.color", 0);
	chrome_paint(f, scroll, disabled);
	PIXEL_CHECK(chrome_count(88) == 0);
	chrome_inline(joystick, "virtual_joystick.disabled.color", 0);
	chrome_paint(f, joystick, disabled);
	PIXEL_CHECK(chrome_count(86) == 0 && chrome_count(130) == 0 && chrome_count(180) == 0);
	xuiWidgetDestroy(slider);
	xuiWidgetDestroy(range);
	xuiWidgetDestroy(scroll);
	xuiWidgetDestroy(joystick);
}

static void chrome_bars(pixel_fixture_t* f)
{
	xui_widget toolbar = NULL, menubar = NULL, statusbar = NULL;
	xui_toolbar_item_t items[3] = {0};
	xui_statusbar_item_t status = {0};
	PIXEL_CHECK(xuiToolbarCreate(f->context, &toolbar, NULL) == XUI_OK);
	PIXEL_CHECK(xuiMenuBarCreate(f->context, &menubar, NULL) == XUI_OK);
	PIXEL_CHECK(xuiStatusBarCreate(f->context, &statusbar, NULL) == XUI_OK);
	chrome_attach(f, toolbar, 300, 36);
	chrome_attach(f, menubar, 300, 36);
	chrome_attach(f, statusbar, 300, 36);
	items[0].sText = "Run"; items[0].iState = XUI_TOOLBAR_ITEM_ENABLED;
	items[0].pIcon = f->target; items[0].tIconSrc = (xui_rect_t){0, 0, 12, 12};
	items[1].iType = XUI_TOOLBAR_ITEM_SEPARATOR;
	items[2].sText = "Stop"; items[2].iState = XUI_TOOLBAR_ITEM_ENABLED;
	PIXEL_CHECK(xuiToolbarSetItems(toolbar, items, 3) == XUI_OK);
	__xuiToolbarGetData(toolbar)->iActive = 0;
	chrome_key(f, toolbar, "toolbar.text.active_color", XUI_COLOR_WHITE, 0);
	chrome_key(f, toolbar, "toolbar.icon.active_color", XUI_COLOR_WHITE, 0);
	chrome_key(f, toolbar, "toolbar.separator.highlight_color", XUI_COLOR_RGBA(255, 255, 255, 112), 0);
	PIXEL_CHECK(xuiWidgetSetRect(toolbar, (xui_rect_t){0, 0, 40, 36}) == XUI_OK);
	__xuiToolbarGetData(toolbar)->bOverflowEnabled = 1;
	__xuiToolbarGetData(toolbar)->bOverflowActive = 1;
	chrome_key(f, toolbar, "toolbar.text.active_color", XUI_COLOR_WHITE, 0);
	PIXEL_CHECK(__xuiToolbarGetData(toolbar)->iOverflowCount > 0);
	PIXEL_CHECK(xuiMenuBarAddItem(menubar, "File", NULL, 1) == XUI_OK);
	__xuiMenuBarGetData(menubar)->iActive = 0;
	chrome_key(f, menubar, "menubar.text.active_color", XUI_COLOR_WHITE, 0);
	status.sText = "Ready"; status.fWidth = 100;
	status.iState = XUI_STATUSBAR_ITEM_ENABLED | XUI_STATUSBAR_ITEM_CLICKABLE;
	PIXEL_CHECK(xuiStatusBarSetItems(statusbar, &status, 1) == XUI_OK);
	__xuiStatusBarGetData(statusbar)->iActive = 0;
	chrome_key(f, statusbar, "statusbar.text.active_color", XUI_COLOR_WHITE, 0);
	chrome_inline(toolbar, "toolbar.focus.color", 0);
	chrome_paint(f, toolbar, 0);
	PIXEL_CHECK(chrome_count(160) == 0);
	chrome_inline(statusbar, "statusbar.item.active_color", 0);
	chrome_paint(f, statusbar, 0);
	PIXEL_CHECK(chrome_count(235) == 0);
	xuiWidgetDestroy(toolbar);
	xuiWidgetDestroy(menubar);
	xuiWidgetDestroy(statusbar);
}

int main(void)
{
	pixel_fixture_t f;
	if (!pixel_init_proxy(&f, chrome_configure)) return 1;
	chrome_sliders(&f);
	chrome_bars(&f);
	pixel_cleanup(&f);
	return pixel_result("xui_style_chrome_test");
}
