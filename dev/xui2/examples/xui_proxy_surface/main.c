#include "xui.h"
#include "xge.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEMO_SRC_W	128
#define DEMO_SRC_H	128
#define DEMO_TARGET_W	320
#define DEMO_TARGET_H	192

typedef struct xui_proxy_surface_demo_t {
	xui_proxy_t tProxy;
	xui_surface pFileSurface;
	xui_surface pMemorySurface;
	xui_surface pRgbaSurface;
	xui_surface pCompositeSurface;
	int iFrame;
	int iMaxFrames;
	double fMaxSeconds;
} xui_proxy_surface_demo_t;

static void __xuiProxySurfaceFillFilePixels(unsigned char* pPixels, int iWidth, int iHeight)
{
	int iX;
	int iY;
	int iIndex;
	int iStripe;

	for ( iY = 0; iY < iHeight; iY++ ) {
		for ( iX = 0; iX < iWidth; iX++ ) {
			iIndex = (iY * iWidth + iX) * 4;
			iStripe = ((iX + iY) / 14) & 1;
			pPixels[iIndex + 0] = (unsigned char)(iStripe ? 238 : 188);
			pPixels[iIndex + 1] = (unsigned char)(90 + (iX * 90 / iWidth));
			pPixels[iIndex + 2] = (unsigned char)(36 + (iY * 50 / iHeight));
			pPixels[iIndex + 3] = 255;
		}
	}
}

static void __xuiProxySurfaceFillMemoryPixels(unsigned char* pPixels, int iWidth, int iHeight)
{
	int iX;
	int iY;
	int iIndex;
	int iRing;
	int iDX;
	int iDY;

	for ( iY = 0; iY < iHeight; iY++ ) {
		for ( iX = 0; iX < iWidth; iX++ ) {
			iIndex = (iY * iWidth + iX) * 4;
			iDX = iX - (iWidth / 2);
			iDY = iY - (iHeight / 2);
			iRing = ((iDX * iDX + iDY * iDY) / 180) & 1;
			pPixels[iIndex + 0] = (unsigned char)(34 + (iY * 54 / iHeight));
			pPixels[iIndex + 1] = (unsigned char)(iRing ? 205 : 125);
			pPixels[iIndex + 2] = (unsigned char)(150 + (iX * 70 / iWidth));
			pPixels[iIndex + 3] = 255;
		}
	}
}

static void __xuiProxySurfaceFillRgbaPixels(unsigned char* pPixels, int iWidth, int iHeight)
{
	int iX;
	int iY;
	int iIndex;
	int bCell;
	unsigned char iAlpha;

	for ( iY = 0; iY < iHeight; iY++ ) {
		for ( iX = 0; iX < iWidth; iX++ ) {
			iIndex = (iY * iWidth + iX) * 4;
			bCell = (((iX / 16) + (iY / 16)) & 1);
			iAlpha = (unsigned char)(bCell ? 224 : 150);
			pPixels[iIndex + 0] = (unsigned char)((38u * iAlpha + 127u) / 255u);
			pPixels[iIndex + 1] = (unsigned char)((112u * iAlpha + 127u) / 255u);
			pPixels[iIndex + 2] = (unsigned char)((236u * iAlpha + 127u) / 255u);
			pPixels[iIndex + 3] = iAlpha;
		}
	}
}

static void __xuiProxySurfaceRect(float fX, float fY, float fW, float fH, xui_rect_t* pRect)
{
	pRect->fX = fX;
	pRect->fY = fY;
	pRect->fW = fW;
	pRect->fH = fH;
}

static int __xuiProxySurfaceFullPath(const char* sPath, char* sOut, int iCapacity)
{
	if ( (sPath == NULL) || (sOut == NULL) || (iCapacity <= 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
#if defined(_WIN32) || defined(_WIN64)
	if ( _fullpath(sOut, sPath, (size_t)iCapacity) == NULL ) {
		return XGE_ERROR_FILE_NOT_FOUND;
	}
#else
	if ( strlen(sPath) >= (size_t)iCapacity ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	strcpy(sOut, sPath);
#endif
	return XGE_OK;
}

static void __xuiProxySurfaceUsage(void)
{
	printf("usage: xui_proxy_surface [--frames N] [--seconds N]\n");
	printf("       no duration option means run until the window is closed.\n");
}

static int __xuiProxySurfaceParseArgs(xui_proxy_surface_demo_t* pDemo, int argc, char** argv)
{
	int i;

	if ( pDemo == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
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
			__xuiProxySurfaceUsage();
			return 1;
		} else {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
	}
	return XGE_OK;
}

static void __xuiProxySurfaceQuad(float fX, float fY, float fW, float fH, float fPhase, xui_surface_vertex_t* pVertices)
{
	float fSkew;
	float fCenterX;
	float fCenterY;
	float fRotation;
	float fCos;
	float fSin;
	float fDX;
	float fDY;
	int i;

	fSkew = sinf(fPhase) * 26.0f;
	pVertices[0].fX = fX + fSkew;
	pVertices[0].fY = fY;
	pVertices[0].fU = 0.0f;
	pVertices[0].fV = 0.0f;

	pVertices[1].fX = fX + fW;
	pVertices[1].fY = fY + 18.0f;
	pVertices[1].fU = 1.0f;
	pVertices[1].fV = 0.0f;

	pVertices[2].fX = fX;
	pVertices[2].fY = fY + fH - 12.0f;
	pVertices[2].fU = 0.0f;
	pVertices[2].fV = 1.0f;

	pVertices[3].fX = fX + fW - fSkew;
	pVertices[3].fY = fY + fH;
	pVertices[3].fU = 1.0f;
	pVertices[3].fV = 1.0f;

	pVertices[0].fZ = pVertices[1].fZ = pVertices[2].fZ = pVertices[3].fZ = 0.0f;
	pVertices[0].fW = pVertices[1].fW = pVertices[2].fW = pVertices[3].fW = 1.0f;
	pVertices[0].iColor = pVertices[1].iColor = pVertices[2].iColor = pVertices[3].iColor = XUI_COLOR_WHITE;

	fCenterX = fX + fW * 0.5f;
	fCenterY = fY + fH * 0.5f;
	fRotation = sinf(fPhase * 0.73f) * 0.16f;
	fCos = cosf(fRotation);
	fSin = sinf(fRotation);
	for ( i = 0; i < 4; i++ ) {
		fDX = pVertices[i].fX - fCenterX;
		fDY = pVertices[i].fY - fCenterY;
		pVertices[i].fX = fCenterX + (fDX * fCos) - (fDY * fSin);
		pVertices[i].fY = fCenterY + (fDX * fSin) + (fDY * fCos);
	}
}

static int __xuiProxySurfaceCreateAssets(xui_proxy_surface_demo_t* pDemo)
{
	unsigned char arrFilePixels[DEMO_SRC_W * DEMO_SRC_H * 4];
	unsigned char arrMemoryPixels[DEMO_SRC_W * DEMO_SRC_H * 4];
	unsigned char arrRgbaPixels[DEMO_SRC_W * DEMO_SRC_H * 4];
	xui_surface_desc_t tTargetDesc;
	xge_resource_t tMemoryResource;
	char arrFilePath[1024];
	char arrMemoryPath[1024];
	int iRet;

	__xuiProxySurfaceFillFilePixels(arrFilePixels, DEMO_SRC_W, DEMO_SRC_H);
	__xuiProxySurfaceFillMemoryPixels(arrMemoryPixels, DEMO_SRC_W, DEMO_SRC_H);
	__xuiProxySurfaceFillRgbaPixels(arrRgbaPixels, DEMO_SRC_W, DEMO_SRC_H);

	iRet = xgeImageSavePNG("xui_proxy_surface_file.png", DEMO_SRC_W, DEMO_SRC_H, arrFilePixels, DEMO_SRC_W * 4);
	if ( iRet != XGE_OK ) {
		printf("xui_proxy_surface: save file png failed: %d\n", iRet);
		return iRet;
	}
	iRet = xgeImageSavePNG("xui_proxy_surface_memory.png", DEMO_SRC_W, DEMO_SRC_H, arrMemoryPixels, DEMO_SRC_W * 4);
	if ( iRet != XGE_OK ) {
		printf("xui_proxy_surface: save memory png failed: %d\n", iRet);
		return iRet;
	}
	iRet = __xuiProxySurfaceFullPath("xui_proxy_surface_file.png", arrFilePath, (int)sizeof(arrFilePath));
	if ( iRet != XGE_OK ) {
		printf("xui_proxy_surface: file png full path failed: %d\n", iRet);
		return iRet;
	}
	iRet = __xuiProxySurfaceFullPath("xui_proxy_surface_memory.png", arrMemoryPath, (int)sizeof(arrMemoryPath));
	if ( iRet != XGE_OK ) {
		printf("xui_proxy_surface: memory png full path failed: %d\n", iRet);
		return iRet;
	}

	iRet = pDemo->tProxy.surfaceLoadFile(&pDemo->tProxy, &pDemo->pFileSurface, arrFilePath, XUI_SURFACE_ALPHA_PREMULTIPLIED);
	if ( iRet != XGE_OK ) {
		printf("xui_proxy_surface: surfaceLoadFile failed: %d\n", iRet);
		return iRet;
	}

	memset(&tMemoryResource, 0, sizeof(tMemoryResource));
	iRet = xgeResourceLoad(arrMemoryPath, &tMemoryResource);
	if ( iRet != XGE_OK ) {
		printf("xui_proxy_surface: xgeResourceLoad failed: %d\n", iRet);
		return iRet;
	}
	iRet = pDemo->tProxy.surfaceLoadMemory(&pDemo->tProxy, &pDemo->pMemorySurface, tMemoryResource.pData, tMemoryResource.iSize, XUI_SURFACE_ALPHA_PREMULTIPLIED);
	xgeResourceFree(&tMemoryResource);
	if ( iRet != XGE_OK ) {
		printf("xui_proxy_surface: surfaceLoadMemory failed: %d\n", iRet);
		return iRet;
	}

	iRet = pDemo->tProxy.surfaceCreateRGBA(&pDemo->tProxy, &pDemo->pRgbaSurface, DEMO_SRC_W, DEMO_SRC_H, arrRgbaPixels, DEMO_SRC_W * 4, XUI_SURFACE_ALPHA_PREMULTIPLIED);
	if ( iRet != XGE_OK ) {
		printf("xui_proxy_surface: surfaceCreateRGBA failed: %d\n", iRet);
		return iRet;
	}

	memset(&tTargetDesc, 0, sizeof(tTargetDesc));
	tTargetDesc.iKind = XUI_SURFACE_KIND_TEXTURE;
	tTargetDesc.iFormat = XUI_SURFACE_FORMAT_RGBA8;
	tTargetDesc.iWidth = DEMO_TARGET_W;
	tTargetDesc.iHeight = DEMO_TARGET_H;
	tTargetDesc.iFlags = XUI_SURFACE_ALPHA_PREMULTIPLIED | XUI_SURFACE_USAGE_TARGET;
	iRet = pDemo->tProxy.surfaceCreate(&pDemo->tProxy, &pDemo->pCompositeSurface, &tTargetDesc);
	if ( iRet != XGE_OK ) {
		printf("xui_proxy_surface: target surfaceCreate failed: %d\n", iRet);
	}
	return iRet;
}

static int __xuiProxySurfaceUpdateComposite(xui_proxy_surface_demo_t* pDemo)
{
	xui_rect_t tSrc;
	xui_rect_t tDst;
	xui_surface_vertex_t arrQuad[4];
	int iRet;

	__xuiProxySurfaceRect(0.0f, 0.0f, (float)DEMO_SRC_W, (float)DEMO_SRC_H, &tSrc);
	iRet = pDemo->tProxy.surfaceClear(&pDemo->tProxy, pDemo->pCompositeSurface, XUI_COLOR_RGBA(0, 0, 0, 0));
	if ( iRet != XGE_OK ) {
		return iRet;
	}

	__xuiProxySurfaceRect(0.0f, 0.0f, 190.0f, 190.0f, &tDst);
	iRet = pDemo->tProxy.surfaceDrawTo(&pDemo->tProxy, pDemo->pCompositeSurface, pDemo->pFileSurface, tSrc, tDst, XUI_COLOR_WHITE, 0);
	if ( iRet != XGE_OK ) {
		return iRet;
	}

	__xuiProxySurfaceRect(86.0f, 14.0f, 180.0f, 156.0f, &tDst);
	iRet = pDemo->tProxy.surfaceDrawTo(&pDemo->tProxy, pDemo->pCompositeSurface, pDemo->pMemorySurface, tSrc, tDst, XUI_COLOR_RGBA(255, 255, 255, 218), 0);
	if ( iRet != XGE_OK ) {
		return iRet;
	}

	__xuiProxySurfaceRect(184.0f, 28.0f, 112.0f, 112.0f, &tDst);
	iRet = pDemo->tProxy.surfaceDrawTo(&pDemo->tProxy, pDemo->pCompositeSurface, pDemo->pRgbaSurface, tSrc, tDst, XUI_COLOR_WHITE, 0);
	if ( iRet != XGE_OK ) {
		return iRet;
	}

	__xuiProxySurfaceQuad(76.0f, 82.0f, 140.0f, 78.0f, (float)pDemo->iFrame * 0.045f, arrQuad);
	return pDemo->tProxy.surfaceDrawQuadTo(&pDemo->tProxy, pDemo->pCompositeSurface, pDemo->pRgbaSurface, arrQuad, 0);
}

static int __xuiProxySurfaceFrame(void* pUser)
{
	xui_proxy_surface_demo_t* pDemo;
	xui_rect_t tSrc;
	xui_rect_t tDst;
	xui_surface_vertex_t arrQuad[4];
	int iRet;

	pDemo = (xui_proxy_surface_demo_t*)pUser;
	iRet = xgeBegin();
	if ( iRet != XGE_OK ) {
		return iRet;
	}

	iRet = __xuiProxySurfaceUpdateComposite(pDemo);
	if ( iRet != XGE_OK ) {
		return iRet;
	}

	xgeClear(XGE_COLOR_RGBA(18, 22, 26, 255));
	__xuiProxySurfaceRect(0.0f, 0.0f, (float)DEMO_SRC_W, (float)DEMO_SRC_H, &tSrc);

	__xuiProxySurfaceRect(30.0f, 30.0f, 128.0f, 128.0f, &tDst);
	iRet = pDemo->tProxy.surfaceDraw(&pDemo->tProxy, pDemo->pFileSurface, tSrc, tDst, XUI_COLOR_WHITE, XUI_SURFACE_DRAW_SCREEN_SPACE);
	if ( iRet != XGE_OK ) {
		return iRet;
	}

	__xuiProxySurfaceRect(182.0f, 30.0f, 128.0f, 128.0f, &tDst);
	iRet = pDemo->tProxy.surfaceDraw(&pDemo->tProxy, pDemo->pMemorySurface, tSrc, tDst, XUI_COLOR_WHITE, XUI_SURFACE_DRAW_SCREEN_SPACE);
	if ( iRet != XGE_OK ) {
		return iRet;
	}

	__xuiProxySurfaceRect(334.0f, 30.0f, 128.0f, 128.0f, &tDst);
	iRet = pDemo->tProxy.surfaceDraw(&pDemo->tProxy, pDemo->pRgbaSurface, tSrc, tDst, XUI_COLOR_WHITE, XUI_SURFACE_DRAW_SCREEN_SPACE);
	if ( iRet != XGE_OK ) {
		return iRet;
	}

	__xuiProxySurfaceRect(0.0f, 0.0f, (float)DEMO_TARGET_W, (float)DEMO_TARGET_H, &tSrc);
	__xuiProxySurfaceRect(30.0f, 194.0f, 320.0f, 192.0f, &tDst);
	iRet = pDemo->tProxy.surfaceDraw(&pDemo->tProxy, pDemo->pCompositeSurface, tSrc, tDst, XUI_COLOR_WHITE, XUI_SURFACE_DRAW_SCREEN_SPACE);
	if ( iRet != XGE_OK ) {
		return iRet;
	}

	__xuiProxySurfaceQuad(390.0f, 194.0f, 340.0f, 192.0f, (float)pDemo->iFrame * 0.035f, arrQuad);
	iRet = pDemo->tProxy.surfaceDrawQuad(&pDemo->tProxy, pDemo->pCompositeSurface, arrQuad, XUI_SURFACE_DRAW_SCREEN_SPACE);
	if ( iRet != XGE_OK ) {
		return iRet;
	}

	iRet = xgeEnd();
	if ( iRet != XGE_OK ) {
		return iRet;
	}

	pDemo->iFrame++;
	if ( ((pDemo->iMaxFrames > 0) && (pDemo->iFrame >= pDemo->iMaxFrames)) ||
	     ((pDemo->fMaxSeconds > 0.0) && (xgeTimer() >= pDemo->fMaxSeconds)) ) {
		xgeQuit();
	}
	return 0;
}

int main(int argc, char** argv)
{
	xui_proxy_surface_demo_t tDemo;
	xge_desc_t tDesc;
	int iRet;

	memset(&tDemo, 0, sizeof(tDemo));
	iRet = __xuiProxySurfaceParseArgs(&tDemo, argc, argv);
	if ( iRet == 1 ) {
		return 0;
	}
	if ( iRet != XGE_OK ) {
		__xuiProxySurfaceUsage();
		return 1;
	}

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iWidth = 760;
	tDesc.iHeight = 420;
	tDesc.sTitle = "xui_proxy_surface";
	tDesc.iFlags = XGE_INIT_VSYNC;
	tDesc.iRunMode = XGE_RUN_GAME_LOOP;

	iRet = xgeInit(&tDesc);
	if ( iRet != XGE_OK ) {
		printf("xui_proxy_surface: xgeInit failed: %d\n", iRet);
		return 1;
	}

	tDemo.tProxy = xuiProxyXge();
	iRet = __xuiProxySurfaceCreateAssets(&tDemo);
	if ( iRet != XGE_OK ) {
		printf("xui_proxy_surface: asset creation failed: %d\n", iRet);
		xgeUnit();
		return 1;
	}

	iRet = xgeRun(__xuiProxySurfaceFrame, &tDemo);
	tDemo.tProxy.surfaceDestroy(&tDemo.tProxy, tDemo.pCompositeSurface);
	tDemo.tProxy.surfaceDestroy(&tDemo.tProxy, tDemo.pRgbaSurface);
	tDemo.tProxy.surfaceDestroy(&tDemo.tProxy, tDemo.pMemorySurface);
	tDemo.tProxy.surfaceDestroy(&tDemo.tProxy, tDemo.pFileSurface);
	xgeUnit();
	return (iRet == XGE_OK) ? 0 : 1;
}
