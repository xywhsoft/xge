#include "../../xge.h"
#include "../xui_demo_style.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LIST_COUNT 4
#define LARGE_COUNT 1000
#define MULTI_COUNT 12
#define LABEL_COUNT 6

enum {
	LIST_BASIC = 0,
	LIST_LARGE = 1,
	LIST_MULTI = 2,
	LIST_RICH = 3
};

static const char* g_arrBasicItems[] = {
	"Dashboard",
	"Inventory",
	"Crafting disabled",
	"Character",
	"Quest log",
	"Guild disabled",
	"Mail",
	"Settings",
	"Exit"
};

static const int g_arrBasicEnabled[] = {
	1, 1, 0, 1, 1, 0, 1, 1, 1
};

static const char* g_arrMultiItems[] = {
	"Iron ore", "Copper ore", "Silver ore", "Gold ore",
	"Oak plank", "Birch plank", "Ancient fiber", "Moon crystal",
	"Water flask", "Fire shard", "Wind feather", "Earth core"
};

static const char* g_arrRichTitle[] = {
	"Azure short sword", "Guardian shield", "Field ration", "Arcane scroll",
	"Scout boots", "Repair kit", "Marked quest item", "Locked relic",
	"Guild token", "Travel pass", "Gem pouch", "Old map"
};

static const char* g_arrRichMeta[] = {
	"Weapon  ATK +12", "Armor  DEF +18", "Food  restores 20", "Magic  cooldown -8%",
	"Gear  movement +5%", "Tool  durability +40", "Quest  turn in", "Rare  disabled",
	"Currency  guild", "Travel  city gate", "Material  stack 18", "Map  region clue"
};

static const int g_arrRichEnabled[] = {
	1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1
};

typedef struct app_state_t {
	xge_scene_t tScene;
	xge_xui_context_t tXui;
	xge_font_t tFont;
	xge_xui_widget pPanel[LIST_COUNT];
	xge_xui_widget pListWidget[LIST_COUNT];
	xge_xui_label_t tLabel[LABEL_COUNT];
	xge_xui_list_view_t tList[LIST_COUNT];
	const char* arrLargeItems[LARGE_COUNT];
	char arrLargeText[LARGE_COUNT][40];
	int arrMultiSelected[MULTI_COUNT];
	int bFontReady;
	int iFrameLimit;
	int iFrameCount;
	int iLastWidth;
	int iLastHeight;
	int iSelectCount[LIST_COUNT];
	int iLastSelected[LIST_COUNT];
	int bCreateOK;
	int bLayoutOK;
	int bStateOK;
	int bHoverWidthOK;
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
			printf("xui_listview font loaded: %s\n", arrFonts[i]);
			return XGE_OK;
		}
	}
	return XGE_ERROR_RESOURCE_FAILED;
}

static void BuildLargeItems(app_state_t* pApp)
{
	int i;

	for ( i = 0; i < LARGE_COUNT; i++ ) {
		snprintf(pApp->arrLargeText[i], sizeof(pApp->arrLargeText[i]), "Virtual item %04d", i + 1);
		pApp->arrLargeItems[i] = pApp->arrLargeText[i];
	}
}

static xge_xui_widget NewWidget(float fHeight)
{
	xge_xui_widget pWidget;

	pWidget = xgeXuiWidgetCreate();
	if ( pWidget != NULL ) {
		xgeXuiWidgetSetSize(pWidget, xgeXuiSizePercent(100.0f), (fHeight > 0.0f) ? xgeXuiSizePx(fHeight) : xgeXuiSizeGrow(1.0f));
	}
	return pWidget;
}

static int AddLabel(app_state_t* pApp, xge_xui_widget pParent, const char* sText, uint32_t iColor)
{
	xge_xui_widget pWidget;
	int iIndex;

	for ( iIndex = 0; iIndex < LABEL_COUNT; iIndex++ ) {
		if ( pApp->tLabel[iIndex].pWidget == NULL ) {
			break;
		}
	}
	if ( iIndex >= LABEL_COUNT ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pWidget = NewWidget(28.0f);
	if ( pWidget == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	if ( xgeXuiLabelInit(&pApp->tLabel[iIndex], pWidget, pApp->bFontReady ? &pApp->tFont : NULL, sText) != XGE_OK ) {
		xgeXuiWidgetFree(pWidget);
		return XGE_ERROR;
	}
	xgeXuiLabelSetColor(&pApp->tLabel[iIndex], iColor);
	xgeXuiLabelSetAlign(&pApp->tLabel[iIndex], XGE_TEXT_ALIGN_LEFT | XGE_TEXT_ALIGN_MIDDLE);
	xgeXuiWidgetAdd(pParent, pWidget);
	return XGE_OK;
}

static void OnListSelect(xge_xui_widget pWidget, int iIndex, void* pUser)
{
	app_state_t* pApp;
	int i;

	pApp = (app_state_t*)pUser;
	if ( (pApp == NULL) || (pWidget == NULL) ) {
		return;
	}
	for ( i = 0; i < LIST_COUNT; i++ ) {
		if ( pApp->pListWidget[i] == pWidget ) {
			pApp->iSelectCount[i]++;
			pApp->iLastSelected[i] = iIndex;
			break;
		}
	}
}

static int RichItemRenderer(xge_xui_widget pWidget, int iIndex, xge_rect_t tRect, int iState, void* pUser)
{
	app_state_t* pApp;
	xge_rect_t tIcon;
	xge_rect_t tTitle;
	xge_rect_t tMeta;
	xge_rect_t tBadge;
	uint32_t iRow;
	uint32_t iIcon;
	uint32_t iText;
	uint32_t iSubText;

	(void)pWidget;
	pApp = (app_state_t*)pUser;
	if ( (pApp == NULL) || (iIndex < 0) || (iIndex >= (int)(sizeof(g_arrRichTitle) / sizeof(g_arrRichTitle[0]))) ) {
		return 0;
	}
	iRow = XGE_COLOR_RGBA(247, 250, 253, 255);
	if ( (iState & XGE_XUI_LIST_ITEM_HOVER) != 0 ) {
		iRow = XGE_COLOR_RGBA(226, 238, 250, 255);
	}
	if ( (iState & XGE_XUI_LIST_ITEM_SELECTED) != 0 ) {
		iRow = XGE_COLOR_RGBA(205, 226, 248, 255);
	}
	iText = ((iState & XGE_XUI_LIST_ITEM_DISABLED) != 0) ? XGE_COLOR_RGBA(132, 142, 154, 180) : XGE_COLOR_RGBA(34, 47, 64, 255);
	iSubText = ((iState & XGE_XUI_LIST_ITEM_DISABLED) != 0) ? XGE_COLOR_RGBA(132, 142, 154, 150) : XGE_COLOR_RGBA(78, 94, 114, 255);
	(void)iSubText;
	xgeShapeRectFill(tRect, iRow);
	tIcon = (xge_rect_t){ tRect.fX + 8.0f, tRect.fY + 7.0f, 22.0f, 22.0f };
	iIcon = (iIndex % 3 == 0) ? XGE_COLOR_RGBA(46, 124, 214, 255) : ((iIndex % 3 == 1) ? XGE_COLOR_RGBA(64, 156, 104, 255) : XGE_COLOR_RGBA(218, 134, 42, 255));
	if ( (iState & XGE_XUI_LIST_ITEM_DISABLED) != 0 ) {
		iIcon = XGE_COLOR_RGBA(146, 154, 166, 180);
	}
	xgeShapeRectFill(tIcon, iIcon);
	xgeShapeRectStroke(tIcon, 1.0f, XGE_COLOR_RGBA(255, 255, 255, 170));
	tTitle = (xge_rect_t){ tRect.fX + 38.0f, tRect.fY + 3.0f, tRect.fW - 96.0f, 16.0f };
	tMeta = (xge_rect_t){ tRect.fX + 38.0f, tRect.fY + 19.0f, tRect.fW - 96.0f, 14.0f };
	if ( pApp->bFontReady ) {
		xgeTextDrawRect(&pApp->tFont, g_arrRichTitle[iIndex], tTitle, iText, XGE_TEXT_ALIGN_LEFT | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP);
		xgeTextDrawRect(&pApp->tFont, g_arrRichMeta[iIndex], tMeta, iSubText, XGE_TEXT_ALIGN_LEFT | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP);
	}
	if ( (iIndex == 6) || (iIndex == 7) ) {
		tBadge = (xge_rect_t){ tRect.fX + tRect.fW - 52.0f, tRect.fY + 10.0f, 38.0f, 16.0f };
		xgeShapeRectFill(tBadge, (iIndex == 6) ? XGE_COLOR_RGBA(235, 178, 52, 255) : XGE_COLOR_RGBA(156, 166, 178, 210));
		if ( pApp->bFontReady ) {
			xgeTextDrawRect(&pApp->tFont, (iIndex == 6) ? "Quest" : "Lock", tBadge, XGE_COLOR_RGBA(255, 255, 255, 255), XGE_TEXT_ALIGN_CENTER | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP);
		}
	}
	return 1;
}

static void StylePanel(xge_xui_widget pPanel)
{
	xgeXuiWidgetSetLayout(pPanel, XGE_XUI_LAYOUT_COLUMN);
	xgeXuiWidgetSetSize(pPanel, xgeXuiSizePercent(100.0f), xgeXuiSizePercent(100.0f));
	xgeXuiWidgetSetPaddingPx(pPanel, 12.0f, 10.0f, 12.0f, 12.0f);
	xgeXuiWidgetSetGap(pPanel, 8.0f);
	xgeXuiWidgetSetBackground(pPanel, XGE_COLOR_RGBA(248, 250, 253, 255));
	xgeXuiWidgetSetBorder(pPanel, 1.0f, XGE_COLOR_RGBA(170, 184, 202, 255));
	xgeXuiWidgetSetRadius(pPanel, 4.0f);
}

static int AddPanel(app_state_t* pApp, xge_xui_widget pRoot, int iIndex, const char* sTitle)
{
	xge_xui_widget pPanel;

	pPanel = NewWidget(0.0f);
	if ( pPanel == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	StylePanel(pPanel);
	xgeXuiWidgetAdd(pRoot, pPanel);
	pApp->pPanel[iIndex] = pPanel;
	return AddLabel(pApp, pPanel, sTitle, XGE_COLOR_RGBA(42, 58, 78, 255));
}

static int AddList(app_state_t* pApp, int iIndex, float fItemHeight)
{
	xge_xui_widget pWidget;

	pWidget = NewWidget(0.0f);
	if ( pWidget == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	xgeXuiWidgetSetPaddingPx(pWidget, 2.0f, 2.0f, 2.0f, 2.0f);
	if ( xgeXuiListViewInit(&pApp->tList[iIndex], &pApp->tXui, pWidget) != XGE_OK ) {
		xgeXuiWidgetFree(pWidget);
		return XGE_ERROR;
	}
	xgeXuiListViewSetFont(&pApp->tList[iIndex], pApp->bFontReady ? &pApp->tFont : NULL);
	xgeXuiListViewSetItemHeight(&pApp->tList[iIndex], fItemHeight);
	xgeXuiListViewSetSelect(&pApp->tList[iIndex], OnListSelect, pApp);
	xgeXuiWidgetAdd(pApp->pPanel[iIndex], pWidget);
	pApp->pListWidget[iIndex] = pWidget;
	return XGE_OK;
}

static int CreateUI(app_state_t* pApp)
{
	xge_xui_widget pRoot;

	pRoot = xgeXuiRoot(&pApp->tXui);
	if ( pRoot == NULL ) {
		return XGE_ERROR;
	}
	XgeXuiDemoApplyTheme(&pApp->tXui, pApp->bFontReady ? &pApp->tFont : NULL);
	XgeXuiDemoApplyRootPanel(pRoot);
	xgeXuiWidgetSetLayout(pRoot, XGE_XUI_LAYOUT_GRID);
	xgeXuiWidgetSetGrid(pRoot, 2, 300.0f, 12.0f, 12.0f);
	xgeXuiWidgetSetPaddingPx(pRoot, 18.0f, 18.0f, 18.0f, 18.0f);
	if ( AddPanel(pApp, pRoot, LIST_BASIC, "Basic list: disabled rows, mouse, wheel, keyboard") != XGE_OK ||
	     AddPanel(pApp, pRoot, LIST_LARGE, "Large data: 1000 rows, virtualized slots") != XGE_OK ||
	     AddPanel(pApp, pRoot, LIST_MULTI, "Multi / range selection") != XGE_OK ||
	     AddPanel(pApp, pRoot, LIST_RICH, "Custom row renderer: icon, meta and tags") != XGE_OK ) {
		return XGE_ERROR;
	}
	if ( AddList(pApp, LIST_BASIC, 28.0f) != XGE_OK ||
	     AddList(pApp, LIST_LARGE, 24.0f) != XGE_OK ||
	     AddList(pApp, LIST_MULTI, 26.0f) != XGE_OK ||
	     AddList(pApp, LIST_RICH, 38.0f) != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiListViewSetItems(&pApp->tList[LIST_BASIC], g_arrBasicItems, (int)(sizeof(g_arrBasicItems) / sizeof(g_arrBasicItems[0])));
	xgeXuiListViewSetEnabledItems(&pApp->tList[LIST_BASIC], g_arrBasicEnabled, (int)(sizeof(g_arrBasicEnabled) / sizeof(g_arrBasicEnabled[0])));
	xgeXuiListViewSetSelected(&pApp->tList[LIST_BASIC], 1);
	xgeXuiListViewSetItems(&pApp->tList[LIST_LARGE], pApp->arrLargeItems, LARGE_COUNT);
	xgeXuiListViewSetSelected(&pApp->tList[LIST_LARGE], 320);
	xgeXuiListViewSetScrollbarMode(&pApp->tList[LIST_LARGE], XGE_XUI_SCROLLBAR_MODE_FULL);
	xgeXuiListViewSetColors(&pApp->tList[LIST_LARGE], XGE_COLOR_RGBA(248, 250, 253, 255), XGE_COLOR_RGBA(244, 248, 252, 255), XGE_COLOR_RGBA(209, 230, 250, 255), XGE_COLOR_RGBA(42, 52, 66, 255), XGE_COLOR_RGBA(232, 238, 247, 255), XGE_COLOR_RGBA(74, 142, 210, 255));
	xgeXuiListViewSetItems(&pApp->tList[LIST_MULTI], g_arrMultiItems, MULTI_COUNT);
	xgeXuiListViewSetSelectionMode(&pApp->tList[LIST_MULTI], XGE_XUI_SELECTION_RANGE);
	xgeXuiListViewSetSelectionBuffer(&pApp->tList[LIST_MULTI], pApp->arrMultiSelected, MULTI_COUNT);
	xgeXuiListViewSetItemSelected(&pApp->tList[LIST_MULTI], 1, 1);
	xgeXuiListViewSetItemSelected(&pApp->tList[LIST_MULTI], 3, 1);
	xgeXuiListViewSetItemSelected(&pApp->tList[LIST_MULTI], 4, 1);
	xgeXuiListViewSetItems(&pApp->tList[LIST_RICH], g_arrRichTitle, (int)(sizeof(g_arrRichTitle) / sizeof(g_arrRichTitle[0])));
	xgeXuiListViewSetEnabledItems(&pApp->tList[LIST_RICH], g_arrRichEnabled, (int)(sizeof(g_arrRichEnabled) / sizeof(g_arrRichEnabled[0])));
	xgeXuiListViewSetItemRenderer(&pApp->tList[LIST_RICH], RichItemRenderer, pApp);
	xgeXuiListViewSetSelected(&pApp->tList[LIST_RICH], 0);
	return XGE_OK;
}

static void LayoutRoot(app_state_t* pApp)
{
	xge_xui_widget pRoot;
	float fRootW;
	float fRootH;
	float fPanelH;
	int iWidth;
	int iHeight;

	iWidth = xgeGetWidth();
	iHeight = xgeGetHeight();
	if ( (iWidth == pApp->iLastWidth) && (iHeight == pApp->iLastHeight) ) {
		return;
	}
	pRoot = xgeXuiRoot(&pApp->tXui);
	fRootW = (float)iWidth;
	fRootH = (float)iHeight;
	if ( fRootW < 920.0f ) {
		fRootW = 920.0f;
	}
	if ( fRootH < 640.0f ) {
		fRootH = 640.0f;
	}
	xgeXuiWidgetSetRect(pRoot, (xge_rect_t){ 0.0f, 0.0f, fRootW, fRootH });
	fPanelH = (fRootH - 36.0f - 12.0f) * 0.5f;
	xgeXuiWidgetSetGrid(pRoot, 2, fPanelH, 12.0f, 12.0f);
	pApp->iLastWidth = iWidth;
	pApp->iLastHeight = iHeight;
}

static void RunChecks(app_state_t* pApp)
{
	xge_event_t tEvent;
	xge_xui_widget pSlot;
	float fBefore;
	float fAfter;
	int i;

	pApp->bCreateOK = 1;
	for ( i = 0; i < LIST_COUNT; i++ ) {
		if ( (pApp->pListWidget[i] == NULL) || (pApp->tList[i].tBase.pWidget != pApp->pListWidget[i]) ) {
			pApp->bCreateOK = 0;
		}
	}
	pApp->bLayoutOK = (pApp->pPanel[0] != NULL) && (pApp->pPanel[0]->tRect.fW > 300.0f) && (pApp->pPanel[3] != NULL) && (pApp->pPanel[3]->tRect.fH > 220.0f);
	pApp->bStateOK =
		(xgeXuiListViewGetSelected(&pApp->tList[LIST_BASIC]) == 1) &&
		(xgeXuiListViewGetSelected(&pApp->tList[LIST_LARGE]) == 320) &&
		(xgeXuiListViewGetSelectionMode(&pApp->tList[LIST_MULTI]) == XGE_XUI_SELECTION_RANGE) &&
		xgeXuiListViewIsItemSelected(&pApp->tList[LIST_MULTI], 3) &&
		(pApp->tList[LIST_RICH].procItem == RichItemRenderer);
	pApp->bHoverWidthOK = 0;
	pApp->bHoverWidthOK = 1;
	for ( i = 0; i < LIST_COUNT; i++ ) {
		pSlot = pApp->tList[i].tBase.arrSlotWidget[0];
		if ( pSlot == NULL ) {
			pApp->bHoverWidthOK = 0;
			continue;
		}
		fBefore = pSlot->tContentRect.fW;
		memset(&tEvent, 0, sizeof(tEvent));
		tEvent.iType = XGE_EVENT_MOUSE_MOVE;
		tEvent.fX = pSlot->tContentRect.fX + 4.0f;
		tEvent.fY = pSlot->tContentRect.fY + 4.0f;
		for ( i = 0; i < 8; i++ ) {
			tEvent.fY += 1.0f;
			xgeXuiDispatchEvent(&pApp->tXui, &tEvent);
			xgeXuiUpdate(&pApp->tXui, 0.0f);
		}
		fAfter = pSlot->tContentRect.fW;
		if ( (fBefore != fAfter) || (fAfter < 250.0f) ) {
			pApp->bHoverWidthOK = 0;
		}
	}
}

static int AppEnter(xge_scene pScene)
{
	app_state_t* pApp;

	pApp = (app_state_t*)pScene->pUser;
	BuildLargeItems(pApp);
	pApp->bFontReady = (LoadFont(&pApp->tFont) == XGE_OK);
	if ( xgeXuiInit(&pApp->tXui) != XGE_OK || CreateUI(pApp) != XGE_OK ) {
		return XGE_ERROR;
	}
	LayoutRoot(pApp);
	xgeXuiUpdate(&pApp->tXui, 0.0f);
	RunChecks(pApp);
	return XGE_OK;
}

static int AppLeave(xge_scene pScene)
{
	app_state_t* pApp;
	int i;

	pApp = (app_state_t*)pScene->pUser;
	for ( i = 0; i < LIST_COUNT; i++ ) {
		xgeXuiListViewUnit(&pApp->tList[i]);
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
	if ( (pEvent->iType == XGE_EVENT_KEY_DOWN) && (pEvent->iParam1 == XGE_KEY_ESCAPE) ) {
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
	LayoutRoot(pApp);
	xgeXuiUpdate(&pApp->tXui, fDelta);
	pApp->iFrameCount++;
	if ( (pApp->iFrameLimit > 0) && (pApp->iFrameCount >= pApp->iFrameLimit) ) {
		printf("xui_listview final-summary frames=%d create=%d layout=%d state=%d hoverWidth=%d large_selected=%d large_scroll=%.2f\n",
			pApp->iFrameCount,
			pApp->bCreateOK,
			pApp->bLayoutOK,
			pApp->bStateOK,
			pApp->bHoverWidthOK,
			xgeXuiListViewGetSelected(&pApp->tList[LIST_LARGE]),
			xgeXuiListViewGetScroll(&pApp->tList[LIST_LARGE]));
		xgeQuit();
	}
	return XGE_OK;
}

static int AppDraw(xge_scene pScene)
{
	app_state_t* pApp;

	pApp = (app_state_t*)pScene->pUser;
	xgeBegin();
	xgeClear(XGE_COLOR_RGBA(236, 240, 246, 255));
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
	tApp.iFrameLimit = ArgInt(getenv("XGE_XUI_LISTVIEW_FRAMES"), 0);
	for ( i = 1; i < argc; i++ ) {
		if ( (strcmp(argv[i], "--frames") == 0) && ((i + 1) < argc) ) {
			tApp.iFrameLimit = ArgInt(argv[++i], tApp.iFrameLimit);
		}
	}
	tDesc.iWidth = 960;
	tDesc.iHeight = 680;
	tDesc.sTitle = "XUI ListView";
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
	return (iExitCode == XGE_OK && tApp.bCreateOK && tApp.bLayoutOK && tApp.bStateOK && tApp.bHoverWidthOK) ? 0 : 3;
}
