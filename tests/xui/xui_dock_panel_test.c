#include "xui.h"
#include "xui_test_proxy.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

#define XUI_TEST_CHECK(expr, msg) \
	do { \
		if ( !(expr) ) { \
			printf("xui_dock_panel_test failed: %s\n", (msg)); \
			iFailed = 1; \
			goto cleanup; \
		} \
	} while ( 0 )

typedef struct dock_test_data_t {
	int iStateChanged;
	int iActiveChanged;
	int iClose;
	int iContext;
	int iClientPointerDown;
} dock_test_data_t;

typedef struct dock_destroy_test_data_t {
	xui_widget* ppWidget;
	int iCount;
} dock_destroy_test_data_t;

typedef struct dock_replace_close_test_data_t {
	xui_widget pReplacementClient;
	int iReplacementWindow;
	int iResult;
	int iCount;
} dock_replace_close_test_data_t;

static xui_text_draw_proc gDockTextDraw;
static xui_font gDockTrackedFont;
static int gDockTrackedFontDrawCount;

static int __xuiDockTestTextDraw(xui_proxy pProxy, xui_surface pTarget,
	const xui_text_item_t* pTextItem, xui_rect_t tRect, uint32_t iColor, uint32_t iFlags)
{
	if ( pTextItem != NULL && pTextItem->pFont == gDockTrackedFont ) {
		gDockTrackedFontDrawCount++;
	}
	return gDockTextDraw(pProxy, pTarget, pTextItem, tRect, iColor, iFlags);
}

static void __xuiDockTestDestroyWidget(xui_widget pWidget, dock_destroy_test_data_t* pData)
{
	if ( pData == NULL || pData->ppWidget == NULL || *pData->ppWidget != pWidget ) return;
	pData->iCount++;
	xuiWidgetDestroy(pWidget);
	*pData->ppWidget = NULL;
}

static void __xuiDockTestDestroyOnState(xui_widget pWidget, int iWindow, int iOldState, int iNewState, void* pUser)
{
	(void)iWindow;
	(void)iOldState;
	(void)iNewState;
	__xuiDockTestDestroyWidget(pWidget, (dock_destroy_test_data_t*)pUser);
}

static void __xuiDockTestDestroyOnActive(xui_widget pWidget, int iPane, int iOldWindow, int iNewWindow, void* pUser)
{
	(void)iPane;
	(void)iOldWindow;
	(void)iNewWindow;
	__xuiDockTestDestroyWidget(pWidget, (dock_destroy_test_data_t*)pUser);
}

static void __xuiDockTestDestroyOnClose(xui_widget pWidget, int iWindow, void* pUser)
{
	(void)iWindow;
	__xuiDockTestDestroyWidget(pWidget, (dock_destroy_test_data_t*)pUser);
}

static int __xuiDockTestDestroyOnContext(xui_widget pWidget, const xui_dock_hit_t* pHit,
	float fX, float fY, void* pUser)
{
	(void)pHit;
	(void)fX;
	(void)fY;
	__xuiDockTestDestroyWidget(pWidget, (dock_destroy_test_data_t*)pUser);
	return XUI_OK;
}

static void __xuiDockTestReplaceOnClose(xui_widget pWidget, int iWindow, void* pUser)
{
	dock_replace_close_test_data_t* pData = (dock_replace_close_test_data_t*)pUser;
	(void)iWindow;
	if ( pData == NULL || pData->iCount != 0 ) return;
	pData->iCount++;
	pData->iReplacementWindow = -1;
	pData->iResult = xuiDockPanelSetWindowClose(pWidget, NULL, NULL);
	if ( pData->iResult == XUI_OK ) pData->iResult = xuiDockPanelClear(pWidget);
	if ( pData->iResult == XUI_OK ) {
		pData->iResult = xuiDockPanelAddWindow(pWidget, "Replacement",
			pData->pReplacementClient, &pData->iReplacementWindow);
		if ( pData->iResult == XUI_OK ) pData->pReplacementClient = NULL;
	}
	if ( pData->iResult == XUI_OK ) {
		pData->iResult = xuiDockPanelDockWindow(pWidget, pData->iReplacementWindow,
			XUI_DOCK_PANEL_REGION_DOCUMENT, XUI_DOCK_PANEL_SIDE_FILL, 0.0f, NULL);
	}
}

static int __xuiDockTestClientEvent(xui_widget pWidget, const xui_event_t* pEvent, void* pUser)
{
	dock_test_data_t* pData = (dock_test_data_t*)pUser;
	(void)pWidget;
	if ( pData != NULL && pEvent != NULL && pEvent->iType == XUI_EVENT_POINTER_DOWN ) {
		pData->iClientPointerDown++;
	}
	return XUI_OK;
}

static void __xuiDockTestState(xui_widget pWidget, int iWindow, int iOldState, int iNewState, void* pUser)
{
	dock_test_data_t* pData = (dock_test_data_t*)pUser;
	(void)pWidget;
	(void)iWindow;
	(void)iOldState;
	(void)iNewState;
	if ( pData != NULL ) pData->iStateChanged++;
}

static void __xuiDockTestActive(xui_widget pWidget, int iPane, int iOldWindow, int iNewWindow, void* pUser)
{
	dock_test_data_t* pData = (dock_test_data_t*)pUser;
	(void)pWidget;
	(void)iPane;
	(void)iOldWindow;
	(void)iNewWindow;
	if ( pData != NULL ) pData->iActiveChanged++;
}

static void __xuiDockTestClose(xui_widget pWidget, int iWindow, void* pUser)
{
	dock_test_data_t* pData = (dock_test_data_t*)pUser;
	(void)pWidget;
	(void)iWindow;
	if ( pData != NULL ) pData->iClose++;
}

static int __xuiDockTestContext(xui_widget pWidget, const xui_dock_hit_t* pHit, float fX, float fY, void* pUser)
{
	dock_test_data_t* pData = (dock_test_data_t*)pUser;
	(void)pWidget;
	(void)pHit;
	(void)fX;
	(void)fY;
	if ( pData != NULL ) pData->iContext++;
	return XUI_EVENT_DISPATCH_STOP;
}

static int __xuiDockTestClick(xui_context pContext, float fX, float fY)
{
	int iRet;
	iRet = xuiInputPointerDown(pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	if ( iRet == XUI_OK ) iRet = xuiInputPointerUp(pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, 0);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	return iRet;
}

static int __xuiDockTestMiddleClick(xui_context pContext, float fX, float fY)
{
	int iRet;
	iRet = xuiInputPointerDown(pContext, fX, fY, XUI_POINTER_BUTTON_MIDDLE, XUI_POINTER_BUTTON_MIDDLE);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	if ( iRet == XUI_OK ) iRet = xuiInputPointerUp(pContext, fX, fY, XUI_POINTER_BUTTON_MIDDLE, 0);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	return iRet;
}

static int __xuiDockTestDrag(xui_context pContext, float fX0, float fY0, float fX1, float fY1)
{
	int iRet;
	iRet = xuiInputPointerDown(pContext, fX0, fY0, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	if ( iRet == XUI_OK ) iRet = xuiInputPointerMove(pContext, fX1, fY1, XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	if ( iRet == XUI_OK ) iRet = xuiInputPointerUp(pContext, fX1, fY1, XUI_POINTER_BUTTON_LEFT, 0);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	return iRet;
}

static int __xuiDockTestDragMove(xui_context pContext, float fX0, float fY0, float fX1, float fY1)
{
	int iRet;
	iRet = xuiInputPointerDown(pContext, fX0, fY0, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	if ( iRet == XUI_OK ) iRet = xuiInputPointerMove(pContext, fX1, fY1, XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	return iRet;
}

static int __xuiDockTestDragUp(xui_context pContext, float fX, float fY)
{
	int iRet;
	iRet = xuiInputPointerUp(pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, 0);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	return iRet;
}

static int __xuiDockTestDragEx(xui_context pContext, float fX0, float fY0, float fX1, float fY1, uint32_t iModifiers)
{
	int iRet;
	iRet = xuiInputSetModifiers(pContext, iModifiers);
	if ( iRet == XUI_OK ) iRet = xuiInputPointerDown(pContext, fX0, fY0, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	if ( iRet == XUI_OK ) iRet = xuiInputPointerMove(pContext, fX1, fY1, XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	if ( iRet == XUI_OK ) iRet = xuiInputPointerUp(pContext, fX1, fY1, XUI_POINTER_BUTTON_LEFT, 0);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	if ( iRet == XUI_OK ) iRet = xuiInputSetModifiers(pContext, 0);
	return iRet;
}

static int __xuiDockFindMenuValue(xui_widget pMenu, int iValue)
{
	int count;
	int i;
	count = xuiMenuGetItemCount(pMenu);
	for ( i = 0; i < count; i++ ) {
		const xui_menu_item_t* pItem = xuiMenuGetItem(pMenu, i);
		if ( (pItem != NULL) && (pItem->iValue == iValue) ) return i;
	}
	return -1;
}

static int __xuiDockFindSplitter(xui_widget pDock, xui_dock_hit_t* pHit)
{
	float x;
	float y;
	for ( y = 10.0f; y < 420.0f; y += 8.0f ) {
		for ( x = 10.0f; x < 620.0f; x += 8.0f ) {
			if ( xuiDockPanelHitTest(pDock, x, y, pHit) == XUI_OK && pHit->iType == XUI_DOCK_PANEL_HIT_SPLITTER ) {
				return 1;
			}
		}
	}
	return 0;
}

static int __xuiDockFindRegionSplitter(xui_widget pDock, int iRegion, xui_dock_hit_t* pHit)
{
	float x;
	float y;
	for ( y = 10.0f; y < 420.0f; y += 8.0f ) {
		for ( x = 10.0f; x < 620.0f; x += 8.0f ) {
			if ( xuiDockPanelHitTest(pDock, x, y, pHit) == XUI_OK &&
			     pHit->iType == XUI_DOCK_PANEL_HIT_SPLITTER &&
			     pHit->iRegion == iRegion ) {
				return 1;
			}
		}
	}
	return 0;
}

static int __xuiDockFindNodeSplitter(xui_widget pDock, int iOrientation, int iNode, xui_dock_hit_t* pHit)
{
	float x;
	float y;
	for ( y = 2.0f; y < 420.0f; y += 4.0f ) {
		for ( x = 2.0f; x < 624.0f; x += 4.0f ) {
			if ( xuiDockPanelHitTest(pDock, x, y, pHit) != XUI_OK ||
			     pHit->iType != XUI_DOCK_PANEL_HIT_SPLITTER || pHit->iNode < 0 ) continue;
			if ( iNode >= 0 && pHit->iNode != iNode ) continue;
			if ( iOrientation == XUI_ORIENTATION_VERTICAL && pHit->tRect.fW <= pHit->tRect.fH ) return 1;
			if ( iOrientation == XUI_ORIENTATION_HORIZONTAL && pHit->tRect.fH < pHit->tRect.fW ) return 1;
		}
	}
	return 0;
}

static int __xuiDockNear(float a, float b)
{
	float d = a - b;
	if ( d < 0.0f ) d = -d;
	return d <= 1.01f;
}

static int __xuiDockTestAutoHideResize(xui_context pContext, xui_widget pDock, int iRegion)
{
	xui_rect_t world;
	xui_rect_t before;
	xui_rect_t pending;
	xui_rect_t after;
	float x0;
	float y0;
	float x1;
	float y1;
	float beforeExtent;
	float afterExtent;
	int grow;
	int cursor;
	int iRet;
	world = xuiWidgetGetWorldRect(pDock);
	before = xuiDockPanelGetAutoHideExpandRect(pDock);
	if ( before.fW <= 0.0f || before.fH <= 0.0f ) return 0;
	x0 = world.fX + before.fX + before.fW * 0.5f;
	y0 = world.fY + before.fY + before.fH * 0.5f;
	x1 = x0;
	y1 = y0;
	grow = (iRegion == XUI_DOCK_PANEL_REGION_LEFT || iRegion == XUI_DOCK_PANEL_REGION_RIGHT) ?
		(before.fW < 380.0f) : (before.fH < 280.0f);
	if ( iRegion == XUI_DOCK_PANEL_REGION_LEFT ) {
		x0 = world.fX + before.fX + before.fW - 2.0f;
		x1 = x0 + (grow ? 28.0f : -24.0f);
		cursor = XUI_CURSOR_RESIZE_EW;
	} else if ( iRegion == XUI_DOCK_PANEL_REGION_RIGHT ) {
		x0 = world.fX + before.fX + 2.0f;
		x1 = x0 + (grow ? -28.0f : 24.0f);
		cursor = XUI_CURSOR_RESIZE_EW;
	} else if ( iRegion == XUI_DOCK_PANEL_REGION_TOP ) {
		y0 = world.fY + before.fY + before.fH - 2.0f;
		y1 = y0 + (grow ? 24.0f : -20.0f);
		cursor = XUI_CURSOR_RESIZE_NS;
	} else if ( iRegion == XUI_DOCK_PANEL_REGION_BOTTOM ) {
		y0 = world.fY + before.fY + 2.0f;
		y1 = y0 + (grow ? -24.0f : 20.0f);
		cursor = XUI_CURSOR_RESIZE_NS;
	} else {
		return 0;
	}
	if ( xuiQueryCursor(pContext, (int)x0, (int)y0) != cursor ) return 0;
	iRet = __xuiDockTestDragMove(pContext, x0, y0, x1, y1);
	if ( iRet != XUI_OK ) return 0;
	pending = xuiDockPanelGetAutoHideExpandRect(pDock);
	if ( !__xuiDockNear(pending.fX, before.fX) || !__xuiDockNear(pending.fY, before.fY) ||
	     !__xuiDockNear(pending.fW, before.fW) || !__xuiDockNear(pending.fH, before.fH) ) return 0;
	iRet = __xuiDockTestDragUp(pContext, x1, y1);
	if ( iRet == XUI_OK ) iRet = xuiLayout(pContext);
	if ( iRet != XUI_OK ) return 0;
	after = xuiDockPanelGetAutoHideExpandRect(pDock);
	beforeExtent = (iRegion == XUI_DOCK_PANEL_REGION_LEFT || iRegion == XUI_DOCK_PANEL_REGION_RIGHT) ? before.fW : before.fH;
	afterExtent = (iRegion == XUI_DOCK_PANEL_REGION_LEFT || iRegion == XUI_DOCK_PANEL_REGION_RIGHT) ? after.fW : after.fH;
	if ( grow ) {
		if ( afterExtent < beforeExtent + 16.0f ) return 0;
	} else if ( afterExtent > beforeExtent - 14.0f ) {
		return 0;
	}
	if ( iRegion == XUI_DOCK_PANEL_REGION_LEFT ) {
		return __xuiDockNear(after.fX, before.fX);
	}
	if ( iRegion == XUI_DOCK_PANEL_REGION_RIGHT ) {
		return __xuiDockNear(after.fX + after.fW, before.fX + before.fW);
	}
	if ( iRegion == XUI_DOCK_PANEL_REGION_TOP ) {
		return __xuiDockNear(after.fY, before.fY);
	}
	return __xuiDockNear(after.fY + after.fH, before.fY + before.fH);
}

static int __xuiDockFindTabClose(xui_widget pDock, int iWindow, xui_rect_t tTabRect, xui_dock_hit_t* pHit)
{
	float x;
	float y;
	float x0;
	if ( tTabRect.fW <= 0.0f || tTabRect.fH <= 0.0f ) return 0;
	x0 = tTabRect.fX + tTabRect.fW - 26.0f;
	if ( x0 < tTabRect.fX + 1.0f ) x0 = tTabRect.fX + 1.0f;
	for ( y = tTabRect.fY + 1.0f; y < tTabRect.fY + tTabRect.fH - 1.0f; y += 2.0f ) {
		for ( x = x0; x < tTabRect.fX + tTabRect.fW - 1.0f; x += 2.0f ) {
			if ( xuiDockPanelHitTest(pDock, x, y, pHit) == XUI_OK &&
			     pHit->iType == XUI_DOCK_PANEL_HIT_PANE_TAB_CLOSE &&
			     pHit->iWindow == iWindow ) {
				return 1;
			}
		}
	}
	return 0;
}

static int __xuiDockFindPaneFreeDropPoint(xui_widget pDock, int iWindow, xui_rect_t tRect, float* pX, float* pY)
{
	xui_dock_drop_info_t tDrop;
	float x;
	float y;
	for ( y = tRect.fY + 56.0f; y < tRect.fY + tRect.fH - 56.0f; y += 17.0f ) {
		for ( x = tRect.fX + 56.0f; x < tRect.fX + tRect.fW - 56.0f; x += 19.0f ) {
			if ( xuiDockPanelFindDropTarget(pDock, iWindow, x, y, &tDrop) == XUI_OK && !tDrop.bValid ) {
				if ( pX != NULL ) *pX = x;
				if ( pY != NULL ) *pY = y;
				return 1;
			}
		}
	}
	return 0;
}

static int __xuiDockTestCapacityRollback(xui_context pContext, xui_widget pRoot,
	const xui_dock_panel_desc_t* pDesc)
{
	xui_widget pDock = NULL;
	xui_widget pClient = NULL;
	xui_dock_window_info_t tInfo;
	int arrWindows[49];
	int iBasePane = -1;
	int iPane = -1;
	int iRet;
	int i;
	int bPassed = 0;
	memset(arrWindows, 0xff, sizeof(arrWindows));
	iRet = xuiDockPanelCreate(pContext, &pDock, pDesc);
	if ( iRet == XUI_OK ) iRet = xuiWidgetAddChild(pRoot, pDock);
	for ( i = 0; iRet == XUI_OK && i < 49; i++ ) {
		iRet = xuiWidgetCreate(pContext, &pClient);
		if ( iRet == XUI_OK ) iRet = xuiDockPanelAddWindow(pDock, "Capacity", pClient, &arrWindows[i]);
		if ( iRet == XUI_OK ) pClient = NULL;
	}
	if ( iRet == XUI_OK ) {
		iRet = xuiDockPanelDockWindow(pDock, arrWindows[0], XUI_DOCK_PANEL_REGION_DOCUMENT,
			XUI_DOCK_PANEL_SIDE_FILL, 0.0f, &iBasePane);
	}
	for ( i = 1; iRet == XUI_OK && i < 48; i++ ) {
		iRet = xuiDockPanelDockWindow(pDock, arrWindows[i], XUI_DOCK_PANEL_REGION_DOCUMENT,
			XUI_DOCK_PANEL_SIDE_RIGHT, 0.25f, &iPane);
	}
	if ( iRet == XUI_OK ) iRet = xuiDockPanelDockWindowToPane(pDock, arrWindows[48], iBasePane);
	if ( iRet == XUI_OK && xuiDockPanelGetPaneCount(pDock) == XUI_DOCK_PANEL_PANE_CAPACITY &&
	     xuiDockPanelGetPaneWindowCount(pDock, iBasePane) == 2 ) {
		iRet = xuiDockPanelDockWindow(pDock, arrWindows[48], XUI_DOCK_PANEL_REGION_DOCUMENT,
			XUI_DOCK_PANEL_SIDE_RIGHT, 0.25f, NULL);
		if ( iRet == XUI_ERROR_OUT_OF_MEMORY &&
		     xuiDockPanelGetWindowInfo(pDock, arrWindows[48], &tInfo) == XUI_OK &&
		     tInfo.iState == XUI_DOCK_PANEL_WINDOW_DOCKED && tInfo.iPane == iBasePane &&
		     xuiDockPanelGetPaneWindowCount(pDock, iBasePane) == 2 ) {
			bPassed = 1;
		}
	}
	if ( pClient != NULL && xuiWidgetGetParent(pClient) == NULL ) xuiWidgetDestroy(pClient);
	if ( pDock != NULL ) xuiWidgetDestroy(pDock);
	return bPassed;
}

int main(void)
{
	xui_test_proxy_state_t tState;
	xui_context pContext;
	xui_widget pRoot;
	xui_widget pDock;
	xui_widget arrClient[11];
	xui_widget pNestedClient;
	xui_widget pTempDock;
	xui_widget pTempClient;
	xui_widget pTempClient2;
	xui_widget pTempButton;
	xui_widget pHitWidget;
	xui_widget pMenu;
	xui_surface pTarget;
	xui_surface pCache;
	xui_surface pIndicatorCache;
	xui_font pFont;
	xui_font pFont2;
	xui_dock_panel_desc_t tDesc;
	xui_dock_panel_desc_t tBadDesc;
	xui_dock_window_info_t tWinInfo;
	xui_dock_window_info_t tToolboxInfo;
	xui_dock_pane_info_t tPaneInfo;
	xui_dock_hit_t tHit;
	xui_dock_hit_t tExtremeHit;
	xui_dock_drop_info_t tDrop;
	xui_dock_panel_metrics_t tMetrics;
	xui_dock_panel_metrics_t tSavedMetrics;
	xui_dock_panel_colors_t tColors;
	xui_render_node_t tRenderNode;
	xui_accessible_node_t tAccessibleNode;
	xvalue* pState;
	xvalue* pRegions;
	xvalue* pRegionState;
	dock_test_data_t tData;
	dock_destroy_test_data_t tDestroyData;
	dock_replace_close_test_data_t tReplaceCloseData;
	int iFailed;
	int iRet;
	int iRegionCount;
	int iWindowCount;
	int iFloatingCount;
	int iLayerProps;
	int iLayerToolbox;
	int iLayerClient;
	int iLayerNested;
	int iZProps;
	int iZToolbox;
	int iZClient;
	int iZNested;
	int iMenuIndex;
	int iRenderNodeCount;
	int iAccessibleCount;
	int iDragNodeIndex;
	int iClientBefore;
	int iChangeBefore;
	int iLayoutChangeBefore;
	int iWindowChangeBefore;
	int iRegionMode;
	float fOldX;
	float fOldW;
	float fOldY;
	float fOldH;
	float fRegionBefore;
	float fRegionAfter;
	float fAutoHidePaneW;
	float fNoDropX;
	float fNoDropY;
	xui_rect_t tDoc1Tab;
	xui_rect_t tDoc2Tab;
	xui_rect_t tTipRect;
	xui_rect_t tAssetRect;
	xui_rect_t tSrcRect;
	xui_rect_t tDockWorld;
	xui_rect_t tPreviewWorld;
	xui_rect_t tUnrelatedPreview;
	xui_rect_t tClientWorld;
	xui_rect_t tTempButtonWorld;
	xui_event_t tContextEvent;
	xui_widget pDragOverlay;
	float fPreviewCenter;
	float fActualCenter;
	int iExtremeNode;
	int doc1;
	int doc2;
	int doc3;
	int doc4;
	int doc5;
	int toolbox;
	int props;
	int output;
	int scratch1;
	int scratch2;
	int scratch3;
	int docPane;
	int toolboxPane;
	int propsPane;
	int outputPane;
	int scratchPane;
	int dragPane;
	int iActiveBefore;
	int iOverlayChildCount;
	int iTempWindow;
	int iTempWindow2;
	int iCloseBefore;
	int i;
	char sLongTitle[XUI_DOCK_PANEL_TITLE_CAPACITY + 4];

	pContext = NULL;
	pRoot = NULL;
	pDock = NULL;
	pNestedClient = NULL;
	pTempDock = NULL;
	pTempClient = NULL;
	pTempClient2 = NULL;
	pTempButton = NULL;
	pHitWidget = NULL;
	pMenu = NULL;
	pTarget = NULL;
	pIndicatorCache = NULL;
	pDragOverlay = NULL;
	pFont = NULL;
	pFont2 = NULL;
	pState = NULL;
	pRegions = NULL;
	pRegionState = NULL;
	iFailed = 0;
	iRegionCount = iWindowCount = iFloatingCount = 0;
	iLayerProps = iLayerToolbox = iZProps = iZToolbox = 0;
	iLayerClient = iLayerNested = iZClient = iZNested = 0;
	iMenuIndex = -1;
	iRenderNodeCount = 0;
	iAccessibleCount = 0;
	iDragNodeIndex = -1;
	iActiveBefore = 0;
	iOverlayChildCount = 0;
	iTempWindow = -1;
	iTempWindow2 = -1;
	iCloseBefore = 0;
	fRegionBefore = fRegionAfter = 0.0f;
	fAutoHidePaneW = 0.0f;
	fNoDropX = fNoDropY = 0.0f;
	doc1 = doc2 = doc3 = doc4 = doc5 = toolbox = props = output = -1;
	scratch1 = scratch2 = scratch3 = -1;
	docPane = toolboxPane = propsPane = outputPane = scratchPane = -1;
	dragPane = -1;
	memset(arrClient, 0, sizeof(arrClient));
	memset(&tData, 0, sizeof(tData));
	memset(&tDestroyData, 0, sizeof(tDestroyData));
	memset(&tReplaceCloseData, 0, sizeof(tReplaceCloseData));
	xuiTestProxyInit(&tState);
	gDockTextDraw = tState.tProxy.textDraw;
	gDockTrackedFont = NULL;
	gDockTrackedFontDrawCount = 0;
	tState.tProxy.textDraw = __xuiDockTestTextDraw;

	iRet = xuiCreate(&pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && pContext != NULL, "context create");
	iRet = xuiSetProxy(pContext, &tState.tProxy);
	XUI_TEST_CHECK(iRet == XUI_OK, "proxy set");
	iRet = xuiInputViewport(pContext, 640.0f, 420.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "viewport");
	iRet = tState.tProxy.fontLoadMemory(&tState.tProxy, &pFont, "dock", 4, 13.0f, XUI_FONT_FORMAT_TTF);
	XUI_TEST_CHECK(iRet == XUI_OK && pFont != NULL, "font create");
	iRet = tState.tProxy.fontLoadMemory(&tState.tProxy, &pFont2, "dock2", 5, 13.0f, XUI_FONT_FORMAT_TTF);
	XUI_TEST_CHECK(iRet == XUI_OK && pFont2 != NULL, "second font create");
	iRet = xuiSetDefaultFont(pContext, pFont);
	XUI_TEST_CHECK(iRet == XUI_OK, "default font");

	iRet = xuiWidgetCreate(pContext, &pRoot);
	XUI_TEST_CHECK(iRet == XUI_OK && pRoot != NULL, "root create");
	iRet = xuiWidgetSetRect(pRoot, (xui_rect_t){0.0f, 0.0f, 640.0f, 420.0f});
	XUI_TEST_CHECK(iRet == XUI_OK, "root rect");
	iRet = xuiSetRootWidget(pContext, pRoot);
	XUI_TEST_CHECK(iRet == XUI_OK, "root set");

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.pFont = pFont;
	tBadDesc = tDesc;
	tBadDesc.iSize = 1;
	pTempDock = pRoot;
	iRet = xuiDockPanelCreate(pContext, &pTempDock, &tBadDesc);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && pTempDock == NULL,
		"invalid descriptor clears create output");
	tBadDesc = tDesc;
	tBadDesc.bHasMetrics = 1;
	memset(&tBadDesc.tMetrics, 0, sizeof(tBadDesc.tMetrics));
	tBadDesc.tMetrics.iSize = sizeof(tBadDesc.tMetrics);
	tBadDesc.tMetrics.fCaptionHeight = NAN;
	pTempDock = pRoot;
	iRet = xuiDockPanelCreate(pContext, &pTempDock, &tBadDesc);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && pTempDock == NULL,
		"invalid descriptor metrics rejected");
	pTempDock = pRoot;
	iRet = xuiDockPanelCreate(NULL, &pTempDock, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && pTempDock == NULL,
		"invalid context clears create output");
	iRet = xuiDockPanelCreate(pContext, &pDock, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pDock != NULL, "dock create");
	iRet = xuiWidgetSetRect(pDock, (xui_rect_t){8.0f, 8.0f, 624.0f, 404.0f});
	XUI_TEST_CHECK(iRet == XUI_OK, "dock rect");
	iRet = xuiWidgetAddChild(pRoot, pDock);
	XUI_TEST_CHECK(iRet == XUI_OK, "dock add");
	iRet = xuiDockPanelSetWindowStateChanged(pDock, __xuiDockTestState, &tData);
	XUI_TEST_CHECK(iRet == XUI_OK, "state callback");
	iRet = xuiDockPanelSetActiveChanged(pDock, __xuiDockTestActive, &tData);
	XUI_TEST_CHECK(iRet == XUI_OK, "active callback");
	iRet = xuiDockPanelSetWindowClose(pDock, __xuiDockTestClose, &tData);
	XUI_TEST_CHECK(iRet == XUI_OK, "close callback");
	iRet = xuiDockPanelSetContextMenu(pDock, __xuiDockTestContext, &tData);
	XUI_TEST_CHECK(iRet == XUI_OK, "context callback");
	iTempWindow = 123;
	iRet = xuiDockPanelAddWindow(NULL, "invalid", NULL, &iTempWindow);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && iTempWindow == -1,
		"add window failure clears output");

	for ( i = 0; i < 11; i++ ) {
		iRet = xuiWidgetCreate(pContext, &arrClient[i]);
		XUI_TEST_CHECK(iRet == XUI_OK && arrClient[i] != NULL, "client create");
		iRet = xuiWidgetSetFocusable(arrClient[i], 1);
		XUI_TEST_CHECK(iRet == XUI_OK, "client focusable");
		iRet = xuiWidgetSetTabStop(arrClient[i], 0);
		XUI_TEST_CHECK(iRet == XUI_OK, "client tab stop");
	}
	iRet = xuiWidgetCreate(pContext, &pNestedClient);
	XUI_TEST_CHECK(iRet == XUI_OK && pNestedClient != NULL, "nested client create");
	iRet = xuiWidgetSetFocusable(pNestedClient, 1);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pNestedClient, XUI_EVENT_POINTER_DOWN, __xuiDockTestClientEvent, &tData);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetRect(pNestedClient, (xui_rect_t){12.0f, 12.0f, 80.0f, 28.0f});
	if ( iRet == XUI_OK ) iRet = xuiWidgetAddChild(arrClient[6], pNestedClient);
	XUI_TEST_CHECK(iRet == XUI_OK, "nested client add");
	iRet = xuiDockPanelAddWindow(pDock, "Document.c", arrClient[0], &doc1);
	XUI_TEST_CHECK(iRet == XUI_OK, "doc1 add");
	iTempWindow = 123;
	iRet = xuiDockPanelAddWindow(pDock, "Duplicate client", arrClient[0], &iTempWindow);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && iTempWindow == -1 &&
		xuiDockPanelGetWindowCount(pDock) == 1 &&
		xuiDockPanelGetWindowClient(pDock, doc1) == arrClient[0],
		"add window rejects client already owned by another window");
	memset(sLongTitle, 't', XUI_DOCK_PANEL_TITLE_CAPACITY - 2);
	memcpy(sLongTitle + XUI_DOCK_PANEL_TITLE_CAPACITY - 2, "\xe4\xb8\xad", 4);
	iRet = xuiDockPanelSetWindowTitle(pDock, doc1, sLongTitle);
	XUI_TEST_CHECK(iRet == XUI_OK &&
		strlen(xuiDockPanelGetWindowTitle(pDock, doc1)) == XUI_DOCK_PANEL_TITLE_CAPACITY - 2,
		"window title truncation preserves utf8 boundary");
	iRet = xuiDockPanelGetWindowInfo(pDock, doc1, &tWinInfo);
	XUI_TEST_CHECK(iRet == XUI_OK && strlen(tWinInfo.sTitle) == XUI_DOCK_PANEL_TITLE_CAPACITY - 2,
		"window info title remains valid after utf8 truncation");
	iRet = xuiDockPanelSetWindowTitle(pDock, doc1, "Document.c");
	XUI_TEST_CHECK(iRet == XUI_OK, "doc1 title restore");
	iRet = xuiDockPanelAddWindow(pDock, "Preview.h", arrClient[1], &doc2);
	XUI_TEST_CHECK(iRet == XUI_OK, "doc2 add");
	iRet = xuiDockPanelAddWindow(pDock, "Scene.json", arrClient[2], &doc3);
	XUI_TEST_CHECK(iRet == XUI_OK, "doc3 add");
	iRet = xuiDockPanelAddWindow(pDock, "Readme.md", arrClient[3], &doc4);
	XUI_TEST_CHECK(iRet == XUI_OK, "doc4 add");
	iRet = xuiDockPanelAddWindow(pDock, "Notes.txt", arrClient[4], &doc5);
	XUI_TEST_CHECK(iRet == XUI_OK, "doc5 add");
	iRet = xuiDockPanelAddWindow(pDock, "Toolbox", arrClient[5], &toolbox);
	XUI_TEST_CHECK(iRet == XUI_OK, "toolbox add");
	iRet = xuiDockPanelAddWindow(pDock, "Properties", arrClient[6], &props);
	XUI_TEST_CHECK(iRet == XUI_OK, "props add");
	iRet = xuiDockPanelAddWindow(pDock, "Output", arrClient[7], &output);
	XUI_TEST_CHECK(iRet == XUI_OK, "output add");
	iRet = xuiDockPanelAddWindow(pDock, "Scratch A", arrClient[8], &scratch1);
	XUI_TEST_CHECK(iRet == XUI_OK, "scratch1 add");
	iRet = xuiDockPanelAddWindow(pDock, "Scratch B", arrClient[9], &scratch2);
	XUI_TEST_CHECK(iRet == XUI_OK, "scratch2 add");
	iRet = xuiDockPanelAddWindow(pDock, "Scratch C", arrClient[10], &scratch3);
	XUI_TEST_CHECK(iRet == XUI_OK, "scratch3 add");

	iRet = xuiDockPanelDockWindow(pDock, doc1, XUI_DOCK_PANEL_REGION_DOCUMENT, XUI_DOCK_PANEL_SIDE_FILL, 0.0f, &docPane);
	XUI_TEST_CHECK(iRet == XUI_OK && docPane >= 0, "dock doc1");
	iRet = xuiDockPanelDockWindowToPane(pDock, doc2, docPane);
	XUI_TEST_CHECK(iRet == XUI_OK, "dock doc2 to tab");
	iRet = xuiDockPanelDockWindowToPane(pDock, doc3, docPane);
	XUI_TEST_CHECK(iRet == XUI_OK, "dock doc3 to tab");
	iRet = xuiDockPanelDockWindowToPane(pDock, doc4, docPane);
	XUI_TEST_CHECK(iRet == XUI_OK, "dock doc4 to tab");
	iRet = xuiDockPanelDockWindowToPane(pDock, doc5, docPane);
	XUI_TEST_CHECK(iRet == XUI_OK, "dock doc5 to tab");
	iRet = xuiDockPanelSetPaneActiveWindow(pDock, docPane, doc2);
	XUI_TEST_CHECK(iRet == XUI_OK, "set initial active doc2");
	iRet = xuiDockPanelDockWindow(pDock, toolbox, XUI_DOCK_PANEL_REGION_DOCUMENT, XUI_DOCK_PANEL_SIDE_LEFT, 0.22f, &toolboxPane);
	XUI_TEST_CHECK(iRet == XUI_OK && toolboxPane >= 0, "dock toolbox");
	iRet = xuiDockPanelDockWindow(pDock, props, XUI_DOCK_PANEL_REGION_DOCUMENT, XUI_DOCK_PANEL_SIDE_RIGHT, 0.24f, &propsPane);
	XUI_TEST_CHECK(iRet == XUI_OK && propsPane >= 0, "dock props");
	iRet = xuiDockPanelDockWindow(pDock, output, XUI_DOCK_PANEL_REGION_DOCUMENT, XUI_DOCK_PANEL_SIDE_BOTTOM, 0.25f, &outputPane);
	XUI_TEST_CHECK(iRet == XUI_OK && outputPane >= 0, "dock output");

	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "layout");
	iRet = xuiDockPanelSaveState(pDock, &pState);
	XUI_TEST_CHECK(iRet == XUI_OK && pState != NULL, "save extreme drag baseline");
	iRet = xuiDockPanelGetMetrics(pDock, &tMetrics);
	XUI_TEST_CHECK(iRet == XUI_OK && __xuiDockNear(tMetrics.fSplitterSize, 3.0f) &&
		tMetrics.fSplitterHitSize > tMetrics.fSplitterSize, "splitter visual and hit metrics");
	tSavedMetrics = tMetrics;
	tMetrics.fCaptionHeight = NAN;
	iRet = xuiDockPanelSetMetrics(pDock, &tMetrics);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT, "metrics reject NaN");
	tMetrics = tSavedMetrics;
	tMetrics.fTabMaxWidth = tMetrics.fTabMinWidth - 1.0f;
	iRet = xuiDockPanelSetMetrics(pDock, &tMetrics);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT, "metrics reject inverted tab bounds");
	iRet = xuiDockPanelGetMetrics(pDock, &tMetrics);
	XUI_TEST_CHECK(iRet == XUI_OK && memcmp(&tMetrics, &tSavedMetrics, sizeof(tMetrics)) == 0,
		"invalid metrics preserve previous state");
	iChangeBefore = xuiDockPanelGetChangeCount(pDock);
	iLayoutChangeBefore = xuiDockPanelGetLayoutChangeCount(pDock);
	iRet = xuiDockPanelGetRegionSize(pDock, XUI_DOCK_PANEL_REGION_LEFT, &iRegionMode, &fRegionBefore);
	if ( iRet == XUI_OK ) iRet = xuiDockPanelSetRegionSize(pDock, XUI_DOCK_PANEL_REGION_LEFT, iRegionMode, NAN);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT &&
		xuiDockPanelGetChangeCount(pDock) == iChangeBefore &&
		xuiDockPanelGetLayoutChangeCount(pDock) == iLayoutChangeBefore,
		"region size rejects NaN without mutation");
	iRet = xuiDockPanelSetRegionSize(pDock, XUI_DOCK_PANEL_REGION_LEFT, iRegionMode, fRegionBefore);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiDockPanelGetChangeCount(pDock) == iChangeBefore &&
		xuiDockPanelGetLayoutChangeCount(pDock) == iLayoutChangeBefore,
		"region size no-op does not dirty layout");
	iRet = xuiDockPanelSetWindowMinSize(pDock, doc1, INFINITY, 100.0f);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT, "window minimum rejects infinity");
	iRet = xuiDockPanelSetWindowIcon(pDock, doc1, NULL, (xui_rect_t){NAN, 0.0f, 0.0f, 0.0f});
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT, "window icon source rejects NaN");
	dragPane = 123;
	iRet = xuiDockPanelDockWindow(pDock, doc1, XUI_DOCK_PANEL_REGION_DOCUMENT,
		XUI_DOCK_PANEL_SIDE_FILL, NAN, &dragPane);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && dragPane == -1,
		"dock ratio rejects NaN and clears pane output");
	iRet = xuiDockPanelFloatWindow(pDock, doc1, (xui_rect_t){0.0f, 0.0f, INFINITY, 120.0f});
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT, "float rect rejects infinity");
	iRet = xuiDockPanelGetColors(pDock, &tColors);
	XUI_TEST_CHECK(iRet == XUI_OK, "splitter colors");
	tDockWorld = xuiWidgetGetWorldRect(pDock);

	XUI_TEST_CHECK(__xuiDockFindNodeSplitter(pDock, XUI_ORIENTATION_VERTICAL, -1, &tExtremeHit),
		"vertical extreme splitter found");
	iExtremeNode = tExtremeHit.iNode;
	iRet = __xuiDockTestDragMove(pContext,
		tDockWorld.fX + tExtremeHit.tRect.fX + tExtremeHit.tRect.fW * 0.5f,
		tDockWorld.fY + tExtremeHit.tRect.fY + tExtremeHit.tRect.fH * 0.5f,
		tDockWorld.fX + tDockWorld.fW + 300.0f,
		tDockWorld.fY + tExtremeHit.tRect.fY + tExtremeHit.tRect.fH * 0.5f);
	pDragOverlay = xuiOverlayTop(pContext);
	if ( iRet == XUI_OK ) iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && pDragOverlay != NULL && xuiOverlayGetOwner(pDragOverlay) == pDock,
		"vertical extreme preview overlay");
	tPreviewWorld = xuiWidgetGetWorldRect(pDragOverlay);
	fPreviewCenter = tPreviewWorld.fX + tPreviewWorld.fW * 0.5f;
	XUI_TEST_CHECK(__xuiDockNear(tPreviewWorld.fW - 2.0f, tMetrics.fSplitterSize),
		"vertical preview uses splitter visual width");
	iRet = xuiRenderPrepare(pContext);
	pCache = xuiWidgetGetCacheSurface(pDragOverlay, xuiWidgetGetStateId(pDragOverlay));
	XUI_TEST_CHECK(iRet == XUI_OK && pCache != NULL &&
		xuiTestSurfaceGetLastColor(pCache) == tColors.iSplitterActiveColor,
		"vertical preview uses active splitter color");
	iRet = __xuiDockTestDragUp(pContext, tDockWorld.fX + tDockWorld.fW + 300.0f,
		tDockWorld.fY + tExtremeHit.tRect.fY + tExtremeHit.tRect.fH * 0.5f);
	if ( iRet == XUI_OK ) iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK &&
		__xuiDockFindNodeSplitter(pDock, XUI_ORIENTATION_VERTICAL, iExtremeNode, &tExtremeHit),
		"vertical extreme splitter committed");
	fActualCenter = tDockWorld.fX + tExtremeHit.tRect.fX + tExtremeHit.tRect.fW * 0.5f;
	XUI_TEST_CHECK(__xuiDockNear(fPreviewCenter, fActualCenter),
		"vertical preview matches committed splitter");

	XUI_TEST_CHECK(__xuiDockFindNodeSplitter(pDock, XUI_ORIENTATION_HORIZONTAL, -1, &tExtremeHit),
		"horizontal extreme splitter found");
	iExtremeNode = tExtremeHit.iNode;
	iRet = __xuiDockTestDragMove(pContext,
		tDockWorld.fX + tExtremeHit.tRect.fX + tExtremeHit.tRect.fW * 0.5f,
		tDockWorld.fY + tExtremeHit.tRect.fY + tExtremeHit.tRect.fH * 0.5f,
		tDockWorld.fX + tExtremeHit.tRect.fX + tExtremeHit.tRect.fW * 0.5f,
		tDockWorld.fY + tDockWorld.fH + 300.0f);
	pDragOverlay = xuiOverlayTop(pContext);
	if ( iRet == XUI_OK ) iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && pDragOverlay != NULL && xuiOverlayGetOwner(pDragOverlay) == pDock,
		"horizontal extreme preview overlay");
	tPreviewWorld = xuiWidgetGetWorldRect(pDragOverlay);
	fPreviewCenter = tPreviewWorld.fY + tPreviewWorld.fH * 0.5f;
	XUI_TEST_CHECK(__xuiDockNear(tPreviewWorld.fH - 2.0f, tMetrics.fSplitterSize),
		"horizontal preview uses splitter visual height");
	iRet = xuiRenderPrepare(pContext);
	pCache = xuiWidgetGetCacheSurface(pDragOverlay, xuiWidgetGetStateId(pDragOverlay));
	XUI_TEST_CHECK(iRet == XUI_OK && pCache != NULL &&
		xuiTestSurfaceGetLastColor(pCache) == tColors.iSplitterActiveColor,
		"horizontal preview uses active splitter color");
	iRet = __xuiDockTestDragUp(pContext,
		tDockWorld.fX + tExtremeHit.tRect.fX + tExtremeHit.tRect.fW * 0.5f,
		tDockWorld.fY + tDockWorld.fH + 300.0f);
	if ( iRet == XUI_OK ) iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK &&
		__xuiDockFindNodeSplitter(pDock, XUI_ORIENTATION_HORIZONTAL, iExtremeNode, &tExtremeHit),
		"horizontal extreme splitter committed");
	fActualCenter = tDockWorld.fY + tExtremeHit.tRect.fY + tExtremeHit.tRect.fH * 0.5f;
	XUI_TEST_CHECK(__xuiDockNear(fPreviewCenter, fActualCenter),
		"horizontal preview matches committed splitter");

	XUI_TEST_CHECK(__xuiDockFindNodeSplitter(pDock, XUI_ORIENTATION_VERTICAL, -1, &tExtremeHit),
		"vertical reverse splitter found");
	iExtremeNode = tExtremeHit.iNode;
	iRet = __xuiDockTestDragMove(pContext,
		tDockWorld.fX + tExtremeHit.tRect.fX + tExtremeHit.tRect.fW * 0.5f,
		tDockWorld.fY + tExtremeHit.tRect.fY + tExtremeHit.tRect.fH * 0.5f,
		tDockWorld.fX - 300.0f,
		tDockWorld.fY + tExtremeHit.tRect.fY + tExtremeHit.tRect.fH * 0.5f);
	pDragOverlay = xuiOverlayTop(pContext);
	if ( iRet == XUI_OK ) iRet = xuiLayout(pContext);
	tPreviewWorld = pDragOverlay != NULL ? xuiWidgetGetWorldRect(pDragOverlay) : (xui_rect_t){0};
	fPreviewCenter = tPreviewWorld.fX + tPreviewWorld.fW * 0.5f;
	XUI_TEST_CHECK(iRet == XUI_OK && pDragOverlay != NULL && xuiOverlayGetOwner(pDragOverlay) == pDock,
		"vertical reverse preview overlay");
	iRet = __xuiDockTestDragUp(pContext, tDockWorld.fX - 300.0f,
		tDockWorld.fY + tExtremeHit.tRect.fY + tExtremeHit.tRect.fH * 0.5f);
	if ( iRet == XUI_OK ) iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK &&
		__xuiDockFindNodeSplitter(pDock, XUI_ORIENTATION_VERTICAL, iExtremeNode, &tExtremeHit),
		"vertical reverse splitter committed");
	fActualCenter = tDockWorld.fX + tExtremeHit.tRect.fX + tExtremeHit.tRect.fW * 0.5f;
	XUI_TEST_CHECK(__xuiDockNear(fPreviewCenter, fActualCenter),
		"vertical reverse preview matches committed splitter");

	XUI_TEST_CHECK(__xuiDockFindNodeSplitter(pDock, XUI_ORIENTATION_HORIZONTAL, -1, &tExtremeHit),
		"horizontal reverse splitter found");
	iExtremeNode = tExtremeHit.iNode;
	iRet = __xuiDockTestDragMove(pContext,
		tDockWorld.fX + tExtremeHit.tRect.fX + tExtremeHit.tRect.fW * 0.5f,
		tDockWorld.fY + tExtremeHit.tRect.fY + tExtremeHit.tRect.fH * 0.5f,
		tDockWorld.fX + tExtremeHit.tRect.fX + tExtremeHit.tRect.fW * 0.5f,
		tDockWorld.fY - 300.0f);
	pDragOverlay = xuiOverlayTop(pContext);
	if ( iRet == XUI_OK ) iRet = xuiLayout(pContext);
	tPreviewWorld = pDragOverlay != NULL ? xuiWidgetGetWorldRect(pDragOverlay) : (xui_rect_t){0};
	fPreviewCenter = tPreviewWorld.fY + tPreviewWorld.fH * 0.5f;
	XUI_TEST_CHECK(iRet == XUI_OK && pDragOverlay != NULL && xuiOverlayGetOwner(pDragOverlay) == pDock,
		"horizontal reverse preview overlay");
	iRet = __xuiDockTestDragUp(pContext,
		tDockWorld.fX + tExtremeHit.tRect.fX + tExtremeHit.tRect.fW * 0.5f,
		tDockWorld.fY - 300.0f);
	if ( iRet == XUI_OK ) iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK &&
		__xuiDockFindNodeSplitter(pDock, XUI_ORIENTATION_HORIZONTAL, iExtremeNode, &tExtremeHit),
		"horizontal reverse splitter committed");
	fActualCenter = tDockWorld.fY + tExtremeHit.tRect.fY + tExtremeHit.tRect.fH * 0.5f;
	XUI_TEST_CHECK(__xuiDockNear(fPreviewCenter, fActualCenter),
		"horizontal reverse preview matches committed splitter");
	XUI_TEST_CHECK(__xuiDockFindNodeSplitter(pDock, XUI_ORIENTATION_VERTICAL, -1, &tExtremeHit),
		"multi-touch splitter found");
	iRet = xuiInputPointerDownEx(pContext, 1801u, XUI_POINTER_TYPE_TOUCH,
		(int)(tDockWorld.fX + tExtremeHit.tRect.fX + tExtremeHit.tRect.fW * 0.5f),
		(int)(tDockWorld.fY + tExtremeHit.tRect.fY + tExtremeHit.tRect.fH * 0.5f),
		XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK &&
		xuiGetPointerCaptureEx(pContext, 1801u, XUI_POINTER_TYPE_TOUCH) == pDock,
		"dock splitter captures exact owner touch");
	iRet = xuiInputPointerMoveEx(pContext, 1801u, XUI_POINTER_TYPE_TOUCH,
		(int)(tDockWorld.fX + tExtremeHit.tRect.fX + tExtremeHit.tRect.fW * 0.5f + 24.0f),
		(int)(tDockWorld.fY + tExtremeHit.tRect.fY + tExtremeHit.tRect.fH * 0.5f),
		XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	if ( iRet == XUI_OK ) iRet = xuiLayout(pContext);
	pDragOverlay = xuiOverlayTop(pContext);
	tPreviewWorld = pDragOverlay != NULL ? xuiWidgetGetWorldRect(pDragOverlay) : (xui_rect_t){0};
	XUI_TEST_CHECK(iRet == XUI_OK && pDragOverlay != NULL && xuiOverlayGetOwner(pDragOverlay) == pDock,
		"owner touch moves dock splitter preview");
	iRet = xuiInputPointerDownEx(pContext, 1802u, XUI_POINTER_TYPE_TOUCH,
		(int)(tDockWorld.fX + tExtremeHit.tRect.fX + 4.0f),
		(int)(tDockWorld.fY + tExtremeHit.tRect.fY + 4.0f),
		XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	if ( iRet == XUI_OK ) iRet = xuiInputPointerMoveEx(pContext, 1802u,
		XUI_POINTER_TYPE_TOUCH, (int)(tDockWorld.fX + tDockWorld.fW - 4.0f),
		(int)(tDockWorld.fY + tDockWorld.fH - 4.0f), XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	if ( iRet == XUI_OK ) iRet = xuiInputPointerUpEx(pContext, 1802u,
		XUI_POINTER_TYPE_TOUCH, (int)(tDockWorld.fX + tDockWorld.fW - 4.0f),
		(int)(tDockWorld.fY + tDockWorld.fH - 4.0f), XUI_POINTER_BUTTON_LEFT, 0);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	if ( iRet == XUI_OK ) iRet = xuiLayout(pContext);
	tUnrelatedPreview = pDragOverlay != NULL ? xuiWidgetGetWorldRect(pDragOverlay) : (xui_rect_t){0};
	XUI_TEST_CHECK(iRet == XUI_OK &&
		__xuiDockNear(tUnrelatedPreview.fX, tPreviewWorld.fX) &&
		__xuiDockNear(tUnrelatedPreview.fY, tPreviewWorld.fY) &&
		xuiGetPointerCaptureEx(pContext, 1801u, XUI_POINTER_TYPE_TOUCH) == pDock &&
		xuiGetPointerCaptureEx(pContext, 1802u, XUI_POINTER_TYPE_TOUCH) == NULL,
		"unrelated touch cannot move dock splitter or release owner");
	iRet = xuiInputPointerMoveEx(pContext, 1801u, XUI_POINTER_TYPE_TOUCH,
		(int)(tDockWorld.fX + tExtremeHit.tRect.fX + tExtremeHit.tRect.fW * 0.5f + 52.0f),
		(int)(tDockWorld.fY + tExtremeHit.tRect.fY + tExtremeHit.tRect.fH * 0.5f),
		XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	if ( iRet == XUI_OK ) iRet = xuiLayout(pContext);
	tUnrelatedPreview = pDragOverlay != NULL ? xuiWidgetGetWorldRect(pDragOverlay) : (xui_rect_t){0};
	XUI_TEST_CHECK(iRet == XUI_OK && !__xuiDockNear(tUnrelatedPreview.fX, tPreviewWorld.fX),
		"dock owner touch continues moving splitter preview");
	iRet = xuiInputPointerUpEx(pContext, 1801u, XUI_POINTER_TYPE_TOUCH,
		(int)(tDockWorld.fX + tExtremeHit.tRect.fX + tExtremeHit.tRect.fW * 0.5f + 52.0f),
		(int)(tDockWorld.fY + tExtremeHit.tRect.fY + tExtremeHit.tRect.fH * 0.5f),
		XUI_POINTER_BUTTON_LEFT, 0);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK &&
		xuiGetPointerCaptureEx(pContext, 1801u, XUI_POINTER_TYPE_TOUCH) == NULL,
		"dock owner touch releases splitter capture");
	iRet = xuiDockPanelLoadState(pDock, pState);
	xuiDockPanelStateFree(pState);
	pState = NULL;
	if ( iRet == XUI_OK ) iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "restore extreme drag baseline");
	iRet = xuiDockPanelGetWindowInfo(pDock, doc1, &tWinInfo);
	if ( iRet == XUI_OK ) docPane = tWinInfo.iPane;
	if ( iRet == XUI_OK ) iRet = xuiDockPanelGetWindowInfo(pDock, toolbox, &tWinInfo);
	if ( iRet == XUI_OK ) toolboxPane = tWinInfo.iPane;
	if ( iRet == XUI_OK ) iRet = xuiDockPanelGetWindowInfo(pDock, props, &tWinInfo);
	if ( iRet == XUI_OK ) propsPane = tWinInfo.iPane;
	if ( iRet == XUI_OK ) iRet = xuiDockPanelGetWindowInfo(pDock, output, &tWinInfo);
	if ( iRet == XUI_OK ) outputPane = tWinInfo.iPane;
	XUI_TEST_CHECK(iRet == XUI_OK && docPane >= 0 && toolboxPane >= 0 && propsPane >= 0 && outputPane >= 0,
		"refresh restored pane ids");
	XUI_TEST_CHECK(xuiDockPanelGetWindowCount(pDock) == 11, "window count");
	XUI_TEST_CHECK(xuiDockPanelGetPaneCount(pDock) == 4, "pane count");
	XUI_TEST_CHECK(xuiDockPanelGetPaneWindowCount(pDock, docPane) == 5, "doc tab count");
	XUI_TEST_CHECK(xuiDockPanelGetPaneActiveWindow(pDock, docPane) == doc2, "doc2 active");
	iAccessibleCount = xuiWidgetGetAccessibleNodeCount(pDock);
	memset(&tAccessibleNode, 0, sizeof(tAccessibleNode));
	tAccessibleNode.iSize = sizeof(tAccessibleNode);
	iRet = xuiWidgetGetAccessibleNode(pDock, 0, &tAccessibleNode);
	XUI_TEST_CHECK(iRet == XUI_OK && iAccessibleCount == 9 &&
		tAccessibleNode.iRole == XUI_ACCESSIBLE_ROLE_GROUP &&
		(tAccessibleNode.iActions & XUI_ACCESSIBLE_ACTION_MASK(XUI_ACCESSIBLE_ACTION_FOCUS)) != 0u,
		"dock accessibility exposes root and visible windows");
	memset(&tAccessibleNode, 0, sizeof(tAccessibleNode));
	tAccessibleNode.iSize = sizeof(tAccessibleNode);
	iRet = xuiWidgetGetAccessibleNode(pDock, 1, &tAccessibleNode);
	XUI_TEST_CHECK(iRet == XUI_OK && tAccessibleNode.iId == (uint64_t)doc1 + 2u &&
		tAccessibleNode.iRole == XUI_ACCESSIBLE_ROLE_TAB && strcmp(tAccessibleNode.sName, "Document.c") == 0,
		"dock accessibility exposes document tab");
	iRet = xuiWidgetPerformAccessibleAction(pDock, (uint64_t)doc1 + 2u,
		XUI_ACCESSIBLE_ACTION_SET_SELECTION, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiDockPanelGetPaneActiveWindow(pDock, docPane) == doc1,
		"dock accessibility selects tab");
	iRet = xuiDockPanelSetPaneActiveWindow(pDock, docPane, doc2);
	if ( iRet == XUI_OK ) iRet = xuiInputKeyDown(pContext, XUI_KEY_TAB, XUI_MOD_CTRL);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiDockPanelGetPaneActiveWindow(pDock, docPane) == doc3,
		"ctrl tab switches dock tabs from client focus");
	iRet = xuiInputKeyDown(pContext, XUI_KEY_TAB, XUI_MOD_CTRL | XUI_MOD_SHIFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiDockPanelGetPaneActiveWindow(pDock, docPane) == doc2,
		"ctrl shift tab switches dock tabs backward");
	iRet = xuiInputSetModifiers(pContext, 0u);
	XUI_TEST_CHECK(iRet == XUI_OK, "ctrl tab restores input modifiers");
	iRet = xuiDockPanelDockWindowToPane(pDock, toolbox, toolboxPane);
	if ( iRet == XUI_OK ) iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK &&
		xuiDockPanelGetPaneCount(pDock) == 4 &&
		xuiDockPanelGetPaneWindowCount(pDock, toolboxPane) == 1 &&
		xuiDockPanelGetPaneActiveWindow(pDock, toolboxPane) == toolbox,
		"single window self-dock preserves pane tree");
	iRet = xuiDockPanelGetWindowInfo(pDock, props, &tWinInfo);
	XUI_TEST_CHECK(iRet == XUI_OK && tWinInfo.pClientWidget == arrClient[6], "client replacement baseline");
	iRet = xuiDockPanelSetWindowClient(pDock, props, pDock);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT &&
		xuiWidgetGetParent(pDock) == pRoot &&
		xuiDockPanelGetWindowClient(pDock, props) == arrClient[6] &&
		xuiWidgetGetParent(arrClient[6]) == tWinInfo.pHostWidget,
		"invalid client replacement has no side effects");
	iRet = xuiDockPanelGetWindowInfo(pDock, doc1, &tToolboxInfo);
	XUI_TEST_CHECK(iRet == XUI_OK, "duplicate client replacement source info");
	iRet = xuiDockPanelSetWindowClient(pDock, props, arrClient[0]);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT &&
		xuiDockPanelGetWindowClient(pDock, props) == arrClient[6] &&
		xuiDockPanelGetWindowClient(pDock, doc1) == arrClient[0] &&
		xuiWidgetGetParent(arrClient[6]) == tWinInfo.pHostWidget &&
		xuiWidgetGetParent(arrClient[0]) == tToolboxInfo.pHostWidget,
		"client replacement rejects client owned by another window");
	pTempButton = xuiDockPanelGetWindowCloseButtonWidget(pDock, doc1);
	iRet = xuiDockPanelSetWindowClient(pDock, props, pTempButton);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT &&
		xuiDockPanelGetWindowClient(pDock, props) == arrClient[6] &&
		xuiWidgetGetParent(pTempButton) == tToolboxInfo.pHostWidget,
		"client replacement rejects internal window chrome");
	pTempButton = NULL;
	iRet = xuiDockPanelGetPaneInfo(pDock, docPane, &tPaneInfo);
	XUI_TEST_CHECK(iRet == XUI_OK && tPaneInfo.tTabStripRect.fW > 0.0f && tPaneInfo.iVisibleTabCount > 1, "multi window pane shows tabs");
	iRet = xuiDockPanelGetPaneInfo(pDock, toolboxPane, &tPaneInfo);
	XUI_TEST_CHECK(iRet == XUI_OK && tPaneInfo.iWindowCount == 1 && tPaneInfo.tCaptionRect.fW > 0.0f && tPaneInfo.tCaptionRect.fH > 0.0f, "single window pane shows caption");
	XUI_TEST_CHECK(tPaneInfo.tTabStripRect.fW <= 0.0f && tPaneInfo.tTabStripRect.fH <= 0.0f, "single window pane hides tab strip");
	iRet = xuiDockPanelGetWindowInfo(pDock, toolbox, &tWinInfo);
	XUI_TEST_CHECK(iRet == XUI_OK && tWinInfo.tTabRect.fW <= 0.0f && tWinInfo.tTabRect.fH <= 0.0f, "single window has no tab rect");
	iRet = xuiDockPanelHitTest(pDock, tPaneInfo.tCaptionRect.fX + 8.0f, tPaneInfo.tCaptionRect.fY + tPaneInfo.tCaptionRect.fH * 0.5f, &tHit);
	XUI_TEST_CHECK(iRet == XUI_OK && tHit.iType == XUI_DOCK_PANEL_HIT_PANE_CAPTION && tHit.iWindow == toolbox, "single window caption hit");

	iRet = xuiDockPanelSetWindowFlags(pDock, toolbox, 1, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "toolbox non dockable");
	iRet = xuiDockPanelOpenPaneMenu(pDock, toolboxPane);
	XUI_TEST_CHECK(iRet == XUI_OK, "non dockable pane menu open");
	pMenu = xuiDockPanelGetOptionMenu(pDock);
	iMenuIndex = __xuiDockFindMenuValue(pMenu, XUI_DOCK_PANEL_MENU_FLOAT);
	XUI_TEST_CHECK(iMenuIndex >= 0 && (xuiMenuGetItemState(pMenu, iMenuIndex) & XUI_MENU_ITEM_ENABLED) == 0u, "non dockable float disabled");
	iMenuIndex = __xuiDockFindMenuValue(pMenu, XUI_DOCK_PANEL_MENU_AUTO_HIDE);
	XUI_TEST_CHECK(iMenuIndex >= 0 && (xuiMenuGetItemState(pMenu, iMenuIndex) & XUI_MENU_ITEM_ENABLED) == 0u, "non dockable auto hide disabled");
	(void)xuiMenuClose(pMenu);
	iRet = xuiDockPanelFloatWindow(pDock, toolbox, (xui_rect_t){96.0f, 72.0f, 190.0f, 135.0f});
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT, "non dockable float api disabled");
	iRet = xuiDockPanelDockWindow(pDock, toolbox, XUI_DOCK_PANEL_REGION_RIGHT, XUI_DOCK_PANEL_SIDE_FILL, 0.0f, NULL);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT, "non dockable region dock disabled");
	iRet = xuiDockPanelDockWindowToPane(pDock, toolbox, docPane);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT, "non dockable pane dock disabled");
	iRet = xuiDockPanelDockWindowToPaneSide(pDock, toolbox, docPane, XUI_DOCK_PANEL_SIDE_RIGHT, 0.22f, NULL);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT, "non dockable pane side dock disabled");
	iRet = xuiDockPanelAutoHideWindow(pDock, toolbox);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT, "non dockable auto hide api disabled");
	iRet = xuiDockPanelGetPaneInfo(pDock, docPane, &tPaneInfo);
	XUI_TEST_CHECK(iRet == XUI_OK && tPaneInfo.tRect.fW > 0.0f, "non dockable drop target pane info");
	iRet = xuiDockPanelFindDropTarget(pDock, toolbox, tPaneInfo.tRect.fX + tPaneInfo.tRect.fW * 0.5f, tPaneInfo.tRect.fY + tPaneInfo.tRect.fH * 0.5f, &tDrop);
	XUI_TEST_CHECK(iRet == XUI_OK && !tDrop.bValid, "non dockable drop target disabled");
	iRet = xuiDockPanelSetWindowFlags(pDock, toolbox, 1, 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "toolbox dockable restored");

	iRet = xuiDockPanelDockWindow(pDock, scratch1, XUI_DOCK_PANEL_REGION_LEFT, XUI_DOCK_PANEL_SIDE_FILL, 0.0f, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK, "side region dock");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "side region layout");
	XUI_TEST_CHECK(__xuiDockFindRegionSplitter(pDock, XUI_DOCK_PANEL_REGION_LEFT, &tHit), "side region splitter hit");
	iRet = xuiDockPanelGetRegionSize(pDock, XUI_DOCK_PANEL_REGION_LEFT, NULL, &fRegionBefore);
	XUI_TEST_CHECK(iRet == XUI_OK, "side region size before");
	tDockWorld = xuiWidgetGetWorldRect(pDock);
	iRet = __xuiDockTestDragMove(pContext,
		tDockWorld.fX + tHit.tRect.fX + tHit.tRect.fW * 0.5f,
		tDockWorld.fY + tHit.tRect.fY + tHit.tRect.fH * 0.5f,
		tDockWorld.fX + tDockWorld.fW + 300.0f,
		tDockWorld.fY + tHit.tRect.fY + tHit.tRect.fH * 0.5f);
	if ( iRet == XUI_OK ) iRet = xuiDockPanelGetRegionSize(pDock, XUI_DOCK_PANEL_REGION_LEFT, NULL, &fRegionAfter);
	XUI_TEST_CHECK(iRet == XUI_OK && fRegionAfter == fRegionBefore, "side region splitter move remains deferred");
	pDragOverlay = xuiOverlayTop(pContext);
	if ( iRet == XUI_OK ) iRet = xuiLayout(pContext);
	tPreviewWorld = pDragOverlay != NULL ? xuiWidgetGetWorldRect(pDragOverlay) : (xui_rect_t){0};
	fPreviewCenter = tPreviewWorld.fX + tPreviewWorld.fW * 0.5f;
	XUI_TEST_CHECK(iRet == XUI_OK && pDragOverlay != NULL && xuiOverlayGetOwner(pDragOverlay) == pDock &&
		__xuiDockNear(tPreviewWorld.fW - 2.0f, tMetrics.fSplitterSize),
		"side region extreme preview geometry");
	if ( iRet == XUI_OK ) iRet = __xuiDockTestDragUp(pContext,
		tDockWorld.fX + tDockWorld.fW + 300.0f,
		tDockWorld.fY + tHit.tRect.fY + tHit.tRect.fH * 0.5f);
	if ( iRet == XUI_OK ) iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "side region splitter drag");
	iRet = xuiDockPanelGetRegionSize(pDock, XUI_DOCK_PANEL_REGION_LEFT, NULL, &fRegionAfter);
	XUI_TEST_CHECK(iRet == XUI_OK && fRegionAfter > fRegionBefore + 0.03f &&
		__xuiDockFindRegionSplitter(pDock, XUI_DOCK_PANEL_REGION_LEFT, &tHit),
		"side region splitter updates size");
	fActualCenter = tDockWorld.fX + tHit.tRect.fX + tHit.tRect.fW * 0.5f;
	XUI_TEST_CHECK(__xuiDockNear(fPreviewCenter, fActualCenter),
		"side region preview matches committed splitter");
	iRet = xuiDockPanelHideWindow(pDock, scratch1);
	XUI_TEST_CHECK(iRet == XUI_OK, "side region cleanup hide");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "side region cleanup layout");

	iRet = xuiDockPanelSetPaneActiveWindow(pDock, docPane, doc1);
	XUI_TEST_CHECK(iRet == XUI_OK, "set active doc1");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "layout active");
	XUI_TEST_CHECK(xuiGetFocusWidget(pContext) == arrClient[0], "active client focus");
	iRet = xuiDockPanelGetWindowInfo(pDock, doc1, &tWinInfo);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiWidgetGetVisible(tWinInfo.pHostWidget), "active host visible");
	iRet = xuiDockPanelGetWindowInfo(pDock, doc2, &tWinInfo);
	XUI_TEST_CHECK(iRet == XUI_OK && !xuiWidgetGetVisible(tWinInfo.pHostWidget), "inactive host hidden");

	iRet = xuiDockPanelGetWindowInfo(pDock, doc2, &tWinInfo);
	XUI_TEST_CHECK(iRet == XUI_OK && tWinInfo.tTabRect.fW > 0.0f, "doc2 tab rect");
	XUI_TEST_CHECK(__xuiDockFindTabClose(pDock, doc2, tWinInfo.tTabRect, &tHit), "doc2 tab close hit");
	iRet = __xuiDockTestClick(pContext, 8.0f + tWinInfo.tTabRect.fX + tWinInfo.tTabRect.fW * 0.5f, 8.0f + tWinInfo.tTabRect.fY + tWinInfo.tTabRect.fH * 0.5f);
	XUI_TEST_CHECK(iRet == XUI_OK, "tab click");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiDockPanelGetPaneActiveWindow(pDock, docPane) == doc2, "tab activates");
	XUI_TEST_CHECK(xuiGetFocusWidget(pContext) == arrClient[1], "tab click focuses client");
	XUI_TEST_CHECK(tData.iActiveChanged > 0, "active callback fired");
	iRet = xuiDockPanelGetWindowInfo(pDock, doc2, &tWinInfo);
	XUI_TEST_CHECK(iRet == XUI_OK && tWinInfo.tTabRect.fW > 0.0f, "doc2 reorder source");
	tDoc2Tab = tWinInfo.tTabRect;
	iRet = xuiInputPointerMove(pContext, 8.0f + tDoc2Tab.fX + tDoc2Tab.fW * 0.5f, 8.0f + tDoc2Tab.fY + tDoc2Tab.fH * 0.5f, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "tab tooltip pointer move");
	iRet = xuiUpdate(pContext, 0.0f);
	tTipRect = xuiWidgetTooltipGetRect(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiWidgetTooltipIsOpen(pContext) &&
		xuiWidgetTooltipGetOwner(pContext) == pDock &&
		tTipRect.fW > 0.0f && tTipRect.fH > 0.0f, "tab tooltip opens");
	iRet = xuiDockPanelGetWindowInfo(pDock, doc1, &tWinInfo);
	XUI_TEST_CHECK(iRet == XUI_OK && tWinInfo.tTabRect.fW > 0.0f, "doc1 reorder target");
	tDoc1Tab = tWinInfo.tTabRect;
	iRet = __xuiDockTestDrag(pContext,
		8.0f + tDoc2Tab.fX + tDoc2Tab.fW * 0.5f,
		8.0f + tDoc2Tab.fY + tDoc2Tab.fH * 0.5f,
		8.0f + tDoc1Tab.fX + 2.0f,
		8.0f + tDoc1Tab.fY + tDoc1Tab.fH * 0.5f);
	if ( iRet == XUI_OK ) iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "tab reorder drag");
	XUI_TEST_CHECK(xuiDockPanelGetPaneWindow(pDock, docPane, 0) == doc2 &&
		xuiDockPanelGetPaneWindow(pDock, docPane, 1) == doc1 &&
		xuiDockPanelGetPaneActiveWindow(pDock, docPane) == doc2, "tab drag reorders");

	iRet = xuiDockPanelOpenPaneMenu(pDock, docPane);
	XUI_TEST_CHECK(iRet == XUI_OK, "document pane menu open");
	pMenu = xuiDockPanelGetOptionMenu(pDock);
	iMenuIndex = __xuiDockFindMenuValue(pMenu, XUI_DOCK_PANEL_MENU_AUTO_HIDE);
	XUI_TEST_CHECK(iMenuIndex >= 0 && (xuiMenuGetItemState(pMenu, iMenuIndex) & XUI_MENU_ITEM_ENABLED) == 0u, "document pane auto hide disabled");
	(void)xuiMenuClose(pMenu);
	iRet = xuiDockPanelAutoHideWindow(pDock, doc2);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT, "document pane auto hide api disabled");

	iRet = xuiDockPanelOpenPaneMenu(pDock, toolboxPane);
	XUI_TEST_CHECK(iRet == XUI_OK, "pane menu open");
	pMenu = xuiDockPanelGetOptionMenu(pDock);
	XUI_TEST_CHECK(pMenu != NULL && xuiMenuIsOpen(pMenu) && xuiMenuGetItemCount(pMenu) >= 4, "pane menu state");
	iMenuIndex = __xuiDockFindMenuValue(pMenu, XUI_DOCK_PANEL_MENU_AUTO_HIDE);
	XUI_TEST_CHECK(iMenuIndex >= 0 && (xuiMenuGetItemState(pMenu, iMenuIndex) & XUI_MENU_ITEM_ENABLED) != 0u, "side split pane auto hide enabled");
	(void)xuiMenuClose(pMenu);

	iRet = xuiWidgetSetRect(pDock, (xui_rect_t){8.0f, 8.0f, 360.0f, 404.0f});
	XUI_TEST_CHECK(iRet == XUI_OK, "dock narrow rect");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "layout narrow");
	iRet = xuiDockPanelGetPaneInfo(pDock, docPane, &tPaneInfo);
	XUI_TEST_CHECK(iRet == XUI_OK && tPaneInfo.bOverflow && tPaneInfo.tOverflowRect.fW <= 0.0f && tPaneInfo.tOverflowRect.fH <= 0.0f && tPaneInfo.iVisibleTabCount < tPaneInfo.iWindowCount, "tab overflow uses pane option menu");
	XUI_TEST_CHECK(tPaneInfo.tOptionRect.fW > 0.0f && xuiDockPanelHitTest(pDock, tPaneInfo.tOptionRect.fX + 1.0f, tPaneInfo.tOptionRect.fY + 1.0f, &tHit) == XUI_OK && tHit.iType == XUI_DOCK_PANEL_HIT_PANE_OPTION, "option hit covers overflow menu");
	iRet = xuiDockPanelOpenOverflowMenu(pDock, docPane);
	XUI_TEST_CHECK(iRet == XUI_OK, "programmatic overflow menu open");
	pMenu = xuiDockPanelGetOverflowMenu(pDock);
	XUI_TEST_CHECK(pMenu != NULL && xuiMenuIsOpen(pMenu) && xuiMenuGetItemCount(pMenu) == xuiDockPanelGetPaneWindowCount(pDock, docPane), "programmatic overflow menu state");
	(void)xuiMenuClose(pMenu);
	iRet = xuiDockPanelOpenPaneMenu(pDock, docPane);
	XUI_TEST_CHECK(iRet == XUI_OK, "option menu opens overflowed tabs");
	pMenu = xuiDockPanelGetOptionMenu(pDock);
	iRet = xuiMenuSetHoverIndex(pMenu, __xuiDockFindMenuValue(pMenu, XUI_DOCK_PANEL_MENU_WINDOW_BASE + doc5));
	XUI_TEST_CHECK(iRet == XUI_OK, "option menu hover hidden tab");
	iRet = xuiMenuCommitHover(pMenu);
	XUI_TEST_CHECK(iRet == XUI_OK || iRet == XUI_EVENT_DISPATCH_STOP, "option menu commit hidden tab");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiDockPanelGetPaneActiveWindow(pDock, docPane) == doc5, "option menu selects hidden tab");
	iRet = xuiWidgetSetRect(pDock, (xui_rect_t){8.0f, 8.0f, 624.0f, 404.0f});
	XUI_TEST_CHECK(iRet == XUI_OK, "dock restore rect");
	iRet = xuiDockPanelSetPaneActiveWindow(pDock, docPane, doc2);
	XUI_TEST_CHECK(iRet == XUI_OK, "restore active doc2");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "layout restored");

	iRet = xuiDockPanelDockWindow(pDock, scratch1, XUI_DOCK_PANEL_REGION_DOCUMENT, XUI_DOCK_PANEL_SIDE_TOP, 0.18f, &scratchPane);
	XUI_TEST_CHECK(iRet == XUI_OK && scratchPane >= 0, "scratch pane dock");
	iRet = xuiDockPanelDockWindowToPane(pDock, scratch2, scratchPane);
	XUI_TEST_CHECK(iRet == XUI_OK, "scratch2 tab");
	iRet = xuiDockPanelDockWindowToPane(pDock, scratch3, scratchPane);
	XUI_TEST_CHECK(iRet == XUI_OK, "scratch3 tab");
	iRet = xuiDockPanelSetPaneActiveWindow(pDock, scratchPane, scratch2);
	XUI_TEST_CHECK(iRet == XUI_OK, "scratch2 active");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "scratch layout");
	iRet = xuiDockPanelOpenPaneMenu(pDock, scratchPane);
	XUI_TEST_CHECK(iRet == XUI_OK, "scratch pane menu open");
	pMenu = xuiDockPanelGetOptionMenu(pDock);
	XUI_TEST_CHECK(pMenu != NULL && xuiMenuIsOpen(pMenu), "scratch menu state");
	XUI_TEST_CHECK(__xuiDockFindMenuValue(pMenu, XUI_DOCK_PANEL_MENU_CLOSE_OTHERS) >= 0, "close others menu item");
	XUI_TEST_CHECK(__xuiDockFindMenuValue(pMenu, XUI_DOCK_PANEL_MENU_CLOSE_ALL) >= 0, "close all menu item");
	iRet = xuiMenuSetHoverIndex(pMenu, __xuiDockFindMenuValue(pMenu, XUI_DOCK_PANEL_MENU_CLOSE_OTHERS));
	XUI_TEST_CHECK(iRet == XUI_OK, "close others hover");
	iRet = xuiMenuCommitHover(pMenu);
	XUI_TEST_CHECK(iRet == XUI_OK || iRet == XUI_EVENT_DISPATCH_STOP, "close others commit");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "close others layout");
	XUI_TEST_CHECK(xuiDockPanelGetPaneWindowCount(pDock, scratchPane) == 1 && xuiDockPanelGetPaneActiveWindow(pDock, scratchPane) == scratch2, "close others keeps active");
	iRet = xuiDockPanelGetWindowInfo(pDock, scratch1, &tWinInfo);
	XUI_TEST_CHECK(iRet == XUI_OK && tWinInfo.iState == XUI_DOCK_PANEL_WINDOW_HIDDEN && tWinInfo.iLastTabIndex == 0, "close others hides scratch1");
	iRet = xuiDockPanelGetWindowInfo(pDock, scratch3, &tWinInfo);
	XUI_TEST_CHECK(iRet == XUI_OK && tWinInfo.iState == XUI_DOCK_PANEL_WINDOW_HIDDEN && tWinInfo.iLastTabIndex == 2, "close others hides scratch3");

	iRet = xuiDockPanelDockWindowToPane(pDock, scratch1, scratchPane);
	XUI_TEST_CHECK(iRet == XUI_OK, "scratch1 restore tab");
	iRet = xuiDockPanelDockWindowToPane(pDock, scratch3, scratchPane);
	XUI_TEST_CHECK(iRet == XUI_OK, "scratch3 restore tab");
	iRet = xuiDockPanelSetPaneActiveWindow(pDock, scratchPane, scratch3);
	XUI_TEST_CHECK(iRet == XUI_OK, "scratch3 active");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "scratch restore layout");
	XUI_TEST_CHECK(xuiDockPanelGetPaneWindowCount(pDock, scratchPane) == 3 &&
		xuiDockPanelGetPaneWindow(pDock, scratchPane, 0) == scratch1 &&
		xuiDockPanelGetPaneWindow(pDock, scratchPane, 1) == scratch2 &&
		xuiDockPanelGetPaneWindow(pDock, scratchPane, 2) == scratch3, "scratch restore keeps tab order");
	iActiveBefore = tData.iActiveChanged;
	iRet = xuiDockPanelHideWindow(pDock, scratch3);
	XUI_TEST_CHECK(iRet == XUI_OK, "hide active scratch3");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "hide active scratch layout");
	XUI_TEST_CHECK(xuiDockPanelGetPaneWindowCount(pDock, scratchPane) == 2 &&
		xuiDockPanelGetPaneActiveWindow(pDock, scratchPane) == scratch2 &&
		tData.iActiveChanged > iActiveBefore, "hide active falls back");
	XUI_TEST_CHECK(xuiGetFocusWidget(pContext) == arrClient[9], "hide active focuses fallback");
	iRet = xuiDockPanelDockWindowToPane(pDock, scratch3, scratchPane);
	XUI_TEST_CHECK(iRet == XUI_OK, "scratch3 restore after active hide");
	iRet = xuiDockPanelSetPaneActiveWindow(pDock, scratchPane, scratch3);
	XUI_TEST_CHECK(iRet == XUI_OK, "scratch3 active after active hide");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "scratch layout after active hide");
	XUI_TEST_CHECK(xuiDockPanelGetPaneWindowCount(pDock, scratchPane) == 3 &&
		xuiDockPanelGetPaneWindow(pDock, scratchPane, 0) == scratch1 &&
		xuiDockPanelGetPaneWindow(pDock, scratchPane, 1) == scratch2 &&
		xuiDockPanelGetPaneWindow(pDock, scratchPane, 2) == scratch3 &&
		xuiDockPanelGetPaneActiveWindow(pDock, scratchPane) == scratch3, "scratch active restore keeps tab order");
	XUI_TEST_CHECK(xuiGetFocusWidget(pContext) == arrClient[10], "scratch active focus restored");
	iRet = xuiDockPanelGetWindowInfo(pDock, scratch1, &tWinInfo);
	XUI_TEST_CHECK(iRet == XUI_OK && __xuiDockFindTabClose(pDock, scratch1, tWinInfo.tTabRect, &tHit), "tab close button hit");
	iRet = __xuiDockTestClick(pContext, 8.0f + tHit.tRect.fX + tHit.tRect.fW * 0.5f, 8.0f + tHit.tRect.fY + tHit.tRect.fH * 0.5f);
	XUI_TEST_CHECK(iRet == XUI_OK, "tab close button click");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "tab close button layout");
	iRet = xuiDockPanelGetWindowInfo(pDock, scratch1, &tWinInfo);
	XUI_TEST_CHECK(iRet == XUI_OK && tWinInfo.iState == XUI_DOCK_PANEL_WINDOW_HIDDEN && xuiDockPanelGetPaneWindowCount(pDock, scratchPane) == 2, "tab close button hides tab");
	iRet = xuiDockPanelDockWindowToPane(pDock, scratch1, scratchPane);
	XUI_TEST_CHECK(iRet == XUI_OK, "scratch1 restore after tab close");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "scratch1 restore after tab close layout");
	iRet = xuiDockPanelGetWindowInfo(pDock, scratch1, &tWinInfo);
	XUI_TEST_CHECK(iRet == XUI_OK && tWinInfo.tTabRect.fW > 0.0f, "middle close source");
	iRet = __xuiDockTestMiddleClick(pContext, 8.0f + tWinInfo.tTabRect.fX + tWinInfo.tTabRect.fW * 0.5f, 8.0f + tWinInfo.tTabRect.fY + tWinInfo.tTabRect.fH * 0.5f);
	XUI_TEST_CHECK(iRet == XUI_OK, "middle close click");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "middle close layout");
	iRet = xuiDockPanelGetWindowInfo(pDock, scratch1, &tWinInfo);
	XUI_TEST_CHECK(iRet == XUI_OK && tWinInfo.iState == XUI_DOCK_PANEL_WINDOW_HIDDEN && xuiDockPanelGetPaneWindowCount(pDock, scratchPane) == 2, "middle close hides tab");

	iRet = xuiDockPanelOpenPaneMenu(pDock, scratchPane);
	XUI_TEST_CHECK(iRet == XUI_OK, "close all menu open");
	pMenu = xuiDockPanelGetOptionMenu(pDock);
	iRet = xuiMenuSetHoverIndex(pMenu, __xuiDockFindMenuValue(pMenu, XUI_DOCK_PANEL_MENU_CLOSE_ALL));
	XUI_TEST_CHECK(iRet == XUI_OK, "close all hover");
	iRet = xuiMenuCommitHover(pMenu);
	XUI_TEST_CHECK(iRet == XUI_OK || iRet == XUI_EVENT_DISPATCH_STOP, "close all commit");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "close all layout");
	iRet = xuiDockPanelGetWindowInfo(pDock, scratch2, &tWinInfo);
	XUI_TEST_CHECK(iRet == XUI_OK && tWinInfo.iState == XUI_DOCK_PANEL_WINDOW_HIDDEN, "close all hides scratch2");
	iRet = xuiDockPanelGetWindowInfo(pDock, scratch3, &tWinInfo);
	XUI_TEST_CHECK(iRet == XUI_OK && tWinInfo.iState == XUI_DOCK_PANEL_WINDOW_HIDDEN, "close all hides scratch3");

	XUI_TEST_CHECK(__xuiDockFindSplitter(pDock, &tHit), "splitter hit found");
	XUI_TEST_CHECK(xuiQueryCursor(pContext,
		(int)(8.0f + tHit.tRect.fX + tHit.tRect.fW * 0.5f),
		(int)(8.0f + tHit.tRect.fY + tHit.tRect.fH * 0.5f)) ==
		((tHit.tRect.fW <= tHit.tRect.fH) ? XUI_CURSOR_RESIZE_EW : XUI_CURSOR_RESIZE_NS),
		"splitter resize cursor");
	iRet = __xuiDockTestDrag(pContext, 8.0f + tHit.tRect.fX + 1.0f, 8.0f + tHit.tRect.fY + 1.0f, 8.0f + tHit.tRect.fX + 24.0f, 8.0f + tHit.tRect.fY + 12.0f);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiDockPanelGetLayoutChangeCount(pDock) > 0, "splitter drag");

	iRet = xuiDockPanelGetPaneInfo(pDock, toolboxPane, &tPaneInfo);
	XUI_TEST_CHECK(iRet == XUI_OK && tPaneInfo.tRect.fW > 0.0f, "auto hide remembers source pane");
	fAutoHidePaneW = tPaneInfo.tRect.fW;
	iRet = xuiDockPanelAutoHideWindow(pDock, toolbox);
	XUI_TEST_CHECK(iRet == XUI_OK, "auto hide");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "layout auto hide");
	gDockTrackedFont = pFont2;
	gDockTrackedFontDrawCount = 0;
	iRet = xuiDockPanelSetFont(pDock, pFont2);
	if ( iRet == XUI_OK ) iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiDockPanelGetFont(pDock) == pFont2 &&
		gDockTrackedFontDrawCount > 0,
		"font change redraws cached vertical auto hide labels");
	gDockTrackedFont = NULL;
	iRet = xuiDockPanelSetFont(pDock, pFont);
	if ( iRet == XUI_OK ) iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "restore dock font after auto hide cache test");
	iRet = xuiDockPanelGetWindowInfo(pDock, toolbox, &tWinInfo);
	XUI_TEST_CHECK(iRet == XUI_OK && tWinInfo.iState == XUI_DOCK_PANEL_WINDOW_AUTO_HIDE, "auto hide state");
	XUI_TEST_CHECK(tWinInfo.iLastRegion == XUI_DOCK_PANEL_REGION_DOCUMENT && tWinInfo.iLastSide == XUI_DOCK_PANEL_SIDE_LEFT, "auto hide keeps dock target");
	XUI_TEST_CHECK(__xuiDockNear(tWinInfo.tAutoHideRect.fW, tMetrics.fAutoHideStripSize) &&
		tWinInfo.tAutoHideRect.fH > tMetrics.fAutoHideStripSize, "left auto hide uses vertical title tab");
	iRet = xuiDockPanelGetPaneInfo(pDock, docPane, &tPaneInfo);
	XUI_TEST_CHECK(iRet == XUI_OK &&
		tWinInfo.tAutoHideRect.fX + tWinInfo.tAutoHideRect.fW <= tPaneInfo.tRect.fX + 1.01f,
		"left auto hide rail does not cover document pane");
	iRet = __xuiDockTestClick(pContext, 8.0f + tWinInfo.tAutoHideRect.fX + tWinInfo.tAutoHideRect.fW * 0.5f, 8.0f + tWinInfo.tAutoHideRect.fY + tWinInfo.tAutoHideRect.fH * 0.5f);
	XUI_TEST_CHECK(iRet == XUI_OK, "auto hide strip click");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "layout auto hide expand");
	iRet = xuiDockPanelGetWindowInfo(pDock, toolbox, &tWinInfo);
	XUI_TEST_CHECK(iRet == XUI_OK && tWinInfo.iState == XUI_DOCK_PANEL_WINDOW_AUTO_HIDE && xuiDockPanelGetAutoHideExpandedWindow(pDock) == toolbox, "auto hide expanded state");
	XUI_TEST_CHECK(xuiWidgetGetVisible(tWinInfo.pHostWidget) && tWinInfo.tRect.fW > 0.0f && tWinInfo.tClientRect.fW > 0.0f, "auto hide overlay visible");
	iRet = xuiWidgetGetLayer(tWinInfo.pHostWidget, &iLayerToolbox, &iZToolbox);
	if ( iRet == XUI_OK ) iRet = xuiRenderPrepare(pContext);
	pCache = (iRet == XUI_OK) ? xuiWidgetGetCacheSurface(tWinInfo.pHostWidget, xuiWidgetGetStateId(tWinInfo.pHostWidget)) : NULL;
	XUI_TEST_CHECK(iRet == XUI_OK && iLayerToolbox == XUI_LAYER_FLOATING && pCache != NULL &&
		xuiTestSurfaceGetRectFillCount(pCache) >= 3,
		"auto hide host paints opaque chrome in floating layer");
	tHit.tRect = xuiDockPanelGetAutoHideExpandRect(pDock);
	XUI_TEST_CHECK(tHit.tRect.fW > 0.0f && tHit.tRect.fH > 0.0f, "auto hide expand rect");
	if ( fAutoHidePaneW < 160.0f ) {
		XUI_TEST_CHECK(tHit.tRect.fW >= 159.0f && tHit.tRect.fW <= 190.0f, "auto hide clamps remembered small width");
	} else if ( fAutoHidePaneW < 420.0f ) {
		XUI_TEST_CHECK(tHit.tRect.fW > fAutoHidePaneW - 6.0f && tHit.tRect.fW < fAutoHidePaneW + 6.0f, "auto hide expand remembers pane width");
	} else {
		XUI_TEST_CHECK(tHit.tRect.fW <= 420.0f, "auto hide expand clamps large width");
	}
	XUI_TEST_CHECK(__xuiDockTestAutoHideResize(pContext, pDock, XUI_DOCK_PANEL_REGION_LEFT),
		"left auto hide resize is deferred and anchored");
	iRet = xuiDockPanelCollapseAutoHide(pDock);
	XUI_TEST_CHECK(iRet == XUI_OK, "collapse auto hide");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiDockPanelGetAutoHideExpandedWindow(pDock) < 0, "auto hide collapsed");
	iRet = xuiDockPanelExpandAutoHideWindow(pDock, toolbox);
	XUI_TEST_CHECK(iRet == XUI_OK, "expand auto hide api");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiDockPanelGetAutoHideExpandedWindow(pDock) == toolbox, "auto hide api expanded");
	iRet = xuiDockPanelDockAutoHideWindow(pDock, toolbox);
	XUI_TEST_CHECK(iRet == XUI_OK, "dock auto hide");
	iRet = xuiDockPanelGetWindowInfo(pDock, toolbox, &tWinInfo);
	XUI_TEST_CHECK(iRet == XUI_OK && tWinInfo.iState == XUI_DOCK_PANEL_WINDOW_DOCKED && tWinInfo.iRegion == XUI_DOCK_PANEL_REGION_DOCUMENT && tWinInfo.iLastSide == XUI_DOCK_PANEL_SIDE_LEFT && xuiDockPanelGetAutoHideExpandedWindow(pDock) < 0, "auto dock state");

	iRet = xuiDockPanelAutoHideWindow(pDock, props);
	XUI_TEST_CHECK(iRet == XUI_OK, "right auto hide");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "layout right auto hide");
	iRet = xuiDockPanelGetWindowInfo(pDock, props, &tWinInfo);
	XUI_TEST_CHECK(iRet == XUI_OK && tWinInfo.iState == XUI_DOCK_PANEL_WINDOW_AUTO_HIDE &&
		__xuiDockNear(tWinInfo.tAutoHideRect.fW, tMetrics.fAutoHideStripSize) &&
		tWinInfo.tAutoHideRect.fH > tMetrics.fAutoHideStripSize,
		"right auto hide uses vertical title tab");
	iRet = xuiDockPanelGetPaneInfo(pDock, docPane, &tPaneInfo);
	XUI_TEST_CHECK(iRet == XUI_OK &&
		tPaneInfo.tRect.fX + tPaneInfo.tRect.fW <= tWinInfo.tAutoHideRect.fX + 1.01f,
		"right auto hide rail does not cover document pane");
	iRet = xuiDockPanelExpandAutoHideWindow(pDock, props);
	XUI_TEST_CHECK(iRet == XUI_OK, "expand right auto hide");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "layout right auto hide expand");
	tHit.tRect = xuiDockPanelGetAutoHideExpandRect(pDock);
	XUI_TEST_CHECK(tHit.tRect.fX + tHit.tRect.fW <= tWinInfo.tAutoHideRect.fX + 1.01f,
		"right auto hide expansion stays inside rail");
	XUI_TEST_CHECK(__xuiDockTestAutoHideResize(pContext, pDock, XUI_DOCK_PANEL_REGION_RIGHT),
		"right auto hide resize is deferred and anchored");
	tHit.tRect = xuiDockPanelGetAutoHideExpandRect(pDock);
	tClientWorld = xuiWidgetGetWorldRect(pNestedClient);
	iClientBefore = tData.iClientPointerDown;
	iRet = __xuiDockTestClick(pContext, tClientWorld.fX + tClientWorld.fW * 0.5f, tClientWorld.fY + tClientWorld.fH * 0.5f);
	XUI_TEST_CHECK(iRet == XUI_OK && tData.iClientPointerDown == iClientBefore + 1, "auto hide client remains interactive");
	iRet = xuiDockPanelGetWindowInfo(pDock, props, &tWinInfo);
	if ( iRet == XUI_OK ) tTipRect = xuiWidgetGetWorldRect(tWinInfo.pCloseButtonWidget);
	if ( iRet == XUI_OK ) iRet = __xuiDockTestClick(pContext,
		tTipRect.fX + tTipRect.fW * 0.5f, tTipRect.fY + tTipRect.fH * 0.5f);
	XUI_TEST_CHECK(iRet == XUI_OK, "auto hide close button click");
	iRet = xuiLayout(pContext);
	if ( iRet == XUI_OK ) iRet = xuiDockPanelGetWindowInfo(pDock, props, &tWinInfo);
	XUI_TEST_CHECK(iRet == XUI_OK && tWinInfo.iState == XUI_DOCK_PANEL_WINDOW_HIDDEN, "auto hide close button hides window");
	iRet = xuiDockPanelDockWindow(pDock, props, XUI_DOCK_PANEL_REGION_DOCUMENT, XUI_DOCK_PANEL_SIDE_RIGHT, 0.24f, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK, "restore right window after auto hide close");
	if ( iRet == XUI_OK ) iRet = xuiDockPanelAutoHideWindow(pDock, props);
	if ( iRet == XUI_OK ) iRet = xuiDockPanelExpandAutoHideWindow(pDock, props);
	if ( iRet == XUI_OK ) iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "restore expanded auto hide for dock button");
	tHit.tRect = xuiDockPanelGetAutoHideExpandRect(pDock);
	iRet = xuiDockPanelGetWindowInfo(pDock, props, &tWinInfo);
	if ( iRet == XUI_OK ) tTipRect = xuiWidgetGetWorldRect(tWinInfo.pPinButtonWidget);
	if ( iRet == XUI_OK ) iRet = __xuiDockTestClick(pContext,
		tTipRect.fX + tTipRect.fW * 0.5f, tTipRect.fY + tTipRect.fH * 0.5f);
	XUI_TEST_CHECK(iRet == XUI_OK, "auto hide dock button click");
	iRet = xuiLayout(pContext);
	if ( iRet == XUI_OK ) iRet = xuiDockPanelGetWindowInfo(pDock, props, &tWinInfo);
	XUI_TEST_CHECK(iRet == XUI_OK && tWinInfo.iState == XUI_DOCK_PANEL_WINDOW_DOCKED, "auto hide dock button restores docked state");
	iRet = xuiDockPanelAutoHideWindow(pDock, props);
	if ( iRet == XUI_OK ) iRet = xuiDockPanelExpandAutoHideWindow(pDock, props);
	if ( iRet == XUI_OK ) iRet = xuiLayout(pContext);
	tHit.tRect = xuiDockPanelGetAutoHideExpandRect(pDock);
	XUI_TEST_CHECK(xuiQueryCursor(pContext,
		(int)(8.0f + tHit.tRect.fX + 32.0f),
		(int)(8.0f + tHit.tRect.fY + tMetrics.fCaptionHeight * 0.5f)) == XUI_CURSOR_MOVE,
		"expanded auto hide caption move cursor");
	if ( iRet == XUI_OK ) iRet = xuiDockPanelGetPaneInfo(pDock, docPane, &tPaneInfo);
	if ( iRet == XUI_OK ) iRet = __xuiDockTestDrag(pContext,
		8.0f + tHit.tRect.fX + 32.0f,
		8.0f + tHit.tRect.fY + tMetrics.fCaptionHeight * 0.5f,
		8.0f + tPaneInfo.tRect.fX + tPaneInfo.tRect.fW * 0.5f,
		8.0f + tPaneInfo.tRect.fY + tPaneInfo.tRect.fH * 0.5f);
	if ( iRet == XUI_OK ) iRet = xuiLayout(pContext);
	if ( iRet == XUI_OK ) iRet = xuiDockPanelGetWindowInfo(pDock, props, &tWinInfo);
	XUI_TEST_CHECK(iRet == XUI_OK && tWinInfo.iState == XUI_DOCK_PANEL_WINDOW_DOCKED, "expanded auto hide caption drag docks window");
	iRet = xuiDockPanelDockWindow(pDock, props, XUI_DOCK_PANEL_REGION_DOCUMENT, XUI_DOCK_PANEL_SIDE_RIGHT, 0.24f, NULL);
	if ( iRet == XUI_OK ) iRet = xuiDockPanelAutoHideWindow(pDock, props);
	if ( iRet == XUI_OK ) iRet = xuiLayout(pContext);
	if ( iRet == XUI_OK ) iRet = xuiDockPanelGetWindowInfo(pDock, props, &tWinInfo);
	XUI_TEST_CHECK(iRet == XUI_OK && tWinInfo.iState == XUI_DOCK_PANEL_WINDOW_AUTO_HIDE, "auto hide title drag source");
	XUI_TEST_CHECK(xuiQueryCursor(pContext,
		(int)(8.0f + tWinInfo.tAutoHideRect.fX + tWinInfo.tAutoHideRect.fW * 0.5f),
		(int)(8.0f + tWinInfo.tAutoHideRect.fY + tWinInfo.tAutoHideRect.fH * 0.5f)) == XUI_CURSOR_MOVE,
		"auto hide side title move cursor");
	iRet = xuiDockPanelGetPaneInfo(pDock, docPane, &tPaneInfo);
	if ( iRet == XUI_OK ) iRet = __xuiDockTestDrag(pContext,
		8.0f + tWinInfo.tAutoHideRect.fX + tWinInfo.tAutoHideRect.fW * 0.5f,
		8.0f + tWinInfo.tAutoHideRect.fY + tWinInfo.tAutoHideRect.fH * 0.5f,
		8.0f + tPaneInfo.tRect.fX + tPaneInfo.tRect.fW * 0.5f,
		8.0f + tPaneInfo.tRect.fY + tPaneInfo.tRect.fH * 0.5f);
	if ( iRet == XUI_OK ) iRet = xuiLayout(pContext);
	if ( iRet == XUI_OK ) iRet = xuiDockPanelGetWindowInfo(pDock, props, &tWinInfo);
	XUI_TEST_CHECK(iRet == XUI_OK && tWinInfo.iState == XUI_DOCK_PANEL_WINDOW_DOCKED, "auto hide side title drag docks window");

	iRet = xuiDockPanelDockWindow(pDock, props, XUI_DOCK_PANEL_REGION_DOCUMENT, XUI_DOCK_PANEL_SIDE_TOP, 0.24f, NULL);
	if ( iRet == XUI_OK ) iRet = xuiDockPanelAutoHideWindow(pDock, props);
	if ( iRet == XUI_OK ) iRet = xuiDockPanelExpandAutoHideWindow(pDock, props);
	if ( iRet == XUI_OK ) iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && __xuiDockTestAutoHideResize(pContext, pDock, XUI_DOCK_PANEL_REGION_TOP),
		"top auto hide resize is deferred and anchored");
	iRet = xuiDockPanelDockAutoHideWindow(pDock, props);
	if ( iRet == XUI_OK ) iRet = xuiDockPanelDockWindow(pDock, props, XUI_DOCK_PANEL_REGION_DOCUMENT, XUI_DOCK_PANEL_SIDE_BOTTOM, 0.24f, NULL);
	if ( iRet == XUI_OK ) iRet = xuiDockPanelAutoHideWindow(pDock, props);
	if ( iRet == XUI_OK ) iRet = xuiDockPanelExpandAutoHideWindow(pDock, props);
	if ( iRet == XUI_OK ) iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && __xuiDockTestAutoHideResize(pContext, pDock, XUI_DOCK_PANEL_REGION_BOTTOM),
		"bottom auto hide resize is deferred and anchored");
	iRet = xuiDockPanelDockAutoHideWindow(pDock, props);
	XUI_TEST_CHECK(iRet == XUI_OK, "restore bottom auto hide after resize");

	iRet = xuiDockPanelFloatWindow(pDock, props, (xui_rect_t){330.0f, 44.0f, 210.0f, 150.0f});
	XUI_TEST_CHECK(iRet == XUI_OK, "float props");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "layout float");
	iRet = xuiDockPanelGetWindowInfo(pDock, props, &tWinInfo);
	XUI_TEST_CHECK(iRet == XUI_OK && tWinInfo.iState == XUI_DOCK_PANEL_WINDOW_FLOATING && xuiWidgetGetVisible(tWinInfo.pHostWidget), "float visible");
	XUI_TEST_CHECK(xuiQueryCursor(pContext,
		(int)(8.0f + tWinInfo.tRect.fX + 2.0f),
		(int)(8.0f + tWinInfo.tRect.fY + tWinInfo.tRect.fH * 0.5f)) == XUI_CURSOR_RESIZE_EW,
		"floating left edge resize cursor");
	XUI_TEST_CHECK(xuiQueryCursor(pContext,
		(int)(8.0f + tWinInfo.tRect.fX + tWinInfo.tRect.fW - 2.0f),
		(int)(8.0f + tWinInfo.tRect.fY + 2.0f)) == XUI_CURSOR_RESIZE_NESW,
		"floating top-right resize cursor");
	XUI_TEST_CHECK(xuiWidgetGetVisible(arrClient[6]) && xuiWidgetGetVisible(pNestedClient), "floating client tree visible");
	iRet = xuiWidgetGetLayer(tWinInfo.pHostWidget, &iLayerProps, &iZProps);
	if ( iRet == XUI_OK ) iRet = xuiWidgetGetLayer(arrClient[6], &iLayerClient, &iZClient);
	if ( iRet == XUI_OK ) iRet = xuiWidgetGetLayer(pNestedClient, &iLayerNested, &iZNested);
	XUI_TEST_CHECK(iRet == XUI_OK &&
		iLayerProps == XUI_LAYER_FLOATING &&
		iLayerClient == iLayerProps && iZClient == iZProps &&
		iLayerNested == iLayerProps && iZNested == iZProps,
		"floating client subtree follows host layer");
	iRet = xuiSetFocusWidget(pContext, pNestedClient);
	if ( iRet == XUI_OK ) iRet = xuiWidgetInvalidate(pDock, XUI_WIDGET_DIRTY_LAYOUT);
	if ( iRet == XUI_OK ) iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiGetFocusWidget(pContext) == pNestedClient, "stable layout keeps floating client focus");
	iRet = xuiWidgetInvalidate(pDock, XUI_WIDGET_DIRTY_LAYOUT);
	if ( iRet == XUI_OK ) iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiGetFocusWidget(pContext) == pNestedClient, "repeated layout does not reparent floating host");
	iRet = xuiDockPanelSetPaneActiveWindow(pDock, docPane, doc2);
	if ( iRet == XUI_OK ) iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiDockPanelGetPaneActiveWindow(pDock, docPane) == doc2, "covered tab baseline active");
	iRet = xuiDockPanelGetWindowInfo(pDock, doc1, &tWinInfo);
	XUI_TEST_CHECK(iRet == XUI_OK && tWinInfo.tTabRect.fW > 0.0f, "covered tab target");
	tDoc1Tab = tWinInfo.tTabRect;
	iRet = xuiDockPanelFloatWindow(pDock, props, (xui_rect_t){tDoc1Tab.fX + 4.0f, tDoc1Tab.fY - 1.0f, 210.0f, 150.0f});
	if ( iRet == XUI_OK ) iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "float over covered tab");
	iRet = xuiDockPanelGetWindowInfo(pDock, props, &tWinInfo);
	XUI_TEST_CHECK(iRet == XUI_OK && tWinInfo.iState == XUI_DOCK_PANEL_WINDOW_FLOATING, "covered tab floating state");
	pHitWidget = xuiHitTest(pContext, 8.0f + tWinInfo.tRect.fX + 28.0f, 8.0f + tWinInfo.tRect.fY + 10.0f, XUI_WIDGET_HIT_DEFAULT);
	XUI_TEST_CHECK(pHitWidget == tWinInfo.pHostWidget, "floating title hit wins over covered tab");
	iRet = __xuiDockTestClick(pContext, 8.0f + tWinInfo.tRect.fX + 28.0f, 8.0f + tWinInfo.tRect.fY + 10.0f);
	if ( iRet == XUI_OK ) iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "floating title click over covered tab");
	XUI_TEST_CHECK(xuiDockPanelGetPaneActiveWindow(pDock, docPane) == doc2, "floating title click does not activate covered tab");
	iRet = xuiDockPanelGetWindowInfo(pDock, props, &tWinInfo);
	XUI_TEST_CHECK(iRet == XUI_OK && tWinInfo.iState == XUI_DOCK_PANEL_WINDOW_FLOATING, "covered tab click keeps floating");
	iRet = xuiDockPanelGetPaneInfo(pDock, docPane, &tPaneInfo);
	XUI_TEST_CHECK(iRet == XUI_OK && tPaneInfo.tRect.fW > 0.0f, "drop target pane info");
	iRet = xuiDockPanelFindDropTarget(pDock, props, tPaneInfo.tRect.fX + tPaneInfo.tRect.fW * 0.5f, tPaneInfo.tRect.fY + tPaneInfo.tRect.fH * 0.5f, &tDrop);
	XUI_TEST_CHECK(iRet == XUI_OK && tDrop.bValid && tDrop.iPane == docPane && tDrop.iSide == XUI_DOCK_PANEL_SIDE_FILL, "drop target fill");
	XUI_TEST_CHECK(__xuiDockFindPaneFreeDropPoint(pDock, props, tPaneInfo.tRect, &fNoDropX, &fNoDropY), "pane free drop point");
	iRet = xuiDockPanelFindDropTarget(pDock, props, fNoDropX, fNoDropY, &tDrop);
	XUI_TEST_CHECK(iRet == XUI_OK && !tDrop.bValid, "pane interior is not automatic drop target");
	iRet = __xuiDockTestDragEx(pContext,
		8.0f + tWinInfo.tRect.fX + 20.0f,
		8.0f + tWinInfo.tRect.fY + 10.0f,
		8.0f + tPaneInfo.tRect.fX + tPaneInfo.tRect.fW - 8.0f,
		8.0f + tPaneInfo.tRect.fY + tPaneInfo.tRect.fH * 0.5f,
		XUI_MOD_CTRL);
	if ( iRet == XUI_OK ) iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "ctrl float drag");
	iRet = xuiDockPanelGetDragPreview(pDock, &tDrop);
	XUI_TEST_CHECK(iRet == XUI_OK && !tDrop.bValid, "ctrl suppresses drop preview");
	iRet = xuiDockPanelGetWindowInfo(pDock, props, &tWinInfo);
	XUI_TEST_CHECK(iRet == XUI_OK && tWinInfo.iState == XUI_DOCK_PANEL_WINDOW_FLOATING, "ctrl drag keeps floating");
	iRet = xuiInputPointerDown(pContext, 8.0f + tWinInfo.tRect.fX + 20.0f, 8.0f + tWinInfo.tRect.fY + 10.0f, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	if ( iRet == XUI_OK ) iRet = xuiInputPointerMove(pContext, 8.0f + tPaneInfo.tRect.fX + tPaneInfo.tRect.fW - 8.0f, 8.0f + tPaneInfo.tRect.fY + tPaneInfo.tRect.fH * 0.5f, XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "float drag preview move");
	iRet = xuiDockPanelGetDragPreview(pDock, &tDrop);
	XUI_TEST_CHECK(iRet == XUI_OK && tDrop.bValid && tDrop.iPane == docPane && tDrop.iSide == XUI_DOCK_PANEL_SIDE_RIGHT && tDrop.tRect.fW > 0.0f, "drag preview side");
	iRet = xuiInputPointerUp(pContext, 8.0f + tPaneInfo.tRect.fX + tPaneInfo.tRect.fW - 8.0f, 8.0f + tPaneInfo.tRect.fY + tPaneInfo.tRect.fH * 0.5f, XUI_POINTER_BUTTON_LEFT, 0);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	if ( iRet == XUI_OK ) iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "float drag drop commit");
	iRet = xuiDockPanelGetWindowInfo(pDock, props, &tWinInfo);
	dragPane = tWinInfo.iPane;
	XUI_TEST_CHECK(iRet == XUI_OK && tWinInfo.iState == XUI_DOCK_PANEL_WINDOW_DOCKED && dragPane >= 0 && dragPane != docPane, "drag drop docked split");
	XUI_TEST_CHECK(xuiDockPanelGetPaneWindowCount(pDock, dragPane) == 1, "drag drop new pane");
	iRet = xuiDockPanelGetDragPreview(pDock, &tDrop);
	XUI_TEST_CHECK(iRet == XUI_OK && !tDrop.bValid, "drag preview cleared");

	iRet = xuiDockPanelGetWindowInfo(pDock, props, &tWinInfo);
	XUI_TEST_CHECK(iRet == XUI_OK && tWinInfo.tTabRect.fW <= 0.0f, "single docked pane has no tab source");
	iRet = xuiDockPanelGetPaneInfo(pDock, dragPane, &tPaneInfo);
	XUI_TEST_CHECK(iRet == XUI_OK && tPaneInfo.tCaptionRect.fW > 0.0f && tPaneInfo.tTabStripRect.fW <= 0.0f, "single docked caption drag source");
	tSrcRect = tPaneInfo.tCaptionRect;
	iRet = xuiDockPanelGetPaneInfo(pDock, docPane, &tPaneInfo);
	XUI_TEST_CHECK(iRet == XUI_OK && tPaneInfo.tRect.fW > 0.0f, "docked tab drag target");
	iRet = __xuiDockTestDrag(pContext,
		8.0f + tSrcRect.fX + tSrcRect.fW * 0.5f,
		8.0f + tSrcRect.fY + tSrcRect.fH * 0.5f,
		8.0f + tPaneInfo.tRect.fX + tPaneInfo.tRect.fW * 0.5f,
		8.0f + tPaneInfo.tRect.fY + tPaneInfo.tRect.fH * 0.5f);
	if ( iRet == XUI_OK ) iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "docked tab drag dock commit");
	iRet = xuiDockPanelGetWindowInfo(pDock, props, &tWinInfo);
	XUI_TEST_CHECK(iRet == XUI_OK && tWinInfo.iState == XUI_DOCK_PANEL_WINDOW_DOCKED && tWinInfo.iPane == docPane, "docked tab drag docked fill");
	iRet = xuiDockPanelGetDragPreview(pDock, &tDrop);
	XUI_TEST_CHECK(iRet == XUI_OK && !tDrop.bValid, "docked tab drag preview cleared");

	XUI_TEST_CHECK(__xuiDockFindPaneFreeDropPoint(pDock, props, tPaneInfo.tRect, &fNoDropX, &fNoDropY), "docked tab free point");
	XUI_TEST_CHECK(tWinInfo.tTabRect.fW > 0.0f, "docked tab free-float source");
	iRet = __xuiDockTestDrag(pContext,
		8.0f + tWinInfo.tTabRect.fX + tWinInfo.tTabRect.fW * 0.5f,
		8.0f + tWinInfo.tTabRect.fY + tWinInfo.tTabRect.fH * 0.5f,
		8.0f + fNoDropX,
		8.0f + fNoDropY);
	if ( iRet == XUI_OK ) iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "docked tab drag to pane interior");
	iRet = xuiDockPanelGetWindowInfo(pDock, props, &tWinInfo);
	XUI_TEST_CHECK(iRet == XUI_OK && tWinInfo.iState == XUI_DOCK_PANEL_WINDOW_FLOATING && xuiWidgetGetVisible(tWinInfo.pHostWidget), "pane interior drag leaves floating");
	iRet = xuiDockPanelGetDragPreview(pDock, &tDrop);
	XUI_TEST_CHECK(iRet == XUI_OK && !tDrop.bValid, "pane interior float preview cleared");
	fOldX = tWinInfo.tRect.fX;
	fOldW = tWinInfo.tRect.fW;
	iRet = __xuiDockTestDrag(pContext,
		8.0f + tWinInfo.tRect.fX + 24.0f,
		8.0f + tWinInfo.tRect.fY + tMetrics.fFloatTitleHeight * 0.5f,
		8.0f + tWinInfo.tRect.fX + 72.0f,
		8.0f + tWinInfo.tRect.fY + tMetrics.fFloatTitleHeight * 0.5f + 20.0f);
	if ( iRet == XUI_OK ) iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "drag-out floating title drag");
	iRet = xuiDockPanelGetWindowInfo(pDock, props, &tWinInfo);
	XUI_TEST_CHECK(iRet == XUI_OK && tWinInfo.iState == XUI_DOCK_PANEL_WINDOW_FLOATING &&
		tWinInfo.tRect.fX > fOldX + 20.0f && tWinInfo.tRect.fW > fOldW - 1.0f && tWinInfo.tRect.fW < fOldW + 1.0f,
		"drag-out floating title moves window");
	iRet = xuiDockPanelDockWindowToPane(pDock, props, docPane);
	XUI_TEST_CHECK(iRet == XUI_OK, "redock props after free-float");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "layout redock props");
	iRet = xuiDockPanelGetWindowInfo(pDock, props, &tWinInfo);
	XUI_TEST_CHECK(iRet == XUI_OK && tWinInfo.iState == XUI_DOCK_PANEL_WINDOW_DOCKED && tWinInfo.iPane == docPane && tWinInfo.tTabRect.fW > 0.0f, "redock props state");

	XUI_TEST_CHECK(tWinInfo.tTabRect.fW > 0.0f, "docked tab float source");
	iRet = __xuiDockTestDrag(pContext,
		8.0f + tWinInfo.tTabRect.fX + tWinInfo.tTabRect.fW * 0.5f,
		8.0f + tWinInfo.tTabRect.fY + tWinInfo.tTabRect.fH * 0.5f,
		636.0f,
		36.0f);
	if ( iRet == XUI_OK ) iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "docked tab drag float commit");
	iRet = xuiDockPanelGetWindowInfo(pDock, props, &tWinInfo);
	XUI_TEST_CHECK(iRet == XUI_OK && tWinInfo.iState == XUI_DOCK_PANEL_WINDOW_FLOATING && xuiWidgetGetVisible(tWinInfo.pHostWidget), "docked tab drag floated");
	iRet = xuiDockPanelGetDragPreview(pDock, &tDrop);
	XUI_TEST_CHECK(iRet == XUI_OK && !tDrop.bValid, "docked tab float preview cleared");

	iRet = xuiDockPanelHideWindow(pDock, output);
	XUI_TEST_CHECK(iRet == XUI_OK, "hide output");
	iRet = xuiDockPanelGetWindowInfo(pDock, output, &tWinInfo);
	XUI_TEST_CHECK(iRet == XUI_OK && tWinInfo.iState == XUI_DOCK_PANEL_WINDOW_HIDDEN, "hidden state");

	iRet = xuiDockPanelSaveState(pDock, &pState);
	XUI_TEST_CHECK(iRet == XUI_OK && pState != NULL, "save state");
	iRet = xuiDockPanelStateGetCounts(pState, &iRegionCount, &iWindowCount, &iFloatingCount);
	XUI_TEST_CHECK(iRet == XUI_OK && iRegionCount == XUI_DOCK_PANEL_REGION_COUNT && iWindowCount == 11 && iFloatingCount == 1, "state counts");
	iRet = xuiDockPanelGetRegionSize(pDock, XUI_DOCK_PANEL_REGION_LEFT, &iRegionMode, &fRegionBefore);
	pRegions = xrtValueObjectGet(pState, (xstrview){ "regions", 7 });
	pRegionState = xrtValueArrayGet(pRegions, 0);
	XUI_TEST_CHECK(iRet == XUI_OK && pRegionState != NULL &&
		xrtValueObjectSetNew(pRegionState, (xstrview){ "value", 5 }, xrtValueFloat(NAN)),
		"corrupt saved region size");
	iRet = xuiDockPanelLoadState(pDock, pState);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT, "reject nonfinite saved region size");
	iRet = xuiDockPanelGetRegionSize(pDock, XUI_DOCK_PANEL_REGION_LEFT, &iRegionCount, &fRegionAfter);
	XUI_TEST_CHECK(iRet == XUI_OK && iRegionCount == iRegionMode && fRegionAfter == fRegionBefore,
		"invalid saved state preserves layout");
	xuiDockPanelStateFree(pState);
	pState = NULL;
	pRegions = NULL;
	pRegionState = NULL;
	iRet = xuiDockPanelSaveState(pDock, &pState);
	XUI_TEST_CHECK(iRet == XUI_OK && pState != NULL, "resave valid state");
	iRet = xuiDockPanelDockWindow(pDock, props, XUI_DOCK_PANEL_REGION_DOCUMENT, XUI_DOCK_PANEL_SIDE_FILL, 0.0f, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK, "mutate props docked");
	iRet = xuiDockPanelDockWindow(pDock, output, XUI_DOCK_PANEL_REGION_DOCUMENT, XUI_DOCK_PANEL_SIDE_BOTTOM, 0.22f, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK, "mutate output docked");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "layout mutated state");
	iRet = xuiDockPanelLoadState(pDock, pState);
	XUI_TEST_CHECK(iRet == XUI_OK, "load state");
	xuiDockPanelStateFree(pState);
	pState = NULL;
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "layout loaded state");
	iRet = xuiDockPanelGetWindowInfo(pDock, props, &tWinInfo);
	XUI_TEST_CHECK(iRet == XUI_OK && tWinInfo.iState == XUI_DOCK_PANEL_WINDOW_FLOATING, "load restores floating");
	iRet = xuiDockPanelGetWindowInfo(pDock, output, &tWinInfo);
	XUI_TEST_CHECK(iRet == XUI_OK && tWinInfo.iState == XUI_DOCK_PANEL_WINDOW_HIDDEN, "load restores hidden");
	iRet = xuiDockPanelGetWindowInfo(pDock, doc2, &tWinInfo);
	XUI_TEST_CHECK(iRet == XUI_OK && tWinInfo.iState == XUI_DOCK_PANEL_WINDOW_DOCKED && tWinInfo.iPane >= 0, "load restores document pane");
	docPane = tWinInfo.iPane;

	iRet = xuiDockPanelSaveXSONFile(pDock, "build\\dockpanel_roundtrip.xson");
	XUI_TEST_CHECK(iRet == XUI_OK, "save xson");
	iRet = xuiDockPanelDockWindow(pDock, props, XUI_DOCK_PANEL_REGION_DOCUMENT, XUI_DOCK_PANEL_SIDE_FILL, 0.0f, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK, "xson mutate props docked");
	iRet = xuiDockPanelLoadXSONFile(pDock, "build\\dockpanel_roundtrip.xson");
	XUI_TEST_CHECK(iRet == XUI_OK, "load xson");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "layout xson loaded");
	iRet = xuiDockPanelGetWindowInfo(pDock, props, &tWinInfo);
	XUI_TEST_CHECK(iRet == XUI_OK && tWinInfo.iState == XUI_DOCK_PANEL_WINDOW_FLOATING, "xson restores floating");
	fOldX = tWinInfo.tRect.fX;
	fOldW = tWinInfo.tRect.fW;
	iRet = __xuiDockTestDragMove(pContext,
		8.0f + tWinInfo.tRect.fX + 2.0f,
		8.0f + tWinInfo.tRect.fY + tWinInfo.tRect.fH * 0.5f,
		8.0f + tWinInfo.tRect.fX + 38.0f,
		8.0f + tWinInfo.tRect.fY + tWinInfo.tRect.fH * 0.5f);
	if ( iRet == XUI_OK ) iRet = xuiDockPanelGetWindowInfo(pDock, props, &tWinInfo);
	XUI_TEST_CHECK(iRet == XUI_OK && tWinInfo.tRect.fX == fOldX && tWinInfo.tRect.fW == fOldW, "floating resize move remains deferred");
	if ( iRet == XUI_OK ) iRet = __xuiDockTestDragUp(pContext,
		8.0f + fOldX + 38.0f,
		8.0f + tWinInfo.tRect.fY + tWinInfo.tRect.fH * 0.5f);
	if ( iRet == XUI_OK ) iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "floating resize drag");
	iRet = xuiDockPanelGetWindowInfo(pDock, props, &tWinInfo);
	XUI_TEST_CHECK(iRet == XUI_OK && tWinInfo.iState == XUI_DOCK_PANEL_WINDOW_FLOATING && tWinInfo.tRect.fX > fOldX + 10.0f && tWinInfo.tRect.fW < fOldW - 10.0f, "floating resize updates rect");
	fOldY = tWinInfo.tRect.fY;
	fOldH = tWinInfo.tRect.fH;
	iRet = __xuiDockTestDrag(pContext,
		8.0f + tWinInfo.tRect.fX + tWinInfo.tRect.fW * 0.5f,
		8.0f + tWinInfo.tRect.fY + 2.0f,
		8.0f + tWinInfo.tRect.fX + tWinInfo.tRect.fW * 0.5f,
		8.0f + tWinInfo.tRect.fY + 24.0f);
	if ( iRet == XUI_OK ) iRet = xuiLayout(pContext);
	if ( iRet == XUI_OK ) iRet = xuiDockPanelGetWindowInfo(pDock, props, &tWinInfo);
	XUI_TEST_CHECK(iRet == XUI_OK && tWinInfo.tRect.fY > fOldY + 10.0f && tWinInfo.tRect.fH < fOldH - 10.0f,
		"floating top edge resize takes priority over title move");
	iRet = xuiDockPanelFloatWindow(pDock, toolbox, (xui_rect_t){42.0f, 58.0f, 190.0f, 135.0f});
	XUI_TEST_CHECK(iRet == XUI_OK, "float toolbox for z order");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "layout two floating windows");
	iRet = xuiDockPanelGetWindowInfo(pDock, props, &tWinInfo);
	XUI_TEST_CHECK(iRet == XUI_OK && tWinInfo.iState == XUI_DOCK_PANEL_WINDOW_FLOATING, "props still floating");
	iRet = xuiWidgetGetLayer(tWinInfo.pHostWidget, &iLayerProps, &iZProps);
	XUI_TEST_CHECK(iRet == XUI_OK && iLayerProps == XUI_LAYER_FLOATING, "props floating layer");
	iRet = xuiDockPanelGetWindowInfo(pDock, toolbox, &tWinInfo);
	XUI_TEST_CHECK(iRet == XUI_OK && tWinInfo.iState == XUI_DOCK_PANEL_WINDOW_FLOATING, "toolbox floating state");
	iRet = xuiWidgetGetLayer(tWinInfo.pHostWidget, &iLayerToolbox, &iZToolbox);
	XUI_TEST_CHECK(iRet == XUI_OK && iLayerToolbox == XUI_LAYER_FLOATING && iZToolbox > iZProps, "new floating window is topmost");
	iRet = xuiDockPanelGetWindowInfo(pDock, props, &tWinInfo);
	XUI_TEST_CHECK(iRet == XUI_OK && tWinInfo.tRect.fW > 0.0f, "props z click source");
	iRet = __xuiDockTestClick(pContext, 8.0f + tWinInfo.tRect.fX + 18.0f, 8.0f + tWinInfo.tRect.fY + 10.0f);
	if ( iRet == XUI_OK ) iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "props bring to front click");
	iRet = xuiDockPanelGetWindowInfo(pDock, props, &tWinInfo);
	XUI_TEST_CHECK(iRet == XUI_OK, "props info after bring front");
	iRet = xuiWidgetGetLayer(tWinInfo.pHostWidget, &iLayerProps, &iZProps);
	XUI_TEST_CHECK(iRet == XUI_OK, "props layer after bring front");
	iRet = xuiDockPanelGetWindowInfo(pDock, toolbox, &tWinInfo);
	XUI_TEST_CHECK(iRet == XUI_OK, "toolbox info after props bring front");
	iRet = xuiWidgetGetLayer(tWinInfo.pHostWidget, &iLayerToolbox, &iZToolbox);
	XUI_TEST_CHECK(iRet == XUI_OK && iZProps > iZToolbox, "click brings floating window to front");
	iRet = xuiDockPanelFloatWindow(pDock, props, (xui_rect_t){260.0f, 76.0f, 230.0f, 150.0f});
	if ( iRet == XUI_OK ) iRet = xuiDockPanelFloatWindow(pDock, toolbox, (xui_rect_t){286.0f, 86.0f, 230.0f, 150.0f});
	if ( iRet == XUI_OK ) iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "overlap floating windows");
	iRet = xuiDockPanelGetWindowInfo(pDock, props, &tWinInfo);
	if ( iRet == XUI_OK ) iRet = xuiDockPanelGetWindowInfo(pDock, toolbox, &tToolboxInfo);
	XUI_TEST_CHECK(iRet == XUI_OK && tToolboxInfo.iState == XUI_DOCK_PANEL_WINDOW_FLOATING && tWinInfo.iState == XUI_DOCK_PANEL_WINDOW_FLOATING, "overlap floating state");
	iRet = xuiDockPanelFloatWindow(pDock, props, tWinInfo.tRect);
	if ( iRet == XUI_OK ) iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "overlap props bring front api");
	iRet = xuiDockPanelGetWindowInfo(pDock, props, &tWinInfo);
	if ( iRet == XUI_OK ) iRet = xuiDockPanelGetWindowInfo(pDock, toolbox, &tToolboxInfo);
	XUI_TEST_CHECK(iRet == XUI_OK, "overlap front window info");
	iRet = xuiWidgetGetLayer(tWinInfo.pHostWidget, &iLayerProps, &iZProps);
	if ( iRet == XUI_OK ) iRet = xuiWidgetGetLayer(tToolboxInfo.pHostWidget, &iLayerToolbox, &iZToolbox);
	XUI_TEST_CHECK(iRet == XUI_OK && iZProps > iZToolbox, "overlap props visual front");
	pHitWidget = xuiHitTest(pContext, 8.0f + tWinInfo.tRect.fX + 48.0f, 8.0f + tWinInfo.tRect.fY + 14.0f, XUI_WIDGET_HIT_DEFAULT);
	XUI_TEST_CHECK(pHitWidget == tWinInfo.pHostWidget, "overlap front floating title hit");
	tClientWorld = xuiWidgetGetWorldRect(tWinInfo.pHostWidget);
	fOldX = tWinInfo.tRect.fX;
	iRet = xuiInputSetModifiers(pContext, XUI_MOD_CTRL);
	if ( iRet == XUI_OK ) iRet = xuiInputPointerDownEx(pContext, 1901u,
		XUI_POINTER_TYPE_TOUCH, (int)(tClientWorld.fX + 48.0f),
		(int)(tClientWorld.fY + 14.0f), XUI_POINTER_BUTTON_LEFT,
		XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK &&
		xuiGetPointerCaptureEx(pContext, 1901u, XUI_POINTER_TYPE_TOUCH) == tWinInfo.pHostWidget,
		"floating title captures exact owner touch");
	iRet = xuiInputPointerDownEx(pContext, 1902u, XUI_POINTER_TYPE_TOUCH,
		(int)(tClientWorld.fX + 54.0f), (int)(tClientWorld.fY + 14.0f),
		XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	if ( iRet == XUI_OK ) iRet = xuiInputPointerUpEx(pContext, 1902u,
		XUI_POINTER_TYPE_TOUCH, (int)(tClientWorld.fX + 120.0f),
		(int)(tClientWorld.fY + 40.0f), XUI_POINTER_BUTTON_LEFT, 0);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK &&
		xuiGetPointerCaptureEx(pContext, 1901u, XUI_POINTER_TYPE_TOUCH) == tWinInfo.pHostWidget &&
		xuiGetPointerCaptureEx(pContext, 1902u, XUI_POINTER_TYPE_TOUCH) == NULL,
		"unrelated touch cannot release floating title owner");
	iRet = xuiInputPointerMoveEx(pContext, 1901u, XUI_POINTER_TYPE_TOUCH,
		(int)(tClientWorld.fX + 98.0f), (int)(tClientWorld.fY + 34.0f),
		XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	if ( iRet == XUI_OK ) iRet = xuiInputPointerUpEx(pContext, 1901u,
		XUI_POINTER_TYPE_TOUCH, (int)(tClientWorld.fX + 98.0f),
		(int)(tClientWorld.fY + 34.0f), XUI_POINTER_BUTTON_LEFT, 0);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	if ( iRet == XUI_OK ) iRet = xuiInputSetModifiers(pContext, 0);
	if ( iRet == XUI_OK ) iRet = xuiLayout(pContext);
	if ( iRet == XUI_OK ) iRet = xuiDockPanelGetWindowInfo(pDock, props, &tWinInfo);
	XUI_TEST_CHECK(iRet == XUI_OK && tWinInfo.tRect.fX > fOldX + 20.0f &&
		xuiGetPointerCaptureEx(pContext, 1901u, XUI_POINTER_TYPE_TOUCH) == NULL,
		"floating owner touch moves window and releases capture");
	fOldX = tWinInfo.tRect.fX;
	iRet = xuiInputSetModifiers(pContext, XUI_MOD_CTRL);
	if ( iRet == XUI_OK ) iRet = xuiInputPointerDown(pContext, 8.0f + tWinInfo.tRect.fX + 48.0f, 8.0f + tWinInfo.tRect.fY + 14.0f, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiGetPointerCapture(pContext) == tWinInfo.pHostWidget, "overlap front floating title captures");
	if ( iRet == XUI_OK ) iRet = xuiInputPointerMove(pContext, 8.0f + tWinInfo.tRect.fX + 98.0f, 8.0f + tWinInfo.tRect.fY + 34.0f, XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	if ( iRet == XUI_OK ) iRet = xuiInputPointerUp(pContext, 8.0f + tWinInfo.tRect.fX + 98.0f, 8.0f + tWinInfo.tRect.fY + 34.0f, XUI_POINTER_BUTTON_LEFT, 0);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	if ( iRet == XUI_OK ) iRet = xuiInputSetModifiers(pContext, 0);
	if ( iRet == XUI_OK ) iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "overlap front floating title drag");
	iRet = xuiDockPanelGetWindowInfo(pDock, props, &tWinInfo);
	XUI_TEST_CHECK(iRet == XUI_OK && tWinInfo.tRect.fX > fOldX + 20.0f, "overlap front floating title moves");
	iRet = xuiDockPanelGetPaneInfo(pDock, docPane, &tPaneInfo);
	XUI_TEST_CHECK(iRet == XUI_OK && tPaneInfo.tRect.fW > 120.0f && tPaneInfo.tRect.fH > 120.0f, "indicator target pane");
	XUI_TEST_CHECK(__xuiDockFindPaneFreeDropPoint(pDock, props, tPaneInfo.tRect, &fNoDropX, &fNoDropY), "indicator free pane point");
	iRet = xuiInputPointerDown(pContext, 8.0f + tWinInfo.tRect.fX + 48.0f, 8.0f + tWinInfo.tRect.fY + 14.0f, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	if ( iRet == XUI_OK ) iRet = xuiInputPointerMove(pContext, 8.0f + fNoDropX, 8.0f + fNoDropY, XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "indicator drag over pane center");
	iRet = xuiDockPanelGetDragPreview(pDock, &tDrop);
	XUI_TEST_CHECK(iRet == XUI_OK && !tDrop.bValid, "pane center has indicator without drop preview");
	iRet = xuiBuiltinAssetGetRect("dock_indicator_pane_diamond", &tAssetRect);
	if ( iRet == XUI_OK ) iRet = xuiTestSurfaceCreate(&tState, &pTarget, 640, 420, XUI_SURFACE_USAGE_TARGET | XUI_SURFACE_ALPHA_PREMULTIPLIED);
	if ( iRet == XUI_OK ) iRet = xuiRender(pContext, pTarget, NULL, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "render pane diamond indicator overlay");
	iRenderNodeCount = xuiGetRenderNodeCount(pContext);
	iDragNodeIndex = -1;
	pIndicatorCache = NULL;
	for ( i = 0; i < iRenderNodeCount; i++ ) {
		iRet = xuiGetRenderNode(pContext, i, &tRenderNode);
		if ( iRet != XUI_OK ) break;
		if ( tRenderNode.iLayer == XUI_LAYER_DRAG ) {
			iDragNodeIndex = i;
			pDragOverlay = tRenderNode.pWidget;
			pIndicatorCache = xuiWidgetGetCacheSurface(tRenderNode.pWidget, tRenderNode.iStateId);
		}
	}
	XUI_TEST_CHECK(iRet == XUI_OK && iDragNodeIndex >= 0 && pIndicatorCache != NULL, "pane indicator overlay render node");
	tSrcRect = xuiTestSurfaceGetLastSrc(pIndicatorCache);
	XUI_TEST_CHECK(pIndicatorCache != NULL &&
		tSrcRect.fX == tAssetRect.fX && tSrcRect.fY == tAssetRect.fY &&
		tSrcRect.fW == tAssetRect.fW && tSrcRect.fH == tAssetRect.fH,
		"pane diamond indicator drawn above floating host before drop side");
	iRet = xuiInputPointerUp(pContext, 8.0f + fNoDropX, 8.0f + fNoDropY, XUI_POINTER_BUTTON_LEFT, 0);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	if ( iRet == XUI_OK ) iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "release pane indicator drag");
	XUI_TEST_CHECK(pDragOverlay != NULL && !xuiWidgetGetVisible(pDragOverlay),
		"pane indicator overlay hides on pointer release");
	xuiTestSurfaceReset(pTarget);
	iRet = xuiRender(pContext, pTarget, NULL, 0);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiTestSurfaceGetDrawCount(pTarget) > 0,
		"render after pane indicator release");
	iRet = xuiDockPanelGetWindowInfo(pDock, props, &tWinInfo);
	XUI_TEST_CHECK(iRet == XUI_OK && tWinInfo.iState == XUI_DOCK_PANEL_WINDOW_FLOATING, "free pane indicator release keeps floating");
	iRet = xuiDockPanelSaveState(pDock, &pState);
	XUI_TEST_CHECK(iRet == XUI_OK && pState != NULL, "save z order state");
	iRet = xuiDockPanelStateGetCounts(pState, NULL, NULL, &iFloatingCount);
	XUI_TEST_CHECK(iRet == XUI_OK && iFloatingCount == 2, "state keeps two floating windows");
	iRet = xuiDockPanelDockWindow(pDock, props, XUI_DOCK_PANEL_REGION_DOCUMENT, XUI_DOCK_PANEL_SIDE_FILL, 0.0f, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK, "mutate z order state");
	iRet = xuiDockPanelLoadState(pDock, pState);
	XUI_TEST_CHECK(iRet == XUI_OK, "restore z order state");
	xuiDockPanelStateFree(pState);
	pState = NULL;
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "layout z order restored");
	iRet = xuiDockPanelGetWindowInfo(pDock, props, &tWinInfo);
	XUI_TEST_CHECK(iRet == XUI_OK && tWinInfo.iState == XUI_DOCK_PANEL_WINDOW_FLOATING, "z state restores props floating");
	iRet = xuiWidgetGetLayer(tWinInfo.pHostWidget, &iLayerProps, &iZProps);
	XUI_TEST_CHECK(iRet == XUI_OK, "props z restored layer");
	iRet = xuiDockPanelGetWindowInfo(pDock, toolbox, &tWinInfo);
	XUI_TEST_CHECK(iRet == XUI_OK && tWinInfo.iState == XUI_DOCK_PANEL_WINDOW_FLOATING, "z state restores toolbox floating");
	iRet = xuiWidgetGetLayer(tWinInfo.pHostWidget, &iLayerToolbox, &iZToolbox);
	XUI_TEST_CHECK(iRet == XUI_OK && iZProps > iZToolbox, "load restores floating z order");
	iRet = xuiDockPanelGetWindowInfo(pDock, output, &tWinInfo);
	XUI_TEST_CHECK(iRet == XUI_OK && tWinInfo.iState == XUI_DOCK_PANEL_WINDOW_HIDDEN, "xson restores hidden");
	iRet = xuiDockPanelGetWindowInfo(pDock, doc2, &tWinInfo);
	XUI_TEST_CHECK(iRet == XUI_OK && tWinInfo.iState == XUI_DOCK_PANEL_WINDOW_DOCKED && tWinInfo.iPane >= 0, "xson restores document pane");
	docPane = tWinInfo.iPane;

	iRet = xuiDockPanelGetPaneInfo(pDock, docPane, &tPaneInfo);
	XUI_TEST_CHECK(iRet == XUI_OK && tPaneInfo.tClientRect.fW > 0.0f && tPaneInfo.tClientRect.fH > 0.0f, "pane info");
	iRet = xuiRenderPrepare(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "render prepare");
	pCache = xuiWidgetGetCacheSurface(pDock, xuiWidgetGetStateId(pDock));
	XUI_TEST_CHECK(pCache != NULL && xuiTestSurfaceGetRectFillCount(pCache) > 0, "dock cache draw");
	if ( pTarget == NULL ) iRet = xuiTestSurfaceCreate(&tState, &pTarget, 640, 420, XUI_SURFACE_USAGE_TARGET | XUI_SURFACE_ALPHA_PREMULTIPLIED);
	XUI_TEST_CHECK(iRet == XUI_OK && pTarget != NULL, "target create");
	iRet = xuiRender(pContext, pTarget, NULL, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "render target");
	XUI_TEST_CHECK(xuiDockPanelGetChangeCount(pDock) > 0 && tData.iStateChanged > 0, "change counters");

	memset(&tContextEvent, 0, sizeof(tContextEvent));
	tContextEvent.iSize = sizeof(tContextEvent);
	tContextEvent.iType = XUI_EVENT_CONTEXT_MENU;
	tContextEvent.pTarget = pDock;
	tContextEvent.iKey = XUI_KEY_CONTEXT_MENU;
	iRet = xuiDispatchEvent(pContext, &tContextEvent);
	XUI_TEST_CHECK(iRet == XUI_OK && tData.iContext == 1, "context callback before clear");
	iChangeBefore = xuiDockPanelGetChangeCount(pDock);
	iLayoutChangeBefore = xuiDockPanelGetLayoutChangeCount(pDock);
	iWindowChangeBefore = xuiDockPanelGetWindowChangeCount(pDock);
	iRet = xuiDockPanelClear(pDock);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiDockPanelGetWindowCount(pDock) == 0 &&
		xuiDockPanelGetChangeCount(pDock) > iChangeBefore &&
		xuiDockPanelGetLayoutChangeCount(pDock) > iLayoutChangeBefore &&
		xuiDockPanelGetWindowChangeCount(pDock) > iWindowChangeBefore,
		"clear dock content and preserve monotonic counters");
	iRet = xuiDispatchEvent(pContext, &tContextEvent);
	XUI_TEST_CHECK(iRet == XUI_OK && tData.iContext == 2, "clear preserves context callback and user data");
	XUI_TEST_CHECK(__xuiDockTestCapacityRollback(pContext, pRoot, &tDesc),
		"full layout rejects split without detaching source window");

	pTempDock = NULL;
	pTempClient = NULL;
	iTempWindow = -1;
	iRet = xuiDockPanelCreate(pContext, &pTempDock, &tDesc);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetRect(pTempDock, (xui_rect_t){24.0f, 24.0f, 360.0f, 260.0f});
	if ( iRet == XUI_OK ) iRet = xuiWidgetAddChild(pRoot, pTempDock);
	if ( iRet == XUI_OK ) iRet = xuiWidgetCreate(pContext, &pTempClient);
	if ( iRet == XUI_OK ) iRet = xuiDockPanelAddWindow(pTempDock, "Window mechanics", pTempClient, &iTempWindow);
	if ( iRet == XUI_OK ) iRet = xuiDockPanelSetWindowClose(pTempDock, __xuiDockTestClose, &tData);
	if ( iRet == XUI_OK ) iRet = xuiDockPanelSetWindowMinSize(pTempDock, iTempWindow, 200.0f, 150.0f);
	if ( iRet == XUI_OK ) iRet = xuiDockPanelSetWindowResizeEdges(pTempDock, iTempWindow,
		XUI_WINDOW_EDGE_RIGHT | XUI_WINDOW_EDGE_BOTTOM);
	if ( iRet == XUI_OK ) iRet = xuiDockPanelFloatWindow(pTempDock, iTempWindow,
		(xui_rect_t){42.0f, 38.0f, 240.0f, 180.0f});
	if ( iRet == XUI_OK ) iRet = xuiLayout(pContext);
	if ( iRet == XUI_OK ) iRet = xuiDockPanelGetWindowInfo(pTempDock, iTempWindow, &tWinInfo);
	XUI_TEST_CHECK(iRet == XUI_OK && tWinInfo.bMovable && tWinInfo.bResizable &&
		tWinInfo.iResizeEdges == (XUI_WINDOW_EDGE_RIGHT | XUI_WINDOW_EDGE_BOTTOM) &&
		tWinInfo.fMinWidth == 200.0f && tWinInfo.fMinHeight == 150.0f &&
		tWinInfo.pCloseButtonWidget != NULL && tWinInfo.pPinButtonWidget != NULL,
		"dock window mechanics api defaults and overrides");
	pTempButton = xuiDockPanelGetWindowCloseButtonWidget(pTempDock, iTempWindow);
	XUI_TEST_CHECK(pTempButton == tWinInfo.pCloseButtonWidget && xuiWidgetGetVisible(pTempButton),
		"floating close is a visible child widget");
	iRet = xuiDockPanelSetWindowFlags(pTempDock, iTempWindow, 0, 1);
	if ( iRet == XUI_OK ) iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && !xuiWidgetGetVisible(pTempButton), "nonclosable floating window hides close button");
	iRet = xuiDockPanelSetWindowFlags(pTempDock, iTempWindow, 1, 1);
	if ( iRet == XUI_OK ) iRet = xuiLayout(pContext);
	tTempButtonWorld = xuiWidgetGetWorldRect(pTempButton);
	iCloseBefore = tData.iClose;
	iRet = xuiInputPointerDown(pContext, tTempButtonWorld.fX + tTempButtonWorld.fW * 0.5f,
		tTempButtonWorld.fY + tTempButtonWorld.fH * 0.5f, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	if ( iRet == XUI_OK ) iRet = xuiDockPanelGetWindowInfo(pTempDock, iTempWindow, &tWinInfo);
	XUI_TEST_CHECK(iRet == XUI_OK && tWinInfo.iState == XUI_DOCK_PANEL_WINDOW_FLOATING &&
		tData.iClose == iCloseBefore, "close button does not commit on pointer down");
	iRet = xuiInputPointerUp(pContext, tTempButtonWorld.fX - 12.0f, tTempButtonWorld.fY - 12.0f,
		XUI_POINTER_BUTTON_LEFT, 0);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	if ( iRet == XUI_OK ) iRet = xuiDockPanelGetWindowInfo(pTempDock, iTempWindow, &tWinInfo);
	XUI_TEST_CHECK(iRet == XUI_OK && tWinInfo.iState == XUI_DOCK_PANEL_WINDOW_FLOATING &&
		tData.iClose == iCloseBefore, "close button pointer-up outside cancels");
	iRet = xuiDockPanelSetWindowMovable(pTempDock, iTempWindow, 0);
	if ( iRet == XUI_OK ) iRet = xuiDockPanelGetWindowInfo(pTempDock, iTempWindow, &tWinInfo);
	fOldX = tWinInfo.tRect.fX;
	fOldY = tWinInfo.tRect.fY;
	if ( iRet == XUI_OK ) iRet = __xuiDockTestDrag(pContext,
		xuiWidgetGetWorldRect(tWinInfo.pHostWidget).fX + 36.0f,
		xuiWidgetGetWorldRect(tWinInfo.pHostWidget).fY + 12.0f,
		xuiWidgetGetWorldRect(tWinInfo.pHostWidget).fX + 76.0f,
		xuiWidgetGetWorldRect(tWinInfo.pHostWidget).fY + 42.0f);
	if ( iRet == XUI_OK ) iRet = xuiLayout(pContext);
	if ( iRet == XUI_OK ) iRet = xuiDockPanelGetWindowInfo(pTempDock, iTempWindow, &tWinInfo);
	XUI_TEST_CHECK(iRet == XUI_OK && tWinInfo.tRect.fX == fOldX && tWinInfo.tRect.fY == fOldY,
		"nonmovable floating title does not move");
	iRet = xuiDockPanelSetWindowMovable(pTempDock, iTempWindow, 1);
	if ( iRet == XUI_OK ) iRet = xuiDockPanelSetWindowResizable(pTempDock, iTempWindow, 1);
	if ( iRet == XUI_OK ) iRet = xuiDockPanelGetWindowInfo(pTempDock, iTempWindow, &tWinInfo);
	tDockWorld = xuiWidgetGetWorldRect(tWinInfo.pHostWidget);
	if ( iRet == XUI_OK ) iRet = __xuiDockTestDrag(pContext,
		tDockWorld.fX + tDockWorld.fW - 1.0f, tDockWorld.fY + tDockWorld.fH * 0.5f,
		tDockWorld.fX + 20.0f, tDockWorld.fY + tDockWorld.fH * 0.5f);
	if ( iRet == XUI_OK ) iRet = xuiLayout(pContext);
	if ( iRet == XUI_OK ) iRet = xuiDockPanelGetWindowInfo(pTempDock, iTempWindow, &tWinInfo);
	XUI_TEST_CHECK(iRet == XUI_OK && tWinInfo.tRect.fW >= 200.0f && tWinInfo.tRect.fH >= 150.0f,
		"floating resize respects per-window minimum size");
	pTempButton = xuiDockPanelGetWindowCloseButtonWidget(pTempDock, iTempWindow);
	tTempButtonWorld = xuiWidgetGetWorldRect(pTempButton);
	iRet = __xuiDockTestClick(pContext,
		tTempButtonWorld.fX + tTempButtonWorld.fW * 0.5f,
		tTempButtonWorld.fY + tTempButtonWorld.fH * 0.5f);
	if ( iRet == XUI_OK ) iRet = xuiLayout(pContext);
	if ( iRet == XUI_OK ) iRet = xuiDockPanelGetWindowInfo(pTempDock, iTempWindow, &tWinInfo);
	XUI_TEST_CHECK(iRet == XUI_OK && tWinInfo.iState == XUI_DOCK_PANEL_WINDOW_HIDDEN &&
		tData.iClose == iCloseBefore + 1, "close button commits on pointer-up inside");
	xuiWidgetDestroy(pTempDock);
	pTempDock = NULL;
	pTempClient = NULL;
	pTempButton = NULL;

	memset(&tDestroyData, 0, sizeof(tDestroyData));
	pTempDock = NULL;
	pTempClient = NULL;
	iTempWindow = -1;
	iRet = xuiDockPanelCreate(pContext, &pTempDock, &tDesc);
	if ( iRet == XUI_OK ) iRet = xuiWidgetAddChild(pRoot, pTempDock);
	if ( iRet == XUI_OK ) iRet = xuiWidgetCreate(pContext, &pTempClient);
	if ( iRet == XUI_OK ) iRet = xuiDockPanelAddWindow(pTempDock, "Destroy state", pTempClient, &iTempWindow);
	if ( iRet == XUI_OK ) {
		pTempClient = NULL;
		tDestroyData.ppWidget = &pTempDock;
		iRet = xuiDockPanelSetWindowStateChanged(pTempDock, __xuiDockTestDestroyOnState, &tDestroyData);
	}
	if ( iRet == XUI_OK ) iRet = xuiDockPanelDockWindow(pTempDock, iTempWindow,
		XUI_DOCK_PANEL_REGION_DOCUMENT, XUI_DOCK_PANEL_SIDE_FILL, 0.0f, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK && pTempDock == NULL && tDestroyData.iCount == 1,
		"state callback may destroy dock panel");

	memset(&tDestroyData, 0, sizeof(tDestroyData));
	pTempDock = NULL;
	pTempClient = NULL;
	pTempClient2 = NULL;
	iTempWindow = -1;
	iTempWindow2 = -1;
	dragPane = -1;
	iRet = xuiDockPanelCreate(pContext, &pTempDock, &tDesc);
	if ( iRet == XUI_OK ) iRet = xuiWidgetAddChild(pRoot, pTempDock);
	if ( iRet == XUI_OK ) iRet = xuiWidgetCreate(pContext, &pTempClient);
	if ( iRet == XUI_OK ) iRet = xuiWidgetCreate(pContext, &pTempClient2);
	if ( iRet == XUI_OK ) iRet = xuiDockPanelAddWindow(pTempDock, "Active A", pTempClient, &iTempWindow);
	if ( iRet == XUI_OK ) {
		pTempClient = NULL;
		iRet = xuiDockPanelAddWindow(pTempDock, "Active B", pTempClient2, &iTempWindow2);
	}
	if ( iRet == XUI_OK ) pTempClient2 = NULL;
	if ( iRet == XUI_OK ) iRet = xuiDockPanelDockWindow(pTempDock, iTempWindow,
		XUI_DOCK_PANEL_REGION_DOCUMENT, XUI_DOCK_PANEL_SIDE_FILL, 0.0f, &dragPane);
	if ( iRet == XUI_OK ) iRet = xuiDockPanelDockWindowToPane(pTempDock, iTempWindow2, dragPane);
	if ( iRet == XUI_OK ) {
		tDestroyData.ppWidget = &pTempDock;
		iRet = xuiDockPanelSetActiveChanged(pTempDock, __xuiDockTestDestroyOnActive, &tDestroyData);
	}
	if ( iRet == XUI_OK ) iRet = xuiDockPanelSetPaneActiveWindow(pTempDock, dragPane, iTempWindow);
	XUI_TEST_CHECK(iRet == XUI_OK && pTempDock == NULL && tDestroyData.iCount == 1,
		"active callback may destroy dock panel");

	memset(&tDestroyData, 0, sizeof(tDestroyData));
	pTempDock = NULL;
	pTempClient = NULL;
	pTempButton = NULL;
	iTempWindow = -1;
	iRet = xuiDockPanelCreate(pContext, &pTempDock, &tDesc);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetRect(pTempDock, (xui_rect_t){24.0f, 24.0f, 300.0f, 210.0f});
	if ( iRet == XUI_OK ) iRet = xuiWidgetAddChild(pRoot, pTempDock);
	if ( iRet == XUI_OK ) iRet = xuiWidgetCreate(pContext, &pTempClient);
	if ( iRet == XUI_OK ) iRet = xuiDockPanelAddWindow(pTempDock, "Destroy close", pTempClient, &iTempWindow);
	if ( iRet == XUI_OK ) pTempClient = NULL;
	if ( iRet == XUI_OK ) iRet = xuiDockPanelFloatWindow(pTempDock, iTempWindow,
		(xui_rect_t){32.0f, 28.0f, 210.0f, 140.0f});
	if ( iRet == XUI_OK ) iRet = xuiLayout(pContext);
	if ( iRet == XUI_OK ) pTempButton = xuiDockPanelGetWindowCloseButtonWidget(pTempDock, iTempWindow);
	if ( iRet == XUI_OK && pTempButton != NULL ) tTempButtonWorld = xuiWidgetGetWorldRect(pTempButton);
	if ( iRet == XUI_OK ) {
		tDestroyData.ppWidget = &pTempDock;
		iRet = xuiDockPanelSetWindowClose(pTempDock, __xuiDockTestDestroyOnClose, &tDestroyData);
	}
	if ( iRet == XUI_OK ) iRet = __xuiDockTestClick(pContext,
		tTempButtonWorld.fX + tTempButtonWorld.fW * 0.5f,
		tTempButtonWorld.fY + tTempButtonWorld.fH * 0.5f);
	pTempButton = NULL;
	XUI_TEST_CHECK(iRet == XUI_OK && pTempDock == NULL && tDestroyData.iCount == 1,
		"close callback may destroy dock panel");

	memset(&tReplaceCloseData, 0, sizeof(tReplaceCloseData));
	pTempDock = NULL;
	pTempClient = NULL;
	pTempClient2 = NULL;
	pTempButton = NULL;
	iTempWindow = -1;
	iRet = xuiDockPanelCreate(pContext, &pTempDock, &tDesc);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetRect(pTempDock, (xui_rect_t){24.0f, 24.0f, 300.0f, 210.0f});
	if ( iRet == XUI_OK ) iRet = xuiWidgetAddChild(pRoot, pTempDock);
	if ( iRet == XUI_OK ) iRet = xuiWidgetCreate(pContext, &pTempClient);
	if ( iRet == XUI_OK ) iRet = xuiDockPanelAddWindow(pTempDock, "Original", pTempClient, &iTempWindow);
	if ( iRet == XUI_OK ) pTempClient = NULL;
	if ( iRet == XUI_OK ) iRet = xuiDockPanelFloatWindow(pTempDock, iTempWindow,
		(xui_rect_t){32.0f, 28.0f, 210.0f, 140.0f});
	if ( iRet == XUI_OK ) iRet = xuiLayout(pContext);
	if ( iRet == XUI_OK ) iRet = xuiWidgetCreate(pContext, &pTempClient2);
	if ( iRet == XUI_OK ) {
		tReplaceCloseData.pReplacementClient = pTempClient2;
		pTempClient2 = NULL;
		iRet = xuiDockPanelSetWindowClose(pTempDock, __xuiDockTestReplaceOnClose, &tReplaceCloseData);
	}
	if ( iRet == XUI_OK ) pTempButton = xuiDockPanelGetWindowCloseButtonWidget(pTempDock, iTempWindow);
	if ( iRet == XUI_OK && pTempButton != NULL ) tTempButtonWorld = xuiWidgetGetWorldRect(pTempButton);
	if ( iRet == XUI_OK ) iRet = __xuiDockTestClick(pContext,
		tTempButtonWorld.fX + tTempButtonWorld.fW * 0.5f,
		tTempButtonWorld.fY + tTempButtonWorld.fH * 0.5f);
	pTempClient2 = tReplaceCloseData.pReplacementClient;
	tReplaceCloseData.pReplacementClient = NULL;
	if ( iRet == XUI_OK ) iRet = xuiLayout(pContext);
	if ( iRet == XUI_OK ) iRet = xuiDockPanelGetWindowInfo(pTempDock,
		tReplaceCloseData.iReplacementWindow, &tWinInfo);
	XUI_TEST_CHECK(iRet == XUI_OK && tReplaceCloseData.iResult == XUI_OK &&
		tReplaceCloseData.iCount == 1 && tWinInfo.iState == XUI_DOCK_PANEL_WINDOW_DOCKED &&
		strcmp(tWinInfo.sTitle, "Replacement") == 0,
		"close callback replacement is not hidden by stale close operation");
	pTempButton = NULL;
	xuiWidgetDestroy(pTempDock);
	pTempDock = NULL;
	pTempClient2 = NULL;

	memset(&tDestroyData, 0, sizeof(tDestroyData));
	pTempDock = NULL;
	pTempClient = NULL;
	iTempWindow = -1;
	iRet = xuiDockPanelCreate(pContext, &pTempDock, &tDesc);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetRect(pTempDock, (xui_rect_t){24.0f, 24.0f, 300.0f, 210.0f});
	if ( iRet == XUI_OK ) iRet = xuiWidgetAddChild(pRoot, pTempDock);
	if ( iRet == XUI_OK ) iRet = xuiWidgetCreate(pContext, &pTempClient);
	if ( iRet == XUI_OK ) iRet = xuiDockPanelAddWindow(pTempDock, "Destroy host context", pTempClient, &iTempWindow);
	if ( iRet == XUI_OK ) pTempClient = NULL;
	if ( iRet == XUI_OK ) iRet = xuiDockPanelFloatWindow(pTempDock, iTempWindow,
		(xui_rect_t){32.0f, 28.0f, 210.0f, 140.0f});
	if ( iRet == XUI_OK ) iRet = xuiLayout(pContext);
	if ( iRet == XUI_OK ) iRet = xuiDockPanelGetWindowInfo(pTempDock, iTempWindow, &tWinInfo);
	if ( iRet == XUI_OK ) tDockWorld = xuiWidgetGetWorldRect(tWinInfo.pHostWidget);
	if ( iRet == XUI_OK ) {
		tDestroyData.ppWidget = &pTempDock;
		iRet = xuiDockPanelSetContextMenu(pTempDock, __xuiDockTestDestroyOnContext, &tDestroyData);
	}
	memset(&tContextEvent, 0, sizeof(tContextEvent));
	tContextEvent.iSize = sizeof(tContextEvent);
	tContextEvent.iType = XUI_EVENT_CONTEXT_MENU;
	tContextEvent.pTarget = tWinInfo.pHostWidget;
	tContextEvent.fX = tDockWorld.fX + 12.0f;
	tContextEvent.fY = tDockWorld.fY + 8.0f;
	if ( iRet == XUI_OK ) iRet = xuiDispatchEvent(pContext, &tContextEvent);
	XUI_TEST_CHECK((iRet == XUI_OK || iRet == XUI_EVENT_DISPATCH_STOP) &&
		pTempDock == NULL && tDestroyData.iCount == 1,
		"floating host context callback may destroy dock panel");

	memset(&tDestroyData, 0, sizeof(tDestroyData));
	pTempDock = NULL;
	iRet = xuiDockPanelCreate(pContext, &pTempDock, &tDesc);
	if ( iRet == XUI_OK ) iRet = xuiWidgetAddChild(pRoot, pTempDock);
	if ( iRet == XUI_OK ) {
		tDestroyData.ppWidget = &pTempDock;
		iRet = xuiDockPanelSetContextMenu(pTempDock, __xuiDockTestDestroyOnContext, &tDestroyData);
	}
	memset(&tContextEvent, 0, sizeof(tContextEvent));
	tContextEvent.iSize = sizeof(tContextEvent);
	tContextEvent.iType = XUI_EVENT_CONTEXT_MENU;
	tContextEvent.pTarget = pTempDock;
	tContextEvent.iKey = XUI_KEY_CONTEXT_MENU;
	if ( iRet == XUI_OK ) iRet = xuiDispatchEvent(pContext, &tContextEvent);
	XUI_TEST_CHECK(iRet == XUI_OK && pTempDock == NULL && tDestroyData.iCount == 1,
		"context callback may destroy dock panel");

	iOverlayChildCount = xuiWidgetGetChildCount(xuiOverlayRoot(pContext));
	for ( i = 0; i < 24; i++ ) {
		pTempDock = NULL;
		pTempClient = NULL;
		iTempWindow = -1;
		iRet = xuiDockPanelCreate(pContext, &pTempDock, &tDesc);
		if ( iRet == XUI_OK ) iRet = xuiWidgetSetRect(pTempDock, (xui_rect_t){24.0f, 24.0f, 280.0f, 190.0f});
		if ( iRet == XUI_OK ) iRet = xuiWidgetAddChild(pRoot, pTempDock);
		if ( iRet == XUI_OK ) iRet = xuiWidgetCreate(pContext, &pTempClient);
		if ( iRet == XUI_OK ) iRet = xuiDockPanelAddWindow(pTempDock, "Lifecycle", pTempClient, &iTempWindow);
		if ( iRet == XUI_OK ) iRet = xuiDockPanelDockWindow(pTempDock, iTempWindow, XUI_DOCK_PANEL_REGION_DOCUMENT, XUI_DOCK_PANEL_SIDE_FILL, 0.0f, NULL);
		if ( iRet == XUI_OK ) iRet = xuiLayout(pContext);
		if ( iRet == XUI_OK ) iRet = xuiDockPanelFloatWindow(pTempDock, iTempWindow, (xui_rect_t){32.0f, 28.0f, 210.0f, 140.0f});
		if ( iRet == XUI_OK ) iRet = xuiLayout(pContext);
		if ( iRet != XUI_OK ) break;
		xuiWidgetDestroy(pTempDock);
		pTempDock = NULL;
		pTempClient = NULL;
	}
	XUI_TEST_CHECK(iRet == XUI_OK && i == 24, "dock lifecycle stress");
	XUI_TEST_CHECK(xuiWidgetGetChildCount(xuiOverlayRoot(pContext)) == iOverlayChildCount, "dock lifecycle releases overlays");

cleanup:
	if ( tReplaceCloseData.pReplacementClient != NULL &&
	     xuiWidgetGetParent(tReplaceCloseData.pReplacementClient) == NULL ) {
		xuiWidgetDestroy(tReplaceCloseData.pReplacementClient);
	}
	if ( pTempClient2 != NULL && xuiWidgetGetParent(pTempClient2) == NULL ) xuiWidgetDestroy(pTempClient2);
	if ( pTempClient != NULL && xuiWidgetGetParent(pTempClient) == NULL ) xuiWidgetDestroy(pTempClient);
	if ( pTempDock != NULL ) xuiWidgetDestroy(pTempDock);
	if ( pState != NULL ) xuiDockPanelStateFree(pState);
	if ( pTarget != NULL ) tState.tProxy.surfaceDestroy(&tState.tProxy, pTarget);
	if ( pFont2 != NULL ) tState.tProxy.fontDestroy(&tState.tProxy, pFont2);
	if ( pFont != NULL ) tState.tProxy.fontDestroy(&tState.tProxy, pFont);
	if ( pContext != NULL ) xuiDestroy(pContext);
	if ( iFailed ) return 1;
	printf("xui_dock_panel_test passed\n");
	return 0;
}
