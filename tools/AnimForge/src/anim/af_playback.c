/*
 * af_playback.c - AnimForge playback state machine implementation
 */

#include "af_playback.h"
#include <string.h>

/* ------------------------------------------------------------------ */
/* Initialization                                                     */
/* ------------------------------------------------------------------ */

void afPlaybackInit(af_playback_t* pPlay, af_doc pDoc)
{
	if ( pPlay == NULL ) return;
	memset(pPlay, 0, sizeof(af_playback_t));
	pPlay->iState = AF_PLAY_STOPPED;
	pPlay->bLoop = 1;
	pPlay->fSpeed = 1.0f;
	pPlay->iCurrentFrame = 0;
	if ( pDoc != NULL ) {
		pPlay->fFrameRate = pDoc->fFrameRate;
		pPlay->iFrameCount = pDoc->iFrameCount;
	} else {
		pPlay->fFrameRate = 24.0f;
		pPlay->iFrameCount = 120;
	}
}

/* ------------------------------------------------------------------ */
/* Transport controls                                                 */
/* ------------------------------------------------------------------ */

void afPlaybackPlay(af_playback_t* pPlay)
{
	if ( pPlay == NULL ) return;
	pPlay->iState = AF_PLAY_PLAYING;
	pPlay->fAccum = 0.0;
}

void afPlaybackPause(af_playback_t* pPlay)
{
	if ( pPlay == NULL ) return;
	if ( pPlay->iState == AF_PLAY_PLAYING ) {
		pPlay->iState = AF_PLAY_PAUSED;
	}
}

void afPlaybackStop(af_playback_t* pPlay)
{
	if ( pPlay == NULL ) return;
	pPlay->iState = AF_PLAY_STOPPED;
	pPlay->iCurrentFrame = 0;
	pPlay->fAccum = 0.0;
}

void afPlaybackToggle(af_playback_t* pPlay)
{
	if ( pPlay == NULL ) return;
	if ( pPlay->iState == AF_PLAY_PLAYING ) {
		afPlaybackPause(pPlay);
	} else {
		afPlaybackPlay(pPlay);
	}
}

/* ------------------------------------------------------------------ */
/* Frame stepping                                                     */
/* ------------------------------------------------------------------ */

void afPlaybackStepForward(af_playback_t* pPlay)
{
	if ( pPlay == NULL ) return;
	if ( pPlay->iCurrentFrame < pPlay->iFrameCount - 1 ) {
		pPlay->iCurrentFrame++;
	} else if ( pPlay->bLoop ) {
		pPlay->iCurrentFrame = 0;
	}
}

void afPlaybackStepBack(af_playback_t* pPlay)
{
	if ( pPlay == NULL ) return;
	if ( pPlay->iCurrentFrame > 0 ) {
		pPlay->iCurrentFrame--;
	} else if ( pPlay->bLoop ) {
		pPlay->iCurrentFrame = pPlay->iFrameCount - 1;
	}
}

void afPlaybackGotoFrame(af_playback_t* pPlay, uint32_t iFrame)
{
	if ( pPlay == NULL ) return;
	if ( iFrame >= pPlay->iFrameCount ) iFrame = pPlay->iFrameCount - 1;
	pPlay->iCurrentFrame = iFrame;
	pPlay->fAccum = 0.0;
}

/* ------------------------------------------------------------------ */
/* Update                                                             */
/* ------------------------------------------------------------------ */

void afPlaybackUpdate(af_playback_t* pPlay, float fDelta)
{
	double fFrameDuration;

	if ( pPlay == NULL ) return;
	if ( pPlay->iState != AF_PLAY_PLAYING ) return;
	if ( pPlay->fFrameRate <= 0.0f ) return;

	fFrameDuration = 1.0 / (double)(pPlay->fFrameRate * pPlay->fSpeed);
	pPlay->fAccum += (double)fDelta;

	while ( pPlay->fAccum >= fFrameDuration ) {
		pPlay->fAccum -= fFrameDuration;
		if ( pPlay->iCurrentFrame < pPlay->iFrameCount - 1 ) {
			pPlay->iCurrentFrame++;
		} else {
			if ( pPlay->bLoop ) {
				pPlay->iCurrentFrame = 0;
			} else {
				pPlay->iState = AF_PLAY_STOPPED;
				pPlay->fAccum = 0.0;
				break;
			}
		}
	}
}

/* ------------------------------------------------------------------ */
/* Sync from document                                                 */
/* ------------------------------------------------------------------ */

void afPlaybackSyncDoc(af_playback_t* pPlay, af_doc pDoc)
{
	if ( pPlay == NULL || pDoc == NULL ) return;
	pPlay->fFrameRate = pDoc->fFrameRate;
	pPlay->iFrameCount = pDoc->iFrameCount;
	if ( pPlay->iCurrentFrame >= pPlay->iFrameCount ) {
		pPlay->iCurrentFrame = pPlay->iFrameCount - 1;
	}
}

/* ------------------------------------------------------------------ */
/* Frame evaluation                                                   */
/* ------------------------------------------------------------------ */

int afPlaybackEvalFrame(af_doc pDoc, uint32_t iFrame, af_element_t* pOut, int iMaxOut)
{
	int iLayer, iCount = 0;
	af_timeline_t* pTl;

	if ( pDoc == NULL || pOut == NULL || iMaxOut <= 0 ) return 0;
	pTl = &pDoc->tTimeline;

	/* Evaluate layers from bottom to top (index 0 = bottom) */
	for ( iLayer = 0; iLayer < pTl->iLayerCount && iCount < iMaxOut; iLayer++ ) {
		af_layer_t* pLayer = &pTl->arrLayers[iLayer];
		af_element_t tEval;

		/* Skip invisible layers */
		if ( (pLayer->iFlags & AF_LAYER_FLAG_VISIBLE) == 0 ) continue;
		/* Skip guide layers in playback */
		if ( pLayer->iType == AF_LAYER_GUIDE ) continue;

		if ( afTweenEvalElement(pDoc, pLayer->iId, iFrame, &tEval) == 0 ) {
			/* Skip invisible elements */
			if ( (tEval.iFlags & AF_ELEM_FLAG_VISIBLE) == 0 ) continue;
			pOut[iCount] = tEval;
			iCount++;
		}
	}

	return iCount;
}
