#include "xui.h"
#include "xge.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEMO_TARGET_W	940
#define DEMO_TARGET_H	560
#define DEMO_OFFSET_X	10.0f
#define DEMO_OFFSET_Y	20.0f
#define IMAGE_AREA_COUNT	4
#define IMAGE_PER_AREA		9
#define IMAGE_COUNT		(IMAGE_AREA_COUNT * IMAGE_PER_AREA)
#define SURFACE_COUNT		3

typedef struct xui_image_box_t {
	xui_proxy pProxy;
	uint32_t iFill;
	uint32_t iStroke;
	float fRadius;
	float fStrokeWidth;
} xui_image_box_t;

typedef struct xui_image_demo_t {
	xui_proxy_t tProxy;
	xui_context pContext;
	xui_surface pTarget;
	xui_surface pSurface[SURFACE_COUNT];
	xui_widget pRoot;
	xui_widget pArea[IMAGE_AREA_COUNT];
	xui_widget pCell[IMAGE_COUNT];
	xui_widget pImage[IMAGE_COUNT];
	xui_image_box_t tRootBox;
	xui_image_box_t tAreaBox[IMAGE_AREA_COUNT];
	xui_image_box_t tCellBox[IMAGE_COUNT];
	int iFrame;
	int iMaxFrames;
	double fMaxSeconds;
	int bCreateOK;
	int bLayoutOK;
	int bStateOK;
} xui_image_demo_t;

static uint32_t __xuiImageColorR(uint32_t iColor) { return (iColor >> 24) & 0xffu; }
static uint32_t __xuiImageColorG(uint32_t iColor) { return (iColor >> 16) & 0xffu; }
static uint32_t __xuiImageColorB(uint32_t iColor) { return (iColor >> 8) & 0xffu; }
static uint32_t __xuiImageColorA(uint32_t iColor) { return iColor & 0xffu; }

static void __xuiImageUsage(void)
{
	printf("usage: xui_image [--frames N] [--seconds N]\n");
	printf("       no duration option means run until the window is closed.\n");
}

static int __xuiImageParseArgs(xui_image_demo_t* pDemo, int argc, char** argv)
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
			__xuiImageUsage();
			return 1;
		} else {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
	}
	return XGE_OK;
}

static uint32_t __xuiImageBlend(uint32_t iA, uint32_t iB, int iStep, int iTotal)
{
	int r;
	int g;
	int b;
	int a;

	if ( iTotal <= 0 ) {
		return iA;
	}
	r = (int)__xuiImageColorR(iA) + ((int)__xuiImageColorR(iB) - (int)__xuiImageColorR(iA)) * iStep / iTotal;
	g = (int)__xuiImageColorG(iA) + ((int)__xuiImageColorG(iB) - (int)__xuiImageColorG(iA)) * iStep / iTotal;
	b = (int)__xuiImageColorB(iA) + ((int)__xuiImageColorB(iB) - (int)__xuiImageColorB(iA)) * iStep / iTotal;
	a = (int)__xuiImageColorA(iA) + ((int)__xuiImageColorA(iB) - (int)__xuiImageColorA(iA)) * iStep / iTotal;
	return XUI_COLOR_RGBA(r, g, b, a);
}

static void __xuiImageWritePixel(unsigned char* pPixels, int iWidth, int x, int y, uint32_t iColor)
{
	pPixels[((y * iWidth + x) * 4) + 0] = (unsigned char)__xuiImageColorR(iColor);
	pPixels[((y * iWidth + x) * 4) + 1] = (unsigned char)__xuiImageColorG(iColor);
	pPixels[((y * iWidth + x) * 4) + 2] = (unsigned char)__xuiImageColorB(iColor);
	pPixels[((y * iWidth + x) * 4) + 3] = (unsigned char)__xuiImageColorA(iColor);
}

static void __xuiImageMakePixels(unsigned char* pPixels, int iWidth, int iHeight, uint32_t iTop, uint32_t iBottom)
{
	int x;
	int y;
	int bLine;
	uint32_t iColor;

	for ( y = 0; y < iHeight; y++ ) {
		for ( x = 0; x < iWidth; x++ ) {
			bLine = ((x % 8) == 0) || ((y % 8) == 0) || (x == 0) || (y == 0) || (x == iWidth - 1) || (y == iHeight - 1);
			iColor = bLine ? XUI_COLOR_RGBA(32, 92, 168, 255) : __xuiImageBlend(iTop, iBottom, y, iHeight - 1);
			if ( ((x / 12) + (y / 12)) % 2 == 0 ) {
				iColor = __xuiImageBlend(iColor, XUI_COLOR_RGBA(255, 255, 255, 255), 1, 7);
			}
			__xuiImageWritePixel(pPixels, iWidth, x, y, iColor);
		}
	}
}

static int __xuiImageLoadFileBytes(const char* sPath, unsigned char** ppData, int* pSize)
{
	FILE* pFile;
	long iSize;
	unsigned char* pData;

	if ( (sPath == NULL) || (ppData == NULL) || (pSize == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	*ppData = NULL;
	*pSize = 0;
	pFile = fopen(sPath, "rb");
	if ( pFile == NULL ) {
		return XUI_ERROR_FILE_NOT_FOUND;
	}
	if ( (fseek(pFile, 0, SEEK_END) != 0) || ((iSize = ftell(pFile)) <= 0) || (fseek(pFile, 0, SEEK_SET) != 0) ) {
		fclose(pFile);
		return XUI_ERROR_RESOURCE_FAILED;
	}
	pData = (unsigned char*)xrtMalloc((size_t)iSize);
	if ( pData == NULL ) {
		fclose(pFile);
		return XUI_ERROR_OUT_OF_MEMORY;
	}
	if ( fread(pData, 1, (size_t)iSize, pFile) != (size_t)iSize ) {
		xrtFree(pData);
		fclose(pFile);
		return XUI_ERROR_RESOURCE_FAILED;
	}
	fclose(pFile);
	*ppData = pData;
	*pSize = (int)iSize;
	return XUI_OK;
}

static int __xuiImageFullPath(const char* sPath, char* sOut, int iCapacity)
{
	if ( (sPath == NULL) || (sOut == NULL) || (iCapacity <= 0) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
#if defined(_WIN32) || defined(_WIN64)
	if ( _fullpath(sOut, sPath, (size_t)iCapacity) == NULL ) {
		return XUI_ERROR_FILE_NOT_FOUND;
	}
#else
	if ( strlen(sPath) >= (size_t)iCapacity ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	strcpy(sOut, sPath);
#endif
	return XUI_OK;
}

static int __xuiImageCreateSurfaces(xui_image_demo_t* pDemo)
{
	unsigned char arrPixels[96 * 56 * 4];
	unsigned char* pMemory;
	char arrFilePath[1024];
	char arrMemoryPath[1024];
	int iMemorySize;
	int iRet;

	pMemory = NULL;
	iMemorySize = 0;
	__xuiImageMakePixels(arrPixels, 96, 56, XUI_COLOR_RGBA(65, 172, 234, 255), XUI_COLOR_RGBA(42, 94, 190, 255));
	iRet = pDemo->tProxy.surfaceCreateRGBA(&pDemo->tProxy, &pDemo->pSurface[0], 96, 56, arrPixels, 96 * 4, XUI_SURFACE_ALPHA_PREMULTIPLIED);
	if ( iRet != XUI_OK ) {
		printf("xui_image: surfaceCreateRGBA failed: %d\n", iRet);
		return iRet;
	}
	iRet = xgeImageSavePNG("xui_image_file.png", 96, 56, arrPixels, 96 * 4);
	if ( iRet != XGE_OK ) {
		printf("xui_image: save file png failed: %d\n", iRet);
		return XUI_ERROR_RESOURCE_FAILED;
	}
	iRet = __xuiImageFullPath("xui_image_file.png", arrFilePath, (int)sizeof(arrFilePath));
	if ( iRet != XUI_OK ) {
		printf("xui_image: file png full path failed: %d\n", iRet);
		return iRet;
	}
	iRet = pDemo->tProxy.surfaceLoadFile(&pDemo->tProxy, &pDemo->pSurface[1], arrFilePath, XUI_SURFACE_ALPHA_PREMULTIPLIED);
	if ( iRet != XUI_OK ) {
		printf("xui_image: surfaceLoadFile failed: %d\n", iRet);
		return iRet;
	}
	__xuiImageMakePixels(arrPixels, 96, 56, XUI_COLOR_RGBA(126, 206, 150, 255), XUI_COLOR_RGBA(48, 146, 92, 255));
	iRet = xgeImageSavePNG("xui_image_memory.png", 96, 56, arrPixels, 96 * 4);
	if ( iRet != XGE_OK ) {
		printf("xui_image: save memory png failed: %d\n", iRet);
		return XUI_ERROR_RESOURCE_FAILED;
	}
	iRet = __xuiImageFullPath("xui_image_memory.png", arrMemoryPath, (int)sizeof(arrMemoryPath));
	if ( iRet != XUI_OK ) {
		printf("xui_image: memory png full path failed: %d\n", iRet);
		return iRet;
	}
	iRet = __xuiImageLoadFileBytes(arrMemoryPath, &pMemory, &iMemorySize);
	if ( iRet != XUI_OK ) {
		printf("xui_image: load memory png bytes failed: %d\n", iRet);
		return iRet;
	}
	iRet = pDemo->tProxy.surfaceLoadMemory(&pDemo->tProxy, &pDemo->pSurface[2], pMemory, iMemorySize, XUI_SURFACE_ALPHA_PREMULTIPLIED);
	xrtFree(pMemory);
	if ( iRet != XUI_OK ) {
		printf("xui_image: surfaceLoadMemory failed: %d\n", iRet);
	}
	return iRet;
}

static int __xuiImageBoxRender(xui_widget pWidget, xui_draw_context pDraw, uint32_t iStateId, void* pUser)
{
	xui_image_box_t* pBox;
	xui_rect_t tRect;
	int iRet;

	(void)iStateId;
	pBox = (xui_image_box_t*)pUser;
	if ( (pWidget == NULL) || (pDraw == NULL) || (pBox == NULL) || (pBox->pProxy == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	tRect = xuiWidgetGetRect(pWidget);
	tRect.fX = 0.0f;
	tRect.fY = 0.0f;
	iRet = pBox->pProxy->drawRoundRectFill(pBox->pProxy, pDraw, tRect, pBox->fRadius, pBox->iFill);
	if ( iRet != XUI_OK ) {
		return iRet;
	}
	if ( pBox->fStrokeWidth > 0.0f ) {
		iRet = pBox->pProxy->drawRoundRectStroke(pBox->pProxy, pDraw, tRect, pBox->fRadius, pBox->fStrokeWidth, pBox->iStroke);
	}
	return iRet;
}

static void __xuiImageThickness(float fLeft, float fTop, float fRight, float fBottom, xui_thickness_t* pOut)
{
	pOut->fLeft = fLeft;
	pOut->fTop = fTop;
	pOut->fRight = fRight;
	pOut->fBottom = fBottom;
}

static int __xuiImageSetFillLayout(xui_widget pWidget)
{
	int iRet;

	iRet = xuiWidgetSetSizeMode(pWidget, XUI_SIZE_FILL, XUI_SIZE_FILL);
	if ( iRet == XUI_OK ) {
		iRet = xuiWidgetSetAlign(pWidget, XUI_ALIGN_STRETCH, XUI_ALIGN_STRETCH);
	}
	return iRet;
}

static int __xuiImageAlignX(int iSlot)
{
	static const int arrAlign[IMAGE_PER_AREA] = {
		XUI_ALIGN_START, XUI_ALIGN_CENTER, XUI_ALIGN_END,
		XUI_ALIGN_START, XUI_ALIGN_CENTER, XUI_ALIGN_END,
		XUI_ALIGN_START, XUI_ALIGN_CENTER, XUI_ALIGN_END
	};
	return arrAlign[iSlot % IMAGE_PER_AREA];
}

static int __xuiImageAlignY(int iSlot)
{
	static const int arrAlign[IMAGE_PER_AREA] = {
		XUI_ALIGN_START, XUI_ALIGN_START, XUI_ALIGN_START,
		XUI_ALIGN_CENTER, XUI_ALIGN_CENTER, XUI_ALIGN_CENTER,
		XUI_ALIGN_END, XUI_ALIGN_END, XUI_ALIGN_END
	};
	return arrAlign[iSlot % IMAGE_PER_AREA];
}

static void __xuiImageSetupBox(xui_image_box_t* pBox, xui_proxy pProxy, uint32_t iFill, uint32_t iStroke, float fRadius, float fStrokeWidth)
{
	pBox->pProxy = pProxy;
	pBox->iFill = iFill;
	pBox->iStroke = iStroke;
	pBox->fRadius = fRadius;
	pBox->fStrokeWidth = fStrokeWidth;
}

static int __xuiImageAddArea(xui_image_demo_t* pDemo, int iArea, uint32_t iFill)
{
	xui_thickness_t tPadding;
	int iRet;

	iRet = xuiWidgetCreate(pDemo->pContext, &pDemo->pArea[iArea]);
	if ( iRet != XUI_OK ) {
		return iRet;
	}
	__xuiImageSetupBox(&pDemo->tAreaBox[iArea], &pDemo->tProxy, iFill, XUI_COLOR_RGBA(170, 184, 202, 255), 5.0f, 1.0f);
	__xuiImageThickness(10.0f, 10.0f, 10.0f, 10.0f, &tPadding);
	(void)xuiWidgetSetLayoutType(pDemo->pArea[iArea], XUI_LAYOUT_GRID);
	(void)xuiWidgetSetGap(pDemo->pArea[iArea], 8.0f);
	(void)xuiWidgetSetPadding(pDemo->pArea[iArea], tPadding);
	(void)__xuiImageSetFillLayout(pDemo->pArea[iArea]);
	(void)xuiWidgetSetCacheRenderCallback(pDemo->pArea[iArea], __xuiImageBoxRender, &pDemo->tAreaBox[iArea]);
	return xuiWidgetAddChild(pDemo->pRoot, pDemo->pArea[iArea]);
}

static int __xuiImageAddImage(xui_image_demo_t* pDemo, int iArea, int iSlot)
{
	xui_image_desc_t tDesc;
	xui_thickness_t tPadding;
	uint32_t iFill;
	uint32_t iStroke;
	int iIndex;
	int iRet;

	iIndex = iArea * IMAGE_PER_AREA + iSlot;
	iFill = (iArea == 2) ?
		XUI_COLOR_RGBA(235 + (iSlot % 3) * 4, 240 + (iSlot / 3) * 4, 247, 255) :
		XUI_COLOR_RGBA(247, 250, 253, 255);
	iStroke = (iArea == 1) ? XUI_COLOR_RGBA(74, 126, 184, 255) : XUI_COLOR_RGBA(198, 208, 222, 255);

	iRet = xuiWidgetCreate(pDemo->pContext, &pDemo->pCell[iIndex]);
	if ( iRet != XUI_OK ) {
		return iRet;
	}
	__xuiImageSetupBox(&pDemo->tCellBox[iIndex], &pDemo->tProxy, iFill, iStroke, 3.0f, (iArea == 0) ? 0.0f : 1.0f);
	(void)xuiWidgetSetLayoutType(pDemo->pCell[iIndex], XUI_LAYOUT_OVERLAY);
	(void)__xuiImageSetFillLayout(pDemo->pCell[iIndex]);
	(void)xuiWidgetSetCacheRenderCallback(pDemo->pCell[iIndex], __xuiImageBoxRender, &pDemo->tCellBox[iIndex]);
	iRet = xuiWidgetAddChild(pDemo->pArea[iArea], pDemo->pCell[iIndex]);
	if ( iRet != XUI_OK ) {
		return iRet;
	}

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.pSurface = pDemo->pSurface[(iArea == 0 && iSlot < SURFACE_COUNT) ? iSlot : 0];
	tDesc.tSrc = (xui_rect_t){8.0f, 8.0f, 64.0f, 40.0f};
	tDesc.iColor = XUI_COLOR_WHITE;
	tDesc.iMode = XUI_IMAGE_NATURAL;
	tDesc.iAlignX = __xuiImageAlignX(iSlot);
	tDesc.iAlignY = __xuiImageAlignY(iSlot);
	iRet = xuiImageCreate(pDemo->pContext, &pDemo->pImage[iIndex], &tDesc);
	if ( iRet != XUI_OK ) {
		return iRet;
	}
	__xuiImageThickness(6.0f, 6.0f, 6.0f, 6.0f, &tPadding);
	(void)xuiWidgetSetPadding(pDemo->pImage[iIndex], tPadding);
	(void)__xuiImageSetFillLayout(pDemo->pImage[iIndex]);
	return xuiWidgetAddChild(pDemo->pCell[iIndex], pDemo->pImage[iIndex]);
}

static int __xuiImageCreateUi(xui_image_demo_t* pDemo)
{
	xui_thickness_t tPadding;
	int i;
	int iRet;

	iRet = xuiWidgetCreate(pDemo->pContext, &pDemo->pRoot);
	if ( iRet != XUI_OK ) {
		return iRet;
	}
	iRet = xuiSetRootWidget(pDemo->pContext, pDemo->pRoot);
	if ( iRet != XUI_OK ) {
		return iRet;
	}
	__xuiImageSetupBox(&pDemo->tRootBox, &pDemo->tProxy, XUI_COLOR_RGBA(229, 235, 244, 255), XUI_COLOR_RGBA(0, 0, 0, 0), 0.0f, 0.0f);
	__xuiImageThickness(18.0f, 18.0f, 18.0f, 18.0f, &tPadding);
	(void)xuiWidgetSetRect(pDemo->pRoot, (xui_rect_t){0.0f, 0.0f, (float)DEMO_TARGET_W, (float)DEMO_TARGET_H});
	(void)xuiWidgetSetLayoutType(pDemo->pRoot, XUI_LAYOUT_GRID);
	(void)xuiWidgetSetGap(pDemo->pRoot, 12.0f);
	(void)xuiWidgetSetPadding(pDemo->pRoot, tPadding);
	(void)xuiWidgetSetGridMetrics(pDemo->pRoot, 2, 446.0f, 256.0f);
	(void)xuiWidgetSetCacheRenderCallback(pDemo->pRoot, __xuiImageBoxRender, &pDemo->tRootBox);
	for ( i = 0; i < IMAGE_AREA_COUNT; i++ ) {
		iRet = __xuiImageAddArea(pDemo, i, (i == 2) ? XUI_COLOR_RGBA(231, 238, 247, 255) : XUI_COLOR_RGBA(248, 250, 253, 255));
		if ( iRet != XUI_OK ) return iRet;
	}
	for ( i = 0; i < IMAGE_PER_AREA; i++ ) {
		if ( (__xuiImageAddImage(pDemo, 0, i) != XUI_OK) ||
		     (__xuiImageAddImage(pDemo, 1, i) != XUI_OK) ||
		     (__xuiImageAddImage(pDemo, 2, i) != XUI_OK) ||
		     (__xuiImageAddImage(pDemo, 3, i) != XUI_OK) ) {
			return XUI_ERROR;
		}
	}
	(void)xuiImageSetMode(pDemo->pImage[(3 * IMAGE_PER_AREA) + 0], XUI_IMAGE_STRETCH);
	(void)xuiImageSetMode(pDemo->pImage[(3 * IMAGE_PER_AREA) + 1], XUI_IMAGE_CONTAIN);
	(void)xuiImageSetMode(pDemo->pImage[(3 * IMAGE_PER_AREA) + 2], XUI_IMAGE_COVER);
	(void)xuiImageSetMode(pDemo->pImage[(3 * IMAGE_PER_AREA) + 3], XUI_IMAGE_SCALE_DOWN);
	(void)xuiImageSetCustomRect(pDemo->pImage[(3 * IMAGE_PER_AREA) + 4], 14.0f, 10.0f, 74.0f, 52.0f);
	(void)xuiImageClearSource(pDemo->pImage[(3 * IMAGE_PER_AREA) + 5]);
	(void)xuiImageSetTint(pDemo->pImage[(3 * IMAGE_PER_AREA) + 6], XUI_COLOR_RGBA(120, 190, 255, 220));
	(void)xuiWidgetSetEnabled(pDemo->pImage[(3 * IMAGE_PER_AREA) + 7], 0);
	(void)xuiImageSetSurface(pDemo->pImage[(3 * IMAGE_PER_AREA) + 8], NULL);
	return XUI_OK;
}

static void __xuiImageLayout(xui_image_demo_t* pDemo)
{
	float fAreaW;
	float fAreaH;
	float fCellW;
	float fCellH;
	int i;

	(void)xuiWidgetSetRect(pDemo->pRoot, (xui_rect_t){0.0f, 0.0f, (float)DEMO_TARGET_W, (float)DEMO_TARGET_H});
	fAreaW = ((float)DEMO_TARGET_W - 36.0f - 12.0f) * 0.5f;
	fAreaH = ((float)DEMO_TARGET_H - 36.0f - 12.0f) * 0.5f;
	(void)xuiWidgetSetGridMetrics(pDemo->pRoot, 2, fAreaW, fAreaH);
	fCellW = (fAreaW - 20.0f - 16.0f) / 3.0f;
	fCellH = (fAreaH - 20.0f - 16.0f) / 3.0f;
	for ( i = 0; i < IMAGE_AREA_COUNT; i++ ) {
		(void)xuiWidgetSetGridMetrics(pDemo->pArea[i], 3, fCellW, fCellH);
	}
}

static void __xuiImageRunChecks(xui_image_demo_t* pDemo)
{
	xui_rect_t tArea;
	xui_rect_t tDraw;
	int i;

	pDemo->bCreateOK = (pDemo->pRoot != NULL);
	for ( i = 0; i < SURFACE_COUNT; i++ ) {
		if ( pDemo->pSurface[i] == NULL ) {
			pDemo->bCreateOK = 0;
		}
	}
	for ( i = 0; i < IMAGE_COUNT; i++ ) {
		if ( (pDemo->pCell[i] == NULL) || (pDemo->pImage[i] == NULL) ) {
			pDemo->bCreateOK = 0;
		}
	}
	tArea = xuiWidgetGetRect(pDemo->pArea[0]);
	pDemo->bLayoutOK = (tArea.fW > 300.0f) && (tArea.fH > 180.0f);
	tDraw = xuiImageGetDrawRect(pDemo->pImage[(3 * IMAGE_PER_AREA) + 4]);
	pDemo->bStateOK =
		(xuiImageGetMode(pDemo->pImage[0]) == XUI_IMAGE_NATURAL) &&
		(xuiImageGetSurface(pDemo->pImage[1]) == pDemo->pSurface[1]) &&
		(xuiImageGetSurface(pDemo->pImage[2]) == pDemo->pSurface[2]) &&
		(xuiImageGetMode(pDemo->pImage[(3 * IMAGE_PER_AREA) + 0]) == XUI_IMAGE_STRETCH) &&
		(xuiImageGetMode(pDemo->pImage[(3 * IMAGE_PER_AREA) + 4]) == XUI_IMAGE_CUSTOM) &&
		(tDraw.fW == 60.0f) &&
		(xuiImageGetColor(pDemo->pImage[(3 * IMAGE_PER_AREA) + 6]) == XUI_COLOR_RGBA(120, 190, 255, 220)) &&
		(xuiImageGetSurface(pDemo->pImage[(3 * IMAGE_PER_AREA) + 8]) == NULL);
}

static int __xuiImageCreateAssets(xui_image_demo_t* pDemo)
{
	xui_surface_desc_t tSurfaceDesc;
	int iRet;

	pDemo->tProxy = xuiProxyXge();
	iRet = xuiCreate(&pDemo->pContext);
	if ( iRet != XUI_OK ) {
		printf("xui_image: xuiCreate failed: %d\n", iRet);
		return iRet;
	}
	iRet = xuiSetProxy(pDemo->pContext, &pDemo->tProxy);
	if ( iRet != XUI_OK ) {
		printf("xui_image: xuiSetProxy failed: %d\n", iRet);
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
	iRet = __xuiImageCreateSurfaces(pDemo);
	if ( iRet != XUI_OK ) return iRet;
	iRet = __xuiImageCreateUi(pDemo);
	if ( iRet != XUI_OK ) {
		printf("xui_image: create UI failed: %d\n", iRet);
	}
	return iRet;
}

static void __xuiImageDestroyAssets(xui_image_demo_t* pDemo)
{
	int i;

	if ( pDemo->pContext != NULL ) {
		xuiDestroy(pDemo->pContext);
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

static int __xuiImageFrame(void* pUser)
{
	xui_image_demo_t* pDemo;
	xui_render_stats_t tStats;
	xui_cache_stats_t tCacheStats;
	xui_rect_i_t tFullRect;
	xui_rect_t tSrc;
	xui_rect_t tDst;
	int iRet;

	pDemo = (xui_image_demo_t*)pUser;
	if ( xgeKeyPressed(XGE_KEY_ESCAPE) ) {
		xgeQuit();
	}
	iRet = xgeBegin();
	if ( iRet != XGE_OK ) return iRet;
	__xuiImageLayout(pDemo);
	iRet = xuiUpdate(pDemo->pContext, xgeGetDelta());
	if ( iRet != XUI_OK ) return iRet;
	iRet = pDemo->tProxy.surfaceClear(&pDemo->tProxy, pDemo->pTarget, XUI_COLOR_RGBA(229, 235, 244, 255));
	if ( iRet != XUI_OK ) return iRet;
	tFullRect = (xui_rect_i_t){0, 0, DEMO_TARGET_W, DEMO_TARGET_H};
	iRet = xuiRender(pDemo->pContext, pDemo->pTarget, &tFullRect, 1);
	if ( iRet != XUI_OK ) return iRet;
	__xuiImageRunChecks(pDemo);
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
		printf("xui_image final-summary frames=%d create=%d layout=%d state=%d images=%d updatedCaches=%d drawnCaches=%d cacheSurfaces=%d\n",
			pDemo->iFrame,
			pDemo->bCreateOK,
			pDemo->bLayoutOK,
			pDemo->bStateOK,
			IMAGE_COUNT,
			tStats.iUpdatedCaches,
			tStats.iDrawnCaches,
			tCacheStats.iSurfaceCount);
		xgeQuit();
	}
	return XGE_OK;
}

int main(int argc, char** argv)
{
	xui_image_demo_t tDemo;
	xge_desc_t tDesc;
	int iRet;

	memset(&tDemo, 0, sizeof(tDemo));
	iRet = __xuiImageParseArgs(&tDemo, argc, argv);
	if ( iRet == 1 ) {
		return 0;
	}
	if ( iRet != XGE_OK ) {
		__xuiImageUsage();
		return 1;
	}
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iWidth = DEMO_TARGET_W + 20;
	tDesc.iHeight = DEMO_TARGET_H + 50;
	tDesc.sTitle = "XUI Image";
	tDesc.iFlags = XGE_INIT_WINDOW | XGE_INIT_VSYNC;
	tDesc.iRunMode = XGE_RUN_GAME_LOOP;
	tDesc.iTargetFPS = 60;
	iRet = xgeInit(&tDesc);
	if ( iRet != XGE_OK ) {
		printf("xui_image: xgeInit failed: %d\n", iRet);
		return 1;
	}
	iRet = __xuiImageCreateAssets(&tDemo);
	if ( iRet != XUI_OK ) {
		printf("xui_image: create assets failed: %d\n", iRet);
		__xuiImageDestroyAssets(&tDemo);
		xgeUnit();
		return 1;
	}
	iRet = xgeRun(__xuiImageFrame, &tDemo);
	__xuiImageDestroyAssets(&tDemo);
	xgeUnit();
	return (iRet == XGE_OK && tDemo.bCreateOK && tDemo.bLayoutOK && tDemo.bStateOK) ? 0 : 1;
}
