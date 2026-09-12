#include "../src/xui_internal.h"
#include "xui_test_proxy.h"
#include <stdio.h>
#include <string.h>

static int failures;
#define CHECK(e) do { if (!(e)) { ++failures; printf("prepare paint: %d: %s\n", __LINE__, #e); } } while (0)

typedef struct fixture_t {
	xui_context context;
	xui_widget owner, child;
	uint32_t color, painted;
	int prepares, paints, mode, inside, destroyed;
} fixture_t;

static void destroyed(xui_widget widget, void* data, void* user)
{
	fixture_t* f = user;
	(void)widget; (void)data;
	CHECK(!f->inside);
	++f->destroyed;
}

static int prepare(xui_widget widget)
{
	fixture_t* f = xuiWidgetGetUserData(widget);
	xui_style_property_t value;
	uint32_t color = 0x123456ff;
	++f->prepares;
	CHECK(f->context->iOperationDepth > 0);
	if (f->mode) {
		f->inside = 1;
		if (f->mode == 1) xuiWidgetDestroy(widget);
		else if (f->mode == 2) xuiDestroy(f->context);
		else CHECK(xuiWidgetSetVisible(widget, 0) == XUI_OK);
		CHECK(f->destroyed == 0);
		f->inside = 0;
		return XUI_OK;
	}
	if (xuiWidgetGetResolvedStyleProperty(widget, "test.paint.color", &value) == XUI_OK &&
		value.tValue.iType == XUI_STYLE_VALUE_COLOR) color = value.tValue.iColor;
	if (color != f->color) {
		f->color = color;
		xuiWidgetInvalidate(f->child, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	}
	return XUI_OK;
}

static int paint(xui_widget widget, xui_draw_context draw, uint32_t state, void* user)
{
	fixture_t* f = user;
	(void)widget; (void)draw; (void)state;
	CHECK(f->prepares > 0);
	f->painted = f->color;
	++f->paints;
	return XUI_OK;
}

static void run_case(int mode, int overlay, int initial_style)
{
	xui_test_proxy_state_t proxy;
	fixture_t f = {0};
	xui_widget root;
	xui_widget_type parent, derived;
	xui_widget_type_desc_t desc = {0};
	xui_style_property_info_t info = {0};
	xui_style_property_t prop = {0};
	xui_style_desc_t style = {0};
	xui_cache_policy_t policy;
	int count;
	f.mode = mode;
	xuiTestProxyInit(&proxy);
	CHECK(xuiCreate(&f.context) == XUI_OK);
	CHECK(xuiSetProxy(f.context, &proxy.tProxy) == XUI_OK);
	CHECK(xuiSetViewportSize(f.context, 200, 100) == XUI_OK);
	CHECK(xuiWidgetCreate(f.context, &root) == XUI_OK);
	CHECK(xuiSetRootWidget(f.context, root) == XUI_OK);
	CHECK(xuiWidgetSetLayoutType(root, XUI_LAYOUT_MANUAL) == XUI_OK);
	if (overlay) root = xuiOverlayRoot(f.context);
	desc.iSize = sizeof(desc); desc.sName = "test.prepare.base";
	desc.onDestroy = destroyed; desc.pUser = &f;
	CHECK(xuiWidgetRegisterType(f.context, &parent, &desc) == XUI_OK);
	parent->onPreparePaint = prepare;
	desc.sName = "test.prepare.derived"; desc.pParent = parent;
	desc.onDestroy = NULL;
	CHECK(xuiWidgetRegisterType(f.context, &derived, &desc) == XUI_OK);
	info.iSize = sizeof(info); info.sName = "test.paint.color";
	info.pWidgetType = parent; info.iValueType = XUI_STYLE_VALUE_COLOR;
	info.iDirtyFlags = XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER;
	CHECK(xuiStyleRegisterProperty(f.context, &info, NULL) == XUI_OK);
	prop.iSize = sizeof(prop); prop.sName = "test.paint.color";
	prop.tValue.iSize = sizeof(prop.tValue); prop.tValue.iType = XUI_STYLE_VALUE_COLOR;
	prop.tValue.iColor = 0x314159ff;
	if (initial_style == 2) CHECK(xuiStyleSetDefault(f.context, &prop, 1) == XUI_OK);
	CHECK(xuiWidgetCreateTyped(f.context, derived, &f.owner, NULL) == XUI_OK);
	xuiWidgetSetUserData(f.owner, &f);
	if (initial_style != 1) CHECK(xuiWidgetAddChild(root, f.owner) == XUI_OK);
	CHECK(xuiWidgetSetRect(f.owner, (xui_rect_t){5, 8, 180, 80}) == XUI_OK);
	CHECK(xuiWidgetSetLayoutType(f.owner, XUI_LAYOUT_MANUAL) == XUI_OK);
	CHECK(xuiWidgetCreate(f.context, &f.child) == XUI_OK);
	CHECK(xuiWidgetAddChild(f.owner, f.child) == XUI_OK);
	CHECK(xuiWidgetSetRect(f.child, (xui_rect_t){2, 4, 100, 50}) == XUI_OK);
	CHECK(xuiWidgetSetCacheRenderCallback(f.child, paint, &f) == XUI_OK);
	policy = xuiWidgetGetCachePolicy(f.child);
	policy.iFlags |= XUI_CACHE_UPDATE_ALL_STATES;
	CHECK(xuiWidgetSetCachePolicy(f.child, &policy) == XUI_OK);
	CHECK(xuiWidgetSetCacheStateCount(f.child, 3) == XUI_OK);
	CHECK(xuiWidgetSetCacheStateId(f.child, 0, 0) == XUI_OK);
	CHECK(xuiWidgetSetCacheStateId(f.child, 1, 1) == XUI_OK);
	CHECK(xuiWidgetSetCacheStateId(f.child, 2, 2) == XUI_OK);
	if (initial_style == 1) {
		CHECK(xuiStyleSetDefault(f.context, &prop, 1) == XUI_OK);
		CHECK(xuiWidgetAddChild(root, f.owner) == XUI_OK);
	}
	CHECK(xuiRenderPrepare(f.context) == XUI_OK);
	CHECK(f.prepares == 1);
	if (!mode) {
		CHECK(f.paints == 3 && f.painted == (initial_style ? 0x314159ffu : 0x123456ffu));
		count = f.paints;
		CHECK(xuiRenderPrepare(f.context) == XUI_OK && f.paints == count);
		prop.tValue.iColor = 0x876543ff;
		CHECK(xuiStyleSetDefault(f.context, &prop, 1) == XUI_OK);
		CHECK(xuiRenderPrepare(f.context) == XUI_OK && f.painted == prop.tValue.iColor && f.paints == count + 3);
		count = f.paints;
		CHECK(xuiRenderPrepare(f.context) == XUI_OK && f.paints == count);
		prop.tValue.iColor = 0xabcdefff;
		style.iSize = sizeof(style); style.pProperties = &prop; style.iPropertyCount = 1;
		CHECK(xuiStyleSetType(f.context, parent, &style) == XUI_OK);
		CHECK(xuiRenderPrepare(f.context) == XUI_OK && f.painted == prop.tValue.iColor);
		prop.tValue.iColor = 0;
		CHECK(xuiWidgetSetInlineStyle(f.owner, &prop, 1) == XUI_OK);
		CHECK(xuiRenderPrepare(f.context) == XUI_OK && f.painted == 0);
		CHECK(xuiWidgetSetInlineStyle(f.owner, NULL, 0) == XUI_OK);
		style.pProperties = NULL; style.iPropertyCount = 0;
		CHECK(xuiStyleSetType(f.context, parent, &style) == XUI_OK);
		CHECK(xuiStyleClearDefault(f.context) == XUI_OK);
		CHECK(xuiRenderPrepare(f.context) == XUI_OK && f.painted == 0x123456ff);
		CHECK((f.owner->iDirtyFlags & XUI_WIDGET_DIRTY_LAYOUT) == 0);
	} else {
		CHECK(f.paints == 0);
		CHECK(f.destroyed == (mode == 3 ? 0 : 1));
	}
	if (mode != 2) xuiDestroy(f.context);
	CHECK(f.destroyed == 1);
}

int main(void)
{
	int mode, overlay;
	for (mode = 0; mode < 4; ++mode)
		for (overlay = 0; overlay < 2; ++overlay) run_case(mode, overlay, 0);
	for (overlay = 0; overlay < 2; ++overlay) {
		run_case(0, overlay, 1);
		run_case(0, overlay, 2);
	}
	printf("xui_prepare_paint_test: %d failures\n", failures);
	return failures != 0;
}
