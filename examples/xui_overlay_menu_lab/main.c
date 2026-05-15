#include "../../xge.h"
#include "../xui_demo_style.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char* g_arrComboItems[] = {
	"Low",
	"Medium",
	"High",
	"Ultra"
};

static const xge_xui_menu_item_t g_arrMenuItems[] = {
	{ "Open", NULL, XGE_XUI_MENU_ITEM_NORMAL, XGE_XUI_MENU_ITEM_ENABLED, 0, 0, NULL, NULL },
	{ "Disabled", NULL, XGE_XUI_MENU_ITEM_NORMAL, 0, 1, 0, NULL, NULL },
	{ "Rename", NULL, XGE_XUI_MENU_ITEM_NORMAL, XGE_XUI_MENU_ITEM_ENABLED, 2, 0, NULL, NULL },
	{ "Delete", NULL, XGE_XUI_MENU_ITEM_NORMAL, XGE_XUI_MENU_ITEM_ENABLED | XGE_XUI_MENU_ITEM_DANGER, 3, 0, NULL, NULL }
};

typedef struct app_state_t {
	xge_xui_context_t tXui;
	xge_font_t tFont;
	xge_xui_widget pRootPanel;
	xge_xui_widget pStatusWidget;
	xge_xui_widget pPopupOwnerWidget;
	xge_xui_widget pTooltipOwnerWidget;
	xge_xui_widget pComboWidget;
	xge_xui_widget pMenuOwnerWidget;
	xge_xui_widget pDialogWidget;
	xge_xui_widget pDialogLabelWidget;
	xge_xui_widget pPopupWidget;
	xge_xui_widget pPopupLabelWidget;
	xge_xui_label_t tStatusLabel;
	xge_xui_label_t tPopupOwnerLabel;
	xge_xui_label_t tTooltipOwnerLabel;
	xge_xui_label_t tMenuOwnerLabel;
	xge_xui_label_t tDialogLabel;
	xge_xui_label_t tPopupLabel;
	xge_xui_popup_t tPopup;
	xge_xui_combo_box_t tCombo;
	xge_xui_menu_t tMenu;
	xge_xui_dialog_t tDialog;
	int bFontReady;
	int iFrameLimit;
	int iFrameCount;
	int iLastWidth;
	int iLastHeight;
	int iPopupCloseCount;
	int iDialogCloseCount;
	int iComboSelectCount;
	int iMenuSelectCount;
	int iTooltipOwnerCaptureCount;
	int iLastComboSelected;
	int iLastMenuSelected;
	int bPopupOK;
	int bTooltipOK;
	int bComboOK;
	int bMenuOK;
	int bDialogOK;
	int bPolicyOK;
} app_state_t;

static int ArgInt(const char* sText, int iDefault)
{
	int iValue;

	if ( (sText == NULL) || (sText[0] == 0) ) {
		return iDefault;
	}
	iValue = atoi(sText);
	return (iValue > 0) ? iValue : iDefault;
}

static int FloatNear(float fA, float fB, float fEpsilon)
{
	return fabsf(fA - fB) <= fEpsilon;
}

static void MakeMouseEvent(xge_event_t* pEvent, int iType, int iButton, float fX, float fY)
{
	memset(pEvent, 0, sizeof(*pEvent));
	pEvent->iType = iType;
	pEvent->iParam1 = iButton;
	pEvent->fX = fX;
	pEvent->fY = fY;
}

static void MakeKeyEvent(xge_event_t* pEvent, int iKey)
{
	memset(pEvent, 0, sizeof(*pEvent));
	pEvent->iType = XGE_EVENT_KEY_DOWN;
	pEvent->iParam1 = iKey;
}

static xge_vec2_t WidgetCenter(xge_xui_widget pWidget)
{
	xge_vec2_t tCenter;

	tCenter.fX = pWidget->tRect.fX + (pWidget->tRect.fW * 0.5f);
	tCenter.fY = pWidget->tRect.fY + (pWidget->tRect.fH * 0.5f);
	return tCenter;
}

static xge_rect_t DialogCloseRect(xge_xui_dialog pDialog)
{
	xge_rect_t tClose;

	memset(&tClose, 0, sizeof(tClose));
	tClose.fW = 18.0f;
	tClose.fH = 18.0f;
	tClose.fX = pDialog->pWidget->tContentRect.fX + pDialog->pWidget->tContentRect.fW - tClose.fW - 2.0f;
	tClose.fY = pDialog->pWidget->tContentRect.fY + 3.0f;
	return tClose;
}

static int LoadFont(app_state_t* pApp)
{
	const char* arrFonts[] = {
		"C:/Windows/Fonts/simsun.ttc",
		"C:/Windows/Fonts/Deng.ttf",
		"C:/Windows/Fonts/msyh.ttc",
		"C:/Windows/Fonts/arial.ttf"
	};
	int i;

	for ( i = 0; i < (int)(sizeof(arrFonts) / sizeof(arrFonts[0])); i++ ) {
		memset(&pApp->tFont, 0, sizeof(pApp->tFont));
		if ( xgeFontLoad(&pApp->tFont, arrFonts[i], XGE_XUI_DEMO_FONT_SIZE) == XGE_OK ) {
			pApp->bFontReady = 1;
			printf("xui-overlay-menu-lab font loaded: %s\n", arrFonts[i]);
			return XGE_OK;
		}
	}
	printf("xui-overlay-menu-lab font load failed\n");
	return XGE_ERROR_RESOURCE_FAILED;
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

static void ComboSelect(xge_xui_widget pWidget, int iIndex, void* pUser)
{
	app_state_t* pApp;

	(void)pWidget;
	pApp = (app_state_t*)pUser;
	if ( pApp != NULL ) {
		pApp->iComboSelectCount++;
		pApp->iLastComboSelected = iIndex;
	}
}

static void MenuSelect(xge_xui_widget pWidget, int iIndex, int iValue, void* pUser)
{
	app_state_t* pApp;

	(void)pWidget;
	(void)iIndex;
	pApp = (app_state_t*)pUser;
	if ( pApp != NULL ) {
		pApp->iMenuSelectCount++;
		pApp->iLastMenuSelected = iValue;
	}
}

static int TooltipOwnerCapture(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser)
{
	app_state_t* pApp;

	(void)pWidget;
	pApp = (app_state_t*)pUser;
	if ( (pApp != NULL) && (pEvent != NULL) && (pEvent->iType == XGE_EVENT_MOUSE_MOVE) ) {
		pApp->iTooltipOwnerCaptureCount++;
	}
	return XGE_XUI_EVENT_CONTINUE;
}

static void LayoutRoot(app_state_t* pApp)
{
	xge_rect_t tRoot;
	int iWidth;
	int iHeight;

	iWidth = xgeGetWidth();
	iHeight = xgeGetHeight();
	if ( (iWidth == pApp->iLastWidth) && (iHeight == pApp->iLastHeight) ) {
		return;
	}
	tRoot.fX = 20.0f;
	tRoot.fY = 20.0f;
	tRoot.fW = (float)iWidth - 40.0f;
	tRoot.fH = (float)iHeight - 40.0f;
	if ( tRoot.fW < 620.0f ) {
		tRoot.fW = 620.0f;
	}
	if ( tRoot.fH < 340.0f ) {
		tRoot.fH = 340.0f;
	}

	xgeXuiWidgetSetRect(pApp->pRootPanel, tRoot);
	xgeXuiWidgetSetRect(pApp->pStatusWidget, (xge_rect_t){ 0.0f, 0.0f, tRoot.fW, 38.0f });
	xgeXuiWidgetSetRect(pApp->pPopupOwnerWidget, (xge_rect_t){ 28.0f, 70.0f, 180.0f, 34.0f });
	xgeXuiWidgetSetRect(pApp->pTooltipOwnerWidget, (xge_rect_t){ 28.0f, 118.0f, 180.0f, 34.0f });
	xgeXuiWidgetSetRect(pApp->pComboWidget, (xge_rect_t){ 28.0f, 168.0f, 180.0f, 32.0f });
	xgeXuiWidgetSetRect(pApp->pMenuOwnerWidget, (xge_rect_t){ 28.0f, 216.0f, 180.0f, 34.0f });
	xgeXuiWidgetSetRect(pApp->pDialogWidget, (xge_rect_t){ 250.0f, 76.0f, 280.0f, 152.0f });
	xgeXuiWidgetSetRect(pApp->pDialogLabelWidget, (xge_rect_t){ 14.0f, 36.0f, 232.0f, 68.0f });
	xgeXuiWidgetSetRect(pApp->pPopupWidget, (xge_rect_t){ 248.0f, 88.0f, 196.0f, 84.0f });
	xgeXuiWidgetSetRect(pApp->pPopupLabelWidget, (xge_rect_t){ 0.0f, 0.0f, 164.0f, 44.0f });

	pApp->iLastWidth = iWidth;
	pApp->iLastHeight = iHeight;
}

static void UpdateStatus(app_state_t* pApp)
{
	char sText[256];

	snprintf(
		sText,
		sizeof(sText),
		"popup=%d tooltip=%d combo=%d menu=%d dialog=%d policy=%d cb=%d/%d/%d/%d tip=%d",
		pApp->bPopupOK,
		pApp->bTooltipOK,
		pApp->bComboOK,
		pApp->bMenuOK,
		pApp->bDialogOK,
		pApp->bPolicyOK,
		pApp->iPopupCloseCount,
		pApp->iDialogCloseCount,
		pApp->iComboSelectCount,
		pApp->iMenuSelectCount,
		pApp->iTooltipOwnerCaptureCount);
	xgeXuiLabelSetText(&pApp->tStatusLabel, sText);
}

static int CreateUI(app_state_t* pApp)
{
	xge_xui_theme_t tTheme;
	xge_xui_widget pRoot;
	xge_xui_widget pOverlayRoot;
	xge_font pFont;

	pRoot = xgeXuiRoot(&pApp->tXui);
	pOverlayRoot = xgeXuiOverlayRoot(&pApp->tXui);
	pFont = pApp->bFontReady ? &pApp->tFont : NULL;
	if ( (pRoot == NULL) || (pOverlayRoot == NULL) ) {
		return XGE_ERROR;
	}

	xgeXuiThemeDefault(&tTheme);
	tTheme.pFont = pFont;
	tTheme.iTextColor = XGE_COLOR_RGBA(240, 244, 250, 255);
	tTheme.iBackgroundColor = XGE_COLOR_RGBA(20, 26, 36, 255);
	tTheme.iPanelColor = XGE_COLOR_RGBA(32, 40, 54, 255);
	tTheme.iBorderColor = XGE_COLOR_RGBA(84, 98, 122, 255);
	tTheme.iSelectionColor = XGE_COLOR_RGBA(96, 164, 255, 255);
	tTheme.iAccentColor = XGE_COLOR_RGBA(96, 214, 144, 255);
	tTheme.iStateNormal = XGE_COLOR_RGBA(56, 124, 214, 255);
	tTheme.iStateHover = XGE_COLOR_RGBA(78, 146, 236, 255);
	tTheme.iStateActive = XGE_COLOR_RGBA(42, 96, 176, 255);
	tTheme.iStateFocus = XGE_COLOR_RGBA(86, 108, 144, 255);
	tTheme.iStateDisabled = XGE_COLOR_RGBA(84, 86, 92, 180);
	tTheme.fRadius = 8.0f;
	tTheme.fPadding = 8.0f;
	tTheme.fSpacing = 10.0f;
	xgeXuiSetTheme(&pApp->tXui, &tTheme);
	XgeXuiDemoApplyTheme(&pApp->tXui, pFont);

	pApp->pRootPanel = xgeXuiWidgetCreate();
	pApp->pStatusWidget = xgeXuiWidgetCreate();
	pApp->pPopupOwnerWidget = xgeXuiWidgetCreate();
	pApp->pTooltipOwnerWidget = xgeXuiWidgetCreate();
	pApp->pComboWidget = xgeXuiWidgetCreate();
	pApp->pMenuOwnerWidget = xgeXuiWidgetCreate();
	pApp->pDialogWidget = xgeXuiWidgetCreate();
	pApp->pDialogLabelWidget = xgeXuiWidgetCreate();
	pApp->pPopupWidget = xgeXuiWidgetCreate();
	pApp->pPopupLabelWidget = xgeXuiWidgetCreate();
	if ( (pApp->pRootPanel == NULL) || (pApp->pStatusWidget == NULL) || (pApp->pPopupOwnerWidget == NULL) || (pApp->pTooltipOwnerWidget == NULL) || (pApp->pComboWidget == NULL) || (pApp->pMenuOwnerWidget == NULL) || (pApp->pDialogWidget == NULL) || (pApp->pDialogLabelWidget == NULL) || (pApp->pPopupWidget == NULL) || (pApp->pPopupLabelWidget == NULL) ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}

	xgeXuiWidgetSetLayout(pApp->pRootPanel, XGE_XUI_LAYOUT_ABSOLUTE);
	xgeXuiWidgetSetBackground(pApp->pRootPanel, XGE_COLOR_RGBA(22, 28, 38, 255));
	xgeXuiWidgetSetRadius(pApp->pRootPanel, 8.0f);
	if ( xgeXuiWidgetAdd(pRoot, pApp->pRootPanel) != XGE_OK ) {
		return XGE_ERROR;
	}

	xgeXuiWidgetSetBackground(pApp->pStatusWidget, XGE_COLOR_RGBA(44, 58, 78, 255));
	xgeXuiWidgetSetPaddingPx(pApp->pStatusWidget, 10.0f, 8.0f, 10.0f, 8.0f);
	xgeXuiLabelInit(&pApp->tStatusLabel, pApp->pStatusWidget, pFont, "xui overlay menu lab");
	xgeXuiLabelSetColor(&pApp->tStatusLabel, XGE_COLOR_RGBA(248, 250, 252, 255));
	if ( xgeXuiWidgetAdd(pApp->pRootPanel, pApp->pStatusWidget) != XGE_OK ) {
		return XGE_ERROR;
	}

	xgeXuiWidgetSetBackground(pApp->pPopupOwnerWidget, XGE_COLOR_RGBA(52, 70, 94, 255));
	xgeXuiWidgetSetPaddingPx(pApp->pPopupOwnerWidget, 10.0f, 8.0f, 10.0f, 8.0f);
	xgeXuiWidgetSetFocusable(pApp->pPopupOwnerWidget, 1);
	xgeXuiLabelInit(&pApp->tPopupOwnerLabel, pApp->pPopupOwnerWidget, pFont, "Popup owner");
	xgeXuiLabelSetColor(&pApp->tPopupOwnerLabel, XGE_COLOR_RGBA(248, 250, 252, 255));
	if ( xgeXuiWidgetAdd(pApp->pRootPanel, pApp->pPopupOwnerWidget) != XGE_OK ) {
		return XGE_ERROR;
	}

	xgeXuiWidgetSetBackground(pApp->pTooltipOwnerWidget, XGE_COLOR_RGBA(62, 78, 98, 255));
	xgeXuiWidgetSetPaddingPx(pApp->pTooltipOwnerWidget, 10.0f, 8.0f, 10.0f, 8.0f);
	xgeXuiWidgetSetFocusable(pApp->pTooltipOwnerWidget, 1);
	xgeXuiWidgetSetCaptureEventUser(pApp->pTooltipOwnerWidget, TooltipOwnerCapture, pApp);
	xgeXuiLabelInit(&pApp->tTooltipOwnerLabel, pApp->pTooltipOwnerWidget, pFont, "Tooltip target");
	xgeXuiLabelSetColor(&pApp->tTooltipOwnerLabel, XGE_COLOR_RGBA(248, 250, 252, 255));
	if ( xgeXuiWidgetAdd(pApp->pRootPanel, pApp->pTooltipOwnerWidget) != XGE_OK ) {
		return XGE_ERROR;
	}

	xgeXuiComboBoxInit(&pApp->tCombo, &pApp->tXui, pApp->pComboWidget);
	if ( xgeXuiWidgetAdd(pApp->pRootPanel, pApp->pComboWidget) != XGE_OK ) {
		return XGE_ERROR;
	}

	xgeXuiWidgetSetBackground(pApp->pMenuOwnerWidget, XGE_COLOR_RGBA(52, 70, 94, 255));
	xgeXuiWidgetSetPaddingPx(pApp->pMenuOwnerWidget, 10.0f, 8.0f, 10.0f, 8.0f);
	xgeXuiLabelInit(&pApp->tMenuOwnerLabel, pApp->pMenuOwnerWidget, pFont, "Menu owner");
	xgeXuiWidgetSetFocusable(pApp->pMenuOwnerWidget, 1);
	xgeXuiLabelSetColor(&pApp->tMenuOwnerLabel, XGE_COLOR_RGBA(248, 250, 252, 255));
	if ( xgeXuiWidgetAdd(pApp->pRootPanel, pApp->pMenuOwnerWidget) != XGE_OK ) {
		return XGE_ERROR;
	}

	xgeXuiPopupInit(&pApp->tPopup, &pApp->tXui, pApp->pPopupWidget);
	xgeXuiPopupSetOwner(&pApp->tPopup, pApp->pPopupOwnerWidget);
	xgeXuiPopupSetClose(&pApp->tPopup, PopupClose, pApp);
	xgeXuiPopupSetBackground(&pApp->tPopup, XGE_COLOR_RGBA(44, 52, 68, 245));
	xgeXuiWidgetSetZ(pApp->pPopupWidget, 1100);
	if ( xgeXuiWidgetAdd(pOverlayRoot, pApp->pPopupWidget) != XGE_OK ) {
		return XGE_ERROR;
	}

	xgeXuiWidgetSetPaddingPx(pApp->pPopupLabelWidget, 0.0f, 0.0f, 0.0f, 0.0f);
	xgeXuiLabelInit(&pApp->tPopupLabel, pApp->pPopupLabelWidget, pFont, "Popup overlay\noutside click or ESC closes");
	xgeXuiLabelSetColor(&pApp->tPopupLabel, XGE_COLOR_RGBA(248, 250, 252, 255));
	if ( xgeXuiWidgetAdd(pApp->pPopupWidget, pApp->pPopupLabelWidget) != XGE_OK ) {
		return XGE_ERROR;
	}

	xgeXuiMenuInit(&pApp->tMenu, &pApp->tXui);

	xgeXuiDialogInit(&pApp->tDialog, &pApp->tXui, pApp->pDialogWidget);
	xgeXuiDialogSetTitle(&pApp->tDialog, pFont, "Overlay dialog");
	xgeXuiDialogSetClose(&pApp->tDialog, DialogClose, pApp);
	xgeXuiDialogSetColors(&pApp->tDialog, XGE_COLOR_RGBA(0, 0, 0, 120), XGE_COLOR_RGBA(42, 48, 60, 255), XGE_COLOR_RGBA(248, 250, 252, 255), XGE_COLOR_RGBA(212, 96, 96, 255));
	xgeXuiWidgetSetZ(pApp->pDialogWidget, 1050);
	if ( xgeXuiWidgetAdd(pOverlayRoot, pApp->pDialogWidget) != XGE_OK ) {
		return XGE_ERROR;
	}

	xgeXuiLabelInit(&pApp->tDialogLabel, pApp->pDialogLabelWidget, pFont, "Dialog blocks outside input while modal.\nClose button and ESC are verified.");
	xgeXuiLabelSetColor(&pApp->tDialogLabel, XGE_COLOR_RGBA(248, 250, 252, 255));
	if ( xgeXuiWidgetAdd(pApp->pDialogWidget, pApp->pDialogLabelWidget) != XGE_OK ) {
		return XGE_ERROR;
	}

	return XGE_OK;
}

static int RunStaticChecks(app_state_t* pApp)
{
	xge_event_t tEvent;
	xge_rect_t tRect;
	xge_vec2_t tCenter;
	int iRet;
	int bPopupPolicyOK;
	int bTooltipPolicyOK;
	int bComboPolicyOK;
	int bMenuPolicyOK;
	int bDialogDefaultsOK;
	int bDialogPolicyOK;
	xge_xui_menu_metrics_t tMenuMetrics;
	xge_xui_menu_colors_t tMenuColors;

	if ( CreateUI(pApp) != XGE_OK ) {
		return XGE_ERROR;
	}
	LayoutRoot(pApp);
	xgeXuiUpdate(&pApp->tXui, 0.0f);

	bDialogDefaultsOK =
		(xgeXuiDialogIsOpen(&pApp->tDialog) != 0) &&
		(pApp->tDialog.bModal == 1) &&
		(pApp->tDialog.bCloseOnEscape == 1) &&
		(pApp->tDialog.bShowClose == 1) &&
		((pApp->pDialogWidget->iFlags & XGE_XUI_WIDGET_CLIP) != 0) &&
		(xgeXuiWidgetIsFocusable(pApp->pDialogWidget) != 0);
	xgeXuiDialogSetOpen(&pApp->tDialog, 0);

	xgeXuiPopupSetAutoClose(&pApp->tPopup, 1, 1);
	xgeXuiPopupSetOpen(&pApp->tPopup, 1);
	xgeXuiUpdate(&pApp->tXui, 0.0f);
	bPopupPolicyOK =
		(xgeXuiPopupIsOpen(&pApp->tPopup) != 0) &&
		(xgeXuiWidgetIsVisible(pApp->pPopupWidget) != 0) &&
		(xgeXuiWidgetIsFocusable(pApp->pPopupWidget) != 0) &&
		((pApp->pPopupWidget->iFlags & XGE_XUI_WIDGET_CLIP) != 0);
	tCenter = WidgetCenter(pApp->pPopupWidget);
	bPopupPolicyOK =
		bPopupPolicyOK &&
		(xgeXuiHitTest(&pApp->tXui, tCenter.fX, tCenter.fY) == pApp->pPopupWidget);
	tCenter = WidgetCenter(pApp->pPopupOwnerWidget);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_DOWN, XGE_MOUSE_LEFT, tCenter.fX, tCenter.fY);
	iRet = xgeXuiDispatchEvent(&pApp->tXui, &tEvent);
	bPopupPolicyOK =
		bPopupPolicyOK &&
		(iRet == XGE_XUI_EVENT_CONTINUE) &&
		(xgeXuiPopupIsOpen(&pApp->tPopup) != 0);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_DOWN, XGE_MOUSE_LEFT, 560.0f, 280.0f);
	iRet = xgeXuiDispatchEvent(&pApp->tXui, &tEvent);
	bPopupPolicyOK =
		bPopupPolicyOK &&
		(iRet == XGE_XUI_EVENT_CONSUMED) &&
		(xgeXuiPopupIsOpen(&pApp->tPopup) == 0) &&
		(pApp->iPopupCloseCount == 1);
	xgeXuiPopupSetOpen(&pApp->tPopup, 1);
	MakeKeyEvent(&tEvent, XGE_KEY_ESCAPE);
	iRet = xgeXuiDispatchEvent(&pApp->tXui, &tEvent);
	bPopupPolicyOK =
		bPopupPolicyOK &&
		(iRet == XGE_XUI_EVENT_CONSUMED) &&
		(xgeXuiPopupIsOpen(&pApp->tPopup) == 0) &&
		(pApp->iPopupCloseCount == 2);
	xgeXuiPopupSetAutoClose(&pApp->tPopup, 0, 1);
	xgeXuiPopupSetOpen(&pApp->tPopup, 1);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_DOWN, XGE_MOUSE_LEFT, 560.0f, 280.0f);
	iRet = xgeXuiDispatchEvent(&pApp->tXui, &tEvent);
	bPopupPolicyOK =
		bPopupPolicyOK &&
		(iRet == XGE_XUI_EVENT_CONTINUE) &&
		(xgeXuiPopupIsOpen(&pApp->tPopup) != 0);
	xgeXuiPopupSetOpen(&pApp->tPopup, 0);
	xgeXuiPopupSetAutoClose(&pApp->tPopup, 1, 1);
	pApp->bPopupOK = bPopupPolicyOK;

	{
		xge_xui_tooltip_desc_t tTooltipDesc;
		memset(&tTooltipDesc, 0, sizeof(tTooltipDesc));
		tTooltipDesc.iType = XGE_XUI_TOOLTIP_TEXT;
		tTooltipDesc.sText = "Tooltip text";
		tTooltipDesc.iAnchor = XGE_XUI_TOOLTIP_ANCHOR_WIDGET_BOTTOM;
		tTooltipDesc.fOffsetX = 5.0f;
		tTooltipDesc.fOffsetY = 3.0f;
		tTooltipDesc.fDelay = 0.0f;
		xgeXuiWidgetSetTooltip(pApp->pTooltipOwnerWidget, &tTooltipDesc);
	}
	tCenter = WidgetCenter(pApp->pTooltipOwnerWidget);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_MOVE, 0, tCenter.fX, tCenter.fY);
	iRet = xgeXuiDispatchEvent(&pApp->tXui, &tEvent);
	xgeXuiUpdate(&pApp->tXui, 0.0f);
	tRect = xgeXuiWidgetTooltipGetRect(&pApp->tXui);
	bTooltipPolicyOK =
		(iRet == XGE_XUI_EVENT_CONTINUE) &&
		(xgeXuiWidgetTooltipIsOpen(&pApp->tXui) != 0) &&
		(pApp->iTooltipOwnerCaptureCount == 1) &&
		FloatNear(tRect.fX, pApp->pTooltipOwnerWidget->tRect.fX + 5.0f, 0.01f) &&
		FloatNear(tRect.fY, pApp->pTooltipOwnerWidget->tRect.fY + pApp->pTooltipOwnerWidget->tRect.fH + 3.0f, 0.01f);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_MOVE, 0, 560.0f, 280.0f);
	xgeXuiDispatchEvent(&pApp->tXui, &tEvent);
	bTooltipPolicyOK =
		bTooltipPolicyOK &&
		(xgeXuiWidgetTooltipGetOwner(&pApp->tXui) == NULL);
	xgeXuiWidgetClearTooltip(pApp->pTooltipOwnerWidget);
	bTooltipPolicyOK =
		bTooltipPolicyOK &&
		(xgeXuiWidgetGetTooltip(pApp->pTooltipOwnerWidget)->iType == XGE_XUI_TOOLTIP_NONE);
	pApp->bTooltipOK =
		bTooltipPolicyOK &&
		(pApp->pTooltipOwnerWidget->procCaptureEvent == TooltipOwnerCapture);

	xgeXuiComboBoxSetFont(&pApp->tCombo, pApp->bFontReady ? &pApp->tFont : NULL);
	xgeXuiComboBoxSetItems(&pApp->tCombo, g_arrComboItems, (int)(sizeof(g_arrComboItems) / sizeof(g_arrComboItems[0])));
	xgeXuiComboBoxSetSelected(&pApp->tCombo, 1);
	xgeXuiComboBoxSetPopupHeight(&pApp->tCombo, 96.0f);
	xgeXuiComboBoxSetSelect(&pApp->tCombo, ComboSelect, pApp);
	xgeXuiComboBoxSetColors(&pApp->tCombo, XGE_COLOR_RGBA(52, 70, 94, 255), XGE_COLOR_RGBA(70, 92, 120, 255), XGE_COLOR_RGBA(96, 112, 142, 255), XGE_COLOR_RGBA(84, 86, 92, 180), XGE_COLOR_RGBA(248, 250, 252, 255), XGE_COLOR_RGBA(42, 50, 64, 255));
	tCenter = WidgetCenter(pApp->pComboWidget);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_DOWN, XGE_MOUSE_LEFT, tCenter.fX, tCenter.fY);
	iRet = xgeXuiDispatchEvent(&pApp->tXui, &tEvent);
	xgeXuiUpdate(&pApp->tXui, 0.0f);
	bComboPolicyOK =
		(iRet == XGE_XUI_EVENT_CONSUMED) &&
		(xgeXuiComboBoxIsOpen(&pApp->tCombo) != 0) &&
		(pApp->tXui.pFocus == pApp->tCombo.pListWidget);
	MakeMouseEvent(
		&tEvent,
		XGE_EVENT_MOUSE_DOWN,
		XGE_MOUSE_LEFT,
		pApp->tCombo.pListWidget->tContentRect.fX + 12.0f,
		pApp->tCombo.pListWidget->tContentRect.fY + pApp->tCombo.tList.tBase.fItemHeight * 2.0f + 6.0f);
	iRet = xgeXuiDispatchEvent(&pApp->tXui, &tEvent);
	bComboPolicyOK =
		bComboPolicyOK &&
		(iRet == XGE_XUI_EVENT_CONSUMED) &&
		(xgeXuiComboBoxGetSelected(&pApp->tCombo) == 2) &&
		(xgeXuiComboBoxIsOpen(&pApp->tCombo) == 0) &&
		(pApp->iComboSelectCount == 1) &&
		(pApp->iLastComboSelected == 2) &&
		(pApp->tXui.pFocus == pApp->pComboWidget);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_DOWN, XGE_MOUSE_LEFT, tCenter.fX, tCenter.fY);
	xgeXuiDispatchEvent(&pApp->tXui, &tEvent);
	xgeXuiUpdate(&pApp->tXui, 0.0f);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_DOWN, XGE_MOUSE_LEFT, 560.0f, 280.0f);
	iRet = xgeXuiDispatchEvent(&pApp->tXui, &tEvent);
	bComboPolicyOK =
		bComboPolicyOK &&
		(iRet == XGE_XUI_EVENT_CONSUMED) &&
		(xgeXuiComboBoxIsOpen(&pApp->tCombo) == 0);
	xgeXuiSetFocus(&pApp->tXui, pApp->pComboWidget);
	MakeKeyEvent(&tEvent, XGE_KEY_DOWN);
	iRet = xgeXuiDispatchEvent(&pApp->tXui, &tEvent);
	bComboPolicyOK =
		bComboPolicyOK &&
		(iRet == XGE_XUI_EVENT_CONSUMED) &&
		(xgeXuiComboBoxIsOpen(&pApp->tCombo) != 0);
	MakeKeyEvent(&tEvent, XGE_KEY_ESCAPE);
	iRet = xgeXuiDispatchEvent(&pApp->tXui, &tEvent);
	bComboPolicyOK =
		bComboPolicyOK &&
		(iRet == XGE_XUI_EVENT_CONSUMED) &&
		(xgeXuiComboBoxIsOpen(&pApp->tCombo) == 0);
	xgeXuiComboBoxSetItems(&pApp->tCombo, g_arrComboItems, 1);
	bComboPolicyOK =
		bComboPolicyOK &&
		(xgeXuiComboBoxGetSelected(&pApp->tCombo) == -1);
	xgeXuiComboBoxSetItems(&pApp->tCombo, g_arrComboItems, (int)(sizeof(g_arrComboItems) / sizeof(g_arrComboItems[0])));
	xgeXuiComboBoxSetSelected(&pApp->tCombo, 0);
	xgeXuiWidgetSetEnabled(pApp->pComboWidget, 0);
	bComboPolicyOK =
		bComboPolicyOK &&
		((xgeXuiComboBoxGetState(&pApp->tCombo) & XGE_XUI_STATE_DISABLED) != 0);
	xgeXuiWidgetSetEnabled(pApp->pComboWidget, 1);
	pApp->bComboOK = bComboPolicyOK;

	xgeXuiMenuSetItems(&pApp->tMenu, g_arrMenuItems, (int)(sizeof(g_arrMenuItems) / sizeof(g_arrMenuItems[0])));
	xgeXuiMenuSetFont(&pApp->tMenu, pApp->bFontReady ? &pApp->tFont : NULL);
	xgeXuiMenuSetSelect(&pApp->tMenu, MenuSelect, pApp);
	tMenuMetrics = pApp->tMenu.tMetrics;
	tMenuMetrics.fMinWidth = 168.0f;
	tMenuMetrics.fMaxHeight = 120.0f;
	xgeXuiMenuSetMetrics(&pApp->tMenu, &tMenuMetrics);
	tMenuColors = pApp->tMenu.tColors;
	tMenuColors.iPanel = XGE_COLOR_RGBA(42, 50, 64, 245);
	tMenuColors.iRow = XGE_COLOR_RGBA(52, 64, 82, 255);
	tMenuColors.iHover = XGE_COLOR_RGBA(72, 132, 208, 255);
	tMenuColors.iText = XGE_COLOR_RGBA(248, 250, 252, 255);
	tMenuColors.iDisabledText = XGE_COLOR_RGBA(128, 138, 150, 220);
	xgeXuiMenuSetColors(&pApp->tMenu, &tMenuColors);
	xgeXuiMenuOpenForOwner(&pApp->tMenu, pApp->pMenuOwnerWidget);
	xgeXuiUpdate(&pApp->tXui, 0.0f);
	bMenuPolicyOK =
		(xgeXuiMenuIsOpen(&pApp->tMenu) != 0) &&
		(pApp->tXui.pFocus == pApp->tMenu.pContentWidget);
	MakeMouseEvent(
		&tEvent,
		XGE_EVENT_MOUSE_DOWN,
		XGE_MOUSE_LEFT,
		pApp->tMenu.pContentWidget->tRect.fX + 12.0f,
		pApp->tMenu.pContentWidget->tRect.fY + pApp->tMenu.tMetrics.fPaddingY + pApp->tMenu.tMetrics.fItemHeight * 1.0f + 6.0f);
	iRet = xgeXuiDispatchEvent(&pApp->tXui, &tEvent);
	bMenuPolicyOK =
		bMenuPolicyOK &&
		(iRet == XGE_XUI_EVENT_CONSUMED) &&
		(xgeXuiMenuIsOpen(&pApp->tMenu) != 0) &&
		(pApp->iMenuSelectCount == 0);
	MakeMouseEvent(
		&tEvent,
		XGE_EVENT_MOUSE_DOWN,
		XGE_MOUSE_LEFT,
		pApp->tMenu.pContentWidget->tRect.fX + 12.0f,
		pApp->tMenu.pContentWidget->tRect.fY + pApp->tMenu.tMetrics.fPaddingY + pApp->tMenu.tMetrics.fItemHeight * 2.0f + 6.0f);
	iRet = xgeXuiDispatchEvent(&pApp->tXui, &tEvent);
	bMenuPolicyOK =
		bMenuPolicyOK &&
		(iRet == XGE_XUI_EVENT_CONSUMED) &&
		(xgeXuiMenuIsOpen(&pApp->tMenu) == 0) &&
		(pApp->iMenuSelectCount == 1) &&
		(pApp->iLastMenuSelected == 2) &&
		(pApp->tXui.pFocus == pApp->pMenuOwnerWidget);
	xgeXuiMenuOpenForOwner(&pApp->tMenu, pApp->pMenuOwnerWidget);
	xgeXuiUpdate(&pApp->tXui, 0.0f);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_DOWN, XGE_MOUSE_LEFT, 560.0f, 280.0f);
	iRet = xgeXuiDispatchEvent(&pApp->tXui, &tEvent);
	bMenuPolicyOK =
		bMenuPolicyOK &&
		(iRet == XGE_XUI_EVENT_CONSUMED) &&
		(xgeXuiMenuIsOpen(&pApp->tMenu) == 0);
	pApp->bMenuOK = bMenuPolicyOK;

	xgeXuiDialogSetTitle(&pApp->tDialog, pApp->bFontReady ? &pApp->tFont : NULL, "Overlay dialog");
	xgeXuiDialogSetClose(&pApp->tDialog, DialogClose, pApp);
	xgeXuiDialogSetColors(&pApp->tDialog, XGE_COLOR_RGBA(0, 0, 0, 120), XGE_COLOR_RGBA(42, 48, 60, 255), XGE_COLOR_RGBA(248, 250, 252, 255), XGE_COLOR_RGBA(212, 96, 96, 255));
	xgeXuiDialogSetOpen(&pApp->tDialog, 1);
	xgeXuiUpdate(&pApp->tXui, 0.0f);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_DOWN, XGE_MOUSE_LEFT, 560.0f, 280.0f);
	iRet = xgeXuiDispatchEvent(&pApp->tXui, &tEvent);
	bDialogPolicyOK =
		bDialogDefaultsOK &&
		(iRet == XGE_XUI_EVENT_CONSUMED) &&
		(xgeXuiDialogIsOpen(&pApp->tDialog) != 0) &&
		(pApp->tXui.pFocus == pApp->pDialogWidget);
	tRect = DialogCloseRect(&pApp->tDialog);
	pApp->tDialog.tCloseRect = tRect;
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_DOWN, XGE_MOUSE_LEFT, tRect.fX + 1.0f, tRect.fY + 1.0f);
	iRet = xgeXuiDispatchEvent(&pApp->tXui, &tEvent);
	bDialogPolicyOK =
		bDialogPolicyOK &&
		(iRet == XGE_XUI_EVENT_CONSUMED) &&
		(xgeXuiDialogIsOpen(&pApp->tDialog) == 0) &&
		(pApp->iDialogCloseCount == 1);
	xgeXuiDialogSetOpen(&pApp->tDialog, 1);
	xgeXuiDialogSetCloseOnEscape(&pApp->tDialog, 0);
	MakeKeyEvent(&tEvent, XGE_KEY_ESCAPE);
	iRet = xgeXuiDispatchEvent(&pApp->tXui, &tEvent);
	bDialogPolicyOK =
		bDialogPolicyOK &&
		(iRet == XGE_XUI_EVENT_CONSUMED) &&
		(xgeXuiDialogIsOpen(&pApp->tDialog) != 0) &&
		(pApp->iDialogCloseCount == 1);
	xgeXuiDialogSetModal(&pApp->tDialog, 0);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_DOWN, XGE_MOUSE_LEFT, 560.0f, 280.0f);
	iRet = xgeXuiDispatchEvent(&pApp->tXui, &tEvent);
	bDialogPolicyOK =
		bDialogPolicyOK &&
		(iRet == XGE_XUI_EVENT_CONTINUE) &&
		(xgeXuiDialogIsOpen(&pApp->tDialog) != 0);
	xgeXuiDialogSetModal(&pApp->tDialog, 1);
	xgeXuiDialogSetShowClose(&pApp->tDialog, 0);
	MakeMouseEvent(
		&tEvent,
		XGE_EVENT_MOUSE_DOWN,
		XGE_MOUSE_LEFT,
		pApp->pDialogWidget->tContentRect.fX + pApp->pDialogWidget->tContentRect.fW - 8.0f,
		pApp->pDialogWidget->tContentRect.fY + 8.0f);
	iRet = xgeXuiDispatchEvent(&pApp->tXui, &tEvent);
	bDialogPolicyOK =
		bDialogPolicyOK &&
		(iRet == XGE_XUI_EVENT_CONSUMED) &&
		(xgeXuiDialogIsOpen(&pApp->tDialog) != 0);
	xgeXuiDialogSetShowClose(&pApp->tDialog, 1);
	xgeXuiDialogSetCloseOnEscape(&pApp->tDialog, 1);
	xgeXuiDialogSetOpen(&pApp->tDialog, 0);
	pApp->bDialogOK = bDialogPolicyOK;

	pApp->bPolicyOK = bPopupPolicyOK && bMenuPolicyOK && bDialogPolicyOK;
	UpdateStatus(pApp);
	return XGE_OK;
}

static int AppInit(app_state_t* pApp, int iFrameLimit)
{
	memset(pApp, 0, sizeof(*pApp));
	pApp->iFrameLimit = iFrameLimit;
	pApp->iLastComboSelected = -1;
	pApp->iLastMenuSelected = -1;
	if ( xgeXuiInit(&pApp->tXui) != XGE_OK ) {
		return XGE_ERROR;
	}
	(void)LoadFont(pApp);
	return RunStaticChecks(pApp);
}

static void AppUnit(app_state_t* pApp)
{
	xgeXuiLabelUnit(&pApp->tDialogLabel);
	xgeXuiDialogUnit(&pApp->tDialog);
	xgeXuiMenuUnit(&pApp->tMenu);
	xgeXuiComboBoxUnit(&pApp->tCombo);
	xgeXuiLabelUnit(&pApp->tPopupLabel);
	xgeXuiPopupUnit(&pApp->tPopup);
	xgeXuiLabelUnit(&pApp->tMenuOwnerLabel);
	xgeXuiLabelUnit(&pApp->tTooltipOwnerLabel);
	xgeXuiLabelUnit(&pApp->tPopupOwnerLabel);
	xgeXuiLabelUnit(&pApp->tStatusLabel);
	if ( pApp->bFontReady ) {
		xgeFontFree(&pApp->tFont);
	}
	xgeXuiUnit(&pApp->tXui);
}

static int AppFrame(void* pUser)
{
	app_state_t* pApp;

	pApp = (app_state_t*)pUser;
	LayoutRoot(pApp);
	UpdateStatus(pApp);
	xgeXuiUpdate(&pApp->tXui, xgeGetDelta());
	xgeBegin();
	xgeClear(XGE_COLOR_RGBA(18, 22, 30, 255));
	xgeXuiPaint(&pApp->tXui);
	xgeEnd();
	xgePresent();

	pApp->iFrameCount++;
	if ( (pApp->iFrameLimit > 0) && (pApp->iFrameCount >= pApp->iFrameLimit) ) {
		printf(
			"xui-overlay-menu-lab final-summary frames=%d popup=%d tooltip=%d combo=%d menu=%d dialog=%d policy=%d popup(close=%d open=%d) tooltip(open=%d hook=%d) combo(selected=%d cb=%d state=%d changes=%d) menu(last=%d cb=%d open=%d) dialog(close=%d open=%d modal=%d)\n",
			pApp->iFrameCount,
			pApp->bPopupOK,
			pApp->bTooltipOK,
			pApp->bComboOK,
			pApp->bMenuOK,
			pApp->bDialogOK,
			pApp->bPolicyOK,
			pApp->iPopupCloseCount,
			xgeXuiPopupIsOpen(&pApp->tPopup),
			xgeXuiWidgetTooltipIsOpen(&pApp->tXui),
			pApp->iTooltipOwnerCaptureCount,
			xgeXuiComboBoxGetSelected(&pApp->tCombo),
			pApp->iComboSelectCount,
			xgeXuiComboBoxGetState(&pApp->tCombo),
			pApp->tCombo.iChangeCount,
			pApp->iLastMenuSelected,
			pApp->iMenuSelectCount,
			xgeXuiMenuIsOpen(&pApp->tMenu),
			pApp->iDialogCloseCount,
			xgeXuiDialogIsOpen(&pApp->tDialog),
			pApp->tDialog.bModal);
		printf("xui-overlay-menu-lab summary frames=%d/%d\n", pApp->iFrameCount, pApp->iFrameLimit);
		xgeQuit();
	}
	return 0;
}

int main(int argc, char** argv)
{
	xge_desc_t tDesc;
	app_state_t tApp;
	int iFrameLimit;
	int iExitCode;
	int i;

	memset(&tDesc, 0, sizeof(tDesc));
	iFrameLimit = ArgInt(getenv("XGE_XUI_OVERLAY_MENU_FRAMES"), 0);
	for ( i = 1; i < argc; i++ ) {
		if ( (strcmp(argv[i], "--frames") == 0) && ((i + 1) < argc) ) {
			iFrameLimit = ArgInt(argv[++i], iFrameLimit);
		}
	}

	tDesc.iWidth = 640;
	tDesc.iHeight = 380;
	tDesc.sTitle = "XGE XUI Overlay Menu Lab";
	tDesc.iFlags = XGE_INIT_WINDOW | XGE_INIT_VSYNC;
	tDesc.iRunMode = XGE_RUN_GAME_LOOP;
	tDesc.iTargetFPS = 60;
	if ( xgeInit(&tDesc) != XGE_OK ) {
		return 1;
	}
	if ( AppInit(&tApp, iFrameLimit) != XGE_OK ) {
		printf("xui-overlay-menu-lab init failed\n");
		xgeUnit();
		return 2;
	}
	xgeRun(AppFrame, &tApp);
	iExitCode =
		(tApp.bPopupOK && tApp.bTooltipOK && tApp.bComboOK &&
		 tApp.bMenuOK && tApp.bDialogOK && tApp.bPolicyOK) ? 0 : 3;
	AppUnit(&tApp);
	xgeUnit();
	return iExitCode;
}

