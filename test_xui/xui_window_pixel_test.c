#include "src/xui_internal.h"
#include "xui_test_proxy.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

static int failures, checks;
#define CHECK(expr) do { ++checks; if (!(expr)) { if (failures < 12) { \
	printf("FAIL %d: %s\n", __LINE__, #expr); } ++failures; } } while (0)

static xui_rect_t pixels(float x, float y, float w, float h)
{
	xui_rect_t r;
	r.fX = (int)floorf(x + 0.5f);
	r.fY = (int)floorf(y + 0.5f);
	r.fW = (int)floorf(x + w + 0.5f) - r.fX;
	r.fH = (int)floorf(y + h + 0.5f) - r.fY;
	if (r.fW < 0) r.fW = 0;
	if (r.fH < 0) r.fH = 0;
	return r;
}

static int same(xui_rect_t a, xui_rect_t b)
{
	return a.fX == b.fX && a.fY == b.fY && a.fW == b.fW && a.fH == b.fH;
}

static void frame_geometry(void)
{
	xui_internal_window_frame_metrics_t m;
	xui_internal_window_frame_layout_t l;
	xui_rect_t r, title, client, button, bounds;
	int x, y, w, h, i, edge, metric;
	const float delta[] = {-24.25f, -1.5f, -0.6f, 0.6f, 1.5f, 24.25f};
	memset(&m, 0, sizeof(m));
	m.bShowTitleBar = 1;
	m.fBorderWidth = 1.75f;
	m.fTitleBarHeight = 23.5f;
	m.fButtonSize = 13.75f;
	m.fButtonGap = 2.25f;
	m.fButtonInset = 3.75f;
	for (metric = 0; metric < 2; ++metric) {
	m.fBorderWidth = metric ? 1.5f : 1.75f;
	for (x = -17; x <= 17; x += 17) for (y = -11; y <= 11; y += 11)
	for (w = 200; w <= 201; ++w) for (h = 140; h <= 141; ++h) {
		r = (xui_rect_t){x, y, w, h};
		xuiInternalWindowFrameLayout(r, &m, &l);
		title = pixels(x + m.fBorderWidth, y + m.fBorderWidth, w - 2 * m.fBorderWidth, 23.5f);
		client = pixels(x + m.fBorderWidth, y + m.fBorderWidth + 23.5f,
			w - 2 * m.fBorderWidth, h - 2 * m.fBorderWidth - 23.5f);
		CHECK(same(l.tTitleBarRect, title));
		CHECK(same(l.tClientRect, client));
		CHECK(l.tTitleBarRect.fY + l.tTitleBarRect.fH == l.tClientRect.fY);
		for (i = 0; i < 3; ++i) {
			button = pixels(title.fX + title.fW - 3.75f - 13.75f - i * 16.0f,
				title.fY + (title.fH - 13.75f) * 0.5f, 13.75f, 13.75f);
			CHECK(same(xuiInternalWindowFrameTrailingButton(&l, &m, i), button));
		}
	}
	}
	m.bShowTitleBar = 0;
	xuiInternalWindowFrameLayout((xui_rect_t){-3, -5, 51, 41}, &m, &l);
	CHECK(same(l.tClientRect, pixels(-1.5f, -3.5f, 48, 38)));
	CHECK(l.tTitleBarRect.fH == 0);
	m.bShowTitleBar = 1;
	xuiInternalWindowFrameLayout((xui_rect_t){-3, -5, 51, 11}, &m, &l);
	CHECK(l.tTitleBarRect.fY + l.tTitleBarRect.fH == l.tClientRect.fY);
	CHECK(l.tClientRect.fH == 0);
	bounds = (xui_rect_t){-20, -15, 260, 180};
	r = (xui_rect_t){1, 5, 101, 77};
	for (i = 0; i < 6; ++i) {
		float dx = delta[i];
		float x0 = fminf(fmaxf(r.fX + dx, bounds.fX), bounds.fX + bounds.fW - r.fW);
		float y0 = fminf(fmaxf(r.fY - dx, bounds.fY), bounds.fY + bounds.fH - r.fH);
		CHECK(same(xuiInternalWindowFrameMove(r, bounds, dx, -dx), pixels(x0, y0, 101, 77)));
		for (edge = 1; edge <= 8; edge <<= 1) {
			float left = r.fX, top = r.fY, right = r.fX + r.fW, bottom = r.fY + r.fH;
			if (edge == XUI_WINDOW_EDGE_LEFT) left += dx;
			if (edge == XUI_WINDOW_EDGE_RIGHT) right += dx;
			if (edge == XUI_WINDOW_EDGE_TOP) top += dx;
			if (edge == XUI_WINDOW_EDGE_BOTTOM) bottom += dx;
			if (left < bounds.fX) { right += bounds.fX - left; left = bounds.fX; }
			if (top < bounds.fY) { bottom += bounds.fY - top; top = bounds.fY; }
			CHECK(same(xuiInternalWindowFrameResize(r, bounds, edge, dx, dx, 30.75f, 20.75f),
				pixels(left, top, right - left, bottom - top)));
		}
	}
	CHECK(same(xuiInternalWindowFrameResize(r, bounds, XUI_WINDOW_EDGE_LEFT | XUI_WINDOW_EDGE_TOP,
		200.6f, 200.6f, 30.75f, 20.75f), pixels(71.25f, 61.25f, 30.75f, 20.75f)));
}

static xui_surface tracked;
static xui_rect_t copiedSrc, copiedDst;
static int copies;
static xui_surface_draw_to_proc originalCopy;
static int capture_copy(xui_proxy p, xui_surface target, xui_surface source,
	xui_rect_t src, xui_rect_t dst, uint32_t color, uint32_t flags)
{
	if (source == tracked) { copiedSrc = src; copiedDst = dst; ++copies; }
	return originalCopy(p, target, source, src, dst, color, flags);
}

static void window_geometry(float border)
{
	xui_test_proxy_state_t proxy;
	xui_context context = NULL;
	xui_widget root, parent, window, adorner;
	xui_surface target = NULL;
	xui_window_desc_t d;
	xui_rect_t local, world, title, client, expected, preview;
	int w, h, x, y;
	xuiTestProxyInit(&proxy);
	originalCopy = proxy.tProxy.surfaceDrawTo;
	proxy.tProxy.surfaceDrawTo = capture_copy;
	CHECK(xuiCreate(&context) == XUI_OK);
	CHECK(xuiSetProxy(context, &proxy.tProxy) == XUI_OK);
	CHECK(xuiSetViewportSize(context, 400, 300) == XUI_OK);
	CHECK(xuiWidgetCreate(context, &root) == XUI_OK);
	CHECK(xuiSetRootWidget(context, root) == XUI_OK);
	CHECK(xuiWidgetSetLayoutType(root, XUI_LAYOUT_MANUAL) == XUI_OK);
	CHECK(xuiWidgetCreate(context, &parent) == XUI_OK);
	CHECK(xuiWidgetAddChild(root, parent) == XUI_OK);
	CHECK(xuiWidgetSetLayoutType(parent, XUI_LAYOUT_MANUAL) == XUI_OK);
	CHECK(xuiWidgetSetOverflow(parent, XUI_OVERFLOW_HIDDEN) == XUI_OK);
	memset(&d, 0, sizeof(d));
	d.iSize = sizeof(d);
	d.fBorderWidth = border;
	d.fTitleBarHeight = 23.5f;
	d.fButtonSize = 13.75f;
	CHECK(xuiWindowCreate(context, &window, &d) == XUI_OK);
	CHECK(xuiOverlayDetach(window) == XUI_OK);
	CHECK(xuiWidgetAddChild(parent, window) == XUI_OK);
	CHECK(xuiTestSurfaceCreate(&proxy, &target, 400, 300, XUI_SURFACE_USAGE_TARGET) == XUI_OK);
	for (x = -13; x <= 13; x += 26) for (y = -7; y <= 7; y += 14)
	for (w = 200; w <= 201; ++w) for (h = 140; h <= 141; ++h) {
		CHECK(xuiWidgetSetRect(parent, (xui_rect_t){x, y, 360, 240}) == XUI_OK);
		CHECK(xuiWidgetSetRect(window, (xui_rect_t){5, 9, w, h}) == XUI_OK);
		CHECK(xuiLayout(context) == XUI_OK);
		title = pixels(border, border, w - 2 * border, 23.5f);
		client = pixels(border, border + 23.5f, w - 2 * border, h - 2 * border - 23.5f);
		CHECK(same(xuiWindowGetTitleBarRect(window), title));
		CHECK(same(xuiWindowGetClientRect(window), client));
		CHECK(same(xuiWidgetGetContentRect(window), client));
		CHECK(same(xuiWidgetGetRect(xuiWindowGetClientWidget(window)), client));
		world = xuiWidgetGetWorldRect(window);
		CHECK(same(world, (xui_rect_t){x + 5, y + 9, w, h}));
		CHECK(xuiHitTest(context, -1, world.fY + 30, XUI_WIDGET_HIT_DEFAULT) == NULL);
		CHECK(xuiRenderPrepare(context) == XUI_OK);
		tracked = xuiWidgetGetCacheSurface(window, xuiWidgetGetInputState(window));
		copies = 0;
		CHECK(xuiRender(context, target, NULL, 0) == XUI_OK);
		expected = world;
		if (expected.fX < 0) { expected.fW += expected.fX; expected.fX = 0; }
		CHECK(copies > 0 && same(copiedDst, expected));
		CHECK(same(copiedSrc, (xui_rect_t){expected.fX - world.fX, 0, expected.fW, expected.fH}));
	}
	CHECK(xuiWindowBringToFront(window) == XUI_OK);
	CHECK(xuiWidgetSetRect(window, (xui_rect_t){60, 50, 201, 141}) == XUI_OK);
	CHECK(xuiLayout(context) == XUI_OK);
	local = xuiWidgetGetRect(window);
	CHECK(xuiInputPointerDown(context, 100, 60, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT) == XUI_OK);
	CHECK(xuiDispatchPendingEvents(context) == XUI_OK);
	CHECK(xuiInputPointerMove(context, 113, 69, XUI_POINTER_BUTTON_LEFT) == XUI_OK);
	CHECK(xuiDispatchPendingEvents(context) == XUI_OK);
	adorner = xuiOverlayTop(context);
	CHECK(xuiOverlayGetOwner(adorner) == window);
	CHECK(xuiLayout(context) == XUI_OK);
	CHECK(context->iDragAdornerPrimitiveCount == 1);
	preview = context->arrDragAdornerPrimitives[0].tRect;
	CHECK(same(xuiWidgetGetWorldRect(adorner),
		(xui_rect_t){preview.fX - 4, preview.fY - 4, preview.fW + 8, preview.fH + 8}));
	CHECK(same(preview, (xui_rect_t){local.fX + 13, local.fY + 9, local.fW, local.fH}));
	CHECK(xuiInputPointerUp(context, 113, 69, XUI_POINTER_BUTTON_LEFT, 0) == XUI_OK);
	CHECK(xuiDispatchPendingEvents(context) == XUI_OK);
	CHECK(same(preview, xuiWidgetGetRect(window)));
	CHECK(xuiWindowSetCollapsed(window, 1) == XUI_OK);
	CHECK(xuiLayout(context) == XUI_OK);
	CHECK(xuiWidgetGetRect(window).fH == pixels(0, 0, 201, 2 * border + 23.5f).fH);
	CHECK(xuiWindowGetClientRect(window).fH == 0);
	CHECK(xuiWindowSetCollapsed(window, 0) == XUI_OK);
	CHECK(xuiLayout(context) == XUI_OK);
	CHECK(same(preview, xuiWidgetGetRect(window)));
	tracked = NULL;
	proxy.tProxy.surfaceDestroy(&proxy.tProxy, target);
	xuiDestroy(context);
}

int main(int argc, char** argv)
{
	frame_geometry();
	if (argc < 2 || strcmp(argv[1], "--frame-only") != 0) {
		window_geometry(1.75f);
		window_geometry(1.5f);
	}
	printf("xui_window_pixel_test: %d checks, %d failures\n", checks, failures);
	return failures != 0;
}
