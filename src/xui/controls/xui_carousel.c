#include "xui_config.h"
#if XUI_ENABLE_CAROUSEL
#include "xui_internal.h"
#include "xui_pointer_press_internal.h"

#include <float.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#define XUI_CAROUSEL_MAX_GEOMETRY_METRIC \
	(XUI_LAYOUT_UNBOUNDED / (2.0f * (float)XUI_CAROUSEL_MAX_PAGES))

typedef struct xui_carousel_data_t {
	xui_carousel_change_proc onChange;
	void* pChangeUser;
	xui_widget arrPages[XUI_CAROUSEL_MAX_PAGES];
	xui_widget pOverlay;
	xui_font pFont;
	int iPageCount;
	int iCurrent;
	int bLoop;
	int bAutoPlay;
	int bShowIndicators;
	int bShowArrowsOnHover;
	int iHoverIndicator;
	int iActiveIndicator;
	int iHoverArrow;
	int iActiveArrow;
	int iChangeCount;
	int bHover;
	xui_pointer_owner_t tPointerOwner;
	float fAutoInterval;
	float fAutoElapsed;
	float fArrowSize;
	float fIndicatorSize;
	float fIndicatorGap;
	float fIndicatorBottom;
	xui_rect_t tContentRect;
	xui_rect_t tPrevArrowRect;
	xui_rect_t tNextArrowRect;
	xui_rect_t tIndicatorGroupRect;
	xui_rect_t arrIndicatorRects[XUI_CAROUSEL_MAX_PAGES];
	uint32_t iBackgroundColor;
	uint32_t iArrowColor;
	uint32_t iArrowHoverColor;
	uint32_t iArrowTextColor;
	uint32_t iIndicatorColor;
	uint32_t iChromeStyleVersion;
	uint32_t iIndicatorActiveColor;
	uint32_t iIndicatorHoverColor;
	uint32_t iFocusColor;
	char sAccessibleValue[32];
	char arrAccessiblePageNames[XUI_CAROUSEL_MAX_PAGES][24];
} xui_carousel_data_t;

typedef struct xui_carousel_style_t {
	int bAutoPlay;
	int bShowIndicators;
	int bShowArrowsOnHover;
	float fAutoInterval;
	float fArrowSize;
	float fIndicatorSize;
	float fIndicatorGap;
	float fIndicatorBottom;
	uint32_t iBackgroundColor;
	uint32_t iArrowColor;
	uint32_t iArrowHoverColor;
	uint32_t iArrowTextColor;
	uint32_t iIndicatorColor;
	uint32_t iIndicatorBackgroundColor;
	uint32_t iIndicatorActiveColor;
	uint32_t iIndicatorHoverColor;
	uint32_t iFocusColor;
} xui_carousel_style_t;

static int __xuiCarouselArrowAvailable(const xui_carousel_data_t* pData, int iArrow);

static int __xuiCarouselMetricValid(float fValue, float fMax)
{
	return isfinite(fValue) && (fValue >= 0.0f) && (fValue <= fMax);
}

static int __xuiCarouselDescValid(const xui_carousel_desc_t* pDesc)
{
	if ( pDesc == NULL ) {
		return 1;
	}
	if ( (pDesc->iSize != 0) && (pDesc->iSize < sizeof(*pDesc)) ) {
		return 0;
	}
	if ( (pDesc->iPageCount < 0) || (pDesc->iPageCount > XUI_CAROUSEL_MAX_PAGES) ) {
		return 0;
	}
	if ( !__xuiCarouselMetricValid(pDesc->fAutoInterval, FLT_MAX) ||
	     !__xuiCarouselMetricValid(pDesc->fArrowSize,
		XUI_CAROUSEL_MAX_GEOMETRY_METRIC) ||
	     !__xuiCarouselMetricValid(pDesc->fIndicatorSize,
		XUI_CAROUSEL_MAX_GEOMETRY_METRIC) ||
	     !__xuiCarouselMetricValid(pDesc->fIndicatorGap,
		XUI_CAROUSEL_MAX_GEOMETRY_METRIC) ||
	     !__xuiCarouselMetricValid(pDesc->fIndicatorBottom,
		XUI_CAROUSEL_MAX_GEOMETRY_METRIC) ) {
		return 0;
	}
	return 1;
}

static int __xuiCarouselAlpha(uint32_t iColor)
{
	return (int)(iColor & 0xffu);
}

static float __xuiCarouselMaxFloat(float fA, float fB)
{
	return (fA > fB) ? fA : fB;
}

static int __xuiCarouselClampInt(int iValue, int iMin, int iMax)
{
	if ( iValue < iMin ) return iMin;
	if ( iValue > iMax ) return iMax;
	return iValue;
}

static int __xuiCarouselRectContains(xui_rect_t tRect, float fX, float fY)
{
	return (tRect.fW > 0.0f) &&
	       (tRect.fH > 0.0f) &&
	       (fX >= tRect.fX) &&
	       (fY >= tRect.fY) &&
	       (fX < (tRect.fX + tRect.fW)) &&
	       (fY < (tRect.fY + tRect.fH));
}

static xui_rect_t __xuiCarouselMinHitRect(xui_rect_t tRect, float fMinSize)
{
	float fExtra;

	if ( tRect.fW < fMinSize ) {
		fExtra = (fMinSize - tRect.fW) * 0.5f;
		tRect.fX -= fExtra;
		tRect.fW = fMinSize;
	}
	if ( tRect.fH < fMinSize ) {
		fExtra = (fMinSize - tRect.fH) * 0.5f;
		tRect.fY -= fExtra;
		tRect.fH = fMinSize;
	}
	return tRect;
}

static int __xuiCarouselContainsWidget(xui_widget pCarousel, xui_widget pWidget)
{
	for ( ; pWidget != NULL; pWidget = xuiWidgetGetParent(pWidget) ) {
		if ( pWidget == pCarousel ) return 1;
	}
	return 0;
}

static xui_carousel_data_t* __xuiCarouselGetData(xui_widget pWidget)
{
	xui_context pContext;
	xui_widget_type pType;

	if ( !xuiInternalWidgetIsValid(pWidget) ) {
		return NULL;
	}
	pContext = xuiWidgetGetContext(pWidget);
	if ( !xuiInternalContextIsValid(pContext) ) {
		return NULL;
	}
	pType = xuiWidgetFindType(pContext, "carousel");
	if ( (pType == NULL) || !xuiWidgetIsType(pWidget, pType) ) {
		return NULL;
	}
	return (xui_carousel_data_t*)xuiWidgetGetTypeData(pWidget);
}

static int __xuiCarouselWidgetAlive(xui_widget pWidget, xui_carousel_data_t* pData)
{
	xui_context pContext;

	if ( !xuiInternalWidgetIsValid(pWidget) ) return 0;
	pContext = xuiWidgetGetContext(pWidget);
	return xuiInternalContextIsValid(pContext) && !xuiInternalContextDestroyPending(pContext) &&
		__xuiCarouselGetData(pWidget) == pData;
}

static int __xuiCarouselStyleColor(xui_widget pWidget, const char* sName, uint32_t* pColor)
{
	xui_style_property_t tProperty;
	int iRet;

	memset(&tProperty, 0, sizeof(tProperty));
	tProperty.iSize = sizeof(tProperty);
	iRet = xuiWidgetGetResolvedStyleProperty(pWidget, sName, &tProperty);
	if ( (iRet == XUI_OK) && (tProperty.tValue.iType == XUI_STYLE_VALUE_COLOR) ) {
		*pColor = tProperty.tValue.iColor;
		return 1;
	}
	return 0;
}

static int __xuiCarouselStyleFloat(xui_widget pWidget, const char* sName,
	float fMax, float* pValue)
{
	xui_style_property_t tProperty;
	int iRet;

	memset(&tProperty, 0, sizeof(tProperty));
	tProperty.iSize = sizeof(tProperty);
	iRet = xuiWidgetGetResolvedStyleProperty(pWidget, sName, &tProperty);
	if ( (iRet == XUI_OK) && (tProperty.tValue.iType == XUI_STYLE_VALUE_FLOAT) &&
	     __xuiCarouselMetricValid(tProperty.tValue.fFloat, fMax) ) {
		*pValue = tProperty.tValue.fFloat;
		return 1;
	}
	return 0;
}

static int __xuiCarouselStyleInt(xui_widget pWidget, const char* sName, int* pValue)
{
	xui_style_property_t tProperty;
	int iRet;

	memset(&tProperty, 0, sizeof(tProperty));
	tProperty.iSize = sizeof(tProperty);
	iRet = xuiWidgetGetResolvedStyleProperty(pWidget, sName, &tProperty);
	if ( (iRet == XUI_OK) &&
	     ((tProperty.tValue.iType == XUI_STYLE_VALUE_INT) || (tProperty.tValue.iType == XUI_STYLE_VALUE_BOOL)) ) {
		*pValue = tProperty.tValue.iInt;
		return 1;
	}
	return 0;
}

static void __xuiCarouselResolve(xui_widget pWidget, const xui_carousel_data_t* pData, xui_carousel_style_t* pResolved)
{
	int iValue;

	memset(pResolved, 0, sizeof(*pResolved));
	pResolved->bAutoPlay = pData->bAutoPlay;
	pResolved->bShowIndicators = pData->bShowIndicators;
	pResolved->bShowArrowsOnHover = pData->bShowArrowsOnHover;
	pResolved->fAutoInterval = pData->fAutoInterval;
	pResolved->fArrowSize = pData->fArrowSize;
	pResolved->fIndicatorSize = pData->fIndicatorSize;
	pResolved->fIndicatorGap = pData->fIndicatorGap;
	pResolved->fIndicatorBottom = pData->fIndicatorBottom;
	pResolved->iBackgroundColor = pData->iBackgroundColor;
	pResolved->iArrowColor = pData->iArrowColor;
	pResolved->iArrowHoverColor = pData->iArrowHoverColor;
	pResolved->iArrowTextColor = pData->iArrowTextColor;
	pResolved->iIndicatorColor = pData->iIndicatorColor;
	pResolved->iIndicatorActiveColor = pData->iIndicatorActiveColor;
	pResolved->iIndicatorHoverColor = pData->iIndicatorHoverColor;
	pResolved->iFocusColor = pData->iFocusColor;
	(void)__xuiCarouselStyleColor(pWidget, "carousel.background.color", &pResolved->iBackgroundColor);
	(void)__xuiCarouselStyleColor(pWidget, "carousel.arrow.color", &pResolved->iArrowColor);
	(void)__xuiCarouselStyleColor(pWidget, "carousel.arrow.hover_color", &pResolved->iArrowHoverColor);
	(void)__xuiCarouselStyleColor(pWidget, "carousel.arrow.text_color", &pResolved->iArrowTextColor);
	(void)__xuiCarouselStyleColor(pWidget, "carousel.indicator.color", &pResolved->iIndicatorColor);
	pResolved->iIndicatorBackgroundColor = XUI_COLOR_RGBA(0, 0, 0, 62);
	(void)__xuiCarouselStyleColor(pWidget, "carousel.indicator.background_color", &pResolved->iIndicatorBackgroundColor);
	(void)__xuiCarouselStyleColor(pWidget, "carousel.indicator.active_color", &pResolved->iIndicatorActiveColor);
	(void)__xuiCarouselStyleColor(pWidget, "carousel.indicator.hover_color", &pResolved->iIndicatorHoverColor);
	(void)__xuiCarouselStyleColor(pWidget, "carousel.focus.color", &pResolved->iFocusColor);
	(void)__xuiCarouselStyleFloat(pWidget, "carousel.arrow.size",
		XUI_CAROUSEL_MAX_GEOMETRY_METRIC, &pResolved->fArrowSize);
	(void)__xuiCarouselStyleFloat(pWidget, "carousel.indicator.size",
		XUI_CAROUSEL_MAX_GEOMETRY_METRIC, &pResolved->fIndicatorSize);
	(void)__xuiCarouselStyleFloat(pWidget, "carousel.indicator.gap",
		XUI_CAROUSEL_MAX_GEOMETRY_METRIC, &pResolved->fIndicatorGap);
	(void)__xuiCarouselStyleFloat(pWidget, "carousel.indicator.bottom",
		XUI_CAROUSEL_MAX_GEOMETRY_METRIC, &pResolved->fIndicatorBottom);
	(void)__xuiCarouselStyleFloat(pWidget, "carousel.auto.interval", FLT_MAX,
		&pResolved->fAutoInterval);
	iValue = pResolved->bAutoPlay;
	if ( __xuiCarouselStyleInt(pWidget, "carousel.auto.enabled", &iValue) ) pResolved->bAutoPlay = iValue ? 1 : 0;
	iValue = pResolved->bShowIndicators;
	if ( __xuiCarouselStyleInt(pWidget, "carousel.indicator.visible", &iValue) ) pResolved->bShowIndicators = iValue ? 1 : 0;
	iValue = pResolved->bShowArrowsOnHover;
	if ( __xuiCarouselStyleInt(pWidget, "carousel.arrow.hover_only", &iValue) ) pResolved->bShowArrowsOnHover = iValue ? 1 : 0;
	if ( pResolved->fArrowSize < 16.0f ) pResolved->fArrowSize = 16.0f;
	if ( pResolved->fIndicatorSize < 4.0f ) pResolved->fIndicatorSize = 4.0f;
	if ( pResolved->fIndicatorGap < 0.0f ) pResolved->fIndicatorGap = 0.0f;
	if ( pResolved->fIndicatorBottom < 0.0f ) pResolved->fIndicatorBottom = 0.0f;
	if ( pResolved->fAutoInterval < 0.0f ) pResolved->fAutoInterval = 0.0f;
	if ( pResolved->bAutoPlay && pResolved->fAutoInterval <= 0.0f ) {
		pResolved->fAutoInterval = 3.0f;
	}
}

static int __xuiCarouselInvalidate(xui_widget pWidget, xui_carousel_data_t* pData, uint32_t iFlags)
{
	int iRet;

	iRet = xuiWidgetInvalidate(pWidget, iFlags);
	if ( (iRet == XUI_OK) && (pData != NULL) && (pData->pOverlay != NULL) ) {
		iRet = xuiWidgetInvalidate(pData->pOverlay, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	}
	return iRet;
}

static void __xuiCarouselClearRects(xui_carousel_data_t* pData)
{
	int i;

	if ( pData == NULL ) return;
	pData->tPrevArrowRect = (xui_rect_t){0.0f, 0.0f, 0.0f, 0.0f};
	pData->tNextArrowRect = (xui_rect_t){0.0f, 0.0f, 0.0f, 0.0f};
	pData->tIndicatorGroupRect = (xui_rect_t){0.0f, 0.0f, 0.0f, 0.0f};
	for ( i = 0; i < XUI_CAROUSEL_MAX_PAGES; i++ ) {
		pData->arrIndicatorRects[i] = (xui_rect_t){0.0f, 0.0f, 0.0f, 0.0f};
	}
}

static void __xuiCarouselComputeRects(xui_carousel_data_t* pData,
	const xui_carousel_style_t* pResolved, xui_rect_t tContent)
{
	float fArrow;
	float fIndicator;
	float fGap;
	float fGroupW;
	float fGroupH;
	float fX;
	float fY;
	int i;

	if ( (pData == NULL) || (pResolved == NULL) ) return;
	pData->tContentRect = tContent;
	__xuiCarouselClearRects(pData);
	if ( (tContent.fW <= 0.0f) || (tContent.fH <= 0.0f) ) return;

	fArrow = __xuiCarouselMaxFloat(16.0f, pResolved->fArrowSize);
	pData->tPrevArrowRect = xuiInternalSnapRect((xui_rect_t){
		tContent.fX + 18.0f,
		tContent.fY + (tContent.fH - fArrow) * 0.5f,
		fArrow,
		fArrow
	});
	pData->tNextArrowRect = xuiInternalSnapRect((xui_rect_t){
		tContent.fX + tContent.fW - fArrow - 18.0f,
		tContent.fY + (tContent.fH - fArrow) * 0.5f,
		fArrow,
		fArrow
	});
	if ( pResolved->bShowIndicators && (pData->iPageCount > 0) ) {
		fIndicator = __xuiCarouselMaxFloat(4.0f, pResolved->fIndicatorSize);
		fGap = __xuiCarouselMaxFloat(0.0f, pResolved->fIndicatorGap);
		fGroupW = ((float)pData->iPageCount * fIndicator) + ((float)(pData->iPageCount - 1) * fGap) + 16.0f;
		fGroupH = fIndicator + 10.0f;
		fX = tContent.fX + (tContent.fW - fGroupW) * 0.5f;
		fY = tContent.fY + tContent.fH - fGroupH - pResolved->fIndicatorBottom;
		pData->tIndicatorGroupRect = xuiInternalSnapRect((xui_rect_t){fX, fY, fGroupW, fGroupH});
		fX += 8.0f;
		fY += (fGroupH - fIndicator) * 0.5f;
		for ( i = 0; i < pData->iPageCount; i++ ) {
			pData->arrIndicatorRects[i] = xuiInternalSnapRect((xui_rect_t){fX, fY, fIndicator, fIndicator});
			fX += fIndicator + fGap;
		}
	}
}

static int __xuiCarouselDrawFill(xui_proxy pProxy, xui_draw_context pDraw, xui_rect_t tRect, uint32_t iColor)
{
	if ( (tRect.fW <= 0.0f) || (tRect.fH <= 0.0f) || (__xuiCarouselAlpha(iColor) == 0) ) {
		return XUI_OK;
	}
	if ( (pProxy != NULL) && (pProxy->drawRectFill != NULL) && (pDraw != NULL) ) {
		return pProxy->drawRectFill(pProxy, pDraw, xuiInternalSnapRect(tRect), iColor);
	}
	return XUI_ERROR_NOT_INITIALIZED;
}

static int __xuiCarouselDrawStroke(xui_proxy pProxy, xui_draw_context pDraw, xui_rect_t tRect, float fWidth, uint32_t iColor)
{
	if ( (tRect.fW <= 0.0f) || (tRect.fH <= 0.0f) || (fWidth <= 0.0f) || (__xuiCarouselAlpha(iColor) == 0) ) {
		return XUI_OK;
	}
	if ( (pProxy != NULL) && (pDraw != NULL) ) {
		if ( pProxy->drawRectStroke != NULL ) {
			return pProxy->drawRectStroke(pProxy, pDraw, xuiInternalSnapRect(tRect), fWidth, iColor);
		}
	}
	return XUI_ERROR_NOT_INITIALIZED;
}

static int __xuiCarouselDrawCircle(xui_proxy pProxy, xui_draw_context pDraw, float fX, float fY, float fRadius, uint32_t iColor)
{
	if ( (fRadius <= 0.0f) || (__xuiCarouselAlpha(iColor) == 0) ) {
		return XUI_OK;
	}
	if ( (pProxy != NULL) && (pProxy->drawCircleFill != NULL) && (pDraw != NULL) ) {
		return pProxy->drawCircleFill(pProxy, pDraw, xuiInternalSnapPixel(fX), xuiInternalSnapPixel(fY), fRadius, iColor);
	}
	return __xuiCarouselDrawFill(pProxy, pDraw,
		(xui_rect_t){fX - fRadius, fY - fRadius, fRadius * 2.0f, fRadius * 2.0f},
		iColor);
}

static int __xuiCarouselDrawLine(xui_proxy pProxy, xui_draw_context pDraw, float fX0, float fY0, float fX1, float fY1, float fWidth, uint32_t iColor)
{
	if ( __xuiCarouselAlpha(iColor) == 0 ) {
		return XUI_OK;
	}
	if ( (pProxy != NULL) && (pProxy->drawLine != NULL) && (pDraw != NULL) ) {
		return pProxy->drawLine(pProxy, pDraw,
			xuiInternalSnapPixel(fX0), xuiInternalSnapPixel(fY0),
			xuiInternalSnapPixel(fX1), xuiInternalSnapPixel(fY1),
			fWidth, iColor);
	}
	return XUI_ERROR_NOT_INITIALIZED;
}

static int __xuiCarouselDrawArrow(xui_proxy pProxy, xui_draw_context pDraw, xui_rect_t tRect, int iArrow, uint32_t iFill, uint32_t iLine)
{
	float fCX;
	float fCY;
	float fS;
	int iRet;

	if ( (tRect.fW <= 0.0f) || (tRect.fH <= 0.0f) ) return XUI_OK;
	fCX = tRect.fX + tRect.fW * 0.5f;
	fCY = tRect.fY + tRect.fH * 0.5f;
	fS = tRect.fW * 0.22f;
	iRet = __xuiCarouselDrawCircle(pProxy, pDraw, fCX, fCY, tRect.fW * 0.5f, iFill);
	if ( iRet != XUI_OK ) return iRet;
	if ( iArrow == XUI_CAROUSEL_ARROW_PREV ) {
		iRet = __xuiCarouselDrawLine(pProxy, pDraw, fCX + fS * 0.45f, fCY - fS, fCX - fS * 0.45f, fCY, 2.0f, iLine);
		if ( iRet == XUI_OK ) iRet = __xuiCarouselDrawLine(pProxy, pDraw, fCX - fS * 0.45f, fCY, fCX + fS * 0.45f, fCY + fS, 2.0f, iLine);
	} else {
		iRet = __xuiCarouselDrawLine(pProxy, pDraw, fCX - fS * 0.45f, fCY - fS, fCX + fS * 0.45f, fCY, 2.0f, iLine);
		if ( iRet == XUI_OK ) iRet = __xuiCarouselDrawLine(pProxy, pDraw, fCX + fS * 0.45f, fCY, fCX - fS * 0.45f, fCY + fS, 2.0f, iLine);
	}
	return iRet;
}

static int __xuiCarouselBaseRender(xui_widget pWidget, xui_draw_context pDraw, uint32_t iStateId, void* pUser)
{
	xui_carousel_data_t* pData;
	xui_carousel_style_t tResolved;
	xui_proxy pProxy;

	(void)iStateId;
	(void)pUser;
	pData = __xuiCarouselGetData(pWidget);
	if ( (pData == NULL) || (pDraw == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	pProxy = xuiInternalContextGetProxy(xuiWidgetGetContext(pWidget));
	if ( pProxy == NULL ) return XUI_ERROR_NOT_INITIALIZED;
	__xuiCarouselResolve(pWidget, pData, &tResolved);
	return __xuiCarouselDrawFill(pProxy, pDraw, xuiWidgetGetContentRect(pWidget), tResolved.iBackgroundColor);
}

static int __xuiCarouselOverlayRender(xui_widget pOverlay, xui_draw_context pDraw, uint32_t iStateId, void* pUser)
{
	xui_widget pCarousel;
	xui_carousel_data_t* pData;
	xui_carousel_style_t tResolved;
	xui_proxy pProxy;
	uint32_t iFill;
	float fCX;
	float fCY;
	float fRadius;
	xui_rect_t tFocusRect;
	int i;
	int iRet;
	int bShowArrows;

	(void)iStateId;
	pCarousel = (xui_widget)pUser;
	pData = __xuiCarouselGetData(pCarousel);
	if ( (pOverlay == NULL) || (pData == NULL) || (pDraw == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	pProxy = xuiInternalContextGetProxy(xuiWidgetGetContext(pCarousel));
	if ( pProxy == NULL ) return XUI_ERROR_NOT_INITIALIZED;
	__xuiCarouselResolve(pCarousel, pData, &tResolved);
	__xuiCarouselComputeRects(pData, &tResolved, xuiWidgetGetContentRect(pCarousel));

	iRet = XUI_OK;
	bShowArrows = !tResolved.bShowArrowsOnHover || pData->bHover || (pData->iActiveArrow != XUI_CAROUSEL_ARROW_NONE);
	if ( bShowArrows && (pData->iPageCount > 1) ) {
		if ( __xuiCarouselArrowAvailable(pData, XUI_CAROUSEL_ARROW_PREV) ) {
			iFill = pData->iHoverArrow == XUI_CAROUSEL_ARROW_PREV ?
				tResolved.iArrowHoverColor : tResolved.iArrowColor;
			iRet = __xuiCarouselDrawArrow(pProxy, pDraw, pData->tPrevArrowRect, XUI_CAROUSEL_ARROW_PREV, iFill, tResolved.iArrowTextColor);
			if ( iRet != XUI_OK ) return iRet;
		}
		if ( __xuiCarouselArrowAvailable(pData, XUI_CAROUSEL_ARROW_NEXT) ) {
			iFill = pData->iHoverArrow == XUI_CAROUSEL_ARROW_NEXT ?
				tResolved.iArrowHoverColor : tResolved.iArrowColor;
			iRet = __xuiCarouselDrawArrow(pProxy, pDraw, pData->tNextArrowRect, XUI_CAROUSEL_ARROW_NEXT, iFill, tResolved.iArrowTextColor);
			if ( iRet != XUI_OK ) return iRet;
		}
	}
	if ( tResolved.bShowIndicators && (pData->iPageCount > 1) ) {
		iRet = __xuiCarouselDrawFill(pProxy, pDraw, pData->tIndicatorGroupRect,
			tResolved.iIndicatorBackgroundColor);
		if ( iRet != XUI_OK ) return iRet;
		for ( i = 0; i < pData->iPageCount; i++ ) {
			fCX = pData->arrIndicatorRects[i].fX + pData->arrIndicatorRects[i].fW * 0.5f;
			fCY = pData->arrIndicatorRects[i].fY + pData->arrIndicatorRects[i].fH * 0.5f;
			fRadius = pData->arrIndicatorRects[i].fW * 0.5f;
			if ( i == pData->iCurrent ) {
				iFill = tResolved.iIndicatorActiveColor;
			} else if ( i == pData->iHoverIndicator ) {
				iFill = tResolved.iIndicatorHoverColor;
			} else {
				iFill = tResolved.iIndicatorColor;
			}
			iRet = __xuiCarouselDrawCircle(pProxy, pDraw, fCX, fCY, fRadius, iFill);
			if ( iRet != XUI_OK ) return iRet;
		}
	}
	if ( (xuiGetFocusWidget(xuiWidgetGetContext(pCarousel)) == pCarousel) && (tResolved.iFocusColor & 0xffu) != 0u ) {
		tFocusRect = xuiWidgetGetContentRect(pOverlay);
		tFocusRect.fX += 0.5f;
		tFocusRect.fY += 0.5f;
		tFocusRect.fW -= 1.0f;
		tFocusRect.fH -= 1.0f;
		iRet = __xuiCarouselDrawStroke(pProxy, pDraw, tFocusRect, 1.0f, tResolved.iFocusColor);
	}
	return iRet;
}

static int __xuiCarouselConfigurePage(xui_widget pPage)
{
	int iRet;

	if ( pPage == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	iRet = xuiWidgetSetLayoutType(pPage, XUI_LAYOUT_MANUAL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetFlowMode(pPage, XUI_FLOW_BLOCK);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetSizeMode(pPage, XUI_SIZE_FILL, XUI_SIZE_FILL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetAlign(pPage, XUI_ALIGN_STRETCH, XUI_ALIGN_STRETCH);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetOverflow(pPage, XUI_OVERFLOW_CLIP);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetFocusable(pPage, 0);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetTabStop(pPage, 0);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetHitTestVisible(pPage, 1);
	return iRet;
}

static int __xuiCarouselConfigureOverlay(xui_widget pOverlay, xui_widget pCarousel)
{
	xui_cache_policy_t tPolicy;
	int iRet;

	if ( (pOverlay == NULL) || (pCarousel == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	memset(&tPolicy, 0, sizeof(tPolicy));
	tPolicy.iSize = sizeof(tPolicy);
	tPolicy.iPolicy = XUI_CACHE_POLICY_SELF;
	tPolicy.iFlags = XUI_CACHE_CLEAR_ON_UPDATE | XUI_CACHE_UPDATE_ALL_STATES;
	tPolicy.iClearColor = XUI_COLOR_RGBA(0, 0, 0, 0);
	iRet = xuiWidgetSetCachePolicy(pOverlay, &tPolicy);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetCacheRenderCallback(pOverlay, __xuiCarouselOverlayRender, pCarousel);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetLayoutType(pOverlay, XUI_LAYOUT_MANUAL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetFlowMode(pOverlay, XUI_FLOW_BLOCK);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetSizeMode(pOverlay, XUI_SIZE_FILL, XUI_SIZE_FILL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetAlign(pOverlay, XUI_ALIGN_STRETCH, XUI_ALIGN_STRETCH);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetOverflow(pOverlay, XUI_OVERFLOW_VISIBLE);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetFocusable(pOverlay, 0);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetTabStop(pOverlay, 0);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetHitTestVisible(pOverlay, 0);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetLayer(pOverlay, XUI_LAYER_NORMAL, 10);
	return iRet;
}

static int __xuiCarouselEnsurePage(xui_widget pCarousel, xui_carousel_data_t* pData, int iIndex)
{
	xui_widget pPage;
	int iRet;

	if ( (pCarousel == NULL) || (pData == NULL) || (iIndex < 0) || (iIndex >= XUI_CAROUSEL_MAX_PAGES) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( pData->arrPages[iIndex] != NULL ) {
		return XUI_OK;
	}
	iRet = xuiWidgetCreate(xuiWidgetGetContext(pCarousel), &pPage);
	if ( iRet != XUI_OK ) return iRet;
	iRet = __xuiCarouselConfigurePage(pPage);
	if ( iRet == XUI_OK ) iRet = xuiWidgetAddChild(pCarousel, pPage);
	if ( iRet != XUI_OK ) {
		xuiWidgetDestroy(pPage);
		return iRet;
	}
	iRet = xuiWidgetSetVisible(pPage, 0);
	if ( iRet != XUI_OK ) {
		xuiWidgetDestroy(pPage);
		return iRet;
	}
	pData->arrPages[iIndex] = pPage;
	return XUI_OK;
}

static int __xuiCarouselSyncPages(xui_widget pCarousel, xui_carousel_data_t* pData)
{
	int i;
	int iRet;

	if ( (pCarousel == NULL) || (pData == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	pData->iCurrent = __xuiCarouselClampInt(pData->iCurrent, 0, pData->iPageCount - 1);
	for ( i = 0; i < XUI_CAROUSEL_MAX_PAGES; i++ ) {
		if ( pData->arrPages[i] == NULL ) continue;
		iRet = xuiWidgetSetVisible(pData->arrPages[i], (i < pData->iPageCount) && (i == pData->iCurrent));
		if ( iRet != XUI_OK ) return iRet;
		iRet = xuiWidgetSetEnabled(pData->arrPages[i], (i < pData->iPageCount));
		if ( iRet != XUI_OK ) return iRet;
	}
	if ( pData->pOverlay != NULL ) {
		iRet = xuiWidgetSetVisible(pData->pOverlay, 1);
		if ( iRet != XUI_OK ) return iRet;
	}
	return __xuiCarouselInvalidate(pCarousel, pData, XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
}

static int __xuiCarouselSetPageCountInternal(xui_widget pCarousel, xui_carousel_data_t* pData, int iPageCount)
{
	xui_widget arrOldPages[XUI_CAROUSEL_MAX_PAGES];
	int i;
	int iRet;
	int iOldPageCount;
	int iOldCurrent;
	int iOldChangeCount;
	int iOldHoverIndicator;
	int iOldHoverArrow;
	int iOldActiveIndicator;
	float fOldAutoElapsed;
	int iNewCurrent;

	if ( (pCarousel == NULL) || (pData == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( iPageCount < 1 ) iPageCount = 1;
	if ( iPageCount > XUI_CAROUSEL_MAX_PAGES ) return XUI_ERROR_INVALID_ARGUMENT;
	memcpy(arrOldPages, pData->arrPages, sizeof(arrOldPages));
	iOldPageCount = pData->iPageCount;
	iOldCurrent = pData->iCurrent;
	iOldChangeCount = pData->iChangeCount;
	iOldHoverIndicator = pData->iHoverIndicator;
	iOldHoverArrow = pData->iHoverArrow;
	iOldActiveIndicator = pData->iActiveIndicator;
	fOldAutoElapsed = pData->fAutoElapsed;
	for ( i = 0; i < iPageCount; i++ ) {
		iRet = __xuiCarouselEnsurePage(pCarousel, pData, i);
		if ( iRet != XUI_OK ) goto rollback;
	}
	pData->iPageCount = iPageCount;
	iNewCurrent = __xuiCarouselClampInt(pData->iCurrent, 0, iPageCount - 1);
	pData->iCurrent = iNewCurrent;
	pData->iHoverIndicator = -1;
	pData->iHoverArrow = XUI_CAROUSEL_ARROW_NONE;
	if ( pData->iActiveIndicator >= iPageCount ) pData->iActiveIndicator = -1;
	pData->fAutoElapsed = 0.0f;
	iRet = __xuiCarouselSyncPages(pCarousel, pData);
	if ( iRet != XUI_OK ) goto rollback;
	if ( iOldCurrent != iNewCurrent ) {
		if ( pData->iChangeCount < INT_MAX ) pData->iChangeCount++;
		xuiInternalAccessibilityQueue(pCarousel, XUI_ACCESSIBLE_EVENT_SELECTION_CHANGED);
	}
	xuiInternalAccessibilityQueue(pCarousel, XUI_ACCESSIBLE_EVENT_TREE_CHANGED);
	return XUI_OK;

rollback:
	for ( i = 0; i < XUI_CAROUSEL_MAX_PAGES; i++ ) {
		if ( arrOldPages[i] == NULL && pData->arrPages[i] != NULL ) {
			xuiWidgetDestroy(pData->arrPages[i]);
		}
	}
	memcpy(pData->arrPages, arrOldPages, sizeof(arrOldPages));
	pData->iPageCount = iOldPageCount;
	pData->iCurrent = iOldCurrent;
	pData->iChangeCount = iOldChangeCount;
	pData->iHoverIndicator = iOldHoverIndicator;
	pData->iHoverArrow = iOldHoverArrow;
	pData->iActiveIndicator = iOldActiveIndicator;
	pData->fAutoElapsed = fOldAutoElapsed;
	/* Preserve the original failure; restoring widget state is best-effort. */
	(void)__xuiCarouselSyncPages(pCarousel, pData);
	return iRet;
}

static int __xuiCarouselSetCurrentInternal(xui_widget pCarousel, xui_carousel_data_t* pData, int iIndex, int bNotify)
{
	int iOld;
	int iRet;
	float fOldElapsed;

	if ( (pCarousel == NULL) || (pData == NULL) || (iIndex < 0) || (iIndex >= pData->iPageCount) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( pData->iCurrent == iIndex ) {
		pData->fAutoElapsed = 0.0f;
		return XUI_OK;
	}
	iOld = pData->iCurrent;
	fOldElapsed = pData->fAutoElapsed;
	pData->iCurrent = iIndex;
	pData->fAutoElapsed = 0.0f;
	iRet = __xuiCarouselSyncPages(pCarousel, pData);
	if ( iRet != XUI_OK ) {
		pData->iCurrent = iOld;
		pData->fAutoElapsed = fOldElapsed;
		/* Preserve the original failure; restoring widget state is best-effort. */
		(void)__xuiCarouselSyncPages(pCarousel, pData);
		return iRet;
	}
	if ( !__xuiCarouselArrowAvailable(pData, pData->iHoverArrow) ) {
		pData->iHoverArrow = XUI_CAROUSEL_ARROW_NONE;
	}
	if ( pData->iChangeCount < INT_MAX ) pData->iChangeCount++;
	xuiInternalAccessibilityQueue(pCarousel, XUI_ACCESSIBLE_EVENT_SELECTION_CHANGED);
	if ( bNotify && (pData->onChange != NULL) ) {
		pData->onChange(pCarousel, iOld, iIndex, pData->pChangeUser);
	}
	return XUI_OK;
}

static int __xuiCarouselMove(xui_widget pCarousel, xui_carousel_data_t* pData, int iDelta, int bNotify)
{
	int iNext;

	if ( (pCarousel == NULL) || (pData == NULL) || (pData->iPageCount <= 0) ) return XUI_ERROR_INVALID_ARGUMENT;
	iNext = pData->iCurrent + iDelta;
	if ( pData->bLoop ) {
		while ( iNext < 0 ) iNext += pData->iPageCount;
		while ( iNext >= pData->iPageCount ) iNext -= pData->iPageCount;
	} else {
		if ( iNext < 0 ) iNext = 0;
		if ( iNext >= pData->iPageCount ) iNext = pData->iPageCount - 1;
	}
	return __xuiCarouselSetCurrentInternal(pCarousel, pData, iNext, bNotify);
}

static int __xuiCarouselHitIndicator(const xui_carousel_data_t* pData, float fX, float fY)
{
	float fDX;
	float fDY;
	float fDistance;
	float fBestDistance;
	int iBest;
	int i;

	if ( pData == NULL ) return -1;
	iBest = -1;
	fBestDistance = 0.0f;
	for ( i = 0; i < pData->iPageCount; i++ ) {
		if ( __xuiCarouselRectContains(
			__xuiCarouselMinHitRect(pData->arrIndicatorRects[i], 16.0f), fX, fY) ) {
			fDX = fX - (pData->arrIndicatorRects[i].fX + pData->arrIndicatorRects[i].fW * 0.5f);
			fDY = fY - (pData->arrIndicatorRects[i].fY + pData->arrIndicatorRects[i].fH * 0.5f);
			fDistance = fDX * fDX + fDY * fDY;
			if ( iBest < 0 || fDistance < fBestDistance ) {
				iBest = i;
				fBestDistance = fDistance;
			}
		}
	}
	return iBest;
}

static int __xuiCarouselArrowAvailable(const xui_carousel_data_t* pData, int iArrow)
{
	if ( pData == NULL || pData->iPageCount <= 1 ) return 0;
	if ( pData->bLoop ) return 1;
	if ( iArrow == XUI_CAROUSEL_ARROW_PREV ) return pData->iCurrent > 0;
	if ( iArrow == XUI_CAROUSEL_ARROW_NEXT ) return pData->iCurrent + 1 < pData->iPageCount;
	return 0;
}

static int __xuiCarouselHitArrow(const xui_carousel_data_t* pData, const xui_carousel_style_t* pResolved, float fX, float fY)
{
	int bShowArrows;

	if ( (pData == NULL) || (pResolved == NULL) || (pData->iPageCount <= 1) ) return XUI_CAROUSEL_ARROW_NONE;
	bShowArrows = !pResolved->bShowArrowsOnHover || pData->bHover || (pData->iActiveArrow != XUI_CAROUSEL_ARROW_NONE);
	if ( !bShowArrows ) return XUI_CAROUSEL_ARROW_NONE;
	if ( __xuiCarouselArrowAvailable(pData, XUI_CAROUSEL_ARROW_PREV) &&
	     __xuiCarouselRectContains(pData->tPrevArrowRect, fX, fY) ) return XUI_CAROUSEL_ARROW_PREV;
	if ( __xuiCarouselArrowAvailable(pData, XUI_CAROUSEL_ARROW_NEXT) &&
	     __xuiCarouselRectContains(pData->tNextArrowRect, fX, fY) ) return XUI_CAROUSEL_ARROW_NEXT;
	return XUI_CAROUSEL_ARROW_NONE;
}

static int __xuiCarouselUpdateHover(xui_widget pCarousel,
	xui_carousel_data_t* pData, int iIndicator, int iArrow)
{
	int bChanged;

	if ( (pCarousel == NULL) || (pData == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	bChanged = (pData->iHoverIndicator != iIndicator) || (pData->iHoverArrow != iArrow);
	pData->iHoverIndicator = iIndicator;
	pData->iHoverArrow = iArrow;
	if ( bChanged ) {
		return __xuiCarouselInvalidate(pCarousel, pData, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	}
	return XUI_OK;
}

static int __xuiCarouselCancelPress(xui_widget pWidget, xui_carousel_data_t* pData, int bClearHover)
{
	xui_context pContext;
	int bChanged;
	int iRet;
	int iInvalidateRet;

	if ( (pWidget == NULL) || (pData == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	pContext = xuiWidgetGetContext(pWidget);
	bChanged = pData->tPointerOwner.bActive || (pData->iActiveIndicator >= 0) ||
		(pData->iActiveArrow != XUI_CAROUSEL_ARROW_NONE);
	if ( bClearHover ) {
		bChanged = bChanged || pData->bHover || (pData->iHoverIndicator >= 0) ||
			(pData->iHoverArrow != XUI_CAROUSEL_ARROW_NONE);
	}
	iRet = xuiInternalPointerOwnerCancel(pContext, pWidget, &pData->tPointerOwner);
	if ( !__xuiCarouselWidgetAlive(pWidget, pData) ) return iRet;
	pData->iActiveIndicator = -1;
	pData->iActiveArrow = XUI_CAROUSEL_ARROW_NONE;
	if ( bClearHover ) {
		pData->bHover = 0;
		pData->iHoverIndicator = -1;
		pData->iHoverArrow = XUI_CAROUSEL_ARROW_NONE;
	}
	if ( !bChanged ) return iRet;
	iInvalidateRet = __xuiCarouselInvalidate(pWidget, pData,
		XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	return (iRet == XUI_OK) ? iInvalidateRet : iRet;
}

static int __xuiCarouselEvent(xui_widget pWidget, const xui_event_t* pEvent, void* pUser)
{
	xui_carousel_data_t* pData;
	xui_carousel_style_t tResolved;
	xui_rect_t tWorld;
	float fX;
	float fY;
	int iHitIndicator;
	int iHitArrow;
	int iActiveIndicator;
	int iActiveArrow;
	int iCancelRet;
	int iReleaseRet;
	int iRet;

	(void)pUser;
	pData = __xuiCarouselGetData(pWidget);
	if ( (pData == NULL) || (pEvent == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pEvent->pTarget != pWidget && pEvent->iPhase == XUI_EVENT_PHASE_BUBBLE ) return XUI_OK;
	__xuiCarouselResolve(pWidget, pData, &tResolved);
	__xuiCarouselComputeRects(pData, &tResolved, xuiWidgetGetContentRect(pWidget));
	tWorld = xuiWidgetGetWorldRect(pWidget);
	fX = pEvent->fX - tWorld.fX;
	fY = pEvent->fY - tWorld.fY;
	iHitIndicator = tResolved.bShowIndicators ? __xuiCarouselHitIndicator(pData, fX, fY) : -1;
	iHitArrow = __xuiCarouselHitArrow(pData, &tResolved, fX, fY);

	switch ( pEvent->iType ) {
	case XUI_EVENT_POINTER_ENTER:
		if ( __xuiCarouselContainsWidget(pWidget, pEvent->pRelated) ) return XUI_OK;
		if ( !xuiWidgetGetEffectiveEnabled(pWidget) ||
		     !xuiWidgetGetEffectiveVisible(pWidget) ) {
			return __xuiCarouselCancelPress(pWidget, pData, 1);
		}
		pData->bHover = 1;
		iRet = __xuiCarouselUpdateHover(pWidget, pData, iHitIndicator, iHitArrow);
		if ( iRet != XUI_OK ) return iRet;
		return __xuiCarouselInvalidate(pWidget, pData, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	case XUI_EVENT_POINTER_MOVE:
		if ( !xuiWidgetGetEffectiveEnabled(pWidget) ||
		     !xuiWidgetGetEffectiveVisible(pWidget) ) {
			return __xuiCarouselCancelPress(pWidget, pData, 1);
		}
		if ( pData->tPointerOwner.bActive &&
		     !xuiInternalPointerOwnerMatches(&pData->tPointerOwner, pEvent) ) {
			return XUI_OK;
		}
		if ( pData->tPointerOwner.bActive &&
		     ((pEvent->iButtons & XUI_POINTER_BUTTON_LEFT) == 0u) ) {
			iRet = __xuiCarouselCancelPress(pWidget, pData, 0);
			if ( iRet != XUI_OK || !__xuiCarouselWidgetAlive(pWidget, pData) ) return iRet;
			iRet = __xuiCarouselUpdateHover(pWidget, pData, iHitIndicator, iHitArrow);
			if ( iRet != XUI_OK ) return iRet;
			return XUI_EVENT_DISPATCH_STOP;
		}
		pData->bHover = 1;
		iRet = __xuiCarouselUpdateHover(pWidget, pData, iHitIndicator, iHitArrow);
		if ( iRet != XUI_OK ) return iRet;
		return (pData->iHoverIndicator >= 0 || pData->iHoverArrow != XUI_CAROUSEL_ARROW_NONE) ? XUI_EVENT_DISPATCH_STOP : XUI_OK;
	case XUI_EVENT_POINTER_LEAVE:
		if ( __xuiCarouselContainsWidget(pWidget, pEvent->pRelated) ) return XUI_OK;
		pData->bHover = 0;
		pData->iHoverIndicator = -1;
		pData->iHoverArrow = XUI_CAROUSEL_ARROW_NONE;
		return __xuiCarouselInvalidate(pWidget, pData, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	case XUI_EVENT_POINTER_DOWN:
		if ( xuiWidgetGetEffectiveEnabled(pWidget) && xuiWidgetGetEffectiveVisible(pWidget) &&
		     (pEvent->iButton == XUI_POINTER_BUTTON_LEFT) &&
		     ((iHitIndicator >= 0) || (iHitArrow != XUI_CAROUSEL_ARROW_NONE)) ) {
			if ( pData->tPointerOwner.bActive ) {
				return xuiInternalPointerOwnerMatches(&pData->tPointerOwner, pEvent) ?
					XUI_EVENT_DISPATCH_STOP : XUI_OK;
			}
			pData->iActiveIndicator = iHitIndicator;
			pData->iActiveArrow = iHitArrow;
			pData->bHover = 1;
			pData->iHoverIndicator = iHitIndicator;
			pData->iHoverArrow = iHitArrow;
			iRet = xuiSetFocusWidget(xuiWidgetGetContext(pWidget), pWidget);
			if ( !__xuiCarouselWidgetAlive(pWidget, pData) ) return iRet;
			if ( iRet != XUI_OK ) {
				pData->iActiveIndicator = -1;
				pData->iActiveArrow = XUI_CAROUSEL_ARROW_NONE;
				return iRet;
			}
			iRet = xuiInternalPointerOwnerBegin(xuiWidgetGetContext(pWidget), pWidget,
				pEvent, &pData->tPointerOwner);
			if ( !__xuiCarouselWidgetAlive(pWidget, pData) ) return iRet;
			if ( iRet != XUI_OK ) {
				pData->iActiveIndicator = -1;
				pData->iActiveArrow = XUI_CAROUSEL_ARROW_NONE;
				return iRet;
			}
			iRet = __xuiCarouselInvalidate(pWidget, pData,
				XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
			if ( iRet != XUI_OK ) {
				iCancelRet = __xuiCarouselCancelPress(pWidget, pData, 0);
				return iCancelRet != XUI_OK ? iCancelRet : iRet;
			}
			return XUI_EVENT_DISPATCH_STOP;
		}
		break;
	case XUI_EVENT_POINTER_UP:
		if ( (pEvent->iButton == XUI_POINTER_BUTTON_LEFT) &&
		     xuiInternalPointerOwnerMatches(&pData->tPointerOwner, pEvent) &&
		     ((pData->iActiveIndicator >= 0) || (pData->iActiveArrow != XUI_CAROUSEL_ARROW_NONE)) ) {
			iActiveIndicator = pData->iActiveIndicator;
			iActiveArrow = pData->iActiveArrow;
			iReleaseRet = xuiInternalPointerOwnerEnd(xuiWidgetGetContext(pWidget), pWidget,
				pEvent, &pData->tPointerOwner);
			if ( !__xuiCarouselWidgetAlive(pWidget, pData) ) return iReleaseRet;
			pData->iActiveIndicator = -1;
			pData->iActiveArrow = XUI_CAROUSEL_ARROW_NONE;
			iRet = __xuiCarouselUpdateHover(pWidget, pData,
				xuiWidgetGetEffectiveEnabled(pWidget) && xuiWidgetGetEffectiveVisible(pWidget) ?
					iHitIndicator : -1,
				xuiWidgetGetEffectiveEnabled(pWidget) && xuiWidgetGetEffectiveVisible(pWidget) ?
					iHitArrow : XUI_CAROUSEL_ARROW_NONE);
			if ( iRet != XUI_OK ) return iRet;
			iRet = __xuiCarouselInvalidate(pWidget, pData, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
			if ( iRet != XUI_OK ) return iRet;
			if ( iReleaseRet != XUI_OK ) return iReleaseRet;
			iRet = XUI_OK;
			if ( (iActiveIndicator >= 0) && (iHitIndicator == iActiveIndicator) ) {
				iRet = __xuiCarouselSetCurrentInternal(pWidget, pData, iActiveIndicator, 1);
			} else if ( (iActiveArrow == XUI_CAROUSEL_ARROW_PREV) && (iHitArrow == XUI_CAROUSEL_ARROW_PREV) ) {
				iRet = __xuiCarouselMove(pWidget, pData, -1, 1);
			} else if ( (iActiveArrow == XUI_CAROUSEL_ARROW_NEXT) && (iHitArrow == XUI_CAROUSEL_ARROW_NEXT) ) {
				iRet = __xuiCarouselMove(pWidget, pData, 1, 1);
			}
			return (iRet == XUI_OK) ? (int)XUI_EVENT_DISPATCH_STOP : iRet;
		}
		break;
	case XUI_EVENT_POINTER_CAPTURE_LOST:
		if ( pEvent->pTarget != pWidget ) return XUI_OK;
		if ( !xuiInternalPointerOwnerCaptureLost(&pData->tPointerOwner, pEvent) ) {
			return XUI_OK;
		}
		pData->iActiveIndicator = -1;
		pData->iActiveArrow = XUI_CAROUSEL_ARROW_NONE;
		return __xuiCarouselInvalidate(pWidget, pData, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	case XUI_EVENT_KEY_DOWN:
		if ( pEvent->pTarget != pWidget || pEvent->iPhase != XUI_EVENT_PHASE_TARGET ) return XUI_OK;
		if ( pEvent->iKey == XUI_KEY_ESCAPE ) {
			if ( !pData->tPointerOwner.bActive &&
			     pData->iActiveIndicator < 0 &&
			     pData->iActiveArrow == XUI_CAROUSEL_ARROW_NONE ) return XUI_OK;
			iRet = __xuiCarouselCancelPress(pWidget, pData, 0);
			return (iRet == XUI_OK) ? (int)XUI_EVENT_DISPATCH_STOP : iRet;
		}
		if ( !xuiWidgetGetEffectiveEnabled(pWidget) ||
		     !xuiWidgetGetEffectiveVisible(pWidget) ) return XUI_OK;
		if ( (pEvent->iModifiers & (XUI_MOD_CTRL | XUI_MOD_ALT)) != 0u ) return XUI_OK;
		if ( pEvent->iKey == XUI_KEY_LEFT || pEvent->iKey == XUI_KEY_RIGHT ) {
			iRet = __xuiCarouselMove(pWidget, pData,
				(pEvent->iKey == XUI_KEY_LEFT) ? -1 : 1, 1);
			return (iRet == XUI_OK) ? (int)XUI_EVENT_DISPATCH_STOP : iRet;
		}
		if ( pEvent->iKey == XUI_KEY_HOME || pEvent->iKey == XUI_KEY_END ) {
			iRet = __xuiCarouselSetCurrentInternal(pWidget, pData,
				(pEvent->iKey == XUI_KEY_HOME) ? 0 : pData->iPageCount - 1, 1);
			return (iRet == XUI_OK) ? (int)XUI_EVENT_DISPATCH_STOP : iRet;
		}
		break;
	case XUI_EVENT_FOCUS:
		if ( pEvent->pTarget != pWidget ) return XUI_OK;
		return __xuiCarouselInvalidate(pWidget, pData, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	case XUI_EVENT_BLUR:
		if ( pEvent->pTarget != pWidget ) return XUI_OK;
		return __xuiCarouselCancelPress(pWidget, pData, 0);
	case XUI_EVENT_ENABLED_CHANGED:
	case XUI_EVENT_VISIBLE_CHANGED:
		if ( pEvent->pTarget != pWidget ) return XUI_OK;
		if ( !xuiWidgetGetEffectiveEnabled(pWidget) || !xuiWidgetGetEffectiveVisible(pWidget) ) {
			pData->fAutoElapsed = 0.0f;
			return __xuiCarouselCancelPress(pWidget, pData, 1);
		}
		return __xuiCarouselInvalidate(pWidget, pData, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	default:
		break;
	}
	return XUI_OK;
}

static int __xuiCarouselContentMeasure(xui_widget pWidget, xui_vec2_t tConstraint, xui_vec2_t* pSize, void* pUser)
{
	(void)pWidget;
	(void)pUser;
	if ( pSize == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	pSize->fX = 460.0f;
	pSize->fY = 290.0f;
	if ( (tConstraint.fX >= 0.0f) && (tConstraint.fX < XUI_LAYOUT_UNBOUNDED) &&
	     (pSize->fX > tConstraint.fX) ) pSize->fX = tConstraint.fX;
	if ( (tConstraint.fY >= 0.0f) && (tConstraint.fY < XUI_LAYOUT_UNBOUNDED) &&
	     (pSize->fY > tConstraint.fY) ) pSize->fY = tConstraint.fY;
	return XUI_OK;
}

static int __xuiCarouselPreparePaint(xui_widget pWidget)
{
	xui_carousel_data_t* pData = __xuiCarouselGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pData->iChromeStyleVersion == pWidget->iStyleVersion ) return XUI_OK;
	pData->iChromeStyleVersion = pWidget->iStyleVersion;
	return xuiWidgetInvalidate(pData->pOverlay, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
}

static int __xuiCarouselUpdate(xui_widget pWidget, float fDelta, void* pUser)
{
	xui_carousel_data_t* pData;
	xui_carousel_style_t tResolved;
	int iRet;

	(void)pUser;
	pData = __xuiCarouselGetData(pWidget);
	if ( (pData == NULL) || !isfinite(fDelta) || (fDelta < 0.0f) ) return XUI_ERROR_INVALID_ARGUMENT;
	__xuiCarouselResolve(pWidget, pData, &tResolved);
	if ( !xuiWidgetGetEffectiveEnabled(pWidget) || !xuiWidgetGetEffectiveVisible(pWidget) ||
	     !tResolved.bAutoPlay || (tResolved.fAutoInterval <= 0.0f) || (pData->iPageCount <= 1) ) {
		pData->fAutoElapsed = 0.0f;
		return XUI_OK;
	}
	if ( pData->bHover || pData->iActiveArrow != XUI_CAROUSEL_ARROW_NONE ||
	     pData->iActiveIndicator >= 0 ) {
		pData->fAutoElapsed = 0.0f;
		return XUI_OK;
	}
	pData->fAutoElapsed += fDelta;
	if ( pData->fAutoElapsed < tResolved.fAutoInterval ) {
		return XUI_OK;
	}
	pData->fAutoElapsed = 0.0f;
	iRet = __xuiCarouselMove(pWidget, pData, 1, 1);
	return (iRet == XUI_ERROR_INVALID_ARGUMENT) ? XUI_OK : iRet;
}

static int __xuiCarouselAccessibleCount(xui_widget pWidget, void* pUser)
{
	const xui_carousel_data_t* pData = (const xui_carousel_data_t*)pUser;

	return pData != NULL && __xuiCarouselWidgetAlive(pWidget,
		(xui_carousel_data_t*)pData) ? pData->iPageCount + 1 : 0;
}

static int __xuiCarouselAccessibleGet(xui_widget pWidget, int iIndex,
	xui_accessible_node_t* pNode, void* pUser)
{
	xui_carousel_data_t* pData = (xui_carousel_data_t*)pUser;
	xui_carousel_style_t tResolved;
	xui_rect_t tWorld;
	xui_rect_t tIndicator;
	int iPage;
	int bDisabled;
	int bOffscreen;

	if ( pData == NULL || pNode == NULL || iIndex < 0 || iIndex > pData->iPageCount ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( !__xuiCarouselWidgetAlive(pWidget, pData) ) return XUI_ERROR_INVALID_ARGUMENT;
	__xuiCarouselResolve(pWidget, pData, &tResolved);
	__xuiCarouselComputeRects(pData, &tResolved, xuiWidgetGetContentRect(pWidget));
	tWorld = xuiWidgetGetWorldRect(pWidget);
	bDisabled = !xuiWidgetGetEffectiveEnabled(pWidget);
	bOffscreen = !xuiWidgetGetEffectiveVisible(pWidget);
	if ( iIndex == 0 ) {
		(void)snprintf(pData->sAccessibleValue, sizeof(pData->sAccessibleValue),
			"Page %d of %d", pData->iCurrent + 1, pData->iPageCount);
		pNode->iId = 1;
		pNode->iRole = XUI_ACCESSIBLE_ROLE_GROUP;
		pNode->sName = xuiWidgetGetAccessibleName(pWidget) != NULL ?
			xuiWidgetGetAccessibleName(pWidget) : "Carousel";
		pNode->sValue = pData->sAccessibleValue;
		pNode->sDescription = xuiWidgetGetAccessibleDescription(pWidget);
		pNode->tBounds = tWorld;
		pNode->iState |= XUI_ACCESSIBLE_STATE_FOCUSABLE;
		if ( xuiGetFocusWidget(xuiWidgetGetContext(pWidget)) == pWidget ) {
			pNode->iState |= XUI_ACCESSIBLE_STATE_FOCUSED;
		}
		if ( bDisabled ) pNode->iState |= XUI_ACCESSIBLE_STATE_DISABLED;
		if ( bOffscreen ) pNode->iState |= XUI_ACCESSIBLE_STATE_OFFSCREEN;
		pNode->iRowCount = 1;
		pNode->iColumnCount = pData->iPageCount;
		pNode->iActions = XUI_ACCESSIBLE_ACTION_MASK(XUI_ACCESSIBLE_ACTION_FOCUS) |
			XUI_ACCESSIBLE_ACTION_MASK(XUI_ACCESSIBLE_ACTION_INCREMENT) |
			XUI_ACCESSIBLE_ACTION_MASK(XUI_ACCESSIBLE_ACTION_DECREMENT);
		if ( bDisabled ) pNode->iActions = 0;
		return XUI_OK;
	}
	iPage = iIndex - 1;
	(void)snprintf(pData->arrAccessiblePageNames[iPage],
		sizeof(pData->arrAccessiblePageNames[iPage]), "Page %d", iPage + 1);
	tIndicator = pData->arrIndicatorRects[iPage];
	pNode->iId = (uint64_t)iPage + 2u;
	pNode->iParentId = 1;
	pNode->iRole = XUI_ACCESSIBLE_ROLE_BUTTON;
	pNode->sName = pData->arrAccessiblePageNames[iPage];
	pNode->sDescription = iPage == pData->iCurrent ? "Current page" : NULL;
	pNode->tBounds = (xui_rect_t){tWorld.fX + tIndicator.fX, tWorld.fY + tIndicator.fY,
		tIndicator.fW, tIndicator.fH};
	pNode->iRow = 0;
	pNode->iColumn = iPage;
	pNode->iRowCount = 1;
	pNode->iColumnCount = 1;
	pNode->iState |= XUI_ACCESSIBLE_STATE_FOCUSABLE | XUI_ACCESSIBLE_STATE_SELECTABLE;
	if ( iPage == pData->iCurrent ) {
		pNode->iState |= XUI_ACCESSIBLE_STATE_SELECTED;
		if ( xuiGetFocusWidget(xuiWidgetGetContext(pWidget)) == pWidget ) {
			pNode->iState |= XUI_ACCESSIBLE_STATE_FOCUSED;
		}
	}
	if ( bDisabled ) pNode->iState |= XUI_ACCESSIBLE_STATE_DISABLED;
	if ( bOffscreen || !tResolved.bShowIndicators ||
	     tIndicator.fW <= 0.0f || tIndicator.fH <= 0.0f ) {
		pNode->iState |= XUI_ACCESSIBLE_STATE_OFFSCREEN;
	}
	pNode->iActions = XUI_ACCESSIBLE_ACTION_MASK(XUI_ACCESSIBLE_ACTION_FOCUS) |
		XUI_ACCESSIBLE_ACTION_MASK(XUI_ACCESSIBLE_ACTION_SET_SELECTION) |
		XUI_ACCESSIBLE_ACTION_MASK(XUI_ACCESSIBLE_ACTION_ACTIVATE);
	if ( bDisabled ) pNode->iActions = 0;
	return XUI_OK;
}

static int __xuiCarouselAccessibleAction(xui_widget pWidget, uint64_t iNodeId,
	int iAction, const void* pPayload, void* pUser)
{
	xui_carousel_data_t* pData = (xui_carousel_data_t*)pUser;
	int iPage;
	int iRet;

	(void)pPayload;
	if ( pData == NULL || !__xuiCarouselWidgetAlive(pWidget, pData) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( !xuiWidgetGetEffectiveEnabled(pWidget) ||
	     !xuiWidgetGetEffectiveVisible(pWidget) ) return XUI_ERROR_INVALID_STATE;
	if ( iNodeId == 0 || iNodeId == 1 ) {
		if ( iAction == XUI_ACCESSIBLE_ACTION_FOCUS ) {
			return xuiSetFocusWidget(xuiWidgetGetContext(pWidget), pWidget);
		}
		if ( iAction == XUI_ACCESSIBLE_ACTION_INCREMENT ||
		     iAction == XUI_ACCESSIBLE_ACTION_DECREMENT ) {
			iRet = xuiSetFocusWidget(xuiWidgetGetContext(pWidget), pWidget);
			if ( iRet != XUI_OK || !__xuiCarouselWidgetAlive(pWidget, pData) ) return iRet;
			return __xuiCarouselMove(pWidget, pData,
				iAction == XUI_ACCESSIBLE_ACTION_INCREMENT ? 1 : -1, 1);
		}
		return XUI_ERROR_UNSUPPORTED;
	}
	if ( iNodeId < 2 || iNodeId > (uint64_t)pData->iPageCount + 1u ) return XUI_ERROR_NOT_FOUND;
	iPage = (int)iNodeId - 2;
	if ( iAction != XUI_ACCESSIBLE_ACTION_FOCUS &&
	     iAction != XUI_ACCESSIBLE_ACTION_SET_SELECTION &&
	     iAction != XUI_ACCESSIBLE_ACTION_ACTIVATE ) return XUI_ERROR_UNSUPPORTED;
	iRet = xuiSetFocusWidget(xuiWidgetGetContext(pWidget), pWidget);
	if ( iRet != XUI_OK || !__xuiCarouselWidgetAlive(pWidget, pData) ) return iRet;
	return __xuiCarouselSetCurrentInternal(pWidget, pData, iPage,
		iAction != XUI_ACCESSIBLE_ACTION_FOCUS);
}

static void __xuiCarouselDefaultLayout(xui_layout_t* pLayout)
{
	memset(pLayout, 0, sizeof(*pLayout));
	pLayout->iLayoutType = XUI_LAYOUT_OVERLAY;
	pLayout->iWidthMode = XUI_SIZE_FIXED;
	pLayout->iHeightMode = XUI_SIZE_FIXED;
	pLayout->iFlowMode = XUI_FLOW_BLOCK;
	pLayout->iDock = XUI_DOCK_FILL;
	pLayout->iOverflow = XUI_OVERFLOW_CLIP;
	pLayout->iAlignX = XUI_ALIGN_START;
	pLayout->iAlignY = XUI_ALIGN_START;
	pLayout->iTableRowSpan = 1;
	pLayout->iTableColumnSpan = 1;
	pLayout->iGridColumnCount = 1;
	pLayout->fPreferredWidth = 460.0f;
	pLayout->fPreferredHeight = 290.0f;
	pLayout->fMaxWidth = XUI_LAYOUT_UNBOUNDED;
	pLayout->fMaxHeight = XUI_LAYOUT_UNBOUNDED;
	pLayout->fShrink = 1.0f;
}

static void __xuiCarouselDefaultCachePolicy(xui_cache_policy_t* pPolicy)
{
	memset(pPolicy, 0, sizeof(*pPolicy));
	pPolicy->iSize = sizeof(*pPolicy);
	pPolicy->iPolicy = XUI_CACHE_POLICY_SELF;
	pPolicy->iFlags = XUI_CACHE_CLEAR_ON_UPDATE | XUI_CACHE_UPDATE_ALL_STATES;
	pPolicy->iClearColor = XUI_COLOR_RGBA(0, 0, 0, 0);
}

static int __xuiCarouselInitEvents(xui_widget pWidget)
{
	int iRet;

	iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_POINTER_ENTER, __xuiCarouselEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_POINTER_LEAVE, __xuiCarouselEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_POINTER_MOVE, __xuiCarouselEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_POINTER_DOWN, __xuiCarouselEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_POINTER_UP, __xuiCarouselEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_POINTER_CAPTURE_LOST, __xuiCarouselEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_KEY_DOWN, __xuiCarouselEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_FOCUS, __xuiCarouselEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_BLUR, __xuiCarouselEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_ENABLED_CHANGED, __xuiCarouselEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_VISIBLE_CHANGED, __xuiCarouselEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetAccessibilityProvider(pWidget,
		__xuiCarouselAccessibleCount, __xuiCarouselAccessibleGet,
		__xuiCarouselAccessibleAction, __xuiCarouselGetData(pWidget));
	return iRet;
}

static int __xuiCarouselInit(xui_widget pWidget, void* pTypeData, const void* pCreateData, void* pUser)
{
	xui_carousel_data_t* pData;
	const xui_carousel_desc_t* pDesc;
	xui_context pContext;
	int iRet;
	int iPageCount;

	(void)pUser;
	pData = (xui_carousel_data_t*)pTypeData;
	pDesc = (const xui_carousel_desc_t*)pCreateData;
	if ( (pWidget == NULL) || (pData == NULL) || !__xuiCarouselDescValid(pDesc) ) return XUI_ERROR_INVALID_ARGUMENT;
	memset(pData, 0, sizeof(*pData));
	pContext = xuiWidgetGetContext(pWidget);
	pData->pFont = (pDesc != NULL && pDesc->pFont != NULL) ? pDesc->pFont : xuiGetDefaultFont(pContext);
	pData->iPageCount = 1;
	pData->iCurrent = 0;
	pData->bLoop = (pDesc != NULL) ? (pDesc->bLoop ? 1 : 0) : 1;
	pData->bAutoPlay = (pDesc != NULL && pDesc->bAutoPlay) ? 1 : 0;
	pData->bShowIndicators = (pDesc != NULL) ? (pDesc->bShowIndicators ? 1 : 0) : 1;
	pData->bShowArrowsOnHover = (pDesc != NULL) ? (pDesc->bShowArrowsOnHover ? 1 : 0) : 1;
	pData->iHoverIndicator = -1;
	pData->iActiveIndicator = -1;
	pData->iHoverArrow = XUI_CAROUSEL_ARROW_NONE;
	pData->iActiveArrow = XUI_CAROUSEL_ARROW_NONE;
	pData->fAutoInterval = (pDesc != NULL && pDesc->fAutoInterval > 0.0f) ? pDesc->fAutoInterval : 0.0f;
	if ( pData->bAutoPlay && pData->fAutoInterval <= 0.0f ) pData->fAutoInterval = 3.0f;
	pData->fArrowSize = (pDesc != NULL && pDesc->fArrowSize > 0.0f) ? pDesc->fArrowSize : 36.0f;
	pData->fIndicatorSize = (pDesc != NULL && pDesc->fIndicatorSize > 0.0f) ? pDesc->fIndicatorSize : 9.0f;
	pData->fIndicatorGap = (pDesc != NULL && pDesc->fIndicatorGap > 0.0f) ? pDesc->fIndicatorGap : 7.0f;
	pData->fIndicatorBottom = (pDesc != NULL && pDesc->fIndicatorBottom > 0.0f) ? pDesc->fIndicatorBottom : 20.0f;
	pData->iBackgroundColor = (pDesc != NULL && pDesc->iBackgroundColor != 0) ? pDesc->iBackgroundColor : XUI_COLOR_RGBA(255, 82, 36, 255);
	pData->iArrowColor = (pDesc != NULL && pDesc->iArrowColor != 0) ? pDesc->iArrowColor : XUI_COLOR_RGBA(96, 44, 28, 100);
	pData->iArrowHoverColor = (pDesc != NULL && pDesc->iArrowHoverColor != 0) ? pDesc->iArrowHoverColor : XUI_COLOR_RGBA(96, 44, 28, 170);
	pData->iArrowTextColor = (pDesc != NULL && pDesc->iArrowTextColor != 0) ? pDesc->iArrowTextColor : XUI_COLOR_RGBA(255, 255, 255, 245);
	pData->iIndicatorColor = (pDesc != NULL && pDesc->iIndicatorColor != 0) ? pDesc->iIndicatorColor : XUI_COLOR_RGBA(255, 255, 255, 150);
	pData->iIndicatorActiveColor = (pDesc != NULL && pDesc->iIndicatorActiveColor != 0) ? pDesc->iIndicatorActiveColor : XUI_COLOR_RGBA(255, 255, 255, 255);
	pData->iIndicatorHoverColor = (pDesc != NULL && pDesc->iIndicatorHoverColor != 0) ? pDesc->iIndicatorHoverColor : XUI_COLOR_RGBA(255, 255, 255, 215);
	pData->iFocusColor = (pDesc != NULL && pDesc->iFocusColor != 0) ? pDesc->iFocusColor : XUI_COLOR_RGBA(255, 255, 255, 110);
	iRet = xuiWidgetSetLayoutType(pWidget, XUI_LAYOUT_OVERLAY);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetOverflow(pWidget, XUI_OVERFLOW_CLIP);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetFocusable(pWidget, 1);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetTabStop(pWidget, 1);
	if ( iRet == XUI_OK ) iRet = xuiWidgetCreate(pContext, &pData->pOverlay);
	if ( iRet != XUI_OK ) return iRet;
	iRet = __xuiCarouselConfigureOverlay(pData->pOverlay, pWidget);
	if ( iRet == XUI_OK ) iRet = __xuiCarouselInitEvents(pWidget);
	if ( iRet != XUI_OK ) {
		xuiWidgetDestroy(pData->pOverlay);
		pData->pOverlay = NULL;
		return iRet;
	}
	iPageCount = (pDesc != NULL && pDesc->iPageCount > 0) ? pDesc->iPageCount : 1;
	if ( iPageCount > XUI_CAROUSEL_MAX_PAGES ) iPageCount = XUI_CAROUSEL_MAX_PAGES;
	iRet = __xuiCarouselSetPageCountInternal(pWidget, pData, iPageCount);
	if ( iRet == XUI_OK ) {
		pData->iCurrent = __xuiCarouselClampInt((pDesc != NULL) ? pDesc->iCurrent : 0, 0, pData->iPageCount - 1);
		iRet = __xuiCarouselSyncPages(pWidget, pData);
	}
	if ( iRet == XUI_OK ) iRet = xuiWidgetAddChild(pWidget, pData->pOverlay);
	if ( iRet != XUI_OK ) {
		xuiWidgetDestroy(pData->pOverlay);
		pData->pOverlay = NULL;
		return iRet;
	}
	return XUI_OK;
}

static void __xuiCarouselDestroy(xui_widget pWidget, void* pTypeData, void* pUser)
{
	xui_carousel_data_t* pData;

	(void)pUser;
	pData = (xui_carousel_data_t*)pTypeData;
	if ( pData != NULL ) {
		(void)xuiInternalPointerOwnerCancel(xuiWidgetGetContext(pWidget), pWidget,
			&pData->tPointerOwner);
		memset(pData, 0, sizeof(*pData));
	}
}

static void __xuiCarouselRegisterStyleProperty(xui_context pContext, xui_widget_type pType, const char* sName, int iValueType, uint32_t iDirtyFlags, uint32_t iFlags)
{
	xui_style_property_info_t tInfo;

	if ( xuiStyleFindProperty(pContext, sName) != 0 ) return;
	memset(&tInfo, 0, sizeof(tInfo));
	tInfo.iSize = sizeof(tInfo);
	tInfo.sName = sName;
	tInfo.iValueType = iValueType;
	tInfo.iDirtyFlags = iDirtyFlags;
	tInfo.iFlags = iFlags;
	tInfo.pWidgetType = pType;
	(void)xuiStyleRegisterProperty(pContext, &tInfo, NULL);
}

static void __xuiCarouselRegisterStyleProperties(xui_context pContext, xui_widget_type pType)
{
	uint32_t iPaintDirty;

	pType->onPreparePaint = __xuiCarouselPreparePaint;
	iPaintDirty = XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER;
	__xuiCarouselRegisterStyleProperty(pContext, pType, "carousel.background.color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiCarouselRegisterStyleProperty(pContext, pType, "carousel.arrow.color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiCarouselRegisterStyleProperty(pContext, pType, "carousel.arrow.hover_color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiCarouselRegisterStyleProperty(pContext, pType, "carousel.arrow.text_color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiCarouselRegisterStyleProperty(pContext, pType, "carousel.indicator.color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiCarouselRegisterStyleProperty(pContext, pType, "carousel.indicator.background_color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiCarouselRegisterStyleProperty(pContext, pType, "carousel.indicator.active_color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiCarouselRegisterStyleProperty(pContext, pType, "carousel.indicator.hover_color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiCarouselRegisterStyleProperty(pContext, pType, "carousel.focus.color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiCarouselRegisterStyleProperty(pContext, pType, "carousel.arrow.size", XUI_STYLE_VALUE_FLOAT, iPaintDirty, 0);
	__xuiCarouselRegisterStyleProperty(pContext, pType, "carousel.indicator.size", XUI_STYLE_VALUE_FLOAT, iPaintDirty, 0);
	__xuiCarouselRegisterStyleProperty(pContext, pType, "carousel.indicator.gap", XUI_STYLE_VALUE_FLOAT, iPaintDirty, 0);
	__xuiCarouselRegisterStyleProperty(pContext, pType, "carousel.indicator.bottom", XUI_STYLE_VALUE_FLOAT, iPaintDirty, 0);
	__xuiCarouselRegisterStyleProperty(pContext, pType, "carousel.auto.enabled", XUI_STYLE_VALUE_BOOL, iPaintDirty, 0);
	__xuiCarouselRegisterStyleProperty(pContext, pType, "carousel.auto.interval", XUI_STYLE_VALUE_FLOAT, iPaintDirty, 0);
	__xuiCarouselRegisterStyleProperty(pContext, pType, "carousel.indicator.visible", XUI_STYLE_VALUE_BOOL, iPaintDirty, 0);
	__xuiCarouselRegisterStyleProperty(pContext, pType, "carousel.arrow.hover_only", XUI_STYLE_VALUE_BOOL, iPaintDirty, 0);
	__xuiCarouselRegisterStyleProperty(pContext, pType, "font.name", XUI_STYLE_VALUE_STRING, iPaintDirty, XUI_STYLE_PROPERTY_INHERITED);
}

XUI_API xui_widget_type xuiCarouselGetType(xui_context pContext)
{
	xui_widget_type_desc_t tDesc;
	xui_cache_policy_t tPolicy;
	xui_widget_type pType;
	int iRet;

	if ( !xuiInternalContextIsValid(pContext) ) return NULL;
	pType = xuiWidgetFindType(pContext, "carousel");
	if ( pType != NULL ) {
		__xuiCarouselRegisterStyleProperties(pContext, pType);
		return pType;
	}
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.sName = "carousel";
	tDesc.pParent = xuiWidgetGetBaseType();
	tDesc.iFlags = XUI_WIDGET_TYPE_DEFAULT_LAYOUT | XUI_WIDGET_TYPE_DEFAULT_CACHE_POLICY;
	tDesc.iTypeDataSize = sizeof(xui_carousel_data_t);
	tDesc.onInit = __xuiCarouselInit;
	tDesc.onDestroy = __xuiCarouselDestroy;
	tDesc.onContentMeasure = __xuiCarouselContentMeasure;
	tDesc.onCacheRender = __xuiCarouselBaseRender;
	tDesc.onUpdate = __xuiCarouselUpdate;
	__xuiCarouselDefaultLayout(&tDesc.tLayout);
	__xuiCarouselDefaultCachePolicy(&tPolicy);
	tDesc.tCachePolicy = tPolicy;
	iRet = xuiWidgetRegisterType(pContext, &pType, &tDesc);
	if ( iRet != XUI_OK ) return NULL;
	__xuiCarouselRegisterStyleProperties(pContext, pType);
	return pType;
}

XUI_API int xuiCarouselCreate(xui_context pContext, xui_widget* ppWidget, const xui_carousel_desc_t* pDesc)
{
	xui_widget_type pType;

	if ( ppWidget == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	*ppWidget = NULL;
	if ( !xuiInternalContextIsValid(pContext) ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( !__xuiCarouselDescValid(pDesc) ) return XUI_ERROR_INVALID_ARGUMENT;
	pType = xuiCarouselGetType(pContext);
	if ( pType == NULL ) return XUI_ERROR_NOT_INITIALIZED;
	return xuiWidgetCreateTyped(pContext, pType, ppWidget, pDesc);
}

XUI_API int xuiCarouselSetChange(xui_widget pWidget, xui_carousel_change_proc onChange, void* pUser)
{
	xui_carousel_data_t* pData = __xuiCarouselGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	pData->onChange = onChange;
	pData->pChangeUser = pUser;
	return XUI_OK;
}

XUI_API int xuiCarouselSetPageCount(xui_widget pWidget, int iPageCount)
{
	xui_carousel_data_t* pData = __xuiCarouselGetData(pWidget);
	int iRet;
	if ( (pData == NULL) || (iPageCount < 1) || (iPageCount > XUI_CAROUSEL_MAX_PAGES) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( pData->iPageCount == iPageCount ) return XUI_OK;
	iRet = __xuiCarouselCancelPress(pWidget, pData, 0);
	if ( iRet != XUI_OK || !__xuiCarouselWidgetAlive(pWidget, pData) ) return iRet;
	return __xuiCarouselSetPageCountInternal(pWidget, pData, iPageCount);
}

XUI_API int xuiCarouselGetPageCount(xui_widget pWidget)
{
	xui_carousel_data_t* pData = __xuiCarouselGetData(pWidget);
	return (pData != NULL) ? pData->iPageCount : 0;
}

XUI_API xui_widget xuiCarouselGetPageWidget(xui_widget pWidget, int iIndex)
{
	xui_carousel_data_t* pData = __xuiCarouselGetData(pWidget);
	if ( (pData == NULL) || (iIndex < 0) || (iIndex >= pData->iPageCount) ) return NULL;
	return pData->arrPages[iIndex];
}

XUI_API int xuiCarouselAddPageChild(xui_widget pWidget, int iIndex, xui_widget pChild)
{
	xui_widget pPage;

	if ( pChild == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	pPage = xuiCarouselGetPageWidget(pWidget, iIndex);
	if ( pPage == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	return xuiWidgetAddChild(pPage, pChild);
}

XUI_API int xuiCarouselSetCurrent(xui_widget pWidget, int iIndex, int bNotify)
{
	xui_carousel_data_t* pData = __xuiCarouselGetData(pWidget);
	int iRet;
	if ( (pData == NULL) || (iIndex < 0) || (iIndex >= pData->iPageCount) ) return XUI_ERROR_INVALID_ARGUMENT;
	iRet = __xuiCarouselCancelPress(pWidget, pData, 0);
	if ( iRet != XUI_OK || !__xuiCarouselWidgetAlive(pWidget, pData) ) return iRet;
	return __xuiCarouselSetCurrentInternal(pWidget, pData, iIndex, bNotify);
}

XUI_API int xuiCarouselGetCurrent(xui_widget pWidget)
{
	xui_carousel_data_t* pData = __xuiCarouselGetData(pWidget);
	return (pData != NULL) ? pData->iCurrent : -1;
}

XUI_API int xuiCarouselNext(xui_widget pWidget, int bNotify)
{
	xui_carousel_data_t* pData = __xuiCarouselGetData(pWidget);
	int iRet;
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	iRet = __xuiCarouselCancelPress(pWidget, pData, 0);
	if ( iRet != XUI_OK || !__xuiCarouselWidgetAlive(pWidget, pData) ) return iRet;
	return __xuiCarouselMove(pWidget, pData, 1, bNotify);
}

XUI_API int xuiCarouselPrev(xui_widget pWidget, int bNotify)
{
	xui_carousel_data_t* pData = __xuiCarouselGetData(pWidget);
	int iRet;
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	iRet = __xuiCarouselCancelPress(pWidget, pData, 0);
	if ( iRet != XUI_OK || !__xuiCarouselWidgetAlive(pWidget, pData) ) return iRet;
	return __xuiCarouselMove(pWidget, pData, -1, bNotify);
}

XUI_API int xuiCarouselSetAutoPlay(xui_widget pWidget, int bEnabled, float fInterval)
{
	xui_carousel_data_t* pData = __xuiCarouselGetData(pWidget);
	int bOldAutoPlay;
	float fOldInterval;
	float fOldElapsed;
	int iRet;

	if ( (pData == NULL) || !__xuiCarouselMetricValid(fInterval, FLT_MAX) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	bOldAutoPlay = pData->bAutoPlay;
	fOldInterval = pData->fAutoInterval;
	fOldElapsed = pData->fAutoElapsed;
	pData->bAutoPlay = bEnabled ? 1 : 0;
	pData->fAutoInterval = (fInterval > 0.0f) ? fInterval : 0.0f;
	if ( pData->bAutoPlay && pData->fAutoInterval <= 0.0f ) pData->fAutoInterval = 3.0f;
	pData->fAutoElapsed = 0.0f;
	iRet = __xuiCarouselInvalidate(pWidget, pData,
		XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	if ( iRet != XUI_OK ) {
		pData->bAutoPlay = bOldAutoPlay;
		pData->fAutoInterval = fOldInterval;
		pData->fAutoElapsed = fOldElapsed;
	}
	return iRet;
}

XUI_API int xuiCarouselGetAutoPlay(xui_widget pWidget, int* pEnabled, float* pInterval)
{
	xui_carousel_data_t* pData = __xuiCarouselGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pEnabled != NULL ) *pEnabled = pData->bAutoPlay;
	if ( pInterval != NULL ) *pInterval = pData->fAutoInterval;
	return XUI_OK;
}

XUI_API int xuiCarouselSetLoop(xui_widget pWidget, int bLoop)
{
	xui_carousel_data_t* pData = __xuiCarouselGetData(pWidget);
	int bNewLoop;
	int bOldLoop;
	int iOldHoverArrow;
	int iRet;

	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	bNewLoop = bLoop ? 1 : 0;
	if ( pData->bLoop == bNewLoop ) return XUI_OK;
	bOldLoop = pData->bLoop;
	iOldHoverArrow = pData->iHoverArrow;
	iRet = __xuiCarouselCancelPress(pWidget, pData, 0);
	if ( iRet != XUI_OK || !__xuiCarouselWidgetAlive(pWidget, pData) ) return iRet;
	pData->bLoop = bNewLoop;
	pData->iHoverArrow = XUI_CAROUSEL_ARROW_NONE;
	iRet = __xuiCarouselInvalidate(pWidget, pData,
		XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	if ( iRet != XUI_OK ) {
		pData->bLoop = bOldLoop;
		pData->iHoverArrow = iOldHoverArrow;
	}
	return iRet;
}

XUI_API int xuiCarouselGetLoop(xui_widget pWidget)
{
	xui_carousel_data_t* pData = __xuiCarouselGetData(pWidget);
	return (pData != NULL) ? pData->bLoop : 0;
}

XUI_API int xuiCarouselSetIndicatorsVisible(xui_widget pWidget, int bVisible)
{
	xui_carousel_data_t* pData = __xuiCarouselGetData(pWidget);
	int bNewVisible;
	int bOldVisible;
	int iOldHoverIndicator;
	int iRet;
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	bNewVisible = bVisible ? 1 : 0;
	if ( pData->bShowIndicators == bNewVisible ) return XUI_OK;
	bOldVisible = pData->bShowIndicators;
	iOldHoverIndicator = pData->iHoverIndicator;
	iRet = __xuiCarouselCancelPress(pWidget, pData, 0);
	if ( iRet != XUI_OK || !__xuiCarouselWidgetAlive(pWidget, pData) ) return iRet;
	pData->bShowIndicators = bNewVisible;
	pData->iHoverIndicator = -1;
	pData->iActiveIndicator = -1;
	iRet = __xuiCarouselInvalidate(pWidget, pData,
		XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	if ( iRet != XUI_OK ) {
		pData->bShowIndicators = bOldVisible;
		pData->iHoverIndicator = iOldHoverIndicator;
	}
	return iRet;
}

XUI_API int xuiCarouselGetIndicatorsVisible(xui_widget pWidget)
{
	xui_carousel_data_t* pData = __xuiCarouselGetData(pWidget);
	return (pData != NULL) ? pData->bShowIndicators : 0;
}

XUI_API int xuiCarouselSetFont(xui_widget pWidget, xui_font pFont)
{
	xui_carousel_data_t* pData = __xuiCarouselGetData(pWidget);
	xui_font pNewFont;
	xui_font pOldFont;
	int iRet;

	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	pNewFont = (pFont != NULL) ? pFont : xuiGetDefaultFont(xuiWidgetGetContext(pWidget));
	if ( pData->pFont == pNewFont ) return XUI_OK;
	pOldFont = pData->pFont;
	pData->pFont = pNewFont;
	iRet = __xuiCarouselInvalidate(pWidget, pData,
		XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	if ( iRet != XUI_OK ) pData->pFont = pOldFont;
	return iRet;
}

XUI_API xui_font xuiCarouselGetFont(xui_widget pWidget)
{
	xui_carousel_data_t* pData = __xuiCarouselGetData(pWidget);
	return (pData != NULL) ? pData->pFont : NULL;
}

XUI_API int xuiCarouselSetArrowsOnHover(xui_widget pWidget, int bEnabled)
{
	xui_carousel_data_t* pData = __xuiCarouselGetData(pWidget);
	int bNewEnabled;
	int bOldEnabled;
	int iOldHoverArrow;
	int iRet;
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	bNewEnabled = bEnabled ? 1 : 0;
	if ( pData->bShowArrowsOnHover == bNewEnabled ) return XUI_OK;
	bOldEnabled = pData->bShowArrowsOnHover;
	iOldHoverArrow = pData->iHoverArrow;
	iRet = __xuiCarouselCancelPress(pWidget, pData, 0);
	if ( iRet != XUI_OK || !__xuiCarouselWidgetAlive(pWidget, pData) ) return iRet;
	pData->bShowArrowsOnHover = bNewEnabled;
	pData->iHoverArrow = XUI_CAROUSEL_ARROW_NONE;
	pData->iActiveArrow = XUI_CAROUSEL_ARROW_NONE;
	iRet = __xuiCarouselInvalidate(pWidget, pData,
		XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	if ( iRet != XUI_OK ) {
		pData->bShowArrowsOnHover = bOldEnabled;
		pData->iHoverArrow = iOldHoverArrow;
	}
	return iRet;
}

XUI_API int xuiCarouselGetArrowsOnHover(xui_widget pWidget)
{
	xui_carousel_data_t* pData = __xuiCarouselGetData(pWidget);
	return (pData != NULL) ? pData->bShowArrowsOnHover : 0;
}

XUI_API int xuiCarouselSetMetrics(xui_widget pWidget, float fArrowSize, float fIndicatorSize, float fIndicatorGap, float fIndicatorBottom)
{
	xui_carousel_data_t* pData = __xuiCarouselGetData(pWidget);
	float fNewArrowSize;
	float fNewIndicatorSize;
	float fOldArrowSize;
	float fOldIndicatorSize;
	float fOldIndicatorGap;
	float fOldIndicatorBottom;
	int iOldHoverIndicator;
	int iOldHoverArrow;
	int iRet;
	if ( (pData == NULL) ||
	     !__xuiCarouselMetricValid(fArrowSize,
		XUI_CAROUSEL_MAX_GEOMETRY_METRIC) ||
	     !__xuiCarouselMetricValid(fIndicatorSize,
		XUI_CAROUSEL_MAX_GEOMETRY_METRIC) ||
	     !__xuiCarouselMetricValid(fIndicatorGap,
		XUI_CAROUSEL_MAX_GEOMETRY_METRIC) ||
	     !__xuiCarouselMetricValid(fIndicatorBottom,
		XUI_CAROUSEL_MAX_GEOMETRY_METRIC) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	fNewArrowSize = (fArrowSize > 0.0f) ? fArrowSize : 36.0f;
	fNewIndicatorSize = (fIndicatorSize > 0.0f) ? fIndicatorSize : 9.0f;
	if ( pData->fArrowSize == fNewArrowSize &&
	     pData->fIndicatorSize == fNewIndicatorSize &&
	     pData->fIndicatorGap == fIndicatorGap &&
	     pData->fIndicatorBottom == fIndicatorBottom ) return XUI_OK;
	iRet = __xuiCarouselCancelPress(pWidget, pData, 0);
	if ( iRet != XUI_OK || !__xuiCarouselWidgetAlive(pWidget, pData) ) return iRet;
	fOldArrowSize = pData->fArrowSize;
	fOldIndicatorSize = pData->fIndicatorSize;
	fOldIndicatorGap = pData->fIndicatorGap;
	fOldIndicatorBottom = pData->fIndicatorBottom;
	iOldHoverIndicator = pData->iHoverIndicator;
	iOldHoverArrow = pData->iHoverArrow;
	pData->fArrowSize = fNewArrowSize;
	pData->fIndicatorSize = fNewIndicatorSize;
	pData->fIndicatorGap = fIndicatorGap;
	pData->fIndicatorBottom = fIndicatorBottom;
	pData->iHoverIndicator = -1;
	pData->iHoverArrow = XUI_CAROUSEL_ARROW_NONE;
	iRet = __xuiCarouselInvalidate(pWidget, pData,
		XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	if ( iRet != XUI_OK ) {
		pData->fArrowSize = fOldArrowSize;
		pData->fIndicatorSize = fOldIndicatorSize;
		pData->fIndicatorGap = fOldIndicatorGap;
		pData->fIndicatorBottom = fOldIndicatorBottom;
		pData->iHoverIndicator = iOldHoverIndicator;
		pData->iHoverArrow = iOldHoverArrow;
	}
	return iRet;
}

XUI_API int xuiCarouselGetMetrics(xui_widget pWidget, float* pArrowSize, float* pIndicatorSize, float* pIndicatorGap, float* pIndicatorBottom)
{
	xui_carousel_data_t* pData = __xuiCarouselGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pArrowSize != NULL ) *pArrowSize = pData->fArrowSize;
	if ( pIndicatorSize != NULL ) *pIndicatorSize = pData->fIndicatorSize;
	if ( pIndicatorGap != NULL ) *pIndicatorGap = pData->fIndicatorGap;
	if ( pIndicatorBottom != NULL ) *pIndicatorBottom = pData->fIndicatorBottom;
	return XUI_OK;
}

XUI_API int xuiCarouselSetColors(xui_widget pWidget, uint32_t iBackground, uint32_t iArrow, uint32_t iArrowHover, uint32_t iArrowText, uint32_t iIndicator, uint32_t iIndicatorActive, uint32_t iIndicatorHover)
{
	xui_carousel_data_t* pData = __xuiCarouselGetData(pWidget);
	uint32_t arrOld[7];
	int iRet;

	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pData->iBackgroundColor == iBackground && pData->iArrowColor == iArrow &&
	     pData->iArrowHoverColor == iArrowHover && pData->iArrowTextColor == iArrowText &&
	     pData->iIndicatorColor == iIndicator &&
	     pData->iIndicatorActiveColor == iIndicatorActive &&
	     pData->iIndicatorHoverColor == iIndicatorHover ) return XUI_OK;
	arrOld[0] = pData->iBackgroundColor;
	arrOld[1] = pData->iArrowColor;
	arrOld[2] = pData->iArrowHoverColor;
	arrOld[3] = pData->iArrowTextColor;
	arrOld[4] = pData->iIndicatorColor;
	arrOld[5] = pData->iIndicatorActiveColor;
	arrOld[6] = pData->iIndicatorHoverColor;
	pData->iBackgroundColor = iBackground;
	pData->iArrowColor = iArrow;
	pData->iArrowHoverColor = iArrowHover;
	pData->iArrowTextColor = iArrowText;
	pData->iIndicatorColor = iIndicator;
	pData->iIndicatorActiveColor = iIndicatorActive;
	pData->iIndicatorHoverColor = iIndicatorHover;
	iRet = __xuiCarouselInvalidate(pWidget, pData,
		XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	if ( iRet != XUI_OK ) {
		pData->iBackgroundColor = arrOld[0];
		pData->iArrowColor = arrOld[1];
		pData->iArrowHoverColor = arrOld[2];
		pData->iArrowTextColor = arrOld[3];
		pData->iIndicatorColor = arrOld[4];
		pData->iIndicatorActiveColor = arrOld[5];
		pData->iIndicatorHoverColor = arrOld[6];
	}
	return iRet;
}

XUI_API int xuiCarouselGetColors(xui_widget pWidget, uint32_t* pBackground, uint32_t* pArrow, uint32_t* pArrowHover, uint32_t* pArrowText, uint32_t* pIndicator, uint32_t* pIndicatorActive, uint32_t* pIndicatorHover)
{
	xui_carousel_data_t* pData = __xuiCarouselGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pBackground != NULL ) *pBackground = pData->iBackgroundColor;
	if ( pArrow != NULL ) *pArrow = pData->iArrowColor;
	if ( pArrowHover != NULL ) *pArrowHover = pData->iArrowHoverColor;
	if ( pArrowText != NULL ) *pArrowText = pData->iArrowTextColor;
	if ( pIndicator != NULL ) *pIndicator = pData->iIndicatorColor;
	if ( pIndicatorActive != NULL ) *pIndicatorActive = pData->iIndicatorActiveColor;
	if ( pIndicatorHover != NULL ) *pIndicatorHover = pData->iIndicatorHoverColor;
	return XUI_OK;
}

XUI_API int xuiCarouselSetFocusColor(xui_widget pWidget, uint32_t iColor)
{
	xui_carousel_data_t* pData = __xuiCarouselGetData(pWidget);
	uint32_t iOldColor;
	int iRet;

	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pData->iFocusColor == iColor ) return XUI_OK;
	iOldColor = pData->iFocusColor;
	pData->iFocusColor = iColor;
	iRet = __xuiCarouselInvalidate(pWidget, pData,
		XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	if ( iRet != XUI_OK ) pData->iFocusColor = iOldColor;
	return iRet;
}

XUI_API uint32_t xuiCarouselGetFocusColor(xui_widget pWidget)
{
	xui_carousel_data_t* pData = __xuiCarouselGetData(pWidget);
	return (pData != NULL) ? pData->iFocusColor : 0u;
}

XUI_API int xuiCarouselGetHoverIndicator(xui_widget pWidget)
{
	xui_carousel_data_t* pData = __xuiCarouselGetData(pWidget);
	return (pData != NULL) ? pData->iHoverIndicator : -1;
}

XUI_API int xuiCarouselGetHoverArrow(xui_widget pWidget)
{
	xui_carousel_data_t* pData = __xuiCarouselGetData(pWidget);
	return (pData != NULL) ? pData->iHoverArrow : XUI_CAROUSEL_ARROW_NONE;
}

XUI_API int xuiCarouselGetChangeCount(xui_widget pWidget)
{
	xui_carousel_data_t* pData = __xuiCarouselGetData(pWidget);
	return (pData != NULL) ? pData->iChangeCount : 0;
}

#endif
