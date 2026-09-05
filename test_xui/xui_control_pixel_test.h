#ifndef XUI_CONTROL_PIXEL_TEST_H
#define XUI_CONTROL_PIXEL_TEST_H

#include "xui.h"
#include "xui_test_proxy.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

static int g_checks;
static int g_failures;

#define PIXEL_CHECK(expr) do { \
	g_checks++; \
	if (!(expr)) { \
		if (g_failures < 16) printf("line %d: %s\n", __LINE__, #expr); \
		g_failures++; \
	} \
} while (0)

typedef struct pixel_fixture_t {
	xui_test_proxy_state_t proxy;
	xui_context context;
	xui_widget root;
	xui_font font;
	xui_surface target;
} pixel_fixture_t;

/* Independent oracle: round edges, never round an extent on its own. */
static void pixel_rect(xui_rect_t actual, float x, float y, float w, float h)
{
	int left = (int)floorf(x + 0.5f);
	int top = (int)floorf(y + 0.5f);
	int right = (int)floorf(x + w + 0.5f);
	int bottom = (int)floorf(y + h + 0.5f);
	int width = right > left ? right - left : 0;
	int height = bottom > top ? bottom - top : 0;
	g_checks++;
	if (actual.fX != left || actual.fY != top || actual.fW != width || actual.fH != height) {
		if (g_failures < 16) printf("rect (%g,%g,%g,%g): got (%d,%d,%d,%d), expected (%d,%d,%d,%d)\n",
			x, y, w, h, actual.fX, actual.fY, actual.fW, actual.fH, left, top, width, height);
		g_failures++;
	}
}

static int pixel_init(pixel_fixture_t* f)
{
	memset(f, 0, sizeof(*f));
	xuiTestProxyInit(&f->proxy);
	if (xuiCreate(&f->context) != XUI_OK || xuiSetProxy(f->context, &f->proxy.tProxy) != XUI_OK) return 0;
	if (xuiInputViewport(f->context, 640, 480) != XUI_OK) return 0;
	if (f->proxy.tProxy.fontLoadMemory(&f->proxy.tProxy, &f->font, "pixel", 5, 16.0f, XUI_FONT_FORMAT_TTF) != XUI_OK) return 0;
	if (xuiSetDefaultFont(f->context, f->font) != XUI_OK) return 0;
	if (xuiTestSurfaceCreate(&f->proxy, &f->target, 640, 480, 0) != XUI_OK) return 0;
	if (xuiWidgetCreate(f->context, &f->root) != XUI_OK) return 0;
	if (xuiWidgetSetRect(f->root, (xui_rect_t){0, 0, 640, 480}) != XUI_OK) return 0;
	return xuiSetRootWidget(f->context, f->root) == XUI_OK;
}

/* Exercise the installed paint callback even for zero-size boundary cases. */
static void pixel_paint(pixel_fixture_t* f, xui_widget widget, uint32_t state)
{
	xui_widget_cache_render_proc render = NULL;
	xui_draw_context draw = NULL;
	void* user = NULL;
	PIXEL_CHECK(xuiWidgetGetCacheRenderCallback(widget, &render, &user) == XUI_OK);
	PIXEL_CHECK(render != NULL);
	PIXEL_CHECK(f->proxy.tProxy.drawBegin(&f->proxy.tProxy, &draw, f->target) == XUI_OK);
	if (render && draw) PIXEL_CHECK(render(widget, draw, state, user) == XUI_OK);
	if (draw) PIXEL_CHECK(f->proxy.tProxy.drawEnd(&f->proxy.tProxy, draw) == XUI_OK);
}

static void pixel_cleanup(pixel_fixture_t* f)
{
	if (f->context) xuiDestroy(f->context);
	if (f->target) f->proxy.tProxy.surfaceDestroy(&f->proxy.tProxy, f->target);
	if (f->font) f->proxy.tProxy.fontDestroy(&f->proxy.tProxy, f->font);
}

static int pixel_result(const char* name)
{
	printf("%s: %d checks, %d failures\n", name, g_checks, g_failures);
	return g_failures ? 1 : 0;
}

#endif
