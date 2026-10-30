#include "xui_config.h"
#if XUI_ENABLE_SEPARATOR
#include "xui_internal.h"

#include <math.h>
#include <string.h>

#define XUI_SEPARATOR_MAX_PATTERN_SEGMENTS 16384

typedef struct xui_separator_data_t {
	uint32_t iColor;
	float fThickness;
	int iOrientation;
	int iAlign;
	int iLineStyle;
} xui_separator_data_t;

static uint32_t __xuiSeparatorColorAlpha(uint32_t iColor)
{
	return iColor & 0xffu;
}

static int __xuiSeparatorOrientationValid(int iOrientation)
{
	return (iOrientation == XUI_SEPARATOR_HORIZONTAL) ||
	       (iOrientation == XUI_SEPARATOR_VERTICAL);
}

static int __xuiSeparatorAlignValid(int iAlign)
{
	return (iAlign == XUI_ALIGN_START) ||
	       (iAlign == XUI_ALIGN_CENTER) ||
	       (iAlign == XUI_ALIGN_END);
}

static int __xuiSeparatorLineStyleValid(int iLineStyle)
{
	return (iLineStyle == XUI_SEPARATOR_SOLID) ||
	       (iLineStyle == XUI_SEPARATOR_DOT) ||
	       (iLineStyle == XUI_SEPARATOR_DASH) ||
	       (iLineStyle == XUI_SEPARATOR_DASH_DOT);
}

static int __xuiSeparatorThicknessValid(float fThickness)
{
	return isfinite(fThickness) && fThickness >= 0.0f &&
	       fThickness <= XUI_LAYOUT_UNBOUNDED;
}

static int __xuiSeparatorDescValid(const xui_separator_desc_t* pDesc)
{
	if ( pDesc == NULL ) {
		return 1;
	}
	if ( (pDesc->iSize != 0) && (pDesc->iSize < sizeof(*pDesc)) ) {
		return 0;
	}
	if ( !__xuiSeparatorThicknessValid(pDesc->fThickness) ) {
		return 0;
	}
	if ( !__xuiSeparatorOrientationValid(pDesc->iOrientation) ||
	     !__xuiSeparatorLineStyleValid(pDesc->iLineStyle) ) {
		return 0;
	}
	if ( (pDesc->iAlign != 0) && !__xuiSeparatorAlignValid(pDesc->iAlign) ) {
		return 0;
	}
	return 1;
}

static xui_separator_data_t* __xuiSeparatorGetData(xui_widget pWidget)
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
	pType = xuiWidgetFindType(pContext, "separator");
	if ( (pType == NULL) || !xuiWidgetIsType(pWidget, pType) ) {
		return NULL;
	}
	return (xui_separator_data_t*)xuiWidgetGetTypeData(pWidget);
}

static int __xuiSeparatorStyleColor(xui_widget pWidget, const char* sName, uint32_t* pColor)
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

static int __xuiSeparatorStyleFloat(xui_widget pWidget, const char* sName, float* pValue)
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
	     __xuiSeparatorThicknessValid(tProperty.tValue.fFloat) ) {
		*pValue = tProperty.tValue.fFloat;
		return 1;
	}
	return 0;
}

static int __xuiSeparatorStyleInt(xui_widget pWidget, const char* sName, int* pValue)
{
	xui_style_property_t tProperty;
	int iRet;

	if ( (pWidget == NULL) || (sName == NULL) || (pValue == NULL) ) {
		return 0;
	}
	memset(&tProperty, 0, sizeof(tProperty));
	tProperty.iSize = sizeof(tProperty);
	iRet = xuiWidgetGetResolvedStyleProperty(pWidget, sName, &tProperty);
	if ( (iRet == XUI_OK) && (tProperty.tValue.iType == XUI_STYLE_VALUE_INT) ) {
		*pValue = tProperty.tValue.iInt;
		return 1;
	}
	return 0;
}

static void __xuiSeparatorResolve(xui_widget pWidget, const xui_separator_data_t* pData, xui_separator_data_t* pResolved)
{
	int iValue;

	*pResolved = *pData;
	(void)__xuiSeparatorStyleColor(pWidget, "separator.color", &pResolved->iColor);
	(void)__xuiSeparatorStyleFloat(pWidget, "separator.thickness", &pResolved->fThickness);
	iValue = pResolved->iOrientation;
	if ( __xuiSeparatorStyleInt(pWidget, "separator.orientation", &iValue) && __xuiSeparatorOrientationValid(iValue) ) {
		pResolved->iOrientation = iValue;
	}
	iValue = pResolved->iAlign;
	if ( __xuiSeparatorStyleInt(pWidget, "separator.align", &iValue) && __xuiSeparatorAlignValid(iValue) ) {
		pResolved->iAlign = iValue;
	}
	iValue = pResolved->iLineStyle;
	if ( __xuiSeparatorStyleInt(pWidget, "separator.line_style", &iValue) && __xuiSeparatorLineStyleValid(iValue) ) {
		pResolved->iLineStyle = iValue;
	}
	if ( !__xuiSeparatorThicknessValid(pResolved->fThickness) ) {
		pResolved->fThickness = 0.0f;
	}
}

static int __xuiSeparatorApplyData(xui_widget pWidget, xui_separator_data_t* pData,
	const xui_separator_data_t* pNext)
{
	xui_separator_data_t tOldResolved;
	xui_separator_data_t tNextResolved;
	uint32_t iDirtyFlags;
	int iOldThickness;
	int iNextThickness;
	int iRet;

	if ( (pWidget == NULL) || (pData == NULL) || (pNext == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	__xuiSeparatorResolve(pWidget, pData, &tOldResolved);
	__xuiSeparatorResolve(pWidget, pNext, &tNextResolved);
	iOldThickness = xuiInternalSnapSize(tOldResolved.fThickness);
	iNextThickness = xuiInternalSnapSize(tNextResolved.fThickness);
	iDirtyFlags = 0u;
	if ( (tOldResolved.iOrientation != tNextResolved.iOrientation) ||
	     (iOldThickness != iNextThickness) ) {
		iDirtyFlags |= XUI_WIDGET_DIRTY_LAYOUT;
	}
	if ( (tOldResolved.iColor != tNextResolved.iColor) ||
	     (iOldThickness != iNextThickness) ||
	     (tOldResolved.iOrientation != tNextResolved.iOrientation) ||
	     (tOldResolved.iAlign != tNextResolved.iAlign) ||
	     (tOldResolved.iLineStyle != tNextResolved.iLineStyle) ) {
		iDirtyFlags |= XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER;
	}
	if ( iDirtyFlags != 0u ) {
		iRet = xuiWidgetInvalidate(pWidget, iDirtyFlags);
		if ( iRet != XUI_OK ) {
			return iRet;
		}
	}
	*pData = *pNext;
	if ( tOldResolved.iOrientation != tNextResolved.iOrientation ) {
		xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_STATE_CHANGED);
	}
	return XUI_OK;
}

static xui_rect_t __xuiSeparatorLineRectFromData(xui_widget pWidget, const xui_separator_data_t* pData)
{
	xui_rect_t tRect;
	int iThickness;
	int iRemaining;

	memset(&tRect, 0, sizeof(tRect));
	if ( (pWidget == NULL) || (pData == NULL) ) {
		return tRect;
	}
	tRect = xuiInternalSnapRect(xuiWidgetGetContentRect(pWidget));
	iThickness = xuiInternalSnapSize(pData->fThickness);
	if ( pData->iOrientation == XUI_SEPARATOR_VERTICAL ) {
		if ( iThickness > tRect.fW ) {
			iThickness = tRect.fW;
		}
		iRemaining = tRect.fW - iThickness;
		if ( pData->iAlign == XUI_ALIGN_CENTER ) {
			tRect.fX += (iRemaining + 1) / 2;
		} else if ( pData->iAlign == XUI_ALIGN_END ) {
			tRect.fX += iRemaining;
		}
		tRect.fW = iThickness;
	} else {
		if ( iThickness > tRect.fH ) {
			iThickness = tRect.fH;
		}
		iRemaining = tRect.fH - iThickness;
		if ( pData->iAlign == XUI_ALIGN_CENTER ) {
			tRect.fY += (iRemaining + 1) / 2;
		} else if ( pData->iAlign == XUI_ALIGN_END ) {
			tRect.fY += iRemaining;
		}
		tRect.fH = iThickness;
	}
	return tRect;
}

static int __xuiSeparatorDrawSegment(xui_proxy pProxy, xui_draw_context pDraw,
	xui_rect_t tLine, int bVertical, int64_t iStart, int64_t iLength, uint32_t iColor)
{
	xui_rect_t tPart;
	int64_t iLimit;
	int64_t iRemaining;

	if ( (iStart < 0) || (iLength <= 0) ) {
		return XUI_OK;
	}
	tPart = tLine;
	if ( bVertical ) {
		iLimit = tLine.fH;
		if ( iStart >= iLimit ) {
			return XUI_OK;
		}
		iRemaining = iLimit - iStart;
		tPart.fY += (int)iStart;
		tPart.fH = (int)((iLength < iRemaining) ? iLength : iRemaining);
	} else {
		iLimit = tLine.fW;
		if ( iStart >= iLimit ) {
			return XUI_OK;
		}
		iRemaining = iLimit - iStart;
		tPart.fX += (int)iStart;
		tPart.fW = (int)((iLength < iRemaining) ? iLength : iRemaining);
	}
	if ( (tPart.fW <= 0) || (tPart.fH <= 0) ) {
		return XUI_OK;
	}
	return pProxy->drawRectFill(pProxy, pDraw, tPart, iColor);
}

static int __xuiSeparatorDrawPattern(xui_proxy pProxy, xui_draw_context pDraw, const xui_separator_data_t* pData, xui_rect_t tLine)
{
	int64_t iThickness;
	int64_t iCursor;
	int64_t iMain;
	int64_t iDash;
	int64_t iDot;
	int64_t iGap;
	int64_t iPatternSpan;
	int64_t iSegmentCount;
	int bVertical;
	int iRet;

	bVertical = (pData->iOrientation == XUI_SEPARATOR_VERTICAL);
	iMain = bVertical ? tLine.fH : tLine.fW;
	if ( pData->iLineStyle == XUI_SEPARATOR_SOLID ) {
		return pProxy->drawRectFill(pProxy, pDraw, tLine, pData->iColor);
	}
	iThickness = xuiInternalSnapSize(pData->fThickness);
	if ( iThickness < 1 ) iThickness = 1;
	iDot = iThickness;
	iDash = iThickness * 4;
	iGap = iThickness * 2;
	if ( pData->iLineStyle == XUI_SEPARATOR_DOT ) {
		iPatternSpan = iDot + iThickness;
		iSegmentCount = (iMain + iPatternSpan - 1) / iPatternSpan;
	} else if ( pData->iLineStyle == XUI_SEPARATOR_DASH ) {
		iPatternSpan = iDash + iGap;
		iSegmentCount = (iMain + iPatternSpan - 1) / iPatternSpan;
	} else {
		iPatternSpan = iDash + iGap + iDot + iGap;
		iSegmentCount = ((iMain + iPatternSpan - 1) / iPatternSpan) * 2;
	}
	if ( iSegmentCount > XUI_SEPARATOR_MAX_PATTERN_SEGMENTS ) {
		return pProxy->drawRectFill(pProxy, pDraw, tLine, pData->iColor);
	}
	iCursor = 0;
	while ( iCursor < iMain ) {
		if ( pData->iLineStyle == XUI_SEPARATOR_DOT ) {
			iRet = __xuiSeparatorDrawSegment(pProxy, pDraw, tLine, bVertical,
				iCursor, iDot, pData->iColor);
			if ( iRet != XUI_OK ) return iRet;
			iCursor += iDot + iThickness;
		} else if ( pData->iLineStyle == XUI_SEPARATOR_DASH ) {
			iRet = __xuiSeparatorDrawSegment(pProxy, pDraw, tLine, bVertical,
				iCursor, iDash, pData->iColor);
			if ( iRet != XUI_OK ) return iRet;
			iCursor += iDash + iGap;
		} else {
			iRet = __xuiSeparatorDrawSegment(pProxy, pDraw, tLine, bVertical,
				iCursor, iDash, pData->iColor);
			if ( iRet != XUI_OK ) return iRet;
			iCursor += iDash + iGap;
			iRet = __xuiSeparatorDrawSegment(pProxy, pDraw, tLine, bVertical,
				iCursor, iDot, pData->iColor);
			if ( iRet != XUI_OK ) return iRet;
			iCursor += iDot + iGap;
		}
	}
	return XUI_OK;
}

static int __xuiSeparatorContentMeasure(xui_widget pWidget, xui_vec2_t tConstraint, xui_vec2_t* pSize, void* pUser)
{
	xui_separator_data_t tResolved;
	float fThickness;

	if ( (pWidget == NULL) || (pSize == NULL) || (pUser == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	__xuiSeparatorResolve(pWidget, (xui_separator_data_t*)pUser, &tResolved);
	fThickness = xuiInternalSnapSize(tResolved.fThickness);
	pSize->fX = (tResolved.iOrientation == XUI_SEPARATOR_VERTICAL) ? fThickness : 0.0f;
	pSize->fY = (tResolved.iOrientation == XUI_SEPARATOR_VERTICAL) ? 0.0f : fThickness;
	if ( (tConstraint.fX >= 0.0f) &&
	     (tConstraint.fX < XUI_LAYOUT_UNBOUNDED) &&
	     (pSize->fX > tConstraint.fX) ) pSize->fX = tConstraint.fX;
	if ( (tConstraint.fY >= 0.0f) &&
	     (tConstraint.fY < XUI_LAYOUT_UNBOUNDED) &&
	     (pSize->fY > tConstraint.fY) ) pSize->fY = tConstraint.fY;
	return XUI_OK;
}

static int __xuiSeparatorCacheRender(xui_widget pWidget, xui_draw_context pDraw, uint32_t iStateId, void* pUser)
{
	xui_separator_data_t* pData;
	xui_separator_data_t tResolved;
	xui_proxy pProxy;
	xui_rect_t tLine;

	(void)iStateId;
	(void)pUser;
	if ( (pWidget == NULL) || (pDraw == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pData = __xuiSeparatorGetData(pWidget);
	if ( pData == NULL ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	__xuiSeparatorResolve(pWidget, pData, &tResolved);
	if ( (__xuiSeparatorColorAlpha(tResolved.iColor) == 0) || (tResolved.fThickness <= 0.0f) ) {
		return XUI_OK;
	}
	pProxy = xuiInternalContextGetProxy(xuiWidgetGetContext(pWidget));
	if ( (pProxy == NULL) || (pProxy->drawRectFill == NULL) ) {
		return XUI_ERROR_NOT_INITIALIZED;
	}
	tLine = __xuiSeparatorLineRectFromData(pWidget, &tResolved);
	if ( (tLine.fW <= 0.0f) || (tLine.fH <= 0.0f) ) {
		return XUI_OK;
	}
	return __xuiSeparatorDrawPattern(pProxy, pDraw, &tResolved, tLine);
}

static int __xuiSeparatorInit(xui_widget pWidget, void* pTypeData, const void* pCreateData, void* pUser)
{
	xui_separator_data_t* pData;
	const xui_separator_desc_t* pDesc;
	int iRet;

	(void)pUser;
	pData = (xui_separator_data_t*)pTypeData;
	pDesc = (const xui_separator_desc_t*)pCreateData;
	if ( (pWidget == NULL) || (pData == NULL) || !__xuiSeparatorDescValid(pDesc) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	memset(pData, 0, sizeof(*pData));
	pData->iColor = XUI_COLOR_RGBA(90, 104, 122, 180);
	pData->fThickness = 1.0f;
	pData->iOrientation = XUI_SEPARATOR_HORIZONTAL;
	pData->iAlign = XUI_ALIGN_CENTER;
	pData->iLineStyle = XUI_SEPARATOR_SOLID;
	if ( pDesc != NULL ) {
		pData->iColor = (pDesc->iColor != 0) ? pDesc->iColor : pData->iColor;
		pData->fThickness = (pDesc->fThickness > 0.0f) ? pDesc->fThickness : pData->fThickness;
		pData->iOrientation = pDesc->iOrientation;
		if ( pDesc->iAlign != 0 ) {
			pData->iAlign = pDesc->iAlign;
		}
		pData->iLineStyle = pDesc->iLineStyle;
	}
	iRet = xuiWidgetSetFocusable(pWidget, 0);
	if ( iRet == XUI_OK ) {
		iRet = xuiWidgetSetTabStop(pWidget, 0);
	}
	return iRet;
}

static void __xuiSeparatorDestroy(xui_widget pWidget, void* pTypeData, void* pUser)
{
	(void)pWidget;
	(void)pUser;
	if ( pTypeData != NULL ) {
		memset(pTypeData, 0, sizeof(xui_separator_data_t));
	}
}

static void __xuiSeparatorDefaultLayout(xui_layout_t* pLayout)
{
	memset(pLayout, 0, sizeof(*pLayout));
	pLayout->iLayoutType = XUI_LAYOUT_MANUAL;
	pLayout->iWidthMode = XUI_SIZE_CONTENT;
	pLayout->iHeightMode = XUI_SIZE_CONTENT;
	pLayout->iFlowMode = XUI_FLOW_BLOCK;
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

static void __xuiSeparatorDefaultCachePolicy(xui_cache_policy_t* pPolicy)
{
	memset(pPolicy, 0, sizeof(*pPolicy));
	pPolicy->iSize = sizeof(*pPolicy);
	pPolicy->iPolicy = XUI_CACHE_POLICY_SELF;
	pPolicy->iFlags = XUI_CACHE_CLEAR_ON_UPDATE;
	pPolicy->iClearColor = XUI_COLOR_RGBA(0, 0, 0, 0);
}

static void __xuiSeparatorRegisterStyleProperty(xui_context pContext, xui_widget_type pType, const char* sName, int iValueType, uint32_t iDirtyFlags)
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
	tInfo.pWidgetType = pType;
	(void)xuiStyleRegisterProperty(pContext, &tInfo, NULL);
}

static void __xuiSeparatorRegisterStyleProperties(xui_context pContext, xui_widget_type pType)
{
	__xuiSeparatorRegisterStyleProperty(pContext, pType, "separator.color", XUI_STYLE_VALUE_COLOR, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	__xuiSeparatorRegisterStyleProperty(pContext, pType, "separator.thickness", XUI_STYLE_VALUE_FLOAT, XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	__xuiSeparatorRegisterStyleProperty(pContext, pType, "separator.orientation", XUI_STYLE_VALUE_INT, XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	__xuiSeparatorRegisterStyleProperty(pContext, pType, "separator.align", XUI_STYLE_VALUE_INT, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	__xuiSeparatorRegisterStyleProperty(pContext, pType, "separator.line_style", XUI_STYLE_VALUE_INT, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
}

static int __xuiSeparatorAccessibleNode(xui_widget pWidget, xui_accessible_node_t* pNode)
{
	xui_separator_data_t* pData;
	xui_separator_data_t tResolved;

	pData = __xuiSeparatorGetData(pWidget);
	if ( (pData == NULL) || (pNode == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	__xuiSeparatorResolve(pWidget, pData, &tResolved);
	pNode->iRole = XUI_ACCESSIBLE_ROLE_SEPARATOR;
	pNode->sName = xuiWidgetGetAccessibleName(pWidget) != NULL ?
		xuiWidgetGetAccessibleName(pWidget) : "Separator";
	pNode->sDescription = xuiWidgetGetAccessibleDescription(pWidget);
	if ( tResolved.iOrientation == XUI_SEPARATOR_VERTICAL ) {
		pNode->iState |= XUI_ACCESSIBLE_STATE_VERTICAL;
	}
	return XUI_OK;
}

static const xui_internal_accessibility_adapter_t g_xuiSeparatorAccessible = {
	.getNode = __xuiSeparatorAccessibleNode,
	.performAction = NULL
};

XUI_API xui_widget_type xuiSeparatorGetType(xui_context pContext)
{
	xui_widget_type_desc_t tDesc;
	xui_widget_type pType;
	int iRet;

	if ( !xuiInternalContextIsValid(pContext) ) {
		return NULL;
	}
	pType = xuiWidgetFindType(pContext, "separator");
	if ( pType != NULL ) {
		__xuiSeparatorRegisterStyleProperties(pContext, pType);
		pType->pAccessibleAdapter = &g_xuiSeparatorAccessible;
		return pType;
	}
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.sName = "separator";
	tDesc.pParent = xuiWidgetGetBaseType();
	tDesc.iFlags = XUI_WIDGET_TYPE_DEFAULT_LAYOUT | XUI_WIDGET_TYPE_DEFAULT_CACHE_POLICY;
	tDesc.iTypeDataSize = sizeof(xui_separator_data_t);
	tDesc.onInit = __xuiSeparatorInit;
	tDesc.onDestroy = __xuiSeparatorDestroy;
	tDesc.onContentMeasure = __xuiSeparatorContentMeasure;
	tDesc.onCacheRender = __xuiSeparatorCacheRender;
	__xuiSeparatorDefaultLayout(&tDesc.tLayout);
	__xuiSeparatorDefaultCachePolicy(&tDesc.tCachePolicy);
	iRet = xuiWidgetRegisterType(pContext, &pType, &tDesc);
	if ( iRet != XUI_OK ) {
		return NULL;
	}
	__xuiSeparatorRegisterStyleProperties(pContext, pType);
	pType->pAccessibleAdapter = &g_xuiSeparatorAccessible;
	return pType;
}

XUI_API int xuiSeparatorCreate(xui_context pContext, xui_widget* ppWidget, const xui_separator_desc_t* pDesc)
{
	xui_widget_type pType;

	if ( ppWidget == NULL ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	*ppWidget = NULL;
	if ( !__xuiSeparatorDescValid(pDesc) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pType = xuiSeparatorGetType(pContext);
	if ( pType == NULL ) {
		return XUI_ERROR_NOT_INITIALIZED;
	}
	return xuiWidgetCreateTyped(pContext, pType, ppWidget, pDesc);
}

XUI_API int xuiSeparatorSetColor(xui_widget pWidget, uint32_t iColor)
{
	xui_separator_data_t* pData;
	xui_separator_data_t tNext;

	pData = __xuiSeparatorGetData(pWidget);
	if ( pData == NULL ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( pData->iColor == iColor ) {
		return XUI_OK;
	}
	tNext = *pData;
	tNext.iColor = iColor;
	return __xuiSeparatorApplyData(pWidget, pData, &tNext);
}

XUI_API uint32_t xuiSeparatorGetColor(xui_widget pWidget)
{
	xui_separator_data_t* pData;

	pData = __xuiSeparatorGetData(pWidget);
	return (pData != NULL) ? pData->iColor : 0;
}

XUI_API int xuiSeparatorSetThickness(xui_widget pWidget, float fThickness)
{
	xui_separator_data_t* pData;
	xui_separator_data_t tNext;

	if ( !__xuiSeparatorThicknessValid(fThickness) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pData = __xuiSeparatorGetData(pWidget);
	if ( pData == NULL ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( pData->fThickness == fThickness ) {
		return XUI_OK;
	}
	tNext = *pData;
	tNext.fThickness = fThickness;
	return __xuiSeparatorApplyData(pWidget, pData, &tNext);
}

XUI_API float xuiSeparatorGetThickness(xui_widget pWidget)
{
	xui_separator_data_t* pData;

	pData = __xuiSeparatorGetData(pWidget);
	return (pData != NULL) ? pData->fThickness : 0.0f;
}

XUI_API int xuiSeparatorSetOrientation(xui_widget pWidget, int iOrientation)
{
	xui_separator_data_t* pData;
	xui_separator_data_t tNext;

	if ( !__xuiSeparatorOrientationValid(iOrientation) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pData = __xuiSeparatorGetData(pWidget);
	if ( pData == NULL ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( pData->iOrientation == iOrientation ) {
		return XUI_OK;
	}
	tNext = *pData;
	tNext.iOrientation = iOrientation;
	return __xuiSeparatorApplyData(pWidget, pData, &tNext);
}

XUI_API int xuiSeparatorGetOrientation(xui_widget pWidget)
{
	xui_separator_data_t* pData;

	pData = __xuiSeparatorGetData(pWidget);
	return (pData != NULL) ? pData->iOrientation : XUI_SEPARATOR_HORIZONTAL;
}

XUI_API int xuiSeparatorSetAlign(xui_widget pWidget, int iAlign)
{
	xui_separator_data_t* pData;
	xui_separator_data_t tNext;

	if ( !__xuiSeparatorAlignValid(iAlign) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pData = __xuiSeparatorGetData(pWidget);
	if ( pData == NULL ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( pData->iAlign == iAlign ) {
		return XUI_OK;
	}
	tNext = *pData;
	tNext.iAlign = iAlign;
	return __xuiSeparatorApplyData(pWidget, pData, &tNext);
}

XUI_API int xuiSeparatorGetAlign(xui_widget pWidget)
{
	xui_separator_data_t* pData;

	pData = __xuiSeparatorGetData(pWidget);
	return (pData != NULL) ? pData->iAlign : XUI_ALIGN_START;
}

XUI_API int xuiSeparatorSetLineStyle(xui_widget pWidget, int iLineStyle)
{
	xui_separator_data_t* pData;
	xui_separator_data_t tNext;

	if ( !__xuiSeparatorLineStyleValid(iLineStyle) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pData = __xuiSeparatorGetData(pWidget);
	if ( pData == NULL ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( pData->iLineStyle == iLineStyle ) {
		return XUI_OK;
	}
	tNext = *pData;
	tNext.iLineStyle = iLineStyle;
	return __xuiSeparatorApplyData(pWidget, pData, &tNext);
}

XUI_API int xuiSeparatorGetLineStyle(xui_widget pWidget)
{
	xui_separator_data_t* pData;

	pData = __xuiSeparatorGetData(pWidget);
	return (pData != NULL) ? pData->iLineStyle : XUI_SEPARATOR_SOLID;
}

XUI_API xui_rect_t xuiSeparatorGetLineRect(xui_widget pWidget)
{
	xui_separator_data_t* pData;
	xui_separator_data_t tResolved;
	xui_rect_t tRect;

	memset(&tRect, 0, sizeof(tRect));
	pData = __xuiSeparatorGetData(pWidget);
	if ( pData == NULL ) {
		return tRect;
	}
	__xuiSeparatorResolve(pWidget, pData, &tResolved);
	return __xuiSeparatorLineRectFromData(pWidget, &tResolved);
}

#endif
