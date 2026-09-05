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

int main(void)
{
	if (!cache_transitions()) return 1;
	puts("xui_cache_transition_test: passed");
	return 0;
}
