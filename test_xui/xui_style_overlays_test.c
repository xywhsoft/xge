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
	xui_test_proxy_state_t proxy;
} fixture_t;

static int init(fixture_t* f)
{
	memset(f, 0, sizeof(*f));
	xuiTestProxyInit(&f->proxy);
	OK(xuiCreate(&f->context));
	OK(xuiSetProxy(f->context, &f->proxy.tProxy));
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
	OK(xuiUpdate(f->context, 0.0f));
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
	finish(&f);
	puts("PASS popup rendered palette, cache transitions, clear, token/default/type/inline, transparent zero, paint-only");
	return 0;
}

int main(void)
{
	CHECK(popup_colors() == 0);
	puts("PASS style_overlays");
	return 0;
}
