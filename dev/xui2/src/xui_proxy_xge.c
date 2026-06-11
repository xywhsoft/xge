#include "xui_internal.h"
#include "../xge.h"

#if defined(_WIN32) || defined(_WIN64)
	#ifndef WIN32_LEAN_AND_MEAN
		#define WIN32_LEAN_AND_MEAN
	#endif
	#ifndef NOMINMAX
		#define NOMINMAX
	#endif
	#include <windows.h>
	#include <imm.h>
#endif

#include <math.h>
#include <string.h>

#define XUI_PROXY_XGE_SURFACE_MAGIC	0x58554953u
#define XUI_PROXY_XGE_FONT_MAGIC	0x58554946u
#define XUI_PROXY_XGE_DRAW_MAGIC	0x58554944u
#define XUI_PROXY_XGE_PI		3.14159265358979323846f
#define XUI_PROXY_XGE_PI_HALF		1.57079632679489661923f
#define XUI_PROXY_XGE_PI_TWO		6.28318530717958647692f
#define XUI_PROXY_XGE_ROUND_RECT_OVERLAP	1.0f
#define XUI_PROXY_XGE_ROUND_RECT_SEGMENTS	8

struct xui_surface_t {
	uint32_t iMagic;
	int iKind;
	int iFormat;
	uint32_t iFlags;
	uint32_t iGeneration;
	xge_texture_t tTexture;
	xge_render_target_t tTarget;
};

struct xui_font_t {
	uint32_t iMagic;
	uint32_t iFlags;
	xge_font_t tFont;
	xge_font_t tFallbackFont;
	int bHasFallback;
};

struct xui_draw_context_t {
	uint32_t iMagic;
	xui_surface pTarget;
	xge_pass_t tPass;
	int bDirty;
};

static uint32_t __xuiProxyXgeImageFlags(uint32_t iFlags)
{
	(void)iFlags;
	return XGE_IMAGE_PREMULTIPLIED;
}

static uint32_t __xuiProxyXgeDrawFlags(uint32_t iFlags)
{
	uint32_t iDrawFlags;

	iDrawFlags = 0;
	if ( (iFlags & XUI_SURFACE_DRAW_FLIP_X) != 0 ) {
		iDrawFlags |= XGE_DRAW_FLIP_X;
	}
	if ( (iFlags & XUI_SURFACE_DRAW_FLIP_Y) != 0 ) {
		iDrawFlags |= XGE_DRAW_FLIP_Y;
	}
	if ( (iFlags & XUI_SURFACE_DRAW_SCREEN_SPACE) != 0 ) {
		iDrawFlags |= XGE_DRAW_SCREEN_SPACE;
	}
	return iDrawFlags;
}

static uint32_t __xuiProxyXgeSurfaceFlags(uint32_t iFlags)
{
	iFlags &= ~XUI_SURFACE_ALPHA_STRAIGHT;
	iFlags |= XUI_SURFACE_ALPHA_PREMULTIPLIED;
	return iFlags;
}

static xge_sampler_t __xuiProxyXgeSampler(const xui_surface_sampler_t* pSampler)
{
	xge_sampler_t tSampler;

	tSampler = xgeSamplerDefault();
	if ( pSampler == NULL ) {
		return tSampler;
	}
	if ( pSampler->iMinFilter == XUI_SURFACE_FILTER_NEAREST ) {
		tSampler.iMinFilter = XGE_FILTER_NEAREST;
	} else if ( pSampler->iMinFilter == XUI_SURFACE_FILTER_LINEAR ) {
		tSampler.iMinFilter = XGE_FILTER_LINEAR;
	}
	if ( pSampler->iMagFilter == XUI_SURFACE_FILTER_NEAREST ) {
		tSampler.iMagFilter = XGE_FILTER_NEAREST;
	} else if ( pSampler->iMagFilter == XUI_SURFACE_FILTER_LINEAR ) {
		tSampler.iMagFilter = XGE_FILTER_LINEAR;
	}
	if ( pSampler->iWrapS == XUI_SURFACE_WRAP_REPEAT ) {
		tSampler.iWrapS = XGE_WRAP_REPEAT;
	} else if ( pSampler->iWrapS == XUI_SURFACE_WRAP_CLAMP ) {
		tSampler.iWrapS = XGE_WRAP_CLAMP;
	}
	if ( pSampler->iWrapT == XUI_SURFACE_WRAP_REPEAT ) {
		tSampler.iWrapT = XGE_WRAP_REPEAT;
	} else if ( pSampler->iWrapT == XUI_SURFACE_WRAP_CLAMP ) {
		tSampler.iWrapT = XGE_WRAP_CLAMP;
	}
	return tSampler;
}

static xui_surface_sampler_t __xuiProxyXgeSurfaceSampler(xge_sampler_t tSampler)
{
	xui_surface_sampler_t tRet;

	tRet.iMinFilter = (tSampler.iMinFilter == XGE_FILTER_NEAREST) ? XUI_SURFACE_FILTER_NEAREST : XUI_SURFACE_FILTER_LINEAR;
	tRet.iMagFilter = (tSampler.iMagFilter == XGE_FILTER_NEAREST) ? XUI_SURFACE_FILTER_NEAREST : XUI_SURFACE_FILTER_LINEAR;
	tRet.iWrapS = (tSampler.iWrapS == XGE_WRAP_REPEAT) ? XUI_SURFACE_WRAP_REPEAT : XUI_SURFACE_WRAP_CLAMP;
	tRet.iWrapT = (tSampler.iWrapT == XGE_WRAP_REPEAT) ? XUI_SURFACE_WRAP_REPEAT : XUI_SURFACE_WRAP_CLAMP;
	return tRet;
}

static int __xuiProxyXgeAsciiEqualNoCase(const char* sA, const char* sB)
{
	char cA;
	char cB;

	if ( (sA == NULL) || (sB == NULL) ) {
		return 0;
	}
	while ( (*sA != 0) && (*sB != 0) ) {
		cA = *sA++;
		cB = *sB++;
		if ( (cA >= 'A') && (cA <= 'Z') ) {
			cA = (char)(cA + ('a' - 'A'));
		}
		if ( (cB >= 'A') && (cB <= 'Z') ) {
			cB = (char)(cB + ('a' - 'A'));
		}
		if ( cA != cB ) {
			return 0;
		}
	}
	return (*sA == 0) && (*sB == 0);
}

static int __xuiProxyXgeFontUseXrf(const char* sPath, uint32_t iFlags)
{
	const char* sExt;

	if ( (iFlags & XUI_FONT_FORMAT_XRF) != 0 ) {
		return 1;
	}
	if ( (iFlags & XUI_FONT_FORMAT_TTF) != 0 ) {
		return 0;
	}
	if ( sPath == NULL ) {
		return 0;
	}
	sExt = strrchr(sPath, '.');
	return __xuiProxyXgeAsciiEqualNoCase(sExt, ".xrf");
}

static uint32_t __xuiProxyXgeTextFlags(uint32_t iFlags)
{
	uint32_t iTextFlags;

	iTextFlags = XGE_TEXT_SCREEN_SPACE;
	if ( (iFlags & XUI_TEXT_ALIGN_RIGHT) == XUI_TEXT_ALIGN_RIGHT ) {
		iTextFlags |= XGE_TEXT_ALIGN_RIGHT;
	} else if ( (iFlags & XUI_TEXT_ALIGN_CENTER) == XUI_TEXT_ALIGN_CENTER ) {
		iTextFlags |= XGE_TEXT_ALIGN_CENTER;
	}
	if ( (iFlags & XUI_TEXT_ALIGN_BOTTOM) == XUI_TEXT_ALIGN_BOTTOM ) {
		iTextFlags |= XGE_TEXT_ALIGN_BOTTOM;
	} else if ( (iFlags & XUI_TEXT_ALIGN_MIDDLE) == XUI_TEXT_ALIGN_MIDDLE ) {
		iTextFlags |= XGE_TEXT_ALIGN_MIDDLE;
	}
	if ( (iFlags & XUI_TEXT_CLIP) != 0 ) {
		iTextFlags |= XGE_TEXT_CLIP;
	}
	if ( (iFlags & XUI_TEXT_UNDERLINE) != 0 ) {
		iTextFlags |= XGE_TEXT_UNDERLINE;
	}
	return iTextFlags;
}

static int __xuiProxyXgeFontValid(xui_font pFont)
{
	return (pFont != NULL) &&
	       (pFont->iMagic == XUI_PROXY_XGE_FONT_MAGIC) &&
	       (pFont->tFont.iRefCount > 0);
}

static int __xuiProxyXgeSurfaceValid(xui_surface pSurface)
{
	xge_texture pTexture;

	if ( (pSurface == NULL) ||
	     (pSurface->iMagic != XUI_PROXY_XGE_SURFACE_MAGIC) ||
	     (pSurface->iKind != XUI_SURFACE_KIND_TEXTURE) ||
	     (pSurface->iFormat != XUI_SURFACE_FORMAT_RGBA8) ) {
		return 0;
	}
	if ( (pSurface->iFlags & XUI_SURFACE_USAGE_TARGET) != 0 ) {
		pTexture = xgeRenderTargetTexture(&pSurface->tTarget);
		return (pTexture != NULL) && (pTexture->iRefCount > 0);
	}
	return (pSurface != NULL) &&
	       (pSurface->iMagic == XUI_PROXY_XGE_SURFACE_MAGIC) &&
	       (pSurface->iKind == XUI_SURFACE_KIND_TEXTURE) &&
	       (pSurface->iFormat == XUI_SURFACE_FORMAT_RGBA8) &&
	       (pSurface->tTexture.iRefCount > 0);
}

static int __xuiProxyXgeSurfaceTargetValid(xui_surface pSurface)
{
	return __xuiProxyXgeSurfaceValid(pSurface) &&
	       ((pSurface->iFlags & XUI_SURFACE_USAGE_TARGET) != 0) &&
	       ((pSurface->tTarget.iFlags & XGE_RENDER_TARGET_TEXTURE) != 0);
}

static xge_texture __xuiProxyXgeSurfaceTexture(xui_surface pSurface)
{
	if ( !__xuiProxyXgeSurfaceValid(pSurface) ) {
		return NULL;
	}
	if ( (pSurface->iFlags & XUI_SURFACE_USAGE_TARGET) != 0 ) {
		return xgeRenderTargetTexture(&pSurface->tTarget);
	}
	return &pSurface->tTexture;
}

static int __xuiProxyXgeSurfaceSize(xui_surface pSurface, int* pWidth, int* pHeight)
{
	xge_texture pTexture;

	pTexture = __xuiProxyXgeSurfaceTexture(pSurface);
	if ( pTexture == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( pWidth != NULL ) {
		*pWidth = pTexture->iWidth;
	}
	if ( pHeight != NULL ) {
		*pHeight = pTexture->iHeight;
	}
	return XGE_OK;
}

static void __xuiProxyXgePremultiplyRGBA(unsigned char* pDst, const unsigned char* pSrc, int iCount)
{
	unsigned int iA;
	int i;

	for ( i = 0; i < iCount; i++ ) {
		iA = (unsigned int)pSrc[(i * 4) + 3];
		pDst[(i * 4) + 0] = (unsigned char)(((unsigned int)pSrc[(i * 4) + 0] * iA + 127u) / 255u);
		pDst[(i * 4) + 1] = (unsigned char)(((unsigned int)pSrc[(i * 4) + 1] * iA + 127u) / 255u);
		pDst[(i * 4) + 2] = (unsigned char)(((unsigned int)pSrc[(i * 4) + 2] * iA + 127u) / 255u);
		pDst[(i * 4) + 3] = pSrc[(i * 4) + 3];
	}
}

static int __xuiProxyXgeSurfacePackRGBA(const void* pPixels, int iWidth, int iHeight, int iStride, uint32_t iFlags, void** ppPacked, const void** ppOutPixels)
{
	unsigned char* pPacked;
	const unsigned char* pSrc;
	int iRowBytes;
	int bPremultiply;
	int i;

	if ( (ppPacked == NULL) || (ppOutPixels == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	*ppPacked = NULL;
	*ppOutPixels = NULL;
	if ( pPixels == NULL ) {
		return XGE_OK;
	}
	if ( (iWidth <= 0) || (iHeight <= 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	iRowBytes = iWidth * 4;
	if ( iStride <= 0 ) {
		iStride = iRowBytes;
	}
	if ( iStride < iRowBytes ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	bPremultiply = ((iFlags & XUI_SURFACE_ALPHA_STRAIGHT) != 0);
	if ( (iStride == iRowBytes) && !bPremultiply ) {
		*ppOutPixels = pPixels;
		return XGE_OK;
	}
	pPacked = (unsigned char*)xrtMalloc((size_t)iRowBytes * (size_t)iHeight);
	if ( pPacked == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pSrc = (const unsigned char*)pPixels;
	for ( i = 0; i < iHeight; i++ ) {
		memcpy(pPacked + ((size_t)i * (size_t)iRowBytes), pSrc + ((size_t)i * (size_t)iStride), (size_t)iRowBytes);
	}
	if ( bPremultiply ) {
		__xuiProxyXgePremultiplyRGBA(pPacked, pPacked, iWidth * iHeight);
	}
	*ppPacked = pPacked;
	*ppOutPixels = pPacked;
	return XGE_OK;
}

static xui_surface __xuiProxyXgeSurfaceAlloc(uint32_t iFlags)
{
	xui_surface pSurface;

	pSurface = (xui_surface)xrtCalloc(1, sizeof(*pSurface));
	if ( pSurface == NULL ) {
		return NULL;
	}
	pSurface->iMagic = XUI_PROXY_XGE_SURFACE_MAGIC;
	pSurface->iKind = XUI_SURFACE_KIND_TEXTURE;
	pSurface->iFormat = XUI_SURFACE_FORMAT_RGBA8;
	pSurface->iFlags = __xuiProxyXgeSurfaceFlags(iFlags);
	pSurface->iGeneration = 1u;
	return pSurface;
}

static void __xuiProxyXgeSurfaceBump(xui_surface pSurface)
{
	if ( !__xuiProxyXgeSurfaceValid(pSurface) ) {
		return;
	}
	pSurface->iGeneration++;
	if ( pSurface->iGeneration == 0 ) {
		pSurface->iGeneration = 1u;
	}
}

static void __xuiProxyXgeSurfaceVertexCopy(xge_vertex_t* pDst, const xui_surface_vertex_t* pSrc)
{
	int i;

	for ( i = 0; i < 4; i++ ) {
		pDst[i].fX = pSrc[i].fX;
		pDst[i].fY = pSrc[i].fY;
		pDst[i].fZ = pSrc[i].fZ;
		pDst[i].fW = pSrc[i].fW;
		pDst[i].fU = pSrc[i].fU;
		pDst[i].fV = pSrc[i].fV;
		pDst[i].iColor = pSrc[i].iColor;
	}
}

static int __xuiProxyXgeSurfaceIsTarget(xui_surface pSurface)
{
	return __xuiProxyXgeSurfaceValid(pSurface) &&
	       ((pSurface->iFlags & XUI_SURFACE_USAGE_TARGET) != 0);
}

static uint32_t __xuiProxyXgeSurfaceDrawFlags(xui_surface pSurface, uint32_t iFlags)
{
	uint32_t iDrawFlags;

	iDrawFlags = __xuiProxyXgeDrawFlags(iFlags);
	if ( __xuiProxyXgeSurfaceIsTarget(pSurface) ) {
		iDrawFlags ^= XGE_DRAW_FLIP_Y;
	}
	return iDrawFlags;
}

static xge_rect_t __xuiProxyXgeSurfaceSourceRect(xui_surface pSurface, xui_rect_t tSrc)
{
	xge_rect_t tRet;
	int iWidth;
	int iHeight;

	tRet.fX = tSrc.fX;
	tRet.fY = tSrc.fY;
	tRet.fW = tSrc.fW;
	tRet.fH = tSrc.fH;
	if ( __xuiProxyXgeSurfaceSize(pSurface, &iWidth, &iHeight) != XGE_OK ) {
		return tRet;
	}
	if ( tRet.fW == 0.0f ) {
		tRet.fW = (float)iWidth;
	}
	if ( tRet.fH == 0.0f ) {
		tRet.fH = (float)iHeight;
	}
	if ( __xuiProxyXgeSurfaceIsTarget(pSurface) ) {
		tRet.fY = (float)iHeight - (tRet.fY + tRet.fH);
	}
	return tRet;
}

static void __xuiProxyXgeSurfaceVertexMapUV(xui_surface pSurface, xge_vertex_t* pVertices, uint32_t iFlags)
{
	int bFlipX;
	int bFlipY;
	int i;

	if ( pVertices == NULL ) {
		return;
	}
	bFlipX = ((iFlags & XUI_SURFACE_DRAW_FLIP_X) != 0);
	bFlipY = ((iFlags & XUI_SURFACE_DRAW_FLIP_Y) != 0);
	if ( __xuiProxyXgeSurfaceIsTarget(pSurface) ) {
		bFlipY = !bFlipY;
	}
	if ( !bFlipX && !bFlipY ) {
		return;
	}
	for ( i = 0; i < 4; i++ ) {
		if ( bFlipX ) {
			pVertices[i].fU = 1.0f - pVertices[i].fU;
		}
		if ( bFlipY ) {
			pVertices[i].fV = 1.0f - pVertices[i].fV;
		}
	}
}

static xge_vec2_t __xuiProxyXgeVec2(xui_vec2_t tPoint)
{
	xge_vec2_t tRet;

	tRet.fX = tPoint.fX;
	tRet.fY = tPoint.fY;
	return tRet;
}

static xge_rect_t __xuiProxyXgeRect(xui_rect_t tRect)
{
	xge_rect_t tRet;

	tRet.fX = tRect.fX;
	tRet.fY = tRect.fY;
	tRet.fW = tRect.fW;
	tRet.fH = tRect.fH;
	return tRet;
}

static float __xuiProxyXgeClampRadius(xui_rect_t tRect, float fRadius)
{
	float fMax;

	if ( fRadius < 0.0f ) {
		fRadius = 0.0f;
	}
	fMax = tRect.fW < tRect.fH ? tRect.fW : tRect.fH;
	fMax *= 0.5f;
	if ( fRadius > fMax ) {
		fRadius = fMax;
	}
	return fRadius;
}

static int __xuiProxyXgeTargetBegin(xui_proxy pProxy, xui_surface pTarget, xge_pass_t* pPass)
{
	if ( (pProxy == NULL) || (pPass == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	(void)pProxy;
	if ( !__xuiProxyXgeSurfaceTargetValid(pTarget) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	(void)xgeFlush();
	xgePassInit(pPass, &pTarget->tTarget, 0, 0);
	return xgePassBegin(pPass);
}

static int __xuiProxyXgeTargetEnd(xge_pass_t* pPass, int iRet)
{
	int iEndRet;

	if ( pPass == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( iRet == XGE_OK ) {
		iRet = xgeFlush();
	}
	iEndRet = xgePassEnd(pPass);
	if ( (iEndRet != XGE_OK) && (iRet == XGE_OK) ) {
		iRet = iEndRet;
	}
	return iRet;
}

static int __xuiProxyXgeTargetEndDirty(xge_pass_t* pPass, xui_surface pTarget, int iRet)
{
	iRet = __xuiProxyXgeTargetEnd(pPass, iRet);
	if ( iRet == XGE_OK ) {
		__xuiProxyXgeSurfaceBump(pTarget);
	}
	return iRet;
}

static int __xuiProxyXgeDrawValid(xui_draw_context pDraw)
{
	return (pDraw != NULL) &&
	       (pDraw->iMagic == XUI_PROXY_XGE_DRAW_MAGIC) &&
	       (pDraw->tPass.bActive != 0) &&
	       __xuiProxyXgeSurfaceTargetValid(pDraw->pTarget);
}

static void __xuiProxyXgeDrawMarkDirty(xui_draw_context pDraw)
{
	if ( __xuiProxyXgeDrawValid(pDraw) ) {
		pDraw->bDirty = 1;
	}
}

static int __xuiProxyXgeClearRectLocal(xui_rect_t tRect, uint32_t iColor)
{
	if ( (tRect.fW <= 0.0f) || (tRect.fH <= 0.0f) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	(void)xgeFlush();
	xgeClipSet(__xuiProxyXgeRect(tRect));
	xgeClear(iColor);
	xgeClipClear();
	return XGE_OK;
}

static xge_rect_t __xuiProxyXgeIntersectClip(xge_rect_t tA, xge_rect_t tB)
{
	float fLeft;
	float fTop;
	float fRight;
	float fBottom;

	fLeft = (tA.fX > tB.fX) ? tA.fX : tB.fX;
	fTop = (tA.fY > tB.fY) ? tA.fY : tB.fY;
	fRight = ((tA.fX + tA.fW) < (tB.fX + tB.fW)) ? (tA.fX + tA.fW) : (tB.fX + tB.fW);
	fBottom = ((tA.fY + tA.fH) < (tB.fY + tB.fH)) ? (tA.fY + tA.fH) : (tB.fY + tB.fH);
	if ( (fRight <= fLeft) || (fBottom <= fTop) ) {
		return (xge_rect_t){0.0f, 0.0f, 0.0f, 0.0f};
	}
	return (xge_rect_t){fLeft, fTop, fRight - fLeft, fBottom - fTop};
}

static void __xuiProxyXgeRoundRectAddArc(xge_shape_vertex_t* pVertices, int* pCount, float fCX, float fCY, float fRadius, float fStart, float fEnd, uint32_t iColor)
{
	float fT;
	float fAngle;
	int i;

	if ( (pVertices == NULL) || (pCount == NULL) ) {
		return;
	}
	for ( i = 0; i <= XUI_PROXY_XGE_ROUND_RECT_SEGMENTS; i++ ) {
		fT = (float)i / (float)XUI_PROXY_XGE_ROUND_RECT_SEGMENTS;
		fAngle = fStart + ((fEnd - fStart) * fT);
		pVertices[*pCount].fX = fCX + cosf(fAngle) * fRadius;
		pVertices[*pCount].fY = fCY + sinf(fAngle) * fRadius;
		pVertices[*pCount].iColor = iColor;
		(*pCount)++;
	}
}

static void __xuiProxyXgeShapeRoundRectFillLocal(xui_rect_t tRect, float fRadius, uint32_t iColor)
{
	xge_shape_vertex_t arrVertices[1 + ((XUI_PROXY_XGE_ROUND_RECT_SEGMENTS + 1) * 4)];
	uint32_t arrIndices[((XUI_PROXY_XGE_ROUND_RECT_SEGMENTS + 1) * 4) * 3];
	int iVertexCount;
	int iPerimeterCount;
	int i;

	tRect = xuiInternalSnapRect(tRect);
	fRadius = xuiInternalSnapPixel(fRadius);
	fRadius = __xuiProxyXgeClampRadius(tRect, fRadius);
	if ( (tRect.fW <= 0.0f) || (tRect.fH <= 0.0f) || (XGE_COLOR_GET_A(iColor) == 0) ) {
		return;
	}
	if ( fRadius <= 0.0f ) {
		xgeShapeRectFillPx(__xuiProxyXgeRect(tRect), iColor);
		return;
	}

	iVertexCount = 0;
	arrVertices[iVertexCount].fX = tRect.fX + tRect.fW * 0.5f;
	arrVertices[iVertexCount].fY = tRect.fY + tRect.fH * 0.5f;
	arrVertices[iVertexCount].iColor = iColor;
	iVertexCount++;
	__xuiProxyXgeRoundRectAddArc(arrVertices, &iVertexCount, tRect.fX + fRadius, tRect.fY + fRadius, fRadius, XUI_PROXY_XGE_PI, XUI_PROXY_XGE_PI + XUI_PROXY_XGE_PI_HALF, iColor);
	__xuiProxyXgeRoundRectAddArc(arrVertices, &iVertexCount, tRect.fX + tRect.fW - fRadius, tRect.fY + fRadius, fRadius, XUI_PROXY_XGE_PI + XUI_PROXY_XGE_PI_HALF, XUI_PROXY_XGE_PI_TWO, iColor);
	__xuiProxyXgeRoundRectAddArc(arrVertices, &iVertexCount, tRect.fX + tRect.fW - fRadius, tRect.fY + tRect.fH - fRadius, fRadius, 0.0f, XUI_PROXY_XGE_PI_HALF, iColor);
	__xuiProxyXgeRoundRectAddArc(arrVertices, &iVertexCount, tRect.fX + fRadius, tRect.fY + tRect.fH - fRadius, fRadius, XUI_PROXY_XGE_PI_HALF, XUI_PROXY_XGE_PI, iColor);
	iPerimeterCount = iVertexCount - 1;
	for ( i = 0; i < iPerimeterCount; i++ ) {
		arrIndices[(i * 3) + 0] = 0u;
		arrIndices[(i * 3) + 1] = (uint32_t)(1 + i);
		arrIndices[(i * 3) + 2] = (uint32_t)(1 + ((i + 1) % iPerimeterCount));
	}
	(void)xgeShapeMeshFillPx(arrVertices, iVertexCount, arrIndices, iPerimeterCount * 3);
}

static void __xuiProxyXgeShapeRoundRectStrokeLocal(xui_rect_t tRect, float fRadius, float fWidth, uint32_t iColor)
{
	float fOverlap;

	if ( (tRect.fW <= 0.0f) || (tRect.fH <= 0.0f) || (fWidth <= 0.0f) ) {
		return;
	}
	fWidth = xuiInternalSnapSize(fWidth);
	tRect = xuiInternalStrokeCenterRectInside(tRect, fWidth, &fRadius);
	fRadius = __xuiProxyXgeClampRadius(tRect, fRadius);
	if ( fRadius <= 0.0f ) {
		xgeShapeRectStrokePx(__xuiProxyXgeRect(xuiInternalSnapRectOut(tRect)), fWidth, iColor);
		return;
	}
	fOverlap = XUI_PROXY_XGE_ROUND_RECT_OVERLAP;
	if ( fOverlap > fRadius ) {
		fOverlap = fRadius;
	}
	xgeShapeLinePx(tRect.fX + fRadius - fOverlap, tRect.fY, tRect.fX + tRect.fW - fRadius + fOverlap, tRect.fY, fWidth, iColor);
	xgeShapeLinePx(tRect.fX + tRect.fW, tRect.fY + fRadius - fOverlap, tRect.fX + tRect.fW, tRect.fY + tRect.fH - fRadius + fOverlap, fWidth, iColor);
	xgeShapeLinePx(tRect.fX + tRect.fW - fRadius + fOverlap, tRect.fY + tRect.fH, tRect.fX + fRadius - fOverlap, tRect.fY + tRect.fH, fWidth, iColor);
	xgeShapeLinePx(tRect.fX, tRect.fY + tRect.fH - fRadius + fOverlap, tRect.fX, tRect.fY + fRadius - fOverlap, fWidth, iColor);
	xgeShapeArcPx(tRect.fX + fRadius, tRect.fY + fRadius, fRadius, XUI_PROXY_XGE_PI, XUI_PROXY_XGE_PI + XUI_PROXY_XGE_PI_HALF, fWidth, iColor);
	xgeShapeArcPx(tRect.fX + tRect.fW - fRadius, tRect.fY + fRadius, fRadius, XUI_PROXY_XGE_PI + XUI_PROXY_XGE_PI_HALF, XUI_PROXY_XGE_PI_TWO, fWidth, iColor);
	xgeShapeArcPx(tRect.fX + tRect.fW - fRadius, tRect.fY + tRect.fH - fRadius, fRadius, 0.0f, XUI_PROXY_XGE_PI_HALF, fWidth, iColor);
	xgeShapeArcPx(tRect.fX + fRadius, tRect.fY + tRect.fH - fRadius, fRadius, XUI_PROXY_XGE_PI_HALF, XUI_PROXY_XGE_PI, fWidth, iColor);
}

#if defined(_WIN32) || defined(_WIN64)
typedef HIMC (WINAPI *xui_imm_get_context_proc)(HWND);
typedef BOOL (WINAPI *xui_imm_release_context_proc)(HWND, HIMC);
typedef BOOL (WINAPI *xui_imm_set_candidate_window_proc)(HIMC, LPCANDIDATEFORM);

static HMODULE g_xuiProxyXgeImm32;
static xui_imm_get_context_proc g_xuiProxyXgeImmGetContext;
static xui_imm_release_context_proc g_xuiProxyXgeImmReleaseContext;
static xui_imm_set_candidate_window_proc g_xuiProxyXgeImmSetCandidateWindow;

static int __xuiProxyXgeImeEnsureWin32(void)
{
	if ( g_xuiProxyXgeImm32 != NULL ) {
		return (g_xuiProxyXgeImmGetContext != NULL) &&
		       (g_xuiProxyXgeImmReleaseContext != NULL) &&
		       (g_xuiProxyXgeImmSetCandidateWindow != NULL);
	}
	g_xuiProxyXgeImm32 = LoadLibraryA("imm32.dll");
	if ( g_xuiProxyXgeImm32 == NULL ) {
		return 0;
	}
	g_xuiProxyXgeImmGetContext = (xui_imm_get_context_proc)GetProcAddress(g_xuiProxyXgeImm32, "ImmGetContext");
	g_xuiProxyXgeImmReleaseContext = (xui_imm_release_context_proc)GetProcAddress(g_xuiProxyXgeImm32, "ImmReleaseContext");
	g_xuiProxyXgeImmSetCandidateWindow = (xui_imm_set_candidate_window_proc)GetProcAddress(g_xuiProxyXgeImm32, "ImmSetCandidateWindow");
	return (g_xuiProxyXgeImmGetContext != NULL) &&
	       (g_xuiProxyXgeImmReleaseContext != NULL) &&
	       (g_xuiProxyXgeImmSetCandidateWindow != NULL);
}

static HWND __xuiProxyXgeImeWindowGet(void)
{
	HWND hWnd;

	hWnd = GetFocus();
	if ( hWnd == NULL ) {
		hWnd = GetActiveWindow();
	}
	if ( hWnd == NULL ) {
		hWnd = GetForegroundWindow();
	}
	return hWnd;
}
#endif

static int __xuiProxyXgeGetCaps(xui_proxy pProxy, xui_proxy_caps_t* pCaps)
{
	xge_sampler_t tSampler;

	if ( (pProxy == NULL) || (pCaps == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	(void)pProxy;
	memset(pCaps, 0, sizeof(*pCaps));
	pCaps->iSize = (uint32_t)sizeof(*pCaps);
	pCaps->iCaps = XUI_PROXY_CAP_SURFACE_TARGET |
	               XUI_PROXY_CAP_SURFACE_READ |
	               XUI_PROXY_CAP_SURFACE_QUAD |
	               XUI_PROXY_CAP_SURFACE_CLEAR_RECT |
	               XUI_PROXY_CAP_SURFACE_SAMPLER |
	               XUI_PROXY_CAP_DRAW_CONTEXT |
	               XUI_PROXY_CAP_SHAPE |
	               XUI_PROXY_CAP_MESH_TRIANGLES |
	               XUI_PROXY_CAP_FONT_TTF |
	               XUI_PROXY_CAP_FONT_XRF |
	               XUI_PROXY_CAP_TEXT;
	pCaps->iSurfaceFormat = XUI_SURFACE_FORMAT_RGBA8;
	pCaps->iInternalAlpha = XUI_SURFACE_ALPHA_PREMULTIPLIED;
	tSampler = xgeSamplerDefault();
	pCaps->tDefaultSampler = __xuiProxyXgeSurfaceSampler(tSampler);
	return XGE_OK;
}

static int __xuiProxyXgeClipboardSetText(xui_proxy pProxy, const char* sText)
{
	if ( pProxy == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	(void)pProxy;
	xgeClipboardSetText(sText);
	return XGE_OK;
}

static int __xuiProxyXgeClipboardGetText(xui_proxy pProxy, char* sText, int iCapacity)
{
	const char* sSource;
	size_t iLength;
	size_t iCopy;

	if ( (pProxy == NULL) || (sText == NULL) || (iCapacity <= 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	(void)pProxy;
	sSource = xgeClipboardGetText();
	if ( sSource == NULL ) {
		sSource = "";
	}
	iLength = strlen(sSource);
	iCopy = iLength;
	if ( iCopy >= (size_t)iCapacity ) {
		iCopy = (size_t)iCapacity - 1u;
	}
	if ( iCopy > 0u ) {
		memcpy(sText, sSource, iCopy);
	}
	sText[iCopy] = 0;
	return (int)iLength;
}

static int __xuiProxyXgeImeGetEnabled(xui_proxy pProxy)
{
	if ( pProxy == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	(void)pProxy;
	return xgeImeGetEnabled();
}

static int __xuiProxyXgeImeSetEnabled(xui_proxy pProxy, int bEnabled)
{
	if ( pProxy == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	(void)pProxy;
	return xgeImeSetEnabled(bEnabled);
}

static int __xuiProxyXgeImeSetCandidateRect(xui_proxy pProxy, xui_rect_t tRect)
{
#if defined(_WIN32) || defined(_WIN64)
	HWND hWnd;
	HIMC hImc;
	CANDIDATEFORM tForm;

	if ( pProxy == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	(void)pProxy;
	if ( __xuiProxyXgeImeEnsureWin32() == 0 ) {
		return XGE_ERROR_UNSUPPORTED;
	}
	hWnd = __xuiProxyXgeImeWindowGet();
	if ( hWnd == NULL ) {
		return XGE_ERROR_UNSUPPORTED;
	}
	hImc = g_xuiProxyXgeImmGetContext(hWnd);
	if ( hImc == NULL ) {
		return XGE_ERROR_UNSUPPORTED;
	}
	memset(&tForm, 0, sizeof(tForm));
	tForm.dwIndex = 0;
	tForm.dwStyle = CFS_CANDIDATEPOS;
	tForm.ptCurrentPos.x = (LONG)tRect.fX;
	tForm.ptCurrentPos.y = (LONG)(tRect.fY + tRect.fH);
	if ( g_xuiProxyXgeImmSetCandidateWindow(hImc, &tForm) == 0 ) {
		(void)g_xuiProxyXgeImmReleaseContext(hWnd, hImc);
		return XGE_ERROR_BACKEND_FAILED;
	}
	(void)g_xuiProxyXgeImmReleaseContext(hWnd, hImc);
	return XGE_OK;
#else
	(void)tRect;
	if ( pProxy == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	(void)pProxy;
	return XGE_ERROR_UNSUPPORTED;
#endif
}

static int __xuiProxyXgeSurfaceCreate(xui_proxy pProxy, xui_surface* ppSurface, const xui_surface_desc_t* pDesc)
{
	xui_surface pSurface;
	int iRet;
	int iKind;
	int iFormat;

	if ( (pProxy == NULL) || (ppSurface == NULL) || (pDesc == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	(void)pProxy;
	*ppSurface = NULL;
	iKind = (pDesc->iKind != 0) ? pDesc->iKind : XUI_SURFACE_KIND_TEXTURE;
	iFormat = (pDesc->iFormat != 0) ? pDesc->iFormat : XUI_SURFACE_FORMAT_RGBA8;
	if ( (iKind != XUI_SURFACE_KIND_TEXTURE) || (iFormat != XUI_SURFACE_FORMAT_RGBA8) ||
	     (pDesc->iWidth <= 0) || (pDesc->iHeight <= 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pSurface = __xuiProxyXgeSurfaceAlloc(pDesc->iFlags);
	if ( pSurface == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	if ( (pSurface->iFlags & XUI_SURFACE_USAGE_TARGET) != 0 ) {
		iRet = xgeRenderTargetCreate(&pSurface->tTarget, pDesc->iWidth, pDesc->iHeight);
	} else {
		iRet = xgeTextureCreateRGBA(&pSurface->tTexture, pDesc->iWidth, pDesc->iHeight, NULL);
	}
	if ( iRet != XGE_OK ) {
		xrtFree(pSurface);
		return iRet;
	}
	*ppSurface = pSurface;
	return XGE_OK;
}

static int __xuiProxyXgeSurfaceCreateRGBA(xui_proxy pProxy, xui_surface* ppSurface, int iWidth, int iHeight, const void* pPixels, int iStride, uint32_t iFlags)
{
	xui_surface pSurface;
	void* pPacked;
	const void* pUploadPixels;
	int iRet;
	xui_rect_i_t tRect;

	if ( (pProxy == NULL) || (ppSurface == NULL) || (iWidth <= 0) || (iHeight <= 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	(void)pProxy;
	*ppSurface = NULL;
	iRet = __xuiProxyXgeSurfacePackRGBA(pPixels, iWidth, iHeight, iStride, iFlags, &pPacked, &pUploadPixels);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	pSurface = __xuiProxyXgeSurfaceAlloc(iFlags);
	if ( pSurface == NULL ) {
		if ( pPacked != NULL ) {
			xrtFree(pPacked);
		}
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	if ( (pSurface->iFlags & XUI_SURFACE_USAGE_TARGET) != 0 ) {
		iRet = xgeRenderTargetCreate(&pSurface->tTarget, iWidth, iHeight);
		if ( (iRet == XGE_OK) && (pUploadPixels != NULL) ) {
			tRect.iX = 0;
			tRect.iY = 0;
			tRect.iW = iWidth;
			tRect.iH = iHeight;
			iRet = xgeTextureUpdateRGBA(&pSurface->tTarget.tTexture, tRect.iX, tRect.iY, tRect.iW, tRect.iH, pUploadPixels, iWidth * 4);
		}
	} else {
		iRet = xgeTextureCreateRGBA(&pSurface->tTexture, iWidth, iHeight, pUploadPixels);
	}
	if ( pPacked != NULL ) {
		xrtFree(pPacked);
	}
	if ( iRet != XGE_OK ) {
		xrtFree(pSurface);
		return iRet;
	}
	*ppSurface = pSurface;
	return XGE_OK;
}

static int __xuiProxyXgeSurfaceLoadFile(xui_proxy pProxy, xui_surface* ppSurface, const char* sPath, uint32_t iFlags)
{
	xui_surface pSurface;
	int iRet;

	if ( (pProxy == NULL) || (ppSurface == NULL) || (sPath == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( (iFlags & XUI_SURFACE_USAGE_TARGET) != 0 ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	(void)pProxy;
	*ppSurface = NULL;
	pSurface = __xuiProxyXgeSurfaceAlloc(iFlags);
	if ( pSurface == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	iRet = xgeTextureLoadEx(&pSurface->tTexture, sPath, __xuiProxyXgeImageFlags(iFlags));
	if ( iRet != XGE_OK ) {
		xrtFree(pSurface);
		return iRet;
	}
	*ppSurface = pSurface;
	return XGE_OK;
}

static int __xuiProxyXgeSurfaceLoadMemory(xui_proxy pProxy, xui_surface* ppSurface, const void* pData, int iSize, uint32_t iFlags)
{
	xui_surface pSurface;
	int iRet;

	if ( (pProxy == NULL) || (ppSurface == NULL) || (pData == NULL) || (iSize <= 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( (iFlags & XUI_SURFACE_USAGE_TARGET) != 0 ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	(void)pProxy;
	*ppSurface = NULL;
	pSurface = __xuiProxyXgeSurfaceAlloc(iFlags);
	if ( pSurface == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	iRet = xgeTextureLoadMemoryEx(&pSurface->tTexture, pData, iSize, __xuiProxyXgeImageFlags(iFlags));
	if ( iRet != XGE_OK ) {
		xrtFree(pSurface);
		return iRet;
	}
	*ppSurface = pSurface;
	return XGE_OK;
}

static int __xuiProxyXgeSurfaceUpdateRGBA(xui_proxy pProxy, xui_surface pSurface, xui_rect_i_t tRect, const void* pPixels, int iStride)
{
	xge_texture pTexture;
	int iRet;

	if ( pProxy == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	(void)pProxy;
	if ( !__xuiProxyXgeSurfaceValid(pSurface) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pTexture = __xuiProxyXgeSurfaceTexture(pSurface);
	if ( pTexture == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	iRet = xgeTextureUpdateRGBA(pTexture, tRect.iX, tRect.iY, tRect.iW, tRect.iH, pPixels, iStride);
	if ( iRet == XGE_OK ) {
		__xuiProxyXgeSurfaceBump(pSurface);
	}
	return iRet;
}

static int __xuiProxyXgeSurfaceReadRGBA(xui_proxy pProxy, xui_surface pSurface, void* pPixels, int iStride)
{
	xge_texture pTexture;

	if ( pProxy == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	(void)pProxy;
	if ( !__xuiProxyXgeSurfaceValid(pSurface) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( (pSurface->iFlags & XUI_SURFACE_USAGE_TARGET) != 0 ) {
		return xgeRenderTargetReadPixels(&pSurface->tTarget, pPixels, iStride);
	}
	pTexture = __xuiProxyXgeSurfaceTexture(pSurface);
	if ( pTexture == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	return xgeTextureReadPixels(pTexture, pPixels, iStride);
}

static int __xuiProxyXgeSurfaceGetDesc(xui_proxy pProxy, xui_surface pSurface, xui_surface_desc_t* pDesc)
{
	if ( (pProxy == NULL) || (pDesc == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	(void)pProxy;
	if ( !__xuiProxyXgeSurfaceValid(pSurface) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	memset(pDesc, 0, sizeof(*pDesc));
	pDesc->iKind = pSurface->iKind;
	pDesc->iFormat = pSurface->iFormat;
	(void)__xuiProxyXgeSurfaceSize(pSurface, &pDesc->iWidth, &pDesc->iHeight);
	pDesc->iFlags = pSurface->iFlags;
	return XGE_OK;
}

static int __xuiProxyXgeSurfaceDraw(xui_proxy pProxy, xui_surface pSurface, xui_rect_t tSrc, xui_rect_t tDst, uint32_t iColor, uint32_t iFlags)
{
	xge_draw_t tDraw;
	xge_texture pTexture;

	if ( pProxy == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	(void)pProxy;
	if ( !__xuiProxyXgeSurfaceValid(pSurface) || (tDst.fW == 0.0f) || (tDst.fH == 0.0f) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( XGE_COLOR_GET_A(iColor) == 0 ) {
		return XGE_OK;
	}
	pTexture = __xuiProxyXgeSurfaceTexture(pSurface);
	if ( pTexture == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	memset(&tDraw, 0, sizeof(tDraw));
	tDraw.pTexture = pTexture;
	tDraw.tSrc = __xuiProxyXgeSurfaceSourceRect(pSurface, tSrc);
	tDraw.tDst.fX = tDst.fX;
	tDraw.tDst.fY = tDst.fY;
	tDraw.tDst.fW = tDst.fW;
	tDraw.tDst.fH = tDst.fH;
	tDraw.iColor = iColor;
	tDraw.iFlags = __xuiProxyXgeSurfaceDrawFlags(pSurface, iFlags);
	xgeDrawEx(&tDraw);
	return XGE_OK;
}

static int __xuiProxyXgeSurfaceClear(xui_proxy pProxy, xui_surface pTarget, uint32_t iColor)
{
	xge_pass_t tPass;
	int iRet;

	if ( pProxy == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	(void)pProxy;
	if ( !__xuiProxyXgeSurfaceTargetValid(pTarget) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	(void)xgeFlush();
	xgePassInit(&tPass, &pTarget->tTarget, XGE_PASS_CLEAR_COLOR, iColor);
	iRet = xgePassBegin(&tPass);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	iRet = xgePassEnd(&tPass);
	if ( iRet == XGE_OK ) {
		__xuiProxyXgeSurfaceBump(pTarget);
	}
	return iRet;
}

static int __xuiProxyXgeSurfaceClearRect(xui_proxy pProxy, xui_surface pTarget, xui_rect_t tRect, uint32_t iColor)
{
	xge_pass_t tPass;
	int iRet;

	iRet = __xuiProxyXgeTargetBegin(pProxy, pTarget, &tPass);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	iRet = __xuiProxyXgeClearRectLocal(tRect, iColor);
	return __xuiProxyXgeTargetEndDirty(&tPass, pTarget, iRet);
}

static int __xuiProxyXgeSurfaceDrawTo(xui_proxy pProxy, xui_surface pTarget, xui_surface pSurface, xui_rect_t tSrc, xui_rect_t tDst, uint32_t iColor, uint32_t iFlags)
{
	xge_pass_t tPass;
	int iRet;

	if ( pProxy == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( !__xuiProxyXgeSurfaceTargetValid(pTarget) || !__xuiProxyXgeSurfaceValid(pSurface) || (pTarget == pSurface) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( XGE_COLOR_GET_A(iColor) == 0 ) {
		return XGE_OK;
	}
	(void)xgeFlush();
	xgePassInit(&tPass, &pTarget->tTarget, 0, 0);
	iRet = xgePassBegin(&tPass);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	iRet = __xuiProxyXgeSurfaceDraw(pProxy, pSurface, tSrc, tDst, iColor, iFlags | XUI_SURFACE_DRAW_SCREEN_SPACE);
	if ( iRet == XGE_OK ) {
		iRet = xgeFlush();
	}
	if ( xgePassEnd(&tPass) != XGE_OK && iRet == XGE_OK ) {
		iRet = XGE_ERROR_BACKEND_FAILED;
	}
	if ( iRet == XGE_OK ) {
		__xuiProxyXgeSurfaceBump(pTarget);
	}
	return iRet;
}

static int __xuiProxyXgeSurfaceDrawQuad(xui_proxy pProxy, xui_surface pSurface, const xui_surface_vertex_t* pVertices, uint32_t iFlags)
{
	xge_vertex_t arrVertices[4];
	xge_texture pTexture;

	if ( (pProxy == NULL) || (pVertices == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	(void)pProxy;
	if ( !__xuiProxyXgeSurfaceValid(pSurface) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( (XGE_COLOR_GET_A(pVertices[0].iColor) == 0) &&
	     (XGE_COLOR_GET_A(pVertices[1].iColor) == 0) &&
	     (XGE_COLOR_GET_A(pVertices[2].iColor) == 0) &&
	     (XGE_COLOR_GET_A(pVertices[3].iColor) == 0) ) {
		return XGE_OK;
	}
	pTexture = __xuiProxyXgeSurfaceTexture(pSurface);
	if ( pTexture == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	(void)xgeFlush();
	if ( pTexture->iBackendId == 0 ) {
		return XGE_ERROR_NOT_INITIALIZED;
	}
	__xuiProxyXgeSurfaceVertexCopy(arrVertices, pVertices);
	__xuiProxyXgeSurfaceVertexMapUV(pSurface, arrVertices, iFlags);
	xgeDrawQuad3D(pTexture, arrVertices, __xuiProxyXgeDrawFlags(iFlags & XUI_SURFACE_DRAW_SCREEN_SPACE));
	return XGE_OK;
}

static int __xuiProxyXgeSurfaceDrawQuadTo(xui_proxy pProxy, xui_surface pTarget, xui_surface pSurface, const xui_surface_vertex_t* pVertices, uint32_t iFlags)
{
	xge_pass_t tPass;
	int iRet;

	if ( pProxy == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( !__xuiProxyXgeSurfaceTargetValid(pTarget) || !__xuiProxyXgeSurfaceValid(pSurface) || (pTarget == pSurface) || (pVertices == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( (XGE_COLOR_GET_A(pVertices[0].iColor) == 0) &&
	     (XGE_COLOR_GET_A(pVertices[1].iColor) == 0) &&
	     (XGE_COLOR_GET_A(pVertices[2].iColor) == 0) &&
	     (XGE_COLOR_GET_A(pVertices[3].iColor) == 0) ) {
		return XGE_OK;
	}
	(void)xgeFlush();
	xgePassInit(&tPass, &pTarget->tTarget, 0, 0);
	iRet = xgePassBegin(&tPass);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	iRet = __xuiProxyXgeSurfaceDrawQuad(pProxy, pSurface, pVertices, iFlags | XUI_SURFACE_DRAW_SCREEN_SPACE);
	if ( xgePassEnd(&tPass) != XGE_OK && iRet == XGE_OK ) {
		iRet = XGE_ERROR_BACKEND_FAILED;
	}
	if ( iRet == XGE_OK ) {
		__xuiProxyXgeSurfaceBump(pTarget);
	}
	return iRet;
}

static int __xuiProxyXgeSurfaceGetSampler(xui_proxy pProxy, xui_surface pSurface, xui_surface_sampler_t* pSampler)
{
	xge_texture pTexture;

	if ( (pProxy == NULL) || (pSampler == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	(void)pProxy;
	pTexture = __xuiProxyXgeSurfaceTexture(pSurface);
	if ( pTexture == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	*pSampler = __xuiProxyXgeSurfaceSampler(xgeTextureGetSampler(pTexture));
	return XGE_OK;
}

static int __xuiProxyXgeSurfaceSetSampler(xui_proxy pProxy, xui_surface pSurface, const xui_surface_sampler_t* pSampler)
{
	xge_texture pTexture;
	xge_sampler_t tSampler;
	int iRet;

	if ( (pProxy == NULL) || (pSampler == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	(void)pProxy;
	pTexture = __xuiProxyXgeSurfaceTexture(pSurface);
	if ( pTexture == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	tSampler = __xuiProxyXgeSampler(pSampler);
	iRet = xgeTextureSetSampler(pTexture, &tSampler);
	if ( iRet == XGE_OK ) {
		__xuiProxyXgeSurfaceBump(pSurface);
	}
	return iRet;
}

static int __xuiProxyXgeSurfaceGetGeneration(xui_proxy pProxy, xui_surface pSurface, uint32_t* pGeneration)
{
	if ( (pProxy == NULL) || (pGeneration == NULL) || !__xuiProxyXgeSurfaceValid(pSurface) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	(void)pProxy;
	*pGeneration = pSurface->iGeneration;
	return XGE_OK;
}

static int __xuiProxyXgeShapePoint(xui_proxy pProxy, xui_surface pTarget, float fX, float fY, float fSize, uint32_t iColor)
{
	xge_pass_t tPass;
	int iRet;

	if ( fSize <= 0.0f ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( (pProxy == NULL) || !__xuiProxyXgeSurfaceTargetValid(pTarget) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( XGE_COLOR_GET_A(iColor) == 0 ) {
		return XGE_OK;
	}
	iRet = __xuiProxyXgeTargetBegin(pProxy, pTarget, &tPass);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	xgeShapePointPx(fX, fY, fSize, iColor);
	return __xuiProxyXgeTargetEndDirty(&tPass, pTarget, XGE_OK);
}

static int __xuiProxyXgeShapeLine(xui_proxy pProxy, xui_surface pTarget, float fX0, float fY0, float fX1, float fY1, float fWidth, uint32_t iColor)
{
	xge_pass_t tPass;
	int iRet;

	if ( fWidth <= 0.0f ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( (pProxy == NULL) || !__xuiProxyXgeSurfaceTargetValid(pTarget) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( XGE_COLOR_GET_A(iColor) == 0 ) {
		return XGE_OK;
	}
	iRet = __xuiProxyXgeTargetBegin(pProxy, pTarget, &tPass);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	xgeShapeLinePx(fX0, fY0, fX1, fY1, fWidth, iColor);
	return __xuiProxyXgeTargetEndDirty(&tPass, pTarget, XGE_OK);
}

static int __xuiProxyXgeShapeTriangleFill(xui_proxy pProxy, xui_surface pTarget, xui_vec2_t tA, xui_vec2_t tB, xui_vec2_t tC, uint32_t iColor)
{
	xge_pass_t tPass;
	int iRet;

	if ( (pProxy == NULL) || !__xuiProxyXgeSurfaceTargetValid(pTarget) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( XGE_COLOR_GET_A(iColor) == 0 ) {
		return XGE_OK;
	}
	iRet = __xuiProxyXgeTargetBegin(pProxy, pTarget, &tPass);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	xgeShapeTriangleFillPx(__xuiProxyXgeVec2(tA), __xuiProxyXgeVec2(tB), __xuiProxyXgeVec2(tC), iColor);
	return __xuiProxyXgeTargetEndDirty(&tPass, pTarget, XGE_OK);
}

static int __xuiProxyXgeShapeTriangleStroke(xui_proxy pProxy, xui_surface pTarget, xui_vec2_t tA, xui_vec2_t tB, xui_vec2_t tC, float fWidth, uint32_t iColor)
{
	xge_pass_t tPass;
	int iRet;

	if ( fWidth <= 0.0f ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( (pProxy == NULL) || !__xuiProxyXgeSurfaceTargetValid(pTarget) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( XGE_COLOR_GET_A(iColor) == 0 ) {
		return XGE_OK;
	}
	iRet = __xuiProxyXgeTargetBegin(pProxy, pTarget, &tPass);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	xgeShapeLinePx(tA.fX, tA.fY, tB.fX, tB.fY, fWidth, iColor);
	xgeShapeLinePx(tB.fX, tB.fY, tC.fX, tC.fY, fWidth, iColor);
	xgeShapeLinePx(tC.fX, tC.fY, tA.fX, tA.fY, fWidth, iColor);
	return __xuiProxyXgeTargetEndDirty(&tPass, pTarget, XGE_OK);
}

static int __xuiProxyXgeShapeRectFill(xui_proxy pProxy, xui_surface pTarget, xui_rect_t tRect, uint32_t iColor)
{
	xge_pass_t tPass;
	int iRet;

	if ( (tRect.fW <= 0.0f) || (tRect.fH <= 0.0f) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( (pProxy == NULL) || !__xuiProxyXgeSurfaceTargetValid(pTarget) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( XGE_COLOR_GET_A(iColor) == 0 ) {
		return XGE_OK;
	}
	iRet = __xuiProxyXgeTargetBegin(pProxy, pTarget, &tPass);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	xgeShapeRectFillPx(__xuiProxyXgeRect(tRect), iColor);
	return __xuiProxyXgeTargetEndDirty(&tPass, pTarget, XGE_OK);
}

static int __xuiProxyXgeShapeRectStroke(xui_proxy pProxy, xui_surface pTarget, xui_rect_t tRect, float fWidth, uint32_t iColor)
{
	xge_pass_t tPass;
	int iRet;

	if ( (tRect.fW <= 0.0f) || (tRect.fH <= 0.0f) || (fWidth <= 0.0f) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( (pProxy == NULL) || !__xuiProxyXgeSurfaceTargetValid(pTarget) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( XGE_COLOR_GET_A(iColor) == 0 ) {
		return XGE_OK;
	}
	iRet = __xuiProxyXgeTargetBegin(pProxy, pTarget, &tPass);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	xgeShapeRectStrokePx(__xuiProxyXgeRect(tRect), fWidth, iColor);
	return __xuiProxyXgeTargetEndDirty(&tPass, pTarget, XGE_OK);
}

static int __xuiProxyXgeShapeCircleFill(xui_proxy pProxy, xui_surface pTarget, float fX, float fY, float fRadius, uint32_t iColor)
{
	xge_pass_t tPass;
	int iRet;

	if ( fRadius <= 0.0f ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( (pProxy == NULL) || !__xuiProxyXgeSurfaceTargetValid(pTarget) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( XGE_COLOR_GET_A(iColor) == 0 ) {
		return XGE_OK;
	}
	iRet = __xuiProxyXgeTargetBegin(pProxy, pTarget, &tPass);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	xgeShapeCircleFillPx(fX, fY, fRadius, iColor);
	return __xuiProxyXgeTargetEndDirty(&tPass, pTarget, XGE_OK);
}

static int __xuiProxyXgeShapeCircleStroke(xui_proxy pProxy, xui_surface pTarget, float fX, float fY, float fRadius, float fWidth, uint32_t iColor)
{
	xge_pass_t tPass;
	int iRet;

	if ( (fRadius <= 0.0f) || (fWidth <= 0.0f) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( (pProxy == NULL) || !__xuiProxyXgeSurfaceTargetValid(pTarget) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( XGE_COLOR_GET_A(iColor) == 0 ) {
		return XGE_OK;
	}
	iRet = __xuiProxyXgeTargetBegin(pProxy, pTarget, &tPass);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	xgeShapeCircleStrokePx(fX, fY, fRadius, fWidth, iColor);
	return __xuiProxyXgeTargetEndDirty(&tPass, pTarget, XGE_OK);
}

static int __xuiProxyXgeShapeRoundRectFill(xui_proxy pProxy, xui_surface pTarget, xui_rect_t tRect, float fRadius, uint32_t iColor)
{
	xge_pass_t tPass;
	int iRet;

	if ( (tRect.fW <= 0.0f) || (tRect.fH <= 0.0f) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( (pProxy == NULL) || !__xuiProxyXgeSurfaceTargetValid(pTarget) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( XGE_COLOR_GET_A(iColor) == 0 ) {
		return XGE_OK;
	}
	iRet = __xuiProxyXgeTargetBegin(pProxy, pTarget, &tPass);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	__xuiProxyXgeShapeRoundRectFillLocal(tRect, fRadius, iColor);
	return __xuiProxyXgeTargetEndDirty(&tPass, pTarget, XGE_OK);
}

static int __xuiProxyXgeShapeRoundRectStroke(xui_proxy pProxy, xui_surface pTarget, xui_rect_t tRect, float fRadius, float fWidth, uint32_t iColor)
{
	xge_pass_t tPass;
	int iRet;

	if ( (tRect.fW <= 0.0f) || (tRect.fH <= 0.0f) || (fWidth <= 0.0f) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( (pProxy == NULL) || !__xuiProxyXgeSurfaceTargetValid(pTarget) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( XGE_COLOR_GET_A(iColor) == 0 ) {
		return XGE_OK;
	}
	iRet = __xuiProxyXgeTargetBegin(pProxy, pTarget, &tPass);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	__xuiProxyXgeShapeRoundRectStrokeLocal(tRect, fRadius, fWidth, iColor);
	return __xuiProxyXgeTargetEndDirty(&tPass, pTarget, XGE_OK);
}

static int __xuiProxyXgeFontLoadFile(xui_proxy pProxy, xui_font* ppFont, const char* sPath, float fSize, uint32_t iFlags)
{
	xui_font pFont;
	int iRet;
	int bUseXrf;

	if ( (pProxy == NULL) || (ppFont == NULL) || (sPath == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	(void)pProxy;
	*ppFont = NULL;
	pFont = (xui_font)xrtCalloc(1, sizeof(*pFont));
	if ( pFont == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	bUseXrf = __xuiProxyXgeFontUseXrf(sPath, iFlags);
	if ( bUseXrf ) {
		iRet = xgeFontLoadXRF(&pFont->tFont, sPath);
	} else {
		if ( fSize <= 0.0f ) {
			xrtFree(pFont);
			return XGE_ERROR_INVALID_ARGUMENT;
		}
		iRet = xgeFontLoad(&pFont->tFont, sPath, fSize);
	}
	if ( iRet != XGE_OK ) {
		xrtFree(pFont);
		return iRet;
	}
	if ( !bUseXrf && xgeFontFallbackGet(&pFont->tFallbackFont, fSize) == XGE_OK ) {
		xgeFontSetFallback(&pFont->tFont, &pFont->tFallbackFont);
		pFont->bHasFallback = 1;
	}
	pFont->iMagic = XUI_PROXY_XGE_FONT_MAGIC;
	pFont->iFlags = iFlags;
	*ppFont = pFont;
	return XGE_OK;
}

static int __xuiProxyXgeFontLoadMemory(xui_proxy pProxy, xui_font* ppFont, const void* pData, int iSize, float fSize, uint32_t iFlags)
{
	xui_font pFont;
	int iRet;

	if ( (pProxy == NULL) || (ppFont == NULL) || (pData == NULL) || (iSize <= 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	(void)pProxy;
	*ppFont = NULL;
	pFont = (xui_font)xrtCalloc(1, sizeof(*pFont));
	if ( pFont == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	if ( (iFlags & XUI_FONT_FORMAT_XRF) != 0 ) {
		iRet = xgeFontLoadXRFMemory(&pFont->tFont, pData, iSize);
	} else {
		if ( fSize <= 0.0f ) {
			xrtFree(pFont);
			return XGE_ERROR_INVALID_ARGUMENT;
		}
		iRet = xgeFontLoadMemory(&pFont->tFont, pData, iSize, fSize);
	}
	if ( iRet != XGE_OK ) {
		xrtFree(pFont);
		return iRet;
	}
	if ( (iFlags & XUI_FONT_FORMAT_XRF) == 0 && xgeFontFallbackGet(&pFont->tFallbackFont, fSize) == XGE_OK ) {
		xgeFontSetFallback(&pFont->tFont, &pFont->tFallbackFont);
		pFont->bHasFallback = 1;
	}
	pFont->iMagic = XUI_PROXY_XGE_FONT_MAGIC;
	pFont->iFlags = iFlags;
	*ppFont = pFont;
	return XGE_OK;
}

static int __xuiProxyXgeFontGetMetrics(xui_proxy pProxy, xui_font pFont, xui_font_metrics_t* pMetrics)
{
	if ( (pProxy == NULL) || !__xuiProxyXgeFontValid(pFont) || (pMetrics == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	(void)pProxy;
	pMetrics->fSize = pFont->tFont.fSize;
	pMetrics->fAscent = pFont->tFont.fAscent;
	pMetrics->fDescent = pFont->tFont.fDescent;
	pMetrics->fLineGap = pFont->tFont.fLineGap;
	pMetrics->fLineHeight = pFont->tFont.fLineHeight;
	return XGE_OK;
}

static void __xuiProxyXgeFontDestroy(xui_proxy pProxy, xui_font pFont)
{
	if ( pProxy == NULL ) {
		return;
	}
	(void)pProxy;
	if ( !__xuiProxyXgeFontValid(pFont) ) {
		return;
	}
	xgeFontFree(&pFont->tFont);
	if ( pFont->bHasFallback ) {
		xgeFontFree(&pFont->tFallbackFont);
	}
	pFont->iMagic = 0;
	xrtFree(pFont);
}

static int __xuiProxyXgeTextMeasure(xui_proxy pProxy, xui_font pFont, const char* sText, xui_vec2_t* pSize)
{
	xge_vec2_t tSize;

	if ( (pProxy == NULL) || !__xuiProxyXgeFontValid(pFont) || (sText == NULL) || (pSize == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	(void)pProxy;
	tSize = xgeTextMeasure(&pFont->tFont, sText);
	pSize->fX = tSize.fX;
	pSize->fY = tSize.fY;
	return XGE_OK;
}

static int __xuiProxyXgeTextDraw(xui_proxy pProxy, xui_surface pTarget, xui_font pFont, const char* sText, xui_rect_t tRect, uint32_t iColor, uint32_t iFlags)
{
	xge_pass_t tPass;
	int iRet;

	if ( !__xuiProxyXgeFontValid(pFont) || (sText == NULL) || (tRect.fW <= 0.0f) || (tRect.fH <= 0.0f) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	iRet = __xuiProxyXgeTargetBegin(pProxy, pTarget, &tPass);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	xgeTextDrawRect(&pFont->tFont, sText, __xuiProxyXgeRect(tRect), iColor, __xuiProxyXgeTextFlags(iFlags));
	return __xuiProxyXgeTargetEndDirty(&tPass, pTarget, XGE_OK);
}

static int __xuiProxyXgeDrawBegin(xui_proxy pProxy, xui_draw_context* ppDraw, xui_surface pTarget)
{
	xui_draw_context pDraw;
	int iRet;

	if ( (pProxy == NULL) || (ppDraw == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	*ppDraw = NULL;
	pDraw = (xui_draw_context)xrtCalloc(1, sizeof(*pDraw));
	if ( pDraw == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pDraw->iMagic = XUI_PROXY_XGE_DRAW_MAGIC;
	pDraw->pTarget = pTarget;
	iRet = __xuiProxyXgeTargetBegin(pProxy, pTarget, &pDraw->tPass);
	if ( iRet != XGE_OK ) {
		xrtFree(pDraw);
		return iRet;
	}
	*ppDraw = pDraw;
	return XGE_OK;
}

static int __xuiProxyXgeDrawEnd(xui_proxy pProxy, xui_draw_context pDraw)
{
	xui_surface pTarget;
	int bDirty;
	int iRet;

	if ( (pProxy == NULL) || !__xuiProxyXgeDrawValid(pDraw) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	(void)pProxy;
	pTarget = pDraw->pTarget;
	bDirty = pDraw->bDirty;
	iRet = __xuiProxyXgeTargetEnd(&pDraw->tPass, XGE_OK);
	if ( (iRet == XGE_OK) && bDirty ) {
		__xuiProxyXgeSurfaceBump(pTarget);
	}
	pDraw->iMagic = 0;
	xrtFree(pDraw);
	return iRet;
}

static int __xuiProxyXgeDrawClearRect(xui_proxy pProxy, xui_draw_context pDraw, xui_rect_t tRect, uint32_t iColor)
{
	int iRet;

	if ( (pProxy == NULL) || !__xuiProxyXgeDrawValid(pDraw) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	(void)pProxy;
	iRet = __xuiProxyXgeClearRectLocal(tRect, iColor);
	if ( iRet == XGE_OK ) {
		__xuiProxyXgeDrawMarkDirty(pDraw);
	}
	return iRet;
}

static int __xuiProxyXgeDrawSurface(xui_proxy pProxy, xui_draw_context pDraw, xui_surface pSurface, xui_rect_t tSrc, xui_rect_t tDst, uint32_t iColor, uint32_t iFlags)
{
	int iRet;

	if ( (pProxy == NULL) || !__xuiProxyXgeDrawValid(pDraw) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	iRet = __xuiProxyXgeSurfaceDraw(pProxy, pSurface, tSrc, tDst, iColor, iFlags | XUI_SURFACE_DRAW_SCREEN_SPACE);
	if ( iRet == XGE_OK ) {
		__xuiProxyXgeDrawMarkDirty(pDraw);
	}
	return iRet;
}

static int __xuiProxyXgeDrawSurfaceQuad(xui_proxy pProxy, xui_draw_context pDraw, xui_surface pSurface, const xui_surface_vertex_t* pVertices, uint32_t iFlags)
{
	int iRet;

	if ( (pProxy == NULL) || !__xuiProxyXgeDrawValid(pDraw) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	iRet = __xuiProxyXgeSurfaceDrawQuad(pProxy, pSurface, pVertices, iFlags | XUI_SURFACE_DRAW_SCREEN_SPACE);
	if ( iRet == XGE_OK ) {
		__xuiProxyXgeDrawMarkDirty(pDraw);
	}
	return iRet;
}

static int __xuiProxyXgeDrawMeshTriangles(xui_proxy pProxy, xui_draw_context pDraw, const xui_mesh_vertex_t* pVertices, int iVertexCount, const uint32_t* pIndices, int iIndexCount, uint32_t iFlags)
{
	xge_shape_vertex_t* pXgeVertices;
	int iRet;
	int i;

	(void)iFlags;
	if ( (pProxy == NULL) || !__xuiProxyXgeDrawValid(pDraw) || (pVertices == NULL) ||
	     (iVertexCount <= 0) || (pIndices == NULL) || (iIndexCount <= 0) || ((iIndexCount % 3) != 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( iVertexCount > (INT32_MAX / (int)sizeof(*pXgeVertices)) ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	for ( i = 0; i < iVertexCount; i++ ) {
		if ( XGE_COLOR_GET_A(pVertices[i].iColor) != 0 ) {
			break;
		}
	}
	if ( i == iVertexCount ) {
		return XGE_OK;
	}
	pXgeVertices = (xge_shape_vertex_t*)xrtMalloc((size_t)iVertexCount * sizeof(*pXgeVertices));
	if ( pXgeVertices == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	for ( i = 0; i < iVertexCount; i++ ) {
		pXgeVertices[i].fX = pVertices[i].fX;
		pXgeVertices[i].fY = pVertices[i].fY;
		pXgeVertices[i].iColor = pVertices[i].iColor;
	}
	iRet = xgeShapeMeshFillPx(pXgeVertices, iVertexCount, pIndices, iIndexCount);
	xrtFree(pXgeVertices);
	if ( iRet == XGE_OK ) {
		__xuiProxyXgeDrawMarkDirty(pDraw);
	}
	return iRet;
}

static int __xuiProxyXgeDrawPoint(xui_proxy pProxy, xui_draw_context pDraw, float fX, float fY, float fSize, uint32_t iColor)
{
	if ( (pProxy == NULL) || !__xuiProxyXgeDrawValid(pDraw) || (fSize <= 0.0f) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( XGE_COLOR_GET_A(iColor) == 0 ) {
		return XGE_OK;
	}
	(void)pProxy;
	xgeShapePointPx(fX, fY, fSize, iColor);
	__xuiProxyXgeDrawMarkDirty(pDraw);
	return XGE_OK;
}

static int __xuiProxyXgeDrawLine(xui_proxy pProxy, xui_draw_context pDraw, float fX0, float fY0, float fX1, float fY1, float fWidth, uint32_t iColor)
{
	if ( (pProxy == NULL) || !__xuiProxyXgeDrawValid(pDraw) || (fWidth <= 0.0f) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( XGE_COLOR_GET_A(iColor) == 0 ) {
		return XGE_OK;
	}
	(void)pProxy;
	xgeShapeLinePx(fX0, fY0, fX1, fY1, fWidth, iColor);
	__xuiProxyXgeDrawMarkDirty(pDraw);
	return XGE_OK;
}

static int __xuiProxyXgeDrawTriangleFill(xui_proxy pProxy, xui_draw_context pDraw, xui_vec2_t tA, xui_vec2_t tB, xui_vec2_t tC, uint32_t iColor)
{
	if ( (pProxy == NULL) || !__xuiProxyXgeDrawValid(pDraw) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( XGE_COLOR_GET_A(iColor) == 0 ) {
		return XGE_OK;
	}
	(void)pProxy;
	xgeShapeTriangleFillPx(__xuiProxyXgeVec2(tA), __xuiProxyXgeVec2(tB), __xuiProxyXgeVec2(tC), iColor);
	__xuiProxyXgeDrawMarkDirty(pDraw);
	return XGE_OK;
}

static int __xuiProxyXgeDrawTriangleStroke(xui_proxy pProxy, xui_draw_context pDraw, xui_vec2_t tA, xui_vec2_t tB, xui_vec2_t tC, float fWidth, uint32_t iColor)
{
	if ( (pProxy == NULL) || !__xuiProxyXgeDrawValid(pDraw) || (fWidth <= 0.0f) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( XGE_COLOR_GET_A(iColor) == 0 ) {
		return XGE_OK;
	}
	(void)pProxy;
	xgeShapeLinePx(tA.fX, tA.fY, tB.fX, tB.fY, fWidth, iColor);
	xgeShapeLinePx(tB.fX, tB.fY, tC.fX, tC.fY, fWidth, iColor);
	xgeShapeLinePx(tC.fX, tC.fY, tA.fX, tA.fY, fWidth, iColor);
	__xuiProxyXgeDrawMarkDirty(pDraw);
	return XGE_OK;
}

static int __xuiProxyXgeDrawRectFill(xui_proxy pProxy, xui_draw_context pDraw, xui_rect_t tRect, uint32_t iColor)
{
	if ( (pProxy == NULL) || !__xuiProxyXgeDrawValid(pDraw) || (tRect.fW <= 0.0f) || (tRect.fH <= 0.0f) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( XGE_COLOR_GET_A(iColor) == 0 ) {
		return XGE_OK;
	}
	(void)pProxy;
	xgeShapeRectFillPx(__xuiProxyXgeRect(tRect), iColor);
	__xuiProxyXgeDrawMarkDirty(pDraw);
	return XGE_OK;
}

static int __xuiProxyXgeDrawRectStroke(xui_proxy pProxy, xui_draw_context pDraw, xui_rect_t tRect, float fWidth, uint32_t iColor)
{
	if ( (pProxy == NULL) || !__xuiProxyXgeDrawValid(pDraw) || (tRect.fW <= 0.0f) || (tRect.fH <= 0.0f) || (fWidth <= 0.0f) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( XGE_COLOR_GET_A(iColor) == 0 ) {
		return XGE_OK;
	}
	(void)pProxy;
	xgeShapeRectStrokePx(__xuiProxyXgeRect(tRect), fWidth, iColor);
	__xuiProxyXgeDrawMarkDirty(pDraw);
	return XGE_OK;
}

static int __xuiProxyXgeDrawCircleFill(xui_proxy pProxy, xui_draw_context pDraw, float fX, float fY, float fRadius, uint32_t iColor)
{
	if ( (pProxy == NULL) || !__xuiProxyXgeDrawValid(pDraw) || (fRadius <= 0.0f) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( XGE_COLOR_GET_A(iColor) == 0 ) {
		return XGE_OK;
	}
	(void)pProxy;
	xgeShapeCircleFillPx(fX, fY, fRadius, iColor);
	__xuiProxyXgeDrawMarkDirty(pDraw);
	return XGE_OK;
}

static int __xuiProxyXgeDrawCircleStroke(xui_proxy pProxy, xui_draw_context pDraw, float fX, float fY, float fRadius, float fWidth, uint32_t iColor)
{
	if ( (pProxy == NULL) || !__xuiProxyXgeDrawValid(pDraw) || (fRadius <= 0.0f) || (fWidth <= 0.0f) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( XGE_COLOR_GET_A(iColor) == 0 ) {
		return XGE_OK;
	}
	(void)pProxy;
	xgeShapeCircleStrokePx(fX, fY, fRadius, fWidth, iColor);
	__xuiProxyXgeDrawMarkDirty(pDraw);
	return XGE_OK;
}

static int __xuiProxyXgeDrawRoundRectFill(xui_proxy pProxy, xui_draw_context pDraw, xui_rect_t tRect, float fRadius, uint32_t iColor)
{
	if ( (pProxy == NULL) || !__xuiProxyXgeDrawValid(pDraw) || (tRect.fW <= 0.0f) || (tRect.fH <= 0.0f) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( XGE_COLOR_GET_A(iColor) == 0 ) {
		return XGE_OK;
	}
	(void)pProxy;
	__xuiProxyXgeShapeRoundRectFillLocal(tRect, fRadius, iColor);
	__xuiProxyXgeDrawMarkDirty(pDraw);
	return XGE_OK;
}

static int __xuiProxyXgeDrawRoundRectStroke(xui_proxy pProxy, xui_draw_context pDraw, xui_rect_t tRect, float fRadius, float fWidth, uint32_t iColor)
{
	if ( (pProxy == NULL) || !__xuiProxyXgeDrawValid(pDraw) || (tRect.fW <= 0.0f) || (tRect.fH <= 0.0f) || (fWidth <= 0.0f) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( XGE_COLOR_GET_A(iColor) == 0 ) {
		return XGE_OK;
	}
	(void)pProxy;
	__xuiProxyXgeShapeRoundRectStrokeLocal(tRect, fRadius, fWidth, iColor);
	__xuiProxyXgeDrawMarkDirty(pDraw);
	return XGE_OK;
}

static int __xuiProxyXgeDrawText(xui_proxy pProxy, xui_draw_context pDraw, xui_font pFont, const char* sText, xui_rect_t tRect, uint32_t iColor, uint32_t iFlags)
{
	if ( (pProxy == NULL) || !__xuiProxyXgeDrawValid(pDraw) || !__xuiProxyXgeFontValid(pFont) || (sText == NULL) || (tRect.fW <= 0.0f) || (tRect.fH <= 0.0f) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( XGE_COLOR_GET_A(iColor) == 0 ) {
		return XGE_OK;
	}
	(void)pProxy;
	xgeTextDrawRect(&pFont->tFont, sText, __xuiProxyXgeRect(tRect), iColor, __xuiProxyXgeTextFlags(iFlags));
	__xuiProxyXgeDrawMarkDirty(pDraw);
	return XGE_OK;
}

static int __xuiProxyXgeDrawClipGet(xui_proxy pProxy, xui_draw_context pDraw, xui_rect_t* pRect, int* pHasClip)
{
	xge_rect_t tClip;

	if ( (pProxy == NULL) || !__xuiProxyXgeDrawValid(pDraw) || (pRect == NULL) || (pHasClip == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	(void)pProxy;
	tClip = xgeClipGet();
	*pHasClip = ((tClip.fW > 0.0f) && (tClip.fH > 0.0f)) ? 1 : 0;
	pRect->fX = tClip.fX;
	pRect->fY = tClip.fY;
	pRect->fW = tClip.fW;
	pRect->fH = tClip.fH;
	return XGE_OK;
}

static int __xuiProxyXgeDrawClipSet(xui_proxy pProxy, xui_draw_context pDraw, xui_rect_t tRect)
{
	if ( (pProxy == NULL) || !__xuiProxyXgeDrawValid(pDraw) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	(void)pProxy;
	(void)xgeFlush();
	xgeClipSet(__xuiProxyXgeRect(xuiInternalSnapRect(tRect)));
	return XGE_OK;
}

static int __xuiProxyXgeDrawClipClear(xui_proxy pProxy, xui_draw_context pDraw)
{
	if ( (pProxy == NULL) || !__xuiProxyXgeDrawValid(pDraw) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	(void)pProxy;
	(void)xgeFlush();
	xgeClipClear();
	return XGE_OK;
}

static void __xuiProxyXgeSurfaceDestroy(xui_proxy pProxy, xui_surface pSurface)
{
	if ( pProxy == NULL ) {
		return;
	}
	(void)pProxy;
	if ( !__xuiProxyXgeSurfaceValid(pSurface) ) {
		return;
	}
	if ( (pSurface->iFlags & XUI_SURFACE_USAGE_TARGET) != 0 ) {
		xgeRenderTargetFree(&pSurface->tTarget);
	} else {
		xgeTextureFree(&pSurface->tTexture);
	}
	pSurface->iMagic = 0;
	xrtFree(pSurface);
}

XUI_API xui_proxy_t xuiProxyXge(void)
{
	xui_proxy_t tProxy;

	memset(&tProxy, 0, sizeof(tProxy));
	tProxy.iSize = (uint32_t)sizeof(tProxy);
	tProxy.iVersion = XUI_PROXY_VERSION;
	tProxy.getCaps = __xuiProxyXgeGetCaps;
	tProxy.clipboardSetText = __xuiProxyXgeClipboardSetText;
	tProxy.clipboardGetText = __xuiProxyXgeClipboardGetText;
	tProxy.imeGetEnabled = __xuiProxyXgeImeGetEnabled;
	tProxy.imeSetEnabled = __xuiProxyXgeImeSetEnabled;
	tProxy.imeSetCandidateRect = __xuiProxyXgeImeSetCandidateRect;
	tProxy.surfaceCreate = __xuiProxyXgeSurfaceCreate;
	tProxy.surfaceCreateRGBA = __xuiProxyXgeSurfaceCreateRGBA;
	tProxy.surfaceLoadFile = __xuiProxyXgeSurfaceLoadFile;
	tProxy.surfaceLoadMemory = __xuiProxyXgeSurfaceLoadMemory;
	tProxy.surfaceUpdateRGBA = __xuiProxyXgeSurfaceUpdateRGBA;
	tProxy.surfaceReadRGBA = __xuiProxyXgeSurfaceReadRGBA;
	tProxy.surfaceGetDesc = __xuiProxyXgeSurfaceGetDesc;
	tProxy.surfaceDraw = __xuiProxyXgeSurfaceDraw;
	tProxy.surfaceClear = __xuiProxyXgeSurfaceClear;
	tProxy.surfaceClearRect = __xuiProxyXgeSurfaceClearRect;
	tProxy.surfaceDrawTo = __xuiProxyXgeSurfaceDrawTo;
	tProxy.surfaceDrawQuad = __xuiProxyXgeSurfaceDrawQuad;
	tProxy.surfaceDrawQuadTo = __xuiProxyXgeSurfaceDrawQuadTo;
	tProxy.surfaceGetSampler = __xuiProxyXgeSurfaceGetSampler;
	tProxy.surfaceSetSampler = __xuiProxyXgeSurfaceSetSampler;
	tProxy.surfaceGetGeneration = __xuiProxyXgeSurfaceGetGeneration;
	tProxy.surfaceDestroy = __xuiProxyXgeSurfaceDestroy;
	tProxy.shapePoint = __xuiProxyXgeShapePoint;
	tProxy.shapeLine = __xuiProxyXgeShapeLine;
	tProxy.shapeTriangleFill = __xuiProxyXgeShapeTriangleFill;
	tProxy.shapeTriangleStroke = __xuiProxyXgeShapeTriangleStroke;
	tProxy.shapeRectFill = __xuiProxyXgeShapeRectFill;
	tProxy.shapeRectStroke = __xuiProxyXgeShapeRectStroke;
	tProxy.shapeCircleFill = __xuiProxyXgeShapeCircleFill;
	tProxy.shapeCircleStroke = __xuiProxyXgeShapeCircleStroke;
	tProxy.shapeRoundRectFill = __xuiProxyXgeShapeRoundRectFill;
	tProxy.shapeRoundRectStroke = __xuiProxyXgeShapeRoundRectStroke;
	tProxy.fontLoadFile = __xuiProxyXgeFontLoadFile;
	tProxy.fontLoadMemory = __xuiProxyXgeFontLoadMemory;
	tProxy.fontGetMetrics = __xuiProxyXgeFontGetMetrics;
	tProxy.fontDestroy = __xuiProxyXgeFontDestroy;
	tProxy.textMeasure = __xuiProxyXgeTextMeasure;
	tProxy.textDraw = __xuiProxyXgeTextDraw;
	tProxy.drawBegin = __xuiProxyXgeDrawBegin;
	tProxy.drawEnd = __xuiProxyXgeDrawEnd;
	tProxy.drawClearRect = __xuiProxyXgeDrawClearRect;
	tProxy.drawSurface = __xuiProxyXgeDrawSurface;
	tProxy.drawSurfaceQuad = __xuiProxyXgeDrawSurfaceQuad;
	tProxy.drawMeshTriangles = __xuiProxyXgeDrawMeshTriangles;
	tProxy.drawPoint = __xuiProxyXgeDrawPoint;
	tProxy.drawLine = __xuiProxyXgeDrawLine;
	tProxy.drawTriangleFill = __xuiProxyXgeDrawTriangleFill;
	tProxy.drawTriangleStroke = __xuiProxyXgeDrawTriangleStroke;
	tProxy.drawRectFill = __xuiProxyXgeDrawRectFill;
	tProxy.drawRectStroke = __xuiProxyXgeDrawRectStroke;
	tProxy.drawCircleFill = __xuiProxyXgeDrawCircleFill;
	tProxy.drawCircleStroke = __xuiProxyXgeDrawCircleStroke;
	tProxy.drawRoundRectFill = __xuiProxyXgeDrawRoundRectFill;
	tProxy.drawRoundRectStroke = __xuiProxyXgeDrawRoundRectStroke;
	tProxy.drawText = __xuiProxyXgeDrawText;
	tProxy.drawClipGet = __xuiProxyXgeDrawClipGet;
	tProxy.drawClipSet = __xuiProxyXgeDrawClipSet;
	tProxy.drawClipClear = __xuiProxyXgeDrawClipClear;
	return tProxy;
}
