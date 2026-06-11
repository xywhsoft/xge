#include "xui.h"

#include <stdio.h>
#include <string.h>

#define XUI_TEST_CHECK(expr, msg) \
	do { \
		if ( !(expr) ) { \
			printf("xui_input_test failed: %s\n", (msg)); \
			iFailed = 1; \
			goto cleanup; \
		} \
	} while ( 0 )

static int __xuiTestPoll(xui_context pContext, int iType, xui_widget pTarget, xui_event_t* pEvent)
{
	int iRet;

	iRet = xuiPollEvent(pContext, pEvent);
	return (iRet == 1) && (pEvent->iType == iType) && (pEvent->pTarget == pTarget);
}

typedef struct xui_input_dispatch_log_t {
	int iCount;
	xui_widget arrWidget[16];
	int arrPhase[16];
	int arrType[16];
	int arrKey[16];
	uint32_t arrModifiers[16];
	uint32_t arrCodepoint[16];
} xui_input_dispatch_log_t;

typedef struct xui_input_extended_log_t {
	int iDoubleClickCount;
	int iContextMenuCount;
	int iDragBeginCount;
	int iDragMoveCount;
	int iDragEndCount;
	int iCommandCount;
	int iHotkeyCount;
	int iDefaultCount;
	int iCancelCount;
	int iImeCount;
	int iLastCommand;
	char sLastCommand[64];
} xui_input_extended_log_t;

typedef struct xui_input_multi_pointer_log_t {
	xui_widget pA;
	xui_widget pB;
	int iDownA;
	int iDownB;
	int iMoveA;
	int iMoveB;
	int iUpA;
	int iUpB;
	int iCaptureLostA;
	int iCaptureLostB;
	int iLegacyCaptureOk;
	uint64_t iLastPointerId;
	int iLastPointerType;
} xui_input_multi_pointer_log_t;

typedef struct xui_input_tooltip_resolver_t {
	const char* sText;
	int bEnabled;
	int iResolveCount;
} xui_input_tooltip_resolver_t;

static int __xuiTestEventCallback(xui_widget pWidget, const xui_event_t* pEvent, void* pUser)
{
	xui_input_dispatch_log_t* pLog;
	int iIndex;

	pLog = (xui_input_dispatch_log_t*)pUser;
	if ( (pLog == NULL) || (pLog->iCount >= (int)(sizeof(pLog->arrWidget) / sizeof(pLog->arrWidget[0]))) ) {
		return XUI_OK;
	}
	iIndex = pLog->iCount++;
	pLog->arrWidget[iIndex] = pWidget;
	pLog->arrPhase[iIndex] = pEvent->iPhase;
	pLog->arrType[iIndex] = pEvent->iType;
	pLog->arrKey[iIndex] = pEvent->iKey;
	pLog->arrModifiers[iIndex] = pEvent->iModifiers;
	pLog->arrCodepoint[iIndex] = pEvent->iCodepoint;
	if ( pEvent->pCurrentTarget != pWidget ) {
		return XUI_ERROR;
	}
	return XUI_OK;
}

static int __xuiTestExtendedEventCallback(xui_widget pWidget, const xui_event_t* pEvent, void* pUser)
{
	xui_input_extended_log_t* pLog;

	(void)pWidget;
	pLog = (xui_input_extended_log_t*)pUser;
	if ( pLog == NULL ) {
		return XUI_OK;
	}
	switch ( pEvent->iType ) {
	case XUI_EVENT_POINTER_DOUBLE_CLICK:
		pLog->iDoubleClickCount++;
		break;
	case XUI_EVENT_CONTEXT_MENU:
		pLog->iContextMenuCount++;
		break;
	case XUI_EVENT_DRAG_BEGIN:
		pLog->iDragBeginCount++;
		break;
	case XUI_EVENT_DRAG_MOVE:
		pLog->iDragMoveCount++;
		break;
	case XUI_EVENT_DRAG_END:
		pLog->iDragEndCount++;
		break;
	case XUI_EVENT_COMMAND:
		pLog->iCommandCount++;
		pLog->iLastCommand = pEvent->iCommand;
		if ( pEvent->sCommand != NULL ) {
			snprintf(pLog->sLastCommand, sizeof(pLog->sLastCommand), "%s", pEvent->sCommand);
		}
		break;
	case XUI_EVENT_HOTKEY:
		pLog->iHotkeyCount++;
		break;
	case XUI_EVENT_IME_COMPOSITION:
		pLog->iImeCount++;
		break;
	default:
		break;
	}
	return XUI_OK;
}

static int __xuiTestMultiPointerCallback(xui_widget pWidget, const xui_event_t* pEvent, void* pUser)
{
	xui_input_multi_pointer_log_t* pLog;
	xui_context pContext;

	pLog = (xui_input_multi_pointer_log_t*)pUser;
	if ( (pLog == NULL) || (pEvent == NULL) ) {
		return XUI_OK;
	}
	pContext = xuiWidgetGetContext(pWidget);
	pLog->iLastPointerId = pEvent->iPointerId;
	pLog->iLastPointerType = pEvent->iPointerType;
	if ( pEvent->iType == XUI_EVENT_POINTER_DOWN ) {
		if ( pWidget == pLog->pA ) pLog->iDownA++;
		if ( pWidget == pLog->pB ) pLog->iDownB++;
		if ( xuiSetPointerCapture(pContext, pWidget) == XUI_OK && xuiGetPointerCapture(pContext) == pWidget ) {
			pLog->iLegacyCaptureOk++;
		}
	} else if ( pEvent->iType == XUI_EVENT_POINTER_MOVE ) {
		if ( pWidget == pLog->pA ) pLog->iMoveA++;
		if ( pWidget == pLog->pB ) pLog->iMoveB++;
	} else if ( pEvent->iType == XUI_EVENT_POINTER_UP ) {
		if ( pWidget == pLog->pA ) pLog->iUpA++;
		if ( pWidget == pLog->pB ) pLog->iUpB++;
		(void)xuiReleasePointerCapture(pContext, pWidget);
	} else if ( pEvent->iType == XUI_EVENT_POINTER_CAPTURE_LOST ) {
		if ( pWidget == pLog->pA ) pLog->iCaptureLostA++;
		if ( pWidget == pLog->pB ) pLog->iCaptureLostB++;
	}
	return XUI_OK;
}

static void __xuiTestActionCallback(xui_widget pWidget, void* pUser)
{
	int* pCount;

	(void)pWidget;
	pCount = (int*)pUser;
	if ( pCount != NULL ) {
		(*pCount)++;
	}
}

static xui_rect_t __xuiTestImeRectCallback(xui_widget pWidget, void* pUser)
{
	xui_rect_t* pRect;

	(void)pWidget;
	pRect = (xui_rect_t*)pUser;
	return (pRect != NULL) ? *pRect : (xui_rect_t){0.0f, 0.0f, 0.0f, 0.0f};
}

static int __xuiTestTooltipResolve(xui_context pContext, xui_widget pWidget, xui_tooltip_desc_t* pDesc, void* pUser)
{
	xui_input_tooltip_resolver_t* pResolver;

	(void)pContext;
	(void)pWidget;
	if ( (pDesc == NULL) || (pUser == NULL) ) {
		return 0;
	}
	pResolver = (xui_input_tooltip_resolver_t*)pUser;
	if ( !pResolver->bEnabled || (pResolver->sText == NULL) ) {
		return 0;
	}
	pResolver->iResolveCount++;
	memset(pDesc, 0, sizeof(*pDesc));
	pDesc->iSize = sizeof(*pDesc);
	pDesc->iType = XUI_TOOLTIP_TEXT;
	pDesc->sText = pResolver->sText;
	pDesc->iAnchor = XUI_TOOLTIP_ANCHOR_CURSOR;
	pDesc->fOffsetX = 4.0f;
	pDesc->fOffsetY = 5.0f;
	pDesc->fDelay = 0.0f;
	pDesc->bFollowCursor = 1;
	return 1;
}

int main(void)
{
	xui_input_dispatch_log_t tLog;
	xui_input_extended_log_t tExt;
	xui_input_multi_pointer_log_t tMulti;
	xui_context pContext;
	xui_widget pRoot;
	xui_widget pA;
	xui_widget pB;
	xui_widget pC;
	xui_widget pOverlay;
	xui_event_t tEvent;
	xui_debug_widget_info_t tDebugInfo;
	xui_input_tooltip_resolver_t tTooltipResolver;
	xui_widget_event_proc onEvent;
	void* pEventUser;
	xui_rect_t tImeRect;
	xui_rect_t tRect;
	char sDebug[2048];
	int iRet;
	int iFailed;

	memset(&tLog, 0, sizeof(tLog));
	memset(&tExt, 0, sizeof(tExt));
	memset(&tMulti, 0, sizeof(tMulti));
	memset(&tTooltipResolver, 0, sizeof(tTooltipResolver));
	pContext = NULL;
	pRoot = NULL;
	pA = NULL;
	pB = NULL;
	pC = NULL;
	pOverlay = NULL;
	iFailed = 0;

	iRet = xuiCreate(&pContext);
	XUI_TEST_CHECK((iRet == XUI_OK) && (pContext != NULL), "xuiCreate failed");
	iRet = xuiInputViewport(pContext, 200.0f, 100.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "input viewport failed");
	XUI_TEST_CHECK(__xuiTestPoll(pContext, XUI_EVENT_VIEWPORT, NULL, &tEvent) && (tEvent.fX == 200.0f) && (tEvent.fY == 100.0f), "viewport event failed");

	iRet = xuiWidgetCreate(pContext, &pRoot);
	XUI_TEST_CHECK((iRet == XUI_OK) && (pRoot != NULL), "root create failed");
	iRet = xuiWidgetCreate(pContext, &pA);
	XUI_TEST_CHECK((iRet == XUI_OK) && (pA != NULL), "A create failed");
	iRet = xuiWidgetCreate(pContext, &pB);
	XUI_TEST_CHECK((iRet == XUI_OK) && (pB != NULL), "B create failed");
	iRet = xuiSetRootWidget(pContext, pRoot);
	XUI_TEST_CHECK(iRet == XUI_OK, "set root failed");
	iRet = xuiWidgetSetRect(pA, (xui_rect_t){10.0f, 10.0f, 50.0f, 40.0f});
	XUI_TEST_CHECK(iRet == XUI_OK, "A rect failed");
	iRet = xuiWidgetSetRect(pB, (xui_rect_t){20.0f, 20.0f, 50.0f, 40.0f});
	XUI_TEST_CHECK(iRet == XUI_OK, "B rect failed");
	iRet = xuiWidgetSetFocusable(pA, 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "A focusable failed");
	iRet = xuiWidgetSetFocusable(pB, 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "B focusable failed");
	iRet = xuiWidgetAddChild(pRoot, pA);
	XUI_TEST_CHECK(iRet == XUI_OK, "add A failed");
	iRet = xuiWidgetAddChild(pRoot, pB);
	XUI_TEST_CHECK(iRet == XUI_OK, "add B failed");
	xuiClearEvents(pContext);

	XUI_TEST_CHECK(xuiHitTest(pContext, 25.0f, 25.0f, XUI_WIDGET_HIT_DEFAULT) == pB, "z-order hit test failed");
	iRet = xuiWidgetSetEventCallback(pRoot, __xuiTestEventCallback, &tLog);
	XUI_TEST_CHECK(iRet == XUI_OK, "root event callback set failed");
	iRet = xuiWidgetSetEventCallback(pB, __xuiTestEventCallback, &tLog);
	XUI_TEST_CHECK(iRet == XUI_OK, "B event callback set failed");
	iRet = xuiWidgetGetEventCallback(pB, &onEvent, &pEventUser);
	XUI_TEST_CHECK((iRet == XUI_OK) && (onEvent == __xuiTestEventCallback) && (pEventUser == &tLog), "event callback getter failed");
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iSize = sizeof(tEvent);
	tEvent.iType = XUI_EVENT_KEY_DOWN;
	tEvent.pTarget = pB;
	iRet = xuiDispatchEvent(pContext, &tEvent);
	XUI_TEST_CHECK(iRet == XUI_OK, "direct dispatch failed");
	XUI_TEST_CHECK((tLog.iCount == 3) &&
	               (tLog.arrWidget[0] == pRoot) && (tLog.arrPhase[0] == XUI_EVENT_PHASE_CAPTURE) &&
	               (tLog.arrWidget[1] == pB) && (tLog.arrPhase[1] == XUI_EVENT_PHASE_TARGET) &&
	               (tLog.arrWidget[2] == pRoot) && (tLog.arrPhase[2] == XUI_EVENT_PHASE_BUBBLE), "direct dispatch path failed");

	iRet = xuiInputPointerMove(pContext, 25.0f, 25.0f, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "pointer move failed");
	XUI_TEST_CHECK(xuiGetHoverWidget(pContext) == pB, "hover widget failed");
	XUI_TEST_CHECK((xuiWidgetGetInputState(pB) & XUI_WIDGET_STATE_HOVER) != 0, "hover state failed");
	XUI_TEST_CHECK(__xuiTestPoll(pContext, XUI_EVENT_POINTER_ENTER, pB, &tEvent), "enter event failed");
	XUI_TEST_CHECK(__xuiTestPoll(pContext, XUI_EVENT_POINTER_MOVE, pB, &tEvent), "move event failed");

	iRet = xuiInputPointerDown(pContext, 25.0f, 25.0f, XUI_POINTER_BUTTON_LEFT, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "pointer down failed");
	XUI_TEST_CHECK((xuiGetActiveWidget(pContext) == pB) && (xuiGetFocusWidget(pContext) == pB), "active/focus widget failed");
	XUI_TEST_CHECK((xuiWidgetGetInputState(pB) & (XUI_WIDGET_STATE_ACTIVE | XUI_WIDGET_STATE_FOCUS)) == (XUI_WIDGET_STATE_ACTIVE | XUI_WIDGET_STATE_FOCUS), "active/focus state failed");
	XUI_TEST_CHECK(__xuiTestPoll(pContext, XUI_EVENT_FOCUS, pB, &tEvent), "focus event failed");
	XUI_TEST_CHECK(__xuiTestPoll(pContext, XUI_EVENT_POINTER_DOWN, pB, &tEvent) && (tEvent.iButton == XUI_POINTER_BUTTON_LEFT), "down event failed");

	memset(&tLog, 0, sizeof(tLog));
	iRet = xuiInputKeyDown(pContext, 65, XUI_MOD_CTRL);
	XUI_TEST_CHECK(iRet == XUI_OK, "key down failed");
	XUI_TEST_CHECK(xuiInputGetModifiers(pContext) == XUI_MOD_CTRL, "input modifiers failed");
	XUI_TEST_CHECK((tLog.iCount == 3) &&
	               (tLog.arrType[1] == XUI_EVENT_KEY_DOWN) &&
	               (tLog.arrWidget[1] == pB) &&
	               (tLog.arrPhase[1] == XUI_EVENT_PHASE_TARGET) &&
	               (tLog.arrKey[1] == 65) &&
	               (tLog.arrModifiers[1] == XUI_MOD_CTRL), "key event failed");
	memset(&tLog, 0, sizeof(tLog));
	iRet = xuiInputText(pContext, 'A');
	XUI_TEST_CHECK(iRet == XUI_OK, "text input failed");
	XUI_TEST_CHECK((tLog.iCount == 3) &&
	               (tLog.arrType[1] == XUI_EVENT_TEXT) &&
	               (tLog.arrWidget[1] == pB) &&
	               (tLog.arrPhase[1] == XUI_EVENT_PHASE_TARGET) &&
	               (tLog.arrCodepoint[1] == 'A') &&
	               (tLog.arrModifiers[1] == XUI_MOD_CTRL), "text event failed");
	iRet = xuiInputSetModifiers(pContext, 0);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiInputGetModifiers(pContext) == 0u, "clear input modifiers failed");

	iRet = xuiInputPointerUp(pContext, 25.0f, 25.0f, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	XUI_TEST_CHECK(iRet == XUI_OK, "pointer up failed");
	XUI_TEST_CHECK(xuiGetActiveWidget(pContext) == NULL, "active should clear on up");
	XUI_TEST_CHECK(__xuiTestPoll(pContext, XUI_EVENT_POINTER_UP, pB, &tEvent), "up event failed");
	XUI_TEST_CHECK(__xuiTestPoll(pContext, XUI_EVENT_POINTER_CLICK, pB, &tEvent), "click event failed");

	iRet = xuiInputPointerMove(pContext, 12.0f, 12.0f, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "pointer move to A failed");
	XUI_TEST_CHECK(xuiGetHoverWidget(pContext) == pA, "hover A failed");
	XUI_TEST_CHECK(__xuiTestPoll(pContext, XUI_EVENT_POINTER_LEAVE, pB, &tEvent), "leave B failed");
	XUI_TEST_CHECK(__xuiTestPoll(pContext, XUI_EVENT_POINTER_ENTER, pA, &tEvent), "enter A failed");
	XUI_TEST_CHECK(__xuiTestPoll(pContext, XUI_EVENT_POINTER_MOVE, pA, &tEvent), "move A failed");

	iRet = xuiWidgetSetEnabled(pA, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "disable A failed");
	XUI_TEST_CHECK((xuiGetHoverWidget(pContext) == NULL) && ((xuiWidgetGetInputState(pA) & XUI_WIDGET_STATE_DISABLED) != 0), "disable should detach hover and set disabled state");
	XUI_TEST_CHECK(xuiHitTest(pContext, 12.0f, 12.0f, XUI_WIDGET_HIT_DEFAULT) != pA, "disabled widget should not hit");

	iRet = xuiInputDpi(pContext, 2.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "dpi input failed");
	XUI_TEST_CHECK(__xuiTestPoll(pContext, XUI_EVENT_DPI, NULL, &tEvent) && (tEvent.fX == 2.0f), "dpi event failed");

	iRet = xuiWidgetSetEnabled(pA, 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "reenable A failed");

	iRet = xuiWidgetSetTooltipText(pA, "A tooltip");
	XUI_TEST_CHECK(iRet == XUI_OK, "tooltip text set failed");
	XUI_TEST_CHECK((xuiWidgetGetTooltip(pA) != NULL) &&
	               (xuiWidgetGetTooltip(pA)->iType == XUI_TOOLTIP_TEXT) &&
	               (strcmp(xuiWidgetGetTooltip(pA)->sText, "A tooltip") == 0) &&
	               ((xuiWidgetGetEventMask(pA) & XUI_EVENT_MASK_TOOLTIP) != 0), "tooltip text getter/mask failed");
	iRet = xuiInputPointerMove(pContext, 12.0f, 12.0f, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "tooltip pointer move failed");
	iRet = xuiUpdate(pContext, 0.10f);
	XUI_TEST_CHECK((iRet == XUI_OK) && !xuiWidgetTooltipIsOpen(pContext), "tooltip should wait for delay");
	iRet = xuiUpdate(pContext, 0.30f);
	tRect = xuiWidgetTooltipGetRect(pContext);
	XUI_TEST_CHECK((iRet == XUI_OK) && xuiWidgetTooltipIsOpen(pContext) &&
	               (xuiWidgetTooltipGetOwner(pContext) == pA) &&
	               (tRect.fW >= 12.0f) && (tRect.fH >= 18.0f) &&
	               (tRect.fY >= 50.0f), "tooltip delayed open failed");
	iRet = xuiBuildRenderTree(pContext);
	XUI_TEST_CHECK((iRet == XUI_OK) &&
	               (xuiWidgetTooltipGetRect(pContext).fX == tRect.fX) &&
	               (xuiWidgetTooltipGetRect(pContext).fY == tRect.fY) &&
	               (xuiWidgetGetRect(xuiOverlayTop(pContext)).fX == tRect.fX) &&
	               (xuiWidgetGetRect(xuiOverlayTop(pContext)).fY == tRect.fY), "tooltip layout should keep resolved rect");
	iRet = xuiInputPointerDown(pContext, 12.0f, 12.0f, XUI_POINTER_BUTTON_LEFT, 0);
	XUI_TEST_CHECK((iRet == XUI_OK) && !xuiWidgetTooltipIsOpen(pContext), "tooltip should close on pointer down");
	iRet = xuiInputPointerUp(pContext, 12.0f, 12.0f, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	XUI_TEST_CHECK(iRet == XUI_OK, "tooltip pointer up cleanup failed");
	tTooltipResolver.sText = "Dynamic tooltip";
	tTooltipResolver.bEnabled = 1;
	iRet = xuiWidgetSetTooltipResolver(pB, __xuiTestTooltipResolve, &tTooltipResolver);
	XUI_TEST_CHECK(iRet == XUI_OK, "tooltip resolver set failed");
	iRet = xuiInputPointerMove(pContext, 25.0f, 25.0f, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "resolver tooltip move failed");
	iRet = xuiUpdate(pContext, 0.0f);
	tRect = xuiWidgetTooltipGetRect(pContext);
	XUI_TEST_CHECK((iRet == XUI_OK) && xuiWidgetTooltipIsOpen(pContext) &&
	               (xuiWidgetTooltipGetOwner(pContext) == pB) &&
	               (tRect.fX >= 25.0f) && (tRect.fY >= 25.0f) &&
	               (tTooltipResolver.iResolveCount >= 1), "resolver tooltip open failed");
	iRet = xuiInputPointerMove(pContext, 35.0f, 35.0f, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "follow tooltip move failed");
	iRet = xuiUpdate(pContext, 0.0f);
	tRect = xuiWidgetTooltipGetRect(pContext);
	XUI_TEST_CHECK((iRet == XUI_OK) && xuiWidgetTooltipIsOpen(pContext) &&
	               (tRect.fX >= 35.0f) && (tRect.fY >= 35.0f), "follow tooltip rect failed");
	tTooltipResolver.sText = "Changed tooltip";
	iRet = xuiUpdate(pContext, 0.0f);
	XUI_TEST_CHECK((iRet == XUI_OK) && xuiWidgetTooltipIsOpen(pContext) &&
	               (xuiWidgetTooltipGetOwner(pContext) == pB), "resolver tooltip refresh failed");
	iRet = xuiWidgetSetVisible(pB, 0);
	XUI_TEST_CHECK((iRet == XUI_OK) && !xuiWidgetTooltipIsOpen(pContext) &&
	               (xuiWidgetTooltipGetOwner(pContext) == NULL), "tooltip should close when owner hides");
	iRet = xuiWidgetSetVisible(pB, 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "tooltip owner show failed");
	iRet = xuiInputPointerMove(pContext, 25.0f, 25.0f, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "tooltip visible restore move failed");
	iRet = xuiUpdate(pContext, 0.0f);
	XUI_TEST_CHECK((iRet == XUI_OK) && xuiWidgetTooltipIsOpen(pContext) &&
	               (xuiWidgetTooltipGetOwner(pContext) == pB), "tooltip should reopen after owner shows");
	iRet = xuiWidgetSetEnabled(pB, 0);
	XUI_TEST_CHECK((iRet == XUI_OK) && !xuiWidgetTooltipIsOpen(pContext) &&
	               (xuiWidgetTooltipGetOwner(pContext) == NULL), "tooltip should close when owner disables");
	iRet = xuiWidgetSetEnabled(pB, 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "tooltip owner enable failed");
	tTooltipResolver.bEnabled = 0;
	iRet = xuiInputPointerMove(pContext, 25.0f, 25.0f, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "disabled resolver move failed");
	iRet = xuiUpdate(pContext, 0.0f);
	XUI_TEST_CHECK((iRet == XUI_OK) && !xuiWidgetTooltipIsOpen(pContext) &&
	               (xuiWidgetTooltipGetOwner(pContext) == NULL), "disabled resolver should not open tooltip");
	iRet = xuiWidgetClearTooltip(pB);
	XUI_TEST_CHECK((iRet == XUI_OK) && !xuiWidgetTooltipIsOpen(pContext) &&
	               ((xuiWidgetGetEventMask(pB) & XUI_EVENT_MASK_TOOLTIP) == 0), "tooltip clear failed");
	iRet = xuiWidgetClearTooltip(pA);
	XUI_TEST_CHECK((iRet == XUI_OK) && ((xuiWidgetGetEventMask(pA) & XUI_EVENT_MASK_TOOLTIP) == 0), "tooltip clear A failed");
	iRet = xuiSetFocusWidget(pContext, pB);
	XUI_TEST_CHECK((iRet == XUI_OK) && (xuiGetFocusWidget(pContext) == pB), "restore focus before capture failed");
	iRet = xuiInputPointerMove(pContext, 12.0f, 12.0f, 0);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiGetHoverWidget(pContext) == pA, "capture pre-hover setup failed");
	xuiClearEvents(pContext);
	iRet = xuiSetPointerCapture(pContext, pA);
	XUI_TEST_CHECK((iRet == XUI_OK) && (xuiGetPointerCapture(pContext) == pA), "pointer capture set failed");
	iRet = xuiInputPointerLeave(pContext);
	XUI_TEST_CHECK((iRet == XUI_OK) && (xuiGetPointerCapture(pContext) == pA), "pointer leave should keep capture");
	xuiClearEvents(pContext);
	iRet = xuiInputPointerMove(pContext, 25.0f, 25.0f, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "captured pointer move failed");
	XUI_TEST_CHECK(__xuiTestPoll(pContext, XUI_EVENT_POINTER_ENTER, pB, &tEvent), "captured move should still update hover hit");
	XUI_TEST_CHECK(__xuiTestPoll(pContext, XUI_EVENT_POINTER_MOVE, pA, &tEvent) && (tEvent.pRelated == pB), "captured move should target capture widget");
	iRet = xuiReleasePointerCapture(pContext, pB);
	XUI_TEST_CHECK((iRet == XUI_OK) && (xuiGetPointerCapture(pContext) == pA), "unrelated capture release should be ignored");
	iRet = xuiReleasePointerCapture(pContext, pA);
	XUI_TEST_CHECK((iRet == XUI_OK) && (xuiGetPointerCapture(pContext) == NULL), "pointer capture release failed");
	XUI_TEST_CHECK(__xuiTestPoll(pContext, XUI_EVENT_POINTER_CAPTURE_LOST, pA, &tEvent), "capture lost event failed");

	xuiClearEvents(pContext);
	memset(&tMulti, 0, sizeof(tMulti));
	tMulti.pA = pA;
	tMulti.pB = pB;
	iRet = xuiWidgetSetEventHandler(pA, XUI_EVENT_POINTER_DOWN, __xuiTestMultiPointerCallback, &tMulti);
	XUI_TEST_CHECK(iRet == XUI_OK, "multi pointer A down handler failed");
	iRet = xuiWidgetSetEventHandler(pA, XUI_EVENT_POINTER_MOVE, __xuiTestMultiPointerCallback, &tMulti);
	XUI_TEST_CHECK(iRet == XUI_OK, "multi pointer A move handler failed");
	iRet = xuiWidgetSetEventHandler(pA, XUI_EVENT_POINTER_UP, __xuiTestMultiPointerCallback, &tMulti);
	XUI_TEST_CHECK(iRet == XUI_OK, "multi pointer A up handler failed");
	iRet = xuiWidgetSetEventHandler(pA, XUI_EVENT_POINTER_CAPTURE_LOST, __xuiTestMultiPointerCallback, &tMulti);
	XUI_TEST_CHECK(iRet == XUI_OK, "multi pointer A capture handler failed");
	iRet = xuiWidgetSetEventHandler(pB, XUI_EVENT_POINTER_DOWN, __xuiTestMultiPointerCallback, &tMulti);
	XUI_TEST_CHECK(iRet == XUI_OK, "multi pointer B down handler failed");
	iRet = xuiWidgetSetEventHandler(pB, XUI_EVENT_POINTER_MOVE, __xuiTestMultiPointerCallback, &tMulti);
	XUI_TEST_CHECK(iRet == XUI_OK, "multi pointer B move handler failed");
	iRet = xuiWidgetSetEventHandler(pB, XUI_EVENT_POINTER_UP, __xuiTestMultiPointerCallback, &tMulti);
	XUI_TEST_CHECK(iRet == XUI_OK, "multi pointer B up handler failed");
	iRet = xuiWidgetSetEventHandler(pB, XUI_EVENT_POINTER_CAPTURE_LOST, __xuiTestMultiPointerCallback, &tMulti);
	XUI_TEST_CHECK(iRet == XUI_OK, "multi pointer B capture handler failed");
	iRet = xuiInputPointerDownEx(pContext, 101, XUI_POINTER_TYPE_TOUCH, 12.0f, 12.0f, XUI_POINTER_BUTTON_LEFT, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "touch A down failed");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "touch A down dispatch failed");
	XUI_TEST_CHECK((tMulti.iDownA == 1) && (tMulti.iLegacyCaptureOk == 1) &&
	               (xuiGetPointerCaptureEx(pContext, 101, XUI_POINTER_TYPE_TOUCH) == pA), "touch A capture failed");
	iRet = xuiInputPointerDownEx(pContext, 202, XUI_POINTER_TYPE_TOUCH, 65.0f, 55.0f, XUI_POINTER_BUTTON_LEFT, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "touch B down failed");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "touch B down dispatch failed");
	XUI_TEST_CHECK((tMulti.iDownB == 1) && (tMulti.iLegacyCaptureOk == 2) &&
	               (xuiGetPointerCaptureEx(pContext, 101, XUI_POINTER_TYPE_TOUCH) == pA) &&
	               (xuiGetPointerCaptureEx(pContext, 202, XUI_POINTER_TYPE_TOUCH) == pB), "independent touch capture failed");
	iRet = xuiInputPointerMoveEx(pContext, 101, XUI_POINTER_TYPE_TOUCH, 150.0f, 90.0f, XUI_POINTER_BUTTON_LEFT);
	XUI_TEST_CHECK(iRet == XUI_OK, "touch A captured move failed");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "touch A move dispatch failed");
	XUI_TEST_CHECK((tMulti.iMoveA >= 1) && (tMulti.iLastPointerId == 101) &&
	               (tMulti.iLastPointerType == XUI_POINTER_TYPE_TOUCH), "touch A move target/id failed");
	iRet = xuiInputPointerUpEx(pContext, 101, XUI_POINTER_TYPE_TOUCH, 150.0f, 90.0f, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	XUI_TEST_CHECK(iRet == XUI_OK, "touch A up failed");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "touch A up dispatch failed");
	XUI_TEST_CHECK((tMulti.iUpA == 1) && (tMulti.iCaptureLostA == 1) &&
	               (xuiGetPointerCaptureEx(pContext, 101, XUI_POINTER_TYPE_TOUCH) == NULL) &&
	               (xuiGetPointerCaptureEx(pContext, 202, XUI_POINTER_TYPE_TOUCH) == pB), "touch A release should not affect B");
	iRet = xuiInputPointerCancelEx(pContext, 202, XUI_POINTER_TYPE_TOUCH);
	XUI_TEST_CHECK(iRet == XUI_OK, "touch B cancel failed");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "touch B cancel dispatch failed");
	XUI_TEST_CHECK((tMulti.iCaptureLostB == 1) &&
	               (xuiGetPointerCaptureEx(pContext, 202, XUI_POINTER_TYPE_TOUCH) == NULL), "touch B cancel release failed");
	(void)xuiWidgetSetEventHandler(pA, XUI_EVENT_POINTER_DOWN, NULL, NULL);
	(void)xuiWidgetSetEventHandler(pA, XUI_EVENT_POINTER_MOVE, NULL, NULL);
	(void)xuiWidgetSetEventHandler(pA, XUI_EVENT_POINTER_UP, NULL, NULL);
	(void)xuiWidgetSetEventHandler(pA, XUI_EVENT_POINTER_CAPTURE_LOST, NULL, NULL);
	(void)xuiWidgetSetEventHandler(pB, XUI_EVENT_POINTER_DOWN, NULL, NULL);
	(void)xuiWidgetSetEventHandler(pB, XUI_EVENT_POINTER_MOVE, NULL, NULL);
	(void)xuiWidgetSetEventHandler(pB, XUI_EVENT_POINTER_UP, NULL, NULL);
	(void)xuiWidgetSetEventHandler(pB, XUI_EVENT_POINTER_CAPTURE_LOST, NULL, NULL);

	xuiClearEvents(pContext);
	iRet = xuiSetFocusWidget(pContext, pB);
	XUI_TEST_CHECK((iRet == XUI_OK) && (xuiGetFocusWidget(pContext) == pB), "restore focus before traversal failed");
	xuiClearEvents(pContext);
	iRet = xuiSetFocusWidget(pContext, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK, "clear focus before traversal failed");
	XUI_TEST_CHECK(__xuiTestPoll(pContext, XUI_EVENT_BLUR, pB, &tEvent), "clear focus before traversal event failed");
	iRet = xuiFocusNext(pContext, 1);
	XUI_TEST_CHECK((iRet == XUI_OK) && (xuiGetFocusWidget(pContext) == pA), "focus next should select A");
	XUI_TEST_CHECK(__xuiTestPoll(pContext, XUI_EVENT_FOCUS, pA, &tEvent), "focus next A event failed");
	iRet = xuiFocusNext(pContext, 1);
	XUI_TEST_CHECK((iRet == XUI_OK) && (xuiGetFocusWidget(pContext) == pB), "focus next should select B");
	XUI_TEST_CHECK(__xuiTestPoll(pContext, XUI_EVENT_BLUR, pA, &tEvent), "focus next blur A event failed");
	XUI_TEST_CHECK(__xuiTestPoll(pContext, XUI_EVENT_FOCUS, pB, &tEvent), "focus next B event failed");
	iRet = xuiFocusNext(pContext, 0);
	XUI_TEST_CHECK((iRet == XUI_OK) && (xuiGetFocusWidget(pContext) == pA), "focus previous should select A");
	XUI_TEST_CHECK(__xuiTestPoll(pContext, XUI_EVENT_BLUR, pB, &tEvent), "focus previous blur B event failed");
	XUI_TEST_CHECK(__xuiTestPoll(pContext, XUI_EVENT_FOCUS, pA, &tEvent), "focus previous A event failed");

	iRet = xuiSetFocusWidget(pContext, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK, "clear focus failed");
	XUI_TEST_CHECK(__xuiTestPoll(pContext, XUI_EVENT_BLUR, pA, &tEvent), "blur event failed");
	iRet = xuiSetFocusWidget(pContext, pB);
	XUI_TEST_CHECK(iRet == XUI_OK, "set focus B failed");
	XUI_TEST_CHECK(__xuiTestPoll(pContext, XUI_EVENT_FOCUS, pB, &tEvent), "focus B event failed");
	xuiClearEvents(pContext);
	memset(&tLog, 0, sizeof(tLog));
	iRet = xuiInputKeyDown(pContext, 66, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "queued dispatch key failed");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "dispatch pending failed");
	XUI_TEST_CHECK((tLog.iCount == 3) &&
	               (tLog.arrType[1] == XUI_EVENT_KEY_DOWN) &&
	               (tLog.arrWidget[1] == pB) &&
	               (tLog.arrPhase[1] == XUI_EVENT_PHASE_TARGET) &&
	               (tLog.arrKey[1] == 66), "queued dispatch path failed");

	iRet = xuiWidgetSetEventHandler(pB, XUI_EVENT_POINTER_DOUBLE_CLICK, __xuiTestExtendedEventCallback, &tExt);
	XUI_TEST_CHECK(iRet == XUI_OK, "double-click handler set failed");
	iRet = xuiWidgetSetEventHandler(pB, XUI_EVENT_CONTEXT_MENU, __xuiTestExtendedEventCallback, &tExt);
	XUI_TEST_CHECK(iRet == XUI_OK, "context menu handler set failed");
	iRet = xuiWidgetSetEventHandler(pB, XUI_EVENT_DRAG_BEGIN, __xuiTestExtendedEventCallback, &tExt);
	XUI_TEST_CHECK(iRet == XUI_OK, "drag begin handler set failed");
	iRet = xuiWidgetSetEventHandler(pB, XUI_EVENT_DRAG_MOVE, __xuiTestExtendedEventCallback, &tExt);
	XUI_TEST_CHECK(iRet == XUI_OK, "drag move handler set failed");
	iRet = xuiWidgetSetEventHandler(pB, XUI_EVENT_DRAG_END, __xuiTestExtendedEventCallback, &tExt);
	XUI_TEST_CHECK(iRet == XUI_OK, "drag end handler set failed");
	XUI_TEST_CHECK((xuiWidgetGetEventMask(pB) & XUI_EVENT_MASK_DOUBLE_CLICK) != 0, "event handler mask failed");
	XUI_TEST_CHECK((xuiWidgetGetSubtreeEventMask(pRoot) & XUI_EVENT_MASK_DOUBLE_CLICK) != 0, "subtree event mask failed");

	xuiClearEvents(pContext);
	memset(&tExt, 0, sizeof(tExt));
	iRet = xuiInputPointerDown(pContext, 65.0f, 55.0f, XUI_POINTER_BUTTON_LEFT, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "double click first down failed");
	iRet = xuiInputPointerUp(pContext, 65.0f, 55.0f, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	XUI_TEST_CHECK(iRet == XUI_OK, "double click first up failed");
	iRet = xuiInputPointerDown(pContext, 65.0f, 55.0f, XUI_POINTER_BUTTON_LEFT, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "double click second down failed");
	iRet = xuiInputPointerUp(pContext, 65.0f, 55.0f, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	XUI_TEST_CHECK(iRet == XUI_OK, "double click second up failed");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "double click dispatch failed");
	XUI_TEST_CHECK(tExt.iDoubleClickCount == 1, "double click handler failed");

	xuiClearEvents(pContext);
	memset(&tExt, 0, sizeof(tExt));
	iRet = xuiInputPointerDown(pContext, 65.0f, 55.0f, XUI_POINTER_BUTTON_RIGHT, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "context down failed");
	iRet = xuiInputPointerUp(pContext, 65.0f, 55.0f, XUI_POINTER_BUTTON_RIGHT, XUI_POINTER_BUTTON_RIGHT);
	XUI_TEST_CHECK(iRet == XUI_OK, "context up failed");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "context dispatch failed");
	XUI_TEST_CHECK(tExt.iContextMenuCount == 1, "context menu handler failed");

	xuiClearEvents(pContext);
	memset(&tExt, 0, sizeof(tExt));
	iRet = xuiWidgetSetDragEnabled(pB, 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "drag enable failed");
	iRet = xuiInputPointerDown(pContext, 30.0f, 30.0f, XUI_POINTER_BUTTON_LEFT, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "drag down failed");
	iRet = xuiInputPointerMove(pContext, 42.0f, 42.0f, XUI_POINTER_BUTTON_LEFT);
	XUI_TEST_CHECK(iRet == XUI_OK, "drag move failed");
	iRet = xuiInputPointerUp(pContext, 42.0f, 42.0f, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	XUI_TEST_CHECK(iRet == XUI_OK, "drag up failed");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "drag dispatch failed");
	XUI_TEST_CHECK((tExt.iDragBeginCount == 1) && (tExt.iDragMoveCount >= 1) && (tExt.iDragEndCount == 1), "drag events failed");

	iRet = xuiWidgetSetEventHandler(pB, XUI_EVENT_COMMAND, __xuiTestExtendedEventCallback, &tExt);
	XUI_TEST_CHECK(iRet == XUI_OK, "command handler set failed");
	iRet = xuiWidgetSetEventHandler(pB, XUI_EVENT_HOTKEY, __xuiTestExtendedEventCallback, &tExt);
	XUI_TEST_CHECK(iRet == XUI_OK, "hotkey handler set failed");
	iRet = xuiHotKeyRegisterCommand(pContext, pB, 'S', XUI_MOD_CTRL, 100, "save", NULL);
	XUI_TEST_CHECK(iRet == XUI_OK, "command hotkey register failed");
	iRet = xuiHotKeyRegister(pContext, pB, 'H', XUI_MOD_CTRL, NULL, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK, "hotkey register failed");
	xuiClearEvents(pContext);
	memset(&tExt, 0, sizeof(tExt));
	iRet = xuiInputKeyDown(pContext, 'S', XUI_MOD_CTRL);
	XUI_TEST_CHECK(iRet == XUI_OK, "command hotkey keydown failed");
	iRet = xuiInputKeyDown(pContext, 'H', XUI_MOD_CTRL);
	XUI_TEST_CHECK(iRet == XUI_OK, "hotkey keydown failed");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "hotkey dispatch failed");
	XUI_TEST_CHECK((tExt.iCommandCount == 1) && (tExt.iLastCommand == 100) &&
	               (strcmp(tExt.sLastCommand, "save") == 0) &&
	               (tExt.iHotkeyCount == 1), "hotkey/command events failed");

	iRet = xuiWidgetSetTabIndex(pA, 2);
	XUI_TEST_CHECK(iRet == XUI_OK, "A tab index failed");
	iRet = xuiWidgetSetTabIndex(pB, 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "B tab index failed");
	xuiClearEvents(pContext);
	iRet = xuiSetFocusWidget(pContext, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK, "clear focus before tab key failed");
	iRet = xuiInputKeyDown(pContext, XUI_KEY_TAB, 0);
	XUI_TEST_CHECK((iRet == XUI_OK) && (xuiGetFocusWidget(pContext) == pB), "tab should follow tab index");
	iRet = xuiInputKeyDown(pContext, XUI_KEY_TAB, XUI_MOD_SHIFT);
	XUI_TEST_CHECK((iRet == XUI_OK) && (xuiGetFocusWidget(pContext) == pA), "shift tab should go backward");

	iRet = xuiWidgetCreate(pContext, &pC);
	XUI_TEST_CHECK((iRet == XUI_OK) && (pC != NULL), "C create failed");
	iRet = xuiWidgetSetFocusable(pC, 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "C focusable failed");
	iRet = xuiWidgetAddChild(pA, pC);
	XUI_TEST_CHECK(iRet == XUI_OK, "add C failed");
	iRet = xuiWidgetSetFocusScope(pA, 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "focus scope set failed");
	iRet = xuiSetFocusWidget(pContext, pC);
	XUI_TEST_CHECK(iRet == XUI_OK, "focus C failed");
	iRet = xuiFocusNext(pContext, 1);
	XUI_TEST_CHECK((iRet == XUI_OK) && (xuiGetFocusWidget(pContext) == pA), "focus scope traversal failed");

	iRet = xuiWidgetSetDefaultAction(pRoot, __xuiTestActionCallback, &tExt.iDefaultCount);
	XUI_TEST_CHECK(iRet == XUI_OK, "default action set failed");
	iRet = xuiWidgetSetCancelAction(pRoot, __xuiTestActionCallback, &tExt.iCancelCount);
	XUI_TEST_CHECK(iRet == XUI_OK, "cancel action set failed");
	iRet = xuiSetFocusWidget(pContext, pB);
	XUI_TEST_CHECK(iRet == XUI_OK, "focus B before action failed");
	iRet = xuiInputKeyDown(pContext, XUI_KEY_ENTER, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "enter action key failed");
	iRet = xuiInputKeyDown(pContext, XUI_KEY_ESCAPE, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "escape action key failed");
	XUI_TEST_CHECK((tExt.iDefaultCount == 1) && (tExt.iCancelCount == 1), "default/cancel action failed");

	tImeRect = (xui_rect_t){11.0f, 12.0f, 13.0f, 14.0f};
	iRet = xuiWidgetSetImeMode(pB, XUI_IME_ENABLED);
	XUI_TEST_CHECK(iRet == XUI_OK, "IME mode set failed");
	iRet = xuiWidgetSetImeCandidateRect(pB, __xuiTestImeRectCallback, &tImeRect);
	XUI_TEST_CHECK(iRet == XUI_OK, "IME rect callback set failed");
	iRet = xuiSetFocusWidget(pContext, pB);
	XUI_TEST_CHECK(iRet == XUI_OK, "IME focus failed");
	tRect = xuiGetImeCandidateRect(pContext);
	XUI_TEST_CHECK(xuiHasImeCandidateRect(pContext) &&
	               (tRect.fX == 11.0f) && (tRect.fY == 12.0f) &&
	               (tRect.fW == 13.0f) && (tRect.fH == 14.0f), "IME rect failed");
	xuiClearEvents(pContext);
	iRet = xuiInputImeComposition(pContext, "abc", -1, 1, 2);
	XUI_TEST_CHECK(iRet == XUI_OK, "IME composition input failed");
	XUI_TEST_CHECK(__xuiTestPoll(pContext, XUI_EVENT_IME_COMPOSITION, pB, &tEvent) &&
	               (strcmp(tEvent.sText, "abc") == 0) &&
	               (tEvent.iCompositionStart == 1) &&
	               (tEvent.iCompositionLength == 2), "IME composition event failed");

	iRet = xuiWidgetCreate(pContext, &pOverlay);
	XUI_TEST_CHECK((iRet == XUI_OK) && (pOverlay != NULL), "overlay create failed");
	iRet = xuiWidgetSetPreferredSize(pOverlay, (xui_vec2_t){30.0f, 30.0f});
	XUI_TEST_CHECK(iRet == XUI_OK, "overlay preferred size failed");
	iRet = xuiOverlayAttach(pContext, pB, pOverlay, XUI_LAYER_POPUP, 10);
	XUI_TEST_CHECK(iRet == XUI_OK, "overlay attach failed");
	XUI_TEST_CHECK((xuiOverlayRoot(pContext) != NULL) &&
	               (xuiOverlayTop(pContext) == pOverlay) &&
	               (xuiOverlayGetOwner(pOverlay) == pB), "overlay root/top/owner failed");
	XUI_TEST_CHECK(xuiHitTest(pContext, 5.0f, 5.0f, XUI_WIDGET_HIT_DEFAULT) == pOverlay, "overlay hit failed");
	iRet = xuiOverlayBringToFront(pOverlay);
	XUI_TEST_CHECK(iRet == XUI_OK, "overlay bring front failed");
	iRet = xuiOverlayDetach(pOverlay);
	XUI_TEST_CHECK((iRet == XUI_OK) && (xuiOverlayGetOwner(pOverlay) == NULL), "overlay detach failed");
	xuiWidgetDestroy(pOverlay);
	pOverlay = NULL;

	iRet = xuiDebugWidgetInspect(pB, &tDebugInfo);
	XUI_TEST_CHECK((iRet == XUI_OK) && (tDebugInfo.pWidget == pB) &&
	               (tDebugInfo.bDragEnabled == 1) &&
	               ((tDebugInfo.iEventMask & XUI_EVENT_MASK_COMMAND) != 0), "debug inspect failed");
	XUI_TEST_CHECK(xuiDebugWidgetInspectAt(pContext, 25.0f, 25.0f, &tDebugInfo) == pB, "debug inspect at failed");
	iRet = xuiDebugWidgetTreeDump(pContext, NULL, sDebug, (int)sizeof(sDebug));
	XUI_TEST_CHECK((iRet == XUI_OK) && (strstr(sDebug, "root") != NULL), "debug tree dump failed");
	iRet = xuiDebugLayoutSnapshot(pContext, sDebug, (int)sizeof(sDebug));
	XUI_TEST_CHECK((iRet == XUI_OK) && (strstr(sDebug, "viewport") != NULL), "debug layout snapshot failed");
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iSize = sizeof(tEvent);
	tEvent.iType = XUI_EVENT_COMMAND;
	tEvent.pTarget = pB;
	iRet = xuiDebugEventTrace(pContext, &tEvent, sDebug, (int)sizeof(sDebug));
	XUI_TEST_CHECK((iRet == XUI_OK) && (strstr(sDebug, "event type=") != NULL), "debug event trace failed");

	iRet = xuiSetFocusWidget(pContext, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK, "clear focus before scrub failed");
	xuiClearEvents(pContext);
	iRet = xuiSetFocusWidget(pContext, pB);
	XUI_TEST_CHECK(iRet == XUI_OK, "set focus B before destroy failed");
	xuiWidgetDestroy(pB);
	pB = NULL;
	XUI_TEST_CHECK(__xuiTestPoll(pContext, XUI_EVENT_FOCUS, NULL, &tEvent), "destroy should scrub queued target");

cleanup:
	if ( pOverlay != NULL ) {
		xuiWidgetDestroy(pOverlay);
	}
	if ( pB != NULL ) {
		xuiWidgetDestroy(pB);
	}
	if ( pContext != NULL ) {
		xuiDestroy(pContext);
	}
	if ( iFailed ) {
		return 1;
	}
	printf("xui_input_test passed\n");
	return 0;
}
