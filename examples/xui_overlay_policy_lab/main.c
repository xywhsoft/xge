#include "../../xge.h"
#include "../xui_demo_style.h"
#include <stdio.h>
#include <string.h>

static const char* g_arrMenuItems[] = { "Open", "Save", "Export", "Close" };
static const int g_arrMenuEnabled[] = { 1, 1, 0, 1 };

typedef struct app_state_t {
	xge_xui_context_t tXui;
	xge_xui_widget pOwnerWidget;
	xge_xui_widget pPopupWidget;
	xge_xui_widget pDialogWidget;
	xge_xui_widget pTipOwnerWidget;
	xge_xui_popup_t tPopup;
	xge_xui_dialog_t tDialog;
	xge_xui_menu_t tMenu;
	int iPopupCloseCount;
	int iDialogCloseCount;
	int iLegacyCaptureCount;
} app_state_t;

static void MakeMouseEvent(xge_event_t* pEvent, int iType, float fX, float fY)
{
	memset(pEvent, 0, sizeof(*pEvent));
	pEvent->iType = iType;
	pEvent->iParam1 = 0;
	pEvent->fX = fX;
	pEvent->fY = fY;
}

static void MakeKeyEvent(xge_event_t* pEvent, int iKey)
{
	memset(pEvent, 0, sizeof(*pEvent));
	pEvent->iType = XGE_EVENT_KEY_DOWN;
	pEvent->iParam1 = iKey;
}

static void PopupClose(xge_xui_widget pWidget, void* pUser)
{
	app_state_t* pApp;

	(void)pWidget;
	pApp = (app_state_t*)pUser;
	if ( pApp != NULL ) {
		pApp->iPopupCloseCount++;
	}
}

static void DialogClose(xge_xui_widget pWidget, void* pUser)
{
	app_state_t* pApp;

	(void)pWidget;
	pApp = (app_state_t*)pUser;
	if ( pApp != NULL ) {
		pApp->iDialogCloseCount++;
	}
}

static int LegacyCapture(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser)
{
	app_state_t* pApp;

	(void)pWidget;
	(void)pEvent;
	pApp = (app_state_t*)pUser;
	if ( pApp != NULL ) {
		pApp->iLegacyCaptureCount++;
	}
	return XGE_XUI_EVENT_CONTINUE;
}

static int CreateWidgets(app_state_t* pApp)
{
	xge_xui_widget pRoot;
	xge_xui_widget pOverlayRoot;

	pRoot = xgeXuiRoot(&pApp->tXui);
	pOverlayRoot = xgeXuiOverlayRoot(&pApp->tXui);
	if ( (pRoot == NULL) || (pOverlayRoot == NULL) ) {
		return 0;
	}

	pApp->pOwnerWidget = xgeXuiWidgetCreate();
	pApp->pPopupWidget = xgeXuiWidgetCreate();
	pApp->pDialogWidget = xgeXuiWidgetCreate();
	pApp->pTipOwnerWidget = xgeXuiWidgetCreate();
	if ( (pApp->pOwnerWidget == NULL) || (pApp->pPopupWidget == NULL) || (pApp->pDialogWidget == NULL) ||
	     (pApp->pTipOwnerWidget == NULL) ) {
		return 0;
	}

	xgeXuiWidgetSetRect(pApp->pOwnerWidget, (xge_rect_t){ 20.0f, 20.0f, 100.0f, 30.0f });
	xgeXuiWidgetSetRect(pApp->pPopupWidget, (xge_rect_t){ 160.0f, 20.0f, 120.0f, 70.0f });
	xgeXuiWidgetSetRect(pApp->pDialogWidget, (xge_rect_t){ 160.0f, 120.0f, 180.0f, 100.0f });
	xgeXuiWidgetSetRect(pApp->pTipOwnerWidget, (xge_rect_t){ 20.0f, 80.0f, 120.0f, 32.0f });
	xgeXuiWidgetSetFocusable(pApp->pOwnerWidget, 1);
	xgeXuiWidgetSetFocusable(pApp->pTipOwnerWidget, 1);

	xgeXuiWidgetAdd(pRoot, pApp->pOwnerWidget);
	xgeXuiWidgetAdd(pRoot, pApp->pTipOwnerWidget);
	xgeXuiWidgetAdd(pOverlayRoot, pApp->pPopupWidget);
	xgeXuiWidgetAdd(pOverlayRoot, pApp->pDialogWidget);
	return 1;
}

static int TestPopup(app_state_t* pApp)
{
	xge_event_t tEvent;
	int bInitOK;
	int bNoAutoCloseOK;
	int bOwnerPassthroughOK;
	int bOutsideOK;
	int bEscapeOK;
	int bFocusOK;
	int iBefore;

	if ( xgeXuiPopupInit(&pApp->tPopup, &pApp->tXui, pApp->pPopupWidget) != XGE_OK ) {
		return 0;
	}
	xgeXuiPopupSetOwner(&pApp->tPopup, pApp->pOwnerWidget);
	xgeXuiPopupSetClose(&pApp->tPopup, PopupClose, pApp);

	bInitOK = (pApp->pPopupWidget->procEvent == xgeXuiPopupEventProc) &&
		((pApp->pPopupWidget->iFlags & XGE_XUI_WIDGET_VISIBLE) == 0) &&
		(pApp->tPopup.bCloseOnOutside == 1) && (pApp->tPopup.bCloseOnEscape == 1);

	xgeXuiPopupSetAutoClose(&pApp->tPopup, 0, 0);
	xgeXuiPopupSetOpen(&pApp->tPopup, 1);
	iBefore = pApp->iPopupCloseCount;
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_DOWN, 500.0f, 500.0f);
	(void)xgeXuiPopupEvent(&pApp->tPopup, &tEvent);
	MakeKeyEvent(&tEvent, XGE_KEY_ESCAPE);
	(void)xgeXuiPopupEvent(&pApp->tPopup, &tEvent);
	bNoAutoCloseOK = xgeXuiPopupIsOpen(&pApp->tPopup) && (pApp->iPopupCloseCount == iBefore);

	xgeXuiPopupSetAutoClose(&pApp->tPopup, 1, 1);
	xgeXuiPopupSetOpen(&pApp->tPopup, 1);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_DOWN, 40.0f, 30.0f);
	bOwnerPassthroughOK = (xgeXuiPopupEvent(&pApp->tPopup, &tEvent) == XGE_XUI_EVENT_CONTINUE) && xgeXuiPopupIsOpen(&pApp->tPopup);

	xgeXuiPopupSetAutoClose(&pApp->tPopup, 1, 0);
	xgeXuiPopupSetOpen(&pApp->tPopup, 1);
	iBefore = pApp->iPopupCloseCount;
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_DOWN, 500.0f, 500.0f);
	bOutsideOK = (xgeXuiPopupEvent(&pApp->tPopup, &tEvent) == XGE_XUI_EVENT_CONSUMED) &&
		(!xgeXuiPopupIsOpen(&pApp->tPopup)) && (pApp->iPopupCloseCount == iBefore + 1);

	xgeXuiPopupSetAutoClose(&pApp->tPopup, 0, 1);
	xgeXuiPopupSetOpen(&pApp->tPopup, 1);
	iBefore = pApp->iPopupCloseCount;
	MakeKeyEvent(&tEvent, XGE_KEY_ESCAPE);
	bEscapeOK = (xgeXuiPopupEvent(&pApp->tPopup, &tEvent) == XGE_XUI_EVENT_CONSUMED) &&
		(!xgeXuiPopupIsOpen(&pApp->tPopup)) && (pApp->iPopupCloseCount == iBefore + 1);

	xgeXuiPopupSetOpen(&pApp->tPopup, 1);
	bFocusOK = ((pApp->pPopupWidget->iFlags & XGE_XUI_WIDGET_VISIBLE) != 0) && (pApp->tXui.pFocus == pApp->pPopupWidget);

	return bInitOK && bNoAutoCloseOK && bOwnerPassthroughOK && bOutsideOK && bEscapeOK && bFocusOK;
}

static int TestDialog(app_state_t* pApp)
{
	xge_event_t tEvent;
	int bInitOK;
	int bModalOK;
	int bNonModalOK;
	int bEscapeDisabledOK;
	int bEscapeEnabledOK;
	int bReopenOK;
	int bOutsideCloseOK;
	int bFocusRestoreOK;
	int bZOK;
	int iBefore;

	if ( xgeXuiDialogInit(&pApp->tDialog, &pApp->tXui, pApp->pDialogWidget) != XGE_OK ) {
		return 0;
	}
	xgeXuiDialogSetClose(&pApp->tDialog, DialogClose, pApp);
	xgeXuiDialogSetTitle(&pApp->tDialog, NULL, "Policy");

	bInitOK = xgeXuiDialogIsOpen(&pApp->tDialog) && (pApp->tDialog.bModal == 1) &&
		(pApp->tDialog.bCloseOnEscape == 1) && (pApp->tDialog.bShowClose == 1);

	xgeXuiDialogSetOpen(&pApp->tDialog, 1);
	xgeXuiDialogSetModal(&pApp->tDialog, 1);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_DOWN, 20.0f, 20.0f);
	bModalOK = (xgeXuiDialogEvent(&pApp->tDialog, &tEvent) == XGE_XUI_EVENT_CONSUMED) && xgeXuiDialogIsOpen(&pApp->tDialog);

	xgeXuiDialogSetModal(&pApp->tDialog, 0);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_DOWN, 20.0f, 20.0f);
	bNonModalOK = (xgeXuiDialogEvent(&pApp->tDialog, &tEvent) == XGE_XUI_EVENT_CONTINUE) && xgeXuiDialogIsOpen(&pApp->tDialog);

	xgeXuiDialogSetModal(&pApp->tDialog, 1);
	xgeXuiDialogSetCloseOnEscape(&pApp->tDialog, 0);
	xgeXuiDialogSetOpen(&pApp->tDialog, 1);
	iBefore = pApp->iDialogCloseCount;
	MakeKeyEvent(&tEvent, XGE_KEY_ESCAPE);
	bEscapeDisabledOK = (xgeXuiDialogEvent(&pApp->tDialog, &tEvent) == XGE_XUI_EVENT_CONSUMED) &&
		xgeXuiDialogIsOpen(&pApp->tDialog) && (pApp->iDialogCloseCount == iBefore);

	xgeXuiDialogSetCloseOnEscape(&pApp->tDialog, 1);
	iBefore = pApp->iDialogCloseCount;
	MakeKeyEvent(&tEvent, XGE_KEY_ESCAPE);
	bEscapeEnabledOK = (xgeXuiDialogEvent(&pApp->tDialog, &tEvent) == XGE_XUI_EVENT_CONSUMED) &&
		(!xgeXuiDialogIsOpen(&pApp->tDialog)) && (pApp->iDialogCloseCount == iBefore + 1);

	xgeXuiDialogSetOpen(&pApp->tDialog, 1);
	bReopenOK = xgeXuiDialogIsOpen(&pApp->tDialog) && (pApp->tXui.pFocus == pApp->pDialogWidget);
	xgeXuiDialogSetFocusRestore(&pApp->tDialog, pApp->pOwnerWidget);
	xgeXuiDialogSetZBase(&pApp->tDialog, 1400);
	bZOK = (xgeXuiWidgetGetZ(pApp->pDialogWidget) == 1400);
	xgeXuiDialogSetCloseOnOutside(&pApp->tDialog, 1);
	iBefore = pApp->iDialogCloseCount;
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_DOWN, 20.0f, 20.0f);
	bOutsideCloseOK = (xgeXuiDialogEvent(&pApp->tDialog, &tEvent) == XGE_XUI_EVENT_CONSUMED) &&
		(!xgeXuiDialogIsOpen(&pApp->tDialog)) && (pApp->iDialogCloseCount == iBefore + 1);
	bFocusRestoreOK = (pApp->tXui.pFocus == pApp->pOwnerWidget);
	xgeXuiDialogSetCloseOnOutside(&pApp->tDialog, 0);

	return bInitOK && bModalOK && bNonModalOK && bEscapeDisabledOK && bEscapeEnabledOK && bReopenOK && bOutsideCloseOK && bFocusRestoreOK && bZOK;
}

static int TestMenu(app_state_t* pApp)
{
	xge_event_t tEvent;
	int bInitOK;
	int bOpenOK;
	int bCloseOK;
	int bEscapeOK;

	if ( xgeXuiMenuInit(&pApp->tMenu, &pApp->tXui, pApp->pOwnerWidget) != XGE_OK ) {
		return 0;
	}
	xgeXuiMenuSetItems(&pApp->tMenu, g_arrMenuItems, (int)(sizeof(g_arrMenuItems) / sizeof(g_arrMenuItems[0])));
	xgeXuiMenuSetEnabledItems(&pApp->tMenu, g_arrMenuEnabled, (int)(sizeof(g_arrMenuEnabled) / sizeof(g_arrMenuEnabled[0])));
	xgeXuiMenuSetSize(&pApp->tMenu, 160.0f, 120.0f);

	bInitOK = (pApp->tMenu.pPopupWidget != NULL) && (pApp->tMenu.pListWidget != NULL) &&
		(pApp->tMenu.iItemCount == 4) && (pApp->tMenu.iEnabledCount == 4);

	xgeXuiMenuOpen(&pApp->tMenu, 40.0f, 60.0f);
	bOpenOK = xgeXuiMenuIsOpen(&pApp->tMenu) && (pApp->tXui.pFocus == pApp->tMenu.pListWidget);

	xgeXuiMenuClose(&pApp->tMenu);
	bCloseOK = (!xgeXuiMenuIsOpen(&pApp->tMenu)) && (pApp->tXui.pFocus == pApp->pOwnerWidget);

	xgeXuiMenuOpen(&pApp->tMenu, 40.0f, 60.0f);
	MakeKeyEvent(&tEvent, XGE_KEY_ESCAPE);
	bEscapeOK = (xgeXuiPopupEvent(&pApp->tMenu.tPopup, &tEvent) == XGE_XUI_EVENT_CONSUMED) &&
		(!xgeXuiMenuIsOpen(&pApp->tMenu)) && (pApp->tXui.pFocus == pApp->pOwnerWidget);

	return bInitOK && bOpenOK && bCloseOK && bEscapeOK;
}

static int TestTooltip(app_state_t* pApp)
{
	xge_event_t tEvent;
	xge_xui_tooltip_desc_t tDesc;
	xge_rect_t tRect;

	xgeXuiPopupSetOpen(&pApp->tPopup, 0);
	xgeXuiDialogSetOpen(&pApp->tDialog, 0);
	xgeXuiMenuClose(&pApp->tMenu);
	xgeXuiSetCapture(&pApp->tXui, NULL);
	xgeXuiWidgetSetCaptureEventUser(pApp->pTipOwnerWidget, LegacyCapture, pApp);
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iType = XGE_XUI_TOOLTIP_TEXT;
	tDesc.sText = "Overlay hint";
	tDesc.iAnchor = XGE_XUI_TOOLTIP_ANCHOR_WIDGET_BOTTOM;
	tDesc.fOffsetX = 8.0f;
	tDesc.fOffsetY = 10.0f;
	tDesc.fDelay = 0.0f;
	xgeXuiWidgetSetTooltip(pApp->pTipOwnerWidget, &tDesc);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_MOVE, 48.0f, 92.0f);
	xgeXuiTooltipHandleEvent(&pApp->tXui, pApp->pTipOwnerWidget, &tEvent);
	xgeXuiTooltipUpdate(&pApp->tXui, 0.0f);
	tRect = xgeXuiTooltipGetRect(&pApp->tXui);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_DOWN, 48.0f, 92.0f);
	xgeXuiTooltipHandleEvent(&pApp->tXui, pApp->pTipOwnerWidget, &tEvent);
	return (pApp->pTipOwnerWidget->procCaptureEvent == LegacyCapture) &&
		(xgeXuiTooltipGetOwner(&pApp->tXui) == NULL) &&
		(tRect.fW > 0.0f) && (tRect.fH > 0.0f) &&
		(xgeXuiTooltipIsOpen(&pApp->tXui) == 0);
}

static int TestTopOverlayEscape(app_state_t* pApp)
{
	xge_event_t tEvent;
	int iBefore;
	int iResult;

	xgeXuiPopupSetAutoClose(&pApp->tPopup, 1, 1);
	xgeXuiPopupSetOpen(&pApp->tPopup, 1);
	xgeXuiMenuOpen(&pApp->tMenu, 44.0f, 64.0f);
	iBefore = pApp->iPopupCloseCount;
	MakeKeyEvent(&tEvent, XGE_KEY_ESCAPE);
	iResult = xgeXuiDispatchEvent(&pApp->tXui, &tEvent);
	return (iResult == XGE_XUI_EVENT_CONSUMED) &&
		(!xgeXuiMenuIsOpen(&pApp->tMenu)) &&
		xgeXuiPopupIsOpen(&pApp->tPopup) &&
		(pApp->iPopupCloseCount == iBefore) &&
		(pApp->tXui.pFocus == pApp->pOwnerWidget);
}

static int TestPopupPolicy(app_state_t* pApp)
{
	xge_event_t tEvent;
	xge_rect_t tRect;
	int bPlacementOK;
	int bTopOK;
	int bModalOK;
	int bOwnerDisabledOK;

	xgeXuiWidgetSetRect(pApp->pPopupWidget, (xge_rect_t){ 0.0f, 0.0f, 120.0f, 70.0f });
	xgeXuiPopupSetPlacement(&pApp->tPopup, XGE_XUI_OVERLAY_PLACEMENT_BOTTOM_LEFT);
	xgeXuiPopupSetAnchorRect(&pApp->tPopup, pApp->pOwnerWidget->tRect);
	xgeXuiPopupSetOffset(&pApp->tPopup, 5.0f, 6.0f);
	xgeXuiPopupApplyPlacement(&pApp->tPopup);
	tRect = pApp->pPopupWidget->tRect;
	bPlacementOK = (tRect.fX == 25.0f) && (tRect.fY == 56.0f) && (tRect.fW == 120.0f) && (tRect.fH == 70.0f);

	xgeXuiPopupSetZBase(&pApp->tPopup, 1100);
	xgeXuiPopupSetOpen(&pApp->tPopup, 1);
	xgeXuiMenuOpen(&pApp->tMenu, 44.0f, 64.0f);
	bTopOK = (xgeXuiOverlayTop(&pApp->tXui) == pApp->tMenu.pPopupWidget);
	xgeXuiMenuClose(&pApp->tMenu);

	xgeXuiPopupSetAutoClose(&pApp->tPopup, 0, 0);
	xgeXuiPopupSetModal(&pApp->tPopup, 1);
	xgeXuiPopupSetOpen(&pApp->tPopup, 1);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_DOWN, 500.0f, 500.0f);
	bModalOK = (xgeXuiPopupEvent(&pApp->tPopup, &tEvent) == XGE_XUI_EVENT_CONSUMED) && xgeXuiPopupIsOpen(&pApp->tPopup);
	xgeXuiPopupSetModal(&pApp->tPopup, 0);

	xgeXuiWidgetSetEnabled(pApp->pOwnerWidget, 0);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_DOWN, 500.0f, 500.0f);
	bOwnerDisabledOK = (xgeXuiPopupEvent(&pApp->tPopup, &tEvent) == XGE_XUI_EVENT_CONSUMED) && (!xgeXuiPopupIsOpen(&pApp->tPopup));
	xgeXuiWidgetSetEnabled(pApp->pOwnerWidget, 1);
	xgeXuiPopupSetAutoClose(&pApp->tPopup, 1, 1);

	return bPlacementOK && bTopOK && bModalOK && bOwnerDisabledOK;
}

int main(void)
{
	app_state_t tApp;
	int bCreateOK;
	int bPopupOK;
	int bDialogOK;
	int bMenuOK;
	int bTooltipOK;
	int bTopEscapeOK;
	int bPolicyOK;

	memset(&tApp, 0, sizeof(tApp));
	if ( xgeXuiInit(&tApp.tXui) != XGE_OK ) {
		printf("xui-overlay-policy-lab init failed\n");
		return 1;
	}

	bCreateOK = CreateWidgets(&tApp);
	bPopupOK = bCreateOK && TestPopup(&tApp);
	bDialogOK = bCreateOK && TestDialog(&tApp);
	bMenuOK = bCreateOK && TestMenu(&tApp);
	bTooltipOK = bCreateOK && TestTooltip(&tApp);
	bTopEscapeOK = bCreateOK && bPopupOK && bMenuOK && TestTopOverlayEscape(&tApp);
	bPolicyOK = bCreateOK && bPopupOK && bMenuOK && TestPopupPolicy(&tApp);

	printf(
		"xui-overlay-policy-lab final-summary create=%d popup=%d dialog=%d menu=%d tooltip=%d top_escape=%d policy=%d popup_close=%d dialog_close=%d legacy=%d\n",
		bCreateOK,
		bPopupOK,
		bDialogOK,
		bMenuOK,
		bTooltipOK,
		bTopEscapeOK,
		bPolicyOK,
		tApp.iPopupCloseCount,
		tApp.iDialogCloseCount,
		tApp.iLegacyCaptureCount);

	xgeXuiMenuUnit(&tApp.tMenu);
	xgeXuiDialogUnit(&tApp.tDialog);
	xgeXuiPopupUnit(&tApp.tPopup);
	xgeXuiUnit(&tApp.tXui);
	return (bCreateOK && bPopupOK && bDialogOK && bMenuOK && bTooltipOK && bTopEscapeOK && bPolicyOK) ? 0 : 2;
}
