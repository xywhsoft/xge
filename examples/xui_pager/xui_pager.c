#include "../../xge.h"
#include "../xui_demo_style.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PAGER_COUNT 6
#define LABEL_COUNT 7

typedef struct app_state_t {
	xge_scene_t tScene;
	xge_xui_context_t tXui;
	xge_font_t tFont;
	xge_xui_widget pPagerWidget[PAGER_COUNT];
	xge_xui_widget pLabelWidget[LABEL_COUNT];
	xge_xui_label_t tLabel[LABEL_COUNT];
	xge_xui_pager_t tPager[PAGER_COUNT];
	int bFontReady;
	int iFrameLimit;
	int iFrameCount;
	int iChangeCount;
	int iLastOldPage;
	int iLastNewPage;
	int bCreateOK;
	int bLayoutOK;
	int bStateOK;
	int bInteractionOK;
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

static int LoadFont(xge_font pFont)
{
	const char* arrFonts[] = {
		"C:/Windows/Fonts/simsun.ttc",
		"C:/Windows/Fonts/Deng.ttf",
		"C:/Windows/Fonts/msyh.ttc",
		"C:/Windows/Fonts/arial.ttf"
	};
	int i;

	for ( i = 0; i < (int)(sizeof(arrFonts) / sizeof(arrFonts[0])); i++ ) {
		memset(pFont, 0, sizeof(*pFont));
		if ( xgeFontLoad(pFont, arrFonts[i], XGE_XUI_DEMO_FONT_SIZE) == XGE_OK ) {
			printf("xui_pager font loaded: %s\n", arrFonts[i]);
			return XGE_OK;
		}
	}
	return XGE_ERROR_RESOURCE_FAILED;
}

static void OnPagerChange(xge_xui_widget pWidget, int iOldPage, int iNewPage, void* pUser)
{
	app_state_t* pApp;

	(void)pWidget;
	pApp = (app_state_t*)pUser;
	if ( pApp == NULL ) {
		return;
	}
	pApp->iChangeCount++;
	pApp->iLastOldPage = iOldPage;
	pApp->iLastNewPage = iNewPage;
}

static int AddLabel(app_state_t* pApp, xge_xui_widget pRoot, int iIndex, const char* sText, float fX, float fY, float fW)
{
	xge_xui_widget pWidget;

	pWidget = xgeXuiWidgetCreate();
	if ( pWidget == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pApp->pLabelWidget[iIndex] = pWidget;
	xgeXuiWidgetSetRect(pWidget, (xge_rect_t){ fX, fY, fW, 24.0f });
	if ( xgeXuiLabelInit(&pApp->tLabel[iIndex], pWidget, pApp->bFontReady ? &pApp->tFont : NULL, sText) != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiLabelSetColor(&pApp->tLabel[iIndex], XGE_COLOR_RGBA(54, 80, 106, 255));
	xgeXuiLabelSetAlign(&pApp->tLabel[iIndex], XGE_TEXT_ALIGN_LEFT | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP);
	xgeXuiWidgetAdd(pRoot, pWidget);
	return XGE_OK;
}

static int AddPager(app_state_t* pApp, xge_xui_widget pRoot, int iIndex, float fX, float fY, float fW, int iPageCount, int iCurrent, int iWindowSize)
{
	xge_xui_widget pWidget;

	pWidget = xgeXuiWidgetCreate();
	if ( pWidget == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pApp->pPagerWidget[iIndex] = pWidget;
	xgeXuiWidgetSetRect(pWidget, (xge_rect_t){ fX, fY, fW, 28.0f });
	if ( xgeXuiPagerInit(&pApp->tPager[iIndex], &pApp->tXui, pWidget) != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiPagerSetFont(&pApp->tPager[iIndex], pApp->bFontReady ? &pApp->tFont : NULL);
	xgeXuiPagerSetPageCount(&pApp->tPager[iIndex], iPageCount);
	xgeXuiPagerSetWindowSize(&pApp->tPager[iIndex], iWindowSize);
	xgeXuiPagerSetCurrent(&pApp->tPager[iIndex], iCurrent, 0);
	xgeXuiPagerSetChange(&pApp->tPager[iIndex], OnPagerChange, pApp);
	xgeXuiWidgetAdd(pRoot, pWidget);
	return XGE_OK;
}

static int CreateUI(app_state_t* pApp)
{
	xge_xui_widget pRoot;

	XgeXuiDemoApplyTheme(&pApp->tXui, pApp->bFontReady ? &pApp->tFont : NULL);
	pRoot = xgeXuiRoot(&pApp->tXui);
	if ( pRoot == NULL ) {
		return XGE_ERROR;
	}
	XgeXuiDemoApplyRootPanel(pRoot);
	xgeXuiWidgetSetLayout(pRoot, XGE_XUI_LAYOUT_ABSOLUTE);
	xgeXuiWidgetSetPaddingPx(pRoot, 0.0f, 0.0f, 0.0f, 0.0f);
	xgeXuiWidgetSetRect(pRoot, (xge_rect_t){ 0.0f, 0.0f, 840.0f, 470.0f });
	if ( AddLabel(pApp, pRoot, 0, "Pager: first/last, prev/next, window pages, ellipsis, keyboard and disabled state", 26.0f, 22.0f, 780.0f) != XGE_OK ||
		AddLabel(pApp, pRoot, 1, "middle page", 48.0f, 72.0f, 160.0f) != XGE_OK ||
		AddPager(pApp, pRoot, 0, 210.0f, 70.0f, 470.0f, 80, 6, 5) != XGE_OK ||
		AddLabel(pApp, pRoot, 2, "near first", 48.0f, 126.0f, 160.0f) != XGE_OK ||
		AddPager(pApp, pRoot, 1, 210.0f, 124.0f, 360.0f, 12, 1, 5) != XGE_OK ||
		AddLabel(pApp, pRoot, 3, "near last", 48.0f, 180.0f, 160.0f) != XGE_OK ||
		AddPager(pApp, pRoot, 2, 210.0f, 178.0f, 360.0f, 12, 12, 5) != XGE_OK ||
		AddLabel(pApp, pRoot, 4, "small page count", 48.0f, 234.0f, 160.0f) != XGE_OK ||
		AddPager(pApp, pRoot, 3, 210.0f, 232.0f, 320.0f, 4, 2, 5) != XGE_OK ||
		AddLabel(pApp, pRoot, 5, "custom color", 48.0f, 288.0f, 160.0f) != XGE_OK ||
		AddPager(pApp, pRoot, 4, 210.0f, 286.0f, 470.0f, 40, 18, 7) != XGE_OK ||
		AddLabel(pApp, pRoot, 6, "disabled", 48.0f, 342.0f, 160.0f) != XGE_OK ||
		AddPager(pApp, pRoot, 5, 210.0f, 340.0f, 470.0f, 40, 18, 5) != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiPagerSetColors(&pApp->tPager[4],
		XGE_COLOR_RGBA(255, 255, 255, 255),
		XGE_COLOR_RGBA(139, 190, 160, 255),
		XGE_COLOR_RGBA(41, 76, 56, 255),
		XGE_COLOR_RGBA(234, 248, 239, 255),
		XGE_COLOR_RGBA(214, 238, 222, 255),
		XGE_COLOR_RGBA(67, 170, 111, 255),
		XGE_COLOR_RGBA(255, 255, 255, 255),
		XGE_COLOR_RGBA(160, 172, 184, 255));
	xgeXuiWidgetSetEnabled(pApp->pPagerWidget[5], 0);
	return XGE_OK;
}

static void MakeMouse(xge_event_t* pEvent, int iType, xge_rect_t tRect)
{
	memset(pEvent, 0, sizeof(*pEvent));
	pEvent->iType = iType;
	pEvent->iParam1 = XGE_MOUSE_LEFT;
	pEvent->fX = tRect.fX + tRect.fW * 0.5f;
	pEvent->fY = tRect.fY + tRect.fH * 0.5f;
}

static void MakeKey(xge_event_t* pEvent, int iKey)
{
	memset(pEvent, 0, sizeof(*pEvent));
	pEvent->iType = XGE_EVENT_KEY_DOWN;
	pEvent->iParam1 = iKey;
}

static int FindPagerItem(xge_xui_pager pPager, int iType, int iPage)
{
	int i;

	for ( i = 0; i < pPager->iItemCount; i++ ) {
		if ( pPager->arrItem[i].iType != iType ) {
			continue;
		}
		if ( (iPage <= 0) || (pPager->arrItem[i].iPage == iPage) ) {
			return i;
		}
	}
	return -1;
}

static xge_rect_t PagerItemScreenRect(xge_xui_pager pPager, int iIndex)
{
	xge_rect_t tRect;
	float fDx;
	float fDy;

	tRect = pPager->arrItem[iIndex].tRect;
	if ( pPager->iItemCount > 0 ) {
		fDx = pPager->pWidget->tContentRect.fX - pPager->arrItem[0].tRect.fX;
		fDy = pPager->pWidget->tContentRect.fY - pPager->arrItem[0].tRect.fY;
		tRect.fX += fDx;
		tRect.fY += fDy;
	}
	return tRect;
}

static void RunChecks(app_state_t* pApp)
{
	xge_event_t tEvent;
	int iNext;
	int iDisabledNext;

	if ( pApp->bInteractionOK ) {
		return;
	}
	xgeXuiUpdate(&pApp->tXui, 0.0f);
	xgeXuiPaint(&pApp->tXui);
	pApp->bCreateOK =
		(pApp->pPagerWidget[0] != NULL) &&
		(pApp->tPager[0].pWidget == pApp->pPagerWidget[0]) &&
		(pApp->pPagerWidget[5] != NULL);
	pApp->bLayoutOK = pApp->bCreateOK &&
		(pApp->pPagerWidget[0]->tRect.fX == 210.0f) &&
		(pApp->pPagerWidget[0]->tRect.fY == 70.0f) &&
		(pApp->pPagerWidget[0]->tRect.fH == 28.0f);
	pApp->bStateOK = pApp->bCreateOK &&
		(xgeXuiPagerGetPageCount(&pApp->tPager[0]) == 80) &&
		(xgeXuiPagerGetCurrent(&pApp->tPager[0]) == 6) &&
		(pApp->tPager[0].fEllipsisWidth == 32.0f) &&
		(xgeXuiPagerGetCurrent(&pApp->tPager[1]) == 1) &&
		(xgeXuiPagerGetCurrent(&pApp->tPager[2]) == 12) &&
		(xgeXuiPagerGetPageCount(&pApp->tPager[3]) == 4) &&
		(pApp->tPager[4].iCurrentColor == XGE_COLOR_RGBA(67, 170, 111, 255)) &&
		(xgeXuiWidgetIsEnabled(pApp->pPagerWidget[5]) == 0);
	iNext = FindPagerItem(&pApp->tPager[0], XGE_XUI_PAGER_ITEM_NEXT, 0);
	if ( iNext >= 0 ) {
		MakeMouse(&tEvent, XGE_EVENT_MOUSE_DOWN, PagerItemScreenRect(&pApp->tPager[0], iNext));
		xgeXuiPagerEvent(&pApp->tPager[0], &tEvent);
		MakeMouse(&tEvent, XGE_EVENT_MOUSE_UP, PagerItemScreenRect(&pApp->tPager[0], iNext));
		xgeXuiPagerEvent(&pApp->tPager[0], &tEvent);
	}
	xgeXuiSetFocus(&pApp->tXui, pApp->pPagerWidget[0]);
	MakeKey(&tEvent, XGE_KEY_END);
	xgeXuiPagerEvent(&pApp->tPager[0], &tEvent);
	iDisabledNext = FindPagerItem(&pApp->tPager[5], XGE_XUI_PAGER_ITEM_NEXT, 0);
	if ( iDisabledNext >= 0 ) {
		MakeMouse(&tEvent, XGE_EVENT_MOUSE_DOWN, PagerItemScreenRect(&pApp->tPager[5], iDisabledNext));
		xgeXuiPagerEvent(&pApp->tPager[5], &tEvent);
		MakeMouse(&tEvent, XGE_EVENT_MOUSE_UP, PagerItemScreenRect(&pApp->tPager[5], iDisabledNext));
		xgeXuiPagerEvent(&pApp->tPager[5], &tEvent);
	}
	pApp->bInteractionOK =
		(xgeXuiPagerGetCurrent(&pApp->tPager[0]) == 80) &&
		(pApp->iChangeCount >= 2) &&
		(pApp->iLastOldPage == 7) &&
		(pApp->iLastNewPage == 80) &&
		(xgeXuiPagerGetCurrent(&pApp->tPager[5]) == 18);
	if ( pApp->bInteractionOK ) {
		xgeXuiPagerSetCurrent(&pApp->tPager[0], 6, 0);
	}
}

static int AppEnter(xge_scene pScene)
{
	app_state_t* pApp;

	pApp = (app_state_t*)pScene->pUser;
	pApp->bFontReady = (LoadFont(&pApp->tFont) == XGE_OK);
	if ( (xgeXuiInit(&pApp->tXui) != XGE_OK) || (CreateUI(pApp) != XGE_OK) ) {
		return XGE_ERROR;
	}
	RunChecks(pApp);
	return XGE_OK;
}

static int AppLeave(xge_scene pScene)
{
	app_state_t* pApp;
	int i;

	pApp = (app_state_t*)pScene->pUser;
	for ( i = 0; i < PAGER_COUNT; i++ ) {
		xgeXuiPagerUnit(&pApp->tPager[i]);
	}
	for ( i = 0; i < LABEL_COUNT; i++ ) {
		xgeXuiLabelUnit(&pApp->tLabel[i]);
	}
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
	if ( (pEvent != NULL) && (pEvent->iType == XGE_EVENT_KEY_DOWN) && (pEvent->iParam1 == XGE_KEY_ESCAPE) ) {
		xgeQuit();
		return XGE_OK;
	}
	xgeXuiDispatchEvent(&pApp->tXui, pEvent);
	return XGE_OK;
}

static int AppUpdate(xge_scene pScene, float fDelta)
{
	app_state_t* pApp;

	pApp = (app_state_t*)pScene->pUser;
	xgeXuiUpdate(&pApp->tXui, fDelta);
	RunChecks(pApp);
	pApp->iFrameCount++;
	if ( (pApp->iFrameLimit > 0) && (pApp->iFrameCount >= pApp->iFrameLimit) ) {
		printf("xui_pager final-summary frames=%d create=%d layout=%d state=%d interaction=%d changes=%d current=%d\n",
			pApp->iFrameCount,
			pApp->bCreateOK,
			pApp->bLayoutOK,
			pApp->bStateOK,
			pApp->bInteractionOK,
			pApp->iChangeCount,
			xgeXuiPagerGetCurrent(&pApp->tPager[0]));
		xgeQuit();
	}
	return XGE_OK;
}

static int AppDraw(xge_scene pScene)
{
	app_state_t* pApp;

	pApp = (app_state_t*)pScene->pUser;
	xgeBegin();
	xgeClear(XGE_COLOR_RGBA(229, 235, 244, 255));
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
	tApp.iFrameLimit = ArgInt(getenv("XGE_XUI_PAGER_FRAMES"), 0);
	for ( i = 1; i < argc; i++ ) {
		if ( (strcmp(argv[i], "--frames") == 0) && ((i + 1) < argc) ) {
			tApp.iFrameLimit = ArgInt(argv[++i], tApp.iFrameLimit);
		}
	}
	tDesc.iWidth = 840;
	tDesc.iHeight = 470;
	tDesc.sTitle = "XUI Pager";
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
	return (iExitCode == XGE_OK && tApp.bCreateOK && tApp.bLayoutOK && tApp.bStateOK && tApp.bInteractionOK) ? 0 : 3;
}
