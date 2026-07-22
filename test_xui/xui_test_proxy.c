#include "xui_test_proxy.h"

#include <string.h>

#define XUI_TEST_SURFACE_MAGIC 0x54535552u
#define XUI_TEST_DRAW_MAGIC 0x54445257u
#define XUI_TEST_FONT_MAGIC 0x54464f4eu

struct xui_surface_t {
	uint32_t iMagic;
	xui_surface_desc_t tDesc;
	uint32_t iGeneration;
	int iDrawCount;
	int iRectFillCount;
	int iTextDrawCount;
	int iClearCount;
	xui_rect_t tLastRect;
	xui_rect_t tLastSrc;
	xui_rect_t tLastDst;
	xui_rect_t tLastTextRect;
	uint32_t iLastColor;
	uint32_t arrRectFillCountColors[8];
	int arrRectFillColorCounts[8];
	uint32_t iLastTextColor;
	uint32_t iLastFlags;
	uint32_t iLastTextFlags;
};

struct xui_draw_context_t {
	uint32_t iMagic;
	xui_surface pTarget;
};

struct xui_font_t {
	uint32_t iMagic;
	float fSize;
};

static int __xuiTestSurfaceValid(xui_surface pSurface)
{
	return (pSurface != NULL) && (pSurface->iMagic == XUI_TEST_SURFACE_MAGIC);
}

static int __xuiTestDrawValid(xui_draw_context pDraw)
{
	return (pDraw != NULL) && (pDraw->iMagic == XUI_TEST_DRAW_MAGIC) && __xuiTestSurfaceValid(pDraw->pTarget);
}

static int __xuiTestFontValid(xui_font pFont)
{
	return (pFont != NULL) && (pFont->iMagic == XUI_TEST_FONT_MAGIC);
}

static int __xuiTestClipboardSetText(xui_proxy pProxy, const char* sText)
{
	xui_test_proxy_state_t* pState;

	if ( pProxy == NULL ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pState = (xui_test_proxy_state_t*)pProxy->pUser;
	if ( pState == NULL ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( sText == NULL ) {
		sText = "";
	}
	strncpy(pState->sClipboard, sText, sizeof(pState->sClipboard) - 1u);
	pState->sClipboard[sizeof(pState->sClipboard) - 1u] = '\0';
	return XUI_OK;
}

static int __xuiTestClipboardGetText(xui_proxy pProxy, char* sText, int iCapacity)
{
	xui_test_proxy_state_t* pState;
	int iLen;

	if ( pProxy == NULL ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pState = (xui_test_proxy_state_t*)pProxy->pUser;
	if ( pState == NULL ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( (sText != NULL) && (iCapacity > 0) ) {
		strncpy(sText, pState->sClipboard, (size_t)iCapacity - 1u);
		sText[iCapacity - 1] = '\0';
	}
	iLen = (int)strlen(pState->sClipboard);
	if ( iCapacity > 0 && iLen >= iCapacity ) {
		iLen = iCapacity - 1;
	}
	return iLen;
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

static int __xuiTestGetCaps(xui_proxy pProxy, xui_proxy_caps_t* pCaps)
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
	               XUI_PROXY_CAP_FONT_XRF | XUI_PROXY_CAP_TEXT |
	               XUI_PROXY_CAP_MESH_TRIANGLES | XUI_PROXY_CAP_PATH_FILL |
	               XUI_PROXY_CAP_PATH_STROKE | XUI_PROXY_CAP_PATH_DASH |
	               XUI_PROXY_CAP_PATH_AA;
	pCaps->iSurfaceFormat = XUI_SURFACE_FORMAT_RGBA8;
	pCaps->iInternalAlpha = XUI_SURFACE_ALPHA_PREMULTIPLIED;
	pCaps->tDefaultSampler.iMinFilter = XUI_SURFACE_FILTER_NEAREST;
	pCaps->tDefaultSampler.iMagFilter = XUI_SURFACE_FILTER_NEAREST;
	pCaps->tDefaultSampler.iWrapS = XUI_SURFACE_WRAP_CLAMP;
	pCaps->tDefaultSampler.iWrapT = XUI_SURFACE_WRAP_CLAMP;
	return XUI_OK;
}

static int __xuiTestSurfaceCreate(xui_proxy pProxy, xui_surface* ppSurface, const xui_surface_desc_t* pDesc)
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

static int __xuiTestSurfaceCreateRGBA(xui_proxy pProxy, xui_surface* ppSurface, int iWidth, int iHeight, const void* pPixels, int iStride, uint32_t iFlags)
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
	return __xuiTestSurfaceCreate(pProxy, ppSurface, &tDesc);
}

static int __xuiTestSurfaceLoadFile(xui_proxy pProxy, xui_surface* ppSurface, const char* sPath, uint32_t iFlags)
{
	(void)sPath;
	return __xuiTestSurfaceCreateRGBA(pProxy, ppSurface, 8, 8, NULL, 0, iFlags);
}

static int __xuiTestSurfaceLoadMemory(xui_proxy pProxy, xui_surface* ppSurface, const void* pData, int iSize, uint32_t iFlags)
{
	(void)pData;
	(void)iSize;
	return __xuiTestSurfaceCreateRGBA(pProxy, ppSurface, 8, 8, NULL, 0, iFlags);
}

static int __xuiTestSurfaceUpdateRGBA(xui_proxy pProxy, xui_surface pSurface, xui_rect_i_t tRect, const void* pPixels, int iStride)
{
	(void)pProxy;
	(void)tRect;
	(void)pPixels;
	(void)iStride;
	if ( !__xuiTestSurfaceValid(pSurface) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pSurface->iGeneration++;
	return XUI_OK;
}

static int __xuiTestSurfaceReadRGBA(xui_proxy pProxy, xui_surface pSurface, void* pPixels, int iStride)
{
	(void)pProxy;
	(void)pPixels;
	(void)iStride;
	return __xuiTestSurfaceValid(pSurface) ? XUI_OK : XUI_ERROR_INVALID_ARGUMENT;
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
	if ( !__xuiTestSurfaceValid(pSurface) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pSurface->iDrawCount++;
	pSurface->tLastSrc = tSrc;
	pSurface->tLastDst = tDst;
	pSurface->iLastColor = iColor;
	pSurface->iLastFlags = iFlags;
	return XUI_OK;
}

static int __xuiTestSurfaceClear(xui_proxy pProxy, xui_surface pTarget, uint32_t iColor)
{
	(void)pProxy;
	if ( !__xuiTestSurfaceValid(pTarget) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pTarget->iClearCount++;
	pTarget->iLastColor = iColor;
	return XUI_OK;
}

static int __xuiTestSurfaceClearRect(xui_proxy pProxy, xui_surface pTarget, xui_rect_t tRect, uint32_t iColor)
{
	(void)tRect;
	return __xuiTestSurfaceClear(pProxy, pTarget, iColor);
}

static int __xuiTestSurfaceDrawTo(xui_proxy pProxy, xui_surface pTarget, xui_surface pSurface, xui_rect_t tSrc, xui_rect_t tDst, uint32_t iColor, uint32_t iFlags)
{
	(void)pProxy;
	if ( !__xuiTestSurfaceValid(pTarget) || !__xuiTestSurfaceValid(pSurface) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pTarget->iDrawCount++;
	pTarget->tLastSrc = tSrc;
	pTarget->tLastDst = tDst;
	pTarget->iLastColor = iColor;
	pTarget->iLastFlags = iFlags;
	return XUI_OK;
}

static int __xuiTestSurfaceDrawQuad(xui_proxy pProxy, xui_surface pSurface, const xui_surface_vertex_t* pVertices, uint32_t iFlags)
{
	(void)pProxy;
	(void)pVertices;
	(void)iFlags;
	if ( !__xuiTestSurfaceValid(pSurface) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pSurface->iDrawCount++;
	return XUI_OK;
}

static int __xuiTestSurfaceDrawQuadTo(xui_proxy pProxy, xui_surface pTarget, xui_surface pSurface, const xui_surface_vertex_t* pVertices, uint32_t iFlags)
{
	(void)pProxy;
	(void)pVertices;
	(void)iFlags;
	if ( !__xuiTestSurfaceValid(pTarget) || !__xuiTestSurfaceValid(pSurface) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pTarget->iDrawCount++;
	return XUI_OK;
}

static int __xuiTestSurfaceGetSampler(xui_proxy pProxy, xui_surface pSurface, xui_surface_sampler_t* pSampler)
{
	(void)pProxy;
	if ( !__xuiTestSurfaceValid(pSurface) || (pSampler == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pSampler->iMinFilter = XUI_SURFACE_FILTER_NEAREST;
	pSampler->iMagFilter = XUI_SURFACE_FILTER_NEAREST;
	pSampler->iWrapS = XUI_SURFACE_WRAP_CLAMP;
	pSampler->iWrapT = XUI_SURFACE_WRAP_CLAMP;
	return XUI_OK;
}

static int __xuiTestSurfaceSetSampler(xui_proxy pProxy, xui_surface pSurface, const xui_surface_sampler_t* pSampler)
{
	(void)pProxy;
	(void)pSampler;
	return __xuiTestSurfaceValid(pSurface) ? XUI_OK : XUI_ERROR_INVALID_ARGUMENT;
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
	if ( __xuiTestSurfaceValid(pSurface) ) {
		pSurface->iMagic = 0;
		xrtFree(pSurface);
	}
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
	(void)tRect;
	if ( !__xuiTestDrawValid(pDraw) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pDraw->pTarget->iClearCount++;
	pDraw->pTarget->iLastColor = iColor;
	return XUI_OK;
}

static int __xuiTestDrawSurface(xui_proxy pProxy, xui_draw_context pDraw, xui_surface pSurface, xui_rect_t tSrc, xui_rect_t tDst, uint32_t iColor, uint32_t iFlags)
{
	(void)pProxy;
	if ( !__xuiTestDrawValid(pDraw) || !__xuiTestSurfaceValid(pSurface) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pDraw->pTarget->iDrawCount++;
	pDraw->pTarget->tLastSrc = tSrc;
	pDraw->pTarget->tLastDst = tDst;
	pDraw->pTarget->iLastColor = iColor;
	pDraw->pTarget->iLastFlags = iFlags;
	return XUI_OK;
}

static int __xuiTestDrawSurfaceQuad(xui_proxy pProxy, xui_draw_context pDraw, xui_surface pSurface, const xui_surface_vertex_t* pVertices, uint32_t iFlags)
{
	(void)pProxy;
	(void)pVertices;
	(void)iFlags;
	if ( !__xuiTestDrawValid(pDraw) || !__xuiTestSurfaceValid(pSurface) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pDraw->pTarget->iDrawCount++;
	return XUI_OK;
}

static int __xuiTestDrawMeshTriangles(xui_proxy pProxy, xui_draw_context pDraw, const xui_mesh_vertex_t* pVertices, int iVertexCount, const uint32_t* pIndices, int iIndexCount, uint32_t iFlags)
{
	xui_test_proxy_state_t* pState;

	(void)iFlags;
	if ( !__xuiTestDrawValid(pDraw) || (pVertices == NULL) || (iVertexCount <= 0) ||
	     (pIndices == NULL) || (iIndexCount <= 0) || ((iIndexCount % 3) != 0) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pState = (xui_test_proxy_state_t*)pProxy->pUser;
	if ( pState != NULL ) {
		pState->iMeshDrawCount++;
		pState->iLastMeshVertexCount = iVertexCount;
		pState->iLastMeshIndexCount = iIndexCount;
	}
	pDraw->pTarget->iDrawCount++;
	return XUI_OK;
}

static int __xuiTestDrawPath(xui_proxy pProxy, xui_draw_context pDraw, const xui_path_command_t* pCommands, int iCommandCount, const xui_path_style_t* pStyle, float fTolerance)
{
	xui_test_proxy_state_t* pState;

	if ( !__xuiTestDrawValid(pDraw) || (pCommands == NULL) || (iCommandCount <= 0) ||
	     (pStyle == NULL) || (pStyle->iSize < sizeof(*pStyle)) || (fTolerance <= 0.0f) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pState = (xui_test_proxy_state_t*)pProxy->pUser;
	if ( pState != NULL ) {
		pState->iPathDrawCount++;
		pState->iLastPathCommandCount = iCommandCount;
	}
	pDraw->pTarget->iDrawCount++;
	return XUI_OK;
}

static int __xuiTestDrawSvgPath(xui_proxy pProxy, xui_draw_context pDraw, const char* sPath, xui_rect_t tViewBox, xui_rect_t tTarget, const xui_path_style_t* pStyle, float fTolerance)
{
	xui_test_proxy_state_t* pState;

	if ( !__xuiTestDrawValid(pDraw) || (sPath == NULL) || (sPath[0] == '\0') ||
	     (tViewBox.fW <= 0.0f) || (tViewBox.fH <= 0.0f) ||
	     (tTarget.fW <= 0.0f) || (tTarget.fH <= 0.0f) ||
	     (pStyle == NULL) || (pStyle->iSize < sizeof(*pStyle)) || (fTolerance <= 0.0f) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pState = (xui_test_proxy_state_t*)pProxy->pUser;
	if ( pState != NULL ) pState->iSvgPathDrawCount++;
	pDraw->pTarget->iDrawCount++;
	return XUI_OK;
}

static int __xuiTestDrawRectFill(xui_proxy pProxy, xui_draw_context pDraw, xui_rect_t tRect, uint32_t iColor)
{
	int iSlot;

	(void)pProxy;
	if ( !__xuiTestDrawValid(pDraw) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pDraw->pTarget->iDrawCount++;
	pDraw->pTarget->iRectFillCount++;
	for ( iSlot = 0; iSlot < (int)(sizeof(pDraw->pTarget->arrRectFillCountColors) / sizeof(pDraw->pTarget->arrRectFillCountColors[0])); iSlot++ ) {
		if ( pDraw->pTarget->arrRectFillCountColors[iSlot] == iColor ) {
			pDraw->pTarget->arrRectFillColorCounts[iSlot]++;
			break;
		}
		if ( pDraw->pTarget->arrRectFillCountColors[iSlot] == 0u ) {
			pDraw->pTarget->arrRectFillCountColors[iSlot] = iColor;
			pDraw->pTarget->arrRectFillColorCounts[iSlot] = 1;
			break;
		}
	}
	pDraw->pTarget->tLastRect = tRect;
	pDraw->pTarget->iLastColor = iColor;
	return XUI_OK;
}

static int __xuiTestDrawPoint(xui_proxy pProxy, xui_draw_context pDraw, float fX, float fY, float fSize, uint32_t iColor)
{
	(void)fX;
	(void)fY;
	(void)fSize;
	return __xuiTestDrawRectFill(pProxy, pDraw, (xui_rect_t){0.0f, 0.0f, 1.0f, 1.0f}, iColor);
}

static int __xuiTestDrawLine(xui_proxy pProxy, xui_draw_context pDraw, float fX0, float fY0, float fX1, float fY1, float fWidth, uint32_t iColor)
{
	(void)fX0;
	(void)fY0;
	(void)fX1;
	(void)fY1;
	(void)fWidth;
	return __xuiTestDrawPoint(pProxy, pDraw, 0.0f, 0.0f, 1.0f, iColor);
}

static int __xuiTestDrawTriangleFill(xui_proxy pProxy, xui_draw_context pDraw, xui_vec2_t tA, xui_vec2_t tB, xui_vec2_t tC, uint32_t iColor)
{
	(void)tA;
	(void)tB;
	(void)tC;
	return __xuiTestDrawPoint(pProxy, pDraw, 0.0f, 0.0f, 1.0f, iColor);
}

static int __xuiTestDrawTriangleStroke(xui_proxy pProxy, xui_draw_context pDraw, xui_vec2_t tA, xui_vec2_t tB, xui_vec2_t tC, float fWidth, uint32_t iColor)
{
	(void)fWidth;
	return __xuiTestDrawTriangleFill(pProxy, pDraw, tA, tB, tC, iColor);
}

static int __xuiTestDrawRectStroke(xui_proxy pProxy, xui_draw_context pDraw, xui_rect_t tRect, float fWidth, uint32_t iColor)
{
	(void)fWidth;
	return __xuiTestDrawRectFill(pProxy, pDraw, tRect, iColor);
}

static int __xuiTestDrawCircleFill(xui_proxy pProxy, xui_draw_context pDraw, float fX, float fY, float fRadius, uint32_t iColor)
{
	(void)fX;
	(void)fY;
	(void)fRadius;
	return __xuiTestDrawPoint(pProxy, pDraw, 0.0f, 0.0f, 1.0f, iColor);
}

static int __xuiTestDrawCircleStroke(xui_proxy pProxy, xui_draw_context pDraw, float fX, float fY, float fRadius, float fWidth, uint32_t iColor)
{
	(void)fWidth;
	return __xuiTestDrawCircleFill(pProxy, pDraw, fX, fY, fRadius, iColor);
}



static int __xuiTestDrawText(xui_proxy pProxy, xui_draw_context pDraw, xui_font pFont, const char* sText, xui_rect_t tRect, uint32_t iColor, uint32_t iFlags)
{
	(void)pProxy;
	(void)sText;
	if ( !__xuiTestDrawValid(pDraw) || !__xuiTestFontValid(pFont) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pDraw->pTarget->iDrawCount++;
	pDraw->pTarget->iTextDrawCount++;
	pDraw->pTarget->tLastTextRect = tRect;
	pDraw->pTarget->iLastTextColor = iColor;
	pDraw->pTarget->iLastTextFlags = iFlags;
	return XUI_OK;
}

static int __xuiTestShapeRectFill(xui_proxy pProxy, xui_surface pTarget, xui_rect_t tRect, uint32_t iColor)
{
	int iSlot;

	(void)pProxy;
	if ( !__xuiTestSurfaceValid(pTarget) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pTarget->iDrawCount++;
	pTarget->iRectFillCount++;
	for ( iSlot = 0; iSlot < (int)(sizeof(pTarget->arrRectFillCountColors) / sizeof(pTarget->arrRectFillCountColors[0])); iSlot++ ) {
		if ( pTarget->arrRectFillCountColors[iSlot] == iColor ) {
			pTarget->arrRectFillColorCounts[iSlot]++;
			break;
		}
		if ( pTarget->arrRectFillCountColors[iSlot] == 0u ) {
			pTarget->arrRectFillCountColors[iSlot] = iColor;
			pTarget->arrRectFillColorCounts[iSlot] = 1;
			break;
		}
	}
	pTarget->tLastRect = tRect;
	pTarget->iLastColor = iColor;
	return XUI_OK;
}

static int __xuiTestShapePoint(xui_proxy pProxy, xui_surface pTarget, float fX, float fY, float fSize, uint32_t iColor)
{
	(void)fX;
	(void)fY;
	(void)fSize;
	return __xuiTestShapeRectFill(pProxy, pTarget, (xui_rect_t){0.0f, 0.0f, 1.0f, 1.0f}, iColor);
}

static int __xuiTestShapeLine(xui_proxy pProxy, xui_surface pTarget, float fX0, float fY0, float fX1, float fY1, float fWidth, uint32_t iColor)
{
	(void)fX0;
	(void)fY0;
	(void)fX1;
	(void)fY1;
	(void)fWidth;
	return __xuiTestShapePoint(pProxy, pTarget, 0.0f, 0.0f, 1.0f, iColor);
}

static int __xuiTestShapeTriangleFill(xui_proxy pProxy, xui_surface pTarget, xui_vec2_t tA, xui_vec2_t tB, xui_vec2_t tC, uint32_t iColor)
{
	(void)tA;
	(void)tB;
	(void)tC;
	return __xuiTestShapePoint(pProxy, pTarget, 0.0f, 0.0f, 1.0f, iColor);
}

static int __xuiTestShapeTriangleStroke(xui_proxy pProxy, xui_surface pTarget, xui_vec2_t tA, xui_vec2_t tB, xui_vec2_t tC, float fWidth, uint32_t iColor)
{
	(void)fWidth;
	return __xuiTestShapeTriangleFill(pProxy, pTarget, tA, tB, tC, iColor);
}

static int __xuiTestShapeRectStroke(xui_proxy pProxy, xui_surface pTarget, xui_rect_t tRect, float fWidth, uint32_t iColor)
{
	(void)fWidth;
	return __xuiTestShapeRectFill(pProxy, pTarget, tRect, iColor);
}

static int __xuiTestShapeCircleFill(xui_proxy pProxy, xui_surface pTarget, float fX, float fY, float fRadius, uint32_t iColor)
{
	(void)fX;
	(void)fY;
	(void)fRadius;
	return __xuiTestShapePoint(pProxy, pTarget, 0.0f, 0.0f, 1.0f, iColor);
}

static int __xuiTestShapeCircleStroke(xui_proxy pProxy, xui_surface pTarget, float fX, float fY, float fRadius, float fWidth, uint32_t iColor)
{
	(void)fWidth;
	return __xuiTestShapeCircleFill(pProxy, pTarget, fX, fY, fRadius, iColor);
}



static int __xuiTestFontLoadFile(xui_proxy pProxy, xui_font* ppFont, const char* sPath, float fSize, uint32_t iFlags)
{
	xui_font pFont;

	(void)pProxy;
	(void)sPath;
	(void)iFlags;
	if ( ppFont == NULL ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	*ppFont = NULL;
	pFont = (xui_font)xrtCalloc(1, sizeof(*pFont));
	if ( pFont == NULL ) {
		return XUI_ERROR_OUT_OF_MEMORY;
	}
	pFont->iMagic = XUI_TEST_FONT_MAGIC;
	pFont->fSize = (fSize > 0.0f) ? fSize : 16.0f;
	*ppFont = pFont;
	return XUI_OK;
}

static int __xuiTestFontLoadMemory(xui_proxy pProxy, xui_font* ppFont, const void* pData, int iSize, float fSize, uint32_t iFlags)
{
	(void)pData;
	(void)iSize;
	return __xuiTestFontLoadFile(pProxy, ppFont, NULL, fSize, iFlags);
}

static int __xuiTestFontGetMetrics(xui_proxy pProxy, xui_font pFont, xui_font_metrics_t* pMetrics)
{
	(void)pProxy;
	if ( !__xuiTestFontValid(pFont) || (pMetrics == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	memset(pMetrics, 0, sizeof(*pMetrics));
	pMetrics->fSize = pFont->fSize;
	pMetrics->fAscent = pFont->fSize * 0.8f;
	pMetrics->fDescent = pFont->fSize * 0.2f;
	pMetrics->fLineHeight = pFont->fSize;
	return XUI_OK;
}

static void __xuiTestFontDestroy(xui_proxy pProxy, xui_font pFont)
{
	(void)pProxy;
	if ( __xuiTestFontValid(pFont) ) {
		pFont->iMagic = 0;
		xrtFree(pFont);
	}
}

static int __xuiTestTextMeasure(xui_proxy pProxy, xui_font pFont, const char* sText, xui_vec2_t* pSize)
{
	int iLen;

	(void)pProxy;
	if ( !__xuiTestFontValid(pFont) || (pSize == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	iLen = (sText != NULL) ? (int)strlen(sText) : 0;
	pSize->fX = (float)iLen * pFont->fSize * 0.5f;
	pSize->fY = pFont->fSize;
	return XUI_OK;
}

static int __xuiTestTextDraw(xui_proxy pProxy, xui_surface pTarget, xui_font pFont, const char* sText, xui_rect_t tRect, uint32_t iColor, uint32_t iFlags)
{
	(void)pProxy;
	(void)sText;
	if ( !__xuiTestSurfaceValid(pTarget) || !__xuiTestFontValid(pFont) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pTarget->iDrawCount++;
	pTarget->iTextDrawCount++;
	pTarget->tLastTextRect = tRect;
	pTarget->iLastTextColor = iColor;
	pTarget->iLastTextFlags = iFlags;
	return XUI_OK;
}

void xuiTestProxyInit(xui_test_proxy_state_t* pState)
{
	if ( pState == NULL ) {
		return;
	}
	memset(pState, 0, sizeof(*pState));
	pState->tProxy.iSize = sizeof(pState->tProxy);
	pState->tProxy.iVersion = XUI_PROXY_VERSION;
	pState->tProxy.pUser = pState;
	pState->tProxy.getCaps = __xuiTestGetCaps;
	pState->tProxy.clipboardSetText = __xuiTestClipboardSetText;
	pState->tProxy.clipboardGetText = __xuiTestClipboardGetText;
	pState->tProxy.imeGetEnabled = __xuiTestImeGetEnabled;
	pState->tProxy.imeSetEnabled = __xuiTestImeSetEnabled;
	pState->tProxy.imeSetCandidateRect = __xuiTestImeSetCandidateRect;
	pState->tProxy.surfaceCreate = __xuiTestSurfaceCreate;
	pState->tProxy.surfaceCreateRGBA = __xuiTestSurfaceCreateRGBA;
	pState->tProxy.surfaceLoadFile = __xuiTestSurfaceLoadFile;
	pState->tProxy.surfaceLoadMemory = __xuiTestSurfaceLoadMemory;
	pState->tProxy.surfaceUpdateRGBA = __xuiTestSurfaceUpdateRGBA;
	pState->tProxy.surfaceReadRGBA = __xuiTestSurfaceReadRGBA;
	pState->tProxy.surfaceGetDesc = __xuiTestSurfaceGetDesc;
	pState->tProxy.surfaceDraw = __xuiTestSurfaceDraw;
	pState->tProxy.surfaceClear = __xuiTestSurfaceClear;
	pState->tProxy.surfaceClearRect = __xuiTestSurfaceClearRect;
	pState->tProxy.surfaceDrawTo = __xuiTestSurfaceDrawTo;
	pState->tProxy.surfaceDrawQuad = __xuiTestSurfaceDrawQuad;
	pState->tProxy.surfaceDrawQuadTo = __xuiTestSurfaceDrawQuadTo;
	pState->tProxy.surfaceGetSampler = __xuiTestSurfaceGetSampler;
	pState->tProxy.surfaceSetSampler = __xuiTestSurfaceSetSampler;
	pState->tProxy.surfaceGetGeneration = __xuiTestSurfaceGetGeneration;
	pState->tProxy.surfaceDestroy = __xuiTestSurfaceDestroy;
	pState->tProxy.shapePoint = __xuiTestShapePoint;
	pState->tProxy.shapeLine = __xuiTestShapeLine;
	pState->tProxy.shapeTriangleFill = __xuiTestShapeTriangleFill;
	pState->tProxy.shapeTriangleStroke = __xuiTestShapeTriangleStroke;
	pState->tProxy.shapeRectFill = __xuiTestShapeRectFill;
	pState->tProxy.shapeRectStroke = __xuiTestShapeRectStroke;
	pState->tProxy.shapeCircleFill = __xuiTestShapeCircleFill;
	pState->tProxy.shapeCircleStroke = __xuiTestShapeCircleStroke;
	pState->tProxy.fontLoadFile = __xuiTestFontLoadFile;
	pState->tProxy.fontLoadMemory = __xuiTestFontLoadMemory;
	pState->tProxy.fontGetMetrics = __xuiTestFontGetMetrics;
	pState->tProxy.fontDestroy = __xuiTestFontDestroy;
	pState->tProxy.textMeasure = __xuiTestTextMeasure;
	pState->tProxy.textDraw = __xuiTestTextDraw;
	pState->tProxy.drawBegin = __xuiTestDrawBegin;
	pState->tProxy.drawEnd = __xuiTestDrawEnd;
	pState->tProxy.drawClearRect = __xuiTestDrawClearRect;
	pState->tProxy.drawSurface = __xuiTestDrawSurface;
	pState->tProxy.drawSurfaceQuad = __xuiTestDrawSurfaceQuad;
	pState->tProxy.drawMeshTriangles = __xuiTestDrawMeshTriangles;
	pState->tProxy.drawPath = __xuiTestDrawPath;
	pState->tProxy.drawSvgPath = __xuiTestDrawSvgPath;
	pState->tProxy.drawPoint = __xuiTestDrawPoint;
	pState->tProxy.drawLine = __xuiTestDrawLine;
	pState->tProxy.drawTriangleFill = __xuiTestDrawTriangleFill;
	pState->tProxy.drawTriangleStroke = __xuiTestDrawTriangleStroke;
	pState->tProxy.drawRectFill = __xuiTestDrawRectFill;
	pState->tProxy.drawRectStroke = __xuiTestDrawRectStroke;
	pState->tProxy.drawCircleFill = __xuiTestDrawCircleFill;
	pState->tProxy.drawCircleStroke = __xuiTestDrawCircleStroke;
	pState->tProxy.drawText = __xuiTestDrawText;
}

int xuiTestProxySetClipboardText(xui_test_proxy_state_t* pState, const char* sText)
{
	if ( pState == NULL ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	return __xuiTestClipboardSetText(&pState->tProxy, sText);
}

const char* xuiTestProxyGetClipboardText(xui_test_proxy_state_t* pState)
{
	return (pState != NULL) ? pState->sClipboard : "";
}

int xuiTestSurfaceCreate(xui_test_proxy_state_t* pState, xui_surface* ppSurface, int iWidth, int iHeight, uint32_t iFlags)
{
	if ( pState == NULL ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	return __xuiTestSurfaceCreateRGBA(&pState->tProxy, ppSurface, iWidth, iHeight, NULL, 0, iFlags);
}

void xuiTestSurfaceReset(xui_surface pSurface)
{
	if ( __xuiTestSurfaceValid(pSurface) ) {
		pSurface->iDrawCount = 0;
		pSurface->iRectFillCount = 0;
		memset(pSurface->arrRectFillCountColors, 0, sizeof(pSurface->arrRectFillCountColors));
		memset(pSurface->arrRectFillColorCounts, 0, sizeof(pSurface->arrRectFillColorCounts));
		pSurface->iTextDrawCount = 0;
		pSurface->iClearCount = 0;
		memset(&pSurface->tLastRect, 0, sizeof(pSurface->tLastRect));
		memset(&pSurface->tLastSrc, 0, sizeof(pSurface->tLastSrc));
		memset(&pSurface->tLastDst, 0, sizeof(pSurface->tLastDst));
		memset(&pSurface->tLastTextRect, 0, sizeof(pSurface->tLastTextRect));
		pSurface->iLastColor = 0;
		pSurface->iLastTextColor = 0;
		pSurface->iLastFlags = 0;
		pSurface->iLastTextFlags = 0;
	}
}

int xuiTestSurfaceGetDrawCount(xui_surface pSurface)
{
	return __xuiTestSurfaceValid(pSurface) ? pSurface->iDrawCount : 0;
}

int xuiTestSurfaceGetRectFillCount(xui_surface pSurface)
{
	return __xuiTestSurfaceValid(pSurface) ? pSurface->iRectFillCount : 0;
}

int xuiTestSurfaceGetRectFillColorCount(xui_surface pSurface, uint32_t iColor)
{
	int iSlot;

	if ( !__xuiTestSurfaceValid(pSurface) ) return 0;
	for ( iSlot = 0; iSlot < (int)(sizeof(pSurface->arrRectFillCountColors) / sizeof(pSurface->arrRectFillCountColors[0])); iSlot++ ) {
		if ( pSurface->arrRectFillCountColors[iSlot] == iColor ) return pSurface->arrRectFillColorCounts[iSlot];
	}
	for ( iSlot = 0; iSlot < (int)(sizeof(pSurface->arrRectFillCountColors) / sizeof(pSurface->arrRectFillCountColors[0])); iSlot++ ) {
		if ( pSurface->arrRectFillCountColors[iSlot] == 0u ) {
			pSurface->arrRectFillCountColors[iSlot] = iColor;
			pSurface->arrRectFillColorCounts[iSlot] = 0;
			return 0;
		}
	}
	return 0;
}

int xuiTestSurfaceGetTextDrawCount(xui_surface pSurface)
{
	return __xuiTestSurfaceValid(pSurface) ? pSurface->iTextDrawCount : 0;
}

xui_rect_t xuiTestSurfaceGetLastRect(xui_surface pSurface)
{
	xui_rect_t tRect;

	memset(&tRect, 0, sizeof(tRect));
	return __xuiTestSurfaceValid(pSurface) ? pSurface->tLastRect : tRect;
}

xui_rect_t xuiTestSurfaceGetLastSrc(xui_surface pSurface)
{
	xui_rect_t tRect;

	memset(&tRect, 0, sizeof(tRect));
	return __xuiTestSurfaceValid(pSurface) ? pSurface->tLastSrc : tRect;
}

xui_rect_t xuiTestSurfaceGetLastDst(xui_surface pSurface)
{
	xui_rect_t tRect;

	memset(&tRect, 0, sizeof(tRect));
	return __xuiTestSurfaceValid(pSurface) ? pSurface->tLastDst : tRect;
}

xui_rect_t xuiTestSurfaceGetLastTextRect(xui_surface pSurface)
{
	xui_rect_t tRect;

	memset(&tRect, 0, sizeof(tRect));
	return __xuiTestSurfaceValid(pSurface) ? pSurface->tLastTextRect : tRect;
}

uint32_t xuiTestSurfaceGetLastColor(xui_surface pSurface)
{
	return __xuiTestSurfaceValid(pSurface) ? pSurface->iLastColor : 0;
}

uint32_t xuiTestSurfaceGetLastTextColor(xui_surface pSurface)
{
	return __xuiTestSurfaceValid(pSurface) ? pSurface->iLastTextColor : 0;
}

int xuiTestProxyGetMeshDrawCount(xui_test_proxy_state_t* pState)
{
	return (pState != NULL) ? pState->iMeshDrawCount : 0;
}

int xuiTestProxyGetLastMeshVertexCount(xui_test_proxy_state_t* pState)
{
	return (pState != NULL) ? pState->iLastMeshVertexCount : 0;
}

int xuiTestProxyGetLastMeshIndexCount(xui_test_proxy_state_t* pState)
{
	return (pState != NULL) ? pState->iLastMeshIndexCount : 0;
}

int xuiTestProxyGetPathDrawCount(xui_test_proxy_state_t* pState)
{
	return (pState != NULL) ? pState->iPathDrawCount : 0;
}

int xuiTestProxyGetSvgPathDrawCount(xui_test_proxy_state_t* pState)
{
	return (pState != NULL) ? pState->iSvgPathDrawCount : 0;
}

int xuiTestProxyGetLastPathCommandCount(xui_test_proxy_state_t* pState)
{
	return (pState != NULL) ? pState->iLastPathCommandCount : 0;
}
