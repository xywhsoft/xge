#include "xui.h"
#include "xui_test_proxy.h"

#include <stddef.h>
#include <stdio.h>
#include <string.h>

#define CHECK(expr, message) \
	do { if ( !(expr) ) { printf("xui_incremental_render_test failed: %s\n", message); failed = 1; goto cleanup; } } while ( 0 )

static int test_event(xui_widget widget, const xui_event_t* event, void* user)
{
	(void)widget;
	(void)event;
	(void)user;
	return XUI_OK;
}

static int test_update(xui_widget widget, float delta, void* user)
{
	int* count = (int*)user;
	(void)widget;
	(void)delta;
	++*count;
	return XUI_OK;
}

static int debug_info(xui_widget widget, xui_debug_widget_info_t* info)
{
	memset(info, 0, sizeof(*info));
	info->iSize = sizeof(*info);
	return xuiDebugWidgetInspect(widget, info);
}

static int bytes_equal(const unsigned char* bytes, size_t begin, size_t end, unsigned char value)
{
	size_t i;
	for ( i = begin; i < end; ++i ) if ( bytes[i] != value ) return 0;
	return 1;
}

int main(void)
{
	xui_test_proxy_state_t proxy_state;
	xui_context context = NULL;
	xui_widget root = NULL;
	xui_widget branch = NULL;
	xui_widget leaf = NULL;
	xui_widget sibling = NULL;
	xui_widget overlay = NULL;
	xui_surface target = NULL;
	xui_cache_policy_t no_cache;
	xui_debug_widget_info_t info;
	xui_render_stats_t stats;
	uint32_t root_revision;
	int root_node_count;
	int update_count = 0;
	int failed = 0;

	xuiTestProxyInit(&proxy_state);
	CHECK(xuiCreate(&context) == XUI_OK, "context create");
	CHECK(xuiSetProxy(context, &proxy_state.tProxy) == XUI_OK, "proxy set");
	CHECK(xuiSetViewportSize(context, 320, 200) == XUI_OK, "viewport set");
	CHECK(xuiTestSurfaceCreate(&proxy_state, &target, 320, 200, XUI_SURFACE_USAGE_TARGET) == XUI_OK,
		"target create");
	memset(&no_cache, 0, sizeof(no_cache));
	no_cache.iSize = sizeof(no_cache);
	no_cache.iPolicy = XUI_CACHE_POLICY_NONE;

	CHECK(xuiWidgetCreate(context, &root) == XUI_OK &&
		xuiWidgetCreate(context, &branch) == XUI_OK &&
		xuiWidgetCreate(context, &leaf) == XUI_OK &&
		xuiWidgetCreate(context, &sibling) == XUI_OK, "widget create");
	CHECK(xuiWidgetSetCachePolicy(root, &no_cache) == XUI_OK &&
		xuiWidgetSetCachePolicy(branch, &no_cache) == XUI_OK &&
		xuiWidgetSetCachePolicy(leaf, &no_cache) == XUI_OK &&
		xuiWidgetSetCachePolicy(sibling, &no_cache) == XUI_OK, "cache policy");
	CHECK(xuiWidgetSetLayoutType(root, XUI_LAYOUT_MANUAL) == XUI_OK &&
		xuiWidgetSetLayoutType(branch, XUI_LAYOUT_MANUAL) == XUI_OK, "manual layout");
	CHECK(xuiWidgetAddChild(root, branch) == XUI_OK &&
		xuiWidgetAddChild(branch, leaf) == XUI_OK &&
		xuiWidgetAddChild(root, sibling) == XUI_OK, "tree attach");
	CHECK(xuiSetRootWidget(context, root) == XUI_OK, "root set");

	CHECK(xuiWidgetSetEventInterest(leaf, XUI_EVENT_MASK_POINTER_DOWN, 1) == XUI_OK,
		"leaf interest set");
	CHECK(debug_info(root, &info) == XUI_OK &&
		(info.iSubtreeEventMask & XUI_EVENT_MASK_POINTER_DOWN) != 0, "interest did not propagate");
	CHECK(xuiWidgetSetEventHandler(sibling, XUI_EVENT_KEY_DOWN, test_event, NULL) == XUI_OK,
		"sibling handler set");
	CHECK(debug_info(root, &info) == XUI_OK &&
		(info.iSubtreeEventMask & XUI_EVENT_MASK_KEY_DOWN) != 0, "handler did not propagate");
	CHECK(xuiWidgetSetEventInterest(leaf, XUI_EVENT_MASK_POINTER_DOWN, 0) == XUI_OK,
		"leaf interest clear");
	CHECK(debug_info(root, &info) == XUI_OK &&
		(info.iSubtreeEventMask & XUI_EVENT_MASK_POINTER_DOWN) == 0 &&
		(info.iSubtreeEventMask & XUI_EVENT_MASK_KEY_DOWN) != 0, "last event provider clear failed");
	CHECK(xuiWidgetRemoveFromParent(sibling) == XUI_OK, "sibling detach");
	CHECK(debug_info(root, &info) == XUI_OK &&
		(info.iSubtreeEventMask & XUI_EVENT_MASK_KEY_DOWN) == 0, "detached mask remained on root");
	CHECK(debug_info(sibling, &info) == XUI_OK &&
		(info.iSubtreeEventMask & XUI_EVENT_MASK_KEY_DOWN) != 0, "detached subtree lost its mask");
	CHECK(xuiWidgetAddChild(root, sibling) == XUI_OK, "sibling reattach");

	CHECK(xuiWidgetSetUpdateCallback(leaf, test_update, &update_count) == XUI_OK,
		"leaf update callback set");
	CHECK(xuiUpdate(context, 0.016f) == XUI_OK && update_count == 1,
		"dynamic update callback did not propagate");
	CHECK(xuiWidgetSetUpdateCallback(leaf, NULL, NULL) == XUI_OK,
		"leaf update callback clear");
	CHECK(xuiUpdate(context, 0.016f) == XUI_OK && update_count == 1,
		"cleared update callback remained scheduled");

	CHECK(xuiRender(context, target, NULL, 0) == XUI_OK, "baseline render");
	CHECK(debug_info(root, &info) == XUI_OK && info.iSubtreeDirtyFlags == 0,
		"baseline dirty aggregate");
	{
		xui_render_stats_t compat_stats;
		xui_debug_widget_info_t compat_info;
		size_t stats_prefix = offsetof(xui_render_stats_t, iPrepareVisitedWidgets);
		size_t info_prefix = offsetof(xui_debug_widget_info_t, iSubtreeDirtyFlags);
		memset(&compat_stats, 0xa5, sizeof(compat_stats));
		compat_stats.iSize = (uint32_t)stats_prefix;
		CHECK(xuiGetRenderStats(context, &compat_stats) == XUI_OK &&
			bytes_equal((const unsigned char*)&compat_stats, stats_prefix, sizeof(compat_stats), 0xa5),
			"render stats exceeded caller structure size");
		memset(&compat_info, 0xa5, sizeof(compat_info));
		compat_info.iSize = (uint32_t)info_prefix;
		CHECK(xuiDebugWidgetInspect(root, &compat_info) == XUI_OK &&
			bytes_equal((const unsigned char*)&compat_info, info_prefix, sizeof(compat_info), 0xa5),
			"debug info exceeded caller structure size");
	}
	CHECK(xuiWidgetInvalidate(leaf, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER) == XUI_OK,
		"leaf invalidate");
	CHECK(debug_info(root, &info) == XUI_OK &&
		(info.iSubtreeDirtyFlags & XUI_WIDGET_DIRTY_CACHE) != 0, "dirty bits did not propagate");
	xuiWidgetClearDirty(leaf, XUI_WIDGET_DIRTY_ALL);
	CHECK(debug_info(root, &info) == XUI_OK && info.iSubtreeDirtyFlags == 0,
		"dirty aggregate did not repair upward");

	root_node_count = xuiGetRenderNodeCount(context);
	CHECK(xuiGetRenderStats(context, &stats) == XUI_OK, "baseline stats");
	root_revision = stats.iRootStructureRevision;
	CHECK(xuiWidgetCreate(context, &overlay) == XUI_OK, "overlay create");
	CHECK(xuiWidgetSetCachePolicy(overlay, &no_cache) == XUI_OK, "overlay cache policy");
	CHECK(xuiOverlayAttach(context, NULL, overlay, XUI_LAYER_POPUP, 0) == XUI_OK,
		"overlay attach");
	CHECK(xuiRender(context, target, NULL, 0) == XUI_OK, "overlay render");
	CHECK(xuiGetRenderStats(context, &stats) == XUI_OK, "overlay stats");
	CHECK(stats.iRootStructureRevision == root_revision, "overlay changed root structure revision");
	CHECK(stats.iPrepareSkippedSubtrees > 0, "clean application root was not skipped");
	CHECK(stats.iRenderNodesReused >= root_node_count, "application render records were rebuilt");
	CHECK(xuiWidgetRemoveFromParent(overlay) == XUI_OK, "overlay detach");
	CHECK(xuiRender(context, target, NULL, 0) == XUI_OK, "overlay close render");
	CHECK(xuiGetRenderStats(context, &stats) == XUI_OK, "close stats");
	CHECK(stats.iRenderNodesReused >= root_node_count, "close rebuilt application render records");
	CHECK(xuiGetRenderNodeCount(context) == root_node_count + 1,
		"closed overlay remained in render order");
	CHECK(xuiOverlayAttach(context, NULL, overlay, XUI_LAYER_POPUP, 0) == XUI_OK,
		"overlay reattach before destroy");
	CHECK(xuiRender(context, target, NULL, 0) == XUI_OK, "reattached overlay render");
	CHECK(xuiWidgetRemoveFromParent(overlay) == XUI_OK, "overlay detach before destroy");
	xuiWidgetDestroy(overlay);
	overlay = NULL;
	CHECK(xuiRender(context, target, NULL, 0) == XUI_OK, "detached overlay destroy render");
	CHECK(xuiGetRenderNodeCount(context) == root_node_count + 1,
		"detached destroyed overlay left a stable render range");

cleanup:
	if ( overlay != NULL ) xuiWidgetDestroy(overlay);
	if ( sibling != NULL && xuiWidgetGetParent(sibling) == NULL ) xuiWidgetDestroy(sibling);
	if ( context != NULL ) xuiDestroy(context);
	if ( target != NULL ) proxy_state.tProxy.surfaceDestroy(&proxy_state.tProxy, target);
	if ( failed ) return 1;
	printf("xui_incremental_render_test passed\n");
	return 0;
}
