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
	int bClickOK;
	int bDragOK;
	int bDoubleOK;
	int bFocusOK;
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

static float InputTextX(app_state_t* pApp, const char* sPrefix, float fOffset)
{
	xge_vec2_t tSize;

	tSize = xgeTextMeasure(&pApp->tFont, sPrefix);
	return pApp->pInputWidget->tContentRect.fX + tSize.fX - pApp->tInput.fScrollX + fOffset;
}

static float InputTextY(app_state_t* pApp)
{
	return pApp->pInputWidget->tContentRect.fY + pApp->pInputWidget->tContentRect.fH * 0.5f;
}

static void DispatchMouse(app_state_t* pApp, int iType, float fX, float fY)
{
	xge_event_t tEvent;

	MakeMouseEvent(&tEvent, iType, XGE_MOUSE_LEFT, fX, fY);
	(void)xgeXuiDispatchEvent(&pApp->tXui, &tEvent);
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
			printf("xui-input-pointer-select-lab font loaded: %s\n", arrFonts[i]);
			return XGE_OK;
		}
	}
	printf("xui-input-pointer-select-lab font load failed\n");
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
	int iStart;
	int iEnd;

	xgeXuiInputGetSelection(&pApp->tInput, &iStart, &iEnd);
	snprintf(
		sText,
		sizeof(sText),
		"init=%d click=%d drag=%d double=%d focus=%d cursor=%d select=%d..%d capture=%d",
		pApp->bInitOK,
		pApp->bClickOK,
		pApp->bDragOK,
		pApp->bDoubleOK,
		pApp->bFocusOK,
		xgeXuiTextGetCursor(&pApp->tInput.tText),
		iStart,
		iEnd,
		pApp->tXui.pCapture != NULL);
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

	if ( xgeXuiLabelInit(&pApp->tStatusLabel, pApp->pStatusWidget, pFont, "xui input pointer select lab") != XGE_OK ) {
		return XGE_ERROR;
	}
	if ( xgeXuiLabelInit(&pApp->tHintLabel, pApp->pHintWidget, pFont, "Auto-check covers click caret placement, drag selection, double-click word select, and focus route.") != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiLabelSetColor(&pApp->tStatusLabel, XGE_COLOR_RGBA(248, 250, 252, 255));
	xgeXuiLabelSetColor(&pApp->tHintLabel, XGE_COLOR_RGBA(164, 182, 206, 255));

	if ( xgeXuiInputInit(&pApp->tInput, &pApp->tXui, pApp->pInputWidget, pFont) != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiInputSetColors(&pApp->tInput, XGE_COLOR_RGBA(242, 246, 252, 255), XGE_COLOR_RGBA(28, 36, 48, 255), XGE_COLOR_RGBA(42, 58, 82, 255), XGE_COLOR_RGBA(255, 214, 92, 255));
	xgeXuiInputSetText(&pApp->tInput, "alpha beta gamma");
	return XGE_OK;
}

static int RunChecks(app_state_t* pApp)
{
	float fY;
	float fClickX;
	float fDragStartX;
	float fDragEndX;
	float fDoubleX;
	int iStart;
	int iEnd;

	LayoutRoot(pApp);
	xgeXuiUpdate(&pApp->tXui, 0.0f);
	fY = InputTextY(pApp);

	pApp->bInitOK =
		(pApp->pInputWidget->procEvent == xgeXuiInputEventProc) &&
		(pApp->pInputWidget->procUpdate == xgeXuiInputUpdateProc) &&
		(pApp->pInputWidget->procPaint == xgeXuiInputPaintProc) &&
		(strcmp(xgeXuiInputGetText(&pApp->tInput), "alpha beta gamma") == 0);

	xgeXuiSetFocus(&pApp->tXui, NULL);
	fClickX = InputTextX(pApp, "alph", 2.0f);
	DispatchMouse(pApp, XGE_EVENT_MOUSE_DOWN, fClickX, fY);
	DispatchMouse(pApp, XGE_EVENT_MOUSE_UP, fClickX, fY);
	xgeXuiInputGetSelection(&pApp->tInput, &iStart, &iEnd);
	pApp->bFocusOK =
		(pApp->tXui.pFocus == pApp->pInputWidget) &&
		(pApp->tInput.bCursorVisible == 1) &&
		(iStart == iEnd) &&
		(xgeXuiTextGetCursor(&pApp->tInput.tText) >= 2);

	fClickX = InputTextX(pApp, "alpha beta ga", 2.0f);
	DispatchMouse(pApp, XGE_EVENT_MOUSE_DOWN, fClickX, fY);
	DispatchMouse(pApp, XGE_EVENT_MOUSE_UP, fClickX, fY);
	xgeXuiInputGetSelection(&pApp->tInput, &iStart, &iEnd);
	pApp->bClickOK =
		(pApp->tXui.pFocus == pApp->pInputWidget) &&
		(iStart == iEnd) &&
		(xgeXuiTextGetCursor(&pApp->tInput.tText) >= 12);

	fDragStartX = InputTextX(pApp, "alpha ", 1.0f);
	fDragEndX = InputTextX(pApp, "alpha beta ga", 1.0f);
	DispatchMouse(pApp, XGE_EVENT_MOUSE_DOWN, fDragStartX, fY);
	DispatchMouse(pApp, XGE_EVENT_MOUSE_MOVE, fDragEndX, fY);
	DispatchMouse(pApp, XGE_EVENT_MOUSE_UP, fDragEndX, fY);
	xgeXuiInputGetSelection(&pApp->tInput, &iStart, &iEnd);
	pApp->bDragOK =
		(iStart == 6) &&
		(iEnd == 13) &&
		(pApp->tXui.pCapture == NULL);

	fDoubleX = InputTextX(pApp, "alpha be", 1.0f);
	pApp->tInput.fLastClickTime = xgeTimer();
	pApp->tInput.fLastClickX = fDoubleX;
	pApp->tInput.fLastClickY = fY;
	DispatchMouse(pApp, XGE_EVENT_MOUSE_DOWN, fDoubleX, fY);
	DispatchMouse(pApp, XGE_EVENT_MOUSE_UP, fDoubleX, fY);
	xgeXuiInputGetSelection(&pApp->tInput, &iStart, &iEnd);
	pApp->bDoubleOK =
		(iStart == 6) &&
		(iEnd == 10) &&
		(pApp->tInput.bSelecting == 0);

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
	int iStart;
	int iEnd;

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
		xgeXuiInputGetSelection(&pApp->tInput, &iStart, &iEnd);
		printf(
			"xui-input-pointer-select-lab final-summary frames=%d init=%d click=%d drag=%d double=%d focus=%d cursor=%d select=%d..%d capture=%d\n",
			pApp->iFrameCount,
			pApp->bInitOK,
			pApp->bClickOK,
			pApp->bDragOK,
			pApp->bDoubleOK,
			pApp->bFocusOK,
			xgeXuiTextGetCursor(&pApp->tInput.tText),
			iStart,
			iEnd,
			pApp->tXui.pCapture != NULL);
		printf("xui-input-pointer-select-lab summary frames=%d/%d\n", pApp->iFrameCount, pApp->iFrameLimit);
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
	tDesc.sTitle = "XGE XUI Input Pointer Select Lab";
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
	iExitCode = (tApp.bInitOK && tApp.bClickOK && tApp.bDragOK && tApp.bDoubleOK && tApp.bFocusOK) ? 0 : 3;
	AppUnit(&tApp);
	xgeUnit();
	return iExitCode;
}

