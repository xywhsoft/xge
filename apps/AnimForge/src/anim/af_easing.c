/*
 * af_easing.c - AnimForge easing functions implementation
 */

#include "af_easing.h"
#include <math.h>
#include <stddef.h>

#ifndef AF_PI
#define AF_PI 3.14159265358979323846f
#endif

/* ------------------------------------------------------------------ */
/* Basic easing functions                                             */
/* ------------------------------------------------------------------ */

float afEaseLinear(float t) { return t; }

float afEaseQuadIn(float t) { return t * t; }
float afEaseQuadOut(float t) { return t * (2.0f - t); }
float afEaseQuadInOut(float t)
{
	if ( t < 0.5f ) return 2.0f * t * t;
	return -1.0f + (4.0f - 2.0f * t) * t;
}

float afEaseCubicIn(float t) { return t * t * t; }
float afEaseCubicOut(float t) { float u = t - 1.0f; return u * u * u + 1.0f; }
float afEaseCubicInOut(float t)
{
	if ( t < 0.5f ) return 4.0f * t * t * t;
	float u = 2.0f * t - 2.0f;
	return 0.5f * u * u * u + 1.0f;
}

float afEaseSineIn(float t) { return 1.0f - cosf(t * AF_PI * 0.5f); }
float afEaseSineOut(float t) { return sinf(t * AF_PI * 0.5f); }
float afEaseSineInOut(float t) { return 0.5f * (1.0f - cosf(AF_PI * t)); }

float afEaseExpoIn(float t)
{
	if ( t <= 0.0f ) return 0.0f;
	return powf(2.0f, 10.0f * (t - 1.0f));
}
float afEaseExpoOut(float t)
{
	if ( t >= 1.0f ) return 1.0f;
	return 1.0f - powf(2.0f, -10.0f * t);
}
float afEaseExpoInOut(float t)
{
	if ( t <= 0.0f ) return 0.0f;
	if ( t >= 1.0f ) return 1.0f;
	if ( t < 0.5f ) return 0.5f * powf(2.0f, 20.0f * t - 10.0f);
	return 1.0f - 0.5f * powf(2.0f, -20.0f * t + 10.0f);
}

float afEaseCircIn(float t) { return 1.0f - sqrtf(1.0f - t * t); }
float afEaseCircOut(float t) { float u = t - 1.0f; return sqrtf(1.0f - u * u); }
float afEaseCircInOut(float t)
{
	if ( t < 0.5f ) return 0.5f * (1.0f - sqrtf(1.0f - 4.0f * t * t));
	float u = 2.0f * t - 2.0f;
	return 0.5f * (sqrtf(1.0f - u * u) + 1.0f);
}

/* ------------------------------------------------------------------ */
/* Back easing (overshoot)                                            */
/* ------------------------------------------------------------------ */

#define AF_BACK_S 1.70158f

float afEaseBackIn(float t)
{
	return t * t * ((AF_BACK_S + 1.0f) * t - AF_BACK_S);
}
float afEaseBackOut(float t)
{
	float u = t - 1.0f;
	return u * u * ((AF_BACK_S + 1.0f) * u + AF_BACK_S) + 1.0f;
}
float afEaseBackInOut(float t)
{
	float s = AF_BACK_S * 1.525f;
	float u = t * 2.0f;
	if ( u < 1.0f ) return 0.5f * (u * u * ((s + 1.0f) * u - s));
	u -= 2.0f;
	return 0.5f * (u * u * ((s + 1.0f) * u + s) + 2.0f);
}

/* ------------------------------------------------------------------ */
/* Elastic easing                                                     */
/* ------------------------------------------------------------------ */

float afEaseElasticIn(float t)
{
	if ( t <= 0.0f ) return 0.0f;
	if ( t >= 1.0f ) return 1.0f;
	return -powf(2.0f, 10.0f * (t - 1.0f)) * sinf((t - 1.1f) * 5.0f * AF_PI);
}
float afEaseElasticOut(float t)
{
	if ( t <= 0.0f ) return 0.0f;
	if ( t >= 1.0f ) return 1.0f;
	return powf(2.0f, -10.0f * t) * sinf((t - 0.1f) * 5.0f * AF_PI) + 1.0f;
}
float afEaseElasticInOut(float t)
{
	if ( t <= 0.0f ) return 0.0f;
	if ( t >= 1.0f ) return 1.0f;
	float u = t * 2.0f;
	if ( u < 1.0f ) {
		return -0.5f * powf(2.0f, 10.0f * (u - 1.0f)) * sinf((u - 1.1f) * 5.0f * AF_PI);
	}
	return 0.5f * powf(2.0f, -10.0f * (u - 1.0f)) * sinf((u - 1.1f) * 5.0f * AF_PI) + 1.0f;
}

/* ------------------------------------------------------------------ */
/* Bounce easing                                                      */
/* ------------------------------------------------------------------ */

float afEaseBounceOut(float t)
{
	if ( t < 1.0f / 2.75f ) {
		return 7.5625f * t * t;
	} else if ( t < 2.0f / 2.75f ) {
		float u = t - 1.5f / 2.75f;
		return 7.5625f * u * u + 0.75f;
	} else if ( t < 2.5f / 2.75f ) {
		float u = t - 2.25f / 2.75f;
		return 7.5625f * u * u + 0.9375f;
	} else {
		float u = t - 2.625f / 2.75f;
		return 7.5625f * u * u + 0.984375f;
	}
}
float afEaseBounceIn(float t) { return 1.0f - afEaseBounceOut(1.0f - t); }
float afEaseBounceInOut(float t)
{
	if ( t < 0.5f ) return 0.5f * afEaseBounceIn(t * 2.0f);
	return 0.5f * afEaseBounceOut(t * 2.0f - 1.0f) + 0.5f;
}

/* ------------------------------------------------------------------ */
/* Custom cubic-bezier (CSS-style)                                    */
/* ------------------------------------------------------------------ */

float afEaseCubicBezier(float t, float x1, float y1, float x2, float y2)
{
	/* Newton-Raphson to solve for parameter u where bezierX(u) = t */
	float u = t;
	int i;
	for ( i = 0; i < 8; i++ ) {
		float bx = 3.0f * (1.0f - u) * (1.0f - u) * u * x1
		         + 3.0f * (1.0f - u) * u * u * x2
		         + u * u * u;
		float dx = 3.0f * (1.0f - u) * (1.0f - u) * x1
		         + 6.0f * (1.0f - u) * u * (x2 - x1)
		         + 3.0f * u * u * (1.0f - x2);
		float err = bx - t;
		if ( fabsf(err) < 1e-6f ) break;
		if ( fabsf(dx) < 1e-6f ) break;
		u -= err / dx;
	}
	/* Clamp */
	if ( u < 0.0f ) u = 0.0f;
	if ( u > 1.0f ) u = 1.0f;
	/* Evaluate Y at u */
	return 3.0f * (1.0f - u) * (1.0f - u) * u * y1
	     + 3.0f * (1.0f - u) * u * u * y2
	     + u * u * u;
}

/* ------------------------------------------------------------------ */
/* Unified evaluator                                                  */
/* ------------------------------------------------------------------ */

float afEaseEval(float fT, int iType, float fEaseIn, float fEaseOut, const float* pBezier)
{
	if ( fT <= 0.0f ) return 0.0f;
	if ( fT >= 1.0f ) return 1.0f;

	switch ( iType ) {
	case AF_INTERP_HOLD:      return 0.0f;
	case AF_INTERP_LINEAR:    return fT;
	case AF_INTERP_EASE_IN:   return afEaseCubicIn(fT);
	case AF_INTERP_EASE_OUT:  return afEaseCubicOut(fT);
	case AF_INTERP_EASE_BOTH: return afEaseCubicInOut(fT);
	case AF_INTERP_ELASTIC:   return afEaseElasticInOut(fT);
	case AF_INTERP_BOUNCE:    return afEaseBounceInOut(fT);
	case AF_INTERP_BACK:      return afEaseBackInOut(fT);
	case AF_INTERP_BEZIER:
		if ( pBezier != NULL ) {
			return afEaseCubicBezier(fT, pBezier[0], pBezier[1], pBezier[2], pBezier[3]);
		}
		return fT;
	default:
		break;
	}

	/* Weighted ease in/out blend */
	{
		float fLin = fT;
		float fIn = afEaseCubicIn(fT);
		float fOut = afEaseCubicOut(fT);
		float fBoth = afEaseCubicInOut(fT);
		(void)fIn; (void)fOut;
		(void)fEaseIn; (void)fEaseOut;
		return fBoth * 0.5f + fLin * 0.5f;
	}
}
