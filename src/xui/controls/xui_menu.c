#include "xui_config.h"
#if XUI_ENABLE_MENU
#include "xui_internal.h"
#include "xui_pointer_press_internal.h"

#include <ctype.h>
#include <limits.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#define XUI_MENU_TEXT_SAFETY_PAD 6.0f
#define XUI_MENU_SUBMENU_HOVER_DELAY 0.20f
#define XUI_MENU_SUBMENU_AIM_DELAY 0.30f
#define XUI_MENU_ITEM_STATE_MASK (XUI_MENU_ITEM_ENABLED | XUI_MENU_ITEM_CHECKED | XUI_MENU_ITEM_DEFAULT | XUI_MENU_ITEM_DANGER)

/* Private bridge between these two overlay implementations, not a public API. */
int xuiInternalPopupSetPanelColors(xui_widget pWidget, uint32_t iPanel, uint32_t iBorder, uint32_t iShadow);

typedef struct xui_menu_data_t {
	struct xui_menu_data_t* pNext;
	xui_widget pWidget;
	xui_widget pPopup;
	xui_widget pParentMenu;
	xui_widget pOpenSubmenu;
	xui_font pFont;
	xui_menu_item_t arrItems[XUI_MENU_ITEM_CAPACITY];
	xui_rect_t arrItemRect[XUI_MENU_ITEM_CAPACITY];
	float arrTextWidth[XUI_MENU_ITEM_CAPACITY];
	float arrShortcutWidth[XUI_MENU_ITEM_CAPACITY];
	xui_menu_metrics_t tMetrics;
	xui_menu_colors_t tColors;
	xui_menu_select_proc onSelect;
	void* pSelectUser;
	float fContentW;
	float fContentH;
	int iItemCount;
	int iHover;
	int iPressed;
	xui_pointer_owner_t tPointerOwner;
	int iPendingSubmenu;
	float fPendingSubmenuTime;
	float fLastPointerX;
	float fLastPointerY;
	float fAimOriginX;
	float fAimOriginY;
	float fDeferredHoverTime;
	int iParentItem;
	int iSelectCount;
	int iDeferredHover;
	int bLastPointer;
	int bAimActive;
	int bMeasureDirty;
	uint32_t iMeasureDpiGeneration;
} xui_menu_data_t;

static xui_menu_data_t* __xuiMenuGetData(xui_widget pWidget);
static int __xuiMenuApplyPopupSize(xui_widget pWidget, xui_menu_data_t* pData);
static int __xuiMenuCloseSubmenu(xui_widget pWidget, xui_menu_data_t* pData);
static int __xuiMenuUpdate(xui_widget pWidget, float fDelta, void* pUser);

static int __xuiMenuCancelPress(xui_widget pWidget, xui_menu_data_t* pData)
{
	int iRet;

	if ( (pWidget == NULL) || (pData == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	pData->iPressed = -1;
	iRet = xuiInternalPointerOwnerCancel(xuiWidgetGetContext(pWidget), pWidget,
		&pData->tPointerOwner);
	if ( iRet != XUI_OK ) return iRet;
	return XUI_OK;
}
static int __xuiMenuMetricsValid(const xui_menu_metrics_t* pMetrics);
static int __xuiMenuColorsValid(const xui_menu_colors_t* pColors);

static xui_menu_data_t* __xuiMenuFindData(xui_context pContext, xui_widget pWidget)
{
	xui_menu_data_t* pScan;

	if ( !xuiInternalContextIsValid(pContext) || pWidget == NULL ) return NULL;
	for ( pScan = pContext->pMenuList; pScan != NULL; pScan = pScan->pNext ) {
		if ( pScan->pWidget == pWidget ) {
			return xuiInternalWidgetIsValid(pWidget) ? pScan : NULL;
		}
	}
	return NULL;
}

int xuiInternalMenuIsLive(xui_context pContext, xui_widget pWidget)
{
	return __xuiMenuFindData(pContext, pWidget) != NULL;
}

static const struct {
	const char* sName;
	size_t iOffset;
} g_xuiMenuColorProperties[] = {
	{"menu.panel.color", offsetof(xui_menu_colors_t, iPanelColor)},
	{"menu.border.color", offsetof(xui_menu_colors_t, iBorderColor)},
	{"menu.shadow.color", offsetof(xui_menu_colors_t, iShadowColor)},
	{"menu.item.hover_color", offsetof(xui_menu_colors_t, iHoverColor)},
	{"menu.text.color", offsetof(xui_menu_colors_t, iTextColor)},
	{"menu.text.hover_color", offsetof(xui_menu_colors_t, iHoverTextColor)},
	{"menu.text.disabled_color", offsetof(xui_menu_colors_t, iDisabledTextColor)},
	{"menu.shortcut.color", offsetof(xui_menu_colors_t, iShortcutColor)},
	{"menu.text.danger_color", offsetof(xui_menu_colors_t, iDangerTextColor)},
	{"menu.mark.color", offsetof(xui_menu_colors_t, iMarkColor)},
	{"menu.separator.color", offsetof(xui_menu_colors_t, iSeparatorColor)},
	{"menu.focus.color", offsetof(xui_menu_colors_t, iFocusColor)}
};

static void __xuiMenuResolveColors(xui_widget pWidget, const xui_menu_data_t* pData, xui_menu_colors_t* pColors)
{
	xui_style_property_t tProperty;
	size_t i;
	*pColors = pData->tColors;
	for ( i = 0; i < sizeof(g_xuiMenuColorProperties) / sizeof(g_xuiMenuColorProperties[0]); ++i ) {
		memset(&tProperty, 0, sizeof(tProperty));
		tProperty.iSize = sizeof(tProperty);
		if ( xuiWidgetGetResolvedStyleProperty(pWidget, g_xuiMenuColorProperties[i].sName, &tProperty) == XUI_OK &&
			tProperty.tValue.iType == XUI_STYLE_VALUE_COLOR )
			*(uint32_t*)((char*)pColors + g_xuiMenuColorProperties[i].iOffset) = tProperty.tValue.iColor;
	}
}

static void __xuiMenuRegisterColors(xui_context pContext, xui_widget_type pType)
{
	xui_style_property_info_t tInfo;
	size_t i;
	memset(&tInfo, 0, sizeof(tInfo));
	tInfo.iSize = sizeof(tInfo);
	tInfo.pWidgetType = pType;
	tInfo.iValueType = XUI_STYLE_VALUE_COLOR;
	tInfo.iDirtyFlags = XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER;
	for ( i = 0; i < sizeof(g_xuiMenuColorProperties) / sizeof(g_xuiMenuColorProperties[0]); ++i ) {
		tInfo.sName = g_xuiMenuColorProperties[i].sName;
		(void)xuiStyleRegisterProperty(pContext, &tInfo, NULL);
	}
}

static int __xuiMenuApplyPopupColors(xui_menu_data_t* pData, const xui_menu_colors_t* pColors)
{
	if ( pData->pPopup != NULL )
		return xuiInternalPopupSetPanelColors(pData->pPopup, pColors->iPanelColor, pColors->iBorderColor, pColors->iShadowColor);
	return XUI_OK;
}

static int __xuiMenuPreparePaint(xui_widget pWidget)
{
	xui_menu_data_t* pData = __xuiMenuGetData(pWidget);
	xui_menu_colors_t tColors;
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	__xuiMenuResolveColors(pWidget, pData, &tColors);
	return __xuiMenuApplyPopupColors(pData, &tColors);
}

static int __xuiMenuAlpha(uint32_t iColor)
{
	return (int)(iColor & 0xffu);
}

static float __xuiMenuMax(float fA, float fB)
{
	return (fA > fB) ? fA : fB;
}

static int __xuiMenuCoordinateValid(float fValue)
{
	return isfinite(fValue) &&
	       fValue >= -XUI_CONTEXT_MAX_VIEWPORT &&
	       fValue <= XUI_CONTEXT_MAX_VIEWPORT;
}

static int __xuiMenuMetricValid(float fValue)
{
	return isfinite(fValue) && fValue >= 0.0f && fValue <= XUI_LAYOUT_UNBOUNDED;
}

static const char* __xuiMenuText(const char* sText)
{
	return (sText != NULL) ? sText : "";
}

static int __xuiMenuDescValid(const xui_menu_desc_t* pDesc)
{
	if ( pDesc == NULL ) {
		return 1;
	}
	if ( (pDesc->iSize != 0) && (pDesc->iSize < sizeof(*pDesc)) ) return 0;
	if ( pDesc->bHasMetrics && !__xuiMenuMetricsValid(&pDesc->tMetrics) ) return 0;
	if ( pDesc->bHasColors && !__xuiMenuColorsValid(&pDesc->tColors) ) return 0;
	return 1;
}

static int __xuiMenuMetricsValid(const xui_menu_metrics_t* pMetrics)
{
	double fFixedWidth;
	double fMaximumHeight;
	double fMaximumItemHeight;

	if ( pMetrics == NULL ) {
		return 0;
	}
	if ( (pMetrics->iSize != 0) && (pMetrics->iSize < sizeof(*pMetrics)) ) {
		return 0;
	}
	if ( !__xuiMenuMetricValid(pMetrics->fItemHeight) ||
	     !__xuiMenuMetricValid(pMetrics->fSeparatorHeight) ||
	     !__xuiMenuMetricValid(pMetrics->fPaddingX) ||
	     !__xuiMenuMetricValid(pMetrics->fPaddingY) ||
	     !__xuiMenuMetricValid(pMetrics->fMarkWidth) ||
	     !__xuiMenuMetricValid(pMetrics->fIconWidth) ||
	     !__xuiMenuMetricValid(pMetrics->fShortcutGap) ||
	     !__xuiMenuMetricValid(pMetrics->fArrowWidth) ||
	     !__xuiMenuMetricValid(pMetrics->fMinWidth) ||
	     !__xuiMenuMetricValid(pMetrics->fMaxHeight) ||
	     (pMetrics->fItemHeight <= 0.0f) ||
	     (pMetrics->fSeparatorHeight <= 0.0f) ) {
		return 0;
	}
	fFixedWidth = (double)pMetrics->fPaddingX * 2.0 +
		(double)pMetrics->fMarkWidth + (double)pMetrics->fIconWidth +
		8.0 + (double)XUI_MENU_TEXT_SAFETY_PAD * 2.0 +
		(double)pMetrics->fShortcutGap + (double)pMetrics->fArrowWidth;
	fMaximumItemHeight = pMetrics->fItemHeight > pMetrics->fSeparatorHeight ?
		(double)pMetrics->fItemHeight : (double)pMetrics->fSeparatorHeight;
	fMaximumHeight = (double)pMetrics->fPaddingY * 2.0 +
		fMaximumItemHeight * (double)XUI_MENU_ITEM_CAPACITY;
	if ( fFixedWidth > (double)XUI_LAYOUT_UNBOUNDED ||
	     fMaximumHeight > (double)XUI_LAYOUT_UNBOUNDED ) return 0;
	return 1;
}

static int __xuiMenuColorsValid(const xui_menu_colors_t* pColors)
{
	if ( pColors == NULL ) {
		return 0;
	}
	return (pColors->iSize == 0) || (pColors->iSize >= sizeof(*pColors));
}

static int __xuiMenuMetricsEqual(const xui_menu_metrics_t* pA, const xui_menu_metrics_t* pB)
{
	return pA->fItemHeight == pB->fItemHeight &&
	       pA->fSeparatorHeight == pB->fSeparatorHeight &&
	       pA->fPaddingX == pB->fPaddingX &&
	       pA->fPaddingY == pB->fPaddingY &&
	       pA->fMarkWidth == pB->fMarkWidth &&
	       pA->fIconWidth == pB->fIconWidth &&
	       pA->fShortcutGap == pB->fShortcutGap &&
	       pA->fArrowWidth == pB->fArrowWidth &&
	       pA->fMinWidth == pB->fMinWidth &&
	       pA->fMaxHeight == pB->fMaxHeight;
}

static int __xuiMenuColorsEqual(const xui_menu_colors_t* pA, const xui_menu_colors_t* pB)
{
	return pA->iPanelColor == pB->iPanelColor &&
	       pA->iBorderColor == pB->iBorderColor &&
	       pA->iShadowColor == pB->iShadowColor &&
	       pA->iHoverColor == pB->iHoverColor &&
	       pA->iTextColor == pB->iTextColor &&
	       pA->iHoverTextColor == pB->iHoverTextColor &&
	       pA->iDisabledTextColor == pB->iDisabledTextColor &&
	       pA->iShortcutColor == pB->iShortcutColor &&
	       pA->iDangerTextColor == pB->iDangerTextColor &&
	       pA->iMarkColor == pB->iMarkColor &&
	       pA->iSeparatorColor == pB->iSeparatorColor &&
	       pA->iFocusColor == pB->iFocusColor;
}

static int __xuiMenuItemTypeValid(int iType)
{
	return (iType >= XUI_MENU_ITEM_NORMAL) && (iType <= XUI_MENU_ITEM_SUBMENU);
}

static int __xuiMenuItemEnabled(const xui_menu_item_t* pItem)
{
	return (pItem != NULL) &&
	       (pItem->iType != XUI_MENU_ITEM_SEPARATOR) &&
	       ((pItem->iState & XUI_MENU_ITEM_ENABLED) != 0u);
}

static int __xuiMenuStringEqual(const char* sA, const char* sB)
{
	if ( sA == sB ) return 1;
	if ( sA == NULL || sB == NULL ) return 0;
	return strcmp(sA, sB) == 0;
}

static int __xuiMenuItemsEqual(const xui_menu_data_t* pData,
	const xui_menu_item_t* pItems, int iCount)
{
	const xui_menu_item_t* pCurrent;
	const xui_menu_item_t* pNext;
	int i;

	if ( pData == NULL || pData->iItemCount != iCount ) return 0;
	for ( i = 0; i < iCount; i++ ) {
		pCurrent = &pData->arrItems[i];
		pNext = &pItems[i];
		if ( !__xuiMenuStringEqual(pCurrent->sText, pNext->sText) ||
		     !__xuiMenuStringEqual(pCurrent->sShortcut, pNext->sShortcut) ||
		     pCurrent->iType != pNext->iType ||
		     pCurrent->iState != pNext->iState ||
		     pCurrent->iValue != pNext->iValue ||
		     pCurrent->iIcon != pNext->iIcon ||
		     pCurrent->pSubmenu != pNext->pSubmenu ||
		     pCurrent->pUser != pNext->pUser ) return 0;
	}
	return 1;
}

static void __xuiMenuFreeItemStrings(xui_menu_item_t* pItems, int iCount)
{
	int i;

	if ( pItems == NULL ) return;
	for ( i = 0; i < iCount; i++ ) {
		xrtFree((void*)pItems[i].sText);
		xrtFree((void*)pItems[i].sShortcut);
		pItems[i].sText = NULL;
		pItems[i].sShortcut = NULL;
	}
}

static char* __xuiMenuDuplicateString(const char* sText)
{
	char* sCopy;
	size_t iSize;

	if ( sText == NULL ) return NULL;
	iSize = strlen(sText) + 1u;
	sCopy = (char*)xrtMalloc(iSize);
	if ( sCopy != NULL ) memcpy(sCopy, sText, iSize);
	return sCopy;
}

static int __xuiMenuCopyItems(xui_menu_item_t* pTarget, const xui_menu_item_t* pItems, int iCount)
{
	int i;

	memset(pTarget, 0, sizeof(xui_menu_item_t) * XUI_MENU_ITEM_CAPACITY);
	for ( i = 0; i < iCount; i++ ) {
		pTarget[i] = pItems[i];
		pTarget[i].sText = __xuiMenuDuplicateString(pItems[i].sText);
		if ( (pItems[i].sText != NULL) && (pTarget[i].sText == NULL) ) goto failed;
		pTarget[i].sShortcut = __xuiMenuDuplicateString(pItems[i].sShortcut);
		if ( (pItems[i].sShortcut != NULL) && (pTarget[i].sShortcut == NULL) ) goto failed;
	}
	return XUI_OK;

failed:
	__xuiMenuFreeItemStrings(pTarget, i + 1);
	return XUI_ERROR_OUT_OF_MEMORY;
}

static int __xuiMenuItemValid(xui_widget pWidget, const xui_menu_item_t* pItem)
{
	xui_menu_data_t* pSubData;
	xui_context pContext;

	if ( (pWidget == NULL) || (pItem == NULL) || !__xuiMenuItemTypeValid(pItem->iType) ||
	     ((pItem->iState & ~XUI_MENU_ITEM_STATE_MASK) != 0u) ) return 0;
	if ( pItem->iType != XUI_MENU_ITEM_SUBMENU ) return 1;
	if ( (pItem->pSubmenu == NULL) || (pItem->pSubmenu == pWidget) ) return 0;
	pContext = xuiWidgetGetContext(pWidget);
	pSubData = __xuiMenuFindData(pContext, pItem->pSubmenu);
	return pSubData != NULL;
}

static void __xuiMenuDefaultMetrics(xui_menu_metrics_t* pMetrics)
{
	memset(pMetrics, 0, sizeof(*pMetrics));
	pMetrics->iSize = sizeof(*pMetrics);
	pMetrics->fItemHeight = 24.0f;
	pMetrics->fSeparatorHeight = 9.0f;
	pMetrics->fPaddingX = 1.0f;
	pMetrics->fPaddingY = 1.0f;
	pMetrics->fMarkWidth = 22.0f;
	pMetrics->fIconWidth = 4.0f;
	pMetrics->fShortcutGap = 20.0f;
	pMetrics->fArrowWidth = 16.0f;
	pMetrics->fMinWidth = 112.0f;
	pMetrics->fMaxHeight = 0.0f;
}

static void __xuiMenuDefaultColors(xui_menu_colors_t* pColors)
{
	memset(pColors, 0, sizeof(*pColors));
	pColors->iSize = sizeof(*pColors);
	pColors->iPanelColor = XUI_COLOR_RGBA(250, 252, 255, 255);
	pColors->iBorderColor = XUI_COLOR_RGBA(122, 164, 202, 255);
	pColors->iShadowColor = XUI_COLOR_RGBA(44, 70, 96, 46);
	pColors->iHoverColor = XUI_COLOR_RGBA(54, 125, 190, 255);
	pColors->iTextColor = XUI_COLOR_RGBA(28, 60, 94, 255);
	pColors->iHoverTextColor = XUI_COLOR_RGBA(255, 255, 255, 255);
	pColors->iDisabledTextColor = XUI_COLOR_RGBA(142, 152, 166, 210);
	pColors->iShortcutColor = XUI_COLOR_RGBA(84, 111, 140, 255);
	pColors->iDangerTextColor = XUI_COLOR_RGBA(184, 54, 54, 255);
	pColors->iMarkColor = XUI_COLOR_RGBA(37, 94, 145, 255);
	pColors->iSeparatorColor = XUI_COLOR_RGBA(202, 218, 232, 255);
	/* Menus use the active item highlight for keyboard focus by default. */
	pColors->iFocusColor = XUI_COLOR_RGBA(0, 0, 0, 0);
}

static xui_font __xuiMenuResolveFont(xui_widget pWidget, xui_menu_data_t* pData)
{
	if ( (pData != NULL) && (pData->pFont != NULL) ) {
		return pData->pFont;
	}
	return xuiGetDefaultFont(xuiWidgetGetContext(pWidget));
}

static xui_vec2_t __xuiMenuMeasureText(xui_widget pWidget, xui_font pFont, const char* sText)
{
	xui_vec2_t tSize;
	xui_proxy pProxy;
	const unsigned char* pScan;
	float fAsciiW;
	float fWideW;
	float fEstimate;

	tSize.fX = 0.0f;
	tSize.fY = 18.0f;
	if ( (sText == NULL) || (sText[0] == '\0') ) {
		return tSize;
	}
	pProxy = xuiInternalContextGetProxy(xuiWidgetGetContext(pWidget));
	fAsciiW = 7.0f;
	fWideW = 14.0f;
	if ( (pProxy != NULL) && (pProxy->fontGetMetrics != NULL) && (pFont != NULL) ) {
		xui_font_metrics_t tMetrics;
		if ( (pProxy->fontGetMetrics(pProxy, pFont, &tMetrics) == XUI_OK) && (tMetrics.fSize > 0.0f) ) {
			fAsciiW = tMetrics.fSize * 0.5f;
			fWideW = tMetrics.fSize;
			tSize.fY = (tMetrics.fLineHeight > 0.0f) ? tMetrics.fLineHeight : tMetrics.fSize;
		}
	}
	fEstimate = 0.0f;
	pScan = (const unsigned char*)sText;
	while ( *pScan != '\0' ) {
		if ( *pScan == '\n' ) {
			break;
		}
		if ( *pScan < 0x80u ) {
			fEstimate += fAsciiW;
			pScan++;
		} else {
			fEstimate += fWideW;
			if ( (*pScan & 0xe0u) == 0xc0u && pScan[1] != '\0' ) {
				pScan += 2;
			} else if ( (*pScan & 0xf0u) == 0xe0u && pScan[1] != '\0' && pScan[2] != '\0' ) {
				pScan += 3;
			} else if ( (*pScan & 0xf8u) == 0xf0u && pScan[1] != '\0' && pScan[2] != '\0' && pScan[3] != '\0' ) {
				pScan += 4;
			} else {
				pScan++;
			}
		}
	}
	if ( (pProxy != NULL) && (pProxy->textMeasure != NULL) && (pFont != NULL) &&
	     (pProxy->textMeasure(pProxy, &(xui_text_item_t){.iSize=sizeof(xui_text_item_t), .pFont=pFont, .sText=sText, .iTextSize=-1, .iFlags=XUI_TEXT_SHAPE_DEFAULT}, &tSize) == XUI_OK) &&
	     isfinite(tSize.fX) && isfinite(tSize.fY) &&
	     (tSize.fX >= 0.0f) && (tSize.fY >= 0.0f) ) {
		return tSize;
	}
	tSize.fX = fEstimate;
	return tSize;
}

static int __xuiMenuTextExtentValid(xui_vec2_t tSize)
{
	return isfinite(tSize.fX) && isfinite(tSize.fY) &&
	       tSize.fX >= 0.0f && tSize.fY >= 0.0f &&
	       tSize.fX <= XUI_LAYOUT_UNBOUNDED &&
	       tSize.fY <= XUI_LAYOUT_UNBOUNDED;
}

static int __xuiMenuMeasureData(xui_widget pWidget, xui_menu_data_t* pData)
{
	xui_context pContext;
	xui_font pFont;
	xui_vec2_t tText;
	xui_vec2_t tShortcut;
	float fTextW;
	float fShortcutW;
	double fWidth;
	double fHeight;
	float fItemH;
	int i;

	if ( (pWidget == NULL) || (pData == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pContext = xuiWidgetGetContext(pWidget);
	if ( !xuiInternalContextIsValid(pContext) ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( !pData->bMeasureDirty &&
	     pData->iMeasureDpiGeneration == pContext->iDpiGeneration ) return XUI_OK;
	pFont = __xuiMenuResolveFont(pWidget, pData);
	fTextW = 0.0f;
	fShortcutW = 0.0f;
	fHeight = pData->tMetrics.fPaddingY * 2.0f;
	for ( i = 0; i < pData->iItemCount; i++ ) {
		if ( pData->arrItems[i].iType == XUI_MENU_ITEM_SEPARATOR ) {
			fHeight += pData->tMetrics.fSeparatorHeight;
			continue;
		}
		tText = __xuiMenuMeasureText(pWidget, pFont, pData->arrItems[i].sText);
		tShortcut = __xuiMenuMeasureText(pWidget, pFont, pData->arrItems[i].sShortcut);
		if ( !__xuiMenuTextExtentValid(tText) || !__xuiMenuTextExtentValid(tShortcut) ) {
			return XUI_ERROR_INVALID_ARGUMENT;
		}
		pData->arrTextWidth[i] = tText.fX;
		pData->arrShortcutWidth[i] = tShortcut.fX;
		fTextW = __xuiMenuMax(fTextW, tText.fX);
		fShortcutW = __xuiMenuMax(fShortcutW, tShortcut.fX);
		fHeight += pData->tMetrics.fItemHeight;
	}
	fWidth = (double)pData->tMetrics.fPaddingX * 2.0 +
	         (double)pData->tMetrics.fMarkWidth +
	         (double)pData->tMetrics.fIconWidth +
	         4.0 +
	         (double)fTextW +
	         (double)XUI_MENU_TEXT_SAFETY_PAD +
	         4.0 +
	         (double)pData->tMetrics.fArrowWidth;
	if ( fShortcutW > 0.0f ) {
		fWidth += (double)pData->tMetrics.fShortcutGap +
			(double)fShortcutW + (double)XUI_MENU_TEXT_SAFETY_PAD;
	}
	if ( fWidth < (double)pData->tMetrics.fMinWidth ) fWidth = pData->tMetrics.fMinWidth;
	if ( fWidth > (double)XUI_LAYOUT_UNBOUNDED ||
	     fHeight > (double)XUI_LAYOUT_UNBOUNDED ) return XUI_ERROR_INVALID_ARGUMENT;
	pData->fContentW = (float)xuiInternalSnapSize((float)fWidth);
	pData->fContentH = (float)xuiInternalSnapSize((float)fHeight);
	fHeight = pData->tMetrics.fPaddingY;
	for ( i = 0; i < pData->iItemCount; i++ ) {
		fItemH = (pData->arrItems[i].iType == XUI_MENU_ITEM_SEPARATOR) ?
			pData->tMetrics.fSeparatorHeight : pData->tMetrics.fItemHeight;
		pData->arrItemRect[i] = xuiInternalSnapRect((xui_rect_t){
			pData->tMetrics.fPaddingX,
			(float)fHeight,
			__xuiMenuMax(1.0f, pData->fContentW - pData->tMetrics.fPaddingX * 2.0f),
			fItemH
		});
		fHeight += fItemH;
	}
	for ( ; i < XUI_MENU_ITEM_CAPACITY; i++ ) {
		pData->arrTextWidth[i] = 0.0f;
		pData->arrShortcutWidth[i] = 0.0f;
		memset(&pData->arrItemRect[i], 0, sizeof(pData->arrItemRect[i]));
	}
	pData->bMeasureDirty = 0;
	pData->iMeasureDpiGeneration = pContext->iDpiGeneration;
	return XUI_OK;
}

static int __xuiMenuIndexAt(xui_menu_data_t* pData, float fX, float fY)
{
	xui_rect_t tRect;
	int i;

	if ( pData == NULL ) {
		return -1;
	}
	for ( i = 0; i < pData->iItemCount; i++ ) {
		if ( pData->arrItems[i].iType == XUI_MENU_ITEM_SEPARATOR ) {
			continue;
		}
		tRect = pData->arrItemRect[i];
		if ( (fX >= tRect.fX) &&
		     (fY >= tRect.fY) &&
		     (fX < tRect.fX + tRect.fW) &&
		     (fY < tRect.fY + tRect.fH) ) {
			return i;
		}
	}
	return -1;
}

static int __xuiMenuNextEnabled(xui_menu_data_t* pData, int iStart, int iStep)
{
	int i;
	int iIndex;

	if ( (pData == NULL) || (pData->iItemCount <= 0) || (iStep == 0) ) {
		return -1;
	}
	iIndex = iStart;
	for ( i = 0; i < pData->iItemCount; i++ ) {
		iIndex += iStep;
		if ( iIndex < 0 ) {
			iIndex = pData->iItemCount - 1;
		} else if ( iIndex >= pData->iItemCount ) {
			iIndex = 0;
		}
		if ( __xuiMenuItemEnabled(&pData->arrItems[iIndex]) ) {
			return iIndex;
		}
	}
	return -1;
}

static int __xuiMenuInvalidateItem(xui_widget pWidget, xui_menu_data_t* pData, int iIndex)
{
	xui_rect_t tRect;
	int64_t iLeft;
	int64_t iTop;
	int64_t iRight;
	int64_t iBottom;

	if ( (pWidget == NULL) || (pData == NULL) || (iIndex < 0) || (iIndex >= pData->iItemCount) ) {
		return XUI_OK;
	}
	tRect = pData->arrItemRect[iIndex];
	if ( (tRect.fW <= 0.0f) || (tRect.fH <= 0.0f) ) {
		return XUI_OK;
	}
	iLeft = (int64_t)tRect.fX - 2;
	iTop = (int64_t)tRect.fY - 2;
	iRight = (int64_t)tRect.fX + (int64_t)tRect.fW + 2;
	iBottom = (int64_t)tRect.fY + (int64_t)tRect.fH + 2;
	if ( iLeft < 0 ) iLeft = 0;
	if ( iTop < 0 ) iTop = 0;
	if ( iRight > (int64_t)pData->fContentW ) iRight = (int64_t)pData->fContentW;
	if ( iBottom > (int64_t)pData->fContentH ) iBottom = (int64_t)pData->fContentH;
	tRect = (xui_rect_t){
		(int)iLeft,
		(int)iTop,
		(int)(iRight > iLeft ? iRight - iLeft : 0),
		(int)(iBottom > iTop ? iBottom - iTop : 0)
	};
	return xuiWidgetInvalidateRect(pWidget, tRect, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
}

static int __xuiMenuEnsureItemVisible(xui_menu_data_t* pData, int iIndex)
{
	xui_widget pScrollView;
	xui_rect_t tTarget;
	float fOffsetX;
	int iRet;

	if ( pData == NULL || iIndex < 0 || iIndex >= pData->iItemCount ||
	     pData->pPopup == NULL || !xuiPopupIsOpen(pData->pPopup) ) return XUI_OK;
	pScrollView = xuiPopupGetScrollViewWidget(pData->pPopup);
	if ( pScrollView == NULL ) return XUI_ERROR_NOT_INITIALIZED;
	iRet = xuiPopupGetScroll(pData->pPopup, &fOffsetX, NULL);
	if ( iRet != XUI_OK ) return iRet;
	tTarget = pData->arrItemRect[iIndex];
	/* Keyboard navigation only needs vertical reveal. Preserve any deliberate
	 * horizontal scroll instead of aligning the full-width row. */
	tTarget.fX = xuiInternalSnapPixel(fOffsetX);
	tTarget.fW = 1;
	return xuiScrollViewEnsureRectVisible(pScrollView, tTarget);
}

static int __xuiMenuSetHover(xui_widget pWidget, xui_menu_data_t* pData, int iIndex)
{
	int iOldHover;
	int iRet;

	if ( (pWidget == NULL) || (pData == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( (iIndex < 0) || (iIndex >= pData->iItemCount) ||
	     !__xuiMenuItemEnabled(&pData->arrItems[iIndex]) ) {
		iIndex = -1;
	}
	if ( pData->iHover == iIndex ) {
		return XUI_OK;
	}
	iOldHover = pData->iHover;
	pData->iHover = iIndex;
	iRet = __xuiMenuInvalidateItem(pWidget, pData, iOldHover);
	if ( iRet == XUI_OK ) {
		iRet = __xuiMenuInvalidateItem(pWidget, pData, iIndex);
	}
	if ( iRet == XUI_OK ) {
		iRet = __xuiMenuEnsureItemVisible(pData, iIndex);
	}
	if ( iRet != XUI_OK ) {
		pData->iHover = iOldHover;
		(void)__xuiMenuInvalidateItem(pWidget, pData, iOldHover);
		(void)__xuiMenuInvalidateItem(pWidget, pData, iIndex);
		return iRet;
	}
	xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_SELECTION_CHANGED);
	return iRet;
}

static int __xuiMenuApplyHoverTarget(xui_widget pWidget, xui_menu_data_t* pData, int iIndex)
{
	int bInsideItem;
	int iRet;

	bInsideItem = iIndex >= 0 && iIndex < pData->iItemCount;
	iRet = __xuiMenuSetHover(pWidget, pData, iIndex);
	if ( iRet != XUI_OK ) return iRet;
	iIndex = pData->iHover;
	if ( iIndex >= 0 && pData->arrItems[iIndex].iType == XUI_MENU_ITEM_SUBMENU ) {
		if ( pData->iPendingSubmenu != iIndex ) {
			pData->iPendingSubmenu = iIndex;
			pData->fPendingSubmenuTime = 0.0f;
		}
		return XUI_OK;
	}
	if ( iIndex >= 0 ) return __xuiMenuCloseSubmenu(pWidget, pData);
	if ( bInsideItem ) return __xuiMenuCloseSubmenu(pWidget, pData);
	pData->iPendingSubmenu = -1;
	pData->fPendingSubmenuTime = 0.0f;
	return XUI_OK;
}

static float __xuiMenuTriangleSign(float fPX, float fPY, float fAX, float fAY, float fBX, float fBY)
{
	return (fPX - fBX) * (fAY - fBY) - (fAX - fBX) * (fPY - fBY);
}

static int __xuiMenuPointInTriangle(float fPX, float fPY,
	float fAX, float fAY, float fBX, float fBY, float fCX, float fCY)
{
	float fD1 = __xuiMenuTriangleSign(fPX, fPY, fAX, fAY, fBX, fBY);
	float fD2 = __xuiMenuTriangleSign(fPX, fPY, fBX, fBY, fCX, fCY);
	float fD3 = __xuiMenuTriangleSign(fPX, fPY, fCX, fCY, fAX, fAY);
	int bNegative = (fD1 < 0.0f) || (fD2 < 0.0f) || (fD3 < 0.0f);
	int bPositive = (fD1 > 0.0f) || (fD2 > 0.0f) || (fD3 > 0.0f);
	return !(bNegative && bPositive);
}

static int __xuiMenuShouldDeferHover(xui_widget pWidget, xui_menu_data_t* pData,
	float fX, float fY, int iIndex)
{
	xui_widget pSubmenu;
	xui_widget pPopup;
	xui_rect_t tMenu;
	xui_rect_t tPopup;
	float fEdgeX;
	int bRight;

	if ( !pData->bLastPointer || iIndex == pData->iHover || pData->iHover < 0 ||
	     __xuiMenuFindData(xuiWidgetGetContext(pWidget), pData->pOpenSubmenu) == NULL ||
	     !xuiMenuIsOpen(pData->pOpenSubmenu) ) return 0;
	if ( pData->iHover >= pData->iItemCount ||
	     pData->arrItems[pData->iHover].pSubmenu != pData->pOpenSubmenu ) return 0;
	pSubmenu = pData->pOpenSubmenu;
	pPopup = xuiMenuGetPopupWidget(pSubmenu);
	if ( pPopup == NULL ) return 0;
	tMenu = xuiWidgetGetWorldRect(pWidget);
	tPopup = xuiPopupGetPopupRect(pPopup);
	bRight = tPopup.fX + tPopup.fW * 0.5f >= tMenu.fX + tMenu.fW * 0.5f;
	if ( (bRight && fX + 0.5f < pData->fLastPointerX) ||
	     (!bRight && fX - 0.5f > pData->fLastPointerX) ) {
		pData->bAimActive = 0;
		return 0;
	}
	if ( !pData->bAimActive ) {
		pData->fAimOriginX = pData->fLastPointerX;
		pData->fAimOriginY = pData->fLastPointerY;
		pData->bAimActive = 1;
	}
	fEdgeX = bRight ? tPopup.fX : tPopup.fX + tPopup.fW;
	return __xuiMenuPointInTriangle(fX, fY,
		pData->fAimOriginX, pData->fAimOriginY,
		fEdgeX, tPopup.fY - 6.0f,
		fEdgeX, tPopup.fY + tPopup.fH + 6.0f);
}

static int __xuiMenuDrawRectFill(xui_proxy pProxy, xui_draw_context pDraw, xui_rect_t tRect, uint32_t iColor)
{
	if ( __xuiMenuAlpha(iColor) == 0 ) {
		return XUI_OK;
	}
	return (pProxy->drawRectFill != NULL) ? pProxy->drawRectFill(pProxy, pDraw, tRect, iColor) : XUI_OK;
}

static int __xuiMenuDrawRectStroke(xui_proxy pProxy, xui_draw_context pDraw, xui_rect_t tRect, float fWidth, uint32_t iColor)
{
	if ( (fWidth <= 0.0f) || (__xuiMenuAlpha(iColor) == 0) ) {
		return XUI_OK;
	}
	return (pProxy->drawRectStroke != NULL) ? pProxy->drawRectStroke(pProxy, pDraw, tRect, fWidth, iColor) : XUI_OK;
}

static int __xuiMenuDrawCheck(xui_proxy pProxy, xui_draw_context pDraw, xui_rect_t tRect, uint32_t iColor)
{
	float fX0;
	float fY0;
	float fX1;
	float fY1;
	float fX2;
	float fY2;
	int iRet;

	if ( pProxy->drawLine == NULL ) {
		return XUI_OK;
	}
	fX0 = tRect.fX + tRect.fW * 0.25f;
	fY0 = tRect.fY + tRect.fH * 0.54f;
	fX1 = tRect.fX + tRect.fW * 0.43f;
	fY1 = tRect.fY + tRect.fH * 0.72f;
	fX2 = tRect.fX + tRect.fW * 0.78f;
	fY2 = tRect.fY + tRect.fH * 0.28f;
	iRet = pProxy->drawLine(pProxy, pDraw, fX0, fY0, fX1, fY1, 2.0f, iColor);
	if ( iRet != XUI_OK ) return iRet;
	return pProxy->drawLine(pProxy, pDraw, fX1, fY1, fX2, fY2, 2.0f, iColor);
}

static int __xuiMenuDrawRadio(xui_proxy pProxy, xui_draw_context pDraw, xui_rect_t tRect, uint32_t iColor)
{
	float fCx;
	float fCy;
	float fR;
	int iRet;

	if ( (pProxy->drawCircleStroke == NULL) || (pProxy->drawCircleFill == NULL) ) {
		return XUI_OK;
	}
	fCx = tRect.fX + tRect.fW * 0.5f;
	fCy = tRect.fY + tRect.fH * 0.5f;
	fR = (tRect.fW < tRect.fH ? tRect.fW : tRect.fH) * 0.32f;
	iRet = pProxy->drawCircleStroke(pProxy, pDraw, fCx, fCy, fR, 1.4f, iColor);
	if ( iRet != XUI_OK ) return iRet;
	return pProxy->drawCircleFill(pProxy, pDraw, fCx, fCy, fR * 0.48f, iColor);
}

static int __xuiMenuDrawArrow(xui_proxy pProxy, xui_draw_context pDraw, xui_rect_t tRect, uint32_t iColor)
{
	xui_vec2_t tA;
	xui_vec2_t tB;
	xui_vec2_t tC;
	float fCx;
	float fCy;

	fCx = tRect.fX + tRect.fW * 0.56f;
	fCy = tRect.fY + tRect.fH * 0.5f;
	tA = (xui_vec2_t){fCx - 2.0f, fCy - 4.0f};
	tB = (xui_vec2_t){fCx - 2.0f, fCy + 4.0f};
	tC = (xui_vec2_t){fCx + 3.0f, fCy};
	if ( pProxy->drawTriangleFill != NULL ) {
		return pProxy->drawTriangleFill(pProxy, pDraw, tA, tB, tC, iColor);
	}
	if ( pProxy->drawLine != NULL ) {
		return pProxy->drawLine(pProxy, pDraw, tA.fX, tA.fY, tC.fX, tC.fY, 1.4f, iColor);
	}
	return XUI_OK;
}

static int __xuiMenuCacheRender(xui_widget pWidget, xui_draw_context pDraw, uint32_t iStateId, void* pUser)
{
	xui_menu_data_t* pData;
	xui_menu_colors_t tColors;
	xui_proxy pProxy;
	xui_font pFont;
	xui_rect_t tRect;
	xui_rect_t tItem;
	xui_rect_t tMark;
	xui_rect_t tText;
	xui_rect_t tShortcut;
	xui_rect_t tArrow;
	float fTextX;
	float fTextRight;
	float fShortcutW;
	uint32_t iTextColor;
	uint32_t iShortcutColor;
	int iRet;
	int i;

	(void)iStateId;
	(void)pUser;
	pData = __xuiMenuGetData(pWidget);
	if ( (pData == NULL) || (pDraw == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pProxy = xuiInternalContextGetProxy(xuiWidgetGetContext(pWidget));
	if ( pProxy == NULL ) {
		return XUI_ERROR_NOT_INITIALIZED;
	}
	iRet = __xuiMenuMeasureData(pWidget, pData);
	if ( iRet != XUI_OK ) return iRet;
	pFont = __xuiMenuResolveFont(pWidget, pData);
	__xuiMenuResolveColors(pWidget, pData, &tColors);
	tRect = (xui_rect_t){0.0f, 0.0f, pData->fContentW, pData->fContentH};
	iRet = __xuiMenuDrawRectFill(pProxy, pDraw, tRect, tColors.iPanelColor);
	if ( iRet != XUI_OK ) return iRet;
	for ( i = 0; i < pData->iItemCount; i++ ) {
		tItem = pData->arrItemRect[i];
		if ( pData->arrItems[i].iType == XUI_MENU_ITEM_SEPARATOR ) {
			if ( (pProxy->drawLine != NULL) && (__xuiMenuAlpha(tColors.iSeparatorColor) != 0) ) {
				iRet = pProxy->drawLine(pProxy, pDraw,
					tItem.fX + pData->tMetrics.fMarkWidth + 4.0f,
					tItem.fY + tItem.fH * 0.5f,
					tItem.fX + tItem.fW - 6.0f,
					tItem.fY + tItem.fH * 0.5f,
					1.0f,
					tColors.iSeparatorColor);
				if ( iRet != XUI_OK ) return iRet;
			}
			continue;
		}
		if ( i == pData->iHover ) {
			iRet = __xuiMenuDrawRectFill(pProxy, pDraw, tItem, tColors.iHoverColor);
			if ( iRet != XUI_OK ) return iRet;
		}
		tMark = (xui_rect_t){
			tItem.fX + 3.0f,
			tItem.fY + (tItem.fH - 15.0f) * 0.5f,
			15.0f,
			15.0f
		};
		iTextColor = tColors.iTextColor;
		iShortcutColor = tColors.iShortcutColor;
		if ( i == pData->iHover ) {
			iTextColor = tColors.iHoverTextColor;
			iShortcutColor = tColors.iHoverTextColor;
		}
		if ( !__xuiMenuItemEnabled(&pData->arrItems[i]) ) {
			iTextColor = tColors.iDisabledTextColor;
			iShortcutColor = tColors.iDisabledTextColor;
		} else if ( (pData->arrItems[i].iState & XUI_MENU_ITEM_DANGER) != 0u && i != pData->iHover ) {
			iTextColor = tColors.iDangerTextColor;
		}
		if ( ((pData->arrItems[i].iType == XUI_MENU_ITEM_CHECK) ||
		      (pData->arrItems[i].iType == XUI_MENU_ITEM_RADIO)) &&
		     ((pData->arrItems[i].iState & XUI_MENU_ITEM_CHECKED) != 0u) ) {
			if ( pData->arrItems[i].iType == XUI_MENU_ITEM_RADIO ) {
				iRet = __xuiMenuDrawRadio(pProxy, pDraw, tMark, (i == pData->iHover) ? tColors.iHoverTextColor : tColors.iMarkColor);
			} else {
				iRet = __xuiMenuDrawCheck(pProxy, pDraw, tMark, (i == pData->iHover) ? tColors.iHoverTextColor : tColors.iMarkColor);
			}
			if ( iRet != XUI_OK ) return iRet;
		}
		if ( pData->arrItems[i].iIcon != 0 ) {
			xui_rect_t tIcon = (xui_rect_t){
				tItem.fX + pData->tMetrics.fMarkWidth + 2.0f,
				tItem.fY + tItem.fH * 0.5f - 3.0f,
				6.0f,
				6.0f
			};
			iRet = __xuiMenuDrawRectFill(pProxy, pDraw, tIcon, (i == pData->iHover) ? tColors.iHoverTextColor : tColors.iMarkColor);
			if ( iRet != XUI_OK ) return iRet;
		}
		fTextX = tItem.fX + pData->tMetrics.fMarkWidth + pData->tMetrics.fIconWidth + 4.0f;
		fTextRight = tItem.fX + tItem.fW - pData->tMetrics.fArrowWidth - 4.0f;
		fShortcutW = 0.0f;
		if ( (pData->arrItems[i].sShortcut != NULL) && (pData->arrItems[i].sShortcut[0] != '\0') ) {
			fShortcutW = pData->arrShortcutWidth[i] + XUI_MENU_TEXT_SAFETY_PAD;
			fTextRight -= pData->tMetrics.fShortcutGap + fShortcutW;
		}
		tText = (xui_rect_t){
			fTextX,
			tItem.fY,
			__xuiMenuMax(1.0f, fTextRight - fTextX),
			tItem.fH
		};
		if ( (pFont != NULL) && (pProxy->drawText != NULL) && (pData->arrItems[i].sText != NULL) ) {
			iRet = pProxy->drawText(pProxy, pDraw, &(xui_text_item_t){.iSize=sizeof(xui_text_item_t), .pFont=pFont, .sText=pData->arrItems[i].sText, .iTextSize=-1, .iFlags=XUI_TEXT_SHAPE_DEFAULT | ((XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP) & XUI_TEXT_RTL ? XUI_TEXT_SHAPE_RTL : 0)}, tText, iTextColor, XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP);
			if ( iRet != XUI_OK ) return iRet;
		}
		if ( (pFont != NULL) && (pProxy->drawText != NULL) &&
		     (pData->arrItems[i].sShortcut != NULL) && (pData->arrItems[i].sShortcut[0] != '\0') ) {
			tShortcut = (xui_rect_t){
				tItem.fX + tItem.fW - pData->tMetrics.fArrowWidth - 4.0f - fShortcutW,
				tItem.fY,
				__xuiMenuMax(1.0f, fShortcutW),
				tItem.fH
			};
			iRet = pProxy->drawText(pProxy, pDraw, &(xui_text_item_t){.iSize=sizeof(xui_text_item_t), .pFont=pFont, .sText=pData->arrItems[i].sShortcut, .iTextSize=-1, .iFlags=XUI_TEXT_SHAPE_DEFAULT | ((XUI_TEXT_ALIGN_RIGHT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP) & XUI_TEXT_RTL ? XUI_TEXT_SHAPE_RTL : 0)}, tShortcut, iShortcutColor, XUI_TEXT_ALIGN_RIGHT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP);
			if ( iRet != XUI_OK ) return iRet;
		}
		if ( pData->arrItems[i].iType == XUI_MENU_ITEM_SUBMENU ) {
			tArrow = (xui_rect_t){tItem.fX + tItem.fW - pData->tMetrics.fArrowWidth, tItem.fY, pData->tMetrics.fArrowWidth, tItem.fH};
			iRet = __xuiMenuDrawArrow(pProxy, pDraw, tArrow, iShortcutColor);
			if ( iRet != XUI_OK ) return iRet;
		}
	}
	if ( (xuiWidgetGetInputState(pWidget) & XUI_WIDGET_STATE_FOCUS) != 0u )
		return __xuiMenuDrawRectStroke(pProxy, pDraw, tRect, 1.0f, tColors.iFocusColor);
	return XUI_OK;
}

static int __xuiMenuApplyCheckState(xui_menu_data_t* pData, int iIndex)
{
	int i;

	if ( (pData == NULL) || (iIndex < 0) || (iIndex >= pData->iItemCount) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( pData->arrItems[iIndex].iType == XUI_MENU_ITEM_CHECK ) {
		pData->arrItems[iIndex].iState ^= XUI_MENU_ITEM_CHECKED;
		return XUI_OK;
	}
	if ( pData->arrItems[iIndex].iType == XUI_MENU_ITEM_RADIO ) {
		for ( i = iIndex; i >= 0 && pData->arrItems[i].iType == XUI_MENU_ITEM_RADIO; i-- ) {
			pData->arrItems[i].iState &= ~XUI_MENU_ITEM_CHECKED;
		}
		for ( i = iIndex + 1; i < pData->iItemCount && pData->arrItems[i].iType == XUI_MENU_ITEM_RADIO; i++ ) {
			pData->arrItems[i].iState &= ~XUI_MENU_ITEM_CHECKED;
		}
		pData->arrItems[iIndex].iState |= XUI_MENU_ITEM_CHECKED;
	}
	return XUI_OK;
}

static xui_widget __xuiMenuRoot(xui_widget pWidget)
{
	xui_menu_data_t* pData;
	xui_widget pScan;
	int iDepth;

	pScan = pWidget;
	for ( iDepth = 0; iDepth < XUI_MENU_ITEM_CAPACITY; iDepth++ ) {
		pData = __xuiMenuGetData(pScan);
		if ( (pData == NULL) || (pData->pParentMenu == NULL) ) {
			break;
		}
		pScan = pData->pParentMenu;
	}
	return pScan;
}

static int __xuiMenuSubmenuWouldCycle(xui_widget pWidget, xui_widget pSubmenu)
{
	xui_menu_data_t* pData;
	xui_widget pScan;
	int iDepth;

	for ( pScan = pWidget, iDepth = 0;
	      pScan != NULL && iDepth < XUI_MENU_ITEM_CAPACITY;
	      iDepth++ ) {
		if ( pScan == pSubmenu ) return 1;
		pData = __xuiMenuGetData(pScan);
		if ( pData == NULL ) return 1;
		pScan = pData->pParentMenu;
	}
	return pScan != NULL;
}

static void __xuiMenuResolveSelect(xui_widget pWidget, xui_menu_select_proc* pOnSelect, void** ppUser)
{
	xui_menu_data_t* pData;
	xui_widget pScan;
	int iDepth;

	*pOnSelect = NULL;
	*ppUser = NULL;
	for ( pScan = pWidget, iDepth = 0;
	      pScan != NULL && iDepth < XUI_MENU_ITEM_CAPACITY;
	      iDepth++ ) {
		pData = __xuiMenuGetData(pScan);
		if ( pData == NULL ) return;
		if ( pData->onSelect != NULL ) {
			*pOnSelect = pData->onSelect;
			*ppUser = pData->pSelectUser;
			return;
		}
		pScan = pData->pParentMenu;
	}
}

static int __xuiMenuOpenSubmenu(xui_widget pWidget, xui_menu_data_t* pData, int iIndex)
{
	xui_menu_data_t* pSubData;
	xui_widget pSubmenu;
	xui_rect_t tItem;
	xui_rect_t tWorld;
	xui_widget pOwner;
	int iRet;

	if ( (pWidget == NULL) || (pData == NULL) || (iIndex < 0) || (iIndex >= pData->iItemCount) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( pData->arrItems[iIndex].iType != XUI_MENU_ITEM_SUBMENU ) {
		return XUI_OK;
	}
	pSubmenu = pData->arrItems[iIndex].pSubmenu;
	pSubData = __xuiMenuFindData(xuiWidgetGetContext(pWidget), pSubmenu);
	if ( (pSubData == NULL) ||
	     __xuiMenuSubmenuWouldCycle(pWidget, pSubmenu) ) return XUI_ERROR_INVALID_STATE;
	if ( pData->pOpenSubmenu == pSubmenu && xuiMenuIsOpen(pSubmenu) ) {
		return XUI_OK;
	}
	iRet = __xuiMenuCloseSubmenu(pWidget, pData);
	if ( iRet != XUI_OK ) return iRet;
	pSubData->pParentMenu = pWidget;
	pSubData->iParentItem = iIndex;
	pData->pOpenSubmenu = pSubmenu;
	pData->iDeferredHover = -1;
	pData->fDeferredHoverTime = 0.0f;
	pData->bAimActive = 0;
	tItem = pData->arrItemRect[iIndex];
	tWorld = xuiWidgetGetWorldRect(pWidget);
	pOwner = pWidget;
	iRet = xuiMenuOpenAt(pSubmenu, pOwner, tWorld.fX + tItem.fX + tItem.fW - 1.0f, tWorld.fY + tItem.fY);
	if ( iRet != XUI_OK ) {
		if ( __xuiMenuGetData(pWidget) == pData && pData->pOpenSubmenu == pSubmenu ) pData->pOpenSubmenu = NULL;
		pSubData = __xuiMenuGetData(pSubmenu);
		if ( pSubData != NULL && pSubData->pParentMenu == pWidget ) {
			pSubData->pParentMenu = NULL;
			pSubData->iParentItem = -1;
		}
		return iRet;
	}
	return XUI_OK;
}

static int __xuiMenuCloseSubmenu(xui_widget pWidget, xui_menu_data_t* pData)
{
	xui_widget pSubmenu;
	xui_menu_data_t* pSubData;
	int iRet;
	int iCloseRet;

	(void)pWidget;
	if ( pData == NULL ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pSubmenu = pData->pOpenSubmenu;
	pData->iPendingSubmenu = -1;
	pData->fPendingSubmenuTime = 0.0f;
	pData->iDeferredHover = -1;
	pData->fDeferredHoverTime = 0.0f;
	pData->bAimActive = 0;
	pData->pOpenSubmenu = NULL;
	pSubData = __xuiMenuGetData(pSubmenu);
	if ( pSubData == NULL ) return XUI_OK;
	pSubData->pParentMenu = NULL;
	pSubData->iParentItem = -1;
	pSubData->iHover = -1;
	iRet = __xuiMenuCloseSubmenu(pSubmenu, pSubData);
	pSubData = __xuiMenuGetData(pSubmenu);
	if ( pSubData == NULL ) return iRet;
	iCloseRet = (pSubData->pPopup != NULL) ? xuiPopupSetOpen(pSubData->pPopup, 0) : XUI_OK;
	if ( xuiInternalWidgetIsValid(pSubmenu) ) {
		xuiInternalAccessibilityQueue(pSubmenu, XUI_ACCESSIBLE_EVENT_STATE_CHANGED);
	}
	return (iRet != XUI_OK) ? iRet : iCloseRet;
}

static int __xuiMenuUpdate(xui_widget pWidget, float fDelta, void* pUser)
{
	xui_menu_data_t* pData;
	xui_context pContext;
	xui_widget pSubPopup;
	xui_rect_t tSubRect;
	int iIndex;
	int iRet;

	(void)pUser;
	if ( !isfinite(fDelta) || (fDelta < 0.0f) ) return XUI_ERROR_INVALID_ARGUMENT;
	pData = __xuiMenuGetData(pWidget);
	if ( pData == NULL ) return XUI_OK;
	if ( pData->iDeferredHover >= 0 ) {
		pContext = xuiWidgetGetContext(pWidget);
		pSubPopup = (__xuiMenuFindData(pContext, pData->pOpenSubmenu) != NULL) ?
			xuiMenuGetPopupWidget(pData->pOpenSubmenu) : NULL;
		if ( pSubPopup != NULL ) {
			tSubRect = xuiPopupGetPopupRect(pSubPopup);
			if ( pContext->fPointerX >= tSubRect.fX && pContext->fPointerX < tSubRect.fX + tSubRect.fW &&
			     pContext->fPointerY >= tSubRect.fY && pContext->fPointerY < tSubRect.fY + tSubRect.fH ) {
				pData->iDeferredHover = -1;
				pData->fDeferredHoverTime = 0.0f;
				pData->bAimActive = 0;
			} else {
				pData->fDeferredHoverTime += fDelta;
			}
		} else {
			pData->fDeferredHoverTime = XUI_MENU_SUBMENU_AIM_DELAY;
		}
		if ( pData->iDeferredHover >= 0 && pData->fDeferredHoverTime >= XUI_MENU_SUBMENU_AIM_DELAY ) {
			iIndex = pData->iDeferredHover;
			pData->iDeferredHover = -1;
			pData->fDeferredHoverTime = 0.0f;
			pData->bAimActive = 0;
			iRet = __xuiMenuApplyHoverTarget(pWidget, pData, iIndex);
			if ( iRet != XUI_OK ) return iRet;
		}
	}
	if ( (pData == NULL) || (pData->iPendingSubmenu < 0) ||
	     (pData->iPendingSubmenu >= pData->iItemCount) ||
	     (pData->iHover != pData->iPendingSubmenu) ||
	     (pData->arrItems[pData->iPendingSubmenu].iType != XUI_MENU_ITEM_SUBMENU) ) {
		if ( pData != NULL ) {
			pData->iPendingSubmenu = -1;
			pData->fPendingSubmenuTime = 0.0f;
		}
		return XUI_OK;
	}
	if ( fDelta > 0.0f ) {
		pData->fPendingSubmenuTime += fDelta;
	}
	if ( pData->fPendingSubmenuTime < XUI_MENU_SUBMENU_HOVER_DELAY ) {
		return XUI_OK;
	}
	iIndex = pData->iPendingSubmenu;
	pData->iPendingSubmenu = -1;
	pData->fPendingSubmenuTime = 0.0f;
	return __xuiMenuOpenSubmenu(pWidget, pData, iIndex);
}

static int __xuiMenuCommit(xui_widget pWidget, xui_menu_data_t* pData, int iIndex)
{
	xui_widget pRoot;
	xui_menu_select_proc onSelect;
	void* pUser;
	uint32_t arrOldState[XUI_MENU_ITEM_CAPACITY];
	int iValue;
	int iRet;
	int i;
	int bStateChanged = 0;

	if ( (pWidget == NULL) || (pData == NULL) || (iIndex < 0) || (iIndex >= pData->iItemCount) ||
	     !__xuiMenuItemEnabled(&pData->arrItems[iIndex]) ) {
		return XUI_OK;
	}
	if ( pData->arrItems[iIndex].iType == XUI_MENU_ITEM_SUBMENU ) {
		return __xuiMenuOpenSubmenu(pWidget, pData, iIndex);
	}
	for ( i = 0; i < pData->iItemCount; i++ ) arrOldState[i] = pData->arrItems[i].iState;
	iRet = __xuiMenuApplyCheckState(pData, iIndex);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	if ( iRet != XUI_OK ) {
		for ( i = 0; i < pData->iItemCount; i++ ) pData->arrItems[i].iState = arrOldState[i];
		return iRet;
	}
	for ( i = 0; i < pData->iItemCount; i++ ) {
		if ( pData->arrItems[i].iState != arrOldState[i] ) {
			bStateChanged = 1;
			break;
		}
	}
	if ( bStateChanged ) xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_STATE_CHANGED);
	if ( pData->iSelectCount < INT_MAX ) pData->iSelectCount++;
	__xuiMenuResolveSelect(pWidget, &onSelect, &pUser);
	iValue = pData->arrItems[iIndex].iValue;
	pRoot = __xuiMenuRoot(pWidget);
	iRet = xuiMenuClose(pRoot);
	if ( iRet != XUI_OK ) return iRet;
	if ( onSelect != NULL && xuiInternalWidgetIsValid(pWidget) &&
	     !xuiInternalContextDestroyPending(xuiWidgetGetContext(pWidget)) ) {
		onSelect(pWidget, iIndex, iValue, pUser);
	}
	return XUI_EVENT_DISPATCH_STOP;
}

static int __xuiMenuAsciiEqual(const char* sA, const char* sB, int iLen)
{
	int i;

	if ( (sA == NULL) || (sB == NULL) ) {
		return 0;
	}
	for ( i = 0; i < iLen; i++ ) {
		if ( tolower((unsigned char)sA[i]) != tolower((unsigned char)sB[i]) ) {
			return 0;
		}
	}
	return sB[iLen] == '\0';
}

enum {
	XUI_MENU_SHORTCUT_TOKEN_INVALID = 0,
	XUI_MENU_SHORTCUT_TOKEN_MODIFIER,
	XUI_MENU_SHORTCUT_TOKEN_KEY
};

static int __xuiMenuShortcutToken(const char* sToken, int iLen, int* pKey, uint32_t* pModifiers)
{
	int iValue;

	if ( (sToken == NULL) || (iLen <= 0) || (pKey == NULL) || (pModifiers == NULL) ) {
		return 0;
	}
	if ( __xuiMenuAsciiEqual(sToken, "ctrl", iLen) || __xuiMenuAsciiEqual(sToken, "control", iLen) ) {
		*pModifiers |= XUI_MOD_CTRL;
		return XUI_MENU_SHORTCUT_TOKEN_MODIFIER;
	}
	if ( __xuiMenuAsciiEqual(sToken, "alt", iLen) || __xuiMenuAsciiEqual(sToken, "option", iLen) ) {
		*pModifiers |= XUI_MOD_ALT;
		return XUI_MENU_SHORTCUT_TOKEN_MODIFIER;
	}
	if ( __xuiMenuAsciiEqual(sToken, "shift", iLen) ) {
		*pModifiers |= XUI_MOD_SHIFT;
		return XUI_MENU_SHORTCUT_TOKEN_MODIFIER;
	}
	if ( __xuiMenuAsciiEqual(sToken, "super", iLen) || __xuiMenuAsciiEqual(sToken, "win", iLen) || __xuiMenuAsciiEqual(sToken, "cmd", iLen) ) {
		*pModifiers |= XUI_MOD_SUPER;
		return XUI_MENU_SHORTCUT_TOKEN_MODIFIER;
	}
	if ( iLen == 1 ) {
		*pKey = toupper((unsigned char)sToken[0]);
		return XUI_MENU_SHORTCUT_TOKEN_KEY;
	}
	if ( __xuiMenuAsciiEqual(sToken, "space", iLen) ) { *pKey = XUI_KEY_SPACE; return XUI_MENU_SHORTCUT_TOKEN_KEY; }
	if ( __xuiMenuAsciiEqual(sToken, "tab", iLen) ) { *pKey = XUI_KEY_TAB; return XUI_MENU_SHORTCUT_TOKEN_KEY; }
	if ( __xuiMenuAsciiEqual(sToken, "enter", iLen) || __xuiMenuAsciiEqual(sToken, "return", iLen) ) { *pKey = XUI_KEY_ENTER; return XUI_MENU_SHORTCUT_TOKEN_KEY; }
	if ( __xuiMenuAsciiEqual(sToken, "esc", iLen) || __xuiMenuAsciiEqual(sToken, "escape", iLen) ) { *pKey = XUI_KEY_ESCAPE; return XUI_MENU_SHORTCUT_TOKEN_KEY; }
	if ( __xuiMenuAsciiEqual(sToken, "left", iLen) ) { *pKey = XUI_KEY_LEFT; return XUI_MENU_SHORTCUT_TOKEN_KEY; }
	if ( __xuiMenuAsciiEqual(sToken, "right", iLen) ) { *pKey = XUI_KEY_RIGHT; return XUI_MENU_SHORTCUT_TOKEN_KEY; }
	if ( __xuiMenuAsciiEqual(sToken, "up", iLen) ) { *pKey = XUI_KEY_UP; return XUI_MENU_SHORTCUT_TOKEN_KEY; }
	if ( __xuiMenuAsciiEqual(sToken, "down", iLen) ) { *pKey = XUI_KEY_DOWN; return XUI_MENU_SHORTCUT_TOKEN_KEY; }
	if ( __xuiMenuAsciiEqual(sToken, "home", iLen) ) { *pKey = XUI_KEY_HOME; return XUI_MENU_SHORTCUT_TOKEN_KEY; }
	if ( __xuiMenuAsciiEqual(sToken, "end", iLen) ) { *pKey = XUI_KEY_END; return XUI_MENU_SHORTCUT_TOKEN_KEY; }
	if ( __xuiMenuAsciiEqual(sToken, "pageup", iLen) || __xuiMenuAsciiEqual(sToken, "pgup", iLen) ) { *pKey = XUI_KEY_PAGE_UP; return XUI_MENU_SHORTCUT_TOKEN_KEY; }
	if ( __xuiMenuAsciiEqual(sToken, "pagedown", iLen) || __xuiMenuAsciiEqual(sToken, "pgdn", iLen) ) { *pKey = XUI_KEY_PAGE_DOWN; return XUI_MENU_SHORTCUT_TOKEN_KEY; }
	if ( __xuiMenuAsciiEqual(sToken, "delete", iLen) || __xuiMenuAsciiEqual(sToken, "del", iLen) ) { *pKey = XUI_KEY_DELETE; return XUI_MENU_SHORTCUT_TOKEN_KEY; }
	if ( __xuiMenuAsciiEqual(sToken, "backspace", iLen) || __xuiMenuAsciiEqual(sToken, "bksp", iLen) ) { *pKey = XUI_KEY_BACKSPACE; return XUI_MENU_SHORTCUT_TOKEN_KEY; }
	if ( (iLen == 2 || iLen == 3) && (tolower((unsigned char)sToken[0]) == 'f') &&
	     isdigit((unsigned char)sToken[1]) && (iLen == 2 || isdigit((unsigned char)sToken[2])) ) {
		iValue = sToken[1] - '0';
		if ( iLen == 3 ) iValue = iValue * 10 + (sToken[2] - '0');
		if ( (iValue >= 1) && (iValue <= 24) ) {
			*pKey = XUI_KEY_F1 + iValue - 1;
			return XUI_MENU_SHORTCUT_TOKEN_KEY;
		}
	}
	return XUI_MENU_SHORTCUT_TOKEN_INVALID;
}

static int __xuiMenuShortcutParse(const char* sShortcut, int* pKey, uint32_t* pModifiers)
{
	const char* sStart;
	const char* sEnd;
	int iKey;
	int iTokenType;
	int bHasKey;
	uint32_t iModifiers;

	if ( (sShortcut == NULL) || (pKey == NULL) || (pModifiers == NULL) ) {
		return 0;
	}
	iKey = 0;
	bHasKey = 0;
	iModifiers = 0;
	sStart = sShortcut;
	while ( *sStart != '\0' ) {
		while ( (*sStart == ' ') || (*sStart == '\t') || (*sStart == '+') ) sStart++;
		if ( *sStart == '\0' ) break;
		sEnd = sStart;
		while ( (*sEnd != '\0') && (*sEnd != '+') ) sEnd++;
		while ( (sEnd > sStart) && ((sEnd[-1] == ' ') || (sEnd[-1] == '\t')) ) sEnd--;
		iTokenType = __xuiMenuShortcutToken(sStart, (int)(sEnd - sStart), &iKey, &iModifiers);
		if ( iTokenType == XUI_MENU_SHORTCUT_TOKEN_INVALID ||
		     (iTokenType == XUI_MENU_SHORTCUT_TOKEN_KEY && bHasKey) ) {
			return 0;
		}
		if ( iTokenType == XUI_MENU_SHORTCUT_TOKEN_KEY ) bHasKey = 1;
		sStart = sEnd;
	}
	if ( !bHasKey || iKey == 0 ) {
		return 0;
	}
	*pKey = iKey;
	*pModifiers = iModifiers;
	return 1;
}

static int __xuiMenuShortcutMatch(const char* sShortcut, int iKey, uint32_t iModifiers)
{
	int iShortcutKey;
	uint32_t iShortcutModifiers;

	if ( !__xuiMenuShortcutParse(sShortcut, &iShortcutKey, &iShortcutModifiers) ) {
		return 0;
	}
	if ( (iShortcutKey >= 'A') && (iShortcutKey <= 'Z') && (iKey >= 'a') && (iKey <= 'z') ) {
		iKey = toupper((unsigned char)iKey);
	}
	return (iShortcutKey == iKey) && (iShortcutModifiers == iModifiers);
}

static int __xuiMenuCommitShortcut(xui_widget pWidget, xui_menu_data_t* pData, int iKey, uint32_t iModifiers)
{
	int i;

	if ( (pWidget == NULL) || (pData == NULL) ) {
		return XUI_OK;
	}
	for ( i = 0; i < pData->iItemCount; i++ ) {
		if ( __xuiMenuItemEnabled(&pData->arrItems[i]) &&
		     __xuiMenuShortcutMatch(pData->arrItems[i].sShortcut, iKey, iModifiers) ) {
			return __xuiMenuCommit(pWidget, pData, i);
		}
	}
	return XUI_OK;
}

static int __xuiMenuEvent(xui_widget pWidget, const xui_event_t* pEvent, void* pUser)
{
	xui_menu_data_t* pData;
	xui_rect_t tWorld;
	float fLocalX;
	float fLocalY;
	int iIndex;
	int iNext;
	int iRet;

	(void)pUser;
	if ( (pWidget == NULL) || (pEvent == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( pEvent->iPhase == XUI_EVENT_PHASE_CAPTURE ) {
		return XUI_OK;
	}
	pData = __xuiMenuGetData(pWidget);
	if ( pData == NULL ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	switch ( pEvent->iType ) {
	case XUI_EVENT_POINTER_MOVE:
		if ( pData->tPointerOwner.bActive &&
		     !xuiInternalPointerOwnerMatches(&pData->tPointerOwner, pEvent) ) {
			return XUI_OK;
		}
		iRet = __xuiMenuMeasureData(pWidget, pData);
		if ( iRet != XUI_OK ) return iRet;
		tWorld = xuiWidgetGetWorldRect(pWidget);
		fLocalX = pEvent->fX - tWorld.fX;
		fLocalY = pEvent->fY - tWorld.fY;
		iIndex = __xuiMenuIndexAt(pData, fLocalX, fLocalY);
		if ( __xuiMenuShouldDeferHover(pWidget, pData, pEvent->fX, pEvent->fY, iIndex) ) {
			if ( pData->iDeferredHover != iIndex ) {
				pData->iDeferredHover = iIndex;
				pData->fDeferredHoverTime = 0.0f;
			}
		} else {
			pData->iDeferredHover = -1;
			pData->fDeferredHoverTime = 0.0f;
			pData->bAimActive = 0;
			iRet = __xuiMenuApplyHoverTarget(pWidget, pData, iIndex);
			if ( iRet != XUI_OK ) return iRet;
		}
		pData->fLastPointerX = pEvent->fX;
		pData->fLastPointerY = pEvent->fY;
		pData->bLastPointer = 1;
		return XUI_EVENT_DISPATCH_STOP;
	case XUI_EVENT_POINTER_DOWN:
		if ( pEvent->iButton != 0 && pEvent->iButton != XUI_POINTER_BUTTON_LEFT ) {
			return XUI_OK;
		}
		if ( pData->tPointerOwner.bActive ) {
			return xuiInternalPointerOwnerMatches(&pData->tPointerOwner, pEvent) ?
				XUI_EVENT_DISPATCH_STOP : XUI_OK;
		}
		iRet = __xuiMenuMeasureData(pWidget, pData);
		if ( iRet != XUI_OK ) return iRet;
		tWorld = xuiWidgetGetWorldRect(pWidget);
		fLocalX = pEvent->fX - tWorld.fX;
		fLocalY = pEvent->fY - tWorld.fY;
		iIndex = __xuiMenuIndexAt(pData, fLocalX, fLocalY);
		iRet = __xuiMenuSetHover(pWidget, pData, iIndex);
		if ( iRet != XUI_OK ) return iRet;
		iRet = xuiInternalPointerOwnerBegin(xuiWidgetGetContext(pWidget), pWidget,
			pEvent, &pData->tPointerOwner);
		if ( iRet != XUI_OK ) return iRet;
		pData->iPressed = iIndex;
		return XUI_EVENT_DISPATCH_STOP;
	case XUI_EVENT_POINTER_UP:
		if ( pEvent->iButton != 0 && pEvent->iButton != XUI_POINTER_BUTTON_LEFT ) {
			return XUI_OK;
		}
		if ( !xuiInternalPointerOwnerMatches(&pData->tPointerOwner, pEvent) ) {
			return XUI_OK;
		}
		iRet = __xuiMenuMeasureData(pWidget, pData);
		if ( iRet != XUI_OK ) return iRet;
		tWorld = xuiWidgetGetWorldRect(pWidget);
		fLocalX = pEvent->fX - tWorld.fX;
		fLocalY = pEvent->fY - tWorld.fY;
		iIndex = __xuiMenuIndexAt(pData, fLocalX, fLocalY);
		iNext = pData->iPressed;
		pData->iPressed = -1;
		iRet = xuiInternalPointerOwnerEnd(xuiWidgetGetContext(pWidget), pWidget,
			pEvent, &pData->tPointerOwner);
		if ( iRet != XUI_OK ) return iRet;
		if ( __xuiMenuGetData(pWidget) != pData ) return XUI_EVENT_DISPATCH_STOP;
		if ( iNext >= 0 && iIndex == iNext ) {
			return __xuiMenuCommit(pWidget, pData, iIndex);
		}
		return XUI_EVENT_DISPATCH_STOP;
	case XUI_EVENT_POINTER_CAPTURE_LOST:
		if ( !xuiInternalPointerOwnerCaptureLost(&pData->tPointerOwner, pEvent) ) {
			return XUI_OK;
		}
		pData->iPressed = -1;
		return XUI_EVENT_DISPATCH_STOP;
	case XUI_EVENT_KEY_DOWN:
		iRet = __xuiMenuCommitShortcut(pWidget, pData, pEvent->iKey, pEvent->iModifiers);
		if ( iRet != XUI_OK ) return iRet;
		if ( pEvent->iKey == XUI_KEY_DOWN ) {
			iNext = __xuiMenuNextEnabled(pData, pData->iHover, 1);
			iRet = __xuiMenuSetHover(pWidget, pData, iNext);
			if ( iRet != XUI_OK ) return iRet;
			return XUI_EVENT_DISPATCH_STOP;
		}
		if ( pEvent->iKey == XUI_KEY_UP ) {
			iNext = __xuiMenuNextEnabled(pData, pData->iHover, -1);
			iRet = __xuiMenuSetHover(pWidget, pData, iNext);
			if ( iRet != XUI_OK ) return iRet;
			return XUI_EVENT_DISPATCH_STOP;
		}
		if ( pEvent->iKey == XUI_KEY_HOME || pEvent->iKey == XUI_KEY_END ) {
			iNext = __xuiMenuNextEnabled(pData, (pEvent->iKey == XUI_KEY_HOME) ? -1 : 0,
				(pEvent->iKey == XUI_KEY_HOME) ? 1 : -1);
			iRet = __xuiMenuSetHover(pWidget, pData, iNext);
			if ( iRet != XUI_OK ) return iRet;
			return XUI_EVENT_DISPATCH_STOP;
		}
		if ( pEvent->iKey == XUI_KEY_RIGHT ) {
			if ( pData->iHover >= 0 && pData->arrItems[pData->iHover].iType == XUI_MENU_ITEM_SUBMENU ) {
				iRet = __xuiMenuOpenSubmenu(pWidget, pData, pData->iHover);
				if ( iRet != XUI_OK ) return iRet;
				return XUI_EVENT_DISPATCH_STOP;
			}
			return XUI_EVENT_DISPATCH_STOP;
		}
		if ( pEvent->iKey == XUI_KEY_LEFT ) {
			if ( pData->pParentMenu != NULL ) {
				xui_widget pParentMenu = pData->pParentMenu;
				xui_context pContext = xuiWidgetGetContext(pWidget);
				xui_menu_data_t* pParentData = __xuiMenuGetData(pParentMenu);
				if ( pParentData != NULL ) {
					pParentData->pOpenSubmenu = NULL;
				}
				iRet = xuiMenuClose(pWidget);
				if ( iRet != XUI_OK ) return iRet;
				if ( !xuiInternalContextDestroyPending(pContext) && xuiInternalWidgetIsValid(pParentMenu) ) {
					iRet = xuiSetFocusWidget(pContext, pParentMenu);
					if ( iRet != XUI_OK ) return iRet;
				}
			}
			return XUI_EVENT_DISPATCH_STOP;
		}
		if ( (pEvent->iKey == XUI_KEY_ENTER) || (pEvent->iKey == XUI_KEY_SPACE) ) {
			return __xuiMenuCommit(pWidget, pData, pData->iHover);
		}
		if ( pEvent->iKey == XUI_KEY_ESCAPE ) {
			iRet = xuiMenuClose(__xuiMenuRoot(pWidget));
			if ( iRet != XUI_OK ) return iRet;
			return XUI_EVENT_DISPATCH_STOP;
		}
		break;
	case XUI_EVENT_BLUR:
	case XUI_EVENT_FOCUS:
		return xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	default:
		break;
	}
	return XUI_OK;
}

static void __xuiMenuCancelAction(xui_widget pWidget, void* pUser)
{
	xui_context pContext;
	int iRet;

	(void)pUser;
	if ( !xuiInternalWidgetIsValid(pWidget) ) return;
	pContext = xuiWidgetGetContext(pWidget);
	iRet = xuiMenuClose(__xuiMenuRoot(pWidget));
	if ( iRet != XUI_OK && xuiInternalContextIsValid(pContext) ) {
		xuiInternalReportError(pContext, pWidget, iRet, XUI_ERROR_STAGE_INPUT, 1,
			"menu.cancel", "The menu could not close after its cancel action.");
	}
}

static void __xuiMenuPopupChanged(xui_widget pPopup, int bOpen, void* pUser)
{
	xui_widget pMenu;
	xui_menu_data_t* pData;
	xui_menu_data_t* pParentData;
	xui_widget pParentMenu;
	xui_context pContext;
	int iRet;

	(void)pPopup;
	pMenu = (xui_widget)pUser;
	pData = __xuiMenuGetData(pMenu);
	if ( pData == NULL ) {
		return;
	}
	if ( !bOpen ) {
		pContext = xuiWidgetGetContext(pMenu);
		pParentMenu = pData->pParentMenu;
		iRet = __xuiMenuCloseSubmenu(pMenu, pData);
		pData = __xuiMenuGetData(pMenu);
		if ( pData == NULL || xuiInternalContextDestroyPending(pContext) ) return;
		pData->iHover = -1;
		if ( pParentMenu != NULL ) {
			pParentData = __xuiMenuGetData(pParentMenu);
			if ( (pParentData != NULL) && (pParentData->pOpenSubmenu == pMenu) ) {
				pParentData->pOpenSubmenu = NULL;
			}
		}
		if ( iRet == XUI_OK ) iRet = xuiWidgetInvalidate(pMenu, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
		if ( iRet != XUI_OK ) {
			xuiInternalReportError(pContext, pMenu, iRet, XUI_ERROR_STAGE_INPUT, 1,
				"menu.close", "The menu could not finish closing its submenu chain.");
		}
	}
}

static void __xuiMenuDefaultLayout(xui_layout_t* pLayout)
{
	memset(pLayout, 0, sizeof(*pLayout));
	pLayout->iLayoutType = XUI_LAYOUT_MANUAL;
	pLayout->iWidthMode = XUI_SIZE_CONTENT;
	pLayout->iHeightMode = XUI_SIZE_CONTENT;
	pLayout->iFlowMode = XUI_FLOW_ABSOLUTE;
	pLayout->iDock = XUI_DOCK_FILL;
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

static void __xuiMenuDefaultCachePolicy(xui_cache_policy_t* pPolicy)
{
	memset(pPolicy, 0, sizeof(*pPolicy));
	pPolicy->iSize = sizeof(*pPolicy);
	pPolicy->iPolicy = XUI_CACHE_POLICY_SELF;
	pPolicy->iFlags = XUI_CACHE_CLEAR_ON_UPDATE | XUI_CACHE_UPDATE_ALL_STATES;
	pPolicy->iClearColor = XUI_COLOR_RGBA(0, 0, 0, 0);
}

static int __xuiMenuInitEvents(xui_widget pWidget)
{
	int iRet;

	iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_POINTER_MOVE, __xuiMenuEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_POINTER_DOWN, __xuiMenuEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_POINTER_UP, __xuiMenuEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_POINTER_CAPTURE_LOST, __xuiMenuEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_KEY_DOWN, __xuiMenuEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_FOCUS, __xuiMenuEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_BLUR, __xuiMenuEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetCancelAction(pWidget, __xuiMenuCancelAction, NULL);
	return iRet;
}

static int __xuiMenuAccessibleCount(xui_widget pWidget, void* pUser)
{
	xui_menu_data_t* pData = (xui_menu_data_t*)pUser;
	(void)pWidget;
	return (pData != NULL) ? pData->iItemCount + 1 : 0;
}

static int __xuiMenuAccessibleGet(xui_widget pWidget, int iIndex,
	xui_accessible_node_t* pNode, void* pUser)
{
	xui_menu_data_t* pData = (xui_menu_data_t*)pUser;
	xui_menu_item_t* pItem;
	xui_rect_t tWorld;
	xui_rect_t tItem;
	int iItem;

	if ( (pData == NULL) || (pNode == NULL) || (iIndex < 0) || (iIndex > pData->iItemCount) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pNode->iId = (uint64_t)iIndex + 1u;
	if ( iIndex == 0 ) {
		pNode->iRole = XUI_ACCESSIBLE_ROLE_MENU;
		pNode->sName = xuiWidgetGetAccessibleName(pWidget) != NULL ?
			xuiWidgetGetAccessibleName(pWidget) : "Menu";
		pNode->sDescription = xuiWidgetGetAccessibleDescription(pWidget);
		pNode->tBounds = xuiWidgetGetWorldRect(pWidget);
		pNode->iState |= xuiMenuIsOpen(pWidget) ? XUI_ACCESSIBLE_STATE_EXPANDED : XUI_ACCESSIBLE_STATE_COLLAPSED;
		pNode->iRowCount = pData->iItemCount;
		pNode->iColumnCount = 1;
		pNode->iActions = XUI_ACCESSIBLE_ACTION_MASK(XUI_ACCESSIBLE_ACTION_FOCUS);
		return XUI_OK;
	}
	iItem = iIndex - 1;
	pItem = &pData->arrItems[iItem];
	pNode->iParentId = 1;
	pNode->iRole = (pItem->iType == XUI_MENU_ITEM_SEPARATOR) ?
		XUI_ACCESSIBLE_ROLE_SEPARATOR : XUI_ACCESSIBLE_ROLE_MENU_ITEM;
	pNode->sName = __xuiMenuText(pItem->sText);
	pNode->sDescription = pItem->sShortcut;
	tWorld = xuiWidgetGetWorldRect(pWidget);
	tItem = pData->arrItemRect[iItem];
	pNode->tBounds = (xui_rect_t){tWorld.fX + tItem.fX, tWorld.fY + tItem.fY, tItem.fW, tItem.fH};
	if ( pItem->iType == XUI_MENU_ITEM_SEPARATOR ) return XUI_OK;
	pNode->iRow = iItem;
	pNode->iColumn = 0;
	pNode->iRowCount = 1;
	pNode->iColumnCount = 1;
	pNode->iState |= XUI_ACCESSIBLE_STATE_SELECTABLE;
	if ( !__xuiMenuItemEnabled(pItem) ) pNode->iState |= XUI_ACCESSIBLE_STATE_DISABLED;
	if ( iItem == pData->iHover ) pNode->iState |= XUI_ACCESSIBLE_STATE_SELECTED;
	if ( (pItem->iState & XUI_MENU_ITEM_CHECKED) != 0u ) pNode->iState |= XUI_ACCESSIBLE_STATE_CHECKED;
	if ( pItem->iType == XUI_MENU_ITEM_SUBMENU ) {
		pNode->iState |= (pData->pOpenSubmenu == pItem->pSubmenu && xuiMenuIsOpen(pItem->pSubmenu)) ?
			XUI_ACCESSIBLE_STATE_EXPANDED : XUI_ACCESSIBLE_STATE_COLLAPSED;
		pNode->iActions = (pNode->iState & XUI_ACCESSIBLE_STATE_EXPANDED) != 0u ?
			XUI_ACCESSIBLE_ACTION_MASK(XUI_ACCESSIBLE_ACTION_COLLAPSE) :
			XUI_ACCESSIBLE_ACTION_MASK(XUI_ACCESSIBLE_ACTION_EXPAND);
	} else {
		pNode->iActions = XUI_ACCESSIBLE_ACTION_MASK(XUI_ACCESSIBLE_ACTION_ACTIVATE);
		if ( pItem->iType == XUI_MENU_ITEM_CHECK || pItem->iType == XUI_MENU_ITEM_RADIO )
			pNode->iActions |= XUI_ACCESSIBLE_ACTION_MASK(XUI_ACCESSIBLE_ACTION_TOGGLE);
	}
	pNode->iActions |= XUI_ACCESSIBLE_ACTION_MASK(XUI_ACCESSIBLE_ACTION_FOCUS);
	return XUI_OK;
}

static int __xuiMenuAccessibleAction(xui_widget pWidget, uint64_t iNodeId,
	int iAction, const void* pPayload, void* pUser)
{
	xui_menu_data_t* pData = (xui_menu_data_t*)pUser;
	int iIndex;
	int iRet;

	(void)pPayload;
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( iNodeId == 0 || iNodeId == 1 ) {
		return (iAction == XUI_ACCESSIBLE_ACTION_FOCUS) ?
			xuiSetFocusWidget(xuiWidgetGetContext(pWidget), pWidget) : XUI_ERROR_UNSUPPORTED;
	}
	if ( iNodeId < 2 || iNodeId > (uint64_t)pData->iItemCount + 1u ) return XUI_ERROR_NOT_FOUND;
	iIndex = (int)iNodeId - 2;
	if ( !__xuiMenuItemEnabled(&pData->arrItems[iIndex]) ) return XUI_ERROR_INVALID_STATE;
	if ( iAction == XUI_ACCESSIBLE_ACTION_FOCUS ) {
		iRet = __xuiMenuSetHover(pWidget, pData, iIndex);
		if ( iRet != XUI_OK ) return iRet;
		return xuiSetFocusWidget(xuiWidgetGetContext(pWidget), pWidget);
	}
	if ( iAction == XUI_ACCESSIBLE_ACTION_COLLAPSE ) {
		if ( pData->arrItems[iIndex].iType != XUI_MENU_ITEM_SUBMENU ) return XUI_ERROR_UNSUPPORTED;
		if ( pData->pOpenSubmenu != pData->arrItems[iIndex].pSubmenu ||
		     !xuiMenuIsOpen(pData->arrItems[iIndex].pSubmenu) ) return XUI_ERROR_INVALID_STATE;
		return __xuiMenuCloseSubmenu(pWidget, pData);
	}
	if ( iAction == XUI_ACCESSIBLE_ACTION_EXPAND ) {
		if ( pData->arrItems[iIndex].iType != XUI_MENU_ITEM_SUBMENU ) return XUI_ERROR_UNSUPPORTED;
		iRet = __xuiMenuSetHover(pWidget, pData, iIndex);
		if ( iRet != XUI_OK ) return iRet;
		return __xuiMenuCommit(pWidget, pData, iIndex);
	}
	if ( iAction == XUI_ACCESSIBLE_ACTION_TOGGLE ) {
		if ( pData->arrItems[iIndex].iType != XUI_MENU_ITEM_CHECK &&
		     pData->arrItems[iIndex].iType != XUI_MENU_ITEM_RADIO ) return XUI_ERROR_UNSUPPORTED;
		iRet = __xuiMenuSetHover(pWidget, pData, iIndex);
		if ( iRet != XUI_OK ) return iRet;
		return __xuiMenuCommit(pWidget, pData, iIndex);
	}
	if ( iAction == XUI_ACCESSIBLE_ACTION_ACTIVATE ) {
		if ( pData->arrItems[iIndex].iType == XUI_MENU_ITEM_SUBMENU ) return XUI_ERROR_UNSUPPORTED;
		iRet = __xuiMenuSetHover(pWidget, pData, iIndex);
		if ( iRet != XUI_OK ) return iRet;
		return __xuiMenuCommit(pWidget, pData, iIndex);
	}
	return XUI_ERROR_UNSUPPORTED;
}

static int __xuiMenuInit(xui_widget pWidget, void* pTypeData, const void* pCreateData, void* pUser)
{
	xui_menu_data_t* pData;
	const xui_menu_desc_t* pDesc;
	xui_cache_policy_t tPolicy;
	int iRet;

	(void)pUser;
	if ( (pWidget == NULL) || (pTypeData == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pDesc = (const xui_menu_desc_t*)pCreateData;
	if ( !__xuiMenuDescValid(pDesc) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pData = (xui_menu_data_t*)pTypeData;
	__xuiMenuDefaultMetrics(&pData->tMetrics);
	__xuiMenuDefaultColors(&pData->tColors);
	pData->pFont = (pDesc != NULL && pDesc->pFont != NULL) ? pDesc->pFont : xuiGetDefaultFont(xuiWidgetGetContext(pWidget));
	pData->iHover = -1;
	pData->iPressed = -1;
	pData->iPendingSubmenu = -1;
	pData->iDeferredHover = -1;
	pData->iParentItem = -1;
	pData->bMeasureDirty = 1;
	if ( pDesc != NULL && pDesc->bHasMetrics && __xuiMenuMetricsValid(&pDesc->tMetrics) ) {
		pData->tMetrics = pDesc->tMetrics;
		pData->tMetrics.iSize = sizeof(pData->tMetrics);
	}
	if ( pDesc != NULL && pDesc->bHasColors && __xuiMenuColorsValid(&pDesc->tColors) ) {
		pData->tColors = pDesc->tColors;
		pData->tColors.iSize = sizeof(pData->tColors);
	}
	iRet = xuiWidgetSetLayoutType(pWidget, XUI_LAYOUT_MANUAL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetFlowMode(pWidget, XUI_FLOW_ABSOLUTE);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetOverflow(pWidget, XUI_OVERFLOW_VISIBLE);
	if ( iRet != XUI_OK ) return iRet;
	pWidget->onUpdate = __xuiMenuUpdate;
	iRet = xuiWidgetSetFocusable(pWidget, 1);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetTabStop(pWidget, 0);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetFocusScope(pWidget, 1);
	if ( iRet != XUI_OK ) return iRet;
	__xuiMenuDefaultCachePolicy(&tPolicy);
	iRet = xuiWidgetSetCachePolicy(pWidget, &tPolicy);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetCacheRenderCallback(pWidget, __xuiMenuCacheRender, NULL);
	if ( iRet == XUI_OK ) iRet = __xuiMenuInitEvents(pWidget);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetAccessibilityProvider(pWidget,
		__xuiMenuAccessibleCount, __xuiMenuAccessibleGet, __xuiMenuAccessibleAction, pData);
	if ( iRet != XUI_OK ) return iRet;
	iRet = __xuiMenuMeasureData(pWidget, pData);
	if ( iRet != XUI_OK ) return iRet;
	pData->pWidget = pWidget;
	pData->pNext = pWidget->pContext->pMenuList;
	pWidget->pContext->pMenuList = pData;
	return XUI_OK;
}

static void __xuiMenuDestroy(xui_widget pWidget, void* pTypeData, void* pUser)
{
	xui_menu_data_t* pData;
	xui_menu_data_t* pScan;
	xui_menu_data_t** ppScan;
	xui_context pContext;
	xui_widget pPopup;
	xui_menu_data_t* pParentData;
	int iOldHover;
	int bChanged;
	int bTreeChanged;
	int i;

	(void)pUser;
	pData = (xui_menu_data_t*)pTypeData;
	if ( pData != NULL ) {
		pContext = pWidget->pContext;
		(void)xuiInternalPointerOwnerCancel(pContext, pWidget,
			&pData->tPointerOwner);
		for ( ppScan = &pContext->pMenuList; *ppScan != NULL; ppScan = &(*ppScan)->pNext ) {
			if ( *ppScan == pData ) {
				*ppScan = pData->pNext;
				break;
			}
		}
		pData->pNext = NULL;
		if ( pContext->onMenuUnavailable != NULL ) {
			pContext->onMenuUnavailable(pContext, pWidget);
		}
		for ( pScan = pContext->pMenuList; pScan != NULL; pScan = pScan->pNext ) {
			bChanged = 0;
			bTreeChanged = 0;
			iOldHover = pScan->iHover;
			if ( pScan->pParentMenu == pWidget ) {
				pScan->pParentMenu = NULL;
				pScan->iParentItem = -1;
				bChanged = 1;
			}
			if ( pScan->pOpenSubmenu == pWidget ) {
				pScan->pOpenSubmenu = NULL;
				pScan->iPendingSubmenu = -1;
				pScan->fPendingSubmenuTime = 0.0f;
				pScan->iDeferredHover = -1;
				pScan->fDeferredHoverTime = 0.0f;
				pScan->bAimActive = 0;
				bChanged = 1;
			}
			for ( i = 0; i < pScan->iItemCount; i++ ) {
				if ( pScan->arrItems[i].pSubmenu != pWidget ) continue;
				pScan->arrItems[i].pSubmenu = NULL;
				pScan->arrItems[i].iState &= ~XUI_MENU_ITEM_ENABLED;
				bChanged = 1;
				bTreeChanged = 1;
			}
			if ( pScan->iHover >= 0 && pScan->iHover < pScan->iItemCount &&
			     !__xuiMenuItemEnabled(&pScan->arrItems[pScan->iHover]) ) {
				pScan->iHover = __xuiMenuNextEnabled(pScan, pScan->iHover, 1);
			}
			if ( bChanged && xuiInternalWidgetIsValid(pScan->pWidget) &&
			     !xuiInternalContextDestroyPending(pContext) ) {
				(void)xuiWidgetInvalidate(pScan->pWidget,
					XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
				if ( bTreeChanged ) xuiInternalAccessibilityQueue(pScan->pWidget,
					XUI_ACCESSIBLE_EVENT_TREE_CHANGED);
				xuiInternalAccessibilityQueue(pScan->pWidget,
					XUI_ACCESSIBLE_EVENT_STATE_CHANGED);
				if ( iOldHover != pScan->iHover ) xuiInternalAccessibilityQueue(pScan->pWidget,
					XUI_ACCESSIBLE_EVENT_SELECTION_CHANGED);
			}
		}
		pPopup = pData->pPopup;
		pData->pPopup = NULL;
		pParentData = __xuiMenuGetData(pData->pParentMenu);
		if ( pParentData != NULL && pParentData->pOpenSubmenu == pWidget ) {
			pParentData->pOpenSubmenu = NULL;
		}
		__xuiMenuFreeItemStrings(pData->arrItems, pData->iItemCount);
		if ( xuiInternalWidgetIsValid(pPopup) ) xuiWidgetDestroy(pPopup);
		memset(pData, 0, sizeof(*pData));
	}
}

static xui_menu_data_t* __xuiMenuGetData(xui_widget pWidget)
{
	xui_context pContext;
	xui_widget_type pType;

	if ( !xuiInternalWidgetIsValid(pWidget) ) {
		return NULL;
	}
	pContext = xuiWidgetGetContext(pWidget);
	if ( !xuiInternalContextIsValid(pContext) ) return NULL;
	pType = xuiWidgetFindType(pContext, "menu");
	if ( (pType == NULL) || !xuiWidgetIsType(pWidget, pType) ) {
		return NULL;
	}
	return (xui_menu_data_t*)xuiWidgetGetTypeData(pWidget);
}

static int __xuiMenuCreatePopup(xui_widget pWidget, xui_menu_data_t* pData, const xui_menu_desc_t* pDesc)
{
	xui_popup_desc_t tPopupDesc;
	xui_widget pPopup;
	xui_widget pContent;
	int iRet;

	if ( (pWidget == NULL) || (pData == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	memset(&tPopupDesc, 0, sizeof(tPopupDesc));
	tPopupDesc.iSize = sizeof(tPopupDesc);
	tPopupDesc.pOwner = (pDesc != NULL) ? pDesc->pOwner : NULL;
	tPopupDesc.fContentWidth = pData->fContentW;
	tPopupDesc.fContentHeight = pData->fContentH;
	tPopupDesc.fPadding = 3.0f;
	tPopupDesc.fBorderWidth = 1.0f;
	tPopupDesc.fShadowSize = 4.0f;
	tPopupDesc.fScrollbarSize = 8.0f;
	tPopupDesc.iAnchor = XUI_POPUP_ANCHOR_BOTTOM_LEFT;
	tPopupDesc.iDirection = XUI_POPUP_DIRECTION_RIGHT_DOWN;
	tPopupDesc.iOutsidePolicy = XUI_POPUP_OUTSIDE_CLOSE;
	tPopupDesc.iOwnerPolicy = XUI_POPUP_OWNER_CLOSE;
	tPopupDesc.iEscapePolicy = XUI_POPUP_ESCAPE_CLOSE;
	tPopupDesc.iFocusPolicy = XUI_POPUP_FOCUS_CUSTOM;
	tPopupDesc.bConsumeInside = 1;
	tPopupDesc.iScrollbarMode = XUI_SCROLLBAR_MODE_COMPACT;
	tPopupDesc.iPanelColor = pData->tColors.iPanelColor;
	tPopupDesc.iBorderColor = pData->tColors.iBorderColor;
	tPopupDesc.iShadowColor = pData->tColors.iShadowColor;
	tPopupDesc.iBackdropColor = XUI_COLOR_RGBA(0, 0, 0, 0);
	pPopup = NULL;
	iRet = xuiPopupCreate(xuiWidgetGetContext(pWidget), &pPopup, &tPopupDesc);
	if ( iRet != XUI_OK ) return iRet;
	pData->pPopup = pPopup;
	iRet = xuiPopupSetChange(pPopup, __xuiMenuPopupChanged, pWidget);
	if ( iRet == XUI_OK ) iRet = xuiPopupSetFocusPolicy(pPopup, XUI_POPUP_FOCUS_CUSTOM, pWidget);
	if ( iRet == XUI_OK ) {
		pContent = xuiPopupGetContentWidget(pPopup);
		if ( pContent == NULL ) {
			iRet = XUI_ERROR_NOT_INITIALIZED;
		} else {
			iRet = xuiWidgetAddChild(pContent, pWidget);
		}
	}
	if ( iRet == XUI_OK ) {
		iRet = __xuiMenuApplyPopupSize(pWidget, pData);
	}
	if ( iRet != XUI_OK ) {
		(void)xuiWidgetRemoveFromParent(pWidget);
		xuiWidgetDestroy(pData->pPopup);
		pData->pPopup = NULL;
		return iRet;
	}
	return XUI_OK;
}

static int __xuiMenuApplyPopupSize(xui_widget pWidget, xui_menu_data_t* pData)
{
	int iRet;
	xui_menu_colors_t tColors;

	if ( (pWidget == NULL) || (pData == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	iRet = __xuiMenuMeasureData(pWidget, pData);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiWidgetSetRect(pWidget, (xui_rect_t){0.0f, 0.0f, pData->fContentW, pData->fContentH});
	if ( iRet != XUI_OK ) return iRet;
	if ( pData->pPopup != NULL ) {
		iRet = xuiInternalPopupSetSizing(pData->pPopup,
			pData->fContentW, pData->fContentH, 0.0f, pData->tMetrics.fMaxHeight);
		if ( iRet != XUI_OK ) return iRet;
		__xuiMenuResolveColors(pWidget, pData, &tColors);
		iRet = __xuiMenuApplyPopupColors(pData, &tColors);
		if ( iRet != XUI_OK ) return iRet;
	}
	return xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
}

XUI_API xui_widget_type xuiMenuGetType(xui_context pContext)
{
	xui_widget_type pType;
	xui_widget_type_desc_t tDesc;
	xui_layout_t tLayout;
	xui_cache_policy_t tPolicy;
	int iRet;

	if ( !xuiInternalContextIsValid(pContext) ) {
		return NULL;
	}
	pType = xuiWidgetFindType(pContext, "menu");
	if ( pType != NULL ) {
		return pType;
	}
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.sName = "menu";
	tDesc.pParent = xuiWidgetGetBaseType();
	tDesc.iFlags = XUI_WIDGET_TYPE_DEFAULT_LAYOUT | XUI_WIDGET_TYPE_DEFAULT_CACHE_POLICY;
	tDesc.iTypeDataSize = sizeof(xui_menu_data_t);
	tDesc.onInit = __xuiMenuInit;
	tDesc.onDestroy = __xuiMenuDestroy;
	tDesc.onCacheRender = __xuiMenuCacheRender;
	__xuiMenuDefaultLayout(&tLayout);
	__xuiMenuDefaultCachePolicy(&tPolicy);
	tDesc.tLayout = tLayout;
	tDesc.tCachePolicy = tPolicy;
	iRet = xuiWidgetRegisterType(pContext, &pType, &tDesc);
	if ( iRet != XUI_OK ) {
		return NULL;
	}
	__xuiMenuRegisterColors(pContext, pType);
	pType->onPreparePaint = __xuiMenuPreparePaint;
	return pType;
}

XUI_API int xuiMenuCreate(xui_context pContext, xui_widget* ppWidget, const xui_menu_desc_t* pDesc)
{
	xui_widget_type pType;
	xui_menu_data_t* pData;
	int iRet;

	if ( ppWidget != NULL ) *ppWidget = NULL;
	if ( !xuiInternalContextIsValid(pContext) || (ppWidget == NULL) || !__xuiMenuDescValid(pDesc) ||
	     (pDesc != NULL && pDesc->pOwner != NULL &&
	      (!xuiInternalWidgetIsValid(pDesc->pOwner) || xuiWidgetGetContext(pDesc->pOwner) != pContext)) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pType = xuiMenuGetType(pContext);
	if ( pType == NULL ) {
		return XUI_ERROR_NOT_INITIALIZED;
	}
	iRet = xuiWidgetCreateTyped(pContext, pType, ppWidget, pDesc);
	if ( iRet != XUI_OK ) {
		return iRet;
	}
	pData = __xuiMenuGetData(*ppWidget);
	iRet = __xuiMenuCreatePopup(*ppWidget, pData, pDesc);
	if ( iRet != XUI_OK ) {
		xuiWidgetDestroy(*ppWidget);
		*ppWidget = NULL;
		return iRet;
	}
	return XUI_OK;
}

XUI_API int xuiMenuSetItems(xui_widget pWidget, const xui_menu_item_t* pItems, int iCount)
{
	xui_menu_data_t* pData = __xuiMenuGetData(pWidget);
	xui_menu_item_t arrNew[XUI_MENU_ITEM_CAPACITY];
	xui_menu_item_t arrOld[XUI_MENU_ITEM_CAPACITY];
	int iOldCount;
	int iOldHover;
	int iRet;
	int i;

	if ( (pData == NULL) || (iCount < 0) || (iCount > XUI_MENU_ITEM_CAPACITY) || ((iCount > 0) && (pItems == NULL)) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	for ( i = 0; i < iCount; i++ ) {
		if ( !__xuiMenuItemValid(pWidget, &pItems[i]) ) {
			return XUI_ERROR_INVALID_ARGUMENT;
		}
	}
	if ( __xuiMenuItemsEqual(pData, pItems, iCount) ) return XUI_OK;
	iRet = __xuiMenuCopyItems(arrNew, pItems, iCount);
	if ( iRet != XUI_OK ) return iRet;
	iRet = __xuiMenuCancelPress(pWidget, pData);
	if ( iRet != XUI_OK ) {
		__xuiMenuFreeItemStrings(arrNew, iCount);
		return iRet;
	}
	if ( __xuiMenuGetData(pWidget) != pData ) {
		__xuiMenuFreeItemStrings(arrNew, iCount);
		return XUI_OK;
	}
	/* Old item metadata may own an open child popup. Close it before replacing
	 * the item array so it cannot outlive its parent menu selection. */
	iRet = __xuiMenuCloseSubmenu(pWidget, pData);
	if ( iRet != XUI_OK ) {
		__xuiMenuFreeItemStrings(arrNew, iCount);
		return iRet;
	}
	memcpy(arrOld, pData->arrItems, sizeof(arrOld));
	iOldCount = pData->iItemCount;
	iOldHover = pData->iHover;
	memcpy(pData->arrItems, arrNew, sizeof(arrNew));
	pData->iItemCount = iCount;
	pData->iPressed = -1;
	pData->iHover = __xuiMenuNextEnabled(pData, -1, 1);
	pData->bMeasureDirty = 1;
	iRet = __xuiMenuApplyPopupSize(pWidget, pData);
	if ( iRet != XUI_OK ) {
		memcpy(pData->arrItems, arrOld, sizeof(arrOld));
		pData->iItemCount = iOldCount;
		pData->iHover = iOldHover;
		pData->bMeasureDirty = 1;
		__xuiMenuFreeItemStrings(arrNew, iCount);
		(void)__xuiMenuApplyPopupSize(pWidget, pData);
		return iRet;
	}
	__xuiMenuFreeItemStrings(arrOld, iOldCount);
	xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_TREE_CHANGED);
	xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_SELECTION_CHANGED);
	return XUI_OK;
}

XUI_API int xuiMenuAddItem(xui_widget pWidget, const xui_menu_item_t* pItem)
{
	xui_menu_data_t* pData = __xuiMenuGetData(pWidget);
	xui_menu_item_t arrNew[XUI_MENU_ITEM_CAPACITY];
	int iOldHover;
	int iRet;

	if ( (pData == NULL) || (pItem == NULL) || (pData->iItemCount >= XUI_MENU_ITEM_CAPACITY) ||
	     !__xuiMenuItemValid(pWidget, pItem) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	iRet = __xuiMenuCopyItems(arrNew, pItem, 1);
	if ( iRet != XUI_OK ) return iRet;
	iRet = __xuiMenuCancelPress(pWidget, pData);
	if ( iRet != XUI_OK ) {
		__xuiMenuFreeItemStrings(arrNew, 1);
		return iRet;
	}
	if ( __xuiMenuGetData(pWidget) != pData ) {
		__xuiMenuFreeItemStrings(arrNew, 1);
		return XUI_OK;
	}
	iRet = __xuiMenuCloseSubmenu(pWidget, pData);
	if ( iRet != XUI_OK ) {
		__xuiMenuFreeItemStrings(arrNew, 1);
		return iRet;
	}
	if ( __xuiMenuGetData(pWidget) != pData ) {
		__xuiMenuFreeItemStrings(arrNew, 1);
		return XUI_OK;
	}
	iOldHover = pData->iHover;
	pData->arrItems[pData->iItemCount++] = arrNew[0];
	pData->bMeasureDirty = 1;
	if ( pData->iHover < 0 ) {
		pData->iHover = __xuiMenuNextEnabled(pData, -1, 1);
	}
	iRet = __xuiMenuApplyPopupSize(pWidget, pData);
	if ( iRet != XUI_OK ) {
		pData->iItemCount--;
		pData->iHover = iOldHover;
		pData->bMeasureDirty = 1;
		__xuiMenuFreeItemStrings(&pData->arrItems[pData->iItemCount], 1);
		memset(&pData->arrItems[pData->iItemCount], 0, sizeof(pData->arrItems[0]));
		(void)__xuiMenuApplyPopupSize(pWidget, pData);
	}
	if ( iRet == XUI_OK ) xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_TREE_CHANGED);
	return iRet;
}

XUI_API int xuiMenuAddSeparator(xui_widget pWidget)
{
	xui_menu_item_t tItem;

	memset(&tItem, 0, sizeof(tItem));
	tItem.iType = XUI_MENU_ITEM_SEPARATOR;
	return xuiMenuAddItem(pWidget, &tItem);
}

XUI_API int xuiMenuClear(xui_widget pWidget)
{
	xui_menu_data_t* pData = __xuiMenuGetData(pWidget);
	xui_menu_item_t arrOld[XUI_MENU_ITEM_CAPACITY];
	int iOldCount;
	int iOldHover;
	int iRet;

	if ( pData == NULL ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	iRet = __xuiMenuCancelPress(pWidget, pData);
	if ( iRet != XUI_OK ) return iRet;
	if ( __xuiMenuGetData(pWidget) != pData ) return XUI_OK;
	iRet = __xuiMenuCloseSubmenu(pWidget, pData);
	if ( iRet != XUI_OK ) return iRet;
	memcpy(arrOld, pData->arrItems, sizeof(arrOld));
	iOldCount = pData->iItemCount;
	iOldHover = pData->iHover;
	memset(pData->arrItems, 0, sizeof(pData->arrItems));
	memset(pData->arrItemRect, 0, sizeof(pData->arrItemRect));
	pData->iItemCount = 0;
	pData->iHover = -1;
	pData->iPressed = -1;
	pData->bMeasureDirty = 1;
	iRet = __xuiMenuApplyPopupSize(pWidget, pData);
	if ( iRet != XUI_OK ) {
		memcpy(pData->arrItems, arrOld, sizeof(arrOld));
		pData->iItemCount = iOldCount;
		pData->iHover = iOldHover;
		pData->bMeasureDirty = 1;
		(void)__xuiMenuApplyPopupSize(pWidget, pData);
		return iRet;
	}
	__xuiMenuFreeItemStrings(arrOld, iOldCount);
	xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_TREE_CHANGED);
	if ( iOldHover >= 0 ) xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_SELECTION_CHANGED);
	return XUI_OK;
}

XUI_API int xuiMenuGetItemCount(xui_widget pWidget)
{
	xui_menu_data_t* pData = __xuiMenuGetData(pWidget);
	return (pData != NULL) ? pData->iItemCount : 0;
}

XUI_API const xui_menu_item_t* xuiMenuGetItem(xui_widget pWidget, int iIndex)
{
	xui_menu_data_t* pData = __xuiMenuGetData(pWidget);
	if ( (pData == NULL) || (iIndex < 0) || (iIndex >= pData->iItemCount) ) {
		return NULL;
	}
	return &pData->arrItems[iIndex];
}

XUI_API xui_rect_t xuiMenuGetItemRect(xui_widget pWidget, int iIndex)
{
	xui_menu_data_t* pData = __xuiMenuGetData(pWidget);
	xui_rect_t tEmpty;

	memset(&tEmpty, 0, sizeof(tEmpty));
	if ( (pData == NULL) || (iIndex < 0) || (iIndex >= pData->iItemCount) ) {
		return tEmpty;
	}
	(void)__xuiMenuMeasureData(pWidget, pData);
	return pData->arrItemRect[iIndex];
}

XUI_API int xuiMenuSetItemState(xui_widget pWidget, int iIndex, uint32_t iMask, uint32_t iState)
{
	xui_menu_data_t* pData = __xuiMenuGetData(pWidget);
	uint32_t iOld;
	uint32_t iNew;
	int iOldHover;
	int iRet;

	if ( (pData == NULL) || (iIndex < 0) || (iIndex >= pData->iItemCount) ||
	     ((iMask & ~XUI_MENU_ITEM_STATE_MASK) != 0u) || ((iState & ~iMask) != 0u) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	iOld = pData->arrItems[iIndex].iState;
	iNew = (iOld & ~iMask) | (iState & iMask);
	if ( iNew == iOld ) return XUI_OK;
	if ( iIndex == pData->iPressed && (iNew & XUI_MENU_ITEM_ENABLED) == 0u ) {
		iRet = __xuiMenuCancelPress(pWidget, pData);
		if ( iRet != XUI_OK ) return iRet;
		if ( __xuiMenuGetData(pWidget) != pData ) return XUI_OK;
	}
	iOldHover = pData->iHover;
	pData->arrItems[iIndex].iState = iNew;
	if ( iIndex == pData->iHover && !__xuiMenuItemEnabled(&pData->arrItems[iIndex]) ) {
		iRet = __xuiMenuCloseSubmenu(pWidget, pData);
		if ( iRet != XUI_OK ) {
			pData->arrItems[iIndex].iState = iOld;
			return iRet;
		}
		pData->iHover = __xuiMenuNextEnabled(pData, iIndex, 1);
	}
	iRet = xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	if ( iRet != XUI_OK ) {
		pData->arrItems[iIndex].iState = iOld;
		pData->iHover = iOldHover;
	}
	if ( iRet == XUI_OK ) {
		xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_STATE_CHANGED);
		if ( pData->iHover != iOldHover ) xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_SELECTION_CHANGED);
	}
	return iRet;
}

XUI_API uint32_t xuiMenuGetItemState(xui_widget pWidget, int iIndex)
{
	xui_menu_data_t* pData = __xuiMenuGetData(pWidget);
	if ( (pData == NULL) || (iIndex < 0) || (iIndex >= pData->iItemCount) ) {
		return 0u;
	}
	return pData->arrItems[iIndex].iState;
}

XUI_API int xuiMenuGetHoverIndex(xui_widget pWidget)
{
	xui_menu_data_t* pData = __xuiMenuGetData(pWidget);
	return (pData != NULL) ? pData->iHover : -1;
}

XUI_API int xuiMenuSetHoverIndex(xui_widget pWidget, int iIndex)
{
	xui_menu_data_t* pData = __xuiMenuGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	return __xuiMenuSetHover(pWidget, pData, iIndex);
}

XUI_API int xuiMenuCommitHover(xui_widget pWidget)
{
	xui_menu_data_t* pData = __xuiMenuGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	return __xuiMenuCommit(pWidget, pData, pData->iHover);
}

XUI_API int xuiMenuSetSelect(xui_widget pWidget, xui_menu_select_proc onSelect, void* pUser)
{
	xui_menu_data_t* pData = __xuiMenuGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	pData->onSelect = onSelect;
	pData->pSelectUser = pUser;
	return XUI_OK;
}

/* Private bridge used by MenuBar to restore a Menu callback after a temporary binding. */
int xuiInternalMenuGetSelectBinding(xui_widget pWidget, xui_menu_select_proc* pOnSelect, void** ppUser)
{
	xui_menu_data_t* pData = __xuiMenuGetData(pWidget);

	if ( pData == NULL || pOnSelect == NULL || ppUser == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	*pOnSelect = pData->onSelect;
	*ppUser = pData->pSelectUser;
	return XUI_OK;
}

XUI_API int xuiMenuGetSelectCount(xui_widget pWidget)
{
	xui_menu_data_t* pData = __xuiMenuGetData(pWidget);
	return (pData != NULL) ? pData->iSelectCount : 0;
}

XUI_API int xuiMenuSetFont(xui_widget pWidget, xui_font pFont)
{
	xui_menu_data_t* pData = __xuiMenuGetData(pWidget);
	xui_font pOldFont;
	int iRet;
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pData->pFont == pFont ) return XUI_OK;
	pOldFont = pData->pFont;
	pData->pFont = pFont;
	pData->bMeasureDirty = 1;
	iRet = __xuiMenuApplyPopupSize(pWidget, pData);
	if ( iRet != XUI_OK ) {
		pData->pFont = pOldFont;
		pData->bMeasureDirty = 1;
		(void)__xuiMenuApplyPopupSize(pWidget, pData);
	}
	return iRet;
}

XUI_API xui_font xuiMenuGetFont(xui_widget pWidget)
{
	xui_menu_data_t* pData = __xuiMenuGetData(pWidget);
	return (pData != NULL) ? pData->pFont : NULL;
}

XUI_API int xuiMenuSetMetrics(xui_widget pWidget, const xui_menu_metrics_t* pMetrics)
{
	xui_menu_data_t* pData = __xuiMenuGetData(pWidget);
	xui_menu_metrics_t tNewMetrics;
	xui_menu_metrics_t tOldMetrics;
	int iRet;
	if ( (pData == NULL) || !__xuiMenuMetricsValid(pMetrics) ) return XUI_ERROR_INVALID_ARGUMENT;
	tNewMetrics = *pMetrics;
	tNewMetrics.iSize = sizeof(tNewMetrics);
	if ( __xuiMenuMetricsEqual(&pData->tMetrics, &tNewMetrics) ) return XUI_OK;
	tOldMetrics = pData->tMetrics;
	pData->tMetrics = tNewMetrics;
	pData->bMeasureDirty = 1;
	iRet = __xuiMenuApplyPopupSize(pWidget, pData);
	if ( iRet != XUI_OK ) {
		pData->tMetrics = tOldMetrics;
		pData->bMeasureDirty = 1;
		(void)__xuiMenuApplyPopupSize(pWidget, pData);
	}
	return iRet;
}

XUI_API int xuiMenuGetMetrics(xui_widget pWidget, xui_menu_metrics_t* pMetrics)
{
	xui_menu_data_t* pData = __xuiMenuGetData(pWidget);
	if ( (pData == NULL) || (pMetrics == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	*pMetrics = pData->tMetrics;
	pMetrics->iSize = sizeof(*pMetrics);
	return XUI_OK;
}

XUI_API int xuiMenuSetColors(xui_widget pWidget, const xui_menu_colors_t* pColors)
{
	xui_menu_data_t* pData = __xuiMenuGetData(pWidget);
	xui_menu_colors_t tColors;
	xui_menu_colors_t tNewColors;
	xui_menu_colors_t tOldColors;
	int iRet;
	if ( (pData == NULL) || !__xuiMenuColorsValid(pColors) ) return XUI_ERROR_INVALID_ARGUMENT;
	tNewColors = *pColors;
	tNewColors.iSize = sizeof(tNewColors);
	if ( __xuiMenuColorsEqual(&pData->tColors, &tNewColors) ) return XUI_OK;
	tOldColors = pData->tColors;
	pData->tColors = tNewColors;
	__xuiMenuResolveColors(pWidget, pData, &tColors);
	iRet = __xuiMenuApplyPopupColors(pData, &tColors);
	if ( iRet == XUI_OK ) iRet = xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	if ( iRet != XUI_OK ) {
		pData->tColors = tOldColors;
		__xuiMenuResolveColors(pWidget, pData, &tColors);
		(void)__xuiMenuApplyPopupColors(pData, &tColors);
	}
	return iRet;
}

XUI_API int xuiMenuGetColors(xui_widget pWidget, xui_menu_colors_t* pColors)
{
	xui_menu_data_t* pData = __xuiMenuGetData(pWidget);
	if ( (pData == NULL) || (pColors == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	*pColors = pData->tColors;
	pColors->iSize = sizeof(*pColors);
	return XUI_OK;
}

XUI_API int xuiMenuMeasure(xui_widget pWidget, float* pWidth, float* pHeight)
{
	xui_menu_data_t* pData = __xuiMenuGetData(pWidget);
	int iRet;

	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	iRet = __xuiMenuMeasureData(pWidget, pData);
	if ( iRet != XUI_OK ) return iRet;
	if ( pWidth != NULL ) *pWidth = pData->fContentW;
	if ( pHeight != NULL ) *pHeight = pData->fContentH;
	return XUI_OK;
}

XUI_API int xuiMenuOpenAt(xui_widget pWidget, xui_widget pOwner, float fX, float fY)
{
	xui_menu_data_t* pData = __xuiMenuGetData(pWidget);
	xui_rect_t tAnchor;
	xui_context pContext;
	int iOldHover;
	int iRet;

	if ( pData == NULL || pData->pPopup == NULL ||
	     !__xuiMenuCoordinateValid(fX) || !__xuiMenuCoordinateValid(fY) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pContext = xuiWidgetGetContext(pWidget);
	if ( pOwner != NULL && (!xuiInternalWidgetIsValid(pOwner) || xuiWidgetGetContext(pOwner) != pContext) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	iRet = __xuiMenuCancelPress(pWidget, pData);
	if ( iRet != XUI_OK ) return iRet;
	if ( __xuiMenuGetData(pWidget) != pData ) return XUI_OK;
	iRet = __xuiMenuApplyPopupSize(pWidget, pData);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiPopupSetOwner(pData->pPopup, pOwner);
	if ( iRet == XUI_OK ) iRet = xuiPopupSetFocusRestore(pData->pPopup, (pData->pParentMenu != NULL) ? pData->pParentMenu : pOwner);
	if ( iRet == XUI_OK ) iRet = xuiPopupSetClosePolicy(pData->pPopup,
		(pData->pParentMenu != NULL) ? XUI_POPUP_OUTSIDE_IGNORE : XUI_POPUP_OUTSIDE_CLOSE,
		(pData->pParentMenu != NULL) ? XUI_POPUP_OWNER_PASSTHROUGH : XUI_POPUP_OWNER_CLOSE,
		XUI_POPUP_ESCAPE_CLOSE);
	tAnchor = (xui_rect_t){fX, fY, 0.0f, 0.0f};
	if ( iRet == XUI_OK ) iRet = xuiPopupSetAnchorRect(pData->pPopup, tAnchor);
	if ( iRet == XUI_OK ) iRet = xuiPopupSetAnchor(pData->pPopup, XUI_POPUP_ANCHOR_FIXED);
	if ( iRet == XUI_OK ) iRet = xuiPopupSetDirection(pData->pPopup, XUI_POPUP_DIRECTION_RIGHT_DOWN);
	if ( iRet == XUI_OK ) iRet = xuiPopupSetGap(pData->pPopup, 0.0f);
	if ( iRet == XUI_OK ) iRet = xuiPopupSetScroll(pData->pPopup, 0.0f, 0.0f);
	if ( iRet != XUI_OK ) return iRet;
	iOldHover = pData->iHover;
	pData->iPendingSubmenu = -1;
	pData->fPendingSubmenuTime = 0.0f;
	pData->iDeferredHover = -1;
	pData->fDeferredHoverTime = 0.0f;
	pData->bLastPointer = 0;
	pData->bAimActive = 0;
	pData->iHover = __xuiMenuNextEnabled(pData, -1, 1);
	iRet = xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	if ( iRet == XUI_OK ) iRet = xuiPopupSetOpen(pData->pPopup, 1);
	if ( iRet == XUI_OK && xuiInternalWidgetIsValid(pWidget) && !xuiInternalContextDestroyPending(pContext) ) {
		iRet = xuiSetFocusWidget(pContext, pWidget);
	}
	if ( iRet != XUI_OK && __xuiMenuGetData(pWidget) == pData ) pData->iHover = iOldHover;
	if ( iRet == XUI_OK && __xuiMenuGetData(pWidget) == pData ) {
		xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_STATE_CHANGED);
		xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_SELECTION_CHANGED);
	}
	return iRet;
}

XUI_API int xuiMenuOpenForOwner(xui_widget pWidget, xui_widget pOwner)
{
	xui_menu_data_t* pData = __xuiMenuGetData(pWidget);
	xui_context pContext;
	int iOldHover;
	int iRet;

	if ( pData == NULL || pData->pPopup == NULL || pOwner == NULL || !xuiInternalWidgetIsValid(pOwner) ) return XUI_ERROR_INVALID_ARGUMENT;
	pContext = xuiWidgetGetContext(pWidget);
	if ( xuiWidgetGetContext(pOwner) != pContext ) return XUI_ERROR_INVALID_ARGUMENT;
	iRet = __xuiMenuCancelPress(pWidget, pData);
	if ( iRet != XUI_OK ) return iRet;
	if ( __xuiMenuGetData(pWidget) != pData ) return XUI_OK;
	iRet = __xuiMenuApplyPopupSize(pWidget, pData);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiPopupSetOwner(pData->pPopup, pOwner);
	if ( iRet == XUI_OK ) iRet = xuiPopupSetFocusRestore(pData->pPopup, (pData->pParentMenu != NULL) ? pData->pParentMenu : pOwner);
	if ( iRet == XUI_OK ) iRet = xuiPopupSetClosePolicy(pData->pPopup,
		(pData->pParentMenu != NULL) ? XUI_POPUP_OUTSIDE_IGNORE : XUI_POPUP_OUTSIDE_CLOSE,
		(pData->pParentMenu != NULL) ? XUI_POPUP_OWNER_PASSTHROUGH : XUI_POPUP_OWNER_CLOSE,
		XUI_POPUP_ESCAPE_CLOSE);
	if ( iRet == XUI_OK ) iRet = xuiPopupClearAnchorRect(pData->pPopup);
	if ( iRet == XUI_OK ) iRet = xuiPopupSetAnchor(pData->pPopup, XUI_POPUP_ANCHOR_BOTTOM_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiPopupSetDirection(pData->pPopup, XUI_POPUP_DIRECTION_RIGHT_DOWN);
	if ( iRet == XUI_OK ) iRet = xuiPopupSetGap(pData->pPopup, 2.0f);
	if ( iRet == XUI_OK ) iRet = xuiPopupSetScroll(pData->pPopup, 0.0f, 0.0f);
	if ( iRet != XUI_OK ) return iRet;
	iOldHover = pData->iHover;
	pData->iPendingSubmenu = -1;
	pData->fPendingSubmenuTime = 0.0f;
	pData->iDeferredHover = -1;
	pData->fDeferredHoverTime = 0.0f;
	pData->bLastPointer = 0;
	pData->bAimActive = 0;
	pData->iHover = __xuiMenuNextEnabled(pData, -1, 1);
	iRet = xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	if ( iRet == XUI_OK ) iRet = xuiPopupSetOpen(pData->pPopup, 1);
	if ( iRet == XUI_OK && xuiInternalWidgetIsValid(pWidget) && !xuiInternalContextDestroyPending(pContext) ) {
		iRet = xuiSetFocusWidget(pContext, pWidget);
	}
	if ( iRet != XUI_OK && __xuiMenuGetData(pWidget) == pData ) pData->iHover = iOldHover;
	if ( iRet == XUI_OK && __xuiMenuGetData(pWidget) == pData ) {
		xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_STATE_CHANGED);
		xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_SELECTION_CHANGED);
	}
	return iRet;
}

XUI_API int xuiMenuClose(xui_widget pWidget)
{
	xui_menu_data_t* pData = __xuiMenuGetData(pWidget);
	xui_menu_data_t* pParentData;
	xui_widget pParentMenu;
	xui_widget pPopup;
	int iRet;
	int iCloseRet;

	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	iRet = __xuiMenuCancelPress(pWidget, pData);
	if ( iRet != XUI_OK ) return iRet;
	if ( __xuiMenuGetData(pWidget) != pData ) return XUI_OK;
	pParentMenu = pData->pParentMenu;
	pPopup = pData->pPopup;
	if ( pParentMenu != NULL ) {
		pParentData = __xuiMenuGetData(pParentMenu);
		if ( (pParentData != NULL) && (pParentData->pOpenSubmenu == pWidget) ) pParentData->pOpenSubmenu = NULL;
	}
	pData->pParentMenu = NULL;
	pData->iParentItem = -1;
	iRet = __xuiMenuCloseSubmenu(pWidget, pData);
	pData = __xuiMenuGetData(pWidget);
	if ( pData == NULL ) return iRet;
	pData->iHover = -1;
	pData->iPendingSubmenu = -1;
	pData->fPendingSubmenuTime = 0.0f;
	iCloseRet = (pPopup != NULL) ? xuiPopupSetOpen(pPopup, 0) : XUI_OK;
	if ( iRet == XUI_OK ) iRet = iCloseRet;
	if ( !xuiInternalWidgetIsValid(pWidget) ) return iRet;
	iCloseRet = xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	if ( iCloseRet == XUI_OK ) {
		xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_STATE_CHANGED);
		xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_SELECTION_CHANGED);
	}
	return (iRet != XUI_OK) ? iRet : iCloseRet;
}

XUI_API int xuiMenuIsOpen(xui_widget pWidget)
{
	xui_menu_data_t* pData = __xuiMenuGetData(pWidget);
	return (pData != NULL && pData->pPopup != NULL) ? xuiPopupIsOpen(pData->pPopup) : 0;
}

XUI_API xui_widget xuiMenuGetPopupWidget(xui_widget pWidget)
{
	xui_menu_data_t* pData = __xuiMenuGetData(pWidget);
	return (pData != NULL) ? pData->pPopup : NULL;
}

XUI_API xui_widget xuiMenuGetContentWidget(xui_widget pWidget)
{
	return (xuiInternalWidgetIsValid(pWidget) && (__xuiMenuGetData(pWidget) != NULL)) ? pWidget : NULL;
}

XUI_API xui_widget xuiMenuGetOwner(xui_widget pWidget)
{
	xui_menu_data_t* pData = __xuiMenuGetData(pWidget);
	if ( (pData == NULL) || (pData->pPopup == NULL) ) {
		return NULL;
	}
	return xuiPopupGetOwner(pData->pPopup);
}

#endif
