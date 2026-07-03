#include "xui.h"
#include "xge.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEMO_TARGET_W 760
#define DEMO_TARGET_H 620
#define DEMO_OFFSET_X 10.0f
#define DEMO_OFFSET_Y 20.0f
#define PROGRESS_COUNT 10
#define LABEL_COUNT 10
#define SURFACE_COUNT 6

typedef struct xui_progress_box_t {
	xui_proxy pProxy;
	uint32_t iFill;
} xui_progress_box_t;

typedef struct xui_progress_demo_t {
	xui_proxy_t tProxy;
	xui_context pContext;
	xui_surface pTarget;
	xui_surface pSurface[SURFACE_COUNT];
	xui_font pFont;
	xui_widget pRoot;
	xui_widget pLabel[LABEL_COUNT];
	xui_widget pProgress[PROGRESS_COUNT];
	xui_progress_box_t tRootBox;
	float fProgress;
	float fPause;
	int iLabelCount;
	int iProgressCount;
	int iFrame;
	int iMaxFrames;
	double fMaxSeconds;
	int bCreateOK;
	int bLayoutOK;
	int bStateOK;
} xui_progress_demo_t;

static uint32_t __xuiProgressColorR(uint32_t iColor) { return (iColor >> 24) & 0xffu; }
static uint32_t __xuiProgressColorG(uint32_t iColor) { return (iColor >> 16) & 0xffu; }
static uint32_t __xuiProgressColorB(uint32_t iColor) { return (iColor >> 8) & 0xffu; }
static uint32_t __xuiProgressColorA(uint32_t iColor) { return iColor & 0xffu; }

static void __xuiProgressUsage(void)
{
	printf("usage: xui_progress [--frames N] [--seconds N]\n");
	printf("       no duration option means run until the window is closed.\n");
}

static int __xuiProgressParseArgs(xui_progress_demo_t* pDemo, int argc, char** argv)
{
	int i;

	for ( i = 1; i < argc; i++ ) {
		if ( strcmp(argv[i], "--frames") == 0 ) {
			if ( i + 1 >= argc ) {
				return XGE_ERROR_INVALID_ARGUMENT;
			}
			pDemo->iMaxFrames = atoi(argv[++i]);
			if ( pDemo->iMaxFrames <= 0 ) {
				return XGE_ERROR_INVALID_ARGUMENT;
			}
		} else if ( strncmp(argv[i], "--frames=", 9) == 0 ) {
			pDemo->iMaxFrames = atoi(argv[i] + 9);
			if ( pDemo->iMaxFrames <= 0 ) {
				return XGE_ERROR_INVALID_ARGUMENT;
			}
		} else if ( strcmp(argv[i], "--seconds") == 0 ) {
			if ( i + 1 >= argc ) {
				return XGE_ERROR_INVALID_ARGUMENT;
			}
			pDemo->fMaxSeconds = atof(argv[++i]);
			if ( pDemo->fMaxSeconds <= 0.0 ) {
				return XGE_ERROR_INVALID_ARGUMENT;
			}
		} else if ( strncmp(argv[i], "--seconds=", 10) == 0 ) {
			pDemo->fMaxSeconds = atof(argv[i] + 10);
			if ( pDemo->fMaxSeconds <= 0.0 ) {
				return XGE_ERROR_INVALID_ARGUMENT;
			}
		} else if ( (strcmp(argv[i], "--help") == 0) || (strcmp(argv[i], "-h") == 0) ) {
			__xuiProgressUsage();
			return 1;
		} else {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
	}
	return XGE_OK;
}

static const char* __xuiProgressFindTtf(void)
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

static uint32_t __xuiProgressBlend(uint32_t iA, uint32_t iB, int iStep, int iTotal)
{
	int r;
	int g;
	int b;
	int a;

	if ( iTotal <= 0 ) {
		return iA;
	}
	r = (int)__xuiProgressColorR(iA) + ((int)__xuiProgressColorR(iB) - (int)__xuiProgressColorR(iA)) * iStep / iTotal;
	g = (int)__xuiProgressColorG(iA) + ((int)__xuiProgressColorG(iB) - (int)__xuiProgressColorG(iA)) * iStep / iTotal;
	b = (int)__xuiProgressColorB(iA) + ((int)__xuiProgressColorB(iB) - (int)__xuiProgressColorB(iA)) * iStep / iTotal;
	a = (int)__xuiProgressColorA(iA) + ((int)__xuiProgressColorA(iB) - (int)__xuiProgressColorA(iA)) * iStep / iTotal;
	return XUI_COLOR_RGBA(r, g, b, a);
}

static void __xuiProgressWritePixel(unsigned char* pPixels, int iWidth, int x, int y, uint32_t iColor)
{
	pPixels[((y * iWidth + x) * 4) + 0] = (unsigned char)__xuiProgressColorR(iColor);
	pPixels[((y * iWidth + x) * 4) + 1] = (unsigned char)__xuiProgressColorG(iColor);
	pPixels[((y * iWidth + x) * 4) + 2] = (unsigned char)__xuiProgressColorB(iColor);
	pPixels[((y * iWidth + x) * 4) + 3] = (unsigned char)__xuiProgressColorA(iColor);
}

static void __xuiProgressMakeBarPixels(unsigned char* pPixels, int iWidth, int iHeight, uint32_t iTop, uint32_t iBottom, uint32_t iBorder, int bRounded, int bPattern)
{
	int iRadius;
	int x;
	int y;
	int dx;
	int dy;
	int bInside;
	uint32_t iColor;

	iRadius = iHeight / 2;
	for ( y = 0; y < iHeight; y++ ) {
		for ( x = 0; x < iWidth; x++ ) {
			bInside = 1;
			if ( bRounded ) {
				if ( x < iRadius ) {
					dx = iRadius - x - 1;
					dy = iRadius - y - 1;
					bInside = (dx * dx + dy * dy) <= (iRadius * iRadius);
				} else if ( x >= iWidth - iRadius ) {
					dx = x - (iWidth - iRadius);
					dy = iRadius - y - 1;
					bInside = (dx * dx + dy * dy) <= (iRadius * iRadius);
				}
			}
			if ( !bInside ) {
				__xuiProgressWritePixel(pPixels, iWidth, x, y, XUI_COLOR_RGBA(0, 0, 0, 0));
				continue;
			}
			iColor = __xuiProgressBlend(iTop, iBottom, y, iHeight - 1);
			if ( bPattern && ((x / 4) % 2 == 0) && (x > iRadius) && (x < iWidth - iRadius) ) {
				iColor = __xuiProgressBlend(iColor, XUI_COLOR_WHITE, 1, 5);
			}
			if ( (y == 0) || (y == iHeight - 1) || (x == 0) || (x == iWidth - 1) ) {
				iColor = iBorder;
			}
			__xuiProgressWritePixel(pPixels, iWidth, x, y, iColor);
		}
	}
}

static int __xuiProgressCreateSurface(xui_progress_demo_t* pDemo, int iIndex, int iWidth, int iHeight, uint32_t iTop, uint32_t iBottom, uint32_t iBorder, int bRounded, int bPattern)
{
	unsigned char arrPixels[64 * 24 * 4];

	if ( (iIndex < 0) || (iIndex >= SURFACE_COUNT) || (iWidth > 64) || (iHeight > 24) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	memset(arrPixels, 0, sizeof(arrPixels));
	__xuiProgressMakeBarPixels(arrPixels, iWidth, iHeight, iTop, iBottom, iBorder, bRounded, bPattern);
	return pDemo->tProxy.surfaceCreateRGBA(&pDemo->tProxy, &pDemo->pSurface[iIndex], iWidth, iHeight, arrPixels, iWidth * 4, XUI_SURFACE_ALPHA_PREMULTIPLIED);
}

static int __xuiProgressCreateSurfaces(xui_progress_demo_t* pDemo)
{
	if ( __xuiProgressCreateSurface(pDemo, 0, 32, 12, XUI_COLOR_RGBA(208, 232, 246, 255), XUI_COLOR_RGBA(186, 217, 236, 255), XUI_COLOR_RGBA(128, 170, 196, 255), 0, 0) != XUI_OK ||
	     __xuiProgressCreateSurface(pDemo, 1, 32, 12, XUI_COLOR_RGBA(72, 150, 222, 255), XUI_COLOR_RGBA(35, 110, 196, 255), XUI_COLOR_RGBA(18, 82, 160, 255), 0, 0) != XUI_OK ||
	     __xuiProgressCreateSurface(pDemo, 2, 48, 18, XUI_COLOR_RGBA(218, 236, 246, 255), XUI_COLOR_RGBA(194, 222, 238, 255), XUI_COLOR_RGBA(130, 174, 202, 255), 1, 0) != XUI_OK ||
	     __xuiProgressCreateSurface(pDemo, 3, 48, 18, XUI_COLOR_RGBA(75, 166, 230, 255), XUI_COLOR_RGBA(33, 126, 206, 255), XUI_COLOR_RGBA(19, 88, 168, 255), 1, 0) != XUI_OK ||
	     __xuiProgressCreateSurface(pDemo, 4, 48, 18, XUI_COLOR_RGBA(226, 235, 226, 255), XUI_COLOR_RGBA(198, 218, 204, 255), XUI_COLOR_RGBA(126, 164, 140, 255), 1, 1) != XUI_OK ||
	     __xuiProgressCreateSurface(pDemo, 5, 48, 18, XUI_COLOR_RGBA(85, 180, 136, 255), XUI_COLOR_RGBA(43, 146, 98, 255), XUI_COLOR_RGBA(26, 105, 72, 255), 1, 1) != XUI_OK ) {
		return XUI_ERROR;
	}
	return XUI_OK;
}

static void __xuiProgressThickness(float fLeft, float fTop, float fRight, float fBottom, xui_thickness_t* pOut)
{
	pOut->fLeft = fLeft;
	pOut->fTop = fTop;
	pOut->fRight = fRight;
	pOut->fBottom = fBottom;
}

static int __xuiProgressBoxRender(xui_widget pWidget, xui_draw_context pDraw, uint32_t iStateId, void* pUser)
{
	xui_progress_box_t* pBox;
	xui_rect_t tRect;

	(void)iStateId;
	pBox = (xui_progress_box_t*)pUser;
	if ( (pWidget == NULL) || (pDraw == NULL) || (pBox == NULL) || (pBox->pProxy == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	tRect = xuiWidgetGetRect(pWidget);
	tRect.fX = 0.0f;
	tRect.fY = 0.0f;
	return pBox->pProxy->drawRectFill(pBox->pProxy, pDraw, tRect, pBox->iFill);
}

static int __xuiProgressSetFillLayout(xui_widget pWidget)
{
	int iRet;

	iRet = xuiWidgetSetSizeMode(pWidget, XUI_SIZE_FILL, XUI_SIZE_FILL);
	if ( iRet == XUI_OK ) {
		iRet = xuiWidgetSetAlign(pWidget, XUI_ALIGN_STRETCH, XUI_ALIGN_STRETCH);
	}
	return iRet;
}

static int __xuiProgressSetFixedHeight(xui_widget pWidget, float fHeight)
{
	int iRet;

	iRet = xuiWidgetSetSizeMode(pWidget, XUI_SIZE_FILL, XUI_SIZE_FIXED);
	if ( iRet == XUI_OK ) {
		iRet = xuiWidgetSetPreferredSize(pWidget, (xui_vec2_t){0.0f, fHeight});
	}
	if ( iRet == XUI_OK ) {
		iRet = xuiWidgetSetAlign(pWidget, XUI_ALIGN_STRETCH, XUI_ALIGN_START);
	}
	return iRet;
}

static int __xuiProgressAddLabel(xui_progress_demo_t* pDemo, const char* sText)
{
	xui_label_desc_t tDesc;
	xui_widget pLabel;
	int iRet;

	if ( pDemo->iLabelCount >= LABEL_COUNT ) {
		return XUI_ERROR;
	}
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.sText = sText;
	tDesc.pFont = pDemo->pFont;
	tDesc.iTextColor = XUI_COLOR_RGBA(42, 52, 68, 255);
	tDesc.iDisabledTextColor = XUI_COLOR_RGBA(126, 134, 148, 255);
	tDesc.iTextFlags = XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP;
	iRet = xuiLabelCreate(pDemo->pContext, &pLabel, &tDesc);
	if ( iRet != XUI_OK ) {
		return iRet;
	}
	(void)__xuiProgressSetFixedHeight(pLabel, 18.0f);
	iRet = xuiWidgetAddChild(pDemo->pRoot, pLabel);
	if ( iRet != XUI_OK ) {
		xuiWidgetDestroy(pLabel);
		return iRet;
	}
	pDemo->pLabel[pDemo->iLabelCount++] = pLabel;
	return XUI_OK;
}

static xui_widget __xuiProgressAddProgress(xui_progress_demo_t* pDemo, const char* sTemplate, uint32_t iTrack, uint32_t iFill)
{
	xui_progress_desc_t tDesc;
	xui_widget pProgress;
	int iRet;

	if ( pDemo->iProgressCount >= PROGRESS_COUNT ) {
		return NULL;
	}
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.pFont = pDemo->pFont;
	tDesc.sText = sTemplate;
	tDesc.fMin = 0.0f;
	tDesc.fMax = 100.0f;
	tDesc.fValue = pDemo->fProgress;
	tDesc.iTrackColor = iTrack;
	tDesc.iFillColor = iFill;
	tDesc.iTextColor = XUI_COLOR_RGBA(36, 52, 70, 255);
	tDesc.iFillTextColor = XUI_COLOR_WHITE;
	iRet = xuiProgressCreate(pDemo->pContext, &pProgress, &tDesc);
	if ( iRet != XUI_OK ) {
		return NULL;
	}
	(void)__xuiProgressSetFixedHeight(pProgress, 24.0f);
	iRet = xuiWidgetAddChild(pDemo->pRoot, pProgress);
	if ( iRet != XUI_OK ) {
		xuiWidgetDestroy(pProgress);
		return NULL;
	}
	pDemo->pProgress[pDemo->iProgressCount++] = pProgress;
	return pProgress;
}

static void __xuiProgressPatchSimple(xui_nine_patch_t* pPatch, xui_surface pSurface)
{
	memset(pPatch, 0, sizeof(*pPatch));
	pPatch->iSize = sizeof(*pPatch);
	pPatch->pSurface = pSurface;
	pPatch->iColor = XUI_COLOR_WHITE;
	pPatch->iMode = XUI_NINE_PATCH_STRETCH;
}

static void __xuiProgressPatchNine(xui_nine_patch_t* pPatch, xui_surface pSurface, int iMode)
{
	memset(pPatch, 0, sizeof(*pPatch));
	pPatch->iSize = sizeof(*pPatch);
	pPatch->pSurface = pSurface;
	pPatch->tSlice.fLeft = 18.0f;
	pPatch->tSlice.fTop = 4.0f;
	pPatch->tSlice.fRight = 18.0f;
	pPatch->tSlice.fBottom = 4.0f;
	pPatch->iColor = XUI_COLOR_WHITE;
	pPatch->iMode = iMode;
}

static xui_widget __xuiProgressAddImageProgress(xui_progress_demo_t* pDemo, const char* sTemplate, int iFillPatchMode)
{
	xui_widget pProgress;
	xui_nine_patch_t tTrackPatch;
	xui_nine_patch_t tFillPatch;

	pProgress = __xuiProgressAddProgress(pDemo, sTemplate, XUI_COLOR_RGBA(0, 0, 0, 0), XUI_COLOR_RGBA(0, 0, 0, 0));
	if ( pProgress == NULL ) {
		return NULL;
	}
	__xuiProgressPatchSimple(&tTrackPatch, pDemo->pSurface[0]);
	__xuiProgressPatchSimple(&tFillPatch, pDemo->pSurface[1]);
	(void)xuiProgressSetTrackPatch(pProgress, &tTrackPatch);
	(void)xuiProgressSetFillPatch(pProgress, &tFillPatch);
	(void)xuiProgressSetFillPatchMode(pProgress, iFillPatchMode);
	return pProgress;
}

static xui_widget __xuiProgressAddPatchProgress(xui_progress_demo_t* pDemo, const char* sTemplate, xui_surface pTrack, xui_surface pFill, int iMode)
{
	xui_widget pProgress;
	xui_nine_patch_t tTrackPatch;
	xui_nine_patch_t tFillPatch;

	pProgress = __xuiProgressAddProgress(pDemo, sTemplate, XUI_COLOR_RGBA(0, 0, 0, 0), XUI_COLOR_RGBA(0, 0, 0, 0));
	if ( pProgress == NULL ) {
		return NULL;
	}
	__xuiProgressPatchNine(&tTrackPatch, pTrack, iMode);
	__xuiProgressPatchNine(&tFillPatch, pFill, iMode);
	(void)xuiProgressSetTrackPatch(pProgress, &tTrackPatch);
	(void)xuiProgressSetFillPatch(pProgress, &tFillPatch);
	return pProgress;
}

static int __xuiProgressCreateUi(xui_progress_demo_t* pDemo)
{
	xui_thickness_t tPadding;
	uint32_t iTrack;
	uint32_t iFill;

	iTrack = XUI_COLOR_RGBA(215, 232, 244, 255);
	iFill = XUI_COLOR_RGBA(46, 124, 214, 255);
	if ( xuiWidgetCreate(pDemo->pContext, &pDemo->pRoot) != XUI_OK ) {
		return XUI_ERROR;
	}
	pDemo->tRootBox.pProxy = &pDemo->tProxy;
	pDemo->tRootBox.iFill = XUI_COLOR_RGBA(229, 235, 244, 255);
	__xuiProgressThickness(28.0f, 18.0f, 28.0f, 18.0f, &tPadding);
	(void)xuiWidgetSetLayoutType(pDemo->pRoot, XUI_LAYOUT_COLUMN);
	(void)xuiWidgetSetGap(pDemo->pRoot, 5.0f);
	(void)xuiWidgetSetPadding(pDemo->pRoot, tPadding);
	(void)xuiWidgetSetCacheRenderCallback(pDemo->pRoot, __xuiProgressBoxRender, &pDemo->tRootBox);
	(void)__xuiProgressSetFillLayout(pDemo->pRoot);
	if ( xuiSetRootWidget(pDemo->pContext, pDemo->pRoot) != XUI_OK ) {
		return XUI_ERROR;
	}
	if ( (__xuiProgressAddLabel(pDemo, "Blank progress") != XUI_OK) ||
	     (__xuiProgressAddProgress(pDemo, NULL, iTrack, iFill) == NULL) ||
	     (__xuiProgressAddLabel(pDemo, "Percent template") != XUI_OK) ||
	     (__xuiProgressAddProgress(pDemo, "%1.0f%%", iTrack, iFill) == NULL) ||
	     (__xuiProgressAddLabel(pDemo, "Static text") != XUI_OK) ||
	     (__xuiProgressAddProgress(pDemo, "Loading", iTrack, iFill) == NULL) ||
	     (__xuiProgressAddLabel(pDemo, "Custom template") != XUI_OK) ||
	     (__xuiProgressAddProgress(pDemo, "Progress %1.2f", iTrack, iFill) == NULL) ||
	     (__xuiProgressAddLabel(pDemo, "Custom colors") != XUI_OK) ||
	     (__xuiProgressAddProgress(pDemo, "%1.0f%%", XUI_COLOR_RGBA(248, 226, 188, 255), XUI_COLOR_RGBA(211, 118, 36, 255)) == NULL) ||
	     (__xuiProgressAddLabel(pDemo, "Green compact") != XUI_OK) ||
	     (__xuiProgressAddProgress(pDemo, "%1.0f%%", XUI_COLOR_RGBA(211, 227, 239, 255), XUI_COLOR_RGBA(52, 151, 111, 255)) == NULL) ||
	     (__xuiProgressAddLabel(pDemo, "Image stretch") != XUI_OK) ||
	     (__xuiProgressAddImageProgress(pDemo, "%1.0f%%", XUI_PROGRESS_FILL_STRETCH) == NULL) ||
	     (__xuiProgressAddLabel(pDemo, "Image reveal") != XUI_OK) ||
	     (__xuiProgressAddImageProgress(pDemo, "%1.0f%%", XUI_PROGRESS_FILL_REVEAL) == NULL) ||
	     (__xuiProgressAddLabel(pDemo, "Nine-patch stretch") != XUI_OK) ||
	     (__xuiProgressAddPatchProgress(pDemo, "%1.0f%%", pDemo->pSurface[2], pDemo->pSurface[3], XUI_NINE_PATCH_STRETCH) == NULL) ||
	     (__xuiProgressAddLabel(pDemo, "Nine-patch tile") != XUI_OK) ||
	     (__xuiProgressAddPatchProgress(pDemo, "%1.0f%%", pDemo->pSurface[4], pDemo->pSurface[5], XUI_NINE_PATCH_TILE) == NULL) ) {
		return XUI_ERROR;
	}
	return XUI_OK;
}

static void __xuiProgressLayout(xui_progress_demo_t* pDemo)
{
	(void)xuiWidgetSetRect(pDemo->pRoot, (xui_rect_t){0.0f, 0.0f, (float)DEMO_TARGET_W, (float)DEMO_TARGET_H});
}

static void __xuiProgressUpdateValue(xui_progress_demo_t* pDemo, float fDelta)
{
	int i;

	if ( pDemo->fProgress >= 100.0f ) {
		pDemo->fPause += fDelta;
		if ( pDemo->fPause >= 0.75f ) {
			pDemo->fProgress = 0.0f;
			pDemo->fPause = 0.0f;
		}
	} else {
		pDemo->fProgress += fDelta * 34.0f;
		if ( pDemo->fProgress > 100.0f ) {
			pDemo->fProgress = 100.0f;
		}
	}
	for ( i = 0; i < pDemo->iProgressCount; i++ ) {
		(void)xuiProgressSetValue(pDemo->pProgress[i], pDemo->fProgress);
	}
}

static void __xuiProgressRunChecks(xui_progress_demo_t* pDemo)
{
	pDemo->bCreateOK = (pDemo->iLabelCount == LABEL_COUNT) && (pDemo->iProgressCount == PROGRESS_COUNT);
	pDemo->bLayoutOK = (xuiWidgetGetRect(pDemo->pRoot).fW == (float)DEMO_TARGET_W);
	pDemo->bStateOK = (xuiProgressGetTextTemplate(pDemo->pProgress[0]) == NULL) &&
		(xuiProgressGetDisplayText(pDemo->pProgress[1]) != NULL) &&
		(xuiProgressGetDisplayText(pDemo->pProgress[2]) != NULL) &&
		(xuiProgressHasFillPatch(pDemo->pProgress[6]) != 0) &&
		(xuiProgressGetFillPatchMode(pDemo->pProgress[7]) == XUI_PROGRESS_FILL_REVEAL) &&
		(xuiProgressHasTrackPatch(pDemo->pProgress[9]) != 0);
}

static int __xuiProgressCreateAssets(xui_progress_demo_t* pDemo)
{
	xui_surface_desc_t tSurfaceDesc;
	const char* sFontPath;
	int iRet;

	pDemo->tProxy = xuiProxyXge();
	iRet = xuiCreate(&pDemo->pContext);
	if ( iRet != XUI_OK ) {
		printf("xui_progress: xuiCreate failed: %d\n", iRet);
		return iRet;
	}
	iRet = xuiSetProxy(pDemo->pContext, &pDemo->tProxy);
	if ( iRet != XUI_OK ) {
		printf("xui_progress: xuiSetProxy failed: %d\n", iRet);
		return iRet;
	}
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
	sFontPath = __xuiProgressFindTtf();
	if ( sFontPath == NULL ) {
		printf("xui_progress: no usable system TTF font found\n");
		return XUI_ERROR_FILE_NOT_FOUND;
	}
	iRet = pDemo->tProxy.fontLoadFile(&pDemo->tProxy, &pDemo->pFont, sFontPath, 16.0f, XUI_FONT_FORMAT_TTF);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiSetDefaultFont(pDemo->pContext, pDemo->pFont);
	iRet = __xuiProgressCreateSurfaces(pDemo);
	if ( iRet != XUI_OK ) return iRet;
	return __xuiProgressCreateUi(pDemo);
}

static void __xuiProgressDestroyAssets(xui_progress_demo_t* pDemo)
{
	int i;

	if ( pDemo->pContext != NULL ) {
		xuiDestroy(pDemo->pContext);
	}
	if ( pDemo->pFont != NULL ) {
		pDemo->tProxy.fontDestroy(&pDemo->tProxy, pDemo->pFont);
	}
	for ( i = 0; i < SURFACE_COUNT; i++ ) {
		if ( pDemo->pSurface[i] != NULL ) {
			pDemo->tProxy.surfaceDestroy(&pDemo->tProxy, pDemo->pSurface[i]);
		}
	}
	if ( pDemo->pTarget != NULL ) {
		pDemo->tProxy.surfaceDestroy(&pDemo->tProxy, pDemo->pTarget);
	}
}

static int __xuiProgressFrame(void* pUser)
{
	xui_progress_demo_t* pDemo;
	xui_render_stats_t tStats;
	xui_cache_stats_t tCacheStats;
	xui_rect_i_t tFullRect;
	xui_rect_t tSrc;
	xui_rect_t tDst;
	int iRet;

	pDemo = (xui_progress_demo_t*)pUser;
	if ( xgeKeyPressed(XGE_KEY_ESCAPE) ) {
		xgeQuit();
	}
	iRet = xgeBegin();
	if ( iRet != XGE_OK ) return iRet;
	__xuiProgressUpdateValue(pDemo, xgeGetDelta());
	__xuiProgressLayout(pDemo);
	iRet = xuiUpdate(pDemo->pContext, xgeGetDelta());
	if ( iRet != XUI_OK ) return iRet;
	iRet = pDemo->tProxy.surfaceClear(&pDemo->tProxy, pDemo->pTarget, XUI_COLOR_RGBA(229, 235, 244, 255));
	if ( iRet != XUI_OK ) return iRet;
	tFullRect = (xui_rect_i_t){0, 0, DEMO_TARGET_W, DEMO_TARGET_H};
	iRet = xuiRender(pDemo->pContext, pDemo->pTarget, &tFullRect, 1);
	if ( iRet != XUI_OK ) return iRet;
	__xuiProgressRunChecks(pDemo);
	xgeClear(XUI_COLOR_RGBA(22, 26, 32, 255));
	tSrc = (xui_rect_t){0.0f, 0.0f, (float)DEMO_TARGET_W, (float)DEMO_TARGET_H};
	tDst = (xui_rect_t){DEMO_OFFSET_X, DEMO_OFFSET_Y, (float)DEMO_TARGET_W, (float)DEMO_TARGET_H};
	iRet = pDemo->tProxy.surfaceDraw(&pDemo->tProxy, pDemo->pTarget, tSrc, tDst, XUI_COLOR_WHITE, XUI_SURFACE_DRAW_SCREEN_SPACE);
	if ( iRet == XUI_OK ) {
		iRet = xgeEnd();
	}
	if ( iRet != XUI_OK ) return iRet;
	pDemo->iFrame++;
	if ( ((pDemo->iMaxFrames > 0) && (pDemo->iFrame >= pDemo->iMaxFrames)) ||
	     ((pDemo->fMaxSeconds > 0.0) && (xgeTimer() >= pDemo->fMaxSeconds)) ) {
		memset(&tStats, 0, sizeof(tStats));
		memset(&tCacheStats, 0, sizeof(tCacheStats));
		(void)xuiGetRenderStats(pDemo->pContext, &tStats);
		(void)xuiGetCacheStats(pDemo->pContext, &tCacheStats);
		printf("xui_progress final-summary frames=%d create=%d layout=%d state=%d labels=%d progress=%d value=%.2f updatedCaches=%d drawnCaches=%d cacheSurfaces=%d\n",
			pDemo->iFrame,
			pDemo->bCreateOK,
			pDemo->bLayoutOK,
			pDemo->bStateOK,
			pDemo->iLabelCount,
			pDemo->iProgressCount,
			pDemo->fProgress,
			tStats.iUpdatedCaches,
			tStats.iDrawnCaches,
			tCacheStats.iSurfaceCount);
		xgeQuit();
	}
	return XGE_OK;
}

int main(int argc, char** argv)
{
	xui_progress_demo_t tDemo;
	xge_desc_t tDesc;
	int iRet;

	memset(&tDemo, 0, sizeof(tDemo));
	iRet = __xuiProgressParseArgs(&tDemo, argc, argv);
	if ( iRet == 1 ) {
		return 0;
	}
	if ( iRet != XGE_OK ) {
		__xuiProgressUsage();
		return 1;
	}
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iWidth = DEMO_TARGET_W + 20;
	tDesc.iHeight = DEMO_TARGET_H + 50;
	tDesc.sTitle = "XUI Progress";
	tDesc.iFlags = XGE_INIT_WINDOW | XGE_INIT_VSYNC;
	tDesc.iRunMode = XGE_RUN_GAME_LOOP;
	tDesc.iTargetFPS = 60;
	iRet = xgeInit(&tDesc);
	if ( iRet != XGE_OK ) {
		printf("xui_progress: xgeInit failed: %d\n", iRet);
		return 1;
	}
	iRet = __xuiProgressCreateAssets(&tDemo);
	if ( iRet != XUI_OK ) {
		printf("xui_progress: create assets failed: %d\n", iRet);
		__xuiProgressDestroyAssets(&tDemo);
		xgeUnit();
		return 1;
	}
	iRet = xgeRun(__xuiProgressFrame, &tDemo);
	__xuiProgressDestroyAssets(&tDemo);
	xgeUnit();
	return (iRet == XGE_OK && tDemo.bCreateOK && tDemo.bLayoutOK && tDemo.bStateOK) ? 0 : 1;
}
