#include "xui.h"

#include <stdio.h>
#include <string.h>

#define XUI_TEST_CHECK(expr, msg) \
	do { \
		if ( !(expr) ) { \
			printf("xui_label_test failed: %s\n", (msg)); \
			iFailed = 1; \
			goto cleanup; \
		} \
	} while ( 0 )

#define XUI_TEST_SURFACE_MAGIC 0x4c535552u
#define XUI_TEST_DRAW_MAGIC 0x4c445257u
#define XUI_TEST_FONT_MAGIC 0x4c464f4eu

struct xui_surface_t {
	uint32_t iMagic;
	xui_surface_desc_t tDesc;
	uint32_t iGeneration;
};

struct xui_draw_context_t {
	uint32_t iMagic;
	xui_surface pTarget;
};

struct xui_font_t {
	uint32_t iMagic;
	float fSize;
};

typedef struct xui_label_test_state_t {
	xui_proxy_t tProxy;
	int iDrawTextCount;
	int iTextDrawCount;
	int iCommandCount;
	uint32_t iLastColor;
	uint32_t iLastFlags;
	xui_rect_t tLastRect;
	char sLastText[128];
} xui_label_test_state_t;

static xui_label_test_state_t* __xuiLabelTestState(xui_proxy pProxy)
{
	return (pProxy != NULL) ? (xui_label_test_state_t*)pProxy->pUser : NULL;
}

static int __xuiLabelTestSurfaceValid(xui_surface pSurface)
{
	return (pSurface != NULL) && (pSurface->iMagic == XUI_TEST_SURFACE_MAGIC);
}

static int __xuiLabelTestDrawValid(xui_draw_context pDraw)
{
	return (pDraw != NULL) && (pDraw->iMagic == XUI_TEST_DRAW_MAGIC) && __xuiLabelTestSurfaceValid(pDraw->pTarget);
}

static int __xuiLabelTestFontValid(xui_font pFont)
{
	return (pFont != NULL) && (pFont->iMagic == XUI_TEST_FONT_MAGIC);
}

static xui_style_value_t __xuiLabelTestColor(uint32_t iColor)
{
	xui_style_value_t tValue;

	memset(&tValue, 0, sizeof(tValue));
	tValue.iSize = sizeof(tValue);
	tValue.iType = XUI_STYLE_VALUE_COLOR;
	tValue.iColor = iColor;
	return tValue;
}

static xui_style_value_t __xuiLabelTestInt(int iValue)
{
	xui_style_value_t tValue;

	memset(&tValue, 0, sizeof(tValue));
	tValue.iSize = sizeof(tValue);
	tValue.iType = XUI_STYLE_VALUE_INT;
	tValue.iInt = iValue;
	return tValue;
}

static xui_style_value_t __xuiLabelTestBool(int bValue)
{
	xui_style_value_t tValue;

	memset(&tValue, 0, sizeof(tValue));
	tValue.iSize = sizeof(tValue);
	tValue.iType = XUI_STYLE_VALUE_BOOL;
	tValue.iInt = bValue ? 1 : 0;
	return tValue;
}

static xui_style_value_t __xuiLabelTestFloat(float fValue)
{
	xui_style_value_t tValue;

	memset(&tValue, 0, sizeof(tValue));
	tValue.iSize = sizeof(tValue);
	tValue.iType = XUI_STYLE_VALUE_FLOAT;
	tValue.fFloat = fValue;
	return tValue;
}

static xui_style_value_t __xuiLabelTestString(const char* sText)
{
	xui_style_value_t tValue;

	memset(&tValue, 0, sizeof(tValue));
	tValue.iSize = sizeof(tValue);
	tValue.iType = XUI_STYLE_VALUE_STRING;
	tValue.sText = sText;
	return tValue;
}

static xui_style_value_t __xuiLabelTestToken(const char* sText)
{
	xui_style_value_t tValue;

	memset(&tValue, 0, sizeof(tValue));
	tValue.iSize = sizeof(tValue);
	tValue.iType = XUI_STYLE_VALUE_TOKEN;
	tValue.sText = sText;
	return tValue;
}

static xui_style_property_t __xuiLabelTestProp(const char* sName, xui_style_value_t tValue)
{
	xui_style_property_t tProp;

	memset(&tProp, 0, sizeof(tProp));
	tProp.iSize = sizeof(tProp);
	tProp.sName = sName;
	tProp.tValue = tValue;
	return tProp;
}

static xui_style_desc_t __xuiLabelTestStyle(const xui_style_property_t* pProps, int iCount)
{
	xui_style_desc_t tStyle;

	memset(&tStyle, 0, sizeof(tStyle));
	tStyle.iSize = sizeof(tStyle);
	tStyle.pProperties = pProps;
	tStyle.iPropertyCount = iCount;
	return tStyle;
}

static int __xuiLabelTestGetCaps(xui_proxy pProxy, xui_proxy_caps_t* pCaps)
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

static int __xuiLabelTestClipboardSetText(xui_proxy pProxy, const char* sText)
{
	(void)pProxy;
	(void)sText;
	return XUI_OK;
}

static int __xuiLabelTestClipboardGetText(xui_proxy pProxy, char* sText, int iCapacity)
{
	(void)pProxy;
	if ( (sText != NULL) && (iCapacity > 0) ) {
		sText[0] = '\0';
	}
	return 0;
}

static int __xuiLabelTestImeGetEnabled(xui_proxy pProxy)
{
	(void)pProxy;
	return 0;
}

static int __xuiLabelTestImeSetEnabled(xui_proxy pProxy, int bEnabled)
{
	(void)pProxy;
	(void)bEnabled;
	return XUI_OK;
}

static int __xuiLabelTestImeSetCandidateRect(xui_proxy pProxy, xui_rect_t tRect)
{
	(void)pProxy;
	(void)tRect;
	return XUI_OK;
}

static int __xuiLabelTestSurfaceCreate(xui_proxy pProxy, xui_surface* ppSurface, const xui_surface_desc_t* pDesc)
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

static int __xuiLabelTestSurfaceCreateRGBA(xui_proxy pProxy, xui_surface* ppSurface, int iWidth, int iHeight, const void* pPixels, int iStride, uint32_t iFlags)
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
	return __xuiLabelTestSurfaceCreate(pProxy, ppSurface, &tDesc);
}

static int __xuiLabelTestSurfaceLoadFile(xui_proxy pProxy, xui_surface* ppSurface, const char* sPath, uint32_t iFlags)
{
	(void)pProxy;
	(void)ppSurface;
	(void)sPath;
	(void)iFlags;
	return XUI_ERROR_UNSUPPORTED;
}

static int __xuiLabelTestSurfaceLoadMemory(xui_proxy pProxy, xui_surface* ppSurface, const void* pData, int iSize, uint32_t iFlags)
{
	(void)pProxy;
	(void)ppSurface;
	(void)pData;
	(void)iSize;
	(void)iFlags;
	return XUI_ERROR_UNSUPPORTED;
}

static int __xuiLabelTestSurfaceUpdateRGBA(xui_proxy pProxy, xui_surface pSurface, xui_rect_i_t tRect, const void* pPixels, int iStride)
{
	(void)pProxy;
	(void)tRect;
	(void)pPixels;
	(void)iStride;
	if ( !__xuiLabelTestSurfaceValid(pSurface) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pSurface->iGeneration++;
	return XUI_OK;
}

static int __xuiLabelTestSurfaceReadRGBA(xui_proxy pProxy, xui_surface pSurface, void* pPixels, int iStride)
{
	(void)pProxy;
	if ( !__xuiLabelTestSurfaceValid(pSurface) || (pPixels == NULL) || (iStride <= 0) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	return XUI_OK;
}

static int __xuiLabelTestSurfaceGetDesc(xui_proxy pProxy, xui_surface pSurface, xui_surface_desc_t* pDesc)
{
	(void)pProxy;
	if ( !__xuiLabelTestSurfaceValid(pSurface) || (pDesc == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	*pDesc = pSurface->tDesc;
	return XUI_OK;
}

static int __xuiLabelTestSurfaceDraw(xui_proxy pProxy, xui_surface pSurface, xui_rect_t tSrc, xui_rect_t tDst, uint32_t iColor, uint32_t iFlags)
{
	(void)pProxy;
	(void)tSrc;
	(void)tDst;
	(void)iColor;
	(void)iFlags;
	return __xuiLabelTestSurfaceValid(pSurface) ? XUI_OK : XUI_ERROR_INVALID_ARGUMENT;
}

static int __xuiLabelTestSurfaceClear(xui_proxy pProxy, xui_surface pTarget, uint32_t iColor)
{
	(void)pProxy;
	(void)iColor;
	if ( !__xuiLabelTestSurfaceValid(pTarget) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pTarget->iGeneration++;
	return XUI_OK;
}

static int __xuiLabelTestSurfaceClearRect(xui_proxy pProxy, xui_surface pTarget, xui_rect_t tRect, uint32_t iColor)
{
	(void)tRect;
	return __xuiLabelTestSurfaceClear(pProxy, pTarget, iColor);
}

static int __xuiLabelTestSurfaceDrawTo(xui_proxy pProxy, xui_surface pTarget, xui_surface pSurface, xui_rect_t tSrc, xui_rect_t tDst, uint32_t iColor, uint32_t iFlags)
{
	(void)pProxy;
	(void)tSrc;
	(void)tDst;
	(void)iColor;
	(void)iFlags;
	if ( !__xuiLabelTestSurfaceValid(pTarget) || !__xuiLabelTestSurfaceValid(pSurface) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pTarget->iGeneration++;
	return XUI_OK;
}

static int __xuiLabelTestSurfaceDrawQuad(xui_proxy pProxy, xui_surface pSurface, const xui_surface_vertex_t* pVertices, uint32_t iFlags)
{
	(void)pProxy;
	(void)pVertices;
	(void)iFlags;
	return __xuiLabelTestSurfaceValid(pSurface) ? XUI_OK : XUI_ERROR_INVALID_ARGUMENT;
}

static int __xuiLabelTestSurfaceDrawQuadTo(xui_proxy pProxy, xui_surface pTarget, xui_surface pSurface, const xui_surface_vertex_t* pVertices, uint32_t iFlags)
{
	(void)pProxy;
	(void)pVertices;
	(void)iFlags;
	if ( !__xuiLabelTestSurfaceValid(pTarget) || !__xuiLabelTestSurfaceValid(pSurface) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pTarget->iGeneration++;
	return XUI_OK;
}

static int __xuiLabelTestSurfaceGetSampler(xui_proxy pProxy, xui_surface pSurface, xui_surface_sampler_t* pSampler)
{
	(void)pProxy;
	if ( !__xuiLabelTestSurfaceValid(pSurface) || (pSampler == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pSampler->iMinFilter = XUI_SURFACE_FILTER_NEAREST;
	pSampler->iMagFilter = XUI_SURFACE_FILTER_NEAREST;
	pSampler->iWrapS = XUI_SURFACE_WRAP_CLAMP;
	pSampler->iWrapT = XUI_SURFACE_WRAP_CLAMP;
	return XUI_OK;
}

static int __xuiLabelTestSurfaceSetSampler(xui_proxy pProxy, xui_surface pSurface, const xui_surface_sampler_t* pSampler)
{
	(void)pProxy;
	(void)pSampler;
	return __xuiLabelTestSurfaceValid(pSurface) ? XUI_OK : XUI_ERROR_INVALID_ARGUMENT;
}

static int __xuiLabelTestSurfaceGetGeneration(xui_proxy pProxy, xui_surface pSurface, uint32_t* pGeneration)
{
	(void)pProxy;
	if ( !__xuiLabelTestSurfaceValid(pSurface) || (pGeneration == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	*pGeneration = pSurface->iGeneration;
	return XUI_OK;
}

static void __xuiLabelTestSurfaceDestroy(xui_proxy pProxy, xui_surface pSurface)
{
	(void)pProxy;
	if ( !__xuiLabelTestSurfaceValid(pSurface) ) {
		return;
	}
	pSurface->iMagic = 0;
	xrtFree(pSurface);
}

static int __xuiLabelTestShapeOk(xui_proxy pProxy, xui_surface pTarget)
{
	(void)pProxy;
	return __xuiLabelTestSurfaceValid(pTarget) ? XUI_OK : XUI_ERROR_INVALID_ARGUMENT;
}

static int __xuiLabelTestShapePoint(xui_proxy pProxy, xui_surface pTarget, float fX, float fY, float fSize, uint32_t iColor)
{
	(void)fX;
	(void)fY;
	(void)fSize;
	(void)iColor;
	return __xuiLabelTestShapeOk(pProxy, pTarget);
}

static int __xuiLabelTestShapeLine(xui_proxy pProxy, xui_surface pTarget, float fX0, float fY0, float fX1, float fY1, float fWidth, uint32_t iColor)
{
	(void)fX0;
	(void)fY0;
	(void)fX1;
	(void)fY1;
	(void)fWidth;
	(void)iColor;
	return __xuiLabelTestShapeOk(pProxy, pTarget);
}

static int __xuiLabelTestShapeTriangleFill(xui_proxy pProxy, xui_surface pTarget, xui_vec2_t tA, xui_vec2_t tB, xui_vec2_t tC, uint32_t iColor)
{
	(void)tA;
	(void)tB;
	(void)tC;
	(void)iColor;
	return __xuiLabelTestShapeOk(pProxy, pTarget);
}

static int __xuiLabelTestShapeTriangleStroke(xui_proxy pProxy, xui_surface pTarget, xui_vec2_t tA, xui_vec2_t tB, xui_vec2_t tC, float fWidth, uint32_t iColor)
{
	(void)tA;
	(void)tB;
	(void)tC;
	(void)fWidth;
	(void)iColor;
	return __xuiLabelTestShapeOk(pProxy, pTarget);
}

static int __xuiLabelTestShapeRectFill(xui_proxy pProxy, xui_surface pTarget, xui_rect_t tRect, uint32_t iColor)
{
	(void)tRect;
	(void)iColor;
	return __xuiLabelTestShapeOk(pProxy, pTarget);
}

static int __xuiLabelTestShapeRectStroke(xui_proxy pProxy, xui_surface pTarget, xui_rect_t tRect, float fWidth, uint32_t iColor)
{
	(void)tRect;
	(void)fWidth;
	(void)iColor;
	return __xuiLabelTestShapeOk(pProxy, pTarget);
}

static int __xuiLabelTestShapeCircleFill(xui_proxy pProxy, xui_surface pTarget, float fX, float fY, float fRadius, uint32_t iColor)
{
	(void)fX;
	(void)fY;
	(void)fRadius;
	(void)iColor;
	return __xuiLabelTestShapeOk(pProxy, pTarget);
}

static int __xuiLabelTestShapeCircleStroke(xui_proxy pProxy, xui_surface pTarget, float fX, float fY, float fRadius, float fWidth, uint32_t iColor)
{
	(void)fX;
	(void)fY;
	(void)fRadius;
	(void)fWidth;
	(void)iColor;
	return __xuiLabelTestShapeOk(pProxy, pTarget);
}

static int __xuiLabelTestShapeRoundRectFill(xui_proxy pProxy, xui_surface pTarget, xui_rect_t tRect, float fRadius, uint32_t iColor)
{
	(void)tRect;
	(void)fRadius;
	(void)iColor;
	return __xuiLabelTestShapeOk(pProxy, pTarget);
}

static int __xuiLabelTestShapeRoundRectStroke(xui_proxy pProxy, xui_surface pTarget, xui_rect_t tRect, float fRadius, float fWidth, uint32_t iColor)
{
	(void)tRect;
	(void)fRadius;
	(void)fWidth;
	(void)iColor;
	return __xuiLabelTestShapeOk(pProxy, pTarget);
}

static int __xuiLabelTestFontCreate(xui_font* ppFont, float fSize)
{
	xui_font pFont;

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

static int __xuiLabelTestFontLoadFile(xui_proxy pProxy, xui_font* ppFont, const char* sPath, float fSize, uint32_t iFlags)
{
	(void)pProxy;
	(void)sPath;
	(void)iFlags;
	return __xuiLabelTestFontCreate(ppFont, fSize);
}

static int __xuiLabelTestFontLoadMemory(xui_proxy pProxy, xui_font* ppFont, const void* pData, int iSize, float fSize, uint32_t iFlags)
{
	(void)pProxy;
	(void)pData;
	(void)iSize;
	(void)iFlags;
	return __xuiLabelTestFontCreate(ppFont, fSize);
}

static int __xuiLabelTestFontGetMetrics(xui_proxy pProxy, xui_font pFont, xui_font_metrics_t* pMetrics)
{
	(void)pProxy;
	if ( !__xuiLabelTestFontValid(pFont) || (pMetrics == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	memset(pMetrics, 0, sizeof(*pMetrics));
	pMetrics->fSize = pFont->fSize;
	pMetrics->fAscent = pFont->fSize * 0.8f;
	pMetrics->fDescent = pFont->fSize * 0.2f;
	pMetrics->fLineGap = 0.0f;
	pMetrics->fLineHeight = pFont->fSize;
	return XUI_OK;
}

static void __xuiLabelTestFontDestroy(xui_proxy pProxy, xui_font pFont)
{
	(void)pProxy;
	if ( !__xuiLabelTestFontValid(pFont) ) {
		return;
	}
	pFont->iMagic = 0;
	xrtFree(pFont);
}

static int __xuiLabelTestTextMeasure(xui_proxy pProxy, xui_font pFont, const char* sText, xui_vec2_t* pSize)
{
	size_t iLen;

	(void)pProxy;
	if ( !__xuiLabelTestFontValid(pFont) || (pSize == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( sText == NULL ) {
		sText = "";
	}
	iLen = strlen(sText);
	pSize->fX = (float)iLen * pFont->fSize * 0.5f;
	pSize->fY = pFont->fSize;
	return XUI_OK;
}

static int __xuiLabelTestTextDraw(xui_proxy pProxy, xui_surface pTarget, xui_font pFont, const char* sText, xui_rect_t tRect, uint32_t iColor, uint32_t iFlags)
{
	xui_label_test_state_t* pState;

	(void)pTarget;
	(void)pFont;
	(void)sText;
	(void)tRect;
	(void)iColor;
	(void)iFlags;
	pState = __xuiLabelTestState(pProxy);
	if ( pState != NULL ) {
		pState->iTextDrawCount++;
	}
	return XUI_OK;
}

static int __xuiLabelTestDrawBegin(xui_proxy pProxy, xui_draw_context* ppDraw, xui_surface pTarget)
{
	xui_draw_context pDraw;

	(void)pProxy;
	if ( (ppDraw == NULL) || !__xuiLabelTestSurfaceValid(pTarget) ) {
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

static int __xuiLabelTestDrawEnd(xui_proxy pProxy, xui_draw_context pDraw)
{
	(void)pProxy;
	if ( !__xuiLabelTestDrawValid(pDraw) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pDraw->iMagic = 0;
	xrtFree(pDraw);
	return XUI_OK;
}

static int __xuiLabelTestDrawClearRect(xui_proxy pProxy, xui_draw_context pDraw, xui_rect_t tRect, uint32_t iColor)
{
	(void)pProxy;
	(void)tRect;
	(void)iColor;
	return __xuiLabelTestDrawValid(pDraw) ? XUI_OK : XUI_ERROR_INVALID_ARGUMENT;
}

static int __xuiLabelTestDrawSurface(xui_proxy pProxy, xui_draw_context pDraw, xui_surface pSurface, xui_rect_t tSrc, xui_rect_t tDst, uint32_t iColor, uint32_t iFlags)
{
	(void)pProxy;
	(void)tSrc;
	(void)tDst;
	(void)iColor;
	(void)iFlags;
	return (__xuiLabelTestDrawValid(pDraw) && __xuiLabelTestSurfaceValid(pSurface)) ? XUI_OK : XUI_ERROR_INVALID_ARGUMENT;
}

static int __xuiLabelTestDrawSurfaceQuad(xui_proxy pProxy, xui_draw_context pDraw, xui_surface pSurface, const xui_surface_vertex_t* pVertices, uint32_t iFlags)
{
	(void)pProxy;
	(void)pVertices;
	(void)iFlags;
	return (__xuiLabelTestDrawValid(pDraw) && __xuiLabelTestSurfaceValid(pSurface)) ? XUI_OK : XUI_ERROR_INVALID_ARGUMENT;
}

static int __xuiLabelTestDrawPoint(xui_proxy pProxy, xui_draw_context pDraw, float fX, float fY, float fSize, uint32_t iColor)
{
	(void)pProxy;
	(void)fX;
	(void)fY;
	(void)fSize;
	(void)iColor;
	return __xuiLabelTestDrawValid(pDraw) ? XUI_OK : XUI_ERROR_INVALID_ARGUMENT;
}

static int __xuiLabelTestDrawLine(xui_proxy pProxy, xui_draw_context pDraw, float fX0, float fY0, float fX1, float fY1, float fWidth, uint32_t iColor)
{
	(void)pProxy;
	(void)fX0;
	(void)fY0;
	(void)fX1;
	(void)fY1;
	(void)fWidth;
	(void)iColor;
	return __xuiLabelTestDrawValid(pDraw) ? XUI_OK : XUI_ERROR_INVALID_ARGUMENT;
}

static int __xuiLabelTestDrawTriangleFill(xui_proxy pProxy, xui_draw_context pDraw, xui_vec2_t tA, xui_vec2_t tB, xui_vec2_t tC, uint32_t iColor)
{
	(void)pProxy;
	(void)tA;
	(void)tB;
	(void)tC;
	(void)iColor;
	return __xuiLabelTestDrawValid(pDraw) ? XUI_OK : XUI_ERROR_INVALID_ARGUMENT;
}

static int __xuiLabelTestDrawTriangleStroke(xui_proxy pProxy, xui_draw_context pDraw, xui_vec2_t tA, xui_vec2_t tB, xui_vec2_t tC, float fWidth, uint32_t iColor)
{
	(void)pProxy;
	(void)tA;
	(void)tB;
	(void)tC;
	(void)fWidth;
	(void)iColor;
	return __xuiLabelTestDrawValid(pDraw) ? XUI_OK : XUI_ERROR_INVALID_ARGUMENT;
}

static int __xuiLabelTestDrawRectFill(xui_proxy pProxy, xui_draw_context pDraw, xui_rect_t tRect, uint32_t iColor)
{
	(void)pProxy;
	(void)tRect;
	(void)iColor;
	return __xuiLabelTestDrawValid(pDraw) ? XUI_OK : XUI_ERROR_INVALID_ARGUMENT;
}

static int __xuiLabelTestDrawRectStroke(xui_proxy pProxy, xui_draw_context pDraw, xui_rect_t tRect, float fWidth, uint32_t iColor)
{
	(void)pProxy;
	(void)tRect;
	(void)fWidth;
	(void)iColor;
	return __xuiLabelTestDrawValid(pDraw) ? XUI_OK : XUI_ERROR_INVALID_ARGUMENT;
}

static int __xuiLabelTestDrawCircleFill(xui_proxy pProxy, xui_draw_context pDraw, float fX, float fY, float fRadius, uint32_t iColor)
{
	(void)pProxy;
	(void)fX;
	(void)fY;
	(void)fRadius;
	(void)iColor;
	return __xuiLabelTestDrawValid(pDraw) ? XUI_OK : XUI_ERROR_INVALID_ARGUMENT;
}

static int __xuiLabelTestDrawCircleStroke(xui_proxy pProxy, xui_draw_context pDraw, float fX, float fY, float fRadius, float fWidth, uint32_t iColor)
{
	(void)pProxy;
	(void)fX;
	(void)fY;
	(void)fRadius;
	(void)fWidth;
	(void)iColor;
	return __xuiLabelTestDrawValid(pDraw) ? XUI_OK : XUI_ERROR_INVALID_ARGUMENT;
}

static int __xuiLabelTestDrawRoundRectFill(xui_proxy pProxy, xui_draw_context pDraw, xui_rect_t tRect, float fRadius, uint32_t iColor)
{
	(void)pProxy;
	(void)tRect;
	(void)fRadius;
	(void)iColor;
	return __xuiLabelTestDrawValid(pDraw) ? XUI_OK : XUI_ERROR_INVALID_ARGUMENT;
}

static int __xuiLabelTestDrawRoundRectStroke(xui_proxy pProxy, xui_draw_context pDraw, xui_rect_t tRect, float fRadius, float fWidth, uint32_t iColor)
{
	(void)pProxy;
	(void)tRect;
	(void)fRadius;
	(void)fWidth;
	(void)iColor;
	return __xuiLabelTestDrawValid(pDraw) ? XUI_OK : XUI_ERROR_INVALID_ARGUMENT;
}

static int __xuiLabelTestDrawText(xui_proxy pProxy, xui_draw_context pDraw, xui_font pFont, const char* sText, xui_rect_t tRect, uint32_t iColor, uint32_t iFlags)
{
	xui_label_test_state_t* pState;

	if ( !__xuiLabelTestDrawValid(pDraw) || !__xuiLabelTestFontValid(pFont) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pState = __xuiLabelTestState(pProxy);
	if ( pState != NULL ) {
		pState->iDrawTextCount++;
		pState->iLastColor = iColor;
		pState->iLastFlags = iFlags;
		pState->tLastRect = tRect;
		snprintf(pState->sLastText, sizeof(pState->sLastText), "%s", (sText != NULL) ? sText : "");
	}
	return XUI_OK;
}

static xui_proxy_t __xuiLabelTestProxy(xui_label_test_state_t* pState)
{
	xui_proxy_t tProxy;

	memset(&tProxy, 0, sizeof(tProxy));
	tProxy.iSize = sizeof(tProxy);
	tProxy.iVersion = XUI_PROXY_VERSION;
	tProxy.pUser = pState;
	tProxy.getCaps = __xuiLabelTestGetCaps;
	tProxy.clipboardSetText = __xuiLabelTestClipboardSetText;
	tProxy.clipboardGetText = __xuiLabelTestClipboardGetText;
	tProxy.imeGetEnabled = __xuiLabelTestImeGetEnabled;
	tProxy.imeSetEnabled = __xuiLabelTestImeSetEnabled;
	tProxy.imeSetCandidateRect = __xuiLabelTestImeSetCandidateRect;
	tProxy.surfaceCreate = __xuiLabelTestSurfaceCreate;
	tProxy.surfaceCreateRGBA = __xuiLabelTestSurfaceCreateRGBA;
	tProxy.surfaceLoadFile = __xuiLabelTestSurfaceLoadFile;
	tProxy.surfaceLoadMemory = __xuiLabelTestSurfaceLoadMemory;
	tProxy.surfaceUpdateRGBA = __xuiLabelTestSurfaceUpdateRGBA;
	tProxy.surfaceReadRGBA = __xuiLabelTestSurfaceReadRGBA;
	tProxy.surfaceGetDesc = __xuiLabelTestSurfaceGetDesc;
	tProxy.surfaceDraw = __xuiLabelTestSurfaceDraw;
	tProxy.surfaceClear = __xuiLabelTestSurfaceClear;
	tProxy.surfaceClearRect = __xuiLabelTestSurfaceClearRect;
	tProxy.surfaceDrawTo = __xuiLabelTestSurfaceDrawTo;
	tProxy.surfaceDrawQuad = __xuiLabelTestSurfaceDrawQuad;
	tProxy.surfaceDrawQuadTo = __xuiLabelTestSurfaceDrawQuadTo;
	tProxy.surfaceGetSampler = __xuiLabelTestSurfaceGetSampler;
	tProxy.surfaceSetSampler = __xuiLabelTestSurfaceSetSampler;
	tProxy.surfaceGetGeneration = __xuiLabelTestSurfaceGetGeneration;
	tProxy.surfaceDestroy = __xuiLabelTestSurfaceDestroy;
	tProxy.shapePoint = __xuiLabelTestShapePoint;
	tProxy.shapeLine = __xuiLabelTestShapeLine;
	tProxy.shapeTriangleFill = __xuiLabelTestShapeTriangleFill;
	tProxy.shapeTriangleStroke = __xuiLabelTestShapeTriangleStroke;
	tProxy.shapeRectFill = __xuiLabelTestShapeRectFill;
	tProxy.shapeRectStroke = __xuiLabelTestShapeRectStroke;
	tProxy.shapeCircleFill = __xuiLabelTestShapeCircleFill;
	tProxy.shapeCircleStroke = __xuiLabelTestShapeCircleStroke;
	tProxy.shapeRoundRectFill = __xuiLabelTestShapeRoundRectFill;
	tProxy.shapeRoundRectStroke = __xuiLabelTestShapeRoundRectStroke;
	tProxy.fontLoadFile = __xuiLabelTestFontLoadFile;
	tProxy.fontLoadMemory = __xuiLabelTestFontLoadMemory;
	tProxy.fontGetMetrics = __xuiLabelTestFontGetMetrics;
	tProxy.fontDestroy = __xuiLabelTestFontDestroy;
	tProxy.textMeasure = __xuiLabelTestTextMeasure;
	tProxy.textDraw = __xuiLabelTestTextDraw;
	tProxy.drawBegin = __xuiLabelTestDrawBegin;
	tProxy.drawEnd = __xuiLabelTestDrawEnd;
	tProxy.drawClearRect = __xuiLabelTestDrawClearRect;
	tProxy.drawSurface = __xuiLabelTestDrawSurface;
	tProxy.drawSurfaceQuad = __xuiLabelTestDrawSurfaceQuad;
	tProxy.drawPoint = __xuiLabelTestDrawPoint;
	tProxy.drawLine = __xuiLabelTestDrawLine;
	tProxy.drawTriangleFill = __xuiLabelTestDrawTriangleFill;
	tProxy.drawTriangleStroke = __xuiLabelTestDrawTriangleStroke;
	tProxy.drawRectFill = __xuiLabelTestDrawRectFill;
	tProxy.drawRectStroke = __xuiLabelTestDrawRectStroke;
	tProxy.drawCircleFill = __xuiLabelTestDrawCircleFill;
	tProxy.drawCircleStroke = __xuiLabelTestDrawCircleStroke;
	tProxy.drawRoundRectFill = __xuiLabelTestDrawRoundRectFill;
	tProxy.drawRoundRectStroke = __xuiLabelTestDrawRoundRectStroke;
	tProxy.drawText = __xuiLabelTestDrawText;
	return tProxy;
}

static int __xuiLabelTestCommand(xui_widget pWidget, const xui_event_t* pEvent, void* pUser)
{
	xui_label_test_state_t* pState;

	(void)pWidget;
	pState = (xui_label_test_state_t*)pUser;
	if ( (pState != NULL) && (pEvent != NULL) && (pEvent->iType == XUI_EVENT_COMMAND) ) {
		pState->iCommandCount++;
	}
	return XUI_OK;
}

int main(void)
{
	xui_label_test_state_t tState;
	xui_label_desc_t tDesc;
	xui_style_property_t arrProps[8];
	xui_style_desc_t tStyle;
	xui_style_value_t tTokenValue;
	xui_render_stats_t tStats;
	xui_cache_stats_t tCacheStats;
	xui_debug_widget_info_t tInfo;
	xui_vec2_t tMeasure;
	xui_context pContext;
	xui_widget pRoot;
	xui_widget pLabel;
	xui_widget pOverlay;
	xui_widget_type pLabelType;
	xui_font pBodyFont;
	xui_font pHeadlineFont;
	int iRet;
	int iFailed;
	int iDrawBefore;

	memset(&tState, 0, sizeof(tState));
	memset(&tDesc, 0, sizeof(tDesc));
	memset(&tStats, 0, sizeof(tStats));
	memset(&tCacheStats, 0, sizeof(tCacheStats));
	pContext = NULL;
	pRoot = NULL;
	pLabel = NULL;
	pOverlay = NULL;
	pBodyFont = NULL;
	pHeadlineFont = NULL;
	iFailed = 0;

	iRet = xuiCreate(&pContext);
	XUI_TEST_CHECK((iRet == XUI_OK) && (pContext != NULL), "context create failed");
	tState.tProxy = __xuiLabelTestProxy(&tState);
	iRet = xuiSetProxy(pContext, &tState.tProxy);
	XUI_TEST_CHECK(iRet == XUI_OK, "proxy set failed");
	iRet = xuiSetViewportSize(pContext, 320.0f, 180.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "viewport set failed");

	iRet = tState.tProxy.fontLoadMemory(&tState.tProxy, &pBodyFont, "body", 4, 16.0f, XUI_FONT_FORMAT_TTF);
	XUI_TEST_CHECK((iRet == XUI_OK) && (pBodyFont != NULL), "body font create failed");
	iRet = tState.tProxy.fontLoadMemory(&tState.tProxy, &pHeadlineFont, "headline", 8, 24.0f, XUI_FONT_FORMAT_TTF);
	XUI_TEST_CHECK((iRet == XUI_OK) && (pHeadlineFont != NULL), "headline font create failed");
	iRet = xuiSetDefaultFont(pContext, pBodyFont);
	XUI_TEST_CHECK(iRet == XUI_OK, "default font set failed");
	iRet = xuiRegisterFont(pContext, "body", pBodyFont);
	XUI_TEST_CHECK(iRet == XUI_OK, "body font register failed");
	iRet = xuiRegisterFont(pContext, "headline", pHeadlineFont);
	XUI_TEST_CHECK(iRet == XUI_OK, "headline font register failed");

	pLabelType = xuiLabelGetType(pContext);
	XUI_TEST_CHECK((pLabelType != NULL) && (strcmp(xuiWidgetTypeGetName(pLabelType), "label") == 0), "label type register failed");

	iRet = xuiWidgetCreate(pContext, &pRoot);
	XUI_TEST_CHECK((iRet == XUI_OK) && (pRoot != NULL), "root create failed");
	iRet = xuiSetRootWidget(pContext, pRoot);
	XUI_TEST_CHECK(iRet == XUI_OK, "root set failed");
	iRet = xuiWidgetSetRect(pRoot, (xui_rect_t){0.0f, 0.0f, 320.0f, 180.0f});
	XUI_TEST_CHECK(iRet == XUI_OK, "root rect failed");
	iRet = xuiWidgetSetLayoutType(pRoot, XUI_LAYOUT_COLUMN);
	XUI_TEST_CHECK(iRet == XUI_OK, "root layout failed");

	tDesc.iSize = sizeof(tDesc);
	tDesc.sText = "Label migration";
	tDesc.pFont = pBodyFont;
	tDesc.iTextColor = XUI_COLOR_RGBA(10, 20, 30, 255);
	tDesc.iDisabledTextColor = XUI_COLOR_RGBA(100, 110, 120, 255);
	tDesc.iTextFlags = XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_TOP;
	tDesc.iWrapMode = XUI_TEXT_WRAP_NONE;
	iRet = xuiLabelCreate(pContext, &pLabel, &tDesc);
	XUI_TEST_CHECK((iRet == XUI_OK) && (pLabel != NULL), "label create failed");
	XUI_TEST_CHECK(xuiWidgetIsType(pLabel, pLabelType), "label type check failed");
	XUI_TEST_CHECK(strcmp(xuiLabelGetText(pLabel), "Label migration") == 0, "label text getter failed");
	XUI_TEST_CHECK(xuiLabelGetTextColor(pLabel) == XUI_COLOR_RGBA(10, 20, 30, 255), "label color getter failed");
	XUI_TEST_CHECK((xuiLabelGetTextFlags(pLabel) & XUI_TEXT_CLIP) != 0, "label clip flag failed");
	iRet = xuiWidgetAddChild(pRoot, pLabel);
	XUI_TEST_CHECK(iRet == XUI_OK, "label add failed");

	iRet = xuiWidgetMeasureContent(pLabel, (xui_vec2_t){XUI_LAYOUT_UNBOUNDED, XUI_LAYOUT_UNBOUNDED}, &tMeasure);
	XUI_TEST_CHECK((iRet == XUI_OK) && (tMeasure.fX > 0.0f) && (tMeasure.fY == 16.0f), "label measure failed");
	iRet = xuiLabelSetText(pLabel, "Label migration wraps words");
	XUI_TEST_CHECK(iRet == XUI_OK, "label text set failed");
	iRet = xuiLabelSetWrapMode(pLabel, XUI_TEXT_WRAP_WORD);
	XUI_TEST_CHECK(iRet == XUI_OK, "label wrap set failed");
	iRet = xuiWidgetMeasureContent(pLabel, (xui_vec2_t){72.0f, XUI_LAYOUT_UNBOUNDED}, &tMeasure);
	XUI_TEST_CHECK((iRet == XUI_OK) && (tMeasure.fY > 16.0f), "label wrap measure failed");

	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "layout failed");
	tState.iDrawTextCount = 0;
	iRet = xuiRenderPrepare(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "render prepare failed");
	iRet = xuiGetRenderStats(pContext, &tStats);
	XUI_TEST_CHECK((iRet == XUI_OK) && (tStats.iUpdatedCaches > 0) && (tState.iDrawTextCount > 0), "label cache render failed");
	XUI_TEST_CHECK(xuiWidgetGetCacheSurface(pLabel, xuiWidgetGetStateId(pLabel)) != NULL, "label cache surface failed");
	XUI_TEST_CHECK(tState.iLastColor == XUI_COLOR_RGBA(10, 20, 30, 255), "label render color failed");

	arrProps[0] = __xuiLabelTestProp("font.name", __xuiLabelTestString("headline"));
	arrProps[1] = __xuiLabelTestProp("text.color", __xuiLabelTestColor(XUI_COLOR_RGBA(220, 80, 10, 255)));
	arrProps[2] = __xuiLabelTestProp("text.disabled_color", __xuiLabelTestColor(XUI_COLOR_RGBA(70, 80, 90, 255)));
	arrProps[3] = __xuiLabelTestProp("text.flags", __xuiLabelTestInt(XUI_TEXT_ALIGN_CENTER | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP));
	arrProps[4] = __xuiLabelTestProp("text.wrap", __xuiLabelTestInt(XUI_TEXT_WRAP_WORD));
	arrProps[5] = __xuiLabelTestProp("text.underline", __xuiLabelTestBool(1));
	arrProps[6] = __xuiLabelTestProp("text.line_gap", __xuiLabelTestFloat(2.0f));
	tStyle = __xuiLabelTestStyle(arrProps, 7);
	iRet = xuiStyleSetClass(pContext, "headline-label", &tStyle);
	XUI_TEST_CHECK(iRet == XUI_OK, "style class set failed");
	iRet = xuiWidgetAddStyleClass(pLabel, "headline-label");
	XUI_TEST_CHECK(iRet == XUI_OK, "style class add failed");
	tState.iDrawTextCount = 0;
	iRet = xuiRenderPrepare(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "styled render prepare failed");
	XUI_TEST_CHECK((tState.iDrawTextCount > 0) &&
	               (tState.iLastColor == XUI_COLOR_RGBA(220, 80, 10, 255)) &&
	               ((tState.iLastFlags & XUI_TEXT_ALIGN_CENTER) != 0) &&
	               ((tState.iLastFlags & XUI_TEXT_UNDERLINE) != 0), "styled label render failed");
	iRet = xuiWidgetMeasureContent(pLabel, (xui_vec2_t){88.0f, XUI_LAYOUT_UNBOUNDED}, &tMeasure);
	XUI_TEST_CHECK((iRet == XUI_OK) && (tMeasure.fY >= 24.0f), "styled label measure failed");

	iRet = xuiWidgetSetEnabled(pLabel, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "label disable failed");
	tState.iDrawTextCount = 0;
	iRet = xuiRenderPrepare(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "disabled render prepare failed");
	XUI_TEST_CHECK((tState.iDrawTextCount > 0) && (tState.iLastColor == XUI_COLOR_RGBA(70, 80, 90, 255)), "disabled color failed");
	iRet = xuiWidgetSetEnabled(pLabel, 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "label enable failed");

	tTokenValue = __xuiLabelTestColor(XUI_COLOR_RGBA(9, 120, 240, 255));
	iRet = xuiStyleSetToken(pContext, "skin.label", &tTokenValue);
	XUI_TEST_CHECK(iRet == XUI_OK, "style token set failed");
	arrProps[0] = __xuiLabelTestProp("text.color", __xuiLabelTestToken("skin.label"));
	tStyle = __xuiLabelTestStyle(arrProps, 1);
	iRet = xuiStyleSetClass(pContext, "token-label", &tStyle);
	XUI_TEST_CHECK(iRet == XUI_OK, "token style class set failed");
	xuiWidgetClearStyleClasses(pLabel);
	iRet = xuiWidgetAddStyleClass(pLabel, "token-label");
	XUI_TEST_CHECK(iRet == XUI_OK, "token style class add failed");
	tState.iDrawTextCount = 0;
	iRet = xuiRenderPrepare(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "token render prepare failed");
	XUI_TEST_CHECK(tState.iLastColor == XUI_COLOR_RGBA(9, 120, 240, 255), "token color failed");
	tTokenValue = __xuiLabelTestColor(XUI_COLOR_RGBA(30, 200, 90, 255));
	iDrawBefore = tState.iDrawTextCount;
	iRet = xuiStyleSetToken(pContext, "skin.label", &tTokenValue);
	XUI_TEST_CHECK(iRet == XUI_OK, "style token update failed");
	iRet = xuiRenderPrepare(pContext);
	XUI_TEST_CHECK((iRet == XUI_OK) && (tState.iDrawTextCount > iDrawBefore) &&
	               (tState.iLastColor == XUI_COLOR_RGBA(30, 200, 90, 255)), "dynamic skin render failed");

	iRet = xuiWidgetSetEventHandler(pLabel, XUI_EVENT_COMMAND, __xuiLabelTestCommand, &tState);
	XUI_TEST_CHECK(iRet == XUI_OK, "event handler set failed");
	iRet = xuiCommandDispatch(pContext, pLabel, 1001, "label.command", NULL);
	XUI_TEST_CHECK(iRet == XUI_OK, "command queue failed");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK((iRet == XUI_OK) && (tState.iCommandCount == 1), "command dispatch failed");

	memset(&tInfo, 0, sizeof(tInfo));
	tInfo.iSize = sizeof(tInfo);
	iRet = xuiDebugWidgetInspect(pLabel, &tInfo);
	XUI_TEST_CHECK((iRet == XUI_OK) && (tInfo.pWidget == pLabel) && !tInfo.bFocusable && !tInfo.bTabStop, "debug inspect failed");

	tDesc.sText = "Overlay label";
	iRet = xuiLabelCreate(pContext, &pOverlay, &tDesc);
	XUI_TEST_CHECK((iRet == XUI_OK) && (pOverlay != NULL), "overlay label create failed");
	iRet = xuiOverlayAttach(pContext, pLabel, pOverlay, 10, 0);
	XUI_TEST_CHECK((iRet == XUI_OK) && (xuiOverlayGetOwner(pOverlay) == pLabel) && (xuiOverlayTop(pContext) == pOverlay), "overlay attach failed");
	iRet = xuiOverlayDetach(pOverlay);
	XUI_TEST_CHECK(iRet == XUI_OK, "overlay detach failed");

	iRet = xuiGetCacheStats(pContext, &tCacheStats);
	XUI_TEST_CHECK((iRet == XUI_OK) && (tCacheStats.iSurfaceCount > 0), "cache stats failed");

cleanup:
	if ( pOverlay != NULL ) {
		xuiWidgetDestroy(pOverlay);
	}
	if ( pLabel != NULL ) {
		xuiWidgetDestroy(pLabel);
	}
	if ( pRoot != NULL ) {
		xuiWidgetDestroy(pRoot);
	}
	if ( pBodyFont != NULL ) {
		tState.tProxy.fontDestroy(&tState.tProxy, pBodyFont);
	}
	if ( pHeadlineFont != NULL ) {
		tState.tProxy.fontDestroy(&tState.tProxy, pHeadlineFont);
	}
	if ( pContext != NULL ) {
		xuiDestroy(pContext);
	}
	if ( iFailed ) {
		return 1;
	}
	printf("xui_label_test passed\n");
	return 0;
}
