/*
 * af_timeline.c - Timeline model operations
 *
 * Higher-level timeline operations: frame span queries, keyframe
 * navigation, layer reordering, tween span detection, and
 * frame clipboard operations.
 */

#include "af_timeline.h"
#include "../core/af_doc.h"
#include <string.h>

/* ------------------------------------------------------------------ */
/* Keyframe navigation                                                */
/* ------------------------------------------------------------------ */

int afTimelineFindKeyframeIndex(af_layer_t* pLayer, uint32_t iFrame)
{
	int i;
	for ( i = 0; i < pLayer->iKeyframeCount; i++ ) {
		if ( pLayer->arrKeyframes[i].iFrame == iFrame )
			return i;
	}
	return -1;
}

af_keyframe_t* afTimelineGetKeyframeAt(af_layer_t* pLayer, uint32_t iFrame)
{
	int idx = afTimelineFindKeyframeIndex(pLayer, iFrame);
	if ( idx < 0 ) return NULL;
	return &pLayer->arrKeyframes[idx];
}

af_keyframe_t* afTimelineGetPrevKeyframe(af_layer_t* pLayer, uint32_t iFrame)
{
	int i, bestIdx = -1;
	uint32_t bestFrame = 0;

	for ( i = 0; i < pLayer->iKeyframeCount; i++ ) {
		uint32_t f = pLayer->arrKeyframes[i].iFrame;
		if ( f < iFrame && (bestIdx < 0 || f > bestFrame) ) {
			bestIdx = i;
			bestFrame = f;
		}
	}
	if ( bestIdx < 0 ) return NULL;
	return &pLayer->arrKeyframes[bestIdx];
}

af_keyframe_t* afTimelineGetNextKeyframe(af_layer_t* pLayer, uint32_t iFrame)
{
	int i, bestIdx = -1;
	uint32_t bestFrame = 0xFFFFFFFFu;

	for ( i = 0; i < pLayer->iKeyframeCount; i++ ) {
		uint32_t f = pLayer->arrKeyframes[i].iFrame;
		if ( f > iFrame && f < bestFrame ) {
			bestIdx = i;
			bestFrame = f;
		}
	}
	if ( bestIdx < 0 ) return NULL;
	return &pLayer->arrKeyframes[bestIdx];
}

/* Get the keyframe that "owns" a given frame (last keyframe <= frame) */
af_keyframe_t* afTimelineGetOwningKeyframe(af_layer_t* pLayer, uint32_t iFrame)
{
	int i, bestIdx = -1;
	uint32_t bestFrame = 0;

	for ( i = 0; i < pLayer->iKeyframeCount; i++ ) {
		uint32_t f = pLayer->arrKeyframes[i].iFrame;
		if ( f <= iFrame && (bestIdx < 0 || f > bestFrame) ) {
			bestIdx = i;
			bestFrame = f;
		}
	}
	if ( bestIdx < 0 ) return NULL;
	return &pLayer->arrKeyframes[bestIdx];
}

/* ------------------------------------------------------------------ */
/* Frame span queries                                                 */
/* ------------------------------------------------------------------ */

int afTimelineGetSpanEnd(af_layer_t* pLayer, uint32_t iStartFrame)
{
	af_keyframe_t* pNext = afTimelineGetNextKeyframe(pLayer, iStartFrame);
	if ( pNext ) return (int)pNext->iFrame - 1;
	return -1; /* extends to end */
}

int afTimelineIsFrameEmpty(af_layer_t* pLayer, uint32_t iFrame)
{
	af_keyframe_t* pKf = afTimelineGetOwningKeyframe(pLayer, iFrame);
	if ( pKf == NULL ) return 1;
	if ( pKf->iFlags & AF_KF_BLANK ) return 1;
	if ( pKf->iElementCount == 0 ) return 1;
	return 0;
}

/* ------------------------------------------------------------------ */
/* Keyframe insertion/deletion with frame shifting                    */
/* ------------------------------------------------------------------ */

int afTimelineInsertKeyframe(af_doc pDoc, uint32_t iLayerId, uint32_t iFrame)
{
	af_layer_t* pLayer = afDocFindLayer(pDoc, iLayerId);
	af_keyframe_t* pPrev;
	af_keyframe_t* pNew;
	int i;

	if ( pLayer == NULL ) return -1;
	if ( pLayer->iKeyframeCount >= AF_DOC_MAX_KEYFRAMES / AF_DOC_MAX_LAYERS )
		return -2;

	/* Check if keyframe already exists at this frame */
	if ( afTimelineFindKeyframeIndex(pLayer, iFrame) >= 0 )
		return 0; /* already exists */

	/* Find insertion position (sorted by frame) */
	pPrev = afTimelineGetOwningKeyframe(pLayer, iFrame);

	/* Shift array to make room */
	{
		int insertIdx = pLayer->iKeyframeCount;
		for ( i = 0; i < pLayer->iKeyframeCount; i++ ) {
			if ( pLayer->arrKeyframes[i].iFrame > iFrame ) {
				insertIdx = i;
				break;
			}
		}
		for ( i = pLayer->iKeyframeCount; i > insertIdx; i-- ) {
			pLayer->arrKeyframes[i] = pLayer->arrKeyframes[i - 1];
		}
		pNew = &pLayer->arrKeyframes[insertIdx];
	}

	memset(pNew, 0, sizeof(*pNew));
	pNew->iFrame = iFrame;
	pNew->iFlags = 0;
	pNew->sLabel[0] = '\0';

	/* Copy elements from previous keyframe (frame-by-frame behavior) */
	if ( pPrev != NULL && !(pPrev->iFlags & AF_KF_BLANK) ) {
		pNew->iElementCount = pPrev->iElementCount;
		memcpy(pNew->arrElements, pPrev->arrElements,
		       sizeof(af_element_t) * pPrev->iElementCount);
	} else {
		pNew->iElementCount = 0;
	}

	pLayer->iKeyframeCount++;
	pDoc->bModified = 1;
	return 0;
}

int afTimelineInsertBlankKeyframe(af_doc pDoc, uint32_t iLayerId, uint32_t iFrame)
{
	int ret = afTimelineInsertKeyframe(pDoc, iLayerId, iFrame);
	if ( ret != 0 ) return ret;

	{
		af_layer_t* pLayer = afDocFindLayer(pDoc, iLayerId);
		af_keyframe_t* pKf = afTimelineGetKeyframeAt(pLayer, iFrame);
		if ( pKf ) {
			pKf->iFlags |= AF_KF_BLANK;
			pKf->iElementCount = 0;
		}
	}
	return 0;
}

int afTimelineDeleteKeyframe(af_doc pDoc, uint32_t iLayerId, uint32_t iFrame)
{
	af_layer_t* pLayer = afDocFindLayer(pDoc, iLayerId);
	int idx, i;

	if ( pLayer == NULL ) return -1;

	idx = afTimelineFindKeyframeIndex(pLayer, iFrame);
	if ( idx < 0 ) return -2;

	for ( i = idx; i < pLayer->iKeyframeCount - 1; i++ ) {
		pLayer->arrKeyframes[i] = pLayer->arrKeyframes[i + 1];
	}
	pLayer->iKeyframeCount--;
	pDoc->bModified = 1;
	return 0;
}

/* ------------------------------------------------------------------ */
/* Frame insertion (shifts all subsequent keyframes)                  */
/* ------------------------------------------------------------------ */

int afTimelineInsertFrame(af_doc pDoc, uint32_t iLayerId, uint32_t iFrame)
{
	af_layer_t* pLayer = afDocFindLayer(pDoc, iLayerId);
	int i;

	if ( pLayer == NULL ) return -1;

	/* Shift all keyframes at or after iFrame by +1 */
	for ( i = 0; i < pLayer->iKeyframeCount; i++ ) {
		if ( pLayer->arrKeyframes[i].iFrame >= iFrame )
			pLayer->arrKeyframes[i].iFrame++;
	}

	/* Extend total frame count if needed */
	if ( iFrame >= pDoc->iFrameCount )
		pDoc->iFrameCount = iFrame + 1;

	pDoc->bModified = 1;
	return 0;
}

int afTimelineRemoveFrame(af_doc pDoc, uint32_t iLayerId, uint32_t iFrame)
{
	af_layer_t* pLayer = afDocFindLayer(pDoc, iLayerId);
	int i;

	if ( pLayer == NULL ) return -1;

	/* If there's a keyframe at this frame, delete it */
	{
		int idx = afTimelineFindKeyframeIndex(pLayer, iFrame);
		if ( idx >= 0 ) {
			for ( i = idx; i < pLayer->iKeyframeCount - 1; i++ )
				pLayer->arrKeyframes[i] = pLayer->arrKeyframes[i + 1];
			pLayer->iKeyframeCount--;
		}
	}

	/* Shift all keyframes after iFrame by -1 */
	for ( i = 0; i < pLayer->iKeyframeCount; i++ ) {
		if ( pLayer->arrKeyframes[i].iFrame > iFrame )
			pLayer->arrKeyframes[i].iFrame--;
	}

	pDoc->bModified = 1;
	return 0;
}

/* ------------------------------------------------------------------ */
/* Tween span detection                                               */
/* ------------------------------------------------------------------ */

int afTimelineHasTweenSpan(af_doc pDoc, af_layer_t* pLayer, uint32_t iFrame)
{
	af_keyframe_t* pKf = afTimelineGetOwningKeyframe(pLayer, iFrame);
	if ( pKf == NULL ) return 0;

	/* Check if any element on this keyframe has a tween */
	{
		int i;
		for ( i = 0; i < pKf->iElementCount; i++ ) {
			if ( pKf->arrElements[i].iTweenId != 0xFFFFFFFFu )
				return 1;
		}
	}
	return 0;
}

int afTimelineGetTweenSpanRange(af_doc pDoc, af_layer_t* pLayer, uint32_t iFrame,
                                uint32_t* pStart, uint32_t* pEnd)
{
	af_keyframe_t* pKf = afTimelineGetOwningKeyframe(pLayer, iFrame);
	af_keyframe_t* pNext;

	if ( pKf == NULL ) return -1;
	if ( !afTimelineHasTweenSpan(pDoc, pLayer, iFrame) ) return -1;

	if ( pStart ) *pStart = pKf->iFrame;

	pNext = afTimelineGetNextKeyframe(pLayer, pKf->iFrame);
	if ( pEnd ) *pEnd = pNext ? pNext->iFrame : pDoc->iFrameCount;

	return 0;
}

/* ------------------------------------------------------------------ */
/* Layer operations                                                   */
/* ------------------------------------------------------------------ */

int afTimelineReorderLayer(af_doc pDoc, uint32_t iLayerId, int iNewIndex)
{
	af_timeline_t* pTl = &pDoc->tTimeline;
	int srcIdx = -1, i;
	af_layer_t tmp;

	/* Find source index */
	for ( i = 0; i < pTl->iLayerCount; i++ ) {
		if ( pTl->arrLayers[i].iId == iLayerId ) {
			srcIdx = i;
			break;
		}
	}
	if ( srcIdx < 0 ) return -1;
	if ( iNewIndex < 0 || iNewIndex >= pTl->iLayerCount ) return -2;
	if ( srcIdx == iNewIndex ) return 0;

	tmp = pTl->arrLayers[srcIdx];
	if ( srcIdx < iNewIndex ) {
		for ( i = srcIdx; i < iNewIndex; i++ )
			pTl->arrLayers[i] = pTl->arrLayers[i + 1];
	} else {
		for ( i = srcIdx; i > iNewIndex; i-- )
			pTl->arrLayers[i] = pTl->arrLayers[i - 1];
	}
	pTl->arrLayers[iNewIndex] = tmp;
	pDoc->bModified = 1;
	return 0;
}

int afTimelineDuplicateLayer(af_doc pDoc, uint32_t iLayerId, uint32_t* pNewId)
{
	af_timeline_t* pTl = &pDoc->tTimeline;
	int srcIdx = -1, i;
	af_layer_t* pSrc;
	af_layer_t* pDst;

	for ( i = 0; i < pTl->iLayerCount; i++ ) {
		if ( pTl->arrLayers[i].iId == iLayerId ) {
			srcIdx = i;
			break;
		}
	}
	if ( srcIdx < 0 ) return -1;
	if ( pTl->iLayerCount >= AF_DOC_MAX_LAYERS ) return -2;

	pSrc = &pTl->arrLayers[srcIdx];

	/* Shift layers above up */
	for ( i = pTl->iLayerCount; i > srcIdx + 1; i-- )
		pTl->arrLayers[i] = pTl->arrLayers[i - 1];

	pDst = &pTl->arrLayers[srcIdx + 1];
	*pDst = *pSrc;
	pDst->iId = pDoc->iNextLayerId++;

	/* Append " copy" to name */
	{
		int len = (int)strlen(pDst->sName);
		if ( len + 6 < AF_DOC_MAX_NAME ) {
			strcat(pDst->sName, " copy");
		}
	}

	pTl->iLayerCount++;
	if ( pNewId ) *pNewId = pDst->iId;
	pDoc->bModified = 1;
	return 0;
}

/* ------------------------------------------------------------------ */
/* Frame label management                                             */
/* ------------------------------------------------------------------ */

int afTimelineSetFrameLabel(af_doc pDoc, uint32_t iLayerId, uint32_t iFrame, const char* sLabel)
{
	af_layer_t* pLayer = afDocFindLayer(pDoc, iLayerId);
	af_keyframe_t* pKf;

	if ( pLayer == NULL ) return -1;

	pKf = afTimelineGetKeyframeAt(pLayer, iFrame);
	if ( pKf == NULL ) {
		/* Create keyframe first */
		int ret = afTimelineInsertKeyframe(pDoc, iLayerId, iFrame);
		if ( ret != 0 ) return ret;
		pKf = afTimelineGetKeyframeAt(pLayer, iFrame);
		if ( pKf == NULL ) return -1;
	}

	if ( sLabel && sLabel[0] ) {
		strncpy(pKf->sLabel, sLabel, AF_DOC_MAX_NAME - 1);
		pKf->sLabel[AF_DOC_MAX_NAME - 1] = '\0';
	} else {
		pKf->sLabel[0] = '\0';
	}

	pDoc->bModified = 1;
	return 0;
}

int afTimelineFindFrameByLabel(af_doc pDoc, const char* sLabel, uint32_t* pFrame)
{
	int li, ki;
	for ( li = 0; li < pDoc->tTimeline.iLayerCount; li++ ) {
		af_layer_t* pLayer = &pDoc->tTimeline.arrLayers[li];
		for ( ki = 0; ki < pLayer->iKeyframeCount; ki++ ) {
			if ( strcmp(pLayer->arrKeyframes[ki].sLabel, sLabel) == 0 ) {
				if ( pFrame ) *pFrame = pLayer->arrKeyframes[ki].iFrame;
				return 0;
			}
		}
	}
	return -1; /* not found */
}
