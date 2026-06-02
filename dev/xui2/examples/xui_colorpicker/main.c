#include "xui.h"
#include "xge.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEMO_TARGET_W	680
#define DEMO_TARGET_H	440
#define DEMO_OFFSET_X	10.0f
#define DEMO_OFFSET_Y	20.0f
#define PICKER_COUNT	3
#define LABEL_COUNT	7

typedef struct xui_colorpicker_demo_t {
	xui_context pContext;
	xui_proxy_t tProxy;
	xui_surface pTarget;
	xui_font pFont;
	xui_widget pRoot;
	xui_widget pLabel[LABEL_COUNT];
	xui_widget pPicker[PICKER_COUNT];
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
	int bHexOK;
	int bPaletteOK;
	int bAlphaOK;
	int bDisabledOK;
	int iChangeCount;
	uint32_t iLastColor;
} xui_colorpicker_demo_t;

static void __xuiColorPickerUsage(void)
{
	printf("usage: xui_colorpicker [--frames N] [--seconds N]\n");
	printf("       no duration option means run until the window is closed.\n");
}

static int __xuiColorPickerParseArgs(xui_colorpicker_demo_t* pDemo, int argc, char** argv)
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
			__xuiColorPickerUsage();
			return 1;
		} else {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
	}
	return XGE_OK;
}

static const char* __xuiColorPickerFindTtf(void)
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

static int __xuiColorPickerRootRender(xui_widget pWidget, xui_draw_context pDraw, uint32_t iStateId, void* pUser)
{
	xui_colorpicker_demo_t* pDemo;
	xui_rect_t tRect;
	xui_rect_t tPanel;

	(void)iStateId;
	pDemo = (xui_colorpicker_demo_t*)pUser;
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

static void __xuiColorPickerChanged(xui_widget pWidget, uint32_t iColor, void* pUser)
{
	xui_colorpicker_demo_t* pDemo;
	char sText[160];

	(void)pWidget;
	pDemo = (xui_colorpicker_demo_t*)pUser;
	if ( pDemo == NULL ) return;
	pDemo->iChangeCount++;
	pDemo->iLastColor = iColor;
	snprintf(sText, sizeof(sText), "changes=%d color=#%02X%02X%02X%02X",
		pDemo->iChangeCount,
		(int)((iColor >> 24) & 0xffu),
		(int)((iColor >> 16) & 0xffu),
		(int)((iColor >> 8) & 0xffu),
		(int)(iColor & 0xffu));
	if ( pDemo->pStatus != NULL ) {
		(void)xuiLabelSetText(pDemo->pStatus, sText);
	}
}

static int __xuiColorPickerAddLabel(xui_colorpicker_demo_t* pDemo, int iIndex, const char* sText, xui_rect_t tRect)
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

static int __xuiColorPickerCreatePickers(xui_colorpicker_demo_t* pDemo)
{
	uint32_t arrPalette[11];
	xui_color_picker_desc_t tDesc;
	int iRet;
	int i;

	arrPalette[0] = XUI_COLOR_RGBA(0, 0, 0, 255);
	arrPalette[1] = XUI_COLOR_RGBA(255, 255, 255, 255);
	arrPalette[2] = XUI_COLOR_RGBA(148, 158, 168, 255);
	arrPalette[3] = XUI_COLOR_RGBA(225, 58, 70, 255);
	arrPalette[4] = XUI_COLOR_RGBA(230, 126, 34, 255);
	arrPalette[5] = XUI_COLOR_RGBA(244, 201, 54, 255);
	arrPalette[6] = XUI_COLOR_RGBA(74, 165, 91, 255);
	arrPalette[7] = XUI_COLOR_RGBA(43, 184, 203, 255);
	arrPalette[8] = XUI_COLOR_RGBA(46, 124, 214, 255);
	arrPalette[9] = XUI_COLOR_RGBA(132, 86, 209, 255);
	arrPalette[10] = XUI_COLOR_RGBA(132, 86, 209, 120);

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.iColor = XUI_COLOR_RGBA(46, 124, 214, 255);
	tDesc.arrPalette = arrPalette;
	tDesc.iPaletteCount = 10;
	tDesc.pFont = pDemo->pFont;
	iRet = xuiColorPickerCreate(pDemo->pContext, &pDemo->pPicker[0], &tDesc);
	if ( iRet != XUI_OK ) return iRet;

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.iColor = XUI_COLOR_RGBA(74, 165, 91, 160);
	tDesc.arrPalette = arrPalette;
	tDesc.iPaletteCount = 11;
	tDesc.bAlphaEnabled = 1;
	tDesc.pFont = pDemo->pFont;
	iRet = xuiColorPickerCreate(pDemo->pContext, &pDemo->pPicker[1], &tDesc);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiColorPickerSetColors(pDemo->pPicker[1],
		XUI_COLOR_RGBA(24, 78, 66, 255),
		XUI_COLOR_RGBA(130, 158, 152, 210),
		XUI_COLOR_RGBA(244, 252, 249, 255),
		XUI_COLOR_RGBA(229, 247, 240, 255),
		XUI_COLOR_RGBA(214, 239, 230, 255),
		XUI_COLOR_RGBA(235, 240, 245, 255));
	(void)xuiColorPickerSetBorderColors(pDemo->pPicker[1],
		XUI_COLOR_RGBA(90, 176, 146, 255),
		XUI_COLOR_RGBA(54, 150, 115, 255),
		XUI_COLOR_RGBA(37, 128, 94, 255));
	(void)xuiColorPickerSetPopupColors(pDemo->pPicker[1],
		XUI_COLOR_RGBA(249, 254, 252, 255),
		XUI_COLOR_RGBA(120, 190, 164, 255),
		XUI_COLOR_RGBA(35, 82, 66, 42),
		XUI_COLOR_RGBA(26, 76, 62, 255),
		XUI_COLOR_RGBA(86, 120, 108, 255),
		XUI_COLOR_RGBA(40, 146, 106, 255),
		XUI_COLOR_RGBA(245, 252, 249, 255),
		XUI_COLOR_RGBA(120, 190, 164, 255),
		XUI_COLOR_RGBA(196, 224, 214, 255));

	iRet = xuiColorPickerCreate(pDemo->pContext, &pDemo->pPicker[2], &tDesc);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiWidgetSetEnabled(pDemo->pPicker[2], 0);
	for ( i = 0; i < PICKER_COUNT; i++ ) {
		(void)xuiColorPickerSetChange(pDemo->pPicker[i], __xuiColorPickerChanged, pDemo);
	}
	return XUI_OK;
}

static int __xuiColorPickerCreateUi(xui_colorpicker_demo_t* pDemo)
{
	int iRet;

	iRet = xuiWidgetCreate(pDemo->pContext, &pDemo->pRoot);
	if ( iRet != XUI_OK ) return iRet;
	xuiWidgetSetRect(pDemo->pRoot, (xui_rect_t){0.0f, 0.0f, (float)DEMO_TARGET_W, (float)DEMO_TARGET_H});
	(void)xuiWidgetSetLayoutType(pDemo->pRoot, XUI_LAYOUT_MANUAL);
	(void)xuiWidgetSetCacheRenderCallback(pDemo->pRoot, __xuiColorPickerRootRender, pDemo);
	iRet = xuiSetRootWidget(pDemo->pContext, pDemo->pRoot);
	if ( iRet != XUI_OK ) return iRet;
	if ( __xuiColorPickerAddLabel(pDemo, 0, "XUI ColorPicker", (xui_rect_t){48.0f, 30.0f, 220.0f, 24.0f}) != XUI_OK ||
	     __xuiColorPickerAddLabel(pDemo, 1, "RGB", (xui_rect_t){48.0f, 72.0f, 150.0f, 28.0f}) != XUI_OK ||
	     __xuiColorPickerAddLabel(pDemo, 2, "RGBA", (xui_rect_t){48.0f, 120.0f, 150.0f, 28.0f}) != XUI_OK ||
	     __xuiColorPickerAddLabel(pDemo, 3, "Disabled", (xui_rect_t){48.0f, 168.0f, 150.0f, 28.0f}) != XUI_OK ||
	     __xuiColorPickerAddLabel(pDemo, 4, "Open popup and drag inside the color panel.", (xui_rect_t){48.0f, 224.0f, 320.0f, 28.0f}) != XUI_OK ||
	     __xuiColorPickerAddLabel(pDemo, 5, "Palette changes are applied immediately.", (xui_rect_t){48.0f, 252.0f, 320.0f, 28.0f}) != XUI_OK ||
	     __xuiColorPickerAddLabel(pDemo, 6, "changes=0 color=none", (xui_rect_t){360.0f, 72.0f, 270.0f, 28.0f}) != XUI_OK ) {
		return XUI_ERROR;
	}
	pDemo->pStatus = pDemo->pLabel[6];
	iRet = __xuiColorPickerCreatePickers(pDemo);
	if ( iRet != XUI_OK ) return iRet;
	xuiWidgetSetRect(pDemo->pPicker[0], (xui_rect_t){160.0f, 72.0f, 180.0f, 30.0f});
	xuiWidgetSetRect(pDemo->pPicker[1], (xui_rect_t){160.0f, 120.0f, 180.0f, 30.0f});
	xuiWidgetSetRect(pDemo->pPicker[2], (xui_rect_t){160.0f, 168.0f, 180.0f, 30.0f});
	iRet = xuiWidgetAddChild(pDemo->pRoot, pDemo->pPicker[0]);
	if ( iRet == XUI_OK ) iRet = xuiWidgetAddChild(pDemo->pRoot, pDemo->pPicker[1]);
	if ( iRet == XUI_OK ) iRet = xuiWidgetAddChild(pDemo->pRoot, pDemo->pPicker[2]);
	return iRet;
}

static uint32_t __xuiColorPickerReadButtons(void)
{
	uint32_t iButtons;

	iButtons = 0;
	if ( xgeMouseDown(XGE_MOUSE_LEFT) ) iButtons |= XUI_POINTER_BUTTON_LEFT;
	if ( xgeMouseDown(XGE_MOUSE_RIGHT) ) iButtons |= XUI_POINTER_BUTTON_RIGHT;
	if ( xgeMouseDown(XGE_MOUSE_MIDDLE) ) iButtons |= XUI_POINTER_BUTTON_MIDDLE;
	return iButtons;
}

static int __xuiColorPickerHandleInput(xui_colorpicker_demo_t* pDemo)
{
	float fX;
	float fY;
	float fWheelX;
	float fWheelY;
	uint32_t iButtons;
	uint32_t iPressed;
	uint32_t iReleased;
	uint32_t iText;
	int iRet;

	if ( xgeKeyPressed(XGE_KEY_ESCAPE) ) {
		if ( xuiColorPickerIsOpen(pDemo->pPicker[0]) || xuiColorPickerIsOpen(pDemo->pPicker[1]) ) {
			iRet = xuiInputKeyDown(pDemo->pContext, XUI_KEY_ESCAPE, 0);
			if ( iRet != XUI_OK ) return iRet;
		} else {
			xgeQuit();
		}
	}
	if ( xgeKeyPressed(XGE_KEY_ENTER) ) {
		iRet = xuiInputKeyDown(pDemo->pContext, XUI_KEY_ENTER, 0);
		if ( iRet != XUI_OK ) return iRet;
	}
	if ( xgeKeyPressed(XGE_KEY_BACKSPACE) ) {
		iRet = xuiInputKeyDown(pDemo->pContext, 8, 0);
		if ( iRet != XUI_OK ) return iRet;
	}
	if ( xgeKeyPressed(XGE_KEY_DELETE) ) {
		iRet = xuiInputKeyDown(pDemo->pContext, 46, 0);
		if ( iRet != XUI_OK ) return iRet;
	}
	while ( (iText = xgeTextGet()) != 0 ) {
		iRet = xuiInputText(pDemo->pContext, iText);
		if ( iRet != XUI_OK ) return iRet;
	}
	xgeMouseGet(&fX, &fY);
	xgeMouseGetWheel(&fWheelX, &fWheelY);
	pDemo->fUiMouseX = fX - DEMO_OFFSET_X;
	pDemo->fUiMouseY = fY - DEMO_OFFSET_Y;
	iButtons = __xuiColorPickerReadButtons();
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

static int __xuiColorPickerClickPanelRect(xui_colorpicker_demo_t* pDemo, xui_widget pPicker, xui_rect_t tRect)
{
	xui_widget pPanel;
	xui_rect_t tWorld;
	int iRet;

	pPanel = xuiColorPickerGetPanelWidget(pPicker);
	tWorld = xuiWidgetGetWorldRect(pPanel);
	iRet = xuiInputPointerDown(pDemo->pContext, tWorld.fX + tRect.fX + tRect.fW * 0.5f, tWorld.fY + tRect.fY + tRect.fH * 0.5f,
		XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet != XUI_OK ) return iRet;
	return xuiDispatchPendingEvents(pDemo->pContext);
}

static int __xuiColorPickerDragPanelRect(xui_colorpicker_demo_t* pDemo, xui_widget pPicker, xui_rect_t tRect, float fRate)
{
	xui_widget pPanel;
	xui_rect_t tWorld;
	float fY;
	int iRet;

	pPanel = xuiColorPickerGetPanelWidget(pPicker);
	tWorld = xuiWidgetGetWorldRect(pPanel);
	fY = tWorld.fY + tRect.fY + tRect.fH * 0.5f;
	iRet = xuiInputPointerDown(pDemo->pContext, tWorld.fX + tRect.fX + 2.0f, fY, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiDispatchPendingEvents(pDemo->pContext);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiInputPointerMove(pDemo->pContext, tWorld.fX + tRect.fX + tRect.fW * fRate, fY, XUI_POINTER_BUTTON_LEFT);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiDispatchPendingEvents(pDemo->pContext);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiInputPointerUp(pDemo->pContext, tWorld.fX + tRect.fX + tRect.fW * fRate, fY, XUI_POINTER_BUTTON_LEFT, 0);
	if ( iRet != XUI_OK ) return iRet;
	return xuiDispatchPendingEvents(pDemo->pContext);
}

static void __xuiColorPickerRunChecks(xui_colorpicker_demo_t* pDemo, int bExerciseInput)
{
	uint32_t iPaletteColor;
	uint32_t iAlphaColor;
	xui_rect_t tAlpha;

	pDemo->bCreateOK = (pDemo->pRoot != NULL) && (pDemo->pPicker[0] != NULL) && (xuiColorPickerGetPopupWidget(pDemo->pPicker[0]) != NULL);
	pDemo->bLayoutOK = (xuiWidgetGetWorldRect(pDemo->pPicker[0]).fW > 100.0f) && (xuiColorPickerGetButtonRect(pDemo->pPicker[0]).fW > 20.0f);
	if ( bExerciseInput && !pDemo->bExerciseDone ) {
		(void)xuiColorPickerSetHex(pDemo->pPicker[0], "#112233");
		(void)xuiColorPickerSetHex(pDemo->pPicker[1], "#11223380");
		pDemo->bHexOK = (strcmp(xuiColorPickerGetHex(pDemo->pPicker[0]), "#112233") == 0) &&
			(strcmp(xuiColorPickerGetHex(pDemo->pPicker[1]), "#11223380") == 0);

		(void)xuiColorPickerOpen(pDemo->pPicker[0]);
		(void)xuiLayout(pDemo->pContext);
		iPaletteColor = xuiColorPickerGetPaletteColor(pDemo->pPicker[0], 5);
		(void)__xuiColorPickerClickPanelRect(pDemo, pDemo->pPicker[0], xuiColorPickerGetPaletteRect(pDemo->pPicker[0], 5));
		pDemo->bPaletteOK = (xuiColorPickerGetColor(pDemo->pPicker[0]) == iPaletteColor) && (pDemo->iChangeCount > 0);
		(void)xuiColorPickerClose(pDemo->pPicker[0]);

		(void)xuiColorPickerOpen(pDemo->pPicker[1]);
		(void)xuiLayout(pDemo->pContext);
		tAlpha = xuiColorPickerGetAlphaRect(pDemo->pPicker[1]);
		(void)__xuiColorPickerDragPanelRect(pDemo, pDemo->pPicker[1], tAlpha, 0.35f);
		iAlphaColor = xuiColorPickerGetColor(pDemo->pPicker[1]);
		pDemo->bAlphaOK = ((iAlphaColor & 0xffu) >= 70u) && ((iAlphaColor & 0xffu) <= 105u);
		(void)xuiColorPickerClose(pDemo->pPicker[1]);

		(void)xuiColorPickerOpen(pDemo->pPicker[2]);
		pDemo->bDisabledOK = !xuiColorPickerIsOpen(pDemo->pPicker[2]);
		pDemo->bExerciseDone = 1;
	} else if ( !bExerciseInput ) {
		pDemo->bHexOK = 1;
		pDemo->bPaletteOK = 1;
		pDemo->bAlphaOK = 1;
		pDemo->bDisabledOK = 1;
	}
}

static int __xuiColorPickerCreateAssets(xui_colorpicker_demo_t* pDemo)
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
	sFontPath = __xuiColorPickerFindTtf();
	if ( sFontPath == NULL ) return XUI_ERROR_FILE_NOT_FOUND;
	iRet = pDemo->tProxy.fontLoadFile(&pDemo->tProxy, &pDemo->pFont, sFontPath, 14.0f, XUI_FONT_FORMAT_TTF);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiSetDefaultFont(pDemo->pContext, pDemo->pFont);
	return __xuiColorPickerCreateUi(pDemo);
}

static void __xuiColorPickerDestroyAssets(xui_colorpicker_demo_t* pDemo)
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

static int __xuiColorPickerFrame(void* pUser)
{
	xui_colorpicker_demo_t* pDemo;
	xui_rect_i_t tFullRect;
	xui_rect_t tSrc;
	xui_rect_t tDst;
	xui_render_stats_t tStats;
	int iRet;
	int bAutoRun;

	pDemo = (xui_colorpicker_demo_t*)pUser;
	if ( pDemo == NULL ) return XGE_ERROR_INVALID_ARGUMENT;
	bAutoRun = (pDemo->iMaxFrames > 0) || (pDemo->fMaxSeconds > 0.0);
	iRet = xgeBegin();
	if ( iRet != XGE_OK ) return iRet;
	iRet = __xuiColorPickerHandleInput(pDemo);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiDispatchPendingEvents(pDemo->pContext);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiLayout(pDemo->pContext);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiUpdate(pDemo->pContext, xgeGetDelta());
	if ( iRet != XUI_OK ) return iRet;
	__xuiColorPickerRunChecks(pDemo, bAutoRun);
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
		printf("xui_colorpicker final-summary frames=%d create=%d layout=%d hex=%d palette=%d alpha=%d disabled=%d changes=%d last=#%02X%02X%02X%02X updatedCaches=%d drawnCaches=%d\n",
			pDemo->iFrame, pDemo->bCreateOK, pDemo->bLayoutOK, pDemo->bHexOK, pDemo->bPaletteOK, pDemo->bAlphaOK,
			pDemo->bDisabledOK, pDemo->iChangeCount,
			(int)((pDemo->iLastColor >> 24) & 0xffu),
			(int)((pDemo->iLastColor >> 16) & 0xffu),
			(int)((pDemo->iLastColor >> 8) & 0xffu),
			(int)(pDemo->iLastColor & 0xffu),
			tStats.iUpdatedCaches, tStats.iDrawnCaches);
		xgeQuit();
	}
	return XGE_OK;
}

int main(int argc, char** argv)
{
	xui_colorpicker_demo_t tDemo;
	xge_desc_t tDesc;
	int iRet;

	memset(&tDemo, 0, sizeof(tDemo));
	iRet = __xuiColorPickerParseArgs(&tDemo, argc, argv);
	if ( iRet == 1 ) return 0;
	if ( iRet != XGE_OK ) {
		__xuiColorPickerUsage();
		return 1;
	}
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iWidth = DEMO_TARGET_W + 20;
	tDesc.iHeight = DEMO_TARGET_H + 50;
	tDesc.sTitle = "XUI ColorPicker";
	tDesc.iFlags = XGE_INIT_WINDOW | XGE_INIT_VSYNC;
	tDesc.iRunMode = XGE_RUN_GAME_LOOP;
	tDesc.iTargetFPS = 60;
	iRet = xgeInit(&tDesc);
	if ( iRet != XGE_OK ) {
		printf("xui_colorpicker: xgeInit failed: %d\n", iRet);
		return 1;
	}
	iRet = __xuiColorPickerCreateAssets(&tDemo);
	if ( iRet != XUI_OK ) {
		printf("xui_colorpicker: create assets failed: %d\n", iRet);
		__xuiColorPickerDestroyAssets(&tDemo);
		xgeUnit();
		return 1;
	}
	iRet = xgeRun(__xuiColorPickerFrame, &tDemo);
	__xuiColorPickerDestroyAssets(&tDemo);
	xgeUnit();
	return (iRet == XGE_OK && tDemo.bCreateOK && tDemo.bLayoutOK && tDemo.bHexOK &&
		tDemo.bPaletteOK && tDemo.bAlphaOK && tDemo.bDisabledOK) ? 0 : 1;
}
