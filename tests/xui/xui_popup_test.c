#include "xui.h"
#include "xui_test_proxy.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

#define XUI_TEST_CHECK(expr, msg) \
	do { \
		if ( !(expr) ) { \
			printf("xui_popup_test failed: %s\n", (msg)); \
			iFailed = 1; \
			goto cleanup; \
		} \
	} while ( 0 )

static int __xuiPopupNear(float fA, float fB)
{
	float fD;

	fD = fA - fB;
	if ( fD < 0.0f ) {
		fD = -fD;
	}
	return fD < 0.01f;
}

static void __xuiPopupChanged(xui_widget pWidget, int bOpen, void* pUser)
{
	int* pCount;

	(void)pWidget;
	(void)bOpen;
	pCount = (int*)pUser;
	if ( pCount != NULL ) {
		(*pCount)++;
	}
}

static void __xuiPopupDestroyChanged(xui_widget pWidget, int bOpen, void* pUser)
{
	int* pCount = (int*)pUser;
	(void)bOpen;
	if ( pCount != NULL ) (*pCount)++;
	xuiWidgetDestroy(pWidget);
}

static int __xuiPopupOwnerEvent(xui_widget pWidget, const xui_event_t* pEvent, void* pUser)
{
	int* pCount = (int*)pUser;
	(void)pWidget;
	if ( pEvent != NULL && pEvent->iType == XUI_EVENT_POINTER_DOWN &&
	     pEvent->iPhase == XUI_EVENT_PHASE_TARGET && pCount != NULL ) (*pCount)++;
	return XUI_OK;
}

static int __xuiPopupRender(xui_context pContext, xui_surface pTarget)
{
	xui_rect_i_t tFullRect;

	tFullRect = (xui_rect_i_t){0, 0, 360, 260};
	return xuiRender(pContext, pTarget, &tFullRect, 1);
}

static int __xuiPopupDispatchDown(xui_context pContext, float fX, float fY)
{
	int iRet;

	iRet = xuiInputPointerDown(pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet != XUI_OK ) return iRet;
	return xuiDispatchPendingEvents(pContext);
}

static int __xuiPopupDispatchUp(xui_context pContext, float fX, float fY)
{
	int iRet;

	iRet = xuiInputPointerUp(pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, 0);
	if ( iRet != XUI_OK ) return iRet;
	return xuiDispatchPendingEvents(pContext);
}

int main(void)
{
	xui_test_proxy_state_t tState;
	xui_context pContext;
	xui_widget pRoot;
	xui_widget pOwner;
	xui_widget pPopup;
	xui_widget pAutoPopup;
	xui_widget pPanel;
	xui_widget pScrollView;
	xui_widget pFrame;
	xui_widget pContent;
	xui_widget pChild;
	xui_surface pTarget;
	xui_popup_desc_t tDesc;
	xui_popup_desc_t tAutoDesc;
	xui_popup_desc_t tInvalidDesc;
	xui_rect_t tPopup;
	xui_rect_t tPopupChanged;
	xui_rect_t tViewport;
	xui_rect_t tOwner;
	float fOffsetX;
	float fOffsetY;
	int iChanged;
	int iOwnerDown;
	int iFailed;
	int iRet;

	pContext = NULL;
	pRoot = NULL;
	pOwner = NULL;
	pPopup = NULL;
	pAutoPopup = NULL;
	pPanel = NULL;
	pScrollView = NULL;
	pFrame = NULL;
	pContent = NULL;
	pChild = NULL;
	pTarget = NULL;
	iChanged = 0;
	iOwnerDown = 0;
	iFailed = 0;
	xuiTestProxyInit(&tState);

	iRet = xuiCreate(&pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && pContext != NULL, "context create");
	iRet = xuiSetProxy(pContext, &tState.tProxy);
	XUI_TEST_CHECK(iRet == XUI_OK, "proxy set");
	iRet = xuiInputViewport(pContext, 360.0f, 260.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "viewport");

	iRet = xuiWidgetCreate(pContext, &pRoot);
	XUI_TEST_CHECK(iRet == XUI_OK && pRoot != NULL, "root create");
	xuiWidgetSetRect(pRoot, (xui_rect_t){0.0f, 0.0f, 360.0f, 260.0f});
	iRet = xuiSetRootWidget(pContext, pRoot);
	XUI_TEST_CHECK(iRet == XUI_OK, "set root");

	iRet = xuiWidgetCreate(pContext, &pOwner);
	XUI_TEST_CHECK(iRet == XUI_OK && pOwner != NULL, "owner create");
	xuiWidgetSetRect(pOwner, (xui_rect_t){40.0f, 30.0f, 92.0f, 28.0f});
	xuiWidgetSetFocusable(pOwner, 1);
	iRet = xuiWidgetAddChild(pRoot, pOwner);
	XUI_TEST_CHECK(iRet == XUI_OK, "add owner");
	iRet = xuiWidgetSetEventCallback(pOwner, __xuiPopupOwnerEvent, &iOwnerDown);
	XUI_TEST_CHECK(iRet == XUI_OK, "owner event callback");

	memset(&tInvalidDesc, 0, sizeof(tInvalidDesc));
	tInvalidDesc.iSize = sizeof(tInvalidDesc);
	tInvalidDesc.fContentWidth = NAN;
	pPopup = pOwner;
	iRet = xuiPopupCreate(pContext, &pPopup, &tInvalidDesc);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && pPopup == NULL,
		"reject NaN descriptor and clear stale output");
	tInvalidDesc.fContentWidth = 0.0f;
	tInvalidDesc.iScrollbarMode = XUI_SCROLLBAR_MODE_COMPACT;
	tInvalidDesc.fScrollbarSize = 9.0f;
	iRet = xuiPopupCreate(pContext, &pPopup, &tInvalidDesc);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && pPopup == NULL, "reject oversized compact scrollbar descriptor");
	memset(&tInvalidDesc, 0, sizeof(tInvalidDesc));
	tInvalidDesc.iSize = sizeof(tInvalidDesc);
	tInvalidDesc.fPadding = XUI_LAYOUT_UNBOUNDED * 0.5f;
	tInvalidDesc.fBorderWidth = XUI_LAYOUT_UNBOUNDED * 0.5f;
	iRet = xuiPopupCreate(pContext, &pPopup, &tInvalidDesc);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && pPopup == NULL,
		"reject descriptor with overflowing combined metrics");

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.pOwner = pOwner;
	tDesc.fContentWidth = 120.0f;
	tDesc.fContentHeight = 260.0f;
	tDesc.fMaxWidth = 180.0f;
	tDesc.fMaxHeight = 140.0f;
	tDesc.fGap = 5.0f;
	tDesc.fMargin = 4.0f;
	tDesc.fPadding = 4.0f;
	tDesc.fBorderWidth = 1.0f;
	tDesc.iAnchor = XUI_POPUP_ANCHOR_BOTTOM_LEFT;
	tDesc.iDirection = XUI_POPUP_DIRECTION_RIGHT_DOWN;
	tDesc.iOutsidePolicy = XUI_POPUP_OUTSIDE_CLOSE;
	tDesc.iOwnerPolicy = XUI_POPUP_OWNER_PASSTHROUGH;
	tDesc.iEscapePolicy = XUI_POPUP_ESCAPE_CLOSE;
	tDesc.iFocusPolicy = XUI_POPUP_FOCUS_POPUP;
	tDesc.iScrollbarMode = XUI_SCROLLBAR_MODE_COMPACT;
	tDesc.fScrollbarSize = 8.0f;
	iRet = xuiPopupCreate(pContext, &pPopup, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pPopup != NULL, "popup create");
	iRet = xuiPopupSetChange(pPopup, __xuiPopupChanged, &iChanged);
	XUI_TEST_CHECK(iRet == XUI_OK, "change callback");

	pPanel = xuiPopupGetPanelWidget(pPopup);
	pScrollView = xuiPopupGetScrollViewWidget(pPopup);
	pFrame = xuiPopupGetFrameWidget(pPopup);
	pContent = xuiPopupGetContentWidget(pPopup);
	XUI_TEST_CHECK(pPanel != NULL && pScrollView != NULL && pFrame != NULL && pContent != NULL, "popup internals");
	XUI_TEST_CHECK(xuiWidgetGetParent(pScrollView) == pPanel && xuiWidgetGetParent(pContent) == xuiPopupGetViewportWidget(pPopup), "popup tree");

	iRet = xuiWidgetCreate(pContext, &pChild);
	XUI_TEST_CHECK(iRet == XUI_OK && pChild != NULL, "content child create");
	xuiWidgetSetRect(pChild, (xui_rect_t){8.0f, 210.0f, 80.0f, 30.0f});
	iRet = xuiWidgetAddChild(pContent, pChild);
	XUI_TEST_CHECK(iRet == XUI_OK, "add popup child");

	iRet = xuiTestSurfaceCreate(&tState, &pTarget, 360, 260, XUI_SURFACE_USAGE_TARGET);
	XUI_TEST_CHECK(iRet == XUI_OK && pTarget != NULL, "target create");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "initial layout");
	iRet = xuiSetFocusWidget(pContext, pOwner);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiGetFocusWidget(pContext) == pOwner, "focus owner");
	iRet = xuiPopupSetOpen(pPopup, 1);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiPopupIsOpen(pPopup), "open popup");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "open layout");
	iRet = xuiUpdate(pContext, 0.016f);
	XUI_TEST_CHECK(iRet == XUI_OK, "update");
	iRet = __xuiPopupRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK, "render");

	tOwner = xuiWidgetGetWorldRect(pOwner);
	tPopup = xuiPopupGetPopupRect(pPopup);
	XUI_TEST_CHECK(__xuiPopupNear(tPopup.fX, tOwner.fX) && __xuiPopupNear(tPopup.fY, tOwner.fY + tOwner.fH + 5.0f), "anchored below owner");
	XUI_TEST_CHECK(tPopup.fW > 120.0f && tPopup.fW < 150.0f && __xuiPopupNear(tPopup.fH, 140.0f), "outer size with vertical reserve");
	tViewport = xuiPopupGetViewportRect(pPopup);
	XUI_TEST_CHECK(__xuiPopupNear(tViewport.fW, 120.0f), "viewport width preserved");
	XUI_TEST_CHECK(xuiScrollFrameIsVScrollBarVisible(pFrame), "vertical bar visible");
	XUI_TEST_CHECK(!xuiScrollFrameIsHScrollBarVisible(pFrame), "horizontal bar not chained");
	XUI_TEST_CHECK(xuiGetFocusWidget(pContext) == pPopup, "popup focused");
	XUI_TEST_CHECK(xuiPopupSetAnchorRect(pPopup, (xui_rect_t){0, 0, -1, 20}) == XUI_ERROR_INVALID_ARGUMENT,
		"reject negative anchor width");
	XUI_TEST_CHECK(xuiPopupSetScrollbarStyle(pPopup, XUI_SCROLLBAR_MODE_COMPACT, 9.0f) == XUI_ERROR_INVALID_ARGUMENT,
		"reject oversized compact scrollbar API");
	iRet = xuiPopupSetMetrics(pPopup, 8.0f, 2.0f, 4.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "set open popup metrics");
	tPopupChanged = xuiPopupGetPopupRect(pPopup);
	XUI_TEST_CHECK(tPopupChanged.fW > tPopup.fW && __xuiPopupNear(tPopupChanged.fH, tPopup.fH),
		"open metrics immediately replace geometry");
	iRet = xuiPopupSetMetrics(pPopup, 4.0f, 1.0f, 4.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "restore popup metrics");
	tPopupChanged = xuiPopupGetPopupRect(pPopup);
	XUI_TEST_CHECK(__xuiPopupNear(tPopupChanged.fW, tPopup.fW) && __xuiPopupNear(tPopupChanged.fH, tPopup.fH),
		"restored metrics restore geometry");

	iRet = __xuiPopupDispatchDown(pContext, tOwner.fX + 5.0f, tOwner.fY + 5.0f);
	XUI_TEST_CHECK(iRet == XUI_OK && iOwnerDown == 1 && xuiPopupIsOpen(pPopup), "owner passthrough reaches owner");
	iRet = __xuiPopupDispatchUp(pContext, tOwner.fX + 5.0f, tOwner.fY + 5.0f);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiPopupIsOpen(pPopup), "owner passthrough release keeps popup open");

	iRet = xuiInputPointerWheel(pContext, tPopup.fX + 20.0f, tPopup.fY + 20.0f, 0.0f, -1.0f, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "wheel input");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "wheel dispatch");
	iRet = xuiPopupGetScroll(pPopup, &fOffsetX, &fOffsetY);
	XUI_TEST_CHECK(iRet == XUI_OK && __xuiPopupNear(fOffsetX, 0.0f) && fOffsetY > 0.0f, "popup scroll");

	iRet = __xuiPopupDispatchDown(pContext, 320.0f, 220.0f);
	XUI_TEST_CHECK(iRet == XUI_OK && !xuiPopupIsOpen(pPopup), "outside closes popup");
	XUI_TEST_CHECK(xuiGetFocusWidget(pContext) == pOwner, "focus restored");
	XUI_TEST_CHECK(iChanged >= 2, "change callback open close");

	iRet = xuiPopupSetOpen(pPopup, 1);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiPopupIsOpen(pPopup), "reopen popup");
	iRet = xuiPopupGetScroll(pPopup, &fOffsetX, &fOffsetY);
	XUI_TEST_CHECK(iRet == XUI_OK && __xuiPopupNear(fOffsetX, 0.0f) && fOffsetY > 0.0f,
		"generic popup preserves scroll across close and reopen");
	iRet = xuiInputKeyDown(pContext, XUI_KEY_ESCAPE, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "escape input");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && !xuiPopupIsOpen(pPopup), "escape closes popup");

	memset(&tAutoDesc, 0, sizeof(tAutoDesc));
	tAutoDesc.iSize = sizeof(tAutoDesc);
	tAutoDesc.pOwner = pOwner;
	tAutoDesc.iOutsidePolicy = XUI_POPUP_OUTSIDE_IGNORE;
	tAutoDesc.iOwnerPolicy = XUI_POPUP_OWNER_PASSTHROUGH;
	tAutoDesc.iEscapePolicy = XUI_POPUP_ESCAPE_CLOSE;
	tAutoDesc.iFocusPolicy = XUI_POPUP_FOCUS_NONE;
	iRet = xuiPopupCreate(pContext, &pAutoPopup, &tAutoDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pAutoPopup != NULL, "auto-size popup create");
	pContent = xuiPopupGetContentWidget(pAutoPopup);
	iRet = xuiWidgetSetRect(pContent, (xui_rect_t){0, 0, 90, 50});
	XUI_TEST_CHECK(iRet == XUI_OK, "auto-size initial content rect");
	iRet = xuiPopupGetContentSize(pAutoPopup, &fOffsetX, &fOffsetY);
	XUI_TEST_CHECK(iRet == XUI_OK && __xuiPopupNear(fOffsetX, 0.0f) && __xuiPopupNear(fOffsetY, 0.0f),
		"auto-size request remains zero");
	iRet = xuiPopupSetOpen(pAutoPopup, 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "open auto-size popup");
	tPopupChanged = xuiPopupGetContentRect(pAutoPopup);
	XUI_TEST_CHECK(__xuiPopupNear(tPopupChanged.fW, 90.0f) && __xuiPopupNear(tPopupChanged.fH, 50.0f),
		"auto-size resolves initial content rect");
	iRet = xuiWidgetSetRect(pContent, (xui_rect_t){0, 0, 150, 70});
	XUI_TEST_CHECK(iRet == XUI_OK, "auto-size changed content rect");
	iRet = xuiPopupApplyPlacement(pAutoPopup);
	XUI_TEST_CHECK(iRet == XUI_OK, "reapply auto-size placement");
	tPopupChanged = xuiPopupGetContentRect(pAutoPopup);
	XUI_TEST_CHECK(__xuiPopupNear(tPopupChanged.fW, 150.0f) && __xuiPopupNear(tPopupChanged.fH, 70.0f),
		"auto-size re-resolves changed content rect");
	iRet = xuiPopupGetContentSize(pAutoPopup, &fOffsetX, &fOffsetY);
	XUI_TEST_CHECK(iRet == XUI_OK && __xuiPopupNear(fOffsetX, 0.0f) && __xuiPopupNear(fOffsetY, 0.0f),
		"auto-size request stays declarative after placement");
	iRet = xuiPopupSetOpen(pAutoPopup, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "close auto-size popup");
	xuiWidgetDestroy(pAutoPopup);
	pAutoPopup = NULL;

	iRet = xuiPopupSetClosePolicy(pPopup, XUI_POPUP_OUTSIDE_IGNORE, XUI_POPUP_OWNER_CLOSE, XUI_POPUP_ESCAPE_CLOSE);
	XUI_TEST_CHECK(iRet == XUI_OK, "owner close policy");
	iRet = xuiPopupSetOpen(pPopup, 1);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiPopupIsOpen(pPopup), "reopen owner policy");
	tOwner = xuiWidgetGetWorldRect(pOwner);
	iRet = __xuiPopupDispatchDown(pContext, tOwner.fX + 5.0f, tOwner.fY + 5.0f);
	XUI_TEST_CHECK(iRet == XUI_OK && !xuiPopupIsOpen(pPopup), "owner closes popup");

	iRet = xuiPopupSetClosePolicy(pPopup, XUI_POPUP_OUTSIDE_CLOSE, XUI_POPUP_OWNER_PASSTHROUGH, XUI_POPUP_ESCAPE_CLOSE);
	XUI_TEST_CHECK(iRet == XUI_OK, "restore close policy");
	iRet = xuiPopupSetContentSize(pPopup, 100.0f, 80.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "small content size");
	iRet = xuiPopupSetMaxSize(pPopup, 0.0f, 0.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "clear max size");
	iRet = xuiPopupSetAnchorRect(pPopup, (xui_rect_t){50.0f, 220.0f, 80.0f, 30.0f});
	XUI_TEST_CHECK(iRet == XUI_OK, "fallback anchor");
	iRet = xuiPopupSetDirection(pPopup, XUI_POPUP_DIRECTION_RIGHT_DOWN);
	XUI_TEST_CHECK(iRet == XUI_OK, "fallback direction");
	iRet = xuiPopupSetOpen(pPopup, 1);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiPopupIsOpen(pPopup), "open fallback popup");
	tPopup = xuiPopupGetPopupRect(pPopup);
	XUI_TEST_CHECK(tPopup.fY < 220.0f, "vertical fallback");
	iRet = xuiPopupSetClosePolicy(pPopup, XUI_POPUP_OUTSIDE_IGNORE, XUI_POPUP_OWNER_TOGGLE, XUI_POPUP_ESCAPE_CLOSE);
	XUI_TEST_CHECK(iRet == XUI_OK, "owner toggle policy");
	tOwner = xuiWidgetGetWorldRect(pOwner);
	iRet = __xuiPopupDispatchDown(pContext, tOwner.fX + 5.0f, tOwner.fY + 5.0f);
	XUI_TEST_CHECK(iRet == XUI_OK && !xuiPopupIsOpen(pPopup) && iOwnerDown == 2,
		"owner toggle closes and reaches owner");
	iRet = __xuiPopupDispatchUp(pContext, tOwner.fX + 5.0f, tOwner.fY + 5.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "owner toggle release");
	iRet = xuiPopupSetOpen(pPopup, 1);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiPopupIsOpen(pPopup), "reopen after owner toggle");

	XUI_TEST_CHECK(xuiPopupSetMetrics(pPopup, XUI_LAYOUT_UNBOUNDED * 0.5f,
		XUI_LAYOUT_UNBOUNDED * 0.5f, 0.0f) == XUI_ERROR_INVALID_ARGUMENT,
		"reject API metrics whose combined geometry exceeds context limit");
	iRet = xuiPopupSetMargin(pPopup, 1000.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "set margin larger than viewport");
	iRet = xuiPopupSetMetrics(pPopup, 1000.0f, 1000.0f, 4.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "set metrics larger than viewport");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "layout clamped popup metrics");
	tPopup = xuiPopupGetPopupRect(pPopup);
	tViewport = xuiPopupGetViewportRect(pPopup);
	XUI_TEST_CHECK(tPopup.fX >= 0.0f && tPopup.fY >= 0.0f &&
		tPopup.fX + tPopup.fW <= 360.0f && tPopup.fY + tPopup.fH <= 260.0f,
		"oversized margin and metrics keep popup inside viewport");
	XUI_TEST_CHECK(tViewport.fW >= 1.0f && tViewport.fH >= 1.0f &&
		tViewport.fW <= tPopup.fW && tViewport.fH <= tPopup.fH,
		"oversized metrics keep a bounded content viewport");
	iRet = xuiPopupSetMargin(pPopup, 4.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "restore popup margin");
	iRet = xuiPopupSetMetrics(pPopup, 4.0f, 1.0f, 4.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "restore popup metrics after clamp test");

	iRet = xuiPopupSetClosePolicy(pPopup, XUI_POPUP_OUTSIDE_IGNORE, XUI_POPUP_OWNER_CLOSE, XUI_POPUP_ESCAPE_CLOSE);
	XUI_TEST_CHECK(iRet == XUI_OK, "destructive owner close policy");
	iRet = xuiPopupSetChange(pPopup, __xuiPopupDestroyChanged, &iChanged);
	XUI_TEST_CHECK(iRet == XUI_OK, "destructive change callback");
	tOwner = xuiWidgetGetWorldRect(pOwner);
	iRet = __xuiPopupDispatchDown(pContext, tOwner.fX + 5.0f, tOwner.fY + 5.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "owner close callback may destroy popup");
	pPopup = NULL;

cleanup:
	if ( pContext != NULL ) {
		xuiDestroy(pContext);
	}
	if ( iFailed ) {
		return 1;
	}
	printf("xui_popup_test passed\n");
	return 0;
}
