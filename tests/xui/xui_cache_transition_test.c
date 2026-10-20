#define main xui_render_schedule_reference_main
#include "xui_render_schedule_test.c"
#undef main

static int cache_transitions(void)
{
	xui_proxy_t proxy = __xuiTestProxy();
	xui_context context = NULL;
	xui_widget root = NULL, branch = NULL, leaf = NULL;
	xui_surface target = NULL;
	xui_draw_context draw = NULL;
	xui_surface_desc_t desc = {0};
	xui_cache_policy_t policy = {0};
	unsigned char expected[64 * 64 * 4];
	uint32_t white = XUI_COLOR_WHITE;
	uint32_t red = XUI_COLOR_RGBA(255, 0, 0, 128);
	uint32_t blue = XUI_COLOR_RGBA(0, 0, 255, 128);
	int policies[] = {XUI_CACHE_POLICY_SUBTREE, XUI_CACHE_POLICY_SELF,
		XUI_CACHE_POLICY_AUTO, XUI_CACHE_POLICY_NONE, XUI_CACHE_POLICY_SELF};
	int iFailed = 0;
	int i, state;
	XUI_TEST_CHECK(xuiCreate(&context) == XUI_OK &&
		xuiSetProxy(context, &proxy) == XUI_OK &&
		xuiSetViewportSize(context, 64, 64) == XUI_OK, "transition context");
	XUI_TEST_CHECK(xuiWidgetCreate(context, &root) == XUI_OK &&
		xuiWidgetCreate(context, &branch) == XUI_OK &&
		xuiWidgetCreate(context, &leaf) == XUI_OK, "transition widgets");
	(void)xuiSetRootWidget(context, root);
	(void)xuiWidgetSetRect(branch, (xui_rect_t){7, 9, 40, 40});
	(void)xuiWidgetSetRect(leaf, (xui_rect_t){3, 5, 20, 20});
	(void)xuiWidgetAddChild(root, branch);
	(void)xuiWidgetAddChild(branch, leaf);
	(void)xuiWidgetSetCacheRenderCallback(root, __xuiTestSolidDraw, &white);
	(void)xuiWidgetSetCacheRenderCallback(leaf, __xuiTestSolidDraw, &red);
	desc.iWidth = desc.iHeight = 64;
	desc.iKind = XUI_SURFACE_KIND_TEXTURE;
	desc.iFormat = XUI_SURFACE_FORMAT_RGBA8;
	desc.iFlags = XUI_SURFACE_USAGE_TARGET | XUI_SURFACE_ALPHA_PREMULTIPLIED;
	XUI_TEST_CHECK(proxy.surfaceCreate(&proxy, &target, &desc) == XUI_OK, "transition target");
	XUI_TEST_CHECK(xuiRender(context, target, NULL, 0) == XUI_OK, "self baseline");
	memcpy(expected, target->pPixels, sizeof(expected));
	policy = xuiWidgetGetCachePolicy(branch);
	for (i = 0; i < (int)(sizeof(policies) / sizeof(policies[0])); ++i) {
		policy.iPolicy = policies[i];
		XUI_TEST_CHECK(xuiWidgetSetCachePolicy(branch, &policy) == XUI_OK, "switch policy");
		for (state = 0; state < 2; ++state) {
			(void)xuiWidgetSetStateId(branch, (uint32_t)state);
			XUI_TEST_CHECK(xuiRender(context, target, NULL, 0) == XUI_OK, "transition render");
			XUI_TEST_CHECK(memcmp(expected, target->pPixels, sizeof(expected)) == 0,
				"policy transition reused descendants or doubled translucent pixels");
		}
	}
	/* A manually populated SELF cache remains owned by its caller on warm frames. */
	XUI_TEST_CHECK(xuiWidgetUpdateBegin(branch, 1, XUI_WIDGET_UPDATE_CLEAR, 0, &draw) == XUI_OK,
		"manual self begin");
	XUI_TEST_CHECK(__xuiTestSolidDraw(branch, draw, 1, &blue) == XUI_OK, "manual self paint");
	XUI_TEST_CHECK(xuiWidgetUpdateEnd(branch, 1, draw) == XUI_OK, "manual self end");
	draw = NULL;
	XUI_TEST_CHECK(xuiRender(context, target, NULL, 0) == XUI_OK &&
		__xuiPixelEquals(target->pPixels, 256, 8, 10, 127, 127, 255, 255), "manual self missing");
	XUI_TEST_CHECK(xuiRender(context, target, NULL, 0) == XUI_OK &&
		__xuiPixelEquals(target->pPixels, 256, 8, 10, 127, 127, 255, 255), "manual self purged on warm frame");
	policy.iPolicy = XUI_CACHE_POLICY_SUBTREE;
	XUI_TEST_CHECK(xuiWidgetSetCachePolicy(branch, &policy) == XUI_OK, "manual to subtree");
	XUI_TEST_CHECK(xuiRender(context, target, NULL, 0) == XUI_OK &&
		memcmp(expected, target->pPixels, sizeof(expected)) == 0, "manual self leaked into subtree");
	policy.iPolicy = XUI_CACHE_POLICY_AUTO;
	(void)xuiWidgetSetCachePolicy(branch, &policy);
	for (state = 0; state < 2; ++state) {
		(void)xuiWidgetSetStateId(branch, (uint32_t)state);
		XUI_TEST_CHECK(xuiRender(context, target, NULL, 0) == XUI_OK, "auto subtree prime");
	}
	XUI_TEST_CHECK(xuiWidgetRemoveFromParent(leaf) == XUI_OK, "auto remove child");
	for (state = 0; state < 2; ++state) {
		(void)xuiWidgetSetStateId(branch, (uint32_t)state);
		XUI_TEST_CHECK(xuiRender(context, target, NULL, 0) == XUI_OK &&
			__xuiPixelEquals(target->pPixels, 256, 12, 16, 255, 255, 255, 255),
			"auto leaf retained a removed child in an inactive cache state");
	}
	(void)xuiWidgetAddChild(branch, leaf);
	XUI_TEST_CHECK(xuiRender(context, target, NULL, 0) == XUI_OK &&
		memcmp(expected, target->pPixels, sizeof(expected)) == 0, "auto reattach child");
cleanup:
	if (draw != NULL) (void)xuiWidgetUpdateEnd(branch, 1, draw);
	if (leaf != NULL && xuiWidgetGetParent(leaf) == NULL) xuiWidgetDestroy(leaf);
	if (target != NULL) proxy.surfaceDestroy(&proxy, target);
	if (context != NULL) xuiDestroy(context);
	return !iFailed;
}

static int empty_edit_font_metrics(xui_proxy proxy, xui_font font, xui_font_metrics_t* metrics)
{
	(void)proxy;
	(void)font;
	memset(metrics, 0, sizeof(*metrics));
	metrics->fAscent = 12;
	metrics->fDescent = 4;
	metrics->fLineHeight = 16;
	return XUI_OK;
}

static int closed_window_empty_text_edit_first_paint(void)
{
	xui_proxy_t proxy = __xuiTestProxy();
	struct xui_font_t font = {0};
	xui_context context = NULL;
	xui_widget root = NULL;
	xui_widget window = NULL;
	xui_widget client;
	xui_widget toolbar = NULL;
	xui_widget edit = NULL;
	xui_surface target = NULL;
	xui_surface_desc_t surface_desc = {0};
	xui_window_desc_t window_desc = {0};
	xui_text_edit_desc_t edit_desc = {0};
	xui_rect_t edit_world;
	int x;
	int y;
	int iFailed = 0;

	proxy.fontGetMetrics = empty_edit_font_metrics;
	XUI_TEST_CHECK(xuiCreate(&context) == XUI_OK &&
		xuiSetProxy(context, &proxy) == XUI_OK &&
		xuiSetViewportSize(context, 320, 200) == XUI_OK,
		"closed window context");
	XUI_TEST_CHECK(xuiWidgetCreate(context, &root) == XUI_OK &&
		xuiSetRootWidget(context, root) == XUI_OK,
		"closed window root");
	window_desc.iSize = sizeof(window_desc);
	window_desc.sTitle = "Closed window";
	window_desc.bClosed = 1;
	window_desc.bHideCollapse = 1;
	window_desc.bHideMaximize = 1;
	XUI_TEST_CHECK(xuiWindowCreate(context, &window, &window_desc) == XUI_OK,
		"closed window create");
	(void)xuiWidgetSetRect(window, (xui_rect_t){10, 10, 300, 160});
	client = xuiWindowGetClientWidget(window);
	XUI_TEST_CHECK(client != NULL && xuiWidgetSetLayoutType(client, XUI_LAYOUT_COLUMN) == XUI_OK,
		"closed window client");
	XUI_TEST_CHECK(xuiWidgetCreate(context, &toolbar) == XUI_OK,
		"closed window toolbar");
	(void)xuiWidgetSetLayoutType(toolbar, XUI_LAYOUT_ROW);
	(void)xuiWidgetSetSizeMode(toolbar, XUI_SIZE_FILL, XUI_SIZE_FIXED);
	(void)xuiWidgetSetPreferredSize(toolbar, (xui_vec2_t){0, 36});
	edit_desc.iSize = sizeof(edit_desc);
	edit_desc.pFont = &font;
	edit_desc.sPlaceholder = "Empty input";
	XUI_TEST_CHECK(xuiTextEditCreate(context, &edit, &edit_desc) == XUI_OK,
		"closed window text edit");
	(void)xuiWidgetSetSizeMode(edit, XUI_SIZE_FIXED, XUI_SIZE_FILL);
	(void)xuiWidgetSetPreferredSize(edit, (xui_vec2_t){180, 0});
	XUI_TEST_CHECK(xuiWidgetAddChild(toolbar, edit) == XUI_OK &&
		xuiWindowAddChild(window, toolbar) == XUI_OK &&
		xuiWidgetAddChild(root, window) == XUI_OK,
		"closed window attach tree");
	XUI_TEST_CHECK(xuiWindowSetOpen(window, 1) == XUI_OK,
		"closed window open");

	surface_desc.iWidth = 320;
	surface_desc.iHeight = 200;
	surface_desc.iKind = XUI_SURFACE_KIND_TEXTURE;
	surface_desc.iFormat = XUI_SURFACE_FORMAT_RGBA8;
	surface_desc.iFlags = XUI_SURFACE_USAGE_TARGET | XUI_SURFACE_ALPHA_PREMULTIPLIED;
	XUI_TEST_CHECK(proxy.surfaceCreate(&proxy, &target, &surface_desc) == XUI_OK,
		"closed window target");
	XUI_TEST_CHECK(xuiRender(context, target, NULL, 0) == XUI_OK,
		"closed window first render");
	XUI_TEST_CHECK(xuiTextEditGetLineCount(edit) == 1,
		"empty text edit must retain one caret line");
	edit_world = xuiWidgetGetWorldRect(edit);
	x = edit_world.fX + edit_world.fW / 2;
	y = edit_world.fY + edit_world.fH / 2;
	XUI_TEST_CHECK(edit_world.fW > 0 && edit_world.fH > 0,
		"closed window text edit arranged");
	XUI_TEST_CHECK(__xuiPixelEquals(target->pPixels, 320 * 4, x, y, 255, 255, 255, 255),
		"empty text edit missing on first visible frame");

cleanup:
	if (target != NULL) proxy.surfaceDestroy(&proxy, target);
	if (context != NULL) xuiDestroy(context);
	return !iFailed;
}

int main(void)
{
	if (!cache_transitions()) return 1;
	if (!closed_window_empty_text_edit_first_paint()) return 1;
	puts("xui_cache_transition_test: passed");
	return 0;
}
