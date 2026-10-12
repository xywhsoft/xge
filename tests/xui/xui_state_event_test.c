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

typedef struct interaction_cancel_log_t {
	int arrTypes[8];
	int iCount;
} interaction_cancel_log_t;

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

static int interaction_cancel_handler(xui_widget pWidget, const xui_event_t* pEvent, void* pUser)
{
	interaction_cancel_log_t* pLog = (interaction_cancel_log_t*)pUser;
	(void)pWidget;
	if ( pEvent->iPhase != XUI_EVENT_PHASE_TARGET ) return XUI_OK;
	if ( pEvent->iType == XUI_EVENT_DRAG_CANCEL ||
	     pEvent->iType == XUI_EVENT_POINTER_CAPTURE_LOST ||
	     pEvent->iType == XUI_EVENT_POINTER_LEAVE ||
	     pEvent->iType == XUI_EVENT_BLUR ) {
		if ( pLog->iCount < (int)(sizeof(pLog->arrTypes) / sizeof(pLog->arrTypes[0])) ) {
			pLog->arrTypes[pLog->iCount++] = pEvent->iType;
		}
	}
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
	TEST_CHECK(xuiWidgetSetFocusable(pTarget, 1) == XUI_OK, "target focusable failed");
	TEST_CHECK(xuiWidgetGetEffectiveVisible(pTarget) &&
		xuiWidgetGetEffectiveEnabled(pTarget) &&
		xuiWidgetGetEffectiveHitTestVisible(pTarget) &&
		xuiWidgetGetEffectiveFocusable(pTarget), "initial effective state mismatch");
	TEST_CHECK(xuiWidgetSetEnabled(pParent, 0) == XUI_OK, "ancestor disable failed");
	TEST_CHECK(tState.iEnabledCount == 3 && !xuiWidgetGetEffectiveEnabled(pTarget) &&
		(xuiWidgetGetInputState(pTarget) & XUI_WIDGET_STATE_DISABLED) != 0,
		"ancestor disable did not propagate effective state");
	TEST_CHECK(xuiWidgetSetEnabled(pParent, 1) == XUI_OK, "ancestor enable failed");
	TEST_CHECK(tState.iEnabledCount == 4 && xuiWidgetGetEffectiveEnabled(pTarget),
		"ancestor enable did not propagate effective state");
	TEST_CHECK(xuiWidgetSetVisible(pParent, 0) == XUI_OK, "ancestor hide failed");
	TEST_CHECK(tState.iVisibleCount == 3 && !xuiWidgetGetEffectiveVisible(pTarget),
		"ancestor hide did not propagate effective state");
	TEST_CHECK(xuiWidgetSetVisible(pParent, 1) == XUI_OK, "ancestor show failed");
	TEST_CHECK(tState.iVisibleCount == 4 && xuiWidgetGetEffectiveVisible(pTarget),
		"ancestor show did not propagate effective state");
	TEST_CHECK(xuiWidgetSetHitTestVisible(pParent, 0) == XUI_OK,
		"ancestor hit-test disable failed");
	TEST_CHECK(!xuiWidgetGetEffectiveHitTestVisible(pTarget),
		"ancestor hit-test state did not propagate");
	TEST_CHECK(xuiWidgetSetHitTestVisible(pParent, 1) == XUI_OK,
		"ancestor hit-test enable failed");

	tState.bDestroyOnVisible = 1;
	TEST_CHECK(xuiWidgetSetVisible(pTarget, 0) == XUI_OK,
		"self-destroying visible event failed");
	TEST_CHECK(tState.iVisibleCount == 5, "self-destroying visible event was not emitted");
	TEST_CHECK(xuiWidgetGetChildCount(pParent) == 0,
		"state callback destroyed target remained attached");
	xuiDestroy(pContext);
	return 1;
}

static int test_interaction_cancellation(void)
{
	interaction_cancel_log_t tLog;
	xui_context pContext = NULL;
	xui_widget pRoot = NULL;
	xui_widget pParent = NULL;
	xui_widget pTarget = NULL;
	xui_widget pSibling = NULL;
	uint32_t iState;
	memset(&tLog, 0, sizeof(tLog));
	TEST_CHECK(xuiCreate(&pContext) == XUI_OK, "cancel context create failed");
	TEST_CHECK(xuiSetViewportSize(pContext, 240, 140) == XUI_OK,
		"cancel viewport failed");
	TEST_CHECK(xuiWidgetCreate(pContext, &pRoot) == XUI_OK, "cancel root create failed");
	TEST_CHECK(xuiWidgetCreate(pContext, &pParent) == XUI_OK, "cancel parent create failed");
	TEST_CHECK(xuiWidgetCreate(pContext, &pTarget) == XUI_OK, "cancel target create failed");
	TEST_CHECK(xuiWidgetCreate(pContext, &pSibling) == XUI_OK, "cancel sibling create failed");
	TEST_CHECK(xuiSetRootWidget(pContext, pRoot) == XUI_OK, "cancel root attach failed");
	TEST_CHECK(xuiWidgetAddChild(pRoot, pParent) == XUI_OK, "cancel parent attach failed");
	TEST_CHECK(xuiWidgetAddChild(pParent, pTarget) == XUI_OK, "cancel target attach failed");
	TEST_CHECK(xuiWidgetAddChild(pRoot, pSibling) == XUI_OK, "cancel sibling attach failed");
	TEST_CHECK(xuiWidgetSetRect(pRoot, (xui_rect_t){0, 0, 240, 140}) == XUI_OK,
		"cancel root rect failed");
	TEST_CHECK(xuiWidgetSetRect(pParent, (xui_rect_t){10, 10, 100, 80}) == XUI_OK,
		"cancel parent rect failed");
	TEST_CHECK(xuiWidgetSetRect(pTarget, (xui_rect_t){5, 5, 70, 50}) == XUI_OK,
		"cancel target rect failed");
	TEST_CHECK(xuiWidgetSetRect(pSibling, (xui_rect_t){130, 10, 70, 50}) == XUI_OK,
		"cancel sibling rect failed");
	TEST_CHECK(xuiWidgetSetFocusable(pTarget, 1) == XUI_OK, "cancel target focusable failed");
	TEST_CHECK(xuiWidgetSetFocusable(pSibling, 1) == XUI_OK, "cancel sibling focusable failed");
	TEST_CHECK(xuiWidgetSetDragEnabled(pTarget, 1) == XUI_OK, "cancel drag enable failed");
	TEST_CHECK(xuiWidgetSetEventCallback(pTarget, interaction_cancel_handler, &tLog) == XUI_OK,
		"cancel event callback failed");
	TEST_CHECK(xuiInputPointerMove(pContext, 20, 20, 0) == XUI_OK,
		"cancel pointer move failed");
	TEST_CHECK(xuiInputPointerDown(pContext, 20, 20, XUI_POINTER_BUTTON_LEFT,
		XUI_POINTER_BUTTON_LEFT) == XUI_OK, "cancel pointer down failed");
	TEST_CHECK(xuiSetPointerCapture(pContext, pTarget) == XUI_OK,
		"cancel capture failed");
	TEST_CHECK(xuiInputPointerMove(pContext, 40, 40, XUI_POINTER_BUTTON_LEFT) == XUI_OK,
		"cancel drag move failed");
	TEST_CHECK(xuiDispatchPendingEvents(pContext) == XUI_OK,
		"cancel pending event dispatch failed");
	TEST_CHECK(xuiGetHoverWidget(pContext) == pTarget &&
		xuiGetActiveWidget(pContext) == pTarget &&
		xuiGetPointerCapture(pContext) == pTarget &&
		xuiGetFocusWidget(pContext) == pTarget, "cancel setup state mismatch");

	memset(&tLog, 0, sizeof(tLog));
	TEST_CHECK(xuiWidgetSetEnabled(pParent, 0) == XUI_OK,
		"cancel ancestor disable failed");
	TEST_CHECK(tLog.iCount == 4 &&
		tLog.arrTypes[0] == XUI_EVENT_DRAG_CANCEL &&
		tLog.arrTypes[1] == XUI_EVENT_POINTER_CAPTURE_LOST &&
		tLog.arrTypes[2] == XUI_EVENT_POINTER_LEAVE &&
		tLog.arrTypes[3] == XUI_EVENT_BLUR, "interaction cancel event order mismatch");
	TEST_CHECK(xuiGetHoverWidget(pContext) == NULL &&
		xuiGetActiveWidget(pContext) == NULL &&
		xuiGetPointerCapture(pContext) == NULL &&
		xuiGetFocusWidget(pContext) == NULL, "interaction references were not cleared");
	iState = xuiWidgetGetInputState(pTarget);
	TEST_CHECK((iState & (XUI_WIDGET_STATE_HOVER | XUI_WIDGET_STATE_ACTIVE |
		XUI_WIDGET_STATE_FOCUS)) == 0 && (iState & XUI_WIDGET_STATE_DISABLED) != 0,
		"interaction visual state was not cleared");
	TEST_CHECK(!xuiWidgetGetEffectiveFocusable(pTarget),
		"disabled ancestor left descendant effectively focusable");
	TEST_CHECK(xuiSetFocusWidget(pContext, pTarget) == XUI_ERROR_INVALID_ARGUMENT,
		"direct focus accepted a descendant of a disabled ancestor");
	TEST_CHECK(xuiSetFocusWidget(pContext, pSibling) == XUI_OK,
		"sibling focus failed");
	TEST_CHECK(xuiFocusNext(pContext, 1) == XUI_OK &&
		xuiGetFocusWidget(pContext) == pSibling,
		"tab traversal entered a disabled ancestor subtree");
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
	if ( !test_direct_and_ancestor_changes() || !test_interaction_cancellation() ||
	     !test_operation_coalescing() ) return 1;
	printf("xui_state_event_test passed\n");
	return 0;
}
