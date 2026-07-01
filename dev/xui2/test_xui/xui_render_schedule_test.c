#include "xui.h"

#include <stdio.h>
#include <string.h>

#define XUI_TEST_CHECK(expr, msg) \
	do { \
		if ( !(expr) ) { \
			printf("xui_render_schedule_test failed: %s\n", (msg)); \
			iFailed = 1; \
			goto cleanup; \
		} \
	} while ( 0 )

#define XUI_TEST_SURFACE_MAGIC 0x54534653u
#define XUI_TEST_DRAW_MAGIC 0x54445257u

struct xui_surface_t {
	uint32_t iMagic;
	xui_surface_desc_t tDesc;
	uint32_t iGeneration;
	unsigned char* pPixels;
};

struct xui_draw_context_t {
	uint32_t iMagic;
	xui_surface pTarget;
};

struct xui_font_t {
	int iUnused;
};

typedef struct xui_render_schedule_test_state_t {
	xui_proxy_t tProxy;
	int iRenderCount;
	int iResourceDestroyed;
} xui_render_schedule_test_state_t;

static void __xuiTestResourceDestroy(xui_context pContext, void* pHandle, void* pUser)
{
	xui_render_schedule_test_state_t* pState;

	(void)pContext;
	(void)pHandle;
	pState = (xui_render_schedule_test_state_t*)pUser;
	if ( pState != NULL ) {
		pState->iResourceDestroyed++;
	}
}

static int __xuiTestSurfaceValid(xui_surface pSurface)
{
	return (pSurface != NULL) && (pSurface->iMagic == XUI_TEST_SURFACE_MAGIC) && (pSurface->pPixels != NULL);
}

static int __xuiTestDrawValid(xui_draw_context pDraw)
{
	return (pDraw != NULL) && (pDraw->iMagic == XUI_TEST_DRAW_MAGIC) && __xuiTestSurfaceValid(pDraw->pTarget);
}

static void __xuiTestColorWrite(unsigned char* pPixel, uint32_t iColor)
{
	pPixel[0] = (unsigned char)((iColor >> 24) & 0xFFu);
	pPixel[1] = (unsigned char)((iColor >> 16) & 0xFFu);
	pPixel[2] = (unsigned char)((iColor >> 8) & 0xFFu);
	pPixel[3] = (unsigned char)(iColor & 0xFFu);
}

static int __xuiTestSurfaceFillRect(xui_surface pTarget, xui_rect_t tRect, uint32_t iColor)
{
	int iLeft;
	int iTop;
	int iRight;
	int iBottom;
	int x;
	int y;

	if ( !__xuiTestSurfaceValid(pTarget) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	iLeft = (int)tRect.fX;
	iTop = (int)tRect.fY;
	iRight = (int)(tRect.fX + tRect.fW);
	iBottom = (int)(tRect.fY + tRect.fH);
	if ( iLeft < 0 ) {
		iLeft = 0;
	}
	if ( iTop < 0 ) {
		iTop = 0;
	}
	if ( iRight > pTarget->tDesc.iWidth ) {
		iRight = pTarget->tDesc.iWidth;
	}
	if ( iBottom > pTarget->tDesc.iHeight ) {
		iBottom = pTarget->tDesc.iHeight;
	}
	for ( y = iTop; y < iBottom; y++ ) {
		for ( x = iLeft; x < iRight; x++ ) {
			__xuiTestColorWrite(pTarget->pPixels + (((y * pTarget->tDesc.iWidth) + x) * 4), iColor);
		}
	}
	pTarget->iGeneration++;
	return XUI_OK;
}

static int __xuiTestSurfaceBlit(xui_surface pTarget, xui_surface pSurface, xui_rect_t tSrc, xui_rect_t tDst)
{
	int iSrcX;
	int iSrcY;
	int iDstX;
	int iDstY;
	int iWidth;
	int iHeight;
	int x;
	int y;

	if ( !__xuiTestSurfaceValid(pTarget) || !__xuiTestSurfaceValid(pSurface) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	iSrcX = (int)tSrc.fX;
	iSrcY = (int)tSrc.fY;
	iDstX = (int)tDst.fX;
	iDstY = (int)tDst.fY;
	iWidth = (int)tDst.fW;
	iHeight = (int)tDst.fH;
	if ( iWidth > (int)tSrc.fW ) {
		iWidth = (int)tSrc.fW;
	}
	if ( iHeight > (int)tSrc.fH ) {
		iHeight = (int)tSrc.fH;
	}
	for ( y = 0; y < iHeight; y++ ) {
		if ( (iDstY + y < 0) || (iDstY + y >= pTarget->tDesc.iHeight) ||
		     (iSrcY + y < 0) || (iSrcY + y >= pSurface->tDesc.iHeight) ) {
			continue;
		}
		for ( x = 0; x < iWidth; x++ ) {
			if ( (iDstX + x < 0) || (iDstX + x >= pTarget->tDesc.iWidth) ||
			     (iSrcX + x < 0) || (iSrcX + x >= pSurface->tDesc.iWidth) ) {
				continue;
			}
			memcpy(pTarget->pPixels + ((((iDstY + y) * pTarget->tDesc.iWidth) + (iDstX + x)) * 4),
			       pSurface->pPixels + ((((iSrcY + y) * pSurface->tDesc.iWidth) + (iSrcX + x)) * 4),
			       4);
		}
	}
	pTarget->iGeneration++;
	return XUI_OK;
}

static int __xuiTestGetCaps(xui_proxy pProxy, xui_proxy_caps_t* pCaps)
{
	(void)pProxy;
	memset(pCaps, 0, sizeof(*pCaps));
	pCaps->iSize = sizeof(*pCaps);
	pCaps->iCaps = XUI_PROXY_CAP_SURFACE_TARGET | XUI_PROXY_CAP_SURFACE_READ |
	               XUI_PROXY_CAP_SURFACE_QUAD | XUI_PROXY_CAP_SURFACE_CLEAR_RECT |
	               XUI_PROXY_CAP_SURFACE_SAMPLER | XUI_PROXY_CAP_DRAW_CONTEXT |
	               XUI_PROXY_CAP_SHAPE | XUI_PROXY_CAP_FONT_TTF |
	               XUI_PROXY_CAP_FONT_XRF | XUI_PROXY_CAP_TEXT;
	pCaps->iSurfaceFormat = XUI_SURFACE_FORMAT_RGBA8;
	pCaps->iInternalAlpha = XUI_SURFACE_ALPHA_PREMULTIPLIED;
	pCaps->tDefaultSampler.iMinFilter = XUI_SURFACE_FILTER_NEAREST;
	pCaps->tDefaultSampler.iMagFilter = XUI_SURFACE_FILTER_NEAREST;
	pCaps->tDefaultSampler.iWrapS = XUI_SURFACE_WRAP_CLAMP;
	pCaps->tDefaultSampler.iWrapT = XUI_SURFACE_WRAP_CLAMP;
	return XUI_OK;
}

static int __xuiTestClipboardSetText(xui_proxy pProxy, const char* sText)
{
	(void)pProxy;
	(void)sText;
	return XUI_OK;
}

static int __xuiTestClipboardGetText(xui_proxy pProxy, char* sText, int iCapacity)
{
	(void)pProxy;
	if ( (sText != NULL) && (iCapacity > 0) ) {
		sText[0] = '\0';
	}
	return 0;
}

static int __xuiTestImeGetEnabled(xui_proxy pProxy)
{
	(void)pProxy;
	return 0;
}

static int __xuiTestImeSetEnabled(xui_proxy pProxy, int bEnabled)
{
	(void)pProxy;
	(void)bEnabled;
	return XUI_OK;
}

static int __xuiTestImeSetCandidateRect(xui_proxy pProxy, xui_rect_t tRect)
{
	(void)pProxy;
	(void)tRect;
	return XUI_OK;
}

static int __xuiTestSurfaceCreate(xui_proxy pProxy, xui_surface* ppSurface, const xui_surface_desc_t* pDesc)
{
	xui_surface pSurface;
	size_t iBytes;

	(void)pProxy;
	if ( (ppSurface == NULL) || (pDesc == NULL) || (pDesc->iWidth <= 0) || (pDesc->iHeight <= 0) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	*ppSurface = NULL;
	pSurface = (xui_surface)xrtCalloc(1, sizeof(*pSurface));
	if ( pSurface == NULL ) {
		return XUI_ERROR_OUT_OF_MEMORY;
	}
	iBytes = (size_t)pDesc->iWidth * (size_t)pDesc->iHeight * 4u;
	pSurface->pPixels = (unsigned char*)xrtCalloc(iBytes, 1);
	if ( pSurface->pPixels == NULL ) {
		xrtFree(pSurface);
		return XUI_ERROR_OUT_OF_MEMORY;
	}
	pSurface->iMagic = XUI_TEST_SURFACE_MAGIC;
	pSurface->tDesc = *pDesc;
	pSurface->iGeneration = 1;
	*ppSurface = pSurface;
	return XUI_OK;
}

static int __xuiTestSurfaceCreateRGBA(xui_proxy pProxy, xui_surface* ppSurface, int iWidth, int iHeight, const void* pPixels, int iStride, uint32_t iFlags)
{
	xui_surface_desc_t tDesc;
	int iRet;
	int y;

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iKind = XUI_SURFACE_KIND_TEXTURE;
	tDesc.iFormat = XUI_SURFACE_FORMAT_RGBA8;
	tDesc.iWidth = iWidth;
	tDesc.iHeight = iHeight;
	tDesc.iFlags = iFlags;
	iRet = __xuiTestSurfaceCreate(pProxy, ppSurface, &tDesc);
	if ( (iRet != XUI_OK) || (pPixels == NULL) ) {
		return iRet;
	}
	for ( y = 0; y < iHeight; y++ ) {
		memcpy((*ppSurface)->pPixels + ((size_t)y * (size_t)iWidth * 4u),
		       (const unsigned char*)pPixels + ((size_t)y * (size_t)iStride),
		       (size_t)iWidth * 4u);
	}
	return XUI_OK;
}

static int __xuiTestSurfaceUnsupported(xui_proxy pProxy, xui_surface* ppSurface, const char* sPath, uint32_t iFlags)
{
	(void)pProxy;
	(void)ppSurface;
	(void)sPath;
	(void)iFlags;
	return XUI_ERROR_UNSUPPORTED;
}

static int __xuiTestSurfaceUnsupportedMemory(xui_proxy pProxy, xui_surface* ppSurface, const void* pData, int iSize, uint32_t iFlags)
{
	(void)pProxy;
	(void)ppSurface;
	(void)pData;
	(void)iSize;
	(void)iFlags;
	return XUI_ERROR_UNSUPPORTED;
}

static int __xuiTestSurfaceUpdateRGBA(xui_proxy pProxy, xui_surface pSurface, xui_rect_i_t tRect, const void* pPixels, int iStride)
{
	int y;

	(void)pProxy;
	if ( !__xuiTestSurfaceValid(pSurface) || (pPixels == NULL) || (tRect.iW <= 0) || (tRect.iH <= 0) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	for ( y = 0; y < tRect.iH; y++ ) {
		memcpy(pSurface->pPixels + ((((tRect.iY + y) * pSurface->tDesc.iWidth) + tRect.iX) * 4),
		       (const unsigned char*)pPixels + ((size_t)y * (size_t)iStride),
		       (size_t)tRect.iW * 4u);
	}
	pSurface->iGeneration++;
	return XUI_OK;
}

static int __xuiTestSurfaceReadRGBA(xui_proxy pProxy, xui_surface pSurface, void* pPixels, int iStride)
{
	int y;

	(void)pProxy;
	if ( !__xuiTestSurfaceValid(pSurface) || (pPixels == NULL) || (iStride < pSurface->tDesc.iWidth * 4) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	for ( y = 0; y < pSurface->tDesc.iHeight; y++ ) {
		memcpy((unsigned char*)pPixels + ((size_t)y * (size_t)iStride),
		       pSurface->pPixels + ((size_t)y * (size_t)pSurface->tDesc.iWidth * 4u),
		       (size_t)pSurface->tDesc.iWidth * 4u);
	}
	return XUI_OK;
}

static int __xuiTestSurfaceGetDesc(xui_proxy pProxy, xui_surface pSurface, xui_surface_desc_t* pDesc)
{
	(void)pProxy;
	if ( !__xuiTestSurfaceValid(pSurface) || (pDesc == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	*pDesc = pSurface->tDesc;
	return XUI_OK;
}

static int __xuiTestSurfaceDraw(xui_proxy pProxy, xui_surface pSurface, xui_rect_t tSrc, xui_rect_t tDst, uint32_t iColor, uint32_t iFlags)
{
	(void)pProxy;
	(void)pSurface;
	(void)tSrc;
	(void)tDst;
	(void)iColor;
	(void)iFlags;
	return XUI_OK;
}

static int __xuiTestSurfaceClear(xui_proxy pProxy, xui_surface pTarget, uint32_t iColor)
{
	xui_rect_t tRect;

	(void)pProxy;
	if ( !__xuiTestSurfaceValid(pTarget) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	tRect.fX = 0.0f;
	tRect.fY = 0.0f;
	tRect.fW = (float)pTarget->tDesc.iWidth;
	tRect.fH = (float)pTarget->tDesc.iHeight;
	return __xuiTestSurfaceFillRect(pTarget, tRect, iColor);
}

static int __xuiTestSurfaceClearRect(xui_proxy pProxy, xui_surface pTarget, xui_rect_t tRect, uint32_t iColor)
{
	(void)pProxy;
	return __xuiTestSurfaceFillRect(pTarget, tRect, iColor);
}

static int __xuiTestSurfaceDrawTo(xui_proxy pProxy, xui_surface pTarget, xui_surface pSurface, xui_rect_t tSrc, xui_rect_t tDst, uint32_t iColor, uint32_t iFlags)
{
	(void)pProxy;
	(void)iColor;
	(void)iFlags;
	return __xuiTestSurfaceBlit(pTarget, pSurface, tSrc, tDst);
}

static int __xuiTestSurfaceDrawQuad(xui_proxy pProxy, xui_surface pSurface, const xui_surface_vertex_t* pVertices, uint32_t iFlags)
{
	(void)pProxy;
	(void)pSurface;
	(void)pVertices;
	(void)iFlags;
	return XUI_OK;
}

static int __xuiTestSurfaceDrawQuadTo(xui_proxy pProxy, xui_surface pTarget, xui_surface pSurface, const xui_surface_vertex_t* pVertices, uint32_t iFlags)
{
	(void)pProxy;
	(void)pTarget;
	(void)pSurface;
	(void)pVertices;
	(void)iFlags;
	return XUI_OK;
}

static int __xuiTestSurfaceGetSampler(xui_proxy pProxy, xui_surface pSurface, xui_surface_sampler_t* pSampler)
{
	(void)pProxy;
	(void)pSurface;
	pSampler->iMinFilter = XUI_SURFACE_FILTER_NEAREST;
	pSampler->iMagFilter = XUI_SURFACE_FILTER_NEAREST;
	pSampler->iWrapS = XUI_SURFACE_WRAP_CLAMP;
	pSampler->iWrapT = XUI_SURFACE_WRAP_CLAMP;
	return XUI_OK;
}

static int __xuiTestSurfaceSetSampler(xui_proxy pProxy, xui_surface pSurface, const xui_surface_sampler_t* pSampler)
{
	(void)pProxy;
	(void)pSurface;
	(void)pSampler;
	return XUI_OK;
}

static int __xuiTestSurfaceGetGeneration(xui_proxy pProxy, xui_surface pSurface, uint32_t* pGeneration)
{
	(void)pProxy;
	if ( !__xuiTestSurfaceValid(pSurface) || (pGeneration == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	*pGeneration = pSurface->iGeneration;
	return XUI_OK;
}

static void __xuiTestSurfaceDestroy(xui_proxy pProxy, xui_surface pSurface)
{
	(void)pProxy;
	if ( !__xuiTestSurfaceValid(pSurface) ) {
		return;
	}
	pSurface->iMagic = 0;
	xrtFree(pSurface->pPixels);
	xrtFree(pSurface);
}

static int __xuiTestShapeOk(xui_proxy pProxy, xui_surface pTarget)
{
	(void)pProxy;
	(void)pTarget;
	return XUI_OK;
}

static int __xuiTestShapePoint(xui_proxy pProxy, xui_surface pTarget, float fX, float fY, float fSize, uint32_t iColor)
{
	(void)fX;
	(void)fY;
	(void)fSize;
	(void)iColor;
	return __xuiTestShapeOk(pProxy, pTarget);
}

static int __xuiTestShapeLine(xui_proxy pProxy, xui_surface pTarget, float fX0, float fY0, float fX1, float fY1, float fWidth, uint32_t iColor)
{
	(void)fX0;
	(void)fY0;
	(void)fX1;
	(void)fY1;
	(void)fWidth;
	(void)iColor;
	return __xuiTestShapeOk(pProxy, pTarget);
}

static int __xuiTestShapeTriangleFill(xui_proxy pProxy, xui_surface pTarget, xui_vec2_t tA, xui_vec2_t tB, xui_vec2_t tC, uint32_t iColor)
{
	(void)tA;
	(void)tB;
	(void)tC;
	(void)iColor;
	return __xuiTestShapeOk(pProxy, pTarget);
}

static int __xuiTestShapeTriangleStroke(xui_proxy pProxy, xui_surface pTarget, xui_vec2_t tA, xui_vec2_t tB, xui_vec2_t tC, float fWidth, uint32_t iColor)
{
	(void)tA;
	(void)tB;
	(void)tC;
	(void)fWidth;
	(void)iColor;
	return __xuiTestShapeOk(pProxy, pTarget);
}

static int __xuiTestShapeRectFill(xui_proxy pProxy, xui_surface pTarget, xui_rect_t tRect, uint32_t iColor)
{
	(void)pProxy;
	return __xuiTestSurfaceFillRect(pTarget, tRect, iColor);
}

static int __xuiTestShapeRectStroke(xui_proxy pProxy, xui_surface pTarget, xui_rect_t tRect, float fWidth, uint32_t iColor)
{
	(void)tRect;
	(void)fWidth;
	(void)iColor;
	return __xuiTestShapeOk(pProxy, pTarget);
}

static int __xuiTestShapeCircleFill(xui_proxy pProxy, xui_surface pTarget, float fX, float fY, float fRadius, uint32_t iColor)
{
	(void)fX;
	(void)fY;
	(void)fRadius;
	(void)iColor;
	return __xuiTestShapeOk(pProxy, pTarget);
}

static int __xuiTestShapeCircleStroke(xui_proxy pProxy, xui_surface pTarget, float fX, float fY, float fRadius, float fWidth, uint32_t iColor)
{
	(void)fX;
	(void)fY;
	(void)fRadius;
	(void)fWidth;
	(void)iColor;
	return __xuiTestShapeOk(pProxy, pTarget);
}



static int __xuiTestFontLoadFile(xui_proxy pProxy, xui_font* ppFont, const char* sPath, float fSize, uint32_t iFlags)
{
	(void)pProxy;
	(void)ppFont;
	(void)sPath;
	(void)fSize;
	(void)iFlags;
	return XUI_ERROR_UNSUPPORTED;
}

static int __xuiTestFontLoadMemory(xui_proxy pProxy, xui_font* ppFont, const void* pData, int iSize, float fSize, uint32_t iFlags)
{
	(void)pProxy;
	(void)ppFont;
	(void)pData;
	(void)iSize;
	(void)fSize;
	(void)iFlags;
	return XUI_ERROR_UNSUPPORTED;
}

static int __xuiTestFontGetMetrics(xui_proxy pProxy, xui_font pFont, xui_font_metrics_t* pMetrics)
{
	(void)pProxy;
	(void)pFont;
	memset(pMetrics, 0, sizeof(*pMetrics));
	return XUI_OK;
}

static void __xuiTestFontDestroy(xui_proxy pProxy, xui_font pFont)
{
	(void)pProxy;
	(void)pFont;
}

static int __xuiTestTextMeasure(xui_proxy pProxy, xui_font pFont, const char* sText, xui_vec2_t* pSize)
{
	(void)pProxy;
	(void)pFont;
	(void)sText;
	pSize->fX = 0.0f;
	pSize->fY = 0.0f;
	return XUI_OK;
}

static int __xuiTestTextDraw(xui_proxy pProxy, xui_surface pTarget, xui_font pFont, const char* sText, xui_rect_t tRect, uint32_t iColor, uint32_t iFlags)
{
	(void)pProxy;
	(void)pTarget;
	(void)pFont;
	(void)sText;
	(void)tRect;
	(void)iColor;
	(void)iFlags;
	return XUI_OK;
}

static int __xuiTestDrawBegin(xui_proxy pProxy, xui_draw_context* ppDraw, xui_surface pTarget)
{
	xui_draw_context pDraw;

	(void)pProxy;
	if ( (ppDraw == NULL) || !__xuiTestSurfaceValid(pTarget) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	*ppDraw = NULL;
	pDraw = (xui_draw_context)xrtCalloc(1, sizeof(*pDraw));
	if ( pDraw == NULL ) {
		return XUI_ERROR_OUT_OF_MEMORY;
	}
	pDraw->iMagic = XUI_TEST_DRAW_MAGIC;
	pDraw->pTarget = pTarget;
	*ppDraw = pDraw;
	return XUI_OK;
}

static int __xuiTestDrawEnd(xui_proxy pProxy, xui_draw_context pDraw)
{
	(void)pProxy;
	if ( !__xuiTestDrawValid(pDraw) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pDraw->iMagic = 0;
	xrtFree(pDraw);
	return XUI_OK;
}

static int __xuiTestDrawClearRect(xui_proxy pProxy, xui_draw_context pDraw, xui_rect_t tRect, uint32_t iColor)
{
	(void)pProxy;
	if ( !__xuiTestDrawValid(pDraw) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	return __xuiTestSurfaceFillRect(pDraw->pTarget, tRect, iColor);
}

static int __xuiTestDrawSurface(xui_proxy pProxy, xui_draw_context pDraw, xui_surface pSurface, xui_rect_t tSrc, xui_rect_t tDst, uint32_t iColor, uint32_t iFlags)
{
	(void)pProxy;
	(void)iColor;
	(void)iFlags;
	if ( !__xuiTestDrawValid(pDraw) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	return __xuiTestSurfaceBlit(pDraw->pTarget, pSurface, tSrc, tDst);
}

static int __xuiTestDrawSurfaceQuad(xui_proxy pProxy, xui_draw_context pDraw, xui_surface pSurface, const xui_surface_vertex_t* pVertices, uint32_t iFlags)
{
	(void)pProxy;
	(void)pDraw;
	(void)pSurface;
	(void)pVertices;
	(void)iFlags;
	return XUI_OK;
}

static int __xuiTestDrawPoint(xui_proxy pProxy, xui_draw_context pDraw, float fX, float fY, float fSize, uint32_t iColor)
{
	(void)pProxy;
	(void)pDraw;
	(void)fX;
	(void)fY;
	(void)fSize;
	(void)iColor;
	return XUI_OK;
}

static int __xuiTestDrawLine(xui_proxy pProxy, xui_draw_context pDraw, float fX0, float fY0, float fX1, float fY1, float fWidth, uint32_t iColor)
{
	(void)pProxy;
	(void)pDraw;
	(void)fX0;
	(void)fY0;
	(void)fX1;
	(void)fY1;
	(void)fWidth;
	(void)iColor;
	return XUI_OK;
}

static int __xuiTestDrawTriangleFill(xui_proxy pProxy, xui_draw_context pDraw, xui_vec2_t tA, xui_vec2_t tB, xui_vec2_t tC, uint32_t iColor)
{
	(void)pProxy;
	(void)pDraw;
	(void)tA;
	(void)tB;
	(void)tC;
	(void)iColor;
	return XUI_OK;
}

static int __xuiTestDrawTriangleStroke(xui_proxy pProxy, xui_draw_context pDraw, xui_vec2_t tA, xui_vec2_t tB, xui_vec2_t tC, float fWidth, uint32_t iColor)
{
	(void)pProxy;
	(void)pDraw;
	(void)tA;
	(void)tB;
	(void)tC;
	(void)fWidth;
	(void)iColor;
	return XUI_OK;
}

static int __xuiTestDrawRectFill(xui_proxy pProxy, xui_draw_context pDraw, xui_rect_t tRect, uint32_t iColor)
{
	(void)pProxy;
	if ( !__xuiTestDrawValid(pDraw) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	return __xuiTestSurfaceFillRect(pDraw->pTarget, tRect, iColor);
}

static int __xuiTestDrawRectStroke(xui_proxy pProxy, xui_draw_context pDraw, xui_rect_t tRect, float fWidth, uint32_t iColor)
{
	(void)pProxy;
	(void)pDraw;
	(void)tRect;
	(void)fWidth;
	(void)iColor;
	return XUI_OK;
}

static int __xuiTestDrawCircleFill(xui_proxy pProxy, xui_draw_context pDraw, float fX, float fY, float fRadius, uint32_t iColor)
{
	(void)pProxy;
	(void)pDraw;
	(void)fX;
	(void)fY;
	(void)fRadius;
	(void)iColor;
	return XUI_OK;
}

static int __xuiTestDrawCircleStroke(xui_proxy pProxy, xui_draw_context pDraw, float fX, float fY, float fRadius, float fWidth, uint32_t iColor)
{
	(void)pProxy;
	(void)pDraw;
	(void)fX;
	(void)fY;
	(void)fRadius;
	(void)fWidth;
	(void)iColor;
	return XUI_OK;
}



static int __xuiTestDrawText(xui_proxy pProxy, xui_draw_context pDraw, xui_font pFont, const char* sText, xui_rect_t tRect, uint32_t iColor, uint32_t iFlags)
{
	(void)pProxy;
	(void)pDraw;
	(void)pFont;
	(void)sText;
	(void)tRect;
	(void)iColor;
	(void)iFlags;
	return XUI_OK;
}

static xui_proxy_t __xuiTestProxy(void)
{
	xui_proxy_t tProxy;

	memset(&tProxy, 0, sizeof(tProxy));
	tProxy.iSize = sizeof(tProxy);
	tProxy.iVersion = XUI_PROXY_VERSION;
	tProxy.getCaps = __xuiTestGetCaps;
	tProxy.clipboardSetText = __xuiTestClipboardSetText;
	tProxy.clipboardGetText = __xuiTestClipboardGetText;
	tProxy.imeGetEnabled = __xuiTestImeGetEnabled;
	tProxy.imeSetEnabled = __xuiTestImeSetEnabled;
	tProxy.imeSetCandidateRect = __xuiTestImeSetCandidateRect;
	tProxy.surfaceCreate = __xuiTestSurfaceCreate;
	tProxy.surfaceCreateRGBA = __xuiTestSurfaceCreateRGBA;
	tProxy.surfaceLoadFile = __xuiTestSurfaceUnsupported;
	tProxy.surfaceLoadMemory = __xuiTestSurfaceUnsupportedMemory;
	tProxy.surfaceUpdateRGBA = __xuiTestSurfaceUpdateRGBA;
	tProxy.surfaceReadRGBA = __xuiTestSurfaceReadRGBA;
	tProxy.surfaceGetDesc = __xuiTestSurfaceGetDesc;
	tProxy.surfaceDraw = __xuiTestSurfaceDraw;
	tProxy.surfaceClear = __xuiTestSurfaceClear;
	tProxy.surfaceClearRect = __xuiTestSurfaceClearRect;
	tProxy.surfaceDrawTo = __xuiTestSurfaceDrawTo;
	tProxy.surfaceDrawQuad = __xuiTestSurfaceDrawQuad;
	tProxy.surfaceDrawQuadTo = __xuiTestSurfaceDrawQuadTo;
	tProxy.surfaceGetSampler = __xuiTestSurfaceGetSampler;
	tProxy.surfaceSetSampler = __xuiTestSurfaceSetSampler;
	tProxy.surfaceGetGeneration = __xuiTestSurfaceGetGeneration;
	tProxy.surfaceDestroy = __xuiTestSurfaceDestroy;
	tProxy.shapePoint = __xuiTestShapePoint;
	tProxy.shapeLine = __xuiTestShapeLine;
	tProxy.shapeTriangleFill = __xuiTestShapeTriangleFill;
	tProxy.shapeTriangleStroke = __xuiTestShapeTriangleStroke;
	tProxy.shapeRectFill = __xuiTestShapeRectFill;
	tProxy.shapeRectStroke = __xuiTestShapeRectStroke;
	tProxy.shapeCircleFill = __xuiTestShapeCircleFill;
	tProxy.shapeCircleStroke = __xuiTestShapeCircleStroke;
	tProxy.fontLoadFile = __xuiTestFontLoadFile;
	tProxy.fontLoadMemory = __xuiTestFontLoadMemory;
	tProxy.fontGetMetrics = __xuiTestFontGetMetrics;
	tProxy.fontDestroy = __xuiTestFontDestroy;
	tProxy.textMeasure = __xuiTestTextMeasure;
	tProxy.textDraw = __xuiTestTextDraw;
	tProxy.drawBegin = __xuiTestDrawBegin;
	tProxy.drawEnd = __xuiTestDrawEnd;
	tProxy.drawClearRect = __xuiTestDrawClearRect;
	tProxy.drawSurface = __xuiTestDrawSurface;
	tProxy.drawSurfaceQuad = __xuiTestDrawSurfaceQuad;
	tProxy.drawPoint = __xuiTestDrawPoint;
	tProxy.drawLine = __xuiTestDrawLine;
	tProxy.drawTriangleFill = __xuiTestDrawTriangleFill;
	tProxy.drawTriangleStroke = __xuiTestDrawTriangleStroke;
	tProxy.drawRectFill = __xuiTestDrawRectFill;
	tProxy.drawRectStroke = __xuiTestDrawRectStroke;
	tProxy.drawCircleFill = __xuiTestDrawCircleFill;
	tProxy.drawCircleStroke = __xuiTestDrawCircleStroke;
	tProxy.drawText = __xuiTestDrawText;
	return tProxy;
}

static int __xuiRenderScheduleDraw(xui_widget pWidget, xui_draw_context pDraw, uint32_t iStateId, void* pUser)
{
	xui_render_schedule_test_state_t* pState;
	xui_rect_t tRect;
	uint32_t iColor;

	pState = (xui_render_schedule_test_state_t*)pUser;
	tRect = xuiWidgetGetRect(pWidget);
	tRect.fX = 0.0f;
	tRect.fY = 0.0f;
	iColor = (iStateId == 1) ? XUI_COLOR_RGBA(0, 255, 0, 255) : XUI_COLOR_RGBA(255, 0, 0, 255);
	pState->iRenderCount++;
	return pState->tProxy.drawRectFill(&pState->tProxy, pDraw, tRect, iColor);
}

static int __xuiPixelEquals(const unsigned char* pPixels, int iStride, int iX, int iY, unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
	const unsigned char* pPixel;

	pPixel = pPixels + (iY * iStride) + (iX * 4);
	return (pPixel[0] == r) && (pPixel[1] == g) && (pPixel[2] == b) && (pPixel[3] == a);
}

int main(void)
{
	xui_render_schedule_test_state_t tState;
	xui_resource_desc_t tResourceDesc;
	xui_render_node_t tRenderNode;
	xui_surface_desc_t tSurfaceDesc;
	xui_cache_policy_t tPolicy;
	xui_render_stats_t tStats;
	xui_cache_stats_t tCacheStats;
	xui_rect_i_t tDamageRect;
	xui_context pContext;
	xui_widget pRoot;
	xui_widget pChild;
	xui_widget pOverlay;
	xui_resource pResource;
	xui_resource pDependency;
	xui_painter pPainter;
	xui_draw_context pDraw;
	xui_surface pTarget;
	xui_surface pPatchSurface;
	uint32_t iResourceGeneration;
	unsigned char arrPatchPixels[9 * 4];
	unsigned char arrReadback[64 * 64 * 4];
	int iRet;
	int iFailed;

	memset(&tState, 0, sizeof(tState));
	memset(&tStats, 0, sizeof(tStats));
	pContext = NULL;
	pRoot = NULL;
	pChild = NULL;
	pOverlay = NULL;
	pResource = NULL;
	pDependency = NULL;
	pPainter = NULL;
	pDraw = NULL;
	pTarget = NULL;
	pPatchSurface = NULL;
	iFailed = 0;

	tState.tProxy = __xuiTestProxy();
	iRet = xuiCreate(&pContext);
	XUI_TEST_CHECK((iRet == XUI_OK) && (pContext != NULL), "xuiCreate failed");
	iRet = xuiSetProxy(pContext, &tState.tProxy);
	XUI_TEST_CHECK(iRet == XUI_OK, "xuiSetProxy failed");
	iRet = xuiSetViewportSize(pContext, 64.0f, 64.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "xuiSetViewportSize failed");

	memset(&tSurfaceDesc, 0, sizeof(tSurfaceDesc));
	tSurfaceDesc.iKind = XUI_SURFACE_KIND_TEXTURE;
	tSurfaceDesc.iFormat = XUI_SURFACE_FORMAT_RGBA8;
	tSurfaceDesc.iWidth = 64;
	tSurfaceDesc.iHeight = 64;
	tSurfaceDesc.iFlags = XUI_SURFACE_ALPHA_PREMULTIPLIED | XUI_SURFACE_USAGE_TARGET;
	iRet = tState.tProxy.surfaceCreate(&tState.tProxy, &pTarget, &tSurfaceDesc);
	XUI_TEST_CHECK((iRet == XUI_OK) && (pTarget != NULL), "target create failed");

	iRet = xuiWidgetCreate(pContext, &pRoot);
	XUI_TEST_CHECK((iRet == XUI_OK) && (pRoot != NULL), "root create failed");
	iRet = xuiWidgetCreate(pContext, &pChild);
	XUI_TEST_CHECK((iRet == XUI_OK) && (pChild != NULL), "child create failed");

	memset(&tPolicy, 0, sizeof(tPolicy));
	tPolicy.iSize = sizeof(tPolicy);
	tPolicy.iPolicy = XUI_CACHE_POLICY_SUBTREE;
	tPolicy.iFlags = XUI_CACHE_CLEAR_ON_UPDATE;
	tPolicy.iClearColor = XUI_COLOR_RGBA(0, 0, 0, 0);
	iRet = xuiWidgetSetCachePolicy(pRoot, &tPolicy);
	XUI_TEST_CHECK(iRet == XUI_OK, "root subtree policy failed");

	tPolicy.iPolicy = XUI_CACHE_POLICY_SELF;
	iRet = xuiWidgetSetCachePolicy(pChild, &tPolicy);
	XUI_TEST_CHECK(iRet == XUI_OK, "child self policy failed");
	iRet = xuiWidgetSetCacheRenderCallback(pChild, __xuiRenderScheduleDraw, &tState);
	XUI_TEST_CHECK(iRet == XUI_OK, "child cache callback failed");
	iRet = xuiWidgetSetRect(pChild, (xui_rect_t){0.0f, 0.0f, 16.0f, 16.0f});
	XUI_TEST_CHECK(iRet == XUI_OK, "child rect failed");
	iRet = xuiWidgetAddChild(pRoot, pChild);
	XUI_TEST_CHECK(iRet == XUI_OK, "add child failed");
	iRet = xuiSetRootWidget(pContext, pRoot);
	XUI_TEST_CHECK(iRet == XUI_OK, "set root failed");

	iRet = xuiRender(pContext, pTarget, NULL, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "first render failed");
	memset(arrReadback, 0, sizeof(arrReadback));
	iRet = tState.tProxy.surfaceReadRGBA(&tState.tProxy, pTarget, arrReadback, 64 * 4);
	XUI_TEST_CHECK(iRet == XUI_OK, "first readback failed");
	XUI_TEST_CHECK(__xuiPixelEquals(arrReadback, 64 * 4, 0, 0, 255, 0, 0, 255), "first render pixel should be red");
	XUI_TEST_CHECK(xuiWidgetGetCacheSurface(pRoot, 0) != NULL, "root subtree cache missing");
	XUI_TEST_CHECK(xuiWidgetGetCacheSurface(pChild, 0) != NULL, "child cache missing");
	iRet = xuiGetRenderStats(pContext, &tStats);
	XUI_TEST_CHECK(iRet == XUI_OK, "stats getter failed");
	XUI_TEST_CHECK((tStats.iUpdatedCaches >= 2) && (tStats.iDrawnCaches >= 2) && (tStats.iSkippedWidgets >= 1), "first render stats mismatch");

	tPolicy.iPolicy = XUI_CACHE_POLICY_SELF;
	tPolicy.iFlags = XUI_CACHE_CLEAR_ON_UPDATE | XUI_CACHE_UPDATE_ALL_STATES;
	iRet = xuiWidgetSetCachePolicy(pChild, &tPolicy);
	XUI_TEST_CHECK(iRet == XUI_OK, "child all-state policy failed");
	iRet = xuiWidgetSetCacheStateCount(pChild, 2);
	XUI_TEST_CHECK(iRet == XUI_OK, "child cache count failed");
	iRet = xuiWidgetSetCacheStateId(pChild, 0, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "child cache state 0 failed");
	iRet = xuiWidgetSetCacheStateId(pChild, 1, 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "child cache state 1 failed");
	iRet = xuiWidgetSetStateId(pChild, 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "child state switch failed");
	iRet = xuiWidgetInvalidate(pChild, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	XUI_TEST_CHECK(iRet == XUI_OK, "child invalidate failed");
	iRet = xuiRender(pContext, pTarget, NULL, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "second render failed");
	memset(arrReadback, 0, sizeof(arrReadback));
	iRet = tState.tProxy.surfaceReadRGBA(&tState.tProxy, pTarget, arrReadback, 64 * 4);
	XUI_TEST_CHECK(iRet == XUI_OK, "second readback failed");
	XUI_TEST_CHECK(__xuiPixelEquals(arrReadback, 64 * 4, 0, 0, 0, 255, 0, 255), "second render pixel should be green");
	XUI_TEST_CHECK((xuiWidgetGetCacheSurface(pChild, 0) != NULL) &&
	               (xuiWidgetGetCacheSurface(pChild, 1) != NULL), "all-state caches missing");
	iRet = xuiGetRenderStats(pContext, &tStats);
	XUI_TEST_CHECK(iRet == XUI_OK, "second stats getter failed");
	XUI_TEST_CHECK(tStats.iUpdatedCaches >= 3, "all-state render should update child states and root subtree cache");
	iRet = xuiWidgetUpdateBegin(pChild, 2, XUI_WIDGET_UPDATE_CLEAR, XUI_COLOR_RGBA(0, 0, 0, 0), &pDraw);
	XUI_TEST_CHECK((iRet == XUI_OK) && (pDraw != NULL), "public update begin should create requested state slot");
	iRet = xuiWidgetUpdateEnd(pChild, 2, pDraw);
	pDraw = NULL;
	XUI_TEST_CHECK(iRet == XUI_OK, "public update end for requested state failed");
	XUI_TEST_CHECK((xuiWidgetGetCacheStateCount(pChild) >= 3) && (xuiWidgetGetCacheSurface(pChild, 2) != NULL), "public update state cache missing");

	iRet = tState.tProxy.surfaceClear(&tState.tProxy, pTarget, XUI_COLOR_RGBA(0, 0, 0, 0));
	XUI_TEST_CHECK(iRet == XUI_OK, "target clear before damage failed");
	tDamageRect = (xui_rect_i_t){4, 4, 4, 4};
	iRet = xuiRender(pContext, pTarget, &tDamageRect, 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "damage render failed");
	memset(arrReadback, 0, sizeof(arrReadback));
	iRet = tState.tProxy.surfaceReadRGBA(&tState.tProxy, pTarget, arrReadback, 64 * 4);
	XUI_TEST_CHECK(iRet == XUI_OK, "damage readback failed");
	XUI_TEST_CHECK(__xuiPixelEquals(arrReadback, 64 * 4, 4, 4, 0, 255, 0, 255), "damage area should redraw");
	XUI_TEST_CHECK(__xuiPixelEquals(arrReadback, 64 * 4, 0, 0, 0, 0, 0, 0), "outside damage should stay untouched");

	memset(&tCacheStats, 0, sizeof(tCacheStats));
	iRet = xuiGetCacheStats(pContext, &tCacheStats);
	XUI_TEST_CHECK((iRet == XUI_OK) && (tCacheStats.iUsedBytes > 0u) &&
	               (tCacheStats.iSurfaceCount >= 3), "cache stats failed");
	iRet = xuiSetCacheBudget(pContext, 1u);
	XUI_TEST_CHECK(iRet == XUI_OK, "cache budget set failed");
	memset(&tCacheStats, 0, sizeof(tCacheStats));
	iRet = xuiGetCacheStats(pContext, &tCacheStats);
	XUI_TEST_CHECK((iRet == XUI_OK) && (tCacheStats.iBudgetBytes == 1u) &&
	               (tCacheStats.iUsedBytes <= 1u), "cache purge failed");

	memset(&tResourceDesc, 0, sizeof(tResourceDesc));
	tResourceDesc.iSize = sizeof(tResourceDesc);
	tResourceDesc.sName = "test.surface";
	tResourceDesc.iKind = XUI_RESOURCE_SURFACE;
	tResourceDesc.pHandle = pTarget;
	tResourceDesc.pUser = &tState;
	tResourceDesc.onDestroy = __xuiTestResourceDestroy;
	iRet = xuiResourceSet(pContext, &pResource, &tResourceDesc);
	XUI_TEST_CHECK((iRet == XUI_OK) && (pResource != NULL), "resource set failed");
	XUI_TEST_CHECK((xuiResourceFind(pContext, "test.surface") == pResource) &&
	               (xuiResourceGetKind(pResource) == XUI_RESOURCE_SURFACE) &&
	               (xuiResourceGetHandle(pResource) == pTarget) &&
	               (xuiResourceGetGeneration(pResource) != 0), "resource getters failed");
	memset(&tResourceDesc, 0, sizeof(tResourceDesc));
	tResourceDesc.iSize = sizeof(tResourceDesc);
	tResourceDesc.sName = "test.dependency";
	tResourceDesc.iKind = XUI_RESOURCE_USER;
	tResourceDesc.pHandle = NULL;
	tResourceDesc.pUser = &tState;
	tResourceDesc.onDestroy = __xuiTestResourceDestroy;
	iRet = xuiResourceSet(pContext, &pDependency, &tResourceDesc);
	XUI_TEST_CHECK((iRet == XUI_OK) && (pDependency != NULL), "dependency resource set failed");
	iRet = xuiResourceAddRef(pDependency);
	XUI_TEST_CHECK((iRet == XUI_OK) && (xuiResourceGetRefCount(pDependency) == 2), "resource add ref failed");
	iRet = xuiResourceRelease(pDependency);
	XUI_TEST_CHECK((iRet == XUI_OK) && (xuiResourceGetRefCount(pDependency) == 1), "resource release ref failed");
	iRet = xuiResourceAddDependency(pResource, pDependency);
	XUI_TEST_CHECK((iRet == XUI_OK) && (xuiResourceGetDependencyCount(pResource) == 1) &&
	               (xuiResourceGetDependency(pResource, 0) == pDependency), "resource dependency failed");
	iResourceGeneration = xuiResourceGetGeneration(pResource);
	iRet = xuiResourceTouch(pDependency);
	XUI_TEST_CHECK((iRet == XUI_OK) && (xuiResourceGetGeneration(pResource) != iResourceGeneration), "dependency touch propagation failed");
	xuiResourceClearDependencies(pResource);
	XUI_TEST_CHECK(xuiResourceGetDependencyCount(pResource) == 0, "resource dependency clear failed");
	iRet = xuiResourceRelease(pDependency);
	pDependency = NULL;
	XUI_TEST_CHECK((iRet == XUI_OK) && (tState.iResourceDestroyed == 1), "dependency release destroy failed");
	iRet = xuiResourceTouch(pResource);
	XUI_TEST_CHECK(iRet == XUI_OK, "resource touch failed");
	iRet = xuiResourceRemove(pResource);
	pResource = NULL;
	XUI_TEST_CHECK((iRet == XUI_OK) && (tState.iResourceDestroyed == 2) &&
	               (xuiResourceFind(pContext, "test.surface") == NULL), "resource remove failed");

	memset(arrPatchPixels, 0, sizeof(arrPatchPixels));
	arrPatchPixels[0] = 90;
	arrPatchPixels[1] = 10;
	arrPatchPixels[2] = 10;
	arrPatchPixels[3] = 255;
	arrPatchPixels[4] = 20;
	arrPatchPixels[5] = 90;
	arrPatchPixels[6] = 10;
	arrPatchPixels[7] = 255;
	iRet = tState.tProxy.surfaceCreateRGBA(&tState.tProxy, &pPatchSurface, 3, 3, arrPatchPixels, 3 * 4, XUI_SURFACE_ALPHA_PREMULTIPLIED);
	XUI_TEST_CHECK((iRet == XUI_OK) && (pPatchSurface != NULL), "patch surface create failed");
	iRet = xuiPainterBegin(pContext, pTarget, &pPainter);
	XUI_TEST_CHECK((iRet == XUI_OK) && (pPainter != NULL) && (xuiPainterGetDrawContext(pPainter) != NULL), "painter begin failed");
	iRet = xuiPainterFillRect(pPainter, (xui_rect_t){4.0f, 4.0f, 8.0f, 8.0f}, XUI_COLOR_RGBA(10, 20, 30, 255));
	XUI_TEST_CHECK(iRet == XUI_OK, "painter fill rect failed");
	iRet = xuiPainterClearRect(pPainter, (xui_rect_t){12.0f, 12.0f, 2.0f, 2.0f}, XUI_COLOR_RGBA(1, 2, 3, 255));
	XUI_TEST_CHECK(iRet == XUI_OK, "painter clear rect failed");
	iRet = xuiPainterDrawNinePatch(pPainter, pPatchSurface, (xui_rect_t){0.0f, 0.0f, 3.0f, 3.0f},
		(xui_rect_t){20.0f, 20.0f, 5.0f, 5.0f}, (xui_thickness_t){1.0f, 1.0f, 1.0f, 1.0f}, XUI_COLOR_WHITE, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "painter nine patch failed");
	iRet = xuiPainterEnd(pPainter);
	pPainter = NULL;
	XUI_TEST_CHECK(iRet == XUI_OK, "painter end failed");
	memset(arrReadback, 0, sizeof(arrReadback));
	iRet = tState.tProxy.surfaceReadRGBA(&tState.tProxy, pTarget, arrReadback, 64 * 4);
	XUI_TEST_CHECK(iRet == XUI_OK, "painter readback failed");
	XUI_TEST_CHECK(__xuiPixelEquals(arrReadback, 64 * 4, 4, 4, 10, 20, 30, 255), "painter pixel failed");
	XUI_TEST_CHECK(__xuiPixelEquals(arrReadback, 64 * 4, 20, 20, 90, 10, 10, 255), "nine patch corner pixel failed");

	tPolicy.iSize = sizeof(tPolicy);
	tPolicy.iPolicy = XUI_CACHE_POLICY_SUBTREE_TILED;
	tPolicy.iFlags = XUI_CACHE_CLEAR_ON_UPDATE;
	tPolicy.iClearColor = XUI_COLOR_RGBA(0, 0, 0, 0);
	tPolicy.iTileWidth = 16;
	tPolicy.iTileHeight = 16;
	tPolicy.iMaxBytes = 4096;
	iRet = xuiWidgetSetCachePolicy(pRoot, &tPolicy);
	XUI_TEST_CHECK(iRet == XUI_OK, "root tiled cache policy failed");
	tPolicy.iPolicy = XUI_CACHE_POLICY_DISPLAY_LIST;
	tPolicy.iTileWidth = 0;
	tPolicy.iTileHeight = 0;
	tPolicy.iMaxBytes = 0;
	iRet = xuiWidgetSetCachePolicy(pChild, &tPolicy);
	XUI_TEST_CHECK((iRet == XUI_OK) && (xuiWidgetGetCachePolicy(pChild).iPolicy == XUI_CACHE_POLICY_DISPLAY_LIST), "child display-list policy failed");
	iRet = xuiWidgetSetLayer(pRoot, 0, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "root layer failed");
	iRet = xuiWidgetSetLayer(pChild, 3, 7);
	XUI_TEST_CHECK(iRet == XUI_OK, "child layer failed");
	iRet = xuiWidgetGetLayer(pChild, &tPolicy.iTileWidth, &tPolicy.iTileHeight);
	XUI_TEST_CHECK((iRet == XUI_OK) && (tPolicy.iTileWidth == 3) && (tPolicy.iTileHeight == 7), "child layer getter failed");
	iRet = xuiWidgetSetOverflow(pRoot, XUI_OVERFLOW_HIDDEN);
	XUI_TEST_CHECK((iRet == XUI_OK) && (xuiWidgetGetOverflow(pRoot) == XUI_OVERFLOW_HIDDEN), "root overflow failed");
	iRet = xuiBuildRenderTree(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "render tree build failed");
	XUI_TEST_CHECK(xuiGetRenderNodeCount(pContext) == 2, "render tree count failed");
	iRet = xuiGetRenderNode(pContext, 0, &tRenderNode);
	XUI_TEST_CHECK((iRet == XUI_OK) && (tRenderNode.pWidget == pRoot) &&
	               (tRenderNode.iCachePolicy == XUI_CACHE_POLICY_SUBTREE), "root render node failed");
	iRet = xuiGetRenderNode(pContext, 1, &tRenderNode);
	XUI_TEST_CHECK((iRet == XUI_OK) && (tRenderNode.pWidget == pChild) &&
	               (tRenderNode.iLayer == 3) && (tRenderNode.iZIndex == 7) &&
	               (tRenderNode.iCachePolicy == XUI_CACHE_POLICY_SELF), "child render node failed");

	iRet = xuiWidgetCreate(pContext, &pOverlay);
	XUI_TEST_CHECK((iRet == XUI_OK) && (pOverlay != NULL), "overlay create failed");
	tPolicy.iSize = sizeof(tPolicy);
	tPolicy.iPolicy = XUI_CACHE_POLICY_SELF;
	tPolicy.iFlags = XUI_CACHE_CLEAR_ON_UPDATE;
	tPolicy.iClearColor = XUI_COLOR_RGBA(0, 0, 0, 0);
	iRet = xuiWidgetSetCachePolicy(pOverlay, &tPolicy);
	XUI_TEST_CHECK(iRet == XUI_OK, "overlay cache policy failed");
	iRet = xuiWidgetSetCacheRenderCallback(pOverlay, __xuiRenderScheduleDraw, &tState);
	XUI_TEST_CHECK(iRet == XUI_OK, "overlay cache callback failed");
	iRet = xuiWidgetSetRect(pOverlay, (xui_rect_t){2.0f, 2.0f, 8.0f, 8.0f});
	XUI_TEST_CHECK(iRet == XUI_OK, "overlay rect failed");
	iRet = xuiOverlayAttach(pContext, pRoot, pOverlay, XUI_LAYER_POPUP, 10);
	XUI_TEST_CHECK(iRet == XUI_OK, "overlay attach failed");
	iRet = xuiWidgetInvalidate(pOverlay, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	XUI_TEST_CHECK(iRet == XUI_OK, "overlay invalidate failed");
	iRet = xuiRender(pContext, pTarget, NULL, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "overlay render failed");
	XUI_TEST_CHECK((xuiWidgetGetDirtyFlags(pOverlay) & (XUI_WIDGET_DIRTY_RENDER | XUI_WIDGET_DIRTY_TREE)) == 0, "overlay render dirty should clear");

cleanup:
	if ( pDraw != NULL ) {
		(void)xuiWidgetUpdateEnd(pChild, 2, pDraw);
	}
	if ( pPainter != NULL ) {
		xuiPainterEnd(pPainter);
	}
	if ( pDependency != NULL ) {
		(void)xuiResourceRemove(pDependency);
	}
	if ( pResource != NULL ) {
		(void)xuiResourceRemove(pResource);
	}
	if ( pPatchSurface != NULL ) {
		tState.tProxy.surfaceDestroy(&tState.tProxy, pPatchSurface);
	}
	if ( pTarget != NULL ) {
		tState.tProxy.surfaceDestroy(&tState.tProxy, pTarget);
	}
	if ( pContext != NULL ) {
		xuiDestroy(pContext);
	}
	if ( iFailed ) {
		return 1;
	}
	printf("xui_render_schedule_test passed\n");
	return 0;
}
