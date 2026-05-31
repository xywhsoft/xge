#include "xui.h"
#include "xge.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEMO_TARGET_W 760
#define DEMO_TARGET_H 380
#define DEMO_OFFSET_X 10.0f
#define DEMO_OFFSET_Y 20.0f
#define LIST_COUNT 3
#define LABEL_COUNT 5
#define ITEM_COUNT 18

typedef struct xui_listview_demo_t {
	xui_proxy_t tProxy;
	xui_context pContext;
	xui_surface pTarget;
	xui_font pFont;
	xui_widget pRoot;
	xui_widget pLabel[LABEL_COUNT];
	xui_widget pList[LIST_COUNT];
	xui_widget pStatus;
	int arrRangeSelection[ITEM_COUNT];
	int arrEnabled[ITEM_COUNT];
	int iFrame;
	int iMaxFrames;
	double fMaxSeconds;
	int bHasMouse;
	float fLastMouseX;
	float fLastMouseY;
	float fUiMouseX;
	float fUiMouseY;
	uint32_t iLastButtons;
	int bExerciseDone;
	int bCreateOK;
	int bLayoutOK;
	int bSelectOK;
	int bRangeOK;
	int bScrollOK;
	int iSelectCount;
	int iLastIndex;
} xui_listview_demo_t;

static const char* g_arrItems[ITEM_COUNT] = {
	"Scene graph", "Main camera", "Disabled physics layer", "Player controller", "Enemy spawner", "Lighting pass",
	"Audio bus", "Dialogue trigger", "Inventory root", "Quest tracker", "Save point", "Weather volume",
	"Fog controller", "Mini map", "Debug overlay", "Localization", "Input profile", "Build marker"
};

static void __xuiListViewUsage(void)
{
	printf("usage: xui_listview [--frames N] [--seconds N]\n");
	printf("       no duration option means run until the window is closed.\n");
}

static int __xuiListViewParseArgs(xui_listview_demo_t* pDemo, int argc, char** argv)
{
	int i;

	for ( i = 1; i < argc; i++ ) {
		if ( strcmp(argv[i], "--frames") == 0 ) {
			if ( i + 1 >= argc ) return XGE_ERROR_INVALID_ARGUMENT;
			pDemo->iMaxFrames = atoi(argv[++i]);
			if ( pDemo->iMaxFrames <= 0 ) return XGE_ERROR_INVALID_ARGUMENT;
		} else if ( strncmp(argv[i], "--frames=", 9) == 0 ) {
			pDemo->iMaxFrames = atoi(argv[i] + 9);
			if ( pDemo->iMaxFrames <= 0 ) return XGE_ERROR_INVALID_ARGUMENT;
		} else if ( strcmp(argv[i], "--seconds") == 0 ) {
			if ( i + 1 >= argc ) return XGE_ERROR_INVALID_ARGUMENT;
			pDemo->fMaxSeconds = atof(argv[++i]);
			if ( pDemo->fMaxSeconds <= 0.0 ) return XGE_ERROR_INVALID_ARGUMENT;
		} else if ( strncmp(argv[i], "--seconds=", 10) == 0 ) {
			pDemo->fMaxSeconds = atof(argv[i] + 10);
			if ( pDemo->fMaxSeconds <= 0.0 ) return XGE_ERROR_INVALID_ARGUMENT;
		} else if ( strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0 ) {
			__xuiListViewUsage();
			return 1;
		} else {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
	}
	return XGE_OK;
}

static const char* __xuiListViewFindTtf(void)
{
	static const char* arrPaths[] = {
		"C:\\Windows\\Fonts\\segoeui.ttf",
		"C:\\Windows\\Fonts\\arial.ttf",
		"C:\\Windows\\Fonts\\calibri.ttf",
		"C:\\Windows\\Fonts\\msyh.ttc",
		"C:\\Windows\\Fonts\\simhei.ttf"
	};
	FILE* pFile;
	int i;

	for ( i = 0; i < (int)(sizeof(arrPaths) / sizeof(arrPaths[0])); i++ ) {
		pFile = fopen(arrPaths[i], "rb");
		if ( pFile != NULL ) {
			fclose(pFile);
			return arrPaths[i];
		}
	}
	return NULL;
}

static int __xuiListViewRootRender(xui_widget pWidget, xui_draw_context pDraw, uint32_t iStateId, void* pUser)
{
	xui_listview_demo_t* pDemo;
	xui_rect_t tRect;
	xui_rect_t tPanel;

	(void)iStateId;
	pDemo = (xui_listview_demo_t*)pUser;
	if ( (pWidget == NULL) || (pDraw == NULL) || (pDemo == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	tRect = xuiWidgetGetContentRect(pWidget);
	if ( pDemo->tProxy.drawRectFill != NULL ) {
		(void)pDemo->tProxy.drawRectFill(&pDemo->tProxy, pDraw, tRect, XUI_COLOR_RGBA(234, 242, 250, 255));
	}
	if ( pDemo->tProxy.drawRoundRectFill != NULL ) {
		tPanel = (xui_rect_t){24.0f, 22.0f, tRect.fW - 48.0f, tRect.fH - 44.0f};
		(void)pDemo->tProxy.drawRoundRectFill(&pDemo->tProxy, pDraw, tPanel, 6.0f, XUI_COLOR_RGBA(248, 251, 255, 255));
	}
	return XUI_OK;
}

static void __xuiListViewSelected(xui_widget pWidget, int iIndex, void* pUser)
{
	xui_listview_demo_t* pDemo;
	char sText[160];

	(void)pWidget;
	pDemo = (xui_listview_demo_t*)pUser;
	if ( pDemo == NULL ) return;
	pDemo->iSelectCount++;
	pDemo->iLastIndex = iIndex;
	snprintf(sText, sizeof(sText), "Selected index=%d changes=%d", iIndex, pDemo->iSelectCount);
	if ( pDemo->pStatus != NULL ) {
		(void)xuiLabelSetText(pDemo->pStatus, sText);
	}
}

static int __xuiListViewAddLabel(xui_listview_demo_t* pDemo, int iIndex, const char* sText, xui_rect_t tRect)
{
	xui_label_desc_t tDesc;
	xui_widget pLabel;
	int iRet;

	if ( (iIndex < 0) || (iIndex >= LABEL_COUNT) ) return XUI_ERROR_INVALID_ARGUMENT;
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.sText = sText;
	tDesc.pFont = pDemo->pFont;
	tDesc.iTextColor = XUI_COLOR_RGBA(54, 68, 88, 255);
	tDesc.iTextFlags = XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP;
	iRet = xuiLabelCreate(pDemo->pContext, &pLabel, &tDesc);
	if ( iRet != XUI_OK ) return iRet;
	xuiWidgetSetRect(pLabel, tRect);
	iRet = xuiWidgetAddChild(pDemo->pRoot, pLabel);
	if ( iRet != XUI_OK ) {
		xuiWidgetDestroy(pLabel);
		return iRet;
	}
	pDemo->pLabel[iIndex] = pLabel;
	return XUI_OK;
}

static int __xuiListViewAddList(xui_listview_demo_t* pDemo, int iIndex, xui_rect_t tRect)
{
	xui_list_view_desc_t tDesc;
	xui_widget pList;
	int iRet;

	if ( (iIndex < 0) || (iIndex >= LIST_COUNT) ) return XUI_ERROR_INVALID_ARGUMENT;
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.arrItems = g_arrItems;
	tDesc.arrEnabled = pDemo->arrEnabled;
	tDesc.iItemCount = ITEM_COUNT;
	tDesc.iSelected = iIndex + 1;
	tDesc.pFont = pDemo->pFont;
	tDesc.fItemHeight = 24.0f;
	if ( iIndex == 1 ) {
		tDesc.iSelectionMode = XUI_SELECTION_RANGE;
		tDesc.arrSelected = pDemo->arrRangeSelection;
		tDesc.iSelectionCount = ITEM_COUNT;
	}
	iRet = xuiListViewCreate(pDemo->pContext, &pList, &tDesc);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiListViewSetSelect(pList, __xuiListViewSelected, pDemo);
	if ( iIndex == 2 ) {
		(void)xuiListViewSetColors(pList,
			XUI_COLOR_RGBA(247, 253, 250, 255),
			XUI_COLOR_RGBA(103, 176, 146, 255),
			XUI_COLOR_RGBA(38, 141, 104, 255),
			XUI_COLOR_RGBA(255, 255, 255, 0),
			XUI_COLOR_RGBA(226, 247, 239, 255),
			XUI_COLOR_RGBA(42, 151, 110, 255),
			XUI_COLOR_RGBA(31, 76, 60, 255),
			XUI_COLOR_RGBA(128, 152, 144, 210));
		(void)xuiListViewSetScrollbarColors(pList,
			XUI_COLOR_RGBA(219, 239, 233, 255),
			XUI_COLOR_RGBA(47, 150, 110, 245),
			XUI_COLOR_RGBA(31, 170, 120, 250),
			XUI_COLOR_RGBA(19, 125, 88, 255),
			XUI_COLOR_RGBA(39, 150, 106, 180),
			XUI_COLOR_RGBA(158, 174, 168, 135));
	}
	xuiWidgetSetRect(pList, tRect);
	iRet = xuiWidgetAddChild(pDemo->pRoot, pList);
	if ( iRet != XUI_OK ) {
		xuiWidgetDestroy(pList);
		return iRet;
	}
	pDemo->pList[iIndex] = pList;
	return XUI_OK;
}

static int __xuiListViewCreateUi(xui_listview_demo_t* pDemo)
{
	int i;
	int iRet;

	for ( i = 0; i < ITEM_COUNT; i++ ) {
		pDemo->arrEnabled[i] = 1;
	}
	pDemo->arrEnabled[2] = 0;
	iRet = xuiWidgetCreate(pDemo->pContext, &pDemo->pRoot);
	if ( iRet != XUI_OK ) return iRet;
	xuiWidgetSetRect(pDemo->pRoot, (xui_rect_t){0.0f, 0.0f, (float)DEMO_TARGET_W, (float)DEMO_TARGET_H});
	(void)xuiWidgetSetLayoutType(pDemo->pRoot, XUI_LAYOUT_MANUAL);
	(void)xuiWidgetSetCacheRenderCallback(pDemo->pRoot, __xuiListViewRootRender, pDemo);
	iRet = xuiSetRootWidget(pDemo->pContext, pDemo->pRoot);
	if ( iRet != XUI_OK ) return iRet;
	if ( __xuiListViewAddLabel(pDemo, 0, "XUI ListView", (xui_rect_t){48.0f, 30.0f, 180.0f, 24.0f}) != XUI_OK ||
	     __xuiListViewAddLabel(pDemo, 1, "Single selection", (xui_rect_t){48.0f, 62.0f, 180.0f, 22.0f}) != XUI_OK ||
	     __xuiListViewAddLabel(pDemo, 2, "Range selection", (xui_rect_t){285.0f, 62.0f, 180.0f, 22.0f}) != XUI_OK ||
	     __xuiListViewAddLabel(pDemo, 3, "Custom colors", (xui_rect_t){522.0f, 62.0f, 180.0f, 22.0f}) != XUI_OK ||
	     __xuiListViewAddLabel(pDemo, 4, "Selected index=none changes=0", (xui_rect_t){48.0f, 318.0f, 360.0f, 24.0f}) != XUI_OK ) {
		return XUI_ERROR;
	}
	pDemo->pStatus = pDemo->pLabel[4];
	if ( __xuiListViewAddList(pDemo, 0, (xui_rect_t){48.0f, 88.0f, 205.0f, 210.0f}) != XUI_OK ||
	     __xuiListViewAddList(pDemo, 1, (xui_rect_t){285.0f, 88.0f, 205.0f, 210.0f}) != XUI_OK ||
	     __xuiListViewAddList(pDemo, 2, (xui_rect_t){522.0f, 88.0f, 190.0f, 210.0f}) != XUI_OK ) {
		return XUI_ERROR;
	}
	return XUI_OK;
}

static uint32_t __xuiListViewReadButtons(void)
{
	uint32_t iButtons;

	iButtons = 0;
	if ( xgeMouseDown(XGE_MOUSE_LEFT) ) iButtons |= XUI_POINTER_BUTTON_LEFT;
	if ( xgeMouseDown(XGE_MOUSE_RIGHT) ) iButtons |= XUI_POINTER_BUTTON_RIGHT;
	if ( xgeMouseDown(XGE_MOUSE_MIDDLE) ) iButtons |= XUI_POINTER_BUTTON_MIDDLE;
	return iButtons;
}

static int __xuiListViewHandleInput(xui_listview_demo_t* pDemo)
{
	float fX;
	float fY;
	float fWheelX;
	float fWheelY;
	uint32_t iButtons;
	uint32_t iPressed;
	uint32_t iReleased;
	uint32_t iMods;
	int iRet;

	if ( xgeKeyPressed(XGE_KEY_ESCAPE) ) {
		xgeQuit();
	}
	xgeMouseGet(&fX, &fY);
	xgeMouseGetWheel(&fWheelX, &fWheelY);
	pDemo->fUiMouseX = fX - DEMO_OFFSET_X;
	pDemo->fUiMouseY = fY - DEMO_OFFSET_Y;
	iButtons = __xuiListViewReadButtons();
	if ( !pDemo->bHasMouse || (pDemo->fLastMouseX != fX) || (pDemo->fLastMouseY != fY) || (pDemo->iLastButtons != iButtons) ) {
		iRet = xuiInputPointerMove(pDemo->pContext, pDemo->fUiMouseX, pDemo->fUiMouseY, iButtons);
		if ( iRet != XUI_OK ) return iRet;
	}
	if ( fWheelX != 0.0f || fWheelY != 0.0f ) {
		iRet = xuiInputPointerWheel(pDemo->pContext, pDemo->fUiMouseX, pDemo->fUiMouseY, fWheelX, fWheelY, iButtons);
		if ( iRet != XUI_OK ) return iRet;
	}
	iPressed = iButtons & ~pDemo->iLastButtons;
	iReleased = pDemo->iLastButtons & ~iButtons;
	if ( (iPressed & XUI_POINTER_BUTTON_LEFT) != 0 ) {
		iRet = xuiInputPointerDown(pDemo->pContext, pDemo->fUiMouseX, pDemo->fUiMouseY, XUI_POINTER_BUTTON_LEFT, iButtons);
		if ( iRet != XUI_OK ) return iRet;
	}
	if ( (iReleased & XUI_POINTER_BUTTON_LEFT) != 0 ) {
		iRet = xuiInputPointerUp(pDemo->pContext, pDemo->fUiMouseX, pDemo->fUiMouseY, XUI_POINTER_BUTTON_LEFT, iButtons);
		if ( iRet != XUI_OK ) return iRet;
	}
	iMods = 0;
	if ( xgeKeyPressed(XGE_KEY_UP) ) (void)xuiInputKeyDown(pDemo->pContext, XUI_KEY_UP, iMods);
	if ( xgeKeyPressed(XGE_KEY_DOWN) ) (void)xuiInputKeyDown(pDemo->pContext, XUI_KEY_DOWN, iMods);
	if ( xgeKeyPressed(XGE_KEY_HOME) ) (void)xuiInputKeyDown(pDemo->pContext, XUI_KEY_HOME, iMods);
	if ( xgeKeyPressed(XGE_KEY_END) ) (void)xuiInputKeyDown(pDemo->pContext, XUI_KEY_END, iMods);
	pDemo->bHasMouse = 1;
	pDemo->fLastMouseX = fX;
	pDemo->fLastMouseY = fY;
	pDemo->iLastButtons = iButtons;
	return XUI_OK;
}

static int __xuiListViewClickItem(xui_listview_demo_t* pDemo, xui_widget pList, int iIndex)
{
	xui_rect_t tWorld;
	xui_rect_t tItem;
	float fX;
	float fY;
	int iRet;

	tWorld = xuiWidgetGetWorldRect(pList);
	tItem = xuiListViewGetItemRect(pList, iIndex);
	fX = tWorld.fX + tItem.fX + 12.0f;
	fY = tWorld.fY + tItem.fY + tItem.fH * 0.5f;
	iRet = xuiInputPointerDown(pDemo->pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pDemo->pContext);
	if ( iRet == XUI_OK ) iRet = xuiInputPointerUp(pDemo->pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, 0);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pDemo->pContext);
	return iRet;
}

static void __xuiListViewRunChecks(xui_listview_demo_t* pDemo, int bExerciseInput)
{
	pDemo->bCreateOK = (pDemo->pRoot != NULL) && (pDemo->pList[0] != NULL) && (xuiListViewGetFrameWidget(pDemo->pList[0]) != NULL);
	pDemo->bLayoutOK = xuiScrollFrameIsVScrollBarVisible(xuiListViewGetFrameWidget(pDemo->pList[0])) &&
		!xuiScrollFrameIsHScrollBarVisible(xuiListViewGetFrameWidget(pDemo->pList[0]));
	if ( bExerciseInput && !pDemo->bExerciseDone ) {
		(void)__xuiListViewClickItem(pDemo, pDemo->pList[0], 4);
		pDemo->bSelectOK = (xuiListViewGetSelected(pDemo->pList[0]) == 4) && (pDemo->iLastIndex == 4);
		(void)xuiSetFocusWidget(pDemo->pContext, pDemo->pList[1]);
		(void)xuiListViewSetSelected(pDemo->pList[1], 5);
		(void)xuiInputKeyDown(pDemo->pContext, XUI_KEY_DOWN, XUI_MOD_SHIFT);
		(void)xuiDispatchPendingEvents(pDemo->pContext);
		pDemo->bRangeOK = pDemo->arrRangeSelection[5] && pDemo->arrRangeSelection[6];
		(void)xuiListViewEnsureVisible(pDemo->pList[2], ITEM_COUNT - 1);
		pDemo->bScrollOK = xuiListViewGetScroll(pDemo->pList[2]) > 0.0f;
		pDemo->bExerciseDone = 1;
	} else if ( !bExerciseInput ) {
		pDemo->bSelectOK = 1;
		pDemo->bRangeOK = 1;
		pDemo->bScrollOK = 1;
	}
}

static int __xuiListViewCreateAssets(xui_listview_demo_t* pDemo)
{
	xui_surface_desc_t tSurfaceDesc;
	const char* sFontPath;
	int iRet;

	pDemo->tProxy = xuiProxyXge();
	iRet = xuiCreate(&pDemo->pContext);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiSetProxy(pDemo->pContext, &pDemo->tProxy);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiInputViewport(pDemo->pContext, (float)DEMO_TARGET_W, (float)DEMO_TARGET_H);
	if ( iRet != XUI_OK ) return iRet;
	memset(&tSurfaceDesc, 0, sizeof(tSurfaceDesc));
	tSurfaceDesc.iKind = XUI_SURFACE_KIND_TEXTURE;
	tSurfaceDesc.iWidth = DEMO_TARGET_W;
	tSurfaceDesc.iHeight = DEMO_TARGET_H;
	tSurfaceDesc.iFormat = XUI_SURFACE_FORMAT_RGBA8;
	tSurfaceDesc.iFlags = XUI_SURFACE_ALPHA_PREMULTIPLIED | XUI_SURFACE_USAGE_TARGET;
	iRet = pDemo->tProxy.surfaceCreate(&pDemo->tProxy, &pDemo->pTarget, &tSurfaceDesc);
	if ( iRet != XUI_OK ) return iRet;
	sFontPath = __xuiListViewFindTtf();
	if ( sFontPath == NULL ) return XUI_ERROR_FILE_NOT_FOUND;
	iRet = pDemo->tProxy.fontLoadFile(&pDemo->tProxy, &pDemo->pFont, sFontPath, 14.0f, XUI_FONT_FORMAT_TTF);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiSetDefaultFont(pDemo->pContext, pDemo->pFont);
	return __xuiListViewCreateUi(pDemo);
}

static void __xuiListViewDestroyAssets(xui_listview_demo_t* pDemo)
{
	if ( pDemo->pContext != NULL ) {
		xuiDestroy(pDemo->pContext);
		pDemo->pContext = NULL;
	}
	if ( pDemo->pFont != NULL ) {
		pDemo->tProxy.fontDestroy(&pDemo->tProxy, pDemo->pFont);
		pDemo->pFont = NULL;
	}
	if ( pDemo->pTarget != NULL ) {
		pDemo->tProxy.surfaceDestroy(&pDemo->tProxy, pDemo->pTarget);
		pDemo->pTarget = NULL;
	}
}

static int __xuiListViewFrame(void* pUser)
{
	xui_listview_demo_t* pDemo;
	xui_rect_i_t tFullRect;
	xui_rect_t tSrc;
	xui_rect_t tDst;
	xui_render_stats_t tStats;
	int iRet;
	int bAutoRun;

	pDemo = (xui_listview_demo_t*)pUser;
	if ( pDemo == NULL ) return XGE_ERROR_INVALID_ARGUMENT;
	bAutoRun = (pDemo->iMaxFrames > 0) || (pDemo->fMaxSeconds > 0.0);
	iRet = xgeBegin();
	if ( iRet != XGE_OK ) return iRet;
	iRet = __xuiListViewHandleInput(pDemo);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiDispatchPendingEvents(pDemo->pContext);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiLayout(pDemo->pContext);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiUpdate(pDemo->pContext, xgeGetDelta());
	if ( iRet != XUI_OK ) return iRet;
	__xuiListViewRunChecks(pDemo, bAutoRun);
	iRet = pDemo->tProxy.surfaceClear(&pDemo->tProxy, pDemo->pTarget, XUI_COLOR_RGBA(234, 242, 250, 255));
	if ( iRet != XUI_OK ) return iRet;
	tFullRect = (xui_rect_i_t){0, 0, DEMO_TARGET_W, DEMO_TARGET_H};
	iRet = xuiRender(pDemo->pContext, pDemo->pTarget, &tFullRect, 1);
	if ( iRet != XUI_OK ) return iRet;
	xgeClear(XUI_COLOR_RGBA(18, 23, 32, 255));
	tSrc = (xui_rect_t){0.0f, 0.0f, (float)DEMO_TARGET_W, (float)DEMO_TARGET_H};
	tDst = (xui_rect_t){DEMO_OFFSET_X, DEMO_OFFSET_Y, (float)DEMO_TARGET_W, (float)DEMO_TARGET_H};
	iRet = pDemo->tProxy.surfaceDraw(&pDemo->tProxy, pDemo->pTarget, tSrc, tDst, XUI_COLOR_WHITE, XUI_SURFACE_DRAW_SCREEN_SPACE);
	if ( iRet == XUI_OK ) iRet = xgeEnd();
	if ( iRet != XGE_OK ) return iRet;
	pDemo->iFrame++;
	if ( (pDemo->iMaxFrames > 0 && pDemo->iFrame >= pDemo->iMaxFrames) ||
	     (pDemo->fMaxSeconds > 0.0 && xgeTimer() >= pDemo->fMaxSeconds) ) {
		memset(&tStats, 0, sizeof(tStats));
		(void)xuiGetRenderStats(pDemo->pContext, &tStats);
		printf("xui_listview final-summary frames=%d create=%d layout=%d select=%d range=%d scroll=%d selected=%d updatedCaches=%d drawnCaches=%d\n",
			pDemo->iFrame, pDemo->bCreateOK, pDemo->bLayoutOK, pDemo->bSelectOK, pDemo->bRangeOK,
			pDemo->bScrollOK, pDemo->iLastIndex, tStats.iUpdatedCaches, tStats.iDrawnCaches);
		xgeQuit();
	}
	return XGE_OK;
}

int main(int argc, char** argv)
{
	xui_listview_demo_t tDemo;
	xge_desc_t tDesc;
	int iRet;

	memset(&tDemo, 0, sizeof(tDemo));
	iRet = __xuiListViewParseArgs(&tDemo, argc, argv);
	if ( iRet == 1 ) return 0;
	if ( iRet != XGE_OK ) {
		__xuiListViewUsage();
		return 1;
	}
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iWidth = DEMO_TARGET_W + 20;
	tDesc.iHeight = DEMO_TARGET_H + 50;
	tDesc.sTitle = "XUI ListView";
	tDesc.iFlags = XGE_INIT_WINDOW | XGE_INIT_VSYNC;
	tDesc.iRunMode = XGE_RUN_GAME_LOOP;
	tDesc.iTargetFPS = 60;
	iRet = xgeInit(&tDesc);
	if ( iRet != XGE_OK ) {
		printf("xui_listview: xgeInit failed: %d\n", iRet);
		return 1;
	}
	iRet = __xuiListViewCreateAssets(&tDemo);
	if ( iRet != XUI_OK ) {
		printf("xui_listview: create assets failed: %d\n", iRet);
		__xuiListViewDestroyAssets(&tDemo);
		xgeUnit();
		return 1;
	}
	iRet = xgeRun(__xuiListViewFrame, &tDemo);
	__xuiListViewDestroyAssets(&tDemo);
	xgeUnit();
	return (iRet == XGE_OK && tDemo.bCreateOK && tDemo.bLayoutOK && tDemo.bSelectOK && tDemo.bRangeOK && tDemo.bScrollOK) ? 0 : 1;
}
