#include "../../xge.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct app_state_t {
	xge_scene_t tScene;
	xge_xui_context_t tXui;
	xge_font_t tFont;
	xge_xui_widget pPanel;
	xge_xui_widget pStatus;
	xge_xui_widget pIntegerLabel;
	xge_xui_widget pIntegerInput;
	xge_xui_widget pFloatLabel;
	xge_xui_widget pFloatInput;
	xge_xui_panel_t tPanel;
	xge_xui_label_t tStatus;
	xge_xui_label_t tIntegerLabel;
	xge_xui_label_t tFloatLabel;
	xge_xui_numeric_input_t tIntegerInput;
	xge_xui_numeric_input_t tFloatInput;
	int bFontReady;
	int iFrameLimit;
	int iFrameCount;
	int iIntegerChanges;
	int iFloatChanges;
	int bInitOK;
	int bKeyboardOK;
	int bSpinnerOK;
	int bErrorOK;
	int bBlurOK;
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

static int LoadFont(app_state_t* pApp)
{
	const char* arrFonts[] = {
		"C:/Windows/Fonts/simsun.ttc",
		"C:/Windows/Fonts/simhei.ttf",
		"C:/Windows/Fonts/msyh.ttc",
		"C:/Windows/Fonts/arial.ttf"
	};
	int i;

	for ( i = 0; i < (int)(sizeof(arrFonts) / sizeof(arrFonts[0])); i++ ) {
		memset(&pApp->tFont, 0, sizeof(pApp->tFont));
		if ( xgeFontLoad(&pApp->tFont, arrFonts[i], 12.0f) == XGE_OK ) {
			pApp->bFontReady = 1;
			printf("xui-numeric-input-lab font loaded: %s\n", arrFonts[i]);
			return XGE_OK;
		}
	}
	printf("xui-numeric-input-lab font load failed\n");
	return XGE_ERROR_RESOURCE_FAILED;
}

static void NumericChange(xge_xui_widget pWidget, float fValue, void* pUser)
{
	app_state_t* pApp;

	(void)fValue;
	pApp = (app_state_t*)pUser;
	if ( pApp == NULL ) {
		return;
	}
	if ( pWidget == pApp->pIntegerInput ) {
		pApp->iIntegerChanges++;
	} else if ( pWidget == pApp->pFloatInput ) {
		pApp->iFloatChanges++;
	}
}

static int FloatFormat(float fValue, char* sBuffer, int iSize, void* pUser)
{
	(void)pUser;
	if ( (sBuffer == NULL) || (iSize <= 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	snprintf(sBuffer, (size_t)iSize, "%.2f units", fValue);
	sBuffer[iSize - 1] = 0;
	return XGE_OK;
}

static void MakeKey(xge_event_t* pEvent, int iKey)
{
	memset(pEvent, 0, sizeof(*pEvent));
	pEvent->iType = XGE_EVENT_KEY_DOWN;
	pEvent->iParam1 = iKey;
}

static void MakeMouse(xge_event_t* pEvent, float fX, float fY)
{
	memset(pEvent, 0, sizeof(*pEvent));
	pEvent->iType = XGE_EVENT_MOUSE_DOWN;
	pEvent->iParam1 = XGE_MOUSE_LEFT;
	pEvent->fX = fX;
	pEvent->fY = fY;
}

static int CreateUI(app_state_t* pApp)
{
	xge_xui_widget pRoot;
	xge_font pFont;

	pRoot = xgeXuiRoot(&pApp->tXui);
	pFont = pApp->bFontReady ? &pApp->tFont : NULL;
	if ( pRoot == NULL ) {
		return XGE_ERROR;
	}
	pApp->pPanel = xgeXuiWidgetCreate();
	pApp->pStatus = xgeXuiWidgetCreate();
	pApp->pIntegerLabel = xgeXuiWidgetCreate();
	pApp->pIntegerInput = xgeXuiWidgetCreate();
	pApp->pFloatLabel = xgeXuiWidgetCreate();
	pApp->pFloatInput = xgeXuiWidgetCreate();
	if ( (pApp->pPanel == NULL) || (pApp->pStatus == NULL) || (pApp->pIntegerLabel == NULL) || (pApp->pIntegerInput == NULL) || (pApp->pFloatLabel == NULL) || (pApp->pFloatInput == NULL) ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	xgeXuiWidgetSetRect(pApp->pPanel, (xge_rect_t){ 24.0f, 22.0f, 450.0f, 206.0f });
	xgeXuiWidgetSetPaddingPx(pApp->pPanel, 16.0f, 12.0f, 16.0f, 12.0f);
	xgeXuiPanelInit(&pApp->tPanel, pApp->pPanel);
	xgeXuiPanelSetTitle(&pApp->tPanel, pFont, "NumericInput");
	if ( xgeXuiWidgetAdd(pRoot, pApp->pPanel) != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiWidgetSetRect(pApp->pStatus, (xge_rect_t){ 18.0f, 34.0f, 410.0f, 26.0f });
	xgeXuiLabelInit(&pApp->tStatus, pApp->pStatus, pFont, "");
	xgeXuiWidgetAdd(pApp->pPanel, pApp->pStatus);
	xgeXuiWidgetSetRect(pApp->pIntegerLabel, (xge_rect_t){ 18.0f, 82.0f, 130.0f, 24.0f });
	xgeXuiLabelInit(&pApp->tIntegerLabel, pApp->pIntegerLabel, pFont, "integer step 2");
	xgeXuiWidgetAdd(pApp->pPanel, pApp->pIntegerLabel);
	xgeXuiWidgetSetRect(pApp->pIntegerInput, (xge_rect_t){ 156.0f, 78.0f, 160.0f, 30.0f });
	xgeXuiWidgetSetPaddingPx(pApp->pIntegerInput, 8.0f, 5.0f, 24.0f, 5.0f);
	xgeXuiNumericInputInit(&pApp->tIntegerInput, &pApp->tXui, pApp->pIntegerInput, pFont);
	xgeXuiNumericInputSetInteger(&pApp->tIntegerInput, 1);
	xgeXuiNumericInputSetRange(&pApp->tIntegerInput, -10.0f, 10.0f);
	xgeXuiNumericInputSetStep(&pApp->tIntegerInput, 2.0f);
	xgeXuiNumericInputSetValue(&pApp->tIntegerInput, 4.0f);
	xgeXuiNumericInputSetChange(&pApp->tIntegerInput, NumericChange, pApp);
	xgeXuiWidgetAdd(pApp->pPanel, pApp->pIntegerInput);
	xgeXuiWidgetSetRect(pApp->pFloatLabel, (xge_rect_t){ 18.0f, 126.0f, 130.0f, 24.0f });
	xgeXuiLabelInit(&pApp->tFloatLabel, pApp->pFloatLabel, pFont, "float step 0.25");
	xgeXuiWidgetAdd(pApp->pPanel, pApp->pFloatLabel);
	xgeXuiWidgetSetRect(pApp->pFloatInput, (xge_rect_t){ 156.0f, 122.0f, 160.0f, 30.0f });
	xgeXuiWidgetSetPaddingPx(pApp->pFloatInput, 8.0f, 5.0f, 24.0f, 5.0f);
	xgeXuiNumericInputInit(&pApp->tFloatInput, &pApp->tXui, pApp->pFloatInput, pFont);
	xgeXuiNumericInputSetRange(&pApp->tFloatInput, -1.0f, 1.0f);
	xgeXuiNumericInputSetStep(&pApp->tFloatInput, 0.25f);
	xgeXuiNumericInputSetValue(&pApp->tFloatInput, 0.5f);
	xgeXuiNumericInputSetFormatter(&pApp->tFloatInput, FloatFormat, NULL);
	xgeXuiNumericInputSetChange(&pApp->tFloatInput, NumericChange, pApp);
	xgeXuiWidgetAdd(pApp->pPanel, pApp->pFloatInput);
	return XGE_OK;
}

static void RunChecks(app_state_t* pApp)
{
	xge_event_t tEvent;

	pApp->bInitOK = (xgeXuiNumericInputGetValue(&pApp->tIntegerInput) == 4.0f) && (strcmp(xgeXuiInputGetText(&pApp->tFloatInput.tInput), "0.50 units") == 0);
	xgeXuiSetFocus(&pApp->tXui, pApp->pIntegerInput);
	MakeKey(&tEvent, XGE_KEY_UP);
	pApp->bKeyboardOK = (xgeXuiDispatchEvent(&pApp->tXui, &tEvent) == XGE_XUI_EVENT_CONSUMED) && (xgeXuiNumericInputGetValue(&pApp->tIntegerInput) == 6.0f);
	MakeMouse(&tEvent, pApp->pIntegerInput->tContentRect.fX + pApp->pIntegerInput->tContentRect.fW - 8.0f, pApp->pIntegerInput->tContentRect.fY + 8.0f);
	pApp->bSpinnerOK = (xgeXuiDispatchEvent(&pApp->tXui, &tEvent) == XGE_XUI_EVENT_CONSUMED) && (xgeXuiNumericInputGetValue(&pApp->tIntegerInput) == 8.0f);
	xgeXuiInputSetText(&pApp->tFloatInput.tInput, "bad");
	xgeXuiSetFocus(&pApp->tXui, pApp->pFloatInput);
	MakeKey(&tEvent, XGE_KEY_ENTER);
	pApp->bErrorOK = (xgeXuiDispatchEvent(&pApp->tXui, &tEvent) == XGE_XUI_EVENT_CONSUMED) && (pApp->tFloatInput.bError != 0);
	xgeXuiInputSetText(&pApp->tFloatInput.tInput, "0.75");
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_XUI_FOCUS_OUT;
	pApp->bBlurOK = (xgeXuiNumericInputEvent(&pApp->tFloatInput, &tEvent) == XGE_XUI_EVENT_CONTINUE) && (xgeXuiNumericInputGetValue(&pApp->tFloatInput) == 0.75f) && (pApp->tFloatInput.bError == 0);
}

static void UpdateStatus(app_state_t* pApp)
{
	char sText[192];

	snprintf(
		sText,
		sizeof(sText),
		"init=%d keyboard=%d spinner=%d error=%d blur=%d int=%.0f float=%.2f cb=%d/%d",
		pApp->bInitOK,
		pApp->bKeyboardOK,
		pApp->bSpinnerOK,
		pApp->bErrorOK,
		pApp->bBlurOK,
		xgeXuiNumericInputGetValue(&pApp->tIntegerInput),
		xgeXuiNumericInputGetValue(&pApp->tFloatInput),
		pApp->iIntegerChanges,
		pApp->iFloatChanges);
	xgeXuiLabelSetText(&pApp->tStatus, sText);
}

static int AppEnter(xge_scene pScene)
{
	app_state_t* pApp;

	pApp = (app_state_t*)pScene->pUser;
	LoadFont(pApp);
	if ( xgeXuiInit(&pApp->tXui) != XGE_OK || CreateUI(pApp) != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiUpdate(&pApp->tXui, 0.0f);
	RunChecks(pApp);
	UpdateStatus(pApp);
	return XGE_OK;
}

static int AppLeave(xge_scene pScene)
{
	app_state_t* pApp;

	pApp = (app_state_t*)pScene->pUser;
	xgeXuiUnit(&pApp->tXui);
	if ( pApp->bFontReady ) {
		xgeFontFree(&pApp->tFont);
	}
	return XGE_OK;
}

static int AppEvent(xge_scene pScene, const xge_event_t* pEvent)
{
	app_state_t* pApp;

	pApp = (app_state_t*)pScene->pUser;
	if ( pEvent->iType == XGE_EVENT_KEY_DOWN && pEvent->iParam1 == XGE_KEY_ESCAPE ) {
		xgeQuit();
		return XGE_OK;
	}
	xgeXuiDispatchEvent(&pApp->tXui, pEvent);
	UpdateStatus(pApp);
	return XGE_OK;
}

static int AppUpdate(xge_scene pScene, float fDelta)
{
	app_state_t* pApp;

	pApp = (app_state_t*)pScene->pUser;
	(void)fDelta;
	xgeXuiUpdate(&pApp->tXui, fDelta);
	pApp->iFrameCount++;
	if ( (pApp->iFrameLimit > 0) && (pApp->iFrameCount >= pApp->iFrameLimit) ) {
		printf(
			"xui-numeric-input-lab final-summary frames=%d init=%d keyboard=%d spinner=%d error=%d blur=%d int=%.0f float=%.2f changes=%d/%d\n",
			pApp->iFrameCount,
			pApp->bInitOK,
			pApp->bKeyboardOK,
			pApp->bSpinnerOK,
			pApp->bErrorOK,
			pApp->bBlurOK,
			xgeXuiNumericInputGetValue(&pApp->tIntegerInput),
			xgeXuiNumericInputGetValue(&pApp->tFloatInput),
			pApp->iIntegerChanges,
			pApp->iFloatChanges);
		printf("xui-numeric-input-lab summary frames=%d/%d\n", pApp->iFrameCount, pApp->iFrameLimit);
		xgeQuit();
	}
	return XGE_OK;
}

static int AppDraw(xge_scene pScene)
{
	app_state_t* pApp;

	pApp = (app_state_t*)pScene->pUser;
	xgeBegin();
	xgeClear(XGE_COLOR_RGBA(238, 248, 255, 255));
	xgeXuiPaint(&pApp->tXui);
	xgeEnd();
	xgePresent();
	return XGE_OK;
}

int main(int argc, char** argv)
{
	xge_desc_t tDesc;
	app_state_t tApp;
	int i;
	int iExitCode;

	memset(&tDesc, 0, sizeof(tDesc));
	memset(&tApp, 0, sizeof(tApp));
	tApp.iFrameLimit = ArgInt(getenv("XGE_XUI_NUMERIC_INPUT_FRAMES"), 0);
	for ( i = 1; i < argc; i++ ) {
		if ( (strcmp(argv[i], "--frames") == 0) && ((i + 1) < argc) ) {
			tApp.iFrameLimit = ArgInt(argv[++i], tApp.iFrameLimit);
		}
	}
	tDesc.iWidth = 500;
	tDesc.iHeight = 260;
	tDesc.sTitle = "XGE XUI NumericInput Lab";
	tDesc.iFlags = XGE_INIT_WINDOW | XGE_INIT_VSYNC;
	tDesc.iRunMode = XGE_RUN_GAME_LOOP;
	tDesc.iTargetFPS = 60;
	if ( xgeInit(&tDesc) != XGE_OK ) {
		return 1;
	}
	tApp.tScene.pUser = &tApp;
	tApp.tScene.onEnter = AppEnter;
	tApp.tScene.onLeave = AppLeave;
	tApp.tScene.onEvent = AppEvent;
	tApp.tScene.onUpdate = AppUpdate;
	tApp.tScene.onDraw = AppDraw;
	if ( xgeSceneSet(&tApp.tScene) != XGE_OK ) {
		xgeUnit();
		return 2;
	}
	iExitCode = xgeRun(NULL, NULL);
	xgeUnit();
	return (iExitCode == XGE_OK && tApp.bInitOK && tApp.bKeyboardOK && tApp.bSpinnerOK && tApp.bErrorOK && tApp.bBlurOK) ? 0 : 3;
}
