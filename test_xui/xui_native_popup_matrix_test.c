#define main native_state_reference_main
#include "xui_native_state_matrix_test.c"
#undef main

static int popupImages;
static const uint32_t popupColors[] = {
	XUI_COLOR_RGBA(100, 180, 230, 255), XUI_COLOR_RGBA(80, 190, 110, 255),
	XUI_COLOR_RGBA(210, 130, 70, 255)
};

static int popup_contains_focus(xui_widget popup, xui_widget focus)
{
	for (; focus != NULL; focus = xuiWidgetGetParent(focus)) if (focus == popup) return 1;
	return 0;
}

static void popup_blend(int rgb[3], uint32_t color)
{
	int c, alpha = (int)(color & 255u);
	for (c = 0; c < 3; ++c) {
		int value = (int)((color >> (24 - c * 8)) & 255u);
		rgb[c] = (value * alpha + rgb[c] * (255 - alpha) + 127) / 255;
	}
}

static int popup_image(xui_context context, xui_proxy proxy, xui_surface target,
	xui_widget outer, xui_widget inner, int outerOpen, int innerOpen, int color,
	int border, int quadrant, int cache, float dpi, const char* stage)
{
	unsigned char pixels[SIDE * SIDE * 4];
	xui_rect_t rects[2];
	int x, y, c, i, result;
	result = xuiDispatchPendingEvents(context);
	if (result == XUI_OK) result = xuiRender(context, target, NULL, 0);
	if (result == XUI_OK) result = proxy->surfaceReadRGBA(proxy, target, pixels, SIDE * 4);
	if (result != XUI_OK) return result;
	rects[0] = xuiPopupGetPopupRect(outer);
	rects[1] = xuiPopupGetPopupRect(inner);
	for (y = 0; y < SIDE; ++y) for (x = 0; x < SIDE; ++x) {
		int expected[3] = {255, 255, 255};
		for (i = 0; i < 2; ++i) {
			xui_rect_t r = rects[i];
			uint32_t panel = popupColors[i ? 1 : color];
			if (!(i ? innerOpen : outerOpen)) continue;
			popup_blend(expected, XUI_COLOR_RGBA(0, 0, 0, 64));
			if (contains(r, x, y)) {
				if (x < r.fX + border || x >= r.fX + r.fW - border ||
					y < r.fY + border || y >= r.fY + r.fH - border) panel = BORDER_COLOR;
				popup_blend(expected, panel);
			}
		}
		for (c = 0; c < 4; ++c) {
			int value = c < 3 ? expected[c] : 255;
			int actual = pixels[(y * SIDE + x) * 4 + c];
			if (abs(value - actual) > 2) {
				printf("popup pixels failed: dpi=%.2f quadrant=%d cache=%d border=%d stage=%s xy=%d,%d channel=%d expected=%d actual=%d\n",
					dpi, quadrant, cache, border, stage, x, y, c, value, actual);
				(void)xgeImageSavePNG("build/xui_native_popup_failure.png", SIDE, SIDE, pixels, SIDE * 4);
				return XUI_ERROR_BACKEND_FAILED;
			}
		}
	}
	++popupImages;
	return XUI_OK;
}

static int popup_case(float dpi, int quadrant, int cache, int border)
{
	xui_proxy_t proxy = xuiProxyXge();
	xui_context context = NULL;
	xui_widget root = NULL, parent = NULL, owner = NULL, outer = NULL, inner = NULL;
	xui_widget modalFocus = NULL;
	xui_surface target = NULL;
	xui_surface_desc_t surface = {0};
	xui_popup_desc_t desc = {0};
	xui_cache_policy_t policy = {0};
	xui_rect_t r;
	int result = XUI_OK, i, x = (quadrant & 1) ? 50 : 5, y = (quadrant & 2) ? 50 : 5;
#define TRY(expr) do { result = (expr); if (result != XUI_OK) { printf("popup call failed: %s = %d\n", #expr, result); goto cleanup; } } while (0)
#define CHECK(expr) do { if (!(expr)) { printf("popup interaction failed line %d: %s\n", __LINE__, #expr); result = XUI_ERROR_INVALID_STATE; goto cleanup; } } while (0)
#define INPUT(expr) do { TRY(expr); TRY(xuiDispatchPendingEvents(context)); } while (0)
#define IMAGE(stage, o, n, color) TRY(popup_image(context, &proxy, target, outer, inner, o, n, color, border, quadrant, cache, dpi, stage))
	TRY(xuiCreate(&context));
	TRY(xuiSetProxy(context, &proxy));
	TRY(xuiSetViewportSize(context, SIDE, SIDE));
	TRY(xuiInputViewport(context, SIDE, SIDE));
	TRY(xuiSetVirtualDpi(context, dpi));
	TRY(xuiWidgetCreate(context, &root));
	TRY(xuiWidgetCreate(context, &parent));
	TRY(xuiWidgetCreate(context, &owner));
	TRY(xuiSetRootWidget(context, root));
	TRY(xuiWidgetAddChild(root, parent));
	TRY(xuiWidgetAddChild(parent, owner));
	TRY(xuiWidgetSetRect(parent, (xui_rect_t){-5, 7, 64, 64}));
	TRY(xuiWidgetSetRect(owner, (xui_rect_t){x + 5, y - 7, 8, 6}));
	TRY(xuiWidgetSetFocusable(owner, 1));
	TRY(xuiWidgetSetCacheRenderCallback(root, white_background, &proxy));
	policy = xuiWidgetGetCachePolicy(root);
	policy.iPolicy = (cache & 1) ? XUI_CACHE_POLICY_SUBTREE : XUI_CACHE_POLICY_SELF;
	TRY(xuiWidgetSetCachePolicy(root, &policy));
	desc.iSize = sizeof(desc); desc.pOwner = owner;
	desc.fContentWidth = 20; desc.fContentHeight = 14;
	desc.fPadding = 1; desc.fBorderWidth = (float)border; desc.fMargin = 2; desc.fGap = 2;
	desc.iAnchor = XUI_POPUP_ANCHOR_BOTTOM_LEFT;
	desc.iDirection = XUI_POPUP_DIRECTION_RIGHT_DOWN;
	desc.iFocusPolicy = XUI_POPUP_FOCUS_POPUP;
	desc.iOutsidePolicy = XUI_POPUP_OUTSIDE_CLOSE;
	desc.iOwnerPolicy = XUI_POPUP_OWNER_PASSTHROUGH;
	desc.iEscapePolicy = XUI_POPUP_ESCAPE_CLOSE;
	desc.bModal = 1;
	desc.iPanelColor = popupColors[0]; desc.iBorderColor = BORDER_COLOR;
	desc.iBackdropColor = XUI_COLOR_RGBA(0, 0, 0, 64);
	TRY(xuiPopupCreate(context, &outer, &desc));
	desc.pOwner = outer; desc.iPanelColor = popupColors[1];
	TRY(xuiPopupCreate(context, &inner, &desc));
	TRY(xuiPopupSetAnchorRect(inner, (xui_rect_t){16, 20, 4, 4}));
	for (i = 0; i < 2; ++i) {
		xui_widget popup = i ? inner : outer;
		policy = xuiWidgetGetCachePolicy(popup);
		policy.iPolicy = (cache & 2) ? XUI_CACHE_POLICY_SUBTREE : XUI_CACHE_POLICY_SELF;
		TRY(xuiWidgetSetCachePolicy(popup, &policy));
		policy.iPolicy = cache == 3 ? XUI_CACHE_POLICY_SUBTREE : XUI_CACHE_POLICY_SELF;
		TRY(xuiWidgetSetCachePolicy(xuiPopupGetPanelWidget(popup), &policy));
	}
	surface.iWidth = surface.iHeight = SIDE;
	surface.iKind = XUI_SURFACE_KIND_TEXTURE; surface.iFormat = XUI_SURFACE_FORMAT_RGBA8;
	surface.iFlags = XUI_SURFACE_USAGE_TARGET | XUI_SURFACE_ALPHA_PREMULTIPLIED;
	TRY(proxy.surfaceCreate(&proxy, &target, &surface));
	TRY(xuiLayout(context));
	TRY(xuiSetFocusWidget(context, owner));
	IMAGE("closed", 0, 0, 0);
	TRY(xuiPopupSetOpen(outer, 1));
	CHECK(xuiGetFocusWidget(context) == outer);
	r = xuiPopupGetPopupRect(outer);
	CHECK(r.fW == 20 + 2 * (border + 1) && r.fH == 14 + 2 * (border + 1));
	CHECK(r.fX == ((quadrant & 1) ? x + 8 - r.fW : x));
	CHECK(r.fY == ((quadrant & 2) ? y - 2 - r.fH : y + 6 + 2));
	IMAGE("open-flip", 1, 0, 0);
	INPUT(xuiInputKeyDown(context, XUI_KEY_TAB, 0));
	modalFocus = xuiGetFocusWidget(context);
	CHECK(popup_contains_focus(outer, modalFocus));
	CHECK(xuiSetFocusWidget(context, owner) != XUI_OK);
	CHECK(xuiGetFocusWidget(context) == modalFocus);
	IMAGE("modal-focus", 1, 0, 0);
	TRY(xuiPopupSetOpen(inner, 1));
	CHECK(xuiGetFocusWidget(context) == inner);
	IMAGE("nested", 1, 1, 0);
	INPUT(xuiInputKeyDown(context, XUI_KEY_ESCAPE, 0));
	CHECK(!xuiPopupIsOpen(inner) && xuiPopupIsOpen(outer) && xuiGetFocusWidget(context) == modalFocus);
	IMAGE("nested-close", 1, 0, 0);
	TRY(xuiPopupSetAnchorRect(outer, (xui_rect_t){22, 6, 7, 5}));
	IMAGE("moved-damage", 1, 0, 0);
	TRY(xuiPopupSetColors(outer, popupColors[2], BORDER_COLOR, 0, XUI_COLOR_RGBA(0, 0, 0, 64)));
	IMAGE("changed-cache", 1, 0, 2);
	INPUT(xuiInputPointerDown(context, 62, 62, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT));
	INPUT(xuiInputPointerUp(context, 62, 62, XUI_POINTER_BUTTON_LEFT, 0));
	CHECK(!xuiPopupIsOpen(outer) && xuiGetFocusWidget(context) == owner);
	IMAGE("outside-close", 0, 0, 2);
	TRY(xuiPopupSetOpen(outer, 1));
	IMAGE("reopened", 1, 0, 2);
	TRY(xuiWidgetSetVisible(parent, 0));
	CHECK(!xuiPopupIsOpen(outer) && !xuiPopupIsOpen(inner));
	IMAGE("owner-hidden", 0, 0, 2);
cleanup:
	if (context != NULL) xuiDestroy(context);
	if (target != NULL) proxy.surfaceDestroy(&proxy, target);
#undef TRY
#undef CHECK
#undef INPUT
#undef IMAGE
	return result;
}

static int popup_frame(void* user)
{
	const float dpi[] = {1.0f, 1.25f, 1.5f, 2.0f};
	int* result = (int*)user;
	int d, q, c, b;
	*result = xgeBegin();
	if (*result != XGE_OK) return *result;
	for (d = 0; d < 4 && *result == XUI_OK; ++d)
		for (q = 0; q < 4 && *result == XUI_OK; ++q)
			for (c = 0; c < 4 && *result == XUI_OK; ++c)
				for (b = 1; b <= 2 && *result == XUI_OK; ++b) *result = popup_case(dpi[d], q, c, b);
	if (xgeEnd() != XGE_OK && *result == XUI_OK) *result = XUI_ERROR_BACKEND_FAILED;
	xgeQuit();
	return *result;
}

int main(void)
{
	xge_desc_t desc = {0};
	int result = XUI_ERROR_NOT_INITIALIZED, run;
	desc.iWidth = desc.iHeight = SIDE; desc.iFlags = XGE_INIT_OFFSCREEN;
	desc.iRunMode = XGE_RUN_GAME_LOOP; desc.sTitle = "xui_native_popup_matrix_test";
	run = xgeInit(&desc);
	if (run == XGE_OK) run = xgeRun(popup_frame, &result);
	xgeUnit();
	printf("xui_native_popup_matrix_test: run=%d result=%d verified_images=%d\n", run, result, popupImages);
	return run == XGE_OK && result == XUI_OK && popupImages == 1280 ? 0 : 1;
}
