#include "xui.h"
#include "xge.h"
#include "xui_test_proxy.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

#define XUI_TEST_CHECK(expr, msg) \
	do { \
		if ( !(expr) ) { \
			printf("xui_split_layout_test failed: %s\n", (msg)); \
			iFailed = 1; \
			goto cleanup; \
		} \
	} while ( 0 )

static int __xuiSplitNear(float fA, float fB)
{
	float fD;

	fD = fA - fB;
	if ( fD < 0.0f ) {
		fD = -fD;
	}
	return fD < 0.05f;
}

static void __xuiSplitChanged(xui_widget pWidget, int iDivider, void* pUser)
{
	int* pCount;

	(void)pWidget;
	(void)iDivider;
	pCount = (int*)pUser;
	if ( pCount != NULL ) {
		(*pCount)++;
	}
}

typedef struct xui_split_destroy_state_t {
	xui_widget* ppWidget;
	int iCount;
} xui_split_destroy_state_t;

typedef struct xui_split_destroy_context_state_t {
	xui_context* ppContext;
	int iCount;
} xui_split_destroy_context_state_t;

static void __xuiSplitDestroyOnChange(xui_widget pWidget, int iDivider, void* pUser)
{
	xui_split_destroy_state_t* pState;

	(void)iDivider;
	pState = (xui_split_destroy_state_t*)pUser;
	if ( pState == NULL ) return;
	pState->iCount++;
	if ( pState->ppWidget != NULL && *pState->ppWidget == pWidget ) {
		*pState->ppWidget = NULL;
		xuiWidgetDestroy(pWidget);
	}
}

static void __xuiSplitDestroyContextOnChange(xui_widget pWidget, int iDivider, void* pUser)
{
	xui_split_destroy_context_state_t* pState;
	xui_context pContext;

	(void)iDivider;
	pState = (xui_split_destroy_context_state_t*)pUser;
	if ( pState == NULL ) return;
	pState->iCount++;
	pContext = xuiWidgetGetContext(pWidget);
	if ( pState->ppContext != NULL && *pState->ppContext == pContext ) {
		*pState->ppContext = NULL;
		xuiDestroy(pContext);
	}
}

static int __xuiSplitPointerDown(xui_widget pWidget, const xui_event_t* pEvent, void* pUser)
{
	uint32_t* pModifiers;

	(void)pWidget;
	pModifiers = (uint32_t*)pUser;
	if ( (pModifiers != NULL) && (pEvent != NULL) ) {
		*pModifiers = pEvent->iModifiers;
	}
	return XUI_OK;
}

static int __xuiSplitRender(xui_context pContext, xui_surface pTarget)
{
	xui_rect_i_t tFullRect;

	tFullRect = (xui_rect_i_t){0, 0, 640, 360};
	return xuiRender(pContext, pTarget, &tFullRect, 1);
}

static int __xuiSplitDispatchLayoutRender(xui_context pContext, xui_surface pTarget)
{
	int iRet;

	iRet = xuiLayout(pContext);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiUpdate(pContext, 0.016f);
	if ( iRet != XUI_OK ) return iRet;
	return __xuiSplitRender(pContext, pTarget);
}

static int __xuiSplitPumpXgePointer(xui_context pContext, int iType,
	float fX, float fY, uint32_t iButtons, uint32_t iModifiers)
{
	xge_input_event_t tEvent;
	int iRet;

	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iSize = sizeof(tEvent);
	tEvent.iType = iType;
	tEvent.iButton = XGE_MOUSE_LEFT;
	tEvent.iButtons = iButtons;
	tEvent.iModifiers = iModifiers;
	tEvent.fX = fX;
	tEvent.fY = fY;
	iRet = xgeInputEventPost(&tEvent);
	if ( iRet != XGE_OK ) return iRet;
	iRet = xuiProxyXgePumpInput(pContext);
	if ( iRet != XUI_OK ) return iRet;
	return xuiDispatchPendingEvents(pContext);
}

int main(void)
{
	xui_test_proxy_state_t tState;
	xui_context pContext;
	xui_widget pRoot;
	xui_widget pSplit;
	xui_widget pDock;
	xui_widget pTabs;
	xui_widget pOffsetParent;
	xui_widget pNestedSplit;
	xui_widget pMultiSplit;
	xui_widget pDestroySplit;
	xui_widget pInvalidSplit;
	xui_widget pShadowOverlay;
	xui_widget pCompetingOverlay;
	xui_widget pDividerWidget;
	xui_surface pTarget;
	xui_surface pShadowCache;
	xui_surface pDividerCache;
	xui_split_layout_desc_t tDesc;
	xui_event_t tEvent;
	xui_accessible_node_t tAccessibleNode;
	xui_accessible_value_t tAccessibleValue;
	xui_dock_panel_desc_t tDockDesc;
	xui_tabs_desc_t tTabsDesc;
	xge_desc_t tXgeDesc;
	xui_rect_t tPane0;
	xui_rect_t tPane1;
	xui_rect_t tPane2;
	xui_rect_t tDivider;
	xui_rect_t tVisual;
	xui_rect_t tHit;
	xui_rect_t tShadow;
	xui_rect_t tWorld;
	float fLayoutSize;
	float fVisualSize;
	float fHitSize;
	float fDragBefore;
	float fDragDelta;
	float fExpectedSize;
	float fX;
	float fY;
	int iChanged;
	int iMultiChanged;
	int iDockWindow;
	int iDockPane;
	int bXgeInitialized;
	int iFailed;
	int iRet;
	uint32_t iPointerModifiers;
	xui_split_destroy_state_t tDestroyState;
	xui_split_destroy_context_state_t tDestroyContextState;
	const char* arrTabItems[1];

	pContext = NULL;
	pRoot = NULL;
	pSplit = NULL;
	pDock = NULL;
	pTabs = NULL;
	pOffsetParent = NULL;
	pNestedSplit = NULL;
	pMultiSplit = NULL;
	pDestroySplit = NULL;
	pInvalidSplit = NULL;
	pShadowOverlay = NULL;
	pCompetingOverlay = NULL;
	pDividerWidget = NULL;
	pTarget = NULL;
	pShadowCache = NULL;
	pDividerCache = NULL;
	iChanged = 0;
	iMultiChanged = 0;
	iPointerModifiers = 0u;
	bXgeInitialized = 0;
	iFailed = 0;
	memset(&tDestroyState, 0, sizeof(tDestroyState));
	memset(&tDestroyContextState, 0, sizeof(tDestroyContextState));
	xuiTestProxyInit(&tState);

	iRet = xuiCreate(&pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && pContext != NULL, "context create");
	iRet = xuiSetProxy(pContext, &tState.tProxy);
	XUI_TEST_CHECK(iRet == XUI_OK, "proxy set");
	iRet = xuiInputViewport(pContext, 640.0f, 360.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "viewport");

	iRet = xuiWidgetCreate(pContext, &pRoot);
	XUI_TEST_CHECK(iRet == XUI_OK && pRoot != NULL, "root create");
	xuiWidgetSetRect(pRoot, (xui_rect_t){0.0f, 0.0f, 640.0f, 360.0f});
	iRet = xuiSetRootWidget(pContext, pRoot);
	XUI_TEST_CHECK(iRet == XUI_OK, "set root");

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.iPaneCount = 2;
	tDesc.fDividerSize = NAN;
	pInvalidSplit = pRoot;
	iRet = xuiSplitLayoutCreate(pContext, &pInvalidSplit, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && pInvalidSplit == NULL,
	               "invalid descriptor rejects NaN and clears output");

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.iOrientation = XUI_ORIENTATION_VERTICAL;
	tDesc.iPaneCount = 3;
	tDesc.fDividerSize = 10.0f;
	tDesc.fDividerVisualSize = 3.0f;
	tDesc.fDividerHitSize = 14.0f;
	iRet = xuiSplitLayoutCreate(pContext, &pSplit, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pSplit != NULL, "split create");
	iRet = xuiWidgetAddChild(pRoot, pSplit);
	XUI_TEST_CHECK(iRet == XUI_OK, "add split");
	xuiWidgetSetRect(pSplit, (xui_rect_t){20.0f, 20.0f, 460.0f, 180.0f});
	iRet = xuiSplitLayoutSetChange(pSplit, __xuiSplitChanged, &iChanged);
	XUI_TEST_CHECK(iRet == XUI_OK, "set change");

	iRet = xuiSplitLayoutSetPaneMode(pSplit, 0, XUI_SPLIT_PANE_FIXED);
	XUI_TEST_CHECK(iRet == XUI_OK, "left fixed");
	iRet = xuiSplitLayoutSetPaneFixedSize(pSplit, 0, 120.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "left fixed size");
	iRet = xuiSplitLayoutSetPaneMinSize(pSplit, 1, 100.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "center min");
	iRet = xuiSplitLayoutSetPaneMode(pSplit, 2, XUI_SPLIT_PANE_FIXED);
	XUI_TEST_CHECK(iRet == XUI_OK, "right fixed");
	iRet = xuiSplitLayoutSetPaneFixedSize(pSplit, 2, 130.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "right fixed size");

	iRet = xuiTestSurfaceCreate(&tState, &pTarget, 640, 360, XUI_SURFACE_USAGE_TARGET);
	XUI_TEST_CHECK(iRet == XUI_OK && pTarget != NULL, "target create");
	iRet = __xuiSplitDispatchLayoutRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK, "initial render");

	XUI_TEST_CHECK(xuiSplitLayoutGetPaneCount(pSplit) == 3, "pane count");
	XUI_TEST_CHECK(xuiSplitLayoutGetPaneWidget(pSplit, 0) != NULL, "pane widget 0");
	XUI_TEST_CHECK(xuiSplitLayoutGetPaneWidget(pSplit, 1) != NULL, "pane widget 1");
	XUI_TEST_CHECK(xuiSplitLayoutGetPaneWidget(pSplit, 2) != NULL, "pane widget 2");
	XUI_TEST_CHECK(xuiSplitLayoutGetOrientation(pSplit) == XUI_ORIENTATION_VERTICAL, "vertical orientation");
	iRet = xuiSplitLayoutGetDividerMetrics(pSplit, &fLayoutSize, &fVisualSize, &fHitSize);
	XUI_TEST_CHECK(iRet == XUI_OK && fLayoutSize == 10.0f && fVisualSize == 3.0f && fHitSize == 14.0f, "metrics");

	iRet = xuiSplitLayoutSetPaneWeight(pSplit, 1, NAN);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && xuiSplitLayoutGetPaneWeight(pSplit, 1) == 1.0f,
	               "pane weight rejects NaN without mutation");
	iRet = xuiSplitLayoutSetPaneFixedSize(pSplit, 0, INFINITY);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && xuiSplitLayoutGetPaneFixedSize(pSplit, 0) == 120.0f,
	               "fixed size rejects infinity without mutation");
	iRet = xuiSplitLayoutSetPaneMinSize(pSplit, 1, NAN);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && xuiSplitLayoutGetPaneMinSize(pSplit, 1) == 100.0f,
	               "minimum rejects NaN without mutation");
	iRet = xuiSplitLayoutSetPaneMaxSize(pSplit, 1, 50.0f);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && xuiSplitLayoutGetPaneMaxSize(pSplit, 1) == 0.0f,
	               "maximum cannot be smaller than minimum");
	iRet = xuiSplitLayoutSetPaneMaxSize(pSplit, 1, 200.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "set bounded maximum");
	iRet = xuiSplitLayoutSetPaneMinSize(pSplit, 1, 220.0f);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && xuiSplitLayoutGetPaneMinSize(pSplit, 1) == 100.0f,
	               "minimum cannot exceed maximum");
	iRet = xuiSplitLayoutSetPaneMaxSize(pSplit, 1, 0.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "restore unbounded maximum");
	iRet = xuiSplitLayoutSetDividerMetrics(pSplit, 10.0f, NAN, 14.0f);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT, "divider metrics reject NaN");
	iRet = xuiSplitLayoutSetDividerSize(pSplit, INFINITY);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT, "divider layout size rejects infinity");
	iRet = xuiSplitLayoutSetDividerVisualSize(pSplit, NAN);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT, "divider visual size rejects NaN");
	iRet = xuiSplitLayoutSetDividerHitSize(pSplit, INFINITY);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT, "divider hit size rejects infinity");
	iRet = xuiSplitLayoutGetDividerMetrics(pSplit, &fLayoutSize, &fVisualSize, &fHitSize);
	XUI_TEST_CHECK(iRet == XUI_OK && fLayoutSize == 10.0f && fVisualSize == 3.0f && fHitSize == 14.0f,
	               "invalid divider metrics leave state unchanged");
	iRet = xuiSplitLayoutSetPaneCount(pSplit, 4);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiSplitLayoutGetPaneCount(pSplit) == 4 &&
	               xuiSplitLayoutGetPaneWidget(pSplit, 3) != NULL, "grow pane count");
	iRet = xuiSplitLayoutSetPaneMode(pSplit, 3, XUI_SPLIT_PANE_FIXED);
	if ( iRet == XUI_OK ) iRet = xuiSplitLayoutSetPaneFixedSize(pSplit, 3, 77.0f);
	if ( iRet == XUI_OK ) iRet = xuiSplitLayoutSetPaneMaxSize(pSplit, 3, 90.0f);
	if ( iRet == XUI_OK ) iRet = xuiSplitLayoutSetPaneMinSize(pSplit, 3, 70.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "customize removable pane");
	iRet = xuiSplitLayoutSetPaneCount(pSplit, 3);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiSplitLayoutGetPaneCount(pSplit) == 3,
	               "shrink pane count");
	iRet = xuiSplitLayoutSetPaneCount(pSplit, 4);
	XUI_TEST_CHECK(iRet == XUI_OK &&
		xuiSplitLayoutGetPaneMode(pSplit, 3) == XUI_SPLIT_PANE_GROW &&
		xuiSplitLayoutGetPaneWeight(pSplit, 3) == 1.0f &&
		xuiSplitLayoutGetPaneFixedSize(pSplit, 3) == 160.0f &&
		xuiSplitLayoutGetPaneMinSize(pSplit, 3) == 64.0f &&
		xuiSplitLayoutGetPaneMaxSize(pSplit, 3) == 0.0f,
		"regrown pane starts with defaults instead of removed state");
	iRet = xuiSplitLayoutSetPaneCount(pSplit, 3);
	XUI_TEST_CHECK(iRet == XUI_OK, "restore pane count after reset regression");

	tPane0 = xuiSplitLayoutGetPaneRect(pSplit, 0);
	tPane1 = xuiSplitLayoutGetPaneRect(pSplit, 1);
	tPane2 = xuiSplitLayoutGetPaneRect(pSplit, 2);
	tDivider = xuiSplitLayoutGetDividerLayoutRect(pSplit, 0);
	tVisual = xuiSplitLayoutGetDividerVisualRect(pSplit, 0);
	tHit = xuiSplitLayoutGetDividerHitRect(pSplit, 0);
	XUI_TEST_CHECK(__xuiSplitNear(tPane0.fW, 120.0f), "left size");
	XUI_TEST_CHECK(__xuiSplitNear(tPane1.fW, 190.0f), "center grow size");
	XUI_TEST_CHECK(__xuiSplitNear(tPane2.fW, 130.0f), "right size");
	XUI_TEST_CHECK(__xuiSplitNear(tDivider.fX, 120.0f) && __xuiSplitNear(tDivider.fW, 10.0f), "divider layout rect");
	XUI_TEST_CHECK(__xuiSplitNear(tVisual.fW, 3.0f) && tHit.fW > tVisual.fW, "visual hit separation");

	tWorld = xuiWidgetGetWorldRect(pSplit);
	fX = tWorld.fX + tHit.fX + 1.0f;
	fY = tWorld.fY + tHit.fY + tHit.fH * 0.5f;
	XUI_TEST_CHECK(fX < tWorld.fX + tDivider.fX, "expanded hit point is outside divider layout rect");
	iRet = xuiInputPointerMove(pContext, fX, fY, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "hover input");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiSplitLayoutGetHoverDivider(pSplit) == 0, "hover divider");
	iRet = xuiInputPointerDown(pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	XUI_TEST_CHECK(iRet == XUI_OK, "shadow down input");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiSplitLayoutGetActiveDivider(pSplit) == 0, "active divider");
	pShadowOverlay = xuiOverlayTop(pContext);
	XUI_TEST_CHECK(pShadowOverlay != NULL && xuiOverlayGetOwner(pShadowOverlay) == pSplit,
	               "shadow overlay created on pointer down");
	iRet = xuiWidgetCreate(pContext, &pCompetingOverlay);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetRect(pCompetingOverlay, (xui_rect_t){0.0f, 0.0f, 1.0f, 1.0f});
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetHitTestVisible(pCompetingOverlay, 0);
	if ( iRet == XUI_OK ) iRet = xuiOverlayAttach(pContext, NULL, pCompetingOverlay, XUI_LAYER_DRAG, 100);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiOverlayTop(pContext) == pCompetingOverlay,
	               "competing overlay raised after shadow");
	iRet = xuiInputPointerMove(pContext, fX + 30.0f, fY, XUI_POINTER_BUTTON_LEFT);
	XUI_TEST_CHECK(iRet == XUI_OK, "shadow move input");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && iChanged == 0, "shadow move no commit");
	XUI_TEST_CHECK(xuiOverlayTop(pContext) != NULL &&
	               xuiOverlayGetOwner(xuiOverlayTop(pContext)) == pSplit &&
	               xuiWidgetGetVisible(xuiOverlayTop(pContext)) != 0, "shadow overlay visible");
	tShadow = xuiSplitLayoutGetShadowRect(pSplit);
	XUI_TEST_CHECK(tShadow.fW >= 3.0f && tShadow.fH >= 170.0f, "shadow rect");
	XUI_TEST_CHECK(xuiOverlayTop(pContext) == pShadowOverlay,
	               "shadow update raises the same overlay without detach");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "layout moving shadow overlay");
	tVisual = xuiWidgetGetWorldRect(pShadowOverlay);
	XUI_TEST_CHECK(__xuiSplitNear(tVisual.fX, tShadow.fX - 1.0f) &&
	               __xuiSplitNear(tVisual.fY, tShadow.fY - 1.0f) &&
	               __xuiSplitNear(tVisual.fW, tShadow.fW + 2.0f) &&
	               __xuiSplitNear(tVisual.fH, tShadow.fH + 2.0f),
	               "shadow overlay bounds follow the split divider world rect");
	iRet = __xuiSplitRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK, "render visible shadow");
	pShadowCache = xuiWidgetGetCacheSurface(pShadowOverlay, xuiWidgetGetStateId(pShadowOverlay));
	XUI_TEST_CHECK(pShadowCache != NULL && xuiTestSurfaceGetRectFillCount(pShadowCache) > 0,
	               "shadow cache contains visible primitive");
	iRet = xuiInputPointerMove(pContext, fX + 36.0f, fY, XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiOverlayTop(pContext) == pShadowOverlay,
	               "shadow update reuses overlay without detach");
	iRet = xuiInputPointerUp(pContext, fX + 30.0f, fY, XUI_POINTER_BUTTON_LEFT, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "shadow up input");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiGetPointerCapture(pContext) == NULL && xuiSplitLayoutGetActiveDivider(pSplit) == -1, "shadow release");
	iRet = __xuiSplitDispatchLayoutRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK, "render after shadow commit");
	XUI_TEST_CHECK(iChanged == 1, "shadow commit callback");
	XUI_TEST_CHECK(__xuiSplitNear(xuiSplitLayoutGetPaneFixedSize(pSplit, 0), 150.0f), "shadow commit fixed size");
	for ( pDividerWidget = xuiWidgetGetFirstChild(pSplit); pDividerWidget != NULL; pDividerWidget = xuiWidgetGetNextSibling(pDividerWidget) ) {
		int iLayer = 0;
		int iZIndex = 0;
		if ( xuiWidgetGetLayer(pDividerWidget, &iLayer, &iZIndex) == XUI_OK &&
		     iLayer == XUI_LAYER_NORMAL && iZIndex == 10 ) {
			break;
		}
	}
	XUI_TEST_CHECK(xuiSplitLayoutGetHoverDivider(pSplit) == 0 && pDividerWidget != NULL &&
	               xuiWidgetGetStateId(pDividerWidget) == (XUI_WIDGET_STATE_HOVER | XUI_WIDGET_STATE_FOCUS),
	               "release preserves focused hover divider state");
	pDividerCache = xuiWidgetGetCacheSurface(pDividerWidget, xuiWidgetGetStateId(pDividerWidget));
	XUI_TEST_CHECK(pDividerCache != NULL && xuiTestSurfaceGetRectFillCount(pDividerCache) > 0,
	               "focused hover divider remains visible after drag");

	tHit = xuiSplitLayoutGetDividerHitRect(pSplit, 1);
	tWorld = xuiWidgetGetWorldRect(pSplit);
	fX = tWorld.fX + tHit.fX + tHit.fW * 0.5f;
	fY = tWorld.fY + tHit.fY + tHit.fH * 0.5f;
	iRet = xuiInputPointerDown(pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	XUI_TEST_CHECK(iRet == XUI_OK, "second down input");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiSplitLayoutGetActiveDivider(pSplit) == 1, "second active");
	iRet = xuiInputPointerMove(pContext, fX - 20.0f, fY, XUI_POINTER_BUTTON_LEFT);
	XUI_TEST_CHECK(iRet == XUI_OK, "second move input");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiSplitLayoutGetChangeCount(pSplit) == 1 && iChanged == 1, "second move remains deferred");
	iRet = xuiInputKeyDown(pContext, XUI_KEY_ESCAPE, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "escape input");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiGetPointerCapture(pContext) == NULL && xuiSplitLayoutGetActiveDivider(pSplit) == -1, "escape cancels deferred drag");
	(void)xuiInputPointerUp(pContext, fX - 20.0f, fY, XUI_POINTER_BUTTON_LEFT, 0);
	(void)xuiDispatchPendingEvents(pContext);

	iRet = xuiSplitLayoutSetOrientation(pSplit, XUI_ORIENTATION_HORIZONTAL);
	XUI_TEST_CHECK(iRet == XUI_OK, "horizontal set");
	iRet = xuiSplitLayoutSetPaneFixedSize(pSplit, 0, 50.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "horizontal top fixed size");
	iRet = xuiSplitLayoutSetPaneMinSize(pSplit, 0, 20.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "horizontal top min size");
	iRet = xuiSplitLayoutSetPaneMinSize(pSplit, 1, 30.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "horizontal center min size");
	iRet = xuiSplitLayoutSetPaneFixedSize(pSplit, 2, 60.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "horizontal bottom fixed size");
	iRet = xuiSplitLayoutSetPaneMinSize(pSplit, 2, 20.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "horizontal bottom min size");
	xuiWidgetSetRect(pSplit, (xui_rect_t){20.0f, 20.0f, 260.0f, 160.0f});
	iRet = __xuiSplitDispatchLayoutRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK, "horizontal render");
	tPane0 = xuiSplitLayoutGetPaneRect(pSplit, 0);
	tDivider = xuiSplitLayoutGetDividerLayoutRect(pSplit, 0);
	XUI_TEST_CHECK(tPane0.fH > 0.0f && tPane0.fW > 250.0f, "horizontal pane spans width");
	XUI_TEST_CHECK(__xuiSplitNear(tDivider.fY, tPane0.fY + tPane0.fH), "horizontal divider y");

	xuiWidgetSetRect(pSplit, (xui_rect_t){20.0f, 20.0f, 170.0f, 100.0f});
	iRet = __xuiSplitDispatchLayoutRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK, "oversubscribed render");
	XUI_TEST_CHECK(xuiSplitLayoutGetPaneSize(pSplit, 0) >= 0.0f, "oversub pane 0 nonnegative");
	XUI_TEST_CHECK(xuiSplitLayoutGetPaneSize(pSplit, 1) >= 0.0f, "oversub pane 1 nonnegative");
	XUI_TEST_CHECK(xuiSplitLayoutGetPaneSize(pSplit, 2) >= 0.0f, "oversub pane 2 nonnegative");
	iRet = xuiSplitLayoutSetDividerMetrics(pSplit, 0.0f, 0.0f, 0.0f);
	if ( iRet == XUI_OK ) iRet = xuiSplitLayoutGetDividerMetrics(pSplit, &fLayoutSize, &fVisualSize, &fHitSize);
	XUI_TEST_CHECK(iRet == XUI_OK && fLayoutSize == 3.0f && fVisualSize == 3.0f && fHitSize == 12.0f,
	               "default divider metrics remain slim with an expanded hit target");

	memset(&tXgeDesc, 0, sizeof(tXgeDesc));
	tXgeDesc.iWidth = 1280;
	tXgeDesc.iHeight = 720;
	tXgeDesc.iRunMode = XGE_RUN_MANUAL;
	iRet = xgeInit(&tXgeDesc);
	XUI_TEST_CHECK(iRet == XGE_OK, "xge input bridge init");
	bXgeInitialized = 1;

	memset(&tDockDesc, 0, sizeof(tDockDesc));
	tDockDesc.iSize = sizeof(tDockDesc);
	iRet = xuiDockPanelCreate(pContext, &pDock, &tDockDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pDock != NULL, "nested dock create");
	iRet = xuiWidgetAddChild(pRoot, pDock);
	XUI_TEST_CHECK(iRet == XUI_OK, "nested dock add");
	iRet = xuiWidgetSetRect(pDock, (xui_rect_t){54.0f, 46.0f, 540.0f, 292.0f});
	XUI_TEST_CHECK(iRet == XUI_OK, "nested dock rect");

	arrTabItems[0] = "OCR Test";
	memset(&tTabsDesc, 0, sizeof(tTabsDesc));
	tTabsDesc.iSize = sizeof(tTabsDesc);
	tTabsDesc.arrItems = arrTabItems;
	tTabsDesc.iItemCount = 1;
	tTabsDesc.iSelected = 0;
	iRet = xuiTabsCreate(pContext, &pTabs, &tTabsDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pTabs != NULL, "nested tabs create");
	iRet = xuiDockPanelAddWindow(pDock, "Computer Assistant", pTabs, &iDockWindow);
	XUI_TEST_CHECK(iRet == XUI_OK, "nested dock window add");
	iRet = xuiDockPanelDockWindow(pDock, iDockWindow,
		XUI_DOCK_PANEL_REGION_DOCUMENT, XUI_DOCK_PANEL_SIDE_FILL, 0.0f, &iDockPane);
	XUI_TEST_CHECK(iRet == XUI_OK && iDockPane >= 0, "nested dock window dock");

	iRet = xuiWidgetCreate(pContext, &pOffsetParent);
	XUI_TEST_CHECK(iRet == XUI_OK && pOffsetParent != NULL, "nested offset parent create");
	iRet = xuiTabsAddPageChild(pTabs, 0, pOffsetParent);
	XUI_TEST_CHECK(iRet == XUI_OK, "nested offset parent add");
	iRet = xuiWidgetSetRect(pOffsetParent, (xui_rect_t){19.0f, 23.0f, 430.0f, 190.0f});
	XUI_TEST_CHECK(iRet == XUI_OK, "nested offset parent rect");

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.iOrientation = XUI_ORIENTATION_VERTICAL;
	tDesc.iPaneCount = 2;
	tDesc.fDividerSize = 8.0f;
	tDesc.fDividerVisualSize = 2.0f;
	tDesc.fDividerHitSize = 14.0f;
	iRet = xuiSplitLayoutCreate(pContext, &pNestedSplit, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pNestedSplit != NULL, "nested split create");
	iRet = xuiWidgetAddChild(pOffsetParent, pNestedSplit);
	XUI_TEST_CHECK(iRet == XUI_OK, "nested split add");
	iRet = xuiWidgetSetRect(pNestedSplit, (xui_rect_t){31.0f, 37.0f, 320.0f, 128.0f});
	XUI_TEST_CHECK(iRet == XUI_OK, "nested split rect");
	iRet = xuiWidgetSetEventHandler(pNestedSplit, XUI_EVENT_POINTER_DOWN,
		__xuiSplitPointerDown, &iPointerModifiers);
	XUI_TEST_CHECK(iRet == XUI_OK, "nested split pointer observer");
	iRet = xuiSplitLayoutSetPaneMode(pNestedSplit, 0, XUI_SPLIT_PANE_FIXED);
	if ( iRet == XUI_OK ) iRet = xuiSplitLayoutSetPaneFixedSize(pNestedSplit, 0, 110.0f);
	if ( iRet == XUI_OK ) iRet = xuiSplitLayoutSetPaneMinSize(pNestedSplit, 1, 60.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "nested split panes");
	iRet = __xuiSplitDispatchLayoutRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK, "nested dock tabs split render");

	tWorld = xuiWidgetGetWorldRect(pNestedSplit);
	tHit = xuiSplitLayoutGetDividerHitRect(pNestedSplit, 0);
	XUI_TEST_CHECK(tWorld.fX > 80.0f && tWorld.fY > 90.0f,
		"nested split has nonzero dock tab parent offset");
	fX = tWorld.fX + tHit.fX + tHit.fW * 0.5f;
	fY = tWorld.fY + tHit.fY + tHit.fH * 0.5f;
	iRet = __xuiSplitPumpXgePointer(pContext, XGE_EVENT_MOUSE_DOWN,
		fX * 2.0f, fY * 2.0f, XGE_MOUSE_LEFT,
		XGE_KEY_MOD_CTRL | XGE_KEY_MOD_SHIFT);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiSplitLayoutGetActiveDivider(pNestedSplit) == 0,
		"xge proxy nested vertical down");
	XUI_TEST_CHECK(iPointerModifiers == (XUI_MOD_CTRL | XUI_MOD_SHIFT),
		"xge proxy maps pointer modifiers");
	iRet = __xuiSplitPumpXgePointer(pContext, XGE_EVENT_MOUSE_MOVE,
		(fX + 28.0f) * 2.0f, fY * 2.0f, XGE_MOUSE_LEFT, XGE_KEY_MOD_CTRL);
	XUI_TEST_CHECK(iRet == XUI_OK, "xge proxy nested vertical move");
	iRet = __xuiSplitPumpXgePointer(pContext, XGE_EVENT_MOUSE_UP,
		(fX + 28.0f) * 2.0f, fY * 2.0f, 0u, 0u);
	XUI_TEST_CHECK(iRet == XUI_OK &&
		__xuiSplitNear(xuiSplitLayoutGetPaneFixedSize(pNestedSplit, 0), 138.0f),
		"xge proxy nested vertical drag follows pointer");

	iRet = xuiSplitLayoutSetOrientation(pNestedSplit, XUI_ORIENTATION_HORIZONTAL);
	if ( iRet == XUI_OK ) iRet = xuiSplitLayoutSetPaneFixedSize(pNestedSplit, 0, 50.0f);
	if ( iRet == XUI_OK ) iRet = xuiSplitLayoutSetPaneMinSize(pNestedSplit, 1, 20.0f);
	if ( iRet == XUI_OK ) iRet = __xuiSplitDispatchLayoutRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK, "nested horizontal prepare");
	tWorld = xuiWidgetGetWorldRect(pNestedSplit);
	tHit = xuiSplitLayoutGetDividerHitRect(pNestedSplit, 0);
	fDragBefore = xuiSplitLayoutGetPaneRect(pNestedSplit, 0).fH;
	fX = tWorld.fX + tHit.fX + tHit.fW * 0.5f;
	fY = tWorld.fY + tHit.fY + tHit.fH * 0.5f;
	iRet = __xuiSplitPumpXgePointer(pContext, XGE_EVENT_MOUSE_DOWN,
		fX * 2.0f, fY * 2.0f, XGE_MOUSE_LEFT, 0u);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiSplitLayoutGetActiveDivider(pNestedSplit) == 0,
		"xge proxy nested horizontal down");
	iRet = __xuiSplitPumpXgePointer(pContext, XGE_EVENT_MOUSE_MOVE,
		fX * 2.0f, (fY + 22.0f) * 2.0f, XGE_MOUSE_LEFT, 0u);
	XUI_TEST_CHECK(iRet == XUI_OK, "xge proxy nested horizontal move");
	iRet = __xuiSplitPumpXgePointer(pContext, XGE_EVENT_MOUSE_UP,
		fX * 2.0f, (fY + 22.0f) * 2.0f, 0u, 0u);
	XUI_TEST_CHECK(iRet == XUI_OK &&
		__xuiSplitNear(xuiSplitLayoutGetPaneFixedSize(pNestedSplit, 0), fDragBefore + 22.0f),
		"xge proxy nested horizontal drag follows pointer");

	iRet = xuiSplitLayoutSetPaneMode(pNestedSplit, 0, XUI_SPLIT_PANE_GROW);
	if ( iRet == XUI_OK ) iRet = xuiSplitLayoutSetPaneMode(pNestedSplit, 1, XUI_SPLIT_PANE_GROW);
	if ( iRet == XUI_OK ) iRet = xuiSplitLayoutSetPaneWeight(pNestedSplit, 0, 1.0f);
	if ( iRet == XUI_OK ) iRet = xuiSplitLayoutSetPaneWeight(pNestedSplit, 1, 1.0f);
	if ( iRet == XUI_OK ) iRet = xuiSplitLayoutSetPaneMinSize(pNestedSplit, 0, 20.0f);
	if ( iRet == XUI_OK ) iRet = xuiSplitLayoutSetPaneMinSize(pNestedSplit, 1, 30.0f);
	if ( iRet == XUI_OK ) iRet = __xuiSplitDispatchLayoutRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK, "horizontal grow panes prepare");
	tWorld = xuiWidgetGetWorldRect(pNestedSplit);
	tHit = xuiSplitLayoutGetDividerHitRect(pNestedSplit, 0);
	fDragBefore = xuiSplitLayoutGetPaneRect(pNestedSplit, 0).fH;
	fX = tWorld.fX + tHit.fX + tHit.fW * 0.5f;
	fY = tWorld.fY + tHit.fY + tHit.fH * 0.5f;
	iRet = __xuiSplitPumpXgePointer(pContext, XGE_EVENT_MOUSE_DOWN,
		fX * 2.0f, fY * 2.0f, XGE_MOUSE_LEFT, 0u);
	if ( iRet == XUI_OK ) iRet = __xuiSplitPumpXgePointer(pContext, XGE_EVENT_MOUSE_MOVE,
		fX * 2.0f, (fY + 4.0f) * 2.0f, XGE_MOUSE_LEFT, 0u);
	if ( iRet == XUI_OK ) iRet = __xuiSplitPumpXgePointer(pContext, XGE_EVENT_MOUSE_UP,
		fX * 2.0f, (fY + 4.0f) * 2.0f, 0u, 0u);
	if ( iRet == XUI_OK ) iRet = __xuiSplitDispatchLayoutRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK &&
		__xuiSplitNear(xuiSplitLayoutGetPaneRect(pNestedSplit, 0).fH, fDragBefore + 4.0f),
		"small horizontal grow drag does not reverse after commit");

	iRet = xuiSplitLayoutSetOrientation(pNestedSplit, XUI_ORIENTATION_VERTICAL);
	if ( iRet == XUI_OK ) iRet = xuiSplitLayoutSetPaneWeight(pNestedSplit, 0, 1.0f);
	if ( iRet == XUI_OK ) iRet = xuiSplitLayoutSetPaneWeight(pNestedSplit, 1, 1.0f);
	if ( iRet == XUI_OK ) iRet = xuiSplitLayoutSetPaneMinSize(pNestedSplit, 0, 40.0f);
	if ( iRet == XUI_OK ) iRet = xuiSplitLayoutSetPaneMinSize(pNestedSplit, 1, 60.0f);
	if ( iRet == XUI_OK ) iRet = __xuiSplitDispatchLayoutRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK, "vertical grow panes prepare");
	tWorld = xuiWidgetGetWorldRect(pNestedSplit);
	tHit = xuiSplitLayoutGetDividerHitRect(pNestedSplit, 0);
	fDragBefore = xuiSplitLayoutGetPaneRect(pNestedSplit, 0).fW;
	fX = tWorld.fX + tHit.fX + tHit.fW * 0.5f;
	fY = tWorld.fY + tHit.fY + tHit.fH * 0.5f;
	iRet = __xuiSplitPumpXgePointer(pContext, XGE_EVENT_MOUSE_DOWN,
		fX * 2.0f, fY * 2.0f, XGE_MOUSE_LEFT, 0u);
	if ( iRet == XUI_OK ) iRet = __xuiSplitPumpXgePointer(pContext, XGE_EVENT_MOUSE_MOVE,
		(fX + 28.0f) * 2.0f, fY * 2.0f, XGE_MOUSE_LEFT, 0u);
	if ( iRet == XUI_OK ) iRet = __xuiSplitPumpXgePointer(pContext, XGE_EVENT_MOUSE_UP,
		(fX + 28.0f) * 2.0f, fY * 2.0f, 0u, 0u);
	if ( iRet == XUI_OK ) iRet = __xuiSplitDispatchLayoutRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK &&
		__xuiSplitNear(xuiSplitLayoutGetPaneRect(pNestedSplit, 0).fW, fDragBefore + 28.0f),
		"vertical grow panes commit at preview position");

	tPane0 = xuiSplitLayoutGetPaneRect(pNestedSplit, 0);
	tPane1 = xuiSplitLayoutGetPaneRect(pNestedSplit, 1);
	fExpectedSize = tPane0.fW + tPane1.fW - 60.0f;
	fDragDelta = tPane1.fW;
	tWorld = xuiWidgetGetWorldRect(pNestedSplit);
	tHit = xuiSplitLayoutGetDividerHitRect(pNestedSplit, 0);
	fX = tWorld.fX + tHit.fX + tHit.fW * 0.5f;
	fY = tWorld.fY + tHit.fY + tHit.fH * 0.5f;
	iRet = __xuiSplitPumpXgePointer(pContext, XGE_EVENT_MOUSE_DOWN,
		fX * 2.0f, fY * 2.0f, XGE_MOUSE_LEFT, 0u);
	if ( iRet == XUI_OK ) iRet = __xuiSplitPumpXgePointer(pContext, XGE_EVENT_MOUSE_MOVE,
		(fX + fDragDelta) * 2.0f, fY * 2.0f, XGE_MOUSE_LEFT, 0u);
	if ( iRet == XUI_OK ) iRet = __xuiSplitPumpXgePointer(pContext, XGE_EVENT_MOUSE_UP,
		(fX + fDragDelta) * 2.0f, fY * 2.0f, 0u, 0u);
	if ( iRet == XUI_OK ) iRet = __xuiSplitDispatchLayoutRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK &&
		__xuiSplitNear(xuiSplitLayoutGetPaneRect(pNestedSplit, 0).fW, fExpectedSize) &&
		__xuiSplitNear(xuiSplitLayoutGetPaneRect(pNestedSplit, 1).fW, 60.0f),
		"grow panes commit at the clamped preview limit");

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.iOrientation = XUI_ORIENTATION_VERTICAL;
	tDesc.iPaneCount = 2;
	tDesc.fDividerSize = 8.0f;
	tDesc.fDividerVisualSize = 2.0f;
	tDesc.fDividerHitSize = 14.0f;
	iRet = xuiSplitLayoutCreate(pContext, &pMultiSplit, &tDesc);
	if ( iRet == XUI_OK ) iRet = xuiWidgetAddChild(pRoot, pMultiSplit);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetRect(pMultiSplit, (xui_rect_t){330.0f, 215.0f, 280.0f, 120.0f});
	if ( iRet == XUI_OK ) iRet = xuiSplitLayoutSetPaneMode(pMultiSplit, 0, XUI_SPLIT_PANE_FIXED);
	if ( iRet == XUI_OK ) iRet = xuiSplitLayoutSetPaneFixedSize(pMultiSplit, 0, 100.0f);
	if ( iRet == XUI_OK ) iRet = xuiSplitLayoutSetChange(pMultiSplit, __xuiSplitChanged, &iMultiChanged);
	if ( iRet == XUI_OK ) iRet = __xuiSplitDispatchLayoutRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK, "multi-pointer split prepare");
	tWorld = xuiWidgetGetWorldRect(pMultiSplit);
	tHit = xuiSplitLayoutGetDividerHitRect(pMultiSplit, 0);
	fX = tWorld.fX + tHit.fX + tHit.fW * 0.5f;
	fY = tWorld.fY + tHit.fY + tHit.fH * 0.5f;
	iRet = xuiInputPointerDownEx(pContext, 701u, XUI_POINTER_TYPE_TOUCH,
		(int)fX, (int)fY, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiSplitLayoutGetActiveDivider(pMultiSplit) == 0 &&
		xuiGetPointerCaptureEx(pContext, 701u, XUI_POINTER_TYPE_TOUCH) != NULL,
		"first touch owns split drag");
	iRet = xuiInputPointerMoveEx(pContext, 701u, XUI_POINTER_TYPE_TOUCH,
		(int)(fX + 24.0f), (int)fY, XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && iMultiChanged == 0, "first touch preview remains deferred");
	iRet = xuiInputPointerDownEx(pContext, 702u, XUI_POINTER_TYPE_TOUCH,
		(int)fX, (int)fY, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiGetPointerCaptureEx(pContext, 702u, XUI_POINTER_TYPE_TOUCH) == NULL,
		"second touch cannot steal split drag");
	iRet = xuiInputPointerMoveEx(pContext, 702u, XUI_POINTER_TYPE_TOUCH,
		(int)(fX + 70.0f), (int)fY, XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	iRet = (iRet == XUI_OK) ? xuiInputPointerUpEx(pContext, 702u, XUI_POINTER_TYPE_TOUCH,
		(int)(fX + 70.0f), (int)fY, XUI_POINTER_BUTTON_LEFT, 0u) : iRet;
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && iMultiChanged == 0 &&
		xuiSplitLayoutGetActiveDivider(pMultiSplit) == 0,
		"unrelated touch cannot move or commit split drag");
	iRet = xuiInputPointerUpEx(pContext, 701u, XUI_POINTER_TYPE_TOUCH,
		(int)(fX + 24.0f), (int)fY, XUI_POINTER_BUTTON_LEFT, 0u);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && iMultiChanged == 1 &&
		xuiSplitLayoutGetActiveDivider(pMultiSplit) == -1 &&
		xuiGetPointerCaptureEx(pContext, 701u, XUI_POINTER_TYPE_TOUCH) == NULL &&
		__xuiSplitNear(xuiSplitLayoutGetPaneFixedSize(pMultiSplit, 0), 124.0f),
		"owning touch commits the preview position exactly once");
	iRet = __xuiSplitDispatchLayoutRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK, "multi-pointer commit layout");
	tWorld = xuiWidgetGetWorldRect(pMultiSplit);
	tHit = xuiSplitLayoutGetDividerHitRect(pMultiSplit, 0);
	tDivider = xuiSplitLayoutGetDividerLayoutRect(pMultiSplit, 0);
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iSize = sizeof(tEvent);
	tEvent.iType = XUI_EVENT_POINTER_DOUBLE_CLICK;
	tEvent.iButton = XUI_POINTER_BUTTON_LEFT;
	tEvent.iPointerType = XUI_POINTER_TYPE_MOUSE;
	tEvent.fX = (int)(tWorld.fX + tHit.fX + 1.0f);
	tEvent.fY = (int)(tWorld.fY + tHit.fY + tHit.fH * 0.5f);
	tEvent.pTarget = xuiHitTest(pContext, tEvent.fX, tEvent.fY, 0u);
	XUI_TEST_CHECK((float)tEvent.fX < tWorld.fX + tDivider.fX,
		"equalize test point uses expanded divider hit target");
	iRet = xuiDispatchEvent(pContext, &tEvent);
	if ( iRet == XUI_OK ) iRet = __xuiSplitDispatchLayoutRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK && iMultiChanged == 2 &&
		__xuiSplitNear(xuiSplitLayoutGetPaneFixedSize(pMultiSplit, 0), 136.0f),
		"double-click equalizes from the full divider hit target");
	pDividerWidget = NULL;
	for ( pDividerWidget = xuiWidgetGetFirstChild(pMultiSplit); pDividerWidget != NULL;
	      pDividerWidget = xuiWidgetGetNextSibling(pDividerWidget) ) {
		int iLayer = 0;
		int iZIndex = 0;
		if ( xuiWidgetGetLayer(pDividerWidget, &iLayer, &iZIndex) == XUI_OK &&
		     iLayer == XUI_LAYER_NORMAL && iZIndex == 10 ) break;
	}
	memset(&tAccessibleNode, 0, sizeof(tAccessibleNode));
	tAccessibleNode.iSize = sizeof(tAccessibleNode);
	iRet = (pDividerWidget != NULL) ?
		xuiWidgetGetAccessibleNode(pDividerWidget, 0, &tAccessibleNode) : XUI_ERROR_NOT_FOUND;
	XUI_TEST_CHECK(iRet == XUI_OK && xuiWidgetGetAccessibleNodeCount(pDividerWidget) == 1 &&
		xuiWidgetGetTabStop(pDividerWidget) == 1 &&
		tAccessibleNode.iRole == XUI_ACCESSIBLE_ROLE_SEPARATOR &&
		(tAccessibleNode.iState & XUI_ACCESSIBLE_STATE_FOCUSABLE) != 0u &&
		(tAccessibleNode.iState & XUI_ACCESSIBLE_STATE_VERTICAL) != 0u &&
		tAccessibleNode.tRange.fMin <= tAccessibleNode.tRange.fValue &&
		tAccessibleNode.tRange.fMax >= tAccessibleNode.tRange.fValue &&
		(tAccessibleNode.iActions & XUI_ACCESSIBLE_ACTION_MASK(XUI_ACCESSIBLE_ACTION_INCREMENT)) != 0u,
		"divider exposes constrained separator accessibility node");
	iRet = xuiWidgetPerformAccessibleAction(pDividerWidget, 1,
		XUI_ACCESSIBLE_ACTION_INCREMENT, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK && iMultiChanged == 3 &&
		__xuiSplitNear(xuiSplitLayoutGetPaneFixedSize(pMultiSplit, 0), 144.0f),
		"accessibility increment reuses divider commit path");
	iRet = __xuiSplitDispatchLayoutRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK, "accessibility increment layout");
	memset(&tAccessibleValue, 0, sizeof(tAccessibleValue));
	tAccessibleValue.iSize = sizeof(tAccessibleValue);
	tAccessibleValue.iType = XUI_ACCESSIBLE_VALUE_NUMBER;
	tAccessibleValue.fNumber = 132.0;
	iRet = xuiWidgetPerformAccessibleAction(pDividerWidget, 1,
		XUI_ACCESSIBLE_ACTION_SET_VALUE, &tAccessibleValue);
	XUI_TEST_CHECK(iRet == XUI_OK && iMultiChanged == 4 &&
		__xuiSplitNear(xuiSplitLayoutGetPaneFixedSize(pMultiSplit, 0), 132.0f),
		"accessibility set value reuses constrained divider commit path");
	iRet = __xuiSplitDispatchLayoutRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK, "accessibility set value layout");
	tAccessibleValue.fNumber = NAN;
	iRet = xuiWidgetPerformAccessibleAction(pDividerWidget, 1,
		XUI_ACCESSIBLE_ACTION_SET_VALUE, &tAccessibleValue);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && iMultiChanged == 4 &&
		__xuiSplitNear(xuiSplitLayoutGetPaneFixedSize(pMultiSplit, 0), 132.0f),
		"accessibility rejects non-finite divider value without mutation");
	iRet = xuiWidgetSetEnabled(pMultiSplit, 0);
	memset(&tAccessibleNode, 0, sizeof(tAccessibleNode));
	tAccessibleNode.iSize = sizeof(tAccessibleNode);
	if ( iRet == XUI_OK ) iRet = xuiWidgetGetAccessibleNode(pDividerWidget, 0, &tAccessibleNode);
	XUI_TEST_CHECK(iRet == XUI_OK &&
		(tAccessibleNode.iState & XUI_ACCESSIBLE_STATE_DISABLED) != 0u &&
		tAccessibleNode.iActions == 0u,
		"disabled divider exposes state without actions");
	iRet = xuiWidgetPerformAccessibleAction(pDividerWidget, 1,
		XUI_ACCESSIBLE_ACTION_INCREMENT, NULL);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && iMultiChanged == 4,
		"disabled divider rejects accessibility mutation");
	iRet = xuiWidgetSetEnabled(pMultiSplit, 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "reenable accessible divider");
	tWorld = xuiWidgetGetWorldRect(pMultiSplit);
	tHit = xuiSplitLayoutGetDividerHitRect(pMultiSplit, 0);
	fX = tWorld.fX + tHit.fX + tHit.fW * 0.5f;
	fY = tWorld.fY + tHit.fY + tHit.fH * 0.5f;
	iRet = xuiInputPointerDownEx(pContext, 703u, XUI_POINTER_TYPE_TOUCH,
		(int)fX, (int)fY, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiSplitLayoutGetActiveDivider(pMultiSplit) == 0,
		"configuration-cancel drag starts");
	iRet = xuiSplitLayoutSetDividerHitSize(pMultiSplit, 16.0f);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiSplitLayoutGetActiveDivider(pMultiSplit) == -1 &&
		xuiGetPointerCaptureEx(pContext, 703u, XUI_POINTER_TYPE_TOUCH) == NULL,
		"geometry change cancels active drag and exact capture");
	iRet = xuiInputPointerUpEx(pContext, 703u, XUI_POINTER_TYPE_TOUCH,
		(int)(fX + 40.0f), (int)fY, XUI_POINTER_BUTTON_LEFT, 0u);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && iMultiChanged == 4,
		"cancelled drag cannot commit on later pointer up");

	iRet = xuiSplitLayoutCreate(pContext, &pDestroySplit, &tDesc);
	if ( iRet == XUI_OK ) iRet = xuiWidgetAddChild(pRoot, pDestroySplit);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetRect(pDestroySplit, (xui_rect_t){20.0f, 220.0f, 280.0f, 100.0f});
	if ( iRet == XUI_OK ) iRet = xuiSplitLayoutSetPaneMode(pDestroySplit, 0, XUI_SPLIT_PANE_FIXED);
	if ( iRet == XUI_OK ) iRet = xuiSplitLayoutSetPaneFixedSize(pDestroySplit, 0, 100.0f);
	tDestroyState.ppWidget = &pDestroySplit;
	if ( iRet == XUI_OK ) iRet = xuiSplitLayoutSetChange(pDestroySplit,
		__xuiSplitDestroyOnChange, &tDestroyState);
	if ( iRet == XUI_OK ) iRet = __xuiSplitDispatchLayoutRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK, "destroy-on-change split prepare");
	tWorld = xuiWidgetGetWorldRect(pDestroySplit);
	tHit = xuiSplitLayoutGetDividerHitRect(pDestroySplit, 0);
	fX = tWorld.fX + tHit.fX + tHit.fW * 0.5f;
	fY = tWorld.fY + tHit.fY + tHit.fH * 0.5f;
	iRet = xuiInputPointerDown(pContext, fX, fY,
		XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	if ( iRet == XUI_OK ) iRet = xuiInputPointerMove(pContext, fX + 16.0f, fY,
		XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	if ( iRet == XUI_OK ) iRet = xuiInputPointerUp(pContext, fX + 16.0f, fY,
		XUI_POINTER_BUTTON_LEFT, 0u);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && tDestroyState.iCount == 1 && pDestroySplit == NULL &&
		xuiGetPointerCapture(pContext) == NULL,
		"change callback may destroy split without stale access or capture");

	iRet = xuiSplitLayoutCreate(pContext, &pDestroySplit, &tDesc);
	if ( iRet == XUI_OK ) iRet = xuiWidgetAddChild(pRoot, pDestroySplit);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetRect(pDestroySplit, (xui_rect_t){20.0f, 220.0f, 280.0f, 100.0f});
	if ( iRet == XUI_OK ) iRet = xuiSplitLayoutSetPaneMode(pDestroySplit, 0, XUI_SPLIT_PANE_FIXED);
	if ( iRet == XUI_OK ) iRet = xuiSplitLayoutSetPaneFixedSize(pDestroySplit, 0, 100.0f);
	tDestroyContextState.ppContext = &pContext;
	if ( iRet == XUI_OK ) iRet = xuiSplitLayoutSetChange(pDestroySplit,
		__xuiSplitDestroyContextOnChange, &tDestroyContextState);
	if ( iRet == XUI_OK ) iRet = __xuiSplitDispatchLayoutRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK, "destroy-context-on-change split prepare");
	tWorld = xuiWidgetGetWorldRect(pDestroySplit);
	tHit = xuiSplitLayoutGetDividerHitRect(pDestroySplit, 0);
	fX = tWorld.fX + tHit.fX + tHit.fW * 0.5f;
	fY = tWorld.fY + tHit.fY + tHit.fH * 0.5f;
	iRet = xuiInputPointerDown(pContext, fX, fY,
		XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	if ( iRet == XUI_OK ) iRet = xuiInputPointerUp(pContext, fX + 12.0f, fY,
		XUI_POINTER_BUTTON_LEFT, 0u);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && tDestroyContextState.iCount == 1 && pContext == NULL,
		"change callback may destroy context without stale access");

cleanup:
	if ( pTarget != NULL ) {
		tState.tProxy.surfaceDestroy(&tState.tProxy, pTarget);
	}
	if ( pContext != NULL ) {
		xuiDestroy(pContext);
	}
	if ( bXgeInitialized ) {
		xgeUnit();
	}
	if ( iFailed ) {
		return 1;
	}
	printf("xui_split_layout_test passed\n");
	return 0;
}
