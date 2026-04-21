static int __xgeSceneFrame(void);
static void __xgeSceneClear(void);

int xgeInit(const xge_desc_t* pDesc)
{
	xge_desc_t objDesc;

	if ( g_xge.bInitialized ) {
		return XGE_ERROR_ALREADY_INITIALIZED;
	}

	memset(&g_xge, 0, sizeof(g_xge));
	memset(&objDesc, 0, sizeof(objDesc));

	if ( pDesc != NULL ) {
		objDesc = *pDesc;
	}
	if ( objDesc.iWidth <= 0 ) {
		objDesc.iWidth = 800;
	}
	if ( objDesc.iHeight <= 0 ) {
		objDesc.iHeight = 600;
	}
	if ( objDesc.sTitle == NULL ) {
		objDesc.sTitle = "XGE";
	}
	if ( (objDesc.iRunMode != XGE_RUN_GAME_LOOP) && (objDesc.iRunMode != XGE_RUN_MANUAL) ) {
		objDesc.iRunMode = XGE_RUN_GAME_LOOP;
	}
	if ( objDesc.iTargetFPS <= 0 ) {
		objDesc.iTargetFPS = 60;
	}

	if ( xrtInit() == NULL ) {
		return XGE_ERROR_BACKEND_FAILED;
	}

	g_xge.objDesc = objDesc;
	g_xge.iWidth = objDesc.iWidth;
	g_xge.iHeight = objDesc.iHeight;
	g_xge.iClearColor = XGE_COLOR_RGBA(0, 0, 0, 255);
	g_xge.iBlend = XGE_BLEND_ALPHA;
	g_xge.fDelta = 1.0f / 60.0f;
	g_xge.fStartTime = xrtTimer();
	g_xge.iSceneUpdateMode = XGE_UPDATE_VARIABLE;
	g_xge.iSceneMaxUpdates = 1;
	g_xge.fSceneFixedStep = 1.0f / 60.0f;
	g_xge.tCamera = xgeCameraDefault((float)objDesc.iWidth, (float)objDesc.iHeight);
#ifndef XGE_NO_AUDIO
	g_xge.tAudioListener.tForward.fZ = -1.0f;
	g_xge.tAudioListener.tUp.fY = 1.0f;
#endif
	g_xge.bInitialized = 1;
	g_xge.bRunning = 1;
	return XGE_OK;
}

void xgeUnit(void)
{
	if ( g_xge.bInitialized == 0 ) {
		return;
	}
	g_xge.bInitialized = 0;
	g_xge.bRunning = 0;
	__xgeSceneClear();
	xgeAudioUnit();
	xrtUnit();
}

int xgeRun(xge_scene_proc procFrame, void* pUser)
{
	sapp_desc objDesc;

	if ( g_xge.bInitialized == 0 ) {
		return XGE_ERROR_NOT_INITIALIZED;
	}

	g_xge.procFrame = procFrame;
	g_xge.pFrameUser = pUser;
	if ( g_xge.objDesc.iRunMode == XGE_RUN_MANUAL ) {
		return XGE_OK;
	}
	objDesc = __xgeMakeSokolDesc();
	sapp_run(&objDesc);
	return XGE_OK;
}

void xgeQuit(void)
{
	g_xge.bRunning = 0;
	if ( g_xge.bSokolRunning ) {
		sapp_quit();
	}
}

int xgeFrame(void)
{
	int iRet;

	if ( g_xge.bInitialized == 0 ) {
		return XGE_ERROR_NOT_INITIALIZED;
	}
	if ( g_xge.objDesc.iRunMode != XGE_RUN_MANUAL ) {
		return XGE_ERROR_UNSUPPORTED;
	}
	if ( g_xge.bRunning == 0 ) {
		return 0;
	}
	g_xge.iFrameCount++;
	g_xge.tFrameStats.iFrameCount++;
	g_xge.fDelta = 1.0f / (float)g_xge.objDesc.iTargetFPS;
	g_xge.iFPS = g_xge.objDesc.iTargetFPS;
	if ( g_xge.procFrame != NULL ) {
		iRet = g_xge.procFrame(g_xge.pFrameUser);
		if ( iRet != 0 ) {
			xgeQuit();
			return iRet;
		}
	} else {
		iRet = __xgeSceneFrame();
		if ( iRet != 0 ) {
			xgeQuit();
			return iRet;
		}
	}
	__xgeInputBeginFrame();
	return 1;
}

void xgeFrameStatsReset(void)
{
	memset(&g_xge.tFrameStats, 0, sizeof(g_xge.tFrameStats));
}

xge_frame_stats_t xgeFrameStatsGet(void)
{
	xge_frame_stats_t tStats;

	memset(&tStats, 0, sizeof(tStats));
	if ( g_xge.bInitialized == 0 ) {
		return tStats;
	}
	tStats = g_xge.tFrameStats;
	tStats.iDirtyRectCount = g_xge.iDirtyRectCount;
	return tStats;
}

static void __xgeFrameStatsAddDrawCall(void)
{
	g_xge.tFrameStats.iDrawCallCount++;
	g_xge.tFrameStats.iBatchCount++;
}

static int __xgeSceneCall(xge_scene pScene, xge_scene_lifecycle_proc proc)
{
	if ( (pScene == NULL) || (proc == NULL) ) {
		return XGE_OK;
	}
	return proc(pScene);
}

static int __xgeSceneEnter(xge_scene pScene)
{
	int iRet;

	if ( pScene == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( pScene->bEntered ) {
		pScene->bPaused = 0;
		return XGE_OK;
	}
	iRet = __xgeSceneCall(pScene, pScene->onEnter);
	if ( iRet != 0 ) {
		return iRet;
	}
	pScene->bEntered = 1;
	pScene->bPaused = 0;
	return XGE_OK;
}

static int __xgeSceneLeave(xge_scene pScene)
{
	int iRet;

	if ( pScene == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( pScene->bEntered == 0 ) {
		pScene->bPaused = 0;
		return XGE_OK;
	}
	iRet = __xgeSceneCall(pScene, pScene->onLeave);
	if ( iRet != 0 ) {
		return iRet;
	}
	pScene->bEntered = 0;
	pScene->bPaused = 0;
	return XGE_OK;
}

static int __xgeScenePause(xge_scene pScene)
{
	int iRet;

	if ( (pScene == NULL) || (pScene->bPaused != 0) ) {
		return XGE_OK;
	}
	iRet = __xgeSceneCall(pScene, pScene->onPause);
	if ( iRet != 0 ) {
		return iRet;
	}
	pScene->bPaused = 1;
	return XGE_OK;
}

static int __xgeSceneResume(xge_scene pScene)
{
	int iRet;

	if ( (pScene == NULL) || (pScene->bPaused == 0) ) {
		return XGE_OK;
	}
	iRet = __xgeSceneCall(pScene, pScene->onResume);
	if ( iRet != 0 ) {
		return iRet;
	}
	pScene->bPaused = 0;
	return XGE_OK;
}

static int __xgeSceneFree(xge_scene pScene)
{
	return __xgeSceneCall(pScene, pScene ? pScene->onFree : NULL);
}

static void __xgeSceneClear(void)
{
	xge_scene pScene;

	while ( g_xge.iSceneStackCount > 0 ) {
		pScene = g_xge.arrSceneStack[g_xge.iSceneStackCount - 1];
		g_xge.arrSceneStack[g_xge.iSceneStackCount - 1] = NULL;
		g_xge.iSceneStackCount--;
		(void)__xgeSceneLeave(pScene);
		(void)__xgeSceneFree(pScene);
	}
}

static int __xgeSceneFrame(void)
{
	xge_scene pScene;
	int iRet;
	int iUpdateCount;
	float fOldDelta;

	pScene = xgeSceneCurrent();
	if ( pScene == NULL ) {
		return XGE_OK;
	}
	if ( pScene->onUpdate != NULL ) {
		if ( g_xge.iSceneUpdateMode == XGE_UPDATE_FIXED ) {
			fOldDelta = g_xge.fDelta;
			g_xge.fSceneAccumulator += fOldDelta;
			iUpdateCount = 0;
			while ( (g_xge.fSceneAccumulator >= g_xge.fSceneFixedStep) && (iUpdateCount < g_xge.iSceneMaxUpdates) ) {
				g_xge.fDelta = g_xge.fSceneFixedStep;
				iRet = pScene->onUpdate(pScene, g_xge.fSceneFixedStep);
				if ( iRet != 0 ) {
					g_xge.fDelta = fOldDelta;
					return iRet;
				}
				g_xge.fSceneAccumulator -= g_xge.fSceneFixedStep;
				iUpdateCount++;
			}
			if ( iUpdateCount >= g_xge.iSceneMaxUpdates ) {
				g_xge.fSceneAccumulator = 0.0f;
			}
			g_xge.fDelta = fOldDelta;
		} else {
			iRet = pScene->onUpdate(pScene, g_xge.fDelta);
			if ( iRet != 0 ) {
				return iRet;
			}
		}
	}
	if ( pScene->onDraw != NULL ) {
		iRet = pScene->onDraw(pScene);
		if ( iRet != 0 ) {
			return iRet;
		}
	}
	return XGE_OK;
}

int xgeSceneSet(xge_scene pScene)
{
	if ( g_xge.bInitialized == 0 ) {
		return XGE_ERROR_NOT_INITIALIZED;
	}
	__xgeSceneClear();
	if ( pScene == NULL ) {
		return XGE_OK;
	}
	g_xge.fSceneAccumulator = 0.0f;
	g_xge.arrSceneStack[0] = pScene;
	g_xge.iSceneStackCount = 1;
	return __xgeSceneEnter(pScene);
}

int xgeScenePush(xge_scene pScene)
{
	xge_scene pCurrent;
	int iRet;

	if ( g_xge.bInitialized == 0 ) {
		return XGE_ERROR_NOT_INITIALIZED;
	}
	if ( pScene == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( g_xge.iSceneStackCount >= XGE_SCENE_STACK_MAX ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pCurrent = xgeSceneCurrent();
	iRet = __xgeScenePause(pCurrent);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	g_xge.fSceneAccumulator = 0.0f;
	g_xge.arrSceneStack[g_xge.iSceneStackCount++] = pScene;
	iRet = __xgeSceneEnter(pScene);
	if ( iRet != XGE_OK ) {
		g_xge.iSceneStackCount--;
		g_xge.arrSceneStack[g_xge.iSceneStackCount] = NULL;
		(void)__xgeSceneResume(pCurrent);
		return iRet;
	}
	return XGE_OK;
}

int xgeScenePop(void)
{
	xge_scene pScene;
	int iRet;

	if ( g_xge.bInitialized == 0 ) {
		return XGE_ERROR_NOT_INITIALIZED;
	}
	if ( g_xge.iSceneStackCount <= 0 ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pScene = g_xge.arrSceneStack[g_xge.iSceneStackCount - 1];
	g_xge.arrSceneStack[g_xge.iSceneStackCount - 1] = NULL;
	g_xge.iSceneStackCount--;
	iRet = __xgeSceneLeave(pScene);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	iRet = __xgeSceneFree(pScene);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	g_xge.fSceneAccumulator = 0.0f;
	return __xgeSceneResume(xgeSceneCurrent());
}

int xgeSceneReplace(xge_scene pScene)
{
	xge_scene pOld;
	int iRet;

	if ( g_xge.bInitialized == 0 ) {
		return XGE_ERROR_NOT_INITIALIZED;
	}
	if ( pScene == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( g_xge.iSceneStackCount <= 0 ) {
		return xgeScenePush(pScene);
	}
	pOld = g_xge.arrSceneStack[g_xge.iSceneStackCount - 1];
	iRet = __xgeSceneLeave(pOld);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	iRet = __xgeSceneFree(pOld);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	g_xge.arrSceneStack[g_xge.iSceneStackCount - 1] = pScene;
	g_xge.fSceneAccumulator = 0.0f;
	return __xgeSceneEnter(pScene);
}

xge_scene xgeSceneCurrent(void)
{
	if ( (g_xge.bInitialized == 0) || (g_xge.iSceneStackCount <= 0) ) {
		return NULL;
	}
	return g_xge.arrSceneStack[g_xge.iSceneStackCount - 1];
}

int xgeSceneCount(void)
{
	if ( g_xge.bInitialized == 0 ) {
		return 0;
	}
	return g_xge.iSceneStackCount;
}

int xgeSceneDispatchEvent(const xge_event_t* pEvent)
{
	xge_scene pScene;

	if ( g_xge.bInitialized == 0 ) {
		return XGE_ERROR_NOT_INITIALIZED;
	}
	if ( pEvent == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pScene = xgeSceneCurrent();
	if ( (pScene == NULL) || (pScene->onEvent == NULL) ) {
		return XGE_OK;
	}
	return pScene->onEvent(pScene, pEvent);
}

int xgeSceneUpdateStrategySet(int iMode, float fFixedStep, int iMaxUpdates)
{
	if ( g_xge.bInitialized == 0 ) {
		return XGE_ERROR_NOT_INITIALIZED;
	}
	if ( (iMode != XGE_UPDATE_VARIABLE) && (iMode != XGE_UPDATE_FIXED) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( fFixedStep <= 0.0f ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( iMaxUpdates <= 0 ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	g_xge.iSceneUpdateMode = iMode;
	g_xge.fSceneFixedStep = fFixedStep;
	g_xge.iSceneMaxUpdates = iMaxUpdates;
	g_xge.fSceneAccumulator = 0.0f;
	return XGE_OK;
}

void xgeSceneUpdateStrategyGet(int* pMode, float* pFixedStep, int* pMaxUpdates)
{
	if ( pMode != NULL ) {
		*pMode = g_xge.iSceneUpdateMode;
	}
	if ( pFixedStep != NULL ) {
		*pFixedStep = g_xge.fSceneFixedStep;
	}
	if ( pMaxUpdates != NULL ) {
		*pMaxUpdates = g_xge.iSceneMaxUpdates;
	}
}

int xgeRender(void)
{
	if ( g_xge.bInitialized == 0 ) {
		return XGE_ERROR_NOT_INITIALIZED;
	}
	if ( g_xge.objDesc.iRunMode != XGE_RUN_MANUAL ) {
		return XGE_ERROR_UNSUPPORTED;
	}
	if ( g_xge.bRunning == 0 ) {
		return 0;
	}
	return 1;
}

static xge_rect_t __xgeRectNormalize(xge_rect_t tRect)
{
	if ( tRect.fW < 0.0f ) {
		tRect.fX += tRect.fW;
		tRect.fW = -tRect.fW;
	}
	if ( tRect.fH < 0.0f ) {
		tRect.fY += tRect.fH;
		tRect.fH = -tRect.fH;
	}
	return tRect;
}

static xge_rect_t __xgeRectClampToWindow(xge_rect_t tRect)
{
	float fRight;
	float fBottom;
	float fMaxW;
	float fMaxH;

	tRect = __xgeRectNormalize(tRect);
	fMaxW = (float)g_xge.iWidth;
	fMaxH = (float)g_xge.iHeight;
	fRight = tRect.fX + tRect.fW;
	fBottom = tRect.fY + tRect.fH;
	if ( tRect.fX < 0.0f ) {
		tRect.fX = 0.0f;
	}
	if ( tRect.fY < 0.0f ) {
		tRect.fY = 0.0f;
	}
	if ( fRight > fMaxW ) {
		fRight = fMaxW;
	}
	if ( fBottom > fMaxH ) {
		fBottom = fMaxH;
	}
	tRect.fW = fRight - tRect.fX;
	tRect.fH = fBottom - tRect.fY;
	if ( tRect.fW < 0.0f ) {
		tRect.fW = 0.0f;
	}
	if ( tRect.fH < 0.0f ) {
		tRect.fH = 0.0f;
	}
	return tRect;
}

static xge_rect_t __xgeRectUnion(xge_rect_t tA, xge_rect_t tB)
{
	float fLeft;
	float fTop;
	float fRight;
	float fBottom;
	xge_rect_t tRect;

	tA = __xgeRectNormalize(tA);
	tB = __xgeRectNormalize(tB);
	fLeft = (tA.fX < tB.fX) ? tA.fX : tB.fX;
	fTop = (tA.fY < tB.fY) ? tA.fY : tB.fY;
	fRight = ((tA.fX + tA.fW) > (tB.fX + tB.fW)) ? (tA.fX + tA.fW) : (tB.fX + tB.fW);
	fBottom = ((tA.fY + tA.fH) > (tB.fY + tB.fH)) ? (tA.fY + tA.fH) : (tB.fY + tB.fH);
	tRect.fX = fLeft;
	tRect.fY = fTop;
	tRect.fW = fRight - fLeft;
	tRect.fH = fBottom - fTop;
	return tRect;
}

static xge_rect_t __xgeDirtyRectUnionAll(void)
{
	xge_rect_t tRect;
	int i;

	memset(&tRect, 0, sizeof(tRect));
	if ( g_xge.iDirtyRectCount <= 0 ) {
		return tRect;
	}
	tRect = g_xge.arrDirtyRects[0];
	for ( i = 1; i < g_xge.iDirtyRectCount; i++ ) {
		tRect = __xgeRectUnion(tRect, g_xge.arrDirtyRects[i]);
	}
	return tRect;
}

int xgeGetWidth(void)
{
	return g_xge.iWidth;
}

int xgeGetHeight(void)
{
	return g_xge.iHeight;
}

float xgeGetDelta(void)
{
	return g_xge.fDelta;
}

int xgeGetFPS(void)
{
	return g_xge.iFPS;
}

double xgeTimer(void)
{
	if ( g_xge.bInitialized == 0 ) {
		return 0.0;
	}
	return xrtTimer() - g_xge.fStartTime;
}

int64_t xgeTimeNow(void)
{
	return (int64_t)xrtNow();
}

void xgeSleep(uint32_t iMilliseconds)
{
	xrtSleep(iMilliseconds);
}

void xgeSetTitle(const char* sTitle)
{
	if ( sTitle != NULL ) {
		sapp_set_window_title(sTitle);
	}
}

void xgeClear(uint32_t iColor)
{
	float fR;
	float fG;
	float fB;
	float fA;
	xge_rect_t tRect;

	g_xge.iClearColor = iColor;
	tRect.fX = 0.0f;
	tRect.fY = 0.0f;
	tRect.fW = (float)g_xge.iWidth;
	tRect.fH = (float)g_xge.iHeight;
	xgeInvalidateRect(tRect);
	if ( g_xge.bSokolRunning ) {
		__xgeColorToFloat(iColor, &fR, &fG, &fB, &fA);
		glClearColor(fR, fG, fB, fA);
		glClear(GL_COLOR_BUFFER_BIT);
	}
}

void xgePresent(void)
{
	xgeDirtyRectClear();
}

void xgeInvalidateRect(xge_rect_t tRect)
{
	if ( g_xge.bInitialized == 0 ) {
		return;
	}
	tRect = __xgeRectClampToWindow(tRect);
	if ( (tRect.fW <= 0.0f) || (tRect.fH <= 0.0f) ) {
		return;
	}
	if ( g_xge.iDirtyRectCount >= XGE_DIRTY_RECT_MAX ) {
		g_xge.arrDirtyRects[0] = __xgeRectUnion(__xgeDirtyRectUnionAll(), tRect);
		g_xge.iDirtyRectCount = 1;
		return;
	}
	g_xge.arrDirtyRects[g_xge.iDirtyRectCount++] = tRect;
}

int xgeDirtyRectCount(void)
{
	if ( g_xge.bInitialized == 0 ) {
		return 0;
	}
	return g_xge.iDirtyRectCount;
}

int xgeDirtyRectGet(int iIndex, xge_rect_t* pRect)
{
	if ( (g_xge.bInitialized == 0) || (pRect == NULL) || (iIndex < 0) || (iIndex >= g_xge.iDirtyRectCount) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	*pRect = g_xge.arrDirtyRects[iIndex];
	return XGE_OK;
}

void xgeDirtyRectClear(void)
{
	g_xge.iDirtyRectCount = 0;
	memset(g_xge.arrDirtyRects, 0, sizeof(g_xge.arrDirtyRects));
}

uint32_t xgeColorRGBA(int iR, int iG, int iB, int iA)
{
	return XGE_COLOR_RGBA(iR, iG, iB, iA);
}

xge_color_t xgeColorUnpack(uint32_t iColor)
{
	xge_color_t tColor;

	tColor.fR = (float)XGE_COLOR_GET_R(iColor) / 255.0f;
	tColor.fG = (float)XGE_COLOR_GET_G(iColor) / 255.0f;
	tColor.fB = (float)XGE_COLOR_GET_B(iColor) / 255.0f;
	tColor.fA = (float)XGE_COLOR_GET_A(iColor) / 255.0f;
	return tColor;
}

void xgeBlendSet(int iBlend)
{
	g_xge.iBlend = iBlend;
	__xgeBlendApply(iBlend);
}

int xgeBlendGet(void)
{
	return g_xge.iBlend;
}

xge_camera_t xgeCameraDefault(float fWidth, float fHeight)
{
	xge_camera_t tCamera;

	memset(&tCamera, 0, sizeof(tCamera));
	tCamera.tScale.fX = 1.0f;
	tCamera.tScale.fY = 1.0f;
	tCamera.tViewport.fX = 0.0f;
	tCamera.tViewport.fY = 0.0f;
	tCamera.tViewport.fW = fWidth;
	tCamera.tViewport.fH = fHeight;
	tCamera.iCoordinateMode = XGE_COORD_Y_DOWN;
	return tCamera;
}

void xgeCameraSet(const xge_camera_t* pCamera)
{
	if ( pCamera == NULL ) {
		return;
	}
	g_xge.tCamera = *pCamera;
	if ( g_xge.tCamera.tScale.fX == 0.0f ) {
		g_xge.tCamera.tScale.fX = 1.0f;
	}
	if ( g_xge.tCamera.tScale.fY == 0.0f ) {
		g_xge.tCamera.tScale.fY = 1.0f;
	}
}

xge_camera_t xgeCameraGet(void)
{
	return g_xge.tCamera;
}

xge_vec2_t xgeWorldToScreen(xge_vec2_t tPoint)
{
	xge_vec2_t tRet;
	xge_camera_t* pCamera;

	pCamera = &g_xge.tCamera;
	tRet.fX = (tPoint.fX - pCamera->tPosition.fX) * pCamera->tScale.fX;
	tRet.fY = (tPoint.fY - pCamera->tPosition.fY) * pCamera->tScale.fY;
	if ( pCamera->iCoordinateMode == XGE_COORD_CENTER ) {
		tRet.fX += pCamera->tViewport.fX + (pCamera->tViewport.fW * 0.5f);
		tRet.fY += pCamera->tViewport.fY + (pCamera->tViewport.fH * 0.5f);
	} else {
		tRet.fX += pCamera->tViewport.fX;
		tRet.fY += pCamera->tViewport.fY;
	}
	return tRet;
}

xge_vec2_t xgeScreenToWorld(xge_vec2_t tPoint)
{
	xge_vec2_t tRet;
	xge_camera_t* pCamera;
	float fX;
	float fY;

	pCamera = &g_xge.tCamera;
	fX = tPoint.fX - pCamera->tViewport.fX;
	fY = tPoint.fY - pCamera->tViewport.fY;
	if ( pCamera->iCoordinateMode == XGE_COORD_CENTER ) {
		fX -= pCamera->tViewport.fW * 0.5f;
		fY -= pCamera->tViewport.fH * 0.5f;
	}
	tRet.fX = (fX / pCamera->tScale.fX) + pCamera->tPosition.fX;
	tRet.fY = (fY / pCamera->tScale.fY) + pCamera->tPosition.fY;
	return tRet;
}

