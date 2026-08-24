#include "xge.h"
#include "xui.h"

#include <stdio.h>
#include <string.h>

#define CHECK(expr, message) \
	do { \
		if ( !(expr) ) { \
			printf("xui_pixel_geometry_test failed: %s\n", (message)); \
			failed = 1; \
			goto cleanup; \
		} \
	} while ( 0 )

static int rect_equals(xui_rect_t rect, int x, int y, int width, int height)
{
	return rect.fX == x && rect.fY == y && rect.fW == width && rect.fH == height;
}

int main(void)
{
	xui_context context = NULL;
	xui_widget root = NULL;
	xui_widget parent = NULL;
	xui_widget children[3] = { NULL, NULL, NULL };
	xui_rect_t rects[3];
	xui_rect_t world;
	xge_rect_i_t pixels;
	int failed = 0;
	int i;

	pixels = xgeRectToPixelsNearest((xge_rect_t){0.49f, -0.51f, 10.02f, 5.02f});
	CHECK(pixels.iX == 0 && pixels.iY == -1 && pixels.iW == 11 && pixels.iH == 6,
		"nearest conversion must round edges, not width independently");
	pixels = xgeRectToPixelsOutward((xge_rect_t){0.25f, 1.25f, 9.5f, 4.5f});
	CHECK(pixels.iX == 0 && pixels.iY == 1 && pixels.iW == 10 && pixels.iH == 5,
		"outward conversion");
	pixels = xgeRectToPixelsInward((xge_rect_t){0.25f, 1.25f, 9.5f, 4.5f});
	CHECK(pixels.iX == 1 && pixels.iY == 2 && pixels.iW == 8 && pixels.iH == 3,
		"inward conversion");
	CHECK(rect_equals(xuiRectFromFloatNearest(0.49f, -0.51f, 10.02f, 5.02f), 0, -1, 11, 6),
		"XUI nearest conversion must round global edges");

	CHECK(xuiCreate(&context) == XUI_OK, "create context");
	CHECK(xuiSetViewportSize(context, 115, 48) == XUI_OK, "set integer viewport");
	CHECK(xuiWidgetCreate(context, &root) == XUI_OK, "create root");
	CHECK(xuiWidgetCreate(context, &parent) == XUI_OK, "create parent");
	CHECK(xuiSetRootWidget(context, root) == XUI_OK, "set root");
	CHECK(xuiWidgetAddChild(root, parent) == XUI_OK, "attach parent");
	CHECK(xuiWidgetSetLayoutType(root, XUI_LAYOUT_MANUAL) == XUI_OK, "manual root");
	CHECK(xuiWidgetSetRect(parent, (xui_rect_t){7, 5, 101, 30}) == XUI_OK, "parent rect");
	CHECK(xuiWidgetSetLayoutType(parent, XUI_LAYOUT_ROW) == XUI_OK, "row parent");
	for ( i = 0; i < 3; ++i ) {
		CHECK(xuiWidgetCreate(context, &children[i]) == XUI_OK, "create child");
		CHECK(xuiWidgetSetSizeMode(children[i], XUI_SIZE_FILL, XUI_SIZE_FILL) == XUI_OK,
			"fill child");
		CHECK(xuiWidgetAddChild(parent, children[i]) == XUI_OK, "attach child");
	}
	CHECK(xuiLayout(context) == XUI_OK, "layout");
	for ( i = 0; i < 3; ++i ) rects[i] = xuiWidgetGetRect(children[i]);
	CHECK(rects[0].fX == 0, "first child starts at parent edge");
	CHECK(rects[0].fX + rects[0].fW == rects[1].fX, "first shared edge");
	CHECK(rects[1].fX + rects[1].fW == rects[2].fX, "second shared edge");
	CHECK(rects[2].fX + rects[2].fW == 101, "distributed widths preserve parent extent");
	world = xuiWidgetGetWorldRect(children[1]);
	CHECK(world.fX == 7 + rects[1].fX && world.fY == 5,
		"world coordinates are exact integer parent sums");
	CHECK(xuiHitTest(context, world.fX, world.fY, XUI_WIDGET_HIT_DEFAULT) == children[1],
		"left/top edge is included");
	CHECK(xuiHitTest(context, world.fX + world.fW, world.fY, XUI_WIDGET_HIT_DEFAULT) != children[1],
		"right edge is excluded");
	CHECK(rect_equals(xuiWidgetGetRect(parent), 7, 5, 101, 30), "manual parent remains exact");

cleanup:
	if ( context != NULL ) xuiDestroy(context);
	if ( !failed ) printf("xui_pixel_geometry_test passed\n");
	return failed ? 1 : 0;
}
