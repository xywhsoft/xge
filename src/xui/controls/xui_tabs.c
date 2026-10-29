#include "xui_config.h"
#if XUI_ENABLE_TABS
#include "xui_internal.h"
#include "xui_pointer_press_internal.h"

#include <math.h>
#include <string.h>

typedef struct xui_tabs_page_t {
	xui_widget pButton;
	xui_widget pPage;
	char sTitle[XUI_TABS_TITLE_CAPACITY];
	xui_surface pIcon;
	xui_rect_t tIconSrc;
	xui_rect_t tTabRect;
	xui_rect_t tTextRect;
	xui_rect_t tIconRect;
	xui_rect_t tDirtyRect;
	xui_rect_t tCloseRect;
	xui_surface_dependency_t tIconDependency;
	int bEnabled;
	int bDirty;
} xui_tabs_page_t;

typedef struct xui_tabs_data_t {
	xui_tabs_select_proc onSelect;
	xui_tabs_close_proc onClose;
	xui_tabs_context_proc onContext;
	void* pSelectUser;
	void* pCloseUser;
	void* pContextUser;
	xui_widget pTabBar;
	xui_widget pClient;
	xui_widget pOverflowMenu;
	xui_tabs_page_t arrPages[XUI_TABS_PAGE_CAPACITY];
	xui_font pFont;
	int iPageCount;
	int iSelected;
	int iPlacement;
	int iHoverIndex;
	int iActiveIndex;
	int iCloseHoverIndex;
	int iCloseActiveIndex;
	int iChangeCount;
	int iCloseCount;
	xui_pointer_owner_t tPointerOwner;
	int bScrollable;
	int bCloseButtons;
	int bOverflow;
	int bOverflowHover;
	int bOverflowActive;
	int iFirstVisibleTab;
	int iVisibleTabCount;
	float fTabWidth;
	float fTabHeight;
	float fResolvedTabWidth;
	float fResolvedTabHeight;
	float fScrollX;
	float fMaxScroll;
	float fTotalAxis;
	xui_rect_t tTabBarRect;
	xui_rect_t tClientRect;
	xui_rect_t tOverflowRect;
	uint32_t iBackgroundColor;
	uint32_t iTabColor;
	uint32_t iHoverColor;
	uint32_t iActiveColor;
	uint32_t iFocusColor;
	uint32_t iDisabledColor;
	uint32_t iTextColor;
	uint32_t iActiveTextColor;
	uint32_t iBorderColor;
	uint32_t iClientColor;
	uint32_t iDisabledTextColor;
	uint32_t iCloseColor;
	uint32_t iCloseDisabledColor;
	uint32_t iCloseHoverColor;
	uint32_t iCloseActiveColor;
	uint32_t iCloseHoverBackgroundColor;
	uint32_t iCloseActiveBackgroundColor;
	uint32_t iChromeStyleVersion;
} xui_tabs_data_t;

#define XUI_TABS_TAB_START		3.0f
#define XUI_TABS_TAB_END		4.0f
#define XUI_TABS_TAB_OVERLAP		1.0f
#define XUI_TABS_OVERFLOW_BUTTON	16.0f
#define XUI_TABS_OVERFLOW_GAP		3.0f

static int __xuiTabsButtonRender(xui_widget pButton, xui_draw_context pDraw, uint32_t iStateId, void* pUser);
static int __xuiTabsInvalidateAll(xui_widget pWidget, xui_tabs_data_t* pData, uint32_t iFlags);

static xui_thickness_t __xuiTabsThickness(float fLeft, float fTop, float fRight, float fBottom)
{
	xui_thickness_t tValue;

	tValue.fLeft = fLeft;
	tValue.fTop = fTop;
	tValue.fRight = fRight;
	tValue.fBottom = fBottom;
	return tValue;
}

static int __xuiTabsAlpha(uint32_t iColor)
{
	return (int)(iColor & 0xffu);
}

static int __xuiTabsPlacementValid(int iPlacement)
{
	return (iPlacement == XUI_TABS_PLACEMENT_TOP) ||
	       (iPlacement == XUI_TABS_PLACEMENT_BOTTOM) ||
	       (iPlacement == XUI_TABS_PLACEMENT_LEFT) ||
	       (iPlacement == XUI_TABS_PLACEMENT_RIGHT);
}

static int __xuiTabsVertical(int iPlacement)
{
	return (iPlacement == XUI_TABS_PLACEMENT_LEFT) || (iPlacement == XUI_TABS_PLACEMENT_RIGHT);
}

static int __xuiTabsDescValid(const xui_tabs_desc_t* pDesc)
{
	if ( pDesc == NULL ) {
		return 1;
	}
	if ( (pDesc->iSize != 0) && (pDesc->iSize < sizeof(*pDesc)) ) {
		return 0;
	}
	if ( (pDesc->iItemCount < 0) || (pDesc->iItemCount > XUI_TABS_PAGE_CAPACITY) ) {
		return 0;
	}
	if ( (pDesc->iPlacement != 0) && !__xuiTabsPlacementValid(pDesc->iPlacement) ) {
		return 0;
	}
	if ( !isfinite(pDesc->fTabWidth) || !isfinite(pDesc->fTabHeight) ||
	     (pDesc->fTabWidth < 0.0f) || (pDesc->fTabHeight < 0.0f) ||
	     (pDesc->fTabWidth > XUI_LAYOUT_UNBOUNDED) || (pDesc->fTabHeight > XUI_LAYOUT_UNBOUNDED) ) {
		return 0;
	}
	return 1;
}

static int __xuiTabsRectContains(xui_rect_t tRect, float fX, float fY)
{
	return (fX >= tRect.fX) && (fY >= tRect.fY) && (fX < (tRect.fX + tRect.fW)) && (fY < (tRect.fY + tRect.fH));
}

static float __xuiTabsMinFloat(float fA, float fB)
{
	return (fA < fB) ? fA : fB;
}

static float __xuiTabsMaxFloat(float fA, float fB)
{
	return (fA > fB) ? fA : fB;
}

static float __xuiTabsClampFloat(float fValue, float fMin, float fMax)
{
	if ( fValue < fMin ) return fMin;
	if ( fValue > fMax ) return fMax;
	return fValue;
}

static xui_tabs_data_t* __xuiTabsGetData(xui_widget pWidget)
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
	pType = xuiWidgetFindType(pContext, "tabs");
	if ( (pType == NULL) || !xuiWidgetIsType(pWidget, pType) ) {
		return NULL;
	}
	return (xui_tabs_data_t*)xuiWidgetGetTypeData(pWidget);
}

static int __xuiTabsWidgetAlive(xui_widget pWidget, xui_tabs_data_t* pData)
{
	xui_context pContext;

	if ( !xuiInternalWidgetIsValid(pWidget) ) return 0;
	pContext = xuiWidgetGetContext(pWidget);
	return xuiInternalContextIsValid(pContext) && !xuiInternalContextDestroyPending(pContext) &&
		__xuiTabsGetData(pWidget) == pData;
}

static int __xuiTabsResetInteraction(xui_widget pTabs, xui_tabs_data_t* pData, int bClearHover)
{
	xui_context pContext;
	xui_widget pCapture;
	int bChanged;
	int iRet;

	if ( pTabs == NULL || pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	pContext = xuiWidgetGetContext(pTabs);
	pCapture = NULL;
	if ( pData->tPointerOwner.bActive ) {
		pCapture = xuiGetPointerCaptureEx(pContext,
			pData->tPointerOwner.iPointerId, pData->tPointerOwner.iPointerType);
	}
	bChanged = pData->iActiveIndex >= 0 || pData->iCloseActiveIndex >= 0 ||
		pData->bOverflowActive || pData->tPointerOwner.bActive ||
		(bClearHover && (pData->iHoverIndex >= 0 || pData->iCloseHoverIndex >= 0 ||
		pData->bOverflowHover));
	if ( bClearHover ) pData->iHoverIndex = -1;
	pData->iActiveIndex = -1;
	if ( bClearHover ) pData->iCloseHoverIndex = -1;
	pData->iCloseActiveIndex = -1;
	if ( bClearHover ) pData->bOverflowHover = 0;
	pData->bOverflowActive = 0;
	iRet = xuiInternalPointerOwnerCancel(pContext, pCapture, &pData->tPointerOwner);
	if ( iRet != XUI_OK ) return iRet;
	return bChanged ? __xuiTabsInvalidateAll(pTabs, pData,
		XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER) : XUI_OK;
}

static int __xuiTabsClearInteraction(xui_widget pTabs, xui_tabs_data_t* pData)
{
	return __xuiTabsResetInteraction(pTabs, pData, 0);
}

static int __xuiTabsClearAllInteraction(xui_widget pTabs, xui_tabs_data_t* pData)
{
	return __xuiTabsResetInteraction(pTabs, pData, 1);
}

static int __xuiTabsIconRectValid(xui_rect_t tRect)
{
	return tRect.fX >= -(int)XUI_LAYOUT_UNBOUNDED && tRect.fX <= (int)XUI_LAYOUT_UNBOUNDED &&
		tRect.fY >= -(int)XUI_LAYOUT_UNBOUNDED && tRect.fY <= (int)XUI_LAYOUT_UNBOUNDED &&
		tRect.fW >= 0 && tRect.fW <= (int)XUI_LAYOUT_UNBOUNDED &&
		tRect.fH >= 0 && tRect.fH <= (int)XUI_LAYOUT_UNBOUNDED;
}

static int __xuiTabsStyleColor(xui_widget pWidget, const char* sName, uint32_t* pColor)
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

static int __xuiTabsStyleFloat(xui_widget pWidget, const char* sName, float* pValue)
{
	xui_style_property_t tProperty;
	int iRet;

	memset(&tProperty, 0, sizeof(tProperty));
	tProperty.iSize = sizeof(tProperty);
	iRet = xuiWidgetGetResolvedStyleProperty(pWidget, sName, &tProperty);
	if ( (iRet == XUI_OK) &&
	     (tProperty.tValue.iType == XUI_STYLE_VALUE_FLOAT) &&
	     isfinite(tProperty.tValue.fFloat) &&
	     (tProperty.tValue.fFloat >= 0.0f) &&
	     (tProperty.tValue.fFloat <= XUI_LAYOUT_UNBOUNDED) ) {
		*pValue = tProperty.tValue.fFloat;
		return 1;
	}
	return 0;
}

static void __xuiTabsResolve(xui_widget pWidget, xui_tabs_data_t* pData)
{
	pData->fResolvedTabWidth = pData->fTabWidth;
	pData->fResolvedTabHeight = pData->fTabHeight;
	(void)__xuiTabsStyleFloat(pWidget, "tabs.tab.width", &pData->fResolvedTabWidth);
	(void)__xuiTabsStyleFloat(pWidget, "tabs.tab.height", &pData->fResolvedTabHeight);
	if ( pData->fResolvedTabWidth < 24.0f ) {
		pData->fResolvedTabWidth = 24.0f;
	}
	if ( pData->fResolvedTabHeight < 18.0f ) {
		pData->fResolvedTabHeight = 18.0f;
	}
}

static void __xuiTabsResolveColors(xui_widget pWidget, const xui_tabs_data_t* pData, xui_tabs_data_t* pOut)
{
	pOut->iBackgroundColor = pData->iBackgroundColor;
	pOut->iTabColor = pData->iTabColor;
	pOut->iHoverColor = pData->iHoverColor;
	pOut->iActiveColor = pData->iActiveColor;
	pOut->iFocusColor = pData->iFocusColor;
	pOut->iDisabledColor = pData->iDisabledColor;
	pOut->iTextColor = pData->iTextColor;
	pOut->iActiveTextColor = pData->iActiveTextColor;
	pOut->iBorderColor = pData->iBorderColor;
	pOut->iClientColor = pData->iClientColor;
	(void)__xuiTabsStyleColor(pWidget, "tabs.background.color", &pOut->iBackgroundColor);
	(void)__xuiTabsStyleColor(pWidget, "tabs.tab.color", &pOut->iTabColor);
	(void)__xuiTabsStyleColor(pWidget, "tabs.tab.hover_color", &pOut->iHoverColor);
	(void)__xuiTabsStyleColor(pWidget, "tabs.tab.active_color", &pOut->iActiveColor);
	(void)__xuiTabsStyleColor(pWidget, "tabs.tab.focus_color", &pOut->iFocusColor);
	(void)__xuiTabsStyleColor(pWidget, "tabs.tab.disabled_color", &pOut->iDisabledColor);
	(void)__xuiTabsStyleColor(pWidget, "tabs.text.color", &pOut->iTextColor);
	(void)__xuiTabsStyleColor(pWidget, "tabs.text.active_color", &pOut->iActiveTextColor);
	(void)__xuiTabsStyleColor(pWidget, "tabs.border.color", &pOut->iBorderColor);
	(void)__xuiTabsStyleColor(pWidget, "tabs.client.color", &pOut->iClientColor);
	pOut->iDisabledTextColor = __xuiTabsAlpha(pOut->iTextColor) ? (pOut->iTextColor & 0xffffff00u) | 120u : pOut->iTextColor;
	pOut->iCloseColor = pOut->iTextColor;
	pOut->iCloseDisabledColor = __xuiTabsAlpha(pOut->iTextColor) ? (pOut->iTextColor & 0xffffff00u) | 100u : pOut->iTextColor;
	pOut->iCloseHoverBackgroundColor = XUI_COLOR_RGBA(220, 232, 246, 255);
	(void)__xuiTabsStyleColor(pWidget, "tabs.text.disabled_color", &pOut->iDisabledTextColor);
	(void)__xuiTabsStyleColor(pWidget, "tabs.close.color", &pOut->iCloseColor);
	(void)__xuiTabsStyleColor(pWidget, "tabs.close.disabled_color", &pOut->iCloseDisabledColor);
	(void)__xuiTabsStyleColor(pWidget, "tabs.close.hover_background_color", &pOut->iCloseHoverBackgroundColor);
	pOut->iCloseHoverColor = pOut->iCloseColor;
	(void)__xuiTabsStyleColor(pWidget, "tabs.close.hover_color", &pOut->iCloseHoverColor);
	pOut->iCloseActiveColor = pOut->iCloseHoverColor;
	pOut->iCloseActiveBackgroundColor = pOut->iCloseHoverBackgroundColor;
	(void)__xuiTabsStyleColor(pWidget, "tabs.close.active_color", &pOut->iCloseActiveColor);
	(void)__xuiTabsStyleColor(pWidget, "tabs.close.active_background_color", &pOut->iCloseActiveBackgroundColor);
}

static int __xuiTabsDrawFill(xui_proxy pProxy, xui_draw_context pDraw, xui_rect_t tRect, uint32_t iColor)
{
	if ( (pProxy == NULL) || (pProxy->drawRectFill == NULL) || (pDraw == NULL) ) {
		return XUI_ERROR_NOT_INITIALIZED;
	}
	if ( (tRect.fW <= 0.0f) || (tRect.fH <= 0.0f) || (__xuiTabsAlpha(iColor) == 0) ) {
		return XUI_OK;
	}
	return pProxy->drawRectFill(pProxy, pDraw, xuiInternalSnapRect(tRect), iColor);
}

static int __xuiTabsDrawStroke(xui_proxy pProxy, xui_draw_context pDraw, xui_rect_t tRect, uint32_t iColor)
{
	if ( (pProxy == NULL) || (pProxy->drawRectStroke == NULL) || (pDraw == NULL) ) {
		return XUI_ERROR_NOT_INITIALIZED;
	}
	if ( (tRect.fW <= 0.0f) || (tRect.fH <= 0.0f) || (__xuiTabsAlpha(iColor) == 0) ) {
		return XUI_OK;
	}
	return pProxy->drawRectStroke(pProxy, pDraw, xuiInternalSnapRect(tRect), 1.0f, iColor);
}

static int __xuiTabsDrawEdgeRect(xui_proxy pProxy, xui_draw_context pDraw, xui_rect_t tRect, uint32_t iColor, int bLeft, int bTop, int bRight, int bBottom)
{
	float fRight;
	float fBottom;
	int iRet;

	if ( (tRect.fW <= 0.0f) || (tRect.fH <= 0.0f) || (__xuiTabsAlpha(iColor) == 0) ) {
		return XUI_OK;
	}
	fRight = tRect.fX + tRect.fW;
	fBottom = tRect.fY + tRect.fH;
	iRet = XUI_OK;
	if ( bTop ) {
		iRet = __xuiTabsDrawFill(pProxy, pDraw, (xui_rect_t){tRect.fX, tRect.fY, tRect.fW, 1.0f}, iColor);
	}
	if ( iRet == XUI_OK && bBottom ) {
		iRet = __xuiTabsDrawFill(pProxy, pDraw, (xui_rect_t){tRect.fX, fBottom - 1.0f, tRect.fW, 1.0f}, iColor);
	}
	if ( iRet == XUI_OK && bLeft ) {
		iRet = __xuiTabsDrawFill(pProxy, pDraw, (xui_rect_t){tRect.fX, tRect.fY, 1.0f, tRect.fH}, iColor);
	}
	if ( iRet == XUI_OK && bRight ) {
		iRet = __xuiTabsDrawFill(pProxy, pDraw, (xui_rect_t){fRight - 1.0f, tRect.fY, 1.0f, tRect.fH}, iColor);
	}
	return iRet;
}

static int __xuiTabsDrawLine(xui_proxy pProxy, xui_draw_context pDraw, float fX0, float fY0, float fX1, float fY1, uint32_t iColor)
{
	if ( (pProxy == NULL) || (pProxy->drawLine == NULL) || (pDraw == NULL) ) {
		return XUI_ERROR_NOT_INITIALIZED;
	}
	if ( __xuiTabsAlpha(iColor) == 0 ) {
		return XUI_OK;
	}
	return pProxy->drawLine(pProxy, pDraw,
		xuiInternalSnapPixel(fX0), xuiInternalSnapPixel(fY0),
		xuiInternalSnapPixel(fX1), xuiInternalSnapPixel(fY1),
		1.0f, iColor);
}

static int __xuiTabsDrawCircle(xui_proxy pProxy, xui_draw_context pDraw, float fX, float fY, float fRadius, uint32_t iColor)
{
	if ( (fRadius <= 0.0f) || (__xuiTabsAlpha(iColor) == 0) ) {
		return XUI_OK;
	}
	if ( (pProxy != NULL) && (pProxy->drawCircleFill != NULL) && (pDraw != NULL) ) {
		return pProxy->drawCircleFill(pProxy, pDraw, fX, fY, fRadius, iColor);
	}
	return __xuiTabsDrawFill(pProxy, pDraw, (xui_rect_t){fX - fRadius, fY - fRadius, fRadius * 2.0f, fRadius * 2.0f}, iColor);
}

static int __xuiTabsDrawText(xui_proxy pProxy, xui_draw_context pDraw, const xui_text_item_t* pTextItem, xui_rect_t tRect, uint32_t iColor, uint32_t iFlags)
{
    xui_font pFont = pTextItem && pTextItem->iSize >= sizeof(*pTextItem) ? pTextItem->pFont : NULL;
    const char* sText = pTextItem && pTextItem->iSize >= sizeof(*pTextItem) ? pTextItem->sText : NULL;

	if ( (sText == NULL) || (*sText == 0) || (pFont == NULL) ) {
		return XUI_OK;
	}
	if ( (pProxy == NULL) || (pProxy->drawText == NULL) || (pDraw == NULL) ) {
		return XUI_ERROR_NOT_INITIALIZED;
	}
	if ( (tRect.fW <= 0.0f) || (tRect.fH <= 0.0f) || (__xuiTabsAlpha(iColor) == 0) ) {
		return XUI_OK;
	}
	return pProxy->drawText(pProxy, pDraw, pTextItem, xuiInternalSnapRect(tRect), iColor, iFlags | XUI_TEXT_CLIP);
}

static int __xuiTabsButtonIndex(const xui_tabs_data_t* pData, xui_widget pButton)
{
	int i;

	if ( (pData == NULL) || (pButton == NULL) ) {
		return -1;
	}
	for ( i = 0; i < XUI_TABS_PAGE_CAPACITY; i++ ) {
		if ( pData->arrPages[i].pButton == pButton ) {
			return i;
		}
	}
	return -1;
}

static int __xuiTabsItemEnabled(const xui_tabs_data_t* pData, int iIndex)
{
	return (pData != NULL) &&
	       (iIndex >= 0) &&
	       (iIndex < pData->iPageCount) &&
	       (pData->arrPages[iIndex].bEnabled != 0);
}

static int __xuiTabsFirstEnabled(const xui_tabs_data_t* pData)
{
	int i;

	if ( pData == NULL ) {
		return -1;
	}
	for ( i = 0; i < pData->iPageCount; i++ ) {
		if ( __xuiTabsItemEnabled(pData, i) ) {
			return i;
		}
	}
	return -1;
}

static int __xuiTabsStepEnabled(const xui_tabs_data_t* pData, int iStart, int iDelta)
{
	int i;
	int iIndex;

	if ( (pData == NULL) || (pData->iPageCount <= 0) || (iDelta == 0) ) {
		return -1;
	}
	iIndex = iStart;
	for ( i = 0; i < pData->iPageCount; i++ ) {
		iIndex += iDelta;
		if ( iIndex < 0 ) iIndex = pData->iPageCount - 1;
		if ( iIndex >= pData->iPageCount ) iIndex = 0;
		if ( __xuiTabsItemEnabled(pData, iIndex) ) {
			return iIndex;
		}
	}
	return -1;
}

static int __xuiTabsLastEnabled(const xui_tabs_data_t* pData)
{
	int i;

	if ( pData == NULL ) {
		return -1;
	}
	for ( i = pData->iPageCount - 1; i >= 0; i-- ) {
		if ( __xuiTabsItemEnabled(pData, i) ) {
			return i;
		}
	}
	return -1;
}

static void __xuiTabsInvalidateButton(xui_tabs_data_t* pData, int iIndex)
{
	if ( (pData != NULL) &&
	     (iIndex >= 0) &&
	     (iIndex < XUI_TABS_PAGE_CAPACITY) &&
	     (pData->arrPages[iIndex].pButton != NULL) ) {
		(void)xuiWidgetInvalidate(pData->arrPages[iIndex].pButton, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	}
}

static int __xuiTabsInvalidateAll(xui_widget pWidget, xui_tabs_data_t* pData, uint32_t iFlags)
{
	int i;

	if ( pData != NULL ) {
		for ( i = 0; i < pData->iPageCount; i++ ) {
			__xuiTabsInvalidateButton(pData, i);
		}
	}
	return xuiWidgetInvalidate(pWidget, iFlags);
}

static int __xuiTabsSetHover(xui_widget pTabs, xui_tabs_data_t* pData, int iHover)
{
	int iOld;

	if ( (pTabs == NULL) || (pData == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( iHover >= pData->iPageCount ) {
		iHover = -1;
	}
	iOld = pData->iHoverIndex;
	if ( iOld == iHover ) {
		return XUI_OK;
	}
	pData->iHoverIndex = iHover;
	__xuiTabsInvalidateButton(pData, iOld);
	__xuiTabsInvalidateButton(pData, iHover);
	return xuiWidgetInvalidate(pTabs, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
}

static int __xuiTabsSetCloseHover(xui_widget pTabs, xui_tabs_data_t* pData, int iHover)
{
	int iOld;

	if ( (pTabs == NULL) || (pData == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( iHover >= pData->iPageCount ) {
		iHover = -1;
	}
	iOld = pData->iCloseHoverIndex;
	if ( iOld == iHover ) {
		return XUI_OK;
	}
	pData->iCloseHoverIndex = iHover;
	__xuiTabsInvalidateButton(pData, iOld);
	__xuiTabsInvalidateButton(pData, iHover);
	return xuiWidgetInvalidate(pTabs, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
}

static int __xuiTabsLocalRectContains(xui_tabs_page_t* pPage, xui_rect_t tRootRect, float fX, float fY)
{
	tRootRect.fX -= pPage->tTabRect.fX;
	tRootRect.fY -= pPage->tTabRect.fY;
	return __xuiTabsRectContains(tRootRect, fX, fY);
}

static float __xuiTabsVisibleAxis(const xui_tabs_data_t* pData)
{
	if ( pData == NULL ) {
		return 0.0f;
	}
	if ( __xuiTabsVertical(pData->iPlacement) ) {
		return __xuiTabsMaxFloat(0.0f, pData->tTabBarRect.fH - XUI_TABS_TAB_START - XUI_TABS_TAB_END);
	}
	return __xuiTabsMaxFloat(0.0f, pData->tTabBarRect.fW - XUI_TABS_TAB_START - XUI_TABS_TAB_END);
}

static float __xuiTabsOverflowButtonAxis(const xui_tabs_data_t* pData)
{
	(void)pData;
	return XUI_TABS_OVERFLOW_BUTTON;
}

static float __xuiTabsTabStep(const xui_tabs_data_t* pData)
{
	if ( (pData == NULL) || (pData->fResolvedTabWidth <= XUI_TABS_TAB_OVERLAP) ) {
		return 1.0f;
	}
	return pData->fResolvedTabWidth - XUI_TABS_TAB_OVERLAP;
}

static void __xuiTabsOverflowMenuSelect(xui_widget pMenu, int iIndex, int iValue, void* pUser);

static int __xuiTabsComputeVisibleCount(const xui_tabs_data_t* pData, int* pOverflow)
{
	float fVisible;
	float fNatural;
	float fButton;
	float fTabAxis;
	float fStep;
	int iCount;

	if ( pOverflow != NULL ) {
		*pOverflow = 0;
	}
	if ( (pData == NULL) || (pData->iPageCount <= 0) || (pData->fResolvedTabWidth <= 0.0f) ) {
		return 0;
	}
	fVisible = __xuiTabsVisibleAxis(pData);
	fStep = __xuiTabsTabStep(pData);
	fNatural = ((float)(pData->iPageCount - 1) * fStep) + pData->fResolvedTabWidth;
	if ( fNatural <= fVisible + 0.5f ) {
		return pData->iPageCount;
	}
	if ( pOverflow != NULL ) {
		*pOverflow = 1;
	}
	fButton = __xuiTabsOverflowButtonAxis(pData) + XUI_TABS_OVERFLOW_GAP;
	fTabAxis = __xuiTabsMaxFloat(0.0f, fVisible - fButton);
	iCount = (int)((fTabAxis + XUI_TABS_TAB_OVERLAP) / fStep);
	if ( iCount < 1 ) iCount = 1;
	if ( iCount > pData->iPageCount ) iCount = pData->iPageCount;
	return iCount;
}

static int __xuiTabsComputeFirstVisible(xui_tabs_data_t* pData, int iVisibleCount)
{
	int iFirst;
	int iMaxFirst;

	if ( (pData == NULL) || (iVisibleCount <= 0) || (pData->iPageCount <= 0) ) {
		return 0;
	}
	if ( iVisibleCount >= pData->iPageCount ) {
		return 0;
	}
	iFirst = (__xuiTabsTabStep(pData) > 0.0f) ? (int)(pData->fScrollX / __xuiTabsTabStep(pData)) : 0;
	iMaxFirst = pData->iPageCount - iVisibleCount;
	if ( iFirst < 0 ) iFirst = 0;
	if ( iFirst > iMaxFirst ) iFirst = iMaxFirst;
	if ( pData->iSelected >= 0 ) {
		if ( pData->iSelected < iFirst ) {
			iFirst = pData->iSelected;
		} else if ( pData->iSelected >= iFirst + iVisibleCount ) {
			iFirst = pData->iSelected - iVisibleCount + 1;
		}
	}
	if ( iFirst < 0 ) iFirst = 0;
	if ( iFirst > iMaxFirst ) iFirst = iMaxFirst;
	return iFirst;
}

static void __xuiTabsUpdateMaxScroll(xui_tabs_data_t* pData)
{
	int iVisibleCount;
	int iOverflow;
	int iMaxFirst;

	if ( pData == NULL ) {
		return;
	}
	iVisibleCount = __xuiTabsComputeVisibleCount(pData, &iOverflow);
	iMaxFirst = (iOverflow && iVisibleCount < pData->iPageCount) ? (pData->iPageCount - iVisibleCount) : 0;
	pData->fTotalAxis = (pData->iPageCount > 0) ? (((float)(pData->iPageCount - 1) * __xuiTabsTabStep(pData)) + pData->fResolvedTabWidth) : 0.0f;
	pData->fMaxScroll = (float)iMaxFirst * __xuiTabsTabStep(pData);
	pData->fScrollX = __xuiTabsClampFloat(pData->fScrollX, 0.0f, pData->fMaxScroll);
}

static void __xuiTabsEnsureVisibleInternal(xui_tabs_data_t* pData, int iIndex)
{
	int iVisibleCount;
	int iOverflow;
	int iFirst;

	if ( (pData == NULL) || !pData->bScrollable || (iIndex < 0) || (iIndex >= pData->iPageCount) ) {
		return;
	}
	__xuiTabsUpdateMaxScroll(pData);
	iVisibleCount = __xuiTabsComputeVisibleCount(pData, &iOverflow);
	if ( !iOverflow || (iVisibleCount <= 0) ) {
		pData->fScrollX = 0.0f;
		return;
	}
	iFirst = __xuiTabsComputeFirstVisible(pData, iVisibleCount);
	if ( iIndex < iFirst ) {
		iFirst = iIndex;
	} else if ( iIndex >= iFirst + iVisibleCount ) {
		iFirst = iIndex - iVisibleCount + 1;
	}
	if ( iFirst < 0 ) iFirst = 0;
	if ( iFirst > pData->iPageCount - iVisibleCount ) iFirst = pData->iPageCount - iVisibleCount;
	pData->fScrollX = (float)iFirst * __xuiTabsTabStep(pData);
	pData->fScrollX = __xuiTabsClampFloat(pData->fScrollX, 0.0f, pData->fMaxScroll);
}

static int __xuiTabsSyncPages(xui_widget pTabs, xui_tabs_data_t* pData)
{
	int iResult;
	int iRet;
	int i;

	if ( (pTabs == NULL) || (pData == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	iResult = XUI_OK;
	for ( i = 0; i < XUI_TABS_PAGE_CAPACITY; i++ ) {
		if ( pData->arrPages[i].pButton != NULL ) {
			iRet = xuiWidgetSetVisible(pData->arrPages[i].pButton, i < pData->iPageCount);
			if ( iResult == XUI_OK && iRet != XUI_OK ) iResult = iRet;
			iRet = xuiWidgetSetEnabled(pData->arrPages[i].pButton, pData->arrPages[i].bEnabled);
			if ( iResult == XUI_OK && iRet != XUI_OK ) iResult = iRet;
			iRet = xuiWidgetSetLayer(pData->arrPages[i].pButton, XUI_LAYER_NORMAL, (i == pData->iSelected) ? 2 : 1);
			if ( iResult == XUI_OK && iRet != XUI_OK ) iResult = iRet;
		}
		if ( pData->arrPages[i].pPage != NULL ) {
			iRet = xuiWidgetSetVisible(pData->arrPages[i].pPage, (i < pData->iPageCount) && (i == pData->iSelected));
			if ( iResult == XUI_OK && iRet != XUI_OK ) iResult = iRet;
			iRet = xuiWidgetSetEnabled(pData->arrPages[i].pPage, (i < pData->iPageCount) && (pData->arrPages[i].bEnabled != 0));
			if ( iResult == XUI_OK && iRet != XUI_OK ) iResult = iRet;
		}
	}
	if ( iResult != XUI_OK ) return iResult;
	return __xuiTabsInvalidateAll(pTabs, pData, XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
}

static int __xuiTabsSelectInternal(xui_widget pTabs, xui_tabs_data_t* pData, int iIndex, int bNotify)
{
	float fOldScroll;
	int iOldSelected;
	int iOldChangeCount;
	int iRet;

	if ( (pTabs == NULL) || (pData == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( (iIndex < 0) || (iIndex >= pData->iPageCount) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( !__xuiTabsItemEnabled(pData, iIndex) ) {
		return XUI_OK;
	}
	if ( pData->iSelected == iIndex ) {
		__xuiTabsEnsureVisibleInternal(pData, iIndex);
		return __xuiTabsInvalidateAll(pTabs, pData, XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	}
	iOldSelected = pData->iSelected;
	iOldChangeCount = pData->iChangeCount;
	fOldScroll = pData->fScrollX;
	pData->iSelected = iIndex;
	pData->iChangeCount++;
	__xuiTabsEnsureVisibleInternal(pData, iIndex);
	iRet = __xuiTabsSyncPages(pTabs, pData);
	if ( iRet != XUI_OK ) {
		pData->iSelected = iOldSelected;
		pData->iChangeCount = iOldChangeCount;
		pData->fScrollX = fOldScroll;
		/* Preserve the original failure; restoring widget state is best-effort. */
		(void)__xuiTabsSyncPages(pTabs, pData);
		return iRet;
	}
	xuiInternalAccessibilityQueue(pTabs, XUI_ACCESSIBLE_EVENT_SELECTION_CHANGED);
	if ( bNotify && (pData->onSelect != NULL) ) {
		pData->onSelect(pTabs, iIndex, pData->pSelectUser);
	}
	return XUI_OK;
}

static int __xuiTabsButtonEvent(xui_widget pButton, const xui_event_t* pEvent, void* pUser)
{
	xui_widget pTabs;
	xui_tabs_data_t* pData;
	xui_tabs_page_t* pPage;
	xui_rect_t tWorld;
	float fX;
	float fY;
	int iIndex;
	int bClose;
	int iRet;

	pTabs = (xui_widget)pUser;
	pData = __xuiTabsGetData(pTabs);
	if ( (pButton == NULL) || (pEvent == NULL) || (pData == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	iIndex = __xuiTabsButtonIndex(pData, pButton);
	if ( iIndex < 0 ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pPage = &pData->arrPages[iIndex];
	tWorld = xuiWidgetGetWorldRect(pButton);
	fX = pEvent->fX - tWorld.fX;
	fY = pEvent->fY - tWorld.fY;
	bClose = pData->bCloseButtons &&
	         pData->onClose != NULL &&
	         __xuiTabsLocalRectContains(pPage, pPage->tCloseRect, fX, fY);

	switch ( pEvent->iType ) {
	case XUI_EVENT_POINTER_ENTER:
	case XUI_EVENT_POINTER_MOVE:
		if ( pData->tPointerOwner.bActive &&
		     !xuiInternalPointerOwnerMatches(&pData->tPointerOwner, pEvent) ) {
			return XUI_OK;
		}
		if ( __xuiTabsItemEnabled(pData, iIndex) ) {
			iRet = __xuiTabsSetHover(pTabs, pData, iIndex);
			if ( iRet == XUI_OK ) iRet = __xuiTabsSetCloseHover(pTabs, pData, bClose ? iIndex : -1);
			if ( iRet != XUI_OK ) return iRet;
		}
		return XUI_EVENT_DISPATCH_STOP;
	case XUI_EVENT_POINTER_LEAVE:
		if ( pData->iHoverIndex == iIndex ) {
			iRet = __xuiTabsSetHover(pTabs, pData, -1);
			if ( iRet != XUI_OK ) return iRet;
		}
		if ( pData->iCloseHoverIndex == iIndex ) {
			iRet = __xuiTabsSetCloseHover(pTabs, pData, -1);
			if ( iRet != XUI_OK ) return iRet;
		}
		return XUI_EVENT_DISPATCH_STOP;
	case XUI_EVENT_POINTER_DOWN:
		if ( (pEvent->iButton == XUI_POINTER_BUTTON_LEFT) && __xuiTabsItemEnabled(pData, iIndex) ) {
			if ( pData->tPointerOwner.bActive ) {
				return xuiInternalPointerOwnerMatches(&pData->tPointerOwner, pEvent) ?
					XUI_EVENT_DISPATCH_STOP : XUI_OK;
			}
			pData->iActiveIndex = iIndex;
			pData->iCloseActiveIndex = bClose ? iIndex : -1;
			if ( !bClose ) {
				iRet = __xuiTabsSelectInternal(pTabs, pData, iIndex, 1);
				if ( iRet != XUI_OK ) return iRet;
				if ( !__xuiTabsWidgetAlive(pTabs, pData) ) return XUI_EVENT_DISPATCH_STOP;
			}
			iRet = xuiSetFocusWidget(xuiWidgetGetContext(pTabs), pTabs);
			if ( iRet == XUI_OK && !__xuiTabsWidgetAlive(pTabs, pData) ) {
				return XUI_EVENT_DISPATCH_STOP;
			}
			if ( iRet == XUI_OK ) iRet = xuiInternalPointerOwnerBegin(
				xuiWidgetGetContext(pTabs), pButton, pEvent, &pData->tPointerOwner);
			if ( iRet == XUI_OK ) iRet = __xuiTabsInvalidateAll(pTabs, pData,
				XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
			if ( iRet != XUI_OK ) {
				(void)__xuiTabsClearInteraction(pTabs, pData);
				return iRet;
			}
			return XUI_EVENT_DISPATCH_STOP;
		}
		break;
	case XUI_EVENT_POINTER_UP:
		if ( pData->iActiveIndex == iIndex &&
		     xuiInternalPointerOwnerMatches(&pData->tPointerOwner, pEvent) ) {
			if ( pData->iCloseActiveIndex == iIndex && bClose ) {
				pData->iCloseCount++;
				pData->iActiveIndex = -1;
				pData->iCloseActiveIndex = -1;
				iRet = xuiInternalPointerOwnerEnd(xuiWidgetGetContext(pTabs),
					pButton, pEvent, &pData->tPointerOwner);
				if ( iRet == XUI_OK ) iRet = __xuiTabsInvalidateAll(pTabs, pData,
					XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
				if ( iRet != XUI_OK ) return iRet;
				if ( pData->onClose != NULL ) {
					pData->onClose(pTabs, iIndex, pData->pCloseUser);
				}
				return XUI_EVENT_DISPATCH_STOP;
			}
			pData->iActiveIndex = -1;
			pData->iCloseActiveIndex = -1;
			iRet = xuiInternalPointerOwnerEnd(xuiWidgetGetContext(pTabs), pButton,
				pEvent, &pData->tPointerOwner);
			if ( iRet != XUI_OK ) return iRet;
			if ( __xuiTabsRectContains((xui_rect_t){0.0f, 0.0f, pPage->tTabRect.fW, pPage->tTabRect.fH}, fX, fY) ) {
				iRet = __xuiTabsSelectInternal(pTabs, pData, iIndex, 1);
				if ( iRet != XUI_OK ) return iRet;
				if ( !__xuiTabsWidgetAlive(pTabs, pData) ) return XUI_EVENT_DISPATCH_STOP;
			}
			iRet = __xuiTabsInvalidateAll(pTabs, pData,
				XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
			if ( iRet != XUI_OK ) return iRet;
			return XUI_EVENT_DISPATCH_STOP;
		}
		break;
	case XUI_EVENT_POINTER_CAPTURE_LOST:
		if ( pData->iActiveIndex == iIndex &&
		     xuiInternalPointerOwnerCaptureLost(&pData->tPointerOwner, pEvent) ) {
			pData->iActiveIndex = -1;
			pData->iCloseActiveIndex = -1;
			iRet = __xuiTabsInvalidateAll(pTabs, pData,
				XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
			if ( iRet != XUI_OK ) return iRet;
		}
		return XUI_EVENT_DISPATCH_STOP;
	default:
		break;
	}
	return XUI_OK;
}

static xui_menu_item_t __xuiTabsMenuItem(const char* sText, int iIndex, int bEnabled, int bChecked)
{
	xui_menu_item_t tItem;

	memset(&tItem, 0, sizeof(tItem));
	tItem.sText = sText;
	tItem.iType = XUI_MENU_ITEM_RADIO;
	tItem.iState = (bEnabled ? XUI_MENU_ITEM_ENABLED : 0u) | (bChecked ? XUI_MENU_ITEM_CHECKED : 0u);
	tItem.iValue = iIndex;
	return tItem;
}

static int __xuiTabsEnsureOverflowMenu(xui_widget pTabs, xui_tabs_data_t* pData)
{
	xui_menu_desc_t tMenuDesc;
	xui_widget pMenu;
	int iRet;

	if ( (pTabs == NULL) || (pData == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( pData->pOverflowMenu != NULL ) {
		return XUI_OK;
	}
	memset(&tMenuDesc, 0, sizeof(tMenuDesc));
	tMenuDesc.iSize = sizeof(tMenuDesc);
	tMenuDesc.pOwner = NULL;
	tMenuDesc.pFont = pData->pFont;
	pMenu = NULL;
	iRet = xuiMenuCreate(xuiWidgetGetContext(pTabs), &pMenu, &tMenuDesc);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiMenuSetSelect(pMenu, __xuiTabsOverflowMenuSelect, pTabs);
	if ( iRet != XUI_OK ) {
		xuiWidgetDestroy(pMenu);
		return iRet;
	}
	pData->pOverflowMenu = pMenu;
	return XUI_OK;
}

static int __xuiTabsBuildOverflowMenu(xui_widget pTabs, xui_tabs_data_t* pData)
{
	xui_menu_item_t arrItems[XUI_MENU_ITEM_CAPACITY];
	int i;
	int iCount;
	int iRet;

	if ( (pTabs == NULL) || (pData == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	iRet = __xuiTabsEnsureOverflowMenu(pTabs, pData);
	if ( iRet != XUI_OK ) return iRet;
	memset(arrItems, 0, sizeof(arrItems));
	iCount = 0;
	for ( i = 0; (i < pData->iPageCount) && (iCount < XUI_MENU_ITEM_CAPACITY); i++ ) {
		if ( pData->bOverflow && (i >= pData->iFirstVisibleTab) &&
			 (i < pData->iFirstVisibleTab + pData->iVisibleTabCount) ) {
			continue;
		}
		arrItems[iCount++] = __xuiTabsMenuItem(
			pData->arrPages[i].sTitle,
			i,
			pData->arrPages[i].bEnabled != 0,
			i == pData->iSelected);
	}
	return xuiMenuSetItems(pData->pOverflowMenu, arrItems, iCount);
}

static int __xuiTabsOpenOverflowMenu(xui_widget pTabs, xui_tabs_data_t* pData)
{
	xui_rect_t tWorld;
	xui_rect_t tAnchor;
	int iRet;

	if ( (pTabs == NULL) || (pData == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	iRet = __xuiTabsBuildOverflowMenu(pTabs, pData);
	if ( iRet != XUI_OK ) return iRet;
	tAnchor = (pData->tOverflowRect.fW > 0.0f) ? pData->tOverflowRect : pData->tTabBarRect;
	tWorld = xuiWidgetGetWorldRect(pTabs);
	return xuiMenuOpenAt(pData->pOverflowMenu, pTabs, tWorld.fX + tAnchor.fX, tWorld.fY + tAnchor.fY + tAnchor.fH);
}

static void __xuiTabsOverflowMenuSelect(xui_widget pMenu, int iIndex, int iValue, void* pUser)
{
	xui_widget pTabs;
	xui_tabs_data_t* pData;

	(void)pMenu;
	(void)iIndex;
	pTabs = (xui_widget)pUser;
	pData = __xuiTabsGetData(pTabs);
	if ( (pData == NULL) || (iValue < 0) || (iValue >= pData->iPageCount) ) {
		return;
	}
	(void)__xuiTabsSelectInternal(pTabs, pData, iValue, 1);
}

static int __xuiTabsOverflowHit(xui_widget pTabs, xui_tabs_data_t* pData, const xui_event_t* pEvent)
{
	xui_rect_t tWorld;
	float fX;
	float fY;

	if ( (pTabs == NULL) || (pData == NULL) || (pEvent == NULL) || !pData->bOverflow || (pData->tOverflowRect.fW <= 0.0f) || (pData->tOverflowRect.fH <= 0.0f) ) {
		return 0;
	}
	tWorld = xuiWidgetGetWorldRect(pTabs);
	fX = pEvent->fX - tWorld.fX;
	fY = pEvent->fY - tWorld.fY;
	return __xuiTabsRectContains(pData->tOverflowRect, fX, fY);
}

static int __xuiTabsSetOverflowHover(xui_widget pTabs, xui_tabs_data_t* pData, int bHover)
{
	if ( (pTabs == NULL) || (pData == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	bHover = bHover ? 1 : 0;
	if ( pData->bOverflowHover == bHover ) {
		return XUI_OK;
	}
	pData->bOverflowHover = bHover;
	return xuiWidgetInvalidate(pTabs, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
}

static int __xuiTabsTabBarEvent(xui_widget pTabBar, const xui_event_t* pEvent, void* pUser)
{
	xui_widget pTabs;
	xui_tabs_data_t* pData;
	float fDelta;
	float fOld;
	int bHit;
	int iRet;

	pTabs = (xui_widget)pUser;
	pData = __xuiTabsGetData(pTabs);
	if ( (pEvent == NULL) || (pData == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( pEvent->iType == XUI_EVENT_POINTER_WHEEL ) {
		if ( !pData->bScrollable ) {
			return XUI_OK;
		}
		fDelta = (pEvent->fWheelX != 0.0f) ? pEvent->fWheelX : -pEvent->fWheelY;
		if ( fDelta == 0.0f ) {
			return XUI_OK;
		}
		fOld = pData->fScrollX;
		pData->fScrollX += (fDelta > 0.0f ? 1.0f : -1.0f) * __xuiTabsMaxFloat(24.0f, __xuiTabsTabStep(pData));
		__xuiTabsUpdateMaxScroll(pData);
		if ( fOld != pData->fScrollX ) {
			iRet = __xuiTabsInvalidateAll(pTabs, pData,
				XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
			if ( iRet != XUI_OK ) return iRet;
		}
		return XUI_EVENT_DISPATCH_STOP;
	}
	bHit = __xuiTabsOverflowHit(pTabs, pData, pEvent);
	switch ( pEvent->iType ) {
	case XUI_EVENT_POINTER_ENTER:
	case XUI_EVENT_POINTER_MOVE:
		if ( pData->tPointerOwner.bActive &&
		     !xuiInternalPointerOwnerMatches(&pData->tPointerOwner, pEvent) ) {
			return XUI_OK;
		}
		iRet = __xuiTabsSetOverflowHover(pTabs, pData, bHit);
		if ( iRet != XUI_OK ) return iRet;
		return bHit ? XUI_EVENT_DISPATCH_STOP : XUI_OK;
	case XUI_EVENT_POINTER_LEAVE:
		iRet = __xuiTabsSetOverflowHover(pTabs, pData, 0);
		if ( iRet != XUI_OK ) return iRet;
		return XUI_OK;
	case XUI_EVENT_POINTER_DOWN:
		if ( (pEvent->iButton == XUI_POINTER_BUTTON_LEFT) && bHit ) {
			if ( pData->tPointerOwner.bActive ) {
				return xuiInternalPointerOwnerMatches(&pData->tPointerOwner, pEvent) ?
					XUI_EVENT_DISPATCH_STOP : XUI_OK;
			}
			pData->bOverflowActive = 1;
			pData->bOverflowHover = 1;
			iRet = xuiSetFocusWidget(xuiWidgetGetContext(pTabs), pTabs);
			if ( iRet == XUI_OK && !__xuiTabsWidgetAlive(pTabs, pData) ) {
				return XUI_EVENT_DISPATCH_STOP;
			}
			if ( iRet == XUI_OK ) iRet = xuiInternalPointerOwnerBegin(
				xuiWidgetGetContext(pTabs), pTabBar, pEvent, &pData->tPointerOwner);
			if ( iRet == XUI_OK ) iRet = xuiWidgetInvalidate(pTabs,
				XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
			if ( iRet != XUI_OK ) {
				(void)__xuiTabsClearInteraction(pTabs, pData);
				return iRet;
			}
			return XUI_EVENT_DISPATCH_STOP;
		}
		break;
	case XUI_EVENT_POINTER_UP:
		if ( pData->bOverflowActive &&
		     xuiInternalPointerOwnerMatches(&pData->tPointerOwner, pEvent) ) {
			pData->bOverflowActive = 0;
			iRet = xuiInternalPointerOwnerEnd(xuiWidgetGetContext(pTabs), pTabBar,
				pEvent, &pData->tPointerOwner);
			if ( iRet == XUI_OK ) iRet = xuiWidgetInvalidate(pTabs,
				XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
			if ( iRet != XUI_OK ) return iRet;
			if ( bHit ) {
				iRet = __xuiTabsOpenOverflowMenu(pTabs, pData);
				if ( iRet != XUI_OK ) return iRet;
			}
			return XUI_EVENT_DISPATCH_STOP;
		}
		break;
	case XUI_EVENT_POINTER_CAPTURE_LOST:
		if ( pData->bOverflowActive &&
		     xuiInternalPointerOwnerCaptureLost(&pData->tPointerOwner, pEvent) ) {
			pData->bOverflowActive = 0;
			iRet = xuiWidgetInvalidate(pTabs, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
			if ( iRet != XUI_OK ) return iRet;
		}
		return XUI_EVENT_DISPATCH_STOP;
	default:
		break;
	}
	return XUI_OK;
}

static int __xuiTabsEvent(xui_widget pWidget, const xui_event_t* pEvent, void* pUser)
{
	xui_tabs_data_t* pData;
	int iNext;
	int iIndex;
	int iRet;
	float fX;
	float fY;
	xui_rect_t tAnchor;
	xui_rect_t tWorld;

	(void)pUser;
	pData = __xuiTabsGetData(pWidget);
	if ( (pData == NULL) || (pEvent == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	switch ( pEvent->iType ) {
	case XUI_EVENT_CONTEXT_MENU:
		if ( !xuiWidgetGetEffectiveEnabled(pWidget) || pEvent->iPhase == XUI_EVENT_PHASE_CAPTURE ) return XUI_OK;
		iIndex = -1;
		if ( pEvent->iKey == XUI_KEY_CONTEXT_MENU ) {
			if ( pEvent->pTarget != pWidget || pEvent->iPhase != XUI_EVENT_PHASE_TARGET ) return XUI_OK;
			iIndex = pData->iSelected;
		} else {
			iIndex = __xuiTabsButtonIndex(pData, pEvent->pTarget);
			if ( iIndex < 0 ) {
				tWorld = xuiWidgetGetWorldRect(pWidget);
				for ( iNext = 0; iNext < pData->iPageCount; iNext++ ) {
					xui_rect_t tTab = pData->arrPages[iNext].tTabRect;
					tTab.fX += tWorld.fX;
					tTab.fY += tWorld.fY;
					if ( __xuiTabsRectContains(tTab, pEvent->fX, pEvent->fY) ) { iIndex = iNext; break; }
				}
			}
			if ( iIndex < 0 ) {
				tWorld = xuiWidgetGetWorldRect(pWidget);
				if ( !__xuiTabsRectContains(pData->tTabBarRect,
					pEvent->fX - tWorld.fX, pEvent->fY - tWorld.fY) ) return XUI_OK;
			}
		}
		tWorld = xuiWidgetGetWorldRect(pWidget);
		tAnchor = tWorld;
		if ( iIndex >= 0 && iIndex < pData->iPageCount ) {
			tAnchor = pData->arrPages[iIndex].tTabRect;
			tAnchor.fX += tWorld.fX;
			tAnchor.fY += tWorld.fY;
			if ( __xuiTabsItemEnabled(pData, iIndex) ) {
				iRet = __xuiTabsSelectInternal(pWidget, pData, iIndex, 1);
				if ( iRet != XUI_OK ) return iRet;
				if ( !__xuiTabsWidgetAlive(pWidget, pData) ) return XUI_EVENT_DISPATCH_STOP;
			}
		} else {
			iIndex = -1;
		}
		xuiInternalContextMenuPoint(pEvent, tAnchor, &fX, &fY);
		iRet = xuiSetFocusWidget(xuiWidgetGetContext(pWidget), pWidget);
		if ( iRet != XUI_OK ) return iRet;
		if ( !__xuiTabsWidgetAlive(pWidget, pData) ) return XUI_EVENT_DISPATCH_STOP;
		if ( pData->onContext != NULL ) return pData->onContext(pWidget, iIndex, fX, fY, pData->pContextUser);
		return XUI_OK;
	case XUI_EVENT_KEY_DOWN:
		if ( !xuiWidgetGetEffectiveEnabled(pWidget) ) {
			return XUI_OK;
		}
		iNext = -1;
		if ( (pEvent->iKey == XUI_KEY_TAB) && ((pEvent->iModifiers & XUI_MOD_CTRL) != 0u) ) {
			iNext = __xuiTabsStepEnabled(pData, pData->iSelected,
				((pEvent->iModifiers & XUI_MOD_SHIFT) != 0u) ? -1 : 1);
		} else if ( pEvent->pTarget != pWidget || pEvent->iPhase != XUI_EVENT_PHASE_TARGET ) {
			return XUI_OK;
		} else if ( (pEvent->iKey == XUI_KEY_RIGHT) || (pEvent->iKey == XUI_KEY_DOWN) ) {
			iNext = __xuiTabsStepEnabled(pData, pData->iSelected, 1);
		} else if ( (pEvent->iKey == XUI_KEY_LEFT) || (pEvent->iKey == XUI_KEY_UP) ) {
			iNext = __xuiTabsStepEnabled(pData, pData->iSelected, -1);
		} else if ( pEvent->iKey == XUI_KEY_HOME ) {
			iNext = __xuiTabsFirstEnabled(pData);
		} else if ( pEvent->iKey == XUI_KEY_END ) {
			iNext = __xuiTabsLastEnabled(pData);
		} else if ( pEvent->iKey == XUI_KEY_ESCAPE ) {
			iRet = __xuiTabsClearInteraction(pWidget, pData);
			if ( iRet != XUI_OK ) return iRet;
			return XUI_EVENT_DISPATCH_STOP;
		}
		if ( iNext >= 0 ) {
			iRet = __xuiTabsSelectInternal(pWidget, pData, iNext, 1);
			if ( iRet != XUI_OK ) return iRet;
			return XUI_EVENT_DISPATCH_STOP;
		}
		break;
	case XUI_EVENT_FOCUS:
		if ( pEvent->pTarget != pWidget ) return XUI_OK;
		iRet = __xuiTabsSyncPages(pWidget, pData);
		if ( iRet != XUI_OK ) return iRet;
		return XUI_EVENT_DISPATCH_STOP;
	case XUI_EVENT_BLUR:
		if ( pEvent->pTarget != pWidget ) return XUI_OK;
		iRet = __xuiTabsClearInteraction(pWidget, pData);
		if ( iRet != XUI_OK ) return iRet;
		iRet = __xuiTabsSyncPages(pWidget, pData);
		if ( iRet != XUI_OK ) return iRet;
		return XUI_EVENT_DISPATCH_STOP;
	case XUI_EVENT_ENABLED_CHANGED:
	case XUI_EVENT_VISIBLE_CHANGED:
		if ( pEvent->pTarget != pWidget ) return XUI_OK;
		if ( !xuiWidgetGetEffectiveEnabled(pWidget) || !xuiWidgetGetEffectiveVisible(pWidget) ) {
			iRet = __xuiTabsClearAllInteraction(pWidget, pData);
			if ( iRet != XUI_OK ) return iRet;
			if ( pData->pOverflowMenu != NULL && xuiMenuIsOpen(pData->pOverflowMenu) ) {
				iRet = xuiMenuClose(pData->pOverflowMenu);
				if ( iRet != XUI_OK ) return iRet;
				if ( !__xuiTabsWidgetAlive(pWidget, pData) ) return XUI_EVENT_DISPATCH_STOP;
			}
		}
		iRet = __xuiTabsSyncPages(pWidget, pData);
		if ( iRet != XUI_OK ) return iRet;
		return XUI_EVENT_DISPATCH_STOP;
	default:
		break;
	}
	return XUI_OK;
}

static int __xuiTabsConfigurePage(xui_widget pPage)
{
	int iRet;

	if ( pPage == NULL ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	iRet = xuiWidgetSetLayoutType(pPage, XUI_LAYOUT_COLUMN);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetSizeMode(pPage, XUI_SIZE_FILL, XUI_SIZE_FILL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetFlex(pPage, 1.0f, 1.0f);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetPadding(pPage, __xuiTabsThickness(10.0f, 10.0f, 10.0f, 10.0f));
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetGap(pPage, 6.0f);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetOverflow(pPage, XUI_OVERFLOW_CLIP);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetAlign(pPage, XUI_ALIGN_STRETCH, XUI_ALIGN_STRETCH);
	return iRet;
}

static void __xuiTabsDefaultCachePolicy(xui_cache_policy_t* pPolicy)
{
	memset(pPolicy, 0, sizeof(*pPolicy));
	pPolicy->iSize = sizeof(*pPolicy);
	pPolicy->iPolicy = XUI_CACHE_POLICY_SELF;
	pPolicy->iFlags = XUI_CACHE_CLEAR_ON_UPDATE | XUI_CACHE_UPDATE_ALL_STATES;
	pPolicy->iClearColor = XUI_COLOR_RGBA(0, 0, 0, 0);
}

static int __xuiTabsConfigureButton(xui_widget pTabs, xui_widget pButton)
{
	xui_cache_policy_t tPolicy;
	int iRet;

	if ( (pTabs == NULL) || (pButton == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	__xuiTabsDefaultCachePolicy(&tPolicy);
	iRet = xuiWidgetSetCachePolicy(pButton, &tPolicy);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetLayoutType(pButton, XUI_LAYOUT_MANUAL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetSizeMode(pButton, XUI_SIZE_FIXED, XUI_SIZE_FIXED);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetOverflow(pButton, XUI_OVERFLOW_CLIP);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetFocusable(pButton, 0);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetTabStop(pButton, 0);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pButton, XUI_EVENT_POINTER_ENTER, __xuiTabsButtonEvent, pTabs);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pButton, XUI_EVENT_POINTER_LEAVE, __xuiTabsButtonEvent, pTabs);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pButton, XUI_EVENT_POINTER_MOVE, __xuiTabsButtonEvent, pTabs);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pButton, XUI_EVENT_POINTER_DOWN, __xuiTabsButtonEvent, pTabs);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pButton, XUI_EVENT_POINTER_UP, __xuiTabsButtonEvent, pTabs);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pButton, XUI_EVENT_POINTER_CAPTURE_LOST, __xuiTabsButtonEvent, pTabs);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetCacheRenderCallback(pButton, __xuiTabsButtonRender, pTabs);
	return iRet;
}

static int __xuiTabsCreatePage(xui_widget pTabs, xui_tabs_data_t* pData, int iIndex)
{
	xui_widget pButton;
	xui_widget pPage;
	int iRet;

	if ( (pTabs == NULL) || (pData == NULL) || (iIndex < 0) || (iIndex >= XUI_TABS_PAGE_CAPACITY) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( pData->arrPages[iIndex].pButton != NULL ) {
		return XUI_OK;
	}
	iRet = xuiWidgetCreate(xuiWidgetGetContext(pTabs), &pButton);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiWidgetCreate(xuiWidgetGetContext(pTabs), &pPage);
	if ( iRet != XUI_OK ) {
		xuiWidgetDestroy(pButton);
		return iRet;
	}
	iRet = __xuiTabsConfigureButton(pTabs, pButton);
	if ( iRet == XUI_OK ) iRet = __xuiTabsConfigurePage(pPage);
	if ( iRet == XUI_OK ) iRet = xuiWidgetAddChild(pData->pTabBar, pButton);
	if ( iRet == XUI_OK ) iRet = xuiWidgetAddChild(pData->pClient, pPage);
	if ( iRet != XUI_OK ) {
		xuiWidgetDestroy(pButton);
		xuiWidgetDestroy(pPage);
		return iRet;
	}
	pData->arrPages[iIndex].pButton = pButton;
	pData->arrPages[iIndex].pPage = pPage;
	pData->arrPages[iIndex].bEnabled = 1;
	pData->arrPages[iIndex].bDirty = 0;
	iRet = xuiWidgetSetVisible(pButton, 0);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetVisible(pPage, 0);
	if ( iRet != XUI_OK ) {
		memset(&pData->arrPages[iIndex], 0, sizeof(pData->arrPages[iIndex]));
		xuiWidgetDestroy(pButton);
		xuiWidgetDestroy(pPage);
		return iRet;
	}
	return XUI_OK;
}

static void __xuiTabsCopyTitle(char sOutput[XUI_TABS_TITLE_CAPACITY], const char* sTitle)
{
	(void)xuiInternalUtf8CopyGraphemes(sOutput, XUI_TABS_TITLE_CAPACITY, sTitle);
}

static void __xuiTabsSetTitle(xui_tabs_page_t* pPage, const char* sTitle)
{
	if ( pPage == NULL ) return;
	__xuiTabsCopyTitle(pPage->sTitle, sTitle);
}

static int __xuiTabsApplyItems(xui_widget pTabs, xui_tabs_data_t* pData, const char** arrItems, int iItemCount)
{
	xui_tabs_page_t arrOldPages[XUI_TABS_PAGE_CAPACITY];
	char arrNewTitles[XUI_TABS_PAGE_CAPACITY][XUI_TABS_TITLE_CAPACITY];
	float fOldScroll;
	int iOldPageCount;
	int iOldSelected;
	int iOldChangeCount;
	int iOldHoverIndex;
	int iOldActiveIndex;
	int iOldCloseHoverIndex;
	int iOldCloseActiveIndex;
	int bSameItems;
	int i;
	int iRet;

	if ( (pTabs == NULL) || (pData == NULL) || (iItemCount < 0) || (iItemCount > XUI_TABS_PAGE_CAPACITY) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	bSameItems = pData->iPageCount == iItemCount;
	for ( i = 0; i < iItemCount; i++ ) {
		__xuiTabsCopyTitle(arrNewTitles[i], (arrItems != NULL) ? arrItems[i] : "");
		if ( bSameItems && strcmp(pData->arrPages[i].sTitle, arrNewTitles[i]) != 0 ) {
			bSameItems = 0;
		}
	}
	if ( bSameItems ) return XUI_OK;
	iRet = __xuiTabsClearAllInteraction(pTabs, pData);
	if ( iRet != XUI_OK ) return iRet;
	memcpy(arrOldPages, pData->arrPages, sizeof(arrOldPages));
	iOldPageCount = pData->iPageCount;
	iOldSelected = pData->iSelected;
	iOldChangeCount = pData->iChangeCount;
	iOldHoverIndex = pData->iHoverIndex;
	iOldActiveIndex = pData->iActiveIndex;
	iOldCloseHoverIndex = pData->iCloseHoverIndex;
	iOldCloseActiveIndex = pData->iCloseActiveIndex;
	fOldScroll = pData->fScrollX;
	for ( i = 0; i < iItemCount; i++ ) {
		iRet = __xuiTabsCreatePage(pTabs, pData, i);
		if ( iRet != XUI_OK ) goto rollback;
		__xuiTabsSetTitle(&pData->arrPages[i], arrNewTitles[i]);
	}
	pData->iPageCount = iItemCount;
	if ( (pData->iSelected < 0) || (pData->iSelected >= pData->iPageCount) || !__xuiTabsItemEnabled(pData, pData->iSelected) ) {
		pData->iSelected = __xuiTabsFirstEnabled(pData);
	}
	if ( pData->iHoverIndex >= pData->iPageCount ) pData->iHoverIndex = -1;
	if ( pData->iActiveIndex >= pData->iPageCount ) pData->iActiveIndex = -1;
	if ( pData->iCloseHoverIndex >= pData->iPageCount ) pData->iCloseHoverIndex = -1;
	if ( pData->iCloseActiveIndex >= pData->iPageCount ) pData->iCloseActiveIndex = -1;
	if ( iOldSelected != pData->iSelected ) pData->iChangeCount++;
	if ( pData->pOverflowMenu != NULL ) {
		iRet = xuiMenuClose(pData->pOverflowMenu);
		if ( iRet == XUI_OK ) iRet = xuiMenuClear(pData->pOverflowMenu);
		if ( iRet != XUI_OK ) goto rollback;
	}
	iRet = __xuiTabsSyncPages(pTabs, pData);
	if ( iRet != XUI_OK ) goto rollback;
	xuiInternalAccessibilityQueue(pTabs, XUI_ACCESSIBLE_EVENT_TREE_CHANGED);
	if ( iOldSelected != pData->iSelected ) {
		xuiInternalAccessibilityQueue(pTabs, XUI_ACCESSIBLE_EVENT_SELECTION_CHANGED);
	}
	return XUI_OK;

rollback:
	for ( i = 0; i < XUI_TABS_PAGE_CAPACITY; i++ ) {
		if ( arrOldPages[i].pButton == NULL && pData->arrPages[i].pButton != NULL ) {
			xuiWidgetDestroy(pData->arrPages[i].pButton);
			xuiWidgetDestroy(pData->arrPages[i].pPage);
		}
	}
	memcpy(pData->arrPages, arrOldPages, sizeof(arrOldPages));
	pData->iPageCount = iOldPageCount;
	pData->iSelected = iOldSelected;
	pData->iChangeCount = iOldChangeCount;
	pData->iHoverIndex = iOldHoverIndex;
	pData->iActiveIndex = iOldActiveIndex;
	pData->iCloseHoverIndex = iOldCloseHoverIndex;
	pData->iCloseActiveIndex = iOldCloseActiveIndex;
	pData->fScrollX = fOldScroll;
	/* Preserve the original failure; restoring widget state is best-effort. */
	(void)__xuiTabsSyncPages(pTabs, pData);
	return iRet;
}

static int __xuiTabsDrawBorderGap(xui_proxy pProxy, xui_draw_context pDraw, xui_rect_t tRect, int iPlacement, xui_rect_t tGap, uint32_t iColor)
{
	float fLeft;
	float fTop;
	float fRight;
	float fBottom;
	float fGapStart;
	float fGapEnd;
	int iRet;

	fLeft = tRect.fX;
	fTop = tRect.fY;
	if ( (tRect.fW <= 0.0f) || (tRect.fH <= 0.0f) ) {
		return XUI_OK;
	}
	fRight = tRect.fX + tRect.fW;
	fBottom = tRect.fY + tRect.fH;
	iRet = XUI_OK;
	if ( iPlacement == XUI_TABS_PLACEMENT_TOP ) {
		iRet = __xuiTabsDrawEdgeRect(pProxy, pDraw, tRect, iColor, 1, 0, 1, 1);
		fGapStart = __xuiTabsClampFloat(tGap.fX, fLeft, fRight);
		fGapEnd = __xuiTabsClampFloat(tGap.fX + tGap.fW, fLeft, fRight);
		if ( iRet == XUI_OK && fGapStart > fLeft ) iRet = __xuiTabsDrawFill(pProxy, pDraw, (xui_rect_t){fLeft, fTop, fGapStart - fLeft, 1.0f}, iColor);
		if ( iRet == XUI_OK && fGapEnd < fRight ) iRet = __xuiTabsDrawFill(pProxy, pDraw, (xui_rect_t){fGapEnd, fTop, fRight - fGapEnd, 1.0f}, iColor);
	} else if ( iPlacement == XUI_TABS_PLACEMENT_BOTTOM ) {
		iRet = __xuiTabsDrawEdgeRect(pProxy, pDraw, tRect, iColor, 1, 1, 1, 0);
		fGapStart = __xuiTabsClampFloat(tGap.fX, fLeft, fRight);
		fGapEnd = __xuiTabsClampFloat(tGap.fX + tGap.fW, fLeft, fRight);
		if ( iRet == XUI_OK && fGapStart > fLeft ) iRet = __xuiTabsDrawFill(pProxy, pDraw, (xui_rect_t){fLeft, fBottom - 1.0f, fGapStart - fLeft, 1.0f}, iColor);
		if ( iRet == XUI_OK && fGapEnd < fRight ) iRet = __xuiTabsDrawFill(pProxy, pDraw, (xui_rect_t){fGapEnd, fBottom - 1.0f, fRight - fGapEnd, 1.0f}, iColor);
	} else if ( iPlacement == XUI_TABS_PLACEMENT_LEFT ) {
		iRet = __xuiTabsDrawEdgeRect(pProxy, pDraw, tRect, iColor, 0, 1, 1, 1);
		fGapStart = __xuiTabsClampFloat(tGap.fY, fTop, fBottom);
		fGapEnd = __xuiTabsClampFloat(tGap.fY + tGap.fH, fTop, fBottom);
		if ( iRet == XUI_OK && fGapStart > fTop ) iRet = __xuiTabsDrawFill(pProxy, pDraw, (xui_rect_t){fLeft, fTop, 1.0f, fGapStart - fTop}, iColor);
		if ( iRet == XUI_OK && fGapEnd < fBottom ) iRet = __xuiTabsDrawFill(pProxy, pDraw, (xui_rect_t){fLeft, fGapEnd, 1.0f, fBottom - fGapEnd}, iColor);
	} else {
		iRet = __xuiTabsDrawEdgeRect(pProxy, pDraw, tRect, iColor, 1, 1, 0, 1);
		fGapStart = __xuiTabsClampFloat(tGap.fY, fTop, fBottom);
		fGapEnd = __xuiTabsClampFloat(tGap.fY + tGap.fH, fTop, fBottom);
		if ( iRet == XUI_OK && fGapStart > fTop ) iRet = __xuiTabsDrawFill(pProxy, pDraw, (xui_rect_t){fRight - 1.0f, fTop, 1.0f, fGapStart - fTop}, iColor);
		if ( iRet == XUI_OK && fGapEnd < fBottom ) iRet = __xuiTabsDrawFill(pProxy, pDraw, (xui_rect_t){fRight - 1.0f, fGapEnd, 1.0f, fBottom - fGapEnd}, iColor);
	}
	return iRet;
}

static int __xuiTabsDrawOverflowButton(xui_proxy pProxy, xui_draw_context pDraw,
	const xui_tabs_data_t* pData, const xui_tabs_data_t* pColors, int bEnabled)
{
	xui_rect_t tRect;
	float fCX;
	float fCY;
	float fSize;
	uint32_t iFill;
	uint32_t iLine;
	int iRet;

	if ( (pData == NULL) || (pColors == NULL) || !pData->bOverflow || (pData->tOverflowRect.fW <= 0.0f) || (pData->tOverflowRect.fH <= 0.0f) ) {
		return XUI_OK;
	}
	tRect = pData->tOverflowRect;
	iFill = bEnabled ?
		(pData->bOverflowActive ? pColors->iHoverColor : (pData->bOverflowHover ? pColors->iHoverColor : 0u)) :
		pColors->iDisabledColor;
	iLine = bEnabled ? pColors->iTextColor : pColors->iDisabledTextColor;
	iRet = __xuiTabsDrawFill(pProxy, pDraw, tRect, iFill);
	if ( iRet == XUI_OK && bEnabled && (pData->bOverflowHover || pData->bOverflowActive) ) {
		iRet = __xuiTabsDrawEdgeRect(pProxy, pDraw, tRect, pColors->iFocusColor, 1, 1, 1, 1);
	}
	if ( iRet != XUI_OK ) return iRet;
	fCX = tRect.fX + tRect.fW * 0.5f;
	fCY = tRect.fY + tRect.fH * 0.5f;
	fSize = __xuiTabsMinFloat(tRect.fW, tRect.fH) * 0.16f;
	if ( fSize < 3.0f ) fSize = 3.0f;
	iRet = __xuiTabsDrawLine(pProxy, pDraw, fCX - fSize, fCY - fSize * 0.35f, fCX, fCY + fSize * 0.55f, iLine);
	if ( iRet == XUI_OK ) iRet = __xuiTabsDrawLine(pProxy, pDraw, fCX, fCY + fSize * 0.55f, fCX + fSize, fCY - fSize * 0.35f, iLine);
	return iRet;
}

static int __xuiTabsCacheRender(xui_widget pWidget, xui_draw_context pDraw, uint32_t iStateId, void* pUser)
{
	xui_tabs_data_t* pData;
	xui_tabs_data_t tColors;
	xui_proxy pProxy;
	xui_rect_t tRect;
	xui_rect_t tGap;
	int iRet;

	(void)iStateId;
	(void)pUser;
	pData = __xuiTabsGetData(pWidget);
	if ( (pData == NULL) || (pDraw == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pProxy = xuiInternalContextGetProxy(xuiWidgetGetContext(pWidget));
	if ( pProxy == NULL ) {
		return XUI_ERROR_NOT_INITIALIZED;
	}
	__xuiTabsResolveColors(pWidget, pData, &tColors);
	tRect = xuiWidgetGetContentRect(pWidget);
	iRet = __xuiTabsDrawFill(pProxy, pDraw, tRect, tColors.iBackgroundColor);
	if ( iRet == XUI_OK ) iRet = __xuiTabsDrawFill(pProxy, pDraw, pData->tTabBarRect, tColors.iBackgroundColor);
	if ( iRet == XUI_OK ) iRet = __xuiTabsDrawFill(pProxy, pDraw, pData->tClientRect, tColors.iClientColor);
	tGap = (pData->iSelected >= 0 && pData->iSelected < pData->iPageCount) ? pData->arrPages[pData->iSelected].tTabRect : (xui_rect_t){0.0f, 0.0f, 0.0f, 0.0f};
	if ( iRet == XUI_OK ) iRet = __xuiTabsDrawBorderGap(pProxy, pDraw, pData->tClientRect, pData->iPlacement, tGap, tColors.iBorderColor);
	if ( iRet == XUI_OK ) iRet = __xuiTabsDrawOverflowButton(pProxy, pDraw, pData,
		&tColors, xuiWidgetGetEffectiveEnabled(pWidget));
	return iRet;
}

static int __xuiTabsDrawVerticalText(xui_proxy pProxy, xui_draw_context pDraw, xui_font pFont, const char* sText, xui_rect_t tRect, uint32_t iColor)
{
	xui_font_metrics_t tMetrics;
	xui_text_item_t tItem;
	xui_rect_t tChar;
	float fLine;
	float fY;
	int iTextSize;
	int iAt;
	int iNext;
	int iRet;

	if ( (sText == NULL) || (*sText == 0) || (pFont == NULL) ) {
		return XUI_OK;
	}
	fLine = 14.0f;
	memset(&tMetrics, 0, sizeof(tMetrics));
	if ( pProxy != NULL && pProxy->fontGetMetrics != NULL &&
	     pProxy->fontGetMetrics(pProxy, pFont, &tMetrics) == XUI_OK &&
	     tMetrics.fLineHeight > 0.0f ) fLine = tMetrics.fLineHeight;
	iTextSize = (int)strlen(sText);
	fY = tRect.fY;
	for ( iAt = 0; iAt < iTextSize && fY < tRect.fY + tRect.fH; iAt = iNext ) {
		iNext = xuiInternalTextGraphemeNext(sText, iTextSize, iAt);
		if ( iNext <= iAt || iNext > iTextSize ) iNext = iAt + 1;
		tChar = (xui_rect_t){tRect.fX, fY, tRect.fW, fLine};
		memset(&tItem, 0, sizeof(tItem));
		tItem.iSize = sizeof(tItem);
		tItem.pFont = pFont;
		tItem.sText = sText + iAt;
		tItem.iTextSize = iNext - iAt;
		tItem.iFlags = XUI_TEXT_SHAPE_DEFAULT;
		iRet = __xuiTabsDrawText(pProxy, pDraw, &tItem, tChar, iColor,
			XUI_TEXT_ALIGN_CENTER | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP);
		if ( iRet != XUI_OK ) return iRet;
		fY += fLine;
	}
	return XUI_OK;
}

static int __xuiTabsDrawTabBorder(xui_proxy pProxy, xui_draw_context pDraw, xui_rect_t tRect, int iPlacement, uint32_t iColor)
{
	if ( (tRect.fW <= 0.0f) || (tRect.fH <= 0.0f) ) {
		return XUI_OK;
	}
	return __xuiTabsDrawEdgeRect(
		pProxy,
		pDraw,
		tRect,
		iColor,
		iPlacement != XUI_TABS_PLACEMENT_RIGHT,
		iPlacement != XUI_TABS_PLACEMENT_BOTTOM,
		iPlacement != XUI_TABS_PLACEMENT_LEFT,
		iPlacement != XUI_TABS_PLACEMENT_TOP);
}

static int __xuiTabsButtonRender(xui_widget pButton, xui_draw_context pDraw, uint32_t iStateId, void* pUser)
{
	xui_widget pTabs;
	xui_tabs_data_t* pData;
	xui_tabs_data_t tColors;
	xui_tabs_page_t* pPage;
	xui_proxy pProxy;
	xui_font pFont;
	xui_rect_t tRect;
	xui_rect_t tLocal;
	xui_rect_t tIcon;
	xui_rect_t tDirty;
	xui_rect_t tClose;
	uint32_t iFill;
	uint32_t iText;
	int iIndex;
	int bSelected;
	int bEnabled;
	int iRet;

	(void)iStateId;
	pTabs = (xui_widget)pUser;
	pData = __xuiTabsGetData(pTabs);
	if ( (pButton == NULL) || (pData == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	iIndex = __xuiTabsButtonIndex(pData, pButton);
	if ( iIndex < 0 ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pPage = &pData->arrPages[iIndex];
	pProxy = xuiInternalContextGetProxy(xuiWidgetGetContext(pTabs));
	if ( pProxy == NULL ) {
		return XUI_ERROR_NOT_INITIALIZED;
	}
	__xuiTabsResolveColors(pTabs, pData, &tColors);
	bSelected = (iIndex == pData->iSelected);
	bEnabled = xuiWidgetGetEffectiveEnabled(pTabs) && pPage->bEnabled;
	iFill = tColors.iTabColor;
	iText = tColors.iTextColor;
	if ( !bEnabled ) {
		iFill = tColors.iDisabledColor;
		iText = tColors.iDisabledTextColor;
	} else if ( bSelected ) {
		iFill = tColors.iClientColor;
		iText = tColors.iActiveTextColor;
	} else if ( pData->iActiveIndex == iIndex ) {
		iFill = tColors.iHoverColor;
	} else if ( pData->iHoverIndex == iIndex ) {
		iFill = tColors.iHoverColor;
	}
	tRect = (xui_rect_t){0.0f, 0.0f, pPage->tTabRect.fW, pPage->tTabRect.fH};
	iRet = __xuiTabsDrawFill(pProxy, pDraw, tRect, iFill);
	if ( iRet != XUI_OK ) return iRet;
	iRet = __xuiTabsDrawTabBorder(pProxy, pDraw, tRect, pData->iPlacement, tColors.iBorderColor);
	if ( iRet != XUI_OK ) return iRet;
	if ( bSelected ) {
		if ( pData->iPlacement == XUI_TABS_PLACEMENT_BOTTOM ) {
			iRet = __xuiTabsDrawFill(pProxy, pDraw, (xui_rect_t){1.0f, tRect.fH - 3.0f, __xuiTabsMaxFloat(0.0f, tRect.fW - 2.0f), 2.0f}, tColors.iActiveColor);
		} else if ( pData->iPlacement == XUI_TABS_PLACEMENT_LEFT ) {
			iRet = __xuiTabsDrawFill(pProxy, pDraw, (xui_rect_t){1.0f, 1.0f, 2.0f, __xuiTabsMaxFloat(0.0f, tRect.fH - 2.0f)}, tColors.iActiveColor);
		} else if ( pData->iPlacement == XUI_TABS_PLACEMENT_RIGHT ) {
			iRet = __xuiTabsDrawFill(pProxy, pDraw, (xui_rect_t){tRect.fW - 3.0f, 1.0f, 2.0f, __xuiTabsMaxFloat(0.0f, tRect.fH - 2.0f)}, tColors.iActiveColor);
		} else {
			iRet = __xuiTabsDrawFill(pProxy, pDraw, (xui_rect_t){1.0f, 1.0f, __xuiTabsMaxFloat(0.0f, tRect.fW - 2.0f), 2.0f}, tColors.iActiveColor);
		}
	}
	if ( iRet != XUI_OK ) return iRet;
	pFont = (pData->pFont != NULL) ? pData->pFont : xuiGetDefaultFont(xuiWidgetGetContext(pTabs));
	if ( pPage->pIcon != NULL && pProxy->drawSurface != NULL ) {
		tIcon = pPage->tIconRect;
		tIcon.fX -= pPage->tTabRect.fX;
		tIcon.fY -= pPage->tTabRect.fY;
		if ( (tIcon.fW > 0.0f) && (tIcon.fH > 0.0f) &&
		     (pPage->tIconSrc.fW > 0.0f) && (pPage->tIconSrc.fH > 0.0f) ) {
			iRet = pProxy->drawSurface(pProxy, pDraw, pPage->pIcon, pPage->tIconSrc,
				tIcon, XUI_COLOR_RGBA(255, 255, 255, 255), 0);
			if ( iRet != XUI_OK ) return iRet;
		}
	}
	tLocal = pPage->tTextRect;
	tLocal.fX -= pPage->tTabRect.fX;
	tLocal.fY -= pPage->tTabRect.fY;
	if ( __xuiTabsVertical(pData->iPlacement) ) {
		iRet = __xuiTabsDrawVerticalText(pProxy, pDraw, pFont, pPage->sTitle, tLocal, iText);
	} else {
		iRet = __xuiTabsDrawText(pProxy, pDraw, &(xui_text_item_t){.iSize=sizeof(xui_text_item_t), .pFont=pFont, .sText=pPage->sTitle, .iTextSize=-1, .iFlags=XUI_TEXT_SHAPE_DEFAULT | ((XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP) & XUI_TEXT_RTL ? XUI_TEXT_SHAPE_RTL : 0)}, tLocal, iText, XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP);
	}
	if ( iRet != XUI_OK ) return iRet;
	if ( pPage->bDirty ) {
		tDirty = pPage->tDirtyRect;
		tDirty.fX -= pPage->tTabRect.fX;
		tDirty.fY -= pPage->tTabRect.fY;
		iRet = __xuiTabsDrawCircle(pProxy, pDraw, tDirty.fX + tDirty.fW * 0.5f, tDirty.fY + tDirty.fH * 0.5f, __xuiTabsMinFloat(tDirty.fW, tDirty.fH) * 0.5f, tColors.iActiveColor);
		if ( iRet != XUI_OK ) return iRet;
	}
	if ( pData->bCloseButtons && pData->onClose != NULL ) {
		uint32_t iCloseColor;
		tClose = pPage->tCloseRect;
		tClose.fX -= pPage->tTabRect.fX;
		tClose.fY -= pPage->tTabRect.fY;
		iCloseColor = bEnabled ? tColors.iCloseColor : tColors.iCloseDisabledColor;
		if ( bEnabled && pData->iCloseActiveIndex == iIndex ) {
			iRet = __xuiTabsDrawFill(pProxy, pDraw, tClose, tColors.iCloseActiveBackgroundColor);
			if ( iRet != XUI_OK ) return iRet;
			iCloseColor = tColors.iCloseActiveColor;
		} else if ( pData->iCloseHoverIndex == iIndex ) {
			iRet = __xuiTabsDrawFill(pProxy, pDraw, tClose, tColors.iCloseHoverBackgroundColor);
			if ( iRet != XUI_OK ) return iRet;
			if ( bEnabled ) iCloseColor = tColors.iCloseHoverColor;
		}
		iRet = __xuiTabsDrawLine(pProxy, pDraw, tClose.fX + 5.0f, tClose.fY + 5.0f, tClose.fX + tClose.fW - 5.0f, tClose.fY + tClose.fH - 5.0f, iCloseColor);
		if ( iRet == XUI_OK ) iRet = __xuiTabsDrawLine(pProxy, pDraw, tClose.fX + tClose.fW - 5.0f, tClose.fY + 5.0f, tClose.fX + 5.0f, tClose.fY + tClose.fH - 5.0f, iCloseColor);
	}
	return iRet;
}

static int __xuiTabsContentMeasure(xui_widget pWidget, xui_vec2_t tConstraint, xui_vec2_t* pSize, void* pUser)
{
	xui_tabs_data_t* pData;

	(void)tConstraint;
	(void)pUser;
	pData = __xuiTabsGetData(pWidget);
	if ( (pData == NULL) || (pSize == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	__xuiTabsResolve(pWidget, pData);
	if ( __xuiTabsVertical(pData->iPlacement) ) {
		pSize->fX = 240.0f + pData->fResolvedTabHeight;
		pSize->fY = 180.0f;
	} else {
		pSize->fX = 320.0f;
		pSize->fY = 180.0f + pData->fResolvedTabHeight;
	}
	return XUI_OK;
}

static void __xuiTabsComputeRects(xui_tabs_data_t* pData, int iIndex, xui_rect_t tTab)
{
	xui_tabs_page_t* pPage;
	float fPad;
	float fClose;
	float fDirty;
	float fIcon;

	if ( (pData == NULL) || (iIndex < 0) || (iIndex >= pData->iPageCount) ) {
		return;
	}
	pPage = &pData->arrPages[iIndex];
	fPad = 12.0f;
	fClose = (pData->bCloseButtons && pData->onClose != NULL) ? 18.0f : 0.0f;
	fDirty = pPage->bDirty ? 10.0f : 0.0f;
	fIcon = (pPage->pIcon != NULL) ? 18.0f : 0.0f;
	pPage->tTabRect = tTab;
	if ( __xuiTabsVertical(pData->iPlacement) ) {
		pPage->tCloseRect = (xui_rect_t){tTab.fX + 4.0f, tTab.fY + tTab.fH - 22.0f, tTab.fW - 8.0f, 18.0f};
		pPage->tDirtyRect = (xui_rect_t){tTab.fX + tTab.fW - 9.0f, tTab.fY + 6.0f, 6.0f, 6.0f};
		pPage->tIconRect = (xui_rect_t){tTab.fX + (tTab.fW - 14.0f) * 0.5f, tTab.fY + 8.0f, 14.0f, 14.0f};
		pPage->tTextRect = (xui_rect_t){tTab.fX + 4.0f, tTab.fY + 8.0f + fIcon, tTab.fW - 8.0f, tTab.fH - 16.0f - fIcon - fClose};
	} else {
		pPage->tCloseRect = (xui_rect_t){tTab.fX + tTab.fW - 22.0f, tTab.fY + (tTab.fH - 18.0f) * 0.5f, 18.0f, 18.0f};
		pPage->tDirtyRect = (xui_rect_t){tTab.fX + tTab.fW - fClose - 12.0f, tTab.fY + 7.0f, 6.0f, 6.0f};
		pPage->tIconRect = (xui_rect_t){tTab.fX + fPad, tTab.fY + (tTab.fH - 14.0f) * 0.5f, 14.0f, 14.0f};
		pPage->tTextRect = (xui_rect_t){tTab.fX + fPad + fIcon, tTab.fY + 3.0f, tTab.fW - (fPad * 2.0f) - fIcon - fDirty - fClose, tTab.fH - 6.0f};
	}
	if ( pPage->tTextRect.fW < 0.0f ) pPage->tTextRect.fW = 0.0f;
	if ( pPage->tTextRect.fH < 0.0f ) pPage->tTextRect.fH = 0.0f;
}

static xui_table_track_t __xuiTabsTrack(int iMode, float fValue, float fWeight, float fShrink)
{
	xui_table_track_t tTrack;
	tTrack.iSizeMode = iMode;
	tTrack.fValue = fValue;
	tTrack.fMin = 0.0f;
	tTrack.fMax = XUI_LAYOUT_UNBOUNDED;
	tTrack.fWeight = fWeight;
	tTrack.fShrink = fShrink;
	return tTrack;
}

static int __xuiTabsSetCell(xui_widget pWidget, int iRow, int iColumn)
{
	int iCurrentRow;
	int iCurrentColumn;
	int iRowSpan;
	int iColumnSpan;
	int iRet = xuiWidgetGetTableCell(pWidget, &iCurrentRow, &iCurrentColumn, &iRowSpan, &iColumnSpan);
	if ( iRet != XUI_OK ) return iRet;
	if ( iCurrentRow == iRow && iCurrentColumn == iColumn && iRowSpan == 1 && iColumnSpan == 1 ) return XUI_OK;
	return xuiWidgetSetTableCell(pWidget, iRow, iColumn, 1, 1);
}

static int __xuiTabsPrepare(xui_widget pWidget, void* pUser)
{
	xui_tabs_data_t* pData = (xui_tabs_data_t*)pUser;
	xui_table_track_t tStrip;
	xui_table_track_t tFill;
	float fStrip;
	int iRows;
	int iColumns;
	int iTabRow;
	int iTabColumn;
	int iClientRow;
	int iClientColumn;
	int iRet;
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	__xuiTabsResolve(pWidget, pData);
	fStrip = pData->fResolvedTabHeight + 2.0f;
	iRows = __xuiTabsVertical(pData->iPlacement) ? 1 : 2;
	iColumns = __xuiTabsVertical(pData->iPlacement) ? 2 : 1;
	if ( pWidget->tLayout.iLayoutType != XUI_LAYOUT_TABLE ) {
		iRet = xuiWidgetSetLayoutType(pWidget, XUI_LAYOUT_TABLE);
		if ( iRet != XUI_OK ) return iRet;
	}
	if ( pWidget->iTableRowCount != iRows || pWidget->iTableColumnCount != iColumns ) {
		iRet = xuiWidgetSetTableSize(pWidget, iRows, iColumns);
		if ( iRet != XUI_OK ) return iRet;
	}
	tStrip = __xuiTabsTrack(XUI_SIZE_FIXED, fStrip, 0.0f, 1.0f);
	tFill = __xuiTabsTrack(XUI_SIZE_FILL, 0.0f, 1.0f, 1.0f);
	if ( __xuiTabsVertical(pData->iPlacement) ) {
		iRet = xuiWidgetSetTableRow(pWidget, 0, &tFill);
		if ( iRet != XUI_OK ) return iRet;
		iRet = xuiWidgetSetTableColumn(pWidget, 0,
			pData->iPlacement == XUI_TABS_PLACEMENT_LEFT ? &tStrip : &tFill);
		if ( iRet != XUI_OK ) return iRet;
		iRet = xuiWidgetSetTableColumn(pWidget, 1,
			pData->iPlacement == XUI_TABS_PLACEMENT_LEFT ? &tFill : &tStrip);
		iTabRow = iClientRow = 0;
		iTabColumn = pData->iPlacement == XUI_TABS_PLACEMENT_LEFT ? 0 : 1;
		iClientColumn = 1 - iTabColumn;
	} else {
		iRet = xuiWidgetSetTableColumn(pWidget, 0, &tFill);
		if ( iRet != XUI_OK ) return iRet;
		iRet = xuiWidgetSetTableRow(pWidget, 0,
			pData->iPlacement == XUI_TABS_PLACEMENT_TOP ? &tStrip : &tFill);
		if ( iRet != XUI_OK ) return iRet;
		iRet = xuiWidgetSetTableRow(pWidget, 1,
			pData->iPlacement == XUI_TABS_PLACEMENT_TOP ? &tFill : &tStrip);
		iTabColumn = iClientColumn = 0;
		iTabRow = pData->iPlacement == XUI_TABS_PLACEMENT_TOP ? 0 : 1;
		iClientRow = 1 - iTabRow;
	}
	if ( iRet != XUI_OK ) return iRet;
	iRet = __xuiTabsSetCell(pData->pTabBar, iTabRow, iTabColumn);
	if ( iRet != XUI_OK ) return iRet;
	return __xuiTabsSetCell(pData->pClient, iClientRow, iClientColumn);
}

static int __xuiTabsLayoutButtons(xui_widget pTabBar, xui_rect_t tContentRect, void* pUser)
{
	xui_widget pTabs = (xui_widget)pUser;
	xui_tabs_data_t* pData = __xuiTabsGetData(pTabs);
	xui_rect_t tTab;
	float fStrip;
	float fAxis;
	float fButtonAxis;
	float fStep;
	int iOverflow;
	int iVisibleCount;
	int iFirstVisible;
	int iVisible;
	int iRet;
	int i;
	if ( pData == NULL ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	fStrip = pData->fResolvedTabHeight + 2.0f;
	pData->tTabBarRect = xuiInternalSnapRect(tContentRect);
	__xuiTabsUpdateMaxScroll(pData);
	if ( (pData->iSelected >= 0) && (pData->iSelected < pData->iPageCount) ) {
		__xuiTabsEnsureVisibleInternal(pData, pData->iSelected);
	}
	iVisibleCount = __xuiTabsComputeVisibleCount(pData, &iOverflow);
	iFirstVisible = __xuiTabsComputeFirstVisible(pData, iVisibleCount);
	pData->bOverflow = iOverflow;
	pData->iVisibleTabCount = iVisibleCount;
	pData->iFirstVisibleTab = iFirstVisible;
	if ( iOverflow ) {
		pData->fScrollX = (float)iFirstVisible * __xuiTabsTabStep(pData);
		fButtonAxis = __xuiTabsOverflowButtonAxis(pData);
		if ( __xuiTabsVertical(pData->iPlacement) ) {
			pData->tOverflowRect = xuiInternalSnapRect((xui_rect_t){
				pData->tTabBarRect.fX + (pData->tTabBarRect.fW - fButtonAxis) * 0.5f,
				pData->tTabBarRect.fY + __xuiTabsMaxFloat(0.0f, pData->tTabBarRect.fH - fButtonAxis - XUI_TABS_TAB_END),
				fButtonAxis,
				fButtonAxis
			});
		} else {
			pData->tOverflowRect = xuiInternalSnapRect((xui_rect_t){
				pData->tTabBarRect.fX + __xuiTabsMaxFloat(0.0f, pData->tTabBarRect.fW - fButtonAxis - XUI_TABS_TAB_END),
				pData->tTabBarRect.fY + (pData->tTabBarRect.fH - fButtonAxis) * 0.5f,
				fButtonAxis,
				fButtonAxis
			});
		}
	} else {
		pData->tOverflowRect = (xui_rect_t){0.0f, 0.0f, 0.0f, 0.0f};
		pData->bOverflowHover = 0;
		pData->bOverflowActive = 0;
		pData->fScrollX = 0.0f;
	}
	fAxis = XUI_TABS_TAB_START;
	fStep = __xuiTabsTabStep(pData);
	for ( i = 0; i < pData->iPageCount; i++ ) {
		iVisible = (!pData->bOverflow || ((i >= iFirstVisible) && (i < iFirstVisible + iVisibleCount))) ? 1 : 0;
		if ( !iVisible ) {
			pData->arrPages[i].tTabRect = (xui_rect_t){0.0f, 0.0f, 0.0f, 0.0f};
			pData->arrPages[i].tTextRect = (xui_rect_t){0.0f, 0.0f, 0.0f, 0.0f};
			pData->arrPages[i].tIconRect = (xui_rect_t){0.0f, 0.0f, 0.0f, 0.0f};
			pData->arrPages[i].tDirtyRect = (xui_rect_t){0.0f, 0.0f, 0.0f, 0.0f};
			pData->arrPages[i].tCloseRect = (xui_rect_t){0.0f, 0.0f, 0.0f, 0.0f};
			if ( pData->arrPages[i].pButton != NULL ) {
				iRet = xuiLayoutArrangeChild(pTabBar, pData->arrPages[i].pButton,
					(xui_rect_t){0.0f, 0.0f, 0.0f, 0.0f});
				if ( iRet != XUI_OK ) return iRet;
			}
			continue;
		}
		if ( __xuiTabsVertical(pData->iPlacement) ) {
			tTab = (xui_rect_t){
				pData->tTabBarRect.fX + ((i == pData->iSelected) ? 0.0f : 2.0f),
				pData->tTabBarRect.fY + fAxis,
				(i == pData->iSelected) ? fStrip : pData->fResolvedTabHeight,
				pData->fResolvedTabWidth
			};
		} else {
			tTab = (xui_rect_t){
				pData->tTabBarRect.fX + fAxis,
				pData->tTabBarRect.fY + ((i == pData->iSelected) ? 0.0f : 2.0f),
				pData->fResolvedTabWidth,
				(i == pData->iSelected) ? fStrip : pData->fResolvedTabHeight
			};
		}
		tTab = xuiInternalSnapRect(tTab);
		__xuiTabsComputeRects(pData, i, tTab);
		if ( pData->arrPages[i].pButton != NULL ) {
			iRet = xuiLayoutArrangeChild(pTabBar, pData->arrPages[i].pButton, tTab);
			if ( iRet != XUI_OK ) return iRet;
		}
		fAxis += fStep;
	}
	return XUI_OK;
}

static void __xuiTabsOffsetRect(xui_rect_t* pRect, float fX, float fY)
{
	if ( pRect->fW <= 0.0f || pRect->fH <= 0.0f ) return;
	pRect->fX += fX;
	pRect->fY += fY;
}

static int __xuiTabsLayoutComplete(xui_widget pWidget, xui_rect_t tContentRect, void* pUser)
{
	xui_tabs_data_t* pData = (xui_tabs_data_t*)pUser;
	float fX;
	float fY;
	int i;
	(void)pWidget;
	(void)tContentRect;
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	pData->tTabBarRect = xuiWidgetGetRect(pData->pTabBar);
	pData->tClientRect = xuiWidgetGetRect(pData->pClient);
	fX = pData->tTabBarRect.fX;
	fY = pData->tTabBarRect.fY;
	__xuiTabsOffsetRect(&pData->tOverflowRect, fX, fY);
	for ( i = 0; i < pData->iPageCount; ++i ) {
		__xuiTabsOffsetRect(&pData->arrPages[i].tTabRect, fX, fY);
		__xuiTabsOffsetRect(&pData->arrPages[i].tTextRect, fX, fY);
		__xuiTabsOffsetRect(&pData->arrPages[i].tIconRect, fX, fY);
		__xuiTabsOffsetRect(&pData->arrPages[i].tDirtyRect, fX, fY);
		__xuiTabsOffsetRect(&pData->arrPages[i].tCloseRect, fX, fY);
	}
	return XUI_OK;
}

static int __xuiTabsAccessibleCount(xui_widget pWidget, void* pUser)
{
	const xui_tabs_data_t* pData = (const xui_tabs_data_t*)pUser;

	(void)pWidget;
	return pData != NULL ? pData->iPageCount + 1 : 0;
}

static int __xuiTabsAccessibleGet(xui_widget pWidget, int iIndex,
	xui_accessible_node_t* pNode, void* pUser)
{
	xui_tabs_data_t* pData = (xui_tabs_data_t*)pUser;
	xui_rect_t tWorld;
	xui_rect_t tTab;
	int iTab;
	int bDisabled;
	int bWidgetOffscreen;

	if ( pData == NULL || pNode == NULL || iIndex < 0 || iIndex > pData->iPageCount ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	tWorld = xuiWidgetGetWorldRect(pWidget);
	bWidgetOffscreen = !xuiWidgetGetEffectiveVisible(pWidget) ||
		tWorld.fW <= 0.0f || tWorld.fH <= 0.0f;
	if ( iIndex == 0 ) {
		pNode->iId = 1;
		pNode->iRole = XUI_ACCESSIBLE_ROLE_TAB_LIST;
		pNode->sName = xuiWidgetGetAccessibleName(pWidget) != NULL ?
			xuiWidgetGetAccessibleName(pWidget) : "Tabs";
		pNode->sValue = pData->iSelected >= 0 && pData->iSelected < pData->iPageCount ?
			pData->arrPages[pData->iSelected].sTitle : "";
		pNode->sDescription = xuiWidgetGetAccessibleDescription(pWidget);
		pNode->tBounds = tWorld;
		pNode->iState |= XUI_ACCESSIBLE_STATE_FOCUSABLE;
		if ( pData->iSelected >= 0 ) {
			pNode->iState |= XUI_ACCESSIBLE_STATE_SELECTION_REQUIRED;
		}
		if ( __xuiTabsVertical(pData->iPlacement) ) pNode->iState |= XUI_ACCESSIBLE_STATE_VERTICAL;
		if ( bWidgetOffscreen ) pNode->iState |= XUI_ACCESSIBLE_STATE_OFFSCREEN;
		if ( xuiGetFocusWidget(xuiWidgetGetContext(pWidget)) == pWidget ) {
			pNode->iState |= XUI_ACCESSIBLE_STATE_FOCUSED;
		}
		pNode->iRowCount = __xuiTabsVertical(pData->iPlacement) ? pData->iPageCount : 1;
		pNode->iColumnCount = __xuiTabsVertical(pData->iPlacement) ? 1 : pData->iPageCount;
		pNode->iActions = XUI_ACCESSIBLE_ACTION_MASK(XUI_ACCESSIBLE_ACTION_FOCUS);
		if ( !xuiWidgetGetEffectiveEnabled(pWidget) ) {
			pNode->iState |= XUI_ACCESSIBLE_STATE_DISABLED;
			pNode->iActions = 0;
		}
		if ( bWidgetOffscreen ) pNode->iActions = 0;
		return XUI_OK;
	}
	iTab = iIndex - 1;
	tTab = pData->arrPages[iTab].tTabRect;
	pNode->iId = (uint64_t)iTab + 2u;
	pNode->iParentId = 1;
	pNode->iRole = XUI_ACCESSIBLE_ROLE_TAB;
	pNode->sName = pData->arrPages[iTab].sTitle;
	pNode->tBounds = (xui_rect_t){tWorld.fX + tTab.fX, tWorld.fY + tTab.fY,
		tTab.fW, tTab.fH};
	pNode->iRow = __xuiTabsVertical(pData->iPlacement) ? iTab : 0;
	pNode->iColumn = __xuiTabsVertical(pData->iPlacement) ? 0 : iTab;
	pNode->iRowCount = 1;
	pNode->iColumnCount = 1;
	pNode->iState |= XUI_ACCESSIBLE_STATE_FOCUSABLE | XUI_ACCESSIBLE_STATE_SELECTABLE;
	if ( iTab == pData->iSelected ) {
		pNode->iState |= XUI_ACCESSIBLE_STATE_SELECTED;
		if ( xuiGetFocusWidget(xuiWidgetGetContext(pWidget)) == pWidget ) {
			pNode->iState |= XUI_ACCESSIBLE_STATE_FOCUSED;
		}
	}
	bDisabled = !xuiWidgetGetEffectiveEnabled(pWidget) || !__xuiTabsItemEnabled(pData, iTab);
	if ( bDisabled ) pNode->iState |= XUI_ACCESSIBLE_STATE_DISABLED;
	if ( bWidgetOffscreen || tTab.fW <= 0.0f || tTab.fH <= 0.0f ||
	     tTab.fX + tTab.fW <= pData->tTabBarRect.fX ||
	     tTab.fY + tTab.fH <= pData->tTabBarRect.fY ||
	     tTab.fX >= pData->tTabBarRect.fX + pData->tTabBarRect.fW ||
	     tTab.fY >= pData->tTabBarRect.fY + pData->tTabBarRect.fH ) {
		pNode->iState |= XUI_ACCESSIBLE_STATE_OFFSCREEN;
	}
	pNode->iActions = XUI_ACCESSIBLE_ACTION_MASK(XUI_ACCESSIBLE_ACTION_FOCUS) |
		XUI_ACCESSIBLE_ACTION_MASK(XUI_ACCESSIBLE_ACTION_SET_SELECTION) |
		XUI_ACCESSIBLE_ACTION_MASK(XUI_ACCESSIBLE_ACTION_ACTIVATE) |
		XUI_ACCESSIBLE_ACTION_MASK(XUI_ACCESSIBLE_ACTION_SCROLL_INTO_VIEW);
	if ( bDisabled ) pNode->iActions = 0;
	if ( bWidgetOffscreen ) pNode->iActions = 0;
	return XUI_OK;
}

static int __xuiTabsAccessibleAction(xui_widget pWidget, uint64_t iNodeId,
	int iAction, const void* pPayload, void* pUser)
{
	xui_tabs_data_t* pData = (xui_tabs_data_t*)pUser;
	int iTab;
	int iRet;

	(void)pPayload;
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( !xuiWidgetGetEffectiveEnabled(pWidget) || !xuiWidgetGetEffectiveVisible(pWidget) ) {
		return XUI_ERROR_INVALID_STATE;
	}
	if ( iNodeId == 0 || iNodeId == 1 ) {
		return iAction == XUI_ACCESSIBLE_ACTION_FOCUS ?
			xuiSetFocusWidget(xuiWidgetGetContext(pWidget), pWidget) : XUI_ERROR_UNSUPPORTED;
	}
	if ( iNodeId < 2 || iNodeId > (uint64_t)pData->iPageCount + 1u ) return XUI_ERROR_NOT_FOUND;
	iTab = (int)iNodeId - 2;
	if ( !__xuiTabsItemEnabled(pData, iTab) ) return XUI_ERROR_INVALID_STATE;
	if ( iAction == XUI_ACCESSIBLE_ACTION_SCROLL_INTO_VIEW ) {
		__xuiTabsEnsureVisibleInternal(pData, iTab);
		return __xuiTabsInvalidateAll(pWidget, pData,
			XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	}
	if ( iAction != XUI_ACCESSIBLE_ACTION_FOCUS &&
	     iAction != XUI_ACCESSIBLE_ACTION_SET_SELECTION &&
	     iAction != XUI_ACCESSIBLE_ACTION_ACTIVATE ) return XUI_ERROR_UNSUPPORTED;
	iRet = xuiSetFocusWidget(xuiWidgetGetContext(pWidget), pWidget);
	if ( iRet != XUI_OK || !__xuiTabsWidgetAlive(pWidget, pData) ) return iRet;
	return __xuiTabsSelectInternal(pWidget, pData, iTab,
		iAction != XUI_ACCESSIBLE_ACTION_FOCUS);
}

static void __xuiTabsDefaultLayout(xui_layout_t* pLayout)
{
	memset(pLayout, 0, sizeof(*pLayout));
	pLayout->iLayoutType = XUI_LAYOUT_TABLE;
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
	pLayout->fPreferredWidth = 420.0f;
	pLayout->fPreferredHeight = 260.0f;
	pLayout->fMaxWidth = XUI_LAYOUT_UNBOUNDED;
	pLayout->fMaxHeight = XUI_LAYOUT_UNBOUNDED;
	pLayout->fShrink = 1.0f;
}

static void __xuiTabsDefaultData(xui_tabs_data_t* pData)
{
	int i;

	memset(pData, 0, sizeof(*pData));
	pData->iPageCount = 0;
	pData->iSelected = -1;
	pData->iPlacement = XUI_TABS_PLACEMENT_TOP;
	pData->iHoverIndex = -1;
	pData->iActiveIndex = -1;
	pData->iCloseHoverIndex = -1;
	pData->iCloseActiveIndex = -1;
	pData->bScrollable = 0;
	pData->bCloseButtons = 0;
	pData->bOverflow = 0;
	pData->bOverflowHover = 0;
	pData->bOverflowActive = 0;
	pData->iFirstVisibleTab = 0;
	pData->iVisibleTabCount = 0;
	pData->fTabWidth = 104.0f;
	pData->fTabHeight = 30.0f;
	pData->fResolvedTabWidth = pData->fTabWidth;
	pData->fResolvedTabHeight = pData->fTabHeight;
	pData->iBackgroundColor = XUI_COLOR_RGBA(232, 243, 251, 255);
	pData->iTabColor = XUI_COLOR_RGBA(239, 247, 255, 255);
	pData->iHoverColor = XUI_COLOR_RGBA(220, 236, 251, 255);
	pData->iActiveColor = XUI_COLOR_RGBA(46, 124, 214, 255);
	pData->iFocusColor = XUI_COLOR_RGBA(77, 147, 226, 255);
	pData->iDisabledColor = XUI_COLOR_RGBA(229, 236, 244, 255);
	pData->iTextColor = XUI_COLOR_RGBA(42, 58, 78, 255);
	pData->iActiveTextColor = XUI_COLOR_RGBA(24, 54, 92, 255);
	pData->iBorderColor = XUI_COLOR_RGBA(164, 190, 219, 255);
	pData->iClientColor = XUI_COLOR_RGBA(248, 252, 255, 255);
	for ( i = 0; i < XUI_TABS_PAGE_CAPACITY; i++ ) {
		pData->arrPages[i].bEnabled = 1;
	}
}

static int __xuiTabsCreateChrome(xui_widget pWidget, xui_tabs_data_t* pData)
{
	int iRet;

	iRet = xuiWidgetCreate(xuiWidgetGetContext(pWidget), &pData->pTabBar);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiWidgetCreate(xuiWidgetGetContext(pWidget), &pData->pClient);
	if ( iRet != XUI_OK ) {
		xuiWidgetDestroy(pData->pTabBar);
		pData->pTabBar = NULL;
		return iRet;
	}
	iRet = xuiWidgetSetLayoutType(pData->pTabBar, XUI_LAYOUT_OVERLAY);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetSizeMode(pData->pTabBar, XUI_SIZE_FILL, XUI_SIZE_FILL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetLayoutChildrenCallback(pData->pTabBar, __xuiTabsLayoutButtons, pWidget);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetOverflow(pData->pTabBar, XUI_OVERFLOW_CLIP);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetFocusable(pData->pTabBar, 0);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetTabStop(pData->pTabBar, 0);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pData->pTabBar, XUI_EVENT_POINTER_ENTER, __xuiTabsTabBarEvent, pWidget);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pData->pTabBar, XUI_EVENT_POINTER_LEAVE, __xuiTabsTabBarEvent, pWidget);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pData->pTabBar, XUI_EVENT_POINTER_MOVE, __xuiTabsTabBarEvent, pWidget);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pData->pTabBar, XUI_EVENT_POINTER_DOWN, __xuiTabsTabBarEvent, pWidget);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pData->pTabBar, XUI_EVENT_POINTER_UP, __xuiTabsTabBarEvent, pWidget);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pData->pTabBar, XUI_EVENT_POINTER_CAPTURE_LOST, __xuiTabsTabBarEvent, pWidget);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pData->pTabBar, XUI_EVENT_POINTER_WHEEL, __xuiTabsTabBarEvent, pWidget);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetLayoutType(pData->pClient, XUI_LAYOUT_OVERLAY);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetSizeMode(pData->pClient, XUI_SIZE_FILL, XUI_SIZE_FILL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetPadding(pData->pClient, __xuiTabsThickness(8.0f, 8.0f, 8.0f, 8.0f));
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetOverflow(pData->pClient, XUI_OVERFLOW_CLIP);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetFocusable(pData->pClient, 0);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetTabStop(pData->pClient, 0);
	if ( iRet == XUI_OK ) iRet = xuiWidgetAddChild(pWidget, pData->pTabBar);
	if ( iRet == XUI_OK ) iRet = xuiWidgetAddChild(pWidget, pData->pClient);
	if ( iRet != XUI_OK ) {
		xuiWidgetDestroy(pData->pTabBar);
		xuiWidgetDestroy(pData->pClient);
		pData->pTabBar = NULL;
		pData->pClient = NULL;
		return iRet;
	}
	return XUI_OK;
}

static int __xuiTabsInit(xui_widget pWidget, void* pTypeData, const void* pCreateData, void* pUser)
{
	xui_tabs_data_t* pData;
	const xui_tabs_desc_t* pDesc;
	int iRet;

	(void)pUser;
	pData = (xui_tabs_data_t*)pTypeData;
	pDesc = (const xui_tabs_desc_t*)pCreateData;
	if ( (pWidget == NULL) || (pData == NULL) || !__xuiTabsDescValid(pDesc) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	__xuiTabsDefaultData(pData);
	if ( pDesc != NULL ) {
		if ( __xuiTabsPlacementValid(pDesc->iPlacement) ) pData->iPlacement = pDesc->iPlacement;
		if ( pDesc->fTabWidth > 0.0f ) pData->fTabWidth = pDesc->fTabWidth;
		if ( pDesc->fTabHeight > 0.0f ) pData->fTabHeight = pDesc->fTabHeight;
		pData->fResolvedTabWidth = pData->fTabWidth;
		pData->fResolvedTabHeight = pData->fTabHeight;
		pData->bScrollable = pDesc->bScrollable ? 1 : 0;
		pData->bCloseButtons = pDesc->bCloseButtons ? 1 : 0;
		pData->pFont = (pDesc->pFont != NULL) ? pDesc->pFont : xuiGetDefaultFont(xuiWidgetGetContext(pWidget));
		if ( pDesc->iSelected >= 0 ) pData->iSelected = pDesc->iSelected;
		if ( __xuiTabsAlpha(pDesc->iBackgroundColor) != 0 ) pData->iBackgroundColor = pDesc->iBackgroundColor;
		if ( __xuiTabsAlpha(pDesc->iTabColor) != 0 ) pData->iTabColor = pDesc->iTabColor;
		if ( __xuiTabsAlpha(pDesc->iHoverColor) != 0 ) pData->iHoverColor = pDesc->iHoverColor;
		if ( __xuiTabsAlpha(pDesc->iActiveColor) != 0 ) pData->iActiveColor = pDesc->iActiveColor;
		if ( __xuiTabsAlpha(pDesc->iFocusColor) != 0 ) pData->iFocusColor = pDesc->iFocusColor;
		if ( __xuiTabsAlpha(pDesc->iDisabledColor) != 0 ) pData->iDisabledColor = pDesc->iDisabledColor;
		if ( __xuiTabsAlpha(pDesc->iTextColor) != 0 ) pData->iTextColor = pDesc->iTextColor;
		if ( __xuiTabsAlpha(pDesc->iActiveTextColor) != 0 ) pData->iActiveTextColor = pDesc->iActiveTextColor;
		if ( __xuiTabsAlpha(pDesc->iBorderColor) != 0 ) pData->iBorderColor = pDesc->iBorderColor;
		if ( __xuiTabsAlpha(pDesc->iClientColor) != 0 ) pData->iClientColor = pDesc->iClientColor;
	}
	iRet = xuiWidgetSetLayoutType(pWidget, XUI_LAYOUT_TABLE);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetOverflow(pWidget, XUI_OVERFLOW_CLIP);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetFocusable(pWidget, 1);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetTabStop(pWidget, 1);
	if ( iRet == XUI_OK ) iRet = __xuiTabsCreateChrome(pWidget, pData);
	if ( iRet != XUI_OK ) return iRet;
	if ( pDesc != NULL && pDesc->iItemCount > 0 ) {
		iRet = __xuiTabsApplyItems(pWidget, pData, pDesc->arrItems, pDesc->iItemCount);
		if ( iRet != XUI_OK ) return iRet;
		iRet = xuiTabsSetEnabledItems(pWidget, pDesc->arrEnabled, pDesc->iItemCount);
		if ( iRet == XUI_OK ) iRet = xuiTabsSetDirtyItems(pWidget, pDesc->arrDirty, pDesc->iItemCount);
		if ( iRet == XUI_OK ) iRet = xuiTabsSetIcons(pWidget, pDesc->arrIcons, pDesc->arrIconSrc, pDesc->iItemCount);
		if ( iRet != XUI_OK ) return iRet;
	}
	iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_KEY_DOWN, __xuiTabsEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_CONTEXT_MENU, __xuiTabsEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_FOCUS, __xuiTabsEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_BLUR, __xuiTabsEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_ENABLED_CHANGED, __xuiTabsEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_VISIBLE_CHANGED, __xuiTabsEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetAccessibilityProvider(pWidget,
		__xuiTabsAccessibleCount, __xuiTabsAccessibleGet,
		__xuiTabsAccessibleAction, pData);
	if ( iRet != XUI_OK ) return iRet;
	return __xuiTabsSyncPages(pWidget, pData);
}

static void __xuiTabsDestroy(xui_widget pWidget, void* pTypeData, void* pUser)
{
	xui_tabs_data_t* pData;

	(void)pUser;
	pData = (xui_tabs_data_t*)pTypeData;
	if ( pData != NULL ) {
		(void)__xuiTabsClearAllInteraction(pWidget, pData);
		xuiWidgetDestroy(pData->pOverflowMenu);
		memset(pData, 0, sizeof(*pData));
	}
}

static void __xuiTabsRegisterStyleProperty(xui_context pContext, xui_widget_type pType, const char* sName, int iValueType, uint32_t iDirtyFlags, uint32_t iFlags)
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

static int __xuiTabsPreparePaint(xui_widget pWidget)
{
	xui_tabs_data_t* pData = __xuiTabsGetData(pWidget);
	xui_proxy pProxy;
	int iRet;
	int i;
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	pProxy = xuiInternalContextGetProxy(xuiWidgetGetContext(pWidget));
	for ( i = 0; i < pData->iPageCount; i++ ) {
		if ( xuiInternalSurfaceDependencySync(pProxy, pData->arrPages[i].pIcon,
		     &pData->arrPages[i].tIconDependency, 0) ) {
			iRet = xuiWidgetInvalidate(pData->arrPages[i].pButton,
				XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
			if ( iRet != XUI_OK ) return iRet;
		}
	}
	if ( pData->iChromeStyleVersion == pWidget->iStyleVersion ) return XUI_OK;
	pData->iChromeStyleVersion = pWidget->iStyleVersion;
	return __xuiTabsInvalidateAll(pWidget, pData, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
}

static void __xuiTabsRegisterStyleProperties(xui_context pContext, xui_widget_type pType)
{
	uint32_t iPaintDirty;
	uint32_t iLayoutDirty;

	pType->onPreparePaint = __xuiTabsPreparePaint;
	iPaintDirty = XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER;
	iLayoutDirty = XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER;
	__xuiTabsRegisterStyleProperty(pContext, pType, "tabs.background.color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiTabsRegisterStyleProperty(pContext, pType, "tabs.tab.color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiTabsRegisterStyleProperty(pContext, pType, "tabs.tab.hover_color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiTabsRegisterStyleProperty(pContext, pType, "tabs.tab.active_color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiTabsRegisterStyleProperty(pContext, pType, "tabs.tab.focus_color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiTabsRegisterStyleProperty(pContext, pType, "tabs.tab.disabled_color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiTabsRegisterStyleProperty(pContext, pType, "tabs.text.color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiTabsRegisterStyleProperty(pContext, pType, "tabs.text.active_color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiTabsRegisterStyleProperty(pContext, pType, "tabs.text.disabled_color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiTabsRegisterStyleProperty(pContext, pType, "tabs.close.color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiTabsRegisterStyleProperty(pContext, pType, "tabs.close.disabled_color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiTabsRegisterStyleProperty(pContext, pType, "tabs.close.hover_color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiTabsRegisterStyleProperty(pContext, pType, "tabs.close.active_color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiTabsRegisterStyleProperty(pContext, pType, "tabs.close.hover_background_color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiTabsRegisterStyleProperty(pContext, pType, "tabs.close.active_background_color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiTabsRegisterStyleProperty(pContext, pType, "tabs.border.color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiTabsRegisterStyleProperty(pContext, pType, "tabs.client.color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiTabsRegisterStyleProperty(pContext, pType, "tabs.tab.width", XUI_STYLE_VALUE_FLOAT, iLayoutDirty, 0);
	__xuiTabsRegisterStyleProperty(pContext, pType, "tabs.tab.height", XUI_STYLE_VALUE_FLOAT, iLayoutDirty, 0);
}

XUI_API xui_widget_type xuiTabsGetType(xui_context pContext)
{
	xui_widget_type_desc_t tDesc;
	xui_widget_type pType;
	xui_cache_policy_t tPolicy;
	int iRet;

	if ( !xuiInternalContextIsValid(pContext) ) {
		return NULL;
	}
	pType = xuiWidgetFindType(pContext, "tabs");
	if ( pType != NULL ) {
		__xuiTabsRegisterStyleProperties(pContext, pType);
		return pType;
	}
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.sName = "tabs";
	tDesc.pParent = xuiWidgetGetBaseType();
	tDesc.iFlags = XUI_WIDGET_TYPE_DEFAULT_LAYOUT | XUI_WIDGET_TYPE_DEFAULT_CACHE_POLICY;
	tDesc.iTypeDataSize = sizeof(xui_tabs_data_t);
	tDesc.onInit = __xuiTabsInit;
	tDesc.onDestroy = __xuiTabsDestroy;
	tDesc.onContentMeasure = __xuiTabsContentMeasure;
	tDesc.onLayoutPrepare = __xuiTabsPrepare;
	tDesc.onLayoutComplete = __xuiTabsLayoutComplete;
	tDesc.onCacheRender = __xuiTabsCacheRender;
	__xuiTabsDefaultLayout(&tDesc.tLayout);
	__xuiTabsDefaultCachePolicy(&tPolicy);
	tDesc.tCachePolicy = tPolicy;
	iRet = xuiWidgetRegisterType(pContext, &pType, &tDesc);
	if ( iRet != XUI_OK ) {
		return NULL;
	}
	__xuiTabsRegisterStyleProperties(pContext, pType);
	return pType;
}

XUI_API int xuiTabsCreate(xui_context pContext, xui_widget* ppWidget, const xui_tabs_desc_t* pDesc)
{
	xui_widget_type pType;
	xui_tabs_data_t* pData;
	int iRet;

	if ( ppWidget == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	*ppWidget = NULL;
	if ( !__xuiTabsDescValid(pDesc) ) return XUI_ERROR_INVALID_ARGUMENT;
	pType = xuiTabsGetType(pContext);
	if ( pType == NULL ) {
		return XUI_ERROR_NOT_INITIALIZED;
	}
	iRet = xuiWidgetCreateTyped(pContext, pType, ppWidget, pDesc);
	if ( iRet != XUI_OK ) return iRet;
	pData = __xuiTabsGetData(*ppWidget);
	iRet = __xuiTabsEnsureOverflowMenu(*ppWidget, pData);
	if ( iRet != XUI_OK ) {
		xuiWidgetDestroy(*ppWidget);
		*ppWidget = NULL;
		return iRet;
	}
	return XUI_OK;
}

XUI_API int xuiTabsSetSelect(xui_widget pWidget, xui_tabs_select_proc onSelect, void* pUser)
{
	xui_tabs_data_t* pData;

	pData = __xuiTabsGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	pData->onSelect = onSelect;
	pData->pSelectUser = pUser;
	return XUI_OK;
}

XUI_API int xuiTabsSetClose(xui_widget pWidget, xui_tabs_close_proc onClose, int bCloseButtons, void* pUser)
{
	xui_tabs_data_t* pData;

	pData = __xuiTabsGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	pData->onClose = onClose;
	pData->bCloseButtons = bCloseButtons ? 1 : 0;
	pData->pCloseUser = pUser;
	if ( !pData->bCloseButtons || onClose == NULL ) {
		pData->iCloseHoverIndex = -1;
		int iRet = __xuiTabsClearInteraction(pWidget, pData);
		if ( iRet != XUI_OK ) return iRet;
	}
	return __xuiTabsInvalidateAll(pWidget, pData, XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
}

XUI_API int xuiTabsAddPage(xui_widget pWidget, const char* sTitle, xui_widget* ppPage)
{
	xui_tabs_data_t* pData;
	xui_tabs_page_t tOldPage;
	xui_widget pNewButton;
	xui_widget pNewPage;
	int iIndex;
	int iOldSelected;
	int iOldChangeCount;
	int bCreated;
	int iRet;

	if ( ppPage != NULL ) *ppPage = NULL;
	pData = __xuiTabsGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pData->iPageCount >= XUI_TABS_PAGE_CAPACITY ) return XUI_ERROR_OUT_OF_MEMORY;
	iIndex = pData->iPageCount;
	tOldPage = pData->arrPages[iIndex];
	iOldSelected = pData->iSelected;
	iOldChangeCount = pData->iChangeCount;
	bCreated = tOldPage.pButton == NULL;
	iRet = __xuiTabsCreatePage(pWidget, pData, iIndex);
	if ( iRet != XUI_OK ) return iRet;
	__xuiTabsSetTitle(&pData->arrPages[iIndex], sTitle);
	pData->arrPages[iIndex].bEnabled = 1;
	pData->arrPages[iIndex].bDirty = 0;
	pData->iPageCount++;
	if ( pData->iSelected < 0 ) pData->iSelected = iIndex;
	if ( iOldSelected != pData->iSelected ) pData->iChangeCount++;
	iRet = __xuiTabsSyncPages(pWidget, pData);
	if ( iRet != XUI_OK ) {
		pNewButton = pData->arrPages[iIndex].pButton;
		pNewPage = pData->arrPages[iIndex].pPage;
		pData->arrPages[iIndex] = tOldPage;
		pData->iPageCount = iIndex;
		pData->iSelected = iOldSelected;
		pData->iChangeCount = iOldChangeCount;
		if ( bCreated ) {
			xuiWidgetDestroy(pNewButton);
			xuiWidgetDestroy(pNewPage);
		}
		/* Preserve the original failure; restoring widget state is best-effort. */
		(void)__xuiTabsSyncPages(pWidget, pData);
		return iRet;
	}
	if ( ppPage != NULL ) *ppPage = pData->arrPages[iIndex].pPage;
	xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_TREE_CHANGED);
	if ( iOldSelected != pData->iSelected ) {
		xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_SELECTION_CHANGED);
	}
	return XUI_OK;
}

XUI_API int xuiTabsSetItemText(xui_widget pWidget, int iIndex, const char* sTitle)
{
	xui_tabs_data_t* pData;
	xui_context pContext;
	char sOldTitle[XUI_TABS_TITLE_CAPACITY];
	char sNewTitle[XUI_TABS_TITLE_CAPACITY];
	int bMenuOpen;
	int iRet;

	pData = __xuiTabsGetData(pWidget);
	if ( pData == NULL || iIndex < 0 || iIndex >= pData->iPageCount ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	__xuiTabsCopyTitle(sNewTitle, sTitle);
	if ( strcmp(pData->arrPages[iIndex].sTitle, sNewTitle) == 0 ) return XUI_OK;
	pContext = xuiWidgetGetContext(pWidget);
	memcpy(sOldTitle, pData->arrPages[iIndex].sTitle, sizeof(sOldTitle));
	bMenuOpen = pData->pOverflowMenu != NULL && xuiMenuIsOpen(pData->pOverflowMenu);
	xuiInternalOperationEnter(pContext);
	__xuiTabsSetTitle(&pData->arrPages[iIndex], sNewTitle);
	iRet = bMenuOpen ? __xuiTabsBuildOverflowMenu(pWidget, pData) : XUI_OK;
	if ( iRet == XUI_OK ) {
		iRet = xuiWidgetInvalidate(pData->arrPages[iIndex].pButton,
			XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	}
	if ( iRet != XUI_OK ) {
		__xuiTabsSetTitle(&pData->arrPages[iIndex], sOldTitle);
		if ( bMenuOpen ) (void)__xuiTabsBuildOverflowMenu(pWidget, pData);
	} else {
		xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_TREE_CHANGED);
		if ( iIndex == pData->iSelected ) {
			xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_VALUE_CHANGED);
		}
	}
	xuiInternalOperationLeave(pContext);
	return iRet;
}

XUI_API int xuiTabsRemovePage(xui_widget pWidget, int iIndex)
{
	xui_tabs_data_t* pData;
	xui_tabs_page_t arrOldPages[XUI_TABS_PAGE_CAPACITY];
	xui_tabs_page_t tRemoved;
	xui_context pContext;
	float fOldScroll;
	int iOldPageCount;
	int iOldSelected;
	int iOldChangeCount;
	int bSelectionChanged;
	int iCandidate;
	int iRet;
	int i;

	pData = __xuiTabsGetData(pWidget);
	if ( pData == NULL || iIndex < 0 || iIndex >= pData->iPageCount ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pContext = xuiWidgetGetContext(pWidget);
	xuiInternalOperationEnter(pContext);
	iRet = __xuiTabsClearAllInteraction(pWidget, pData);
	if ( iRet != XUI_OK || !__xuiTabsWidgetAlive(pWidget, pData) ) goto done;
	if ( pData->pOverflowMenu != NULL ) {
		iRet = xuiMenuClose(pData->pOverflowMenu);
		if ( iRet == XUI_OK ) iRet = xuiMenuClear(pData->pOverflowMenu);
		if ( iRet != XUI_OK || !__xuiTabsWidgetAlive(pWidget, pData) ) goto done;
	}

	memcpy(arrOldPages, pData->arrPages, sizeof(arrOldPages));
	tRemoved = pData->arrPages[iIndex];
	iOldPageCount = pData->iPageCount;
	iOldSelected = pData->iSelected;
	iOldChangeCount = pData->iChangeCount;
	fOldScroll = pData->fScrollX;
	for ( i = iIndex; i < XUI_TABS_PAGE_CAPACITY - 1; i++ ) {
		pData->arrPages[i] = pData->arrPages[i + 1];
	}
	/* Keep the removed widgets discoverable until their detach callbacks finish. */
	pData->arrPages[XUI_TABS_PAGE_CAPACITY - 1] = tRemoved;
	pData->iPageCount--;
	bSelectionChanged = 0;
	if ( iOldSelected == iIndex ) {
		pData->iSelected = -1;
		if ( pData->iPageCount > 0 ) {
			iCandidate = iIndex < pData->iPageCount ? iIndex : pData->iPageCount - 1;
			pData->iSelected = __xuiTabsItemEnabled(pData, iCandidate) ?
				iCandidate : __xuiTabsStepEnabled(pData, iCandidate, 1);
		}
		bSelectionChanged = 1;
	} else if ( iOldSelected > iIndex ) {
		pData->iSelected = iOldSelected - 1;
		bSelectionChanged = 1;
	}
	if ( bSelectionChanged ) pData->iChangeCount++;
	__xuiTabsUpdateMaxScroll(pData);
	if ( pData->iSelected >= 0 ) __xuiTabsEnsureVisibleInternal(pData, pData->iSelected);
	iRet = __xuiTabsSyncPages(pWidget, pData);
	if ( iRet != XUI_OK ) {
		memcpy(pData->arrPages, arrOldPages, sizeof(arrOldPages));
		pData->iPageCount = iOldPageCount;
		pData->iSelected = iOldSelected;
		pData->iChangeCount = iOldChangeCount;
		pData->fScrollX = fOldScroll;
		(void)__xuiTabsSyncPages(pWidget, pData);
		goto done;
	}
	xuiWidgetDestroy(tRemoved.pButton);
	xuiWidgetDestroy(tRemoved.pPage);
	memset(&pData->arrPages[XUI_TABS_PAGE_CAPACITY - 1], 0,
		sizeof(pData->arrPages[XUI_TABS_PAGE_CAPACITY - 1]));
	pData->arrPages[XUI_TABS_PAGE_CAPACITY - 1].bEnabled = 1;
	xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_TREE_CHANGED);
	if ( bSelectionChanged ) {
		xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_SELECTION_CHANGED);
	}

done:
	xuiInternalOperationLeave(pContext);
	return iRet;
}

XUI_API int xuiTabsSetItems(xui_widget pWidget, const char** arrItems, int iItemCount)
{
	xui_tabs_data_t* pData;

	pData = __xuiTabsGetData(pWidget);
	return __xuiTabsApplyItems(pWidget, pData, arrItems, iItemCount);
}

XUI_API int xuiTabsGetItemCount(xui_widget pWidget)
{
	xui_tabs_data_t* pData;

	pData = __xuiTabsGetData(pWidget);
	return (pData != NULL) ? pData->iPageCount : 0;
}

XUI_API const char* xuiTabsGetItemText(xui_widget pWidget, int iIndex)
{
	xui_tabs_data_t* pData;

	pData = __xuiTabsGetData(pWidget);
	if ( (pData == NULL) || (iIndex < 0) || (iIndex >= pData->iPageCount) ) return NULL;
	return pData->arrPages[iIndex].sTitle;
}

XUI_API xui_widget xuiTabsGetTabBarWidget(xui_widget pWidget)
{
	xui_tabs_data_t* pData;

	pData = __xuiTabsGetData(pWidget);
	return (pData != NULL) ? pData->pTabBar : NULL;
}

XUI_API xui_widget xuiTabsGetClientWidget(xui_widget pWidget)
{
	xui_tabs_data_t* pData;

	pData = __xuiTabsGetData(pWidget);
	return (pData != NULL) ? pData->pClient : NULL;
}

XUI_API xui_widget xuiTabsGetPageWidget(xui_widget pWidget, int iIndex)
{
	xui_tabs_data_t* pData;

	pData = __xuiTabsGetData(pWidget);
	if ( (pData == NULL) || (iIndex < 0) || (iIndex >= pData->iPageCount) ) return NULL;
	return pData->arrPages[iIndex].pPage;
}

XUI_API xui_widget xuiTabsGetButtonWidget(xui_widget pWidget, int iIndex)
{
	xui_tabs_data_t* pData;

	pData = __xuiTabsGetData(pWidget);
	if ( (pData == NULL) || (iIndex < 0) || (iIndex >= pData->iPageCount) ) return NULL;
	return pData->arrPages[iIndex].pButton;
}

XUI_API int xuiTabsAddPageChild(xui_widget pWidget, int iIndex, xui_widget pChild)
{
	xui_widget pPage;

	if ( pChild == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	pPage = xuiTabsGetPageWidget(pWidget, iIndex);
	if ( pPage == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	return xuiWidgetAddChild(pPage, pChild);
}

XUI_API int xuiTabsSetEnabledItems(xui_widget pWidget, const int* arrEnabled, int iItemCount)
{
	xui_tabs_data_t* pData;
	int arrOldEnabled[XUI_TABS_PAGE_CAPACITY];
	int iOldSelected;
	int iOldChangeCount;
	int bClearInteraction;
	int i;
	int iRet;

	pData = __xuiTabsGetData(pWidget);
	if ( (pData == NULL) || (iItemCount < 0) ) return XUI_ERROR_INVALID_ARGUMENT;
	bClearInteraction = 0;
	if ( pData->iActiveIndex >= 0 && pData->iActiveIndex < pData->iPageCount &&
	     arrEnabled != NULL && pData->iActiveIndex < iItemCount &&
	     arrEnabled[pData->iActiveIndex] == 0 ) bClearInteraction = 1;
	if ( pData->iCloseActiveIndex >= 0 && pData->iCloseActiveIndex < pData->iPageCount &&
	     arrEnabled != NULL && pData->iCloseActiveIndex < iItemCount &&
	     arrEnabled[pData->iCloseActiveIndex] == 0 ) bClearInteraction = 1;
	if ( pData->iHoverIndex >= 0 && pData->iHoverIndex < pData->iPageCount &&
	     arrEnabled != NULL && pData->iHoverIndex < iItemCount &&
	     arrEnabled[pData->iHoverIndex] == 0 ) bClearInteraction = 1;
	if ( pData->iCloseHoverIndex >= 0 && pData->iCloseHoverIndex < pData->iPageCount &&
	     arrEnabled != NULL && pData->iCloseHoverIndex < iItemCount &&
	     arrEnabled[pData->iCloseHoverIndex] == 0 ) bClearInteraction = 1;
	if ( bClearInteraction ) {
		iRet = __xuiTabsClearAllInteraction(pWidget, pData);
		if ( iRet != XUI_OK ) return iRet;
	}
	iOldSelected = pData->iSelected;
	iOldChangeCount = pData->iChangeCount;
	for ( i = 0; i < pData->iPageCount; i++ ) {
		arrOldEnabled[i] = pData->arrPages[i].bEnabled;
		pData->arrPages[i].bEnabled = (arrEnabled == NULL || i >= iItemCount) ? 1 : (arrEnabled[i] != 0);
	}
	if ( (pData->iSelected >= 0) && !__xuiTabsItemEnabled(pData, pData->iSelected) ) {
		pData->iSelected = __xuiTabsStepEnabled(pData, pData->iSelected, 1);
		if ( pData->iSelected < 0 ) pData->iSelected = __xuiTabsFirstEnabled(pData);
	}
	if ( pData->iSelected < 0 ) {
		pData->iSelected = __xuiTabsFirstEnabled(pData);
	}
	if ( iOldSelected != pData->iSelected ) pData->iChangeCount++;
	iRet = __xuiTabsSyncPages(pWidget, pData);
	if ( iRet != XUI_OK ) {
		for ( i = 0; i < pData->iPageCount; i++ ) pData->arrPages[i].bEnabled = arrOldEnabled[i];
		pData->iSelected = iOldSelected;
		pData->iChangeCount = iOldChangeCount;
		/* Preserve the original failure; restoring widget state is best-effort. */
		(void)__xuiTabsSyncPages(pWidget, pData);
		return iRet;
	}
	xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_STATE_CHANGED);
	if ( iOldSelected != pData->iSelected ) {
		xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_SELECTION_CHANGED);
	}
	return XUI_OK;
}

XUI_API int xuiTabsSetDirtyItems(xui_widget pWidget, const int* arrDirty, int iItemCount)
{
	xui_tabs_data_t* pData;
	int i;

	pData = __xuiTabsGetData(pWidget);
	if ( (pData == NULL) || (iItemCount < 0) ) return XUI_ERROR_INVALID_ARGUMENT;
	for ( i = 0; i < pData->iPageCount; i++ ) {
		pData->arrPages[i].bDirty = (arrDirty != NULL && i < iItemCount && arrDirty[i] != 0);
	}
	return __xuiTabsInvalidateAll(pWidget, pData, XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
}

XUI_API int xuiTabsSetIcons(xui_widget pWidget, xui_surface* arrIcons, const xui_rect_t* arrSrc, int iItemCount)
{
	xui_tabs_data_t* pData;
	int i;

	pData = __xuiTabsGetData(pWidget);
	if ( (pData == NULL) || (iItemCount < 0) ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( arrSrc != NULL ) {
		for ( i = 0; i < pData->iPageCount && i < iItemCount; i++ ) {
			if ( !__xuiTabsIconRectValid(arrSrc[i]) ) return XUI_ERROR_INVALID_ARGUMENT;
		}
	}
	for ( i = 0; i < pData->iPageCount; i++ ) {
		pData->arrPages[i].pIcon = (arrIcons != NULL && i < iItemCount) ? arrIcons[i] : NULL;
		pData->arrPages[i].tIconSrc = (arrSrc != NULL && i < iItemCount) ? arrSrc[i] : (xui_rect_t){0.0f, 0.0f, 0.0f, 0.0f};
	}
	return __xuiTabsInvalidateAll(pWidget, pData, XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
}

XUI_API int xuiTabsSetFont(xui_widget pWidget, xui_font pFont)
{
	xui_tabs_data_t* pData;
	xui_font pNewFont;
	int iRet;

	pData = __xuiTabsGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	pNewFont = (pFont != NULL) ? pFont : xuiGetDefaultFont(xuiWidgetGetContext(pWidget));
	if ( pData->pFont == pNewFont ) return XUI_OK;
	iRet = (pData->pOverflowMenu != NULL) ? xuiMenuSetFont(pData->pOverflowMenu, pNewFont) : XUI_OK;
	if ( iRet != XUI_OK ) return iRet;
	pData->pFont = pNewFont;
	return __xuiTabsInvalidateAll(pWidget, pData, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
}

XUI_API xui_font xuiTabsGetFont(xui_widget pWidget)
{
	xui_tabs_data_t* pData;

	pData = __xuiTabsGetData(pWidget);
	return (pData != NULL) ? pData->pFont : NULL;
}

XUI_API int xuiTabsSetSelected(xui_widget pWidget, int iIndex)
{
	xui_tabs_data_t* pData;

	pData = __xuiTabsGetData(pWidget);
	return __xuiTabsSelectInternal(pWidget, pData, iIndex, 0);
}

XUI_API int xuiTabsGetSelected(xui_widget pWidget)
{
	xui_tabs_data_t* pData;

	pData = __xuiTabsGetData(pWidget);
	return (pData != NULL) ? pData->iSelected : -1;
}

XUI_API int xuiTabsSetTabSize(xui_widget pWidget, float fWidth, float fHeight)
{
	xui_tabs_data_t* pData;

	pData = __xuiTabsGetData(pWidget);
	if ( (pData == NULL) || !isfinite(fWidth) || !isfinite(fHeight) ||
	     (fWidth < 24.0f) || (fHeight < 18.0f) ||
	     (fWidth > XUI_LAYOUT_UNBOUNDED) || (fHeight > XUI_LAYOUT_UNBOUNDED) ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pData->fTabWidth == fWidth && pData->fTabHeight == fHeight ) return XUI_OK;
	pData->fTabWidth = fWidth;
	pData->fTabHeight = fHeight;
	return __xuiTabsInvalidateAll(pWidget, pData, XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
}

XUI_API int xuiTabsGetTabSize(xui_widget pWidget, float* pWidth, float* pHeight)
{
	xui_tabs_data_t* pData;

	pData = __xuiTabsGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pWidth != NULL ) *pWidth = pData->fTabWidth;
	if ( pHeight != NULL ) *pHeight = pData->fTabHeight;
	return XUI_OK;
}

XUI_API int xuiTabsSetPlacement(xui_widget pWidget, int iPlacement)
{
	xui_tabs_data_t* pData;

	pData = __xuiTabsGetData(pWidget);
	if ( (pData == NULL) || !__xuiTabsPlacementValid(iPlacement) ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pData->iPlacement == iPlacement ) return XUI_OK;
	pData->iPlacement = iPlacement;
	pData->fScrollX = 0.0f;
	return __xuiTabsInvalidateAll(pWidget, pData, XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
}

XUI_API int xuiTabsGetPlacement(xui_widget pWidget)
{
	xui_tabs_data_t* pData;

	pData = __xuiTabsGetData(pWidget);
	return (pData != NULL) ? pData->iPlacement : XUI_TABS_PLACEMENT_TOP;
}

XUI_API int xuiTabsSetScrollable(xui_widget pWidget, int bScrollable)
{
	xui_tabs_data_t* pData;

	pData = __xuiTabsGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	pData->bScrollable = bScrollable ? 1 : 0;
	if ( !pData->bScrollable ) pData->fScrollX = 0.0f;
	return __xuiTabsInvalidateAll(pWidget, pData, XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
}

XUI_API int xuiTabsIsScrollable(xui_widget pWidget)
{
	xui_tabs_data_t* pData;

	pData = __xuiTabsGetData(pWidget);
	return (pData != NULL) ? pData->bScrollable : 0;
}

XUI_API int xuiTabsSetScroll(xui_widget pWidget, float fScrollX)
{
	xui_tabs_data_t* pData;

	pData = __xuiTabsGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( !isfinite(fScrollX) || fScrollX > XUI_LAYOUT_UNBOUNDED ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( fScrollX < 0.0f ) fScrollX = 0.0f;
	__xuiTabsUpdateMaxScroll(pData);
	fScrollX = __xuiTabsClampFloat(fScrollX, 0.0f, pData->fMaxScroll);
	if ( pData->fScrollX == fScrollX ) return XUI_OK;
	pData->fScrollX = fScrollX;
	return __xuiTabsInvalidateAll(pWidget, pData, XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
}

XUI_API float xuiTabsGetScroll(xui_widget pWidget)
{
	xui_tabs_data_t* pData;

	pData = __xuiTabsGetData(pWidget);
	return (pData != NULL) ? pData->fScrollX : 0.0f;
}

XUI_API float xuiTabsGetMaxScroll(xui_widget pWidget)
{
	xui_tabs_data_t* pData;

	pData = __xuiTabsGetData(pWidget);
	if ( pData == NULL ) return 0.0f;
	__xuiTabsUpdateMaxScroll(pData);
	return pData->fMaxScroll;
}

XUI_API int xuiTabsEnsureVisible(xui_widget pWidget, int iIndex)
{
	xui_tabs_data_t* pData;

	pData = __xuiTabsGetData(pWidget);
	if ( (pData == NULL) || (iIndex < 0) || (iIndex >= pData->iPageCount) ) return XUI_ERROR_INVALID_ARGUMENT;
	__xuiTabsEnsureVisibleInternal(pData, iIndex);
	return __xuiTabsInvalidateAll(pWidget, pData, XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
}

XUI_API int xuiTabsSetColors(xui_widget pWidget, uint32_t iBackground, uint32_t iTab, uint32_t iHover, uint32_t iActive, uint32_t iFocus, uint32_t iDisabled, uint32_t iText, uint32_t iActiveText)
{
	xui_tabs_data_t* pData;

	pData = __xuiTabsGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	pData->iBackgroundColor = iBackground;
	pData->iTabColor = iTab;
	pData->iHoverColor = iHover;
	pData->iActiveColor = iActive;
	pData->iFocusColor = iFocus;
	pData->iDisabledColor = iDisabled;
	pData->iTextColor = iText;
	pData->iActiveTextColor = iActiveText;
	return __xuiTabsInvalidateAll(pWidget, pData, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
}

XUI_API int xuiTabsGetColors(xui_widget pWidget, uint32_t* pBackground, uint32_t* pTab, uint32_t* pHover, uint32_t* pActive, uint32_t* pFocus, uint32_t* pDisabled, uint32_t* pText, uint32_t* pActiveText)
{
	xui_tabs_data_t* pData;

	pData = __xuiTabsGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pBackground != NULL ) *pBackground = pData->iBackgroundColor;
	if ( pTab != NULL ) *pTab = pData->iTabColor;
	if ( pHover != NULL ) *pHover = pData->iHoverColor;
	if ( pActive != NULL ) *pActive = pData->iActiveColor;
	if ( pFocus != NULL ) *pFocus = pData->iFocusColor;
	if ( pDisabled != NULL ) *pDisabled = pData->iDisabledColor;
	if ( pText != NULL ) *pText = pData->iTextColor;
	if ( pActiveText != NULL ) *pActiveText = pData->iActiveTextColor;
	return XUI_OK;
}

XUI_API int xuiTabsSetContextMenu(xui_widget pWidget, xui_tabs_context_proc onContext, void* pUser)
{
	xui_tabs_data_t* pData = __xuiTabsGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	pData->onContext = onContext;
	pData->pContextUser = pUser;
	return XUI_OK;
}

XUI_API int xuiTabsSetFrameColors(xui_widget pWidget, uint32_t iBorder, uint32_t iClient)
{
	xui_tabs_data_t* pData;

	pData = __xuiTabsGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	pData->iBorderColor = iBorder;
	pData->iClientColor = iClient;
	return __xuiTabsInvalidateAll(pWidget, pData, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
}

XUI_API int xuiTabsGetFrameColors(xui_widget pWidget, uint32_t* pBorder, uint32_t* pClient)
{
	xui_tabs_data_t* pData;

	pData = __xuiTabsGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pBorder != NULL ) *pBorder = pData->iBorderColor;
	if ( pClient != NULL ) *pClient = pData->iClientColor;
	return XUI_OK;
}

XUI_API xui_rect_t xuiTabsGetTabBarRect(xui_widget pWidget)
{
	xui_tabs_data_t* pData;

	pData = __xuiTabsGetData(pWidget);
	return (pData != NULL) ? pData->tTabBarRect : (xui_rect_t){0.0f, 0.0f, 0.0f, 0.0f};
}

XUI_API xui_rect_t xuiTabsGetClientRect(xui_widget pWidget)
{
	xui_tabs_data_t* pData;

	pData = __xuiTabsGetData(pWidget);
	return (pData != NULL) ? pData->tClientRect : (xui_rect_t){0.0f, 0.0f, 0.0f, 0.0f};
}

XUI_API xui_rect_t xuiTabsGetTabRect(xui_widget pWidget, int iIndex)
{
	xui_tabs_data_t* pData;

	pData = __xuiTabsGetData(pWidget);
	if ( (pData == NULL) || (iIndex < 0) || (iIndex >= pData->iPageCount) ) return (xui_rect_t){0.0f, 0.0f, 0.0f, 0.0f};
	return pData->arrPages[iIndex].tTabRect;
}

XUI_API xui_rect_t xuiTabsGetTextRect(xui_widget pWidget, int iIndex)
{
	xui_tabs_data_t* pData;

	pData = __xuiTabsGetData(pWidget);
	if ( (pData == NULL) || (iIndex < 0) || (iIndex >= pData->iPageCount) ) return (xui_rect_t){0.0f, 0.0f, 0.0f, 0.0f};
	return pData->arrPages[iIndex].tTextRect;
}

XUI_API xui_rect_t xuiTabsGetIconRect(xui_widget pWidget, int iIndex)
{
	xui_tabs_data_t* pData;

	pData = __xuiTabsGetData(pWidget);
	if ( (pData == NULL) || (iIndex < 0) || (iIndex >= pData->iPageCount) ) return (xui_rect_t){0.0f, 0.0f, 0.0f, 0.0f};
	return pData->arrPages[iIndex].tIconRect;
}

XUI_API xui_rect_t xuiTabsGetDirtyRect(xui_widget pWidget, int iIndex)
{
	xui_tabs_data_t* pData;

	pData = __xuiTabsGetData(pWidget);
	if ( (pData == NULL) || (iIndex < 0) || (iIndex >= pData->iPageCount) ) return (xui_rect_t){0.0f, 0.0f, 0.0f, 0.0f};
	return pData->arrPages[iIndex].tDirtyRect;
}

XUI_API xui_rect_t xuiTabsGetCloseRect(xui_widget pWidget, int iIndex)
{
	xui_tabs_data_t* pData;

	pData = __xuiTabsGetData(pWidget);
	if ( (pData == NULL) || (iIndex < 0) || (iIndex >= pData->iPageCount) ) return (xui_rect_t){0.0f, 0.0f, 0.0f, 0.0f};
	return pData->arrPages[iIndex].tCloseRect;
}

XUI_API int xuiTabsIsOverflow(xui_widget pWidget)
{
	xui_tabs_data_t* pData;

	pData = __xuiTabsGetData(pWidget);
	return (pData != NULL) ? pData->bOverflow : 0;
}

XUI_API xui_rect_t xuiTabsGetOverflowRect(xui_widget pWidget)
{
	xui_tabs_data_t* pData;

	pData = __xuiTabsGetData(pWidget);
	if ( pData == NULL ) return (xui_rect_t){0.0f, 0.0f, 0.0f, 0.0f};
	return pData->tOverflowRect;
}

XUI_API xui_widget xuiTabsGetOverflowMenu(xui_widget pWidget)
{
	xui_tabs_data_t* pData;

	pData = __xuiTabsGetData(pWidget);
	return (pData != NULL) ? pData->pOverflowMenu : NULL;
}

XUI_API int xuiTabsGetHoverIndex(xui_widget pWidget)
{
	xui_tabs_data_t* pData;

	pData = __xuiTabsGetData(pWidget);
	return (pData != NULL) ? pData->iHoverIndex : -1;
}

XUI_API int xuiTabsGetActiveIndex(xui_widget pWidget)
{
	xui_tabs_data_t* pData;

	pData = __xuiTabsGetData(pWidget);
	return (pData != NULL) ? pData->iActiveIndex : -1;
}

XUI_API int xuiTabsGetCloseHoverIndex(xui_widget pWidget)
{
	xui_tabs_data_t* pData;

	pData = __xuiTabsGetData(pWidget);
	return (pData != NULL) ? pData->iCloseHoverIndex : -1;
}

XUI_API int xuiTabsGetCloseActiveIndex(xui_widget pWidget)
{
	xui_tabs_data_t* pData;

	pData = __xuiTabsGetData(pWidget);
	return (pData != NULL) ? pData->iCloseActiveIndex : -1;
}

XUI_API uint32_t xuiTabsGetState(xui_widget pWidget)
{
	xui_tabs_data_t* pData;
	uint32_t iState;

	pData = __xuiTabsGetData(pWidget);
	if ( pData == NULL ) return 0;
	iState = 0;
	if ( (pData->iHoverIndex >= 0) || pData->bOverflowHover ) iState |= XUI_WIDGET_STATE_HOVER;
	if ( (pData->iActiveIndex >= 0) || pData->bOverflowActive ) iState |= XUI_WIDGET_STATE_ACTIVE;
	if ( xuiGetFocusWidget(xuiWidgetGetContext(pWidget)) == pWidget ) iState |= XUI_WIDGET_STATE_FOCUS;
	if ( !xuiWidgetGetEffectiveEnabled(pWidget) ) iState |= XUI_WIDGET_STATE_DISABLED;
	if ( (pData->pOverflowMenu != NULL) && xuiMenuIsOpen(pData->pOverflowMenu) ) iState |= XUI_TABS_STATE_OPEN;
	return iState;
}

XUI_API int xuiTabsGetChangeCount(xui_widget pWidget)
{
	xui_tabs_data_t* pData;

	pData = __xuiTabsGetData(pWidget);
	return (pData != NULL) ? pData->iChangeCount : 0;
}

XUI_API int xuiTabsGetCloseCount(xui_widget pWidget)
{
	xui_tabs_data_t* pData;

	pData = __xuiTabsGetData(pWidget);
	return (pData != NULL) ? pData->iCloseCount : 0;
}

#endif
