#include "xui_config.h"
#if XUI_ENABLE_TOAST
#include "xui_internal.h"
#include "xui_text_internal.h"

#include <limits.h>
#include <string.h>

#define XUI_TOAST_MAGIC 0x58544F41u

typedef struct xui_toast_item_t {
	int iId;
	int iType;
	char* sTitle;
	char* sMessage;
	float fDuration;
	float fElapsed;
	float fHeight;
	xui_rect_t tRect;
	xui_rect_t tIconRect;
	xui_rect_t tTitleRect;
	xui_rect_t tMessageRect;
	xui_rect_t tCloseRect;
	xui_toast_click_proc onClick;
	void* pUser;
	int bClosing;
	int iCloseReason;
	int iMessageLineCount;
} xui_toast_item_t;

typedef struct xui_toast_item_view_t {
	xui_toast pToast;
	int iSlot;
} xui_toast_item_view_t;

struct xui_toast_t {
	uint32_t iMagic;
	xui_context pContext;
	xui_font pFont;
	xui_widget arrWidgets[XUI_TOAST_VISIBLE_CAPACITY];
	xui_toast_item_t arrActive[XUI_TOAST_VISIBLE_CAPACITY];
	xui_toast_item_t arrPending[XUI_TOAST_QUEUE_CAPACITY];
	xui_toast_metrics_t tMetrics;
	xui_toast_colors_t tColors;
	xui_toast_close_proc onClose;
	void* pCloseUser;
	int iNextId;
	int iPlacement;
	int iDirection;
	int iActiveCount;
	int iPendingCount;
	int iHoverItem;
	int iHoverClose;
	xui_rect_t tLayoutRootRect;
	int iShowCount;
	int iCloseCount;
	int iExpireCount;
	int iDropCount;
	int iChangeCount;
	int iCallbackDepth;
	int bDestroyPending;
};

static int __xuiToastValid(xui_toast pToast)
{
	return (pToast != NULL) && (pToast->iMagic == XUI_TOAST_MAGIC);
}

static uint32_t __xuiToastStyleColor(xui_widget pWidget, const char* sName, uint32_t iBase)
{
	xui_style_property_t tProperty;
	memset(&tProperty, 0, sizeof(tProperty));
	tProperty.iSize = sizeof(tProperty);
	if ( xuiWidgetGetResolvedStyleProperty(pWidget, sName, &tProperty) == XUI_OK &&
		tProperty.tValue.iType == XUI_STYLE_VALUE_COLOR ) return tProperty.tValue.iColor;
	return iBase;
}

static void __xuiToastResolveColors(xui_widget pWidget, xui_toast pToast, xui_toast_colors_t* pColors)
{
	*pColors = pToast->tColors;
	pColors->iBackgroundColor = __xuiToastStyleColor(pWidget, "toast.background.color", pColors->iBackgroundColor);
	pColors->iBorderColor = __xuiToastStyleColor(pWidget, "toast.border.color", pColors->iBorderColor);
	pColors->iTextColor = __xuiToastStyleColor(pWidget, "toast.text.color", pColors->iTextColor);
	pColors->iMutedTextColor = __xuiToastStyleColor(pWidget, "toast.text.muted_color", pColors->iMutedTextColor);
	pColors->iShadowColor = __xuiToastStyleColor(pWidget, "toast.shadow.color", pColors->iShadowColor);
	pColors->iInfoColor = __xuiToastStyleColor(pWidget, "toast.info.color", pColors->iInfoColor);
	pColors->iSuccessColor = __xuiToastStyleColor(pWidget, "toast.success.color", pColors->iSuccessColor);
	pColors->iWarningColor = __xuiToastStyleColor(pWidget, "toast.warning.color", pColors->iWarningColor);
	pColors->iErrorColor = __xuiToastStyleColor(pWidget, "toast.error.color", pColors->iErrorColor);
	pColors->iCloseColor = __xuiToastStyleColor(pWidget, "toast.close.color", pColors->iCloseColor);
	pColors->iCloseHoverColor = __xuiToastStyleColor(pWidget, "toast.close.hover_color", pColors->iCloseHoverColor);
}

static void __xuiToastRegisterColors(xui_context pContext, xui_widget_type pType)
{
	static const char* arrNames[] = {
		"toast.background.color",
		"toast.border.color",
		"toast.text.color",
		"toast.text.muted_color",
		"toast.shadow.color",
		"toast.info.color",
		"toast.success.color",
		"toast.warning.color",
		"toast.error.color",
		"toast.close.color",
		"toast.close.hover_color",
		"toast.icon.background.color",
		"toast.progress.color"
	};
	xui_style_property_info_t tInfo;
	size_t i;
	memset(&tInfo, 0, sizeof(tInfo));
	tInfo.iSize = sizeof(tInfo);
	tInfo.pWidgetType = pType;
	tInfo.iValueType = XUI_STYLE_VALUE_COLOR;
	tInfo.iDirtyFlags = XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER;
	for ( i = 0; i < sizeof(arrNames) / sizeof(arrNames[0]); ++i ) {
		tInfo.sName = arrNames[i];
		(void)xuiStyleRegisterProperty(pContext, &tInfo, NULL);
	}
}

static float __xuiToastMin(float fA, float fB)
{
	return (fA < fB) ? fA : fB;
}

static float __xuiToastMax(float fA, float fB)
{
	return (fA > fB) ? fA : fB;
}

static float __xuiToastClamp(float fValue, float fMin, float fMax)
{
	if ( fMax < fMin ) return fMin;
	if ( fValue < fMin ) return fMin;
	if ( fValue > fMax ) return fMax;
	return fValue;
}

static int __xuiToastRectSame(xui_rect_t tA, xui_rect_t tB)
{
	return (tA.fX == tB.fX) && (tA.fY == tB.fY) &&
	       (tA.fW == tB.fW) && (tA.fH == tB.fH);
}

static void __xuiToastCountAdd(int* pCount, int iAmount)
{
	if ( pCount == NULL || iAmount <= 0 || *pCount >= INT_MAX ) return;
	if ( iAmount >= INT_MAX - *pCount ) *pCount = INT_MAX;
	else *pCount += iAmount;
}

static int __xuiToastFloatValid(float fValue)
{
	return (fValue == fValue) && (fValue >= 0.0f) && (fValue <= XUI_LAYOUT_UNBOUNDED);
}

static uint32_t __xuiToastColorWithAlpha(uint32_t iColor, uint32_t iAlpha)
{
	return (iColor & 0xFFFFFF00u) | (((iColor & 0xFFu) * (iAlpha & 0xFFu) + 127u) / 255u);
}

static uint32_t __xuiToastAlpha(uint32_t iColor)
{
	return iColor & 0xffu;
}

static int __xuiToastTypeValid(int iType)
{
	return (iType >= XUI_TOAST_TYPE_INFO) && (iType <= XUI_TOAST_TYPE_ERROR);
}

static int __xuiToastPlacementValid(int iPlacement)
{
	return (iPlacement >= XUI_TOAST_PLACEMENT_TOP_RIGHT) &&
	       (iPlacement <= XUI_TOAST_PLACEMENT_BOTTOM_CENTER);
}

static int __xuiToastDirectionValid(int iDirection)
{
	return (iDirection == XUI_TOAST_DIRECTION_AUTO) ||
	       (iDirection == XUI_TOAST_DIRECTION_DOWN) ||
	       (iDirection == XUI_TOAST_DIRECTION_UP);
}

static int __xuiToastDirection(xui_toast pToast)
{
	if ( !__xuiToastValid(pToast) ) {
		return XUI_TOAST_DIRECTION_DOWN;
	}
	if ( pToast->iDirection == XUI_TOAST_DIRECTION_DOWN || pToast->iDirection == XUI_TOAST_DIRECTION_UP ) {
		return pToast->iDirection;
	}
	if ( pToast->iPlacement == XUI_TOAST_PLACEMENT_BOTTOM_LEFT ||
	     pToast->iPlacement == XUI_TOAST_PLACEMENT_BOTTOM_CENTER ||
	     pToast->iPlacement == XUI_TOAST_PLACEMENT_BOTTOM_RIGHT ) {
		return XUI_TOAST_DIRECTION_UP;
	}
	return XUI_TOAST_DIRECTION_DOWN;
}

static char* __xuiToastDupText(const char* sText)
{
	char* sCopy;
	size_t iLen;

	if ( sText == NULL ) {
		sText = "";
	}
	iLen = strlen(sText);
	sCopy = (char*)xrtMalloc(iLen + 1u);
	if ( sCopy == NULL ) {
		return NULL;
	}
	memcpy(sCopy, sText, iLen + 1u);
	return sCopy;
}

static void __xuiToastItemFree(xui_toast_item_t* pItem)
{
	if ( pItem == NULL ) {
		return;
	}
	if ( pItem->sTitle != NULL ) {
		xrtFree(pItem->sTitle);
	}
	if ( pItem->sMessage != NULL ) {
		xrtFree(pItem->sMessage);
	}
	memset(pItem, 0, sizeof(*pItem));
}

static void __xuiToastItemMove(xui_toast_item_t* pDst, xui_toast_item_t* pSrc)
{
	if ( (pDst == NULL) || (pSrc == NULL) ) {
		return;
	}
	*pDst = *pSrc;
	memset(pSrc, 0, sizeof(*pSrc));
}

static void __xuiToastDefaultMetrics(xui_toast_metrics_t* pMetrics)
{
	memset(pMetrics, 0, sizeof(*pMetrics));
	pMetrics->iSize = sizeof(*pMetrics);
	pMetrics->fWidth = 320.0f;
	pMetrics->fMinWidth = 180.0f;
	pMetrics->fMinHeight = 58.0f;
	pMetrics->fMargin = 18.0f;
	pMetrics->fGap = 8.0f;
	pMetrics->fPaddingX = 14.0f;
	pMetrics->fPaddingY = 10.0f;
	pMetrics->fIconSize = 28.0f;
	pMetrics->fIconGap = 10.0f;
	pMetrics->fCloseSize = 16.0f;
	pMetrics->fProgressHeight = 2.0f;
	pMetrics->iMaxVisible = 0;
}

static void __xuiToastDefaultColors(xui_toast_colors_t* pColors)
{
	memset(pColors, 0, sizeof(*pColors));
	pColors->iSize = sizeof(*pColors);
	pColors->iBackgroundColor = XUI_COLOR_RGBA(247, 252, 255, 246);
	pColors->iBorderColor = XUI_COLOR_RGBA(128, 174, 207, 210);
	pColors->iTextColor = XUI_COLOR_RGBA(31, 58, 82, 255);
	pColors->iMutedTextColor = XUI_COLOR_RGBA(92, 112, 130, 255);
	pColors->iShadowColor = XUI_COLOR_RGBA(6, 22, 42, 48);
	pColors->iInfoColor = XUI_COLOR_RGBA(78, 159, 220, 255);
	pColors->iSuccessColor = XUI_COLOR_RGBA(43, 184, 150, 255);
	pColors->iWarningColor = XUI_COLOR_RGBA(231, 156, 45, 255);
	pColors->iErrorColor = XUI_COLOR_RGBA(216, 76, 90, 255);
	pColors->iCloseColor = XUI_COLOR_RGBA(96, 126, 148, 255);
	pColors->iCloseHoverColor = XUI_COLOR_RGBA(31, 58, 82, 255);
}

static int __xuiToastMetricsValid(const xui_toast_metrics_t* pMetrics)
{
	if ( pMetrics == NULL ) return 0;
	if ( (pMetrics->iSize != 0) && (pMetrics->iSize < sizeof(*pMetrics)) ) return 0;
	return __xuiToastFloatValid(pMetrics->fWidth) &&
	       __xuiToastFloatValid(pMetrics->fMinWidth) &&
	       __xuiToastFloatValid(pMetrics->fMinHeight) &&
	       __xuiToastFloatValid(pMetrics->fMargin) &&
	       __xuiToastFloatValid(pMetrics->fGap) &&
	       __xuiToastFloatValid(pMetrics->fPaddingX) &&
	       __xuiToastFloatValid(pMetrics->fPaddingY) &&
	       __xuiToastFloatValid(pMetrics->fIconSize) &&
	       __xuiToastFloatValid(pMetrics->fIconGap) &&
	       __xuiToastFloatValid(pMetrics->fCloseSize) &&
	       __xuiToastFloatValid(pMetrics->fProgressHeight) &&
	       (pMetrics->fMinWidth >= 32.0f) &&
	       (pMetrics->fWidth >= pMetrics->fMinWidth) &&
	       (pMetrics->fMinHeight >= 24.0f) &&
	       (pMetrics->fProgressHeight <= pMetrics->fMinHeight) &&
	       (pMetrics->iMaxVisible >= 0) &&
	       (pMetrics->iMaxVisible <= XUI_TOAST_VISIBLE_CAPACITY);
}

static int __xuiToastColorsValid(const xui_toast_colors_t* pColors)
{
	if ( pColors == NULL ) return 0;
	return (pColors->iSize == 0) || (pColors->iSize >= sizeof(*pColors));
}

static int __xuiToastDescValid(const xui_toast_desc_t* pDesc)
{
	if ( pDesc == NULL ) return 1;
	if ( (pDesc->iSize != 0) && (pDesc->iSize < sizeof(*pDesc)) ) return 0;
	if ( !__xuiToastPlacementValid(pDesc->iPlacement) ||
	     !__xuiToastDirectionValid(pDesc->iDirection) ) return 0;
	if ( pDesc->bHasMetrics && !__xuiToastMetricsValid(&pDesc->tMetrics) ) return 0;
	if ( pDesc->bHasColors && !__xuiToastColorsValid(&pDesc->tColors) ) return 0;
	return 1;
}

static xui_font __xuiToastFont(xui_toast pToast)
{
	if ( pToast == NULL ) return NULL;
	return (pToast->pFont != NULL) ? pToast->pFont : xuiGetDefaultFont(pToast->pContext);
}

static uint32_t __xuiToastTypeColor(const xui_toast_colors_t* pColors, int iType)
{
	if ( pColors == NULL ) return 0;
	switch ( iType ) {
	case XUI_TOAST_TYPE_SUCCESS:
		return pColors->iSuccessColor;
	case XUI_TOAST_TYPE_WARNING:
		return pColors->iWarningColor;
	case XUI_TOAST_TYPE_ERROR:
		return pColors->iErrorColor;
	case XUI_TOAST_TYPE_INFO:
	default:
		return pColors->iInfoColor;
	}
}

static const char* __xuiToastIconName(int iType)
{
	switch ( iType ) {
	case XUI_TOAST_TYPE_SUCCESS:
		return "property_check_12";
	case XUI_TOAST_TYPE_WARNING:
		return "msgbox_war";
	case XUI_TOAST_TYPE_ERROR:
		return "msgbox_error";
	case XUI_TOAST_TYPE_INFO:
	default:
		return "msgbox_info";
	}
}

static xui_rect_t __xuiToastRootRect(xui_toast pToast)
{
	xui_widget pRoot;
	xui_size_t tViewport;
	xui_rect_t tRect;

	tRect = (xui_rect_t){0.0f, 0.0f, 640.0f, 420.0f};
	if ( !__xuiToastValid(pToast) ) {
		return tRect;
	}
	pRoot = xuiGetRootWidget(pToast->pContext);
	if ( pRoot != NULL ) {
		tRect = xuiWidgetGetRect(pRoot);
	}
	if ( (tRect.fW <= 1.0f) || (tRect.fH <= 1.0f) ) {
		tViewport = xuiGetViewportSize(pToast->pContext);
		if ( (tViewport.iW > 1) && (tViewport.iH > 1) ) {
			tRect = (xui_rect_t){0, 0, tViewport.iW, tViewport.iH};
		}
	}
	return tRect;
}

static int __xuiToastMeasureText(xui_toast pToast, const char* sText,
	float fWidth, float fHeight, xui_vec2_t* pSize, int* pLineCount)
{
	xui_text_layout_desc_t tDesc;
	xui_text_layout pLayout;
	xui_font pFont;
	int iRet;

	if ( !__xuiToastValid(pToast) || (pSize == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pSize->fX = 0.0f;
	pSize->fY = 0.0f;
	if ( pLineCount != NULL ) *pLineCount = 0;
	if ( sText == NULL ) sText = "";
	if ( fWidth <= 0.0f || fHeight <= 0.0f || sText[0] == 0 ) {
		return XUI_OK;
	}
	pFont = __xuiToastFont(pToast);
	if ( pFont == NULL ) {
		pSize->fX = __xuiToastMin((float)strlen(sText) * 7.0f, fWidth);
		pSize->fY = __xuiToastMin(18.0f, fHeight);
		if ( pLineCount != NULL ) *pLineCount = 1;
		return XUI_OK;
	}
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.sText = sText;
	tDesc.iTextSize = -1;
	tDesc.pFont = pFont;
	tDesc.fMaxWidth = fWidth;
	tDesc.fMaxHeight = fHeight;
	tDesc.iWrapMode = XUI_TEXT_WRAP_WORD;
	tDesc.iFlags = XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_TOP;
	tDesc.fLineGap = 2.0f;
	pLayout = NULL;
	iRet = xuiTextLayoutCreate(pToast->pContext, &pLayout, &tDesc);
	if ( iRet != XUI_OK ) {
		return iRet;
	}
	*pSize = xuiTextLayoutGetSize(pLayout);
	if ( pLineCount != NULL ) {
		*pLineCount = xuiTextLayoutGetLineCount(pLayout);
	}
	xuiTextLayoutDestroy(pLayout);
	return XUI_OK;
}

static int __xuiToastVisibleLimit(xui_toast pToast)
{
	xui_rect_t tRoot;
	float fGap;
	float fItemHeight;
	float fMargin;
	float fUsable;
	int iAutomaticLimit;
	int iLimit;

	if ( !__xuiToastValid(pToast) ) {
		return 0;
	}
	tRoot = __xuiToastRootRect(pToast);
	fMargin = ((float)tRoot.fH > pToast->tMetrics.fMargin * 2.0f + 1.0f) ?
		pToast->tMetrics.fMargin : 0.0f;
	fUsable = __xuiToastMax(1.0f, (float)tRoot.fH - fMargin * 2.0f);
	fItemHeight = __xuiToastMin(pToast->tMetrics.fMinHeight, fUsable);
	fGap = __xuiToastMin(pToast->tMetrics.fGap,
		__xuiToastMax(0.0f, fUsable - fItemHeight));
	iAutomaticLimit = (int)((fUsable + fGap) / (fItemHeight + fGap));
	if ( iAutomaticLimit < 1 ) iAutomaticLimit = 1;
	iLimit = iAutomaticLimit;
	if ( pToast->tMetrics.iMaxVisible > 0 &&
	     pToast->tMetrics.iMaxVisible < iLimit ) {
		iLimit = pToast->tMetrics.iMaxVisible;
	}
	if ( iLimit < 1 ) iLimit = 1;
	if ( iLimit > XUI_TOAST_VISIBLE_CAPACITY ) iLimit = XUI_TOAST_VISIBLE_CAPACITY;
	return iLimit;
}

static int __xuiToastInvalidateActive(xui_toast pToast)
{
	int i;
	int iRet;

	if ( !__xuiToastValid(pToast) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	for ( i = 0; i < XUI_TOAST_VISIBLE_CAPACITY; i++ ) {
		if ( pToast->arrWidgets[i] == NULL ) {
			continue;
		}
		iRet = xuiWidgetInvalidate(pToast->arrWidgets[i], XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
		if ( iRet != XUI_OK ) {
			return iRet;
		}
	}
	return XUI_OK;
}

static int __xuiToastLayout(xui_toast pToast)
{
	xui_toast_item_t* pItem;
	xui_rect_t tRoot;
	xui_vec2_t tTitleSize;
	xui_vec2_t tMessageSize;
	float fAfterClose;
	float fAvailableH;
	float fAvailableW;
	float fCloseGap;
	float fCloseSize;
	float fWidth;
	float fTextX;
	float fCloseX;
	float fTextW;
	float fTitleH;
	float fTitleGap;
	float fMessageH;
	float fBodyH;
	float fDesiredH;
	float fIconGap;
	float fIconSize;
	float fInnerW;
	float fItemMaxH;
	float fItemMinH;
	float fMarginX;
	float fMarginY;
	float fMaxContentH;
	float fPaddingX;
	float fPaddingY;
	float fRemainingH;
	float fReservedH;
	float fStackGap;
	float fStackH;
	float fX;
	float fY;
	float fContentY;
	int iDirection;
	int iRemaining;
	int i;
	int iRet;

	if ( !__xuiToastValid(pToast) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	tRoot = __xuiToastRootRect(pToast);
	if ( tRoot.fW <= 0 || tRoot.fH <= 0 ) {
		for ( i = 0; i < XUI_TOAST_VISIBLE_CAPACITY; i++ ) {
			if ( pToast->arrWidgets[i] == NULL ) continue;
			iRet = xuiWidgetSetVisible(pToast->arrWidgets[i], 0);
			if ( iRet == XUI_OK ) iRet = xuiWidgetSetEnabled(pToast->arrWidgets[i], 0);
			if ( iRet == XUI_OK ) iRet = xuiWidgetSetHitTestVisible(pToast->arrWidgets[i], 0);
			if ( iRet != XUI_OK ) return iRet;
		}
		pToast->tLayoutRootRect = tRoot;
		return XUI_OK;
	}
	fMarginX = ((float)tRoot.fW > pToast->tMetrics.fMargin * 2.0f + 1.0f) ?
		pToast->tMetrics.fMargin : 0.0f;
	fMarginY = ((float)tRoot.fH > pToast->tMetrics.fMargin * 2.0f + 1.0f) ?
		pToast->tMetrics.fMargin : 0.0f;
	fAvailableW = __xuiToastMax(1.0f, (float)tRoot.fW - fMarginX * 2.0f);
	fAvailableH = __xuiToastMax(1.0f, (float)tRoot.fH - fMarginY * 2.0f);
	fWidth = __xuiToastClamp(pToast->tMetrics.fWidth,
		__xuiToastMin(pToast->tMetrics.fMinWidth, fAvailableW), fAvailableW);
	fPaddingX = __xuiToastMin(pToast->tMetrics.fPaddingX, fWidth * 0.25f);
	fInnerW = __xuiToastMax(0.0f, fWidth - fPaddingX * 2.0f);
	fCloseSize = __xuiToastMin(pToast->tMetrics.fCloseSize, fInnerW);
	fCloseGap = (fCloseSize > 0.0f) ? __xuiToastMin(8.0f,
		__xuiToastMax(0.0f, fInnerW - fCloseSize)) : 0.0f;
	fAfterClose = __xuiToastMax(0.0f, fInnerW - fCloseSize - fCloseGap);
	fIconSize = __xuiToastMin(pToast->tMetrics.fIconSize,
		__xuiToastMax(0.0f, fAfterClose - __xuiToastMin(32.0f, fAfterClose)));
	fIconGap = (fIconSize > 0.0f) ? __xuiToastMin(pToast->tMetrics.fIconGap,
		__xuiToastMax(0.0f, fAfterClose - fIconSize - __xuiToastMin(32.0f, fAfterClose))) : 0.0f;
	fTextX = fPaddingX + fIconSize + fIconGap;
	fCloseX = fWidth - fPaddingX - fCloseSize;
	fTextW = __xuiToastMax(0.0f, fCloseX - fCloseGap - fTextX);
	if ( pToast->iActiveCount > 1 ) {
		fStackGap = __xuiToastMin(pToast->tMetrics.fGap,
			__xuiToastMax(0.0f,
				(fAvailableH - (float)pToast->iActiveCount) /
				(float)(pToast->iActiveCount - 1)));
	} else {
		fStackGap = 0.0f;
	}
	fItemMinH = (pToast->iActiveCount > 0) ?
		__xuiToastMin(pToast->tMetrics.fMinHeight,
			__xuiToastMax(1.0f,
				(fAvailableH - fStackGap * (float)(pToast->iActiveCount - 1)) /
				(float)pToast->iActiveCount)) : 1.0f;
	fPaddingY = __xuiToastMin(pToast->tMetrics.fPaddingY, fItemMinH * 0.25f);
	fRemainingH = fAvailableH;
	fStackH = 0.0f;
	for ( i = 0; i < pToast->iActiveCount; i++ ) {
		pItem = &pToast->arrActive[i];
		iRemaining = pToast->iActiveCount - i - 1;
		fReservedH = (float)iRemaining * (fItemMinH + fStackGap);
		fItemMaxH = __xuiToastMax(1.0f, fRemainingH - fReservedH);
		fMaxContentH = __xuiToastMax(0.0f, fItemMaxH - fPaddingY * 2.0f);
		tTitleSize = (xui_vec2_t){0.0f, 0.0f};
		tMessageSize = (xui_vec2_t){0.0f, 0.0f};
		fTitleH = 0.0f;
		fMessageH = 0.0f;
		fTitleGap = 0.0f;
		if ( (pItem->sTitle != NULL) && (pItem->sTitle[0] != 0) &&
		     fTextW > 0.0f && fMaxContentH > 0.0f ) {
			iRet = __xuiToastMeasureText(pToast, pItem->sTitle, fTextW,
				fMaxContentH, &tTitleSize, NULL);
			if ( iRet != XUI_OK ) return iRet;
			fTitleH = __xuiToastMin(__xuiToastMax(tTitleSize.fY, 16.0f),
				fMaxContentH);
		}
		if ( fTitleH > 0.0f && (pItem->sMessage != NULL) &&
		     (pItem->sMessage[0] != 0) ) {
			fTitleGap = __xuiToastMin(2.0f,
				__xuiToastMax(0.0f, fMaxContentH - fTitleH));
		}
		if ( (pItem->sMessage != NULL) && (pItem->sMessage[0] != 0) &&
		     fTextW > 0.0f && fMaxContentH > fTitleH + fTitleGap ) {
			iRet = __xuiToastMeasureText(pToast, pItem->sMessage, fTextW,
				fMaxContentH - fTitleH - fTitleGap,
				&tMessageSize, &pItem->iMessageLineCount);
			if ( iRet != XUI_OK ) return iRet;
			fMessageH = tMessageSize.fY;
		} else {
			pItem->iMessageLineCount = 0;
		}
		if ( fMessageH <= 0.0f ) fTitleGap = 0.0f;
		fBodyH = __xuiToastMax(fTitleH + fTitleGap + fMessageH,
			__xuiToastMin(fIconSize, fMaxContentH));
		fDesiredH = __xuiToastMax(fItemMinH, fBodyH + fPaddingY * 2.0f);
		pItem->fHeight = __xuiToastClamp(fDesiredH, 1.0f, fItemMaxH);
		if ( fIconSize > 0.0f && pItem->fHeight > fPaddingY * 2.0f ) {
			float fRenderIconSize = __xuiToastMin(fIconSize,
				pItem->fHeight - fPaddingY * 2.0f);
			pItem->tIconRect = xuiInternalRectFromFloatNearest(
				fPaddingX, (pItem->fHeight - fRenderIconSize) * 0.5f,
				fRenderIconSize, fRenderIconSize);
		} else {
			pItem->tIconRect = (xui_rect_t){0, 0, 0, 0};
		}
		if ( fCloseSize > 0.0f ) {
			float fRenderCloseSize = __xuiToastMin(fCloseSize, pItem->fHeight);
			pItem->tCloseRect = xuiInternalRectFromFloatNearest(
				fWidth - fPaddingX - fRenderCloseSize,
				(pItem->fHeight - fRenderCloseSize) * 0.5f,
				fRenderCloseSize, fRenderCloseSize);
		} else {
			pItem->tCloseRect = (xui_rect_t){0, 0, 0, 0};
		}
		fContentY = (pItem->fHeight - fBodyH) * 0.5f;
		if ( fTitleH > 0.0f ) {
			pItem->tTitleRect = xuiInternalRectFromFloatNearest(
				fTextX, fContentY, fTextW, fTitleH);
			if ( fMessageH > 0.0f ) {
				pItem->tMessageRect = xuiInternalRectFromFloatNearest(
					fTextX, fContentY + fTitleH + fTitleGap,
					fTextW, fMessageH);
			} else {
				pItem->tMessageRect = (xui_rect_t){0, 0, 0, 0};
			}
		} else {
			pItem->tTitleRect = (xui_rect_t){0, 0, 0, 0};
			pItem->tMessageRect = (fMessageH > 0.0f) ?
				xuiInternalRectFromFloatNearest(fTextX,
					(pItem->fHeight - fMessageH) * 0.5f,
					fTextW, fMessageH) : (xui_rect_t){0, 0, 0, 0};
		}
		fStackH += pItem->fHeight;
		fRemainingH -= pItem->fHeight;
		if ( i + 1 < pToast->iActiveCount ) {
			fStackH += fStackGap;
			fRemainingH -= fStackGap;
		}
	}
	if ( pToast->iPlacement == XUI_TOAST_PLACEMENT_TOP_LEFT || pToast->iPlacement == XUI_TOAST_PLACEMENT_BOTTOM_LEFT ) {
		fX = (float)tRoot.fX + fMarginX;
	} else if ( pToast->iPlacement == XUI_TOAST_PLACEMENT_TOP_CENTER || pToast->iPlacement == XUI_TOAST_PLACEMENT_BOTTOM_CENTER ) {
		fX = (float)tRoot.fX + ((float)tRoot.fW - fWidth) * 0.5f;
	} else {
		fX = (float)(tRoot.fX + tRoot.fW) - fMarginX - fWidth;
	}
	if ( fX < tRoot.fX ) fX = tRoot.fX;
	if ( fX + fWidth > tRoot.fX + tRoot.fW ) fX = tRoot.fX + tRoot.fW - fWidth;
	iDirection = __xuiToastDirection(pToast);
	if ( iDirection == XUI_TOAST_DIRECTION_UP ) {
		fY = (float)(tRoot.fY + tRoot.fH) - fMarginY - fStackH;
	} else {
		fY = (float)tRoot.fY + fMarginY;
	}
	if ( fY < tRoot.fY ) fY = tRoot.fY;
	for ( i = 0; i < pToast->iActiveCount; i++ ) {
		pItem = &pToast->arrActive[i];
		pItem->tRect = xuiInternalRectFromFloatNearest(fX, fY,
			fWidth, pItem->fHeight);
		if ( pToast->arrWidgets[i] != NULL ) {
			iRet = xuiWidgetSetRect(pToast->arrWidgets[i], pItem->tRect);
			if ( iRet == XUI_OK ) iRet = xuiWidgetSetVisible(pToast->arrWidgets[i], 1);
			if ( iRet == XUI_OK ) iRet = xuiWidgetSetEnabled(pToast->arrWidgets[i], 1);
			if ( iRet == XUI_OK ) iRet = xuiWidgetSetHitTestVisible(pToast->arrWidgets[i], 1);
			if ( iRet == XUI_OK ) iRet = xuiWidgetSetLayer(pToast->arrWidgets[i], XUI_LAYER_TOOLTIP, 120 + i);
			if ( iRet != XUI_OK ) return iRet;
		}
		fY += pItem->fHeight + fStackGap;
	}
	for ( i = pToast->iActiveCount; i < XUI_TOAST_VISIBLE_CAPACITY; i++ ) {
		if ( pToast->arrWidgets[i] != NULL ) {
			iRet = xuiWidgetSetVisible(pToast->arrWidgets[i], 0);
			if ( iRet == XUI_OK ) iRet = xuiWidgetSetEnabled(pToast->arrWidgets[i], 0);
			if ( iRet == XUI_OK ) iRet = xuiWidgetSetHitTestVisible(pToast->arrWidgets[i], 0);
			if ( iRet == XUI_OK ) iRet = xuiWidgetSetRect(pToast->arrWidgets[i], (xui_rect_t){0, 0, 0, 0});
			if ( iRet != XUI_OK ) return iRet;
		}
	}
	iRet = __xuiToastInvalidateActive(pToast);
	if ( iRet == XUI_OK ) pToast->tLayoutRootRect = tRoot;
	return iRet;
}

static int __xuiToastCallbackLeave(xui_toast pToast)
{
	if ( pToast == NULL ) return 0;
	if ( pToast->iCallbackDepth > 0 ) pToast->iCallbackDepth--;
	if ( pToast->bDestroyPending && pToast->iCallbackDepth == 0 ) {
		xrtFree(pToast);
		return 0;
	}
	return __xuiToastValid(pToast);
}

static int __xuiToastNotifyClose(xui_toast pToast, int iToastId, int iReason)
{
	xui_toast_close_proc onClose;
	void* pUser;

	if ( !__xuiToastValid(pToast) ) return 0;
	onClose = pToast->onClose;
	pUser = pToast->pCloseUser;
	if ( (onClose == NULL) || (iToastId <= 0) ) return 1;
	pToast->iCallbackDepth++;
	onClose(pToast, iToastId, iReason, pUser);
	return __xuiToastCallbackLeave(pToast);
}

static int __xuiToastNotifyClick(xui_toast pToast, xui_toast_click_proc onClick,
	int iToastId, void* pUser)
{
	if ( !__xuiToastValid(pToast) ) return 0;
	if ( (onClick == NULL) || (iToastId <= 0) ) return 1;
	pToast->iCallbackDepth++;
	onClick(pToast, iToastId, pUser);
	return __xuiToastCallbackLeave(pToast);
}

static int __xuiToastFindActiveById(xui_toast pToast, int iToastId)
{
	int i;

	if ( !__xuiToastValid(pToast) || (iToastId <= 0) ) return -1;
	for ( i = 0; i < pToast->iActiveCount; i++ ) {
		if ( pToast->arrActive[i].iId == iToastId ) return i;
	}
	return -1;
}

static int __xuiToastFindPendingById(xui_toast pToast, int iToastId)
{
	int i;

	if ( !__xuiToastValid(pToast) || (iToastId <= 0) ) return -1;
	for ( i = 0; i < pToast->iPendingCount; i++ ) {
		if ( pToast->arrPending[i].iId == iToastId ) return i;
	}
	return -1;
}

static int __xuiToastAllocateId(xui_toast pToast)
{
	int iCandidate;
	int iAttempts;

	if ( !__xuiToastValid(pToast) ) return 0;
	iCandidate = (pToast->iNextId > 0) ? pToast->iNextId : 1;
	for ( iAttempts = 0;
	      iAttempts <= XUI_TOAST_VISIBLE_CAPACITY + XUI_TOAST_QUEUE_CAPACITY;
	      iAttempts++ ) {
		if ( (__xuiToastFindActiveById(pToast, iCandidate) < 0) &&
		     (__xuiToastFindPendingById(pToast, iCandidate) < 0) ) {
			pToast->iNextId = (iCandidate == INT_MAX) ? 1 : iCandidate + 1;
			return iCandidate;
		}
		iCandidate = (iCandidate == INT_MAX) ? 1 : iCandidate + 1;
	}
	return 0;
}

static int __xuiToastDropPendingAt(xui_toast pToast, int iIndex)
{
	int iToastId;
	int i;

	if ( !__xuiToastValid(pToast) || (iIndex < 0) || (iIndex >= pToast->iPendingCount) ) return 0;
	iToastId = pToast->arrPending[iIndex].iId;
	__xuiToastItemFree(&pToast->arrPending[iIndex]);
	for ( i = iIndex; i + 1 < pToast->iPendingCount; i++ ) {
		__xuiToastItemMove(&pToast->arrPending[i], &pToast->arrPending[i + 1]);
	}
	pToast->iPendingCount--;
	__xuiToastCountAdd(&pToast->iDropCount, 1);
	__xuiToastCountAdd(&pToast->iChangeCount, 1);
	return iToastId;
}

static int __xuiToastPromotePending(xui_toast pToast)
{
	int iLimit;
	int iPromoted;
	int i;

	if ( !__xuiToastValid(pToast) ) {
		return -1;
	}
	iLimit = __xuiToastVisibleLimit(pToast);
	iPromoted = 0;
	while ( (pToast->iActiveCount < iLimit) && (pToast->iPendingCount > 0) ) {
		__xuiToastItemMove(&pToast->arrActive[pToast->iActiveCount], &pToast->arrPending[0]);
		pToast->arrActive[pToast->iActiveCount].fElapsed = 0.0f;
		pToast->iActiveCount++;
		iPromoted++;
		for ( i = 0; i + 1 < pToast->iPendingCount; i++ ) {
			__xuiToastItemMove(&pToast->arrPending[i], &pToast->arrPending[i + 1]);
		}
		pToast->iPendingCount--;
	}
	return iPromoted;
}

static int __xuiToastSyncLayout(xui_toast pToast)
{
	xui_rect_t tRoot;

	if ( !__xuiToastValid(pToast) ) return XUI_ERROR_INVALID_ARGUMENT;
	tRoot = __xuiToastRootRect(pToast);
	if ( __xuiToastRectSame(tRoot, pToast->tLayoutRootRect) ) return XUI_OK;
	if ( __xuiToastPromotePending(pToast) < 0 ) return XUI_ERROR_INVALID_ARGUMENT;
	return __xuiToastLayout(pToast);
}

static int __xuiToastRemoveActiveAt(xui_toast pToast, int iIndex, int iReason, int* pAlive)
{
	int iToastId;
	int i;
	int iRet;

	if ( !__xuiToastValid(pToast) || (iIndex < 0) || (iIndex >= pToast->iActiveCount) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( pAlive != NULL ) *pAlive = 1;
	iToastId = pToast->arrActive[iIndex].iId;
	__xuiToastItemFree(&pToast->arrActive[iIndex]);
	for ( i = iIndex; i + 1 < pToast->iActiveCount; i++ ) {
		__xuiToastItemMove(&pToast->arrActive[i], &pToast->arrActive[i + 1]);
	}
	pToast->iActiveCount--;
	pToast->iHoverItem = -1;
	pToast->iHoverClose = -1;
	if ( iReason == XUI_TOAST_CLOSE_TIMEOUT ) {
		__xuiToastCountAdd(&pToast->iExpireCount, 1);
	} else {
		__xuiToastCountAdd(&pToast->iCloseCount, 1);
	}
	__xuiToastCountAdd(&pToast->iChangeCount, 1);
	(void)__xuiToastPromotePending(pToast);
	iRet = __xuiToastLayout(pToast);
	if ( !__xuiToastNotifyClose(pToast, iToastId, iReason) && pAlive != NULL ) *pAlive = 0;
	return iRet;
}

static int __xuiToastRemovePendingAt(xui_toast pToast, int iIndex, int iReason, int* pAlive)
{
	int iToastId;
	int i;
	int iRet;

	if ( !__xuiToastValid(pToast) || (iIndex < 0) || (iIndex >= pToast->iPendingCount) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( pAlive != NULL ) *pAlive = 1;
	iToastId = pToast->arrPending[iIndex].iId;
	__xuiToastItemFree(&pToast->arrPending[iIndex]);
	for ( i = iIndex; i + 1 < pToast->iPendingCount; i++ ) {
		__xuiToastItemMove(&pToast->arrPending[i], &pToast->arrPending[i + 1]);
	}
	pToast->iPendingCount--;
	__xuiToastCountAdd(&pToast->iCloseCount, 1);
	__xuiToastCountAdd(&pToast->iChangeCount, 1);
	iRet = XUI_OK;
	if ( !__xuiToastNotifyClose(pToast, iToastId, iReason) && pAlive != NULL ) *pAlive = 0;
	return iRet;
}

static int __xuiToastRequestActiveClose(xui_toast pToast, int iIndex, int iReason)
{
	xui_toast_item_t* pItem;
	int iOldHoverItem;
	int iOldHoverClose;
	int iRet;

	if ( !__xuiToastValid(pToast) || (iIndex < 0) || (iIndex >= pToast->iActiveCount) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pItem = &pToast->arrActive[iIndex];
	if ( pItem->bClosing ) {
		return XUI_OK;
	}
	pItem->bClosing = 1;
	pItem->iCloseReason = iReason;
	iOldHoverItem = pToast->iHoverItem;
	iOldHoverClose = pToast->iHoverClose;
	pToast->iHoverItem = -1;
	pToast->iHoverClose = -1;
	if ( pToast->arrWidgets[iIndex] != NULL ) {
		iRet = xuiWidgetInvalidate(pToast->arrWidgets[iIndex], XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
		if ( iRet != XUI_OK ) {
			pItem->bClosing = 0;
			pItem->iCloseReason = 0;
			pToast->iHoverItem = iOldHoverItem;
			pToast->iHoverClose = iOldHoverClose;
		}
		return iRet;
	}
	return XUI_OK;
}

static int __xuiToastProcessClosing(xui_toast pToast, int* pAlive)
{
	xui_toast_click_proc onClick;
	void* pUser;
	int bAlive;
	int iId;
	int iReason;
	int iIndex;
	int i;
	int iRet;

	if ( !__xuiToastValid(pToast) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( pAlive != NULL ) *pAlive = 1;
	i = 0;
	while ( i < pToast->iActiveCount ) {
		if ( !pToast->arrActive[i].bClosing ) {
			i++;
			continue;
		}
		iId = pToast->arrActive[i].iId;
		iReason = pToast->arrActive[i].iCloseReason;
		onClick = (iReason == XUI_TOAST_CLOSE_CLICK) ? pToast->arrActive[i].onClick : NULL;
		pUser = pToast->arrActive[i].pUser;
		if ( !__xuiToastNotifyClick(pToast, onClick, iId, pUser) ) {
			if ( pAlive != NULL ) *pAlive = 0;
			return XUI_OK;
		}
		iIndex = __xuiToastFindActiveById(pToast, iId);
		if ( iIndex >= 0 ) {
			bAlive = 1;
			iRet = __xuiToastRemoveActiveAt(pToast, iIndex, iReason, &bAlive);
			if ( !bAlive ) {
				if ( pAlive != NULL ) *pAlive = 0;
				return iRet;
			}
			if ( iRet != XUI_OK ) return iRet;
			i = 0;
			continue;
		}
		i = 0;
	}
	return XUI_OK;
}

static int __xuiToastUpdateService(xui_toast pToast, float fDelta)
{
	int arrActiveIds[XUI_TOAST_VISIBLE_CAPACITY];
	int iActiveCount;
	int iIndex;
	int bAlive;
	int i;
	int iRet;

	if ( !__xuiToastValid(pToast) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	iRet = __xuiToastSyncLayout(pToast);
	if ( iRet != XUI_OK ) return iRet;
	bAlive = 1;
	iRet = __xuiToastProcessClosing(pToast, &bAlive);
	if ( iRet != XUI_OK ) return iRet;
	if ( !bAlive ) return XUI_OK;
	if ( (fDelta == fDelta) && fDelta > 0.0f ) {
		iActiveCount = pToast->iActiveCount;
		for ( i = 0; i < iActiveCount; i++ ) {
			arrActiveIds[i] = pToast->arrActive[i].iId;
		}
		for ( i = 0; i < iActiveCount; i++ ) {
			iIndex = __xuiToastFindActiveById(pToast, arrActiveIds[i]);
			if ( iIndex < 0 || pToast->arrActive[iIndex].bClosing ||
			     pToast->arrActive[iIndex].fDuration <= 0.0f ||
			     pToast->iHoverItem == iIndex ) {
				continue;
			}
			if ( pToast->arrActive[iIndex].fElapsed >= pToast->arrActive[iIndex].fDuration ||
			     fDelta >= pToast->arrActive[iIndex].fDuration - pToast->arrActive[iIndex].fElapsed ) {
				bAlive = 1;
				iRet = __xuiToastRemoveActiveAt(pToast, iIndex,
					XUI_TOAST_CLOSE_TIMEOUT, &bAlive);
				if ( !bAlive ) return iRet;
				if ( iRet != XUI_OK ) return iRet;
				continue;
			}
			pToast->arrActive[iIndex].fElapsed += fDelta;
			if ( pToast->arrWidgets[iIndex] != NULL ) {
				iRet = xuiWidgetInvalidate(pToast->arrWidgets[iIndex],
					XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
				if ( iRet != XUI_OK ) return iRet;
			}
		}
	}
	return XUI_OK;
}

static int __xuiToastDrawTextLayout(xui_toast pToast, xui_draw_context pDraw,
	const char* sText, xui_rect_t tRect, uint32_t iColor, uint32_t iFlags)
{
	xui_text_layout_desc_t tDesc;
	xui_text_layout pLayout;
	xui_proxy pProxy;
	xui_font pFont;
	int iRet;

	if ( !__xuiToastValid(pToast) || (pDraw == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( (sText == NULL) || (sText[0] == 0) ||
	     (tRect.fW <= 0) || (tRect.fH <= 0) ) {
		return XUI_OK;
	}
	pProxy = xuiInternalContextGetProxy(pToast->pContext);
	pFont = __xuiToastFont(pToast);
	if ( (pProxy == NULL) || (pProxy->drawText == NULL) || (pFont == NULL) ) {
		return XUI_OK;
	}
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.sText = sText;
	tDesc.iTextSize = -1;
	tDesc.pFont = pFont;
	tDesc.fMaxWidth = tRect.fW;
	tDesc.fMaxHeight = tRect.fH;
	tDesc.iWrapMode = XUI_TEXT_WRAP_WORD;
	tDesc.iFlags = iFlags;
	tDesc.fLineGap = 2.0f;
	pLayout = NULL;
	iRet = xuiTextLayoutCreate(pToast->pContext, &pLayout, &tDesc);
	if ( iRet != XUI_OK ) {
		return iRet;
	}
	iRet = xuiInternalTextLayoutDrawContext(pLayout, pDraw, tRect, iColor, 0);
	xuiTextLayoutDestroy(pLayout);
	return iRet;
}

static int __xuiToastDrawAtlas(xui_toast pToast, xui_draw_context pDraw, const char* sName, xui_rect_t tDst, uint32_t iColor)
{
	xui_proxy pProxy;
	xui_surface pAtlas;
	xui_rect_t tSrc;
	int iRet;

	if ( !__xuiToastValid(pToast) || (pDraw == NULL) || (sName == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( tDst.fW <= 0 || tDst.fH <= 0 ) return XUI_OK;
	pProxy = xuiInternalContextGetProxy(pToast->pContext);
	if ( (pProxy == NULL) || (pProxy->drawSurface == NULL) ) {
		return XUI_ERROR_UNSUPPORTED;
	}
	pAtlas = NULL;
	iRet = xuiBuiltinAssetGetAtlas(pToast->pContext, &pAtlas);
	if ( iRet == XUI_OK ) {
		iRet = xuiBuiltinAssetGetRect(sName, &tSrc);
	}
	if ( (iRet == XUI_OK) && (pAtlas != NULL) ) {
		return pProxy->drawSurface(pProxy, pDraw, pAtlas, tSrc, xuiInternalSnapRect(tDst), iColor, 0);
	}
	return iRet;
}

static int __xuiToastRender(xui_widget pWidget, xui_draw_context pDraw, uint32_t iStateId, void* pUser)
{
	xui_toast_item_view_t* pView;
	xui_toast pToast;
	xui_toast_colors_t tColors;
	xui_toast_item_t* pItem;
	xui_proxy pProxy;
	xui_rect_t tRect;
	xui_rect_t tShadow;
	xui_rect_t tBand;
	xui_rect_t tIconBg;
	xui_rect_t tIcon;
	xui_rect_t tClose;
	xui_rect_t tProgress;
	uint32_t iTypeColor;
	uint32_t iCloseColor;
	float fIconInset;
	float fProgress;
	int iRet;

	(void)iStateId;
	pView = (xui_toast_item_view_t*)pUser;
	pToast = (pView != NULL) ? pView->pToast : NULL;
	if ( !__xuiToastValid(pToast) || (pView->iSlot < 0) || (pView->iSlot >= pToast->iActiveCount) || (pDraw == NULL) ) {
		return XUI_OK;
	}
	pItem = &pToast->arrActive[pView->iSlot];
	pProxy = xuiInternalContextGetProxy(pToast->pContext);
	if ( pProxy == NULL ) {
		return XUI_OK;
	}
	tRect = xuiInternalSnapRect((xui_rect_t){0.0f, 0.0f, pItem->tRect.fW, pItem->tRect.fH});
	if ( tRect.fW <= 0 || tRect.fH <= 0 ) return XUI_OK;
	__xuiToastResolveColors(pWidget, pToast, &tColors);
	iTypeColor = __xuiToastTypeColor(&tColors, pItem->iType);
	if ( (__xuiToastAlpha(tColors.iShadowColor) != 0) && (pProxy->drawRectFill != NULL) ) {
		tShadow = tRect;
		iRet = pProxy->drawRectFill(pProxy, pDraw, tShadow, tColors.iShadowColor);
		if ( iRet != XUI_OK ) return iRet;
		tRect.fW = __xuiToastMax(1.0f, tRect.fW - 2.0f);
		tRect.fH = __xuiToastMax(1.0f, tRect.fH - 2.0f);
	}
	if ( pProxy->drawRectFill != NULL ) {
		iRet = pProxy->drawRectFill(pProxy, pDraw, tRect, tColors.iBackgroundColor);
		if ( iRet != XUI_OK ) return iRet;
	}
	if ( (__xuiToastAlpha(tColors.iBorderColor) != 0) && (pProxy->drawRectStroke != NULL) ) {
		iRet = pProxy->drawRectStroke(pProxy, pDraw, tRect, 1.0f, tColors.iBorderColor);
		if ( iRet != XUI_OK ) return iRet;
	}
	if ( pProxy->drawRectFill != NULL ) {
		tBand = (xui_rect_t){0, 0, (tRect.fW < 4) ? tRect.fW : 4, tRect.fH};
		iRet = pProxy->drawRectFill(pProxy, pDraw, tBand, iTypeColor);
		if ( iRet != XUI_OK ) return iRet;
	}
	tIconBg = xuiInternalSnapRect(pItem->tIconRect);
	if ( tIconBg.fW > 0 && tIconBg.fH > 0 && pProxy->drawRectFill != NULL ) {
		iRet = pProxy->drawRectFill(pProxy, pDraw, tIconBg, __xuiToastStyleColor(pWidget, "toast.icon.background.color", __xuiToastColorWithAlpha(iTypeColor, 32)));
		if ( iRet != XUI_OK ) return iRet;
	}
	fIconInset = __xuiToastMin(3.0f,
		__xuiToastMax(0.0f, (float)pItem->tIconRect.fW * 0.1f));
	tIcon = xuiInternalInsetRect(pItem->tIconRect, fIconInset);
	iRet = __xuiToastDrawAtlas(pToast, pDraw,
		__xuiToastIconName(pItem->iType), tIcon, iTypeColor);
	if ( iRet != XUI_OK && pItem->tIconRect.fW > 0 && pItem->tIconRect.fH > 0 ) {
		if ( pProxy->drawCircleFill != NULL ) {
			iRet = pProxy->drawCircleFill(pProxy, pDraw, pItem->tIconRect.fX + pItem->tIconRect.fW * 0.5f, pItem->tIconRect.fY + pItem->tIconRect.fH * 0.5f, pItem->tIconRect.fW * 0.28f, iTypeColor);
			if ( iRet != XUI_OK ) return iRet;
		}
	}
	if ( (pProxy->drawText != NULL) && (__xuiToastFont(pToast) != NULL) ) {
		if ( (pItem->sTitle != NULL) && (pItem->sTitle[0] != 0) ) {
			iRet = __xuiToastDrawTextLayout(pToast, pDraw, pItem->sTitle, pItem->tTitleRect,
				tColors.iTextColor, XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP);
			if ( iRet != XUI_OK ) return iRet;
		}
		iRet = __xuiToastDrawTextLayout(pToast, pDraw, pItem->sMessage, pItem->tMessageRect,
			tColors.iMutedTextColor, XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_TOP | XUI_TEXT_CLIP);
		if ( iRet != XUI_OK ) return iRet;
	}
	iCloseColor = (pView->iSlot == pToast->iHoverClose) ? tColors.iCloseHoverColor : tColors.iCloseColor;
	tClose = xuiInternalInsetRect(pItem->tCloseRect,
		__xuiToastMax(0.0f, ((float)pItem->tCloseRect.fW - 10.0f) * 0.5f));
	iRet = __xuiToastDrawAtlas(pToast, pDraw, "clear_10", tClose, iCloseColor);
	if ( iRet != XUI_OK && tClose.fW > 0 && tClose.fH > 0 &&
	     pProxy->drawLine != NULL ) {
		iRet = pProxy->drawLine(pProxy, pDraw, tClose.fX, tClose.fY, tClose.fX + tClose.fW, tClose.fY + tClose.fH, 1.0f, iCloseColor);
		if ( iRet == XUI_OK ) iRet = pProxy->drawLine(pProxy, pDraw, tClose.fX + tClose.fW, tClose.fY, tClose.fX, tClose.fY + tClose.fH, 1.0f, iCloseColor);
		if ( iRet != XUI_OK ) return iRet;
	}
	if ( (pProxy->drawRectFill != NULL) && (pItem->fDuration > 0.0f) &&
	     (pToast->tMetrics.fProgressHeight > 0.0f) && (tRect.fW > 4) ) {
		float fProgressHeight;
		fProgress = 1.0f - (pItem->fElapsed / pItem->fDuration);
		if ( fProgress < 0.0f ) fProgress = 0.0f;
		if ( fProgress > 1.0f ) fProgress = 1.0f;
		fProgressHeight = __xuiToastMin(pToast->tMetrics.fProgressHeight,
			(float)tRect.fH);
		tProgress = xuiInternalRectFromFloatNearest(4.0f,
			(float)tRect.fH - fProgressHeight,
			((float)tRect.fW - 4.0f) * fProgress, fProgressHeight);
		if ( tProgress.fW > 0 && tProgress.fH > 0 ) {
			iRet = pProxy->drawRectFill(pProxy, pDraw, tProgress,
				__xuiToastStyleColor(pWidget, "toast.progress.color",
					__xuiToastColorWithAlpha(iTypeColor, 96)));
			if ( iRet != XUI_OK ) return iRet;
		}
	}
	(void)pWidget;
	return XUI_OK;
}

static int __xuiToastUpdate(xui_widget pWidget, float fDelta, void* pUser)
{
	xui_toast_item_view_t* pView;

	(void)pWidget;
	pView = (xui_toast_item_view_t*)pUser;
	if ( (pView == NULL) || (pView->iSlot != 0) ) {
		return XUI_OK;
	}
	return __xuiToastUpdateService(pView->pToast, fDelta);
}

static int __xuiToastPointInRect(xui_rect_t tRect, float fX, float fY)
{
	return (fX >= tRect.fX) && (fY >= tRect.fY) && (fX < tRect.fX + tRect.fW) && (fY < tRect.fY + tRect.fH);
}

static int __xuiToastEvent(xui_widget pWidget, const xui_event_t* pEvent, void* pUser)
{
	xui_toast_item_view_t* pView;
	xui_toast pToast;
	xui_toast_item_t* pItem;
	xui_rect_t tWorld;
	float fLocalX;
	float fLocalY;
	int bClose;
	int iRet;

	pView = (xui_toast_item_view_t*)pUser;
	pToast = (pView != NULL) ? pView->pToast : NULL;
	if ( !__xuiToastValid(pToast) || (pEvent == NULL) || (pView->iSlot < 0) || (pView->iSlot >= pToast->iActiveCount) ) {
		return XUI_OK;
	}
	pItem = &pToast->arrActive[pView->iSlot];
	tWorld = xuiWidgetGetWorldRect(pWidget);
	fLocalX = pEvent->fX - tWorld.fX;
	fLocalY = pEvent->fY - tWorld.fY;
	switch ( pEvent->iType ) {
	case XUI_EVENT_POINTER_ENTER:
	case XUI_EVENT_POINTER_MOVE:
		bClose = __xuiToastPointInRect(pItem->tCloseRect, fLocalX, fLocalY);
		if ( (pToast->iHoverItem != pView->iSlot) || (pToast->iHoverClose != (bClose ? pView->iSlot : -1)) ) {
			pToast->iHoverItem = pView->iSlot;
			pToast->iHoverClose = bClose ? pView->iSlot : -1;
			(void)xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
		}
		return XUI_EVENT_DISPATCH_STOP;
	case XUI_EVENT_POINTER_LEAVE:
	case XUI_EVENT_POINTER_CAPTURE_LOST:
		if ( pToast->iHoverItem == pView->iSlot || pToast->iHoverClose == pView->iSlot ) {
			pToast->iHoverItem = -1;
			pToast->iHoverClose = -1;
			(void)xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
		}
		return XUI_OK;
	case XUI_EVENT_POINTER_CLICK:
		if ( pEvent->iButton != XUI_POINTER_BUTTON_LEFT || pItem->bClosing ) {
			return XUI_EVENT_DISPATCH_STOP;
		}
		bClose = __xuiToastPointInRect(pItem->tCloseRect, fLocalX, fLocalY);
		iRet = __xuiToastRequestActiveClose(pToast, pView->iSlot, bClose ? XUI_TOAST_CLOSE_BUTTON : XUI_TOAST_CLOSE_CLICK);
		if ( iRet < 0 ) return iRet;
		return XUI_EVENT_DISPATCH_STOP;
	default:
		break;
	}
	return XUI_OK;
}

static int __xuiToastTypeInit(xui_widget pWidget, void* pTypeData, const void* pCreateData, void* pUser)
{
	xui_cache_policy_t tPolicy;
	xui_toast_item_view_t* pView;
	const xui_toast_item_view_t* pCreateView;
	int iRet;

	(void)pUser;
	if ( (pWidget == NULL) || (pTypeData == NULL) || (pCreateData == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pView = (xui_toast_item_view_t*)pTypeData;
	pCreateView = (const xui_toast_item_view_t*)pCreateData;
	*pView = *pCreateView;
	memset(&tPolicy, 0, sizeof(tPolicy));
	tPolicy.iSize = sizeof(tPolicy);
	tPolicy.iPolicy = XUI_CACHE_POLICY_SELF;
	tPolicy.iFlags = XUI_CACHE_CLEAR_ON_UPDATE | XUI_CACHE_UPDATE_ALL_STATES;
	tPolicy.iClearColor = XUI_COLOR_RGBA(0, 0, 0, 0);
	iRet = xuiWidgetSetLayoutType(pWidget, XUI_LAYOUT_MANUAL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetFlowMode(pWidget, XUI_FLOW_ABSOLUTE);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetOverflow(pWidget, XUI_OVERFLOW_VISIBLE);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetFocusable(pWidget, 0);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetTabStop(pWidget, 0);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetVisible(pWidget, 0);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEnabled(pWidget, 0);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetHitTestVisible(pWidget, 0);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetLayer(pWidget, XUI_LAYER_TOOLTIP, 120 + pView->iSlot);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetCachePolicy(pWidget, &tPolicy);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_POINTER_ENTER, __xuiToastEvent, pTypeData);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_POINTER_LEAVE, __xuiToastEvent, pTypeData);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_POINTER_MOVE, __xuiToastEvent, pTypeData);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_POINTER_CLICK, __xuiToastEvent, pTypeData);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_POINTER_CAPTURE_LOST, __xuiToastEvent, pTypeData);
	return iRet;
}

static void __xuiToastTypeDestroy(xui_widget pWidget, void* pTypeData, void* pUser)
{
	xui_toast_item_view_t* pView;

	(void)pWidget;
	(void)pUser;
	pView = (xui_toast_item_view_t*)pTypeData;
	if ( pView != NULL ) {
		pView->pToast = NULL;
		pView->iSlot = -1;
	}
}

static xui_widget_type __xuiToastEnsureType(xui_context pContext)
{
	xui_widget_type pType;
	xui_widget_type_desc_t tDesc;
	int iRet;

	if ( !xuiInternalContextIsValid(pContext) ) {
		return NULL;
	}
	pType = xuiWidgetFindType(pContext, "toast-item");
	if ( pType != NULL ) {
		return pType;
	}
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.sName = "toast-item";
	tDesc.pParent = xuiWidgetGetBaseType();
	tDesc.iTypeDataSize = sizeof(xui_toast_item_view_t);
	tDesc.onInit = __xuiToastTypeInit;
	tDesc.onDestroy = __xuiToastTypeDestroy;
	tDesc.onCacheRender = __xuiToastRender;
	tDesc.onUpdate = __xuiToastUpdate;
	iRet = xuiWidgetRegisterType(pContext, &pType, &tDesc);
	if ( iRet == XUI_OK ) __xuiToastRegisterColors(pContext, pType);
	if ( iRet == XUI_ERROR_ALREADY_INITIALIZED ) {
		return xuiWidgetFindType(pContext, "toast-item");
	}
	return (iRet == XUI_OK) ? pType : NULL;
}

static int __xuiToastCreateWidgets(xui_toast pToast)
{
	xui_widget_type pType;
	xui_toast_item_view_t tView;
	int i;
	int iRet;

	if ( !__xuiToastValid(pToast) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pType = __xuiToastEnsureType(pToast->pContext);
	if ( pType == NULL ) {
		return XUI_ERROR_NOT_INITIALIZED;
	}
	for ( i = 0; i < XUI_TOAST_VISIBLE_CAPACITY; i++ ) {
		memset(&tView, 0, sizeof(tView));
		tView.pToast = pToast;
		tView.iSlot = i;
		iRet = xuiWidgetCreateTyped(pToast->pContext, pType, &pToast->arrWidgets[i], &tView);
		if ( iRet != XUI_OK ) return iRet;
		iRet = xuiOverlayAttach(pToast->pContext, NULL, pToast->arrWidgets[i], XUI_LAYER_TOOLTIP, 120 + i);
		if ( iRet != XUI_OK ) return iRet;
	}
	return XUI_OK;
}

XUI_API int xuiToastCreate(xui_context pContext, xui_toast* ppToast, const xui_toast_desc_t* pDesc)
{
	xui_toast pToast;
	int iRet;

	if ( ppToast == NULL ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	*ppToast = NULL;
	if ( !xuiInternalContextIsValid(pContext) || !__xuiToastDescValid(pDesc) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pToast = (xui_toast)xrtCalloc(1, sizeof(*pToast));
	if ( pToast == NULL ) {
		return XUI_ERROR_OUT_OF_MEMORY;
	}
	pToast->iMagic = XUI_TOAST_MAGIC;
	pToast->pContext = pContext;
	pToast->pFont = (pDesc != NULL && pDesc->pFont != NULL) ? pDesc->pFont : xuiGetDefaultFont(pContext);
	pToast->iNextId = 1;
	pToast->iPlacement = (pDesc != NULL) ? pDesc->iPlacement : XUI_TOAST_PLACEMENT_TOP_RIGHT;
	pToast->iDirection = (pDesc != NULL) ? pDesc->iDirection : XUI_TOAST_DIRECTION_AUTO;
	pToast->iHoverItem = -1;
	pToast->iHoverClose = -1;
	__xuiToastDefaultMetrics(&pToast->tMetrics);
	__xuiToastDefaultColors(&pToast->tColors);
	if ( pDesc != NULL && pDesc->bHasMetrics ) pToast->tMetrics = pDesc->tMetrics;
	if ( pDesc != NULL && pDesc->bHasColors ) pToast->tColors = pDesc->tColors;
	pToast->tMetrics.iSize = sizeof(pToast->tMetrics);
	pToast->tColors.iSize = sizeof(pToast->tColors);
	iRet = __xuiToastCreateWidgets(pToast);
	if ( iRet != XUI_OK ) {
		xuiToastDestroy(pToast);
		return iRet;
	}
	*ppToast = pToast;
	return XUI_OK;
}

XUI_API void xuiToastDestroy(xui_toast pToast)
{
	int bDeferFree;
	int i;

	if ( !__xuiToastValid(pToast) ) {
		return;
	}
	bDeferFree = pToast->iCallbackDepth > 0;
	pToast->iMagic = 0;
	for ( i = 0; i < XUI_TOAST_VISIBLE_CAPACITY; i++ ) {
		if ( pToast->arrWidgets[i] != NULL ) {
			xuiWidgetDestroy(pToast->arrWidgets[i]);
			pToast->arrWidgets[i] = NULL;
		}
		__xuiToastItemFree(&pToast->arrActive[i]);
	}
	for ( i = 0; i < XUI_TOAST_QUEUE_CAPACITY; i++ ) {
		__xuiToastItemFree(&pToast->arrPending[i]);
	}
	if ( bDeferFree ) {
		pToast->bDestroyPending = 1;
		return;
	}
	xrtFree(pToast);
}

XUI_API int xuiToastShow(xui_toast pToast, int iType, const char* sTitle, const char* sMessage, float fDuration, xui_toast_click_proc onClick, void* pUser)
{
	xui_toast_item_t tItem;
	int iId;
	int iDroppedId;
	int iPromoted;
	int iRet;

	if ( !__xuiToastValid(pToast) || !__xuiToastTypeValid(iType) ||
	     !__xuiToastFloatValid(fDuration) ) {
		return -1;
	}
	memset(&tItem, 0, sizeof(tItem));
	tItem.sTitle = __xuiToastDupText(sTitle);
	tItem.sMessage = __xuiToastDupText(sMessage);
	if ( (tItem.sTitle == NULL) || (tItem.sMessage == NULL) ) {
		__xuiToastItemFree(&tItem);
		return -1;
	}
	iId = __xuiToastAllocateId(pToast);
	if ( iId <= 0 ) {
		__xuiToastItemFree(&tItem);
		return -1;
	}
	tItem.iId = iId;
	tItem.iType = iType;
	tItem.fDuration = fDuration;
	tItem.onClick = onClick;
	tItem.pUser = pUser;
	iDroppedId = 0;
	if ( pToast->iPendingCount >= XUI_TOAST_QUEUE_CAPACITY ) {
		iDroppedId = __xuiToastDropPendingAt(pToast, 0);
	}
	__xuiToastItemMove(&pToast->arrPending[pToast->iPendingCount], &tItem);
	pToast->iPendingCount++;
	__xuiToastCountAdd(&pToast->iShowCount, 1);
	__xuiToastCountAdd(&pToast->iChangeCount, 1);
	iPromoted = __xuiToastPromotePending(pToast);
	iRet = (iPromoted > 0) ? __xuiToastLayout(pToast) : XUI_OK;
	if ( iDroppedId > 0 ) {
		(void)__xuiToastNotifyClose(pToast, iDroppedId, XUI_TOAST_CLOSE_CAPACITY_DROP);
	}
	return (iRet == XUI_OK) ? iId : -1;
}

XUI_API int xuiToastClose(xui_toast pToast, int iToastId)
{
	int iIndex;

	if ( !__xuiToastValid(pToast) || (iToastId <= 0) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	iIndex = __xuiToastFindActiveById(pToast, iToastId);
	if ( iIndex >= 0 ) {
		return __xuiToastRemoveActiveAt(pToast, iIndex, XUI_TOAST_CLOSE_API, NULL);
	}
	iIndex = __xuiToastFindPendingById(pToast, iToastId);
	if ( iIndex >= 0 ) {
		return __xuiToastRemovePendingAt(pToast, iIndex, XUI_TOAST_CLOSE_API, NULL);
	}
	return XUI_ERROR;
}

XUI_API int xuiToastClear(xui_toast pToast)
{
	int arrIds[XUI_TOAST_VISIBLE_CAPACITY + XUI_TOAST_QUEUE_CAPACITY];
	int iCount;
	int i;
	int iRet;

	if ( !__xuiToastValid(pToast) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	iCount = 0;
	for ( i = 0; i < pToast->iActiveCount; i++ ) {
		arrIds[iCount++] = pToast->arrActive[i].iId;
		__xuiToastItemFree(&pToast->arrActive[i]);
	}
	for ( i = 0; i < pToast->iPendingCount; i++ ) {
		arrIds[iCount++] = pToast->arrPending[i].iId;
		__xuiToastItemFree(&pToast->arrPending[i]);
	}
	pToast->iActiveCount = 0;
	pToast->iPendingCount = 0;
	pToast->iHoverItem = -1;
	pToast->iHoverClose = -1;
	__xuiToastCountAdd(&pToast->iCloseCount, iCount);
	__xuiToastCountAdd(&pToast->iChangeCount, 1);
	iRet = __xuiToastLayout(pToast);
	for ( i = 0; i < iCount; i++ ) {
		if ( !__xuiToastNotifyClose(pToast, arrIds[i], XUI_TOAST_CLOSE_CLEAR) ) return iRet;
	}
	return iRet;
}

XUI_API int xuiToastGetActiveCount(xui_toast pToast)
{
	return __xuiToastValid(pToast) ? pToast->iActiveCount : 0;
}

XUI_API int xuiToastGetPendingCount(xui_toast pToast)
{
	return __xuiToastValid(pToast) ? pToast->iPendingCount : 0;
}

XUI_API int xuiToastSetPlacement(xui_toast pToast, int iPlacement)
{
	int iOldPlacement;
	int iRet;

	if ( !__xuiToastValid(pToast) || !__xuiToastPlacementValid(iPlacement) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( pToast->iPlacement == iPlacement ) return XUI_OK;
	iOldPlacement = pToast->iPlacement;
	pToast->iPlacement = iPlacement;
	iRet = __xuiToastLayout(pToast);
	if ( iRet != XUI_OK ) {
		pToast->iPlacement = iOldPlacement;
		(void)__xuiToastLayout(pToast);
		return iRet;
	}
	__xuiToastCountAdd(&pToast->iChangeCount, 1);
	return XUI_OK;
}

XUI_API int xuiToastGetPlacement(xui_toast pToast)
{
	return __xuiToastValid(pToast) ? pToast->iPlacement : XUI_TOAST_PLACEMENT_TOP_RIGHT;
}

XUI_API int xuiToastSetDirection(xui_toast pToast, int iDirection)
{
	int iOldDirection;
	int iRet;

	if ( !__xuiToastValid(pToast) || !__xuiToastDirectionValid(iDirection) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( pToast->iDirection == iDirection ) return XUI_OK;
	iOldDirection = pToast->iDirection;
	pToast->iDirection = iDirection;
	iRet = __xuiToastLayout(pToast);
	if ( iRet != XUI_OK ) {
		pToast->iDirection = iOldDirection;
		(void)__xuiToastLayout(pToast);
		return iRet;
	}
	__xuiToastCountAdd(&pToast->iChangeCount, 1);
	return XUI_OK;
}

XUI_API int xuiToastGetDirection(xui_toast pToast)
{
	return __xuiToastValid(pToast) ? pToast->iDirection : XUI_TOAST_DIRECTION_AUTO;
}

XUI_API int xuiToastSetMetrics(xui_toast pToast, const xui_toast_metrics_t* pMetrics)
{
	xui_toast_metrics_t tNewMetrics;
	xui_toast_metrics_t tOldMetrics;
	int iRet;

	if ( !__xuiToastValid(pToast) || !__xuiToastMetricsValid(pMetrics) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	tNewMetrics = *pMetrics;
	tNewMetrics.iSize = sizeof(tNewMetrics);
	if ( memcmp(&pToast->tMetrics, &tNewMetrics, sizeof(tNewMetrics)) == 0 ) return XUI_OK;
	tOldMetrics = pToast->tMetrics;
	pToast->tMetrics = tNewMetrics;
	(void)__xuiToastPromotePending(pToast);
	iRet = __xuiToastLayout(pToast);
	if ( iRet != XUI_OK ) {
		pToast->tMetrics = tOldMetrics;
		(void)__xuiToastLayout(pToast);
		return iRet;
	}
	__xuiToastCountAdd(&pToast->iChangeCount, 1);
	return XUI_OK;
}

XUI_API int xuiToastGetMetrics(xui_toast pToast, xui_toast_metrics_t* pMetrics)
{
	if ( !__xuiToastValid(pToast) || (pMetrics == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	*pMetrics = pToast->tMetrics;
	pMetrics->iSize = sizeof(*pMetrics);
	return XUI_OK;
}

XUI_API int xuiToastSetColors(xui_toast pToast, const xui_toast_colors_t* pColors)
{
	xui_toast_colors_t tNewColors;
	xui_toast_colors_t tOldColors;
	int iRet;

	if ( !__xuiToastValid(pToast) || !__xuiToastColorsValid(pColors) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	tNewColors = *pColors;
	tNewColors.iSize = sizeof(tNewColors);
	if ( memcmp(&pToast->tColors, &tNewColors, sizeof(tNewColors)) == 0 ) return XUI_OK;
	tOldColors = pToast->tColors;
	pToast->tColors = tNewColors;
	iRet = __xuiToastInvalidateActive(pToast);
	if ( iRet != XUI_OK ) {
		pToast->tColors = tOldColors;
		(void)__xuiToastInvalidateActive(pToast);
		return iRet;
	}
	__xuiToastCountAdd(&pToast->iChangeCount, 1);
	return XUI_OK;
}

XUI_API int xuiToastGetColors(xui_toast pToast, xui_toast_colors_t* pColors)
{
	if ( !__xuiToastValid(pToast) || (pColors == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	*pColors = pToast->tColors;
	pColors->iSize = sizeof(*pColors);
	return XUI_OK;
}

XUI_API int xuiToastSetFont(xui_toast pToast, xui_font pFont)
{
	xui_font pOldFont;
	int iRet;

	if ( !__xuiToastValid(pToast) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( pToast->pFont == pFont ) return XUI_OK;
	pOldFont = pToast->pFont;
	pToast->pFont = pFont;
	iRet = __xuiToastLayout(pToast);
	if ( iRet != XUI_OK ) {
		pToast->pFont = pOldFont;
		(void)__xuiToastLayout(pToast);
		return iRet;
	}
	__xuiToastCountAdd(&pToast->iChangeCount, 1);
	return XUI_OK;
}

XUI_API xui_font xuiToastGetFont(xui_toast pToast)
{
	return __xuiToastValid(pToast) ? pToast->pFont : NULL;
}

XUI_API int xuiToastSetClose(xui_toast pToast, xui_toast_close_proc onClose, void* pUser)
{
	if ( !__xuiToastValid(pToast) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pToast->onClose = onClose;
	pToast->pCloseUser = pUser;
	return XUI_OK;
}

XUI_API xui_widget xuiToastGetItemWidget(xui_toast pToast, int iSlot)
{
	if ( !__xuiToastValid(pToast) || (iSlot < 0) || (iSlot >= XUI_TOAST_VISIBLE_CAPACITY) ) {
		return NULL;
	}
	return pToast->arrWidgets[iSlot];
}

XUI_API xui_rect_t xuiToastGetItemRect(xui_toast pToast, int iSlot)
{
	if ( !__xuiToastValid(pToast) || (iSlot < 0) || (iSlot >= pToast->iActiveCount) ) {
		return (xui_rect_t){0.0f, 0.0f, 0.0f, 0.0f};
	}
	return pToast->arrActive[iSlot].tRect;
}

XUI_API xui_rect_t xuiToastGetIconRect(xui_toast pToast, int iSlot)
{
	xui_rect_t tRect;

	if ( !__xuiToastValid(pToast) || (iSlot < 0) || (iSlot >= pToast->iActiveCount) ) {
		return (xui_rect_t){0.0f, 0.0f, 0.0f, 0.0f};
	}
	tRect = pToast->arrActive[iSlot].tIconRect;
	tRect.fX += pToast->arrActive[iSlot].tRect.fX;
	tRect.fY += pToast->arrActive[iSlot].tRect.fY;
	return tRect;
}

XUI_API xui_rect_t xuiToastGetCloseRect(xui_toast pToast, int iSlot)
{
	xui_rect_t tRect;

	if ( !__xuiToastValid(pToast) || (iSlot < 0) || (iSlot >= pToast->iActiveCount) ) {
		return (xui_rect_t){0.0f, 0.0f, 0.0f, 0.0f};
	}
	tRect = pToast->arrActive[iSlot].tCloseRect;
	tRect.fX += pToast->arrActive[iSlot].tRect.fX;
	tRect.fY += pToast->arrActive[iSlot].tRect.fY;
	return tRect;
}

XUI_API int xuiToastGetShowCount(xui_toast pToast)
{
	return __xuiToastValid(pToast) ? pToast->iShowCount : 0;
}

XUI_API int xuiToastGetCloseCount(xui_toast pToast)
{
	return __xuiToastValid(pToast) ? pToast->iCloseCount : 0;
}

XUI_API int xuiToastGetExpireCount(xui_toast pToast)
{
	return __xuiToastValid(pToast) ? pToast->iExpireCount : 0;
}

XUI_API int xuiToastGetDropCount(xui_toast pToast)
{
	return __xuiToastValid(pToast) ? pToast->iDropCount : 0;
}

XUI_API int xuiToastGetChangeCount(xui_toast pToast)
{
	return __xuiToastValid(pToast) ? pToast->iChangeCount : 0;
}

#endif
