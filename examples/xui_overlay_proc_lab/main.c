#include "../../xge.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct app_state_t {
	xge_xui_context_t tXui;
	xge_font_t tFont;
	xge_xui_widget pRootPanel;
	xge_xui_widget pStatusWidget;
	xge_xui_widget pPopupOwnerWidget;
	xge_xui_widget pComboWidget;
	xge_xui_widget pPopupWidget;
	xge_xui_widget pPopupLabelWidget;
	xge_xui_widget pDialogWidget;
	xge_xui_widget pDialogLabelWidget;
	xge_xui_label_t tStatusLabel;
	xge_xui_label_t tPopupLabel;
	xge_xui_label_t tDialogLabel;
	xge_xui_popup_t tPopup;
	xge_xui_combo_box_t tCombo;
	xge_xui_dialog_t tDialog;
	int bFontReady;
	int iFrameLimit;
	int iFrameCount;
	int iLastWidth;
	int iLastHeight;
	int iPopupCloseCount;
	int iComboSelectCount;
	int iDialogCloseCount;
	int iLastComboSelected;
	int bPopupOK;
	int bComboOK;
	int bListProcOK;
	int bDialogOK;
	int bDirectPaintOK;
} app_state_t;

static const char* g_arrComboItems[] = {
	"North Dock",
	"Workshop",
	"Archive Wing",
	"Signal Room"
};

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

	tCenter.fX = pWidget->tRect.fX + pWidget->tRect.fW * 0.5f;
	tCenter.fY = pWidget->tRect.fY + pWidget->tRect.fH * 0.5f;
	return tCenter;
}

static xge_rect_t DialogCloseRect(xge_xui_dialog pDialog)
{
	xge_rect_t tClose;

	memset(&tClose, 0, sizeof(tClose));
	if ( (pDialog == NULL) || (pDialog->pWidget == NULL) ) {
		return tClose;
	}
	tClose.fW = 18.0f;
	tClose.fH = 18.0f;
	tClose.fX = pDialog->pWidget->tContentRect.fX + pDialog->pWidget->tContentRect.fW - tClose.fW - 2.0f;
	tClose.fY = pDialog->pWidget->tContentRect.fY + 3.0f;
	return tClose;
}

static int LoadFont(app_state_t* pApp)
{
	const char* arrFonts[] = {
		"C:/Windows/Fonts/simhei.ttf",
		"C:/Windows/Fonts/Deng.ttf",
		"C:/Windows/Fonts/msyh.ttc",
		"C:/Windows/Fonts/arial.ttf"
	};
	int i;

	for ( i = 0; i < (int)(sizeof(arrFonts) / sizeof(arrFonts[0])); i++ ) {
		memset(&pApp->tFont, 0, sizeof(pApp->tFont));
		if ( xgeFontLoad(&pApp->tFont, arrFonts[i], 18.0f) == XGE_OK ) {
			pApp->bFontReady = 1;
			printf("xui-overlay-proc-lab font loaded: %s\n", arrFonts[i]);
			return XGE_OK;
		}
	}
	printf("xui-overlay-proc-lab font load failed\n");
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

static void DialogClose(xge_xui_widget pWidget, void* pUser)
{
	app_state_t* pApp;

	(void)pWidget;
	pApp = (app_state_t*)pUser;
	if ( pApp != NULL ) {
		pApp->iDialogCloseCount++;
	}
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
	tRoot.fX = 18.0f;
	tRoot.fY = 18.0f;
	tRoot.fW = (float)iWidth - 36.0f;
	tRoot.fH = (float)iHeight - 36.0f;
	if ( tRoot.fW < 760.0f ) {
		tRoot.fW = 760.0f;
	}
	if ( tRoot.fH < 380.0f ) {
		tRoot.fH = 380.0f;
	}
	xgeXuiWidgetSetRect(pApp->pRootPanel, tRoot);
	xgeXuiWidgetSetRect(pApp->pStatusWidget, (xge_rect_t){ 0.0f, 0.0f, tRoot.fW, 40.0f });
	xgeXuiWidgetSetRect(pApp->pPopupOwnerWidget, (xge_rect_t){ 24.0f, 72.0f, 180.0f, 38.0f });
	xgeXuiWidgetSetRect(pApp->pComboWidget, (xge_rect_t){ 260.0f, 72.0f, 210.0f, 38.0f });
	xgeXuiWidgetSetRect(pApp->pPopupWidget, (xge_rect_t){ 24.0f, 126.0f, 196.0f, 96.0f });
	xgeXuiWidgetSetRect(pApp->pPopupLabelWidget, (xge_rect_t){ 12.0f, 12.0f, 160.0f, 56.0f });
	xgeXuiWidgetSetRect(pApp->pDialogWidget, (xge_rect_t){ 520.0f, 62.0f, 220.0f, 156.0f });
	xgeXuiWidgetSetRect(pApp->pDialogLabelWidget, (xge_rect_t){ 14.0f, 38.0f, 176.0f, 72.0f });
	pApp->iLastWidth = iWidth;
	pApp->iLastHeight = iHeight;
}

static void UpdateStatus(app_state_t* pApp)
{
	char sText[256];

	snprintf(
		sText,
		sizeof(sText),
		"popup=%d combo=%d list=%d dialog=%d paint=%d close=%d/%d select=%d last=%d",
		pApp->bPopupOK,
		pApp->bComboOK,
		pApp->bListProcOK,
		pApp->bDialogOK,
		pApp->bDirectPaintOK,
		pApp->iPopupCloseCount,
		pApp->iDialogCloseCount,
		pApp->iComboSelectCount,
		pApp->iLastComboSelected);
	xgeXuiLabelSetText(&pApp->tStatusLabel, sText);
}

static int CreateUI(app_state_t* pApp)
{
	xge_xui_theme_t tTheme;
	xge_xui_widget pOverlayRoot;
	xge_xui_widget pRoot;
	xge_font pFont;

	pRoot = xgeXuiRoot(&pApp->tXui);
	pOverlayRoot = xgeXuiOverlayRoot(&pApp->tXui);
	pFont = pApp->bFontReady ? &pApp->tFont : NULL;
	if ( (pRoot == NULL) || (pOverlayRoot == NULL) ) {
		return XGE_ERROR;
	}

	xgeXuiThemeDefault(&tTheme);
	tTheme.pFont = pFont;
	tTheme.iTextColor = XGE_COLOR_RGBA(238, 242, 248, 255);
	tTheme.iBackgroundColor = XGE_COLOR_RGBA(18, 22, 30, 255);
	tTheme.iPanelColor = XGE_COLOR_RGBA(34, 40, 52, 255);
	tTheme.iBorderColor = XGE_COLOR_RGBA(94, 102, 120, 255);
	tTheme.iAccentColor = XGE_COLOR_RGBA(104, 196, 255, 255);
	tTheme.iStateNormal = XGE_COLOR_RGBA(54, 92, 148, 255);
	tTheme.iStateHover = XGE_COLOR_RGBA(72, 118, 182, 255);
	tTheme.iStateActive = XGE_COLOR_RGBA(42, 74, 126, 255);
	tTheme.iStateFocus = XGE_COLOR_RGBA(88, 106, 140, 255);
	tTheme.iStateDisabled = XGE_COLOR_RGBA(84, 88, 94, 180);
	tTheme.fRadius = 8.0f;
	tTheme.fPadding = 8.0f;
	tTheme.fSpacing = 10.0f;
	xgeXuiSetTheme(&pApp->tXui, &tTheme);

	pApp->pRootPanel = xgeXuiWidgetCreate();
	pApp->pStatusWidget = xgeXuiWidgetCreate();
	pApp->pPopupOwnerWidget = xgeXuiWidgetCreate();
	pApp->pComboWidget = xgeXuiWidgetCreate();
	pApp->pPopupWidget = xgeXuiWidgetCreate();
	pApp->pPopupLabelWidget = xgeXuiWidgetCreate();
	pApp->pDialogWidget = xgeXuiWidgetCreate();
	pApp->pDialogLabelWidget = xgeXuiWidgetCreate();
	if ( (pApp->pRootPanel == NULL) || (pApp->pStatusWidget == NULL) || (pApp->pPopupOwnerWidget == NULL) ||
	     (pApp->pComboWidget == NULL) || (pApp->pPopupWidget == NULL) || (pApp->pPopupLabelWidget == NULL) ||
	     (pApp->pDialogWidget == NULL) || (pApp->pDialogLabelWidget == NULL) ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}

	xgeXuiWidgetAdd(pRoot, pApp->pRootPanel);
	xgeXuiWidgetAdd(pApp->pRootPanel, pApp->pStatusWidget);
	xgeXuiWidgetAdd(pApp->pRootPanel, pApp->pPopupOwnerWidget);
	xgeXuiWidgetAdd(pApp->pRootPanel, pApp->pComboWidget);
	xgeXuiWidgetAdd(pOverlayRoot, pApp->pPopupWidget);
	xgeXuiWidgetAdd(pApp->pPopupWidget, pApp->pPopupLabelWidget);
	xgeXuiWidgetAdd(pOverlayRoot, pApp->pDialogWidget);
	xgeXuiWidgetAdd(pApp->pDialogWidget, pApp->pDialogLabelWidget);

	xgeXuiWidgetSetBackground(pApp->pRootPanel, XGE_COLOR_RGBA(18, 22, 30, 255));
	xgeXuiWidgetSetRadius(pApp->pRootPanel, 8.0f);
	xgeXuiWidgetSetBackground(pApp->pStatusWidget, XGE_COLOR_RGBA(44, 56, 74, 255));
	xgeXuiWidgetSetPaddingPx(pApp->pStatusWidget, 10.0f, 8.0f, 10.0f, 8.0f);
	xgeXuiWidgetSetBackground(pApp->pPopupOwnerWidget, XGE_COLOR_RGBA(54, 94, 146, 255));
	xgeXuiWidgetSetRadius(pApp->pPopupOwnerWidget, 8.0f);

	if ( xgeXuiLabelInit(&pApp->tStatusLabel, pApp->pStatusWidget, pFont, "xui overlay proc lab") != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiLabelSetColor(&pApp->tStatusLabel, XGE_COLOR_RGBA(248, 250, 252, 255));

	if ( xgeXuiLabelInit(&pApp->tPopupLabel, pApp->pPopupLabelWidget, pFont, "Popup Event/EventProc\noutside close + ESC close") != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiLabelSetColor(&pApp->tPopupLabel, XGE_COLOR_RGBA(250, 250, 252, 255));

	if ( xgeXuiLabelInit(&pApp->tDialogLabel, pApp->pDialogLabelWidget, pFont, "Dialog Event/EventProc\nmodal + close glyph") != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiLabelSetColor(&pApp->tDialogLabel, XGE_COLOR_RGBA(248, 250, 252, 255));

	if ( xgeXuiPopupInit(&pApp->tPopup, &pApp->tXui, pApp->pPopupWidget) != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiPopupSetOwner(&pApp->tPopup, pApp->pPopupOwnerWidget);
	xgeXuiPopupSetClose(&pApp->tPopup, PopupClose, pApp);
	xgeXuiPopupSetBackground(&pApp->tPopup, XGE_COLOR_RGBA(42, 50, 64, 246));
	xgeXuiWidgetSetZ(pApp->pPopupWidget, 1010);

	if ( xgeXuiComboBoxInit(&pApp->tCombo, &pApp->tXui, pApp->pComboWidget) != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiComboBoxSetFont(&pApp->tCombo, pFont);
	xgeXuiComboBoxSetItems(&pApp->tCombo, g_arrComboItems, (int)(sizeof(g_arrComboItems) / sizeof(g_arrComboItems[0])));
	xgeXuiComboBoxSetSelected(&pApp->tCombo, 1);
	xgeXuiComboBoxSetDropDownHeight(&pApp->tCombo, 96.0f);
	xgeXuiComboBoxSetSelect(&pApp->tCombo, ComboSelect, pApp);
	xgeXuiComboBoxSetColors(&pApp->tCombo, XGE_COLOR_RGBA(50, 70, 98, 255), XGE_COLOR_RGBA(68, 96, 130, 255), XGE_COLOR_RGBA(40, 60, 84, 255), XGE_COLOR_RGBA(92, 104, 132, 255), XGE_COLOR_RGBA(82, 84, 92, 180), XGE_COLOR_RGBA(248, 250, 252, 255), XGE_COLOR_RGBA(40, 48, 62, 248));

	if ( xgeXuiDialogInit(&pApp->tDialog, &pApp->tXui, pApp->pDialogWidget) != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiDialogSetTitle(&pApp->tDialog, pFont, "Overlay Proc");
	xgeXuiDialogSetClose(&pApp->tDialog, DialogClose, pApp);
	xgeXuiDialogSetColors(&pApp->tDialog, XGE_COLOR_RGBA(0, 0, 0, 120), XGE_COLOR_RGBA(40, 48, 60, 255), XGE_COLOR_RGBA(248, 250, 252, 255), XGE_COLOR_RGBA(214, 100, 100, 255));
	xgeXuiDialogSetOpen(&pApp->tDialog, 0);
	xgeXuiWidgetSetZ(pApp->pDialogWidget, 1050);

	return XGE_OK;
}

static void PrepareShowcaseState(app_state_t* pApp)
{
	xge_event_t tEvent;
	xge_vec2_t tCenter;

	xgeXuiPopupSetAutoClose(&pApp->tPopup, 1, 1);
	xgeXuiPopupSetOpen(&pApp->tPopup, 1);
	xgeXuiDialogSetColors(&pApp->tDialog, XGE_COLOR_RGBA(0, 0, 0, 0), XGE_COLOR_RGBA(40, 48, 60, 255), XGE_COLOR_RGBA(248, 250, 252, 255), XGE_COLOR_RGBA(214, 100, 100, 255));
	xgeXuiDialogSetModal(&pApp->tDialog, 0);
	xgeXuiDialogSetShowClose(&pApp->tDialog, 1);
	xgeXuiDialogSetCloseOnEscape(&pApp->tDialog, 1);
	xgeXuiDialogSetOpen(&pApp->tDialog, 1);
	xgeXuiSetFocus(&pApp->tXui, pApp->pComboWidget);
	tCenter = WidgetCenter(pApp->pComboWidget);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_DOWN, XGE_MOUSE_LEFT, tCenter.fX, tCenter.fY);
	(void)xgeXuiComboBoxEvent(&pApp->tCombo, &tEvent);
	xgeXuiUpdate(&pApp->tXui, 0.0f);
}

static int RunStaticChecks(app_state_t* pApp)
{
	xge_event_t tEvent;
	xge_vec2_t tCenter;
	xge_rect_t tClose;
	int iRet;
	int bPopupOK;
	int bComboOK;
	int bListOK;
	int bDialogOK;

	LayoutRoot(pApp);
	xgeXuiUpdate(&pApp->tXui, 0.0f);

	xgeXuiPopupSetAutoClose(&pApp->tPopup, 1, 1);
	xgeXuiPopupSetOpen(&pApp->tPopup, 1);
	tCenter = WidgetCenter(pApp->pPopupOwnerWidget);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_DOWN, XGE_MOUSE_LEFT, tCenter.fX, tCenter.fY);
	iRet = xgeXuiPopupEvent(&pApp->tPopup, &tEvent);
	bPopupOK =
		(pApp->pPopupWidget->procEvent == xgeXuiPopupEventProc) &&
		(pApp->pPopupWidget->procPaint == xgeXuiPopupPaintProc) &&
		(iRet == XGE_XUI_EVENT_CONTINUE) &&
		(xgeXuiPopupIsOpen(&pApp->tPopup) != 0);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_DOWN, XGE_MOUSE_LEFT, 720.0f, 320.0f);
	iRet = xgeXuiPopupEventProc(pApp->pPopupWidget, &tEvent, &pApp->tPopup);
	bPopupOK =
		bPopupOK &&
		(iRet == XGE_XUI_EVENT_CONSUMED) &&
		(xgeXuiPopupIsOpen(&pApp->tPopup) == 0) &&
		(pApp->iPopupCloseCount == 1);
	xgeXuiPopupSetOpen(&pApp->tPopup, 1);
	MakeKeyEvent(&tEvent, XGE_KEY_ESCAPE);
	iRet = xgeXuiPopupEventProc(pApp->pPopupWidget, &tEvent, &pApp->tPopup);
	bPopupOK =
		bPopupOK &&
		(iRet == XGE_XUI_EVENT_CONSUMED) &&
		(xgeXuiPopupIsOpen(&pApp->tPopup) == 0) &&
		(pApp->iPopupCloseCount == 2);
	xgeXuiPopupSetAutoClose(&pApp->tPopup, 0, 1);
	xgeXuiPopupSetOpen(&pApp->tPopup, 1);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_DOWN, XGE_MOUSE_LEFT, 720.0f, 320.0f);
	iRet = xgeXuiPopupEvent(&pApp->tPopup, &tEvent);
	bPopupOK =
		bPopupOK &&
		(iRet == XGE_XUI_EVENT_CONTINUE) &&
		(xgeXuiPopupIsOpen(&pApp->tPopup) != 0);
	xgeXuiPopupSetOpen(&pApp->tPopup, 0);
	xgeXuiPopupSetAutoClose(&pApp->tPopup, 1, 1);
	pApp->bPopupOK = bPopupOK;

	tCenter = WidgetCenter(pApp->pComboWidget);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_MOVE, 0, tCenter.fX, tCenter.fY);
	iRet = xgeXuiComboBoxEvent(&pApp->tCombo, &tEvent);
	bComboOK =
		(iRet == XGE_XUI_EVENT_CONTINUE) &&
		((xgeXuiComboBoxGetState(&pApp->tCombo) & XGE_XUI_STATE_HOVER) != 0);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_DOWN, XGE_MOUSE_LEFT, tCenter.fX, tCenter.fY);
	iRet = xgeXuiComboBoxEvent(&pApp->tCombo, &tEvent);
	xgeXuiUpdate(&pApp->tXui, 0.0f);
	bComboOK =
		bComboOK &&
		(pApp->pComboWidget->procEvent == xgeXuiComboBoxEventProc) &&
		(pApp->pComboWidget->procPaint == xgeXuiComboBoxPaintProc) &&
		(iRet == XGE_XUI_EVENT_CONSUMED) &&
		(xgeXuiComboBoxIsOpen(&pApp->tCombo) != 0);
	MakeMouseEvent(
		&tEvent,
		XGE_EVENT_MOUSE_DOWN,
		XGE_MOUSE_LEFT,
		pApp->tCombo.pListWidget->tContentRect.fX + 10.0f,
		pApp->tCombo.pListWidget->tContentRect.fY + pApp->tCombo.tList.fItemHeight * 2.0f + 6.0f);
	iRet = xgeXuiListViewEventProc(pApp->tCombo.pListWidget, &tEvent, &pApp->tCombo.tList);
	bListOK =
		(pApp->tCombo.pListWidget->procEvent == xgeXuiListViewEventProc) &&
		(pApp->tCombo.pListWidget->procPaint == xgeXuiListViewPaintProc) &&
		(iRet == XGE_XUI_EVENT_CONSUMED) &&
		(xgeXuiComboBoxGetSelected(&pApp->tCombo) == 2) &&
		(xgeXuiListViewGetSelected(&pApp->tCombo.tList) == 2) &&
		(xgeXuiComboBoxIsOpen(&pApp->tCombo) == 0) &&
		(pApp->iComboSelectCount == 1) &&
		(pApp->iLastComboSelected == 2);
	xgeXuiSetFocus(&pApp->tXui, pApp->pComboWidget);
	MakeKeyEvent(&tEvent, XGE_KEY_DOWN);
	iRet = xgeXuiComboBoxEventProc(pApp->pComboWidget, &tEvent, &pApp->tCombo);
	xgeXuiUpdate(&pApp->tXui, 0.0f);
	bComboOK =
		bComboOK &&
		(iRet == XGE_XUI_EVENT_CONSUMED) &&
		(xgeXuiComboBoxIsOpen(&pApp->tCombo) != 0);
	MakeKeyEvent(&tEvent, XGE_KEY_ESCAPE);
	iRet = xgeXuiPopupEvent(&pApp->tCombo.tPopup, &tEvent);
	bComboOK =
		bComboOK &&
		(iRet == XGE_XUI_EVENT_CONSUMED) &&
		(xgeXuiComboBoxIsOpen(&pApp->tCombo) == 0);
	xgeXuiWidgetSetEnabled(pApp->pComboWidget, 0);
	bComboOK =
		bComboOK &&
		((xgeXuiComboBoxGetState(&pApp->tCombo) & XGE_XUI_STATE_DISABLED) != 0);
	xgeXuiWidgetSetEnabled(pApp->pComboWidget, 1);
	pApp->bComboOK = bComboOK;
	pApp->bListProcOK = bListOK;

	xgeXuiDialogSetOpen(&pApp->tDialog, 1);
	xgeXuiDialogSetModal(&pApp->tDialog, 1);
	xgeXuiDialogSetCloseOnEscape(&pApp->tDialog, 1);
	xgeXuiDialogSetShowClose(&pApp->tDialog, 1);
	xgeXuiUpdate(&pApp->tXui, 0.0f);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_DOWN, XGE_MOUSE_LEFT, 64.0f, 306.0f);
	iRet = xgeXuiDialogEvent(&pApp->tDialog, &tEvent);
	bDialogOK =
		(pApp->pDialogWidget->procEvent == xgeXuiDialogEventProc) &&
		(pApp->pDialogWidget->procPaint == xgeXuiDialogPaintProc) &&
		(iRet == XGE_XUI_EVENT_CONSUMED) &&
		(xgeXuiDialogIsOpen(&pApp->tDialog) != 0);
	tClose = DialogCloseRect(&pApp->tDialog);
	pApp->tDialog.tCloseRect = tClose;
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_DOWN, XGE_MOUSE_LEFT, tClose.fX + 1.0f, tClose.fY + 1.0f);
	iRet = xgeXuiDialogEventProc(pApp->pDialogWidget, &tEvent, &pApp->tDialog);
	bDialogOK =
		bDialogOK &&
		(iRet == XGE_XUI_EVENT_CONSUMED) &&
		(xgeXuiDialogIsOpen(&pApp->tDialog) == 0) &&
		(pApp->iDialogCloseCount == 1);
	xgeXuiDialogSetOpen(&pApp->tDialog, 1);
	xgeXuiDialogSetCloseOnEscape(&pApp->tDialog, 0);
	MakeKeyEvent(&tEvent, XGE_KEY_ESCAPE);
	iRet = xgeXuiDialogEvent(&pApp->tDialog, &tEvent);
	bDialogOK =
		bDialogOK &&
		(iRet == XGE_XUI_EVENT_CONSUMED) &&
		(xgeXuiDialogIsOpen(&pApp->tDialog) != 0);
	xgeXuiDialogSetModal(&pApp->tDialog, 0);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_DOWN, XGE_MOUSE_LEFT, 64.0f, 306.0f);
	iRet = xgeXuiDialogEventProc(pApp->pDialogWidget, &tEvent, &pApp->tDialog);
	bDialogOK =
		bDialogOK &&
		(iRet == XGE_XUI_EVENT_CONTINUE) &&
		(xgeXuiDialogIsOpen(&pApp->tDialog) != 0);
	xgeXuiDialogSetShowClose(&pApp->tDialog, 0);
	MakeMouseEvent(
		&tEvent,
		XGE_EVENT_MOUSE_DOWN,
		XGE_MOUSE_LEFT,
		pApp->pDialogWidget->tContentRect.fX + pApp->pDialogWidget->tContentRect.fW - 8.0f,
		pApp->pDialogWidget->tContentRect.fY + 8.0f);
	iRet = xgeXuiDialogEvent(&pApp->tDialog, &tEvent);
	bDialogOK =
		bDialogOK &&
		(iRet == XGE_XUI_EVENT_CONSUMED) &&
		(xgeXuiDialogIsOpen(&pApp->tDialog) != 0);
	xgeXuiDialogSetOpen(&pApp->tDialog, 0);
	pApp->bDialogOK = bDialogOK;

	PrepareShowcaseState(pApp);
	UpdateStatus(pApp);
	return XGE_OK;
}

static int AppInit(app_state_t* pApp, int iFrameLimit)
{
	memset(pApp, 0, sizeof(*pApp));
	pApp->iFrameLimit = iFrameLimit;
	pApp->iLastComboSelected = -1;
	if ( xgeXuiInit(&pApp->tXui) != XGE_OK ) {
		return XGE_ERROR;
	}
	if ( LoadFont(pApp) != XGE_OK ) {
		return XGE_ERROR;
	}
	if ( CreateUI(pApp) != XGE_OK ) {
		return XGE_ERROR;
	}
	return RunStaticChecks(pApp);
}

static void AppUnit(app_state_t* pApp)
{
	xgeXuiLabelUnit(&pApp->tDialogLabel);
	xgeXuiDialogUnit(&pApp->tDialog);
	xgeXuiComboBoxUnit(&pApp->tCombo);
	xgeXuiLabelUnit(&pApp->tPopupLabel);
	xgeXuiPopupUnit(&pApp->tPopup);
	xgeXuiLabelUnit(&pApp->tStatusLabel);
	xgeXuiUnit(&pApp->tXui);
	if ( pApp->bFontReady ) {
		xgeFontFree(&pApp->tFont);
	}
	memset(pApp, 0, sizeof(*pApp));
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
	if ( pApp->bDirectPaintOK == 0 ) {
		xgeXuiPopupPaintProc(pApp->pPopupWidget, &pApp->tPopup);
		xgeXuiComboBoxPaintProc(pApp->pComboWidget, &pApp->tCombo);
		xgeXuiListViewPaintProc(pApp->tCombo.pListWidget, &pApp->tCombo.tList);
		xgeXuiDialogPaintProc(pApp->pDialogWidget, &pApp->tDialog);
		pApp->bDirectPaintOK = 1;
	}
	xgeXuiPaint(&pApp->tXui);
	xgeEnd();
	xgePresent();

	pApp->iFrameCount++;
	if ( pApp->iFrameCount >= pApp->iFrameLimit ) {
		printf(
			"xui-overlay-proc-lab final-summary frames=%d popup=%d combo=%d list=%d dialog=%d paint=%d popup(close=%d open=%d) combo(selected=%d open=%d state=%d cb=%d) list(selected=%d scroll=%.2f) dialog(close=%d open=%d modal=%d showclose=%d)\n",
			pApp->iFrameCount,
			pApp->bPopupOK,
			pApp->bComboOK,
			pApp->bListProcOK,
			pApp->bDialogOK,
			pApp->bDirectPaintOK,
			pApp->iPopupCloseCount,
			xgeXuiPopupIsOpen(&pApp->tPopup),
			xgeXuiComboBoxGetSelected(&pApp->tCombo),
			xgeXuiComboBoxIsOpen(&pApp->tCombo),
			xgeXuiComboBoxGetState(&pApp->tCombo),
			pApp->iComboSelectCount,
			xgeXuiListViewGetSelected(&pApp->tCombo.tList),
			xgeXuiListViewGetScroll(&pApp->tCombo.tList),
			pApp->iDialogCloseCount,
			xgeXuiDialogIsOpen(&pApp->tDialog),
			pApp->tDialog.bModal,
			pApp->tDialog.bShowClose);
		printf("xui-overlay-proc-lab summary frames=%d/%d\n", pApp->iFrameCount, pApp->iFrameLimit);
		xgeQuit();
	}
	return 0;
}

int main(int argc, char** argv)
{
	xge_desc_t tDesc;
	app_state_t tApp;
	int iFrameLimit;
	int i;
	int iExitCode;

	iFrameLimit = 180;
	for ( i = 1; i < argc; i++ ) {
		if ( strcmp(argv[i], "--frames") == 0 && (i + 1) < argc ) {
			iFrameLimit = ArgInt(argv[i + 1], iFrameLimit);
			i++;
		}
	}

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iWidth = 780;
	tDesc.iHeight = 420;
	tDesc.sTitle = "XGE XUI Overlay Proc Lab";
	tDesc.iFlags = XGE_INIT_WINDOW | XGE_INIT_VSYNC;
	tDesc.iRunMode = XGE_RUN_GAME_LOOP;
	tDesc.iTargetFPS = 60;
	if ( xgeInit(&tDesc) != XGE_OK ) {
		return 1;
	}
	if ( AppInit(&tApp, iFrameLimit) != XGE_OK ) {
		AppUnit(&tApp);
		xgeUnit();
		return 2;
	}
	xgeRun(AppFrame, &tApp);
	iExitCode = (tApp.bPopupOK && tApp.bComboOK && tApp.bListProcOK && tApp.bDialogOK && tApp.bDirectPaintOK) ? 0 : 3;
	AppUnit(&tApp);
	xgeUnit();
	return iExitCode;
}
