#include "../../xge.h"
#include "../xui_demo_style.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PAGER_COUNT 6

typedef struct app_state_t {
	xge_scene_t tScene;
	xge_xui_context_t tXui;
	xge_xui_page_t tPage;
	xge_font_t tFont;
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

static const char sXson[] =
"{\"xui\":1,"
"\"styles\":{"
"\"root\":{\"type\":\"absolute\",\"width\":840,\"height\":470,\"background\":\"#E5EBF4FF\"},"
"\"label\":{\"type\":\"label\",\"font\":\"@fonts.body\",\"width\":160,\"height\":24,\"textColor\":\"#36506AFF\",\"textAlign\":\"left\",\"textVAlign\":\"middle\"},"
"\"title\":{\"@parent\":\"label\",\"width\":780,\"height\":26},"
"\"pager\":{\"type\":\"pager\",\"font\":\"@fonts.body\",\"width\":470,\"height\":28,\"pageCount\":80,\"windowSize\":5},"
"\"customPager\":{\"@parent\":\"pager\",\"width\":540,\"borderColor\":\"#8BBEA0FF\",\"textColor\":\"#294C38FF\",\"hoverColor\":\"#EAF8EFFF\",\"activeColor\":\"#D6EEDEFF\",\"currentColor\":\"#43AA6FFF\",\"windowSize\":7}"
"},"
"\"tree\":{\"type\":\"absolute\",\"id\":\"root\",\"style\":\"root\",\"children\":["
"{\"type\":\"label\",\"id\":\"title\",\"style\":\"title\",\"x\":26,\"y\":22,\"text\":\"Pager XSON: direct page count, total/pageSize, window size, custom colors, disabled\"},"
"{\"type\":\"label\",\"id\":\"l0\",\"style\":\"label\",\"x\":48,\"y\":72,\"text\":\"middle page\"},"
"{\"type\":\"pager\",\"id\":\"p0\",\"style\":\"pager\",\"x\":210,\"y\":70,\"currentPage\":6},"
"{\"type\":\"label\",\"id\":\"l1\",\"style\":\"label\",\"x\":48,\"y\":126,\"text\":\"total/pageSize\"},"
"{\"type\":\"pager\",\"id\":\"p1\",\"style\":\"pager\",\"x\":210,\"y\":124,\"total\":123,\"pageSize\":10,\"current\":3},"
"{\"type\":\"label\",\"id\":\"l2\",\"style\":\"label\",\"x\":48,\"y\":180,\"text\":\"small page count\"},"
"{\"type\":\"pager\",\"id\":\"p2\",\"style\":\"pager\",\"x\":210,\"y\":178,\"pageCount\":4,\"page\":2,\"width\":360},"
"{\"type\":\"label\",\"id\":\"l3\",\"style\":\"label\",\"x\":48,\"y\":234,\"text\":\"custom text\"},"
"{\"type\":\"pager\",\"id\":\"p3\",\"style\":\"pager\",\"x\":210,\"y\":232,\"pages\":40,\"value\":18,\"firstText\":\"Start\",\"lastText\":\"End\",\"prevText\":\"<\",\"nextText\":\">\"},"
"{\"type\":\"label\",\"id\":\"l4\",\"style\":\"label\",\"x\":48,\"y\":288,\"text\":\"custom color\"},"
"{\"type\":\"pager\",\"id\":\"p4\",\"style\":\"customPager\",\"x\":210,\"y\":286,\"pageCount\":40,\"currentPage\":18},"
"{\"type\":\"label\",\"id\":\"l5\",\"style\":\"label\",\"x\":48,\"y\":342,\"text\":\"disabled\"},"
"{\"type\":\"pager\",\"id\":\"p5\",\"style\":\"pager\",\"x\":210,\"y\":340,\"pageCount\":40,\"currentPage\":18,\"enabled\":false}"
"]}}";

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
			printf("xui_pager_xson font loaded: %s\n", arrFonts[i]);
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

static int PagerFitsWidget(xge_xui_pager pPager)
{
	xge_vec2_t tSize;

	tSize = xgeXuiPagerMeasureProc(pPager->pWidget, pPager);
	return (pPager != NULL) && (pPager->pWidget != NULL) && (tSize.fX <= pPager->pWidget->tContentRect.fW + 0.01f);
}

static void RunChecks(app_state_t* pApp)
{
	xge_event_t tEvent;
	int iNext;

	if ( pApp->bInteractionOK ) {
		return;
	}
	xgeXuiUpdate(&pApp->tXui, 0.0f);
	xgeXuiPaint(&pApp->tXui);
	pApp->bCreateOK =
		(pApp->tPage.iPagerCount == PAGER_COUNT) &&
		(xgeXuiPageFind(&pApp->tPage, "p0") != NULL) &&
		(xgeXuiPageFind(&pApp->tPage, "p4") != NULL);
	pApp->bLayoutOK = pApp->bCreateOK &&
		(pApp->tPage.arrPager[0].pWidget->tRect.fX == 210.0f) &&
		(pApp->tPage.arrPager[0].pWidget->tRect.fY == 70.0f) &&
		(pApp->tPage.arrPager[0].pWidget->tRect.fH == 28.0f);
	pApp->bStateOK = pApp->bCreateOK &&
		(xgeXuiPagerGetPageCount(&pApp->tPage.arrPager[0]) == 80) &&
		(xgeXuiPagerGetCurrent(&pApp->tPage.arrPager[0]) == 6) &&
		(xgeXuiPagerGetPageCount(&pApp->tPage.arrPager[1]) == 13) &&
		(xgeXuiPagerGetCurrent(&pApp->tPage.arrPager[1]) == 3) &&
		(xgeXuiPagerGetPageCount(&pApp->tPage.arrPager[2]) == 4) &&
		(pApp->tPage.arrPager[3].sFirstText != NULL) &&
		(strcmp(pApp->tPage.arrPager[3].sFirstText, "Start") == 0) &&
		(pApp->tPage.arrPager[4].iCurrentColor == XGE_COLOR_RGBA(67, 170, 111, 255)) &&
		(pApp->tPage.arrPager[4].iWindowSize == 7) &&
		(xgeXuiWidgetIsEnabled(pApp->tPage.arrPager[5].pWidget) == 0) &&
		PagerFitsWidget(&pApp->tPage.arrPager[0]) &&
		PagerFitsWidget(&pApp->tPage.arrPager[1]) &&
		PagerFitsWidget(&pApp->tPage.arrPager[2]) &&
		PagerFitsWidget(&pApp->tPage.arrPager[3]) &&
		PagerFitsWidget(&pApp->tPage.arrPager[4]) &&
		PagerFitsWidget(&pApp->tPage.arrPager[5]);
	xgeXuiPagerSetChange(&pApp->tPage.arrPager[0], OnPagerChange, pApp);
	iNext = FindPagerItem(&pApp->tPage.arrPager[0], XGE_XUI_PAGER_ITEM_NEXT, 0);
	if ( iNext >= 0 ) {
		MakeMouse(&tEvent, XGE_EVENT_MOUSE_DOWN, PagerItemScreenRect(&pApp->tPage.arrPager[0], iNext));
		xgeXuiPagerEvent(&pApp->tPage.arrPager[0], &tEvent);
		MakeMouse(&tEvent, XGE_EVENT_MOUSE_UP, PagerItemScreenRect(&pApp->tPage.arrPager[0], iNext));
		xgeXuiPagerEvent(&pApp->tPage.arrPager[0], &tEvent);
	}
	xgeXuiSetFocus(&pApp->tXui, pApp->tPage.arrPager[0].pWidget);
	MakeKey(&tEvent, XGE_KEY_HOME);
	xgeXuiPagerEvent(&pApp->tPage.arrPager[0], &tEvent);
	pApp->bInteractionOK =
		(xgeXuiPagerGetCurrent(&pApp->tPage.arrPager[0]) == 1) &&
		(pApp->iChangeCount >= 2) &&
		(pApp->iLastOldPage == 7) &&
		(pApp->iLastNewPage == 1);
	if ( pApp->bInteractionOK ) {
		xgeXuiPagerSetCurrent(&pApp->tPage.arrPager[0], 6, 0);
	} else {
		xgeXuiPagerSetCurrent(&pApp->tPage.arrPager[0], 6, 0);
		pApp->iChangeCount = 0;
		pApp->iLastOldPage = 0;
		pApp->iLastNewPage = 0;
	}
}

static int CreateUI(app_state_t* pApp)
{
	XgeXuiDemoApplyTheme(&pApp->tXui, pApp->bFontReady ? &pApp->tFont : NULL);
	if ( xgeXuiTokenSetFont(&pApp->tXui, "body", pApp->bFontReady ? &pApp->tFont : NULL) != XGE_OK ) {
		return XGE_ERROR;
	}
	if ( xgeXuiPageLoadMemory(&pApp->tXui, sXson, (int)strlen(sXson), NULL, &pApp->tPage) != XGE_OK ) {
		printf("xui_pager_xson load failed: %s\n", xgeXuiPageGetError(&pApp->tPage));
		return XGE_ERROR;
	}
	RunChecks(pApp);
	return XGE_OK;
}

static int AppEnter(xge_scene pScene)
{
	app_state_t* pApp;

	pApp = (app_state_t*)pScene->pUser;
	pApp->bFontReady = (LoadFont(&pApp->tFont) == XGE_OK);
	if ( (xgeXuiInit(&pApp->tXui) != XGE_OK) || (CreateUI(pApp) != XGE_OK) ) {
		return XGE_ERROR;
	}
	return XGE_OK;
}

static int AppLeave(xge_scene pScene)
{
	app_state_t* pApp;

	pApp = (app_state_t*)pScene->pUser;
	xgeXuiPageUnload(&pApp->tPage);
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
		printf("xui_pager_xson final-summary frames=%d create=%d layout=%d state=%d interaction=%d pagers=%d changes=%d current=%d\n",
			pApp->iFrameCount,
			pApp->bCreateOK,
			pApp->bLayoutOK,
			pApp->bStateOK,
			pApp->bInteractionOK,
			pApp->tPage.iPagerCount,
			pApp->iChangeCount,
			xgeXuiPagerGetCurrent(&pApp->tPage.arrPager[0]));
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
	static app_state_t tApp;
	int i;
	int iExitCode;

	memset(&tDesc, 0, sizeof(tDesc));
	memset(&tApp, 0, sizeof(tApp));
	tApp.iFrameLimit = ArgInt(getenv("XGE_XUI_PAGER_XSON_FRAMES"), 0);
	for ( i = 1; i < argc; i++ ) {
		if ( (strcmp(argv[i], "--frames") == 0) && ((i + 1) < argc) ) {
			tApp.iFrameLimit = ArgInt(argv[++i], tApp.iFrameLimit);
		}
	}
	tDesc.iWidth = 840;
	tDesc.iHeight = 470;
	tDesc.sTitle = "XUI Pager XSON";
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
