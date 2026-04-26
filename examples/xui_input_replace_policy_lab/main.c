#include "../../xge.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct app_state_t {
	xge_xui_context_t tXui;
	xge_font_t tFont;
	xge_xui_widget pRootPanel;
	xge_xui_widget pStatusWidget;
	xge_xui_widget pInputWidget;
	xge_xui_widget pHintWidget;
	xge_xui_label_t tStatusLabel;
	xge_xui_label_t tHintLabel;
	xge_xui_input_t tInput;
	int bFontReady;
	int iFrameLimit;
	int iFrameCount;
	int iLastWidth;
	int iLastHeight;
	int bInitOK;
	int bTextReplaceOK;
	int bPasteReplaceOK;
	int bDeleteReplaceOK;
	int bReadonlyGuardOK;
	int bClipboardReady;
	int iSelectStart;
	int iSelectEnd;
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

static void MakeTextEvent(xge_event_t* pEvent, uint32_t iCodepoint)
{
	memset(pEvent, 0, sizeof(*pEvent));
	pEvent->iType = XGE_EVENT_TEXT;
	pEvent->iCodepoint = iCodepoint;
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
			printf("xui-input-replace-policy-lab font loaded: %s\n", arrFonts[i]);
			return XGE_OK;
		}
	}
	printf("xui-input-replace-policy-lab font load failed\n");
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
	if ( tRoot.fW < 760.0f ) {
		tRoot.fW = 760.0f;
	}
	if ( tRoot.fH < 240.0f ) {
		tRoot.fH = 240.0f;
	}

	xgeXuiWidgetSetRect(pApp->pRootPanel, tRoot);
	xgeXuiWidgetSetRect(pApp->pStatusWidget, (xge_rect_t){ 0.0f, 0.0f, tRoot.fW, 40.0f });
	xgeXuiWidgetSetRect(pApp->pInputWidget, (xge_rect_t){ 24.0f, 92.0f, tRoot.fW - 48.0f, 38.0f });
	xgeXuiWidgetSetRect(pApp->pHintWidget, (xge_rect_t){ 24.0f, tRoot.fH - 40.0f, tRoot.fW - 48.0f, 24.0f });

	pApp->iLastWidth = iWidth;
	pApp->iLastHeight = iHeight;
}

static void UpdateStatus(app_state_t* pApp)
{
	char sText[256];

	xgeXuiInputGetSelection(&pApp->tInput, &pApp->iSelectStart, &pApp->iSelectEnd);
	snprintf(
		sText,
		sizeof(sText),
		"init=%d text=%d paste=%d delete=%d readonly=%d clip=%d select=%d..%d text=%s",
		pApp->bInitOK,
		pApp->bTextReplaceOK,
		pApp->bPasteReplaceOK,
		pApp->bDeleteReplaceOK,
		pApp->bReadonlyGuardOK,
		pApp->bClipboardReady,
		pApp->iSelectStart,
		pApp->iSelectEnd,
		xgeXuiInputGetText(&pApp->tInput));
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
	pApp->pInputWidget = xgeXuiWidgetCreate();
	pApp->pHintWidget = xgeXuiWidgetCreate();
	if ( (pApp->pRootPanel == NULL) || (pApp->pStatusWidget == NULL) || (pApp->pInputWidget == NULL) || (pApp->pHintWidget == NULL) ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}

	xgeXuiWidgetSetLayout(pApp->pRootPanel, XGE_XUI_LAYOUT_ABSOLUTE);
	xgeXuiWidgetSetBackground(pApp->pRootPanel, XGE_COLOR_RGBA(24, 30, 40, 255));
	xgeXuiWidgetSetRadius(pApp->pRootPanel, 8.0f);
	xgeXuiWidgetAdd(pRoot, pApp->pRootPanel);
	xgeXuiWidgetAdd(pApp->pRootPanel, pApp->pStatusWidget);
	xgeXuiWidgetAdd(pApp->pRootPanel, pApp->pInputWidget);
	xgeXuiWidgetAdd(pApp->pRootPanel, pApp->pHintWidget);

	xgeXuiWidgetSetBackground(pApp->pStatusWidget, XGE_COLOR_RGBA(46, 58, 78, 255));
	xgeXuiWidgetSetPaddingPx(pApp->pStatusWidget, 10.0f, 8.0f, 10.0f, 8.0f);
	xgeXuiWidgetSetPaddingPx(pApp->pInputWidget, 10.0f, 7.0f, 10.0f, 7.0f);

	if ( xgeXuiLabelInit(&pApp->tStatusLabel, pApp->pStatusWidget, pFont, "xui input replace policy lab") != XGE_OK ) {
		return XGE_ERROR;
	}
	if ( xgeXuiLabelInit(&pApp->tHintLabel, pApp->pHintWidget, pFont, "Auto-check covers selection replace by typing, Ctrl+V, Delete, and readonly guard.") != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiLabelSetColor(&pApp->tStatusLabel, XGE_COLOR_RGBA(248, 250, 252, 255));
	xgeXuiLabelSetColor(&pApp->tHintLabel, XGE_COLOR_RGBA(164, 182, 206, 255));

	if ( xgeXuiInputInit(&pApp->tInput, &pApp->tXui, pApp->pInputWidget, pFont) != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiInputSetColors(&pApp->tInput, XGE_COLOR_RGBA(242, 246, 252, 255), XGE_COLOR_RGBA(28, 36, 48, 255), XGE_COLOR_RGBA(42, 58, 82, 255), XGE_COLOR_RGBA(255, 214, 92, 255));
	xgeXuiInputSetText(&pApp->tInput, "alpha beta");
	return XGE_OK;
}

static int RunChecks(app_state_t* pApp)
{
	xge_event_t tEvent;

	LayoutRoot(pApp);
	xgeXuiUpdate(&pApp->tXui, 0.0f);
	xgeXuiSetFocus(&pApp->tXui, pApp->pInputWidget);

	pApp->bInitOK =
		(pApp->pInputWidget->procEvent == xgeXuiInputEventProc) &&
		(pApp->pInputWidget->procUpdate == xgeXuiInputUpdateProc) &&
		(pApp->pInputWidget->procPaint == xgeXuiInputPaintProc);

	xgeXuiInputSetReadonly(&pApp->tInput, 0);
	xgeXuiInputSetText(&pApp->tInput, "alpha beta");
	xgeXuiInputSetSelection(&pApp->tInput, 6, 10);
	MakeTextEvent(&tEvent, 'X');
	(void)xgeXuiInputEvent(&pApp->tInput, &tEvent);
	xgeXuiInputGetSelection(&pApp->tInput, &pApp->iSelectStart, &pApp->iSelectEnd);
	pApp->bTextReplaceOK =
		(strcmp(xgeXuiInputGetText(&pApp->tInput), "alpha X") == 0) &&
		(pApp->iSelectStart == 7) &&
		(pApp->iSelectEnd == 7);

	xgeXuiInputSetText(&pApp->tInput, "alpha beta");
	xgeXuiInputSetSelection(&pApp->tInput, 6, 10);
	xgeClipboardSetText("Z");
	pApp->bClipboardReady = 0;
	if ( xgeClipboardGetText() != NULL && strcmp(xgeClipboardGetText(), "Z") == 0 ) {
		pApp->bClipboardReady = 1;
		MakeKeyEvent(&tEvent, 'V', XGE_KEY_MOD_CTRL);
		(void)xgeXuiInputEvent(&pApp->tInput, &tEvent);
	} else {
		MakeTextEvent(&tEvent, 'Z');
		(void)xgeXuiInputEvent(&pApp->tInput, &tEvent);
	}
	xgeXuiInputGetSelection(&pApp->tInput, &pApp->iSelectStart, &pApp->iSelectEnd);
	pApp->bPasteReplaceOK =
		(strcmp(xgeXuiInputGetText(&pApp->tInput), "alpha Z") == 0) &&
		(pApp->iSelectStart == 7) &&
		(pApp->iSelectEnd == 7);

	xgeXuiInputSetText(&pApp->tInput, "alpha beta");
	xgeXuiInputSetSelection(&pApp->tInput, 5, 10);
	MakeKeyEvent(&tEvent, XGE_KEY_DELETE, 0);
	(void)xgeXuiInputEvent(&pApp->tInput, &tEvent);
	xgeXuiInputGetSelection(&pApp->tInput, &pApp->iSelectStart, &pApp->iSelectEnd);
	pApp->bDeleteReplaceOK =
		(strcmp(xgeXuiInputGetText(&pApp->tInput), "alpha") == 0) &&
		(pApp->iSelectStart == 5) &&
		(pApp->iSelectEnd == 5);

	xgeXuiInputSetText(&pApp->tInput, "alpha beta");
	xgeXuiInputSetSelection(&pApp->tInput, 6, 10);
	xgeXuiInputSetReadonly(&pApp->tInput, 1);
	MakeTextEvent(&tEvent, 'Q');
	(void)xgeXuiInputEvent(&pApp->tInput, &tEvent);
	MakeKeyEvent(&tEvent, XGE_KEY_BACKSPACE, 0);
	(void)xgeXuiInputEvent(&pApp->tInput, &tEvent);
	MakeKeyEvent(&tEvent, 'V', XGE_KEY_MOD_CTRL);
	(void)xgeXuiInputEvent(&pApp->tInput, &tEvent);
	xgeXuiInputGetSelection(&pApp->tInput, &pApp->iSelectStart, &pApp->iSelectEnd);
	pApp->bReadonlyGuardOK =
		(strcmp(xgeXuiInputGetText(&pApp->tInput), "alpha beta") == 0) &&
		(pApp->iSelectStart == 6) &&
		(pApp->iSelectEnd == 10);
	xgeXuiInputSetReadonly(&pApp->tInput, 0);

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
	if ( (pApp->iFrameLimit > 0) && (pApp->iFrameCount >= pApp->iFrameLimit) ) {
		printf(
			"xui-input-replace-policy-lab final-summary frames=%d init=%d text=%d paste=%d delete=%d readonly=%d clip=%d select=%d..%d text=%s\n",
			pApp->iFrameCount,
			pApp->bInitOK,
			pApp->bTextReplaceOK,
			pApp->bPasteReplaceOK,
			pApp->bDeleteReplaceOK,
			pApp->bReadonlyGuardOK,
			pApp->bClipboardReady,
			pApp->iSelectStart,
			pApp->iSelectEnd,
			xgeXuiInputGetText(&pApp->tInput));
		printf("xui-input-replace-policy-lab summary frames=%d/%d\n", pApp->iFrameCount, pApp->iFrameLimit);
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
	tDesc.iWidth = 780;
	tDesc.iHeight = 240;
	tDesc.sTitle = "XGE XUI Input Replace Policy Lab";
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
	iExitCode = (tApp.bInitOK && tApp.bTextReplaceOK && tApp.bPasteReplaceOK && tApp.bDeleteReplaceOK && tApp.bReadonlyGuardOK) ? 0 : 3;
	AppUnit(&tApp);
	xgeUnit();
	return iExitCode;
}

