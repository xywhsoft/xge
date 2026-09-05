#include "xui_control_pixel_test.h"
#include "../src/xui_slider.c"
#include "../src/xui_range_slider.c"
#include "../src/xui_scrollbar.c"
#include "../src/xui_virtual_joystick.c"
#include "../src/xui_toolbar.c"
#include "../src/xui_menubar.c"
#include "../src/xui_statusbar.c"
#include "../src/xui_tabs.c"
#include "../src/xui_accordion.c"
#include "../src/xui_carousel.c"
#include "../src/xui_window.c"
#include "../src/xui_chart.c"
#include "../src/xui_inventory_grid.c"
#include "../src/xui_step_bar.c"
#include "../src/xui_tag_input.c"
#include "../src/xui_panel.c"

/* Record the colors actually submitted to the renderer, not just resolution. */
static uint32_t g_colors[32768];
static int g_color_count;
static const char* g_key;
static xui_widget g_paint_widget;

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
	if (xuiWidgetGetType(w)->onPreparePaint != NULL)
		PIXEL_CHECK(xuiWidgetGetType(w)->onPreparePaint(w) == XUI_OK);
	g_color_count = 0;
	pixel_paint(f, g_paint_widget ? g_paint_widget : w, state);
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
	PIXEL_CHECK((info.iDirtyFlags & ~XUI_WIDGET_DIRTY_STYLE) == (XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER));
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
	if (xuiWidgetGetParent(w) == NULL) PIXEL_CHECK(xuiWidgetAddChild(f->root, w) == XUI_OK);
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

static void chrome_frame(pixel_fixture_t* f)
{
	g_color_count = 0;
	PIXEL_CHECK(xuiRender(f->context, f->target, NULL, 0) == XUI_OK);
}

static void chrome_cached(pixel_fixture_t* f, xui_widget owner, xui_widget child, const char* key, uint32_t base)
{
	xui_style_property_t p = chrome_prop(key, 0x1d3e5fffu);
	xui_style_value_t token = p.tValue;
	chrome_frame(f);
	chrome_frame(f);
	PIXEL_CHECK(xuiWidgetGetCacheSurface(child, xuiWidgetGetStateId(child)) != NULL);
	PIXEL_CHECK(xuiStyleSetToken(f->context, "cached-chrome", &token) == XUI_OK);
	p.tValue.iType = XUI_STYLE_VALUE_TOKEN; p.tValue.sText = "cached-chrome";
	PIXEL_CHECK(xuiWidgetSetInlineStyle(owner, &p, 1) == XUI_OK);
	PIXEL_CHECK((xuiWidgetGetDirtyFlags(owner) & XUI_WIDGET_DIRTY_LAYOUT) == 0);
	chrome_frame(f);
	if (!chrome_count(token.iColor)) printf("cached child did not refresh: %s\n", key);
	PIXEL_CHECK(chrome_count(token.iColor) > 0);
	PIXEL_CHECK((xuiWidgetGetDirtyFlags(owner) & XUI_WIDGET_DIRTY_LAYOUT) == 0);
	PIXEL_CHECK((xuiWidgetGetDirtyFlags(child) & XUI_WIDGET_DIRTY_LAYOUT) == 0);
	chrome_frame(f);
	PIXEL_CHECK(chrome_count(token.iColor) == 0);
	token.iColor = 0x2e4f70ffu;
	PIXEL_CHECK(xuiStyleSetToken(f->context, "cached-chrome", &token) == XUI_OK);
	chrome_frame(f);
	PIXEL_CHECK(chrome_count(token.iColor) > 0);
	token.iColor = 0;
	PIXEL_CHECK(xuiStyleSetToken(f->context, "cached-chrome", &token) == XUI_OK);
	chrome_frame(f);
	PIXEL_CHECK(chrome_count(0x2e4f70ffu) == 0);
	PIXEL_CHECK(xuiWidgetSetInlineStyle(owner, NULL, 0) == XUI_OK);
	chrome_frame(f);
	PIXEL_CHECK(chrome_count(base) > 0);
	PIXEL_CHECK(xuiStyleRemoveToken(f->context, "cached-chrome") == XUI_OK);
}

static void chrome_close(xui_widget w, int index, void* user) { }

static void chrome_composites(pixel_fixture_t* f)
{
	xui_widget tabs = NULL, accordion = NULL, carousel = NULL, window = NULL;
	xui_tabs_data_t* td;
	xui_accordion_data_t* ad;
	xui_carousel_data_t* cd;
	xui_window_data_t* wd;
	PIXEL_CHECK(xuiTabsCreate(f->context, &tabs, NULL) == XUI_OK);
	PIXEL_CHECK(xuiAccordionCreate(f->context, &accordion, NULL) == XUI_OK);
	PIXEL_CHECK(xuiCarouselCreate(f->context, &carousel, NULL) == XUI_OK);
	PIXEL_CHECK(xuiWindowCreate(f->context, &window, NULL) == XUI_OK);
	chrome_attach(f, tabs, 320, 200);
	chrome_attach(f, accordion, 320, 200);
	chrome_attach(f, carousel, 320, 200);
	chrome_attach(f, window, 320, 200);
	PIXEL_CHECK(xuiTabsAddPage(tabs, "Document", NULL) == XUI_OK);
	PIXEL_CHECK(xuiTabsSetClose(tabs, chrome_close, 1, NULL) == XUI_OK);
	PIXEL_CHECK(xuiAccordionAddSection(accordion, "Section", 1, 1, NULL) == XUI_OK);
	PIXEL_CHECK(xuiCarouselSetPageCount(carousel, 2) == XUI_OK);
	PIXEL_CHECK(xuiCarouselSetAutoPlay(carousel, 0, 1) == XUI_OK);
	PIXEL_CHECK(xuiWindowSetOpen(window, 1) == XUI_OK);
	PIXEL_CHECK(xuiWindowSetTopMost(window, 1) == XUI_OK);
	PIXEL_CHECK(xuiLayout(f->context) == XUI_OK);
	td = __xuiTabsGetData(tabs); ad = __xuiAccordionGetData(accordion);
	cd = __xuiCarouselGetData(carousel); wd = __xuiWindowGetData(window);
	g_paint_widget = td->arrPages[0].pButton;
	chrome_key(f, tabs, "tabs.close.color", td->iTextColor, 0);
	td->iCloseHoverIndex = 0;
	chrome_key(f, tabs, "tabs.close.hover_color", td->iTextColor, 0);
	chrome_key(f, tabs, "tabs.close.hover_background_color", XUI_COLOR_RGBA(220, 232, 246, 255), 0);
	td->iCloseActiveIndex = 0;
	chrome_key(f, tabs, "tabs.close.active_color", td->iTextColor, 0);
	chrome_key(f, tabs, "tabs.close.active_background_color", XUI_COLOR_RGBA(220, 232, 246, 255), 0);
	td->iCloseHoverIndex = td->iCloseActiveIndex = -1;
	td->arrPages[0].bEnabled = 0;
	chrome_key(f, tabs, "tabs.close.disabled_color", (td->iTextColor & 0xffffff00u) | 100u, 0);
	chrome_key(f, tabs, "tabs.text.disabled_color", (td->iTextColor & 0xffffff00u) | 120u, 0);
	td->arrPages[0].bEnabled = 1;
	g_paint_widget = ad->arrSections[0].pHeader;
	chrome_key(f, accordion, "accordion.indicator.color", XUI_ACCORDION_DEFAULT_INDICATOR_COLOR, 0);
	g_paint_widget = cd->pOverlay;
	chrome_key(f, carousel, "carousel.indicator.background_color", 62, 0);
	g_paint_widget = wd->pCloseButton;
	chrome_key(f, window, "window.close.icon_color", XUI_COLOR_RGBA(171, 72, 76, 255), 0);
	g_paint_widget = NULL;
	chrome_key(f, window, "window.titlebar.topmost_color", XUI_COLOR_RGBA(47, 128, 208, 255), 0);
	chrome_cached(f, tabs, td->arrPages[0].pButton, "tabs.close.color", td->iTextColor);
	chrome_cached(f, accordion, ad->arrSections[0].pHeader, "accordion.indicator.color", XUI_ACCORDION_DEFAULT_INDICATOR_COLOR);
	chrome_cached(f, carousel, cd->pOverlay, "carousel.indicator.background_color", 62);
	chrome_cached(f, window, wd->pCloseButton, "window.close.icon_color", XUI_COLOR_RGBA(171, 72, 76, 255));
	chrome_cached(f, window, wd->pClient, "window.client.color", wd->iClientColor);
	xuiWidgetDestroy(tabs);
	xuiWidgetDestroy(accordion);
	xuiWidgetDestroy(carousel);
	xuiWidgetDestroy(window);
}

static void chrome_chart(pixel_fixture_t* f)
{
	xui_widget chart = NULL;
	xui_chart_point_t points[2] = {0};
	xui_chart_data_t* data;
	int index;
	PIXEL_CHECK(xuiChartCreate(f->context, &chart, NULL) == XUI_OK);
	chrome_attach(f, chart, 400, 280);
	PIXEL_CHECK(xuiChartAddSeries(chart, XUI_CHART_SERIES_SCATTER, "Data", &index) == XUI_OK);
	points[0].x = 1; points[0].y = 1;
	points[1].x = 5; points[1].y = 5; points[1].color = 0xb72345ffu;
	PIXEL_CHECK(xuiChartSetSeriesData(chart, index, points, 2) == XUI_OK);
	PIXEL_CHECK(xuiChartSetSeriesColor(chart, index, 0x812345ffu) == XUI_OK);
	PIXEL_CHECK(xuiChartSetLegendVisible(chart, 1) == XUI_OK);
	PIXEL_CHECK(xuiChartAddSeries(chart, XUI_CHART_SERIES_LINE, "Hidden", &index) == XUI_OK);
	PIXEL_CHECK(xuiChartSetSeriesVisible(chart, index, 0) == XUI_OK);
	PIXEL_CHECK(xuiChartSetBrushRange(chart, 1, 4, 1, 4) == XUI_OK);
	data = __xuiChartGetData(chart);
	data->tSelected.iPart = XUI_CHART_HIT_SERIES;
	data->tSelected.iSeries = 0; data->tSelected.iItem = 0;
	chrome_key(f, chart, "chart.legend.hidden_color", XUI_COLOR_RGBA(120, 128, 136, 120), 0);
	chrome_key(f, chart, "chart.brush.color", XUI_COLOR_RGBA(42, 124, 221, 38), 0);
	chrome_key(f, chart, "chart.brush.border_color", XUI_COLOR_RGBA(42, 124, 221, 180), 0);
	chrome_key(f, chart, "chart.selection.color", XUI_COLOR_RGBA(30, 40, 52, 220), 0);
	chrome_inline(chart, "chart.selection.color", 0x123456ffu);
	chrome_paint(f, chart, 0);
	PIXEL_CHECK(chrome_count(0x812345ffu) > 0 && chrome_count(0xb72345ffu) > 0);
	PIXEL_CHECK(data->arrSeries[0].iColor == 0x812345ffu && data->arrSeries[0].pPoints[1].color == 0xb72345ffu);
	xuiWidgetDestroy(chart);
}

static void chrome_inventory(pixel_fixture_t* f)
{
	xui_widget grid = NULL, tooltip;
	xui_inventory_slot_t slot = {0};
	xui_inventory_grid_data_t* data;
	xui_rect_t rect, world;
	PIXEL_CHECK(xuiInventoryGridCreate(f->context, &grid, NULL) == XUI_OK);
	chrome_attach(f, grid, 320, 220);
	PIXEL_CHECK(xuiInventoryGridSetSlotCount(grid, 1) == XUI_OK);
	slot.iItemId = 1; slot.iCount = 2; slot.iMaxCount = 10;
	strcpy(slot.sText, "Item"); strcpy(slot.sHotkey, "1");
	slot.iIconTint = 0xa82547ffu;
	slot.pIcon = f->target; slot.tIconSrc = (xui_rect_t){0, 0, 20, 20};
	PIXEL_CHECK(xuiInventoryGridSetSlot(grid, 0, &slot) == XUI_OK);
	chrome_frame(f);
	PIXEL_CHECK(xuiInventoryGridGetSlotRect(grid, 0, &rect) == XUI_OK);
	world = xuiWidgetGetWorldRect(grid);
	PIXEL_CHECK(xuiInputPointerMove(f->context, world.fX + rect.fX + 8, world.fY + rect.fY + 8, 0) == XUI_OK);
	PIXEL_CHECK(xuiUpdate(f->context, 0.3f) == XUI_OK);
	PIXEL_CHECK(xuiWidgetTooltipIsOpen(f->context) && xuiWidgetTooltipGetOwner(f->context) == grid);
	tooltip = f->context->pTooltipPopupWidget;
	PIXEL_CHECK(tooltip != NULL);
	data = __xuiInventoryGetData(grid);
	g_paint_widget = tooltip;
	chrome_key(f, grid, "inventory.tooltip.icon_background_color", XUI_COLOR_RGBA(245, 249, 253, 255), 0);
	chrome_key(f, grid, "inventory.text.color", data->tColors.iTextColor, 0);
	chrome_key(f, grid, "inventory.text.muted_color", data->tColors.iMutedTextColor, 0);
	chrome_key(f, grid, "inventory.hotkey.color", data->tColors.iHotkeyColor, 0);
	chrome_key(f, grid, "inventory.slot.quality_color", data->tColors.iQualityColor, 0);
	chrome_inline(grid, "inventory.slot.quality_color", 0);
	chrome_paint(f, grid, 0);
	PIXEL_CHECK(chrome_count(data->tColors.iBorderColor) == 0);
	PIXEL_CHECK(chrome_count(slot.iIconTint) > 0);
	PIXEL_CHECK(xuiWidgetSetInlineStyle(grid, NULL, 0) == XUI_OK);
	g_paint_widget = NULL;
	chrome_cached(f, grid, tooltip, "inventory.tooltip.icon_background_color", XUI_COLOR_RGBA(245, 249, 253, 255));
	slot.iQualityColor = 0xb93658ffu;
	PIXEL_CHECK(xuiInventoryGridSetSlot(grid, 0, &slot) == XUI_OK);
	chrome_inline(grid, "inventory.slot.quality_color", 0x123456ffu);
	chrome_paint(f, grid, 0);
	PIXEL_CHECK(chrome_count(slot.iQualityColor) > 0 && chrome_count(slot.iIconTint) > 0);
	PIXEL_CHECK(data->arrSlots[0].iQualityColor == slot.iQualityColor && data->arrSlots[0].iIconTint == slot.iIconTint);
	xuiWidgetDestroy(grid);
}

static void chrome_steps(pixel_fixture_t* f)
{
	xui_widget steps = NULL;
	const char* titles[] = {"Done", "Current", "Pending"};
	PIXEL_CHECK(xuiStepBarCreate(f->context, &steps, NULL) == XUI_OK);
	chrome_attach(f, steps, 420, 240);
	PIXEL_CHECK(xuiStepBarSetSteps(steps, titles, 3) == XUI_OK);
	PIXEL_CHECK(xuiStepBarSetCurrent(steps, 1) == XUI_OK);
	chrome_key(f, steps, "stepbar.arrow.text_color", XUI_COLOR_WHITE, 0);
	PIXEL_CHECK(xuiStepBarSetStyle(steps, XUI_STEP_BAR_STYLE_DOT) == XUI_OK);
	chrome_key(f, steps, "stepbar.check.active_color", XUI_COLOR_WHITE, 0);
	PIXEL_CHECK(xuiStepBarSetStyle(steps, XUI_STEP_BAR_STYLE_VERTICAL) == XUI_OK);
	chrome_key(f, steps, "stepbar.check.active_color", XUI_COLOR_WHITE, 0);
	PIXEL_CHECK(xuiStepBarSetColors(steps, 0x813557ffu, 0x924668ffu, 0xa35779ffu, 0xb4688affu) == XUI_OK);
	chrome_key(f, steps, "stepbar.done.color", 0x813557ffu, 0);
	chrome_key(f, steps, "stepbar.active.color", 0x924668ffu, 0);
	xuiWidgetDestroy(steps);
}

static void chrome_tags(pixel_fixture_t* f)
{
	xui_widget tags = NULL, input;
	xui_tag_input_data_t* data;
	uint32_t selection;
	PIXEL_CHECK(xuiTagInputCreate(f->context, &tags, NULL) == XUI_OK);
	chrome_attach(f, tags, 420, 80);
	input = xuiTagInputGetInputWidget(tags);
	PIXEL_CHECK(input != NULL);
	PIXEL_CHECK(xuiInputSetText(input, "Selected text") == XUI_OK);
	PIXEL_CHECK(xuiInputSelectAll(input) == XUI_OK);
	PIXEL_CHECK(xuiSetFocusWidget(f->context, input) == XUI_OK);
	PIXEL_CHECK(xuiLayout(f->context) == XUI_OK);
	data = __xuiTagInputGetData(tags);
	PIXEL_CHECK(xuiInputSetExtendedColors(input, data->iPlaceholderColor, data->iDisabledTextColor,
		0, 0, 0, 0x8a3557ffu, data->iTextColor) == XUI_OK);
	g_paint_widget = input;
	chrome_key(f, tags, "taginput.text.color", data->iTextColor, XUI_WIDGET_STATE_FOCUS);
	PIXEL_CHECK(xuiWidgetSetEnabled(tags, 0) == XUI_OK);
	chrome_key(f, tags, "taginput.text.disabled_color", data->iDisabledTextColor, XUI_WIDGET_STATE_DISABLED);
	PIXEL_CHECK(xuiWidgetSetEnabled(tags, 1) == XUI_OK);
	PIXEL_CHECK(xuiSetFocusWidget(f->context, input) == XUI_OK);
	PIXEL_CHECK(xuiInputGetExtendedColors(input, NULL, NULL, NULL, NULL, NULL, &selection, NULL) == XUI_OK);
	PIXEL_CHECK(selection == 0x8a3557ffu);
	chrome_key(f, input, "input.selection.color", selection, XUI_WIDGET_STATE_FOCUS);
	chrome_inline(tags, "taginput.text.color", 0x923557ffu);
	chrome_inline(input, "input.text.color", 0xa34668ffu);
	chrome_paint(f, tags, XUI_WIDGET_STATE_FOCUS);
	PIXEL_CHECK(chrome_count(0xa34668ffu) > 0 && chrome_count(0x923557ffu) == 0);
	PIXEL_CHECK(chrome_count(selection) > 0);
	PIXEL_CHECK(xuiWidgetSetInlineStyle(tags, NULL, 0) == XUI_OK);
	PIXEL_CHECK(xuiWidgetSetInlineStyle(input, NULL, 0) == XUI_OK);
	g_paint_widget = NULL;
	chrome_cached(f, tags, input, "taginput.text.color", data->iTextColor);
	PIXEL_CHECK(xuiInputSetText(input, "") == XUI_OK);
	PIXEL_CHECK(xuiTagInputSetPlaceholder(tags, "Placeholder") == XUI_OK);
	g_paint_widget = input;
	chrome_key(f, tags, "taginput.placeholder.color", data->iPlaceholderColor, 0);
	g_paint_widget = NULL;
	chrome_cached(f, tags, input, "taginput.placeholder.color", data->iPlaceholderColor);
	PIXEL_CHECK(xuiInputGetExtendedColors(input, NULL, NULL, NULL, NULL, NULL, &selection, NULL) == XUI_OK);
	PIXEL_CHECK(selection == 0x8a3557ffu);
	xuiWidgetDestroy(tags);
}

static void chrome_panel(pixel_fixture_t* f)
{
	xui_widget panel = NULL, title;
	xui_panel_data_t* data;
	PIXEL_CHECK(xuiPanelCreate(f->context, &panel, NULL) == XUI_OK);
	chrome_attach(f, panel, 320, 200);
	PIXEL_CHECK(xuiPanelSetTitle(panel, "Panel title") == XUI_OK);
	PIXEL_CHECK(xuiPanelSetTitleColor(panel, 0x853557ffu) == XUI_OK);
	PIXEL_CHECK(xuiPanelSetDisabledTitleColor(panel, 0x964668ffu) == XUI_OK);
	PIXEL_CHECK(xuiLayout(f->context) == XUI_OK);
	title = xuiPanelGetTitleWidget(panel);
	data = __xuiPanelGetData(panel);
	chrome_key(f, panel, "panel.background.color", data->iBackgroundColor, 0);
	chrome_key(f, panel, "panel.header.color", data->iHeaderColor, 0);
	chrome_key(f, panel, "panel.client.color", data->iClientColor, 0);
	chrome_key(f, panel, "panel.border.color", data->iBorderColor, 0);
	g_paint_widget = title;
	chrome_key(f, panel, "text.color", 0x853557ffu, 0);
	PIXEL_CHECK(xuiWidgetSetEnabled(title, 0) == XUI_OK);
	chrome_key(f, panel, "text.disabled_color", 0x964668ffu, XUI_WIDGET_STATE_DISABLED);
	PIXEL_CHECK(xuiWidgetSetEnabled(title, 1) == XUI_OK);
	g_paint_widget = NULL;
	chrome_cached(f, panel, title, "text.color", 0x853557ffu);
	PIXEL_CHECK(xuiPanelGetTitleColor(panel) == 0x853557ffu);
	PIXEL_CHECK(xuiPanelGetDisabledTitleColor(panel) == 0x964668ffu);
	xuiWidgetDestroy(panel);
}

int main(void)
{
	pixel_fixture_t f;
	if (!pixel_init_proxy(&f, chrome_configure)) return 1;
	f.proxy.tProxy.surfaceDestroy(&f.proxy.tProxy, f.target);
	PIXEL_CHECK(xuiTestSurfaceCreate(&f.proxy, &f.target, 640, 480, XUI_SURFACE_USAGE_TARGET) == XUI_OK);
	chrome_sliders(&f);
	chrome_bars(&f);
	chrome_composites(&f);
	chrome_chart(&f);
	chrome_inventory(&f);
	chrome_steps(&f);
	chrome_tags(&f);
	chrome_panel(&f);
	pixel_cleanup(&f);
	return pixel_result("xui_style_chrome_test");
}
