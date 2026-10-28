#include "xui_config.h"
#if XUI_ENABLE_STEP_BAR
#include "xui_internal.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

#define XUI_STEP_BAR_DEFAULT_W		420.0f
#define XUI_STEP_BAR_ARROW_H		30.0f
#define XUI_STEP_BAR_DOT_H		72.0f
#define XUI_STEP_BAR_VERTICAL_W		180.0f
#define XUI_STEP_BAR_VERTICAL_ROW_H	72.0f

typedef struct xui_step_bar_data_t {
	char* arrTitles[XUI_STEP_BAR_MAX_STEPS];
	char sAccessibleValue[256];
	xui_font pFont;
	int iStepCount;
	int iCurrent;
	int iStyle;
	int iChangeCount;
	uint32_t iDoneColor;
	uint32_t iActiveColor;
	uint32_t iPendingColor;
	uint32_t iLineColor;
	uint32_t iTextColor;
	uint32_t iActiveTextColor;
	uint32_t iPendingTextColor;
	uint32_t iArrowTextColor;
	uint32_t iActiveCheckColor;
	uint32_t iBackgroundColor;
	float fBarHeight;
	float fDotRadius;
	float fLineWidth;
} xui_step_bar_data_t;

static int __xuiStepBarStyleValid(int iStyle)
{
	return (iStyle == XUI_STEP_BAR_STYLE_ARROW) ||
	       (iStyle == XUI_STEP_BAR_STYLE_DOT) ||
	       (iStyle == XUI_STEP_BAR_STYLE_VERTICAL);
}

static int __xuiStepBarAlpha(uint32_t iColor)
{
	return (int)(iColor & 0xffu);
}

static int __xuiStepBarNonNegativeFloatValid(float fValue)
{
	return isfinite(fValue) && fValue >= 0.0f &&
	       fValue <= XUI_LAYOUT_UNBOUNDED;
}

static void __xuiStepBarFormatArrowTitle(char* sText, size_t iCapacity, int iIndex, const char* sTitle)
{
	int iWritten;

	if ( sText == NULL || iCapacity == 0u ) return;
	iWritten = snprintf(sText, iCapacity, "%d.%s", iIndex + 1, (sTitle != NULL) ? sTitle : "");
	if ( iWritten < 0 ) {
		sText[0] = '\0';
	} else if ( (size_t)iWritten >= iCapacity ) {
		xuiInternalUtf8TrimIncomplete(sText);
	}
}

static int __xuiStepBarDescValid(const xui_step_bar_desc_t* pDesc)
{
	if ( pDesc == NULL ) {
		return 1;
	}
	if ( (pDesc->iSize != 0) && (pDesc->iSize < sizeof(*pDesc)) ) {
		return 0;
	}
	if ( (pDesc->iStepCount < 0) || (pDesc->iStepCount > XUI_STEP_BAR_MAX_STEPS) ) {
		return 0;
	}
	if ( (pDesc->iStepCount == 0 && pDesc->iCurrent != 0) ||
	     (pDesc->iStepCount > 0 && (pDesc->iCurrent < 0 || pDesc->iCurrent >= pDesc->iStepCount)) ) {
		return 0;
	}
	if ( (pDesc->iStyle != 0) && !__xuiStepBarStyleValid(pDesc->iStyle) ) {
		return 0;
	}
	if ( !__xuiStepBarNonNegativeFloatValid(pDesc->fBarHeight) ||
	     !__xuiStepBarNonNegativeFloatValid(pDesc->fDotRadius) ||
	     !__xuiStepBarNonNegativeFloatValid(pDesc->fLineWidth) ) {
		return 0;
	}
	return 1;
}

static int __xuiStepBarTitlesEqual(const xui_step_bar_data_t* pData,
	const char* const* ppTitles, int iStepCount)
{
	int i;

	if ( (pData == NULL) || (pData->iStepCount != iStepCount) ) return 0;
	for ( i = 0; i < iStepCount; i++ ) {
		const char* sOld = pData->arrTitles[i] != NULL ? pData->arrTitles[i] : "";
		const char* sNew = (ppTitles != NULL && ppTitles[i] != NULL) ? ppTitles[i] : "";
		if ( strcmp(sOld, sNew) != 0 ) return 0;
	}
	return 1;
}

static int __xuiStepBarStringSet(char** psText, const char* sText)
{
	char* sNew;
	size_t iNeed;

	if ( psText == NULL ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( sText == NULL ) {
		sText = "";
	}
	if ( sText[0] == '\0' ) {
		if ( *psText != NULL ) {
			xrtFree(*psText);
			*psText = NULL;
		}
		return XUI_OK;
	}
	iNeed = strlen(sText) + 1u;
	sNew = (char*)xrtMalloc(iNeed);
	if ( sNew == NULL ) {
		return XUI_ERROR_OUT_OF_MEMORY;
	}
	memcpy(sNew, sText, iNeed);
	if ( *psText != NULL ) {
		xrtFree(*psText);
	}
	*psText = sNew;
	return XUI_OK;
}

static void __xuiStepBarClearTitles(xui_step_bar_data_t* pData)
{
	int i;

	if ( pData == NULL ) {
		return;
	}
	for ( i = 0; i < XUI_STEP_BAR_MAX_STEPS; i++ ) {
		if ( pData->arrTitles[i] != NULL ) {
			xrtFree(pData->arrTitles[i]);
			pData->arrTitles[i] = NULL;
		}
	}
	pData->iStepCount = 0;
}

static int __xuiStepBarSetTitlesData(xui_step_bar_data_t* pData, const char* const* ppTitles, int iStepCount)
{
	char* arrNew[XUI_STEP_BAR_MAX_STEPS];
	size_t iNeed;
	int i;
	int j;

	if ( (pData == NULL) || (iStepCount < 0) || (iStepCount > XUI_STEP_BAR_MAX_STEPS) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	memset(arrNew, 0, sizeof(arrNew));
	for ( i = 0; i < iStepCount; i++ ) {
		const char* sTitle = (ppTitles != NULL && ppTitles[i] != NULL) ? ppTitles[i] : "";
		if ( sTitle[0] == '\0' ) {
			continue;
		}
		iNeed = strlen(sTitle) + 1u;
		arrNew[i] = (char*)xrtMalloc(iNeed);
		if ( arrNew[i] == NULL ) {
			for ( j = 0; j < i; j++ ) {
				xrtFree(arrNew[j]);
			}
			return XUI_ERROR_OUT_OF_MEMORY;
		}
		memcpy(arrNew[i], sTitle, iNeed);
	}
	__xuiStepBarClearTitles(pData);
	for ( i = 0; i < iStepCount; i++ ) {
		pData->arrTitles[i] = arrNew[i];
	}
	pData->iStepCount = iStepCount;
	if ( pData->iCurrent >= iStepCount ) {
		pData->iCurrent = (iStepCount > 0) ? iStepCount - 1 : 0;
	}
	if ( pData->iCurrent < 0 ) {
		pData->iCurrent = 0;
	}
	return XUI_OK;
}

static xui_step_bar_data_t* __xuiStepBarGetData(xui_widget pWidget)
{
	xui_context pContext;
	xui_widget_type pType;

	if ( !xuiInternalWidgetIsValid(pWidget) ) {
		return NULL;
	}
	pContext = xuiWidgetGetContext(pWidget);
	pType = xuiWidgetFindType(pContext, "stepbar");
	if ( (pType == NULL) || !xuiWidgetIsType(pWidget, pType) ) {
		return NULL;
	}
	return (xui_step_bar_data_t*)xuiWidgetGetTypeData(pWidget);
}

static int __xuiStepBarStyleColor(xui_widget pWidget, const char* sName, uint32_t* pColor)
{
	xui_style_property_t tProperty;
	int iRet;

	if ( (pWidget == NULL) || (sName == NULL) || (pColor == NULL) ) {
		return 0;
	}
	memset(&tProperty, 0, sizeof(tProperty));
	tProperty.iSize = sizeof(tProperty);
	iRet = xuiWidgetGetResolvedStyleProperty(pWidget, sName, &tProperty);
	if ( (iRet == XUI_OK) && (tProperty.tValue.iType == XUI_STYLE_VALUE_COLOR) ) {
		*pColor = tProperty.tValue.iColor;
		return 1;
	}
	return 0;
}

static int __xuiStepBarStyleInt(xui_widget pWidget, const char* sName, int* pValue)
{
	xui_style_property_t tProperty;
	int iRet;

	if ( (pWidget == NULL) || (sName == NULL) || (pValue == NULL) ) {
		return 0;
	}
	memset(&tProperty, 0, sizeof(tProperty));
	tProperty.iSize = sizeof(tProperty);
	iRet = xuiWidgetGetResolvedStyleProperty(pWidget, sName, &tProperty);
	if ( (iRet == XUI_OK) && ((tProperty.tValue.iType == XUI_STYLE_VALUE_INT) || (tProperty.tValue.iType == XUI_STYLE_VALUE_BOOL)) ) {
		*pValue = tProperty.tValue.iInt;
		return 1;
	}
	return 0;
}

static int __xuiStepBarStyleFloat(xui_widget pWidget, const char* sName, float* pValue)
{
	xui_style_property_t tProperty;
	int iRet;

	if ( (pWidget == NULL) || (sName == NULL) || (pValue == NULL) ) {
		return 0;
	}
	memset(&tProperty, 0, sizeof(tProperty));
	tProperty.iSize = sizeof(tProperty);
	iRet = xuiWidgetGetResolvedStyleProperty(pWidget, sName, &tProperty);
	if ( (iRet == XUI_OK) && (tProperty.tValue.iType == XUI_STYLE_VALUE_FLOAT) &&
	     __xuiStepBarNonNegativeFloatValid(tProperty.tValue.fFloat) ) {
		*pValue = tProperty.tValue.fFloat;
		return 1;
	}
	return 0;
}

static xui_font __xuiStepBarStyleFont(xui_widget pWidget, xui_font pBaseFont)
{
	xui_style_property_t tProperty;
	xui_font pFont;
	int iRet;

	memset(&tProperty, 0, sizeof(tProperty));
	tProperty.iSize = sizeof(tProperty);
	iRet = xuiWidgetGetResolvedStyleProperty(pWidget, "font.name", &tProperty);
	if ( (iRet == XUI_OK) &&
	     (tProperty.tValue.iType == XUI_STYLE_VALUE_STRING) &&
	     (tProperty.tValue.sText != NULL) ) {
		pFont = xuiFindFont(xuiWidgetGetContext(pWidget), tProperty.tValue.sText);
		if ( pFont != NULL ) {
			return pFont;
		}
	}
	return pBaseFont;
}

static void __xuiStepBarResolve(xui_widget pWidget, const xui_step_bar_data_t* pData, xui_step_bar_data_t* pResolved)
{
	int iStyle;

	*pResolved = *pData;
	pResolved->pFont = __xuiStepBarStyleFont(pWidget, (pData->pFont != NULL) ? pData->pFont : xuiGetDefaultFont(xuiWidgetGetContext(pWidget)));
	(void)__xuiStepBarStyleColor(pWidget, "stepbar.done.color", &pResolved->iDoneColor);
	(void)__xuiStepBarStyleColor(pWidget, "stepbar.active.color", &pResolved->iActiveColor);
	(void)__xuiStepBarStyleColor(pWidget, "stepbar.pending.color", &pResolved->iPendingColor);
	(void)__xuiStepBarStyleColor(pWidget, "stepbar.line.color", &pResolved->iLineColor);
	(void)__xuiStepBarStyleColor(pWidget, "stepbar.text.color", &pResolved->iTextColor);
	(void)__xuiStepBarStyleColor(pWidget, "stepbar.text.active_color", &pResolved->iActiveTextColor);
	(void)__xuiStepBarStyleColor(pWidget, "stepbar.text.pending_color", &pResolved->iPendingTextColor);
	pResolved->iArrowTextColor = XUI_COLOR_WHITE;
	pResolved->iActiveCheckColor = XUI_COLOR_WHITE;
	(void)__xuiStepBarStyleColor(pWidget, "stepbar.arrow.text_color", &pResolved->iArrowTextColor);
	(void)__xuiStepBarStyleColor(pWidget, "stepbar.check.active_color", &pResolved->iActiveCheckColor);
	(void)__xuiStepBarStyleColor(pWidget, "stepbar.background.color", &pResolved->iBackgroundColor);
	(void)__xuiStepBarStyleFloat(pWidget, "stepbar.bar_height", &pResolved->fBarHeight);
	(void)__xuiStepBarStyleFloat(pWidget, "stepbar.dot_radius", &pResolved->fDotRadius);
	(void)__xuiStepBarStyleFloat(pWidget, "stepbar.line_width", &pResolved->fLineWidth);
	iStyle = pResolved->iStyle;
	if ( __xuiStepBarStyleInt(pWidget, "stepbar.style", &iStyle) && __xuiStepBarStyleValid(iStyle) ) {
		pResolved->iStyle = iStyle;
	}
}

static uint32_t __xuiStepBarStatusColor(const xui_step_bar_data_t* pData, int iIndex)
{
	if ( iIndex < pData->iCurrent ) return pData->iDoneColor;
	if ( iIndex == pData->iCurrent ) return pData->iActiveColor;
	return pData->iPendingColor;
}

static uint32_t __xuiStepBarTextColor(const xui_step_bar_data_t* pData, int iIndex)
{
	if ( iIndex == pData->iCurrent ) return pData->iActiveTextColor;
	if ( iIndex > pData->iCurrent ) return pData->iPendingTextColor;
	return pData->iTextColor;
}

static const char* __xuiStepBarTitle(const xui_step_bar_data_t* pData, int iIndex)
{
	if ( (pData == NULL) || (iIndex < 0) || (iIndex >= pData->iStepCount) || (pData->arrTitles[iIndex] == NULL) ) {
		return "";
	}
	return pData->arrTitles[iIndex];
}

static float __xuiStepBarMinFloat(float fA, float fB)
{
	return (fA < fB) ? fA : fB;
}

static float __xuiStepBarDotRadius(const xui_step_bar_data_t* pResolved,
	float fPrimaryExtent, float fCrossExtent)
{
	float fRadius;
	float fCrossLimit;
	float fPrimaryLimit;

	if ( (pResolved == NULL) || (pResolved->iStepCount <= 0) ||
	     (fPrimaryExtent <= 0.0f) || (fCrossExtent <= 0.0f) ) {
		return 0.0f;
	}
	fRadius = (pResolved->fDotRadius > 0.0f) ? pResolved->fDotRadius : 11.0f;
	fCrossLimit = (fCrossExtent >= 4.0f) ? (fCrossExtent - 4.0f) * 0.5f : fCrossExtent * 0.5f;
	fPrimaryLimit = fPrimaryExtent / ((float)pResolved->iStepCount * 2.0f);
	fRadius = __xuiStepBarMinFloat(fRadius, fCrossLimit);
	fRadius = __xuiStepBarMinFloat(fRadius, fPrimaryLimit);
	return (fRadius > 0.0f) ? fRadius : 0.0f;
}

static void __xuiStepBarGeometry(xui_widget pWidget, const xui_step_bar_data_t* pResolved,
	int iIndex, xui_rect_t* pStepRect, xui_rect_t* pIndicatorRect)
{
	xui_rect_t tContent;
	xui_rect_t tStep;
	xui_rect_t tIndicator;
	float fRadius;
	float fSpacing;
	float fCenter;
	float fExtent;
	float fPosition;

	memset(&tStep, 0, sizeof(tStep));
	memset(&tIndicator, 0, sizeof(tIndicator));
	if ( (pWidget == NULL) || (pResolved == NULL) || (iIndex < 0) ||
	     (iIndex >= pResolved->iStepCount) ) {
		goto done;
	}
	tContent = xuiWidgetGetContentRect(pWidget);
	if ( (tContent.fW <= 0.0f) || (tContent.fH <= 0.0f) ) {
		goto done;
	}
	if ( pResolved->iStyle == XUI_STEP_BAR_STYLE_ARROW ) {
		float fHeight = (pResolved->fBarHeight > 0.0f) ? pResolved->fBarHeight : XUI_STEP_BAR_ARROW_H;
		float fStepWidth = tContent.fW / (float)pResolved->iStepCount;
		if ( fHeight > tContent.fH ) fHeight = tContent.fH;
		tStep = (xui_rect_t){
			tContent.fX + fStepWidth * (float)iIndex,
			tContent.fY + (tContent.fH - fHeight) * 0.5f,
			fStepWidth,
			fHeight
		};
		tIndicator = tStep;
		goto done;
	}
	if ( pResolved->iStyle == XUI_STEP_BAR_STYLE_DOT ) {
		fRadius = __xuiStepBarDotRadius(pResolved, tContent.fW, tContent.fH);
		fSpacing = (pResolved->iStepCount > 1) ?
			(tContent.fW - fRadius * 2.0f) / (float)(pResolved->iStepCount - 1) : 0.0f;
		fCenter = (pResolved->iStepCount > 1) ?
			tContent.fX + fRadius + fSpacing * (float)iIndex :
			tContent.fX + tContent.fW * 0.5f;
		fPosition = (tContent.fH >= fRadius * 2.0f + 4.0f) ?
			tContent.fY + fRadius + 4.0f : tContent.fY + tContent.fH * 0.5f;
		tIndicator = (xui_rect_t){fCenter - fRadius, fPosition - fRadius,
			fRadius * 2.0f, fRadius * 2.0f};
		fExtent = __xuiStepBarMinFloat(tContent.fW, 152.0f);
		if ( (pResolved->iStepCount > 1) && (fExtent > fSpacing) ) fExtent = fSpacing;
		fPosition = fCenter - fExtent * 0.5f;
		if ( fPosition < tContent.fX ) fPosition = tContent.fX;
		if ( fPosition + fExtent > tContent.fX + tContent.fW ) {
			fPosition = tContent.fX + tContent.fW - fExtent;
		}
		tStep = (xui_rect_t){fPosition, tContent.fY, fExtent, tContent.fH};
		goto done;
	}
	fRadius = __xuiStepBarDotRadius(pResolved, tContent.fH, tContent.fW);
	fSpacing = (pResolved->iStepCount > 1) ?
		(tContent.fH - fRadius * 2.0f) / (float)(pResolved->iStepCount - 1) : 0.0f;
	fCenter = (pResolved->iStepCount > 1) ?
		tContent.fY + fRadius + fSpacing * (float)iIndex :
		tContent.fY + tContent.fH * 0.5f;
	fPosition = (tContent.fW >= fRadius * 2.0f + 4.0f) ?
		tContent.fX + fRadius + 4.0f : tContent.fX + tContent.fW * 0.5f;
	tIndicator = (xui_rect_t){fPosition - fRadius, fCenter - fRadius,
		fRadius * 2.0f, fRadius * 2.0f};
	fExtent = __xuiStepBarMinFloat(tContent.fH, 56.0f);
	if ( (pResolved->iStepCount > 1) && (fExtent > fSpacing) ) fExtent = fSpacing;
	fPosition = fCenter - fExtent * 0.5f;
	if ( fPosition < tContent.fY ) fPosition = tContent.fY;
	if ( fPosition + fExtent > tContent.fY + tContent.fH ) {
		fPosition = tContent.fY + tContent.fH - fExtent;
	}
	tStep = (xui_rect_t){tContent.fX, fPosition, tContent.fW, fExtent};

done:
	if ( pStepRect != NULL ) *pStepRect = tStep;
	if ( pIndicatorRect != NULL ) *pIndicatorRect = tIndicator;
}

static int __xuiStepBarDrawText(xui_proxy pProxy, xui_draw_context pDraw, const xui_text_item_t* pTextItem, xui_rect_t tRect, uint32_t iColor, uint32_t iFlags)
{
    xui_font pFont = pTextItem && pTextItem->iSize >= sizeof(*pTextItem) ? pTextItem->pFont : NULL;
    const char* sText = pTextItem && pTextItem->iSize >= sizeof(*pTextItem) ? pTextItem->sText : NULL;

	if ( (pProxy == NULL) || (pDraw == NULL) || (pProxy->drawText == NULL) || (pFont == NULL) || (sText == NULL) || (sText[0] == '\0') ||
	     (tRect.fW <= 0.0f) || (tRect.fH <= 0.0f) || ((iColor & 0xffu) == 0u) ) {
		return XUI_OK;
	}
	return pProxy->drawText(pProxy, pDraw, pTextItem, xuiInternalSnapRect(tRect), iColor, iFlags);
}

static int __xuiStepBarDrawLine(xui_proxy pProxy, xui_draw_context pDraw,
	float fX0, float fY0, float fX1, float fY1, float fWidth, uint32_t iColor)
{
	if ( (pProxy == NULL) || (pDraw == NULL) || (pProxy->drawLine == NULL) ||
	     (fWidth <= 0.0f) || (__xuiStepBarAlpha(iColor) == 0) ) {
		return XUI_OK;
	}
	return pProxy->drawLine(pProxy, pDraw, fX0, fY0, fX1, fY1, fWidth, iColor);
}

static int __xuiStepBarDrawCircleFill(xui_proxy pProxy, xui_draw_context pDraw,
	float fX, float fY, float fRadius, uint32_t iColor)
{
	if ( (pProxy == NULL) || (pDraw == NULL) || (pProxy->drawCircleFill == NULL) ||
	     (fRadius <= 0.0f) || (__xuiStepBarAlpha(iColor) == 0) ) {
		return XUI_OK;
	}
	return pProxy->drawCircleFill(pProxy, pDraw, fX, fY, fRadius, iColor);
}

static int __xuiStepBarDrawCircleStroke(xui_proxy pProxy, xui_draw_context pDraw,
	float fX, float fY, float fRadius, float fWidth, uint32_t iColor)
{
	if ( (pProxy == NULL) || (pDraw == NULL) || (pProxy->drawCircleStroke == NULL) ||
	     (fRadius <= 0.0f) || (fWidth <= 0.0f) || (__xuiStepBarAlpha(iColor) == 0) ) {
		return XUI_OK;
	}
	return pProxy->drawCircleStroke(pProxy, pDraw, fX, fY, fRadius, fWidth, iColor);
}

static int __xuiStepBarDrawCheck(xui_proxy pProxy, xui_draw_context pDraw, float fX, float fY, float fRadius, uint32_t iColor)
{
	int iRet;

	if ( fRadius <= 0.0f ) {
		return XUI_OK;
	}
	iRet = __xuiStepBarDrawLine(pProxy, pDraw, fX - fRadius * 0.35f, fY,
		fX - fRadius * 0.08f, fY + fRadius * 0.28f, 1.5f, iColor);
	if ( iRet != XUI_OK ) return iRet;
	return __xuiStepBarDrawLine(pProxy, pDraw, fX - fRadius * 0.08f, fY + fRadius * 0.28f,
		fX + fRadius * 0.42f, fY - fRadius * 0.34f, 1.5f, iColor);
}

static int __xuiStepBarDrawArrow(xui_widget pWidget, xui_proxy pProxy, xui_draw_context pDraw, const xui_step_bar_data_t* pResolved)
{
	xui_rect_t tStep;
	xui_rect_t tRect;
	xui_vec2_t tA;
	xui_vec2_t tB;
	xui_vec2_t tC;
	char sText[160];
	float fArrowW;
	uint32_t iColor;
	int i;
	int iRet;

	for ( i = 0; i < pResolved->iStepCount; i++ ) {
		__xuiStepBarGeometry(pWidget, pResolved, i, &tStep, NULL);
		fArrowW = (tStep.fW < 28.0f) ? tStep.fW * 0.35f : 16.0f;
		iColor = __xuiStepBarStatusColor(pResolved, i);
		if ( (__xuiStepBarAlpha(iColor) != 0) && (pProxy->drawRectFill != NULL) &&
		     (tStep.fW > 0.0f) && (tStep.fH > 0.0f) ) {
			tRect = tStep;
			if ( i < pResolved->iStepCount - 1 ) {
				tRect.fW -= fArrowW;
			}
			iRet = pProxy->drawRectFill(pProxy, pDraw, xuiInternalSnapRect(tRect), iColor);
			if ( iRet != XUI_OK ) return iRet;
		}
		if ( i < pResolved->iStepCount - 1 && (__xuiStepBarAlpha(iColor) != 0) && pProxy->drawTriangleFill != NULL ) {
			tA = (xui_vec2_t){tStep.fX + tStep.fW - fArrowW, tStep.fY};
			tB = (xui_vec2_t){tStep.fX + tStep.fW, tStep.fY + tStep.fH * 0.5f};
			tC = (xui_vec2_t){tStep.fX + tStep.fW - fArrowW, tStep.fY + tStep.fH};
			iRet = pProxy->drawTriangleFill(pProxy, pDraw, tA, tB, tC, iColor);
			if ( iRet != XUI_OK ) return iRet;
			iRet = __xuiStepBarDrawLine(pProxy, pDraw, tA.fX, tA.fY,
				tB.fX, tB.fY, 1.0f, pResolved->iBackgroundColor);
			if ( iRet != XUI_OK ) return iRet;
			iRet = __xuiStepBarDrawLine(pProxy, pDraw, tB.fX, tB.fY,
				tC.fX, tC.fY, 1.0f, pResolved->iBackgroundColor);
			if ( iRet != XUI_OK ) return iRet;
		}
		__xuiStepBarFormatArrowTitle(sText, sizeof(sText), i, __xuiStepBarTitle(pResolved, i));
		tRect = tStep;
		tRect.fX += 8.0f;
		tRect.fW -= 14.0f + ((i < pResolved->iStepCount - 1) ? fArrowW : 0.0f);
		iRet = __xuiStepBarDrawText(pProxy, pDraw, &(xui_text_item_t){.iSize=sizeof(xui_text_item_t), .pFont=pResolved->pFont, .sText=sText, .iTextSize=-1, .iFlags=XUI_TEXT_SHAPE_DEFAULT | ((XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP) & XUI_TEXT_RTL ? XUI_TEXT_SHAPE_RTL : 0)}, tRect, pResolved->iArrowTextColor, XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP);
		if ( iRet != XUI_OK ) return iRet;
	}
	return XUI_OK;
}

static int __xuiStepBarDrawHorizontalDot(xui_widget pWidget, xui_proxy pProxy, xui_draw_context pDraw, const xui_step_bar_data_t* pResolved)
{
	xui_rect_t tContent;
	xui_rect_t tStep;
	xui_rect_t tIndicator;
	xui_rect_t tPrevIndicator;
	xui_rect_t tText;
	char sIndex[16];
	float fRadius;
	float fLineY;
	float fX;
	uint32_t iTextColor;
	uint32_t iCircleColor;
	uint32_t iTextFlags;
	int i;
	int iRet;

	tContent = xuiWidgetGetContentRect(pWidget);
	__xuiStepBarGeometry(pWidget, pResolved, 0, NULL, &tPrevIndicator);
	for ( i = 1; i < pResolved->iStepCount; i++ ) {
		__xuiStepBarGeometry(pWidget, pResolved, i, NULL, &tIndicator);
		iCircleColor = (i <= pResolved->iCurrent) ? pResolved->iDoneColor : pResolved->iLineColor;
		iRet = __xuiStepBarDrawLine(pProxy, pDraw,
			tPrevIndicator.fX + tPrevIndicator.fW,
			tPrevIndicator.fY + tPrevIndicator.fH * 0.5f,
			tIndicator.fX,
			tIndicator.fY + tIndicator.fH * 0.5f,
			(pResolved->fLineWidth > 0.0f) ? pResolved->fLineWidth : 1.0f,
			iCircleColor);
		if ( iRet != XUI_OK ) return iRet;
		tPrevIndicator = tIndicator;
	}
	for ( i = 0; i < pResolved->iStepCount; i++ ) {
		__xuiStepBarGeometry(pWidget, pResolved, i, &tStep, &tIndicator);
		fRadius = __xuiStepBarMinFloat(tIndicator.fW, tIndicator.fH) * 0.5f;
		fX = tIndicator.fX + tIndicator.fW * 0.5f;
		fLineY = tIndicator.fY + tIndicator.fH * 0.5f;
		iCircleColor = __xuiStepBarStatusColor(pResolved, i);
		if ( i < pResolved->iCurrent ) {
			iRet = __xuiStepBarDrawCircleFill(pProxy, pDraw, fX, fLineY, fRadius, pResolved->iBackgroundColor);
			if ( iRet != XUI_OK ) return iRet;
			iRet = __xuiStepBarDrawCircleStroke(pProxy, pDraw, fX, fLineY, fRadius, 1.0f, pResolved->iDoneColor);
			if ( iRet != XUI_OK ) return iRet;
			iRet = __xuiStepBarDrawCheck(pProxy, pDraw, fX, fLineY, fRadius, pResolved->iDoneColor);
			if ( iRet != XUI_OK ) return iRet;
		} else if ( i == pResolved->iCurrent ) {
			iRet = __xuiStepBarDrawCircleFill(pProxy, pDraw, fX, fLineY, fRadius, iCircleColor);
			if ( iRet != XUI_OK ) return iRet;
			iRet = __xuiStepBarDrawCheck(pProxy, pDraw, fX, fLineY, fRadius, pResolved->iActiveCheckColor);
			if ( iRet != XUI_OK ) return iRet;
		} else {
			iRet = __xuiStepBarDrawCircleFill(pProxy, pDraw, fX, fLineY, fRadius, pResolved->iBackgroundColor);
			if ( iRet != XUI_OK ) return iRet;
			iRet = __xuiStepBarDrawCircleStroke(pProxy, pDraw, fX, fLineY, fRadius, 1.0f, iCircleColor);
			if ( iRet != XUI_OK ) return iRet;
			snprintf(sIndex, sizeof(sIndex), "%d", i + 1);
			iRet = __xuiStepBarDrawText(pProxy, pDraw, &(xui_text_item_t){.iSize=sizeof(xui_text_item_t), .pFont=pResolved->pFont, .sText=sIndex, .iTextSize=-1, .iFlags=XUI_TEXT_SHAPE_DEFAULT | ((XUI_TEXT_ALIGN_CENTER | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP) & XUI_TEXT_RTL ? XUI_TEXT_SHAPE_RTL : 0)}, tIndicator, pResolved->iPendingTextColor, XUI_TEXT_ALIGN_CENTER | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP);
			if ( iRet != XUI_OK ) return iRet;
		}
		iTextColor = __xuiStepBarTextColor(pResolved, i);
		tText = (xui_rect_t){tStep.fX, fLineY + fRadius + 2.0f, tStep.fW,
			tContent.fY + tContent.fH - (fLineY + fRadius + 2.0f)};
		iTextFlags = XUI_TEXT_ALIGN_CENTER | XUI_TEXT_ALIGN_TOP | XUI_TEXT_CLIP;
		if ( (pResolved->iStepCount > 1) && (i == 0) ) {
			iTextFlags = XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_TOP | XUI_TEXT_CLIP;
		} else if ( (pResolved->iStepCount > 1) && (i == pResolved->iStepCount - 1) ) {
			iTextFlags = XUI_TEXT_ALIGN_RIGHT | XUI_TEXT_ALIGN_TOP | XUI_TEXT_CLIP;
		}
		iRet = __xuiStepBarDrawText(pProxy, pDraw, &(xui_text_item_t){.iSize=sizeof(xui_text_item_t), .pFont=pResolved->pFont, .sText=__xuiStepBarTitle(pResolved, i), .iTextSize=-1, .iFlags=XUI_TEXT_SHAPE_DEFAULT | ((iTextFlags) & XUI_TEXT_RTL ? XUI_TEXT_SHAPE_RTL : 0)}, tText, iTextColor, iTextFlags);
		if ( iRet != XUI_OK ) return iRet;
	}
	return XUI_OK;
}

static int __xuiStepBarDrawVerticalDot(xui_widget pWidget, xui_proxy pProxy, xui_draw_context pDraw, const xui_step_bar_data_t* pResolved)
{
	xui_rect_t tContent;
	xui_rect_t tStep;
	xui_rect_t tIndicator;
	xui_rect_t tPrevIndicator;
	xui_rect_t tText;
	char sIndex[16];
	float fRadius;
	float fLineX;
	float fY;
	float fTextHeight;
	uint32_t iTextColor;
	int i;
	int iRet;

	tContent = xuiWidgetGetContentRect(pWidget);
	__xuiStepBarGeometry(pWidget, pResolved, 0, NULL, &tPrevIndicator);
	for ( i = 1; i < pResolved->iStepCount; i++ ) {
		__xuiStepBarGeometry(pWidget, pResolved, i, NULL, &tIndicator);
		iTextColor = (i <= pResolved->iCurrent) ? pResolved->iDoneColor : pResolved->iLineColor;
		iRet = __xuiStepBarDrawLine(pProxy, pDraw,
			tPrevIndicator.fX + tPrevIndicator.fW * 0.5f,
			tPrevIndicator.fY + tPrevIndicator.fH,
			tIndicator.fX + tIndicator.fW * 0.5f,
			tIndicator.fY,
			(pResolved->fLineWidth > 0.0f) ? pResolved->fLineWidth : 1.0f,
			iTextColor);
		if ( iRet != XUI_OK ) return iRet;
		tPrevIndicator = tIndicator;
	}
	for ( i = 0; i < pResolved->iStepCount; i++ ) {
		__xuiStepBarGeometry(pWidget, pResolved, i, &tStep, &tIndicator);
		fRadius = __xuiStepBarMinFloat(tIndicator.fW, tIndicator.fH) * 0.5f;
		fLineX = tIndicator.fX + tIndicator.fW * 0.5f;
		fY = tIndicator.fY + tIndicator.fH * 0.5f;
		if ( i < pResolved->iCurrent ) {
			iRet = __xuiStepBarDrawCircleFill(pProxy, pDraw, fLineX, fY, fRadius, pResolved->iBackgroundColor);
			if ( iRet != XUI_OK ) return iRet;
			iRet = __xuiStepBarDrawCircleStroke(pProxy, pDraw, fLineX, fY, fRadius, 1.0f, pResolved->iDoneColor);
			if ( iRet != XUI_OK ) return iRet;
			iRet = __xuiStepBarDrawCheck(pProxy, pDraw, fLineX, fY, fRadius, pResolved->iDoneColor);
			if ( iRet != XUI_OK ) return iRet;
		} else if ( i == pResolved->iCurrent ) {
			iRet = __xuiStepBarDrawCircleFill(pProxy, pDraw, fLineX, fY, fRadius, pResolved->iActiveColor);
			if ( iRet != XUI_OK ) return iRet;
			iRet = __xuiStepBarDrawCheck(pProxy, pDraw, fLineX, fY, fRadius, pResolved->iActiveCheckColor);
			if ( iRet != XUI_OK ) return iRet;
		} else {
			iRet = __xuiStepBarDrawCircleFill(pProxy, pDraw, fLineX, fY, fRadius, pResolved->iBackgroundColor);
			if ( iRet != XUI_OK ) return iRet;
			iRet = __xuiStepBarDrawCircleStroke(pProxy, pDraw, fLineX, fY, fRadius, 1.0f, pResolved->iPendingColor);
			if ( iRet != XUI_OK ) return iRet;
			snprintf(sIndex, sizeof(sIndex), "%d", i + 1);
			iRet = __xuiStepBarDrawText(pProxy, pDraw, &(xui_text_item_t){.iSize=sizeof(xui_text_item_t), .pFont=pResolved->pFont, .sText=sIndex, .iTextSize=-1, .iFlags=XUI_TEXT_SHAPE_DEFAULT | ((XUI_TEXT_ALIGN_CENTER | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP) & XUI_TEXT_RTL ? XUI_TEXT_SHAPE_RTL : 0)}, tIndicator, pResolved->iPendingTextColor, XUI_TEXT_ALIGN_CENTER | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP);
			if ( iRet != XUI_OK ) return iRet;
		}
		iTextColor = __xuiStepBarTextColor(pResolved, i);
		fTextHeight = __xuiStepBarMinFloat(tStep.fH, 30.0f);
		tText = (xui_rect_t){fLineX + fRadius + 8.0f,
			tStep.fY + (tStep.fH - fTextHeight) * 0.5f,
			tContent.fW - (fLineX - tContent.fX) - fRadius - 10.0f,
			fTextHeight};
		iRet = __xuiStepBarDrawText(pProxy, pDraw, &(xui_text_item_t){.iSize=sizeof(xui_text_item_t), .pFont=pResolved->pFont, .sText=__xuiStepBarTitle(pResolved, i), .iTextSize=-1, .iFlags=XUI_TEXT_SHAPE_DEFAULT | ((XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP) & XUI_TEXT_RTL ? XUI_TEXT_SHAPE_RTL : 0)}, tText, iTextColor, XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP);
		if ( iRet != XUI_OK ) return iRet;
	}
	return XUI_OK;
}

static int __xuiStepBarContentMeasure(xui_widget pWidget, xui_vec2_t tConstraint, xui_vec2_t* pSize, void* pUser)
{
	xui_step_bar_data_t* pData;
	xui_step_bar_data_t tResolved;
	float fW;
	float fH;

	pData = (xui_step_bar_data_t*)pUser;
	if ( (pWidget == NULL) || (pSize == NULL) || (pData == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	__xuiStepBarResolve(pWidget, pData, &tResolved);
	if ( tResolved.iStyle == XUI_STEP_BAR_STYLE_VERTICAL ) {
		fW = XUI_STEP_BAR_VERTICAL_W;
		fH = (float)((tResolved.iStepCount > 0) ? tResolved.iStepCount : 3) * XUI_STEP_BAR_VERTICAL_ROW_H;
	} else if ( tResolved.iStyle == XUI_STEP_BAR_STYLE_DOT ) {
		fW = XUI_STEP_BAR_DEFAULT_W;
		fH = XUI_STEP_BAR_DOT_H;
	} else {
		fW = XUI_STEP_BAR_DEFAULT_W;
		fH = (tResolved.fBarHeight > 0.0f) ? tResolved.fBarHeight : XUI_STEP_BAR_ARROW_H;
	}
	if ( (tConstraint.fX > 0.0f) && (tConstraint.fX < XUI_LAYOUT_UNBOUNDED) ) fW = tConstraint.fX;
	if ( (tConstraint.fY > 0.0f) && (tConstraint.fY < XUI_LAYOUT_UNBOUNDED) ) fH = tConstraint.fY;
	pSize->fX = fW;
	pSize->fY = fH;
	return XUI_OK;
}

static int __xuiStepBarCacheRender(xui_widget pWidget, xui_draw_context pDraw, uint32_t iStateId, void* pUser)
{
	xui_step_bar_data_t* pData;
	xui_step_bar_data_t tResolved;
	xui_proxy pProxy;
	xui_rect_t tContent;
	int iRet;

	(void)iStateId;
	pData = (xui_step_bar_data_t*)pUser;
	if ( (pWidget == NULL) || (pDraw == NULL) || (pData == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pProxy = xuiInternalContextGetProxy(xuiWidgetGetContext(pWidget));
	if ( pProxy == NULL ) {
		return XUI_ERROR_NOT_INITIALIZED;
	}
	__xuiStepBarResolve(pWidget, pData, &tResolved);
	tContent = xuiWidgetGetContentRect(pWidget);
	if ( (tContent.fW > 0.0f) && (tContent.fH > 0.0f) &&
	     (tResolved.iBackgroundColor & 0xffu) != 0 && pProxy->drawRectFill != NULL ) {
		iRet = pProxy->drawRectFill(pProxy, pDraw, xuiInternalSnapRect(tContent), tResolved.iBackgroundColor);
		if ( iRet != XUI_OK ) return iRet;
	}
	if ( tResolved.iStepCount <= 0 ) {
		return XUI_OK;
	}
	if ( tResolved.iStyle == XUI_STEP_BAR_STYLE_VERTICAL ) {
		return __xuiStepBarDrawVerticalDot(pWidget, pProxy, pDraw, &tResolved);
	}
	if ( tResolved.iStyle == XUI_STEP_BAR_STYLE_DOT ) {
		return __xuiStepBarDrawHorizontalDot(pWidget, pProxy, pDraw, &tResolved);
	}
	return __xuiStepBarDrawArrow(pWidget, pProxy, pDraw, &tResolved);
}

static void __xuiStepBarDefaults(xui_step_bar_data_t* pData)
{
	memset(pData, 0, sizeof(*pData));
	pData->iStyle = XUI_STEP_BAR_STYLE_ARROW;
	pData->iCurrent = 0;
	pData->iDoneColor = XUI_COLOR_RGBA(151, 207, 166, 255);
	pData->iActiveColor = XUI_COLOR_RGBA(82, 181, 113, 255);
	pData->iPendingColor = XUI_COLOR_RGBA(204, 204, 204, 255);
	pData->iLineColor = XUI_COLOR_RGBA(207, 207, 207, 255);
	pData->iTextColor = XUI_COLOR_RGBA(82, 181, 113, 255);
	pData->iActiveTextColor = XUI_COLOR_RGBA(82, 181, 113, 255);
	pData->iPendingTextColor = XUI_COLOR_RGBA(0, 0, 0, 255);
	pData->iBackgroundColor = XUI_COLOR_RGBA(255, 255, 255, 0);
	pData->fBarHeight = XUI_STEP_BAR_ARROW_H;
	pData->fDotRadius = 11.0f;
	pData->fLineWidth = 1.0f;
}

static int __xuiStepBarInit(xui_widget pWidget, void* pTypeData, const void* pCreateData, void* pUser)
{
	xui_step_bar_data_t* pData;
	const xui_step_bar_desc_t* pDesc;
	int iRet;

	(void)pUser;
	pData = (xui_step_bar_data_t*)pTypeData;
	pDesc = (const xui_step_bar_desc_t*)pCreateData;
	if ( (pWidget == NULL) || (pData == NULL) || !__xuiStepBarDescValid(pDesc) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	__xuiStepBarDefaults(pData);
	if ( pDesc != NULL ) {
		pData->pFont = pDesc->pFont;
		pData->iCurrent = pDesc->iCurrent;
		pData->iStyle = pDesc->iStyle;
		if ( pDesc->iDoneColor != 0 ) pData->iDoneColor = pDesc->iDoneColor;
		if ( pDesc->iActiveColor != 0 ) pData->iActiveColor = pDesc->iActiveColor;
		if ( pDesc->iPendingColor != 0 ) pData->iPendingColor = pDesc->iPendingColor;
		if ( pDesc->iLineColor != 0 ) pData->iLineColor = pDesc->iLineColor;
		if ( pDesc->iTextColor != 0 ) pData->iTextColor = pDesc->iTextColor;
		if ( pDesc->iActiveTextColor != 0 ) pData->iActiveTextColor = pDesc->iActiveTextColor;
		if ( pDesc->iPendingTextColor != 0 ) pData->iPendingTextColor = pDesc->iPendingTextColor;
		if ( pDesc->iBackgroundColor != 0 ) pData->iBackgroundColor = pDesc->iBackgroundColor;
		if ( pDesc->fBarHeight > 0.0f ) pData->fBarHeight = pDesc->fBarHeight;
		if ( pDesc->fDotRadius > 0.0f ) pData->fDotRadius = pDesc->fDotRadius;
		if ( pDesc->fLineWidth > 0.0f ) pData->fLineWidth = pDesc->fLineWidth;
		iRet = __xuiStepBarSetTitlesData(pData, pDesc->ppTitles, pDesc->iStepCount);
		if ( iRet != XUI_OK ) return iRet;
	}
	if ( pData->pFont == NULL ) {
		pData->pFont = xuiGetDefaultFont(xuiWidgetGetContext(pWidget));
	}
	iRet = xuiWidgetSetFocusable(pWidget, 0);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiWidgetSetTabStop(pWidget, 0);
	if ( iRet != XUI_OK ) return iRet;
	return XUI_OK;
}

static void __xuiStepBarDestroy(xui_widget pWidget, void* pTypeData, void* pUser)
{
	(void)pWidget;
	(void)pUser;
	__xuiStepBarClearTitles((xui_step_bar_data_t*)pTypeData);
}

static void __xuiStepBarDefaultLayout(xui_layout_t* pLayout)
{
	memset(pLayout, 0, sizeof(*pLayout));
	pLayout->iLayoutType = XUI_LAYOUT_MANUAL;
	pLayout->iWidthMode = XUI_SIZE_CONTENT;
	pLayout->iHeightMode = XUI_SIZE_CONTENT;
	pLayout->iFlowMode = XUI_FLOW_BLOCK;
	pLayout->iDock = 0;
	pLayout->iOverflow = XUI_OVERFLOW_VISIBLE;
	pLayout->iAlignX = XUI_ALIGN_START;
	pLayout->iAlignY = XUI_ALIGN_START;
	pLayout->iTableRowSpan = 1;
	pLayout->iTableColumnSpan = 1;
	pLayout->iGridColumnCount = 1;
	pLayout->fMaxWidth = XUI_LAYOUT_UNBOUNDED;
	pLayout->fMaxHeight = XUI_LAYOUT_UNBOUNDED;
	pLayout->fShrink = 1.0f;
}

static void __xuiStepBarDefaultCachePolicy(xui_cache_policy_t* pPolicy)
{
	memset(pPolicy, 0, sizeof(*pPolicy));
	pPolicy->iSize = sizeof(*pPolicy);
	pPolicy->iPolicy = XUI_CACHE_POLICY_SELF;
	pPolicy->iFlags = XUI_CACHE_CLEAR_ON_UPDATE;
	pPolicy->iClearColor = XUI_COLOR_RGBA(0, 0, 0, 0);
}

static void __xuiStepBarRegisterStyleProperty(xui_context pContext, xui_widget_type pType, const char* sName, int iValueType, uint32_t iDirtyFlags, uint32_t iFlags)
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

static void __xuiStepBarRegisterStyleProperties(xui_context pContext, xui_widget_type pType)
{
	uint32_t iPaintDirty;
	uint32_t iLayoutDirty;

	iPaintDirty = XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER;
	iLayoutDirty = XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER;
	__xuiStepBarRegisterStyleProperty(pContext, pType, "stepbar.style", XUI_STYLE_VALUE_INT, iLayoutDirty, 0);
	__xuiStepBarRegisterStyleProperty(pContext, pType, "stepbar.done.color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiStepBarRegisterStyleProperty(pContext, pType, "stepbar.active.color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiStepBarRegisterStyleProperty(pContext, pType, "stepbar.pending.color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiStepBarRegisterStyleProperty(pContext, pType, "stepbar.line.color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiStepBarRegisterStyleProperty(pContext, pType, "stepbar.text.color", XUI_STYLE_VALUE_COLOR, iPaintDirty, XUI_STYLE_PROPERTY_INHERITED);
	__xuiStepBarRegisterStyleProperty(pContext, pType, "stepbar.text.active_color", XUI_STYLE_VALUE_COLOR, iPaintDirty, XUI_STYLE_PROPERTY_INHERITED);
	__xuiStepBarRegisterStyleProperty(pContext, pType, "stepbar.text.pending_color", XUI_STYLE_VALUE_COLOR, iPaintDirty, XUI_STYLE_PROPERTY_INHERITED);
	__xuiStepBarRegisterStyleProperty(pContext, pType, "stepbar.arrow.text_color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiStepBarRegisterStyleProperty(pContext, pType, "stepbar.check.active_color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiStepBarRegisterStyleProperty(pContext, pType, "stepbar.background.color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiStepBarRegisterStyleProperty(pContext, pType, "stepbar.bar_height", XUI_STYLE_VALUE_FLOAT, iLayoutDirty, 0);
	__xuiStepBarRegisterStyleProperty(pContext, pType, "stepbar.dot_radius", XUI_STYLE_VALUE_FLOAT, iLayoutDirty, 0);
	__xuiStepBarRegisterStyleProperty(pContext, pType, "stepbar.line_width", XUI_STYLE_VALUE_FLOAT, iPaintDirty, 0);
	__xuiStepBarRegisterStyleProperty(pContext, pType, "font.name", XUI_STYLE_VALUE_STRING, iLayoutDirty, XUI_STYLE_PROPERTY_INHERITED);
}

static int __xuiStepBarAccessibleNode(xui_widget pWidget, xui_accessible_node_t* pNode)
{
	xui_step_bar_data_t* pData;
	const char* sTitle;

	pData = __xuiStepBarGetData(pWidget);
	if ( (pData == NULL) || (pNode == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	pNode->iRole = XUI_ACCESSIBLE_ROLE_PROGRESS;
	pNode->iState |= XUI_ACCESSIBLE_STATE_READONLY;
	if ( pData->iStepCount > 0 ) {
		sTitle = __xuiStepBarTitle(pData, pData->iCurrent);
		if ( sTitle[0] != '\0' ) {
			(void)snprintf(pData->sAccessibleValue, sizeof(pData->sAccessibleValue),
				"%d/%d %s", pData->iCurrent + 1, pData->iStepCount, sTitle);
		} else {
			(void)snprintf(pData->sAccessibleValue, sizeof(pData->sAccessibleValue),
				"%d/%d", pData->iCurrent + 1, pData->iStepCount);
		}
		pData->sAccessibleValue[sizeof(pData->sAccessibleValue) - 1u] = '\0';
		xuiInternalUtf8TrimIncomplete(pData->sAccessibleValue);
		pNode->sValue = pData->sAccessibleValue;
		pNode->tRange.iSize = sizeof(pNode->tRange);
		pNode->tRange.fMin = 1.0;
		pNode->tRange.fMax = (double)pData->iStepCount;
		pNode->tRange.fValue = (double)(pData->iCurrent + 1);
	}
	return XUI_OK;
}

static const xui_internal_accessibility_adapter_t g_xuiStepBarAccessible = {
	.getNode = __xuiStepBarAccessibleNode,
	.performAction = NULL
};

XUI_API xui_widget_type xuiStepBarGetType(xui_context pContext)
{
	xui_widget_type_desc_t tDesc;
	xui_widget_type pType;
	int iRet;

	if ( !xuiInternalContextIsValid(pContext) ) {
		return NULL;
	}
	pType = xuiWidgetFindType(pContext, "stepbar");
	if ( pType != NULL ) {
		__xuiStepBarRegisterStyleProperties(pContext, pType);
		pType->pAccessibleAdapter = &g_xuiStepBarAccessible;
		return pType;
	}
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.sName = "stepbar";
	tDesc.pParent = xuiWidgetGetBaseType();
	tDesc.iFlags = XUI_WIDGET_TYPE_DEFAULT_LAYOUT | XUI_WIDGET_TYPE_DEFAULT_CACHE_POLICY;
	tDesc.iTypeDataSize = sizeof(xui_step_bar_data_t);
	tDesc.onInit = __xuiStepBarInit;
	tDesc.onDestroy = __xuiStepBarDestroy;
	tDesc.onContentMeasure = __xuiStepBarContentMeasure;
	tDesc.onCacheRender = __xuiStepBarCacheRender;
	__xuiStepBarDefaultLayout(&tDesc.tLayout);
	__xuiStepBarDefaultCachePolicy(&tDesc.tCachePolicy);
	iRet = xuiWidgetRegisterType(pContext, &pType, &tDesc);
	if ( iRet != XUI_OK ) {
		return NULL;
	}
	__xuiStepBarRegisterStyleProperties(pContext, pType);
	pType->pAccessibleAdapter = &g_xuiStepBarAccessible;
	return pType;
}

XUI_API int xuiStepBarCreate(xui_context pContext, xui_widget* ppWidget, const xui_step_bar_desc_t* pDesc)
{
	xui_widget_type pType;

	if ( ppWidget == NULL ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	*ppWidget = NULL;
	if ( !xuiInternalContextIsValid(pContext) || !__xuiStepBarDescValid(pDesc) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pType = xuiStepBarGetType(pContext);
	if ( pType == NULL ) {
		return XUI_ERROR_NOT_INITIALIZED;
	}
	return xuiWidgetCreateTyped(pContext, pType, ppWidget, pDesc);
}

XUI_API int xuiStepBarSetSteps(xui_widget pWidget, const char* const* ppTitles, int iStepCount)
{
	xui_step_bar_data_t* pData;
	int iRet;

	pData = __xuiStepBarGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( (iStepCount < 0) || (iStepCount > XUI_STEP_BAR_MAX_STEPS) ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( __xuiStepBarTitlesEqual(pData, ppTitles, iStepCount) ) return XUI_OK;
	iRet = __xuiStepBarSetTitlesData(pData, ppTitles, iStepCount);
	if ( iRet != XUI_OK ) return iRet;
	pData->iChangeCount++;
	xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_NODE_CHANGED);
	xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_VALUE_CHANGED);
	return xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
}

XUI_API int xuiStepBarSetTitle(xui_widget pWidget, int iIndex, const char* sTitle)
{
	xui_step_bar_data_t* pData;
	int iRet;

	pData = __xuiStepBarGetData(pWidget);
	if ( pData == NULL || iIndex < 0 || iIndex >= pData->iStepCount ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( strcmp(__xuiStepBarTitle(pData, iIndex), sTitle != NULL ? sTitle : "") == 0 ) return XUI_OK;
	iRet = __xuiStepBarStringSet(&pData->arrTitles[iIndex], sTitle);
	if ( iRet != XUI_OK ) return iRet;
	pData->iChangeCount++;
	xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_NODE_CHANGED);
	if ( iIndex == pData->iCurrent ) {
		xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_VALUE_CHANGED);
	}
	return xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
}

XUI_API int xuiStepBarGetStepCount(xui_widget pWidget)
{
	xui_step_bar_data_t* pData;

	pData = __xuiStepBarGetData(pWidget);
	return (pData != NULL) ? pData->iStepCount : 0;
}

XUI_API const char* xuiStepBarGetTitle(xui_widget pWidget, int iIndex)
{
	xui_step_bar_data_t* pData;

	pData = __xuiStepBarGetData(pWidget);
	return __xuiStepBarTitle(pData, iIndex);
}

XUI_API int xuiStepBarSetCurrent(xui_widget pWidget, int iCurrent)
{
	xui_step_bar_data_t* pData;

	pData = __xuiStepBarGetData(pWidget);
	if ( pData == NULL || iCurrent < 0 ||
	     (pData->iStepCount > 0 && iCurrent >= pData->iStepCount) ||
	     (pData->iStepCount == 0 && iCurrent != 0) ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pData->iCurrent == iCurrent ) return XUI_OK;
	pData->iCurrent = iCurrent;
	pData->iChangeCount++;
	xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_VALUE_CHANGED);
	return xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
}

XUI_API int xuiStepBarGetCurrent(xui_widget pWidget)
{
	xui_step_bar_data_t* pData;

	pData = __xuiStepBarGetData(pWidget);
	return (pData != NULL) ? pData->iCurrent : 0;
}

XUI_API int xuiStepBarSetStyle(xui_widget pWidget, int iStyle)
{
	xui_step_bar_data_t* pData;

	pData = __xuiStepBarGetData(pWidget);
	if ( pData == NULL || !__xuiStepBarStyleValid(iStyle) ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pData->iStyle == iStyle ) return XUI_OK;
	pData->iStyle = iStyle;
	pData->iChangeCount++;
	return xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
}

XUI_API int xuiStepBarGetStyle(xui_widget pWidget)
{
	xui_step_bar_data_t* pData;

	pData = __xuiStepBarGetData(pWidget);
	return (pData != NULL) ? pData->iStyle : XUI_STEP_BAR_STYLE_ARROW;
}

XUI_API int xuiStepBarSetFont(xui_widget pWidget, xui_font pFont)
{
	xui_step_bar_data_t* pData;

	pData = __xuiStepBarGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pData->pFont == pFont ) return XUI_OK;
	pData->pFont = pFont;
	return xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
}

XUI_API xui_font xuiStepBarGetFont(xui_widget pWidget)
{
	xui_step_bar_data_t* pData;

	pData = __xuiStepBarGetData(pWidget);
	return (pData != NULL) ? pData->pFont : NULL;
}

XUI_API int xuiStepBarSetColors(xui_widget pWidget, uint32_t iDone, uint32_t iActive, uint32_t iPending, uint32_t iLine)
{
	xui_step_bar_data_t* pData;

	pData = __xuiStepBarGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pData->iDoneColor == iDone && pData->iActiveColor == iActive &&
	     pData->iPendingColor == iPending && pData->iLineColor == iLine ) return XUI_OK;
	pData->iDoneColor = iDone;
	pData->iActiveColor = iActive;
	pData->iPendingColor = iPending;
	pData->iLineColor = iLine;
	return xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
}

XUI_API int xuiStepBarGetColors(xui_widget pWidget, uint32_t* pDone, uint32_t* pActive, uint32_t* pPending, uint32_t* pLine)
{
	xui_step_bar_data_t* pData;

	pData = __xuiStepBarGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pDone != NULL ) *pDone = pData->iDoneColor;
	if ( pActive != NULL ) *pActive = pData->iActiveColor;
	if ( pPending != NULL ) *pPending = pData->iPendingColor;
	if ( pLine != NULL ) *pLine = pData->iLineColor;
	return XUI_OK;
}

XUI_API int xuiStepBarSetTextColors(xui_widget pWidget, uint32_t iText, uint32_t iActiveText, uint32_t iPendingText)
{
	xui_step_bar_data_t* pData;

	pData = __xuiStepBarGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pData->iTextColor == iText && pData->iActiveTextColor == iActiveText &&
	     pData->iPendingTextColor == iPendingText ) return XUI_OK;
	pData->iTextColor = iText;
	pData->iActiveTextColor = iActiveText;
	pData->iPendingTextColor = iPendingText;
	return xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
}

XUI_API int xuiStepBarGetTextColors(xui_widget pWidget, uint32_t* pText, uint32_t* pActiveText, uint32_t* pPendingText)
{
	xui_step_bar_data_t* pData;

	pData = __xuiStepBarGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pText != NULL ) *pText = pData->iTextColor;
	if ( pActiveText != NULL ) *pActiveText = pData->iActiveTextColor;
	if ( pPendingText != NULL ) *pPendingText = pData->iPendingTextColor;
	return XUI_OK;
}

XUI_API int xuiStepBarSetBackgroundColor(xui_widget pWidget, uint32_t iColor)
{
	xui_step_bar_data_t* pData;

	pData = __xuiStepBarGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pData->iBackgroundColor == iColor ) return XUI_OK;
	pData->iBackgroundColor = iColor;
	return xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
}

XUI_API uint32_t xuiStepBarGetBackgroundColor(xui_widget pWidget)
{
	xui_step_bar_data_t* pData;

	pData = __xuiStepBarGetData(pWidget);
	return (pData != NULL) ? pData->iBackgroundColor : 0u;
}

XUI_API int xuiStepBarSetMetrics(xui_widget pWidget, float fBarHeight, float fDotRadius, float fLineWidth)
{
	xui_step_bar_data_t* pData;

	pData = __xuiStepBarGetData(pWidget);
	if ( pData == NULL ||
	     !__xuiStepBarNonNegativeFloatValid(fBarHeight) ||
	     !__xuiStepBarNonNegativeFloatValid(fDotRadius) ||
	     !__xuiStepBarNonNegativeFloatValid(fLineWidth) ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( (fBarHeight == 0.0f || pData->fBarHeight == fBarHeight) &&
	     (fDotRadius == 0.0f || pData->fDotRadius == fDotRadius) &&
	     (fLineWidth == 0.0f || pData->fLineWidth == fLineWidth) ) return XUI_OK;
	if ( fBarHeight > 0.0f ) pData->fBarHeight = fBarHeight;
	if ( fDotRadius > 0.0f ) pData->fDotRadius = fDotRadius;
	if ( fLineWidth > 0.0f ) pData->fLineWidth = fLineWidth;
	return xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
}

XUI_API int xuiStepBarGetMetrics(xui_widget pWidget, float* pBarHeight, float* pDotRadius, float* pLineWidth)
{
	xui_step_bar_data_t* pData;

	pData = __xuiStepBarGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pBarHeight != NULL ) *pBarHeight = pData->fBarHeight;
	if ( pDotRadius != NULL ) *pDotRadius = pData->fDotRadius;
	if ( pLineWidth != NULL ) *pLineWidth = pData->fLineWidth;
	return XUI_OK;
}

static xui_rect_t __xuiStepBarGetComputedRect(xui_widget pWidget, int iIndex, int bIndicator)
{
	xui_step_bar_data_t* pData;
	xui_step_bar_data_t tResolved;
	xui_rect_t tStep;
	xui_rect_t tIndicator;

	pData = __xuiStepBarGetData(pWidget);
	if ( pData == NULL || iIndex < 0 || iIndex >= pData->iStepCount ) return (xui_rect_t){0.0f, 0.0f, 0.0f, 0.0f};
	__xuiStepBarResolve(pWidget, pData, &tResolved);
	__xuiStepBarGeometry(pWidget, &tResolved, iIndex, &tStep, &tIndicator);
	return xuiInternalSnapRect(bIndicator ? tIndicator : tStep);
}

XUI_API xui_rect_t xuiStepBarGetStepRect(xui_widget pWidget, int iIndex)
{
	return __xuiStepBarGetComputedRect(pWidget, iIndex, 0);
}

XUI_API xui_rect_t xuiStepBarGetIndicatorRect(xui_widget pWidget, int iIndex)
{
	return __xuiStepBarGetComputedRect(pWidget, iIndex, 1);
}

XUI_API int xuiStepBarGetChangeCount(xui_widget pWidget)
{
	xui_step_bar_data_t* pData;

	pData = __xuiStepBarGetData(pWidget);
	return (pData != NULL) ? pData->iChangeCount : 0;
}

#endif
