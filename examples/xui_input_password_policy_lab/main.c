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
	xge_xui_widget pInputWidget;
	xge_xui_widget pProbeWidget;
	xge_xui_widget pHintWidget;
	xge_xui_label_t tStatusLabel;
	xge_xui_label_t tHintLabel;
	xge_xui_input_t tInput;
	xge_xui_input_t tProbeInput;
	int bFontReady;
	int iFrameLimit;
	int iFrameCount;
	int iLastWidth;
	int iLastHeight;
	int bInitOK;
	int bMaskOK;
	int bCopyGuardOK;
	int bCutGuardOK;
	int bPasteOK;
	int bReadonlyGuardOK;
	int bImeBlockedOK;
	int bCandidateOK;
	int bClipboardReady;
	int bMouseDown;
	int iSelectStart;
	int iSelectEnd;
	float fCandStartX;
	float fCandEndX;
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

static void MakeImeEvent(xge_event_t* pEvent, int iType, xge_ime_event_t* pIme, const char* sText)
{
	memset(pEvent, 0, sizeof(*pEvent));
	if ( pIme != NULL ) {
		memset(pIme, 0, sizeof(*pIme));
		pIme->sText = sText;
	}
	pEvent->iType = iType;
	pEvent->pData = (iType == XGE_EVENT_IME_END) ? NULL : pIme;
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
			printf("xui-input-password-policy-lab font loaded: %s\n", arrFonts[i]);
			return XGE_OK;
		}
	}
	printf("xui-input-password-policy-lab font load failed\n");
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
	if ( tRoot.fH < 280.0f ) {
		tRoot.fH = 280.0f;
	}

	xgeXuiWidgetSetRect(pApp->pRootPanel, tRoot);
	xgeXuiWidgetSetRect(pApp->pStatusWidget, (xge_rect_t){ 0.0f, 0.0f, tRoot.fW, 40.0f });
	xgeXuiWidgetSetRect(pApp->pStatusTextWidget, (xge_rect_t){ 10.0f, 8.0f, tRoot.fW - 20.0f, 24.0f });
	xgeXuiWidgetSetRect(pApp->pInputWidget, (xge_rect_t){ 24.0f, 92.0f, tRoot.fW - 48.0f, 38.0f });
	xgeXuiWidgetSetRect(pApp->pProbeWidget, (xge_rect_t){ 24.0f, 146.0f, tRoot.fW - 48.0f, 38.0f });
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
		"init=%d mask=%d copy=%d cut=%d paste=%d readonly=%d ime=%d cand=%d clip=%d len=%d select=%d..%d cand=%.0f->%.0f",
		pApp->bInitOK,
		pApp->bMaskOK,
		pApp->bCopyGuardOK,
		pApp->bCutGuardOK,
		pApp->bPasteOK,
		pApp->bReadonlyGuardOK,
		pApp->bImeBlockedOK,
		pApp->bCandidateOK,
		pApp->bClipboardReady,
		(int)strlen(xgeXuiInputGetText(&pApp->tInput)),
		pApp->iSelectStart,
		pApp->iSelectEnd,
		pApp->fCandStartX,
		pApp->fCandEndX);
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
	pApp->pStatusTextWidget = xgeXuiWidgetCreate();
	pApp->pInputWidget = xgeXuiWidgetCreate();
	pApp->pProbeWidget = xgeXuiWidgetCreate();
	pApp->pHintWidget = xgeXuiWidgetCreate();
	if ( (pApp->pRootPanel == NULL) || (pApp->pStatusWidget == NULL) || (pApp->pStatusTextWidget == NULL) || (pApp->pInputWidget == NULL) || (pApp->pProbeWidget == NULL) || (pApp->pHintWidget == NULL) ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}

	xgeXuiWidgetSetLayout(pApp->pRootPanel, XGE_XUI_LAYOUT_ABSOLUTE);
	xgeXuiWidgetSetLayout(pApp->pStatusWidget, XGE_XUI_LAYOUT_ABSOLUTE);
	xgeXuiWidgetSetBackground(pApp->pRootPanel, XGE_COLOR_RGBA(24, 30, 40, 255));
	xgeXuiWidgetSetRadius(pApp->pRootPanel, 8.0f);
	xgeXuiWidgetAdd(pRoot, pApp->pRootPanel);
	xgeXuiWidgetAdd(pApp->pRootPanel, pApp->pStatusWidget);
	xgeXuiWidgetAdd(pApp->pStatusWidget, pApp->pStatusTextWidget);
	xgeXuiWidgetAdd(pApp->pRootPanel, pApp->pInputWidget);
	xgeXuiWidgetAdd(pApp->pRootPanel, pApp->pProbeWidget);
	xgeXuiWidgetAdd(pApp->pRootPanel, pApp->pHintWidget);

	xgeXuiWidgetSetBackground(pApp->pStatusWidget, XGE_COLOR_RGBA(46, 58, 78, 255));
	xgeXuiWidgetSetPaddingPx(pApp->pInputWidget, 10.0f, 7.0f, 10.0f, 7.0f);
	xgeXuiWidgetSetPaddingPx(pApp->pProbeWidget, 10.0f, 7.0f, 10.0f, 7.0f);

	if ( xgeXuiLabelInit(&pApp->tStatusLabel, pApp->pStatusTextWidget, pFont, "xui input password policy lab") != XGE_OK ) {
		return XGE_ERROR;
	}
	if ( xgeXuiLabelInit(&pApp->tHintLabel, pApp->pHintWidget, pFont, "Click the lower plain input to move focus away from password and confirm IME restores after leaving.") != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiLabelSetColor(&pApp->tStatusLabel, XGE_COLOR_RGBA(248, 250, 252, 255));
	xgeXuiLabelSetColor(&pApp->tHintLabel, XGE_COLOR_RGBA(164, 182, 206, 255));

	if ( xgeXuiInputInit(&pApp->tInput, &pApp->tXui, pApp->pInputWidget, pFont) != XGE_OK ) {
		return XGE_ERROR;
	}
	if ( xgeXuiInputInit(&pApp->tProbeInput, &pApp->tXui, pApp->pProbeWidget, pFont) != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiInputSetColors(&pApp->tInput, XGE_COLOR_RGBA(242, 246, 252, 255), XGE_COLOR_RGBA(28, 36, 48, 255), XGE_COLOR_RGBA(42, 58, 82, 255), XGE_COLOR_RGBA(255, 214, 92, 255));
	xgeXuiInputSetColors(&pApp->tProbeInput, XGE_COLOR_RGBA(242, 246, 252, 255), XGE_COLOR_RGBA(28, 36, 48, 255), XGE_COLOR_RGBA(42, 58, 82, 255), XGE_COLOR_RGBA(255, 214, 92, 255));
	xgeXuiInputSetPassword(&pApp->tInput, 1);
	xgeXuiInputSetText(&pApp->tInput, "secret42");
	xgeXuiInputSetPlaceholder(&pApp->tProbeInput, "Plain input for focus-out / IME-restore check");
	return XGE_OK;
}

static int RunChecks(app_state_t* pApp)
{
	xge_event_t tEvent;
	xge_ime_event_t tIme;
	xge_rect_t tCand;

	LayoutRoot(pApp);
	xgeXuiUpdate(&pApp->tXui, 0.0f);
	xgeXuiSetFocus(&pApp->tXui, pApp->pInputWidget);

	pApp->bInitOK =
		(pApp->pInputWidget->procEvent == xgeXuiInputEventProc) &&
		(pApp->pInputWidget->procUpdate == xgeXuiInputUpdateProc) &&
		(pApp->pInputWidget->procPaint == xgeXuiInputPaintProc);

	pApp->bMaskOK = (pApp->tInput.bPassword != 0) && ((int)strlen(xgeXuiInputGetText(&pApp->tInput)) == 8);

	xgeClipboardSetText("seed");
	pApp->bClipboardReady = (xgeClipboardGetText() != NULL);
	if ( pApp->bClipboardReady ) {
		pApp->bClipboardReady = (strcmp(xgeClipboardGetText(), "seed") == 0);
	}

	xgeXuiInputSetReadonly(&pApp->tInput, 0);
	xgeXuiInputSetText(&pApp->tInput, "secret42");
	xgeXuiInputSetSelection(&pApp->tInput, 0, 8);
	MakeKeyEvent(&tEvent, 'C', XGE_KEY_MOD_CTRL);
	(void)xgeXuiInputEvent(&pApp->tInput, &tEvent);
	pApp->bCopyGuardOK = 1;
	if ( pApp->bClipboardReady ) {
		pApp->bCopyGuardOK = (strcmp(xgeClipboardGetText(), "seed") == 0);
	}

	xgeXuiInputSetText(&pApp->tInput, "secret42");
	xgeXuiInputSetSelection(&pApp->tInput, 6, 8);
	MakeKeyEvent(&tEvent, 'X', XGE_KEY_MOD_CTRL);
	(void)xgeXuiInputEvent(&pApp->tInput, &tEvent);
	xgeXuiInputGetSelection(&pApp->tInput, &pApp->iSelectStart, &pApp->iSelectEnd);
	pApp->bCutGuardOK =
		(strcmp(xgeXuiInputGetText(&pApp->tInput), "secret42") == 0) &&
		(pApp->iSelectStart == 6) &&
		(pApp->iSelectEnd == 8);
	if ( pApp->bCutGuardOK && pApp->bClipboardReady ) {
		pApp->bCutGuardOK = (strcmp(xgeClipboardGetText(), "seed") == 0);
	}

	xgeXuiInputSetText(&pApp->tInput, "secret");
	xgeXuiTextSetCursor(&pApp->tInput.tText, 6);
	if ( pApp->bClipboardReady ) {
		xgeClipboardSetText("99");
		MakeKeyEvent(&tEvent, 'V', XGE_KEY_MOD_CTRL);
		(void)xgeXuiInputEvent(&pApp->tInput, &tEvent);
		pApp->bPasteOK = (strcmp(xgeXuiInputGetText(&pApp->tInput), "secret99") == 0);
	} else {
		pApp->bPasteOK = 1;
		xgeXuiInputSetText(&pApp->tInput, "secret99");
	}

	xgeXuiInputSetText(&pApp->tInput, "secret42");
	xgeXuiInputSetSelection(&pApp->tInput, 0, 8);
	xgeXuiInputSetReadonly(&pApp->tInput, 1);
	MakeKeyEvent(&tEvent, 'V', XGE_KEY_MOD_CTRL);
	(void)xgeXuiInputEvent(&pApp->tInput, &tEvent);
	xgeXuiInputGetSelection(&pApp->tInput, &pApp->iSelectStart, &pApp->iSelectEnd);
	pApp->bReadonlyGuardOK =
		(strcmp(xgeXuiInputGetText(&pApp->tInput), "secret42") == 0) &&
		(pApp->iSelectStart == 0) &&
		(pApp->iSelectEnd == 8);
	xgeXuiInputSetReadonly(&pApp->tInput, 0);

	xgeXuiInputSetText(&pApp->tInput, "secret42");
	xgeXuiInputSetSelection(&pApp->tInput, 0, 8);
	MakeImeEvent(&tEvent, XGE_EVENT_IME_START, &tIme, NULL);
	(void)xgeXuiInputEvent(&pApp->tInput, &tEvent);
		MakeImeEvent(&tEvent, XGE_EVENT_IME_UPDATE, &tIme, "ni hao");
		(void)xgeXuiInputEvent(&pApp->tInput, &tEvent);
		memset(&tEvent, 0, sizeof(tEvent));
		tEvent.iType = XGE_EVENT_TEXT;
		tEvent.iCodepoint = 0x4E2D;
		(void)xgeXuiInputEvent(&pApp->tInput, &tEvent);
		MakeImeEvent(&tEvent, XGE_EVENT_IME_END, &tIme, NULL);
	(void)xgeXuiInputEvent(&pApp->tInput, &tEvent);
	pApp->bImeBlockedOK =
		(strcmp(xgeXuiInputGetText(&pApp->tInput), "secret42") == 0) &&
		(pApp->tInput.tText.sComposition != NULL) &&
		(pApp->tInput.tText.sComposition[0] == 0);

	xgeXuiInputSetText(&pApp->tInput, "secret42");
	xgeXuiTextSetCursor(&pApp->tInput.tText, 0);
	tCand = xgeXuiInputGetCandidateRect(&pApp->tInput);
	pApp->fCandStartX = tCand.fX;
	xgeXuiTextSetCursor(&pApp->tInput.tText, 8);
	tCand = xgeXuiInputGetCandidateRect(&pApp->tInput);
	pApp->fCandEndX = tCand.fX;
	pApp->bCandidateOK = (pApp->fCandEndX > pApp->fCandStartX);

	xgeXuiInputSetSelection(&pApp->tInput, 0, 8);
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
	xgeXuiInputUnit(&pApp->tProbeInput);
	xgeXuiInputUnit(&pApp->tInput);
	xgeXuiLabelUnit(&pApp->tHintLabel);
	xgeXuiLabelUnit(&pApp->tStatusLabel);
	if ( pApp->bFontReady ) {
		xgeFontFree(&pApp->tFont);
	}
	xgeXuiUnit(&pApp->tXui);
}

static int AppEnter(xge_scene pScene)
{
	app_state_t* pApp;

	pApp = (app_state_t*)pScene->pUser;
	return AppSetup(pApp);
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
	if ( (pApp->iFrameLimit > 0) && (pApp->iFrameCount >= pApp->iFrameLimit) ) {
		printf(
			"xui-input-password-policy-lab final-summary frames=%d init=%d mask=%d copy=%d cut=%d paste=%d readonly=%d ime=%d cand=%d clip=%d len=%d select=%d..%d cand=%.0f->%.0f\n",
			pApp->iFrameCount,
			pApp->bInitOK,
			pApp->bMaskOK,
			pApp->bCopyGuardOK,
			pApp->bCutGuardOK,
			pApp->bPasteOK,
			pApp->bReadonlyGuardOK,
			pApp->bImeBlockedOK,
			pApp->bCandidateOK,
			pApp->bClipboardReady,
			(int)strlen(xgeXuiInputGetText(&pApp->tInput)),
			pApp->iSelectStart,
			pApp->iSelectEnd,
			pApp->fCandStartX,
			pApp->fCandEndX);
		printf("xui-input-password-policy-lab summary frames=%d/%d\n", pApp->iFrameCount, pApp->iFrameLimit);
		xgeQuit();
	}
	return XGE_OK;
}

static int AppDraw(xge_scene pScene)
{
	app_state_t* pApp;

	pApp = (app_state_t*)pScene->pUser;
	xgeBegin();
	xgeClear(XGE_COLOR_RGBA(20, 26, 34, 255));
	xgeXuiPaint(&pApp->tXui);
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

	iFrameLimit = 0;
	for ( i = 1; i < argc; i++ ) {
		if ( strcmp(argv[i], "--frames") == 0 && (i + 1) < argc ) {
			iFrameLimit = ArgInt(argv[i + 1], iFrameLimit);
			i++;
		}
	}
	AppStateInit(&tApp, iFrameLimit);

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iWidth = 780;
	tDesc.iHeight = 280;
	tDesc.sTitle = "XGE XUI Input Password Policy Lab";
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
	iExitCode = (tApp.bInitOK && tApp.bMaskOK && tApp.bCopyGuardOK && tApp.bCutGuardOK && tApp.bPasteOK && tApp.bReadonlyGuardOK && tApp.bImeBlockedOK && tApp.bCandidateOK) ? 0 : 3;
	xgeUnit();
	return iExitCode;
}

