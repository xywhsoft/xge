#include "xui.h"
#include "xge.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEMO_TARGET_W	620
#define DEMO_TARGET_H	340
#define DEMO_OFFSET_X	10.0f
#define DEMO_OFFSET_Y	20.0f
#define COMBO_COUNT	4
#define LABEL_COUNT	6

typedef struct xui_combobox_demo_t {
	xui_context pContext;
	xui_proxy_t tProxy;
	xui_surface pTarget;
	xui_font pFont;
	xui_widget pRoot;
	xui_widget pLabel[LABEL_COUNT];
	xui_widget pCombo[COMBO_COUNT];
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
	int bSelectOK;
	int bDisabledOK;
	int bKeyOK;
	int bPlacementOK;
	int iSelectCount;
	int iLastIndex;
	int iLastValue;
} xui_combobox_demo_t;

static void __xuiComboBoxUsage(void)
{
	printf("usage: xui_combobox [--frames N] [--seconds N]\n");
	printf("       no duration option means run until the window is closed.\n");
}

static int __xuiComboBoxParseArgs(xui_combobox_demo_t* pDemo, int argc, char** argv)
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
			__xuiComboBoxUsage();
			return 1;
		} else {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
	}
	return XGE_OK;
}

static const char* __xuiComboBoxFindTtf(void)
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

static int __xuiComboBoxRootRender(xui_widget pWidget, xui_draw_context pDraw, uint32_t iStateId, void* pUser)
{
	xui_combobox_demo_t* pDemo;
	xui_rect_t tRect;
	xui_rect_t tPanel;

	(void)iStateId;
	pDemo = (xui_combobox_demo_t*)pUser;
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

static void __xuiComboBoxSelected(xui_widget pWidget, int iIndex, int iValue, void* pUser)
{
	xui_combobox_demo_t* pDemo;
	char sText[128];

	(void)pWidget;
	pDemo = (xui_combobox_demo_t*)pUser;
	if ( pDemo == NULL ) return;
	pDemo->iSelectCount++;
	pDemo->iLastIndex = iIndex;
	pDemo->iLastValue = iValue;
	snprintf(sText, sizeof(sText), "Selected index=%d value=%d", iIndex, iValue);
	if ( pDemo->pStatus != NULL ) {
		(void)xuiLabelSetText(pDemo->pStatus, sText);
	}
}

static int __xuiComboBoxAddLabel(xui_combobox_demo_t* pDemo, int iIndex, const char* sText, xui_rect_t tRect)
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

static int __xuiComboBoxCreateCombos(xui_combobox_demo_t* pDemo)
{
	static const char* arrQuality[] = {
		"Low",
		"Balanced",
		"High",
		"Ultra"
	};
	xui_combobox_item_t arrItems[6];
	xui_combobox_desc_t tDesc;
	int iRet;

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.arrItems = arrQuality;
	tDesc.iItemCount = 4;
	tDesc.iSelected = 1;
	tDesc.pFont = pDemo->pFont;
	iRet = xuiComboBoxCreate(pDemo->pContext, &pDemo->pCombo[0], &tDesc);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiComboBoxSetSelect(pDemo->pCombo[0], __xuiComboBoxSelected, pDemo);

	memset(arrItems, 0, sizeof(arrItems));
	arrItems[0].sText = "D3D11";
	arrItems[0].iValue = 110;
	arrItems[0].bEnabled = 1;
	arrItems[1].sText = "Vulkan disabled";
	arrItems[1].iValue = 120;
	arrItems[1].bEnabled = 0;
	arrItems[2].bSeparator = 1;
	arrItems[3].sText = "OpenGL";
	arrItems[3].iValue = 130;
	arrItems[3].bEnabled = 1;
	arrItems[4].sText = "Software";
	arrItems[4].iValue = 140;
	arrItems[4].bEnabled = 1;
	arrItems[5].sText = "Remote";
	arrItems[5].iValue = 150;
	arrItems[5].bEnabled = 1;
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.arrItemData = arrItems;
	tDesc.iItemCount = 6;
	tDesc.bUseValue = 1;
	tDesc.iSelectedValue = 130;
	tDesc.pFont = pDemo->pFont;
	tDesc.fPopupMaxHeight = 118.0f;
	iRet = xuiComboBoxCreate(pDemo->pContext, &pDemo->pCombo[1], &tDesc);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiComboBoxSetSelect(pDemo->pCombo[1], __xuiComboBoxSelected, pDemo);

	iRet = xuiComboBoxCreate(pDemo->pContext, &pDemo->pCombo[2], &tDesc);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiComboBoxSetSelect(pDemo->pCombo[2], __xuiComboBoxSelected, pDemo);
	(void)xuiComboBoxSetPopupPlacement(pDemo->pCombo[2], XUI_COMBOBOX_POPUP_TOP);
	(void)xuiComboBoxSetColors(pDemo->pCombo[2],
		XUI_COLOR_RGBA(24, 78, 66, 255),
		XUI_COLOR_RGBA(130, 158, 152, 210),
		XUI_COLOR_RGBA(244, 252, 249, 255),
		XUI_COLOR_RGBA(229, 247, 240, 255),
		XUI_COLOR_RGBA(214, 239, 230, 255),
		XUI_COLOR_RGBA(235, 240, 245, 255));
	(void)xuiComboBoxSetBorderColors(pDemo->pCombo[2],
		XUI_COLOR_RGBA(90, 176, 146, 255),
		XUI_COLOR_RGBA(54, 150, 115, 255),
		XUI_COLOR_RGBA(37, 128, 94, 255));
	(void)xuiComboBoxSetArrowColors(pDemo->pCombo[2],
		XUI_COLOR_RGBA(32, 132, 94, 255),
		XUI_COLOR_RGBA(130, 158, 152, 160));
	(void)xuiComboBoxSetPopupColors(pDemo->pCombo[2],
		XUI_COLOR_RGBA(249, 254, 252, 255),
		XUI_COLOR_RGBA(120, 190, 164, 255),
		XUI_COLOR_RGBA(35, 82, 66, 42),
		XUI_COLOR_RGBA(40, 146, 106, 255),
		XUI_COLOR_RGBA(26, 76, 62, 255),
		XUI_COLOR_RGBA(255, 255, 255, 255),
		XUI_COLOR_RGBA(132, 154, 148, 210),
		XUI_COLOR_RGBA(196, 224, 214, 255));

	iRet = xuiComboBoxCreate(pDemo->pContext, &pDemo->pCombo[3], &tDesc);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiWidgetSetEnabled(pDemo->pCombo[3], 0);
	return XUI_OK;
}

static int __xuiComboBoxCreateUi(xui_combobox_demo_t* pDemo)
{
	int iRet;

	iRet = xuiWidgetCreate(pDemo->pContext, &pDemo->pRoot);
	if ( iRet != XUI_OK ) return iRet;
	xuiWidgetSetRect(pDemo->pRoot, (xui_rect_t){0.0f, 0.0f, (float)DEMO_TARGET_W, (float)DEMO_TARGET_H});
	(void)xuiWidgetSetLayoutType(pDemo->pRoot, XUI_LAYOUT_MANUAL);
	(void)xuiWidgetSetCacheRenderCallback(pDemo->pRoot, __xuiComboBoxRootRender, pDemo);
	iRet = xuiSetRootWidget(pDemo->pContext, pDemo->pRoot);
	if ( iRet != XUI_OK ) return iRet;
	if ( __xuiComboBoxAddLabel(pDemo, 0, "Quality", (xui_rect_t){48.0f, 58.0f, 150.0f, 28.0f}) != XUI_OK ||
	     __xuiComboBoxAddLabel(pDemo, 1, "Renderer", (xui_rect_t){48.0f, 106.0f, 150.0f, 28.0f}) != XUI_OK ||
	     __xuiComboBoxAddLabel(pDemo, 2, "Top popup", (xui_rect_t){48.0f, 214.0f, 150.0f, 28.0f}) != XUI_OK ||
	     __xuiComboBoxAddLabel(pDemo, 3, "Disabled", (xui_rect_t){48.0f, 260.0f, 150.0f, 28.0f}) != XUI_OK ||
	     __xuiComboBoxAddLabel(pDemo, 4, "XUI ComboBox", (xui_rect_t){48.0f, 30.0f, 200.0f, 24.0f}) != XUI_OK ||
	     __xuiComboBoxAddLabel(pDemo, 5, "Selected index=none value=none", (xui_rect_t){310.0f, 58.0f, 260.0f, 28.0f}) != XUI_OK ) {
		return XUI_ERROR;
	}
	pDemo->pStatus = pDemo->pLabel[5];
	iRet = __xuiComboBoxCreateCombos(pDemo);
	if ( iRet != XUI_OK ) return iRet;
	xuiWidgetSetRect(pDemo->pCombo[0], (xui_rect_t){160.0f, 58.0f, 132.0f, 30.0f});
	xuiWidgetSetRect(pDemo->pCombo[1], (xui_rect_t){160.0f, 106.0f, 170.0f, 30.0f});
	xuiWidgetSetRect(pDemo->pCombo[2], (xui_rect_t){160.0f, 214.0f, 170.0f, 30.0f});
	xuiWidgetSetRect(pDemo->pCombo[3], (xui_rect_t){160.0f, 260.0f, 170.0f, 30.0f});
	iRet = xuiWidgetAddChild(pDemo->pRoot, pDemo->pCombo[0]);
	if ( iRet == XUI_OK ) iRet = xuiWidgetAddChild(pDemo->pRoot, pDemo->pCombo[1]);
	if ( iRet == XUI_OK ) iRet = xuiWidgetAddChild(pDemo->pRoot, pDemo->pCombo[2]);
	if ( iRet == XUI_OK ) iRet = xuiWidgetAddChild(pDemo->pRoot, pDemo->pCombo[3]);
	return iRet;
}

static uint32_t __xuiComboBoxReadButtons(void)
{
	uint32_t iButtons;

	iButtons = 0;
	if ( xgeMouseDown(XGE_MOUSE_LEFT) ) iButtons |= XUI_POINTER_BUTTON_LEFT;
	if ( xgeMouseDown(XGE_MOUSE_RIGHT) ) iButtons |= XUI_POINTER_BUTTON_RIGHT;
	if ( xgeMouseDown(XGE_MOUSE_MIDDLE) ) iButtons |= XUI_POINTER_BUTTON_MIDDLE;
	return iButtons;
}

static int __xuiComboBoxHandleInput(xui_combobox_demo_t* pDemo)
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
		if ( xuiComboBoxIsOpen(pDemo->pCombo[0]) || xuiComboBoxIsOpen(pDemo->pCombo[1]) || xuiComboBoxIsOpen(pDemo->pCombo[2]) ) {
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
	iButtons = __xuiComboBoxReadButtons();
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

static int __xuiComboBoxClickItem(xui_combobox_demo_t* pDemo, xui_widget pMenu, int iIndex)
{
	xui_rect_t tWorld;
	xui_rect_t tItem;
	int iRet;

	tWorld = xuiWidgetGetWorldRect(pMenu);
	tItem = xuiMenuGetItemRect(pMenu, iIndex);
	iRet = xuiInputPointerDown(pDemo->pContext, tWorld.fX + tItem.fX + tItem.fW * 0.5f, tWorld.fY + tItem.fY + tItem.fH * 0.5f,
		XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet != XUI_OK ) return iRet;
	return xuiDispatchPendingEvents(pDemo->pContext);
}

static void __xuiComboBoxRunChecks(xui_combobox_demo_t* pDemo, int bExerciseInput)
{
	xui_widget pMenu;
	xui_rect_t tPopup;
	int iBefore;

	pDemo->bCreateOK = (pDemo->pRoot != NULL) && (pDemo->pCombo[0] != NULL) && (xuiComboBoxGetMenuWidget(pDemo->pCombo[0]) != NULL);
	if ( bExerciseInput && !pDemo->bExerciseDone ) {
		(void)xuiComboBoxOpen(pDemo->pCombo[0]);
		(void)xuiLayout(pDemo->pContext);
		pMenu = xuiComboBoxGetMenuWidget(pDemo->pCombo[0]);
		(void)__xuiComboBoxClickItem(pDemo, pMenu, 2);
		pDemo->bSelectOK = (xuiComboBoxGetSelected(pDemo->pCombo[0]) == 2) && (pDemo->iLastValue == 2);

		(void)xuiComboBoxOpen(pDemo->pCombo[1]);
		(void)xuiLayout(pDemo->pContext);
		iBefore = xuiComboBoxGetSelected(pDemo->pCombo[1]);
		pMenu = xuiComboBoxGetMenuWidget(pDemo->pCombo[1]);
		(void)__xuiComboBoxClickItem(pDemo, pMenu, 1);
		pDemo->bDisabledOK = xuiComboBoxIsOpen(pDemo->pCombo[1]) && (xuiComboBoxGetSelected(pDemo->pCombo[1]) == iBefore);
		(void)xuiComboBoxClose(pDemo->pCombo[1]);

		(void)xuiSetFocusWidget(pDemo->pContext, pDemo->pCombo[2]);
		(void)xuiInputKeyDown(pDemo->pContext, XUI_KEY_DOWN, 0);
		(void)xuiDispatchPendingEvents(pDemo->pContext);
		pDemo->bKeyOK = xuiComboBoxIsOpen(pDemo->pCombo[2]);
		tPopup = xuiPopupGetPopupRect(xuiComboBoxGetPopupWidget(pDemo->pCombo[2]));
		pDemo->bPlacementOK = (tPopup.fY < xuiWidgetGetWorldRect(pDemo->pCombo[2]).fY);
		pDemo->bExerciseDone = 1;
	} else if ( !bExerciseInput ) {
		pDemo->bSelectOK = 1;
		pDemo->bDisabledOK = 1;
		pDemo->bKeyOK = 1;
		pDemo->bPlacementOK = 1;
	}
	pDemo->bLayoutOK = (xuiWidgetGetWorldRect(pDemo->pCombo[0]).fW > 100.0f) && (xuiComboBoxGetButtonRect(pDemo->pCombo[0]).fW > 20.0f);
}

static int __xuiComboBoxCreateAssets(xui_combobox_demo_t* pDemo)
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
	sFontPath = __xuiComboBoxFindTtf();
	if ( sFontPath == NULL ) return XUI_ERROR_FILE_NOT_FOUND;
	iRet = pDemo->tProxy.fontLoadFile(&pDemo->tProxy, &pDemo->pFont, sFontPath, 14.0f, XUI_FONT_FORMAT_TTF);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiSetDefaultFont(pDemo->pContext, pDemo->pFont);
	return __xuiComboBoxCreateUi(pDemo);
}

static void __xuiComboBoxDestroyAssets(xui_combobox_demo_t* pDemo)
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

static int __xuiComboBoxFrame(void* pUser)
{
	xui_combobox_demo_t* pDemo;
	xui_rect_i_t tFullRect;
	xui_rect_t tSrc;
	xui_rect_t tDst;
	xui_render_stats_t tStats;
	int iRet;
	int bAutoRun;

	pDemo = (xui_combobox_demo_t*)pUser;
	if ( pDemo == NULL ) return XGE_ERROR_INVALID_ARGUMENT;
	bAutoRun = (pDemo->iMaxFrames > 0) || (pDemo->fMaxSeconds > 0.0);
	iRet = xgeBegin();
	if ( iRet != XGE_OK ) return iRet;
	iRet = __xuiComboBoxHandleInput(pDemo);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiDispatchPendingEvents(pDemo->pContext);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiLayout(pDemo->pContext);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiUpdate(pDemo->pContext, xgeGetDelta());
	if ( iRet != XUI_OK ) return iRet;
	__xuiComboBoxRunChecks(pDemo, bAutoRun);
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
		printf("xui_combobox final-summary frames=%d create=%d layout=%d select=%d disabled=%d key=%d placement=%d selectedIndex=%d selectedValue=%d updatedCaches=%d drawnCaches=%d\n",
			pDemo->iFrame, pDemo->bCreateOK, pDemo->bLayoutOK, pDemo->bSelectOK, pDemo->bDisabledOK, pDemo->bKeyOK,
			pDemo->bPlacementOK, pDemo->iLastIndex, pDemo->iLastValue, tStats.iUpdatedCaches, tStats.iDrawnCaches);
		xgeQuit();
	}
	return XGE_OK;
}

int main(int argc, char** argv)
{
	xui_combobox_demo_t tDemo;
	xge_desc_t tDesc;
	int iRet;

	memset(&tDemo, 0, sizeof(tDemo));
	iRet = __xuiComboBoxParseArgs(&tDemo, argc, argv);
	if ( iRet == 1 ) return 0;
	if ( iRet != XGE_OK ) {
		__xuiComboBoxUsage();
		return 1;
	}
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iWidth = DEMO_TARGET_W + 20;
	tDesc.iHeight = DEMO_TARGET_H + 50;
	tDesc.sTitle = "XUI ComboBox";
	tDesc.iFlags = XGE_INIT_WINDOW | XGE_INIT_VSYNC;
	tDesc.iRunMode = XGE_RUN_GAME_LOOP;
	tDesc.iTargetFPS = 60;
	iRet = xgeInit(&tDesc);
	if ( iRet != XGE_OK ) {
		printf("xui_combobox: xgeInit failed: %d\n", iRet);
		return 1;
	}
	iRet = __xuiComboBoxCreateAssets(&tDemo);
	if ( iRet != XUI_OK ) {
		printf("xui_combobox: create assets failed: %d\n", iRet);
		__xuiComboBoxDestroyAssets(&tDemo);
		xgeUnit();
		return 1;
	}
	iRet = xgeRun(__xuiComboBoxFrame, &tDemo);
	__xuiComboBoxDestroyAssets(&tDemo);
	xgeUnit();
	return (iRet == XGE_OK && tDemo.bCreateOK && tDemo.bLayoutOK && tDemo.bSelectOK &&
		tDemo.bDisabledOK && tDemo.bKeyOK && tDemo.bPlacementOK) ? 0 : 1;
}
