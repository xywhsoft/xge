#include "../../xge.h"
#include "../xui_demo_style.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NUMERIC_INPUT_COUNT 8

enum {
	NUMERIC_INTEGER = 0,
	NUMERIC_FLOAT,
	NUMERIC_CUSTOM,
	NUMERIC_NO_SPINNER,
	NUMERIC_DISABLED,
	NUMERIC_READONLY,
	NUMERIC_MIN_BOUND,
	NUMERIC_ERROR
};

typedef struct app_state_t {
	xge_scene_t tScene;
	xge_xui_context_t tXui;
	xge_font_t tFont;
	xge_xui_widget pPanel;
	xge_xui_widget pStatus;
	xge_xui_widget pLabel[NUMERIC_INPUT_COUNT];
	xge_xui_widget pInput[NUMERIC_INPUT_COUNT];
	xge_xui_panel_t tPanel;
	xge_xui_label_t tStatus;
	xge_xui_label_t tLabel[NUMERIC_INPUT_COUNT];
	xge_xui_numeric_input_t tInput[NUMERIC_INPUT_COUNT];
	int bFontReady;
	int iFrameLimit;
	int iFrameCount;
	int iChangeCount[NUMERIC_INPUT_COUNT];
	int bInitOK;
	int bKeyboardOK;
	int bSpinnerOK;
	int bWheelOK;
	int bErrorOK;
	int bReadonlyOK;
	int bDisabledOK;
	int bBoundaryOK;
	int bNoSpinnerOK;
	int bCaptureOK;
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
		"C:/Windows/Fonts/msyh.ttc",
		"C:/Windows/Fonts/simsun.ttc",
		"C:/Windows/Fonts/arial.ttf"
	};
	int i;

	for ( i = 0; i < (int)(sizeof(arrFonts) / sizeof(arrFonts[0])); i++ ) {
		memset(&pApp->tFont, 0, sizeof(pApp->tFont));
		if ( xgeFontLoad(&pApp->tFont, arrFonts[i], XGE_XUI_DEMO_FONT_SIZE) == XGE_OK ) {
			pApp->bFontReady = 1;
			printf("xui_numericinput font loaded: %s\n", arrFonts[i]);
			return XGE_OK;
		}
	}
	printf("xui_numericinput font load failed\n");
	return XGE_ERROR_RESOURCE_FAILED;
}

static void NumericChange(xge_xui_widget pWidget, float fValue, void* pUser)
{
	app_state_t* pApp;
	int i;

	(void)fValue;
	pApp = (app_state_t*)pUser;
	if ( pApp == NULL ) {
		return;
	}
	for ( i = 0; i < NUMERIC_INPUT_COUNT; i++ ) {
		if ( pWidget == pApp->pInput[i] ) {
			pApp->iChangeCount[i]++;
			return;
		}
	}
}

static void MakeKey(xge_event_t* pEvent, int iKey)
{
	memset(pEvent, 0, sizeof(*pEvent));
	pEvent->iType = XGE_EVENT_KEY_DOWN;
	pEvent->iParam1 = iKey;
}

static void MakeMouse(xge_event_t* pEvent, int iType, float fX, float fY)
{
	memset(pEvent, 0, sizeof(*pEvent));
	pEvent->iType = iType;
	pEvent->iParam1 = XGE_MOUSE_LEFT;
	pEvent->fX = fX;
	pEvent->fY = fY;
}

static void MakeWheel(xge_event_t* pEvent, float fX, float fY, float fDY)
{
	memset(pEvent, 0, sizeof(*pEvent));
	pEvent->iType = XGE_EVENT_MOUSE_WHEEL;
	pEvent->fX = fX;
	pEvent->fY = fY;
	pEvent->fDY = fDY;
}

static void InputPoint(app_state_t* pApp, int iIndex, float* pX, float* pY)
{
	xge_xui_widget pWidget;

	pWidget = pApp->pInput[iIndex];
	*pX = pWidget->tBorderRect.fX + pWidget->tBorderRect.fW * 0.5f;
	*pY = pWidget->tBorderRect.fY + pWidget->tBorderRect.fH * 0.5f;
}

static void SpinnerPoint(app_state_t* pApp, int iIndex, int iUp, float* pX, float* pY)
{
	xge_xui_widget pWidget;
	float fSpinnerWidth;

	pWidget = pApp->pInput[iIndex];
	fSpinnerWidth = pApp->tInput[iIndex].fSpinnerWidth;
	*pX = pWidget->tBorderRect.fX + pWidget->tBorderRect.fW - fSpinnerWidth * 0.5f;
	*pY = pWidget->tBorderRect.fY + (iUp ? (pWidget->tBorderRect.fH * 0.25f) : (pWidget->tBorderRect.fH * 0.75f));
}

static int AddNumeric(app_state_t* pApp, int iIndex, const char* sLabel, float fX, float fY, xge_font pFont)
{
	xge_xui_widget pPanel;

	pPanel = pApp->pPanel;
	pApp->pLabel[iIndex] = xgeXuiWidgetCreate();
	pApp->pInput[iIndex] = xgeXuiWidgetCreate();
	if ( (pApp->pLabel[iIndex] == NULL) || (pApp->pInput[iIndex] == NULL) ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	xgeXuiWidgetSetRect(pApp->pLabel[iIndex], (xge_rect_t){ fX, fY, 158.0f, 24.0f });
	xgeXuiLabelInit(&pApp->tLabel[iIndex], pApp->pLabel[iIndex], pFont, sLabel);
	xgeXuiLabelSetColor(&pApp->tLabel[iIndex], XGE_COLOR_RGBA(60, 82, 104, 255));
	xgeXuiWidgetAdd(pPanel, pApp->pLabel[iIndex]);

	xgeXuiWidgetSetRect(pApp->pInput[iIndex], (xge_rect_t){ fX + 166.0f, fY - 3.0f, 148.0f, 30.0f });
	xgeXuiWidgetSetPaddingPx(pApp->pInput[iIndex], 8.0f, 5.0f, 8.0f, 5.0f);
	xgeXuiNumericInputInit(&pApp->tInput[iIndex], &pApp->tXui, pApp->pInput[iIndex], pFont);
	xgeXuiNumericInputSetChange(&pApp->tInput[iIndex], NumericChange, pApp);
	xgeXuiWidgetAdd(pPanel, pApp->pInput[iIndex]);
	return XGE_OK;
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
	XgeXuiDemoApplyTheme(&pApp->tXui, pFont);
	pApp->pPanel = xgeXuiWidgetCreate();
	pApp->pStatus = xgeXuiWidgetCreate();
	if ( (pApp->pPanel == NULL) || (pApp->pStatus == NULL) ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	xgeXuiWidgetSetRect(pApp->pPanel, (xge_rect_t){ 20.0f, 18.0f, 720.0f, 388.0f });
	xgeXuiWidgetSetPaddingPx(pApp->pPanel, 16.0f, 12.0f, 16.0f, 12.0f);
	xgeXuiPanelInit(&pApp->tPanel, pApp->pPanel);
	xgeXuiPanelSetTitle(&pApp->tPanel, pFont, "NumericInput");
	xgeXuiWidgetAdd(pRoot, pApp->pPanel);

	xgeXuiWidgetSetRect(pApp->pStatus, (xge_rect_t){ 18.0f, 32.0f, 680.0f, 30.0f });
	xgeXuiLabelInit(&pApp->tStatus, pApp->pStatus, pFont, "");
	xgeXuiLabelSetColor(&pApp->tStatus, XGE_COLOR_RGBA(36, 82, 118, 255));
	xgeXuiWidgetAdd(pApp->pPanel, pApp->pStatus);

	if ( AddNumeric(pApp, NUMERIC_INTEGER, "integer step 2", 18.0f, 84.0f, pFont) != XGE_OK ||
		AddNumeric(pApp, NUMERIC_FLOAT, "float precision 2", 18.0f, 128.0f, pFont) != XGE_OK ||
		AddNumeric(pApp, NUMERIC_CUSTOM, "custom spinner", 18.0f, 172.0f, pFont) != XGE_OK ||
		AddNumeric(pApp, NUMERIC_NO_SPINNER, "no spinner", 18.0f, 216.0f, pFont) != XGE_OK ||
		AddNumeric(pApp, NUMERIC_DISABLED, "disabled", 374.0f, 84.0f, pFont) != XGE_OK ||
		AddNumeric(pApp, NUMERIC_READONLY, "readonly", 374.0f, 128.0f, pFont) != XGE_OK ||
		AddNumeric(pApp, NUMERIC_MIN_BOUND, "min boundary", 374.0f, 172.0f, pFont) != XGE_OK ||
		AddNumeric(pApp, NUMERIC_ERROR, "error commit", 374.0f, 216.0f, pFont) != XGE_OK ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}

	xgeXuiNumericInputSetInteger(&pApp->tInput[NUMERIC_INTEGER], 1);
	xgeXuiNumericInputSetRange(&pApp->tInput[NUMERIC_INTEGER], -10.0f, 10.0f);
	xgeXuiNumericInputSetStep(&pApp->tInput[NUMERIC_INTEGER], 2.0f);
	xgeXuiNumericInputSetValue(&pApp->tInput[NUMERIC_INTEGER], 4.0f);

	xgeXuiNumericInputSetRange(&pApp->tInput[NUMERIC_FLOAT], -1.0f, 1.0f);
	xgeXuiNumericInputSetStep(&pApp->tInput[NUMERIC_FLOAT], 0.25f);
	xgeXuiNumericInputSetPrecision(&pApp->tInput[NUMERIC_FLOAT], 2);
	xgeXuiNumericInputSetValue(&pApp->tInput[NUMERIC_FLOAT], 0.50f);

	xgeXuiNumericInputSetRange(&pApp->tInput[NUMERIC_CUSTOM], 0.0f, 100.0f);
	xgeXuiNumericInputSetStep(&pApp->tInput[NUMERIC_CUSTOM], 5.0f);
	xgeXuiNumericInputSetValue(&pApp->tInput[NUMERIC_CUSTOM], 25.0f);
	xgeXuiNumericInputSetSpinnerWidth(&pApp->tInput[NUMERIC_CUSTOM], 30.0f);
	xgeXuiNumericInputSetSpinnerColors(
		&pApp->tInput[NUMERIC_CUSTOM],
		XGE_COLOR_RGBA(246, 250, 244, 255),
		XGE_COLOR_RGBA(226, 243, 219, 255),
		XGE_COLOR_RGBA(197, 225, 185, 255),
		XGE_COLOR_RGBA(235, 240, 235, 255),
		XGE_COLOR_RGBA(130, 168, 118, 255),
		XGE_COLOR_RGBA(52, 116, 48, 255),
		XGE_COLOR_RGBA(145, 160, 145, 255));

	xgeXuiNumericInputSetRange(&pApp->tInput[NUMERIC_NO_SPINNER], 0.0f, 10.0f);
	xgeXuiNumericInputSetValue(&pApp->tInput[NUMERIC_NO_SPINNER], 3.0f);
	xgeXuiNumericInputSetSpinnerVisible(&pApp->tInput[NUMERIC_NO_SPINNER], 0);

	xgeXuiNumericInputSetValue(&pApp->tInput[NUMERIC_DISABLED], 5.0f);
	xgeXuiInputSetDisabled(&pApp->tInput[NUMERIC_DISABLED].tInput, 1);

	xgeXuiNumericInputSetValue(&pApp->tInput[NUMERIC_READONLY], 7.0f);
	xgeXuiInputSetReadonly(&pApp->tInput[NUMERIC_READONLY].tInput, 1);

	xgeXuiNumericInputSetInteger(&pApp->tInput[NUMERIC_MIN_BOUND], 1);
	xgeXuiNumericInputSetRange(&pApp->tInput[NUMERIC_MIN_BOUND], 0.0f, 4.0f);
	xgeXuiNumericInputSetStep(&pApp->tInput[NUMERIC_MIN_BOUND], 1.0f);
	xgeXuiNumericInputSetValue(&pApp->tInput[NUMERIC_MIN_BOUND], 0.0f);

	xgeXuiNumericInputSetRange(&pApp->tInput[NUMERIC_ERROR], -1.0f, 1.0f);
	xgeXuiNumericInputSetStep(&pApp->tInput[NUMERIC_ERROR], 0.25f);
	xgeXuiNumericInputSetPrecision(&pApp->tInput[NUMERIC_ERROR], 2);
	xgeXuiNumericInputSetValue(&pApp->tInput[NUMERIC_ERROR], 0.5f);
	return XGE_OK;
}

static void RunChecks(app_state_t* pApp)
{
	xge_event_t tEvent;
	float fX;
	float fY;

	pApp->bInitOK =
		(xgeXuiNumericInputGetValue(&pApp->tInput[NUMERIC_INTEGER]) == 4.0f) &&
		(strcmp(xgeXuiInputGetText(&pApp->tInput[NUMERIC_FLOAT].tInput), "0.50") == 0) &&
		(pApp->tInput[NUMERIC_CUSTOM].fSpinnerWidth == 30.0f) &&
		(pApp->tInput[NUMERIC_NO_SPINNER].bShowSpinner == 0);

	xgeXuiSetFocus(&pApp->tXui, pApp->pInput[NUMERIC_INTEGER]);
	MakeKey(&tEvent, XGE_KEY_UP);
	pApp->bKeyboardOK = (xgeXuiDispatchEvent(&pApp->tXui, &tEvent) == XGE_XUI_EVENT_CONSUMED) && (xgeXuiNumericInputGetValue(&pApp->tInput[NUMERIC_INTEGER]) == 6.0f);

	SpinnerPoint(pApp, NUMERIC_INTEGER, 1, &fX, &fY);
	MakeMouse(&tEvent, XGE_EVENT_MOUSE_DOWN, fX, fY);
	pApp->bCaptureOK = (xgeXuiDispatchEvent(&pApp->tXui, &tEvent) == XGE_XUI_EVENT_CONSUMED) && (pApp->tInput[NUMERIC_INTEGER].iActiveButton == XGE_XUI_NUMERIC_INPUT_BUTTON_UP) && (xgeXuiGetPointerCapture(&pApp->tXui, 0) == pApp->pInput[NUMERIC_INTEGER]);
	MakeMouse(&tEvent, XGE_EVENT_MOUSE_UP, fX, fY);
	pApp->bSpinnerOK = (xgeXuiDispatchEvent(&pApp->tXui, &tEvent) == XGE_XUI_EVENT_CONSUMED) && (xgeXuiNumericInputGetValue(&pApp->tInput[NUMERIC_INTEGER]) == 8.0f);
	pApp->bCaptureOK = pApp->bCaptureOK && (xgeXuiGetPointerCapture(&pApp->tXui, 0) == NULL);

	InputPoint(pApp, NUMERIC_FLOAT, &fX, &fY);
	MakeWheel(&tEvent, fX, fY, 1.0f);
	pApp->bWheelOK = (xgeXuiDispatchEvent(&pApp->tXui, &tEvent) == XGE_XUI_EVENT_CONSUMED) && (xgeXuiNumericInputGetValue(&pApp->tInput[NUMERIC_FLOAT]) == 0.75f);

	SpinnerPoint(pApp, NUMERIC_READONLY, 1, &fX, &fY);
	MakeMouse(&tEvent, XGE_EVENT_MOUSE_DOWN, fX, fY);
	pApp->bReadonlyOK = (xgeXuiDispatchEvent(&pApp->tXui, &tEvent) == XGE_XUI_EVENT_CONSUMED) && (pApp->tInput[NUMERIC_READONLY].iActiveButton == XGE_XUI_NUMERIC_INPUT_BUTTON_NONE) && (xgeXuiNumericInputGetValue(&pApp->tInput[NUMERIC_READONLY]) == 7.0f);
	MakeMouse(&tEvent, XGE_EVENT_MOUSE_UP, fX, fY);
	(void)xgeXuiDispatchEvent(&pApp->tXui, &tEvent);

	InputPoint(pApp, NUMERIC_DISABLED, &fX, &fY);
	MakeWheel(&tEvent, fX, fY, 1.0f);
	pApp->bDisabledOK = (xgeXuiDispatchEvent(&pApp->tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED) && (xgeXuiNumericInputGetValue(&pApp->tInput[NUMERIC_DISABLED]) == 5.0f);

	SpinnerPoint(pApp, NUMERIC_MIN_BOUND, 0, &fX, &fY);
	MakeMouse(&tEvent, XGE_EVENT_MOUSE_DOWN, fX, fY);
	pApp->bBoundaryOK = (xgeXuiDispatchEvent(&pApp->tXui, &tEvent) == XGE_XUI_EVENT_CONSUMED) && (pApp->tInput[NUMERIC_MIN_BOUND].iActiveButton == XGE_XUI_NUMERIC_INPUT_BUTTON_NONE) && (xgeXuiNumericInputGetValue(&pApp->tInput[NUMERIC_MIN_BOUND]) == 0.0f);
	MakeMouse(&tEvent, XGE_EVENT_MOUSE_UP, fX, fY);
	(void)xgeXuiDispatchEvent(&pApp->tXui, &tEvent);

	SpinnerPoint(pApp, NUMERIC_NO_SPINNER, 1, &fX, &fY);
	MakeMouse(&tEvent, XGE_EVENT_MOUSE_DOWN, fX, fY);
	pApp->bNoSpinnerOK = (xgeXuiDispatchEvent(&pApp->tXui, &tEvent) == XGE_XUI_EVENT_CONSUMED) && (pApp->tInput[NUMERIC_NO_SPINNER].iActiveButton == XGE_XUI_NUMERIC_INPUT_BUTTON_NONE) && (xgeXuiNumericInputGetValue(&pApp->tInput[NUMERIC_NO_SPINNER]) == 3.0f);
	MakeMouse(&tEvent, XGE_EVENT_MOUSE_UP, fX, fY);
	(void)xgeXuiDispatchEvent(&pApp->tXui, &tEvent);

	xgeXuiInputSetText(&pApp->tInput[NUMERIC_ERROR].tInput, "bad");
	xgeXuiSetFocus(&pApp->tXui, pApp->pInput[NUMERIC_ERROR]);
	MakeKey(&tEvent, XGE_KEY_ENTER);
	pApp->bErrorOK = (xgeXuiNumericInputEvent(&pApp->tInput[NUMERIC_ERROR], &tEvent) == XGE_XUI_EVENT_CONSUMED) && (pApp->tInput[NUMERIC_ERROR].bError != 0);
}

static void UpdateStatus(app_state_t* pApp)
{
	char sText[256];

	snprintf(
		sText,
		sizeof(sText),
		"init=%d key=%d spin=%d wheel=%d error=%d ro=%d dis=%d bound=%d noSpin=%d capture=%d",
		pApp->bInitOK,
		pApp->bKeyboardOK,
		pApp->bSpinnerOK,
		pApp->bWheelOK,
		pApp->bErrorOK,
		pApp->bReadonlyOK,
		pApp->bDisabledOK,
		pApp->bBoundaryOK,
		pApp->bNoSpinnerOK,
		pApp->bCaptureOK);
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
			"xui_numericinput final-summary frames=%d init=%d key=%d spin=%d wheel=%d error=%d ro=%d dis=%d bound=%d noSpin=%d capture=%d int=%.0f float=%.2f changes=%d/%d/%d\n",
			pApp->iFrameCount,
			pApp->bInitOK,
			pApp->bKeyboardOK,
			pApp->bSpinnerOK,
			pApp->bWheelOK,
			pApp->bErrorOK,
			pApp->bReadonlyOK,
			pApp->bDisabledOK,
			pApp->bBoundaryOK,
			pApp->bNoSpinnerOK,
			pApp->bCaptureOK,
			xgeXuiNumericInputGetValue(&pApp->tInput[NUMERIC_INTEGER]),
			xgeXuiNumericInputGetValue(&pApp->tInput[NUMERIC_FLOAT]),
			pApp->iChangeCount[NUMERIC_INTEGER],
			pApp->iChangeCount[NUMERIC_FLOAT],
			pApp->iChangeCount[NUMERIC_ERROR]);
		xgeQuit();
	}
	return XGE_OK;
}

static int AppDraw(xge_scene pScene)
{
	app_state_t* pApp;

	pApp = (app_state_t*)pScene->pUser;
	xgeBegin();
	xgeClear(XGE_COLOR_RGBA(235, 244, 250, 255));
	xgeXuiPaint(&pApp->tXui);
	xgeEnd();
	xgePresent();
	return XGE_OK;
}

int main(int argc, char** argv)
{
	xge_desc_t tDesc;
	static app_state_t tApp;
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
	tDesc.iWidth = 760;
	tDesc.iHeight = 430;
	tDesc.sTitle = "XUI NumericInput";
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
	return (iExitCode == XGE_OK && tApp.bInitOK && tApp.bKeyboardOK && tApp.bSpinnerOK && tApp.bWheelOK && tApp.bErrorOK && tApp.bReadonlyOK && tApp.bDisabledOK && tApp.bBoundaryOK && tApp.bNoSpinnerOK && tApp.bCaptureOK) ? 0 : 3;
}
