#include "../../xge.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct app_state_t {
	xge_xui_context_t tXui;
	xge_font_t tFont;
	xge_xui_widget pRootPanel;
	xge_xui_widget pStatusWidget;
	xge_xui_widget pInputLabelWidget;
	xge_xui_widget pInputWidget;
	xge_xui_widget pHintWidget;
	xge_xui_label_t tStatusLabel;
	xge_xui_label_t tInputLabel;
	xge_xui_label_t tHintLabel;
	xge_xui_input_t tInput;
	int bFontReady;
	int iFrameLimit;
	int iFrameCount;
	int iLastWidth;
	int iLastHeight;
	int bInitOK;
	int bWordOK;
	int bHomeEndOK;
	int bCollapseOK;
	int bScrollOK;
	float fScrollValue;
	int iCursorValue;
	int iSelectStart;
	int iSelectEnd;
} app_state_t;

static const char* g_sNavText = "alpha beta gamma delta epsilon zeta eta theta iota kappa lambda mu";

static int ArgInt(const char* sText, int iDefault)
{
	int iValue;

	if ( (sText == NULL) || (sText[0] == 0) ) {
		return iDefault;
	}
	iValue = atoi(sText);
	return (iValue > 0) ? iValue : iDefault;
}

static void MakeKeyEvent(xge_event_t* pEvent, int iKey, int iMods)
{
	memset(pEvent, 0, sizeof(*pEvent));
	pEvent->iType = XGE_EVENT_KEY_DOWN;
	pEvent->iParam1 = iKey;
	pEvent->iParam2 = iMods;
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
			printf("xui-input-nav-policy-lab font loaded: %s\n", arrFonts[i]);
			return XGE_OK;
		}
	}
	printf("xui-input-nav-policy-lab font load failed\n");
	return XGE_ERROR_RESOURCE_FAILED;
}

static void LayoutRoot(app_state_t* pApp)
{
	xge_rect_t tRoot;
	float fInputW;
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
	if ( tRoot.fH < 260.0f ) {
		tRoot.fH = 260.0f;
	}
	fInputW = 320.0f;
	if ( fInputW > tRoot.fW - 48.0f ) {
		fInputW = tRoot.fW - 48.0f;
	}

	xgeXuiWidgetSetRect(pApp->pRootPanel, tRoot);
	xgeXuiWidgetSetRect(pApp->pStatusWidget, (xge_rect_t){ 0.0f, 0.0f, tRoot.fW, 40.0f });
	xgeXuiWidgetSetRect(pApp->pInputLabelWidget, (xge_rect_t){ 24.0f, 72.0f, tRoot.fW - 48.0f, 24.0f });
	xgeXuiWidgetSetRect(pApp->pInputWidget, (xge_rect_t){ 24.0f, 102.0f, fInputW, 36.0f });
	xgeXuiWidgetSetRect(pApp->pHintWidget, (xge_rect_t){ 24.0f, tRoot.fH - 40.0f, tRoot.fW - 48.0f, 24.0f });

	pApp->iLastWidth = iWidth;
	pApp->iLastHeight = iHeight;
}

static void UpdateStatus(app_state_t* pApp)
{
	char sText[320];
	char sInput[256];

	snprintf(
		sText,
		sizeof(sText),
		"init=%d word=%d homeend=%d collapse=%d scroll=%d cursor=%d sel=%d..%d scroll=%.0f",
		pApp->bInitOK,
		pApp->bWordOK,
		pApp->bHomeEndOK,
		pApp->bCollapseOK,
		pApp->bScrollOK,
		pApp->iCursorValue,
		pApp->iSelectStart,
		pApp->iSelectEnd,
		pApp->fScrollValue);
	xgeXuiLabelSetText(&pApp->tStatusLabel, sText);

	snprintf(
		sInput,
		sizeof(sInput),
		"Input: \"%s\"",
		xgeXuiInputGetText(&pApp->tInput));
	xgeXuiLabelSetText(&pApp->tInputLabel, sInput);
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
	tTheme.iBackgroundColor = XGE_COLOR_RGBA(18, 24, 34, 255);
	tTheme.iPanelColor = XGE_COLOR_RGBA(32, 40, 54, 255);
	tTheme.iBorderColor = XGE_COLOR_RGBA(84, 96, 118, 255);
	tTheme.iStateNormal = XGE_COLOR_RGBA(50, 64, 84, 255);
	tTheme.iStateHover = XGE_COLOR_RGBA(68, 84, 108, 255);
	tTheme.iStateActive = XGE_COLOR_RGBA(44, 58, 78, 255);
	tTheme.iStateFocus = XGE_COLOR_RGBA(44, 64, 96, 255);
	tTheme.iStateDisabled = XGE_COLOR_RGBA(78, 82, 92, 180);
	tTheme.fRadius = 8.0f;
	tTheme.fPadding = 8.0f;
	tTheme.fSpacing = 10.0f;
	xgeXuiSetTheme(&pApp->tXui, &tTheme);

	pApp->pRootPanel = xgeXuiWidgetCreate();
	pApp->pStatusWidget = xgeXuiWidgetCreate();
	pApp->pInputLabelWidget = xgeXuiWidgetCreate();
	pApp->pInputWidget = xgeXuiWidgetCreate();
	pApp->pHintWidget = xgeXuiWidgetCreate();
	if ( (pApp->pRootPanel == NULL) || (pApp->pStatusWidget == NULL) || (pApp->pInputLabelWidget == NULL) ||
	     (pApp->pInputWidget == NULL) || (pApp->pHintWidget == NULL) ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}

	xgeXuiWidgetSetLayout(pApp->pRootPanel, XGE_XUI_LAYOUT_ABSOLUTE);
	xgeXuiWidgetSetBackground(pApp->pRootPanel, XGE_COLOR_RGBA(20, 26, 36, 255));
	xgeXuiWidgetSetRadius(pApp->pRootPanel, 8.0f);
	xgeXuiWidgetAdd(pRoot, pApp->pRootPanel);
	xgeXuiWidgetAdd(pApp->pRootPanel, pApp->pStatusWidget);
	xgeXuiWidgetAdd(pApp->pRootPanel, pApp->pInputLabelWidget);
	xgeXuiWidgetAdd(pApp->pRootPanel, pApp->pInputWidget);
	xgeXuiWidgetAdd(pApp->pRootPanel, pApp->pHintWidget);

	xgeXuiWidgetSetBackground(pApp->pStatusWidget, XGE_COLOR_RGBA(44, 56, 74, 255));
	xgeXuiWidgetSetPaddingPx(pApp->pStatusWidget, 10.0f, 8.0f, 10.0f, 8.0f);
	xgeXuiWidgetSetPaddingPx(pApp->pInputWidget, 8.0f, 7.0f, 8.0f, 7.0f);

	if ( xgeXuiLabelInit(&pApp->tStatusLabel, pApp->pStatusWidget, pFont, "xui input nav policy lab") != XGE_OK ) {
		return XGE_ERROR;
	}
	if ( xgeXuiLabelInit(&pApp->tInputLabel, pApp->pInputLabelWidget, pFont, "Input") != XGE_OK ) {
		return XGE_ERROR;
	}
	if ( xgeXuiLabelInit(&pApp->tHintLabel, pApp->pHintWidget, pFont, "Auto-check covers Ctrl+Left/Right, Home/End, selection collapse and scroll tracking.") != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiLabelSetColor(&pApp->tStatusLabel, XGE_COLOR_RGBA(248, 250, 252, 255));
	xgeXuiLabelSetColor(&pApp->tInputLabel, XGE_COLOR_RGBA(188, 204, 224, 255));
	xgeXuiLabelSetColor(&pApp->tHintLabel, XGE_COLOR_RGBA(164, 182, 206, 255));

	if ( xgeXuiInputInit(&pApp->tInput, &pApp->tXui, pApp->pInputWidget, pFont) != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiInputSetColors(
		&pApp->tInput,
		XGE_COLOR_RGBA(242, 246, 252, 255),
		XGE_COLOR_RGBA(28, 36, 48, 255),
		XGE_COLOR_RGBA(42, 58, 82, 255),
		XGE_COLOR_RGBA(255, 214, 92, 255));
	xgeXuiInputSetText(&pApp->tInput, g_sNavText);
	return XGE_OK;
}

static int RunChecks(app_state_t* pApp)
{
	xge_event_t tEvent;
	int iResult;
	int iAfterLeft1;
	int iAfterLeft2;
	int iAfterRight;

	LayoutRoot(pApp);
	xgeXuiUpdate(&pApp->tXui, 0.0f);
	xgeXuiSetFocus(&pApp->tXui, pApp->pInputWidget);

	pApp->bInitOK =
		(pApp->pInputWidget->procEvent == xgeXuiInputEventProc) &&
		(pApp->pInputWidget->procUpdate == xgeXuiInputUpdateProc) &&
		(pApp->pInputWidget->procPaint == xgeXuiInputPaintProc);

	xgeXuiTextSetCursor(&pApp->tInput.tText, pApp->tInput.tText.iSize);
	MakeKeyEvent(&tEvent, XGE_KEY_LEFT, XGE_KEY_MOD_CTRL);
	iResult = xgeXuiInputEvent(&pApp->tInput, &tEvent);
	iAfterLeft1 = xgeXuiTextGetCursor(&pApp->tInput.tText);
	pApp->bWordOK =
		(iResult == XGE_XUI_EVENT_CONSUMED) &&
		(iAfterLeft1 < pApp->tInput.tText.iSize);
	iResult = xgeXuiInputEvent(&pApp->tInput, &tEvent);
	iAfterLeft2 = xgeXuiTextGetCursor(&pApp->tInput.tText);
	pApp->bWordOK =
		pApp->bWordOK &&
		(iResult == XGE_XUI_EVENT_CONSUMED) &&
		(iAfterLeft2 < iAfterLeft1);
	MakeKeyEvent(&tEvent, XGE_KEY_RIGHT, XGE_KEY_MOD_CTRL);
	iResult = xgeXuiInputEvent(&pApp->tInput, &tEvent);
	iAfterRight = xgeXuiTextGetCursor(&pApp->tInput.tText);
	pApp->bWordOK =
		pApp->bWordOK &&
		(iResult == XGE_XUI_EVENT_CONSUMED) &&
		(iAfterRight > iAfterLeft2) &&
		(iAfterRight <= pApp->tInput.tText.iSize);

	MakeKeyEvent(&tEvent, XGE_KEY_HOME, 0);
	iResult = xgeXuiInputEvent(&pApp->tInput, &tEvent);
	pApp->bHomeEndOK =
		(iResult == XGE_XUI_EVENT_CONSUMED) &&
		(xgeXuiTextGetCursor(&pApp->tInput.tText) == 0);
	MakeKeyEvent(&tEvent, XGE_KEY_END, 0);
	iResult = xgeXuiInputEvent(&pApp->tInput, &tEvent);
	pApp->bHomeEndOK =
		pApp->bHomeEndOK &&
		(iResult == XGE_XUI_EVENT_CONSUMED) &&
		(xgeXuiTextGetCursor(&pApp->tInput.tText) == pApp->tInput.tText.iSize);

	xgeXuiInputSetSelection(&pApp->tInput, 6, 10);
	MakeKeyEvent(&tEvent, XGE_KEY_LEFT, 0);
	iResult = xgeXuiInputEvent(&pApp->tInput, &tEvent);
	pApp->bCollapseOK =
		(iResult == XGE_XUI_EVENT_CONSUMED) &&
		(xgeXuiTextGetCursor(&pApp->tInput.tText) == 6);
	xgeXuiInputSetSelection(&pApp->tInput, 6, 10);
	MakeKeyEvent(&tEvent, XGE_KEY_RIGHT, 0);
	iResult = xgeXuiInputEvent(&pApp->tInput, &tEvent);
	pApp->bCollapseOK =
		pApp->bCollapseOK &&
		(iResult == XGE_XUI_EVENT_CONSUMED) &&
		(xgeXuiTextGetCursor(&pApp->tInput.tText) == 10);

	xgeXuiTextSetCursor(&pApp->tInput.tText, 0);
	MakeKeyEvent(&tEvent, XGE_KEY_END, 0);
	iResult = xgeXuiInputEvent(&pApp->tInput, &tEvent);
	pApp->fScrollValue = pApp->tInput.fScrollX;
	pApp->bScrollOK =
		(iResult == XGE_XUI_EVENT_CONSUMED) &&
		(pApp->fScrollValue > 0.0f);

	xgeXuiInputGetSelection(&pApp->tInput, &pApp->iSelectStart, &pApp->iSelectEnd);
	pApp->iCursorValue = xgeXuiTextGetCursor(&pApp->tInput.tText);
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
	xgeXuiInputUnit(&pApp->tInput);
	xgeXuiLabelUnit(&pApp->tHintLabel);
	xgeXuiLabelUnit(&pApp->tInputLabel);
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
	xgeXuiInputGetSelection(&pApp->tInput, &pApp->iSelectStart, &pApp->iSelectEnd);
	pApp->iCursorValue = xgeXuiTextGetCursor(&pApp->tInput.tText);
	pApp->fScrollValue = pApp->tInput.fScrollX;
	UpdateStatus(pApp);
	xgeXuiUpdate(&pApp->tXui, xgeGetDelta());

	xgeBegin();
	xgeClear(XGE_COLOR_RGBA(18, 24, 34, 255));
	xgeXuiPaint(&pApp->tXui);
	xgeEnd();
	xgePresent();

	pApp->iFrameCount++;
	if ( (pApp->iFrameLimit > 0) && (pApp->iFrameCount >= pApp->iFrameLimit) ) {
		printf(
			"xui-input-nav-policy-lab final-summary frames=%d init=%d word=%d homeend=%d collapse=%d scroll=%d cursor=%d select=%d..%d scroll=%.2f\n",
			pApp->iFrameCount,
			pApp->bInitOK,
			pApp->bWordOK,
			pApp->bHomeEndOK,
			pApp->bCollapseOK,
			pApp->bScrollOK,
			pApp->iCursorValue,
			pApp->iSelectStart,
			pApp->iSelectEnd,
			pApp->fScrollValue);
		printf("xui-input-nav-policy-lab summary frames=%d/%d\n", pApp->iFrameCount, pApp->iFrameLimit);
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

	iFrameLimit = 0;
	for ( i = 1; i < argc; i++ ) {
		if ( strcmp(argv[i], "--frames") == 0 && (i + 1) < argc ) {
			iFrameLimit = ArgInt(argv[i + 1], iFrameLimit);
			i++;
		}
	}

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iWidth = 820;
	tDesc.iHeight = 280;
	tDesc.sTitle = "XGE XUI Input Nav Policy Lab";
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
	iExitCode =
		(tApp.bInitOK && tApp.bWordOK && tApp.bHomeEndOK &&
		 tApp.bCollapseOK && tApp.bScrollOK) ? 0 : 3;
	AppUnit(&tApp);
	xgeUnit();
	return iExitCode;
}

