#include "xui.h"

#include <stdio.h>
#include <string.h>

#define XUI_TEST_CHECK(expr, msg) \
	do { \
		if ( !(expr) ) { \
			printf("xui_image_test failed: %s\n", (msg)); \
			iFailed = 1; \
			goto cleanup; \
		} \
	} while ( 0 )

#define XUI_TEST_SURFACE_MAGIC 0x49535552u
#define XUI_TEST_DRAW_MAGIC 0x49445257u

struct xui_surface_t {
	uint32_t iMagic;
	xui_surface_desc_t tDesc;
	uint32_t iGeneration;
	int iDrawCount;
	xui_rect_t tLastSrc;
	xui_rect_t tLastDst;
	uint32_t iLastColor;
	uint32_t iLastFlags;
};

struct xui_draw_context_t {
	uint32_t iMagic;
	xui_surface pTarget;
};

typedef struct xui_image_test_state_t {
	xui_proxy_t tProxy;
} xui_image_test_state_t;

static int __xuiImageTestSurfaceValid(xui_surface pSurface)
{
	return (pSurface != NULL) && (pSurface->iMagic == XUI_TEST_SURFACE_MAGIC);
}

static int __xuiImageTestDrawValid(xui_draw_context pDraw)
{
	return (pDraw != NULL) && (pDraw->iMagic == XUI_TEST_DRAW_MAGIC) && __xuiImageTestSurfaceValid(pDraw->pTarget);
}

static int __xuiImageTestClipboardSetText(xui_proxy pProxy, const char* sText)
{
	(void)pProxy;
	(void)sText;
	return XUI_OK;
}

static int __xuiImageTestClipboardGetText(xui_proxy pProxy, char* sText, int iCapacity)
{
	(void)pProxy;
	if ( (sText != NULL) && (iCapacity > 0) ) {
		sText[0] = '\0';
	}
	return 0;
}

static int __xuiImageTestImeGetEnabled(xui_proxy pProxy)
{
	(void)pProxy;
	return 0;
}

static int __xuiImageTestImeSetEnabled(xui_proxy pProxy, int bEnabled)
{
	(void)pProxy;
	(void)bEnabled;
	return XUI_OK;
}

static int __xuiImageTestImeSetCandidateRect(xui_proxy pProxy, xui_rect_t tRect)
{
	(void)pProxy;
	(void)tRect;
	return XUI_OK;
}

static int __xuiImageTestGetCaps(xui_proxy pProxy, xui_proxy_caps_t* pCaps)
{
	(void)pProxy;
	if ( pCaps == NULL ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
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

static int __xuiImageTestSurfaceCreate(xui_proxy pProxy, xui_surface* ppSurface, const xui_surface_desc_t* pDesc)
{
	xui_surface pSurface;

	(void)pProxy;
	if ( (ppSurface == NULL) || (pDesc == NULL) || (pDesc->iWidth <= 0) || (pDesc->iHeight <= 0) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	*ppSurface = NULL;
	pSurface = (xui_surface)xrtCalloc(1, sizeof(*pSurface));
	if ( pSurface == NULL ) {
		return XUI_ERROR_OUT_OF_MEMORY;
	}
	pSurface->iMagic = XUI_TEST_SURFACE_MAGIC;
	pSurface->tDesc = *pDesc;
	pSurface->iGeneration = 1;
	*ppSurface = pSurface;
	return XUI_OK;
}

static int __xuiImageTestSurfaceCreateRGBA(xui_proxy pProxy, xui_surface* ppSurface, int iWidth, int iHeight, const void* pPixels, int iStride, uint32_t iFlags)
{
	xui_surface_desc_t tDesc;

	(void)pPixels;
	(void)iStride;
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iKind = XUI_SURFACE_KIND_TEXTURE;
	tDesc.iFormat = XUI_SURFACE_FORMAT_RGBA8;
	tDesc.iWidth = iWidth;
	tDesc.iHeight = iHeight;
	tDesc.iFlags = iFlags;
	return __xuiImageTestSurfaceCreate(pProxy, ppSurface, &tDesc);
}

static int __xuiImageTestSurfaceLoadFile(xui_proxy pProxy, xui_surface* ppSurface, const char* sPath, uint32_t iFlags)
{
	(void)sPath;
	return __xuiImageTestSurfaceCreateRGBA(pProxy, ppSurface, 8, 8, NULL, 0, iFlags);
}

static int __xuiImageTestSurfaceLoadMemory(xui_proxy pProxy, xui_surface* ppSurface, const void* pData, int iSize, uint32_t iFlags)
{
	(void)pData;
	(void)iSize;
	return __xuiImageTestSurfaceCreateRGBA(pProxy, ppSurface, 8, 8, NULL, 0, iFlags);
}

static int __xuiImageTestSurfaceUpdateRGBA(xui_proxy pProxy, xui_surface pSurface, xui_rect_i_t tRect, const void* pPixels, int iStride)
{
	(void)pProxy;
	(void)tRect;
	(void)pPixels;
	(void)iStride;
	if ( !__xuiImageTestSurfaceValid(pSurface) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pSurface->iGeneration++;
	return XUI_OK;
}

static int __xuiImageTestSurfaceReadRGBA(xui_proxy pProxy, xui_surface pSurface, void* pPixels, int iStride)
{
	(void)pProxy;
	(void)pPixels;
	(void)iStride;
	return __xuiImageTestSurfaceValid(pSurface) ? XUI_OK : XUI_ERROR_INVALID_ARGUMENT;
}

static int __xuiImageTestSurfaceGetDesc(xui_proxy pProxy, xui_surface pSurface, xui_surface_desc_t* pDesc)
{
	(void)pProxy;
	if ( !__xuiImageTestSurfaceValid(pSurface) || (pDesc == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	*pDesc = pSurface->tDesc;
	return XUI_OK;
}

static int __xuiImageTestSurfaceDraw(xui_proxy pProxy, xui_surface pSurface, xui_rect_t tSrc, xui_rect_t tDst, uint32_t iColor, uint32_t iFlags)
{
	(void)pProxy;
	(void)tSrc;
	if ( !__xuiImageTestSurfaceValid(pSurface) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pSurface->iDrawCount++;
	pSurface->tLastDst = tDst;
	pSurface->iLastColor = iColor;
	pSurface->iLastFlags = iFlags;
	return XUI_OK;
}

static int __xuiImageTestSurfaceClear(xui_proxy pProxy, xui_surface pTarget, uint32_t iColor)
{
	(void)pProxy;
	(void)iColor;
	return __xuiImageTestSurfaceValid(pTarget) ? XUI_OK : XUI_ERROR_INVALID_ARGUMENT;
}

static int __xuiImageTestSurfaceClearRect(xui_proxy pProxy, xui_surface pTarget, xui_rect_t tRect, uint32_t iColor)
{
	(void)tRect;
	return __xuiImageTestSurfaceClear(pProxy, pTarget, iColor);
}

static int __xuiImageTestSurfaceGetGeneration(xui_proxy pProxy, xui_surface pSurface, uint32_t* pGeneration)
{
	(void)pProxy;
	if ( !__xuiImageTestSurfaceValid(pSurface) || (pGeneration == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	*pGeneration = pSurface->iGeneration;
	return XUI_OK;
}

static void __xuiImageTestSurfaceDestroy(xui_proxy pProxy, xui_surface pSurface)
{
	(void)pProxy;
	if ( __xuiImageTestSurfaceValid(pSurface) ) {
		pSurface->iMagic = 0;
		xrtFree(pSurface);
	}
}

static int __xuiImageTestDrawBegin(xui_proxy pProxy, xui_draw_context* ppDraw, xui_surface pTarget)
{
	xui_draw_context pDraw;

	(void)pProxy;
	if ( (ppDraw == NULL) || !__xuiImageTestSurfaceValid(pTarget) ) {
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

static int __xuiImageTestDrawEnd(xui_proxy pProxy, xui_draw_context pDraw)
{
	(void)pProxy;
	if ( !__xuiImageTestDrawValid(pDraw) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pDraw->iMagic = 0;
	xrtFree(pDraw);
	return XUI_OK;
}

static int __xuiImageTestDrawClearRect(xui_proxy pProxy, xui_draw_context pDraw, xui_rect_t tRect, uint32_t iColor)
{
	(void)pProxy;
	(void)tRect;
	(void)iColor;
	return __xuiImageTestDrawValid(pDraw) ? XUI_OK : XUI_ERROR_INVALID_ARGUMENT;
}

static int __xuiImageTestDrawSurface(xui_proxy pProxy, xui_draw_context pDraw, xui_surface pSurface, xui_rect_t tSrc, xui_rect_t tDst, uint32_t iColor, uint32_t iFlags)
{
	(void)pProxy;
	if ( !__xuiImageTestDrawValid(pDraw) || !__xuiImageTestSurfaceValid(pSurface) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pDraw->pTarget->iDrawCount++;
	pDraw->pTarget->tLastSrc = tSrc;
	pDraw->pTarget->tLastDst = tDst;
	pDraw->pTarget->iLastColor = iColor;
	pDraw->pTarget->iLastFlags = iFlags;
	return XUI_OK;
}

static int __xuiImageTestDrawSurfaceQuad(xui_proxy pProxy, xui_draw_context pDraw, xui_surface pSurface, const xui_surface_vertex_t* pVertices, uint32_t iFlags)
{
	(void)pProxy;
	(void)pVertices;
	(void)iFlags;
	return (__xuiImageTestDrawValid(pDraw) && __xuiImageTestSurfaceValid(pSurface)) ? XUI_OK : XUI_ERROR_INVALID_ARGUMENT;
}

static int __xuiImageTestDrawPoint(xui_proxy pProxy, xui_draw_context pDraw, float fX, float fY, float fSize, uint32_t iColor)
{
	(void)pProxy;
	(void)fX;
	(void)fY;
	(void)fSize;
	(void)iColor;
	return __xuiImageTestDrawValid(pDraw) ? XUI_OK : XUI_ERROR_INVALID_ARGUMENT;
}

static int __xuiImageTestDrawLine(xui_proxy pProxy, xui_draw_context pDraw, float fX0, float fY0, float fX1, float fY1, float fWidth, uint32_t iColor)
{
	(void)fX0;
	(void)fY0;
	(void)fX1;
	(void)fY1;
	(void)fWidth;
	return __xuiImageTestDrawPoint(pProxy, pDraw, 0.0f, 0.0f, 1.0f, iColor);
}

static int __xuiImageTestDrawTriangleFill(xui_proxy pProxy, xui_draw_context pDraw, xui_vec2_t tA, xui_vec2_t tB, xui_vec2_t tC, uint32_t iColor)
{
	(void)tA;
	(void)tB;
	(void)tC;
	return __xuiImageTestDrawPoint(pProxy, pDraw, 0.0f, 0.0f, 1.0f, iColor);
}

static int __xuiImageTestDrawTriangleStroke(xui_proxy pProxy, xui_draw_context pDraw, xui_vec2_t tA, xui_vec2_t tB, xui_vec2_t tC, float fWidth, uint32_t iColor)
{
	(void)fWidth;
	return __xuiImageTestDrawTriangleFill(pProxy, pDraw, tA, tB, tC, iColor);
}

static int __xuiImageTestDrawRectFill(xui_proxy pProxy, xui_draw_context pDraw, xui_rect_t tRect, uint32_t iColor)
{
	(void)tRect;
	return __xuiImageTestDrawPoint(pProxy, pDraw, 0.0f, 0.0f, 1.0f, iColor);
}

static int __xuiImageTestDrawRectStroke(xui_proxy pProxy, xui_draw_context pDraw, xui_rect_t tRect, float fWidth, uint32_t iColor)
{
	(void)fWidth;
	return __xuiImageTestDrawRectFill(pProxy, pDraw, tRect, iColor);
}

static int __xuiImageTestDrawCircleFill(xui_proxy pProxy, xui_draw_context pDraw, float fX, float fY, float fRadius, uint32_t iColor)
{
	(void)fRadius;
	return __xuiImageTestDrawPoint(pProxy, pDraw, fX, fY, 1.0f, iColor);
}

static int __xuiImageTestDrawCircleStroke(xui_proxy pProxy, xui_draw_context pDraw, float fX, float fY, float fRadius, float fWidth, uint32_t iColor)
{
	(void)fWidth;
	return __xuiImageTestDrawCircleFill(pProxy, pDraw, fX, fY, fRadius, iColor);
}

static int __xuiImageTestDrawRoundRectFill(xui_proxy pProxy, xui_draw_context pDraw, xui_rect_t tRect, float fRadius, uint32_t iColor)
{
	(void)fRadius;
	return __xuiImageTestDrawRectFill(pProxy, pDraw, tRect, iColor);
}

static int __xuiImageTestDrawRoundRectStroke(xui_proxy pProxy, xui_draw_context pDraw, xui_rect_t tRect, float fRadius, float fWidth, uint32_t iColor)
{
	(void)fRadius;
	return __xuiImageTestDrawRectStroke(pProxy, pDraw, tRect, fWidth, iColor);
}

static int __xuiImageTestDrawText(xui_proxy pProxy, xui_draw_context pDraw, xui_font pFont, const char* sText, xui_rect_t tRect, uint32_t iColor, uint32_t iFlags)
{
	(void)pProxy;
	(void)pFont;
	(void)sText;
	(void)tRect;
	(void)iColor;
	(void)iFlags;
	return __xuiImageTestDrawValid(pDraw) ? XUI_OK : XUI_ERROR_INVALID_ARGUMENT;
}

static int __xuiImageTestSurfaceDrawTo(xui_proxy pProxy, xui_surface pTarget, xui_surface pSurface, xui_rect_t tSrc, xui_rect_t tDst, uint32_t iColor, uint32_t iFlags)
{
	(void)pProxy;
	(void)tSrc;
	if ( !__xuiImageTestSurfaceValid(pTarget) || !__xuiImageTestSurfaceValid(pSurface) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pTarget->iDrawCount++;
	pTarget->tLastDst = tDst;
	pTarget->iLastColor = iColor;
	pTarget->iLastFlags = iFlags;
	return XUI_OK;
}

static int __xuiImageTestSurfaceDrawQuad(xui_proxy pProxy, xui_surface pSurface, const xui_surface_vertex_t* pVertices, uint32_t iFlags)
{
	(void)pProxy;
	(void)pVertices;
	(void)iFlags;
	return __xuiImageTestSurfaceValid(pSurface) ? XUI_OK : XUI_ERROR_INVALID_ARGUMENT;
}

static int __xuiImageTestSurfaceDrawQuadTo(xui_proxy pProxy, xui_surface pTarget, xui_surface pSurface, const xui_surface_vertex_t* pVertices, uint32_t iFlags)
{
	(void)pProxy;
	(void)pVertices;
	(void)iFlags;
	return (__xuiImageTestSurfaceValid(pTarget) && __xuiImageTestSurfaceValid(pSurface)) ? XUI_OK : XUI_ERROR_INVALID_ARGUMENT;
}

static int __xuiImageTestSurfaceGetSampler(xui_proxy pProxy, xui_surface pSurface, xui_surface_sampler_t* pSampler)
{
	(void)pProxy;
	if ( !__xuiImageTestSurfaceValid(pSurface) || (pSampler == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pSampler->iMinFilter = XUI_SURFACE_FILTER_NEAREST;
	pSampler->iMagFilter = XUI_SURFACE_FILTER_NEAREST;
	pSampler->iWrapS = XUI_SURFACE_WRAP_CLAMP;
	pSampler->iWrapT = XUI_SURFACE_WRAP_CLAMP;
	return XUI_OK;
}

static int __xuiImageTestSurfaceSetSampler(xui_proxy pProxy, xui_surface pSurface, const xui_surface_sampler_t* pSampler)
{
	(void)pProxy;
	(void)pSampler;
	return __xuiImageTestSurfaceValid(pSurface) ? XUI_OK : XUI_ERROR_INVALID_ARGUMENT;
}

static int __xuiImageTestShapePoint(xui_proxy pProxy, xui_surface pTarget, float fX, float fY, float fSize, uint32_t iColor)
{
	(void)pProxy;
	(void)fX;
	(void)fY;
	(void)fSize;
	(void)iColor;
	return __xuiImageTestSurfaceValid(pTarget) ? XUI_OK : XUI_ERROR_INVALID_ARGUMENT;
}

static int __xuiImageTestShapeLine(xui_proxy pProxy, xui_surface pTarget, float fX0, float fY0, float fX1, float fY1, float fWidth, uint32_t iColor)
{
	(void)fX0;
	(void)fY0;
	(void)fX1;
	(void)fY1;
	(void)fWidth;
	return __xuiImageTestShapePoint(pProxy, pTarget, 0.0f, 0.0f, 1.0f, iColor);
}

static int __xuiImageTestShapeTriangleFill(xui_proxy pProxy, xui_surface pTarget, xui_vec2_t tA, xui_vec2_t tB, xui_vec2_t tC, uint32_t iColor)
{
	(void)tA;
	(void)tB;
	(void)tC;
	return __xuiImageTestShapePoint(pProxy, pTarget, 0.0f, 0.0f, 1.0f, iColor);
}

static int __xuiImageTestShapeTriangleStroke(xui_proxy pProxy, xui_surface pTarget, xui_vec2_t tA, xui_vec2_t tB, xui_vec2_t tC, float fWidth, uint32_t iColor)
{
	(void)fWidth;
	return __xuiImageTestShapeTriangleFill(pProxy, pTarget, tA, tB, tC, iColor);
}

static int __xuiImageTestShapeRectFill(xui_proxy pProxy, xui_surface pTarget, xui_rect_t tRect, uint32_t iColor)
{
	(void)tRect;
	return __xuiImageTestShapePoint(pProxy, pTarget, 0.0f, 0.0f, 1.0f, iColor);
}

static int __xuiImageTestShapeRectStroke(xui_proxy pProxy, xui_surface pTarget, xui_rect_t tRect, float fWidth, uint32_t iColor)
{
	(void)fWidth;
	return __xuiImageTestShapeRectFill(pProxy, pTarget, tRect, iColor);
}

static int __xuiImageTestShapeCircleFill(xui_proxy pProxy, xui_surface pTarget, float fX, float fY, float fRadius, uint32_t iColor)
{
	(void)fRadius;
	return __xuiImageTestShapePoint(pProxy, pTarget, fX, fY, 1.0f, iColor);
}

static int __xuiImageTestShapeCircleStroke(xui_proxy pProxy, xui_surface pTarget, float fX, float fY, float fRadius, float fWidth, uint32_t iColor)
{
	(void)fWidth;
	return __xuiImageTestShapeCircleFill(pProxy, pTarget, fX, fY, fRadius, iColor);
}

static int __xuiImageTestShapeRoundRectFill(xui_proxy pProxy, xui_surface pTarget, xui_rect_t tRect, float fRadius, uint32_t iColor)
{
	(void)fRadius;
	return __xuiImageTestShapeRectFill(pProxy, pTarget, tRect, iColor);
}

static int __xuiImageTestShapeRoundRectStroke(xui_proxy pProxy, xui_surface pTarget, xui_rect_t tRect, float fRadius, float fWidth, uint32_t iColor)
{
	(void)fRadius;
	return __xuiImageTestShapeRectStroke(pProxy, pTarget, tRect, fWidth, iColor);
}

static int __xuiImageTestFontLoadFile(xui_proxy pProxy, xui_font* ppFont, const char* sPath, float fSize, uint32_t iFlags)
{
	(void)pProxy;
	(void)ppFont;
	(void)sPath;
	(void)fSize;
	(void)iFlags;
	return XUI_ERROR_UNSUPPORTED;
}

static int __xuiImageTestFontLoadMemory(xui_proxy pProxy, xui_font* ppFont, const void* pData, int iSize, float fSize, uint32_t iFlags)
{
	(void)pProxy;
	(void)ppFont;
	(void)pData;
	(void)iSize;
	(void)fSize;
	(void)iFlags;
	return XUI_ERROR_UNSUPPORTED;
}

static int __xuiImageTestFontGetMetrics(xui_proxy pProxy, xui_font pFont, xui_font_metrics_t* pMetrics)
{
	(void)pProxy;
	(void)pFont;
	if ( pMetrics == NULL ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	memset(pMetrics, 0, sizeof(*pMetrics));
	return XUI_OK;
}

static void __xuiImageTestFontDestroy(xui_proxy pProxy, xui_font pFont)
{
	(void)pProxy;
	(void)pFont;
}

static int __xuiImageTestTextMeasure(xui_proxy pProxy, xui_font pFont, const char* sText, xui_vec2_t* pSize)
{
	(void)pProxy;
	(void)pFont;
	(void)sText;
	if ( pSize == NULL ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pSize->fX = 0.0f;
	pSize->fY = 0.0f;
	return XUI_OK;
}

static int __xuiImageTestTextDraw(xui_proxy pProxy, xui_surface pTarget, xui_font pFont, const char* sText, xui_rect_t tRect, uint32_t iColor, uint32_t iFlags)
{
	(void)pProxy;
	(void)pFont;
	(void)sText;
	(void)tRect;
	(void)iColor;
	(void)iFlags;
	return __xuiImageTestSurfaceValid(pTarget) ? XUI_OK : XUI_ERROR_INVALID_ARGUMENT;
}

static void __xuiImageTestInitProxy(xui_image_test_state_t* pState)
{
	memset(pState, 0, sizeof(*pState));
	pState->tProxy.iSize = sizeof(pState->tProxy);
	pState->tProxy.iVersion = XUI_PROXY_VERSION;
	pState->tProxy.pUser = pState;
	pState->tProxy.getCaps = __xuiImageTestGetCaps;
	pState->tProxy.clipboardSetText = __xuiImageTestClipboardSetText;
	pState->tProxy.clipboardGetText = __xuiImageTestClipboardGetText;
	pState->tProxy.imeGetEnabled = __xuiImageTestImeGetEnabled;
	pState->tProxy.imeSetEnabled = __xuiImageTestImeSetEnabled;
	pState->tProxy.imeSetCandidateRect = __xuiImageTestImeSetCandidateRect;
	pState->tProxy.surfaceCreate = __xuiImageTestSurfaceCreate;
	pState->tProxy.surfaceCreateRGBA = __xuiImageTestSurfaceCreateRGBA;
	pState->tProxy.surfaceLoadFile = __xuiImageTestSurfaceLoadFile;
	pState->tProxy.surfaceLoadMemory = __xuiImageTestSurfaceLoadMemory;
	pState->tProxy.surfaceUpdateRGBA = __xuiImageTestSurfaceUpdateRGBA;
	pState->tProxy.surfaceReadRGBA = __xuiImageTestSurfaceReadRGBA;
	pState->tProxy.surfaceGetDesc = __xuiImageTestSurfaceGetDesc;
	pState->tProxy.surfaceDraw = __xuiImageTestSurfaceDraw;
	pState->tProxy.surfaceClear = __xuiImageTestSurfaceClear;
	pState->tProxy.surfaceClearRect = __xuiImageTestSurfaceClearRect;
	pState->tProxy.surfaceDrawTo = __xuiImageTestSurfaceDrawTo;
	pState->tProxy.surfaceDrawQuad = __xuiImageTestSurfaceDrawQuad;
	pState->tProxy.surfaceDrawQuadTo = __xuiImageTestSurfaceDrawQuadTo;
	pState->tProxy.surfaceGetSampler = __xuiImageTestSurfaceGetSampler;
	pState->tProxy.surfaceSetSampler = __xuiImageTestSurfaceSetSampler;
	pState->tProxy.surfaceGetGeneration = __xuiImageTestSurfaceGetGeneration;
	pState->tProxy.surfaceDestroy = __xuiImageTestSurfaceDestroy;
	pState->tProxy.shapePoint = __xuiImageTestShapePoint;
	pState->tProxy.shapeLine = __xuiImageTestShapeLine;
	pState->tProxy.shapeTriangleFill = __xuiImageTestShapeTriangleFill;
	pState->tProxy.shapeTriangleStroke = __xuiImageTestShapeTriangleStroke;
	pState->tProxy.shapeRectFill = __xuiImageTestShapeRectFill;
	pState->tProxy.shapeRectStroke = __xuiImageTestShapeRectStroke;
	pState->tProxy.shapeCircleFill = __xuiImageTestShapeCircleFill;
	pState->tProxy.shapeCircleStroke = __xuiImageTestShapeCircleStroke;
	pState->tProxy.shapeRoundRectFill = __xuiImageTestShapeRoundRectFill;
	pState->tProxy.shapeRoundRectStroke = __xuiImageTestShapeRoundRectStroke;
	pState->tProxy.fontLoadFile = __xuiImageTestFontLoadFile;
	pState->tProxy.fontLoadMemory = __xuiImageTestFontLoadMemory;
	pState->tProxy.fontGetMetrics = __xuiImageTestFontGetMetrics;
	pState->tProxy.fontDestroy = __xuiImageTestFontDestroy;
	pState->tProxy.textMeasure = __xuiImageTestTextMeasure;
	pState->tProxy.textDraw = __xuiImageTestTextDraw;
	pState->tProxy.drawBegin = __xuiImageTestDrawBegin;
	pState->tProxy.drawEnd = __xuiImageTestDrawEnd;
	pState->tProxy.drawClearRect = __xuiImageTestDrawClearRect;
	pState->tProxy.drawSurface = __xuiImageTestDrawSurface;
	pState->tProxy.drawSurfaceQuad = __xuiImageTestDrawSurfaceQuad;
	pState->tProxy.drawPoint = __xuiImageTestDrawPoint;
	pState->tProxy.drawLine = __xuiImageTestDrawLine;
	pState->tProxy.drawTriangleFill = __xuiImageTestDrawTriangleFill;
	pState->tProxy.drawTriangleStroke = __xuiImageTestDrawTriangleStroke;
	pState->tProxy.drawRectFill = __xuiImageTestDrawRectFill;
	pState->tProxy.drawRectStroke = __xuiImageTestDrawRectStroke;
	pState->tProxy.drawCircleFill = __xuiImageTestDrawCircleFill;
	pState->tProxy.drawCircleStroke = __xuiImageTestDrawCircleStroke;
	pState->tProxy.drawRoundRectFill = __xuiImageTestDrawRoundRectFill;
	pState->tProxy.drawRoundRectStroke = __xuiImageTestDrawRoundRectStroke;
	pState->tProxy.drawText = __xuiImageTestDrawText;
}

static int __xuiImageRectEq(xui_rect_t tRect, float fX, float fY, float fW, float fH)
{
	return (tRect.fX == fX) && (tRect.fY == fY) && (tRect.fW == fW) && (tRect.fH == fH);
}

int main(void)
{
	xui_image_test_state_t tState;
	xui_context pContext;
	xui_widget pRoot;
	xui_widget pImage;
	xui_widget pTopLeft;
	xui_surface pSource;
	xui_surface pCache;
	xui_surface pTopLeftCache;
	xui_image_desc_t tDesc;
	xui_vec2_t tMeasured;
	xui_rect_t tRect;
	int iFailed;
	int iRet;
	int iAlignX;
	int iAlignY;

	pContext = NULL;
	pRoot = NULL;
	pImage = NULL;
	pTopLeft = NULL;
	pSource = NULL;
	pCache = NULL;
	pTopLeftCache = NULL;
	iFailed = 0;
	__xuiImageTestInitProxy(&tState);

	iRet = xuiCreate(&pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "context create");
	iRet = xuiSetProxy(pContext, &tState.tProxy);
	XUI_TEST_CHECK(iRet == XUI_OK, "proxy set");
	iRet = xuiInputViewport(pContext, 240.0f, 160.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "viewport");

	iRet = tState.tProxy.surfaceCreateRGBA(&tState.tProxy, &pSource, 64, 32, NULL, 0, XUI_SURFACE_ALPHA_PREMULTIPLIED);
	XUI_TEST_CHECK(iRet == XUI_OK && pSource != NULL, "source create");

	iRet = xuiWidgetCreate(pContext, &pRoot);
	XUI_TEST_CHECK(iRet == XUI_OK && pRoot != NULL, "root create");
	iRet = xuiSetRootWidget(pContext, pRoot);
	XUI_TEST_CHECK(iRet == XUI_OK, "root set");
	iRet = xuiWidgetSetRect(pRoot, (xui_rect_t){0.0f, 0.0f, 240.0f, 160.0f});
	XUI_TEST_CHECK(iRet == XUI_OK, "root rect");

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.pSurface = pSource;
	tDesc.iColor = XUI_COLOR_RGBA(255, 255, 255, 255);
	tDesc.iAlignX = XUI_ALIGN_CENTER;
	tDesc.iAlignY = XUI_ALIGN_CENTER;
	iRet = xuiImageCreate(pContext, &pImage, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pImage != NULL, "image create");
	iRet = xuiWidgetSetRect(pImage, (xui_rect_t){20.0f, 20.0f, 100.0f, 80.0f});
	XUI_TEST_CHECK(iRet == XUI_OK, "image rect");
	iRet = xuiWidgetAddChild(pRoot, pImage);
	XUI_TEST_CHECK(iRet == XUI_OK, "image add");

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.pSurface = pSource;
	tDesc.iColor = XUI_COLOR_RGBA(255, 255, 255, 255);
	tDesc.iAlignX = XUI_ALIGN_START;
	tDesc.iAlignY = XUI_ALIGN_START;
	iRet = xuiImageCreate(pContext, &pTopLeft, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pTopLeft != NULL, "top-left image create");
	iRet = xuiWidgetSetRect(pTopLeft, (xui_rect_t){130.0f, 20.0f, 80.0f, 60.0f});
	XUI_TEST_CHECK(iRet == XUI_OK, "top-left image rect");
	iRet = xuiWidgetAddChild(pRoot, pTopLeft);
	XUI_TEST_CHECK(iRet == XUI_OK, "top-left image add");
	iRet = xuiImageGetAlign(pTopLeft, &iAlignX, &iAlignY);
	XUI_TEST_CHECK(iRet == XUI_OK && iAlignX == XUI_ALIGN_START && iAlignY == XUI_ALIGN_START, "desc start align get");

	iRet = xuiWidgetMeasureContent(pImage, (xui_vec2_t){XUI_LAYOUT_UNBOUNDED, XUI_LAYOUT_UNBOUNDED}, &tMeasured);
	XUI_TEST_CHECK(iRet == XUI_OK && tMeasured.fX == 64.0f && tMeasured.fY == 32.0f, "natural measure");
	iRet = xuiRenderPrepare(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "render prepare natural");
	pCache = xuiWidgetGetCacheSurface(pImage, xuiWidgetGetStateId(pImage));
	XUI_TEST_CHECK(pCache != NULL && pCache->iDrawCount > 0, "cache surface natural");
	XUI_TEST_CHECK(__xuiImageRectEq(pCache->tLastSrc, 0.0f, 0.0f, 64.0f, 32.0f), "natural src");
	XUI_TEST_CHECK(__xuiImageRectEq(pCache->tLastDst, 18.0f, 24.0f, 64.0f, 32.0f), "natural dst center");
	pTopLeftCache = xuiWidgetGetCacheSurface(pTopLeft, xuiWidgetGetStateId(pTopLeft));
	XUI_TEST_CHECK(pTopLeftCache != NULL && pTopLeftCache->iDrawCount > 0, "cache surface top-left");
	XUI_TEST_CHECK(__xuiImageRectEq(pTopLeftCache->tLastDst, 0.0f, 0.0f, 64.0f, 32.0f), "desc start dst");

	iRet = xuiImageSetSourceRect(pImage, 8.0f, 4.0f, 32.0f, 20.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "source rect");
	iRet = xuiWidgetMeasureContent(pImage, (xui_vec2_t){XUI_LAYOUT_UNBOUNDED, XUI_LAYOUT_UNBOUNDED}, &tMeasured);
	XUI_TEST_CHECK(iRet == XUI_OK && tMeasured.fX == 24.0f && tMeasured.fY == 16.0f, "source measure");
	iRet = xuiRenderPrepare(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "render prepare source");
	XUI_TEST_CHECK(__xuiImageRectEq(pCache->tLastSrc, 8.0f, 4.0f, 24.0f, 16.0f), "source src");
	XUI_TEST_CHECK(__xuiImageRectEq(pCache->tLastDst, 38.0f, 32.0f, 24.0f, 16.0f), "source dst");

	iRet = xuiImageClearSource(pImage);
	XUI_TEST_CHECK(iRet == XUI_OK, "clear source");
	iRet = xuiImageSetMode(pImage, XUI_IMAGE_STRETCH);
	XUI_TEST_CHECK(iRet == XUI_OK, "stretch mode");
	iRet = xuiRenderPrepare(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "render prepare stretch");
	XUI_TEST_CHECK(__xuiImageRectEq(pCache->tLastDst, 0.0f, 0.0f, 100.0f, 80.0f), "stretch dst");

	iRet = xuiImageSetMode(pImage, XUI_IMAGE_CONTAIN);
	XUI_TEST_CHECK(iRet == XUI_OK, "contain mode");
	iRet = xuiRenderPrepare(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "render prepare contain");
	XUI_TEST_CHECK(__xuiImageRectEq(pCache->tLastDst, 0.0f, 15.0f, 100.0f, 50.0f), "contain dst");

	iRet = xuiImageSetMode(pImage, XUI_IMAGE_COVER);
	XUI_TEST_CHECK(iRet == XUI_OK, "cover mode");
	iRet = xuiRenderPrepare(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "render prepare cover");
	XUI_TEST_CHECK(__xuiImageRectEq(pCache->tLastDst, -30.0f, 0.0f, 160.0f, 80.0f), "cover dst");

	iRet = xuiImageSetMode(pImage, XUI_IMAGE_SCALE_DOWN);
	XUI_TEST_CHECK(iRet == XUI_OK, "scale down mode");
	iRet = xuiRenderPrepare(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "render prepare scale down");
	XUI_TEST_CHECK(__xuiImageRectEq(pCache->tLastDst, 18.0f, 24.0f, 64.0f, 32.0f), "scale down dst");

	iRet = xuiImageSetAlign(pImage, XUI_ALIGN_END, XUI_ALIGN_START);
	XUI_TEST_CHECK(iRet == XUI_OK, "align set");
	iRet = xuiImageGetAlign(pImage, &iAlignX, &iAlignY);
	XUI_TEST_CHECK(iRet == XUI_OK && iAlignX == XUI_ALIGN_END && iAlignY == XUI_ALIGN_START, "align get");
	iRet = xuiRenderPrepare(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "render prepare align");
	XUI_TEST_CHECK(__xuiImageRectEq(pCache->tLastDst, 36.0f, 0.0f, 64.0f, 32.0f), "align dst");

	iRet = xuiImageSetCustomRect(pImage, 5.0f, 7.0f, 35.0f, 27.0f);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiImageGetMode(pImage) == XUI_IMAGE_CUSTOM, "custom rect set");
	tRect = xuiImageGetCustomRect(pImage);
	XUI_TEST_CHECK(__xuiImageRectEq(tRect, 5.0f, 7.0f, 30.0f, 20.0f), "custom rect get");
	iRet = xuiRenderPrepare(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "render prepare custom");
	XUI_TEST_CHECK(__xuiImageRectEq(pCache->tLastDst, 5.0f, 7.0f, 30.0f, 20.0f), "custom dst");

	iRet = xuiImageSetTint(pImage, XUI_COLOR_RGBA(120, 190, 255, 220));
	XUI_TEST_CHECK(iRet == XUI_OK && xuiImageGetColor(pImage) == XUI_COLOR_RGBA(120, 190, 255, 220), "tint set");
	iRet = xuiRenderPrepare(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "render prepare tint");
	XUI_TEST_CHECK(pCache->iLastColor == XUI_COLOR_RGBA(120, 190, 255, 220), "tint draw color");

	tRect = xuiImageGetDrawRect(pImage);
	XUI_TEST_CHECK(__xuiImageRectEq(tRect, 5.0f, 7.0f, 30.0f, 20.0f), "draw rect get");
	iRet = xuiImageSetSurface(pImage, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiImageGetSurface(pImage) == NULL, "surface clear");

cleanup:
	if ( pSource != NULL ) {
		tState.tProxy.surfaceDestroy(&tState.tProxy, pSource);
	}
	if ( pContext != NULL ) {
		xuiDestroy(pContext);
	}
	if ( iFailed ) {
		return 1;
	}
	printf("xui_image_test passed\n");
	return 0;
}
