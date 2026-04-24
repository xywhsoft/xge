#include "../../xge.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct auto_pause_lab_t auto_pause_lab_t;
typedef struct gameplay_scene_t gameplay_scene_t;
typedef struct pause_scene_t pause_scene_t;

struct gameplay_scene_t {
	xge_scene_t tScene;
	auto_pause_lab_t* pLab;
	int iEnterCount;
	int iLeaveCount;
	int iPauseCount;
	int iResumeCount;
	int iUpdateCount;
	int iDrawCount;
	int iFreeCount;
	float fMarkerX;
	float fMarkerY;
	float fVelocity;
};

struct pause_scene_t {
	xge_scene_t tScene;
	auto_pause_lab_t* pLab;
	int iEnterCount;
	int iLeaveCount;
	int iUpdateCount;
	int iDrawCount;
	int iFreeCount;
	int iLocalFrames;
};

struct auto_pause_lab_t {
	gameplay_scene_t tGame;
	pause_scene_t tPause;
	int iFrameLimit;
	int iRenderFrames;
	int iStage;
	int bPushOK;
	int bPauseOK;
	int bResumeOK;
	int bFreezeOK;
	int bOverlayOK;
	int bStackOK;
	int bCurrentOK;
	int bPopOK;
	float fPauseX;
	float fPauseSnapshotX;
	float fResumeX;
	int iGameUpdatesAtPause;
};

static int ArgInt(const char* sText, int iDefault)
{
	int iValue;

	if ( (sText == NULL) || (sText[0] == 0) ) {
		return iDefault;
	}
	iValue = atoi(sText);
	return (iValue > 0) ? iValue : iDefault;
}

static int FloatNear(float a, float b, float fTolerance)
{
	float fDiff;

	fDiff = a - b;
	if ( fDiff < 0.0f ) {
		fDiff = -fDiff;
	}
	return fDiff <= fTolerance;
}

static void DrawWorld(float fMarkerX, float fMarkerY, int bPausedView)
{
	uint32_t iBg;
	uint32_t iPanel;
	uint32_t iBorder;
	uint32_t iTrack;
	uint32_t iMarker;

	if ( bPausedView ) {
		iBg = XGE_COLOR_RGBA(22, 24, 28, 255);
		iPanel = XGE_COLOR_RGBA(42, 44, 48, 255);
		iBorder = XGE_COLOR_RGBA(118, 122, 128, 255);
		iTrack = XGE_COLOR_RGBA(138, 142, 148, 255);
		iMarker = XGE_COLOR_RGBA(220, 186, 102, 255);
	} else {
		iBg = XGE_COLOR_RGBA(18, 28, 42, 255);
		iPanel = XGE_COLOR_RGBA(28, 48, 74, 255);
		iBorder = XGE_COLOR_RGBA(104, 164, 226, 255);
		iTrack = XGE_COLOR_RGBA(150, 204, 255, 255);
		iMarker = XGE_COLOR_RGBA(246, 196, 88, 255);
	}

	xgeClear(iBg);
	xgeShapeRectFillPx((xge_rect_t){ 64.0f, 56.0f, 552.0f, 248.0f }, iPanel);
	xgeShapeRectStrokePx((xge_rect_t){ 64.0f, 56.0f, 552.0f, 248.0f }, 3.0f, iBorder);
	xgeShapeLinePx(92.0f, 240.0f, 588.0f, 240.0f, 4.0f, iTrack);
	xgeShapeRectFillPx((xge_rect_t){ 88.0f, 112.0f, 188.0f, 18.0f }, iTrack);
	xgeShapeRectFillPx((xge_rect_t){ 88.0f, 144.0f, 242.0f, 18.0f }, iTrack);
	xgeShapeRectFillPx((xge_rect_t){ 88.0f, 176.0f, 154.0f, 18.0f }, iTrack);
	xgeShapeCircleFillPx(fMarkerX, fMarkerY, 18.0f, iMarker);
	xgeShapeCircleStrokePx(fMarkerX, fMarkerY, 18.0f, 3.0f, XGE_COLOR_RGBA(255, 255, 255, 220));
	xgeShapeRectFillPx((xge_rect_t){ fMarkerX - 12.0f, fMarkerY - 44.0f, 24.0f, 10.0f }, iMarker);
}

static int GameEnter(xge_scene pScene)
{
	gameplay_scene_t* pGame;

	pGame = (gameplay_scene_t*)pScene->pUser;
	pGame->iEnterCount++;
	pGame->fMarkerX = 136.0f;
	pGame->fMarkerY = 208.0f;
	pGame->fVelocity = 36.0f;
	return XGE_OK;
}

static int GameLeave(xge_scene pScene)
{
	gameplay_scene_t* pGame;

	pGame = (gameplay_scene_t*)pScene->pUser;
	pGame->iLeaveCount++;
	return XGE_OK;
}

static int GamePause(xge_scene pScene)
{
	gameplay_scene_t* pGame;

	pGame = (gameplay_scene_t*)pScene->pUser;
	pGame->iPauseCount++;
	pGame->pLab->bPauseOK = 1;
	return XGE_OK;
}

static int GameResume(xge_scene pScene)
{
	gameplay_scene_t* pGame;
	auto_pause_lab_t* pLab;

	pGame = (gameplay_scene_t*)pScene->pUser;
	pLab = pGame->pLab;
	pGame->iResumeCount++;
	pLab->bResumeOK = 1;
	pLab->bStackOK = pLab->bStackOK && (xgeSceneCount() == 1);
	pLab->bCurrentOK = pLab->bCurrentOK && (xgeSceneCurrent() == &pGame->tScene);
	return XGE_OK;
}

static int GameUpdate(xge_scene pScene, float fDelta)
{
	gameplay_scene_t* pGame;
	auto_pause_lab_t* pLab;

	(void)fDelta;
	pGame = (gameplay_scene_t*)pScene->pUser;
	pLab = pGame->pLab;
	pGame->iUpdateCount++;
	pGame->fMarkerX += pGame->fVelocity;
	if ( pGame->fMarkerX > 544.0f ) {
		pGame->fMarkerX = 136.0f;
	}

	if ( (pLab->iStage == 0) && (pGame->iUpdateCount >= 2) ) {
		pLab->fPauseX = pGame->fMarkerX;
		pLab->iGameUpdatesAtPause = pGame->iUpdateCount;
		pLab->bPushOK = (xgeScenePush(&pLab->tPause.tScene) == XGE_OK);
		if ( pLab->bPushOK ) {
			pLab->iStage = 1;
		} else {
			xgeQuit();
		}
	} else if ( pLab->iStage == 2 ) {
		pLab->fResumeX = pGame->fMarkerX;
		pLab->iStage = 3;
		xgeQuit();
	}
	return XGE_OK;
}

static int GameDraw(xge_scene pScene)
{
	gameplay_scene_t* pGame;
	auto_pause_lab_t* pLab;

	pGame = (gameplay_scene_t*)pScene->pUser;
	pLab = pGame->pLab;
	pGame->iDrawCount++;
	pLab->iRenderFrames++;
	xgeBegin();
	DrawWorld(pGame->fMarkerX, pGame->fMarkerY, 0);
	xgeShapeRectFillPx((xge_rect_t){ 74.0f, 324.0f, 532.0f, 20.0f }, XGE_COLOR_RGBA(22, 32, 48, 255));
	xgeShapeRectFillPx((xge_rect_t){ 74.0f, 324.0f, (float)(pGame->iUpdateCount * 88), 20.0f }, XGE_COLOR_RGBA(96, 184, 255, 255));
	xgeEnd();
	xgePresent();
	if ( (pLab->iFrameLimit > 0) && (pLab->iRenderFrames >= pLab->iFrameLimit) && (pLab->iStage < 3) ) {
		xgeQuit();
	}
	return XGE_OK;
}

static int GameFree(xge_scene pScene)
{
	gameplay_scene_t* pGame;

	pGame = (gameplay_scene_t*)pScene->pUser;
	pGame->iFreeCount++;
	return XGE_OK;
}

static int PauseEnter(xge_scene pScene)
{
	pause_scene_t* pPause;
	auto_pause_lab_t* pLab;

	pPause = (pause_scene_t*)pScene->pUser;
	pLab = pPause->pLab;
	pPause->iEnterCount++;
	pPause->iLocalFrames = 0;
	pLab->bStackOK = (xgeSceneCount() == 2);
	pLab->bCurrentOK = (xgeSceneCurrent() == &pPause->tScene);
	return XGE_OK;
}

static int PauseLeave(xge_scene pScene)
{
	pause_scene_t* pPause;

	pPause = (pause_scene_t*)pScene->pUser;
	pPause->iLeaveCount++;
	return XGE_OK;
}

static int PauseUpdate(xge_scene pScene, float fDelta)
{
	pause_scene_t* pPause;
	auto_pause_lab_t* pLab;

	(void)fDelta;
	pPause = (pause_scene_t*)pScene->pUser;
	pLab = pPause->pLab;
	pPause->iUpdateCount++;
	pPause->iLocalFrames++;
	if ( pPause->iLocalFrames == 1 ) {
		pLab->fPauseSnapshotX = pLab->tGame.fMarkerX;
		pLab->bFreezeOK =
			FloatNear(pLab->fPauseSnapshotX, pLab->fPauseX, 0.01f) &&
			(pLab->tGame.iUpdateCount == pLab->iGameUpdatesAtPause);
	}
	return XGE_OK;
}

static int PauseDraw(xge_scene pScene)
{
	pause_scene_t* pPause;
	auto_pause_lab_t* pLab;
	xge_rect_t tPanel;

	pPause = (pause_scene_t*)pScene->pUser;
	pLab = pPause->pLab;
	pPause->iDrawCount++;
	pLab->iRenderFrames++;
	xgeBegin();
	DrawWorld(pLab->tGame.fMarkerX, pLab->tGame.fMarkerY, 1);
	xgeShapeRectFillPx((xge_rect_t){ 0.0f, 0.0f, 680.0f, 380.0f }, XGE_COLOR_RGBA(8, 12, 16, 136));
	tPanel.fX = 178.0f;
	tPanel.fY = 118.0f;
	tPanel.fW = 324.0f;
	tPanel.fH = 132.0f;
	xgeShapeRectFillPx(tPanel, XGE_COLOR_RGBA(28, 32, 38, 220));
	xgeShapeRectStrokePx(tPanel, 3.0f, XGE_COLOR_RGBA(210, 214, 222, 255));
	xgeShapeRectFillPx((xge_rect_t){ 214.0f, 142.0f, 40.0f, 32.0f }, XGE_COLOR_RGBA(246, 196, 88, 255));
	xgeShapeCircleStrokePx(234.0f, 142.0f, 14.0f, 4.0f, XGE_COLOR_RGBA(246, 196, 88, 255));
	xgeShapeRectFillPx((xge_rect_t){ 272.0f, 144.0f, 168.0f, 12.0f }, XGE_COLOR_RGBA(160, 168, 176, 255));
	xgeShapeRectFillPx((xge_rect_t){ 272.0f, 170.0f, 128.0f, 12.0f }, XGE_COLOR_RGBA(118, 122, 128, 255));
	xgeShapeRectFillPx((xge_rect_t){ 272.0f, 196.0f, 196.0f, 10.0f }, XGE_COLOR_RGBA(86, 176, 255, 255));
	xgeEnd();
	xgePresent();
	pLab->bOverlayOK = 1;
	if ( pPause->iLocalFrames >= 2 ) {
		pLab->bPopOK = (xgeScenePop() == XGE_OK);
		if ( pLab->bPopOK ) {
			pLab->iStage = 2;
		} else {
			xgeQuit();
		}
		return XGE_OK;
	}
	if ( (pLab->iFrameLimit > 0) && (pLab->iRenderFrames >= pLab->iFrameLimit) && (pLab->iStage < 3) ) {
		xgeQuit();
	}
	return XGE_OK;
}

static int PauseFree(xge_scene pScene)
{
	pause_scene_t* pPause;

	pPause = (pause_scene_t*)pScene->pUser;
	pPause->iFreeCount++;
	return XGE_OK;
}

static void SetupLab(auto_pause_lab_t* pLab, int iFrameLimit)
{
	memset(pLab, 0, sizeof(*pLab));
	pLab->iFrameLimit = iFrameLimit;

	pLab->tGame.pLab = pLab;
	pLab->tGame.tScene.pUser = &pLab->tGame;
	pLab->tGame.tScene.onEnter = GameEnter;
	pLab->tGame.tScene.onLeave = GameLeave;
	pLab->tGame.tScene.onPause = GamePause;
	pLab->tGame.tScene.onResume = GameResume;
	pLab->tGame.tScene.onUpdate = GameUpdate;
	pLab->tGame.tScene.onDraw = GameDraw;
	pLab->tGame.tScene.onFree = GameFree;

	pLab->tPause.pLab = pLab;
	pLab->tPause.tScene.pUser = &pLab->tPause;
	pLab->tPause.tScene.onEnter = PauseEnter;
	pLab->tPause.tScene.onLeave = PauseLeave;
	pLab->tPause.tScene.onUpdate = PauseUpdate;
	pLab->tPause.tScene.onDraw = PauseDraw;
	pLab->tPause.tScene.onFree = PauseFree;
}

int main(int argc, char** argv)
{
	xge_desc_t tDesc;
	auto_pause_lab_t tLab;
	int i;
	int iRet;
	int bOK;

	SetupLab(&tLab, ArgInt(getenv("XGE_AUTO_PAUSE_LAB_FRAMES"), 180));
	for ( i = 1; i < argc; i++ ) {
		if ( (strcmp(argv[i], "--frames") == 0) && ((i + 1) < argc) ) {
			tLab.iFrameLimit = ArgInt(argv[++i], tLab.iFrameLimit);
		}
	}

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iWidth = 680;
	tDesc.iHeight = 380;
	tDesc.sTitle = "XGE Auto Pause Lab";
	tDesc.iFlags = XGE_INIT_WINDOW | XGE_INIT_VSYNC;
	tDesc.iRunMode = XGE_RUN_GAME_LOOP;
	tDesc.iTargetFPS = 60;
	iRet = xgeInit(&tDesc);
	if ( iRet != XGE_OK ) {
		return 1;
	}
	iRet = xgeSceneSet(&tLab.tGame.tScene);
	if ( iRet != XGE_OK ) {
		xgeUnit();
		return 2;
	}
	xgeRun(NULL, NULL);
	xgeSceneSet(NULL);

	bOK =
		tLab.bPushOK &&
		tLab.bPauseOK &&
		tLab.bResumeOK &&
		tLab.bFreezeOK &&
		tLab.bOverlayOK &&
		tLab.bStackOK &&
		tLab.bCurrentOK &&
		tLab.bPopOK &&
		(tLab.tGame.iEnterCount == 1) &&
		(tLab.tGame.iPauseCount == 1) &&
		(tLab.tGame.iResumeCount == 1) &&
		(tLab.tGame.iLeaveCount == 1) &&
		(tLab.tGame.iFreeCount == 1) &&
		(tLab.tPause.iEnterCount == 1) &&
		(tLab.tPause.iLeaveCount == 1) &&
		(tLab.tPause.iFreeCount == 1) &&
		(tLab.tGame.iUpdateCount >= 3) &&
		(tLab.tPause.iUpdateCount >= 2) &&
		(tLab.fResumeX > tLab.fPauseX);

	printf(
		"auto-pause-lab final-summary frames=%d flow=%d pause=%d resume=%d freeze=%d overlay=%d stack=%d pop=%d game(enter=%d pause=%d resume=%d leave=%d update=%d draw=%d free=%d x=%.2f/%.2f/%.2f) pause(enter=%d leave=%d update=%d draw=%d free=%d)\n",
		tLab.iRenderFrames,
		tLab.bPushOK,
		tLab.bPauseOK,
		tLab.bResumeOK,
		tLab.bFreezeOK,
		tLab.bOverlayOK,
		tLab.bStackOK && tLab.bCurrentOK,
		tLab.bPopOK,
		tLab.tGame.iEnterCount,
		tLab.tGame.iPauseCount,
		tLab.tGame.iResumeCount,
		tLab.tGame.iLeaveCount,
		tLab.tGame.iUpdateCount,
		tLab.tGame.iDrawCount,
		tLab.tGame.iFreeCount,
		tLab.fPauseX,
		tLab.fPauseSnapshotX,
		tLab.fResumeX,
		tLab.tPause.iEnterCount,
		tLab.tPause.iLeaveCount,
		tLab.tPause.iUpdateCount,
		tLab.tPause.iDrawCount,
		tLab.tPause.iFreeCount);
	xgeUnit();
	return bOK ? 0 : 3;
}
