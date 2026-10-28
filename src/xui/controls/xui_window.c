#include "xui_config.h"
#if XUI_ENABLE_WINDOW
#include "xui_internal.h"

#include <string.h>

typedef struct xui_window_data_t {
	xui_widget pClient;
	xui_widget pCollapseButton;
	xui_widget pMaximizeButton;
	xui_widget pCloseButton;
	xui_font pFont;
	xui_surface pIconSurface;
	xui_rect_t tIconSrc;
	xui_window_close_proc onClose;
	void* pCloseUser;
	xui_window_context_proc onContext;
	void* pContextUser;
	char sTitle[XUI_WINDOW_TITLE_CAPACITY];
	int bHasIcon;
	int bOpen;
	int bTopMost;
	int bActive;
	int bShowTitleBar;
	int bMovable;
	int bDragAnywhere;
	int bResizable;
	int bResizeEdgesExplicit;
	uint32_t iResizeEdges;
	int bShowCollapse;
	int bShowMaximize;
	int bShowClose;
	int bCollapsed;
	int bMaximized;
	int iHoverPart;
	int iActivePart;
	int iInteractionEdges;
	uint64_t iInteractionPointerId;
	int iInteractionPointerType;
	uint64_t iButtonPointerId;
	int iButtonPointerType;
	xui_rect_t tRestoreRect;
	xui_rect_t tDragStartRect;
	float fDragStartX;
	float fDragStartY;
	float fExpandedHeight;
	float fTitleBarHeight;
	float fBorderWidth;
	float fResizeGrip;
	float fButtonSize;
	float fIconSize;
	float fMinWidth;
	float fMinHeight;
	uint32_t iBackgroundColor;
	uint32_t iClientColor;
	uint32_t iTitleBarColor;
	uint32_t iInactiveTitleBarColor;
	uint32_t iTitleTextColor;
	uint32_t iInactiveTitleTextColor;
	uint32_t iBorderColor;
	uint32_t iActiveBorderColor;
	uint32_t iButtonColor;
	uint32_t iButtonHoverColor;
	uint32_t iButtonActiveColor;
	uint32_t iCloseHoverColor;
	uint32_t iCloseActiveColor;
	xui_rect_t tTitleBarRect;
	xui_rect_t tClientRect;
	xui_rect_t tCollapseButtonRect;
	xui_rect_t tMaximizeButtonRect;
	xui_rect_t tCloseButtonRect;
	int iChangeCount;
	int iCloseCount;
	uint32_t iChromeStyleVersion;
} xui_window_data_t;

typedef struct xui_window_resolved_t {
	uint32_t iCloseIconColor;
	uint32_t iTopMostColor;
	xui_font pFont;
	float fTitleBarHeight;
	float fBorderWidth;
	float fResizeGrip;
	float fButtonSize;
	float fIconSize;
	float fMinWidth;
	float fMinHeight;
	uint32_t iBackgroundColor;
	uint32_t iClientColor;
	uint32_t iTitleBarColor;
	uint32_t iInactiveTitleBarColor;
	uint32_t iTitleTextColor;
	uint32_t iInactiveTitleTextColor;
	uint32_t iBorderColor;
	uint32_t iActiveBorderColor;
	uint32_t iButtonColor;
	uint32_t iButtonHoverColor;
	uint32_t iButtonActiveColor;
	uint32_t iCloseHoverColor;
	uint32_t iCloseActiveColor;
} xui_window_resolved_t;

static int __xuiWindowSetOpenInternal(xui_widget pWidget, xui_window_data_t* pData, int bOpen, int bRaise);
static int __xuiWindowApplyLayerAndZ(xui_widget pWidget, xui_window_data_t* pData);
static float __xuiWindowMinWidth(const xui_window_data_t* pData, const xui_window_resolved_t* pResolved);
static float __xuiWindowMinHeight(const xui_window_data_t* pData, const xui_window_resolved_t* pResolved);

static uint32_t __xuiWindowAlpha(uint32_t iColor)
{
	return iColor & 0xffu;
}

static float __xuiWindowMin(float fA, float fB)
{
	return (fA < fB) ? fA : fB;
}

static float __xuiWindowMax(float fA, float fB)
{
	return (fA > fB) ? fA : fB;
}

static float __xuiWindowClamp(float fValue, float fMin, float fMax)
{
	if ( fMax < fMin ) {
		return fMin;
	}
	if ( fValue < fMin ) {
		return fMin;
	}
	if ( fValue > fMax ) {
		return fMax;
	}
	return fValue;
}

static int __xuiWindowFloatValid(float fValue)
{
	return (fValue == fValue) && (fValue >= 0.0f) && (fValue <= XUI_LAYOUT_UNBOUNDED);
}

static uint32_t __xuiWindowDefaultResizeEdges(const xui_window_data_t* pData)
{
	uint32_t iEdges;

	iEdges = XUI_WINDOW_EDGE_ALL;
	if ( (pData != NULL) && !pData->bShowTitleBar && pData->bDragAnywhere ) {
		iEdges &= ~XUI_WINDOW_EDGE_TOP;
	}
	return iEdges;
}

static void __xuiWindowApplyDefaultResizeEdges(xui_window_data_t* pData)
{
	if ( (pData == NULL) || pData->bResizeEdgesExplicit ) {
		return;
	}
	pData->iResizeEdges = __xuiWindowDefaultResizeEdges(pData);
}

static int __xuiWindowDescValid(const xui_window_desc_t* pDesc)
{
	if ( pDesc == NULL ) {
		return 1;
	}
	if ( (pDesc->iSize != 0) && (pDesc->iSize < sizeof(*pDesc)) ) {
		return 0;
	}
	if ( pDesc->tIconSrc.fX < -(int)XUI_LAYOUT_UNBOUNDED || pDesc->tIconSrc.fX > (int)XUI_LAYOUT_UNBOUNDED ||
	     pDesc->tIconSrc.fY < -(int)XUI_LAYOUT_UNBOUNDED || pDesc->tIconSrc.fY > (int)XUI_LAYOUT_UNBOUNDED ||
	     pDesc->tIconSrc.fW < 0 || pDesc->tIconSrc.fW > (int)XUI_LAYOUT_UNBOUNDED ||
	     pDesc->tIconSrc.fH < 0 || pDesc->tIconSrc.fH > (int)XUI_LAYOUT_UNBOUNDED ) {
		return 0;
	}
	if ( ((pDesc->fTitleBarHeight != 0.0f) && !__xuiWindowFloatValid(pDesc->fTitleBarHeight)) ||
	     ((pDesc->fBorderWidth != 0.0f) && !__xuiWindowFloatValid(pDesc->fBorderWidth)) ||
	     ((pDesc->fResizeGrip != 0.0f) && !__xuiWindowFloatValid(pDesc->fResizeGrip)) ||
	     ((pDesc->fButtonSize != 0.0f) && !__xuiWindowFloatValid(pDesc->fButtonSize)) ||
	     ((pDesc->fIconSize != 0.0f) && !__xuiWindowFloatValid(pDesc->fIconSize)) ||
	     ((pDesc->fMinWidth != 0.0f) && !__xuiWindowFloatValid(pDesc->fMinWidth)) ||
	     ((pDesc->fMinHeight != 0.0f) && !__xuiWindowFloatValid(pDesc->fMinHeight)) ) {
		return 0;
	}
	return 1;
}

static xui_window_data_t* __xuiWindowGetData(xui_widget pWidget)
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
	pType = xuiWidgetFindType(pContext, "window");
	if ( (pType == NULL) || !xuiWidgetIsType(pWidget, pType) ) {
		return NULL;
	}
	return (xui_window_data_t*)xuiWidgetGetTypeData(pWidget);
}

static int __xuiWindowDataAlive(xui_widget pWidget, const xui_window_data_t* pExpected)
{
	xui_context pContext;

	if ( !xuiInternalWidgetIsValid(pWidget) ) return 0;
	pContext = xuiWidgetGetContext(pWidget);
	return xuiInternalContextIsValid(pContext) && !xuiInternalContextDestroyPending(pContext) &&
		__xuiWindowGetData(pWidget) == pExpected;
}

static int __xuiWindowRectEqual(xui_rect_t tA, xui_rect_t tB)
{
	return tA.fX == tB.fX && tA.fY == tB.fY && tA.fW == tB.fW && tA.fH == tB.fH;
}

static void __xuiWindowCopyTitle(char* sDst, size_t iCapacity, const char* sTitle)
{
	(void)xuiInternalUtf8CopyGraphemes(sDst, iCapacity, sTitle);
}

static int __xuiWindowRectContains(xui_rect_t tRect, float fX, float fY)
{
	return (fX >= tRect.fX) &&
	       (fY >= tRect.fY) &&
	       (fX < (tRect.fX + tRect.fW)) &&
	       (fY < (tRect.fY + tRect.fH));
}

static int __xuiWindowStyleColor(xui_widget pWidget, const char* sName, uint32_t* pColor)
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

static int __xuiWindowStyleFloat(xui_widget pWidget, const char* sName, float* pValue)
{
	xui_style_property_t tProperty;
	int iRet;

	memset(&tProperty, 0, sizeof(tProperty));
	tProperty.iSize = sizeof(tProperty);
	iRet = xuiWidgetGetResolvedStyleProperty(pWidget, sName, &tProperty);
	if ( (iRet == XUI_OK) && (tProperty.tValue.iType == XUI_STYLE_VALUE_FLOAT) && __xuiWindowFloatValid(tProperty.tValue.fFloat) ) {
		*pValue = tProperty.tValue.fFloat;
		return 1;
	}
	return 0;
}

static xui_font __xuiWindowStyleFont(xui_widget pWidget, xui_font pBaseFont)
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

static void __xuiWindowResolve(xui_widget pWidget, const xui_window_data_t* pData, xui_window_resolved_t* pResolved)
{
	memset(pResolved, 0, sizeof(*pResolved));
	pResolved->pFont = (pData->pFont != NULL) ? pData->pFont : xuiGetDefaultFont(xuiWidgetGetContext(pWidget));
	pResolved->fTitleBarHeight = pData->fTitleBarHeight;
	pResolved->fBorderWidth = pData->fBorderWidth;
	pResolved->fResizeGrip = pData->fResizeGrip;
	pResolved->fButtonSize = pData->fButtonSize;
	pResolved->fIconSize = pData->fIconSize;
	pResolved->fMinWidth = pData->fMinWidth;
	pResolved->fMinHeight = pData->fMinHeight;
	pResolved->iBackgroundColor = pData->iBackgroundColor;
	pResolved->iClientColor = pData->iClientColor;
	pResolved->iTitleBarColor = pData->iTitleBarColor;
	pResolved->iInactiveTitleBarColor = pData->iInactiveTitleBarColor;
	pResolved->iTitleTextColor = pData->iTitleTextColor;
	pResolved->iInactiveTitleTextColor = pData->iInactiveTitleTextColor;
	pResolved->iBorderColor = pData->iBorderColor;
	pResolved->iActiveBorderColor = pData->iActiveBorderColor;
	pResolved->iButtonColor = pData->iButtonColor;
	pResolved->iButtonHoverColor = pData->iButtonHoverColor;
	pResolved->iButtonActiveColor = pData->iButtonActiveColor;
	pResolved->iCloseHoverColor = pData->iCloseHoverColor;
	pResolved->iCloseActiveColor = pData->iCloseActiveColor;

	(void)__xuiWindowStyleColor(pWidget, "window.background.color", &pResolved->iBackgroundColor);
	(void)__xuiWindowStyleColor(pWidget, "window.client.color", &pResolved->iClientColor);
	(void)__xuiWindowStyleColor(pWidget, "window.titlebar.color", &pResolved->iTitleBarColor);
	(void)__xuiWindowStyleColor(pWidget, "window.titlebar.inactive_color", &pResolved->iInactiveTitleBarColor);
	(void)__xuiWindowStyleColor(pWidget, "window.title.text_color", &pResolved->iTitleTextColor);
	(void)__xuiWindowStyleColor(pWidget, "window.title.inactive_text_color", &pResolved->iInactiveTitleTextColor);
	(void)__xuiWindowStyleColor(pWidget, "window.border.color", &pResolved->iBorderColor);
	(void)__xuiWindowStyleColor(pWidget, "window.border.active_color", &pResolved->iActiveBorderColor);
	(void)__xuiWindowStyleColor(pWidget, "window.button.color", &pResolved->iButtonColor);
	(void)__xuiWindowStyleColor(pWidget, "window.button.hover_color", &pResolved->iButtonHoverColor);
	(void)__xuiWindowStyleColor(pWidget, "window.button.active_color", &pResolved->iButtonActiveColor);
	(void)__xuiWindowStyleColor(pWidget, "window.close.hover_color", &pResolved->iCloseHoverColor);
	(void)__xuiWindowStyleColor(pWidget, "window.close.active_color", &pResolved->iCloseActiveColor);
	pResolved->iCloseIconColor = XUI_COLOR_RGBA(171, 72, 76, 255);
	pResolved->iTopMostColor = XUI_COLOR_RGBA(47, 128, 208, 255);
	(void)__xuiWindowStyleColor(pWidget, "window.close.icon_color", &pResolved->iCloseIconColor);
	(void)__xuiWindowStyleColor(pWidget, "window.titlebar.topmost_color", &pResolved->iTopMostColor);
	(void)__xuiWindowStyleFloat(pWidget, "window.titlebar.height", &pResolved->fTitleBarHeight);
	(void)__xuiWindowStyleFloat(pWidget, "window.border.width", &pResolved->fBorderWidth);
	(void)__xuiWindowStyleFloat(pWidget, "window.resize_grip", &pResolved->fResizeGrip);
	(void)__xuiWindowStyleFloat(pWidget, "window.button.size", &pResolved->fButtonSize);
	(void)__xuiWindowStyleFloat(pWidget, "window.icon.size", &pResolved->fIconSize);
	(void)__xuiWindowStyleFloat(pWidget, "window.min_width", &pResolved->fMinWidth);
	(void)__xuiWindowStyleFloat(pWidget, "window.min_height", &pResolved->fMinHeight);
	pResolved->pFont = __xuiWindowStyleFont(pWidget, pResolved->pFont);

	if ( pResolved->fTitleBarHeight < 0.0f ) pResolved->fTitleBarHeight = 0.0f;
	if ( pResolved->fButtonSize < 12.0f ) pResolved->fButtonSize = 12.0f;
	if ( pResolved->fResizeGrip < 2.0f ) pResolved->fResizeGrip = 2.0f;
}

static void __xuiWindowFrameMetrics(const xui_window_data_t* pData,
	const xui_window_resolved_t* pResolved, xui_internal_window_frame_metrics_t* pMetrics)
{
	memset(pMetrics, 0, sizeof(*pMetrics));
	pMetrics->fTitleBarHeight = pResolved->fTitleBarHeight;
	pMetrics->fBorderWidth = pResolved->fBorderWidth;
	pMetrics->fResizeGrip = pResolved->fResizeGrip;
	pMetrics->fButtonSize = pResolved->fButtonSize;
	pMetrics->fButtonGap = 4.0f;
	pMetrics->fButtonInset = 5.0f;
	pMetrics->iResizeEdges = pData->iResizeEdges;
	pMetrics->bShowTitleBar = pData->bShowTitleBar;
	pMetrics->bResizable = pData->bResizable && !pData->bMaximized;
}

static int __xuiWindowDrawRectFill(xui_proxy pProxy, xui_draw_context pDraw, xui_rect_t tRect, uint32_t iColor)
{
	if ( __xuiWindowAlpha(iColor) == 0u ) {
		return XUI_OK;
	}
	if ( (pProxy != NULL) && (pProxy->drawRectFill != NULL) ) {
		return pProxy->drawRectFill(pProxy, pDraw, tRect, iColor);
	}
	return XUI_ERROR_NOT_INITIALIZED;
}

static int __xuiWindowDrawRectStroke(xui_proxy pProxy, xui_draw_context pDraw, xui_rect_t tRect, float fWidth, uint32_t iColor)
{
	if ( (fWidth <= 0.0f) || (__xuiWindowAlpha(iColor) == 0u) ) {
		return XUI_OK;
	}
	if ( (pProxy != NULL) && (pProxy->drawRectStroke != NULL) ) {
		return pProxy->drawRectStroke(pProxy, pDraw, tRect, fWidth, iColor);
	}
	return XUI_ERROR_NOT_INITIALIZED;
}

static int __xuiWindowDrawLine(xui_proxy pProxy, xui_draw_context pDraw, float fX0, float fY0, float fX1, float fY1, float fWidth, uint32_t iColor)
{
	if ( __xuiWindowAlpha(iColor) == 0u ) {
		return XUI_OK;
	}
	if ( (pProxy != NULL) && (pProxy->drawLine != NULL) ) {
		return pProxy->drawLine(pProxy, pDraw, fX0, fY0, fX1, fY1, fWidth, iColor);
	}
	if ( (pProxy != NULL) && (pProxy->drawRectFill != NULL) ) {
		float fX = __xuiWindowMin(fX0, fX1);
		float fY = __xuiWindowMin(fY0, fY1);
		float fW = __xuiWindowMax(1.0f, __xuiWindowMax(fX0, fX1) - fX + fWidth);
		float fH = __xuiWindowMax(1.0f, __xuiWindowMax(fY0, fY1) - fY + fWidth);
		return pProxy->drawRectFill(pProxy, pDraw, xuiInternalRectFromFloatNearest(fX, fY, fW, fH), iColor);
	}
	return XUI_ERROR_NOT_INITIALIZED;
}

static int __xuiWindowDrawBottomSquareFill(xui_proxy pProxy, xui_draw_context pDraw, xui_rect_t tRect, uint32_t iColor)
{
	return __xuiWindowDrawRectFill(pProxy, pDraw, tRect, iColor);
}

static int __xuiWindowDrawBottomSquareStroke(xui_proxy pProxy, xui_draw_context pDraw, xui_rect_t tRect, float fWidth, uint32_t iBottomFillColor, uint32_t iBorderColor)
{
	(void)iBottomFillColor;
	return __xuiWindowDrawRectStroke(pProxy, pDraw, tRect, fWidth, iBorderColor);
}

static float __xuiWindowCollapsedHeight(const xui_window_data_t* pData, const xui_window_resolved_t* pResolved)
{
	float fHeight;

	if ( (pData == NULL) || (pResolved == NULL) ) {
		return 28.0f;
	}
	fHeight = pResolved->fBorderWidth * 2.0f;
	if ( pData->bShowTitleBar ) {
		fHeight += pResolved->fTitleBarHeight;
	} else {
		fHeight += 24.0f;
	}
	if ( fHeight < 24.0f ) {
		fHeight = 24.0f;
	}
	return fHeight;
}

static float __xuiWindowMinWidth(const xui_window_data_t* pData, const xui_window_resolved_t* pResolved)
{
	float fWidth;
	float fButtonCount;

	if ( (pData == NULL) || (pResolved == NULL) ) {
		return 120.0f;
	}
	fWidth = (pResolved->fMinWidth > 0.0f) ? pResolved->fMinWidth : 160.0f;
	if ( pData->bShowTitleBar ) {
		fButtonCount = 0.0f;
		if ( pData->bShowCollapse ) fButtonCount += 1.0f;
		if ( pData->bShowMaximize ) fButtonCount += 1.0f;
		if ( pData->bShowClose ) fButtonCount += 1.0f;
		fWidth = __xuiWindowMax(fWidth, 16.0f + fButtonCount * (pResolved->fButtonSize + 4.0f) + 72.0f);
	}
	return __xuiWindowMax(fWidth, 120.0f);
}

static float __xuiWindowMinHeight(const xui_window_data_t* pData, const xui_window_resolved_t* pResolved)
{
	float fHeight;
	float fCollapsed;

	if ( (pData == NULL) || (pResolved == NULL) ) {
		return 80.0f;
	}
	fCollapsed = __xuiWindowCollapsedHeight(pData, pResolved);
	if ( pData->bCollapsed ) {
		return fCollapsed;
	}
	fHeight = (pResolved->fMinHeight > 0.0f) ? pResolved->fMinHeight : (fCollapsed + 72.0f);
	return __xuiWindowMax(fHeight, fCollapsed + 32.0f);
}

static xui_rect_t __xuiWindowParentRect(xui_widget pWidget)
{
	xui_context pContext;
	xui_widget pParent;
	xui_size_t tViewport;
	xui_rect_t tRect;

	memset(&tRect, 0, sizeof(tRect));
	if ( !xuiInternalWidgetIsValid(pWidget) ) {
		return tRect;
	}
	pContext = xuiWidgetGetContext(pWidget);
	tViewport = xuiGetViewportSize(pContext);
	if ( tViewport.iW > 0 && tViewport.iH > 0 ) {
		tRect.fW = tViewport.iW;
		tRect.fH = tViewport.iH;
		return tRect;
	}
	pParent = xuiWidgetGetParent(pWidget);
	if ( pParent != NULL ) {
		tRect = xuiWidgetGetWorldRect(pParent);
		tRect.fX = 0.0f;
		tRect.fY = 0.0f;
		return tRect;
	}
	tRect.fW = 1.0f;
	tRect.fH = 1.0f;
	return tRect;
}

static xui_rect_t __xuiWindowClampRect(xui_widget pWidget, xui_window_data_t* pData, const xui_window_resolved_t* pResolved, xui_rect_t tRect)
{
	xui_rect_t tParent;
	float fMinW;
	float fMinH;

	tParent = __xuiWindowParentRect(pWidget);
	fMinW = __xuiWindowMinWidth(pData, pResolved);
	fMinH = pData->bCollapsed ? __xuiWindowCollapsedHeight(pData, pResolved) : __xuiWindowMinHeight(pData, pResolved);
	if ( pData->bCollapsed ) tRect.fH = 0;
	return xuiInternalWindowFrameClamp(tRect, tParent, fMinW, fMinH);
}

static int __xuiWindowApplyRect(xui_widget pWidget, xui_window_data_t* pData, xui_rect_t tRect)
{
	xui_window_resolved_t tResolved;
	float fExpandedHeight;
	int iRet;

	if ( (pWidget == NULL) || (pData == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	__xuiWindowResolve(pWidget, pData, &tResolved);
	tRect = __xuiWindowClampRect(pWidget, pData, &tResolved, tRect);
	fExpandedHeight = pData->fExpandedHeight;
	if ( !pData->bCollapsed && tRect.fH > __xuiWindowCollapsedHeight(pData, &tResolved) ) {
		fExpandedHeight = tRect.fH;
	}
	iRet = xuiWidgetSetRect(pWidget, tRect);
	if ( iRet == XUI_OK ) pData->fExpandedHeight = fExpandedHeight;
	return iRet;
}

static int __xuiWindowApplyLayerAndZ(xui_widget pWidget, xui_window_data_t* pData)
{
	int iLayer;
	int iZIndex;

	if ( (pWidget == NULL) || (pData == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( pWidget->bModalFocus &&
		xuiWidgetGetLayer(pWidget, &iLayer, &iZIndex) == XUI_OK &&
		iLayer >= XUI_LAYER_MODAL ) {
		return xuiInternalWidgetSetTreeLayer(pWidget, iLayer, iZIndex);
	}
	return xuiInternalWidgetSetTreeLayer(pWidget, XUI_LAYER_FLOATING,
		pData->bTopMost ? XUI_WINDOW_Z_TOPMOST : XUI_WINDOW_Z_NORMAL);
}

static int __xuiWindowDeactivateInTree(xui_widget pRoot, xui_widget pExcept)
{
	xui_widget pChild;
	xui_window_data_t* pData;
	int iRet;

	if ( pRoot == NULL ) {
		return XUI_OK;
	}
	pData = __xuiWindowGetData(pRoot);
	if ( (pData != NULL) && (pRoot != pExcept) && pData->bActive ) {
		pData->bActive = 0;
		(void)__xuiWindowApplyLayerAndZ(pRoot, pData);
		iRet = xuiWidgetInvalidate(pRoot, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
		if ( iRet != XUI_OK ) return iRet;
	}
	for ( pChild = pRoot->pFirstChild; pChild != NULL; pChild = pChild->pNextSibling ) {
		iRet = __xuiWindowDeactivateInTree(pChild, pExcept);
		if ( iRet != XUI_OK ) return iRet;
	}
	return XUI_OK;
}

static int __xuiWindowActivate(xui_widget pWidget, xui_window_data_t* pData, int bRaise)
{
	xui_context pContext;
	xui_widget pOverlayRoot;
	int iRet;

	if ( (pWidget == NULL) || (pData == NULL) || !pData->bOpen ) {
		return XUI_OK;
	}
	if ( !xuiWidgetGetEffectiveVisible(pWidget) || !xuiWidgetGetEffectiveEnabled(pWidget) ) {
		return XUI_ERROR_INVALID_STATE;
	}
	pContext = xuiWidgetGetContext(pWidget);
	iRet = __xuiWindowDeactivateInTree(xuiGetRootWidget(pContext), pWidget);
	if ( iRet != XUI_OK ) return iRet;
	iRet = __xuiWindowDeactivateInTree(xuiOverlayRoot(pContext), pWidget);
	if ( iRet != XUI_OK ) return iRet;
	pData->bActive = 1;
	iRet = __xuiWindowApplyLayerAndZ(pWidget, pData);
	if ( iRet != XUI_OK ) return iRet;
	if ( bRaise ) {
		pOverlayRoot = xuiOverlayRoot(pContext);
		if ( pWidget->pParent != pOverlayRoot ) {
			iRet = xuiOverlayAttach(pContext, NULL, pWidget, XUI_LAYER_FLOATING, pData->bTopMost ? XUI_WINDOW_Z_TOPMOST : XUI_WINDOW_Z_NORMAL);
			if ( iRet != XUI_OK ) return iRet;
			(void)__xuiWindowApplyLayerAndZ(pWidget, pData);
		} else {
			iRet = xuiOverlayBringToFront(pWidget);
			if ( iRet != XUI_OK ) return iRet;
		}
	}
	iRet = xuiSetFocusWidget(pContext, pWidget);
	if ( iRet != XUI_OK ) return iRet;
	if ( !__xuiWindowDataAlive(pWidget, pData) ) return XUI_OK;
	return xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
}

static xui_widget __xuiWindowFindActiveInTree(xui_widget pRoot)
{
	xui_widget pChild;
	xui_widget pFound;
	xui_window_data_t* pData;

	if ( pRoot == NULL ) {
		return NULL;
	}
	pData = __xuiWindowGetData(pRoot);
	if ( (pData != NULL) && pData->bOpen && pData->bActive &&
	     xuiWidgetGetEffectiveVisible(pRoot) && xuiWidgetGetEffectiveEnabled(pRoot) ) {
		return pRoot;
	}
	for ( pChild = pRoot->pFirstChild; pChild != NULL; pChild = pChild->pNextSibling ) {
		pFound = __xuiWindowFindActiveInTree(pChild);
		if ( pFound != NULL ) {
			return pFound;
		}
	}
	return NULL;
}

static uint32_t __xuiWindowResizeEdgesAtData(xui_widget pWidget, xui_window_data_t* pData, const xui_window_resolved_t* pResolved, float fX, float fY)
{
	xui_rect_t tWorld;
	xui_internal_window_frame_metrics_t tMetrics;

	if ( (pWidget == NULL) || (pData == NULL) || (pResolved == NULL) || !pData->bResizable || pData->bMaximized ) {
		return 0u;
	}
	tWorld = xuiWidgetGetWorldRect(pWidget);
	__xuiWindowFrameMetrics(pData, pResolved, &tMetrics);
	return xuiInternalWindowFrameResizeEdgesAt(tWorld, &tMetrics, fX, fY);
}

static int __xuiWindowQueryCursor(xui_widget pWidget, int iX, int iY, void* pUser)
{
	xui_window_data_t* pData = (xui_window_data_t*)pUser;
	xui_window_resolved_t tResolved;
	uint32_t iEdges;

	if ( (pWidget == NULL) || (pData == NULL) ) return XUI_CURSOR_INHERIT;
	if ( !xuiWidgetGetEffectiveEnabled(pWidget) ) return XUI_CURSOR_NOT_ALLOWED;
	__xuiWindowResolve(pWidget, pData, &tResolved);
	iEdges = __xuiWindowResizeEdgesAtData(pWidget, pData, &tResolved, (float)iX, (float)iY);
	return xuiInternalWindowFrameResizeCursor(iEdges);
}

static int __xuiWindowCanStartMove(xui_widget pWidget, xui_window_data_t* pData, const xui_window_resolved_t* pResolved, const xui_event_t* pEvent)
{
	xui_rect_t tWorld;
	float fLocalX;
	float fLocalY;

	if ( (pWidget == NULL) || (pData == NULL) || (pResolved == NULL) || (pEvent == NULL) ) {
		return 0;
	}
	if ( !pData->bMovable || pData->bMaximized ) {
		return 0;
	}
	tWorld = xuiWidgetGetWorldRect(pWidget);
	fLocalX = pEvent->fX - tWorld.fX;
	fLocalY = pEvent->fY - tWorld.fY;
	if ( pData->bDragAnywhere ) {
		return (pEvent->pTarget == pWidget) || (pEvent->pTarget == pData->pClient);
	}
	if ( !pData->bShowTitleBar || pEvent->pTarget != pWidget ) {
		return 0;
	}
	return __xuiWindowRectContains(xuiInternalRectFromFloatNearest(0.0f, 0.0f, tWorld.fW,
		pResolved->fBorderWidth + pResolved->fTitleBarHeight), fLocalX, fLocalY);
}

static xui_rect_t __xuiWindowBuildInteractionRect(xui_widget pWidget, xui_window_data_t* pData, float fX, float fY)
{
	xui_window_resolved_t tResolved;
	xui_rect_t tRect;
	xui_rect_t tBounds;
	float fDX;
	float fDY;
	float fMinW;
	float fMinH;

	__xuiWindowResolve(pWidget, pData, &tResolved);
	tRect = pData->tDragStartRect;
	fDX = fX - pData->fDragStartX;
	fDY = fY - pData->fDragStartY;
	tBounds = __xuiWindowParentRect(pWidget);
	if ( pData->iInteractionEdges == -1 ) {
		return xuiInternalWindowFrameMove(tRect, tBounds, fDX, fDY);
	}
	fMinW = __xuiWindowMinWidth(pData, &tResolved);
	fMinH = __xuiWindowMinHeight(pData, &tResolved);
	return xuiInternalWindowFrameResize(tRect, tBounds, (uint32_t)pData->iInteractionEdges,
		fDX, fDY, fMinW, fMinH);
}

static xui_rect_t __xuiWindowInteractionWorldRect(xui_widget pWidget, xui_rect_t tRect)
{
	xui_rect_t tLocal;
	xui_rect_t tWorld;

	tLocal = xuiWidgetGetRect(pWidget);
	tWorld = xuiWidgetGetWorldRect(pWidget);
	tRect.fX += tWorld.fX - tLocal.fX;
	tRect.fY += tWorld.fY - tLocal.fY;
	return tRect;
}

static int __xuiWindowUpdateInteractionAdorner(xui_widget pWidget, xui_window_data_t* pData, xui_rect_t tRect)
{
	xui_drag_adorner_primitive_t tPrimitive;
	xui_window_resolved_t tResolved;

	__xuiWindowResolve(pWidget, pData, &tResolved);
	memset(&tPrimitive, 0, sizeof(tPrimitive));
	tPrimitive.iType = XUI_DRAG_ADORNER_RECT_STROKE;
	tPrimitive.tRect = __xuiWindowInteractionWorldRect(pWidget, tRect);
	tPrimitive.fWidth = 2.0f;
	tPrimitive.iColor = tResolved.iActiveBorderColor;
	return xuiInternalDragAdornerSet(xuiWidgetGetContext(pWidget), pWidget, &tPrimitive, 1);
}

static int __xuiWindowSetHoverPart(xui_widget pWidget, xui_window_data_t* pData, int iPart)
{
	if ( pData->iHoverPart == iPart ) {
		return XUI_OK;
	}
	pData->iHoverPart = iPart;
	return xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
}

static int __xuiWindowSetActivePart(xui_widget pWidget, xui_window_data_t* pData, int iPart)
{
	if ( pData->iActivePart == iPart ) {
		return XUI_OK;
	}
	pData->iActivePart = iPart;
	return xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
}

static int __xuiWindowButtonPart(xui_widget pButton, const xui_window_data_t* pData)
{
	if ( pButton == NULL || pData == NULL ) return XUI_WINDOW_PART_NONE;
	if ( pButton == pData->pCollapseButton ) return XUI_WINDOW_PART_COLLAPSE;
	if ( pButton == pData->pMaximizeButton ) return XUI_WINDOW_PART_MAXIMIZE;
	if ( pButton == pData->pCloseButton ) return XUI_WINDOW_PART_CLOSE;
	return XUI_WINDOW_PART_NONE;
}

static xui_widget __xuiWindowButtonForPart(const xui_window_data_t* pData, int iPart)
{
	if ( pData == NULL ) return NULL;
	if ( iPart == XUI_WINDOW_PART_COLLAPSE ) return pData->pCollapseButton;
	if ( iPart == XUI_WINDOW_PART_MAXIMIZE ) return pData->pMaximizeButton;
	if ( iPart == XUI_WINDOW_PART_CLOSE ) return pData->pCloseButton;
	return NULL;
}

static int __xuiWindowInteractionPointerMatches(const xui_window_data_t* pData,
	const xui_event_t* pEvent)
{
	return pData != NULL && pEvent != NULL && pData->iInteractionEdges != 0 &&
		pData->iInteractionPointerId == pEvent->iPointerId &&
		pData->iInteractionPointerType == pEvent->iPointerType;
}

static int __xuiWindowButtonPointerMatches(const xui_window_data_t* pData,
	const xui_event_t* pEvent)
{
	return pData != NULL && pEvent != NULL &&
		__xuiWindowButtonForPart(pData, pData->iActivePart) != NULL &&
		pData->iButtonPointerId == pEvent->iPointerId &&
		pData->iButtonPointerType == pEvent->iPointerType;
}

static void __xuiWindowCancelInteraction(xui_widget pWidget, xui_window_data_t* pData,
	int bReleaseCapture)
{
	xui_context pContext;
	uint64_t iPointerId;
	int iPointerType;

	if ( pWidget == NULL || pData == NULL || pData->iInteractionEdges == 0 ) return;
	pContext = xuiWidgetGetContext(pWidget);
	iPointerId = pData->iInteractionPointerId;
	iPointerType = pData->iInteractionPointerType;
	pData->iInteractionEdges = 0;
	pData->iInteractionPointerId = 0;
	pData->iInteractionPointerType = XUI_POINTER_TYPE_MOUSE;
	xuiInternalDragAdornerHide(pContext, pWidget);
	(void)__xuiWindowSetActivePart(pWidget, pData, XUI_WINDOW_PART_NONE);
	if ( bReleaseCapture && xuiInternalContextIsValid(pContext) &&
	     xuiGetPointerCaptureEx(pContext, iPointerId, iPointerType) == pWidget ) {
		(void)xuiReleasePointerCaptureEx(pContext, iPointerId, iPointerType, pWidget);
	}
}

static void __xuiWindowCancelButtonPress(xui_widget pWindow, xui_window_data_t* pData,
	int bReleaseCapture)
{
	xui_context pContext;
	xui_widget pButton;
	uint64_t iPointerId;
	int iPointerType;

	if ( pWindow == NULL || pData == NULL ) return;
	pButton = __xuiWindowButtonForPart(pData, pData->iActivePart);
	if ( pButton == NULL ) return;
	pContext = xuiWidgetGetContext(pWindow);
	iPointerId = pData->iButtonPointerId;
	iPointerType = pData->iButtonPointerType;
	pData->iButtonPointerId = 0;
	pData->iButtonPointerType = XUI_POINTER_TYPE_MOUSE;
	(void)__xuiWindowSetActivePart(pWindow, pData, XUI_WINDOW_PART_NONE);
	if ( bReleaseCapture && xuiInternalContextIsValid(pContext) &&
	     xuiGetPointerCaptureEx(pContext, iPointerId, iPointerType) == pButton ) {
		(void)xuiReleasePointerCaptureEx(pContext, iPointerId, iPointerType, pButton);
	}
}

static int __xuiWindowRequestClose(xui_widget pWidget, xui_window_data_t* pData)
{
	xui_window_close_proc onClose;
	void* pCloseUser;
	int iRet;

	if ( pWidget == NULL || pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( !pData->bOpen ) return XUI_OK;
	onClose = pData->onClose;
	pCloseUser = pData->pCloseUser;
	iRet = xuiWindowSetOpen(pWidget, 0);
	if ( iRet != XUI_OK || !__xuiWindowDataAlive(pWidget, pData) ) return iRet;
	pData->iCloseCount++;
	if ( onClose != NULL ) onClose(pWidget, pCloseUser);
	return XUI_OK;
}

static xui_rect_t __xuiWindowButtonRect(xui_widget pButton)
{
	xui_rect_t tRect;

	tRect = xuiWidgetGetRect(pButton);
	tRect.fX = 0.0f;
	tRect.fY = 0.0f;
	return xuiInternalSnapRect(tRect);
}

static int __xuiWindowButtonRender(xui_widget pButton, xui_draw_context pDraw, uint32_t iStateId, void* pUser)
{
	xui_widget pWindow;
	xui_window_data_t* pData;
	xui_window_resolved_t tResolved;
	xui_proxy pProxy;
	xui_rect_t tRect;
	xui_rect_t tIcon;
	uint32_t iFill;
	uint32_t iIcon;
	int iPart;
	int iRet;

	(void)iStateId;
	pWindow = (xui_widget)pUser;
	pData = __xuiWindowGetData(pWindow);
	if ( (pButton == NULL) || (pDraw == NULL) || (pData == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	__xuiWindowResolve(pWindow, pData, &tResolved);
	pProxy = xuiInternalContextGetProxy(xuiWidgetGetContext(pButton));
	if ( pProxy == NULL ) {
		return XUI_ERROR_NOT_INITIALIZED;
	}
	iPart = __xuiWindowButtonPart(pButton, pData);
	tRect = __xuiWindowButtonRect(pButton);
	iFill = tResolved.iButtonColor;
	if ( pData->iActivePart == iPart ) {
		iFill = (iPart == XUI_WINDOW_PART_CLOSE) ? tResolved.iCloseActiveColor : tResolved.iButtonActiveColor;
	} else if ( pData->iHoverPart == iPart ) {
		iFill = (iPart == XUI_WINDOW_PART_CLOSE) ? tResolved.iCloseHoverColor : tResolved.iButtonHoverColor;
	}
	iIcon = (iPart == XUI_WINDOW_PART_CLOSE) ? tResolved.iCloseIconColor :
		(pData->bActive ? tResolved.iTitleTextColor : tResolved.iInactiveTitleTextColor);
	iRet = __xuiWindowDrawRectFill(pProxy, pDraw, tRect, iFill);
	if ( iRet != XUI_OK ) return iRet;
	tIcon = xuiInternalRectFromFloatNearest(tRect.fX + (tRect.fW - 10.0f) * 0.5f,
		tRect.fY + (tRect.fH - 10.0f) * 0.5f, 10.0f, 10.0f);
	if ( iPart == XUI_WINDOW_PART_COLLAPSE ) {
		if ( pData->bCollapsed ) {
			iRet = __xuiWindowDrawLine(pProxy, pDraw, tIcon.fX + 1.0f, tIcon.fY + 6.0f, tIcon.fX + 5.0f, tIcon.fY + 2.0f, 1.2f, iIcon);
			if ( iRet != XUI_OK ) return iRet;
			return __xuiWindowDrawLine(pProxy, pDraw, tIcon.fX + 5.0f, tIcon.fY + 2.0f, tIcon.fX + 9.0f, tIcon.fY + 6.0f, 1.2f, iIcon);
		}
		iRet = __xuiWindowDrawLine(pProxy, pDraw, tIcon.fX + 1.0f, tIcon.fY + 4.0f, tIcon.fX + 5.0f, tIcon.fY + 8.0f, 1.2f, iIcon);
		if ( iRet != XUI_OK ) return iRet;
		return __xuiWindowDrawLine(pProxy, pDraw, tIcon.fX + 5.0f, tIcon.fY + 8.0f, tIcon.fX + 9.0f, tIcon.fY + 4.0f, 1.2f, iIcon);
	}
	if ( iPart == XUI_WINDOW_PART_MAXIMIZE ) {
		if ( pData->bMaximized ) {
			iRet = __xuiWindowDrawRectStroke(pProxy, pDraw, (xui_rect_t){tIcon.fX + 1.0f, tIcon.fY + 4.0f, 6.0f, 5.0f}, 1.0f, iIcon);
			if ( iRet != XUI_OK ) return iRet;
			return __xuiWindowDrawRectStroke(pProxy, pDraw, (xui_rect_t){tIcon.fX + 3.0f, tIcon.fY + 1.0f, 6.0f, 5.0f}, 1.0f, iIcon);
		}
		return __xuiWindowDrawRectStroke(pProxy, pDraw, (xui_rect_t){tIcon.fX + 1.0f, tIcon.fY + 1.0f, 8.0f, 8.0f}, 1.0f, iIcon);
	}
	if ( iPart == XUI_WINDOW_PART_CLOSE ) {
		iRet = __xuiWindowDrawLine(pProxy, pDraw, tIcon.fX + 1.0f, tIcon.fY + 1.0f, tIcon.fX + 9.0f, tIcon.fY + 9.0f, 1.3f, iIcon);
		if ( iRet != XUI_OK ) return iRet;
		return __xuiWindowDrawLine(pProxy, pDraw, tIcon.fX + 9.0f, tIcon.fY + 1.0f, tIcon.fX + 1.0f, tIcon.fY + 9.0f, 1.3f, iIcon);
	}
	return XUI_OK;
}

static int __xuiWindowButtonEvent(xui_widget pButton, const xui_event_t* pEvent, void* pUser)
{
	xui_widget pWindow;
	xui_window_data_t* pData;
	xui_context pContext;
	int iPart;
	int bLeftButton;
	int bInside;
	int iRet;

	pWindow = (xui_widget)pUser;
	pData = __xuiWindowGetData(pWindow);
	if ( (pButton == NULL) || (pEvent == NULL) || (pData == NULL) ) {
		return XUI_OK;
	}
	pContext = xuiWidgetGetContext(pWindow);
	iPart = __xuiWindowButtonPart(pButton, pData);
	bLeftButton = (pEvent->iButton == 0) || (pEvent->iButton == XUI_POINTER_BUTTON_LEFT);
	switch ( pEvent->iType ) {
	case XUI_EVENT_POINTER_ENTER:
	case XUI_EVENT_POINTER_MOVE:
		return __xuiWindowSetHoverPart(pWindow, pData, iPart);
	case XUI_EVENT_POINTER_LEAVE:
		if ( pData->iHoverPart == iPart ) {
			return __xuiWindowSetHoverPart(pWindow, pData, XUI_WINDOW_PART_NONE);
		}
		return XUI_OK;
	case XUI_EVENT_POINTER_DOWN:
		if ( bLeftButton && xuiWidgetGetEffectiveEnabled(pWindow) ) {
			if ( __xuiWindowButtonForPart(pData, pData->iActivePart) != NULL ) {
				return XUI_EVENT_DISPATCH_STOP;
			}
			iRet = __xuiWindowActivate(pWindow, pData, 1);
			if ( iRet != XUI_OK ) return iRet;
			if ( !__xuiWindowDataAlive(pWindow, pData) ) return XUI_EVENT_DISPATCH_STOP;
			iRet = __xuiWindowSetActivePart(pWindow, pData, iPart);
			if ( iRet != XUI_OK ) {
				(void)__xuiWindowSetActivePart(pWindow, pData, XUI_WINDOW_PART_NONE);
				return iRet;
			}
			iRet = xuiSetPointerCaptureEx(pContext, pEvent->iPointerId,
				pEvent->iPointerType, pButton);
			if ( iRet != XUI_OK ) {
				(void)__xuiWindowSetActivePart(pWindow, pData, XUI_WINDOW_PART_NONE);
				return iRet;
			}
			pData->iButtonPointerId = pEvent->iPointerId;
			pData->iButtonPointerType = pEvent->iPointerType;
			return XUI_EVENT_DISPATCH_STOP;
		}
		break;
	case XUI_EVENT_POINTER_UP:
		if ( bLeftButton && __xuiWindowButtonForPart(pData, pData->iActivePart) != NULL &&
		     !__xuiWindowButtonPointerMatches(pData, pEvent) ) {
			return XUI_EVENT_DISPATCH_STOP;
		}
		if ( bLeftButton && (pData->iActivePart == iPart) &&
		     __xuiWindowButtonPointerMatches(pData, pEvent) ) {
			xui_rect_t tWorld = xuiWidgetGetWorldRect(pButton);
			bInside = __xuiWindowRectContains(tWorld, pEvent->fX, pEvent->fY);
			__xuiWindowCancelButtonPress(pWindow, pData, 1);
			if ( bInside ) {
				if ( iPart == XUI_WINDOW_PART_COLLAPSE ) {
					iRet = xuiWindowSetCollapsed(pWindow, !pData->bCollapsed);
					if ( iRet != XUI_OK ) return iRet;
					return XUI_EVENT_DISPATCH_STOP;
				} else if ( iPart == XUI_WINDOW_PART_MAXIMIZE ) {
					iRet = xuiWindowSetMaximized(pWindow, !pData->bMaximized);
					if ( iRet != XUI_OK ) return iRet;
					return XUI_EVENT_DISPATCH_STOP;
				} else if ( iPart == XUI_WINDOW_PART_CLOSE ) {
					iRet = __xuiWindowRequestClose(pWindow, pData);
					if ( iRet != XUI_OK ) return iRet;
					return XUI_EVENT_DISPATCH_STOP;
				}
			}
			return XUI_EVENT_DISPATCH_STOP;
		}
		break;
	case XUI_EVENT_POINTER_CAPTURE_LOST:
		if ( pData->iActivePart == iPart && __xuiWindowButtonPointerMatches(pData, pEvent) ) {
			__xuiWindowCancelButtonPress(pWindow, pData, 0);
		}
		return XUI_EVENT_DISPATCH_STOP;
	default:
		break;
	}
	return XUI_OK;
}

static int __xuiWindowClientRender(xui_widget pClient, xui_draw_context pDraw, uint32_t iStateId, void* pUser)
{
	xui_widget pWindow;
	xui_window_data_t* pData;
	xui_window_resolved_t tResolved;
	xui_proxy pProxy;
	xui_rect_t tRect;

	(void)iStateId;
	pWindow = (xui_widget)pUser;
	pData = __xuiWindowGetData(pWindow);
	if ( (pClient == NULL) || (pDraw == NULL) || (pData == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	__xuiWindowResolve(pWindow, pData, &tResolved);
	pProxy = xuiInternalContextGetProxy(xuiWidgetGetContext(pClient));
	if ( pProxy == NULL ) {
		return XUI_ERROR_NOT_INITIALIZED;
	}
	tRect = xuiWidgetGetRect(pClient);
	tRect.fX = 0.0f;
	tRect.fY = 0.0f;
	return __xuiWindowDrawRectFill(pProxy, pDraw, xuiInternalSnapRect(tRect), tResolved.iClientColor);
}

static int __xuiWindowCacheRender(xui_widget pWidget, xui_draw_context pDraw, uint32_t iStateId, void* pUser)
{
	xui_window_data_t* pData;
	xui_window_resolved_t tResolved;
	xui_proxy pProxy;
	xui_rect_t tRect;
	xui_rect_t tTitle;
	xui_rect_t tText;
	xui_rect_t tIcon;
	uint32_t iBorder;
	uint32_t iTitle;
	uint32_t iTitleText;
	uint32_t iBottomFill;
	int iTextRight;
	int iRet;

	(void)iStateId;
	(void)pUser;
	pData = __xuiWindowGetData(pWidget);
	if ( (pWidget == NULL) || (pDraw == NULL) || (pData == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	__xuiWindowResolve(pWidget, pData, &tResolved);
	pProxy = xuiInternalContextGetProxy(xuiWidgetGetContext(pWidget));
	if ( pProxy == NULL ) {
		return XUI_ERROR_NOT_INITIALIZED;
	}
	tRect = xuiWidgetGetRect(pWidget);
	tRect.fX = 0.0f;
	tRect.fY = 0.0f;
	tRect = xuiInternalSnapRect(tRect);
	iBorder = pData->bActive ? tResolved.iActiveBorderColor : tResolved.iBorderColor;
	iTitle = pData->bActive ? tResolved.iTitleBarColor : tResolved.iInactiveTitleBarColor;
	iTitleText = pData->bActive ? tResolved.iTitleTextColor : tResolved.iInactiveTitleTextColor;
	iBottomFill = (pData->bCollapsed && pData->bShowTitleBar) ? iTitle : tResolved.iBackgroundColor;
	iRet = __xuiWindowDrawBottomSquareFill(pProxy, pDraw, tRect, tResolved.iBackgroundColor);
	if ( iRet != XUI_OK ) return iRet;
	if ( pData->bShowTitleBar ) {
		tTitle = pData->tTitleBarRect;
		tTitle = xuiInternalSnapRect(tTitle);
		iRet = __xuiWindowDrawRectFill(pProxy, pDraw, tTitle, iTitle);
		if ( iRet != XUI_OK ) return iRet;
		if ( tTitle.fH > 1.0f ) {
			iRet = __xuiWindowDrawRectFill(pProxy, pDraw, (xui_rect_t){tTitle.fX, tTitle.fY + tTitle.fH - 1.0f, tTitle.fW, 1.0f}, iBorder);
			if ( iRet != XUI_OK ) return iRet;
		}
		if ( pData->bTopMost ) {
			iRet = __xuiWindowDrawRectFill(pProxy, pDraw, (xui_rect_t){tTitle.fX, tTitle.fY, tTitle.fW, 2.0f}, tResolved.iTopMostColor);
			if ( iRet != XUI_OK ) return iRet;
		}
		tText = tTitle;
		tText.fX += 8.0f;
		tText.fW -= 16.0f;
		iTextRight = tTitle.fX + tTitle.fW - 8;
		if ( xuiWidgetGetVisible(pData->pCloseButton) ) iTextRight = pData->tCloseButtonRect.fX - 6;
		if ( xuiWidgetGetVisible(pData->pMaximizeButton) && pData->tMaximizeButtonRect.fX < iTextRight ) iTextRight = pData->tMaximizeButtonRect.fX - 6;
		if ( xuiWidgetGetVisible(pData->pCollapseButton) && pData->tCollapseButtonRect.fX < iTextRight ) iTextRight = pData->tCollapseButtonRect.fX - 6;
		tText.fW = iTextRight - tText.fX;
		if ( tText.fW < 0.0f ) tText.fW = 0.0f;
		if ( pData->bHasIcon && pData->pIconSurface != NULL && pProxy->drawSurface != NULL ) {
			tIcon = xuiInternalRectFromFloatNearest(tTitle.fX + 8.0f,
				tTitle.fY + (tTitle.fH - tResolved.fIconSize) * 0.5f,
				tResolved.fIconSize, tResolved.fIconSize);
			iRet = pProxy->drawSurface(pProxy, pDraw, pData->pIconSurface, pData->tIconSrc, xuiInternalSnapRect(tIcon), XUI_COLOR_RGBA(255, 255, 255, 255), 0);
			if ( iRet != XUI_OK ) return iRet;
			tText.fX = tIcon.fX + tIcon.fW + 8.0f;
			tText.fW = iTextRight - tText.fX;
			if ( tText.fW < 0.0f ) tText.fW = 0.0f;
		}
		if ( (tResolved.pFont != NULL) && (pProxy->drawText != NULL) && (pData->sTitle[0] != '\0') ) {
			iRet = pProxy->drawText(pProxy, pDraw, &(xui_text_item_t){.iSize=sizeof(xui_text_item_t), .pFont=tResolved.pFont, .sText=pData->sTitle, .iTextSize=-1, .iFlags=XUI_TEXT_SHAPE_DEFAULT | ((XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP) & XUI_TEXT_RTL ? XUI_TEXT_SHAPE_RTL : 0)}, xuiInternalSnapRect(tText), iTitleText, XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP);
			if ( iRet != XUI_OK ) return iRet;
		}
	}
	return __xuiWindowDrawBottomSquareStroke(pProxy, pDraw, tRect, tResolved.fBorderWidth, iBottomFill, iBorder);
}

static int __xuiWindowContentMeasure(xui_widget pWidget, xui_vec2_t tConstraint, xui_vec2_t* pSize, void* pUser)
{
	xui_window_data_t* pData;
	xui_window_resolved_t tResolved;
	float fChrome;

	(void)tConstraint;
	(void)pUser;
	if ( (pWidget == NULL) || (pSize == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pData = __xuiWindowGetData(pWidget);
	if ( pData == NULL ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	__xuiWindowResolve(pWidget, pData, &tResolved);
	fChrome = tResolved.fBorderWidth * 2.0f + (pData->bShowTitleBar ? tResolved.fTitleBarHeight : 0.0f);
	pSize->fX = __xuiWindowMax(0.0f,
		__xuiWindowMinWidth(pData, &tResolved) - tResolved.fBorderWidth * 2.0f);
	pSize->fY = pData->bCollapsed ? 0.0f : __xuiWindowMax(0.0f,
		__xuiWindowMinHeight(pData, &tResolved) - fChrome);
	return XUI_OK;
}

static int __xuiWindowPrepare(xui_widget pWidget, void* pUser)
{
	xui_window_data_t* pData = (xui_window_data_t*)pUser;
	xui_window_resolved_t tResolved;
	xui_thickness_t tPadding;
	xui_rect_t tRect;
	xui_rect_t tParent;
	int iRet;
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pData->bMaximized ) {
		tRect = xuiWidgetGetRect(pWidget);
		tParent = __xuiWindowParentRect(pWidget);
		if ( !__xuiWindowRectEqual(tRect, tParent) ) {
			iRet = __xuiWindowApplyRect(pWidget, pData, tParent);
			if ( iRet != XUI_OK ) return iRet;
		}
	}
	__xuiWindowResolve(pWidget, pData, &tResolved);
	tPadding.fLeft = xuiInternalSnapPixel(tResolved.fBorderWidth);
	tPadding.fTop = xuiInternalSnapPixel(tResolved.fBorderWidth + (pData->bShowTitleBar ? tResolved.fTitleBarHeight : 0.0f));
	/* Far edges round at size - border, including half-pixel ties. */
	tPadding.fRight = -xuiInternalSnapPixel(-tResolved.fBorderWidth);
	tPadding.fBottom = -xuiInternalSnapPixel(-tResolved.fBorderWidth);
	iRet = xuiWidgetSetPadding(pWidget, tPadding);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetVisible(pData->pClient, !pData->bCollapsed);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetVisible(pData->pCollapseButton,
		pData->bShowTitleBar && pData->bShowCollapse);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetVisible(pData->pMaximizeButton,
		pData->bShowTitleBar && pData->bShowMaximize);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetVisible(pData->pCloseButton,
		pData->bShowTitleBar && pData->bShowClose);
	return iRet;
}

static int __xuiWindowLayoutChildren(xui_widget pWidget, xui_rect_t tContentRect, void* pUser)
{
	xui_window_data_t* pData = (xui_window_data_t*)pUser;
	xui_window_resolved_t tResolved;
	xui_internal_window_frame_metrics_t tMetrics;
	xui_internal_window_frame_layout_t tLayout;
	xui_rect_t tFrame;
	int iTrailing;
	int iRet;

	if ( pData == NULL ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	__xuiWindowResolve(pWidget, pData, &tResolved);
	__xuiWindowFrameMetrics(pData, &tResolved, &tMetrics);
	tFrame = xuiWidgetGetRect(pWidget);
	tFrame.fX = 0;
	tFrame.fY = 0;
	xuiInternalWindowFrameLayout(tFrame, &tMetrics, &tLayout);
	pData->tTitleBarRect = tLayout.tTitleBarRect;
	pData->tClientRect = tLayout.tClientRect;
	if ( pData->bCollapsed ) pData->tClientRect.fH = 0.0f;
	iTrailing = 0;
	if ( pData->bShowTitleBar && pData->bShowClose ) {
		pData->tCloseButtonRect = xuiInternalWindowFrameTrailingButton(&tLayout, &tMetrics, iTrailing++);
	} else {
		memset(&pData->tCloseButtonRect, 0, sizeof(pData->tCloseButtonRect));
	}
	if ( pData->bShowTitleBar && pData->bShowMaximize ) {
		pData->tMaximizeButtonRect = xuiInternalWindowFrameTrailingButton(&tLayout, &tMetrics, iTrailing++);
	} else {
		memset(&pData->tMaximizeButtonRect, 0, sizeof(pData->tMaximizeButtonRect));
	}
	if ( pData->bShowTitleBar && pData->bShowCollapse ) {
		pData->tCollapseButtonRect = xuiInternalWindowFrameTrailingButton(&tLayout, &tMetrics, iTrailing);
	} else {
		memset(&pData->tCollapseButtonRect, 0, sizeof(pData->tCollapseButtonRect));
	}
	iRet = xuiLayoutArrangeChild(pWidget, pData->pClient, pData->tClientRect);
	if ( iRet == XUI_OK ) iRet = xuiLayoutArrangeChild(pWidget, pData->pCollapseButton, pData->tCollapseButtonRect);
	if ( iRet == XUI_OK ) iRet = xuiLayoutArrangeChild(pWidget, pData->pMaximizeButton, pData->tMaximizeButtonRect);
	if ( iRet == XUI_OK ) iRet = xuiLayoutArrangeChild(pWidget, pData->pCloseButton, pData->tCloseButtonRect);
	if ( iRet != XUI_OK ) return iRet;
	return XUI_OK;
}

static int __xuiWindowEvent(xui_widget pWidget, const xui_event_t* pEvent, void* pUser)
{
	xui_window_data_t* pData;
	xui_window_resolved_t tResolved;
	xui_context pContext;
	uint32_t iEdges;
	xui_rect_t tRect;
	int bLeftButton;
	int iPart;
	int iRet;

	(void)pUser;
	pData = __xuiWindowGetData(pWidget);
	if ( (pWidget == NULL) || (pEvent == NULL) || (pData == NULL) || !pData->bOpen ) {
		return XUI_OK;
	}
	pContext = xuiWidgetGetContext(pWidget);
	bLeftButton = (pEvent->iButton == 0) || (pEvent->iButton == XUI_POINTER_BUTTON_LEFT);
	__xuiWindowResolve(pWidget, pData, &tResolved);
	switch ( pEvent->iType ) {
	case XUI_EVENT_POINTER_DOUBLE_CLICK:
		if ( bLeftButton && pData->bResizable &&
		     xuiWidgetGetEffectiveEnabled(pWidget) &&
		     __xuiWindowCanStartMove(pWidget, pData, &tResolved, pEvent) ) {
			__xuiWindowCancelInteraction(pWidget, pData, 1);
			iRet = xuiWindowSetMaximized(pWidget, !pData->bMaximized);
			if ( iRet != XUI_OK ) return iRet;
			return XUI_EVENT_DISPATCH_STOP;
		}
		break;
	case XUI_EVENT_POINTER_DOWN:
		if ( bLeftButton && xuiWidgetGetEffectiveEnabled(pWidget) &&
		     __xuiWindowRectContains(xuiWidgetGetWorldRect(pWidget), pEvent->fX, pEvent->fY) ) {
			iRet = __xuiWindowActivate(pWidget, pData, 1);
			if ( iRet != XUI_OK ) return iRet;
			if ( !__xuiWindowDataAlive(pWidget, pData) ) return XUI_EVENT_DISPATCH_STOP;
			if ( pEvent->iPhase == XUI_EVENT_PHASE_CAPTURE ) {
				return XUI_OK;
			}
			if ( pData->iInteractionEdges != 0 ) return XUI_EVENT_DISPATCH_STOP;
			iEdges = __xuiWindowResizeEdgesAtData(pWidget, pData, &tResolved, pEvent->fX, pEvent->fY);
			if ( iEdges != 0u ) {
				pData->iInteractionEdges = (int)iEdges;
				iPart = XUI_WINDOW_PART_RESIZE;
			} else if ( __xuiWindowCanStartMove(pWidget, pData, &tResolved, pEvent) ) {
				pData->iInteractionEdges = -1;
				iPart = XUI_WINDOW_PART_TITLE_BAR;
			} else {
				if ( pEvent->pTarget == pWidget ) {
					return XUI_EVENT_DISPATCH_STOP;
				}
				return XUI_OK;
			}
			pData->fDragStartX = pEvent->fX;
			pData->fDragStartY = pEvent->fY;
			pData->tDragStartRect = xuiWidgetGetRect(pWidget);
			pData->iInteractionPointerId = pEvent->iPointerId;
			pData->iInteractionPointerType = pEvent->iPointerType;
			iRet = __xuiWindowSetActivePart(pWidget, pData, iPart);
			if ( iRet != XUI_OK ) {
				__xuiWindowCancelInteraction(pWidget, pData, 0);
				return iRet;
			}
			iRet = xuiSetPointerCaptureEx(pContext, pEvent->iPointerId,
				pEvent->iPointerType, pWidget);
			if ( iRet != XUI_OK ) {
				__xuiWindowCancelInteraction(pWidget, pData, 0);
				return iRet;
			}
			iRet = __xuiWindowUpdateInteractionAdorner(pWidget, pData, pData->tDragStartRect);
			if ( iRet != XUI_OK ) {
				__xuiWindowCancelInteraction(pWidget, pData, 1);
				return iRet;
			}
			return XUI_EVENT_DISPATCH_STOP;
		}
		break;
	case XUI_EVENT_POINTER_MOVE:
		if ( pData->iInteractionEdges != 0 && !__xuiWindowInteractionPointerMatches(pData, pEvent) ) {
			return XUI_EVENT_DISPATCH_STOP;
		}
		if ( __xuiWindowInteractionPointerMatches(pData, pEvent) &&
		     xuiGetPointerCaptureEx(pContext, pEvent->iPointerId, pEvent->iPointerType) == pWidget ) {
			tRect = __xuiWindowBuildInteractionRect(pWidget, pData, pEvent->fX, pEvent->fY);
			iRet = __xuiWindowUpdateInteractionAdorner(pWidget, pData, tRect);
			if ( iRet != XUI_OK ) return iRet;
			return XUI_EVENT_DISPATCH_STOP;
		}
		iEdges = __xuiWindowResizeEdgesAtData(pWidget, pData, &tResolved, pEvent->fX, pEvent->fY);
		if ( iEdges != 0u ) {
			(void)__xuiWindowSetHoverPart(pWidget, pData, XUI_WINDOW_PART_RESIZE);
		} else if ( __xuiWindowCanStartMove(pWidget, pData, &tResolved, pEvent) ) {
			(void)__xuiWindowSetHoverPart(pWidget, pData, XUI_WINDOW_PART_TITLE_BAR);
		} else if ( pData->iHoverPart == XUI_WINDOW_PART_RESIZE || pData->iHoverPart == XUI_WINDOW_PART_TITLE_BAR ) {
			(void)__xuiWindowSetHoverPart(pWidget, pData, XUI_WINDOW_PART_NONE);
		}
		break;
	case XUI_EVENT_POINTER_UP:
		if ( bLeftButton && pData->iInteractionEdges != 0 &&
		     !__xuiWindowInteractionPointerMatches(pData, pEvent) ) {
			return XUI_EVENT_DISPATCH_STOP;
		}
		if ( bLeftButton && __xuiWindowInteractionPointerMatches(pData, pEvent) ) {
			xui_rect_t tOldRect = xuiWidgetGetRect(pWidget);
			int bMoving = pData->iInteractionEdges == -1;
			tRect = __xuiWindowBuildInteractionRect(pWidget, pData, pEvent->fX, pEvent->fY);
			iRet = bMoving ? xuiWidgetSetRect(pWidget, tRect) :
				__xuiWindowApplyRect(pWidget, pData, tRect);
			if ( iRet == XUI_OK && !__xuiWindowRectEqual(tOldRect, xuiWidgetGetRect(pWidget)) ) {
				pData->iChangeCount++;
			}
			__xuiWindowCancelInteraction(pWidget, pData, 1);
			if ( iRet != XUI_OK ) return iRet;
			return XUI_EVENT_DISPATCH_STOP;
		}
		break;
	case XUI_EVENT_POINTER_LEAVE:
		if ( pData->iHoverPart == XUI_WINDOW_PART_RESIZE || pData->iHoverPart == XUI_WINDOW_PART_TITLE_BAR ) {
			(void)__xuiWindowSetHoverPart(pWidget, pData, XUI_WINDOW_PART_NONE);
		}
		break;
	case XUI_EVENT_POINTER_CAPTURE_LOST:
		if ( __xuiWindowInteractionPointerMatches(pData, pEvent) ) {
			__xuiWindowCancelInteraction(pWidget, pData, 0);
		}
		return XUI_EVENT_DISPATCH_STOP;
	case XUI_EVENT_CONTEXT_MENU: {
		xui_rect_t tWorld;
		xui_rect_t tTitle;
		float fX;
		float fY;
		if ( pData->onContext == NULL || pEvent->pTarget != pWidget || !pData->bShowTitleBar ) break;
		tWorld = xuiWidgetGetWorldRect(pWidget);
		tTitle = pData->tTitleBarRect;
		tTitle.fX += tWorld.fX;
		tTitle.fY += tWorld.fY;
		if ( pEvent->iKey != XUI_KEY_CONTEXT_MENU &&
		     !__xuiWindowRectContains(tTitle, pEvent->fX, pEvent->fY) ) break;
		xuiInternalContextMenuPoint(pEvent, tTitle, &fX, &fY);
		return pData->onContext(pWidget, XUI_WINDOW_PART_TITLE_BAR,
			fX, fY, pData->pContextUser);
	}
	case XUI_EVENT_KEY_DOWN:
		if ( pData->bActive && pData->bShowClose &&
		     xuiWidgetGetEffectiveEnabled(pWidget) &&
		     xuiWidgetGetEffectiveVisible(pWidget) &&
		     pEvent->iKey == XUI_KEY_F4 &&
		     (pEvent->iModifiers & XUI_MOD_ALT) != 0u ) {
			iRet = __xuiWindowRequestClose(pWidget, pData);
			if ( iRet != XUI_OK ) return iRet;
			return XUI_EVENT_DISPATCH_STOP;
		}
		if ( (pEvent->iKey == XUI_KEY_ESCAPE) && (pData->iInteractionEdges != 0) ) {
			__xuiWindowCancelInteraction(pWidget, pData, 1);
			return XUI_EVENT_DISPATCH_STOP;
		}
		break;
	case XUI_EVENT_FOCUS:
		if ( xuiGetFocusWidget(pContext) == pWidget ) {
			(void)__xuiWindowActivate(pWidget, pData, 0);
		}
		return XUI_EVENT_DISPATCH_STOP;
	case XUI_EVENT_BLUR:
		(void)xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
		return XUI_EVENT_DISPATCH_STOP;
	case XUI_EVENT_ENABLED_CHANGED:
	case XUI_EVENT_VISIBLE_CHANGED:
		if ( pEvent->pTarget != pWidget ) return XUI_OK;
		if ( !xuiWidgetGetEffectiveEnabled(pWidget) ||
		     !xuiWidgetGetEffectiveVisible(pWidget) ) {
			__xuiWindowCancelInteraction(pWidget, pData, 1);
			__xuiWindowCancelButtonPress(pWidget, pData, 1);
			pData->bActive = 0;
		}
		(void)xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
		return XUI_EVENT_DISPATCH_STOP;
	default:
		break;
	}
	return XUI_OK;
}

static void __xuiWindowDefaultLayout(xui_layout_t* pLayout)
{
	memset(pLayout, 0, sizeof(*pLayout));
	pLayout->iLayoutType = XUI_LAYOUT_OVERLAY;
	pLayout->iWidthMode = XUI_SIZE_FIXED;
	pLayout->iHeightMode = XUI_SIZE_FIXED;
	pLayout->iFlowMode = XUI_FLOW_ABSOLUTE;
	pLayout->iDock = 0;
	pLayout->iOverflow = XUI_OVERFLOW_VISIBLE;
	pLayout->iAlignX = XUI_ALIGN_START;
	pLayout->iAlignY = XUI_ALIGN_START;
	pLayout->iTableRowSpan = 1;
	pLayout->iTableColumnSpan = 1;
	pLayout->iGridColumnCount = 1;
	pLayout->iLayer = XUI_LAYER_FLOATING;
	pLayout->iZIndex = XUI_WINDOW_Z_NORMAL;
	pLayout->fPreferredWidth = 300.0f;
	pLayout->fPreferredHeight = 200.0f;
	pLayout->fMinWidth = 120.0f;
	pLayout->fMinHeight = 64.0f;
	pLayout->fMaxWidth = XUI_LAYOUT_UNBOUNDED;
	pLayout->fMaxHeight = XUI_LAYOUT_UNBOUNDED;
	pLayout->fShrink = 1.0f;
}

static void __xuiWindowDefaultCachePolicy(xui_cache_policy_t* pPolicy)
{
	memset(pPolicy, 0, sizeof(*pPolicy));
	pPolicy->iSize = sizeof(*pPolicy);
	pPolicy->iPolicy = XUI_CACHE_POLICY_SELF;
	pPolicy->iFlags = XUI_CACHE_CLEAR_ON_UPDATE;
	pPolicy->iClearColor = XUI_COLOR_RGBA(0, 0, 0, 0);
}

static xui_thickness_t __xuiWindowThickness(float fLeft, float fTop, float fRight, float fBottom)
{
	xui_thickness_t tValue;

	tValue.fLeft = fLeft;
	tValue.fTop = fTop;
	tValue.fRight = fRight;
	tValue.fBottom = fBottom;
	return tValue;
}

static int __xuiWindowConfigureButton(xui_widget pWindow, xui_widget pButton,
	const xui_cache_policy_t* pPolicy)
{
	int iRet;

	if ( pWindow == NULL || pButton == NULL || pPolicy == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	iRet = xuiWidgetSetLayoutType(pButton, XUI_LAYOUT_MANUAL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetSizeMode(pButton, XUI_SIZE_FIXED, XUI_SIZE_FIXED);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetFocusable(pButton, 0);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetTabStop(pButton, 0);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetCachePolicy(pButton, pPolicy);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetCacheRenderCallback(pButton, __xuiWindowButtonRender, pWindow);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pButton, XUI_EVENT_POINTER_ENTER, __xuiWindowButtonEvent, pWindow);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pButton, XUI_EVENT_POINTER_LEAVE, __xuiWindowButtonEvent, pWindow);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pButton, XUI_EVENT_POINTER_MOVE, __xuiWindowButtonEvent, pWindow);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pButton, XUI_EVENT_POINTER_DOWN, __xuiWindowButtonEvent, pWindow);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pButton, XUI_EVENT_POINTER_UP, __xuiWindowButtonEvent, pWindow);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pButton, XUI_EVENT_POINTER_CAPTURE_LOST, __xuiWindowButtonEvent, pWindow);
	return iRet;
}

static int __xuiWindowInitChildren(xui_widget pWidget, xui_window_data_t* pData)
{
	xui_cache_policy_t tPolicy;
	xui_widget pClient;
	xui_widget pCollapse;
	xui_widget pMaximize;
	xui_widget pClose;
	int iRet;

	pClient = NULL;
	pCollapse = NULL;
	pMaximize = NULL;
	pClose = NULL;
	iRet = xuiWidgetCreate(xuiWidgetGetContext(pWidget), &pClient);
	if ( iRet == XUI_OK ) iRet = xuiWidgetCreate(xuiWidgetGetContext(pWidget), &pCollapse);
	if ( iRet == XUI_OK ) iRet = xuiWidgetCreate(xuiWidgetGetContext(pWidget), &pMaximize);
	if ( iRet == XUI_OK ) iRet = xuiWidgetCreate(xuiWidgetGetContext(pWidget), &pClose);
	if ( iRet != XUI_OK ) {
		xuiWidgetDestroy(pClient);
		xuiWidgetDestroy(pCollapse);
		xuiWidgetDestroy(pMaximize);
		xuiWidgetDestroy(pClose);
		return iRet;
	}
	__xuiWindowDefaultCachePolicy(&tPolicy);
	pData->pClient = pClient;
	pData->pCollapseButton = pCollapse;
	pData->pMaximizeButton = pMaximize;
	pData->pCloseButton = pClose;

	iRet = xuiWidgetSetLayoutType(pClient, XUI_LAYOUT_COLUMN);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetSizeMode(pClient, XUI_SIZE_FILL, XUI_SIZE_FILL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetFlowMode(pClient, XUI_FLOW_BLOCK);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetPadding(pClient, __xuiWindowThickness(10.0f, 10.0f, 10.0f, 10.0f));
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetGap(pClient, 6.0f);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetOverflow(pClient, XUI_OVERFLOW_CLIP);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetCachePolicy(pClient, &tPolicy);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetCacheRenderCallback(pClient, __xuiWindowClientRender, pWidget);
	if ( iRet == XUI_OK ) iRet = __xuiWindowConfigureButton(pWidget, pCollapse, &tPolicy);
	if ( iRet == XUI_OK ) iRet = __xuiWindowConfigureButton(pWidget, pMaximize, &tPolicy);
	if ( iRet == XUI_OK ) iRet = __xuiWindowConfigureButton(pWidget, pClose, &tPolicy);
	if ( iRet == XUI_OK ) iRet = xuiWidgetAddChild(pWidget, pClient);
	if ( iRet == XUI_OK ) iRet = xuiWidgetAddChild(pWidget, pCollapse);
	if ( iRet == XUI_OK ) iRet = xuiWidgetAddChild(pWidget, pMaximize);
	if ( iRet == XUI_OK ) iRet = xuiWidgetAddChild(pWidget, pClose);
	if ( iRet != XUI_OK ) {
		xuiWidgetDestroy(pClient);
		xuiWidgetDestroy(pCollapse);
		xuiWidgetDestroy(pMaximize);
		xuiWidgetDestroy(pClose);
		pData->pClient = NULL;
		pData->pCollapseButton = NULL;
		pData->pMaximizeButton = NULL;
		pData->pCloseButton = NULL;
		return iRet;
	}
	return XUI_OK;
}

static void __xuiWindowDefaults(xui_window_data_t* pData)
{
	memset(pData, 0, sizeof(*pData));
	pData->bOpen = 1;
	pData->bShowTitleBar = 1;
	pData->bMovable = 1;
	pData->bResizable = 1;
	pData->iResizeEdges = __xuiWindowDefaultResizeEdges(pData);
	pData->bShowCollapse = 1;
	pData->bShowMaximize = 1;
	pData->bShowClose = 1;
	pData->iInteractionPointerType = XUI_POINTER_TYPE_MOUSE;
	pData->iButtonPointerType = XUI_POINTER_TYPE_MOUSE;
	pData->fExpandedHeight = 200.0f;
	pData->fTitleBarHeight = 30.0f;
	pData->fBorderWidth = 1.0f;
	pData->fResizeGrip = 6.0f;
	pData->fButtonSize = 20.0f;
	pData->fIconSize = 16.0f;
	pData->fMinWidth = 160.0f;
	pData->fMinHeight = 120.0f;
	pData->iBackgroundColor = XUI_COLOR_RGBA(248, 252, 255, 255);
	pData->iClientColor = XUI_COLOR_RGBA(248, 252, 255, 255);
	pData->iTitleBarColor = XUI_COLOR_RGBA(226, 240, 250, 255);
	pData->iInactiveTitleBarColor = XUI_COLOR_RGBA(236, 245, 251, 255);
	pData->iTitleTextColor = XUI_COLOR_RGBA(34, 59, 84, 255);
	pData->iInactiveTitleTextColor = XUI_COLOR_RGBA(102, 123, 142, 255);
	pData->iBorderColor = XUI_COLOR_RGBA(151, 178, 205, 255);
	pData->iActiveBorderColor = XUI_COLOR_RGBA(47, 128, 237, 255);
	pData->iButtonColor = XUI_COLOR_RGBA(0, 0, 0, 0);
	pData->iButtonHoverColor = XUI_COLOR_RGBA(206, 225, 243, 255);
	pData->iButtonActiveColor = XUI_COLOR_RGBA(184, 209, 235, 255);
	pData->iCloseHoverColor = XUI_COLOR_RGBA(255, 226, 226, 255);
	pData->iCloseActiveColor = XUI_COLOR_RGBA(244, 205, 205, 255);
}

static void __xuiWindowApplyDesc(xui_window_data_t* pData, const xui_window_desc_t* pDesc)
{
	if ( (pData == NULL) || (pDesc == NULL) ) {
		return;
	}
	if ( pDesc->sTitle != NULL ) {
		__xuiWindowCopyTitle(pData->sTitle, sizeof(pData->sTitle), pDesc->sTitle);
	}
	pData->pFont = pDesc->pFont;
	pData->pIconSurface = pDesc->pIconSurface;
	pData->tIconSrc = pDesc->tIconSrc;
	pData->bHasIcon = (pDesc->bHasIcon || (pDesc->pIconSurface != NULL)) ? 1 : 0;
	pData->bOpen = pDesc->bClosed ? 0 : 1;
	pData->bTopMost = pDesc->bTopMost ? 1 : 0;
	pData->bShowTitleBar = pDesc->bNoTitleBar ? 0 : 1;
	pData->bMovable = pDesc->bNotMovable ? 0 : 1;
	pData->bDragAnywhere = pDesc->bDragAnywhere ? 1 : 0;
	pData->bResizable = pDesc->bNotResizable ? 0 : 1;
	pData->bShowCollapse = pDesc->bHideCollapse ? 0 : 1;
	pData->bShowMaximize = pDesc->bHideMaximize ? 0 : 1;
	pData->bShowClose = pDesc->bHideClose ? 0 : 1;
	pData->bCollapsed = pDesc->bCollapsed ? 1 : 0;
	pData->bMaximized = pDesc->bMaximized ? 1 : 0;
	if ( pDesc->iResizeEdges != 0u ) {
		pData->bResizeEdgesExplicit = 1;
		pData->iResizeEdges = pDesc->iResizeEdges & XUI_WINDOW_EDGE_ALL;
	} else {
		__xuiWindowApplyDefaultResizeEdges(pData);
	}
	if ( pDesc->fTitleBarHeight > 0.0f ) pData->fTitleBarHeight = pDesc->fTitleBarHeight;
	if ( pDesc->fBorderWidth > 0.0f ) pData->fBorderWidth = pDesc->fBorderWidth;
	if ( pDesc->fResizeGrip > 0.0f ) pData->fResizeGrip = pDesc->fResizeGrip;
	if ( pDesc->fButtonSize > 0.0f ) pData->fButtonSize = pDesc->fButtonSize;
	if ( pDesc->fIconSize > 0.0f ) pData->fIconSize = pDesc->fIconSize;
	if ( pDesc->fMinWidth > 0.0f ) pData->fMinWidth = pDesc->fMinWidth;
	if ( pDesc->fMinHeight > 0.0f ) pData->fMinHeight = pDesc->fMinHeight;
	if ( __xuiWindowAlpha(pDesc->iBackgroundColor) != 0u ) pData->iBackgroundColor = pDesc->iBackgroundColor;
	if ( __xuiWindowAlpha(pDesc->iClientColor) != 0u ) pData->iClientColor = pDesc->iClientColor;
	if ( __xuiWindowAlpha(pDesc->iTitleBarColor) != 0u ) pData->iTitleBarColor = pDesc->iTitleBarColor;
	if ( __xuiWindowAlpha(pDesc->iInactiveTitleBarColor) != 0u ) pData->iInactiveTitleBarColor = pDesc->iInactiveTitleBarColor;
	if ( __xuiWindowAlpha(pDesc->iTitleTextColor) != 0u ) pData->iTitleTextColor = pDesc->iTitleTextColor;
	if ( __xuiWindowAlpha(pDesc->iInactiveTitleTextColor) != 0u ) pData->iInactiveTitleTextColor = pDesc->iInactiveTitleTextColor;
	if ( __xuiWindowAlpha(pDesc->iBorderColor) != 0u ) pData->iBorderColor = pDesc->iBorderColor;
	if ( __xuiWindowAlpha(pDesc->iActiveBorderColor) != 0u ) pData->iActiveBorderColor = pDesc->iActiveBorderColor;
	if ( pDesc->iButtonColor != 0u ) pData->iButtonColor = pDesc->iButtonColor;
	if ( pDesc->iButtonHoverColor != 0u ) pData->iButtonHoverColor = pDesc->iButtonHoverColor;
	if ( pDesc->iButtonActiveColor != 0u ) pData->iButtonActiveColor = pDesc->iButtonActiveColor;
	if ( pDesc->iCloseHoverColor != 0u ) pData->iCloseHoverColor = pDesc->iCloseHoverColor;
	if ( pDesc->iCloseActiveColor != 0u ) pData->iCloseActiveColor = pDesc->iCloseActiveColor;
}

static int __xuiWindowInit(xui_widget pWidget, void* pTypeData, const void* pCreateData, void* pUser)
{
	xui_window_data_t* pData;
	const xui_window_desc_t* pDesc;
	xui_cache_policy_t tPolicy;
	xui_rect_t tInitial;
	int bOpen;
	int iRet;

	(void)pUser;
	if ( (pWidget == NULL) || (pTypeData == NULL) || !__xuiWindowDescValid((const xui_window_desc_t*)pCreateData) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pData = (xui_window_data_t*)pTypeData;
	pDesc = (const xui_window_desc_t*)pCreateData;
	__xuiWindowDefaults(pData);
	__xuiWindowApplyDesc(pData, pDesc);
	bOpen = pData->bOpen;
	pData->bOpen = 0;
	iRet = __xuiWindowInitChildren(pWidget, pData);
	if ( iRet != XUI_OK ) return iRet;
	__xuiWindowDefaultCachePolicy(&tPolicy);
	iRet = xuiWidgetSetFocusable(pWidget, 1);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetTabStop(pWidget, 0);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetFocusScope(pWidget, 1);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetLayoutType(pWidget, XUI_LAYOUT_MANUAL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetFlowMode(pWidget, XUI_FLOW_ABSOLUTE);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetOverflow(pWidget, XUI_OVERFLOW_VISIBLE);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetSizeMode(pWidget, XUI_SIZE_FIXED, XUI_SIZE_FIXED);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetPreferredSize(pWidget, (xui_vec2_t){300.0f, 200.0f});
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetMinSize(pWidget, (xui_vec2_t){pData->fMinWidth, pData->fMinHeight});
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetCachePolicy(pWidget, &tPolicy);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetCacheRenderCallback(pWidget, __xuiWindowCacheRender, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventCallback(pWidget, __xuiWindowEvent, NULL);
	if ( iRet == XUI_OK ) iRet = __xuiWindowApplyLayerAndZ(pWidget, pData);
	if ( iRet != XUI_OK ) return iRet;
	tInitial = xuiWidgetGetRect(pWidget);
	if ( tInitial.fW <= 0.0f ) tInitial.fW = 300.0f;
	if ( tInitial.fH <= 0.0f ) tInitial.fH = pData->bCollapsed ? 32.0f : 200.0f;
	iRet = xuiWidgetSetRect(pWidget, tInitial);
	if ( iRet != XUI_OK ) return iRet;
	if ( bOpen ) {
		iRet = __xuiWindowSetOpenInternal(pWidget, pData, 1, 1);
		if ( iRet != XUI_OK ) return iRet;
	} else {
		iRet = xuiWidgetSetVisible(pWidget, 0);
		if ( iRet != XUI_OK ) return iRet;
	}
	return XUI_OK;
}

static void __xuiWindowDestroy(xui_widget pWidget, void* pTypeData, void* pUser)
{
	xui_window_data_t* pData;

	(void)pWidget;
	(void)pUser;
	pData = (xui_window_data_t*)pTypeData;
	if ( pData != NULL ) {
		memset(pData, 0, sizeof(*pData));
	}
}

static void __xuiWindowRegisterStyleProperty(xui_context pContext, xui_widget_type pType, const char* sName, int iValueType, uint32_t iDirtyFlags, uint32_t iFlags)
{
	xui_style_property_info_t tInfo;

	memset(&tInfo, 0, sizeof(tInfo));
	tInfo.iSize = sizeof(tInfo);
	tInfo.sName = sName;
	tInfo.iValueType = iValueType;
	tInfo.iDirtyFlags = iDirtyFlags;
	tInfo.iFlags = iFlags;
	tInfo.pWidgetType = pType;
	(void)xuiStyleRegisterProperty(pContext, &tInfo, NULL);
}

static int __xuiWindowPreparePaint(xui_widget pWidget)
{
	xui_window_data_t* pData = __xuiWindowGetData(pWidget);
	xui_context pContext = xuiWidgetGetContext(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pContext->pDragAdornerOwner == pWidget && pContext->iDragAdornerPrimitiveCount == 1 ) {
		uint32_t iColor = pData->iActiveBorderColor;
		(void)__xuiWindowStyleColor(pWidget, "window.border.active_color", &iColor);
		(void)xuiInternalDragAdornerSetColor(pContext, pWidget, 0, iColor);
	}
	if ( pData->iChromeStyleVersion == pWidget->iStyleVersion ) return XUI_OK;
	pData->iChromeStyleVersion = pWidget->iStyleVersion;
	(void)xuiWidgetInvalidate(pData->pClient, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	(void)xuiWidgetInvalidate(pData->pCollapseButton, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	(void)xuiWidgetInvalidate(pData->pMaximizeButton, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	return xuiWidgetInvalidate(pData->pCloseButton, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
}

static void __xuiWindowRegisterStyleProperties(xui_context pContext, xui_widget_type pType)
{
	uint32_t iPaintDirty;
	uint32_t iLayoutDirty;

	pType->onPreparePaint = __xuiWindowPreparePaint;
	iPaintDirty = XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER;
	iLayoutDirty = XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER;
	__xuiWindowRegisterStyleProperty(pContext, pType, "font.name", XUI_STYLE_VALUE_STRING, iLayoutDirty, XUI_STYLE_PROPERTY_INHERITED);
	__xuiWindowRegisterStyleProperty(pContext, pType, "window.background.color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiWindowRegisterStyleProperty(pContext, pType, "window.client.color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiWindowRegisterStyleProperty(pContext, pType, "window.titlebar.color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiWindowRegisterStyleProperty(pContext, pType, "window.titlebar.inactive_color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiWindowRegisterStyleProperty(pContext, pType, "window.title.text_color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiWindowRegisterStyleProperty(pContext, pType, "window.title.inactive_text_color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiWindowRegisterStyleProperty(pContext, pType, "window.border.color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiWindowRegisterStyleProperty(pContext, pType, "window.border.active_color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiWindowRegisterStyleProperty(pContext, pType, "window.button.color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiWindowRegisterStyleProperty(pContext, pType, "window.button.hover_color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiWindowRegisterStyleProperty(pContext, pType, "window.button.active_color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiWindowRegisterStyleProperty(pContext, pType, "window.close.hover_color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiWindowRegisterStyleProperty(pContext, pType, "window.close.active_color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiWindowRegisterStyleProperty(pContext, pType, "window.close.icon_color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiWindowRegisterStyleProperty(pContext, pType, "window.titlebar.topmost_color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiWindowRegisterStyleProperty(pContext, pType, "window.titlebar.height", XUI_STYLE_VALUE_FLOAT, iLayoutDirty, 0);
	__xuiWindowRegisterStyleProperty(pContext, pType, "window.border.width", XUI_STYLE_VALUE_FLOAT, iLayoutDirty, 0);
	__xuiWindowRegisterStyleProperty(pContext, pType, "window.resize_grip", XUI_STYLE_VALUE_FLOAT, iLayoutDirty, 0);
	__xuiWindowRegisterStyleProperty(pContext, pType, "window.button.size", XUI_STYLE_VALUE_FLOAT, iLayoutDirty, 0);
	__xuiWindowRegisterStyleProperty(pContext, pType, "window.icon.size", XUI_STYLE_VALUE_FLOAT, iLayoutDirty, 0);
	__xuiWindowRegisterStyleProperty(pContext, pType, "window.min_width", XUI_STYLE_VALUE_FLOAT, iLayoutDirty, 0);
	__xuiWindowRegisterStyleProperty(pContext, pType, "window.min_height", XUI_STYLE_VALUE_FLOAT, iLayoutDirty, 0);
}

static int __xuiWindowAccessibleNode(xui_widget pWidget, xui_accessible_node_t* pNode)
{
	xui_window_data_t* pData = __xuiWindowGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	pNode->iRole = XUI_ACCESSIBLE_ROLE_WINDOW;
	pNode->sName = pData->sTitle;
	pNode->iState |= pData->bCollapsed ? XUI_ACCESSIBLE_STATE_COLLAPSED : XUI_ACCESSIBLE_STATE_EXPANDED;
	if ( pData->bShowClose ) pNode->iActions |= XUI_ACCESSIBLE_ACTION_MASK(XUI_ACCESSIBLE_ACTION_CLOSE);
	return XUI_OK;
}

static int __xuiWindowAccessibleAction(xui_widget pWidget, int iAction, const void* pPayload)
{
	xui_window_data_t* pData = __xuiWindowGetData(pWidget);
	(void)pPayload;
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( iAction != XUI_ACCESSIBLE_ACTION_CLOSE || !pData->bShowClose ) return XUI_ERROR_UNSUPPORTED;
	return __xuiWindowRequestClose(pWidget, pData);
}

static const xui_internal_accessibility_adapter_t g_xuiWindowAccessible = {
	.getNode = __xuiWindowAccessibleNode, .performAction = __xuiWindowAccessibleAction
};

XUI_API xui_widget_type xuiWindowGetType(xui_context pContext)
{
	xui_widget_type_desc_t tDesc;
	xui_widget_type pType;
	int iRet;

	if ( !xuiInternalContextIsValid(pContext) ) {
		return NULL;
	}
	pType = xuiWidgetFindType(pContext, "window");
	if ( pType != NULL ) {
		__xuiWindowRegisterStyleProperties(pContext, pType);
		return pType;
	}
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.sName = "window";
	tDesc.pParent = xuiWidgetGetBaseType();
	tDesc.iFlags = XUI_WIDGET_TYPE_DEFAULT_LAYOUT | XUI_WIDGET_TYPE_DEFAULT_CACHE_POLICY;
	tDesc.iTypeDataSize = sizeof(xui_window_data_t);
	tDesc.onInit = __xuiWindowInit;
	tDesc.onDestroy = __xuiWindowDestroy;
	tDesc.onContentMeasure = __xuiWindowContentMeasure;
	tDesc.onLayoutPrepare = __xuiWindowPrepare;
	tDesc.onLayoutChildren = __xuiWindowLayoutChildren;
	tDesc.onCacheRender = __xuiWindowCacheRender;
	tDesc.onQueryCursor = __xuiWindowQueryCursor;
	__xuiWindowDefaultLayout(&tDesc.tLayout);
	__xuiWindowDefaultCachePolicy(&tDesc.tCachePolicy);
	iRet = xuiWidgetRegisterType(pContext, &pType, &tDesc);
	if ( iRet != XUI_OK ) {
		return NULL;
	}
	__xuiWindowRegisterStyleProperties(pContext, pType);
	pType->pAccessibleAdapter = &g_xuiWindowAccessible;
	return pType;
}

XUI_API int xuiWindowCreate(xui_context pContext, xui_widget* ppWidget, const xui_window_desc_t* pDesc)
{
	xui_widget_type pType;

	if ( ppWidget == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	*ppWidget = NULL;
	if ( !xuiInternalContextIsValid(pContext) || !__xuiWindowDescValid(pDesc) ) return XUI_ERROR_INVALID_ARGUMENT;
	pType = xuiWindowGetType(pContext);
	if ( pType == NULL ) {
		return XUI_ERROR_NOT_INITIALIZED;
	}
	return xuiWidgetCreateTyped(pContext, pType, ppWidget, pDesc);
}

XUI_API int xuiWindowSetClose(xui_widget pWidget, xui_window_close_proc onClose, void* pUser)
{
	xui_window_data_t* pData = __xuiWindowGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	pData->onClose = onClose;
	pData->pCloseUser = pUser;
	return XUI_OK;
}

XUI_API int xuiWindowSetContextMenu(xui_widget pWidget, xui_window_context_proc onContext, void* pUser)
{
	xui_window_data_t* pData = __xuiWindowGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	pData->onContext = onContext;
	pData->pContextUser = pUser;
	return XUI_OK;
}

XUI_API xui_widget xuiWindowGetClientWidget(xui_widget pWidget)
{
	xui_window_data_t* pData = __xuiWindowGetData(pWidget);
	return (pData != NULL) ? pData->pClient : NULL;
}

XUI_API xui_widget xuiWindowGetCollapseButtonWidget(xui_widget pWidget)
{
	xui_window_data_t* pData = __xuiWindowGetData(pWidget);
	return (pData != NULL) ? pData->pCollapseButton : NULL;
}

XUI_API xui_widget xuiWindowGetMaximizeButtonWidget(xui_widget pWidget)
{
	xui_window_data_t* pData = __xuiWindowGetData(pWidget);
	return (pData != NULL) ? pData->pMaximizeButton : NULL;
}

XUI_API xui_widget xuiWindowGetCloseButtonWidget(xui_widget pWidget)
{
	xui_window_data_t* pData = __xuiWindowGetData(pWidget);
	return (pData != NULL) ? pData->pCloseButton : NULL;
}

XUI_API int xuiWindowAddChild(xui_widget pWidget, xui_widget pChild)
{
	xui_widget pClient;
	int iLayer;
	int iZIndex;
	int iRet;

	pClient = xuiWindowGetClientWidget(pWidget);
	if ( (pClient == NULL) || (pChild == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	iRet = xuiWidgetAddChild(pClient, pChild);
	if ( iRet != XUI_OK ) {
		return iRet;
	}
	iRet = xuiWidgetGetLayer(pWidget, &iLayer, &iZIndex);
	if ( iRet != XUI_OK ) {
		return iRet;
	}
	return xuiInternalWidgetSetTreeLayer(pChild, iLayer, iZIndex);
}

XUI_API int xuiWindowInsertBefore(xui_widget pWidget, xui_widget pChild, xui_widget pBefore)
{
	xui_widget pClient;
	int iLayer;
	int iZIndex;
	int iRet;

	pClient = xuiWindowGetClientWidget(pWidget);
	if ( (pClient == NULL) || (pChild == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	iRet = xuiWidgetInsertBefore(pClient, pChild, pBefore);
	if ( iRet != XUI_OK ) {
		return iRet;
	}
	iRet = xuiWidgetGetLayer(pWidget, &iLayer, &iZIndex);
	if ( iRet != XUI_OK ) {
		return iRet;
	}
	return xuiInternalWidgetSetTreeLayer(pChild, iLayer, iZIndex);
}

XUI_API int xuiWindowSetTitle(xui_widget pWidget, const char* sTitle)
{
	xui_window_data_t* pData = __xuiWindowGetData(pWidget);
	char sCopy[XUI_WINDOW_TITLE_CAPACITY];
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	__xuiWindowCopyTitle(sCopy, sizeof(sCopy), sTitle);
	if ( strcmp(pData->sTitle, sCopy) == 0 ) return XUI_OK;
	memcpy(pData->sTitle, sCopy, sizeof(pData->sTitle));
	xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_NODE_CHANGED);
	return xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
}

XUI_API const char* xuiWindowGetTitle(xui_widget pWidget)
{
	xui_window_data_t* pData = __xuiWindowGetData(pWidget);
	return (pData != NULL) ? pData->sTitle : NULL;
}

XUI_API int xuiWindowSetFont(xui_widget pWidget, xui_font pFont)
{
	xui_window_data_t* pData = __xuiWindowGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	pFont = (pFont != NULL) ? pFont : xuiGetDefaultFont(xuiWidgetGetContext(pWidget));
	if ( pData->pFont == pFont ) return XUI_OK;
	pData->pFont = pFont;
	return xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
}

XUI_API xui_font xuiWindowGetFont(xui_widget pWidget)
{
	xui_window_data_t* pData = __xuiWindowGetData(pWidget);
	return (pData != NULL) ? pData->pFont : NULL;
}

XUI_API int xuiWindowSetIcon(xui_widget pWidget, xui_surface pSurface, xui_rect_t tSrc)
{
	xui_window_data_t* pData = __xuiWindowGetData(pWidget);
	if ( pData == NULL ||
	     tSrc.fX < -(int)XUI_LAYOUT_UNBOUNDED || tSrc.fX > (int)XUI_LAYOUT_UNBOUNDED ||
	     tSrc.fY < -(int)XUI_LAYOUT_UNBOUNDED || tSrc.fY > (int)XUI_LAYOUT_UNBOUNDED ||
	     tSrc.fW < 0 || tSrc.fW > (int)XUI_LAYOUT_UNBOUNDED ||
	     tSrc.fH < 0 || tSrc.fH > (int)XUI_LAYOUT_UNBOUNDED ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pData->pIconSurface == pSurface && memcmp(&pData->tIconSrc, &tSrc, sizeof(tSrc)) == 0 ) return XUI_OK;
	pData->pIconSurface = pSurface;
	pData->tIconSrc = tSrc;
	pData->bHasIcon = (pSurface != NULL) ? 1 : 0;
	return xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
}

XUI_API xui_surface xuiWindowGetIconSurface(xui_widget pWidget)
{
	xui_window_data_t* pData = __xuiWindowGetData(pWidget);
	return (pData != NULL) ? pData->pIconSurface : NULL;
}

XUI_API xui_rect_t xuiWindowGetIconSource(xui_widget pWidget)
{
	xui_window_data_t* pData = __xuiWindowGetData(pWidget);
	xui_rect_t tEmpty;
	memset(&tEmpty, 0, sizeof(tEmpty));
	return (pData != NULL) ? pData->tIconSrc : tEmpty;
}

static int __xuiWindowSetOpenInternal(xui_widget pWidget, xui_window_data_t* pData, int bOpen, int bRaise)
{
	int bOldActive;
	int iOpenLayer;
	int iOpenZ;
	int iRet;

	if ( (pWidget == NULL) || (pData == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	bOpen = bOpen ? 1 : 0;
	if ( pData->bOpen == bOpen ) {
		if ( bOpen && bRaise && xuiWidgetGetEffectiveVisible(pWidget) &&
		     xuiWidgetGetEffectiveEnabled(pWidget) ) {
			return __xuiWindowActivate(pWidget, pData, 1);
		}
		return XUI_OK;
	}
	if ( bOpen ) {
		bOldActive = pData->bActive;
		iOpenLayer = XUI_LAYER_FLOATING;
		iOpenZ = pData->bTopMost ? XUI_WINDOW_Z_TOPMOST : XUI_WINDOW_Z_NORMAL;
		/* Modal wrappers prepare the window before opening so activation is
		 * evaluated inside the new modal focus scope, not the previous one. */
		if ( pWidget->bModalFocus &&
			xuiWidgetGetLayer(pWidget, &iOpenLayer, &iOpenZ) == XUI_OK &&
			iOpenLayer < XUI_LAYER_MODAL ) {
			iOpenLayer = XUI_LAYER_MODAL;
			iOpenZ = 0;
		}
		pData->bOpen = 1;
		iRet = xuiWidgetSetVisible(pWidget, 1);
		if ( iRet != XUI_OK ) goto open_failed;
		iRet = xuiOverlayAttach(xuiWidgetGetContext(pWidget), NULL, pWidget, iOpenLayer, iOpenZ);
		if ( iRet != XUI_OK ) goto open_failed;
		if ( xuiWidgetGetEffectiveEnabled(pWidget) ) {
			iRet = __xuiWindowActivate(pWidget, pData, bRaise);
			if ( !__xuiWindowDataAlive(pWidget, pData) ) return iRet;
			if ( iRet != XUI_OK ) goto open_failed;
		} else {
			pData->bActive = 0;
		}
		pData->iChangeCount++;
		return XUI_OK;
	open_failed:
		pData->bOpen = 0;
		pData->bActive = bOldActive;
		(void)xuiWidgetSetVisible(pWidget, 0);
		return iRet;
	}
	__xuiWindowCancelInteraction(pWidget, pData, 1);
	__xuiWindowCancelButtonPress(pWidget, pData, 1);
	iRet = xuiWidgetSetVisible(pWidget, 0);
	if ( iRet != XUI_OK ) return iRet;
	pData->bOpen = 0;
	pData->bActive = 0;
	pData->iChangeCount++;
	return xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
}

XUI_API int xuiWindowSetOpen(xui_widget pWidget, int bOpen)
{
	xui_window_data_t* pData = __xuiWindowGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	return __xuiWindowSetOpenInternal(pWidget, pData, bOpen, 1);
}

XUI_API int xuiWindowIsOpen(xui_widget pWidget)
{
	xui_window_data_t* pData = __xuiWindowGetData(pWidget);
	return (pData != NULL) ? pData->bOpen : 0;
}

XUI_API int xuiWindowSetShowTitleBar(xui_widget pWidget, int bShow)
{
	xui_window_data_t* pData = __xuiWindowGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	bShow = bShow ? 1 : 0;
	if ( pData->bShowTitleBar == bShow ) return XUI_OK;
	__xuiWindowCancelInteraction(pWidget, pData, 1);
	__xuiWindowCancelButtonPress(pWidget, pData, 1);
	pData->bShowTitleBar = bShow;
	__xuiWindowApplyDefaultResizeEdges(pData);
	pData->iChangeCount++;
	return xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
}

XUI_API int xuiWindowGetShowTitleBar(xui_widget pWidget)
{
	xui_window_data_t* pData = __xuiWindowGetData(pWidget);
	return (pData != NULL) ? pData->bShowTitleBar : 0;
}

XUI_API int xuiWindowSetMovable(xui_widget pWidget, int bEnabled)
{
	xui_window_data_t* pData = __xuiWindowGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	bEnabled = bEnabled ? 1 : 0;
	if ( pData->bMovable == bEnabled ) return XUI_OK;
	__xuiWindowCancelInteraction(pWidget, pData, 1);
	pData->bMovable = bEnabled;
	return XUI_OK;
}

XUI_API int xuiWindowIsMovable(xui_widget pWidget)
{
	xui_window_data_t* pData = __xuiWindowGetData(pWidget);
	return (pData != NULL) ? pData->bMovable : 0;
}

XUI_API int xuiWindowSetDragAnywhere(xui_widget pWidget, int bEnabled)
{
	xui_window_data_t* pData = __xuiWindowGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	bEnabled = bEnabled ? 1 : 0;
	if ( pData->bDragAnywhere == bEnabled ) return XUI_OK;
	__xuiWindowCancelInteraction(pWidget, pData, 1);
	pData->bDragAnywhere = bEnabled;
	__xuiWindowApplyDefaultResizeEdges(pData);
	return xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
}

XUI_API int xuiWindowIsDragAnywhere(xui_widget pWidget)
{
	xui_window_data_t* pData = __xuiWindowGetData(pWidget);
	return (pData != NULL) ? pData->bDragAnywhere : 0;
}

XUI_API int xuiWindowSetResizable(xui_widget pWidget, int bEnabled)
{
	xui_window_data_t* pData = __xuiWindowGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	bEnabled = bEnabled ? 1 : 0;
	if ( pData->bResizable == bEnabled ) return XUI_OK;
	__xuiWindowCancelInteraction(pWidget, pData, 1);
	pData->bResizable = bEnabled;
	return xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
}

XUI_API int xuiWindowIsResizable(xui_widget pWidget)
{
	xui_window_data_t* pData = __xuiWindowGetData(pWidget);
	return (pData != NULL) ? pData->bResizable : 0;
}

XUI_API int xuiWindowSetResizeEdges(xui_widget pWidget, uint32_t iEdges)
{
	xui_window_data_t* pData = __xuiWindowGetData(pWidget);
	uint32_t iNewEdges;
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	iNewEdges = iEdges & XUI_WINDOW_EDGE_ALL;
	if ( pData->bResizeEdgesExplicit && pData->iResizeEdges == iNewEdges ) return XUI_OK;
	__xuiWindowCancelInteraction(pWidget, pData, 1);
	pData->bResizeEdgesExplicit = 1;
	pData->iResizeEdges = iNewEdges;
	return xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
}

XUI_API uint32_t xuiWindowGetResizeEdges(xui_widget pWidget)
{
	xui_window_data_t* pData = __xuiWindowGetData(pWidget);
	return (pData != NULL) ? pData->iResizeEdges : 0u;
}

XUI_API int xuiWindowSetShowCollapse(xui_widget pWidget, int bShow)
{
	xui_window_data_t* pData = __xuiWindowGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	bShow = bShow ? 1 : 0;
	if ( pData->bShowCollapse == bShow ) return XUI_OK;
	__xuiWindowCancelButtonPress(pWidget, pData, 1);
	pData->bShowCollapse = bShow;
	return xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
}

XUI_API int xuiWindowGetShowCollapse(xui_widget pWidget)
{
	xui_window_data_t* pData = __xuiWindowGetData(pWidget);
	return (pData != NULL) ? pData->bShowCollapse : 0;
}

XUI_API int xuiWindowSetShowMaximize(xui_widget pWidget, int bShow)
{
	xui_window_data_t* pData = __xuiWindowGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	bShow = bShow ? 1 : 0;
	if ( pData->bShowMaximize == bShow ) return XUI_OK;
	__xuiWindowCancelButtonPress(pWidget, pData, 1);
	pData->bShowMaximize = bShow;
	return xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
}

XUI_API int xuiWindowGetShowMaximize(xui_widget pWidget)
{
	xui_window_data_t* pData = __xuiWindowGetData(pWidget);
	return (pData != NULL) ? pData->bShowMaximize : 0;
}

XUI_API int xuiWindowSetShowClose(xui_widget pWidget, int bShow)
{
	xui_window_data_t* pData = __xuiWindowGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	bShow = bShow ? 1 : 0;
	if ( pData->bShowClose == bShow ) return XUI_OK;
	__xuiWindowCancelButtonPress(pWidget, pData, 1);
	pData->bShowClose = bShow;
	xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_NODE_CHANGED);
	return xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
}

XUI_API int xuiWindowGetShowClose(xui_widget pWidget)
{
	xui_window_data_t* pData = __xuiWindowGetData(pWidget);
	return (pData != NULL) ? pData->bShowClose : 0;
}

XUI_API int xuiWindowSetCollapsed(xui_widget pWidget, int bCollapsed)
{
	xui_window_data_t* pData = __xuiWindowGetData(pWidget);
	xui_window_resolved_t tResolved;
	xui_rect_t tOldRect;
	xui_rect_t tRect;
	float fOldExpandedHeight;
	float fCollapsedHeight;
	int bOldCollapsed;
	int iRet;
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	bCollapsed = bCollapsed ? 1 : 0;
	if ( pData->bCollapsed == bCollapsed ) return XUI_OK;
	__xuiWindowCancelInteraction(pWidget, pData, 1);
	__xuiWindowCancelButtonPress(pWidget, pData, 1);
	__xuiWindowResolve(pWidget, pData, &tResolved);
	tOldRect = xuiWidgetGetRect(pWidget);
	tRect = tOldRect;
	bOldCollapsed = pData->bCollapsed;
	fOldExpandedHeight = pData->fExpandedHeight;
	fCollapsedHeight = __xuiWindowCollapsedHeight(pData, &tResolved);
	if ( bCollapsed ) {
		if ( tRect.fH > fCollapsedHeight ) pData->fExpandedHeight = tRect.fH;
		tRect.fH = 0;
	} else {
		if ( pData->fExpandedHeight < fCollapsedHeight + 48.0f ) pData->fExpandedHeight = fCollapsedHeight + 96.0f;
		tRect = xuiInternalRectFromFloatNearest(tRect.fX, tRect.fY, tRect.fW, pData->fExpandedHeight);
	}
	pData->bCollapsed = bCollapsed;
	iRet = __xuiWindowApplyRect(pWidget, pData, tRect);
	if ( iRet != XUI_OK ) {
		pData->bCollapsed = bOldCollapsed;
		pData->fExpandedHeight = fOldExpandedHeight;
		(void)__xuiWindowApplyRect(pWidget, pData, tOldRect);
		return iRet;
	}
	xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_STATE_CHANGED);
	pData->iChangeCount++;
	return xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
}

XUI_API int xuiWindowIsCollapsed(xui_widget pWidget)
{
	xui_window_data_t* pData = __xuiWindowGetData(pWidget);
	return (pData != NULL) ? pData->bCollapsed : 0;
}

XUI_API int xuiWindowSetMaximized(xui_widget pWidget, int bMaximized)
{
	xui_window_data_t* pData = __xuiWindowGetData(pWidget);
	xui_rect_t tOldRect;
	xui_rect_t tOldRestoreRect;
	xui_rect_t tRect;
	float fOldExpandedHeight;
	int bOldCollapsed;
	int bOldMaximized;
	int iRet;
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	bMaximized = bMaximized ? 1 : 0;
	if ( pData->bMaximized == bMaximized && !(bMaximized && pData->bCollapsed) ) return XUI_OK;
	__xuiWindowCancelInteraction(pWidget, pData, 1);
	__xuiWindowCancelButtonPress(pWidget, pData, 1);
	tOldRect = xuiWidgetGetRect(pWidget);
	tOldRestoreRect = pData->tRestoreRect;
	bOldCollapsed = pData->bCollapsed;
	bOldMaximized = pData->bMaximized;
	fOldExpandedHeight = pData->fExpandedHeight;
	if ( pData->bMaximized == bMaximized ) {
		if ( bMaximized && pData->bCollapsed ) {
			pData->bCollapsed = 0;
			tRect = __xuiWindowParentRect(pWidget);
			iRet = __xuiWindowApplyRect(pWidget, pData, tRect);
			if ( iRet != XUI_OK ) {
				pData->bCollapsed = bOldCollapsed;
				pData->fExpandedHeight = fOldExpandedHeight;
				(void)__xuiWindowApplyRect(pWidget, pData, tOldRect);
				return iRet;
			}
			pData->iChangeCount++;
			xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_STATE_CHANGED);
			return xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
		}
		return XUI_OK;
	}
	if ( bMaximized ) {
		pData->bCollapsed = 0;
		pData->bMaximized = 1;
		tRect = __xuiWindowParentRect(pWidget);
		iRet = __xuiWindowApplyRect(pWidget, pData, tRect);
		if ( iRet == XUI_OK ) pData->tRestoreRect = tOldRect;
	} else {
		pData->bMaximized = 0;
		if ( pData->tRestoreRect.fW > 0.0f && pData->tRestoreRect.fH > 0.0f ) {
			iRet = __xuiWindowApplyRect(pWidget, pData, pData->tRestoreRect);
		} else iRet = XUI_OK;
	}
	if ( iRet != XUI_OK ) {
		pData->bCollapsed = bOldCollapsed;
		pData->bMaximized = bOldMaximized;
		pData->tRestoreRect = tOldRestoreRect;
		pData->fExpandedHeight = fOldExpandedHeight;
		(void)__xuiWindowApplyRect(pWidget, pData, tOldRect);
		return iRet;
	}
	pData->iChangeCount++;
	xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_STATE_CHANGED);
	return xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
}

XUI_API int xuiWindowIsMaximized(xui_widget pWidget)
{
	xui_window_data_t* pData = __xuiWindowGetData(pWidget);
	return (pData != NULL) ? pData->bMaximized : 0;
}

XUI_API int xuiWindowBringToFront(xui_widget pWidget)
{
	xui_window_data_t* pData = __xuiWindowGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	return __xuiWindowActivate(pWidget, pData, 1);
}

XUI_API int xuiWindowSetTopMost(xui_widget pWidget, int bTopMost)
{
	xui_window_data_t* pData = __xuiWindowGetData(pWidget);
	int bOldTopMost;
	int iRet;
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	bTopMost = bTopMost ? 1 : 0;
	if ( pData->bTopMost == bTopMost ) return XUI_OK;
	bOldTopMost = pData->bTopMost;
	pData->bTopMost = bTopMost;
	iRet = __xuiWindowApplyLayerAndZ(pWidget, pData);
	if ( pData->bOpen ) {
		if ( iRet == XUI_OK ) iRet = xuiOverlayBringToFront(pWidget);
	}
	if ( iRet != XUI_OK ) {
		pData->bTopMost = bOldTopMost;
		(void)__xuiWindowApplyLayerAndZ(pWidget, pData);
		return iRet;
	}
	pData->iChangeCount++;
	return xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
}

XUI_API int xuiWindowIsTopMost(xui_widget pWidget)
{
	xui_window_data_t* pData = __xuiWindowGetData(pWidget);
	return (pData != NULL) ? pData->bTopMost : 0;
}

XUI_API int xuiWindowIsActive(xui_widget pWidget)
{
	xui_window_data_t* pData = __xuiWindowGetData(pWidget);
	return (pData != NULL) ? pData->bActive : 0;
}

XUI_API xui_widget xuiWindowGetActive(xui_context pContext)
{
	xui_widget pFound;
	if ( !xuiInternalContextIsValid(pContext) ) return NULL;
	pFound = __xuiWindowFindActiveInTree(xuiOverlayRoot(pContext));
	if ( pFound != NULL ) return pFound;
	return __xuiWindowFindActiveInTree(xuiGetRootWidget(pContext));
}

XUI_API int xuiWindowSetChrome(xui_widget pWidget, float fTitleBarHeight, float fBorderWidth, float fResizeGrip, float fButtonSize)
{
	xui_window_data_t* pData = __xuiWindowGetData(pWidget);
	if ( pData == NULL || !__xuiWindowFloatValid(fTitleBarHeight) ||
	     !__xuiWindowFloatValid(fBorderWidth) || !__xuiWindowFloatValid(fResizeGrip) ||
	     !__xuiWindowFloatValid(fButtonSize) || fTitleBarHeight < 20.0f ||
	     fResizeGrip < 2.0f || fButtonSize < 12.0f ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pData->fTitleBarHeight == fTitleBarHeight && pData->fBorderWidth == fBorderWidth &&
	     pData->fResizeGrip == fResizeGrip && pData->fButtonSize == fButtonSize ) return XUI_OK;
	__xuiWindowCancelInteraction(pWidget, pData, 1);
	__xuiWindowCancelButtonPress(pWidget, pData, 1);
	pData->fTitleBarHeight = fTitleBarHeight;
	pData->fBorderWidth = fBorderWidth;
	pData->fResizeGrip = fResizeGrip;
	pData->fButtonSize = fButtonSize;
	return xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
}

XUI_API int xuiWindowGetChrome(xui_widget pWidget, float* pTitleBarHeight, float* pBorderWidth, float* pResizeGrip, float* pButtonSize)
{
	xui_window_data_t* pData = __xuiWindowGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pTitleBarHeight != NULL ) *pTitleBarHeight = pData->fTitleBarHeight;
	if ( pBorderWidth != NULL ) *pBorderWidth = pData->fBorderWidth;
	if ( pResizeGrip != NULL ) *pResizeGrip = pData->fResizeGrip;
	if ( pButtonSize != NULL ) *pButtonSize = pData->fButtonSize;
	return XUI_OK;
}

XUI_API int xuiWindowSetIconSize(xui_widget pWidget, float fIconSize)
{
	xui_window_data_t* pData = __xuiWindowGetData(pWidget);
	if ( pData == NULL || !__xuiWindowFloatValid(fIconSize) || fIconSize < 1.0f ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pData->fIconSize == fIconSize ) return XUI_OK;
	pData->fIconSize = fIconSize;
	return xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
}

XUI_API float xuiWindowGetIconSize(xui_widget pWidget)
{
	xui_window_data_t* pData = __xuiWindowGetData(pWidget);
	return (pData != NULL) ? pData->fIconSize : 0.0f;
}

XUI_API int xuiWindowSetMinSize(xui_widget pWidget, float fWidth, float fHeight)
{
	xui_window_data_t* pData = __xuiWindowGetData(pWidget);
	int iRet;
	if ( pData == NULL || !__xuiWindowFloatValid(fWidth) || !__xuiWindowFloatValid(fHeight) ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pData->fMinWidth == fWidth && pData->fMinHeight == fHeight ) return XUI_OK;
	__xuiWindowCancelInteraction(pWidget, pData, 1);
	iRet = xuiWidgetSetMinSize(pWidget, (xui_vec2_t){fWidth, fHeight});
	if ( iRet != XUI_OK ) return iRet;
	pData->fMinWidth = fWidth;
	pData->fMinHeight = fHeight;
	return xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
}

XUI_API int xuiWindowGetMinSize(xui_widget pWidget, float* pWidth, float* pHeight)
{
	xui_window_data_t* pData = __xuiWindowGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pWidth != NULL ) *pWidth = pData->fMinWidth;
	if ( pHeight != NULL ) *pHeight = pData->fMinHeight;
	return XUI_OK;
}

XUI_API int xuiWindowSetColors(xui_widget pWidget, uint32_t iBackground, uint32_t iClient, uint32_t iTitleBar, uint32_t iTitleText, uint32_t iBorder, uint32_t iButtonNormal, uint32_t iButtonHover, uint32_t iButtonActive)
{
	xui_window_data_t* pData = __xuiWindowGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	pData->iBackgroundColor = iBackground;
	pData->iClientColor = iClient;
	pData->iTitleBarColor = iTitleBar;
	pData->iTitleTextColor = iTitleText;
	pData->iBorderColor = iBorder;
	pData->iActiveBorderColor = iBorder;
	pData->iButtonColor = iButtonNormal;
	pData->iButtonHoverColor = iButtonHover;
	pData->iButtonActiveColor = iButtonActive;
	return xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
}

XUI_API int xuiWindowGetColors(xui_widget pWidget, uint32_t* pBackground, uint32_t* pClient, uint32_t* pTitleBar, uint32_t* pTitleText, uint32_t* pBorder, uint32_t* pButtonNormal, uint32_t* pButtonHover, uint32_t* pButtonActive)
{
	xui_window_data_t* pData = __xuiWindowGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pBackground != NULL ) *pBackground = pData->iBackgroundColor;
	if ( pClient != NULL ) *pClient = pData->iClientColor;
	if ( pTitleBar != NULL ) *pTitleBar = pData->iTitleBarColor;
	if ( pTitleText != NULL ) *pTitleText = pData->iTitleTextColor;
	if ( pBorder != NULL ) *pBorder = pData->iBorderColor;
	if ( pButtonNormal != NULL ) *pButtonNormal = pData->iButtonColor;
	if ( pButtonHover != NULL ) *pButtonHover = pData->iButtonHoverColor;
	if ( pButtonActive != NULL ) *pButtonActive = pData->iButtonActiveColor;
	return XUI_OK;
}

XUI_API int xuiWindowSetStateColors(xui_widget pWidget, uint32_t iInactiveTitleBar, uint32_t iInactiveTitleText, uint32_t iActiveBorder, uint32_t iCloseHover, uint32_t iCloseActive)
{
	xui_window_data_t* pData = __xuiWindowGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	pData->iInactiveTitleBarColor = iInactiveTitleBar;
	pData->iInactiveTitleTextColor = iInactiveTitleText;
	pData->iActiveBorderColor = iActiveBorder;
	pData->iCloseHoverColor = iCloseHover;
	pData->iCloseActiveColor = iCloseActive;
	return xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
}

XUI_API int xuiWindowGetStateColors(xui_widget pWidget, uint32_t* pInactiveTitleBar, uint32_t* pInactiveTitleText, uint32_t* pActiveBorder, uint32_t* pCloseHover, uint32_t* pCloseActive)
{
	xui_window_data_t* pData = __xuiWindowGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pInactiveTitleBar != NULL ) *pInactiveTitleBar = pData->iInactiveTitleBarColor;
	if ( pInactiveTitleText != NULL ) *pInactiveTitleText = pData->iInactiveTitleTextColor;
	if ( pActiveBorder != NULL ) *pActiveBorder = pData->iActiveBorderColor;
	if ( pCloseHover != NULL ) *pCloseHover = pData->iCloseHoverColor;
	if ( pCloseActive != NULL ) *pCloseActive = pData->iCloseActiveColor;
	return XUI_OK;
}

XUI_API xui_rect_t xuiWindowGetTitleBarRect(xui_widget pWidget)
{
	xui_window_data_t* pData = __xuiWindowGetData(pWidget);
	xui_rect_t tEmpty;
	memset(&tEmpty, 0, sizeof(tEmpty));
	return (pData != NULL) ? pData->tTitleBarRect : tEmpty;
}

XUI_API xui_rect_t xuiWindowGetClientRect(xui_widget pWidget)
{
	xui_window_data_t* pData = __xuiWindowGetData(pWidget);
	xui_rect_t tEmpty;
	memset(&tEmpty, 0, sizeof(tEmpty));
	return (pData != NULL) ? pData->tClientRect : tEmpty;
}

XUI_API xui_rect_t xuiWindowGetCollapseButtonRect(xui_widget pWidget)
{
	xui_window_data_t* pData = __xuiWindowGetData(pWidget);
	xui_rect_t tEmpty;
	memset(&tEmpty, 0, sizeof(tEmpty));
	return (pData != NULL) ? pData->tCollapseButtonRect : tEmpty;
}

XUI_API xui_rect_t xuiWindowGetMaximizeButtonRect(xui_widget pWidget)
{
	xui_window_data_t* pData = __xuiWindowGetData(pWidget);
	xui_rect_t tEmpty;
	memset(&tEmpty, 0, sizeof(tEmpty));
	return (pData != NULL) ? pData->tMaximizeButtonRect : tEmpty;
}

XUI_API xui_rect_t xuiWindowGetCloseButtonRect(xui_widget pWidget)
{
	xui_window_data_t* pData = __xuiWindowGetData(pWidget);
	xui_rect_t tEmpty;
	memset(&tEmpty, 0, sizeof(tEmpty));
	return (pData != NULL) ? pData->tCloseButtonRect : tEmpty;
}

XUI_API uint32_t xuiWindowGetResizeEdgesAt(xui_widget pWidget, float fX, float fY)
{
	xui_window_data_t* pData = __xuiWindowGetData(pWidget);
	xui_window_resolved_t tResolved;
	if ( pData == NULL ) return 0u;
	__xuiWindowResolve(pWidget, pData, &tResolved);
	return __xuiWindowResizeEdgesAtData(pWidget, pData, &tResolved, fX, fY);
}

XUI_API int xuiWindowGetHoverPart(xui_widget pWidget)
{
	xui_window_data_t* pData = __xuiWindowGetData(pWidget);
	return (pData != NULL) ? pData->iHoverPart : XUI_WINDOW_PART_NONE;
}

XUI_API int xuiWindowGetActivePart(xui_widget pWidget)
{
	xui_window_data_t* pData = __xuiWindowGetData(pWidget);
	return (pData != NULL) ? pData->iActivePart : XUI_WINDOW_PART_NONE;
}

XUI_API uint32_t xuiWindowGetState(xui_widget pWidget)
{
	xui_window_data_t* pData = __xuiWindowGetData(pWidget);
	uint32_t iState;
	if ( pData == NULL ) return 0u;
	iState = 0u;
	if ( pData->bOpen ) iState |= XUI_WINDOW_STATE_OPEN;
	if ( pData->bCollapsed ) iState |= XUI_WINDOW_STATE_COLLAPSED;
	if ( pData->bMaximized ) iState |= XUI_WINDOW_STATE_MAXIMIZED;
	if ( pData->bActive ) iState |= XUI_WINDOW_STATE_ACTIVE;
	if ( pData->bTopMost ) iState |= XUI_WINDOW_STATE_TOPMOST;
	if ( pData->iInteractionEdges == -1 ) iState |= XUI_WINDOW_STATE_DRAGGING;
	if ( pData->iInteractionEdges > 0 ) iState |= XUI_WINDOW_STATE_RESIZING;
	return iState;
}

XUI_API int xuiWindowGetChangeCount(xui_widget pWidget)
{
	xui_window_data_t* pData = __xuiWindowGetData(pWidget);
	return (pData != NULL) ? pData->iChangeCount : 0;
}

XUI_API int xuiWindowGetCloseCount(xui_widget pWidget)
{
	xui_window_data_t* pData = __xuiWindowGetData(pWidget);
	return (pData != NULL) ? pData->iCloseCount : 0;
}

#endif
