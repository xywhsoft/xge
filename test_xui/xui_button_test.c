#include "xui.h"

#include <stdio.h>
#include <string.h>

#define XUI_TEST_CHECK(expr, msg) \
	do { \
		if ( !(expr) ) { \
			printf("xui_button_test failed: %s\n", (msg)); \
			iFailed = 1; \
			goto cleanup; \
		} \
	} while ( 0 )

#define XUI_TEST_SURFACE_MAGIC 0x42535552u
#define XUI_TEST_DRAW_MAGIC 0x42445257u
#define XUI_TEST_FONT_MAGIC 0x42464f4eu

struct xui_surface_t {
	uint32_t iMagic;
	xui_surface_desc_t tDesc;
	uint32_t iGeneration;
	uint32_t iLastFillColor;
	int iFillCount;
};

struct xui_draw_context_t {
	uint32_t iMagic;
	xui_surface pTarget;
};

struct xui_font_t {
	uint32_t iMagic;
	float fSize;
};

typedef struct xui_button_test_state_t {
	xui_proxy_t tProxy;
	int iDrawTextCount;
	int iDrawSurfaceCount;
	int iFillCount;
	int iStrokeCount;
	int iCircleCount;
	char sLastText[128];
} xui_button_test_state_t;

static int __xuiButtonTestSurfaceValid(xui_surface pSurface)
{
	return (pSurface != NULL) && (pSurface->iMagic == XUI_TEST_SURFACE_MAGIC);
}

static int __xuiButtonTestDrawValid(xui_draw_context pDraw)
{
	return (pDraw != NULL) && (pDraw->iMagic == XUI_TEST_DRAW_MAGIC) && __xuiButtonTestSurfaceValid(pDraw->pTarget);
}

static int __xuiButtonTestFontValid(xui_font pFont)
{
	return (pFont != NULL) && (pFont->iMagic == XUI_TEST_FONT_MAGIC);
}

static xui_button_test_state_t* __xuiButtonTestState(xui_proxy pProxy)
{
	return (pProxy != NULL) ? (xui_button_test_state_t*)pProxy->pUser : NULL;
}

static int __xuiButtonTestGetCaps(xui_proxy pProxy, xui_proxy_caps_t* pCaps)
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

static int __xuiButtonTestClipboardSetText(xui_proxy pProxy, const char* sText)
{
	(void)pProxy;
	(void)sText;
	return XUI_OK;
}

static int __xuiButtonTestClipboardGetText(xui_proxy pProxy, char* sText, int iCapacity)
{
	(void)pProxy;
	if ( (sText != NULL) && (iCapacity > 0) ) {
		sText[0] = '\0';
	}
	return 0;
}

static int __xuiButtonTestImeGetEnabled(xui_proxy pProxy)
{
	(void)pProxy;
	return 0;
}

static int __xuiButtonTestImeSetEnabled(xui_proxy pProxy, int bEnabled)
{
	(void)pProxy;
	(void)bEnabled;
	return XUI_OK;
}

static int __xuiButtonTestImeSetCandidateRect(xui_proxy pProxy, xui_rect_t tRect)
{
	(void)pProxy;
	(void)tRect;
	return XUI_OK;
}

static int __xuiButtonTestSurfaceCreate(xui_proxy pProxy, xui_surface* ppSurface, const xui_surface_desc_t* pDesc)
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

static int __xuiButtonTestSurfaceCreateRGBA(xui_proxy pProxy, xui_surface* ppSurface, int iWidth, int iHeight, const void* pPixels, int iStride, uint32_t iFlags)
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
	return __xuiButtonTestSurfaceCreate(pProxy, ppSurface, &tDesc);
}

static int __xuiButtonTestSurfaceUnsupported(xui_proxy pProxy, xui_surface* ppSurface, const char* sPath, uint32_t iFlags)
{
	(void)pProxy;
	(void)ppSurface;
	(void)sPath;
	(void)iFlags;
	return XUI_ERROR_UNSUPPORTED;
}

static int __xuiButtonTestSurfaceLoadMemory(xui_proxy pProxy, xui_surface* ppSurface, const void* pData, int iSize, uint32_t iFlags)
{
	(void)pData;
	(void)iSize;
	return __xuiButtonTestSurfaceCreateRGBA(pProxy, ppSurface, 8, 8, NULL, 0, iFlags);
}

static int __xuiButtonTestSurfaceUpdateRGBA(xui_proxy pProxy, xui_surface pSurface, xui_rect_i_t tRect, const void* pPixels, int iStride)
{
	(void)pProxy;
	(void)tRect;
	(void)pPixels;
	(void)iStride;
	if ( !__xuiButtonTestSurfaceValid(pSurface) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pSurface->iGeneration++;
	return XUI_OK;
}

static int __xuiButtonTestSurfaceReadRGBA(xui_proxy pProxy, xui_surface pSurface, void* pPixels, int iStride)
{
	(void)pProxy;
	(void)pPixels;
	(void)iStride;
	return __xuiButtonTestSurfaceValid(pSurface) ? XUI_OK : XUI_ERROR_INVALID_ARGUMENT;
}

static int __xuiButtonTestSurfaceGetDesc(xui_proxy pProxy, xui_surface pSurface, xui_surface_desc_t* pDesc)
{
	(void)pProxy;
	if ( !__xuiButtonTestSurfaceValid(pSurface) || (pDesc == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	*pDesc = pSurface->tDesc;
	return XUI_OK;
}

static int __xuiButtonTestSurfaceDraw(xui_proxy pProxy, xui_surface pSurface, xui_rect_t tSrc, xui_rect_t tDst, uint32_t iColor, uint32_t iFlags)
{
	xui_button_test_state_t* pState;

	(void)tSrc;
	(void)tDst;
	(void)iColor;
	(void)iFlags;
	if ( !__xuiButtonTestSurfaceValid(pSurface) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pState = __xuiButtonTestState(pProxy);
	if ( pState != NULL ) {
		pState->iDrawSurfaceCount++;
	}
	return XUI_OK;
}

static int __xuiButtonTestSurfaceClear(xui_proxy pProxy, xui_surface pTarget, uint32_t iColor)
{
	(void)pProxy;
	(void)iColor;
	if ( !__xuiButtonTestSurfaceValid(pTarget) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pTarget->iGeneration++;
	return XUI_OK;
}

static int __xuiButtonTestSurfaceClearRect(xui_proxy pProxy, xui_surface pTarget, xui_rect_t tRect, uint32_t iColor)
{
	(void)tRect;
	return __xuiButtonTestSurfaceClear(pProxy, pTarget, iColor);
}

static int __xuiButtonTestSurfaceDrawTo(xui_proxy pProxy, xui_surface pTarget, xui_surface pSurface, xui_rect_t tSrc, xui_rect_t tDst, uint32_t iColor, uint32_t iFlags)
{
	if ( !__xuiButtonTestSurfaceValid(pTarget) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	return __xuiButtonTestSurfaceDraw(pProxy, pSurface, tSrc, tDst, iColor, iFlags);
}

static int __xuiButtonTestSurfaceDrawQuad(xui_proxy pProxy, xui_surface pSurface, const xui_surface_vertex_t* pVertices, uint32_t iFlags)
{
	(void)pVertices;
	return __xuiButtonTestSurfaceDraw(pProxy, pSurface, (xui_rect_t){0, 0, 1, 1}, (xui_rect_t){0, 0, 1, 1}, XUI_COLOR_WHITE, iFlags);
}

static int __xuiButtonTestSurfaceDrawQuadTo(xui_proxy pProxy, xui_surface pTarget, xui_surface pSurface, const xui_surface_vertex_t* pVertices, uint32_t iFlags)
{
	(void)pTarget;
	return __xuiButtonTestSurfaceDrawQuad(pProxy, pSurface, pVertices, iFlags);
}

static int __xuiButtonTestSurfaceGetSampler(xui_proxy pProxy, xui_surface pSurface, xui_surface_sampler_t* pSampler)
{
	(void)pProxy;
	if ( !__xuiButtonTestSurfaceValid(pSurface) || (pSampler == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pSampler->iMinFilter = XUI_SURFACE_FILTER_NEAREST;
	pSampler->iMagFilter = XUI_SURFACE_FILTER_NEAREST;
	pSampler->iWrapS = XUI_SURFACE_WRAP_CLAMP;
	pSampler->iWrapT = XUI_SURFACE_WRAP_CLAMP;
	return XUI_OK;
}

static int __xuiButtonTestSurfaceSetSampler(xui_proxy pProxy, xui_surface pSurface, const xui_surface_sampler_t* pSampler)
{
	(void)pProxy;
	(void)pSampler;
	return __xuiButtonTestSurfaceValid(pSurface) ? XUI_OK : XUI_ERROR_INVALID_ARGUMENT;
}

static int __xuiButtonTestSurfaceGetGeneration(xui_proxy pProxy, xui_surface pSurface, uint32_t* pGeneration)
{
	(void)pProxy;
	if ( !__xuiButtonTestSurfaceValid(pSurface) || (pGeneration == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	*pGeneration = pSurface->iGeneration;
	return XUI_OK;
}

static void __xuiButtonTestSurfaceDestroy(xui_proxy pProxy, xui_surface pSurface)
{
	(void)pProxy;
	if ( __xuiButtonTestSurfaceValid(pSurface) ) {
		pSurface->iMagic = 0;
		xrtFree(pSurface);
	}
}

static int __xuiButtonTestDrawBegin(xui_proxy pProxy, xui_draw_context* ppDraw, xui_surface pTarget)
{
	xui_draw_context pDraw;

	(void)pProxy;
	if ( (ppDraw == NULL) || !__xuiButtonTestSurfaceValid(pTarget) ) {
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

static int __xuiButtonTestDrawEnd(xui_proxy pProxy, xui_draw_context pDraw)
{
	(void)pProxy;
	if ( !__xuiButtonTestDrawValid(pDraw) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pDraw->pTarget->iGeneration++;
	pDraw->iMagic = 0;
	xrtFree(pDraw);
	return XUI_OK;
}

static int __xuiButtonTestDrawClearRect(xui_proxy pProxy, xui_draw_context pDraw, xui_rect_t tRect, uint32_t iColor)
{
	(void)pProxy;
	(void)tRect;
	(void)iColor;
	return __xuiButtonTestDrawValid(pDraw) ? XUI_OK : XUI_ERROR_INVALID_ARGUMENT;
}

static int __xuiButtonTestDrawSurface(xui_proxy pProxy, xui_draw_context pDraw, xui_surface pSurface, xui_rect_t tSrc, xui_rect_t tDst, uint32_t iColor, uint32_t iFlags)
{
	if ( !__xuiButtonTestDrawValid(pDraw) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	return __xuiButtonTestSurfaceDraw(pProxy, pSurface, tSrc, tDst, iColor, iFlags);
}

static int __xuiButtonTestDrawSurfaceQuad(xui_proxy pProxy, xui_draw_context pDraw, xui_surface pSurface, const xui_surface_vertex_t* pVertices, uint32_t iFlags)
{
	if ( !__xuiButtonTestDrawValid(pDraw) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	return __xuiButtonTestSurfaceDrawQuad(pProxy, pSurface, pVertices, iFlags);
}

static int __xuiButtonTestShapePointTarget(xui_proxy pProxy, xui_surface pTarget, float fX, float fY, float fSize, uint32_t iColor)
{
	(void)pProxy;
	(void)fX;
	(void)fY;
	(void)fSize;
	(void)iColor;
	return __xuiButtonTestSurfaceValid(pTarget) ? XUI_OK : XUI_ERROR_INVALID_ARGUMENT;
}

static int __xuiButtonTestShapeLineTarget(xui_proxy pProxy, xui_surface pTarget, float fX0, float fY0, float fX1, float fY1, float fWidth, uint32_t iColor)
{
	(void)pProxy;
	(void)fX0;
	(void)fY0;
	(void)fX1;
	(void)fY1;
	(void)fWidth;
	(void)iColor;
	return __xuiButtonTestSurfaceValid(pTarget) ? XUI_OK : XUI_ERROR_INVALID_ARGUMENT;
}

static int __xuiButtonTestShapeTriangleFillTarget(xui_proxy pProxy, xui_surface pTarget, xui_vec2_t tA, xui_vec2_t tB, xui_vec2_t tC, uint32_t iColor)
{
	(void)pProxy;
	(void)tA;
	(void)tB;
	(void)tC;
	(void)iColor;
	return __xuiButtonTestSurfaceValid(pTarget) ? XUI_OK : XUI_ERROR_INVALID_ARGUMENT;
}

static int __xuiButtonTestShapeTriangleStrokeTarget(xui_proxy pProxy, xui_surface pTarget, xui_vec2_t tA, xui_vec2_t tB, xui_vec2_t tC, float fWidth, uint32_t iColor)
{
	(void)pProxy;
	(void)tA;
	(void)tB;
	(void)tC;
	(void)fWidth;
	(void)iColor;
	return __xuiButtonTestSurfaceValid(pTarget) ? XUI_OK : XUI_ERROR_INVALID_ARGUMENT;
}

static int __xuiButtonTestShapeRectFillTarget(xui_proxy pProxy, xui_surface pTarget, xui_rect_t tRect, uint32_t iColor)
{
	(void)pProxy;
	(void)tRect;
	(void)iColor;
	return __xuiButtonTestSurfaceValid(pTarget) ? XUI_OK : XUI_ERROR_INVALID_ARGUMENT;
}

static int __xuiButtonTestShapeRectStrokeTarget(xui_proxy pProxy, xui_surface pTarget, xui_rect_t tRect, float fWidth, uint32_t iColor)
{
	(void)pProxy;
	(void)tRect;
	(void)fWidth;
	(void)iColor;
	return __xuiButtonTestSurfaceValid(pTarget) ? XUI_OK : XUI_ERROR_INVALID_ARGUMENT;
}

static int __xuiButtonTestShapeCircleFillTarget(xui_proxy pProxy, xui_surface pTarget, float fX, float fY, float fRadius, uint32_t iColor)
{
	(void)pProxy;
	(void)fX;
	(void)fY;
	(void)fRadius;
	(void)iColor;
	return __xuiButtonTestSurfaceValid(pTarget) ? XUI_OK : XUI_ERROR_INVALID_ARGUMENT;
}

static int __xuiButtonTestShapeCircleStrokeTarget(xui_proxy pProxy, xui_surface pTarget, float fX, float fY, float fRadius, float fWidth, uint32_t iColor)
{
	(void)pProxy;
	(void)fX;
	(void)fY;
	(void)fRadius;
	(void)fWidth;
	(void)iColor;
	return __xuiButtonTestSurfaceValid(pTarget) ? XUI_OK : XUI_ERROR_INVALID_ARGUMENT;
}



static int __xuiButtonTestDrawPoint(xui_proxy pProxy, xui_draw_context pDraw, float fX, float fY, float fSize, uint32_t iColor)
{
	(void)pProxy;
	(void)fX;
	(void)fY;
	(void)fSize;
	(void)iColor;
	return __xuiButtonTestDrawValid(pDraw) ? XUI_OK : XUI_ERROR_INVALID_ARGUMENT;
}

static int __xuiButtonTestDrawLine(xui_proxy pProxy, xui_draw_context pDraw, float fX0, float fY0, float fX1, float fY1, float fWidth, uint32_t iColor)
{
	(void)pProxy;
	(void)fX0;
	(void)fY0;
	(void)fX1;
	(void)fY1;
	(void)fWidth;
	(void)iColor;
	return __xuiButtonTestDrawValid(pDraw) ? XUI_OK : XUI_ERROR_INVALID_ARGUMENT;
}

static int __xuiButtonTestDrawTriangleFill(xui_proxy pProxy, xui_draw_context pDraw, xui_vec2_t tA, xui_vec2_t tB, xui_vec2_t tC, uint32_t iColor)
{
	(void)pProxy;
	(void)tA;
	(void)tB;
	(void)tC;
	(void)iColor;
	return __xuiButtonTestDrawValid(pDraw) ? XUI_OK : XUI_ERROR_INVALID_ARGUMENT;
}

static int __xuiButtonTestDrawTriangleStroke(xui_proxy pProxy, xui_draw_context pDraw, xui_vec2_t tA, xui_vec2_t tB, xui_vec2_t tC, float fWidth, uint32_t iColor)
{
	(void)pProxy;
	(void)tA;
	(void)tB;
	(void)tC;
	(void)fWidth;
	(void)iColor;
	return __xuiButtonTestDrawValid(pDraw) ? XUI_OK : XUI_ERROR_INVALID_ARGUMENT;
}

static int __xuiButtonTestShapeRectFill(xui_proxy pProxy, xui_draw_context pDraw, xui_rect_t tRect, uint32_t iColor)
{
	xui_button_test_state_t* pState;

	(void)tRect;
	if ( !__xuiButtonTestDrawValid(pDraw) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pDraw->pTarget->iLastFillColor = iColor;
	pDraw->pTarget->iFillCount++;
	pState = __xuiButtonTestState(pProxy);
	if ( pState != NULL ) {
		pState->iFillCount++;
	}
	return XUI_OK;
}

static int __xuiButtonTestShapeRectStroke(xui_proxy pProxy, xui_draw_context pDraw, xui_rect_t tRect, float fWidth, uint32_t iColor)
{
	xui_button_test_state_t* pState;

	(void)tRect;
	(void)fWidth;
	(void)iColor;
	if ( !__xuiButtonTestDrawValid(pDraw) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pState = __xuiButtonTestState(pProxy);
	if ( pState != NULL ) {
		pState->iStrokeCount++;
	}
	return XUI_OK;
}

static int __xuiButtonTestShapeCircleFill(xui_proxy pProxy, xui_draw_context pDraw, float fX, float fY, float fRadius, uint32_t iColor)
{
	xui_button_test_state_t* pState;

	(void)fX;
	(void)fY;
	(void)fRadius;
	(void)iColor;
	if ( !__xuiButtonTestDrawValid(pDraw) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pState = __xuiButtonTestState(pProxy);
	if ( pState != NULL ) {
		pState->iCircleCount++;
	}
	return XUI_OK;
}

static int __xuiButtonTestShapeCircleStroke(xui_proxy pProxy, xui_draw_context pDraw, float fX, float fY, float fRadius, float fWidth, uint32_t iColor)
{
	(void)fX;
	(void)fY;
	(void)fRadius;
	(void)fWidth;
	(void)iColor;
	return __xuiButtonTestShapeCircleFill(pProxy, pDraw, 0.0f, 0.0f, 1.0f, XUI_COLOR_WHITE);
}



static int __xuiButtonTestDrawText(xui_proxy pProxy, xui_draw_context pDraw, xui_font pFont, const char* sText, xui_rect_t tRect, uint32_t iColor, uint32_t iFlags)
{
	xui_button_test_state_t* pState;

	(void)tRect;
	(void)iColor;
	(void)iFlags;
	if ( !__xuiButtonTestDrawValid(pDraw) || !__xuiButtonTestFontValid(pFont) || (sText == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pState = __xuiButtonTestState(pProxy);
	if ( pState != NULL ) {
		pState->iDrawTextCount++;
		strncpy(pState->sLastText, sText, sizeof(pState->sLastText) - 1);
		pState->sLastText[sizeof(pState->sLastText) - 1] = '\0';
	}
	return XUI_OK;
}

static int __xuiButtonTestTextMeasure(xui_proxy pProxy, xui_font pFont, const char* sText, xui_vec2_t* pSize)
{
	(void)pProxy;
	if ( !__xuiButtonTestFontValid(pFont) || (sText == NULL) || (pSize == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pSize->fX = (float)strlen(sText) * 8.0f;
	pSize->fY = pFont->fSize;
	return XUI_OK;
}

static int __xuiButtonTestFontLoadFile(xui_proxy pProxy, xui_font* ppFont, const char* sPath, float fSize, uint32_t iFlags)
{
	xui_font pFont;

	(void)pProxy;
	(void)sPath;
	(void)iFlags;
	if ( (ppFont == NULL) || (fSize <= 0.0f) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	*ppFont = NULL;
	pFont = (xui_font)xrtCalloc(1, sizeof(*pFont));
	if ( pFont == NULL ) {
		return XUI_ERROR_OUT_OF_MEMORY;
	}
	pFont->iMagic = XUI_TEST_FONT_MAGIC;
	pFont->fSize = fSize;
	*ppFont = pFont;
	return XUI_OK;
}

static int __xuiButtonTestFontLoadMemory(xui_proxy pProxy, xui_font* ppFont, const void* pData, int iSize, float fSize, uint32_t iFlags)
{
	(void)pData;
	(void)iSize;
	return __xuiButtonTestFontLoadFile(pProxy, ppFont, "", fSize, iFlags);
}

static int __xuiButtonTestFontGetMetrics(xui_proxy pProxy, xui_font pFont, xui_font_metrics_t* pMetrics)
{
	(void)pProxy;
	if ( !__xuiButtonTestFontValid(pFont) || (pMetrics == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	memset(pMetrics, 0, sizeof(*pMetrics));
	pMetrics->fSize = pFont->fSize;
	pMetrics->fAscent = pFont->fSize * 0.8f;
	pMetrics->fDescent = pFont->fSize * 0.2f;
	pMetrics->fLineHeight = pFont->fSize;
	return XUI_OK;
}

static void __xuiButtonTestFontDestroy(xui_proxy pProxy, xui_font pFont)
{
	(void)pProxy;
	if ( __xuiButtonTestFontValid(pFont) ) {
		pFont->iMagic = 0;
		xrtFree(pFont);
	}
}

static int __xuiButtonTestTextDraw(xui_proxy pProxy, xui_surface pTarget, xui_font pFont, const char* sText, xui_rect_t tRect, uint32_t iColor, uint32_t iFlags)
{
	(void)pProxy;
	(void)sText;
	(void)tRect;
	(void)iColor;
	(void)iFlags;
	if ( !__xuiButtonTestSurfaceValid(pTarget) || !__xuiButtonTestFontValid(pFont) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	return XUI_OK;
}

static void __xuiButtonTestInitProxy(xui_button_test_state_t* pState)
{
	xui_proxy_t* pProxy;

	memset(pState, 0, sizeof(*pState));
	pProxy = &pState->tProxy;
	pProxy->iSize = sizeof(*pProxy);
	pProxy->iVersion = XUI_PROXY_VERSION;
	pProxy->pUser = pState;
	pProxy->getCaps = __xuiButtonTestGetCaps;
	pProxy->clipboardSetText = __xuiButtonTestClipboardSetText;
	pProxy->clipboardGetText = __xuiButtonTestClipboardGetText;
	pProxy->imeGetEnabled = __xuiButtonTestImeGetEnabled;
	pProxy->imeSetEnabled = __xuiButtonTestImeSetEnabled;
	pProxy->imeSetCandidateRect = __xuiButtonTestImeSetCandidateRect;
	pProxy->surfaceCreate = __xuiButtonTestSurfaceCreate;
	pProxy->surfaceCreateRGBA = __xuiButtonTestSurfaceCreateRGBA;
	pProxy->surfaceLoadFile = __xuiButtonTestSurfaceUnsupported;
	pProxy->surfaceLoadMemory = __xuiButtonTestSurfaceLoadMemory;
	pProxy->surfaceUpdateRGBA = __xuiButtonTestSurfaceUpdateRGBA;
	pProxy->surfaceReadRGBA = __xuiButtonTestSurfaceReadRGBA;
	pProxy->surfaceGetDesc = __xuiButtonTestSurfaceGetDesc;
	pProxy->surfaceDraw = __xuiButtonTestSurfaceDraw;
	pProxy->surfaceClear = __xuiButtonTestSurfaceClear;
	pProxy->surfaceClearRect = __xuiButtonTestSurfaceClearRect;
	pProxy->surfaceDrawTo = __xuiButtonTestSurfaceDrawTo;
	pProxy->surfaceDrawQuad = __xuiButtonTestSurfaceDrawQuad;
	pProxy->surfaceDrawQuadTo = __xuiButtonTestSurfaceDrawQuadTo;
	pProxy->surfaceGetSampler = __xuiButtonTestSurfaceGetSampler;
	pProxy->surfaceSetSampler = __xuiButtonTestSurfaceSetSampler;
	pProxy->surfaceGetGeneration = __xuiButtonTestSurfaceGetGeneration;
	pProxy->surfaceDestroy = __xuiButtonTestSurfaceDestroy;
	pProxy->shapePoint = __xuiButtonTestShapePointTarget;
	pProxy->shapeLine = __xuiButtonTestShapeLineTarget;
	pProxy->shapeTriangleFill = __xuiButtonTestShapeTriangleFillTarget;
	pProxy->shapeTriangleStroke = __xuiButtonTestShapeTriangleStrokeTarget;
	pProxy->shapeRectFill = __xuiButtonTestShapeRectFillTarget;
	pProxy->shapeRectStroke = __xuiButtonTestShapeRectStrokeTarget;
	pProxy->shapeCircleFill = __xuiButtonTestShapeCircleFillTarget;
	pProxy->shapeCircleStroke = __xuiButtonTestShapeCircleStrokeTarget;
	pProxy->fontLoadFile = __xuiButtonTestFontLoadFile;
	pProxy->fontLoadMemory = __xuiButtonTestFontLoadMemory;
	pProxy->fontGetMetrics = __xuiButtonTestFontGetMetrics;
	pProxy->fontDestroy = __xuiButtonTestFontDestroy;
	pProxy->textMeasure = __xuiButtonTestTextMeasure;
	pProxy->textDraw = __xuiButtonTestTextDraw;
	pProxy->drawBegin = __xuiButtonTestDrawBegin;
	pProxy->drawEnd = __xuiButtonTestDrawEnd;
	pProxy->drawClearRect = __xuiButtonTestDrawClearRect;
	pProxy->drawSurface = __xuiButtonTestDrawSurface;
	pProxy->drawSurfaceQuad = __xuiButtonTestDrawSurfaceQuad;
	pProxy->drawPoint = __xuiButtonTestDrawPoint;
	pProxy->drawLine = __xuiButtonTestDrawLine;
	pProxy->drawTriangleFill = __xuiButtonTestDrawTriangleFill;
	pProxy->drawTriangleStroke = __xuiButtonTestDrawTriangleStroke;
	pProxy->drawRectFill = __xuiButtonTestShapeRectFill;
	pProxy->drawRectStroke = __xuiButtonTestShapeRectStroke;
	pProxy->drawCircleFill = __xuiButtonTestShapeCircleFill;
	pProxy->drawCircleStroke = __xuiButtonTestShapeCircleStroke;
	pProxy->drawText = __xuiButtonTestDrawText;
}

static void __xuiButtonTestClick(xui_widget pWidget, void* pUser)
{
	int* pCount;

	(void)pWidget;
	pCount = (int*)pUser;
	(*pCount)++;
}

int main(void)
{
	xui_button_test_state_t tState;
	xui_context pContext;
	xui_widget pRoot;
	xui_widget pButton;
	xui_surface pIcon;
	xui_surface pFocusSurface;
	xui_button_desc_t tDesc;
	xui_surface_desc_t tSurfaceDesc;
	xui_nine_patch_t tPatch;
	xui_rect_t tRect;
	struct xui_font_t tFont;
	int iFailed;
	int iRet;
	int iClickCount;

	pContext = NULL;
	pRoot = NULL;
	pButton = NULL;
	pIcon = NULL;
	pFocusSurface = NULL;
	iFailed = 0;
	iClickCount = 0;
	__xuiButtonTestInitProxy(&tState);
	tFont.iMagic = XUI_TEST_FONT_MAGIC;
	tFont.fSize = 16.0f;

	iRet = xuiCreate(&pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "context create");
	iRet = xuiSetProxy(pContext, &tState.tProxy);
	XUI_TEST_CHECK(iRet == XUI_OK, "proxy set");
	(void)xuiSetDefaultFont(pContext, &tFont);
	iRet = xuiInputViewport(pContext, 240.0f, 120.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "viewport");

	iRet = xuiWidgetCreate(pContext, &pRoot);
	XUI_TEST_CHECK(iRet == XUI_OK && pRoot != NULL, "root create");
	iRet = xuiSetRootWidget(pContext, pRoot);
	XUI_TEST_CHECK(iRet == XUI_OK, "root set");
	iRet = xuiWidgetSetRect(pRoot, (xui_rect_t){0.0f, 0.0f, 240.0f, 120.0f});
	XUI_TEST_CHECK(iRet == XUI_OK, "root rect");

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.sText = "Press";
	tDesc.pFont = &tFont;
	tDesc.iTextColor = XUI_COLOR_RGBA(12, 24, 40, 255);
	iRet = xuiButtonCreate(pContext, &pButton, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pButton != NULL, "button create");
	iRet = xuiWidgetSetRect(pButton, (xui_rect_t){20.0f, 20.0f, 120.0f, 38.0f});
	XUI_TEST_CHECK(iRet == XUI_OK, "button rect");
	iRet = xuiWidgetAddChild(pRoot, pButton);
	XUI_TEST_CHECK(iRet == XUI_OK, "button add");
	iRet = xuiButtonSetClick(pButton, __xuiButtonTestClick, &iClickCount);
	XUI_TEST_CHECK(iRet == XUI_OK, "click set");
	XUI_TEST_CHECK(strcmp(xuiButtonGetText(pButton), "Press") == 0, "text get");
	XUI_TEST_CHECK(xuiWidgetGetCacheStateCount(pButton) >= 6, "cache state count");

	iRet = xuiRenderPrepare(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "initial render prepare");
	XUI_TEST_CHECK(tState.iDrawTextCount > 0, "button text rendered");
	XUI_TEST_CHECK(tState.iFillCount > 0, "button fill rendered");
	XUI_TEST_CHECK(xuiWidgetGetCacheSurface(pButton, xuiWidgetGetStateId(pButton)) != NULL, "button cache surface");

	memset(&tSurfaceDesc, 0, sizeof(tSurfaceDesc));
	tSurfaceDesc.iKind = XUI_SURFACE_KIND_TEXTURE;
	tSurfaceDesc.iFormat = XUI_SURFACE_FORMAT_RGBA8;
	tSurfaceDesc.iWidth = 32;
	tSurfaceDesc.iHeight = 32;
	iRet = tState.tProxy.surfaceCreate(&tState.tProxy, &pIcon, &tSurfaceDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pIcon != NULL, "icon surface create");
	iRet = xuiButtonSetIcon(pButton, pIcon, (xui_rect_t){0.0f, 0.0f, 32.0f, 32.0f});
	XUI_TEST_CHECK(iRet == XUI_OK, "icon set");
	iRet = xuiButtonSetIconLayout(pButton, XUI_BUTTON_ICON_RIGHT, 18.0f, 5.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "icon layout");
	memset(&tPatch, 0, sizeof(tPatch));
	tPatch.iSize = sizeof(tPatch);
	tPatch.pSurface = pIcon;
	tPatch.tSrc = (xui_rect_t){0.0f, 0.0f, 32.0f, 32.0f};
	tPatch.tSlice = (xui_thickness_t){8.0f, 8.0f, 8.0f, 8.0f};
	tPatch.iColor = XUI_COLOR_WHITE;
	tPatch.iMode = XUI_NINE_PATCH_TILE;
	iRet = xuiButtonSetPatch(pButton, 0, &tPatch);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiButtonHasPatch(pButton, 0), "patch set");
	iRet = xuiButtonSetBadgeVisible(pButton, 1);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiButtonGetBadgeVisible(pButton), "badge set");
	iRet = xuiRenderPrepare(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "patched render prepare");
	XUI_TEST_CHECK(tState.iDrawSurfaceCount > 0, "surface pieces rendered");
	XUI_TEST_CHECK(tState.iCircleCount > 0, "badge rendered");

	iRet = xuiInputPointerMove(pContext, 40.0f, 35.0f, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "pointer move");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "dispatch move");
	XUI_TEST_CHECK((xuiButtonGetState(pButton) & XUI_WIDGET_STATE_HOVER) != 0, "hover state");
	iRet = xuiInputPointerDown(pContext, 40.0f, 35.0f, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	XUI_TEST_CHECK(iRet == XUI_OK, "pointer down");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "dispatch down");
	XUI_TEST_CHECK(xuiGetPointerCapture(pContext) == pButton, "pointer capture");
	XUI_TEST_CHECK((xuiButtonGetState(pButton) & XUI_WIDGET_STATE_ACTIVE) != 0, "active state");
	XUI_TEST_CHECK(xuiWidgetGetStateId(pButton) == XUI_WIDGET_STATE_ACTIVE, "active visual state");
	iRet = xuiRenderPrepare(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "active render prepare");
	XUI_TEST_CHECK(xuiWidgetGetCacheSurface(pButton, xuiWidgetGetStateId(pButton)) != NULL, "active cache surface");
	iRet = xuiInputPointerUp(pContext, 40.0f, 35.0f, XUI_POINTER_BUTTON_LEFT, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "pointer up");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "dispatch up");
	XUI_TEST_CHECK(xuiGetPointerCapture(pContext) == NULL, "capture release");
	XUI_TEST_CHECK(xuiButtonGetClickCount(pButton) == 1 && iClickCount == 1, "click count");
	iRet = xuiButtonClearPatch(pButton, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "clear patch");
	iRet = xuiButtonSetBadgeVisible(pButton, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "hide badge");
	iRet = xuiButtonSetColors(
		pButton,
		XUI_COLOR_RGBA(210, 60, 72, 255),
		XUI_COLOR_RGBA(224, 86, 96, 255),
		XUI_COLOR_RGBA(172, 42, 54, 255),
		XUI_COLOR_RGBA(255, 222, 80, 255),
		XUI_COLOR_RGBA(150, 156, 166, 130));
	XUI_TEST_CHECK(iRet == XUI_OK, "focus colors");
	iRet = xuiInputPointerMove(pContext, 220.0f, 100.0f, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "pointer move out");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "dispatch move out");
	XUI_TEST_CHECK((xuiButtonGetState(pButton) & XUI_WIDGET_STATE_FOCUS) != 0, "focus state after click");
	XUI_TEST_CHECK((xuiButtonGetState(pButton) & XUI_WIDGET_STATE_HOVER) == 0, "hover cleared after move out");
	XUI_TEST_CHECK(xuiWidgetGetStateId(pButton) == XUI_WIDGET_STATE_FOCUS, "focus visual state");
	iRet = xuiRenderPrepare(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "focus render prepare");
	pFocusSurface = xuiWidgetGetCacheSurface(pButton, XUI_WIDGET_STATE_FOCUS);
	XUI_TEST_CHECK(pFocusSurface != NULL, "focus cache surface");
	XUI_TEST_CHECK(pFocusSurface->iFillCount > 0, "focus cache fill");
	XUI_TEST_CHECK(pFocusSurface->iLastFillColor == XUI_COLOR_RGBA(210, 60, 72, 255), "focus cache uses normal fill");

	iRet = xuiButtonSetSelectable(pButton, 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "selectable");
	iRet = xuiButtonSetSelected(pButton, 1);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiButtonIsSelected(pButton), "selected");
	XUI_TEST_CHECK((xuiButtonGetState(pButton) & XUI_BUTTON_STATE_CHECKED) != 0, "checked state");

	iRet = xuiSetFocusWidget(pContext, pButton);
	XUI_TEST_CHECK(iRet == XUI_OK, "focus");
	iRet = xuiInputKeyDown(pContext, XUI_KEY_SPACE, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "space down");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "dispatch space down");
	XUI_TEST_CHECK((xuiButtonGetState(pButton) & XUI_WIDGET_STATE_ACTIVE) != 0, "keyboard active");
	iRet = xuiInputKeyUp(pContext, XUI_KEY_SPACE, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "space up");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "dispatch space up");
	XUI_TEST_CHECK(xuiButtonGetClickCount(pButton) == 2 && iClickCount == 2, "keyboard click");

	iRet = xuiWidgetSetEnabled(pButton, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "disable");
	XUI_TEST_CHECK((xuiButtonGetState(pButton) & XUI_WIDGET_STATE_DISABLED) != 0, "disabled state");
	iRet = xuiRenderPrepare(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "disabled render prepare");
	tRect = xuiWidgetGetRect(pButton);
	XUI_TEST_CHECK(tRect.fX == 20.0f && tRect.fY == 20.0f && tRect.fW == 120.0f && tRect.fH == 38.0f, "button rect stable");

cleanup:
	if ( pIcon != NULL ) {
		tState.tProxy.surfaceDestroy(&tState.tProxy, pIcon);
	}
	if ( pContext != NULL ) {
		xuiDestroy(pContext);
	}
	if ( iFailed ) {
		return 1;
	}
	printf("xui_button_test passed\n");
	return 0;
}
