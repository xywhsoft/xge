#include "xui.h"
#include "xge.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEMO_TARGET_W	640
#define DEMO_TARGET_H	360
#define DEMO_OFFSET_X	10.0f
#define DEMO_OFFSET_Y	20.0f
#define TOGGLE_COUNT	7

typedef struct xui_toggle_demo_t {
	xui_proxy_t tProxy;
	xui_context pContext;
	xui_surface pTarget;
	xui_surface pAtlas;
	xui_font pFont;
	xui_widget pRoot;
	xui_widget pToggle[TOGGLE_COUNT];
	int iFrame;
	int iMaxFrames;
	double fMaxSeconds;
	int bHasMouse;
	float fLastMouseX;
	float fLastMouseY;
	float fUiMouseX;
	float fUiMouseY;
	uint32_t iLastButtons;
	int iChangeCount;
	int bExerciseDone;
	int bCreateOK;
	int bLayoutOK;
	int bStateOK;
	int bInputOK;
	int bAtlasOK;
} xui_toggle_demo_t;

static void __xuiToggleUsage(void)
{
	printf("usage: xui_toggle [--frames N] [--seconds N]\n");
}

static int __xuiToggleParseArgs(xui_toggle_demo_t* pDemo, int argc, char** argv)
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
		} else if ( (strcmp(argv[i], "--help") == 0) || (strcmp(argv[i], "-h") == 0) ) {
			__xuiToggleUsage();
			return 1;
		} else {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
	}
	return XGE_OK;
}

static const char* __xuiToggleFindTtf(void)
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

static int __xuiToggleRootRender(xui_widget pWidget, xui_draw_context pDraw, uint32_t iStateId, void* pUser)
{
	xui_toggle_demo_t* pDemo;
	xui_rect_t tRect;

	(void)iStateId;
	pDemo = (xui_toggle_demo_t*)pUser;
	if ( pDemo == NULL ) return XUI_OK;
	tRect = xuiWidgetGetContentRect(pWidget);
	return pDemo->tProxy.drawRoundRectFill(&pDemo->tProxy, pDraw, tRect, 8.0f, XUI_COLOR_RGBA(246, 249, 253, 255));
}

static int __xuiToggleSetFillLayout(xui_widget pWidget)
{
	int iRet;

	iRet = xuiWidgetSetSizeMode(pWidget, XUI_SIZE_FILL, XUI_SIZE_FILL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetDock(pWidget, XUI_DOCK_FILL);
	return iRet;
}

static int __xuiToggleSetFixedHeight(xui_widget pWidget, float fHeight)
{
	int iRet;

	iRet = xuiWidgetSetSizeMode(pWidget, XUI_SIZE_FILL, XUI_SIZE_FIXED);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetPreferredSize(pWidget, (xui_vec2_t){0.0f, fHeight});
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetAlign(pWidget, XUI_ALIGN_STRETCH, XUI_ALIGN_START);
	return iRet;
}

static void __xuiToggleChanged(xui_widget pWidget, int bChecked, void* pUser)
{
	xui_toggle_demo_t* pDemo;

	(void)pWidget;
	(void)bChecked;
	pDemo = (xui_toggle_demo_t*)pUser;
	if ( pDemo != NULL ) {
		pDemo->iChangeCount++;
	}
}

static int __xuiToggleAddLabel(xui_toggle_demo_t* pDemo, const char* sText)
{
	xui_label_desc_t tDesc;
	xui_widget pLabel;
	int iRet;

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.sText = sText;
	tDesc.pFont = pDemo->pFont;
	tDesc.iTextColor = XUI_COLOR_RGBA(70, 84, 104, 255);
	tDesc.iTextFlags = XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP;
	iRet = xuiLabelCreate(pDemo->pContext, &pLabel, &tDesc);
	if ( iRet != XUI_OK ) return iRet;
	(void)__xuiToggleSetFixedHeight(pLabel, 22.0f);
	iRet = xuiWidgetAddChild(pDemo->pRoot, pLabel);
	if ( iRet != XUI_OK ) {
		xuiWidgetDestroy(pLabel);
	}
	return iRet;
}

static int __xuiToggleAddToggle(xui_toggle_demo_t* pDemo, int iIndex, const char* sText, const char* sUncheckedText, const char* sCheckedText, int bChecked, int bEnabled)
{
	xui_toggle_desc_t tDesc;
	xui_widget pToggle;
	int iRet;

	if ( (iIndex < 0) || (iIndex >= TOGGLE_COUNT) ) return XUI_ERROR_INVALID_ARGUMENT;
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.sText = sText;
	tDesc.pFont = pDemo->pFont;
	tDesc.sUncheckedText = sUncheckedText;
	tDesc.sCheckedText = sCheckedText;
	tDesc.bChecked = bChecked;
	iRet = xuiToggleCreate(pDemo->pContext, &pToggle, &tDesc);
	if ( iRet != XUI_OK ) return iRet;
	(void)__xuiToggleSetFixedHeight(pToggle, 30.0f);
	if ( !bEnabled ) {
		(void)xuiWidgetSetEnabled(pToggle, 0);
	}
	(void)xuiToggleSetChange(pToggle, __xuiToggleChanged, pDemo);
	iRet = xuiWidgetAddChild(pDemo->pRoot, pToggle);
	if ( iRet != XUI_OK ) {
		xuiWidgetDestroy(pToggle);
		return iRet;
	}
	pDemo->pToggle[iIndex] = pToggle;
	return XUI_OK;
}

static void __xuiTogglePutPixel(unsigned char* pPixels, int iStride, int iX, int iY, uint32_t iColor)
{
	unsigned char* p;

	if ( (iX < 0) || (iX >= 96) || (iY < 0) || (iY >= 24) ) {
		return;
	}
	p = pPixels + iY * iStride + iX * 4;
	p[0] = (unsigned char)((iColor >> 24) & 0xffu);
	p[1] = (unsigned char)((iColor >> 16) & 0xffu);
	p[2] = (unsigned char)((iColor >> 8) & 0xffu);
	p[3] = (unsigned char)(iColor & 0xffu);
}

static void __xuiToggleFillCircle(unsigned char* pPixels, int iStride, int iCX, int iCY, int iRadius, uint32_t iColor)
{
	int x;
	int y;
	int dx;
	int dy;
	int r2;

	r2 = iRadius * iRadius;
	for ( y = iCY - iRadius; y <= iCY + iRadius; y++ ) {
		for ( x = iCX - iRadius; x <= iCX + iRadius; x++ ) {
			dx = x - iCX;
			dy = y - iCY;
			if ( dx * dx + dy * dy <= r2 ) {
				__xuiTogglePutPixel(pPixels, iStride, x, y, iColor);
			}
		}
	}
}

static void __xuiToggleFillPill(unsigned char* pPixels, int iStride, int iX, int iY, int iW, int iH, uint32_t iColor)
{
	int iRadius;
	int x;
	int y;

	iRadius = iH / 2;
	for ( y = iY; y < iY + iH; y++ ) {
		for ( x = iX + iRadius; x < iX + iW - iRadius; x++ ) {
			__xuiTogglePutPixel(pPixels, iStride, x, y, iColor);
		}
	}
	__xuiToggleFillCircle(pPixels, iStride, iX + iRadius, iY + iRadius, iRadius, iColor);
	__xuiToggleFillCircle(pPixels, iStride, iX + iW - iRadius - 1, iY + iRadius, iRadius, iColor);
}

static int __xuiToggleBuildAtlas(xui_toggle_demo_t* pDemo)
{
	unsigned char* pPixels;
	int iStride;
	int iRet;

	iStride = 96 * 4;
	pPixels = (unsigned char*)calloc(24, (size_t)iStride);
	if ( pPixels == NULL ) {
		return XUI_ERROR_OUT_OF_MEMORY;
	}
	__xuiToggleFillPill(pPixels, iStride, 1, 1, 44, 22, XUI_COLOR_RGBA(216, 226, 237, 255));
	__xuiToggleFillCircle(pPixels, iStride, 13, 12, 7, XUI_COLOR_WHITE);
	__xuiToggleFillPill(pPixels, iStride, 51, 1, 44, 22, XUI_COLOR_RGBA(47, 128, 237, 255));
	__xuiToggleFillCircle(pPixels, iStride, 82, 12, 7, XUI_COLOR_WHITE);
	iRet = pDemo->tProxy.surfaceCreateRGBA(&pDemo->tProxy, &pDemo->pAtlas, 96, 24, pPixels, iStride, XUI_SURFACE_ALPHA_PREMULTIPLIED);
	free(pPixels);
	return iRet;
}

static int __xuiToggleCreateUi(xui_toggle_demo_t* pDemo)
{
	xui_thickness_t tPadding;
	int iRet;

	iRet = xuiWidgetCreate(pDemo->pContext, &pDemo->pRoot);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiWidgetSetLayoutType(pDemo->pRoot, XUI_LAYOUT_COLUMN);
	(void)xuiWidgetSetGap(pDemo->pRoot, 8.0f);
	tPadding = (xui_thickness_t){28.0f, 22.0f, 28.0f, 22.0f};
	(void)xuiWidgetSetPadding(pDemo->pRoot, tPadding);
	(void)xuiWidgetSetCacheRenderCallback(pDemo->pRoot, __xuiToggleRootRender, pDemo);
	(void)__xuiToggleSetFillLayout(pDemo->pRoot);
	iRet = xuiSetRootWidget(pDemo->pContext, pDemo->pRoot);
	if ( iRet != XUI_OK ) return iRet;
	if ( __xuiToggleAddLabel(pDemo, "Default style") != XUI_OK ||
	     __xuiToggleAddToggle(pDemo, 0, "Unchecked option", "OFF", "ON", 0, 1) != XUI_OK ||
	     __xuiToggleAddToggle(pDemo, 1, "Checked option", "OFF", "ON", 1, 1) != XUI_OK ||
	     __xuiToggleAddToggle(pDemo, 2, "Disabled unchecked", "OFF", "ON", 0, 0) != XUI_OK ||
	     __xuiToggleAddToggle(pDemo, 3, "Disabled checked", "OFF", "ON", 1, 0) != XUI_OK ||
	     __xuiToggleAddLabel(pDemo, "Custom style") != XUI_OK ||
	     __xuiToggleAddToggle(pDemo, 4, "Green accent, compact thumb", "NO", "YES", 1, 1) != XUI_OK ||
	     __xuiToggleAddLabel(pDemo, "Atlas surface") != XUI_OK ||
	     __xuiToggleAddToggle(pDemo, 5, "Atlas unchecked", NULL, NULL, 0, 1) != XUI_OK ||
	     __xuiToggleAddToggle(pDemo, 6, "Atlas checked", NULL, NULL, 1, 1) != XUI_OK ) {
		return XUI_ERROR;
	}
	(void)xuiToggleSetTrackSize(pDemo->pToggle[4], 36.0f, 20.0f);
	(void)xuiToggleSetThumbSize(pDemo->pToggle[4], 12.0f);
	(void)xuiToggleSetColors(pDemo->pToggle[4],
		XUI_COLOR_RGBA(24, 166, 113, 255),
		XUI_COLOR_RGBA(214, 231, 224, 255),
		XUI_COLOR_RGBA(196, 222, 212, 255),
		XUI_COLOR_RGBA(24, 166, 113, 255));
	if ( pDemo->pAtlas != NULL ) {
		(void)xuiToggleSetTrackSize(pDemo->pToggle[5], 44.0f, 22.0f);
		(void)xuiToggleSetTrackSize(pDemo->pToggle[6], 44.0f, 22.0f);
		(void)xuiToggleSetIndicatorSurface(pDemo->pToggle[5], pDemo->pAtlas, (xui_rect_t){0.0f, 0.0f, 48.0f, 24.0f}, pDemo->pAtlas, (xui_rect_t){48.0f, 0.0f, 48.0f, 24.0f});
		(void)xuiToggleSetIndicatorSurface(pDemo->pToggle[6], pDemo->pAtlas, (xui_rect_t){0.0f, 0.0f, 48.0f, 24.0f}, pDemo->pAtlas, (xui_rect_t){48.0f, 0.0f, 48.0f, 24.0f});
		pDemo->bAtlasOK = 1;
	}
	return XUI_OK;
}

static void __xuiToggleLayout(xui_toggle_demo_t* pDemo)
{
	(void)xuiWidgetSetRect(pDemo->pRoot, (xui_rect_t){0.0f, 0.0f, (float)DEMO_TARGET_W, (float)DEMO_TARGET_H});
	(void)xuiLayout(pDemo->pContext);
}

static uint32_t __xuiToggleReadButtons(void)
{
	uint32_t iButtons;

	iButtons = 0;
	if ( xgeMouseDown(XGE_MOUSE_LEFT) ) {
		iButtons |= XUI_POINTER_BUTTON_LEFT;
	}
	if ( xgeMouseDown(XGE_MOUSE_RIGHT) ) {
		iButtons |= XUI_POINTER_BUTTON_RIGHT;
	}
	if ( xgeMouseDown(XGE_MOUSE_MIDDLE) ) {
		iButtons |= XUI_POINTER_BUTTON_MIDDLE;
	}
	return iButtons;
}

static int __xuiToggleSendButtonTransitions(xui_toggle_demo_t* pDemo, float fX, float fY, uint32_t iButtons, uint32_t iPressed, uint32_t iReleased)
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

static int __xuiToggleHandleInput(xui_toggle_demo_t* pDemo)
{
	float fX;
	float fY;
	uint32_t iButtons;
	uint32_t iPressed;
	uint32_t iReleased;
	int iRet;

	if ( xgeKeyPressed(XGE_KEY_ESCAPE) ) {
		xgeQuit();
	}
	xgeMouseGet(&fX, &fY);
	pDemo->fUiMouseX = fX - DEMO_OFFSET_X;
	pDemo->fUiMouseY = fY - DEMO_OFFSET_Y;
	iButtons = __xuiToggleReadButtons();
	if ( !pDemo->bHasMouse || (pDemo->fLastMouseX != fX) || (pDemo->fLastMouseY != fY) || (pDemo->iLastButtons != iButtons) ) {
		iRet = xuiInputPointerMove(pDemo->pContext, pDemo->fUiMouseX, pDemo->fUiMouseY, iButtons);
		if ( iRet != XUI_OK ) return iRet;
	}
	iPressed = iButtons & ~pDemo->iLastButtons;
	iReleased = pDemo->iLastButtons & ~iButtons;
	iRet = __xuiToggleSendButtonTransitions(pDemo, pDemo->fUiMouseX, pDemo->fUiMouseY, iButtons, iPressed, iReleased);
	if ( iRet != XUI_OK ) return iRet;
	pDemo->bHasMouse = 1;
	pDemo->fLastMouseX = fX;
	pDemo->fLastMouseY = fY;
	pDemo->iLastButtons = iButtons;
	return XUI_OK;
}

static void __xuiToggleRunChecks(xui_toggle_demo_t* pDemo, int bExerciseInput)
{
	xui_rect_t tRect;
	xui_rect_t tToggle;
	float fX;
	float fY;
	int i;

	pDemo->bCreateOK = (pDemo->pRoot != NULL);
	for ( i = 0; i < TOGGLE_COUNT; i++ ) {
		if ( pDemo->pToggle[i] == NULL ) pDemo->bCreateOK = 0;
	}
	tRect = xuiWidgetGetRect(pDemo->pToggle[0]);
	pDemo->bLayoutOK = (tRect.fW > 100.0f) && (tRect.fH >= 24.0f);
	pDemo->bStateOK = (xuiToggleGetChecked(pDemo->pToggle[1]) != 0) &&
	                  (xuiWidgetGetEnabled(pDemo->pToggle[2]) == 0) &&
	                  (strcmp(xuiToggleGetUncheckedText(pDemo->pToggle[0]), "OFF") == 0) &&
	                  (strcmp(xuiToggleGetCheckedText(pDemo->pToggle[1]), "ON") == 0) &&
	                  (xuiToggleGetThumbSize(pDemo->pToggle[4]) == 12.0f) &&
	                  (pDemo->bAtlasOK != 0);
	if ( bExerciseInput && !pDemo->bExerciseDone && pDemo->bLayoutOK ) {
		tToggle = xuiWidgetGetWorldRect(pDemo->pToggle[0]);
		fX = tToggle.fX + 18.0f;
		fY = tToggle.fY + tToggle.fH * 0.5f;
		(void)xuiInputPointerMove(pDemo->pContext, fX, fY, 0);
		(void)xuiDispatchPendingEvents(pDemo->pContext);
		(void)xuiInputPointerDown(pDemo->pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
		(void)xuiDispatchPendingEvents(pDemo->pContext);
		(void)xuiInputPointerUp(pDemo->pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, 0);
		(void)xuiDispatchPendingEvents(pDemo->pContext);
		pDemo->bExerciseDone = 1;
	}
	pDemo->bInputOK = !bExerciseInput || ((pDemo->iChangeCount > 0) && (xuiToggleGetChecked(pDemo->pToggle[0]) != 0));
}

static int __xuiToggleCreateAssets(xui_toggle_demo_t* pDemo)
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
	iRet = __xuiToggleBuildAtlas(pDemo);
	if ( iRet != XUI_OK ) return iRet;
	sFontPath = __xuiToggleFindTtf();
	if ( sFontPath == NULL ) return XUI_ERROR_FILE_NOT_FOUND;
	iRet = pDemo->tProxy.fontLoadFile(&pDemo->tProxy, &pDemo->pFont, sFontPath, 15.0f, 0);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiSetDefaultFont(pDemo->pContext, pDemo->pFont);
	return __xuiToggleCreateUi(pDemo);
}

static void __xuiToggleDestroyAssets(xui_toggle_demo_t* pDemo)
{
	if ( pDemo->pContext != NULL ) {
		xuiDestroy(pDemo->pContext);
		pDemo->pContext = NULL;
	}
	if ( pDemo->pFont != NULL ) {
		pDemo->tProxy.fontDestroy(&pDemo->tProxy, pDemo->pFont);
		pDemo->pFont = NULL;
	}
	if ( pDemo->pAtlas != NULL ) {
		pDemo->tProxy.surfaceDestroy(&pDemo->tProxy, pDemo->pAtlas);
		pDemo->pAtlas = NULL;
	}
	if ( pDemo->pTarget != NULL ) {
		pDemo->tProxy.surfaceDestroy(&pDemo->tProxy, pDemo->pTarget);
		pDemo->pTarget = NULL;
	}
}

static int __xuiToggleFrame(void* pUser)
{
	xui_toggle_demo_t* pDemo;
	xui_rect_i_t tFullRect;
	xui_rect_t tSrc;
	xui_rect_t tDst;
	xui_render_stats_t tStats;
	xui_cache_stats_t tCacheStats;
	int iRet;
	int bAutoRun;

	pDemo = (xui_toggle_demo_t*)pUser;
	if ( pDemo == NULL ) return XGE_ERROR_INVALID_ARGUMENT;
	bAutoRun = (pDemo->iMaxFrames > 0) || (pDemo->fMaxSeconds > 0.0);
	iRet = xgeBegin();
	if ( iRet != XGE_OK ) return iRet;
	iRet = __xuiToggleHandleInput(pDemo);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiDispatchPendingEvents(pDemo->pContext);
	if ( iRet != XUI_OK ) return iRet;
	__xuiToggleLayout(pDemo);
	iRet = xuiUpdate(pDemo->pContext, xgeGetDelta());
	if ( iRet != XUI_OK ) return iRet;
	__xuiToggleRunChecks(pDemo, bAutoRun);
	iRet = pDemo->tProxy.surfaceClear(&pDemo->tProxy, pDemo->pTarget, XUI_COLOR_RGBA(229, 235, 244, 255));
	if ( iRet != XUI_OK ) return iRet;
	tFullRect = (xui_rect_i_t){0, 0, DEMO_TARGET_W, DEMO_TARGET_H};
	iRet = xuiRender(pDemo->pContext, pDemo->pTarget, &tFullRect, 1);
	if ( iRet != XUI_OK ) return iRet;
	xgeClear(XUI_COLOR_RGBA(22, 26, 32, 255));
	tSrc = (xui_rect_t){0.0f, 0.0f, (float)DEMO_TARGET_W, (float)DEMO_TARGET_H};
	tDst = (xui_rect_t){DEMO_OFFSET_X, DEMO_OFFSET_Y, (float)DEMO_TARGET_W, (float)DEMO_TARGET_H};
	iRet = pDemo->tProxy.surfaceDraw(&pDemo->tProxy, pDemo->pTarget, tSrc, tDst, XUI_COLOR_WHITE, XUI_SURFACE_DRAW_SCREEN_SPACE);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xgeEnd();
	if ( iRet != XGE_OK ) return iRet;
	pDemo->iFrame++;
	if ( (pDemo->iMaxFrames > 0 && pDemo->iFrame >= pDemo->iMaxFrames) ||
	     (pDemo->fMaxSeconds > 0.0 && xgeTimer() >= pDemo->fMaxSeconds) ) {
		memset(&tStats, 0, sizeof(tStats));
		memset(&tCacheStats, 0, sizeof(tCacheStats));
		tStats.iSize = sizeof(tStats);
		tCacheStats.iSize = sizeof(tCacheStats);
		(void)xuiGetRenderStats(pDemo->pContext, &tStats);
		(void)xuiGetCacheStats(pDemo->pContext, &tCacheStats);
		printf("xui_toggle final-summary frames=%d create=%d layout=%d state=%d input=%d atlas=%d changes=%d updatedCaches=%d drawnCaches=%d cacheSurfaces=%d\n",
			pDemo->iFrame, pDemo->bCreateOK, pDemo->bLayoutOK, pDemo->bStateOK, pDemo->bInputOK, pDemo->bAtlasOK,
			pDemo->iChangeCount, tStats.iUpdatedCaches, tStats.iDrawnCaches, tCacheStats.iSurfaceCount);
		xgeQuit();
	}
	return XGE_OK;
}

int main(int argc, char** argv)
{
	xui_toggle_demo_t tDemo;
	xge_desc_t tDesc;
	int iRet;

	memset(&tDemo, 0, sizeof(tDemo));
	iRet = __xuiToggleParseArgs(&tDemo, argc, argv);
	if ( iRet == 1 ) return 0;
	if ( iRet != XGE_OK ) {
		__xuiToggleUsage();
		return 1;
	}
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iWidth = DEMO_TARGET_W + 20;
	tDesc.iHeight = DEMO_TARGET_H + 50;
	tDesc.sTitle = "XUI Toggle";
	tDesc.iFlags = XGE_INIT_WINDOW | XGE_INIT_VSYNC;
	tDesc.iRunMode = XGE_RUN_GAME_LOOP;
	tDesc.iTargetFPS = 60;
	iRet = xgeInit(&tDesc);
	if ( iRet != XGE_OK ) {
		printf("xui_toggle: xgeInit failed: %d\n", iRet);
		return 1;
	}
	iRet = __xuiToggleCreateAssets(&tDemo);
	if ( iRet != XUI_OK ) {
		printf("xui_toggle: create assets failed: %d\n", iRet);
		__xuiToggleDestroyAssets(&tDemo);
		xgeUnit();
		return 1;
	}
	iRet = xgeRun(__xuiToggleFrame, &tDemo);
	__xuiToggleDestroyAssets(&tDemo);
	xgeUnit();
	return (iRet == XGE_OK && tDemo.bCreateOK && tDemo.bLayoutOK && tDemo.bStateOK && tDemo.bInputOK && tDemo.bAtlasOK) ? 0 : 1;
}
