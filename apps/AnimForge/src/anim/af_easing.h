/*
 * af_easing.h - AnimForge easing functions library
 *
 * Standard easing functions for tween interpolation.
 * All functions take t in [0,1] and return eased value in [0,1]
 * (some may overshoot for elastic/back/bounce).
 */

#ifndef AF_EASING_H
#define AF_EASING_H

#ifdef __cplusplus
extern "C" {
#endif

/* Interpolation types (match xanim format) */
#define AF_INTERP_HOLD       0
#define AF_INTERP_LINEAR     1
#define AF_INTERP_EASE_IN    2
#define AF_INTERP_EASE_OUT   3
#define AF_INTERP_EASE_BOTH  4
#define AF_INTERP_BEZIER     5
#define AF_INTERP_ELASTIC    6
#define AF_INTERP_BOUNCE     7
#define AF_INTERP_BACK       8

/*
 * Evaluate easing by type.
 * fT: normalized time [0,1]
 * iType: AF_INTERP_*
 * fEaseIn/fEaseOut: strength for ease in/out (0~1)
 * pBezier: 4 floats (x1,y1,x2,y2) for BEZIER type, may be NULL
 */
float afEaseEval(float fT, int iType, float fEaseIn, float fEaseOut, const float* pBezier);

/* Individual easing functions */
float afEaseLinear(float t);
float afEaseQuadIn(float t);
float afEaseQuadOut(float t);
float afEaseQuadInOut(float t);
float afEaseCubicIn(float t);
float afEaseCubicOut(float t);
float afEaseCubicInOut(float t);
float afEaseSineIn(float t);
float afEaseSineOut(float t);
float afEaseSineInOut(float t);
float afEaseExpoIn(float t);
float afEaseExpoOut(float t);
float afEaseExpoInOut(float t);
float afEaseCircIn(float t);
float afEaseCircOut(float t);
float afEaseCircInOut(float t);
float afEaseBackIn(float t);
float afEaseBackOut(float t);
float afEaseBackInOut(float t);
float afEaseElasticIn(float t);
float afEaseElasticOut(float t);
float afEaseElasticInOut(float t);
float afEaseBounceIn(float t);
float afEaseBounceOut(float t);
float afEaseBounceInOut(float t);

/* Custom cubic-bezier(x1,y1,x2,y2) evaluation */
float afEaseCubicBezier(float t, float x1, float y1, float x2, float y2);

#ifdef __cplusplus
}
#endif

#endif /* AF_EASING_H */
