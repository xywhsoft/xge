#define main xui_render_schedule_reference_main
#include "xui_render_schedule_test.c"
#undef main
#include <limits.h>

static int overlay_stacking(void)
{
	xui_proxy_t proxy = __xuiTestProxy();
	xui_context context = NULL;
	xui_widget root = NULL, child = NULL, widgets[4] = {0};
	xui_surface target = NULL;
	xui_surface_desc_t desc = {0};
	uint32_t colors[4] = {XUI_COLOR_RGBA(255, 0, 0, 255), XUI_COLOR_RGBA(0, 0, 255, 255),
		XUI_COLOR_RGBA(0, 255, 0, 255), XUI_COLOR_RGBA(255, 255, 0, 255)};
	int iFailed = 0, i, layer, z;
	XUI_TEST_CHECK(xuiCreate(&context) == XUI_OK &&
		xuiSetProxy(context, &proxy) == XUI_OK &&
		xuiSetViewportSize(context, 64, 64) == XUI_OK, "overlay context");
	XUI_TEST_CHECK(xuiWidgetCreate(context, &root) == XUI_OK &&
		xuiSetRootWidget(context, root) == XUI_OK, "overlay root");
	for (i = 0; i < 4; ++i) {
		XUI_TEST_CHECK(xuiWidgetCreate(context, &widgets[i]) == XUI_OK, "overlay widget");
		(void)xuiWidgetSetRect(widgets[i], (xui_rect_t){7, 9, 40, 40});
		(void)xuiWidgetSetCacheRenderCallback(widgets[i], __xuiTestSolidDraw, &colors[i]);
	}
	XUI_TEST_CHECK(xuiWidgetCreate(context, &child) == XUI_OK, "overlay child");
	(void)xuiWidgetSetRect(child, (xui_rect_t){0, 0, 2, 2});
	(void)xuiWidgetSetLayer(child, XUI_LAYER_NORMAL, 42);
	(void)xuiWidgetAddChild(widgets[0], child);
	(void)xuiOverlayAttach(context, root, widgets[2], XUI_LAYER_FLOATING, 100);
	(void)xuiOverlayAttach(context, root, widgets[0], XUI_LAYER_FLOATING, 0);
	(void)xuiOverlayAttach(context, root, widgets[3], XUI_LAYER_POPUP, -10);
	(void)xuiOverlayAttach(context, root, widgets[1], XUI_LAYER_FLOATING, 0);
	XUI_TEST_CHECK(xuiOverlayTop(context) == widgets[3], "overlay top ignored layer/z order");
	(void)xuiWidgetGetLayer(child, &layer, &z);
	XUI_TEST_CHECK(layer == XUI_LAYER_NORMAL && z == 42, "attach overwrote child stacking context");
	XUI_TEST_CHECK(xuiHitTest(context, 20, 20, 0) == widgets[3], "popup hit disagreed with top");
	(void)xuiWidgetSetVisible(widgets[3], 0);
	(void)xuiOverlayBringToFront(widgets[1]);
	XUI_TEST_CHECK(xuiOverlayTop(context) == widgets[2] &&
		xuiHitTest(context, 20, 20, 0) == widgets[2], "bring-to-front crossed explicit z group");
	(void)xuiWidgetSetVisible(widgets[2], 0);
	desc.iWidth = desc.iHeight = 64;
	desc.iKind = XUI_SURFACE_KIND_TEXTURE;
	desc.iFormat = XUI_SURFACE_FORMAT_RGBA8;
	desc.iFlags = XUI_SURFACE_USAGE_TARGET | XUI_SURFACE_ALPHA_PREMULTIPLIED;
	XUI_TEST_CHECK(proxy.surfaceCreate(&proxy, &target, &desc) == XUI_OK, "overlay target");
	for (i = 0; i < 256; ++i) {
		int index = i & 1;
		XUI_TEST_CHECK(xuiOverlayBringToFront(widgets[index]) == XUI_OK, "overlay front");
		(void)xuiWidgetGetLayer(widgets[index], &layer, &z);
		XUI_TEST_CHECK(layer == XUI_LAYER_FLOATING && z == 0, "bring-to-front accumulated z");
		(void)xuiWidgetGetLayer(child, &layer, &z);
		XUI_TEST_CHECK(layer == XUI_LAYER_NORMAL && z == 42, "bring-to-front changed descendants");
		XUI_TEST_CHECK(xuiOverlayTop(context) == widgets[index] &&
			xuiHitTest(context, 20, 20, 0) == widgets[index], "equal-z front hit disagreed with top");
		XUI_TEST_CHECK(xuiRender(context, target, NULL, 0) == XUI_OK &&
			__xuiPixelEquals(target->pPixels, 256, 20, 20, index ? 0 : 255, 0, index ? 255 : 0, 255),
			"equal-z front pixel disagreed with hit");
	}
	(void)xuiWidgetSetLayer(widgets[0], XUI_LAYER_FLOATING, INT_MAX);
	(void)xuiOverlayBringToFront(widgets[0]);
	(void)xuiWidgetGetLayer(widgets[0], &layer, &z);
	XUI_TEST_CHECK(z == INT_MAX && xuiOverlayTop(context) == widgets[0], "front overflowed z index");
	(void)xuiWidgetSetVisible(widgets[0], 0);
	(void)xuiWidgetSetVisible(widgets[1], 0);
	XUI_TEST_CHECK(xuiOverlayTop(context) == NULL, "hidden overlay reported as top");
cleanup:
	if (target != NULL) proxy.surfaceDestroy(&proxy, target);
	if (context != NULL) xuiDestroy(context);
	return !iFailed;
}

int main(void)
{
	if (!overlay_stacking()) return 1;
	puts("xui_overlay_stacking_test: passed");
	return 0;
}
