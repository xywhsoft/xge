#include "../../xge.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct app_state_t {
	xge_scene_t tScene;
	xge_xui_context_t tXui;
	xge_font_t tFont;
	xge_xui_widget pRootPanel;
	xge_xui_widget pStatusWidget;
	xge_xui_widget pStatusTextWidget;
	xge_xui_widget pHintWidget;
	xge_xui_widget pInputLabelWidget;
	xge_xui_widget pInputWidget;
	xge_xui_widget pPlaceholderLabelWidget;
	xge_xui_widget pPlaceholderWidget;
	xge_xui_label_t tStatusLabel;
	xge_xui_label_t tHintLabel;
	xge_xui_label_t tInputLabel;
	xge_xui_label_t tPlaceholderLabel;
	xge_xui_input_t tInput;
	xge_xui_input_t tPlaceholderInput;
	int bFontReady;
	int iFrameLimit;
	int iFrameCount;
	int iLastWidth;
	int iLastHeight;
	int bInitOK;
	int bStartOK;
	int bUpdateOK;
	int bMoveOK;
	int bEndOK;
	int bTextOK;
	int bPlaceholderOK;
	xge_rect_t tCandidateBefore;
	xge_rect_t tCandidateAfter;
	char sComposition[64];
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

static void MakeKeyEvent(xge_event_t* pEvent, int iKey, int iMods)
{
	memset(pEvent, 0, sizeof(*pEvent));
	pEvent->iType = XGE_EVENT_KEY_DOWN;
	pEvent->iParam1 = iKey;
	pEvent->iParam2 = iMods;
}

static void MakeTextEvent(xge_event_t* pEvent, uint32_t iCodepoint)
{
	memset(pEvent, 0, sizeof(*pEvent));
	pEvent->iType = XGE_EVENT_TEXT;
	pEvent->iCodepoint = iCodepoint;
}

static void MakeImeEvent(xge_event_t* pEvent, int iType, xge_ime_event_t* pIme, const char* sText)
{
	memset(pEvent, 0, sizeof(*pEvent));
	if ( pIme != NULL ) {
		memset(pIme, 0, sizeof(*pIme));
	}
	pEvent->iType = iType;
	if ( iType == XGE_EVENT_IME_UPDATE ) {
		if ( pIme != NULL ) {
			pIme->sText = sText;
			pEvent->pData = pIme;
		}
	} else if ( iType == XGE_EVENT_IME_END ) {
		pEvent->pData = NULL;
	}
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
			printf("xui-input-ime-policy-lab font loaded: %s\n", arrFonts[i]);
			return XGE_OK;
		}
	}
	printf("xui-input-ime-policy-lab font load failed\n");
	return XGE_ERROR_RESOURCE_FAILED;
}

static void SetupInputColors(xge_xui_input pInput)
{
	xgeXuiInputSetColors(
		pInput,
		XGE_COLOR_RGBA(242, 246, 252, 255),
		XGE_COLOR_RGBA(28, 36, 48, 255),
		XGE_COLOR_RGBA(44, 62, 88, 255),
		XGE_COLOR_RGBA(255, 214, 96, 255));
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
	if ( tRoot.fH < 320.0f ) {
		tRoot.fH = 320.0f;
	}
	fInputW = tRoot.fW - 210.0f;

	xgeXuiWidgetSetRect(pApp->pRootPanel, tRoot);
	xgeXuiWidgetSetRect(pApp->pStatusWidget, (xge_rect_t){ 0.0f, 0.0f, tRoot.fW, 40.0f });
	xgeXuiWidgetSetRect(pApp->pStatusTextWidget, (xge_rect_t){ 10.0f, 8.0f, tRoot.fW - 20.0f, 24.0f });
	xgeXuiWidgetSetRect(pApp->pInputLabelWidget, (xge_rect_t){ 26.0f, 84.0f, 140.0f, 24.0f });
	xgeXuiWidgetSetRect(pApp->pInputWidget, (xge_rect_t){ 182.0f, 78.0f, fInputW, 36.0f });
	xgeXuiWidgetSetRect(pApp->pPlaceholderLabelWidget, (xge_rect_t){ 26.0f, 148.0f, 140.0f, 24.0f });
	xgeXuiWidgetSetRect(pApp->pPlaceholderWidget, (xge_rect_t){ 182.0f, 142.0f, fInputW, 36.0f });
	xgeXuiWidgetSetRect(pApp->pHintWidget, (xge_rect_t){ 26.0f, tRoot.fH - 42.0f, tRoot.fW - 52.0f, 24.0f });

	pApp->iLastWidth = iWidth;
	pApp->iLastHeight = iHeight;
}

static void UpdateStatus(app_state_t* pApp)
{
	char sText[320];
	const char* sComp;

	sComp = pApp->tInput.tText.sComposition != NULL ? pApp->tInput.tText.sComposition : "";
	snprintf(
		sText,
		sizeof(sText),
		"init=%d start=%d update=%d move=%d end=%d text=%d placeholder=%d comp=%s cand=%.0f->%.0f",
		pApp->bInitOK,
		pApp->bStartOK,
		pApp->bUpdateOK,
		pApp->bMoveOK,
		pApp->bEndOK,
		pApp->bTextOK,
		pApp->bPlaceholderOK,
		sComp,
		pApp->tCandidateBefore.fX,
		pApp->tCandidateAfter.fX);
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
	pApp->pStatusTextWidget = xgeXuiWidgetCreate();
	pApp->pHintWidget = xgeXuiWidgetCreate();
	pApp->pInputLabelWidget = xgeXuiWidgetCreate();
	pApp->pInputWidget = xgeXuiWidgetCreate();
	pApp->pPlaceholderLabelWidget = xgeXuiWidgetCreate();
	pApp->pPlaceholderWidget = xgeXuiWidgetCreate();
	if ( (pApp->pRootPanel == NULL) || (pApp->pStatusWidget == NULL) || (pApp->pStatusTextWidget == NULL) || (pApp->pHintWidget == NULL) ||
	     (pApp->pInputLabelWidget == NULL) || (pApp->pInputWidget == NULL) ||
	     (pApp->pPlaceholderLabelWidget == NULL) || (pApp->pPlaceholderWidget == NULL) ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}

	xgeXuiWidgetSetLayout(pApp->pRootPanel, XGE_XUI_LAYOUT_ABSOLUTE);
	xgeXuiWidgetSetLayout(pApp->pStatusWidget, XGE_XUI_LAYOUT_ABSOLUTE);
	xgeXuiWidgetSetBackground(pApp->pRootPanel, XGE_COLOR_RGBA(20, 26, 36, 255));
	xgeXuiWidgetSetRadius(pApp->pRootPanel, 8.0f);
	xgeXuiWidgetAdd(pRoot, pApp->pRootPanel);
	xgeXuiWidgetAdd(pApp->pRootPanel, pApp->pStatusWidget);
	xgeXuiWidgetAdd(pApp->pStatusWidget, pApp->pStatusTextWidget);
	xgeXuiWidgetAdd(pApp->pRootPanel, pApp->pInputLabelWidget);
	xgeXuiWidgetAdd(pApp->pRootPanel, pApp->pInputWidget);
	xgeXuiWidgetAdd(pApp->pRootPanel, pApp->pPlaceholderLabelWidget);
	xgeXuiWidgetAdd(pApp->pRootPanel, pApp->pPlaceholderWidget);
	xgeXuiWidgetAdd(pApp->pRootPanel, pApp->pHintWidget);

	xgeXuiWidgetSetBackground(pApp->pStatusWidget, XGE_COLOR_RGBA(44, 56, 74, 255));
	xgeXuiWidgetSetPaddingPx(pApp->pInputWidget, 8.0f, 7.0f, 8.0f, 7.0f);
	xgeXuiWidgetSetPaddingPx(pApp->pPlaceholderWidget, 8.0f, 7.0f, 8.0f, 7.0f);

	if ( xgeXuiLabelInit(&pApp->tStatusLabel, pApp->pStatusTextWidget, pFont, "xui input ime policy lab") != XGE_OK ) {
		return XGE_ERROR;
	}
	if ( xgeXuiLabelInit(&pApp->tInputLabel, pApp->pInputLabelWidget, pFont, "IME candidate") != XGE_OK ) {
		return XGE_ERROR;
	}
	if ( xgeXuiLabelInit(&pApp->tPlaceholderLabel, pApp->pPlaceholderLabelWidget, pFont, "Placeholder + IME") != XGE_OK ) {
		return XGE_ERROR;
	}
	if ( xgeXuiLabelInit(&pApp->tHintLabel, pApp->pHintWidget, pFont, "Yellow stroke marks candidate rect. Check IME start/update/move/end and placeholder composition.") != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiLabelSetColor(&pApp->tStatusLabel, XGE_COLOR_RGBA(248, 250, 252, 255));
	xgeXuiLabelSetColor(&pApp->tInputLabel, XGE_COLOR_RGBA(214, 224, 238, 255));
	xgeXuiLabelSetColor(&pApp->tPlaceholderLabel, XGE_COLOR_RGBA(214, 224, 238, 255));
	xgeXuiLabelSetColor(&pApp->tHintLabel, XGE_COLOR_RGBA(164, 182, 206, 255));

	if ( xgeXuiInputInit(&pApp->tInput, &pApp->tXui, pApp->pInputWidget, pFont) != XGE_OK ) {
		return XGE_ERROR;
	}
	if ( xgeXuiInputInit(&pApp->tPlaceholderInput, &pApp->tXui, pApp->pPlaceholderWidget, pFont) != XGE_OK ) {
		return XGE_ERROR;
	}
	SetupInputColors(&pApp->tInput);
	SetupInputColors(&pApp->tPlaceholderInput);
	xgeXuiInputSetText(&pApp->tInput, "prefix ");
	xgeXuiInputSetPlaceholder(&pApp->tPlaceholderInput, "Placeholder should hide during composition");
	return XGE_OK;
}

static int RunChecks(app_state_t* pApp)
{
	xge_event_t tEvent;
	xge_ime_event_t tIme;
	int iResult;

	LayoutRoot(pApp);
	xgeXuiUpdate(&pApp->tXui, 0.0f);

	pApp->bInitOK =
		(pApp->pInputWidget->procEvent == xgeXuiInputEventProc) &&
		(pApp->pInputWidget->procUpdate == xgeXuiInputUpdateProc) &&
		(pApp->pInputWidget->procPaint == xgeXuiInputPaintProc) &&
		(pApp->pPlaceholderWidget->procEvent == xgeXuiInputEventProc);

	xgeXuiSetFocus(&pApp->tXui, pApp->pInputWidget);
	MakeImeEvent(&tEvent, XGE_EVENT_IME_START, &tIme, NULL);
	iResult = xgeXuiInputEvent(&pApp->tInput, &tEvent);
	pApp->bStartOK =
		(iResult == XGE_XUI_EVENT_CONSUMED) &&
		(pApp->tInput.tText.sComposition != NULL) &&
		(pApp->tInput.tText.sComposition[0] == 0);

	MakeImeEvent(&tEvent, XGE_EVENT_IME_UPDATE, &tIme, "ni hao");
	iResult = xgeXuiInputEvent(&pApp->tInput, &tEvent);
	pApp->tCandidateBefore = xgeXuiInputGetCandidateRect(&pApp->tInput);
	snprintf(pApp->sComposition, sizeof(pApp->sComposition), "%s", pApp->tInput.tText.sComposition != NULL ? pApp->tInput.tText.sComposition : "");
	pApp->bUpdateOK =
		(iResult == XGE_XUI_EVENT_CONSUMED) &&
		(strcmp(pApp->sComposition, "ni hao") == 0) &&
		(pApp->tCandidateBefore.fW > 0.0f) &&
		(pApp->tCandidateBefore.fH > 0.0f);

	MakeKeyEvent(&tEvent, XGE_KEY_HOME, 0);
	iResult = xgeXuiInputEvent(&pApp->tInput, &tEvent);
	MakeImeEvent(&tEvent, XGE_EVENT_IME_UPDATE, &tIme, "han");
	iResult = (iResult == XGE_XUI_EVENT_CONSUMED) ? xgeXuiInputEvent(&pApp->tInput, &tEvent) : iResult;
	pApp->tCandidateAfter = xgeXuiInputGetCandidateRect(&pApp->tInput);
	pApp->bMoveOK =
		(iResult == XGE_XUI_EVENT_CONSUMED) &&
		(pApp->tCandidateAfter.fX < pApp->tCandidateBefore.fX) &&
		(pApp->tCandidateAfter.fX >= (pApp->pInputWidget->tContentRect.fX - 1.0f));

	MakeImeEvent(&tEvent, XGE_EVENT_IME_END, &tIme, NULL);
	iResult = xgeXuiInputEvent(&pApp->tInput, &tEvent);
	pApp->bEndOK =
		(iResult == XGE_XUI_EVENT_CONSUMED) &&
		(pApp->tInput.tText.sComposition != NULL) &&
		(pApp->tInput.tText.sComposition[0] == 0);

	MakeTextEvent(&tEvent, 'Z');
	iResult = xgeXuiInputEvent(&pApp->tInput, &tEvent);
	pApp->bTextOK =
		(iResult == XGE_XUI_EVENT_CONSUMED) &&
		(strcmp(xgeXuiInputGetText(&pApp->tInput), "Zprefix ") == 0);

	xgeXuiSetFocus(&pApp->tXui, pApp->pPlaceholderWidget);
	MakeImeEvent(&tEvent, XGE_EVENT_IME_START, &tIme, NULL);
	(void)xgeXuiInputEvent(&pApp->tPlaceholderInput, &tEvent);
	MakeImeEvent(&tEvent, XGE_EVENT_IME_UPDATE, &tIme, "zhong");
	iResult = xgeXuiInputEvent(&pApp->tPlaceholderInput, &tEvent);
	pApp->bPlaceholderOK =
		(iResult == XGE_XUI_EVENT_CONSUMED) &&
		(strcmp(xgeXuiInputGetText(&pApp->tPlaceholderInput), "") == 0) &&
		(pApp->tPlaceholderInput.tText.sComposition != NULL) &&
		(strcmp(pApp->tPlaceholderInput.tText.sComposition, "zhong") == 0);

	UpdateStatus(pApp);
	return XGE_OK;
}

static int AppSetup(app_state_t* pApp)
{
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
	xgeXuiInputUnit(&pApp->tPlaceholderInput);
	xgeXuiInputUnit(&pApp->tInput);
	xgeXuiLabelUnit(&pApp->tHintLabel);
	xgeXuiLabelUnit(&pApp->tPlaceholderLabel);
	xgeXuiLabelUnit(&pApp->tInputLabel);
	xgeXuiLabelUnit(&pApp->tStatusLabel);
	if ( pApp->bFontReady ) {
		xgeFontFree(&pApp->tFont);
	}
	xgeXuiUnit(&pApp->tXui);
}

static int AppEnter(xge_scene pScene)
{
	return AppSetup((app_state_t*)pScene->pUser);
}

static int AppLeave(xge_scene pScene)
{
	AppUnit((app_state_t*)pScene->pUser);
	return XGE_OK;
}

static int AppEvent(xge_scene pScene, const xge_event_t* pEvent)
{
	app_state_t* pApp;
	xge_xui_widget pHit;

	pApp = (app_state_t*)pScene->pUser;
	if ( pEvent == NULL ) {
		return XGE_OK;
	}
	if ( (pEvent->iType == XGE_EVENT_MOUSE_DOWN) && (pEvent->iParam1 == XGE_MOUSE_LEFT) ) {
		pHit = xgeXuiHitTest(&pApp->tXui, pEvent->fX, pEvent->fY);
		if ( pHit == NULL || pHit == pApp->pRootPanel || pHit == pApp->pStatusWidget || pHit == pApp->pStatusTextWidget || pHit == pApp->pHintWidget ) {
			xgeXuiSetFocus(&pApp->tXui, NULL);
		}
	}
	if ( xgeXuiDispatchEvent(&pApp->tXui, pEvent) == XGE_XUI_EVENT_CONSUMED ) {
		return XGE_OK;
	}
	if ( (pEvent->iType == XGE_EVENT_KEY_DOWN) && (pEvent->iParam1 == XGE_KEY_ESCAPE) ) {
		xgeQuit();
		return 1;
	}
	return XGE_OK;
}

static int AppUpdate(xge_scene pScene, float fDelta)
{
	app_state_t* pApp;

	pApp = (app_state_t*)pScene->pUser;
	LayoutRoot(pApp);
	UpdateStatus(pApp);
	xgeXuiUpdate(&pApp->tXui, fDelta);

	pApp->iFrameCount++;
	if ( pApp->iFrameCount >= pApp->iFrameLimit ) {
		printf(
			"xui-input-ime-policy-lab final-summary frames=%d init=%d start=%d update=%d move=%d end=%d text=%d placeholder=%d comp=%s cand=%.2f->%.2f text=%s\n",
			pApp->iFrameCount,
			pApp->bInitOK,
			pApp->bStartOK,
			pApp->bUpdateOK,
			pApp->bMoveOK,
			pApp->bEndOK,
			pApp->bTextOK,
			pApp->bPlaceholderOK,
			pApp->sComposition,
			pApp->tCandidateBefore.fX,
			pApp->tCandidateAfter.fX,
			xgeXuiInputGetText(&pApp->tInput));
		printf("xui-input-ime-policy-lab summary frames=%d/%d\n", pApp->iFrameCount, pApp->iFrameLimit);
		xgeQuit();
	}
	return XGE_OK;
}

static int AppDraw(xge_scene pScene)
{
	app_state_t* pApp;
	xge_rect_t tComp;

	pApp = (app_state_t*)pScene->pUser;

	xgeBegin();
	xgeClear(XGE_COLOR_RGBA(18, 24, 34, 255));
	xgeXuiPaint(&pApp->tXui);
	tComp = xgeXuiInputGetCandidateRect(&pApp->tInput);
	xgeShapeRectStrokePx(tComp, 2.0f, XGE_COLOR_RGBA(255, 220, 96, 255));
	xgeEnd();
	xgePresent();
	return XGE_OK;
}

static void AppStateInit(app_state_t* pApp, int iFrameLimit)
{
	memset(pApp, 0, sizeof(*pApp));
	pApp->iFrameLimit = iFrameLimit;
	pApp->tScene.pUser = pApp;
	pApp->tScene.onEnter = AppEnter;
	pApp->tScene.onLeave = AppLeave;
	pApp->tScene.onEvent = AppEvent;
	pApp->tScene.onUpdate = AppUpdate;
	pApp->tScene.onDraw = AppDraw;
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
	AppStateInit(&tApp, iFrameLimit);

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iWidth = 820;
	tDesc.iHeight = 340;
	tDesc.sTitle = "XGE XUI Input IME Policy Lab";
	tDesc.iFlags = XGE_INIT_WINDOW | XGE_INIT_VSYNC;
	tDesc.iRunMode = XGE_RUN_GAME_LOOP;
	tDesc.iTargetFPS = 60;
	if ( xgeInit(&tDesc) != XGE_OK ) {
		return 1;
	}
	if ( xgeSceneSet(&tApp.tScene) != XGE_OK ) {
		xgeUnit();
		return 2;
	}
	xgeRun(NULL, NULL);
	iExitCode =
		(tApp.bInitOK && tApp.bStartOK && tApp.bUpdateOK &&
		 tApp.bMoveOK && tApp.bEndOK && tApp.bTextOK &&
		 tApp.bPlaceholderOK) ? 0 : 3;
	xgeUnit();
	return iExitCode;
}
