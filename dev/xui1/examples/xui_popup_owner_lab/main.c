#include "../../xge.h"
#include "../xui_demo_style.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct app_state_t {
	xge_xui_context_t tXui;
	xge_font_t tFont;
	xge_xui_widget pRootPanel;
	xge_xui_widget pStatusWidget;
	xge_xui_widget pOwnerWidget;
	xge_xui_widget pPopupWidget;
	xge_xui_widget pPopupLabelWidget;
	xge_xui_widget pHintWidget;
	xge_xui_label_t tStatusLabel;
	xge_xui_label_t tOwnerLabel;
	xge_xui_label_t tPopupLabel;
	xge_xui_label_t tHint;
	xge_xui_popup_t tPopup;
	int bFontReady;
	int iFrameLimit;
	int iFrameCount;
	int iLastWidth;
	int iLastHeight;
	int iCloseCount;
	int bInitOK;
	int bOpenOK;
	int bOwnerOK;
	int bAutoCloseOK;
	int bProcOK;
	int bBackgroundOK;
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

static void PopupClose(xge_xui_widget pWidget, void* pUser)
{
	app_state_t* pApp;

	(void)pWidget;
	pApp = (app_state_t*)pUser;
	if ( pApp != NULL ) {
		pApp->iCloseCount++;
	}
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
			printf("xui-popup-owner-lab font loaded: %s\n", arrFonts[i]);
			return XGE_OK;
		}
	}
	printf("xui-popup-owner-lab font load failed\n");
	return XGE_ERROR_RESOURCE_FAILED;
}

static void LayoutRoot(app_state_t* pApp)
{
	xge_rect_t tRoot;
	float fInnerW;
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
	if ( tRoot.fW < 660.0f ) {
		tRoot.fW = 660.0f;
	}
	if ( tRoot.fH < 360.0f ) {
		tRoot.fH = 360.0f;
	}
	fInnerW = tRoot.fW - 40.0f;

	xgeXuiWidgetSetRect(pApp->pRootPanel, tRoot);
	xgeXuiWidgetSetRect(pApp->pStatusWidget, (xge_rect_t){ 0.0f, 0.0f, tRoot.fW, 40.0f });
	xgeXuiWidgetSetRect(pApp->pOwnerWidget, (xge_rect_t){ 24.0f, 84.0f, 220.0f, 42.0f });
	xgeXuiWidgetSetRect(pApp->pPopupWidget, (xge_rect_t){ 276.0f, 90.0f, 212.0f, 88.0f });
	xgeXuiWidgetSetRect(pApp->pPopupLabelWidget, (xge_rect_t){ 12.0f, 12.0f, 176.0f, 48.0f });
	xgeXuiWidgetSetRect(pApp->pHintWidget, (xge_rect_t){ 24.0f, tRoot.fH - 46.0f, fInnerW, 24.0f });

	pApp->iLastWidth = iWidth;
	pApp->iLastHeight = iHeight;
}

static void UpdateStatus(app_state_t* pApp)
{
	char sText[320];

	snprintf(
		sText,
		sizeof(sText),
		"init=%d open=%d owner=%d autoclose=%d proc=%d bg=%d close=%d open=%d",
		pApp->bInitOK,
		pApp->bOpenOK,
		pApp->bOwnerOK,
		pApp->bAutoCloseOK,
		pApp->bProcOK,
		pApp->bBackgroundOK,
		pApp->iCloseCount,
		xgeXuiPopupIsOpen(&pApp->tPopup));
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
	tTheme.iTextColor = XGE_COLOR_RGBA(238, 242, 248, 255);
	tTheme.iBackgroundColor = XGE_COLOR_RGBA(20, 26, 34, 255);
	tTheme.iPanelColor = XGE_COLOR_RGBA(34, 42, 56, 255);
	tTheme.iBorderColor = XGE_COLOR_RGBA(84, 96, 118, 255);
	tTheme.iAccentColor = XGE_COLOR_RGBA(102, 170, 255, 255);
	tTheme.iStateNormal = XGE_COLOR_RGBA(50, 64, 84, 255);
	tTheme.iStateHover = XGE_COLOR_RGBA(68, 84, 108, 255);
	tTheme.iStateActive = XGE_COLOR_RGBA(44, 58, 78, 255);
	tTheme.iStateFocus = XGE_COLOR_RGBA(54, 74, 104, 255);
	tTheme.iStateDisabled = XGE_COLOR_RGBA(78, 82, 92, 180);
	tTheme.fRadius = 8.0f;
	tTheme.fPadding = 8.0f;
	tTheme.fSpacing = 10.0f;
	xgeXuiSetTheme(&pApp->tXui, &tTheme);
	XgeXuiDemoApplyTheme(&pApp->tXui, pFont);

	pApp->pRootPanel = xgeXuiWidgetCreate();
	pApp->pStatusWidget = xgeXuiWidgetCreate();
	pApp->pOwnerWidget = xgeXuiWidgetCreate();
	pApp->pPopupWidget = xgeXuiWidgetCreate();
	pApp->pPopupLabelWidget = xgeXuiWidgetCreate();
	pApp->pHintWidget = xgeXuiWidgetCreate();
	if ( (pApp->pRootPanel == NULL) || (pApp->pStatusWidget == NULL) || (pApp->pOwnerWidget == NULL) ||
	     (pApp->pPopupWidget == NULL) || (pApp->pPopupLabelWidget == NULL) || (pApp->pHintWidget == NULL) ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}

	xgeXuiWidgetSetLayout(pApp->pRootPanel, XGE_XUI_LAYOUT_ABSOLUTE);
	xgeXuiWidgetSetBackground(pApp->pRootPanel, XGE_COLOR_RGBA(24, 30, 40, 255));
	xgeXuiWidgetSetRadius(pApp->pRootPanel, 8.0f);
	xgeXuiWidgetAdd(pRoot, pApp->pRootPanel);
	xgeXuiWidgetAdd(pApp->pRootPanel, pApp->pStatusWidget);
	xgeXuiWidgetAdd(pApp->pRootPanel, pApp->pOwnerWidget);
	xgeXuiWidgetAdd(pApp->pRootPanel, pApp->pHintWidget);

	xgeXuiWidgetSetBackground(pApp->pStatusWidget, XGE_COLOR_RGBA(46, 58, 78, 255));
	xgeXuiWidgetSetPaddingPx(pApp->pStatusWidget, 10.0f, 8.0f, 10.0f, 8.0f);
	xgeXuiWidgetSetBackground(pApp->pOwnerWidget, XGE_COLOR_RGBA(52, 70, 94, 255));
	xgeXuiWidgetSetPaddingPx(pApp->pOwnerWidget, 12.0f, 10.0f, 12.0f, 10.0f);
	xgeXuiWidgetSetFocusable(pApp->pOwnerWidget, 1);

	if ( xgeXuiLabelInit(&pApp->tStatusLabel, pApp->pStatusWidget, pFont, "xui popup owner lab") != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiLabelSetColor(&pApp->tStatusLabel, XGE_COLOR_RGBA(248, 250, 252, 255));

	if ( xgeXuiLabelInit(&pApp->tOwnerLabel, pApp->pOwnerWidget, pFont, "Popup owner") != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiLabelSetColor(&pApp->tOwnerLabel, XGE_COLOR_RGBA(248, 250, 252, 255));

	if ( xgeXuiLabelInit(&pApp->tPopupLabel, pApp->pPopupLabelWidget, pFont, "Popup overlay\nowner / outside / ESC paths") != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiLabelSetColor(&pApp->tPopupLabel, XGE_COLOR_RGBA(248, 250, 252, 255));

	if ( xgeXuiLabelInit(&pApp->tHint, pApp->pHintWidget, pFont, "Auto-check covers owner passthrough, outside close, ESC close, direct PopupEvent/PopupEventProc and close callback.") != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiLabelSetColor(&pApp->tHint, XGE_COLOR_RGBA(164, 182, 206, 255));

	if ( xgeXuiPopupInit(&pApp->tPopup, &pApp->tXui, pApp->pPopupWidget) != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiWidgetAdd(pOverlayRoot, pApp->pPopupWidget);
	xgeXuiWidgetAdd(pApp->pPopupWidget, pApp->pPopupLabelWidget);
	return XGE_OK;
}

static int RunChecks(app_state_t* pApp)
{
	xge_event_t tEvent;
	xge_vec2_t tCenter;
	int iRet;

	LayoutRoot(pApp);
	xgeXuiUpdate(&pApp->tXui, 0.0f);

	xgeXuiPopupSetOwner(&pApp->tPopup, pApp->pOwnerWidget);
	xgeXuiPopupSetClose(&pApp->tPopup, PopupClose, pApp);
	xgeXuiPopupSetBackground(&pApp->tPopup, XGE_COLOR_RGBA(42, 50, 64, 245));
	pApp->bInitOK =
		(pApp->tPopup.pWidget == pApp->pPopupWidget) &&
		(pApp->tPopup.pOwner == pApp->pOwnerWidget) &&
		(pApp->pPopupWidget->procEvent == xgeXuiPopupEventProc) &&
		(pApp->pPopupWidget->procPaint == NULL);

	pApp->bBackgroundOK = (pApp->pPopupWidget->tStyle.iBackgroundColor == XGE_COLOR_RGBA(42, 50, 64, 245));

	xgeXuiPopupSetAutoClose(&pApp->tPopup, 1, 1);
	xgeXuiPopupSetOpen(&pApp->tPopup, 1);
	pApp->bOpenOK =
		(xgeXuiPopupIsOpen(&pApp->tPopup) != 0) &&
		(xgeXuiWidgetIsVisible(pApp->pPopupWidget) != 0) &&
		(pApp->tXui.pFocus == pApp->pPopupWidget);

	tCenter = WidgetCenter(pApp->pOwnerWidget);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_DOWN, XGE_MOUSE_LEFT, tCenter.fX, tCenter.fY);
	iRet = xgeXuiDispatchEvent(&pApp->tXui, &tEvent);
	pApp->bOwnerOK =
		(iRet == XGE_XUI_EVENT_CONTINUE) &&
		(xgeXuiPopupIsOpen(&pApp->tPopup) != 0);

	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_DOWN, XGE_MOUSE_LEFT, (float)xgeGetWidth() - 12.0f, (float)xgeGetHeight() - 12.0f);
	iRet = xgeXuiDispatchEvent(&pApp->tXui, &tEvent);
	pApp->bAutoCloseOK =
		(iRet == XGE_XUI_EVENT_CONSUMED) &&
		(xgeXuiPopupIsOpen(&pApp->tPopup) == 0) &&
		(pApp->iCloseCount == 1);

	xgeXuiPopupSetOpen(&pApp->tPopup, 1);
	MakeKeyEvent(&tEvent, XGE_KEY_ESCAPE);
	iRet = xgeXuiDispatchEvent(&pApp->tXui, &tEvent);
	pApp->bAutoCloseOK =
		pApp->bAutoCloseOK &&
		(iRet == XGE_XUI_EVENT_CONSUMED) &&
		(xgeXuiPopupIsOpen(&pApp->tPopup) == 0) &&
		(pApp->iCloseCount == 2);

	xgeXuiPopupSetAutoClose(&pApp->tPopup, 0, 1);
	xgeXuiPopupSetOpen(&pApp->tPopup, 1);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_DOWN, XGE_MOUSE_LEFT, (float)xgeGetWidth() - 12.0f, (float)xgeGetHeight() - 12.0f);
	iRet = xgeXuiPopupEvent(&pApp->tPopup, &tEvent);
	pApp->bProcOK =
		(iRet == XGE_XUI_EVENT_CONTINUE) &&
		(xgeXuiPopupIsOpen(&pApp->tPopup) != 0);

	xgeXuiPopupSetAutoClose(&pApp->tPopup, 1, 1);
	iRet = xgeXuiPopupEventProc(pApp->pPopupWidget, &tEvent, &pApp->tPopup);
	pApp->bProcOK =
		pApp->bProcOK &&
		(iRet == XGE_XUI_EVENT_CONSUMED) &&
		(xgeXuiPopupIsOpen(&pApp->tPopup) == 0) &&
		(pApp->iCloseCount == 3);

	xgeXuiPopupSetOpen(&pApp->tPopup, 1);
	UpdateStatus(pApp);
	return XGE_OK;
}

static int AppInit(app_state_t* pApp, int iFrameLimit)
{
	memset(pApp, 0, sizeof(*pApp));
	pApp->iFrameLimit = iFrameLimit;
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
	xgeXuiPopupUnit(&pApp->tPopup);
	xgeXuiLabelUnit(&pApp->tHint);
	xgeXuiLabelUnit(&pApp->tPopupLabel);
	xgeXuiLabelUnit(&pApp->tOwnerLabel);
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
	xge_rect_t tRect;

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
		tRect = xgeXuiWidgetGetRect(pApp->pPopupWidget);
		printf(
			"xui-popup-owner-lab final-summary frames=%d init=%d open=%d owner=%d autoclose=%d proc=%d bg=%d close=%d open=%d rect=%.2f,%.2f,%.2f,%.2f focus=%d\n",
			pApp->iFrameCount,
			pApp->bInitOK,
			pApp->bOpenOK,
			pApp->bOwnerOK,
			pApp->bAutoCloseOK,
			pApp->bProcOK,
			pApp->bBackgroundOK,
			pApp->iCloseCount,
			xgeXuiPopupIsOpen(&pApp->tPopup),
			tRect.fX,
			tRect.fY,
			tRect.fW,
			tRect.fH,
			(pApp->tXui.pFocus == pApp->pPopupWidget));
		printf("xui-popup-owner-lab summary frames=%d/%d\n", pApp->iFrameCount, pApp->iFrameLimit);
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
	tDesc.iHeight = 380;
	tDesc.sTitle = "XGE XUI Popup Owner Lab";
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
		(tApp.bInitOK && tApp.bOpenOK && tApp.bOwnerOK && tApp.bAutoCloseOK && tApp.bProcOK && tApp.bBackgroundOK) ? 0 : 3;
	AppUnit(&tApp);
	xgeUnit();
	return iExitCode;
}

