#include "xui.h"

#include <stdio.h>
#include <string.h>

#define CHECK(call) do { if ( (call) != XUI_OK ) goto cleanup; } while ( 0 )

static xui_thickness_t thickness(float value)
{
	return (xui_thickness_t){value, value, value, value};
}

static int rect_is(xui_widget widget, float x, float y, float w, float h)
{
	xui_rect_t rect = xuiWidgetGetRect(widget);
	return rect.fX == x && rect.fY == y && rect.fW == w && rect.fH == h;
}

static void print_rect(const char* name, xui_widget widget)
{
	xui_rect_t rect = xuiWidgetGetRect(widget);
	printf("%s=(%.0f,%.0f %.0fx%.0f)\n", name, rect.fX, rect.fY, rect.fW, rect.fH);
}

int main(void)
{
	xui_context context = NULL;
	xui_widget root = NULL;
	xui_widget header = NULL;
	xui_widget body = NULL;
	xui_widget sidebar = NULL;
	xui_widget workspace = NULL;
	xui_widget tile = NULL;
	xui_layout_stats_t stats;
	int i;
	int result = 1;

	CHECK(xuiCreate(&context));
	CHECK(xuiSetViewportSize(context, 720.0f, 420.0f));
	CHECK(xuiWidgetCreate(context, &root));
	CHECK(xuiSetRootWidget(context, root));
	CHECK(xuiWidgetSetLayoutType(root, XUI_LAYOUT_COLUMN));
	CHECK(xuiWidgetSetPadding(root, thickness(12.0f)));
	CHECK(xuiWidgetSetGap(root, 8.0f));

	CHECK(xuiWidgetCreate(context, &header));
	CHECK(xuiWidgetAddChild(root, header));
	CHECK(xuiWidgetSetLayoutType(header, XUI_LAYOUT_ROW));
	CHECK(xuiWidgetSetSizeMode(header, XUI_SIZE_FILL, XUI_SIZE_FIXED));
	CHECK(xuiWidgetSetPreferredSize(header, (xui_vec2_t){0.0f, 48.0f}));
	CHECK(xuiWidgetSetGap(header, 6.0f));

	CHECK(xuiWidgetCreate(context, &body));
	CHECK(xuiWidgetAddChild(root, body));
	CHECK(xuiWidgetSetLayoutType(body, XUI_LAYOUT_ROW));
	CHECK(xuiWidgetSetSizeMode(body, XUI_SIZE_FILL, XUI_SIZE_FILL));
	CHECK(xuiWidgetSetGap(body, 8.0f));

	CHECK(xuiWidgetCreate(context, &sidebar));
	CHECK(xuiWidgetAddChild(body, sidebar));
	CHECK(xuiWidgetSetSizeMode(sidebar, XUI_SIZE_FIXED, XUI_SIZE_FILL));
	CHECK(xuiWidgetSetPreferredSize(sidebar, (xui_vec2_t){160.0f, 0.0f}));
	CHECK(xuiWidgetSetFlex(sidebar, 0.0f, 0.0f));

	CHECK(xuiWidgetCreate(context, &workspace));
	CHECK(xuiWidgetAddChild(body, workspace));
	CHECK(xuiWidgetSetLayoutType(workspace, XUI_LAYOUT_FLOW));
	CHECK(xuiWidgetSetSizeMode(workspace, XUI_SIZE_FILL, XUI_SIZE_FILL));
	CHECK(xuiWidgetSetPadding(workspace, thickness(10.0f)));
	CHECK(xuiWidgetSetGap(workspace, 6.0f));

	for ( i = 0; i < 7; i++ ) {
		CHECK(xuiWidgetCreate(context, &tile));
		CHECK(xuiWidgetAddChild(workspace, tile));
		CHECK(xuiWidgetSetFlowMode(tile, XUI_FLOW_INLINE));
		CHECK(xuiWidgetSetSizeMode(tile, XUI_SIZE_FIXED, XUI_SIZE_FIXED));
		CHECK(xuiWidgetSetPreferredSize(tile, (xui_vec2_t){120.0f, 44.0f}));
	}

	CHECK(xuiLayout(context));
	memset(&stats, 0, sizeof(stats));
	stats.iSize = sizeof(stats);
	CHECK(xuiGetLayoutStats(context, &stats));
	printf("layout: passes=%d stable=%d\n", stats.iPassCount, stats.bStabilized);
	print_rect("root", root);
	print_rect("header", header);
	print_rect("body", body);
	print_rect("sidebar", sidebar);
	print_rect("workspace", workspace);
	if ( !stats.bStabilized || stats.iPassCount != 1 ) goto cleanup;
	if ( !rect_is(root, 0.0f, 0.0f, 720.0f, 420.0f) ) goto cleanup;
	if ( !rect_is(header, 12.0f, 12.0f, 696.0f, 48.0f) ) goto cleanup;
	if ( !rect_is(body, 12.0f, 68.0f, 696.0f, 340.0f) ) goto cleanup;
	if ( !rect_is(sidebar, 0.0f, 0.0f, 160.0f, 340.0f) ) goto cleanup;
	if ( !rect_is(workspace, 168.0f, 0.0f, 528.0f, 340.0f) ) goto cleanup;

	CHECK(xuiLayout(context));
	CHECK(xuiGetLayoutStats(context, &stats));
	if ( !stats.bStabilized || stats.iPassCount != 0 ) goto cleanup;

	printf("xui_layout example passed: first_passes=1 cached_passes=0\n");
	result = 0;

cleanup:
	if ( context != NULL ) xuiDestroy(context);
	return result;
}
