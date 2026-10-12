#define main xui_render_schedule_reference_main
#include "xui_render_schedule_test.c"
#undef main

static int complete_count(xui_widget widget, xui_rect_t content, void* user)
{
	(void)widget;
	(void)content;
	++*(int*)user;
	return XUI_OK;
}

static xui_rect_t ime_count(xui_widget widget, void* user)
{
	++*(int*)user;
	return xuiWidgetGetWorldRect(widget);
}

int main(void)
{
	xui_proxy_t proxy = __xuiTestProxy();
	xui_context context = NULL;
	xui_widget root = NULL, first = NULL, fill = NULL, overlay = NULL;
	xui_layout_stats_t before, after;
	int iFailed = 0, i, completes = 0, ime = 0, oldCompletes, oldIme;
	XUI_TEST_CHECK(xuiCreate(&context) == XUI_OK && xuiSetProxy(context, &proxy) == XUI_OK &&
		xuiSetViewportSize(context, 64, 64) == XUI_OK, "generation context");
	XUI_TEST_CHECK(xuiWidgetCreate(context, &root) == XUI_OK &&
		xuiWidgetCreate(context, &first) == XUI_OK &&
		xuiWidgetCreate(context, &fill) == XUI_OK &&
		xuiWidgetCreate(context, &overlay) == XUI_OK, "generation widgets");
	(void)xuiSetRootWidget(context, root);
	(void)xuiWidgetSetLayoutType(root, XUI_LAYOUT_COLUMN);
	(void)xuiWidgetSetLayoutCompleteCallback(root, complete_count, &completes);
	(void)xuiWidgetSetPreferredSize(first, (xui_vec2_t){20, 12});
	(void)xuiWidgetSetSizeMode(fill, XUI_SIZE_FILL, XUI_SIZE_FILL);
	(void)xuiWidgetAddChild(root, first);
	(void)xuiWidgetAddChild(root, fill);
	(void)xuiWidgetSetFocusable(first, 1);
	(void)xuiWidgetSetImeMode(first, XUI_IME_ENABLED);
	(void)xuiWidgetSetImeCandidateRect(first, ime_count, &ime);
	(void)xuiSetFocusWidget(context, first);
	XUI_TEST_CHECK(xuiLayout(context) == XUI_OK, "generation initial layout");
	(void)xuiGetLayoutStats(context, &before);
	oldCompletes = completes;
	oldIme = ime;
	for (i = 0; i < 200; ++i) {
		XUI_TEST_CHECK(xuiHitTest(context, 1, 1, 0) == first, "stable hit");
		XUI_TEST_CHECK(xuiRenderPrepare(context) == XUI_OK && xuiBuildRenderTree(context) == XUI_OK,
			"stable render preparation");
	}
	(void)xuiGetLayoutStats(context, &after);
	XUI_TEST_CHECK(before.iGeneration == after.iGeneration && completes == oldCompletes && ime == oldIme,
		"stable hit/prepare/build started a redundant layout or IME refresh");
	(void)xuiWidgetSetPreferredSize(first, (xui_vec2_t){20, 29});
	XUI_TEST_CHECK(xuiHitTest(context, 1, 28, 0) == first, "dirty hit did not commit layout");
	(void)xuiGetLayoutStats(context, &after);
	XUI_TEST_CHECK(after.iGeneration != before.iGeneration && xuiWidgetGetRect(first).fH == 29 &&
		xuiWidgetGetRect(fill).fY == 29 && completes > oldCompletes && ime > oldIme, "dirty generation missing");
	before = after;
	XUI_TEST_CHECK(xuiRenderPrepare(context) == XUI_OK && xuiBuildRenderTree(context) == XUI_OK, "dirty render preparation");
	(void)xuiGetLayoutStats(context, &after);
	XUI_TEST_CHECK(after.iGeneration == before.iGeneration, "render did not reuse input layout commit");
	(void)xuiWidgetSetRect(overlay, (xui_rect_t){5, 7, 30, 20});
	(void)xuiOverlayAttach(context, root, overlay, XUI_LAYER_POPUP, 0);
	XUI_TEST_CHECK(xuiHitTest(context, 10, 10, 0) == overlay, "new overlay layout skipped");
	(void)xuiGetLayoutStats(context, &after);
	XUI_TEST_CHECK(after.iGeneration != before.iGeneration, "overlay generation missing");
	before = after;
	(void)xuiSetViewportSize(context, 73, 71);
	XUI_TEST_CHECK(xuiBuildRenderTree(context) == XUI_OK &&
		xuiWidgetGetRect(root).fW == 73 && xuiWidgetGetRect(root).fH == 71, "viewport layout skipped");
	(void)xuiGetLayoutStats(context, &after);
	XUI_TEST_CHECK(after.iGeneration != before.iGeneration, "viewport generation missing");
	before = after;
	(void)xuiSetVirtualDpi(context, 1.5f);
	XUI_TEST_CHECK(xuiRenderPrepare(context) == XUI_OK, "dpi layout skipped");
	(void)xuiGetLayoutStats(context, &after);
	XUI_TEST_CHECK(after.iGeneration != before.iGeneration, "dpi generation missing");
cleanup:
	if (overlay != NULL && xuiWidgetGetParent(overlay) == NULL) xuiWidgetDestroy(overlay);
	if (context != NULL) xuiDestroy(context);
	if (!iFailed) puts("xui_layout_generation_test: passed");
	return iFailed ? 1 : 0;
}
