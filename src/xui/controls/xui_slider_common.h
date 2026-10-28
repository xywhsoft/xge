#ifndef XUI_SLIDER_COMMON_H
#define XUI_SLIDER_COMMON_H

#include <float.h>
#include <math.h>

typedef struct xui_slider_common_rect_t {
	float fX;
	float fY;
	float fW;
	float fH;
} xui_slider_common_rect_t;

static inline int xuiSliderCommonMetricValid(float fValue, float fMin)
{
	return isfinite(fValue) && fValue >= fMin &&
	       fValue <= XUI_LAYOUT_UNBOUNDED;
}

static inline float xuiSliderCommonClamp(float fValue, float fMin, float fMax)
{
	if ( fValue < fMin ) return fMin;
	if ( fValue > fMax ) return fMax;
	return fValue;
}

static inline float xuiSliderCommonAbs(float fValue)
{
	return (fValue < 0.0f) ? -fValue : fValue;
}

static inline float xuiSliderCommonMin(float fA, float fB)
{
	return (fA < fB) ? fA : fB;
}

static inline float xuiSliderCommonMax(float fA, float fB)
{
	return (fA > fB) ? fA : fB;
}

static inline void xuiSliderCommonNormalizeRange(float* pMin, float* pMax)
{
	float fSwap;

	if ( (*pMax) < (*pMin) ) {
		fSwap = *pMin;
		*pMin = *pMax;
		*pMax = fSwap;
	}
	if ( (*pMax) == (*pMin) ) {
		*pMax = (*pMin) + 1.0f;
		if ( !isfinite(*pMax) || (*pMax == *pMin) ) {
			*pMax = nextafterf(*pMin, INFINITY);
			if ( !isfinite(*pMax) ) {
				*pMax = *pMin;
				*pMin = nextafterf(*pMax, -INFINITY);
			}
		}
	}
}

static inline float xuiSliderCommonRate(float fMin, float fMax, float fValue)
{
	double fRate;

	if ( fMax == fMin ) return 0.0f;
	fRate = ((double)fValue - (double)fMin) /
		((double)fMax - (double)fMin);
	if ( fRate <= 0.0 ) return 0.0f;
	if ( fRate >= 1.0 ) return 1.0f;
	return (float)fRate;
}

static inline float xuiSliderCommonStep(float fMin, float fMax,
	float fConfigured, double fFraction)
{
	double fRange;
	double fStep;

	if ( fConfigured > 0.0f ) return fConfigured;
	fRange = (double)fMax - (double)fMin;
	fStep = (fRange > 0.0) ? (fRange * fFraction) : 1.0;
	if ( (fRange >= 1.0) && (fStep < 1.0) ) fStep = 1.0;
	if ( fStep > (double)FLT_MAX ) fStep = (double)FLT_MAX;
	return (fStep > 0.0) ? (float)fStep : 1.0f;
}

static inline float xuiSliderCommonValueWithDelta(float fValue, float fMin,
	float fMax, double fDelta)
{
	double fResult;

	if ( isnan(fDelta) || fDelta == 0.0 ) return fValue;
	fResult = (double)fValue + fDelta;
	if ( fResult <= (double)fMin ) return fMin;
	if ( fResult >= (double)fMax ) return fMax;
	return (float)fResult;
}

static inline xui_slider_common_rect_t xuiSliderCommonTrackRect(
	xui_rect_t tContent, float fTrackSize, float fKnobSize, int iOrientation)
{
	xui_slider_common_rect_t tTrack;
	float fCrossSize;
	float fInset;

	if ( (tContent.fW <= 0.0f) || (tContent.fH <= 0.0f) ) {
		return (xui_slider_common_rect_t){0};
	}
	tTrack = (xui_slider_common_rect_t){tContent.fX, tContent.fY,
		tContent.fW, tContent.fH};
	fCrossSize = (fTrackSize > 0.0f) ? fTrackSize : 4.0f;
	if ( iOrientation == XUI_ORIENTATION_HORIZONTAL ) {
		if ( fCrossSize > tContent.fH ) fCrossSize = tContent.fH;
		fInset = xuiSliderCommonMin(fKnobSize * 0.5f, tContent.fW * 0.5f);
		tTrack.fX = tContent.fX + fInset;
		tTrack.fW = tContent.fW - fInset * 2.0f;
		tTrack.fY = tContent.fY + (tContent.fH - fCrossSize) * 0.5f;
		tTrack.fH = fCrossSize;
	} else {
		if ( fCrossSize > tContent.fW ) fCrossSize = tContent.fW;
		fInset = xuiSliderCommonMin(fKnobSize * 0.5f, tContent.fH * 0.5f);
		tTrack.fY = tContent.fY + fInset;
		tTrack.fH = tContent.fH - fInset * 2.0f;
		tTrack.fX = tContent.fX + (tContent.fW - fCrossSize) * 0.5f;
		tTrack.fW = fCrossSize;
	}
	if ( tTrack.fW < 0.0f ) tTrack.fW = 0.0f;
	if ( tTrack.fH < 0.0f ) tTrack.fH = 0.0f;
	return tTrack;
}

static inline xui_slider_common_rect_t xuiSliderCommonKnobRect(
	xui_rect_t tContent, xui_slider_common_rect_t tTrack, float fKnobSize,
	int iOrientation, float fRate)
{
	xui_slider_common_rect_t tRect;
	float fSize;
	float fCenter;

	tRect = (xui_slider_common_rect_t){0};
	if ( (tContent.fW <= 0.0f) || (tContent.fH <= 0.0f) ) return tRect;
	fRate = xuiSliderCommonClamp(fRate, 0.0f, 1.0f);
	fSize = (fKnobSize > 0.0f) ? fKnobSize : 14.0f;
	if ( iOrientation == XUI_ORIENTATION_HORIZONTAL ) {
		if ( fSize > tContent.fH ) fSize = tContent.fH;
		if ( fSize > tContent.fW ) fSize = tContent.fW;
		fCenter = tTrack.fX + tTrack.fW * fRate;
		tRect.fX = fCenter - fSize * 0.5f;
		tRect.fY = tContent.fY + (tContent.fH - fSize) * 0.5f;
		tRect.fW = fSize;
		tRect.fH = fSize;
		if ( tRect.fX < tContent.fX ) tRect.fX = tContent.fX;
		if ( tRect.fX + tRect.fW > tContent.fX + tContent.fW )
			tRect.fX = tContent.fX + tContent.fW - tRect.fW;
	} else {
		if ( fSize > tContent.fW ) fSize = tContent.fW;
		if ( fSize > tContent.fH ) fSize = tContent.fH;
		fCenter = tTrack.fY + tTrack.fH * (1.0f - fRate);
		tRect.fX = tContent.fX + (tContent.fW - fSize) * 0.5f;
		tRect.fY = fCenter - fSize * 0.5f;
		tRect.fW = fSize;
		tRect.fH = fSize;
		if ( tRect.fY < tContent.fY ) tRect.fY = tContent.fY;
		if ( tRect.fY + tRect.fH > tContent.fY + tContent.fH )
			tRect.fY = tContent.fY + tContent.fH - tRect.fH;
	}
	return tRect;
}

static inline int xuiSliderCommonRectContains(xui_slider_common_rect_t tRect,
	float fX, float fY)
{
	return (tRect.fW > 0.0f) && (tRect.fH > 0.0f) &&
	       (fX >= tRect.fX) && (fY >= tRect.fY) &&
	       (fX < tRect.fX + tRect.fW) && (fY < tRect.fY + tRect.fH);
}

static inline xui_rect_t xuiSliderCommonOutputRect(xui_slider_common_rect_t tRect)
{
	return xuiInternalRectFromFloatNearest(tRect.fX, tRect.fY,
		tRect.fW, tRect.fH);
}

static inline float xuiSliderCommonValueFromPoint(float fCurrent, float fMin,
	float fMax, int iOrientation, xui_slider_common_rect_t tTrack,
	float fLocalX, float fLocalY)
{
	float fRate;
	double fValue;

	if ( iOrientation == XUI_ORIENTATION_HORIZONTAL ) {
		if ( tTrack.fW <= 0.0f ) return fCurrent;
		fRate = (fLocalX - tTrack.fX) / tTrack.fW;
	} else {
		if ( tTrack.fH <= 0.0f ) return fCurrent;
		fRate = 1.0f - ((fLocalY - tTrack.fY) / tTrack.fH);
	}
	fRate = xuiSliderCommonClamp(fRate, 0.0f, 1.0f);
	fValue = (double)fMin + ((double)fMax - (double)fMin) * (double)fRate;
	if ( fValue <= (double)fMin ) return fMin;
	if ( fValue >= (double)fMax ) return fMax;
	return (float)fValue;
}

#endif
