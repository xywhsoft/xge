#include "xui.h"
#include "xge.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SIDE 64
#define BORDER_COLOR XUI_COLOR_RGBA(0, 0, 0, 255)
static const uint32_t fills[] = {
	XUI_COLOR_RGBA(180, 80, 60, 255), XUI_COLOR_RGBA(70, 150, 80, 255),
	XUI_COLOR_RGBA(60, 90, 180, 255), XUI_COLOR_RGBA(150, 150, 150, 255),
	XUI_COLOR_RGBA(170, 90, 170, 255)
};
static int imageCount;

static int white_background(xui_widget widget, xui_draw_context draw, uint32_t state, void* user)
{
	xui_proxy proxy = (xui_proxy)user;
	xui_rect_t rect = xuiWidgetGetRect(widget);
	(void)state;
	rect.fX = rect.fY = 0;
	return proxy->drawRectFill(proxy, draw, rect, XUI_COLOR_WHITE);
}

static int contains(xui_rect_t rect, int x, int y)
{
	return x >= rect.fX && x < rect.fX + rect.fW && y >= rect.fY && y < rect.fY + rect.fH;
}

static int check_image(xui_context context, xui_proxy proxy, xui_surface target, xui_widget button,
	xui_widget parent, int clipped, int visible, int fill, int border, float dpi, int geometry, int cache, const char* stage)
{
	unsigned char pixels[SIDE * SIDE * 4];
	xui_rect_t rect = xuiWidgetGetWorldRect(button);
	xui_rect_t clip = clipped ? xuiWidgetGetWorldRect(parent) : (xui_rect_t){0, 0, SIDE, SIDE};
	int x, y, c, result;
	result = xuiDispatchPendingEvents(context);
	if (result != XUI_OK) return result;
	result = xuiRender(context, target, NULL, 0);
	if (result != XUI_OK) return result;
	result = proxy->surfaceReadRGBA(proxy, target, pixels, SIDE * 4);
	if (result != XUI_OK) return result;
	for (y = 0; y < SIDE; ++y) {
		for (x = 0; x < SIDE; ++x) {
			uint32_t color = XUI_COLOR_WHITE;
			if (visible && contains(rect, x, y) && contains(clip, x, y)) {
				color = (x < rect.fX + border || x >= rect.fX + rect.fW - border ||
					y < rect.fY + border || y >= rect.fY + rect.fH - border) ? BORDER_COLOR : fills[fill];
			}
			for (c = 0; c < 4; ++c) {
				int expected = (int)((color >> (24 - c * 8)) & 255u);
				int actual = pixels[(y * SIDE + x) * 4 + c];
				if (abs(actual - expected) > 1) {
					printf("native matrix failed: dpi=%.2f geometry=%d cache=%d border=%d stage=%s x=%d y=%d channel=%d expected=%d actual=%d\n",
						dpi, geometry, cache, border, stage, x, y, c, expected, actual);
					(void)xgeImageSavePNG("build/xui_native_matrix_failure.png", SIDE, SIDE, pixels, SIDE * 4);
					return XUI_ERROR_BACKEND_FAILED;
				}
			}
		}
	}
	++imageCount;
	return XUI_OK;
}

static int run_case(float dpi, int geometry, int cache, int border)
{
	xui_proxy_t proxy = xuiProxyXge();
	xui_context context = NULL;
	xui_widget root = NULL, parent = NULL, button = NULL;
	xui_surface target = NULL;
	xui_surface_desc_t desc = {0};
	xui_cache_policy_t policy = {0};
	xui_rect_t rect;
	int result = XUI_OK, x, y, i;
	const uint32_t states[] = {0, XUI_WIDGET_STATE_HOVER, XUI_WIDGET_STATE_ACTIVE,
		XUI_WIDGET_STATE_DISABLED, XUI_BUTTON_STATE_CHECKED};
#define TRY(expr) do { result = (expr); if (result != XUI_OK) goto cleanup; } while (0)
#define INPUT(expr) do { TRY(expr); TRY(xuiDispatchPendingEvents(context)); } while (0)
#define CHECK(expr) do { if (!(expr)) { printf("native matrix interaction failed: %s\n", #expr); result = XUI_ERROR_INVALID_STATE; goto cleanup; } } while (0)
#define IMAGE(name, fill, visible) TRY(check_image(context, &proxy, target, button, parent, geometry == 2, visible, fill, border, dpi, geometry, cache, name))
	TRY(xuiCreate(&context));
	TRY(xuiSetProxy(context, &proxy));
	TRY(xuiSetViewportSize(context, SIDE, SIDE));
	TRY(xuiInputViewport(context, SIDE, SIDE));
	TRY(xuiSetVirtualDpi(context, dpi));
	TRY(xuiWidgetCreate(context, &root));
	TRY(xuiWidgetCreate(context, &parent));
	TRY(xuiButtonCreate(context, &button, NULL));
	TRY(xuiSetRootWidget(context, root));
	TRY(xuiWidgetSetRect(parent, geometry == 1 ? (xui_rect_t){-9, -7, 48, 48} :
		geometry == 2 ? (xui_rect_t){11, 13, 20, 17} : (xui_rect_t){11, 13, 36, 32}));
	TRY(xuiWidgetSetRect(button, geometry == 1 ? (xui_rect_t){15, 17, 31, 21} :
		geometry == 2 ? (xui_rect_t){-6, -5, 32, 23} : (xui_rect_t){3, 5, 32, 22}));
	TRY(xuiWidgetSetOverflow(parent, geometry == 2 ? XUI_OVERFLOW_CLIP : XUI_OVERFLOW_VISIBLE));
	TRY(xuiWidgetAddChild(root, parent));
	TRY(xuiWidgetAddChild(parent, button));
	TRY(xuiWidgetSetCacheRenderCallback(root, white_background, &proxy));
	for (i = 0; i < 5; ++i) TRY(xuiButtonSetStateVisual(button, states[i], fills[i], border, BORDER_COLOR));
	TRY(xuiButtonSetStateVisual(button, XUI_WIDGET_STATE_FOCUS, fills[0], border, BORDER_COLOR));
	policy = xuiWidgetGetCachePolicy(root);
	policy.iPolicy = (cache & 1) ? XUI_CACHE_POLICY_SUBTREE : XUI_CACHE_POLICY_SELF;
	TRY(xuiWidgetSetCachePolicy(root, &policy));
	policy.iPolicy = (cache & 2) ? XUI_CACHE_POLICY_SUBTREE : XUI_CACHE_POLICY_SELF;
	TRY(xuiWidgetSetCachePolicy(parent, &policy));
	desc.iWidth = desc.iHeight = SIDE;
	desc.iKind = XUI_SURFACE_KIND_TEXTURE;
	desc.iFormat = XUI_SURFACE_FORMAT_RGBA8;
	desc.iFlags = XUI_SURFACE_USAGE_TARGET | XUI_SURFACE_ALPHA_PREMULTIPLIED;
	TRY(proxy.surfaceCreate(&proxy, &target, &desc));
	TRY(xuiLayout(context));
	rect = xuiWidgetGetWorldRect(button);
	x = rect.fX + rect.fW / 2;
	y = rect.fY + rect.fH / 2;
	IMAGE("normal", 0, 1);
	CHECK(xuiHitTest(context, x, y, 0) == button);
	INPUT(xuiInputPointerMove(context, x, y, 0));
	IMAGE("hover", 1, 1);
	INPUT(xuiInputPointerDown(context, x, y, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT));
	IMAGE("pressed", 2, 1);
	INPUT(xuiInputPointerUp(context, x, y, XUI_POINTER_BUTTON_LEFT, 0));
	CHECK(xuiButtonGetClickCount(button) == 1);
	INPUT(xuiInputPointerMove(context, 62, 62, 0));
	CHECK(xuiGetFocusWidget(context) == button);
	IMAGE("focus", 0, 1);
	INPUT(xuiInputKeyDown(context, XUI_KEY_SPACE, 0));
	IMAGE("keyboard-pressed", 2, 1);
	INPUT(xuiInputKeyUp(context, XUI_KEY_SPACE, 0));
	CHECK(xuiButtonGetClickCount(button) == 2);
	TRY(xuiButtonSetSelected(button, 1));
	IMAGE("selected", 4, 1);
	TRY(xuiWidgetSetEnabled(parent, 0));
	CHECK(xuiGetFocusWidget(context) == NULL && xuiHitTest(context, x, y, 0) != button);
	IMAGE("ancestor-disabled", 3, 1);
	TRY(xuiWidgetSetEnabled(parent, 1));
	TRY(xuiButtonSetSelected(button, 0));
	IMAGE("restored", 0, 1);
	TRY(xuiWidgetSetVisible(parent, 0));
	IMAGE("hidden-damage", 0, 0);
	TRY(xuiWidgetSetVisible(parent, 1));
	rect = xuiWidgetGetRect(parent);
	rect.fX += 4;
	rect.fY += 3;
	TRY(xuiWidgetSetRect(parent, rect));
	IMAGE("moved-damage", 0, 1);
cleanup:
	if (context != NULL) xuiDestroy(context);
	if (target != NULL) proxy.surfaceDestroy(&proxy, target);
#undef TRY
#undef INPUT
#undef CHECK
#undef IMAGE
	return result;
}

static int frame(void* user)
{
	const float dpi[] = {1.0f, 1.25f, 1.5f, 2.0f};
	int* result = (int*)user;
	int d, g, c, border;
	*result = xgeBegin();
	if (*result != XGE_OK) return *result;
	for (d = 0; d < 4 && *result == XUI_OK; ++d)
		for (g = 0; g < 3 && *result == XUI_OK; ++g)
			for (c = 0; c < 4 && *result == XUI_OK; ++c)
				for (border = 1; border <= 2 && *result == XUI_OK; ++border)
					*result = run_case(dpi[d], g, c, border);
	if (xgeEnd() != XGE_OK && *result == XUI_OK) *result = XUI_ERROR_BACKEND_FAILED;
	xgeQuit();
	return *result;
}

int main(void)
{
	xge_desc_t desc = {0};
	int result = XUI_ERROR_NOT_INITIALIZED;
	int run;
	desc.iWidth = desc.iHeight = SIDE;
	desc.iFlags = XGE_INIT_OFFSCREEN;
	desc.iRunMode = XGE_RUN_GAME_LOOP;
	desc.sTitle = "xui_native_state_matrix_test";
	run = xgeInit(&desc);
	if (run == XGE_OK) run = xgeRun(frame, &result);
	xgeUnit();
	printf("xui_native_state_matrix_test: run=%d result=%d verified_images=%d\n", run, result, imageCount);
	return run == XGE_OK && result == XUI_OK && imageCount == 960 ? 0 : 1;
}
