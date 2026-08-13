#include "xui.h"
#include "xge.h"
#include "xui_test_proxy.h"

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
	xui_widget pShadowOverlay;
	xui_widget pCompetingOverlay;
	xui_widget pDividerWidget;
	xui_surface pTarget;
	xui_surface pShadowCache;
	xui_surface pDividerCache;
	xui_split_layout_desc_t tDesc;
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
	float fX;
	float fY;
	int iChanged;
	int iDockWindow;
	int iDockPane;
	int bXgeInitialized;
	int iFailed;
	int iRet;
	uint32_t iPointerModifiers;
	const char* arrTabItems[1];

	pContext = NULL;
	pRoot = NULL;
	pSplit = NULL;
	pDock = NULL;
	pTabs = NULL;
	pOffsetParent = NULL;
	pNestedSplit = NULL;
	pShadowOverlay = NULL;
	pCompetingOverlay = NULL;
	pDividerWidget = NULL;
	pTarget = NULL;
	pShadowCache = NULL;
	pDividerCache = NULL;
	iChanged = 0;
	iPointerModifiers = 0u;
	bXgeInitialized = 0;
	iFailed = 0;
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
