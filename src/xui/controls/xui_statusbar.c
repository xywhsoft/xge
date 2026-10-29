#include "xui_config.h"
#if XUI_ENABLE_STATUSBAR
#include "xui_internal.h"
#include "xui_pointer_press_internal.h"

#include <limits.h>
#include <math.h>
#include <string.h>

#define XUI_STATUSBAR_ITEM_STATE_MASK (XUI_STATUSBAR_ITEM_ENABLED | XUI_STATUSBAR_ITEM_CLICKABLE)

typedef struct xui_statusbar_data_t {
	xui_statusbar_item_t arrItems[XUI_STATUSBAR_ITEM_CAPACITY];
	char* arrOwnedTexts[XUI_STATUSBAR_ITEM_CAPACITY];
	xui_statusbar_metrics_t tMetrics;
	xui_statusbar_colors_t tColors;
	xui_statusbar_select_proc onSelect;
	void* pSelectUser;
	xui_statusbar_context_proc onContext;
	void* pContextUser;
	xui_font pFont;
	int iItemCount;
	int iHover;
	int iActive;
	xui_pointer_owner_t tPointerOwner;
	int iSelectCount;
	int iChangeCount;
} xui_statusbar_data_t;

typedef struct xui_statusbar_resolved_t {
	uint32_t iActiveTextColor;
	xui_statusbar_metrics_t tMetrics;
	xui_statusbar_colors_t tColors;
	xui_font pFont;
} xui_statusbar_resolved_t;

static xui_statusbar_data_t* __xuiStatusBarGetData(xui_widget pWidget);
static int __xuiStatusBarMetricsValid(const xui_statusbar_metrics_t* pMetrics);
static int __xuiStatusBarColorsValid(const xui_statusbar_colors_t* pColors);

static void __xuiStatusBarMarkChanged(xui_statusbar_data_t* pData)
{
	if ( pData != NULL && pData->iChangeCount < INT_MAX ) pData->iChangeCount++;
}

static void __xuiStatusBarIncrementCount(int* pCount)
{
	if ( pCount != NULL && *pCount < INT_MAX ) (*pCount)++;
}

static char* __xuiStatusBarCopyString(const char* sText)
{
	size_t iLength;
	char* sCopy;

	if ( sText == NULL || sText[0] == '\0' ) return NULL;
	iLength = strlen(sText);
	sCopy = (char*)xrtMalloc(iLength + 1u);
	if ( sCopy != NULL ) memcpy(sCopy, sText, iLength + 1u);
	return sCopy;
}

static void __xuiStatusBarFreeStrings(char** arrTexts, int iCount)
{
	int i;

	if ( arrTexts == NULL ) return;
	for ( i = 0; i < iCount; i++ ) {
		xrtFree(arrTexts[i]);
		arrTexts[i] = NULL;
	}
}

static int __xuiStatusBarAlpha(uint32_t iColor)
{
	return (int)(iColor & 0xffu);
}

static uint32_t __xuiStatusBarColorAlpha(uint32_t iColor, uint32_t iAlpha)
{
	if ( (iColor & 0xffu) == 0u ) return iColor;
	if ( iAlpha > 255u ) iAlpha = 255u;
	return (iColor & 0xffffff00u) | (iAlpha & 0xffu);
}

static int __xuiStatusBarSectionValid(int iSection)
{
	return (iSection == XUI_STATUSBAR_SECTION_LEFT) ||
	       (iSection == XUI_STATUSBAR_SECTION_CENTER) ||
	       (iSection == XUI_STATUSBAR_SECTION_RIGHT);
}

static int __xuiStatusBarItemTypeValid(int iType)
{
	return (iType == XUI_STATUSBAR_ITEM_TEXT) ||
	       (iType == XUI_STATUSBAR_ITEM_PROGRESS) ||
	       (iType == XUI_STATUSBAR_ITEM_SPACER);
}

static int __xuiStatusBarFloatValid(float fValue)
{
	return isfinite(fValue);
}

static int __xuiStatusBarNonNegativeFloatValid(float fValue)
{
	return __xuiStatusBarFloatValid(fValue) && fValue >= 0.0f &&
	       fValue <= XUI_LAYOUT_UNBOUNDED;
}

static int __xuiStatusBarItemValid(const xui_statusbar_item_t* pItem)
{
	if ( (pItem == NULL) || !__xuiStatusBarItemTypeValid(pItem->iType) ||
	     !__xuiStatusBarSectionValid(pItem->iSection) ||
	     (pItem->iState & ~XUI_STATUSBAR_ITEM_STATE_MASK) != 0u ||
	     !__xuiStatusBarNonNegativeFloatValid(pItem->fWidth) ||
	     !__xuiStatusBarNonNegativeFloatValid(pItem->fFlex) ) {
		return 0;
	}
	if ( pItem->iType == XUI_STATUSBAR_ITEM_PROGRESS ) {
		return __xuiStatusBarFloatValid(pItem->fMin) &&
		       __xuiStatusBarFloatValid(pItem->fMax) &&
		       __xuiStatusBarFloatValid(pItem->fValue);
	}
	return 1;
}

static const char* __xuiStatusBarStringValue(const char* sText)
{
	return sText != NULL ? sText : "";
}

static int __xuiStatusBarItemEqual(const xui_statusbar_item_t* pCurrent,
	const xui_statusbar_item_t* pNext)
{
	uint32_t iState;
	float fMax;
	float fValue;

	if ( pCurrent == NULL || pNext == NULL ) return 0;
	iState = pNext->iType == XUI_STATUSBAR_ITEM_SPACER ? 0u : pNext->iState;
	fMax = pNext->fMax;
	fValue = pNext->fValue;
	if ( pNext->iType == XUI_STATUSBAR_ITEM_PROGRESS ) {
		if ( fMax < pNext->fMin ) fMax = pNext->fMin;
		if ( fValue < pNext->fMin ) fValue = pNext->fMin;
		if ( fValue > fMax ) fValue = fMax;
	}
	return strcmp(__xuiStatusBarStringValue(pCurrent->sText),
			__xuiStatusBarStringValue(pNext->sText)) == 0 &&
		pCurrent->iType == pNext->iType && pCurrent->iSection == pNext->iSection &&
		pCurrent->iState == iState && pCurrent->iValue == pNext->iValue &&
		pCurrent->fWidth == pNext->fWidth && pCurrent->fFlex == pNext->fFlex &&
		pCurrent->fMin == pNext->fMin && pCurrent->fMax == fMax &&
		pCurrent->fValue == fValue && pCurrent->pUser == pNext->pUser;
}

static int __xuiStatusBarItemInteractive(const xui_statusbar_item_t* pItem)
{
	return (pItem != NULL) &&
	       (pItem->iType != XUI_STATUSBAR_ITEM_SPACER) &&
	       ((pItem->iState & XUI_STATUSBAR_ITEM_ENABLED) != 0u) &&
	       ((pItem->iState & XUI_STATUSBAR_ITEM_CLICKABLE) != 0u);
}

static int __xuiStatusBarRectContains(xui_rect_t tRect, float fX, float fY)
{
	return (fX >= tRect.fX) && (fY >= tRect.fY) && (fX < tRect.fX + tRect.fW) && (fY < tRect.fY + tRect.fH);
}

static xui_rect_t __xuiStatusBarInsetRect(xui_rect_t tRect, float fLeft, float fTop, float fRight, float fBottom)
{
	tRect.fX += fLeft;
	tRect.fY += fTop;
	tRect.fW -= fLeft + fRight;
	tRect.fH -= fTop + fBottom;
	if ( tRect.fW < 0.0f ) tRect.fW = 0.0f;
	if ( tRect.fH < 0.0f ) tRect.fH = 0.0f;
	return tRect;
}

static int __xuiStatusBarDescValid(const xui_statusbar_desc_t* pDesc)
{
	int i;

	if ( pDesc == NULL ) return 1;
	if ( (pDesc->iSize != 0) && (pDesc->iSize < sizeof(*pDesc)) ) return 0;
	if ( (pDesc->iItemCount < 0) || (pDesc->iItemCount > XUI_STATUSBAR_ITEM_CAPACITY) ) return 0;
	if ( (pDesc->iItemCount > 0) && (pDesc->pItems == NULL) ) return 0;
	if ( pDesc->bHasMetrics && !__xuiStatusBarMetricsValid(&pDesc->tMetrics) ) return 0;
	if ( pDesc->bHasColors && !__xuiStatusBarColorsValid(&pDesc->tColors) ) return 0;
	for ( i = 0; i < pDesc->iItemCount; i++ ) {
		if ( !__xuiStatusBarItemValid(&pDesc->pItems[i]) ) return 0;
	}
	return 1;
}

static int __xuiStatusBarMetricsValid(const xui_statusbar_metrics_t* pMetrics)
{
	double fHorizontal;
	double fVertical;
	double fItemHorizontal;
	double fItemVertical;

	if ( pMetrics == NULL ) return 0;
	if ( (pMetrics->iSize != 0) && (pMetrics->iSize < sizeof(*pMetrics)) ) return 0;
	if ( !__xuiStatusBarNonNegativeFloatValid(pMetrics->fHeight) || (pMetrics->fHeight <= 0.0f) ||
	     !__xuiStatusBarNonNegativeFloatValid(pMetrics->fPaddingX) ||
	     !__xuiStatusBarNonNegativeFloatValid(pMetrics->fPaddingY) ||
	     !__xuiStatusBarNonNegativeFloatValid(pMetrics->fGap) ||
	     !__xuiStatusBarNonNegativeFloatValid(pMetrics->fItemPaddingX) ||
	     !__xuiStatusBarNonNegativeFloatValid(pMetrics->fItemPaddingY) ||
	     !__xuiStatusBarNonNegativeFloatValid(pMetrics->fProgressHeight) || (pMetrics->fProgressHeight <= 0.0f) ||
	     !__xuiStatusBarNonNegativeFloatValid(pMetrics->fBorderWidth) ||
	     !__xuiStatusBarNonNegativeFloatValid(pMetrics->fTopBorderWidth) ) {
		return 0;
	}
	fHorizontal = (double)pMetrics->fPaddingX * 2.0 +
		(double)pMetrics->fGap * (double)(XUI_STATUSBAR_ITEM_CAPACITY - 1);
	fVertical = (double)pMetrics->fPaddingY * 2.0;
	fItemHorizontal = (double)pMetrics->fItemPaddingX * 2.0;
	fItemVertical = (double)pMetrics->fItemPaddingY * 2.0;
	if ( fHorizontal > (double)XUI_LAYOUT_UNBOUNDED ||
	     fVertical > (double)XUI_LAYOUT_UNBOUNDED ||
	     fItemHorizontal > (double)XUI_LAYOUT_UNBOUNDED ||
	     fItemVertical > (double)XUI_LAYOUT_UNBOUNDED ||
	     (double)pMetrics->fBorderWidth * 2.0 > (double)XUI_LAYOUT_UNBOUNDED ||
	     (double)pMetrics->fTopBorderWidth + 1.0 > (double)XUI_LAYOUT_UNBOUNDED ) {
		return 0;
	}
	return 1;
}

static int __xuiStatusBarColorsValid(const xui_statusbar_colors_t* pColors)
{
	if ( pColors == NULL ) return 0;
	return (pColors->iSize == 0) || (pColors->iSize >= sizeof(*pColors));
}

static int __xuiStatusBarMetricsEqual(const xui_statusbar_metrics_t* pA, const xui_statusbar_metrics_t* pB)
{
	return pA->fHeight == pB->fHeight && pA->fPaddingX == pB->fPaddingX &&
		pA->fPaddingY == pB->fPaddingY && pA->fGap == pB->fGap &&
		pA->fItemPaddingX == pB->fItemPaddingX && pA->fItemPaddingY == pB->fItemPaddingY &&
		pA->fProgressHeight == pB->fProgressHeight && pA->fBorderWidth == pB->fBorderWidth &&
		pA->fTopBorderWidth == pB->fTopBorderWidth;
}

static int __xuiStatusBarColorsEqual(const xui_statusbar_colors_t* pA, const xui_statusbar_colors_t* pB)
{
	return pA->iBackgroundColor == pB->iBackgroundColor && pA->iBorderColor == pB->iBorderColor &&
		pA->iHighlightColor == pB->iHighlightColor && pA->iItemColor == pB->iItemColor &&
		pA->iHoverColor == pB->iHoverColor && pA->iActiveColor == pB->iActiveColor &&
		pA->iFocusColor == pB->iFocusColor && pA->iTextColor == pB->iTextColor &&
		pA->iDisabledTextColor == pB->iDisabledTextColor &&
		pA->iProgressTrackColor == pB->iProgressTrackColor &&
		pA->iProgressFillColor == pB->iProgressFillColor;
}

static void __xuiStatusBarDefaultMetrics(xui_statusbar_metrics_t* pMetrics)
{
	memset(pMetrics, 0, sizeof(*pMetrics));
	pMetrics->iSize = sizeof(*pMetrics);
	pMetrics->fHeight = 26.0f;
	pMetrics->fPaddingX = 8.0f;
	pMetrics->fPaddingY = 2.0f;
	pMetrics->fGap = 6.0f;
	pMetrics->fItemPaddingX = 8.0f;
	pMetrics->fItemPaddingY = 3.0f;
	pMetrics->fProgressHeight = 9.0f;
	pMetrics->fBorderWidth = 1.0f;
	pMetrics->fTopBorderWidth = 1.0f;
}

static void __xuiStatusBarDefaultColors(xui_statusbar_colors_t* pColors)
{
	memset(pColors, 0, sizeof(*pColors));
	pColors->iSize = sizeof(*pColors);
	pColors->iBackgroundColor = XUI_COLOR_RGBA(241, 247, 253, 255);
	pColors->iBorderColor = XUI_COLOR_RGBA(192, 210, 228, 255);
	pColors->iHighlightColor = XUI_COLOR_RGBA(255, 255, 255, 130);
	pColors->iItemColor = XUI_COLOR_RGBA(0, 0, 0, 0);
	pColors->iHoverColor = XUI_COLOR_RGBA(225, 239, 251, 255);
	pColors->iActiveColor = XUI_COLOR_RGBA(47, 128, 214, 255);
	pColors->iFocusColor = XUI_COLOR_RGBA(47, 128, 214, 255);
	pColors->iTextColor = XUI_COLOR_RGBA(38, 59, 86, 255);
	pColors->iDisabledTextColor = XUI_COLOR_RGBA(145, 157, 170, 210);
	pColors->iProgressTrackColor = XUI_COLOR_RGBA(214, 231, 246, 255);
	pColors->iProgressFillColor = XUI_COLOR_RGBA(47, 128, 214, 255);
}

static xui_vec2_t __xuiStatusBarMeasureText(xui_widget pWidget, xui_font pFont, const char* sText)
{
	xui_vec2_t tSize;
	xui_proxy pProxy;
	double fFallbackWidth;

	tSize.fX = 0.0f;
	tSize.fY = 16.0f;
	if ( (sText == NULL) || (sText[0] == '\0') ) return tSize;
	pProxy = xuiInternalContextGetProxy(xuiWidgetGetContext(pWidget));
	if ( (pProxy != NULL) && (pProxy->textMeasure != NULL) && (pFont != NULL) &&
	     (pProxy->textMeasure(pProxy, &(xui_text_item_t){.iSize=sizeof(xui_text_item_t), .pFont=pFont, .sText=sText, .iTextSize=-1, .iFlags=XUI_TEXT_SHAPE_DEFAULT}, &tSize) == XUI_OK) &&
	     __xuiStatusBarNonNegativeFloatValid(tSize.fX) &&
	     __xuiStatusBarNonNegativeFloatValid(tSize.fY) ) {
		return tSize;
	}
	fFallbackWidth = (double)strlen(sText) * 7.0;
	if ( fFallbackWidth > (double)XUI_LAYOUT_UNBOUNDED ) {
		fFallbackWidth = (double)XUI_LAYOUT_UNBOUNDED;
	}
	tSize.fX = (float)fFallbackWidth;
	tSize.fY = 16.0f;
	return tSize;
}

static int __xuiStatusBarStyleColor(xui_widget pWidget, const char* sName, uint32_t* pColor)
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

static int __xuiStatusBarStyleFloat(xui_widget pWidget, const char* sName, float* pValue)
{
	xui_style_property_t tProperty;
	int iRet;

	memset(&tProperty, 0, sizeof(tProperty));
	tProperty.iSize = sizeof(tProperty);
	iRet = xuiWidgetGetResolvedStyleProperty(pWidget, sName, &tProperty);
	if ( (iRet == XUI_OK) && (tProperty.tValue.iType == XUI_STYLE_VALUE_FLOAT) &&
	     __xuiStatusBarNonNegativeFloatValid(tProperty.tValue.fFloat) ) {
		*pValue = tProperty.tValue.fFloat;
		return 1;
	}
	return 0;
}

static xui_font __xuiStatusBarStyleFont(xui_widget pWidget, xui_font pBaseFont)
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
		if ( pFont != NULL ) return pFont;
	}
	return pBaseFont;
}

static void __xuiStatusBarResolve(xui_widget pWidget, const xui_statusbar_data_t* pData, xui_statusbar_resolved_t* pOut)
{
	xui_statusbar_metrics_t tMetrics;

	memset(pOut, 0, sizeof(*pOut));
	tMetrics = pData->tMetrics;
	tMetrics.iSize = sizeof(tMetrics);
	pOut->tColors = pData->tColors;
	pOut->tColors.iSize = sizeof(pOut->tColors);
	pOut->pFont = (pData->pFont != NULL) ? pData->pFont : xuiGetDefaultFont(xuiWidgetGetContext(pWidget));
	(void)__xuiStatusBarStyleFloat(pWidget, "statusbar.height", &tMetrics.fHeight);
	(void)__xuiStatusBarStyleFloat(pWidget, "statusbar.padding.x", &tMetrics.fPaddingX);
	(void)__xuiStatusBarStyleFloat(pWidget, "statusbar.padding.y", &tMetrics.fPaddingY);
	(void)__xuiStatusBarStyleFloat(pWidget, "statusbar.gap", &tMetrics.fGap);
	(void)__xuiStatusBarStyleFloat(pWidget, "statusbar.item.padding.x", &tMetrics.fItemPaddingX);
	(void)__xuiStatusBarStyleFloat(pWidget, "statusbar.item.padding.y", &tMetrics.fItemPaddingY);
	(void)__xuiStatusBarStyleFloat(pWidget, "statusbar.progress.height", &tMetrics.fProgressHeight);
	(void)__xuiStatusBarStyleFloat(pWidget, "statusbar.border.width", &tMetrics.fBorderWidth);
	(void)__xuiStatusBarStyleFloat(pWidget, "statusbar.top_border.width", &tMetrics.fTopBorderWidth);
	(void)__xuiStatusBarStyleColor(pWidget, "statusbar.background.color", &pOut->tColors.iBackgroundColor);
	(void)__xuiStatusBarStyleColor(pWidget, "statusbar.border.color", &pOut->tColors.iBorderColor);
	(void)__xuiStatusBarStyleColor(pWidget, "statusbar.highlight.color", &pOut->tColors.iHighlightColor);
	(void)__xuiStatusBarStyleColor(pWidget, "statusbar.item.color", &pOut->tColors.iItemColor);
	(void)__xuiStatusBarStyleColor(pWidget, "statusbar.item.hover_color", &pOut->tColors.iHoverColor);
	(void)__xuiStatusBarStyleColor(pWidget, "statusbar.item.active_color", &pOut->tColors.iActiveColor);
	(void)__xuiStatusBarStyleColor(pWidget, "statusbar.focus.color", &pOut->tColors.iFocusColor);
	(void)__xuiStatusBarStyleColor(pWidget, "statusbar.text.color", &pOut->tColors.iTextColor);
	pOut->iActiveTextColor = XUI_COLOR_WHITE;
	(void)__xuiStatusBarStyleColor(pWidget, "statusbar.text.active_color", &pOut->iActiveTextColor);
	(void)__xuiStatusBarStyleColor(pWidget, "statusbar.text.disabled_color", &pOut->tColors.iDisabledTextColor);
	(void)__xuiStatusBarStyleColor(pWidget, "statusbar.progress.track_color", &pOut->tColors.iProgressTrackColor);
	(void)__xuiStatusBarStyleColor(pWidget, "statusbar.progress.fill_color", &pOut->tColors.iProgressFillColor);
	pOut->pFont = __xuiStatusBarStyleFont(pWidget, pOut->pFont);
	if ( tMetrics.fHeight <= 0.0f ) tMetrics.fHeight = 1.0f;
	if ( tMetrics.fProgressHeight <= 0.0f ) tMetrics.fProgressHeight = 1.0f;
	pOut->tMetrics = __xuiStatusBarMetricsValid(&tMetrics) ? tMetrics : pData->tMetrics;
	pOut->tMetrics.iSize = sizeof(pOut->tMetrics);
}

static int __xuiStatusBarItemWidth(xui_widget pWidget, const xui_statusbar_item_t* pItem,
	const xui_statusbar_resolved_t* pResolved, double* pWidth)
{
	xui_vec2_t tSize;
	double fWidth;

	if ( pWidget == NULL || pItem == NULL || pResolved == NULL || pWidth == NULL ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( pItem->fWidth > 0.0f ) fWidth = pItem->fWidth;
	else if ( pItem->fFlex > 0.0f ) fWidth = 0.0;
	else if ( pItem->iType == XUI_STATUSBAR_ITEM_PROGRESS ) fWidth = 92.0;
	else if ( pItem->iType == XUI_STATUSBAR_ITEM_SPACER ) fWidth = 12.0;
	else {
		fWidth = 36.0;
		if ( pItem->sText != NULL && pItem->sText[0] != '\0' ) {
			tSize = __xuiStatusBarMeasureText(pWidget, pResolved->pFont, pItem->sText);
			fWidth = (double)tSize.fX + (double)pResolved->tMetrics.fItemPaddingX * 2.0;
		}
		if ( fWidth < 12.0 ) fWidth = 12.0;
	}
	if ( fWidth < 0.0 || fWidth > (double)XUI_LAYOUT_UNBOUNDED ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	*pWidth = fWidth;
	return XUI_OK;
}

static int __xuiStatusBarMeasureSection(xui_widget pWidget, const xui_statusbar_data_t* pData,
	const xui_statusbar_resolved_t* pResolved, int iSection, double* pWidth, double* pFlex)
{
	double fItemWidth;
	double fTotal;
	double fFlex;
	int iCount;
	int iRet;
	int i;

	if ( pWidget == NULL || pData == NULL || pResolved == NULL ||
	     pWidth == NULL || pFlex == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	fTotal = 0.0;
	fFlex = 0.0;
	iCount = 0;
	for ( i = 0; i < pData->iItemCount; i++ ) {
		if ( pData->arrItems[i].iSection != iSection ) continue;
		if ( iCount > 0 ) fTotal += (double)pResolved->tMetrics.fGap;
		iRet = __xuiStatusBarItemWidth(pWidget, &pData->arrItems[i], pResolved, &fItemWidth);
		if ( iRet != XUI_OK ) return iRet;
		fTotal += fItemWidth;
		if ( pData->arrItems[i].fFlex > 0.0f && pData->arrItems[i].fWidth <= 0.0f ) {
			fFlex += pData->arrItems[i].fFlex;
		}
		iCount++;
	}
	*pWidth = fTotal;
	*pFlex = fFlex;
	return XUI_OK;
}

static int __xuiStatusBarSectionCount(const xui_statusbar_data_t* pData, int iSection)
{
	int iCount;
	int i;

	if ( pData == NULL ) return 0;
	iCount = 0;
	for ( i = 0; i < pData->iItemCount; i++ ) {
		if ( pData->arrItems[i].iSection == iSection ) iCount++;
	}
	return iCount;
}

static int __xuiStatusBarRectFromDouble(double fX, double fY, double fW, double fH,
	xui_rect_t* pRect)
{
	double fRight;
	double fBottom;
	double fBound = (double)XUI_LAYOUT_UNBOUNDED;

	if ( pRect == NULL || fW < 0.0 || fH < 0.0 ||
	     fX < -fBound || fX > fBound || fY < -fBound || fY > fBound ||
	     fW > fBound || fH > fBound ) return XUI_ERROR_INVALID_ARGUMENT;
	fRight = fX + fW;
	fBottom = fY + fH;
	if ( fRight < -fBound || fRight > fBound ||
	     fBottom < -fBound || fBottom > fBound ) return XUI_ERROR_INVALID_ARGUMENT;
	*pRect = xuiInternalRectFromFloatNearest((float)fX, (float)fY, (float)fW, (float)fH);
	return XUI_OK;
}

static int __xuiStatusBarLayoutSection(xui_widget pWidget, const xui_statusbar_data_t* pData,
	const xui_statusbar_resolved_t* pResolved, xui_rect_t* arrRects, int iSection,
	double fStart, double fEnd, double fY, double fHeight)
{
	double fFixed;
	double fFlexTotal;
	double fFlexExtra;
	double fCursor;
	double fWidth;
	double fAvailable;
	double fVisibleStart;
	double fVisibleEnd;
	int iCount;
	int iRet;
	int i;

	if ( pWidget == NULL || pData == NULL || pResolved == NULL || arrRects == NULL ||
	     fEnd < fStart || fHeight < 0.0 ) return XUI_ERROR_INVALID_ARGUMENT;
	fAvailable = fEnd - fStart;
	fFixed = 0.0;
	fFlexTotal = 0.0;
	iCount = 0;
	for ( i = 0; i < pData->iItemCount; i++ ) {
		if ( pData->arrItems[i].iSection != iSection ) continue;
		if ( iCount > 0 ) fFixed += pResolved->tMetrics.fGap;
		if ( pData->arrItems[i].fFlex > 0.0f && pData->arrItems[i].fWidth <= 0.0f ) {
			fFlexTotal += pData->arrItems[i].fFlex;
		} else {
			iRet = __xuiStatusBarItemWidth(pWidget, &pData->arrItems[i], pResolved, &fWidth);
			if ( iRet != XUI_OK ) return iRet;
			fFixed += fWidth;
		}
		iCount++;
	}
	fFlexExtra = 0.0;
	if ( (fFlexTotal > 0.0f) && (fAvailable > fFixed) ) {
		fFlexExtra = fAvailable - fFixed;
	}
	fCursor = fStart;
	for ( i = 0; i < pData->iItemCount; i++ ) {
		if ( pData->arrItems[i].iSection != iSection ) continue;
		if ( pData->arrItems[i].fFlex > 0.0f && pData->arrItems[i].fWidth <= 0.0f && (fFlexTotal > 0.0f) ) {
			fWidth = fFlexExtra * (pData->arrItems[i].fFlex / fFlexTotal);
		} else {
			iRet = __xuiStatusBarItemWidth(pWidget, &pData->arrItems[i], pResolved, &fWidth);
			if ( iRet != XUI_OK ) return iRet;
		}
		fVisibleStart = fCursor > fStart ? fCursor : fStart;
		if ( fVisibleStart > fEnd ) fVisibleStart = fEnd;
		fVisibleEnd = fCursor + fWidth;
		if ( fVisibleEnd < fStart ) fVisibleEnd = fStart;
		if ( fVisibleEnd > fEnd ) fVisibleEnd = fEnd;
		if ( fVisibleEnd < fVisibleStart ) fVisibleEnd = fVisibleStart;
		iRet = __xuiStatusBarRectFromDouble(fVisibleStart, fY,
			fVisibleEnd - fVisibleStart, fHeight, &arrRects[i]);
		if ( iRet != XUI_OK ) return iRet;
		fCursor += fWidth + pResolved->tMetrics.fGap;
	}
	return XUI_OK;
}

static int __xuiStatusBarLayoutItems(xui_widget pWidget, xui_statusbar_data_t* pData, const xui_statusbar_resolved_t* pResolved)
{
	xui_rect_t arrRects[XUI_STATUSBAR_ITEM_CAPACITY];
	xui_rect_t tContent;
	double fStart;
	double fEnd;
	double fY;
	double fHeight;
	double fAvailable;
	double fLeftWidth;
	double fCenterWidth;
	double fRightWidth;
	double fLeftFlex;
	double fCenterFlex;
	double fRightFlex;
	double fFlexTotal;
	double fSectionGaps;
	double fMinimumTotal;
	double fExtra;
	double fRightStart;
	double fCenterStart;
	double fCenterEnd;
	double fCenterMaxStart;
	double fCenterMinStart;
	double fCenterIdeal;
	double fLeftEnd;
	int bHasLeft;
	int bHasCenter;
	int bHasRight;
	int iSectionCount;
	int bSelectionChanged;
	int iRet;
	int i;

	if ( (pWidget == NULL) || (pData == NULL) || (pResolved == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( !__xuiStatusBarMetricsValid(&pResolved->tMetrics) ) return XUI_ERROR_INVALID_ARGUMENT;
	tContent = xuiInternalSnapRect(xuiWidgetGetContentRect(pWidget));
	if ( tContent.fX < -(int)XUI_LAYOUT_UNBOUNDED || tContent.fX > (int)XUI_LAYOUT_UNBOUNDED ||
	     tContent.fY < -(int)XUI_LAYOUT_UNBOUNDED || tContent.fY > (int)XUI_LAYOUT_UNBOUNDED ||
	     tContent.fW > (int)XUI_LAYOUT_UNBOUNDED || tContent.fH > (int)XUI_LAYOUT_UNBOUNDED ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	memset(arrRects, 0, sizeof(arrRects));
	fStart = (double)tContent.fX + (double)pResolved->tMetrics.fPaddingX;
	fEnd = (double)tContent.fX + (double)tContent.fW - (double)pResolved->tMetrics.fPaddingX;
	if ( fEnd < fStart ) fEnd = fStart;
	fY = (double)tContent.fY + (double)pResolved->tMetrics.fPaddingY;
	fHeight = (double)tContent.fH - (double)pResolved->tMetrics.fPaddingY * 2.0;
	if ( fHeight < 0.0 ) fHeight = 0.0;
	fAvailable = fEnd - fStart;
	bHasLeft = __xuiStatusBarSectionCount(pData, XUI_STATUSBAR_SECTION_LEFT) > 0;
	bHasCenter = __xuiStatusBarSectionCount(pData, XUI_STATUSBAR_SECTION_CENTER) > 0;
	bHasRight = __xuiStatusBarSectionCount(pData, XUI_STATUSBAR_SECTION_RIGHT) > 0;
	iRet = __xuiStatusBarMeasureSection(pWidget, pData, pResolved,
		XUI_STATUSBAR_SECTION_LEFT, &fLeftWidth, &fLeftFlex);
	if ( iRet == XUI_OK ) iRet = __xuiStatusBarMeasureSection(pWidget, pData, pResolved,
		XUI_STATUSBAR_SECTION_CENTER, &fCenterWidth, &fCenterFlex);
	if ( iRet == XUI_OK ) iRet = __xuiStatusBarMeasureSection(pWidget, pData, pResolved,
		XUI_STATUSBAR_SECTION_RIGHT, &fRightWidth, &fRightFlex);
	if ( iRet != XUI_OK ) return iRet;
	iSectionCount = bHasLeft + bHasCenter + bHasRight;
	fSectionGaps = iSectionCount > 1 ?
		(double)pResolved->tMetrics.fGap * (double)(iSectionCount - 1) : 0.0;
	fMinimumTotal = fLeftWidth + fCenterWidth + fRightWidth + fSectionGaps;
	fFlexTotal = fLeftFlex + fCenterFlex + fRightFlex;
	if ( fMinimumTotal <= fAvailable && fFlexTotal > 0.0 ) {
		fExtra = fAvailable - fMinimumTotal;
		fLeftWidth += fExtra * (fLeftFlex / fFlexTotal);
		fCenterWidth += fExtra * (fCenterFlex / fFlexTotal);
		fRightWidth += fExtra * (fRightFlex / fFlexTotal);
	}
	if ( fRightWidth > fAvailable ) fRightWidth = fAvailable;
	fRightStart = fEnd - fRightWidth;
	fCenterEnd = fRightStart - ((bHasCenter && bHasRight) ? pResolved->tMetrics.fGap : 0.0f);
	if ( fCenterEnd < fStart ) fCenterEnd = fStart;
	if ( fCenterWidth > fCenterEnd - fStart ) fCenterWidth = fCenterEnd - fStart;
	if ( fCenterWidth < 0.0f ) fCenterWidth = 0.0f;
	fCenterMaxStart = fCenterEnd - fCenterWidth;
	fCenterMinStart = fStart + fLeftWidth +
		((bHasLeft && bHasCenter) ? pResolved->tMetrics.fGap : 0.0f);
	if ( fCenterMinStart > fCenterMaxStart ) fCenterMinStart = fCenterMaxStart;
	fCenterIdeal = (fStart + fEnd - fCenterWidth) * 0.5f;
	fCenterStart = fCenterIdeal;
	if ( fCenterStart < fCenterMinStart ) fCenterStart = fCenterMinStart;
	if ( fCenterStart > fCenterMaxStart ) fCenterStart = fCenterMaxStart;
	if ( fCenterStart < fStart ) fCenterStart = fStart;
	fLeftEnd = fStart + fLeftWidth;
	if ( fMinimumTotal > fAvailable ) {
		fLeftEnd = bHasCenter ? fCenterStart - (bHasLeft ? pResolved->tMetrics.fGap : 0.0f) :
			fRightStart - ((bHasLeft && bHasRight) ? pResolved->tMetrics.fGap : 0.0f);
	}
	if ( fLeftEnd < fStart ) fLeftEnd = fStart;
	if ( fLeftEnd > fEnd ) fLeftEnd = fEnd;
	if ( bHasLeft ) iRet = __xuiStatusBarLayoutSection(pWidget, pData, pResolved, arrRects,
		XUI_STATUSBAR_SECTION_LEFT, fStart, fLeftEnd, fY, fHeight);
	if ( iRet == XUI_OK && bHasCenter ) iRet = __xuiStatusBarLayoutSection(pWidget, pData,
		pResolved, arrRects, XUI_STATUSBAR_SECTION_CENTER, fCenterStart,
		fCenterStart + fCenterWidth, fY, fHeight);
	if ( iRet == XUI_OK && bHasRight ) iRet = __xuiStatusBarLayoutSection(pWidget, pData,
		pResolved, arrRects, XUI_STATUSBAR_SECTION_RIGHT, fRightStart, fEnd, fY, fHeight);
	if ( iRet != XUI_OK ) return iRet;
	bSelectionChanged = 0;
	if ( pData->iHover >= 0 &&
	     (pData->iHover >= pData->iItemCount || arrRects[pData->iHover].fW <= 0 ||
	      arrRects[pData->iHover].fH <= 0) ) {
		pData->iHover = -1;
		bSelectionChanged = 1;
	}
	for ( i = 0; i < pData->iItemCount; i++ ) pData->arrItems[i].tRect = arrRects[i];
	if ( bSelectionChanged ) {
		xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_SELECTION_CHANGED);
	}
	return XUI_OK;
}

static int __xuiStatusBarIndexAt(xui_statusbar_data_t* pData, float fX, float fY)
{
	int i;

	if ( pData == NULL ) return -1;
	for ( i = 0; i < pData->iItemCount; i++ ) {
		if ( __xuiStatusBarItemInteractive(&pData->arrItems[i]) &&
		     __xuiStatusBarRectContains(pData->arrItems[i].tRect, fX, fY) ) {
			return i;
		}
	}
	return -1;
}

static int __xuiStatusBarVisualIndexAt(xui_statusbar_data_t* pData, float fX, float fY)
{
	int i;

	if ( pData == NULL ) return -1;
	for ( i = 0; i < pData->iItemCount; i++ ) {
		if ( pData->arrItems[i].iType != XUI_STATUSBAR_ITEM_SPACER &&
		     pData->arrItems[i].tRect.fW > 0 && pData->arrItems[i].tRect.fH > 0 &&
		     __xuiStatusBarRectContains(pData->arrItems[i].tRect, fX, fY) ) {
			return i;
		}
	}
	return -1;
}

static int __xuiStatusBarFirstVisibleItem(xui_statusbar_data_t* pData)
{
	int i;

	if ( pData == NULL ) return -1;
	for ( i = 0; i < pData->iItemCount; i++ ) {
		if ( pData->arrItems[i].iType != XUI_STATUSBAR_ITEM_SPACER &&
		     pData->arrItems[i].tRect.fW > 0 && pData->arrItems[i].tRect.fH > 0 ) {
			return i;
		}
	}
	return -1;
}

static int __xuiStatusBarFirstInteractive(xui_statusbar_data_t* pData)
{
	int i;

	if ( pData == NULL ) return -1;
	for ( i = 0; i < pData->iItemCount; i++ ) {
		if ( __xuiStatusBarItemInteractive(&pData->arrItems[i]) &&
		     pData->arrItems[i].tRect.fW > 0.0f && pData->arrItems[i].tRect.fH > 0.0f ) return i;
	}
	return -1;
}

static int __xuiStatusBarLastInteractive(xui_statusbar_data_t* pData)
{
	int i;

	if ( pData == NULL ) return -1;
	for ( i = pData->iItemCount - 1; i >= 0; i-- ) {
		if ( __xuiStatusBarItemInteractive(&pData->arrItems[i]) &&
		     pData->arrItems[i].tRect.fW > 0.0f && pData->arrItems[i].tRect.fH > 0.0f ) return i;
	}
	return -1;
}

static int __xuiStatusBarNextInteractive(xui_statusbar_data_t* pData, int iCurrent, int iStep)
{
	int iIndex;
	int i;

	if ( pData == NULL || pData->iItemCount <= 0 || iStep == 0 ) return -1;
	iIndex = iCurrent;
	for ( i = 0; i < pData->iItemCount; i++ ) {
		iIndex += iStep;
		if ( iIndex < 0 ) iIndex = pData->iItemCount - 1;
		else if ( iIndex >= pData->iItemCount ) iIndex = 0;
		if ( __xuiStatusBarItemInteractive(&pData->arrItems[iIndex]) &&
		     pData->arrItems[iIndex].tRect.fW > 0.0f && pData->arrItems[iIndex].tRect.fH > 0.0f ) return iIndex;
	}
	return -1;
}

static int __xuiStatusBarInvalidateLocalRect(xui_widget pWidget, xui_rect_t tRect)
{
	xui_rect_t tWidget;
	int64_t iContentW;
	int64_t iContentH;
	int64_t iLeft;
	int64_t iTop;
	int64_t iRight;
	int64_t iBottom;

	if ( pWidget == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( tRect.fW <= 0 || tRect.fH <= 0 ) return XUI_OK;
	tWidget = xuiWidgetGetRect(pWidget);
	iContentW = tWidget.fW > 0 ? (int64_t)tWidget.fW : 0;
	iContentH = tWidget.fH > 0 ? (int64_t)tWidget.fH : 0;
	iLeft = (int64_t)tRect.fX - 2;
	iTop = (int64_t)tRect.fY - 2;
	iRight = (int64_t)tRect.fX + (int64_t)tRect.fW + 2;
	iBottom = (int64_t)tRect.fY + (int64_t)tRect.fH + 2;
	if ( iLeft < 0 ) iLeft = 0;
	if ( iTop < 0 ) iTop = 0;
	if ( iRight > iContentW ) iRight = iContentW;
	if ( iBottom > iContentH ) iBottom = iContentH;
	tRect = (xui_rect_t){
		(int)iLeft,
		(int)iTop,
		(int)(iRight > iLeft ? iRight - iLeft : 0),
		(int)(iBottom > iTop ? iBottom - iTop : 0)
	};
	if ( tRect.fW <= 0 || tRect.fH <= 0 ) return XUI_OK;
	return xuiWidgetInvalidateRect(pWidget, tRect,
		XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
}

static int __xuiStatusBarInvalidateItem(xui_widget pWidget, xui_statusbar_data_t* pData, int iIndex)
{

	if ( (pWidget == NULL) || (pData == NULL) || (iIndex < 0) || (iIndex >= pData->iItemCount) ) {
		return XUI_OK;
	}
	return __xuiStatusBarInvalidateLocalRect(pWidget, pData->arrItems[iIndex].tRect);
}

static int __xuiStatusBarSetHover(xui_widget pWidget, xui_statusbar_data_t* pData, int iIndex)
{
	int iOldHover;
	int iRet;

	if ( (pWidget == NULL) || (pData == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( (iIndex < 0) || (iIndex >= pData->iItemCount) ||
	     !__xuiStatusBarItemInteractive(&pData->arrItems[iIndex]) ||
	     pData->arrItems[iIndex].tRect.fW <= 0 || pData->arrItems[iIndex].tRect.fH <= 0 ) {
		iIndex = -1;
	}
	if ( pData->iHover == iIndex ) return XUI_OK;
	iOldHover = pData->iHover;
	pData->iHover = iIndex;
	iRet = __xuiStatusBarInvalidateItem(pWidget, pData, iOldHover);
	if ( iRet == XUI_OK ) {
		iRet = __xuiStatusBarInvalidateItem(pWidget, pData, iIndex);
	}
	if ( iRet != XUI_OK ) {
		pData->iHover = iOldHover;
		(void)__xuiStatusBarInvalidateItem(pWidget, pData, iOldHover);
		(void)__xuiStatusBarInvalidateItem(pWidget, pData, iIndex);
		return iRet;
	}
	xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_SELECTION_CHANGED);
	return XUI_OK;
}

static int __xuiStatusBarCancelPress(xui_widget pWidget, xui_statusbar_data_t* pData, int bClearHover)
{
	xui_context pContext;
	int bChanged;
	int iRet;

	if ( (pWidget == NULL) || (pData == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	pContext = xuiWidgetGetContext(pWidget);
	bChanged = (pData->iActive >= 0) || pData->tPointerOwner.bActive ||
		(bClearHover && (pData->iHover >= 0));
	pData->iActive = -1;
	if ( bClearHover ) pData->iHover = -1;
	iRet = xuiInternalPointerOwnerCancel(pContext, pWidget, &pData->tPointerOwner);
	if ( iRet != XUI_OK ) return iRet;
	if ( __xuiStatusBarGetData(pWidget) != pData ) return XUI_OK;
	return bChanged ? xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER) : XUI_OK;
}

static int __xuiStatusBarSelect(xui_widget pWidget, xui_statusbar_data_t* pData, int iIndex)
{
	xui_statusbar_select_proc onSelect;
	void* pSelectUser;
	int iValue;
	int iOldSelectCount;
	int iRet;

	if ( (pWidget == NULL) || (pData == NULL) ||
	     (iIndex < 0) || (iIndex >= pData->iItemCount) ||
	     !__xuiStatusBarItemInteractive(&pData->arrItems[iIndex]) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	iOldSelectCount = pData->iSelectCount;
	__xuiStatusBarIncrementCount(&pData->iSelectCount);
	iRet = xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	if ( iRet != XUI_OK ) {
		pData->iSelectCount = iOldSelectCount;
		return iRet;
	}
	xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_SELECTION_CHANGED);
	onSelect = pData->onSelect;
	pSelectUser = pData->pSelectUser;
	iValue = pData->arrItems[iIndex].iValue;
	if ( onSelect != NULL ) onSelect(pWidget, iIndex, iValue, pSelectUser);
	return XUI_OK;
}

static int __xuiStatusBarDrawRectFill(xui_proxy pProxy, xui_draw_context pDraw, xui_rect_t tRect, uint32_t iColor)
{
	if ( __xuiStatusBarAlpha(iColor) == 0 ) return XUI_OK;
	return pProxy->drawRectFill(pProxy, pDraw, tRect, iColor);
}

static int __xuiStatusBarDrawRectStroke(xui_proxy pProxy, xui_draw_context pDraw, xui_rect_t tRect, float fWidth, uint32_t iColor)
{
	if ( (fWidth <= 0.0f) || (__xuiStatusBarAlpha(iColor) == 0) ) return XUI_OK;
	return pProxy->drawRectStroke(pProxy, pDraw, tRect, fWidth, iColor);
}

static int __xuiStatusBarDrawProgress(xui_proxy pProxy, xui_draw_context pDraw, xui_rect_t tRect, const xui_statusbar_item_t* pItem, const xui_statusbar_resolved_t* pResolved)
{
	xui_rect_t tTrack;
	xui_rect_t tFill;
	float fRatio;
	double fRange;
	double fValue;
	float fProgressHeight;
	int iRet;

	tTrack = __xuiStatusBarInsetRect(tRect, pResolved->tMetrics.fItemPaddingX, 0.0f, pResolved->tMetrics.fItemPaddingX, 0.0f);
	fProgressHeight = pResolved->tMetrics.fProgressHeight;
	if ( fProgressHeight > tTrack.fH ) fProgressHeight = (float)tTrack.fH;
	tTrack.fY += (tTrack.fH - fProgressHeight) * 0.5f;
	tTrack.fH = fProgressHeight;
	if ( tTrack.fW <= 0.0f || tTrack.fH <= 0.0f ) return XUI_OK;
	iRet = __xuiStatusBarDrawRectFill(pProxy, pDraw, xuiInternalSnapRect(tTrack), pResolved->tColors.iProgressTrackColor);
	if ( iRet != XUI_OK ) return iRet;
	fRange = (double)pItem->fMax - (double)pItem->fMin;
	fValue = (double)pItem->fValue - (double)pItem->fMin;
	fRatio = (fRange > 0.0) ? (float)(fValue / fRange) : 0.0f;
	if ( fRatio < 0.0f ) fRatio = 0.0f;
	if ( fRatio > 1.0f ) fRatio = 1.0f;
	tFill = __xuiStatusBarInsetRect(tTrack, 1.0f, 1.0f, 1.0f, 1.0f);
	tFill.fW *= fRatio;
	if ( tFill.fW > 0.0f ) {
		iRet = __xuiStatusBarDrawRectFill(pProxy, pDraw, xuiInternalSnapRect(tFill), pResolved->tColors.iProgressFillColor);
		if ( iRet != XUI_OK ) return iRet;
	}
	return __xuiStatusBarDrawRectStroke(pProxy, pDraw, xuiInternalSnapRect(tTrack), pResolved->tMetrics.fBorderWidth, __xuiStatusBarColorAlpha(pResolved->tColors.iBorderColor, 130));
}

static int __xuiStatusBarCacheRender(xui_widget pWidget, xui_draw_context pDraw, uint32_t iStateId, void* pUser)
{
	xui_statusbar_data_t* pData;
	xui_statusbar_resolved_t tResolved;
	xui_proxy pProxy;
	xui_rect_t tRect;
	xui_rect_t tLine;
	xui_rect_t tCell;
	xui_rect_t tText;
	uint32_t iFill;
	uint32_t iBorder;
	uint32_t iText;
	int bPressed;
	int iRet;
	int i;

	(void)iStateId;
	(void)pUser;
	pData = __xuiStatusBarGetData(pWidget);
	if ( (pData == NULL) || (pDraw == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	__xuiStatusBarResolve(pWidget, pData, &tResolved);
	iRet = __xuiStatusBarLayoutItems(pWidget, pData, &tResolved);
	if ( iRet != XUI_OK ) return iRet;
	pProxy = xuiInternalContextGetProxy(xuiWidgetGetContext(pWidget));
	if ( pProxy == NULL ) return XUI_ERROR_NOT_INITIALIZED;
	tRect = xuiWidgetGetRect(pWidget);
	tRect.fX = 0.0f;
	tRect.fY = 0.0f;
	iRet = __xuiStatusBarDrawRectFill(pProxy, pDraw, xuiInternalSnapRect(tRect), tResolved.tColors.iBackgroundColor);
	if ( iRet != XUI_OK ) return iRet;
	if ( tResolved.tMetrics.fTopBorderWidth > 0.0f ) {
		tLine = (xui_rect_t){0.0f, 0.0f, tRect.fW, tResolved.tMetrics.fTopBorderWidth};
		if ( tLine.fH > tRect.fH ) tLine.fH = tRect.fH;
		iRet = __xuiStatusBarDrawRectFill(pProxy, pDraw, xuiInternalSnapRect(tLine), tResolved.tColors.iBorderColor);
		if ( iRet != XUI_OK ) return iRet;
		tLine.fY += tResolved.tMetrics.fTopBorderWidth;
		tLine.fH = tLine.fY < tRect.fH ? 1.0f : 0.0f;
		if ( tLine.fH > 0.0f ) {
			iRet = __xuiStatusBarDrawRectFill(pProxy, pDraw, xuiInternalSnapRect(tLine), tResolved.tColors.iHighlightColor);
			if ( iRet != XUI_OK ) return iRet;
		}
	}
	for ( i = 0; i < pData->iItemCount; i++ ) {
		if ( (pData->arrItems[i].tRect.fW <= 0.0f) || (pData->arrItems[i].tRect.fH <= 0.0f) ||
		     (pData->arrItems[i].iType == XUI_STATUSBAR_ITEM_SPACER) ) {
			continue;
		}
		iFill = tResolved.tColors.iItemColor;
		iBorder = XUI_COLOR_RGBA(0, 0, 0, 0);
		iText = ((pData->arrItems[i].iState & XUI_STATUSBAR_ITEM_ENABLED) != 0u) ?
			tResolved.tColors.iTextColor : tResolved.tColors.iDisabledTextColor;
		bPressed = 0;
		if ( i == pData->iActive && i == pData->iHover ) {
			iFill = tResolved.tColors.iActiveColor;
			iBorder = __xuiStatusBarColorAlpha(tResolved.tColors.iActiveColor, 235);
			iText = tResolved.iActiveTextColor;
			bPressed = 1;
		} else if ( i == pData->iHover ) {
			iFill = tResolved.tColors.iHoverColor;
			iBorder = __xuiStatusBarColorAlpha(tResolved.tColors.iFocusColor, 160);
		}
		if ( (pData->arrItems[i].iState & XUI_STATUSBAR_ITEM_CLICKABLE) != 0u ) {
			tCell = __xuiStatusBarInsetRect(pData->arrItems[i].tRect, 1.0f, tResolved.tMetrics.fItemPaddingY, 1.0f, tResolved.tMetrics.fItemPaddingY);
			if ( bPressed ) tCell.fY += 1.0f;
			if ( (tCell.fW > 0.0f) && (tCell.fH > 0.0f) ) {
				iRet = __xuiStatusBarDrawRectFill(pProxy, pDraw, xuiInternalSnapRect(tCell), iFill);
				if ( iRet != XUI_OK ) return iRet;
				iRet = __xuiStatusBarDrawRectStroke(pProxy, pDraw, xuiInternalSnapRect(tCell), tResolved.tMetrics.fBorderWidth, iBorder);
				if ( iRet != XUI_OK ) return iRet;
			}
		}
		if ( pData->arrItems[i].iType == XUI_STATUSBAR_ITEM_PROGRESS ) {
			iRet = __xuiStatusBarDrawProgress(pProxy, pDraw, pData->arrItems[i].tRect, &pData->arrItems[i], &tResolved);
			if ( iRet != XUI_OK ) return iRet;
		} else if ( (pProxy->drawText != NULL) && (tResolved.pFont != NULL) &&
		            (pData->arrItems[i].sText != NULL) && (pData->arrItems[i].sText[0] != '\0') ) {
			tText = __xuiStatusBarInsetRect(pData->arrItems[i].tRect, tResolved.tMetrics.fItemPaddingX, 0.0f, tResolved.tMetrics.fItemPaddingX, 0.0f);
			if ( bPressed ) tText.fY += 1.0f;
			if ( tText.fW > 0.0f ) {
				iRet = pProxy->drawText(pProxy, pDraw, &(xui_text_item_t){.iSize=sizeof(xui_text_item_t), .pFont=tResolved.pFont, .sText=pData->arrItems[i].sText, .iTextSize=-1, .iFlags=XUI_TEXT_SHAPE_DEFAULT | ((XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP) & XUI_TEXT_RTL ? XUI_TEXT_SHAPE_RTL : 0)}, tText, iText, XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP);
				if ( iRet != XUI_OK ) return iRet;
			}
		}
	}
	return XUI_OK;
}

static int __xuiStatusBarContentMeasure(xui_widget pWidget, xui_vec2_t tConstraint, xui_vec2_t* pSize, void* pUser)
{
	xui_statusbar_data_t* pData;
	xui_statusbar_resolved_t tResolved;
	double fLeft;
	double fCenter;
	double fRight;
	double fUnusedFlex;
	double fWidth;
	int iSectionCount;
	int iRet;

	(void)tConstraint;
	(void)pUser;
	if ( (pWidget == NULL) || (pSize == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	pData = __xuiStatusBarGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	__xuiStatusBarResolve(pWidget, pData, &tResolved);
	iRet = __xuiStatusBarMeasureSection(pWidget, pData, &tResolved,
		XUI_STATUSBAR_SECTION_LEFT, &fLeft, &fUnusedFlex);
	if ( iRet == XUI_OK ) iRet = __xuiStatusBarMeasureSection(pWidget, pData, &tResolved,
		XUI_STATUSBAR_SECTION_CENTER, &fCenter, &fUnusedFlex);
	if ( iRet == XUI_OK ) iRet = __xuiStatusBarMeasureSection(pWidget, pData, &tResolved,
		XUI_STATUSBAR_SECTION_RIGHT, &fRight, &fUnusedFlex);
	if ( iRet != XUI_OK ) return iRet;
	iSectionCount = (__xuiStatusBarSectionCount(pData, XUI_STATUSBAR_SECTION_LEFT) > 0) +
		(__xuiStatusBarSectionCount(pData, XUI_STATUSBAR_SECTION_CENTER) > 0) +
		(__xuiStatusBarSectionCount(pData, XUI_STATUSBAR_SECTION_RIGHT) > 0);
	fWidth = (double)tResolved.tMetrics.fPaddingX * 2.0 + fLeft + fCenter + fRight;
	if ( iSectionCount > 1 ) {
		fWidth += (double)tResolved.tMetrics.fGap * (double)(iSectionCount - 1);
	}
	if ( fWidth > (double)XUI_LAYOUT_UNBOUNDED ) fWidth = (double)XUI_LAYOUT_UNBOUNDED;
	pSize->fX = xuiInternalSnapSize((float)fWidth);
	pSize->fY = xuiInternalSnapSize(tResolved.tMetrics.fHeight);
	return XUI_OK;
}

static int __xuiStatusBarEvent(xui_widget pWidget, const xui_event_t* pEvent, void* pUser)
{
	xui_statusbar_data_t* pData;
	xui_statusbar_resolved_t tResolved;
	xui_context pContext;
	xui_rect_t tWorld;
	float fLocalX;
	float fLocalY;
	int iIndex;
	int iNext;
	int iRet;
	int bLeftButton;

	(void)pUser;
	if ( (pWidget == NULL) || (pEvent == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pEvent->iPhase == XUI_EVENT_PHASE_CAPTURE ) return XUI_OK;
	pData = __xuiStatusBarGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	__xuiStatusBarResolve(pWidget, pData, &tResolved);
	iRet = __xuiStatusBarLayoutItems(pWidget, pData, &tResolved);
	if ( iRet != XUI_OK ) return iRet;
	pContext = xuiWidgetGetContext(pWidget);
	tWorld = xuiWidgetGetWorldRect(pWidget);
	fLocalX = pEvent->fX - tWorld.fX;
	fLocalY = pEvent->fY - tWorld.fY;
	iIndex = __xuiStatusBarIndexAt(pData, fLocalX, fLocalY);
	bLeftButton = (pEvent->iButton == 0) || (pEvent->iButton == XUI_POINTER_BUTTON_LEFT);
	switch ( pEvent->iType ) {
	case XUI_EVENT_POINTER_MOVE:
		if ( pData->tPointerOwner.bActive &&
		     !xuiInternalPointerOwnerMatches(&pData->tPointerOwner, pEvent) ) {
			return XUI_OK;
		}
		if ( pData->iActive >= 0 ) {
			iIndex = iIndex == pData->iActive ? pData->iActive : -1;
		}
		iRet = __xuiStatusBarSetHover(pWidget, pData, iIndex);
		if ( iRet != XUI_OK ) return iRet;
		return (pData->iActive >= 0) ? XUI_EVENT_DISPATCH_STOP : XUI_OK;
	case XUI_EVENT_POINTER_LEAVE:
		if ( pData->tPointerOwner.bActive &&
		     !xuiInternalPointerOwnerMatches(&pData->tPointerOwner, pEvent) ) {
			return XUI_OK;
		}
		iRet = __xuiStatusBarSetHover(pWidget, pData, -1);
		if ( iRet != XUI_OK ) return iRet;
		return pData->iActive >= 0 ? XUI_EVENT_DISPATCH_STOP : XUI_OK;
	case XUI_EVENT_POINTER_DOWN:
		if ( !xuiWidgetGetEnabled(pWidget) || !bLeftButton || (iIndex < 0) ) return XUI_OK;
		if ( pData->tPointerOwner.bActive ) {
			return xuiInternalPointerOwnerMatches(&pData->tPointerOwner, pEvent) ?
				XUI_EVENT_DISPATCH_STOP : XUI_OK;
		}
		iRet = xuiSetFocusWidget(pContext, pWidget);
		if ( iRet != XUI_OK ) return iRet;
		if ( __xuiStatusBarGetData(pWidget) != pData ) return XUI_EVENT_DISPATCH_STOP;
		iRet = __xuiStatusBarSetHover(pWidget, pData, iIndex);
		if ( iRet != XUI_OK ) return iRet;
		iRet = xuiInternalPointerOwnerBegin(pContext, pWidget, pEvent,
			&pData->tPointerOwner);
		if ( iRet != XUI_OK ) return iRet;
		pData->iActive = iIndex;
		iRet = xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
		if ( iRet != XUI_OK ) {
			(void)__xuiStatusBarCancelPress(pWidget, pData, 0);
			return iRet;
		}
		return XUI_EVENT_DISPATCH_STOP;
	case XUI_EVENT_POINTER_UP:
		if ( !xuiInternalPointerOwnerMatches(&pData->tPointerOwner, pEvent) ) {
			return XUI_OK;
		}
		{
			int iPressed = pData->iActive;
			pData->iActive = -1;
			iRet = xuiInternalPointerOwnerEnd(pContext, pWidget, pEvent,
				&pData->tPointerOwner);
			if ( iRet != XUI_OK ) return iRet;
			if ( __xuiStatusBarGetData(pWidget) != pData ) return XUI_EVENT_DISPATCH_STOP;
			iRet = __xuiStatusBarSetHover(pWidget, pData, iIndex);
			if ( iRet != XUI_OK ) return iRet;
			iRet = xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
			if ( iRet != XUI_OK ) return iRet;
			if ( iIndex == iPressed ) {
				iRet = __xuiStatusBarSelect(pWidget, pData, iIndex);
				if ( iRet != XUI_OK ) return iRet;
				return XUI_EVENT_DISPATCH_STOP;
			}
		}
		return XUI_EVENT_DISPATCH_STOP;
	case XUI_EVENT_POINTER_CAPTURE_LOST:
		if ( !xuiInternalPointerOwnerCaptureLost(&pData->tPointerOwner, pEvent) ) {
			return XUI_OK;
		}
		pData->iActive = -1;
		return xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	case XUI_EVENT_CONTEXT_MENU: {
		xui_statusbar_context_proc onContext;
		void* pContextUser;
		xui_rect_t tAnchor = tWorld;
		float fX;
		float fY;
		int iContextIndex;
		int iValue;
		if ( pData->onContext == NULL ) break;
		if ( pEvent->iKey == XUI_KEY_CONTEXT_MENU ) {
			iContextIndex = (pData->iHover >= 0) ? pData->iHover :
				__xuiStatusBarFirstVisibleItem(pData);
			if ( iContextIndex >= 0 ) {
				tAnchor = pData->arrItems[iContextIndex].tRect;
				tAnchor.fX += tWorld.fX;
				tAnchor.fY += tWorld.fY;
			}
		} else {
			iContextIndex = __xuiStatusBarVisualIndexAt(pData, fLocalX, fLocalY);
			if ( iContextIndex >= 0 &&
			     __xuiStatusBarItemInteractive(&pData->arrItems[iContextIndex]) ) {
				iRet = __xuiStatusBarSetHover(pWidget, pData, iContextIndex);
				if ( iRet != XUI_OK ) return iRet;
			}
		}
		xuiInternalContextMenuPoint(pEvent, tAnchor, &fX, &fY);
		onContext = pData->onContext;
		pContextUser = pData->pContextUser;
		iValue = iContextIndex >= 0 ? pData->arrItems[iContextIndex].iValue : 0;
		return onContext(pWidget, iContextIndex, iValue, fX, fY, pContextUser);
	}
	case XUI_EVENT_KEY_DOWN:
		if ( xuiGetFocusWidget(pContext) != pWidget ) return XUI_OK;
		if ( pEvent->iKey == XUI_KEY_LEFT || pEvent->iKey == XUI_KEY_RIGHT ) {
			iNext = __xuiStatusBarNextInteractive(pData, pData->iHover,
				pEvent->iKey == XUI_KEY_RIGHT ? 1 : -1);
			if ( iNext >= 0 ) {
				iRet = __xuiStatusBarSetHover(pWidget, pData, iNext);
				if ( iRet != XUI_OK ) return iRet;
			}
			return XUI_EVENT_DISPATCH_STOP;
		}
		if ( pEvent->iKey == XUI_KEY_HOME || pEvent->iKey == XUI_KEY_END ) {
			iNext = pEvent->iKey == XUI_KEY_HOME ?
				__xuiStatusBarFirstInteractive(pData) : __xuiStatusBarLastInteractive(pData);
			if ( iNext >= 0 ) {
				iRet = __xuiStatusBarSetHover(pWidget, pData, iNext);
				if ( iRet != XUI_OK ) return iRet;
			}
			return XUI_EVENT_DISPATCH_STOP;
		}
		if ( (pEvent->iKey == XUI_KEY_ENTER) || (pEvent->iKey == XUI_KEY_SPACE) ) {
			iIndex = ((pData->iHover >= 0) && (pData->iHover < pData->iItemCount) && __xuiStatusBarItemInteractive(&pData->arrItems[pData->iHover])) ?
				pData->iHover : __xuiStatusBarFirstInteractive(pData);
			if ( iIndex >= 0 ) {
				iRet = __xuiStatusBarSetHover(pWidget, pData, iIndex);
				if ( iRet != XUI_OK ) return iRet;
				iRet = __xuiStatusBarSelect(pWidget, pData, iIndex);
				if ( iRet != XUI_OK ) return iRet;
				return XUI_EVENT_DISPATCH_STOP;
			}
		}
		break;
	case XUI_EVENT_BLUR:
		iRet = __xuiStatusBarCancelPress(pWidget, pData, 0);
		if ( iRet == XUI_OK ) xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_STATE_CHANGED);
		return iRet;
	case XUI_EVENT_ENABLED_CHANGED:
	case XUI_EVENT_VISIBLE_CHANGED:
		if ( (pEvent->iType == XUI_EVENT_ENABLED_CHANGED && !xuiWidgetGetEnabled(pWidget)) ||
		     (pEvent->iType == XUI_EVENT_VISIBLE_CHANGED && !xuiWidgetGetVisible(pWidget)) ) {
			return __xuiStatusBarCancelPress(pWidget, pData, 1);
		}
		return XUI_OK;
	case XUI_EVENT_FOCUS:
		xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_STATE_CHANGED);
		return xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	default:
		break;
	}
	return XUI_OK;
}

static void __xuiStatusBarDefaultLayout(xui_layout_t* pLayout)
{
	memset(pLayout, 0, sizeof(*pLayout));
	pLayout->iLayoutType = XUI_LAYOUT_MANUAL;
	pLayout->iWidthMode = XUI_SIZE_FILL;
	pLayout->iHeightMode = XUI_SIZE_CONTENT;
	pLayout->iFlowMode = XUI_FLOW_BLOCK;
	pLayout->iDock = XUI_DOCK_BOTTOM;
	pLayout->iOverflow = XUI_OVERFLOW_VISIBLE;
	pLayout->iAlignX = XUI_ALIGN_STRETCH;
	pLayout->iAlignY = XUI_ALIGN_END;
	pLayout->iTableRowSpan = 1;
	pLayout->iTableColumnSpan = 1;
	pLayout->iGridColumnCount = 1;
	pLayout->fMaxWidth = XUI_LAYOUT_UNBOUNDED;
	pLayout->fMaxHeight = XUI_LAYOUT_UNBOUNDED;
	pLayout->fShrink = 1.0f;
}

static void __xuiStatusBarDefaultCachePolicy(xui_cache_policy_t* pPolicy)
{
	memset(pPolicy, 0, sizeof(*pPolicy));
	pPolicy->iSize = sizeof(*pPolicy);
	pPolicy->iPolicy = XUI_CACHE_POLICY_SELF;
	pPolicy->iFlags = XUI_CACHE_CLEAR_ON_UPDATE | XUI_CACHE_UPDATE_ALL_STATES;
	pPolicy->iClearColor = XUI_COLOR_RGBA(0, 0, 0, 0);
}

static int __xuiStatusBarInitEvents(xui_widget pWidget)
{
	int iRet;

	iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_POINTER_MOVE, __xuiStatusBarEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_POINTER_LEAVE, __xuiStatusBarEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_POINTER_DOWN, __xuiStatusBarEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_POINTER_UP, __xuiStatusBarEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_POINTER_CAPTURE_LOST, __xuiStatusBarEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_KEY_DOWN, __xuiStatusBarEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_FOCUS, __xuiStatusBarEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_BLUR, __xuiStatusBarEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_ENABLED_CHANGED, __xuiStatusBarEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_VISIBLE_CHANGED, __xuiStatusBarEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_CONTEXT_MENU, __xuiStatusBarEvent, NULL);
	return iRet;
}

static int __xuiStatusBarAccessibleItemCount(const xui_statusbar_data_t* pData)
{
	int iCount;
	int i;

	if ( pData == NULL ) return 0;
	iCount = 0;
	for ( i = 0; i < pData->iItemCount; i++ ) {
		if ( pData->arrItems[i].iType != XUI_STATUSBAR_ITEM_SPACER ) iCount++;
	}
	return iCount;
}

static int __xuiStatusBarAccessibleItemAt(const xui_statusbar_data_t* pData, int iOrdinal)
{
	int iCurrent;
	int i;

	if ( pData == NULL || iOrdinal < 0 ) return -1;
	iCurrent = 0;
	for ( i = 0; i < pData->iItemCount; i++ ) {
		if ( pData->arrItems[i].iType == XUI_STATUSBAR_ITEM_SPACER ) continue;
		if ( iCurrent == iOrdinal ) return i;
		iCurrent++;
	}
	return -1;
}

static int __xuiStatusBarAccessibleCount(xui_widget pWidget, void* pUser)
{
	(void)pWidget;
	return 1 + __xuiStatusBarAccessibleItemCount((const xui_statusbar_data_t*)pUser);
}

static int __xuiStatusBarAccessibleGet(xui_widget pWidget, int iIndex,
	xui_accessible_node_t* pNode, void* pUser)
{
	xui_statusbar_data_t* pData = (xui_statusbar_data_t*)pUser;
	xui_statusbar_resolved_t tResolved;
	xui_statusbar_item_t* pItem;
	xui_rect_t tWorld;
	xui_rect_t tItem;
	int iItem;
	int iRet;

	if ( pData == NULL || pNode == NULL || iIndex < 0 ) return XUI_ERROR_INVALID_ARGUMENT;
	__xuiStatusBarResolve(pWidget, pData, &tResolved);
	iRet = __xuiStatusBarLayoutItems(pWidget, pData, &tResolved);
	if ( iRet != XUI_OK ) return iRet;
	tWorld = xuiWidgetGetWorldRect(pWidget);
	if ( iIndex == 0 ) {
		pNode->iId = 1;
		pNode->iRole = XUI_ACCESSIBLE_ROLE_GROUP;
		pNode->sName = xuiWidgetGetAccessibleName(pWidget) != NULL ?
			xuiWidgetGetAccessibleName(pWidget) : "Status bar";
		pNode->sDescription = xuiWidgetGetAccessibleDescription(pWidget);
		pNode->tBounds = tWorld;
		pNode->iState |= XUI_ACCESSIBLE_STATE_FOCUSABLE;
		if ( xuiGetFocusWidget(xuiWidgetGetContext(pWidget)) == pWidget ) {
			pNode->iState |= XUI_ACCESSIBLE_STATE_FOCUSED;
		}
		pNode->iRowCount = 1;
		pNode->iColumnCount = __xuiStatusBarAccessibleItemCount(pData);
		pNode->iActions = XUI_ACCESSIBLE_ACTION_MASK(XUI_ACCESSIBLE_ACTION_FOCUS);
		return XUI_OK;
	}
	iItem = __xuiStatusBarAccessibleItemAt(pData, iIndex - 1);
	if ( iItem < 0 ) return XUI_ERROR_INVALID_ARGUMENT;
	pItem = &pData->arrItems[iItem];
	pNode->iId = (uint64_t)iItem + 2u;
	pNode->iParentId = 1;
	pNode->iRole = pItem->iType == XUI_STATUSBAR_ITEM_PROGRESS ? XUI_ACCESSIBLE_ROLE_PROGRESS :
		((pItem->iState & XUI_STATUSBAR_ITEM_CLICKABLE) != 0u ?
		 XUI_ACCESSIBLE_ROLE_BUTTON : XUI_ACCESSIBLE_ROLE_TEXT);
	pNode->sName = (pItem->sText != NULL && pItem->sText[0] != '\0') ? pItem->sText :
		(pItem->iType == XUI_STATUSBAR_ITEM_PROGRESS ? "Progress" : "Status");
	tItem = pItem->tRect;
	pNode->tBounds = (xui_rect_t){tWorld.fX + tItem.fX, tWorld.fY + tItem.fY, tItem.fW, tItem.fH};
	pNode->iRow = 0;
	pNode->iColumn = iIndex - 1;
	pNode->iRowCount = 1;
	pNode->iColumnCount = 1;
	if ( tItem.fW <= 0.0f || tItem.fH <= 0.0f ) pNode->iState |= XUI_ACCESSIBLE_STATE_OFFSCREEN;
	if ( (pItem->iState & XUI_STATUSBAR_ITEM_ENABLED) == 0u ) pNode->iState |= XUI_ACCESSIBLE_STATE_DISABLED;
	if ( pItem->iType == XUI_STATUSBAR_ITEM_PROGRESS ) {
		pNode->tRange.iSize = sizeof(pNode->tRange);
		pNode->tRange.fValue = pItem->fValue;
		pNode->tRange.fMin = pItem->fMin;
		pNode->tRange.fMax = pItem->fMax;
	}
	if ( __xuiStatusBarItemInteractive(pItem) && tItem.fW > 0 && tItem.fH > 0 ) {
		pNode->iState |= XUI_ACCESSIBLE_STATE_FOCUSABLE | XUI_ACCESSIBLE_STATE_SELECTABLE;
		if ( iItem == pData->iHover ) {
			pNode->iState |= XUI_ACCESSIBLE_STATE_SELECTED;
			if ( xuiGetFocusWidget(xuiWidgetGetContext(pWidget)) == pWidget ) {
				pNode->iState |= XUI_ACCESSIBLE_STATE_FOCUSED;
			}
		}
		pNode->iActions = XUI_ACCESSIBLE_ACTION_MASK(XUI_ACCESSIBLE_ACTION_FOCUS) |
			XUI_ACCESSIBLE_ACTION_MASK(XUI_ACCESSIBLE_ACTION_ACTIVATE);
	}
	return XUI_OK;
}

static int __xuiStatusBarAccessibleAction(xui_widget pWidget, uint64_t iNodeId,
	int iAction, const void* pPayload, void* pUser)
{
	xui_statusbar_data_t* pData = (xui_statusbar_data_t*)pUser;
	xui_statusbar_resolved_t tResolved;
	int iItem;
	int iRet;

	(void)pPayload;
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( iNodeId == 0 || iNodeId == 1 ) {
		return iAction == XUI_ACCESSIBLE_ACTION_FOCUS ?
			xuiSetFocusWidget(xuiWidgetGetContext(pWidget), pWidget) : XUI_ERROR_UNSUPPORTED;
	}
	if ( iNodeId < 2 || iNodeId > (uint64_t)pData->iItemCount + 1u ) return XUI_ERROR_NOT_FOUND;
	iItem = (int)iNodeId - 2;
	if ( pData->arrItems[iItem].iType == XUI_STATUSBAR_ITEM_SPACER ) return XUI_ERROR_NOT_FOUND;
	__xuiStatusBarResolve(pWidget, pData, &tResolved);
	iRet = __xuiStatusBarLayoutItems(pWidget, pData, &tResolved);
	if ( iRet != XUI_OK ) return iRet;
	if ( !__xuiStatusBarItemInteractive(&pData->arrItems[iItem]) ) return XUI_ERROR_INVALID_STATE;
	if ( pData->arrItems[iItem].tRect.fW <= 0 || pData->arrItems[iItem].tRect.fH <= 0 ) {
		return XUI_ERROR_INVALID_STATE;
	}
	if ( iAction == XUI_ACCESSIBLE_ACTION_FOCUS ) {
		iRet = __xuiStatusBarSetHover(pWidget, pData, iItem);
		if ( iRet != XUI_OK ) return iRet;
		if ( __xuiStatusBarGetData(pWidget) != pData ) return XUI_OK;
		return xuiSetFocusWidget(xuiWidgetGetContext(pWidget), pWidget);
	}
	if ( iAction == XUI_ACCESSIBLE_ACTION_ACTIVATE ) return __xuiStatusBarSelect(pWidget, pData, iItem);
	return XUI_ERROR_UNSUPPORTED;
}

static int __xuiStatusBarInit(xui_widget pWidget, void* pTypeData, const void* pCreateData, void* pUser)
{
	xui_statusbar_data_t* pData;
	const xui_statusbar_desc_t* pDesc;
	int iRet;

	(void)pUser;
	if ( (pWidget == NULL) || (pTypeData == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	pData = (xui_statusbar_data_t*)pTypeData;
	pDesc = (const xui_statusbar_desc_t*)pCreateData;
	memset(pData, 0, sizeof(*pData));
	__xuiStatusBarDefaultMetrics(&pData->tMetrics);
	__xuiStatusBarDefaultColors(&pData->tColors);
	pData->iHover = -1;
	pData->iActive = -1;
	pData->pFont = xuiGetDefaultFont(xuiWidgetGetContext(pWidget));
	if ( (pDesc != NULL) && (pDesc->pFont != NULL) ) pData->pFont = pDesc->pFont;
	if ( (pDesc != NULL) && pDesc->bHasMetrics ) {
		if ( !__xuiStatusBarMetricsValid(&pDesc->tMetrics) ) return XUI_ERROR_INVALID_ARGUMENT;
		pData->tMetrics = pDesc->tMetrics;
		pData->tMetrics.iSize = sizeof(pData->tMetrics);
	}
	if ( (pDesc != NULL) && pDesc->bHasColors ) {
		if ( !__xuiStatusBarColorsValid(&pDesc->tColors) ) return XUI_ERROR_INVALID_ARGUMENT;
		pData->tColors = pDesc->tColors;
		pData->tColors.iSize = sizeof(pData->tColors);
	}
	iRet = xuiWidgetSetFocusable(pWidget, 1);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetTabStop(pWidget, 0);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetOverflow(pWidget, XUI_OVERFLOW_VISIBLE);
	if ( iRet == XUI_OK ) iRet = __xuiStatusBarInitEvents(pWidget);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetAccessibilityProvider(pWidget,
		__xuiStatusBarAccessibleCount, __xuiStatusBarAccessibleGet,
		__xuiStatusBarAccessibleAction, pData);
	if ( iRet != XUI_OK ) return iRet;
	if ( (pDesc != NULL) && (pDesc->iItemCount > 0) ) {
		return xuiStatusBarSetItems(pWidget, pDesc->pItems, pDesc->iItemCount);
	}
	return XUI_OK;
}

static void __xuiStatusBarDestroy(xui_widget pWidget, void* pTypeData, void* pUser)
{
	xui_statusbar_data_t* pData;

	(void)pUser;
	pData = (xui_statusbar_data_t*)pTypeData;
	if ( pWidget != NULL ) {
		if ( pData != NULL ) (void)xuiInternalPointerOwnerCancel(
			xuiWidgetGetContext(pWidget), pWidget, &pData->tPointerOwner);
	}
	if ( pData != NULL ) {
		__xuiStatusBarFreeStrings(pData->arrOwnedTexts, XUI_STATUSBAR_ITEM_CAPACITY);
		memset(pData, 0, sizeof(*pData));
	}
}

static xui_statusbar_data_t* __xuiStatusBarGetData(xui_widget pWidget)
{
	xui_context pContext;
	xui_widget_type pType;

	if ( !xuiInternalWidgetIsValid(pWidget) ) return NULL;
	pContext = xuiWidgetGetContext(pWidget);
	pType = xuiWidgetFindType(pContext, "statusbar");
	if ( (pType == NULL) || !xuiWidgetIsType(pWidget, pType) ) return NULL;
	return (xui_statusbar_data_t*)xuiWidgetGetTypeData(pWidget);
}

static void __xuiStatusBarRegisterStyleProperty(xui_context pContext, xui_widget_type pType, const char* sName, int iValueType, uint32_t iDirtyFlags, uint32_t iFlags)
{
	xui_style_property_info_t tInfo;

	if ( xuiStyleFindProperty(pContext, sName) != 0 ) return;
	memset(&tInfo, 0, sizeof(tInfo));
	tInfo.iSize = sizeof(tInfo);
	tInfo.sName = sName;
	tInfo.iValueType = iValueType;
	tInfo.iFlags = iFlags;
	tInfo.iDirtyFlags = iDirtyFlags;
	tInfo.pWidgetType = pType;
	(void)xuiStyleRegisterProperty(pContext, &tInfo, NULL);
}

static void __xuiStatusBarRegisterStyleProperties(xui_context pContext, xui_widget_type pType)
{
	uint32_t iPaintDirty;
	uint32_t iLayoutDirty;

	iPaintDirty = XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER;
	iLayoutDirty = XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER;
	__xuiStatusBarRegisterStyleProperty(pContext, pType, "font.name", XUI_STYLE_VALUE_STRING, iLayoutDirty, XUI_STYLE_PROPERTY_INHERITED);
	__xuiStatusBarRegisterStyleProperty(pContext, pType, "statusbar.background.color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiStatusBarRegisterStyleProperty(pContext, pType, "statusbar.border.color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiStatusBarRegisterStyleProperty(pContext, pType, "statusbar.highlight.color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiStatusBarRegisterStyleProperty(pContext, pType, "statusbar.item.color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiStatusBarRegisterStyleProperty(pContext, pType, "statusbar.item.hover_color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiStatusBarRegisterStyleProperty(pContext, pType, "statusbar.item.active_color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiStatusBarRegisterStyleProperty(pContext, pType, "statusbar.focus.color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiStatusBarRegisterStyleProperty(pContext, pType, "statusbar.text.color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiStatusBarRegisterStyleProperty(pContext, pType, "statusbar.text.active_color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiStatusBarRegisterStyleProperty(pContext, pType, "statusbar.text.disabled_color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiStatusBarRegisterStyleProperty(pContext, pType, "statusbar.progress.track_color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiStatusBarRegisterStyleProperty(pContext, pType, "statusbar.progress.fill_color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiStatusBarRegisterStyleProperty(pContext, pType, "statusbar.height", XUI_STYLE_VALUE_FLOAT, iLayoutDirty, 0);
	__xuiStatusBarRegisterStyleProperty(pContext, pType, "statusbar.padding.x", XUI_STYLE_VALUE_FLOAT, iLayoutDirty, 0);
	__xuiStatusBarRegisterStyleProperty(pContext, pType, "statusbar.padding.y", XUI_STYLE_VALUE_FLOAT, iLayoutDirty, 0);
	__xuiStatusBarRegisterStyleProperty(pContext, pType, "statusbar.gap", XUI_STYLE_VALUE_FLOAT, iLayoutDirty, 0);
	__xuiStatusBarRegisterStyleProperty(pContext, pType, "statusbar.item.padding.x", XUI_STYLE_VALUE_FLOAT, iLayoutDirty, 0);
	__xuiStatusBarRegisterStyleProperty(pContext, pType, "statusbar.item.padding.y", XUI_STYLE_VALUE_FLOAT, iLayoutDirty, 0);
	__xuiStatusBarRegisterStyleProperty(pContext, pType, "statusbar.progress.height", XUI_STYLE_VALUE_FLOAT, iPaintDirty, 0);
	__xuiStatusBarRegisterStyleProperty(pContext, pType, "statusbar.border.width", XUI_STYLE_VALUE_FLOAT, iPaintDirty, 0);
	__xuiStatusBarRegisterStyleProperty(pContext, pType, "statusbar.top_border.width", XUI_STYLE_VALUE_FLOAT, iPaintDirty, 0);
}

XUI_API xui_widget_type xuiStatusBarGetType(xui_context pContext)
{
	xui_widget_type pType;
	xui_widget_type_desc_t tDesc;
	int iRet;

	if ( !xuiInternalContextIsValid(pContext) ) return NULL;
	pType = xuiWidgetFindType(pContext, "statusbar");
	if ( pType != NULL ) return pType;
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.sName = "statusbar";
	tDesc.pParent = xuiWidgetGetBaseType();
	tDesc.iFlags = XUI_WIDGET_TYPE_DEFAULT_LAYOUT | XUI_WIDGET_TYPE_DEFAULT_CACHE_POLICY;
	tDesc.iTypeDataSize = sizeof(xui_statusbar_data_t);
	tDesc.onInit = __xuiStatusBarInit;
	tDesc.onDestroy = __xuiStatusBarDestroy;
	tDesc.onContentMeasure = __xuiStatusBarContentMeasure;
	tDesc.onCacheRender = __xuiStatusBarCacheRender;
	__xuiStatusBarDefaultLayout(&tDesc.tLayout);
	__xuiStatusBarDefaultCachePolicy(&tDesc.tCachePolicy);
	iRet = xuiWidgetRegisterType(pContext, &pType, &tDesc);
	if ( iRet != XUI_OK ) return NULL;
	__xuiStatusBarRegisterStyleProperties(pContext, pType);
	return pType;
}

XUI_API int xuiStatusBarCreate(xui_context pContext, xui_widget* ppWidget, const xui_statusbar_desc_t* pDesc)
{
	xui_widget_type pType;

	if ( !xuiInternalContextIsValid(pContext) || (ppWidget == NULL) || !__xuiStatusBarDescValid(pDesc) ) return XUI_ERROR_INVALID_ARGUMENT;
	*ppWidget = NULL;
	pType = xuiStatusBarGetType(pContext);
	if ( pType == NULL ) return XUI_ERROR_NOT_INITIALIZED;
	return xuiWidgetCreateTyped(pContext, pType, ppWidget, pDesc);
}

XUI_API int xuiStatusBarSetSelect(xui_widget pWidget, xui_statusbar_select_proc onSelect, void* pUser)
{
	xui_statusbar_data_t* pData = __xuiStatusBarGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	pData->onSelect = onSelect;
	pData->pSelectUser = pUser;
	return XUI_OK;
}

XUI_API int xuiStatusBarSetItems(xui_widget pWidget, const xui_statusbar_item_t* pItems, int iCount)
{
	xui_statusbar_data_t* pData = __xuiStatusBarGetData(pWidget);
	xui_statusbar_item_t arrNew[XUI_STATUSBAR_ITEM_CAPACITY];
	xui_statusbar_item_t arrOld[XUI_STATUSBAR_ITEM_CAPACITY];
	char* arrNewTexts[XUI_STATUSBAR_ITEM_CAPACITY] = {0};
	char* arrOldTexts[XUI_STATUSBAR_ITEM_CAPACITY];
	int iOldCount;
	int iOldHover;
	int iOldActive;
	int iOldChangeCount;
	int iRet;
	int i;

	if ( (pData == NULL) || (iCount < 0) || (iCount > XUI_STATUSBAR_ITEM_CAPACITY) || ((iCount > 0) && (pItems == NULL)) ) return XUI_ERROR_INVALID_ARGUMENT;
	memset(arrNew, 0, sizeof(arrNew));
	for ( i = 0; i < iCount; i++ ) {
		if ( !__xuiStatusBarItemValid(&pItems[i]) ) return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( pData->iItemCount == iCount ) {
		for ( i = 0; i < iCount; i++ ) {
			if ( !__xuiStatusBarItemEqual(&pData->arrItems[i], &pItems[i]) ) break;
		}
		if ( i == iCount ) return XUI_OK;
	}
	for ( i = 0; i < iCount; i++ ) {
		arrNew[i] = pItems[i];
		if ( arrNew[i].iType == XUI_STATUSBAR_ITEM_SPACER ) {
			arrNew[i].iState &= ~(XUI_STATUSBAR_ITEM_ENABLED | XUI_STATUSBAR_ITEM_CLICKABLE);
		}
		if ( arrNew[i].iType == XUI_STATUSBAR_ITEM_PROGRESS ) {
			if ( arrNew[i].fMax < arrNew[i].fMin ) arrNew[i].fMax = arrNew[i].fMin;
			if ( arrNew[i].fValue < arrNew[i].fMin ) arrNew[i].fValue = arrNew[i].fMin;
			if ( arrNew[i].fValue > arrNew[i].fMax ) arrNew[i].fValue = arrNew[i].fMax;
		}
		memset(&arrNew[i].tRect, 0, sizeof(arrNew[i].tRect));
		arrNewTexts[i] = __xuiStatusBarCopyString(arrNew[i].sText);
		if ( arrNew[i].sText != NULL && arrNew[i].sText[0] != '\0' && arrNewTexts[i] == NULL ) {
			__xuiStatusBarFreeStrings(arrNewTexts, i + 1);
			return XUI_ERROR_OUT_OF_MEMORY;
		}
		arrNew[i].sText = arrNewTexts[i] != NULL ? arrNewTexts[i] : "";
	}
	iRet = __xuiStatusBarCancelPress(pWidget, pData, 1);
	if ( iRet != XUI_OK ) {
		__xuiStatusBarFreeStrings(arrNewTexts, iCount);
		return iRet;
	}
	if ( __xuiStatusBarGetData(pWidget) != pData ) {
		__xuiStatusBarFreeStrings(arrNewTexts, iCount);
		return XUI_OK;
	}
	memcpy(arrOld, pData->arrItems, sizeof(arrOld));
	memcpy(arrOldTexts, pData->arrOwnedTexts, sizeof(arrOldTexts));
	iOldCount = pData->iItemCount;
	iOldHover = pData->iHover;
	iOldActive = pData->iActive;
	iOldChangeCount = pData->iChangeCount;
	memcpy(pData->arrItems, arrNew, sizeof(arrNew));
	memcpy(pData->arrOwnedTexts, arrNewTexts, sizeof(arrNewTexts));
	pData->iItemCount = iCount;
	pData->iHover = -1;
	pData->iActive = -1;
	__xuiStatusBarMarkChanged(pData);
	iRet = xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	if ( iRet != XUI_OK ) {
		memcpy(pData->arrItems, arrOld, sizeof(arrOld));
		memcpy(pData->arrOwnedTexts, arrOldTexts, sizeof(arrOldTexts));
		pData->iItemCount = iOldCount;
		pData->iHover = iOldHover;
		pData->iActive = iOldActive;
		pData->iChangeCount = iOldChangeCount;
		__xuiStatusBarFreeStrings(arrNewTexts, iCount);
		return iRet;
	}
	__xuiStatusBarFreeStrings(arrOldTexts, iOldCount);
	xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_TREE_CHANGED);
	xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_SELECTION_CHANGED);
	return XUI_OK;
}

static int __xuiStatusBarAddItem(xui_widget pWidget, const xui_statusbar_item_t* pSource)
{
	xui_statusbar_data_t* pData = __xuiStatusBarGetData(pWidget);
	xui_statusbar_item_t* pItem;
	char* sCopy;
	int iOldChangeCount;
	int iRet;
	int iIndex;

	if ( (pData == NULL) || !__xuiStatusBarItemValid(pSource) ||
	     (pData->iItemCount >= XUI_STATUSBAR_ITEM_CAPACITY) ) return -1;
	sCopy = __xuiStatusBarCopyString(pSource->sText);
	if ( pSource->sText != NULL && pSource->sText[0] != '\0' && sCopy == NULL ) return -1;
	iOldChangeCount = pData->iChangeCount;
	iIndex = pData->iItemCount++;
	pItem = &pData->arrItems[iIndex];
	*pItem = *pSource;
	memset(&pItem->tRect, 0, sizeof(pItem->tRect));
	pData->arrOwnedTexts[iIndex] = sCopy;
	pItem->sText = sCopy != NULL ? sCopy : "";
	if ( pItem->iType == XUI_STATUSBAR_ITEM_SPACER ) {
		pItem->iState = 0u;
	}
	else if ( pItem->iType == XUI_STATUSBAR_ITEM_PROGRESS ) {
		if ( pItem->fMax < pItem->fMin ) pItem->fMax = pItem->fMin;
		if ( pItem->fValue < pItem->fMin ) pItem->fValue = pItem->fMin;
		if ( pItem->fValue > pItem->fMax ) pItem->fValue = pItem->fMax;
	}
	__xuiStatusBarMarkChanged(pData);
	iRet = xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	if ( iRet != XUI_OK ) {
		xrtFree(pData->arrOwnedTexts[iIndex]);
		pData->arrOwnedTexts[iIndex] = NULL;
		memset(pItem, 0, sizeof(*pItem));
		pData->iItemCount--;
		pData->iChangeCount = iOldChangeCount;
		return -1;
	}
	xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_TREE_CHANGED);
	return iIndex;
}

XUI_API int xuiStatusBarAddText(xui_widget pWidget, int iSection, const char* sText, float fWidth, int bClickable, int iValue)
{
	xui_statusbar_item_t tItem;
	memset(&tItem, 0, sizeof(tItem));
	tItem.sText = sText;
	tItem.iType = XUI_STATUSBAR_ITEM_TEXT;
	tItem.iSection = iSection;
	tItem.iState = XUI_STATUSBAR_ITEM_ENABLED;
	if ( bClickable ) tItem.iState |= XUI_STATUSBAR_ITEM_CLICKABLE;
	tItem.iValue = iValue;
	tItem.fWidth = fWidth;
	return __xuiStatusBarAddItem(pWidget, &tItem);
}

XUI_API int xuiStatusBarAddProgress(xui_widget pWidget, int iSection, float fMin, float fMax, float fValue, float fWidth)
{
	xui_statusbar_item_t tItem;
	memset(&tItem, 0, sizeof(tItem));
	tItem.sText = "";
	tItem.iType = XUI_STATUSBAR_ITEM_PROGRESS;
	tItem.iSection = iSection;
	tItem.iState = XUI_STATUSBAR_ITEM_ENABLED;
	tItem.fWidth = fWidth;
	tItem.fMin = fMin;
	tItem.fMax = fMax;
	tItem.fValue = fValue;
	return __xuiStatusBarAddItem(pWidget, &tItem);
}

XUI_API int xuiStatusBarAddSpacer(xui_widget pWidget, int iSection, float fWidth)
{
	xui_statusbar_item_t tItem;
	memset(&tItem, 0, sizeof(tItem));
	tItem.sText = "";
	tItem.iType = XUI_STATUSBAR_ITEM_SPACER;
	tItem.iSection = iSection;
	tItem.fWidth = fWidth;
	return __xuiStatusBarAddItem(pWidget, &tItem);
}

XUI_API int xuiStatusBarAddFlexibleSpacer(xui_widget pWidget, int iSection, float fWeight)
{
	xui_statusbar_item_t tItem;
	memset(&tItem, 0, sizeof(tItem));
	if ( fWeight <= 0.0f ) fWeight = 1.0f;
	tItem.sText = "";
	tItem.iType = XUI_STATUSBAR_ITEM_SPACER;
	tItem.iSection = iSection;
	tItem.fFlex = fWeight;
	return __xuiStatusBarAddItem(pWidget, &tItem);
}

XUI_API int xuiStatusBarClear(xui_widget pWidget)
{
	xui_statusbar_data_t* pData = __xuiStatusBarGetData(pWidget);
	xui_statusbar_item_t arrOld[XUI_STATUSBAR_ITEM_CAPACITY];
	char* arrOldTexts[XUI_STATUSBAR_ITEM_CAPACITY];
	int iOldCount;
	int iOldHover;
	int iOldActive;
	int iOldChangeCount;
	int iRet;
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pData->iItemCount == 0 ) return XUI_OK;
	iRet = __xuiStatusBarCancelPress(pWidget, pData, 1);
	if ( iRet != XUI_OK ) return iRet;
	if ( __xuiStatusBarGetData(pWidget) != pData ) return XUI_OK;
	memcpy(arrOld, pData->arrItems, sizeof(arrOld));
	memcpy(arrOldTexts, pData->arrOwnedTexts, sizeof(arrOldTexts));
	iOldCount = pData->iItemCount;
	iOldHover = pData->iHover;
	iOldActive = pData->iActive;
	iOldChangeCount = pData->iChangeCount;
	memset(pData->arrItems, 0, sizeof(pData->arrItems));
	memset(pData->arrOwnedTexts, 0, sizeof(pData->arrOwnedTexts));
	pData->iItemCount = 0;
	pData->iHover = -1;
	pData->iActive = -1;
	__xuiStatusBarMarkChanged(pData);
	iRet = xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	if ( iRet != XUI_OK ) {
		memcpy(pData->arrItems, arrOld, sizeof(arrOld));
		memcpy(pData->arrOwnedTexts, arrOldTexts, sizeof(arrOldTexts));
		pData->iItemCount = iOldCount;
		pData->iHover = iOldHover;
		pData->iActive = iOldActive;
		pData->iChangeCount = iOldChangeCount;
		return iRet;
	}
	__xuiStatusBarFreeStrings(arrOldTexts, iOldCount);
	xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_TREE_CHANGED);
	xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_SELECTION_CHANGED);
	return XUI_OK;
}

XUI_API int xuiStatusBarGetItemCount(xui_widget pWidget)
{
	xui_statusbar_data_t* pData = __xuiStatusBarGetData(pWidget);
	return (pData != NULL) ? pData->iItemCount : 0;
}

XUI_API int xuiStatusBarSetContextMenu(xui_widget pWidget, xui_statusbar_context_proc onContext, void* pUser)
{
	xui_statusbar_data_t* pData = __xuiStatusBarGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	pData->onContext = onContext;
	pData->pContextUser = pUser;
	return XUI_OK;
}

XUI_API const xui_statusbar_item_t* xuiStatusBarGetItem(xui_widget pWidget, int iIndex)
{
	xui_statusbar_data_t* pData = __xuiStatusBarGetData(pWidget);
	if ( (pData == NULL) || (iIndex < 0) || (iIndex >= pData->iItemCount) ) return NULL;
	return &pData->arrItems[iIndex];
}

XUI_API xui_rect_t xuiStatusBarGetItemRect(xui_widget pWidget, int iIndex)
{
	xui_statusbar_data_t* pData = __xuiStatusBarGetData(pWidget);
	xui_statusbar_resolved_t tResolved;
	xui_rect_t tRect;

	memset(&tRect, 0, sizeof(tRect));
	if ( (pData == NULL) || (iIndex < 0) || (iIndex >= pData->iItemCount) ) return tRect;
	__xuiStatusBarResolve(pWidget, pData, &tResolved);
	if ( __xuiStatusBarLayoutItems(pWidget, pData, &tResolved) != XUI_OK ) return tRect;
	return pData->arrItems[iIndex].tRect;
}

XUI_API int xuiStatusBarGetItemAt(xui_widget pWidget, float fX, float fY)
{
	xui_statusbar_data_t* pData = __xuiStatusBarGetData(pWidget);
	xui_statusbar_resolved_t tResolved;
	if ( pData == NULL ) return -1;
	__xuiStatusBarResolve(pWidget, pData, &tResolved);
	if ( __xuiStatusBarLayoutItems(pWidget, pData, &tResolved) != XUI_OK ) return -1;
	return __xuiStatusBarIndexAt(pData, fX, fY);
}

XUI_API int xuiStatusBarSetItemEnabled(xui_widget pWidget, int iIndex, int bEnabled)
{
	xui_statusbar_data_t* pData = __xuiStatusBarGetData(pWidget);
	uint32_t iOldState;
	int iOldHover;
	int iOldActive;
	int iOldChangeCount;
	int iRet;
	if ( (pData == NULL) || (iIndex < 0) || (iIndex >= pData->iItemCount) ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pData->arrItems[iIndex].iType == XUI_STATUSBAR_ITEM_SPACER ) return XUI_OK;
	if ( ((pData->arrItems[iIndex].iState & XUI_STATUSBAR_ITEM_ENABLED) != 0u) == (bEnabled != 0) ) return XUI_OK;
	if ( !bEnabled && pData->iActive == iIndex ) {
		iRet = __xuiStatusBarCancelPress(pWidget, pData, 0);
		if ( iRet != XUI_OK ) return iRet;
		if ( __xuiStatusBarGetData(pWidget) != pData ) return XUI_OK;
	}
	iOldState = pData->arrItems[iIndex].iState;
	iOldHover = pData->iHover;
	iOldActive = pData->iActive;
	iOldChangeCount = pData->iChangeCount;
	if ( bEnabled ) pData->arrItems[iIndex].iState |= XUI_STATUSBAR_ITEM_ENABLED;
	else {
		pData->arrItems[iIndex].iState &= ~XUI_STATUSBAR_ITEM_ENABLED;
		if ( pData->iHover == iIndex ) pData->iHover = -1;
	}
	__xuiStatusBarMarkChanged(pData);
	iRet = xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	if ( iRet != XUI_OK ) {
		pData->arrItems[iIndex].iState = iOldState;
		pData->iHover = iOldHover;
		pData->iActive = iOldActive;
		pData->iChangeCount = iOldChangeCount;
		return iRet;
	}
	xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_STATE_CHANGED);
	return XUI_OK;
}

XUI_API int xuiStatusBarIsItemEnabled(xui_widget pWidget, int iIndex)
{
	xui_statusbar_data_t* pData = __xuiStatusBarGetData(pWidget);
	if ( (pData == NULL) || (iIndex < 0) || (iIndex >= pData->iItemCount) ) return 0;
	return (pData->arrItems[iIndex].iState & XUI_STATUSBAR_ITEM_ENABLED) != 0u;
}

XUI_API int xuiStatusBarSetItemClickable(xui_widget pWidget, int iIndex, int bClickable)
{
	xui_statusbar_data_t* pData = __xuiStatusBarGetData(pWidget);
	uint32_t iOldState;
	int iOldHover;
	int iOldActive;
	int iOldChangeCount;
	int iRet;
	if ( (pData == NULL) || (iIndex < 0) || (iIndex >= pData->iItemCount) ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pData->arrItems[iIndex].iType == XUI_STATUSBAR_ITEM_SPACER ) return XUI_OK;
	if ( ((pData->arrItems[iIndex].iState & XUI_STATUSBAR_ITEM_CLICKABLE) != 0u) == (bClickable != 0) ) return XUI_OK;
	if ( !bClickable && pData->iActive == iIndex ) {
		iRet = __xuiStatusBarCancelPress(pWidget, pData, 0);
		if ( iRet != XUI_OK ) return iRet;
		if ( __xuiStatusBarGetData(pWidget) != pData ) return XUI_OK;
	}
	iOldState = pData->arrItems[iIndex].iState;
	iOldHover = pData->iHover;
	iOldActive = pData->iActive;
	iOldChangeCount = pData->iChangeCount;
	if ( bClickable ) pData->arrItems[iIndex].iState |= XUI_STATUSBAR_ITEM_CLICKABLE;
	else {
		pData->arrItems[iIndex].iState &= ~XUI_STATUSBAR_ITEM_CLICKABLE;
		if ( pData->iHover == iIndex ) pData->iHover = -1;
	}
	__xuiStatusBarMarkChanged(pData);
	iRet = xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	if ( iRet != XUI_OK ) {
		pData->arrItems[iIndex].iState = iOldState;
		pData->iHover = iOldHover;
		pData->iActive = iOldActive;
		pData->iChangeCount = iOldChangeCount;
		return iRet;
	}
	xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_STATE_CHANGED);
	return XUI_OK;
}

XUI_API int xuiStatusBarIsItemClickable(xui_widget pWidget, int iIndex)
{
	xui_statusbar_data_t* pData = __xuiStatusBarGetData(pWidget);
	if ( (pData == NULL) || (iIndex < 0) || (iIndex >= pData->iItemCount) ) return 0;
	return (pData->arrItems[iIndex].iState & XUI_STATUSBAR_ITEM_CLICKABLE) != 0u;
}

XUI_API int xuiStatusBarSetItemText(xui_widget pWidget, int iIndex, const char* sText)
{
	xui_statusbar_data_t* pData = __xuiStatusBarGetData(pWidget);
	const char* sValue = sText != NULL ? sText : "";
	char* sCopy;
	char* sOld;
	const char* sOldValue;
	int iOldChangeCount;
	int iRet;
	if ( (pData == NULL) || (iIndex < 0) || (iIndex >= pData->iItemCount) ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pData->arrItems[iIndex].sText != NULL && strcmp(pData->arrItems[iIndex].sText, sValue) == 0 ) return XUI_OK;
	sCopy = __xuiStatusBarCopyString(sValue);
	if ( sValue[0] != '\0' && sCopy == NULL ) return XUI_ERROR_OUT_OF_MEMORY;
	sOld = pData->arrOwnedTexts[iIndex];
	sOldValue = pData->arrItems[iIndex].sText;
	iOldChangeCount = pData->iChangeCount;
	pData->arrOwnedTexts[iIndex] = sCopy;
	pData->arrItems[iIndex].sText = sCopy != NULL ? sCopy : "";
	__xuiStatusBarMarkChanged(pData);
	iRet = xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	if ( iRet != XUI_OK ) {
		pData->arrOwnedTexts[iIndex] = sOld;
		pData->arrItems[iIndex].sText = sOldValue;
		pData->iChangeCount = iOldChangeCount;
		xrtFree(sCopy);
		return iRet;
	}
	xrtFree(sOld);
	xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_NODE_CHANGED);
	xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_BOUNDS_CHANGED);
	return XUI_OK;
}

XUI_API int xuiStatusBarSetItemValue(xui_widget pWidget, int iIndex, int iValue)
{
	xui_statusbar_data_t* pData = __xuiStatusBarGetData(pWidget);
	if ( (pData == NULL) || (iIndex < 0) || (iIndex >= pData->iItemCount) ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pData->arrItems[iIndex].iValue == iValue ) return XUI_OK;
	pData->arrItems[iIndex].iValue = iValue;
	__xuiStatusBarMarkChanged(pData);
	xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_NODE_CHANGED);
	return XUI_OK;
}

XUI_API int xuiStatusBarGetItemValue(xui_widget pWidget, int iIndex)
{
	xui_statusbar_data_t* pData = __xuiStatusBarGetData(pWidget);
	if ( (pData == NULL) || (iIndex < 0) || (iIndex >= pData->iItemCount) ) return 0;
	return pData->arrItems[iIndex].iValue;
}

XUI_API int xuiStatusBarSetItemWidth(xui_widget pWidget, int iIndex, float fWidth)
{
	xui_statusbar_data_t* pData = __xuiStatusBarGetData(pWidget);
	float fOldWidth;
	int iOldChangeCount;
	int iRet;
	if ( (pData == NULL) || (iIndex < 0) || (iIndex >= pData->iItemCount) || !__xuiStatusBarNonNegativeFloatValid(fWidth) ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pData->arrItems[iIndex].fWidth == fWidth ) return XUI_OK;
	fOldWidth = pData->arrItems[iIndex].fWidth;
	iOldChangeCount = pData->iChangeCount;
	pData->arrItems[iIndex].fWidth = fWidth;
	__xuiStatusBarMarkChanged(pData);
	iRet = xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	if ( iRet != XUI_OK ) {
		pData->arrItems[iIndex].fWidth = fOldWidth;
		pData->iChangeCount = iOldChangeCount;
		return iRet;
	}
	xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_BOUNDS_CHANGED);
	return XUI_OK;
}

XUI_API int xuiStatusBarSetItemFlex(xui_widget pWidget, int iIndex, float fFlex)
{
	xui_statusbar_data_t* pData = __xuiStatusBarGetData(pWidget);
	float fOldFlex;
	float fOldWidth;
	int iOldChangeCount;
	int iRet;
	if ( (pData == NULL) || (iIndex < 0) || (iIndex >= pData->iItemCount) || !__xuiStatusBarNonNegativeFloatValid(fFlex) ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pData->arrItems[iIndex].fFlex == fFlex && (fFlex <= 0.0f || pData->arrItems[iIndex].fWidth == 0.0f) ) return XUI_OK;
	fOldFlex = pData->arrItems[iIndex].fFlex;
	fOldWidth = pData->arrItems[iIndex].fWidth;
	iOldChangeCount = pData->iChangeCount;
	pData->arrItems[iIndex].fFlex = fFlex;
	if ( fFlex > 0.0f ) pData->arrItems[iIndex].fWidth = 0.0f;
	__xuiStatusBarMarkChanged(pData);
	iRet = xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	if ( iRet != XUI_OK ) {
		pData->arrItems[iIndex].fFlex = fOldFlex;
		pData->arrItems[iIndex].fWidth = fOldWidth;
		pData->iChangeCount = iOldChangeCount;
		return iRet;
	}
	xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_BOUNDS_CHANGED);
	return XUI_OK;
}

XUI_API int xuiStatusBarSetProgress(xui_widget pWidget, int iIndex, float fValue)
{
	xui_statusbar_data_t* pData = __xuiStatusBarGetData(pWidget);
	xui_statusbar_item_t* pItem;
	float fOldValue;
	int iOldChangeCount;
	int iRet;
	if ( (pData == NULL) || (iIndex < 0) || (iIndex >= pData->iItemCount) ) return XUI_ERROR_INVALID_ARGUMENT;
	pItem = &pData->arrItems[iIndex];
	if ( (pItem->iType != XUI_STATUSBAR_ITEM_PROGRESS) || !__xuiStatusBarFloatValid(fValue) ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( fValue < pItem->fMin ) fValue = pItem->fMin;
	if ( fValue > pItem->fMax ) fValue = pItem->fMax;
	if ( pItem->fValue == fValue ) return XUI_OK;
	fOldValue = pItem->fValue;
	iOldChangeCount = pData->iChangeCount;
	pItem->fValue = fValue;
	__xuiStatusBarMarkChanged(pData);
	iRet = xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	if ( iRet != XUI_OK ) {
		pItem->fValue = fOldValue;
		pData->iChangeCount = iOldChangeCount;
		return iRet;
	}
	xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_VALUE_CHANGED);
	return XUI_OK;
}

XUI_API float xuiStatusBarGetProgress(xui_widget pWidget, int iIndex)
{
	xui_statusbar_data_t* pData = __xuiStatusBarGetData(pWidget);
	if ( (pData == NULL) || (iIndex < 0) || (iIndex >= pData->iItemCount) ||
	     pData->arrItems[iIndex].iType != XUI_STATUSBAR_ITEM_PROGRESS ) return 0.0f;
	return pData->arrItems[iIndex].fValue;
}

XUI_API int xuiStatusBarGetHoverIndex(xui_widget pWidget)
{
	xui_statusbar_data_t* pData = __xuiStatusBarGetData(pWidget);
	return (pData != NULL) ? pData->iHover : -1;
}

XUI_API int xuiStatusBarSetHoverIndex(xui_widget pWidget, int iIndex)
{
	xui_statusbar_data_t* pData = __xuiStatusBarGetData(pWidget);
	xui_statusbar_resolved_t tResolved;
	int iRet;
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	__xuiStatusBarResolve(pWidget, pData, &tResolved);
	iRet = __xuiStatusBarLayoutItems(pWidget, pData, &tResolved);
	if ( iRet != XUI_OK ) return iRet;
	return __xuiStatusBarSetHover(pWidget, pData, iIndex);
}

XUI_API int xuiStatusBarGetActiveIndex(xui_widget pWidget)
{
	xui_statusbar_data_t* pData = __xuiStatusBarGetData(pWidget);
	return (pData != NULL) ? pData->iActive : -1;
}

XUI_API int xuiStatusBarSetFont(xui_widget pWidget, xui_font pFont)
{
	xui_statusbar_data_t* pData = __xuiStatusBarGetData(pWidget);
	xui_font pOldFont;
	int iOldChangeCount;
	int iRet;
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pData->pFont == pFont ) return XUI_OK;
	pOldFont = pData->pFont;
	iOldChangeCount = pData->iChangeCount;
	pData->pFont = pFont;
	__xuiStatusBarMarkChanged(pData);
	iRet = xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	if ( iRet != XUI_OK ) {
		pData->pFont = pOldFont;
		pData->iChangeCount = iOldChangeCount;
		return iRet;
	}
	xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_BOUNDS_CHANGED);
	return XUI_OK;
}

XUI_API xui_font xuiStatusBarGetFont(xui_widget pWidget)
{
	xui_statusbar_data_t* pData = __xuiStatusBarGetData(pWidget);
	return (pData != NULL) ? pData->pFont : NULL;
}

XUI_API int xuiStatusBarSetMetrics(xui_widget pWidget, const xui_statusbar_metrics_t* pMetrics)
{
	xui_statusbar_data_t* pData = __xuiStatusBarGetData(pWidget);
	xui_statusbar_metrics_t tNew;
	xui_statusbar_metrics_t tOld;
	int iOldChangeCount;
	int iRet;
	if ( (pData == NULL) || !__xuiStatusBarMetricsValid(pMetrics) ) return XUI_ERROR_INVALID_ARGUMENT;
	tNew = *pMetrics;
	tNew.iSize = sizeof(tNew);
	if ( __xuiStatusBarMetricsEqual(&pData->tMetrics, &tNew) ) return XUI_OK;
	tOld = pData->tMetrics;
	iOldChangeCount = pData->iChangeCount;
	pData->tMetrics = tNew;
	__xuiStatusBarMarkChanged(pData);
	iRet = xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	if ( iRet != XUI_OK ) {
		pData->tMetrics = tOld;
		pData->iChangeCount = iOldChangeCount;
		return iRet;
	}
	xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_BOUNDS_CHANGED);
	return XUI_OK;
}

XUI_API int xuiStatusBarGetMetrics(xui_widget pWidget, xui_statusbar_metrics_t* pMetrics)
{
	xui_statusbar_data_t* pData = __xuiStatusBarGetData(pWidget);
	if ( (pData == NULL) || (pMetrics == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	*pMetrics = pData->tMetrics;
	pMetrics->iSize = sizeof(*pMetrics);
	return XUI_OK;
}

XUI_API int xuiStatusBarSetColors(xui_widget pWidget, const xui_statusbar_colors_t* pColors)
{
	xui_statusbar_data_t* pData = __xuiStatusBarGetData(pWidget);
	xui_statusbar_colors_t tNew;
	xui_statusbar_colors_t tOld;
	int iOldChangeCount;
	int iRet;
	if ( (pData == NULL) || !__xuiStatusBarColorsValid(pColors) ) return XUI_ERROR_INVALID_ARGUMENT;
	tNew = *pColors;
	tNew.iSize = sizeof(tNew);
	if ( __xuiStatusBarColorsEqual(&pData->tColors, &tNew) ) return XUI_OK;
	tOld = pData->tColors;
	iOldChangeCount = pData->iChangeCount;
	pData->tColors = tNew;
	__xuiStatusBarMarkChanged(pData);
	iRet = xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	if ( iRet != XUI_OK ) {
		pData->tColors = tOld;
		pData->iChangeCount = iOldChangeCount;
	}
	return iRet;
}

XUI_API int xuiStatusBarGetColors(xui_widget pWidget, xui_statusbar_colors_t* pColors)
{
	xui_statusbar_data_t* pData = __xuiStatusBarGetData(pWidget);
	if ( (pData == NULL) || (pColors == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	*pColors = pData->tColors;
	pColors->iSize = sizeof(*pColors);
	return XUI_OK;
}

XUI_API uint32_t xuiStatusBarGetState(xui_widget pWidget)
{
	xui_statusbar_data_t* pData = __xuiStatusBarGetData(pWidget);
	uint32_t iState = 0u;
	if ( pData == NULL ) return XUI_WIDGET_STATE_DISABLED;
	if ( !xuiWidgetGetEnabled(pWidget) ) iState |= XUI_WIDGET_STATE_DISABLED;
	if ( xuiGetFocusWidget(xuiWidgetGetContext(pWidget)) == pWidget ) iState |= XUI_WIDGET_STATE_FOCUS;
	if ( pData->iHover >= 0 ) iState |= XUI_WIDGET_STATE_HOVER;
	if ( pData->iActive >= 0 ) iState |= XUI_WIDGET_STATE_ACTIVE;
	return iState;
}

XUI_API int xuiStatusBarGetSelectCount(xui_widget pWidget)
{
	xui_statusbar_data_t* pData = __xuiStatusBarGetData(pWidget);
	return (pData != NULL) ? pData->iSelectCount : 0;
}

XUI_API int xuiStatusBarGetChangeCount(xui_widget pWidget)
{
	xui_statusbar_data_t* pData = __xuiStatusBarGetData(pWidget);
	return (pData != NULL) ? pData->iChangeCount : 0;
}

#endif
