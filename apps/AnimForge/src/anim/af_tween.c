/*
 * af_tween.c - AnimForge tween interpolation engine implementation
 */

#include "af_tween.h"
#include <string.h>
#include <math.h>

/* ------------------------------------------------------------------ */
/* Helper: extract element channel values                             */
/* ------------------------------------------------------------------ */

static void __elemToChannels(const af_element_t* pElem, float* pOut)
{
	pOut[AF_CH_X]        = pElem->fTx;
	pOut[AF_CH_Y]        = pElem->fTy;
	pOut[AF_CH_SCALE_X]  = pElem->fScaleX;
	pOut[AF_CH_SCALE_Y]  = pElem->fScaleY;
	pOut[AF_CH_ROTATION] = pElem->fRotation;
	pOut[AF_CH_OPACITY]  = pElem->fOpacity;
	pOut[AF_CH_SKEW_X]   = pElem->fSkewX;
	pOut[AF_CH_SKEW_Y]   = pElem->fSkewY;
	pOut[AF_CH_PIVOT_X]  = pElem->fPivotX;
	pOut[AF_CH_PIVOT_Y]  = pElem->fPivotY;
	/* Color transform packed as RGBA bytes */
	pOut[AF_CH_COLOR_R]  = (float)((pElem->iColorTransform >> 24) & 0xFF);
	pOut[AF_CH_COLOR_G]  = (float)((pElem->iColorTransform >> 16) & 0xFF);
	pOut[AF_CH_COLOR_B]  = (float)((pElem->iColorTransform >> 8) & 0xFF);
	pOut[AF_CH_COLOR_A]  = (float)(pElem->iColorTransform & 0xFF);
}

static void __channelsToElem(const float* pIn, af_element_t* pElem)
{
	uint32_t r, g, b, a;
	pElem->fTx       = pIn[AF_CH_X];
	pElem->fTy       = pIn[AF_CH_Y];
	pElem->fScaleX   = pIn[AF_CH_SCALE_X];
	pElem->fScaleY   = pIn[AF_CH_SCALE_Y];
	pElem->fRotation = pIn[AF_CH_ROTATION];
	pElem->fOpacity  = pIn[AF_CH_OPACITY];
	pElem->fSkewX    = pIn[AF_CH_SKEW_X];
	pElem->fSkewY    = pIn[AF_CH_SKEW_Y];
	pElem->fPivotX   = pIn[AF_CH_PIVOT_X];
	pElem->fPivotY   = pIn[AF_CH_PIVOT_Y];
	r = (uint32_t)(pIn[AF_CH_COLOR_R] + 0.5f) & 0xFF;
	g = (uint32_t)(pIn[AF_CH_COLOR_G] + 0.5f) & 0xFF;
	b = (uint32_t)(pIn[AF_CH_COLOR_B] + 0.5f) & 0xFF;
	a = (uint32_t)(pIn[AF_CH_COLOR_A] + 0.5f) & 0xFF;
	pElem->iColorTransform = (r << 24) | (g << 16) | (b << 8) | a;
}

/* ------------------------------------------------------------------ */
/* Curve evaluation                                                   */
/* ------------------------------------------------------------------ */

int afTweenEvalCurve(const af_curve_t* pCurve, float fFrame, float* pOut)
{
	int i;
	const af_curve_key_t* pPrev;
	const af_curve_key_t* pNext;
	float fT, fEased;

	if ( pCurve == NULL || pOut == NULL ) return -1;
	if ( pCurve->iKeyCount == 0 ) return -1;

	/* Before first key: hold first value */
	if ( fFrame <= (float)pCurve->arrKeys[0].iFrame ) {
		memcpy(pOut, pCurve->arrKeys[0].arrValues, sizeof(float) * AF_CHANNEL_COUNT);
		return 0;
	}
	/* After last key: hold last value */
	if ( fFrame >= (float)pCurve->arrKeys[pCurve->iKeyCount - 1].iFrame ) {
		memcpy(pOut, pCurve->arrKeys[pCurve->iKeyCount - 1].arrValues, sizeof(float) * AF_CHANNEL_COUNT);
		return 0;
	}

	/* Find surrounding keys */
	pPrev = &pCurve->arrKeys[0];
	pNext = &pCurve->arrKeys[1];
	for ( i = 1; i < pCurve->iKeyCount; i++ ) {
		if ( (float)pCurve->arrKeys[i].iFrame >= fFrame ) {
			pPrev = &pCurve->arrKeys[i - 1];
			pNext = &pCurve->arrKeys[i];
			break;
		}
	}

	/* Compute normalized time */
	{
		float fSpan = (float)(pNext->iFrame - pPrev->iFrame);
		if ( fSpan <= 0.0f ) {
			memcpy(pOut, pNext->arrValues, sizeof(float) * AF_CHANNEL_COUNT);
			return 0;
		}
		fT = (fFrame - (float)pPrev->iFrame) / fSpan;
	}

	/* Apply easing */
	fEased = afEaseEval(fT, (int)pPrev->iInterpType, pPrev->fEaseIn, pPrev->fEaseOut, pPrev->arrBezier);

	/* Interpolate each active channel */
	for ( i = 0; i < AF_CHANNEL_COUNT; i++ ) {
		if ( (pCurve->iChannelMask & (1u << i)) != 0 ) {
			pOut[i] = pPrev->arrValues[i] + (pNext->arrValues[i] - pPrev->arrValues[i]) * fEased;
		} else {
			pOut[i] = pPrev->arrValues[i];
		}
	}
	return 0;
}

/* ------------------------------------------------------------------ */
/* Active keyframe lookup                                             */
/* ------------------------------------------------------------------ */

af_keyframe_t* afTweenFindActiveKeyframe(af_layer_t* pLayer, uint32_t iFrame)
{
	int i;
	af_keyframe_t* pBest = NULL;

	if ( pLayer == NULL ) return NULL;

	for ( i = 0; i < pLayer->iKeyframeCount; i++ ) {
		af_keyframe_t* pKf = &pLayer->arrKeyframes[i];
		if ( pKf->iFrame <= iFrame ) {
			if ( pBest == NULL || pKf->iFrame > pBest->iFrame ) {
				pBest = pKf;
			}
		}
	}
	return pBest;
}

/* ------------------------------------------------------------------ */
/* Element interpolation across keyframes                             */
/* ------------------------------------------------------------------ */

int afTweenEvalElement(af_doc pDoc, uint32_t iLayerId, uint32_t iFrame, af_element_t* pResult)
{
	af_layer_t* pLayer;
	af_keyframe_t* pKfA = NULL;
	af_keyframe_t* pKfB = NULL;
	int i;

	if ( pDoc == NULL || pResult == NULL ) return -1;
	pLayer = afDocFindLayer(pDoc, iLayerId);
	if ( pLayer == NULL ) return -1;
	if ( pLayer->iKeyframeCount == 0 ) return -1;

	/* Find keyframe A (at or before iFrame) and B (after iFrame) */
	for ( i = 0; i < pLayer->iKeyframeCount; i++ ) {
		af_keyframe_t* pKf = &pLayer->arrKeyframes[i];
		if ( pKf->iFrame <= iFrame ) {
			if ( pKfA == NULL || pKf->iFrame > pKfA->iFrame ) pKfA = pKf;
		}
		if ( pKf->iFrame > iFrame ) {
			if ( pKfB == NULL || pKf->iFrame < pKfB->iFrame ) pKfB = pKf;
		}
	}

	if ( pKfA == NULL ) return -1;

	/* If no next keyframe or same frame, just copy from A */
	if ( pKfB == NULL || pKfA->iFrame == iFrame ) {
		if ( pKfA->iElementCount > 0 ) {
			*pResult = pKfA->arrElements[0];
			return 0;
		}
		return -1;
	}

	/* Check if element has a tween curve */
	if ( pKfA->iElementCount > 0 ) {
		af_element_t* pElemA = &pKfA->arrElements[0];

		if ( pElemA->iTweenId != 0xFFFFFFFFu ) {
			/* Use curve interpolation */
			af_curve_t* pCurve = afDocFindCurve(pDoc, pElemA->iTweenId);
			if ( pCurve != NULL && pCurve->iKeyCount >= 2 ) {
				float arrOut[AF_CHANNEL_COUNT];
				if ( afTweenEvalCurve(pCurve, (float)iFrame, arrOut) == 0 ) {
					*pResult = *pElemA;
					__channelsToElem(arrOut, pResult);
					return 0;
				}
			}
		}

		/* No tween: linear interpolation between A and B elements */
		if ( pKfB->iElementCount > 0 ) {
			af_element_t* pElemB = &pKfB->arrElements[0];
			float fSpan = (float)(pKfB->iFrame - pKfA->iFrame);
			float fT = (fSpan > 0.0f) ? (float)(iFrame - pKfA->iFrame) / fSpan : 0.0f;

			*pResult = *pElemA;
			pResult->fTx       = pElemA->fTx + (pElemB->fTx - pElemA->fTx) * fT;
			pResult->fTy       = pElemA->fTy + (pElemB->fTy - pElemA->fTy) * fT;
			pResult->fScaleX   = pElemA->fScaleX + (pElemB->fScaleX - pElemA->fScaleX) * fT;
			pResult->fScaleY   = pElemA->fScaleY + (pElemB->fScaleY - pElemA->fScaleY) * fT;
			pResult->fRotation = pElemA->fRotation + (pElemB->fRotation - pElemA->fRotation) * fT;
			pResult->fOpacity  = pElemA->fOpacity + (pElemB->fOpacity - pElemA->fOpacity) * fT;
			pResult->fSkewX    = pElemA->fSkewX + (pElemB->fSkewX - pElemA->fSkewX) * fT;
			pResult->fSkewY    = pElemA->fSkewY + (pElemB->fSkewY - pElemA->fSkewY) * fT;
			return 0;
		}

		/* Only A has element, hold its value */
		*pResult = *pElemA;
		return 0;
	}

	return -1;
}

/* ------------------------------------------------------------------ */
/* Motion tween creation                                              */
/* ------------------------------------------------------------------ */

uint32_t afTweenCreateMotionTween(af_doc pDoc, uint32_t iLayerId,
                                   uint32_t iFrameA, uint32_t iFrameB,
                                   uint32_t iInstanceId)
{
	af_keyframe_t* pKfA;
	af_keyframe_t* pKfB;
	af_element_t* pElemA = NULL;
	af_element_t* pElemB = NULL;
	af_curve_t* pCurve;
	uint32_t iCurveId = 0;
	float arrValsA[AF_CHANNEL_COUNT];
	float arrValsB[AF_CHANNEL_COUNT];
	int i;

	if ( pDoc == NULL ) return 0;
	if ( iFrameA >= iFrameB ) return 0;

	pKfA = afDocGetKeyframe(pDoc, iLayerId, iFrameA);
	pKfB = afDocGetKeyframe(pDoc, iLayerId, iFrameB);
	if ( pKfA == NULL || pKfB == NULL ) return 0;

	/* Find the element in both keyframes */
	for ( i = 0; i < pKfA->iElementCount; i++ ) {
		if ( pKfA->arrElements[i].iInstanceId == iInstanceId ) {
			pElemA = &pKfA->arrElements[i];
			break;
		}
	}
	for ( i = 0; i < pKfB->iElementCount; i++ ) {
		if ( pKfB->arrElements[i].iInstanceId == iInstanceId ) {
			pElemB = &pKfB->arrElements[i];
			break;
		}
	}
	if ( pElemA == NULL || pElemB == NULL ) return 0;

	/* Create curve */
	if ( afDocAddCurve(pDoc, iInstanceId, 0x03FFu /* channels 0-9 */, &iCurveId) != 0 ) return 0;
	pCurve = afDocFindCurve(pDoc, iCurveId);
	if ( pCurve == NULL ) return 0;

	/* Fill key A */
	__elemToChannels(pElemA, arrValsA);
	pCurve->arrKeys[0].iFrame = iFrameA;
	pCurve->arrKeys[0].iInterpType = AF_INTERP_EASE_BOTH;
	pCurve->arrKeys[0].fEaseIn = 0.0f;
	pCurve->arrKeys[0].fEaseOut = 0.0f;
	memcpy(pCurve->arrKeys[0].arrValues, arrValsA, sizeof(arrValsA));
	memset(pCurve->arrKeys[0].arrBezier, 0, sizeof(pCurve->arrKeys[0].arrBezier));

	/* Fill key B */
	__elemToChannels(pElemB, arrValsB);
	pCurve->arrKeys[1].iFrame = iFrameB;
	pCurve->arrKeys[1].iInterpType = AF_INTERP_LINEAR;
	pCurve->arrKeys[1].fEaseIn = 0.0f;
	pCurve->arrKeys[1].fEaseOut = 0.0f;
	memcpy(pCurve->arrKeys[1].arrValues, arrValsB, sizeof(arrValsB));
	memset(pCurve->arrKeys[1].arrBezier, 0, sizeof(pCurve->arrKeys[1].arrBezier));

	pCurve->iKeyCount = 2;

	/* Link element to curve */
	pElemA->iTweenId = iCurveId;

	pDoc->bModified = 1;
	return iCurveId;
}

/* ------------------------------------------------------------------ */
/* Remove motion tween                                                */
/* ------------------------------------------------------------------ */

int afTweenRemoveMotionTween(af_doc pDoc, uint32_t iInstanceId)
{
	int li, ki, ei;
	af_timeline_t* pTl;

	if ( pDoc == NULL ) return -1;
	pTl = &pDoc->tTimeline;

	/* Find element and clear its tween reference */
	for ( li = 0; li < pTl->iLayerCount; li++ ) {
		af_layer_t* pLayer = &pTl->arrLayers[li];
		for ( ki = 0; ki < pLayer->iKeyframeCount; ki++ ) {
			af_keyframe_t* pKf = &pLayer->arrKeyframes[ki];
			for ( ei = 0; ei < pKf->iElementCount; ei++ ) {
				if ( pKf->arrElements[ei].iInstanceId == iInstanceId &&
				     pKf->arrElements[ei].iTweenId != 0xFFFFFFFFu ) {
					uint32_t iCurveId = pKf->arrElements[ei].iTweenId;
					pKf->arrElements[ei].iTweenId = 0xFFFFFFFFu;
					afDocRemoveCurve(pDoc, iCurveId);
					pDoc->bModified = 1;
					return 0;
				}
			}
		}
	}
	return -4;
}
