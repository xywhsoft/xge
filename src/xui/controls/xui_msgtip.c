#include "xui_config.h"
#if XUI_ENABLE_MSGTIP
#include "xui_internal.h"
#include "xui_text_internal.h"

#include <limits.h>
#include <string.h>

#define XUI_MSGTIP_MAGIC 0x584D5354u

struct xui_msgtip_t {
	uint32_t iMagic;
	xui_context pContext;
	xui_widget pWidget;
	char* sText;
	xui_font pFont;
	xui_surface pIconSurface;
	xui_rect_t tIconSrc;
	xui_rect_t tTipRect;
	xui_rect_t tIconRect;
	xui_rect_t tTextRect;
	xui_rect_t tLayoutRootRect;
	xui_msgtip_metrics_t tMetrics;
	xui_msgtip_colors_t tColors;
	xui_msgtip_close_proc onClose;
	void* pCloseUser;
	float fDuration;
	float fElapsed;
	int iType;
	int iWrapLineCount;
	int iShowCount;
	int iCloseCount;
	int iExpireCount;
	int iChangeCount;
	int bOpen;
	int bCustomIcon;
	int bHasColors;
};

static int __xuiMsgTipValid(xui_msgtip pTip)
{
	return (pTip != NULL) && (pTip->iMagic == XUI_MSGTIP_MAGIC);
}

static uint32_t __xuiMsgTipStyleColor(xui_msgtip pTip, const char* sName, uint32_t iBase)
{
	xui_style_property_t tProperty;
	memset(&tProperty, 0, sizeof(tProperty));
	tProperty.iSize = sizeof(tProperty);
	if ( xuiWidgetGetResolvedStyleProperty(pTip->pWidget, sName, &tProperty) == XUI_OK &&
		tProperty.tValue.iType == XUI_STYLE_VALUE_COLOR ) return tProperty.tValue.iColor;
	return iBase;
}

static void __xuiMsgTipRegisterColors(xui_context pContext, xui_widget_type pType)
{
	static const char* arrNames[] = {
		"msgtip.background.color",
		"msgtip.border.color",
		"msgtip.text.color",
		"msgtip.icon.color",
		"msgtip.shadow.color",
		"msgtip.icon.text.color"
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

static float __xuiMsgTipMin(float fA, float fB)
{
	return (fA < fB) ? fA : fB;
}

static float __xuiMsgTipMax(float fA, float fB)
{
	return (fA > fB) ? fA : fB;
}

static float __xuiMsgTipPixelCeil(float fValue)
{
	if ( fValue <= 0.0f ) {
		return 0.0f;
	}
	return (float)xuiInternalPixelCeil(fValue);
}

static float __xuiMsgTipClamp(float fValue, float fMin, float fMax)
{
	if ( fMax < fMin ) {
		return fMin;
	}
	if ( fValue < fMin ) return fMin;
	if ( fValue > fMax ) return fMax;
	return fValue;
}

static int __xuiMsgTipFloatValid(float fValue)
{
	return (fValue == fValue) && (fValue >= 0.0f) && (fValue <= XUI_LAYOUT_UNBOUNDED);
}

static int __xuiMsgTipTypeValid(int iType)
{
	return (iType >= XUI_MSGTIP_ICON_NONE) && (iType <= XUI_MSGTIP_ICON_ERROR);
}

static int __xuiMsgTipRectValid(xui_rect_t tRect)
{
	return tRect.fX >= -XUI_LAYOUT_UNBOUNDED && tRect.fX <= XUI_LAYOUT_UNBOUNDED &&
	       tRect.fY >= -XUI_LAYOUT_UNBOUNDED && tRect.fY <= XUI_LAYOUT_UNBOUNDED &&
	       __xuiMsgTipFloatValid(tRect.fW) && __xuiMsgTipFloatValid(tRect.fH);
}

static int __xuiMsgTipRectSame(xui_rect_t tA, xui_rect_t tB)
{
	return tA.fX == tB.fX && tA.fY == tB.fY &&
	       tA.fW == tB.fW && tA.fH == tB.fH;
}

static int __xuiMsgTipNormalizeType(int iType)
{
	if ( iType < XUI_MSGTIP_ICON_NONE || iType > XUI_MSGTIP_ICON_ERROR ) {
		return XUI_MSGTIP_ICON_INFO;
	}
	return iType;
}

static char* __xuiMsgTipDupText(const char* sText)
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

static int __xuiMsgTipReplaceText(char** ppText, const char* sText)
{
	char* sCopy;

	if ( ppText == NULL ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	sCopy = __xuiMsgTipDupText(sText);
	if ( sCopy == NULL ) {
		return XUI_ERROR_OUT_OF_MEMORY;
	}
	if ( *ppText != NULL ) {
		xrtFree(*ppText);
	}
	*ppText = sCopy;
	return XUI_OK;
}

static int __xuiMsgTipTextSame(const char* sA, const char* sB)
{
	if ( sA == NULL ) sA = "";
	if ( sB == NULL ) sB = "";
	return strcmp(sA, sB) == 0;
}

static void __xuiMsgTipDefaultMetrics(xui_msgtip_metrics_t* pMetrics)
{
	memset(pMetrics, 0, sizeof(*pMetrics));
	pMetrics->iSize = sizeof(*pMetrics);
	pMetrics->fMinWidth = 92.0f;
	pMetrics->fMaxWidth = 360.0f;
	pMetrics->fMinHeight = 40.0f;
	pMetrics->fPaddingX = 16.0f;
	pMetrics->fPaddingY = 10.0f;
	pMetrics->fIconSize = 20.0f;
	pMetrics->fIconGap = 8.0f;
	pMetrics->fOffsetY = -72.0f;
}

static uint32_t __xuiMsgTipIconColor(int iType)
{
	switch ( iType ) {
	case XUI_MSGTIP_ICON_WAR:
		return XUI_COLOR_RGBA(231, 156, 45, 255);
	case XUI_MSGTIP_ICON_ERROR:
		return XUI_COLOR_RGBA(216, 76, 90, 255);
	case XUI_MSGTIP_ICON_QUEST:
		return XUI_COLOR_RGBA(59, 126, 204, 255);
	case XUI_MSGTIP_ICON_INFO:
	default:
		return XUI_COLOR_RGBA(60, 177, 128, 255);
	}
}

static void __xuiMsgTipDefaultColors(xui_msgtip_colors_t* pColors)
{
	memset(pColors, 0, sizeof(*pColors));
	pColors->iSize = sizeof(*pColors);
	pColors->iBackgroundColor = XUI_COLOR_RGBA(32, 42, 54, 222);
	pColors->iBorderColor = XUI_COLOR_RGBA(114, 151, 190, 120);
	pColors->iTextColor = XUI_COLOR_RGBA(255, 255, 255, 255);
	pColors->iIconColor = __xuiMsgTipIconColor(XUI_MSGTIP_ICON_INFO);
	pColors->iShadowColor = 0;
}

static int __xuiMsgTipMetricsValid(const xui_msgtip_metrics_t* pMetrics)
{
	if ( pMetrics == NULL ) return 0;
	if ( (pMetrics->iSize != 0) && (pMetrics->iSize < sizeof(*pMetrics)) ) return 0;
	return __xuiMsgTipFloatValid(pMetrics->fMinWidth) &&
	       __xuiMsgTipFloatValid(pMetrics->fMaxWidth) &&
	       __xuiMsgTipFloatValid(pMetrics->fMinHeight) &&
	       __xuiMsgTipFloatValid(pMetrics->fPaddingX) &&
	       __xuiMsgTipFloatValid(pMetrics->fPaddingY) &&
	       __xuiMsgTipFloatValid(pMetrics->fIconSize) &&
	       __xuiMsgTipFloatValid(pMetrics->fIconGap) &&
	       (pMetrics->fMinWidth >= 32.0f) &&
	       (pMetrics->fMaxWidth >= pMetrics->fMinWidth) &&
	       (pMetrics->fMinHeight >= 24.0f) &&
	       (pMetrics->fOffsetY == pMetrics->fOffsetY) &&
	       (pMetrics->fOffsetY >= -XUI_CONTEXT_MAX_VIEWPORT) &&
	       (pMetrics->fOffsetY <= XUI_CONTEXT_MAX_VIEWPORT);
}

static int __xuiMsgTipColorsValid(const xui_msgtip_colors_t* pColors)
{
	if ( pColors == NULL ) return 0;
	return (pColors->iSize == 0) || (pColors->iSize >= sizeof(*pColors));
}

static int __xuiMsgTipDescValid(const xui_msgtip_desc_t* pDesc)
{
	if ( pDesc == NULL ) return 1;
	if ( (pDesc->iSize != 0) && (pDesc->iSize < sizeof(*pDesc)) ) return 0;
	if ( !__xuiMsgTipTypeValid(pDesc->iType) ) return 0;
	if ( !__xuiMsgTipFloatValid(pDesc->fDuration) ) return 0;
	if ( !__xuiMsgTipRectValid(pDesc->tIconSrc) ) return 0;
	if ( pDesc->bHasCustomIcon && pDesc->pIconSurface != NULL &&
	     (pDesc->tIconSrc.fW <= 0 || pDesc->tIconSrc.fH <= 0) ) return 0;
	if ( pDesc->bHasMetrics && !__xuiMsgTipMetricsValid(&pDesc->tMetrics) ) return 0;
	if ( pDesc->bHasColors && !__xuiMsgTipColorsValid(&pDesc->tColors) ) return 0;
	return 1;
}

static xui_font __xuiMsgTipFont(xui_msgtip pTip)
{
	if ( pTip == NULL ) return NULL;
	return (pTip->pFont != NULL) ? pTip->pFont : xuiGetDefaultFont(pTip->pContext);
}

static const char* __xuiMsgTipIconName(int iType)
{
	switch ( iType ) {
	case XUI_MSGTIP_ICON_INFO:
		return "msgbox_info";
	case XUI_MSGTIP_ICON_QUEST:
		return "msgbox_quest";
	case XUI_MSGTIP_ICON_WAR:
		return "msgbox_war";
	case XUI_MSGTIP_ICON_ERROR:
		return "msgbox_error";
	default:
		return NULL;
	}
}

static const char* __xuiMsgTipIconFallbackText(int iType)
{
	switch ( iType ) {
	case XUI_MSGTIP_ICON_QUEST:
		return "?";
	case XUI_MSGTIP_ICON_WAR:
		return "!";
	case XUI_MSGTIP_ICON_ERROR:
		return "x";
	case XUI_MSGTIP_ICON_INFO:
	default:
		return "i";
	}
}

static int __xuiMsgTipHasIcon(xui_msgtip pTip)
{
	if ( !__xuiMsgTipValid(pTip) ) return 0;
	if ( pTip->bCustomIcon && (pTip->pIconSurface != NULL) ) return 1;
	return pTip->iType != XUI_MSGTIP_ICON_NONE;
}

static xui_rect_t __xuiMsgTipRootRect(xui_msgtip pTip)
{
	xui_size_t tViewport;
	xui_widget pRoot;
	xui_rect_t tRect;

	tRect = (xui_rect_t){0.0f, 0.0f, 640.0f, 420.0f};
	if ( !__xuiMsgTipValid(pTip) ) {
		return tRect;
	}
	pRoot = xuiGetRootWidget(pTip->pContext);
	if ( pRoot != NULL ) {
		tRect = xuiWidgetGetRect(pRoot);
	}
	if ( (tRect.fW <= 1.0f) || (tRect.fH <= 1.0f) ) {
		tViewport = xuiGetViewportSize(pTip->pContext);
		if ( (tViewport.iW > 1) && (tViewport.iH > 1) ) {
			tRect = (xui_rect_t){0, 0, tViewport.iW, tViewport.iH};
		}
	}
	return tRect;
}

static int __xuiMsgTipMeasureText(xui_msgtip pTip, float fWidth,
	float fHeight, xui_vec2_t* pSize, int* pLineCount)
{
	xui_text_layout_desc_t tDesc;
	xui_text_layout pLayout;
	xui_font pFont;
	const char* sText;
	int iRet;

	if ( !__xuiMsgTipValid(pTip) || (pSize == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pSize->fX = 0.0f;
	pSize->fY = 0.0f;
	if ( pLineCount != NULL ) {
		*pLineCount = 0;
	}
	sText = (pTip->sText != NULL) ? pTip->sText : "";
	if ( fWidth <= 0.0f || fHeight <= 0.0f || sText[0] == '\0' ) {
		return XUI_OK;
	}
	pFont = __xuiMsgTipFont(pTip);
	if ( pFont == NULL ) {
		pSize->fX = __xuiMsgTipMin((float)strlen(sText) * 7.0f, fWidth);
		pSize->fY = __xuiMsgTipMin(18.0f, fHeight);
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
	tDesc.iWrapMode = XUI_TEXT_WRAP_CHAR;
	tDesc.iFlags = XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_TOP;
	tDesc.fLineGap = 2.0f;
	pLayout = NULL;
	iRet = xuiTextLayoutCreate(pTip->pContext, &pLayout, &tDesc);
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

static int __xuiMsgTipLayout(xui_msgtip pTip)
{
	xui_rect_t tRoot;
	xui_rect_t tTip;
	xui_vec2_t tTextSize;
	float fAvailableWidth;
	float fAvailableHeight;
	float fMaxWidth;
	float fMinWidth;
	float fTextLimit;
	float fMeasuredTextLimit;
	float fTextW;
	float fContentW;
	float fContentH;
	float fMaxContentH;
	float fIconBlock;
	float fIconSize;
	float fIconGap;
	float fPaddingX;
	float fPaddingY;
	float fWidth;
	float fHeight;
	float fX;
	float fY;
	int bIcon;
	int iTipWidth;
	int iTipHeight;
	int iRet;

	if ( !__xuiMsgTipValid(pTip) || (pTip->pWidget == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	tRoot = __xuiMsgTipRootRect(pTip);
	if ( tRoot.fW <= 0 || tRoot.fH <= 0 ) {
		pTip->tLayoutRootRect = tRoot;
		return XUI_OK;
	}
	fAvailableWidth = (tRoot.fW > 24) ? (float)(tRoot.fW - 24) : (float)tRoot.fW;
	fAvailableHeight = (float)tRoot.fH;
	if ( fAvailableWidth < 1.0f ) fAvailableWidth = 1.0f;
	if ( fAvailableHeight < 1.0f ) fAvailableHeight = 1.0f;
	fMaxWidth = __xuiMsgTipMin(pTip->tMetrics.fMaxWidth, fAvailableWidth);
	if ( fMaxWidth < 1.0f ) fMaxWidth = 1.0f;
	fMinWidth = __xuiMsgTipMin(pTip->tMetrics.fMinWidth, fMaxWidth);
	fPaddingX = __xuiMsgTipMin(pTip->tMetrics.fPaddingX,
		fMaxWidth * 0.5f);
	fPaddingY = __xuiMsgTipMin(pTip->tMetrics.fPaddingY,
		fAvailableHeight * 0.5f);
	fMaxContentH = __xuiMsgTipMax(0.0f,
		fAvailableHeight - fPaddingY * 2.0f);
	bIcon = __xuiMsgTipHasIcon(pTip);
	fIconSize = bIcon ? __xuiMsgTipMin(pTip->tMetrics.fIconSize,
		__xuiMsgTipMin(fMaxContentH,
			__xuiMsgTipMax(0.0f, fMaxWidth - fPaddingX * 2.0f))) : 0.0f;
	fIconGap = (fIconSize > 0.0f) ?
		__xuiMsgTipMin(pTip->tMetrics.fIconGap,
			__xuiMsgTipMax(0.0f, fMaxWidth - fPaddingX * 2.0f - fIconSize)) : 0.0f;
	fIconBlock = (fIconSize > 0.0f) ? fIconSize + fIconGap : 0.0f;
	fTextLimit = __xuiMsgTipMax(0.0f,
		fMaxWidth - fPaddingX * 2.0f - fIconBlock);
	fMeasuredTextLimit = fTextLimit;
	iRet = __xuiMsgTipMeasureText(pTip, fTextLimit,
		XUI_LAYOUT_UNBOUNDED, &tTextSize, &pTip->iWrapLineCount);
	if ( iRet != XUI_OK ) return iRet;
	fTextW = __xuiMsgTipMin(__xuiMsgTipPixelCeil(tTextSize.fX), fTextLimit);
	fContentW = __xuiMsgTipPixelCeil(fIconBlock + fTextW);
	fWidth = __xuiMsgTipClamp(fContentW + fPaddingX * 2.0f,
		fMinWidth, fMaxWidth);
	iTipWidth = xuiInternalSnapSize(fWidth);
	if ( iTipWidth > (int)fAvailableWidth ) iTipWidth = (int)fAvailableWidth;
	if ( iTipWidth < 1 ) iTipWidth = 1;
	fWidth = (float)iTipWidth;
	fTextLimit = __xuiMsgTipMax(0.0f,
		fWidth - fPaddingX * 2.0f - fIconBlock);
	if ( fTextLimit != fMeasuredTextLimit ) {
		iRet = __xuiMsgTipMeasureText(pTip, fTextLimit,
			XUI_LAYOUT_UNBOUNDED, &tTextSize, &pTip->iWrapLineCount);
		if ( iRet != XUI_OK ) return iRet;
	}
	fContentH = __xuiMsgTipMax(tTextSize.fY, fIconSize);
	fHeight = __xuiMsgTipClamp(fContentH + fPaddingY * 2.0f,
		__xuiMsgTipMin(pTip->tMetrics.fMinHeight, fAvailableHeight),
		fAvailableHeight);
	iTipHeight = xuiInternalSnapSize(fHeight);
	if ( iTipHeight > (int)fAvailableHeight ) iTipHeight = (int)fAvailableHeight;
	if ( iTipHeight < 1 ) iTipHeight = 1;
	fHeight = (float)iTipHeight;
	fMaxContentH = __xuiMsgTipMax(0.0f, fHeight - fPaddingY * 2.0f);
	fIconSize = __xuiMsgTipMin(fIconSize, fMaxContentH);
	if ( tTextSize.fY > fMaxContentH ) {
		iRet = __xuiMsgTipMeasureText(pTip, fTextLimit, fMaxContentH,
			&tTextSize, &pTip->iWrapLineCount);
		if ( iRet != XUI_OK ) return iRet;
	}
	fX = (float)tRoot.fX + ((float)tRoot.fW - fWidth) * 0.5f;
	fY = (float)tRoot.fY + ((float)tRoot.fH - fHeight) * 0.5f +
		pTip->tMetrics.fOffsetY;
	fY = __xuiMsgTipClamp(fY, (float)tRoot.fY,
		(float)(tRoot.fY + tRoot.fH) - fHeight);
	tTip = xuiInternalRectFromFloatNearest(fX, fY, fWidth, fHeight);
	pTip->tTipRect = tTip;
	if ( fIconSize > 0.0f ) {
		pTip->tIconRect = xuiInternalRectFromFloatNearest(
			fPaddingX,
			((float)tTip.fH - fIconSize) * 0.5f,
			fIconSize, fIconSize);
	} else {
		pTip->tIconRect = (xui_rect_t){0, 0, 0, 0};
	}
	pTip->tTextRect = xuiInternalRectFromFloatNearest(
		fPaddingX + fIconBlock,
		((float)tTip.fH - tTextSize.fY) * 0.5f,
		__xuiMsgTipMax(0.0f, (float)tTip.fW - fPaddingX * 2.0f - fIconBlock),
		tTextSize.fY);
	iRet = xuiWidgetSetRect(pTip->pWidget, tTip);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiWidgetInvalidate(pTip->pWidget,
		XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	if ( iRet == XUI_OK ) pTip->tLayoutRootRect = tRoot;
	return iRet;
}

static int __xuiMsgTipSyncLayout(xui_msgtip pTip)
{
	xui_rect_t tRoot;

	if ( !__xuiMsgTipValid(pTip) || !pTip->bOpen ) return XUI_OK;
	tRoot = __xuiMsgTipRootRect(pTip);
	if ( __xuiMsgTipRectSame(tRoot, pTip->tLayoutRootRect) ) return XUI_OK;
	return __xuiMsgTipLayout(pTip);
}

static int __xuiMsgTipDrawTextLayout(xui_msgtip pTip, xui_draw_context pDraw)
{
	xui_text_layout_desc_t tDesc;
	xui_text_layout pLayout;
	xui_proxy pProxy;
	xui_font pFont;
	int iRet;

	if ( !__xuiMsgTipValid(pTip) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pProxy = xuiInternalContextGetProxy(pTip->pContext);
	pFont = __xuiMsgTipFont(pTip);
	if ( (pProxy == NULL) || (pProxy->drawText == NULL) || (pDraw == NULL) || (pFont == NULL) ) {
		return XUI_OK;
	}
	if ( pTip->tTextRect.fW <= 0 || pTip->tTextRect.fH <= 0 ||
	     pTip->sText == NULL || pTip->sText[0] == '\0' ) {
		return XUI_OK;
	}
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.sText = (pTip->sText != NULL) ? pTip->sText : "";
	tDesc.iTextSize = -1;
	tDesc.pFont = pFont;
	tDesc.fMaxWidth = pTip->tTextRect.fW;
	tDesc.fMaxHeight = pTip->tTextRect.fH;
	tDesc.iWrapMode = XUI_TEXT_WRAP_CHAR;
	tDesc.iFlags = XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_TOP | XUI_TEXT_CLIP;
	tDesc.fLineGap = 2.0f;
	pLayout = NULL;
	iRet = xuiTextLayoutCreate(pTip->pContext, &pLayout, &tDesc);
	if ( iRet != XUI_OK ) {
		return iRet;
	}
	iRet = xuiInternalTextLayoutDrawContext(pLayout, pDraw,
		pTip->tTextRect,
		__xuiMsgTipStyleColor(pTip, "msgtip.text.color",
			pTip->tColors.iTextColor), 0);
	xuiTextLayoutDestroy(pLayout);
	return iRet;
}

static int __xuiMsgTipDrawFallbackIcon(xui_msgtip pTip, xui_draw_context pDraw, xui_proxy pProxy)
{
	xui_rect_t tIcon;
	xui_rect_t tText;
	float fRadius;
	int iRet;

	if ( (pProxy == NULL) || (pDraw == NULL) || (pTip == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	tIcon = pTip->tIconRect;
	fRadius = __xuiMsgTipMin(tIcon.fW, tIcon.fH) * 0.5f;
	if ( pProxy->drawCircleFill != NULL ) {
		iRet = pProxy->drawCircleFill(pProxy, pDraw, tIcon.fX + tIcon.fW * 0.5f, tIcon.fY + tIcon.fH * 0.5f, fRadius, __xuiMsgTipStyleColor(pTip, "msgtip.icon.color", pTip->tColors.iIconColor));
		if ( iRet != XUI_OK ) return iRet;
	} else if ( pProxy->drawRectFill != NULL ) {
		iRet = pProxy->drawRectFill(pProxy, pDraw, tIcon,
			__xuiMsgTipStyleColor(pTip, "msgtip.icon.color",
				pTip->tColors.iIconColor));
		if ( iRet != XUI_OK ) return iRet;
	}
	if ( (pProxy->drawText != NULL) && (__xuiMsgTipFont(pTip) != NULL) ) {
		tText = tIcon;
		tText.fY -= 1.0f;
		return pProxy->drawText(pProxy, pDraw, &(xui_text_item_t){.iSize=sizeof(xui_text_item_t), .pFont=__xuiMsgTipFont(pTip), .sText=__xuiMsgTipIconFallbackText(pTip->iType), .iTextSize=1, .iFlags=XUI_TEXT_SHAPE_DEFAULT | ((XUI_TEXT_ALIGN_CENTER | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP) & XUI_TEXT_RTL ? XUI_TEXT_SHAPE_RTL : 0)}, tText, __xuiMsgTipStyleColor(pTip, "msgtip.icon.text.color", XUI_COLOR_WHITE), XUI_TEXT_ALIGN_CENTER | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP);
	}
	return XUI_OK;
}

static int __xuiMsgTipDrawBuiltinIcon(xui_msgtip pTip, xui_draw_context pDraw, xui_proxy pProxy)
{
	xui_surface pAtlas;
	xui_rect_t tSrc;
	const char* sName;
	int iRet;

	if ( (pTip == NULL) || (pDraw == NULL) || (pProxy == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	sName = __xuiMsgTipIconName(pTip->iType);
	if ( sName == NULL ) {
		return XUI_OK;
	}
	pAtlas = NULL;
	iRet = xuiBuiltinAssetGetAtlas(pTip->pContext, &pAtlas);
	if ( iRet == XUI_OK ) {
		iRet = xuiBuiltinAssetGetRect(sName, &tSrc);
	}
	if ( (iRet == XUI_OK) && (pAtlas != NULL) && (pProxy->drawSurface != NULL) ) {
		return pProxy->drawSurface(pProxy, pDraw, pAtlas, tSrc, pTip->tIconRect, __xuiMsgTipStyleColor(pTip, "msgtip.icon.color", pTip->tColors.iIconColor), 0);
	}
	return __xuiMsgTipDrawFallbackIcon(pTip, pDraw, pProxy);
}

static int __xuiMsgTipRender(xui_widget pWidget, xui_draw_context pDraw, uint32_t iStateId, void* pUser)
{
	xui_msgtip* ppTip;
	xui_msgtip pTip;
	xui_proxy pProxy;
	xui_rect_t tRect;
	uint32_t iBorder;
	int iRet;

	(void)iStateId;
	(void)pWidget;
	ppTip = (xui_msgtip*)pUser;
	pTip = (ppTip != NULL) ? *ppTip : NULL;
	if ( !__xuiMsgTipValid(pTip) || !pTip->bOpen ) {
		return XUI_OK;
	}
	pProxy = xuiInternalContextGetProxy(pTip->pContext);
	if ( (pProxy == NULL) || (pDraw == NULL) ) {
		return XUI_OK;
	}
	tRect = xuiInternalSnapRect((xui_rect_t){0.0f, 0.0f, pTip->tTipRect.fW, pTip->tTipRect.fH});
	if ( pProxy->drawRectFill != NULL ) {
		uint32_t iShadow = __xuiMsgTipStyleColor(pTip, "msgtip.shadow.color", pTip->tColors.iShadowColor);
		if ( (iShadow & 0xffu) != 0u ) {
			iRet = pProxy->drawRectFill(pProxy, pDraw, tRect, iShadow);
			if ( iRet != XUI_OK ) return iRet;
			tRect.fW = __xuiMsgTipMax(1.0f, tRect.fW - 2.0f);
			tRect.fH = __xuiMsgTipMax(1.0f, tRect.fH - 2.0f);
		}
	}
	if ( pProxy->drawRectFill != NULL ) {
		iRet = pProxy->drawRectFill(pProxy, pDraw, tRect, __xuiMsgTipStyleColor(pTip, "msgtip.background.color", pTip->tColors.iBackgroundColor));
		if ( iRet != XUI_OK ) return iRet;
	}
	iBorder = __xuiMsgTipStyleColor(pTip, "msgtip.border.color",
		pTip->tColors.iBorderColor);
	if ( ((iBorder & 0xffu) != 0u) && (pProxy->drawRectStroke != NULL) ) {
		iRet = pProxy->drawRectStroke(pProxy, pDraw, tRect, 1.0f, iBorder);
		if ( iRet != XUI_OK ) return iRet;
	}
	if ( __xuiMsgTipHasIcon(pTip) &&
	     pTip->tIconRect.fW > 0 && pTip->tIconRect.fH > 0 ) {
		if ( pTip->bCustomIcon && (pTip->pIconSurface != NULL) ) {
			if ( pProxy->drawSurface != NULL ) {
				iRet = pProxy->drawSurface(pProxy, pDraw, pTip->pIconSurface,
					pTip->tIconSrc, pTip->tIconRect, XUI_COLOR_WHITE, 0);
			} else {
				iRet = __xuiMsgTipDrawFallbackIcon(pTip, pDraw, pProxy);
			}
		} else {
			iRet = __xuiMsgTipDrawBuiltinIcon(pTip, pDraw, pProxy);
		}
		if ( iRet != XUI_OK ) return iRet;
	}
	return __xuiMsgTipDrawTextLayout(pTip, pDraw);
}

static int __xuiMsgTipCloseInternal(xui_msgtip pTip, int bExpired)
{
	xui_msgtip_close_proc onClose;
	void* pCloseUser;
	int iRet;

	if ( !__xuiMsgTipValid(pTip) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( !pTip->bOpen ) {
		return XUI_OK;
	}
	iRet = xuiWidgetSetHitTestVisible(pTip->pWidget, 0);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEnabled(pTip->pWidget, 0);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetVisible(pTip->pWidget, 0);
	if ( iRet == XUI_OK ) iRet = xuiWidgetInvalidate(pTip->pWidget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	if ( iRet != XUI_OK ) {
		(void)xuiWidgetSetVisible(pTip->pWidget, 1);
		(void)xuiWidgetSetEnabled(pTip->pWidget, 1);
		(void)xuiWidgetSetHitTestVisible(pTip->pWidget, 1);
		return iRet;
	}
	pTip->bOpen = 0;
	pTip->fElapsed = 0.0f;
	if ( bExpired ) {
		if ( pTip->iExpireCount < INT_MAX ) pTip->iExpireCount++;
	} else {
		if ( pTip->iCloseCount < INT_MAX ) pTip->iCloseCount++;
	}
	if ( pTip->iChangeCount < INT_MAX ) pTip->iChangeCount++;
	onClose = pTip->onClose;
	pCloseUser = pTip->pCloseUser;
	if ( onClose != NULL ) onClose(pTip, bExpired ? 1 : 0, pCloseUser);
	return XUI_OK;
}

static int __xuiMsgTipUpdate(xui_widget pWidget, float fDelta, void* pUser)
{
	xui_msgtip* ppTip;
	xui_msgtip pTip;
	int iRet;

	(void)pWidget;
	ppTip = (xui_msgtip*)pUser;
	pTip = (ppTip != NULL) ? *ppTip : NULL;
	if ( !__xuiMsgTipValid(pTip) || !pTip->bOpen ) {
		return XUI_OK;
	}
	iRet = __xuiMsgTipSyncLayout(pTip);
	if ( iRet != XUI_OK ) return iRet;
	if ( pTip->fDuration <= 0.0f || (fDelta != fDelta) || fDelta <= 0.0f ) {
		return XUI_OK;
	}
	if ( pTip->fElapsed >= pTip->fDuration ||
	     fDelta >= pTip->fDuration - pTip->fElapsed ) {
		return __xuiMsgTipCloseInternal(pTip, 1);
	}
	pTip->fElapsed += fDelta;
	return XUI_OK;
}

static int __xuiMsgTipEvent(xui_widget pWidget, const xui_event_t* pEvent, void* pUser)
{
	xui_msgtip* ppTip;
	xui_msgtip pTip;

	(void)pWidget;
	ppTip = (xui_msgtip*)pUser;
	pTip = (ppTip != NULL) ? *ppTip : NULL;
	if ( !__xuiMsgTipValid(pTip) || (pEvent == NULL) || !pTip->bOpen ) {
		return XUI_OK;
	}
	if ( (pEvent->iType == XUI_EVENT_POINTER_DOWN) && (pEvent->iButton == XUI_POINTER_BUTTON_LEFT) ) {
		int iRet = __xuiMsgTipCloseInternal(pTip, 0);
		if ( iRet != XUI_OK ) return iRet;
		return XUI_EVENT_DISPATCH_STOP;
	}
	return XUI_OK;
}

static int __xuiMsgTipTypeInit(xui_widget pWidget, void* pTypeData, const void* pCreateData, void* pUser)
{
	xui_cache_policy_t tPolicy;
	xui_msgtip* ppTip;
	int iRet;

	(void)pUser;
	if ( (pWidget == NULL) || (pTypeData == NULL) || (pCreateData == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	ppTip = (xui_msgtip*)pTypeData;
	*ppTip = (xui_msgtip)pCreateData;
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
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetLayer(pWidget, XUI_LAYER_TOOLTIP, 100);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetCachePolicy(pWidget, &tPolicy);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_POINTER_DOWN, __xuiMsgTipEvent, pTypeData);
	return iRet;
}

static void __xuiMsgTipTypeDestroy(xui_widget pWidget, void* pTypeData, void* pUser)
{
	xui_msgtip* ppTip;

	(void)pWidget;
	(void)pUser;
	ppTip = (xui_msgtip*)pTypeData;
	if ( ppTip != NULL ) {
		*ppTip = NULL;
	}
}

static xui_widget_type __xuiMsgTipEnsureType(xui_context pContext)
{
	xui_widget_type pType;
	xui_widget_type_desc_t tDesc;
	int iRet;

	if ( !xuiInternalContextIsValid(pContext) ) {
		return NULL;
	}
	pType = xuiWidgetFindType(pContext, "msgtip");
	if ( pType != NULL ) {
		return pType;
	}
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.sName = "msgtip";
	tDesc.pParent = xuiWidgetGetBaseType();
	tDesc.iTypeDataSize = sizeof(xui_msgtip);
	tDesc.onInit = __xuiMsgTipTypeInit;
	tDesc.onDestroy = __xuiMsgTipTypeDestroy;
	tDesc.onCacheRender = __xuiMsgTipRender;
	tDesc.onUpdate = __xuiMsgTipUpdate;
	iRet = xuiWidgetRegisterType(pContext, &pType, &tDesc);
	if ( iRet == XUI_OK ) __xuiMsgTipRegisterColors(pContext, pType);
	if ( iRet == XUI_ERROR_ALREADY_INITIALIZED ) {
		return xuiWidgetFindType(pContext, "msgtip");
	}
	return (iRet == XUI_OK) ? pType : NULL;
}

XUI_API int xuiMsgTipCreate(xui_context pContext, xui_msgtip* ppTip, const xui_msgtip_desc_t* pDesc)
{
	xui_msgtip pTip;
	xui_widget_type pType;
	int iRet;

	if ( ppTip == NULL ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	*ppTip = NULL;
	if ( !xuiInternalContextIsValid(pContext) || !__xuiMsgTipDescValid(pDesc) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pType = __xuiMsgTipEnsureType(pContext);
	if ( pType == NULL ) {
		return XUI_ERROR_NOT_INITIALIZED;
	}
	pTip = (xui_msgtip)xrtCalloc(1, sizeof(*pTip));
	if ( pTip == NULL ) {
		return XUI_ERROR_OUT_OF_MEMORY;
	}
	pTip->iMagic = XUI_MSGTIP_MAGIC;
	pTip->pContext = pContext;
	pTip->pFont = (pDesc != NULL && pDesc->pFont != NULL) ? pDesc->pFont : xuiGetDefaultFont(pContext);
	pTip->iType = (pDesc != NULL) ? __xuiMsgTipNormalizeType(pDesc->iType) : XUI_MSGTIP_ICON_INFO;
	pTip->fDuration = (pDesc != NULL) ? pDesc->fDuration : 1.8f;
	__xuiMsgTipDefaultMetrics(&pTip->tMetrics);
	__xuiMsgTipDefaultColors(&pTip->tColors);
	if ( pDesc != NULL && pDesc->bHasMetrics ) pTip->tMetrics = pDesc->tMetrics;
	if ( pDesc != NULL && pDesc->bHasColors ) pTip->tColors = pDesc->tColors;
	pTip->bHasColors = pDesc != NULL && pDesc->bHasColors;
	pTip->tMetrics.iSize = sizeof(pTip->tMetrics);
	pTip->tColors.iSize = sizeof(pTip->tColors);
	if ( !(pDesc != NULL && pDesc->bHasColors) ) {
		if ( !pTip->bHasColors ) pTip->tColors.iIconColor = __xuiMsgTipIconColor(pTip->iType);
	}
	if ( pDesc != NULL && pDesc->bHasCustomIcon && pDesc->pIconSurface != NULL ) {
		pTip->pIconSurface = pDesc->pIconSurface;
		pTip->tIconSrc = pDesc->tIconSrc;
		pTip->bCustomIcon = 1;
	}
	iRet = __xuiMsgTipReplaceText(&pTip->sText, (pDesc != NULL && pDesc->sText != NULL) ? pDesc->sText : "");
	if ( iRet != XUI_OK ) {
		xrtFree(pTip);
		return iRet;
	}
	iRet = xuiWidgetCreateTyped(pContext, pType, &pTip->pWidget, pTip);
	if ( iRet != XUI_OK ) {
		xuiMsgTipDestroy(pTip);
		return iRet;
	}
	iRet = __xuiMsgTipLayout(pTip);
	if ( iRet != XUI_OK ) {
		xuiMsgTipDestroy(pTip);
		return iRet;
	}
	*ppTip = pTip;
	return XUI_OK;
}

XUI_API void xuiMsgTipDestroy(xui_msgtip pTip)
{
	if ( !__xuiMsgTipValid(pTip) ) {
		return;
	}
	if ( pTip->bOpen ) {
		pTip->bOpen = 0;
		(void)xuiWidgetSetHitTestVisible(pTip->pWidget, 0);
		(void)xuiWidgetSetEnabled(pTip->pWidget, 0);
		(void)xuiWidgetSetVisible(pTip->pWidget, 0);
	}
	if ( pTip->pWidget != NULL ) {
		xuiWidgetDestroy(pTip->pWidget);
		pTip->pWidget = NULL;
	}
	if ( pTip->sText != NULL ) {
		xrtFree(pTip->sText);
		pTip->sText = NULL;
	}
	pTip->iMagic = 0;
	xrtFree(pTip);
}

XUI_API int xuiMsgTipShow(xui_msgtip pTip, int iType, const char* sText, float fDuration)
{
	char* sNewText;
	char* sOldText;
	uint32_t iOldIconColor;
	float fOldDuration;
	float fOldElapsed;
	int iOldType;
	int bOldOpen;
	int bWasAttached;
	int iRet;

	if ( !__xuiMsgTipValid(pTip) || !__xuiMsgTipTypeValid(iType) || !__xuiMsgTipFloatValid(fDuration) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	sNewText = __xuiMsgTipDupText(sText);
	if ( sNewText == NULL ) return XUI_ERROR_OUT_OF_MEMORY;
	sOldText = pTip->sText;
	iOldType = pTip->iType;
	iOldIconColor = pTip->tColors.iIconColor;
	fOldDuration = pTip->fDuration;
	fOldElapsed = pTip->fElapsed;
	bOldOpen = pTip->bOpen;
	bWasAttached = xuiWidgetGetParent(pTip->pWidget) != NULL;
	pTip->sText = sNewText;
	pTip->iType = iType;
	if ( !pTip->bCustomIcon ) {
		if ( !pTip->bHasColors ) pTip->tColors.iIconColor = __xuiMsgTipIconColor(pTip->iType);
	}
	pTip->fDuration = fDuration;
	pTip->fElapsed = 0.0f;
	iRet = __xuiMsgTipLayout(pTip);
	if ( iRet == XUI_OK && !bWasAttached ) {
		iRet = xuiOverlayAttach(pTip->pContext, NULL, pTip->pWidget, XUI_LAYER_TOOLTIP, 100);
	}
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetLayer(pTip->pWidget, XUI_LAYER_TOOLTIP, 100);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetVisible(pTip->pWidget, 1);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEnabled(pTip->pWidget, 1);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetHitTestVisible(pTip->pWidget, 1);
	if ( iRet == XUI_OK ) iRet = xuiOverlayBringToFront(pTip->pWidget);
	if ( iRet != XUI_OK ) {
		pTip->sText = sOldText;
		pTip->iType = iOldType;
		pTip->tColors.iIconColor = iOldIconColor;
		pTip->fDuration = fOldDuration;
		pTip->fElapsed = fOldElapsed;
		pTip->bOpen = bOldOpen;
		(void)__xuiMsgTipLayout(pTip);
		(void)xuiWidgetSetVisible(pTip->pWidget, bOldOpen);
		(void)xuiWidgetSetEnabled(pTip->pWidget, bOldOpen);
		(void)xuiWidgetSetHitTestVisible(pTip->pWidget, bOldOpen);
		if ( !bWasAttached && xuiWidgetGetParent(pTip->pWidget) != NULL ) (void)xuiOverlayDetach(pTip->pWidget);
		xrtFree(sNewText);
		return iRet;
	}
	pTip->bOpen = 1;
	if ( pTip->iShowCount < INT_MAX ) pTip->iShowCount++;
	if ( pTip->iChangeCount < INT_MAX ) pTip->iChangeCount++;
	if ( sOldText != NULL ) xrtFree(sOldText);
	return XUI_OK;
}

XUI_API int xuiMsgTipClose(xui_msgtip pTip)
{
	return __xuiMsgTipCloseInternal(pTip, 0);
}

XUI_API int xuiMsgTipIsOpen(xui_msgtip pTip)
{
	return __xuiMsgTipValid(pTip) ? pTip->bOpen : 0;
}

XUI_API int xuiMsgTipSetText(xui_msgtip pTip, const char* sText)
{
	char* sNewText;
	char* sOldText;
	int iRet;

	if ( !__xuiMsgTipValid(pTip) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( __xuiMsgTipTextSame(pTip->sText, sText) ) return XUI_OK;
	sNewText = __xuiMsgTipDupText(sText);
	if ( sNewText == NULL ) return XUI_ERROR_OUT_OF_MEMORY;
	sOldText = pTip->sText;
	pTip->sText = sNewText;
	iRet = __xuiMsgTipLayout(pTip);
	if ( iRet != XUI_OK ) {
		pTip->sText = sOldText;
		(void)__xuiMsgTipLayout(pTip);
		xrtFree(sNewText);
		return iRet;
	}
	if ( sOldText != NULL ) xrtFree(sOldText);
	if ( pTip->iChangeCount < INT_MAX ) pTip->iChangeCount++;
	return XUI_OK;
}

XUI_API const char* xuiMsgTipGetText(xui_msgtip pTip)
{
	return __xuiMsgTipValid(pTip) ? pTip->sText : NULL;
}

XUI_API int xuiMsgTipSetType(xui_msgtip pTip, int iType)
{
	uint32_t iOldIconColor;
	int iOldType;
	int iRet;

	if ( !__xuiMsgTipValid(pTip) || !__xuiMsgTipTypeValid(iType) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( pTip->iType == iType ) return XUI_OK;
	iOldType = pTip->iType;
	iOldIconColor = pTip->tColors.iIconColor;
	pTip->iType = iType;
	if ( !pTip->bCustomIcon ) {
		if ( !pTip->bHasColors ) pTip->tColors.iIconColor = __xuiMsgTipIconColor(pTip->iType);
	}
	iRet = __xuiMsgTipLayout(pTip);
	if ( iRet != XUI_OK ) {
		pTip->iType = iOldType;
		pTip->tColors.iIconColor = iOldIconColor;
		(void)__xuiMsgTipLayout(pTip);
		return iRet;
	}
	if ( pTip->iChangeCount < INT_MAX ) pTip->iChangeCount++;
	return XUI_OK;
}

XUI_API int xuiMsgTipGetType(xui_msgtip pTip)
{
	return __xuiMsgTipValid(pTip) ? pTip->iType : XUI_MSGTIP_ICON_NONE;
}

XUI_API int xuiMsgTipSetDuration(xui_msgtip pTip, float fDuration)
{
	if ( !__xuiMsgTipValid(pTip) || !__xuiMsgTipFloatValid(fDuration) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( pTip->fDuration == fDuration ) return XUI_OK;
	pTip->fDuration = fDuration;
	if ( pTip->iChangeCount < INT_MAX ) pTip->iChangeCount++;
	return XUI_OK;
}

XUI_API float xuiMsgTipGetDuration(xui_msgtip pTip)
{
	return __xuiMsgTipValid(pTip) ? pTip->fDuration : 0.0f;
}

XUI_API int xuiMsgTipSetIconSurface(xui_msgtip pTip, xui_surface pSurface, xui_rect_t tSrc)
{
	xui_surface pOldSurface;
	xui_rect_t tOldSrc;
	int bOldCustomIcon;
	int iRet;

	if ( !__xuiMsgTipValid(pTip) || !__xuiMsgTipRectValid(tSrc) ||
	     (pSurface != NULL && (tSrc.fW <= 0 || tSrc.fH <= 0)) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( pTip->pIconSurface == pSurface && pTip->bCustomIcon == (pSurface != NULL) &&
	     __xuiMsgTipRectSame(pTip->tIconSrc, tSrc) ) return XUI_OK;
	pOldSurface = pTip->pIconSurface;
	tOldSrc = pTip->tIconSrc;
	bOldCustomIcon = pTip->bCustomIcon;
	pTip->pIconSurface = pSurface;
	pTip->tIconSrc = tSrc;
	pTip->bCustomIcon = (pSurface != NULL);
	iRet = __xuiMsgTipLayout(pTip);
	if ( iRet != XUI_OK ) {
		pTip->pIconSurface = pOldSurface;
		pTip->tIconSrc = tOldSrc;
		pTip->bCustomIcon = bOldCustomIcon;
		(void)__xuiMsgTipLayout(pTip);
		return iRet;
	}
	if ( pTip->iChangeCount < INT_MAX ) pTip->iChangeCount++;
	return XUI_OK;
}

XUI_API int xuiMsgTipUseBuiltinIcon(xui_msgtip pTip)
{
	xui_surface pOldSurface;
	xui_rect_t tOldSrc;
	uint32_t iOldIconColor;
	int bOldCustomIcon;
	int iRet;

	if ( !__xuiMsgTipValid(pTip) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( !pTip->bCustomIcon && pTip->pIconSurface == NULL ) return XUI_OK;
	pOldSurface = pTip->pIconSurface;
	tOldSrc = pTip->tIconSrc;
	iOldIconColor = pTip->tColors.iIconColor;
	bOldCustomIcon = pTip->bCustomIcon;
	pTip->pIconSurface = NULL;
	pTip->tIconSrc = (xui_rect_t){0.0f, 0.0f, 0.0f, 0.0f};
	pTip->bCustomIcon = 0;
	if ( !pTip->bHasColors ) pTip->tColors.iIconColor = __xuiMsgTipIconColor(pTip->iType);
	iRet = __xuiMsgTipLayout(pTip);
	if ( iRet != XUI_OK ) {
		pTip->pIconSurface = pOldSurface;
		pTip->tIconSrc = tOldSrc;
		pTip->tColors.iIconColor = iOldIconColor;
		pTip->bCustomIcon = bOldCustomIcon;
		(void)__xuiMsgTipLayout(pTip);
		return iRet;
	}
	if ( pTip->iChangeCount < INT_MAX ) pTip->iChangeCount++;
	return XUI_OK;
}

XUI_API int xuiMsgTipSetMetrics(xui_msgtip pTip, const xui_msgtip_metrics_t* pMetrics)
{
	xui_msgtip_metrics_t tOldMetrics;
	xui_msgtip_metrics_t tNewMetrics;
	int iRet;

	if ( !__xuiMsgTipValid(pTip) || !__xuiMsgTipMetricsValid(pMetrics) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	tNewMetrics = *pMetrics;
	tNewMetrics.iSize = sizeof(tNewMetrics);
	if ( memcmp(&pTip->tMetrics, &tNewMetrics, sizeof(tNewMetrics)) == 0 ) return XUI_OK;
	tOldMetrics = pTip->tMetrics;
	pTip->tMetrics = tNewMetrics;
	iRet = __xuiMsgTipLayout(pTip);
	if ( iRet != XUI_OK ) {
		pTip->tMetrics = tOldMetrics;
		(void)__xuiMsgTipLayout(pTip);
		return iRet;
	}
	if ( pTip->iChangeCount < INT_MAX ) pTip->iChangeCount++;
	return XUI_OK;
}

XUI_API int xuiMsgTipGetMetrics(xui_msgtip pTip, xui_msgtip_metrics_t* pMetrics)
{
	if ( !__xuiMsgTipValid(pTip) || (pMetrics == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	*pMetrics = pTip->tMetrics;
	pMetrics->iSize = sizeof(*pMetrics);
	return XUI_OK;
}

XUI_API int xuiMsgTipSetColors(xui_msgtip pTip, const xui_msgtip_colors_t* pColors)
{
	xui_msgtip_colors_t tOldColors;
	xui_msgtip_colors_t tNewColors;
	int bOldHasColors;
	int iRet;

	if ( !__xuiMsgTipValid(pTip) || !__xuiMsgTipColorsValid(pColors) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	tNewColors = *pColors;
	tNewColors.iSize = sizeof(tNewColors);
	if ( pTip->bHasColors && memcmp(&pTip->tColors, &tNewColors, sizeof(tNewColors)) == 0 ) return XUI_OK;
	tOldColors = pTip->tColors;
	bOldHasColors = pTip->bHasColors;
	pTip->tColors = tNewColors;
	pTip->bHasColors = 1;
	iRet = xuiWidgetInvalidate(pTip->pWidget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	if ( iRet != XUI_OK ) {
		pTip->tColors = tOldColors;
		pTip->bHasColors = bOldHasColors;
		return iRet;
	}
	if ( pTip->iChangeCount < INT_MAX ) pTip->iChangeCount++;
	return XUI_OK;
}

XUI_API int xuiMsgTipGetColors(xui_msgtip pTip, xui_msgtip_colors_t* pColors)
{
	if ( !__xuiMsgTipValid(pTip) || (pColors == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	*pColors = pTip->tColors;
	pColors->iSize = sizeof(*pColors);
	return XUI_OK;
}

XUI_API int xuiMsgTipSetClose(xui_msgtip pTip, xui_msgtip_close_proc onClose, void* pUser)
{
	if ( !__xuiMsgTipValid(pTip) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pTip->onClose = onClose;
	pTip->pCloseUser = pUser;
	return XUI_OK;
}

XUI_API xui_widget xuiMsgTipGetWidget(xui_msgtip pTip)
{
	return __xuiMsgTipValid(pTip) ? pTip->pWidget : NULL;
}

XUI_API xui_rect_t xuiMsgTipGetTipRect(xui_msgtip pTip)
{
	return __xuiMsgTipValid(pTip) ? pTip->tTipRect : (xui_rect_t){0.0f, 0.0f, 0.0f, 0.0f};
}

XUI_API xui_rect_t xuiMsgTipGetIconRect(xui_msgtip pTip)
{
	return __xuiMsgTipValid(pTip) ? pTip->tIconRect : (xui_rect_t){0.0f, 0.0f, 0.0f, 0.0f};
}

XUI_API xui_rect_t xuiMsgTipGetTextRect(xui_msgtip pTip)
{
	return __xuiMsgTipValid(pTip) ? pTip->tTextRect : (xui_rect_t){0.0f, 0.0f, 0.0f, 0.0f};
}

XUI_API int xuiMsgTipGetWrapLineCount(xui_msgtip pTip)
{
	return __xuiMsgTipValid(pTip) ? pTip->iWrapLineCount : 0;
}

XUI_API int xuiMsgTipGetShowCount(xui_msgtip pTip)
{
	return __xuiMsgTipValid(pTip) ? pTip->iShowCount : 0;
}

XUI_API int xuiMsgTipGetCloseCount(xui_msgtip pTip)
{
	return __xuiMsgTipValid(pTip) ? pTip->iCloseCount : 0;
}

XUI_API int xuiMsgTipGetExpireCount(xui_msgtip pTip)
{
	return __xuiMsgTipValid(pTip) ? pTip->iExpireCount : 0;
}

XUI_API int xuiMsgTipGetChangeCount(xui_msgtip pTip)
{
	return __xuiMsgTipValid(pTip) ? pTip->iChangeCount : 0;
}

#endif
