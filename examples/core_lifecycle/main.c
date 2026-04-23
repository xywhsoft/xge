#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../xge.h"

typedef struct lifecycle_state_t {
	int iFrameLimit;
	double fSecondLimit;
	int iManualFrames;
	int iWindowFrames;
	int bTitleUpdated;
} lifecycle_state_t;

static int ArgInt(const char* sText, int iDefault)
{
	int iValue;

	if ( (sText == NULL) || (sText[0] == 0) ) {
		return iDefault;
	}
	iValue = atoi(sText);
	return (iValue > 0) ? iValue : iDefault;
}

static double ArgDouble(const char* sText, double fDefault)
{
	double fValue;

	if ( (sText == NULL) || (sText[0] == 0) ) {
		return fDefault;
	}
	fValue = atof(sText);
	return (fValue > 0.0) ? fValue : fDefault;
}

static void PrintStats(const char* sPrefix, const lifecycle_state_t* pState)
{
	xge_frame_stats_t tStats;

	tStats = xgeFrameStatsGet();
	printf("%s width=%d height=%d frame=%d manual=%d window=%d fps=%d delta=%.5f timer=%.3f now=%lld draw=%d batch=%d dirty=%d frame_ms=%.3f avg_ms=%.3f max_ms=%.3f\n",
		sPrefix,
		xgeGetWidth(),
		xgeGetHeight(),
		tStats.iFrameCount,
		(pState != NULL) ? pState->iManualFrames : 0,
		(pState != NULL) ? pState->iWindowFrames : 0,
		xgeGetFPS(),
		xgeGetDelta(),
		xgeTimer(),
		(long long)xgeTimeNow(),
		tStats.iDrawCallCount,
		tStats.iBatchCount,
		tStats.iDirtyRectCount,
		tStats.fFrameTimeMs,
		tStats.fFrameTimeAvgMs,
		tStats.fFrameTimeMaxMs);
}

static int ManualFrame(void* pUser)
{
	lifecycle_state_t* pState;
	xge_rect_t tRect;

	pState = (lifecycle_state_t*)pUser;
	pState->iManualFrames++;
	if ( xgeRender() < 0 ) {
		return 1;
	}
	tRect.fX = 16.0f;
	tRect.fY = 16.0f;
	tRect.fW = 96.0f;
	tRect.fH = 48.0f;
	xgeClear(XGE_COLOR_RGBA(18, 24, 36, 255));
	xgeShapeRectFill(tRect, XGE_COLOR_RGBA(64, 128, 220, 255));
	xgePresent();
	if ( pState->iManualFrames >= 3 ) {
		PrintStats("core-lifecycle manual-summary", pState);
		xgeQuit();
		return 1;
	}
	return 0;
}

static int WindowFrame(void* pUser)
{
	lifecycle_state_t* pState;
	xge_rect_t tBar;
	float fProgress;
	int iWidth;
	int iHeight;

	pState = (lifecycle_state_t*)pUser;
	pState->iWindowFrames++;
	iWidth = xgeGetWidth();
	iHeight = xgeGetHeight();
	if ( pState->bTitleUpdated == 0 ) {
		xgeSetTitle("XGE Core Lifecycle");
		pState->bTitleUpdated = 1;
	}
	if ( (pState->iWindowFrames == 1) || ((pState->iWindowFrames % 60) == 0) ) {
		PrintStats("core-lifecycle frame", pState);
	}
	if ( xgeKeyDown(XGE_KEY_ESCAPE) ) {
		PrintStats("core-lifecycle esc-summary", pState);
		xgeQuit();
		return 1;
	}
	fProgress = 0.0f;
	if ( pState->iFrameLimit > 0 ) {
		fProgress = (float)pState->iWindowFrames / (float)pState->iFrameLimit;
		if ( fProgress > 1.0f ) {
			fProgress = 1.0f;
		}
	}
	xgeClear(XGE_COLOR_RGBA(20, 28, 42, 255));
	tBar.fX = 32.0f;
	tBar.fY = (float)iHeight - 64.0f;
	tBar.fW = ((float)iWidth - 64.0f) * ((fProgress > 0.02f) ? fProgress : 0.02f);
	tBar.fH = 20.0f;
	xgeShapeRectFill(tBar, XGE_COLOR_RGBA(84, 160, 240, 255));
	xgeShapeCircleFill((float)iWidth * 0.5f, (float)iHeight * 0.5f, 48.0f + (float)(pState->iWindowFrames % 60), XGE_COLOR_RGBA(240, 128, 72, 220));
	if ( ((pState->iFrameLimit > 0) && (pState->iWindowFrames >= pState->iFrameLimit)) || ((pState->fSecondLimit > 0.0) && (xgeTimer() >= pState->fSecondLimit)) ) {
		PrintStats("core-lifecycle final-summary", pState);
		xgeQuit();
		return 1;
	}
	return 0;
}

static int RunManualProbe(lifecycle_state_t* pState)
{
	xge_desc_t tDesc;
	int iRet;
	int iFrameRet;

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iWidth = 320;
	tDesc.iHeight = 180;
	tDesc.sTitle = "XGE Core Lifecycle Manual Probe";
	tDesc.iFlags = 0;
	tDesc.iRunMode = XGE_RUN_MANUAL;
	tDesc.iTargetFPS = 60;
	iRet = xgeInit(&tDesc);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	xgeFrameStatsReset();
	xgeSleep(1);
	iRet = xgeRun(ManualFrame, pState);
	while ( iRet == XGE_OK ) {
		iFrameRet = xgeFrame();
		if ( iFrameRet <= 0 ) {
			break;
		}
		if ( pState->iManualFrames >= 3 ) {
			break;
		}
	}
	xgeUnit();
	return XGE_OK;
}

static int RunWindowLoop(lifecycle_state_t* pState)
{
	xge_desc_t tDesc;
	int iRet;

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iWidth = 800;
	tDesc.iHeight = 480;
	tDesc.sTitle = "XGE Core Lifecycle";
	tDesc.iFlags = XGE_INIT_WINDOW | XGE_INIT_VSYNC;
	tDesc.iRunMode = XGE_RUN_GAME_LOOP;
	tDesc.iTargetFPS = 60;
	iRet = xgeInit(&tDesc);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	xgeFrameStatsReset();
	iRet = xgeRun(WindowFrame, pState);
	xgeUnit();
	return iRet;
}

int main(int argc, char** argv)
{
	lifecycle_state_t tState;
	int i;
	int iRet;

	memset(&tState, 0, sizeof(tState));
	tState.iFrameLimit = ArgInt(getenv("XGE_LIFECYCLE_FRAMES"), 180);
	tState.fSecondLimit = ArgDouble(getenv("XGE_LIFECYCLE_SECONDS"), 0.0);
	for ( i = 1; i < argc; i++ ) {
		if ( (strcmp(argv[i], "--frames") == 0) && ((i + 1) < argc) ) {
			tState.iFrameLimit = ArgInt(argv[++i], tState.iFrameLimit);
		} else if ( (strcmp(argv[i], "--seconds") == 0) && ((i + 1) < argc) ) {
			tState.fSecondLimit = ArgDouble(argv[++i], tState.fSecondLimit);
		}
	}
	iRet = RunManualProbe(&tState);
	if ( iRet != XGE_OK ) {
		fprintf(stderr, "core-lifecycle manual probe failed: %d\n", iRet);
		return 1;
	}
	iRet = RunWindowLoop(&tState);
	if ( iRet != XGE_OK ) {
		fprintf(stderr, "core-lifecycle window loop failed: %d\n", iRet);
		return 2;
	}
	printf("core-lifecycle summary manual=%d window=%d\n", tState.iManualFrames, tState.iWindowFrames);
	return 0;
}
