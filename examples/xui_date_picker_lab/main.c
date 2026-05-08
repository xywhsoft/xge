#include "../../xge.h"
#include "../xui_demo_style.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct app_state_t {
	xge_scene_t tScene;
	xge_xui_context_t tXui;
	xge_font_t tFont;
	xge_xui_panel_t tPanel;
	xge_xui_label_t tSummary;
	xge_xui_date_picker_t tPicker;
	xge_xui_widget pPanel;
	xge_xui_widget pSummary;
	xge_xui_widget pPickerWidget;
	int bFontReady;
	int iFrameLimit;
	int iFrameCount;
	int bInitOK;
	int bSelectOK;
	int bKeyboardOK;
	int iChangeCount;
	int iLastYear;
	int iLastMonth;
	int iLastDay;
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
		"C:/Windows/Fonts/msyh.ttc",
		"C:/Windows/Fonts/arial.ttf"
	};
	int i;

	for ( i = 0; i < (int)(sizeof(arrFonts) / sizeof(arrFonts[0])); i++ ) {
		memset(&pApp->tFont, 0, sizeof(pApp->tFont));
		if ( xgeFontLoad(&pApp->tFont, arrFonts[i], XGE_XUI_DEMO_FONT_SIZE) == XGE_OK ) {
			pApp->bFontReady = 1;
			printf("xui-date-picker-lab font loaded: %s\n", arrFonts[i]);
			return XGE_OK;
		}
	}
	printf("xui-date-picker-lab font load failed\n");
	return XGE_ERROR_RESOURCE_FAILED;
}

static void DateChanged(xge_xui_widget pWidget, int iYear, int iMonth, int iDay, void* pUser)
{
	app_state_t* pApp;

	(void)pWidget;
	pApp = (app_state_t*)pUser;
	if ( pApp != NULL ) {
		pApp->iChangeCount++;
		pApp->iLastYear = iYear;
		pApp->iLastMonth = iMonth;
		pApp->iLastDay = iDay;
	}
}

static void MakeMouse(xge_event_t* pEvent, int iType, float fX, float fY)
{
	memset(pEvent, 0, sizeof(*pEvent));
	pEvent->iType = iType;
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
	XgeXuiDemoApplyTheme(&pApp->tXui, pFont);
	pApp->pPanel = xgeXuiWidgetCreate();
	pApp->pSummary = xgeXuiWidgetCreate();
	pApp->pPickerWidget = xgeXuiWidgetCreate();
	if ( (pApp->pPanel == NULL) || (pApp->pSummary == NULL) || (pApp->pPickerWidget == NULL) ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	xgeXuiWidgetSetRect(pApp->pPanel, (xge_rect_t){ 24.0f, 22.0f, 360.0f, 286.0f });
	xgeXuiWidgetSetPaddingPx(pApp->pPanel, 16.0f, 12.0f, 16.0f, 12.0f);
	xgeXuiPanelInit(&pApp->tPanel, pApp->pPanel);
	xgeXuiPanelSetTitle(&pApp->tPanel, pFont, "DatePicker");
	xgeXuiWidgetAdd(pRoot, pApp->pPanel);
	xgeXuiWidgetSetRect(pApp->pSummary, (xge_rect_t){ 18.0f, 38.0f, 316.0f, 24.0f });
	xgeXuiLabelInit(&pApp->tSummary, pApp->pSummary, pFont, "");
	xgeXuiWidgetAdd(pApp->pPanel, pApp->pSummary);
	xgeXuiWidgetSetRect(pApp->pPickerWidget, (xge_rect_t){ 18.0f, 72.0f, 316.0f, 178.0f });
	xgeXuiDatePickerInit(&pApp->tPicker, &pApp->tXui, pApp->pPickerWidget, pFont);
	xgeXuiDatePickerSetChange(&pApp->tPicker, DateChanged, pApp);
	xgeXuiDatePickerSetRange(&pApp->tPicker, 2024, 2, 1, 2024, 12, 31);
	xgeXuiDatePickerSetDate(&pApp->tPicker, 2024, 2, 29);
	xgeXuiWidgetAdd(pApp->pPanel, pApp->pPickerWidget);
	return XGE_OK;
}

static void RunChecks(app_state_t* pApp)
{
	xge_event_t tEvent;
	xge_rect_t tDayRect;

	xgeXuiPaint(&pApp->tXui);
	pApp->bInitOK = pApp->tPicker.iYear == 2024 && pApp->tPicker.iMonth == 2 && pApp->tPicker.iDay == 29 && pApp->tPicker.bHasMin != 0 && pApp->tPicker.bHasMax != 0;
	tDayRect = pApp->tPicker.arrDayRect[18];
	MakeMouse(&tEvent, XGE_EVENT_MOUSE_DOWN, tDayRect.fX + tDayRect.fW * 0.5f, tDayRect.fY + tDayRect.fH * 0.5f);
	xgeXuiDispatchEvent(&pApp->tXui, &tEvent);
	pApp->bSelectOK = pApp->iChangeCount == 1 && pApp->iLastYear == 2024 && pApp->iLastMonth == 2 && pApp->iLastDay == 15;
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_KEY_DOWN;
	tEvent.iParam1 = XGE_KEY_RIGHT;
	xgeXuiDispatchEvent(&pApp->tXui, &tEvent);
	pApp->bKeyboardOK = pApp->iChangeCount == 2 && pApp->iLastYear == 2024 && pApp->iLastMonth == 2 && pApp->iLastDay == 16;
}

static void UpdateSummary(app_state_t* pApp)
{
	char sText[192];

	snprintf(
		sText,
		sizeof(sText),
		"init=%d select=%d keyboard=%d changes=%d date=%04d-%02d-%02d",
		pApp->bInitOK,
		pApp->bSelectOK,
		pApp->bKeyboardOK,
		pApp->iChangeCount,
		pApp->iLastYear,
		pApp->iLastMonth,
		pApp->iLastDay);
	xgeXuiLabelSetText(&pApp->tSummary, sText);
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
	UpdateSummary(pApp);
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
	UpdateSummary(pApp);
	return XGE_OK;
}

static int AppUpdate(xge_scene pScene, float fDelta)
{
	app_state_t* pApp;

	pApp = (app_state_t*)pScene->pUser;
	xgeXuiUpdate(&pApp->tXui, fDelta);
	pApp->iFrameCount++;
	if ( (pApp->iFrameLimit > 0) && (pApp->iFrameCount >= pApp->iFrameLimit) ) {
		printf(
			"xui-date-picker-lab final-summary frames=%d init=%d select=%d keyboard=%d changes=%d date=%04d-%02d-%02d\n",
			pApp->iFrameCount,
			pApp->bInitOK,
			pApp->bSelectOK,
			pApp->bKeyboardOK,
			pApp->iChangeCount,
			pApp->iLastYear,
			pApp->iLastMonth,
			pApp->iLastDay);
		printf("xui-date-picker-lab summary frames=%d/%d\n", pApp->iFrameCount, pApp->iFrameLimit);
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
	tApp.iFrameLimit = ArgInt(getenv("XGE_XUI_DATE_PICKER_FRAMES"), 0);
	for ( i = 1; i < argc; i++ ) {
		if ( (strcmp(argv[i], "--frames") == 0) && ((i + 1) < argc) ) {
			tApp.iFrameLimit = ArgInt(argv[++i], tApp.iFrameLimit);
		}
	}
	tDesc.iWidth = 408;
	tDesc.iHeight = 332;
	tDesc.sTitle = "XGE XUI DatePicker Lab";
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
	return (iExitCode == XGE_OK && tApp.bInitOK && tApp.bSelectOK && tApp.bKeyboardOK) ? 0 : 3;
}
