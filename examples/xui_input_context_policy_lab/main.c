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
	int bClipboardReady;
	int iMenuOpenCount;
	int bFlagsOK;
	int bActionsOK;
	int bReadonlyGuardOK;
	int bPasswordGuardOK;
	int bDisabledGuardOK;
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

static void MakeMouseEvent(xge_event_t* pEvent, int iType, int iButton, float fX, float fY)
{
	memset(pEvent, 0, sizeof(*pEvent));
	pEvent->iType = iType;
	pEvent->iParam1 = iButton;
	pEvent->fX = fX;
	pEvent->fY = fY;
}

static void MakeTextEvent(xge_event_t* pEvent, uint32_t iCodepoint)
{
	memset(pEvent, 0, sizeof(*pEvent));
	pEvent->iType = XGE_EVENT_TEXT;
	pEvent->iCodepoint = iCodepoint;
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
			printf("xui-input-context-policy-lab font loaded: %s\n", arrFonts[i]);
			return XGE_OK;
		}
	}
	printf("xui-input-context-policy-lab font load failed\n");
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
	if ( tRoot.fH < 240.0f ) {
		tRoot.fH = 240.0f;
	}

	xgeXuiWidgetSetRect(pApp->pRootPanel, tRoot);
	xgeXuiWidgetSetRect(pApp->pStatusWidget, (xge_rect_t){ 0.0f, 0.0f, tRoot.fW, 40.0f });
	xgeXuiWidgetSetRect(pApp->pInputWidget, (xge_rect_t){ 24.0f, 84.0f, tRoot.fW - 48.0f, 40.0f });
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
		"flags=%d actions=%d readonly=%d password=%d disabled=%d menus=%d sel=%d..%d text=%s",
		pApp->bFlagsOK,
		pApp->bActionsOK,
		pApp->bReadonlyGuardOK,
		pApp->bPasswordGuardOK,
		pApp->bDisabledGuardOK,
		pApp->iMenuOpenCount,
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
	tTheme.iAccentColor = XGE_COLOR_RGBA(86, 166, 255, 255);
	tTheme.iStateNormal = XGE_COLOR_RGBA(52, 64, 84, 255);
	tTheme.iStateHover = XGE_COLOR_RGBA(72, 88, 112, 255);
	tTheme.iStateActive = XGE_COLOR_RGBA(44, 58, 78, 255);
	tTheme.iStateFocus = XGE_COLOR_RGBA(54, 74, 104, 255);
	tTheme.iStateDisabled = XGE_COLOR_RGBA(78, 82, 92, 180);
	tTheme.fRadius = 8.0f;
	tTheme.fPadding = 8.0f;
	tTheme.fSpacing = 10.0f;
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
	xgeXuiWidgetSetPaddingPx(pApp->pInputWidget, 8.0f, 7.0f, 8.0f, 7.0f);

	if ( xgeXuiLabelInit(&pApp->tStatusLabel, pApp->pStatusWidget, pFont, "xui input context policy lab") != XGE_OK ) {
		return XGE_ERROR;
	}
	if ( xgeXuiLabelInit(&pApp->tHintLabel, pApp->pHintWidget, pFont, "Auto-check covers menu enabled rules plus SelectAll/Cut/Copy/Paste/Delete for Input only.") != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiLabelSetColor(&pApp->tStatusLabel, XGE_COLOR_RGBA(248, 250, 252, 255));
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
	xgeXuiInputSetText(&pApp->tInput, "alpha beta");
	return XGE_OK;
}

static int OpenMenu(app_state_t* pApp)
{
	xge_event_t tEvent;
	xge_vec2_t tPoint;

	tPoint = ContentPoint(pApp->pInputWidget, 0.12f, 0.5f);
	MakeMouseEvent(&tEvent, XGE_EVENT_XUI_CONTEXT_BEGIN, XGE_MOUSE_RIGHT, tPoint.fX, tPoint.fY);
	if ( xgeXuiInputEvent(&pApp->tInput, &tEvent) != XGE_XUI_EVENT_CONSUMED ) {
		return 0;
	}
	pApp->iMenuOpenCount++;
	return xgeXuiMenuIsOpen(pApp->tInput.pDefaultMenu);
}

static void RunMenuAction(app_state_t* pApp, int iIndex)
{
	pApp->tInput.pDefaultMenu->procSelect(pApp->pInputWidget, iIndex, pApp->tInput.pDefaultMenu->pUser);
}

static int RunChecks(app_state_t* pApp)
{
	xge_event_t tEvent;

	LayoutRoot(pApp);
	xgeXuiUpdate(&pApp->tXui, 0.0f);
	xgeXuiSetFocus(&pApp->tXui, pApp->pInputWidget);
	xgeClipboardSetText("PASTE");
	pApp->bClipboardReady = (xgeClipboardGetText() != NULL) && (xgeClipboardGetText()[0] != 0);

	xgeXuiInputSetReadonly(&pApp->tInput, 0);
	xgeXuiInputSetPassword(&pApp->tInput, 0);
	xgeXuiInputSetDisabled(&pApp->tInput, 0);
	xgeXuiInputSetText(&pApp->tInput, "alpha beta");
	xgeXuiInputSetSelection(&pApp->tInput, 0, 5);
	pApp->bFlagsOK =
		OpenMenu(pApp) &&
		(pApp->tInput.arrDefaultMenuEnabled[0] == 1) &&
		(pApp->tInput.arrDefaultMenuEnabled[1] == 1) &&
		(pApp->tInput.arrDefaultMenuEnabled[2] == 1) &&
		(pApp->tInput.arrDefaultMenuEnabled[3] == pApp->bClipboardReady) &&
		(pApp->tInput.arrDefaultMenuEnabled[4] == 1);
	xgeXuiMenuClose(pApp->tInput.pDefaultMenu);

	RunMenuAction(pApp, 2);
	pApp->bActionsOK = 1;
	if ( pApp->bClipboardReady ) {
		pApp->bActionsOK = (strcmp(xgeClipboardGetText(), "alpha") == 0);
	}
	xgeXuiInputSetSelection(&pApp->tInput, 0, 5);
	RunMenuAction(pApp, 1);
	pApp->bActionsOK =
		pApp->bActionsOK &&
		(strcmp(xgeXuiInputGetText(&pApp->tInput), " beta") == 0);
	if ( pApp->bClipboardReady ) {
		RunMenuAction(pApp, 3);
		pApp->bActionsOK =
			pApp->bActionsOK &&
			(strcmp(xgeXuiInputGetText(&pApp->tInput), "alpha beta") == 0);
	} else {
		xgeXuiInputSetText(&pApp->tInput, "alpha beta");
	}
	xgeXuiInputSetSelection(&pApp->tInput, 0, 5);
	RunMenuAction(pApp, 4);
	pApp->bActionsOK =
		pApp->bActionsOK &&
		(strcmp(xgeXuiInputGetText(&pApp->tInput), " beta") == 0);
	RunMenuAction(pApp, 0);
	xgeXuiInputGetSelection(&pApp->tInput, &pApp->iSelectStart, &pApp->iSelectEnd);
	pApp->bActionsOK =
		pApp->bActionsOK &&
		(pApp->iSelectStart == 0) &&
		(pApp->iSelectEnd == (int)strlen(xgeXuiInputGetText(&pApp->tInput)));

	xgeXuiInputSetReadonly(&pApp->tInput, 1);
	xgeXuiInputSetPassword(&pApp->tInput, 0);
	xgeXuiInputSetSelection(&pApp->tInput, 0, 1);
	pApp->bReadonlyGuardOK =
		OpenMenu(pApp) &&
		(pApp->tInput.arrDefaultMenuEnabled[0] == 1) &&
		(pApp->tInput.arrDefaultMenuEnabled[1] == 0) &&
		(pApp->tInput.arrDefaultMenuEnabled[2] == 1) &&
		(pApp->tInput.arrDefaultMenuEnabled[3] == 0) &&
		(pApp->tInput.arrDefaultMenuEnabled[4] == 0);
	xgeXuiMenuClose(pApp->tInput.pDefaultMenu);

	xgeXuiInputSetReadonly(&pApp->tInput, 0);
	xgeXuiInputSetPassword(&pApp->tInput, 1);
	pApp->bPasswordGuardOK =
		(OpenMenu(pApp) == 0) &&
		(xgeXuiMenuIsOpen(pApp->tInput.pDefaultMenu) == 0);

	xgeXuiInputSetPassword(&pApp->tInput, 0);
	xgeXuiInputSetDisabled(&pApp->tInput, 1);
	MakeTextEvent(&tEvent, 'Z');
	pApp->bDisabledGuardOK =
		(xgeXuiInputEvent(&pApp->tInput, &tEvent) == XGE_XUI_EVENT_CONTINUE) &&
		(strcmp(xgeXuiInputGetText(&pApp->tInput), " beta") == 0) &&
		(xgeXuiWidgetIsEnabled(pApp->pInputWidget) == 0);
	xgeXuiInputSetDisabled(&pApp->tInput, 0);

	xgeXuiInputGetSelection(&pApp->tInput, &pApp->iSelectStart, &pApp->iSelectEnd);
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
	xgeXuiInputGetSelection(&pApp->tInput, &pApp->iSelectStart, &pApp->iSelectEnd);
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
			"xui-input-context-policy-lab final-summary frames=%d flags=%d actions=%d readonly=%d password=%d disabled=%d menus=%d select=%d..%d text=%s\n",
			pApp->iFrameCount,
			pApp->bFlagsOK,
			pApp->bActionsOK,
			pApp->bReadonlyGuardOK,
			pApp->bPasswordGuardOK,
			pApp->bDisabledGuardOK,
			pApp->iMenuOpenCount,
			pApp->iSelectStart,
			pApp->iSelectEnd,
			xgeXuiInputGetText(&pApp->tInput));
		printf("xui-input-context-policy-lab summary frames=%d/%d\n", pApp->iFrameCount, pApp->iFrameLimit);
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
	tDesc.iWidth = 760;
	tDesc.iHeight = 260;
	tDesc.sTitle = "XGE XUI Input Context Policy Lab";
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
		(tApp.bFlagsOK && tApp.bActionsOK && tApp.bReadonlyGuardOK &&
		 tApp.bPasswordGuardOK && tApp.bDisabledGuardOK) ? 0 : 3;
	AppUnit(&tApp);
	xgeUnit();
	return iExitCode;
}

