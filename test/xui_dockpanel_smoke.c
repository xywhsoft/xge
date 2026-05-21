#include "../xge.h"
#include <stdio.h>
#include <string.h>

typedef struct dock_smoke_host_t {
	int iDrawRect;
	int iDrawImage;
	int iDrawText;
	int iMeasureText;
	int iClipSet;
	int iClipClear;
	int bClipEnabled;
	xge_rect_t tLastClip;
} dock_smoke_host_t;

static void DockSmokeDrawRect(xge_rect_t tRect, uint32_t iColor, void* pUser)
{
	dock_smoke_host_t* pHost;

	(void)tRect;
	(void)iColor;
	pHost = (dock_smoke_host_t*)pUser;
	pHost->iDrawRect++;
}

static void DockSmokeDrawImage(const xge_draw_t* pDraw, void* pUser)
{
	dock_smoke_host_t* pHost;

	(void)pDraw;
	pHost = (dock_smoke_host_t*)pUser;
	pHost->iDrawImage++;
}

static void DockSmokeDrawText(xui_font pFont, const char* sText, xge_rect_t tRect, uint32_t iColor, uint32_t iFlags, void* pUser)
{
	dock_smoke_host_t* pHost;

	(void)pFont;
	(void)sText;
	(void)tRect;
	(void)iColor;
	(void)iFlags;
	pHost = (dock_smoke_host_t*)pUser;
	pHost->iDrawText++;
}

static xge_vec2_t DockSmokeMeasureText(xui_font pFont, const char* sText, void* pUser)
{
	dock_smoke_host_t* pHost;
	xge_vec2_t tSize;

	(void)pFont;
	pHost = (dock_smoke_host_t*)pUser;
	pHost->iMeasureText++;
	tSize.fX = (float)(sText != NULL ? strlen(sText) : 0u) * 8.0f;
	tSize.fY = 16.0f;
	return tSize;
}

static void DockSmokeClipSet(xge_rect_t tRect, void* pUser)
{
	dock_smoke_host_t* pHost;

	pHost = (dock_smoke_host_t*)pUser;
	pHost->tLastClip = tRect;
	pHost->bClipEnabled = 1;
	pHost->iClipSet++;
}

static void DockSmokeClipClear(void* pUser)
{
	dock_smoke_host_t* pHost;

	pHost = (dock_smoke_host_t*)pUser;
	pHost->bClipEnabled = 0;
	pHost->iClipClear++;
}

static int Fail(int iCode, const char* sStep)
{
	printf("xui_dockpanel_smoke failed code=%d step=%s\n", iCode, sStep != NULL ? sStep : "");
	return iCode;
}

static int CountDockPanes(xge_xui_dock_node pNode)
{
	if ( pNode == NULL ) {
		return 0;
	}
	if ( pNode->iType == XGE_XUI_DOCK_NODE_PANE ) {
		return 1;
	}
	return CountDockPanes(pNode->pFirst) + CountDockPanes(pNode->pSecond);
}

static int CheckBuiltinAssetRect(const char* sName, float fW, float fH)
{
	xge_rect_t tRect;

	if ( xgeXuiBuiltinAssetGetRect(sName, &tRect) != XGE_OK ) {
		return 0;
	}
	return (tRect.fX >= 0.0f) && (tRect.fY >= 0.0f) && (tRect.fW == fW) && (tRect.fH == fH);
}

static int PaneSatisfiesAxisMin(xge_xui_dock_pane pPane, int iAxis)
{
	float fSize;
	float fMin;

	if ( (pPane == NULL) || (pPane->pNode == NULL) ) {
		return 0;
	}
	if ( iAxis == XGE_XUI_ORIENTATION_HORIZONTAL ) {
		fSize = pPane->tRect.fH;
		fMin = pPane->pNode->fMinHeight;
	} else {
		fSize = pPane->tRect.fW;
		fMin = pPane->pNode->fMinWidth;
	}
	return (fSize + 0.5f) >= fMin;
}

int main(void)
{
	dock_smoke_host_t tHostState;
	xge_xui_host_t tHost;
	xge_xui_context_t tXui;
	xge_xui_dock_layout_t tLayout;
	xge_xui_dock_window_t tDoc;
	xge_xui_dock_window_t tOut;
	xge_xui_dock_window_t tTool;
	xge_xui_dock_window_t tAux;
	xge_xui_widget pRoot;
	xge_xui_widget pLayoutWidget;
	xge_xui_widget pDocContent;
	xge_xui_widget pOutContent;
	xge_xui_widget pToolContent;
	xge_xui_widget pAuxContent;
	xge_xui_widget pModalOverlay;
	xge_xui_widget pFocusBefore;
	xge_xui_dock_pane pDocPane;
	xge_xui_dock_pane pToolPane;
	xge_xui_dock_pane pAuxPane;
	xge_xui_dock_pane pDraggedPane;
	xge_xui_dock_node pSplit;
	xge_event_t tEvent;
	xge_rect_t tTabRect;
	xge_rect_t tFloatRect;
	xvalue pState;
	float fSplitRatio;
	int iPaintCount;
	int iPaneCount;
	int iSavedRegionCount;
	int iSavedWindowCount;
	int iSavedFloatingCount;
	int iRet;

	memset(&tHostState, 0, sizeof(tHostState));
	memset(&tHost, 0, sizeof(tHost));
	memset(&tXui, 0, sizeof(tXui));
	memset(&tLayout, 0, sizeof(tLayout));
	memset(&tDoc, 0, sizeof(tDoc));
	memset(&tOut, 0, sizeof(tOut));
	memset(&tTool, 0, sizeof(tTool));
	memset(&tAux, 0, sizeof(tAux));
	pLayoutWidget = NULL;
	pDocContent = NULL;
	pOutContent = NULL;
	pToolContent = NULL;
	pAuxContent = NULL;
	pModalOverlay = NULL;
	pState = NULL;
	iRet = 0;

	tHost.draw_rect = DockSmokeDrawRect;
	tHost.draw_image = DockSmokeDrawImage;
	tHost.draw_text_rect = DockSmokeDrawText;
	tHost.measure_text = DockSmokeMeasureText;
	tHost.clip_set = DockSmokeClipSet;
	tHost.clip_clear = DockSmokeClipClear;
	tHost.pUser = &tHostState;

	if ( xgeXuiBuiltinAssetGetCount() < 43 ) {
		return Fail(72, "builtin asset count");
	}
	if ( !CheckBuiltinAssetRect("dock_indicator_pane_diamond_left", 88.0f, 88.0f) || !CheckBuiltinAssetRect("dock_indicator_panel_left_active", 31.0f, 29.0f) ) {
		return Fail(73, "dock indicator asset rect");
	}
	if ( !CheckBuiltinAssetRect("dock_pane_close", 16.0f, 15.0f) || !CheckBuiltinAssetRect("dock_pane_auto_hide", 16.0f, 15.0f) || !CheckBuiltinAssetRect("dock_pane_dock", 16.0f, 15.0f) || !CheckBuiltinAssetRect("dock_pane_option", 16.0f, 15.0f) || !CheckBuiltinAssetRect("dock_pane_option_overflow", 16.0f, 15.0f) ) {
		return Fail(74, "dock pane button asset rect");
	}

	if ( xgeXuiInit(&tXui) != XGE_OK ) {
		return Fail(10, "xui init");
	}
	xgeXuiSetHost(&tXui, &tHost);
	pRoot = xgeXuiRoot(&tXui);
	pLayoutWidget = xgeXuiWidgetCreate();
	pDocContent = xgeXuiWidgetCreate();
	pOutContent = xgeXuiWidgetCreate();
	pToolContent = xgeXuiWidgetCreate();
	pAuxContent = xgeXuiWidgetCreate();
	if ( (pRoot == NULL) || (pLayoutWidget == NULL) || (pDocContent == NULL) || (pOutContent == NULL) || (pToolContent == NULL) || (pAuxContent == NULL) ) {
		iRet = Fail(11, "create widgets");
		goto cleanup;
	}
	xgeXuiWidgetSetFocusable(pDocContent, 1);
	xgeXuiWidgetSetFocusable(pOutContent, 1);
	xgeXuiWidgetSetFocusable(pToolContent, 1);
	xgeXuiWidgetSetFocusable(pAuxContent, 1);
	xgeXuiWidgetSetLayout(pRoot, XGE_XUI_LAYOUT_ABSOLUTE);
	xgeXuiWidgetSetRect(pRoot, (xge_rect_t){ 0.0f, 0.0f, 640.0f, 360.0f });
	xgeXuiWidgetSetRect(pLayoutWidget, (xge_rect_t){ 0.0f, 0.0f, 640.0f, 360.0f });
	xgeXuiWidgetAdd(pRoot, pLayoutWidget);
	if ( xgeXuiDockLayoutInit(&tLayout, &tXui, pLayoutWidget) != XGE_OK ) {
		iRet = Fail(12, "dock layout init");
		goto cleanup;
	}
	if ( tLayout.fTabStripHeight != 25.0f || tLayout.fCaptionHeight != 25.0f || tLayout.fButtonWidth != 16.0f || tLayout.fButtonHeight != 15.0f ) {
		iRet = Fail(143, "dock layout chrome metrics");
		goto cleanup;
	}
	if ( xgeXuiDockWindowInit(&tDoc, &tXui) != XGE_OK || xgeXuiDockWindowInit(&tOut, &tXui) != XGE_OK || xgeXuiDockWindowInit(&tTool, &tXui) != XGE_OK || xgeXuiDockWindowInit(&tAux, &tXui) != XGE_OK ) {
		iRet = Fail(13, "dock window init");
		goto cleanup;
	}
	xgeXuiDockWindowSetTitle(&tDoc, "Document");
	xgeXuiDockWindowSetTitle(&tOut, "Output");
	xgeXuiDockWindowSetTitle(&tTool, "Toolbox");
	xgeXuiDockWindowSetTitle(&tAux, "Aux");
	xgeXuiDockWindowSetClosable(&tDoc, 0);
	xgeXuiDockWindowSetClosable(&tOut, 1);
	xgeXuiDockWindowSetClosable(&tTool, 1);
	xgeXuiDockWindowSetClosable(&tAux, 1);
	xgeXuiDockWindowSetDockable(&tDoc, 1);
	xgeXuiDockWindowSetDockable(&tOut, 1);
	xgeXuiDockWindowSetDockable(&tTool, 1);
	xgeXuiDockWindowSetDockable(&tAux, 1);
	xgeXuiDockWindowSetClientWidget(&tDoc, pDocContent);
	xgeXuiDockWindowSetClientWidget(&tOut, pOutContent);
	xgeXuiDockWindowSetClientWidget(&tTool, pToolContent);
	xgeXuiDockWindowSetClientWidget(&tAux, pAuxContent);
	pDocPane = xgeXuiDockLayoutDockWindow(&tLayout, &tDoc, XGE_XUI_DOCK_REGION_DOCUMENT, XGE_XUI_DOCK_SIDE_FILL, 0.0f);
	if ( (pDocPane == NULL) || (xgeXuiDockPaneGetWindowCount(pDocPane) != 1) || (xgeXuiDockWindowGetState(&tDoc) != XGE_XUI_DOCK_WINDOW_DOCKED) || (tDoc.pClientWidget->pParent != pLayoutWidget) || (pDocContent->pParent != tDoc.pClientWidget) ) {
		iRet = Fail(14, "dock document");
		goto cleanup;
	}
	if ( xgeXuiDockLayoutDockWindow(&tLayout, &tOut, XGE_XUI_DOCK_REGION_DOCUMENT, XGE_XUI_DOCK_SIDE_FILL, 0.0f) != pDocPane ) {
		iRet = Fail(15, "tab fill");
		goto cleanup;
	}
	if ( (xgeXuiDockPaneGetWindowCount(pDocPane) != 2) || (xgeXuiDockPaneGetActiveWindow(pDocPane) != &tOut) ) {
		iRet = Fail(16, "tab active");
		goto cleanup;
	}
	xgeXuiDockPaneSetActiveIndex(pDocPane, 0);
	if ( xgeXuiUpdate(&tXui, 0.0f) != XGE_OK || pDocPane->tClientRect.fW <= 0.0f || pDocPane->tClientRect.fH <= 0.0f ) {
		iRet = Fail(17, "layout update");
		goto cleanup;
	}
	if ( tXui.pFocus != pDocContent ) {
		iRet = Fail(83, "focus document active");
		goto cleanup;
	}
	xgeXuiDockPaneSetActiveIndex(pDocPane, 1);
	if ( xgeXuiUpdate(&tXui, 0.0f) != XGE_OK || pDocPane->tCloseRect.fW <= 0.0f || xgeXuiDockPaneGetActiveWindow(pDocPane) != &tOut ) {
		iRet = Fail(61, "close button layout");
		goto cleanup;
	}
	if ( tXui.pFocus != pOutContent ) {
		iRet = Fail(84, "focus output active");
		goto cleanup;
	}
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_MOUSE_MOVE;
	tEvent.iPointerId = 12;
	tEvent.fX = pDocPane->tTabStripRect.fX + 96.0f;
	tEvent.fY = pDocPane->tTabStripRect.fY + 10.0f;
	(void)xgeXuiDispatchEvent(&tXui, &tEvent);
	if ( xgeXuiUpdate(&tXui, 0.50f) != XGE_OK || !xgeXuiWidgetTooltipIsOpen(&tXui) || xgeXuiWidgetTooltipGetOwner(&tXui) != pLayoutWidget || tXui.tActiveTooltip.sText == NULL || strcmp(tXui.tActiveTooltip.sText, "Output") != 0 ) {
		iRet = Fail(91, "tab tooltip");
		goto cleanup;
	}
	tEvent.fX = pDocPane->tCloseRect.fX + pDocPane->tCloseRect.fW * 0.50f;
	tEvent.fY = pDocPane->tCloseRect.fY + pDocPane->tCloseRect.fH * 0.50f;
	(void)xgeXuiDispatchEvent(&tXui, &tEvent);
	if ( xgeXuiUpdate(&tXui, 0.50f) != XGE_OK || !xgeXuiWidgetTooltipIsOpen(&tXui) || tXui.tActiveTooltip.sText == NULL || strcmp(tXui.tActiveTooltip.sText, "Close") != 0 ) {
		iRet = Fail(92, "close tooltip");
		goto cleanup;
	}
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_MOUSE_DOWN;
	tEvent.iParam1 = XGE_MOUSE_LEFT;
	tEvent.iPointerId = 10;
	tEvent.fX = pDocPane->tCloseRect.fX + pDocPane->tCloseRect.fW * 0.50f;
	tEvent.fY = pDocPane->tCloseRect.fY + pDocPane->tCloseRect.fH * 0.50f;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED ) {
		iRet = Fail(62, "close button down");
		goto cleanup;
	}
	tEvent.iType = XGE_EVENT_MOUSE_UP;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || xgeXuiDockWindowGetState(&tOut) != XGE_XUI_DOCK_WINDOW_HIDDEN || xgeXuiDockPaneGetWindowCount(pDocPane) != 1 || xgeXuiDockPaneGetActiveWindow(pDocPane) != &tDoc || tOut.pClientWidget->pParent != tOut.pWindowWidget ) {
		iRet = Fail(63, "close button active fallback");
		goto cleanup;
	}
	if ( xgeXuiUpdate(&tXui, 0.0f) != XGE_OK || pDocPane->tCloseRect.fW != 0.0f ) {
		iRet = Fail(64, "close button nonclosable hidden");
		goto cleanup;
	}
	if ( tXui.pFocus != pDocContent ) {
		iRet = Fail(85, "focus close fallback");
		goto cleanup;
	}
	if ( xgeXuiDockLayoutDockWindow(&tLayout, &tOut, XGE_XUI_DOCK_REGION_DOCUMENT, XGE_XUI_DOCK_SIDE_FILL, 0.0f) != pDocPane || xgeXuiDockWindowGetState(&tOut) != XGE_XUI_DOCK_WINDOW_DOCKED || xgeXuiDockPaneGetWindowCount(pDocPane) != 2 || xgeXuiDockPaneGetActiveWindow(pDocPane) != &tOut ) {
		iRet = Fail(65, "close button restore");
		goto cleanup;
	}
	if ( xgeXuiUpdate(&tXui, 0.0f) != XGE_OK || tOut.pClientWidget->pParent != pLayoutWidget || pOutContent->pParent != tOut.pClientWidget ) {
		iRet = Fail(66, "close button restore update");
		goto cleanup;
	}
	if ( tXui.pFocus != pOutContent ) {
		iRet = Fail(86, "focus restore output");
		goto cleanup;
	}
	if ( pDocPane->tAutoHideRect.fW <= 0.0f || pDocPane->tOptionRect.fW <= 0.0f ) {
		iRet = Fail(80, "disabled pane button layout");
		goto cleanup;
	}
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_MOUSE_DOWN;
	tEvent.iParam1 = XGE_MOUSE_LEFT;
	tEvent.iPointerId = 13;
	tEvent.fX = pDocPane->tAutoHideRect.fX + pDocPane->tAutoHideRect.fW * 0.50f;
	tEvent.fY = pDocPane->tAutoHideRect.fY + pDocPane->tAutoHideRect.fH * 0.50f;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) == XGE_XUI_EVENT_CONSUMED || xgeXuiGetPointerCapture(&tXui, tEvent.iPointerId) != NULL || tLayout.iDragPhase != XGE_XUI_DOCK_DRAG_IDLE ) {
		iRet = Fail(81, "disabled auto-hide hit");
		goto cleanup;
	}
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_MOUSE_DOWN;
	tEvent.iParam1 = XGE_MOUSE_LEFT;
	tEvent.iPointerId = 14;
	tEvent.fX = pDocPane->tOptionRect.fX + pDocPane->tOptionRect.fW * 0.50f;
	tEvent.fY = pDocPane->tOptionRect.fY + pDocPane->tOptionRect.fH * 0.50f;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || xgeXuiGetPointerCapture(&tXui, tEvent.iPointerId) != pLayoutWidget || tLayout.iDragPhase != XGE_XUI_DOCK_DRAG_IDLE ) {
		iRet = Fail(82, "option menu down");
		goto cleanup;
	}
	tEvent.iType = XGE_EVENT_MOUSE_UP;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || xgeXuiGetPointerCapture(&tXui, tEvent.iPointerId) != NULL || !xgeXuiMenuIsOpen(&tLayout.tOptionMenu) || tLayout.pOptionMenuPane != pDocPane ) {
		iRet = Fail(93, "option menu open");
		goto cleanup;
	}
	if ( tLayout.tOptionMenu.iItemCount != 6 || strcmp(tLayout.tOptionMenu.arrItems[0].sText, "Float") != 0 || strcmp(tLayout.tOptionMenu.arrItems[3].sText, "Close") != 0 || strcmp(tLayout.tOptionMenu.arrItems[5].sText, "Close All") != 0 || ((tLayout.tOptionMenu.arrItems[0].iState & XGE_XUI_MENU_ITEM_ENABLED) == 0) || ((tLayout.tOptionMenu.arrItems[3].iState & XGE_XUI_MENU_ITEM_ENABLED) == 0) ) {
		iRet = Fail(94, "option menu items");
		goto cleanup;
	}
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_KEY_DOWN;
	tEvent.iParam1 = XGE_KEY_ESCAPE;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || xgeXuiMenuIsOpen(&tLayout.tOptionMenu) ) {
		iRet = Fail(95, "option menu escape");
		goto cleanup;
	}
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_MOUSE_DOWN;
	tEvent.iParam1 = XGE_MOUSE_LEFT;
	tEvent.iPointerId = 15;
	tEvent.fX = pDocPane->tOptionRect.fX + pDocPane->tOptionRect.fW * 0.50f;
	tEvent.fY = pDocPane->tOptionRect.fY + pDocPane->tOptionRect.fH * 0.50f;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED ) {
		iRet = Fail(96, "option float down");
		goto cleanup;
	}
	tEvent.iType = XGE_EVENT_MOUSE_UP;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || !xgeXuiMenuIsOpen(&tLayout.tOptionMenu) ) {
		iRet = Fail(97, "option float open");
		goto cleanup;
	}
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_KEY_DOWN;
	tEvent.iParam1 = XGE_KEY_ENTER;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || xgeXuiMenuIsOpen(&tLayout.tOptionMenu) || xgeXuiDockWindowGetState(&tOut) != XGE_XUI_DOCK_WINDOW_FLOATING || pDocPane->arrWindows.Count != 1u || tLayout.arrFloatingWindows.Count != 1u ) {
		iRet = Fail(98, "option float commit");
		goto cleanup;
	}
	if ( xgeXuiDockLayoutDockWindow(&tLayout, &tOut, XGE_XUI_DOCK_REGION_DOCUMENT, XGE_XUI_DOCK_SIDE_FILL, 0.0f) != pDocPane || xgeXuiUpdate(&tXui, 0.0f) != XGE_OK || xgeXuiDockWindowGetState(&tOut) != XGE_XUI_DOCK_WINDOW_DOCKED || tLayout.arrFloatingWindows.Count != 0u || pDocPane->arrWindows.Count != 2u ) {
		iRet = Fail(99, "option float restore");
		goto cleanup;
	}
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_MOUSE_DOWN;
	tEvent.iParam1 = XGE_MOUSE_LEFT;
	tEvent.iPointerId = 16;
	tEvent.fX = pDocPane->tOptionRect.fX + pDocPane->tOptionRect.fW * 0.50f;
	tEvent.fY = pDocPane->tOptionRect.fY + pDocPane->tOptionRect.fH * 0.50f;
	(void)xgeXuiDispatchEvent(&tXui, &tEvent);
	tEvent.iType = XGE_EVENT_MOUSE_UP;
	(void)xgeXuiDispatchEvent(&tXui, &tEvent);
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_KEY_DOWN;
	tEvent.iParam1 = XGE_KEY_DOWN;
	(void)xgeXuiDispatchEvent(&tXui, &tEvent);
	(void)xgeXuiDispatchEvent(&tXui, &tEvent);
	tEvent.iParam1 = XGE_KEY_ENTER;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || xgeXuiDockWindowGetState(&tOut) != XGE_XUI_DOCK_WINDOW_HIDDEN || pDocPane->arrWindows.Count != 1u || xgeXuiDockPaneGetActiveWindow(pDocPane) != &tDoc ) {
		iRet = Fail(100, "option close all");
		goto cleanup;
	}
	if ( xgeXuiDockLayoutDockWindow(&tLayout, &tOut, XGE_XUI_DOCK_REGION_DOCUMENT, XGE_XUI_DOCK_SIDE_FILL, 0.0f) != pDocPane || xgeXuiUpdate(&tXui, 0.0f) != XGE_OK || xgeXuiDockWindowGetState(&tOut) != XGE_XUI_DOCK_WINDOW_DOCKED || pDocPane->arrWindows.Count != 2u ) {
		iRet = Fail(101, "option close all restore");
		goto cleanup;
	}
	xgeXuiWidgetSetRect(pRoot, (xge_rect_t){ 0.0f, 0.0f, 210.0f, 360.0f });
	xgeXuiWidgetSetRect(pLayoutWidget, (xge_rect_t){ 0.0f, 0.0f, 210.0f, 360.0f });
	xgeXuiDockPaneSetActiveIndex(pDocPane, 1);
	if ( xgeXuiUpdate(&tXui, 0.0f) != XGE_OK || pDocPane->tOverflowRect.fW > 0.0f ) {
		iRet = Fail(146, "compressed tabs before overflow");
		goto cleanup;
	}
	xgeXuiWidgetSetRect(pRoot, (xge_rect_t){ 0.0f, 0.0f, 190.0f, 360.0f });
	xgeXuiWidgetSetRect(pLayoutWidget, (xge_rect_t){ 0.0f, 0.0f, 190.0f, 360.0f });
	if ( xgeXuiUpdate(&tXui, 0.0f) != XGE_OK || pDocPane->tOverflowRect.fW <= 0.0f ) {
		iRet = Fail(102, "overflow layout");
		goto cleanup;
	}
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_MOUSE_DOWN;
	tEvent.iParam1 = XGE_MOUSE_LEFT;
	tEvent.iPointerId = 17;
	tEvent.fX = pDocPane->tOverflowRect.fX + pDocPane->tOverflowRect.fW * 0.50f;
	tEvent.fY = pDocPane->tOverflowRect.fY + pDocPane->tOverflowRect.fH * 0.50f;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED ) {
		iRet = Fail(103, "overflow down");
		goto cleanup;
	}
	tEvent.iType = XGE_EVENT_MOUSE_UP;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || !xgeXuiMenuIsOpen(&tLayout.tOverflowMenu) || tLayout.tOverflowMenu.iItemCount != 2 || strcmp(tLayout.tOverflowMenu.arrItems[0].sText, "Document") != 0 || strcmp(tLayout.tOverflowMenu.arrItems[1].sText, "Output") != 0 || ((tLayout.tOverflowMenu.arrItems[1].iState & XGE_XUI_MENU_ITEM_CHECKED) == 0) ) {
		iRet = Fail(104, "overflow open");
		goto cleanup;
	}
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_KEY_DOWN;
	tEvent.iParam1 = XGE_KEY_ENTER;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || xgeXuiMenuIsOpen(&tLayout.tOverflowMenu) || xgeXuiDockPaneGetActiveWindow(pDocPane) != &tDoc ) {
		iRet = Fail(105, "overflow select");
		goto cleanup;
	}
	xgeXuiWidgetSetRect(pRoot, (xge_rect_t){ 0.0f, 0.0f, 640.0f, 360.0f });
	xgeXuiWidgetSetRect(pLayoutWidget, (xge_rect_t){ 0.0f, 0.0f, 640.0f, 360.0f });
	xgeXuiDockPaneSetActiveIndex(pDocPane, 1);
	if ( xgeXuiUpdate(&tXui, 0.0f) != XGE_OK || pDocPane->tOverflowRect.fW > 0.0f || xgeXuiDockPaneGetActiveWindow(pDocPane) != &tOut ) {
		iRet = Fail(106, "overflow restore");
		goto cleanup;
	}
	xgeXuiDockWindowSetDockable(&tOut, 0);
	xgeXuiDockPaneSetActiveIndex(pDocPane, 1);
	if ( xgeXuiUpdate(&tXui, 0.0f) != XGE_OK || xgeXuiDockPaneGetActiveWindow(pDocPane) != &tOut ) {
		iRet = Fail(67, "nondockable active setup");
		goto cleanup;
	}
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_MOUSE_DOWN;
	tEvent.iParam1 = XGE_MOUSE_LEFT;
	tEvent.iPointerId = 11;
	tEvent.fX = pDocPane->tTabStripRect.fX + 90.0f;
	tEvent.fY = pDocPane->tTabStripRect.fY + 10.0f;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || tLayout.iDragPhase != XGE_XUI_DOCK_DRAG_IDLE || tLayout.pDragWindow != NULL ) {
		iRet = Fail(68, "nondockable drag down");
		goto cleanup;
	}
	tEvent.iType = XGE_EVENT_MOUSE_MOVE;
	tEvent.fX = -120.0f;
	tEvent.fY = 38.0f;
	(void)xgeXuiDispatchEvent(&tXui, &tEvent);
	if ( tLayout.iDragPhase != XGE_XUI_DOCK_DRAG_IDLE || tLayout.pDragWindow != NULL || tLayout.tPreviewRect.fW != 0.0f || tLayout.tIndicatorRect.fW != 0.0f || ((tLayout.pDragOverlayWidget != NULL) && (xgeXuiWidgetIsVisible(tLayout.pDragOverlayWidget) != 0)) ) {
		iRet = Fail(69, "nondockable drag ignored");
		goto cleanup;
	}
	tEvent.iType = XGE_EVENT_MOUSE_UP;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || xgeXuiGetPointerCapture(&tXui, tEvent.iPointerId) != NULL || tLayout.iDragPhase != XGE_XUI_DOCK_DRAG_IDLE || xgeXuiDockWindowGetState(&tOut) != XGE_XUI_DOCK_WINDOW_DOCKED || xgeXuiDockPaneGetWindowCount(pDocPane) != 2 ) {
		iRet = Fail(70, "nondockable drag release");
		goto cleanup;
	}
	xgeXuiDockWindowSetDockable(&tOut, 1);
	if ( xgeXuiUpdate(&tXui, 0.0f) != XGE_OK ) {
		iRet = Fail(71, "nondockable restore update");
		goto cleanup;
	}
	xgeXuiDockPaneSetActiveIndex(pDocPane, 1);
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_MOUSE_DOWN;
	tEvent.iParam1 = XGE_MOUSE_LEFT;
	tEvent.iPointerId = 18;
	tEvent.fX = pDocPane->tTabStripRect.fX + 90.0f;
	tEvent.fY = pDocPane->tTabStripRect.fY + 10.0f;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || tLayout.iDragPhase != XGE_XUI_DOCK_DRAG_PENDING ) {
		iRet = Fail(107, "ctrl suppress pending");
		goto cleanup;
	}
	tEvent.iType = XGE_EVENT_MOUSE_MOVE;
	tEvent.iParam2 = XGE_KEY_MOD_CTRL;
	tEvent.fX = pDocPane->tRect.fX + pDocPane->tRect.fW - 8.0f;
	tEvent.fY = pDocPane->tRect.fY + pDocPane->tRect.fH * 0.50f;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || tLayout.iDragPhase != XGE_XUI_DOCK_DRAG_DRAGGING || tLayout.pHoverPane != NULL || tLayout.pHoverRegion != NULL || tLayout.iHoverSide != XGE_XUI_DOCK_SIDE_NONE || tLayout.tIndicatorRect.fW != 0.0f || tLayout.tPreviewRect.fW <= 0.0f ) {
		iRet = Fail(108, "ctrl suppress hover");
		goto cleanup;
	}
	tEvent.iType = XGE_EVENT_MOUSE_UP;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || xgeXuiDockWindowGetState(&tOut) != XGE_XUI_DOCK_WINDOW_FLOATING || pDocPane->arrWindows.Count != 1u || tLayout.arrFloatingWindows.Count != 1u ) {
		iRet = Fail(109, "ctrl suppress float");
		goto cleanup;
	}
	if ( xgeXuiDockLayoutDockWindow(&tLayout, &tOut, XGE_XUI_DOCK_REGION_DOCUMENT, XGE_XUI_DOCK_SIDE_FILL, 0.0f) != pDocPane || xgeXuiUpdate(&tXui, 0.0f) != XGE_OK || xgeXuiDockWindowGetState(&tOut) != XGE_XUI_DOCK_WINDOW_DOCKED || pDocPane->arrWindows.Count != 2u ) {
		iRet = Fail(110, "ctrl suppress restore");
		goto cleanup;
	}
	pToolPane = xgeXuiDockLayoutDockWindow(&tLayout, &tTool, XGE_XUI_DOCK_REGION_DOCUMENT, XGE_XUI_DOCK_SIDE_RIGHT, 0.35f);
	pAuxPane = xgeXuiDockLayoutDockWindow(&tLayout, &tAux, XGE_XUI_DOCK_REGION_DOCUMENT, XGE_XUI_DOCK_SIDE_BOTTOM, 0.30f);
	if ( (pToolPane == NULL) || (pAuxPane == NULL) || (pToolPane == pDocPane) || (pAuxPane == pDocPane) || (pAuxPane == pToolPane) ) {
		iRet = Fail(111, "deep collapse setup");
		goto cleanup;
	}
	if ( xgeXuiUpdate(&tXui, 0.0f) != XGE_OK || CountDockPanes(tLayout.arrRegions[XGE_XUI_DOCK_REGION_DOCUMENT].pRoot) != 3 ) {
		iRet = Fail(112, "deep collapse setup update");
		goto cleanup;
	}
	if ( xgeXuiDockLayoutHideWindow(&tLayout, &tTool) != XGE_OK || xgeXuiUpdate(&tXui, 0.0f) != XGE_OK ) {
		iRet = Fail(113, "deep collapse hide nested");
		goto cleanup;
	}
	if ( CountDockPanes(tLayout.arrRegions[XGE_XUI_DOCK_REGION_DOCUMENT].pRoot) != 2 || tLayout.arrRegions[XGE_XUI_DOCK_REGION_DOCUMENT].pRoot == NULL || tLayout.arrRegions[XGE_XUI_DOCK_REGION_DOCUMENT].pRoot->iType != XGE_XUI_DOCK_NODE_SPLIT || tTool.pPane != NULL || xgeXuiDockWindowGetState(&tTool) != XGE_XUI_DOCK_WINDOW_HIDDEN || pDocPane->pNode == NULL || pAuxPane->pNode == NULL ) {
		iRet = Fail(114, "deep collapse nested state");
		goto cleanup;
	}
	pToolPane = NULL;
	if ( xgeXuiDockLayoutHideWindow(&tLayout, &tAux) != XGE_OK || xgeXuiUpdate(&tXui, 0.0f) != XGE_OK ) {
		iRet = Fail(115, "deep collapse hide root");
		goto cleanup;
	}
	if ( CountDockPanes(tLayout.arrRegions[XGE_XUI_DOCK_REGION_DOCUMENT].pRoot) != 1 || tLayout.arrRegions[XGE_XUI_DOCK_REGION_DOCUMENT].pRoot == NULL || tLayout.arrRegions[XGE_XUI_DOCK_REGION_DOCUMENT].pRoot->iType != XGE_XUI_DOCK_NODE_PANE || tLayout.arrRegions[XGE_XUI_DOCK_REGION_DOCUMENT].pRoot->pPane != pDocPane || tAux.pPane != NULL || xgeXuiDockWindowGetState(&tAux) != XGE_XUI_DOCK_WINDOW_HIDDEN || pDocPane->arrWindows.Count != 2u ) {
		iRet = Fail(116, "deep collapse root state");
		goto cleanup;
	}
	pAuxPane = NULL;
	xgeXuiDockPaneSetActiveIndex(pDocPane, 1);
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_MOUSE_DOWN;
	tEvent.iParam1 = XGE_MOUSE_LEFT;
	tEvent.iPointerId = 12;
	tEvent.fX = pDocPane->tTabStripRect.fX + 90.0f;
	tEvent.fY = pDocPane->tTabStripRect.fY + 10.0f;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || tLayout.iDragPhase != XGE_XUI_DOCK_DRAG_PENDING ) {
		iRet = Fail(75, "drop failure pending");
		goto cleanup;
	}
	tEvent.iType = XGE_EVENT_MOUSE_MOVE;
	tEvent.fX = pDocPane->tRect.fX + pDocPane->tRect.fW * 0.50f;
	tEvent.fY = pDocPane->tRect.fY + pDocPane->tRect.fH * 0.50f;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || tLayout.iDragPhase != XGE_XUI_DOCK_DRAG_DRAGGING || tLayout.pHoverPane != pDocPane || tLayout.iHoverSide != XGE_XUI_DOCK_SIDE_FILL || tLayout.tPreviewRect.fW <= 0.0f ) {
		iRet = Fail(76, "drop failure hover");
		goto cleanup;
	}
	xgeXuiDockWindowSetDockable(&tOut, 0);
	tEvent.iType = XGE_EVENT_MOUSE_UP;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || tLayout.iDragPhase != XGE_XUI_DOCK_DRAG_IDLE || xgeXuiGetPointerCapture(&tXui, tEvent.iPointerId) != NULL ) {
		iRet = Fail(77, "drop failure release");
		goto cleanup;
	}
	if ( xgeXuiDockWindowGetState(&tOut) != XGE_XUI_DOCK_WINDOW_DOCKED || tOut.pPane != pDocPane || xgeXuiDockPaneGetWindowCount(pDocPane) != 2 || tLayout.arrFloatingWindows.Count != 0u || tLayout.arrRegions[XGE_XUI_DOCK_REGION_DOCUMENT].pRoot->iType != XGE_XUI_DOCK_NODE_PANE ) {
		iRet = Fail(78, "drop failure rollback");
		goto cleanup;
	}
	xgeXuiDockWindowSetDockable(&tOut, 1);
	if ( xgeXuiUpdate(&tXui, 0.0f) != XGE_OK || xgeXuiDockPaneGetActiveWindow(pDocPane) != &tOut ) {
		iRet = Fail(79, "drop failure restore");
		goto cleanup;
	}
	xgeXuiDockPaneSetActiveIndex(pDocPane, 1);
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_MOUSE_DOWN;
	tEvent.iParam1 = XGE_MOUSE_LEFT;
	tEvent.iPointerId = 8;
	tEvent.fX = pDocPane->tTabStripRect.fX + 90.0f;
	tEvent.fY = pDocPane->tTabStripRect.fY + 10.0f;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || tLayout.iDragPhase != XGE_XUI_DOCK_DRAG_PENDING ) {
		iRet = Fail(52, "tab float pending");
		goto cleanup;
	}
	tEvent.iType = XGE_EVENT_MOUSE_MOVE;
	tEvent.fX = -120.0f;
	tEvent.fY = 38.0f;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || tLayout.iDragPhase != XGE_XUI_DOCK_DRAG_DRAGGING || tLayout.iHoverSide != XGE_XUI_DOCK_SIDE_NONE || tLayout.pHoverPane != NULL || tLayout.pHoverRegion != NULL || tLayout.tPreviewRect.fW <= 0.0f || tLayout.tIndicatorRect.fW != 0.0f ) {
		iRet = Fail(53, "tab float hover");
		goto cleanup;
	}
	tEvent.iType = XGE_EVENT_MOUSE_UP;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || tLayout.iDragPhase != XGE_XUI_DOCK_DRAG_IDLE || xgeXuiDockWindowGetState(&tOut) != XGE_XUI_DOCK_WINDOW_FLOATING || tLayout.arrFloatingWindows.Count != 1u || xgeXuiDockPaneGetWindowCount(pDocPane) != 1 || tOut.pClientWidget->pParent != tOut.pWindowWidget ) {
		iRet = Fail(54, "tab float commit");
		goto cleanup;
	}
	if ( xgeXuiDockLayoutDockWindow(&tLayout, &tOut, XGE_XUI_DOCK_REGION_DOCUMENT, XGE_XUI_DOCK_SIDE_FILL, 0.0f) != pDocPane || xgeXuiDockWindowGetState(&tOut) != XGE_XUI_DOCK_WINDOW_DOCKED || tLayout.arrFloatingWindows.Count != 0u || xgeXuiDockPaneGetWindowCount(pDocPane) != 2 ) {
		iRet = Fail(55, "tab float restore");
		goto cleanup;
	}
	if ( xgeXuiUpdate(&tXui, 0.0f) != XGE_OK ) {
		iRet = Fail(56, "tab float restore update");
		goto cleanup;
	}
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_MOUSE_DOWN;
	tEvent.iParam1 = XGE_MOUSE_LEFT;
	tEvent.iPointerId = 1;
	tEvent.fX = pDocPane->tTabStripRect.fX + 90.0f;
	tEvent.fY = pDocPane->tTabStripRect.fY + 10.0f;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED ) {
		iRet = Fail(18, "tab mouse down");
		goto cleanup;
	}
	tEvent.iType = XGE_EVENT_MOUSE_UP;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || xgeXuiDockPaneGetActiveWindow(pDocPane) != &tOut ) {
		iRet = Fail(19, "tab mouse up");
		goto cleanup;
	}
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_MOUSE_DOWN;
	tEvent.iParam1 = XGE_MOUSE_MIDDLE;
	tEvent.iPointerId = 19;
	tEvent.fX = pDocPane->tTabStripRect.fX + 90.0f;
	tEvent.fY = pDocPane->tTabStripRect.fY + 10.0f;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || tLayout.iDragPhase != XGE_XUI_DOCK_DRAG_IDLE || xgeXuiGetPointerCapture(&tXui, tEvent.iPointerId) != pLayoutWidget ) {
		iRet = Fail(117, "tab middle down");
		goto cleanup;
	}
	tEvent.iType = XGE_EVENT_MOUSE_UP;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || xgeXuiDockWindowGetState(&tOut) != XGE_XUI_DOCK_WINDOW_HIDDEN || pDocPane->arrWindows.Count != 1u || xgeXuiDockPaneGetActiveWindow(pDocPane) != &tDoc || xgeXuiGetPointerCapture(&tXui, tEvent.iPointerId) != NULL ) {
		iRet = Fail(118, "tab middle close");
		goto cleanup;
	}
	if ( xgeXuiDockLayoutDockWindow(&tLayout, &tOut, XGE_XUI_DOCK_REGION_DOCUMENT, XGE_XUI_DOCK_SIDE_FILL, 0.0f) != pDocPane || xgeXuiUpdate(&tXui, 0.0f) != XGE_OK || xgeXuiDockWindowGetState(&tOut) != XGE_XUI_DOCK_WINDOW_DOCKED || pDocPane->arrWindows.Count != 2u || xgeXuiDockPaneGetActiveWindow(pDocPane) != &tOut ) {
		iRet = Fail(119, "tab middle restore");
		goto cleanup;
	}
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_MOUSE_DOWN;
	tEvent.iParam1 = XGE_MOUSE_RIGHT;
	tEvent.iPointerId = 20;
	tEvent.fX = pDocPane->tTabStripRect.fX + 90.0f;
	tEvent.fY = pDocPane->tTabStripRect.fY + 10.0f;
	(void)xgeXuiDispatchEvent(&tXui, &tEvent);
	if ( tLayout.iDragPhase != XGE_XUI_DOCK_DRAG_IDLE || xgeXuiGetPointerCapture(&tXui, tEvent.iPointerId) == pLayoutWidget ) {
		iRet = Fail(120, "tab right no dock drag");
		goto cleanup;
	}
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_MOUSE_DOWN;
	tEvent.iParam1 = XGE_MOUSE_LEFT;
	tEvent.iPointerId = 6;
	tEvent.fX = pDocPane->tTabStripRect.fX + 90.0f;
	tEvent.fY = pDocPane->tTabStripRect.fY + 10.0f;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || tLayout.iDragPhase != XGE_XUI_DOCK_DRAG_PENDING ) {
		iRet = Fail(46, "tab reorder pending");
		goto cleanup;
	}
	tEvent.iType = XGE_EVENT_MOUSE_MOVE;
	tEvent.fX = pDocPane->tTabStripRect.fX + 8.0f;
	tEvent.fY = pDocPane->tTabStripRect.fY + 10.0f;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || tLayout.iDragPhase != XGE_XUI_DOCK_DRAG_DRAGGING || tLayout.pHoverPane != pDocPane || tLayout.iHoverTabIndex != 0 || tLayout.tPreviewRect.fW <= 0.0f || tLayout.tIndicatorRect.fW != 0.0f ) {
		iRet = Fail(47, "tab reorder hover");
		goto cleanup;
	}
	tEvent.iType = XGE_EVENT_MOUSE_UP;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || tLayout.iDragPhase != XGE_XUI_DOCK_DRAG_IDLE || xgeXuiDockPaneGetWindow(pDocPane, 0) != &tOut || xgeXuiDockPaneGetActiveWindow(pDocPane) != &tOut ) {
		iRet = Fail(48, "tab reorder commit");
		goto cleanup;
	}
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_MOUSE_DOWN;
	tEvent.iParam1 = XGE_MOUSE_LEFT;
	tEvent.iPointerId = 7;
	tEvent.fX = pDocPane->tTabStripRect.fX + 8.0f;
	tEvent.fY = pDocPane->tTabStripRect.fY + 10.0f;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || tLayout.iDragPhase != XGE_XUI_DOCK_DRAG_PENDING ) {
		iRet = Fail(49, "tab reorder restore pending");
		goto cleanup;
	}
	tEvent.iType = XGE_EVENT_MOUSE_MOVE;
	tEvent.fX = pDocPane->tTabStripRect.fX + 170.0f;
	tEvent.fY = pDocPane->tTabStripRect.fY + 10.0f;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || tLayout.iDragPhase != XGE_XUI_DOCK_DRAG_DRAGGING || tLayout.pHoverPane != pDocPane || tLayout.iHoverTabIndex != 2 ) {
		iRet = Fail(50, "tab reorder restore hover");
		goto cleanup;
	}
	tEvent.iType = XGE_EVENT_MOUSE_UP;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || tLayout.iDragPhase != XGE_XUI_DOCK_DRAG_IDLE || xgeXuiDockPaneGetWindow(pDocPane, 0) != &tDoc || xgeXuiDockPaneGetWindow(pDocPane, 1) != &tOut || xgeXuiDockPaneGetActiveWindow(pDocPane) != &tOut ) {
		iRet = Fail(51, "tab reorder restore commit");
		goto cleanup;
	}
	pToolPane = xgeXuiDockLayoutDockWindow(&tLayout, &tTool, XGE_XUI_DOCK_REGION_DOCUMENT, XGE_XUI_DOCK_SIDE_RIGHT, 0.35f);
	if ( (pToolPane == NULL) || (pToolPane == pDocPane) || (tLayout.arrRegions[XGE_XUI_DOCK_REGION_DOCUMENT].pRoot == NULL) || (tLayout.arrRegions[XGE_XUI_DOCK_REGION_DOCUMENT].pRoot->iType != XGE_XUI_DOCK_NODE_SPLIT) ) {
		iRet = Fail(20, "side split");
		goto cleanup;
	}
	if ( xgeXuiUpdate(&tXui, 0.0f) != XGE_OK || pToolPane->tClientRect.fW <= 0.0f || tTool.pClientWidget->pParent != pLayoutWidget || pToolContent->pParent != tTool.pClientWidget ) {
		iRet = Fail(21, "split update");
		goto cleanup;
	}
	if ( pToolPane->tAutoHideRect.fW <= 0.0f ) {
		iRet = Fail(127, "auto-hide button layout");
		goto cleanup;
	}
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_MOUSE_DOWN;
	tEvent.iParam1 = XGE_MOUSE_LEFT;
	tEvent.iPointerId = 21;
	tEvent.fX = pToolPane->tAutoHideRect.fX + pToolPane->tAutoHideRect.fW * 0.50f;
	tEvent.fY = pToolPane->tAutoHideRect.fY + pToolPane->tAutoHideRect.fH * 0.50f;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || xgeXuiGetPointerCapture(&tXui, tEvent.iPointerId) != pLayoutWidget ) {
		iRet = Fail(128, "auto-hide button down");
		goto cleanup;
	}
	tEvent.iType = XGE_EVENT_MOUSE_UP;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || xgeXuiDockWindowGetState(&tTool) != XGE_XUI_DOCK_WINDOW_AUTO_HIDE || tTool.pPane != NULL || tTool.pClientWidget->pParent != tTool.pWindowWidget ) {
		iRet = Fail(129, "auto-hide commit");
		goto cleanup;
	}
	if ( xgeXuiUpdate(&tXui, 0.0f) != XGE_OK || tTool.tAutoHideStripRect.fW <= 0.0f || tTool.tAutoHideStripRect.fH <= 0.0f ) {
		iRet = Fail(130, "auto-hide strip layout");
		goto cleanup;
	}
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_MOUSE_DOWN;
	tEvent.iParam1 = XGE_MOUSE_LEFT;
	tEvent.iPointerId = 22;
	tEvent.fX = tTool.tAutoHideStripRect.fX + tTool.tAutoHideStripRect.fW * 0.50f;
	tEvent.fY = tTool.tAutoHideStripRect.fY + tTool.tAutoHideStripRect.fH * 0.50f;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || xgeXuiGetPointerCapture(&tXui, tEvent.iPointerId) != pLayoutWidget ) {
		iRet = Fail(131, "auto-hide strip down");
		goto cleanup;
	}
	tEvent.iType = XGE_EVENT_MOUSE_UP;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || xgeXuiGetPointerCapture(&tXui, tEvent.iPointerId) != NULL || xgeXuiDockWindowGetState(&tTool) != XGE_XUI_DOCK_WINDOW_AUTO_HIDE || tTool.pPane != NULL || tLayout.pAutoHideExpandWindow != &tTool || tLayout.pAutoHideOverlayWidget == NULL || xgeXuiWidgetIsVisible(tLayout.pAutoHideOverlayWidget) == 0 || tTool.pClientWidget->pParent != tLayout.pAutoHideOverlayWidget || tLayout.tAutoHideExpandRect.fW <= 0.0f || tLayout.tAutoHideExpandDockRect.fW <= 0.0f || tLayout.tAutoHideExpandCloseRect.fW <= 0.0f ) {
		iRet = Fail(132, "auto-hide expand open");
		goto cleanup;
	}
	if ( xgeXuiUpdate(&tXui, 0.0f) != XGE_OK || tTool.pClientWidget->pParent != tLayout.pAutoHideOverlayWidget || pToolContent->pParent != tTool.pClientWidget || tLayout.tAutoHideExpandClientRect.fW <= 0.0f || tLayout.tAutoHideExpandClientRect.fH <= 0.0f ) {
		iRet = Fail(133, "auto-hide expand update");
		goto cleanup;
	}
	if ( tXui.pFocus != pToolContent ) {
		iRet = Fail(134, "auto-hide expand focus");
		goto cleanup;
	}
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_MOUSE_DOWN;
	tEvent.iParam1 = XGE_MOUSE_LEFT;
	tEvent.iPointerId = 23;
	tEvent.fX = tLayout.tAutoHideExpandCloseRect.fX + tLayout.tAutoHideExpandCloseRect.fW * 0.50f;
	tEvent.fY = tLayout.tAutoHideExpandCloseRect.fY + tLayout.tAutoHideExpandCloseRect.fH * 0.50f;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || xgeXuiGetPointerCapture(&tXui, tEvent.iPointerId) != tLayout.pAutoHideOverlayWidget ) {
		iRet = Fail(135, "auto-hide expand close down");
		goto cleanup;
	}
	tEvent.iType = XGE_EVENT_MOUSE_UP;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || xgeXuiGetPointerCapture(&tXui, tEvent.iPointerId) != NULL || tLayout.pAutoHideExpandWindow != NULL || xgeXuiWidgetIsVisible(tLayout.pAutoHideOverlayWidget) != 0 || xgeXuiDockWindowGetState(&tTool) != XGE_XUI_DOCK_WINDOW_AUTO_HIDE || tTool.pClientWidget->pParent != tTool.pWindowWidget ) {
		iRet = Fail(136, "auto-hide expand close");
		goto cleanup;
	}
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_MOUSE_DOWN;
	tEvent.iParam1 = XGE_MOUSE_LEFT;
	tEvent.iPointerId = 24;
	tEvent.fX = tTool.tAutoHideStripRect.fX + tTool.tAutoHideStripRect.fW * 0.50f;
	tEvent.fY = tTool.tAutoHideStripRect.fY + tTool.tAutoHideStripRect.fH * 0.50f;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED ) {
		iRet = Fail(137, "auto-hide strip reopen down");
		goto cleanup;
	}
	tEvent.iType = XGE_EVENT_MOUSE_UP;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || tLayout.pAutoHideExpandWindow != &tTool || tTool.pClientWidget->pParent != tLayout.pAutoHideOverlayWidget ) {
		iRet = Fail(138, "auto-hide strip reopen");
		goto cleanup;
	}
	tEvent.iType = XGE_EVENT_MOUSE_DOWN;
	tEvent.fX = tLayout.tAutoHideExpandDockRect.fX + tLayout.tAutoHideExpandDockRect.fW * 0.50f;
	tEvent.fY = tLayout.tAutoHideExpandDockRect.fY + tLayout.tAutoHideExpandDockRect.fH * 0.50f;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || xgeXuiGetPointerCapture(&tXui, tEvent.iPointerId) != tLayout.pAutoHideOverlayWidget ) {
		iRet = Fail(139, "auto-hide expand dock down");
		goto cleanup;
	}
	tEvent.iType = XGE_EVENT_MOUSE_UP;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || xgeXuiDockWindowGetState(&tTool) != XGE_XUI_DOCK_WINDOW_DOCKED || tTool.pPane == NULL || tTool.pClientWidget->pParent != pLayoutWidget || tLayout.pAutoHideExpandWindow != NULL || xgeXuiWidgetIsVisible(tLayout.pAutoHideOverlayWidget) != 0 ) {
		iRet = Fail(140, "auto-hide dock restore");
		goto cleanup;
	}
	pToolPane = tTool.pPane;
	if ( xgeXuiUpdate(&tXui, 0.0f) != XGE_OK || pToolPane->tClientRect.fW <= 0.0f || pToolPane->tAutoHideRect.fW <= 0.0f ) {
		iRet = Fail(141, "auto-hide restore update");
		goto cleanup;
	}
	pSplit = tLayout.arrRegions[XGE_XUI_DOCK_REGION_DOCUMENT].pRoot;
	fSplitRatio = pSplit->fRatio;
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_MOUSE_DOWN;
	tEvent.iParam1 = XGE_MOUSE_LEFT;
	tEvent.iPointerId = 9;
	tEvent.fX = pSplit->tSplitterRect.fX + pSplit->tSplitterRect.fW * 0.50f;
	tEvent.fY = pSplit->tSplitterRect.fY + pSplit->tSplitterRect.fH * 0.50f;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || tLayout.pSplitterDragNode != pSplit || xgeXuiGetPointerCapture(&tXui, tEvent.iPointerId) != pLayoutWidget ) {
		iRet = Fail(57, "splitter drag begin");
		goto cleanup;
	}
	tEvent.iType = XGE_EVENT_MOUSE_MOVE;
	if ( pSplit->iAxis == XGE_XUI_ORIENTATION_HORIZONTAL ) {
		tEvent.fY -= 42.0f;
	} else {
		tEvent.fX -= 42.0f;
	}
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || pSplit->fRatio == fSplitRatio ) {
		iRet = Fail(58, "splitter drag ratio");
		goto cleanup;
	}
	tEvent.iType = XGE_EVENT_MOUSE_UP;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || tLayout.pSplitterDragNode != NULL || xgeXuiGetPointerCapture(&tXui, tEvent.iPointerId) != NULL ) {
		iRet = Fail(59, "splitter drag end");
		goto cleanup;
	}
	if ( xgeXuiUpdate(&tXui, 0.0f) != XGE_OK || pDocPane->tClientRect.fW <= 0.0f || pToolPane->tClientRect.fW <= 0.0f ) {
		iRet = Fail(60, "splitter drag update");
		goto cleanup;
	}
	pSplit->fRatio = 0.01f;
	xgeXuiWidgetMarkLayout(pLayoutWidget);
	if ( xgeXuiUpdate(&tXui, 0.0f) != XGE_OK || !PaneSatisfiesAxisMin(pDocPane, pSplit->iAxis) || !PaneSatisfiesAxisMin(pToolPane, pSplit->iAxis) || pSplit->fRatio <= 0.01f ) {
		iRet = Fail(87, "splitter min clamp low");
		goto cleanup;
	}
	pSplit->fRatio = 0.99f;
	xgeXuiWidgetMarkLayout(pLayoutWidget);
	if ( xgeXuiUpdate(&tXui, 0.0f) != XGE_OK || !PaneSatisfiesAxisMin(pDocPane, pSplit->iAxis) || !PaneSatisfiesAxisMin(pToolPane, pSplit->iAxis) || pSplit->fRatio >= 0.99f ) {
		iRet = Fail(88, "splitter min clamp high");
		goto cleanup;
	}
	pSplit->fRatio = 0.65f;
	xgeXuiWidgetMarkLayout(pLayoutWidget);
	if ( xgeXuiUpdate(&tXui, 0.0f) != XGE_OK || !PaneSatisfiesAxisMin(pDocPane, pSplit->iAxis) || !PaneSatisfiesAxisMin(pToolPane, pSplit->iAxis) ) {
		iRet = Fail(89, "splitter min restore");
		goto cleanup;
	}
	xgeXuiDockPaneSetActiveIndex(pDocPane, 0);
	tTabRect = (xge_rect_t){ pDocPane->tTabStripRect.fX + 8.0f, pDocPane->tTabStripRect.fY + 8.0f, 1.0f, 1.0f };
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_MOUSE_DOWN;
	tEvent.iParam1 = XGE_MOUSE_LEFT;
	tEvent.iPointerId = 2;
	tEvent.fX = tTabRect.fX;
	tEvent.fY = tTabRect.fY;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || tLayout.iDragPhase != XGE_XUI_DOCK_DRAG_PENDING ) {
		iRet = Fail(27, "drag pending");
		goto cleanup;
	}
	tEvent.iType = XGE_EVENT_MOUSE_MOVE;
	tEvent.fX = pDocPane->tRect.fX + pDocPane->tRect.fW - 8.0f;
	tEvent.fY = pDocPane->tRect.fY + pDocPane->tRect.fH * 0.50f;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || tLayout.iDragPhase != XGE_XUI_DOCK_DRAG_DRAGGING || tLayout.pDragOverlayWidget == NULL || xgeXuiWidgetIsVisible(tLayout.pDragOverlayWidget) == 0 || tLayout.iHoverSide != XGE_XUI_DOCK_SIDE_RIGHT || tLayout.tPreviewRect.fW <= 0.0f ) {
		iRet = Fail(28, "drag preview");
		goto cleanup;
	}
	tEvent.iType = XGE_EVENT_MOUSE_UP;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || tLayout.iDragPhase != XGE_XUI_DOCK_DRAG_IDLE || xgeXuiWidgetIsVisible(tLayout.pDragOverlayWidget) != 0 ) {
		iRet = Fail(29, "drag commit");
		goto cleanup;
	}
	pDraggedPane = tDoc.pPane;
	if ( (pDraggedPane == NULL) || (pDraggedPane == pDocPane) || (xgeXuiDockWindowGetState(&tDoc) != XGE_XUI_DOCK_WINDOW_DOCKED) || (tDoc.pClientWidget->pParent != pLayoutWidget) || (pDocPane->arrWindows.Count != 1u) ) {
		iRet = Fail(30, "drag dock state");
		goto cleanup;
	}
	iPaneCount = CountDockPanes(tLayout.arrRegions[XGE_XUI_DOCK_REGION_DOCUMENT].pRoot);
	if ( iPaneCount != 3 ) {
		iRet = Fail(31, "pane split count");
		goto cleanup;
	}
	memset(&tHostState, 0, sizeof(tHostState));
	iPaintCount = xgeXuiPaint(&tXui);
	if ( (iPaintCount <= 0) || (tHostState.iDrawRect <= 0) || (tHostState.iDrawText <= 0) ) {
		iRet = Fail(22, "paint");
		goto cleanup;
	}
	tFloatRect = (xge_rect_t){ 20.0f, 30.0f, 180.0f, 120.0f };
	if ( xgeXuiDockLayoutFloatWindow(&tLayout, &tDoc, tFloatRect) != XGE_OK ) {
		iRet = Fail(23, "float");
		goto cleanup;
	}
	if ( (xgeXuiDockWindowGetState(&tDoc) != XGE_XUI_DOCK_WINDOW_FLOATING) || (xgeXuiWindowIsOpen(xgeXuiDockWindowBaseWindow(&tDoc)) == 0) || (pDocPane->arrWindows.Count != 1u) || (tLayout.arrFloatingWindows.Count != 1u) || (tDoc.pClientWidget->pParent != tDoc.pWindowWidget) || (pDocContent->pParent != tDoc.pClientWidget) ) {
		iRet = Fail(24, "float state");
		goto cleanup;
	}
	if ( xgeXuiUpdate(&tXui, 0.0f) != XGE_OK ) {
		iRet = Fail(32, "float update");
		goto cleanup;
	}
	iPaneCount = CountDockPanes(tLayout.arrRegions[XGE_XUI_DOCK_REGION_DOCUMENT].pRoot);
	if ( iPaneCount != 2 ) {
		iRet = Fail(33, "source pane collapse");
		goto cleanup;
	}
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_MOUSE_DOWN;
	tEvent.iParam1 = XGE_MOUSE_LEFT;
	tEvent.iPointerId = 3;
	tEvent.fX = tDoc.pWindowWidget->tRect.fX + 42.0f;
	tEvent.fY = tDoc.pWindowWidget->tRect.fY + 10.0f;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || tLayout.iDragPhase != XGE_XUI_DOCK_DRAG_PENDING || tLayout.pDragSourcePane != NULL ) {
		iRet = Fail(34, "floating drag pending");
		goto cleanup;
	}
	tEvent.iType = XGE_EVENT_MOUSE_MOVE;
	tEvent.fX = pToolPane->tRect.fX + pToolPane->tRect.fW * 0.50f;
	tEvent.fY = pToolPane->tRect.fY + pToolPane->tRect.fH * 0.50f;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || tLayout.iDragPhase != XGE_XUI_DOCK_DRAG_DRAGGING || tLayout.pHoverPane != pToolPane || tLayout.iHoverSide != XGE_XUI_DOCK_SIDE_FILL ) {
		iRet = Fail(35, "floating drag hover");
		goto cleanup;
	}
	tEvent.iType = XGE_EVENT_MOUSE_UP;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || tLayout.iDragPhase != XGE_XUI_DOCK_DRAG_IDLE || tLayout.arrFloatingWindows.Count != 0u ) {
		iRet = Fail(36, "floating drag commit");
		goto cleanup;
	}
	if ( (xgeXuiDockWindowGetState(&tDoc) != XGE_XUI_DOCK_WINDOW_DOCKED) || (tDoc.pPane != pToolPane) || (pToolPane->arrWindows.Count != 2u) || (xgeXuiWindowIsOpen(xgeXuiDockWindowBaseWindow(&tDoc)) != 0) || (tDoc.pClientWidget->pParent != pLayoutWidget) ) {
		iRet = Fail(37, "floating dock state");
		goto cleanup;
	}
	tTabRect = pToolPane->tTabStripRect;
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_MOUSE_DOWN;
	tEvent.iParam1 = XGE_MOUSE_LEFT;
	tEvent.iPointerId = 4;
	tEvent.fX = tTabRect.fX + 8.0f;
	tEvent.fY = tTabRect.fY + 8.0f;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || tLayout.iDragPhase != XGE_XUI_DOCK_DRAG_PENDING ) {
		iRet = Fail(38, "cancel drag pending");
		goto cleanup;
	}
	tEvent.iType = XGE_EVENT_MOUSE_MOVE;
	tEvent.fX = pToolPane->tRect.fX + pToolPane->tRect.fW - 8.0f;
	tEvent.fY = pToolPane->tRect.fY + pToolPane->tRect.fH * 0.50f;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || tLayout.iDragPhase != XGE_XUI_DOCK_DRAG_DRAGGING ) {
		iRet = Fail(39, "cancel drag active");
		goto cleanup;
	}
	pFocusBefore = tXui.pFocus;
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_KEY_DOWN;
	tEvent.iParam1 = XGE_KEY_TAB;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || tLayout.iDragPhase != XGE_XUI_DOCK_DRAG_DRAGGING || tXui.pFocus != pFocusBefore ) {
		iRet = Fail(90, "tab consumed during drag");
		goto cleanup;
	}
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_KEY_DOWN;
	tEvent.iParam1 = XGE_KEY_ESCAPE;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || tLayout.iDragPhase != XGE_XUI_DOCK_DRAG_IDLE || xgeXuiGetPointerCapture(&tXui, 4) != NULL || tDoc.pPane != pToolPane || tLayout.arrFloatingWindows.Count != 0u ) {
		iRet = Fail(40, "escape cancel drag");
		goto cleanup;
	}
	tFloatRect = (xge_rect_t){ 30.0f, 40.0f, 180.0f, 120.0f };
	if ( xgeXuiDockLayoutFloatWindow(&tLayout, &tDoc, tFloatRect) != XGE_OK || tLayout.arrFloatingWindows.Count != 1u || xgeXuiDockWindowGetState(&tDoc) != XGE_XUI_DOCK_WINDOW_FLOATING ) {
		iRet = Fail(41, "global region float");
		goto cleanup;
	}
	pState = xgeXuiDockLayoutSaveState(&tLayout);
	if ( pState == NULL ) {
		iRet = Fail(121, "save state create");
		goto cleanup;
	}
	iSavedRegionCount = 0;
	iSavedWindowCount = 0;
	iSavedFloatingCount = 0;
	if ( xgeXuiDockLayoutStateGetCounts(pState, &iSavedRegionCount, &iSavedWindowCount, &iSavedFloatingCount) != XGE_OK ||
	     iSavedRegionCount != XGE_XUI_DOCK_REGION_COUNT || iSavedWindowCount < 3 || iSavedFloatingCount != 1 ) {
		iRet = Fail(122, "save state arrays");
		goto cleanup;
	}
	xgeXuiWidgetSetName(tDoc.pWindowWidget, "duplicate-dock-id");
	xgeXuiWidgetSetName(tOut.pWindowWidget, "duplicate-dock-id");
	if ( xgeXuiDockLayoutLoadState(&tLayout, pState) == XGE_OK || xgeXuiDockWindowGetState(&tDoc) != XGE_XUI_DOCK_WINDOW_FLOATING || tLayout.arrFloatingWindows.Count != 1u ) {
		iRet = Fail(142, "load duplicate id rejected");
		goto cleanup;
	}
	xgeXuiWidgetSetName(tDoc.pWindowWidget, "");
	xgeXuiWidgetSetName(tOut.pWindowWidget, "");
	if ( xgeXuiDockLayoutLoadState(&tLayout, NULL) == XGE_OK || xgeXuiDockWindowGetState(&tDoc) != XGE_XUI_DOCK_WINDOW_FLOATING || tLayout.arrFloatingWindows.Count != 1u ) {
		iRet = Fail(123, "load invalid rollback");
		goto cleanup;
	}
	if ( xgeXuiDockLayoutDockWindow(&tLayout, &tDoc, XGE_XUI_DOCK_REGION_DOCUMENT, XGE_XUI_DOCK_SIDE_FILL, 0.0f) == NULL || xgeXuiDockLayoutHideWindow(&tLayout, &tOut) != XGE_OK || xgeXuiDockWindowGetState(&tDoc) != XGE_XUI_DOCK_WINDOW_DOCKED || xgeXuiDockWindowGetState(&tOut) != XGE_XUI_DOCK_WINDOW_HIDDEN ) {
		iRet = Fail(124, "roundtrip mutate");
		goto cleanup;
	}
	if ( xgeXuiDockLayoutLoadState(&tLayout, pState) != XGE_OK ) {
		iRet = Fail(125, "load state roundtrip");
		goto cleanup;
	}
	pDocPane = tOut.pPane;
	pToolPane = tTool.pPane;
	if ( xgeXuiDockWindowGetState(&tDoc) != XGE_XUI_DOCK_WINDOW_FLOATING || tDoc.pPane != NULL || tLayout.arrFloatingWindows.Count != 1u || xgeXuiDockWindowGetState(&tOut) != XGE_XUI_DOCK_WINDOW_DOCKED || xgeXuiDockWindowGetState(&tTool) != XGE_XUI_DOCK_WINDOW_DOCKED || pDocPane == NULL || pToolPane == NULL || tDoc.pClientWidget->pParent != tDoc.pWindowWidget || tOut.pClientWidget->pParent != pLayoutWidget || tTool.pClientWidget->pParent != pLayoutWidget ) {
		iRet = Fail(126, "load state restored");
		goto cleanup;
	}
	xgeXuiDockLayoutStateFree(pState);
	pState = NULL;
	if ( xgeXuiUpdate(&tXui, 0.0f) != XGE_OK ) {
		iRet = Fail(42, "global region float update");
		goto cleanup;
	}
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_MOUSE_DOWN;
	tEvent.iParam1 = XGE_MOUSE_LEFT;
	tEvent.iPointerId = 5;
	tEvent.fX = tDoc.pWindowWidget->tRect.fX + 42.0f;
	tEvent.fY = tDoc.pWindowWidget->tRect.fY + 10.0f;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || tLayout.iDragPhase != XGE_XUI_DOCK_DRAG_PENDING || tLayout.pDragSourcePane != NULL ) {
		iRet = Fail(43, "global region drag pending");
		goto cleanup;
	}
	tEvent.iType = XGE_EVENT_MOUSE_MOVE;
	tEvent.fX = pLayoutWidget->tContentRect.fX + 8.0f;
	tEvent.fY = pLayoutWidget->tContentRect.fY + pLayoutWidget->tContentRect.fH * 0.50f;
	pModalOverlay = xgeXuiWidgetCreate();
	if ( pModalOverlay == NULL ) {
		iRet = Fail(145, "modal overlay create");
		goto cleanup;
	}
	xgeXuiWidgetSetLayout(pModalOverlay, XGE_XUI_LAYOUT_ABSOLUTE);
	xgeXuiWidgetSetRect(pModalOverlay, tXui.pRoot->tRect);
	xgeXuiWidgetSetBackground(pModalOverlay, XGE_COLOR_RGBA(0, 0, 0, 0));
	if ( xgeXuiOverlayAttach(&tXui, pModalOverlay, NULL, XGE_XUI_LAYER_MODAL) != XGE_OK ) {
		iRet = Fail(145, "modal overlay attach");
		goto cleanup;
	}
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || tLayout.iDragPhase != XGE_XUI_DOCK_DRAG_DRAGGING || tLayout.pHoverPane != NULL || tLayout.pHoverRegion != NULL || tLayout.iHoverSide != XGE_XUI_DOCK_SIDE_NONE || tLayout.tIndicatorRect.fW != 0.0f || tLayout.tPreviewRect.fW <= 0.0f ) {
		iRet = Fail(145, "modal suppress docking");
		goto cleanup;
	}
	xgeXuiOverlayDetach(&tXui, pModalOverlay);
	xgeXuiWidgetFree(pModalOverlay);
	pModalOverlay = NULL;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || tLayout.iDragPhase != XGE_XUI_DOCK_DRAG_DRAGGING || tLayout.pHoverRegion != &tLayout.arrRegions[XGE_XUI_DOCK_REGION_LEFT] || tLayout.iHoverSide != XGE_XUI_DOCK_SIDE_LEFT || tLayout.tPreviewRect.fW <= 0.0f || tLayout.tIndicatorRect.fW <= 0.0f ) {
		iRet = Fail(44, "global region hover");
		goto cleanup;
	}
	if ( tLayout.pDragOverlayWidget == NULL || xgeXuiWidgetGetLayer(tLayout.pDragOverlayWidget) != XGE_XUI_LAYER_DRAG_ADORNER || xgeXuiWidgetGetTreeOrder(tLayout.pDragOverlayWidget) <= xgeXuiWidgetGetTreeOrder(tDoc.pWindowWidget) ) {
		iRet = Fail(144, "drag overlay z order");
		goto cleanup;
	}
	tEvent.iType = XGE_EVENT_MOUSE_UP;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || tLayout.iDragPhase != XGE_XUI_DOCK_DRAG_IDLE || tLayout.arrFloatingWindows.Count != 0u || xgeXuiDockWindowGetState(&tDoc) != XGE_XUI_DOCK_WINDOW_DOCKED || tLayout.arrRegions[XGE_XUI_DOCK_REGION_LEFT].pRoot == NULL || tLayout.arrRegions[XGE_XUI_DOCK_REGION_LEFT].pRoot->pPane != tDoc.pPane || tDoc.pClientWidget->pParent != pLayoutWidget ) {
		iRet = Fail(45, "global region commit");
		goto cleanup;
	}
	if ( xgeXuiDockLayoutHideWindow(&tLayout, &tDoc) != XGE_OK ) {
		iRet = Fail(25, "hide");
		goto cleanup;
	}
	if ( (xgeXuiDockWindowGetState(&tDoc) != XGE_XUI_DOCK_WINDOW_HIDDEN) || (xgeXuiWindowIsOpen(xgeXuiDockWindowBaseWindow(&tDoc)) != 0) || (tLayout.arrFloatingWindows.Count != 0u) ) {
		iRet = Fail(26, "hide state");
		goto cleanup;
	}
	printf("xui_dockpanel_smoke passed tabs=%u drawRect=%d drawText=%d drawImage=%d\n", (unsigned)xgeXuiDockPaneGetWindowCount(pDocPane), tHostState.iDrawRect, tHostState.iDrawText, tHostState.iDrawImage);

cleanup:
	if ( pModalOverlay != NULL ) {
		xgeXuiOverlayDetach(&tXui, pModalOverlay);
		xgeXuiWidgetFree(pModalOverlay);
		pModalOverlay = NULL;
	}
	if ( pState != NULL ) {
		xgeXuiDockLayoutStateFree(pState);
	}
	xgeXuiDockWindowUnit(&tAux);
	xgeXuiDockWindowUnit(&tTool);
	xgeXuiDockWindowUnit(&tOut);
	xgeXuiDockWindowUnit(&tDoc);
	if ( pAuxContent != NULL ) {
		xgeXuiWidgetFree(pAuxContent);
	}
	if ( pToolContent != NULL ) {
		xgeXuiWidgetFree(pToolContent);
	}
	if ( pOutContent != NULL ) {
		xgeXuiWidgetFree(pOutContent);
	}
	if ( pDocContent != NULL ) {
		xgeXuiWidgetFree(pDocContent);
	}
	xgeXuiDockLayoutUnit(&tLayout);
	if ( pLayoutWidget != NULL ) {
		xgeXuiWidgetFree(pLayoutWidget);
	}
	xgeXuiUnit(&tXui);
	return iRet;
}
