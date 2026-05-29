#include "xui_internal.h"

#include <string.h>

#define XUI_CONTEXT_REQUIRED_CAPS \
	(XUI_PROXY_CAP_SURFACE_TARGET | \
	 XUI_PROXY_CAP_SURFACE_READ | \
	 XUI_PROXY_CAP_SURFACE_QUAD | \
	 XUI_PROXY_CAP_SURFACE_CLEAR_RECT | \
	 XUI_PROXY_CAP_SURFACE_SAMPLER | \
	 XUI_PROXY_CAP_DRAW_CONTEXT | \
	 XUI_PROXY_CAP_SHAPE | \
	 XUI_PROXY_CAP_FONT_TTF | \
	 XUI_PROXY_CAP_FONT_XRF | \
	 XUI_PROXY_CAP_TEXT)

struct xui_resource_t {
	uint32_t iMagic;
	xui_context pContext;
	xui_resource pNext;
	char* sName;
	int iKind;
	void* pHandle;
	void* pUser;
	void (*onDestroy)(xui_context pContext, void* pHandle, void* pUser);
	uint32_t iGeneration;
	int iRefCount;
	xui_resource_dependency_t* pDependencies;
};

struct xui_resource_dependency_t {
	xui_resource pResource;
	xui_resource_dependency_t* pNext;
};

static int __xuiContextValid(xui_context pContext)
{
	return (pContext != NULL) && (pContext->iMagic == XUI_CONTEXT_MAGIC);
}

static int __xuiResourceValid(xui_resource pResource)
{
	return (pResource != NULL) && (pResource->iMagic == XUI_RESOURCE_MAGIC);
}

static char* __xuiCoreStringDuplicate(const char* sText)
{
	char* sCopy;
	size_t iSize;

	if ( sText == NULL ) {
		return NULL;
	}
	iSize = strlen(sText) + 1u;
	sCopy = (char*)xrtMalloc(iSize);
	if ( sCopy == NULL ) {
		return NULL;
	}
	memcpy(sCopy, sText, iSize);
	return sCopy;
}

static int __xuiPainterValid(xui_painter pPainter)
{
	return (pPainter != NULL) && (pPainter->iMagic == XUI_PAINTER_MAGIC);
}

static int __xuiFloatValid(float fValue)
{
	return (fValue == fValue) && (fValue >= 0.0f) && (fValue <= XUI_CONTEXT_MAX_VIEWPORT);
}

static int __xuiRectValid(xui_rect_i_t tRect)
{
	return (tRect.iW > 0) && (tRect.iH > 0);
}

static int __xuiContextCeilPositive(float fValue)
{
	if ( fValue <= 0.0f ) {
		return 0;
	}
	return xuiInternalPixelCeil(fValue);
}

static xui_rect_i_t __xuiRectUnion(xui_rect_i_t tA, xui_rect_i_t tB)
{
	xui_rect_i_t tRet;
	int iLeft;
	int iTop;
	int iRight;
	int iBottom;

	iLeft = (tA.iX < tB.iX) ? tA.iX : tB.iX;
	iTop = (tA.iY < tB.iY) ? tA.iY : tB.iY;
	iRight = ((tA.iX + tA.iW) > (tB.iX + tB.iW)) ? (tA.iX + tA.iW) : (tB.iX + tB.iW);
	iBottom = ((tA.iY + tA.iH) > (tB.iY + tB.iH)) ? (tA.iY + tA.iH) : (tB.iY + tB.iH);
	tRet.iX = iLeft;
	tRet.iY = iTop;
	tRet.iW = iRight - iLeft;
	tRet.iH = iBottom - iTop;
	return tRet;
}

static int __xuiRectTouchesOrIntersects(xui_rect_i_t tA, xui_rect_i_t tB)
{
	if ( (tA.iX > (tB.iX + tB.iW)) || (tB.iX > (tA.iX + tA.iW)) ) {
		return 0;
	}
	if ( (tA.iY > (tB.iY + tB.iH)) || (tB.iY > (tA.iY + tA.iH)) ) {
		return 0;
	}
	return 1;
}

static int __xuiContextClipToViewport(xui_context pContext, xui_rect_i_t* pRect)
{
	int iWidth;
	int iHeight;
	int iLeft;
	int iTop;
	int iRight;
	int iBottom;

	iWidth = __xuiContextCeilPositive(pContext->fViewportWidth);
	iHeight = __xuiContextCeilPositive(pContext->fViewportHeight);
	if ( (iWidth <= 0) || (iHeight <= 0) ) {
		return 1;
	}
	iLeft = pRect->iX;
	iTop = pRect->iY;
	iRight = pRect->iX + pRect->iW;
	iBottom = pRect->iY + pRect->iH;
	if ( iLeft < 0 ) {
		iLeft = 0;
	}
	if ( iTop < 0 ) {
		iTop = 0;
	}
	if ( iRight > iWidth ) {
		iRight = iWidth;
	}
	if ( iBottom > iHeight ) {
		iBottom = iHeight;
	}
	pRect->iX = iLeft;
	pRect->iY = iTop;
	pRect->iW = iRight - iLeft;
	pRect->iH = iBottom - iTop;
	return __xuiRectValid(*pRect);
}

static int __xuiContextDamageReserve(xui_context pContext, int iCapacity)
{
	xui_rect_i_t* pDamage;

	if ( iCapacity <= pContext->iDamageCapacity ) {
		return XUI_OK;
	}
	if ( iCapacity < (pContext->iDamageCapacity * 2) ) {
		iCapacity = pContext->iDamageCapacity * 2;
	}
	if ( pContext->pDamage == pContext->arrInlineDamage ) {
		pDamage = (xui_rect_i_t*)xrtMalloc(sizeof(*pDamage) * (size_t)iCapacity);
		if ( pDamage == NULL ) {
			return XUI_ERROR_OUT_OF_MEMORY;
		}
		memcpy(pDamage, pContext->pDamage, sizeof(*pDamage) * (size_t)pContext->iDamageCount);
	} else {
		pDamage = (xui_rect_i_t*)xrtRealloc(pContext->pDamage, sizeof(*pDamage) * (size_t)iCapacity);
		if ( pDamage == NULL ) {
			return XUI_ERROR_OUT_OF_MEMORY;
		}
	}
	pContext->pDamage = pDamage;
	pContext->iDamageCapacity = iCapacity;
	return XUI_OK;
}

static void __xuiContextBumpGeneration(xui_context pContext)
{
	pContext->iGeneration++;
	if ( pContext->iGeneration == 0 ) {
		pContext->iGeneration = 1;
	}
}

static void __xuiContextRemoveDamage(xui_context pContext, int iIndex)
{
	int iMoveCount;

	iMoveCount = pContext->iDamageCount - iIndex - 1;
	if ( iMoveCount > 0 ) {
		memmove(&pContext->pDamage[iIndex], &pContext->pDamage[iIndex + 1], sizeof(pContext->pDamage[0]) * (size_t)iMoveCount);
	}
	pContext->iDamageCount--;
}

static int __xuiContextAddDamage(xui_context pContext, xui_rect_i_t tRect)
{
	int i;
	int j;
	int iRet;

	if ( !__xuiRectValid(tRect) ) {
		return XUI_OK;
	}
	if ( !__xuiContextClipToViewport(pContext, &tRect) ) {
		return XUI_OK;
	}
	for ( i = 0; i < pContext->iDamageCount; i++ ) {
		if ( __xuiRectTouchesOrIntersects(pContext->pDamage[i], tRect) ) {
			pContext->pDamage[i] = __xuiRectUnion(pContext->pDamage[i], tRect);
			j = i + 1;
			while ( j < pContext->iDamageCount ) {
				if ( __xuiRectTouchesOrIntersects(pContext->pDamage[i], pContext->pDamage[j]) ) {
					pContext->pDamage[i] = __xuiRectUnion(pContext->pDamage[i], pContext->pDamage[j]);
					__xuiContextRemoveDamage(pContext, j);
				} else {
					j++;
				}
			}
			__xuiContextBumpGeneration(pContext);
			return XUI_OK;
		}
	}
	iRet = __xuiContextDamageReserve(pContext, pContext->iDamageCount + 1);
	if ( iRet != XUI_OK ) {
		return iRet;
	}
	pContext->pDamage[pContext->iDamageCount++] = tRect;
	__xuiContextBumpGeneration(pContext);
	return XUI_OK;
}

int xuiInternalContextIsValid(xui_context pContext)
{
	return __xuiContextValid(pContext);
}

int xuiInternalContextHasProxy(xui_context pContext)
{
	return __xuiContextValid(pContext) && pContext->bHasProxy;
}

xui_proxy xuiInternalContextGetProxy(xui_context pContext)
{
	if ( !__xuiContextValid(pContext) || !pContext->bHasProxy ) {
		return NULL;
	}
	return &pContext->tProxy;
}

int xuiInternalContextInvalidateRect(xui_context pContext, xui_rect_i_t tRect)
{
	if ( !__xuiContextValid(pContext) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	return __xuiContextAddDamage(pContext, tRect);
}

int xuiInternalContextInvalidateAll(xui_context pContext)
{
	if ( !__xuiContextValid(pContext) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	return xuiInvalidateAll(pContext);
}

void xuiInternalContextBumpGeneration(xui_context pContext)
{
	if ( __xuiContextValid(pContext) ) {
		__xuiContextBumpGeneration(pContext);
	}
}

void xuiInternalContextDestroyRenderTree(xui_context pContext)
{
	if ( !__xuiContextValid(pContext) ) {
		return;
	}
	if ( pContext->pRenderNodes != NULL ) {
		xrtFree(pContext->pRenderNodes);
	}
	pContext->pRenderNodes = NULL;
	pContext->iRenderNodeCount = 0;
	pContext->iRenderNodeCapacity = 0;
	pContext->iRenderTreeGeneration = 0;
}

void xuiInternalContextDestroyInput(xui_context pContext)
{
	int i;

	if ( !__xuiContextValid(pContext) ) {
		return;
	}
	for ( i = 0; i < pContext->iHotkeyCount; i++ ) {
		if ( pContext->pHotkeys[i].sCommand != NULL ) {
			xrtFree(pContext->pHotkeys[i].sCommand);
		}
	}
	if ( pContext->pHotkeys != pContext->arrInlineHotkeys ) {
		xrtFree(pContext->pHotkeys);
	}
	pContext->pHotkeys = pContext->arrInlineHotkeys;
	pContext->iHotkeyCount = 0;
	pContext->iHotkeyCapacity = XUI_CONTEXT_HOTKEY_INLINE;
	pContext->pDragWidget = NULL;
	pContext->bDragActive = 0;
	pContext->pLastClickWidget = NULL;
	if ( pContext->sActiveTooltipText != NULL ) {
		xrtFree(pContext->sActiveTooltipText);
		pContext->sActiveTooltipText = NULL;
	}
	pContext->pTooltipOwner = NULL;
	pContext->pTooltipPopupWidget = NULL;
	pContext->bTooltipOpen = 0;
	pContext->fTooltipHoverTime = 0.0f;
	memset(&pContext->tActiveTooltip, 0, sizeof(pContext->tActiveTooltip));
	pContext->bImeEnabled = 0;
	pContext->bHasImeCandidateRect = 0;
}

static void __xuiResourceDestroyOne(xui_resource pResource)
{
	xui_resource_dependency_t* pDependency;
	xui_resource_dependency_t* pNextDependency;

	if ( !__xuiResourceValid(pResource) ) {
		return;
	}
	pDependency = pResource->pDependencies;
	while ( pDependency != NULL ) {
		pNextDependency = pDependency->pNext;
		xrtFree(pDependency);
		pDependency = pNextDependency;
	}
	pResource->pDependencies = NULL;
	if ( pResource->onDestroy != NULL ) {
		pResource->onDestroy(pResource->pContext, pResource->pHandle, pResource->pUser);
	}
	if ( pResource->sName != NULL ) {
		xrtFree(pResource->sName);
	}
	pResource->iMagic = 0;
	xrtFree(pResource);
}

static void __xuiResourceRemoveDependencyReferences(xui_context pContext, xui_resource pRemoved)
{
	xui_resource pResource;
	xui_resource_dependency_t* pDependency;
	xui_resource_dependency_t* pPrev;
	xui_resource_dependency_t* pNext;

	for ( pResource = pContext->pResources; pResource != NULL; pResource = pResource->pNext ) {
		if ( pResource == pRemoved ) {
			continue;
		}
		pPrev = NULL;
		pDependency = pResource->pDependencies;
		while ( pDependency != NULL ) {
			pNext = pDependency->pNext;
			if ( pDependency->pResource == pRemoved ) {
				if ( pPrev != NULL ) {
					pPrev->pNext = pNext;
				} else {
					pResource->pDependencies = pNext;
				}
				xrtFree(pDependency);
				pResource->iGeneration = pContext->iNextResourceGeneration++;
				if ( pContext->iNextResourceGeneration == 0 ) {
					pContext->iNextResourceGeneration = 1;
				}
			} else {
				pPrev = pDependency;
			}
			pDependency = pNext;
		}
	}
}

void xuiInternalContextDestroyResources(xui_context pContext)
{
	xui_resource pResource;
	xui_resource pNext;

	if ( !__xuiContextValid(pContext) ) {
		return;
	}
	pResource = pContext->pResources;
	while ( pResource != NULL ) {
		pNext = pResource->pNext;
		__xuiResourceDestroyOne(pResource);
		pResource = pNext;
	}
	pContext->pResources = NULL;
}

static int __xuiContextWidgetContains(xui_widget pRoot, xui_widget pWidget)
{
	xui_widget pScan;

	if ( (pRoot == NULL) || (pWidget == NULL) ) {
		return 0;
	}
	for ( pScan = pWidget; pScan != NULL; pScan = pScan->pParent ) {
		if ( pScan == pRoot ) {
			return 1;
		}
	}
	return 0;
}

static void __xuiContextClearOverlayOwners(xui_widget pScan, xui_widget pRemoved)
{
	xui_widget pChild;

	if ( pScan == NULL ) {
		return;
	}
	if ( __xuiContextWidgetContains(pRemoved, pScan->pOverlayOwner) ) {
		pScan->pOverlayOwner = NULL;
	}
	for ( pChild = pScan->pFirstChild; pChild != NULL; pChild = pChild->pNextSibling ) {
		__xuiContextClearOverlayOwners(pChild, pRemoved);
	}
}

void xuiInternalContextDetachWidget(xui_context pContext, xui_widget pWidget)
{
	int i;

	if ( !__xuiContextValid(pContext) || (pWidget == NULL) ) {
		return;
	}
	if ( __xuiContextWidgetContains(pWidget, pContext->pHoverWidget) ) {
		if ( pContext->pHoverWidget != NULL ) {
			pContext->pHoverWidget->iInputState &= ~XUI_WIDGET_STATE_HOVER;
		}
		pContext->pHoverWidget = NULL;
	}
	if ( __xuiContextWidgetContains(pWidget, pContext->pActiveWidget) ) {
		if ( pContext->pActiveWidget != NULL ) {
			pContext->pActiveWidget->iInputState &= ~XUI_WIDGET_STATE_ACTIVE;
		}
		pContext->pActiveWidget = NULL;
		pContext->iActiveButton = 0;
	}
	if ( __xuiContextWidgetContains(pWidget, pContext->pFocusWidget) ) {
		if ( pContext->pFocusWidget != NULL ) {
			pContext->pFocusWidget->iInputState &= ~XUI_WIDGET_STATE_FOCUS;
		}
		pContext->pFocusWidget = NULL;
	}
	if ( __xuiContextWidgetContains(pWidget, pContext->pPointerCaptureWidget) ) {
		pContext->pPointerCaptureWidget = NULL;
	}
	if ( __xuiContextWidgetContains(pWidget, pContext->pDragWidget) ) {
		pContext->pDragWidget = NULL;
		pContext->bDragActive = 0;
		pContext->iDragButton = 0;
	}
	if ( __xuiContextWidgetContains(pWidget, pContext->pLastClickWidget) ) {
		pContext->pLastClickWidget = NULL;
		pContext->iLastClickButton = 0;
		pContext->fLastClickTime = 0.0;
	}
	xuiInternalTooltipDetachWidget(pContext, pWidget);
	for ( i = 0; i < pContext->iHotkeyCount; i++ ) {
		if ( __xuiContextWidgetContains(pWidget, pContext->pHotkeys[i].pWidget) ) {
			pContext->pHotkeys[i].pWidget = NULL;
		}
	}
	__xuiContextClearOverlayOwners(pContext->pOverlayRoot, pWidget);
	for ( i = pContext->iEventRead; i < pContext->iEventCount; i++ ) {
		if ( __xuiContextWidgetContains(pWidget, pContext->pEvents[i].pTarget) ) {
			pContext->pEvents[i].pTarget = NULL;
		}
		if ( __xuiContextWidgetContains(pWidget, pContext->pEvents[i].pRelated) ) {
			pContext->pEvents[i].pRelated = NULL;
		}
	}
}

static xui_rect_i_t __xuiContextFullRect(xui_context pContext)
{
	xui_rect_i_t tRect;

	tRect.iX = 0;
	tRect.iY = 0;
	tRect.iW = __xuiContextCeilPositive(pContext->fViewportWidth);
	tRect.iH = __xuiContextCeilPositive(pContext->fViewportHeight);
	return tRect;
}

static int __xuiProxyRequiredCallbacks(const xui_proxy_t* pProxy)
{
	return (pProxy->getCaps != NULL) &&
	       (pProxy->clipboardSetText != NULL) &&
	       (pProxy->clipboardGetText != NULL) &&
	       (pProxy->imeGetEnabled != NULL) &&
	       (pProxy->imeSetEnabled != NULL) &&
	       (pProxy->imeSetCandidateRect != NULL) &&
	       (pProxy->surfaceCreate != NULL) &&
	       (pProxy->surfaceCreateRGBA != NULL) &&
	       (pProxy->surfaceLoadFile != NULL) &&
	       (pProxy->surfaceLoadMemory != NULL) &&
	       (pProxy->surfaceUpdateRGBA != NULL) &&
	       (pProxy->surfaceReadRGBA != NULL) &&
	       (pProxy->surfaceGetDesc != NULL) &&
	       (pProxy->surfaceDraw != NULL) &&
	       (pProxy->surfaceClear != NULL) &&
	       (pProxy->surfaceClearRect != NULL) &&
	       (pProxy->surfaceDrawTo != NULL) &&
	       (pProxy->surfaceDrawQuad != NULL) &&
	       (pProxy->surfaceDrawQuadTo != NULL) &&
	       (pProxy->surfaceGetSampler != NULL) &&
	       (pProxy->surfaceSetSampler != NULL) &&
	       (pProxy->surfaceGetGeneration != NULL) &&
	       (pProxy->surfaceDestroy != NULL) &&
	       (pProxy->shapePoint != NULL) &&
	       (pProxy->shapeLine != NULL) &&
	       (pProxy->shapeTriangleFill != NULL) &&
	       (pProxy->shapeTriangleStroke != NULL) &&
	       (pProxy->shapeRectFill != NULL) &&
	       (pProxy->shapeRectStroke != NULL) &&
	       (pProxy->shapeCircleFill != NULL) &&
	       (pProxy->shapeCircleStroke != NULL) &&
	       (pProxy->shapeRoundRectFill != NULL) &&
	       (pProxy->shapeRoundRectStroke != NULL) &&
	       (pProxy->fontLoadFile != NULL) &&
	       (pProxy->fontLoadMemory != NULL) &&
	       (pProxy->fontGetMetrics != NULL) &&
	       (pProxy->fontDestroy != NULL) &&
	       (pProxy->textMeasure != NULL) &&
	       (pProxy->textDraw != NULL) &&
	       (pProxy->drawBegin != NULL) &&
	       (pProxy->drawEnd != NULL) &&
	       (pProxy->drawClearRect != NULL) &&
	       (pProxy->drawSurface != NULL) &&
	       (pProxy->drawSurfaceQuad != NULL) &&
	       (pProxy->drawPoint != NULL) &&
	       (pProxy->drawLine != NULL) &&
	       (pProxy->drawTriangleFill != NULL) &&
	       (pProxy->drawTriangleStroke != NULL) &&
	       (pProxy->drawRectFill != NULL) &&
	       (pProxy->drawRectStroke != NULL) &&
	       (pProxy->drawCircleFill != NULL) &&
	       (pProxy->drawCircleStroke != NULL) &&
	       (pProxy->drawRoundRectFill != NULL) &&
	       (pProxy->drawRoundRectStroke != NULL) &&
	       (pProxy->drawText != NULL);
}

static int __xuiContextValidateProxy(const xui_proxy_t* pProxy, xui_proxy_caps_t* pCaps)
{
	int iRet;

	if ( (pProxy == NULL) || (pCaps == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( (pProxy->iSize < sizeof(*pProxy)) || (pProxy->iVersion != XUI_PROXY_VERSION) ) {
		return XUI_ERROR_UNSUPPORTED;
	}
	if ( !__xuiProxyRequiredCallbacks(pProxy) ) {
		return XUI_ERROR_UNSUPPORTED;
	}
	memset(pCaps, 0, sizeof(*pCaps));
	iRet = pProxy->getCaps((xui_proxy)pProxy, pCaps);
	if ( iRet != XUI_OK ) {
		return iRet;
	}
	if ( pCaps->iSize < sizeof(*pCaps) ) {
		return XUI_ERROR_UNSUPPORTED;
	}
	if ( (pCaps->iCaps & XUI_CONTEXT_REQUIRED_CAPS) != XUI_CONTEXT_REQUIRED_CAPS ) {
		return XUI_ERROR_UNSUPPORTED;
	}
	if ( (pCaps->iSurfaceFormat != XUI_SURFACE_FORMAT_RGBA8) ||
	     (pCaps->iInternalAlpha != XUI_SURFACE_ALPHA_PREMULTIPLIED) ) {
		return XUI_ERROR_UNSUPPORTED;
	}
	if ( ((pCaps->tDefaultSampler.iMinFilter != XUI_SURFACE_FILTER_NEAREST) &&
	      (pCaps->tDefaultSampler.iMinFilter != XUI_SURFACE_FILTER_LINEAR)) ||
	     ((pCaps->tDefaultSampler.iMagFilter != XUI_SURFACE_FILTER_NEAREST) &&
	      (pCaps->tDefaultSampler.iMagFilter != XUI_SURFACE_FILTER_LINEAR)) ||
	     ((pCaps->tDefaultSampler.iWrapS != XUI_SURFACE_WRAP_CLAMP) &&
	      (pCaps->tDefaultSampler.iWrapS != XUI_SURFACE_WRAP_REPEAT)) ||
	     ((pCaps->tDefaultSampler.iWrapT != XUI_SURFACE_WRAP_CLAMP) &&
	      (pCaps->tDefaultSampler.iWrapT != XUI_SURFACE_WRAP_REPEAT)) ) {
		return XUI_ERROR_UNSUPPORTED;
	}
	return XUI_OK;
}

XUI_API int xuiCreate(xui_context* ppContext)
{
	xui_context pContext;

	if ( ppContext == NULL ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	*ppContext = NULL;
	pContext = (xui_context)xrtMalloc(sizeof(*pContext));
	if ( pContext == NULL ) {
		return XUI_ERROR_OUT_OF_MEMORY;
	}
	memset(pContext, 0, sizeof(*pContext));
	pContext->iMagic = XUI_CONTEXT_MAGIC;
	pContext->fDpiScale = 1.0f;
	pContext->iGeneration = 1;
	pContext->iNextStylePropertyId = 1;
	pContext->iNextResourceGeneration = 1;
	pContext->pDamage = pContext->arrInlineDamage;
	pContext->iDamageCapacity = XUI_CONTEXT_DAMAGE_INLINE;
	pContext->pEvents = pContext->arrInlineEvents;
	pContext->iEventCapacity = XUI_CONTEXT_EVENT_INLINE;
	pContext->pHotkeys = pContext->arrInlineHotkeys;
	pContext->iHotkeyCapacity = XUI_CONTEXT_HOTKEY_INLINE;
	pContext->pFonts = pContext->arrInlineFonts;
	pContext->iFontCapacity = XUI_CONTEXT_FONT_INLINE;
	xuiThemeDefault(&pContext->tTheme);
	pContext->tChromeStyle.iSize = sizeof(pContext->tChromeStyle);
	pContext->tChromeStyle.iBackdropColor = XUI_COLOR_RGBA(18, 20, 24, 255);
	pContext->tChromeStyle.iPopupColor = XUI_COLOR_RGBA(248, 249, 251, 255);
	pContext->tChromeStyle.iPopupBorderColor = XUI_COLOR_RGBA(120, 126, 138, 255);
	pContext->tChromeStyle.iTooltipColor = XUI_COLOR_RGBA(28, 32, 38, 245);
	pContext->tChromeStyle.iTooltipTextColor = XUI_COLOR_RGBA(248, 250, 252, 255);
	pContext->tChromeStyle.iModalOverlayColor = XUI_COLOR_RGBA(0, 0, 0, 96);
	pContext->tChromeStyle.fRadius = 4.0f;
	pContext->tChromeStyle.fBorderWidth = 1.0f;
	pContext->tChromeStyle.fShadowSize = 8.0f;
	*ppContext = pContext;
	return XUI_OK;
}

static void __xuiContextDestroyFonts(xui_context pContext)
{
	int i;

	if ( !__xuiContextValid(pContext) ) {
		return;
	}
	for ( i = 0; i < pContext->iFontCount; i++ ) {
		if ( pContext->pFonts[i].sName != NULL ) {
			xrtFree(pContext->pFonts[i].sName);
		}
	}
	if ( pContext->pFonts != pContext->arrInlineFonts ) {
		xrtFree(pContext->pFonts);
	}
	pContext->pFonts = pContext->arrInlineFonts;
	pContext->iFontCount = 0;
	pContext->iFontCapacity = XUI_CONTEXT_FONT_INLINE;
	pContext->pDefaultFont = NULL;
}

XUI_API void xuiDestroy(xui_context pContext)
{
	if ( !__xuiContextValid(pContext) ) {
		return;
	}
	if ( pContext->pRoot != NULL ) {
		xuiWidgetDestroy(pContext->pRoot);
		pContext->pRoot = NULL;
	}
	if ( pContext->pOverlayRoot != NULL ) {
		xuiWidgetDestroy(pContext->pOverlayRoot);
		pContext->pOverlayRoot = NULL;
	}
	if ( pContext->pDamage != pContext->arrInlineDamage ) {
		xrtFree(pContext->pDamage);
	}
	if ( pContext->pEvents != pContext->arrInlineEvents ) {
		xrtFree(pContext->pEvents);
	}
	xuiInternalContextDestroyInput(pContext);
	xuiInternalContextDestroyRenderTree(pContext);
	xuiInternalContextDestroyResources(pContext);
	xuiInternalContextDestroyStyles(pContext);
	xuiInternalContextDestroyWidgetTypes(pContext);
	__xuiContextDestroyFonts(pContext);
	pContext->iMagic = 0;
	xrtFree(pContext);
}

XUI_API int xuiSetProxy(xui_context pContext, const xui_proxy_t* pProxy)
{
	xui_proxy_caps_t tCaps;
	int iRet;

	if ( !__xuiContextValid(pContext) || (pProxy == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( pContext->bHasProxy ) {
		return XUI_ERROR_ALREADY_INITIALIZED;
	}
	iRet = __xuiContextValidateProxy(pProxy, &tCaps);
	if ( iRet != XUI_OK ) {
		return iRet;
	}
	pContext->tProxy = *pProxy;
	pContext->tProxyCaps = tCaps;
	pContext->bHasProxy = 1;
	__xuiContextBumpGeneration(pContext);
	return XUI_OK;
}

XUI_API int xuiGetProxy(xui_context pContext, xui_proxy_t* pProxy)
{
	if ( !__xuiContextValid(pContext) || (pProxy == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( !pContext->bHasProxy ) {
		return XUI_ERROR_NOT_INITIALIZED;
	}
	*pProxy = pContext->tProxy;
	return XUI_OK;
}

XUI_API int xuiGetProxyCaps(xui_context pContext, xui_proxy_caps_t* pCaps)
{
	if ( !__xuiContextValid(pContext) || (pCaps == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( !pContext->bHasProxy ) {
		return XUI_ERROR_NOT_INITIALIZED;
	}
	*pCaps = pContext->tProxyCaps;
	return XUI_OK;
}

XUI_API int xuiSetViewportSize(xui_context pContext, float fWidth, float fHeight)
{
	if ( !__xuiContextValid(pContext) || !__xuiFloatValid(fWidth) || !__xuiFloatValid(fHeight) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( (pContext->fViewportWidth == fWidth) && (pContext->fViewportHeight == fHeight) ) {
		return XUI_OK;
	}
	pContext->fViewportWidth = fWidth;
	pContext->fViewportHeight = fHeight;
	pContext->iDamageCount = 0;
	if ( (__xuiContextCeilPositive(fWidth) <= 0) || (__xuiContextCeilPositive(fHeight) <= 0) ) {
		__xuiContextBumpGeneration(pContext);
		return XUI_OK;
	}
	return __xuiContextAddDamage(pContext, __xuiContextFullRect(pContext));
}

XUI_API xui_vec2_t xuiGetViewportSize(xui_context pContext)
{
	xui_vec2_t tSize;

	tSize.fX = 0.0f;
	tSize.fY = 0.0f;
	if ( __xuiContextValid(pContext) ) {
		tSize.fX = pContext->fViewportWidth;
		tSize.fY = pContext->fViewportHeight;
	}
	return tSize;
}

XUI_API int xuiSetVirtualDpi(xui_context pContext, float fDpiScale)
{
	if ( !__xuiContextValid(pContext) || (fDpiScale != fDpiScale) || (fDpiScale <= 0.0f) || (fDpiScale > 64.0f) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( pContext->fDpiScale == fDpiScale ) {
		return XUI_OK;
	}
	pContext->fDpiScale = fDpiScale;
	pContext->iDamageCount = 0;
	if ( (__xuiContextCeilPositive(pContext->fViewportWidth) <= 0) ||
	     (__xuiContextCeilPositive(pContext->fViewportHeight) <= 0) ) {
		__xuiContextBumpGeneration(pContext);
		return XUI_OK;
	}
	return __xuiContextAddDamage(pContext, __xuiContextFullRect(pContext));
}

XUI_API float xuiGetVirtualDpi(xui_context pContext)
{
	if ( !__xuiContextValid(pContext) ) {
		return 0.0f;
	}
	return pContext->fDpiScale;
}

XUI_API void xuiThemeDefault(xui_theme_t* pTheme)
{
	if ( pTheme == NULL ) {
		return;
	}
	memset(pTheme, 0, sizeof(*pTheme));
	pTheme->iSize = sizeof(*pTheme);
	pTheme->iTextColor = XUI_COLOR_RGBA(34, 37, 41, 255);
	pTheme->iBackgroundColor = XUI_COLOR_RGBA(245, 247, 250, 255);
	pTheme->iPanelColor = XUI_COLOR_RGBA(255, 255, 255, 255);
	pTheme->iBorderColor = XUI_COLOR_RGBA(188, 195, 205, 255);
	pTheme->iAccentColor = XUI_COLOR_RGBA(0, 112, 210, 255);
	pTheme->iSelectionColor = XUI_COLOR_RGBA(0, 112, 210, 72);
	pTheme->iStateNormalColor = XUI_COLOR_RGBA(255, 255, 255, 255);
	pTheme->iStateHoverColor = XUI_COLOR_RGBA(236, 244, 252, 255);
	pTheme->iStateActiveColor = XUI_COLOR_RGBA(214, 232, 248, 255);
	pTheme->iStateFocusColor = XUI_COLOR_RGBA(0, 112, 210, 255);
	pTheme->iStateDisabledColor = XUI_COLOR_RGBA(162, 168, 176, 255);
	pTheme->fRadius = 4.0f;
	pTheme->fPadding = 6.0f;
	pTheme->fSpacing = 6.0f;
	pTheme->fBorderWidth = 1.0f;
}

static int __xuiCoreSetColorToken(xui_context pContext, const char* sName, uint32_t iColor)
{
	xui_style_value_t tValue;

	memset(&tValue, 0, sizeof(tValue));
	tValue.iSize = sizeof(tValue);
	tValue.iType = XUI_STYLE_VALUE_COLOR;
	tValue.iColor = iColor;
	return xuiStyleSetToken(pContext, sName, &tValue);
}

static int __xuiCoreSetFloatToken(xui_context pContext, const char* sName, float fValue)
{
	xui_style_value_t tValue;

	memset(&tValue, 0, sizeof(tValue));
	tValue.iSize = sizeof(tValue);
	tValue.iType = XUI_STYLE_VALUE_FLOAT;
	tValue.fFloat = fValue;
	return xuiStyleSetToken(pContext, sName, &tValue);
}

XUI_API int xuiSetTheme(xui_context pContext, const xui_theme_t* pTheme)
{
	xui_theme_t tTheme;
	int iRet;

	if ( !__xuiContextValid(pContext) || (pTheme == NULL) ||
	     ((pTheme->iSize != 0) && (pTheme->iSize < sizeof(*pTheme))) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	tTheme = *pTheme;
	tTheme.iSize = sizeof(tTheme);
	pContext->tTheme = tTheme;
	pContext->pDefaultFont = tTheme.pFont;
	iRet = xuiStyleBeginUpdate(pContext);
	if ( iRet != XUI_OK ) {
		return iRet;
	}
	iRet = __xuiCoreSetColorToken(pContext, "theme.text", tTheme.iTextColor);
	if ( iRet == XUI_OK ) iRet = __xuiCoreSetColorToken(pContext, "theme.background", tTheme.iBackgroundColor);
	if ( iRet == XUI_OK ) iRet = __xuiCoreSetColorToken(pContext, "theme.panel", tTheme.iPanelColor);
	if ( iRet == XUI_OK ) iRet = __xuiCoreSetColorToken(pContext, "theme.border", tTheme.iBorderColor);
	if ( iRet == XUI_OK ) iRet = __xuiCoreSetColorToken(pContext, "theme.accent", tTheme.iAccentColor);
	if ( iRet == XUI_OK ) iRet = __xuiCoreSetColorToken(pContext, "theme.selection", tTheme.iSelectionColor);
	if ( iRet == XUI_OK ) iRet = __xuiCoreSetColorToken(pContext, "theme.state.normal", tTheme.iStateNormalColor);
	if ( iRet == XUI_OK ) iRet = __xuiCoreSetColorToken(pContext, "theme.state.hover", tTheme.iStateHoverColor);
	if ( iRet == XUI_OK ) iRet = __xuiCoreSetColorToken(pContext, "theme.state.active", tTheme.iStateActiveColor);
	if ( iRet == XUI_OK ) iRet = __xuiCoreSetColorToken(pContext, "theme.state.focus", tTheme.iStateFocusColor);
	if ( iRet == XUI_OK ) iRet = __xuiCoreSetColorToken(pContext, "theme.state.disabled", tTheme.iStateDisabledColor);
	if ( iRet == XUI_OK ) iRet = __xuiCoreSetFloatToken(pContext, "theme.radius", tTheme.fRadius);
	if ( iRet == XUI_OK ) iRet = __xuiCoreSetFloatToken(pContext, "theme.padding", tTheme.fPadding);
	if ( iRet == XUI_OK ) iRet = __xuiCoreSetFloatToken(pContext, "theme.spacing", tTheme.fSpacing);
	if ( iRet == XUI_OK ) iRet = __xuiCoreSetFloatToken(pContext, "theme.border_width", tTheme.fBorderWidth);
	if ( iRet != XUI_OK ) {
		(void)xuiStyleEndUpdate(pContext);
		return iRet;
	}
	return xuiStyleEndUpdate(pContext);
}

XUI_API int xuiGetTheme(xui_context pContext, xui_theme_t* pTheme)
{
	if ( !__xuiContextValid(pContext) || (pTheme == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	*pTheme = pContext->tTheme;
	pTheme->iSize = sizeof(*pTheme);
	return XUI_OK;
}

XUI_API int xuiSetChromeStyle(xui_context pContext, const xui_chrome_style_t* pChrome)
{
	xui_chrome_style_t tChrome;
	int iRet;

	if ( !__xuiContextValid(pContext) || (pChrome == NULL) ||
	     ((pChrome->iSize != 0) && (pChrome->iSize < sizeof(*pChrome))) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	tChrome = *pChrome;
	tChrome.iSize = sizeof(tChrome);
	pContext->tChromeStyle = tChrome;
	iRet = xuiStyleBeginUpdate(pContext);
	if ( iRet != XUI_OK ) {
		return iRet;
	}
	iRet = __xuiCoreSetColorToken(pContext, "chrome.backdrop", tChrome.iBackdropColor);
	if ( iRet == XUI_OK ) iRet = __xuiCoreSetColorToken(pContext, "chrome.popup", tChrome.iPopupColor);
	if ( iRet == XUI_OK ) iRet = __xuiCoreSetColorToken(pContext, "chrome.popup_border", tChrome.iPopupBorderColor);
	if ( iRet == XUI_OK ) iRet = __xuiCoreSetColorToken(pContext, "chrome.tooltip", tChrome.iTooltipColor);
	if ( iRet == XUI_OK ) iRet = __xuiCoreSetColorToken(pContext, "chrome.tooltip_text", tChrome.iTooltipTextColor);
	if ( iRet == XUI_OK ) iRet = __xuiCoreSetColorToken(pContext, "chrome.modal_overlay", tChrome.iModalOverlayColor);
	if ( iRet == XUI_OK ) iRet = __xuiCoreSetFloatToken(pContext, "chrome.radius", tChrome.fRadius);
	if ( iRet == XUI_OK ) iRet = __xuiCoreSetFloatToken(pContext, "chrome.border_width", tChrome.fBorderWidth);
	if ( iRet == XUI_OK ) iRet = __xuiCoreSetFloatToken(pContext, "chrome.shadow_size", tChrome.fShadowSize);
	if ( iRet != XUI_OK ) {
		(void)xuiStyleEndUpdate(pContext);
		return iRet;
	}
	return xuiStyleEndUpdate(pContext);
}

XUI_API int xuiGetChromeStyle(xui_context pContext, xui_chrome_style_t* pChrome)
{
	if ( !__xuiContextValid(pContext) || (pChrome == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	*pChrome = pContext->tChromeStyle;
	pChrome->iSize = sizeof(*pChrome);
	return XUI_OK;
}

XUI_API int xuiSetDefaultFont(xui_context pContext, xui_font pFont)
{
	if ( !__xuiContextValid(pContext) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pContext->pDefaultFont = pFont;
	pContext->tTheme.pFont = pFont;
	xuiInternalContextBumpGeneration(pContext);
	return XUI_OK;
}

XUI_API xui_font xuiGetDefaultFont(xui_context pContext)
{
	return __xuiContextValid(pContext) ? pContext->pDefaultFont : NULL;
}

static int __xuiCoreReserveFonts(xui_context pContext, int iCapacity)
{
	xui_font_entry_t* pFonts;

	if ( iCapacity <= pContext->iFontCapacity ) {
		return XUI_OK;
	}
	if ( iCapacity < (pContext->iFontCapacity * 2) ) {
		iCapacity = pContext->iFontCapacity * 2;
	}
	if ( pContext->pFonts == pContext->arrInlineFonts ) {
		pFonts = (xui_font_entry_t*)xrtMalloc(sizeof(*pFonts) * (size_t)iCapacity);
		if ( pFonts == NULL ) {
			return XUI_ERROR_OUT_OF_MEMORY;
		}
		memcpy(pFonts, pContext->pFonts, sizeof(*pFonts) * (size_t)pContext->iFontCount);
	} else {
		pFonts = (xui_font_entry_t*)xrtRealloc(pContext->pFonts, sizeof(*pFonts) * (size_t)iCapacity);
		if ( pFonts == NULL ) {
			return XUI_ERROR_OUT_OF_MEMORY;
		}
	}
	pContext->pFonts = pFonts;
	pContext->iFontCapacity = iCapacity;
	return XUI_OK;
}

XUI_API int xuiRegisterFont(xui_context pContext, const char* sName, xui_font pFont)
{
	char* sCopy;
	int i;
	int iRet;

	if ( !__xuiContextValid(pContext) || (sName == NULL) || (sName[0] == '\0') || (pFont == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	for ( i = 0; i < pContext->iFontCount; i++ ) {
		if ( strcmp(pContext->pFonts[i].sName, sName) == 0 ) {
			pContext->pFonts[i].pFont = pFont;
			xuiInternalContextBumpGeneration(pContext);
			return XUI_OK;
		}
	}
	iRet = __xuiCoreReserveFonts(pContext, pContext->iFontCount + 1);
	if ( iRet != XUI_OK ) {
		return iRet;
	}
	sCopy = __xuiCoreStringDuplicate(sName);
	if ( sCopy == NULL ) {
		return XUI_ERROR_OUT_OF_MEMORY;
	}
	pContext->pFonts[pContext->iFontCount].sName = sCopy;
	pContext->pFonts[pContext->iFontCount].pFont = pFont;
	pContext->iFontCount++;
	xuiInternalContextBumpGeneration(pContext);
	return XUI_OK;
}

XUI_API xui_font xuiFindFont(xui_context pContext, const char* sName)
{
	int i;

	if ( !__xuiContextValid(pContext) || (sName == NULL) || (sName[0] == '\0') ) {
		return NULL;
	}
	for ( i = 0; i < pContext->iFontCount; i++ ) {
		if ( strcmp(pContext->pFonts[i].sName, sName) == 0 ) {
			return pContext->pFonts[i].pFont;
		}
	}
	return NULL;
}

XUI_API void xuiClearFonts(xui_context pContext)
{
	if ( !__xuiContextValid(pContext) ) {
		return;
	}
	__xuiContextDestroyFonts(pContext);
	xuiInternalContextBumpGeneration(pContext);
}

XUI_API int xuiInvalidateRect(xui_context pContext, xui_rect_i_t tRect)
{
	if ( !__xuiContextValid(pContext) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	return __xuiContextAddDamage(pContext, tRect);
}

XUI_API int xuiInvalidateAll(xui_context pContext)
{
	if ( !__xuiContextValid(pContext) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pContext->iDamageCount = 0;
	return __xuiContextAddDamage(pContext, __xuiContextFullRect(pContext));
}

XUI_API int xuiHasDamage(xui_context pContext)
{
	if ( !__xuiContextValid(pContext) ) {
		return 0;
	}
	return pContext->iDamageCount > 0;
}

XUI_API int xuiGetDamageRects(xui_context pContext, xui_rect_i_t* pRects, int iCapacity)
{
	int i;
	int iCopyCount;

	if ( !__xuiContextValid(pContext) || (iCapacity < 0) || ((iCapacity > 0) && (pRects == NULL)) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	iCopyCount = pContext->iDamageCount;
	if ( iCopyCount > iCapacity ) {
		iCopyCount = iCapacity;
	}
	for ( i = 0; i < iCopyCount; i++ ) {
		pRects[i] = pContext->pDamage[i];
	}
	return pContext->iDamageCount;
}

XUI_API void xuiClearDamage(xui_context pContext)
{
	if ( !__xuiContextValid(pContext) ) {
		return;
	}
	pContext->iDamageCount = 0;
}

XUI_API int xuiResourceSet(xui_context pContext, xui_resource* ppResource, const xui_resource_desc_t* pDesc)
{
	xui_resource pResource;
	xui_resource pExisting;
	xui_resource pPrev;
	char* sName;

	if ( !__xuiContextValid(pContext) || (pDesc == NULL) || (pDesc->iSize < sizeof(*pDesc)) ||
	     (pDesc->sName == NULL) || (pDesc->sName[0] == '\0') || (pDesc->iKind <= 0) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( ppResource != NULL ) {
		*ppResource = NULL;
	}
	sName = __xuiCoreStringDuplicate(pDesc->sName);
	if ( sName == NULL ) {
		return XUI_ERROR_OUT_OF_MEMORY;
	}
	pPrev = NULL;
	for ( pExisting = pContext->pResources; pExisting != NULL; pExisting = pExisting->pNext ) {
		if ( strcmp(pExisting->sName, pDesc->sName) == 0 ) {
			if ( pPrev != NULL ) {
				pPrev->pNext = pExisting->pNext;
			} else {
				pContext->pResources = pExisting->pNext;
			}
			__xuiResourceDestroyOne(pExisting);
			break;
		}
		pPrev = pExisting;
	}
	pResource = (xui_resource)xrtCalloc(1, sizeof(*pResource));
	if ( pResource == NULL ) {
		xrtFree(sName);
		return XUI_ERROR_OUT_OF_MEMORY;
	}
	pResource->iMagic = XUI_RESOURCE_MAGIC;
	pResource->pContext = pContext;
	pResource->sName = sName;
	pResource->iKind = pDesc->iKind;
	pResource->pHandle = pDesc->pHandle;
	pResource->pUser = pDesc->pUser;
	pResource->onDestroy = pDesc->onDestroy;
	pResource->iGeneration = pContext->iNextResourceGeneration++;
	pResource->iRefCount = 1;
	if ( pContext->iNextResourceGeneration == 0 ) {
		pContext->iNextResourceGeneration = 1;
	}
	pResource->pNext = pContext->pResources;
	pContext->pResources = pResource;
	__xuiContextBumpGeneration(pContext);
	if ( ppResource != NULL ) {
		*ppResource = pResource;
	}
	return XUI_OK;
}

XUI_API xui_resource xuiResourceFind(xui_context pContext, const char* sName)
{
	xui_resource pResource;

	if ( !__xuiContextValid(pContext) || (sName == NULL) || (sName[0] == '\0') ) {
		return NULL;
	}
	for ( pResource = pContext->pResources; pResource != NULL; pResource = pResource->pNext ) {
		if ( strcmp(pResource->sName, sName) == 0 ) {
			return pResource;
		}
	}
	return NULL;
}

XUI_API int xuiResourceRemove(xui_resource pResource)
{
	xui_context pContext;
	xui_resource pScan;
	xui_resource pPrev;

	if ( !__xuiResourceValid(pResource) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pContext = pResource->pContext;
	if ( !__xuiContextValid(pContext) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pPrev = NULL;
	for ( pScan = pContext->pResources; pScan != NULL; pScan = pScan->pNext ) {
		if ( pScan == pResource ) {
			if ( pPrev != NULL ) {
				pPrev->pNext = pScan->pNext;
			} else {
				pContext->pResources = pScan->pNext;
			}
			__xuiResourceRemoveDependencyReferences(pContext, pScan);
			__xuiResourceDestroyOne(pScan);
			__xuiContextBumpGeneration(pContext);
			return XUI_OK;
		}
		pPrev = pScan;
	}
	return XUI_ERROR_INVALID_ARGUMENT;
}

XUI_API int xuiResourceTouch(xui_resource pResource)
{
	xui_context pContext;
	xui_resource pScan;
	xui_resource_dependency_t* pDependency;

	if ( !__xuiResourceValid(pResource) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pContext = pResource->pContext;
	if ( !__xuiContextValid(pContext) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pResource->iGeneration = pContext->iNextResourceGeneration++;
	if ( pContext->iNextResourceGeneration == 0 ) {
		pContext->iNextResourceGeneration = 1;
	}
	for ( pScan = pContext->pResources; pScan != NULL; pScan = pScan->pNext ) {
		if ( pScan == pResource ) {
			continue;
		}
		for ( pDependency = pScan->pDependencies; pDependency != NULL; pDependency = pDependency->pNext ) {
			if ( pDependency->pResource == pResource ) {
				pScan->iGeneration = pContext->iNextResourceGeneration++;
				if ( pContext->iNextResourceGeneration == 0 ) {
					pContext->iNextResourceGeneration = 1;
				}
				break;
			}
		}
	}
	__xuiContextBumpGeneration(pContext);
	return XUI_OK;
}

XUI_API const char* xuiResourceGetName(xui_resource pResource)
{
	return __xuiResourceValid(pResource) ? pResource->sName : NULL;
}

XUI_API int xuiResourceGetKind(xui_resource pResource)
{
	return __xuiResourceValid(pResource) ? pResource->iKind : 0;
}

XUI_API void* xuiResourceGetHandle(xui_resource pResource)
{
	return __xuiResourceValid(pResource) ? pResource->pHandle : NULL;
}

XUI_API uint32_t xuiResourceGetGeneration(xui_resource pResource)
{
	return __xuiResourceValid(pResource) ? pResource->iGeneration : 0;
}

XUI_API int xuiResourceAddRef(xui_resource pResource)
{
	if ( !__xuiResourceValid(pResource) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pResource->iRefCount++;
	if ( pResource->iRefCount <= 0 ) {
		pResource->iRefCount = 1;
		return XUI_ERROR_UNSUPPORTED;
	}
	return XUI_OK;
}

XUI_API int xuiResourceRelease(xui_resource pResource)
{
	if ( !__xuiResourceValid(pResource) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pResource->iRefCount--;
	if ( pResource->iRefCount > 0 ) {
		return XUI_OK;
	}
	return xuiResourceRemove(pResource);
}

XUI_API int xuiResourceGetRefCount(xui_resource pResource)
{
	return __xuiResourceValid(pResource) ? pResource->iRefCount : 0;
}

XUI_API int xuiResourceAddDependency(xui_resource pResource, xui_resource pDependency)
{
	xui_resource_dependency_t* pScan;
	xui_resource_dependency_t* pNewDependency;

	if ( !__xuiResourceValid(pResource) ||
	     !__xuiResourceValid(pDependency) ||
	     (pResource->pContext != pDependency->pContext) ||
	     (pResource == pDependency) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	for ( pScan = pResource->pDependencies; pScan != NULL; pScan = pScan->pNext ) {
		if ( pScan->pResource == pDependency ) {
			return XUI_OK;
		}
	}
	pNewDependency = (xui_resource_dependency_t*)xrtCalloc(1, sizeof(*pNewDependency));
	if ( pNewDependency == NULL ) {
		return XUI_ERROR_OUT_OF_MEMORY;
	}
	pNewDependency->pResource = pDependency;
	pNewDependency->pNext = pResource->pDependencies;
	pResource->pDependencies = pNewDependency;
	return xuiResourceTouch(pResource);
}

XUI_API void xuiResourceClearDependencies(xui_resource pResource)
{
	xui_resource_dependency_t* pDependency;
	xui_resource_dependency_t* pNext;

	if ( !__xuiResourceValid(pResource) ) {
		return;
	}
	pDependency = pResource->pDependencies;
	while ( pDependency != NULL ) {
		pNext = pDependency->pNext;
		xrtFree(pDependency);
		pDependency = pNext;
	}
	pResource->pDependencies = NULL;
	(void)xuiResourceTouch(pResource);
}

XUI_API int xuiResourceGetDependencyCount(xui_resource pResource)
{
	xui_resource_dependency_t* pDependency;
	int iCount;

	if ( !__xuiResourceValid(pResource) ) {
		return 0;
	}
	iCount = 0;
	for ( pDependency = pResource->pDependencies; pDependency != NULL; pDependency = pDependency->pNext ) {
		iCount++;
	}
	return iCount;
}

XUI_API xui_resource xuiResourceGetDependency(xui_resource pResource, int iIndex)
{
	xui_resource_dependency_t* pDependency;
	int i;

	if ( !__xuiResourceValid(pResource) || (iIndex < 0) ) {
		return NULL;
	}
	i = 0;
	for ( pDependency = pResource->pDependencies; pDependency != NULL; pDependency = pDependency->pNext ) {
		if ( i == iIndex ) {
			return pDependency->pResource;
		}
		i++;
	}
	return NULL;
}

XUI_API int xuiPainterBegin(xui_context pContext, xui_surface pTarget, xui_painter* ppPainter)
{
	xui_painter pPainter;
	xui_proxy pProxy;
	int iRet;

	if ( !__xuiContextValid(pContext) || (pTarget == NULL) || (ppPainter == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	*ppPainter = NULL;
	pProxy = xuiInternalContextGetProxy(pContext);
	if ( pProxy == NULL ) {
		return XUI_ERROR_NOT_INITIALIZED;
	}
	pPainter = (xui_painter)xrtCalloc(1, sizeof(*pPainter));
	if ( pPainter == NULL ) {
		return XUI_ERROR_OUT_OF_MEMORY;
	}
	pPainter->iMagic = XUI_PAINTER_MAGIC;
	pPainter->pContext = pContext;
	pPainter->pTarget = pTarget;
	iRet = pProxy->drawBegin(pProxy, &pPainter->pDraw, pTarget);
	if ( iRet != XUI_OK ) {
		pPainter->iMagic = 0;
		xrtFree(pPainter);
		return iRet;
	}
	*ppPainter = pPainter;
	return XUI_OK;
}

XUI_API int xuiPainterEnd(xui_painter pPainter)
{
	xui_proxy pProxy;
	int iRet;

	if ( !__xuiPainterValid(pPainter) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pProxy = xuiInternalContextGetProxy(pPainter->pContext);
	if ( pProxy == NULL ) {
		iRet = XUI_ERROR_NOT_INITIALIZED;
	} else {
		iRet = pProxy->drawEnd(pProxy, pPainter->pDraw);
	}
	pPainter->iMagic = 0;
	xrtFree(pPainter);
	return iRet;
}

XUI_API xui_draw_context xuiPainterGetDrawContext(xui_painter pPainter)
{
	return __xuiPainterValid(pPainter) ? pPainter->pDraw : NULL;
}

XUI_API int xuiPainterClearRect(xui_painter pPainter, xui_rect_t tRect, uint32_t iColor)
{
	xui_proxy pProxy;

	if ( !__xuiPainterValid(pPainter) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pProxy = xuiInternalContextGetProxy(pPainter->pContext);
	tRect = xuiInternalSnapRectOut(tRect);
	return (pProxy != NULL) ? pProxy->drawClearRect(pProxy, pPainter->pDraw, tRect, iColor) : XUI_ERROR_NOT_INITIALIZED;
}

XUI_API int xuiPainterDrawSurface(xui_painter pPainter, xui_surface pSurface, xui_rect_t tSrc, xui_rect_t tDst, uint32_t iColor, uint32_t iFlags)
{
	xui_proxy pProxy;

	if ( !__xuiPainterValid(pPainter) || (pSurface == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pProxy = xuiInternalContextGetProxy(pPainter->pContext);
	tDst = xuiInternalSnapRect(tDst);
	return (pProxy != NULL) ? pProxy->drawSurface(pProxy, pPainter->pDraw, pSurface, tSrc, tDst, iColor, iFlags) : XUI_ERROR_NOT_INITIALIZED;
}

XUI_API int xuiPainterDrawSurfaceQuad(xui_painter pPainter, xui_surface pSurface, const xui_surface_vertex_t* pVertices, uint32_t iFlags)
{
	xui_proxy pProxy;

	if ( !__xuiPainterValid(pPainter) || (pSurface == NULL) || (pVertices == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pProxy = xuiInternalContextGetProxy(pPainter->pContext);
	return (pProxy != NULL) ? pProxy->drawSurfaceQuad(pProxy, pPainter->pDraw, pSurface, pVertices, iFlags) : XUI_ERROR_NOT_INITIALIZED;
}

XUI_API int xuiPainterFillRect(xui_painter pPainter, xui_rect_t tRect, uint32_t iColor)
{
	xui_proxy pProxy;

	if ( !__xuiPainterValid(pPainter) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pProxy = xuiInternalContextGetProxy(pPainter->pContext);
	tRect = xuiInternalSnapRect(tRect);
	return (pProxy != NULL) ? pProxy->drawRectFill(pProxy, pPainter->pDraw, tRect, iColor) : XUI_ERROR_NOT_INITIALIZED;
}

XUI_API int xuiPainterStrokeRect(xui_painter pPainter, xui_rect_t tRect, float fWidth, uint32_t iColor)
{
	xui_proxy pProxy;

	if ( !__xuiPainterValid(pPainter) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pProxy = xuiInternalContextGetProxy(pPainter->pContext);
	tRect = xuiInternalSnapRect(tRect);
	fWidth = xuiInternalSnapSize(fWidth);
	return (pProxy != NULL) ? pProxy->drawRectStroke(pProxy, pPainter->pDraw, tRect, fWidth, iColor) : XUI_ERROR_NOT_INITIALIZED;
}

XUI_API int xuiPainterFillRoundRect(xui_painter pPainter, xui_rect_t tRect, float fRadius, uint32_t iColor)
{
	xui_proxy pProxy;

	if ( !__xuiPainterValid(pPainter) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pProxy = xuiInternalContextGetProxy(pPainter->pContext);
	tRect = xuiInternalSnapRect(tRect);
	fRadius = xuiInternalSnapPixel(fRadius);
	return (pProxy != NULL) ? pProxy->drawRoundRectFill(pProxy, pPainter->pDraw, tRect, fRadius, iColor) : XUI_ERROR_NOT_INITIALIZED;
}

XUI_API int xuiPainterStrokeRoundRect(xui_painter pPainter, xui_rect_t tRect, float fRadius, float fWidth, uint32_t iColor)
{
	xui_proxy pProxy;

	if ( !__xuiPainterValid(pPainter) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pProxy = xuiInternalContextGetProxy(pPainter->pContext);
	fWidth = xuiInternalSnapSize(fWidth);
	tRect = xuiInternalSnapRect(tRect);
	fRadius = xuiInternalSnapPixel(fRadius);
	return (pProxy != NULL) ? pProxy->drawRoundRectStroke(pProxy, pPainter->pDraw, tRect, fRadius, fWidth, iColor) : XUI_ERROR_NOT_INITIALIZED;
}

XUI_API int xuiPainterDrawText(xui_painter pPainter, xui_font pFont, const char* sText, xui_rect_t tRect, uint32_t iColor, uint32_t iFlags)
{
	xui_proxy pProxy;

	if ( !__xuiPainterValid(pPainter) || (pFont == NULL) || (sText == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pProxy = xuiInternalContextGetProxy(pPainter->pContext);
	tRect = xuiInternalSnapRect(tRect);
	return (pProxy != NULL) ? pProxy->drawText(pProxy, pPainter->pDraw, pFont, sText, tRect, iColor, iFlags) : XUI_ERROR_NOT_INITIALIZED;
}

XUI_API int xuiPainterDrawNinePatch(xui_painter pPainter, xui_surface pSurface, xui_rect_t tSrc, xui_rect_t tDst, xui_thickness_t tSlice, uint32_t iColor, uint32_t iFlags)
{
	xui_rect_t arrSrc[9];
	xui_rect_t arrDst[9];
	float arrSrcX[4];
	float arrSrcY[4];
	float arrDstX[4];
	float arrDstY[4];
	float fSrcLeft;
	float fSrcTop;
	float fSrcRight;
	float fSrcBottom;
	float fDstLeft;
	float fDstTop;
	float fDstRight;
	float fDstBottom;
	int x;
	int y;
	int iIndex;
	int iRet;

	if ( !__xuiPainterValid(pPainter) || (pSurface == NULL) ||
	     (tSrc.fW < 0.0f) || (tSrc.fH < 0.0f) || (tDst.fW < 0.0f) || (tDst.fH < 0.0f) ||
	     (tSlice.fLeft < 0.0f) || (tSlice.fTop < 0.0f) || (tSlice.fRight < 0.0f) || (tSlice.fBottom < 0.0f) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	fSrcLeft = tSlice.fLeft;
	fSrcTop = tSlice.fTop;
	fSrcRight = tSlice.fRight;
	fSrcBottom = tSlice.fBottom;
	if ( fSrcLeft + fSrcRight > tSrc.fW ) {
		fSrcRight = tSrc.fW - fSrcLeft;
		if ( fSrcRight < 0.0f ) {
			fSrcRight = 0.0f;
			fSrcLeft = tSrc.fW;
		}
	}
	if ( fSrcTop + fSrcBottom > tSrc.fH ) {
		fSrcBottom = tSrc.fH - fSrcTop;
		if ( fSrcBottom < 0.0f ) {
			fSrcBottom = 0.0f;
			fSrcTop = tSrc.fH;
		}
	}
	fDstLeft = fSrcLeft;
	fDstTop = fSrcTop;
	fDstRight = fSrcRight;
	fDstBottom = fSrcBottom;
	if ( fDstLeft + fDstRight > tDst.fW ) {
		fDstRight = tDst.fW - fDstLeft;
		if ( fDstRight < 0.0f ) {
			fDstRight = 0.0f;
			fDstLeft = tDst.fW;
		}
	}
	if ( fDstTop + fDstBottom > tDst.fH ) {
		fDstBottom = tDst.fH - fDstTop;
		if ( fDstBottom < 0.0f ) {
			fDstBottom = 0.0f;
			fDstTop = tDst.fH;
		}
	}
	arrSrcX[0] = tSrc.fX;
	arrSrcX[1] = tSrc.fX + fSrcLeft;
	arrSrcX[2] = tSrc.fX + tSrc.fW - fSrcRight;
	arrSrcX[3] = tSrc.fX + tSrc.fW;
	arrSrcY[0] = tSrc.fY;
	arrSrcY[1] = tSrc.fY + fSrcTop;
	arrSrcY[2] = tSrc.fY + tSrc.fH - fSrcBottom;
	arrSrcY[3] = tSrc.fY + tSrc.fH;
	arrDstX[0] = tDst.fX;
	arrDstX[1] = tDst.fX + fDstLeft;
	arrDstX[2] = tDst.fX + tDst.fW - fDstRight;
	arrDstX[3] = tDst.fX + tDst.fW;
	arrDstY[0] = tDst.fY;
	arrDstY[1] = tDst.fY + fDstTop;
	arrDstY[2] = tDst.fY + tDst.fH - fDstBottom;
	arrDstY[3] = tDst.fY + tDst.fH;
	for ( y = 0; y < 3; y++ ) {
		for ( x = 0; x < 3; x++ ) {
			iIndex = (y * 3) + x;
			arrSrc[iIndex].fX = arrSrcX[x];
			arrSrc[iIndex].fY = arrSrcY[y];
			arrSrc[iIndex].fW = arrSrcX[x + 1] - arrSrcX[x];
			arrSrc[iIndex].fH = arrSrcY[y + 1] - arrSrcY[y];
			arrDst[iIndex].fX = arrDstX[x];
			arrDst[iIndex].fY = arrDstY[y];
			arrDst[iIndex].fW = arrDstX[x + 1] - arrDstX[x];
			arrDst[iIndex].fH = arrDstY[y + 1] - arrDstY[y];
		}
	}
	for ( iIndex = 0; iIndex < 9; iIndex++ ) {
		if ( (arrSrc[iIndex].fW <= 0.0f) || (arrSrc[iIndex].fH <= 0.0f) ||
		     (arrDst[iIndex].fW <= 0.0f) || (arrDst[iIndex].fH <= 0.0f) ) {
			continue;
		}
		iRet = xuiPainterDrawSurface(pPainter, pSurface, arrSrc[iIndex], arrDst[iIndex], iColor, iFlags);
		if ( iRet != XUI_OK ) {
			return iRet;
		}
	}
	return XUI_OK;
}

static int __xuiCoreEffectiveCachePolicy(xui_widget pWidget)
{
	if ( pWidget->tCachePolicy.iPolicy == XUI_CACHE_POLICY_AUTO ) {
		return (pWidget->iChildCount > 0) ? XUI_CACHE_POLICY_SUBTREE : XUI_CACHE_POLICY_SELF;
	}
	if ( pWidget->tCachePolicy.iPolicy == XUI_CACHE_POLICY_SUBTREE_TILED ) {
		return XUI_CACHE_POLICY_SUBTREE;
	}
	if ( pWidget->tCachePolicy.iPolicy == XUI_CACHE_POLICY_DISPLAY_LIST ) {
		return XUI_CACHE_POLICY_SELF;
	}
	return pWidget->tCachePolicy.iPolicy;
}

static xui_widget_cache_slot_t* __xuiCoreFindCacheSlot(xui_widget pWidget, uint32_t iStateId)
{
	int i;

	if ( pWidget == NULL ) {
		return NULL;
	}
	for ( i = 0; i < pWidget->iCacheCount; i++ ) {
		if ( pWidget->pCacheSlots[i].iStateId == iStateId ) {
			return &pWidget->pCacheSlots[i];
		}
	}
	return NULL;
}

static int __xuiCoreFloatRectValid(xui_rect_t tRect)
{
	return (tRect.fW > 0.0f) && (tRect.fH > 0.0f);
}

static xui_rect_t __xuiCoreRectFromInt(xui_rect_i_t tRect)
{
	return xuiInternalRectFromInt(tRect);
}

static xui_rect_t __xuiCoreIntersectRect(xui_rect_t tA, xui_rect_t tB)
{
	xui_rect_t tRet;
	float fLeft;
	float fTop;
	float fRight;
	float fBottom;

	fLeft = (tA.fX > tB.fX) ? tA.fX : tB.fX;
	fTop = (tA.fY > tB.fY) ? tA.fY : tB.fY;
	fRight = ((tA.fX + tA.fW) < (tB.fX + tB.fW)) ? (tA.fX + tA.fW) : (tB.fX + tB.fW);
	fBottom = ((tA.fY + tA.fH) < (tB.fY + tB.fH)) ? (tA.fY + tA.fH) : (tB.fY + tB.fH);
	tRet.fX = fLeft;
	tRet.fY = fTop;
	tRet.fW = fRight - fLeft;
	tRet.fH = fBottom - fTop;
	if ( tRet.fW < 0.0f ) {
		tRet.fW = 0.0f;
	}
	if ( tRet.fH < 0.0f ) {
		tRet.fH = 0.0f;
	}
	return tRet;
}

static int __xuiCoreClipDamageToTarget(const xui_surface_desc_t* pDesc, xui_rect_i_t* pRect)
{
	int iLeft;
	int iTop;
	int iRight;
	int iBottom;

	iLeft = pRect->iX;
	iTop = pRect->iY;
	iRight = pRect->iX + pRect->iW;
	iBottom = pRect->iY + pRect->iH;
	if ( iLeft < 0 ) {
		iLeft = 0;
	}
	if ( iTop < 0 ) {
		iTop = 0;
	}
	if ( iRight > pDesc->iWidth ) {
		iRight = pDesc->iWidth;
	}
	if ( iBottom > pDesc->iHeight ) {
		iBottom = pDesc->iHeight;
	}
	pRect->iX = iLeft;
	pRect->iY = iTop;
	pRect->iW = iRight - iLeft;
	pRect->iH = iBottom - iTop;
	return __xuiRectValid(*pRect);
}

static int __xuiCoreRectContains(xui_rect_t tOuter, xui_rect_t tInner)
{
	return (tInner.fX >= tOuter.fX) &&
	       (tInner.fY >= tOuter.fY) &&
	       ((tInner.fX + tInner.fW) <= (tOuter.fX + tOuter.fW)) &&
	       ((tInner.fY + tInner.fH) <= (tOuter.fY + tOuter.fH));
}

static int __xuiCoreNodeCoveredBySubtreeCache(const xui_render_node_t* pNode)
{
	xui_widget pParent;
	xui_widget_cache_slot_t* pSlot;
	xui_rect_t tParentRect;

	for ( pParent = pNode->pWidget->pParent; pParent != NULL; pParent = pParent->pParent ) {
		if ( __xuiCoreEffectiveCachePolicy(pParent) != XUI_CACHE_POLICY_SUBTREE ) {
			continue;
		}
		pSlot = __xuiCoreFindCacheSlot(pParent, pParent->iStateId);
		if ( (pSlot == NULL) || (pSlot->pSurface == NULL) ) {
			continue;
		}
		tParentRect = xuiWidgetGetWorldRect(pParent);
		if ( __xuiCoreRectContains(tParentRect, pNode->tPaintRect) ) {
			return 1;
		}
	}
	return 0;
}

static int __xuiCoreDrawRenderNode(xui_context pContext, xui_surface pTarget, const xui_render_node_t* pNode, xui_rect_i_t tDamage)
{
	xui_widget_cache_slot_t* pSlot;
	xui_proxy pProxy;
	xui_rect_t tDamageRect;
	xui_rect_t tClipRect;
	xui_rect_t tSrc;
	xui_rect_t tDst;
	float fScaleX;
	float fScaleY;
	int iPolicy;
	int iRet;

	if ( (pNode == NULL) || (pNode->pWidget == NULL) || !pNode->pWidget->bVisible ) {
		return XUI_OK;
	}
	iPolicy = pNode->iCachePolicy;
	if ( iPolicy == XUI_CACHE_POLICY_NONE ) {
		return XUI_OK;
	}
	pSlot = __xuiCoreFindCacheSlot(pNode->pWidget, pNode->iStateId);
	if ( (pSlot == NULL) || (pSlot->pSurface == NULL) ||
	     (pSlot->iWidth <= 0) || (pSlot->iHeight <= 0) ||
	     (pNode->tWorldRect.fW <= 0.0f) || (pNode->tWorldRect.fH <= 0.0f) ) {
		return XUI_OK;
	}
	tDamageRect = __xuiCoreRectFromInt(tDamage);
	tClipRect = __xuiCoreIntersectRect(pNode->tPaintRect, tDamageRect);
	tClipRect = xuiInternalSnapRect(tClipRect);
	if ( !__xuiCoreFloatRectValid(tClipRect) ) {
		return XUI_OK;
	}
	fScaleX = (float)pSlot->iWidth / pNode->tWorldRect.fW;
	fScaleY = (float)pSlot->iHeight / pNode->tWorldRect.fH;
	tSrc.fX = (tClipRect.fX - pNode->tWorldRect.fX) * fScaleX;
	tSrc.fY = (tClipRect.fY - pNode->tWorldRect.fY) * fScaleY;
	tSrc.fW = tClipRect.fW * fScaleX;
	tSrc.fH = tClipRect.fH * fScaleY;
	tSrc = xuiInternalSnapRect(tSrc);
	tDst = xuiInternalSnapRect(tClipRect);
	pProxy = xuiInternalContextGetProxy(pContext);
	if ( pProxy == NULL ) {
		return XUI_ERROR_NOT_INITIALIZED;
	}
	iRet = pProxy->surfaceDrawTo(pProxy, pTarget, pSlot->pSurface, tSrc, tDst, XUI_COLOR_WHITE, 0);
	if ( iRet == XUI_OK ) {
		pContext->tRenderStats.iDrawnCaches++;
	}
	return iRet;
}

static void __xuiCoreClearRenderDirtyRecursive(xui_widget pWidget)
{
	xui_widget pChild;

	if ( pWidget == NULL ) {
		return;
	}
	pWidget->iDirtyFlags &= ~(XUI_WIDGET_DIRTY_RENDER | XUI_WIDGET_DIRTY_TREE);
	pWidget->iSubtreeDirtyFlags &= ~(XUI_WIDGET_DIRTY_RENDER | XUI_WIDGET_DIRTY_TREE);
	for ( pChild = pWidget->pFirstChild; pChild != NULL; pChild = pChild->pNextSibling ) {
		__xuiCoreClearRenderDirtyRecursive(pChild);
	}
}

static int __xuiCoreComposeRenderTree(xui_context pContext, xui_surface pTarget, const xui_surface_desc_t* pTargetDesc, const xui_rect_i_t* pRects, int iRectCount)
{
	xui_rect_i_t tDamage;
	const xui_render_node_t* pNode;
	int i;
	int n;
	int iRet;

	for ( i = 0; i < iRectCount; i++ ) {
		tDamage = pRects[i];
		if ( !__xuiCoreClipDamageToTarget(pTargetDesc, &tDamage) ) {
			continue;
		}
		for ( n = 0; n < pContext->iRenderNodeCount; n++ ) {
			pNode = &pContext->pRenderNodes[n];
			if ( __xuiCoreNodeCoveredBySubtreeCache(pNode) ) {
				pContext->tRenderStats.iSkippedWidgets++;
				continue;
			}
			iRet = __xuiCoreDrawRenderNode(pContext, pTarget, pNode, tDamage);
			if ( iRet != XUI_OK ) {
				return iRet;
			}
		}
	}
	if ( pContext->pRoot != NULL ) {
		__xuiCoreClearRenderDirtyRecursive(pContext->pRoot);
	}
	if ( pContext->pOverlayRoot != NULL ) {
		__xuiCoreClearRenderDirtyRecursive(pContext->pOverlayRoot);
	}
	return XUI_OK;
}

XUI_API int xuiRender(xui_context pContext, xui_surface pTarget, const xui_rect_i_t* pRects, int iRectCount)
{
	xui_surface_desc_t tDesc;
	xui_rect_i_t tInlineRect;
	const xui_rect_i_t* pRenderRects;
	int iRenderRectCount;
	int i;
	int iRet;

	if ( !__xuiContextValid(pContext) || (pTarget == NULL) || (iRectCount < 0) || ((iRectCount > 0) && (pRects == NULL)) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( !pContext->bHasProxy ) {
		return XUI_ERROR_NOT_INITIALIZED;
	}
	for ( i = 0; i < iRectCount; i++ ) {
		if ( !__xuiRectValid(pRects[i]) ) {
			return XUI_ERROR_INVALID_ARGUMENT;
		}
	}
	memset(&tDesc, 0, sizeof(tDesc));
	iRet = pContext->tProxy.surfaceGetDesc(&pContext->tProxy, pTarget, &tDesc);
	if ( iRet != XUI_OK ) {
		return iRet;
	}
	if ( (tDesc.iKind != XUI_SURFACE_KIND_TEXTURE) ||
	     (tDesc.iFormat != XUI_SURFACE_FORMAT_RGBA8) ||
	     ((tDesc.iFlags & XUI_SURFACE_USAGE_TARGET) == 0) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( pContext->pRoot != NULL ) {
		iRet = xuiRenderPrepare(pContext);
		if ( iRet != XUI_OK ) {
			return iRet;
		}
		iRet = xuiBuildRenderTree(pContext);
		if ( iRet != XUI_OK ) {
			return iRet;
		}
		if ( iRectCount > 0 ) {
			pRenderRects = pRects;
			iRenderRectCount = iRectCount;
		} else if ( pContext->iDamageCount > 0 ) {
			pRenderRects = pContext->pDamage;
			iRenderRectCount = pContext->iDamageCount;
		} else {
			tInlineRect.iX = 0;
			tInlineRect.iY = 0;
			tInlineRect.iW = tDesc.iWidth;
			tInlineRect.iH = tDesc.iHeight;
			pRenderRects = &tInlineRect;
			iRenderRectCount = 1;
		}
		iRet = __xuiCoreComposeRenderTree(pContext, pTarget, &tDesc, pRenderRects, iRenderRectCount);
		if ( iRet != XUI_OK ) {
			return iRet;
		}
	}
	pContext->iDamageCount = 0;
	return XUI_OK;
}
