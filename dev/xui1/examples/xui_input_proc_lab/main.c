#include "../../xge.h"
#include "../xui_demo_style.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct app_state_t {
	xge_xui_context_t tXui;
	xge_font_t tFont;
	xge_xui_widget pRootPanel;
	xge_xui_widget pStatusWidget;
	xge_xui_widget pInputWidget;
	xge_xui_widget pEditWidget;
	xge_xui_label_t tStatusLabel;
	xge_xui_input_t tInput;
	xge_xui_text_edit_t tEdit;
	int bFontReady;
	int iFrameLimit;
	int iFrameCount;
	int iLastWidth;
	int iLastHeight;
	int bInputOK;
	int bEditOK;
	int bSelectionOK;
	int bUpdateOK;
	int bMenuOK;
	int bDirectPaintOK;
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
			printf("xui-input-proc-lab font loaded: %s\n", arrFonts[i]);
			return XGE_OK;
		}
	}
	printf("xui-input-proc-lab font load failed\n");
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
	if ( tRoot.fW < 620.0f ) {
		tRoot.fW = 620.0f;
	}
	if ( tRoot.fH < 380.0f ) {
		tRoot.fH = 380.0f;
	}
	xgeXuiWidgetSetRect(pApp->pRootPanel, tRoot);
	xgeXuiWidgetSetRect(pApp->pStatusWidget, (xge_rect_t){ 0.0f, 0.0f, tRoot.fW, 40.0f });
	xgeXuiWidgetSetRect(pApp->pInputWidget, (xge_rect_t){ 20.0f, 70.0f, tRoot.fW - 40.0f, 40.0f });
	xgeXuiWidgetSetRect(pApp->pEditWidget, (xge_rect_t){ 20.0f, 132.0f, tRoot.fW - 40.0f, tRoot.fH - 164.0f });
	pApp->iLastWidth = iWidth;
	pApp->iLastHeight = iHeight;
}

static void UpdateStatus(app_state_t* pApp)
{
	char sText[256];
	int iStart;
	int iEnd;
	xge_rect_t tInputCandidate;
	xge_rect_t tEditCandidate;

	xgeXuiInputGetSelection(&pApp->tInput, &iStart, &iEnd);
	tInputCandidate = xgeXuiInputGetCandidateRect(&pApp->tInput);
	tEditCandidate = xgeXuiTextEditGetCandidateRect(&pApp->tEdit);
	snprintf(
		sText,
		sizeof(sText),
		"input=%d edit=%d select=%d update=%d menu=%d paint=%d sel=%d..%d cand=%.0f/%.0f",
		pApp->bInputOK,
		pApp->bEditOK,
		pApp->bSelectionOK,
		pApp->bUpdateOK,
		pApp->bMenuOK,
		pApp->bDirectPaintOK,
		iStart,
		iEnd,
		tInputCandidate.fH,
		tEditCandidate.fH);
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
	tTheme.iBackgroundColor = XGE_COLOR_RGBA(18, 24, 34, 255);
	tTheme.iPanelColor = XGE_COLOR_RGBA(32, 40, 54, 255);
	tTheme.iStateNormal = XGE_COLOR_RGBA(50, 64, 84, 255);
	tTheme.iStateHover = XGE_COLOR_RGBA(68, 84, 108, 255);
	tTheme.iStateActive = XGE_COLOR_RGBA(44, 58, 78, 255);
	tTheme.iStateFocus = XGE_COLOR_RGBA(44, 64, 96, 255);
	tTheme.iStateDisabled = XGE_COLOR_RGBA(78, 82, 92, 180);
	xgeXuiSetTheme(&pApp->tXui, &tTheme);
	XgeXuiDemoApplyTheme(&pApp->tXui, pFont);

	pApp->pRootPanel = xgeXuiWidgetCreate();
	pApp->pStatusWidget = xgeXuiWidgetCreate();
	pApp->pInputWidget = xgeXuiWidgetCreate();
	pApp->pEditWidget = xgeXuiWidgetCreate();
	if ( (pApp->pRootPanel == NULL) || (pApp->pStatusWidget == NULL) || (pApp->pInputWidget == NULL) || (pApp->pEditWidget == NULL) ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}

	xgeXuiWidgetAdd(pRoot, pApp->pRootPanel);
	xgeXuiWidgetAdd(pApp->pRootPanel, pApp->pStatusWidget);
	xgeXuiWidgetAdd(pApp->pRootPanel, pApp->pInputWidget);
	xgeXuiWidgetAdd(pApp->pRootPanel, pApp->pEditWidget);
	xgeXuiWidgetSetBackground(pApp->pRootPanel, XGE_COLOR_RGBA(20, 26, 36, 255));
	xgeXuiWidgetSetRadius(pApp->pRootPanel, 8.0f);
	xgeXuiWidgetSetBackground(pApp->pStatusWidget, XGE_COLOR_RGBA(44, 56, 74, 255));
	xgeXuiWidgetSetPaddingPx(pApp->pStatusWidget, 10.0f, 8.0f, 10.0f, 8.0f);
	xgeXuiWidgetSetPaddingPx(pApp->pInputWidget, 8.0f, 7.0f, 8.0f, 7.0f);
	xgeXuiWidgetSetPaddingPx(pApp->pEditWidget, 8.0f, 8.0f, 8.0f, 8.0f);

	if ( xgeXuiLabelInit(&pApp->tStatusLabel, pApp->pStatusWidget, pFont, "xui input proc lab") != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiLabelSetColor(&pApp->tStatusLabel, XGE_COLOR_RGBA(248, 250, 252, 255));

	if ( xgeXuiInputInit(&pApp->tInput, &pApp->tXui, pApp->pInputWidget, pFont) != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiInputSetColors(
		&pApp->tInput,
		XGE_COLOR_RGBA(242, 246, 252, 255),
		XGE_COLOR_RGBA(28, 36, 48, 255),
		XGE_COLOR_RGBA(42, 58, 82, 255),
		XGE_COLOR_RGBA(255, 214, 92, 255));
	xgeXuiInputSetPlaceholder(&pApp->tInput, "Input Event / Update / Paint Proc");
	xgeXuiInputSetText(&pApp->tInput, "alpha beta");

	if ( xgeXuiTextEditInit(&pApp->tEdit, &pApp->tXui, pApp->pEditWidget, pFont) != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiTextEditSetColors(
		&pApp->tEdit,
		XGE_COLOR_RGBA(242, 246, 252, 255),
		XGE_COLOR_RGBA(26, 34, 46, 255),
		XGE_COLOR_RGBA(42, 58, 82, 255),
		XGE_COLOR_RGBA(255, 214, 92, 255));
	xgeXuiTextEditSetText(&pApp->tEdit, "line 1\nline 2");
	xgeXuiTextEditSetWordWrap(&pApp->tEdit, 1);
	return XGE_OK;
}

static int RunStaticChecks(app_state_t* pApp)
{
	xge_event_t tEvent;
	xge_vec2_t tPoint;
	xge_rect_t tCandidate;
	int iStart;
	int iEnd;
	int bInputBlinkStart;
	int bEditBlinkStart;

	LayoutRoot(pApp);
	xgeXuiUpdate(&pApp->tXui, 0.0f);

	pApp->bInputOK =
		(pApp->pInputWidget->procEvent == xgeXuiInputEventProc) &&
		(pApp->pInputWidget->procUpdate == xgeXuiInputUpdateProc) &&
		(pApp->pInputWidget->procPaint == xgeXuiInputPaintProc) &&
		(pApp->tInput.pDefaultMenu != NULL);
	tPoint = ContentPoint(pApp->pInputWidget, 0.04f, 0.5f);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_DOWN, XGE_MOUSE_LEFT, tPoint.fX, tPoint.fY);
	(void)xgeXuiInputEvent(&pApp->tInput, &tEvent);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_UP, XGE_MOUSE_LEFT, tPoint.fX, tPoint.fY);
	(void)xgeXuiInputEventProc(pApp->pInputWidget, &tEvent, &pApp->tInput);
	xgeXuiTextSetCursor(&pApp->tInput.tText, 0);
	MakeTextEvent(&tEvent, 'Z');
	(void)xgeXuiInputEvent(&pApp->tInput, &tEvent);
	pApp->bInputOK =
		pApp->bInputOK &&
		(strcmp(xgeXuiInputGetText(&pApp->tInput), "Zalpha beta") == 0);

	xgeXuiInputSetSelection(&pApp->tInput, 1, 5);
	xgeXuiInputGetSelection(&pApp->tInput, &iStart, &iEnd);
	tCandidate = xgeXuiInputGetCandidateRect(&pApp->tInput);
	pApp->bSelectionOK =
		(iStart == 1) &&
		(iEnd == 5) &&
		FloatNear(tCandidate.fH, pApp->pInputWidget->tContentRect.fH, 0.01f);

	xgeXuiSetFocus(&pApp->tXui, pApp->pInputWidget);
	bInputBlinkStart = pApp->tInput.bCursorVisible;
	xgeXuiInputUpdateProc(pApp->pInputWidget, 0.51f, &pApp->tInput);
	pApp->bUpdateOK = (pApp->tInput.bCursorVisible != bInputBlinkStart);

	MakeMouseEvent(&tEvent, XGE_EVENT_XUI_CONTEXT_BEGIN, XGE_MOUSE_RIGHT, tPoint.fX, tPoint.fY);
	(void)xgeXuiInputEvent(&pApp->tInput, &tEvent);
	pApp->bMenuOK = (pApp->tInput.pDefaultMenu != NULL) && xgeXuiMenuIsOpen(pApp->tInput.pDefaultMenu);
	xgeXuiMenuClose(pApp->tInput.pDefaultMenu);

	pApp->bEditOK =
		(pApp->pEditWidget->procEvent == xgeXuiTextEditEventProc) &&
		(pApp->pEditWidget->procUpdate == xgeXuiTextEditUpdateProc) &&
		(pApp->pEditWidget->procPaint == xgeXuiTextEditPaintProc) &&
		(pApp->tEdit.pDefaultMenu != NULL);
	tPoint = ContentPoint(pApp->pEditWidget, 0.04f, 0.08f);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_DOWN, XGE_MOUSE_LEFT, tPoint.fX, tPoint.fY);
	(void)xgeXuiTextEditEvent(&pApp->tEdit, &tEvent);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_UP, XGE_MOUSE_LEFT, tPoint.fX, tPoint.fY);
	(void)xgeXuiTextEditEventProc(pApp->pEditWidget, &tEvent, &pApp->tEdit);
	MakeKeyEvent(&tEvent, XGE_KEY_END, 0);
	(void)xgeXuiTextEditEvent(&pApp->tEdit, &tEvent);
	MakeTextEvent(&tEvent, 'Q');
	(void)xgeXuiTextEditEventProc(pApp->pEditWidget, &tEvent, &pApp->tEdit);
	pApp->bEditOK =
		pApp->bEditOK &&
		(strstr(xgeXuiTextEditGetText(&pApp->tEdit), "Q") != NULL);

	xgeXuiTextSetSelection(&pApp->tEdit.tText, 0, 4);
	tCandidate = xgeXuiTextEditGetCandidateRect(&pApp->tEdit);
	pApp->bSelectionOK =
		pApp->bSelectionOK &&
		(tCandidate.fH > 8.0f);

	xgeXuiSetFocus(&pApp->tXui, pApp->pEditWidget);
	bEditBlinkStart = pApp->tEdit.bCursorVisible;
	xgeXuiTextEditUpdateProc(pApp->pEditWidget, 0.51f, &pApp->tEdit);
	pApp->bUpdateOK =
		pApp->bUpdateOK &&
		(pApp->tEdit.bCursorVisible != bEditBlinkStart);

	MakeMouseEvent(&tEvent, XGE_EVENT_XUI_CONTEXT_BEGIN, XGE_MOUSE_RIGHT, tPoint.fX, tPoint.fY);
	(void)xgeXuiTextEditEvent(&pApp->tEdit, &tEvent);
	pApp->bMenuOK =
		pApp->bMenuOK &&
		(pApp->tEdit.pDefaultMenu != NULL) &&
		xgeXuiMenuIsOpen(pApp->tEdit.pDefaultMenu);
	xgeXuiMenuClose(pApp->tEdit.pDefaultMenu);

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
	return RunStaticChecks(pApp);
}

static void AppUnit(app_state_t* pApp)
{
	xgeXuiTextEditUnit(&pApp->tEdit);
	xgeXuiInputUnit(&pApp->tInput);
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
	int iStart;
	int iEnd;
	xge_rect_t tInputCandidate;
	xge_rect_t tEditCandidate;

	pApp = (app_state_t*)pUser;
	LayoutRoot(pApp);
	UpdateStatus(pApp);
	xgeXuiUpdate(&pApp->tXui, xgeGetDelta());

	xgeBegin();
	xgeClear(XGE_COLOR_RGBA(18, 24, 34, 255));
	if ( pApp->bDirectPaintOK == 0 ) {
		xgeXuiInputPaintProc(pApp->pInputWidget, &pApp->tInput);
		xgeXuiTextEditPaintProc(pApp->pEditWidget, &pApp->tEdit);
		pApp->bDirectPaintOK = 1;
	}
	xgeXuiPaint(&pApp->tXui);
	xgeEnd();
	xgePresent();

	pApp->iFrameCount++;
	if ( (pApp->iFrameLimit > 0) && (pApp->iFrameCount >= pApp->iFrameLimit) ) {
		xgeXuiInputGetSelection(&pApp->tInput, &iStart, &iEnd);
		tInputCandidate = xgeXuiInputGetCandidateRect(&pApp->tInput);
		tEditCandidate = xgeXuiTextEditGetCandidateRect(&pApp->tEdit);
		printf(
			"xui-input-proc-lab final-summary frames=%d input=%d edit=%d select=%d update=%d menu=%d paint=%d input(sel=%d..%d blink=%d cand=%.2f) edit(blink=%d cand=%.2f text=%d)\n",
			pApp->iFrameCount,
			pApp->bInputOK,
			pApp->bEditOK,
			pApp->bSelectionOK,
			pApp->bUpdateOK,
			pApp->bMenuOK,
			pApp->bDirectPaintOK,
			iStart,
			iEnd,
			pApp->tInput.bCursorVisible,
			tInputCandidate.fH,
			pApp->tEdit.bCursorVisible,
			tEditCandidate.fH,
			(int)strlen(xgeXuiTextEditGetText(&pApp->tEdit)));
		printf("xui-input-proc-lab summary frames=%d/%d\n", pApp->iFrameCount, pApp->iFrameLimit);
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
	tDesc.iWidth = 680;
	tDesc.iHeight = 420;
	tDesc.sTitle = "XGE XUI Input Proc Lab";
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
		(tApp.bInputOK && tApp.bEditOK && tApp.bSelectionOK &&
		 tApp.bUpdateOK && tApp.bMenuOK && tApp.bDirectPaintOK) ? 0 : 3;
	AppUnit(&tApp);
	xgeUnit();
	return iExitCode;
}

