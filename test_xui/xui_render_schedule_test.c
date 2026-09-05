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
	xui_widget pFailWidget;
	int iErrorCount;
	int iLastError;
	int iLastStage;
	int bLastRecoverable;
	xui_widget pLastErrorWidget;
	xui_widget pFailUpdateWidget;
	xui_widget pDestroyUpdateWidget;
	int iHealthyUpdateCount;
	int iDestroyUpdateCount;
	xui_widget pFailEventWidget;
	int iHealthyEventCount;
} xui_render_schedule_test_state_t;

static xui_render_schedule_test_state_t* g_pXuiUpdateTestState;

static void __xuiTestError(xui_context pContext, const xui_error_info_t* pError, void* pUser)
{
	xui_render_schedule_test_state_t* pState;

	(void)pContext;
	pState = (xui_render_schedule_test_state_t*)pUser;
	if ( (pState == NULL) || (pError == NULL) ) {
		return;
	}
	pState->iErrorCount++;
	pState->iLastError = pError->iCode;
	pState->iLastStage = pError->iStage;
	pState->bLastRecoverable = pError->bRecoverable;
	pState->pLastErrorWidget = pError->pWidget;
}

static int __xuiTestUpdate(xui_widget pWidget, float fDelta, void* pUser)
{
	xui_render_schedule_test_state_t* pState;

	(void)fDelta;
	(void)pUser;
	pState = g_pXuiUpdateTestState;
	if ( pState == NULL ) {
		return XUI_ERROR_NOT_INITIALIZED;
	}
	if ( pState->pFailUpdateWidget == pWidget ) {
		return XUI_ERROR;
	}
	if ( pState->pDestroyUpdateWidget == pWidget ) {
		pState->iDestroyUpdateCount++;
		xuiWidgetDestroy(pWidget);
		return XUI_OK;
	}
	pState->iHealthyUpdateCount++;
	return XUI_OK;
}

static int __xuiTestPendingEvent(xui_widget pWidget, const xui_event_t* pEvent, void* pUser)
{
	xui_render_schedule_test_state_t* pState;

	pState = (xui_render_schedule_test_state_t*)pUser;
	if ( (pState->pFailEventWidget == pWidget) && (pEvent->iType == XUI_EVENT_VIEWPORT) ) {
		return XUI_ERROR_RESOURCE_FAILED;
	}
	pState->iHealthyEventCount++;
	return XUI_OK;
}

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
	unsigned int a = iColor & 0xFFu;
	pPixel[0] = (unsigned char)((((iColor >> 24) & 0xFFu) * a + 127u) / 255u);
	pPixel[1] = (unsigned char)((((iColor >> 16) & 0xFFu) * a + 127u) / 255u);
	pPixel[2] = (unsigned char)((((iColor >> 8) & 0xFFu) * a + 127u) / 255u);
	pPixel[3] = (unsigned char)a;
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
	int c;
	unsigned char* pDst;
	const unsigned char* pSrc;

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
			pDst = pTarget->pPixels + ((((iDstY + y) * pTarget->tDesc.iWidth) + (iDstX + x)) * 4);
			pSrc = pSurface->pPixels + ((((iSrcY + y) * pSurface->tDesc.iWidth) + (iSrcX + x)) * 4);
			for ( c = 0; c < 4; ++c ) {
				pDst[c] = (unsigned char)(pSrc[c] + (pDst[c] * (255u - pSrc[3]) + 127u) / 255u);
			}
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
	               XUI_PROXY_CAP_FONT_XRF | XUI_PROXY_CAP_TEXT |
	               XUI_PROXY_CAP_PATH_FILL | XUI_PROXY_CAP_PATH_STROKE |
	               XUI_PROXY_CAP_PATH_DASH | XUI_PROXY_CAP_PATH_AA;
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

static int __xuiTestDrawPath(xui_proxy pProxy, xui_draw_context pDraw, const xui_path_command_t* pCommands, int iCommandCount, const xui_path_style_t* pStyle, float fTolerance)
{
	(void)pProxy;
	(void)pDraw;
	(void)pCommands;
	(void)iCommandCount;
	(void)pStyle;
	(void)fTolerance;
	return XUI_OK;
}

static int __xuiTestDrawSvgPath(xui_proxy pProxy, xui_draw_context pDraw, const char* sPath, xui_rect_t tViewBox, xui_rect_t tTarget, const xui_path_style_t* pStyle, float fTolerance)
{
	(void)pProxy;
	(void)pDraw;
	(void)sPath;
	(void)tViewBox;
	(void)tTarget;
	(void)pStyle;
	(void)fTolerance;
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

static int __xuiTestZstdDecompress(xui_proxy pProxy, void* pOutput, int iOutputCapacity, const void* pInput, int iInputSize, int* pOutputSize)
{
	(void)pProxy;
	(void)pInput;
	(void)iInputSize;
	if ( (pOutput == NULL) || (iOutputCapacity <= 0) || (pOutputSize == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	memset(pOutput, 0, (size_t)iOutputCapacity);
	*pOutputSize = iOutputCapacity;
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
	tProxy.zstdDecompress = __xuiTestZstdDecompress;
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
	tProxy.drawPath = __xuiTestDrawPath;
	tProxy.drawSvgPath = __xuiTestDrawSvgPath;
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
	if ( pState->pFailWidget == pWidget ) {
		return XUI_ERROR_BACKEND_FAILED;
	}
	tRect = xuiWidgetGetRect(pWidget);
	tRect.fX = 0.0f;
	tRect.fY = 0.0f;
	iColor = (iStateId == 1) ? XUI_COLOR_RGBA(0, 255, 0, 255) : XUI_COLOR_RGBA(255, 0, 0, 255);
	pState->iRenderCount++;
	return pState->tProxy.drawRectFill(&pState->tProxy, pDraw, tRect, iColor);
}

static int __xuiRenderScheduleDestroySelf(xui_widget pWidget, xui_draw_context pDraw,
	uint32_t iStateId, void* pUser)
{
	int* pCount = (int*)pUser;
	(void)pDraw;
	(void)iStateId;
	(*pCount)++;
	xuiWidgetDestroy(pWidget);
	return XUI_OK;
}

static int __xuiPixelEquals(const unsigned char* pPixels, int iStride, int iX, int iY, unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
	const unsigned char* pPixel;

	pPixel = pPixels + (iY * iStride) + (iX * 4);
	return (pPixel[0] == r) && (pPixel[1] == g) && (pPixel[2] == b) && (pPixel[3] == a);
}

static int __xuiTestHideOnLayout(xui_widget pWidget, xui_rect_t tContent, void* pUser)
{
	xui_widget* ppHide = (xui_widget*)pUser;
	(void)pWidget;
	(void)tContent;
	if ( *ppHide != NULL ) {
		xui_widget pHide = *ppHide;
		*ppHide = NULL;
		return xuiWidgetSetVisible(pHide, 0);
	}
	return XUI_OK;
}

static int __xuiTestLayoutDamage(void)
{
	xui_render_schedule_test_state_t tState;
	xui_surface_desc_t tSurfaceDesc;
	xui_layout_t tLayout;
	xui_context pContext = NULL;
	xui_widget pRoot = NULL;
	xui_widget pA = NULL;
	xui_widget pB = NULL;
	xui_widget pHide = NULL;
	xui_surface pTarget = NULL;
	int iFailed = 0;
	memset(&tState, 0, sizeof(tState));
	tState.tProxy = __xuiTestProxy();
	XUI_TEST_CHECK(xuiCreate(&pContext) == XUI_OK &&
		xuiSetProxy(pContext, &tState.tProxy) == XUI_OK &&
		xuiSetViewportSize(pContext, 64, 64) == XUI_OK, "damage context");
	XUI_TEST_CHECK(xuiWidgetCreate(pContext, &pRoot) == XUI_OK &&
		xuiWidgetCreate(pContext, &pA) == XUI_OK &&
		xuiWidgetCreate(pContext, &pB) == XUI_OK, "damage widgets");
	(void)xuiSetRootWidget(pContext, pRoot);
	(void)xuiWidgetSetLayoutType(pRoot, XUI_LAYOUT_ROW);
	(void)xuiWidgetSetLayoutCompleteCallback(pRoot, __xuiTestHideOnLayout, &pHide);
	(void)xuiWidgetSetPreferredSize(pA, (xui_vec2_t){10, 10});
	(void)xuiWidgetSetPreferredSize(pB, (xui_vec2_t){10, 10});
	(void)xuiWidgetAddChild(pRoot, pA);
	(void)xuiWidgetAddChild(pRoot, pB);
	(void)xuiWidgetSetCacheRenderCallback(pRoot, __xuiRenderScheduleDraw, &tState);
	(void)xuiWidgetSetCacheRenderCallback(pA, __xuiRenderScheduleDraw, &tState);
	(void)xuiWidgetSetCacheRenderCallback(pB, __xuiRenderScheduleDraw, &tState);
	(void)xuiWidgetSetStateId(pB, 1);
	memset(&tSurfaceDesc, 0, sizeof(tSurfaceDesc));
	tSurfaceDesc.iWidth = 64;
	tSurfaceDesc.iHeight = 64;
	tSurfaceDesc.iKind = XUI_SURFACE_KIND_TEXTURE;
	tSurfaceDesc.iFormat = XUI_SURFACE_FORMAT_RGBA8;
	tSurfaceDesc.iFlags = XUI_SURFACE_USAGE_TARGET;
	XUI_TEST_CHECK(tState.tProxy.surfaceCreate(&tState.tProxy, &pTarget, &tSurfaceDesc) == XUI_OK,
		"damage target");
	XUI_TEST_CHECK(xuiRender(pContext, pTarget, NULL, 0) == XUI_OK, "damage initial render");
	XUI_TEST_CHECK(__xuiPixelEquals(pTarget->pPixels, 256, 15, 5, 0, 255, 0, 255), "initial sibling position");
	(void)xuiWidgetSetPreferredSize(pA, (xui_vec2_t){25, 10});
	XUI_TEST_CHECK(xuiRender(pContext, pTarget, NULL, 0) == XUI_OK, "grow partial render");
	XUI_TEST_CHECK(__xuiPixelEquals(pTarget->pPixels, 256, 15, 5, 255, 0, 0, 255), "old sibling pixels survived growth");
	XUI_TEST_CHECK(__xuiPixelEquals(pTarget->pPixels, 256, 30, 5, 0, 255, 0, 255), "new sibling pixels missing after growth");
	XUI_TEST_CHECK(xuiGetDamageRects(pContext, NULL, 0) == 0, "layout damage was queued again after composition");
	(void)xuiWidgetSetPreferredSize(pA, (xui_vec2_t){5, 10});
	XUI_TEST_CHECK(xuiRender(pContext, pTarget, NULL, 0) == XUI_OK, "shrink partial render");
	XUI_TEST_CHECK(__xuiPixelEquals(pTarget->pPixels, 256, 30, 5, 255, 0, 0, 255), "old sibling pixels survived shrink");
	XUI_TEST_CHECK(__xuiPixelEquals(pTarget->pPixels, 256, 10, 5, 0, 255, 0, 255), "new sibling pixels missing after shrink");
	(void)xuiWidgetSetLayoutType(pRoot, XUI_LAYOUT_MANUAL);
	(void)xuiWidgetRemoveFromParent(pB);
	(void)xuiWidgetSetRect(pA, (xui_rect_t){5, 20, 5, 5});
	(void)xuiWidgetSetOverflow(pA, XUI_OVERFLOW_VISIBLE);
	(void)xuiWidgetSetRect(pB, (xui_rect_t){15, 0, 6, 6});
	(void)xuiWidgetAddChild(pA, pB);
	XUI_TEST_CHECK(xuiRender(pContext, pTarget, NULL, 0) == XUI_OK, "overflow partial render");
	XUI_TEST_CHECK(__xuiPixelEquals(pTarget->pPixels, 256, 20, 21, 0, 255, 0, 255), "added overflow not drawn");
	(void)xuiWidgetSetRect(pA, (xui_rect_t){15, 20, 5, 5});
	XUI_TEST_CHECK(xuiRender(pContext, pTarget, NULL, 0) == XUI_OK, "parent move partial render");
	XUI_TEST_CHECK(__xuiPixelEquals(pTarget->pPixels, 256, 20, 21, 255, 0, 0, 255), "moved overflow left stale pixels");
	XUI_TEST_CHECK(__xuiPixelEquals(pTarget->pPixels, 256, 30, 21, 0, 255, 0, 255), "moved overflow not drawn");
	(void)xuiWidgetSetOverflow(pA, XUI_OVERFLOW_CLIP);
	XUI_TEST_CHECK(xuiRender(pContext, pTarget, NULL, 0) == XUI_OK, "clip partial render");
	XUI_TEST_CHECK(__xuiPixelEquals(pTarget->pPixels, 256, 30, 21, 255, 0, 0, 255), "clipped overflow left stale pixels");
	tLayout = xuiWidgetGetLayout(pA);
	tLayout.iOverflow = XUI_OVERFLOW_VISIBLE;
	(void)xuiWidgetSetLayout(pA, &tLayout);
	XUI_TEST_CHECK(xuiRender(pContext, pTarget, NULL, 0) == XUI_OK, "unclip partial render");
	XUI_TEST_CHECK(__xuiPixelEquals(pTarget->pPixels, 256, 30, 21, 0, 255, 0, 255), "unclipped overflow not drawn");
	pHide = pA;
	(void)xuiWidgetInvalidate(pA, XUI_WIDGET_DIRTY_LAYOUT);
	XUI_TEST_CHECK(xuiRender(pContext, pTarget, NULL, 0) == XUI_OK, "hide partial render");
	XUI_TEST_CHECK(__xuiPixelEquals(pTarget->pPixels, 256, 30, 21, 255, 0, 0, 255), "hidden overflow left stale pixels");
	XUI_TEST_CHECK(xuiGetDamageRects(pContext, NULL, 0) == 0, "hidden overflow damage missed the current frame");
	(void)xuiWidgetSetVisible(pA, 1);
	XUI_TEST_CHECK(xuiRender(pContext, pTarget, NULL, 0) == XUI_OK, "show partial render");
	XUI_TEST_CHECK(__xuiPixelEquals(pTarget->pPixels, 256, 30, 21, 0, 255, 0, 255), "shown overflow not drawn");
	xuiWidgetDestroy(pA);
	pA = pB = NULL;
	XUI_TEST_CHECK(xuiRender(pContext, pTarget, NULL, 0) == XUI_OK, "destroy partial render");
	XUI_TEST_CHECK(__xuiPixelEquals(pTarget->pPixels, 256, 30, 21, 255, 0, 0, 255), "destroyed overflow left stale pixels");
cleanup:
	if ( pTarget != NULL ) tState.tProxy.surfaceDestroy(&tState.tProxy, pTarget);
	if ( pContext != NULL ) xuiDestroy(pContext);
	return !iFailed;
}

static int __xuiTestSolidDraw(xui_widget pWidget, xui_draw_context pDraw, uint32_t iStateId, void* pUser)
{
	xui_rect_t tRect = xuiWidgetGetRect(pWidget);
	(void)iStateId;
	tRect.fX = tRect.fY = 0;
	return __xuiTestSurfaceFillRect(pDraw->pTarget, tRect, *(const uint32_t*)pUser);
}

static int __xuiTestCacheClipImage(int iCacheMask, int iParentOverflow, int iBranchOverflow, int iGeometry,
	unsigned char* pPixels)
{
	xui_proxy_t tProxy = __xuiTestProxy();
	xui_surface_desc_t tDesc = {0};
	xui_cache_policy_t tPolicy = {0};
	xui_context pContext = NULL;
	xui_widget arrWidgets[5] = {0};
	xui_surface pTarget = NULL;
	const uint32_t arrColors[] = {XUI_COLOR_WHITE, XUI_COLOR_RGBA(255, 0, 0, 128),
		XUI_COLOR_RGBA(0, 0, 255, 128)};
	const xui_rect_t arrRects[] = {{0, 0, 64, 64}, {10, 10, 28, 28},
		{6, 6, 12, 12}, {-10, -10, 40, 40}, {16, 16, 20, 20}};
	int iFailed = 0;
	int i;
	XUI_TEST_CHECK(xuiCreate(&pContext) == XUI_OK &&
		xuiSetProxy(pContext, &tProxy) == XUI_OK &&
		xuiSetViewportSize(pContext, 64, 64) == XUI_OK, "cache clip context");
	tPolicy.iSize = sizeof(tPolicy);
	tPolicy.iFlags = XUI_CACHE_CLEAR_ON_UPDATE;
	for ( i = 0; i < 5; ++i ) {
		XUI_TEST_CHECK(xuiWidgetCreate(pContext, &arrWidgets[i]) == XUI_OK, "cache clip widget");
		(void)xuiWidgetSetRect(arrWidgets[i], arrRects[i]);
		tPolicy.iPolicy = (i < 3 && (iCacheMask & (1 << i))) ?
			XUI_CACHE_POLICY_SUBTREE : XUI_CACHE_POLICY_SELF;
		(void)xuiWidgetSetCachePolicy(arrWidgets[i], &tPolicy);
		(void)xuiWidgetSetOverflow(arrWidgets[i], XUI_OVERFLOW_VISIBLE);
	}
	if ( iGeometry == 1 ) {
		(void)xuiWidgetSetRect(arrWidgets[1], (xui_rect_t){9, 11, 29, 27});
		(void)xuiWidgetSetRect(arrWidgets[2], (xui_rect_t){7, 5, 13, 11});
		(void)xuiWidgetSetRect(arrWidgets[3], (xui_rect_t){-11, -8, 41, 39});
		(void)xuiWidgetSetRect(arrWidgets[4], (xui_rect_t){17, 15, 21, 19});
	} else if ( iGeometry == 2 ) {
		(void)xuiWidgetSetRect(arrWidgets[1], (xui_rect_t){-8, -6, 28, 28});
	}
	(void)xuiSetRootWidget(pContext, arrWidgets[0]);
	(void)xuiWidgetAddChild(arrWidgets[0], arrWidgets[1]);
	(void)xuiWidgetAddChild(arrWidgets[1], arrWidgets[2]);
	(void)xuiWidgetAddChild(arrWidgets[2], arrWidgets[3]);
	(void)xuiWidgetAddChild(arrWidgets[1], arrWidgets[4]);
	(void)xuiWidgetSetOverflow(arrWidgets[1], iParentOverflow);
	(void)xuiWidgetSetOverflow(arrWidgets[2], iBranchOverflow);
	(void)xuiWidgetSetCacheRenderCallback(arrWidgets[0], __xuiTestSolidDraw, (void*)&arrColors[0]);
	(void)xuiWidgetSetCacheRenderCallback(arrWidgets[3], __xuiTestSolidDraw, (void*)&arrColors[1]);
	(void)xuiWidgetSetCacheRenderCallback(arrWidgets[4], __xuiTestSolidDraw, (void*)&arrColors[2]);
	tDesc.iWidth = tDesc.iHeight = 64;
	tDesc.iKind = XUI_SURFACE_KIND_TEXTURE;
	tDesc.iFormat = XUI_SURFACE_FORMAT_RGBA8;
	tDesc.iFlags = XUI_SURFACE_USAGE_TARGET | XUI_SURFACE_ALPHA_PREMULTIPLIED;
	XUI_TEST_CHECK(tProxy.surfaceCreate(&tProxy, &pTarget, &tDesc) == XUI_OK, "cache clip target");
	XUI_TEST_CHECK(xuiRender(pContext, pTarget, NULL, 0) == XUI_OK, "cache clip render");
	memcpy(pPixels, pTarget->pPixels, 64 * 64 * 4);
cleanup:
	if ( pTarget != NULL ) tProxy.surfaceDestroy(&tProxy, pTarget);
	if ( pContext != NULL ) xuiDestroy(pContext);
	return !iFailed;
}

static int __xuiTestCacheClipping(void)
{
	const int arrOverflow[] = {XUI_OVERFLOW_VISIBLE, XUI_OVERFLOW_CLIP, XUI_OVERFLOW_HIDDEN};
	unsigned char arrExpected[64 * 64 * 4];
	unsigned char arrActual[64 * 64 * 4];
	int g, p, b, mask, k;
	for ( g = 0; g < 3; ++g ) {
		for ( p = 0; p < 3; ++p ) {
			for ( b = 0; b < 3; ++b ) {
				if ( !__xuiTestCacheClipImage(0, arrOverflow[p], arrOverflow[b], g, arrExpected) ) return 0;
				for ( mask = 1; mask < 8; ++mask ) {
					if ( !__xuiTestCacheClipImage(mask, arrOverflow[p], arrOverflow[b], g, arrActual) ) return 0;
					for ( k = 0; k < (int)sizeof(arrExpected); ++k ) {
						if ( arrExpected[k] != arrActual[k] ) {
							printf("cache clip mismatch: geometry=%d mask=%d parent=%d branch=%d x=%d y=%d channel=%d expected=%d actual=%d\n",
								g, mask, p, b, (k / 4) % 64, (k / 4) / 64, k % 4, arrExpected[k], arrActual[k]);
							return 0;
						}
					}
				}
			}
		}
	}
	return 1;
}

static int __xuiDpiDraw(xui_widget pWidget, xui_draw_context pDraw, uint32_t iStateId, void* pUser)
{
	xui_render_schedule_test_state_t* pState = (xui_render_schedule_test_state_t*)pUser;
	xui_rect_t tRect = xuiWidgetGetRect(pWidget);
	int iColor = (int)(100.0f * xuiGetVirtualDpi(xuiWidgetGetContext(pWidget)));
	tRect.fX = tRect.fY = 0;
	pState->iRenderCount++;
	return pState->tProxy.drawRectFill(&pState->tProxy, pDraw, tRect,
		XUI_COLOR_RGBA(iColor, iStateId ? 64 : 0, 0, 255));
}

static int __xuiDpiMeasure(xui_widget pWidget, xui_vec2_t tConstraint, xui_vec2_t* pSize, void* pUser)
{
	(void)tConstraint;
	(void)pUser;
	pSize->fX = pSize->fY = 8.0f * xuiGetVirtualDpi(xuiWidgetGetContext(pWidget));
	return XUI_OK;
}

static int __xuiTestDpiCaches(void)
{
	const float arrDpi[] = {1.0f, 1.25f, 1.5f, 2.0f, 1.0f};
	xui_render_schedule_test_state_t tState;
	xui_surface_desc_t tSurfaceDesc = {0};
	xui_context pContext = NULL;
	xui_widget pRoot = NULL, pSizer = NULL, pCached = NULL, pDetached = NULL;
	xui_surface pTarget = NULL;
	xui_vec2_t tMeasured;
	int iFailed = 0;
	int i, s;
	memset(&tState, 0, sizeof(tState));
	tState.tProxy = __xuiTestProxy();
	XUI_TEST_CHECK(xuiCreate(&pContext) == XUI_OK &&
		xuiSetProxy(pContext, &tState.tProxy) == XUI_OK &&
		xuiSetViewportSize(pContext, 64, 64) == XUI_OK, "dpi context");
	XUI_TEST_CHECK(xuiWidgetCreate(pContext, &pRoot) == XUI_OK &&
		xuiWidgetCreate(pContext, &pSizer) == XUI_OK &&
		xuiWidgetCreate(pContext, &pCached) == XUI_OK &&
		xuiWidgetCreate(pContext, &pDetached) == XUI_OK, "dpi widgets");
	(void)xuiSetRootWidget(pContext, pRoot);
	(void)xuiWidgetSetLayoutType(pRoot, XUI_LAYOUT_ROW);
	(void)xuiWidgetSetSizeMode(pSizer, XUI_SIZE_CONTENT, XUI_SIZE_CONTENT);
	(void)xuiWidgetSetSizeMode(pDetached, XUI_SIZE_CONTENT, XUI_SIZE_CONTENT);
	(void)xuiWidgetSetContentMeasureCallback(pSizer, __xuiDpiMeasure, NULL);
	(void)xuiWidgetSetContentMeasureCallback(pDetached, __xuiDpiMeasure, NULL);
	(void)xuiWidgetSetPreferredSize(pCached, (xui_vec2_t){20, 20});
	(void)xuiWidgetAddChild(pRoot, pSizer);
	(void)xuiWidgetAddChild(pRoot, pCached);
	(void)xuiWidgetSetCacheRenderCallback(pCached, __xuiDpiDraw, &tState);
	tSurfaceDesc.iWidth = tSurfaceDesc.iHeight = 64;
	tSurfaceDesc.iKind = XUI_SURFACE_KIND_TEXTURE;
	tSurfaceDesc.iFormat = XUI_SURFACE_FORMAT_RGBA8;
	tSurfaceDesc.iFlags = XUI_SURFACE_USAGE_TARGET;
	XUI_TEST_CHECK(tState.tProxy.surfaceCreate(&tState.tProxy, &pTarget, &tSurfaceDesc) == XUI_OK, "dpi target");
	for ( i = 0; i < (int)(sizeof(arrDpi) / sizeof(arrDpi[0])); ++i ) {
		XUI_TEST_CHECK(xuiSetVirtualDpi(pContext, arrDpi[i]) == XUI_OK, "dpi set");
		for ( s = 0; s < 2; ++s ) {
			(void)xuiWidgetSetStateId(pCached, (uint32_t)s);
			XUI_TEST_CHECK(xuiRender(pContext, pTarget, NULL, 0) == XUI_OK, "dpi render");
			XUI_TEST_CHECK(__xuiPixelEquals(pTarget->pPixels, 256, xuiWidgetGetRect(pCached).fX + 1, 1,
				(unsigned char)(100.0f * arrDpi[i]), s ? 64 : 0, 0, 255), "stale dpi cache state pixels");
		}
		XUI_TEST_CHECK(xuiWidgetGetRect(pSizer).fW == (int)(8.0f * arrDpi[i]), "attached dpi measure was reused");
		XUI_TEST_CHECK(xuiWidgetGetRect(pCached).fW == 20, "fixed pixel box was scaled twice");
		XUI_TEST_CHECK(xuiWidgetMeasure(pDetached, (xui_vec2_t){64, 64}, &tMeasured) == XUI_OK &&
			tMeasured.fX == 8.0f * arrDpi[i], "detached dpi measure was reused");
		XUI_TEST_CHECK(tState.iRenderCount == 2 * (i + 1), "dpi did not refresh each cache state exactly once");
		(void)xuiSetVirtualDpi(pContext, arrDpi[i]);
		XUI_TEST_CHECK(xuiRender(pContext, pTarget, NULL, 0) == XUI_OK &&
			tState.iRenderCount == 2 * (i + 1), "unchanged dpi rebuilt the cache");
	}
cleanup:
	if ( pTarget != NULL ) tState.tProxy.surfaceDestroy(&tState.tProxy, pTarget);
	if ( pDetached != NULL ) xuiWidgetDestroy(pDetached);
	if ( pContext != NULL ) xuiDestroy(pContext);
	return !iFailed;
}

static int __xuiTestCacheStacking(void)
{
	xui_render_schedule_test_state_t tState;
	xui_surface_desc_t tSurfaceDesc = {0};
	xui_cache_policy_t tPolicy;
	xui_layout_t tLayout;
	xui_context pContext = NULL;
	xui_widget pRoot = NULL, pBranch = NULL, pA = NULL, pB = NULL;
	xui_surface pTarget = NULL;
	int iFailed = 0;
	int iNested;
	memset(&tState, 0, sizeof(tState));
	tState.tProxy = __xuiTestProxy();
	XUI_TEST_CHECK(xuiCreate(&pContext) == XUI_OK &&
		xuiSetProxy(pContext, &tState.tProxy) == XUI_OK &&
		xuiSetViewportSize(pContext, 64, 64) == XUI_OK, "stack context");
	XUI_TEST_CHECK(xuiWidgetCreate(pContext, &pRoot) == XUI_OK &&
		xuiWidgetCreate(pContext, &pBranch) == XUI_OK &&
		xuiWidgetCreate(pContext, &pA) == XUI_OK &&
		xuiWidgetCreate(pContext, &pB) == XUI_OK, "stack widgets");
	(void)xuiSetRootWidget(pContext, pRoot);
	(void)xuiWidgetSetRect(pBranch, (xui_rect_t){3, 5, 55, 55});
	(void)xuiWidgetSetRect(pA, (xui_rect_t){2, 2, 30, 30});
	(void)xuiWidgetSetRect(pB, (xui_rect_t){2, 2, 30, 30});
	(void)xuiWidgetAddChild(pRoot, pBranch);
	(void)xuiWidgetAddChild(pBranch, pA);
	(void)xuiWidgetAddChild(pBranch, pB);
	(void)xuiWidgetSetCacheRenderCallback(pRoot, __xuiRenderScheduleDraw, &tState);
	(void)xuiWidgetSetCacheRenderCallback(pA, __xuiRenderScheduleDraw, &tState);
	(void)xuiWidgetSetCacheRenderCallback(pB, __xuiRenderScheduleDraw, &tState);
	(void)xuiWidgetSetStateId(pB, 1);
	tPolicy = xuiWidgetGetCachePolicy(pRoot);
	tPolicy.iPolicy = XUI_CACHE_POLICY_SUBTREE;
	(void)xuiWidgetSetCachePolicy(pRoot, &tPolicy);
	tSurfaceDesc.iWidth = tSurfaceDesc.iHeight = 64;
	tSurfaceDesc.iKind = XUI_SURFACE_KIND_TEXTURE;
	tSurfaceDesc.iFormat = XUI_SURFACE_FORMAT_RGBA8;
	tSurfaceDesc.iFlags = XUI_SURFACE_USAGE_TARGET;
	XUI_TEST_CHECK(tState.tProxy.surfaceCreate(&tState.tProxy, &pTarget, &tSurfaceDesc) == XUI_OK, "stack target");
	for ( iNested = 0; iNested < 2; ++iNested ) {
		(void)xuiWidgetSetLayer(pA, XUI_LAYER_NORMAL, 10);
		(void)xuiWidgetSetLayer(pB, XUI_LAYER_NORMAL, 0);
		XUI_TEST_CHECK(xuiRender(pContext, pTarget, NULL, 0) == XUI_OK, "subtree stack render");
		XUI_TEST_CHECK(xuiHitTest(pContext, 10, 10, 0) == pA, "subtree stack hit");
		XUI_TEST_CHECK(__xuiPixelEquals(pTarget->pPixels, 256, 10, 10, 255, 0, 0, 255), "subtree cache ignored z order");
		tLayout = xuiWidgetGetLayout(pB);
		tLayout.iZIndex = 20;
		(void)xuiWidgetSetLayout(pB, &tLayout);
		XUI_TEST_CHECK(xuiHitTest(pContext, 10, 10, 0) == pB, "layout replacement left stale stack hit");
		XUI_TEST_CHECK(xuiRender(pContext, pTarget, NULL, 0) == XUI_OK, "layout stack render");
		XUI_TEST_CHECK(__xuiPixelEquals(pTarget->pPixels, 256, 10, 10, 0, 255, 0, 255), "layout replacement left stale stack pixels");
		(void)xuiWidgetSetCachePolicy(pBranch, &tPolicy);
	}
cleanup:
	if ( pTarget != NULL ) tState.tProxy.surfaceDestroy(&tState.tProxy, pTarget);
	if ( pContext != NULL ) xuiDestroy(pContext);
	return !iFailed;
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
	xui_widget_type_desc_t tUpdateTypeDesc;
	xui_rect_i_t tDamageRect;
	xui_context pContext;
	xui_widget_type pUpdateType;
	xui_widget pRoot;
	xui_widget pChild;
	xui_widget pOverlay;
	xui_widget pUpdateBad;
	xui_widget pUpdateGood;
	xui_widget pUpdateDestroy;
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
	int iDestroyCacheCount;

	memset(&tState, 0, sizeof(tState));
	memset(&tStats, 0, sizeof(tStats));
	pContext = NULL;
	pUpdateType = NULL;
	pRoot = NULL;
	pChild = NULL;
	pOverlay = NULL;
	pUpdateBad = NULL;
	pUpdateGood = NULL;
	pUpdateDestroy = NULL;
	pResource = NULL;
	pDependency = NULL;
	pPainter = NULL;
	pDraw = NULL;
	pTarget = NULL;
	pPatchSurface = NULL;
	iFailed = 0;
	iDestroyCacheCount = 0;

	tState.tProxy = __xuiTestProxy();
	iRet = xuiCreate(&pContext);
	XUI_TEST_CHECK((iRet == XUI_OK) && (pContext != NULL), "xuiCreate failed");
	iRet = xuiSetProxy(pContext, &tState.tProxy);
	XUI_TEST_CHECK(iRet == XUI_OK, "xuiSetProxy failed");
	iRet = xuiSetErrorCallback(pContext, __xuiTestError, &tState);
	XUI_TEST_CHECK(iRet == XUI_OK, "xuiSetErrorCallback failed");
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
	XUI_TEST_CHECK(iRet == XUI_ERROR_UNSUPPORTED &&
		xuiWidgetGetCachePolicy(pRoot).iPolicy == XUI_CACHE_POLICY_SUBTREE,
		"unsupported tiled policy changed the root policy");
	tPolicy.iPolicy = XUI_CACHE_POLICY_DISPLAY_LIST;
	tPolicy.iTileWidth = 0;
	tPolicy.iTileHeight = 0;
	tPolicy.iMaxBytes = 0;
	iRet = xuiWidgetSetCachePolicy(pChild, &tPolicy);
	XUI_TEST_CHECK(iRet == XUI_ERROR_UNSUPPORTED &&
		xuiWidgetGetCachePolicy(pChild).iPolicy == XUI_CACHE_POLICY_SELF &&
		(xuiWidgetGetCachePolicy(pChild).iFlags & XUI_CACHE_UPDATE_ALL_STATES) != 0,
		"unsupported display-list policy changed the child policy");
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

	tPolicy.iSize = sizeof(tPolicy);
	tPolicy.iPolicy = XUI_CACHE_POLICY_NONE;
	tPolicy.iFlags = 0;
	iRet = xuiWidgetSetCachePolicy(pRoot, &tPolicy);
	XUI_TEST_CHECK(iRet == XUI_OK, "recovery root policy failed");
	tPolicy.iPolicy = XUI_CACHE_POLICY_SELF;
	tPolicy.iFlags = XUI_CACHE_CLEAR_ON_UPDATE;
	tPolicy.iClearColor = XUI_COLOR_RGBA(0, 0, 0, 0);
	iRet = xuiWidgetSetCachePolicy(pChild, &tPolicy);
	XUI_TEST_CHECK(iRet == XUI_OK, "recovery child policy failed");
	tState.pFailWidget = pChild;
	tState.iErrorCount = 0;
	iRet = xuiWidgetInvalidate(pChild, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	XUI_TEST_CHECK(iRet == XUI_OK, "recovery child invalidate failed");
	iRet = xuiWidgetInvalidate(pOverlay, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	XUI_TEST_CHECK(iRet == XUI_OK, "recovery overlay invalidate failed");
	iRet = tState.tProxy.surfaceClear(&tState.tProxy, pTarget, XUI_COLOR_RGBA(0, 0, 0, 0));
	XUI_TEST_CHECK(iRet == XUI_OK, "recovery target clear failed");
	iRet = xuiRender(pContext, pTarget, NULL, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "recoverable widget error escaped xuiRender");
	XUI_TEST_CHECK((tState.iErrorCount == 1) && (tState.iLastError == XUI_ERROR_BACKEND_FAILED) &&
	               (tState.iLastStage == XUI_ERROR_STAGE_CACHE) && tState.bLastRecoverable &&
	               (tState.pLastErrorWidget == pChild), "recoverable error callback mismatch");
	XUI_TEST_CHECK(xuiWidgetGetCacheSurface(pChild, xuiWidgetGetStateId(pChild)) == NULL,
	               "failed widget retained a partial cache");
	memset(arrReadback, 0, sizeof(arrReadback));
	iRet = tState.tProxy.surfaceReadRGBA(&tState.tProxy, pTarget, arrReadback, 64 * 4);
	XUI_TEST_CHECK(iRet == XUI_OK, "recovery readback failed");
	XUI_TEST_CHECK(__xuiPixelEquals(arrReadback, 64 * 4, 2, 2, 255, 0, 0, 255),
	               "healthy overlay did not render after sibling failure");
	iRet = xuiGetRenderStats(pContext, &tStats);
	XUI_TEST_CHECK((iRet == XUI_OK) && (tStats.iRecoveredErrors == 1), "recovery stats mismatch");
	tState.pFailWidget = NULL;

	memset(&tUpdateTypeDesc, 0, sizeof(tUpdateTypeDesc));
	tUpdateTypeDesc.iSize = sizeof(tUpdateTypeDesc);
	tUpdateTypeDesc.sName = "test.recoverable_update";
	tUpdateTypeDesc.pParent = xuiWidgetGetBaseType();
	tUpdateTypeDesc.onUpdate = __xuiTestUpdate;
	iRet = xuiWidgetRegisterType(pContext, &pUpdateType, &tUpdateTypeDesc);
	XUI_TEST_CHECK((iRet == XUI_OK) && (pUpdateType != NULL), "update test type registration failed");
	iRet = xuiWidgetCreateTyped(pContext, pUpdateType, &pUpdateBad, NULL);
	XUI_TEST_CHECK((iRet == XUI_OK) && (pUpdateBad != NULL), "failing update widget create failed");
	iRet = xuiWidgetCreateTyped(pContext, pUpdateType, &pUpdateGood, NULL);
	XUI_TEST_CHECK((iRet == XUI_OK) && (pUpdateGood != NULL), "healthy update widget create failed");
	iRet = xuiWidgetAddChild(pRoot, pUpdateBad);
	XUI_TEST_CHECK(iRet == XUI_OK, "failing update widget attach failed");
	iRet = xuiWidgetAddChild(pRoot, pUpdateGood);
	XUI_TEST_CHECK(iRet == XUI_OK, "healthy update widget attach failed");
	g_pXuiUpdateTestState = &tState;
	tState.pFailUpdateWidget = pUpdateBad;
	tState.iHealthyUpdateCount = 0;
	tState.iErrorCount = 0;
	iRet = xuiUpdate(pContext, 0.016f);
	XUI_TEST_CHECK(iRet == XUI_OK, "recoverable update error escaped xuiUpdate");
	XUI_TEST_CHECK((tState.iErrorCount == 1) && (tState.iLastError == XUI_ERROR) &&
	               (tState.iLastStage == XUI_ERROR_STAGE_UPDATE) && tState.bLastRecoverable &&
	               (tState.pLastErrorWidget == pUpdateBad), "update error callback mismatch");
	XUI_TEST_CHECK(tState.iHealthyUpdateCount == 1, "healthy overlay update was skipped");

	iRet = xuiWidgetCreateTyped(pContext, pUpdateType, &pUpdateDestroy, NULL);
	XUI_TEST_CHECK((iRet == XUI_OK) && (pUpdateDestroy != NULL), "destroying update widget create failed");
	iRet = xuiWidgetAddChild(pRoot, pUpdateDestroy);
	XUI_TEST_CHECK(iRet == XUI_OK, "destroying update widget attach failed");
	tState.pDestroyUpdateWidget = pUpdateDestroy;
	tState.iDestroyUpdateCount = 0;
	iRet = xuiUpdate(pContext, 0.016f);
	XUI_TEST_CHECK(iRet == XUI_OK, "self-destroying update callback escaped xuiUpdate");
	XUI_TEST_CHECK(tState.iDestroyUpdateCount == 1, "self-destroying update callback was not called");
	pUpdateDestroy = NULL;
	tState.pDestroyUpdateWidget = NULL;

	iRet = xuiWidgetSetEventHandler(pRoot, XUI_EVENT_VIEWPORT, __xuiTestPendingEvent, &tState);
	XUI_TEST_CHECK(iRet == XUI_OK, "failing event handler setup failed");
	iRet = xuiWidgetSetEventHandler(pRoot, XUI_EVENT_DPI, __xuiTestPendingEvent, &tState);
	XUI_TEST_CHECK(iRet == XUI_OK, "healthy event handler setup failed");
	tState.pFailEventWidget = pRoot;
	tState.iHealthyEventCount = 0;
	tState.iErrorCount = 0;
	iRet = xuiInputViewport(pContext, 65.0f, 64.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "failing viewport event enqueue failed");
	iRet = xuiInputDpi(pContext, 1.25f);
	XUI_TEST_CHECK(iRet == XUI_OK, "healthy dpi event enqueue failed");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "recoverable handler error escaped pending event pump");
	XUI_TEST_CHECK((tState.iErrorCount == 1) && (tState.iLastError == XUI_ERROR_RESOURCE_FAILED) &&
	               (tState.iLastStage == XUI_ERROR_STAGE_INPUT) && tState.bLastRecoverable &&
	               (tState.pLastErrorWidget == pRoot), "event error callback mismatch");
	XUI_TEST_CHECK(tState.iHealthyEventCount == 1, "healthy queued event was skipped");

	iRet = xuiWidgetSetCacheRenderCallback(pChild, __xuiRenderScheduleDestroySelf,
		&iDestroyCacheCount);
	XUI_TEST_CHECK(iRet == XUI_OK, "self-destroying cache callback setup failed");
	iRet = xuiWidgetInvalidate(pChild, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	XUI_TEST_CHECK(iRet == XUI_OK, "self-destroying cache invalidate failed");
	iRet = xuiRender(pContext, pTarget, NULL, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "self-destroying cache callback escaped xuiRender");
	XUI_TEST_CHECK(iDestroyCacheCount > 0, "self-destroying cache callback was not called");
	pChild = NULL;

cleanup:
	g_pXuiUpdateTestState = NULL;
	if ( pUpdateBad != NULL ) {
		xuiWidgetDestroy(pUpdateBad);
		pUpdateBad = NULL;
	}
	if ( pUpdateGood != NULL ) {
		xuiWidgetDestroy(pUpdateGood);
		pUpdateGood = NULL;
	}
	if ( pUpdateDestroy != NULL ) {
		xuiWidgetDestroy(pUpdateDestroy);
		pUpdateDestroy = NULL;
	}
	if ( pUpdateType != NULL ) {
		(void)xuiWidgetUnregisterType(pUpdateType);
		pUpdateType = NULL;
	}
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
	if ( !__xuiTestLayoutDamage() ) return 1;
	if ( !__xuiTestCacheStacking() ) return 1;
	if ( !__xuiTestDpiCaches() ) return 1;
	if ( !__xuiTestCacheClipping() ) return 1;
	printf("xui_render_schedule_test passed\n");
	return 0;
}
