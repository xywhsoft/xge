#include "../../xge.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct app_state_t {
	xge_xui_context_t tXui;
	xge_font_t tFont;
	xge_xui_widget pRootPanel;
	xge_xui_widget pStatusWidget;
	xge_xui_widget pPrimaryWidget;
	xge_xui_widget pSecondaryWidget;
	xge_xui_widget pHintWidget;
	xge_xui_label_t tStatusLabel;
	xge_xui_label_t tHintLabel;
	xge_xui_input_t tPrimaryInput;
	xge_xui_input_t tSecondaryInput;
	int bFontReady;
	int iFrameLimit;
	int iFrameCount;
	int iLastWidth;
	int iLastHeight;
	int bInitOK;
	int bBlinkOK;
	int bResetOK;
	int bSwitchOK;
	int bFocusRouteOK;
} app_state_t;

static int ArgInt(const char* sText, int iDefault)
{
	int iValue;

	if ( (sText == NULL) || (sText[0] == 0) ) {
		return iDefault;
	}
	iValue = atoi(sText);
	return (iValue > 0) ? iDefault + (iValue - iDefault) : iDefault;
}

static void MakeMouseEvent(xge_event_t* pEvent, int iType, int iButton, float fX, float fY)
{
	memset(pEvent, 0, sizeof(*pEvent));
	pEvent->iType = iType;
	pEvent->iParam1 = iButton;
	pEvent->fX = fX;
	pEvent->fY = fY;
}

static xge_vec2_t ContentPoint(xge_xui_widget pWidget, float fRatioX, float fRatioY)
{
	xge_vec2_t tPoint;

	tPoint.fX = pWidget->tContentRect.fX + pWidget->tContentRect.fW * fRatioX;
	tPoint.fY = pWidget->tContentRect.fY + pWidget->tContentRect.fH * fRatioY;
	return tPoint;
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
			printf("xui-input-blink-focus-lab font loaded: %s\n", arrFonts[i]);
			return XGE_OK;
		}
	}
	printf("xui-input-blink-focus-lab font load failed\n");
	return XGE_ERROR_RESOURCE_FAILED;
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
	if ( tRoot.fW < 720.0f ) {
		tRoot.fW = 720.0f;
	}
	if ( tRoot.fH < 260.0f ) {
		tRoot.fH = 260.0f;
	}

	xgeXuiWidgetSetRect(pApp->pRootPanel, tRoot);
	xgeXuiWidgetSetRect(pApp->pStatusWidget, (xge_rect_t){ 0.0f, 0.0f, tRoot.fW, 40.0f });
	xgeXuiWidgetSetRect(pApp->pPrimaryWidget, (xge_rect_t){ 24.0f, 84.0f, tRoot.fW - 48.0f, 36.0f });
	xgeXuiWidgetSetRect(pApp->pSecondaryWidget, (xge_rect_t){ 24.0f, 136.0f, tRoot.fW - 48.0f, 36.0f });
	xgeXuiWidgetSetRect(pApp->pHintWidget, (xge_rect_t){ 24.0f, tRoot.fH - 40.0f, tRoot.fW - 48.0f, 24.0f });

	pApp->iLastWidth = iWidth;
	pApp->iLastHeight = iHeight;
}

static void UpdateStatus(app_state_t* pApp)
{
	char sText[320];

	snprintf(
		sText,
		sizeof(sText),
		"init=%d blink=%d reset=%d switch=%d route=%d focus=%d/%d visible=%d/%d",
		pApp->bInitOK,
		pApp->bBlinkOK,
		pApp->bResetOK,
		pApp->bSwitchOK,
		pApp->bFocusRouteOK,
		pApp->tXui.pFocus == pApp->pPrimaryWidget,
		pApp->tXui.pFocus == pApp->pSecondaryWidget,
		pApp->tPrimaryInput.bCursorVisible,
		pApp->tSecondaryInput.bCursorVisible);
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
	tTheme.iTextColor = XGE_COLOR_RGBA(236, 241, 248, 255);
	tTheme.iBackgroundColor = XGE_COLOR_RGBA(20, 26, 34, 255);
	tTheme.iPanelColor = XGE_COLOR_RGBA(34, 42, 56, 255);
	tTheme.iBorderColor = XGE_COLOR_RGBA(84, 96, 118, 255);
	tTheme.iStateNormal = XGE_COLOR_RGBA(52, 64, 84, 255);
	tTheme.iStateHover = XGE_COLOR_RGBA(72, 88, 112, 255);
	tTheme.iStateActive = XGE_COLOR_RGBA(44, 58, 78, 255);
	tTheme.iStateFocus = XGE_COLOR_RGBA(54, 74, 104, 255);
	tTheme.iStateDisabled = XGE_COLOR_RGBA(78, 82, 92, 180);
	xgeXuiSetTheme(&pApp->tXui, &tTheme);

	pApp->pRootPanel = xgeXuiWidgetCreate();
	pApp->pStatusWidget = xgeXuiWidgetCreate();
	pApp->pPrimaryWidget = xgeXuiWidgetCreate();
	pApp->pSecondaryWidget = xgeXuiWidgetCreate();
	pApp->pHintWidget = xgeXuiWidgetCreate();
	if ( (pApp->pRootPanel == NULL) || (pApp->pStatusWidget == NULL) || (pApp->pPrimaryWidget == NULL) || (pApp->pSecondaryWidget == NULL) || (pApp->pHintWidget == NULL) ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}

	xgeXuiWidgetSetLayout(pApp->pRootPanel, XGE_XUI_LAYOUT_ABSOLUTE);
	xgeXuiWidgetSetBackground(pApp->pRootPanel, XGE_COLOR_RGBA(24, 30, 40, 255));
	xgeXuiWidgetSetRadius(pApp->pRootPanel, 8.0f);
	xgeXuiWidgetAdd(pRoot, pApp->pRootPanel);
	xgeXuiWidgetAdd(pApp->pRootPanel, pApp->pStatusWidget);
	xgeXuiWidgetAdd(pApp->pRootPanel, pApp->pPrimaryWidget);
	xgeXuiWidgetAdd(pApp->pRootPanel, pApp->pSecondaryWidget);
	xgeXuiWidgetAdd(pApp->pRootPanel, pApp->pHintWidget);

	xgeXuiWidgetSetBackground(pApp->pStatusWidget, XGE_COLOR_RGBA(46, 58, 78, 255));
	xgeXuiWidgetSetPaddingPx(pApp->pStatusWidget, 10.0f, 8.0f, 10.0f, 8.0f);
	xgeXuiWidgetSetPaddingPx(pApp->pPrimaryWidget, 8.0f, 7.0f, 8.0f, 7.0f);
	xgeXuiWidgetSetPaddingPx(pApp->pSecondaryWidget, 8.0f, 7.0f, 8.0f, 7.0f);

	if ( xgeXuiLabelInit(&pApp->tStatusLabel, pApp->pStatusWidget, pFont, "xui input blink focus lab") != XGE_OK ) {
		return XGE_ERROR;
	}
	if ( xgeXuiLabelInit(&pApp->tHintLabel, pApp->pHintWidget, pFont, "Auto-check covers focus ownership, blink toggle, unfocused reset, and focus route by click.") != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiLabelSetColor(&pApp->tStatusLabel, XGE_COLOR_RGBA(248, 250, 252, 255));
	xgeXuiLabelSetColor(&pApp->tHintLabel, XGE_COLOR_RGBA(164, 182, 206, 255));

	if ( xgeXuiInputInit(&pApp->tPrimaryInput, &pApp->tXui, pApp->pPrimaryWidget, pFont) != XGE_OK ) {
		return XGE_ERROR;
	}
	if ( xgeXuiInputInit(&pApp->tSecondaryInput, &pApp->tXui, pApp->pSecondaryWidget, pFont) != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiInputSetColors(&pApp->tPrimaryInput, XGE_COLOR_RGBA(242, 246, 252, 255), XGE_COLOR_RGBA(28, 36, 48, 255), XGE_COLOR_RGBA(42, 58, 82, 255), XGE_COLOR_RGBA(255, 214, 92, 255));
	xgeXuiInputSetColors(&pApp->tSecondaryInput, XGE_COLOR_RGBA(242, 246, 252, 255), XGE_COLOR_RGBA(28, 36, 48, 255), XGE_COLOR_RGBA(42, 58, 82, 255), XGE_COLOR_RGBA(96, 214, 255, 255));
	xgeXuiInputSetText(&pApp->tPrimaryInput, "Primary input");
	xgeXuiInputSetText(&pApp->tSecondaryInput, "Secondary input");
	return XGE_OK;
}

static int RunChecks(app_state_t* pApp)
{
	xge_event_t tEvent;
	xge_vec2_t tPoint;
	int bStartVisible;

	LayoutRoot(pApp);
	xgeXuiUpdate(&pApp->tXui, 0.0f);
	pApp->bInitOK =
		(pApp->pPrimaryWidget->procEvent == xgeXuiInputEventProc) &&
		(pApp->pPrimaryWidget->procUpdate == xgeXuiInputUpdateProc) &&
		(pApp->pPrimaryWidget->procPaint == xgeXuiInputPaintProc) &&
		(pApp->pSecondaryWidget->procEvent == xgeXuiInputEventProc);

	xgeXuiSetFocus(&pApp->tXui, pApp->pPrimaryWidget);
	bStartVisible = pApp->tPrimaryInput.bCursorVisible;
	xgeXuiInputUpdateProc(pApp->pPrimaryWidget, 0.51f, &pApp->tPrimaryInput);
	pApp->bBlinkOK =
		(pApp->tXui.pFocus == pApp->pPrimaryWidget) &&
		(pApp->tPrimaryInput.bCursorVisible != bStartVisible);

	xgeXuiSetFocus(&pApp->tXui, NULL);
	pApp->tPrimaryInput.bCursorVisible = 0;
	xgeXuiInputUpdateProc(pApp->pPrimaryWidget, 0.10f, &pApp->tPrimaryInput);
	pApp->bResetOK =
		(pApp->tXui.pFocus == NULL) &&
		(pApp->tPrimaryInput.bCursorVisible == 1);

	xgeXuiSetFocus(&pApp->tXui, pApp->pPrimaryWidget);
	pApp->tSecondaryInput.bCursorVisible = 0;
	xgeXuiSetFocus(&pApp->tXui, pApp->pSecondaryWidget);
	xgeXuiInputUpdateProc(pApp->pSecondaryWidget, 0.51f, &pApp->tSecondaryInput);
	pApp->bSwitchOK =
		(pApp->tXui.pFocus == pApp->pSecondaryWidget) &&
		(pApp->tPrimaryInput.bCursorVisible == 1) &&
		(pApp->tSecondaryInput.bCursorVisible == 1);

	xgeXuiSetFocus(&pApp->tXui, NULL);
	tPoint = ContentPoint(pApp->pPrimaryWidget, 0.15f, 0.5f);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_DOWN, XGE_MOUSE_LEFT, tPoint.fX, tPoint.fY);
	(void)xgeXuiInputEvent(&pApp->tPrimaryInput, &tEvent);
	pApp->bFocusRouteOK =
		(pApp->tXui.pFocus == pApp->pPrimaryWidget) &&
		(pApp->tPrimaryInput.bCursorVisible == 1);

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
	xgeXuiInputUnit(&pApp->tSecondaryInput);
	xgeXuiInputUnit(&pApp->tPrimaryInput);
	xgeXuiLabelUnit(&pApp->tHintLabel);
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
	xgeClear(XGE_COLOR_RGBA(20, 26, 34, 255));
	xgeXuiPaint(&pApp->tXui);
	xgeEnd();
	xgePresent();

	pApp->iFrameCount++;
	if ( pApp->iFrameCount >= pApp->iFrameLimit ) {
		printf(
			"xui-input-blink-focus-lab final-summary frames=%d init=%d blink=%d reset=%d switch=%d route=%d focus=%d/%d visible=%d/%d\n",
			pApp->iFrameCount,
			pApp->bInitOK,
			pApp->bBlinkOK,
			pApp->bResetOK,
			pApp->bSwitchOK,
			pApp->bFocusRouteOK,
			pApp->tXui.pFocus == pApp->pPrimaryWidget,
			pApp->tXui.pFocus == pApp->pSecondaryWidget,
			pApp->tPrimaryInput.bCursorVisible,
			pApp->tSecondaryInput.bCursorVisible);
		printf("xui-input-blink-focus-lab summary frames=%d/%d\n", pApp->iFrameCount, pApp->iFrameLimit);
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
	tDesc.iWidth = 760;
	tDesc.iHeight = 260;
	tDesc.sTitle = "XGE XUI Input Blink Focus Lab";
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
	iExitCode = (tApp.bInitOK && tApp.bBlinkOK && tApp.bResetOK && tApp.bSwitchOK && tApp.bFocusRouteOK) ? 0 : 3;
	AppUnit(&tApp);
	xgeUnit();
	return iExitCode;
}
