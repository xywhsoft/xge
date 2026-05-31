#include "xui.h"
#include "xui_test_proxy.h"

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
} dock_test_data_t;

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

int main(void)
{
	xui_test_proxy_state_t tState;
	xui_context pContext;
	xui_widget pRoot;
	xui_widget pDock;
	xui_widget arrClient[11];
	xui_widget pHitWidget;
	xui_widget pMenu;
	xui_surface pTarget;
	xui_surface pCache;
	xui_surface pIndicatorCache;
	xui_font pFont;
	xui_dock_panel_desc_t tDesc;
	xui_dock_window_info_t tWinInfo;
	xui_dock_window_info_t tToolboxInfo;
	xui_dock_pane_info_t tPaneInfo;
	xui_dock_hit_t tHit;
	xui_dock_drop_info_t tDrop;
	xui_render_node_t tRenderNode;
	xvalue pState;
	dock_test_data_t tData;
	int iFailed;
	int iRet;
	int iRegionCount;
	int iWindowCount;
	int iFloatingCount;
	int iLayerProps;
	int iLayerToolbox;
	int iZProps;
	int iZToolbox;
	int iMenuIndex;
	int iRenderNodeCount;
	int iDragNodeIndex;
	float fOldX;
	float fOldW;
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
	int i;

	pContext = NULL;
	pRoot = NULL;
	pDock = NULL;
	pHitWidget = NULL;
	pMenu = NULL;
	pTarget = NULL;
	pIndicatorCache = NULL;
	pFont = NULL;
	pState = NULL;
	iFailed = 0;
	iRegionCount = iWindowCount = iFloatingCount = 0;
	iLayerProps = iLayerToolbox = iZProps = iZToolbox = 0;
	iMenuIndex = -1;
	iRenderNodeCount = 0;
	iDragNodeIndex = -1;
	iActiveBefore = 0;
	fRegionBefore = fRegionAfter = 0.0f;
	fAutoHidePaneW = 0.0f;
	fNoDropX = fNoDropY = 0.0f;
	doc1 = doc2 = doc3 = doc4 = doc5 = toolbox = props = output = -1;
	scratch1 = scratch2 = scratch3 = -1;
	docPane = toolboxPane = propsPane = outputPane = scratchPane = -1;
	dragPane = -1;
	memset(arrClient, 0, sizeof(arrClient));
	memset(&tData, 0, sizeof(tData));
	xuiTestProxyInit(&tState);

	iRet = xuiCreate(&pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && pContext != NULL, "context create");
	iRet = xuiSetProxy(pContext, &tState.tProxy);
	XUI_TEST_CHECK(iRet == XUI_OK, "proxy set");
	iRet = xuiInputViewport(pContext, 640.0f, 420.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "viewport");
	iRet = tState.tProxy.fontLoadMemory(&tState.tProxy, &pFont, "dock", 4, 13.0f, XUI_FONT_FORMAT_TTF);
	XUI_TEST_CHECK(iRet == XUI_OK && pFont != NULL, "font create");
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

	for ( i = 0; i < 11; i++ ) {
		iRet = xuiWidgetCreate(pContext, &arrClient[i]);
		XUI_TEST_CHECK(iRet == XUI_OK && arrClient[i] != NULL, "client create");
		iRet = xuiWidgetSetFocusable(arrClient[i], 1);
		XUI_TEST_CHECK(iRet == XUI_OK, "client focusable");
		iRet = xuiWidgetSetTabStop(arrClient[i], 0);
		XUI_TEST_CHECK(iRet == XUI_OK, "client tab stop");
	}
	iRet = xuiDockPanelAddWindow(pDock, "Document.c", arrClient[0], &doc1);
	XUI_TEST_CHECK(iRet == XUI_OK, "doc1 add");
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
	XUI_TEST_CHECK(xuiDockPanelGetWindowCount(pDock) == 11, "window count");
	XUI_TEST_CHECK(xuiDockPanelGetPaneCount(pDock) == 4, "pane count");
	XUI_TEST_CHECK(xuiDockPanelGetPaneWindowCount(pDock, docPane) == 5, "doc tab count");
	XUI_TEST_CHECK(xuiDockPanelGetPaneActiveWindow(pDock, docPane) == doc2, "doc2 active");

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
	iRet = __xuiDockTestDrag(pContext, 8.0f + tHit.tRect.fX + tHit.tRect.fW * 0.5f, 8.0f + tHit.tRect.fY + tHit.tRect.fH * 0.5f, 8.0f + tHit.tRect.fX + 42.0f, 8.0f + tHit.tRect.fY + tHit.tRect.fH * 0.5f);
	if ( iRet == XUI_OK ) iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "side region splitter drag");
	iRet = xuiDockPanelGetRegionSize(pDock, XUI_DOCK_PANEL_REGION_LEFT, NULL, &fRegionAfter);
	XUI_TEST_CHECK(iRet == XUI_OK && fRegionAfter > fRegionBefore + 0.03f, "side region splitter updates size");
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
	iRet = __xuiDockTestDrag(pContext, 8.0f + tHit.tRect.fX + 1.0f, 8.0f + tHit.tRect.fY + 1.0f, 8.0f + tHit.tRect.fX + 24.0f, 8.0f + tHit.tRect.fY + 12.0f);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiDockPanelGetLayoutChangeCount(pDock) > 0, "splitter drag");

	iRet = xuiDockPanelGetPaneInfo(pDock, toolboxPane, &tPaneInfo);
	XUI_TEST_CHECK(iRet == XUI_OK && tPaneInfo.tRect.fW > 0.0f, "auto hide remembers source pane");
	fAutoHidePaneW = tPaneInfo.tRect.fW;
	iRet = xuiDockPanelAutoHideWindow(pDock, toolbox);
	XUI_TEST_CHECK(iRet == XUI_OK, "auto hide");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "layout auto hide");
	iRet = xuiDockPanelGetWindowInfo(pDock, toolbox, &tWinInfo);
	XUI_TEST_CHECK(iRet == XUI_OK && tWinInfo.iState == XUI_DOCK_PANEL_WINDOW_AUTO_HIDE, "auto hide state");
	XUI_TEST_CHECK(tWinInfo.iLastRegion == XUI_DOCK_PANEL_REGION_DOCUMENT && tWinInfo.iLastSide == XUI_DOCK_PANEL_SIDE_LEFT, "auto hide keeps dock target");
	XUI_TEST_CHECK(tWinInfo.tAutoHideRect.fW > 0.0f && tWinInfo.tAutoHideRect.fH > 0.0f, "auto hide strip");
	iRet = __xuiDockTestClick(pContext, 8.0f + tWinInfo.tAutoHideRect.fX + tWinInfo.tAutoHideRect.fW * 0.5f, 8.0f + tWinInfo.tAutoHideRect.fY + tWinInfo.tAutoHideRect.fH * 0.5f);
	XUI_TEST_CHECK(iRet == XUI_OK, "auto hide strip click");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "layout auto hide expand");
	iRet = xuiDockPanelGetWindowInfo(pDock, toolbox, &tWinInfo);
	XUI_TEST_CHECK(iRet == XUI_OK && tWinInfo.iState == XUI_DOCK_PANEL_WINDOW_AUTO_HIDE && xuiDockPanelGetAutoHideExpandedWindow(pDock) == toolbox, "auto hide expanded state");
	XUI_TEST_CHECK(xuiWidgetGetVisible(tWinInfo.pHostWidget) && tWinInfo.tRect.fW > 0.0f && tWinInfo.tClientRect.fW > 0.0f, "auto hide overlay visible");
	tHit.tRect = xuiDockPanelGetAutoHideExpandRect(pDock);
	XUI_TEST_CHECK(tHit.tRect.fW > 0.0f && tHit.tRect.fH > 0.0f, "auto hide expand rect");
	if ( fAutoHidePaneW < 160.0f ) {
		XUI_TEST_CHECK(tHit.tRect.fW >= 159.0f && tHit.tRect.fW <= 190.0f, "auto hide clamps remembered small width");
	} else if ( fAutoHidePaneW < 420.0f ) {
		XUI_TEST_CHECK(tHit.tRect.fW > fAutoHidePaneW - 6.0f && tHit.tRect.fW < fAutoHidePaneW + 6.0f, "auto hide expand remembers pane width");
	} else {
		XUI_TEST_CHECK(tHit.tRect.fW <= 420.0f, "auto hide expand clamps large width");
	}
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

	iRet = xuiDockPanelFloatWindow(pDock, props, (xui_rect_t){330.0f, 44.0f, 210.0f, 150.0f});
	XUI_TEST_CHECK(iRet == XUI_OK, "float props");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "layout float");
	iRet = xuiDockPanelGetWindowInfo(pDock, props, &tWinInfo);
	XUI_TEST_CHECK(iRet == XUI_OK && tWinInfo.iState == XUI_DOCK_PANEL_WINDOW_FLOATING && xuiWidgetGetVisible(tWinInfo.pHostWidget), "float visible");
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
	XUI_TEST_CHECK(iRet == XUI_OK && tWinInfo.tTabRect.fW > 0.0f, "docked tab drag source");
	iRet = xuiDockPanelGetPaneInfo(pDock, docPane, &tPaneInfo);
	XUI_TEST_CHECK(iRet == XUI_OK && tPaneInfo.tRect.fW > 0.0f, "docked tab drag target");
	iRet = __xuiDockTestDrag(pContext,
		8.0f + tWinInfo.tTabRect.fX + tWinInfo.tTabRect.fW * 0.5f,
		8.0f + tWinInfo.tTabRect.fY + tWinInfo.tTabRect.fH * 0.5f,
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
		8.0f + tWinInfo.tRect.fY + 3.0f,
		8.0f + tWinInfo.tRect.fX + 72.0f,
		8.0f + tWinInfo.tRect.fY + 23.0f);
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
	iRet = __xuiDockTestDrag(pContext,
		8.0f + tWinInfo.tRect.fX + 2.0f,
		8.0f + tWinInfo.tRect.fY + tWinInfo.tRect.fH * 0.5f,
		8.0f + tWinInfo.tRect.fX + 38.0f,
		8.0f + tWinInfo.tRect.fY + tWinInfo.tRect.fH * 0.5f);
	if ( iRet == XUI_OK ) iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "floating resize drag");
	iRet = xuiDockPanelGetWindowInfo(pDock, props, &tWinInfo);
	XUI_TEST_CHECK(iRet == XUI_OK && tWinInfo.iState == XUI_DOCK_PANEL_WINDOW_FLOATING && tWinInfo.tRect.fX > fOldX + 10.0f && tWinInfo.tRect.fW < fOldW - 10.0f, "floating resize updates rect");
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

cleanup:
	if ( pState != NULL ) xuiDockPanelStateFree(pState);
	if ( pTarget != NULL ) tState.tProxy.surfaceDestroy(&tState.tProxy, pTarget);
	if ( pFont != NULL ) tState.tProxy.fontDestroy(&tState.tProxy, pFont);
	if ( pContext != NULL ) xuiDestroy(pContext);
	if ( iFailed ) return 1;
	printf("xui_dock_panel_test passed\n");
	return 0;
}
