#include "xui_config.h"
#if XUI_ENABLE_CHECK_CARD
#include "xui_internal.h"
#include "xui_choice_internal.h"
#include "xui_pointer_press_internal.h"

#include <math.h>
#include <string.h>

typedef struct xui_check_card_data_t {
	xui_check_card_change_proc onChange;
	void* pChangeUser;
	xui_widget pRadioGroup;
	int bChecked;
	int bKeyboardActive;
	xui_pointer_press_t tPointerPress;
	int iChangeCount;
	float fBorderWidth;
	float fCheckedBorderWidth;
	float fCornerSize;
	float fFocusWidth;
	uint32_t iBackgroundColor;
	uint32_t iHoverBackgroundColor;
	uint32_t iActiveBackgroundColor;
	uint32_t iCheckedBackgroundColor;
	uint32_t iBorderColor;
	uint32_t iHoverBorderColor;
	uint32_t iCheckedBorderColor;
	uint32_t iDisabledBorderColor;
	uint32_t iCornerColor;
	uint32_t iCheckColor;
	uint32_t iFocusColor;
	xui_rect_t tCornerRect;
} xui_check_card_data_t;

static int __xuiCheckCardNonNegativeFloatValid(float fValue)
{
	return isfinite(fValue) && fValue >= 0.0f &&
	       fValue <= XUI_LAYOUT_UNBOUNDED;
}

static int __xuiCheckCardDescValid(const xui_check_card_desc_t* pDesc)
{
	if ( pDesc == NULL ) return 1;
	if ( (pDesc->iSize != 0) && (pDesc->iSize < sizeof(*pDesc)) ) return 0;
	if ( !__xuiCheckCardNonNegativeFloatValid(pDesc->fBorderWidth) ||
	     !__xuiCheckCardNonNegativeFloatValid(pDesc->fCheckedBorderWidth) ||
	     !__xuiCheckCardNonNegativeFloatValid(pDesc->fCornerSize) ||
	     !__xuiCheckCardNonNegativeFloatValid(pDesc->fFocusWidth) ||
	     !__xuiCheckCardNonNegativeFloatValid(pDesc->fMinWidth) ||
	     !__xuiCheckCardNonNegativeFloatValid(pDesc->fMinHeight) ||
	     !__xuiCheckCardNonNegativeFloatValid(pDesc->tPadding.fLeft) ||
	     !__xuiCheckCardNonNegativeFloatValid(pDesc->tPadding.fTop) ||
	     !__xuiCheckCardNonNegativeFloatValid(pDesc->tPadding.fRight) ||
	     !__xuiCheckCardNonNegativeFloatValid(pDesc->tPadding.fBottom) ) return 0;
	return 1;
}

static int __xuiCheckCardAlpha(uint32_t iColor)
{
	return (int)(iColor & 0xffu);
}

static xui_check_card_data_t* __xuiCheckCardGetData(xui_widget pWidget)
{
	xui_context pContext;
	xui_widget_type pType;

	if ( !xuiInternalWidgetIsValid(pWidget) ) return NULL;
	pContext = xuiWidgetGetContext(pWidget);
	if ( !xuiInternalContextIsValid(pContext) ) return NULL;
	pType = xuiWidgetFindType(pContext, "checkcard");
	if ( (pType == NULL) || !xuiWidgetIsType(pWidget, pType) ) return NULL;
	return (xui_check_card_data_t*)xuiWidgetGetTypeData(pWidget);
}

static int __xuiCheckCardDataAlive(xui_widget pWidget, const xui_check_card_data_t* pData)
{
	return (pData != NULL) && (__xuiCheckCardGetData(pWidget) == pData);
}

static int __xuiCheckCardIsRadioGroup(xui_widget pWidget, xui_widget pGroup)
{
	xui_context pContext;
	xui_widget_type pType;

	if ( !xuiInternalWidgetIsValid(pWidget) || !xuiInternalWidgetIsValid(pGroup) ) return 0;
	pContext = xuiWidgetGetContext(pWidget);
	if ( !xuiInternalContextIsValid(pContext) || xuiWidgetGetContext(pGroup) != pContext ) return 0;
	pType = xuiWidgetFindType(pContext, "radiogroup");
	return (pType != NULL) && xuiWidgetIsType(pGroup, pType);
}

static xui_widget __xuiCheckCardResolveRadioGroup(xui_widget pWidget, xui_check_card_data_t* pData)
{
	xui_widget pParent;

	if ( (pData == NULL) || !xuiInternalWidgetIsValid(pWidget) ) return NULL;
	pParent = xuiWidgetGetParent(pWidget);
	if ( __xuiCheckCardIsRadioGroup(pWidget, pParent) ) {
		pData->pRadioGroup = pParent;
		return pParent;
	}
	pData->pRadioGroup = NULL;
	return NULL;
}

static int __xuiCheckCardStyleColor(xui_widget pWidget, const char* sName, uint32_t* pColor)
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

static int __xuiCheckCardStyleFloat(xui_widget pWidget, const char* sName, float* pValue)
{
	xui_style_property_t tProperty;
	int iRet;

	memset(&tProperty, 0, sizeof(tProperty));
	tProperty.iSize = sizeof(tProperty);
	iRet = xuiWidgetGetResolvedStyleProperty(pWidget, sName, &tProperty);
	if ( (iRet == XUI_OK) && (tProperty.tValue.iType == XUI_STYLE_VALUE_FLOAT) &&
	     __xuiCheckCardNonNegativeFloatValid(tProperty.tValue.fFloat) ) {
		*pValue = tProperty.tValue.fFloat;
		return 1;
	}
	return 0;
}

static void __xuiCheckCardResolve(xui_widget pWidget, const xui_check_card_data_t* pData, xui_check_card_data_t* pResolved)
{
	*pResolved = *pData;
	(void)__xuiCheckCardStyleColor(pWidget, "checkcard.background.color", &pResolved->iBackgroundColor);
	(void)__xuiCheckCardStyleColor(pWidget, "checkcard.background.hover_color", &pResolved->iHoverBackgroundColor);
	(void)__xuiCheckCardStyleColor(pWidget, "checkcard.background.active_color", &pResolved->iActiveBackgroundColor);
	(void)__xuiCheckCardStyleColor(pWidget, "checkcard.background.checked_color", &pResolved->iCheckedBackgroundColor);
	(void)__xuiCheckCardStyleColor(pWidget, "checkcard.border.color", &pResolved->iBorderColor);
	(void)__xuiCheckCardStyleColor(pWidget, "checkcard.border.hover_color", &pResolved->iHoverBorderColor);
	(void)__xuiCheckCardStyleColor(pWidget, "checkcard.border.checked_color", &pResolved->iCheckedBorderColor);
	(void)__xuiCheckCardStyleColor(pWidget, "checkcard.border.disabled_color", &pResolved->iDisabledBorderColor);
	(void)__xuiCheckCardStyleColor(pWidget, "checkcard.corner.color", &pResolved->iCornerColor);
	(void)__xuiCheckCardStyleColor(pWidget, "checkcard.check.color", &pResolved->iCheckColor);
	(void)__xuiCheckCardStyleColor(pWidget, "checkcard.focus.color", &pResolved->iFocusColor);
	(void)__xuiCheckCardStyleFloat(pWidget, "checkcard.border.width", &pResolved->fBorderWidth);
	(void)__xuiCheckCardStyleFloat(pWidget, "checkcard.border.checked_width", &pResolved->fCheckedBorderWidth);
	(void)__xuiCheckCardStyleFloat(pWidget, "checkcard.corner.size", &pResolved->fCornerSize);
	(void)__xuiCheckCardStyleFloat(pWidget, "checkcard.focus.width", &pResolved->fFocusWidth);
}

static uint32_t __xuiCheckCardComputeState(xui_widget pWidget, const xui_check_card_data_t* pData)
{
	uint32_t iState;

	iState = xuiWidgetGetInputState(pWidget);
	if ( (pData != NULL) && pData->bChecked ) iState |= XUI_CHECKCARD_STATE_CHECKED;
	if ( (pData != NULL) && pData->bKeyboardActive ) iState |= XUI_WIDGET_STATE_ACTIVE;
	return iState;
}

static uint32_t __xuiCheckCardStateId(uint32_t iState)
{
	if ( ((iState & XUI_WIDGET_STATE_DISABLED) != 0) && ((iState & XUI_CHECKCARD_STATE_CHECKED) != 0) ) return XUI_WIDGET_STATE_DISABLED | XUI_CHECKCARD_STATE_CHECKED;
	if ( (iState & XUI_WIDGET_STATE_DISABLED) != 0 ) return XUI_WIDGET_STATE_DISABLED;
	if ( ((iState & XUI_CHECKCARD_STATE_CHECKED) != 0) && ((iState & XUI_WIDGET_STATE_ACTIVE) != 0) ) return XUI_CHECKCARD_STATE_CHECKED | XUI_WIDGET_STATE_ACTIVE;
	if ( ((iState & XUI_CHECKCARD_STATE_CHECKED) != 0) && ((iState & XUI_WIDGET_STATE_HOVER) != 0) ) return XUI_CHECKCARD_STATE_CHECKED | XUI_WIDGET_STATE_HOVER;
	if ( ((iState & XUI_CHECKCARD_STATE_CHECKED) != 0) && ((iState & XUI_WIDGET_STATE_FOCUS) != 0) ) return XUI_CHECKCARD_STATE_CHECKED | XUI_WIDGET_STATE_FOCUS;
	if ( (iState & XUI_WIDGET_STATE_ACTIVE) != 0 ) return XUI_WIDGET_STATE_ACTIVE;
	if ( (iState & XUI_WIDGET_STATE_HOVER) != 0 ) return XUI_WIDGET_STATE_HOVER;
	if ( (iState & XUI_WIDGET_STATE_FOCUS) != 0 ) return XUI_WIDGET_STATE_FOCUS;
	if ( (iState & XUI_CHECKCARD_STATE_CHECKED) != 0 ) return XUI_CHECKCARD_STATE_CHECKED;
	return 0;
}

static int __xuiCheckCardSyncState(xui_widget pWidget, xui_check_card_data_t* pData)
{
	return xuiWidgetSetStateId(pWidget, __xuiCheckCardStateId(__xuiCheckCardComputeState(pWidget, pData)));
}

static void __xuiCheckCardNotify(xui_widget pWidget, xui_check_card_data_t* pData)
{
	xui_check_card_change_proc onChange;
	void* pChangeUser;
	int bChecked;

	onChange = pData->onChange;
	pChangeUser = pData->pChangeUser;
	bChecked = pData->bChecked;
	pData->iChangeCount++;
	if ( onChange != NULL ) onChange(pWidget, bChecked, pChangeUser);
}

static int __xuiCheckCardSetCheckedInternal(xui_widget pWidget, xui_check_card_data_t* pData, int bChecked, int bNotify, int bFromGroup)
{
	xui_widget pGroup;
	int bOldChecked;
	int iRet;

	bChecked = (bChecked != 0);
	pGroup = __xuiCheckCardResolveRadioGroup(pWidget, pData);
	if ( !bFromGroup && (pGroup != NULL) ) {
		if ( bChecked ) return xuiRadioGroupSetSelectedWidget(pGroup, pWidget);
		if ( xuiRadioGroupGetSelectedWidget(pGroup) == pWidget ) {
			return xuiRadioGroupSetSelectedWidget(pGroup, NULL);
		}
	}
	if ( pData->bChecked == bChecked ) return XUI_OK;
	bOldChecked = pData->bChecked;
	pData->bChecked = bChecked;
	iRet = __xuiCheckCardSyncState(pWidget, pData);
	if ( iRet != XUI_OK ) {
		pData->bChecked = bOldChecked;
		(void)__xuiCheckCardSyncState(pWidget, pData);
		return iRet;
	}
	xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_STATE_CHANGED);
	xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_VALUE_CHANGED);
	if ( bNotify ) {
		__xuiCheckCardNotify(pWidget, pData);
		if ( !__xuiCheckCardDataAlive(pWidget, pData) ) return XUI_OK;
	}
	return XUI_OK;
}

static int __xuiCheckCardActivate(xui_widget pWidget, xui_check_card_data_t* pData)
{
	if ( (pData == NULL) || !xuiWidgetGetEffectiveEnabled(pWidget) ||
	     !xuiWidgetGetEffectiveVisible(pWidget) ) {
		return XUI_OK;
	}
	return __xuiCheckCardSetCheckedInternal(pWidget, pData,
		__xuiCheckCardResolveRadioGroup(pWidget, pData) != NULL ? 1 : !pData->bChecked, 1, 0);
}

static void __xuiCheckCardDefaultAction(xui_widget pWidget, void* pUser)
{
	xui_check_card_data_t* pData;

	(void)pUser;
	pData = __xuiCheckCardGetData(pWidget);
	(void)__xuiCheckCardActivate(pWidget, pData);
}

static int __xuiCheckCardEvent(xui_widget pWidget, const xui_event_t* pEvent, void* pUser)
{
	xui_check_card_data_t* pData;
	xui_context pContext;
	int iRet;

	(void)pUser;
	if ( (pWidget == NULL) || (pEvent == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	pData = __xuiCheckCardGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	pContext = xuiWidgetGetContext(pWidget);
	if ( !xuiWidgetGetEffectiveEnabled(pWidget) ||
	     !xuiWidgetGetEffectiveVisible(pWidget) ) {
		pData->bKeyboardActive = 0;
		iRet = xuiInternalPointerPressCancel(pContext, pWidget, &pData->tPointerPress);
		if ( iRet != XUI_OK ) return iRet;
		iRet = __xuiCheckCardSyncState(pWidget, pData);
#if XUI_ENABLE_RADIO_GROUP
		if ( iRet == XUI_OK && (pEvent->iType == XUI_EVENT_ENABLED_CHANGED ||
		     pEvent->iType == XUI_EVENT_VISIBLE_CHANGED) ) {
			xui_widget pGroup = __xuiCheckCardResolveRadioGroup(pWidget, pData);
			if ( pGroup != NULL ) iRet = xuiInternalRadioGroupSyncTabStops(pGroup);
		}
#endif
		return iRet;
	}
	switch ( pEvent->iType ) {
	case XUI_EVENT_POINTER_ENTER:
	case XUI_EVENT_POINTER_LEAVE:
	case XUI_EVENT_POINTER_MOVE:
	case XUI_EVENT_FOCUS:
		return __xuiCheckCardSyncState(pWidget, pData);
	case XUI_EVENT_BLUR:
		pData->bKeyboardActive = 0;
		return __xuiCheckCardSyncState(pWidget, pData);
	case XUI_EVENT_ENABLED_CHANGED:
	case XUI_EVENT_VISIBLE_CHANGED:
		pData->bKeyboardActive = 0;
		iRet = __xuiCheckCardSyncState(pWidget, pData);
#if XUI_ENABLE_RADIO_GROUP
		if ( iRet == XUI_OK ) {
			xui_widget pGroup = __xuiCheckCardResolveRadioGroup(pWidget, pData);
			if ( pGroup != NULL ) iRet = xuiInternalRadioGroupSyncTabStops(pGroup);
		}
#endif
		return iRet;
	case XUI_EVENT_POINTER_DOWN:
		if ( (pEvent->pTarget == pWidget) && (pEvent->iButton == XUI_POINTER_BUTTON_LEFT) ) {
			if ( pData->tPointerPress.tOwner.bActive ) {
				return xuiInternalPointerPressMatches(&pData->tPointerPress, pEvent) ?
					XUI_EVENT_DISPATCH_STOP : XUI_OK;
			}
			iRet = xuiInternalPointerPressBegin(pContext, pWidget, pEvent,
				&pData->tPointerPress);
			if ( iRet == XUI_OK ) iRet = __xuiCheckCardSyncState(pWidget, pData);
			if ( iRet != XUI_OK ) {
				(void)xuiInternalPointerPressCancel(pContext, pWidget, &pData->tPointerPress);
				return iRet;
			}
			return XUI_EVENT_DISPATCH_STOP;
		}
		break;
	case XUI_EVENT_POINTER_UP:
		if ( (pEvent->pTarget == pWidget) && (pEvent->iButton == XUI_POINTER_BUTTON_LEFT) &&
		     xuiInternalPointerPressMatches(&pData->tPointerPress, pEvent) ) {
			iRet = xuiInternalPointerPressEnd(pContext, pWidget, pEvent,
				&pData->tPointerPress);
			if ( iRet == XUI_OK && __xuiCheckCardDataAlive(pWidget, pData) ) {
				iRet = __xuiCheckCardSyncState(pWidget, pData);
			}
			if ( iRet != XUI_OK ) return iRet;
			return XUI_EVENT_DISPATCH_STOP;
		}
		break;
	case XUI_EVENT_POINTER_CLICK:
		if ( (pEvent->pTarget == pWidget) && (pEvent->iButton == XUI_POINTER_BUTTON_LEFT) ) {
			if ( !xuiInternalPointerPressAcceptClick(&pData->tPointerPress, pEvent) ) {
				return XUI_EVENT_DISPATCH_STOP;
			}
			iRet = __xuiCheckCardActivate(pWidget, pData);
			if ( iRet != XUI_OK ) return iRet;
			return XUI_EVENT_DISPATCH_STOP;
		}
		break;
	case XUI_EVENT_POINTER_CAPTURE_LOST:
		if ( !xuiInternalPointerPressCaptureLost(&pData->tPointerPress, pEvent) ) return XUI_OK;
		return __xuiCheckCardSyncState(pWidget, pData);
	case XUI_EVENT_KEY_DOWN:
		if ( (pEvent->pTarget == pWidget) &&
		     ((pEvent->iKey == XUI_KEY_SPACE) || (pEvent->iKey == XUI_KEY_ENTER)) ) {
			pData->bKeyboardActive = 1;
			iRet = __xuiCheckCardSyncState(pWidget, pData);
			if ( iRet != XUI_OK ) return iRet;
			return XUI_EVENT_DISPATCH_STOP;
		}
#if XUI_ENABLE_RADIO_GROUP
		if ( (pEvent->pTarget == pWidget) &&
		     ((pEvent->iKey == XUI_KEY_LEFT) || (pEvent->iKey == XUI_KEY_UP)) ) {
			iRet = xuiInternalRadioGroupMoveSelection(pWidget, -1);
			if ( iRet == XUI_ERROR_UNSUPPORTED ) return XUI_OK;
			if ( iRet != XUI_OK ) return iRet;
			return XUI_EVENT_DISPATCH_STOP;
		}
		if ( (pEvent->pTarget == pWidget) &&
		     ((pEvent->iKey == XUI_KEY_RIGHT) || (pEvent->iKey == XUI_KEY_DOWN)) ) {
			iRet = xuiInternalRadioGroupMoveSelection(pWidget, 1);
			if ( iRet == XUI_ERROR_UNSUPPORTED ) return XUI_OK;
			if ( iRet != XUI_OK ) return iRet;
			return XUI_EVENT_DISPATCH_STOP;
		}
#endif
		break;
	case XUI_EVENT_KEY_UP:
		if ( (pEvent->pTarget == pWidget) &&
		     ((pEvent->iKey == XUI_KEY_SPACE) || (pEvent->iKey == XUI_KEY_ENTER)) &&
		     pData->bKeyboardActive ) {
			pData->bKeyboardActive = 0;
			iRet = __xuiCheckCardSyncState(pWidget, pData);
			if ( iRet == XUI_OK ) iRet = __xuiCheckCardActivate(pWidget, pData);
			if ( iRet != XUI_OK ) return iRet;
			return XUI_EVENT_DISPATCH_STOP;
		}
		break;
	default:
		break;
	}
	return XUI_OK;
}

static int __xuiCheckCardDrawRectFill(xui_proxy pProxy, xui_draw_context pDraw, xui_rect_t tRect, uint32_t iColor)
{
	if ( __xuiCheckCardAlpha(iColor) == 0 ) return XUI_OK;
	return pProxy->drawRectFill(pProxy, pDraw, tRect, iColor);
}

static int __xuiCheckCardDrawRectStroke(xui_proxy pProxy, xui_draw_context pDraw, xui_rect_t tRect, float fWidth, uint32_t iColor)
{
	if ( fWidth <= 0.0f || __xuiCheckCardAlpha(iColor) == 0 ) return XUI_OK;
	return pProxy->drawRectStroke(pProxy, pDraw, tRect, fWidth, iColor);
}

static int __xuiCheckCardCacheRender(xui_widget pWidget, xui_draw_context pDraw, uint32_t iStateId, void* pUser)
{
	xui_check_card_data_t* pData;
	xui_check_card_data_t tResolved;
	xui_proxy pProxy;
	xui_rect_t tRect;
	xui_vec2_t tA;
	xui_vec2_t tB;
	xui_vec2_t tC;
	uint32_t iBg;
	uint32_t iBorder;
	float fBorderWidth;
	float fCorner;
	float fCornerMax;
	float fCornerInset;
	float fCornerRight;
	float fCornerTop;
	float fFocusWidth;
	float fMetricMax;
	xui_rect_t tFocusRect;
	int bChecked;
	int iRet;

	(void)pUser;
	pData = __xuiCheckCardGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	pProxy = xuiInternalContextGetProxy(xuiWidgetGetContext(pWidget));
	if ( pProxy == NULL ) return XUI_ERROR_NOT_INITIALIZED;
	__xuiCheckCardResolve(pWidget, pData, &tResolved);
	tRect = xuiWidgetGetContentRect(pWidget);
	bChecked = ((iStateId & XUI_CHECKCARD_STATE_CHECKED) != 0);
	iBg = bChecked ? tResolved.iCheckedBackgroundColor : tResolved.iBackgroundColor;
	if ( (iStateId & XUI_WIDGET_STATE_ACTIVE) != 0 ) iBg = tResolved.iActiveBackgroundColor;
	else if ( (iStateId & XUI_WIDGET_STATE_HOVER) != 0 ) iBg = tResolved.iHoverBackgroundColor;
	iBorder = bChecked ? tResolved.iCheckedBorderColor : tResolved.iBorderColor;
	if ( (iStateId & XUI_WIDGET_STATE_DISABLED) != 0 ) iBorder = tResolved.iDisabledBorderColor;
	else if ( !bChecked && ((iStateId & XUI_WIDGET_STATE_HOVER) != 0) ) iBorder = tResolved.iHoverBorderColor;
	fBorderWidth = bChecked ? tResolved.fCheckedBorderWidth : tResolved.fBorderWidth;
	fMetricMax = (tRect.fW < tRect.fH) ? tRect.fW : tRect.fH;
	if ( fMetricMax < 0.0f ) fMetricMax = 0.0f;
	if ( fBorderWidth > fMetricMax ) fBorderWidth = fMetricMax;
	iRet = __xuiCheckCardDrawRectFill(pProxy, pDraw, tRect, iBg);
	if ( iRet != XUI_OK ) return iRet;
	iRet = __xuiCheckCardDrawRectStroke(pProxy, pDraw, xuiInternalInsetRect(tRect, fBorderWidth * 0.5f), fBorderWidth, iBorder);
	if ( iRet != XUI_OK ) return iRet;
	fCornerInset = (fBorderWidth > 0.0f) ? fBorderWidth : 0.0f;
	fCornerMax = tRect.fW - fCornerInset * 2.0f;
	if ( fCornerMax > tRect.fH - fCornerInset * 2.0f ) {
		fCornerMax = tRect.fH - fCornerInset * 2.0f;
	}
	if ( fCornerMax < 0.0f ) fCornerMax = 0.0f;
	fCorner = (tResolved.fCornerSize < fCornerMax) ? tResolved.fCornerSize : fCornerMax;
	pData->tCornerRect = (xui_rect_t){tRect.fX + tRect.fW - fCorner - fCornerInset,
		tRect.fY + fCornerInset, fCorner, fCorner};
	if ( bChecked && fCorner > 0.0f && pProxy->drawTriangleFill != NULL ) {
		fCornerRight = tRect.fX + tRect.fW - fCornerInset;
		fCornerTop = tRect.fY + fCornerInset;
		tA = (xui_vec2_t){fCornerRight - fCorner, fCornerTop};
		tB = (xui_vec2_t){fCornerRight, fCornerTop};
		tC = (xui_vec2_t){fCornerRight, fCornerTop + fCorner};
		iRet = pProxy->drawTriangleFill(pProxy, pDraw, tA, tB, tC, tResolved.iCornerColor);
		if ( iRet != XUI_OK ) return iRet;
		if ( pProxy->drawLine != NULL ) {
			iRet = pProxy->drawLine(pProxy, pDraw, fCornerRight - fCorner * 0.40f, fCornerTop + fCorner * 0.32f, fCornerRight - fCorner * 0.28f, fCornerTop + fCorner * 0.46f, 1.7f, tResolved.iCheckColor);
			if ( iRet == XUI_OK ) iRet = pProxy->drawLine(pProxy, pDraw, fCornerRight - fCorner * 0.28f, fCornerTop + fCorner * 0.46f, fCornerRight - fCorner * 0.12f, fCornerTop + fCorner * 0.24f, 1.7f, tResolved.iCheckColor);
			if ( iRet != XUI_OK ) return iRet;
		}
	}
	if ( ((iStateId & XUI_WIDGET_STATE_FOCUS) != 0) && ((iStateId & XUI_WIDGET_STATE_DISABLED) == 0) ) {
		tFocusRect = xuiInternalInsetRect(tRect, 2.0f);
		fMetricMax = (tFocusRect.fW < tFocusRect.fH) ? tFocusRect.fW : tFocusRect.fH;
		if ( fMetricMax < 0.0f ) fMetricMax = 0.0f;
		fFocusWidth = (tResolved.fFocusWidth < fMetricMax) ? tResolved.fFocusWidth : fMetricMax;
		iRet = __xuiCheckCardDrawRectStroke(pProxy, pDraw, tFocusRect, fFocusWidth, tResolved.iFocusColor);
	}
	return iRet;
}

static void __xuiCheckCardDefaultLayout(xui_layout_t* pLayout)
{
	memset(pLayout, 0, sizeof(*pLayout));
	pLayout->iLayoutType = XUI_LAYOUT_COLUMN;
	pLayout->iWidthMode = XUI_SIZE_CONTENT;
	pLayout->iHeightMode = XUI_SIZE_CONTENT;
	pLayout->iFlowMode = XUI_FLOW_BLOCK;
	pLayout->iDock = XUI_DOCK_FILL;
	pLayout->iOverflow = XUI_OVERFLOW_CLIP;
	pLayout->iAlignX = XUI_ALIGN_START;
	pLayout->iAlignY = XUI_ALIGN_START;
	pLayout->iTableRowSpan = 1;
	pLayout->iTableColumnSpan = 1;
	pLayout->iGridColumnCount = 1;
	pLayout->fMaxWidth = XUI_LAYOUT_UNBOUNDED;
	pLayout->fMaxHeight = XUI_LAYOUT_UNBOUNDED;
	pLayout->fShrink = 1.0f;
}

static void __xuiCheckCardDefaultCachePolicy(xui_cache_policy_t* pPolicy)
{
	memset(pPolicy, 0, sizeof(*pPolicy));
	pPolicy->iSize = sizeof(*pPolicy);
	pPolicy->iPolicy = XUI_CACHE_POLICY_SELF;
	pPolicy->iFlags = XUI_CACHE_CLEAR_ON_UPDATE | XUI_CACHE_UPDATE_ALL_STATES;
	pPolicy->iClearColor = XUI_COLOR_RGBA(0, 0, 0, 0);
}

static int __xuiCheckCardInitCacheStates(xui_widget pWidget)
{
	static const uint32_t arrState[] = {
		0, XUI_WIDGET_STATE_HOVER, XUI_WIDGET_STATE_ACTIVE, XUI_WIDGET_STATE_FOCUS, XUI_WIDGET_STATE_DISABLED,
		XUI_CHECKCARD_STATE_CHECKED, XUI_CHECKCARD_STATE_CHECKED | XUI_WIDGET_STATE_HOVER,
		XUI_CHECKCARD_STATE_CHECKED | XUI_WIDGET_STATE_ACTIVE, XUI_CHECKCARD_STATE_CHECKED | XUI_WIDGET_STATE_FOCUS,
		XUI_CHECKCARD_STATE_CHECKED | XUI_WIDGET_STATE_DISABLED
	};
	int i;
	int iRet;

	iRet = xuiWidgetSetCacheStateCount(pWidget, (int)(sizeof(arrState) / sizeof(arrState[0])));
	if ( iRet != XUI_OK ) return iRet;
	for ( i = 0; i < (int)(sizeof(arrState) / sizeof(arrState[0])); i++ ) {
		iRet = xuiWidgetSetCacheStateId(pWidget, i, 0x7c000000u + (uint32_t)i);
		if ( iRet != XUI_OK ) return iRet;
	}
	for ( i = 0; i < (int)(sizeof(arrState) / sizeof(arrState[0])); i++ ) {
		iRet = xuiWidgetSetCacheStateId(pWidget, i, arrState[i]);
		if ( iRet != XUI_OK ) return iRet;
	}
	return XUI_OK;
}

static int __xuiCheckCardInit(xui_widget pWidget, void* pTypeData, const void* pCreateData, void* pUser)
{
	xui_check_card_data_t* pData;
	const xui_check_card_desc_t* pDesc;
	xui_thickness_t tPadding;
	int iRet;

	(void)pUser;
	pData = (xui_check_card_data_t*)pTypeData;
	pDesc = (const xui_check_card_desc_t*)pCreateData;
	if ( (pWidget == NULL) || (pData == NULL) || !__xuiCheckCardDescValid(pDesc) ) return XUI_ERROR_INVALID_ARGUMENT;
	memset(pData, 0, sizeof(*pData));
	pData->bChecked = (pDesc != NULL) ? (pDesc->bChecked != 0) : 0;
	pData->fBorderWidth = (pDesc != NULL && pDesc->fBorderWidth > 0.0f) ? pDesc->fBorderWidth : 1.0f;
	pData->fCheckedBorderWidth = (pDesc != NULL && pDesc->fCheckedBorderWidth > 0.0f) ? pDesc->fCheckedBorderWidth : 1.0f;
	pData->fCornerSize = (pDesc != NULL && pDesc->fCornerSize > 0.0f) ? pDesc->fCornerSize : 16.0f;
	pData->fFocusWidth = (pDesc != NULL && pDesc->fFocusWidth > 0.0f) ? pDesc->fFocusWidth : 0.0f;
	pData->iBackgroundColor = (pDesc != NULL && pDesc->iBackgroundColor != 0) ? pDesc->iBackgroundColor : XUI_COLOR_RGBA(255, 255, 255, 255);
	pData->iHoverBackgroundColor = (pDesc != NULL && pDesc->iHoverBackgroundColor != 0) ? pDesc->iHoverBackgroundColor : XUI_COLOR_RGBA(250, 253, 253, 255);
	pData->iActiveBackgroundColor = (pDesc != NULL && pDesc->iActiveBackgroundColor != 0) ? pDesc->iActiveBackgroundColor : XUI_COLOR_RGBA(244, 250, 250, 255);
	pData->iCheckedBackgroundColor = (pDesc != NULL && pDesc->iCheckedBackgroundColor != 0) ? pDesc->iCheckedBackgroundColor : XUI_COLOR_RGBA(255, 255, 255, 255);
	pData->iBorderColor = (pDesc != NULL && pDesc->iBorderColor != 0) ? pDesc->iBorderColor : XUI_COLOR_RGBA(222, 226, 230, 255);
	pData->iHoverBorderColor = (pDesc != NULL && pDesc->iHoverBorderColor != 0) ? pDesc->iHoverBorderColor : XUI_COLOR_RGBA(126, 211, 205, 255);
	pData->iCheckedBorderColor = (pDesc != NULL && pDesc->iCheckedBorderColor != 0) ? pDesc->iCheckedBorderColor : XUI_COLOR_RGBA(29, 196, 186, 255);
	pData->iDisabledBorderColor = (pDesc != NULL && pDesc->iDisabledBorderColor != 0) ? pDesc->iDisabledBorderColor : XUI_COLOR_RGBA(214, 219, 225, 160);
	pData->iCornerColor = (pDesc != NULL && pDesc->iCornerColor != 0) ? pDesc->iCornerColor : pData->iCheckedBorderColor;
	pData->iCheckColor = (pDesc != NULL && pDesc->iCheckColor != 0) ? pDesc->iCheckColor : XUI_COLOR_RGBA(255, 255, 255, 255);
	pData->iFocusColor = (pDesc != NULL && pDesc->iFocusColor != 0) ? pDesc->iFocusColor : XUI_COLOR_RGBA(29, 196, 186, 180);
	tPadding = (xui_thickness_t){16.0f, 14.0f, 16.0f, 14.0f};
	if ( pDesc != NULL && (pDesc->tPadding.fLeft != 0.0f || pDesc->tPadding.fTop != 0.0f || pDesc->tPadding.fRight != 0.0f || pDesc->tPadding.fBottom != 0.0f) ) tPadding = pDesc->tPadding;
	iRet = xuiWidgetSetPadding(pWidget, tPadding);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetMinSize(pWidget,
		(xui_vec2_t){(pDesc != NULL && pDesc->fMinWidth > 0.0f) ? pDesc->fMinWidth : 120.0f,
			(pDesc != NULL && pDesc->fMinHeight > 0.0f) ? pDesc->fMinHeight : 56.0f});
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetFocusable(pWidget, 1);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetTabStop(pWidget, 1);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetOverflow(pWidget, XUI_OVERFLOW_CLIP);
	if ( iRet == XUI_OK ) iRet = __xuiCheckCardInitCacheStates(pWidget);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetDefaultAction(pWidget, __xuiCheckCardDefaultAction, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_POINTER_ENTER, __xuiCheckCardEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_POINTER_LEAVE, __xuiCheckCardEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_POINTER_MOVE, __xuiCheckCardEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_POINTER_DOWN, __xuiCheckCardEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_POINTER_UP, __xuiCheckCardEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_POINTER_CLICK, __xuiCheckCardEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_POINTER_CAPTURE_LOST, __xuiCheckCardEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_FOCUS, __xuiCheckCardEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_BLUR, __xuiCheckCardEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_KEY_DOWN, __xuiCheckCardEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_KEY_UP, __xuiCheckCardEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_ENABLED_CHANGED, __xuiCheckCardEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_VISIBLE_CHANGED, __xuiCheckCardEvent, NULL);
	if ( iRet != XUI_OK ) return iRet;
	if ( (pDesc != NULL) && (pDesc->pRadioGroup != NULL) ) {
		iRet = xuiRadioGroupAddCheckCard(pDesc->pRadioGroup, pWidget);
		if ( iRet != XUI_OK ) return iRet;
	}
	return __xuiCheckCardSyncState(pWidget, pData);
}

static void __xuiCheckCardDestroy(xui_widget pWidget, void* pTypeData, void* pUser)
{
	xui_check_card_data_t* pData;

	(void)pUser;
	pData = (xui_check_card_data_t*)pTypeData;
	if ( pData == NULL ) return;
	(void)xuiInternalPointerPressCancel(xuiWidgetGetContext(pWidget), pWidget,
		&pData->tPointerPress);
	memset(pData, 0, sizeof(*pData));
}

static void __xuiCheckCardRegisterStyleProperty(xui_context pContext, xui_widget_type pType, const char* sName, int iValueType, uint32_t iDirtyFlags)
{
	xui_style_property_info_t tInfo;

	if ( xuiStyleFindProperty(pContext, sName) != 0 ) return;
	memset(&tInfo, 0, sizeof(tInfo));
	tInfo.iSize = sizeof(tInfo);
	tInfo.sName = sName;
	tInfo.iValueType = iValueType;
	tInfo.iDirtyFlags = iDirtyFlags;
	tInfo.pWidgetType = pType;
	(void)xuiStyleRegisterProperty(pContext, &tInfo, NULL);
}

static void __xuiCheckCardRegisterStyleProperties(xui_context pContext, xui_widget_type pType)
{
	uint32_t iPaintDirty;

	iPaintDirty = XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER;
	__xuiCheckCardRegisterStyleProperty(pContext, pType, "checkcard.background.color", XUI_STYLE_VALUE_COLOR, iPaintDirty);
	__xuiCheckCardRegisterStyleProperty(pContext, pType, "checkcard.background.hover_color", XUI_STYLE_VALUE_COLOR, iPaintDirty);
	__xuiCheckCardRegisterStyleProperty(pContext, pType, "checkcard.background.active_color", XUI_STYLE_VALUE_COLOR, iPaintDirty);
	__xuiCheckCardRegisterStyleProperty(pContext, pType, "checkcard.background.checked_color", XUI_STYLE_VALUE_COLOR, iPaintDirty);
	__xuiCheckCardRegisterStyleProperty(pContext, pType, "checkcard.border.color", XUI_STYLE_VALUE_COLOR, iPaintDirty);
	__xuiCheckCardRegisterStyleProperty(pContext, pType, "checkcard.border.hover_color", XUI_STYLE_VALUE_COLOR, iPaintDirty);
	__xuiCheckCardRegisterStyleProperty(pContext, pType, "checkcard.border.checked_color", XUI_STYLE_VALUE_COLOR, iPaintDirty);
	__xuiCheckCardRegisterStyleProperty(pContext, pType, "checkcard.border.disabled_color", XUI_STYLE_VALUE_COLOR, iPaintDirty);
	__xuiCheckCardRegisterStyleProperty(pContext, pType, "checkcard.corner.color", XUI_STYLE_VALUE_COLOR, iPaintDirty);
	__xuiCheckCardRegisterStyleProperty(pContext, pType, "checkcard.check.color", XUI_STYLE_VALUE_COLOR, iPaintDirty);
	__xuiCheckCardRegisterStyleProperty(pContext, pType, "checkcard.focus.color", XUI_STYLE_VALUE_COLOR, iPaintDirty);
	__xuiCheckCardRegisterStyleProperty(pContext, pType, "checkcard.border.width", XUI_STYLE_VALUE_FLOAT, iPaintDirty);
	__xuiCheckCardRegisterStyleProperty(pContext, pType, "checkcard.border.checked_width", XUI_STYLE_VALUE_FLOAT, iPaintDirty);
	__xuiCheckCardRegisterStyleProperty(pContext, pType, "checkcard.corner.size", XUI_STYLE_VALUE_FLOAT, iPaintDirty);
	__xuiCheckCardRegisterStyleProperty(pContext, pType, "checkcard.focus.width", XUI_STYLE_VALUE_FLOAT, iPaintDirty);
}

static int __xuiCheckCardAccessibleNode(xui_widget pWidget, xui_accessible_node_t* pNode)
{
	xui_check_card_data_t* pData;
	xui_widget pGroup;

	pData = __xuiCheckCardGetData(pWidget);
	if ( (pData == NULL) || (pNode == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	pGroup = __xuiCheckCardResolveRadioGroup(pWidget, pData);
	pNode->iRole = (pGroup != NULL) ? XUI_ACCESSIBLE_ROLE_RADIO_BUTTON : XUI_ACCESSIBLE_ROLE_CHECKBOX;
	pNode->sName = xuiWidgetGetAccessibleName(pWidget) != NULL ?
		xuiWidgetGetAccessibleName(pWidget) : "Check card";
	pNode->sDescription = xuiWidgetGetAccessibleDescription(pWidget);
	pNode->sValue = pData->bChecked ? "true" : "false";
	if ( pData->bChecked ) pNode->iState |= XUI_ACCESSIBLE_STATE_CHECKED;
	pNode->iActions = XUI_ACCESSIBLE_ACTION_MASK(XUI_ACCESSIBLE_ACTION_ACTIVATE);
	if ( pGroup == NULL ) {
		pNode->iActions |= XUI_ACCESSIBLE_ACTION_MASK(XUI_ACCESSIBLE_ACTION_TOGGLE);
	}
	return XUI_OK;
}

static const xui_internal_accessibility_adapter_t g_xuiCheckCardAccessible = {
	.getNode = __xuiCheckCardAccessibleNode,
	.performAction = NULL
};

XUI_API xui_widget_type xuiCheckCardGetType(xui_context pContext)
{
	xui_widget_type_desc_t tDesc;
	xui_widget_type pType;
	int iRet;

	if ( !xuiInternalContextIsValid(pContext) ) return NULL;
	pType = xuiWidgetFindType(pContext, "checkcard");
	if ( pType != NULL ) {
		__xuiCheckCardRegisterStyleProperties(pContext, pType);
		pType->pAccessibleAdapter = &g_xuiCheckCardAccessible;
#if XUI_ENABLE_RADIO_GROUP
		pType->onParentChanged = xuiInternalRadioGroupOptionParentChanged;
#endif
		return pType;
	}
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.sName = "checkcard";
	tDesc.pParent = xuiWidgetGetBaseType();
	tDesc.iFlags = XUI_WIDGET_TYPE_DEFAULT_LAYOUT | XUI_WIDGET_TYPE_DEFAULT_CACHE_POLICY;
	tDesc.iTypeDataSize = sizeof(xui_check_card_data_t);
	tDesc.onInit = __xuiCheckCardInit;
	tDesc.onDestroy = __xuiCheckCardDestroy;
	tDesc.onCacheRender = __xuiCheckCardCacheRender;
	__xuiCheckCardDefaultLayout(&tDesc.tLayout);
	__xuiCheckCardDefaultCachePolicy(&tDesc.tCachePolicy);
	iRet = xuiWidgetRegisterType(pContext, &pType, &tDesc);
	if ( iRet != XUI_OK ) return NULL;
	__xuiCheckCardRegisterStyleProperties(pContext, pType);
	pType->pAccessibleAdapter = &g_xuiCheckCardAccessible;
#if XUI_ENABLE_RADIO_GROUP
	pType->onParentChanged = xuiInternalRadioGroupOptionParentChanged;
#endif
	return pType;
}

XUI_API int xuiCheckCardCreate(xui_context pContext, xui_widget* ppWidget, const xui_check_card_desc_t* pDesc)
{
	xui_widget_type pType;

	if ( ppWidget == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	*ppWidget = NULL;
	if ( !xuiInternalContextIsValid(pContext) || !__xuiCheckCardDescValid(pDesc) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pType = xuiCheckCardGetType(pContext);
	if ( pType == NULL ) return XUI_ERROR_NOT_INITIALIZED;
	return xuiWidgetCreateTyped(pContext, pType, ppWidget, pDesc);
}

XUI_API int xuiCheckCardSetChange(xui_widget pWidget, xui_check_card_change_proc onChange, void* pUser)
{
	xui_check_card_data_t* pData = __xuiCheckCardGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	pData->onChange = onChange;
	pData->pChangeUser = pUser;
	return XUI_OK;
}

XUI_API int xuiCheckCardSetChecked(xui_widget pWidget, int bChecked)
{
	xui_check_card_data_t* pData = __xuiCheckCardGetData(pWidget);
	return (pData != NULL) ? __xuiCheckCardSetCheckedInternal(pWidget, pData, bChecked, 0, 0) : XUI_ERROR_INVALID_ARGUMENT;
}

XUI_API int xuiCheckCardSetCheckedFromGroup(xui_widget pWidget, int bChecked, int bNotify)
{
	xui_check_card_data_t* pData = __xuiCheckCardGetData(pWidget);
	return (pData != NULL) ? __xuiCheckCardSetCheckedInternal(pWidget, pData, bChecked, bNotify, 1) : XUI_ERROR_INVALID_ARGUMENT;
}

int xuiInternalCheckCardNotifyFromGroup(xui_widget pWidget)
{
	xui_check_card_data_t* pData = __xuiCheckCardGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	__xuiCheckCardNotify(pWidget, pData);
	return XUI_OK;
}

XUI_API int xuiCheckCardGetChecked(xui_widget pWidget)
{
	xui_check_card_data_t* pData = __xuiCheckCardGetData(pWidget);
	return (pData != NULL) ? pData->bChecked : 0;
}

XUI_API int xuiCheckCardSetRadioGroup(xui_widget pWidget, xui_widget pGroup)
{
	xui_check_card_data_t* pData;
	xui_widget pOldGroup;
	xui_widget pOldParent;
	xui_widget pOldSelected;
	xui_widget pScan;
	int iRet;

	pData = __xuiCheckCardGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	pOldGroup = __xuiCheckCardResolveRadioGroup(pWidget, pData);
	pOldParent = xuiWidgetGetParent(pWidget);
	if ( pGroup == NULL ) {
		if ( (pOldGroup != NULL) && (pOldParent == pOldGroup) ) {
			iRet = xuiInternalRadioGroupRemoveOption(pOldGroup, pWidget, 1);
			if ( iRet != XUI_OK || !__xuiCheckCardDataAlive(pWidget, pData) ) return iRet;
		}
		pData->pRadioGroup = NULL;
		if ( pOldGroup != NULL ) {
			xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_NODE_CHANGED);
		}
		return XUI_OK;
	}
	if ( !__xuiCheckCardIsRadioGroup(pWidget, pGroup) ) return XUI_ERROR_INVALID_ARGUMENT;
	for ( pScan = pGroup; pScan != NULL; pScan = xuiWidgetGetParent(pScan) ) {
		if ( pScan == pWidget ) return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( pOldParent != pGroup ) {
		pOldSelected = (pOldGroup != NULL) ? xuiRadioGroupGetSelectedWidget(pOldGroup) : NULL;
		if ( pOldParent != NULL ) {
			if ( pOldGroup != NULL ) {
				iRet = xuiInternalRadioGroupRemoveOption(pOldGroup, pWidget, 1);
				if ( iRet != XUI_OK || !__xuiCheckCardDataAlive(pWidget, pData) ) return iRet;
				if ( !__xuiCheckCardIsRadioGroup(pWidget, pGroup) ) return XUI_OK;
			} else {
				iRet = xuiWidgetRemoveFromParent(pWidget);
				if ( iRet != XUI_OK ) return iRet;
			}
		}
		iRet = xuiWidgetAddChild(pGroup, pWidget);
		if ( iRet != XUI_OK ) {
			if ( pOldParent != NULL && xuiWidgetAddChild(pOldParent, pWidget) == XUI_OK &&
			     pOldGroup != NULL && pOldSelected != NULL ) {
				(void)xuiRadioGroupSetSelectedWidget(pOldGroup, pOldSelected);
			}
			return iRet;
		}
	}
	pData->pRadioGroup = pGroup;
	if ( pOldGroup != pGroup ) {
		xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_NODE_CHANGED);
	}
	if ( pData->bChecked ) return xuiRadioGroupSetSelectedWidget(pGroup, pWidget);
	return XUI_OK;
}

XUI_API xui_widget xuiCheckCardGetRadioGroup(xui_widget pWidget)
{
	xui_check_card_data_t* pData = __xuiCheckCardGetData(pWidget);
	return (pData != NULL) ? __xuiCheckCardResolveRadioGroup(pWidget, pData) : NULL;
}

XUI_API int xuiCheckCardSetMetrics(xui_widget pWidget, float fBorderWidth, float fCheckedBorderWidth, float fCornerSize, float fFocusWidth)
{
	xui_check_card_data_t* pData = __xuiCheckCardGetData(pWidget);
	int iRet;

	if ( (pData == NULL) || !__xuiCheckCardNonNegativeFloatValid(fBorderWidth) ||
	     !__xuiCheckCardNonNegativeFloatValid(fCheckedBorderWidth) ||
	     !__xuiCheckCardNonNegativeFloatValid(fCornerSize) ||
	     !__xuiCheckCardNonNegativeFloatValid(fFocusWidth) ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( (pData->fBorderWidth == fBorderWidth) &&
	     (pData->fCheckedBorderWidth == fCheckedBorderWidth) &&
	     (pData->fCornerSize == fCornerSize) &&
	     (pData->fFocusWidth == fFocusWidth) ) return XUI_OK;
	iRet = xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	if ( iRet != XUI_OK ) return iRet;
	pData->fBorderWidth = fBorderWidth; pData->fCheckedBorderWidth = fCheckedBorderWidth; pData->fCornerSize = fCornerSize; pData->fFocusWidth = fFocusWidth;
	return XUI_OK;
}

XUI_API int xuiCheckCardGetMetrics(xui_widget pWidget, float* pBorderWidth, float* pCheckedBorderWidth, float* pCornerSize, float* pFocusWidth)
{
	xui_check_card_data_t* pData = __xuiCheckCardGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pBorderWidth != NULL ) *pBorderWidth = pData->fBorderWidth;
	if ( pCheckedBorderWidth != NULL ) *pCheckedBorderWidth = pData->fCheckedBorderWidth;
	if ( pCornerSize != NULL ) *pCornerSize = pData->fCornerSize;
	if ( pFocusWidth != NULL ) *pFocusWidth = pData->fFocusWidth;
	return XUI_OK;
}

XUI_API int xuiCheckCardSetColors(xui_widget pWidget, uint32_t iBackground, uint32_t iHoverBackground, uint32_t iActiveBackground, uint32_t iCheckedBackground, uint32_t iBorder, uint32_t iHoverBorder, uint32_t iCheckedBorder, uint32_t iCorner, uint32_t iCheck)
{
	xui_check_card_data_t* pData = __xuiCheckCardGetData(pWidget);
	int iRet;

	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( (pData->iBackgroundColor == iBackground) &&
	     (pData->iHoverBackgroundColor == iHoverBackground) &&
	     (pData->iActiveBackgroundColor == iActiveBackground) &&
	     (pData->iCheckedBackgroundColor == iCheckedBackground) &&
	     (pData->iBorderColor == iBorder) &&
	     (pData->iHoverBorderColor == iHoverBorder) &&
	     (pData->iCheckedBorderColor == iCheckedBorder) &&
	     (pData->iCornerColor == iCorner) && (pData->iCheckColor == iCheck) ) return XUI_OK;
	iRet = xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	if ( iRet != XUI_OK ) return iRet;
	pData->iBackgroundColor = iBackground; pData->iHoverBackgroundColor = iHoverBackground; pData->iActiveBackgroundColor = iActiveBackground; pData->iCheckedBackgroundColor = iCheckedBackground;
	pData->iBorderColor = iBorder; pData->iHoverBorderColor = iHoverBorder; pData->iCheckedBorderColor = iCheckedBorder; pData->iCornerColor = iCorner; pData->iCheckColor = iCheck;
	return XUI_OK;
}

XUI_API int xuiCheckCardGetColors(xui_widget pWidget, uint32_t* pBackground, uint32_t* pHoverBackground, uint32_t* pActiveBackground, uint32_t* pCheckedBackground, uint32_t* pBorder, uint32_t* pHoverBorder, uint32_t* pCheckedBorder, uint32_t* pCorner, uint32_t* pCheck)
{
	xui_check_card_data_t* pData = __xuiCheckCardGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pBackground != NULL ) *pBackground = pData->iBackgroundColor;
	if ( pHoverBackground != NULL ) *pHoverBackground = pData->iHoverBackgroundColor;
	if ( pActiveBackground != NULL ) *pActiveBackground = pData->iActiveBackgroundColor;
	if ( pCheckedBackground != NULL ) *pCheckedBackground = pData->iCheckedBackgroundColor;
	if ( pBorder != NULL ) *pBorder = pData->iBorderColor;
	if ( pHoverBorder != NULL ) *pHoverBorder = pData->iHoverBorderColor;
	if ( pCheckedBorder != NULL ) *pCheckedBorder = pData->iCheckedBorderColor;
	if ( pCorner != NULL ) *pCorner = pData->iCornerColor;
	if ( pCheck != NULL ) *pCheck = pData->iCheckColor;
	return XUI_OK;
}

XUI_API int xuiCheckCardSetStateColors(xui_widget pWidget, uint32_t iDisabledBorder, uint32_t iFocus)
{
	xui_check_card_data_t* pData = __xuiCheckCardGetData(pWidget);
	int iRet;

	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( (pData->iDisabledBorderColor == iDisabledBorder) && (pData->iFocusColor == iFocus) ) return XUI_OK;
	iRet = xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	if ( iRet != XUI_OK ) return iRet;
	pData->iDisabledBorderColor = iDisabledBorder;
	pData->iFocusColor = iFocus;
	return XUI_OK;
}

XUI_API int xuiCheckCardGetStateColors(xui_widget pWidget, uint32_t* pDisabledBorder, uint32_t* pFocus)
{
	xui_check_card_data_t* pData = __xuiCheckCardGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pDisabledBorder != NULL ) *pDisabledBorder = pData->iDisabledBorderColor;
	if ( pFocus != NULL ) *pFocus = pData->iFocusColor;
	return XUI_OK;
}

XUI_API xui_rect_t xuiCheckCardGetCornerRect(xui_widget pWidget)
{
	xui_check_card_data_t* pData = __xuiCheckCardGetData(pWidget);
	return (pData != NULL) ? pData->tCornerRect : (xui_rect_t){0.0f, 0.0f, 0.0f, 0.0f};
}

XUI_API uint32_t xuiCheckCardGetState(xui_widget pWidget)
{
	xui_check_card_data_t* pData = __xuiCheckCardGetData(pWidget);
	return (pData != NULL) ? __xuiCheckCardComputeState(pWidget, pData) : 0;
}

XUI_API int xuiCheckCardGetChangeCount(xui_widget pWidget)
{
	xui_check_card_data_t* pData = __xuiCheckCardGetData(pWidget);
	return (pData != NULL) ? pData->iChangeCount : 0;
}

#endif
