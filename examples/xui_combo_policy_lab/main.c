#include "../../xge.h"
#include "../xui_demo_style.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char* g_arrItems[] = {
	"Alpha",
	"Bravo",
	"Charlie",
	"Delta",
	"Epsilon",
	"Foxtrot"
};

typedef struct app_state_t {
	xge_xui_context_t tXui;
	xge_font_t tFont;
	xge_xui_widget pRootPanel;
	xge_xui_widget pStatusWidget;
	xge_xui_widget pComboWidget;
	xge_xui_widget pHintWidget;
	xge_xui_label_t tStatusLabel;
	xge_xui_label_t tHint;
	xge_xui_combo_box_t tCombo;
	int bFontReady;
	int iFrameLimit;
	int iFrameCount;
	int iLastWidth;
	int iLastHeight;
	int iSelectCount;
	int iLastSelected;
	int bWideLayout;
	int bInitOK;
	int bConfigOK;
	int bSyncOK;
	int bHeightOK;
	int bResetOK;
	int bLayoutOK;
	int bSelectOK;
	int bCloseOK;
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

static void MakeMouseEvent(xge_event_t* pEvent, int iType, int iButton, float fX, float fY)
{
	memset(pEvent, 0, sizeof(*pEvent));
	pEvent->iType = iType;
	pEvent->iParam1 = iButton;
	pEvent->fX = fX;
	pEvent->fY = fY;
}

static xge_vec2_t WidgetCenter(xge_xui_widget pWidget)
{
	xge_vec2_t tCenter;

	tCenter.fX = pWidget->tRect.fX + pWidget->tRect.fW * 0.5f;
	tCenter.fY = pWidget->tRect.fY + pWidget->tRect.fH * 0.5f;
	return tCenter;
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
			printf("xui-combo-policy-lab font loaded: %s\n", arrFonts[i]);
			return XGE_OK;
		}
	}
	printf("xui-combo-policy-lab font load failed\n");
	return XGE_ERROR_RESOURCE_FAILED;
}

static void ComboSelect(xge_xui_widget pWidget, int iIndex, void* pUser)
{
	app_state_t* pApp;

	(void)pWidget;
	pApp = (app_state_t*)pUser;
	if ( pApp != NULL ) {
		pApp->iSelectCount++;
		pApp->iLastSelected = iIndex;
	}
}

static void LayoutRoot(app_state_t* pApp)
{
	xge_rect_t tRoot;
	float fComboW;
	int iWidth;
	int iHeight;

	iWidth = xgeGetWidth();
	iHeight = xgeGetHeight();
	if ( (iWidth == pApp->iLastWidth) && (iHeight == pApp->iLastHeight) ) {
		/* still update combo width if policy switched */
	}

	tRoot.fX = 18.0f;
	tRoot.fY = 18.0f;
	tRoot.fW = (float)iWidth - 36.0f;
	tRoot.fH = (float)iHeight - 36.0f;
	if ( tRoot.fW < 660.0f ) {
		tRoot.fW = 660.0f;
	}
	if ( tRoot.fH < 280.0f ) {
		tRoot.fH = 280.0f;
	}
	fComboW = pApp->bWideLayout ? 296.0f : 240.0f;

	xgeXuiWidgetSetRect(pApp->pRootPanel, tRoot);
	xgeXuiWidgetSetRect(pApp->pStatusWidget, (xge_rect_t){ 0.0f, 0.0f, tRoot.fW, 40.0f });
	xgeXuiWidgetSetRect(pApp->pComboWidget, (xge_rect_t){ 28.0f, 82.0f, fComboW, 34.0f });
	xgeXuiWidgetSetRect(pApp->pHintWidget, (xge_rect_t){ 28.0f, 148.0f, tRoot.fW - 56.0f, 26.0f });

	pApp->iLastWidth = iWidth;
	pApp->iLastHeight = iHeight;
}

static void UpdateStatus(app_state_t* pApp)
{
	char sText[320];

	snprintf(
		sText,
		sizeof(sText),
		"init=%d config=%d sync=%d height=%d reset=%d layout=%d select=%d close=%d sel=%d open=%d cb=%d last=%d popup=%.0fx%.0f",
		pApp->bInitOK,
		pApp->bConfigOK,
		pApp->bSyncOK,
		pApp->bHeightOK,
		pApp->bResetOK,
		pApp->bLayoutOK,
		pApp->bSelectOK,
		pApp->bCloseOK,
		xgeXuiComboBoxGetSelected(&pApp->tCombo),
		xgeXuiComboBoxIsOpen(&pApp->tCombo),
		pApp->iSelectCount,
		pApp->iLastSelected,
		pApp->tCombo.pPopupWidget != NULL ? pApp->tCombo.pPopupWidget->tRect.fW : 0.0f,
		pApp->tCombo.pPopupWidget != NULL ? pApp->tCombo.pPopupWidget->tRect.fH : 0.0f);
	xgeXuiLabelSetText(&pApp->tStatusLabel, sText);
}

static int CreateUI(app_state_t* pApp)
{
	xge_xui_theme_t tTheme;
	xge_xui_widget pRoot;
	xge_font pFont;

	pRoot = xgeXuiRoot(&pApp->tXui);
	pFont = pApp->bFontReady ? &pApp->tFont : NULL;
	if ( pRoot == NULL ) {
		return XGE_ERROR;
	}

	xgeXuiThemeDefault(&tTheme);
	tTheme.pFont = pFont;
	tTheme.iTextColor = XGE_COLOR_RGBA(240, 244, 250, 255);
	tTheme.iBackgroundColor = XGE_COLOR_RGBA(20, 26, 36, 255);
	tTheme.iPanelColor = XGE_COLOR_RGBA(32, 40, 54, 255);
	tTheme.iBorderColor = XGE_COLOR_RGBA(84, 98, 122, 255);
	tTheme.iStateNormal = XGE_COLOR_RGBA(52, 70, 94, 255);
	tTheme.iStateHover = XGE_COLOR_RGBA(70, 92, 120, 255);
	tTheme.iStateActive = XGE_COLOR_RGBA(36, 54, 78, 255);
	tTheme.iStateFocus = XGE_COLOR_RGBA(96, 112, 142, 255);
	tTheme.iStateDisabled = XGE_COLOR_RGBA(84, 86, 92, 180);
	tTheme.fRadius = 8.0f;
	tTheme.fPadding = 8.0f;
	tTheme.fSpacing = 10.0f;
	xgeXuiSetTheme(&pApp->tXui, &tTheme);
	XgeXuiDemoApplyTheme(&pApp->tXui, pFont);

	pApp->pRootPanel = xgeXuiWidgetCreate();
	pApp->pStatusWidget = xgeXuiWidgetCreate();
	pApp->pComboWidget = xgeXuiWidgetCreate();
	pApp->pHintWidget = xgeXuiWidgetCreate();
	if ( (pApp->pRootPanel == NULL) || (pApp->pStatusWidget == NULL) || (pApp->pComboWidget == NULL) || (pApp->pHintWidget == NULL) ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}

	xgeXuiWidgetSetLayout(pApp->pRootPanel, XGE_XUI_LAYOUT_ABSOLUTE);
	xgeXuiWidgetSetBackground(pApp->pRootPanel, XGE_COLOR_RGBA(22, 28, 38, 255));
	xgeXuiWidgetSetRadius(pApp->pRootPanel, 8.0f);
	xgeXuiWidgetAdd(pRoot, pApp->pRootPanel);
	xgeXuiWidgetAdd(pApp->pRootPanel, pApp->pStatusWidget);
	xgeXuiWidgetAdd(pApp->pRootPanel, pApp->pComboWidget);
	xgeXuiWidgetAdd(pApp->pRootPanel, pApp->pHintWidget);

	xgeXuiWidgetSetBackground(pApp->pStatusWidget, XGE_COLOR_RGBA(44, 58, 78, 255));
	xgeXuiWidgetSetPaddingPx(pApp->pStatusWidget, 10.0f, 8.0f, 10.0f, 8.0f);

	if ( xgeXuiLabelInit(&pApp->tStatusLabel, pApp->pStatusWidget, pFont, "xui combo policy lab") != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiLabelSetColor(&pApp->tStatusLabel, XGE_COLOR_RGBA(248, 250, 252, 255));

	if ( xgeXuiComboBoxInit(&pApp->tCombo, &pApp->tXui, pApp->pComboWidget) != XGE_OK ) {
		return XGE_ERROR;
	}

	if ( xgeXuiLabelInit(&pApp->tHint, pApp->pHintWidget, pFont, "Auto-check covers popup/list selection sync, live drop-down height update, item reset and wide-layout reopen.") != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiLabelSetColor(&pApp->tHint, XGE_COLOR_RGBA(164, 182, 206, 255));
	return XGE_OK;
}

static int RunChecks(app_state_t* pApp)
{
	xge_event_t tEvent;
	xge_vec2_t tCenter;
	int iRet;

	LayoutRoot(pApp);
	xgeXuiUpdate(&pApp->tXui, 0.0f);

	pApp->bInitOK =
		(pApp->pComboWidget->procEvent == xgeXuiComboBoxEventProc) &&
		(pApp->pComboWidget->procPaint == xgeXuiComboBoxPaintProc) &&
		(pApp->tCombo.pPopupWidget != NULL) &&
		(pApp->tCombo.pListWidget != NULL) &&
		(xgeXuiComboBoxGetSelected(&pApp->tCombo) == -1) &&
		(xgeXuiComboBoxIsOpen(&pApp->tCombo) == 0);

	xgeXuiComboBoxSetFont(&pApp->tCombo, pApp->bFontReady ? &pApp->tFont : NULL);
	xgeXuiComboBoxSetItems(&pApp->tCombo, g_arrItems, (int)(sizeof(g_arrItems) / sizeof(g_arrItems[0])));
	xgeXuiComboBoxSetSelected(&pApp->tCombo, 3);
	xgeXuiComboBoxSetPopupHeight(&pApp->tCombo, 84.0f);
	xgeXuiComboBoxSetSelect(&pApp->tCombo, ComboSelect, pApp);
	xgeXuiComboBoxSetColors(
		&pApp->tCombo,
		XGE_COLOR_RGBA(52, 70, 94, 255),
		XGE_COLOR_RGBA(70, 92, 120, 255),
		XGE_COLOR_RGBA(96, 112, 142, 255),
		XGE_COLOR_RGBA(84, 86, 92, 180),
		XGE_COLOR_RGBA(248, 250, 252, 255),
		XGE_COLOR_RGBA(42, 50, 64, 255));
	pApp->bConfigOK =
		(xgeXuiComboBoxGetSelected(&pApp->tCombo) == 3) &&
		(pApp->tCombo.iItemCount == 6) &&
		(pApp->tCombo.procSelect == ComboSelect) &&
		(pApp->tCombo.pUser == pApp) &&
		(pApp->tCombo.pFont == (pApp->bFontReady ? &pApp->tFont : NULL)) &&
		(pApp->tCombo.fPopupHeight == 84.0f);

	tCenter = WidgetCenter(pApp->pComboWidget);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_DOWN, XGE_MOUSE_LEFT, tCenter.fX, tCenter.fY);
	iRet = xgeXuiDispatchEvent(&pApp->tXui, &tEvent);
	xgeXuiUpdate(&pApp->tXui, 0.0f);
	pApp->bSyncOK =
		(iRet == XGE_XUI_EVENT_CONSUMED) &&
		(xgeXuiComboBoxIsOpen(&pApp->tCombo) != 0) &&
		(pApp->tXui.pFocus == pApp->tCombo.pListWidget) &&
		(xgeXuiListViewGetSelected(&pApp->tCombo.tList) == 3) &&
		(pApp->tCombo.pPopupWidget->tRect.fW == pApp->pComboWidget->tRect.fW) &&
		(pApp->tCombo.pPopupWidget->tRect.fH == 84.0f);

	xgeXuiComboBoxSetPopupHeight(&pApp->tCombo, 112.0f);
	pApp->bHeightOK =
		(xgeXuiComboBoxIsOpen(&pApp->tCombo) != 0) &&
		(pApp->tCombo.pPopupWidget->tRect.fH == 112.0f);

	xgeXuiListViewSetSelected(&pApp->tCombo.tList, 0);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_DOWN, XGE_MOUSE_LEFT, (float)xgeGetWidth() - 12.0f, (float)xgeGetHeight() - 12.0f);
	iRet = xgeXuiDispatchEvent(&pApp->tXui, &tEvent);
	pApp->bCloseOK =
		(iRet == XGE_XUI_EVENT_CONSUMED) &&
		(xgeXuiComboBoxIsOpen(&pApp->tCombo) == 0);

	pApp->bWideLayout = 1;
	LayoutRoot(pApp);
	xgeXuiUpdate(&pApp->tXui, 0.0f);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_DOWN, XGE_MOUSE_LEFT, WidgetCenter(pApp->pComboWidget).fX, WidgetCenter(pApp->pComboWidget).fY);
	iRet = xgeXuiDispatchEvent(&pApp->tXui, &tEvent);
	xgeXuiUpdate(&pApp->tXui, 0.0f);
	pApp->bLayoutOK =
		(iRet == XGE_XUI_EVENT_CONSUMED) &&
		(xgeXuiComboBoxIsOpen(&pApp->tCombo) != 0) &&
		(pApp->pComboWidget->tRect.fW == 296.0f) &&
		(pApp->tCombo.pPopupWidget->tRect.fW == 296.0f) &&
		(xgeXuiListViewGetSelected(&pApp->tCombo.tList) == 3);

	xgeXuiComboBoxSetItems(&pApp->tCombo, g_arrItems, 2);
	pApp->bResetOK =
		(xgeXuiComboBoxGetSelected(&pApp->tCombo) == -1) &&
		(xgeXuiListViewGetSelected(&pApp->tCombo.tList) == -1);
	xgeXuiComboBoxSetItems(&pApp->tCombo, g_arrItems, (int)(sizeof(g_arrItems) / sizeof(g_arrItems[0])));
	xgeXuiComboBoxSetSelected(&pApp->tCombo, 1);

	if ( xgeXuiComboBoxIsOpen(&pApp->tCombo) == 0 ) {
		MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_DOWN, XGE_MOUSE_LEFT, WidgetCenter(pApp->pComboWidget).fX, WidgetCenter(pApp->pComboWidget).fY);
		(void)xgeXuiDispatchEvent(&pApp->tXui, &tEvent);
		xgeXuiUpdate(&pApp->tXui, 0.0f);
	}

	MakeMouseEvent(
		&tEvent,
		XGE_EVENT_MOUSE_DOWN,
		XGE_MOUSE_LEFT,
		pApp->tCombo.pListWidget->tContentRect.fX + 12.0f,
		pApp->tCombo.pListWidget->tContentRect.fY + pApp->tCombo.tList.tBase.fItemHeight * 2.0f + 6.0f);
	iRet = xgeXuiDispatchEvent(&pApp->tXui, &tEvent);
	pApp->bSelectOK =
		(iRet == XGE_XUI_EVENT_CONSUMED) &&
		(xgeXuiComboBoxIsOpen(&pApp->tCombo) == 0) &&
		(xgeXuiComboBoxGetSelected(&pApp->tCombo) == 2) &&
		(pApp->iSelectCount == 1) &&
		(pApp->iLastSelected == 2);

	xgeXuiComboBoxSetSelected(&pApp->tCombo, 1);

	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_DOWN, XGE_MOUSE_LEFT, WidgetCenter(pApp->pComboWidget).fX, WidgetCenter(pApp->pComboWidget).fY);
	(void)xgeXuiDispatchEvent(&pApp->tXui, &tEvent);
	xgeXuiUpdate(&pApp->tXui, 0.0f);
	pApp->bLayoutOK =
		pApp->bLayoutOK &&
		(xgeXuiListViewGetSelected(&pApp->tCombo.tList) == 1) &&
		(pApp->tCombo.pPopupWidget->tRect.fW == 296.0f) &&
		(pApp->tCombo.pPopupWidget->tRect.fH == 112.0f);

	UpdateStatus(pApp);
	return XGE_OK;
}

static int AppInit(app_state_t* pApp, int iFrameLimit)
{
	memset(pApp, 0, sizeof(*pApp));
	pApp->iFrameLimit = iFrameLimit;
	pApp->iLastSelected = -1;
	if ( xgeXuiInit(&pApp->tXui) != XGE_OK ) {
		return XGE_ERROR;
	}
	if ( LoadFont(pApp) != XGE_OK ) {
		return XGE_ERROR;
	}
	if ( CreateUI(pApp) != XGE_OK ) {
		return XGE_ERROR;
	}
	return RunChecks(pApp);
}

static void AppUnit(app_state_t* pApp)
{
	xgeXuiComboBoxUnit(&pApp->tCombo);
	xgeXuiLabelUnit(&pApp->tHint);
	xgeXuiLabelUnit(&pApp->tStatusLabel);
	if ( pApp->bFontReady ) {
		xgeFontFree(&pApp->tFont);
	}
	xgeXuiUnit(&pApp->tXui);
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
	xgeXuiPaint(&pApp->tXui);
	xgeEnd();
	xgePresent();

	pApp->iFrameCount++;
	if ( (pApp->iFrameLimit > 0) && (pApp->iFrameCount >= pApp->iFrameLimit) ) {
		printf(
			"xui-combo-policy-lab final-summary frames=%d init=%d config=%d sync=%d height=%d reset=%d layout=%d select=%d close=%d selected=%d open=%d cb=%d last=%d popup=%.2f,%.2f\n",
			pApp->iFrameCount,
			pApp->bInitOK,
			pApp->bConfigOK,
			pApp->bSyncOK,
			pApp->bHeightOK,
			pApp->bResetOK,
			pApp->bLayoutOK,
			pApp->bSelectOK,
			pApp->bCloseOK,
			xgeXuiComboBoxGetSelected(&pApp->tCombo),
			xgeXuiComboBoxIsOpen(&pApp->tCombo),
			pApp->iSelectCount,
			pApp->iLastSelected,
			pApp->tCombo.pPopupWidget->tRect.fW,
			pApp->tCombo.pPopupWidget->tRect.fH);
		printf("xui-combo-policy-lab summary frames=%d/%d\n", pApp->iFrameCount, pApp->iFrameLimit);
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

	iFrameLimit = 0;
	for ( i = 1; i < argc; i++ ) {
		if ( strcmp(argv[i], "--frames") == 0 && (i + 1) < argc ) {
			iFrameLimit = ArgInt(argv[i + 1], iFrameLimit);
			i++;
		}
	}

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iWidth = 720;
	tDesc.iHeight = 280;
	tDesc.sTitle = "XGE XUI Combo Policy Lab";
	tDesc.iFlags = XGE_INIT_WINDOW | XGE_INIT_VSYNC;
	tDesc.iRunMode = XGE_RUN_GAME_LOOP;
	tDesc.iTargetFPS = 60;
	if ( xgeInit(&tDesc) != XGE_OK ) {
		return 1;
	}
	if ( AppInit(&tApp, iFrameLimit) != XGE_OK ) {
		xgeUnit();
		return 2;
	}
	xgeRun(AppFrame, &tApp);
	iExitCode =
		(tApp.bInitOK && tApp.bConfigOK && tApp.bSyncOK && tApp.bHeightOK &&
		 tApp.bResetOK && tApp.bLayoutOK && tApp.bSelectOK && tApp.bCloseOK) ? 0 : 3;
	AppUnit(&tApp);
	xgeUnit();
	return iExitCode;
}

