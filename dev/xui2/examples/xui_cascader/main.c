#include "xui.h"
#include "xge.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEMO_TARGET_W	720
#define DEMO_TARGET_H	420
#define DEMO_OFFSET_X	10.0f
#define DEMO_OFFSET_Y	20.0f
#define CASCADER_COUNT	3
#define LABEL_COUNT	7

typedef struct xui_cascader_demo_t {
	xui_context pContext;
	xui_proxy_t tProxy;
	xui_surface pTarget;
	xui_font pFont;
	xui_widget pRoot;
	xui_widget pLabel[LABEL_COUNT];
	xui_widget pCascader[CASCADER_COUNT];
	xui_widget pStatus;
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
	int bOpenOK;
	int bSelectOK;
	int bClearOK;
	int bKeyOK;
	int bPlacementOK;
	int iChangeCount;
	int iLastLeaf;
	int iLastDepth;
	int arrLastValues[XUI_CASCADER_PATH_CAPACITY];
} xui_cascader_demo_t;

static const xui_cascader_item_t g_arrCascaderItems[] = {
	{"Guide", 1, -1, 0u, NULL},
	{"Components", 2, -1, 0u, NULL},
	{"Resources", 3, -1, 0u, NULL},
	{"Design", 11, 0, XUI_CASCADER_ITEM_LEAF, NULL},
	{"Navigation", 12, 0, 0u, NULL},
	{"Side nav", 121, 4, XUI_CASCADER_ITEM_LEAF, NULL},
	{"Top nav", 122, 4, XUI_CASCADER_ITEM_LEAF, NULL},
	{"Disabled leaf", 123, 4, XUI_CASCADER_ITEM_DISABLED | XUI_CASCADER_ITEM_LEAF, NULL},
	{"Forms", 21, 1, 0u, NULL},
	{"Input", 211, 8, XUI_CASCADER_ITEM_LEAF, NULL},
	{"DatePicker", 212, 8, XUI_CASCADER_ITEM_LEAF, NULL},
	{"Images", 31, 2, XUI_CASCADER_ITEM_LEAF, NULL},
	{"Downloads", 32, 2, XUI_CASCADER_ITEM_LEAF, NULL}
};

static void __xuiCascaderUsage(void)
{
	printf("usage: xui_cascader [--frames N] [--seconds N]\n");
	printf("       no duration option means run until the window is closed.\n");
}

static int __xuiCascaderParseArgs(xui_cascader_demo_t* pDemo, int argc, char** argv)
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
			__xuiCascaderUsage();
			return 1;
		} else {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
	}
	return XGE_OK;
}

static const char* __xuiCascaderFindTtf(void)
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

static int __xuiCascaderRootRender(xui_widget pWidget, xui_draw_context pDraw, uint32_t iStateId, void* pUser)
{
	xui_cascader_demo_t* pDemo;
	xui_rect_t tRect;
	xui_rect_t tPanel;

	(void)iStateId;
	pDemo = (xui_cascader_demo_t*)pUser;
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

static void __xuiCascaderChanged(xui_widget pWidget, int iLeafIndex, const int* arrValues, int iDepth, void* pUser)
{
	xui_cascader_demo_t* pDemo;
	char sText[180];
	int i;

	(void)pWidget;
	pDemo = (xui_cascader_demo_t*)pUser;
	if ( pDemo == NULL ) return;
	pDemo->iChangeCount++;
	pDemo->iLastLeaf = iLeafIndex;
	pDemo->iLastDepth = iDepth;
	for ( i = 0; i < XUI_CASCADER_PATH_CAPACITY; i++ ) {
		pDemo->arrLastValues[i] = (arrValues != NULL && i < iDepth) ? arrValues[i] : -1;
	}
	snprintf(sText, sizeof(sText), "changes=%d leaf=%d depth=%d text=%s",
		pDemo->iChangeCount, iLeafIndex, iDepth, xuiCascaderGetSelectedText(pWidget));
	if ( pDemo->pStatus != NULL ) {
		(void)xuiLabelSetText(pDemo->pStatus, sText);
	}
}

static int __xuiCascaderAddLabel(xui_cascader_demo_t* pDemo, int iIndex, const char* sText, xui_rect_t tRect)
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

static int __xuiCascaderCreateControls(xui_cascader_demo_t* pDemo)
{
	xui_cascader_desc_t tDesc;
	int arrFirst[3] = {1, 12, 121};
	int arrSecond[3] = {2, 21, 211};
	int arrThird[2] = {1, 12};
	int iRet;

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.arrItems = g_arrCascaderItems;
	tDesc.iItemCount = (int)(sizeof(g_arrCascaderItems) / sizeof(g_arrCascaderItems[0]));
	tDesc.arrSelectedValues = arrFirst;
	tDesc.iSelectedDepth = 3;
	tDesc.pFont = pDemo->pFont;
	tDesc.bClearable = 1;
	tDesc.fColumnWidth = 126.0f;
	tDesc.fPopupMaxHeight = 176.0f;
	iRet = xuiCascaderCreate(pDemo->pContext, &pDemo->pCascader[0], &tDesc);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiCascaderSetChange(pDemo->pCascader[0], __xuiCascaderChanged, pDemo);

	tDesc.arrSelectedValues = arrSecond;
	tDesc.iSelectedDepth = 3;
	iRet = xuiCascaderCreate(pDemo->pContext, &pDemo->pCascader[1], &tDesc);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiCascaderSetChange(pDemo->pCascader[1], __xuiCascaderChanged, pDemo);
	(void)xuiCascaderSetShowAllLevels(pDemo->pCascader[1], 0);

	tDesc.arrSelectedValues = arrThird;
	tDesc.iSelectedDepth = 2;
	tDesc.bSelectAnyLevel = 1;
	tDesc.iExpandTrigger = XUI_CASCADER_EXPAND_HOVER;
	iRet = xuiCascaderCreate(pDemo->pContext, &pDemo->pCascader[2], &tDesc);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiCascaderSetChange(pDemo->pCascader[2], __xuiCascaderChanged, pDemo);
	(void)xuiCascaderSetPopupPlacement(pDemo->pCascader[2], XUI_CASCADER_POPUP_TOP);
	(void)xuiCascaderSetColors(pDemo->pCascader[2],
		XUI_COLOR_RGBA(24, 78, 66, 255),
		XUI_COLOR_RGBA(130, 158, 152, 210),
		XUI_COLOR_RGBA(132, 154, 148, 210),
		XUI_COLOR_RGBA(244, 252, 249, 255),
		XUI_COLOR_RGBA(229, 247, 240, 255),
		XUI_COLOR_RGBA(235, 250, 244, 255),
		XUI_COLOR_RGBA(235, 240, 245, 255));
	(void)xuiCascaderSetBorderColors(pDemo->pCascader[2],
		XUI_COLOR_RGBA(90, 176, 146, 255),
		XUI_COLOR_RGBA(54, 150, 115, 255),
		XUI_COLOR_RGBA(37, 128, 94, 255));
	(void)xuiCascaderSetArrowColors(pDemo->pCascader[2],
		XUI_COLOR_RGBA(32, 132, 94, 255),
		XUI_COLOR_RGBA(130, 158, 152, 160));
	(void)xuiCascaderSetPopupColors(pDemo->pCascader[2],
		XUI_COLOR_RGBA(249, 254, 252, 255),
		XUI_COLOR_RGBA(120, 190, 164, 255),
		XUI_COLOR_RGBA(35, 82, 66, 42),
		XUI_COLOR_RGBA(24, 78, 66, 255),
		XUI_COLOR_RGBA(98, 132, 124, 220),
		XUI_COLOR_RGBA(230, 247, 240, 255),
		XUI_COLOR_RGBA(29, 185, 124, 255),
		XUI_COLOR_RGBA(214, 239, 230, 255),
		XUI_COLOR_RGBA(255, 255, 255, 255),
		XUI_COLOR_RGBA(132, 154, 148, 210),
		XUI_COLOR_RGBA(196, 224, 214, 255));
	return XUI_OK;
}

static int __xuiCascaderCreateUi(xui_cascader_demo_t* pDemo)
{
	int iRet;

	iRet = xuiWidgetCreate(pDemo->pContext, &pDemo->pRoot);
	if ( iRet != XUI_OK ) return iRet;
	xuiWidgetSetRect(pDemo->pRoot, (xui_rect_t){0.0f, 0.0f, (float)DEMO_TARGET_W, (float)DEMO_TARGET_H});
	(void)xuiWidgetSetLayoutType(pDemo->pRoot, XUI_LAYOUT_MANUAL);
	(void)xuiWidgetSetCacheRenderCallback(pDemo->pRoot, __xuiCascaderRootRender, pDemo);
	iRet = xuiSetRootWidget(pDemo->pContext, pDemo->pRoot);
	if ( iRet != XUI_OK ) return iRet;
	if ( __xuiCascaderAddLabel(pDemo, 0, "XUI Cascader", (xui_rect_t){48.0f, 30.0f, 200.0f, 24.0f}) != XUI_OK ||
	     __xuiCascaderAddLabel(pDemo, 1, "Show all levels", (xui_rect_t){48.0f, 72.0f, 150.0f, 28.0f}) != XUI_OK ||
	     __xuiCascaderAddLabel(pDemo, 2, "Last level only", (xui_rect_t){48.0f, 126.0f, 150.0f, 28.0f}) != XUI_OK ||
	     __xuiCascaderAddLabel(pDemo, 3, "Hover + top popup", (xui_rect_t){48.0f, 280.0f, 150.0f, 28.0f}) != XUI_OK ||
	     __xuiCascaderAddLabel(pDemo, 4, "Output:", (xui_rect_t){390.0f, 72.0f, 80.0f, 28.0f}) != XUI_OK ||
	     __xuiCascaderAddLabel(pDemo, 5, "changes=0 leaf=none depth=0 text=none", (xui_rect_t){450.0f, 72.0f, 230.0f, 28.0f}) != XUI_OK ||
	     __xuiCascaderAddLabel(pDemo, 6, "Open a selector, choose a leaf, or clear the selected path.", (xui_rect_t){48.0f, 344.0f, 540.0f, 28.0f}) != XUI_OK ) {
		return XUI_ERROR;
	}
	pDemo->pStatus = pDemo->pLabel[5];
	iRet = __xuiCascaderCreateControls(pDemo);
	if ( iRet != XUI_OK ) return iRet;
	xuiWidgetSetRect(pDemo->pCascader[0], (xui_rect_t){180.0f, 72.0f, 190.0f, 30.0f});
	xuiWidgetSetRect(pDemo->pCascader[1], (xui_rect_t){180.0f, 126.0f, 190.0f, 30.0f});
	xuiWidgetSetRect(pDemo->pCascader[2], (xui_rect_t){180.0f, 280.0f, 190.0f, 30.0f});
	iRet = xuiWidgetAddChild(pDemo->pRoot, pDemo->pCascader[0]);
	if ( iRet == XUI_OK ) iRet = xuiWidgetAddChild(pDemo->pRoot, pDemo->pCascader[1]);
	if ( iRet == XUI_OK ) iRet = xuiWidgetAddChild(pDemo->pRoot, pDemo->pCascader[2]);
	return iRet;
}

static uint32_t __xuiCascaderReadButtons(void)
{
	uint32_t iButtons;

	iButtons = 0;
	if ( xgeMouseDown(XGE_MOUSE_LEFT) ) iButtons |= XUI_POINTER_BUTTON_LEFT;
	if ( xgeMouseDown(XGE_MOUSE_RIGHT) ) iButtons |= XUI_POINTER_BUTTON_RIGHT;
	if ( xgeMouseDown(XGE_MOUSE_MIDDLE) ) iButtons |= XUI_POINTER_BUTTON_MIDDLE;
	return iButtons;
}

static int __xuiCascaderHandleInput(xui_cascader_demo_t* pDemo)
{
	float fX;
	float fY;
	float fWheelX;
	float fWheelY;
	uint32_t iButtons;
	uint32_t iPressed;
	uint32_t iReleased;
	int iRet;

	if ( xgeKeyPressed(XGE_KEY_ESCAPE) ) {
		if ( xuiCascaderIsOpen(pDemo->pCascader[0]) || xuiCascaderIsOpen(pDemo->pCascader[1]) || xuiCascaderIsOpen(pDemo->pCascader[2]) ) {
			iRet = xuiInputKeyDown(pDemo->pContext, XUI_KEY_ESCAPE, 0);
			if ( iRet != XUI_OK ) return iRet;
		} else {
			xgeQuit();
		}
	}
	xgeMouseGet(&fX, &fY);
	xgeMouseGetWheel(&fWheelX, &fWheelY);
	pDemo->fUiMouseX = fX - DEMO_OFFSET_X;
	pDemo->fUiMouseY = fY - DEMO_OFFSET_Y;
	iButtons = __xuiCascaderReadButtons();
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
	pDemo->bHasMouse = 1;
	pDemo->fLastMouseX = fX;
	pDemo->fLastMouseY = fY;
	pDemo->iLastButtons = iButtons;
	return XUI_OK;
}

static int __xuiCascaderClickItem(xui_cascader_demo_t* pDemo, xui_widget pCascader, int iColumn, int iItem)
{
	xui_widget pPanel;
	xui_rect_t tWorld;
	xui_rect_t tItem;
	int iRet;

	pPanel = xuiCascaderGetPanelWidget(pCascader);
	if ( pPanel == NULL ) return XUI_ERROR;
	tWorld = xuiWidgetGetWorldRect(pPanel);
	tItem = xuiCascaderGetItemRect(pCascader, iColumn, iItem);
	iRet = xuiInputPointerDown(pDemo->pContext, tWorld.fX + tItem.fX + tItem.fW * 0.5f, tWorld.fY + tItem.fY + tItem.fH * 0.5f,
		XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet != XUI_OK ) return iRet;
	return xuiDispatchPendingEvents(pDemo->pContext);
}

static int __xuiCascaderClickOwnerRect(xui_cascader_demo_t* pDemo, xui_widget pCascader, xui_rect_t tLocal)
{
	xui_rect_t tWorld;
	int iRet;

	tWorld = xuiWidgetGetWorldRect(pCascader);
	iRet = xuiInputPointerDown(pDemo->pContext, tWorld.fX + tLocal.fX + tLocal.fW * 0.5f, tWorld.fY + tLocal.fY + tLocal.fH * 0.5f,
		XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet != XUI_OK ) return iRet;
	return xuiDispatchPendingEvents(pDemo->pContext);
}

static void __xuiCascaderRunChecks(xui_cascader_demo_t* pDemo, int bExerciseInput)
{
	xui_rect_t tClear;
	xui_rect_t tPopup;
	int iBefore;

	pDemo->bCreateOK = (pDemo->pRoot != NULL) && (pDemo->pCascader[0] != NULL) && (xuiCascaderGetPanelWidget(pDemo->pCascader[0]) != NULL);
	if ( bExerciseInput && !pDemo->bExerciseDone ) {
		(void)xuiCascaderOpen(pDemo->pCascader[0]);
		(void)xuiLayout(pDemo->pContext);
		pDemo->bOpenOK = xuiCascaderIsOpen(pDemo->pCascader[0]) && (xuiCascaderGetColumnCount(pDemo->pCascader[0]) == 3);
		(void)__xuiCascaderClickItem(pDemo, pDemo->pCascader[0], 2, 6);
		pDemo->bSelectOK = (xuiCascaderGetSelectedLeaf(pDemo->pCascader[0]) == 6) && (pDemo->iLastLeaf == 6);

		iBefore = pDemo->iChangeCount;
		tClear = xuiCascaderGetClearRect(pDemo->pCascader[0]);
		(void)__xuiCascaderClickOwnerRect(pDemo, pDemo->pCascader[0], tClear);
		pDemo->bClearOK = (pDemo->iChangeCount == iBefore + 1) && (xuiCascaderGetSelectedDepth(pDemo->pCascader[0]) == 0);

		(void)xuiSetFocusWidget(pDemo->pContext, pDemo->pCascader[2]);
		(void)xuiInputKeyDown(pDemo->pContext, XUI_KEY_DOWN, 0);
		(void)xuiDispatchPendingEvents(pDemo->pContext);
		pDemo->bKeyOK = xuiCascaderIsOpen(pDemo->pCascader[2]);
		tPopup = xuiPopupGetPopupRect(xuiCascaderGetPopupWidget(pDemo->pCascader[2]));
		pDemo->bPlacementOK = (tPopup.fY < xuiWidgetGetWorldRect(pDemo->pCascader[2]).fY);
		pDemo->bExerciseDone = 1;
	} else if ( !bExerciseInput ) {
		pDemo->bOpenOK = 1;
		pDemo->bSelectOK = 1;
		pDemo->bClearOK = 1;
		pDemo->bKeyOK = 1;
		pDemo->bPlacementOK = 1;
	}
	pDemo->bLayoutOK = (xuiWidgetGetWorldRect(pDemo->pCascader[0]).fW > 100.0f) && (xuiCascaderGetButtonRect(pDemo->pCascader[0]).fW > 20.0f);
}

static int __xuiCascaderCreateAssets(xui_cascader_demo_t* pDemo)
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
	sFontPath = __xuiCascaderFindTtf();
	if ( sFontPath == NULL ) return XUI_ERROR_FILE_NOT_FOUND;
	iRet = pDemo->tProxy.fontLoadFile(&pDemo->tProxy, &pDemo->pFont, sFontPath, 14.0f, XUI_FONT_FORMAT_TTF);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiSetDefaultFont(pDemo->pContext, pDemo->pFont);
	return __xuiCascaderCreateUi(pDemo);
}

static void __xuiCascaderDestroyAssets(xui_cascader_demo_t* pDemo)
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

static int __xuiCascaderFrame(void* pUser)
{
	xui_cascader_demo_t* pDemo;
	xui_rect_i_t tFullRect;
	xui_rect_t tSrc;
	xui_rect_t tDst;
	xui_render_stats_t tStats;
	int iRet;
	int bAutoRun;

	pDemo = (xui_cascader_demo_t*)pUser;
	if ( pDemo == NULL ) return XGE_ERROR_INVALID_ARGUMENT;
	bAutoRun = (pDemo->iMaxFrames > 0) || (pDemo->fMaxSeconds > 0.0);
	iRet = xgeBegin();
	if ( iRet != XGE_OK ) return iRet;
	iRet = __xuiCascaderHandleInput(pDemo);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiDispatchPendingEvents(pDemo->pContext);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiLayout(pDemo->pContext);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiUpdate(pDemo->pContext, xgeGetDelta());
	if ( iRet != XUI_OK ) return iRet;
	__xuiCascaderRunChecks(pDemo, bAutoRun);
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
		printf("xui_cascader final-summary frames=%d create=%d layout=%d open=%d select=%d clear=%d key=%d placement=%d leaf=%d depth=%d updatedCaches=%d drawnCaches=%d\n",
			pDemo->iFrame, pDemo->bCreateOK, pDemo->bLayoutOK, pDemo->bOpenOK, pDemo->bSelectOK, pDemo->bClearOK,
			pDemo->bKeyOK, pDemo->bPlacementOK, pDemo->iLastLeaf, pDemo->iLastDepth, tStats.iUpdatedCaches, tStats.iDrawnCaches);
		xgeQuit();
	}
	return XGE_OK;
}

int main(int argc, char** argv)
{
	xui_cascader_demo_t tDemo;
	xge_desc_t tDesc;
	int iRet;

	memset(&tDemo, 0, sizeof(tDemo));
	iRet = __xuiCascaderParseArgs(&tDemo, argc, argv);
	if ( iRet == 1 ) return 0;
	if ( iRet != XGE_OK ) {
		__xuiCascaderUsage();
		return 1;
	}
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iWidth = DEMO_TARGET_W + 20;
	tDesc.iHeight = DEMO_TARGET_H + 50;
	tDesc.sTitle = "XUI Cascader";
	tDesc.iFlags = XGE_INIT_WINDOW | XGE_INIT_VSYNC;
	tDesc.iRunMode = XGE_RUN_GAME_LOOP;
	tDesc.iTargetFPS = 60;
	iRet = xgeInit(&tDesc);
	if ( iRet != XGE_OK ) {
		printf("xui_cascader: xgeInit failed: %d\n", iRet);
		return 1;
	}
	iRet = __xuiCascaderCreateAssets(&tDemo);
	if ( iRet != XUI_OK ) {
		printf("xui_cascader: create assets failed: %d\n", iRet);
		__xuiCascaderDestroyAssets(&tDemo);
		xgeUnit();
		return 1;
	}
	iRet = xgeRun(__xuiCascaderFrame, &tDemo);
	__xuiCascaderDestroyAssets(&tDemo);
	xgeUnit();
	return (iRet == XGE_OK && tDemo.bCreateOK && tDemo.bLayoutOK && tDemo.bOpenOK && tDemo.bSelectOK &&
		tDemo.bClearOK && tDemo.bKeyOK && tDemo.bPlacementOK) ? 0 : 1;
}
