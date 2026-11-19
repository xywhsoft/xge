#ifdef XGE_XRT_HOST
#define XRT_MODULE_ALL
#ifdef _WIN32
#define XRT_USE_SHARED
#endif
#endif
#include "xui.h"
#include "xui_test_proxy.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

#ifdef XGE_XRT_HOST
#include <xge_host_xrt_contract.h>

static xge_host_xrt_proc resolve_xrt(void* user, const char* name)
{
	(void)user;
#define XGE_HOST_XRT_SLOT(symbol) if (strcmp(name, #symbol) == 0) return (xge_host_xrt_proc)&symbol;
#include <xge_host_xrt_slots.inc>
#undef XGE_HOST_XRT_SLOT
	return NULL;
}
#endif

enum action { NO_CHANGE, CLEAR_ON_HOLD, CLEAR_IN_ITEM, UNBIND_ICON, CLOSE_IN_ICON };

typedef struct painter_state {
	xui_widget tree;
	size_t references;
	int items;
	int icons;
	enum action action;
} painter_state;

static bool hold(const void* identity)
{
	painter_state* state = (painter_state*)identity;
	++state->references;
	if ( state->action == CLEAR_ON_HOLD ) {
		state->action = NO_CHANGE;
		assert(xuiTreeViewClear(state->tree) == XUI_OK);
	}
	return true;
}

static bool drop(const void* identity)
{
	painter_state* state = (painter_state*)identity;
	assert(state->references > 1);
	--state->references;
	return true;
}

static bool count(const void* identity, size_t* output)
{
	*output = ((const painter_state*)identity)->references;
	return true;
}

static bool trace(const void* identity, xrtownershipvisitor visitor, void* user)
{
	(void)identity;
	(void)visitor;
	(void)user;
	return true;
}

static const xrtownershipops operations = {count, trace};
static const xui_tree_view_node_t node = {
	.iId = 10, .iParent = -1, .sText = "original row", .bEnabled = 1, .bIconReserved = 1
};

static int paint_item(xui_widget tree, int id, int row, const xui_tree_view_node_t* snapshot,
	xui_draw_context draw, xui_rect_t rect, int flags, void* user)
{
	painter_state* state = user;
	(void)flags;
	assert(tree == state->tree && state->references == 4);
	assert(draw && rect.fW > 0 && id == node.iId && row == 0);
	++state->items;
	if ( state->action == CLEAR_IN_ITEM ) {
		assert(xuiTreeViewClear(tree) == XUI_OK);
		assert(xuiTreeViewGetNodeCount(tree) == 0);
	}
	assert(strcmp(snapshot->sText, node.sText) == 0);
	return 0;
}

static int paint_icon(xui_widget tree, int id, xui_draw_context draw, xui_rect_t rect,
	int flags, void* user)
{
	painter_state* state = user;
	(void)flags;
	assert(tree == state->tree && state->references == 4);
	assert(draw && id == node.iId && rect.fW > 0 && rect.fH > 0);
	++state->icons;
	if ( state->action == UNBIND_ICON ) {
		assert(xuiTreeViewSetIconRenderer(tree, NULL, NULL) == XUI_OK);
		assert(state->references == 3);
	}
	if ( state->action == CLOSE_IN_ICON ) {
		assert(xuiWidgetDestroyChecked(tree) == XUI_OK);
		assert(state->references >= 2);
	}
	return 1;
}

static void verify(enum action action, int items, int icons)
{
	const xui_rect_i_t dirty = {0, 0, 400, 200};
	const xui_rect_t bounds = {0, 0, 400, 200};
	xui_test_proxy_state_t mock;
	xui_context context = NULL;
	xui_surface target = NULL;
	painter_state state = {.references = 1};
	const xui_callback_owner_t owner = {{&state, &operations}, hold, drop};
	xuiTestProxyInit(&mock);
	assert(xuiTestSurfaceCreate(&mock, &target, dirty.iW, dirty.iH, XUI_SURFACE_USAGE_TARGET) == XUI_OK);
	assert(xuiCreate(&context) == XUI_OK && xuiSetProxy(context, &mock.tProxy) == XUI_OK);
	assert(xuiInputViewport(context, dirty.iW, dirty.iH) == XUI_OK);
	assert(xuiTreeViewCreate(context, &state.tree, NULL) == XUI_OK);
	assert(xuiSetRootWidget(context, state.tree) == XUI_OK);
	assert(xuiWidgetSetRect(state.tree, bounds) == XUI_OK);
	assert(xuiTreeViewSetNodes(state.tree, &node, 1) == XUI_OK);
	assert(xuiTreeViewSetItemRendererOwned(state.tree, paint_item, &state, &owner) == XUI_OK);
	assert(xuiTreeViewSetIconRendererOwned(state.tree, paint_icon, &state, &owner) == XUI_OK);
	assert(state.references == 3);
	assert(xuiLayout(context) == XUI_OK);
	state.action = action;
	assert(xuiRender(context, target, &dirty, 1) == XUI_OK);
	assert(state.items == items && state.icons == icons);
	if ( action == CLEAR_ON_HOLD || action == CLEAR_IN_ITEM )
		assert(xuiTreeViewGetNodeCount(state.tree) == 0);
	if ( action == UNBIND_ICON ) assert(state.references == 2);
	xuiDestroy(context);
	assert(state.references == 1);
	mock.tProxy.surfaceDestroy(&mock.tProxy, target);
}

int main(void)
{
#ifdef XGE_XRT_HOST
	const xge_host_xrt_contract_t contract = XGE_HOST_XRT_LOCAL_CONTRACT_INIT;
	assert(xgeHostXrtBind(&contract, resolve_xrt, NULL, NULL) == XGE_HOST_XRT_OK);
#endif
	verify(NO_CHANGE, 1, 1);
	verify(CLEAR_ON_HOLD, 0, 0);
	verify(CLEAR_IN_ITEM, 1, 0);
	verify(UNBIND_ICON, 1, 1);
	verify(CLOSE_IN_ICON, 1, 1);
	puts("TreeView painter ownership: 5 passed");
	return 0;
}
