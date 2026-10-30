#include "xui_config.h"
#if XUI_ENABLE_SLIDER
#include "xui_internal.h"
#include "xui_slider_common.h"

#include <float.h>
#include <limits.h>
#include <math.h>
#include <string.h>
#include <stdio.h>

typedef struct xui_slider_geometry_t {
	xui_slider_common_rect_t tTrack;
	xui_slider_common_rect_t tFill;
	xui_slider_common_rect_t tKnob;
} xui_slider_geometry_t;

typedef struct xui_slider_data_t {
	xui_slider_change_proc onChange;
	void* pChangeUser;
	float fMin;
	float fMax;
	float fValue;
	float fStep;
	float fPageStep;
	float fTrackSize;
	float fKnobSize;
	float fTrackRadius;
	uint32_t iTrackColor;
	uint32_t iFillColor;
	uint32_t iFillHoverColor;
	uint32_t iFillActiveColor;
	uint32_t iKnobColor;
	uint32_t iDisabledKnobColor;
	uint32_t iKnobBorderColor;
	uint32_t iFocusColor;
	uint32_t iDisabledColor;
	int iOrientation;
	int bDragging;
	int bPointerActive;
	uint64_t iActivePointerId;
	int iActivePointerType;
	float fDragOffset;
	int iChangeCount;
} xui_slider_data_t;

static int __xuiSliderOrientationValid(int iOrientation);

static int __xuiSliderMetricValid(float fValue, float fMin)
{
	return xuiSliderCommonMetricValid(fValue, fMin);
}

static int __xuiSliderDescValid(const xui_slider_desc_t* pDesc)
{
	if ( pDesc == NULL ) {
		return 1;
	}
	if ( (pDesc->iSize != 0) && (pDesc->iSize < sizeof(*pDesc)) ) return 0;
	if ( !isfinite(pDesc->fMin) || !isfinite(pDesc->fMax) || !isfinite(pDesc->fValue) ) return 0;
	if ( !isfinite(pDesc->fStep) || pDesc->fStep < 0.0f ||
	     !isfinite(pDesc->fPageStep) || pDesc->fPageStep < 0.0f ) return 0;
	if ( !__xuiSliderMetricValid(pDesc->fTrackSize, 0.0f) ||
	     !__xuiSliderMetricValid(pDesc->fKnobSize, 0.0f) ||
	     !__xuiSliderMetricValid(pDesc->fTrackRadius, -1.0f) ) return 0;
	return __xuiSliderOrientationValid(pDesc->iOrientation);
}

static int __xuiSliderOrientationValid(int iOrientation)
{
	return (iOrientation == XUI_ORIENTATION_HORIZONTAL) || (iOrientation == XUI_ORIENTATION_VERTICAL);
}

static float __xuiSliderClampFloat(float fValue, float fMin, float fMax)
{
	return xuiSliderCommonClamp(fValue, fMin, fMax);
}

static float __xuiSliderAbsFloat(float fValue)
{
	return xuiSliderCommonAbs(fValue);
}

static float __xuiSliderMinFloat(float fA, float fB)
{
	return xuiSliderCommonMin(fA, fB);
}

static float __xuiSliderMaxFloat(float fA, float fB)
{
	return xuiSliderCommonMax(fA, fB);
}

static void __xuiSliderNormalizeRange(float* pMin, float* pMax)
{
	xuiSliderCommonNormalizeRange(pMin, pMax);
}

static uint32_t __xuiSliderColorWithAlpha(uint32_t iColor, uint32_t iAlpha)
{
	if ( (iColor & 0xffu) == 0u ) return iColor;
	return (iColor & 0xffffff00u) | (iAlpha & 0xffu);
}

static int __xuiSliderColorAlpha(uint32_t iColor)
{
	return (int)(iColor & 0xffu);
}

static uint32_t __xuiSliderLighten(uint32_t iColor, int iAmount)
{
	int iR;
	int iG;
	int iB;
	uint32_t iA;

	iR = (int)((iColor >> 24) & 0xffu) + iAmount;
	iG = (int)((iColor >> 16) & 0xffu) + iAmount;
	iB = (int)((iColor >> 8) & 0xffu) + iAmount;
	iA = iColor & 0xffu;
	if ( iR < 0 ) iR = 0;
	if ( iR > 255 ) iR = 255;
	if ( iG < 0 ) iG = 0;
	if ( iG > 255 ) iG = 255;
	if ( iB < 0 ) iB = 0;
	if ( iB > 255 ) iB = 255;
	return XUI_COLOR_RGBA(iR, iG, iB, iA);
}

static float __xuiSliderRateFromData(const xui_slider_data_t* pData)
{
	return pData != NULL ?
		xuiSliderCommonRate(pData->fMin, pData->fMax, pData->fValue) : 0.0f;
}

static float __xuiSliderSmallStep(const xui_slider_data_t* pData)
{
	return pData != NULL ? xuiSliderCommonStep(pData->fMin, pData->fMax,
		pData->fStep, 0.01) : 1.0f;
}

static float __xuiSliderPageStep(const xui_slider_data_t* pData)
{
	return pData != NULL ? xuiSliderCommonStep(pData->fMin, pData->fMax,
		pData->fPageStep, 0.10) : 1.0f;
}

static float __xuiSliderValueWithDelta(const xui_slider_data_t* pData, double fDelta)
{
	if ( pData == NULL ) return 0.0f;
	return xuiSliderCommonValueWithDelta(pData->fValue, pData->fMin,
		pData->fMax, fDelta);
}

static xui_slider_data_t* __xuiSliderGetData(xui_widget pWidget)
{
	xui_context pContext;
	xui_widget_type pType;

	if ( !xuiInternalWidgetIsValid(pWidget) ) {
		return NULL;
	}
	pContext = xuiWidgetGetContext(pWidget);
	pType = xuiWidgetFindType(pContext, "slider");
	if ( (pType == NULL) || !xuiWidgetIsType(pWidget, pType) ) {
		return NULL;
	}
	return (xui_slider_data_t*)xuiWidgetGetTypeData(pWidget);
}

static int __xuiSliderDataAlive(xui_widget pWidget, const xui_slider_data_t* pData)
{
	xui_context pContext;

	if ( (pData == NULL) || (__xuiSliderGetData(pWidget) != pData) ) return 0;
	pContext = xuiWidgetGetContext(pWidget);
	return xuiInternalContextIsValid(pContext) && !xuiInternalContextDestroyPending(pContext);
}

static int __xuiSliderCanInteract(xui_widget pWidget)
{
	return xuiWidgetGetEffectiveEnabled(pWidget) &&
	       xuiWidgetGetEffectiveVisible(pWidget);
}

static void __xuiSliderIncrementCounter(int* pValue)
{
	if ( (pValue != NULL) && (*pValue < INT_MAX) ) {
		(*pValue)++;
	}
}

static int __xuiSliderPointerMatches(const xui_slider_data_t* pData,
	const xui_event_t* pEvent)
{
	return pData != NULL && pEvent != NULL && pData->bPointerActive &&
	       pData->iActivePointerId == pEvent->iPointerId &&
	       pData->iActivePointerType == pEvent->iPointerType;
}

static int __xuiSliderStyleColor(xui_widget pWidget, const char* sName, uint32_t* pColor)
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

static int __xuiSliderStyleFloat(xui_widget pWidget, const char* sName, float* pValue)
{
	xui_style_property_t tProperty;
	int iRet;

	memset(&tProperty, 0, sizeof(tProperty));
	tProperty.iSize = sizeof(tProperty);
	iRet = xuiWidgetGetResolvedStyleProperty(pWidget, sName, &tProperty);
	if ( (iRet == XUI_OK) && (tProperty.tValue.iType == XUI_STYLE_VALUE_FLOAT) &&
	     __xuiSliderMetricValid(tProperty.tValue.fFloat, 0.0f) ) {
		*pValue = tProperty.tValue.fFloat;
		return 1;
	}
	return 0;
}

static void __xuiSliderResolve(xui_widget pWidget, const xui_slider_data_t* pData, xui_slider_data_t* pResolved)
{
	*pResolved = *pData;
	(void)__xuiSliderStyleColor(pWidget, "slider.track.color", &pResolved->iTrackColor);
	(void)__xuiSliderStyleColor(pWidget, "slider.fill.color", &pResolved->iFillColor);
	(void)__xuiSliderStyleColor(pWidget, "slider.fill.hover_color", &pResolved->iFillHoverColor);
	(void)__xuiSliderStyleColor(pWidget, "slider.fill.active_color", &pResolved->iFillActiveColor);
	(void)__xuiSliderStyleColor(pWidget, "slider.knob.color", &pResolved->iKnobColor);
	pResolved->iDisabledKnobColor = XUI_COLOR_RGBA(232, 237, 244, 255);
	(void)__xuiSliderStyleColor(pWidget, "slider.knob.disabled_color", &pResolved->iDisabledKnobColor);
	(void)__xuiSliderStyleColor(pWidget, "slider.knob.border_color", &pResolved->iKnobBorderColor);
	(void)__xuiSliderStyleColor(pWidget, "slider.focus.color", &pResolved->iFocusColor);
	(void)__xuiSliderStyleColor(pWidget, "slider.disabled.color", &pResolved->iDisabledColor);
	(void)__xuiSliderStyleFloat(pWidget, "slider.track.size", &pResolved->fTrackSize);
	(void)__xuiSliderStyleFloat(pWidget, "slider.knob.size", &pResolved->fKnobSize);
	(void)__xuiSliderStyleFloat(pWidget, "slider.track.radius", &pResolved->fTrackRadius);
	if ( pResolved->fTrackSize <= 0.0f ) {
		pResolved->fTrackSize = 4.0f;
	}
	if ( pResolved->fKnobSize <= 0.0f ) {
		pResolved->fKnobSize = 14.0f;
	}
	if ( pResolved->fTrackRadius < -1.0f ) {
		pResolved->fTrackRadius = -1.0f;
	}
}

static uint32_t __xuiSliderBaseState(xui_widget pWidget, const xui_slider_data_t* pData)
{
	uint32_t iState;

	iState = xuiWidgetGetInputState(pWidget);
	if ( !xuiWidgetGetEffectiveEnabled(pWidget) ) {
		iState |= XUI_WIDGET_STATE_DISABLED;
	}
	if ( (iState & XUI_WIDGET_STATE_DISABLED) != 0 ) {
		return XUI_WIDGET_STATE_DISABLED;
	}
	iState &= (XUI_WIDGET_STATE_HOVER | XUI_WIDGET_STATE_FOCUS);
	if ( (pData != NULL) && (pData->bDragging != 0) ) {
		iState |= XUI_WIDGET_STATE_ACTIVE;
	}
	return iState;
}

static uint32_t __xuiSliderStateId(xui_widget pWidget, const xui_slider_data_t* pData)
{
	uint32_t iState;

	iState = __xuiSliderBaseState(pWidget, pData);
	if ( (iState & XUI_WIDGET_STATE_DISABLED) != 0 ) {
		return XUI_WIDGET_STATE_DISABLED;
	}
	if ( (iState & XUI_WIDGET_STATE_ACTIVE) != 0 ) {
		return XUI_WIDGET_STATE_ACTIVE;
	}
	if ( (iState & XUI_WIDGET_STATE_HOVER) != 0 ) {
		return XUI_WIDGET_STATE_HOVER;
	}
	if ( (iState & XUI_WIDGET_STATE_FOCUS) != 0 ) {
		return XUI_WIDGET_STATE_FOCUS;
	}
	return 0;
}

static int __xuiSliderSyncState(xui_widget pWidget, xui_slider_data_t* pData)
{
	if ( (pWidget == NULL) || (pData == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	return xuiWidgetSetStateId(pWidget, __xuiSliderStateId(pWidget, pData));
}

static int __xuiSliderRectContains(xui_rect_t tRect, float fX, float fY)
{
	return (tRect.fW > 0.0f) &&
	       (tRect.fH > 0.0f) &&
	       (fX >= tRect.fX) &&
	       (fY >= tRect.fY) &&
	       (fX < (tRect.fX + tRect.fW)) &&
	       (fY < (tRect.fY + tRect.fH));
}

static xui_slider_common_rect_t __xuiSliderTrackRect(
	const xui_slider_data_t* pData, xui_rect_t tContent)
{
	if ( pData == NULL ) return (xui_slider_common_rect_t){0};
	return xuiSliderCommonTrackRect(tContent, pData->fTrackSize,
		pData->fKnobSize, pData->iOrientation);
}

static xui_slider_common_rect_t __xuiSliderFillRect(
	const xui_slider_data_t* pData, xui_slider_common_rect_t tTrack)
{
	xui_slider_common_rect_t tFill;
	float fRate;

	tFill = tTrack;
	fRate = __xuiSliderRateFromData(pData);
	if ( pData->iOrientation == XUI_ORIENTATION_HORIZONTAL ) {
		tFill.fW = tTrack.fW * fRate;
	} else {
		tFill.fH = tTrack.fH * fRate;
		tFill.fY = tTrack.fY + tTrack.fH - tFill.fH;
	}
	return tFill;
}

static xui_slider_common_rect_t __xuiSliderKnobRect(
	const xui_slider_data_t* pData, xui_rect_t tContent,
	xui_slider_common_rect_t tTrack)
{
	if ( pData == NULL ) return (xui_slider_common_rect_t){0};
	return xuiSliderCommonKnobRect(tContent, tTrack, pData->fKnobSize,
		pData->iOrientation, __xuiSliderRateFromData(pData));
}

static void __xuiSliderComputeGeometry(xui_widget pWidget,
	const xui_slider_data_t* pResolved, xui_slider_geometry_t* pGeometry)
{
	xui_rect_t tContent;

	memset(pGeometry, 0, sizeof(*pGeometry));
	tContent = xuiWidgetGetContentRect(pWidget);
	pGeometry->tTrack = __xuiSliderTrackRect(pResolved, tContent);
	pGeometry->tFill = __xuiSliderFillRect(pResolved, pGeometry->tTrack);
	pGeometry->tKnob = __xuiSliderKnobRect(pResolved, tContent, pGeometry->tTrack);
}

static xui_rect_t __xuiSliderOutputRect(xui_slider_common_rect_t tRect)
{
	return xuiSliderCommonOutputRect(tRect);
}

static int __xuiSliderFloatRectContains(xui_slider_common_rect_t tRect,
	float fX, float fY)
{
	return xuiSliderCommonRectContains(tRect, fX, fY);
}

static float __xuiSliderValueFromLocalPoint(const xui_slider_data_t* pData,
	xui_slider_common_rect_t tTrack, float fLocalX, float fLocalY)
{
	if ( pData == NULL ) return 0.0f;
	return xuiSliderCommonValueFromPoint(pData->fValue, pData->fMin,
		pData->fMax, pData->iOrientation, tTrack, fLocalX, fLocalY);
}

static int __xuiSliderSetValueInternal(xui_widget pWidget, xui_slider_data_t* pData, float fValue, int bNotify)
{
	xui_slider_change_proc onChange;
	void* pChangeUser;
	int iRet;

	if ( (pWidget == NULL) || (pData == NULL) || !isfinite(fValue) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	fValue = __xuiSliderClampFloat(fValue, pData->fMin, pData->fMax);
	if ( pData->fValue == fValue ) {
		return XUI_OK;
	}
	iRet = xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	if ( iRet != XUI_OK ) return iRet;
	pData->fValue = fValue;
	xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_VALUE_CHANGED);
	__xuiSliderIncrementCounter(&pData->iChangeCount);
	onChange = pData->onChange;
	pChangeUser = pData->pChangeUser;
	if ( bNotify && (onChange != NULL) ) {
		onChange(pWidget, fValue, pChangeUser);
	}
	return XUI_OK;
}

static int __xuiSliderSetValueFromEvent(xui_widget pWidget, xui_slider_data_t* pData, const xui_event_t* pEvent, int bNotify)
{
	xui_slider_data_t tResolved;
	xui_slider_geometry_t tGeometry;
	xui_rect_t tWorld;
	float fLocalX;
	float fLocalY;

	if ( (pWidget == NULL) || (pData == NULL) || (pEvent == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	__xuiSliderResolve(pWidget, pData, &tResolved);
	__xuiSliderComputeGeometry(pWidget, &tResolved, &tGeometry);
	tWorld = xuiWidgetGetWorldRect(pWidget);
	fLocalX = pEvent->fX - tWorld.fX;
	fLocalY = pEvent->fY - tWorld.fY;
	if ( pData->iOrientation == XUI_ORIENTATION_HORIZONTAL ) {
		fLocalX -= pData->fDragOffset;
	} else {
		fLocalY -= pData->fDragOffset;
	}
	return __xuiSliderSetValueInternal(pWidget, pData,
		__xuiSliderValueFromLocalPoint(pData, tGeometry.tTrack, fLocalX, fLocalY), bNotify);
}

static float __xuiSliderGrabOffset(xui_widget pWidget,
	const xui_slider_data_t* pData, const xui_event_t* pEvent)
{
	xui_slider_data_t tResolved;
	xui_slider_geometry_t tGeometry;
	xui_rect_t tWorld;
	float fLocalX;
	float fLocalY;

	__xuiSliderResolve(pWidget, pData, &tResolved);
	__xuiSliderComputeGeometry(pWidget, &tResolved, &tGeometry);
	tWorld = xuiWidgetGetWorldRect(pWidget);
	fLocalX = pEvent->fX - tWorld.fX;
	fLocalY = pEvent->fY - tWorld.fY;
	if ( !__xuiSliderFloatRectContains(tGeometry.tKnob, fLocalX, fLocalY) ) {
		return 0.0f;
	}
	if ( pData->iOrientation == XUI_ORIENTATION_HORIZONTAL ) {
		return fLocalX - (tGeometry.tKnob.fX + tGeometry.tKnob.fW * 0.5f);
	}
	return fLocalY - (tGeometry.tKnob.fY + tGeometry.tKnob.fH * 0.5f);
}

static void __xuiSliderCancelDrag(xui_widget pWidget, xui_slider_data_t* pData)
{
	xui_context pContext;
	uint64_t iPointerId;
	int iPointerType;
	int bPointerActive;

	if ( (pWidget == NULL) || (pData == NULL) ) {
		return;
	}
	pContext = xuiWidgetGetContext(pWidget);
	iPointerId = pData->iActivePointerId;
	iPointerType = pData->iActivePointerType;
	bPointerActive = pData->bPointerActive;
	pData->bDragging = 0;
	pData->bPointerActive = 0;
	pData->iActivePointerId = 0;
	pData->iActivePointerType = XUI_POINTER_TYPE_MOUSE;
	pData->fDragOffset = 0.0f;
	(void)__xuiSliderSyncState(pWidget, pData);
	if ( bPointerActive && xuiInternalContextIsValid(pContext) &&
	     xuiGetPointerCaptureEx(pContext, iPointerId, iPointerType) == pWidget ) {
		(void)xuiReleasePointerCaptureEx(pContext, iPointerId, iPointerType, pWidget);
	}
}

static void __xuiSliderCancelAction(xui_widget pWidget, void* pUser)
{
	xui_slider_data_t* pData;

	(void)pUser;
	pData = __xuiSliderGetData(pWidget);
	__xuiSliderCancelDrag(pWidget, pData);
}

static int __xuiSliderPointerInside(xui_widget pWidget, const xui_event_t* pEvent)
{
	xui_rect_t tWorld;
	xui_rect_t tContent;
	float fLocalX;
	float fLocalY;

	if ( (pWidget == NULL) || (pEvent == NULL) ) {
		return 0;
	}
	tWorld = xuiWidgetGetWorldRect(pWidget);
	tContent = xuiWidgetGetContentRect(pWidget);
	fLocalX = pEvent->fX - tWorld.fX;
	fLocalY = pEvent->fY - tWorld.fY;
	return __xuiSliderRectContains(tContent, fLocalX, fLocalY);
}

static int __xuiSliderPointerDown(xui_widget pWidget, xui_slider_data_t* pData, const xui_event_t* pEvent)
{
	xui_context pContext;
	float fDragOffset;
	int iRet;

	if ( pEvent->iButton != XUI_POINTER_BUTTON_LEFT ) {
		return XUI_OK;
	}
	if ( !__xuiSliderCanInteract(pWidget) ||
	     !__xuiSliderPointerInside(pWidget, pEvent) ) {
		return XUI_OK;
	}
	if ( pData->bPointerActive ) {
		return __xuiSliderPointerMatches(pData, pEvent) ? XUI_EVENT_DISPATCH_STOP : XUI_OK;
	}
	pContext = xuiWidgetGetContext(pWidget);
	iRet = xuiSetFocusWidget(pContext, pWidget);
	if ( iRet != XUI_OK ) return iRet;
	if ( !__xuiSliderDataAlive(pWidget, pData) ) return XUI_EVENT_DISPATCH_STOP;
	if ( !__xuiSliderCanInteract(pWidget) ) {
		return XUI_EVENT_DISPATCH_STOP;
	}
	fDragOffset = __xuiSliderGrabOffset(pWidget, pData, pEvent);
	iRet = xuiSetPointerCaptureEx(pContext, pEvent->iPointerId, pEvent->iPointerType, pWidget);
	if ( iRet != XUI_OK ) return iRet;
	if ( !__xuiSliderDataAlive(pWidget, pData) ) return XUI_EVENT_DISPATCH_STOP;
	pData->bPointerActive = 1;
	pData->iActivePointerId = pEvent->iPointerId;
	pData->iActivePointerType = pEvent->iPointerType;
	pData->fDragOffset = fDragOffset;
	pData->bDragging = 1;
	if ( !__xuiSliderCanInteract(pWidget) ) {
		__xuiSliderCancelDrag(pWidget, pData);
		return XUI_EVENT_DISPATCH_STOP;
	}
	iRet = __xuiSliderSyncState(pWidget, pData);
	if ( iRet != XUI_OK ) {
		__xuiSliderCancelDrag(pWidget, pData);
		return iRet;
	}
	iRet = __xuiSliderSetValueFromEvent(pWidget, pData, pEvent, 1);
	if ( iRet != XUI_OK ) {
		__xuiSliderCancelDrag(pWidget, pData);
		return iRet;
	}
	if ( !__xuiSliderDataAlive(pWidget, pData) ) return XUI_EVENT_DISPATCH_STOP;
	if ( !__xuiSliderCanInteract(pWidget) ) {
		__xuiSliderCancelDrag(pWidget, pData);
	}
	return XUI_EVENT_DISPATCH_STOP;
}

static int __xuiSliderPointerMove(xui_widget pWidget, xui_slider_data_t* pData, const xui_event_t* pEvent)
{
	xui_context pContext;
	int iRet;

	if ( pData->bPointerActive && !__xuiSliderPointerMatches(pData, pEvent) ) return XUI_OK;
	pContext = xuiWidgetGetContext(pWidget);
	if ( (pData->bDragging != 0) &&
	     (xuiGetPointerCaptureEx(pContext, pEvent->iPointerId, pEvent->iPointerType) == pWidget) ) {
		if ( !__xuiSliderCanInteract(pWidget) ) {
			__xuiSliderCancelDrag(pWidget, pData);
			return XUI_EVENT_DISPATCH_STOP;
		}
		iRet = __xuiSliderSetValueFromEvent(pWidget, pData, pEvent, 1);
		if ( iRet != XUI_OK ) return iRet;
		if ( !__xuiSliderDataAlive(pWidget, pData) ) return XUI_EVENT_DISPATCH_STOP;
		if ( !__xuiSliderCanInteract(pWidget) ) {
			__xuiSliderCancelDrag(pWidget, pData);
		}
		return XUI_EVENT_DISPATCH_STOP;
	}
	return __xuiSliderSyncState(pWidget, pData);
}

static int __xuiSliderPointerUp(xui_widget pWidget, xui_slider_data_t* pData, const xui_event_t* pEvent)
{
	xui_context pContext;
	int bWasDragging;
	int iRet;

	if ( pEvent->iButton != XUI_POINTER_BUTTON_LEFT ) {
		return XUI_OK;
	}
	if ( !__xuiSliderPointerMatches(pData, pEvent) ) return XUI_OK;
	pContext = xuiWidgetGetContext(pWidget);
	bWasDragging = (pData->bDragging != 0) ||
		(xuiGetPointerCaptureEx(pContext, pEvent->iPointerId, pEvent->iPointerType) == pWidget);
	if ( bWasDragging ) {
		if ( __xuiSliderCanInteract(pWidget) ) {
			iRet = __xuiSliderSetValueFromEvent(pWidget, pData, pEvent, 1);
			if ( iRet != XUI_OK ) {
				if ( __xuiSliderDataAlive(pWidget, pData) ) {
					__xuiSliderCancelDrag(pWidget, pData);
				}
				return iRet;
			}
			if ( !__xuiSliderDataAlive(pWidget, pData) ) return XUI_EVENT_DISPATCH_STOP;
		}
	}
	__xuiSliderCancelDrag(pWidget, pData);
	return bWasDragging ? XUI_EVENT_DISPATCH_STOP : XUI_OK;
}

static int __xuiSliderWheel(xui_widget pWidget, xui_slider_data_t* pData, const xui_event_t* pEvent)
{
	float fDelta;
	int iRet;

	if ( !__xuiSliderCanInteract(pWidget) ) {
		return XUI_OK;
	}
	fDelta = (__xuiSliderAbsFloat(pEvent->fWheelY) >= __xuiSliderAbsFloat(pEvent->fWheelX)) ? pEvent->fWheelY : pEvent->fWheelX;
	if ( !isfinite(fDelta) || fDelta == 0.0f ) {
		return XUI_OK;
	}
	iRet = __xuiSliderSetValueInternal(pWidget, pData,
		__xuiSliderValueWithDelta(pData,
			(double)fDelta * (double)__xuiSliderSmallStep(pData)), 1);
	if ( iRet != XUI_OK ) return iRet;
	return XUI_EVENT_DISPATCH_STOP;
}

static int __xuiSliderKeyDown(xui_widget pWidget, xui_slider_data_t* pData, const xui_event_t* pEvent)
{
	float fValue;
	int iRet;

	if ( !__xuiSliderCanInteract(pWidget) ) {
		return XUI_OK;
	}
	switch ( pEvent->iKey ) {
	case XUI_KEY_ESCAPE:
		__xuiSliderCancelDrag(pWidget, pData);
		return XUI_EVENT_DISPATCH_STOP;
	case XUI_KEY_HOME:
		fValue = pData->fMin;
		break;
	case XUI_KEY_END:
		fValue = pData->fMax;
		break;
	case XUI_KEY_PAGE_DOWN:
		fValue = __xuiSliderValueWithDelta(pData,
			-(double)__xuiSliderPageStep(pData));
		break;
	case XUI_KEY_PAGE_UP:
		fValue = __xuiSliderValueWithDelta(pData,
			(double)__xuiSliderPageStep(pData));
		break;
	case XUI_KEY_LEFT:
	case XUI_KEY_DOWN:
		fValue = __xuiSliderValueWithDelta(pData,
			-(double)__xuiSliderSmallStep(pData));
		break;
	case XUI_KEY_RIGHT:
	case XUI_KEY_UP:
		fValue = __xuiSliderValueWithDelta(pData,
			(double)__xuiSliderSmallStep(pData));
		break;
	default:
		return XUI_OK;
	}
	iRet = __xuiSliderSetValueInternal(pWidget, pData, fValue, 1);
	if ( iRet != XUI_OK ) return iRet;
	return XUI_EVENT_DISPATCH_STOP;
}

static int __xuiSliderEvent(xui_widget pWidget, const xui_event_t* pEvent, void* pUser)
{
	xui_slider_data_t* pData;
	int iRet;

	(void)pUser;
	if ( (pWidget == NULL) || (pEvent == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pData = __xuiSliderGetData(pWidget);
	if ( pData == NULL ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	switch ( pEvent->iType ) {
	case XUI_EVENT_POINTER_ENTER:
	case XUI_EVENT_POINTER_MOVE:
		return __xuiSliderPointerMove(pWidget, pData, pEvent);
	case XUI_EVENT_POINTER_LEAVE:
		if ( pData->bDragging == 0 ) {
			return __xuiSliderSyncState(pWidget, pData);
		}
		break;
	case XUI_EVENT_POINTER_DOWN:
		return __xuiSliderPointerDown(pWidget, pData, pEvent);
	case XUI_EVENT_POINTER_UP:
		return __xuiSliderPointerUp(pWidget, pData, pEvent);
	case XUI_EVENT_POINTER_CLICK:
		if ( pEvent->iButton == XUI_POINTER_BUTTON_LEFT ) {
			return XUI_EVENT_DISPATCH_STOP;
		}
		break;
	case XUI_EVENT_POINTER_WHEEL:
		return __xuiSliderWheel(pWidget, pData, pEvent);
	case XUI_EVENT_POINTER_CAPTURE_LOST:
		if ( __xuiSliderPointerMatches(pData, pEvent) ) {
			pData->bDragging = 0;
			pData->bPointerActive = 0;
			pData->iActivePointerId = 0;
			pData->iActivePointerType = XUI_POINTER_TYPE_MOUSE;
			pData->fDragOffset = 0.0f;
			return __xuiSliderSyncState(pWidget, pData);
		}
		return XUI_OK;
	case XUI_EVENT_FOCUS:
		return __xuiSliderSyncState(pWidget, pData);
	case XUI_EVENT_BLUR:
		__xuiSliderCancelDrag(pWidget, pData);
		return __xuiSliderSyncState(pWidget, pData);
	case XUI_EVENT_KEY_DOWN:
		return __xuiSliderKeyDown(pWidget, pData, pEvent);
	case XUI_EVENT_ENABLED_CHANGED:
	case XUI_EVENT_VISIBLE_CHANGED:
		if ( !__xuiSliderCanInteract(pWidget) ) {
			__xuiSliderCancelDrag(pWidget, pData);
		}
		iRet = __xuiSliderSyncState(pWidget, pData);
		if ( iRet != XUI_OK ) return iRet;
		return xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	default:
		break;
	}
	return XUI_OK;
}

static int __xuiSliderDrawRectFill(xui_proxy pProxy, xui_draw_context pDraw, xui_rect_t tRect, uint32_t iColor)
{
	if ( (tRect.fW <= 0.0f) || (tRect.fH <= 0.0f) || (__xuiSliderColorAlpha(iColor) == 0) ) {
		return XUI_OK;
	}
	return pProxy->drawRectFill(pProxy, pDraw, tRect, iColor);
}

static int __xuiSliderDrawRectStroke(xui_proxy pProxy, xui_draw_context pDraw, xui_rect_t tRect, float fWidth, uint32_t iColor)
{
	if ( (tRect.fW <= 0.0f) || (tRect.fH <= 0.0f) || (fWidth <= 0.0f) || (__xuiSliderColorAlpha(iColor) == 0) ) {
		return XUI_OK;
	}
	return pProxy->drawRectStroke(pProxy, pDraw, tRect, fWidth, iColor);
}

static int __xuiSliderDrawTrackFill(xui_proxy pProxy, xui_draw_context pDraw, xui_rect_t tRect, float fRadius, uint32_t iColor)
{
	xui_rect_t tBody;
	float fMaxRadius;
	float fCX0;
	float fCY0;
	float fCX1;
	float fCY1;
	int iRet;

	if ( (tRect.fW <= 0.0f) || (tRect.fH <= 0.0f) || (__xuiSliderColorAlpha(iColor) == 0) ) {
		return XUI_OK;
	}
	fMaxRadius = __xuiSliderMinFloat(tRect.fW, tRect.fH) * 0.5f;
	if ( fRadius < 0.0f ) fRadius = fMaxRadius;
	if ( fRadius > fMaxRadius ) fRadius = fMaxRadius;
	if ( (fRadius <= 0.0f) || (pProxy->drawCircleFill == NULL) ) {
		return __xuiSliderDrawRectFill(pProxy, pDraw, tRect, iColor);
	}
	if ( tRect.fW >= tRect.fH ) {
		tBody = xuiInternalRectFromFloatNearest(tRect.fX + fRadius, tRect.fY,
			tRect.fW - fRadius * 2.0f, tRect.fH);
		fCX0 = tRect.fX + fRadius;
		fCY0 = tRect.fY + tRect.fH * 0.5f;
		fCX1 = tRect.fX + tRect.fW - fRadius;
		fCY1 = fCY0;
	} else {
		tBody = xuiInternalRectFromFloatNearest(tRect.fX, tRect.fY + fRadius,
			tRect.fW, tRect.fH - fRadius * 2.0f);
		fCX0 = tRect.fX + tRect.fW * 0.5f;
		fCY0 = tRect.fY + fRadius;
		fCX1 = fCX0;
		fCY1 = tRect.fY + tRect.fH - fRadius;
	}
	if ( (tBody.fW > 0.0f) && (tBody.fH > 0.0f) ) {
		iRet = __xuiSliderDrawRectFill(pProxy, pDraw, tBody, iColor);
		if ( iRet != XUI_OK ) return iRet;
	}
	iRet = pProxy->drawCircleFill(pProxy, pDraw, fCX0, fCY0, fRadius, iColor);
	if ( iRet != XUI_OK ) return iRet;
	return pProxy->drawCircleFill(pProxy, pDraw, fCX1, fCY1, fRadius, iColor);
}

static int __xuiSliderDrawCircleFill(xui_proxy pProxy, xui_draw_context pDraw, xui_rect_t tRect, uint32_t iColor)
{
	float fRadius;

	if ( (tRect.fW <= 0.0f) || (tRect.fH <= 0.0f) || (__xuiSliderColorAlpha(iColor) == 0) ) {
		return XUI_OK;
	}
	fRadius = __xuiSliderMinFloat(tRect.fW, tRect.fH) * 0.5f;
	if ( pProxy->drawCircleFill != NULL ) {
		return pProxy->drawCircleFill(pProxy, pDraw, tRect.fX + tRect.fW * 0.5f, tRect.fY + tRect.fH * 0.5f, fRadius, iColor);
	}
	return __xuiSliderDrawRectFill(pProxy, pDraw, tRect, iColor);
}

static int __xuiSliderDrawCircleStroke(xui_proxy pProxy, xui_draw_context pDraw, xui_rect_t tRect, float fWidth, uint32_t iColor)
{
	float fRadius;

	if ( (tRect.fW <= 0.0f) || (tRect.fH <= 0.0f) || (fWidth <= 0.0f) || (__xuiSliderColorAlpha(iColor) == 0) ) {
		return XUI_OK;
	}
	fRadius = __xuiSliderMinFloat(tRect.fW, tRect.fH) * 0.5f - fWidth * 0.5f;
	if ( fRadius <= 0.0f ) {
		return XUI_OK;
	}
	if ( pProxy->drawCircleStroke != NULL ) {
		return pProxy->drawCircleStroke(pProxy, pDraw, tRect.fX + tRect.fW * 0.5f, tRect.fY + tRect.fH * 0.5f, fRadius, fWidth, iColor);
	}
	return __xuiSliderDrawRectStroke(pProxy, pDraw, tRect, fWidth, iColor);
}

static uint32_t __xuiSliderFillForState(const xui_slider_data_t* pData, uint32_t iStateId)
{
	if ( (iStateId & XUI_WIDGET_STATE_DISABLED) != 0 ) {
		return pData->iDisabledColor;
	}
	if ( (iStateId & XUI_WIDGET_STATE_ACTIVE) != 0 ) {
		return pData->iFillActiveColor;
	}
	if ( (iStateId & XUI_WIDGET_STATE_HOVER) != 0 ) {
		return pData->iFillHoverColor;
	}
	return pData->iFillColor;
}

static int __xuiSliderCacheRender(xui_widget pWidget, xui_draw_context pDraw, uint32_t iStateId, void* pUser)
{
	xui_slider_data_t* pData;
	xui_slider_data_t tResolved;
	xui_slider_geometry_t tGeometry;
	xui_proxy pProxy;
	xui_rect_t tContent;
	xui_rect_t tFocus;
	xui_rect_t tTrack;
	xui_rect_t tFill;
	xui_rect_t tKnob;
	uint32_t iTrackColor;
	uint32_t iFillColor;
	uint32_t iKnobColor;
	uint32_t iBorderColor;
	float fTrackRadius;
	int iRet;

	(void)pUser;
	pData = __xuiSliderGetData(pWidget);
	if ( (pData == NULL) || (pDraw == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pProxy = xuiInternalContextGetProxy(xuiWidgetGetContext(pWidget));
	if ( pProxy == NULL ) {
		return XUI_ERROR_NOT_INITIALIZED;
	}
	__xuiSliderResolve(pWidget, pData, &tResolved);
	__xuiSliderComputeGeometry(pWidget, &tResolved, &tGeometry);
	tTrack = __xuiSliderOutputRect(tGeometry.tTrack);
	tFill = __xuiSliderOutputRect(tGeometry.tFill);
	tKnob = __xuiSliderOutputRect(tGeometry.tKnob);
	tContent = xuiWidgetGetContentRect(pWidget);
	iTrackColor = ((iStateId & XUI_WIDGET_STATE_DISABLED) != 0) ? __xuiSliderColorWithAlpha(tResolved.iDisabledColor, 72) : tResolved.iTrackColor;
	iFillColor = __xuiSliderFillForState(&tResolved, iStateId);
	iKnobColor = ((iStateId & XUI_WIDGET_STATE_DISABLED) != 0) ? tResolved.iDisabledKnobColor : tResolved.iKnobColor;
	iBorderColor = ((iStateId & XUI_WIDGET_STATE_DISABLED) != 0) ? __xuiSliderColorWithAlpha(tResolved.iKnobBorderColor, 105) : tResolved.iKnobBorderColor;
	fTrackRadius = (tResolved.fTrackRadius >= 0.0f) ? tResolved.fTrackRadius :
		(__xuiSliderMinFloat(tGeometry.tTrack.fW, tGeometry.tTrack.fH) * 0.5f);
	iRet = __xuiSliderDrawTrackFill(pProxy, pDraw, tTrack, fTrackRadius, iTrackColor);
	if ( iRet != XUI_OK ) return iRet;
	iRet = __xuiSliderDrawTrackFill(pProxy, pDraw, tFill,
		__xuiSliderMinFloat(fTrackRadius,
			__xuiSliderMinFloat(tGeometry.tFill.fW, tGeometry.tFill.fH) * 0.5f),
		iFillColor);
	if ( iRet != XUI_OK ) return iRet;
	iRet = __xuiSliderDrawCircleFill(pProxy, pDraw, tKnob, iKnobColor);
	if ( iRet == XUI_OK ) {
		iRet = __xuiSliderDrawCircleStroke(pProxy, pDraw, tKnob, 1.0f, iBorderColor);
	}
	if ( iRet != XUI_OK ) return iRet;
	if ( ((iStateId & XUI_WIDGET_STATE_FOCUS) != 0) && ((iStateId & XUI_WIDGET_STATE_DISABLED) == 0) ) {
		tFocus = xuiInternalInsetRect(tContent, 1.0f);
		iRet = __xuiSliderDrawRectStroke(pProxy, pDraw, tFocus, 1.5f, tResolved.iFocusColor);
		if ( iRet != XUI_OK ) return iRet;
	}
	return XUI_OK;
}

static int __xuiSliderContentMeasure(xui_widget pWidget, xui_vec2_t tConstraint, xui_vec2_t* pSize, void* pUser)
{
	xui_slider_data_t* pData;
	xui_slider_data_t tResolved;
	float fCross;
	float fLong;

	(void)tConstraint;
	(void)pUser;
	if ( pSize == NULL ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pData = __xuiSliderGetData(pWidget);
	if ( pData == NULL ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	__xuiSliderResolve(pWidget, pData, &tResolved);
	fCross = __xuiSliderMaxFloat(tResolved.fKnobSize, tResolved.fTrackSize);
	if ( fCross < 18.0f ) fCross = 18.0f;
	fLong = __xuiSliderMaxFloat(tResolved.fKnobSize * 8.0f, 144.0f);
	if ( fCross > XUI_LAYOUT_UNBOUNDED ) fCross = XUI_LAYOUT_UNBOUNDED;
	if ( fLong > XUI_LAYOUT_UNBOUNDED ) fLong = XUI_LAYOUT_UNBOUNDED;
	if ( tResolved.iOrientation == XUI_ORIENTATION_HORIZONTAL ) {
		*pSize = (xui_vec2_t){fLong, fCross};
	} else {
		*pSize = (xui_vec2_t){fCross, fLong};
	}
	return XUI_OK;
}

static void __xuiSliderDefaultLayout(xui_layout_t* pLayout)
{
	memset(pLayout, 0, sizeof(*pLayout));
	pLayout->iLayoutType = XUI_LAYOUT_MANUAL;
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

static void __xuiSliderDefaultCachePolicy(xui_cache_policy_t* pPolicy)
{
	memset(pPolicy, 0, sizeof(*pPolicy));
	pPolicy->iSize = sizeof(*pPolicy);
	pPolicy->iPolicy = XUI_CACHE_POLICY_SELF;
	pPolicy->iFlags = XUI_CACHE_CLEAR_ON_UPDATE | XUI_CACHE_UPDATE_ALL_STATES;
	pPolicy->iClearColor = XUI_COLOR_RGBA(0, 0, 0, 0);
}

static int __xuiSliderInitCacheStates(xui_widget pWidget)
{
	static const uint32_t arrState[] = {
		0,
		XUI_WIDGET_STATE_HOVER,
		XUI_WIDGET_STATE_ACTIVE,
		XUI_WIDGET_STATE_FOCUS,
		XUI_WIDGET_STATE_DISABLED
	};
	int i;
	int iRet;

	iRet = xuiWidgetSetCacheStateCount(pWidget, (int)(sizeof(arrState) / sizeof(arrState[0])));
	if ( iRet != XUI_OK ) return iRet;
	for ( i = 0; i < (int)(sizeof(arrState) / sizeof(arrState[0])); i++ ) {
		iRet = xuiWidgetSetCacheStateId(pWidget, i, 0x79000000u + (uint32_t)i);
		if ( iRet != XUI_OK ) return iRet;
	}
	for ( i = 0; i < (int)(sizeof(arrState) / sizeof(arrState[0])); i++ ) {
		iRet = xuiWidgetSetCacheStateId(pWidget, i, arrState[i]);
		if ( iRet != XUI_OK ) return iRet;
	}
	return XUI_OK;
}

static int __xuiSliderInitEvents(xui_widget pWidget)
{
	int iRet;

	iRet = xuiWidgetSetCancelAction(pWidget, __xuiSliderCancelAction, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_POINTER_ENTER, __xuiSliderEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_POINTER_LEAVE, __xuiSliderEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_POINTER_MOVE, __xuiSliderEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_POINTER_DOWN, __xuiSliderEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_POINTER_UP, __xuiSliderEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_POINTER_CLICK, __xuiSliderEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_POINTER_WHEEL, __xuiSliderEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_POINTER_CAPTURE_LOST, __xuiSliderEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_FOCUS, __xuiSliderEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_BLUR, __xuiSliderEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_KEY_DOWN, __xuiSliderEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_ENABLED_CHANGED, __xuiSliderEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_VISIBLE_CHANGED, __xuiSliderEvent, NULL);
	return iRet;
}

static int __xuiSliderInit(xui_widget pWidget, void* pTypeData, const void* pCreateData, void* pUser)
{
	xui_slider_data_t* pData;
	const xui_slider_desc_t* pDesc;
	float fMin;
	float fMax;
	uint32_t iFill;
	int iRet;

	(void)pUser;
	pData = (xui_slider_data_t*)pTypeData;
	pDesc = (const xui_slider_desc_t*)pCreateData;
	if ( (pWidget == NULL) || (pData == NULL) || !__xuiSliderDescValid(pDesc) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	memset(pData, 0, sizeof(*pData));
	pData->iActivePointerType = XUI_POINTER_TYPE_MOUSE;
	fMin = (pDesc != NULL) ? pDesc->fMin : 0.0f;
	fMax = (pDesc != NULL) ? pDesc->fMax : 1.0f;
	if ( (pDesc == NULL) || (fMin == 0.0f && fMax == 0.0f) ) {
		fMax = 1.0f;
	}
	__xuiSliderNormalizeRange(&fMin, &fMax);
	iFill = (pDesc != NULL && pDesc->iFillColor != 0) ? pDesc->iFillColor : XUI_COLOR_RGBA(47, 128, 237, 255);
	pData->fMin = fMin;
	pData->fMax = fMax;
	pData->fValue = __xuiSliderClampFloat((pDesc != NULL) ? pDesc->fValue : fMin, fMin, fMax);
	pData->fStep = (pDesc != NULL && pDesc->fStep > 0.0f) ? pDesc->fStep : 0.0f;
	pData->fPageStep = (pDesc != NULL && pDesc->fPageStep > 0.0f) ? pDesc->fPageStep : 0.0f;
	pData->fTrackSize = (pDesc != NULL && pDesc->fTrackSize > 0.0f) ? pDesc->fTrackSize : 4.0f;
	pData->fKnobSize = (pDesc != NULL && pDesc->fKnobSize > 0.0f) ? pDesc->fKnobSize : 14.0f;
	pData->fTrackRadius = (pDesc != NULL && pDesc->fTrackRadius >= 0.0f) ? pDesc->fTrackRadius : -1.0f;
	pData->iTrackColor = (pDesc != NULL && pDesc->iTrackColor != 0) ? pDesc->iTrackColor : XUI_COLOR_RGBA(218, 230, 242, 255);
	pData->iFillColor = iFill;
	pData->iFillHoverColor = __xuiSliderLighten(iFill, 12);
	pData->iFillActiveColor = __xuiSliderLighten(iFill, -18);
	pData->iKnobColor = (pDesc != NULL && pDesc->iKnobColor != 0) ? pDesc->iKnobColor : XUI_COLOR_RGBA(255, 255, 255, 255);
	pData->iKnobBorderColor = (pDesc != NULL && pDesc->iKnobBorderColor != 0) ? pDesc->iKnobBorderColor : XUI_COLOR_RGBA(146, 169, 196, 255);
	pData->iFocusColor = (pDesc != NULL && pDesc->iFocusColor != 0) ? pDesc->iFocusColor : XUI_COLOR_RGBA(47, 128, 237, 130);
	pData->iDisabledColor = (pDesc != NULL && pDesc->iDisabledColor != 0) ? pDesc->iDisabledColor : XUI_COLOR_RGBA(172, 184, 200, 150);
	pData->iOrientation = (pDesc != NULL && __xuiSliderOrientationValid(pDesc->iOrientation)) ? pDesc->iOrientation : XUI_ORIENTATION_HORIZONTAL;
	iRet = xuiWidgetSetPadding(pWidget, (xui_thickness_t){0.0f, 0.0f, 0.0f, 0.0f});
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiWidgetSetOverflow(pWidget, XUI_OVERFLOW_CLIP);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiWidgetSetFocusable(pWidget, 1);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiWidgetSetTabStop(pWidget, 1);
	if ( iRet != XUI_OK ) return iRet;
	iRet = __xuiSliderInitCacheStates(pWidget);
	if ( iRet != XUI_OK ) return iRet;
	iRet = __xuiSliderInitEvents(pWidget);
	if ( iRet != XUI_OK ) return iRet;
	return __xuiSliderSyncState(pWidget, pData);
}

static void __xuiSliderDestroy(xui_widget pWidget, void* pTypeData, void* pUser)
{
	xui_slider_data_t* pData;

	(void)pUser;
	pData = (xui_slider_data_t*)pTypeData;
	if ( pData != NULL ) {
		__xuiSliderCancelDrag(pWidget, pData);
		memset(pData, 0, sizeof(*pData));
	}
}

static void __xuiSliderRegisterStyleProperty(xui_context pContext, xui_widget_type pType, const char* sName, int iValueType, uint32_t iDirtyFlags, uint32_t iFlags)
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

static void __xuiSliderRegisterStyleProperties(xui_context pContext, xui_widget_type pType)
{
	uint32_t iPaintDirty;
	uint32_t iLayoutDirty;

	iPaintDirty = XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER;
	iLayoutDirty = XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER;
	__xuiSliderRegisterStyleProperty(pContext, pType, "slider.track.color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiSliderRegisterStyleProperty(pContext, pType, "slider.fill.color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiSliderRegisterStyleProperty(pContext, pType, "slider.fill.hover_color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiSliderRegisterStyleProperty(pContext, pType, "slider.fill.active_color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiSliderRegisterStyleProperty(pContext, pType, "slider.knob.color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiSliderRegisterStyleProperty(pContext, pType, "slider.knob.disabled_color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiSliderRegisterStyleProperty(pContext, pType, "slider.knob.border_color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiSliderRegisterStyleProperty(pContext, pType, "slider.focus.color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiSliderRegisterStyleProperty(pContext, pType, "slider.disabled.color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiSliderRegisterStyleProperty(pContext, pType, "slider.track.size", XUI_STYLE_VALUE_FLOAT, iLayoutDirty, 0);
	__xuiSliderRegisterStyleProperty(pContext, pType, "slider.knob.size", XUI_STYLE_VALUE_FLOAT, iLayoutDirty, 0);
	__xuiSliderRegisterStyleProperty(pContext, pType, "slider.track.radius", XUI_STYLE_VALUE_FLOAT, iPaintDirty, 0);
}

static int __xuiSliderAccessibleNode(xui_widget pWidget, xui_accessible_node_t* pNode)
{
	xui_slider_data_t* pData = __xuiSliderGetData(pWidget);
	if ( (pData == NULL) || (pNode == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	pNode->iRole = XUI_ACCESSIBLE_ROLE_SLIDER;
	(void)snprintf(pWidget->sAccessibleValue, sizeof(pWidget->sAccessibleValue), "%.9g", (double)pData->fValue);
	pNode->sValue = pWidget->sAccessibleValue;
	pNode->tRange.iSize = sizeof(pNode->tRange);
	pNode->tRange.fValue = pData->fValue;
	pNode->tRange.fMin = pData->fMin;
	pNode->tRange.fMax = pData->fMax;
	pNode->tRange.fStep = __xuiSliderSmallStep(pData);
	pNode->tRange.fPageStep = __xuiSliderPageStep(pData);
	if ( pData->iOrientation == XUI_ORIENTATION_VERTICAL ) pNode->iState |= XUI_ACCESSIBLE_STATE_VERTICAL;
	pNode->iActions = XUI_ACCESSIBLE_ACTION_MASK(XUI_ACCESSIBLE_ACTION_SET_VALUE) |
		XUI_ACCESSIBLE_ACTION_MASK(XUI_ACCESSIBLE_ACTION_INCREMENT) |
		XUI_ACCESSIBLE_ACTION_MASK(XUI_ACCESSIBLE_ACTION_DECREMENT);
	return XUI_OK;
}

static int __xuiSliderAccessibleAction(xui_widget pWidget, int iAction, const void* pPayload)
{
	xui_slider_data_t* pData = __xuiSliderGetData(pWidget);
	double fValue;
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( iAction == XUI_ACCESSIBLE_ACTION_SET_VALUE ) {
		const xui_accessible_value_t* pValue = (const xui_accessible_value_t*)pPayload;
		if ( pValue == NULL || pValue->iSize < sizeof(*pValue) || pValue->iType != XUI_ACCESSIBLE_VALUE_NUMBER )
			return XUI_ERROR_INVALID_ARGUMENT;
		fValue = pValue->fNumber;
		if ( !(fValue >= pData->fMin && fValue <= pData->fMax) ) return XUI_ERROR_INVALID_ARGUMENT;
	} else if ( iAction == XUI_ACCESSIBLE_ACTION_INCREMENT || iAction == XUI_ACCESSIBLE_ACTION_DECREMENT ) {
		fValue = __xuiSliderValueWithDelta(pData,
			(iAction == XUI_ACCESSIBLE_ACTION_INCREMENT ? 1.0 : -1.0) *
			(double)__xuiSliderSmallStep(pData));
	} else return XUI_ERROR_UNSUPPORTED;
	return __xuiSliderSetValueInternal(pWidget, pData, (float)fValue, 1);
}

static const xui_internal_accessibility_adapter_t g_xuiSliderAccessible = {
	.getNode = __xuiSliderAccessibleNode, .performAction = __xuiSliderAccessibleAction
};

XUI_API xui_widget_type xuiSliderGetType(xui_context pContext)
{
	xui_widget_type_desc_t tDesc;
	xui_widget_type pType;
	int iRet;

	if ( !xuiInternalContextIsValid(pContext) ) {
		return NULL;
	}
	pType = xuiWidgetFindType(pContext, "slider");
	if ( pType != NULL ) {
		__xuiSliderRegisterStyleProperties(pContext, pType);
		pType->pAccessibleAdapter = &g_xuiSliderAccessible;
		return pType;
	}
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.sName = "slider";
	tDesc.pParent = xuiWidgetGetBaseType();
	tDesc.iFlags = XUI_WIDGET_TYPE_DEFAULT_LAYOUT | XUI_WIDGET_TYPE_DEFAULT_CACHE_POLICY;
	tDesc.iTypeDataSize = sizeof(xui_slider_data_t);
	tDesc.onInit = __xuiSliderInit;
	tDesc.onDestroy = __xuiSliderDestroy;
	tDesc.onContentMeasure = __xuiSliderContentMeasure;
	tDesc.onCacheRender = __xuiSliderCacheRender;
	__xuiSliderDefaultLayout(&tDesc.tLayout);
	__xuiSliderDefaultCachePolicy(&tDesc.tCachePolicy);
	iRet = xuiWidgetRegisterType(pContext, &pType, &tDesc);
	if ( iRet != XUI_OK ) {
		return NULL;
	}
	__xuiSliderRegisterStyleProperties(pContext, pType);
	pType->pAccessibleAdapter = &g_xuiSliderAccessible;
	return pType;
}

XUI_API int xuiSliderCreate(xui_context pContext, xui_widget* ppWidget, const xui_slider_desc_t* pDesc)
{
	xui_widget_type pType;

	if ( ppWidget == NULL ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	*ppWidget = NULL;
	if ( !xuiInternalContextIsValid(pContext) || !__xuiSliderDescValid(pDesc) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pType = xuiSliderGetType(pContext);
	if ( pType == NULL ) {
		return XUI_ERROR_NOT_INITIALIZED;
	}
	return xuiWidgetCreateTyped(pContext, pType, ppWidget, pDesc);
}

XUI_API int xuiSliderSetChange(xui_widget pWidget, xui_slider_change_proc onChange, void* pUser)
{
	xui_slider_data_t* pData = __xuiSliderGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	pData->onChange = onChange;
	pData->pChangeUser = pUser;
	return XUI_OK;
}

XUI_API int xuiSliderSetRange(xui_widget pWidget, float fMin, float fMax)
{
	xui_slider_data_t* pData;
	float fValue;
	int bValueChanged;
	int iRet;

	pData = __xuiSliderGetData(pWidget);
	if ( pData == NULL || !isfinite(fMin) || !isfinite(fMax) ) return XUI_ERROR_INVALID_ARGUMENT;
	__xuiSliderNormalizeRange(&fMin, &fMax);
	if ( pData->fMin == fMin && pData->fMax == fMax ) return XUI_OK;
	fValue = __xuiSliderClampFloat(pData->fValue, fMin, fMax);
	bValueChanged = pData->fValue != fValue;
	iRet = xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	if ( iRet != XUI_OK ) return iRet;
	__xuiSliderCancelDrag(pWidget, pData);
	pData->fMin = fMin;
	pData->fMax = fMax;
	pData->fValue = fValue;
	xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_NODE_CHANGED);
	if ( bValueChanged ) {
		xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_VALUE_CHANGED);
		__xuiSliderIncrementCounter(&pData->iChangeCount);
	}
	return XUI_OK;
}

XUI_API int xuiSliderGetRange(xui_widget pWidget, float* pMin, float* pMax)
{
	xui_slider_data_t* pData = __xuiSliderGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pMin != NULL ) *pMin = pData->fMin;
	if ( pMax != NULL ) *pMax = pData->fMax;
	return XUI_OK;
}

XUI_API int xuiSliderSetValue(xui_widget pWidget, float fValue)
{
	xui_slider_data_t* pData = __xuiSliderGetData(pWidget);
	if ( pData == NULL || !isfinite(fValue) ) return XUI_ERROR_INVALID_ARGUMENT;
	return __xuiSliderSetValueInternal(pWidget, pData, fValue, 0);
}

XUI_API float xuiSliderGetValue(xui_widget pWidget)
{
	xui_slider_data_t* pData = __xuiSliderGetData(pWidget);
	return (pData != NULL) ? pData->fValue : 0.0f;
}

XUI_API float xuiSliderGetRate(xui_widget pWidget)
{
	xui_slider_data_t* pData = __xuiSliderGetData(pWidget);
	return __xuiSliderRateFromData(pData);
}

XUI_API int xuiSliderSetStep(xui_widget pWidget, float fStep, float fPageStep)
{
	xui_slider_data_t* pData = __xuiSliderGetData(pWidget);
	if ( (pData == NULL) || !isfinite(fStep) || !isfinite(fPageStep) ||
	     (fStep < 0.0f) || (fPageStep < 0.0f) ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pData->fStep == fStep && pData->fPageStep == fPageStep ) return XUI_OK;
	pData->fStep = fStep;
	pData->fPageStep = fPageStep;
	xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_NODE_CHANGED);
	return XUI_OK;
}

XUI_API int xuiSliderGetStep(xui_widget pWidget, float* pStep, float* pPageStep)
{
	xui_slider_data_t* pData = __xuiSliderGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pStep != NULL ) *pStep = pData->fStep;
	if ( pPageStep != NULL ) *pPageStep = pData->fPageStep;
	return XUI_OK;
}

XUI_API int xuiSliderSetOrientation(xui_widget pWidget, int iOrientation)
{
	xui_slider_data_t* pData = __xuiSliderGetData(pWidget);
	int iRet;
	if ( (pData == NULL) || !__xuiSliderOrientationValid(iOrientation) ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pData->iOrientation == iOrientation ) return XUI_OK;
	iRet = xuiWidgetInvalidate(pWidget,
		XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	if ( iRet != XUI_OK ) return iRet;
	__xuiSliderCancelDrag(pWidget, pData);
	pData->iOrientation = iOrientation;
	xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_STATE_CHANGED);
	return XUI_OK;
}

XUI_API int xuiSliderGetOrientation(xui_widget pWidget)
{
	xui_slider_data_t* pData = __xuiSliderGetData(pWidget);
	return (pData != NULL) ? pData->iOrientation : XUI_ORIENTATION_HORIZONTAL;
}

XUI_API int xuiSliderSetMetrics(xui_widget pWidget, float fTrackSize, float fKnobSize, float fTrackRadius)
{
	xui_slider_data_t* pData = __xuiSliderGetData(pWidget);
	int iRet;
	if ( (pData == NULL) || !__xuiSliderMetricValid(fTrackSize, 0.0f) ||
	     !__xuiSliderMetricValid(fKnobSize, 0.0f) ||
	     !__xuiSliderMetricValid(fTrackRadius, -1.0f) ) return XUI_ERROR_INVALID_ARGUMENT;
	fTrackSize = (fTrackSize > 0.0f) ? fTrackSize : 4.0f;
	fKnobSize = (fKnobSize > 0.0f) ? fKnobSize : 14.0f;
	if ( pData->fTrackSize == fTrackSize && pData->fKnobSize == fKnobSize &&
	     pData->fTrackRadius == fTrackRadius ) return XUI_OK;
	iRet = xuiWidgetInvalidate(pWidget,
		XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	if ( iRet != XUI_OK ) return iRet;
	__xuiSliderCancelDrag(pWidget, pData);
	pData->fTrackSize = fTrackSize;
	pData->fKnobSize = fKnobSize;
	pData->fTrackRadius = fTrackRadius;
	return XUI_OK;
}

XUI_API int xuiSliderGetMetrics(xui_widget pWidget, float* pTrackSize, float* pKnobSize, float* pTrackRadius)
{
	xui_slider_data_t* pData = __xuiSliderGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pTrackSize != NULL ) *pTrackSize = pData->fTrackSize;
	if ( pKnobSize != NULL ) *pKnobSize = pData->fKnobSize;
	if ( pTrackRadius != NULL ) *pTrackRadius = pData->fTrackRadius;
	return XUI_OK;
}

XUI_API int xuiSliderSetColors(xui_widget pWidget, uint32_t iTrack, uint32_t iFill, uint32_t iKnob, uint32_t iFocus, uint32_t iDisabled)
{
	xui_slider_data_t* pData = __xuiSliderGetData(pWidget);
	int iRet;
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pData->iTrackColor == iTrack && pData->iFillColor == iFill &&
	     pData->iKnobColor == iKnob && pData->iFocusColor == iFocus &&
	     pData->iDisabledColor == iDisabled ) return XUI_OK;
	iRet = xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	if ( iRet != XUI_OK ) return iRet;
	pData->iTrackColor = iTrack;
	pData->iFillColor = iFill;
	pData->iFillHoverColor = __xuiSliderLighten(iFill, 12);
	pData->iFillActiveColor = __xuiSliderLighten(iFill, -18);
	pData->iKnobColor = iKnob;
	pData->iFocusColor = iFocus;
	pData->iDisabledColor = iDisabled;
	return XUI_OK;
}

XUI_API int xuiSliderGetColors(xui_widget pWidget, uint32_t* pTrack, uint32_t* pFill, uint32_t* pKnob, uint32_t* pFocus, uint32_t* pDisabled)
{
	xui_slider_data_t* pData = __xuiSliderGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pTrack != NULL ) *pTrack = pData->iTrackColor;
	if ( pFill != NULL ) *pFill = pData->iFillColor;
	if ( pKnob != NULL ) *pKnob = pData->iKnobColor;
	if ( pFocus != NULL ) *pFocus = pData->iFocusColor;
	if ( pDisabled != NULL ) *pDisabled = pData->iDisabledColor;
	return XUI_OK;
}

XUI_API int xuiSliderSetKnobBorderColor(xui_widget pWidget, uint32_t iColor)
{
	xui_slider_data_t* pData = __xuiSliderGetData(pWidget);
	int iRet;
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pData->iKnobBorderColor == iColor ) return XUI_OK;
	iRet = xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	if ( iRet != XUI_OK ) return iRet;
	pData->iKnobBorderColor = iColor;
	return XUI_OK;
}

XUI_API uint32_t xuiSliderGetKnobBorderColor(xui_widget pWidget)
{
	xui_slider_data_t* pData = __xuiSliderGetData(pWidget);
	return (pData != NULL) ? pData->iKnobBorderColor : 0;
}

static xui_rect_t __xuiSliderGetComputedRect(xui_widget pWidget, int iWhich)
{
	xui_slider_data_t* pData;
	xui_slider_data_t tResolved;
	xui_slider_geometry_t tGeometry;
	xui_rect_t tZero;

	memset(&tZero, 0, sizeof(tZero));
	pData = __xuiSliderGetData(pWidget);
	if ( pData == NULL ) return tZero;
	__xuiSliderResolve(pWidget, pData, &tResolved);
	__xuiSliderComputeGeometry(pWidget, &tResolved, &tGeometry);
	if ( iWhich == 1 ) return __xuiSliderOutputRect(tGeometry.tTrack);
	if ( iWhich == 2 ) return __xuiSliderOutputRect(tGeometry.tFill);
	if ( iWhich == 3 ) return __xuiSliderOutputRect(tGeometry.tKnob);
	return tZero;
}

XUI_API xui_rect_t xuiSliderGetTrackRect(xui_widget pWidget)
{
	return __xuiSliderGetComputedRect(pWidget, 1);
}

XUI_API xui_rect_t xuiSliderGetFillRect(xui_widget pWidget)
{
	return __xuiSliderGetComputedRect(pWidget, 2);
}

XUI_API xui_rect_t xuiSliderGetKnobRect(xui_widget pWidget)
{
	return __xuiSliderGetComputedRect(pWidget, 3);
}

XUI_API uint32_t xuiSliderGetState(xui_widget pWidget)
{
	xui_slider_data_t* pData;

	if ( !xuiInternalWidgetIsValid(pWidget) ) {
		return 0;
	}
	pData = __xuiSliderGetData(pWidget);
	return __xuiSliderBaseState(pWidget, pData);
}

XUI_API int xuiSliderGetChangeCount(xui_widget pWidget)
{
	xui_slider_data_t* pData = __xuiSliderGetData(pWidget);
	return (pData != NULL) ? pData->iChangeCount : 0;
}

#endif
