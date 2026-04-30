#include "../../xge.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct app_state_t {
	xge_scene_t tScene;
	xge_xui_context_t tXui;
	xge_font_t tFont;
	xge_xui_widget pPanel;
	xge_xui_widget pStatusText;
	xge_xui_widget pStatusBar;
	xge_xui_panel_t tPanel;
	xge_xui_label_t tStatusText;
	xge_xui_status_bar_t tStatusBar;
	int bFontReady;
	int iFrameLimit;
	int iFrameCount;
	int iSelectCount;
	int iLastIndex;
	int iReadyIndex;
	int iProgressIndex;
	int iFlexIndex;
	int iEncodingIndex;
	int bInitOK;
	int bLayoutOK;
	int bFlexOK;
	int bProgressOK;
	int bClickOK;
	int bDisabledOK;
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
			printf("xui-status-bar-lab font loaded: %s\n", arrFonts[i]);
			return XGE_OK;
		}
	}
	printf("xui-status-bar-lab font load failed\n");
	return XGE_ERROR_RESOURCE_FAILED;
}

static void StatusSelect(xge_xui_widget pWidget, int iIndex, void* pUser)
{
	app_state_t* pApp;

	(void)pWidget;
	pApp = (app_state_t*)pUser;
	if ( pApp == NULL ) {
		return;
	}
	pApp->iSelectCount++;
	pApp->iLastIndex = iIndex;
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
	pApp->pPanel = xgeXuiWidgetCreate();
	pApp->pStatusText = xgeXuiWidgetCreate();
	pApp->pStatusBar = xgeXuiWidgetCreate();
	if ( (pApp->pPanel == NULL) || (pApp->pStatusText == NULL) || (pApp->pStatusBar == NULL) ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	xgeXuiWidgetSetRect(pApp->pPanel, (xge_rect_t){ 24.0f, 22.0f, 520.0f, 180.0f });
	xgeXuiWidgetSetPaddingPx(pApp->pPanel, 16.0f, 12.0f, 16.0f, 12.0f);
	xgeXuiPanelInit(&pApp->tPanel, pApp->pPanel);
	xgeXuiPanelSetTitle(&pApp->tPanel, pFont, "StatusBar");
	xgeXuiWidgetAdd(pRoot, pApp->pPanel);
	xgeXuiWidgetSetRect(pApp->pStatusText, (xge_rect_t){ 18.0f, 42.0f, 480.0f, 26.0f });
	xgeXuiLabelInit(&pApp->tStatusText, pApp->pStatusText, pFont, "");
	xgeXuiWidgetAdd(pApp->pPanel, pApp->pStatusText);
	xgeXuiWidgetSetRect(pApp->pStatusBar, (xge_rect_t){ 18.0f, 110.0f, 480.0f, 26.0f });
	xgeXuiWidgetSetPaddingPx(pApp->pStatusBar, 0.0f, 1.0f, 0.0f, 1.0f);
	xgeXuiStatusBarInit(&pApp->tStatusBar, &pApp->tXui, pApp->pStatusBar);
	xgeXuiStatusBarSetFont(&pApp->tStatusBar, pFont);
	xgeXuiStatusBarSetSelect(&pApp->tStatusBar, StatusSelect, pApp);
	pApp->iReadyIndex = xgeXuiStatusBarAddText(&pApp->tStatusBar, XGE_XUI_STATUS_BAR_SECTION_LEFT, "Ready", 74.0f, 1);
	pApp->iProgressIndex = xgeXuiStatusBarAddProgress(&pApp->tStatusBar, XGE_XUI_STATUS_BAR_SECTION_LEFT, 0.0f, 100.0f, 35.0f, 110.0f);
	pApp->iFlexIndex = xgeXuiStatusBarAddFlexibleSpacer(&pApp->tStatusBar, XGE_XUI_STATUS_BAR_SECTION_LEFT, 1.0f);
	xgeXuiStatusBarAddText(&pApp->tStatusBar, XGE_XUI_STATUS_BAR_SECTION_CENTER, "Ln 12, Col 8", 110.0f, 0);
	pApp->iEncodingIndex = xgeXuiStatusBarAddText(&pApp->tStatusBar, XGE_XUI_STATUS_BAR_SECTION_RIGHT, "UTF-8", 72.0f, 1);
	xgeXuiWidgetAdd(pApp->pPanel, pApp->pStatusBar);
	return XGE_OK;
}

static void RunChecks(app_state_t* pApp)
{
	xge_event_t tEvent;

	xgeXuiPaint(&pApp->tXui);
	pApp->bInitOK = (pApp->tStatusBar.iItemCount == 5) && (pApp->iReadyIndex == 0) && (pApp->iProgressIndex == 1) && (pApp->iFlexIndex == 2) && (pApp->iEncodingIndex == 4);
	pApp->bLayoutOK = (pApp->tStatusBar.arrItems[pApp->iReadyIndex].tRect.fW > 0.0f) &&
		(pApp->tStatusBar.arrItems[pApp->iProgressIndex].tRect.fW > 0.0f) &&
		(pApp->tStatusBar.arrItems[pApp->iEncodingIndex].tRect.fW > 0.0f) &&
		(pApp->tStatusBar.arrItems[pApp->iReadyIndex].tRect.fX < pApp->tStatusBar.arrItems[pApp->iProgressIndex].tRect.fX);
	pApp->bFlexOK = (pApp->tStatusBar.arrItems[pApp->iFlexIndex].iType == XGE_XUI_STATUS_BAR_ITEM_SPACER) &&
		(pApp->tStatusBar.arrItems[pApp->iFlexIndex].fFlex > 0.0f) &&
		(pApp->tStatusBar.arrItems[pApp->iFlexIndex].tRect.fW > 100.0f);
	xgeXuiStatusBarSetProgress(&pApp->tStatusBar, pApp->iProgressIndex, 140.0f);
	pApp->bProgressOK = pApp->tStatusBar.arrItems[pApp->iProgressIndex].fValue == 100.0f;
	MakeMouse(&tEvent, XGE_EVENT_MOUSE_DOWN, pApp->tStatusBar.arrItems[pApp->iReadyIndex].tRect.fX + 2.0f, pApp->tStatusBar.arrItems[pApp->iReadyIndex].tRect.fY + 2.0f);
	xgeXuiDispatchEvent(&pApp->tXui, &tEvent);
	MakeMouse(&tEvent, XGE_EVENT_MOUSE_UP, pApp->tStatusBar.arrItems[pApp->iReadyIndex].tRect.fX + 2.0f, pApp->tStatusBar.arrItems[pApp->iReadyIndex].tRect.fY + 2.0f);
	pApp->bClickOK = (xgeXuiDispatchEvent(&pApp->tXui, &tEvent) == XGE_XUI_EVENT_CONSUMED) && (pApp->iLastIndex == pApp->iReadyIndex) && (pApp->iSelectCount == 1);
	xgeXuiStatusBarSetItemEnabled(&pApp->tStatusBar, pApp->iEncodingIndex, 0);
	MakeMouse(&tEvent, XGE_EVENT_MOUSE_DOWN, pApp->tStatusBar.arrItems[pApp->iEncodingIndex].tRect.fX + 2.0f, pApp->tStatusBar.arrItems[pApp->iEncodingIndex].tRect.fY + 2.0f);
	pApp->bDisabledOK = (xgeXuiDispatchEvent(&pApp->tXui, &tEvent) == XGE_XUI_EVENT_CONTINUE) && (pApp->iSelectCount == 1);
}

static void UpdateStatus(app_state_t* pApp)
{
	char sText[192];

	snprintf(
		sText,
		sizeof(sText),
		"init=%d layout=%d flex=%d progress=%d click=%d disabled=%d last=%d cb=%d",
		pApp->bInitOK,
		pApp->bLayoutOK,
		pApp->bFlexOK,
		pApp->bProgressOK,
		pApp->bClickOK,
		pApp->bDisabledOK,
		pApp->iLastIndex,
		pApp->iSelectCount);
	xgeXuiLabelSetText(&pApp->tStatusText, sText);
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
	xgeXuiUpdate(&pApp->tXui, fDelta);
	pApp->iFrameCount++;
	if ( (pApp->iFrameLimit > 0) && (pApp->iFrameCount >= pApp->iFrameLimit) ) {
		printf(
			"xui-status-bar-lab final-summary frames=%d init=%d layout=%d flex=%d progress=%d click=%d disabled=%d last=%d cb=%d\n",
			pApp->iFrameCount,
			pApp->bInitOK,
			pApp->bLayoutOK,
			pApp->bFlexOK,
			pApp->bProgressOK,
			pApp->bClickOK,
			pApp->bDisabledOK,
			pApp->iLastIndex,
			pApp->iSelectCount);
		printf("xui-status-bar-lab summary frames=%d/%d\n", pApp->iFrameCount, pApp->iFrameLimit);
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
	tApp.iFrameLimit = ArgInt(getenv("XGE_XUI_STATUS_BAR_FRAMES"), 0);
	tApp.iLastIndex = -1;
	for ( i = 1; i < argc; i++ ) {
		if ( (strcmp(argv[i], "--frames") == 0) && ((i + 1) < argc) ) {
			tApp.iFrameLimit = ArgInt(argv[++i], tApp.iFrameLimit);
		}
	}
	tDesc.iWidth = 570;
	tDesc.iHeight = 240;
	tDesc.sTitle = "XGE XUI StatusBar Lab";
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
	return (iExitCode == XGE_OK && tApp.bInitOK && tApp.bLayoutOK && tApp.bFlexOK && tApp.bProgressOK && tApp.bClickOK && tApp.bDisabledOK) ? 0 : 3;
}
