/*
 * af_timeline.h - Timeline model operations
 */

#ifndef AF_TIMELINE_H
#define AF_TIMELINE_H

#include "../core/af_doc.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Keyframe navigation */
int afTimelineFindKeyframeIndex(af_layer_t* pLayer, uint32_t iFrame);
af_keyframe_t* afTimelineGetKeyframeAt(af_layer_t* pLayer, uint32_t iFrame);
af_keyframe_t* afTimelineGetPrevKeyframe(af_layer_t* pLayer, uint32_t iFrame);
af_keyframe_t* afTimelineGetNextKeyframe(af_layer_t* pLayer, uint32_t iFrame);
af_keyframe_t* afTimelineGetOwningKeyframe(af_layer_t* pLayer, uint32_t iFrame);

/* Frame span queries */
int afTimelineGetSpanEnd(af_layer_t* pLayer, uint32_t iStartFrame);
int afTimelineIsFrameEmpty(af_layer_t* pLayer, uint32_t iFrame);

/* Keyframe operations */
int afTimelineInsertKeyframe(af_doc pDoc, uint32_t iLayerId, uint32_t iFrame);
int afTimelineInsertBlankKeyframe(af_doc pDoc, uint32_t iLayerId, uint32_t iFrame);
int afTimelineDeleteKeyframe(af_doc pDoc, uint32_t iLayerId, uint32_t iFrame);

/* Frame insertion/removal (shifts keyframes) */
int afTimelineInsertFrame(af_doc pDoc, uint32_t iLayerId, uint32_t iFrame);
int afTimelineRemoveFrame(af_doc pDoc, uint32_t iLayerId, uint32_t iFrame);

/* Tween span detection */
int afTimelineHasTweenSpan(af_doc pDoc, af_layer_t* pLayer, uint32_t iFrame);
int afTimelineGetTweenSpanRange(af_doc pDoc, af_layer_t* pLayer, uint32_t iFrame,
                                uint32_t* pStart, uint32_t* pEnd);

/* Layer operations */
int afTimelineReorderLayer(af_doc pDoc, uint32_t iLayerId, int iNewIndex);
int afTimelineDuplicateLayer(af_doc pDoc, uint32_t iLayerId, uint32_t* pNewId);

/* Frame labels */
int afTimelineSetFrameLabel(af_doc pDoc, uint32_t iLayerId, uint32_t iFrame, const char* sLabel);
int afTimelineFindFrameByLabel(af_doc pDoc, const char* sLabel, uint32_t* pFrame);

#ifdef __cplusplus
}
#endif

#endif /* AF_TIMELINE_H */
