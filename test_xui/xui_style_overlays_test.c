#include "src/xui_internal.h"
#include "test_xui/xui_test_proxy.h"

#include <stdio.h>
#include <string.h>

#define CHECK(expr) do { if (!(expr)) { fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #expr); return 1; } } while (0)
#define OK(expr) CHECK((expr) == XUI_OK)

typedef struct fixture_t {
	xui_context context;
	xui_widget root;
	xui_surface target;
	xui_font font;
	xui_test_proxy_state_t proxy;
} fixture_t;

static xui_draw_begin_proc original_begin;

static int record_begin(xui_proxy proxy, xui_draw_context* draw, xui_surface target)
{
	/* Record the final cache rebuild, not stale draws replaced earlier in this frame. */
	xuiTestSurfaceReset(target);
	return original_begin(proxy, draw, target);
}

static int init(fixture_t* f)
{
	memset(f, 0, sizeof(*f));
	xuiTestProxyInit(&f->proxy);
	original_begin = f->proxy.tProxy.drawBegin;
	f->proxy.tProxy.drawBegin = record_begin;
	OK(xuiCreate(&f->context));
	OK(xuiSetProxy(f->context, &f->proxy.tProxy));
	OK(f->proxy.tProxy.fontLoadMemory(&f->proxy.tProxy, &f->font, "body", 4, 14.0f, XUI_FONT_FORMAT_TTF));
	OK(xuiSetDefaultFont(f->context, f->font));
	OK(xuiInputViewport(f->context, 800.0f, 600.0f));
	OK(xuiWidgetCreate(f->context, &f->root));
	OK(xuiWidgetSetRect(f->root, (xui_rect_t){0, 0, 800, 600}));
	OK(xuiSetRootWidget(f->context, f->root));
	OK(xuiTestSurfaceCreate(&f->proxy, &f->target, 800, 600, XUI_SURFACE_USAGE_TARGET));
	return 0;
}

static void finish(fixture_t* f)
{
	f->proxy.tProxy.surfaceDestroy(&f->proxy.tProxy, f->target);
	xuiDestroy(f->context);
	f->proxy.tProxy.fontDestroy(&f->proxy.tProxy, f->font);
}

static void reset_draws(xui_widget widget)
{
	xui_widget child;
	if ( widget == NULL ) return;
	xuiTestSurfaceReset(xuiWidgetGetCacheSurface(widget, xuiWidgetGetStateId(widget)));
	for ( child = xuiWidgetGetFirstChild(widget); child != NULL; child = xuiWidgetGetNextSibling(child) ) reset_draws(child);
}

static int render(fixture_t* f)
{
	xui_rect_i_t full = {0, 0, 800, 600};
	reset_draws(f->root);
	reset_draws(f->context->pOverlayRoot);
	OK(xuiRender(f->context, f->target, &full, 1));
	return 0;
}

static xui_surface cache(xui_widget widget)
{
	return xuiWidgetGetCacheSurface(widget, xuiWidgetGetStateId(widget));
}

static int fill(xui_widget widget, uint32_t color)
{
	return xuiTestSurfaceGetRectFillColorCount(cache(widget), color);
}

static xui_style_property_t color(const char* name, uint32_t value)
{
	xui_style_property_t p;
	memset(&p, 0, sizeof(p));
	p.iSize = sizeof(p);
	p.sName = name;
	p.tValue.iSize = sizeof(p.tValue);
	p.tValue.iType = XUI_STYLE_VALUE_COLOR;
	p.tValue.iColor = value;
	return p;
}

static xui_style_desc_t style(xui_style_property_t* props, int count)
{
	xui_style_desc_t s;
	memset(&s, 0, sizeof(s));
	s.iSize = sizeof(s);
	s.pProperties = props;
	s.iPropertyCount = count;
	return s;
}

static int paint_only(xui_widget widget)
{
	CHECK((xuiWidgetGetDirtyFlags(widget) & (XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_TREE)) == 0);
	CHECK((xuiWidgetGetDirtyFlags(widget) & XUI_WIDGET_DIRTY_CACHE) != 0);
	return 0;
}

static int warm_frame(fixture_t* f)
{
	xui_render_stats_t stats;
	CHECK(render(f) == 0);
	memset(&stats, 0, sizeof(stats));
	stats.iSize = sizeof(stats);
	OK(xuiGetRenderStats(f->context, &stats));
	CHECK(stats.iUpdatedCaches == 0 && stats.iUpdatedWidgets == 0 && stats.iRecoveredErrors == 0);
	return 0;
}

static int window_parent_colors(fixture_t* f, xui_widget window)
{
	xui_style_property_t p[2];
	xui_widget close = xuiWindowGetCloseButtonWidget(window);
	xui_widget client = xuiWindowGetClientWidget(window);
	uint32_t base;
	OK(xuiWindowGetColors(window, NULL, &base, NULL, NULL, NULL, NULL, NULL, NULL));
	p[0] = color("window.close.icon_color", 0x396214ff);
	p[1] = color("window.client.color", 0x694321ff);
	OK(xuiWidgetSetInlineStyle(window, p, 2));
	CHECK(paint_only(window) == 0);
	CHECK(render(f) == 0 && fill(close, p[0].tValue.iColor) > 0 && fill(client, p[1].tValue.iColor) > 0);
	CHECK(warm_frame(f) == 0);
	p[0].tValue.iColor = p[1].tValue.iColor = 0;
	OK(xuiWidgetSetInlineStyle(window, p, 2));
	CHECK(render(f) == 0 && fill(close, 0x396214ff) == 0 && xuiTestSurfaceGetRectFillCount(cache(client)) == 0);
	OK(xuiWidgetSetInlineStyle(window, NULL, 0));
	CHECK(render(f) == 0 && fill(client, base) > 0);
	CHECK(warm_frame(f) == 0);
	return 0;
}

static int popup_colors(void)
{
	fixture_t f;
	xui_widget popup, panel, bar;
	xui_popup_desc_t desc;
	xui_style_property_t p[3];
	xui_style_desc_t s;
	xui_style_value_t token;
	uint32_t base, current;
	CHECK(init(&f) == 0);
	memset(&desc, 0, sizeof(desc));
	desc.iSize = sizeof(desc);
	desc.fContentWidth = 140;
	desc.fContentHeight = 400;
	desc.fMaxHeight = 180;
	desc.fPadding = 4;
	desc.fBorderWidth = 1;
	desc.fShadowSize = 3;
	OK(xuiPopupCreate(f.context, &popup, &desc));
	OK(xuiPopupSetColors(popup, 0x18324aff, 0x456789ff, 0x01020388, 0x11223344));
	OK(xuiPopupSetOpen(popup, 1));
	CHECK(render(&f) == 0);
	panel = xuiPopupGetPanelWidget(popup);
	bar = xuiScrollFrameGetVScrollBarWidget(xuiPopupGetFrameWidget(popup));
	CHECK(fill(panel, 0x18324aff) > 0);
	OK(xuiPopupGetColors(popup, &base, NULL, NULL, NULL));
	p[0] = color("popup.panel.color", 0x102938ff);
	p[1] = color("popup.border.color", 0x908172ff);
	p[2] = color("popup.scrollbar.thumb.color", 0x765432ff);
	s = style(p, 3);
	OK(xuiStyleSetType(f.context, xuiPopupGetType(f.context), &s));
	CHECK(paint_only(popup) == 0);
	CHECK(render(&f) == 0);
	CHECK(fill(panel, 0x102938ff) > 0 && fill(panel, 0x908172ff) > 0);
	CHECK(fill(bar, 0x765432ff) > 0);
	OK(xuiPopupGetColors(popup, &current, NULL, NULL, NULL));
	CHECK(current == base);
	p[0] = color("popup.panel.color", 0);
	p[1] = color("popup.border.color", 0);
	p[2] = color("popup.shadow.color", 0);
	OK(xuiWidgetSetInlineStyle(popup, p, 3));
	CHECK(render(&f) == 0);
	CHECK(xuiTestSurfaceGetRectFillCount(cache(panel)) == 0);
	OK(xuiWidgetSetInlineStyle(popup, NULL, 0));
	OK(xuiStyleRemoveType(f.context, xuiPopupGetType(f.context)));
	CHECK(render(&f) == 0);
	CHECK(fill(panel, base) > 0 && fill(bar, 0x765432ff) == 0);
	memset(&token, 0, sizeof(token));
	token.iSize = sizeof(token);
	token.iType = XUI_STYLE_VALUE_COLOR;
	token.iColor = 0x567812ff;
	OK(xuiStyleSetToken(f.context, "overlay.panel", &token));
	p[0] = color("popup.panel.color", 0);
	p[0].tValue.iType = XUI_STYLE_VALUE_TOKEN;
	p[0].tValue.sText = "overlay.panel";
	OK(xuiStyleSetDefault(f.context, p, 1));
	CHECK(render(&f) == 0 && fill(panel, token.iColor) > 0);
	token.iColor = 0x246835ff;
	OK(xuiStyleSetToken(f.context, "overlay.panel", &token));
	CHECK(render(&f) == 0 && fill(panel, token.iColor) > 0);
	OK(xuiStyleClearDefault(f.context));
	CHECK(render(&f) == 0 && fill(panel, base) > 0);
	CHECK(warm_frame(&f) == 0);
	finish(&f);
	puts("PASS popup rendered palette, cache transitions, clear, token/default/type/inline, transparent zero, paint-only");
	return 0;
}

static int popup_dependencies(void)
{
	fixture_t f;
	xui_popup_desc_t desc;
	xui_widget popup, scroll, frame, bar, backdrop;
	xui_style_property_t p[6];
	xui_rect_t before, after, world;
	uint32_t palette[6], buttonBase, iconBase, cornerBase, gripBase;
	CHECK(init(&f) == 0);
	memset(&desc, 0, sizeof(desc));
	desc.iSize = sizeof(desc);
	desc.fContentWidth = 400;
	desc.fContentHeight = 400;
	desc.fMaxWidth = 180;
	desc.fMaxHeight = 180;
	desc.iOutsidePolicy = XUI_POPUP_OUTSIDE_IGNORE;
	desc.iOwnerPolicy = XUI_POPUP_OWNER_PASSTHROUGH;
	desc.iScrollbarMode = XUI_SCROLLBAR_MODE_FULL;
	desc.fScrollbarSize = 16;
	OK(xuiPopupCreate(f.context, &popup, &desc));
	scroll = xuiPopupGetScrollViewWidget(popup);
	frame = xuiPopupGetFrameWidget(popup);
	bar = xuiScrollFrameGetVScrollBarWidget(frame);
	backdrop = xuiWidgetGetFirstChild(popup);
	OK(xuiScrollViewSetCornerMode(scroll, XUI_SCROLL_FRAME_CORNER_GRIP));
	OK(xuiScrollViewSetButtonColors(scroll, 0x341256ff, 0x687453ff));
	OK(xuiScrollViewSetCornerColors(scroll, 0x273456ff, 0x764382ff));
	OK(xuiScrollViewGetButtonColors(scroll, &buttonBase, &iconBase));
	OK(xuiScrollViewGetCornerColors(scroll, &cornerBase, &gripBase));
	OK(xuiScrollViewGetColors(scroll, &palette[0], &palette[1], &palette[2], &palette[3], &palette[4], &palette[5]));
	OK(xuiPopupSetOpen(popup, 1));
	CHECK(render(&f) == 0);
	before = xuiWidgetGetRect(popup);
	CHECK(cache(backdrop) == NULL && cache(popup) == NULL);
	CHECK(before.fW < 800 && before.fH < 600);
	CHECK(fill(bar, buttonBase) > 0 && fill(frame, cornerBase) > 0 && fill(frame, gripBase) > 0);
	p[0] = color("popup.backdrop.color", 0x13245677);
	p[1] = color("popup.scrollbar.button.color", 0x682395ff);
	p[2] = color("popup.scrollbar.button.icon_color", 0x976321ff);
	p[3] = color("popup.scrollbar.corner.color", 0x645231ff);
	p[4] = color("popup.scrollbar.grip.color", 0x956237ff);
	p[5] = color("popup.scrollbar.thumb.color", 0x792345ff);
	OK(xuiWidgetSetInlineStyle(popup, p, 6));
	CHECK(paint_only(popup) == 0 && render(&f) == 0);
	after = xuiWidgetGetRect(popup);
	CHECK(memcmp(&before, &after, sizeof(before)) == 0);
	world = xuiWidgetGetWorldRect(backdrop);
	CHECK(world.fX == 0 && world.fY == 0 && world.fW == 800 && world.fH == 600);
	CHECK(!xuiWidgetGetHitTestVisible(backdrop) && fill(backdrop, p[0].tValue.iColor) > 0);
	CHECK(cache(backdrop) != NULL && cache(popup) == NULL);
	CHECK(fill(bar, p[1].tValue.iColor) > 0 && fill(bar, p[2].tValue.iColor) > 0);
	CHECK(fill(frame, p[3].tValue.iColor) > 0 && fill(frame, p[4].tValue.iColor) > 0);
	CHECK(fill(bar, p[5].tValue.iColor) > 0);
	CHECK(warm_frame(&f) == 0);
	palette[1] = 0x182347ff;
	OK(xuiScrollViewSetColors(scroll, palette[0], palette[1], palette[2], palette[3], palette[4], palette[5]));
	OK(xuiScrollViewSetButtonColors(scroll, 0x387452ff, iconBase));
	CHECK(render(&f) == 0 && fill(bar, p[5].tValue.iColor) > 0 && fill(bar, p[1].tValue.iColor) > 0);
	p[0].tValue.iColor = p[1].tValue.iColor = p[2].tValue.iColor = 0;
	p[3].tValue.iColor = p[4].tValue.iColor = p[5].tValue.iColor = 0;
	OK(xuiWidgetSetInlineStyle(popup, p, 6));
	CHECK(render(&f) == 0 && xuiTestSurfaceGetRectFillCount(cache(backdrop)) == 0);
	CHECK(cache(backdrop) == NULL && cache(popup) == NULL);
	CHECK(fill(frame, 0x645231ff) == 0 && fill(bar, 0x682395ff) == 0 && fill(bar, 0x792345ff) == 0);
	OK(xuiWidgetSetInlineStyle(popup, NULL, 0));
	CHECK(render(&f) == 0 && fill(bar, palette[1]) > 0 && fill(bar, 0x387452ff) > 0);
	CHECK(fill(frame, cornerBase) > 0 && fill(frame, gripBase) > 0);
	OK(xuiPopupSetColors(popup, 0x536781ff, 0x743561ff, 0, 0x27456188));
	CHECK(render(&f) == 0 && cache(backdrop) != NULL && fill(backdrop, 0x27456188) > 0);
	OK(xuiPopupSetColors(popup, 0x536781ff, 0x743561ff, 0, 0));
	CHECK(render(&f) == 0 && cache(backdrop) == NULL);
	CHECK(warm_frame(&f) == 0);
	finish(&f);
	puts("PASS popup render-only backdrop without input shield, scrollbar buttons/corner/grip, transparent/clear/API edits, warm caches");
	return 0;
}

static int menu_colors(void)
{
	fixture_t f;
	xui_widget menu, panel;
	xui_menu_item_t item;
	xui_menu_colors_t base, after;
	xui_style_property_t p[5];
	xui_style_desc_t s;
	xui_rect_i_t full = {0, 0, 800, 600};
	CHECK(init(&f) == 0);
	OK(xuiMenuCreate(f.context, &menu, NULL));
	OK(xuiMenuGetColors(menu, &base));
	base.iPanelColor = 0x123456ff;
	base.iHoverColor = 0x345678ff;
	base.iTextColor = 0x456789ff;
	OK(xuiMenuSetColors(menu, &base));
	memset(&item, 0, sizeof(item));
	item.iType = XUI_MENU_ITEM_NORMAL;
	item.iState = XUI_MENU_ITEM_ENABLED;
	item.sText = "Open";
	OK(xuiMenuAddItem(menu, &item));
	OK(xuiMenuOpenAt(menu, NULL, 30, 30));
	panel = xuiPopupGetPanelWidget(xuiMenuGetPopupWidget(menu));
	CHECK(render(&f) == 0 && fill(panel, base.iPanelColor) > 0 && fill(menu, base.iPanelColor) > 0);
	p[0] = color("menu.panel.color", 0x817263ff);
	p[1] = color("menu.border.color", 0x786543ff);
	p[2] = color("menu.item.hover_color", 0x681723ff);
	p[3] = color("menu.text.hover_color", 0x879867ff);
	p[4] = color("menu.focus.color", 0x787858ff);
	s = style(p, 5);
	OK(xuiStyleSetClass(f.context, "overlay.menu", &s));
	OK(xuiWidgetAddStyleClass(menu, "overlay.menu"));
	CHECK(paint_only(menu) == 0);
	reset_draws(f.context->pOverlayRoot);
	OK(xuiRender(f.context, f.target, &full, 1));
	CHECK(fill(menu, p[0].tValue.iColor) > 0 && fill(panel, p[0].tValue.iColor) > 0);
	CHECK(fill(panel, p[1].tValue.iColor) > 0 && fill(menu, p[2].tValue.iColor) > 0);
	CHECK(xuiTestSurfaceGetLastTextColor(cache(menu)) == p[3].tValue.iColor);
	CHECK(fill(menu, p[4].tValue.iColor) > 0);
	OK(xuiMenuGetColors(menu, &after));
	CHECK(memcmp(&after, &base, sizeof(base)) == 0);
	base.iPanelColor = 0x342198ff;
	OK(xuiMenuSetColors(menu, &base));
	CHECK(paint_only(menu) == 0);
	CHECK(render(&f) == 0 && fill(menu, p[0].tValue.iColor) > 0);
	p[0] = color("menu.panel.color", 0);
	p[1] = color("menu.border.color", 0);
	p[2] = color("menu.shadow.color", 0);
	OK(xuiWidgetSetInlineStyle(menu, p, 3));
	CHECK(render(&f) == 0 && xuiTestSurfaceGetRectFillCount(cache(panel)) == 0);
	OK(xuiWidgetSetInlineStyle(menu, NULL, 0));
	OK(xuiWidgetRemoveStyleClass(menu, "overlay.menu"));
	CHECK(render(&f) == 0 && fill(menu, base.iPanelColor) > 0 && fill(panel, base.iPanelColor) > 0);
	finish(&f);
	puts("PASS menu rendered text/hover/focus/chrome, open popup render-only refresh, class/inline/clear, API palette preservation");
	return 0;
}

static int msgtip_colors(void)
{
	fixture_t f;
	xui_msgtip tip;
	xui_widget widget;
	xui_msgtip_colors_t base, after;
	xui_style_property_t p[5];
	xui_style_desc_t s;
	xui_surface icon;
	CHECK(init(&f) == 0);
	OK(xuiMsgTipCreate(f.context, &tip, NULL));
	OK(xuiMsgTipGetColors(tip, &base));
	base.iBackgroundColor = 0x234567ff;
	base.iIconColor = 0x367894ff;
	OK(xuiMsgTipSetColors(tip, &base));
	OK(xuiMsgTipShow(tip, XUI_MSGTIP_ICON_INFO, "Status", 100));
	widget = xuiMsgTipGetWidget(tip);
	CHECK(render(&f) == 0 && fill(widget, base.iBackgroundColor) > 0);
	p[0] = color("msgtip.background.color", 0x817268ff);
	p[1] = color("msgtip.border.color", 0x783243ff);
	p[2] = color("msgtip.text.color", 0x889922ff);
	p[3] = color("msgtip.icon.color", 0x812213ff);
	p[4] = color("msgtip.shadow.color", 0x23212166);
	s = style(p, 5);
	s.sName = "overlay.tip";
	OK(xuiStyleSetNamed(f.context, &s));
	OK(xuiWidgetSetStyleName(widget, "overlay.tip"));
	CHECK(paint_only(widget) == 0);
	CHECK(render(&f) == 0 && fill(widget, p[0].tValue.iColor) > 0 && fill(widget, p[1].tValue.iColor) > 0);
	CHECK(xuiTestSurfaceGetLastTextColor(cache(widget)) == p[2].tValue.iColor);
	CHECK(xuiTestSurfaceGetLastColor(cache(widget)) == p[3].tValue.iColor);
	CHECK(fill(widget, p[4].tValue.iColor) > 0);
	OK(xuiMsgTipGetColors(tip, &after));
	CHECK(memcmp(&base, &after, sizeof(base)) == 0);
	p[0] = color("msgtip.background.color", 0);
	p[1] = color("msgtip.border.color", 0);
	p[2] = color("msgtip.text.color", 0);
	p[3] = color("msgtip.icon.color", 0);
	p[4] = color("msgtip.shadow.color", 0);
	OK(xuiWidgetSetInlineStyle(widget, p, 5));
	CHECK(render(&f) == 0 && fill(widget, base.iBackgroundColor) == 0);
	CHECK(xuiTestSurfaceGetLastTextColor(cache(widget)) == 0 && xuiTestSurfaceGetLastColor(cache(widget)) == 0);
	OK(xuiTestSurfaceCreate(&f.proxy, &icon, 16, 16, XUI_SURFACE_USAGE_TARGET));
	OK(xuiMsgTipSetIconSurface(tip, icon, (xui_rect_t){0, 0, 16, 16}));
	CHECK(render(&f) == 0 && xuiTestSurfaceGetLastColor(cache(widget)) == XUI_COLOR_WHITE);
	OK(xuiWidgetSetInlineStyle(widget, NULL, 0));
	OK(xuiWidgetSetStyleName(widget, NULL));
	CHECK(render(&f) == 0 && fill(widget, base.iBackgroundColor) > 0);
	xuiMsgTipDestroy(tip);
	f.proxy.tProxy.surfaceDestroy(&f.proxy.tProxy, icon);
	finish(&f);
	puts("PASS msgtip named styles, rendered text/icon/shadow, clear/transparent zero, API/custom icon preservation");
	return 0;
}

static int toast_colors(void)
{
	fixture_t f;
	xui_toast toast;
	xui_widget first, second;
	xui_toast_colors_t base, after;
	xui_style_property_t p[4];
	xui_style_desc_t s;
	CHECK(init(&f) == 0);
	OK(xuiToastCreate(f.context, &toast, NULL));
	OK(xuiToastGetColors(toast, &base));
	base.iBackgroundColor = 0x375643ff;
	OK(xuiToastSetColors(toast, &base));
	CHECK(xuiToastShow(toast, XUI_TOAST_TYPE_INFO, "First", "First message", 100, NULL, NULL) > 0);
	CHECK(xuiToastShow(toast, XUI_TOAST_TYPE_INFO, "Second", "Second message", 100, NULL, NULL) > 0);
	first = xuiToastGetItemWidget(toast, 0);
	second = xuiToastGetItemWidget(toast, 1);
	CHECK(render(&f) == 0 && fill(first, base.iBackgroundColor) > 0);
	p[0] = color("toast.background.color", 0x876534ff);
	p[1] = color("toast.text.muted_color", 0x967831ff);
	p[2] = color("toast.info.color", 0x965427ff);
	p[3] = color("toast.close.color", 0x763237ff);
	s = style(p, 4);
	OK(xuiStyleSetType(f.context, xuiWidgetFindType(f.context, "toast-item"), &s));
	CHECK(paint_only(first) == 0);
	CHECK(render(&f) == 0 && fill(first, p[0].tValue.iColor) > 0 && fill(second, p[0].tValue.iColor) > 0);
	CHECK(xuiTestSurfaceGetLastTextColor(cache(first)) == p[1].tValue.iColor);
	CHECK(fill(first, p[2].tValue.iColor) > 0 && fill(first, 0x96542720) > 0 && fill(first, 0x96542760) > 0);
	p[0] = color("toast.background.color", 0);
	p[1] = color("toast.text.muted_color", 0);
	p[2] = color("toast.info.color", 0);
	OK(xuiWidgetSetInlineStyle(first, p, 3));
	CHECK(render(&f) == 0 && fill(first, 0x876534ff) == 0);
	CHECK(xuiTestSurfaceGetLastTextColor(cache(first)) == 0);
	CHECK(fill(first, 0x00000020) == 0 && fill(first, 0x00000060) == 0);
	OK(xuiToastGetColors(toast, &after));
	CHECK(memcmp(&base, &after, sizeof(base)) == 0);
	OK(xuiWidgetSetInlineStyle(first, NULL, 0));
	OK(xuiStyleRemoveType(f.context, xuiWidgetFindType(f.context, "toast-item")));
	CHECK(render(&f) == 0 && fill(first, base.iBackgroundColor) > 0 && fill(second, base.iBackgroundColor) > 0);
	xuiToastDestroy(toast);
	finish(&f);
	puts("PASS toast per-item rendered palettes, accents and alpha-derived fills, type/inline/clear, service base preservation");
	return 0;
}

static int msgbox_colors(void)
{
	fixture_t f;
	xui_msgbox box;
	xui_msgbox_desc_t desc;
	xui_msgbox_colors_t base, after;
	xui_widget window, content, client, backdrop, button;
	xui_style_property_t p[6];
	xui_style_desc_t s;
	xui_rect_i_t full = {0, 0, 800, 600};
	uint32_t buttonBase, current;
	CHECK(init(&f) == 0);
	memset(&desc, 0, sizeof(desc));
	desc.iSize = sizeof(desc);
	desc.sTitle = "Status";
	desc.sMessage = "Saved message";
	desc.iType = XUI_MSGBOX_ICON_INFO;
	desc.iButtons = XUI_MSGBOX_BUTTON_OK_CANCEL;
	OK(xuiMsgBoxCreate(f.context, &box, &desc));
	OK(xuiMsgBoxGetColors(box, &base));
	base.iClientColor = 0x567831ff;
	base.iIconColor = 0x386714ff;
	OK(xuiMsgBoxSetColors(box, &base));
	OK(xuiMsgBoxSetOpen(box, 1));
	window = xuiMsgBoxGetWindowWidget(box);
	content = xuiMsgBoxGetContentWidget(box);
	client = xuiWindowGetClientWidget(window);
	backdrop = xuiMsgBoxGetBackdropWidget(box);
	button = xuiMsgBoxGetButtonWidget(box, 1);
	OK(xuiButtonSetStateVisual(button, 0, 0x184628ff, 1, 0x615273ff));
	OK(xuiButtonGetStateVisual(button, 0, &buttonBase, NULL, NULL));
	CHECK(render(&f) == 0 && fill(client, base.iClientColor) > 0);
	CHECK(xuiWidgetIsType(window, xuiWindowGetType(f.context)));
	p[0] = color("msgbox.client.color", 0x967453ff);
	p[1] = color("msgbox.backdrop.color", 0x67543277);
	p[2] = color("msgbox.text.color", 0x761259ff);
	p[3] = color("msgbox.icon.color", 0x753219ff);
	p[4] = color("msgbox.button.color", 0x675421ff);
	p[5] = color("msgbox.text.muted_color", 0x716238ff);
	s = style(p, 6);
	OK(xuiStyleSetType(f.context, xuiWidgetFindType(f.context, "msgbox"), &s));
	CHECK(paint_only(window) == 0);
	CHECK(render(&f) == 0 && fill(client, p[0].tValue.iColor) > 0 && fill(backdrop, p[1].tValue.iColor) > 0);
	CHECK(xuiTestSurfaceGetLastTextColor(cache(content)) == p[2].tValue.iColor);
	CHECK(xuiTestSurfaceGetLastColor(cache(content)) == p[3].tValue.iColor);
	CHECK(fill(button, p[4].tValue.iColor) > 0);
	OK(xuiMsgBoxGetColors(box, &after));
	CHECK(memcmp(&base, &after, sizeof(base)) == 0);
	OK(xuiWidgetSetEnabled(button, 0));
	CHECK(render(&f) == 0 && xuiTestSurfaceGetLastTextColor(cache(button)) == p[5].tValue.iColor);
	OK(xuiWidgetSetEnabled(button, 1));
	p[0].tValue.iColor = 0;
	p[1].tValue.iColor = 0;
	p[2].tValue.iColor = 0;
	p[3].tValue.iColor = 0;
	p[4].tValue.iColor = 0;
	OK(xuiWidgetSetInlineStyle(window, p, 5));
	CHECK(paint_only(window) == 0);
	reset_draws(f.root);
	reset_draws(f.context->pOverlayRoot);
	OK(xuiRender(f.context, f.target, &full, 1));
	CHECK(fill(client, 0x967453ff) == 0 && xuiTestSurfaceGetRectFillCount(cache(client)) == 0);
	CHECK(xuiTestSurfaceGetLastTextColor(cache(content)) == 0 && xuiTestSurfaceGetLastColor(cache(content)) == 0);
	CHECK(fill(backdrop, 0x67543277) == 0 && fill(button, 0x675421ff) == 0);
	OK(xuiWidgetSetInlineStyle(window, NULL, 0));
	OK(xuiStyleRemoveType(f.context, xuiWidgetFindType(f.context, "msgbox")));
	CHECK(render(&f) == 0 && fill(client, base.iClientColor) > 0 && fill(backdrop, base.iBackdropColor) > 0);
	OK(xuiButtonGetStateVisual(button, 0, &current, NULL, NULL));
	CHECK(current == buttonBase && fill(button, buttonBase) > 0);
	CHECK(warm_frame(&f) == 0);
	CHECK(window_parent_colors(&f, window) == 0);
	p[0] = color("msgbox.button.color", 0x876534ff);
	OK(xuiWidgetSetInlineStyle(window, p, 1));
	CHECK(render(&f) == 0 && fill(button, p[0].tValue.iColor) > 0);
	OK(xuiButtonSetStateVisual(button, 0, 0x124638ff, 1, 0x615273ff));
	CHECK(render(&f) == 0 && fill(button, p[0].tValue.iColor) > 0);
	OK(xuiWidgetSetInlineStyle(window, NULL, 0));
	CHECK(render(&f) == 0 && fill(button, 0x124638ff) > 0);
	CHECK(warm_frame(&f) == 0);
	xuiMsgBoxDestroy(box);
	finish(&f);
	puts("PASS msgbox composite caches, render-only transparent/clear, API edits, chained Window client/close hook, warm caches");
	return 0;
}

int main(void)
{
	CHECK(popup_colors() == 0);
	CHECK(popup_dependencies() == 0);
	CHECK(menu_colors() == 0);
	CHECK(msgtip_colors() == 0);
	CHECK(toast_colors() == 0);
	CHECK(msgbox_colors() == 0);
	puts("PASS style_overlays");
	return 0;
}
