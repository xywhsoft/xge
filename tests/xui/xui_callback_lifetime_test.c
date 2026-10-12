#include "xui.h"

#include <stdio.h>
#include <string.h>

#define TEST_CHECK(expr, message) \
	do { \
		if ( !(expr) ) { \
			printf("xui_callback_lifetime_test failed: %s\n", (message)); \
			return 0; \
		} \
	} while ( 0 )

typedef struct lifetime_state_t {
	xui_context pContext;
	int iFirstCount;
	int iSecondCount;
} lifetime_state_t;

static int test_setup(xui_context* ppContext, xui_widget* ppRoot)
{
	if ( xuiCreate(ppContext) != XUI_OK ) return 0;
	if ( xuiSetViewportSize(*ppContext, 160, 100) != XUI_OK ) return 0;
	if ( xuiWidgetCreate(*ppContext, ppRoot) != XUI_OK ) return 0;
	if ( xuiWidgetSetFocusable(*ppRoot, 1) != XUI_OK ) return 0;
	if ( xuiSetRootWidget(*ppContext, *ppRoot) != XUI_OK ) return 0;
	return 1;
}

static int destroy_self_on_key(xui_widget pWidget, const xui_event_t* pEvent, void* pUser)
{
	lifetime_state_t* pState = (lifetime_state_t*)pUser;
	if ( pEvent->iType == XUI_EVENT_KEY_DOWN && pEvent->iPhase == XUI_EVENT_PHASE_TARGET ) {
		pState->iFirstCount++;
		xuiWidgetDestroy(pWidget);
	}
	return XUI_OK;
}

static int destroy_self_on_pointer(xui_widget pWidget, const xui_event_t* pEvent, void* pUser)
{
	lifetime_state_t* pState = (lifetime_state_t*)pUser;
	if ( pEvent->iType == XUI_EVENT_POINTER_DOWN ) {
		pState->iFirstCount++;
		xuiWidgetDestroy(pWidget);
	}
	return XUI_OK;
}

static int destroy_self_on_layout(xui_widget pWidget, xui_rect_t tContentRect, void* pUser)
{
	lifetime_state_t* pState = (lifetime_state_t*)pUser;
	(void)tContentRect;
	pState->iFirstCount++;
	xuiWidgetDestroy(pWidget);
	return XUI_OK;
}

static int destroy_self_on_accessible_count(xui_widget_t* pWidget, void* pUser)
{
	lifetime_state_t* pState = (lifetime_state_t*)pUser;
	pState->iFirstCount++;
	xuiWidgetDestroy((xui_widget)pWidget);
	return 1;
}

static int unused_accessible_get(xui_widget_t* pWidget, int iIndex,
	xui_accessible_node_t* pNode, void* pUser)
{
	(void)pWidget;
	(void)iIndex;
	(void)pNode;
	(void)pUser;
	return XUI_OK;
}

static int hotkey_unregister_self(xui_widget pWidget, const xui_event_t* pEvent, void* pUser)
{
	lifetime_state_t* pState = (lifetime_state_t*)pUser;
	(void)pEvent;
	pState->iFirstCount++;
	(void)xuiHotKeyUnregister(pState->pContext, pWidget, 'K', XUI_MOD_CTRL);
	return XUI_OK;
}

static int hotkey_second(xui_widget pWidget, const xui_event_t* pEvent, void* pUser)
{
	lifetime_state_t* pState = (lifetime_state_t*)pUser;
	(void)pWidget;
	(void)pEvent;
	pState->iSecondCount++;
	return XUI_OK;
}

static int destroy_context_on_key(xui_widget pWidget, const xui_event_t* pEvent, void* pUser)
{
	lifetime_state_t* pState = (lifetime_state_t*)pUser;
	(void)pWidget;
	if ( pEvent->iType == XUI_EVENT_KEY_DOWN && pEvent->iPhase == XUI_EVENT_PHASE_TARGET ) {
		pState->iFirstCount++;
		xuiDestroy(pState->pContext);
	}
	return XUI_OK;
}

static void destroy_context_on_error(xui_context pContext, const xui_error_info_t* pError, void* pUser)
{
	lifetime_state_t* pState = (lifetime_state_t*)pUser;
	(void)pError;
	pState->iFirstCount++;
	xuiDestroy(pContext);
}

static int destroy_self_on_type_init(xui_widget pWidget, void* pTypeData,
	const void* pCreateData, void* pUser)
{
	lifetime_state_t* pState = (lifetime_state_t*)pUser;
	(void)pTypeData;
	(void)pCreateData;
	pState->iFirstCount++;
	xuiWidgetDestroy(pWidget);
	return XUI_OK;
}

static void destroy_context_on_type_destroy(xui_widget pWidget, void* pTypeData, void* pUser)
{
	lifetime_state_t* pState = (lifetime_state_t*)pUser;
	(void)pWidget;
	(void)pTypeData;
	pState->iFirstCount++;
	xuiDestroy(pState->pContext);
}

static int test_key_destroy(void)
{
	lifetime_state_t tState;
	xui_context pContext = NULL;
	xui_widget pRoot = NULL;
	memset(&tState, 0, sizeof(tState));
	TEST_CHECK(test_setup(&pContext, &pRoot), "key setup failed");
	TEST_CHECK(xuiWidgetSetEventCallback(pRoot, destroy_self_on_key, &tState) == XUI_OK,
		"key callback setup failed");
	TEST_CHECK(xuiSetFocusWidget(pContext, pRoot) == XUI_OK, "key focus failed");
	TEST_CHECK(xuiInputKeyDown(pContext, 'A', 0) == XUI_OK, "key dispatch failed");
	TEST_CHECK(tState.iFirstCount == 1, "key callback was not called");
	TEST_CHECK(xuiGetRootWidget(pContext) == NULL, "destroyed key target remained attached");
	xuiDestroy(pContext);
	return 1;
}

static int test_pointer_destroy(void)
{
	lifetime_state_t tState;
	xui_context pContext = NULL;
	xui_widget pRoot = NULL;
	xui_widget pTarget = NULL;
	memset(&tState, 0, sizeof(tState));
	TEST_CHECK(test_setup(&pContext, &pRoot), "pointer setup failed");
	TEST_CHECK(xuiWidgetCreate(pContext, &pTarget) == XUI_OK, "pointer target create failed");
	TEST_CHECK(xuiWidgetSetRect(pTarget, (xui_rect_t){0, 0, 80, 80}) == XUI_OK,
		"pointer target rect failed");
	TEST_CHECK(xuiWidgetAddChild(pRoot, pTarget) == XUI_OK, "pointer target attach failed");
	TEST_CHECK(xuiWidgetSetEventCallback(pTarget, destroy_self_on_pointer, &tState) == XUI_OK,
		"pointer callback setup failed");
	TEST_CHECK(xuiHitTest(pContext, 10, 10, XUI_WIDGET_HIT_DEFAULT) == pTarget,
		"pointer target hit test failed");
	TEST_CHECK(xuiInputPointerDown(pContext, 10, 10, XUI_POINTER_BUTTON_LEFT,
		0) == XUI_OK, "pointer dispatch failed");
	TEST_CHECK(xuiDispatchPendingEvents(pContext) == XUI_OK, "pointer event pump failed");
	TEST_CHECK(tState.iFirstCount == 1, "pointer callback was not called");
	TEST_CHECK(xuiWidgetGetChildCount(pRoot) == 0, "destroyed pointer target remained attached");
	xuiDestroy(pContext);
	return 1;
}

static int test_layout_destroy(void)
{
	lifetime_state_t tState;
	xui_context pContext = NULL;
	xui_widget pRoot = NULL;
	memset(&tState, 0, sizeof(tState));
	TEST_CHECK(test_setup(&pContext, &pRoot), "layout setup failed");
	TEST_CHECK(xuiWidgetSetLayoutCompleteCallback(pRoot, destroy_self_on_layout, &tState) == XUI_OK,
		"layout callback setup failed");
	TEST_CHECK(xuiLayout(pContext) == XUI_OK, "layout dispatch failed");
	TEST_CHECK(tState.iFirstCount == 1, "layout callback was not called");
	TEST_CHECK(xuiGetRootWidget(pContext) == NULL, "destroyed layout target remained attached");
	xuiDestroy(pContext);
	return 1;
}

static int test_accessibility_destroy(void)
{
	lifetime_state_t tState;
	xui_context pContext = NULL;
	xui_widget pRoot = NULL;
	memset(&tState, 0, sizeof(tState));
	TEST_CHECK(test_setup(&pContext, &pRoot), "accessibility setup failed");
	TEST_CHECK(xuiWidgetSetAccessibilityProvider(pRoot, destroy_self_on_accessible_count,
		unused_accessible_get, NULL, &tState) == XUI_OK, "accessibility callback setup failed");
	TEST_CHECK(xuiWidgetGetAccessibleNodeCount(pRoot) == 0,
		"destroyed accessibility provider returned a live node");
	TEST_CHECK(tState.iFirstCount == 1, "accessibility callback was not called");
	TEST_CHECK(xuiGetRootWidget(pContext) == NULL, "destroyed accessibility target remained attached");
	xuiDestroy(pContext);
	return 1;
}

static int test_hotkey_mutation(void)
{
	lifetime_state_t tState;
	xui_context pContext = NULL;
	xui_widget pRoot = NULL;
	xui_widget pFirst = NULL;
	xui_widget pSecond = NULL;
	memset(&tState, 0, sizeof(tState));
	TEST_CHECK(test_setup(&pContext, &pRoot), "hotkey setup failed");
	tState.pContext = pContext;
	TEST_CHECK(xuiWidgetCreate(pContext, &pFirst) == XUI_OK, "first hotkey widget create failed");
	TEST_CHECK(xuiWidgetCreate(pContext, &pSecond) == XUI_OK, "second hotkey widget create failed");
	TEST_CHECK(xuiWidgetAddChild(pRoot, pFirst) == XUI_OK, "first hotkey widget attach failed");
	TEST_CHECK(xuiWidgetAddChild(pRoot, pSecond) == XUI_OK, "second hotkey widget attach failed");
	TEST_CHECK(xuiHotKeyRegister(pContext, pFirst, 'K', XUI_MOD_CTRL,
		hotkey_unregister_self, &tState) == XUI_OK, "first hotkey register failed");
	TEST_CHECK(xuiHotKeyRegister(pContext, pSecond, 'K', XUI_MOD_CTRL,
		hotkey_second, &tState) == XUI_OK, "second hotkey register failed");
	TEST_CHECK(xuiInputKeyDown(pContext, 'K', XUI_MOD_CTRL) == XUI_OK, "hotkey dispatch failed");
	TEST_CHECK(tState.iFirstCount == 1 && tState.iSecondCount == 1,
		"hotkey mutation changed the current dispatch snapshot");
	xuiDestroy(pContext);
	return 1;
}

static int test_context_destroy_from_key(void)
{
	lifetime_state_t tState;
	xui_context pContext = NULL;
	xui_widget pRoot = NULL;
	memset(&tState, 0, sizeof(tState));
	TEST_CHECK(test_setup(&pContext, &pRoot), "context key setup failed");
	tState.pContext = pContext;
	TEST_CHECK(xuiWidgetSetEventCallback(pRoot, destroy_context_on_key, &tState) == XUI_OK,
		"context key callback setup failed");
	TEST_CHECK(xuiSetFocusWidget(pContext, pRoot) == XUI_OK, "context key focus failed");
	TEST_CHECK(xuiInputKeyDown(pContext, 'D', 0) == XUI_OK, "context key dispatch failed");
	TEST_CHECK(tState.iFirstCount == 1, "context key callback was not called");
	return 1;
}

static int test_context_destroy_from_error(void)
{
	lifetime_state_t tState;
	xui_error_info_t tError;
	xui_context pContext = NULL;
	memset(&tState, 0, sizeof(tState));
	TEST_CHECK(xuiCreate(&pContext) == XUI_OK, "error context create failed");
	TEST_CHECK(xuiSetErrorCallback(pContext, destroy_context_on_error, &tState) == XUI_OK,
		"error callback setup failed");
	memset(&tError, 0, sizeof(tError));
	tError.iSize = sizeof(tError);
	tError.iCode = XUI_ERROR;
	TEST_CHECK(xuiReportError(pContext, &tError) == XUI_OK, "error callback dispatch failed");
	TEST_CHECK(tState.iFirstCount == 1, "error callback was not called");
	return 1;
}

static int test_widget_destroy_from_type_init(void)
{
	lifetime_state_t tState;
	xui_context pContext = NULL;
	xui_widget_type pType = NULL;
	xui_widget pWidget = NULL;
	xui_widget_type_desc_t tDesc;
	memset(&tState, 0, sizeof(tState));
	TEST_CHECK(xuiCreate(&pContext) == XUI_OK, "type init context create failed");
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.sName = "test.lifetime.init_destroy";
	tDesc.pUser = &tState;
	tDesc.onInit = destroy_self_on_type_init;
	TEST_CHECK(xuiWidgetRegisterType(pContext, &pType, &tDesc) == XUI_OK,
		"type init register failed");
	TEST_CHECK(xuiWidgetCreateTyped(pContext, pType, &pWidget, NULL) == XUI_ERROR,
		"type init self-destroy should fail creation");
	TEST_CHECK(pWidget == NULL, "type init self-destroy returned a dangling widget");
	TEST_CHECK(tState.iFirstCount == 1, "type init callback was not called");
	TEST_CHECK(xuiWidgetUnregisterType(pType) == XUI_OK,
		"type init self-destroy leaked the live widget count");
	xuiDestroy(pContext);
	return 1;
}

static int test_context_destroy_from_type_destroy(void)
{
	lifetime_state_t tState;
	xui_context pContext = NULL;
	xui_widget_type pType = NULL;
	xui_widget pWidget = NULL;
	xui_widget_type_desc_t tDesc;
	memset(&tState, 0, sizeof(tState));
	TEST_CHECK(xuiCreate(&pContext) == XUI_OK, "type destroy context create failed");
	tState.pContext = pContext;
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.sName = "test.lifetime.destroy_context";
	tDesc.pUser = &tState;
	tDesc.onDestroy = destroy_context_on_type_destroy;
	TEST_CHECK(xuiWidgetRegisterType(pContext, &pType, &tDesc) == XUI_OK,
		"type destroy register failed");
	TEST_CHECK(xuiWidgetCreateTyped(pContext, pType, &pWidget, NULL) == XUI_OK,
		"type destroy widget create failed");
	xuiWidgetDestroy(pWidget);
	TEST_CHECK(tState.iFirstCount == 1, "type destroy callback was not called");
	return 1;
}

int main(void)
{
	if ( !test_key_destroy() || !test_pointer_destroy() || !test_layout_destroy() ||
	     !test_accessibility_destroy() || !test_hotkey_mutation() ||
	     !test_context_destroy_from_key() || !test_context_destroy_from_error() ||
	     !test_widget_destroy_from_type_init() ||
	     !test_context_destroy_from_type_destroy() ) {
		return 1;
	}
	printf("xui_callback_lifetime_test passed\n");
	return 0;
}
