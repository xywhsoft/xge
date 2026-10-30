#include "xui_config.h"
#if XUI_ENABLE_RANGE_SLIDER
#include "xui_internal.h"
#include "xui_slider_common.h"

#include <float.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#define XUI_RANGE_SLIDER_ACCESSIBLE_ROOT_ID 1u
#define XUI_RANGE_SLIDER_ACCESSIBLE_START_ID 2u
#define XUI_RANGE_SLIDER_ACCESSIBLE_END_ID 3u

typedef struct xui_range_slider_data_t {
	xui_range_slider_change_proc onChange;
	void* pChangeUser;
	char sAccessibleRootValue[96];
	char sAccessibleStartValue[48];
	char sAccessibleEndValue[48];
	float fMin;
	float fMax;
	float fStart;
	float fEnd;
	float fStep;
	float fPageStep;
	float fMinInterval;
	float fMaxInterval;
	float fTrackSize;
	float fKnobSize;
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
	int iActiveThumb;
	float fDragOffset;
	int iChangeCount;
} xui_range_slider_data_t;

static int __xuiRangeSliderOrientationValid(int iOrientation);

static int __xuiRangeSliderDescValid(const xui_range_slider_desc_t* pDesc)
{
	if ( pDesc == NULL ) {
		return 1;
	}
	if ( (pDesc->iSize != 0) && (pDesc->iSize < sizeof(*pDesc)) ) return 0;
	if ( !isfinite(pDesc->fMin) || !isfinite(pDesc->fMax) ||
	     !isfinite(pDesc->fStart) || !isfinite(pDesc->fEnd) ) return 0;
	if ( !isfinite(pDesc->fStep) || pDesc->fStep < 0.0f ||
	     !isfinite(pDesc->fPageStep) || pDesc->fPageStep < 0.0f ) return 0;
	if ( !isfinite(pDesc->fMinInterval) || pDesc->fMinInterval < 0.0f ||
	     !isfinite(pDesc->fMaxInterval) || pDesc->fMaxInterval < 0.0f ) return 0;
	if ( pDesc->fMaxInterval > 0.0f && pDesc->fMinInterval > pDesc->fMaxInterval ) return 0;
	if ( !xuiSliderCommonMetricValid(pDesc->fTrackSize, 0.0f) ||
	     !xuiSliderCommonMetricValid(pDesc->fKnobSize, 0.0f) ) return 0;
	return __xuiRangeSliderOrientationValid(pDesc->iOrientation);
}

static int __xuiRangeSliderOrientationValid(int iOrientation)
{
	return (iOrientation == XUI_ORIENTATION_HORIZONTAL) || (iOrientation == XUI_ORIENTATION_VERTICAL);
}

static float __xuiRangeSliderClampFloat(float fValue, float fMin, float fMax)
{
	return xuiSliderCommonClamp(fValue, fMin, fMax);
}

static float __xuiRangeSliderAbsFloat(float fValue)
{
	return xuiSliderCommonAbs(fValue);
}

static float __xuiRangeSliderMaxFloat(float fA, float fB)
{
	return xuiSliderCommonMax(fA, fB);
}

static void __xuiRangeSliderNormalizeRange(float* pMin, float* pMax)
{
	xuiSliderCommonNormalizeRange(pMin, pMax);
}

static double __xuiRangeSliderEffectiveMinInterval(float fMinInterval, float fMin, float fMax)
{
	double fRange;

	fRange = (double)fMax - (double)fMin;
	if ( fRange < 0.0 ) fRange = 0.0;
	if ( fMinInterval <= 0.0f ) return 0.0f;
	if ( (double)fMinInterval > fRange ) return fRange;
	return (double)fMinInterval;
}

static double __xuiRangeSliderEffectiveMaxInterval(float fMaxInterval, float fMinInterval, float fMin, float fMax)
{
	double fRange;
	double fMinEffective;

	fRange = (double)fMax - (double)fMin;
	if ( fRange < 0.0 ) fRange = 0.0;
	fMinEffective = __xuiRangeSliderEffectiveMinInterval(fMinInterval, fMin, fMax);
	if ( fMaxInterval <= 0.0f ) return fRange;
	if ( (double)fMaxInterval < fMinEffective ) return fMinEffective;
	if ( (double)fMaxInterval > fRange ) return fRange;
	return (double)fMaxInterval;
}

static uint32_t __xuiRangeSliderColorWithAlpha(uint32_t iColor, uint32_t iAlpha)
{
	if ( (iColor & 0xffu) == 0u ) return iColor;
	return (iColor & 0xffffff00u) | (iAlpha & 0xffu);
}

static int __xuiRangeSliderColorAlpha(uint32_t iColor)
{
	return (int)(iColor & 0xffu);
}

static uint32_t __xuiRangeSliderLighten(uint32_t iColor, int iAmount)
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

static float __xuiRangeSliderRateFromValue(const xui_range_slider_data_t* pData, float fValue)
{
	return pData != NULL ?
		xuiSliderCommonRate(pData->fMin, pData->fMax, fValue) : 0.0f;
}

static float __xuiRangeSliderSmallStep(const xui_range_slider_data_t* pData)
{
	return pData != NULL ? xuiSliderCommonStep(pData->fMin, pData->fMax,
		pData->fStep, 0.01) : 1.0f;
}

static float __xuiRangeSliderPageStep(const xui_range_slider_data_t* pData)
{
	return pData != NULL ? xuiSliderCommonStep(pData->fMin, pData->fMax,
		pData->fPageStep, 0.10) : 1.0f;
}

static xui_range_slider_data_t* __xuiRangeSliderGetData(xui_widget pWidget)
{
	xui_context pContext;
	xui_widget_type pType;

	if ( !xuiInternalWidgetIsValid(pWidget) ) {
		return NULL;
	}
	pContext = xuiWidgetGetContext(pWidget);
	pType = xuiWidgetFindType(pContext, "rangeslider");
	if ( (pType == NULL) || !xuiWidgetIsType(pWidget, pType) ) {
		return NULL;
	}
	return (xui_range_slider_data_t*)xuiWidgetGetTypeData(pWidget);
}

static int __xuiRangeSliderDataAlive(xui_widget pWidget,
	const xui_range_slider_data_t* pData)
{
	xui_context pContext;

	if ( (pData == NULL) || (__xuiRangeSliderGetData(pWidget) != pData) ) return 0;
	pContext = xuiWidgetGetContext(pWidget);
	return xuiInternalContextIsValid(pContext) && !xuiInternalContextDestroyPending(pContext);
}

static int __xuiRangeSliderCanInteract(xui_widget pWidget)
{
	return xuiWidgetGetEffectiveEnabled(pWidget) &&
	       xuiWidgetGetEffectiveVisible(pWidget);
}

static void __xuiRangeSliderIncrementCounter(int* pValue)
{
	if ( (pValue != NULL) && (*pValue < INT_MAX) ) {
		(*pValue)++;
	}
}

static int __xuiRangeSliderPointerMatches(const xui_range_slider_data_t* pData,
	const xui_event_t* pEvent)
{
	return pData != NULL && pEvent != NULL && pData->bPointerActive &&
	       pData->iActivePointerId == pEvent->iPointerId &&
	       pData->iActivePointerType == pEvent->iPointerType;
}

static int __xuiRangeSliderStyleColor(xui_widget pWidget, const char* sName, uint32_t* pColor)
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

static int __xuiRangeSliderStyleFloat(xui_widget pWidget, const char* sName, float* pValue)
{
	xui_style_property_t tProperty;
	int iRet;

	memset(&tProperty, 0, sizeof(tProperty));
	tProperty.iSize = sizeof(tProperty);
	iRet = xuiWidgetGetResolvedStyleProperty(pWidget, sName, &tProperty);
	if ( (iRet == XUI_OK) && (tProperty.tValue.iType == XUI_STYLE_VALUE_FLOAT) &&
	     xuiSliderCommonMetricValid(tProperty.tValue.fFloat, 0.0f) ) {
		*pValue = tProperty.tValue.fFloat;
		return 1;
	}
	return 0;
}

static void __xuiRangeSliderResolve(xui_widget pWidget, const xui_range_slider_data_t* pData, xui_range_slider_data_t* pResolved)
{
	*pResolved = *pData;
	(void)__xuiRangeSliderStyleColor(pWidget, "rangeslider.track.color", &pResolved->iTrackColor);
	(void)__xuiRangeSliderStyleColor(pWidget, "rangeslider.fill.color", &pResolved->iFillColor);
	(void)__xuiRangeSliderStyleColor(pWidget, "rangeslider.fill.hover_color", &pResolved->iFillHoverColor);
	(void)__xuiRangeSliderStyleColor(pWidget, "rangeslider.fill.active_color", &pResolved->iFillActiveColor);
	(void)__xuiRangeSliderStyleColor(pWidget, "rangeslider.knob.color", &pResolved->iKnobColor);
	pResolved->iDisabledKnobColor = XUI_COLOR_RGBA(232, 237, 244, 255);
	(void)__xuiRangeSliderStyleColor(pWidget, "rangeslider.knob.disabled_color", &pResolved->iDisabledKnobColor);
	(void)__xuiRangeSliderStyleColor(pWidget, "rangeslider.knob.border_color", &pResolved->iKnobBorderColor);
	(void)__xuiRangeSliderStyleColor(pWidget, "rangeslider.focus.color", &pResolved->iFocusColor);
	(void)__xuiRangeSliderStyleColor(pWidget, "rangeslider.disabled.color", &pResolved->iDisabledColor);
	(void)__xuiRangeSliderStyleFloat(pWidget, "rangeslider.track.size", &pResolved->fTrackSize);
	(void)__xuiRangeSliderStyleFloat(pWidget, "rangeslider.knob.size", &pResolved->fKnobSize);
	if ( pResolved->fTrackSize <= 0.0f ) pResolved->fTrackSize = 4.0f;
	if ( pResolved->fKnobSize <= 0.0f ) pResolved->fKnobSize = 14.0f;
}

static uint32_t __xuiRangeSliderBaseState(xui_widget pWidget, const xui_range_slider_data_t* pData)
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

static uint32_t __xuiRangeSliderStateId(xui_widget pWidget, const xui_range_slider_data_t* pData)
{
	uint32_t iState;

	iState = __xuiRangeSliderBaseState(pWidget, pData);
	if ( (iState & XUI_WIDGET_STATE_DISABLED) != 0 ) return XUI_WIDGET_STATE_DISABLED;
	if ( (iState & XUI_WIDGET_STATE_ACTIVE) != 0 ) return XUI_WIDGET_STATE_ACTIVE;
	if ( (iState & XUI_WIDGET_STATE_HOVER) != 0 ) return XUI_WIDGET_STATE_HOVER;
	if ( (iState & XUI_WIDGET_STATE_FOCUS) != 0 ) return XUI_WIDGET_STATE_FOCUS;
	return 0;
}

static int __xuiRangeSliderSyncState(xui_widget pWidget, xui_range_slider_data_t* pData)
{
	if ( (pWidget == NULL) || (pData == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	return xuiWidgetSetStateId(pWidget, __xuiRangeSliderStateId(pWidget, pData));
}

static int __xuiRangeSliderRectContains(xui_rect_t tRect, float fX, float fY)
{
	return (tRect.fW > 0.0f) &&
	       (tRect.fH > 0.0f) &&
	       (fX >= tRect.fX) &&
	       (fY >= tRect.fY) &&
	       (fX < (tRect.fX + tRect.fW)) &&
	       (fY < (tRect.fY + tRect.fH));
}

typedef struct xui_range_slider_geometry_t {
	xui_slider_common_rect_t tTrack;
	xui_slider_common_rect_t tFill;
	xui_slider_common_rect_t tStartKnob;
	xui_slider_common_rect_t tEndKnob;
} xui_range_slider_geometry_t;

static xui_slider_common_rect_t __xuiRangeSliderFillRect(
	const xui_range_slider_data_t* pData, xui_slider_common_rect_t tTrack)
{
	xui_slider_common_rect_t tFill;
	float fStartRate;
	float fEndRate;
	float fA;
	float fB;

	tFill = tTrack;
	fStartRate = __xuiRangeSliderRateFromValue(pData, pData->fStart);
	fEndRate = __xuiRangeSliderRateFromValue(pData, pData->fEnd);
	if ( fEndRate < fStartRate ) {
		fA = fEndRate;
		fB = fStartRate;
	} else {
		fA = fStartRate;
		fB = fEndRate;
	}
	if ( pData->iOrientation == XUI_ORIENTATION_HORIZONTAL ) {
		tFill.fX = tTrack.fX + tTrack.fW * fA;
		tFill.fW = tTrack.fW * (fB - fA);
	} else {
		tFill.fY = tTrack.fY + tTrack.fH * (1.0f - fB);
		tFill.fH = tTrack.fH * (fB - fA);
	}
	return tFill;
}

static void __xuiRangeSliderComputeGeometry(xui_widget pWidget,
	const xui_range_slider_data_t* pResolved,
	xui_range_slider_geometry_t* pGeometry)
{
	xui_rect_t tContent;

	memset(pGeometry, 0, sizeof(*pGeometry));
	tContent = xuiWidgetGetContentRect(pWidget);
	pGeometry->tTrack = xuiSliderCommonTrackRect(tContent,
		pResolved->fTrackSize, pResolved->fKnobSize, pResolved->iOrientation);
	pGeometry->tFill = __xuiRangeSliderFillRect(pResolved, pGeometry->tTrack);
	pGeometry->tStartKnob = xuiSliderCommonKnobRect(tContent,
		pGeometry->tTrack, pResolved->fKnobSize, pResolved->iOrientation,
		__xuiRangeSliderRateFromValue(pResolved, pResolved->fStart));
	pGeometry->tEndKnob = xuiSliderCommonKnobRect(tContent,
		pGeometry->tTrack, pResolved->fKnobSize, pResolved->iOrientation,
		__xuiRangeSliderRateFromValue(pResolved, pResolved->fEnd));
}

static int __xuiRangeSliderNearestThumb(
	const xui_range_slider_data_t* pData,
	const xui_range_slider_geometry_t* pGeometry, float fLocalX, float fLocalY)
{
	float fStartCenter;
	float fEndCenter;
	float fDistanceStart;
	float fDistanceEnd;
	int bStartHit;
	int bEndHit;

	if ( (pData == NULL) || (pGeometry == NULL) ) {
		return XUI_RANGE_SLIDER_THUMB_NONE;
	}
	bStartHit = xuiSliderCommonRectContains(pGeometry->tStartKnob,
		fLocalX, fLocalY);
	bEndHit = xuiSliderCommonRectContains(pGeometry->tEndKnob,
		fLocalX, fLocalY);
	if ( bStartHit && bEndHit ) {
		return pData->iActiveThumb == XUI_RANGE_SLIDER_THUMB_START ?
			XUI_RANGE_SLIDER_THUMB_START : XUI_RANGE_SLIDER_THUMB_END;
	}
	if ( bStartHit ) return XUI_RANGE_SLIDER_THUMB_START;
	if ( bEndHit ) return XUI_RANGE_SLIDER_THUMB_END;
	if ( pData->iOrientation == XUI_ORIENTATION_HORIZONTAL ) {
		fStartCenter = pGeometry->tStartKnob.fX +
			pGeometry->tStartKnob.fW * 0.5f;
		fEndCenter = pGeometry->tEndKnob.fX +
			pGeometry->tEndKnob.fW * 0.5f;
		fDistanceStart = __xuiRangeSliderAbsFloat(fLocalX - fStartCenter);
		fDistanceEnd = __xuiRangeSliderAbsFloat(fLocalX - fEndCenter);
	} else {
		fStartCenter = pGeometry->tStartKnob.fY +
			pGeometry->tStartKnob.fH * 0.5f;
		fEndCenter = pGeometry->tEndKnob.fY +
			pGeometry->tEndKnob.fH * 0.5f;
		fDistanceStart = __xuiRangeSliderAbsFloat(fLocalY - fStartCenter);
		fDistanceEnd = __xuiRangeSliderAbsFloat(fLocalY - fEndCenter);
	}
	return (fDistanceStart <= fDistanceEnd) ?
		XUI_RANGE_SLIDER_THUMB_START : XUI_RANGE_SLIDER_THUMB_END;
}

static int __xuiRangeSliderGrabInfo(xui_widget pWidget,
	const xui_range_slider_data_t* pData, const xui_event_t* pEvent,
	float* pOffset)
{
	xui_range_slider_data_t tResolved;
	xui_range_slider_geometry_t tGeometry;
	xui_slider_common_rect_t tKnob;
	xui_rect_t tWorld;
	float fLocalX;
	float fLocalY;
	int iThumb;

	__xuiRangeSliderResolve(pWidget, pData, &tResolved);
	__xuiRangeSliderComputeGeometry(pWidget, &tResolved, &tGeometry);
	tWorld = xuiWidgetGetWorldRect(pWidget);
	fLocalX = pEvent->fX - tWorld.fX;
	fLocalY = pEvent->fY - tWorld.fY;
	iThumb = __xuiRangeSliderNearestThumb(pData, &tGeometry, fLocalX, fLocalY);
	tKnob = iThumb == XUI_RANGE_SLIDER_THUMB_START ?
		tGeometry.tStartKnob : tGeometry.tEndKnob;
	*pOffset = 0.0f;
	if ( xuiSliderCommonRectContains(tKnob, fLocalX, fLocalY) ) {
		*pOffset = pData->iOrientation == XUI_ORIENTATION_HORIZONTAL ?
			fLocalX - (tKnob.fX + tKnob.fW * 0.5f) :
			fLocalY - (tKnob.fY + tKnob.fH * 0.5f);
	}
	return iThumb;
}

static int __xuiRangeSliderSetActiveThumb(xui_widget pWidget,
	xui_range_slider_data_t* pData, int iThumb)
{
	int iRet;

	if ( (pWidget == NULL) || (pData == NULL) ||
	     (iThumb != XUI_RANGE_SLIDER_THUMB_START && iThumb != XUI_RANGE_SLIDER_THUMB_END) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( pData->iActiveThumb == iThumb ) return XUI_OK;
	iRet = xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	if ( iRet != XUI_OK ) return iRet;
	pData->iActiveThumb = iThumb;
	xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_STATE_CHANGED);
	return XUI_OK;
}

static void __xuiRangeSliderNormalizeValues(float* pStart, float* pEnd, float fMin, float fMax, float fMinInterval, float fMaxInterval)
{
	double fCenter;
	double fInterval;
	double fTargetInterval;
	double fMinEffective;
	double fMaxEffective;
	double fStart;
	double fEnd;
	float fSwap;

	*pStart = __xuiRangeSliderClampFloat(*pStart, fMin, fMax);
	*pEnd = __xuiRangeSliderClampFloat(*pEnd, fMin, fMax);
	if ( *pEnd < *pStart ) {
		fSwap = *pStart;
		*pStart = *pEnd;
		*pEnd = fSwap;
	}
	fMinEffective = __xuiRangeSliderEffectiveMinInterval(fMinInterval, fMin, fMax);
	fMaxEffective = __xuiRangeSliderEffectiveMaxInterval(fMaxInterval, fMinInterval, fMin, fMax);
	fStart = (double)*pStart;
	fEnd = (double)*pEnd;
	fInterval = fEnd - fStart;
	fTargetInterval = fInterval;
	if ( fTargetInterval < fMinEffective ) {
		fTargetInterval = fMinEffective;
	}
	if ( fTargetInterval > fMaxEffective ) {
		fTargetInterval = fMaxEffective;
	}
	if ( fTargetInterval != fInterval ) {
		fCenter = fStart + fInterval * 0.5;
		fStart = fCenter - fTargetInterval * 0.5;
		fEnd = fStart + fTargetInterval;
		if ( fStart < (double)fMin ) {
			fStart = (double)fMin;
			fEnd = (double)fMin + fTargetInterval;
		}
		if ( fEnd > (double)fMax ) {
			fEnd = (double)fMax;
			fStart = (double)fMax - fTargetInterval;
		}
		if ( fStart < (double)fMin ) fStart = (double)fMin;
		if ( fStart > (double)fMax ) fStart = (double)fMax;
		if ( fEnd < (double)fMin ) fEnd = (double)fMin;
		if ( fEnd > (double)fMax ) fEnd = (double)fMax;
		*pStart = (float)fStart;
		*pEnd = (float)fEnd;
	}
}

static int __xuiRangeSliderSetValuesInternal(xui_widget pWidget, xui_range_slider_data_t* pData, float fStart, float fEnd, int bNotify)
{
	xui_range_slider_change_proc onChange;
	void* pChangeUser;
	int iRet;

	if ( (pWidget == NULL) || (pData == NULL) || !isfinite(fStart) || !isfinite(fEnd) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	__xuiRangeSliderNormalizeValues(&fStart, &fEnd, pData->fMin, pData->fMax, pData->fMinInterval, pData->fMaxInterval);
	if ( (pData->fStart == fStart) && (pData->fEnd == fEnd) ) {
		return XUI_OK;
	}
	iRet = xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	if ( iRet != XUI_OK ) return iRet;
	pData->fStart = fStart;
	pData->fEnd = fEnd;
	__xuiRangeSliderIncrementCounter(&pData->iChangeCount);
	xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_VALUE_CHANGED);
	xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_BOUNDS_CHANGED);
	onChange = pData->onChange;
	pChangeUser = pData->pChangeUser;
	if ( bNotify && (onChange != NULL) ) {
		onChange(pWidget, fStart, fEnd, pChangeUser);
	}
	return XUI_OK;
}

static int __xuiRangeSliderSetThumbValueInternal(xui_widget pWidget, xui_range_slider_data_t* pData, int iThumb, float fValue, int bNotify)
{
	float fStart;
	float fEnd;
	double fMaxInterval;
	double fMinInterval;
	double fLower;
	double fUpper;

	if ( (pWidget == NULL) || (pData == NULL) || !isfinite(fValue) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	fStart = pData->fStart;
	fEnd = pData->fEnd;
	fMinInterval = __xuiRangeSliderEffectiveMinInterval(pData->fMinInterval, pData->fMin, pData->fMax);
	fMaxInterval = __xuiRangeSliderEffectiveMaxInterval(pData->fMaxInterval, pData->fMinInterval, pData->fMin, pData->fMax);
	if ( iThumb == XUI_RANGE_SLIDER_THUMB_START ) {
		fLower = fmax((double)pData->fMin, (double)fEnd - fMaxInterval);
		fUpper = (double)fEnd - fMinInterval;
		if ( fUpper < fLower ) fUpper = fLower;
		fStart = __xuiRangeSliderClampFloat(fValue, (float)fLower, (float)fUpper);
	} else if ( iThumb == XUI_RANGE_SLIDER_THUMB_END ) {
		fLower = (double)fStart + fMinInterval;
		fUpper = fmin((double)pData->fMax, (double)fStart + fMaxInterval);
		if ( fUpper < fLower ) fLower = fUpper;
		fEnd = __xuiRangeSliderClampFloat(fValue, (float)fLower, (float)fUpper);
	} else {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	return __xuiRangeSliderSetValuesInternal(pWidget, pData, fStart, fEnd, bNotify);
}

static int __xuiRangeSliderSetValueFromEvent(xui_widget pWidget, xui_range_slider_data_t* pData, const xui_event_t* pEvent, int iThumb, int bNotify)
{
	xui_range_slider_data_t tResolved;
	xui_range_slider_geometry_t tGeometry;
	xui_rect_t tWorld;
	float fLocalX;
	float fLocalY;
	float fCurrent;
	float fValue;

	if ( (pWidget == NULL) || (pData == NULL) || (pEvent == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	__xuiRangeSliderResolve(pWidget, pData, &tResolved);
	__xuiRangeSliderComputeGeometry(pWidget, &tResolved, &tGeometry);
	tWorld = xuiWidgetGetWorldRect(pWidget);
	fLocalX = pEvent->fX - tWorld.fX;
	fLocalY = pEvent->fY - tWorld.fY;
	if ( iThumb == XUI_RANGE_SLIDER_THUMB_NONE ) {
		iThumb = __xuiRangeSliderNearestThumb(pData, &tGeometry,
			fLocalX, fLocalY);
	}
	if ( pData->iOrientation == XUI_ORIENTATION_HORIZONTAL ) {
		fLocalX -= pData->fDragOffset;
	} else {
		fLocalY -= pData->fDragOffset;
	}
	fCurrent = iThumb == XUI_RANGE_SLIDER_THUMB_START ?
		pData->fStart : pData->fEnd;
	fValue = xuiSliderCommonValueFromPoint(fCurrent, pData->fMin, pData->fMax,
		pData->iOrientation, tGeometry.tTrack, fLocalX, fLocalY);
	{
		int iRet = __xuiRangeSliderSetActiveThumb(pWidget, pData, iThumb);
		if ( iRet != XUI_OK ) return iRet;
	}
	return __xuiRangeSliderSetThumbValueInternal(pWidget, pData, iThumb, fValue, bNotify);
}

static void __xuiRangeSliderCancelDrag(xui_widget pWidget, xui_range_slider_data_t* pData)
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
	(void)__xuiRangeSliderSyncState(pWidget, pData);
	if ( bPointerActive && xuiInternalContextIsValid(pContext) &&
	     xuiGetPointerCaptureEx(pContext, iPointerId, iPointerType) == pWidget ) {
		(void)xuiReleasePointerCaptureEx(pContext, iPointerId, iPointerType, pWidget);
	}
}

static void __xuiRangeSliderCancelAction(xui_widget pWidget, void* pUser)
{
	xui_range_slider_data_t* pData;

	(void)pUser;
	pData = __xuiRangeSliderGetData(pWidget);
	__xuiRangeSliderCancelDrag(pWidget, pData);
}

static int __xuiRangeSliderPointerInside(xui_widget pWidget, const xui_event_t* pEvent)
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
	return __xuiRangeSliderRectContains(tContent, fLocalX, fLocalY);
}

static int __xuiRangeSliderPointerDown(xui_widget pWidget, xui_range_slider_data_t* pData, const xui_event_t* pEvent)
{
	xui_context pContext;
	float fDragOffset;
	int iThumb;
	int iRet;

	if ( pEvent->iButton != XUI_POINTER_BUTTON_LEFT ) {
		return XUI_OK;
	}
	if ( !__xuiRangeSliderCanInteract(pWidget) ||
	     !__xuiRangeSliderPointerInside(pWidget, pEvent) ) {
		return XUI_OK;
	}
	if ( pData->bPointerActive ) {
		return __xuiRangeSliderPointerMatches(pData, pEvent) ? XUI_EVENT_DISPATCH_STOP : XUI_OK;
	}
	pContext = xuiWidgetGetContext(pWidget);
	iRet = xuiSetFocusWidget(pContext, pWidget);
	if ( iRet != XUI_OK ) return iRet;
	if ( !__xuiRangeSliderDataAlive(pWidget, pData) ) return XUI_EVENT_DISPATCH_STOP;
	if ( !__xuiRangeSliderCanInteract(pWidget) ) {
		return XUI_EVENT_DISPATCH_STOP;
	}
	iThumb = __xuiRangeSliderGrabInfo(pWidget, pData, pEvent, &fDragOffset);
	iRet = xuiSetPointerCaptureEx(pContext, pEvent->iPointerId, pEvent->iPointerType, pWidget);
	if ( iRet != XUI_OK ) return iRet;
	if ( !__xuiRangeSliderDataAlive(pWidget, pData) ) return XUI_EVENT_DISPATCH_STOP;
	pData->bPointerActive = 1;
	pData->iActivePointerId = pEvent->iPointerId;
	pData->iActivePointerType = pEvent->iPointerType;
	pData->fDragOffset = fDragOffset;
	pData->bDragging = 1;
	if ( !__xuiRangeSliderCanInteract(pWidget) ) {
		__xuiRangeSliderCancelDrag(pWidget, pData);
		return XUI_EVENT_DISPATCH_STOP;
	}
	iRet = __xuiRangeSliderSetActiveThumb(pWidget, pData, iThumb);
	if ( iRet != XUI_OK ) {
		__xuiRangeSliderCancelDrag(pWidget, pData);
		return iRet;
	}
	iRet = __xuiRangeSliderSyncState(pWidget, pData);
	if ( iRet != XUI_OK ) {
		__xuiRangeSliderCancelDrag(pWidget, pData);
		return iRet;
	}
	iRet = __xuiRangeSliderSetValueFromEvent(pWidget, pData, pEvent, iThumb, 1);
	if ( iRet != XUI_OK ) {
		__xuiRangeSliderCancelDrag(pWidget, pData);
		return iRet;
	}
	if ( !__xuiRangeSliderDataAlive(pWidget, pData) ) return XUI_EVENT_DISPATCH_STOP;
	if ( !xuiWidgetGetEffectiveEnabled(pWidget) ||
	     !xuiWidgetGetEffectiveVisible(pWidget) ) {
		__xuiRangeSliderCancelDrag(pWidget, pData);
	}
	return XUI_EVENT_DISPATCH_STOP;
}

static int __xuiRangeSliderPointerMove(xui_widget pWidget, xui_range_slider_data_t* pData, const xui_event_t* pEvent)
{
	xui_context pContext;
	int iRet;

	if ( pData->bPointerActive && !__xuiRangeSliderPointerMatches(pData, pEvent) ) return XUI_OK;
	pContext = xuiWidgetGetContext(pWidget);
	if ( (pData->bDragging != 0) &&
	     (xuiGetPointerCaptureEx(pContext, pEvent->iPointerId, pEvent->iPointerType) == pWidget) ) {
		if ( !__xuiRangeSliderCanInteract(pWidget) ) {
			__xuiRangeSliderCancelDrag(pWidget, pData);
			return XUI_EVENT_DISPATCH_STOP;
		}
		iRet = __xuiRangeSliderSetValueFromEvent(pWidget, pData, pEvent, pData->iActiveThumb, 1);
		if ( iRet != XUI_OK ) return iRet;
		if ( !__xuiRangeSliderDataAlive(pWidget, pData) ) return XUI_EVENT_DISPATCH_STOP;
		if ( !__xuiRangeSliderCanInteract(pWidget) ) {
			__xuiRangeSliderCancelDrag(pWidget, pData);
		}
		return XUI_EVENT_DISPATCH_STOP;
	}
	return __xuiRangeSliderSyncState(pWidget, pData);
}

static int __xuiRangeSliderPointerUp(xui_widget pWidget, xui_range_slider_data_t* pData, const xui_event_t* pEvent)
{
	xui_context pContext;
	int bWasDragging;
	int iRet;

	if ( pEvent->iButton != XUI_POINTER_BUTTON_LEFT ) {
		return XUI_OK;
	}
	if ( !__xuiRangeSliderPointerMatches(pData, pEvent) ) return XUI_OK;
	pContext = xuiWidgetGetContext(pWidget);
	bWasDragging = (pData->bDragging != 0) ||
		(xuiGetPointerCaptureEx(pContext, pEvent->iPointerId, pEvent->iPointerType) == pWidget);
	if ( bWasDragging ) {
		if ( __xuiRangeSliderCanInteract(pWidget) ) {
			iRet = __xuiRangeSliderSetValueFromEvent(pWidget, pData, pEvent,
				pData->iActiveThumb, 1);
			if ( iRet != XUI_OK ) {
				if ( __xuiRangeSliderDataAlive(pWidget, pData) ) {
					__xuiRangeSliderCancelDrag(pWidget, pData);
				}
				return iRet;
			}
			if ( !__xuiRangeSliderDataAlive(pWidget, pData) ) return XUI_EVENT_DISPATCH_STOP;
		}
	}
	__xuiRangeSliderCancelDrag(pWidget, pData);
	return bWasDragging ? XUI_EVENT_DISPATCH_STOP : XUI_OK;
}

static int __xuiRangeSliderWheel(xui_widget pWidget, xui_range_slider_data_t* pData, const xui_event_t* pEvent)
{
	xui_range_slider_data_t tResolved;
	xui_range_slider_geometry_t tGeometry;
	xui_rect_t tWorld;
	float fLocalX;
	float fLocalY;
	float fDelta;
	int iThumb;
	int iRet;

	if ( !__xuiRangeSliderCanInteract(pWidget) ) {
		return XUI_OK;
	}
	fDelta = (__xuiRangeSliderAbsFloat(pEvent->fWheelY) >= __xuiRangeSliderAbsFloat(pEvent->fWheelX)) ? pEvent->fWheelY : pEvent->fWheelX;
	if ( !isfinite(fDelta) || fDelta == 0.0f ) {
		return XUI_OK;
	}
	iThumb = pData->iActiveThumb;
	if ( iThumb == XUI_RANGE_SLIDER_THUMB_NONE ) {
		__xuiRangeSliderResolve(pWidget, pData, &tResolved);
		__xuiRangeSliderComputeGeometry(pWidget, &tResolved, &tGeometry);
		tWorld = xuiWidgetGetWorldRect(pWidget);
		fLocalX = pEvent->fX - tWorld.fX;
		fLocalY = pEvent->fY - tWorld.fY;
		iThumb = __xuiRangeSliderNearestThumb(pData, &tGeometry,
			fLocalX, fLocalY);
	}
	iRet = __xuiRangeSliderSetActiveThumb(pWidget, pData, iThumb);
	if ( iRet != XUI_OK ) return iRet;
	iRet = __xuiRangeSliderSetThumbValueInternal(pWidget, pData, iThumb,
		xuiSliderCommonValueWithDelta(
			iThumb == XUI_RANGE_SLIDER_THUMB_START ? pData->fStart : pData->fEnd,
			pData->fMin, pData->fMax,
			(double)fDelta * (double)__xuiRangeSliderSmallStep(pData)), 1);
	if ( iRet != XUI_OK ) return iRet;
	return XUI_EVENT_DISPATCH_STOP;
}

static int __xuiRangeSliderKeyDown(xui_widget pWidget, xui_range_slider_data_t* pData, const xui_event_t* pEvent)
{
	int iThumb;
	int iRet;
	float fValue;

	if ( !__xuiRangeSliderCanInteract(pWidget) ) {
		return XUI_OK;
	}
	iThumb = (pData->iActiveThumb != XUI_RANGE_SLIDER_THUMB_NONE) ? pData->iActiveThumb : XUI_RANGE_SLIDER_THUMB_END;
	fValue = (iThumb == XUI_RANGE_SLIDER_THUMB_START) ? pData->fStart : pData->fEnd;
	switch ( pEvent->iKey ) {
	case XUI_KEY_TAB:
		if ( (pEvent->iModifiers & (XUI_MOD_CTRL | XUI_MOD_ALT)) != 0u ) return XUI_OK;
		if ( (pEvent->iModifiers & XUI_MOD_SHIFT) != 0u ) {
			if ( pData->iActiveThumb != XUI_RANGE_SLIDER_THUMB_END ) return XUI_OK;
			iRet = __xuiRangeSliderSetActiveThumb(pWidget, pData, XUI_RANGE_SLIDER_THUMB_START);
		} else {
			if ( pData->iActiveThumb != XUI_RANGE_SLIDER_THUMB_START ) return XUI_OK;
			iRet = __xuiRangeSliderSetActiveThumb(pWidget, pData, XUI_RANGE_SLIDER_THUMB_END);
		}
		if ( iRet != XUI_OK ) return iRet;
		return XUI_EVENT_DISPATCH_STOP;
	case XUI_KEY_ESCAPE:
		__xuiRangeSliderCancelDrag(pWidget, pData);
		return XUI_EVENT_DISPATCH_STOP;
	case XUI_KEY_HOME:
		iRet = __xuiRangeSliderSetActiveThumb(pWidget, pData, iThumb);
		if ( iRet != XUI_OK ) return iRet;
		iRet = __xuiRangeSliderSetThumbValueInternal(pWidget, pData, iThumb,
			(iThumb == XUI_RANGE_SLIDER_THUMB_START) ? pData->fMin : pData->fStart, 1);
		if ( iRet != XUI_OK ) return iRet;
		return XUI_EVENT_DISPATCH_STOP;
	case XUI_KEY_END:
		iRet = __xuiRangeSliderSetActiveThumb(pWidget, pData, iThumb);
		if ( iRet != XUI_OK ) return iRet;
		iRet = __xuiRangeSliderSetThumbValueInternal(pWidget, pData, iThumb,
			(iThumb == XUI_RANGE_SLIDER_THUMB_START) ? pData->fEnd : pData->fMax, 1);
		if ( iRet != XUI_OK ) return iRet;
		return XUI_EVENT_DISPATCH_STOP;
	case XUI_KEY_PAGE_DOWN:
		iRet = __xuiRangeSliderSetActiveThumb(pWidget, pData, iThumb);
		if ( iRet != XUI_OK ) return iRet;
		iRet = __xuiRangeSliderSetThumbValueInternal(pWidget, pData, iThumb,
			xuiSliderCommonValueWithDelta(fValue, pData->fMin, pData->fMax,
				-(double)__xuiRangeSliderPageStep(pData)), 1);
		if ( iRet != XUI_OK ) return iRet;
		return XUI_EVENT_DISPATCH_STOP;
	case XUI_KEY_PAGE_UP:
		iRet = __xuiRangeSliderSetActiveThumb(pWidget, pData, iThumb);
		if ( iRet != XUI_OK ) return iRet;
		iRet = __xuiRangeSliderSetThumbValueInternal(pWidget, pData, iThumb,
			xuiSliderCommonValueWithDelta(fValue, pData->fMin, pData->fMax,
				(double)__xuiRangeSliderPageStep(pData)), 1);
		if ( iRet != XUI_OK ) return iRet;
		return XUI_EVENT_DISPATCH_STOP;
	case XUI_KEY_LEFT:
	case XUI_KEY_DOWN:
		iRet = __xuiRangeSliderSetActiveThumb(pWidget, pData, iThumb);
		if ( iRet != XUI_OK ) return iRet;
		iRet = __xuiRangeSliderSetThumbValueInternal(pWidget, pData, iThumb,
			xuiSliderCommonValueWithDelta(fValue, pData->fMin, pData->fMax,
				-(double)__xuiRangeSliderSmallStep(pData)), 1);
		if ( iRet != XUI_OK ) return iRet;
		return XUI_EVENT_DISPATCH_STOP;
	case XUI_KEY_RIGHT:
	case XUI_KEY_UP:
		iRet = __xuiRangeSliderSetActiveThumb(pWidget, pData, iThumb);
		if ( iRet != XUI_OK ) return iRet;
		iRet = __xuiRangeSliderSetThumbValueInternal(pWidget, pData, iThumb,
			xuiSliderCommonValueWithDelta(fValue, pData->fMin, pData->fMax,
				(double)__xuiRangeSliderSmallStep(pData)), 1);
		if ( iRet != XUI_OK ) return iRet;
		return XUI_EVENT_DISPATCH_STOP;
	default:
		break;
	}
	return XUI_OK;
}

static int __xuiRangeSliderEvent(xui_widget pWidget, const xui_event_t* pEvent, void* pUser)
{
	xui_range_slider_data_t* pData;
	int iRet;

	(void)pUser;
	if ( (pWidget == NULL) || (pEvent == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pData = __xuiRangeSliderGetData(pWidget);
	if ( pData == NULL ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	switch ( pEvent->iType ) {
	case XUI_EVENT_POINTER_ENTER:
	case XUI_EVENT_POINTER_MOVE:
		if ( (pEvent->pTarget != pWidget) && !pData->bPointerActive ) return XUI_OK;
		return __xuiRangeSliderPointerMove(pWidget, pData, pEvent);
	case XUI_EVENT_POINTER_LEAVE:
		if ( (pEvent->pTarget != pWidget) && !pData->bPointerActive ) return XUI_OK;
		if ( pData->bDragging == 0 ) {
			return __xuiRangeSliderSyncState(pWidget, pData);
		}
		break;
	case XUI_EVENT_POINTER_DOWN:
		if ( pEvent->pTarget != pWidget ) return XUI_OK;
		return __xuiRangeSliderPointerDown(pWidget, pData, pEvent);
	case XUI_EVENT_POINTER_UP:
		return __xuiRangeSliderPointerUp(pWidget, pData, pEvent);
	case XUI_EVENT_POINTER_CLICK:
		if ( (pEvent->pTarget == pWidget) && (pEvent->iButton == XUI_POINTER_BUTTON_LEFT) ) {
			return XUI_EVENT_DISPATCH_STOP;
		}
		break;
	case XUI_EVENT_POINTER_WHEEL:
		if ( pEvent->pTarget != pWidget ) return XUI_OK;
		return __xuiRangeSliderWheel(pWidget, pData, pEvent);
	case XUI_EVENT_POINTER_CAPTURE_LOST:
		if ( __xuiRangeSliderPointerMatches(pData, pEvent) ) {
			pData->bDragging = 0;
			pData->bPointerActive = 0;
			pData->iActivePointerId = 0;
			pData->iActivePointerType = XUI_POINTER_TYPE_MOUSE;
			pData->fDragOffset = 0.0f;
			return __xuiRangeSliderSyncState(pWidget, pData);
		}
		return XUI_OK;
	case XUI_EVENT_FOCUS:
		return __xuiRangeSliderSyncState(pWidget, pData);
	case XUI_EVENT_BLUR:
		__xuiRangeSliderCancelDrag(pWidget, pData);
		return __xuiRangeSliderSyncState(pWidget, pData);
	case XUI_EVENT_KEY_DOWN:
		if ( pEvent->pTarget != pWidget ) return XUI_OK;
		return __xuiRangeSliderKeyDown(pWidget, pData, pEvent);
	case XUI_EVENT_ENABLED_CHANGED:
	case XUI_EVENT_VISIBLE_CHANGED:
		if ( !__xuiRangeSliderCanInteract(pWidget) ) {
			__xuiRangeSliderCancelDrag(pWidget, pData);
		}
		iRet = __xuiRangeSliderSyncState(pWidget, pData);
		if ( iRet != XUI_OK ) return iRet;
		return xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	default:
		break;
	}
	return XUI_OK;
}

static int __xuiRangeSliderDrawRectFill(xui_proxy pProxy, xui_draw_context pDraw, xui_rect_t tRect, uint32_t iColor)
{
	if ( (tRect.fW <= 0.0f) || (tRect.fH <= 0.0f) || (__xuiRangeSliderColorAlpha(iColor) == 0) ) {
		return XUI_OK;
	}
	return pProxy->drawRectFill(pProxy, pDraw, tRect, iColor);
}

static int __xuiRangeSliderDrawRectStroke(xui_proxy pProxy, xui_draw_context pDraw, xui_rect_t tRect, float fWidth, uint32_t iColor)
{
	if ( (tRect.fW <= 0.0f) || (tRect.fH <= 0.0f) || (fWidth <= 0.0f) || (__xuiRangeSliderColorAlpha(iColor) == 0) ) {
		return XUI_OK;
	}
	return pProxy->drawRectStroke(pProxy, pDraw, tRect, fWidth, iColor);
}

static uint32_t __xuiRangeSliderFillForState(const xui_range_slider_data_t* pData, uint32_t iStateId)
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

static int __xuiRangeSliderDrawKnob(xui_proxy pProxy, xui_draw_context pDraw, xui_rect_t tRect, uint32_t iKnobColor, uint32_t iBorderColor)
{
	int iRet;

	iRet = __xuiRangeSliderDrawRectFill(pProxy, pDraw, tRect, iKnobColor);
	if ( iRet == XUI_OK ) {
		iRet = __xuiRangeSliderDrawRectStroke(pProxy, pDraw, tRect, 1.0f, iBorderColor);
	}
	return iRet;
}

static int __xuiRangeSliderCacheRender(xui_widget pWidget, xui_draw_context pDraw, uint32_t iStateId, void* pUser)
{
	xui_range_slider_data_t* pData;
	xui_range_slider_data_t tResolved;
	xui_range_slider_geometry_t tGeometry;
	xui_proxy pProxy;
	xui_rect_t tContent;
	xui_rect_t tFocus;
	xui_rect_t tTrack;
	xui_rect_t tFill;
	xui_rect_t tStartKnob;
	xui_rect_t tEndKnob;
	uint32_t iTrackColor;
	uint32_t iFillColor;
	uint32_t iKnobColor;
	uint32_t iBorderColor;
	int iRet;

	(void)pUser;
	pData = __xuiRangeSliderGetData(pWidget);
	if ( (pData == NULL) || (pDraw == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pProxy = xuiInternalContextGetProxy(xuiWidgetGetContext(pWidget));
	if ( pProxy == NULL ) {
		return XUI_ERROR_NOT_INITIALIZED;
	}
	__xuiRangeSliderResolve(pWidget, pData, &tResolved);
	__xuiRangeSliderComputeGeometry(pWidget, &tResolved, &tGeometry);
	tTrack = xuiSliderCommonOutputRect(tGeometry.tTrack);
	tFill = xuiSliderCommonOutputRect(tGeometry.tFill);
	tStartKnob = xuiSliderCommonOutputRect(tGeometry.tStartKnob);
	tEndKnob = xuiSliderCommonOutputRect(tGeometry.tEndKnob);
	tContent = xuiWidgetGetContentRect(pWidget);
	iTrackColor = ((iStateId & XUI_WIDGET_STATE_DISABLED) != 0) ? __xuiRangeSliderColorWithAlpha(tResolved.iDisabledColor, 72) : tResolved.iTrackColor;
	iFillColor = __xuiRangeSliderFillForState(&tResolved, iStateId);
	iKnobColor = ((iStateId & XUI_WIDGET_STATE_DISABLED) != 0) ? tResolved.iDisabledKnobColor : tResolved.iKnobColor;
	iBorderColor = ((iStateId & XUI_WIDGET_STATE_DISABLED) != 0) ? __xuiRangeSliderColorWithAlpha(tResolved.iKnobBorderColor, 105) : tResolved.iKnobBorderColor;
	iRet = __xuiRangeSliderDrawRectFill(pProxy, pDraw, tTrack, iTrackColor);
	if ( iRet != XUI_OK ) return iRet;
	iRet = __xuiRangeSliderDrawRectFill(pProxy, pDraw, tFill, iFillColor);
	if ( iRet != XUI_OK ) return iRet;
	if ( pData->iActiveThumb == XUI_RANGE_SLIDER_THUMB_START ) {
		iRet = __xuiRangeSliderDrawKnob(pProxy, pDraw, tEndKnob, iKnobColor, iBorderColor);
		if ( iRet != XUI_OK ) return iRet;
		iRet = __xuiRangeSliderDrawKnob(pProxy, pDraw, tStartKnob, iKnobColor, iBorderColor);
	} else {
		iRet = __xuiRangeSliderDrawKnob(pProxy, pDraw, tStartKnob, iKnobColor, iBorderColor);
		if ( iRet != XUI_OK ) return iRet;
		iRet = __xuiRangeSliderDrawKnob(pProxy, pDraw, tEndKnob, iKnobColor, iBorderColor);
	}
	if ( iRet != XUI_OK ) return iRet;
	if ( ((iStateId & XUI_WIDGET_STATE_FOCUS) != 0) && ((iStateId & XUI_WIDGET_STATE_DISABLED) == 0) ) {
		tFocus = xuiInternalInsetRect(tContent, 1.0f);
		iRet = __xuiRangeSliderDrawRectStroke(pProxy, pDraw, tFocus, 1.5f, tResolved.iFocusColor);
		if ( iRet != XUI_OK ) return iRet;
	}
	return XUI_OK;
}

static int __xuiRangeSliderContentMeasure(xui_widget pWidget, xui_vec2_t tConstraint, xui_vec2_t* pSize, void* pUser)
{
	xui_range_slider_data_t* pData;
	xui_range_slider_data_t tResolved;
	float fCross;
	float fLong;

	(void)tConstraint;
	(void)pUser;
	if ( pSize == NULL ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pData = __xuiRangeSliderGetData(pWidget);
	if ( pData == NULL ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	__xuiRangeSliderResolve(pWidget, pData, &tResolved);
	fCross = __xuiRangeSliderMaxFloat(tResolved.fKnobSize, tResolved.fTrackSize);
	if ( fCross < 18.0f ) fCross = 18.0f;
	fLong = __xuiRangeSliderMaxFloat(tResolved.fKnobSize * 8.0f, 144.0f);
	if ( fCross > XUI_LAYOUT_UNBOUNDED ) fCross = XUI_LAYOUT_UNBOUNDED;
	if ( fLong > XUI_LAYOUT_UNBOUNDED ) fLong = XUI_LAYOUT_UNBOUNDED;
	if ( tResolved.iOrientation == XUI_ORIENTATION_HORIZONTAL ) {
		*pSize = (xui_vec2_t){fLong, fCross};
	} else {
		*pSize = (xui_vec2_t){fCross, fLong};
	}
	return XUI_OK;
}

static void __xuiRangeSliderDefaultLayout(xui_layout_t* pLayout)
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

static void __xuiRangeSliderDefaultCachePolicy(xui_cache_policy_t* pPolicy)
{
	memset(pPolicy, 0, sizeof(*pPolicy));
	pPolicy->iSize = sizeof(*pPolicy);
	pPolicy->iPolicy = XUI_CACHE_POLICY_SELF;
	pPolicy->iFlags = XUI_CACHE_CLEAR_ON_UPDATE | XUI_CACHE_UPDATE_ALL_STATES;
	pPolicy->iClearColor = XUI_COLOR_RGBA(0, 0, 0, 0);
}

static int __xuiRangeSliderInitCacheStates(xui_widget pWidget)
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
		iRet = xuiWidgetSetCacheStateId(pWidget, i, 0x7a000000u + (uint32_t)i);
		if ( iRet != XUI_OK ) return iRet;
	}
	for ( i = 0; i < (int)(sizeof(arrState) / sizeof(arrState[0])); i++ ) {
		iRet = xuiWidgetSetCacheStateId(pWidget, i, arrState[i]);
		if ( iRet != XUI_OK ) return iRet;
	}
	return XUI_OK;
}

static int __xuiRangeSliderAccessibleCount(xui_widget pWidget, void* pUser)
{
	return __xuiRangeSliderGetData(pWidget) == (xui_range_slider_data_t*)pUser ? 3 : 0;
}

static void __xuiRangeSliderAccessibleBounds(const xui_range_slider_data_t* pData,
	int iThumb, double* pMin, double* pMax)
{
	double fMinInterval;
	double fMaxInterval;

	fMinInterval = __xuiRangeSliderEffectiveMinInterval(pData->fMinInterval,
		pData->fMin, pData->fMax);
	fMaxInterval = __xuiRangeSliderEffectiveMaxInterval(pData->fMaxInterval,
		pData->fMinInterval, pData->fMin, pData->fMax);
	if ( iThumb == XUI_RANGE_SLIDER_THUMB_START ) {
		*pMin = fmax((double)pData->fMin, (double)pData->fEnd - fMaxInterval);
		*pMax = (double)pData->fEnd - fMinInterval;
	} else {
		*pMin = (double)pData->fStart + fMinInterval;
		*pMax = fmin((double)pData->fMax, (double)pData->fStart + fMaxInterval);
	}
	if ( *pMax < *pMin ) *pMax = *pMin;
}

static int __xuiRangeSliderAccessibleGet(xui_widget pWidget, int iIndex,
	xui_accessible_node_t* pNode, void* pUser)
{
	xui_range_slider_data_t* pData = (xui_range_slider_data_t*)pUser;
	xui_range_slider_data_t tResolved;
	xui_range_slider_geometry_t tGeometry;
	xui_rect_t tWorld;
	xui_rect_t tKnob;
	char* sValue;
	double fMin;
	double fMax;
	int iThumb;

	if ( (pNode == NULL) || (iIndex < 0) || (iIndex > 2) ||
	     (__xuiRangeSliderGetData(pWidget) != pData) ) return XUI_ERROR_INVALID_ARGUMENT;
	tWorld = xuiWidgetGetWorldRect(pWidget);
	if ( iIndex == 0 ) {
		(void)snprintf(pData->sAccessibleRootValue, sizeof(pData->sAccessibleRootValue),
			"%.9g - %.9g", (double)pData->fStart, (double)pData->fEnd);
		pNode->iId = XUI_RANGE_SLIDER_ACCESSIBLE_ROOT_ID;
		pNode->iRole = XUI_ACCESSIBLE_ROLE_GROUP;
		pNode->sName = xuiWidgetGetAccessibleName(pWidget) != NULL ?
			xuiWidgetGetAccessibleName(pWidget) : "Range slider";
		pNode->sDescription = xuiWidgetGetAccessibleDescription(pWidget);
		pNode->sValue = pData->sAccessibleRootValue;
		pNode->tBounds = tWorld;
		pNode->iColumnCount = 2;
		pNode->iState |= XUI_ACCESSIBLE_STATE_FOCUSABLE;
		if ( pData->iOrientation == XUI_ORIENTATION_VERTICAL )
			pNode->iState |= XUI_ACCESSIBLE_STATE_VERTICAL;
		pNode->iActions = XUI_ACCESSIBLE_ACTION_MASK(XUI_ACCESSIBLE_ACTION_FOCUS);
		return XUI_OK;
	}
	iThumb = iIndex == 1 ? XUI_RANGE_SLIDER_THUMB_START : XUI_RANGE_SLIDER_THUMB_END;
	__xuiRangeSliderResolve(pWidget, pData, &tResolved);
	__xuiRangeSliderComputeGeometry(pWidget, &tResolved, &tGeometry);
	tKnob = iThumb == XUI_RANGE_SLIDER_THUMB_START ?
		xuiSliderCommonOutputRect(tGeometry.tStartKnob) :
		xuiSliderCommonOutputRect(tGeometry.tEndKnob);
	sValue = iThumb == XUI_RANGE_SLIDER_THUMB_START ?
		pData->sAccessibleStartValue : pData->sAccessibleEndValue;
	(void)snprintf(sValue, 48u, "%.9g",
		(double)(iThumb == XUI_RANGE_SLIDER_THUMB_START ? pData->fStart : pData->fEnd));
	__xuiRangeSliderAccessibleBounds(pData, iThumb, &fMin, &fMax);
	pNode->iId = iThumb == XUI_RANGE_SLIDER_THUMB_START ?
		XUI_RANGE_SLIDER_ACCESSIBLE_START_ID : XUI_RANGE_SLIDER_ACCESSIBLE_END_ID;
	pNode->iParentId = XUI_RANGE_SLIDER_ACCESSIBLE_ROOT_ID;
	pNode->iRole = XUI_ACCESSIBLE_ROLE_SLIDER;
	pNode->sName = iThumb == XUI_RANGE_SLIDER_THUMB_START ? "Start" : "End";
	pNode->sValue = sValue;
	pNode->tBounds = (xui_rect_t){tWorld.fX + tKnob.fX, tWorld.fY + tKnob.fY,
		tKnob.fW, tKnob.fH};
	pNode->iColumn = iIndex - 1;
	pNode->iColumnCount = 1;
	pNode->iState |= XUI_ACCESSIBLE_STATE_FOCUSABLE;
	if ( pData->iOrientation == XUI_ORIENTATION_VERTICAL )
		pNode->iState |= XUI_ACCESSIBLE_STATE_VERTICAL;
	if ( xuiGetFocusWidget(xuiWidgetGetContext(pWidget)) == pWidget &&
	     pData->iActiveThumb == iThumb ) pNode->iState |= XUI_ACCESSIBLE_STATE_FOCUSED;
	pNode->tRange.iSize = sizeof(pNode->tRange);
	pNode->tRange.fValue = iThumb == XUI_RANGE_SLIDER_THUMB_START ?
		pData->fStart : pData->fEnd;
	pNode->tRange.fMin = fMin;
	pNode->tRange.fMax = fMax;
	pNode->tRange.fStep = __xuiRangeSliderSmallStep(pData);
	pNode->tRange.fPageStep = __xuiRangeSliderPageStep(pData);
	pNode->iActions = XUI_ACCESSIBLE_ACTION_MASK(XUI_ACCESSIBLE_ACTION_FOCUS) |
		XUI_ACCESSIBLE_ACTION_MASK(XUI_ACCESSIBLE_ACTION_SET_VALUE) |
		XUI_ACCESSIBLE_ACTION_MASK(XUI_ACCESSIBLE_ACTION_INCREMENT) |
		XUI_ACCESSIBLE_ACTION_MASK(XUI_ACCESSIBLE_ACTION_DECREMENT);
	return XUI_OK;
}

static int __xuiRangeSliderAccessibleAction(xui_widget pWidget, uint64_t iNodeId,
	int iAction, const void* pPayload, void* pUser)
{
	xui_range_slider_data_t* pData = (xui_range_slider_data_t*)pUser;
	const xui_accessible_value_t* pValue;
	double fMin;
	double fMax;
	double fValue;
	int iThumb;
	int iRet;

	if ( __xuiRangeSliderGetData(pWidget) != pData ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( !__xuiRangeSliderCanInteract(pWidget) )
		return XUI_ERROR_INVALID_STATE;
	if ( iNodeId == 0 || iNodeId == XUI_RANGE_SLIDER_ACCESSIBLE_ROOT_ID ) {
		return iAction == XUI_ACCESSIBLE_ACTION_FOCUS ?
			xuiSetFocusWidget(xuiWidgetGetContext(pWidget), pWidget) : XUI_ERROR_UNSUPPORTED;
	}
	iThumb = iNodeId == XUI_RANGE_SLIDER_ACCESSIBLE_START_ID ?
		XUI_RANGE_SLIDER_THUMB_START :
		(iNodeId == XUI_RANGE_SLIDER_ACCESSIBLE_END_ID ? XUI_RANGE_SLIDER_THUMB_END :
		 XUI_RANGE_SLIDER_THUMB_NONE);
	if ( iThumb == XUI_RANGE_SLIDER_THUMB_NONE ) return XUI_ERROR_NOT_FOUND;
	if ( iAction == XUI_ACCESSIBLE_ACTION_FOCUS ) {
		iRet = __xuiRangeSliderSetActiveThumb(pWidget, pData, iThumb);
		if ( iRet != XUI_OK ) return iRet;
		return xuiSetFocusWidget(xuiWidgetGetContext(pWidget), pWidget);
	}
	__xuiRangeSliderAccessibleBounds(pData, iThumb, &fMin, &fMax);
	if ( iAction == XUI_ACCESSIBLE_ACTION_SET_VALUE ) {
		pValue = (const xui_accessible_value_t*)pPayload;
		if ( pValue == NULL || pValue->iSize < sizeof(*pValue) ||
		     pValue->iType != XUI_ACCESSIBLE_VALUE_NUMBER || !isfinite(pValue->fNumber) )
			return XUI_ERROR_INVALID_ARGUMENT;
		fValue = pValue->fNumber;
		if ( fValue < fMin || fValue > fMax ) return XUI_ERROR_INVALID_ARGUMENT;
	} else if ( iAction == XUI_ACCESSIBLE_ACTION_INCREMENT ||
	            iAction == XUI_ACCESSIBLE_ACTION_DECREMENT ) {
		fValue = xuiSliderCommonValueWithDelta(
			iThumb == XUI_RANGE_SLIDER_THUMB_START ? pData->fStart : pData->fEnd,
			pData->fMin, pData->fMax,
			(iAction == XUI_ACCESSIBLE_ACTION_INCREMENT ? 1.0 : -1.0) *
			(double)__xuiRangeSliderSmallStep(pData));
	} else {
		return XUI_ERROR_UNSUPPORTED;
	}
	iRet = __xuiRangeSliderSetActiveThumb(pWidget, pData, iThumb);
	if ( iRet != XUI_OK ) return iRet;
	return __xuiRangeSliderSetThumbValueInternal(pWidget, pData, iThumb, (float)fValue, 1);
}

static int __xuiRangeSliderInitEvents(xui_widget pWidget)
{
	int iRet;

	iRet = xuiWidgetSetCancelAction(pWidget, __xuiRangeSliderCancelAction, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_POINTER_ENTER, __xuiRangeSliderEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_POINTER_LEAVE, __xuiRangeSliderEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_POINTER_MOVE, __xuiRangeSliderEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_POINTER_DOWN, __xuiRangeSliderEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_POINTER_UP, __xuiRangeSliderEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_POINTER_CLICK, __xuiRangeSliderEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_POINTER_WHEEL, __xuiRangeSliderEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_POINTER_CAPTURE_LOST, __xuiRangeSliderEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_FOCUS, __xuiRangeSliderEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_BLUR, __xuiRangeSliderEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_KEY_DOWN, __xuiRangeSliderEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_ENABLED_CHANGED, __xuiRangeSliderEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_VISIBLE_CHANGED, __xuiRangeSliderEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetAccessibilityProvider(pWidget,
		__xuiRangeSliderAccessibleCount, __xuiRangeSliderAccessibleGet,
		__xuiRangeSliderAccessibleAction, __xuiRangeSliderGetData(pWidget));
	return iRet;
}

static int __xuiRangeSliderInit(xui_widget pWidget, void* pTypeData, const void* pCreateData, void* pUser)
{
	xui_range_slider_data_t* pData;
	const xui_range_slider_desc_t* pDesc;
	float fMin;
	float fMax;
	float fStart;
	float fEnd;
	float fMaxInterval;
	float fMinInterval;
	uint32_t iFill;
	int iRet;

	(void)pUser;
	pData = (xui_range_slider_data_t*)pTypeData;
	pDesc = (const xui_range_slider_desc_t*)pCreateData;
	if ( (pWidget == NULL) || (pData == NULL) || !__xuiRangeSliderDescValid(pDesc) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	memset(pData, 0, sizeof(*pData));
	pData->iActivePointerType = XUI_POINTER_TYPE_MOUSE;
	fMin = (pDesc != NULL) ? pDesc->fMin : 0.0f;
	fMax = (pDesc != NULL) ? pDesc->fMax : 1.0f;
	if ( (pDesc == NULL) || (fMin == 0.0f && fMax == 0.0f) ) {
		fMax = 1.0f;
	}
	__xuiRangeSliderNormalizeRange(&fMin, &fMax);
	fMinInterval = (pDesc != NULL && pDesc->fMinInterval > 0.0f) ? pDesc->fMinInterval : 0.0f;
	fMaxInterval = (pDesc != NULL && pDesc->fMaxInterval > 0.0f) ? pDesc->fMaxInterval : 0.0f;
	if ( (fMaxInterval > 0.0f) && (fMinInterval > fMaxInterval) ) {
		fMinInterval = fMaxInterval;
	}
	fStart = (pDesc != NULL) ? pDesc->fStart : fMin;
	fEnd = (pDesc != NULL) ? pDesc->fEnd : fMax;
	__xuiRangeSliderNormalizeValues(&fStart, &fEnd, fMin, fMax, fMinInterval, fMaxInterval);
	iFill = (pDesc != NULL && pDesc->iFillColor != 0) ? pDesc->iFillColor : XUI_COLOR_RGBA(0, 184, 169, 255);
	pData->fMin = fMin;
	pData->fMax = fMax;
	pData->fStart = fStart;
	pData->fEnd = fEnd;
	pData->fStep = (pDesc != NULL && pDesc->fStep > 0.0f) ? pDesc->fStep : 0.0f;
	pData->fPageStep = (pDesc != NULL && pDesc->fPageStep > 0.0f) ? pDesc->fPageStep : 0.0f;
	pData->fMinInterval = fMinInterval;
	pData->fMaxInterval = fMaxInterval;
	pData->fTrackSize = (pDesc != NULL && pDesc->fTrackSize > 0.0f) ? pDesc->fTrackSize : 4.0f;
	pData->fKnobSize = (pDesc != NULL && pDesc->fKnobSize > 0.0f) ? pDesc->fKnobSize : 14.0f;
	pData->iTrackColor = (pDesc != NULL && pDesc->iTrackColor != 0) ? pDesc->iTrackColor : XUI_COLOR_RGBA(229, 231, 235, 255);
	pData->iFillColor = iFill;
	pData->iFillHoverColor = __xuiRangeSliderLighten(iFill, 12);
	pData->iFillActiveColor = __xuiRangeSliderLighten(iFill, -18);
	pData->iKnobColor = (pDesc != NULL && pDesc->iKnobColor != 0) ? pDesc->iKnobColor : XUI_COLOR_RGBA(255, 255, 255, 255);
	pData->iKnobBorderColor = (pDesc != NULL && pDesc->iKnobBorderColor != 0) ? pDesc->iKnobBorderColor : iFill;
	pData->iFocusColor = (pDesc != NULL && pDesc->iFocusColor != 0) ? pDesc->iFocusColor : __xuiRangeSliderColorWithAlpha(iFill, 130);
	pData->iDisabledColor = (pDesc != NULL && pDesc->iDisabledColor != 0) ? pDesc->iDisabledColor : XUI_COLOR_RGBA(172, 184, 200, 150);
	pData->iOrientation = (pDesc != NULL && __xuiRangeSliderOrientationValid(pDesc->iOrientation)) ? pDesc->iOrientation : XUI_ORIENTATION_HORIZONTAL;
	pData->iActiveThumb = XUI_RANGE_SLIDER_THUMB_START;
	iRet = xuiWidgetSetPadding(pWidget, (xui_thickness_t){0.0f, 0.0f, 0.0f, 0.0f});
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiWidgetSetOverflow(pWidget, XUI_OVERFLOW_CLIP);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiWidgetSetFocusable(pWidget, 1);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiWidgetSetTabStop(pWidget, 1);
	if ( iRet != XUI_OK ) return iRet;
	iRet = __xuiRangeSliderInitCacheStates(pWidget);
	if ( iRet != XUI_OK ) return iRet;
	iRet = __xuiRangeSliderInitEvents(pWidget);
	if ( iRet != XUI_OK ) return iRet;
	return __xuiRangeSliderSyncState(pWidget, pData);
}

static void __xuiRangeSliderDestroy(xui_widget pWidget, void* pTypeData, void* pUser)
{
	xui_range_slider_data_t* pData;

	(void)pUser;
	pData = (xui_range_slider_data_t*)pTypeData;
	if ( pData != NULL ) {
		__xuiRangeSliderCancelDrag(pWidget, pData);
		memset(pData, 0, sizeof(*pData));
	}
}

static void __xuiRangeSliderRegisterStyleProperty(xui_context pContext, xui_widget_type pType, const char* sName, int iValueType, uint32_t iDirtyFlags, uint32_t iFlags)
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

static void __xuiRangeSliderRegisterStyleProperties(xui_context pContext, xui_widget_type pType)
{
	uint32_t iPaintDirty;
	uint32_t iLayoutDirty;

	iPaintDirty = XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER;
	iLayoutDirty = XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER;
	__xuiRangeSliderRegisterStyleProperty(pContext, pType, "rangeslider.track.color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiRangeSliderRegisterStyleProperty(pContext, pType, "rangeslider.fill.color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiRangeSliderRegisterStyleProperty(pContext, pType, "rangeslider.fill.hover_color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiRangeSliderRegisterStyleProperty(pContext, pType, "rangeslider.fill.active_color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiRangeSliderRegisterStyleProperty(pContext, pType, "rangeslider.knob.color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiRangeSliderRegisterStyleProperty(pContext, pType, "rangeslider.knob.disabled_color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiRangeSliderRegisterStyleProperty(pContext, pType, "rangeslider.knob.border_color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiRangeSliderRegisterStyleProperty(pContext, pType, "rangeslider.focus.color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiRangeSliderRegisterStyleProperty(pContext, pType, "rangeslider.disabled.color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiRangeSliderRegisterStyleProperty(pContext, pType, "rangeslider.track.size", XUI_STYLE_VALUE_FLOAT, iLayoutDirty, 0);
	__xuiRangeSliderRegisterStyleProperty(pContext, pType, "rangeslider.knob.size", XUI_STYLE_VALUE_FLOAT, iLayoutDirty, 0);
}

XUI_API xui_widget_type xuiRangeSliderGetType(xui_context pContext)
{
	xui_widget_type_desc_t tDesc;
	xui_widget_type pType;
	int iRet;

	if ( !xuiInternalContextIsValid(pContext) ) {
		return NULL;
	}
	pType = xuiWidgetFindType(pContext, "rangeslider");
	if ( pType != NULL ) {
		__xuiRangeSliderRegisterStyleProperties(pContext, pType);
		return pType;
	}
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.sName = "rangeslider";
	tDesc.pParent = xuiWidgetGetBaseType();
	tDesc.iFlags = XUI_WIDGET_TYPE_DEFAULT_LAYOUT | XUI_WIDGET_TYPE_DEFAULT_CACHE_POLICY;
	tDesc.iTypeDataSize = sizeof(xui_range_slider_data_t);
	tDesc.onInit = __xuiRangeSliderInit;
	tDesc.onDestroy = __xuiRangeSliderDestroy;
	tDesc.onContentMeasure = __xuiRangeSliderContentMeasure;
	tDesc.onCacheRender = __xuiRangeSliderCacheRender;
	__xuiRangeSliderDefaultLayout(&tDesc.tLayout);
	__xuiRangeSliderDefaultCachePolicy(&tDesc.tCachePolicy);
	iRet = xuiWidgetRegisterType(pContext, &pType, &tDesc);
	if ( iRet != XUI_OK ) {
		return NULL;
	}
	__xuiRangeSliderRegisterStyleProperties(pContext, pType);
	return pType;
}

XUI_API int xuiRangeSliderCreate(xui_context pContext, xui_widget* ppWidget, const xui_range_slider_desc_t* pDesc)
{
	xui_widget_type pType;

	if ( ppWidget == NULL ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	*ppWidget = NULL;
	if ( !xuiInternalContextIsValid(pContext) || !__xuiRangeSliderDescValid(pDesc) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pType = xuiRangeSliderGetType(pContext);
	if ( pType == NULL ) {
		return XUI_ERROR_NOT_INITIALIZED;
	}
	return xuiWidgetCreateTyped(pContext, pType, ppWidget, pDesc);
}

XUI_API int xuiRangeSliderSetChange(xui_widget pWidget, xui_range_slider_change_proc onChange, void* pUser)
{
	xui_range_slider_data_t* pData = __xuiRangeSliderGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	pData->onChange = onChange;
	pData->pChangeUser = pUser;
	return XUI_OK;
}

XUI_API int xuiRangeSliderSetRange(xui_widget pWidget, float fMin, float fMax)
{
	xui_range_slider_data_t* pData;
	float fStart;
	float fEnd;
	int bValuesChanged;
	int iRet;

	pData = __xuiRangeSliderGetData(pWidget);
	if ( pData == NULL || !isfinite(fMin) || !isfinite(fMax) ) return XUI_ERROR_INVALID_ARGUMENT;
	__xuiRangeSliderNormalizeRange(&fMin, &fMax);
	if ( pData->fMin == fMin && pData->fMax == fMax ) return XUI_OK;
	fStart = pData->fStart;
	fEnd = pData->fEnd;
	__xuiRangeSliderNormalizeValues(&fStart, &fEnd, fMin, fMax,
		pData->fMinInterval, pData->fMaxInterval);
	bValuesChanged = pData->fStart != fStart || pData->fEnd != fEnd;
	iRet = xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	if ( iRet != XUI_OK ) return iRet;
	__xuiRangeSliderCancelDrag(pWidget, pData);
	pData->fMin = fMin;
	pData->fMax = fMax;
	pData->fStart = fStart;
	pData->fEnd = fEnd;
	xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_NODE_CHANGED);
	xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_BOUNDS_CHANGED);
	if ( bValuesChanged ) {
		__xuiRangeSliderIncrementCounter(&pData->iChangeCount);
		xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_VALUE_CHANGED);
	}
	return XUI_OK;
}

XUI_API int xuiRangeSliderGetRange(xui_widget pWidget, float* pMin, float* pMax)
{
	xui_range_slider_data_t* pData = __xuiRangeSliderGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pMin != NULL ) *pMin = pData->fMin;
	if ( pMax != NULL ) *pMax = pData->fMax;
	return XUI_OK;
}

XUI_API int xuiRangeSliderSetValues(xui_widget pWidget, float fStart, float fEnd)
{
	xui_range_slider_data_t* pData = __xuiRangeSliderGetData(pWidget);
	if ( pData == NULL || !isfinite(fStart) || !isfinite(fEnd) ) return XUI_ERROR_INVALID_ARGUMENT;
	return __xuiRangeSliderSetValuesInternal(pWidget, pData, fStart, fEnd, 0);
}

XUI_API int xuiRangeSliderGetValues(xui_widget pWidget, float* pStart, float* pEnd)
{
	xui_range_slider_data_t* pData = __xuiRangeSliderGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pStart != NULL ) *pStart = pData->fStart;
	if ( pEnd != NULL ) *pEnd = pData->fEnd;
	return XUI_OK;
}

XUI_API float xuiRangeSliderGetStart(xui_widget pWidget)
{
	xui_range_slider_data_t* pData = __xuiRangeSliderGetData(pWidget);
	return (pData != NULL) ? pData->fStart : 0.0f;
}

XUI_API float xuiRangeSliderGetEnd(xui_widget pWidget)
{
	xui_range_slider_data_t* pData = __xuiRangeSliderGetData(pWidget);
	return (pData != NULL) ? pData->fEnd : 0.0f;
}

XUI_API float xuiRangeSliderGetStartRate(xui_widget pWidget)
{
	xui_range_slider_data_t* pData = __xuiRangeSliderGetData(pWidget);
	return __xuiRangeSliderRateFromValue(pData, (pData != NULL) ? pData->fStart : 0.0f);
}

XUI_API float xuiRangeSliderGetEndRate(xui_widget pWidget)
{
	xui_range_slider_data_t* pData = __xuiRangeSliderGetData(pWidget);
	return __xuiRangeSliderRateFromValue(pData, (pData != NULL) ? pData->fEnd : 0.0f);
}

XUI_API int xuiRangeSliderSetStep(xui_widget pWidget, float fStep, float fPageStep)
{
	xui_range_slider_data_t* pData = __xuiRangeSliderGetData(pWidget);
	if ( (pData == NULL) || !isfinite(fStep) || !isfinite(fPageStep) ||
	     (fStep < 0.0f) || (fPageStep < 0.0f) ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pData->fStep == fStep && pData->fPageStep == fPageStep ) return XUI_OK;
	pData->fStep = fStep;
	pData->fPageStep = fPageStep;
	xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_NODE_CHANGED);
	return XUI_OK;
}

XUI_API int xuiRangeSliderGetStep(xui_widget pWidget, float* pStep, float* pPageStep)
{
	xui_range_slider_data_t* pData = __xuiRangeSliderGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pStep != NULL ) *pStep = pData->fStep;
	if ( pPageStep != NULL ) *pPageStep = pData->fPageStep;
	return XUI_OK;
}

XUI_API int xuiRangeSliderSetIntervalLimits(xui_widget pWidget, float fMinInterval, float fMaxInterval)
{
	xui_range_slider_data_t* pData;
	float fStart;
	float fEnd;
	int bValuesChanged;
	int iRet;

	pData = __xuiRangeSliderGetData(pWidget);
	if ( (pData == NULL) || !isfinite(fMinInterval) || !isfinite(fMaxInterval) ||
	     (fMinInterval < 0.0f) || (fMaxInterval < 0.0f) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( (fMaxInterval > 0.0f) && (fMinInterval > fMaxInterval) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( pData->fMinInterval == fMinInterval && pData->fMaxInterval == fMaxInterval ) return XUI_OK;
	fStart = pData->fStart;
	fEnd = pData->fEnd;
	__xuiRangeSliderNormalizeValues(&fStart, &fEnd, pData->fMin, pData->fMax,
		fMinInterval, fMaxInterval);
	bValuesChanged = pData->fStart != fStart || pData->fEnd != fEnd;
	iRet = xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	if ( iRet != XUI_OK ) return iRet;
	__xuiRangeSliderCancelDrag(pWidget, pData);
	pData->fMinInterval = fMinInterval;
	pData->fMaxInterval = fMaxInterval;
	pData->fStart = fStart;
	pData->fEnd = fEnd;
	xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_NODE_CHANGED);
	if ( bValuesChanged ) {
		__xuiRangeSliderIncrementCounter(&pData->iChangeCount);
		xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_VALUE_CHANGED);
		xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_BOUNDS_CHANGED);
	}
	return XUI_OK;
}

XUI_API int xuiRangeSliderGetIntervalLimits(xui_widget pWidget, float* pMinInterval, float* pMaxInterval)
{
	xui_range_slider_data_t* pData;

	pData = __xuiRangeSliderGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pMinInterval != NULL ) *pMinInterval = pData->fMinInterval;
	if ( pMaxInterval != NULL ) *pMaxInterval = pData->fMaxInterval;
	return XUI_OK;
}

XUI_API int xuiRangeSliderSetOrientation(xui_widget pWidget, int iOrientation)
{
	xui_range_slider_data_t* pData = __xuiRangeSliderGetData(pWidget);
	int iRet;
	if ( (pData == NULL) || !__xuiRangeSliderOrientationValid(iOrientation) ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pData->iOrientation == iOrientation ) return XUI_OK;
	iRet = xuiWidgetInvalidate(pWidget,
		XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	if ( iRet != XUI_OK ) return iRet;
	__xuiRangeSliderCancelDrag(pWidget, pData);
	pData->iOrientation = iOrientation;
	xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_STATE_CHANGED);
	xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_BOUNDS_CHANGED);
	return XUI_OK;
}

XUI_API int xuiRangeSliderGetOrientation(xui_widget pWidget)
{
	xui_range_slider_data_t* pData = __xuiRangeSliderGetData(pWidget);
	return (pData != NULL) ? pData->iOrientation : XUI_ORIENTATION_HORIZONTAL;
}

XUI_API int xuiRangeSliderSetMetrics(xui_widget pWidget, float fTrackSize, float fKnobSize)
{
	xui_range_slider_data_t* pData = __xuiRangeSliderGetData(pWidget);
	int iRet;
	if ( (pData == NULL) || !xuiSliderCommonMetricValid(fTrackSize, 0.0f) ||
	     !xuiSliderCommonMetricValid(fKnobSize, 0.0f) )
		return XUI_ERROR_INVALID_ARGUMENT;
	fTrackSize = (fTrackSize > 0.0f) ? fTrackSize : 4.0f;
	fKnobSize = (fKnobSize > 0.0f) ? fKnobSize : 14.0f;
	if ( pData->fTrackSize == fTrackSize && pData->fKnobSize == fKnobSize ) return XUI_OK;
	iRet = xuiWidgetInvalidate(pWidget,
		XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	if ( iRet != XUI_OK ) return iRet;
	__xuiRangeSliderCancelDrag(pWidget, pData);
	pData->fTrackSize = fTrackSize;
	pData->fKnobSize = fKnobSize;
	xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_BOUNDS_CHANGED);
	return XUI_OK;
}

XUI_API int xuiRangeSliderGetMetrics(xui_widget pWidget, float* pTrackSize, float* pKnobSize)
{
	xui_range_slider_data_t* pData = __xuiRangeSliderGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pTrackSize != NULL ) *pTrackSize = pData->fTrackSize;
	if ( pKnobSize != NULL ) *pKnobSize = pData->fKnobSize;
	return XUI_OK;
}

XUI_API int xuiRangeSliderSetColors(xui_widget pWidget, uint32_t iTrack, uint32_t iFill, uint32_t iKnob, uint32_t iFocus, uint32_t iDisabled)
{
	xui_range_slider_data_t* pData = __xuiRangeSliderGetData(pWidget);
	int iRet;
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pData->iTrackColor == iTrack && pData->iFillColor == iFill &&
	     pData->iKnobColor == iKnob && pData->iFocusColor == iFocus &&
	     pData->iDisabledColor == iDisabled ) return XUI_OK;
	iRet = xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	if ( iRet != XUI_OK ) return iRet;
	pData->iTrackColor = iTrack;
	pData->iFillColor = iFill;
	pData->iFillHoverColor = __xuiRangeSliderLighten(iFill, 12);
	pData->iFillActiveColor = __xuiRangeSliderLighten(iFill, -18);
	pData->iKnobColor = iKnob;
	pData->iFocusColor = iFocus;
	pData->iDisabledColor = iDisabled;
	return XUI_OK;
}

XUI_API int xuiRangeSliderGetColors(xui_widget pWidget, uint32_t* pTrack, uint32_t* pFill, uint32_t* pKnob, uint32_t* pFocus, uint32_t* pDisabled)
{
	xui_range_slider_data_t* pData = __xuiRangeSliderGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pTrack != NULL ) *pTrack = pData->iTrackColor;
	if ( pFill != NULL ) *pFill = pData->iFillColor;
	if ( pKnob != NULL ) *pKnob = pData->iKnobColor;
	if ( pFocus != NULL ) *pFocus = pData->iFocusColor;
	if ( pDisabled != NULL ) *pDisabled = pData->iDisabledColor;
	return XUI_OK;
}

XUI_API int xuiRangeSliderSetKnobBorderColor(xui_widget pWidget, uint32_t iColor)
{
	xui_range_slider_data_t* pData = __xuiRangeSliderGetData(pWidget);
	int iRet;
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pData->iKnobBorderColor == iColor ) return XUI_OK;
	iRet = xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	if ( iRet != XUI_OK ) return iRet;
	pData->iKnobBorderColor = iColor;
	return XUI_OK;
}

XUI_API uint32_t xuiRangeSliderGetKnobBorderColor(xui_widget pWidget)
{
	xui_range_slider_data_t* pData = __xuiRangeSliderGetData(pWidget);
	return (pData != NULL) ? pData->iKnobBorderColor : 0;
}

static xui_rect_t __xuiRangeSliderGetComputedRect(xui_widget pWidget, int iWhich)
{
	xui_range_slider_data_t* pData;
	xui_range_slider_data_t tResolved;
	xui_range_slider_geometry_t tGeometry;
	xui_rect_t tZero;

	memset(&tZero, 0, sizeof(tZero));
	pData = __xuiRangeSliderGetData(pWidget);
	if ( pData == NULL ) return tZero;
	__xuiRangeSliderResolve(pWidget, pData, &tResolved);
	__xuiRangeSliderComputeGeometry(pWidget, &tResolved, &tGeometry);
	if ( iWhich == 1 ) return xuiSliderCommonOutputRect(tGeometry.tTrack);
	if ( iWhich == 2 ) return xuiSliderCommonOutputRect(tGeometry.tFill);
	if ( iWhich == 3 ) return xuiSliderCommonOutputRect(tGeometry.tStartKnob);
	if ( iWhich == 4 ) return xuiSliderCommonOutputRect(tGeometry.tEndKnob);
	return tZero;
}

XUI_API xui_rect_t xuiRangeSliderGetTrackRect(xui_widget pWidget)
{
	return __xuiRangeSliderGetComputedRect(pWidget, 1);
}

XUI_API xui_rect_t xuiRangeSliderGetFillRect(xui_widget pWidget)
{
	return __xuiRangeSliderGetComputedRect(pWidget, 2);
}

XUI_API xui_rect_t xuiRangeSliderGetStartKnobRect(xui_widget pWidget)
{
	return __xuiRangeSliderGetComputedRect(pWidget, 3);
}

XUI_API xui_rect_t xuiRangeSliderGetEndKnobRect(xui_widget pWidget)
{
	return __xuiRangeSliderGetComputedRect(pWidget, 4);
}

XUI_API int xuiRangeSliderGetActiveThumb(xui_widget pWidget)
{
	xui_range_slider_data_t* pData = __xuiRangeSliderGetData(pWidget);
	return (pData != NULL) ? pData->iActiveThumb : XUI_RANGE_SLIDER_THUMB_NONE;
}

XUI_API uint32_t xuiRangeSliderGetState(xui_widget pWidget)
{
	xui_range_slider_data_t* pData;

	if ( !xuiInternalWidgetIsValid(pWidget) ) {
		return 0;
	}
	pData = __xuiRangeSliderGetData(pWidget);
	return __xuiRangeSliderBaseState(pWidget, pData);
}

XUI_API int xuiRangeSliderGetChangeCount(xui_widget pWidget)
{
	xui_range_slider_data_t* pData = __xuiRangeSliderGetData(pWidget);
	return (pData != NULL) ? pData->iChangeCount : 0;
}

#endif
