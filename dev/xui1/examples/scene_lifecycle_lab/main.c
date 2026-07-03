#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../xge.h"

typedef struct scene_counts_t {
	const char* sName;
	int iEnter;
	int iLeave;
	int iPause;
	int iResume;
	int iEvent;
	int iUpdate;
	int iDraw;
	int iFree;
} scene_counts_t;

typedef struct scene_lifecycle_lab_t scene_lifecycle_lab_t;

typedef struct scene_ctx_t {
	scene_lifecycle_lab_t* pLab;
	scene_counts_t* pCounts;
	int iId;
	uint32_t iColor;
} scene_ctx_t;

struct scene_lifecycle_lab_t {
	xge_scene_t tBaseScene;
	xge_scene_t tOverlayScene;
	xge_scene_t tReplacementScene;
	scene_ctx_t tBaseCtx;
	scene_ctx_t tOverlayCtx;
	scene_ctx_t tReplacementCtx;
	scene_counts_t tBaseCounts;
	scene_counts_t tOverlayCounts;
	scene_counts_t tReplacementCounts;
	int iFrameLimit;
	int iStage;
	int iTotalUpdates;
	int iTotalDraws;
	int iInitialMode;
	int iInitialMaxUpdates;
	int iVariableMode;
	int iVariableMaxUpdates;
	float fInitialFixedStep;
	float fVariableFixedStep;
	int bCurrentOK;
	int bCountOK;
	int bFixedOK;
	int bVariableOK;
	int bDispatchedBase;
	int bDispatchedOverlay;
	int bDispatchedReplacement;
	int bCleaned;
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

static int SceneEnter(xge_scene pScene)
{
	scene_ctx_t* pCtx;

	pCtx = (scene_ctx_t*)pScene->pUser;
	pCtx->pCounts->iEnter++;
	return XGE_OK;
}

static int SceneLeave(xge_scene pScene)
{
	scene_ctx_t* pCtx;

	pCtx = (scene_ctx_t*)pScene->pUser;
	pCtx->pCounts->iLeave++;
	return XGE_OK;
}

static int ScenePause(xge_scene pScene)
{
	scene_ctx_t* pCtx;

	pCtx = (scene_ctx_t*)pScene->pUser;
	pCtx->pCounts->iPause++;
	return XGE_OK;
}

static int SceneResume(xge_scene pScene)
{
	scene_ctx_t* pCtx;

	pCtx = (scene_ctx_t*)pScene->pUser;
	pCtx->pCounts->iResume++;
	return XGE_OK;
}

static int SceneEvent(xge_scene pScene, const xge_event_t* pEvent)
{
	scene_ctx_t* pCtx;

	(void)pEvent;
	pCtx = (scene_ctx_t*)pScene->pUser;
	pCtx->pCounts->iEvent++;
	return XGE_OK;
}

static int DispatchProbeEvent(scene_lifecycle_lab_t* pLab)
{
	xge_event_t tEvent;

	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_NONE;
	tEvent.iParam1 = pLab->iStage;
	return xgeSceneDispatchEvent(&tEvent);
}

static int SceneUpdate(xge_scene pScene, float fDelta)
{
	scene_ctx_t* pCtx;
	scene_lifecycle_lab_t* pLab;

	(void)fDelta;
	pCtx = (scene_ctx_t*)pScene->pUser;
	pLab = pCtx->pLab;
	pCtx->pCounts->iUpdate++;
	pLab->iTotalUpdates++;
	if ( (pCtx->iId == 0) && (pLab->iStage == 0) ) {
		if ( DispatchProbeEvent(pLab) == XGE_OK ) {
			pLab->bDispatchedBase = 1;
		}
		pLab->bCurrentOK = (xgeSceneCurrent() == &pLab->tBaseScene);
		pLab->bCountOK = (xgeSceneCount() == 1);
		pLab->iStage = 1;
		return xgeScenePush(&pLab->tOverlayScene);
	}
	if ( (pCtx->iId == 1) && (pLab->iStage == 1) ) {
		if ( DispatchProbeEvent(pLab) == XGE_OK ) {
			pLab->bDispatchedOverlay = 1;
		}
		pLab->bCountOK = pLab->bCountOK && (xgeSceneCount() == 2);
		pLab->iStage = 2;
		return xgeScenePop();
	}
	if ( (pCtx->iId == 0) && (pLab->iStage == 2) ) {
		if ( DispatchProbeEvent(pLab) == XGE_OK ) {
			pLab->bDispatchedBase = 1;
		}
		if ( xgeSceneUpdateStrategySet(XGE_UPDATE_VARIABLE, 1.0f / 30.0f, 2) == XGE_OK ) {
			xgeSceneUpdateStrategyGet(&pLab->iVariableMode, &pLab->fVariableFixedStep, &pLab->iVariableMaxUpdates);
			pLab->bVariableOK = (pLab->iVariableMode == XGE_UPDATE_VARIABLE) && (pLab->iVariableMaxUpdates == 2);
		}
		pLab->bCountOK = pLab->bCountOK && (xgeSceneCount() == 1);
		pLab->iStage = 3;
		return xgeSceneReplace(&pLab->tReplacementScene);
	}
	if ( (pCtx->iId == 2) && (pLab->iStage == 3) ) {
		if ( pCtx->pCounts->iUpdate == 1 ) {
			if ( DispatchProbeEvent(pLab) == XGE_OK ) {
				pLab->bDispatchedReplacement = 1;
			}
			pLab->bCurrentOK = pLab->bCurrentOK && (xgeSceneCurrent() == &pLab->tReplacementScene);
			pLab->bCountOK = pLab->bCountOK && (xgeSceneCount() == 1);
		}
		if ( (pCtx->pCounts->iUpdate >= 2) || (pLab->iTotalDraws >= pLab->iFrameLimit) ) {
			pLab->iStage = 4;
			xgeQuit();
		}
	}
	return XGE_OK;
}

static int SceneDraw(xge_scene pScene)
{
	scene_ctx_t* pCtx;
	scene_lifecycle_lab_t* pLab;
	xge_rect_t tRect;
	float fX;

	pCtx = (scene_ctx_t*)pScene->pUser;
	pLab = pCtx->pLab;
	pCtx->pCounts->iDraw++;
	pLab->iTotalDraws++;
	xgeClear(XGE_COLOR_RGBA(14, 18, 24, 255));
	tRect.fX = 48.0f + (float)(pCtx->iId * 48);
	tRect.fY = 52.0f + (float)(pCtx->iId * 26);
	tRect.fW = 320.0f;
	tRect.fH = 156.0f;
	xgeShapeRectFillPx(tRect, XGE_COLOR_RGBA(28, 36, 48, 255));
	xgeShapeRectStrokePx(tRect, 3.0f, pCtx->iColor);
	fX = 96.0f + (float)((pCtx->pCounts->iUpdate * 34) % 360);
	xgeShapeCircleFillPx(fX, 260.0f, 24.0f, pCtx->iColor);
	xgeShapeLinePx(54.0f, 318.0f, 586.0f, 318.0f, 3.0f, XGE_COLOR_RGBA(128, 156, 184, 255));
	return XGE_OK;
}

static int SceneFree(xge_scene pScene)
{
	scene_ctx_t* pCtx;

	pCtx = (scene_ctx_t*)pScene->pUser;
	pCtx->pCounts->iFree++;
	return XGE_OK;
}

static void SetupScene(xge_scene pScene, scene_ctx_t* pCtx, scene_lifecycle_lab_t* pLab, scene_counts_t* pCounts, const char* sName, int iId, uint32_t iColor)
{
	memset(pScene, 0, sizeof(*pScene));
	memset(pCtx, 0, sizeof(*pCtx));
	memset(pCounts, 0, sizeof(*pCounts));
	pCounts->sName = sName;
	pCtx->pLab = pLab;
	pCtx->pCounts = pCounts;
	pCtx->iId = iId;
	pCtx->iColor = iColor;
	pScene->pUser = pCtx;
	pScene->onEnter = SceneEnter;
	pScene->onLeave = SceneLeave;
	pScene->onPause = ScenePause;
	pScene->onResume = SceneResume;
	pScene->onEvent = SceneEvent;
	pScene->onUpdate = SceneUpdate;
	pScene->onDraw = SceneDraw;
	pScene->onFree = SceneFree;
}

static void InitLab(scene_lifecycle_lab_t* pLab)
{
	memset(pLab, 0, sizeof(*pLab));
	pLab->iFrameLimit = ArgInt(getenv("XGE_SCENE_LIFECYCLE_FRAMES"), 0);
	SetupScene(&pLab->tBaseScene, &pLab->tBaseCtx, pLab, &pLab->tBaseCounts, "base", 0, XGE_COLOR_RGBA(96, 184, 255, 255));
	SetupScene(&pLab->tOverlayScene, &pLab->tOverlayCtx, pLab, &pLab->tOverlayCounts, "overlay", 1, XGE_COLOR_RGBA(246, 196, 88, 255));
	SetupScene(&pLab->tReplacementScene, &pLab->tReplacementCtx, pLab, &pLab->tReplacementCounts, "replacement", 2, XGE_COLOR_RGBA(122, 224, 152, 255));
}

static void PrintCounts(const char* sPrefix, const scene_counts_t* pCounts)
{
	printf("%s %s enter=%d leave=%d pause=%d resume=%d event=%d update=%d draw=%d free=%d\n",
		sPrefix,
		pCounts->sName,
		pCounts->iEnter,
		pCounts->iLeave,
		pCounts->iPause,
		pCounts->iResume,
		pCounts->iEvent,
		pCounts->iUpdate,
		pCounts->iDraw,
		pCounts->iFree);
}

static int ValidateLab(scene_lifecycle_lab_t* pLab)
{
	return (pLab->tBaseCounts.iEnter == 1) &&
		(pLab->tBaseCounts.iLeave == 1) &&
		(pLab->tBaseCounts.iPause == 1) &&
		(pLab->tBaseCounts.iResume == 1) &&
		(pLab->tBaseCounts.iEvent >= 1) &&
		(pLab->tBaseCounts.iUpdate >= 2) &&
		(pLab->tBaseCounts.iDraw >= 1) &&
		(pLab->tBaseCounts.iFree == 1) &&
		(pLab->tOverlayCounts.iEnter == 1) &&
		(pLab->tOverlayCounts.iLeave == 1) &&
		(pLab->tOverlayCounts.iEvent == 1) &&
		(pLab->tOverlayCounts.iUpdate >= 1) &&
		(pLab->tOverlayCounts.iDraw >= 1) &&
		(pLab->tOverlayCounts.iFree == 1) &&
		(pLab->tReplacementCounts.iEnter == 1) &&
		(pLab->tReplacementCounts.iLeave == 1) &&
		(pLab->tReplacementCounts.iEvent == 1) &&
		(pLab->tReplacementCounts.iUpdate >= 2) &&
		(pLab->tReplacementCounts.iDraw >= 1) &&
		(pLab->tReplacementCounts.iFree == 1) &&
		pLab->bCurrentOK &&
		pLab->bCountOK &&
		pLab->bFixedOK &&
		pLab->bVariableOK &&
		pLab->bDispatchedBase &&
		pLab->bDispatchedOverlay &&
		pLab->bDispatchedReplacement;
}

int main(int argc, char** argv)
{
	scene_lifecycle_lab_t tLab;
	xge_desc_t tDesc;
	int i;
	int iRet;
	int bOK;

	InitLab(&tLab);
	for ( i = 1; i < argc; i++ ) {
		if ( (strcmp(argv[i], "--frames") == 0) && ((i + 1) < argc) ) {
			tLab.iFrameLimit = ArgInt(argv[++i], tLab.iFrameLimit);
		}
	}
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iWidth = 640;
	tDesc.iHeight = 360;
	tDesc.sTitle = "XGE Scene Lifecycle Lab";
	tDesc.iFlags = XGE_INIT_WINDOW | XGE_INIT_VSYNC;
	tDesc.iRunMode = XGE_RUN_GAME_LOOP;
	tDesc.iTargetFPS = 60;
	iRet = xgeInit(&tDesc);
	if ( iRet != XGE_OK ) {
		return 1;
	}
	iRet = xgeSceneUpdateStrategySet(XGE_UPDATE_FIXED, 1.0f / 60.0f, 4);
	if ( iRet == XGE_OK ) {
		xgeSceneUpdateStrategyGet(&tLab.iInitialMode, &tLab.fInitialFixedStep, &tLab.iInitialMaxUpdates);
		tLab.bFixedOK = (tLab.iInitialMode == XGE_UPDATE_FIXED) && (tLab.iInitialMaxUpdates == 4);
	}
	iRet = xgeSceneSet(&tLab.tBaseScene);
	if ( iRet != XGE_OK ) {
		xgeUnit();
		return 2;
	}
	xgeRun(NULL, NULL);
	xgeSceneSet(NULL);
	tLab.bCleaned = 1;
	PrintCounts("scene-lifecycle-lab counts", &tLab.tBaseCounts);
	PrintCounts("scene-lifecycle-lab counts", &tLab.tOverlayCounts);
	PrintCounts("scene-lifecycle-lab counts", &tLab.tReplacementCounts);
	bOK = ValidateLab(&tLab);
	printf("scene-lifecycle-lab final-summary ok=%d stage=%d updates=%d draws=%d current=%d count=%d fixed=(%d step=%.5f max=%d ok=%d) variable=(%d step=%.5f max=%d ok=%d) dispatched=(%d,%d,%d) cleaned=%d\n",
		bOK,
		tLab.iStage,
		tLab.iTotalUpdates,
		tLab.iTotalDraws,
		tLab.bCurrentOK,
		tLab.bCountOK,
		tLab.iInitialMode,
		tLab.fInitialFixedStep,
		tLab.iInitialMaxUpdates,
		tLab.bFixedOK,
		tLab.iVariableMode,
		tLab.fVariableFixedStep,
		tLab.iVariableMaxUpdates,
		tLab.bVariableOK,
		tLab.bDispatchedBase,
		tLab.bDispatchedOverlay,
		tLab.bDispatchedReplacement,
		tLab.bCleaned);
	xgeUnit();
	return bOK ? 0 : 3;
}
