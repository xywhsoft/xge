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

enum action { NONE, UNBIND_PAINTER, REPLACE_ON_HOLD, CLOSE_ON_ACTIVATE };
typedef struct callback_state {
	xui_widget widget;
	size_t references;
	int paints;
	int activations;
	enum action action;
} callback_state;

static bool hold(const void* identity)
{
	callback_state* state = (callback_state*)identity;
	++state->references;
	if ( state->action == REPLACE_ON_HOLD ) {
		state->action = NONE;
		assert(xuiListViewSetItems(state->widget, NULL, 0) == XUI_OK);
	}
	return true;
}

static bool drop(const void* identity)
{
	callback_state* state = (callback_state*)identity;
	assert(state->references > 1);
	--state->references;
	return true;
}

static bool count(const void* identity, size_t* output)
{
	*output = ((const callback_state*)identity)->references;
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

static int paint(xui_widget widget, int index, xui_draw_context draw, xui_rect_t rect,
	int flags, void* user)
{
	callback_state* state = user;
	(void)flags;
	assert(widget == state->widget && state->references == 4);
	assert(draw && index >= 0 && index < 2 && rect.fW > 0 && rect.fH > 0);
	++state->paints;
	if ( state->action == UNBIND_PAINTER ) {
		assert(xuiListViewSetItemRenderer(widget, NULL, NULL) == XUI_OK);
		assert(state->references == 3);
	}
	return 0;
}

static void activate(xui_widget widget, int index, int button, void* user)
{
	callback_state* state = user;
	assert(widget == state->widget && state->references == 4);
	assert(index == 0 && button == XUI_POINTER_BUTTON_LEFT);
	++state->activations;
	if ( state->action == CLOSE_ON_ACTIVATE ) {
		assert(xuiWidgetDestroyChecked(widget) == XUI_OK);
		assert(state->references >= 2);
	}
}

static void verify(enum action action)
{
	const xui_rect_i_t dirty = {0, 0, 400, 200};
	const xui_rect_t bounds = {0, 0, 400, 200};
	const char* items[] = {"first", "second"};
	xui_test_proxy_state_t mock;
	xui_context context = NULL;
	xui_surface target = NULL;
	callback_state state = {.references = 1};
	const xui_callback_owner_t owner = {{&state, &operations}, hold, drop};
	xuiTestProxyInit(&mock);
	assert(xuiTestSurfaceCreate(&mock, &target, dirty.iW, dirty.iH, XUI_SURFACE_USAGE_TARGET) == XUI_OK);
	assert(xuiCreate(&context) == XUI_OK && xuiSetProxy(context, &mock.tProxy) == XUI_OK);
	assert(xuiInputViewport(context, dirty.iW, dirty.iH) == XUI_OK);
	assert(xuiListViewCreate(context, &state.widget, NULL) == XUI_OK);
	assert(xuiSetRootWidget(context, state.widget) == XUI_OK);
	assert(xuiWidgetSetRect(state.widget, bounds) == XUI_OK);
	assert(xuiListViewSetItems(state.widget, items, 2) == XUI_OK);
	assert(xuiListViewSetItemRendererOwned(state.widget, paint, &state, &owner) == XUI_OK);
	assert(xuiListViewSetActivateOwned(state.widget, activate, &state, &owner) == XUI_OK);
	assert(state.references == 3 && xuiLayout(context) == XUI_OK);

	if ( action == UNBIND_PAINTER ) state.action = action;
	assert(xuiRender(context, target, &dirty, 1) == XUI_OK);
	assert(state.paints == (action == UNBIND_PAINTER ? 1 : 2));
	if ( action == UNBIND_PAINTER ) {
		assert(state.references == 2);
	} else {
		assert(xuiListViewSetSelected(state.widget, 0) == XUI_OK);
		assert(xuiSetFocusWidget(context, state.widget) == XUI_OK);
		state.action = action;
		assert(xuiInputKeyDown(context, XUI_KEY_ENTER, 0) == XUI_OK);
		assert(xuiDispatchPendingEvents(context) == XUI_OK);
		assert(state.activations == (action == REPLACE_ON_HOLD ? 0 : 1));
		if ( action == REPLACE_ON_HOLD ) assert(xuiListViewGetItemCount(state.widget) == 0);
	}
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
	verify(NONE);
	verify(UNBIND_PAINTER);
	verify(REPLACE_ON_HOLD);
	verify(CLOSE_ON_ACTIVATE);
	puts("ListView callback ownership: 4 passed");
	return 0;
}
