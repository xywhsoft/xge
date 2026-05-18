#include "../../xge.h"
#include "../xui_demo_style.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct app_state_t {
	xge_scene_t tScene;
	xge_xui_context_t tXui;
	xge_font_t tFont;
	xge_xui_widget pPanel;
	xge_xui_widget pStatus;
	xge_xui_widget pMainBar;
	xge_xui_widget pSideBar;
	xge_xui_panel_t tPanel;
	xge_xui_label_t tStatus;
	xge_xui_toolbar_t tMainBar;
	xge_xui_toolbar_t tSideBar;
	int bFontReady;
	int iFrameLimit;
	int iFrameCount;
	int iSelectCount;
	int iOverflowCount;
	int iLastIndex;
	int bInitOK;
	int bToggleOK;
	int bDisabledOK;
	int bKeyboardOK;
	int bVerticalOK;
	int bGroupOK;
	int bTooltipOK;
	int bOverflowOK;
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
		"C:/Windows/Fonts/simsun.ttc",
		"C:/Windows/Fonts/msyh.ttc",
		"C:/Windows/Fonts/arial.ttf"
	};
	int i;

	for ( i = 0; i < (int)(sizeof(arrFonts) / sizeof(arrFonts[0])); i++ ) {
		memset(&pApp->tFont, 0, sizeof(pApp->tFont));
		if ( xgeFontLoad(&pApp->tFont, arrFonts[i], XGE_XUI_DEMO_FONT_SIZE) == XGE_OK ) {
			pApp->bFontReady = 1;
			printf("xui-toolbar-lab font loaded: %s\n", arrFonts[i]);
			return XGE_OK;
		}
	}
	printf("xui-toolbar-lab font load failed\n");
	return XGE_ERROR_RESOURCE_FAILED;
}

static void ToolbarSelect(xge_xui_widget pWidget, int iIndex, void* pUser)
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

static void ToolbarOverflow(xge_xui_widget pWidget, void* pUser)
{
	app_state_t* pApp;

	(void)pWidget;
	pApp = (app_state_t*)pUser;
	if ( pApp != NULL ) {
		pApp->iOverflowCount++;
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

static void MakeKey(xge_event_t* pEvent, int iKey)
{
	memset(pEvent, 0, sizeof(*pEvent));
	pEvent->iType = XGE_EVENT_KEY_DOWN;
	pEvent->iParam1 = iKey;
}

static void LayoutRoot(app_state_t* pApp)
{
	xge_xui_widget pRoot;
	float fW;
	float fH;

	pRoot = xgeXuiRoot(&pApp->tXui);
	if ( pRoot == NULL ) {
		return;
	}
	fW = (float)xgeGetWidth();
	fH = (float)xgeGetHeight();
	if ( fW < 560.0f ) {
		fW = 560.0f;
	}
	if ( fH < 270.0f ) {
		fH = 270.0f;
	}
	xgeXuiWidgetSetRect(pRoot, (xge_rect_t){ 0.0f, 0.0f, fW, fH });
}

static int CreateUI(app_state_t* pApp)
{
	xge_xui_widget pRoot;
	xge_font pFont;
	const char* arrMainText[] = { "New", "Pin", "", "Build" };
	int arrMainTypes[] = { XGE_XUI_TOOLBAR_ITEM_BUTTON, XGE_XUI_TOOLBAR_ITEM_TOGGLE, XGE_XUI_TOOLBAR_ITEM_SEPARATOR, XGE_XUI_TOOLBAR_ITEM_BUTTON };
	const char* arrSideText[] = { "Move", "Select", "" };
	int arrSideTypes[] = { XGE_XUI_TOOLBAR_ITEM_BUTTON, XGE_XUI_TOOLBAR_ITEM_TOGGLE, XGE_XUI_TOOLBAR_ITEM_SEPARATOR };

	pRoot = xgeXuiRoot(&pApp->tXui);
	pFont = pApp->bFontReady ? &pApp->tFont : NULL;
	if ( pRoot == NULL ) {
		return XGE_ERROR;
	}
	XgeXuiDemoApplyTheme(&pApp->tXui, pFont);
	LayoutRoot(pApp);
	pApp->pPanel = xgeXuiWidgetCreate();
	pApp->pStatus = xgeXuiWidgetCreate();
	pApp->pMainBar = xgeXuiWidgetCreate();
	pApp->pSideBar = xgeXuiWidgetCreate();
	if ( (pApp->pPanel == NULL) || (pApp->pStatus == NULL) || (pApp->pMainBar == NULL) || (pApp->pSideBar == NULL) ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	xgeXuiWidgetSetRect(pApp->pPanel, (xge_rect_t){ 24.0f, 22.0f, 500.0f, 210.0f });
	xgeXuiWidgetSetPaddingPx(pApp->pPanel, 16.0f, 12.0f, 16.0f, 12.0f);
	xgeXuiPanelInit(&pApp->tPanel, pApp->pPanel);
	xgeXuiPanelSetTitle(&pApp->tPanel, pFont, "Toolbar");
	xgeXuiWidgetAdd(pRoot, pApp->pPanel);
	xgeXuiWidgetSetRect(pApp->pStatus, (xge_rect_t){ 18.0f, 34.0f, 460.0f, 26.0f });
	xgeXuiLabelInit(&pApp->tStatus, pApp->pStatus, pFont, "");
	xgeXuiWidgetAdd(pApp->pPanel, pApp->pStatus);
	xgeXuiWidgetSetRect(pApp->pMainBar, (xge_rect_t){ 18.0f, 78.0f, 268.0f, 30.0f });
	xgeXuiWidgetSetPaddingPx(pApp->pMainBar, 0.0f, 0.0f, 0.0f, 0.0f);
	xgeXuiToolbarInit(&pApp->tMainBar, &pApp->tXui, pApp->pMainBar);
	xgeXuiToolbarSetFont(&pApp->tMainBar, pFont);
	xgeXuiToolbarSetItems(&pApp->tMainBar, arrMainText, arrMainTypes, 4);
	xgeXuiToolbarSetItemEnabled(&pApp->tMainBar, 3, 0);
	xgeXuiToolbarSetItemSize(&pApp->tMainBar, 62.0f, 26.0f, 10.0f);
	xgeXuiToolbarSetGroupGap(&pApp->tMainBar, 12.0f);
	xgeXuiToolbarSetItemGroup(&pApp->tMainBar, 3, 1);
	xgeXuiToolbarSetItemTooltip(&pApp->tMainBar, 0, "Create a new item");
	xgeXuiToolbarSetOverflow(&pApp->tMainBar, 0, 24.0f, ToolbarOverflow, pApp);
	xgeXuiToolbarSetSelect(&pApp->tMainBar, ToolbarSelect, pApp);
	xgeXuiWidgetAdd(pApp->pPanel, pApp->pMainBar);
	xgeXuiWidgetSetRect(pApp->pSideBar, (xge_rect_t){ 320.0f, 74.0f, 82.0f, 100.0f });
	xgeXuiWidgetSetPaddingPx(pApp->pSideBar, 0.0f, 0.0f, 0.0f, 0.0f);
	xgeXuiToolbarInit(&pApp->tSideBar, &pApp->tXui, pApp->pSideBar);
	xgeXuiToolbarSetFont(&pApp->tSideBar, pFont);
	xgeXuiToolbarSetItems(&pApp->tSideBar, arrSideText, arrSideTypes, 3);
	xgeXuiToolbarSetOrientation(&pApp->tSideBar, XGE_XUI_SEPARATOR_VERTICAL);
	xgeXuiToolbarSetItemSize(&pApp->tSideBar, 78.0f, 28.0f, 10.0f);
	xgeXuiToolbarSetSelect(&pApp->tSideBar, ToolbarSelect, pApp);
	xgeXuiWidgetAdd(pApp->pPanel, pApp->pSideBar);
	return XGE_OK;
}

static void RunChecks(app_state_t* pApp)
{
	xge_event_t tEvent;
	xge_rect_t tOverflow;

	xgeXuiPaint(&pApp->tXui);
	pApp->bInitOK = (pApp->tMainBar.iItemCount == 4) && (pApp->tMainBar.arrItems[2].iType == XGE_XUI_TOOLBAR_ITEM_SEPARATOR) && (pApp->tMainBar.arrItems[3].bEnabled == 0);
	MakeMouse(&tEvent, XGE_EVENT_MOUSE_DOWN, pApp->pMainBar->tRect.fX + 72.0f, pApp->pMainBar->tRect.fY + 10.0f);
	xgeXuiToolbarEvent(&pApp->tMainBar, &tEvent);
	MakeMouse(&tEvent, XGE_EVENT_MOUSE_UP, pApp->pMainBar->tRect.fX + 72.0f, pApp->pMainBar->tRect.fY + 10.0f);
	pApp->bToggleOK = (xgeXuiToolbarEvent(&pApp->tMainBar, &tEvent) == XGE_XUI_EVENT_CONSUMED) && (xgeXuiToolbarGetItemChecked(&pApp->tMainBar, 1) == 1) && (pApp->iLastIndex == 1);
	MakeMouse(&tEvent, XGE_EVENT_MOUSE_DOWN, pApp->pMainBar->tRect.fX + 142.0f, pApp->pMainBar->tRect.fY + 10.0f);
	pApp->bDisabledOK = (xgeXuiToolbarEvent(&pApp->tMainBar, &tEvent) == XGE_XUI_EVENT_CONTINUE);
	xgeXuiSetFocus(&pApp->tXui, pApp->pMainBar);
	pApp->tMainBar.iHover = 1;
	MakeKey(&tEvent, XGE_KEY_SPACE);
	pApp->bKeyboardOK = (xgeXuiToolbarEvent(&pApp->tMainBar, &tEvent) == XGE_XUI_EVENT_CONSUMED) && (xgeXuiToolbarGetItemChecked(&pApp->tMainBar, 1) == 0);
	pApp->bVerticalOK = (pApp->tSideBar.iOrientation == XGE_XUI_SEPARATOR_VERTICAL) && (pApp->tSideBar.arrItems[1].tRect.fY > pApp->tSideBar.arrItems[0].tRect.fY);
	pApp->bGroupOK = (xgeXuiToolbarGetItemGroup(&pApp->tMainBar, 3) == 1) &&
		(pApp->tMainBar.arrItems[3].tRect.fX > (pApp->tMainBar.arrItems[2].tRect.fX + pApp->tMainBar.arrItems[2].tRect.fW + 6.0f));
	MakeMouse(&tEvent, XGE_EVENT_MOUSE_MOVE, pApp->pMainBar->tRect.fX + 8.0f, pApp->pMainBar->tRect.fY + 10.0f);
	xgeXuiToolbarEvent(&pApp->tMainBar, &tEvent);
	pApp->bTooltipOK = (strcmp(xgeXuiToolbarGetItemTooltip(&pApp->tMainBar, 0), "Create a new item") == 0) &&
		(strcmp(xgeXuiToolbarGetHoverTooltip(&pApp->tMainBar), "Create a new item") == 0);

	xgeXuiToolbarSetOverflow(&pApp->tMainBar, 1, 24.0f, ToolbarOverflow, pApp);
	xgeXuiWidgetSetRect(pApp->pMainBar, (xge_rect_t){ 18.0f, 78.0f, 150.0f, 30.0f });
	xgeXuiUpdate(&pApp->tXui, 0.0f);
	xgeXuiPaint(&pApp->tXui);
	tOverflow = xgeXuiToolbarGetOverflowRect(&pApp->tMainBar);
	MakeMouse(&tEvent, XGE_EVENT_MOUSE_DOWN, tOverflow.fX + tOverflow.fW * 0.5f, tOverflow.fY + tOverflow.fH * 0.5f);
	xgeXuiToolbarEvent(&pApp->tMainBar, &tEvent);
	MakeMouse(&tEvent, XGE_EVENT_MOUSE_UP, tOverflow.fX + tOverflow.fW * 0.5f, tOverflow.fY + tOverflow.fH * 0.5f);
	pApp->bOverflowOK = (xgeXuiToolbarGetOverflowFirst(&pApp->tMainBar) >= 2) &&
		(xgeXuiToolbarGetOverflowCount(&pApp->tMainBar) > 0) &&
		(tOverflow.fW > 0.0f) &&
		(xgeXuiToolbarEvent(&pApp->tMainBar, &tEvent) == XGE_XUI_EVENT_CONSUMED) &&
		(pApp->iOverflowCount == 1);
}

static void UpdateStatus(app_state_t* pApp)
{
	char sText[192];

	snprintf(
		sText,
		sizeof(sText),
		"init=%d toggle=%d disabled=%d keyboard=%d vertical=%d group=%d tooltip=%d overflow=%d last=%d cb=%d of=%d",
		pApp->bInitOK,
		pApp->bToggleOK,
		pApp->bDisabledOK,
		pApp->bKeyboardOK,
		pApp->bVerticalOK,
		pApp->bGroupOK,
		pApp->bTooltipOK,
		pApp->bOverflowOK,
		pApp->iLastIndex,
		pApp->iSelectCount,
		pApp->iOverflowCount);
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
	LayoutRoot(pApp);
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
	LayoutRoot(pApp);
	xgeXuiUpdate(&pApp->tXui, fDelta);
	pApp->iFrameCount++;
	if ( (pApp->iFrameLimit > 0) && (pApp->iFrameCount >= pApp->iFrameLimit) ) {
		printf(
			"xui-toolbar-lab final-summary frames=%d init=%d toggle=%d disabled=%d keyboard=%d vertical=%d group=%d tooltip=%d overflow=%d last=%d cb=%d of=%d\n",
			pApp->iFrameCount,
			pApp->bInitOK,
			pApp->bToggleOK,
			pApp->bDisabledOK,
			pApp->bKeyboardOK,
			pApp->bVerticalOK,
			pApp->bGroupOK,
			pApp->bTooltipOK,
			pApp->bOverflowOK,
			pApp->iLastIndex,
			pApp->iSelectCount,
			pApp->iOverflowCount);
		printf("xui-toolbar-lab summary frames=%d/%d\n", pApp->iFrameCount, pApp->iFrameLimit);
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
	tApp.iFrameLimit = ArgInt(getenv("XGE_XUI_TOOLBAR_FRAMES"), 0);
	tApp.iLastIndex = -1;
	for ( i = 1; i < argc; i++ ) {
		if ( (strcmp(argv[i], "--frames") == 0) && ((i + 1) < argc) ) {
			tApp.iFrameLimit = ArgInt(argv[++i], tApp.iFrameLimit);
		}
	}
	tDesc.iWidth = 550;
	tDesc.iHeight = 270;
	tDesc.sTitle = "XGE XUI Toolbar Lab";
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
	return (iExitCode == XGE_OK && tApp.bInitOK && tApp.bToggleOK && tApp.bDisabledOK && tApp.bKeyboardOK && tApp.bVerticalOK && tApp.bGroupOK && tApp.bTooltipOK && tApp.bOverflowOK) ? 0 : 3;
}
