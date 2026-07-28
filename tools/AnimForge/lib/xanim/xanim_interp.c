/*
 * xanim_interp.c - Interpolation and easing functions
 *
 * Runtime easing library for .xanim playback.
 * Supports all interpolation types defined in the format.
 */

#include "xanim.h"
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/* ------------------------------------------------------------------ */
/* Basic easing functions                                             */
/* ------------------------------------------------------------------ */

static float __easeQuadIn(float t) { return t * t; }
static float __easeQuadOut(float t) { return t * (2.0f - t); }
static float __easeQuadInOut(float t) {
	return (t < 0.5f) ? 2.0f * t * t : -1.0f + (4.0f - 2.0f * t) * t;
}

static float __easeCubicIn(float t) { return t * t * t; }
static float __easeCubicOut(float t) { float u = t - 1.0f; return u * u * u + 1.0f; }
static float __easeCubicInOut(float t) {
	return (t < 0.5f) ? 4.0f * t * t * t : (t - 1.0f) * (2.0f * t - 2.0f) * (2.0f * t - 2.0f) + 1.0f;
}

static float __easeSineIn(float t) { return 1.0f - cosf(t * (float)M_PI * 0.5f); }
static float __easeSineOut(float t) { return sinf(t * (float)M_PI * 0.5f); }
static float __easeSineInOut(float t) { return 0.5f * (1.0f - cosf((float)M_PI * t)); }

static float __easeExpoIn(float t) {
	return (t <= 0.0f) ? 0.0f : powf(2.0f, 10.0f * (t - 1.0f));
}
static float __easeExpoOut(float t) {
	return (t >= 1.0f) ? 1.0f : 1.0f - powf(2.0f, -10.0f * t);
}
static float __easeExpoInOut(float t) {
	if ( t <= 0.0f ) return 0.0f;
	if ( t >= 1.0f ) return 1.0f;
	if ( t < 0.5f ) return 0.5f * powf(2.0f, 20.0f * t - 10.0f);
	return 1.0f - 0.5f * powf(2.0f, -20.0f * t + 10.0f);
}

static float __easeCircIn(float t) { return 1.0f - sqrtf(1.0f - t * t); }
static float __easeCircOut(float t) { float u = t - 1.0f; return sqrtf(1.0f - u * u); }
static float __easeCircInOut(float t) {
	if ( t < 0.5f ) return 0.5f * (1.0f - sqrtf(1.0f - 4.0f * t * t));
	float u = 2.0f * t - 2.0f;
	return 0.5f * (sqrtf(1.0f - u * u) + 1.0f);
}

static float __easeBackIn(float t) {
	float s = 1.70158f;
	return t * t * ((s + 1.0f) * t - s);
}
static float __easeBackOut(float t) {
	float s = 1.70158f;
	float u = t - 1.0f;
	return u * u * ((s + 1.0f) * u + s) + 1.0f;
}
static float __easeBackInOut(float t) {
	float s = 1.70158f * 1.525f;
	float u = t * 2.0f;
	if ( u < 1.0f ) return 0.5f * (u * u * ((s + 1.0f) * u - s));
	u -= 2.0f;
	return 0.5f * (u * u * ((s + 1.0f) * u + s) + 2.0f);
}

static float __easeElasticOut(float t) {
	if ( t <= 0.0f ) return 0.0f;
	if ( t >= 1.0f ) return 1.0f;
	return powf(2.0f, -10.0f * t) * sinf((t - 0.075f) * (2.0f * (float)M_PI) / 0.3f) + 1.0f;
}
static float __easeElasticIn(float t) {
	return 1.0f - __easeElasticOut(1.0f - t);
}

static float __easeBounceOut(float t) {
	if ( t < 1.0f / 2.75f ) return 7.5625f * t * t;
	if ( t < 2.0f / 2.75f ) { float u = t - 1.5f / 2.75f; return 7.5625f * u * u + 0.75f; }
	if ( t < 2.5f / 2.75f ) { float u = t - 2.25f / 2.75f; return 7.5625f * u * u + 0.9375f; }
	float u = t - 2.625f / 2.75f;
	return 7.5625f * u * u + 0.984375f;
}
static float __easeBounceIn(float t) { return 1.0f - __easeBounceOut(1.0f - t); }

/* ------------------------------------------------------------------ */
/* Cubic bezier solver                                                */
/* ------------------------------------------------------------------ */

static float __cubicBezier(float x1, float y1, float x2, float y2, float t)
{
	/* Newton-Raphson iteration to find t for given x */
	float cx = 3.0f * x1;
	float bx = 3.0f * (x2 - x1) - cx;
	float ax = 1.0f - cx - bx;
	float cy = 3.0f * y1;
	float by = 3.0f * (y2 - y1) - cy;
	float ay = 1.0f - cy - by;
	float guess = t;
	int i;

	for ( i = 0; i < 8; i++ ) {
		float xGuess = ((ax * guess + bx) * guess + cx) * guess;
		float dx = xGuess - t;
		float dxdt = (3.0f * ax * guess + 2.0f * bx) * guess + cx;
		if ( fabsf(dx) < 1e-6f ) break;
		if ( fabsf(dxdt) < 1e-6f ) break;
		guess -= dx / dxdt;
	}

	return ((ay * guess + by) * guess + cy) * guess;
}

/* ------------------------------------------------------------------ */
/* Unified interpolation evaluator                                    */
/* ------------------------------------------------------------------ */

float xanimInterpEval(float fT, uint32_t iInterpType, float fEaseIn, float fEaseOut,
                      const float* pBezier)
{
	float t = fT;

	if ( t <= 0.0f ) return 0.0f;
	if ( t >= 1.0f ) return 1.0f;

	switch ( iInterpType ) {
	case XANIM_INTERP_HOLD:
		return 0.0f;

	case XANIM_INTERP_LINEAR:
		return t;

	case XANIM_INTERP_EASE_IN:
		return __easeCubicIn(t) * fEaseIn + t * (1.0f - fEaseIn);

	case XANIM_INTERP_EASE_OUT:
		return __easeCubicOut(t) * fEaseOut + t * (1.0f - fEaseOut);

	case XANIM_INTERP_EASE_BOTH:
		return __easeCubicInOut(t);

	case XANIM_INTERP_BEZIER:
		if ( pBezier )
			return __cubicBezier(pBezier[0], pBezier[1], pBezier[2], pBezier[3], t);
		return t;

	case XANIM_INTERP_ELASTIC:
		return __easeElasticOut(t);

	case XANIM_INTERP_BOUNCE:
		return __easeBounceOut(t);

	case XANIM_INTERP_BACK:
		return __easeBackOut(t);

	default:
		return t;
	}
}

/* ------------------------------------------------------------------ */
/* Channel value interpolation                                        */
/* ------------------------------------------------------------------ */

float xanimInterpLerp(float fA, float fB, float fT)
{
	return fA + (fB - fA) * fT;
}

void xanimInterpEvalChannels(const xanim_curve_key_t* pK0, const xanim_curve_key_t* pK1,
                             float fT, float* pOut, uint32_t iChannelMask)
{
	float easedT = xanimInterpEval(fT, pK0->interpType, pK0->easeIn, pK0->easeOut,
	                               pK0->bezier);
	int ch;

	for ( ch = 0; ch < XANIM_CHANNEL_COUNT; ch++ ) {
		if ( iChannelMask & (1u << ch) ) {
			pOut[ch] = xanimInterpLerp(pK0->values[ch], pK1->values[ch], easedT);
		} else {
			pOut[ch] = pK0->values[ch];
		}
	}
}
