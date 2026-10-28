#include "xui_config.h"
#if XUI_ENABLE_ACCORDION
#include "xui_internal.h"
#include "xui_pointer_press_internal.h"

#include <math.h>
#include <string.h>

#define XUI_ACCORDION_DEFAULT_BORDER_COLOR XUI_COLOR_RGBA(127, 196, 229, 255)
#define XUI_ACCORDION_DEFAULT_INDICATOR_COLOR XUI_COLOR_RGBA(47, 125, 215, 255)

typedef struct xui_accordion_section_t {
	xui_widget pSection;
	xui_widget pHeader;
	xui_widget pClient;
	char sTitle[XUI_ACCORDION_TITLE_CAPACITY];
	int iId;
	int bExpanded;
	int bEnabled;
	xui_rect_t tSectionRect;
	xui_rect_t tHeaderRect;
	xui_rect_t tClientRect;
	xui_rect_t tArrowRect;
	xui_rect_t tTextRect;
} xui_accordion_section_t;

typedef struct xui_accordion_data_t {
	xui_accordion_select_proc onSelect;
	void* pSelectUser;
	xui_accordion_section_t arrSections[XUI_ACCORDION_SECTION_CAPACITY];
	xui_font pFont;
	int iSectionCount;
	int iMode;
	int iSelected;
	int iHoverIndex;
	int iActiveIndex;
	int iChangeCount;
	xui_pointer_owner_t tPointerOwner;
	uint32_t iChromeStyleVersion;
	float fHeaderHeight;
	float fSpacing;
	float fContentPadding;
	float fContentHeight;
	uint32_t iBackgroundColor;
	uint32_t iHeaderColor;
	uint32_t iHoverColor;
	uint32_t iExpandedColor;
	uint32_t iContentColor;
	uint32_t iBorderColor;
	uint32_t iTextColor;
	uint32_t iActiveTextColor;
	uint32_t iDisabledTextColor;
} xui_accordion_data_t;

typedef struct xui_accordion_resolved_t {
	uint32_t iIndicatorColor;
	xui_font pFont;
	float fHeaderHeight;
	float fSpacing;
	float fContentPadding;
	uint32_t iBackgroundColor;
	uint32_t iHeaderColor;
	uint32_t iHoverColor;
	uint32_t iExpandedColor;
	uint32_t iContentColor;
	uint32_t iBorderColor;
	uint32_t iTextColor;
	uint32_t iActiveTextColor;
	uint32_t iDisabledTextColor;
} xui_accordion_resolved_t;

static xui_thickness_t __xuiAccordionThickness(float fLeft, float fTop, float fRight, float fBottom)
{
	xui_thickness_t tValue;

	tValue.fLeft = fLeft;
	tValue.fTop = fTop;
	tValue.fRight = fRight;
	tValue.fBottom = fBottom;
	return tValue;
}

static int __xuiAccordionAlpha(uint32_t iColor)
{
	return (int)(iColor & 0xffu);
}

static uint32_t __xuiAccordionIndicatorColor(const xui_accordion_resolved_t* pResolved)
{
	if ( pResolved == NULL ) {
		return XUI_ACCORDION_DEFAULT_INDICATOR_COLOR;
	}
	if ( pResolved->iBorderColor == XUI_ACCORDION_DEFAULT_BORDER_COLOR ) {
		return XUI_ACCORDION_DEFAULT_INDICATOR_COLOR;
	}
	return pResolved->iBorderColor;
}

static float __xuiAccordionMaxFloat(float fA, float fB)
{
	return (fA > fB) ? fA : fB;
}

static int __xuiAccordionRectContains(xui_rect_t tRect, float fX, float fY)
{
	return (fX >= tRect.fX) && (fY >= tRect.fY) && (fX < (tRect.fX + tRect.fW)) && (fY < (tRect.fY + tRect.fH));
}

static int __xuiAccordionModeValid(int iMode)
{
	return (iMode == XUI_ACCORDION_MODE_MULTIPLE) || (iMode == XUI_ACCORDION_MODE_SINGLE);
}

static int __xuiAccordionDescValid(const xui_accordion_desc_t* pDesc)
{
	if ( pDesc == NULL ) {
		return 1;
	}
	if ( (pDesc->iSize != 0) && (pDesc->iSize < sizeof(*pDesc)) ) {
		return 0;
	}
	if ( (pDesc->iSectionCount < 0) || (pDesc->iSectionCount > XUI_ACCORDION_SECTION_CAPACITY) ) {
		return 0;
	}
	if ( (pDesc->iSectionCount > 0) && (pDesc->arrSections == NULL) ) {
		return 0;
	}
	if ( (pDesc->iMode != 0) && !__xuiAccordionModeValid(pDesc->iMode) ) {
		return 0;
	}
	if ( !isfinite(pDesc->fHeaderHeight) || (pDesc->fHeaderHeight < 0.0f) ||
	     !isfinite(pDesc->fSpacing) || (pDesc->fSpacing < 0.0f) ||
	     !isfinite(pDesc->fContentPadding) || (pDesc->fContentPadding < 0.0f) ) {
		return 0;
	}
	return 1;
}

static void __xuiAccordionSetTitle(xui_accordion_section_t* pSection, const char* sTitle)
{
	if ( pSection == NULL ) return;
	(void)xuiInternalUtf8CopyGraphemes(pSection->sTitle,
		sizeof(pSection->sTitle), sTitle);
}

static xui_accordion_data_t* __xuiAccordionGetData(xui_widget pWidget)
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
	pType = xuiWidgetFindType(pContext, "accordion");
	if ( (pType == NULL) || !xuiWidgetIsType(pWidget, pType) ) {
		return NULL;
	}
	return (xui_accordion_data_t*)xuiWidgetGetTypeData(pWidget);
}

static int __xuiAccordionStyleColor(xui_widget pWidget, const char* sName, uint32_t* pColor)
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

static int __xuiAccordionStyleFloat(xui_widget pWidget, const char* sName, float* pValue)
{
	xui_style_property_t tProperty;
	int iRet;

	memset(&tProperty, 0, sizeof(tProperty));
	tProperty.iSize = sizeof(tProperty);
	iRet = xuiWidgetGetResolvedStyleProperty(pWidget, sName, &tProperty);
	if ( (iRet == XUI_OK) &&
	     (tProperty.tValue.iType == XUI_STYLE_VALUE_FLOAT) &&
	     isfinite(tProperty.tValue.fFloat) &&
	     (tProperty.tValue.fFloat >= 0.0f) ) {
		*pValue = tProperty.tValue.fFloat;
		return 1;
	}
	return 0;
}

static void __xuiAccordionResolve(xui_widget pWidget, const xui_accordion_data_t* pData, xui_accordion_resolved_t* pOut)
{
	pOut->pFont = (pData->pFont != NULL) ? pData->pFont : xuiGetDefaultFont(xuiWidgetGetContext(pWidget));
	pOut->fHeaderHeight = pData->fHeaderHeight;
	pOut->fSpacing = pData->fSpacing;
	pOut->fContentPadding = pData->fContentPadding;
	pOut->iBackgroundColor = pData->iBackgroundColor;
	pOut->iHeaderColor = pData->iHeaderColor;
	pOut->iHoverColor = pData->iHoverColor;
	pOut->iExpandedColor = pData->iExpandedColor;
	pOut->iContentColor = pData->iContentColor;
	pOut->iBorderColor = pData->iBorderColor;
	pOut->iTextColor = pData->iTextColor;
	pOut->iActiveTextColor = pData->iActiveTextColor;
	pOut->iDisabledTextColor = pData->iDisabledTextColor;
	(void)__xuiAccordionStyleFloat(pWidget, "accordion.header.height", &pOut->fHeaderHeight);
	(void)__xuiAccordionStyleFloat(pWidget, "accordion.spacing", &pOut->fSpacing);
	(void)__xuiAccordionStyleFloat(pWidget, "accordion.content.padding", &pOut->fContentPadding);
	(void)__xuiAccordionStyleColor(pWidget, "accordion.background.color", &pOut->iBackgroundColor);
	(void)__xuiAccordionStyleColor(pWidget, "accordion.header.color", &pOut->iHeaderColor);
	(void)__xuiAccordionStyleColor(pWidget, "accordion.header.hover_color", &pOut->iHoverColor);
	(void)__xuiAccordionStyleColor(pWidget, "accordion.header.expanded_color", &pOut->iExpandedColor);
	(void)__xuiAccordionStyleColor(pWidget, "accordion.content.color", &pOut->iContentColor);
	(void)__xuiAccordionStyleColor(pWidget, "accordion.border.color", &pOut->iBorderColor);
	(void)__xuiAccordionStyleColor(pWidget, "accordion.text.color", &pOut->iTextColor);
	(void)__xuiAccordionStyleColor(pWidget, "accordion.text.active_color", &pOut->iActiveTextColor);
	(void)__xuiAccordionStyleColor(pWidget, "accordion.text.disabled_color", &pOut->iDisabledTextColor);
	pOut->iIndicatorColor = __xuiAccordionIndicatorColor(pOut);
	(void)__xuiAccordionStyleColor(pWidget, "accordion.indicator.color", &pOut->iIndicatorColor);
	if ( pOut->fHeaderHeight < 18.0f ) pOut->fHeaderHeight = 18.0f;
	if ( pOut->fSpacing < 0.0f ) pOut->fSpacing = 0.0f;
	if ( pOut->fContentPadding < 0.0f ) pOut->fContentPadding = 0.0f;
}

static int __xuiAccordionDrawFill(xui_proxy pProxy, xui_draw_context pDraw, xui_rect_t tRect, uint32_t iColor)
{
	if ( (pProxy == NULL) || (pProxy->drawRectFill == NULL) || (pDraw == NULL) ) {
		return XUI_ERROR_NOT_INITIALIZED;
	}
	if ( (tRect.fW <= 0.0f) || (tRect.fH <= 0.0f) || (__xuiAccordionAlpha(iColor) == 0) ) {
		return XUI_OK;
	}
	return pProxy->drawRectFill(pProxy, pDraw, xuiInternalSnapRect(tRect), iColor);
}

static int __xuiAccordionDrawRectLines(xui_proxy pProxy, xui_draw_context pDraw, xui_rect_t tRect, uint32_t iColor);

static int __xuiAccordionDrawRectFill(xui_proxy pProxy, xui_draw_context pDraw, xui_rect_t tRect, uint32_t iColor)
{
	return __xuiAccordionDrawFill(pProxy, pDraw, tRect, iColor);
}

static int __xuiAccordionDrawRectStroke(xui_proxy pProxy, xui_draw_context pDraw, xui_rect_t tRect, uint32_t iColor)
{
	return __xuiAccordionDrawRectLines(pProxy, pDraw, tRect, iColor);
}

static int __xuiAccordionDrawLine(xui_proxy pProxy, xui_draw_context pDraw, float fX0, float fY0, float fX1, float fY1, uint32_t iColor)
{
	if ( (pProxy == NULL) || (pProxy->drawLine == NULL) || (pDraw == NULL) ) {
		return XUI_ERROR_NOT_INITIALIZED;
	}
	if ( __xuiAccordionAlpha(iColor) == 0 ) {
		return XUI_OK;
	}
	return pProxy->drawLine(pProxy, pDraw,
		xuiInternalSnapPixel(fX0), xuiInternalSnapPixel(fY0),
		xuiInternalSnapPixel(fX1), xuiInternalSnapPixel(fY1),
		1.0f, iColor);
}

static int __xuiAccordionDrawRectSideLines(xui_proxy pProxy, xui_draw_context pDraw, xui_rect_t tRect, uint32_t iColor, int bTop, int bRight, int bBottom, int bLeft)
{
	float fLeft;
	float fTop;
	float fRight;
	float fBottom;
	float fWidth;
	float fHeight;
	int iRet;

	if ( (tRect.fW <= 0.0f) || (tRect.fH <= 0.0f) ) {
		return XUI_OK;
	}
	if ( __xuiAccordionAlpha(iColor) == 0 ) {
		return XUI_OK;
	}
	tRect = xuiInternalSnapRect(tRect);
	fLeft = tRect.fX;
	fTop = tRect.fY;
	fRight = tRect.fX + tRect.fW - 1.0f;
	fBottom = tRect.fY + tRect.fH - 1.0f;
	if ( fRight < fLeft ) fRight = fLeft;
	if ( fBottom < fTop ) fBottom = fTop;
	fWidth = fRight - fLeft + 1.0f;
	fHeight = fBottom - fTop + 1.0f;

	iRet = XUI_OK;
	if ( bTop ) iRet = __xuiAccordionDrawFill(pProxy, pDraw, (xui_rect_t){fLeft, fTop, fWidth, 1.0f}, iColor);
	if ( (iRet == XUI_OK) && bRight ) iRet = __xuiAccordionDrawFill(pProxy, pDraw, (xui_rect_t){fRight, fTop, 1.0f, fHeight}, iColor);
	if ( (iRet == XUI_OK) && bBottom ) iRet = __xuiAccordionDrawFill(pProxy, pDraw, (xui_rect_t){fLeft, fBottom, fWidth, 1.0f}, iColor);
	if ( (iRet == XUI_OK) && bLeft ) iRet = __xuiAccordionDrawFill(pProxy, pDraw, (xui_rect_t){fLeft, fTop, 1.0f, fHeight}, iColor);
	return iRet;
}

static int __xuiAccordionDrawRectLines(xui_proxy pProxy, xui_draw_context pDraw, xui_rect_t tRect, uint32_t iColor)
{
	return __xuiAccordionDrawRectSideLines(pProxy, pDraw, tRect, iColor, 1, 1, 1, 1);
}

static int __xuiAccordionDrawExpandedHeader(xui_proxy pProxy, xui_draw_context pDraw, xui_rect_t tRect, uint32_t iFill, uint32_t iBorder)
{
	int iRet;

	if ( (tRect.fW <= 0.0f) || (tRect.fH <= 0.0f) ) {
		return XUI_OK;
	}
	iRet = __xuiAccordionDrawFill(pProxy, pDraw, tRect, iFill);
	if ( (iRet != XUI_OK) || (__xuiAccordionAlpha(iBorder) == 0) ) {
		return iRet;
	}
	return __xuiAccordionDrawRectSideLines(pProxy, pDraw, tRect, iBorder, 1, 1, 0, 1);
}

static int __xuiAccordionDrawExpandedIndicator(xui_proxy pProxy, xui_draw_context pDraw, xui_rect_t tRect, uint32_t iIndicator)
{
	if ( (tRect.fW <= 0.0f) || (tRect.fH <= 0.0f) ) {
		return XUI_OK;
	}
	return __xuiAccordionDrawFill(pProxy, pDraw, (xui_rect_t){0.0f, 0.0f, 3.0f, tRect.fH}, iIndicator);
}

static int __xuiAccordionDrawTriangle(xui_proxy pProxy, xui_draw_context pDraw, xui_vec2_t tA, xui_vec2_t tB, xui_vec2_t tC, uint32_t iColor)
{
	int iRet;

	if ( __xuiAccordionAlpha(iColor) == 0 ) {
		return XUI_OK;
	}
	if ( (pProxy != NULL) && (pProxy->drawTriangleFill != NULL) && (pDraw != NULL) ) {
		return pProxy->drawTriangleFill(pProxy, pDraw, tA, tB, tC, iColor);
	}
	iRet = __xuiAccordionDrawLine(pProxy, pDraw, tA.fX, tA.fY, tB.fX, tB.fY, iColor);
	if ( iRet == XUI_OK ) iRet = __xuiAccordionDrawLine(pProxy, pDraw, tB.fX, tB.fY, tC.fX, tC.fY, iColor);
	if ( iRet == XUI_OK ) iRet = __xuiAccordionDrawLine(pProxy, pDraw, tC.fX, tC.fY, tA.fX, tA.fY, iColor);
	return iRet;
}

static int __xuiAccordionDrawText(xui_proxy pProxy, xui_draw_context pDraw, xui_font pFont, const char* sText, xui_rect_t tRect, uint32_t iColor)
{
	if ( (sText == NULL) || (*sText == 0) || (pFont == NULL) ) {
		return XUI_OK;
	}
	if ( (pProxy == NULL) || (pProxy->drawText == NULL) || (pDraw == NULL) ) {
		return XUI_ERROR_NOT_INITIALIZED;
	}
	if ( (tRect.fW <= 0.0f) || (tRect.fH <= 0.0f) || (__xuiAccordionAlpha(iColor) == 0) ) {
		return XUI_OK;
	}
	return pProxy->drawText(pProxy, pDraw, &(xui_text_item_t){.iSize=sizeof(xui_text_item_t), .pFont=pFont, .sText=sText, .iTextSize=-1, .iFlags=XUI_TEXT_SHAPE_DEFAULT | ((XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP) & XUI_TEXT_RTL ? XUI_TEXT_SHAPE_RTL : 0)}, xuiInternalSnapRect(tRect), iColor, XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP);
}

static void __xuiAccordionDefaultCachePolicy(xui_cache_policy_t* pPolicy)
{
	memset(pPolicy, 0, sizeof(*pPolicy));
	pPolicy->iSize = sizeof(*pPolicy);
	pPolicy->iPolicy = XUI_CACHE_POLICY_SELF;
	pPolicy->iFlags = XUI_CACHE_CLEAR_ON_UPDATE | XUI_CACHE_UPDATE_ALL_STATES;
	pPolicy->iClearColor = XUI_COLOR_RGBA(0, 0, 0, 0);
}

static int __xuiAccordionInvalidateSection(const xui_accordion_section_t* pSection, uint32_t iFlags)
{
	int iRet;

	if ( pSection == NULL ) {
		return XUI_OK;
	}
	iRet = XUI_OK;
	if ( pSection->pSection != NULL ) iRet = xuiWidgetInvalidate(pSection->pSection, iFlags);
	if ( iRet == XUI_OK && pSection->pHeader != NULL ) iRet = xuiWidgetInvalidate(pSection->pHeader, iFlags);
	if ( iRet == XUI_OK && pSection->pClient != NULL ) iRet = xuiWidgetInvalidate(pSection->pClient, iFlags);
	if ( iRet != XUI_OK ) return iRet;
	return XUI_OK;
}

static int __xuiAccordionInvalidateHeader(const xui_accordion_data_t* pData, int iIndex,
	uint32_t iFlags)
{
	if ( pData == NULL || iIndex < 0 || iIndex >= pData->iSectionCount ||
	     pData->arrSections[iIndex].pHeader == NULL ) return XUI_OK;
	return xuiWidgetInvalidate(pData->arrSections[iIndex].pHeader, iFlags);
}

static int __xuiAccordionInvalidateAll(xui_widget pAccordion, xui_accordion_data_t* pData, uint32_t iFlags)
{
	int i;

	if ( (pAccordion == NULL) || (pData == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	int iRet;

	iRet = xuiWidgetInvalidate(pAccordion, iFlags);
	if ( iRet != XUI_OK ) return iRet;
	for ( i = 0; i < pData->iSectionCount; i++ ) {
		iRet = __xuiAccordionInvalidateSection(&pData->arrSections[i], iFlags);
		if ( iRet != XUI_OK ) return iRet;
	}
	return XUI_OK;
}

static int __xuiAccordionHeaderIndex(const xui_accordion_data_t* pData, xui_widget pHeader)
{
	int i;

	if ( pData == NULL ) {
		return -1;
	}
	for ( i = 0; i < pData->iSectionCount; i++ ) {
		if ( pData->arrSections[i].pHeader == pHeader ) {
			return i;
		}
	}
	return -1;
}

static int __xuiAccordionClientIndex(const xui_accordion_data_t* pData, xui_widget pClient)
{
	int i;

	if ( pData == NULL ) {
		return -1;
	}
	for ( i = 0; i < pData->iSectionCount; i++ ) {
		if ( pData->arrSections[i].pClient == pClient ) {
			return i;
		}
	}
	return -1;
}

static int __xuiAccordionSectionEnabled(const xui_accordion_data_t* pData, int iIndex)
{
	return (pData != NULL) &&
	       (iIndex >= 0) &&
	       (iIndex < pData->iSectionCount) &&
	       (pData->arrSections[iIndex].bEnabled != 0);
}

static int __xuiAccordionFirstEnabled(const xui_accordion_data_t* pData)
{
	int i;

	if ( pData == NULL ) return -1;
	for ( i = 0; i < pData->iSectionCount; i++ ) {
		if ( __xuiAccordionSectionEnabled(pData, i) ) {
			return i;
		}
	}
	return -1;
}

static int __xuiAccordionLastEnabled(const xui_accordion_data_t* pData)
{
	int i;

	if ( pData == NULL ) return -1;
	for ( i = pData->iSectionCount - 1; i >= 0; i-- ) {
		if ( __xuiAccordionSectionEnabled(pData, i) ) {
			return i;
		}
	}
	return -1;
}

static int __xuiAccordionStepEnabled(const xui_accordion_data_t* pData, int iStart, int iStep)
{
	int i;
	int iFallback;

	if ( (pData == NULL) || (pData->iSectionCount <= 0) ) {
		return -1;
	}
	iFallback = (iStep >= 0) ? __xuiAccordionFirstEnabled(pData) : __xuiAccordionLastEnabled(pData);
	if ( iFallback < 0 ) {
		return -1;
	}
	if ( (iStart < 0) || (iStart >= pData->iSectionCount) ) {
		return iFallback;
	}
	for ( i = iStart + ((iStep >= 0) ? 1 : -1); (i >= 0) && (i < pData->iSectionCount); i += (iStep >= 0) ? 1 : -1 ) {
		if ( __xuiAccordionSectionEnabled(pData, i) ) {
			return i;
		}
	}
	return iFallback;
}

static int __xuiAccordionSetSelected(xui_widget pAccordion, xui_accordion_data_t* pData, int iIndex)
{
	int iOld;
	int iRet;

	if ( (pAccordion == NULL) || (pData == NULL) ||
	     (iIndex < -1) || (iIndex >= pData->iSectionCount) ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( iIndex >= 0 && !__xuiAccordionSectionEnabled(pData, iIndex) ) return XUI_ERROR_INVALID_STATE;
	if ( pData->iSelected == iIndex ) return XUI_OK;
	iOld = pData->iSelected;
	pData->iSelected = iIndex;
	iRet = __xuiAccordionInvalidateAll(pAccordion, pData, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	if ( iRet != XUI_OK ) {
		pData->iSelected = iOld;
		(void)__xuiAccordionInvalidateAll(pAccordion, pData, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
		return iRet;
	}
	xuiInternalAccessibilityQueue(pAccordion, XUI_ACCESSIBLE_EVENT_SELECTION_CHANGED);
	return XUI_OK;
}

static int __xuiAccordionSyncSections(xui_widget pAccordion, xui_accordion_data_t* pData)
{
	int iRet;
	int i;

	if ( (pAccordion == NULL) || (pData == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	for ( i = 0; i < pData->iSectionCount; i++ ) {
		xui_accordion_section_t* pSection = &pData->arrSections[i];
		if ( pSection->pSection != NULL ) {
			iRet = xuiWidgetSetVisible(pSection->pSection, 1);
			if ( iRet == XUI_OK ) iRet = xuiWidgetSetEnabled(pSection->pSection, pSection->bEnabled);
			if ( iRet != XUI_OK ) return iRet;
			if ( __xuiAccordionGetData(pAccordion) != pData ) return XUI_OK;
		}
		if ( pSection->pHeader != NULL ) {
			iRet = xuiWidgetSetEnabled(pSection->pHeader, pSection->bEnabled);
			if ( iRet != XUI_OK ) return iRet;
			if ( __xuiAccordionGetData(pAccordion) != pData ) return XUI_OK;
		}
		if ( pSection->pClient != NULL ) {
			iRet = xuiWidgetSetVisible(pSection->pClient, pSection->bExpanded);
			if ( iRet == XUI_OK ) iRet = xuiWidgetSetEnabled(pSection->pClient, pSection->bEnabled);
			if ( iRet != XUI_OK ) return iRet;
			if ( __xuiAccordionGetData(pAccordion) != pData ) return XUI_OK;
		}
	}
	return __xuiAccordionInvalidateAll(pAccordion, pData,
		XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
}

static int __xuiAccordionSetHover(xui_widget pAccordion, xui_accordion_data_t* pData, int iIndex)
{
	int iOld;
	int iRet;

	if ( (pAccordion == NULL) || (pData == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( iIndex == pData->iHoverIndex ) {
		return XUI_OK;
	}
	iOld = pData->iHoverIndex;
	pData->iHoverIndex = iIndex;
	iRet = __xuiAccordionInvalidateHeader(pData, iOld,
		XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	if ( iRet == XUI_OK ) iRet = __xuiAccordionInvalidateHeader(pData, iIndex,
		XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	if ( iRet != XUI_OK ) {
		pData->iHoverIndex = iOld;
		(void)__xuiAccordionInvalidateHeader(pData, iOld,
			XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
		(void)__xuiAccordionInvalidateHeader(pData, iIndex,
			XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
		return iRet;
	}
	return XUI_OK;
}

static int __xuiAccordionSetActive(xui_widget pAccordion, xui_accordion_data_t* pData, int iIndex)
{
	int iOld;
	int iRet;

	if ( (pAccordion == NULL) || (pData == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( iIndex == pData->iActiveIndex ) {
		return XUI_OK;
	}
	iOld = pData->iActiveIndex;
	pData->iActiveIndex = iIndex;
	iRet = __xuiAccordionInvalidateHeader(pData, iOld,
		XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	if ( iRet == XUI_OK ) iRet = __xuiAccordionInvalidateHeader(pData, iIndex,
		XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	if ( iRet != XUI_OK ) {
		pData->iActiveIndex = iOld;
		(void)__xuiAccordionInvalidateHeader(pData, iOld,
			XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
		(void)__xuiAccordionInvalidateHeader(pData, iIndex,
			XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
		return iRet;
	}
	return XUI_OK;
}

static int __xuiAccordionCancelPress(xui_widget pAccordion, xui_accordion_data_t* pData)
{
	xui_context pContext;
	xui_widget pCapture;
	int iRet;

	if ( (pAccordion == NULL) || (pData == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	pContext = xuiWidgetGetContext(pAccordion);
	pCapture = NULL;
	if ( pData->tPointerOwner.bActive ) {
		pCapture = xuiGetPointerCaptureEx(pContext,
			pData->tPointerOwner.iPointerId, pData->tPointerOwner.iPointerType);
	}
	if ( pCapture != NULL ) {
		iRet = xuiInternalPointerOwnerCancel(pContext, pCapture,
			&pData->tPointerOwner);
		if ( iRet != XUI_OK ) return iRet;
		if ( __xuiAccordionGetData(pAccordion) != pData ) return XUI_OK;
	} else {
		(void)xuiInternalPointerOwnerCancel(pContext, NULL, &pData->tPointerOwner);
	}
	return __xuiAccordionSetActive(pAccordion, pData, -1);
}

static int __xuiAccordionSetExpandedInternal(xui_widget pAccordion, xui_accordion_data_t* pData, int iIndex, int bExpanded, int bNotify)
{
	xui_accordion_select_proc onSelect;
	void* pSelectUser;
	int arrOldExpanded[XUI_ACCORDION_SECTION_CAPACITY];
	int iOldSelected;
	int iOldChangeCount;
	int iId;
	int iRet;
	int i;
	int bChanged;
	int bSelectionChanged;

	if ( (pAccordion == NULL) || (pData == NULL) || (iIndex < 0) || (iIndex >= pData->iSectionCount) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	bExpanded = bExpanded ? 1 : 0;
	for ( i = 0; i < pData->iSectionCount; i++ ) arrOldExpanded[i] = pData->arrSections[i].bExpanded;
	iOldSelected = pData->iSelected;
	iOldChangeCount = pData->iChangeCount;
	bChanged = 0;
	if ( (pData->iMode == XUI_ACCORDION_MODE_SINGLE) && bExpanded ) {
		for ( i = 0; i < pData->iSectionCount; i++ ) {
			if ( (i != iIndex) && pData->arrSections[i].bExpanded ) {
				pData->arrSections[i].bExpanded = 0;
				bChanged = 1;
			}
		}
	}
	if ( pData->arrSections[iIndex].bExpanded != bExpanded ) {
		pData->arrSections[iIndex].bExpanded = bExpanded;
		bChanged = 1;
	}
	if ( bExpanded ) {
		pData->iSelected = iIndex;
	}
	bSelectionChanged = pData->iSelected != iOldSelected;
	if ( !bChanged && !bSelectionChanged ) return XUI_OK;
	if ( bChanged ) pData->iChangeCount++;
	iRet = __xuiAccordionSyncSections(pAccordion, pData);
	if ( iRet != XUI_OK ) {
		for ( i = 0; i < pData->iSectionCount; i++ ) pData->arrSections[i].bExpanded = arrOldExpanded[i];
		pData->iSelected = iOldSelected;
		pData->iChangeCount = iOldChangeCount;
		(void)__xuiAccordionSyncSections(pAccordion, pData);
		return iRet;
	}
	if ( __xuiAccordionGetData(pAccordion) != pData ) return XUI_OK;
	if ( bChanged ) xuiInternalAccessibilityQueue(pAccordion, XUI_ACCESSIBLE_EVENT_STATE_CHANGED);
	if ( bSelectionChanged ) xuiInternalAccessibilityQueue(pAccordion, XUI_ACCESSIBLE_EVENT_SELECTION_CHANGED);
	onSelect = pData->onSelect;
	pSelectUser = pData->pSelectUser;
	iId = pData->arrSections[iIndex].iId;
	if ( bChanged && bNotify && onSelect != NULL ) onSelect(pAccordion, iIndex, iId, pSelectUser);
	return XUI_OK;
}

static int __xuiAccordionClickSection(xui_widget pAccordion, xui_accordion_data_t* pData, int iIndex)
{
	if ( !__xuiAccordionSectionEnabled(pData, iIndex) ) {
		return XUI_OK;
	}
	return __xuiAccordionSetExpandedInternal(pAccordion, pData, iIndex, !pData->arrSections[iIndex].bExpanded, 1);
}

static int __xuiAccordionHeaderEvent(xui_widget pHeader, const xui_event_t* pEvent, void* pUser)
{
	xui_widget pAccordion;
	xui_accordion_data_t* pData;
	xui_rect_t tWorld;
	xui_rect_t tLocal;
	float fX;
	float fY;
	int iIndex;
	int iRet;
	int bInside;

	pAccordion = (xui_widget)pUser;
	pData = __xuiAccordionGetData(pAccordion);
	if ( (pHeader == NULL) || (pEvent == NULL) || (pData == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	iIndex = __xuiAccordionHeaderIndex(pData, pHeader);
	if ( iIndex < 0 ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	tWorld = xuiWidgetGetWorldRect(pHeader);
	fX = pEvent->fX - tWorld.fX;
	fY = pEvent->fY - tWorld.fY;
	tLocal = (xui_rect_t){0.0f, 0.0f, tWorld.fW, tWorld.fH};

	switch ( pEvent->iType ) {
	case XUI_EVENT_POINTER_ENTER:
	case XUI_EVENT_POINTER_MOVE:
		if ( __xuiAccordionSectionEnabled(pData, iIndex) && __xuiAccordionRectContains(tLocal, fX, fY) ) {
			iRet = __xuiAccordionSetHover(pAccordion, pData, iIndex);
		} else if ( pData->iHoverIndex == iIndex ) {
			iRet = __xuiAccordionSetHover(pAccordion, pData, -1);
		} else {
			iRet = XUI_OK;
		}
		if ( iRet != XUI_OK ) return iRet;
		return XUI_EVENT_DISPATCH_STOP;
	case XUI_EVENT_POINTER_LEAVE:
		if ( pData->iHoverIndex == iIndex ) {
			iRet = __xuiAccordionSetHover(pAccordion, pData, -1);
			if ( iRet != XUI_OK ) return iRet;
		}
		return XUI_EVENT_DISPATCH_STOP;
	case XUI_EVENT_POINTER_DOWN:
		if ( (pEvent->iButton == XUI_POINTER_BUTTON_LEFT) && __xuiAccordionSectionEnabled(pData, iIndex) ) {
			if ( pData->tPointerOwner.bActive ) {
				return xuiInternalPointerOwnerMatches(&pData->tPointerOwner, pEvent) ?
					XUI_EVENT_DISPATCH_STOP : XUI_OK;
			}
			iRet = xuiSetFocusWidget(xuiWidgetGetContext(pAccordion), pAccordion);
			if ( iRet != XUI_OK ) return iRet;
			pData = __xuiAccordionGetData(pAccordion);
			if ( pData == NULL ) return XUI_EVENT_DISPATCH_STOP;
			iRet = __xuiAccordionSetSelected(pAccordion, pData, iIndex);
			if ( iRet != XUI_OK ) return iRet;
			iRet = __xuiAccordionSetActive(pAccordion, pData, iIndex);
			if ( iRet != XUI_OK ) return iRet;
			iRet = xuiInternalPointerOwnerBegin(xuiWidgetGetContext(pAccordion),
				pHeader, pEvent, &pData->tPointerOwner);
			if ( iRet != XUI_OK ) {
				(void)__xuiAccordionSetActive(pAccordion, pData, -1);
				return iRet;
			}
			return XUI_EVENT_DISPATCH_STOP;
		}
		break;
	case XUI_EVENT_POINTER_UP:
		if ( pData->iActiveIndex == iIndex &&
		     xuiInternalPointerOwnerMatches(&pData->tPointerOwner, pEvent) ) {
			bInside = __xuiAccordionRectContains(tLocal, fX, fY);
			iRet = __xuiAccordionCancelPress(pAccordion, pData);
			if ( iRet != XUI_OK ) return iRet;
			pData = __xuiAccordionGetData(pAccordion);
			if ( pData != NULL && bInside ) {
				iRet = __xuiAccordionClickSection(pAccordion, pData, iIndex);
				if ( iRet != XUI_OK ) return iRet;
			}
			return XUI_EVENT_DISPATCH_STOP;
		}
		break;
	case XUI_EVENT_POINTER_CAPTURE_LOST:
		if ( pData->iActiveIndex == iIndex &&
		     xuiInternalPointerOwnerCaptureLost(&pData->tPointerOwner, pEvent) ) {
			iRet = __xuiAccordionSetActive(pAccordion, pData, -1);
			if ( iRet != XUI_OK ) return iRet;
		}
		return XUI_EVENT_DISPATCH_STOP;
	default:
		break;
	}
	return XUI_OK;
}

static int __xuiAccordionEvent(xui_widget pWidget, const xui_event_t* pEvent, void* pUser)
{
	xui_accordion_data_t* pData;
	int iNext;
	int iRet;

	(void)pUser;
	pData = __xuiAccordionGetData(pWidget);
	if ( (pData == NULL) || (pEvent == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	switch ( pEvent->iType ) {
	case XUI_EVENT_KEY_DOWN:
		if ( !xuiWidgetGetEffectiveEnabled(pWidget) ) {
			return XUI_OK;
		}
		if ( pEvent->iKey == XUI_KEY_DOWN ) {
			iNext = __xuiAccordionStepEnabled(pData, pData->iSelected, 1);
			if ( iNext >= 0 ) {
				iRet = __xuiAccordionSetSelected(pWidget, pData, iNext);
				if ( iRet != XUI_OK ) return iRet;
				return XUI_EVENT_DISPATCH_STOP;
			}
		} else if ( pEvent->iKey == XUI_KEY_UP ) {
			iNext = __xuiAccordionStepEnabled(pData, pData->iSelected, -1);
			if ( iNext >= 0 ) {
				iRet = __xuiAccordionSetSelected(pWidget, pData, iNext);
				if ( iRet != XUI_OK ) return iRet;
				return XUI_EVENT_DISPATCH_STOP;
			}
		} else if ( pEvent->iKey == XUI_KEY_HOME ) {
			iNext = __xuiAccordionFirstEnabled(pData);
			if ( iNext >= 0 ) {
				iRet = __xuiAccordionSetSelected(pWidget, pData, iNext);
				if ( iRet != XUI_OK ) return iRet;
				return XUI_EVENT_DISPATCH_STOP;
			}
		} else if ( pEvent->iKey == XUI_KEY_END ) {
			iNext = __xuiAccordionLastEnabled(pData);
			if ( iNext >= 0 ) {
				iRet = __xuiAccordionSetSelected(pWidget, pData, iNext);
				if ( iRet != XUI_OK ) return iRet;
				return XUI_EVENT_DISPATCH_STOP;
			}
		} else if ( (pEvent->iKey == XUI_KEY_RIGHT) || (pEvent->iKey == XUI_KEY_LEFT) ) {
			iNext = pData->iSelected;
			if ( (iNext < 0) || !__xuiAccordionSectionEnabled(pData, iNext) ) {
				iNext = __xuiAccordionFirstEnabled(pData);
			}
			if ( iNext >= 0 ) {
				iRet = __xuiAccordionSetSelected(pWidget, pData, iNext);
				if ( iRet != XUI_OK ) return iRet;
				pData = __xuiAccordionGetData(pWidget);
				if ( pData == NULL ) return XUI_EVENT_DISPATCH_STOP;
				iRet = __xuiAccordionSetExpandedInternal(pWidget, pData, iNext,
					pEvent->iKey == XUI_KEY_RIGHT, 1);
				if ( iRet != XUI_OK ) return iRet;
				return XUI_EVENT_DISPATCH_STOP;
			}
		} else if ( (pEvent->iKey == XUI_KEY_SPACE) || (pEvent->iKey == XUI_KEY_ENTER) ) {
			iNext = pData->iSelected;
			if ( (iNext < 0) || (iNext >= pData->iSectionCount) || !__xuiAccordionSectionEnabled(pData, iNext) ) {
				iNext = __xuiAccordionFirstEnabled(pData);
			}
			if ( iNext >= 0 ) {
				iRet = __xuiAccordionClickSection(pWidget, pData, iNext);
				if ( iRet != XUI_OK ) return iRet;
				return XUI_EVENT_DISPATCH_STOP;
			}
		} else if ( pEvent->iKey == XUI_KEY_ESCAPE ) {
			iRet = __xuiAccordionCancelPress(pWidget, pData);
			if ( iRet != XUI_OK ) return iRet;
			return XUI_EVENT_DISPATCH_STOP;
		}
		break;
	case XUI_EVENT_FOCUS:
		xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_STATE_CHANGED);
		return XUI_EVENT_DISPATCH_STOP;
	case XUI_EVENT_ENABLED_CHANGED:
	case XUI_EVENT_VISIBLE_CHANGED:
		if ( !xuiWidgetGetEffectiveEnabled(pWidget) ||
		     !xuiWidgetGetEffectiveVisible(pWidget) ) {
			iRet = __xuiAccordionSetHover(pWidget, pData, -1);
			if ( iRet != XUI_OK ) return iRet;
			iRet = __xuiAccordionCancelPress(pWidget, pData);
			if ( iRet != XUI_OK ) return iRet;
		}
		iRet = __xuiAccordionSyncSections(pWidget, pData);
		if ( iRet != XUI_OK ) return iRet;
		if ( __xuiAccordionGetData(pWidget) != pData ) return XUI_EVENT_DISPATCH_STOP;
		xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_STATE_CHANGED);
		return XUI_EVENT_DISPATCH_STOP;
	case XUI_EVENT_BLUR:
		iRet = __xuiAccordionCancelPress(pWidget, pData);
		if ( iRet != XUI_OK ) return iRet;
		pData = __xuiAccordionGetData(pWidget);
		if ( pData == NULL ) return XUI_EVENT_DISPATCH_STOP;
		iRet = __xuiAccordionSyncSections(pWidget, pData);
		if ( iRet != XUI_OK ) return iRet;
		if ( __xuiAccordionGetData(pWidget) != pData ) return XUI_EVENT_DISPATCH_STOP;
		xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_STATE_CHANGED);
		return XUI_EVENT_DISPATCH_STOP;
	default:
		break;
	}
	return XUI_OK;
}

static int __xuiAccordionHeaderRender(xui_widget pHeader, xui_draw_context pDraw, uint32_t iStateId, void* pUser)
{
	xui_widget pAccordion;
	xui_accordion_data_t* pData;
	xui_accordion_resolved_t tResolved;
	xui_accordion_section_t* pSection;
	xui_proxy pProxy;
	xui_rect_t tRect;
	xui_rect_t tArrow;
	xui_rect_t tText;
	uint32_t iFill;
	uint32_t iText;
	uint32_t iIndicator;
	int iIndex;
	int bEnabled;
	int iRet;

	(void)iStateId;
	pAccordion = (xui_widget)pUser;
	pData = __xuiAccordionGetData(pAccordion);
	if ( (pHeader == NULL) || (pData == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	iIndex = __xuiAccordionHeaderIndex(pData, pHeader);
	if ( iIndex < 0 ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pSection = &pData->arrSections[iIndex];
	pProxy = xuiInternalContextGetProxy(xuiWidgetGetContext(pAccordion));
	if ( pProxy == NULL ) {
		return XUI_ERROR_NOT_INITIALIZED;
	}
	__xuiAccordionResolve(pAccordion, pData, &tResolved);
	tRect = xuiWidgetGetRect(pHeader);
	tRect.fX = 0.0f;
	tRect.fY = 0.0f;
	iFill = tResolved.iHeaderColor;
	iText = tResolved.iTextColor;
	bEnabled = xuiWidgetGetEffectiveEnabled(pHeader) && pSection->bEnabled;
	if ( !bEnabled ) {
		iText = tResolved.iDisabledTextColor;
	} else if ( pData->iActiveIndex == iIndex ) {
		iFill = tResolved.iHoverColor;
	} else if ( pSection->bExpanded ) {
		iFill = tResolved.iExpandedColor;
		iText = tResolved.iActiveTextColor;
	} else if ( pData->iHoverIndex == iIndex ) {
		iFill = tResolved.iHoverColor;
	}
	if ( pSection->bExpanded ) {
		iRet = __xuiAccordionDrawExpandedHeader(pProxy, pDraw, tRect, iFill, tResolved.iBorderColor);
	} else {
		iRet = __xuiAccordionDrawRectFill(pProxy, pDraw, tRect, iFill);
		if ( iRet == XUI_OK ) iRet = __xuiAccordionDrawRectStroke(pProxy, pDraw, tRect, tResolved.iBorderColor);
	}
	if ( (iRet == XUI_OK) && bEnabled && pSection->bExpanded ) {
		iIndicator = tResolved.iIndicatorColor;
		iRet = __xuiAccordionDrawExpandedIndicator(pProxy, pDraw, tRect, iIndicator);
	}
	if ( iRet != XUI_OK ) return iRet;
	tArrow = (xui_rect_t){8.0f, (tRect.fH - 12.0f) * 0.5f, 12.0f, 12.0f};
	tText = (xui_rect_t){28.0f, 0.0f, __xuiAccordionMaxFloat(0.0f, tRect.fW - 36.0f), tRect.fH};
	pSection->tArrowRect = (xui_rect_t){pSection->tHeaderRect.fX + tArrow.fX, pSection->tHeaderRect.fY + tArrow.fY, tArrow.fW, tArrow.fH};
	pSection->tTextRect = (xui_rect_t){pSection->tHeaderRect.fX + tText.fX, pSection->tHeaderRect.fY + tText.fY, tText.fW, tText.fH};
	if ( pSection->bExpanded ) {
		iRet = __xuiAccordionDrawTriangle(pProxy, pDraw,
			(xui_vec2_t){tArrow.fX + 1.0f, tArrow.fY + 4.0f},
			(xui_vec2_t){tArrow.fX + 11.0f, tArrow.fY + 4.0f},
			(xui_vec2_t){tArrow.fX + 6.0f, tArrow.fY + 9.0f},
			iText);
	} else {
		iRet = __xuiAccordionDrawTriangle(pProxy, pDraw,
			(xui_vec2_t){tArrow.fX + 4.0f, tArrow.fY + 1.0f},
			(xui_vec2_t){tArrow.fX + 9.0f, tArrow.fY + 6.0f},
			(xui_vec2_t){tArrow.fX + 4.0f, tArrow.fY + 11.0f},
			iText);
	}
	if ( iRet == XUI_OK ) {
		iRet = __xuiAccordionDrawText(pProxy, pDraw, tResolved.pFont, pSection->sTitle, tText, iText);
	}
	return iRet;
}

static int __xuiAccordionClientRender(xui_widget pClient, xui_draw_context pDraw, uint32_t iStateId, void* pUser)
{
	xui_widget pAccordion;
	xui_accordion_data_t* pData;
	xui_accordion_resolved_t tResolved;
	xui_proxy pProxy;
	xui_rect_t tRect;
	int iIndex;
	int iRet;

	(void)iStateId;
	pAccordion = (xui_widget)pUser;
	pData = __xuiAccordionGetData(pAccordion);
	if ( (pClient == NULL) || (pData == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	iIndex = __xuiAccordionClientIndex(pData, pClient);
	if ( iIndex < 0 ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pProxy = xuiInternalContextGetProxy(xuiWidgetGetContext(pAccordion));
	if ( pProxy == NULL ) {
		return XUI_ERROR_NOT_INITIALIZED;
	}
	__xuiAccordionResolve(pAccordion, pData, &tResolved);
	tRect = xuiWidgetGetRect(pClient);
	tRect.fX = 0.0f;
	tRect.fY = 0.0f;
	iRet = __xuiAccordionDrawFill(pProxy, pDraw, tRect, tResolved.iContentColor);
	if ( iRet == XUI_OK ) iRet = __xuiAccordionDrawRectLines(pProxy, pDraw, tRect, tResolved.iBorderColor);
	return iRet;
}

static int __xuiAccordionCacheRender(xui_widget pWidget, xui_draw_context pDraw, uint32_t iStateId, void* pUser)
{
	xui_accordion_data_t* pData;
	xui_accordion_resolved_t tResolved;
	xui_proxy pProxy;
	xui_rect_t tRect;

	(void)iStateId;
	(void)pUser;
	pData = __xuiAccordionGetData(pWidget);
	if ( (pData == NULL) || (pDraw == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pProxy = xuiInternalContextGetProxy(xuiWidgetGetContext(pWidget));
	if ( pProxy == NULL ) {
		return XUI_ERROR_NOT_INITIALIZED;
	}
	__xuiAccordionResolve(pWidget, pData, &tResolved);
	tRect = xuiWidgetGetRect(pWidget);
	tRect.fX = 0.0f;
	tRect.fY = 0.0f;
	return __xuiAccordionDrawFill(pProxy, pDraw, tRect, tResolved.iBackgroundColor);
}

static int __xuiAccordionUpdateClientPadding(xui_accordion_data_t* pData, float fContentPadding)
{
	xui_thickness_t tPadding;
	int iRet;
	int i;

	if ( pData == NULL ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	tPadding = __xuiAccordionThickness(fContentPadding, fContentPadding, fContentPadding, fContentPadding);
	for ( i = 0; i < pData->iSectionCount; i++ ) {
		if ( pData->arrSections[i].pClient != NULL ) {
			iRet = xuiWidgetSetPadding(pData->arrSections[i].pClient, tPadding);
			if ( iRet != XUI_OK ) return iRet;
		}
	}
	return XUI_OK;
}

static int __xuiAccordionContentMeasure(xui_widget pWidget, xui_vec2_t tConstraint, xui_vec2_t* pSize, void* pUser)
{
	(void)pWidget;
	(void)tConstraint;
	(void)pUser;
	if ( pSize == NULL ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pSize->fX = 320.0f;
	pSize->fY = 0.0f;
	return XUI_OK;
}

static int __xuiAccordionPrepare(xui_widget pWidget, void* pUser)
{
	xui_accordion_data_t* pData;
	xui_accordion_resolved_t tResolved;
	int iRet;
	int i;

	(void)pUser;
	pData = __xuiAccordionGetData(pWidget);
	if ( pData == NULL ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	__xuiAccordionResolve(pWidget, pData, &tResolved);
	iRet = xuiWidgetSetGap(pWidget, tResolved.fSpacing);
	if ( iRet != XUI_OK ) return iRet;
	for ( i = 0; i < pData->iSectionCount; i++ ) {
		if ( pData->arrSections[i].pHeader != NULL ) {
			iRet = xuiWidgetSetPreferredSize(pData->arrSections[i].pHeader,
				(xui_vec2_t){0.0f, tResolved.fHeaderHeight});
			if ( iRet != XUI_OK ) return iRet;
		}
		if ( pData->arrSections[i].pClient != NULL ) {
			iRet = xuiWidgetSetPadding(pData->arrSections[i].pClient,
				__xuiAccordionThickness(tResolved.fContentPadding, tResolved.fContentPadding,
					tResolved.fContentPadding, tResolved.fContentPadding));
			if ( iRet != XUI_OK ) return iRet;
		}
	}
	return XUI_OK;
}

static int __xuiAccordionLayoutComplete(xui_widget pWidget, xui_rect_t tContentRect, void* pUser)
{
	xui_accordion_data_t* pData;
	xui_accordion_section_t* pSection;
	xui_rect_t tHeader;
	xui_rect_t tClient;
	float fBottom;
	int i;

	(void)pUser;
	pData = __xuiAccordionGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	pData->fContentHeight = 0.0f;
	for ( i = 0; i < pData->iSectionCount; i++ ) {
		pSection = &pData->arrSections[i];
		pSection->tSectionRect = xuiWidgetGetRect(pSection->pSection);
		tHeader = xuiWidgetGetRect(pSection->pHeader);
		pSection->tHeaderRect = (xui_rect_t){
			pSection->tSectionRect.fX + tHeader.fX,
			pSection->tSectionRect.fY + tHeader.fY, tHeader.fW, tHeader.fH};
		if ( pSection->bExpanded ) {
			tClient = xuiWidgetGetRect(pSection->pClient);
			pSection->tClientRect = (xui_rect_t){
				pSection->tSectionRect.fX + tClient.fX,
				pSection->tSectionRect.fY + tClient.fY, tClient.fW, tClient.fH};
		} else {
			pSection->tClientRect = (xui_rect_t){
				pSection->tHeaderRect.fX,
				pSection->tHeaderRect.fY + pSection->tHeaderRect.fH,
				pSection->tHeaderRect.fW, 0.0f};
		}
		pSection->tArrowRect = (xui_rect_t){pSection->tHeaderRect.fX + 8.0f,
			pSection->tHeaderRect.fY + (pSection->tHeaderRect.fH - 12.0f) * 0.5f, 12.0f, 12.0f};
		pSection->tTextRect = (xui_rect_t){pSection->tHeaderRect.fX + 28.0f,
			pSection->tHeaderRect.fY, __xuiAccordionMaxFloat(0.0f, pSection->tHeaderRect.fW - 36.0f),
			pSection->tHeaderRect.fH};
		fBottom = pSection->tSectionRect.fY + pSection->tSectionRect.fH - tContentRect.fY;
		if ( fBottom > pData->fContentHeight ) pData->fContentHeight = fBottom;
	}
	return XUI_OK;
}

static void __xuiAccordionDefaultLayout(xui_layout_t* pLayout)
{
	memset(pLayout, 0, sizeof(*pLayout));
	pLayout->iLayoutType = XUI_LAYOUT_COLUMN;
	pLayout->iWidthMode = XUI_SIZE_FIXED;
	pLayout->iHeightMode = XUI_SIZE_CONTENT;
	pLayout->iFlowMode = XUI_FLOW_BLOCK;
	pLayout->iDock = XUI_DOCK_FILL;
	pLayout->iOverflow = XUI_OVERFLOW_CLIP;
	pLayout->iAlignX = XUI_ALIGN_START;
	pLayout->iAlignY = XUI_ALIGN_START;
	pLayout->iTableRowSpan = 1;
	pLayout->iTableColumnSpan = 1;
	pLayout->iGridColumnCount = 1;
	pLayout->fPreferredWidth = 320.0f;
	pLayout->fPreferredHeight = 180.0f;
	pLayout->fMaxWidth = XUI_LAYOUT_UNBOUNDED;
	pLayout->fMaxHeight = XUI_LAYOUT_UNBOUNDED;
	pLayout->fShrink = 1.0f;
}

static int __xuiAccordionAccessibleCount(xui_widget pWidget, void* pUser)
{
	const xui_accordion_data_t* pData = (const xui_accordion_data_t*)pUser;
	(void)pWidget;
	return pData != NULL ? 1 + pData->iSectionCount : 0;
}

static int __xuiAccordionAccessibleGet(xui_widget pWidget, int iIndex,
	xui_accessible_node_t* pNode, void* pUser)
{
	xui_accordion_data_t* pData = (xui_accordion_data_t*)pUser;
	xui_accordion_section_t* pSection;
	xui_rect_t tWorld;
	int iSection;

	if ( pData == NULL || pNode == NULL || iIndex < 0 ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( iIndex == 0 ) {
		pNode->iId = 1;
		pNode->iRole = XUI_ACCESSIBLE_ROLE_GROUP;
		pNode->sName = xuiWidgetGetAccessibleName(pWidget) != NULL ?
			xuiWidgetGetAccessibleName(pWidget) : "Accordion";
		pNode->sDescription = xuiWidgetGetAccessibleDescription(pWidget);
		pNode->tBounds = xuiWidgetGetWorldRect(pWidget);
		pNode->iState |= XUI_ACCESSIBLE_STATE_FOCUSABLE | XUI_ACCESSIBLE_STATE_VERTICAL;
		if ( xuiGetFocusWidget(xuiWidgetGetContext(pWidget)) == pWidget ) {
			pNode->iState |= XUI_ACCESSIBLE_STATE_FOCUSED;
		}
		if ( !xuiWidgetGetEffectiveEnabled(pWidget) ) pNode->iState |= XUI_ACCESSIBLE_STATE_DISABLED;
		if ( !xuiWidgetGetEffectiveVisible(pWidget) ) pNode->iState |= XUI_ACCESSIBLE_STATE_OFFSCREEN;
		pNode->iRowCount = pData->iSectionCount;
		pNode->iColumnCount = 1;
		pNode->iActions = XUI_ACCESSIBLE_ACTION_MASK(XUI_ACCESSIBLE_ACTION_FOCUS);
		return XUI_OK;
	}
	iSection = iIndex - 1;
	if ( iSection < 0 || iSection >= pData->iSectionCount ) return XUI_ERROR_INVALID_ARGUMENT;
	pSection = &pData->arrSections[iSection];
	tWorld = pSection->pHeader != NULL ? xuiWidgetGetWorldRect(pSection->pHeader) :
		(xui_rect_t){0.0f, 0.0f, 0.0f, 0.0f};
	pNode->iId = (uint64_t)iSection + 2u;
	pNode->iParentId = 1;
	pNode->iRole = XUI_ACCESSIBLE_ROLE_BUTTON;
	pNode->sName = pSection->sTitle;
	pNode->tBounds = tWorld;
	pNode->iRow = iSection;
	pNode->iColumn = 0;
	pNode->iRowCount = 1;
	pNode->iColumnCount = 1;
	pNode->iState |= XUI_ACCESSIBLE_STATE_FOCUSABLE | XUI_ACCESSIBLE_STATE_SELECTABLE |
		(pSection->bExpanded ? XUI_ACCESSIBLE_STATE_EXPANDED : XUI_ACCESSIBLE_STATE_COLLAPSED);
	if ( !pSection->bEnabled || !xuiWidgetGetEffectiveEnabled(pWidget) ) pNode->iState |= XUI_ACCESSIBLE_STATE_DISABLED;
	if ( pSection->pHeader == NULL || !xuiWidgetGetEffectiveVisible(pSection->pHeader) ||
	     tWorld.fW <= 0.0f || tWorld.fH <= 0.0f ) pNode->iState |= XUI_ACCESSIBLE_STATE_OFFSCREEN;
	if ( pData->iSelected == iSection ) {
		pNode->iState |= XUI_ACCESSIBLE_STATE_SELECTED;
		if ( xuiGetFocusWidget(xuiWidgetGetContext(pWidget)) == pWidget ) {
			pNode->iState |= XUI_ACCESSIBLE_STATE_FOCUSED;
		}
	}
	pNode->iActions = XUI_ACCESSIBLE_ACTION_MASK(XUI_ACCESSIBLE_ACTION_FOCUS) |
		XUI_ACCESSIBLE_ACTION_MASK(XUI_ACCESSIBLE_ACTION_ACTIVATE) |
		XUI_ACCESSIBLE_ACTION_MASK(XUI_ACCESSIBLE_ACTION_TOGGLE) |
		XUI_ACCESSIBLE_ACTION_MASK(XUI_ACCESSIBLE_ACTION_EXPAND) |
		XUI_ACCESSIBLE_ACTION_MASK(XUI_ACCESSIBLE_ACTION_COLLAPSE);
	return XUI_OK;
}

static int __xuiAccordionAccessibleAction(xui_widget pWidget, uint64_t iNodeId,
	int iAction, const void* pPayload, void* pUser)
{
	xui_accordion_data_t* pData = (xui_accordion_data_t*)pUser;
	int iSection;
	int iRet;

	(void)pPayload;
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( iNodeId == 0 || iNodeId == 1 ) {
		return iAction == XUI_ACCESSIBLE_ACTION_FOCUS ?
			xuiSetFocusWidget(xuiWidgetGetContext(pWidget), pWidget) : XUI_ERROR_UNSUPPORTED;
	}
	if ( iNodeId < 2 || iNodeId > (uint64_t)pData->iSectionCount + 1u ) return XUI_ERROR_NOT_FOUND;
	iSection = (int)iNodeId - 2;
	if ( !__xuiAccordionSectionEnabled(pData, iSection) ) return XUI_ERROR_INVALID_STATE;
	if ( iAction == XUI_ACCESSIBLE_ACTION_FOCUS ) {
		iRet = __xuiAccordionSetSelected(pWidget, pData, iSection);
		if ( iRet != XUI_OK ) return iRet;
		if ( __xuiAccordionGetData(pWidget) != pData ) return XUI_OK;
		return xuiSetFocusWidget(xuiWidgetGetContext(pWidget), pWidget);
	}
	if ( iAction == XUI_ACCESSIBLE_ACTION_ACTIVATE || iAction == XUI_ACCESSIBLE_ACTION_TOGGLE ) {
		return __xuiAccordionClickSection(pWidget, pData, iSection);
	}
	if ( iAction == XUI_ACCESSIBLE_ACTION_EXPAND || iAction == XUI_ACCESSIBLE_ACTION_COLLAPSE ) {
		return __xuiAccordionSetExpandedInternal(pWidget, pData, iSection,
			iAction == XUI_ACCESSIBLE_ACTION_EXPAND, 1);
	}
	return XUI_ERROR_UNSUPPORTED;
}

static void __xuiAccordionDefaultData(xui_accordion_data_t* pData)
{
	int i;

	memset(pData, 0, sizeof(*pData));
	pData->iMode = XUI_ACCORDION_MODE_MULTIPLE;
	pData->iSelected = -1;
	pData->iHoverIndex = -1;
	pData->iActiveIndex = -1;
	pData->fHeaderHeight = 28.0f;
	pData->fSpacing = 4.0f;
	pData->fContentPadding = 8.0f;
	pData->iBackgroundColor = XUI_COLOR_RGBA(246, 251, 255, 255);
	pData->iHeaderColor = XUI_COLOR_RGBA(220, 236, 248, 255);
	pData->iHoverColor = XUI_COLOR_RGBA(220, 236, 248, 255);
	pData->iExpandedColor = XUI_COLOR_RGBA(220, 236, 248, 255);
	pData->iContentColor = XUI_COLOR_RGBA(248, 252, 255, 255);
	pData->iBorderColor = XUI_ACCORDION_DEFAULT_BORDER_COLOR;
	pData->iTextColor = XUI_COLOR_RGBA(28, 46, 64, 255);
	pData->iActiveTextColor = XUI_COLOR_RGBA(28, 46, 64, 255);
	pData->iDisabledTextColor = XUI_COLOR_RGBA(132, 148, 160, 255);
	for ( i = 0; i < XUI_ACCORDION_SECTION_CAPACITY; i++ ) {
		pData->arrSections[i].bEnabled = 1;
	}
}

static int __xuiAccordionConfigureSectionWidget(xui_widget pWidget)
{
	int iRet;

	if ( pWidget == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	iRet = xuiWidgetSetLayoutType(pWidget, XUI_LAYOUT_COLUMN);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetSizeMode(pWidget, XUI_SIZE_FILL, XUI_SIZE_CONTENT);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetFlowMode(pWidget, XUI_FLOW_BLOCK);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetAlign(pWidget, XUI_ALIGN_STRETCH, XUI_ALIGN_START);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetOverflow(pWidget, XUI_OVERFLOW_CLIP);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetFocusable(pWidget, 0);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetTabStop(pWidget, 0);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetHitTestVisible(pWidget, 1);
	return iRet;
}

static int __xuiAccordionConfigureHeaderWidget(xui_widget pAccordion, xui_widget pHeader)
{
	xui_cache_policy_t tPolicy;
	int iRet;

	if ( (pAccordion == NULL) || (pHeader == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	__xuiAccordionDefaultCachePolicy(&tPolicy);
	iRet = xuiWidgetSetCachePolicy(pHeader, &tPolicy);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetLayoutType(pHeader, XUI_LAYOUT_MANUAL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetSizeMode(pHeader, XUI_SIZE_FILL, XUI_SIZE_FIXED);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetFlowMode(pHeader, XUI_FLOW_BLOCK);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetAlign(pHeader, XUI_ALIGN_STRETCH, XUI_ALIGN_START);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetOverflow(pHeader, XUI_OVERFLOW_CLIP);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetFocusable(pHeader, 0);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetTabStop(pHeader, 0);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pHeader, XUI_EVENT_POINTER_ENTER, __xuiAccordionHeaderEvent, pAccordion);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pHeader, XUI_EVENT_POINTER_LEAVE, __xuiAccordionHeaderEvent, pAccordion);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pHeader, XUI_EVENT_POINTER_MOVE, __xuiAccordionHeaderEvent, pAccordion);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pHeader, XUI_EVENT_POINTER_DOWN, __xuiAccordionHeaderEvent, pAccordion);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pHeader, XUI_EVENT_POINTER_UP, __xuiAccordionHeaderEvent, pAccordion);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pHeader, XUI_EVENT_POINTER_CAPTURE_LOST, __xuiAccordionHeaderEvent, pAccordion);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetCacheRenderCallback(pHeader, __xuiAccordionHeaderRender, pAccordion);
	return iRet;
}

static int __xuiAccordionConfigureClientWidget(xui_widget pAccordion, xui_widget pWidget, float fContentPadding)
{
	xui_cache_policy_t tPolicy;
	int iRet;

	if ( (pAccordion == NULL) || (pWidget == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	__xuiAccordionDefaultCachePolicy(&tPolicy);
	iRet = xuiWidgetSetCachePolicy(pWidget, &tPolicy);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetLayoutType(pWidget, XUI_LAYOUT_COLUMN);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetSizeMode(pWidget, XUI_SIZE_FILL, XUI_SIZE_CONTENT);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetPadding(pWidget, __xuiAccordionThickness(fContentPadding, fContentPadding, fContentPadding, fContentPadding));
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetGap(pWidget, 6.0f);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetOverflow(pWidget, XUI_OVERFLOW_CLIP);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetFocusable(pWidget, 0);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetTabStop(pWidget, 0);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetAlign(pWidget, XUI_ALIGN_STRETCH, XUI_ALIGN_START);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetFlowMode(pWidget, XUI_FLOW_BLOCK);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetCacheRenderCallback(pWidget, __xuiAccordionClientRender, pAccordion);
	return iRet;
}

static int __xuiAccordionCreateSectionWidgets(xui_widget pAccordion, xui_accordion_data_t* pData, int iIndex)
{
	xui_context pContext;
	xui_widget pSection;
	xui_widget pHeader;
	xui_widget pClient;
	int iRet;
	int bHeaderAdded;
	int bClientAdded;

	if ( (pAccordion == NULL) || (pData == NULL) || (iIndex < 0) || (iIndex >= XUI_ACCORDION_SECTION_CAPACITY) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pContext = xuiWidgetGetContext(pAccordion);
	pSection = NULL;
	pHeader = NULL;
	pClient = NULL;
	bHeaderAdded = 0;
	bClientAdded = 0;
	iRet = xuiWidgetCreate(pContext, &pSection);
	if ( iRet == XUI_OK ) iRet = xuiWidgetCreate(pContext, &pHeader);
	if ( iRet == XUI_OK ) iRet = xuiWidgetCreate(pContext, &pClient);
	if ( iRet == XUI_OK ) iRet = __xuiAccordionConfigureSectionWidget(pSection);
	if ( iRet == XUI_OK ) iRet = __xuiAccordionConfigureHeaderWidget(pAccordion, pHeader);
	if ( iRet == XUI_OK ) iRet = __xuiAccordionConfigureClientWidget(pAccordion, pClient, pData->fContentPadding);
	if ( iRet == XUI_OK ) iRet = xuiWidgetAddChild(pSection, pHeader);
	if ( iRet == XUI_OK ) bHeaderAdded = 1;
	if ( iRet == XUI_OK ) iRet = xuiWidgetAddChild(pSection, pClient);
	if ( iRet == XUI_OK ) bClientAdded = 1;
	if ( iRet == XUI_OK ) iRet = xuiWidgetAddChild(pAccordion, pSection);
	if ( iRet != XUI_OK ) {
		if ( pSection != NULL ) xuiWidgetDestroy(pSection);
		if ( !bHeaderAdded && (pHeader != NULL) ) xuiWidgetDestroy(pHeader);
		if ( !bClientAdded && (pClient != NULL) ) xuiWidgetDestroy(pClient);
		return iRet;
	}
	pData->arrSections[iIndex].pSection = pSection;
	pData->arrSections[iIndex].pHeader = pHeader;
	pData->arrSections[iIndex].pClient = pClient;
	return XUI_OK;
}

static int __xuiAccordionInit(xui_widget pWidget, void* pTypeData, const void* pCreateData, void* pUser)
{
	xui_accordion_data_t* pData;
	const xui_accordion_desc_t* pDesc;
	int iRet;
	int i;

	(void)pUser;
	pData = (xui_accordion_data_t*)pTypeData;
	pDesc = (const xui_accordion_desc_t*)pCreateData;
	if ( (pWidget == NULL) || (pData == NULL) || !__xuiAccordionDescValid(pDesc) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	__xuiAccordionDefaultData(pData);
	if ( pDesc != NULL ) {
		if ( __xuiAccordionModeValid(pDesc->iMode) ) pData->iMode = pDesc->iMode;
		if ( pDesc->pFont != NULL ) pData->pFont = pDesc->pFont;
		if ( pDesc->fHeaderHeight > 0.0f ) pData->fHeaderHeight = pDesc->fHeaderHeight;
		if ( pDesc->fSpacing > 0.0f ) pData->fSpacing = pDesc->fSpacing;
		if ( pDesc->fContentPadding > 0.0f ) pData->fContentPadding = pDesc->fContentPadding;
		if ( __xuiAccordionAlpha(pDesc->iBackgroundColor) != 0 ) pData->iBackgroundColor = pDesc->iBackgroundColor;
		if ( __xuiAccordionAlpha(pDesc->iHeaderColor) != 0 ) pData->iHeaderColor = pDesc->iHeaderColor;
		if ( __xuiAccordionAlpha(pDesc->iHoverColor) != 0 ) pData->iHoverColor = pDesc->iHoverColor;
		if ( __xuiAccordionAlpha(pDesc->iExpandedColor) != 0 ) pData->iExpandedColor = pDesc->iExpandedColor;
		if ( __xuiAccordionAlpha(pDesc->iContentColor) != 0 ) pData->iContentColor = pDesc->iContentColor;
		if ( __xuiAccordionAlpha(pDesc->iBorderColor) != 0 ) pData->iBorderColor = pDesc->iBorderColor;
		if ( __xuiAccordionAlpha(pDesc->iTextColor) != 0 ) pData->iTextColor = pDesc->iTextColor;
		if ( __xuiAccordionAlpha(pDesc->iActiveTextColor) != 0 ) pData->iActiveTextColor = pDesc->iActiveTextColor;
		if ( __xuiAccordionAlpha(pDesc->iDisabledTextColor) != 0 ) pData->iDisabledTextColor = pDesc->iDisabledTextColor;
	}
	iRet = xuiWidgetSetLayoutType(pWidget, XUI_LAYOUT_COLUMN);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetOverflow(pWidget, XUI_OVERFLOW_CLIP);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetFocusable(pWidget, 1);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetTabStop(pWidget, 1);
	if ( iRet != XUI_OK ) return iRet;
	if ( (pDesc != NULL) && (pDesc->arrSections != NULL) ) {
		for ( i = 0; i < pDesc->iSectionCount; i++ ) {
			iRet = xuiAccordionAddSection(pWidget, pDesc->arrSections[i].sTitle, pDesc->arrSections[i].iId, pDesc->arrSections[i].bExpanded, NULL);
			if ( iRet != XUI_OK ) return iRet;
			if ( pDesc->arrSections[i].bDisabled ) {
				iRet = xuiAccordionSetSectionEnabled(pWidget, i, 0);
				if ( iRet != XUI_OK ) return iRet;
			}
		}
	}
	iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_KEY_DOWN, __xuiAccordionEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_FOCUS, __xuiAccordionEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_BLUR, __xuiAccordionEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_ENABLED_CHANGED, __xuiAccordionEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_VISIBLE_CHANGED, __xuiAccordionEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetAccessibilityProvider(pWidget,
		__xuiAccordionAccessibleCount, __xuiAccordionAccessibleGet,
		__xuiAccordionAccessibleAction, pData);
	if ( iRet != XUI_OK ) return iRet;
	return __xuiAccordionSyncSections(pWidget, pData);
}

static void __xuiAccordionDestroy(xui_widget pWidget, void* pTypeData, void* pUser)
{
	xui_accordion_data_t* pData;

	(void)pUser;
	pData = (xui_accordion_data_t*)pTypeData;
	if ( pData != NULL ) {
		(void)__xuiAccordionCancelPress(pWidget, pData);
		memset(pData, 0, sizeof(*pData));
	}
}

static void __xuiAccordionRegisterStyleProperty(xui_context pContext, xui_widget_type pType, const char* sName, int iValueType, uint32_t iDirtyFlags, uint32_t iFlags)
{
	xui_style_property_info_t tInfo;

	if ( xuiStyleFindProperty(pContext, sName) != 0 ) {
		return;
	}
	memset(&tInfo, 0, sizeof(tInfo));
	tInfo.iSize = sizeof(tInfo);
	tInfo.sName = sName;
	tInfo.iValueType = iValueType;
	tInfo.iDirtyFlags = iDirtyFlags;
	tInfo.iFlags = iFlags;
	tInfo.pWidgetType = pType;
	(void)xuiStyleRegisterProperty(pContext, &tInfo, NULL);
}

static int __xuiAccordionPreparePaint(xui_widget pWidget)
{
	xui_accordion_data_t* pData = __xuiAccordionGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pData->iChromeStyleVersion == pWidget->iStyleVersion ) return XUI_OK;
	pData->iChromeStyleVersion = pWidget->iStyleVersion;
	return __xuiAccordionInvalidateAll(pWidget, pData, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
}

static void __xuiAccordionRegisterStyleProperties(xui_context pContext, xui_widget_type pType)
{
	uint32_t iPaintDirty;
	uint32_t iLayoutDirty;

	pType->onPreparePaint = __xuiAccordionPreparePaint;
	iPaintDirty = XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER;
	iLayoutDirty = XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER;
	__xuiAccordionRegisterStyleProperty(pContext, pType, "accordion.background.color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiAccordionRegisterStyleProperty(pContext, pType, "accordion.header.color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiAccordionRegisterStyleProperty(pContext, pType, "accordion.header.hover_color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiAccordionRegisterStyleProperty(pContext, pType, "accordion.header.expanded_color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiAccordionRegisterStyleProperty(pContext, pType, "accordion.content.color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiAccordionRegisterStyleProperty(pContext, pType, "accordion.border.color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiAccordionRegisterStyleProperty(pContext, pType, "accordion.indicator.color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiAccordionRegisterStyleProperty(pContext, pType, "accordion.text.color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiAccordionRegisterStyleProperty(pContext, pType, "accordion.text.active_color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiAccordionRegisterStyleProperty(pContext, pType, "accordion.text.disabled_color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiAccordionRegisterStyleProperty(pContext, pType, "accordion.header.height", XUI_STYLE_VALUE_FLOAT, iLayoutDirty, 0);
	__xuiAccordionRegisterStyleProperty(pContext, pType, "accordion.spacing", XUI_STYLE_VALUE_FLOAT, iLayoutDirty, 0);
	__xuiAccordionRegisterStyleProperty(pContext, pType, "accordion.content.padding", XUI_STYLE_VALUE_FLOAT, iLayoutDirty, 0);
}

XUI_API xui_widget_type xuiAccordionGetType(xui_context pContext)
{
	xui_widget_type_desc_t tDesc;
	xui_widget_type pType;
	xui_cache_policy_t tPolicy;
	int iRet;

	if ( !xuiInternalContextIsValid(pContext) ) {
		return NULL;
	}
	pType = xuiWidgetFindType(pContext, "accordion");
	if ( pType != NULL ) {
		__xuiAccordionRegisterStyleProperties(pContext, pType);
		return pType;
	}
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.sName = "accordion";
	tDesc.pParent = xuiWidgetGetBaseType();
	tDesc.iFlags = XUI_WIDGET_TYPE_DEFAULT_LAYOUT | XUI_WIDGET_TYPE_DEFAULT_CACHE_POLICY;
	tDesc.iTypeDataSize = sizeof(xui_accordion_data_t);
	tDesc.onInit = __xuiAccordionInit;
	tDesc.onDestroy = __xuiAccordionDestroy;
	tDesc.onContentMeasure = __xuiAccordionContentMeasure;
	tDesc.onLayoutPrepare = __xuiAccordionPrepare;
	tDesc.onLayoutComplete = __xuiAccordionLayoutComplete;
	tDesc.onCacheRender = __xuiAccordionCacheRender;
	__xuiAccordionDefaultLayout(&tDesc.tLayout);
	__xuiAccordionDefaultCachePolicy(&tPolicy);
	tDesc.tCachePolicy = tPolicy;
	iRet = xuiWidgetRegisterType(pContext, &pType, &tDesc);
	if ( iRet != XUI_OK ) {
		return NULL;
	}
	__xuiAccordionRegisterStyleProperties(pContext, pType);
	return pType;
}

XUI_API int xuiAccordionCreate(xui_context pContext, xui_widget* ppWidget, const xui_accordion_desc_t* pDesc)
{
	xui_widget_type pType;

	if ( (ppWidget == NULL) || !__xuiAccordionDescValid(pDesc) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	*ppWidget = NULL;
	pType = xuiAccordionGetType(pContext);
	if ( pType == NULL ) {
		return XUI_ERROR_NOT_INITIALIZED;
	}
	return xuiWidgetCreateTyped(pContext, pType, ppWidget, pDesc);
}

XUI_API int xuiAccordionSetSelect(xui_widget pWidget, xui_accordion_select_proc onSelect, void* pUser)
{
	xui_accordion_data_t* pData;

	pData = __xuiAccordionGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	pData->onSelect = onSelect;
	pData->pSelectUser = pUser;
	return XUI_OK;
}

XUI_API int xuiAccordionClear(xui_widget pWidget)
{
	xui_accordion_data_t* pData;
	int iRet;
	int i;

	pData = __xuiAccordionGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pData->iSectionCount == 0 ) return XUI_OK;
	iRet = __xuiAccordionCancelPress(pWidget, pData);
	if ( iRet != XUI_OK ) return iRet;
	if ( __xuiAccordionGetData(pWidget) != pData ) return XUI_OK;
	iRet = xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	if ( iRet != XUI_OK ) return iRet;
	for ( i = 0; i < pData->iSectionCount; i++ ) {
		if ( pData->arrSections[i].pSection != NULL ) {
			xuiWidgetDestroy(pData->arrSections[i].pSection);
		}
	}
	memset(pData->arrSections, 0, sizeof(pData->arrSections));
	for ( i = 0; i < XUI_ACCORDION_SECTION_CAPACITY; i++ ) {
		pData->arrSections[i].bEnabled = 1;
	}
	pData->iSectionCount = 0;
	pData->iSelected = -1;
	pData->iHoverIndex = -1;
	pData->iActiveIndex = -1;
	pData->fContentHeight = 0.0f;
	pData->iChangeCount++;
	xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_TREE_CHANGED);
	xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_SELECTION_CHANGED);
	return XUI_OK;
}

XUI_API int xuiAccordionAddSection(xui_widget pWidget, const char* sTitle, int iId, int bExpanded, xui_widget* ppClient)
{
	xui_accordion_data_t* pData;
	xui_accordion_section_t* pSection;
	int arrOldExpanded[XUI_ACCORDION_SECTION_CAPACITY];
	int iOldSelected;
	int iOldChangeCount;
	int iIndex;
	int iRet;
	int i;

	if ( ppClient != NULL ) *ppClient = NULL;
	pData = __xuiAccordionGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pData->iSectionCount >= XUI_ACCORDION_SECTION_CAPACITY ) return XUI_ERROR_OUT_OF_MEMORY;
	for ( i = 0; i < pData->iSectionCount; i++ ) arrOldExpanded[i] = pData->arrSections[i].bExpanded;
	iOldSelected = pData->iSelected;
	iOldChangeCount = pData->iChangeCount;
	iIndex = pData->iSectionCount;
	iRet = __xuiAccordionCreateSectionWidgets(pWidget, pData, iIndex);
	if ( iRet != XUI_OK ) return iRet;
	pSection = &pData->arrSections[iIndex];
	__xuiAccordionSetTitle(pSection, sTitle);
	pSection->iId = iId;
	pSection->bExpanded = bExpanded ? 1 : 0;
	pSection->bEnabled = 1;
	pData->iSectionCount++;
	if ( pData->iMode == XUI_ACCORDION_MODE_SINGLE && pSection->bExpanded ) {
		for ( i = 0; i < iIndex; i++ ) pData->arrSections[i].bExpanded = 0;
		pData->iSelected = iIndex;
	} else if ( pSection->bExpanded && pData->iSelected < 0 ) {
		pData->iSelected = iIndex;
	}
	pData->iChangeCount++;
	iRet = __xuiAccordionSyncSections(pWidget, pData);
	if ( iRet != XUI_OK ) {
		pData->iSectionCount--;
		for ( i = 0; i < pData->iSectionCount; i++ ) pData->arrSections[i].bExpanded = arrOldExpanded[i];
		pData->iSelected = iOldSelected;
		pData->iChangeCount = iOldChangeCount;
		if ( pSection->pSection != NULL ) xuiWidgetDestroy(pSection->pSection);
		memset(pSection, 0, sizeof(*pSection));
		pSection->bEnabled = 1;
		(void)__xuiAccordionSyncSections(pWidget, pData);
		return iRet;
	}
	if ( __xuiAccordionGetData(pWidget) != pData ) return XUI_OK;
	if ( ppClient != NULL ) *ppClient = pSection->pClient;
	xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_TREE_CHANGED);
	xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_SELECTION_CHANGED);
	return XUI_OK;
}

XUI_API int xuiAccordionRemoveSection(xui_widget pWidget, int iIndex)
{
	xui_accordion_data_t* pData;
	xui_accordion_section_t arrOldSections[XUI_ACCORDION_SECTION_CAPACITY];
	xui_accordion_section_t tRemoved;
	xui_context pContext;
	float fOldContentHeight;
	int iOldSectionCount;
	int iOldSelected;
	int iOldHover;
	int iOldChangeCount;
	int iCandidate;
	int bSelectionChanged;
	int iRet;
	int i;

	pData = __xuiAccordionGetData(pWidget);
	if ( pData == NULL || iIndex < 0 || iIndex >= pData->iSectionCount ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pContext = xuiWidgetGetContext(pWidget);
	xuiInternalOperationEnter(pContext);
	iRet = __xuiAccordionCancelPress(pWidget, pData);
	if ( iRet != XUI_OK || __xuiAccordionGetData(pWidget) != pData ) goto done;

	memcpy(arrOldSections, pData->arrSections, sizeof(arrOldSections));
	tRemoved = pData->arrSections[iIndex];
	iOldSectionCount = pData->iSectionCount;
	iOldSelected = pData->iSelected;
	iOldHover = pData->iHoverIndex;
	iOldChangeCount = pData->iChangeCount;
	fOldContentHeight = pData->fContentHeight;
	for ( i = iIndex; i < XUI_ACCORDION_SECTION_CAPACITY - 1; i++ ) {
		pData->arrSections[i] = pData->arrSections[i + 1];
	}
	memset(&pData->arrSections[XUI_ACCORDION_SECTION_CAPACITY - 1], 0,
		sizeof(pData->arrSections[XUI_ACCORDION_SECTION_CAPACITY - 1]));
	pData->arrSections[XUI_ACCORDION_SECTION_CAPACITY - 1].bEnabled = 1;
	pData->iSectionCount--;
	pData->iActiveIndex = -1;
	if ( iOldHover == iIndex ) pData->iHoverIndex = -1;
	else if ( iOldHover > iIndex ) pData->iHoverIndex = iOldHover - 1;
	bSelectionChanged = 0;
	if ( iOldSelected == iIndex ) {
		pData->iSelected = -1;
		if ( pData->iSectionCount > 0 ) {
			iCandidate = iIndex < pData->iSectionCount ? iIndex : pData->iSectionCount - 1;
			pData->iSelected = __xuiAccordionSectionEnabled(pData, iCandidate) ?
				iCandidate : __xuiAccordionStepEnabled(pData, iCandidate, 1);
		}
		bSelectionChanged = 1;
	} else if ( iOldSelected > iIndex ) {
		pData->iSelected = iOldSelected - 1;
		bSelectionChanged = 1;
	}
	pData->fContentHeight = 0.0f;
	pData->iChangeCount++;
	iRet = __xuiAccordionSyncSections(pWidget, pData);
	if ( iRet != XUI_OK ) {
		memcpy(pData->arrSections, arrOldSections, sizeof(arrOldSections));
		pData->iSectionCount = iOldSectionCount;
		pData->iSelected = iOldSelected;
		pData->iHoverIndex = iOldHover;
		pData->iChangeCount = iOldChangeCount;
		pData->fContentHeight = fOldContentHeight;
		(void)__xuiAccordionSyncSections(pWidget, pData);
		goto done;
	}
	if ( __xuiAccordionGetData(pWidget) != pData ) goto done;
	xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_TREE_CHANGED);
	if ( bSelectionChanged ) {
		xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_SELECTION_CHANGED);
	}
	xuiWidgetDestroy(tRemoved.pSection);

done:
	xuiInternalOperationLeave(pContext);
	return iRet;
}

XUI_API int xuiAccordionGetSectionCount(xui_widget pWidget)
{
	xui_accordion_data_t* pData;

	pData = __xuiAccordionGetData(pWidget);
	return (pData != NULL) ? pData->iSectionCount : 0;
}

XUI_API xui_widget xuiAccordionGetSectionWidget(xui_widget pWidget, int iIndex)
{
	xui_accordion_data_t* pData;

	pData = __xuiAccordionGetData(pWidget);
	if ( (pData == NULL) || (iIndex < 0) || (iIndex >= pData->iSectionCount) ) return NULL;
	return pData->arrSections[iIndex].pSection;
}

XUI_API xui_widget xuiAccordionGetHeaderWidget(xui_widget pWidget, int iIndex)
{
	xui_accordion_data_t* pData;

	pData = __xuiAccordionGetData(pWidget);
	if ( (pData == NULL) || (iIndex < 0) || (iIndex >= pData->iSectionCount) ) return NULL;
	return pData->arrSections[iIndex].pHeader;
}

XUI_API xui_widget xuiAccordionGetButtonWidget(xui_widget pWidget, int iIndex)
{
	return xuiAccordionGetHeaderWidget(pWidget, iIndex);
}

XUI_API xui_widget xuiAccordionGetClientWidget(xui_widget pWidget, int iIndex)
{
	xui_accordion_data_t* pData;

	pData = __xuiAccordionGetData(pWidget);
	if ( (pData == NULL) || (iIndex < 0) || (iIndex >= pData->iSectionCount) ) return NULL;
	return pData->arrSections[iIndex].pClient;
}

XUI_API int xuiAccordionAddSectionChild(xui_widget pWidget, int iIndex, xui_widget pChild)
{
	xui_widget pClient;

	if ( pChild == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	pClient = xuiAccordionGetClientWidget(pWidget, iIndex);
	if ( pClient == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	return xuiWidgetAddChild(pClient, pChild);
}

XUI_API int xuiAccordionSetSectionTitle(xui_widget pWidget, int iIndex, const char* sTitle)
{
	xui_accordion_data_t* pData;
	char sOld[XUI_ACCORDION_TITLE_CAPACITY];
	int iRet;

	pData = __xuiAccordionGetData(pWidget);
	if ( (pData == NULL) || (iIndex < 0) || (iIndex >= pData->iSectionCount) ) return XUI_ERROR_INVALID_ARGUMENT;
	memcpy(sOld, pData->arrSections[iIndex].sTitle, sizeof(sOld));
	__xuiAccordionSetTitle(&pData->arrSections[iIndex], sTitle);
	if ( strcmp(sOld, pData->arrSections[iIndex].sTitle) == 0 ) return XUI_OK;
	iRet = __xuiAccordionInvalidateSection(&pData->arrSections[iIndex], XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	if ( iRet != XUI_OK ) {
		memcpy(pData->arrSections[iIndex].sTitle, sOld, sizeof(sOld));
		(void)__xuiAccordionInvalidateSection(&pData->arrSections[iIndex], XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
		return iRet;
	}
	pData->iChangeCount++;
	xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_NODE_CHANGED);
	return XUI_OK;
}

XUI_API const char* xuiAccordionGetSectionTitle(xui_widget pWidget, int iIndex)
{
	xui_accordion_data_t* pData;

	pData = __xuiAccordionGetData(pWidget);
	if ( (pData == NULL) || (iIndex < 0) || (iIndex >= pData->iSectionCount) ) return NULL;
	return pData->arrSections[iIndex].sTitle;
}

XUI_API int xuiAccordionSetSectionId(xui_widget pWidget, int iIndex, int iId)
{
	xui_accordion_data_t* pData;

	pData = __xuiAccordionGetData(pWidget);
	if ( (pData == NULL) || (iIndex < 0) || (iIndex >= pData->iSectionCount) ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pData->arrSections[iIndex].iId == iId ) return XUI_OK;
	pData->arrSections[iIndex].iId = iId;
	pData->iChangeCount++;
	xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_NODE_CHANGED);
	return XUI_OK;
}

XUI_API int xuiAccordionGetSectionId(xui_widget pWidget, int iIndex)
{
	xui_accordion_data_t* pData;

	pData = __xuiAccordionGetData(pWidget);
	if ( (pData == NULL) || (iIndex < 0) || (iIndex >= pData->iSectionCount) ) return 0;
	return pData->arrSections[iIndex].iId;
}

XUI_API int xuiAccordionSetExpanded(xui_widget pWidget, int iIndex, int bExpanded)
{
	xui_accordion_data_t* pData;
	int iRet;

	pData = __xuiAccordionGetData(pWidget);
	if ( (pData == NULL) || (iIndex < 0) || (iIndex >= pData->iSectionCount) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	iRet = __xuiAccordionCancelPress(pWidget, pData);
	if ( iRet != XUI_OK ) return iRet;
	pData = __xuiAccordionGetData(pWidget);
	if ( pData == NULL ) return XUI_OK;
	return __xuiAccordionSetExpandedInternal(pWidget, pData, iIndex, bExpanded, 0);
}

XUI_API int xuiAccordionIsExpanded(xui_widget pWidget, int iIndex)
{
	xui_accordion_data_t* pData;

	pData = __xuiAccordionGetData(pWidget);
	if ( (pData == NULL) || (iIndex < 0) || (iIndex >= pData->iSectionCount) ) return 0;
	return pData->arrSections[iIndex].bExpanded;
}

XUI_API int xuiAccordionSetSectionEnabled(xui_widget pWidget, int iIndex, int bEnabled)
{
	xui_accordion_data_t* pData;
	int iOldEnabled;
	int iOldHover;
	int iOldSelected;
	int iOldChangeCount;
	int iRet;

	pData = __xuiAccordionGetData(pWidget);
	if ( (pData == NULL) || (iIndex < 0) || (iIndex >= pData->iSectionCount) ) return XUI_ERROR_INVALID_ARGUMENT;
	bEnabled = bEnabled ? 1 : 0;
	if ( pData->arrSections[iIndex].bEnabled == bEnabled ) return XUI_OK;
	if ( !bEnabled && (pData->iActiveIndex == iIndex) ) {
		iRet = __xuiAccordionCancelPress(pWidget, pData);
		if ( iRet != XUI_OK ) return iRet;
		pData = __xuiAccordionGetData(pWidget);
		if ( pData == NULL ) return XUI_OK;
	}
	iOldEnabled = pData->arrSections[iIndex].bEnabled;
	iOldHover = pData->iHoverIndex;
	iOldSelected = pData->iSelected;
	iOldChangeCount = pData->iChangeCount;
	pData->arrSections[iIndex].bEnabled = bEnabled;
	if ( !bEnabled ) {
		if ( pData->iHoverIndex == iIndex ) pData->iHoverIndex = -1;
		if ( pData->iSelected == iIndex ) pData->iSelected = __xuiAccordionStepEnabled(pData, iIndex, 1);
	}
	pData->iChangeCount++;
	iRet = __xuiAccordionSyncSections(pWidget, pData);
	if ( iRet != XUI_OK ) {
		pData->arrSections[iIndex].bEnabled = iOldEnabled;
		pData->iHoverIndex = iOldHover;
		pData->iSelected = iOldSelected;
		pData->iChangeCount = iOldChangeCount;
		(void)__xuiAccordionSyncSections(pWidget, pData);
		return iRet;
	}
	if ( __xuiAccordionGetData(pWidget) != pData ) return XUI_OK;
	xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_STATE_CHANGED);
	if ( pData->iSelected != iOldSelected ) xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_SELECTION_CHANGED);
	return XUI_OK;
}

XUI_API int xuiAccordionIsSectionEnabled(xui_widget pWidget, int iIndex)
{
	xui_accordion_data_t* pData;

	pData = __xuiAccordionGetData(pWidget);
	if ( (pData == NULL) || (iIndex < 0) || (iIndex >= pData->iSectionCount) ) return 0;
	return pData->arrSections[iIndex].bEnabled;
}

XUI_API int xuiAccordionSetMode(xui_widget pWidget, int iMode)
{
	xui_accordion_data_t* pData;
	int arrOldExpanded[XUI_ACCORDION_SECTION_CAPACITY];
	int iOldMode;
	int iOldSelected;
	int iOldChangeCount;
	int iRet;
	int i;
	int bKeepFound;

	pData = __xuiAccordionGetData(pWidget);
	if ( (pData == NULL) || !__xuiAccordionModeValid(iMode) ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pData->iMode == iMode ) return XUI_OK;
	iRet = __xuiAccordionCancelPress(pWidget, pData);
	if ( iRet != XUI_OK ) return iRet;
	pData = __xuiAccordionGetData(pWidget);
	if ( pData == NULL ) return XUI_OK;
	for ( i = 0; i < pData->iSectionCount; i++ ) arrOldExpanded[i] = pData->arrSections[i].bExpanded;
	iOldMode = pData->iMode;
	iOldSelected = pData->iSelected;
	iOldChangeCount = pData->iChangeCount;
	pData->iMode = iMode;
	if ( iMode == XUI_ACCORDION_MODE_SINGLE ) {
		bKeepFound = 0;
		for ( i = 0; i < pData->iSectionCount; i++ ) {
			if ( pData->arrSections[i].bExpanded ) {
				if ( !bKeepFound ) {
					bKeepFound = 1;
					pData->iSelected = i;
				} else {
					pData->arrSections[i].bExpanded = 0;
				}
			}
		}
	}
	pData->iChangeCount++;
	iRet = __xuiAccordionSyncSections(pWidget, pData);
	if ( iRet != XUI_OK ) {
		pData->iMode = iOldMode;
		pData->iSelected = iOldSelected;
		pData->iChangeCount = iOldChangeCount;
		for ( i = 0; i < pData->iSectionCount; i++ ) pData->arrSections[i].bExpanded = arrOldExpanded[i];
		(void)__xuiAccordionSyncSections(pWidget, pData);
		return iRet;
	}
	if ( __xuiAccordionGetData(pWidget) != pData ) return XUI_OK;
	xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_STATE_CHANGED);
	if ( pData->iSelected != iOldSelected ) xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_SELECTION_CHANGED);
	return XUI_OK;
}

XUI_API int xuiAccordionGetMode(xui_widget pWidget)
{
	xui_accordion_data_t* pData;

	pData = __xuiAccordionGetData(pWidget);
	return (pData != NULL) ? pData->iMode : XUI_ACCORDION_MODE_MULTIPLE;
}

XUI_API int xuiAccordionSetFont(xui_widget pWidget, xui_font pFont)
{
	xui_accordion_data_t* pData;
	xui_font pOldFont;
	int iOldChangeCount;
	int iRet;

	pData = __xuiAccordionGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pData->pFont == pFont ) return XUI_OK;
	pOldFont = pData->pFont;
	iOldChangeCount = pData->iChangeCount;
	pData->pFont = pFont;
	pData->iChangeCount++;
	iRet = __xuiAccordionInvalidateAll(pWidget, pData, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	if ( iRet != XUI_OK ) {
		pData->pFont = pOldFont;
		pData->iChangeCount = iOldChangeCount;
		(void)__xuiAccordionInvalidateAll(pWidget, pData, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	}
	return iRet;
}

XUI_API xui_font xuiAccordionGetFont(xui_widget pWidget)
{
	xui_accordion_data_t* pData;

	pData = __xuiAccordionGetData(pWidget);
	return (pData != NULL) ? pData->pFont : NULL;
}

XUI_API int xuiAccordionSetMetrics(xui_widget pWidget, float fHeaderHeight, float fSpacing, float fContentPadding)
{
	xui_accordion_data_t* pData;
	float fOldHeaderHeight;
	float fOldSpacing;
	float fOldContentPadding;
	int iOldChangeCount;
	int iRet;

	pData = __xuiAccordionGetData(pWidget);
	if ( (pData == NULL) || !isfinite(fHeaderHeight) || (fHeaderHeight < 18.0f) ||
	     !isfinite(fSpacing) || (fSpacing < 0.0f) ||
	     !isfinite(fContentPadding) || (fContentPadding < 0.0f) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( pData->fHeaderHeight == fHeaderHeight && pData->fSpacing == fSpacing &&
	     pData->fContentPadding == fContentPadding ) return XUI_OK;
	iRet = __xuiAccordionCancelPress(pWidget, pData);
	if ( iRet != XUI_OK ) return iRet;
	pData = __xuiAccordionGetData(pWidget);
	if ( pData == NULL ) return XUI_OK;
	fOldHeaderHeight = pData->fHeaderHeight;
	fOldSpacing = pData->fSpacing;
	fOldContentPadding = pData->fContentPadding;
	iOldChangeCount = pData->iChangeCount;
	pData->fHeaderHeight = fHeaderHeight;
	pData->fSpacing = fSpacing;
	pData->fContentPadding = fContentPadding;
	pData->iChangeCount++;
	iRet = __xuiAccordionUpdateClientPadding(pData, fContentPadding);
	if ( iRet == XUI_OK ) iRet = __xuiAccordionInvalidateAll(pWidget, pData,
		XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	if ( iRet != XUI_OK ) {
		pData->fHeaderHeight = fOldHeaderHeight;
		pData->fSpacing = fOldSpacing;
		pData->fContentPadding = fOldContentPadding;
		pData->iChangeCount = iOldChangeCount;
		(void)__xuiAccordionUpdateClientPadding(pData, fOldContentPadding);
		(void)__xuiAccordionInvalidateAll(pWidget, pData,
			XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
		return iRet;
	}
	xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_BOUNDS_CHANGED);
	return XUI_OK;
}

XUI_API int xuiAccordionGetMetrics(xui_widget pWidget, float* pHeaderHeight, float* pSpacing, float* pContentPadding)
{
	xui_accordion_data_t* pData;

	pData = __xuiAccordionGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pHeaderHeight != NULL ) *pHeaderHeight = pData->fHeaderHeight;
	if ( pSpacing != NULL ) *pSpacing = pData->fSpacing;
	if ( pContentPadding != NULL ) *pContentPadding = pData->fContentPadding;
	return XUI_OK;
}

XUI_API int xuiAccordionSetColors(xui_widget pWidget, uint32_t iBackground, uint32_t iHeader, uint32_t iHover, uint32_t iExpanded, uint32_t iContent, uint32_t iBorder, uint32_t iText, uint32_t iActiveText, uint32_t iDisabledText)
{
	xui_accordion_data_t* pData;
	uint32_t arrOld[9];
	int iOldChangeCount;
	int iRet;

	pData = __xuiAccordionGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pData->iBackgroundColor == iBackground && pData->iHeaderColor == iHeader &&
	     pData->iHoverColor == iHover && pData->iExpandedColor == iExpanded &&
	     pData->iContentColor == iContent && pData->iBorderColor == iBorder &&
	     pData->iTextColor == iText && pData->iActiveTextColor == iActiveText &&
	     pData->iDisabledTextColor == iDisabledText ) return XUI_OK;
	arrOld[0] = pData->iBackgroundColor;
	arrOld[1] = pData->iHeaderColor;
	arrOld[2] = pData->iHoverColor;
	arrOld[3] = pData->iExpandedColor;
	arrOld[4] = pData->iContentColor;
	arrOld[5] = pData->iBorderColor;
	arrOld[6] = pData->iTextColor;
	arrOld[7] = pData->iActiveTextColor;
	arrOld[8] = pData->iDisabledTextColor;
	iOldChangeCount = pData->iChangeCount;
	pData->iBackgroundColor = iBackground;
	pData->iHeaderColor = iHeader;
	pData->iHoverColor = iHover;
	pData->iExpandedColor = iExpanded;
	pData->iContentColor = iContent;
	pData->iBorderColor = iBorder;
	pData->iTextColor = iText;
	pData->iActiveTextColor = iActiveText;
	pData->iDisabledTextColor = iDisabledText;
	pData->iChangeCount++;
	iRet = __xuiAccordionInvalidateAll(pWidget, pData, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	if ( iRet != XUI_OK ) {
		pData->iBackgroundColor = arrOld[0];
		pData->iHeaderColor = arrOld[1];
		pData->iHoverColor = arrOld[2];
		pData->iExpandedColor = arrOld[3];
		pData->iContentColor = arrOld[4];
		pData->iBorderColor = arrOld[5];
		pData->iTextColor = arrOld[6];
		pData->iActiveTextColor = arrOld[7];
		pData->iDisabledTextColor = arrOld[8];
		pData->iChangeCount = iOldChangeCount;
		(void)__xuiAccordionInvalidateAll(pWidget, pData, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	}
	return iRet;
}

XUI_API int xuiAccordionGetColors(xui_widget pWidget, uint32_t* pBackground, uint32_t* pHeader, uint32_t* pHover, uint32_t* pExpanded, uint32_t* pContent, uint32_t* pBorder, uint32_t* pText, uint32_t* pActiveText, uint32_t* pDisabledText)
{
	xui_accordion_data_t* pData;

	pData = __xuiAccordionGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pBackground != NULL ) *pBackground = pData->iBackgroundColor;
	if ( pHeader != NULL ) *pHeader = pData->iHeaderColor;
	if ( pHover != NULL ) *pHover = pData->iHoverColor;
	if ( pExpanded != NULL ) *pExpanded = pData->iExpandedColor;
	if ( pContent != NULL ) *pContent = pData->iContentColor;
	if ( pBorder != NULL ) *pBorder = pData->iBorderColor;
	if ( pText != NULL ) *pText = pData->iTextColor;
	if ( pActiveText != NULL ) *pActiveText = pData->iActiveTextColor;
	if ( pDisabledText != NULL ) *pDisabledText = pData->iDisabledTextColor;
	return XUI_OK;
}

XUI_API float xuiAccordionGetContentHeight(xui_widget pWidget)
{
	xui_accordion_data_t* pData;

	pData = __xuiAccordionGetData(pWidget);
	return (pData != NULL) ? pData->fContentHeight : 0.0f;
}

XUI_API xui_rect_t xuiAccordionGetSectionRect(xui_widget pWidget, int iIndex)
{
	xui_accordion_data_t* pData;

	pData = __xuiAccordionGetData(pWidget);
	if ( (pData == NULL) || (iIndex < 0) || (iIndex >= pData->iSectionCount) ) return (xui_rect_t){0.0f, 0.0f, 0.0f, 0.0f};
	return pData->arrSections[iIndex].tSectionRect;
}

XUI_API xui_rect_t xuiAccordionGetHeaderRect(xui_widget pWidget, int iIndex)
{
	xui_accordion_data_t* pData;

	pData = __xuiAccordionGetData(pWidget);
	if ( (pData == NULL) || (iIndex < 0) || (iIndex >= pData->iSectionCount) ) return (xui_rect_t){0.0f, 0.0f, 0.0f, 0.0f};
	return pData->arrSections[iIndex].tHeaderRect;
}

XUI_API xui_rect_t xuiAccordionGetClientRect(xui_widget pWidget, int iIndex)
{
	xui_accordion_data_t* pData;

	pData = __xuiAccordionGetData(pWidget);
	if ( (pData == NULL) || (iIndex < 0) || (iIndex >= pData->iSectionCount) ) return (xui_rect_t){0.0f, 0.0f, 0.0f, 0.0f};
	return pData->arrSections[iIndex].tClientRect;
}

XUI_API xui_rect_t xuiAccordionGetArrowRect(xui_widget pWidget, int iIndex)
{
	xui_accordion_data_t* pData;

	pData = __xuiAccordionGetData(pWidget);
	if ( (pData == NULL) || (iIndex < 0) || (iIndex >= pData->iSectionCount) ) return (xui_rect_t){0.0f, 0.0f, 0.0f, 0.0f};
	return pData->arrSections[iIndex].tArrowRect;
}

XUI_API int xuiAccordionGetSelected(xui_widget pWidget)
{
	xui_accordion_data_t* pData;

	pData = __xuiAccordionGetData(pWidget);
	return (pData != NULL) ? pData->iSelected : -1;
}

XUI_API int xuiAccordionGetHoverIndex(xui_widget pWidget)
{
	xui_accordion_data_t* pData;

	pData = __xuiAccordionGetData(pWidget);
	return (pData != NULL) ? pData->iHoverIndex : -1;
}

XUI_API int xuiAccordionGetActiveIndex(xui_widget pWidget)
{
	xui_accordion_data_t* pData;

	pData = __xuiAccordionGetData(pWidget);
	return (pData != NULL) ? pData->iActiveIndex : -1;
}

XUI_API uint32_t xuiAccordionGetState(xui_widget pWidget)
{
	xui_accordion_data_t* pData;
	uint32_t iState;

	pData = __xuiAccordionGetData(pWidget);
	if ( pData == NULL ) return 0;
	iState = 0;
	if ( pData->iHoverIndex >= 0 ) iState |= XUI_WIDGET_STATE_HOVER;
	if ( pData->iActiveIndex >= 0 ) iState |= XUI_WIDGET_STATE_ACTIVE;
	if ( xuiGetFocusWidget(xuiWidgetGetContext(pWidget)) == pWidget ) iState |= XUI_WIDGET_STATE_FOCUS;
	if ( !xuiWidgetGetEffectiveEnabled(pWidget) ) iState |= XUI_WIDGET_STATE_DISABLED;
	return iState;
}

XUI_API int xuiAccordionGetChangeCount(xui_widget pWidget)
{
	xui_accordion_data_t* pData;

	pData = __xuiAccordionGetData(pWidget);
	return (pData != NULL) ? pData->iChangeCount : 0;
}

#endif
