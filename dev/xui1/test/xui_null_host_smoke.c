#include "../xge.h"
#include <stdio.h>
#include <string.h>

static int g_iClicked = 0;

static const xge_xui_menu_item_t g_arrMenuItems[] = {
	{ "Smoke", "", XGE_XUI_MENU_ITEM_NORMAL, XGE_XUI_MENU_ITEM_ENABLED, 9001, 0, NULL, NULL }
};

static void OnClick(xge_xui_widget pWidget, void* pUser)
{
	(void)pWidget;
	(void)pUser;
	g_iClicked++;
}

static void MakeMouse(xge_event_t* pEvent, int iType, float fX, float fY)
{
	memset(pEvent, 0, sizeof(*pEvent));
	pEvent->iType = iType;
	pEvent->iParam1 = XGE_MOUSE_LEFT;
	pEvent->fX = fX;
	pEvent->fY = fY;
}

int main(void)
{
	xge_xui_context_t tXui;
	xge_xui_button_t tButton;
	xge_xui_label_t tLabel;
	xge_xui_msg_tip_t tTip;
	xge_xui_dock_layout_t tDockLayout;
	xge_xui_dock_window_t tDockWindow;
	xge_xui_menu_t tMenu;
	xge_xui_widget pRoot;
	xge_xui_widget pButtonWidget;
	xge_xui_widget pLabelWidget;
	xge_xui_widget pTipWidget;
	xge_xui_widget pDockWidget;
	xge_xui_widget pDockContent;
	xge_xui_widget pPixelRow;
	xge_xui_widget pPixelChildA;
	xge_xui_widget pPixelChildB;
	xge_xui_widget pPixelChildC;
	xge_xui_dock_pane pDockPane;
	xge_event_t tEvent;
	xge_rect_t tSnappedLabel;
	xge_rect_t tPixelA;
	xge_rect_t tPixelB;
	xge_rect_t tPixelC;
	xui_texture pTexture;
	int iRet;
	int bOk;
	int bMenuOwnerUpOK;
	int bMenuOwnerDownCloseOK;
	int bMenuHScrollHiddenOK;

	memset(&tXui, 0, sizeof(tXui));
	memset(&tButton, 0, sizeof(tButton));
	memset(&tLabel, 0, sizeof(tLabel));
	memset(&tTip, 0, sizeof(tTip));
	memset(&tDockLayout, 0, sizeof(tDockLayout));
	memset(&tDockWindow, 0, sizeof(tDockWindow));
	memset(&tMenu, 0, sizeof(tMenu));

	if ( xgeXuiInit(&tXui) != XGE_OK ) {
		printf("xui_null_host_smoke failed: xui init\n");
		return 1;
	}
	xgeXuiSetHostV2(&tXui, xgeXuiHostV2Null());
	pRoot = xgeXuiRoot(&tXui);
	pButtonWidget = xgeXuiWidgetCreate();
	pLabelWidget = xgeXuiWidgetCreate();
	pTipWidget = xgeXuiWidgetCreate();
	pDockWidget = xgeXuiWidgetCreate();
	pDockContent = xgeXuiWidgetCreate();
	pPixelRow = xgeXuiWidgetCreate();
	pPixelChildA = xgeXuiWidgetCreate();
	pPixelChildB = xgeXuiWidgetCreate();
	pPixelChildC = xgeXuiWidgetCreate();
	pDockPane = NULL;
	bMenuOwnerUpOK = 0;
	bMenuOwnerDownCloseOK = 0;
	bMenuHScrollHiddenOK = 0;
	if ( (pRoot == NULL) || (pButtonWidget == NULL) || (pLabelWidget == NULL) || (pTipWidget == NULL) || (pDockWidget == NULL) || (pDockContent == NULL) || (pPixelRow == NULL) || (pPixelChildA == NULL) || (pPixelChildB == NULL) || (pPixelChildC == NULL) ) {
		printf("xui_null_host_smoke failed: widget create\n");
		xgeXuiUnit(&tXui);
		return 1;
	}
	xgeXuiWidgetSetLayout(pRoot, XGE_XUI_LAYOUT_ABSOLUTE);
	xgeXuiWidgetSetRect(pRoot, (xge_rect_t){ 0.0f, 0.0f, 320.0f, 180.0f });
	xgeXuiWidgetSetRect(pButtonWidget, (xge_rect_t){ 12.0f, 12.0f, 96.0f, 28.0f });
	xgeXuiWidgetSetRect(pLabelWidget, (xge_rect_t){ 12.5f, 48.5f, 220.2f, 24.2f });
	xgeXuiWidgetSetRect(pDockWidget, (xge_rect_t){ 120.0f, 12.0f, 188.0f, 132.0f });
	xgeXuiWidgetSetRect(pTipWidget, (xge_rect_t){ 0.0f, 0.0f, 320.0f, 180.0f });
	xgeXuiWidgetSetLayout(pPixelRow, XGE_XUI_LAYOUT_ROW);
	xgeXuiWidgetSetRect(pPixelRow, (xge_rect_t){ 20.0f, 92.0f, 29.0f, 12.0f });
	xgeXuiWidgetSetSize(pPixelChildA, xgeXuiSizeGrow(1.0f), xgeXuiSizeGrow(1.0f));
	xgeXuiWidgetSetSize(pPixelChildB, xgeXuiSizeGrow(1.0f), xgeXuiSizeGrow(1.0f));
	xgeXuiWidgetSetSize(pPixelChildC, xgeXuiSizeGrow(1.0f), xgeXuiSizeGrow(1.0f));
	xgeXuiWidgetAdd(pRoot, pButtonWidget);
	xgeXuiWidgetAdd(pRoot, pLabelWidget);
	xgeXuiWidgetAdd(pRoot, pDockWidget);
	xgeXuiWidgetAdd(pRoot, pPixelRow);
	xgeXuiWidgetAdd(pRoot, pTipWidget);
	xgeXuiWidgetAdd(pPixelRow, pPixelChildA);
	xgeXuiWidgetAdd(pPixelRow, pPixelChildB);
	xgeXuiWidgetAdd(pPixelRow, pPixelChildC);

	if ( xgeXuiButtonInit(&tButton, &tXui, pButtonWidget) != XGE_OK ) {
		printf("xui_null_host_smoke failed: button init\n");
		xgeXuiUnit(&tXui);
		return 1;
	}
	xgeXuiButtonSetText(&tButton, NULL, "Null Host");
	xgeXuiButtonSetClick(&tButton, OnClick, NULL);

	if ( xgeXuiLabelInit(&tLabel, pLabelWidget, NULL, "Label") != XGE_OK ) {
		printf("xui_null_host_smoke failed: label init\n");
		xgeXuiUnit(&tXui);
		return 1;
	}

	if ( xgeXuiMsgTipInit(&tTip, &tXui, pTipWidget, NULL) != XGE_OK ) {
		printf("xui_null_host_smoke failed: msgtip init\n");
		xgeXuiUnit(&tXui);
		return 1;
	}
	xgeXuiMsgTipSetMetrics(&tTip, 60.0f, 180.0f, 30.0f, 0.0f);
	xgeXuiMsgTipShow(&tTip, XGE_XUI_MSG_TIP_ICON_WAR, "Null host tip", 0.05f);

	if ( xgeXuiDockLayoutInit(&tDockLayout, &tXui, pDockWidget) != XGE_OK ) {
		printf("xui_null_host_smoke failed: dock layout init\n");
		xgeXuiMsgTipUnit(&tTip);
		xgeXuiLabelUnit(&tLabel);
		xgeXuiButtonUnit(&tButton);
		xgeXuiUnit(&tXui);
		return 1;
	}
	if ( xgeXuiDockWindowInit(&tDockWindow, &tXui) != XGE_OK ) {
		printf("xui_null_host_smoke failed: dock window init\n");
		xgeXuiDockLayoutUnit(&tDockLayout);
		xgeXuiMsgTipUnit(&tTip);
		xgeXuiLabelUnit(&tLabel);
		xgeXuiButtonUnit(&tButton);
		xgeXuiUnit(&tXui);
		return 1;
	}
	xgeXuiDockWindowSetTitle(&tDockWindow, "Docked");
	xgeXuiDockWindowSetClientWidget(&tDockWindow, pDockContent);
	xgeXuiDockWindowSetClosable(&tDockWindow, 1);
	xgeXuiDockWindowSetDockable(&tDockWindow, 1);
	pDockPane = xgeXuiDockLayoutDockWindow(&tDockLayout, &tDockWindow, XGE_XUI_DOCK_REGION_DOCUMENT, XGE_XUI_DOCK_SIDE_FILL, 0.0f);

	if ( xgeXuiMenuInit(&tMenu, &tXui) != XGE_OK ) {
		printf("xui_null_host_smoke failed: menu init\n");
		xgeXuiDockWindowUnit(&tDockWindow);
		xgeXuiDockLayoutUnit(&tDockLayout);
		xgeXuiMsgTipUnit(&tTip);
		xgeXuiLabelUnit(&tLabel);
		xgeXuiButtonUnit(&tButton);
		xgeXuiUnit(&tXui);
		return 1;
	}
	xgeXuiMenuSetItems(&tMenu, g_arrMenuItems, (int)(sizeof(g_arrMenuItems) / sizeof(g_arrMenuItems[0])));

	pTexture = NULL;
	iRet = xgeXuiTextureCreateRGBA(&tXui, 4, 4, NULL, 0, 0, &pTexture);
	bOk = (iRet == XGE_ERROR_UNSUPPORTED) && (pTexture == NULL);
	bOk = bOk && (pDockPane != NULL) && (xgeXuiDockWindowGetState(&tDockWindow) == XGE_XUI_DOCK_WINDOW_DOCKED);

	bOk = bOk && (xgeXuiUpdate(&tXui, 0.016f) == XGE_OK);
	tSnappedLabel = xgeXuiWidgetGetRect(pLabelWidget);
	bOk = bOk && (tSnappedLabel.fX == 13.0f) && (tSnappedLabel.fY == 49.0f) && (tSnappedLabel.fW == 220.0f) && (tSnappedLabel.fH == 24.0f);
	tPixelA = xgeXuiWidgetGetRect(pPixelChildA);
	tPixelB = xgeXuiWidgetGetRect(pPixelChildB);
	tPixelC = xgeXuiWidgetGetRect(pPixelChildC);
	bOk = bOk && (tPixelA.fX == 20.0f) && (tPixelA.fW == 10.0f);
	bOk = bOk && (tPixelB.fX == 30.0f) && (tPixelB.fW == 9.0f);
	bOk = bOk && (tPixelC.fX == 39.0f) && (tPixelC.fW == 10.0f);
	bOk = bOk && (pDockPane->tClientRect.fW > 0.0f) && (pDockPane->tClientRect.fH > 0.0f);
	xgeXuiMenuOpenForOwner(&tMenu, pButtonWidget);
	bOk = bOk && xgeXuiMenuIsOpen(&tMenu);
	xgeXuiUpdate(&tXui, 0.0f);
	bMenuHScrollHiddenOK = (tMenu.tPopup.pScrollView != NULL) && (tMenu.tPopup.pScrollView->tFrame.bShowHScroll == 0);
	bOk = bOk && bMenuHScrollHiddenOK;
	MakeMouse(&tEvent, XGE_EVENT_MOUSE_UP, 20.0f, 20.0f);
	(void)xgeXuiDispatchEvent(&tXui, &tEvent);
	bMenuOwnerUpOK = xgeXuiMenuIsOpen(&tMenu);
	bOk = bOk && bMenuOwnerUpOK;
	MakeMouse(&tEvent, XGE_EVENT_MOUSE_DOWN, 20.0f, 20.0f);
	(void)xgeXuiPopupEvent(&tMenu.tPopup, &tEvent);
	bMenuOwnerDownCloseOK = !xgeXuiMenuIsOpen(&tMenu);
	bOk = bOk && bMenuOwnerDownCloseOK;
	xgeXuiMenuClose(&tMenu);
	bOk = bOk && (xgeXuiPaint(&tXui) >= 0);
	xgeXuiUpdate(&tXui, 0.06f);
	bOk = bOk && !xgeXuiMsgTipIsOpen(&tTip);

	MakeMouse(&tEvent, XGE_EVENT_MOUSE_DOWN, 20.0f, 20.0f);
	bOk = bOk && (xgeXuiDispatchEvent(&tXui, &tEvent) == XGE_XUI_EVENT_CONSUMED);
	MakeMouse(&tEvent, XGE_EVENT_MOUSE_UP, 20.0f, 20.0f);
	bOk = bOk && (xgeXuiDispatchEvent(&tXui, &tEvent) == XGE_XUI_EVENT_CONSUMED);
	bOk = bOk && (g_iClicked == 1);

	xgeXuiMenuUnit(&tMenu);
	xgeXuiDockWindowUnit(&tDockWindow);
	xgeXuiWidgetFree(pDockContent);
	xgeXuiDockLayoutUnit(&tDockLayout);
	xgeXuiWidgetFree(pDockWidget);
	xgeXuiMsgTipUnit(&tTip);
	xgeXuiLabelUnit(&tLabel);
	xgeXuiButtonUnit(&tButton);
	xgeXuiUnit(&tXui);

	if ( !bOk ) {
		printf("xui_null_host_smoke failed: unsupported=%d clicked=%d menuOwnerUp=%d menuOwnerDownClose=%d menuHScrollHidden=%d\n", iRet, g_iClicked, bMenuOwnerUpOK, bMenuOwnerDownCloseOK, bMenuHScrollHiddenOK);
		return 1;
	}
	printf("xui_null_host_smoke ok: update/paint/input/dockpanel/unsupported texture verified\n");
	return 0;
}
