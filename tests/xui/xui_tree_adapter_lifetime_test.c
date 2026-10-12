#define main xui_tree_reference_main
#include "xui_tree_view_test.c"
#undef main

enum { LIVE, REPLACE, CLEAR, RECURSE, REBIND, DESTROY_TREE, DESTROY_PARENT, DESTROY_CONTEXT };
typedef struct adapter_case_t {
	xui_context context;
	xui_widget parent;
	int mode, atNode, done, nodeCalls, recursiveResult, liveMismatch;
	int treeDestroyed, contextDestroyed;
} adapter_case_t;

static int adapter_count(xui_widget tree, void* user);
static int adapter_node(xui_widget tree, int index, xui_tree_view_node_t* node, void* user);

static void mutate(xui_widget tree, adapter_case_t* state)
{
	xui_tree_view_node_t node = __xuiTreeViewTestNode(700, -1);
	if (state->done) return;
	state->done = 1;
	switch (state->mode) {
	case REPLACE: (void)xuiTreeViewSetNodes(tree, &node, 1); break;
	case CLEAR: (void)xuiTreeViewClear(tree); break;
	case RECURSE: state->recursiveResult = xuiTreeViewRefreshAdapter(tree); break;
	case REBIND: state->recursiveResult = xuiTreeViewSetAdapter(tree, NULL, NULL, NULL); break;
	case DESTROY_TREE:
		state->treeDestroyed = 1;
		xuiWidgetDestroy(tree);
		break;
	case DESTROY_PARENT:
		state->treeDestroyed = 1;
		xuiWidgetDestroy(state->parent);
		state->parent = NULL;
		break;
	case DESTROY_CONTEXT:
		state->treeDestroyed = state->contextDestroyed = 1;
		xuiDestroy(state->context);
		break;
	default: break;
	}
}

static int adapter_count(xui_widget tree, void* user)
{
	adapter_case_t* state = (adapter_case_t*)user;
	if (!state->atNode) mutate(tree, state);
	return 2;
}

static int adapter_node(xui_widget tree, int index, xui_tree_view_node_t* node, void* user)
{
	adapter_case_t* state = (adapter_case_t*)user;
	++state->nodeCalls;
	if (state->mode == LIVE && (xuiTreeViewGetNodeCount(tree) != 1 ||
		xuiTreeViewGetNodeById(tree, 50) == NULL)) ++state->liveMismatch;
	if (state->atNode) mutate(tree, state);
	*node = __xuiTreeViewTestNode(100 + index, -1);
	return XUI_OK;
}

static int run_case(int mode, int atNode)
{
	xui_test_proxy_state_t proxy;
	adapter_case_t state = {0};
	xui_widget tree = NULL;
	xui_tree_view_node_t old = __xuiTreeViewTestNode(50, -1);
	int iFailed = 0, result;
	xuiTestProxyInit(&proxy);
	state.mode = mode;
	state.atNode = atNode;
	XUI_TEST_CHECK(xuiCreate(&state.context) == XUI_OK &&
		xuiSetProxy(state.context, &proxy.tProxy) == XUI_OK &&
		xuiWidgetCreate(state.context, &state.parent) == XUI_OK &&
		xuiTreeViewCreate(state.context, &tree, NULL) == XUI_OK, "adapter fixture");
	(void)xuiSetRootWidget(state.context, state.parent);
	(void)xuiWidgetAddChild(state.parent, tree);
	XUI_TEST_CHECK(xuiTreeViewSetNodes(tree, &old, 1) == XUI_OK, "adapter seed");
	result = xuiTreeViewSetAdapter(tree, adapter_count, adapter_node, &state);
	if (mode == LIVE || mode == RECURSE || mode == REBIND) {
		XUI_TEST_CHECK(result == XUI_OK && xuiTreeViewGetNodeCount(tree) == 2 &&
			xuiTreeViewGetNodeById(tree, 100) != NULL && xuiTreeViewGetNodeById(tree, 101) != NULL,
			"adapter final transaction");
		XUI_TEST_CHECK(state.liveMismatch == 0, "onNode observed a partially replaced live tree");
		if (mode != LIVE) XUI_TEST_CHECK(state.recursiveResult == XUI_ERROR_INVALID_STATE,
			"recursive adapter refresh/rebind was not rejected");
	} else {
		XUI_TEST_CHECK(result == XUI_ERROR_INVALID_STATE, "invalidated adapter transaction committed");
		XUI_TEST_CHECK(state.nodeCalls == atNode, "adapter continued callbacks after invalidation");
		if (mode == REPLACE) XUI_TEST_CHECK(xuiTreeViewGetNodeCount(tree) == 1 &&
			xuiTreeViewGetNodeById(tree, 700) != NULL, "adapter overwrote callback replacement");
		if (mode == CLEAR) XUI_TEST_CHECK(xuiTreeViewGetNodeCount(tree) == 0, "adapter overwrote callback clear");
	}
cleanup:
	if (state.context != NULL && !state.contextDestroyed) xuiDestroy(state.context);
	if (!iFailed) printf("adapter case mode=%d atNode=%d passed\n", mode, atNode);
	return !iFailed;
}

int main(void)
{
	int mode, atNode;
	for (mode = LIVE; mode <= DESTROY_CONTEXT; ++mode) {
		for (atNode = 0; atNode < 2; ++atNode) {
			if (!run_case(mode, atNode)) return 1;
		}
	}
	puts("xui_tree_adapter_lifetime_test: 16 cases passed");
	return 0;
}
