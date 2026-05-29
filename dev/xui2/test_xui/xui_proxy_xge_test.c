#include "xui.h"
#include "xge.h"

#include <stdio.h>
#include <string.h>

static int __xuiTestStatusAllowed(int iRet)
{
	return (iRet == XGE_OK) || (iRet == XGE_ERROR_NOT_INITIALIZED) || (iRet == XGE_ERROR_UNSUPPORTED);
}

static void __xuiTestFillPixels(unsigned char* pPixels, int iWidth, int iHeight)
{
	int iX;
	int iY;
	int iIndex;

	for ( iY = 0; iY < iHeight; iY++ ) {
		for ( iX = 0; iX < iWidth; iX++ ) {
			iIndex = (iY * iWidth + iX) * 4;
			pPixels[iIndex + 0] = (unsigned char)(32 + iX * 16);
			pPixels[iIndex + 1] = (unsigned char)(64 + iY * 16);
			pPixels[iIndex + 2] = (unsigned char)(128 + iX + iY);
			pPixels[iIndex + 3] = 255;
		}
	}
}

static const char* __xuiTestFontPath(void)
{
	static const char* arrPaths[] = {
		"C:\\Windows\\Fonts\\arial.ttf",
		"C:\\Windows\\Fonts\\segoeui.ttf",
		"C:\\Windows\\Fonts\\simhei.ttf",
		"C:\\Windows\\Fonts\\simsun.ttc"
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

static int __xuiTestSurface(xui_proxy pProxy)
{
	xge_desc_t tXgeDesc;
	xge_resource_t tFontResource;
	xui_proxy_caps_t tCaps;
	xui_surface_desc_t tSurfaceDesc;
	xui_surface_desc_t tCreatedDesc;
	xui_surface_sampler_t tSampler;
	xui_font_metrics_t tMetrics;
	xui_rect_i_t tUpdateRect;
	xui_rect_t tSrc;
	xui_rect_t tDst;
	xui_rect_t tTopRect;
	xui_rect_t tBottomRect;
	xui_surface pSurface;
	xui_surface pCopySurface;
	xui_font pFont;
	xui_font pMemoryFont;
	xui_draw_context pDraw;
	xui_vec2_t tTextSize;
	const char* sFontPath;
	uint32_t iGeneration0;
	uint32_t iGeneration1;
	unsigned char arrPixels[4 * 4 * 4];
	unsigned char arrReadback[4 * 4 * 4];
	unsigned char arrPatch[2 * 2 * 4];
	int iRet;
	int iPixel;

	if ( (pProxy->getCaps == NULL) ||
	     (pProxy->surfaceCreate == NULL) || (pProxy->surfaceCreateRGBA == NULL) ||
	     (pProxy->surfaceLoadFile == NULL) || (pProxy->surfaceLoadMemory == NULL) ||
	     (pProxy->surfaceUpdateRGBA == NULL) || (pProxy->surfaceReadRGBA == NULL) ||
	     (pProxy->surfaceGetDesc == NULL) || (pProxy->surfaceDraw == NULL) ||
	     (pProxy->surfaceClear == NULL) || (pProxy->surfaceClearRect == NULL) ||
	     (pProxy->surfaceDrawTo == NULL) ||
	     (pProxy->surfaceDrawQuad == NULL) || (pProxy->surfaceDrawQuadTo == NULL) ||
	     (pProxy->surfaceGetSampler == NULL) || (pProxy->surfaceSetSampler == NULL) ||
	     (pProxy->surfaceGetGeneration == NULL) ||
	     (pProxy->surfaceDestroy == NULL) ||
	     (pProxy->shapePoint == NULL) || (pProxy->shapeLine == NULL) ||
	     (pProxy->shapeTriangleFill == NULL) || (pProxy->shapeTriangleStroke == NULL) ||
	     (pProxy->shapeRectFill == NULL) || (pProxy->shapeRectStroke == NULL) ||
	     (pProxy->shapeCircleFill == NULL) || (pProxy->shapeCircleStroke == NULL) ||
	     (pProxy->shapeRoundRectFill == NULL) || (pProxy->shapeRoundRectStroke == NULL) ||
	     (pProxy->fontLoadFile == NULL) || (pProxy->fontLoadMemory == NULL) ||
	     (pProxy->fontGetMetrics == NULL) || (pProxy->fontDestroy == NULL) ||
	     (pProxy->textMeasure == NULL) || (pProxy->textDraw == NULL) ||
	     (pProxy->drawBegin == NULL) || (pProxy->drawEnd == NULL) ||
	     (pProxy->drawClearRect == NULL) || (pProxy->drawSurface == NULL) ||
	     (pProxy->drawSurfaceQuad == NULL) || (pProxy->drawRectFill == NULL) ||
	     (pProxy->drawText == NULL) ) {
		printf("xui_proxy_xge_test failed: missing surface callbacks\n");
		return 1;
	}
	memset(&tCaps, 0, sizeof(tCaps));
	iRet = pProxy->getCaps(pProxy, &tCaps);
	if ( (iRet != XGE_OK) || (tCaps.iSurfaceFormat != XUI_SURFACE_FORMAT_RGBA8) ||
	     (tCaps.iInternalAlpha != XUI_SURFACE_ALPHA_PREMULTIPLIED) ||
	     ((tCaps.iCaps & (XUI_PROXY_CAP_SURFACE_TARGET | XUI_PROXY_CAP_DRAW_CONTEXT | XUI_PROXY_CAP_SURFACE_CLEAR_RECT | XUI_PROXY_CAP_TEXT)) !=
	      (XUI_PROXY_CAP_SURFACE_TARGET | XUI_PROXY_CAP_DRAW_CONTEXT | XUI_PROXY_CAP_SURFACE_CLEAR_RECT | XUI_PROXY_CAP_TEXT)) ) {
		printf("xui_proxy_xge_test failed: caps ret=%d caps=%u\n", iRet, (unsigned)tCaps.iCaps);
		return 1;
	}

	memset(&tXgeDesc, 0, sizeof(tXgeDesc));
	tXgeDesc.iWidth = 64;
	tXgeDesc.iHeight = 64;
	tXgeDesc.sTitle = "xui proxy surface test";
	tXgeDesc.iFlags = XGE_INIT_OFFSCREEN;
	tXgeDesc.iRunMode = XGE_RUN_MANUAL;
	iRet = xgeInit(&tXgeDesc);
	if ( iRet != XGE_OK ) {
		printf("xui_proxy_xge_test failed: xge init ret=%d\n", iRet);
		return 1;
	}

	memset(&tSurfaceDesc, 0, sizeof(tSurfaceDesc));
	tSurfaceDesc.iKind = XUI_SURFACE_KIND_TEXTURE;
	tSurfaceDesc.iFormat = XUI_SURFACE_FORMAT_RGBA8;
	tSurfaceDesc.iWidth = 4;
	tSurfaceDesc.iHeight = 4;
	tSurfaceDesc.iFlags = XUI_SURFACE_ALPHA_PREMULTIPLIED;
	pSurface = NULL;
	iRet = pProxy->surfaceCreate(pProxy, &pSurface, &tSurfaceDesc);
	if ( (iRet != XGE_OK) || (pSurface == NULL) ) {
		printf("xui_proxy_xge_test failed: surface create ret=%d\n", iRet);
		xgeUnit();
		return 1;
	}
	iRet = pProxy->surfaceGetDesc(pProxy, pSurface, &tCreatedDesc);
	if ( (iRet != XGE_OK) || (tCreatedDesc.iWidth != 4) || (tCreatedDesc.iHeight != 4) ||
	     (tCreatedDesc.iKind != XUI_SURFACE_KIND_TEXTURE) || (tCreatedDesc.iFormat != XUI_SURFACE_FORMAT_RGBA8) ) {
		printf("xui_proxy_xge_test failed: surface desc ret=%d width=%d height=%d\n", iRet, tCreatedDesc.iWidth, tCreatedDesc.iHeight);
		pProxy->surfaceDestroy(pProxy, pSurface);
		xgeUnit();
		return 1;
	}
	iRet = pProxy->surfaceGetGeneration(pProxy, pSurface, &iGeneration0);
	if ( (iRet != XGE_OK) || (iGeneration0 == 0) ) {
		printf("xui_proxy_xge_test failed: surface generation ret=%d generation=%u\n", iRet, (unsigned)iGeneration0);
		pProxy->surfaceDestroy(pProxy, pSurface);
		xgeUnit();
		return 1;
	}
	memset(&tSampler, 0, sizeof(tSampler));
	iRet = pProxy->surfaceGetSampler(pProxy, pSurface, &tSampler);
	if ( (iRet != XGE_OK) || (tSampler.iMinFilter != XUI_SURFACE_FILTER_LINEAR) ||
	     (tSampler.iMagFilter != XUI_SURFACE_FILTER_LINEAR) || (tSampler.iWrapS != XUI_SURFACE_WRAP_CLAMP) ||
	     (tSampler.iWrapT != XUI_SURFACE_WRAP_CLAMP) ) {
		printf("xui_proxy_xge_test failed: surface default sampler ret=%d\n", iRet);
		pProxy->surfaceDestroy(pProxy, pSurface);
		xgeUnit();
		return 1;
	}
	tSampler.iMinFilter = XUI_SURFACE_FILTER_NEAREST;
	tSampler.iMagFilter = XUI_SURFACE_FILTER_NEAREST;
	tSampler.iWrapS = XUI_SURFACE_WRAP_REPEAT;
	tSampler.iWrapT = XUI_SURFACE_WRAP_REPEAT;
	iRet = pProxy->surfaceSetSampler(pProxy, pSurface, &tSampler);
	if ( iRet != XGE_OK ) {
		printf("xui_proxy_xge_test failed: surface set sampler ret=%d\n", iRet);
		pProxy->surfaceDestroy(pProxy, pSurface);
		xgeUnit();
		return 1;
	}
	iRet = pProxy->surfaceGetGeneration(pProxy, pSurface, &iGeneration1);
	if ( (iRet != XGE_OK) || (iGeneration1 == iGeneration0) ) {
		printf("xui_proxy_xge_test failed: surface sampler generation ret=%d before=%u after=%u\n", iRet, (unsigned)iGeneration0, (unsigned)iGeneration1);
		pProxy->surfaceDestroy(pProxy, pSurface);
		xgeUnit();
		return 1;
	}
	pProxy->surfaceDestroy(pProxy, pSurface);

	tSurfaceDesc.iFlags = XUI_SURFACE_ALPHA_PREMULTIPLIED | XUI_SURFACE_USAGE_TARGET;
	pSurface = NULL;
	iRet = pProxy->surfaceCreate(pProxy, &pSurface, &tSurfaceDesc);
	if ( (iRet != XGE_OK) || (pSurface == NULL) ) {
		printf("xui_proxy_xge_test failed: target surface create ret=%d\n", iRet);
		xgeUnit();
		return 1;
	}
	iRet = pProxy->surfaceReadRGBA(pProxy, pSurface, arrReadback, 4 * 4);
	if ( iRet != XGE_OK ) {
		printf("xui_proxy_xge_test failed: target surface read ret=%d\n", iRet);
		pProxy->surfaceDestroy(pProxy, pSurface);
		xgeUnit();
		return 1;
	}
	iRet = pProxy->surfaceClear(pProxy, pSurface, XUI_COLOR_RGBA(0, 0, 0, 0));
	if ( __xuiTestStatusAllowed(iRet) == 0 ) {
		printf("xui_proxy_xge_test failed: target surface clear ret=%d\n", iRet);
		pProxy->surfaceDestroy(pProxy, pSurface);
		xgeUnit();
		return 1;
	}
	tSrc.fX = 0.0f;
	tSrc.fY = 0.0f;
	tSrc.fW = 4.0f;
	tSrc.fH = 4.0f;
	iRet = pProxy->shapeRectFill(pProxy, pSurface, tSrc, XUI_COLOR_RGBA(255, 0, 0, 255));
	if ( __xuiTestStatusAllowed(iRet) == 0 ) {
		printf("xui_proxy_xge_test failed: shape rect fill ret=%d\n", iRet);
		pProxy->surfaceDestroy(pProxy, pSurface);
		xgeUnit();
		return 1;
	}
	sFontPath = __xuiTestFontPath();
	if ( sFontPath != NULL ) {
		pFont = NULL;
		iRet = pProxy->fontLoadFile(pProxy, &pFont, sFontPath, 12.0f, XUI_FONT_FORMAT_TTF);
		if ( (iRet != XGE_OK) || (pFont == NULL) ) {
			printf("xui_proxy_xge_test failed: font load ret=%d\n", iRet);
			pProxy->surfaceDestroy(pProxy, pSurface);
			xgeUnit();
			return 1;
		}
		iRet = pProxy->fontGetMetrics(pProxy, pFont, &tMetrics);
		if ( (iRet != XGE_OK) || (tMetrics.fLineHeight <= 0.0f) ) {
			printf("xui_proxy_xge_test failed: font metrics ret=%d lineHeight=%f\n", iRet, tMetrics.fLineHeight);
			pProxy->fontDestroy(pProxy, pFont);
			pProxy->surfaceDestroy(pProxy, pSurface);
			xgeUnit();
			return 1;
		}
		iRet = pProxy->textMeasure(pProxy, pFont, "xui", &tTextSize);
		if ( (iRet != XGE_OK) || (tTextSize.fX <= 0.0f) || (tTextSize.fY <= 0.0f) ) {
			printf("xui_proxy_xge_test failed: text measure ret=%d size=%f,%f\n", iRet, tTextSize.fX, tTextSize.fY);
			pProxy->fontDestroy(pProxy, pFont);
			pProxy->surfaceDestroy(pProxy, pSurface);
			xgeUnit();
			return 1;
		}
		iRet = pProxy->textDraw(pProxy, pSurface, pFont, "xui", tSrc, XUI_COLOR_WHITE, XUI_TEXT_ALIGN_CENTER | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP);
		if ( __xuiTestStatusAllowed(iRet) == 0 ) {
			printf("xui_proxy_xge_test failed: text draw ret=%d\n", iRet);
			pProxy->fontDestroy(pProxy, pFont);
			pProxy->surfaceDestroy(pProxy, pSurface);
			xgeUnit();
			return 1;
		}
		memset(&tFontResource, 0, sizeof(tFontResource));
		iRet = xgeResourceLoad(sFontPath, &tFontResource);
		if ( iRet != XGE_OK ) {
			printf("xui_proxy_xge_test failed: font resource load ret=%d\n", iRet);
			pProxy->fontDestroy(pProxy, pFont);
			pProxy->surfaceDestroy(pProxy, pSurface);
			xgeUnit();
			return 1;
		}
		pMemoryFont = NULL;
		iRet = pProxy->fontLoadMemory(pProxy, &pMemoryFont, tFontResource.pData, tFontResource.iSize, 10.0f, XUI_FONT_FORMAT_TTF);
		xgeResourceFree(&tFontResource);
		if ( (iRet != XGE_OK) || (pMemoryFont == NULL) ) {
			printf("xui_proxy_xge_test failed: font memory load ret=%d\n", iRet);
			pProxy->fontDestroy(pProxy, pFont);
			pProxy->surfaceDestroy(pProxy, pSurface);
			xgeUnit();
			return 1;
		}
		pProxy->fontDestroy(pProxy, pMemoryFont);
		pProxy->fontDestroy(pProxy, pFont);
	}
	pProxy->surfaceDestroy(pProxy, pSurface);

	pSurface = NULL;
	pCopySurface = NULL;
	iRet = pProxy->surfaceCreate(pProxy, &pSurface, &tSurfaceDesc);
	if ( (iRet != XGE_OK) || (pSurface == NULL) ) {
		printf("xui_proxy_xge_test failed: source target create ret=%d\n", iRet);
		xgeUnit();
		return 1;
	}
	iRet = pProxy->surfaceCreate(pProxy, &pCopySurface, &tSurfaceDesc);
	if ( (iRet != XGE_OK) || (pCopySurface == NULL) ) {
		printf("xui_proxy_xge_test failed: copy target create ret=%d\n", iRet);
		pProxy->surfaceDestroy(pProxy, pSurface);
		xgeUnit();
		return 1;
	}
	tTopRect.fX = 0.0f;
	tTopRect.fY = 0.0f;
	tTopRect.fW = 4.0f;
	tTopRect.fH = 2.0f;
	tBottomRect.fX = 0.0f;
	tBottomRect.fY = 2.0f;
	tBottomRect.fW = 4.0f;
	tBottomRect.fH = 2.0f;
	iRet = pProxy->surfaceClear(pProxy, pSurface, XUI_COLOR_RGBA(0, 0, 0, 0));
	if ( iRet == XGE_OK ) {
		iRet = pProxy->shapeRectFill(pProxy, pSurface, tTopRect, XUI_COLOR_RGBA(255, 0, 0, 255));
	}
	if ( iRet == XGE_OK ) {
		iRet = pProxy->shapeRectFill(pProxy, pSurface, tBottomRect, XUI_COLOR_RGBA(0, 0, 255, 255));
	}
	if ( iRet == XGE_OK ) {
		iRet = pProxy->surfaceClear(pProxy, pCopySurface, XUI_COLOR_RGBA(0, 0, 0, 0));
	}
	if ( iRet == XGE_OK ) {
		iRet = pProxy->surfaceDrawTo(pProxy, pCopySurface, pSurface, tSrc, tSrc, XUI_COLOR_WHITE, 0);
	}
	if ( iRet == XGE_OK ) {
		memset(arrReadback, 0, sizeof(arrReadback));
		iRet = pProxy->surfaceReadRGBA(pProxy, pCopySurface, arrReadback, 4 * 4);
	}
	if ( iRet == XGE_OK ) {
		if ( (arrReadback[0] != 255) || (arrReadback[1] != 0) || (arrReadback[2] != 0) || (arrReadback[3] != 255) ||
		     (arrReadback[((3 * 4) + 0) * 4 + 0] != 0) || (arrReadback[((3 * 4) + 0) * 4 + 1] != 0) ||
		     (arrReadback[((3 * 4) + 0) * 4 + 2] != 255) || (arrReadback[((3 * 4) + 0) * 4 + 3] != 255) ) {
			printf("xui_proxy_xge_test failed: target source orientation top=%u,%u,%u,%u bottom=%u,%u,%u,%u\n",
				(unsigned)arrReadback[0], (unsigned)arrReadback[1], (unsigned)arrReadback[2], (unsigned)arrReadback[3],
				(unsigned)arrReadback[((3 * 4) + 0) * 4 + 0], (unsigned)arrReadback[((3 * 4) + 0) * 4 + 1],
				(unsigned)arrReadback[((3 * 4) + 0) * 4 + 2], (unsigned)arrReadback[((3 * 4) + 0) * 4 + 3]);
			pProxy->surfaceDestroy(pProxy, pCopySurface);
			pProxy->surfaceDestroy(pProxy, pSurface);
			xgeUnit();
			return 1;
		}
	} else if ( __xuiTestStatusAllowed(iRet) == 0 ) {
		printf("xui_proxy_xge_test failed: target source orientation ret=%d\n", iRet);
		pProxy->surfaceDestroy(pProxy, pCopySurface);
		pProxy->surfaceDestroy(pProxy, pSurface);
		xgeUnit();
		return 1;
	}
	iRet = pProxy->surfaceClear(pProxy, pCopySurface, XUI_COLOR_RGBA(0, 0, 0, 255));
	if ( iRet == XGE_OK ) {
		iRet = pProxy->surfaceClearRect(pProxy, pCopySurface, tTopRect, XUI_COLOR_RGBA(255, 0, 0, 255));
	}
	if ( iRet == XGE_OK ) {
		memset(arrReadback, 0, sizeof(arrReadback));
		iRet = pProxy->surfaceReadRGBA(pProxy, pCopySurface, arrReadback, 4 * 4);
	}
	if ( iRet == XGE_OK ) {
		if ( (arrReadback[0] != 255) || (arrReadback[1] != 0) || (arrReadback[2] != 0) || (arrReadback[3] != 255) ||
		     (arrReadback[((3 * 4) + 0) * 4 + 0] != 0) || (arrReadback[((3 * 4) + 0) * 4 + 1] != 0) ||
		     (arrReadback[((3 * 4) + 0) * 4 + 2] != 0) || (arrReadback[((3 * 4) + 0) * 4 + 3] != 255) ) {
			printf("xui_proxy_xge_test failed: clear rect readback\n");
			pProxy->surfaceDestroy(pProxy, pCopySurface);
			pProxy->surfaceDestroy(pProxy, pSurface);
			xgeUnit();
			return 1;
		}
	} else if ( __xuiTestStatusAllowed(iRet) == 0 ) {
		printf("xui_proxy_xge_test failed: clear rect ret=%d\n", iRet);
		pProxy->surfaceDestroy(pProxy, pCopySurface);
		pProxy->surfaceDestroy(pProxy, pSurface);
		xgeUnit();
		return 1;
	}
	iRet = pProxy->surfaceGetGeneration(pProxy, pCopySurface, &iGeneration0);
	if ( iRet != XGE_OK ) {
		printf("xui_proxy_xge_test failed: draw generation before ret=%d\n", iRet);
		pProxy->surfaceDestroy(pProxy, pCopySurface);
		pProxy->surfaceDestroy(pProxy, pSurface);
		xgeUnit();
		return 1;
	}
	pDraw = NULL;
	iRet = pProxy->drawBegin(pProxy, &pDraw, pCopySurface);
	if ( iRet == XGE_OK ) {
		iRet = pProxy->drawClearRect(pProxy, pDraw, tSrc, XUI_COLOR_RGBA(0, 0, 0, 255));
	}
	if ( iRet == XGE_OK ) {
		iRet = pProxy->drawRectFill(pProxy, pDraw, tBottomRect, XUI_COLOR_RGBA(0, 0, 255, 255));
	}
	if ( pDraw != NULL ) {
		iRet = pProxy->drawEnd(pProxy, pDraw);
		pDraw = NULL;
	}
	if ( iRet == XGE_OK ) {
		iRet = pProxy->surfaceGetGeneration(pProxy, pCopySurface, &iGeneration1);
	}
	if ( iRet == XGE_OK ) {
		memset(arrReadback, 0, sizeof(arrReadback));
		iRet = pProxy->surfaceReadRGBA(pProxy, pCopySurface, arrReadback, 4 * 4);
	}
	if ( iRet == XGE_OK ) {
		if ( (iGeneration1 == iGeneration0) ||
		     (arrReadback[0] != 0) || (arrReadback[1] != 0) || (arrReadback[2] != 0) || (arrReadback[3] != 255) ||
		     (arrReadback[((3 * 4) + 0) * 4 + 0] != 0) || (arrReadback[((3 * 4) + 0) * 4 + 1] != 0) ||
		     (arrReadback[((3 * 4) + 0) * 4 + 2] != 255) || (arrReadback[((3 * 4) + 0) * 4 + 3] != 255) ) {
			printf("xui_proxy_xge_test failed: draw context readback generation %u -> %u\n", (unsigned)iGeneration0, (unsigned)iGeneration1);
			pProxy->surfaceDestroy(pProxy, pCopySurface);
			pProxy->surfaceDestroy(pProxy, pSurface);
			xgeUnit();
			return 1;
		}
	} else if ( __xuiTestStatusAllowed(iRet) == 0 ) {
		printf("xui_proxy_xge_test failed: draw context ret=%d\n", iRet);
		pProxy->surfaceDestroy(pProxy, pCopySurface);
		pProxy->surfaceDestroy(pProxy, pSurface);
		xgeUnit();
		return 1;
	}
	pProxy->surfaceDestroy(pProxy, pCopySurface);
	pProxy->surfaceDestroy(pProxy, pSurface);

	__xuiTestFillPixels(arrPixels, 4, 4);
	pSurface = NULL;
	iRet = pProxy->surfaceCreateRGBA(pProxy, &pSurface, 4, 4, arrPixels, 4 * 4, XUI_SURFACE_ALPHA_PREMULTIPLIED);
	if ( (iRet != XGE_OK) || (pSurface == NULL) ) {
		printf("xui_proxy_xge_test failed: surface create rgba ret=%d\n", iRet);
		xgeUnit();
		return 1;
	}
	memset(arrReadback, 0, sizeof(arrReadback));
	iRet = pProxy->surfaceReadRGBA(pProxy, pSurface, arrReadback, 4 * 4);
	if ( (iRet != XGE_OK) || (memcmp(arrReadback, arrPixels, sizeof(arrPixels)) != 0) ) {
		printf("xui_proxy_xge_test failed: surface readback ret=%d\n", iRet);
		pProxy->surfaceDestroy(pProxy, pSurface);
		xgeUnit();
		return 1;
	}

	for ( iPixel = 0; iPixel < 4; iPixel++ ) {
		arrPatch[(iPixel * 4) + 0] = 220;
		arrPatch[(iPixel * 4) + 1] = 32;
		arrPatch[(iPixel * 4) + 2] = 48;
		arrPatch[(iPixel * 4) + 3] = 255;
	}
	tUpdateRect.iX = 1;
	tUpdateRect.iY = 1;
	tUpdateRect.iW = 2;
	tUpdateRect.iH = 2;
	iRet = pProxy->surfaceUpdateRGBA(pProxy, pSurface, tUpdateRect, arrPatch, 2 * 4);
	if ( iRet != XGE_OK ) {
		printf("xui_proxy_xge_test failed: surface update ret=%d\n", iRet);
		pProxy->surfaceDestroy(pProxy, pSurface);
		xgeUnit();
		return 1;
	}
	memset(arrReadback, 0, sizeof(arrReadback));
	iRet = pProxy->surfaceReadRGBA(pProxy, pSurface, arrReadback, 4 * 4);
	iPixel = ((1 * 4) + 1) * 4;
	if ( (iRet != XGE_OK) || (arrReadback[iPixel + 0] != 220) || (arrReadback[iPixel + 1] != 32) ||
	     (arrReadback[iPixel + 2] != 48) || (arrReadback[iPixel + 3] != 255) ) {
		printf("xui_proxy_xge_test failed: surface update readback ret=%d\n", iRet);
		pProxy->surfaceDestroy(pProxy, pSurface);
		xgeUnit();
		return 1;
	}

	memset(&tSrc, 0, sizeof(tSrc));
	tDst.fX = 2.0f;
	tDst.fY = 2.0f;
	tDst.fW = 16.0f;
	tDst.fH = 16.0f;
	iRet = xgeBegin();
	if ( iRet == XGE_OK ) {
		iRet = pProxy->surfaceDraw(pProxy, pSurface, tSrc, tDst, XUI_COLOR_WHITE, XUI_SURFACE_DRAW_SCREEN_SPACE);
		if ( iRet == XGE_OK ) {
			iRet = xgeEnd();
		}
	}
	if ( iRet != XGE_OK ) {
		printf("xui_proxy_xge_test failed: surface draw ret=%d\n", iRet);
		pProxy->surfaceDestroy(pProxy, pSurface);
		xgeUnit();
		return 1;
	}

	pProxy->surfaceDestroy(pProxy, pSurface);

	arrPixels[0] = 100;
	arrPixels[1] = 50;
	arrPixels[2] = 25;
	arrPixels[3] = 128;
	pSurface = NULL;
	iRet = pProxy->surfaceCreateRGBA(pProxy, &pSurface, 1, 1, arrPixels, 4, XUI_SURFACE_ALPHA_STRAIGHT);
	if ( (iRet != XGE_OK) || (pSurface == NULL) ) {
		printf("xui_proxy_xge_test failed: straight alpha surface create ret=%d\n", iRet);
		xgeUnit();
		return 1;
	}
	memset(arrReadback, 0, sizeof(arrReadback));
	iRet = pProxy->surfaceReadRGBA(pProxy, pSurface, arrReadback, 4);
	if ( (iRet != XGE_OK) || (arrReadback[0] != 50) || (arrReadback[1] != 25) ||
	     (arrReadback[2] != 13) || (arrReadback[3] != 128) ) {
		printf("xui_proxy_xge_test failed: straight alpha premultiply ret=%d pixel=%u,%u,%u,%u\n", iRet,
			(unsigned)arrReadback[0], (unsigned)arrReadback[1], (unsigned)arrReadback[2], (unsigned)arrReadback[3]);
		pProxy->surfaceDestroy(pProxy, pSurface);
		xgeUnit();
		return 1;
	}
	pProxy->surfaceDestroy(pProxy, pSurface);
	xgeUnit();
	return 0;
}

int main(void)
{
	xui_proxy_t tProxy;
	xui_proxy_caps_t tCaps;
	xui_rect_t tImeRect;
	char arrText[64];
	int iRet;
	int bImeEnabled;

	tProxy = xuiProxyXge();
	if ( (tProxy.iSize != sizeof(tProxy)) || (tProxy.iVersion != XUI_PROXY_VERSION) ) {
		printf("xui_proxy_xge_test failed: bad proxy header size=%u version=%u\n", (unsigned)tProxy.iSize, (unsigned)tProxy.iVersion);
		return 1;
	}
	if ( (tProxy.getCaps == NULL) ||
	     (tProxy.clipboardSetText == NULL) || (tProxy.clipboardGetText == NULL) ||
	     (tProxy.imeGetEnabled == NULL) || (tProxy.imeSetEnabled == NULL) ||
	     (tProxy.imeSetCandidateRect == NULL) ||
	     (tProxy.surfaceClearRect == NULL) || (tProxy.surfaceGetSampler == NULL) ||
	     (tProxy.surfaceSetSampler == NULL) || (tProxy.surfaceGetGeneration == NULL) ||
	     (tProxy.fontLoadFile == NULL) || (tProxy.fontLoadMemory == NULL) ||
	     (tProxy.fontGetMetrics == NULL) || (tProxy.fontDestroy == NULL) ||
	     (tProxy.textMeasure == NULL) || (tProxy.textDraw == NULL) ||
	     (tProxy.drawBegin == NULL) || (tProxy.drawEnd == NULL) ||
	     (tProxy.drawClearRect == NULL) || (tProxy.drawRectFill == NULL) ||
	     (tProxy.drawText == NULL) ) {
		printf("xui_proxy_xge_test failed: missing callbacks\n");
		return 1;
	}
	memset(&tCaps, 0, sizeof(tCaps));
	iRet = tProxy.getCaps(&tProxy, &tCaps);
	if ( (iRet != XGE_OK) || (tCaps.iSize != sizeof(tCaps)) ||
	     (tCaps.iSurfaceFormat != XUI_SURFACE_FORMAT_RGBA8) ||
	     (tCaps.iInternalAlpha != XUI_SURFACE_ALPHA_PREMULTIPLIED) ) {
		printf("xui_proxy_xge_test failed: caps header ret=%d\n", iRet);
		return 1;
	}

	if ( tProxy.clipboardGetText(&tProxy, NULL, (int)sizeof(arrText)) != XGE_ERROR_INVALID_ARGUMENT ||
	     tProxy.clipboardGetText(&tProxy, arrText, 0) != XGE_ERROR_INVALID_ARGUMENT ||
	     tProxy.clipboardGetText(NULL, arrText, (int)sizeof(arrText)) != XGE_ERROR_INVALID_ARGUMENT ) {
		printf("xui_proxy_xge_test failed: clipboard validation\n");
		return 1;
	}
	if ( tProxy.clipboardSetText(&tProxy, "xui proxy clipboard smoke") != XGE_OK ) {
		printf("xui_proxy_xge_test failed: clipboard set\n");
		return 1;
	}
	memset(arrText, 'x', sizeof(arrText));
	iRet = tProxy.clipboardGetText(&tProxy, arrText, (int)sizeof(arrText));
	if ( (iRet < 0) || (arrText[sizeof(arrText) - 1u] != 0 && arrText[sizeof(arrText) - 1u] != 'x') ) {
		printf("xui_proxy_xge_test failed: clipboard get ret=%d\n", iRet);
		return 1;
	}

	bImeEnabled = tProxy.imeGetEnabled(&tProxy);
	if ( (bImeEnabled != 0) && (bImeEnabled != 1) ) {
		printf("xui_proxy_xge_test failed: ime get=%d\n", bImeEnabled);
		return 1;
	}
	iRet = tProxy.imeSetEnabled(&tProxy, bImeEnabled);
	if ( __xuiTestStatusAllowed(iRet) == 0 ) {
		printf("xui_proxy_xge_test failed: ime set ret=%d\n", iRet);
		return 1;
	}

	tImeRect.fX = 12.0f;
	tImeRect.fY = 24.0f;
	tImeRect.fW = 96.0f;
	tImeRect.fH = 20.0f;
	iRet = tProxy.imeSetCandidateRect(&tProxy, tImeRect);
	if ( __xuiTestStatusAllowed(iRet) == 0 ) {
		printf("xui_proxy_xge_test failed: ime candidate rect ret=%d\n", iRet);
		return 1;
	}
	if ( __xuiTestSurface(&tProxy) != 0 ) {
		return 1;
	}

	printf("xui_proxy_xge_test passed clipboard/ime/surface smoke\n");
	return 0;
}
