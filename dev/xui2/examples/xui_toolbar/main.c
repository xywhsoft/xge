#include "xui.h"
#include "xge.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEMO_TARGET_W 640
#define DEMO_TARGET_H 360
#define DEMO_OFFSET_X 10.0f
#define DEMO_OFFSET_Y 20.0f

typedef struct xui_toolbar_demo_t {
	xui_context pContext;
	xui_proxy_t tProxy;
	xui_surface pTarget;
	xui_font pFont;
	xui_widget pRoot;
	xui_widget pToolbar;
	xui_widget pVerticalToolbar;
	xui_widget pStatus;
	xui_widget pHint;
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
	int bToggleOK;
	int bDisabledOK;
	int bKeyboardOK;
	int bOverflowOK;
	int bVerticalOK;
	int iSelectCount;
	int iOverflowCount;
	int iLastIndex;
	int iLastValue;
	int iOverflowFirst;
	int iOverflowItems;
} xui_toolbar_demo_t;

static void __xuiToolbarUsage(void)
{
	printf("usage: xui_toolbar [--frames N] [--seconds N]\n");
	printf("       no duration option means run until the window is closed.\n");
}

static int __xuiToolbarParseArgs(xui_toolbar_demo_t* pDemo, int argc, char** argv)
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
			__xuiToolbarUsage();
			return 1;
		} else {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
	}
	return XGE_OK;
}

static const char* __xuiToolbarFindTtf(void)
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

static int __xuiToolbarRootRender(xui_widget pWidget, xui_draw_context pDraw, uint32_t iStateId, void* pUser)
{
	xui_toolbar_demo_t* pDemo;
	xui_rect_t tRect;
	xui_rect_t tPanel;

	(void)iStateId;
	pDemo = (xui_toolbar_demo_t*)pUser;
	if ( (pWidget == NULL) || (pDraw == NULL) || (pDemo == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	tRect = xuiWidgetGetContentRect(pWidget);
	if ( pDemo->tProxy.drawRectFill != NULL ) {
		(void)pDemo->tProxy.drawRectFill(&pDemo->tProxy, pDraw, tRect, XUI_COLOR_RGBA(232, 240, 249, 255));
	}
	if ( pDemo->tProxy.drawRectFill != NULL ) {
		tPanel = (xui_rect_t){24.0f, 34.0f, tRect.fW - 48.0f, 260.0f};
		(void)pDemo->tProxy.drawRectFill(&pDemo->tProxy, pDraw, tPanel, XUI_COLOR_RGBA(248, 251, 255, 255));
	}
	return XUI_OK;
}

static void __xuiToolbarSelected(xui_widget pToolbar, int iIndex, int iValue, void* pUser)
{
	xui_toolbar_demo_t* pDemo;
	char sText[128];

	(void)pToolbar;
	pDemo = (xui_toolbar_demo_t*)pUser;
	if ( pDemo == NULL ) return;
	pDemo->iSelectCount++;
	pDemo->iLastIndex = iIndex;
	pDemo->iLastValue = iValue;
	snprintf(sText, sizeof(sText), "Selected index=%d value=%d", iIndex, iValue);
	if ( pDemo->pStatus != NULL ) {
		(void)xuiLabelSetText(pDemo->pStatus, sText);
	}
}

static void __xuiToolbarOverflow(xui_widget pToolbar, int iFirst, int iCount, void* pUser)
{
	xui_toolbar_demo_t* pDemo;
	char sText[128];

	(void)pToolbar;
	pDemo = (xui_toolbar_demo_t*)pUser;
	if ( pDemo == NULL ) return;
	pDemo->iOverflowCount++;
	pDemo->iOverflowFirst = iFirst;
	pDemo->iOverflowItems = iCount;
	snprintf(sText, sizeof(sText), "Overflow first=%d count=%d", iFirst, iCount);
	if ( pDemo->pStatus != NULL ) {
		(void)xuiLabelSetText(pDemo->pStatus, sText);
	}
}

static int __xuiToolbarAddLabel(xui_toolbar_demo_t* pDemo, xui_widget* ppWidget, const char* sText, xui_rect_t tRect, uint32_t iColor)
{
	xui_label_desc_t tDesc;
	int iRet;

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.sText = sText;
	tDesc.pFont = pDemo->pFont;
	tDesc.iTextColor = iColor;
	tDesc.iTextFlags = XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP;
	iRet = xuiLabelCreate(pDemo->pContext, ppWidget, &tDesc);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiWidgetSetRect(*ppWidget, tRect);
	return xuiWidgetAddChild(pDemo->pRoot, *ppWidget);
}

static void __xuiToolbarFillItems(xui_toolbar_item_t* pItems, int bVertical)
{
	memset(pItems, 0, sizeof(xui_toolbar_item_t) * 7);
	pItems[0] = (xui_toolbar_item_t){"New", "Create scene", XUI_TOOLBAR_ITEM_BUTTON, XUI_TOOLBAR_ITEM_ENABLED, 10, 0, NULL, {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, NULL};
	pItems[1] = (xui_toolbar_item_t){"Pin", "Toggle pin", XUI_TOOLBAR_ITEM_TOGGLE, XUI_TOOLBAR_ITEM_ENABLED, 11, 0, NULL, {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, NULL};
	pItems[2] = (xui_toolbar_item_t){"", "", XUI_TOOLBAR_ITEM_SEPARATOR, 0, 0, 0, NULL, {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, NULL};
	pItems[3] = (xui_toolbar_item_t){"Save", "Disabled", XUI_TOOLBAR_ITEM_BUTTON, 0, 12, 1, NULL, {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, NULL};
	pItems[4] = (xui_toolbar_item_t){"Run", "Run command", XUI_TOOLBAR_ITEM_BUTTON, XUI_TOOLBAR_ITEM_ENABLED, 13, 1, NULL, {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, NULL};
	pItems[5] = (xui_toolbar_item_t){bVertical ? "Cut" : "Preview", bVertical ? "Cut" : "Preview", XUI_TOOLBAR_ITEM_BUTTON, XUI_TOOLBAR_ITEM_ENABLED, 14, 2, NULL, {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, NULL};
	pItems[6] = (xui_toolbar_item_t){bVertical ? "Copy" : "Export", bVertical ? "Copy" : "Export", XUI_TOOLBAR_ITEM_BUTTON, XUI_TOOLBAR_ITEM_ENABLED, 15, 2, NULL, {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, NULL};
}

static int __xuiToolbarCreateUi(xui_toolbar_demo_t* pDemo)
{
	xui_toolbar_desc_t tDesc;
	xui_toolbar_metrics_t tMetrics;
	xui_toolbar_colors_t tColors;
	xui_toolbar_item_t arrItems[7];
	int iRet;

	iRet = xuiWidgetCreate(pDemo->pContext, &pDemo->pRoot);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiWidgetSetLayoutType(pDemo->pRoot, XUI_LAYOUT_MANUAL);
	(void)xuiWidgetSetCacheRenderCallback(pDemo->pRoot, __xuiToolbarRootRender, pDemo);
	iRet = xuiSetRootWidget(pDemo->pContext, pDemo->pRoot);
	if ( iRet != XUI_OK ) return iRet;

	memset(&tMetrics, 0, sizeof(tMetrics));
	tMetrics.iSize = sizeof(tMetrics);
	tMetrics.iOrientation = XUI_ORIENTATION_HORIZONTAL;
	tMetrics.fItemWidth = 64.0f;
	tMetrics.fItemHeight = 26.0f;
	tMetrics.fSeparatorSize = 10.0f;
	tMetrics.fGroupGap = 8.0f;
	tMetrics.fPaddingX = 4.0f;
	tMetrics.fPaddingY = 3.0f;
	tMetrics.fOverflowSize = 28.0f;
	tMetrics.fBorderWidth = 1.0f;
	tMetrics.fIconSize = 0.0f;
	tMetrics.fIconGap = 4.0f;

	__xuiToolbarFillItems(arrItems, 0);
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.pItems = arrItems;
	tDesc.iItemCount = 7;
	tDesc.pFont = pDemo->pFont;
	tDesc.tMetrics = tMetrics;
	tDesc.bHasMetrics = 1;
	tDesc.bOverflowEnabled = 1;
	iRet = xuiToolbarCreate(pDemo->pContext, &pDemo->pToolbar, &tDesc);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiWidgetSetRect(pDemo->pToolbar, (xui_rect_t){58.0f, 76.0f, 360.0f, 34.0f});
	iRet = xuiWidgetAddChild(pDemo->pRoot, pDemo->pToolbar);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiToolbarSetSelect(pDemo->pToolbar, __xuiToolbarSelected, pDemo);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiToolbarSetOverflow(pDemo->pToolbar, 1, 28.0f, __xuiToolbarOverflow, pDemo);
	if ( iRet != XUI_OK ) return iRet;

	tMetrics.iOrientation = XUI_ORIENTATION_VERTICAL;
	tMetrics.fItemWidth = 58.0f;
	tMetrics.fItemHeight = 28.0f;
	tMetrics.fOverflowSize = 28.0f;
	__xuiToolbarFillItems(arrItems, 1);
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.pItems = arrItems;
	tDesc.iItemCount = 7;
	tDesc.pFont = pDemo->pFont;
	tDesc.tMetrics = tMetrics;
	tDesc.bHasMetrics = 1;
	iRet = xuiToolbarCreate(pDemo->pContext, &pDemo->pVerticalToolbar, &tDesc);
	if ( iRet != XUI_OK ) return iRet;
	memset(&tColors, 0, sizeof(tColors));
	iRet = xuiToolbarGetColors(pDemo->pVerticalToolbar, &tColors);
	if ( iRet != XUI_OK ) return iRet;
	tColors.iBackgroundColor = XUI_COLOR_RGBA(243, 251, 248, 255);
	tColors.iHoverColor = XUI_COLOR_RGBA(214, 243, 232, 255);
	tColors.iActiveColor = XUI_COLOR_RGBA(42, 166, 124, 255);
	tColors.iCheckedColor = XUI_COLOR_RGBA(199, 237, 222, 255);
	tColors.iFocusColor = XUI_COLOR_RGBA(42, 166, 124, 255);
	iRet = xuiToolbarSetColors(pDemo->pVerticalToolbar, &tColors);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiWidgetSetRect(pDemo->pVerticalToolbar, (xui_rect_t){456.0f, 76.0f, 74.0f, 212.0f});
	iRet = xuiWidgetAddChild(pDemo->pRoot, pDemo->pVerticalToolbar);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiToolbarSetSelect(pDemo->pVerticalToolbar, __xuiToolbarSelected, pDemo);
	if ( iRet != XUI_OK ) return iRet;

	iRet = __xuiToolbarAddLabel(pDemo, &pDemo->pStatus, "Selected index=none",
		(xui_rect_t){58.0f, 146.0f, 340.0f, 28.0f}, XUI_COLOR_RGBA(42, 62, 88, 255));
	if ( iRet != XUI_OK ) return iRet;
	return __xuiToolbarAddLabel(pDemo, &pDemo->pHint, "Use mouse, Space/Enter, arrow keys, and the overflow button.",
		(xui_rect_t){58.0f, 186.0f, 420.0f, 28.0f}, XUI_COLOR_RGBA(84, 102, 124, 255));
}

static uint32_t __xuiToolbarReadButtons(void)
{
	uint32_t iButtons = 0;
	if ( xgeMouseDown(XGE_MOUSE_LEFT) ) iButtons |= XUI_POINTER_BUTTON_LEFT;
	if ( xgeMouseDown(XGE_MOUSE_RIGHT) ) iButtons |= XUI_POINTER_BUTTON_RIGHT;
	if ( xgeMouseDown(XGE_MOUSE_MIDDLE) ) iButtons |= XUI_POINTER_BUTTON_MIDDLE;
	return iButtons;
}

static int __xuiToolbarSendButtonTransitions(xui_toolbar_demo_t* pDemo, float fX, float fY, uint32_t iButtons, uint32_t iPressed, uint32_t iReleased)
{
	int iRet;

	if ( (iPressed & XUI_POINTER_BUTTON_LEFT) != 0 ) {
		iRet = xuiInputPointerDown(pDemo->pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, iButtons);
		if ( iRet != XUI_OK ) return iRet;
	}
	if ( (iReleased & XUI_POINTER_BUTTON_LEFT) != 0 ) {
		iRet = xuiInputPointerUp(pDemo->pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, iButtons);
		if ( iRet != XUI_OK ) return iRet;
	}
	return XUI_OK;
}

static int __xuiToolbarSendKey(xui_toolbar_demo_t* pDemo, int iXgeKey, int iXuiKey)
{
	if ( xgeKeyPressed(iXgeKey) ) {
		return xuiInputKeyDown(pDemo->pContext, iXuiKey, 0);
	}
	return XUI_OK;
}

static int __xuiToolbarHandleInput(xui_toolbar_demo_t* pDemo)
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
		xgeQuit();
	}
	xgeMouseGet(&fX, &fY);
	xgeMouseGetWheel(&fWheelX, &fWheelY);
	pDemo->fUiMouseX = fX - DEMO_OFFSET_X;
	pDemo->fUiMouseY = fY - DEMO_OFFSET_Y;
	iButtons = __xuiToolbarReadButtons();
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
	iRet = __xuiToolbarSendButtonTransitions(pDemo, pDemo->fUiMouseX, pDemo->fUiMouseY, iButtons, iPressed, iReleased);
	if ( iRet == XUI_OK ) iRet = __xuiToolbarSendKey(pDemo, XGE_KEY_LEFT, XUI_KEY_LEFT);
	if ( iRet == XUI_OK ) iRet = __xuiToolbarSendKey(pDemo, XGE_KEY_RIGHT, XUI_KEY_RIGHT);
	if ( iRet == XUI_OK ) iRet = __xuiToolbarSendKey(pDemo, XGE_KEY_UP, XUI_KEY_UP);
	if ( iRet == XUI_OK ) iRet = __xuiToolbarSendKey(pDemo, XGE_KEY_DOWN, XUI_KEY_DOWN);
	if ( iRet == XUI_OK ) iRet = __xuiToolbarSendKey(pDemo, XGE_KEY_ENTER, XUI_KEY_ENTER);
	if ( iRet == XUI_OK ) iRet = __xuiToolbarSendKey(pDemo, XGE_KEY_SPACE, XUI_KEY_SPACE);
	if ( iRet != XUI_OK ) return iRet;
	pDemo->bHasMouse = 1;
	pDemo->fLastMouseX = fX;
	pDemo->fLastMouseY = fY;
	pDemo->iLastButtons = iButtons;
	return XUI_OK;
}

static int __xuiToolbarDispatchMove(xui_toolbar_demo_t* pDemo, float fX, float fY)
{
	int iRet = xuiInputPointerMove(pDemo->pContext, fX, fY, 0);
	if ( iRet != XUI_OK ) return iRet;
	return xuiDispatchPendingEvents(pDemo->pContext);
}

static int __xuiToolbarDispatchClick(xui_toolbar_demo_t* pDemo, float fX, float fY)
{
	int iRet;

	iRet = __xuiToolbarDispatchMove(pDemo, fX, fY);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiInputPointerDown(pDemo->pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiDispatchPendingEvents(pDemo->pContext);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiInputPointerUp(pDemo->pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, 0);
	if ( iRet != XUI_OK ) return iRet;
	return xuiDispatchPendingEvents(pDemo->pContext);
}

static int __xuiToolbarClickItem(xui_toolbar_demo_t* pDemo, int iIndex)
{
	xui_rect_t tWorld = xuiWidgetGetWorldRect(pDemo->pToolbar);
	xui_rect_t tItem = xuiToolbarGetItemRect(pDemo->pToolbar, iIndex);
	return __xuiToolbarDispatchClick(pDemo, tWorld.fX + tItem.fX + tItem.fW * 0.5f, tWorld.fY + tItem.fY + tItem.fH * 0.5f);
}

static int __xuiToolbarClickOverflow(xui_toolbar_demo_t* pDemo)
{
	xui_rect_t tWorld = xuiWidgetGetWorldRect(pDemo->pToolbar);
	xui_rect_t tOverflow = xuiToolbarGetOverflowRect(pDemo->pToolbar);
	return __xuiToolbarDispatchClick(pDemo, tWorld.fX + tOverflow.fX + tOverflow.fW * 0.5f, tWorld.fY + tOverflow.fY + tOverflow.fH * 0.5f);
}

static void __xuiToolbarRunChecks(xui_toolbar_demo_t* pDemo, int bExerciseInput)
{
	int iBefore;
	xui_rect_t tV0;
	xui_rect_t tV1;

	pDemo->bCreateOK = (pDemo->pRoot != NULL) && (pDemo->pToolbar != NULL) && (pDemo->pVerticalToolbar != NULL) && (xuiToolbarGetItemCount(pDemo->pToolbar) == 7);
	pDemo->bLayoutOK = (xuiToolbarGetItemRect(pDemo->pToolbar, 1).fX > xuiToolbarGetItemRect(pDemo->pToolbar, 0).fX) && (xuiToolbarGetOverflowCount(pDemo->pToolbar) > 0);
	tV0 = xuiToolbarGetItemRect(pDemo->pVerticalToolbar, 0);
	tV1 = xuiToolbarGetItemRect(pDemo->pVerticalToolbar, 1);
	pDemo->bVerticalOK = (tV1.fY > tV0.fY) && (tV0.fH > 0.0f);
	if ( bExerciseInput && !pDemo->bExerciseDone && pDemo->bCreateOK && pDemo->bLayoutOK ) {
		iBefore = pDemo->iSelectCount;
		(void)__xuiToolbarClickItem(pDemo, 1);
		pDemo->bToggleOK = (pDemo->iSelectCount == iBefore + 1) && xuiToolbarGetItemChecked(pDemo->pToolbar, 1);
		iBefore = pDemo->iSelectCount;
		(void)__xuiToolbarClickItem(pDemo, 3);
		pDemo->bDisabledOK = (pDemo->iSelectCount == iBefore);
		(void)xuiSetFocusWidget(pDemo->pContext, pDemo->pToolbar);
		(void)xuiToolbarSetHoverIndex(pDemo->pToolbar, 0);
		iBefore = pDemo->iSelectCount;
		(void)xuiInputKeyDown(pDemo->pContext, XUI_KEY_SPACE, 0);
		(void)xuiDispatchPendingEvents(pDemo->pContext);
		pDemo->bKeyboardOK = (pDemo->iSelectCount == iBefore + 1) && (pDemo->iLastValue == 10);
		iBefore = pDemo->iOverflowCount;
		(void)__xuiToolbarClickOverflow(pDemo);
		pDemo->bOverflowOK = (pDemo->iOverflowCount == iBefore + 1) && (pDemo->iOverflowFirst >= 0) && (pDemo->iOverflowItems > 0);
		pDemo->bExerciseDone = 1;
	} else if ( !bExerciseInput ) {
		pDemo->bToggleOK = 1;
		pDemo->bDisabledOK = 1;
		pDemo->bKeyboardOK = 1;
		pDemo->bOverflowOK = 1;
	}
}

static int __xuiToolbarCreateAssets(xui_toolbar_demo_t* pDemo)
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
	tSurfaceDesc.iFormat = XUI_SURFACE_FORMAT_RGBA8;
	tSurfaceDesc.iWidth = DEMO_TARGET_W;
	tSurfaceDesc.iHeight = DEMO_TARGET_H;
	tSurfaceDesc.iFlags = XUI_SURFACE_ALPHA_PREMULTIPLIED | XUI_SURFACE_USAGE_TARGET;
	iRet = pDemo->tProxy.surfaceCreate(&pDemo->tProxy, &pDemo->pTarget, &tSurfaceDesc);
	if ( iRet != XUI_OK ) return iRet;
	sFontPath = __xuiToolbarFindTtf();
	if ( sFontPath == NULL ) return XUI_ERROR_FILE_NOT_FOUND;
	iRet = pDemo->tProxy.fontLoadFile(&pDemo->tProxy, &pDemo->pFont, sFontPath, 14.0f, XUI_FONT_FORMAT_TTF);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiSetDefaultFont(pDemo->pContext, pDemo->pFont);
	return __xuiToolbarCreateUi(pDemo);
}

static void __xuiToolbarDestroyAssets(xui_toolbar_demo_t* pDemo)
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

static int __xuiToolbarFrame(void* pUser)
{
	xui_toolbar_demo_t* pDemo;
	xui_render_stats_t tStats;
	xui_cache_stats_t tCacheStats;
	xui_rect_i_t tFullRect;
	xui_rect_t tSrc;
	xui_rect_t tDst;
	int iRet;
	int bAutoRun;

	pDemo = (xui_toolbar_demo_t*)pUser;
	if ( pDemo == NULL ) return XGE_ERROR_INVALID_ARGUMENT;
	bAutoRun = (pDemo->iMaxFrames > 0) || (pDemo->fMaxSeconds > 0.0);
	iRet = xgeBegin();
	if ( iRet != XGE_OK ) return iRet;
	iRet = __xuiToolbarHandleInput(pDemo);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiDispatchPendingEvents(pDemo->pContext);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiWidgetSetRect(pDemo->pRoot, (xui_rect_t){0.0f, 0.0f, (float)DEMO_TARGET_W, (float)DEMO_TARGET_H});
	iRet = xuiLayout(pDemo->pContext);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiUpdate(pDemo->pContext, xgeGetDelta());
	if ( iRet != XUI_OK ) return iRet;
	__xuiToolbarRunChecks(pDemo, bAutoRun);
	iRet = pDemo->tProxy.surfaceClear(&pDemo->tProxy, pDemo->pTarget, XUI_COLOR_RGBA(232, 240, 249, 255));
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
		memset(&tCacheStats, 0, sizeof(tCacheStats));
		(void)xuiGetRenderStats(pDemo->pContext, &tStats);
		(void)xuiGetCacheStats(pDemo->pContext, &tCacheStats);
		printf("xui_toolbar final-summary frames=%d create=%d layout=%d toggle=%d disabled=%d keyboard=%d overflow=%d vertical=%d callbacks=%d last=%d of=%d first=%d count=%d updatedCaches=%d drawnCaches=%d cacheSurfaces=%d\n",
			pDemo->iFrame, pDemo->bCreateOK, pDemo->bLayoutOK, pDemo->bToggleOK, pDemo->bDisabledOK,
			pDemo->bKeyboardOK, pDemo->bOverflowOK, pDemo->bVerticalOK, pDemo->iSelectCount, pDemo->iLastValue,
			pDemo->iOverflowCount, pDemo->iOverflowFirst, pDemo->iOverflowItems,
			tStats.iUpdatedCaches, tStats.iDrawnCaches, tCacheStats.iSurfaceCount);
		xgeQuit();
	}
	return XGE_OK;
}

int main(int argc, char** argv)
{
	xui_toolbar_demo_t tDemo;
	xge_desc_t tDesc;
	int iRet;

	memset(&tDemo, 0, sizeof(tDemo));
	tDemo.iLastIndex = -1;
	tDemo.iLastValue = -1;
	tDemo.iOverflowFirst = -1;
	iRet = __xuiToolbarParseArgs(&tDemo, argc, argv);
	if ( iRet == 1 ) return 0;
	if ( iRet != XGE_OK ) {
		__xuiToolbarUsage();
		return 1;
	}
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iWidth = DEMO_TARGET_W + 20;
	tDesc.iHeight = DEMO_TARGET_H + 50;
	tDesc.sTitle = "XUI Toolbar";
	tDesc.iFlags = XGE_INIT_WINDOW | XGE_INIT_VSYNC;
	tDesc.iRunMode = XGE_RUN_GAME_LOOP;
	tDesc.iTargetFPS = 60;
	iRet = xgeInit(&tDesc);
	if ( iRet != XGE_OK ) {
		printf("xui_toolbar: xgeInit failed: %d\n", iRet);
		return 1;
	}
	iRet = __xuiToolbarCreateAssets(&tDemo);
	if ( iRet != XUI_OK ) {
		printf("xui_toolbar: create assets failed: %d\n", iRet);
		__xuiToolbarDestroyAssets(&tDemo);
		xgeUnit();
		return 1;
	}
	iRet = xgeRun(__xuiToolbarFrame, &tDemo);
	__xuiToolbarDestroyAssets(&tDemo);
	xgeUnit();
	return (iRet == XGE_OK && tDemo.bCreateOK && tDemo.bLayoutOK && tDemo.bToggleOK &&
		tDemo.bDisabledOK && tDemo.bKeyboardOK && tDemo.bOverflowOK && tDemo.bVerticalOK) ? 0 : 1;
}
