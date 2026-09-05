#include "xui.h"

#include <stdio.h>
#include <string.h>

#define TEST_CHECK(expr, message) \
	do { \
		if ( !(expr) ) { \
			printf("xui_state_event_test failed: %s\n", (message)); \
			return 0; \
		} \
	} while ( 0 )

typedef struct state_event_test_t {
	xui_widget pTarget;
	int iBoundsCount;
	int iVisibleCount;
	int iEnabledCount;
	int bVisible;
	int bEnabled;
	int bDestroyOnVisible;
	xui_rect_t tWorldRect;
} state_event_test_t;

typedef struct state_event_driver_t {
	xui_widget pTarget;
	int bRun;
} state_event_driver_t;

static int state_event_handler(xui_widget pWidget, const xui_event_t* pEvent, void* pUser)
{
	state_event_test_t* pState = (state_event_test_t*)pUser;
	(void)pWidget;
	if ( pEvent->iPhase != XUI_EVENT_PHASE_TARGET || pEvent->pTarget != pState->pTarget ) {
		return XUI_OK;
	}
	switch ( pEvent->iType ) {
	case XUI_EVENT_BOUNDS_CHANGED:
		pState->iBoundsCount++;
		pState->tWorldRect = xuiWidgetGetWorldRect(pState->pTarget);
		break;
	case XUI_EVENT_VISIBLE_CHANGED:
		pState->iVisibleCount++;
		pState->bVisible = xuiWidgetGetVisible(pState->pTarget);
		if ( pState->bDestroyOnVisible ) xuiWidgetDestroy(pState->pTarget);
		break;
	case XUI_EVENT_ENABLED_CHANGED:
		pState->iEnabledCount++;
		pState->bEnabled = xuiWidgetGetEnabled(pState->pTarget);
		break;
	default:
		break;
	}
	return XUI_OK;
}

static int state_event_driver_update(xui_widget pWidget, float fDelta, void* pUser)
{
	state_event_driver_t* pDriver = (state_event_driver_t*)pUser;
	(void)pWidget;
	(void)fDelta;
	if ( !pDriver->bRun ) return XUI_OK;
	pDriver->bRun = 0;
	(void)xuiWidgetSetRect(pDriver->pTarget, (xui_rect_t){30, 12, 40, 20});
	(void)xuiWidgetSetRect(pDriver->pTarget, (xui_rect_t){34, 16, 40, 20});
	(void)xuiWidgetSetVisible(pDriver->pTarget, 0);
	(void)xuiWidgetSetVisible(pDriver->pTarget, 1);
	(void)xuiWidgetSetEnabled(pDriver->pTarget, 0);
	(void)xuiWidgetSetEnabled(pDriver->pTarget, 1);
	return XUI_OK;
}

static int test_direct_and_ancestor_changes(void)
{
	state_event_test_t tState;
	xui_context pContext = NULL;
	xui_widget pRoot = NULL;
	xui_widget pParent = NULL;
	xui_widget pTarget = NULL;
	memset(&tState, 0, sizeof(tState));
	TEST_CHECK(xuiCreate(&pContext) == XUI_OK, "context create failed");
	TEST_CHECK(xuiWidgetCreate(pContext, &pRoot) == XUI_OK, "root create failed");
	TEST_CHECK(xuiWidgetCreate(pContext, &pParent) == XUI_OK, "parent create failed");
	TEST_CHECK(xuiWidgetCreate(pContext, &pTarget) == XUI_OK, "target create failed");
	TEST_CHECK(xuiSetRootWidget(pContext, pRoot) == XUI_OK, "root attach failed");
	TEST_CHECK(xuiWidgetAddChild(pRoot, pParent) == XUI_OK, "parent attach failed");
	TEST_CHECK(xuiWidgetAddChild(pParent, pTarget) == XUI_OK, "target attach failed");
	TEST_CHECK(xuiWidgetSetRect(pRoot, (xui_rect_t){0, 0, 200, 100}) == XUI_OK,
		"root rect failed");
	TEST_CHECK(xuiWidgetSetRect(pParent, (xui_rect_t){10, 4, 100, 80}) == XUI_OK,
		"parent rect failed");
	TEST_CHECK(xuiWidgetSetRect(pTarget, (xui_rect_t){5, 6, 40, 20}) == XUI_OK,
		"target rect failed");
	tState.pTarget = pTarget;
	TEST_CHECK(xuiWidgetSetEventHandler(pTarget, XUI_EVENT_BOUNDS_CHANGED,
		state_event_handler, &tState) == XUI_OK, "bounds handler failed");
	TEST_CHECK(xuiWidgetSetEventHandler(pTarget, XUI_EVENT_VISIBLE_CHANGED,
		state_event_handler, &tState) == XUI_OK, "visible handler failed");
	TEST_CHECK(xuiWidgetSetEventHandler(pTarget, XUI_EVENT_ENABLED_CHANGED,
		state_event_handler, &tState) == XUI_OK, "enabled handler failed");

	TEST_CHECK(xuiWidgetSetRect(pTarget, (xui_rect_t){7, 8, 40, 20}) == XUI_OK,
		"target rect update failed");
	TEST_CHECK(tState.iBoundsCount == 1 && tState.tWorldRect.fX == 17 &&
		tState.tWorldRect.fY == 12, "direct bounds event mismatch");
	TEST_CHECK(xuiWidgetSetRect(pTarget, (xui_rect_t){7, 8, 40, 20}) == XUI_OK,
		"same target rect failed");
	TEST_CHECK(tState.iBoundsCount == 1, "unchanged rect emitted an event");

	TEST_CHECK(xuiWidgetSetRect(pParent, (xui_rect_t){20, 14, 100, 80}) == XUI_OK,
		"ancestor rect update failed");
	TEST_CHECK(tState.iBoundsCount == 2 && tState.tWorldRect.fX == 27 &&
		tState.tWorldRect.fY == 22, "ancestor move did not notify descendant bounds");
	TEST_CHECK(xuiWidgetSetVisible(pTarget, 0) == XUI_OK, "hide failed");
	TEST_CHECK(tState.iVisibleCount == 1 && !tState.bVisible, "hide event mismatch");
	TEST_CHECK(xuiWidgetSetVisible(pTarget, 1) == XUI_OK, "show failed");
	TEST_CHECK(tState.iVisibleCount == 2 && tState.bVisible, "show event mismatch");
	TEST_CHECK(xuiWidgetSetEnabled(pTarget, 0) == XUI_OK, "disable failed");
	TEST_CHECK(tState.iEnabledCount == 1 && !tState.bEnabled, "disable event mismatch");
	TEST_CHECK(xuiWidgetSetEnabled(pTarget, 1) == XUI_OK, "enable failed");
	TEST_CHECK(tState.iEnabledCount == 2 && tState.bEnabled, "enable event mismatch");

	tState.bDestroyOnVisible = 1;
	TEST_CHECK(xuiWidgetSetVisible(pTarget, 0) == XUI_OK,
		"self-destroying visible event failed");
	TEST_CHECK(tState.iVisibleCount == 3, "self-destroying visible event was not emitted");
	TEST_CHECK(xuiWidgetGetChildCount(pParent) == 0,
		"state callback destroyed target remained attached");
	xuiDestroy(pContext);
	return 1;
}

static int test_operation_coalescing(void)
{
	state_event_test_t tState;
	state_event_driver_t* pDriverData;
	xui_context pContext = NULL;
	xui_widget_type pDriverType = NULL;
	xui_widget_type_desc_t tDesc;
	xui_widget pRoot = NULL;
	xui_widget pTarget = NULL;
	xui_widget pDriver = NULL;
	memset(&tState, 0, sizeof(tState));
	TEST_CHECK(xuiCreate(&pContext) == XUI_OK, "coalesce context create failed");
	TEST_CHECK(xuiWidgetCreate(pContext, &pRoot) == XUI_OK, "coalesce root create failed");
	TEST_CHECK(xuiWidgetCreate(pContext, &pTarget) == XUI_OK, "coalesce target create failed");
	TEST_CHECK(xuiSetRootWidget(pContext, pRoot) == XUI_OK, "coalesce root attach failed");
	TEST_CHECK(xuiWidgetAddChild(pRoot, pTarget) == XUI_OK, "coalesce target attach failed");
	TEST_CHECK(xuiWidgetSetRect(pTarget, (xui_rect_t){4, 5, 40, 20}) == XUI_OK,
		"coalesce initial rect failed");
	tState.pTarget = pTarget;
	TEST_CHECK(xuiWidgetSetEventHandler(pTarget, XUI_EVENT_BOUNDS_CHANGED,
		state_event_handler, &tState) == XUI_OK, "coalesce bounds handler failed");
	TEST_CHECK(xuiWidgetSetEventHandler(pTarget, XUI_EVENT_VISIBLE_CHANGED,
		state_event_handler, &tState) == XUI_OK, "coalesce visible handler failed");
	TEST_CHECK(xuiWidgetSetEventHandler(pTarget, XUI_EVENT_ENABLED_CHANGED,
		state_event_handler, &tState) == XUI_OK, "coalesce enabled handler failed");

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.sName = "test.state_event.driver";
	tDesc.iTypeDataSize = sizeof(state_event_driver_t);
	tDesc.onUpdate = state_event_driver_update;
	TEST_CHECK(xuiWidgetRegisterType(pContext, &pDriverType, &tDesc) == XUI_OK,
		"driver type register failed");
	TEST_CHECK(xuiWidgetCreateTyped(pContext, pDriverType, &pDriver, NULL) == XUI_OK,
		"driver create failed");
	TEST_CHECK(xuiWidgetAddChild(pRoot, pDriver) == XUI_OK, "driver attach failed");
	pDriverData = (state_event_driver_t*)xuiWidgetGetTypeData(pDriver);
	TEST_CHECK(pDriverData != NULL, "driver data missing");
	pDriverData->pTarget = pTarget;
	pDriverData->bRun = 1;
	TEST_CHECK(xuiUpdate(pContext, 0.016f) == XUI_OK, "coalesced update failed");
	TEST_CHECK(tState.iBoundsCount == 1, "bounds changes were not coalesced");
	TEST_CHECK(tState.iVisibleCount == 0, "net-zero visibility emitted an event");
	TEST_CHECK(tState.iEnabledCount == 0, "net-zero enabled state emitted an event");
	TEST_CHECK(tState.tWorldRect.fX == 34 && tState.tWorldRect.fY == 16,
		"coalesced bounds event did not expose final geometry");

	xuiWidgetDestroy(pDriver);
	pDriver = NULL;
	TEST_CHECK(xuiWidgetUnregisterType(pDriverType) == XUI_OK, "driver type unregister failed");
	xuiDestroy(pContext);
	return 1;
}

int main(void)
{
	if ( !test_direct_and_ancestor_changes() || !test_operation_coalescing() ) return 1;
	printf("xui_state_event_test passed\n");
	return 0;
}
