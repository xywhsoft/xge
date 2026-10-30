#include "xui_config.h"
#if XUI_ENABLE_MENUBAR
#include "xui_internal.h"
#include "xui_pointer_press_internal.h"

#include <ctype.h>
#include <limits.h>
#include <math.h>
#include <string.h>

#define XUI_MENUBAR_ITEM_STATE_MASK XUI_MENUBAR_ITEM_ENABLED

/* Private bridge used to preserve callbacks while a MenuBar temporarily owns a Menu. */
int xuiInternalMenuGetSelectBinding(xui_widget pWidget, xui_menu_select_proc* pOnSelect, void** ppUser);

typedef struct xui_menubar_data_t {
	xui_widget pWidget;
	struct xui_menubar_data_t* pNext;
	xui_font pFont;
	xui_menubar_item_t arrItems[XUI_MENUBAR_ITEM_CAPACITY];
	char* arrDisplayText[XUI_MENUBAR_ITEM_CAPACITY];
	float arrTextWidth[XUI_MENUBAR_ITEM_CAPACITY];
	unsigned char arrSelectBound[XUI_MENUBAR_ITEM_CAPACITY];
	xui_menu_select_proc arrPreviousSelect[XUI_MENUBAR_ITEM_CAPACITY];
	void* arrPreviousSelectUser[XUI_MENUBAR_ITEM_CAPACITY];
	xui_menubar_metrics_t tMetrics;
	xui_menubar_colors_t tColors;
	xui_menubar_metrics_t tLayoutMetrics;
	xui_vec2_t tMeasured;
	xui_font pLayoutFont;
	xui_menu_select_proc onSelect;
	void* pSelectUser;
	float fLayoutHeight;
	uint32_t iLayoutDpiGeneration;
	int iItemCount;
	int iHover;
	int iActive;
	int iOpen;
	xui_pointer_owner_t tPointerOwner;
	int iChangeCount;
	int bLayoutDirty;
	int bLayoutValid;
} xui_menubar_data_t;

typedef struct xui_menubar_resolved_t {
	uint32_t iActiveTextColor;
	xui_font pFont;
	xui_menubar_metrics_t tMetrics;
	xui_menubar_colors_t tColors;
} xui_menubar_resolved_t;

static xui_menubar_data_t* __xuiMenuBarGetData(xui_widget pWidget);
static int __xuiMenuBarMetricsValid(const xui_menubar_metrics_t* pMetrics);
static int __xuiMenuBarColorsValid(const xui_menubar_colors_t* pColors);
static int __xuiMenuBarMenuValid(xui_widget pWidget, xui_widget pMenu);
static int __xuiMenuBarSetHover(xui_widget pWidget, xui_menubar_data_t* pData, int iIndex);
static int __xuiMenuBarCloseOpen(xui_widget pWidget, xui_menubar_data_t* pData);
static int __xuiMenuBarModeExit(xui_widget pWidget, xui_menubar_data_t* pData);
static int __xuiMenuBarHotkeyEvent(xui_widget pWidget, const xui_event_t* pEvent, void* pUser);

static void __xuiMenuBarMarkChanged(xui_menubar_data_t* pData)
{
	if ( pData != NULL && pData->iChangeCount < INT_MAX ) pData->iChangeCount++;
}

static int __xuiMenuBarReleaseSelectBinding(xui_widget pWidget, xui_menubar_data_t* pData, int iIndex)
{
	xui_widget pMenu;
	xui_menu_select_proc onCurrent;
	xui_menu_select_proc onPrevious;
	void* pCurrentUser;
	void* pPreviousUser;
	int iRet;

	if ( pWidget == NULL || pData == NULL || iIndex < 0 || iIndex >= pData->iItemCount ||
	     !pData->arrSelectBound[iIndex] ) return XUI_OK;
	pMenu = pData->arrItems[iIndex].pMenu;
	onPrevious = pData->arrPreviousSelect[iIndex];
	pPreviousUser = pData->arrPreviousSelectUser[iIndex];
	pData->arrSelectBound[iIndex] = 0;
	pData->arrPreviousSelect[iIndex] = NULL;
	pData->arrPreviousSelectUser[iIndex] = NULL;
	if ( !__xuiMenuBarMenuValid(pWidget, pMenu) ) return XUI_OK;
	iRet = xuiInternalMenuGetSelectBinding(pMenu, &onCurrent, &pCurrentUser);
	if ( iRet != XUI_OK ) return iRet;
	if ( onCurrent != pData->onSelect || pCurrentUser != pData->pSelectUser ) return XUI_OK;
	return xuiMenuSetSelect(pMenu, onPrevious, pPreviousUser);
}

static int __xuiMenuBarBindSelect(xui_widget pWidget, xui_menubar_data_t* pData, int iIndex,
	xui_menu_select_proc onSelect, void* pUser)
{
	xui_widget pMenu;
	xui_menu_select_proc onCurrent;
	void* pCurrentUser;
	int iRet;

	if ( pWidget == NULL || pData == NULL || iIndex < 0 || iIndex >= pData->iItemCount ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pMenu = pData->arrItems[iIndex].pMenu;
	if ( !__xuiMenuBarMenuValid(pWidget, pMenu) || pMenu == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	iRet = xuiInternalMenuGetSelectBinding(pMenu, &onCurrent, &pCurrentUser);
	if ( iRet != XUI_OK ) return iRet;
	if ( !pData->arrSelectBound[iIndex] ||
	     onCurrent != pData->onSelect || pCurrentUser != pData->pSelectUser ) {
		pData->arrPreviousSelect[iIndex] = onCurrent;
		pData->arrPreviousSelectUser[iIndex] = pCurrentUser;
	}
	iRet = xuiMenuSetSelect(pMenu, onSelect, pUser);
	if ( iRet != XUI_OK ) return iRet;
	pData->arrSelectBound[iIndex] = 1;
	return XUI_OK;
}

static int __xuiMenuBarAlpha(uint32_t iColor)
{
	return (int)(iColor & 0xffu);
}

static int __xuiMenuBarAsciiUpper(int iChar)
{
	return (iChar >= 'a' && iChar <= 'z') ? (iChar - 'a' + 'A') : iChar;
}

static int __xuiMenuBarItemEnabled(const xui_menubar_item_t* pItem)
{
	return (pItem != NULL) && ((pItem->iState & XUI_MENUBAR_ITEM_ENABLED) != 0);
}

static int __xuiMenuBarDescValid(const xui_menubar_desc_t* pDesc)
{
	if ( pDesc == NULL ) return 1;
	if ( (pDesc->iSize != 0) && (pDesc->iSize < sizeof(*pDesc)) ) return 0;
	if ( pDesc->bHasMetrics && !__xuiMenuBarMetricsValid(&pDesc->tMetrics) ) return 0;
	if ( pDesc->bHasColors && !__xuiMenuBarColorsValid(&pDesc->tColors) ) return 0;
	return 1;
}

static int __xuiMenuBarMetricValid(float fValue)
{
	return isfinite(fValue) && fValue >= 0.0f && fValue <= XUI_LAYOUT_UNBOUNDED;
}

static int __xuiMenuBarMetricsValid(const xui_menubar_metrics_t* pMetrics)
{
	double fMinimumItemWidth;
	double fMaximumWidth;
	double fMaximumHeight;

	if ( pMetrics == NULL ) return 0;
	if ( (pMetrics->iSize != 0) && (pMetrics->iSize < sizeof(*pMetrics)) ) return 0;
	if ( !__xuiMenuBarMetricValid(pMetrics->fHeight) ||
	     !__xuiMenuBarMetricValid(pMetrics->fPaddingX) ||
	     !__xuiMenuBarMetricValid(pMetrics->fPaddingY) ||
	     !__xuiMenuBarMetricValid(pMetrics->fItemPaddingX) ||
	     !__xuiMenuBarMetricValid(pMetrics->fItemGap) ||
	     !__xuiMenuBarMetricValid(pMetrics->fBorderWidth) ||
	     (pMetrics->fHeight <= 0.0f) ||
	     ((double)pMetrics->fPaddingY * 2.0 > (double)XUI_LAYOUT_UNBOUNDED) ||
	     ((double)pMetrics->fBorderWidth * 2.0 > (double)XUI_LAYOUT_UNBOUNDED) ) {
		return 0;
	}
	fMinimumItemWidth = (double)pMetrics->fItemPaddingX * 2.0;
	if ( fMinimumItemWidth < 20.0 ) fMinimumItemWidth = 20.0;
	fMaximumWidth = (double)pMetrics->fPaddingX * 2.0 +
		fMinimumItemWidth * (double)XUI_MENUBAR_ITEM_CAPACITY +
		(double)pMetrics->fItemGap * (double)(XUI_MENUBAR_ITEM_CAPACITY - 1);
	fMaximumHeight = (double)pMetrics->fPaddingY * 2.0 + 1.0;
	if ( fMaximumWidth > (double)XUI_LAYOUT_UNBOUNDED ||
	     fMaximumHeight > (double)XUI_LAYOUT_UNBOUNDED ) return 0;
	return 1;
}

static int __xuiMenuBarColorsValid(const xui_menubar_colors_t* pColors)
{
	if ( pColors == NULL ) return 0;
	return (pColors->iSize == 0) || (pColors->iSize >= sizeof(*pColors));
}

static int __xuiMenuBarMetricsEqual(const xui_menubar_metrics_t* pA,
	const xui_menubar_metrics_t* pB)
{
	return pA->fHeight == pB->fHeight &&
	       pA->fPaddingX == pB->fPaddingX &&
	       pA->fPaddingY == pB->fPaddingY &&
	       pA->fItemPaddingX == pB->fItemPaddingX &&
	       pA->fItemGap == pB->fItemGap &&
	       pA->fBorderWidth == pB->fBorderWidth;
}

static int __xuiMenuBarColorsEqual(const xui_menubar_colors_t* pA,
	const xui_menubar_colors_t* pB)
{
	return pA->iBackgroundColor == pB->iBackgroundColor &&
	       pA->iBorderColor == pB->iBorderColor &&
	       pA->iItemColor == pB->iItemColor &&
	       pA->iHoverColor == pB->iHoverColor &&
	       pA->iActiveColor == pB->iActiveColor &&
	       pA->iTextColor == pB->iTextColor &&
	       pA->iDisabledTextColor == pB->iDisabledTextColor &&
	       pA->iFocusColor == pB->iFocusColor;
}

static void __xuiMenuBarDefaultMetrics(xui_menubar_metrics_t* pMetrics)
{
	memset(pMetrics, 0, sizeof(*pMetrics));
	pMetrics->iSize = sizeof(*pMetrics);
	pMetrics->fHeight = 26.0f;
	pMetrics->fPaddingX = 5.0f;
	pMetrics->fPaddingY = 3.0f;
	pMetrics->fItemPaddingX = 10.0f;
	pMetrics->fItemGap = 1.0f;
	pMetrics->fBorderWidth = 1.0f;
}

static void __xuiMenuBarDefaultColors(xui_menubar_colors_t* pColors)
{
	memset(pColors, 0, sizeof(*pColors));
	pColors->iSize = sizeof(*pColors);
	pColors->iBackgroundColor = XUI_COLOR_RGBA(247, 250, 254, 255);
	pColors->iBorderColor = XUI_COLOR_RGBA(206, 220, 234, 255);
	pColors->iItemColor = XUI_COLOR_RGBA(0, 0, 0, 0);
	pColors->iHoverColor = XUI_COLOR_RGBA(224, 238, 250, 255);
	pColors->iActiveColor = XUI_COLOR_RGBA(47, 128, 214, 255);
	pColors->iTextColor = XUI_COLOR_RGBA(35, 58, 86, 255);
	pColors->iDisabledTextColor = XUI_COLOR_RGBA(142, 152, 166, 210);
	pColors->iFocusColor = XUI_COLOR_RGBA(47, 128, 214, 255);
}

static int __xuiMenuBarTextToDisplay(const char* sText, char* sBuffer, int iBufferSize, int* pMnemonic, int* pMnemonicOffset)
{
	int i;
	int j;
	int iMnemonic;
	int iMnemonicOffset;

	if ( pMnemonic != NULL ) *pMnemonic = 0;
	if ( pMnemonicOffset != NULL ) *pMnemonicOffset = -1;
	if ( (sBuffer != NULL) && (iBufferSize > 0) ) sBuffer[0] = '\0';
	if ( sText == NULL ) return 0;
	iMnemonic = 0;
	iMnemonicOffset = -1;
	j = 0;
	for ( i = 0; sText[i] != '\0'; i++ ) {
		if ( sText[i] == '&' ) {
			if ( sText[i + 1] == '&' ) {
				if ( (sBuffer != NULL) && (j < iBufferSize - 1) ) sBuffer[j] = '&';
				j++;
				i++;
			} else if ( sText[i + 1] != '\0' ) {
				iMnemonic = __xuiMenuBarAsciiUpper((unsigned char)sText[i + 1]);
				iMnemonicOffset = j;
			}
			continue;
		}
		if ( (sBuffer != NULL) && (j < iBufferSize - 1) ) sBuffer[j] = sText[i];
		j++;
	}
	if ( (sBuffer != NULL) && (iBufferSize > 0) ) sBuffer[(j < iBufferSize) ? j : (iBufferSize - 1)] = '\0';
	if ( pMnemonic != NULL ) *pMnemonic = iMnemonic;
	if ( pMnemonicOffset != NULL ) *pMnemonicOffset = iMnemonicOffset;
	return j;
}

static char* __xuiMenuBarDuplicateString(const char* sText)
{
	char* sCopy;
	size_t iSize;

	if ( sText == NULL ) sText = "";
	iSize = strlen(sText) + 1u;
	sCopy = (char*)xrtMalloc(iSize);
	if ( sCopy != NULL ) memcpy(sCopy, sText, iSize);
	return sCopy;
}

static char* __xuiMenuBarCreateDisplayText(const char* sText)
{
	char* sDisplay;
	size_t iSize;

	if ( sText == NULL ) sText = "";
	iSize = strlen(sText) + 1u;
	if ( iSize > (size_t)INT_MAX ) return NULL;
	sDisplay = (char*)xrtMalloc(iSize);
	if ( sDisplay == NULL ) return NULL;
	(void)__xuiMenuBarTextToDisplay(sText, sDisplay, (int)iSize, NULL, NULL);
	return sDisplay;
}

static void __xuiMenuBarFreeItems(xui_menubar_item_t* pItems, char** arrDisplay, int iCount)
{
	int i;

	for ( i = 0; i < iCount; i++ ) {
		if ( pItems[i].sText != NULL ) xrtFree((void*)pItems[i].sText);
		if ( arrDisplay[i] != NULL ) xrtFree(arrDisplay[i]);
		pItems[i].sText = NULL;
		arrDisplay[i] = NULL;
	}
}

static int __xuiMenuBarMenuValid(xui_widget pWidget, xui_widget pMenu)
{
	xui_context pContext;

	if ( pMenu == NULL ) return 1;
	if ( !xuiInternalWidgetIsValid(pWidget) || pMenu == pWidget ) return 0;
	pContext = xuiWidgetGetContext(pWidget);
	return xuiInternalContextIsValid(pContext) && xuiInternalMenuIsLive(pContext, pMenu);
}

static void __xuiMenuBarMenuUnavailable(xui_context pContext, xui_widget pMenu)
{
	xui_menubar_data_t* pData;
	int bChanged;
	int bSelectionChanged;
	int bCancelPress;
	int bCloseMode;
	int i;

	if ( !xuiInternalContextIsValid(pContext) || pMenu == NULL ) return;
	for ( pData = pContext->pMenuBarList; pData != NULL; pData = pData->pNext ) {
		bChanged = 0;
		bSelectionChanged = 0;
		bCancelPress = 0;
		bCloseMode = 0;
		for ( i = 0; i < pData->iItemCount; i++ ) {
			if ( pData->arrItems[i].pMenu != pMenu ) continue;
			pData->arrItems[i].pMenu = NULL;
			pData->arrItems[i].iState &= ~XUI_MENUBAR_ITEM_ENABLED;
			pData->arrSelectBound[i] = 0;
			pData->arrPreviousSelect[i] = NULL;
			pData->arrPreviousSelectUser[i] = NULL;
			if ( pData->iHover == i ) {
				pData->iHover = -1;
				bSelectionChanged = 1;
			}
			if ( pData->iActive == i ) {
				pData->iActive = -1;
				bSelectionChanged = 1;
				bCancelPress = 1;
			}
			if ( pData->iOpen == i ) {
				pData->iOpen = -1;
				bSelectionChanged = 1;
				bCloseMode = 1;
			}
			bChanged = 1;
		}
		if ( !bChanged ) continue;
		if ( bCancelPress && xuiInternalWidgetIsValid(pData->pWidget) ) {
			(void)xuiInternalPointerOwnerCancel(pContext, pData->pWidget,
				&pData->tPointerOwner);
		}
		__xuiMenuBarMarkChanged(pData);
		if ( xuiInternalWidgetIsValid(pData->pWidget) &&
		     !xuiInternalContextDestroyPending(pContext) ) {
			(void)xuiWidgetInvalidate(pData->pWidget,
				XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
			xuiInternalAccessibilityQueue(pData->pWidget, XUI_ACCESSIBLE_EVENT_NODE_CHANGED);
			xuiInternalAccessibilityQueue(pData->pWidget, XUI_ACCESSIBLE_EVENT_STATE_CHANGED);
			if ( bSelectionChanged ) xuiInternalAccessibilityQueue(pData->pWidget,
				XUI_ACCESSIBLE_EVENT_SELECTION_CHANGED);
			if ( bCloseMode && pContext->pMenuBarModeOwner == pData->pWidget ) {
				(void)__xuiMenuBarModeExit(pData->pWidget, pData);
			}
		}
	}
}

static int __xuiMenuBarItemValid(xui_widget pWidget, const xui_menubar_item_t* pItem)
{
	if ( pItem == NULL || (pItem->iState & ~XUI_MENUBAR_ITEM_STATE_MASK) != 0u ||
	     pItem->iMnemonic < 0 ) return 0;
	return __xuiMenuBarMenuValid(pWidget, pItem->pMenu);
}

static int __xuiMenuBarItemsEqual(const xui_menubar_data_t* pData,
	const xui_menubar_item_t* pItems, int iCount)
{
	const xui_menubar_item_t* pCurrent;
	const xui_menubar_item_t* pNext;
	const char* sCurrent;
	const char* sNext;
	int iMnemonic;
	int i;

	if ( pData == NULL || pData->iItemCount != iCount ) return 0;
	for ( i = 0; i < iCount; i++ ) {
		pCurrent = &pData->arrItems[i];
		pNext = &pItems[i];
		sCurrent = pCurrent->sText != NULL ? pCurrent->sText : "";
		sNext = pNext->sText != NULL ? pNext->sText : "";
		iMnemonic = pNext->iMnemonic != 0 ? __xuiMenuBarAsciiUpper(pNext->iMnemonic) : 0;
		if ( strcmp(sCurrent, sNext) != 0 ||
		     pCurrent->iState != pNext->iState ||
		     pCurrent->iValue != pNext->iValue ||
		     pCurrent->iMnemonic != iMnemonic ||
		     pCurrent->pMenu != pNext->pMenu ||
		     pCurrent->pUser != pNext->pUser ) return 0;
	}
	return 1;
}

static int __xuiMenuBarCopyItems(xui_menubar_item_t* pTarget, char** arrDisplay,
	const xui_menubar_item_t* pItems, int iCount)
{
	int i;

	memset(pTarget, 0, sizeof(xui_menubar_item_t) * XUI_MENUBAR_ITEM_CAPACITY);
	memset(arrDisplay, 0, sizeof(char*) * XUI_MENUBAR_ITEM_CAPACITY);
	for ( i = 0; i < iCount; i++ ) {
		pTarget[i] = pItems[i];
		memset(&pTarget[i].tRect, 0, sizeof(pTarget[i].tRect));
		pTarget[i].sText = __xuiMenuBarDuplicateString(pItems[i].sText);
		if ( pTarget[i].sText == NULL ) goto failed;
		arrDisplay[i] = __xuiMenuBarCreateDisplayText(pItems[i].sText);
		if ( arrDisplay[i] == NULL ) goto failed;
		if ( pTarget[i].iMnemonic != 0 ) pTarget[i].iMnemonic = __xuiMenuBarAsciiUpper(pTarget[i].iMnemonic);
	}
	return XUI_OK;

failed:
	__xuiMenuBarFreeItems(pTarget, arrDisplay, i + 1);
	return XUI_ERROR_OUT_OF_MEMORY;
}

static int __xuiMenuBarItemMnemonic(xui_menubar_item_t* pItem)
{
	int iMnemonic;

	if ( pItem == NULL ) return 0;
	if ( pItem->iMnemonic != 0 ) return __xuiMenuBarAsciiUpper(pItem->iMnemonic);
	(void)__xuiMenuBarTextToDisplay(pItem->sText, NULL, 0, &iMnemonic, NULL);
	return iMnemonic;
}

static xui_vec2_t __xuiMenuBarMeasureText(xui_widget pWidget, xui_font pFont, const char* sText)
{
	xui_vec2_t tSize;
	xui_proxy pProxy;
	size_t iLength;

	tSize.fX = 0.0f;
	tSize.fY = 18.0f;
	if ( (sText == NULL) || (sText[0] == '\0') ) return tSize;
	pProxy = xuiInternalContextGetProxy(xuiWidgetGetContext(pWidget));
	if ( (pProxy != NULL) && (pProxy->textMeasure != NULL) && (pFont != NULL) &&
	     (pProxy->textMeasure(pProxy, &(xui_text_item_t){.iSize=sizeof(xui_text_item_t), .pFont=pFont, .sText=sText, .iTextSize=-1, .iFlags=XUI_TEXT_SHAPE_DEFAULT}, &tSize) == XUI_OK) &&
	     isfinite(tSize.fX) && isfinite(tSize.fY) && (tSize.fX >= 0.0f) && (tSize.fY >= 0.0f) ) {
		return tSize;
	}
	iLength = strlen(sText);
	if ( iLength > (size_t)(XUI_LAYOUT_UNBOUNDED / 7.0f) ) {
		tSize.fX = XUI_LAYOUT_UNBOUNDED * 2.0f;
	} else {
		tSize.fX = (float)iLength * 7.0f;
	}
	tSize.fY = 18.0f;
	return tSize;
}

static int __xuiMenuBarTextExtentValid(xui_vec2_t tSize)
{
	return isfinite(tSize.fX) && isfinite(tSize.fY) &&
	       tSize.fX >= 0.0f && tSize.fY >= 0.0f &&
	       tSize.fX <= XUI_LAYOUT_UNBOUNDED &&
	       tSize.fY <= XUI_LAYOUT_UNBOUNDED;
}

static int __xuiMenuBarStyleColor(xui_widget pWidget, const char* sName, uint32_t* pColor)
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

static int __xuiMenuBarStyleFloat(xui_widget pWidget, const char* sName, float* pValue)
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

static xui_font __xuiMenuBarStyleFont(xui_widget pWidget, xui_font pBaseFont)
{
	xui_style_property_t tProperty;
	xui_font pFont;
	int iRet;

	memset(&tProperty, 0, sizeof(tProperty));
	tProperty.iSize = sizeof(tProperty);
	iRet = xuiWidgetGetResolvedStyleProperty(pWidget, "font.name", &tProperty);
	if ( (iRet == XUI_OK) &&
	     (tProperty.tValue.iType == XUI_STYLE_VALUE_STRING) &&
	     (tProperty.tValue.sText != NULL) &&
	     (tProperty.tValue.sText[0] != '\0') ) {
		pFont = xuiFindFont(xuiWidgetGetContext(pWidget), tProperty.tValue.sText);
		if ( pFont != NULL ) return pFont;
	}
	return (pBaseFont != NULL) ? pBaseFont : xuiGetDefaultFont(xuiWidgetGetContext(pWidget));
}

static void __xuiMenuBarResolve(xui_widget pWidget, xui_menubar_data_t* pData, xui_menubar_resolved_t* pOut)
{
	xui_menubar_metrics_t tMetrics;

	if ( (pWidget == NULL) || (pData == NULL) || (pOut == NULL) ) return;
	pOut->pFont = __xuiMenuBarStyleFont(pWidget, pData->pFont);
	tMetrics = pData->tMetrics;
	tMetrics.iSize = sizeof(tMetrics);
	pOut->tColors = pData->tColors;
	pOut->tColors.iSize = sizeof(pOut->tColors);
	(void)__xuiMenuBarStyleFloat(pWidget, "menubar.height", &tMetrics.fHeight);
	(void)__xuiMenuBarStyleFloat(pWidget, "menubar.padding.x", &tMetrics.fPaddingX);
	(void)__xuiMenuBarStyleFloat(pWidget, "menubar.padding.y", &tMetrics.fPaddingY);
	(void)__xuiMenuBarStyleFloat(pWidget, "menubar.item.padding.x", &tMetrics.fItemPaddingX);
	(void)__xuiMenuBarStyleFloat(pWidget, "menubar.item.gap", &tMetrics.fItemGap);
	(void)__xuiMenuBarStyleFloat(pWidget, "menubar.border.width", &tMetrics.fBorderWidth);
	(void)__xuiMenuBarStyleColor(pWidget, "menubar.background.color", &pOut->tColors.iBackgroundColor);
	(void)__xuiMenuBarStyleColor(pWidget, "menubar.border.color", &pOut->tColors.iBorderColor);
	(void)__xuiMenuBarStyleColor(pWidget, "menubar.item.color", &pOut->tColors.iItemColor);
	(void)__xuiMenuBarStyleColor(pWidget, "menubar.item.hover_color", &pOut->tColors.iHoverColor);
	(void)__xuiMenuBarStyleColor(pWidget, "menubar.item.active_color", &pOut->tColors.iActiveColor);
	(void)__xuiMenuBarStyleColor(pWidget, "menubar.text.color", &pOut->tColors.iTextColor);
	pOut->iActiveTextColor = XUI_COLOR_WHITE;
	(void)__xuiMenuBarStyleColor(pWidget, "menubar.text.active_color", &pOut->iActiveTextColor);
	(void)__xuiMenuBarStyleColor(pWidget, "menubar.text.disabled_color", &pOut->tColors.iDisabledTextColor);
	(void)__xuiMenuBarStyleColor(pWidget, "menubar.focus.color", &pOut->tColors.iFocusColor);
	if ( tMetrics.fHeight < 18.0f ) tMetrics.fHeight = 18.0f;
	pOut->tMetrics = __xuiMenuBarMetricsValid(&tMetrics) ? tMetrics : pData->tMetrics;
	if ( pOut->tMetrics.fHeight < 18.0f ) pOut->tMetrics.fHeight = 18.0f;
	pOut->tMetrics.iSize = sizeof(pOut->tMetrics);
}

static int __xuiMenuBarLayoutItems(xui_widget pWidget, xui_menubar_data_t* pData, const xui_menubar_resolved_t* pResolved, xui_vec2_t* pMeasured)
{
	xui_context pContext;
	xui_rect_t arrRect[XUI_MENUBAR_ITEM_CAPACITY];
	float arrTextWidth[XUI_MENUBAR_ITEM_CAPACITY];
	xui_rect_t tRect;
	xui_vec2_t tText;
	xui_vec2_t tMeasured;
	double fX;
	double fY;
	double fH;
	double fW;
	double fMeasuredW;
	int i;

	if ( (pWidget == NULL) || (pData == NULL) || (pResolved == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( !__xuiMenuBarMetricsValid(&pResolved->tMetrics) ) return XUI_ERROR_INVALID_ARGUMENT;
	pContext = xuiWidgetGetContext(pWidget);
	if ( !xuiInternalContextIsValid(pContext) ) return XUI_ERROR_INVALID_ARGUMENT;
	tRect = xuiWidgetGetRect(pWidget);
	if ( pData->bLayoutValid && !pData->bLayoutDirty &&
	     pData->pLayoutFont == pResolved->pFont &&
	     pData->iLayoutDpiGeneration == pContext->iDpiGeneration &&
	     pData->fLayoutHeight == tRect.fH &&
	     pData->tLayoutMetrics.fHeight == pResolved->tMetrics.fHeight &&
	     pData->tLayoutMetrics.fPaddingX == pResolved->tMetrics.fPaddingX &&
	     pData->tLayoutMetrics.fPaddingY == pResolved->tMetrics.fPaddingY &&
	     pData->tLayoutMetrics.fItemPaddingX == pResolved->tMetrics.fItemPaddingX &&
	     pData->tLayoutMetrics.fItemGap == pResolved->tMetrics.fItemGap ) {
		if ( pMeasured != NULL ) *pMeasured = pData->tMeasured;
		return XUI_OK;
	}
	memset(arrRect, 0, sizeof(arrRect));
	memset(arrTextWidth, 0, sizeof(arrTextWidth));
	fX = (double)pResolved->tMetrics.fPaddingX;
	fY = (double)pResolved->tMetrics.fPaddingY;
	fH = (double)pResolved->tMetrics.fHeight - (double)pResolved->tMetrics.fPaddingY * 2.0;
	if ( tRect.fH > 0.0f ) {
		fH = (double)tRect.fH - (double)pResolved->tMetrics.fPaddingY * 2.0;
	}
	if ( fH < 1.0 ) fH = 1.0;
	if ( fX > (double)XUI_LAYOUT_UNBOUNDED ||
	     fY > (double)XUI_LAYOUT_UNBOUNDED ||
	     fH > (double)XUI_LAYOUT_UNBOUNDED ) return XUI_ERROR_INVALID_ARGUMENT;
	for ( i = 0; i < pData->iItemCount; i++ ) {
		tText = __xuiMenuBarMeasureText(pWidget, pResolved->pFont, pData->arrDisplayText[i]);
		if ( !__xuiMenuBarTextExtentValid(tText) ) return XUI_ERROR_INVALID_ARGUMENT;
		arrTextWidth[i] = tText.fX;
		fW = (double)tText.fX + (double)pResolved->tMetrics.fItemPaddingX * 2.0;
		if ( fW < 20.0 ) fW = 20.0;
		if ( fW > (double)XUI_LAYOUT_UNBOUNDED ||
		     fX + fW > (double)XUI_LAYOUT_UNBOUNDED ) return XUI_ERROR_INVALID_ARGUMENT;
		arrRect[i] = xuiInternalRectFromFloatNearest((float)fX, (float)fY,
			(float)fW, (float)fH);
		fX += fW + (double)pResolved->tMetrics.fItemGap;
		if ( fX > (double)XUI_LAYOUT_UNBOUNDED ) return XUI_ERROR_INVALID_ARGUMENT;
	}
	fMeasuredW = fX + (double)pResolved->tMetrics.fPaddingX;
	if ( pData->iItemCount > 0 ) fMeasuredW -= (double)pResolved->tMetrics.fItemGap;
	if ( fMeasuredW < (double)pResolved->tMetrics.fPaddingX * 2.0 ) {
		fMeasuredW = (double)pResolved->tMetrics.fPaddingX * 2.0;
	}
	if ( fMeasuredW > (double)XUI_LAYOUT_UNBOUNDED ) return XUI_ERROR_INVALID_ARGUMENT;
	tMeasured.fX = xuiInternalSnapSize((float)fMeasuredW);
	tMeasured.fY = xuiInternalSnapSize(pResolved->tMetrics.fHeight);
	memcpy(pData->arrTextWidth, arrTextWidth, sizeof(arrTextWidth));
	for ( i = 0; i < pData->iItemCount; i++ ) pData->arrItems[i].tRect = arrRect[i];
	pData->pLayoutFont = pResolved->pFont;
	pData->tLayoutMetrics = pResolved->tMetrics;
	pData->fLayoutHeight = tRect.fH;
	pData->iLayoutDpiGeneration = pContext->iDpiGeneration;
	pData->tMeasured = tMeasured;
	pData->bLayoutDirty = 0;
	pData->bLayoutValid = 1;
	if ( pMeasured != NULL ) *pMeasured = tMeasured;
	return XUI_OK;
}

static int __xuiMenuBarIndexAt(xui_menubar_data_t* pData, float fX, float fY)
{
	xui_rect_t tRect;
	int i;

	if ( pData == NULL ) return -1;
	for ( i = 0; i < pData->iItemCount; i++ ) {
		tRect = pData->arrItems[i].tRect;
		if ( __xuiMenuBarItemEnabled(&pData->arrItems[i]) &&
		     (fX >= tRect.fX) && (fY >= tRect.fY) &&
		     (fX < tRect.fX + tRect.fW) && (fY < tRect.fY + tRect.fH) ) {
			return i;
		}
	}
	return -1;
}

static int __xuiMenuBarNextEnabled(xui_menubar_data_t* pData, int iCurrent, int iStep)
{
	int i;
	int iIndex;

	if ( (pData == NULL) || (pData->iItemCount <= 0) || (iStep == 0) ) return -1;
	iIndex = iCurrent;
	for ( i = 0; i < pData->iItemCount; i++ ) {
		iIndex += iStep;
		if ( iIndex < 0 ) iIndex = pData->iItemCount - 1;
		else if ( iIndex >= pData->iItemCount ) iIndex = 0;
		if ( __xuiMenuBarItemEnabled(&pData->arrItems[iIndex]) ) return iIndex;
	}
	return -1;
}

static int __xuiMenuBarMnemonicIndex(xui_menubar_data_t* pData, int iKey)
{
	int i;
	int iMnemonic;

	if ( pData == NULL ) return -1;
	iKey = __xuiMenuBarAsciiUpper(iKey);
	for ( i = 0; i < pData->iItemCount; i++ ) {
		iMnemonic = __xuiMenuBarItemMnemonic(&pData->arrItems[i]);
		if ( __xuiMenuBarItemEnabled(&pData->arrItems[i]) && (iMnemonic != 0) && (iMnemonic == iKey) ) {
			return i;
		}
	}
	return -1;
}

static int __xuiMenuBarMenuNextEnabled(xui_widget pMenu, int iCurrent, int iStep)
{
	const xui_menu_item_t* pItem;
	int iCount;
	int i;
	int iIndex;

	if ( (pMenu == NULL) || (iStep == 0) ) return -1;
	iCount = xuiMenuGetItemCount(pMenu);
	if ( iCount <= 0 ) return -1;
	iIndex = iCurrent;
	for ( i = 0; i < iCount; i++ ) {
		iIndex += iStep;
		if ( iIndex < 0 ) iIndex = iCount - 1;
		else if ( iIndex >= iCount ) iIndex = 0;
		pItem = xuiMenuGetItem(pMenu, iIndex);
		if ( (pItem != NULL) &&
		     (pItem->iType != XUI_MENU_ITEM_SEPARATOR) &&
		     ((pItem->iState & XUI_MENU_ITEM_ENABLED) != 0u) ) {
			return iIndex;
		}
	}
	return -1;
}

static int __xuiMenuBarSyncOpen(xui_widget pWidget, xui_menubar_data_t* pData)
{
	xui_context pContext;
	xui_widget pMenu;
	int iRet;

	if ( (pWidget == NULL) || (pData == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	pContext = xuiWidgetGetContext(pWidget);
	if ( (pData->iOpen < 0) || (pData->iOpen >= pData->iItemCount) ) return XUI_OK;
	pMenu = pData->arrItems[pData->iOpen].pMenu;
	if ( !__xuiMenuBarMenuValid(pWidget, pMenu) || !xuiMenuIsOpen(pMenu) ) {
		iRet = __xuiMenuBarReleaseSelectBinding(pWidget, pData, pData->iOpen);
		if ( iRet != XUI_OK ) return iRet;
		pData->iOpen = -1;
		pData->iActive = -1;
		__xuiMenuBarMarkChanged(pData);
		iRet = xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
		if ( iRet != XUI_OK ) return iRet;
		xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_STATE_CHANGED);
		if ( pContext->pMenuBarModeOwner == pWidget ) {
			iRet = __xuiMenuBarModeExit(pWidget, pData);
			if ( iRet != XUI_OK ) return iRet;
		}
	}
	return XUI_OK;
}

static xui_rect_t __xuiMenuBarItemAnchorRect(xui_widget pWidget,
	const xui_menubar_data_t* pData, int iIndex)
{
	xui_rect_t tWorld = xuiWidgetGetWorldRect(pWidget);
	xui_rect_t tItem = pData->arrItems[iIndex].tRect;

	return (xui_rect_t){
		tWorld.fX + tItem.fX,
		tWorld.fY + tItem.fY + tItem.fH,
		0.0f,
		0.0f
	};
}

static int __xuiMenuBarSyncOpenPlacement(xui_widget pWidget, xui_menubar_data_t* pData)
{
	xui_menubar_resolved_t tResolved;
	xui_widget pMenu;
	xui_widget pPopup;
	int iRet;

	if ( pWidget == NULL || pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pData->iOpen < 0 || pData->iOpen >= pData->iItemCount ) return XUI_OK;
	pMenu = pData->arrItems[pData->iOpen].pMenu;
	if ( !__xuiMenuBarMenuValid(pWidget, pMenu) || !xuiMenuIsOpen(pMenu) ) return XUI_OK;
	__xuiMenuBarResolve(pWidget, pData, &tResolved);
	iRet = __xuiMenuBarLayoutItems(pWidget, pData, &tResolved, NULL);
	if ( iRet != XUI_OK ) return iRet;
	pPopup = xuiMenuGetPopupWidget(pMenu);
	if ( pPopup == NULL ) return XUI_OK;
	return xuiPopupSetAnchorRect(pPopup,
		__xuiMenuBarItemAnchorRect(pWidget, pData, pData->iOpen));
}

static int __xuiMenuBarLayoutComplete(xui_widget pWidget, xui_rect_t tContentRect, void* pUser)
{
	xui_menubar_data_t* pData;

	(void)tContentRect;
	(void)pUser;
	pData = __xuiMenuBarGetData(pWidget);
	return (pData != NULL) ? __xuiMenuBarSyncOpenPlacement(pWidget, pData) : XUI_OK;
}

static int __xuiMenuBarInvalidateItem(xui_widget pWidget, xui_menubar_data_t* pData, int iIndex)
{
	xui_rect_t tRect;
	xui_rect_t tWidget;
	int64_t iContentW;
	int64_t iContentH;
	int64_t iLeft;
	int64_t iTop;
	int64_t iRight;
	int64_t iBottom;

	if ( (pWidget == NULL) || (pData == NULL) || (iIndex < 0) || (iIndex >= pData->iItemCount) ) {
		return XUI_OK;
	}
	tRect = pData->arrItems[iIndex].tRect;
	if ( (tRect.fW <= 0.0f) || (tRect.fH <= 0.0f) ) {
		return XUI_OK;
	}
	tWidget = xuiWidgetGetRect(pWidget);
	iContentW = tWidget.fW > pData->tMeasured.fX ? tWidget.fW : pData->tMeasured.fX;
	iContentH = tWidget.fH > pData->tMeasured.fY ? tWidget.fH : pData->tMeasured.fY;
	if ( iContentW < 0 ) iContentW = 0;
	if ( iContentH < 0 ) iContentH = 0;
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
	return xuiWidgetInvalidateRect(pWidget, tRect, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
}

static int __xuiMenuBarSetHover(xui_widget pWidget, xui_menubar_data_t* pData, int iIndex)
{
	int iOldHover;
	int iRet;

	if ( (pWidget == NULL) || (pData == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( (iIndex < 0) || (iIndex >= pData->iItemCount) || !__xuiMenuBarItemEnabled(&pData->arrItems[iIndex]) ) {
		iIndex = -1;
	}
	if ( pData->iHover == iIndex ) return XUI_OK;
	iOldHover = pData->iHover;
	pData->iHover = iIndex;
	iRet = __xuiMenuBarInvalidateItem(pWidget, pData, iOldHover);
	if ( iRet == XUI_OK ) {
		iRet = __xuiMenuBarInvalidateItem(pWidget, pData, iIndex);
	}
	if ( iRet != XUI_OK ) {
		pData->iHover = iOldHover;
		(void)__xuiMenuBarInvalidateItem(pWidget, pData, iOldHover);
		(void)__xuiMenuBarInvalidateItem(pWidget, pData, iIndex);
		return iRet;
	}
	xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_SELECTION_CHANGED);
	return iRet;
}

static int __xuiMenuBarModeEnter(xui_widget pWidget, xui_menubar_data_t* pData)
{
	xui_menubar_data_t* pOldData;
	xui_context pContext;
	xui_widget pOldOwner;
	xui_widget pInheritedRestore;
	xui_widget pFocus;
	int iRet;

	if ( pWidget == NULL || pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	pContext = xuiWidgetGetContext(pWidget);
	if ( !xuiInternalContextIsValid(pContext) ) return XUI_ERROR_INVALID_ARGUMENT;
	pOldOwner = pContext->pMenuBarModeOwner;
	pInheritedRestore = NULL;
	if ( pOldOwner != NULL && pOldOwner != pWidget ) {
		pOldData = __xuiMenuBarGetData(pOldOwner);
		if ( pOldData != NULL ) {
			pInheritedRestore = pContext->pMenuBarFocusRestore;
			if ( pOldData->iOpen >= 0 || pOldData->iActive >= 0 ||
			     pOldData->tPointerOwner.bActive ) {
				iRet = __xuiMenuBarCloseOpen(pOldOwner, pOldData);
				if ( iRet != XUI_OK ) return iRet;
			}
			if ( __xuiMenuBarGetData(pOldOwner) == pOldData ) {
				iRet = __xuiMenuBarModeExit(pOldOwner, pOldData);
				if ( iRet != XUI_OK ) return iRet;
			}
		} else {
			pContext->pMenuBarModeOwner = NULL;
			pContext->pMenuBarFocusRestore = NULL;
		}
	}
	if ( pInheritedRestore != NULL &&
	     (!xuiWidgetIsAttachedToContext(pInheritedRestore) ||
	      xuiWidgetGetContext(pInheritedRestore) != pContext) ) {
		pInheritedRestore = NULL;
	}
	pFocus = xuiGetFocusWidget(pContext);
	if ( pContext->pMenuBarModeOwner != pWidget ) {
		pContext->pMenuBarModeOwner = pWidget;
		pContext->pMenuBarFocusRestore = (pInheritedRestore != NULL) ?
			pInheritedRestore : ((pFocus != pWidget) ? pFocus : NULL);
	}
	iRet = xuiSetFocusWidget(pContext, pWidget);
	if ( iRet != XUI_OK ) {
		if ( pContext->pMenuBarModeOwner == pWidget ) {
			pContext->pMenuBarModeOwner = NULL;
			pContext->pMenuBarFocusRestore = NULL;
		}
	}
	return iRet;
}

static void __xuiMenuBarModeAbandon(xui_widget pWidget)
{
	xui_context pContext;

	if ( pWidget == NULL ) return;
	pContext = xuiWidgetGetContext(pWidget);
	if ( !xuiInternalContextIsValid(pContext) || pContext->pMenuBarModeOwner != pWidget ) return;
	pContext->pMenuBarModeOwner = NULL;
	pContext->pMenuBarFocusRestore = NULL;
}

static int __xuiMenuBarModeExit(xui_widget pWidget, xui_menubar_data_t* pData)
{
	xui_context pContext;
	xui_widget pRestore;
	int iFirstRet;
	int iRet;

	if ( pWidget == NULL || pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	pContext = xuiWidgetGetContext(pWidget);
	if ( !xuiInternalContextIsValid(pContext) ) return XUI_ERROR_INVALID_ARGUMENT;
	iFirstRet = __xuiMenuBarSetHover(pWidget, pData, -1);
	if ( pContext->pMenuBarModeOwner != pWidget ) return iFirstRet;
	pRestore = pContext->pMenuBarFocusRestore;
	pContext->pMenuBarModeOwner = NULL;
	pContext->pMenuBarFocusRestore = NULL;
	if ( xuiGetFocusWidget(pContext) != pWidget ) return iFirstRet;
	iRet = xuiSetFocusWidget(pContext, pRestore);
	if ( iRet != XUI_OK && pRestore != NULL ) iRet = xuiSetFocusWidget(pContext, NULL);
	return (iFirstRet != XUI_OK) ? iFirstRet : iRet;
}

static int __xuiMenuBarCloseOpen(xui_widget pWidget, xui_menubar_data_t* pData)
{
	xui_context pContext;
	xui_widget pMenu;
	int iRet;

	if ( (pWidget == NULL) || (pData == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pData->iOpen < 0 && pData->iActive < 0 &&
	     !pData->tPointerOwner.bActive ) return XUI_OK;
	pContext = xuiWidgetGetContext(pWidget);
	iRet = xuiInternalPointerOwnerCancel(pContext, pWidget, &pData->tPointerOwner);
	if ( iRet != XUI_OK ) return iRet;
	if ( __xuiMenuBarGetData(pWidget) != pData ) return XUI_OK;
	if ( (pData->iOpen >= 0) && (pData->iOpen < pData->iItemCount) ) {
		pMenu = pData->arrItems[pData->iOpen].pMenu;
		if ( __xuiMenuBarMenuValid(pWidget, pMenu) && xuiMenuIsOpen(pMenu) ) {
			iRet = xuiMenuClose(pMenu);
			if ( iRet != XUI_OK ) return iRet;
			if ( __xuiMenuBarGetData(pWidget) != pData ) return XUI_OK;
		}
		iRet = __xuiMenuBarReleaseSelectBinding(pWidget, pData, pData->iOpen);
		if ( iRet != XUI_OK ) return iRet;
	}
	pData->iOpen = -1;
	pData->iActive = -1;
	__xuiMenuBarMarkChanged(pData);
	iRet = xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	if ( iRet == XUI_OK ) {
		xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_STATE_CHANGED);
		xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_SELECTION_CHANGED);
	}
	return iRet;
}

static int __xuiMenuBarOpenItemInternal(xui_widget pWidget, xui_menubar_data_t* pData, int iIndex)
{
	xui_menubar_resolved_t tResolved;
	xui_rect_t tAnchor;
	xui_context pContext;
	xui_widget pMenu;
	xui_widget pOldMenu;
	xui_widget pPopup;
	xui_menu_select_proc onSelect;
	void* pSelectUser;
	int iValue;
	int iOldHover;
	int iOldActive;
	int iOldOpen;
	int iOldChangeCount;
	int iRet;

	if ( (pWidget == NULL) || (pData == NULL) || (iIndex < 0) || (iIndex >= pData->iItemCount) ||
	     !__xuiMenuBarItemEnabled(&pData->arrItems[iIndex]) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pContext = xuiWidgetGetContext(pWidget);
	iRet = __xuiMenuBarSyncOpen(pWidget, pData);
	if ( iRet != XUI_OK ) return iRet;
	pMenu = pData->arrItems[iIndex].pMenu;
	if ( !__xuiMenuBarMenuValid(pWidget, pMenu) ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pData->iOpen == iIndex && pMenu != NULL && xuiMenuIsOpen(pMenu) ) return XUI_OK;
	__xuiMenuBarResolve(pWidget, pData, &tResolved);
	iRet = __xuiMenuBarLayoutItems(pWidget, pData, &tResolved, NULL);
	if ( iRet != XUI_OK ) return iRet;
	if ( (pData->iOpen >= 0) && (pData->iOpen < pData->iItemCount) && (pData->iOpen != iIndex) ) {
		pOldMenu = pData->arrItems[pData->iOpen].pMenu;
		if ( __xuiMenuBarMenuValid(pWidget, pOldMenu) && xuiMenuIsOpen(pOldMenu) ) {
			iRet = xuiMenuClose(pOldMenu);
			if ( iRet != XUI_OK ) return iRet;
			if ( __xuiMenuBarGetData(pWidget) != pData ) return XUI_OK;
		}
		iRet = __xuiMenuBarReleaseSelectBinding(pWidget, pData, pData->iOpen);
		if ( iRet != XUI_OK ) return iRet;
		pData->iOpen = -1;
		pData->iActive = -1;
	}
	iOldHover = pData->iHover;
	iOldActive = pData->iActive;
	iOldOpen = pData->iOpen;
	iOldChangeCount = pData->iChangeCount;
	pData->iHover = iIndex;
	pData->iActive = (pMenu != NULL) ? iIndex : -1;
	pData->iOpen = (pMenu != NULL) ? iIndex : -1;
	__xuiMenuBarMarkChanged(pData);
	iRet = xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	if ( iRet != XUI_OK ) {
		pData->iHover = iOldHover;
		pData->iActive = iOldActive;
		pData->iOpen = iOldOpen;
		pData->iChangeCount = iOldChangeCount;
		return iRet;
	}
	if ( pMenu != NULL ) {
		if ( pData->onSelect != NULL ) {
			iRet = __xuiMenuBarBindSelect(pWidget, pData, iIndex, pData->onSelect, pData->pSelectUser);
			if ( iRet != XUI_OK ) goto open_failed;
		} else {
			iRet = __xuiMenuBarReleaseSelectBinding(pWidget, pData, iIndex);
			if ( iRet != XUI_OK ) goto open_failed;
		}
		tAnchor = __xuiMenuBarItemAnchorRect(pWidget, pData, iIndex);
		iRet = xuiMenuOpenAt(pMenu, pWidget, tAnchor.fX, tAnchor.fY);
		if ( iRet != XUI_OK ) goto open_failed;
		if ( __xuiMenuBarGetData(pWidget) != pData || xuiInternalContextDestroyPending(pContext) ) return XUI_OK;
		pPopup = xuiMenuGetPopupWidget(pMenu);
		if ( pPopup != NULL ) {
			iRet = xuiPopupSetClosePolicy(pPopup, XUI_POPUP_OUTSIDE_CLOSE, XUI_POPUP_OWNER_CLOSE, XUI_POPUP_ESCAPE_CLOSE);
			if ( iRet == XUI_OK ) iRet = xuiPopupSetFocusRestore(pPopup, pWidget);
			if ( iRet != XUI_OK ) goto open_failed;
		}
		iRet = xuiSetFocusWidget(pContext, pWidget);
		if ( iRet != XUI_OK ) goto open_failed;
		if ( __xuiMenuBarGetData(pWidget) != pData || xuiInternalContextDestroyPending(pContext) ) return XUI_OK;
		xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_STATE_CHANGED);
		xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_SELECTION_CHANGED);
		return XUI_OK;
	} else {
		onSelect = pData->onSelect;
		pSelectUser = pData->pSelectUser;
		iValue = pData->arrItems[iIndex].iValue;
		xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_SELECTION_CHANGED);
		if ( onSelect != NULL ) onSelect(pWidget, iIndex, iValue, pSelectUser);
		return XUI_OK;
	}

open_failed:
	if ( xuiInternalMenuIsLive(pContext, pMenu) && xuiMenuIsOpen(pMenu) ) (void)xuiMenuClose(pMenu);
	if ( __xuiMenuBarGetData(pWidget) == pData ) {
		(void)__xuiMenuBarReleaseSelectBinding(pWidget, pData, iIndex);
		pData->iHover = iOldHover;
		pData->iActive = iOldActive;
		pData->iOpen = iOldOpen;
		pData->iChangeCount = iOldChangeCount;
		(void)xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	}
	return iRet;
}

static int __xuiMenuBarTrackItem(xui_widget pWidget, xui_menubar_data_t* pData,
	int iIndex)
{
	int iRet;

	if ( pWidget == NULL || pData == NULL || iIndex < 0 ||
	     iIndex >= pData->iItemCount ||
	     !__xuiMenuBarItemEnabled(&pData->arrItems[iIndex]) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( pData->arrItems[iIndex].pMenu != NULL ) {
		return __xuiMenuBarOpenItemInternal(pWidget, pData, iIndex);
	}
	if ( pData->iOpen >= 0 || pData->iActive >= 0 ) {
		iRet = __xuiMenuBarCloseOpen(pWidget, pData);
		if ( iRet != XUI_OK ) return iRet;
		if ( __xuiMenuBarGetData(pWidget) != pData ) return XUI_OK;
	}
	return __xuiMenuBarSetHover(pWidget, pData, iIndex);
}

static int __xuiMenuBarHotkeyEvent(xui_widget pWidget, const xui_event_t* pEvent, void* pUser)
{
	xui_menubar_data_t* pData;
	xui_context pContext;
	int iIndex;
	int iRet;

	(void)pUser;
	if ( (pWidget == NULL) || (pEvent == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	pData = __xuiMenuBarGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	pContext = xuiWidgetGetContext(pWidget);
	if ( !xuiWidgetIsAttachedToContext(pWidget) ||
	     !xuiWidgetGetEffectiveVisible(pWidget) ||
	     !xuiWidgetGetEffectiveEnabled(pWidget) ) return XUI_OK;
	if ( pEvent->iKey == XUI_KEY_F10 && pEvent->iModifiers == 0u ) {
		if ( pContext->pMenuBarModeOwner == pWidget ) {
			if ( pData->iOpen >= 0 ) {
				iRet = __xuiMenuBarCloseOpen(pWidget, pData);
				if ( iRet != XUI_OK ) return iRet;
				if ( __xuiMenuBarGetData(pWidget) != pData ) return XUI_EVENT_DISPATCH_STOP;
			}
			iRet = __xuiMenuBarModeExit(pWidget, pData);
			if ( iRet != XUI_OK ) return iRet;
			return XUI_EVENT_DISPATCH_STOP;
		}
		iRet = __xuiMenuBarModeEnter(pWidget, pData);
		if ( iRet != XUI_OK ) return iRet;
		iIndex = (pData->iHover >= 0) ? pData->iHover :
			__xuiMenuBarNextEnabled(pData, -1, 1);
		iRet = __xuiMenuBarSetHover(pWidget, pData, iIndex);
		if ( iRet != XUI_OK ) return iRet;
		return XUI_EVENT_DISPATCH_STOP;
	}
	if ( xuiGetFocusWidget(pContext) == pWidget ) {
		return XUI_OK;
	}
	iIndex = __xuiMenuBarMnemonicIndex(pData, pEvent->iKey);
	if ( iIndex >= 0 ) {
		iRet = __xuiMenuBarModeEnter(pWidget, pData);
		if ( iRet != XUI_OK ) return iRet;
		if ( __xuiMenuBarGetData(pWidget) != pData ) return XUI_EVENT_DISPATCH_STOP;
		iRet = __xuiMenuBarOpenItemInternal(pWidget, pData, iIndex);
		if ( iRet != XUI_OK ) return iRet;
		if ( __xuiMenuBarGetData(pWidget) == pData &&
		     pData->arrItems[iIndex].pMenu == NULL ) {
			iRet = __xuiMenuBarModeExit(pWidget, pData);
			if ( iRet != XUI_OK ) return iRet;
		}
		return XUI_EVENT_DISPATCH_STOP;
	}
	return XUI_OK;
}

static int __xuiMenuBarRegisterMnemonics(xui_widget pWidget, xui_menubar_data_t* pData)
{
	xui_context pContext;
	int i;
	int iMnemonic;
	int iRet;
	int j;

	if ( (pWidget == NULL) || (pData == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	pContext = xuiWidgetGetContext(pWidget);
	for ( i = 0; i < pData->iItemCount; i++ ) {
		iMnemonic = __xuiMenuBarItemMnemonic(&pData->arrItems[i]);
		for ( j = 0; j < i; j++ ) {
			if ( __xuiMenuBarItemMnemonic(&pData->arrItems[j]) == iMnemonic ) break;
		}
		if ( iMnemonic != 0 && j == i ) {
			iRet = xuiHotKeyRegister(pContext, pWidget, iMnemonic, XUI_MOD_ALT, __xuiMenuBarHotkeyEvent, NULL);
			if ( iRet != XUI_OK ) return iRet;
		}
	}
	return XUI_OK;
}

static int __xuiMenuBarResetMnemonics(xui_widget pWidget, xui_menubar_data_t* pData)
{
	xui_context pContext;
	int i;
	int iMnemonic;
	int iRet;
	int j;

	if ( (pWidget == NULL) || (pData == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	pContext = xuiWidgetGetContext(pWidget);
	for ( i = 0; i < pData->iItemCount; i++ ) {
		iMnemonic = __xuiMenuBarItemMnemonic(&pData->arrItems[i]);
		for ( j = 0; j < i; j++ ) {
			if ( __xuiMenuBarItemMnemonic(&pData->arrItems[j]) == iMnemonic ) break;
		}
		if ( iMnemonic != 0 && j == i ) {
			iRet = xuiHotKeyUnregister(pContext, pWidget, iMnemonic, XUI_MOD_ALT);
			if ( iRet != XUI_OK ) return iRet;
		}
	}
	return XUI_OK;
}

static int __xuiMenuBarDrawRectFill(xui_proxy pProxy, xui_draw_context pDraw, xui_rect_t tRect, uint32_t iColor)
{
	if ( __xuiMenuBarAlpha(iColor) == 0 ) return XUI_OK;
	return (pProxy->drawRectFill != NULL) ? pProxy->drawRectFill(pProxy, pDraw, tRect, iColor) : XUI_OK;
}

static int __xuiMenuBarDrawRectStroke(xui_proxy pProxy, xui_draw_context pDraw, xui_rect_t tRect, float fWidth, uint32_t iColor)
{
	if ( (fWidth <= 0.0f) || (__xuiMenuBarAlpha(iColor) == 0) ) return XUI_OK;
	return (pProxy->drawRectStroke != NULL) ? pProxy->drawRectStroke(pProxy, pDraw, tRect, fWidth, iColor) : XUI_OK;
}

static int __xuiMenuBarCacheRender(xui_widget pWidget, xui_draw_context pDraw, uint32_t iStateId, void* pUser)
{
	xui_menubar_data_t* pData;
	xui_menubar_resolved_t tResolved;
	xui_proxy pProxy;
	xui_rect_t tRect;
	xui_rect_t tItem;
	xui_rect_t tText;
	xui_rect_t tFocus;
	uint32_t iBack;
	uint32_t iText;
	int i;
	int iRet;
	int bHot;

	(void)iStateId;
	(void)pUser;
	pData = __xuiMenuBarGetData(pWidget);
	if ( (pData == NULL) || (pDraw == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	iRet = __xuiMenuBarSyncOpen(pWidget, pData);
	if ( iRet != XUI_OK ) return iRet;
	__xuiMenuBarResolve(pWidget, pData, &tResolved);
	iRet = __xuiMenuBarLayoutItems(pWidget, pData, &tResolved, NULL);
	if ( iRet != XUI_OK ) return iRet;
	pProxy = xuiInternalContextGetProxy(xuiWidgetGetContext(pWidget));
	if ( pProxy == NULL ) return XUI_ERROR_NOT_INITIALIZED;
	tRect = xuiWidgetGetRect(pWidget);
	tRect.fX = 0.0f;
	tRect.fY = 0.0f;
	iRet = __xuiMenuBarDrawRectFill(pProxy, pDraw, tRect, tResolved.tColors.iBackgroundColor);
	if ( iRet != XUI_OK ) return iRet;
	if ( tResolved.tMetrics.fBorderWidth > 0.0f ) {
		iRet = __xuiMenuBarDrawRectStroke(pProxy, pDraw, tRect, tResolved.tMetrics.fBorderWidth, tResolved.tColors.iBorderColor);
		if ( iRet != XUI_OK ) return iRet;
	}
	for ( i = 0; i < pData->iItemCount; i++ ) {
		tItem = pData->arrItems[i].tRect;
		bHot = (i == pData->iHover) || (i == pData->iOpen) || (i == pData->iActive);
		iBack = tResolved.tColors.iItemColor;
		iText = tResolved.tColors.iTextColor;
		if ( !__xuiMenuBarItemEnabled(&pData->arrItems[i]) ) {
			iBack = XUI_COLOR_RGBA(0, 0, 0, 0);
			iText = tResolved.tColors.iDisabledTextColor;
		} else if ( i == pData->iOpen || i == pData->iActive ) {
			iBack = tResolved.tColors.iActiveColor;
			iText = tResolved.iActiveTextColor;
		} else if ( bHot ) {
			iBack = tResolved.tColors.iHoverColor;
		}
		iRet = __xuiMenuBarDrawRectFill(pProxy, pDraw, tItem, iBack);
		if ( iRet != XUI_OK ) return iRet;
		if ( (xuiGetFocusWidget(xuiWidgetGetContext(pWidget)) == pWidget) && (pData->iOpen < 0) &&
		     (i == pData->iHover) && __xuiMenuBarItemEnabled(&pData->arrItems[i]) ) {
			tFocus = tItem;
			tFocus.fX += 4.0f;
			tFocus.fY = tItem.fY + tItem.fH - 3.0f;
			tFocus.fW -= 8.0f;
			tFocus.fH = 2.0f;
			if ( tFocus.fW > 0.0f ) {
				iRet = __xuiMenuBarDrawRectFill(pProxy, pDraw, tFocus, tResolved.tColors.iFocusColor);
				if ( iRet != XUI_OK ) return iRet;
			}
		}
		tText = xuiInternalRectFromFloatNearest(tItem.fX + tResolved.tMetrics.fItemPaddingX,
			tItem.fY, tItem.fW - tResolved.tMetrics.fItemPaddingX * 2.0f, tItem.fH);
		if ( (tResolved.pFont != NULL) && (pProxy->drawText != NULL) && (tText.fW > 0.0f) ) {
			iRet = pProxy->drawText(pProxy, pDraw, &(xui_text_item_t){.iSize=sizeof(xui_text_item_t), .pFont=tResolved.pFont, .sText=pData->arrDisplayText[i], .iTextSize=-1, .iFlags=XUI_TEXT_SHAPE_DEFAULT | ((XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP) & XUI_TEXT_RTL ? XUI_TEXT_SHAPE_RTL : 0)}, tText, iText, XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP);
			if ( iRet != XUI_OK ) return iRet;
		}
	}
	return XUI_OK;
}

static int __xuiMenuBarContentMeasure(xui_widget pWidget, xui_vec2_t tConstraint, xui_vec2_t* pSize, void* pUser)
{
	xui_menubar_data_t* pData;
	xui_menubar_resolved_t tResolved;
	int iRet;

	(void)pUser;
	if ( (pWidget == NULL) || (pSize == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	pData = __xuiMenuBarGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	__xuiMenuBarResolve(pWidget, pData, &tResolved);
	iRet = __xuiMenuBarLayoutItems(pWidget, pData, &tResolved, pSize);
	if ( iRet != XUI_OK ) return iRet;
	if ( (tConstraint.fX >= 0.0f) &&
	     (tConstraint.fX < XUI_LAYOUT_UNBOUNDED) &&
	     (pSize->fX > tConstraint.fX) ) pSize->fX = tConstraint.fX;
	if ( (tConstraint.fY >= 0.0f) &&
	     (tConstraint.fY < XUI_LAYOUT_UNBOUNDED) &&
	     (pSize->fY > tConstraint.fY) ) pSize->fY = tConstraint.fY;
	return XUI_OK;
}

static int __xuiMenuBarUpdate(xui_widget pWidget, float fDelta, void* pUser)
{
	xui_menubar_data_t* pData;
	xui_context pContext;
	xui_rect_t tWorld;
	int iIndex;
	int iRet;

	(void)pUser;
	if ( !isfinite(fDelta) || fDelta < 0.0f ) return XUI_ERROR_INVALID_ARGUMENT;
	pData = __xuiMenuBarGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	pContext = xuiWidgetGetContext(pWidget);
	if ( !xuiWidgetGetEffectiveVisible(pWidget) || !xuiWidgetGetEffectiveEnabled(pWidget) ) {
		if ( pData->iOpen >= 0 || pData->iActive >= 0 || pData->tPointerOwner.bActive ) {
			iRet = __xuiMenuBarCloseOpen(pWidget, pData);
			if ( iRet != XUI_OK ) return iRet;
		}
		return (pContext->pMenuBarModeOwner == pWidget) ?
			__xuiMenuBarModeExit(pWidget, pData) : XUI_OK;
	}
	iRet = __xuiMenuBarSyncOpen(pWidget, pData);
	if ( iRet != XUI_OK ) return iRet;
	if ( pData->iOpen >= 0 || pContext->pMenuBarModeOwner == pWidget ) {
		tWorld = xuiWidgetGetWorldRect(pWidget);
		iIndex = __xuiMenuBarIndexAt(pData, pContext->fPointerX - tWorld.fX, pContext->fPointerY - tWorld.fY);
		if ( iIndex >= 0 &&
		     (iIndex != pData->iHover ||
		      (pData->iOpen >= 0 && iIndex != pData->iOpen)) ) {
			return __xuiMenuBarTrackItem(pWidget, pData, iIndex);
		}
	}
	return XUI_OK;
}

static int __xuiMenuBarEvent(xui_widget pWidget, const xui_event_t* pEvent, void* pUser)
{
	xui_menubar_data_t* pData;
	xui_menubar_resolved_t tResolved;
	xui_rect_t tWorld;
	xui_context pContext;
	xui_widget pMenu;
	const xui_menu_item_t* pMenuItem;
	float fLocalX;
	float fLocalY;
	int iIndex;
	int iNext;
	int iRet;

	(void)pUser;
	if ( (pWidget == NULL) || (pEvent == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pEvent->iPhase == XUI_EVENT_PHASE_CAPTURE ) return XUI_OK;
	pData = __xuiMenuBarGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	pContext = xuiWidgetGetContext(pWidget);
	iRet = __xuiMenuBarSyncOpen(pWidget, pData);
	if ( iRet != XUI_OK ) return iRet;
	__xuiMenuBarResolve(pWidget, pData, &tResolved);
	iRet = __xuiMenuBarLayoutItems(pWidget, pData, &tResolved, NULL);
	if ( iRet != XUI_OK ) return iRet;
	tWorld = xuiWidgetGetWorldRect(pWidget);
	fLocalX = pEvent->fX - tWorld.fX;
	fLocalY = pEvent->fY - tWorld.fY;
	iIndex = __xuiMenuBarIndexAt(pData, fLocalX, fLocalY);
	switch ( pEvent->iType ) {
	case XUI_EVENT_POINTER_MOVE:
		if ( pData->tPointerOwner.bActive &&
		     !xuiInternalPointerOwnerMatches(&pData->tPointerOwner, pEvent) ) {
			return XUI_OK;
		}
		iRet = __xuiMenuBarSetHover(pWidget, pData, iIndex);
		if ( iRet != XUI_OK ) return iRet;
		if ( iIndex >= 0 &&
		     (pData->iOpen >= 0 || pContext->pMenuBarModeOwner == pWidget) &&
		     iIndex != pData->iOpen ) {
			iRet = __xuiMenuBarTrackItem(pWidget, pData, iIndex);
			if ( iRet != XUI_OK ) return iRet;
			return XUI_EVENT_DISPATCH_STOP;
		}
		return (pData->iOpen >= 0 || pContext->pMenuBarModeOwner == pWidget) ?
			XUI_EVENT_DISPATCH_STOP : XUI_OK;
	case XUI_EVENT_POINTER_LEAVE:
		if ( pData->iOpen < 0 && pContext->pMenuBarModeOwner != pWidget ) {
			iRet = __xuiMenuBarSetHover(pWidget, pData, -1);
			if ( iRet != XUI_OK ) return iRet;
		}
		return XUI_OK;
	case XUI_EVENT_POINTER_DOWN:
		if ( pEvent->iButton != 0 && pEvent->iButton != XUI_POINTER_BUTTON_LEFT ) return XUI_OK;
		if ( pData->tPointerOwner.bActive ) {
			return xuiInternalPointerOwnerMatches(&pData->tPointerOwner, pEvent) ?
				XUI_EVENT_DISPATCH_STOP : XUI_OK;
		}
		if ( iIndex < 0 ) {
			if ( pData->iOpen >= 0 ) {
				iRet = __xuiMenuBarCloseOpen(pWidget, pData);
				if ( iRet != XUI_OK ) return iRet;
				if ( __xuiMenuBarGetData(pWidget) == pData ) {
					iRet = __xuiMenuBarModeExit(pWidget, pData);
					if ( iRet != XUI_OK ) return iRet;
				}
				return XUI_EVENT_DISPATCH_STOP;
			}
			return XUI_OK;
		}
		iRet = __xuiMenuBarModeEnter(pWidget, pData);
		if ( iRet != XUI_OK ) return iRet;
		if ( __xuiMenuBarGetData(pWidget) != pData ) return XUI_EVENT_DISPATCH_STOP;
		if ( (pData->iOpen == iIndex) && (pData->arrItems[iIndex].pMenu != NULL) && xuiMenuIsOpen(pData->arrItems[iIndex].pMenu) ) {
			iRet = __xuiMenuBarCloseOpen(pWidget, pData);
			if ( iRet != XUI_OK ) return iRet;
			if ( __xuiMenuBarGetData(pWidget) == pData ) {
				iRet = __xuiMenuBarModeExit(pWidget, pData);
				if ( iRet != XUI_OK ) return iRet;
			}
			return XUI_EVENT_DISPATCH_STOP;
		}
		if ( pData->arrItems[iIndex].pMenu == NULL ) {
			if ( pData->iOpen >= 0 ) {
				iRet = __xuiMenuBarCloseOpen(pWidget, pData);
				if ( iRet != XUI_OK ) return iRet;
				if ( __xuiMenuBarGetData(pWidget) != pData ) return XUI_EVENT_DISPATCH_STOP;
			}
			iRet = xuiInternalPointerOwnerBegin(xuiWidgetGetContext(pWidget),
				pWidget, pEvent, &pData->tPointerOwner);
			if ( iRet != XUI_OK ) {
				(void)__xuiMenuBarModeExit(pWidget, pData);
				return iRet;
			}
			pData->iActive = iIndex;
			iNext = pData->iChangeCount;
			__xuiMenuBarMarkChanged(pData);
			iRet = xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
			if ( iRet != XUI_OK ) {
				pData->iActive = -1;
				pData->iChangeCount = iNext;
				(void)xuiInternalPointerOwnerCancel(xuiWidgetGetContext(pWidget),
					pWidget, &pData->tPointerOwner);
				(void)__xuiMenuBarModeExit(pWidget, pData);
				return iRet;
			}
			return XUI_EVENT_DISPATCH_STOP;
		}
		iRet = __xuiMenuBarOpenItemInternal(pWidget, pData, iIndex);
		if ( iRet != XUI_OK ) {
			(void)__xuiMenuBarModeExit(pWidget, pData);
			return iRet;
		}
		return XUI_EVENT_DISPATCH_STOP;
	case XUI_EVENT_POINTER_UP:
		if ( pData->iActive >= 0 && pData->iOpen < 0 &&
		     xuiInternalPointerOwnerMatches(&pData->tPointerOwner, pEvent) ) {
			int iPressed = pData->iActive;
			pData->iActive = -1;
			iRet = xuiInternalPointerOwnerEnd(xuiWidgetGetContext(pWidget),
				pWidget, pEvent, &pData->tPointerOwner);
			if ( iRet != XUI_OK ) return iRet;
			if ( __xuiMenuBarGetData(pWidget) != pData ) return XUI_EVENT_DISPATCH_STOP;
			__xuiMenuBarMarkChanged(pData);
			iRet = xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
			if ( iRet != XUI_OK ) return iRet;
			if ( iIndex == iPressed ) {
				iRet = __xuiMenuBarOpenItemInternal(pWidget, pData, iPressed);
				if ( iRet != XUI_OK ) return iRet;
			}
			if ( __xuiMenuBarGetData(pWidget) == pData ) {
				iRet = __xuiMenuBarModeExit(pWidget, pData);
				if ( iRet != XUI_OK ) return iRet;
			}
			return XUI_EVENT_DISPATCH_STOP;
		}
		return XUI_OK;
	case XUI_EVENT_POINTER_CAPTURE_LOST:
		if ( pData->iOpen < 0 && pData->iActive >= 0 &&
		     xuiInternalPointerOwnerCaptureLost(&pData->tPointerOwner, pEvent) ) {
			pData->iActive = -1;
			__xuiMenuBarMarkChanged(pData);
			iRet = xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
			if ( iRet != XUI_OK ) return iRet;
			return __xuiMenuBarModeExit(pWidget, pData);
		}
		return XUI_OK;
	case XUI_EVENT_KEY_DOWN:
		if ( (pEvent->iModifiers & XUI_MOD_ALT) != 0u ) {
			iIndex = __xuiMenuBarMnemonicIndex(pData, pEvent->iKey);
			if ( iIndex >= 0 ) {
				iRet = __xuiMenuBarModeEnter(pWidget, pData);
				if ( iRet != XUI_OK ) return iRet;
				if ( __xuiMenuBarGetData(pWidget) != pData ) return XUI_EVENT_DISPATCH_STOP;
				iRet = __xuiMenuBarOpenItemInternal(pWidget, pData, iIndex);
				if ( iRet != XUI_OK ) return iRet;
				if ( __xuiMenuBarGetData(pWidget) == pData &&
				     pData->arrItems[iIndex].pMenu == NULL ) {
					iRet = __xuiMenuBarModeExit(pWidget, pData);
					if ( iRet != XUI_OK ) return iRet;
				}
				return XUI_EVENT_DISPATCH_STOP;
			}
		}
		if ( xuiGetFocusWidget(xuiWidgetGetContext(pWidget)) != pWidget && pData->iOpen < 0 ) return XUI_OK;
		if ( pEvent->iKey == XUI_KEY_F10 && pEvent->iModifiers == 0u ) {
			if ( pContext->pMenuBarModeOwner == pWidget ) {
				if ( pData->iOpen >= 0 ) {
					iRet = __xuiMenuBarCloseOpen(pWidget, pData);
					if ( iRet != XUI_OK ) return iRet;
					if ( __xuiMenuBarGetData(pWidget) != pData ) return XUI_EVENT_DISPATCH_STOP;
				}
				iRet = __xuiMenuBarModeExit(pWidget, pData);
				if ( iRet != XUI_OK ) return iRet;
				return XUI_EVENT_DISPATCH_STOP;
			}
			iRet = __xuiMenuBarModeEnter(pWidget, pData);
			if ( iRet != XUI_OK ) return iRet;
			if ( __xuiMenuBarGetData(pWidget) != pData ) return XUI_EVENT_DISPATCH_STOP;
			iIndex = (pData->iHover >= 0) ? pData->iHover : __xuiMenuBarNextEnabled(pData, -1, 1);
			iRet = __xuiMenuBarSetHover(pWidget, pData, iIndex);
			if ( iRet != XUI_OK ) return iRet;
			return XUI_EVENT_DISPATCH_STOP;
		}
		if ( pEvent->iKey == XUI_KEY_LEFT || pEvent->iKey == XUI_KEY_RIGHT ) {
			if ( pEvent->iKey == XUI_KEY_RIGHT && pData->iOpen >= 0 ) {
				pMenu = pData->arrItems[pData->iOpen].pMenu;
				iIndex = (pMenu != NULL) ? xuiMenuGetHoverIndex(pMenu) : -1;
				pMenuItem = (pMenu != NULL && iIndex >= 0) ? xuiMenuGetItem(pMenu, iIndex) : NULL;
				if ( pMenuItem != NULL && pMenuItem->iType == XUI_MENU_ITEM_SUBMENU &&
				     (pMenuItem->iState & XUI_MENU_ITEM_ENABLED) != 0u ) {
					iRet = xuiMenuCommitHover(pMenu);
					if ( iRet != XUI_OK && iRet != XUI_EVENT_DISPATCH_STOP ) return iRet;
					return XUI_EVENT_DISPATCH_STOP;
				}
			}
			iNext = __xuiMenuBarNextEnabled(pData, (pData->iHover >= 0) ? pData->iHover : pData->iOpen, (pEvent->iKey == XUI_KEY_RIGHT) ? 1 : -1);
			if ( iNext >= 0 ) {
				iRet = (pData->iOpen >= 0 || pContext->pMenuBarModeOwner == pWidget) ?
					__xuiMenuBarTrackItem(pWidget, pData, iNext) :
					__xuiMenuBarSetHover(pWidget, pData, iNext);
				if ( iRet != XUI_OK ) return iRet;
				return XUI_EVENT_DISPATCH_STOP;
			}
		}
		if ( pEvent->iKey == XUI_KEY_HOME || pEvent->iKey == XUI_KEY_END ) {
			if ( pData->iOpen >= 0 ) {
				pMenu = pData->arrItems[pData->iOpen].pMenu;
				iNext = __xuiMenuBarMenuNextEnabled(pMenu,
					(pEvent->iKey == XUI_KEY_HOME) ? -1 : 0,
					(pEvent->iKey == XUI_KEY_HOME) ? 1 : -1);
				if ( iNext >= 0 ) {
					iRet = xuiMenuSetHoverIndex(pMenu, iNext);
					if ( iRet != XUI_OK ) return iRet;
				}
			} else {
				iNext = __xuiMenuBarNextEnabled(pData,
					(pEvent->iKey == XUI_KEY_HOME) ? -1 : 0,
					(pEvent->iKey == XUI_KEY_HOME) ? 1 : -1);
				if ( iNext >= 0 ) {
					iRet = __xuiMenuBarSetHover(pWidget, pData, iNext);
					if ( iRet != XUI_OK ) return iRet;
				}
			}
			return XUI_EVENT_DISPATCH_STOP;
		}
		if ( pEvent->iKey == XUI_KEY_DOWN || pEvent->iKey == XUI_KEY_UP ) {
			if ( pData->iOpen >= 0 ) {
				pMenu = pData->arrItems[pData->iOpen].pMenu;
				iNext = __xuiMenuBarMenuNextEnabled(pMenu, xuiMenuGetHoverIndex(pMenu), (pEvent->iKey == XUI_KEY_DOWN) ? 1 : -1);
				if ( iNext >= 0 ) {
					iRet = xuiMenuSetHoverIndex(pMenu, iNext);
					if ( iRet != XUI_OK ) return iRet;
				}
			} else {
				iIndex = (pData->iHover >= 0) ? pData->iHover : __xuiMenuBarNextEnabled(pData, -1, 1);
				if ( iIndex >= 0 && pData->arrItems[iIndex].pMenu != NULL ) {
					iRet = __xuiMenuBarOpenItemInternal(pWidget, pData, iIndex);
					if ( iRet != XUI_OK ) return iRet;
				}
			}
			return XUI_EVENT_DISPATCH_STOP;
		}
		if ( pEvent->iKey == XUI_KEY_ENTER || pEvent->iKey == XUI_KEY_SPACE ) {
			if ( pData->iOpen >= 0 ) {
				pMenu = pData->arrItems[pData->iOpen].pMenu;
				if ( pMenu != NULL ) {
					iRet = xuiMenuCommitHover(pMenu);
					if ( iRet != XUI_OK && iRet != XUI_EVENT_DISPATCH_STOP ) return iRet;
				}
			} else {
				iIndex = (pData->iHover >= 0) ? pData->iHover : __xuiMenuBarNextEnabled(pData, -1, 1);
				if ( iIndex >= 0 ) {
					iRet = __xuiMenuBarOpenItemInternal(pWidget, pData, iIndex);
					if ( iRet != XUI_OK ) return iRet;
					if ( __xuiMenuBarGetData(pWidget) == pData &&
					     pData->arrItems[iIndex].pMenu == NULL ) {
						iRet = __xuiMenuBarModeExit(pWidget, pData);
						if ( iRet != XUI_OK ) return iRet;
					}
				}
			}
			return XUI_EVENT_DISPATCH_STOP;
		}
		if ( pEvent->iKey == XUI_KEY_ESCAPE ) {
			if ( pData->iOpen >= 0 ) {
				iRet = __xuiMenuBarCloseOpen(pWidget, pData);
				if ( iRet != XUI_OK ) return iRet;
				if ( __xuiMenuBarGetData(pWidget) != pData ) return XUI_EVENT_DISPATCH_STOP;
			}
			if ( pContext->pMenuBarModeOwner == pWidget ) {
				iRet = __xuiMenuBarModeExit(pWidget, pData);
				if ( iRet != XUI_OK ) return iRet;
			} else {
				iRet = __xuiMenuBarSetHover(pWidget, pData, -1);
				if ( iRet == XUI_OK && xuiGetFocusWidget(pContext) == pWidget ) {
					iRet = xuiSetFocusWidget(pContext, NULL);
				}
				if ( iRet != XUI_OK ) return iRet;
			}
			return XUI_EVENT_DISPATCH_STOP;
		}
		break;
	case XUI_EVENT_BLUR:
		if ( pData->iOpen < 0 && pContext->pMenuBarModeOwner == pWidget &&
		     xuiGetFocusWidget(pContext) != pWidget ) {
			if ( __xuiMenuBarGetData(xuiGetFocusWidget(pContext)) == NULL ) {
				__xuiMenuBarModeAbandon(pWidget);
				iRet = __xuiMenuBarSetHover(pWidget, pData, -1);
				if ( iRet != XUI_OK ) return iRet;
			}
		}
		/* fall through */
	case XUI_EVENT_FOCUS:
		if ( pEvent->iType == XUI_EVENT_FOCUS &&
		     pContext->pMenuBarModeOwner != NULL &&
		     pContext->pMenuBarModeOwner != pWidget ) {
			iRet = __xuiMenuBarModeEnter(pWidget, pData);
			if ( iRet != XUI_OK ) return iRet;
		}
		iRet = xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
		if ( iRet != XUI_OK ) return iRet;
		break;
	default:
		break;
	}
	return XUI_OK;
}

static void __xuiMenuBarDefaultLayout(xui_layout_t* pLayout)
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

static void __xuiMenuBarDefaultCachePolicy(xui_cache_policy_t* pPolicy)
{
	memset(pPolicy, 0, sizeof(*pPolicy));
	pPolicy->iSize = sizeof(*pPolicy);
	pPolicy->iPolicy = XUI_CACHE_POLICY_SELF;
	pPolicy->iFlags = XUI_CACHE_CLEAR_ON_UPDATE | XUI_CACHE_UPDATE_ALL_STATES;
	pPolicy->iClearColor = XUI_COLOR_RGBA(0, 0, 0, 0);
}

static int __xuiMenuBarInitEvents(xui_widget pWidget)
{
	int iRet;

	iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_POINTER_MOVE, __xuiMenuBarEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_POINTER_LEAVE, __xuiMenuBarEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_POINTER_DOWN, __xuiMenuBarEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_POINTER_UP, __xuiMenuBarEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_POINTER_CAPTURE_LOST, __xuiMenuBarEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_KEY_DOWN, __xuiMenuBarEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_FOCUS, __xuiMenuBarEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_BLUR, __xuiMenuBarEvent, NULL);
	return iRet;
}

static int __xuiMenuBarAccessibleCount(xui_widget pWidget, void* pUser)
{
	xui_menubar_data_t* pData = (xui_menubar_data_t*)pUser;
	(void)pWidget;
	return (pData != NULL) ? pData->iItemCount + 1 : 0;
}

static int __xuiMenuBarAccessibleGet(xui_widget pWidget, int iIndex,
	xui_accessible_node_t* pNode, void* pUser)
{
	xui_menubar_data_t* pData = (xui_menubar_data_t*)pUser;
	xui_menubar_resolved_t tResolved;
	xui_menubar_item_t* pItem;
	xui_rect_t tWorld;
	xui_rect_t tItem;
	int iItem;
	int iRet;

	if ( pData == NULL || pNode == NULL || iIndex < 0 || iIndex > pData->iItemCount ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	__xuiMenuBarResolve(pWidget, pData, &tResolved);
	iRet = __xuiMenuBarLayoutItems(pWidget, pData, &tResolved, NULL);
	if ( iRet != XUI_OK ) return iRet;
	pNode->iId = (uint64_t)iIndex + 1u;
	if ( iIndex == 0 ) {
		pNode->iRole = XUI_ACCESSIBLE_ROLE_MENU;
		pNode->sName = xuiWidgetGetAccessibleName(pWidget) != NULL ?
			xuiWidgetGetAccessibleName(pWidget) : "Menu bar";
		pNode->sDescription = xuiWidgetGetAccessibleDescription(pWidget);
		pNode->tBounds = xuiWidgetGetWorldRect(pWidget);
		pNode->iState |= pData->iOpen >= 0 ? XUI_ACCESSIBLE_STATE_EXPANDED : XUI_ACCESSIBLE_STATE_COLLAPSED;
		pNode->iRowCount = 1;
		pNode->iColumnCount = pData->iItemCount;
		pNode->iActions = XUI_ACCESSIBLE_ACTION_MASK(XUI_ACCESSIBLE_ACTION_FOCUS);
		return XUI_OK;
	}
	iItem = iIndex - 1;
	pItem = &pData->arrItems[iItem];
	pNode->iParentId = 1;
	pNode->iRole = XUI_ACCESSIBLE_ROLE_MENU_ITEM;
	pNode->sName = pData->arrDisplayText[iItem] != NULL ? pData->arrDisplayText[iItem] : "";
	pNode->iRow = 0;
	pNode->iColumn = iItem;
	pNode->iRowCount = 1;
	pNode->iColumnCount = 1;
	tWorld = xuiWidgetGetWorldRect(pWidget);
	tItem = pItem->tRect;
	pNode->tBounds = (xui_rect_t){tWorld.fX + tItem.fX, tWorld.fY + tItem.fY, tItem.fW, tItem.fH};
	pNode->iState |= XUI_ACCESSIBLE_STATE_SELECTABLE;
	if ( !__xuiMenuBarItemEnabled(pItem) ) pNode->iState |= XUI_ACCESSIBLE_STATE_DISABLED;
	if ( iItem == pData->iHover ) pNode->iState |= XUI_ACCESSIBLE_STATE_SELECTED;
	if ( pItem->pMenu != NULL ) {
		pNode->iState |= (iItem == pData->iOpen && __xuiMenuBarMenuValid(pWidget, pItem->pMenu) &&
			xuiMenuIsOpen(pItem->pMenu)) ? XUI_ACCESSIBLE_STATE_EXPANDED : XUI_ACCESSIBLE_STATE_COLLAPSED;
		pNode->iActions = (pNode->iState & XUI_ACCESSIBLE_STATE_EXPANDED) != 0u ?
			XUI_ACCESSIBLE_ACTION_MASK(XUI_ACCESSIBLE_ACTION_COLLAPSE) :
			XUI_ACCESSIBLE_ACTION_MASK(XUI_ACCESSIBLE_ACTION_EXPAND);
	} else {
		pNode->iActions = XUI_ACCESSIBLE_ACTION_MASK(XUI_ACCESSIBLE_ACTION_ACTIVATE);
	}
	pNode->iActions |= XUI_ACCESSIBLE_ACTION_MASK(XUI_ACCESSIBLE_ACTION_FOCUS);
	return XUI_OK;
}

static int __xuiMenuBarAccessibleAction(xui_widget pWidget, uint64_t iNodeId,
	int iAction, const void* pPayload, void* pUser)
{
	xui_menubar_data_t* pData = (xui_menubar_data_t*)pUser;
	int iIndex;
	int iRet;

	(void)pPayload;
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( iNodeId == 0 || iNodeId == 1 ) {
		return iAction == XUI_ACCESSIBLE_ACTION_FOCUS ?
			xuiSetFocusWidget(xuiWidgetGetContext(pWidget), pWidget) : XUI_ERROR_UNSUPPORTED;
	}
	if ( iNodeId < 2 || iNodeId > (uint64_t)pData->iItemCount + 1u ) return XUI_ERROR_NOT_FOUND;
	iIndex = (int)iNodeId - 2;
	if ( !__xuiMenuBarItemEnabled(&pData->arrItems[iIndex]) ) return XUI_ERROR_INVALID_STATE;
	if ( iAction == XUI_ACCESSIBLE_ACTION_FOCUS ) {
		iRet = __xuiMenuBarSetHover(pWidget, pData, iIndex);
		if ( iRet != XUI_OK ) return iRet;
		return xuiSetFocusWidget(xuiWidgetGetContext(pWidget), pWidget);
	}
	if ( iAction == XUI_ACCESSIBLE_ACTION_COLLAPSE ) {
		if ( pData->arrItems[iIndex].pMenu == NULL ) return XUI_ERROR_UNSUPPORTED;
		if ( pData->iOpen != iIndex || !xuiMenuIsOpen(pData->arrItems[iIndex].pMenu) ) {
			return XUI_ERROR_INVALID_STATE;
		}
		return __xuiMenuBarCloseOpen(pWidget, pData);
	}
	if ( iAction == XUI_ACCESSIBLE_ACTION_EXPAND ) {
		if ( pData->arrItems[iIndex].pMenu == NULL ) return XUI_ERROR_UNSUPPORTED;
		return __xuiMenuBarOpenItemInternal(pWidget, pData, iIndex);
	}
	if ( iAction == XUI_ACCESSIBLE_ACTION_ACTIVATE ) {
		return __xuiMenuBarOpenItemInternal(pWidget, pData, iIndex);
	}
	return XUI_ERROR_UNSUPPORTED;
}

static int __xuiMenuBarInit(xui_widget pWidget, void* pTypeData, const void* pCreateData, void* pUser)
{
	xui_menubar_data_t* pData;
	const xui_menubar_desc_t* pDesc;
	int iRet;

	(void)pUser;
	if ( (pWidget == NULL) || (pTypeData == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	pDesc = (const xui_menubar_desc_t*)pCreateData;
	if ( !__xuiMenuBarDescValid(pDesc) ) return XUI_ERROR_INVALID_ARGUMENT;
	pData = (xui_menubar_data_t*)pTypeData;
	__xuiMenuBarDefaultMetrics(&pData->tMetrics);
	__xuiMenuBarDefaultColors(&pData->tColors);
	pData->pFont = (pDesc != NULL && pDesc->pFont != NULL) ? pDesc->pFont : xuiGetDefaultFont(xuiWidgetGetContext(pWidget));
	pData->iHover = -1;
	pData->iActive = -1;
	pData->iOpen = -1;
	pData->bLayoutDirty = 1;
	if ( pDesc != NULL && pDesc->bHasMetrics && __xuiMenuBarMetricsValid(&pDesc->tMetrics) ) {
		pData->tMetrics = pDesc->tMetrics;
		pData->tMetrics.iSize = sizeof(pData->tMetrics);
	}
	if ( pDesc != NULL && pDesc->bHasColors && __xuiMenuBarColorsValid(&pDesc->tColors) ) {
		pData->tColors = pDesc->tColors;
		pData->tColors.iSize = sizeof(pData->tColors);
	}
	iRet = xuiWidgetSetFocusable(pWidget, 1);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetTabStop(pWidget, 0);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetOverflow(pWidget, XUI_OVERFLOW_VISIBLE);
	if ( iRet == XUI_OK ) iRet = __xuiMenuBarInitEvents(pWidget);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetAccessibilityProvider(pWidget,
		__xuiMenuBarAccessibleCount, __xuiMenuBarAccessibleGet,
		__xuiMenuBarAccessibleAction, pData);
	if ( iRet == XUI_OK ) iRet = xuiHotKeyRegister(xuiWidgetGetContext(pWidget),
		pWidget, XUI_KEY_F10, 0u, __xuiMenuBarHotkeyEvent, NULL);
	if ( iRet != XUI_OK ) return iRet;
	pData->pWidget = pWidget;
	pData->pNext = pWidget->pContext->pMenuBarList;
	pWidget->pContext->pMenuBarList = pData;
	return XUI_OK;
}

static void __xuiMenuBarDestroy(xui_widget pWidget, void* pTypeData, void* pUser)
{
	xui_menubar_data_t* pData;
	xui_menubar_data_t** ppScan;
	xui_context pContext;

	(void)pUser;
	pData = (xui_menubar_data_t*)pTypeData;
	if ( (pWidget != NULL) && (pData != NULL) ) {
		pContext = pWidget->pContext;
		for ( ppScan = &pContext->pMenuBarList; *ppScan != NULL; ppScan = &(*ppScan)->pNext ) {
			if ( *ppScan == pData ) {
				*ppScan = pData->pNext;
				break;
			}
		}
		pData->pNext = NULL;
		(void)__xuiMenuBarResetMnemonics(pWidget, pData);
		(void)__xuiMenuBarCloseOpen(pWidget, pData);
	}
	if ( pData != NULL ) __xuiMenuBarFreeItems(pData->arrItems, pData->arrDisplayText, pData->iItemCount);
	if ( pData != NULL ) memset(pData, 0, sizeof(*pData));
}

static xui_menubar_data_t* __xuiMenuBarGetData(xui_widget pWidget)
{
	xui_context pContext;
	xui_widget_type pType;

	if ( !xuiInternalWidgetIsValid(pWidget) ) return NULL;
	pContext = xuiWidgetGetContext(pWidget);
	if ( !xuiInternalContextIsValid(pContext) ) return NULL;
	pType = xuiWidgetFindType(pContext, "menubar");
	if ( pType == NULL ) pType = xuiWidgetFindType(pContext, "menuBar");
	if ( (pType == NULL) || !xuiWidgetIsType(pWidget, pType) ) return NULL;
	return (xui_menubar_data_t*)xuiWidgetGetTypeData(pWidget);
}

static void __xuiMenuBarRegisterStyleProperty(xui_context pContext, xui_widget_type pType, const char* sName, int iValueType, uint32_t iDirtyFlags, uint32_t iFlags)
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

static void __xuiMenuBarRegisterStyleProperties(xui_context pContext, xui_widget_type pType)
{
	uint32_t iPaintDirty;
	uint32_t iLayoutDirty;

	iPaintDirty = XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER;
	iLayoutDirty = XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER;
	__xuiMenuBarRegisterStyleProperty(pContext, pType, "font.name", XUI_STYLE_VALUE_STRING, iLayoutDirty, XUI_STYLE_PROPERTY_INHERITED);
	__xuiMenuBarRegisterStyleProperty(pContext, pType, "menubar.background.color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiMenuBarRegisterStyleProperty(pContext, pType, "menubar.border.color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiMenuBarRegisterStyleProperty(pContext, pType, "menubar.item.color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiMenuBarRegisterStyleProperty(pContext, pType, "menubar.item.hover_color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiMenuBarRegisterStyleProperty(pContext, pType, "menubar.item.active_color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiMenuBarRegisterStyleProperty(pContext, pType, "menubar.text.color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiMenuBarRegisterStyleProperty(pContext, pType, "menubar.text.active_color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiMenuBarRegisterStyleProperty(pContext, pType, "menubar.text.disabled_color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiMenuBarRegisterStyleProperty(pContext, pType, "menubar.focus.color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiMenuBarRegisterStyleProperty(pContext, pType, "menubar.height", XUI_STYLE_VALUE_FLOAT, iLayoutDirty, 0);
	__xuiMenuBarRegisterStyleProperty(pContext, pType, "menubar.padding.x", XUI_STYLE_VALUE_FLOAT, iLayoutDirty, 0);
	__xuiMenuBarRegisterStyleProperty(pContext, pType, "menubar.padding.y", XUI_STYLE_VALUE_FLOAT, iLayoutDirty, 0);
	__xuiMenuBarRegisterStyleProperty(pContext, pType, "menubar.item.padding.x", XUI_STYLE_VALUE_FLOAT, iLayoutDirty, 0);
	__xuiMenuBarRegisterStyleProperty(pContext, pType, "menubar.item.gap", XUI_STYLE_VALUE_FLOAT, iLayoutDirty, 0);
	__xuiMenuBarRegisterStyleProperty(pContext, pType, "menubar.border.width", XUI_STYLE_VALUE_FLOAT, iPaintDirty, 0);
}

XUI_API xui_widget_type xuiMenuBarGetType(xui_context pContext)
{
	xui_widget_type pType;
	xui_widget_type_desc_t tDesc;
	int iRet;

	if ( !xuiInternalContextIsValid(pContext) ) return NULL;
	pType = xuiWidgetFindType(pContext, "menubar");
	if ( pType != NULL ) {
		pContext->onMenuUnavailable = __xuiMenuBarMenuUnavailable;
		return pType;
	}
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.sName = "menubar";
	tDesc.pParent = xuiWidgetGetBaseType();
	tDesc.iFlags = XUI_WIDGET_TYPE_DEFAULT_LAYOUT | XUI_WIDGET_TYPE_DEFAULT_CACHE_POLICY;
	tDesc.iTypeDataSize = sizeof(xui_menubar_data_t);
	tDesc.onInit = __xuiMenuBarInit;
	tDesc.onDestroy = __xuiMenuBarDestroy;
	tDesc.onContentMeasure = __xuiMenuBarContentMeasure;
	tDesc.onLayoutComplete = __xuiMenuBarLayoutComplete;
	tDesc.onCacheRender = __xuiMenuBarCacheRender;
	tDesc.onUpdate = __xuiMenuBarUpdate;
	__xuiMenuBarDefaultLayout(&tDesc.tLayout);
	__xuiMenuBarDefaultCachePolicy(&tDesc.tCachePolicy);
	iRet = xuiWidgetRegisterType(pContext, &pType, &tDesc);
	if ( iRet != XUI_OK ) return NULL;
	__xuiMenuBarRegisterStyleProperties(pContext, pType);
	pContext->onMenuUnavailable = __xuiMenuBarMenuUnavailable;
	return pType;
}

XUI_API int xuiMenuBarCreate(xui_context pContext, xui_widget* ppWidget, const xui_menubar_desc_t* pDesc)
{
	xui_widget_type pType;

	if ( ppWidget == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	*ppWidget = NULL;
	if ( !xuiInternalContextIsValid(pContext) || !__xuiMenuBarDescValid(pDesc) ) return XUI_ERROR_INVALID_ARGUMENT;
	pType = xuiMenuBarGetType(pContext);
	if ( pType == NULL ) return XUI_ERROR_NOT_INITIALIZED;
	return xuiWidgetCreateTyped(pContext, pType, ppWidget, pDesc);
}

XUI_API int xuiMenuBarSetItems(xui_widget pWidget, const xui_menubar_item_t* pItems, int iCount)
{
	xui_menubar_data_t* pData = __xuiMenuBarGetData(pWidget);
	xui_menubar_item_t arrNew[XUI_MENUBAR_ITEM_CAPACITY];
	xui_menubar_item_t arrOld[XUI_MENUBAR_ITEM_CAPACITY];
	char* arrNewDisplay[XUI_MENUBAR_ITEM_CAPACITY];
	char* arrOldDisplay[XUI_MENUBAR_ITEM_CAPACITY];
	unsigned char arrOldSelectBound[XUI_MENUBAR_ITEM_CAPACITY];
	int iOldCount;
	int iOldHover;
	int iOldActive;
	int iOldOpen;
	int iOldChangeCount;
	int bOldLayoutDirty;
	int iRet;
	int i;

	if ( (pData == NULL) || (iCount < 0) || (iCount > XUI_MENUBAR_ITEM_CAPACITY) || ((iCount > 0) && (pItems == NULL)) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	for ( i = 0; i < iCount; i++ ) {
		if ( !__xuiMenuBarItemValid(pWidget, &pItems[i]) ) return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( __xuiMenuBarItemsEqual(pData, pItems, iCount) ) return XUI_OK;
	iRet = __xuiMenuBarCopyItems(arrNew, arrNewDisplay, pItems, iCount);
	if ( iRet != XUI_OK ) return iRet;
	iRet = __xuiMenuBarCloseOpen(pWidget, pData);
	if ( iRet != XUI_OK ) goto failed_new;
	if ( __xuiMenuBarGetData(pWidget) != pData ) goto abandoned_new;
	if ( xuiWidgetGetContext(pWidget)->pMenuBarModeOwner == pWidget ) {
		iRet = __xuiMenuBarModeExit(pWidget, pData);
		if ( iRet != XUI_OK ) goto failed_new;
	}
	iRet = __xuiMenuBarResetMnemonics(pWidget, pData);
	if ( iRet != XUI_OK ) goto failed_new;
	memcpy(arrOld, pData->arrItems, sizeof(arrOld));
	memcpy(arrOldDisplay, pData->arrDisplayText, sizeof(arrOldDisplay));
	memcpy(arrOldSelectBound, pData->arrSelectBound, sizeof(arrOldSelectBound));
	iOldCount = pData->iItemCount;
	iOldHover = pData->iHover;
	iOldActive = pData->iActive;
	iOldOpen = pData->iOpen;
	iOldChangeCount = pData->iChangeCount;
	bOldLayoutDirty = pData->bLayoutDirty;
	memcpy(pData->arrItems, arrNew, sizeof(arrNew));
	memcpy(pData->arrDisplayText, arrNewDisplay, sizeof(arrNewDisplay));
	memset(pData->arrSelectBound, 0, sizeof(pData->arrSelectBound));
	memset(pData->arrPreviousSelect, 0, sizeof(pData->arrPreviousSelect));
	memset(pData->arrPreviousSelectUser, 0, sizeof(pData->arrPreviousSelectUser));
	pData->iItemCount = iCount;
	pData->iHover = -1;
	pData->iActive = -1;
	pData->iOpen = -1;
	__xuiMenuBarMarkChanged(pData);
	pData->bLayoutDirty = 1;
	iRet = __xuiMenuBarRegisterMnemonics(pWidget, pData);
	if ( iRet == XUI_OK ) {
		iRet = xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	}
	if ( iRet != XUI_OK ) {
		(void)__xuiMenuBarResetMnemonics(pWidget, pData);
		memcpy(pData->arrItems, arrOld, sizeof(arrOld));
		memcpy(pData->arrDisplayText, arrOldDisplay, sizeof(arrOldDisplay));
		memcpy(pData->arrSelectBound, arrOldSelectBound, sizeof(arrOldSelectBound));
		memset(pData->arrPreviousSelect, 0, sizeof(pData->arrPreviousSelect));
		memset(pData->arrPreviousSelectUser, 0, sizeof(pData->arrPreviousSelectUser));
		pData->iItemCount = iOldCount;
		pData->iHover = iOldHover;
		pData->iActive = iOldActive;
		pData->iOpen = iOldOpen;
		pData->iChangeCount = iOldChangeCount;
		pData->bLayoutDirty = bOldLayoutDirty;
		(void)__xuiMenuBarRegisterMnemonics(pWidget, pData);
		(void)xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
		__xuiMenuBarFreeItems(arrNew, arrNewDisplay, iCount);
		return iRet;
	}
	__xuiMenuBarFreeItems(arrOld, arrOldDisplay, iOldCount);
	xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_TREE_CHANGED);
	xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_SELECTION_CHANGED);
	return XUI_OK;

failed_new:
	__xuiMenuBarFreeItems(arrNew, arrNewDisplay, iCount);
	return iRet;

abandoned_new:
	__xuiMenuBarFreeItems(arrNew, arrNewDisplay, iCount);
	return XUI_OK;
}

XUI_API int xuiMenuBarAddItem(xui_widget pWidget, const char* sText, xui_widget pMenu, int iValue)
{
	xui_menubar_data_t* pData = __xuiMenuBarGetData(pWidget);
	xui_menubar_item_t arrItems[XUI_MENUBAR_ITEM_CAPACITY];

	if ( (pData == NULL) || (pData->iItemCount >= XUI_MENUBAR_ITEM_CAPACITY) ||
	     !__xuiMenuBarMenuValid(pWidget, pMenu) ) return XUI_ERROR_INVALID_ARGUMENT;
	memcpy(arrItems, pData->arrItems, sizeof(arrItems));
	memset(&arrItems[pData->iItemCount], 0, sizeof(arrItems[0]));
	arrItems[pData->iItemCount].sText = sText;
	arrItems[pData->iItemCount].iState = XUI_MENUBAR_ITEM_ENABLED;
	arrItems[pData->iItemCount].iValue = iValue;
	arrItems[pData->iItemCount].pMenu = pMenu;
	return xuiMenuBarSetItems(pWidget, arrItems, pData->iItemCount + 1);
}

XUI_API int xuiMenuBarClear(xui_widget pWidget)
{
	if ( __xuiMenuBarGetData(pWidget) == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	return xuiMenuBarSetItems(pWidget, NULL, 0);
}

XUI_API int xuiMenuBarGetItemCount(xui_widget pWidget)
{
	xui_menubar_data_t* pData = __xuiMenuBarGetData(pWidget);
	return (pData != NULL) ? pData->iItemCount : 0;
}

XUI_API const xui_menubar_item_t* xuiMenuBarGetItem(xui_widget pWidget, int iIndex)
{
	xui_menubar_data_t* pData = __xuiMenuBarGetData(pWidget);
	if ( (pData == NULL) || (iIndex < 0) || (iIndex >= pData->iItemCount) ) return NULL;
	return &pData->arrItems[iIndex];
}

XUI_API xui_rect_t xuiMenuBarGetItemRect(xui_widget pWidget, int iIndex)
{
	xui_menubar_data_t* pData = __xuiMenuBarGetData(pWidget);
	xui_menubar_resolved_t tResolved;
	xui_rect_t tEmpty;

	memset(&tEmpty, 0, sizeof(tEmpty));
	if ( (pData == NULL) || (iIndex < 0) || (iIndex >= pData->iItemCount) ) return tEmpty;
	__xuiMenuBarResolve(pWidget, pData, &tResolved);
	if ( __xuiMenuBarLayoutItems(pWidget, pData, &tResolved, NULL) != XUI_OK ) return tEmpty;
	return pData->arrItems[iIndex].tRect;
}

XUI_API int xuiMenuBarSetItemMenu(xui_widget pWidget, int iIndex, xui_widget pMenu)
{
	xui_menubar_data_t* pData = __xuiMenuBarGetData(pWidget);
	xui_widget pOldMenu;
	int iOldChangeCount;
	int iRet;

	if ( (pData == NULL) || (iIndex < 0) || (iIndex >= pData->iItemCount) ||
	     !__xuiMenuBarMenuValid(pWidget, pMenu) ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pData->arrItems[iIndex].pMenu == pMenu ) return XUI_OK;
	pOldMenu = pData->arrItems[iIndex].pMenu;
	if ( pData->iOpen == iIndex || pData->iActive == iIndex ) {
		iRet = __xuiMenuBarCloseOpen(pWidget, pData);
		if ( iRet != XUI_OK ) return iRet;
		if ( __xuiMenuBarGetData(pWidget) != pData ) return XUI_OK;
	}
	pData->arrItems[iIndex].pMenu = pMenu;
	iOldChangeCount = pData->iChangeCount;
	__xuiMenuBarMarkChanged(pData);
	iRet = xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	if ( iRet != XUI_OK ) {
		pData->arrItems[iIndex].pMenu = pOldMenu;
		pData->iChangeCount = iOldChangeCount;
	}
	else xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_NODE_CHANGED);
	return iRet;
}

XUI_API int xuiMenuBarSetItemEnabled(xui_widget pWidget, int iIndex, int bEnabled)
{
	xui_menubar_data_t* pData = __xuiMenuBarGetData(pWidget);
	uint32_t iOldState;
	int iOldHover;
	int iOldChangeCount;
	int iRet;
	int bOldEnabled;

	if ( (pData == NULL) || (iIndex < 0) || (iIndex >= pData->iItemCount) ) return XUI_ERROR_INVALID_ARGUMENT;
	bEnabled = bEnabled ? 1 : 0;
	bOldEnabled = __xuiMenuBarItemEnabled(&pData->arrItems[iIndex]);
	if ( bEnabled == bOldEnabled ) return XUI_OK;
	if ( !bEnabled && (pData->iOpen == iIndex || pData->iActive == iIndex) ) {
		iRet = __xuiMenuBarCloseOpen(pWidget, pData);
		if ( iRet != XUI_OK ) return iRet;
		if ( __xuiMenuBarGetData(pWidget) != pData ) return XUI_OK;
	}
	iOldState = pData->arrItems[iIndex].iState;
	iOldHover = pData->iHover;
	iOldChangeCount = pData->iChangeCount;
	if ( bEnabled ) pData->arrItems[iIndex].iState |= XUI_MENUBAR_ITEM_ENABLED;
	else pData->arrItems[iIndex].iState &= ~XUI_MENUBAR_ITEM_ENABLED;
	if ( !bEnabled && pData->iHover == iIndex ) pData->iHover = -1;
	__xuiMenuBarMarkChanged(pData);
	iRet = xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	if ( iRet != XUI_OK ) {
		pData->arrItems[iIndex].iState = iOldState;
		pData->iHover = iOldHover;
		pData->iChangeCount = iOldChangeCount;
		return iRet;
	}
	xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_STATE_CHANGED);
	if ( pData->iHover != iOldHover ) xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_SELECTION_CHANGED);
	return XUI_OK;
}

XUI_API int xuiMenuBarIsItemEnabled(xui_widget pWidget, int iIndex)
{
	xui_menubar_data_t* pData = __xuiMenuBarGetData(pWidget);
	if ( (pData == NULL) || (iIndex < 0) || (iIndex >= pData->iItemCount) ) return 0;
	return __xuiMenuBarItemEnabled(&pData->arrItems[iIndex]);
}

XUI_API int xuiMenuBarSetItemMnemonic(xui_widget pWidget, int iIndex, int iMnemonic)
{
	xui_menubar_data_t* pData = __xuiMenuBarGetData(pWidget);
	int iOldMnemonic;
	int iRet;

	if ( (pData == NULL) || (iIndex < 0) || (iIndex >= pData->iItemCount) || iMnemonic < 0 ) return XUI_ERROR_INVALID_ARGUMENT;
	iMnemonic = __xuiMenuBarAsciiUpper(iMnemonic);
	if ( pData->arrItems[iIndex].iMnemonic == iMnemonic ) return XUI_OK;
	iOldMnemonic = pData->arrItems[iIndex].iMnemonic;
	iRet = __xuiMenuBarResetMnemonics(pWidget, pData);
	if ( iRet != XUI_OK ) return iRet;
	pData->arrItems[iIndex].iMnemonic = __xuiMenuBarAsciiUpper(iMnemonic);
	iRet = __xuiMenuBarRegisterMnemonics(pWidget, pData);
	if ( iRet == XUI_OK ) iRet = xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	if ( iRet != XUI_OK ) {
		(void)__xuiMenuBarResetMnemonics(pWidget, pData);
		pData->arrItems[iIndex].iMnemonic = iOldMnemonic;
		(void)__xuiMenuBarRegisterMnemonics(pWidget, pData);
		return iRet;
	}
	__xuiMenuBarMarkChanged(pData);
	xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_NODE_CHANGED);
	return XUI_OK;
}

XUI_API int xuiMenuBarGetHoverIndex(xui_widget pWidget)
{
	xui_menubar_data_t* pData = __xuiMenuBarGetData(pWidget);
	return (pData != NULL) ? pData->iHover : -1;
}

XUI_API int xuiMenuBarSetHoverIndex(xui_widget pWidget, int iIndex)
{
	xui_menubar_data_t* pData = __xuiMenuBarGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	return __xuiMenuBarSetHover(pWidget, pData, iIndex);
}

XUI_API int xuiMenuBarGetActiveIndex(xui_widget pWidget)
{
	xui_menubar_data_t* pData = __xuiMenuBarGetData(pWidget);
	return (pData != NULL) ? pData->iActive : -1;
}

XUI_API int xuiMenuBarGetOpenIndex(xui_widget pWidget)
{
	xui_menubar_data_t* pData = __xuiMenuBarGetData(pWidget);
	if ( pData == NULL ) return -1;
	(void)__xuiMenuBarSyncOpen(pWidget, pData);
	return pData->iOpen;
}

XUI_API int xuiMenuBarOpenItem(xui_widget pWidget, int iIndex)
{
	xui_menubar_data_t* pData = __xuiMenuBarGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	return __xuiMenuBarOpenItemInternal(pWidget, pData, iIndex);
}

XUI_API int xuiMenuBarClose(xui_widget pWidget)
{
	xui_menubar_data_t* pData = __xuiMenuBarGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	return __xuiMenuBarCloseOpen(pWidget, pData);
}

XUI_API int xuiMenuBarIsOpen(xui_widget pWidget)
{
	return xuiMenuBarGetOpenIndex(pWidget) >= 0;
}

XUI_API xui_widget xuiMenuBarGetOpenMenu(xui_widget pWidget)
{
	xui_menubar_data_t* pData = __xuiMenuBarGetData(pWidget);
	if ( pData == NULL ) return NULL;
	(void)__xuiMenuBarSyncOpen(pWidget, pData);
	return (pData->iOpen >= 0 && pData->iOpen < pData->iItemCount) ? pData->arrItems[pData->iOpen].pMenu : NULL;
}

XUI_API int xuiMenuBarSetSelect(xui_widget pWidget, xui_menu_select_proc onSelect, void* pUser)
{
	xui_menubar_data_t* pData = __xuiMenuBarGetData(pWidget);
	xui_widget pMenu;
	int iRet;

	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pData->iOpen >= 0 && pData->iOpen < pData->iItemCount ) {
		pMenu = pData->arrItems[pData->iOpen].pMenu;
		if ( onSelect != NULL && __xuiMenuBarMenuValid(pWidget, pMenu) ) {
			iRet = __xuiMenuBarBindSelect(pWidget, pData, pData->iOpen, onSelect, pUser);
			if ( iRet != XUI_OK ) return iRet;
		} else {
			iRet = __xuiMenuBarReleaseSelectBinding(pWidget, pData, pData->iOpen);
			if ( iRet != XUI_OK ) return iRet;
		}
	}
	pData->onSelect = onSelect;
	pData->pSelectUser = pUser;
	return XUI_OK;
}

XUI_API int xuiMenuBarSetFont(xui_widget pWidget, xui_font pFont)
{
	xui_menubar_data_t* pData = __xuiMenuBarGetData(pWidget);
	xui_font pOldFont;
	int bOldLayoutDirty;
	int iRet;

	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pData->pFont == pFont ) return XUI_OK;
	pOldFont = pData->pFont;
	bOldLayoutDirty = pData->bLayoutDirty;
	pData->pFont = pFont;
	pData->bLayoutDirty = 1;
	iRet = xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	if ( iRet != XUI_OK ) {
		pData->pFont = pOldFont;
		pData->bLayoutDirty = bOldLayoutDirty;
	}
	else xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_BOUNDS_CHANGED);
	return iRet;
}

XUI_API xui_font xuiMenuBarGetFont(xui_widget pWidget)
{
	xui_menubar_data_t* pData = __xuiMenuBarGetData(pWidget);
	return (pData != NULL) ? pData->pFont : NULL;
}

XUI_API int xuiMenuBarSetMetrics(xui_widget pWidget, const xui_menubar_metrics_t* pMetrics)
{
	xui_menubar_data_t* pData = __xuiMenuBarGetData(pWidget);
	xui_menubar_resolved_t tResolved;
	xui_menubar_metrics_t tOld;
	int bOldLayoutDirty;
	int iRet;

	if ( (pData == NULL) || !__xuiMenuBarMetricsValid(pMetrics) ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( __xuiMenuBarMetricsEqual(&pData->tMetrics, pMetrics) ) return XUI_OK;
	tOld = pData->tMetrics;
	bOldLayoutDirty = pData->bLayoutDirty;
	pData->tMetrics = *pMetrics;
	pData->tMetrics.iSize = sizeof(pData->tMetrics);
	pData->bLayoutDirty = 1;
	__xuiMenuBarResolve(pWidget, pData, &tResolved);
	iRet = __xuiMenuBarLayoutItems(pWidget, pData, &tResolved, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetInvalidate(pWidget,
		XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	if ( iRet != XUI_OK ) {
		pData->tMetrics = tOld;
		pData->bLayoutDirty = bOldLayoutDirty;
	}
	else xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_BOUNDS_CHANGED);
	return iRet;
}

XUI_API int xuiMenuBarGetMetrics(xui_widget pWidget, xui_menubar_metrics_t* pMetrics)
{
	xui_menubar_data_t* pData = __xuiMenuBarGetData(pWidget);
	if ( (pData == NULL) || (pMetrics == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	*pMetrics = pData->tMetrics;
	pMetrics->iSize = sizeof(*pMetrics);
	return XUI_OK;
}

XUI_API int xuiMenuBarSetColors(xui_widget pWidget, const xui_menubar_colors_t* pColors)
{
	xui_menubar_data_t* pData = __xuiMenuBarGetData(pWidget);
	xui_menubar_colors_t tOld;
	int iRet;

	if ( (pData == NULL) || !__xuiMenuBarColorsValid(pColors) ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( __xuiMenuBarColorsEqual(&pData->tColors, pColors) ) return XUI_OK;
	tOld = pData->tColors;
	pData->tColors = *pColors;
	pData->tColors.iSize = sizeof(pData->tColors);
	iRet = xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	if ( iRet != XUI_OK ) pData->tColors = tOld;
	else xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_NODE_CHANGED);
	return iRet;
}

XUI_API int xuiMenuBarGetColors(xui_widget pWidget, xui_menubar_colors_t* pColors)
{
	xui_menubar_data_t* pData = __xuiMenuBarGetData(pWidget);
	if ( (pData == NULL) || (pColors == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	*pColors = pData->tColors;
	pColors->iSize = sizeof(*pColors);
	return XUI_OK;
}

XUI_API int xuiMenuBarGetState(xui_widget pWidget)
{
	xui_menubar_data_t* pData = __xuiMenuBarGetData(pWidget);
	int iState;

	if ( pData == NULL ) return 0;
	iState = (int)xuiWidgetGetInputState(pWidget);
	if ( xuiMenuBarIsOpen(pWidget) ) iState |= XUI_MENUBAR_STATE_OPEN;
	return iState;
}

XUI_API int xuiMenuBarGetChangeCount(xui_widget pWidget)
{
	xui_menubar_data_t* pData = __xuiMenuBarGetData(pWidget);
	return (pData != NULL) ? pData->iChangeCount : 0;
}

#endif
