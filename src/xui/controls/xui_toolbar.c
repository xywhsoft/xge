#include "xui_config.h"
#if XUI_ENABLE_TOOLBAR
#include "xui_internal.h"
#include "xui_pointer_press_internal.h"

#include <math.h>
#include <string.h>

#define XUI_TOOLBAR_ITEM_STATE_MASK (XUI_TOOLBAR_ITEM_ENABLED | XUI_TOOLBAR_ITEM_CHECKED)
#define XUI_TOOLBAR_ACCESSIBLE_OVERFLOW_ID ((uint64_t)XUI_TOOLBAR_ITEM_CAPACITY + 2u)

typedef struct xui_toolbar_data_t {
	xui_toolbar_item_t arrItems[XUI_TOOLBAR_ITEM_CAPACITY];
	char* arrOwnedTexts[XUI_TOOLBAR_ITEM_CAPACITY];
	char* arrOwnedTooltips[XUI_TOOLBAR_ITEM_CAPACITY];
	xui_toolbar_metrics_t tMetrics;
	xui_toolbar_colors_t tColors;
	xui_toolbar_select_proc onSelect;
	void* pSelectUser;
	xui_toolbar_context_proc onContext;
	void* pContextUser;
	xui_toolbar_overflow_proc onOverflow;
	void* pOverflowUser;
	xui_font pFont;
	int iItemCount;
	int iHover;
	int iActive;
	int bOverflowEnabled;
	int bOverflowFocused;
	int bOverflowActive;
	int iOverflowFirst;
	int iOverflowCount;
	xui_rect_t tOverflowRect;
	xui_pointer_owner_t tPointerOwner;
	int iSelectCount;
	int iOverflowSelectCount;
	int iChangeCount;
} xui_toolbar_data_t;

typedef struct xui_toolbar_resolved_t {
	xui_toolbar_metrics_t tMetrics;
	xui_toolbar_colors_t tColors;
	uint32_t iActiveTextColor;
	uint32_t iActiveIconColor;
	uint32_t iSeparatorHighlightColor;
	xui_font pFont;
} xui_toolbar_resolved_t;

static xui_toolbar_data_t* __xuiToolbarGetData(xui_widget pWidget);
static int __xuiToolbarMetricsValid(const xui_toolbar_metrics_t* pMetrics);
static int __xuiToolbarColorsValid(const xui_toolbar_colors_t* pColors);

static int __xuiToolbarCancelPress(xui_widget pWidget, xui_toolbar_data_t* pData)
{
	xui_context pContext;
	int bChanged;
	int iRet;

	if ( (pWidget == NULL) || (pData == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	pContext = xuiWidgetGetContext(pWidget);
	bChanged = (pData->iActive >= 0) || pData->bOverflowActive ||
		pData->tPointerOwner.bActive;
	pData->iActive = -1;
	pData->bOverflowActive = 0;
	iRet = xuiInternalPointerOwnerCancel(pContext, pWidget, &pData->tPointerOwner);
	if ( iRet != XUI_OK ) return iRet;
	if ( __xuiToolbarGetData(pWidget) != pData ) return XUI_OK;
	return bChanged ? xuiWidgetInvalidate(pWidget,
		XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER) : XUI_OK;
}

static char* __xuiToolbarCopyString(const char* sText)
{
	size_t iLength;
	char* sCopy;
	if ( sText == NULL || sText[0] == '\0' ) return NULL;
	iLength = strlen(sText);
	sCopy = (char*)xrtMalloc(iLength + 1u);
	if ( sCopy != NULL ) memcpy(sCopy, sText, iLength + 1u);
	return sCopy;
}

static void __xuiToolbarReleaseStrings(xui_toolbar_data_t* pData)
{
	int i;
	for ( i = 0; i < XUI_TOOLBAR_ITEM_CAPACITY; i++ ) {
		xrtFree(pData->arrOwnedTexts[i]);
		pData->arrOwnedTexts[i] = NULL;
		xrtFree(pData->arrOwnedTooltips[i]);
		pData->arrOwnedTooltips[i] = NULL;
	}
}

static void __xuiToolbarFreeCopiedItems(char** arrTexts, char** arrTooltips, int iCount)
{
	int i;

	for ( i = 0; i < iCount; i++ ) {
		xrtFree(arrTexts[i]);
		xrtFree(arrTooltips[i]);
		arrTexts[i] = NULL;
		arrTooltips[i] = NULL;
	}
}

static int __xuiToolbarAlpha(uint32_t iColor)
{
	return (int)(iColor & 0xffu);
}

static uint32_t __xuiToolbarColorAlpha(uint32_t iColor, uint32_t iAlpha)
{
	if ( (iColor & 0xffu) == 0u ) return iColor;
	if ( iAlpha > 255u ) iAlpha = 255u;
	return (iColor & 0xffffff00u) | (iAlpha & 0xffu);
}

static float __xuiToolbarMax(float fA, float fB)
{
	return (fA > fB) ? fA : fB;
}

static int __xuiToolbarOrientationValid(int iOrientation)
{
	return (iOrientation == XUI_ORIENTATION_HORIZONTAL) ||
	       (iOrientation == XUI_ORIENTATION_VERTICAL);
}

static int __xuiToolbarItemTypeValid(int iType)
{
	return (iType == XUI_TOOLBAR_ITEM_BUTTON) ||
	       (iType == XUI_TOOLBAR_ITEM_TOGGLE) ||
	       (iType == XUI_TOOLBAR_ITEM_SEPARATOR);
}

static int __xuiToolbarRectNonnegative(xui_rect_t tRect)
{
	return tRect.fW >= 0 && tRect.fH >= 0;
}

static int __xuiToolbarItemValid(const xui_toolbar_item_t* pItem)
{
	if ( pItem == NULL || !__xuiToolbarItemTypeValid(pItem->iType) ||
	     (pItem->iState & ~XUI_TOOLBAR_ITEM_STATE_MASK) != 0u || pItem->iGroup < 0 ||
	     !__xuiToolbarRectNonnegative(pItem->tIconSrc) ) {
		return 0;
	}
	return 1;
}

static const char* __xuiToolbarStringValue(const char* sText)
{
	return sText != NULL ? sText : "";
}

static int __xuiToolbarRectEqual(xui_rect_t tA, xui_rect_t tB)
{
	return tA.fX == tB.fX && tA.fY == tB.fY && tA.fW == tB.fW && tA.fH == tB.fH;
}

static int __xuiToolbarItemEqual(const xui_toolbar_item_t* pA, const xui_toolbar_item_t* pB)
{
	uint32_t iStateB = pB->iType == XUI_TOOLBAR_ITEM_SEPARATOR ? 0u : pB->iState;

	return strcmp(__xuiToolbarStringValue(pA->sText), __xuiToolbarStringValue(pB->sText)) == 0 &&
	       strcmp(__xuiToolbarStringValue(pA->sTooltip), __xuiToolbarStringValue(pB->sTooltip)) == 0 &&
	       pA->iType == pB->iType && pA->iState == iStateB && pA->iValue == pB->iValue &&
	       pA->iGroup == pB->iGroup && pA->pIcon == pB->pIcon &&
	       __xuiToolbarRectEqual(pA->tIconSrc, pB->tIconSrc) && pA->pUser == pB->pUser;
}

static int __xuiToolbarMetricsEqual(const xui_toolbar_metrics_t* pA, const xui_toolbar_metrics_t* pB)
{
	return pA->iOrientation == pB->iOrientation && pA->fItemWidth == pB->fItemWidth &&
	       pA->fItemHeight == pB->fItemHeight && pA->fSeparatorSize == pB->fSeparatorSize &&
	       pA->fGroupGap == pB->fGroupGap && pA->fPaddingX == pB->fPaddingX &&
	       pA->fPaddingY == pB->fPaddingY && pA->fOverflowSize == pB->fOverflowSize &&
	       pA->fBorderWidth == pB->fBorderWidth && pA->fIconSize == pB->fIconSize &&
	       pA->fIconGap == pB->fIconGap;
}

static int __xuiToolbarColorsEqual(const xui_toolbar_colors_t* pA, const xui_toolbar_colors_t* pB)
{
	return pA->iBackgroundColor == pB->iBackgroundColor && pA->iBorderColor == pB->iBorderColor &&
	       pA->iItemColor == pB->iItemColor && pA->iHoverColor == pB->iHoverColor &&
	       pA->iActiveColor == pB->iActiveColor && pA->iCheckedColor == pB->iCheckedColor &&
	       pA->iFocusColor == pB->iFocusColor && pA->iDisabledColor == pB->iDisabledColor &&
	       pA->iSeparatorColor == pB->iSeparatorColor && pA->iTextColor == pB->iTextColor &&
	       pA->iDisabledTextColor == pB->iDisabledTextColor && pA->iIconColor == pB->iIconColor;
}

static int __xuiToolbarItemInteractive(const xui_toolbar_item_t* pItem)
{
	return (pItem != NULL) &&
	       (pItem->iType != XUI_TOOLBAR_ITEM_SEPARATOR) &&
	       ((pItem->iState & XUI_TOOLBAR_ITEM_ENABLED) != 0);
}

static int __xuiToolbarRectContains(xui_rect_t tRect, float fX, float fY)
{
	return (fX >= tRect.fX) && (fY >= tRect.fY) && (fX < tRect.fX + tRect.fW) && (fY < tRect.fY + tRect.fH);
}

static int __xuiToolbarDescValid(const xui_toolbar_desc_t* pDesc)
{
	int i;

	if ( pDesc == NULL ) return 1;
	if ( (pDesc->iSize != 0) && (pDesc->iSize < sizeof(*pDesc)) ) return 0;
	if ( (pDesc->iItemCount < 0) || (pDesc->iItemCount > XUI_TOOLBAR_ITEM_CAPACITY) ) return 0;
	if ( (pDesc->iItemCount > 0) && (pDesc->pItems == NULL) ) return 0;
	if ( pDesc->bHasMetrics && !__xuiToolbarMetricsValid(&pDesc->tMetrics) ) return 0;
	if ( pDesc->bHasColors && !__xuiToolbarColorsValid(&pDesc->tColors) ) return 0;
	for ( i = 0; i < pDesc->iItemCount; i++ ) {
		if ( !__xuiToolbarItemValid(&pDesc->pItems[i]) ) return 0;
	}
	return 1;
}

static int __xuiToolbarMetricsValid(const xui_toolbar_metrics_t* pMetrics)
{
	if ( pMetrics == NULL ) return 0;
	if ( (pMetrics->iSize != 0) && (pMetrics->iSize < sizeof(*pMetrics)) ) return 0;
	if ( !__xuiToolbarOrientationValid(pMetrics->iOrientation) ) return 0;
	if ( !isfinite(pMetrics->fItemWidth) || !isfinite(pMetrics->fItemHeight) ||
	     !isfinite(pMetrics->fSeparatorSize) || !isfinite(pMetrics->fGroupGap) ||
	     !isfinite(pMetrics->fPaddingX) || !isfinite(pMetrics->fPaddingY) ||
	     !isfinite(pMetrics->fOverflowSize) || !isfinite(pMetrics->fBorderWidth) ||
	     !isfinite(pMetrics->fIconSize) || !isfinite(pMetrics->fIconGap) ||
	     (pMetrics->fItemWidth <= 0.0f) ||
	     (pMetrics->fItemHeight <= 0.0f) ||
	     (pMetrics->fSeparatorSize <= 0.0f) ||
	     (pMetrics->fGroupGap < 0.0f) ||
	     (pMetrics->fPaddingX < 0.0f) ||
	     (pMetrics->fPaddingY < 0.0f) ||
	     (pMetrics->fOverflowSize <= 0.0f) ||
	     (pMetrics->fBorderWidth < 0.0f) ||
	     (pMetrics->fIconSize < 0.0f) ||
	     (pMetrics->fIconGap < 0.0f) ) {
		return 0;
	}
	return 1;
}

static int __xuiToolbarColorsValid(const xui_toolbar_colors_t* pColors)
{
	if ( pColors == NULL ) return 0;
	return (pColors->iSize == 0) || (pColors->iSize >= sizeof(*pColors));
}

static void __xuiToolbarDefaultMetrics(xui_toolbar_metrics_t* pMetrics)
{
	memset(pMetrics, 0, sizeof(*pMetrics));
	pMetrics->iSize = sizeof(*pMetrics);
	pMetrics->iOrientation = XUI_ORIENTATION_HORIZONTAL;
	pMetrics->fItemWidth = 64.0f;
	pMetrics->fItemHeight = 26.0f;
	pMetrics->fSeparatorSize = 10.0f;
	pMetrics->fGroupGap = 6.0f;
	pMetrics->fPaddingX = 4.0f;
	pMetrics->fPaddingY = 3.0f;
	pMetrics->fOverflowSize = 26.0f;
	pMetrics->fBorderWidth = 1.0f;
	pMetrics->fIconSize = 14.0f;
	pMetrics->fIconGap = 5.0f;
}

static void __xuiToolbarDefaultColors(xui_toolbar_colors_t* pColors)
{
	memset(pColors, 0, sizeof(*pColors));
	pColors->iSize = sizeof(*pColors);
	pColors->iBackgroundColor = XUI_COLOR_RGBA(247, 250, 254, 255);
	pColors->iBorderColor = XUI_COLOR_RGBA(206, 220, 234, 255);
	pColors->iItemColor = XUI_COLOR_RGBA(0, 0, 0, 0);
	pColors->iHoverColor = XUI_COLOR_RGBA(225, 239, 251, 255);
	pColors->iActiveColor = XUI_COLOR_RGBA(47, 128, 214, 255);
	pColors->iCheckedColor = XUI_COLOR_RGBA(207, 231, 250, 255);
	pColors->iFocusColor = XUI_COLOR_RGBA(47, 128, 214, 255);
	pColors->iDisabledColor = XUI_COLOR_RGBA(231, 236, 243, 190);
	pColors->iSeparatorColor = XUI_COLOR_RGBA(185, 202, 220, 255);
	pColors->iTextColor = XUI_COLOR_RGBA(35, 58, 86, 255);
	pColors->iDisabledTextColor = XUI_COLOR_RGBA(142, 152, 166, 210);
	pColors->iIconColor = XUI_COLOR_RGBA(35, 58, 86, 255);
}

static xui_vec2_t __xuiToolbarMeasureText(xui_widget pWidget, xui_font pFont, const char* sText)
{
	xui_vec2_t tSize;
	xui_proxy pProxy;

	tSize.fX = 0.0f;
	tSize.fY = 18.0f;
	if ( (sText == NULL) || (sText[0] == '\0') ) return tSize;
	pProxy = xuiInternalContextGetProxy(xuiWidgetGetContext(pWidget));
	if ( (pProxy != NULL) && (pProxy->textMeasure != NULL) && (pFont != NULL) &&
	     (pProxy->textMeasure(pProxy, &(xui_text_item_t){.iSize=sizeof(xui_text_item_t), .pFont=pFont, .sText=sText, .iTextSize=-1, .iFlags=XUI_TEXT_SHAPE_DEFAULT}, &tSize) == XUI_OK) &&
	     isfinite(tSize.fX) && isfinite(tSize.fY) && (tSize.fX >= 0.0f) && (tSize.fY >= 0.0f) ) {
		return tSize;
	}
	tSize.fX = (float)strlen(sText) * 7.0f;
	tSize.fY = 18.0f;
	return tSize;
}

static int __xuiToolbarStyleColor(xui_widget pWidget, const char* sName, uint32_t* pColor)
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

static int __xuiToolbarStyleFloat(xui_widget pWidget, const char* sName, float* pValue)
{
	xui_style_property_t tProperty;
	int iRet;

	memset(&tProperty, 0, sizeof(tProperty));
	tProperty.iSize = sizeof(tProperty);
	iRet = xuiWidgetGetResolvedStyleProperty(pWidget, sName, &tProperty);
	if ( (iRet == XUI_OK) && (tProperty.tValue.iType == XUI_STYLE_VALUE_FLOAT) &&
	     isfinite(tProperty.tValue.fFloat) && (tProperty.tValue.fFloat >= 0.0f) ) {
		*pValue = tProperty.tValue.fFloat;
		return 1;
	}
	return 0;
}

static int __xuiToolbarStyleInt(xui_widget pWidget, const char* sName, int* pValue)
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

static xui_font __xuiToolbarStyleFont(xui_widget pWidget, xui_font pBaseFont)
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

static void __xuiToolbarResolve(xui_widget pWidget, const xui_toolbar_data_t* pData, xui_toolbar_resolved_t* pOut)
{
	int iValue;

	memset(pOut, 0, sizeof(*pOut));
	pOut->tMetrics = pData->tMetrics;
	pOut->tMetrics.iSize = sizeof(pOut->tMetrics);
	pOut->tColors = pData->tColors;
	pOut->tColors.iSize = sizeof(pOut->tColors);
	pOut->pFont = (pData->pFont != NULL) ? pData->pFont : xuiGetDefaultFont(xuiWidgetGetContext(pWidget));

	iValue = pOut->tMetrics.iOrientation;
	if ( __xuiToolbarStyleInt(pWidget, "toolbar.orientation", &iValue) && __xuiToolbarOrientationValid(iValue) ) {
		pOut->tMetrics.iOrientation = iValue;
	}
	(void)__xuiToolbarStyleFloat(pWidget, "toolbar.item.width", &pOut->tMetrics.fItemWidth);
	(void)__xuiToolbarStyleFloat(pWidget, "toolbar.item.height", &pOut->tMetrics.fItemHeight);
	(void)__xuiToolbarStyleFloat(pWidget, "toolbar.separator.size", &pOut->tMetrics.fSeparatorSize);
	(void)__xuiToolbarStyleFloat(pWidget, "toolbar.group_gap", &pOut->tMetrics.fGroupGap);
	(void)__xuiToolbarStyleFloat(pWidget, "toolbar.padding.x", &pOut->tMetrics.fPaddingX);
	(void)__xuiToolbarStyleFloat(pWidget, "toolbar.padding.y", &pOut->tMetrics.fPaddingY);
	(void)__xuiToolbarStyleFloat(pWidget, "toolbar.overflow.size", &pOut->tMetrics.fOverflowSize);
	(void)__xuiToolbarStyleFloat(pWidget, "toolbar.border.width", &pOut->tMetrics.fBorderWidth);
	(void)__xuiToolbarStyleFloat(pWidget, "toolbar.icon.size", &pOut->tMetrics.fIconSize);
	(void)__xuiToolbarStyleFloat(pWidget, "toolbar.icon.gap", &pOut->tMetrics.fIconGap);

	(void)__xuiToolbarStyleColor(pWidget, "toolbar.background.color", &pOut->tColors.iBackgroundColor);
	(void)__xuiToolbarStyleColor(pWidget, "toolbar.border.color", &pOut->tColors.iBorderColor);
	(void)__xuiToolbarStyleColor(pWidget, "toolbar.item.color", &pOut->tColors.iItemColor);
	(void)__xuiToolbarStyleColor(pWidget, "toolbar.item.hover_color", &pOut->tColors.iHoverColor);
	(void)__xuiToolbarStyleColor(pWidget, "toolbar.item.active_color", &pOut->tColors.iActiveColor);
	(void)__xuiToolbarStyleColor(pWidget, "toolbar.item.checked_color", &pOut->tColors.iCheckedColor);
	(void)__xuiToolbarStyleColor(pWidget, "toolbar.focus.color", &pOut->tColors.iFocusColor);
	(void)__xuiToolbarStyleColor(pWidget, "toolbar.item.disabled_color", &pOut->tColors.iDisabledColor);
	(void)__xuiToolbarStyleColor(pWidget, "toolbar.separator.color", &pOut->tColors.iSeparatorColor);
	(void)__xuiToolbarStyleColor(pWidget, "toolbar.text.color", &pOut->tColors.iTextColor);
	(void)__xuiToolbarStyleColor(pWidget, "toolbar.text.disabled_color", &pOut->tColors.iDisabledTextColor);
	(void)__xuiToolbarStyleColor(pWidget, "toolbar.icon.color", &pOut->tColors.iIconColor);
	pOut->iActiveTextColor = XUI_COLOR_WHITE;
	pOut->iActiveIconColor = XUI_COLOR_WHITE;
	pOut->iSeparatorHighlightColor = XUI_COLOR_RGBA(255, 255, 255, 112);
	(void)__xuiToolbarStyleColor(pWidget, "toolbar.text.active_color", &pOut->iActiveTextColor);
	(void)__xuiToolbarStyleColor(pWidget, "toolbar.icon.active_color", &pOut->iActiveIconColor);
	(void)__xuiToolbarStyleColor(pWidget, "toolbar.separator.highlight_color", &pOut->iSeparatorHighlightColor);

	if ( pOut->tMetrics.fItemWidth < 1.0f ) pOut->tMetrics.fItemWidth = 1.0f;
	if ( pOut->tMetrics.fItemHeight < 1.0f ) pOut->tMetrics.fItemHeight = 1.0f;
	if ( pOut->tMetrics.fSeparatorSize < 1.0f ) pOut->tMetrics.fSeparatorSize = 1.0f;
	if ( pOut->tMetrics.fOverflowSize < 1.0f ) pOut->tMetrics.fOverflowSize = 1.0f;
	pOut->pFont = __xuiToolbarStyleFont(pWidget, pOut->pFont);
}

static float __xuiToolbarItemSize(const xui_toolbar_item_t* pItem, const xui_toolbar_resolved_t* pResolved)
{
	if ( (pItem != NULL) && (pItem->iType == XUI_TOOLBAR_ITEM_SEPARATOR) ) {
		return pResolved->tMetrics.fSeparatorSize;
	}
	return (pResolved->tMetrics.iOrientation == XUI_ORIENTATION_VERTICAL) ?
		pResolved->tMetrics.fItemHeight : pResolved->tMetrics.fItemWidth;
}

static float __xuiToolbarItemsAxisSize(const xui_toolbar_data_t* pData, const xui_toolbar_resolved_t* pResolved)
{
	float fAxis;
	int iPrevGroup;
	int i;

	fAxis = 0.0f;
	iPrevGroup = 0;
	for ( i = 0; i < pData->iItemCount; i++ ) {
		if ( (i > 0) &&
		     (pData->arrItems[i].iType != XUI_TOOLBAR_ITEM_SEPARATOR) &&
		     (pData->arrItems[i].iGroup != iPrevGroup) ) {
			fAxis += pResolved->tMetrics.fGroupGap;
		}
		fAxis += __xuiToolbarMax(__xuiToolbarItemSize(&pData->arrItems[i], pResolved), 1.0f);
		if ( pData->arrItems[i].iType != XUI_TOOLBAR_ITEM_SEPARATOR ) {
			iPrevGroup = pData->arrItems[i].iGroup;
		}
	}
	return fAxis;
}

static int __xuiToolbarMeasureItems(xui_widget pWidget, xui_toolbar_data_t* pData, const xui_toolbar_resolved_t* pResolved, xui_vec2_t* pSize)
{
	float fAxis;
	float fCross;
	float fItem;
	int iPrevGroup;
	int i;

	(void)pWidget;
	if ( (pData == NULL) || (pResolved == NULL) || (pSize == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	fAxis = pResolved->tMetrics.fPaddingX * 2.0f;
	fCross = pResolved->tMetrics.fPaddingY * 2.0f;
	fCross += (pResolved->tMetrics.iOrientation == XUI_ORIENTATION_VERTICAL) ? pResolved->tMetrics.fItemWidth : pResolved->tMetrics.fItemHeight;
	iPrevGroup = 0;
	for ( i = 0; i < pData->iItemCount; i++ ) {
		if ( (i > 0) &&
		     (pData->arrItems[i].iType != XUI_TOOLBAR_ITEM_SEPARATOR) &&
		     (pData->arrItems[i].iGroup != iPrevGroup) ) {
			fAxis += pResolved->tMetrics.fGroupGap;
		}
		fItem = __xuiToolbarItemSize(&pData->arrItems[i], pResolved);
		fAxis += __xuiToolbarMax(fItem, 1.0f);
		if ( pData->arrItems[i].iType != XUI_TOOLBAR_ITEM_SEPARATOR ) {
			iPrevGroup = pData->arrItems[i].iGroup;
		}
	}
	if ( pResolved->tMetrics.iOrientation == XUI_ORIENTATION_VERTICAL ) {
		pSize->fX = xuiInternalSnapSize(fCross);
		pSize->fY = xuiInternalSnapSize(fAxis);
	} else {
		pSize->fX = xuiInternalSnapSize(fAxis);
		pSize->fY = xuiInternalSnapSize(fCross);
	}
	return XUI_OK;
}

static int __xuiToolbarLayoutItems(xui_widget pWidget, xui_toolbar_data_t* pData, const xui_toolbar_resolved_t* pResolved)
{
	xui_rect_t tContent;
	float fInnerX;
	float fInnerY;
	float fInnerW;
	float fInnerH;
	float fLimit;
	float fCursor;
	float fSize;
	float fVisibleSize;
	float fAxisSize;
	float fOverflowSize;
	int bVertical;
	int bNeedsOverflow;
	int iPrevGroup;
	int i;

	if ( (pWidget == NULL) || (pData == NULL) || (pResolved == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	tContent = xuiInternalSnapRect(xuiWidgetGetContentRect(pWidget));
	fInnerX = tContent.fX + pResolved->tMetrics.fPaddingX;
	fInnerY = tContent.fY + pResolved->tMetrics.fPaddingY;
	fInnerW = __xuiToolbarMax(0.0f, tContent.fW - pResolved->tMetrics.fPaddingX * 2.0f);
	fInnerH = __xuiToolbarMax(0.0f, tContent.fH - pResolved->tMetrics.fPaddingY * 2.0f);
	memset(&pData->tOverflowRect, 0, sizeof(pData->tOverflowRect));
	pData->iOverflowFirst = -1;
	pData->iOverflowCount = 0;
	for ( i = 0; i < pData->iItemCount; i++ ) {
		memset(&pData->arrItems[i].tRect, 0, sizeof(pData->arrItems[i].tRect));
	}
	bVertical = (pResolved->tMetrics.iOrientation == XUI_ORIENTATION_VERTICAL);
	fAxisSize = bVertical ? fInnerH : fInnerW;
	bNeedsOverflow = pData->bOverflowEnabled && pData->iItemCount > 0 &&
		__xuiToolbarItemsAxisSize(pData, pResolved) > fAxisSize;
	fOverflowSize = 0.0f;
	if ( bNeedsOverflow ) {
		fOverflowSize = pResolved->tMetrics.fOverflowSize;
		if ( fOverflowSize > fAxisSize ) fOverflowSize = fAxisSize;
	}
	fLimit = (bVertical ? (fInnerY + fInnerH) : (fInnerX + fInnerW)) - fOverflowSize;
	fCursor = bVertical ? fInnerY : fInnerX;
	iPrevGroup = 0;
	for ( i = 0; i < pData->iItemCount; i++ ) {
		if ( (i > 0) &&
		     (pData->arrItems[i].iType != XUI_TOOLBAR_ITEM_SEPARATOR) &&
		     (pData->arrItems[i].iGroup != iPrevGroup) ) {
			fCursor += pResolved->tMetrics.fGroupGap;
		}
		fSize = __xuiToolbarItemSize(&pData->arrItems[i], pResolved);
		if ( fSize < 1.0f ) fSize = 1.0f;
		if ( bNeedsOverflow && ((fCursor + fSize) > fLimit) ) {
			pData->iOverflowFirst = i;
			pData->iOverflowCount = pData->iItemCount - i;
			break;
		}
		fVisibleSize = (bVertical ? fInnerY + fInnerH : fInnerX + fInnerW) - fCursor;
		if ( fVisibleSize > fSize ) fVisibleSize = fSize;
		if ( fVisibleSize < 0.0f ) fVisibleSize = 0.0f;
		pData->arrItems[i].tRect = bVertical ?
			xuiInternalRectFromFloatNearest(fInnerX, fCursor, fInnerW, fVisibleSize) :
			xuiInternalRectFromFloatNearest(fCursor, fInnerY, fVisibleSize, fInnerH);
		fCursor += fSize;
		if ( pData->arrItems[i].iType != XUI_TOOLBAR_ITEM_SEPARATOR ) {
			iPrevGroup = pData->arrItems[i].iGroup;
		}
	}
	while ( pData->iOverflowFirst > 0 &&
	        pData->arrItems[pData->iOverflowFirst - 1].iType == XUI_TOOLBAR_ITEM_SEPARATOR ) {
		pData->iOverflowFirst--;
		pData->iOverflowCount++;
		memset(&pData->arrItems[pData->iOverflowFirst].tRect, 0,
			sizeof(pData->arrItems[pData->iOverflowFirst].tRect));
	}
	if ( pData->iOverflowCount > 0 ) {
		if ( bVertical ) {
			pData->tOverflowRect = xuiInternalRectFromFloatNearest(fInnerX,
				fInnerY + fInnerH - fOverflowSize, fInnerW, fOverflowSize);
		} else {
			pData->tOverflowRect = xuiInternalRectFromFloatNearest(fInnerX + fInnerW - fOverflowSize,
				fInnerY, fOverflowSize, fInnerH);
		}
	} else {
		pData->bOverflowFocused = 0;
		pData->bOverflowActive = 0;
	}
	return XUI_OK;
}

static int __xuiToolbarIndexAt(xui_toolbar_data_t* pData, float fX, float fY)
{
	int i;

	if ( pData == NULL ) return -1;
	for ( i = 0; i < pData->iItemCount; i++ ) {
		if ( __xuiToolbarItemInteractive(&pData->arrItems[i]) &&
		     __xuiToolbarRectContains(pData->arrItems[i].tRect, fX, fY) ) {
			return i;
		}
	}
	return -1;
}

static int __xuiToolbarItemVisible(const xui_toolbar_data_t* pData, int iIndex)
{
	return pData != NULL && iIndex >= 0 && iIndex < pData->iItemCount &&
	       pData->arrItems[iIndex].tRect.fW > 0.0f && pData->arrItems[iIndex].tRect.fH > 0.0f;
}

static int __xuiToolbarFirstVisibleEnabled(xui_toolbar_data_t* pData)
{
	int i;

	if ( pData == NULL ) return -1;
	for ( i = 0; i < pData->iItemCount; i++ ) {
		if ( __xuiToolbarItemVisible(pData, i) && __xuiToolbarItemInteractive(&pData->arrItems[i]) ) return i;
	}
	return -1;
}

static int __xuiToolbarLastVisibleEnabled(xui_toolbar_data_t* pData)
{
	int i;

	if ( pData == NULL ) return -1;
	for ( i = pData->iItemCount - 1; i >= 0; i-- ) {
		if ( __xuiToolbarItemVisible(pData, i) && __xuiToolbarItemInteractive(&pData->arrItems[i]) ) return i;
	}
	return -1;
}

static int __xuiToolbarNextEnabled(xui_toolbar_data_t* pData, int iCurrent, int iStep)
{
	int i;
	int iIndex;

	if ( (pData == NULL) || (pData->iItemCount <= 0) || (iStep == 0) ) return -1;
	iIndex = iCurrent;
	for ( i = 0; i < pData->iItemCount; i++ ) {
		iIndex += iStep;
		if ( iIndex < 0 ) iIndex = pData->iItemCount - 1;
		else if ( iIndex >= pData->iItemCount ) iIndex = 0;
		if ( __xuiToolbarItemVisible(pData, iIndex) && __xuiToolbarItemInteractive(&pData->arrItems[iIndex]) ) return iIndex;
	}
	return -1;
}

static int __xuiToolbarInvalidateItem(xui_widget pWidget, xui_toolbar_data_t* pData, int iIndex)
{
	xui_rect_t tRect;

	if ( (pWidget == NULL) || (pData == NULL) || (iIndex < 0) || (iIndex >= pData->iItemCount) ) {
		return XUI_OK;
	}
	tRect = pData->arrItems[iIndex].tRect;
	if ( (tRect.fW <= 0.0f) || (tRect.fH <= 0.0f) ) {
		return XUI_OK;
	}
	tRect.fX -= 2.0f;
	tRect.fY -= 2.0f;
	tRect.fW += 4.0f;
	tRect.fH += 4.0f;
	return xuiWidgetInvalidateRect(pWidget, tRect, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
}

static int __xuiToolbarInvalidateOverflow(xui_widget pWidget, xui_toolbar_data_t* pData)
{
	xui_rect_t tRect;

	if ( (pWidget == NULL) || (pData == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	tRect = pData->tOverflowRect;
	if ( (tRect.fW <= 0.0f) || (tRect.fH <= 0.0f) ) return XUI_OK;
	tRect.fX -= 2.0f;
	tRect.fY -= 2.0f;
	tRect.fW += 4.0f;
	tRect.fH += 4.0f;
	return xuiWidgetInvalidateRect(pWidget, tRect, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
}

static int __xuiToolbarSetTarget(xui_widget pWidget, xui_toolbar_data_t* pData, int iIndex, int bOverflow)
{
	int iOldHover;
	int bOldOverflow;
	int iRet;

	if ( (pWidget == NULL) || (pData == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	bOverflow = bOverflow && (pData->iOverflowCount > 0) &&
		(pData->tOverflowRect.fW > 0.0f) && (pData->tOverflowRect.fH > 0.0f);
	if ( (iIndex < 0) || (iIndex >= pData->iItemCount) || !__xuiToolbarItemInteractive(&pData->arrItems[iIndex]) ) {
		iIndex = -1;
	}
	if ( bOverflow ) iIndex = -1;
	if ( pData->iHover == iIndex && pData->bOverflowFocused == bOverflow ) return XUI_OK;
	iOldHover = pData->iHover;
	bOldOverflow = pData->bOverflowFocused;
	pData->iHover = iIndex;
	pData->bOverflowFocused = bOverflow;
	iRet = __xuiToolbarInvalidateItem(pWidget, pData, iOldHover);
	if ( iRet == XUI_OK ) {
		iRet = __xuiToolbarInvalidateItem(pWidget, pData, iIndex);
	}
	if ( iRet == XUI_OK && (bOldOverflow || bOverflow) ) {
		iRet = __xuiToolbarInvalidateOverflow(pWidget, pData);
	}
	if ( iRet != XUI_OK ) {
		pData->iHover = iOldHover;
		pData->bOverflowFocused = bOldOverflow;
		(void)__xuiToolbarInvalidateItem(pWidget, pData, iOldHover);
		(void)__xuiToolbarInvalidateItem(pWidget, pData, iIndex);
		if ( bOldOverflow || bOverflow ) (void)__xuiToolbarInvalidateOverflow(pWidget, pData);
		return iRet;
	}
	xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_SELECTION_CHANGED);
	return XUI_OK;
}

static int __xuiToolbarSetHover(xui_widget pWidget, xui_toolbar_data_t* pData, int iIndex)
{
	return __xuiToolbarSetTarget(pWidget, pData, iIndex, 0);
}

static int __xuiToolbarActivateOverflow(xui_widget pWidget, xui_toolbar_data_t* pData)
{
	xui_toolbar_overflow_proc onOverflow;
	void* pOverflowUser;
	int iOverflowFirst;
	int iOverflowCount;

	if ( (pWidget == NULL) || (pData == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pData->iOverflowCount <= 0 || pData->onOverflow == NULL ) return XUI_ERROR_UNSUPPORTED;
	onOverflow = pData->onOverflow;
	pOverflowUser = pData->pOverflowUser;
	iOverflowFirst = pData->iOverflowFirst;
	iOverflowCount = pData->iOverflowCount;
	pData->iOverflowSelectCount++;
	xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_SELECTION_CHANGED);
	onOverflow(pWidget, iOverflowFirst, iOverflowCount, pOverflowUser);
	return XUI_OK;
}

static int __xuiToolbarSelect(xui_widget pWidget, xui_toolbar_data_t* pData, int iIndex)
{
	xui_toolbar_select_proc onSelect;
	void* pSelectUser;
	int iValue;
	int iRet;
	int bToggle;

	if ( (pWidget == NULL) || (pData == NULL) || (iIndex < 0) || (iIndex >= pData->iItemCount) ||
	     !__xuiToolbarItemInteractive(&pData->arrItems[iIndex]) ) {
		return XUI_OK;
	}
	bToggle = pData->arrItems[iIndex].iType == XUI_TOOLBAR_ITEM_TOGGLE;
	if ( bToggle ) {
		pData->arrItems[iIndex].iState ^= XUI_TOOLBAR_ITEM_CHECKED;
		pData->iChangeCount++;
	}
	pData->iSelectCount++;
	iRet = xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	if ( iRet != XUI_OK ) {
		pData->iSelectCount--;
		if ( bToggle ) {
			pData->arrItems[iIndex].iState ^= XUI_TOOLBAR_ITEM_CHECKED;
			pData->iChangeCount--;
		}
		return iRet;
	}
	xuiInternalAccessibilityQueue(pWidget, bToggle ? XUI_ACCESSIBLE_EVENT_STATE_CHANGED : XUI_ACCESSIBLE_EVENT_SELECTION_CHANGED);
	onSelect = pData->onSelect;
	pSelectUser = pData->pSelectUser;
	iValue = pData->arrItems[iIndex].iValue;
	if ( onSelect != NULL ) onSelect(pWidget, iIndex, iValue, pSelectUser);
	return XUI_OK;
}

static int __xuiToolbarDrawRectFill(xui_proxy pProxy, xui_draw_context pDraw, xui_rect_t tRect, uint32_t iColor)
{
	if ( __xuiToolbarAlpha(iColor) == 0 ) return XUI_OK;
	return pProxy->drawRectFill(pProxy, pDraw, tRect, iColor);
}

static int __xuiToolbarDrawRectStroke(xui_proxy pProxy, xui_draw_context pDraw, xui_rect_t tRect, float fWidth, uint32_t iColor)
{
	if ( (fWidth <= 0.0f) || (__xuiToolbarAlpha(iColor) == 0) ) return XUI_OK;
	return pProxy->drawRectStroke(pProxy, pDraw, tRect, fWidth, iColor);
}

static xui_rect_t __xuiToolbarInsetRect(xui_rect_t tRect, float fLeft, float fTop, float fRight, float fBottom)
{
	tRect.fX += fLeft;
	tRect.fY += fTop;
	tRect.fW -= fLeft + fRight;
	tRect.fH -= fTop + fBottom;
	if ( tRect.fW < 0.0f ) tRect.fW = 0.0f;
	if ( tRect.fH < 0.0f ) tRect.fH = 0.0f;
	return tRect;
}

static int __xuiToolbarDrawSeparator(xui_proxy pProxy, xui_draw_context pDraw, xui_rect_t tRect, int bVerticalToolbar, uint32_t iColor, uint32_t iHighlight)
{
	xui_rect_t tLine;
	int iRet;

	if ( __xuiToolbarAlpha(iColor) == 0 ) return XUI_OK;
	if ( bVerticalToolbar ) {
		tLine = xuiInternalRectFromFloatNearest(tRect.fX + 6.0f,
			tRect.fY + (tRect.fH - 1.0f) * 0.5f, tRect.fW - 12.0f, 1.0f);
	} else {
		tLine = xuiInternalRectFromFloatNearest(tRect.fX + (tRect.fW - 1.0f) * 0.5f,
			tRect.fY + 5.0f, 1.0f, tRect.fH - 10.0f);
	}
	if ( (tLine.fW <= 0.0f) || (tLine.fH <= 0.0f) ) return XUI_OK;
	iRet = pProxy->drawRectFill(pProxy, pDraw, tLine, iColor);
	if ( iRet != XUI_OK ) return iRet;
	if ( bVerticalToolbar ) {
		tLine.fY += 1.0f;
	} else {
		tLine.fX += 1.0f;
	}
	return __xuiToolbarDrawRectFill(pProxy, pDraw, tLine, iHighlight);
}

static int __xuiToolbarDrawOverflowDots(xui_proxy pProxy, xui_draw_context pDraw, xui_rect_t tRect, uint32_t iColor)
{
	xui_rect_t tDot;
	int i;
	int iRet;

	tDot = xuiInternalRectFromFloatNearest(tRect.fX + (tRect.fW - 10.0f) * 0.5f,
		tRect.fY + (tRect.fH - 2.0f) * 0.5f, 2.0f, 2.0f);
	for ( i = 0; i < 3; i++ ) {
		iRet = __xuiToolbarDrawRectFill(pProxy, pDraw, tDot, iColor);
		if ( iRet != XUI_OK ) return iRet;
		tDot.fX += 4.0f;
	}
	return XUI_OK;
}

static int __xuiToolbarDrawItemContent(xui_widget pWidget, xui_proxy pProxy, xui_draw_context pDraw, const xui_toolbar_item_t* pItem, xui_rect_t tRect, const xui_toolbar_resolved_t* pResolved, uint32_t iTextColor, uint32_t iIconColor, int bPressed)
{
	xui_rect_t tIcon;
	xui_rect_t tText;
	xui_vec2_t tTextSize;
	float fIconSize;
	float fIconX;
	float fIconY;
	float fTextX;
	float fGroupW;
	int bHasIcon;
	int bHasText;
	int iRet;

	bHasIcon = (pItem->pIcon != NULL) && (pResolved->tMetrics.fIconSize > 0.0f);
	bHasText = (pItem->sText != NULL) && (pItem->sText[0] != '\0');
	if ( !bHasIcon && !bHasText ) return XUI_OK;
	tText = __xuiToolbarInsetRect(tRect, 6.0f, 0.0f, 6.0f, 0.0f);
	if ( bPressed ) tText.fY += 1.0f;
	if ( bHasIcon ) {
		fIconSize = pResolved->tMetrics.fIconSize;
		if ( fIconSize > tText.fH ) fIconSize = tText.fH;
		fIconY = tText.fY + (tText.fH - fIconSize) * 0.5f;
		if ( !bHasText ) {
			fIconX = tText.fX + (tText.fW - fIconSize) * 0.5f;
		} else {
			tTextSize = __xuiToolbarMeasureText(pWidget, pResolved->pFont, pItem->sText);
			fGroupW = fIconSize + pResolved->tMetrics.fIconGap + tTextSize.fX;
			if ( fGroupW > tText.fW ) fGroupW = tText.fW;
			fIconX = tText.fX + (tText.fW - fGroupW) * 0.5f;
			fTextX = fIconX + fIconSize + pResolved->tMetrics.fIconGap;
			tText = xuiInternalRectFromFloatNearest(fTextX, tText.fY,
				tRect.fX + tRect.fW - 6.0f - fTextX, tText.fH);
		}
		tIcon = xuiInternalRectFromFloatNearest(fIconX, fIconY, fIconSize, fIconSize);
		if ( pProxy->drawSurface != NULL ) {
			iRet = pProxy->drawSurface(pProxy, pDraw, pItem->pIcon, pItem->tIconSrc, tIcon, iIconColor, 0);
			if ( iRet != XUI_OK ) return iRet;
		}
	}
	if ( bHasText && (pResolved->pFont != NULL) && (pProxy->drawText != NULL) && (tText.fW > 0.0f) ) {
		return pProxy->drawText(pProxy, pDraw, &(xui_text_item_t){.iSize=sizeof(xui_text_item_t), .pFont=pResolved->pFont, .sText=pItem->sText, .iTextSize=-1, .iFlags=XUI_TEXT_SHAPE_DEFAULT | (((bHasIcon ? XUI_TEXT_ALIGN_LEFT : XUI_TEXT_ALIGN_CENTER) | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP) & XUI_TEXT_RTL ? XUI_TEXT_SHAPE_RTL : 0)}, tText, iTextColor, (bHasIcon ? XUI_TEXT_ALIGN_LEFT : XUI_TEXT_ALIGN_CENTER) | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP);
	}
	return XUI_OK;
}

static int __xuiToolbarCacheRender(xui_widget pWidget, xui_draw_context pDraw, uint32_t iStateId, void* pUser)
{
	xui_toolbar_data_t* pData;
	xui_toolbar_resolved_t tResolved;
	xui_proxy pProxy;
	xui_rect_t tRect;
	xui_rect_t tDraw;
	xui_rect_t tFocus;
	uint32_t iFill;
	uint32_t iBorder;
	uint32_t iText;
	uint32_t iIcon;
	int bVertical;
	int bPressed;
	int i;
	int iRet;

	(void)iStateId;
	(void)pUser;
	pData = __xuiToolbarGetData(pWidget);
	if ( (pData == NULL) || (pDraw == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	__xuiToolbarResolve(pWidget, pData, &tResolved);
	(void)__xuiToolbarLayoutItems(pWidget, pData, &tResolved);
	pProxy = xuiInternalContextGetProxy(xuiWidgetGetContext(pWidget));
	if ( pProxy == NULL ) return XUI_ERROR_NOT_INITIALIZED;
	tRect = xuiWidgetGetRect(pWidget);
	tRect.fX = 0.0f;
	tRect.fY = 0.0f;
	iRet = __xuiToolbarDrawRectFill(pProxy, pDraw, tRect, tResolved.tColors.iBackgroundColor);
	if ( iRet != XUI_OK ) return iRet;
	if ( tResolved.tMetrics.fBorderWidth > 0.0f ) {
		iRet = __xuiToolbarDrawRectStroke(pProxy, pDraw, tRect, tResolved.tMetrics.fBorderWidth, tResolved.tColors.iBorderColor);
		if ( iRet != XUI_OK ) return iRet;
	}
	bVertical = (tResolved.tMetrics.iOrientation == XUI_ORIENTATION_VERTICAL);
	for ( i = 0; i < pData->iItemCount; i++ ) {
		if ( (pData->arrItems[i].tRect.fW <= 0.0f) || (pData->arrItems[i].tRect.fH <= 0.0f) ) continue;
		if ( pData->arrItems[i].iType == XUI_TOOLBAR_ITEM_SEPARATOR ) {
			iRet = __xuiToolbarDrawSeparator(pProxy, pDraw, pData->arrItems[i].tRect, bVertical, tResolved.tColors.iSeparatorColor, tResolved.iSeparatorHighlightColor);
			if ( iRet != XUI_OK ) return iRet;
			continue;
		}
		iFill = tResolved.tColors.iItemColor;
		iBorder = XUI_COLOR_RGBA(0, 0, 0, 0);
		iText = tResolved.tColors.iTextColor;
		iIcon = tResolved.tColors.iIconColor;
		bPressed = 0;
		if ( !__xuiToolbarItemInteractive(&pData->arrItems[i]) ) {
			iFill = tResolved.tColors.iDisabledColor;
			iText = tResolved.tColors.iDisabledTextColor;
			iIcon = tResolved.tColors.iDisabledTextColor;
			iBorder = __xuiToolbarColorAlpha(tResolved.tColors.iBorderColor, 120);
		} else if ( i == pData->iActive ) {
			iFill = tResolved.tColors.iActiveColor;
			iText = tResolved.iActiveTextColor;
			iIcon = tResolved.iActiveIconColor;
			iBorder = __xuiToolbarColorAlpha(tResolved.tColors.iActiveColor, 235);
			bPressed = 1;
		} else if ( (pData->arrItems[i].iState & XUI_TOOLBAR_ITEM_CHECKED) != 0 ) {
			iFill = tResolved.tColors.iCheckedColor;
			iBorder = __xuiToolbarColorAlpha(tResolved.tColors.iFocusColor, 120);
		} else if ( i == pData->iHover ) {
			iFill = tResolved.tColors.iHoverColor;
			iBorder = __xuiToolbarColorAlpha(tResolved.tColors.iFocusColor, 160);
		}
		tDraw = bVertical ?
			__xuiToolbarInsetRect(pData->arrItems[i].tRect, 3.0f, 2.0f, 3.0f, 2.0f) :
			__xuiToolbarInsetRect(pData->arrItems[i].tRect, 2.0f, 3.0f, 2.0f, 3.0f);
		if ( bPressed ) tDraw.fY += 1.0f;
		iRet = __xuiToolbarDrawRectFill(pProxy, pDraw, xuiInternalSnapRect(tDraw), iFill);
		if ( iRet != XUI_OK ) return iRet;
		iRet = __xuiToolbarDrawRectStroke(pProxy, pDraw, xuiInternalSnapRect(tDraw), tResolved.tMetrics.fBorderWidth, iBorder);
		if ( iRet != XUI_OK ) return iRet;
		if ( (xuiGetFocusWidget(xuiWidgetGetContext(pWidget)) == pWidget) && (i == pData->iHover) ) {
			tFocus = tDraw;
			tFocus.fX += 5.0f;
			tFocus.fY = tDraw.fY + tDraw.fH - 3.0f;
			tFocus.fW -= 10.0f;
			tFocus.fH = 2.0f;
			if ( tFocus.fW > 0.0f ) {
				iRet = __xuiToolbarDrawRectFill(pProxy, pDraw, xuiInternalSnapRect(tFocus), tResolved.tColors.iFocusColor);
				if ( iRet != XUI_OK ) return iRet;
			}
		}
		iRet = __xuiToolbarDrawItemContent(pWidget, pProxy, pDraw, &pData->arrItems[i], pData->arrItems[i].tRect, &tResolved, iText, iIcon, bPressed);
		if ( iRet != XUI_OK ) return iRet;
	}
	if ( (pData->iOverflowCount > 0) && (pData->tOverflowRect.fW > 0.0f) && (pData->tOverflowRect.fH > 0.0f) ) {
		tDraw = bVertical ?
			__xuiToolbarInsetRect(pData->tOverflowRect, 3.0f, 2.0f, 3.0f, 2.0f) :
			__xuiToolbarInsetRect(pData->tOverflowRect, 2.0f, 3.0f, 2.0f, 3.0f);
		if ( pData->bOverflowActive ) tDraw.fY += 1.0f;
		iFill = tResolved.tColors.iItemColor;
		iBorder = XUI_COLOR_RGBA(0, 0, 0, 0);
		iText = pData->bOverflowActive ? tResolved.iActiveTextColor : tResolved.tColors.iTextColor;
		if ( pData->bOverflowActive ) {
			iFill = tResolved.tColors.iActiveColor;
			iBorder = __xuiToolbarColorAlpha(tResolved.tColors.iActiveColor, 235);
		} else if ( pData->bOverflowFocused ) {
			iFill = tResolved.tColors.iHoverColor;
			iBorder = __xuiToolbarColorAlpha(tResolved.tColors.iFocusColor, 160);
		}
		iRet = __xuiToolbarDrawRectFill(pProxy, pDraw, xuiInternalSnapRect(tDraw), iFill);
		if ( iRet != XUI_OK ) return iRet;
		iRet = __xuiToolbarDrawRectStroke(pProxy, pDraw, xuiInternalSnapRect(tDraw), tResolved.tMetrics.fBorderWidth, iBorder);
		if ( iRet != XUI_OK ) return iRet;
		if ( (xuiGetFocusWidget(xuiWidgetGetContext(pWidget)) == pWidget) && pData->bOverflowFocused ) {
			tFocus = tDraw;
			tFocus.fX += 5.0f;
			tFocus.fY = tDraw.fY + tDraw.fH - 3.0f;
			tFocus.fW -= 10.0f;
			tFocus.fH = 2.0f;
			if ( tFocus.fW > 0.0f ) {
				iRet = __xuiToolbarDrawRectFill(pProxy, pDraw, xuiInternalSnapRect(tFocus), tResolved.tColors.iFocusColor);
				if ( iRet != XUI_OK ) return iRet;
			}
		}
		iRet = __xuiToolbarDrawOverflowDots(pProxy, pDraw, tDraw, iText);
		if ( iRet != XUI_OK ) return iRet;
	}
	return XUI_OK;
}

static int __xuiToolbarContentMeasure(xui_widget pWidget, xui_vec2_t tConstraint, xui_vec2_t* pSize, void* pUser)
{
	xui_toolbar_data_t* pData;
	xui_toolbar_resolved_t tResolved;

	(void)tConstraint;
	(void)pUser;
	if ( (pWidget == NULL) || (pSize == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	pData = __xuiToolbarGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	__xuiToolbarResolve(pWidget, pData, &tResolved);
	return __xuiToolbarMeasureItems(pWidget, pData, &tResolved, pSize);
}

static int __xuiToolbarTooltipResolve(xui_context pContext, xui_widget pWidget, xui_tooltip_desc_t* pDesc, void* pUser)
{
	xui_toolbar_data_t* pData;
	xui_toolbar_resolved_t tResolved;
	xui_rect_t tWorld;
	const char* sText;
	int iIndex;

	(void)pUser;
	if ( (pContext == NULL) || (pWidget == NULL) || (pDesc == NULL) ) return 0;
	pData = __xuiToolbarGetData(pWidget);
	if ( pData == NULL ) return 0;
	__xuiToolbarResolve(pWidget, pData, &tResolved);
	(void)__xuiToolbarLayoutItems(pWidget, pData, &tResolved);
	tWorld = xuiWidgetGetWorldRect(pWidget);
	iIndex = __xuiToolbarIndexAt(pData, pContext->fTooltipMouseX - tWorld.fX, pContext->fTooltipMouseY - tWorld.fY);
	if ( iIndex < 0 ) return 0;
	sText = pData->arrItems[iIndex].sTooltip;
	if ( (sText == NULL) || (sText[0] == '\0') ) return 0;
	memset(pDesc, 0, sizeof(*pDesc));
	pDesc->iSize = sizeof(*pDesc);
	pDesc->iType = XUI_TOOLTIP_TEXT;
	pDesc->sText = sText;
	pDesc->iAnchor = XUI_TOOLTIP_ANCHOR_CURSOR;
	pDesc->fOffsetX = 12.0f;
	pDesc->fOffsetY = 16.0f;
	pDesc->bFollowCursor = 1;
	return 1;
}

static int __xuiToolbarEvent(xui_widget pWidget, const xui_event_t* pEvent, void* pUser)
{
	xui_toolbar_data_t* pData;
	xui_toolbar_resolved_t tResolved;
	xui_context pContext;
	xui_rect_t tWorld;
	float fLocalX;
	float fLocalY;
	int iIndex;
	int iNext;
	int iRet;
	int bLeftButton;
	int bOverflowHit;

	(void)pUser;
	if ( (pWidget == NULL) || (pEvent == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pEvent->iPhase == XUI_EVENT_PHASE_CAPTURE ) return XUI_OK;
	pData = __xuiToolbarGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	__xuiToolbarResolve(pWidget, pData, &tResolved);
	(void)__xuiToolbarLayoutItems(pWidget, pData, &tResolved);
	pContext = xuiWidgetGetContext(pWidget);
	tWorld = xuiWidgetGetWorldRect(pWidget);
	fLocalX = pEvent->fX - tWorld.fX;
	fLocalY = pEvent->fY - tWorld.fY;
	iIndex = __xuiToolbarIndexAt(pData, fLocalX, fLocalY);
	bOverflowHit = (pData->iOverflowCount > 0) && __xuiToolbarRectContains(pData->tOverflowRect, fLocalX, fLocalY);
	bLeftButton = (pEvent->iButton == 0) || (pEvent->iButton == XUI_POINTER_BUTTON_LEFT);
	switch ( pEvent->iType ) {
	case XUI_EVENT_POINTER_MOVE:
		if ( pData->tPointerOwner.bActive &&
		     !xuiInternalPointerOwnerMatches(&pData->tPointerOwner, pEvent) ) {
			return XUI_OK;
		}
		if ( pData->bOverflowActive ) {
			return XUI_EVENT_DISPATCH_STOP;
		}
		iRet = __xuiToolbarSetTarget(pWidget, pData, iIndex, bOverflowHit);
		if ( iRet != XUI_OK ) return iRet;
		return (pData->iActive >= 0) ? XUI_EVENT_DISPATCH_STOP : XUI_OK;
	case XUI_EVENT_POINTER_LEAVE:
		if ( (pData->iActive < 0) && !pData->bOverflowActive ) {
			return __xuiToolbarSetTarget(pWidget, pData, -1, 0);
		}
		return XUI_OK;
	case XUI_EVENT_POINTER_DOWN:
		if ( !bLeftButton ) return XUI_OK;
		if ( pData->tPointerOwner.bActive ) {
			return xuiInternalPointerOwnerMatches(&pData->tPointerOwner, pEvent) ?
				XUI_EVENT_DISPATCH_STOP : XUI_OK;
		}
		if ( bOverflowHit ) {
			iRet = xuiSetFocusWidget(pContext, pWidget);
			if ( iRet != XUI_OK ) return iRet;
			if ( __xuiToolbarGetData(pWidget) != pData ) return XUI_EVENT_DISPATCH_STOP;
			iRet = __xuiToolbarSetTarget(pWidget, pData, -1, 1);
			if ( iRet != XUI_OK ) return iRet;
			iRet = xuiInternalPointerOwnerBegin(pContext, pWidget, pEvent,
				&pData->tPointerOwner);
			if ( iRet != XUI_OK ) return iRet;
			pData->iActive = -1;
			pData->bOverflowActive = 1;
			iRet = xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
			if ( iRet != XUI_OK ) {
				(void)__xuiToolbarCancelPress(pWidget, pData);
				return iRet;
			}
			return XUI_EVENT_DISPATCH_STOP;
		}
		if ( iIndex < 0 ) return XUI_OK;
		iRet = xuiSetFocusWidget(pContext, pWidget);
		if ( iRet != XUI_OK ) return iRet;
		if ( __xuiToolbarGetData(pWidget) != pData ) return XUI_EVENT_DISPATCH_STOP;
		iRet = __xuiToolbarSetTarget(pWidget, pData, iIndex, 0);
		if ( iRet != XUI_OK ) return iRet;
		iRet = xuiInternalPointerOwnerBegin(pContext, pWidget, pEvent,
			&pData->tPointerOwner);
		if ( iRet != XUI_OK ) return iRet;
		pData->iActive = iIndex;
		pData->bOverflowActive = 0;
		iRet = xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
		if ( iRet != XUI_OK ) {
			(void)__xuiToolbarCancelPress(pWidget, pData);
			return iRet;
		}
		return XUI_EVENT_DISPATCH_STOP;
	case XUI_EVENT_POINTER_UP: {
		int iActivated;
		int bWasOverflowActive;

		if ( !xuiInternalPointerOwnerMatches(&pData->tPointerOwner, pEvent) ) {
			return XUI_OK;
		}
		iActivated = pData->iActive;
		bWasOverflowActive = pData->bOverflowActive;
		pData->iActive = -1;
		pData->bOverflowActive = 0;
		iRet = xuiInternalPointerOwnerEnd(pContext, pWidget, pEvent,
			&pData->tPointerOwner);
		if ( iRet != XUI_OK ) return iRet;
		if ( __xuiToolbarGetData(pWidget) != pData ) return XUI_EVENT_DISPATCH_STOP;
		iRet = __xuiToolbarSetTarget(pWidget, pData, iIndex, bOverflowHit);
		if ( iRet != XUI_OK ) return iRet;
		iRet = xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
		if ( iRet != XUI_OK ) return iRet;
		if ( bWasOverflowActive ) {
			if ( bOverflowHit && pData->onOverflow != NULL ) return __xuiToolbarActivateOverflow(pWidget, pData);
			return XUI_EVENT_DISPATCH_STOP;
		}
		if ( iIndex == iActivated ) {
			iRet = __xuiToolbarSelect(pWidget, pData, iIndex);
			if ( iRet != XUI_OK ) return iRet;
		}
		return XUI_EVENT_DISPATCH_STOP;
	}
	case XUI_EVENT_POINTER_CAPTURE_LOST:
		if ( !xuiInternalPointerOwnerCaptureLost(&pData->tPointerOwner, pEvent) ) {
			return XUI_OK;
		}
		pData->iActive = -1;
		pData->bOverflowActive = 0;
		return xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	case XUI_EVENT_CONTEXT_MENU: {
		xui_rect_t tAnchor = tWorld;
		float fX;
		float fY;
		if ( pData->onContext == NULL ) break;
		if ( pEvent->iKey == XUI_KEY_CONTEXT_MENU ) {
			if ( pData->bOverflowFocused && pData->iOverflowCount > 0 ) {
				iIndex = -1;
				tAnchor = pData->tOverflowRect;
				tAnchor.fX += tWorld.fX;
				tAnchor.fY += tWorld.fY;
			} else {
				iIndex = (pData->iHover >= 0 && __xuiToolbarItemVisible(pData, pData->iHover)) ?
					pData->iHover : __xuiToolbarFirstVisibleEnabled(pData);
			}
			if ( iIndex >= 0 ) {
				tAnchor = pData->arrItems[iIndex].tRect;
				tAnchor.fX += tWorld.fX;
				tAnchor.fY += tWorld.fY;
			}
		} else if ( iIndex >= 0 ) {
			iRet = __xuiToolbarSetHover(pWidget, pData, iIndex);
			if ( iRet != XUI_OK ) return iRet;
		}
		xuiInternalContextMenuPoint(pEvent, tAnchor, &fX, &fY);
		return pData->onContext(pWidget, iIndex,
			(iIndex >= 0) ? pData->arrItems[iIndex].iValue : 0,
			fX, fY, pData->pContextUser);
	}
	case XUI_EVENT_KEY_DOWN:
		if ( xuiGetFocusWidget(pContext) != pWidget ) return XUI_OK;
		if ( ((tResolved.tMetrics.iOrientation == XUI_ORIENTATION_HORIZONTAL) && (pEvent->iKey == XUI_KEY_LEFT || pEvent->iKey == XUI_KEY_RIGHT)) ||
		     ((tResolved.tMetrics.iOrientation == XUI_ORIENTATION_VERTICAL) && (pEvent->iKey == XUI_KEY_UP || pEvent->iKey == XUI_KEY_DOWN)) ) {
			int iStep = (pEvent->iKey == XUI_KEY_RIGHT || pEvent->iKey == XUI_KEY_DOWN) ? 1 : -1;
			int iFirst = __xuiToolbarFirstVisibleEnabled(pData);
			int iLast = __xuiToolbarLastVisibleEnabled(pData);
			if ( pData->bOverflowFocused ) {
				iNext = (iStep > 0) ? iFirst : iLast;
				iRet = __xuiToolbarSetTarget(pWidget, pData, iNext, 0);
			} else if ( pData->iOverflowCount > 0 &&
			            ((iStep > 0 && pData->iHover == iLast) || (iStep < 0 && pData->iHover == iFirst)) ) {
				iRet = __xuiToolbarSetTarget(pWidget, pData, -1, 1);
			} else {
				iNext = __xuiToolbarNextEnabled(pData, pData->iHover, iStep);
				iRet = (iNext >= 0) ? __xuiToolbarSetTarget(pWidget, pData, iNext, 0) : XUI_OK;
			}
			if ( iRet != XUI_OK ) return iRet;
			return XUI_EVENT_DISPATCH_STOP;
		}
		if ( pEvent->iKey == XUI_KEY_HOME || pEvent->iKey == XUI_KEY_END ) {
			if ( pEvent->iKey == XUI_KEY_END && pData->iOverflowCount > 0 ) {
				iRet = __xuiToolbarSetTarget(pWidget, pData, -1, 1);
			} else {
				iNext = pEvent->iKey == XUI_KEY_HOME ?
					__xuiToolbarFirstVisibleEnabled(pData) : __xuiToolbarLastVisibleEnabled(pData);
				iRet = (iNext >= 0) ? __xuiToolbarSetTarget(pWidget, pData, iNext, 0) : XUI_OK;
			}
			if ( iRet != XUI_OK ) return iRet;
			return XUI_EVENT_DISPATCH_STOP;
		}
		if ( (pEvent->iKey == XUI_KEY_ENTER) || (pEvent->iKey == XUI_KEY_SPACE) ) {
			if ( pData->bOverflowFocused ) {
				if ( pData->onOverflow != NULL ) return __xuiToolbarActivateOverflow(pWidget, pData);
				return XUI_EVENT_DISPATCH_STOP;
			}
			iIndex = ((pData->iHover >= 0) && (pData->iHover < pData->iItemCount) &&
				__xuiToolbarItemVisible(pData, pData->iHover) && __xuiToolbarItemInteractive(&pData->arrItems[pData->iHover])) ?
				pData->iHover : __xuiToolbarFirstVisibleEnabled(pData);
			if ( iIndex >= 0 ) {
				iRet = __xuiToolbarSetTarget(pWidget, pData, iIndex, 0);
				if ( iRet != XUI_OK ) return iRet;
				iRet = __xuiToolbarSelect(pWidget, pData, iIndex);
				if ( iRet != XUI_OK ) return iRet;
				return XUI_EVENT_DISPATCH_STOP;
			}
		}
		break;
	case XUI_EVENT_FOCUS:
	case XUI_EVENT_BLUR:
		xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_STATE_CHANGED);
		return xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	default:
		break;
	}
	return XUI_OK;
}

static void __xuiToolbarDefaultLayout(xui_layout_t* pLayout)
{
	memset(pLayout, 0, sizeof(*pLayout));
	pLayout->iLayoutType = XUI_LAYOUT_MANUAL;
	pLayout->iWidthMode = XUI_SIZE_CONTENT;
	pLayout->iHeightMode = XUI_SIZE_CONTENT;
	pLayout->iFlowMode = XUI_FLOW_BLOCK;
	pLayout->iDock = XUI_DOCK_TOP;
	pLayout->iOverflow = XUI_OVERFLOW_VISIBLE;
	pLayout->iAlignX = XUI_ALIGN_STRETCH;
	pLayout->iAlignY = XUI_ALIGN_START;
	pLayout->iTableRowSpan = 1;
	pLayout->iTableColumnSpan = 1;
	pLayout->iGridColumnCount = 1;
	pLayout->fMaxWidth = XUI_LAYOUT_UNBOUNDED;
	pLayout->fMaxHeight = XUI_LAYOUT_UNBOUNDED;
	pLayout->fShrink = 1.0f;
}

static void __xuiToolbarDefaultCachePolicy(xui_cache_policy_t* pPolicy)
{
	memset(pPolicy, 0, sizeof(*pPolicy));
	pPolicy->iSize = sizeof(*pPolicy);
	pPolicy->iPolicy = XUI_CACHE_POLICY_SELF;
	pPolicy->iFlags = XUI_CACHE_CLEAR_ON_UPDATE | XUI_CACHE_UPDATE_ALL_STATES;
	pPolicy->iClearColor = XUI_COLOR_RGBA(0, 0, 0, 0);
}

static int __xuiToolbarInitEvents(xui_widget pWidget)
{
	int iRet;

	iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_POINTER_MOVE, __xuiToolbarEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_POINTER_LEAVE, __xuiToolbarEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_POINTER_DOWN, __xuiToolbarEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_POINTER_UP, __xuiToolbarEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_POINTER_CAPTURE_LOST, __xuiToolbarEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_KEY_DOWN, __xuiToolbarEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_FOCUS, __xuiToolbarEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_BLUR, __xuiToolbarEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_CONTEXT_MENU, __xuiToolbarEvent, NULL);
	return iRet;
}

static int __xuiToolbarAccessibleCount(xui_widget pWidget, void* pUser)
{
	xui_toolbar_data_t* pData = (xui_toolbar_data_t*)pUser;
	xui_toolbar_resolved_t tResolved;

	if ( pData == NULL ) return 0;
	__xuiToolbarResolve(pWidget, pData, &tResolved);
	(void)__xuiToolbarLayoutItems(pWidget, pData, &tResolved);
	return pData->iItemCount + 1 + (pData->iOverflowCount > 0 ? 1 : 0);
}

static int __xuiToolbarAccessibleGet(xui_widget pWidget, int iIndex,
	xui_accessible_node_t* pNode, void* pUser)
{
	xui_toolbar_data_t* pData = (xui_toolbar_data_t*)pUser;
	xui_toolbar_resolved_t tResolved;
	xui_toolbar_item_t* pItem;
	xui_rect_t tWorld;
	xui_rect_t tItem;
	int iItem;
	int bOverflowNode;

	if ( pData == NULL || pNode == NULL || iIndex < 0 ) return XUI_ERROR_INVALID_ARGUMENT;
	__xuiToolbarResolve(pWidget, pData, &tResolved);
	(void)__xuiToolbarLayoutItems(pWidget, pData, &tResolved);
	bOverflowNode = pData->iOverflowCount > 0 && iIndex == pData->iItemCount + 1;
	if ( iIndex > pData->iItemCount && !bOverflowNode ) return XUI_ERROR_INVALID_ARGUMENT;
	tWorld = xuiWidgetGetWorldRect(pWidget);
	if ( iIndex == 0 ) {
		pNode->iId = 1;
		pNode->iRole = XUI_ACCESSIBLE_ROLE_TOOLBAR;
		pNode->sName = xuiWidgetGetAccessibleName(pWidget) != NULL ?
			xuiWidgetGetAccessibleName(pWidget) : "Toolbar";
		pNode->sDescription = xuiWidgetGetAccessibleDescription(pWidget);
		pNode->tBounds = tWorld;
		pNode->iState |= XUI_ACCESSIBLE_STATE_FOCUSABLE;
		if ( xuiGetFocusWidget(xuiWidgetGetContext(pWidget)) == pWidget ) {
			pNode->iState |= XUI_ACCESSIBLE_STATE_FOCUSED;
		}
		if ( tResolved.tMetrics.iOrientation == XUI_ORIENTATION_VERTICAL ) {
			pNode->iState |= XUI_ACCESSIBLE_STATE_VERTICAL;
			pNode->iRowCount = pData->iItemCount + (pData->iOverflowCount > 0 ? 1 : 0);
			pNode->iColumnCount = 1;
		} else {
			pNode->iRowCount = 1;
			pNode->iColumnCount = pData->iItemCount + (pData->iOverflowCount > 0 ? 1 : 0);
		}
		pNode->iActions = XUI_ACCESSIBLE_ACTION_MASK(XUI_ACCESSIBLE_ACTION_FOCUS);
		return XUI_OK;
	}
	if ( bOverflowNode ) {
		pNode->iId = XUI_TOOLBAR_ACCESSIBLE_OVERFLOW_ID;
		pNode->iParentId = 1;
		pNode->iRole = XUI_ACCESSIBLE_ROLE_BUTTON;
		pNode->sName = "More commands";
		tItem = pData->tOverflowRect;
		pNode->tBounds = (xui_rect_t){tWorld.fX + tItem.fX, tWorld.fY + tItem.fY, tItem.fW, tItem.fH};
		pNode->iRow = tResolved.tMetrics.iOrientation == XUI_ORIENTATION_VERTICAL ? pData->iItemCount : 0;
		pNode->iColumn = tResolved.tMetrics.iOrientation == XUI_ORIENTATION_VERTICAL ? 0 : pData->iItemCount;
		pNode->iRowCount = 1;
		pNode->iColumnCount = 1;
		pNode->iState |= XUI_ACCESSIBLE_STATE_FOCUSABLE;
		if ( pData->bOverflowFocused ) {
			pNode->iState |= XUI_ACCESSIBLE_STATE_SELECTED;
			if ( xuiGetFocusWidget(xuiWidgetGetContext(pWidget)) == pWidget ) {
				pNode->iState |= XUI_ACCESSIBLE_STATE_FOCUSED;
			}
		}
		pNode->iActions = XUI_ACCESSIBLE_ACTION_MASK(XUI_ACCESSIBLE_ACTION_FOCUS);
		if ( pData->onOverflow != NULL ) {
			pNode->iActions |= XUI_ACCESSIBLE_ACTION_MASK(XUI_ACCESSIBLE_ACTION_ACTIVATE);
		}
		return XUI_OK;
	}
	iItem = iIndex - 1;
	pItem = &pData->arrItems[iItem];
	pNode->iId = (uint64_t)iItem + 2u;
	pNode->iParentId = 1;
	pNode->iRole = pItem->iType == XUI_TOOLBAR_ITEM_SEPARATOR ?
		XUI_ACCESSIBLE_ROLE_SEPARATOR : XUI_ACCESSIBLE_ROLE_BUTTON;
	pNode->sName = pItem->sText != NULL ? pItem->sText : "";
	pNode->sDescription = pItem->sTooltip;
	tItem = pItem->tRect;
	pNode->tBounds = (xui_rect_t){tWorld.fX + tItem.fX, tWorld.fY + tItem.fY, tItem.fW, tItem.fH};
	pNode->iRow = tResolved.tMetrics.iOrientation == XUI_ORIENTATION_VERTICAL ? iItem : 0;
	pNode->iColumn = tResolved.tMetrics.iOrientation == XUI_ORIENTATION_VERTICAL ? 0 : iItem;
	pNode->iRowCount = 1;
	pNode->iColumnCount = 1;
	if ( !__xuiToolbarItemVisible(pData, iItem) ) pNode->iState |= XUI_ACCESSIBLE_STATE_OFFSCREEN;
	if ( pItem->iType == XUI_TOOLBAR_ITEM_SEPARATOR ) return XUI_OK;
	pNode->iState |= XUI_ACCESSIBLE_STATE_FOCUSABLE;
	if ( !__xuiToolbarItemInteractive(pItem) ) pNode->iState |= XUI_ACCESSIBLE_STATE_DISABLED;
	if ( iItem == pData->iHover ) {
		pNode->iState |= XUI_ACCESSIBLE_STATE_SELECTED;
		if ( xuiGetFocusWidget(xuiWidgetGetContext(pWidget)) == pWidget ) {
			pNode->iState |= XUI_ACCESSIBLE_STATE_FOCUSED;
		}
	}
	if ( (pItem->iState & XUI_TOOLBAR_ITEM_CHECKED) != 0u ) pNode->iState |= XUI_ACCESSIBLE_STATE_CHECKED;
	pNode->iActions = XUI_ACCESSIBLE_ACTION_MASK(XUI_ACCESSIBLE_ACTION_FOCUS);
	pNode->iActions |= XUI_ACCESSIBLE_ACTION_MASK(
		pItem->iType == XUI_TOOLBAR_ITEM_TOGGLE ? XUI_ACCESSIBLE_ACTION_TOGGLE : XUI_ACCESSIBLE_ACTION_ACTIVATE);
	return XUI_OK;
}

static int __xuiToolbarAccessibleAction(xui_widget pWidget, uint64_t iNodeId,
	int iAction, const void* pPayload, void* pUser)
{
	xui_toolbar_data_t* pData = (xui_toolbar_data_t*)pUser;
	xui_toolbar_resolved_t tResolved;
	int iIndex;
	int iRet;

	(void)pPayload;
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( iNodeId == 0 || iNodeId == 1 ) {
		return iAction == XUI_ACCESSIBLE_ACTION_FOCUS ?
			xuiSetFocusWidget(xuiWidgetGetContext(pWidget), pWidget) : XUI_ERROR_UNSUPPORTED;
	}
	__xuiToolbarResolve(pWidget, pData, &tResolved);
	(void)__xuiToolbarLayoutItems(pWidget, pData, &tResolved);
	if ( iNodeId == XUI_TOOLBAR_ACCESSIBLE_OVERFLOW_ID ) {
		if ( pData->iOverflowCount <= 0 ) return XUI_ERROR_NOT_FOUND;
		if ( iAction == XUI_ACCESSIBLE_ACTION_FOCUS ) {
			iRet = __xuiToolbarSetTarget(pWidget, pData, -1, 1);
			if ( iRet != XUI_OK ) return iRet;
			if ( __xuiToolbarGetData(pWidget) != pData ) return XUI_OK;
			return xuiSetFocusWidget(xuiWidgetGetContext(pWidget), pWidget);
		}
		if ( iAction != XUI_ACCESSIBLE_ACTION_ACTIVATE || pData->onOverflow == NULL ) {
			return XUI_ERROR_UNSUPPORTED;
		}
		return __xuiToolbarActivateOverflow(pWidget, pData);
	}
	if ( iNodeId < 2 || iNodeId > (uint64_t)pData->iItemCount + 1u ) return XUI_ERROR_NOT_FOUND;
	iIndex = (int)iNodeId - 2;
	if ( !__xuiToolbarItemInteractive(&pData->arrItems[iIndex]) ) return XUI_ERROR_INVALID_STATE;
	if ( iAction == XUI_ACCESSIBLE_ACTION_FOCUS ) {
		if ( !__xuiToolbarItemVisible(pData, iIndex) ) return XUI_ERROR_INVALID_STATE;
		iRet = __xuiToolbarSetHover(pWidget, pData, iIndex);
		if ( iRet != XUI_OK ) return iRet;
		if ( __xuiToolbarGetData(pWidget) != pData ) return XUI_OK;
		return xuiSetFocusWidget(xuiWidgetGetContext(pWidget), pWidget);
	}
	if ( iAction == XUI_ACCESSIBLE_ACTION_ACTIVATE ||
	     (iAction == XUI_ACCESSIBLE_ACTION_TOGGLE && pData->arrItems[iIndex].iType == XUI_TOOLBAR_ITEM_TOGGLE) ) {
		return __xuiToolbarSelect(pWidget, pData, iIndex);
	}
	return XUI_ERROR_UNSUPPORTED;
}

static int __xuiToolbarInit(xui_widget pWidget, void* pTypeData, const void* pCreateData, void* pUser)
{
	xui_toolbar_data_t* pData;
	const xui_toolbar_desc_t* pDesc;
	int iRet;

	(void)pUser;
	if ( (pWidget == NULL) || (pTypeData == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	pData = (xui_toolbar_data_t*)pTypeData;
	pDesc = (const xui_toolbar_desc_t*)pCreateData;
	memset(pData, 0, sizeof(*pData));
	__xuiToolbarDefaultMetrics(&pData->tMetrics);
	__xuiToolbarDefaultColors(&pData->tColors);
	pData->iHover = -1;
	pData->iActive = -1;
	pData->iOverflowFirst = -1;
	pData->pFont = xuiGetDefaultFont(xuiWidgetGetContext(pWidget));
	if ( (pDesc != NULL) && (pDesc->pFont != NULL) ) pData->pFont = pDesc->pFont;
	if ( (pDesc != NULL) && pDesc->bHasMetrics ) {
		if ( !__xuiToolbarMetricsValid(&pDesc->tMetrics) ) return XUI_ERROR_INVALID_ARGUMENT;
		pData->tMetrics = pDesc->tMetrics;
		pData->tMetrics.iSize = sizeof(pData->tMetrics);
	}
	if ( (pDesc != NULL) && pDesc->bHasColors ) {
		if ( !__xuiToolbarColorsValid(&pDesc->tColors) ) return XUI_ERROR_INVALID_ARGUMENT;
		pData->tColors = pDesc->tColors;
		pData->tColors.iSize = sizeof(pData->tColors);
	}
	pData->bOverflowEnabled = (pDesc != NULL) ? (pDesc->bOverflowEnabled != 0) : 0;
	iRet = xuiWidgetSetFocusable(pWidget, 1);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetTabStop(pWidget, 0);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetOverflow(pWidget, XUI_OVERFLOW_VISIBLE);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetTooltipResolver(pWidget, __xuiToolbarTooltipResolve, NULL);
	if ( iRet == XUI_OK ) iRet = __xuiToolbarInitEvents(pWidget);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetAccessibilityProvider(pWidget,
		__xuiToolbarAccessibleCount, __xuiToolbarAccessibleGet,
		__xuiToolbarAccessibleAction, pData);
	if ( iRet != XUI_OK ) return iRet;
	if ( (pDesc != NULL) && (pDesc->iItemCount > 0) ) {
		return xuiToolbarSetItems(pWidget, pDesc->pItems, pDesc->iItemCount);
	}
	return XUI_OK;
}

static void __xuiToolbarDestroy(xui_widget pWidget, void* pTypeData, void* pUser)
{
	xui_toolbar_data_t* pData;

	(void)pUser;
	pData = (xui_toolbar_data_t*)pTypeData;
	if ( pWidget != NULL ) {
		if ( pData != NULL ) (void)xuiInternalPointerOwnerCancel(
			xuiWidgetGetContext(pWidget), pWidget, &pData->tPointerOwner);
		(void)xuiWidgetClearTooltip(pWidget);
	}
	if ( pData != NULL ) {
		__xuiToolbarReleaseStrings(pData);
		memset(pData, 0, sizeof(*pData));
	}
}

static xui_toolbar_data_t* __xuiToolbarGetData(xui_widget pWidget)
{
	xui_context pContext;
	xui_widget_type pType;

	if ( !xuiInternalWidgetIsValid(pWidget) ) return NULL;
	pContext = xuiWidgetGetContext(pWidget);
	pType = xuiWidgetFindType(pContext, "toolbar");
	if ( (pType == NULL) || !xuiWidgetIsType(pWidget, pType) ) return NULL;
	return (xui_toolbar_data_t*)xuiWidgetGetTypeData(pWidget);
}

static void __xuiToolbarRegisterStyleProperty(xui_context pContext, xui_widget_type pType, const char* sName, int iValueType, uint32_t iDirtyFlags, uint32_t iFlags)
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

static void __xuiToolbarRegisterStyleProperties(xui_context pContext, xui_widget_type pType)
{
	uint32_t iPaintDirty;
	uint32_t iLayoutDirty;

	iPaintDirty = XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER;
	iLayoutDirty = XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER;
	__xuiToolbarRegisterStyleProperty(pContext, pType, "font.name", XUI_STYLE_VALUE_STRING, iLayoutDirty, XUI_STYLE_PROPERTY_INHERITED);
	__xuiToolbarRegisterStyleProperty(pContext, pType, "toolbar.background.color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiToolbarRegisterStyleProperty(pContext, pType, "toolbar.border.color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiToolbarRegisterStyleProperty(pContext, pType, "toolbar.item.color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiToolbarRegisterStyleProperty(pContext, pType, "toolbar.item.hover_color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiToolbarRegisterStyleProperty(pContext, pType, "toolbar.item.active_color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiToolbarRegisterStyleProperty(pContext, pType, "toolbar.item.checked_color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiToolbarRegisterStyleProperty(pContext, pType, "toolbar.item.disabled_color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiToolbarRegisterStyleProperty(pContext, pType, "toolbar.focus.color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiToolbarRegisterStyleProperty(pContext, pType, "toolbar.separator.color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiToolbarRegisterStyleProperty(pContext, pType, "toolbar.text.color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiToolbarRegisterStyleProperty(pContext, pType, "toolbar.text.disabled_color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiToolbarRegisterStyleProperty(pContext, pType, "toolbar.icon.color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiToolbarRegisterStyleProperty(pContext, pType, "toolbar.text.active_color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiToolbarRegisterStyleProperty(pContext, pType, "toolbar.icon.active_color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiToolbarRegisterStyleProperty(pContext, pType, "toolbar.separator.highlight_color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiToolbarRegisterStyleProperty(pContext, pType, "toolbar.orientation", XUI_STYLE_VALUE_INT, iLayoutDirty, 0);
	__xuiToolbarRegisterStyleProperty(pContext, pType, "toolbar.item.width", XUI_STYLE_VALUE_FLOAT, iLayoutDirty, 0);
	__xuiToolbarRegisterStyleProperty(pContext, pType, "toolbar.item.height", XUI_STYLE_VALUE_FLOAT, iLayoutDirty, 0);
	__xuiToolbarRegisterStyleProperty(pContext, pType, "toolbar.separator.size", XUI_STYLE_VALUE_FLOAT, iLayoutDirty, 0);
	__xuiToolbarRegisterStyleProperty(pContext, pType, "toolbar.group_gap", XUI_STYLE_VALUE_FLOAT, iLayoutDirty, 0);
	__xuiToolbarRegisterStyleProperty(pContext, pType, "toolbar.padding.x", XUI_STYLE_VALUE_FLOAT, iLayoutDirty, 0);
	__xuiToolbarRegisterStyleProperty(pContext, pType, "toolbar.padding.y", XUI_STYLE_VALUE_FLOAT, iLayoutDirty, 0);
	__xuiToolbarRegisterStyleProperty(pContext, pType, "toolbar.overflow.size", XUI_STYLE_VALUE_FLOAT, iLayoutDirty, 0);
	__xuiToolbarRegisterStyleProperty(pContext, pType, "toolbar.border.width", XUI_STYLE_VALUE_FLOAT, iPaintDirty, 0);
	__xuiToolbarRegisterStyleProperty(pContext, pType, "toolbar.icon.size", XUI_STYLE_VALUE_FLOAT, iLayoutDirty, 0);
	__xuiToolbarRegisterStyleProperty(pContext, pType, "toolbar.icon.gap", XUI_STYLE_VALUE_FLOAT, iLayoutDirty, 0);
}

XUI_API xui_widget_type xuiToolbarGetType(xui_context pContext)
{
	xui_widget_type pType;
	xui_widget_type_desc_t tDesc;
	int iRet;

	if ( !xuiInternalContextIsValid(pContext) ) return NULL;
	pType = xuiWidgetFindType(pContext, "toolbar");
	if ( pType != NULL ) return pType;
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.sName = "toolbar";
	tDesc.pParent = xuiWidgetGetBaseType();
	tDesc.iFlags = XUI_WIDGET_TYPE_DEFAULT_LAYOUT | XUI_WIDGET_TYPE_DEFAULT_CACHE_POLICY;
	tDesc.iTypeDataSize = sizeof(xui_toolbar_data_t);
	tDesc.onInit = __xuiToolbarInit;
	tDesc.onDestroy = __xuiToolbarDestroy;
	tDesc.onContentMeasure = __xuiToolbarContentMeasure;
	tDesc.onCacheRender = __xuiToolbarCacheRender;
	__xuiToolbarDefaultLayout(&tDesc.tLayout);
	__xuiToolbarDefaultCachePolicy(&tDesc.tCachePolicy);
	iRet = xuiWidgetRegisterType(pContext, &pType, &tDesc);
	if ( iRet != XUI_OK ) return NULL;
	__xuiToolbarRegisterStyleProperties(pContext, pType);
	return pType;
}

XUI_API int xuiToolbarCreate(xui_context pContext, xui_widget* ppWidget, const xui_toolbar_desc_t* pDesc)
{
	xui_widget_type pType;

	if ( !xuiInternalContextIsValid(pContext) || (ppWidget == NULL) || !__xuiToolbarDescValid(pDesc) ) return XUI_ERROR_INVALID_ARGUMENT;
	*ppWidget = NULL;
	pType = xuiToolbarGetType(pContext);
	if ( pType == NULL ) return XUI_ERROR_NOT_INITIALIZED;
	return xuiWidgetCreateTyped(pContext, pType, ppWidget, pDesc);
}

XUI_API int xuiToolbarSetSelect(xui_widget pWidget, xui_toolbar_select_proc onSelect, void* pUser)
{
	xui_toolbar_data_t* pData = __xuiToolbarGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	pData->onSelect = onSelect;
	pData->pSelectUser = pUser;
	return XUI_OK;
}

XUI_API int xuiToolbarSetContextMenu(xui_widget pWidget, xui_toolbar_context_proc onContext, void* pUser)
{
	xui_toolbar_data_t* pData = __xuiToolbarGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	pData->onContext = onContext;
	pData->pContextUser = pUser;
	return XUI_OK;
}

XUI_API int xuiToolbarSetOverflow(xui_widget pWidget, int bEnabled, float fButtonSize, xui_toolbar_overflow_proc onOverflow, void* pUser)
{
	xui_toolbar_data_t* pData = __xuiToolbarGetData(pWidget);
	xui_toolbar_overflow_proc onOldOverflow;
	void* pOldOverflowUser;
	xui_rect_t tOldOverflowRect;
	float fOldOverflowSize;
	int bOldEnabled;
	int bOldFocused;
	int bOldActive;
	int iOldFirst;
	int iOldCount;
	int iOldChangeCount;
	int bNewEnabled;
	int iRet;
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( !isfinite(fButtonSize) || fButtonSize <= 0.0f ) return XUI_ERROR_INVALID_ARGUMENT;
	bNewEnabled = bEnabled ? 1 : 0;
	if ( pData->bOverflowEnabled == bNewEnabled && pData->tMetrics.fOverflowSize == fButtonSize &&
	     pData->onOverflow == onOverflow && pData->pOverflowUser == pUser ) return XUI_OK;
	if ( !bNewEnabled && pData->bOverflowActive ) {
		iRet = __xuiToolbarCancelPress(pWidget, pData);
		if ( iRet != XUI_OK ) return iRet;
		if ( __xuiToolbarGetData(pWidget) != pData ) return XUI_OK;
	}
	bOldEnabled = pData->bOverflowEnabled;
	bOldFocused = pData->bOverflowFocused;
	bOldActive = pData->bOverflowActive;
	iOldFirst = pData->iOverflowFirst;
	iOldCount = pData->iOverflowCount;
	tOldOverflowRect = pData->tOverflowRect;
	fOldOverflowSize = pData->tMetrics.fOverflowSize;
	onOldOverflow = pData->onOverflow;
	pOldOverflowUser = pData->pOverflowUser;
	iOldChangeCount = pData->iChangeCount;
	pData->bOverflowEnabled = bNewEnabled;
	pData->tMetrics.fOverflowSize = fButtonSize;
	pData->onOverflow = onOverflow;
	pData->pOverflowUser = pUser;
	if ( !pData->bOverflowEnabled ) {
		pData->bOverflowFocused = 0;
		pData->bOverflowActive = 0;
		pData->iOverflowFirst = -1;
		pData->iOverflowCount = 0;
		memset(&pData->tOverflowRect, 0, sizeof(pData->tOverflowRect));
	}
	pData->iChangeCount++;
	iRet = xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	if ( iRet != XUI_OK ) {
		pData->bOverflowEnabled = bOldEnabled;
		pData->bOverflowFocused = bOldFocused;
		pData->bOverflowActive = bOldActive;
		pData->iOverflowFirst = iOldFirst;
		pData->iOverflowCount = iOldCount;
		pData->tOverflowRect = tOldOverflowRect;
		pData->tMetrics.fOverflowSize = fOldOverflowSize;
		pData->onOverflow = onOldOverflow;
		pData->pOverflowUser = pOldOverflowUser;
		pData->iChangeCount = iOldChangeCount;
		return iRet;
	}
	xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_TREE_CHANGED);
	xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_BOUNDS_CHANGED);
	return XUI_OK;
}

XUI_API int xuiToolbarIsOverflowEnabled(xui_widget pWidget)
{
	xui_toolbar_data_t* pData = __xuiToolbarGetData(pWidget);
	return (pData != NULL) ? pData->bOverflowEnabled : 0;
}

XUI_API int xuiToolbarSetItems(xui_widget pWidget, const xui_toolbar_item_t* pItems, int iCount)
{
	xui_toolbar_data_t* pData = __xuiToolbarGetData(pWidget);
	xui_toolbar_item_t arrNew[XUI_TOOLBAR_ITEM_CAPACITY];
	xui_toolbar_item_t arrOld[XUI_TOOLBAR_ITEM_CAPACITY];
	char* arrTexts[XUI_TOOLBAR_ITEM_CAPACITY] = {0};
	char* arrTooltips[XUI_TOOLBAR_ITEM_CAPACITY] = {0};
	char* arrOldTexts[XUI_TOOLBAR_ITEM_CAPACITY];
	char* arrOldTooltips[XUI_TOOLBAR_ITEM_CAPACITY];
	xui_rect_t tOldOverflowRect;
	int iOldCount;
	int iOldHover;
	int iOldActive;
	int iOldOverflowFirst;
	int iOldOverflowCount;
	int iOldOverflowFocused;
	int iOldOverflowActive;
	int iOldChangeCount;
	int iRet;
	int i;

	if ( (pData == NULL) || (iCount < 0) || (iCount > XUI_TOOLBAR_ITEM_CAPACITY) || ((iCount > 0) && (pItems == NULL)) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	for ( i = 0; i < iCount; i++ ) {
		if ( !__xuiToolbarItemValid(&pItems[i]) ) return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( pData->iItemCount == iCount ) {
		for ( i = 0; i < iCount; i++ ) {
			if ( !__xuiToolbarItemEqual(&pData->arrItems[i], &pItems[i]) ) break;
		}
		if ( i == iCount ) return XUI_OK;
	}
	memset(arrNew, 0, sizeof(arrNew));
	for ( i = 0; i < iCount; i++ ) {
		arrNew[i] = pItems[i];
		memset(&arrNew[i].tRect, 0, sizeof(arrNew[i].tRect));
		if ( arrNew[i].iType == XUI_TOOLBAR_ITEM_SEPARATOR ) arrNew[i].iState = 0u;
		arrTexts[i] = __xuiToolbarCopyString(arrNew[i].sText);
		if ( __xuiToolbarStringValue(arrNew[i].sText)[0] != '\0' && arrTexts[i] == NULL ) goto out_of_memory;
		arrTooltips[i] = __xuiToolbarCopyString(arrNew[i].sTooltip);
		if ( __xuiToolbarStringValue(arrNew[i].sTooltip)[0] != '\0' && arrTooltips[i] == NULL ) goto out_of_memory;
		arrNew[i].sText = arrTexts[i] != NULL ? arrTexts[i] : "";
		arrNew[i].sTooltip = arrTooltips[i] != NULL ? arrTooltips[i] : "";
	}
	iRet = xuiWidgetClearTooltip(pWidget);
	if ( iRet != XUI_OK ) goto failed_new;
	iRet = __xuiToolbarCancelPress(pWidget, pData);
	if ( iRet != XUI_OK ) goto failed_new;
	if ( __xuiToolbarGetData(pWidget) != pData ) goto abandoned_new;
	memcpy(arrOld, pData->arrItems, sizeof(arrOld));
	memcpy(arrOldTexts, pData->arrOwnedTexts, sizeof(arrOldTexts));
	memcpy(arrOldTooltips, pData->arrOwnedTooltips, sizeof(arrOldTooltips));
	iOldCount = pData->iItemCount;
	iOldHover = pData->iHover;
	iOldActive = pData->iActive;
	iOldOverflowFirst = pData->iOverflowFirst;
	iOldOverflowCount = pData->iOverflowCount;
	iOldOverflowFocused = pData->bOverflowFocused;
	iOldOverflowActive = pData->bOverflowActive;
	tOldOverflowRect = pData->tOverflowRect;
	iOldChangeCount = pData->iChangeCount;
	memcpy(pData->arrItems, arrNew, sizeof(arrNew));
	memcpy(pData->arrOwnedTexts, arrTexts, sizeof(arrTexts));
	memcpy(pData->arrOwnedTooltips, arrTooltips, sizeof(arrTooltips));
	pData->iItemCount = iCount;
	pData->iHover = -1;
	pData->iActive = -1;
	pData->bOverflowFocused = 0;
	pData->bOverflowActive = 0;
	pData->iOverflowFirst = -1;
	pData->iOverflowCount = 0;
	memset(&pData->tOverflowRect, 0, sizeof(pData->tOverflowRect));
	pData->iChangeCount++;
	iRet = xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	if ( iRet != XUI_OK ) {
		memcpy(pData->arrItems, arrOld, sizeof(arrOld));
		memcpy(pData->arrOwnedTexts, arrOldTexts, sizeof(arrOldTexts));
		memcpy(pData->arrOwnedTooltips, arrOldTooltips, sizeof(arrOldTooltips));
		pData->iItemCount = iOldCount;
		pData->iHover = iOldHover;
		pData->iActive = iOldActive;
		pData->iOverflowFirst = iOldOverflowFirst;
		pData->iOverflowCount = iOldOverflowCount;
		pData->bOverflowFocused = iOldOverflowFocused;
		pData->bOverflowActive = iOldOverflowActive;
		pData->tOverflowRect = tOldOverflowRect;
		pData->iChangeCount = iOldChangeCount;
		goto failed_new;
	}
	__xuiToolbarFreeCopiedItems(arrOldTexts, arrOldTooltips, iOldCount);
	xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_TREE_CHANGED);
	xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_SELECTION_CHANGED);
	return XUI_OK;

out_of_memory:
	iRet = XUI_ERROR_OUT_OF_MEMORY;
failed_new:
	__xuiToolbarFreeCopiedItems(arrTexts, arrTooltips, iCount);
	return iRet;
abandoned_new:
	__xuiToolbarFreeCopiedItems(arrTexts, arrTooltips, iCount);
	return XUI_OK;
}

XUI_API int xuiToolbarAddItem(xui_widget pWidget, const char* sText, int iType, int iValue)
{
	xui_toolbar_data_t* pData = __xuiToolbarGetData(pWidget);
	xui_toolbar_item_t arrItems[XUI_TOOLBAR_ITEM_CAPACITY];
	xui_toolbar_item_t* pItem;

	if ( (pData == NULL) || (pData->iItemCount >= XUI_TOOLBAR_ITEM_CAPACITY) ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( !__xuiToolbarItemTypeValid(iType) ) return XUI_ERROR_INVALID_ARGUMENT;
	memcpy(arrItems, pData->arrItems, sizeof(arrItems));
	pItem = &arrItems[pData->iItemCount];
	memset(pItem, 0, sizeof(*pItem));
	pItem->sText = (sText != NULL) ? sText : "";
	pItem->sTooltip = "";
	pItem->iType = iType;
	pItem->iValue = iValue;
	pItem->iState = (iType == XUI_TOOLBAR_ITEM_SEPARATOR) ? 0 : XUI_TOOLBAR_ITEM_ENABLED;
	return xuiToolbarSetItems(pWidget, arrItems, pData->iItemCount + 1);
}

XUI_API int xuiToolbarAddSeparator(xui_widget pWidget)
{
	return xuiToolbarAddItem(pWidget, "", XUI_TOOLBAR_ITEM_SEPARATOR, 0);
}

XUI_API int xuiToolbarClear(xui_widget pWidget)
{
	return xuiToolbarSetItems(pWidget, NULL, 0);
}

XUI_API int xuiToolbarGetItemCount(xui_widget pWidget)
{
	xui_toolbar_data_t* pData = __xuiToolbarGetData(pWidget);
	return (pData != NULL) ? pData->iItemCount : 0;
}

XUI_API const xui_toolbar_item_t* xuiToolbarGetItem(xui_widget pWidget, int iIndex)
{
	xui_toolbar_data_t* pData = __xuiToolbarGetData(pWidget);
	if ( (pData == NULL) || (iIndex < 0) || (iIndex >= pData->iItemCount) ) return NULL;
	return &pData->arrItems[iIndex];
}

XUI_API xui_rect_t xuiToolbarGetItemRect(xui_widget pWidget, int iIndex)
{
	xui_toolbar_data_t* pData = __xuiToolbarGetData(pWidget);
	xui_toolbar_resolved_t tResolved;
	xui_rect_t tEmpty;

	memset(&tEmpty, 0, sizeof(tEmpty));
	if ( (pData == NULL) || (iIndex < 0) || (iIndex >= pData->iItemCount) ) return tEmpty;
	__xuiToolbarResolve(pWidget, pData, &tResolved);
	(void)__xuiToolbarLayoutItems(pWidget, pData, &tResolved);
	return pData->arrItems[iIndex].tRect;
}

XUI_API int xuiToolbarGetItemAt(xui_widget pWidget, float fX, float fY)
{
	xui_toolbar_data_t* pData = __xuiToolbarGetData(pWidget);
	xui_toolbar_resolved_t tResolved;
	if ( pData == NULL ) return -1;
	__xuiToolbarResolve(pWidget, pData, &tResolved);
	(void)__xuiToolbarLayoutItems(pWidget, pData, &tResolved);
	return __xuiToolbarIndexAt(pData, fX, fY);
}

XUI_API int xuiToolbarSetOrientation(xui_widget pWidget, int iOrientation)
{
	xui_toolbar_data_t* pData = __xuiToolbarGetData(pWidget);
	xui_toolbar_metrics_t tMetrics;
	if ( (pData == NULL) || !__xuiToolbarOrientationValid(iOrientation) ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pData->tMetrics.iOrientation == iOrientation ) return XUI_OK;
	tMetrics = pData->tMetrics;
	tMetrics.iOrientation = iOrientation;
	return xuiToolbarSetMetrics(pWidget, &tMetrics);
}

XUI_API int xuiToolbarGetOrientation(xui_widget pWidget)
{
	xui_toolbar_data_t* pData = __xuiToolbarGetData(pWidget);
	return (pData != NULL) ? pData->tMetrics.iOrientation : XUI_ORIENTATION_HORIZONTAL;
}

XUI_API int xuiToolbarSetItemSize(xui_widget pWidget, float fWidth, float fHeight, float fSeparatorSize)
{
	xui_toolbar_data_t* pData = __xuiToolbarGetData(pWidget);
	xui_toolbar_metrics_t tMetrics;
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( !isfinite(fWidth) || !isfinite(fHeight) || !isfinite(fSeparatorSize) ||
	     fWidth <= 0.0f || fHeight <= 0.0f || fSeparatorSize <= 0.0f ) return XUI_ERROR_INVALID_ARGUMENT;
	tMetrics = pData->tMetrics;
	tMetrics.fItemWidth = fWidth;
	tMetrics.fItemHeight = fHeight;
	tMetrics.fSeparatorSize = fSeparatorSize;
	return xuiToolbarSetMetrics(pWidget, &tMetrics);
}

XUI_API int xuiToolbarSetItemGroup(xui_widget pWidget, int iIndex, int iGroup)
{
	xui_toolbar_data_t* pData = __xuiToolbarGetData(pWidget);
	int iOldGroup;
	int iOldChangeCount;
	int iRet;
	if ( (pData == NULL) || (iIndex < 0) || (iIndex >= pData->iItemCount) || iGroup < 0 ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pData->arrItems[iIndex].iGroup == iGroup ) return XUI_OK;
	iOldGroup = pData->arrItems[iIndex].iGroup;
	iOldChangeCount = pData->iChangeCount;
	pData->arrItems[iIndex].iGroup = iGroup;
	pData->iChangeCount++;
	iRet = xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	if ( iRet != XUI_OK ) {
		pData->arrItems[iIndex].iGroup = iOldGroup;
		pData->iChangeCount = iOldChangeCount;
		return iRet;
	}
	xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_BOUNDS_CHANGED);
	return XUI_OK;
}

XUI_API int xuiToolbarGetItemGroup(xui_widget pWidget, int iIndex)
{
	xui_toolbar_data_t* pData = __xuiToolbarGetData(pWidget);
	if ( (pData == NULL) || (iIndex < 0) || (iIndex >= pData->iItemCount) ) return 0;
	return pData->arrItems[iIndex].iGroup;
}

XUI_API int xuiToolbarSetItemTooltip(xui_widget pWidget, int iIndex, const char* sText)
{
	xui_toolbar_data_t* pData = __xuiToolbarGetData(pWidget);
	const char* sValue = (sText != NULL) ? sText : "";
	char* sCopy;
	int iRet;
	if ( (pData == NULL) || (iIndex < 0) || (iIndex >= pData->iItemCount) ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pData->arrItems[iIndex].sTooltip != NULL && strcmp(pData->arrItems[iIndex].sTooltip, sValue) == 0 ) return XUI_OK;
	sCopy = __xuiToolbarCopyString(sValue);
	if ( sValue[0] != '\0' && sCopy == NULL ) return XUI_ERROR_OUT_OF_MEMORY;
	iRet = xuiWidgetClearTooltip(pWidget);
	if ( iRet != XUI_OK ) {
		xrtFree(sCopy);
		return iRet;
	}
	xrtFree(pData->arrOwnedTooltips[iIndex]);
	pData->arrOwnedTooltips[iIndex] = sCopy;
	pData->arrItems[iIndex].sTooltip = sCopy != NULL ? sCopy : "";
	pData->iChangeCount++;
	xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_NODE_CHANGED);
	return XUI_OK;
}

XUI_API const char* xuiToolbarGetItemTooltip(xui_widget pWidget, int iIndex)
{
	xui_toolbar_data_t* pData = __xuiToolbarGetData(pWidget);
	if ( (pData == NULL) || (iIndex < 0) || (iIndex >= pData->iItemCount) ) return "";
	return (pData->arrItems[iIndex].sTooltip != NULL) ? pData->arrItems[iIndex].sTooltip : "";
}

XUI_API const char* xuiToolbarGetHoverTooltip(xui_widget pWidget)
{
	xui_toolbar_data_t* pData = __xuiToolbarGetData(pWidget);
	if ( (pData == NULL) || (pData->iHover < 0) || (pData->iHover >= pData->iItemCount) ) return "";
	return xuiToolbarGetItemTooltip(pWidget, pData->iHover);
}

XUI_API int xuiToolbarSetItemEnabled(xui_widget pWidget, int iIndex, int bEnabled)
{
	xui_toolbar_data_t* pData = __xuiToolbarGetData(pWidget);
	uint32_t iOldState;
	int bWasEnabled;
	int iOldHover;
	int iOldActive;
	int iOldChangeCount;
	int iRet;
	if ( (pData == NULL) || (iIndex < 0) || (iIndex >= pData->iItemCount) ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pData->arrItems[iIndex].iType == XUI_TOOLBAR_ITEM_SEPARATOR ) return XUI_OK;
	bWasEnabled = (pData->arrItems[iIndex].iState & XUI_TOOLBAR_ITEM_ENABLED) != 0;
	if ( bWasEnabled == (bEnabled != 0) ) return XUI_OK;
	if ( !bEnabled && pData->iActive == iIndex ) {
		iRet = __xuiToolbarCancelPress(pWidget, pData);
		if ( iRet != XUI_OK ) return iRet;
		if ( __xuiToolbarGetData(pWidget) != pData ) return XUI_OK;
	}
	iOldState = pData->arrItems[iIndex].iState;
	iOldHover = pData->iHover;
	iOldActive = pData->iActive;
	iOldChangeCount = pData->iChangeCount;
	if ( bEnabled ) pData->arrItems[iIndex].iState |= XUI_TOOLBAR_ITEM_ENABLED;
	else {
		pData->arrItems[iIndex].iState &= ~XUI_TOOLBAR_ITEM_ENABLED;
		if ( pData->iHover == iIndex ) pData->iHover = -1;
		if ( pData->iActive == iIndex ) pData->iActive = -1;
	}
	pData->iChangeCount++;
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

XUI_API int xuiToolbarIsItemEnabled(xui_widget pWidget, int iIndex)
{
	xui_toolbar_data_t* pData = __xuiToolbarGetData(pWidget);
	if ( (pData == NULL) || (iIndex < 0) || (iIndex >= pData->iItemCount) ) return 0;
	return __xuiToolbarItemInteractive(&pData->arrItems[iIndex]);
}

XUI_API int xuiToolbarSetItemChecked(xui_widget pWidget, int iIndex, int bChecked)
{
	xui_toolbar_data_t* pData = __xuiToolbarGetData(pWidget);
	uint32_t iOldState;
	int iOldChangeCount;
	int iRet;
	if ( (pData == NULL) || (iIndex < 0) || (iIndex >= pData->iItemCount) ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pData->arrItems[iIndex].iType == XUI_TOOLBAR_ITEM_SEPARATOR ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( ((pData->arrItems[iIndex].iState & XUI_TOOLBAR_ITEM_CHECKED) != 0) == (bChecked != 0) ) return XUI_OK;
	iOldState = pData->arrItems[iIndex].iState;
	iOldChangeCount = pData->iChangeCount;
	if ( bChecked ) pData->arrItems[iIndex].iState |= XUI_TOOLBAR_ITEM_CHECKED;
	else pData->arrItems[iIndex].iState &= ~XUI_TOOLBAR_ITEM_CHECKED;
	pData->iChangeCount++;
	iRet = xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	if ( iRet != XUI_OK ) {
		pData->arrItems[iIndex].iState = iOldState;
		pData->iChangeCount = iOldChangeCount;
		return iRet;
	}
	xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_STATE_CHANGED);
	return XUI_OK;
}

XUI_API int xuiToolbarGetItemChecked(xui_widget pWidget, int iIndex)
{
	xui_toolbar_data_t* pData = __xuiToolbarGetData(pWidget);
	if ( (pData == NULL) || (iIndex < 0) || (iIndex >= pData->iItemCount) ) return 0;
	return (pData->arrItems[iIndex].iState & XUI_TOOLBAR_ITEM_CHECKED) != 0;
}

XUI_API int xuiToolbarSetItemIcon(xui_widget pWidget, int iIndex, xui_surface pIcon, xui_rect_t tSrc)
{
	xui_toolbar_data_t* pData = __xuiToolbarGetData(pWidget);
	xui_surface pOldIcon;
	xui_rect_t tOldSrc;
	int iOldChangeCount;
	int iRet;
	if ( (pData == NULL) || (iIndex < 0) || (iIndex >= pData->iItemCount) ||
	     !__xuiToolbarRectNonnegative(tSrc) ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pData->arrItems[iIndex].pIcon == pIcon && __xuiToolbarRectEqual(pData->arrItems[iIndex].tIconSrc, tSrc) ) return XUI_OK;
	pOldIcon = pData->arrItems[iIndex].pIcon;
	tOldSrc = pData->arrItems[iIndex].tIconSrc;
	iOldChangeCount = pData->iChangeCount;
	pData->arrItems[iIndex].pIcon = pIcon;
	pData->arrItems[iIndex].tIconSrc = tSrc;
	pData->iChangeCount++;
	iRet = xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	if ( iRet != XUI_OK ) {
		pData->arrItems[iIndex].pIcon = pOldIcon;
		pData->arrItems[iIndex].tIconSrc = tOldSrc;
		pData->iChangeCount = iOldChangeCount;
		return iRet;
	}
	xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_NODE_CHANGED);
	return XUI_OK;
}

XUI_API int xuiToolbarGetHoverIndex(xui_widget pWidget)
{
	xui_toolbar_data_t* pData = __xuiToolbarGetData(pWidget);
	return (pData != NULL) ? pData->iHover : -1;
}

XUI_API int xuiToolbarSetHoverIndex(xui_widget pWidget, int iIndex)
{
	xui_toolbar_data_t* pData = __xuiToolbarGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	return __xuiToolbarSetHover(pWidget, pData, iIndex);
}

XUI_API int xuiToolbarGetActiveIndex(xui_widget pWidget)
{
	xui_toolbar_data_t* pData = __xuiToolbarGetData(pWidget);
	return (pData != NULL) ? pData->iActive : -1;
}

XUI_API int xuiToolbarGetOverflowFirst(xui_widget pWidget)
{
	xui_toolbar_data_t* pData = __xuiToolbarGetData(pWidget);
	xui_toolbar_resolved_t tResolved;
	if ( pData == NULL ) return -1;
	__xuiToolbarResolve(pWidget, pData, &tResolved);
	(void)__xuiToolbarLayoutItems(pWidget, pData, &tResolved);
	return pData->iOverflowFirst;
}

XUI_API int xuiToolbarGetOverflowCount(xui_widget pWidget)
{
	xui_toolbar_data_t* pData = __xuiToolbarGetData(pWidget);
	xui_toolbar_resolved_t tResolved;
	if ( pData == NULL ) return 0;
	__xuiToolbarResolve(pWidget, pData, &tResolved);
	(void)__xuiToolbarLayoutItems(pWidget, pData, &tResolved);
	return pData->iOverflowCount;
}

XUI_API xui_rect_t xuiToolbarGetOverflowRect(xui_widget pWidget)
{
	xui_toolbar_data_t* pData = __xuiToolbarGetData(pWidget);
	xui_toolbar_resolved_t tResolved;
	xui_rect_t tEmpty;
	memset(&tEmpty, 0, sizeof(tEmpty));
	if ( pData == NULL ) return tEmpty;
	__xuiToolbarResolve(pWidget, pData, &tResolved);
	(void)__xuiToolbarLayoutItems(pWidget, pData, &tResolved);
	return pData->tOverflowRect;
}

XUI_API int xuiToolbarSetFont(xui_widget pWidget, xui_font pFont)
{
	xui_toolbar_data_t* pData = __xuiToolbarGetData(pWidget);
	xui_font pOldFont;
	int iOldChangeCount;
	int iRet;
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pData->pFont == pFont ) return XUI_OK;
	pOldFont = pData->pFont;
	iOldChangeCount = pData->iChangeCount;
	pData->pFont = pFont;
	pData->iChangeCount++;
	iRet = xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	if ( iRet != XUI_OK ) {
		pData->pFont = pOldFont;
		pData->iChangeCount = iOldChangeCount;
	}
	return iRet;
}

XUI_API xui_font xuiToolbarGetFont(xui_widget pWidget)
{
	xui_toolbar_data_t* pData = __xuiToolbarGetData(pWidget);
	return (pData != NULL) ? pData->pFont : NULL;
}

XUI_API int xuiToolbarSetMetrics(xui_widget pWidget, const xui_toolbar_metrics_t* pMetrics)
{
	xui_toolbar_data_t* pData = __xuiToolbarGetData(pWidget);
	xui_toolbar_metrics_t tNew;
	xui_toolbar_metrics_t tOld;
	int iOldChangeCount;
	int iRet;
	if ( (pData == NULL) || !__xuiToolbarMetricsValid(pMetrics) ) return XUI_ERROR_INVALID_ARGUMENT;
	tNew = *pMetrics;
	tNew.iSize = sizeof(tNew);
	if ( __xuiToolbarMetricsEqual(&pData->tMetrics, &tNew) ) return XUI_OK;
	tOld = pData->tMetrics;
	iOldChangeCount = pData->iChangeCount;
	pData->tMetrics = tNew;
	pData->iChangeCount++;
	iRet = xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	if ( iRet != XUI_OK ) {
		pData->tMetrics = tOld;
		pData->iChangeCount = iOldChangeCount;
		return iRet;
	}
	xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_BOUNDS_CHANGED);
	return XUI_OK;
}

XUI_API int xuiToolbarGetMetrics(xui_widget pWidget, xui_toolbar_metrics_t* pMetrics)
{
	xui_toolbar_data_t* pData = __xuiToolbarGetData(pWidget);
	if ( (pData == NULL) || (pMetrics == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	*pMetrics = pData->tMetrics;
	pMetrics->iSize = sizeof(*pMetrics);
	return XUI_OK;
}

XUI_API int xuiToolbarSetColors(xui_widget pWidget, const xui_toolbar_colors_t* pColors)
{
	xui_toolbar_data_t* pData = __xuiToolbarGetData(pWidget);
	xui_toolbar_colors_t tNew;
	xui_toolbar_colors_t tOld;
	int iOldChangeCount;
	int iRet;
	if ( (pData == NULL) || !__xuiToolbarColorsValid(pColors) ) return XUI_ERROR_INVALID_ARGUMENT;
	tNew = *pColors;
	tNew.iSize = sizeof(tNew);
	if ( __xuiToolbarColorsEqual(&pData->tColors, &tNew) ) return XUI_OK;
	tOld = pData->tColors;
	iOldChangeCount = pData->iChangeCount;
	pData->tColors = tNew;
	pData->iChangeCount++;
	iRet = xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	if ( iRet != XUI_OK ) {
		pData->tColors = tOld;
		pData->iChangeCount = iOldChangeCount;
	}
	return iRet;
}

XUI_API int xuiToolbarGetColors(xui_widget pWidget, xui_toolbar_colors_t* pColors)
{
	xui_toolbar_data_t* pData = __xuiToolbarGetData(pWidget);
	if ( (pData == NULL) || (pColors == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	*pColors = pData->tColors;
	pColors->iSize = sizeof(*pColors);
	return XUI_OK;
}

XUI_API uint32_t xuiToolbarGetState(xui_widget pWidget)
{
	xui_toolbar_data_t* pData = __xuiToolbarGetData(pWidget);
	uint32_t iState;
	if ( pData == NULL ) return XUI_WIDGET_STATE_DISABLED;
	iState = xuiWidgetGetInputState(pWidget);
	if ( pData->bOverflowActive ) iState |= XUI_TOOLBAR_STATE_OVERFLOW_ACTIVE;
	return iState;
}

XUI_API int xuiToolbarGetSelectCount(xui_widget pWidget)
{
	xui_toolbar_data_t* pData = __xuiToolbarGetData(pWidget);
	return (pData != NULL) ? pData->iSelectCount : 0;
}

XUI_API int xuiToolbarGetOverflowSelectCount(xui_widget pWidget)
{
	xui_toolbar_data_t* pData = __xuiToolbarGetData(pWidget);
	return (pData != NULL) ? pData->iOverflowSelectCount : 0;
}

XUI_API int xuiToolbarGetChangeCount(xui_widget pWidget)
{
	xui_toolbar_data_t* pData = __xuiToolbarGetData(pWidget);
	return (pData != NULL) ? pData->iChangeCount : 0;
}

#endif
