#include "../../xge.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct app_state_t {
	xge_scene_t tScene;
	xge_xui_context_t tXui;
	xge_font_t tFont;
	xge_xui_panel_t tPanel;
	xge_xui_label_t tSummary;
	xge_xui_search_box_t tSearch;
	xge_xui_widget pPanel;
	xge_xui_widget pSummary;
	xge_xui_widget pSearchWidget;
	int bFontReady;
	int iFrameLimit;
	int iFrameCount;
	int bInitOK;
	int bSubmitOK;
	int bClearOK;
	int iSubmitCount;
	int iClearCount;
	char sSubmitText[80];
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
			printf("xui-search-box-lab font loaded: %s\n", arrFonts[i]);
			return XGE_OK;
		}
	}
	printf("xui-search-box-lab font load failed\n");
	return XGE_ERROR_RESOURCE_FAILED;
}

static void SearchSubmit(xge_xui_widget pWidget, const char* sText, void* pUser)
{
	app_state_t* pApp;

	(void)pWidget;
	pApp = (app_state_t*)pUser;
	if ( pApp != NULL ) {
		pApp->iSubmitCount++;
		snprintf(pApp->sSubmitText, sizeof(pApp->sSubmitText), "%s", (sText != NULL) ? sText : "");
		pApp->sSubmitText[sizeof(pApp->sSubmitText) - 1] = 0;
	}
}

static void SearchClear(xge_xui_widget pWidget, const char* sText, void* pUser)
{
	app_state_t* pApp;

	(void)pWidget;
	(void)sText;
	pApp = (app_state_t*)pUser;
	if ( pApp != NULL ) {
		pApp->iClearCount++;
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
	pApp->pSummary = xgeXuiWidgetCreate();
	pApp->pSearchWidget = xgeXuiWidgetCreate();
	if ( (pApp->pPanel == NULL) || (pApp->pSummary == NULL) || (pApp->pSearchWidget == NULL) ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	xgeXuiWidgetSetRect(pApp->pPanel, (xge_rect_t){ 24.0f, 22.0f, 374.0f, 164.0f });
	xgeXuiWidgetSetPaddingPx(pApp->pPanel, 16.0f, 12.0f, 16.0f, 12.0f);
	xgeXuiPanelInit(&pApp->tPanel, pApp->pPanel);
	xgeXuiPanelSetTitle(&pApp->tPanel, pFont, "SearchBox");
	xgeXuiWidgetAdd(pRoot, pApp->pPanel);
	xgeXuiWidgetSetRect(pApp->pSummary, (xge_rect_t){ 18.0f, 38.0f, 326.0f, 24.0f });
	xgeXuiLabelInit(&pApp->tSummary, pApp->pSummary, pFont, "");
	xgeXuiWidgetAdd(pApp->pPanel, pApp->pSummary);
	xgeXuiWidgetSetRect(pApp->pSearchWidget, (xge_rect_t){ 18.0f, 78.0f, 260.0f, 30.0f });
	xgeXuiSearchBoxInit(&pApp->tSearch, &pApp->tXui, pApp->pSearchWidget, pFont);
	xgeXuiSearchBoxSetPlaceholder(&pApp->tSearch, "Search assets");
	xgeXuiSearchBoxSetSubmit(&pApp->tSearch, SearchSubmit, pApp);
	xgeXuiSearchBoxSetClear(&pApp->tSearch, SearchClear, pApp);
	xgeXuiWidgetAdd(pApp->pPanel, pApp->pSearchWidget);
	return XGE_OK;
}

static void RunChecks(app_state_t* pApp)
{
	xge_event_t tEvent;

	xgeXuiSearchBoxSetText(&pApp->tSearch, "texture");
	xgeXuiSetFocus(&pApp->tXui, pApp->pSearchWidget);
	MakeKey(&tEvent, XGE_KEY_ENTER);
	xgeXuiDispatchEvent(&pApp->tXui, &tEvent);
	pApp->bInitOK = strcmp(xgeXuiSearchBoxGetText(&pApp->tSearch), "texture") == 0 && pApp->tSearch.bSuggestionsReserved;
	pApp->bSubmitOK = pApp->iSubmitCount == 1 && strcmp(pApp->sSubmitText, "texture") == 0;
	MakeMouse(&tEvent, XGE_EVENT_MOUSE_DOWN, pApp->tSearch.tClearRect.fX + 4.0f, pApp->tSearch.tClearRect.fY + 4.0f);
	xgeXuiDispatchEvent(&pApp->tXui, &tEvent);
	pApp->bClearOK = pApp->iClearCount == 1 && strcmp(xgeXuiSearchBoxGetText(&pApp->tSearch), "") == 0;
}

static void UpdateSummary(app_state_t* pApp)
{
	char sText[160];

	snprintf(
		sText,
		sizeof(sText),
		"init=%d submit=%d clear=%d sub=%d clr=%d text=%s",
		pApp->bInitOK,
		pApp->bSubmitOK,
		pApp->bClearOK,
		pApp->iSubmitCount,
		pApp->iClearCount,
		pApp->sSubmitText);
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
			"xui-search-box-lab final-summary frames=%d init=%d submit=%d clear=%d sub=%d clr=%d text=%s\n",
			pApp->iFrameCount,
			pApp->bInitOK,
			pApp->bSubmitOK,
			pApp->bClearOK,
			pApp->iSubmitCount,
			pApp->iClearCount,
			pApp->sSubmitText);
		printf("xui-search-box-lab summary frames=%d/%d\n", pApp->iFrameCount, pApp->iFrameLimit);
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
	tApp.iFrameLimit = ArgInt(getenv("XGE_XUI_SEARCH_BOX_FRAMES"), 0);
	for ( i = 1; i < argc; i++ ) {
		if ( (strcmp(argv[i], "--frames") == 0) && ((i + 1) < argc) ) {
			tApp.iFrameLimit = ArgInt(argv[++i], tApp.iFrameLimit);
		}
	}
	tDesc.iWidth = 422;
	tDesc.iHeight = 210;
	tDesc.sTitle = "XGE XUI SearchBox Lab";
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
	return (iExitCode == XGE_OK && tApp.bInitOK && tApp.bSubmitOK && tApp.bClearOK) ? 0 : 3;
}
