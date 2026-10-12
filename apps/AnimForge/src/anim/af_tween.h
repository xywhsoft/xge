/*
 * af_tween.h - AnimForge tween interpolation engine
 *
 * Evaluates animation curves at arbitrary frame positions,
 * producing interpolated element transforms.
 */

#ifndef AF_TWEEN_H
#define AF_TWEEN_H

#include "../core/af_doc.h"
#include "af_easing.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Channel indices */
#define AF_CH_X         0
#define AF_CH_Y         1
#define AF_CH_SCALE_X   2
#define AF_CH_SCALE_Y   3
#define AF_CH_ROTATION  4
#define AF_CH_OPACITY   5
#define AF_CH_SKEW_X    6
#define AF_CH_SKEW_Y    7
#define AF_CH_PIVOT_X   8
#define AF_CH_PIVOT_Y   9
#define AF_CH_COLOR_R  10
#define AF_CH_COLOR_G  11
#define AF_CH_COLOR_B  12
#define AF_CH_COLOR_A  13

/*
 * Evaluate a curve at a given frame, writing interpolated channel
 * values into pOut (must hold AF_CHANNEL_COUNT floats).
 * Returns 0 on success, -1 if curve not found or empty.
 */
int afTweenEvalCurve(const af_curve_t* pCurve, float fFrame, float* pOut);

/*
 * Evaluate element transform at a given frame by finding the
 * surrounding keyframes on a layer and interpolating.
 * Writes result into pResult element.
 * Returns 0 on success.
 */
int afTweenEvalElement(af_doc pDoc, uint32_t iLayerId, uint32_t iFrame,
                       af_element_t* pResult);

/*
 * Find the keyframe that applies at a given frame (the keyframe
 * at or just before iFrame). Returns NULL if none.
 */
af_keyframe_t* afTweenFindActiveKeyframe(af_layer_t* pLayer, uint32_t iFrame);

/*
 * Create a motion tween between two keyframes for an element.
 * Generates a curve with keys at both frames.
 * Returns curve ID or 0 on failure.
 */
uint32_t afTweenCreateMotionTween(af_doc pDoc, uint32_t iLayerId,
                                   uint32_t iFrameA, uint32_t iFrameB,
                                   uint32_t iInstanceId);

/*
 * Remove tween from an element span.
 */
int afTweenRemoveMotionTween(af_doc pDoc, uint32_t iInstanceId);

#ifdef __cplusplus
}
#endif

#endif /* AF_TWEEN_H */
