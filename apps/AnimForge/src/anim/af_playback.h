/*
 * af_playback.h - AnimForge playback state machine
 *
 * Controls animation playback: play/pause/stop/step,
 * evaluates the document at the current frame, and
 * renders the result to the stage.
 */

#ifndef AF_PLAYBACK_H
#define AF_PLAYBACK_H

#include "../core/af_doc.h"
#include "af_tween.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Playback state */
#define AF_PLAY_STOPPED   0
#define AF_PLAY_PLAYING   1
#define AF_PLAY_PAUSED    2

typedef struct af_playback_t {
	int      iState;          /* AF_PLAY_* */
	int      bLoop;
	float    fSpeed;          /* playback speed multiplier */
	float    fFrameRate;      /* cached from doc */
	uint32_t iCurrentFrame;
	uint32_t iFrameCount;
	double   fAccum;          /* time accumulator (seconds) */
} af_playback_t;

/* Initialize playback from document settings */
void afPlaybackInit(af_playback_t* pPlay, af_doc pDoc);

/* Transport controls */
void afPlaybackPlay(af_playback_t* pPlay);
void afPlaybackPause(af_playback_t* pPlay);
void afPlaybackStop(af_playback_t* pPlay);
void afPlaybackToggle(af_playback_t* pPlay);

/* Frame stepping */
void afPlaybackStepForward(af_playback_t* pPlay);
void afPlaybackStepBack(af_playback_t* pPlay);
void afPlaybackGotoFrame(af_playback_t* pPlay, uint32_t iFrame);

/* Update (call each frame with delta time in seconds) */
void afPlaybackUpdate(af_playback_t* pPlay, float fDelta);

/* Sync from document (after doc changes) */
void afPlaybackSyncDoc(af_playback_t* pPlay, af_doc pDoc);

/*
 * Evaluate the full document at the current frame.
 * For each visible layer, computes the interpolated element
 * transforms and stores them in the output array.
 * Returns number of active elements written.
 */
int afPlaybackEvalFrame(af_doc pDoc, uint32_t iFrame,
                        af_element_t* pOut, int iMaxOut);

#ifdef __cplusplus
}
#endif

#endif /* AF_PLAYBACK_H */
