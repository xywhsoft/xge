#include "../xge.h"
#include <stdio.h>
#include <string.h>

static int g_iProviderFreeCount;
static unsigned char g_arrProviderData[] = { 'x', 'g', 'e', 0 };

typedef struct xrf_test_blob2_t {
	xge_xrf_header_t tHeader;
	xge_xrf_range_t tRange;
	xge_xrf_glyph_t arrGlyphs[2];
	xge_xrf_page_t tPage;
	unsigned char arrPixels[16 * 16];
} xrf_test_blob2_t;

static int __testProviderLoad(const char* sURI, void** ppData, int* pSize, void* pUser)
{
	(void)sURI;
	(void)pUser;
	*ppData = g_arrProviderData;
	*pSize = 3;
	return XGE_OK;
}

static void __testProviderFree(void* pData, void* pUser)
{
	(void)pData;
	(void)pUser;
	g_iProviderFreeCount++;
}

typedef struct xui_host_test_t {
	int iDrawRect;
	int iDrawImage;
	int iDrawText;
	int iMeasureText;
	int iClipSet;
	int iClipClear;
	int iRequestRefresh;
} xui_host_test_t;

static void __testXuiHostDrawRect(xge_rect_t tRect, uint32_t iColor, void* pUser)
{
	xui_host_test_t* pHost;

	(void)tRect;
	(void)iColor;
	pHost = (xui_host_test_t*)pUser;
	pHost->iDrawRect++;
}

static void __testXuiHostDrawImage(const xge_draw_t* pDraw, void* pUser)
{
	xui_host_test_t* pHost;

	(void)pDraw;
	pHost = (xui_host_test_t*)pUser;
	pHost->iDrawImage++;
}

static void __testXuiHostDrawTextRect(xge_font pFont, const char* sText, xge_rect_t tRect, uint32_t iColor, uint32_t iFlags, void* pUser)
{
	xui_host_test_t* pHost;

	(void)pFont;
	(void)sText;
	(void)tRect;
	(void)iColor;
	(void)iFlags;
	pHost = (xui_host_test_t*)pUser;
	pHost->iDrawText++;
}

static xge_vec2_t __testXuiHostMeasureText(xge_font pFont, const char* sText, void* pUser)
{
	xui_host_test_t* pHost;
	xge_vec2_t tSize;

	(void)pFont;
	(void)sText;
	pHost = (xui_host_test_t*)pUser;
	pHost->iMeasureText++;
	tSize.fX = 10.0f;
	tSize.fY = 12.0f;
	return tSize;
}

static void __testXuiHostClipSet(xge_rect_t tRect, void* pUser)
{
	xui_host_test_t* pHost;

	(void)tRect;
	pHost = (xui_host_test_t*)pUser;
	pHost->iClipSet++;
}

static void __testXuiHostClipClear(void* pUser)
{
	xui_host_test_t* pHost;

	pHost = (xui_host_test_t*)pUser;
	pHost->iClipClear++;
}

static void __testXuiHostRequestRefresh(void* pUser)
{
	xui_host_test_t* pHost;

	pHost = (xui_host_test_t*)pUser;
	pHost->iRequestRefresh++;
}

static void __testXuiCustomPaint(xge_xui_widget pWidget, void* pUser)
{
	int* pCount;

	(void)pWidget;
	pCount = (int*)pUser;
	(*pCount)++;
}

typedef struct xge_frame_test_t {
	int iFrameCount;
	int iQuitAt;
} xge_frame_test_t;

typedef struct xge_scene_test_state_t {
	int iEnter;
	int iLeave;
	int iPause;
	int iResume;
	int iEvent;
	int iUpdate;
	int iDraw;
	int iFree;
	int iLastEvent;
	float fLastDelta;
} xge_scene_test_state_t;

static int __testManualFrameProc(void* pUser)
{
	xge_frame_test_t* pState;

	pState = (xge_frame_test_t*)pUser;
	pState->iFrameCount++;
	if ( (pState->iQuitAt > 0) && (pState->iFrameCount >= pState->iQuitAt) ) {
		return 99;
	}
	return 0;
}

static int __testSceneEnter(xge_scene pScene)
{
	xge_scene_test_state_t* pState;

	pState = (xge_scene_test_state_t*)pScene->pUser;
	pState->iEnter++;
	return 0;
}

static int __testSceneLeave(xge_scene pScene)
{
	xge_scene_test_state_t* pState;

	pState = (xge_scene_test_state_t*)pScene->pUser;
	pState->iLeave++;
	return 0;
}

static int __testScenePause(xge_scene pScene)
{
	xge_scene_test_state_t* pState;

	pState = (xge_scene_test_state_t*)pScene->pUser;
	pState->iPause++;
	return 0;
}

static int __testSceneResume(xge_scene pScene)
{
	xge_scene_test_state_t* pState;

	pState = (xge_scene_test_state_t*)pScene->pUser;
	pState->iResume++;
	return 0;
}

static int __testSceneEvent(xge_scene pScene, const xge_event_t* pEvent)
{
	xge_scene_test_state_t* pState;

	pState = (xge_scene_test_state_t*)pScene->pUser;
	pState->iEvent++;
	pState->iLastEvent = pEvent->iType;
	return 0;
}

static int __testSceneUpdate(xge_scene pScene, float fDelta)
{
	xge_scene_test_state_t* pState;

	pState = (xge_scene_test_state_t*)pScene->pUser;
	pState->iUpdate++;
	pState->fLastDelta = fDelta;
	return 0;
}

static int __testSceneDraw(xge_scene pScene)
{
	xge_scene_test_state_t* pState;

	pState = (xge_scene_test_state_t*)pScene->pUser;
	pState->iDraw++;
	return 0;
}

static int __testSceneFree(xge_scene pScene)
{
	xge_scene_test_state_t* pState;

	pState = (xge_scene_test_state_t*)pScene->pUser;
	pState->iFree++;
	return 0;
}

static void __testSceneMake(xge_scene pScene, xge_scene_test_state_t* pState)
{
	memset(pScene, 0, sizeof(*pScene));
	pScene->pUser = pState;
	pScene->onEnter = __testSceneEnter;
	pScene->onLeave = __testSceneLeave;
	pScene->onPause = __testScenePause;
	pScene->onResume = __testSceneResume;
	pScene->onEvent = __testSceneEvent;
	pScene->onUpdate = __testSceneUpdate;
	pScene->onDraw = __testSceneDraw;
	pScene->onFree = __testSceneFree;
}

static int __testCoreLifecycle(void)
{
	xge_desc_t objDesc;

	if ( xgeGetWidth() != 0 ) {
		return 1;
	}

	objDesc.iWidth = 320;
	objDesc.iHeight = 200;
	objDesc.sTitle = "XGE Test";
	objDesc.iFlags = 0;
	objDesc.iRunMode = XGE_RUN_MANUAL;
	objDesc.iTargetFPS = 0;
	objDesc.pNativeWindow = 0;
	objDesc.pUser = 0;

	if ( xgeInit(&objDesc) != XGE_OK ) {
		return 2;
	}
	if ( xgeGetWidth() != 320 || xgeGetHeight() != 200 ) {
		return 3;
	}
	if ( xgeInit(0) != XGE_ERROR_ALREADY_INITIALIZED ) {
		return 4;
	}
	return 0;
}

static int __testSceneSystem(void)
{
	xge_scene_t tSceneA;
	xge_scene_t tSceneB;
	xge_scene_test_state_t tStateA;
	xge_scene_test_state_t tStateB;
	xge_event_t tEvent;
	int iMode;
	int iMaxUpdates;
	float fFixedStep;

	memset(&tStateA, 0, sizeof(tStateA));
	memset(&tStateB, 0, sizeof(tStateB));
	memset(&tEvent, 0, sizeof(tEvent));
	__testSceneMake(&tSceneA, &tStateA);
	__testSceneMake(&tSceneB, &tStateB);

	if ( xgeSceneSet(&tSceneA) != XGE_OK || xgeSceneCurrent() != &tSceneA || xgeSceneCount() != 1 || tStateA.iEnter != 1 ) {
		return 400;
	}
	tEvent.iType = XGE_EVENT_MOUSE_DOWN;
	if ( xgeSceneDispatchEvent(&tEvent) != XGE_OK || tStateA.iEvent != 1 || tStateA.iLastEvent != XGE_EVENT_MOUSE_DOWN ) {
		return 401;
	}
	if ( xgeRun(NULL, NULL) != XGE_OK || xgeFrame() != 1 || tStateA.iUpdate != 1 || tStateA.iDraw != 1 || tStateA.fLastDelta <= 0.0f ) {
		return 402;
	}
	xgeSceneUpdateStrategyGet(&iMode, &fFixedStep, &iMaxUpdates);
	if ( iMode != XGE_UPDATE_VARIABLE || fFixedStep <= 0.0f || iMaxUpdates != 1 ) {
		return 409;
	}
	if ( xgeSceneUpdateStrategySet(XGE_UPDATE_FIXED, 1.0f / 120.0f, 4) != XGE_OK ) {
		return 410;
	}
	if ( xgeFrame() != 1 || tStateA.iUpdate != 3 || tStateA.iDraw != 2 || tStateA.fLastDelta < 0.008f || tStateA.fLastDelta > 0.009f ) {
		return 411;
	}
	if ( xgeSceneUpdateStrategySet(XGE_UPDATE_FIXED, 1.0f / 240.0f, 3) != XGE_OK ) {
		return 412;
	}
	if ( xgeFrame() != 1 || tStateA.iUpdate != 6 || tStateA.iDraw != 3 ) {
		return 413;
	}
	xgeSceneUpdateStrategyGet(&iMode, &fFixedStep, &iMaxUpdates);
	if ( iMode != XGE_UPDATE_FIXED || fFixedStep < 0.004f || fFixedStep > 0.005f || iMaxUpdates != 3 ) {
		return 414;
	}
	if ( xgeSceneUpdateStrategySet(XGE_UPDATE_VARIABLE, 1.0f / 60.0f, 1) != XGE_OK ) {
		return 415;
	}
	if ( xgeFrame() != 1 || tStateA.iUpdate != 7 || tStateA.iDraw != 4 ) {
		return 416;
	}
	if ( xgeSceneUpdateStrategySet(99, 1.0f / 60.0f, 1) != XGE_ERROR_INVALID_ARGUMENT || xgeSceneUpdateStrategySet(XGE_UPDATE_FIXED, 0.0f, 1) != XGE_ERROR_INVALID_ARGUMENT || xgeSceneUpdateStrategySet(XGE_UPDATE_FIXED, 1.0f / 60.0f, 0) != XGE_ERROR_INVALID_ARGUMENT ) {
		return 417;
	}
	if ( xgeScenePush(&tSceneB) != XGE_OK || xgeSceneCurrent() != &tSceneB || xgeSceneCount() != 2 || tStateA.iPause != 1 || tStateB.iEnter != 1 ) {
		return 403;
	}
	if ( xgeFrame() != 1 || tStateB.iUpdate != 1 || tStateB.iDraw != 1 || tStateA.iUpdate != 7 ) {
		return 404;
	}
	if ( xgeScenePop() != XGE_OK || xgeSceneCurrent() != &tSceneA || xgeSceneCount() != 1 || tStateB.iLeave != 1 || tStateB.iFree != 1 || tStateA.iResume != 1 ) {
		return 405;
	}
	if ( xgeSceneReplace(&tSceneB) != XGE_OK || xgeSceneCurrent() != &tSceneB || xgeSceneCount() != 1 || tStateA.iLeave != 1 || tStateA.iFree != 1 || tStateB.iEnter != 2 ) {
		return 406;
	}
	if ( xgeSceneSet(NULL) != XGE_OK || xgeSceneCurrent() != NULL || xgeSceneCount() != 0 || tStateB.iLeave != 2 || tStateB.iFree != 2 ) {
		return 407;
	}
	if ( xgeScenePop() != XGE_ERROR_INVALID_ARGUMENT || xgeScenePush(NULL) != XGE_ERROR_INVALID_ARGUMENT || xgeSceneDispatchEvent(NULL) != XGE_ERROR_INVALID_ARGUMENT ) {
		return 408;
	}
	return 0;
}

static int __testRunModes(void)
{
	xge_frame_test_t tState;
	xge_frame_stats_t tStats;

	memset(&tState, 0, sizeof(tState));
	if ( xgeRun(__testManualFrameProc, &tState) != XGE_OK ) {
		return 13;
	}
	if ( tState.iFrameCount != 0 ) {
		return 14;
	}
	xgeFrameStatsReset();
	tStats = xgeFrameStatsGet();
	if ( tStats.iFrameCount != 0 || tStats.iDrawCallCount != 0 || tStats.iBatchCount != 0 || tStats.iDirtyRectCount != 0 ) {
		return 20;
	}
	if ( xgeFrame() != 1 || tState.iFrameCount != 1 ) {
		return 15;
	}
	tStats = xgeFrameStatsGet();
	if ( tStats.iFrameCount != 1 || tStats.iDrawCallCount != 0 || tStats.iBatchCount != 0 ) {
		return 21;
	}
	if ( xgeGetFPS() != 60 || xgeGetDelta() < 0.016f || xgeGetDelta() > 0.017f ) {
		return 16;
	}
	if ( xgeRender() != 1 ) {
		return 17;
	}
	xgeInvalidateRect((xge_rect_t){ 0.0f, 0.0f, 8.0f, 8.0f });
	tStats = xgeFrameStatsGet();
	if ( tStats.iDirtyRectCount != 1 ) {
		return 22;
	}
	xgeDirtyRectClear();
	tState.iQuitAt = 2;
	if ( xgeFrame() != 99 || tState.iFrameCount != 2 ) {
		return 18;
	}
	if ( xgeFrame() != 0 || xgeRender() != 0 ) {
		return 19;
	}
	return 0;
}

static int __testXrtTimeIntegration(void)
{
	double fStart;
	double fEnd;
	int64_t iNow;

	fStart = xgeTimer();
	iNow = xgeTimeNow();
	xgeSleep(1);
	fEnd = xgeTimer();
	if ( iNow <= 0 ) {
		return 430;
	}
	if ( fStart < 0.0 || fEnd < fStart ) {
		return 431;
	}
	return 0;
}

static int __testDirtyRects(void)
{
	xge_rect_t tRect;
	xge_rect_t tOut;
	int i;

	xgeDirtyRectClear();
	if ( xgeDirtyRectCount() != 0 ) {
		return 5;
	}
	tRect.fX = 10.0f;
	tRect.fY = 20.0f;
	tRect.fW = 30.0f;
	tRect.fH = 40.0f;
	xgeInvalidateRect(tRect);
	if ( xgeDirtyRectCount() != 1 || xgeDirtyRectGet(0, &tOut) != XGE_OK || tOut.fX != 10.0f || tOut.fY != 20.0f || tOut.fW != 30.0f || tOut.fH != 40.0f ) {
		return 6;
	}
	tRect.fX = -5.0f;
	tRect.fY = -6.0f;
	tRect.fW = 20.0f;
	tRect.fH = 30.0f;
	xgeInvalidateRect(tRect);
	if ( xgeDirtyRectCount() != 2 || xgeDirtyRectGet(1, &tOut) != XGE_OK || tOut.fX != 0.0f || tOut.fY != 0.0f || tOut.fW != 15.0f || tOut.fH != 24.0f ) {
		return 7;
	}
	tRect.fX = 10.0f;
	tRect.fY = 10.0f;
	tRect.fW = -5.0f;
	tRect.fH = -5.0f;
	xgeInvalidateRect(tRect);
	if ( xgeDirtyRectCount() != 3 || xgeDirtyRectGet(2, &tOut) != XGE_OK || tOut.fX != 5.0f || tOut.fY != 5.0f || tOut.fW != 5.0f || tOut.fH != 5.0f ) {
		return 8;
	}
	tRect.fX = 0.0f;
	tRect.fY = 0.0f;
	tRect.fW = 0.0f;
	tRect.fH = 10.0f;
	xgeInvalidateRect(tRect);
	if ( xgeDirtyRectCount() != 3 || xgeDirtyRectGet(3, &tOut) != XGE_ERROR_INVALID_ARGUMENT || xgeDirtyRectGet(0, NULL) != XGE_ERROR_INVALID_ARGUMENT ) {
		return 9;
	}
	xgePresent();
	if ( xgeDirtyRectCount() != 0 ) {
		return 10;
	}
	xgeClear(XGE_COLOR_RGBA(1, 2, 3, 255));
	if ( xgeDirtyRectCount() != 1 || xgeDirtyRectGet(0, &tOut) != XGE_OK || tOut.fX != 0.0f || tOut.fY != 0.0f || tOut.fW != 320.0f || tOut.fH != 200.0f ) {
		return 11;
	}
	xgeDirtyRectClear();
	for ( i = 0; i < XGE_DIRTY_RECT_MAX + 1; i++ ) {
		tRect.fX = (float)i;
		tRect.fY = (float)i;
		tRect.fW = 1.0f;
		tRect.fH = 1.0f;
		xgeInvalidateRect(tRect);
	}
	if ( xgeDirtyRectCount() != 1 || xgeDirtyRectGet(0, &tOut) != XGE_OK || tOut.fX != 0.0f || tOut.fY != 0.0f || tOut.fW != (float)(XGE_DIRTY_RECT_MAX + 1) || tOut.fH != (float)(XGE_DIRTY_RECT_MAX + 1) ) {
		return 12;
	}
	xgeDirtyRectClear();
	return 0;
}

static int __testImageMemory(void)
{
	static const unsigned char arrTga[] = {
		0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		1, 0, 1, 0, 32, 8, 0, 0, 255, 255
	};
	static const unsigned char arrHalfAlphaTga[] = {
		0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		1, 0, 1, 0, 32, 8, 0, 0, 255, 128
	};
	xge_image_t objImage;
	unsigned char* pPixels;

	if ( xgeImageLoadMemory(&objImage, arrTga, (int)sizeof(arrTga)) != XGE_OK ) {
		return 10;
	}
	pPixels = (unsigned char*)xgeImageGetPixels(&objImage);
	if ( objImage.iWidth != 1 || objImage.iHeight != 1 || pPixels == 0 ) {
		xgeImageFree(&objImage);
		return 11;
	}
	if ( pPixels[0] != 255 || pPixels[1] != 0 || pPixels[2] != 0 || pPixels[3] != 255 ) {
		xgeImageFree(&objImage);
		return 12;
	}
	xgeImageFree(&objImage);
	if ( objImage.pPixels != 0 ) {
		return 13;
	}

	if ( xgeImageLoadMemory(&objImage, arrHalfAlphaTga, (int)sizeof(arrHalfAlphaTga)) != XGE_OK ) {
		return 14;
	}
	pPixels = (unsigned char*)xgeImageGetPixels(&objImage);
	if ( (objImage.iFlags & XGE_IMAGE_PREMULTIPLIED) == 0 || pPixels[0] != 128 || pPixels[3] != 128 ) {
		xgeImageFree(&objImage);
		return 15;
	}
	xgeImageFree(&objImage);

	if ( xgeImageLoadMemoryEx(&objImage, arrHalfAlphaTga, (int)sizeof(arrHalfAlphaTga), XGE_IMAGE_STRAIGHT_ALPHA) != XGE_OK ) {
		return 16;
	}
	pPixels = (unsigned char*)xgeImageGetPixels(&objImage);
	if ( (objImage.iFlags & XGE_IMAGE_STRAIGHT_ALPHA) == 0 || pPixels[0] != 255 || pPixels[3] != 128 ) {
		xgeImageFree(&objImage);
		return 17;
	}
	xgeImagePremultiply(&objImage);
	if ( (objImage.iFlags & XGE_IMAGE_PREMULTIPLIED) == 0 || pPixels[0] != 128 || pPixels[3] != 128 ) {
		xgeImageFree(&objImage);
		return 18;
	}
	xgeImageFree(&objImage);
	return 0;
}

static int __testBaseTypesAndInput(void)
{
	xge_color_t tColor;
	xge_touch_point_t tPoint;
	xge_camera_t tCamera;
	xge_vec2_t tPointA;
	xge_vec2_t tPointB;
	xge_rect_t tRect;
	float fX;
	float fY;

	if ( xgeColorRGBA(1, 2, 3, 4) != XGE_COLOR_RGBA(1, 2, 3, 4) ) {
		return 20;
	}
	tColor = xgeColorUnpack(XGE_COLOR_RGBA(255, 128, 0, 64));
	if ( (tColor.fR < 0.99f) || (tColor.fG < 0.49f) || (tColor.fB != 0.0f) || (tColor.fA < 0.24f) ) {
		return 21;
	}

	fX = 1.0f;
	fY = 1.0f;
	xgeMouseGetDelta(&fX, &fY);
	if ( (fX != 0.0f) || (fY != 0.0f) ) {
		return 22;
	}
	xgeMouseGetWheel(&fX, &fY);
	if ( (fX != 0.0f) || (fY != 0.0f) ) {
		return 23;
	}
	if ( xgeTextGet() != 0 ) {
		return 24;
	}
	if ( xgeTouchGetCount() != 0 ) {
		return 25;
	}
	if ( xgeTouchGet(0, &tPoint) != XGE_ERROR_INVALID_ARGUMENT ) {
		return 26;
	}
	if ( xgeBlendGet() != XGE_BLEND_ALPHA ) {
		return 27;
	}
	xgeBlendSet(XGE_BLEND_ADD);
	if ( xgeBlendGet() != XGE_BLEND_ADD ) {
		return 28;
	}

	tCamera = xgeCameraDefault(320.0f, 200.0f);
	tCamera.tPosition.fX = 10.0f;
	tCamera.tPosition.fY = 20.0f;
	tCamera.tScale.fX = 2.0f;
	tCamera.tScale.fY = 4.0f;
	xgeCameraSet(&tCamera);
	tPointA.fX = 11.0f;
	tPointA.fY = 22.0f;
	tPointB = xgeWorldToScreen(tPointA);
	if ( (tPointB.fX != 2.0f) || (tPointB.fY != 8.0f) ) {
		return 29;
	}
	tPointA = xgeScreenToWorld(tPointB);
	if ( (tPointA.fX != 11.0f) || (tPointA.fY != 22.0f) ) {
		return 30;
	}
	tCamera.iCoordinateMode = XGE_COORD_CENTER;
	xgeCameraSet(&tCamera);
	tPointA.fX = 10.0f;
	tPointA.fY = 20.0f;
	tPointB = xgeWorldToScreen(tPointA);
	if ( (tPointB.fX != 160.0f) || (tPointB.fY != 100.0f) ) {
		return 31;
	}
	tRect = xgeViewportGet();
	if ( tRect.fX != 0.0f || tRect.fY != 0.0f || tRect.fW != 320.0f || tRect.fH != 200.0f ) {
		return 32;
	}
	tRect.fX = 2.0f;
	tRect.fY = 3.0f;
	tRect.fW = 40.0f;
	tRect.fH = 50.0f;
	xgeViewportSet(tRect);
	tRect = xgeViewportGet();
	if ( tRect.fX != 2.0f || tRect.fY != 3.0f || tRect.fW != 40.0f || tRect.fH != 50.0f ) {
		return 33;
	}
	xgeViewportClear();
	tRect = xgeViewportGet();
	if ( tRect.fX != 0.0f || tRect.fY != 0.0f || tRect.fW != 320.0f || tRect.fH != 200.0f ) {
		return 34;
	}
	tRect.fX = 4.0f;
	tRect.fY = 5.0f;
	tRect.fW = -1.0f;
	tRect.fH = 6.0f;
	xgeClipSet(tRect);
	tRect = xgeClipGet();
	if ( tRect.fX != 4.0f || tRect.fY != 5.0f || tRect.fW != 0.0f || tRect.fH != 6.0f ) {
		return 35;
	}
	xgeClipClear();
	tRect = xgeClipGet();
	if ( tRect.fW != 0.0f || tRect.fH != 0.0f ) {
		return 36;
	}
	return 0;
}

static int __testTextureLifetime(void)
{
	xge_texture_t tTexture;
	xge_rect_t tRect;
	xge_vec2_t arrPoly[4];
	unsigned char arrPixels[16];
	unsigned char arrReadback[24];

	tTexture.iWidth = 1;
	tTexture.iHeight = 1;
	tTexture.iFormat = XGE_PIXEL_RGBA8;
	tTexture.iRefCount = 1;
	tTexture.iFlags = 0;
	tTexture.iBackendId = 0;
	tTexture.pBackend = 0;

	if ( xgeTextureAddRef(&tTexture) != 2 ) {
		return 40;
	}
	xgeTextureFree(&tTexture);
	if ( tTexture.iRefCount != 1 ) {
		return 41;
	}
	xgeTextureFree(&tTexture);
	if ( tTexture.iRefCount != 0 || tTexture.iWidth != 0 ) {
		return 42;
	}
	arrPixels[0] = 1; arrPixels[1] = 2; arrPixels[2] = 3; arrPixels[3] = 4;
	arrPixels[4] = 5; arrPixels[5] = 6; arrPixels[6] = 7; arrPixels[7] = 8;
	arrPixels[8] = 9; arrPixels[9] = 10; arrPixels[10] = 11; arrPixels[11] = 12;
	arrPixels[12] = 13; arrPixels[13] = 14; arrPixels[14] = 15; arrPixels[15] = 16;
	if ( xgeTextureCreateRGBA(&tTexture, 2, 2, arrPixels) != XGE_OK || tTexture.iWidth != 2 || tTexture.iHeight != 2 || tTexture.pBackend == 0 ) {
		return 43;
	}
	memset(arrReadback, 0, sizeof(arrReadback));
	if ( xgeTextureReadPixels(&tTexture, arrReadback, 12) != XGE_OK ) {
		xgeTextureFree(&tTexture);
		return 44;
	}
	if ( arrReadback[0] != 1 || arrReadback[7] != 8 || arrReadback[12] != 9 || arrReadback[19] != 16 ) {
		xgeTextureFree(&tTexture);
		return 45;
	}
	if ( xgeTextureReadPixels(&tTexture, arrReadback, 4) != XGE_ERROR_INVALID_ARGUMENT || xgeTextureReadPixels(NULL, arrReadback, 0) != XGE_ERROR_INVALID_ARGUMENT || xgeTextureReadPixels(&tTexture, NULL, 0) != XGE_ERROR_INVALID_ARGUMENT ) {
		xgeTextureFree(&tTexture);
		return 46;
	}
	xgeTextureFree(&tTexture);
	if ( tTexture.pBackend != 0 || tTexture.iWidth != 0 ) {
		return 47;
	}
	tRect.fX = 0.0f;
	tRect.fY = 0.0f;
	tRect.fW = 10.0f;
	tRect.fH = 10.0f;
	arrPoly[0].fX = 0.0f;
	arrPoly[0].fY = 0.0f;
	arrPoly[1].fX = 10.0f;
	arrPoly[1].fY = 0.0f;
	arrPoly[2].fX = 10.0f;
	arrPoly[2].fY = 10.0f;
	arrPoly[3].fX = 0.0f;
	arrPoly[3].fY = 10.0f;
	xgeShapePoint(5.0f, 5.0f, 2.0f, XGE_COLOR_RGBA(255, 255, 255, 255));
	xgeShapePointPx(6.0f, 6.0f, 3.0f, XGE_COLOR_RGBA(255, 128, 128, 255));
	xgeShapeLine(0.0f, 0.0f, 10.0f, 10.0f, 1.0f, XGE_COLOR_RGBA(255, 255, 255, 255));
	xgeShapeLinePx(0.0f, 10.0f, 10.0f, 0.0f, 2.0f, XGE_COLOR_RGBA(255, 255, 0, 255));
	xgeShapeRectFill(tRect, XGE_COLOR_RGBA(255, 0, 0, 255));
	xgeShapeRectFillPx(tRect, XGE_COLOR_RGBA(0, 255, 0, 255));
	xgeShapeRectStroke(tRect, 1.0f, XGE_COLOR_RGBA(0, 0, 255, 255));
	xgeShapeRectStrokePx(tRect, 2.0f, XGE_COLOR_RGBA(255, 0, 255, 255));
	xgeShapeCircleFill(5.0f, 5.0f, 3.0f, XGE_COLOR_RGBA(64, 128, 255, 255));
	xgeShapeCircleFillPx(6.0f, 6.0f, 4.0f, XGE_COLOR_RGBA(64, 255, 128, 255));
	xgeShapeCircleStroke(5.0f, 5.0f, 3.0f, 1.0f, XGE_COLOR_RGBA(255, 64, 64, 255));
	xgeShapeCircleStrokePx(6.0f, 6.0f, 4.0f, 2.0f, XGE_COLOR_RGBA(255, 128, 64, 255));
	xgeShapeArc(5.0f, 5.0f, 3.0f, 0.0f, 1.57079632679f, 1.0f, XGE_COLOR_RGBA(128, 64, 255, 255));
	xgeShapeArcPx(6.0f, 6.0f, 4.0f, 1.57079632679f, 3.14159265359f, 2.0f, XGE_COLOR_RGBA(128, 255, 64, 255));
	xgeShapeTriangleFill(arrPoly[0], arrPoly[1], arrPoly[2], XGE_COLOR_RGBA(0, 255, 255, 255));
	xgeShapeTriangleFillPx(arrPoly[0], arrPoly[2], arrPoly[3], XGE_COLOR_RGBA(255, 128, 0, 255));
	xgeShapePolygonFill(arrPoly, 4, XGE_COLOR_RGBA(180, 180, 255, 255));
	xgeShapePolygonFillPx(arrPoly, 4, XGE_COLOR_RGBA(180, 255, 180, 255));
	xgeClipSet(tRect);
	xgeClipClear();
	return 0;
}

static int __testResourceProtocol(void)
{
	FILE* pFile;
	xge_resource_provider_t tProvider;
	xge_resource_t tResource;
	static const char arrMemory[] = { 'm', 'e', 'm' };
	static const char arrFile[] = { 'r', 'e', 's' };
	char* pBytes;

	pFile = fopen("build/xge_resource_test.bin", "wb");
	if ( pFile == 0 ) {
		return 50;
	}
	if ( fwrite(arrFile, 1, sizeof(arrFile), pFile) != sizeof(arrFile) ) {
		fclose(pFile);
		return 51;
	}
	fclose(pFile);

	if ( xgeResourceLoad("res://xge_resource_test.bin", &tResource) != XGE_OK ) {
		return 52;
	}
	pBytes = (char*)tResource.pData;
	if ( (tResource.iSize != (int)sizeof(arrFile)) || (memcmp(pBytes, arrFile, sizeof(arrFile)) != 0) ) {
		xgeResourceFree(&tResource);
		return 53;
	}
	xgeResourceFree(&tResource);
	if ( tResource.pData != 0 ) {
		return 54;
	}

	if ( xgeResourceLoadMemory(arrMemory, (int)sizeof(arrMemory), &tResource) != XGE_OK ) {
		return 55;
	}
	pBytes = (char*)tResource.pData;
	if ( (pBytes == arrMemory) || (tResource.iSize != (int)sizeof(arrMemory)) || (memcmp(pBytes, arrMemory, sizeof(arrMemory)) != 0) ) {
		xgeResourceFree(&tResource);
		return 56;
	}
	xgeResourceFree(&tResource);

	memset(&tProvider, 0, sizeof(tProvider));
	tProvider.sScheme = "mock";
	tProvider.load = __testProviderLoad;
	tProvider.free = __testProviderFree;
	if ( xgeResourceProviderAdd(&tProvider) != XGE_OK ) {
		return 57;
	}
	g_iProviderFreeCount = 0;
	if ( xgeResourceLoad("mock://asset", &tResource) != XGE_OK ) {
		return 58;
	}
	if ( (tResource.pData != g_arrProviderData) || (tResource.iSize != 3) ) {
		xgeResourceFree(&tResource);
		return 59;
	}
	xgeResourceProviderClear();
	xgeResourceFree(&tResource);
	if ( g_iProviderFreeCount != 1 ) {
		return 60;
	}
	return 0;
}

static int __testAudioApiShape(void)
{
	xge_audio_listener_t tListener;
	xge_audio_group_t tGroup;
	xge_sound_t tSound;

	if ( xgeAudioIsReady() != 0 ) {
		return 70;
	}
	if ( xgeAudioGetVolume() != 0.0f ) {
		return 71;
	}
	tListener = xgeAudioListenerGet();
	if ( (tListener.tForward.fZ != -1.0f) || (tListener.tUp.fY != 1.0f) ) {
		return 72;
	}
	memset(&tSound, 0, sizeof(tSound));
	tSound.iRefCount = 1;
	tSound.iType = XGE_AUDIO_SOUND;
	if ( xgeSoundAddRef(&tSound) != 2 ) {
		return 73;
	}
	xgeSoundFree(&tSound);
	if ( tSound.iRefCount != 1 ) {
		return 74;
	}
	xgeSoundFree(&tSound);
	if ( tSound.iRefCount != 0 ) {
		return 75;
	}
	if ( xgeSoundPlay(0) != XGE_ERROR_INVALID_ARGUMENT ) {
		return 76;
	}
	memset(&tGroup, 0, sizeof(tGroup));
	if ( xgeAudioGroupInit(&tGroup) != XGE_ERROR_NOT_INITIALIZED ) {
		return 77;
	}
	xgeAudioGroupSetVolume(&tGroup, 0.5f);
	if ( xgeAudioGroupGetVolume(&tGroup) != 0.5f ) {
		return 78;
	}
	xgeAudioGroupFade(&tGroup, 0.5f, 0.25f, 100);
	if ( xgeAudioGroupGetVolume(&tGroup) != 0.5f ) {
		return 79;
	}
	xgeAudioGroupFree(&tGroup);
	if ( xgeSoundLoadGroup(0, "none.wav", &tGroup) != XGE_ERROR_INVALID_ARGUMENT ) {
		return 80;
	}
	return 0;
}

static int __testTextFontBase(void)
{
	const char* sText;
	uint32_t iCodepoint;
	xge_font_t tFont;
	xge_glyph_metrics_t tMetrics;
	xge_glyph_bitmap_t tBitmap;
	xge_glyph_t tGlyphA;
	xge_glyph_t tGlyphB;
	xge_vec2_t tSize;
	FILE* pFile;

	sText = "A\xe4\xb8\xad";
	if ( xgeTextUTF8Next(&sText, &iCodepoint) != XGE_OK || iCodepoint != 'A' ) {
		return 90;
	}
	if ( xgeTextUTF8Next(&sText, &iCodepoint) != XGE_OK || iCodepoint != 0x4E2D ) {
		return 91;
	}
	if ( xgeTextUTF8Next(&sText, &iCodepoint) != XGE_ERROR ) {
		return 92;
	}
	sText = "\xF0\x9F\x98\x80";
	if ( xgeTextUTF8Next(&sText, &iCodepoint) != XGE_ERROR_UNSUPPORTED ) {
		return 93;
	}

	memset(&tFont, 0, sizeof(tFont));
	if ( xgeFontLoadMemory(&tFont, "bad", 3, 16.0f) != XGE_ERROR_RESOURCE_FAILED ) {
		return 94;
	}
	if ( xgeFontLoadMemory(&tFont, "bad", 3, 0.0f) != XGE_ERROR_INVALID_ARGUMENT ) {
		return 95;
	}

	pFile = fopen("C:/Windows/Fonts/arial.ttf", "rb");
	if ( pFile != 0 ) {
		fclose(pFile);
		if ( xgeFontLoad(&tFont, "C:/Windows/Fonts/arial.ttf", 16.0f) != XGE_OK ) {
			return 96;
		}
		if ( tFont.iRefCount != 1 || tFont.fLineHeight <= 0.0f || tFont.pBackend == 0 ) {
			xgeFontFree(&tFont);
			return 97;
		}
		if ( xgeFontGlyphGet(&tFont, 'A', &tMetrics) != XGE_OK || tMetrics.fAdvanceX <= 0.0f ) {
			xgeFontFree(&tFont);
			return 98;
		}
		if ( xgeFontGlyphRasterize(&tFont, 'A', &tBitmap) != XGE_OK || tBitmap.iFormat != XGE_PIXEL_A8 || tBitmap.iWidth <= 0 || tBitmap.iHeight <= 0 ) {
			xgeFontFree(&tFont);
			return 103;
		}
		xgeGlyphBitmapFree(&tBitmap);
		if ( tBitmap.pPixels != 0 ) {
			xgeFontFree(&tFont);
			return 104;
		}
		if ( xgeFontGlyphAtlasGet(&tFont, 'A', &tGlyphA) != XGE_OK || tGlyphA.iPage < 0 || tGlyphA.iWidth <= 0 || tFont.tAtlas.iPageCount <= 0 ) {
			xgeFontFree(&tFont);
			return 105;
		}
		if ( xgeFontGlyphAtlasGet(&tFont, 'A', &tGlyphB) != XGE_OK || tGlyphA.iX != tGlyphB.iX || tGlyphA.iY != tGlyphB.iY || tGlyphA.iPage != tGlyphB.iPage ) {
			xgeFontFree(&tFont);
			return 106;
		}
		tSize = xgeTextMeasure(&tFont, "ABC\nD");
		if ( tSize.fX <= 0.0f || tSize.fY <= tFont.fLineHeight ) {
			xgeFontFree(&tFont);
			return 99;
		}
		if ( xgeFontAddRef(&tFont) != 2 ) {
			xgeFontFree(&tFont);
			return 100;
		}
		xgeFontFree(&tFont);
		if ( tFont.iRefCount != 1 ) {
			xgeFontFree(&tFont);
			return 101;
		}
		xgeFontFree(&tFont);
		if ( tFont.iRefCount != 0 || tFont.pData != 0 ) {
			return 102;
		}
	}
	return 0;
}

static int __testXRFMemory(void)
{
	struct xrf_test_blob_t {
		xge_xrf_header_t tHeader;
		xge_xrf_range_t tRange;
		xge_xrf_glyph_t arrGlyphs[2];
		xge_xrf_page_t tPage;
		unsigned char arrPixels[16 * 16];
	} tBlob;
	xge_font_t tFont;
	xge_glyph_t tGlyph;
	xge_glyph_metrics_t tMetrics;
	xge_vec2_t tSize;
	int i;

	memset(&tBlob, 0, sizeof(tBlob));
	tBlob.tHeader.iMagic = XGE_XRF_MAGIC;
	tBlob.tHeader.iVersion = XGE_XRF_VERSION;
	tBlob.tHeader.iHeaderSize = sizeof(xge_xrf_header_t);
	tBlob.tHeader.iGlyphCount = 2;
	tBlob.tHeader.iPageCount = 1;
	tBlob.tHeader.iRangeCount = 1;
	tBlob.tHeader.fAscent = 10.0f;
	tBlob.tHeader.fDescent = -3.0f;
	tBlob.tHeader.fLineGap = 1.0f;
	tBlob.tHeader.fLineHeight = 14.0f;
	tBlob.tHeader.iRangeOffset = sizeof(xge_xrf_header_t);
	tBlob.tHeader.iGlyphOffset = tBlob.tHeader.iRangeOffset + sizeof(xge_xrf_range_t);
	tBlob.tHeader.iPageOffset = tBlob.tHeader.iGlyphOffset + sizeof(tBlob.arrGlyphs);
	tBlob.tHeader.iPixelOffset = tBlob.tHeader.iPageOffset + sizeof(xge_xrf_page_t);
	tBlob.tRange.iFirstCodepoint = 'A';
	tBlob.tRange.iCount = 2;
	tBlob.arrGlyphs[0].iCodepoint = 'A';
	tBlob.arrGlyphs[0].iPage = 0;
	tBlob.arrGlyphs[0].iGlyph = 1;
	tBlob.arrGlyphs[0].iX = 0;
	tBlob.arrGlyphs[0].iY = 0;
	tBlob.arrGlyphs[0].iWidth = 4;
	tBlob.arrGlyphs[0].iHeight = 6;
	tBlob.arrGlyphs[0].fAdvanceX = 8.0f;
	tBlob.arrGlyphs[1].iCodepoint = 0x4E2D;
	tBlob.arrGlyphs[1].iPage = 0;
	tBlob.arrGlyphs[1].iGlyph = 2;
	tBlob.arrGlyphs[1].iX = 4;
	tBlob.arrGlyphs[1].iY = 0;
	tBlob.arrGlyphs[1].iWidth = 8;
	tBlob.arrGlyphs[1].iHeight = 8;
	tBlob.arrGlyphs[1].fAdvanceX = 12.0f;
	tBlob.tPage.iWidth = 16;
	tBlob.tPage.iHeight = 16;
	tBlob.tPage.iFormat = XGE_XRF_PAGE_A8;
	tBlob.tPage.iPixelOffset = 0;
	tBlob.tPage.iPixelSize = sizeof(tBlob.arrPixels);
	for ( i = 0; i < (int)sizeof(tBlob.arrPixels); i++ ) {
		tBlob.arrPixels[i] = (unsigned char)i;
	}

	memset(&tFont, 0, sizeof(tFont));
	if ( xgeFontLoadXRFMemory(&tFont, &tBlob, sizeof(tBlob)) != XGE_OK ) {
		return 110;
	}
	if ( tFont.fLineHeight != 14.0f || tFont.tAtlas.iPageCount != 1 || tFont.tAtlas.iPageWidth != 16 ) {
		xgeFontFree(&tFont);
		return 111;
	}
	if ( xgeFontGlyphAtlasGet(&tFont, 'A', &tGlyph) != XGE_OK || tGlyph.iPage != 0 || tGlyph.fAdvanceX != 8.0f ) {
		xgeFontFree(&tFont);
		return 112;
	}
	if ( xgeFontGlyphGet(&tFont, 0x4E2D, &tMetrics) != XGE_OK || tMetrics.fAdvanceX != 12.0f ) {
		xgeFontFree(&tFont);
		return 113;
	}
	tSize = xgeTextMeasure(&tFont, "A\xe4\xb8\xad");
	if ( tSize.fX != 20.0f || tSize.fY != 14.0f ) {
		xgeFontFree(&tFont);
		return 114;
	}
	xgeFontFree(&tFont);
	if ( tFont.iRefCount != 0 || tFont.tAtlas.iPageCount != 0 ) {
		return 115;
	}
	tBlob.tHeader.iMagic = 0;
	if ( xgeFontLoadXRFMemory(&tFont, &tBlob, sizeof(tBlob)) != XGE_ERROR_RESOURCE_FAILED ) {
		return 116;
	}
	return 0;
}

static void __testXrfBlobMake(xrf_test_blob2_t* pBlob)
{
	int i;

	memset(pBlob, 0, sizeof(*pBlob));
	pBlob->tHeader.iMagic = XGE_XRF_MAGIC;
	pBlob->tHeader.iVersion = XGE_XRF_VERSION;
	pBlob->tHeader.iHeaderSize = sizeof(xge_xrf_header_t);
	pBlob->tHeader.iGlyphCount = 2;
	pBlob->tHeader.iPageCount = 1;
	pBlob->tHeader.iRangeCount = 1;
	pBlob->tHeader.fAscent = 10.0f;
	pBlob->tHeader.fDescent = -3.0f;
	pBlob->tHeader.fLineGap = 1.0f;
	pBlob->tHeader.fLineHeight = 14.0f;
	pBlob->tHeader.iRangeOffset = sizeof(xge_xrf_header_t);
	pBlob->tHeader.iGlyphOffset = pBlob->tHeader.iRangeOffset + sizeof(xge_xrf_range_t);
	pBlob->tHeader.iPageOffset = pBlob->tHeader.iGlyphOffset + sizeof(pBlob->arrGlyphs);
	pBlob->tHeader.iPixelOffset = pBlob->tHeader.iPageOffset + sizeof(xge_xrf_page_t);
	pBlob->tRange.iFirstCodepoint = 'A';
	pBlob->tRange.iCount = 2;
	pBlob->arrGlyphs[0].iCodepoint = 'A';
	pBlob->arrGlyphs[0].iPage = 0;
	pBlob->arrGlyphs[0].iGlyph = 1;
	pBlob->arrGlyphs[0].iX = 0;
	pBlob->arrGlyphs[0].iY = 0;
	pBlob->arrGlyphs[0].iWidth = 4;
	pBlob->arrGlyphs[0].iHeight = 6;
	pBlob->arrGlyphs[0].fAdvanceX = 8.0f;
	pBlob->arrGlyphs[1].iCodepoint = 0x4E2D;
	pBlob->arrGlyphs[1].iPage = 0;
	pBlob->arrGlyphs[1].iGlyph = 2;
	pBlob->arrGlyphs[1].iX = 4;
	pBlob->arrGlyphs[1].iY = 0;
	pBlob->arrGlyphs[1].iWidth = 8;
	pBlob->arrGlyphs[1].iHeight = 8;
	pBlob->arrGlyphs[1].fAdvanceX = 12.0f;
	pBlob->tPage.iWidth = 16;
	pBlob->tPage.iHeight = 16;
	pBlob->tPage.iFormat = XGE_XRF_PAGE_A8;
	pBlob->tPage.iPixelOffset = 0;
	pBlob->tPage.iPixelSize = sizeof(pBlob->arrPixels);
	for ( i = 0; i < (int)sizeof(pBlob->arrPixels); i++ ) {
		pBlob->arrPixels[i] = (unsigned char)i;
	}
}

static int __testXuiIncubationBase(void)
{
	xui_host_test_t tHostState;
	xge_xui_host_t tHost;
	xge_xui_context_t tXui;
	xge_xui_widget pRoot;
	xge_xui_widget pChild;
	xge_rect_t tRect;
	int iPaintCount;

	memset(&tHostState, 0, sizeof(tHostState));
	memset(&tHost, 0, sizeof(tHost));
	memset(&tXui, 0, sizeof(tXui));
	if ( xgeXuiInit(NULL) != XGE_ERROR_INVALID_ARGUMENT ) {
		return 120;
	}
	if ( xgeXuiInit(&tXui) != XGE_OK ) {
		return 121;
	}
	if ( xgeXuiGetHost(&tXui) == NULL ) {
		xgeXuiUnit(&tXui);
		return 135;
	}
	tHost.draw_rect = __testXuiHostDrawRect;
	tHost.draw_image = __testXuiHostDrawImage;
	tHost.draw_text_rect = __testXuiHostDrawTextRect;
	tHost.measure_text = __testXuiHostMeasureText;
	tHost.clip_set = __testXuiHostClipSet;
	tHost.clip_clear = __testXuiHostClipClear;
	tHost.pUser = &tHostState;
	xgeXuiSetHost(&tXui, &tHost);
	if ( xgeXuiGetHost(&tXui) != &tHost ) {
		xgeXuiUnit(&tXui);
		return 136;
	}
	if ( xgeXuiInit(&tXui) != XGE_ERROR_ALREADY_INITIALIZED ) {
		xgeXuiUnit(&tXui);
		return 122;
	}
	pRoot = xgeXuiRoot(&tXui);
	if ( pRoot == NULL || pRoot->pFirstChild != NULL ) {
		xgeXuiUnit(&tXui);
		return 123;
	}
	pChild = xgeXuiWidgetCreate();
	if ( pChild == NULL ) {
		xgeXuiUnit(&tXui);
		return 124;
	}
	tRect.fX = 10.0f;
	tRect.fY = 20.0f;
	tRect.fW = 30.0f;
	tRect.fH = 40.0f;
	xgeXuiWidgetSetRect(pChild, tRect);
	xgeXuiWidgetSetLayout(pChild, XGE_XUI_LAYOUT_ABSOLUTE);
	xgeXuiWidgetSetBackground(pChild, XGE_COLOR_RGBA(255, 0, 0, 255));
	xgeXuiWidgetSetClip(pChild, 1);
	xgeXuiWidgetSetId(pChild, 1001);
	xgeXuiWidgetSetName(pChild, "base-child");
	if ( (xgeXuiWidgetGetId(pChild) != 1001) || (strcmp(xgeXuiWidgetGetName(pChild), "base-child") != 0) ) {
		xgeXuiWidgetFree(pChild);
		xgeXuiUnit(&tXui);
		return 133;
	}
	if ( (xgeXuiWidgetGetFlags(pChild) & XGE_XUI_WIDGET_CLIP) == 0 ) {
		xgeXuiWidgetFree(pChild);
		xgeXuiUnit(&tXui);
		return 125;
	}
	if ( xgeXuiWidgetAdd(pRoot, pChild) != XGE_OK ) {
		xgeXuiWidgetFree(pChild);
		xgeXuiUnit(&tXui);
		return 126;
	}
	if ( pRoot->pFirstChild != pChild || pChild->pParent != pRoot ) {
		xgeXuiUnit(&tXui);
		return 127;
	}
	if ( xgeXuiWidgetFindById(pRoot, 1001) != pChild || xgeXuiWidgetFindByName(pRoot, "base-child") != pChild || xgeXuiWidgetFindByName(pRoot, "missing") != NULL ) {
		xgeXuiUnit(&tXui);
		return 134;
	}
	if ( xgeXuiUpdate(&tXui, 1.0f / 60.0f) != XGE_OK ) {
		xgeXuiUnit(&tXui);
		return 128;
	}
	tRect = xgeXuiWidgetGetRect(pChild);
	if ( tRect.fX != 10.0f || tRect.fY != 20.0f || tRect.fW != 30.0f || tRect.fH != 40.0f ) {
		xgeXuiUnit(&tXui);
		return 129;
	}
	iPaintCount = xgeXuiPaint(&tXui);
	if ( iPaintCount != 1 || tXui.iPaintCommandCount != 1 || tXui.iPaintFlushCount < 1 ) {
		xgeXuiUnit(&tXui);
		return 130;
	}
	if ( tHostState.iDrawRect != 1 || tHostState.iClipSet != 1 || tHostState.iClipClear != 1 || tHostState.iDrawImage != 0 || tHostState.iDrawText != 0 ) {
		xgeXuiUnit(&tXui);
		return 137;
	}
	iPaintCount = xgeXuiPaint(&tXui);
	if ( iPaintCount != 0 || tXui.iPaintCommandCount != 0 || tXui.iPaintFlushCount != 0 || tHostState.iDrawRect != 1 ) {
		xgeXuiUnit(&tXui);
		return 159;
	}
	xgeXuiUpdate(&tXui, 0.0f);
	if ( (xgeXuiWidgetGetFlags(pRoot) & XGE_XUI_WIDGET_DIRTY_LAYOUT) != 0 || (xgeXuiWidgetGetFlags(pChild) & XGE_XUI_WIDGET_DIRTY_LAYOUT) != 0 ) {
		xgeXuiUnit(&tXui);
		return 139;
	}
	xgeXuiLayoutBatchBegin(&tXui);
	xgeXuiWidgetSetRect(pChild, tRect);
	xgeXuiWidgetSetBackground(pChild, XGE_COLOR_RGBA(0, 255, 0, 255));
	if ( (xgeXuiWidgetGetFlags(pRoot) & XGE_XUI_WIDGET_DIRTY_LAYOUT) != 0 || (xgeXuiWidgetGetFlags(pChild) & XGE_XUI_WIDGET_DIRTY_LAYOUT) != 0 || tXui.bLayoutBatchDirtyLayout == 0 || tXui.bLayoutBatchDirtyPaint == 0 ) {
		xgeXuiUnit(&tXui);
		return 156;
	}
	xgeXuiLayoutBatchEnd(&tXui);
	if ( (xgeXuiWidgetGetFlags(pRoot) & XGE_XUI_WIDGET_DIRTY_LAYOUT) == 0 || (xgeXuiWidgetGetFlags(pRoot) & XGE_XUI_WIDGET_DIRTY_PAINT) == 0 || (xgeXuiWidgetGetFlags(pChild) & XGE_XUI_WIDGET_DIRTY_LAYOUT) != 0 || tXui.iLayoutBatchDepth != 0 ) {
		xgeXuiUnit(&tXui);
		return 157;
	}
	if ( xgeXuiUpdate(&tXui, 0.0f) != XGE_OK || (xgeXuiWidgetGetFlags(pRoot) & XGE_XUI_WIDGET_DIRTY_LAYOUT) != 0 ) {
		xgeXuiUnit(&tXui);
		return 158;
	}
	xgeXuiSetHost(&tXui, NULL);
	if ( xgeXuiGetHost(&tXui) == &tHost ) {
		xgeXuiUnit(&tXui);
		return 138;
	}
	xgeXuiWidgetRemove(pChild);
	if ( pRoot->pFirstChild != NULL || pChild->pParent != NULL ) {
		xgeXuiWidgetFree(pChild);
		xgeXuiUnit(&tXui);
		return 131;
	}
	xgeXuiWidgetFree(pChild);
	xgeXuiUnit(&tXui);
	if ( tXui.bInitialized != 0 || tXui.pRoot != NULL ) {
		return 132;
	}
	return 0;
}

static int __testXuiPaintCommands(void)
{
	xrf_test_blob2_t tBlob;
	xui_host_test_t tHostState;
	xge_xui_host_t tHost;
	xge_font_t tFont;
	xge_texture_t tTexture;
	xge_xui_context_t tXui;
	xge_xui_label_t tLabel;
	xge_xui_image_t tImage;
	xge_xui_widget pRoot;
	xge_xui_widget pPanel;
	xge_xui_widget pLabel;
	xge_xui_widget pImage;
	xge_xui_widget pCustom;
	xge_rect_t tRect;
	int iCustomPaintCount;
	int iPaintCount;

	__testXrfBlobMake(&tBlob);
	memset(&tHostState, 0, sizeof(tHostState));
	memset(&tHost, 0, sizeof(tHost));
	memset(&tFont, 0, sizeof(tFont));
	memset(&tTexture, 0, sizeof(tTexture));
	memset(&tXui, 0, sizeof(tXui));
	memset(&tLabel, 0, sizeof(tLabel));
	memset(&tImage, 0, sizeof(tImage));
	iCustomPaintCount = 0;
	tHost.draw_rect = __testXuiHostDrawRect;
	tHost.draw_image = __testXuiHostDrawImage;
	tHost.draw_text_rect = __testXuiHostDrawTextRect;
	tHost.measure_text = __testXuiHostMeasureText;
	tHost.clip_set = __testXuiHostClipSet;
	tHost.clip_clear = __testXuiHostClipClear;
	tHost.pUser = &tHostState;
	tTexture.iWidth = 16;
	tTexture.iHeight = 8;
	tTexture.iFormat = XGE_PIXEL_RGBA8;
	tTexture.iRefCount = 1;
	if ( xgeFontLoadXRFMemory(&tFont, &tBlob, sizeof(tBlob)) != XGE_OK ) {
		return 391;
	}
	if ( xgeXuiInit(&tXui) != XGE_OK ) {
		xgeFontFree(&tFont);
		return 392;
	}
	xgeXuiSetHost(&tXui, &tHost);
	pRoot = xgeXuiRoot(&tXui);
	pPanel = xgeXuiWidgetCreate();
	pLabel = xgeXuiWidgetCreate();
	pImage = xgeXuiWidgetCreate();
	pCustom = xgeXuiWidgetCreate();
	if ( (pRoot == NULL) || (pPanel == NULL) || (pLabel == NULL) || (pImage == NULL) || (pCustom == NULL) ) {
		xgeXuiWidgetFree(pPanel);
		xgeXuiWidgetFree(pLabel);
		xgeXuiWidgetFree(pImage);
		xgeXuiWidgetFree(pCustom);
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 393;
	}
	tRect.fX = 0.0f;
	tRect.fY = 0.0f;
	tRect.fW = 80.0f;
	tRect.fH = 60.0f;
	xgeXuiWidgetSetRect(pPanel, tRect);
	xgeXuiWidgetSetClip(pPanel, 1);
	xgeXuiWidgetSetBackground(pPanel, XGE_COLOR_RGBA(20, 30, 40, 255));
	xgeXuiWidgetSetRadius(pPanel, 6.0f);
	xgeXuiWidgetSetLayout(pPanel, XGE_XUI_LAYOUT_ROW);
	xgeXuiWidgetAdd(pRoot, pPanel);
	xgeXuiWidgetAdd(pPanel, pLabel);
	xgeXuiWidgetAdd(pPanel, pImage);
	xgeXuiWidgetAdd(pPanel, pCustom);
	xgeXuiWidgetSetPaint(pCustom, __testXuiCustomPaint, &iCustomPaintCount);
	if ( xgeXuiLabelInit(&tLabel, pLabel, &tFont, "A") != XGE_OK ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 394;
	}
	if ( xgeXuiImageInit(&tImage, pImage, &tTexture) != XGE_OK ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 395;
	}
	xgeXuiUpdate(&tXui, 0.0f);
	iPaintCount = xgeXuiPaint(&tXui);
	if ( iPaintCount != 4 || tXui.iPaintCommandCount != 4 || tXui.iPaintFlushCount < 4 || iCustomPaintCount != 1 ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 396;
	}
	if ( tHostState.iDrawRect != 1 || tHostState.iDrawImage != 1 || tHostState.iDrawText != 1 || tHostState.iClipSet != 1 || tHostState.iClipClear != 1 ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 397;
	}
	xgeXuiLabelUnit(&tLabel);
	xgeXuiImageUnit(&tImage);
	xgeXuiUnit(&tXui);
	xgeFontFree(&tFont);
	return 0;
}

static int __testXuiManualRefresh(void)
{
	xui_host_test_t tHostState;
	xge_xui_host_t tHost;
	xge_xui_context_t tXui;
	xge_xui_widget pRoot;
	xge_xui_widget pChild;
	xge_rect_t tRect;
	xge_rect_t tDirty;

	memset(&tHostState, 0, sizeof(tHostState));
	memset(&tHost, 0, sizeof(tHost));
	memset(&tXui, 0, sizeof(tXui));
	tHost.draw_rect = __testXuiHostDrawRect;
	tHost.request_refresh = __testXuiHostRequestRefresh;
	tHost.pUser = &tHostState;
	if ( xgeXuiInit(&tXui) != XGE_OK ) {
		return 411;
	}
	if ( xgeXuiRefreshNeeded(&tXui) != 0 ) {
		xgeXuiUnit(&tXui);
		return 412;
	}
	xgeXuiSetHost(&tXui, &tHost);
	if ( xgeXuiRefreshNeeded(&tXui) == 0 || tHostState.iRequestRefresh != 1 ) {
		xgeXuiUnit(&tXui);
		return 413;
	}
	xgeXuiRefreshRequest(&tXui);
	if ( tHostState.iRequestRefresh != 1 ) {
		xgeXuiUnit(&tXui);
		return 414;
	}
	xgeXuiRefreshClear(&tXui);
	if ( xgeXuiRefreshNeeded(&tXui) != 0 ) {
		xgeXuiUnit(&tXui);
		return 415;
	}
	pRoot = xgeXuiRoot(&tXui);
	pChild = xgeXuiWidgetCreate();
	if ( (pRoot == NULL) || (pChild == NULL) ) {
		xgeXuiWidgetFree(pChild);
		xgeXuiUnit(&tXui);
		return 416;
	}
	tRect.fX = 1.0f;
	tRect.fY = 2.0f;
	tRect.fW = 3.0f;
	tRect.fH = 4.0f;
	xgeXuiWidgetSetRect(pChild, tRect);
	if ( tHostState.iRequestRefresh != 1 ) {
		xgeXuiWidgetFree(pChild);
		xgeXuiUnit(&tXui);
		return 417;
	}
	xgeDirtyRectClear();
	xgeXuiWidgetMarkPaint(pChild);
	if ( xgeDirtyRectCount() != 0 ) {
		xgeXuiWidgetFree(pChild);
		xgeXuiUnit(&tXui);
		return 425;
	}
	if ( xgeXuiWidgetAdd(pRoot, pChild) != XGE_OK ) {
		xgeXuiWidgetFree(pChild);
		xgeXuiUnit(&tXui);
		return 418;
	}
	if ( xgeXuiRefreshNeeded(&tXui) == 0 || tHostState.iRequestRefresh != 2 ) {
		xgeXuiUnit(&tXui);
		return 419;
	}
	xgeDirtyRectClear();
	xgeXuiWidgetMarkPaint(pChild);
	if ( tHostState.iRequestRefresh != 2 ) {
		xgeXuiUnit(&tXui);
		return 420;
	}
	if ( xgeDirtyRectCount() != 1 || xgeDirtyRectGet(0, &tDirty) != XGE_OK || tDirty.fX != 1.0f || tDirty.fY != 2.0f || tDirty.fW != 3.0f || tDirty.fH != 4.0f ) {
		xgeXuiUnit(&tXui);
		return 426;
	}
	xgeXuiRefreshClear(&tXui);
	xgeDirtyRectClear();
	xgeXuiLayoutBatchBegin(&tXui);
	xgeXuiWidgetSetRect(pChild, tRect);
	xgeXuiWidgetSetBackground(pChild, XGE_COLOR_RGBA(10, 20, 30, 255));
	if ( xgeXuiRefreshNeeded(&tXui) != 0 || tHostState.iRequestRefresh != 2 ) {
		xgeXuiUnit(&tXui);
		return 421;
	}
	if ( xgeDirtyRectCount() != 0 ) {
		xgeXuiUnit(&tXui);
		return 427;
	}
	xgeXuiLayoutBatchEnd(&tXui);
	if ( xgeXuiRefreshNeeded(&tXui) == 0 || tHostState.iRequestRefresh != 3 ) {
		xgeXuiUnit(&tXui);
		return 422;
	}
	if ( xgeDirtyRectCount() != 1 || xgeDirtyRectGet(0, &tDirty) != XGE_OK || tDirty.fX != 0.0f || tDirty.fY != 0.0f || tDirty.fW != 320.0f || tDirty.fH != 200.0f ) {
		xgeXuiUnit(&tXui);
		return 428;
	}
	xgeXuiUpdate(&tXui, 0.0f);
	if ( xgeXuiPaint(&tXui) != 1 || xgeXuiRefreshNeeded(&tXui) != 0 ) {
		xgeXuiUnit(&tXui);
		return 423;
	}
	xgeXuiRefreshRequest(&tXui);
	if ( xgeXuiRefreshNeeded(&tXui) == 0 || tHostState.iRequestRefresh != 4 ) {
		xgeXuiUnit(&tXui);
		return 424;
	}
	xgeXuiRefreshClear(&tXui);
	xgeXuiUnit(&tXui);
	return 0;
}

static int __testXuiTheme(void)
{
	xge_xui_context_t tXui;
	xge_xui_theme_t tTheme;
	xge_xui_style_t tStyle;
	xge_font_t tFont;
	xge_xui_button_t tButton;
	xge_xui_input_t tInput;
	xge_xui_toggle_t tToggle;
	xge_xui_slider_t tSlider;
	xge_xui_widget pButton;
	xge_xui_widget pInput;
	xge_xui_widget pToggle;
	xge_xui_widget pSlider;

	memset(&tXui, 0, sizeof(tXui));
	memset(&tTheme, 0, sizeof(tTheme));
	memset(&tStyle, 0, sizeof(tStyle));
	memset(&tFont, 0, sizeof(tFont));
	memset(&tButton, 0, sizeof(tButton));
	memset(&tInput, 0, sizeof(tInput));
	memset(&tToggle, 0, sizeof(tToggle));
	memset(&tSlider, 0, sizeof(tSlider));
	xgeXuiThemeDefault(&tTheme);
	if ( (tTheme.iStateNormal == 0) || (tTheme.iAccentColor == 0) || (tTheme.fRadius <= 0.0f) || (tTheme.fPadding <= 0.0f) || (tTheme.fSpacing <= 0.0f) || (tTheme.fBorderWidth <= 0.0f) ) {
		return 398;
	}
	if ( xgeXuiInit(&tXui) != XGE_OK ) {
		return 399;
	}
	if ( xgeXuiGetTheme(&tXui) != &tXui.tTheme ) {
		xgeXuiUnit(&tXui);
		return 400;
	}
	tTheme.pFont = &tFont;
	tTheme.iTextColor = XGE_COLOR_RGBA(1, 2, 3, 255);
	tTheme.iBackgroundColor = XGE_COLOR_RGBA(4, 5, 6, 255);
	tTheme.iBorderColor = XGE_COLOR_RGBA(7, 8, 9, 255);
	tTheme.iAccentColor = XGE_COLOR_RGBA(10, 11, 12, 255);
	tTheme.iSelectionColor = XGE_COLOR_RGBA(13, 14, 15, 160);
	tTheme.iStateNormal = XGE_COLOR_RGBA(16, 17, 18, 255);
	tTheme.iStateHover = XGE_COLOR_RGBA(19, 20, 21, 255);
	tTheme.iStateActive = XGE_COLOR_RGBA(22, 23, 24, 255);
	tTheme.iStateFocus = XGE_COLOR_RGBA(25, 26, 27, 255);
	tTheme.iStateDisabled = XGE_COLOR_RGBA(28, 29, 30, 160);
	tTheme.fRadius = 9.0f;
	xgeXuiSetTheme(&tXui, &tTheme);
	if ( xgeXuiGetTheme(&tXui)->iAccentColor != tTheme.iAccentColor ) {
		xgeXuiUnit(&tXui);
		return 401;
	}
	pButton = xgeXuiWidgetCreate();
	pInput = xgeXuiWidgetCreate();
	pToggle = xgeXuiWidgetCreate();
	pSlider = xgeXuiWidgetCreate();
	if ( (pButton == NULL) || (pInput == NULL) || (pToggle == NULL) || (pSlider == NULL) ) {
		xgeXuiWidgetFree(pButton);
		xgeXuiWidgetFree(pInput);
		xgeXuiWidgetFree(pToggle);
		xgeXuiWidgetFree(pSlider);
		xgeXuiUnit(&tXui);
		return 402;
	}
	if ( xgeXuiButtonInit(&tButton, &tXui, pButton) != XGE_OK ) {
		xgeXuiWidgetFree(pButton);
		xgeXuiWidgetFree(pInput);
		xgeXuiWidgetFree(pToggle);
		xgeXuiWidgetFree(pSlider);
		xgeXuiUnit(&tXui);
		return 403;
	}
	if ( (tButton.pFont != &tFont) || (tButton.iTextColor != tTheme.iTextColor) || (tButton.iColorNormal != tTheme.iStateNormal) || (tButton.iColorHover != tTheme.iStateHover) || (pButton->tStyle.fRadius != tTheme.fRadius) ) {
		xgeXuiButtonUnit(&tButton);
		xgeXuiWidgetFree(pButton);
		xgeXuiWidgetFree(pInput);
		xgeXuiWidgetFree(pToggle);
		xgeXuiWidgetFree(pSlider);
		xgeXuiUnit(&tXui);
		return 404;
	}
	if ( xgeXuiInputInit(&tInput, &tXui, pInput, NULL) != XGE_OK ) {
		xgeXuiButtonUnit(&tButton);
		xgeXuiWidgetFree(pButton);
		xgeXuiWidgetFree(pInput);
		xgeXuiWidgetFree(pToggle);
		xgeXuiWidgetFree(pSlider);
		xgeXuiUnit(&tXui);
		return 405;
	}
	if ( (tInput.pFont != &tFont) || (tInput.iBackgroundColor != tTheme.iBackgroundColor) || (tInput.iFocusColor != tTheme.iStateFocus) || (tInput.iSelectionColor != tTheme.iSelectionColor) ) {
		xgeXuiInputUnit(&tInput);
		xgeXuiButtonUnit(&tButton);
		xgeXuiWidgetFree(pButton);
		xgeXuiWidgetFree(pInput);
		xgeXuiWidgetFree(pToggle);
		xgeXuiWidgetFree(pSlider);
		xgeXuiUnit(&tXui);
		return 406;
	}
	if ( xgeXuiToggleInit(&tToggle, &tXui, pToggle) != XGE_OK || xgeXuiSliderInit(&tSlider, &tXui, pSlider) != XGE_OK ) {
		xgeXuiInputUnit(&tInput);
		xgeXuiButtonUnit(&tButton);
		xgeXuiWidgetFree(pButton);
		xgeXuiWidgetFree(pInput);
		xgeXuiWidgetFree(pToggle);
		xgeXuiWidgetFree(pSlider);
		xgeXuiUnit(&tXui);
		return 407;
	}
	if ( (tToggle.iColorChecked != tTheme.iAccentColor) || (tSlider.iColorTrack != tTheme.iBorderColor) || (tSlider.iColorFill != tTheme.iAccentColor) || (tSlider.iColorKnob != tTheme.iBackgroundColor) ) {
		xgeXuiSliderUnit(&tSlider);
		xgeXuiToggleUnit(&tToggle);
		xgeXuiInputUnit(&tInput);
		xgeXuiButtonUnit(&tButton);
		xgeXuiWidgetFree(pButton);
		xgeXuiWidgetFree(pInput);
		xgeXuiWidgetFree(pToggle);
		xgeXuiWidgetFree(pSlider);
		xgeXuiUnit(&tXui);
		return 408;
	}
	xgeXuiStyleFromTheme(&tStyle, &tTheme);
	tStyle.iLayout = XGE_XUI_LAYOUT_ROW;
	tStyle.iBackgroundColor = XGE_COLOR_RGBA(31, 32, 33, 255);
	tStyle.fRadius = 12.0f;
	tStyle.tPadding.tLeft = xgeXuiSizePx(6.0f);
	tStyle.tPadding.tTop = xgeXuiSizePx(7.0f);
	tStyle.tPadding.tRight = xgeXuiSizePx(8.0f);
	tStyle.tPadding.tBottom = xgeXuiSizePx(9.0f);
	xgeXuiWidgetSetStyle(pButton, &tStyle);
	if ( (xgeXuiWidgetGetStyle(pButton) != &pButton->tStyle) || (pButton->tStyle.iLayout != XGE_XUI_LAYOUT_ROW) || (pButton->tStyle.iBackgroundColor != XGE_COLOR_RGBA(31, 32, 33, 255)) || (pButton->tStyle.fRadius != 12.0f) ) {
		xgeXuiSliderUnit(&tSlider);
		xgeXuiToggleUnit(&tToggle);
		xgeXuiInputUnit(&tInput);
		xgeXuiButtonUnit(&tButton);
		xgeXuiWidgetFree(pButton);
		xgeXuiWidgetFree(pInput);
		xgeXuiWidgetFree(pToggle);
		xgeXuiWidgetFree(pSlider);
		xgeXuiUnit(&tXui);
		return 409;
	}
	if ( (pButton->tStyle.tPadding.tLeft.fValue != 6.0f) || ((xgeXuiWidgetGetFlags(pButton) & XGE_XUI_WIDGET_DIRTY_LAYOUT) == 0) || ((xgeXuiWidgetGetFlags(pButton) & XGE_XUI_WIDGET_DIRTY_PAINT) == 0) ) {
		xgeXuiSliderUnit(&tSlider);
		xgeXuiToggleUnit(&tToggle);
		xgeXuiInputUnit(&tInput);
		xgeXuiButtonUnit(&tButton);
		xgeXuiWidgetFree(pButton);
		xgeXuiWidgetFree(pInput);
		xgeXuiWidgetFree(pToggle);
		xgeXuiWidgetFree(pSlider);
		xgeXuiUnit(&tXui);
		return 410;
	}
	xgeXuiSliderUnit(&tSlider);
	xgeXuiToggleUnit(&tToggle);
	xgeXuiInputUnit(&tInput);
	xgeXuiButtonUnit(&tButton);
	xgeXuiWidgetFree(pButton);
	xgeXuiWidgetFree(pInput);
	xgeXuiWidgetFree(pToggle);
	xgeXuiWidgetFree(pSlider);
	xgeXuiUnit(&tXui);
	return 0;
}

static int __testXuiLayoutModes(void)
{
	xge_xui_context_t tXui;
	xge_xui_widget pRoot;
	xge_xui_widget pPanel;
	xge_xui_widget pA;
	xge_xui_widget pB;
	xge_xui_widget pC;
	xge_rect_t tRect;

	memset(&tXui, 0, sizeof(tXui));
	if ( xgeXuiInit(&tXui) != XGE_OK ) {
		return 140;
	}
	pRoot = xgeXuiRoot(&tXui);
	pPanel = xgeXuiWidgetCreate();
	pA = xgeXuiWidgetCreate();
	pB = xgeXuiWidgetCreate();
	pC = NULL;
	if ( (pRoot == NULL) || (pPanel == NULL) || (pA == NULL) || (pB == NULL) ) {
		xgeXuiWidgetFree(pPanel);
		xgeXuiWidgetFree(pA);
		xgeXuiWidgetFree(pB);
		xgeXuiUnit(&tXui);
		return 141;
	}

	tRect.fX = 0.0f;
	tRect.fY = 0.0f;
	tRect.fW = 300.0f;
	tRect.fH = 100.0f;
	xgeXuiWidgetSetRect(pPanel, tRect);
	xgeXuiWidgetSetLayout(pPanel, XGE_XUI_LAYOUT_ROW);
	xgeXuiWidgetSetPaddingPx(pPanel, 10.0f, 0.0f, 10.0f, 0.0f);
	xgeXuiWidgetSetSize(pA, xgeXuiSizePx(50.0f), xgeXuiSizePx(20.0f));
	xgeXuiWidgetSetSize(pB, xgeXuiSizeGrow(1.0f), xgeXuiSizePx(30.0f));
	xgeXuiWidgetSetMarginPx(pB, 5.0f, 0.0f, 0.0f, 0.0f);
	xgeXuiWidgetAdd(pRoot, pPanel);
	xgeXuiWidgetAdd(pPanel, pA);
	xgeXuiWidgetAdd(pPanel, pB);
	xgeXuiUpdate(&tXui, 0.0f);
	tRect = xgeXuiWidgetGetRect(pA);
	if ( tRect.fX != 10.0f || tRect.fW != 50.0f || tRect.fH != 20.0f ) {
		xgeXuiUnit(&tXui);
		return 142;
	}
	tRect = xgeXuiWidgetGetRect(pB);
	if ( tRect.fX != 65.0f || tRect.fW != 225.0f || tRect.fH != 30.0f ) {
		xgeXuiUnit(&tXui);
		return 143;
	}

	xgeXuiWidgetSetLayout(pPanel, XGE_XUI_LAYOUT_COLUMN);
	xgeXuiWidgetSetPaddingPx(pPanel, 0.0f, 10.0f, 0.0f, 10.0f);
	xgeXuiWidgetSetMarginPx(pB, 0.0f, 5.0f, 0.0f, 0.0f);
	xgeXuiWidgetSetSize(pA, xgeXuiSizePx(50.0f), xgeXuiSizePx(20.0f));
	xgeXuiWidgetSetSize(pB, xgeXuiSizePx(30.0f), xgeXuiSizeGrow(1.0f));
	xgeXuiUpdate(&tXui, 0.0f);
	tRect = xgeXuiWidgetGetRect(pB);
	if ( tRect.fY != 35.0f || tRect.fW != 30.0f || tRect.fH != 55.0f ) {
		xgeXuiUnit(&tXui);
		return 144;
	}

	xgeXuiWidgetSetLayout(pPanel, XGE_XUI_LAYOUT_STACK);
	xgeXuiWidgetSetPaddingPx(pPanel, 5.0f, 6.0f, 7.0f, 8.0f);
	xgeXuiWidgetSetMarginPx(pA, 1.0f, 2.0f, 3.0f, 4.0f);
	xgeXuiWidgetSetSize(pA, xgeXuiSizePercent(50.0f), xgeXuiSizePercent(50.0f));
	xgeXuiUpdate(&tXui, 0.0f);
	tRect = xgeXuiWidgetGetRect(pA);
	if ( tRect.fX != 6.0f || tRect.fY != 8.0f || tRect.fW != 144.0f || tRect.fH != 43.0f ) {
		xgeXuiUnit(&tXui);
		return 145;
	}

	xgeXuiWidgetSetLayout(pPanel, XGE_XUI_LAYOUT_ABSOLUTE);
	xgeXuiWidgetSetPaddingPx(pPanel, 0.0f, 0.0f, 0.0f, 0.0f);
	xgeXuiWidgetSetMarginPx(pA, 0.0f, 0.0f, 0.0f, 0.0f);
	xgeXuiWidgetSetSize(pA, xgeXuiSizePx(10.0f), xgeXuiSizePx(100.0f));
	xgeXuiWidgetSetMinSize(pA, xgeXuiSizePx(30.0f), xgeXuiSizePx(0.0f));
	xgeXuiWidgetSetMaxSize(pA, xgeXuiSizePx(0.0f), xgeXuiSizePx(40.0f));
	xgeXuiUpdate(&tXui, 0.0f);
	tRect = xgeXuiWidgetGetRect(pA);
	if ( tRect.fW != 30.0f || tRect.fH != 40.0f ) {
		xgeXuiUnit(&tXui);
		return 150;
	}
	xgeXuiWidgetSetRect(pA, pPanel->tContentRect);
	xgeXuiWidgetSetSize(pA, xgeXuiSizePx(30.0f), xgeXuiSizePx(20.0f));
	xgeXuiWidgetSetMinSize(pA, xgeXuiSizePx(0.0f), xgeXuiSizePx(0.0f));
	xgeXuiWidgetSetMaxSize(pA, xgeXuiSizePx(0.0f), xgeXuiSizePx(0.0f));
	xgeXuiWidgetSetAlign(pA, XGE_XUI_ALIGN_CENTER, XGE_XUI_ALIGN_END);
	xgeXuiUpdate(&tXui, 0.0f);
	tRect = xgeXuiWidgetGetRect(pA);
	if ( tRect.fX != 135.0f || tRect.fY != 80.0f || tRect.fW != 30.0f || tRect.fH != 20.0f ) {
		xgeXuiUnit(&tXui);
		return 151;
	}
	xgeXuiWidgetSetAlign(pA, XGE_XUI_ALIGN_STRETCH, XGE_XUI_ALIGN_STRETCH);
	xgeXuiWidgetSetAnchorPx(pA, XGE_XUI_ANCHOR_LEFT | XGE_XUI_ANCHOR_TOP | XGE_XUI_ANCHOR_RIGHT | XGE_XUI_ANCHOR_BOTTOM, 10.0f, 11.0f, 12.0f, 13.0f);
	xgeXuiUpdate(&tXui, 0.0f);
	tRect = xgeXuiWidgetGetRect(pA);
	if ( tRect.fX != 10.0f || tRect.fY != 11.0f || tRect.fW != 278.0f || tRect.fH != 76.0f || xgeXuiWidgetGetAnchor(pA) != (XGE_XUI_ANCHOR_LEFT | XGE_XUI_ANCHOR_TOP | XGE_XUI_ANCHOR_RIGHT | XGE_XUI_ANCHOR_BOTTOM) ) {
		xgeXuiUnit(&tXui);
		return 152;
	}
	xgeXuiWidgetSetAnchorPx(pA, XGE_XUI_ANCHOR_RIGHT | XGE_XUI_ANCHOR_BOTTOM, 0.0f, 0.0f, 7.0f, 8.0f);
	xgeXuiWidgetSetRect(pA, pPanel->tContentRect);
	xgeXuiWidgetSetSize(pA, xgeXuiSizePx(30.0f), xgeXuiSizePx(20.0f));
	xgeXuiUpdate(&tXui, 0.0f);
	tRect = xgeXuiWidgetGetRect(pA);
	if ( tRect.fX != 263.0f || tRect.fY != 72.0f || tRect.fW != 30.0f || tRect.fH != 20.0f ) {
		xgeXuiUnit(&tXui);
		return 153;
	}
	xgeXuiWidgetSetAnchorPx(pA, 0, 0.0f, 0.0f, 0.0f, 0.0f);
	xgeXuiSetDipScale(&tXui, 2.0f);
	tRect.fX = 0.0f;
	tRect.fY = 0.0f;
	tRect.fW = 0.0f;
	tRect.fH = 0.0f;
	xgeXuiWidgetSetRect(pA, tRect);
	xgeXuiWidgetSetSize(pA, xgeXuiSizeDip(15.0f), xgeXuiSizeDip(10.0f));
	xgeXuiUpdate(&tXui, 0.0f);
	tRect = xgeXuiWidgetGetRect(pA);
	if ( xgeXuiGetDipScale(&tXui) != 2.0f || tRect.fW != 30.0f || tRect.fH != 20.0f ) {
		xgeXuiUnit(&tXui);
		return 154;
	}
	xgeXuiSetDipScale(&tXui, 0.0f);
	xgeXuiUpdate(&tXui, 0.0f);
	tRect = xgeXuiWidgetGetRect(pA);
	if ( xgeXuiGetDipScale(&tXui) != 1.0f || tRect.fW != 15.0f || tRect.fH != 10.0f ) {
		xgeXuiUnit(&tXui);
		return 155;
	}

	pC = xgeXuiWidgetCreate();
	if ( pC == NULL ) {
		xgeXuiUnit(&tXui);
		return 146;
	}
	xgeXuiWidgetAdd(pPanel, pC);
	xgeXuiWidgetSetLayout(pPanel, XGE_XUI_LAYOUT_GRID);
	xgeXuiWidgetSetGrid(pPanel, 2, 20.0f, 10.0f, 4.0f);
	xgeXuiWidgetSetPaddingPx(pPanel, 10.0f, 10.0f, 10.0f, 10.0f);
	xgeXuiWidgetSetMarginPx(pA, 0.0f, 0.0f, 0.0f, 0.0f);
	xgeXuiWidgetSetMarginPx(pB, 1.0f, 2.0f, 3.0f, 4.0f);
	xgeXuiWidgetSetMarginPx(pC, 0.0f, 0.0f, 0.0f, 0.0f);
	xgeXuiWidgetSetAlign(pA, XGE_XUI_ALIGN_STRETCH, XGE_XUI_ALIGN_STRETCH);
	xgeXuiWidgetSetAlign(pB, XGE_XUI_ALIGN_STRETCH, XGE_XUI_ALIGN_STRETCH);
	xgeXuiWidgetSetAlign(pC, XGE_XUI_ALIGN_STRETCH, XGE_XUI_ALIGN_STRETCH);
	xgeXuiWidgetSetSize(pA, xgeXuiSizeGrow(1.0f), xgeXuiSizeGrow(1.0f));
	xgeXuiWidgetSetSize(pB, xgeXuiSizeGrow(1.0f), xgeXuiSizeGrow(1.0f));
	xgeXuiWidgetSetSize(pC, xgeXuiSizeGrow(1.0f), xgeXuiSizeGrow(1.0f));
	xgeXuiUpdate(&tXui, 0.0f);
	tRect = xgeXuiWidgetGetRect(pA);
	if ( tRect.fX != 10.0f || tRect.fY != 10.0f || tRect.fW != 135.0f || tRect.fH != 20.0f ) {
		xgeXuiUnit(&tXui);
		return 147;
	}
	tRect = xgeXuiWidgetGetRect(pB);
	if ( tRect.fX != 156.0f || tRect.fY != 12.0f || tRect.fW != 131.0f || tRect.fH != 14.0f ) {
		xgeXuiUnit(&tXui);
		return 148;
	}
	tRect = xgeXuiWidgetGetRect(pC);
	if ( tRect.fX != 10.0f || tRect.fY != 34.0f || tRect.fW != 135.0f || tRect.fH != 20.0f ) {
		xgeXuiUnit(&tXui);
		return 149;
	}

	xgeXuiUnit(&tXui);
	return 0;
}

static int g_iXuiEventLog;

static int __testXuiEventChild(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser)
{
	(void)pWidget;
	(void)pUser;
	if ( pEvent->iType == XGE_EVENT_MOUSE_DOWN ) {
		g_iXuiEventLog = (g_iXuiEventLog * 10) + 1;
		return XGE_XUI_EVENT_CONTINUE;
	}
	if ( pEvent->iType == XGE_EVENT_KEY_DOWN ) {
		g_iXuiEventLog = (g_iXuiEventLog * 10) + 3;
		return XGE_XUI_EVENT_CONSUMED;
	}
	if ( pEvent->iType == XGE_EVENT_XUI_FOCUS_IN ) {
		g_iXuiEventLog = (g_iXuiEventLog * 10) + 4;
		return XGE_XUI_EVENT_CONTINUE;
	}
	if ( pEvent->iType == XGE_EVENT_XUI_FOCUS_OUT ) {
		g_iXuiEventLog = (g_iXuiEventLog * 10) + 5;
		return XGE_XUI_EVENT_CONTINUE;
	}
	return XGE_XUI_EVENT_CONTINUE;
}

static int __testXuiEventParent(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser)
{
	(void)pWidget;
	(void)pUser;
	if ( pEvent->iType == XGE_EVENT_MOUSE_DOWN ) {
		g_iXuiEventLog = (g_iXuiEventLog * 10) + 2;
		return XGE_XUI_EVENT_CONSUMED;
	}
	if ( pEvent->iType == XGE_EVENT_XUI_FOCUS_IN ) {
		g_iXuiEventLog = (g_iXuiEventLog * 10) + 6;
		return XGE_XUI_EVENT_CONTINUE;
	}
	if ( pEvent->iType == XGE_EVENT_XUI_FOCUS_OUT ) {
		g_iXuiEventLog = (g_iXuiEventLog * 10) + 8;
		return XGE_XUI_EVENT_CONTINUE;
	}
	return XGE_XUI_EVENT_CONTINUE;
}

static int __testXuiEventCapture(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser)
{
	(void)pWidget;
	(void)pUser;
	if ( pEvent->iType == XGE_EVENT_MOUSE_MOVE ) {
		g_iXuiEventLog = 7;
		return XGE_XUI_EVENT_CONSUMED;
	}
	return XGE_XUI_EVENT_CONTINUE;
}

static int __testXuiEventCaptureRoot(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser)
{
	(void)pWidget;
	(void)pUser;
	if ( (pEvent->iType == XGE_EVENT_MOUSE_DOWN) || (pEvent->iType == XGE_EVENT_MOUSE_UP) ) {
		g_iXuiEventLog = (g_iXuiEventLog * 10) + 8;
		return XGE_XUI_EVENT_CONTINUE;
	}
	return XGE_XUI_EVENT_CONTINUE;
}

static int __testXuiEventCaptureParent(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser)
{
	(void)pWidget;
	(void)pUser;
	if ( pEvent->iType == XGE_EVENT_MOUSE_DOWN ) {
		g_iXuiEventLog = (g_iXuiEventLog * 10) + 9;
		return XGE_XUI_EVENT_CONTINUE;
	}
	if ( pEvent->iType == XGE_EVENT_MOUSE_UP ) {
		g_iXuiEventLog = (g_iXuiEventLog * 10) + 9;
		return XGE_XUI_EVENT_CONSUMED;
	}
	return XGE_XUI_EVENT_CONTINUE;
}

static int __testXuiEvents(void)
{
	xge_xui_context_t tXui;
	xge_xui_widget pRoot;
	xge_xui_widget pParent;
	xge_xui_widget pChild;
	xge_xui_widget pOverlay;
	xge_xui_widget pHit;
	xge_event_t tEvent;
	xge_event_t tPoppedEvent;
	xge_rect_t tRect;
	int iProcessed;

	memset(&tXui, 0, sizeof(tXui));
	if ( xgeXuiInit(&tXui) != XGE_OK ) {
		return 160;
	}
	pRoot = xgeXuiRoot(&tXui);
	pParent = xgeXuiWidgetCreate();
	pChild = xgeXuiWidgetCreate();
	pOverlay = xgeXuiWidgetCreate();
	if ( (pRoot == NULL) || (pParent == NULL) || (pChild == NULL) || (pOverlay == NULL) ) {
		xgeXuiWidgetFree(pParent);
		xgeXuiWidgetFree(pChild);
		xgeXuiWidgetFree(pOverlay);
		xgeXuiUnit(&tXui);
		return 161;
	}
	tRect.fX = 0.0f;
	tRect.fY = 0.0f;
	tRect.fW = 200.0f;
	tRect.fH = 200.0f;
	xgeXuiWidgetSetRect(pParent, tRect);
	tRect.fX = 10.0f;
	tRect.fY = 10.0f;
	tRect.fW = 50.0f;
	tRect.fH = 50.0f;
	xgeXuiWidgetSetRect(pChild, tRect);
	xgeXuiWidgetSetRect(pOverlay, tRect);
	xgeXuiWidgetSetZ(pOverlay, -1);
	pParent->procEvent = __testXuiEventParent;
	pChild->procEvent = __testXuiEventChild;
	xgeXuiWidgetAdd(pRoot, pParent);
	xgeXuiWidgetAdd(pParent, pChild);
	xgeXuiWidgetAdd(pParent, pOverlay);
	xgeXuiUpdate(&tXui, 0.0f);

	pHit = xgeXuiHitTest(&tXui, 20.0f, 20.0f);
	if ( pHit != pChild ) {
		xgeXuiUnit(&tXui);
		return 162;
	}
	xgeXuiWidgetSetZ(pOverlay, 10);
	pHit = xgeXuiHitTest(&tXui, 20.0f, 20.0f);
	if ( (pHit != pOverlay) || (xgeXuiWidgetGetZ(pOverlay) != 10) ) {
		xgeXuiUnit(&tXui);
		return 169;
	}
	xgeXuiWidgetSetZ(pOverlay, -1);
	xgeXuiWidgetSetVisible(pOverlay, 0);
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_MOUSE_DOWN;
	tEvent.fX = 20.0f;
	tEvent.fY = 20.0f;
	g_iXuiEventLog = 0;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || g_iXuiEventLog != 12 ) {
		xgeXuiUnit(&tXui);
		return 163;
	}

	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_KEY_DOWN;
	xgeXuiWidgetSetFocusable(pChild, 1);
	xgeXuiSetFocus(&tXui, pChild);
	g_iXuiEventLog = 0;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || g_iXuiEventLog != 3 ) {
		xgeXuiUnit(&tXui);
		return 164;
	}
	xgeXuiWidgetSetFocusable(pParent, 1);
	g_iXuiEventLog = 0;
	xgeXuiSetFocus(&tXui, pParent);
	if ( tXui.pFocus != pParent || g_iXuiEventLog != 56 ) {
		xgeXuiUnit(&tXui);
		return 170;
	}
	g_iXuiEventLog = 0;
	xgeXuiSetFocus(&tXui, NULL);
	if ( tXui.pFocus != NULL || g_iXuiEventLog != 8 ) {
		xgeXuiUnit(&tXui);
		return 171;
	}

	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_MOUSE_DOWN;
	tEvent.fX = 20.0f;
	tEvent.fY = 20.0f;
	if ( xgeXuiEventPush(&tXui, &tEvent) != XGE_OK || xgeXuiEventCount(&tXui) != 1 ) {
		xgeXuiUnit(&tXui);
		return 172;
	}
	memset(&tPoppedEvent, 0, sizeof(tPoppedEvent));
	if ( xgeXuiEventPop(&tXui, &tPoppedEvent) != XGE_OK || xgeXuiEventCount(&tXui) != 0 || tPoppedEvent.iType != XGE_EVENT_MOUSE_DOWN ) {
		xgeXuiUnit(&tXui);
		return 173;
	}
	g_iXuiEventLog = 0;
	if ( xgeXuiDispatchEvent(&tXui, &tPoppedEvent) != XGE_XUI_EVENT_CONSUMED || g_iXuiEventLog != 12 ) {
		xgeXuiUnit(&tXui);
		return 174;
	}
	if ( xgeXuiEventPush(&tXui, &tEvent) != XGE_OK ) {
		xgeXuiUnit(&tXui);
		return 175;
	}
	tEvent.fX = 500.0f;
	tEvent.fY = 500.0f;
	if ( xgeXuiEventPush(&tXui, &tEvent) != XGE_OK || xgeXuiEventCount(&tXui) != 2 ) {
		xgeXuiUnit(&tXui);
		return 176;
	}
	g_iXuiEventLog = 0;
	iProcessed = xgeXuiDispatchQueuedEvents(&tXui);
	if ( iProcessed != 2 || xgeXuiEventCount(&tXui) != 0 || g_iXuiEventLog != 12 ) {
		xgeXuiUnit(&tXui);
		return 177;
	}

	xgeXuiWidgetSetCaptureEvent(pRoot, __testXuiEventCaptureRoot);
	xgeXuiWidgetSetCaptureEvent(pParent, __testXuiEventCaptureParent);
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_MOUSE_DOWN;
	tEvent.fX = 20.0f;
	tEvent.fY = 20.0f;
	g_iXuiEventLog = 0;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || g_iXuiEventLog != 8912 ) {
		xgeXuiUnit(&tXui);
		return 178;
	}
	tEvent.iType = XGE_EVENT_MOUSE_UP;
	g_iXuiEventLog = 0;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || g_iXuiEventLog != 89 ) {
		xgeXuiUnit(&tXui);
		return 179;
	}
	xgeXuiWidgetSetCaptureEvent(pRoot, NULL);
	xgeXuiWidgetSetCaptureEvent(pParent, NULL);

	pParent->procEvent = __testXuiEventCapture;
	xgeXuiSetCapture(&tXui, pParent);
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_MOUSE_MOVE;
	tEvent.fX = 500.0f;
	tEvent.fY = 500.0f;
	g_iXuiEventLog = 0;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || g_iXuiEventLog != 7 ) {
		xgeXuiUnit(&tXui);
		return 165;
	}

	xgeXuiSetCapture(&tXui, NULL);
	xgeXuiSetFocus(&tXui, NULL);
	xgeXuiWidgetSetEnabled(pChild, 0);
	pHit = xgeXuiHitTest(&tXui, 20.0f, 20.0f);
	if ( pHit != pParent ) {
		xgeXuiUnit(&tXui);
		return 166;
	}
	xgeXuiWidgetSetEnabled(pChild, 1);
	xgeXuiWidgetSetVisible(pChild, 0);
	pHit = xgeXuiHitTest(&tXui, 20.0f, 20.0f);
	if ( pHit != pParent ) {
		xgeXuiUnit(&tXui);
		return 167;
	}
	xgeXuiWidgetSetVisible(pChild, 1);
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_MOUSE_DOWN;
	tEvent.fX = 500.0f;
	tEvent.fY = 500.0f;
	g_iXuiEventLog = 0;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONTINUE || g_iXuiEventLog != 0 ) {
		xgeXuiUnit(&tXui);
		return 168;
	}

	xgeXuiUnit(&tXui);
	return 0;
}

static int g_iXuiButtonClicks;

static void __testXuiButtonClick(xge_xui_widget pWidget, void* pUser)
{
	(void)pWidget;
	(void)pUser;
	g_iXuiButtonClicks++;
}

static int __testXuiButton(void)
{
	xrf_test_blob2_t tBlob;
	xge_font_t tFont;
	xge_xui_context_t tXui;
	xge_xui_button_t tButton;
	xge_xui_widget pRoot;
	xge_xui_widget pWidget;
	xge_event_t tEvent;
	xge_rect_t tRect;
	int iPaintCount;

	__testXrfBlobMake(&tBlob);
	memset(&tFont, 0, sizeof(tFont));
	memset(&tXui, 0, sizeof(tXui));
	memset(&tButton, 0, sizeof(tButton));
	if ( xgeFontLoadXRFMemory(&tFont, &tBlob, sizeof(tBlob)) != XGE_OK ) {
		return 200;
	}
	if ( xgeXuiInit(&tXui) != XGE_OK ) {
		xgeFontFree(&tFont);
		return 201;
	}
	pRoot = xgeXuiRoot(&tXui);
	pWidget = xgeXuiWidgetCreate();
	if ( (pRoot == NULL) || (pWidget == NULL) ) {
		xgeXuiWidgetFree(pWidget);
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 202;
	}
	tRect.fX = 10.0f;
	tRect.fY = 10.0f;
	tRect.fW = 100.0f;
	tRect.fH = 40.0f;
	xgeXuiWidgetSetRect(pWidget, tRect);
	xgeXuiWidgetAdd(pRoot, pWidget);
	xgeXuiUpdate(&tXui, 0.0f);
	if ( xgeXuiButtonInit(&tButton, &tXui, pWidget) != XGE_OK ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 203;
	}
	xgeXuiButtonSetClick(&tButton, __testXuiButtonClick, NULL);
	xgeXuiButtonSetText(&tButton, &tFont, "A");
	xgeXuiButtonSetTextColor(&tButton, XGE_COLOR_RGBA(10, 20, 30, 255));
	xgeXuiButtonSetColors(&tButton, XGE_COLOR_RGBA(1, 2, 3, 255), XGE_COLOR_RGBA(4, 5, 6, 255), XGE_COLOR_RGBA(7, 8, 9, 255), XGE_COLOR_RGBA(10, 11, 12, 255), XGE_COLOR_RGBA(13, 14, 15, 160));
	if ( xgeXuiWidgetIsFocusable(pWidget) == 0 ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 204;
	}
	iPaintCount = xgeXuiPaint(&tXui);
	if ( iPaintCount != 1 || tXui.iPaintCommandCount != 1 || pWidget->procPaint != xgeXuiButtonPaintProc ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 205;
	}

	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_MOUSE_MOVE;
	tEvent.fX = 20.0f;
	tEvent.fY = 20.0f;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONTINUE || ((xgeXuiButtonGetState(&tButton) & XGE_XUI_STATE_HOVER) == 0) ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 206;
	}
	tEvent.iType = XGE_EVENT_MOUSE_DOWN;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || ((xgeXuiButtonGetState(&tButton) & XGE_XUI_STATE_ACTIVE) == 0) ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 207;
	}
	if ( tXui.pFocus != pWidget || tXui.pCapture != pWidget ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 208;
	}
	g_iXuiButtonClicks = 0;
	tEvent.iType = XGE_EVENT_MOUSE_UP;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || g_iXuiButtonClicks != 1 || tButton.iClickCount != 1 ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 209;
	}
	if ( tXui.pCapture != NULL ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 210;
	}

	xgeXuiWidgetSetEnabled(pWidget, 0);
	tEvent.iType = XGE_EVENT_MOUSE_DOWN;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONTINUE || ((xgeXuiButtonGetState(&tButton) & XGE_XUI_STATE_DISABLED) == 0) ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 211;
	}

	xgeXuiButtonUnit(&tButton);
	if ( pWidget->procEvent != NULL || pWidget->procPaint != NULL || pWidget->pUser != NULL ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 212;
	}
	xgeXuiUnit(&tXui);
	xgeFontFree(&tFont);
	return 0;
}

static int g_iXuiToggleChanges;
static int g_iXuiToggleLastChecked;

static void __testXuiToggleChange(xge_xui_widget pWidget, int bChecked, void* pUser)
{
	(void)pWidget;
	(void)pUser;
	g_iXuiToggleChanges++;
	g_iXuiToggleLastChecked = bChecked;
}

static int __testXuiToggle(void)
{
	xrf_test_blob2_t tBlob;
	xge_font_t tFont;
	xge_xui_context_t tXui;
	xge_xui_toggle_t tToggle;
	xge_xui_widget pRoot;
	xge_xui_widget pWidget;
	xge_event_t tEvent;
	xge_rect_t tRect;
	int iPaintCount;

	__testXrfBlobMake(&tBlob);
	memset(&tFont, 0, sizeof(tFont));
	memset(&tXui, 0, sizeof(tXui));
	memset(&tToggle, 0, sizeof(tToggle));
	if ( xgeFontLoadXRFMemory(&tFont, &tBlob, sizeof(tBlob)) != XGE_OK ) {
		return 260;
	}
	if ( xgeXuiInit(&tXui) != XGE_OK ) {
		xgeFontFree(&tFont);
		return 261;
	}
	pRoot = xgeXuiRoot(&tXui);
	pWidget = xgeXuiWidgetCreate();
	if ( (pRoot == NULL) || (pWidget == NULL) ) {
		xgeXuiWidgetFree(pWidget);
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 262;
	}
	tRect.fX = 10.0f;
	tRect.fY = 10.0f;
	tRect.fW = 120.0f;
	tRect.fH = 32.0f;
	xgeXuiWidgetSetRect(pWidget, tRect);
	xgeXuiWidgetSetPaddingPx(pWidget, 3.0f, 3.0f, 3.0f, 3.0f);
	xgeXuiWidgetAdd(pRoot, pWidget);
	xgeXuiUpdate(&tXui, 0.0f);
	if ( xgeXuiToggleInit(&tToggle, &tXui, pWidget) != XGE_OK ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 263;
	}
	xgeXuiToggleSetChange(&tToggle, __testXuiToggleChange, NULL);
	xgeXuiToggleSetText(&tToggle, &tFont, "A");
	xgeXuiToggleSetTextColor(&tToggle, XGE_COLOR_RGBA(10, 20, 30, 255));
	xgeXuiToggleSetColors(&tToggle, XGE_COLOR_RGBA(1, 2, 3, 255), XGE_COLOR_RGBA(4, 5, 6, 255), XGE_COLOR_RGBA(7, 8, 9, 255), XGE_COLOR_RGBA(10, 11, 12, 255), XGE_COLOR_RGBA(13, 14, 15, 160), XGE_COLOR_RGBA(16, 17, 18, 255));
	if ( xgeXuiWidgetIsFocusable(pWidget) == 0 ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 264;
	}
	iPaintCount = xgeXuiPaint(&tXui);
	if ( iPaintCount != 1 || tXui.iPaintCommandCount != 1 || pWidget->procPaint != xgeXuiTogglePaintProc ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 265;
	}

	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_MOUSE_MOVE;
	tEvent.fX = 20.0f;
	tEvent.fY = 20.0f;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONTINUE || ((xgeXuiToggleGetState(&tToggle) & XGE_XUI_STATE_HOVER) == 0) ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 266;
	}
	tEvent.iType = XGE_EVENT_MOUSE_DOWN;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || ((xgeXuiToggleGetState(&tToggle) & XGE_XUI_STATE_ACTIVE) == 0) ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 267;
	}
	g_iXuiToggleChanges = 0;
	g_iXuiToggleLastChecked = 0;
	tEvent.iType = XGE_EVENT_MOUSE_UP;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || xgeXuiToggleGetChecked(&tToggle) != 1 || g_iXuiToggleChanges != 1 || g_iXuiToggleLastChecked != 1 || tToggle.iChangeCount != 1 ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 268;
	}
	xgeXuiToggleSetChecked(&tToggle, 0);
	if ( xgeXuiToggleGetChecked(&tToggle) != 0 ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 269;
	}
	xgeXuiWidgetSetEnabled(pWidget, 0);
	tEvent.iType = XGE_EVENT_MOUSE_DOWN;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONTINUE || ((xgeXuiToggleGetState(&tToggle) & XGE_XUI_STATE_DISABLED) == 0) ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 270;
	}
	xgeXuiToggleUnit(&tToggle);
	if ( pWidget->procEvent != NULL || pWidget->procPaint != NULL || pWidget->pUser != NULL ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 271;
	}
	xgeXuiUnit(&tXui);
	xgeFontFree(&tFont);
	return 0;
}

static int g_iXuiSliderChanges;
static float g_fXuiSliderLastValue;

static void __testXuiSliderChange(xge_xui_widget pWidget, float fValue, void* pUser)
{
	(void)pWidget;
	(void)pUser;
	g_iXuiSliderChanges++;
	g_fXuiSliderLastValue = fValue;
}

static int __testXuiSlider(void)
{
	xge_xui_context_t tXui;
	xge_xui_slider_t tSlider;
	xge_xui_widget pRoot;
	xge_xui_widget pWidget;
	xge_event_t tEvent;
	xge_rect_t tRect;
	int iPaintCount;

	memset(&tXui, 0, sizeof(tXui));
	memset(&tSlider, 0, sizeof(tSlider));
	if ( xgeXuiInit(&tXui) != XGE_OK ) {
		return 280;
	}
	pRoot = xgeXuiRoot(&tXui);
	pWidget = xgeXuiWidgetCreate();
	if ( (pRoot == NULL) || (pWidget == NULL) ) {
		xgeXuiWidgetFree(pWidget);
		xgeXuiUnit(&tXui);
		return 281;
	}
	tRect.fX = 10.0f;
	tRect.fY = 10.0f;
	tRect.fW = 110.0f;
	tRect.fH = 30.0f;
	xgeXuiWidgetSetRect(pWidget, tRect);
	xgeXuiWidgetSetPaddingPx(pWidget, 5.0f, 4.0f, 5.0f, 4.0f);
	xgeXuiWidgetAdd(pRoot, pWidget);
	xgeXuiUpdate(&tXui, 0.0f);
	if ( xgeXuiSliderInit(&tSlider, &tXui, pWidget) != XGE_OK ) {
		xgeXuiUnit(&tXui);
		return 282;
	}
	xgeXuiSliderSetChange(&tSlider, __testXuiSliderChange, NULL);
	xgeXuiSliderSetRange(&tSlider, 0.0f, 100.0f);
	xgeXuiSliderSetValue(&tSlider, 25.0f);
	xgeXuiSliderSetColors(&tSlider, XGE_COLOR_RGBA(1, 2, 3, 255), XGE_COLOR_RGBA(4, 5, 6, 255), XGE_COLOR_RGBA(7, 8, 9, 255), XGE_COLOR_RGBA(10, 11, 12, 255), XGE_COLOR_RGBA(13, 14, 15, 160));
	if ( xgeXuiSliderGetValue(&tSlider) != 25.0f || xgeXuiWidgetIsFocusable(pWidget) == 0 ) {
		xgeXuiUnit(&tXui);
		return 283;
	}
	iPaintCount = xgeXuiPaint(&tXui);
	if ( iPaintCount != 1 || tXui.iPaintCommandCount != 1 || pWidget->procPaint != xgeXuiSliderPaintProc ) {
		xgeXuiUnit(&tXui);
		return 284;
	}

	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_MOUSE_MOVE;
	tEvent.fX = 65.0f;
	tEvent.fY = 20.0f;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONTINUE || ((xgeXuiSliderGetState(&tSlider) & XGE_XUI_STATE_HOVER) == 0) ) {
		xgeXuiUnit(&tXui);
		return 285;
	}
	g_iXuiSliderChanges = 0;
	g_fXuiSliderLastValue = 0.0f;
	tEvent.iType = XGE_EVENT_MOUSE_DOWN;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || xgeXuiSliderGetValue(&tSlider) != 50.0f || g_iXuiSliderChanges != 1 || g_fXuiSliderLastValue != 50.0f ) {
		xgeXuiUnit(&tXui);
		return 286;
	}
	if ( tXui.pFocus != pWidget || tXui.pCapture != pWidget || ((xgeXuiSliderGetState(&tSlider) & XGE_XUI_STATE_ACTIVE) == 0) ) {
		xgeXuiUnit(&tXui);
		return 287;
	}
	tEvent.iType = XGE_EVENT_MOUSE_MOVE;
	tEvent.fX = 115.0f;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || xgeXuiSliderGetValue(&tSlider) != 100.0f || g_iXuiSliderChanges != 2 || g_fXuiSliderLastValue != 100.0f ) {
		xgeXuiUnit(&tXui);
		return 288;
	}
	tEvent.iType = XGE_EVENT_MOUSE_UP;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || tXui.pCapture != NULL ) {
		xgeXuiUnit(&tXui);
		return 289;
	}
	xgeXuiSliderSetRange(&tSlider, 100.0f, 0.0f);
	xgeXuiSliderSetValue(&tSlider, -20.0f);
	if ( xgeXuiSliderGetValue(&tSlider) != 0.0f ) {
		xgeXuiUnit(&tXui);
		return 290;
	}
	xgeXuiWidgetSetEnabled(pWidget, 0);
	tEvent.iType = XGE_EVENT_MOUSE_DOWN;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONTINUE || ((xgeXuiSliderGetState(&tSlider) & XGE_XUI_STATE_DISABLED) == 0) ) {
		xgeXuiUnit(&tXui);
		return 291;
	}
	xgeXuiSliderUnit(&tSlider);
	if ( pWidget->procEvent != NULL || pWidget->procPaint != NULL || pWidget->pUser != NULL ) {
		xgeXuiUnit(&tXui);
		return 292;
	}
	xgeXuiUnit(&tXui);
	return 0;
}

static int __testXuiProgress(void)
{
	xrf_test_blob2_t tBlob;
	xge_font_t tFont;
	xge_xui_context_t tXui;
	xge_xui_progress_t tProgress;
	xge_xui_widget pRoot;
	xge_xui_widget pWidget;
	xge_rect_t tRect;
	int iPaintCount;

	__testXrfBlobMake(&tBlob);
	memset(&tFont, 0, sizeof(tFont));
	memset(&tXui, 0, sizeof(tXui));
	memset(&tProgress, 0, sizeof(tProgress));
	if ( xgeFontLoadXRFMemory(&tFont, &tBlob, sizeof(tBlob)) != XGE_OK ) {
		return 300;
	}
	if ( xgeXuiInit(&tXui) != XGE_OK ) {
		xgeFontFree(&tFont);
		return 301;
	}
	pRoot = xgeXuiRoot(&tXui);
	pWidget = xgeXuiWidgetCreate();
	if ( (pRoot == NULL) || (pWidget == NULL) ) {
		xgeXuiWidgetFree(pWidget);
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 302;
	}
	tRect.fX = 10.0f;
	tRect.fY = 10.0f;
	tRect.fW = 120.0f;
	tRect.fH = 20.0f;
	xgeXuiWidgetSetRect(pWidget, tRect);
	xgeXuiWidgetSetPaddingPx(pWidget, 2.0f, 2.0f, 2.0f, 2.0f);
	xgeXuiWidgetAdd(pRoot, pWidget);
	xgeXuiUpdate(&tXui, 0.0f);
	if ( xgeXuiProgressInit(&tProgress, pWidget) != XGE_OK ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 303;
	}
	xgeXuiProgressSetRange(&tProgress, 100.0f, 0.0f);
	xgeXuiProgressSetValue(&tProgress, 60.0f);
	xgeXuiProgressSetColors(&tProgress, XGE_COLOR_RGBA(1, 2, 3, 255), XGE_COLOR_RGBA(4, 5, 6, 255));
	xgeXuiProgressSetText(&tProgress, &tFont, "A");
	xgeXuiProgressSetTextColor(&tProgress, XGE_COLOR_RGBA(7, 8, 9, 255));
	if ( xgeXuiProgressGetValue(&tProgress) != 60.0f || tProgress.fMin != 0.0f || tProgress.fMax != 100.0f ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 304;
	}
	xgeXuiProgressSetValue(&tProgress, 200.0f);
	if ( xgeXuiProgressGetValue(&tProgress) != 100.0f ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 305;
	}
	xgeXuiProgressSetValue(&tProgress, -10.0f);
	if ( xgeXuiProgressGetValue(&tProgress) != 0.0f ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 306;
	}
	iPaintCount = xgeXuiPaint(&tXui);
	if ( iPaintCount != 1 || tXui.iPaintCommandCount != 1 || pWidget->procPaint != xgeXuiProgressPaintProc || pWidget->procEvent != NULL ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 307;
	}
	if ( tProgress.pFont != &tFont || tProgress.iTextColor != XGE_COLOR_RGBA(7, 8, 9, 255) || tProgress.iColorFill != XGE_COLOR_RGBA(4, 5, 6, 255) ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 308;
	}
	xgeXuiProgressUnit(&tProgress);
	if ( pWidget->procPaint != NULL || pWidget->pUser != NULL ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 309;
	}
	xgeXuiUnit(&tXui);
	xgeFontFree(&tFont);
	return 0;
}

static int __testXuiPanel(void)
{
	xrf_test_blob2_t tBlob;
	xge_font_t tFont;
	xge_xui_context_t tXui;
	xge_xui_panel_t tPanel;
	xge_xui_widget pRoot;
	xge_xui_widget pWidget;
	xge_rect_t tRect;
	int iPaintCount;

	__testXrfBlobMake(&tBlob);
	memset(&tFont, 0, sizeof(tFont));
	memset(&tXui, 0, sizeof(tXui));
	memset(&tPanel, 0, sizeof(tPanel));
	if ( xgeFontLoadXRFMemory(&tFont, &tBlob, sizeof(tBlob)) != XGE_OK ) {
		return 320;
	}
	if ( xgeXuiInit(&tXui) != XGE_OK ) {
		xgeFontFree(&tFont);
		return 321;
	}
	pRoot = xgeXuiRoot(&tXui);
	pWidget = xgeXuiWidgetCreate();
	if ( (pRoot == NULL) || (pWidget == NULL) ) {
		xgeXuiWidgetFree(pWidget);
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 322;
	}
	tRect.fX = 4.0f;
	tRect.fY = 6.0f;
	tRect.fW = 120.0f;
	tRect.fH = 80.0f;
	xgeXuiWidgetSetRect(pWidget, tRect);
	xgeXuiWidgetSetPaddingPx(pWidget, 3.0f, 4.0f, 5.0f, 6.0f);
	xgeXuiWidgetAdd(pRoot, pWidget);
	xgeXuiUpdate(&tXui, 0.0f);
	if ( xgeXuiPanelInit(&tPanel, pWidget) != XGE_OK ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 323;
	}
	xgeXuiPanelSetBackground(&tPanel, XGE_COLOR_RGBA(1, 2, 3, 255));
	xgeXuiPanelSetTitle(&tPanel, &tFont, "A");
	xgeXuiPanelSetTitleColor(&tPanel, XGE_COLOR_RGBA(4, 5, 6, 255));
	xgeXuiPanelSetTitleAlign(&tPanel, XGE_TEXT_ALIGN_CENTER | XGE_TEXT_ALIGN_TOP);
	xgeXuiPanelSetClip(&tPanel, 1);
	if ( (xgeXuiWidgetGetFlags(pWidget) & XGE_XUI_WIDGET_CLIP) == 0 || tPanel.bClip != 1 ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 324;
	}
	iPaintCount = xgeXuiPaint(&tXui);
	if ( iPaintCount != 1 || tXui.iPaintCommandCount != 1 || pWidget->procPaint != xgeXuiPanelPaintProc || pWidget->procEvent != NULL ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 325;
	}
	if ( tPanel.pFont != &tFont || tPanel.iBackgroundColor != XGE_COLOR_RGBA(1, 2, 3, 255) || tPanel.iTitleColor != XGE_COLOR_RGBA(4, 5, 6, 255) || ((tPanel.iTitleFlags & XGE_TEXT_CLIP) == 0) ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 326;
	}
	xgeXuiPanelSetClip(&tPanel, 0);
	if ( (xgeXuiWidgetGetFlags(pWidget) & XGE_XUI_WIDGET_CLIP) != 0 || tPanel.bClip != 0 ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 327;
	}
	xgeXuiPanelUnit(&tPanel);
	if ( pWidget->procPaint != NULL || pWidget->pUser != NULL ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 328;
	}
	xgeXuiUnit(&tXui);
	xgeFontFree(&tFont);
	return 0;
}

static int __testXuiScrollView(void)
{
	xge_xui_context_t tXui;
	xge_xui_scroll_view_t tScroll;
	xge_xui_widget pRoot;
	xge_xui_widget pWidget;
	xge_event_t tEvent;
	xge_rect_t tRect;
	float fX;
	float fY;
	int iPaintCount;

	memset(&tXui, 0, sizeof(tXui));
	memset(&tScroll, 0, sizeof(tScroll));
	if ( xgeXuiInit(&tXui) != XGE_OK ) {
		return 340;
	}
	pRoot = xgeXuiRoot(&tXui);
	pWidget = xgeXuiWidgetCreate();
	if ( (pRoot == NULL) || (pWidget == NULL) ) {
		xgeXuiWidgetFree(pWidget);
		xgeXuiUnit(&tXui);
		return 341;
	}
	tRect.fX = 10.0f;
	tRect.fY = 10.0f;
	tRect.fW = 100.0f;
	tRect.fH = 80.0f;
	xgeXuiWidgetSetRect(pWidget, tRect);
	xgeXuiWidgetSetPaddingPx(pWidget, 5.0f, 5.0f, 5.0f, 5.0f);
	xgeXuiWidgetAdd(pRoot, pWidget);
	xgeXuiUpdate(&tXui, 0.0f);
	if ( xgeXuiScrollViewInit(&tScroll, &tXui, pWidget) != XGE_OK ) {
		xgeXuiUnit(&tXui);
		return 342;
	}
	xgeXuiScrollViewSetContentSize(&tScroll, 220.0f, 210.0f);
	xgeXuiScrollViewSetColors(&tScroll, XGE_COLOR_RGBA(1, 2, 3, 255), XGE_COLOR_RGBA(4, 5, 6, 255), XGE_COLOR_RGBA(7, 8, 9, 255));
	if ( (xgeXuiWidgetGetFlags(pWidget) & XGE_XUI_WIDGET_CLIP) == 0 || xgeXuiWidgetIsFocusable(pWidget) == 0 ) {
		xgeXuiUnit(&tXui);
		return 343;
	}
	xgeXuiScrollViewSetOffset(&tScroll, 500.0f, 500.0f);
	xgeXuiScrollViewGetOffset(&tScroll, &fX, &fY);
	if ( fX != 130.0f || fY != 140.0f ) {
		xgeXuiUnit(&tXui);
		return 344;
	}
	xgeXuiScrollViewSetOffset(&tScroll, 0.0f, 0.0f);
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_MOUSE_WHEEL;
	tEvent.fX = 20.0f;
	tEvent.fY = 20.0f;
	tEvent.fDY = -1.0f;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED ) {
		xgeXuiUnit(&tXui);
		return 345;
	}
	xgeXuiScrollViewGetOffset(&tScroll, &fX, &fY);
	if ( fX != 0.0f || fY != 32.0f ) {
		xgeXuiUnit(&tXui);
		return 346;
	}
	tEvent.iType = XGE_EVENT_MOUSE_DOWN;
	tEvent.fX = 30.0f;
	tEvent.fY = 30.0f;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || tXui.pCapture != pWidget || tXui.pFocus != pWidget ) {
		xgeXuiUnit(&tXui);
		return 347;
	}
	tEvent.iType = XGE_EVENT_MOUSE_MOVE;
	tEvent.fX = 20.0f;
	tEvent.fY = 10.0f;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED ) {
		xgeXuiUnit(&tXui);
		return 348;
	}
	xgeXuiScrollViewGetOffset(&tScroll, &fX, &fY);
	if ( fX != 10.0f || fY != 52.0f ) {
		xgeXuiUnit(&tXui);
		return 349;
	}
	tEvent.iType = XGE_EVENT_MOUSE_UP;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || tXui.pCapture != NULL || tScroll.bDragging != 0 ) {
		xgeXuiUnit(&tXui);
		return 350;
	}
	iPaintCount = xgeXuiPaint(&tXui);
	if ( iPaintCount != 1 || tXui.iPaintCommandCount != 1 || pWidget->procPaint != xgeXuiScrollViewPaintProc || pWidget->procEvent != xgeXuiScrollViewEventProc ) {
		xgeXuiUnit(&tXui);
		return 351;
	}
	xgeXuiScrollViewUnit(&tScroll);
	if ( pWidget->procEvent != NULL || pWidget->procPaint != NULL || pWidget->pUser != NULL ) {
		xgeXuiUnit(&tXui);
		return 352;
	}
	xgeXuiUnit(&tXui);
	return 0;
}

static int g_iXuiListSelected;
static int g_iXuiListSelectCount;

static void __testXuiListViewSelect(xge_xui_widget pWidget, int iIndex, void* pUser)
{
	(void)pWidget;
	(void)pUser;
	g_iXuiListSelected = iIndex;
	g_iXuiListSelectCount++;
}

static int __testXuiListView(void)
{
	static const char* arrItems[] = {"A", "B", "C", "D", "E", "F"};
	xrf_test_blob2_t tBlob;
	xge_font_t tFont;
	xge_xui_context_t tXui;
	xge_xui_list_view_t tList;
	xge_xui_widget pRoot;
	xge_xui_widget pWidget;
	xge_event_t tEvent;
	xge_rect_t tRect;
	int iPaintCount;

	__testXrfBlobMake(&tBlob);
	memset(&tFont, 0, sizeof(tFont));
	memset(&tXui, 0, sizeof(tXui));
	memset(&tList, 0, sizeof(tList));
	if ( xgeFontLoadXRFMemory(&tFont, &tBlob, sizeof(tBlob)) != XGE_OK ) {
		return 360;
	}
	if ( xgeXuiInit(&tXui) != XGE_OK ) {
		xgeFontFree(&tFont);
		return 361;
	}
	pRoot = xgeXuiRoot(&tXui);
	pWidget = xgeXuiWidgetCreate();
	if ( (pRoot == NULL) || (pWidget == NULL) ) {
		xgeXuiWidgetFree(pWidget);
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 362;
	}
	tRect.fX = 10.0f;
	tRect.fY = 10.0f;
	tRect.fW = 120.0f;
	tRect.fH = 60.0f;
	xgeXuiWidgetSetRect(pWidget, tRect);
	xgeXuiWidgetSetPaddingPx(pWidget, 2.0f, 2.0f, 2.0f, 2.0f);
	xgeXuiWidgetAdd(pRoot, pWidget);
	xgeXuiUpdate(&tXui, 0.0f);
	if ( xgeXuiListViewInit(&tList, &tXui, pWidget) != XGE_OK ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 363;
	}
	xgeXuiListViewSetFont(&tList, &tFont);
	xgeXuiListViewSetItems(&tList, arrItems, 6);
	xgeXuiListViewSetItemHeight(&tList, 20.0f);
	xgeXuiListViewSetSelect(&tList, __testXuiListViewSelect, NULL);
	xgeXuiListViewSetColors(&tList, XGE_COLOR_RGBA(1, 2, 3, 255), XGE_COLOR_RGBA(4, 5, 6, 255), XGE_COLOR_RGBA(7, 8, 9, 255), XGE_COLOR_RGBA(10, 11, 12, 255), XGE_COLOR_RGBA(13, 14, 15, 255), XGE_COLOR_RGBA(16, 17, 18, 255));
	if ( (xgeXuiWidgetGetFlags(pWidget) & XGE_XUI_WIDGET_CLIP) == 0 || xgeXuiWidgetIsFocusable(pWidget) == 0 ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 364;
	}
	xgeXuiListViewSetScroll(&tList, 1000.0f);
	if ( xgeXuiListViewGetScroll(&tList) != 64.0f ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 365;
	}
	xgeXuiListViewSetScroll(&tList, 0.0f);
	g_iXuiListSelected = -1;
	g_iXuiListSelectCount = 0;
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_MOUSE_DOWN;
	tEvent.fX = 20.0f;
	tEvent.fY = 34.0f;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || xgeXuiListViewGetSelected(&tList) != 1 || g_iXuiListSelected != 1 || g_iXuiListSelectCount != 1 || tXui.pFocus != pWidget ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 366;
	}
	tEvent.iType = XGE_EVENT_MOUSE_WHEEL;
	tEvent.fDY = -1.0f;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || xgeXuiListViewGetScroll(&tList) != 20.0f ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 367;
	}
	xgeXuiListViewSetSelected(&tList, 100);
	if ( xgeXuiListViewGetSelected(&tList) != -1 ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 368;
	}
	iPaintCount = xgeXuiPaint(&tXui);
	if ( iPaintCount != 1 || tXui.iPaintCommandCount != 1 || pWidget->procPaint != xgeXuiListViewPaintProc || pWidget->procEvent != xgeXuiListViewEventProc ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 369;
	}
	xgeXuiListViewUnit(&tList);
	if ( pWidget->procEvent != NULL || pWidget->procPaint != NULL || pWidget->pUser != NULL ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 370;
	}
	xgeXuiUnit(&tXui);
	xgeFontFree(&tFont);
	return 0;
}

static int g_iXuiDialogCloseCount;

static void __testXuiDialogClose(xge_xui_widget pWidget, void* pUser)
{
	(void)pWidget;
	(void)pUser;
	g_iXuiDialogCloseCount++;
}

static int __testXuiDialog(void)
{
	xrf_test_blob2_t tBlob;
	xge_font_t tFont;
	xge_xui_context_t tXui;
	xge_xui_dialog_t tDialog;
	xge_xui_widget pRoot;
	xge_xui_widget pWidget;
	xge_event_t tEvent;
	xge_rect_t tRect;
	int iPaintCount;

	__testXrfBlobMake(&tBlob);
	memset(&tFont, 0, sizeof(tFont));
	memset(&tXui, 0, sizeof(tXui));
	memset(&tDialog, 0, sizeof(tDialog));
	if ( xgeFontLoadXRFMemory(&tFont, &tBlob, sizeof(tBlob)) != XGE_OK ) {
		return 380;
	}
	if ( xgeXuiInit(&tXui) != XGE_OK ) {
		xgeFontFree(&tFont);
		return 381;
	}
	pRoot = xgeXuiRoot(&tXui);
	pWidget = xgeXuiWidgetCreate();
	if ( (pRoot == NULL) || (pWidget == NULL) ) {
		xgeXuiWidgetFree(pWidget);
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 382;
	}
	tRect.fX = 10.0f;
	tRect.fY = 10.0f;
	tRect.fW = 120.0f;
	tRect.fH = 80.0f;
	xgeXuiWidgetSetRect(pWidget, tRect);
	xgeXuiWidgetSetPaddingPx(pWidget, 4.0f, 4.0f, 4.0f, 4.0f);
	xgeXuiWidgetAdd(pRoot, pWidget);
	xgeXuiUpdate(&tXui, 0.0f);
	if ( xgeXuiDialogInit(&tDialog, &tXui, pWidget) != XGE_OK ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 383;
	}
	xgeXuiDialogSetTitle(&tDialog, &tFont, "A");
	xgeXuiDialogSetClose(&tDialog, __testXuiDialogClose, NULL);
	xgeXuiDialogSetColors(&tDialog, XGE_COLOR_RGBA(1, 2, 3, 120), XGE_COLOR_RGBA(4, 5, 6, 255), XGE_COLOR_RGBA(7, 8, 9, 255), XGE_COLOR_RGBA(10, 11, 12, 255));
	if ( xgeXuiDialogIsOpen(&tDialog) != 1 || (xgeXuiWidgetGetFlags(pWidget) & XGE_XUI_WIDGET_CLIP) == 0 || xgeXuiWidgetIsFocusable(pWidget) == 0 ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 384;
	}
	iPaintCount = xgeXuiPaint(&tXui);
	if ( iPaintCount != 1 || tXui.iPaintCommandCount != 1 || pWidget->procPaint != xgeXuiDialogPaintProc || pWidget->procEvent != xgeXuiDialogEventProc ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 385;
	}
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_MOUSE_DOWN;
	tEvent.fX = 20.0f;
	tEvent.fY = 20.0f;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || xgeXuiDialogIsOpen(&tDialog) != 1 || tXui.pFocus != pWidget ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 386;
	}
	g_iXuiDialogCloseCount = 0;
	tEvent.fX = tDialog.tCloseRect.fX + 1.0f;
	tEvent.fY = tDialog.tCloseRect.fY + 1.0f;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || xgeXuiDialogIsOpen(&tDialog) != 0 || g_iXuiDialogCloseCount != 1 || tDialog.iCloseCount != 1 ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 387;
	}
	if ( xgeXuiWidgetIsVisible(pWidget) != 0 ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 388;
	}
	xgeXuiDialogSetOpen(&tDialog, 1);
	if ( xgeXuiDialogIsOpen(&tDialog) != 1 || xgeXuiWidgetIsVisible(pWidget) == 0 ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 389;
	}
	xgeXuiDialogUnit(&tDialog);
	if ( pWidget->procEvent != NULL || pWidget->procPaint != NULL || pWidget->pUser != NULL ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 390;
	}
	xgeXuiUnit(&tXui);
	xgeFontFree(&tFont);
	return 0;
}

static int __testXuiLabel(void)
{
	xrf_test_blob2_t tBlob;
	xge_font_t tFont;
	xge_xui_context_t tXui;
	xge_xui_label_t tLabel;
	xge_xui_widget pRoot;
	xge_xui_widget pWidget;
	xge_vec2_t tSize;
	xge_rect_t tRect;
	int iPaintCount;

	__testXrfBlobMake(&tBlob);
	memset(&tFont, 0, sizeof(tFont));
	memset(&tXui, 0, sizeof(tXui));
	memset(&tLabel, 0, sizeof(tLabel));
	if ( xgeFontLoadXRFMemory(&tFont, &tBlob, sizeof(tBlob)) != XGE_OK ) {
		return 220;
	}
	if ( xgeXuiInit(&tXui) != XGE_OK ) {
		xgeFontFree(&tFont);
		return 221;
	}
	pRoot = xgeXuiRoot(&tXui);
	pWidget = xgeXuiWidgetCreate();
	if ( (pRoot == NULL) || (pWidget == NULL) ) {
		xgeXuiWidgetFree(pWidget);
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 222;
	}
	tRect.fX = 0.0f;
	tRect.fY = 0.0f;
	tRect.fW = 80.0f;
	tRect.fH = 30.0f;
	xgeXuiWidgetSetRect(pWidget, tRect);
	xgeXuiWidgetSetPaddingPx(pWidget, 2.0f, 3.0f, 4.0f, 5.0f);
	xgeXuiWidgetAdd(pRoot, pWidget);
	if ( xgeXuiLabelInit(&tLabel, pWidget, &tFont, "A") != XGE_OK ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 223;
	}
	tSize = xgeXuiLabelMeasure(&tLabel);
	if ( tSize.fX != 8.0f || tSize.fY != 14.0f ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 224;
	}
	xgeXuiLabelSetText(&tLabel, "A\xe4\xb8\xad");
	xgeXuiLabelSetColor(&tLabel, XGE_COLOR_RGBA(1, 2, 3, 255));
	xgeXuiLabelSetAlign(&tLabel, XGE_TEXT_ALIGN_CENTER | XGE_TEXT_ALIGN_MIDDLE);
	xgeXuiUpdate(&tXui, 0.0f);
	tSize = xgeXuiLabelMeasure(&tLabel);
	tRect = xgeXuiWidgetGetRect(pWidget);
	if ( tRect.fW != (tSize.fX + 6.0f) || tRect.fH != (tSize.fY + 8.0f) ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 228;
	}
	iPaintCount = xgeXuiPaint(&tXui);
	if ( iPaintCount != 1 || tXui.iPaintCommandCount != 1 ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 225;
	}
	if ( (tLabel.iTextFlags & XGE_TEXT_CLIP) == 0 || tLabel.iColor != XGE_COLOR_RGBA(1, 2, 3, 255) ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 226;
	}
	xgeXuiLabelUnit(&tLabel);
	if ( pWidget->procMeasure != NULL || pWidget->procPaint != NULL || pWidget->pUser != NULL ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 227;
	}
	xgeXuiUnit(&tXui);
	xgeFontFree(&tFont);
	return 0;
}

static int __testXuiImage(void)
{
	xge_xui_context_t tXui;
	xge_xui_image_t tImage;
	xge_xui_widget pRoot;
	xge_xui_widget pWidget;
	xge_texture_t tTexture;
	xge_rect_t tRect;
	int iPaintCount;

	memset(&tXui, 0, sizeof(tXui));
	memset(&tImage, 0, sizeof(tImage));
	memset(&tTexture, 0, sizeof(tTexture));
	tTexture.iWidth = 16;
	tTexture.iHeight = 8;
	tTexture.iFormat = XGE_PIXEL_RGBA8;
	tTexture.iRefCount = 1;
	tTexture.iBackendId = 0;
	if ( xgeXuiInit(&tXui) != XGE_OK ) {
		return 230;
	}
	pRoot = xgeXuiRoot(&tXui);
	pWidget = xgeXuiWidgetCreate();
	if ( (pRoot == NULL) || (pWidget == NULL) ) {
		xgeXuiWidgetFree(pWidget);
		xgeXuiUnit(&tXui);
		return 231;
	}
	tRect.fX = 0.0f;
	tRect.fY = 0.0f;
	tRect.fW = 40.0f;
	tRect.fH = 30.0f;
	xgeXuiWidgetSetRect(pWidget, tRect);
	xgeXuiWidgetSetPaddingPx(pWidget, 2.0f, 3.0f, 4.0f, 5.0f);
	xgeXuiWidgetAdd(pRoot, pWidget);
	if ( xgeXuiImageInit(&tImage, pWidget, &tTexture) != XGE_OK ) {
		xgeXuiUnit(&tXui);
		return 232;
	}
	tRect.fX = 1.0f;
	tRect.fY = 2.0f;
	tRect.fW = 8.0f;
	tRect.fH = 4.0f;
	xgeXuiImageSetSource(&tImage, tRect);
	xgeXuiImageSetColor(&tImage, XGE_COLOR_RGBA(1, 2, 3, 128));
	xgeXuiImageSetMode(&tImage, XGE_XUI_IMAGE_FIT);
	xgeXuiUpdate(&tXui, 0.0f);
	tRect = xgeXuiWidgetGetRect(pWidget);
	if ( tRect.fW != 14.0f || tRect.fH != 12.0f ) {
		xgeXuiUnit(&tXui);
		return 237;
	}
	iPaintCount = xgeXuiPaint(&tXui);
	if ( iPaintCount != 1 || tXui.iPaintCommandCount != 1 || pWidget->procPaint != xgeXuiImagePaintProc ) {
		xgeXuiUnit(&tXui);
		return 233;
	}
	if ( tImage.iMode != XGE_XUI_IMAGE_FIT || tImage.iColor != XGE_COLOR_RGBA(1, 2, 3, 128) || tImage.tSrc.fW != 8.0f ) {
		xgeXuiUnit(&tXui);
		return 234;
	}
	xgeXuiImageSetTexture(&tImage, NULL);
	if ( tImage.pTexture != NULL ) {
		xgeXuiUnit(&tXui);
		return 235;
	}
	xgeXuiImageUnit(&tImage);
	if ( pWidget->procMeasure != NULL || pWidget->procPaint != NULL || pWidget->pUser != NULL ) {
		xgeXuiUnit(&tXui);
		return 236;
	}
	xgeXuiUnit(&tXui);
	return 0;
}

static int __testXuiInput(void)
{
	xrf_test_blob2_t tBlob;
	xge_font_t tFont;
	xge_xui_context_t tXui;
	xge_xui_input_t tInput;
	xge_xui_widget pRoot;
	xge_xui_widget pWidget;
	xge_event_t tEvent;
	xge_ime_event_t tIme;
	xge_rect_t tRect;
	xge_rect_t tCandidate;
	int iPaintCount;
	int iStart;
	int iEnd;

	__testXrfBlobMake(&tBlob);
	memset(&tFont, 0, sizeof(tFont));
	memset(&tXui, 0, sizeof(tXui));
	memset(&tInput, 0, sizeof(tInput));
	if ( xgeFontLoadXRFMemory(&tFont, &tBlob, sizeof(tBlob)) != XGE_OK ) {
		return 240;
	}
	if ( xgeXuiInit(&tXui) != XGE_OK ) {
		xgeFontFree(&tFont);
		return 241;
	}
	pRoot = xgeXuiRoot(&tXui);
	pWidget = xgeXuiWidgetCreate();
	if ( (pRoot == NULL) || (pWidget == NULL) ) {
		xgeXuiWidgetFree(pWidget);
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 242;
	}
	tRect.fX = 0.0f;
	tRect.fY = 0.0f;
	tRect.fW = 160.0f;
	tRect.fH = 80.0f;
	xgeXuiWidgetSetRect(pRoot, tRect);
	tRect.fX = 10.0f;
	tRect.fY = 12.0f;
	tRect.fW = 96.0f;
	tRect.fH = 24.0f;
	xgeXuiWidgetSetRect(pWidget, tRect);
	xgeXuiWidgetSetPaddingPx(pWidget, 3.0f, 2.0f, 3.0f, 2.0f);
	xgeXuiWidgetAdd(pRoot, pWidget);
	if ( xgeXuiInputInit(&tInput, &tXui, pWidget, &tFont) != XGE_OK ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 243;
	}
	xgeXuiInputSetColors(&tInput, XGE_COLOR_RGBA(1, 2, 3, 255), XGE_COLOR_RGBA(4, 5, 6, 255), XGE_COLOR_RGBA(7, 8, 9, 255), XGE_COLOR_RGBA(10, 11, 12, 255));
	xgeXuiUpdate(&tXui, 0.0f);
	iPaintCount = xgeXuiPaint(&tXui);
	if ( iPaintCount != 1 || tXui.iPaintCommandCount != 1 || pWidget->procPaint != xgeXuiInputPaintProc || pWidget->procEvent != xgeXuiInputEventProc ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 244;
	}
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_MOUSE_DOWN;
	tEvent.fX = 12.0f;
	tEvent.fY = 14.0f;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || tXui.pFocus != pWidget ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 245;
	}
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_TEXT;
	tEvent.iCodepoint = 'A';
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || strcmp(xgeXuiInputGetText(&tInput), "A") != 0 ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 246;
	}
	tEvent.iCodepoint = 0x4E2D;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || strcmp(xgeXuiInputGetText(&tInput), "A\xe4\xb8\xad") != 0 ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 247;
	}
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_KEY_DOWN;
	tEvent.iParam1 = XGE_KEY_BACKSPACE;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || strcmp(xgeXuiInputGetText(&tInput), "A") != 0 ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 248;
	}
	xgeXuiInputSetText(&tInput, "B");
	xgeXuiInputSetFont(&tInput, &tFont);
	if ( strcmp(xgeXuiInputGetText(&tInput), "B") != 0 ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 249;
	}
	xgeXuiInputSetText(&tInput, "ABCD");
	xgeXuiInputSetSelection(&tInput, 1, 3);
	xgeXuiInputGetSelection(&tInput, &iStart, &iEnd);
	if ( iStart != 1 || iEnd != 3 ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 251;
	}
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_TEXT;
	tEvent.iCodepoint = 'Z';
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || strcmp(xgeXuiInputGetText(&tInput), "AZD") != 0 ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 252;
	}
	xgeXuiInputSetText(&tInput, "AB");
	xgeXuiInputSetSelection(&tInput, 1, 1);
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_KEY_DOWN;
	tEvent.iParam1 = XGE_KEY_DELETE;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || strcmp(xgeXuiInputGetText(&tInput), "A") != 0 ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 253;
	}
	xgeXuiInputSetText(&tInput, "AA");
	xgeXuiInputSetSelection(&tInput, 1, 1);
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_KEY_DOWN;
	tEvent.iParam1 = XGE_KEY_LEFT;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || xgeXuiTextGetCursor(&tInput.tText) != 0 ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 254;
	}
	tEvent.iParam1 = XGE_KEY_RIGHT;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || xgeXuiTextGetCursor(&tInput.tText) != 1 ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 255;
	}
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_MOUSE_DOWN;
	tEvent.fX = pWidget->tContentRect.fX + 1.0f;
	tEvent.fY = pWidget->tContentRect.fY + 1.0f;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 256;
	}
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_MOUSE_MOVE;
	tEvent.fX = pWidget->tContentRect.fX + 14.0f;
	tEvent.fY = pWidget->tContentRect.fY + 1.0f;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 257;
	}
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_MOUSE_UP;
	tEvent.fX = pWidget->tContentRect.fX + 14.0f;
	tEvent.fY = pWidget->tContentRect.fY + 1.0f;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 258;
	}
	xgeXuiInputGetSelection(&tInput, &iStart, &iEnd);
	if ( iStart == iEnd ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 259;
	}
	memset(&tIme, 0, sizeof(tIme));
	tIme.sText = "ime";
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_IME_UPDATE;
	tEvent.pData = &tIme;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || strcmp(xgeXuiTextGetComposition(&tInput.tText), "ime") != 0 ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 261;
	}
	tCandidate = xgeXuiInputGetCandidateRect(&tInput);
	if ( (tCandidate.fH <= 0.0f) || (tCandidate.fX < pWidget->tContentRect.fX) ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 262;
	}
	tEvent.iType = XGE_EVENT_IME_END;
	tEvent.pData = NULL;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || xgeXuiTextGetComposition(&tInput.tText)[0] != 0 ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 263;
	}
	xgeXuiInputUnit(&tInput);
	if ( pWidget->procPaint != NULL || pWidget->procEvent != NULL || pWidget->pUser != NULL ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 250;
	}
	xgeXuiUnit(&tXui);
	xgeFontFree(&tFont);
	return 0;
}

static int __testXuiTextEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser)
{
	(void)pWidget;
	return xgeXuiTextInputEvent((xge_xui_text)pUser, pEvent);
}

static int __testXuiTextInput(void)
{
	xge_xui_context_t tXui;
	xge_xui_text_t tText;
	xge_xui_widget pRoot;
	xge_xui_widget pInput;
	xge_event_t tEvent;
	xge_ime_event_t tIme;
	int iRet;

	memset(&tXui, 0, sizeof(tXui));
	memset(&tText, 0, sizeof(tText));
	if ( xgeXuiTextInit(&tText) != XGE_OK ) {
		return 180;
	}
	if ( xgeXuiTextSet(&tText, "A") != XGE_OK ) {
		xgeXuiTextUnit(&tText);
		return 181;
	}
	if ( xgeXuiTextInsertCodepoint(&tText, 0x4E2D) != XGE_OK || strcmp(tText.sText, "A\xe4\xb8\xad") != 0 ) {
		xgeXuiTextUnit(&tText);
		return 182;
	}
	if ( xgeXuiTextDeleteBack(&tText) != XGE_OK || strcmp(tText.sText, "A") != 0 ) {
		xgeXuiTextUnit(&tText);
		return 183;
	}
	tText.iSelectStart = 0;
	tText.iSelectEnd = tText.iSize;
	if ( xgeXuiTextInsert(&tText, "B") != XGE_OK || strcmp(tText.sText, "B") != 0 ) {
		xgeXuiTextUnit(&tText);
		return 184;
	}
	if ( xgeXuiTextSet(&tText, "ABC") != XGE_OK ) {
		xgeXuiTextUnit(&tText);
		return 190;
	}
	xgeXuiTextSetSelection(&tText, 1, 3);
	if ( xgeXuiTextInsert(&tText, "D") != XGE_OK || strcmp(tText.sText, "AD") != 0 || xgeXuiTextGetCursor(&tText) != 2 ) {
		xgeXuiTextUnit(&tText);
		return 191;
	}
	if ( xgeXuiTextSet(&tText, "ABC") != XGE_OK ) {
		xgeXuiTextUnit(&tText);
		return 192;
	}
	xgeXuiTextSetCursor(&tText, 1);
	if ( xgeXuiTextDeleteForward(&tText) != XGE_OK || strcmp(tText.sText, "AC") != 0 || xgeXuiTextGetCursor(&tText) != 1 ) {
		xgeXuiTextUnit(&tText);
		return 193;
	}
	if ( xgeXuiTextSet(&tText, "B") != XGE_OK ) {
		xgeXuiTextUnit(&tText);
		return 194;
	}

	if ( xgeXuiInit(&tXui) != XGE_OK ) {
		xgeXuiTextUnit(&tText);
		return 185;
	}
	pRoot = xgeXuiRoot(&tXui);
	pInput = xgeXuiWidgetCreate();
	if ( (pRoot == NULL) || (pInput == NULL) ) {
		xgeXuiWidgetFree(pInput);
		xgeXuiUnit(&tXui);
		xgeXuiTextUnit(&tText);
		return 186;
	}
	pInput->procEvent = __testXuiTextEventProc;
	pInput->pUser = &tText;
	xgeXuiWidgetSetFocusable(pInput, 1);
	xgeXuiWidgetAdd(pRoot, pInput);
	xgeXuiSetFocus(&tXui, pInput);
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_TEXT;
	tEvent.iCodepoint = 'C';
	iRet = xgeXuiDispatchEvent(&tXui, &tEvent);
	if ( (iRet != XGE_XUI_EVENT_CONSUMED) || (strcmp(tText.sText, "BC") != 0) ) {
		xgeXuiUnit(&tXui);
		xgeXuiTextUnit(&tText);
		return 187;
	}
	tEvent.iCodepoint = 0x4E2D;
	iRet = xgeXuiDispatchEvent(&tXui, &tEvent);
	if ( (iRet != XGE_XUI_EVENT_CONSUMED) || (strcmp(tText.sText, "BC\xe4\xb8\xad") != 0) ) {
		xgeXuiUnit(&tXui);
		xgeXuiTextUnit(&tText);
		return 188;
	}
	tEvent.iCodepoint = 0x1F600;
	iRet = xgeXuiDispatchEvent(&tXui, &tEvent);
	if ( (iRet != XGE_XUI_EVENT_CONTINUE) || (strcmp(tText.sText, "BC\xe4\xb8\xad") != 0) ) {
		xgeXuiUnit(&tXui);
		xgeXuiTextUnit(&tText);
		return 189;
	}
	memset(&tIme, 0, sizeof(tIme));
	tIme.sText = "pin";
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_IME_UPDATE;
	tEvent.pData = &tIme;
	iRet = xgeXuiDispatchEvent(&tXui, &tEvent);
	if ( (iRet != XGE_XUI_EVENT_CONSUMED) || (strcmp(xgeXuiTextGetComposition(&tText), "pin") != 0) ) {
		xgeXuiUnit(&tXui);
		xgeXuiTextUnit(&tText);
		return 195;
	}
	tEvent.iType = XGE_EVENT_IME_END;
	tEvent.pData = NULL;
	iRet = xgeXuiDispatchEvent(&tXui, &tEvent);
	if ( (iRet != XGE_XUI_EVENT_CONSUMED) || (xgeXuiTextGetComposition(&tText)[0] != 0) ) {
		xgeXuiUnit(&tXui);
		xgeXuiTextUnit(&tText);
		return 196;
	}
	xgeXuiUnit(&tXui);
	xgeXuiTextUnit(&tText);
	return 0;
}

int main(void)
{
	int iRet;

	iRet = __testCoreLifecycle();
	if ( iRet != 0 ) {
		xgeUnit();
		return iRet;
	}

	iRet = __testSceneSystem();
	if ( iRet != 0 ) {
		xgeUnit();
		return iRet;
	}

	iRet = __testRunModes();
	if ( iRet != 0 ) {
		xgeUnit();
		return iRet;
	}

	iRet = __testXrtTimeIntegration();
	if ( iRet != 0 ) {
		xgeUnit();
		return iRet;
	}

	iRet = __testDirtyRects();
	if ( iRet != 0 ) {
		xgeUnit();
		return iRet;
	}

	iRet = __testImageMemory();
	if ( iRet != 0 ) {
		xgeUnit();
		return iRet;
	}

	iRet = __testBaseTypesAndInput();
	if ( iRet != 0 ) {
		xgeUnit();
		return iRet;
	}

	iRet = __testTextureLifetime();
	if ( iRet != 0 ) {
		xgeUnit();
		return iRet;
	}

	iRet = __testResourceProtocol();
	if ( iRet != 0 ) {
		xgeUnit();
		return iRet;
	}

	iRet = __testAudioApiShape();
	if ( iRet != 0 ) {
		xgeUnit();
		return iRet;
	}

	iRet = __testTextFontBase();
	if ( iRet != 0 ) {
		xgeUnit();
		return iRet;
	}

	iRet = __testXRFMemory();
	if ( iRet != 0 ) {
		xgeUnit();
		return iRet;
	}

	iRet = __testXuiIncubationBase();
	if ( iRet != 0 ) {
		xgeUnit();
		return iRet;
	}

	iRet = __testXuiPaintCommands();
	if ( iRet != 0 ) {
		xgeUnit();
		return iRet;
	}

	iRet = __testXuiManualRefresh();
	if ( iRet != 0 ) {
		xgeUnit();
		return iRet;
	}

	iRet = __testXuiTheme();
	if ( iRet != 0 ) {
		xgeUnit();
		return iRet;
	}

	iRet = __testXuiLayoutModes();
	if ( iRet != 0 ) {
		xgeUnit();
		return iRet;
	}

	iRet = __testXuiEvents();
	if ( iRet != 0 ) {
		xgeUnit();
		return iRet;
	}

	iRet = __testXuiTextInput();
	if ( iRet != 0 ) {
		xgeUnit();
		return iRet;
	}

	iRet = __testXuiButton();
	if ( iRet != 0 ) {
		xgeUnit();
		return iRet;
	}

	iRet = __testXuiToggle();
	if ( iRet != 0 ) {
		xgeUnit();
		return iRet;
	}

	iRet = __testXuiSlider();
	if ( iRet != 0 ) {
		xgeUnit();
		return iRet;
	}

	iRet = __testXuiProgress();
	if ( iRet != 0 ) {
		xgeUnit();
		return iRet;
	}

	iRet = __testXuiPanel();
	if ( iRet != 0 ) {
		xgeUnit();
		return iRet;
	}

	iRet = __testXuiScrollView();
	if ( iRet != 0 ) {
		xgeUnit();
		return iRet;
	}

	iRet = __testXuiListView();
	if ( iRet != 0 ) {
		xgeUnit();
		return iRet;
	}

	iRet = __testXuiDialog();
	if ( iRet != 0 ) {
		xgeUnit();
		return iRet;
	}

	iRet = __testXuiLabel();
	if ( iRet != 0 ) {
		xgeUnit();
		return iRet;
	}

	iRet = __testXuiImage();
	if ( iRet != 0 ) {
		xgeUnit();
		return iRet;
	}

	iRet = __testXuiInput();
	if ( iRet != 0 ) {
		xgeUnit();
		return iRet;
	}

	xgeUnit();
	return 0;
}

