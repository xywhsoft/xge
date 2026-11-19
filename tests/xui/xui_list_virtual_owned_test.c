#define XRT_MODULE_ALL
#if defined(XGE_XRT_HOST) && defined(_WIN32)
#define XRT_USE_SHARED
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

enum action { NORMAL, REPLACE, CLOSE, RECURSE, ERROR_ON_DROP };
typedef struct provider_state {
	size_t references;
	int calls;
	int max_index;
	enum action action;
	const char* returned;
} provider_state;

static bool hold(const void* identity)
{
	++((provider_state*)identity)->references;
	return true;
}

static bool drop(const void* identity)
{
	provider_state* state = (provider_state*)identity;
	assert(state->references > 1);
	--state->references;
	if ( state->action == ERROR_ON_DROP ) xrtSetErrorKind(XERR_VALUE);
	return true;
}

static bool count(const void* identity, size_t* output)
{
	*output = ((const provider_state*)identity)->references;
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

static xstrview get_label(xui_widget widget, int index, void* user)
{
	provider_state* state = user;
	static const char label[] = "a\0\xE5\x90\x8D";
	assert(state->references == 3 && index >= 0 && index < 1000000);
	++state->calls;
	if ( index > state->max_index ) state->max_index = index;
	if ( state->action == REPLACE ) {
		const char* items[] = {"replacement"};
		assert(xuiListViewSetItems(widget, items, 1) == XUI_OK);
		assert(state->references == 2);
	}
	if ( state->action == CLOSE ) assert(xuiWidgetDestroyChecked(widget) == XUI_OK);
	if ( state->action == RECURSE ) {
		char* text = (char*)1;
		size_t size = 99;
		assert(xuiListViewGetItemTextOwned(widget, 0, &text, &size) == XUI_ERROR_BUSY);
		assert(text == NULL && size == 0);
		return (xstrview){0};
	}
	if ( index == 7 ) return (xstrview){0};
	state->returned = xrtMalloc(sizeof(label));
	assert(state->returned != NULL);
	memcpy((void*)state->returned, label, sizeof(label));
	return (xstrview){state->returned, sizeof(label)-1};
}

static const char* read_raw(xui_widget widget, int index, void* user)
{
	(void)widget;
	(void)index;
	(void)user;
	return "raw provider";
}

static void verify(enum action action)
{
	const xui_rect_i_t viewport = {0, 0, 400, 200};
	xui_test_proxy_state_t mock;
	xui_context context = NULL;
	xui_widget list = NULL;
	xui_surface surface = NULL;
	xui_font font = NULL;
	provider_state state = {.references=1, .max_index=-1};
	const xui_callback_owner_t owner = {{&state, &operations}, hold, drop};
	xuiTestProxyInit(&mock);
	assert(xuiCreate(&context) == XUI_OK && xuiSetProxy(context, &mock.tProxy) == XUI_OK);
	assert(xuiInputViewport(context, viewport.iW, viewport.iH) == XUI_OK);
	assert(mock.tProxy.fontLoadFile(&mock.tProxy, &font, "test-font", 16, 0) == XUI_OK);
	xui_list_view_desc_t desc = {.iSize=sizeof(desc), .pFont=font, .iSelected=-1};
	assert(xuiListViewCreate(context, &list, &desc) == XUI_OK);
	assert(xuiSetRootWidget(context, list) == XUI_OK);
	assert(xuiWidgetSetRect(list, (xui_rect_t){0,0,400,200}) == XUI_OK);
	assert(xuiTestSurfaceCreate(&mock, &surface, 400, 200, XUI_SURFACE_USAGE_TARGET) == XUI_OK);

	if ( action == NORMAL ) {
		assert(xuiListViewSetVirtualItems(list, 1000000, read_raw, NULL) == XUI_OK);
		assert(xuiListViewSetSelected(list, 999999) == XUI_OK);
		assert(xrtMemDebugFailAfter(0));
		int failed = xuiListViewSetVirtualItemsOwned(list, 1, get_label, &state, &owner);
		assert(xrtMemDebugFailTriggered());
		xrtMemDebugFailClear();
		xrtClearError();
		assert(failed == XUI_ERROR_OUT_OF_MEMORY && state.references == 1 && state.calls == 0);
		assert(xuiListViewGetItemCount(list) == 1000000 && xuiListViewGetSelected(list) == 999999);
		assert(strcmp(xuiListViewGetItemText(list, 0), "raw provider") == 0);
		assert(xuiListViewSetSelected(list, -1) == XUI_OK);
		assert(xuiListViewSetScroll(list, 0) == XUI_OK);
	}
	assert(xuiListViewSetVirtualItemsOwned(list, 1000000, get_label, &state, &owner) == XUI_OK);
	assert(state.references == 2 && state.calls == 0 && xuiListViewGetItemCount(list) == 1000000);
	assert(xuiLayout(context) == XUI_OK);
	assert(xuiRender(context, surface, &viewport, 1) == XUI_OK);
	assert(state.calls > 0 && state.calls < 20 && state.max_index < 20 && state.references == 2);

	char* text = NULL;
	size_t size = 0;
	state.action = action;
	int result = xuiListViewGetItemTextOwned(list, 999999, &text, &size);
	if ( action == NORMAL ) {
		assert(result == XUI_OK && text == state.returned && size == 5);
		assert(memcmp(text, "a\0\xE5\x90\x8D", size+1) == 0);
		xrtFree(text);
		assert(xuiListViewGetItemTextOwned(list, 7, &text, &size) == XUI_OK && size == 0);
		xrtFree(text);
		assert(strcmp(xuiListViewGetItemText(list, 0), "a") == 0);
		xui_accessible_node_t node = {.iSize=sizeof(node)};
		assert(xuiWidgetGetAccessibleNode(list, 1, &node) == XUI_OK && strcmp(node.sName, "a") == 0);
		assert(xuiWidgetGetAccessibleNode(list, 8, &node) == XUI_OK && strcmp(node.sName, "Item 8") == 0);
		assert(xuiListViewGetItemTextOwned(list, -1, &text, &size) == XUI_ERROR_INVALID_ARGUMENT);
		assert(text == NULL && size == 0);
		assert(xuiListViewSetVirtualItems(list, 1, read_raw, NULL) == XUI_OK);
		assert(state.references == 1 && strcmp(xuiListViewGetItemText(list, 0), "raw provider") == 0);
	} else if ( action == REPLACE || action == CLOSE ) {
		assert(result == XUI_ERROR_INVALID_STATE && text == NULL && size == 0 && state.references == 1);
		if ( action == REPLACE ) assert(strcmp(xuiListViewGetItemText(list, 0), "replacement") == 0);
	} else if ( action == RECURSE ) {
		assert(result == XUI_OK && text != NULL && size == 0 && text[0] == '\0');
		xrtFree(text);
	} else {
		assert(result == XUI_ERROR_CALLBACK_FAILED && text == NULL && size == 0);
		assert(xrtGetError() != NULL);
		xrtClearError();
	}
	state.action = NORMAL;
	xuiDestroy(context);
	assert(state.references == 1);
	mock.tProxy.fontDestroy(&mock.tProxy, font);
	mock.tProxy.surfaceDestroy(&mock.tProxy, surface);
}

int main(void)
{
#ifdef XGE_XRT_HOST
	const xge_host_xrt_contract_t contract = XGE_HOST_XRT_LOCAL_CONTRACT_INIT;
	assert(xgeHostXrtBind(&contract, resolve_xrt, NULL, NULL) == XGE_HOST_XRT_OK);
#endif
	assert(xrtMemDebugEnabled() || xrtMemDebugEnable(true));
	verify(NORMAL);
	verify(REPLACE);
	verify(CLOSE);
	verify(RECURSE);
	verify(ERROR_ON_DROP);
	puts("ListView owned virtual source: 5 passed");
	return 0;
}
