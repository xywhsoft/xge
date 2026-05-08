#include "../xge.h"
#include <stdio.h>
#include <string.h>

static int g_iProviderFreeCount;
static unsigned char g_arrProviderData[] = { 'x', 'g', 'e', 0 };
static char g_arrXPackLastPath[128];

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

static int __testXuiPageProviderLoad(const char* sURI, void** ppData, int* pSize, void* pUser)
{
	static const char sXson[] = "{ \"xui\": 1, \"tree\": { \"type\": \"column\", \"id\": 7, \"children\": [ { \"type\": \"row\", \"name\": \"remote-child\" } ] } }";
	static const char sBadXson[] = "{ \"xui\": 1, \"tree\": { \"children\": 1 } }";
	static const char sImportPage[] = "{ \"xui\": 1, \"imports\": [ \"base.xson\", \"override.xson\" ], \"styles\": { \"panel\": { \"@parent\": \"base\", \"layout\": \"row\", \"height\": 64 } }, \"tree\": { \"type\": \"column\", \"id\": \"import-root\", \"style\": \"panel\" } }";
	static const char sImportBase[] = "{ \"xui\": 1, \"styles\": { \"base\": { \"width\": 100, \"height\": 40, \"background\": \"#010203\" } }, \"tree\": { \"id\": \"ignored-import-tree\" } }";
	static const char sImportOverride[] = "{ \"xui\": 1, \"styles\": { \"base\": { \"width\": 140 } } }";
	static const char sCycleA[] = "{ \"xui\": 1, \"imports\": [ \"b\" ], \"tree\": { \"id\": \"a\" } }";
	static const char sCycleB[] = "{ \"xui\": 1, \"imports\": [ \"a\" ] }";

	(void)pUser;
	if ( (sURI != NULL) && (strstr(sURI, "bad") != NULL) ) {
		*ppData = (void*)sBadXson;
		*pSize = (int)strlen(sBadXson);
		return XGE_OK;
	}
	if ( (sURI != NULL) && (strcmp(sURI, "xui://ui/page") == 0) ) {
		*ppData = (void*)sImportPage;
		*pSize = (int)strlen(sImportPage);
		return XGE_OK;
	}
	if ( (sURI != NULL) && (strcmp(sURI, "xui://ui/base.xson") == 0) ) {
		*ppData = (void*)sImportBase;
		*pSize = (int)strlen(sImportBase);
		return XGE_OK;
	}
	if ( (sURI != NULL) && (strcmp(sURI, "xui://ui/override.xson") == 0) ) {
		*ppData = (void*)sImportOverride;
		*pSize = (int)strlen(sImportOverride);
		return XGE_OK;
	}
	if ( (sURI != NULL) && (strcmp(sURI, "xui://cycle/a") == 0) ) {
		*ppData = (void*)sCycleA;
		*pSize = (int)strlen(sCycleA);
		return XGE_OK;
	}
	if ( (sURI != NULL) && (strcmp(sURI, "xui://cycle/b") == 0) ) {
		*ppData = (void*)sCycleB;
		*pSize = (int)strlen(sCycleB);
		return XGE_OK;
	}
	*ppData = (void*)sXson;
	*pSize = (int)strlen(sXson);
	return XGE_OK;
}

static int __testMiniProgramRequestFrame(void* pUser)
{
	int* pCount;

	pCount = (int*)pUser;
	if ( pCount != NULL ) {
		(*pCount)++;
	}
	return XGE_OK;
}

static int __testMiniProgramAudioCommand(int iCommand, int iHandle, const void* pData, int iSize, void* pUser)
{
	int* pValue;

	(void)pData;
	(void)iSize;
	pValue = (int*)pUser;
	if ( pValue != NULL ) {
		*pValue = iCommand + iHandle;
	}
	return XGE_OK;
}

static void* __testXPackRead(void* pPack, const char* sPath, uint64_t* pSize, void* pUser)
{
	(void)pPack;
	(void)pUser;
	strncpy(g_arrXPackLastPath, sPath, sizeof(g_arrXPackLastPath) - 1u);
	g_arrXPackLastPath[sizeof(g_arrXPackLastPath) - 1u] = 0;
	*pSize = 3;
	return g_arrProviderData;
}

static void __testXPackFree(void* pData, void* pUser)
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

typedef struct xge_async_test_state_t {
	int iCallbackCount;
	int iLastType;
	int iLastStatus;
	int iLastResult;
} xge_async_test_state_t;

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

static void __testAsyncComplete(xge_async_request pRequest, void* pUser)
{
	xge_async_test_state_t* pState;

	pState = (xge_async_test_state_t*)pUser;
	pState->iCallbackCount++;
	pState->iLastType = pRequest->iType;
	pState->iLastStatus = pRequest->iStatus;
	pState->iLastResult = pRequest->iResult;
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
	if ( xgeBegin() != XGE_ERROR_NOT_INITIALIZED || xgeEnd() != XGE_ERROR_NOT_INITIALIZED || xgeFlush() != XGE_ERROR_NOT_INITIALIZED || xgeRenderThreadSet(1) != XGE_ERROR_NOT_INITIALIZED ) {
		return 28;
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
#if XGE_HAS_DEBUGMODE
	xge_debug_stats_t tDebugStats;
#endif
	xge_platform_backend_t tPlatformBackend;
	xge_platform_backend_t tDefaultPlatformBackend;
	xge_graphics_backend_t tGraphicsBackend;
	xge_graphics_backend_t tDefaultGraphicsBackend;
	xge_gpu_caps_t tGpuCaps;
	xge_platform_caps_t tPlatformCaps;
	xge_platform_runtime_t tPlatformRuntime;
	xge_graphics_mapping_t tGraphicsMapping;
	xge_render_thread_caps_t tRenderThreadCaps;
	xge_egl_caps_t tEGLCaps;
	xge_egl_desc_t tRenderThreadEGLDesc;
	xge_texture_t tTexture;
	uint32_t iPixel;
	int iRet;
#if XGE_HAS_DEBUGMODE
	char arrCaps[512];
#endif
	char arrShaderHeader[128];
	char arrLibraryName[128];

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
	if ( tStats.fFrameTimeMs != 0.0f || tStats.fFrameTimeAvgMs != 0.0f || tStats.fFrameTimeMaxMs != 0.0f ) {
		return 26;
	}
#if XGE_HAS_DEBUGMODE
	memset(&tDebugStats, 0, sizeof(tDebugStats));
	if ( xgeDebugGetStats(&tDebugStats) != XGE_OK || tDebugStats.tFrame.iFrameCount != 0 || tDebugStats.iTextureCount != 0 ) {
		return 23;
	}
	memset(arrCaps, 0, sizeof(arrCaps));
	if ( xgeDebugDumpCaps(arrCaps, sizeof(arrCaps)) <= 0 || arrCaps[0] != 'X' || strstr(arrCaps, "RenderThread:") == NULL || strstr(arrCaps, "HighDPI:") == NULL || strstr(arrCaps, "Sokol Target: windows") == NULL || xgeDebugGetStats(NULL) != XGE_ERROR_INVALID_ARGUMENT || xgeDebugDumpCaps(NULL, sizeof(arrCaps)) != XGE_ERROR_INVALID_ARGUMENT ) {
		return 24;
	}
#endif
	{
		int iLogLevel;

		iLogLevel = xgeLogGetLevel();
		if ( xgeLogSetLevel(XGE_LOG_OFF) != XGE_OK || xgeLogGetLevel() != XGE_LOG_OFF || xgeLogWrite(XGE_LOG_ERROR, "test", "suppressed") != XGE_OK || xgeLogWrite(XGE_LOG_OFF + 1, "test", "bad") != XGE_ERROR_INVALID_ARGUMENT || xgeLogWrite(XGE_LOG_ERROR, "test", NULL) != XGE_ERROR_INVALID_ARGUMENT || xgeLogFlush() != XGE_OK ) {
			return 114;
		}
		if ( xgeLogSetLevel(iLogLevel) != XGE_OK ) {
			return 115;
		}
	}
	tPlatformBackend = xgePlatformBackendGet();
	if ( tPlatformBackend.iType != XGE_PLATFORM_BACKEND_SOKOL || tPlatformBackend.sName == NULL || xgePlatformBackendSet(NULL) != XGE_ERROR_INVALID_ARGUMENT ) {
		return 38;
	}
	tDefaultPlatformBackend = tPlatformBackend;
	memset(&tPlatformCaps, 0, sizeof(tPlatformCaps));
	if ( xgePlatformCapsGet(&tPlatformCaps) != XGE_OK || tPlatformCaps.iPlatformBackend != XGE_PLATFORM_BACKEND_SOKOL || tPlatformCaps.bWindow != 1 || tPlatformCaps.bRenderTarget != 1 || tPlatformCaps.bKeyboard != 1 || xgePlatformCapsGet(NULL) != XGE_ERROR_INVALID_ARGUMENT ) {
		return 42;
	}
	memset(&tPlatformRuntime, 0, sizeof(tPlatformRuntime));
	if ( xgePlatformRuntimeGet(&tPlatformRuntime) != XGE_OK || tPlatformRuntime.iWindowWidth != 320 || tPlatformRuntime.iWindowHeight != 200 || tPlatformRuntime.iFramebufferWidth != 320 || tPlatformRuntime.iFramebufferHeight != 200 || tPlatformRuntime.fDpiScale <= 0.0f || xgePlatformRuntimeGet(NULL) != XGE_ERROR_INVALID_ARGUMENT ) {
		return 58;
	}
	if ( tPlatformCaps.bSokol != 1 || tPlatformCaps.bSokolWindows != 1 || tPlatformCaps.bMouse != 1 || tPlatformCaps.bHighDPI != 1 || strcmp(tPlatformCaps.sSokolTargetName, "windows") != 0 ) {
		return 43;
	}
	if ( (tPlatformCaps.bSokolGLCore + tPlatformCaps.bSokolGLES3 + tPlatformCaps.bSokolD3D11 + tPlatformCaps.bSokolMetal + tPlatformCaps.bSokolDummy) <= 0 ) {
		return 57;
	}
	memset(&tRenderThreadCaps, 0, sizeof(tRenderThreadCaps));
	if ( xgeRenderThreadCapsGet(&tRenderThreadCaps) != XGE_OK || tRenderThreadCaps.bSupported != 1 || tRenderThreadCaps.bWorkerDrain != 1 || tRenderThreadCaps.bGLContextOwned != 0 || xgeRenderThreadCapsGet(NULL) != XGE_ERROR_INVALID_ARGUMENT ) {
		return 112;
	}
	tPlatformBackend.iType = XGE_PLATFORM_BACKEND_CUSTOM;
	tPlatformBackend.sName = "test";
	if ( xgePlatformBackendSet(&tPlatformBackend) != XGE_OK || xgePlatformBackendGet().iType != XGE_PLATFORM_BACKEND_CUSTOM ) {
		return 39;
	}
	if ( xgeGpuCapsGet(&tGpuCaps) != XGE_OK || tGpuCaps.iBackend != XGE_GPU_BACKEND_OPENGL33 || tGpuCaps.iMajorVersion != 3 || xgeGpuCapsGet(NULL) != XGE_ERROR_INVALID_ARGUMENT ) {
		return 40;
	}
	memset(arrShaderHeader, 0, sizeof(arrShaderHeader));
	if ( xgeGraphicsShaderHeaderGet(XGE_GPU_BACKEND_OPENGL33, arrShaderHeader, (int)sizeof(arrShaderHeader)) <= 0 || strstr(arrShaderHeader, "#version 330 core") == NULL || xgeGraphicsShaderHeaderGet(XGE_GPU_BACKEND_OPENGL33, NULL, (int)sizeof(arrShaderHeader)) != XGE_ERROR_INVALID_ARGUMENT ) {
		return 47;
	}
	memset(&tGraphicsMapping, 0, sizeof(tGraphicsMapping));
	if ( xgeGraphicsMappingGet(XGE_GPU_BACKEND_OPENGL33, &tGraphicsMapping) <= 0 || tGraphicsMapping.bOpenGLCore != 1 || tGraphicsMapping.bGLES != 0 || tGraphicsMapping.iRGBA8InternalFormat == 0 || xgeGraphicsMappingGet(XGE_GPU_BACKEND_OPENGL33, NULL) != XGE_ERROR_INVALID_ARGUMENT ) {
		return 54;
	}
	memset(arrLibraryName, 0, sizeof(arrLibraryName));
	if ( xgeGraphicsLibraryNameGet(XGE_GPU_BACKEND_OPENGL33, 0, arrLibraryName, (int)sizeof(arrLibraryName)) <= 0 || xgeGraphicsLibraryNameGet(XGE_GPU_BACKEND_OPENGL33, -1, arrLibraryName, (int)sizeof(arrLibraryName)) != XGE_ERROR_INVALID_ARGUMENT ) {
		return 50;
	}
	if ( strstr(arrLibraryName, "GL") == NULL && strstr(arrLibraryName, "gl") == NULL ) {
		return 51;
	}
	tGraphicsBackend = xgeGraphicsBackendGet();
	if ( tGraphicsBackend.iType != XGE_GPU_BACKEND_OPENGL33 || tGraphicsBackend.sName == NULL || xgeGraphicsBackendSet(NULL) != XGE_ERROR_INVALID_ARGUMENT ) {
		return 41;
	}
	tDefaultGraphicsBackend = tGraphicsBackend;
	tGraphicsBackend.iType = XGE_GPU_BACKEND_GLES30;
	tGraphicsBackend.sName = "gles30-test";
	if ( xgeGraphicsBackendSet(&tGraphicsBackend) != XGE_OK || xgeGpuCapsGet(&tGpuCaps) != XGE_OK || tGpuCaps.iBackend != XGE_GPU_BACKEND_GLES30 || tGpuCaps.iMajorVersion != 3 || tGpuCaps.iMinorVersion != 0 ) {
		return 43;
	}
	memset(arrShaderHeader, 0, sizeof(arrShaderHeader));
	if ( xgeGraphicsShaderHeaderGet(XGE_GPU_BACKEND_NONE, arrShaderHeader, (int)sizeof(arrShaderHeader)) <= 0 || strstr(arrShaderHeader, "#version 300 es") == NULL || strstr(arrShaderHeader, "precision mediump float") == NULL ) {
		return 48;
	}
	memset(&tGraphicsMapping, 0, sizeof(tGraphicsMapping));
	if ( xgeGraphicsMappingGet(XGE_GPU_BACKEND_NONE, &tGraphicsMapping) <= 0 || tGraphicsMapping.bGLES != 1 || tGraphicsMapping.bWebGL != 0 || strstr(tGraphicsMapping.sShaderHeader, "#version 300 es") == NULL ) {
		return 55;
	}
	memset(arrLibraryName, 0, sizeof(arrLibraryName));
	if ( xgeGraphicsLibraryNameGet(XGE_GPU_BACKEND_NONE, 0, arrLibraryName, (int)sizeof(arrLibraryName)) <= 0 || strstr(arrLibraryName, "GLES") == NULL ) {
		return 52;
	}
	tGraphicsBackend.iType = XGE_GPU_BACKEND_WEBGL2;
	tGraphicsBackend.sName = "webgl2-test";
	if ( xgeGraphicsBackendSet(&tGraphicsBackend) != XGE_OK || xgeGpuCapsGet(&tGpuCaps) != XGE_OK || tGpuCaps.iBackend != XGE_GPU_BACKEND_WEBGL2 || tGpuCaps.iMajorVersion != 2 || tGpuCaps.iMinorVersion != 0 ) {
		return 44;
	}
	memset(arrShaderHeader, 0, sizeof(arrShaderHeader));
	if ( xgeGraphicsShaderHeaderGet(XGE_GPU_BACKEND_NONE, arrShaderHeader, (int)sizeof(arrShaderHeader)) <= 0 || strstr(arrShaderHeader, "#version 300 es") == NULL ) {
		return 49;
	}
	memset(&tGraphicsMapping, 0, sizeof(tGraphicsMapping));
	if ( xgeGraphicsMappingGet(XGE_GPU_BACKEND_NONE, &tGraphicsMapping) <= 0 || tGraphicsMapping.bWebGL != 1 || tGraphicsMapping.bGLES != 1 || tGraphicsMapping.bClientSideVertexArray != 0 || tGraphicsMapping.bFramebufferObject != 1 ) {
		return 56;
	}
	memset(arrLibraryName, 0, sizeof(arrLibraryName));
	if ( xgeGraphicsLibraryNameGet(XGE_GPU_BACKEND_NONE, 99, arrLibraryName, (int)sizeof(arrLibraryName)) != XGE_ERROR_FILE_NOT_FOUND ) {
		return 53;
	}
	tGraphicsBackend.iType = XGE_GPU_BACKEND_NONE;
	tGraphicsBackend.sName = "custom-test";
	if ( xgeGraphicsBackendSet(&tGraphicsBackend) != XGE_OK || xgeGraphicsBackendGet().iType != XGE_GPU_BACKEND_CUSTOM || xgeGpuCapsGet(&tGpuCaps) != XGE_OK || tGpuCaps.iBackend != XGE_GPU_BACKEND_CUSTOM ) {
		return 45;
	}
	if ( xgePlatformBackendSet(&tDefaultPlatformBackend) != XGE_OK || xgeGraphicsBackendSet(&tDefaultGraphicsBackend) != XGE_OK ) {
		return 46;
	}
	if ( xgeFrame() != 1 || tState.iFrameCount != 1 ) {
		return 15;
	}
	tStats = xgeFrameStatsGet();
	if ( tStats.iFrameCount != 1 || tStats.iDrawCallCount != 0 || tStats.iBatchCount != 0 ) {
		return 21;
	}
	if ( tStats.fFrameTimeMs < 0.0f || tStats.fFrameTimeAvgMs < 0.0f || tStats.fFrameTimeMaxMs < 0.0f ) {
		return 27;
	}
#if XGE_HAS_DEBUGMODE
	if ( xgeDebugGetStats(&tDebugStats) != XGE_OK || tDebugStats.tFrame.iFrameCount != 1 ) {
		return 25;
	}
#endif
	if ( xgeGetFPS() != 60 || xgeGetDelta() < 0.016f || xgeGetDelta() > 0.017f ) {
		return 16;
	}
	if ( xgeRender() != 1 ) {
		return 17;
	}
	if ( xgeBegin() != XGE_OK || xgeBegin() != XGE_ERROR_ALREADY_INITIALIZED || xgeFlush() != XGE_OK || xgeEnd() != XGE_OK || xgeEnd() != XGE_ERROR_INVALID_ARGUMENT ) {
		return 29;
	}
	xgeInvalidateRect((xge_rect_t){ 0.0f, 0.0f, 8.0f, 8.0f });
	tStats = xgeFrameStatsGet();
	if ( tStats.iDirtyRectCount != 1 ) {
		return 22;
	}
	if ( xgeBegin() != XGE_OK ) {
		return 30;
	}
	xgePresent();
#if XGE_HAS_DEBUGMODE
	if ( xgedbgDirtyRectCount() != 0 || xgeEnd() != XGE_ERROR_INVALID_ARGUMENT ) {
		return 31;
	}
#else
	if ( xgeEnd() != XGE_ERROR_INVALID_ARGUMENT ) {
		return 31;
	}
#endif
	if ( xgeBegin() != XGE_OK || xgeFrame() != 1 || xgeBegin() != XGE_OK || xgeEnd() != XGE_OK ) {
		return 32;
	}
	iPixel = XGE_COLOR_RGBA(255, 255, 255, 255);
	if ( xgeTextureCreateRGBA(&tTexture, 1, 1, &iPixel) != XGE_OK ) {
		return 33;
	}
	xgeDraw(&tTexture, 1.0f, 2.0f);
	if ( tTexture.iRefCount != 2 ) {
		xgeTextureFree(&tTexture);
		return 34;
	}
	if ( xgeFlush() != XGE_OK || tTexture.iRefCount != 1 ) {
		xgeTextureFree(&tTexture);
		return 35;
	}
	xgeDraw(&tTexture, 3.0f, 4.0f);
	if ( tTexture.iRefCount != 2 ) {
		xgeTextureFree(&tTexture);
		return 36;
	}
	if ( xgeFrame() != 1 || tTexture.iRefCount != 1 ) {
		xgeTextureFree(&tTexture);
		return 37;
	}
	if ( xgeRenderThreadSet(1) != XGE_OK || xgeRenderThreadGet() != 1 ) {
		xgeTextureFree(&tTexture);
		return 108;
	}
	memset(&tRenderThreadCaps, 0, sizeof(tRenderThreadCaps));
	if ( xgeRenderThreadCapsGet(&tRenderThreadCaps) != XGE_OK || tRenderThreadCaps.bEnabled != 1 || tRenderThreadCaps.bCanUseWithCurrentContext != 1 || tRenderThreadCaps.bAsyncFlush != 0 ) {
		xgeRenderThreadSet(0);
		xgeTextureFree(&tTexture);
		return 113;
	}
	xgeDraw(&tTexture, 5.0f, 6.0f);
	if ( tTexture.iRefCount != 2 ) {
		xgeRenderThreadSet(0);
		xgeTextureFree(&tTexture);
		return 109;
	}
	if ( xgeFlush() != XGE_OK || tTexture.iRefCount != 1 ) {
		xgeRenderThreadSet(0);
		xgeTextureFree(&tTexture);
		return 110;
	}
	if ( xgeRenderThreadSet(0) != XGE_OK || xgeRenderThreadGet() != 0 ) {
		xgeTextureFree(&tTexture);
		return 111;
	}
	memset(&tRenderThreadEGLDesc, 0, sizeof(tRenderThreadEGLDesc));
	tRenderThreadEGLDesc.iWidth = 16;
	tRenderThreadEGLDesc.iHeight = 16;
	tRenderThreadEGLDesc.bPBuffer = 1;
	memset(&tEGLCaps, 0, sizeof(tEGLCaps));
	(void)xgeEGLCapsGet(&tEGLCaps);
	iRet = xgeRenderThreadEGLSet(&tRenderThreadEGLDesc);
	if ( tEGLCaps.bCompiled == 0 ) {
		if ( iRet != XGE_ERROR_UNSUPPORTED ) {
			xgeTextureFree(&tTexture);
			return 114;
		}
	} else if ( iRet == XGE_OK ) {
		if ( xgeRenderThreadSet(1) != XGE_OK ) {
			xgeTextureFree(&tTexture);
			return 115;
		}
		memset(&tRenderThreadCaps, 0, sizeof(tRenderThreadCaps));
		if ( xgeRenderThreadCapsGet(&tRenderThreadCaps) != XGE_OK || tRenderThreadCaps.bGLContextOwned != 1 ) {
			xgeRenderThreadSet(0);
			xgeTextureFree(&tTexture);
			return 116;
		}
		xgeRenderThreadSet(0);
		xgeRenderThreadEGLSet(NULL);
	} else if ( iRet != XGE_ERROR_UNSUPPORTED ) {
		xgeTextureFree(&tTexture);
		return 117;
	}
	xgeTextureFree(&tTexture);
#if XGE_HAS_DEBUGMODE
	xgedbgDirtyRectClear();
#endif
	tState.iQuitAt = tState.iFrameCount + 1;
	if ( xgeFrame() != 99 || tState.iFrameCount != tState.iQuitAt ) {
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
#if XGE_HAS_DEBUGMODE
	xge_rect_t tRect;
	xge_rect_t tOut;
	int i;

	xgedbgDirtyRectClear();
	if ( xgedbgDirtyRectCount() != 0 ) {
		return 5;
	}
	tRect.fX = 10.0f;
	tRect.fY = 20.0f;
	tRect.fW = 30.0f;
	tRect.fH = 40.0f;
	xgeInvalidateRect(tRect);
	if ( xgedbgDirtyRectCount() != 1 || xgedbgDirtyRectGet(0, &tOut) != XGE_OK || tOut.fX != 10.0f || tOut.fY != 20.0f || tOut.fW != 30.0f || tOut.fH != 40.0f ) {
		return 6;
	}
	tRect.fX = -5.0f;
	tRect.fY = -6.0f;
	tRect.fW = 20.0f;
	tRect.fH = 30.0f;
	xgeInvalidateRect(tRect);
	if ( xgedbgDirtyRectCount() != 2 || xgedbgDirtyRectGet(1, &tOut) != XGE_OK || tOut.fX != 0.0f || tOut.fY != 0.0f || tOut.fW != 15.0f || tOut.fH != 24.0f ) {
		return 7;
	}
	tRect.fX = 10.0f;
	tRect.fY = 10.0f;
	tRect.fW = -5.0f;
	tRect.fH = -5.0f;
	xgeInvalidateRect(tRect);
	if ( xgedbgDirtyRectCount() != 3 || xgedbgDirtyRectGet(2, &tOut) != XGE_OK || tOut.fX != 5.0f || tOut.fY != 5.0f || tOut.fW != 5.0f || tOut.fH != 5.0f ) {
		return 8;
	}
	tRect.fX = 0.0f;
	tRect.fY = 0.0f;
	tRect.fW = 0.0f;
	tRect.fH = 10.0f;
	xgeInvalidateRect(tRect);
	if ( xgedbgDirtyRectCount() != 3 || xgedbgDirtyRectGet(3, &tOut) != XGE_ERROR_INVALID_ARGUMENT || xgedbgDirtyRectGet(0, NULL) != XGE_ERROR_INVALID_ARGUMENT ) {
		return 9;
	}
	xgePresent();
	if ( xgedbgDirtyRectCount() != 0 ) {
		return 10;
	}
	xgeClear(XGE_COLOR_RGBA(1, 2, 3, 255));
	if ( xgedbgDirtyRectCount() != 1 || xgedbgDirtyRectGet(0, &tOut) != XGE_OK || tOut.fX != 0.0f || tOut.fY != 0.0f || tOut.fW != 320.0f || tOut.fH != 200.0f ) {
		return 11;
	}
	xgedbgDirtyRectClear();
	for ( i = 0; i < XGE_DIRTY_RECT_MAX + 1; i++ ) {
		tRect.fX = (float)i;
		tRect.fY = (float)i;
		tRect.fW = 1.0f;
		tRect.fH = 1.0f;
		xgeInvalidateRect(tRect);
	}
	if ( xgedbgDirtyRectCount() != 1 || xgedbgDirtyRectGet(0, &tOut) != XGE_OK || tOut.fX != 0.0f || tOut.fY != 0.0f || tOut.fW != (float)(XGE_DIRTY_RECT_MAX + 1) || tOut.fH != (float)(XGE_DIRTY_RECT_MAX + 1) ) {
		return 12;
	}
	xgedbgDirtyRectClear();
#endif
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
	xge_miniprogram_desc_t tMiniDesc;
	xge_miniprogram_bridge_t tMiniBridge;
	xge_miniprogram_touch_t tMiniTouch;
	xge_platform_backend_t tPlatformBackend;
	xge_graphics_backend_t tGraphicsBackend;
	xge_platform_caps_t tPlatformCaps;
	xge_egl_caps_t tEGLCaps;
	xge_egl_context_t tEGL;
	xge_egl_desc_t tEGLDesc;
	xge_offscreen_t tOffscreen;
	xge_gamepad_state_t tGamepad;
	xge_scene_t tScene;
	xge_scene_test_state_t tSceneState;
	xge_camera_t tCamera;
	xge_vertex_t arrVertices[4];
	unsigned char arrOffscreenPixels[8 * 8 * 4];
	xge_vec2_t tPointA;
	xge_vec2_t tPointB;
	xge_rect_t tRect;
	int iMiniBridgeValue;
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
	memset(&tMiniDesc, 0, sizeof(tMiniDesc));
	memset(&tMiniBridge, 0, sizeof(tMiniBridge));
	iMiniBridgeValue = 0;
	tMiniBridge.request_frame = __testMiniProgramRequestFrame;
	tMiniBridge.audio_command = __testMiniProgramAudioCommand;
	tMiniBridge.pUser = &iMiniBridgeValue;
	if ( xgeMiniProgramSetBridge(&tMiniBridge) != XGE_OK || xgeMiniProgramSetBridge(NULL) != XGE_ERROR_INVALID_ARGUMENT ) {
		return 57;
	}
	tMiniDesc.iWidth = 640;
	tMiniDesc.iHeight = 360;
	tMiniDesc.fDevicePixelRatio = 2.0f;
	if ( xgeMiniProgramInit(&tMiniDesc) != XGE_OK || xgePlatformBackendGet().iType != XGE_PLATFORM_BACKEND_MINIPROGRAM || xgeGraphicsBackendGet().iType != XGE_GPU_BACKEND_WEBGL2 ) {
		return 52;
	}
	memset(&tPlatformCaps, 0, sizeof(tPlatformCaps));
	if ( xgePlatformCapsGet(&tPlatformCaps) != XGE_OK || tPlatformCaps.bMiniProgram != 1 || tPlatformCaps.bWASM != 1 || tPlatformCaps.bTouch != 1 || tPlatformCaps.iGraphicsBackend != XGE_GPU_BACKEND_WEBGL2 ) {
		return 64;
	}
	tMiniTouch.iId = 7;
	tMiniTouch.fX = 12.0f;
	tMiniTouch.fY = 34.0f;
	tMiniTouch.fForce = 1.0f;
	if ( xgeMiniProgramTouch(XGE_TOUCH_BEGIN, &tMiniTouch, 1) != XGE_OK || xgeTouchGetCount() != 1 || xgeTouchFind(7, &tPoint) != XGE_OK || tPoint.fX != 12.0f || tPoint.fY != 34.0f ) {
		return 53;
	}
	if ( xgeMiniProgramTouchOne(XGE_TOUCH_MOVE, 8, 56.0f, 78.0f, 1.0f) != XGE_OK || xgeTouchFind(8, &tPoint) != XGE_OK || tPoint.fX != 56.0f || tPoint.fY != 78.0f ) {
		return 59;
	}
	if ( xgeMiniProgramText('Z') != XGE_OK || xgeTextGet() != 'Z' ) {
		return 54;
	}
	if ( xgeMiniProgramResize(320, 200, 1.0f) != XGE_OK || xgeMiniProgramResize(0, 200, 1.0f) != XGE_ERROR_INVALID_ARGUMENT ) {
		return 55;
	}
	if ( xgeMiniProgramRequestFrame() != XGE_OK || iMiniBridgeValue != 1 || xgeMiniProgramAudioCommand(10, 5, NULL, 0) != XGE_OK || iMiniBridgeValue != 15 ) {
		return 56;
	}
	if ( xgeMiniProgramTouch(XGE_TOUCH_BEGIN, NULL, 1) != XGE_ERROR_INVALID_ARGUMENT || xgeMiniProgramTouchOne(XGE_TOUCH_BEGIN, -1, 0.0f, 0.0f, 0.0f) != XGE_ERROR_INVALID_ARGUMENT || xgeMiniProgramText(0) != XGE_ERROR_INVALID_ARGUMENT || xgeMiniProgramAudioCommand(0, 0, NULL, -1) != XGE_ERROR_INVALID_ARGUMENT ) {
		return 58;
	}
	xgeMiniProgramUnit();
	if ( xgeMiniProgramInitSimple(320, 200, 1.0f) != XGE_OK || xgeGetWidth() != 320 || xgeGetHeight() != 200 || xgeMiniProgramInitSimple(0, 160, 1.0f) != XGE_ERROR_INVALID_ARGUMENT ) {
		return 60;
	}
	xgeMiniProgramUnit();
	tPlatformBackend = xgePlatformBackendDefault();
	tGraphicsBackend = xgeGraphicsBackendDefault();
	xgePlatformBackendSet(&tPlatformBackend);
	xgeGraphicsBackendSet(&tGraphicsBackend);
	memset(&tGamepad, 0, sizeof(tGamepad));
	if ( xgeGamepadConnected(0) != 0 || xgeGamepadGetState(0, &tGamepad) != XGE_OK || tGamepad.bConnected != 0 ) {
		return 37;
	}
	if ( xgeGamepadGetState(XGE_GAMEPAD_MAX, &tGamepad) != XGE_ERROR_INVALID_ARGUMENT || xgeGamepadGetState(0, NULL) != XGE_ERROR_INVALID_ARGUMENT ) {
		return 38;
	}
	memset(&tSceneState, 0, sizeof(tSceneState));
	__testSceneMake(&tScene, &tSceneState);
	if ( xgeSceneSet(&tScene) != XGE_OK ) {
		return 39;
	}
	tGamepad.bConnected = 1;
	tGamepad.iButtons = XGE_GAMEPAD_A | XGE_GAMEPAD_B;
	tGamepad.arrAxes[0] = 0.5f;
	tGamepad.arrAxes[1] = -0.25f;
	if ( xgeGamepadSetState(0, &tGamepad) != XGE_OK || xgeGamepadConnected(0) != 1 || tSceneState.iLastEvent != XGE_EVENT_GAMEPAD_CONNECTED ) {
		xgeSceneSet(NULL);
		return 40;
	}
	if ( xgeGamepadButtonDown(0, XGE_GAMEPAD_A) != 1 || xgeGamepadButtonPressed(0, XGE_GAMEPAD_A) != 1 || xgeGamepadAxis(0, 0) != 0.5f || xgeGamepadAxis(0, 1) != -0.25f ) {
		xgeSceneSet(NULL);
		return 41;
	}
	tGamepad.iButtons = XGE_GAMEPAD_B;
	if ( xgeGamepadSetState(0, &tGamepad) != XGE_OK || xgeGamepadButtonReleased(0, XGE_GAMEPAD_A) != 1 || xgeGamepadButtonDown(0, XGE_GAMEPAD_A) != 0 ) {
		xgeSceneSet(NULL);
		return 42;
	}
	if ( xgeGamepadSetConnected(0, 0) != XGE_OK || xgeGamepadConnected(0) != 0 || tSceneState.iLastEvent != XGE_EVENT_GAMEPAD_DISCONNECTED ) {
		xgeSceneSet(NULL);
		return 43;
	}
	if ( xgeGamepadSetConnected(XGE_GAMEPAD_MAX, 1) != XGE_ERROR_INVALID_ARGUMENT || xgeGamepadAxis(0, XGE_GAMEPAD_AXIS_COUNT) != 0.0f ) {
		xgeSceneSet(NULL);
		return 44;
	}
	xgeSceneSet(NULL);
	if ( xgeBlendGet() != XGE_BLEND_ALPHA ) {
		return 27;
	}
	xgeBlendSet(XGE_BLEND_ADD);
	if ( xgeBlendGet() != XGE_BLEND_ADD ) {
		return 28;
	}
	if ( xgeDepthTestGet() != 0 ) {
		return 45;
	}
	xgeDepthTestSet(1);
	if ( xgeDepthTestGet() != 1 ) {
		return 46;
	}
	xgeDepthTestSet(0);
	if ( xgeDepthTestGet() != 0 ) {
		return 47;
	}
	memset(arrVertices, 0, sizeof(arrVertices));
	arrVertices[0].fX = 1.0f;
	arrVertices[0].fY = 2.0f;
	arrVertices[0].fZ = 0.5f;
	arrVertices[0].fW = 1.0f;
	arrVertices[0].iColor = XGE_COLOR_RGBA(255, 255, 255, 255);
	if ( arrVertices[0].fX != 1.0f || arrVertices[0].fY != 2.0f || arrVertices[0].fZ != 0.5f || arrVertices[0].fW != 1.0f ) {
		return 48;
	}

	tCamera = xgeCameraDefault(320.0f, 200.0f);
	if ( tCamera.iProjectionMode != XGE_CAMERA_ORTHO || tCamera.fFovY != 0.0f ) {
		return 49;
	}
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
	tCamera = xgeCameraPerspective(320.0f, 200.0f, 60.0f, 0.1f, 500.0f);
	if ( tCamera.iProjectionMode != XGE_CAMERA_PERSPECTIVE || tCamera.fFovY != 60.0f || tCamera.fNearZ != 0.1f || tCamera.fFarZ != 500.0f || tCamera.fPerspectiveDistance <= 0.0f ) {
		return 50;
	}
	tCamera.tScale.fX = 0.0f;
	tCamera.tScale.fY = 0.0f;
	xgeCameraSet(&tCamera);
	tCamera = xgeCameraGet();
	if ( tCamera.tScale.fX != 1.0f || tCamera.tScale.fY != 1.0f || tCamera.iProjectionMode != XGE_CAMERA_PERSPECTIVE ) {
		return 51;
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
	memset(&tEGLDesc, 0, sizeof(tEGLDesc));
	tEGLDesc.iWidth = 64;
	tEGLDesc.iHeight = 32;
	tEGLDesc.bPBuffer = 1;
	memset(&tEGLCaps, 0, sizeof(tEGLCaps));
	if ( xgeEGLCapsGet(&tEGLCaps) != XGE_OK || xgeEGLCapsGet(NULL) != XGE_ERROR_INVALID_ARGUMENT || tEGLCaps.sBackendName[0] == 0 || tEGLCaps.sLastStage[0] == 0 ) {
		return 107;
	}
	memset(&tEGL, 0, sizeof(tEGL));
	if ( xgeEGLInit(&tEGL, &tEGLDesc) != XGE_ERROR_UNSUPPORTED || tEGL.iWidth != 64 || tEGL.bPBuffer != 1 || tEGL.sLastStage[0] == 0 || xgeEGLMakeCurrent(&tEGL) != XGE_ERROR_NOT_INITIALIZED ) {
		return 59;
	}
	memset(&tEGLDesc, 0, sizeof(tEGLDesc));
	tEGLDesc.iWidth = 64;
	tEGLDesc.iHeight = 32;
	tEGLDesc.bSurfaceless = 1;
	memset(&tEGL, 0, sizeof(tEGL));
	if ( (tEGLCaps.bSurfaceless == 0) && (xgeEGLInit(&tEGL, &tEGLDesc) != XGE_ERROR_UNSUPPORTED || tEGL.bSurfaceless != 1) ) {
		return 67;
	}
	if ( (tEGLCaps.bSurfaceless == 0) && (tEGL.sLastStage[0] == 0) ) {
		return 114;
	}
	memset(&tPlatformCaps, 0, sizeof(tPlatformCaps));
	if ( xgePlatformCapsGet(&tPlatformCaps) != XGE_OK || tPlatformCaps.iPlatformBackend != XGE_PLATFORM_BACKEND_EGL || tPlatformCaps.bOffscreen != tEGLCaps.bOffscreen || tPlatformCaps.bPBuffer != tEGLCaps.bPBuffer || tPlatformCaps.bSurfaceless != tEGLCaps.bSurfaceless || tPlatformCaps.bTouch != 0 ) {
		return 65;
	}
	xgeEGLUnit(&tEGL);
	if ( xgeEGLInit(NULL, &tEGLDesc) != XGE_ERROR_INVALID_ARGUMENT || xgeEGLInit(&tEGL, NULL) != XGE_ERROR_INVALID_ARGUMENT ) {
		return 60;
	}
	if ( xgeOffscreenInit(&tOffscreen, 8, 8) != XGE_OK || xgeOffscreenRenderTarget(&tOffscreen) == NULL || tOffscreen.tTarget.iWidth != 8 ) {
		return 61;
	}
	if ( tOffscreen.bEGLContext != tOffscreen.tEGL.bInitialized || (tOffscreen.bEGLContext == 0 && tOffscreen.bFallbackRenderTarget != 1) ) {
		xgeOffscreenUnit(&tOffscreen);
		return 66;
	}
	memset(arrOffscreenPixels, 0, sizeof(arrOffscreenPixels));
	if ( xgeOffscreenReadPixels(&tOffscreen, arrOffscreenPixels, 0) != XGE_OK ) {
		xgeOffscreenUnit(&tOffscreen);
		return 62;
	}
	xgeOffscreenUnit(&tOffscreen);
	if ( xgeOffscreenRenderTarget(&tOffscreen) != NULL || xgeOffscreenInit(NULL, 8, 8) != XGE_ERROR_INVALID_ARGUMENT || xgeOffscreenReadPixels(&tOffscreen, arrOffscreenPixels, 0) != XGE_ERROR_INVALID_ARGUMENT ) {
		return 63;
	}
	return 0;
}

static int __testTextureLifetime(void)
{
	xge_texture_t tTexture;
	xge_render_target_t tTarget;
	xge_render_target_t tWindowTarget;
	xge_pass_t tPass;
	xge_buffer_t tBuffer;
	xge_sprite_batch_t tBatch;
	xge_shape_batch_t tShapeBatch;
	xge_shader_t tShader;
	xge_shader_variant_set_t tVariantSet;
	xge_shader_define_t arrDefines[2];
	xge_shader pVariantShader;
	xge_material_t tMaterial;
	xge_mesh_t tMesh;
	xge_vertex_t arrMeshVertices[4];
	uint16_t arrMeshIndices[6];
	xge_draw_t tDraw;
	xge_rect_t tRect;
	xge_vec2_t arrPoly[4];
#if XGE_HAS_DEBUGMODE
	xge_debug_stats_t tDebugStats;
#endif
	xge_sampler_t tSampler;
	unsigned char arrPixels[16];
	unsigned char arrReadback[64];
	unsigned char arrFallback[16];

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
#if XGE_HAS_DEBUGMODE
	if ( xgeDebugGetStats(&tDebugStats) != XGE_OK || tDebugStats.iTextureCount <= 0 || tDebugStats.iTextureMemoryBytes < 16 ) {
		xgeTextureFree(&tTexture);
		return 59;
	}
#endif
	tSampler = xgeTextureGetSampler(&tTexture);
	if ( tSampler.iMinFilter != XGE_FILTER_LINEAR || tSampler.iMagFilter != XGE_FILTER_LINEAR || tSampler.iWrapS != XGE_WRAP_CLAMP || tSampler.iWrapT != XGE_WRAP_CLAMP ) {
		xgeTextureFree(&tTexture);
		return 84;
	}
	tSampler.iMinFilter = XGE_FILTER_NEAREST;
	tSampler.iMagFilter = XGE_FILTER_NEAREST;
	tSampler.iWrapS = XGE_WRAP_REPEAT;
	tSampler.iWrapT = XGE_WRAP_REPEAT;
	if ( xgeTextureSetSampler(&tTexture, &tSampler) != XGE_OK || xgeTextureGetSampler(&tTexture).iWrapS != XGE_WRAP_REPEAT ) {
		xgeTextureFree(&tTexture);
		return 85;
	}
	tSampler.iMinFilter = 999;
	if ( xgeTextureSetSampler(&tTexture, &tSampler) != XGE_ERROR_INVALID_ARGUMENT || xgeTextureSetSampler(NULL, &tSampler) != XGE_ERROR_INVALID_ARGUMENT || xgeTextureSetSampler(&tTexture, NULL) != XGE_ERROR_INVALID_ARGUMENT ) {
		xgeTextureFree(&tTexture);
		return 86;
	}
	tSampler = xgeTextureGetSampler(NULL);
	if ( tSampler.iMinFilter != XGE_FILTER_LINEAR || tSampler.iWrapT != XGE_WRAP_CLAMP ) {
		xgeTextureFree(&tTexture);
		return 87;
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
	memset(arrFallback, 0, sizeof(arrFallback));
	arrFallback[0] = 201; arrFallback[1] = 202; arrFallback[2] = 203; arrFallback[3] = 204;
	arrFallback[8] = 205; arrFallback[9] = 206; arrFallback[10] = 207; arrFallback[11] = 208;
	if ( xgeTextureUpdateRGBA(&tTexture, 1, 0, 1, 2, arrFallback, 8) != XGE_OK ) {
		xgeTextureFree(&tTexture);
		return 47;
	}
	memset(arrReadback, 0, sizeof(arrReadback));
	if ( xgeTextureReadPixels(&tTexture, arrReadback, 0) != XGE_OK || arrReadback[0] != 1 || arrReadback[4] != 201 || arrReadback[8] != 9 || arrReadback[12] != 205 ) {
		xgeTextureFree(&tTexture);
		return 48;
	}
	if ( xgeTextureUpdateRGBA(&tTexture, 1, 1, 2, 1, arrFallback, 0) != XGE_ERROR_INVALID_ARGUMENT || xgeTextureUpdateRGBA(&tTexture, 0, 0, 1, 1, NULL, 0) != XGE_ERROR_INVALID_ARGUMENT ) {
		xgeTextureFree(&tTexture);
		return 49;
	}
	memset(&tBuffer, 0, sizeof(tBuffer));
	if ( xgeBufferCreate(&tBuffer, XGE_BUFFER_VERTEX, XGE_BUFFER_DYNAMIC, arrPixels, 16) != XGE_OK || tBuffer.iType != XGE_BUFFER_VERTEX || tBuffer.iUsage != XGE_BUFFER_DYNAMIC || tBuffer.iSize != 16 || tBuffer.pData == NULL ) {
		xgeTextureFree(&tTexture);
		return 79;
	}
	arrFallback[0] = 101;
	arrFallback[1] = 102;
	arrFallback[2] = 103;
	arrFallback[3] = 104;
	if ( xgeBufferUpdate(&tBuffer, 4, arrFallback, 4) != XGE_OK || ((unsigned char*)tBuffer.pData)[4] != 101 || ((unsigned char*)tBuffer.pData)[7] != 104 ) {
		xgeBufferFree(&tBuffer);
		xgeTextureFree(&tTexture);
		return 80;
	}
	if ( xgeBufferUpdate(&tBuffer, 14, arrFallback, 4) != XGE_ERROR_INVALID_ARGUMENT || xgeBufferCreate(NULL, XGE_BUFFER_VERTEX, XGE_BUFFER_STATIC, arrPixels, 16) != XGE_ERROR_INVALID_ARGUMENT ) {
		xgeBufferFree(&tBuffer);
		xgeTextureFree(&tTexture);
		return 81;
	}
	if ( xgeBufferUpload(&tBuffer) != XGE_ERROR_NOT_INITIALIZED ) {
		xgeBufferFree(&tBuffer);
		xgeTextureFree(&tTexture);
		return 82;
	}
	xgeBufferFree(&tBuffer);
	if ( tBuffer.iSize != 0 || tBuffer.pData != NULL || tBuffer.iBackendId != 0 ) {
		xgeTextureFree(&tTexture);
		return 83;
	}
	memset(&tBatch, 0, sizeof(tBatch));
	memset(&tDraw, 0, sizeof(tDraw));
	tDraw.pTexture = &tTexture;
	tDraw.tSrc.fX = 0.0f;
	tDraw.tSrc.fY = 0.0f;
	tDraw.tSrc.fW = 1.0f;
	tDraw.tSrc.fH = 1.0f;
	tDraw.tDst.fX = 1.0f;
	tDraw.tDst.fY = 2.0f;
	tDraw.tDst.fW = 8.0f;
	tDraw.tDst.fH = 9.0f;
	tDraw.iColor = XGE_COLOR_RGBA(255, 255, 255, 255);
	tDraw.iFlags = XGE_DRAW_SCREEN_SPACE;
	if ( xgeSpriteBatchInit(&tBatch, &tTexture, 1, 0) != XGE_OK || tBatch.iCapacity != 1 || tTexture.iRefCount != 2 ) {
		xgeTextureFree(&tTexture);
		return 60;
	}
	if ( xgeSpriteBatchAdd(&tBatch, &tDraw) != XGE_OK || tBatch.iCount != 1 ) {
		xgeSpriteBatchFree(&tBatch);
		xgeTextureFree(&tTexture);
		return 61;
	}
	if ( xgeSpriteBatchAdd(&tBatch, &tDraw) != XGE_ERROR_OUT_OF_MEMORY ) {
		xgeSpriteBatchFree(&tBatch);
		xgeTextureFree(&tTexture);
		return 62;
	}
	if ( xgeSpriteBatchFlush(&tBatch) != XGE_ERROR_NOT_INITIALIZED ) {
		xgeSpriteBatchFree(&tBatch);
		xgeTextureFree(&tTexture);
		return 63;
	}
	xgeSpriteBatchClear(&tBatch);
	if ( tBatch.iCount != 0 || xgeSpriteBatchFlush(&tBatch) != XGE_OK ) {
		xgeSpriteBatchFree(&tBatch);
		xgeTextureFree(&tTexture);
		return 64;
	}
	xgeSpriteBatchFree(&tBatch);
	if ( tTexture.iRefCount != 1 || xgeSpriteBatchInit(NULL, &tTexture, 1, 0) != XGE_ERROR_INVALID_ARGUMENT ) {
		xgeTextureFree(&tTexture);
		return 65;
	}
	memset(&tShader, 0, sizeof(tShader));
	memset(&tVariantSet, 0, sizeof(tVariantSet));
	memset(arrDefines, 0, sizeof(arrDefines));
	pVariantShader = NULL;
	memset(&tMaterial, 0, sizeof(tMaterial));
	if ( xgeShaderCreate(&tShader, "vs", "fs") != XGE_ERROR_NOT_INITIALIZED || xgeShaderCreate(NULL, "vs", "fs") != XGE_ERROR_INVALID_ARGUMENT ) {
		xgeTextureFree(&tTexture);
		return 66;
	}
	if ( xgeShaderUniform1f(&tShader, "uTime", 1.0f) != XGE_ERROR_INVALID_ARGUMENT || xgeShaderAddRef(NULL) != 0 ) {
		xgeTextureFree(&tTexture);
		return 67;
	}
	if ( xgeShaderVariantSetInit(NULL, "vs", "fs") != XGE_ERROR_INVALID_ARGUMENT || xgeShaderVariantGet(NULL, 1, NULL, 0, &pVariantShader) != XGE_ERROR_INVALID_ARGUMENT ) {
		xgeTextureFree(&tTexture);
		return 74;
	}
	if ( xgeShaderVariantSetInit(&tVariantSet, "vs", "fs") != XGE_OK || tVariantSet.sVertexSource == NULL || tVariantSet.sFragmentSource == NULL ) {
		xgeTextureFree(&tTexture);
		return 75;
	}
	arrDefines[0].sName[0] = '1';
	arrDefines[0].sName[1] = 0;
	arrDefines[0].iValue = 1;
	if ( xgeShaderVariantGet(&tVariantSet, 1, arrDefines, 1, &pVariantShader) != XGE_ERROR_INVALID_ARGUMENT ) {
		xgeShaderVariantSetFree(&tVariantSet);
		xgeTextureFree(&tTexture);
		return 76;
	}
	arrDefines[0].sName[0] = 'X';
	arrDefines[0].sName[1] = 'G';
	arrDefines[0].sName[2] = 'E';
	arrDefines[0].sName[3] = '_';
	arrDefines[0].sName[4] = 'T';
	arrDefines[0].sName[5] = 'E';
	arrDefines[0].sName[6] = 'S';
	arrDefines[0].sName[7] = 'T';
	arrDefines[0].sName[8] = 0;
	if ( xgeShaderVariantGet(&tVariantSet, 1, arrDefines, 1, &pVariantShader) != XGE_ERROR_NOT_INITIALIZED || tVariantSet.iCount != 0 || pVariantShader != NULL ) {
		xgeShaderVariantSetFree(&tVariantSet);
		xgeTextureFree(&tTexture);
		return 77;
	}
	xgeShaderVariantSetFree(&tVariantSet);
	if ( tVariantSet.sVertexSource != NULL || tVariantSet.sFragmentSource != NULL || tVariantSet.pVariants != NULL || tVariantSet.iCount != 0 ) {
		xgeTextureFree(&tTexture);
		return 78;
	}
	xgeMaterialInit(&tMaterial);
	xgeMaterialSetShader(&tMaterial, &tShader);
	xgeMaterialSetTexture(&tMaterial, &tTexture);
	xgeMaterialSetColor(&tMaterial, XGE_COLOR_RGBA(128, 128, 128, 255));
	xgeMaterialSetBlend(&tMaterial, XGE_BLEND_ADD);
	if ( tMaterial.pShader != &tShader || tMaterial.pTexture != &tTexture || tTexture.iRefCount != 2 || tMaterial.iColor != XGE_COLOR_RGBA(128, 128, 128, 255) || tMaterial.tPipeline.iBlend != XGE_BLEND_ADD ) {
		xgeMaterialFree(&tMaterial);
		xgeTextureFree(&tTexture);
		return 68;
	}
	xgeMaterialDraw(&tMaterial, &tDraw);
	xgeMaterialFree(&tMaterial);
	if ( tTexture.iRefCount != 1 ) {
		xgeTextureFree(&tTexture);
		return 69;
	}
	xgeShaderFree(&tShader);
	memset(&tMesh, 0, sizeof(tMesh));
	memset(arrMeshVertices, 0, sizeof(arrMeshVertices));
	arrMeshVertices[0].fX = 0.0f; arrMeshVertices[0].fY = 0.0f; arrMeshVertices[0].fZ = 0.1f; arrMeshVertices[0].fW = 1.0f; arrMeshVertices[0].fU = 0.0f; arrMeshVertices[0].fV = 0.0f; arrMeshVertices[0].iColor = XGE_COLOR_RGBA(255, 255, 255, 255);
	arrMeshVertices[1].fX = 10.0f; arrMeshVertices[1].fY = 0.0f; arrMeshVertices[1].fZ = 0.1f; arrMeshVertices[1].fW = 1.0f; arrMeshVertices[1].fU = 1.0f; arrMeshVertices[1].fV = 0.0f; arrMeshVertices[1].iColor = XGE_COLOR_RGBA(255, 255, 255, 255);
	arrMeshVertices[2].fX = 0.0f; arrMeshVertices[2].fY = 10.0f; arrMeshVertices[2].fZ = 0.1f; arrMeshVertices[2].fW = 1.0f; arrMeshVertices[2].fU = 0.0f; arrMeshVertices[2].fV = 1.0f; arrMeshVertices[2].iColor = XGE_COLOR_RGBA(255, 255, 255, 255);
	arrMeshVertices[3].fX = 10.0f; arrMeshVertices[3].fY = 10.0f; arrMeshVertices[3].fZ = 0.1f; arrMeshVertices[3].fW = 1.0f; arrMeshVertices[3].fU = 1.0f; arrMeshVertices[3].fV = 1.0f; arrMeshVertices[3].iColor = XGE_COLOR_RGBA(255, 255, 255, 255);
	arrMeshIndices[0] = 0; arrMeshIndices[1] = 1; arrMeshIndices[2] = 2;
	arrMeshIndices[3] = 2; arrMeshIndices[4] = 1; arrMeshIndices[5] = 3;
	if ( xgeMeshCreate(&tMesh, arrMeshVertices, 4, arrMeshIndices, 6, XGE_MESH_DYNAMIC) != XGE_OK || tMesh.iVertexCount != 4 || tMesh.iIndexCount != 6 || tMesh.pVertices == NULL || tMesh.pIndices == NULL ) {
		xgeTextureFree(&tTexture);
		return 70;
	}
	arrMeshVertices[0].fZ = 0.25f;
	if ( xgeMeshUpdate(&tMesh, arrMeshVertices, 4, arrMeshIndices, 6) != XGE_OK || ((xge_vertex_t*)tMesh.pVertices)[0].fZ != 0.25f ) {
		xgeMeshFree(&tMesh);
		xgeTextureFree(&tTexture);
		return 71;
	}
	xgeMeshDraw(&tMesh, &tTexture, 0);
	if ( xgeMeshCreate(NULL, arrMeshVertices, 4, arrMeshIndices, 6, 0) != XGE_ERROR_INVALID_ARGUMENT || xgeMeshUpdate(&tMesh, NULL, 4, arrMeshIndices, 6) != XGE_ERROR_INVALID_ARGUMENT ) {
		xgeMeshFree(&tMesh);
		xgeTextureFree(&tTexture);
		return 72;
	}
	xgeMeshFree(&tMesh);
	if ( tMesh.pVertices != NULL || tMesh.iVertexCount != 0 ) {
		xgeTextureFree(&tTexture);
		return 73;
	}
	if ( xgeTextureUploadQueue(&tTexture) != XGE_OK || (tTexture.iFlags & XGE_TEXTURE_UPLOAD_QUEUED) == 0 ) {
		xgeTextureFree(&tTexture);
		return 50;
	}
	if ( xgeTextureUploadQueue(&tTexture) != XGE_OK || xgeTextureUploadFlush() != 0 || (tTexture.iFlags & XGE_TEXTURE_UPLOAD_QUEUED) == 0 ) {
		xgeTextureFree(&tTexture);
		return 51;
	}
	xgeTextureFree(&tTexture);
	if ( tTexture.pBackend != 0 || tTexture.iWidth != 0 ) {
		return 47;
	}
	if ( xgeTextureUploadQueue(&tTexture) != XGE_ERROR_INVALID_ARGUMENT ) {
		return 52;
	}
	memset(&tTarget, 0, sizeof(tTarget));
	memset(&tWindowTarget, 0, sizeof(tWindowTarget));
	memset(&tPass, 0, sizeof(tPass));
	if ( xgeRenderTargetWindow(&tWindowTarget) != XGE_OK || tWindowTarget.iWidth != xgeGetWidth() || tWindowTarget.iHeight != xgeGetHeight() || (tWindowTarget.iFlags & XGE_RENDER_TARGET_WINDOW) == 0 ) {
		return 53;
	}
	if ( xgeRenderTargetCreate(&tTarget, 4, 4) != XGE_OK || tTarget.iWidth != 4 || tTarget.iHeight != 4 || xgeRenderTargetTexture(&tTarget) != &tTarget.tTexture ) {
		xgeRenderTargetFree(&tTarget);
		return 54;
	}
	memset(arrReadback, 7, sizeof(arrReadback));
	if ( xgeRenderTargetReadPixels(&tTarget, arrReadback, 0) != XGE_OK || arrReadback[0] != 0 || arrReadback[15] != 0 ) {
		xgeRenderTargetFree(&tTarget);
		return 55;
	}
	if ( xgeRenderTargetResize(&tTarget, 2, 2) != XGE_OK || tTarget.iWidth != 2 || tTarget.iHeight != 2 || tTarget.tTexture.iWidth != 2 ) {
		xgeRenderTargetFree(&tTarget);
		return 56;
	}
	xgePassInit(&tPass, &tTarget, XGE_PASS_CLEAR_COLOR, XGE_COLOR_RGBA(1, 2, 3, 255));
	if ( xgePassBegin(&tPass) != XGE_ERROR_NOT_INITIALIZED || xgePassEnd(&tPass) != XGE_ERROR_INVALID_ARGUMENT ) {
		xgeRenderTargetFree(&tTarget);
		return 57;
	}
	xgeRenderTargetFree(&tTarget);
	if ( tTarget.iWidth != 0 || xgeRenderTargetTexture(&tTarget) != NULL || xgeRenderTargetCreate(NULL, 1, 1) != XGE_ERROR_INVALID_ARGUMENT ) {
		return 58;
	}
	xgeTextureFallbackClear();
	if ( xgeTextureLoad(&tTexture, "missing_texture_for_fallback.png") == XGE_OK ) {
		xgeTextureFree(&tTexture);
		return 48;
	}
	arrFallback[0] = 31; arrFallback[1] = 32; arrFallback[2] = 33; arrFallback[3] = 255;
	arrFallback[4] = 41; arrFallback[5] = 42; arrFallback[6] = 43; arrFallback[7] = 255;
	arrFallback[8] = 51; arrFallback[9] = 52; arrFallback[10] = 53; arrFallback[11] = 255;
	arrFallback[12] = 61; arrFallback[13] = 62; arrFallback[14] = 63; arrFallback[15] = 255;
	if ( xgeTextureFallbackSetRGBA(2, 2, arrFallback) != XGE_OK ) {
		return 48;
	}
	memset(&tTexture, 0, sizeof(tTexture));
	if ( xgeTextureLoad(&tTexture, "missing_texture_for_fallback.png") != XGE_OK || (tTexture.iFlags & XGE_TEXTURE_FALLBACK) == 0 ) {
		xgeTextureFallbackClear();
		return 49;
	}
	memset(arrReadback, 0, sizeof(arrReadback));
	if ( xgeTextureReadPixels(&tTexture, arrReadback, 0) != XGE_OK || arrReadback[0] != 31 || arrReadback[14] != 63 ) {
		xgeTextureFree(&tTexture);
		xgeTextureFallbackClear();
		return 48;
	}
	xgeTextureFree(&tTexture);
	if ( xgeTextureFallbackGet(&tTexture) != XGE_OK || (tTexture.iFlags & XGE_TEXTURE_FALLBACK) == 0 ) {
		xgeTextureFallbackClear();
		return 49;
	}
	xgeTextureFree(&tTexture);
	xgeTextureFallbackClear();
	if ( xgeTextureFallbackGet(&tTexture) != XGE_ERROR_RESOURCE_FAILED ) {
		return 48;
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
	if ( xgeShapeBatchInit(&tShapeBatch, XGE_COLOR_RGBA(255, 255, 255, 255), 2, 0) != XGE_OK || tShapeBatch.iTriangleCapacity != 2 || tShapeBatch.pVertices == NULL ) {
		return 88;
	}
	if ( xgeShapeBatchRectFill(&tShapeBatch, tRect) != XGE_OK || tShapeBatch.iTriangleCount != 2 ) {
		xgeShapeBatchFree(&tShapeBatch);
		return 89;
	}
	if ( xgeShapeBatchTriangleFill(&tShapeBatch, arrPoly[0], arrPoly[1], arrPoly[2]) != XGE_ERROR_OUT_OF_MEMORY ) {
		xgeShapeBatchFree(&tShapeBatch);
		return 90;
	}
	if ( xgeShapeBatchFlush(&tShapeBatch) != XGE_ERROR_NOT_INITIALIZED ) {
		xgeShapeBatchFree(&tShapeBatch);
		return 91;
	}
	xgeShapeBatchClear(&tShapeBatch);
	if ( tShapeBatch.iTriangleCount != 0 || xgeShapeBatchFlush(&tShapeBatch) != XGE_OK ) {
		xgeShapeBatchFree(&tShapeBatch);
		return 92;
	}
	xgeShapeBatchFree(&tShapeBatch);
	if ( tShapeBatch.pVertices != NULL || xgeShapeBatchInit(NULL, XGE_COLOR_RGBA(255, 255, 255, 255), 2, 0) != XGE_ERROR_INVALID_ARGUMENT ) {
		return 93;
	}
	xgeClipSet(tRect);
	xgeClipClear();
	return 0;
}

static int __testResourceProtocol(void)
{
	FILE* pFile;
	xge_resource_provider_t tProvider;
	xge_xpack_provider_t tXPackProvider;
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
	memset(&tXPackProvider, 0, sizeof(tXPackProvider));
	tXPackProvider.pPack = &tXPackProvider;
	tXPackProvider.sRoot = "assets";
	tXPackProvider.read = __testXPackRead;
	tXPackProvider.free = __testXPackFree;
	if ( xgeResourceXPackProviderAdd(&tXPackProvider) != XGE_OK ) {
		return 61;
	}
	g_iProviderFreeCount = 0;
	memset(g_arrXPackLastPath, 0, sizeof(g_arrXPackLastPath));
	if ( xgeResourceLoad("res://images/logo.png", &tResource) != XGE_OK ) {
		xgeResourceProviderClear();
		return 62;
	}
	if ( (tResource.pData != g_arrProviderData) || (tResource.iSize != 3) || (strcmp(g_arrXPackLastPath, "assets/images/logo.png") != 0) ) {
		xgeResourceFree(&tResource);
		xgeResourceProviderClear();
		return 63;
	}
	xgeResourceProviderClear();
	xgeResourceFree(&tResource);
	if ( g_iProviderFreeCount != 1 ) {
		return 64;
	}
	if ( xgeResourceXPackProviderAdd(NULL) != XGE_ERROR_INVALID_ARGUMENT ) {
		return 65;
	}
	return 0;
}

static int __testAsyncResources(void)
{
	xge_async_request_t tRequest;
	xge_async_test_state_t tState;
	xge_image_t tImage;
	xge_texture_t tTexture;
	xge_font_t tFont;
	xge_sound_t tSound;
	xge_glyph_metrics_t tMetrics;
	unsigned char arrFallback[4];
	unsigned char arrReadback[4];
	int iPoll;
	int iSpin;
	FILE* pFile;

	memset(&tState, 0, sizeof(tState));
	memset(&tImage, 0, sizeof(tImage));
	memset(&tTexture, 0, sizeof(tTexture));
	memset(&tFont, 0, sizeof(tFont));
	memset(&tSound, 0, sizeof(tSound));
	xgeAsyncRequestInit(&tRequest);
	if ( tRequest.iStatus != XGE_ASYNC_PENDING || xgeAsyncRequestCancel(&tRequest) != XGE_OK || tRequest.iStatus != XGE_ASYNC_CANCELLED ) {
		return 440;
	}
	xgeAsyncRequestFree(&tRequest);

	if ( xgeAsyncImageLoad(&tRequest, &tImage, "missing_async_image.png", XGE_IMAGE_PREMULTIPLIED, __testAsyncComplete, &tState) == XGE_OK ) {
		xgeAsyncRequestFree(&tRequest);
		xgeImageFree(&tImage);
		return 441;
	}
	if ( tRequest.iType != XGE_ASYNC_IMAGE || tRequest.iStatus != XGE_ASYNC_FAILED || tRequest.sURI == NULL || tState.iCallbackCount != 1 || tState.iLastStatus != XGE_ASYNC_FAILED ) {
		xgeAsyncRequestFree(&tRequest);
		return 442;
	}
	xgeAsyncRequestFree(&tRequest);

	memset(&tState, 0, sizeof(tState));
	memset(&tImage, 0, sizeof(tImage));
	xgeAsyncThreadingSet(1);
	if ( xgeAsyncThreadingGet() != 1 || xgeAsyncImageLoad(&tRequest, &tImage, "missing_async_thread_image.png", XGE_IMAGE_PREMULTIPLIED, __testAsyncComplete, &tState) != XGE_OK || tRequest.iStatus != XGE_ASYNC_LOADING || tState.iCallbackCount != 0 ) {
		xgeAsyncThreadingSet(0);
		xgeAsyncRequestFree(&tRequest);
		xgeImageFree(&tImage);
		return 450;
	}
	iPoll = XGE_ASYNC_LOADING;
	for ( iSpin = 0; iSpin < 200 && iPoll == XGE_ASYNC_LOADING; iSpin++ ) {
		iPoll = xgeAsyncPoll(&tRequest);
		if ( iPoll == XGE_ASYNC_LOADING ) {
			xgeSleep(1);
		}
	}
	if ( iPoll != XGE_ASYNC_FAILED || tRequest.iStatus != XGE_ASYNC_FAILED || tState.iCallbackCount != 1 || tState.iLastStatus != XGE_ASYNC_FAILED || xgeAsyncPoll(NULL) != XGE_ERROR_INVALID_ARGUMENT ) {
		xgeAsyncThreadingSet(0);
		xgeAsyncRequestFree(&tRequest);
		xgeImageFree(&tImage);
		return 451;
	}
	xgeAsyncRequestFree(&tRequest);
	xgeImageFree(&tImage);
	xgeAsyncThreadingSet(0);
	if ( xgeAsyncThreadingGet() != 0 ) {
		return 452;
	}

	arrFallback[0] = 17;
	arrFallback[1] = 18;
	arrFallback[2] = 19;
	arrFallback[3] = 255;
	if ( xgeTextureFallbackSetRGBA(1, 1, arrFallback) != XGE_OK ) {
		return 443;
	}
	if ( xgeAsyncTextureLoad(&tRequest, &tTexture, "missing_async_texture.png", XGE_IMAGE_PREMULTIPLIED, __testAsyncComplete, &tState) != XGE_OK ) {
		xgeTextureFallbackClear();
		xgeAsyncRequestFree(&tRequest);
		return 444;
	}
	if ( tRequest.iType != XGE_ASYNC_TEXTURE || tRequest.iStatus != XGE_ASYNC_READY || (tTexture.iFlags & XGE_TEXTURE_FALLBACK) == 0 || tState.iLastType != XGE_ASYNC_TEXTURE || tState.iLastResult != XGE_OK ) {
		xgeTextureFree(&tTexture);
		xgeTextureFallbackClear();
		xgeAsyncRequestFree(&tRequest);
		return 445;
	}
	memset(arrReadback, 0, sizeof(arrReadback));
	if ( xgeTextureReadPixels(&tTexture, arrReadback, 0) != XGE_OK || arrReadback[0] != 17 || arrReadback[3] != 255 ) {
		xgeTextureFree(&tTexture);
		xgeTextureFallbackClear();
		xgeAsyncRequestFree(&tRequest);
		return 446;
	}
	xgeTextureFree(&tTexture);
	xgeTextureFallbackClear();
	xgeAsyncRequestFree(&tRequest);

	xgeFontFallbackClear();
	if ( xgeAsyncFontLoad(&tRequest, &tFont, "missing_async_font.ttf", 16.0f, __testAsyncComplete, &tState) == XGE_OK || tRequest.iType != XGE_ASYNC_FONT || tRequest.iStatus != XGE_ASYNC_FAILED ) {
		xgeAsyncRequestFree(&tRequest);
		xgeFontFree(&tFont);
		return 447;
	}
	xgeAsyncRequestFree(&tRequest);
	pFile = fopen("C:/Windows/Fonts/arial.ttf", "rb");
	if ( pFile != 0 ) {
		fclose(pFile);
		if ( xgeFontFallbackSet("C:/Windows/Fonts/arial.ttf", 12.0f) != XGE_OK ) {
			return 447;
		}
		memset(&tFont, 0, sizeof(tFont));
		memset(&tState, 0, sizeof(tState));
		if ( xgeAsyncFontLoad(&tRequest, &tFont, "missing_async_font.ttf", 16.0f, __testAsyncComplete, &tState) != XGE_OK ) {
			xgeFontFallbackClear();
			xgeAsyncRequestFree(&tRequest);
			return 447;
		}
		if ( tRequest.iType != XGE_ASYNC_FONT || tRequest.iStatus != XGE_ASYNC_READY || (tFont.iFlags & XGE_FONT_FALLBACK) == 0 || tState.iLastType != XGE_ASYNC_FONT || tState.iLastResult != XGE_OK ) {
			xgeFontFree(&tFont);
			xgeFontFallbackClear();
			xgeAsyncRequestFree(&tRequest);
			return 447;
		}
		if ( xgeFontGlyphGet(&tFont, 'A', &tMetrics) != XGE_OK || tMetrics.fAdvanceX <= 0.0f ) {
			xgeFontFree(&tFont);
			xgeFontFallbackClear();
			xgeAsyncRequestFree(&tRequest);
			return 447;
		}
		xgeFontFree(&tFont);
		xgeFontFallbackClear();
		xgeAsyncRequestFree(&tRequest);
	}
	if ( xgeAsyncSoundLoad(&tRequest, &tSound, "missing_async_sound.wav", __testAsyncComplete, &tState) == XGE_OK || tRequest.iType != XGE_ASYNC_SOUND || tRequest.iStatus != XGE_ASYNC_FAILED ) {
		xgeAsyncRequestFree(&tRequest);
		xgeSoundFree(&tSound);
		return 448;
	}
	xgeAsyncRequestFree(&tRequest);
	if ( xgeAsyncTextureLoad(NULL, &tTexture, "x", XGE_IMAGE_PREMULTIPLIED, NULL, NULL) != XGE_ERROR_INVALID_ARGUMENT ) {
		return 449;
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
	xgeSoundFallbackClear();
	if ( xgeSoundFallbackGet(&tSound) != XGE_ERROR_RESOURCE_FAILED ) {
		return 81;
	}
	if ( xgeSoundFallbackSet(NULL) != XGE_ERROR_INVALID_ARGUMENT ) {
		return 82;
	}
	if ( xgeSoundFallbackSet("missing_fallback_sound.wav") != XGE_OK ) {
		return 83;
	}
	memset(&tSound, 0, sizeof(tSound));
	if ( xgeSoundFallbackGet(&tSound) != XGE_ERROR_NOT_INITIALIZED ) {
		xgeSoundFallbackClear();
		return 84;
	}
	xgeSoundFallbackClear();
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
	xge_font_t tCachedFont;
	xge_glyph_t tGlyph;
	xge_glyph_metrics_t tMetrics;
	xge_vec2_t tSize;
	void* pXRFData;
	int iXRFSize;
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
	pXRFData = NULL;
	iXRFSize = 0;
	if ( xgeFontBuildXRFMemory(&tFont, 'A', 1, &pXRFData, &iXRFSize) != XGE_OK || pXRFData == NULL || iXRFSize <= (int)sizeof(xge_xrf_header_t) ) {
		xgeFontFree(&tFont);
		return 117;
	}
	memset(&tCachedFont, 0, sizeof(tCachedFont));
	if ( xgeFontLoadXRFMemory(&tCachedFont, pXRFData, iXRFSize) != XGE_OK ) {
		xgeMemoryFree(pXRFData);
		xgeFontFree(&tFont);
		return 118;
	}
	xgeMemoryFree(pXRFData);
	if ( xgeFontGlyphGet(&tCachedFont, 'A', &tMetrics) != XGE_OK || tMetrics.fAdvanceX != 8.0f ) {
		xgeFontFree(&tCachedFont);
		xgeFontFree(&tFont);
		return 119;
	}
	xgeFontFree(&tCachedFont);
	if ( xgeFontSaveXRF(&tFont, "xge_font_cache_test.xrf", 'A', 1) != XGE_OK ) {
		xgeFontFree(&tFont);
		return 120;
	}
	memset(&tCachedFont, 0, sizeof(tCachedFont));
	if ( xgeFontLoadCached(&tCachedFont, "bad.ttf", "xge_font_cache_test.xrf", 16.0f, 'A', 1) != XGE_OK ) {
		xgeFontFree(&tFont);
		return 121;
	}
	xgeFontFree(&tCachedFont);
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
	xge_xui_widget pGrand;
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
	xgeXuiSetSafeAreaPx(&tXui, 10.0f, 20.0f, 30.0f, 40.0f);
	if ( xgeXuiUpdate(&tXui, 0.0f) != XGE_OK ) {
		xgeXuiUnit(&tXui);
		return 179;
	}
	if ( pRoot->tRect.fW != 320.0f || pRoot->tRect.fH != 200.0f || pRoot->tContentRect.fX != 10.0f || pRoot->tContentRect.fY != 20.0f || pRoot->tContentRect.fW != 280.0f || pRoot->tContentRect.fH != 140.0f ) {
		xgeXuiUnit(&tXui);
		return 180;
	}
	xgeXuiSetSafeAreaPx(&tXui, 0.0f, 0.0f, 0.0f, 0.0f);
	pChild = xgeXuiWidgetCreate();
	pGrand = xgeXuiWidgetCreate();
	if ( (pChild == NULL) || (pGrand == NULL) ) {
		xgeXuiWidgetFree(pChild);
		xgeXuiWidgetFree(pGrand);
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
	tRect.fX = 1.0f;
	tRect.fY = 2.0f;
	tRect.fW = 5.0f;
	tRect.fH = 6.0f;
	xgeXuiWidgetSetRect(pGrand, tRect);
	xgeXuiWidgetSetSize(pGrand, xgeXuiSizePx(5.0f), xgeXuiSizePx(6.0f));
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
		xgeXuiWidgetFree(pGrand);
		xgeXuiWidgetFree(pChild);
		xgeXuiUnit(&tXui);
		return 126;
	}
	if ( xgeXuiWidgetAdd(pChild, pGrand) != XGE_OK ) {
		xgeXuiWidgetFree(pGrand);
		xgeXuiUnit(&tXui);
		return 177;
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
	xgeClear(XGE_COLOR_RGBA(0, 0, 0, 255));
	iPaintCount = xgeXuiPaint(&tXui);
	if ( iPaintCount != 1 || tXui.iPaintCommandCount != 1 || tHostState.iDrawRect != 2 ) {
		xgeXuiUnit(&tXui);
		xgePresent();
		return 160;
	}
	xgePresent();
	xgeXuiUpdate(&tXui, 0.0f);
	if ( (xgeXuiWidgetGetFlags(pRoot) & XGE_XUI_WIDGET_DIRTY_LAYOUT) != 0 || (xgeXuiWidgetGetFlags(pChild) & XGE_XUI_WIDGET_DIRTY_LAYOUT) != 0 || (xgeXuiWidgetGetFlags(pGrand) & XGE_XUI_WIDGET_DIRTY_LAYOUT) != 0 ) {
		xgeXuiUnit(&tXui);
		return 139;
	}
	xgeXuiLayoutBatchBegin(&tXui);
	xgeXuiWidgetSetRect(pChild, tRect);
	xgeXuiWidgetSetSize(pGrand, xgeXuiSizePx(11.0f), xgeXuiSizePx(12.0f));
	xgeXuiWidgetSetBackground(pChild, XGE_COLOR_RGBA(0, 255, 0, 255));
	if ( (xgeXuiWidgetGetFlags(pRoot) & XGE_XUI_WIDGET_DIRTY_LAYOUT) != 0 || (xgeXuiWidgetGetFlags(pChild) & XGE_XUI_WIDGET_DIRTY_LAYOUT) != 0 || (xgeXuiWidgetGetFlags(pGrand) & XGE_XUI_WIDGET_DIRTY_LAYOUT) != 0 || tXui.bLayoutBatchDirtyLayout == 0 || tXui.bLayoutBatchDirtyPaint == 0 ) {
		xgeXuiUnit(&tXui);
		return 156;
	}
	xgeXuiLayoutBatchEnd(&tXui);
	if ( (xgeXuiWidgetGetFlags(pRoot) & XGE_XUI_WIDGET_DIRTY_LAYOUT) == 0 || (xgeXuiWidgetGetFlags(pRoot) & XGE_XUI_WIDGET_DIRTY_PAINT) == 0 || (xgeXuiWidgetGetFlags(pChild) & XGE_XUI_WIDGET_DIRTY_LAYOUT) == 0 || (xgeXuiWidgetGetFlags(pGrand) & XGE_XUI_WIDGET_DIRTY_LAYOUT) == 0 || tXui.iLayoutBatchDepth != 0 ) {
		xgeXuiUnit(&tXui);
		return 157;
	}
	if ( xgeXuiUpdate(&tXui, 0.0f) != XGE_OK || (xgeXuiWidgetGetFlags(pRoot) & XGE_XUI_WIDGET_DIRTY_LAYOUT) != 0 ) {
		xgeXuiUnit(&tXui);
		return 158;
	}
	tRect = xgeXuiWidgetGetRect(pGrand);
	if ( tRect.fW != 11.0f || tRect.fH != 12.0f || (xgeXuiWidgetGetFlags(pGrand) & XGE_XUI_WIDGET_DIRTY_LAYOUT) != 0 ) {
		xgeXuiUnit(&tXui);
		return 178;
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
#if XGE_HAS_DEBUGMODE
	xge_rect_t tDirty;
#endif

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
#if XGE_HAS_DEBUGMODE
	xgedbgDirtyRectClear();
#endif
	xgeXuiWidgetMarkPaint(pChild);
#if XGE_HAS_DEBUGMODE
	if ( xgedbgDirtyRectCount() != 0 ) {
		xgeXuiWidgetFree(pChild);
		xgeXuiUnit(&tXui);
		return 425;
	}
#endif
	if ( xgeXuiWidgetAdd(pRoot, pChild) != XGE_OK ) {
		xgeXuiWidgetFree(pChild);
		xgeXuiUnit(&tXui);
		return 418;
	}
	if ( xgeXuiRefreshNeeded(&tXui) == 0 || tHostState.iRequestRefresh != 2 ) {
		xgeXuiUnit(&tXui);
		return 419;
	}
#if XGE_HAS_DEBUGMODE
	xgedbgDirtyRectClear();
#endif
	xgeXuiWidgetMarkPaint(pChild);
	if ( tHostState.iRequestRefresh != 2 ) {
		xgeXuiUnit(&tXui);
		return 420;
	}
#if XGE_HAS_DEBUGMODE
	if ( xgedbgDirtyRectCount() != 1 || xgedbgDirtyRectGet(0, &tDirty) != XGE_OK || tDirty.fX != 1.0f || tDirty.fY != 2.0f || tDirty.fW != 3.0f || tDirty.fH != 4.0f ) {
		xgeXuiUnit(&tXui);
		return 426;
	}
#endif
	xgeXuiRefreshClear(&tXui);
#if XGE_HAS_DEBUGMODE
	xgedbgDirtyRectClear();
#endif
	xgeXuiLayoutBatchBegin(&tXui);
	xgeXuiWidgetSetRect(pChild, tRect);
	xgeXuiWidgetSetBackground(pChild, XGE_COLOR_RGBA(10, 20, 30, 255));
	if ( xgeXuiRefreshNeeded(&tXui) != 0 || tHostState.iRequestRefresh != 2 ) {
		xgeXuiUnit(&tXui);
		return 421;
	}
#if XGE_HAS_DEBUGMODE
	if ( xgedbgDirtyRectCount() != 0 ) {
		xgeXuiUnit(&tXui);
		return 427;
	}
#endif
	xgeXuiLayoutBatchEnd(&tXui);
	if ( xgeXuiRefreshNeeded(&tXui) == 0 || tHostState.iRequestRefresh != 3 ) {
		xgeXuiUnit(&tXui);
		return 422;
	}
#if XGE_HAS_DEBUGMODE
	if ( xgedbgDirtyRectCount() != 1 || xgedbgDirtyRectGet(0, &tDirty) != XGE_OK || tDirty.fX != 0.0f || tDirty.fY != 0.0f || tDirty.fW != 320.0f || tDirty.fH != 200.0f ) {
		xgeXuiUnit(&tXui);
		return 428;
	}
#endif
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
	if ( xgeXuiGetThemeVersion(&tXui) != 1 ) {
		xgeXuiUnit(&tXui);
		return 411;
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
	if ( xgeXuiGetThemeVersion(&tXui) != 2 ) {
		xgeXuiUnit(&tXui);
		return 412;
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
	if ( (pButton->tStyle.tPadding.tLeft.fValue != 6.0f) || ((xgeXuiWidgetGetFlags(pButton) & XGE_XUI_WIDGET_DIRTY_LAYOUT) == 0) || ((xgeXuiWidgetGetFlags(pButton) & XGE_XUI_WIDGET_DIRTY_PAINT) == 0) || ((xgeXuiWidgetGetFlags(pButton) & XGE_XUI_WIDGET_DIRTY_STYLE) == 0) || (pButton->iStyleVersion == 0) ) {
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

static xge_vec2_t __testXuiMeasureSmall(xge_xui_widget pWidget, void* pUser)
{
	xge_vec2_t tSize;

	(void)pWidget;
	(void)pUser;
	tSize.fX = 40.0f;
	tSize.fY = 10.0f;
	return tSize;
}

static int __testXuiLayoutModes(void)
{
	xge_xui_context_t tXui;
	xge_xui_widget pRoot;
	xge_xui_widget pPanel;
	xge_xui_widget pA;
	xge_xui_widget pB;
	xge_xui_widget pC;
	xge_xui_widget pD;
	xge_xui_widget pE;
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
	pD = NULL;
	pE = NULL;
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
	xgeXuiWidgetSetMeasure(pA, __testXuiMeasureSmall);
	xgeXuiWidgetSetSize(pA, xgeXuiSizeContent(), xgeXuiSizeContent());
	xgeXuiWidgetSetAlign(pA, XGE_XUI_ALIGN_START, XGE_XUI_ALIGN_STRETCH);
	xgeXuiUpdate(&tXui, 0.0f);
	tRect = xgeXuiWidgetGetRect(pA);
	if ( tRect.fW != 40.0f || tRect.fH != 100.0f ) {
		xgeXuiUnit(&tXui);
		return 164;
	}
	xgeXuiWidgetSetMeasure(pA, NULL);
	xgeXuiWidgetSetAlign(pA, XGE_XUI_ALIGN_STRETCH, XGE_XUI_ALIGN_STRETCH);

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
	xgeXuiWidgetSetMeasure(pA, __testXuiMeasureSmall);
	xgeXuiWidgetSetSize(pA, xgeXuiSizeContent(), xgeXuiSizeContent());
	xgeXuiWidgetSetAlign(pA, XGE_XUI_ALIGN_STRETCH, XGE_XUI_ALIGN_START);
	xgeXuiUpdate(&tXui, 0.0f);
	tRect = xgeXuiWidgetGetRect(pA);
	if ( tRect.fW != 300.0f || tRect.fH != 10.0f ) {
		xgeXuiUnit(&tXui);
		return 165;
	}
	xgeXuiWidgetSetMeasure(pA, NULL);
	xgeXuiWidgetSetAlign(pA, XGE_XUI_ALIGN_STRETCH, XGE_XUI_ALIGN_STRETCH);

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
	xgeXuiWidgetSetMeasure(pA, __testXuiMeasureSmall);
	xgeXuiWidgetSetSize(pA, xgeXuiSizeContent(), xgeXuiSizeContent());
	xgeXuiWidgetSetAlign(pA, XGE_XUI_ALIGN_STRETCH, XGE_XUI_ALIGN_STRETCH);
	xgeXuiUpdate(&tXui, 0.0f);
	tRect = xgeXuiWidgetGetRect(pA);
	if ( tRect.fW != 284.0f || tRect.fH != 80.0f ) {
		xgeXuiUnit(&tXui);
		return 166;
	}
	xgeXuiWidgetSetMeasure(pA, NULL);

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
	xgeXuiWidgetSetGrid(pPanel, 3, 20.0f, 5.0f, 4.0f);
	xgeXuiWidgetSetGridColumnSpan(pA, 2);
	xgeXuiUpdate(&tXui, 0.0f);
	tRect = xgeXuiWidgetGetRect(pA);
	if ( tRect.fX != 10.0f || tRect.fY != 10.0f || tRect.fW != 185.0f || tRect.fH != 20.0f ) {
		xgeXuiUnit(&tXui);
		return 188;
	}
	tRect = xgeXuiWidgetGetRect(pB);
	if ( tRect.fX != 201.0f || tRect.fY != 12.0f || tRect.fW != 86.0f || tRect.fH != 14.0f ) {
		xgeXuiUnit(&tXui);
		return 189;
	}
	tRect = xgeXuiWidgetGetRect(pC);
	if ( tRect.fX != 10.0f || tRect.fY != 34.0f || tRect.fW != 90.0f || tRect.fH != 20.0f ) {
		xgeXuiUnit(&tXui);
		return 190;
	}
	xgeXuiWidgetSetGridColumnSpan(pA, 1);
	xgeXuiWidgetSetGrid(pPanel, 2, 0.0f, 10.0f, 4.0f);
	xgeXuiWidgetSetSize(pA, xgeXuiSizeGrow(1.0f), xgeXuiSizeGrow(1.0f));
	xgeXuiWidgetSetSize(pB, xgeXuiSizeGrow(1.0f), xgeXuiSizeGrow(1.0f));
	xgeXuiWidgetSetSize(pC, xgeXuiSizeGrow(1.0f), xgeXuiSizeGrow(1.0f));
	xgeXuiUpdate(&tXui, 0.0f);
	tRect = xgeXuiWidgetGetRect(pA);
	if ( tRect.fW != 135.0f || tRect.fH != 135.0f ) {
		xgeXuiUnit(&tXui);
		return 174;
	}
	tRect = xgeXuiWidgetGetRect(pB);
	if ( tRect.fW != 131.0f || tRect.fH != 129.0f ) {
		xgeXuiUnit(&tXui);
		return 175;
	}
	tRect = xgeXuiWidgetGetRect(pC);
	if ( tRect.fY != 149.0f || tRect.fW != 135.0f || tRect.fH != 135.0f ) {
		xgeXuiUnit(&tXui);
		return 176;
	}
	xgeXuiWidgetSetGrid(pPanel, 2, 20.0f, 10.0f, 4.0f);
	xgeXuiWidgetSetMeasure(pC, __testXuiMeasureSmall);
	xgeXuiWidgetSetSize(pC, xgeXuiSizeContent(), xgeXuiSizeContent());
	xgeXuiUpdate(&tXui, 0.0f);
	tRect = xgeXuiWidgetGetRect(pC);
	if ( tRect.fW != 135.0f || tRect.fH != 20.0f ) {
		xgeXuiUnit(&tXui);
		return 167;
	}
	xgeXuiWidgetSetMeasure(pC, NULL);

	xgeXuiWidgetSetLayout(pPanel, XGE_XUI_LAYOUT_ROW);
	xgeXuiWidgetSetPaddingPx(pPanel, 0.0f, 0.0f, 0.0f, 0.0f);
	xgeXuiWidgetSetGap(pPanel, 10.0f);
	xgeXuiWidgetSetJustify(pPanel, XGE_XUI_JUSTIFY_SPACE_BETWEEN);
	xgeXuiWidgetSetMarginPx(pA, 0.0f, 0.0f, 0.0f, 0.0f);
	xgeXuiWidgetSetMarginPx(pB, 0.0f, 0.0f, 0.0f, 0.0f);
	xgeXuiWidgetSetMarginPx(pC, 0.0f, 0.0f, 0.0f, 0.0f);
	xgeXuiWidgetSetAlign(pA, XGE_XUI_ALIGN_START, XGE_XUI_ALIGN_START);
	xgeXuiWidgetSetAlign(pB, XGE_XUI_ALIGN_START, XGE_XUI_ALIGN_START);
	xgeXuiWidgetSetAlign(pC, XGE_XUI_ALIGN_START, XGE_XUI_ALIGN_START);
	xgeXuiWidgetSetSize(pA, xgeXuiSizePx(20.0f), xgeXuiSizePx(20.0f));
	xgeXuiWidgetSetSize(pB, xgeXuiSizePx(20.0f), xgeXuiSizePx(20.0f));
	xgeXuiWidgetSetSize(pC, xgeXuiSizePx(20.0f), xgeXuiSizePx(20.0f));
	xgeXuiUpdate(&tXui, 0.0f);
	tRect = xgeXuiWidgetGetRect(pA);
	if ( tRect.fX != 0.0f || tRect.fW != 20.0f ) {
		xgeXuiUnit(&tXui);
		return 160;
	}
	tRect = xgeXuiWidgetGetRect(pB);
	if ( tRect.fX != 140.0f || tRect.fW != 20.0f ) {
		xgeXuiUnit(&tXui);
		return 161;
	}
	tRect = xgeXuiWidgetGetRect(pC);
	if ( tRect.fX != 280.0f || tRect.fW != 20.0f ) {
		xgeXuiUnit(&tXui);
		return 162;
	}
	if ( xgeXuiWidgetGetDesiredSize(pPanel).fX != 80.0f || xgeXuiWidgetGetDesiredSize(pPanel).fY != 20.0f ) {
		xgeXuiUnit(&tXui);
		return 163;
	}
	xgeXuiWidgetSetJustify(pPanel, XGE_XUI_JUSTIFY_START);
	xgeXuiWidgetSetGap(pPanel, 0.0f);
	xgeXuiWidgetSetAlign(pA, XGE_XUI_ALIGN_STRETCH, XGE_XUI_ALIGN_STRETCH);
	xgeXuiWidgetSetAlign(pB, XGE_XUI_ALIGN_STRETCH, XGE_XUI_ALIGN_STRETCH);
	xgeXuiWidgetSetSize(pA, xgeXuiSizeGrow(1.0f), xgeXuiSizePx(20.0f));
	xgeXuiWidgetSetSize(pB, xgeXuiSizeGrow(1.0f), xgeXuiSizePx(20.0f));
	xgeXuiWidgetSetSize(pC, xgeXuiSizePx(0.0f), xgeXuiSizePx(20.0f));
	xgeXuiWidgetSetMaxSize(pA, xgeXuiSizePx(50.0f), xgeXuiSizePx(0.0f));
	xgeXuiUpdate(&tXui, 0.0f);
	tRect = xgeXuiWidgetGetRect(pA);
	if ( tRect.fX != 0.0f || tRect.fW != 50.0f ) {
		xgeXuiUnit(&tXui);
		return 168;
	}
	tRect = xgeXuiWidgetGetRect(pB);
	if ( tRect.fX != 50.0f || tRect.fW != 250.0f ) {
		xgeXuiUnit(&tXui);
		return 169;
	}
	xgeXuiWidgetSetMaxSize(pA, xgeXuiSizePx(0.0f), xgeXuiSizePx(0.0f));
	xgeXuiWidgetSetMinSize(pA, xgeXuiSizePx(220.0f), xgeXuiSizePx(0.0f));
	xgeXuiUpdate(&tXui, 0.0f);
	tRect = xgeXuiWidgetGetRect(pA);
	if ( tRect.fW != 220.0f ) {
		xgeXuiUnit(&tXui);
		return 170;
	}
	tRect = xgeXuiWidgetGetRect(pB);
	if ( tRect.fX != 220.0f || tRect.fW != 80.0f ) {
		xgeXuiUnit(&tXui);
		return 171;
	}
	xgeXuiWidgetSetMinSize(pA, xgeXuiSizePx(0.0f), xgeXuiSizePx(0.0f));
	xgeXuiWidgetSetLayout(pPanel, XGE_XUI_LAYOUT_COLUMN);
	xgeXuiWidgetSetSize(pA, xgeXuiSizePx(20.0f), xgeXuiSizeGrow(1.0f));
	xgeXuiWidgetSetSize(pB, xgeXuiSizePx(20.0f), xgeXuiSizeGrow(1.0f));
	xgeXuiWidgetSetSize(pC, xgeXuiSizePx(20.0f), xgeXuiSizePx(0.0f));
	xgeXuiWidgetSetMaxSize(pA, xgeXuiSizePx(0.0f), xgeXuiSizePx(30.0f));
	xgeXuiUpdate(&tXui, 0.0f);
	tRect = xgeXuiWidgetGetRect(pA);
	if ( tRect.fY != 0.0f || tRect.fH != 30.0f ) {
		xgeXuiUnit(&tXui);
		return 172;
	}
	tRect = xgeXuiWidgetGetRect(pB);
	if ( tRect.fY != 30.0f || tRect.fH != 70.0f ) {
		xgeXuiUnit(&tXui);
		return 173;
	}

	pD = xgeXuiWidgetCreate();
	pE = xgeXuiWidgetCreate();
	if ( (pD == NULL) || (pE == NULL) ) {
		xgeXuiWidgetFree(pD);
		xgeXuiWidgetFree(pE);
		xgeXuiUnit(&tXui);
		return 191;
	}
	xgeXuiWidgetAdd(pPanel, pD);
	xgeXuiWidgetAdd(pPanel, pE);
	tRect.fX = 0.0f;
	tRect.fY = 0.0f;
	tRect.fW = 300.0f;
	tRect.fH = 200.0f;
	xgeXuiWidgetSetRect(pPanel, tRect);
	xgeXuiWidgetSetLayout(pPanel, XGE_XUI_LAYOUT_DOCK);
	xgeXuiWidgetSetPaddingPx(pPanel, 10.0f, 10.0f, 10.0f, 10.0f);
	xgeXuiWidgetSetGap(pPanel, 0.0f);
	xgeXuiWidgetSetMarginPx(pA, 0.0f, 0.0f, 0.0f, 0.0f);
	xgeXuiWidgetSetMarginPx(pB, 0.0f, 0.0f, 0.0f, 0.0f);
	xgeXuiWidgetSetMarginPx(pC, 0.0f, 0.0f, 0.0f, 0.0f);
	xgeXuiWidgetSetMarginPx(pD, 0.0f, 0.0f, 0.0f, 0.0f);
	xgeXuiWidgetSetMarginPx(pE, 0.0f, 0.0f, 0.0f, 0.0f);
	xgeXuiWidgetSetAlign(pA, XGE_XUI_ALIGN_STRETCH, XGE_XUI_ALIGN_STRETCH);
	xgeXuiWidgetSetAlign(pB, XGE_XUI_ALIGN_STRETCH, XGE_XUI_ALIGN_STRETCH);
	xgeXuiWidgetSetAlign(pC, XGE_XUI_ALIGN_STRETCH, XGE_XUI_ALIGN_STRETCH);
	xgeXuiWidgetSetAlign(pD, XGE_XUI_ALIGN_STRETCH, XGE_XUI_ALIGN_STRETCH);
	xgeXuiWidgetSetAlign(pE, XGE_XUI_ALIGN_STRETCH, XGE_XUI_ALIGN_STRETCH);
	xgeXuiWidgetSetMinSize(pA, xgeXuiSizePx(0.0f), xgeXuiSizePx(0.0f));
	xgeXuiWidgetSetMaxSize(pA, xgeXuiSizePx(0.0f), xgeXuiSizePx(0.0f));
	xgeXuiWidgetSetMaxSize(pB, xgeXuiSizePx(0.0f), xgeXuiSizePx(0.0f));
	xgeXuiWidgetSetSize(pA, xgeXuiSizeGrow(1.0f), xgeXuiSizePx(20.0f));
	xgeXuiWidgetSetSize(pB, xgeXuiSizeGrow(1.0f), xgeXuiSizePx(30.0f));
	xgeXuiWidgetSetSize(pC, xgeXuiSizePx(40.0f), xgeXuiSizeGrow(1.0f));
	xgeXuiWidgetSetSize(pD, xgeXuiSizePx(50.0f), xgeXuiSizeGrow(1.0f));
	xgeXuiWidgetSetSize(pE, xgeXuiSizeGrow(1.0f), xgeXuiSizeGrow(1.0f));
	xgeXuiWidgetSetDock(pA, XGE_XUI_DOCK_TOP);
	xgeXuiWidgetSetDock(pB, XGE_XUI_DOCK_BOTTOM);
	xgeXuiWidgetSetDock(pC, XGE_XUI_DOCK_LEFT);
	xgeXuiWidgetSetDock(pD, XGE_XUI_DOCK_RIGHT);
	xgeXuiWidgetSetDock(pE, XGE_XUI_DOCK_FILL);
	xgeXuiUpdate(&tXui, 0.0f);
	tRect = xgeXuiWidgetGetRect(pA);
	if ( tRect.fX != 10.0f || tRect.fY != 10.0f || tRect.fW != 280.0f || tRect.fH != 20.0f ) {
		xgeXuiUnit(&tXui);
		return 192;
	}
	tRect = xgeXuiWidgetGetRect(pB);
	if ( tRect.fX != 10.0f || tRect.fY != 160.0f || tRect.fW != 280.0f || tRect.fH != 30.0f ) {
		xgeXuiUnit(&tXui);
		return 193;
	}
	tRect = xgeXuiWidgetGetRect(pC);
	if ( tRect.fX != 10.0f || tRect.fY != 30.0f || tRect.fW != 40.0f || tRect.fH != 130.0f ) {
		xgeXuiUnit(&tXui);
		return 194;
	}
	tRect = xgeXuiWidgetGetRect(pD);
	if ( tRect.fX != 240.0f || tRect.fY != 30.0f || tRect.fW != 50.0f || tRect.fH != 130.0f ) {
		xgeXuiUnit(&tXui);
		return 195;
	}
	tRect = xgeXuiWidgetGetRect(pE);
	if ( tRect.fX != 50.0f || tRect.fY != 30.0f || tRect.fW != 190.0f || tRect.fH != 130.0f || xgeXuiWidgetGetDock(pC) != XGE_XUI_DOCK_LEFT ) {
		xgeXuiUnit(&tXui);
		return 196;
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
	if ( pEvent->iType == XGE_EVENT_XUI_POINTER_ENTER ) {
		g_iXuiEventLog = (g_iXuiEventLog * 10) + 6;
		return XGE_XUI_EVENT_CONTINUE;
	}
	if ( pEvent->iType == XGE_EVENT_XUI_POINTER_LEAVE ) {
		g_iXuiEventLog = (g_iXuiEventLog * 10) + 7;
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
	if ( pEvent->iType == XGE_EVENT_XUI_CAPTURE_LOST ) {
		g_iXuiEventLog = 6;
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
	xgeXuiWidgetSetPaddingPx(pParent, 0.0f, 0.0f, 140.0f, 0.0f);
	tRect.fX = 80.0f;
	tRect.fY = 10.0f;
	tRect.fW = 50.0f;
	tRect.fH = 50.0f;
	xgeXuiWidgetSetRect(pChild, tRect);
	xgeXuiUpdate(&tXui, 0.0f);
	pHit = xgeXuiHitTest(&tXui, 90.0f, 20.0f);
	if ( pHit != pChild ) {
		xgeXuiUnit(&tXui);
		return 186;
	}
	xgeXuiWidgetSetClip(pParent, 1);
	xgeXuiUpdate(&tXui, 0.0f);
	pHit = xgeXuiHitTest(&tXui, 90.0f, 20.0f);
	if ( pHit != pParent ) {
		xgeXuiUnit(&tXui);
		return 187;
	}
	xgeXuiWidgetSetClip(pParent, 0);
	xgeXuiWidgetSetPaddingPx(pParent, 0.0f, 0.0f, 0.0f, 0.0f);
	tRect.fX = 10.0f;
	tRect.fY = 10.0f;
	tRect.fW = 50.0f;
	tRect.fH = 50.0f;
	xgeXuiWidgetSetRect(pChild, tRect);
	xgeXuiUpdate(&tXui, 0.0f);
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
	tEvent.iType = XGE_EVENT_KEY_DOWN;
	tEvent.iParam1 = XGE_KEY_TAB;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || tXui.pFocus != pParent ) {
		xgeXuiUnit(&tXui);
		return 180;
	}
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || tXui.pFocus != pChild ) {
		xgeXuiUnit(&tXui);
		return 181;
	}
	tEvent.iParam2 = XGE_KEY_MOD_SHIFT;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || tXui.pFocus != pParent ) {
		xgeXuiUnit(&tXui);
		return 182;
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
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_KEY_DOWN;
	tEvent.iParam1 = XGE_KEY_ESCAPE;
	g_iXuiEventLog = 0;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || tXui.pCapture != NULL || g_iXuiEventLog != 6 ) {
		xgeXuiUnit(&tXui);
		return 183;
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
	pParent->procEvent = __testXuiEventParent;
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_MOUSE_MOVE;
	tEvent.fX = 20.0f;
	tEvent.fY = 20.0f;
	g_iXuiEventLog = 0;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONTINUE || g_iXuiEventLog != 6 ) {
		xgeXuiUnit(&tXui);
		return 184;
	}
	tEvent.fX = 500.0f;
	tEvent.fY = 500.0f;
	g_iXuiEventLog = 0;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONTINUE || g_iXuiEventLog != 7 ) {
		xgeXuiUnit(&tXui);
		return 185;
	}

	xgeXuiUnit(&tXui);
	return 0;
}

static int g_iXuiButtonClicks;
static xge_xui_widget g_pXuiLastClickWidget;
static void* g_pXuiLastClickUser;

static void __testXuiButtonClick(xge_xui_widget pWidget, void* pUser)
{
	g_pXuiLastClickWidget = pWidget;
	g_pXuiLastClickUser = pUser;
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
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_KEY_DOWN;
	tEvent.iParam1 = XGE_KEY_ENTER;
	g_iXuiButtonClicks = 0;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || g_iXuiButtonClicks != 1 || tButton.iClickCount != 2 ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 213;
	}
	tEvent.iParam1 = XGE_KEY_SPACE;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || g_iXuiButtonClicks != 2 || tButton.iClickCount != 3 ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 214;
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

static int __testXuiIconButton(void)
{
	xui_host_test_t tHostState;
	xge_xui_host_t tHost;
	xge_texture_t tTexture;
	xge_xui_context_t tXui;
	xge_xui_icon_button_t tButton;
	xge_xui_widget pRoot;
	xge_xui_widget pWidget;
	xge_event_t tEvent;
	xge_rect_t tSrc;
	int iPaintCount;

	memset(&tHostState, 0, sizeof(tHostState));
	memset(&tHost, 0, sizeof(tHost));
	memset(&tTexture, 0, sizeof(tTexture));
	memset(&tXui, 0, sizeof(tXui));
	memset(&tButton, 0, sizeof(tButton));
	tHost.draw_rect = __testXuiHostDrawRect;
	tHost.draw_image = __testXuiHostDrawImage;
	tHost.draw_text_rect = __testXuiHostDrawTextRect;
	tHost.measure_text = __testXuiHostMeasureText;
	tHost.clip_set = __testXuiHostClipSet;
	tHost.clip_clear = __testXuiHostClipClear;
	tHost.pUser = &tHostState;
	tTexture.iWidth = 16;
	tTexture.iHeight = 16;
	tTexture.iFormat = XGE_PIXEL_RGBA8;
	tTexture.iRefCount = 1;
	if ( xgeXuiInit(&tXui) != XGE_OK ) {
		return 510;
	}
	xgeXuiSetHost(&tXui, &tHost);
	pRoot = xgeXuiRoot(&tXui);
	pWidget = xgeXuiWidgetCreate();
	if ( (pRoot == NULL) || (pWidget == NULL) ) {
		xgeXuiWidgetFree(pWidget);
		xgeXuiUnit(&tXui);
		return 511;
	}
	xgeXuiWidgetSetRect(pWidget, (xge_rect_t){ 10.0f, 10.0f, 40.0f, 40.0f });
	xgeXuiWidgetSetPaddingPx(pWidget, 6.0f, 6.0f, 6.0f, 6.0f);
	xgeXuiWidgetAdd(pRoot, pWidget);
	xgeXuiUpdate(&tXui, 0.0f);
	if ( xgeXuiIconButtonInit(&tButton, &tXui, pWidget, &tTexture) != XGE_OK ) {
		xgeXuiUnit(&tXui);
		return 512;
	}
	xgeXuiIconButtonSetClick(&tButton, __testXuiButtonClick, NULL);
	xgeXuiIconButtonSetIconColor(&tButton, XGE_COLOR_RGBA(1, 2, 3, 200));
	xgeXuiIconButtonSetMode(&tButton, XGE_XUI_IMAGE_FIT);
	tSrc = (xge_rect_t){ 1.0f, 2.0f, 8.0f, 8.0f };
	xgeXuiIconButtonSetSource(&tButton, tSrc);
	xgeXuiIconButtonSetColors(&tButton, XGE_COLOR_RGBA(1, 2, 3, 255), XGE_COLOR_RGBA(4, 5, 6, 255), XGE_COLOR_RGBA(7, 8, 9, 255), XGE_COLOR_RGBA(10, 11, 12, 255), XGE_COLOR_RGBA(13, 14, 15, 160));
	if ( xgeXuiWidgetIsFocusable(pWidget) == 0 || pWidget->procEvent != xgeXuiIconButtonEventProc || pWidget->procPaint != xgeXuiIconButtonPaintProc ) {
		xgeXuiUnit(&tXui);
		return 513;
	}
	iPaintCount = xgeXuiPaint(&tXui);
	if ( iPaintCount != 1 || tXui.iPaintCommandCount != 1 || tHostState.iDrawRect != 5 || tHostState.iDrawImage != 1 ) {
		xgeXuiUnit(&tXui);
		return 514;
	}
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_MOUSE_MOVE;
	tEvent.fX = 20.0f;
	tEvent.fY = 20.0f;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONTINUE || ((xgeXuiIconButtonGetState(&tButton) & XGE_XUI_STATE_HOVER) == 0) ) {
		xgeXuiUnit(&tXui);
		return 515;
	}
	tEvent.iType = XGE_EVENT_MOUSE_DOWN;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || ((xgeXuiIconButtonGetState(&tButton) & XGE_XUI_STATE_ACTIVE) == 0) || tXui.pFocus != pWidget ) {
		xgeXuiUnit(&tXui);
		return 516;
	}
	g_iXuiButtonClicks = 0;
	tEvent.iType = XGE_EVENT_MOUSE_UP;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || g_iXuiButtonClicks != 1 || tButton.iClickCount != 1 ) {
		xgeXuiUnit(&tXui);
		return 517;
	}
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_KEY_DOWN;
	tEvent.iParam1 = XGE_KEY_SPACE;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || g_iXuiButtonClicks != 2 || tButton.iClickCount != 2 ) {
		xgeXuiUnit(&tXui);
		return 518;
	}
	xgeXuiWidgetSetEnabled(pWidget, 0);
	tEvent.iType = XGE_EVENT_MOUSE_DOWN;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONTINUE || ((xgeXuiIconButtonGetState(&tButton) & XGE_XUI_STATE_DISABLED) == 0) ) {
		xgeXuiUnit(&tXui);
		return 519;
	}
	xgeXuiIconButtonUnit(&tButton);
	if ( pWidget->procEvent != NULL || pWidget->procPaint != NULL || pWidget->pUser != NULL ) {
		xgeXuiUnit(&tXui);
		return 520;
	}
	xgeXuiUnit(&tXui);
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
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_KEY_DOWN;
	tEvent.iParam1 = XGE_KEY_SPACE;
	g_iXuiToggleChanges = 0;
	g_iXuiToggleLastChecked = 0;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || xgeXuiToggleGetChecked(&tToggle) != 1 || g_iXuiToggleChanges != 1 || g_iXuiToggleLastChecked != 1 ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 272;
	}
	tEvent.iParam1 = XGE_KEY_ENTER;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || xgeXuiToggleGetChecked(&tToggle) != 0 || g_iXuiToggleChanges != 2 || g_iXuiToggleLastChecked != 0 ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 273;
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

static int g_iXuiCheckBoxChanges;
static int g_iXuiCheckBoxLastChecked;

static void __testXuiCheckBoxChange(xge_xui_widget pWidget, int bChecked, void* pUser)
{
	(void)pWidget;
	(void)pUser;
	g_iXuiCheckBoxChanges++;
	g_iXuiCheckBoxLastChecked = bChecked;
}

static int __testXuiCheckBox(void)
{
	xrf_test_blob2_t tBlob;
	xge_font_t tFont;
	xge_xui_context_t tXui;
	xge_xui_checkbox_t tCheckBox;
	xge_xui_widget pRoot;
	xge_xui_widget pWidget;
	xge_event_t tEvent;
	xge_rect_t tRect;
	int iPaintCount;

	__testXrfBlobMake(&tBlob);
	memset(&tFont, 0, sizeof(tFont));
	memset(&tXui, 0, sizeof(tXui));
	memset(&tCheckBox, 0, sizeof(tCheckBox));
	if ( xgeFontLoadXRFMemory(&tFont, &tBlob, sizeof(tBlob)) != XGE_OK ) {
		return 430;
	}
	if ( xgeXuiInit(&tXui) != XGE_OK ) {
		xgeFontFree(&tFont);
		return 431;
	}
	pRoot = xgeXuiRoot(&tXui);
	pWidget = xgeXuiWidgetCreate();
	if ( (pRoot == NULL) || (pWidget == NULL) ) {
		xgeXuiWidgetFree(pWidget);
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 432;
	}
	tRect.fX = 10.0f;
	tRect.fY = 10.0f;
	tRect.fW = 140.0f;
	tRect.fH = 32.0f;
	xgeXuiWidgetSetRect(pWidget, tRect);
	xgeXuiWidgetSetPaddingPx(pWidget, 3.0f, 3.0f, 3.0f, 3.0f);
	xgeXuiWidgetAdd(pRoot, pWidget);
	xgeXuiUpdate(&tXui, 0.0f);
	if ( xgeXuiCheckBoxInit(&tCheckBox, &tXui, pWidget) != XGE_OK ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 433;
	}
	xgeXuiCheckBoxSetChange(&tCheckBox, __testXuiCheckBoxChange, NULL);
	xgeXuiCheckBoxSetText(&tCheckBox, &tFont, "Check");
	xgeXuiCheckBoxSetTextColor(&tCheckBox, XGE_COLOR_RGBA(10, 20, 30, 255));
	xgeXuiCheckBoxSetColors(&tCheckBox, XGE_COLOR_RGBA(1, 2, 3, 0), XGE_COLOR_RGBA(4, 5, 6, 255), XGE_COLOR_RGBA(7, 8, 9, 255), XGE_COLOR_RGBA(10, 11, 12, 255), XGE_COLOR_RGBA(13, 14, 15, 160), XGE_COLOR_RGBA(16, 17, 18, 255), XGE_COLOR_RGBA(19, 20, 21, 255));
	if ( xgeXuiWidgetIsFocusable(pWidget) == 0 || pWidget->procEvent != xgeXuiCheckBoxEventProc || pWidget->procPaint != xgeXuiCheckBoxPaintProc ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 434;
	}
	iPaintCount = xgeXuiPaint(&tXui);
	if ( iPaintCount != 1 || tXui.iPaintCommandCount != 1 ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 435;
	}

	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_MOUSE_MOVE;
	tEvent.fX = 20.0f;
	tEvent.fY = 20.0f;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONTINUE || ((xgeXuiCheckBoxGetState(&tCheckBox) & XGE_XUI_STATE_HOVER) == 0) ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 436;
	}
	tEvent.iType = XGE_EVENT_MOUSE_DOWN;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || ((xgeXuiCheckBoxGetState(&tCheckBox) & XGE_XUI_STATE_ACTIVE) == 0) || tXui.pFocus != pWidget ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 437;
	}
	g_iXuiCheckBoxChanges = 0;
	g_iXuiCheckBoxLastChecked = 0;
	tEvent.iType = XGE_EVENT_MOUSE_UP;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || xgeXuiCheckBoxGetChecked(&tCheckBox) != 1 || g_iXuiCheckBoxChanges != 1 || g_iXuiCheckBoxLastChecked != 1 || tCheckBox.iChangeCount != 1 ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 438;
	}
	xgeXuiCheckBoxSetChecked(&tCheckBox, 0);
	if ( xgeXuiCheckBoxGetChecked(&tCheckBox) != 0 ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 439;
	}
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_KEY_DOWN;
	tEvent.iParam1 = XGE_KEY_SPACE;
	g_iXuiCheckBoxChanges = 0;
	g_iXuiCheckBoxLastChecked = 0;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || xgeXuiCheckBoxGetChecked(&tCheckBox) != 1 || g_iXuiCheckBoxChanges != 1 || g_iXuiCheckBoxLastChecked != 1 ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 440;
	}
	tEvent.iParam1 = XGE_KEY_ENTER;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || xgeXuiCheckBoxGetChecked(&tCheckBox) != 0 || g_iXuiCheckBoxChanges != 2 || g_iXuiCheckBoxLastChecked != 0 ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 441;
	}
	xgeXuiWidgetSetEnabled(pWidget, 0);
	tEvent.iType = XGE_EVENT_MOUSE_DOWN;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONTINUE || ((xgeXuiCheckBoxGetState(&tCheckBox) & XGE_XUI_STATE_DISABLED) == 0) ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 442;
	}
	xgeXuiCheckBoxUnit(&tCheckBox);
	if ( pWidget->procEvent != NULL || pWidget->procPaint != NULL || pWidget->pUser != NULL ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 443;
	}
	xgeXuiUnit(&tXui);
	xgeFontFree(&tFont);
	return 0;
}

static int g_iXuiRadioChanges;
static int g_iXuiRadioLastChecked;
static int g_iXuiRadioGroupChanges;
static int g_iXuiRadioGroupLastValue;

static void __testXuiRadioChange(xge_xui_widget pWidget, int bChecked, void* pUser)
{
	(void)pWidget;
	(void)pUser;
	g_iXuiRadioChanges++;
	g_iXuiRadioLastChecked = bChecked;
}

static void __testXuiRadioGroupChange(xge_xui_widget pWidget, int iIndex, void* pUser)
{
	(void)pWidget;
	(void)pUser;
	g_iXuiRadioGroupChanges++;
	g_iXuiRadioGroupLastValue = iIndex;
}

static int __testXuiRadio(void)
{
	xrf_test_blob2_t tBlob;
	xge_font_t tFont;
	xge_xui_context_t tXui;
	xge_xui_radio_group_t tGroup;
	xge_xui_radio_t tRadioA;
	xge_xui_radio_t tRadioB;
	xge_xui_widget pRoot;
	xge_xui_widget pA;
	xge_xui_widget pB;
	xge_event_t tEvent;

	__testXrfBlobMake(&tBlob);
	memset(&tFont, 0, sizeof(tFont));
	memset(&tXui, 0, sizeof(tXui));
	memset(&tGroup, 0, sizeof(tGroup));
	memset(&tRadioA, 0, sizeof(tRadioA));
	memset(&tRadioB, 0, sizeof(tRadioB));
	if ( xgeFontLoadXRFMemory(&tFont, &tBlob, sizeof(tBlob)) != XGE_OK ) {
		return 450;
	}
	if ( xgeXuiInit(&tXui) != XGE_OK ) {
		xgeFontFree(&tFont);
		return 451;
	}
	pRoot = xgeXuiRoot(&tXui);
	pA = xgeXuiWidgetCreate();
	pB = xgeXuiWidgetCreate();
	if ( (pRoot == NULL) || (pA == NULL) || (pB == NULL) ) {
		xgeXuiWidgetFree(pA);
		xgeXuiWidgetFree(pB);
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 452;
	}
	xgeXuiWidgetSetRect(pA, (xge_rect_t){ 10.0f, 10.0f, 130.0f, 30.0f });
	xgeXuiWidgetSetRect(pB, (xge_rect_t){ 10.0f, 44.0f, 130.0f, 30.0f });
	xgeXuiWidgetSetPaddingPx(pA, 3.0f, 3.0f, 3.0f, 3.0f);
	xgeXuiWidgetSetPaddingPx(pB, 3.0f, 3.0f, 3.0f, 3.0f);
	xgeXuiWidgetAdd(pRoot, pA);
	xgeXuiWidgetAdd(pRoot, pB);
	xgeXuiUpdate(&tXui, 0.0f);
	xgeXuiRadioGroupInit(&tGroup);
	xgeXuiRadioGroupSetChange(&tGroup, __testXuiRadioGroupChange, NULL);
	if ( xgeXuiRadioInit(&tRadioA, &tXui, pA) != XGE_OK || xgeXuiRadioInit(&tRadioB, &tXui, pB) != XGE_OK ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 453;
	}
	xgeXuiRadioSetText(&tRadioA, &tFont, "A");
	xgeXuiRadioSetText(&tRadioB, &tFont, "B");
	xgeXuiRadioSetChange(&tRadioA, __testXuiRadioChange, NULL);
	xgeXuiRadioSetChange(&tRadioB, __testXuiRadioChange, NULL);
	xgeXuiRadioSetGroup(&tRadioA, &tGroup, 10);
	xgeXuiRadioSetGroup(&tRadioB, &tGroup, 20);
	if ( pA->procEvent != xgeXuiRadioEventProc || pA->procPaint != xgeXuiRadioPaintProc || xgeXuiRadioGroupGetSelected(&tGroup) != -1 ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 454;
	}
	if ( xgeXuiPaint(&tXui) != 2 || tXui.iPaintCommandCount != 2 ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 455;
	}
	g_iXuiRadioChanges = 0;
	g_iXuiRadioGroupChanges = 0;
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_MOUSE_DOWN;
	tEvent.fX = 20.0f;
	tEvent.fY = 20.0f;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || ((xgeXuiRadioGetState(&tRadioA) & XGE_XUI_STATE_ACTIVE) == 0) ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 456;
	}
	tEvent.iType = XGE_EVENT_MOUSE_UP;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || xgeXuiRadioGetChecked(&tRadioA) != 1 || xgeXuiRadioGetChecked(&tRadioB) != 0 || xgeXuiRadioGroupGetSelected(&tGroup) != 10 || g_iXuiRadioGroupChanges != 1 || g_iXuiRadioGroupLastValue != 10 ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 457;
	}
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_MOUSE_DOWN;
	tEvent.fX = 20.0f;
	tEvent.fY = 54.0f;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 458;
	}
	tEvent.iType = XGE_EVENT_MOUSE_UP;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || xgeXuiRadioGetChecked(&tRadioA) != 0 || xgeXuiRadioGetChecked(&tRadioB) != 1 || xgeXuiRadioGroupGetSelected(&tGroup) != 20 || g_iXuiRadioGroupChanges != 2 || g_iXuiRadioGroupLastValue != 20 ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 459;
	}
	xgeXuiSetFocus(&tXui, pA);
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_KEY_DOWN;
	tEvent.iParam1 = XGE_KEY_SPACE;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || xgeXuiRadioGetChecked(&tRadioA) != 1 || xgeXuiRadioGetChecked(&tRadioB) != 0 || xgeXuiRadioGroupGetSelected(&tGroup) != 10 ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 460;
	}
	if ( g_iXuiRadioChanges == 0 || g_iXuiRadioLastChecked != 1 ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 461;
	}
	xgeXuiRadioUnit(&tRadioA);
	xgeXuiRadioUnit(&tRadioB);
	xgeXuiRadioGroupUnit(&tGroup);
	if ( pA->procEvent != NULL || pA->procPaint != NULL || pB->procEvent != NULL || pB->procPaint != NULL ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 462;
	}
	xgeXuiUnit(&tXui);
	xgeFontFree(&tFont);
	return 0;
}

static int g_iXuiSwitchChanges;
static int g_iXuiSwitchLastChecked;

static void __testXuiSwitchChange(xge_xui_widget pWidget, int bChecked, void* pUser)
{
	(void)pWidget;
	(void)pUser;
	g_iXuiSwitchChanges++;
	g_iXuiSwitchLastChecked = bChecked;
}

static int __testXuiSwitch(void)
{
	xrf_test_blob2_t tBlob;
	xge_font_t tFont;
	xge_xui_context_t tXui;
	xge_xui_switch_t tSwitch;
	xge_xui_widget pRoot;
	xge_xui_widget pWidget;
	xge_event_t tEvent;
	int iPaintCount;

	__testXrfBlobMake(&tBlob);
	memset(&tFont, 0, sizeof(tFont));
	memset(&tXui, 0, sizeof(tXui));
	memset(&tSwitch, 0, sizeof(tSwitch));
	if ( xgeFontLoadXRFMemory(&tFont, &tBlob, sizeof(tBlob)) != XGE_OK ) {
		return 470;
	}
	if ( xgeXuiInit(&tXui) != XGE_OK ) {
		xgeFontFree(&tFont);
		return 471;
	}
	pRoot = xgeXuiRoot(&tXui);
	pWidget = xgeXuiWidgetCreate();
	if ( (pRoot == NULL) || (pWidget == NULL) ) {
		xgeXuiWidgetFree(pWidget);
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 472;
	}
	xgeXuiWidgetSetRect(pWidget, (xge_rect_t){ 10.0f, 10.0f, 150.0f, 32.0f });
	xgeXuiWidgetSetPaddingPx(pWidget, 3.0f, 3.0f, 3.0f, 3.0f);
	xgeXuiWidgetAdd(pRoot, pWidget);
	xgeXuiUpdate(&tXui, 0.0f);
	if ( xgeXuiSwitchInit(&tSwitch, &tXui, pWidget) != XGE_OK ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 473;
	}
	xgeXuiSwitchSetChange(&tSwitch, __testXuiSwitchChange, NULL);
	xgeXuiSwitchSetText(&tSwitch, &tFont, "Switch");
	xgeXuiSwitchSetTextColor(&tSwitch, XGE_COLOR_RGBA(10, 20, 30, 255));
	xgeXuiSwitchSetColors(&tSwitch, XGE_COLOR_RGBA(1, 2, 3, 0), XGE_COLOR_RGBA(4, 5, 6, 255), XGE_COLOR_RGBA(7, 8, 9, 255), XGE_COLOR_RGBA(10, 11, 12, 255), XGE_COLOR_RGBA(13, 14, 15, 160), XGE_COLOR_RGBA(16, 17, 18, 255), XGE_COLOR_RGBA(19, 20, 21, 255), XGE_COLOR_RGBA(22, 23, 24, 255));
	if ( xgeXuiWidgetIsFocusable(pWidget) == 0 || pWidget->procEvent != xgeXuiSwitchEventProc || pWidget->procPaint != xgeXuiSwitchPaintProc ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 474;
	}
	iPaintCount = xgeXuiPaint(&tXui);
	if ( iPaintCount != 1 || tXui.iPaintCommandCount != 1 ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 475;
	}
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_MOUSE_MOVE;
	tEvent.fX = 20.0f;
	tEvent.fY = 20.0f;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONTINUE || ((xgeXuiSwitchGetState(&tSwitch) & XGE_XUI_STATE_HOVER) == 0) ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 476;
	}
	tEvent.iType = XGE_EVENT_MOUSE_DOWN;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || ((xgeXuiSwitchGetState(&tSwitch) & XGE_XUI_STATE_ACTIVE) == 0) || tXui.pFocus != pWidget ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 477;
	}
	g_iXuiSwitchChanges = 0;
	g_iXuiSwitchLastChecked = 0;
	tEvent.iType = XGE_EVENT_MOUSE_UP;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || xgeXuiSwitchGetChecked(&tSwitch) != 1 || g_iXuiSwitchChanges != 1 || g_iXuiSwitchLastChecked != 1 || tSwitch.iChangeCount != 1 ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 478;
	}
	xgeXuiSwitchSetChecked(&tSwitch, 0);
	if ( xgeXuiSwitchGetChecked(&tSwitch) != 0 ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 479;
	}
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_KEY_DOWN;
	tEvent.iParam1 = XGE_KEY_SPACE;
	g_iXuiSwitchChanges = 0;
	g_iXuiSwitchLastChecked = 0;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || xgeXuiSwitchGetChecked(&tSwitch) != 1 || g_iXuiSwitchChanges != 1 || g_iXuiSwitchLastChecked != 1 ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 480;
	}
	tEvent.iParam1 = XGE_KEY_ENTER;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || xgeXuiSwitchGetChecked(&tSwitch) != 0 || g_iXuiSwitchChanges != 2 || g_iXuiSwitchLastChecked != 0 ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 481;
	}
	xgeXuiWidgetSetEnabled(pWidget, 0);
	tEvent.iType = XGE_EVENT_MOUSE_DOWN;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONTINUE || ((xgeXuiSwitchGetState(&tSwitch) & XGE_XUI_STATE_DISABLED) == 0) ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 482;
	}
	xgeXuiSwitchUnit(&tSwitch);
	if ( pWidget->procEvent != NULL || pWidget->procPaint != NULL || pWidget->pUser != NULL ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 483;
	}
	xgeXuiUnit(&tXui);
	xgeFontFree(&tFont);
	return 0;
}

static int __testXuiSeparator(void)
{
	xge_xui_context_t tXui;
	xge_xui_separator_t tSeparator;
	xge_xui_widget pRoot;
	xge_xui_widget pWidget;
	int iPaintCount;

	memset(&tXui, 0, sizeof(tXui));
	memset(&tSeparator, 0, sizeof(tSeparator));
	if ( xgeXuiInit(&tXui) != XGE_OK ) {
		return 490;
	}
	pRoot = xgeXuiRoot(&tXui);
	pWidget = xgeXuiWidgetCreate();
	if ( (pRoot == NULL) || (pWidget == NULL) ) {
		xgeXuiWidgetFree(pWidget);
		xgeXuiUnit(&tXui);
		return 491;
	}
	xgeXuiWidgetSetRect(pWidget, (xge_rect_t){ 10.0f, 10.0f, 120.0f, 20.0f });
	xgeXuiWidgetSetPaddingPx(pWidget, 2.0f, 2.0f, 2.0f, 2.0f);
	xgeXuiWidgetAdd(pRoot, pWidget);
	xgeXuiUpdate(&tXui, 0.0f);
	if ( xgeXuiSeparatorInit(&tSeparator, pWidget) != XGE_OK ) {
		xgeXuiUnit(&tXui);
		return 492;
	}
	if ( pWidget->procPaint != xgeXuiSeparatorPaintProc || pWidget->procEvent != NULL || pWidget->pUser != &tSeparator ) {
		xgeXuiUnit(&tXui);
		return 493;
	}
	iPaintCount = xgeXuiPaint(&tXui);
	if ( iPaintCount != 1 || tXui.iPaintCommandCount != 1 ) {
		xgeXuiUnit(&tXui);
		return 494;
	}
	xgeXuiSeparatorSetOrientation(&tSeparator, XGE_XUI_SEPARATOR_VERTICAL);
	xgeXuiSeparatorSetThickness(&tSeparator, 3.0f);
	xgeXuiSeparatorSetColor(&tSeparator, XGE_COLOR_RGBA(1, 2, 3, 255));
	if ( tSeparator.iOrientation != XGE_XUI_SEPARATOR_VERTICAL || tSeparator.fThickness != 3.0f || tSeparator.iColor != XGE_COLOR_RGBA(1, 2, 3, 255) ) {
		xgeXuiUnit(&tXui);
		return 495;
	}
	xgeXuiSeparatorSetOrientation(&tSeparator, 99);
	xgeXuiSeparatorSetThickness(&tSeparator, -5.0f);
	if ( tSeparator.iOrientation != XGE_XUI_SEPARATOR_HORIZONTAL || tSeparator.fThickness != 1.0f ) {
		xgeXuiUnit(&tXui);
		return 496;
	}
	xgeXuiSeparatorUnit(&tSeparator);
	if ( pWidget->procPaint != NULL || pWidget->pUser != NULL ) {
		xgeXuiUnit(&tXui);
		return 497;
	}
	xgeXuiUnit(&tXui);
	return 0;
}

static int g_iXuiSplitterChanges;
static float g_fXuiSplitterLastValue;

static void __testXuiSplitterChange(xge_xui_widget pWidget, float fValue, void* pUser)
{
	(void)pWidget;
	(void)pUser;
	g_iXuiSplitterChanges++;
	g_fXuiSplitterLastValue = fValue;
}

static int __testXuiSplitter(void)
{
	xge_xui_context_t tXui;
	xge_xui_splitter_t tSplitter;
	xge_xui_widget pRoot;
	xge_xui_widget pWidget;
	xge_event_t tEvent;

	memset(&tXui, 0, sizeof(tXui));
	memset(&tSplitter, 0, sizeof(tSplitter));
	if ( xgeXuiInit(&tXui) != XGE_OK ) {
		return 530;
	}
	pRoot = xgeXuiRoot(&tXui);
	pWidget = xgeXuiWidgetCreate();
	if ( (pRoot == NULL) || (pWidget == NULL) ) {
		xgeXuiWidgetFree(pWidget);
		xgeXuiUnit(&tXui);
		return 531;
	}
	xgeXuiWidgetSetRect(pWidget, (xge_rect_t){ 20.0f, 10.0f, 12.0f, 100.0f });
	xgeXuiWidgetSetPaddingPx(pWidget, 2.0f, 2.0f, 2.0f, 2.0f);
	xgeXuiWidgetAdd(pRoot, pWidget);
	xgeXuiUpdate(&tXui, 0.0f);
	if ( xgeXuiSplitterInit(&tSplitter, &tXui, pWidget) != XGE_OK ) {
		xgeXuiUnit(&tXui);
		return 532;
	}
	xgeXuiSplitterSetChange(&tSplitter, __testXuiSplitterChange, NULL);
	xgeXuiSplitterSetRange(&tSplitter, -10.0f, 20.0f);
	xgeXuiSplitterSetValue(&tSplitter, 5.0f);
	xgeXuiSplitterSetColors(&tSplitter, XGE_COLOR_RGBA(1, 2, 3, 255), XGE_COLOR_RGBA(4, 5, 6, 255), XGE_COLOR_RGBA(7, 8, 9, 255), XGE_COLOR_RGBA(10, 11, 12, 255), XGE_COLOR_RGBA(13, 14, 15, 160));
	if ( pWidget->procEvent != xgeXuiSplitterEventProc || pWidget->procPaint != xgeXuiSplitterPaintProc || xgeXuiWidgetIsFocusable(pWidget) == 0 || xgeXuiSplitterGetValue(&tSplitter) != 5.0f ) {
		xgeXuiUnit(&tXui);
		return 533;
	}
	if ( xgeXuiPaint(&tXui) != 1 || tXui.iPaintCommandCount != 1 ) {
		xgeXuiUnit(&tXui);
		return 534;
	}
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_MOUSE_MOVE;
	tEvent.fX = 24.0f;
	tEvent.fY = 20.0f;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONTINUE || ((xgeXuiSplitterGetState(&tSplitter) & XGE_XUI_STATE_HOVER) == 0) ) {
		xgeXuiUnit(&tXui);
		return 535;
	}
	tEvent.iType = XGE_EVENT_MOUSE_DOWN;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || tXui.pCapture != pWidget || tXui.pFocus != pWidget || ((xgeXuiSplitterGetState(&tSplitter) & XGE_XUI_STATE_ACTIVE) == 0) ) {
		xgeXuiUnit(&tXui);
		return 536;
	}
	g_iXuiSplitterChanges = 0;
	g_fXuiSplitterLastValue = 0.0f;
	tEvent.iType = XGE_EVENT_MOUSE_MOVE;
	tEvent.fX = 34.0f;
	tEvent.fY = 20.0f;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || xgeXuiSplitterGetValue(&tSplitter) != 15.0f || g_iXuiSplitterChanges != 1 || g_fXuiSplitterLastValue != 15.0f ) {
		xgeXuiUnit(&tXui);
		return 537;
	}
	tEvent.fX = 80.0f;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || xgeXuiSplitterGetValue(&tSplitter) != 20.0f ) {
		xgeXuiUnit(&tXui);
		return 538;
	}
	tEvent.iType = XGE_EVENT_MOUSE_UP;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || tXui.pCapture != NULL ) {
		xgeXuiUnit(&tXui);
		return 539;
	}
	xgeXuiSplitterSetOrientation(&tSplitter, XGE_XUI_SEPARATOR_HORIZONTAL);
	xgeXuiSplitterSetValue(&tSplitter, 0.0f);
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_MOUSE_DOWN;
	tEvent.fX = 24.0f;
	tEvent.fY = 20.0f;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED ) {
		xgeXuiUnit(&tXui);
		return 540;
	}
	tEvent.iType = XGE_EVENT_MOUSE_MOVE;
	tEvent.fY = 35.0f;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || xgeXuiSplitterGetValue(&tSplitter) != 15.0f ) {
		xgeXuiUnit(&tXui);
		return 541;
	}
	xgeXuiSplitterUnit(&tSplitter);
	if ( pWidget->procEvent != NULL || pWidget->procPaint != NULL || pWidget->pUser != NULL ) {
		xgeXuiUnit(&tXui);
		return 542;
	}
	xgeXuiUnit(&tXui);
	return 0;
}

static int g_iXuiTabsChanges;
static int g_iXuiTabsLastSelected;

static void __testXuiTabsSelect(xge_xui_widget pWidget, int iIndex, void* pUser)
{
	(void)pWidget;
	(void)pUser;
	g_iXuiTabsChanges++;
	g_iXuiTabsLastSelected = iIndex;
}

static int __testXuiTabs(void)
{
	xrf_test_blob2_t tBlob;
	xge_font_t tFont;
	xge_xui_context_t tXui;
	xge_xui_tabs_t tTabs;
	xge_xui_widget pRoot;
	xge_xui_widget pWidget;
	xge_event_t tEvent;
	const char* arrItems[] = { "A", "B", "C" };

	__testXrfBlobMake(&tBlob);
	memset(&tFont, 0, sizeof(tFont));
	memset(&tXui, 0, sizeof(tXui));
	memset(&tTabs, 0, sizeof(tTabs));
	if ( xgeFontLoadXRFMemory(&tFont, &tBlob, sizeof(tBlob)) != XGE_OK ) {
		return 550;
	}
	if ( xgeXuiInit(&tXui) != XGE_OK ) {
		xgeFontFree(&tFont);
		return 551;
	}
	pRoot = xgeXuiRoot(&tXui);
	pWidget = xgeXuiWidgetCreate();
	if ( (pRoot == NULL) || (pWidget == NULL) ) {
		xgeXuiWidgetFree(pWidget);
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 552;
	}
	xgeXuiWidgetSetRect(pWidget, (xge_rect_t){ 10.0f, 10.0f, 180.0f, 32.0f });
	xgeXuiWidgetSetPaddingPx(pWidget, 0.0f, 0.0f, 0.0f, 0.0f);
	xgeXuiWidgetAdd(pRoot, pWidget);
	xgeXuiUpdate(&tXui, 0.0f);
	if ( xgeXuiTabsInit(&tTabs, &tXui, pWidget) != XGE_OK ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 553;
	}
	xgeXuiTabsSetItems(&tTabs, arrItems, 3);
	xgeXuiTabsSetFont(&tTabs, &tFont);
	xgeXuiTabsSetSelect(&tTabs, __testXuiTabsSelect, NULL);
	xgeXuiTabsSetTabSize(&tTabs, 50.0f, 24.0f);
	xgeXuiTabsSetColors(&tTabs, XGE_COLOR_RGBA(1, 2, 3, 0), XGE_COLOR_RGBA(4, 5, 6, 255), XGE_COLOR_RGBA(7, 8, 9, 255), XGE_COLOR_RGBA(10, 11, 12, 255), XGE_COLOR_RGBA(13, 14, 15, 160), XGE_COLOR_RGBA(16, 17, 18, 120), XGE_COLOR_RGBA(19, 20, 21, 255), XGE_COLOR_RGBA(22, 23, 24, 255));
	if ( pWidget->procEvent != xgeXuiTabsEventProc || pWidget->procPaint != xgeXuiTabsPaintProc || xgeXuiWidgetIsFocusable(pWidget) == 0 ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 554;
	}
	xgeXuiTabsSetSelected(&tTabs, 1);
	if ( xgeXuiTabsGetSelected(&tTabs) != 1 ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 555;
	}
	if ( xgeXuiPaint(&tXui) != 1 || tXui.iPaintCommandCount != 1 ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 556;
	}
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_MOUSE_MOVE;
	tEvent.fX = 20.0f;
	tEvent.fY = 20.0f;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONTINUE || tTabs.iHover != 0 || ((xgeXuiTabsGetState(&tTabs) & XGE_XUI_STATE_HOVER) == 0) ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 557;
	}
	tEvent.iType = XGE_EVENT_MOUSE_DOWN;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || tXui.pCapture != pWidget || tXui.pFocus != pWidget ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 558;
	}
	g_iXuiTabsChanges = 0;
	g_iXuiTabsLastSelected = -1;
	tEvent.iType = XGE_EVENT_MOUSE_UP;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || xgeXuiTabsGetSelected(&tTabs) != 0 || g_iXuiTabsChanges != 1 || g_iXuiTabsLastSelected != 0 || tXui.pCapture != NULL ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 559;
	}
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_KEY_DOWN;
	tEvent.iParam1 = XGE_KEY_RIGHT;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || xgeXuiTabsGetSelected(&tTabs) != 1 || g_iXuiTabsChanges != 2 || g_iXuiTabsLastSelected != 1 ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 560;
	}
	tEvent.iParam1 = XGE_KEY_LEFT;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || xgeXuiTabsGetSelected(&tTabs) != 0 || g_iXuiTabsChanges != 3 || g_iXuiTabsLastSelected != 0 ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 561;
	}
	xgeXuiTabsSetItems(&tTabs, arrItems, 0);
	if ( xgeXuiTabsGetSelected(&tTabs) != -1 || tTabs.iHover != -1 ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 562;
	}
	xgeXuiTabsUnit(&tTabs);
	if ( pWidget->procEvent != NULL || pWidget->procPaint != NULL || pWidget->pUser != NULL ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 563;
	}
	xgeXuiUnit(&tXui);
	xgeFontFree(&tFont);
	return 0;
}

static int g_iXuiToolbarChanges;
static int g_iXuiToolbarLastSelected;

static void __testXuiToolbarSelect(xge_xui_widget pWidget, int iIndex, void* pUser)
{
	(void)pWidget;
	if ( pUser == (void*)0x3456 ) {
		g_iXuiToolbarChanges++;
		g_iXuiToolbarLastSelected = iIndex;
	}
}

static int __testXuiToolbar(void)
{
	xge_xui_context_t tXui;
	xge_xui_toolbar_t tToolbar;
	xge_xui_widget pRoot;
	xge_xui_widget pWidget;
	xge_event_t tEvent;
	const char* arrText[] = { "New", "Pin", "", "Run" };
	int arrTypes[] = { XGE_XUI_TOOLBAR_ITEM_BUTTON, XGE_XUI_TOOLBAR_ITEM_TOGGLE, XGE_XUI_TOOLBAR_ITEM_SEPARATOR, XGE_XUI_TOOLBAR_ITEM_BUTTON };

	memset(&tXui, 0, sizeof(tXui));
	memset(&tToolbar, 0, sizeof(tToolbar));
	if ( xgeXuiInit(&tXui) != XGE_OK ) {
		return 733;
	}
	pRoot = xgeXuiRoot(&tXui);
	pWidget = xgeXuiWidgetCreate();
	if ( (pRoot == NULL) || (pWidget == NULL) ) {
		xgeXuiWidgetFree(pWidget);
		xgeXuiUnit(&tXui);
		return 734;
	}
	xgeXuiWidgetSetRect(pWidget, (xge_rect_t){ 10.0f, 10.0f, 220.0f, 30.0f });
	xgeXuiWidgetSetPaddingPx(pWidget, 0.0f, 0.0f, 0.0f, 0.0f);
	xgeXuiWidgetAdd(pRoot, pWidget);
	xgeXuiUpdate(&tXui, 0.0f);
	if ( xgeXuiToolbarInit(&tToolbar, &tXui, pWidget) != XGE_OK ) {
		xgeXuiUnit(&tXui);
		return 735;
	}
	xgeXuiToolbarSetItems(&tToolbar, arrText, arrTypes, 4);
	xgeXuiToolbarSetSelect(&tToolbar, __testXuiToolbarSelect, (void*)0x3456);
	xgeXuiToolbarSetItemSize(&tToolbar, 50.0f, 24.0f, 8.0f);
	xgeXuiToolbarSetColors(&tToolbar, XGE_COLOR_RGBA(1, 2, 3, 255), XGE_COLOR_RGBA(4, 5, 6, 255), XGE_COLOR_RGBA(7, 8, 9, 255), XGE_COLOR_RGBA(10, 11, 12, 255), XGE_COLOR_RGBA(13, 14, 15, 255), XGE_COLOR_RGBA(16, 17, 18, 255), XGE_COLOR_RGBA(19, 20, 21, 255), XGE_COLOR_RGBA(22, 23, 24, 255), XGE_COLOR_RGBA(25, 26, 27, 255), XGE_COLOR_RGBA(28, 29, 30, 255));
	if ( pWidget->procEvent != xgeXuiToolbarEventProc || pWidget->procPaint != xgeXuiToolbarPaintProc || xgeXuiWidgetIsFocusable(pWidget) == 0 ) {
		xgeXuiUnit(&tXui);
		return 736;
	}
	if ( xgeXuiPaint(&tXui) != 1 || tToolbar.arrItems[2].tRect.fW != 8.0f ) {
		xgeXuiUnit(&tXui);
		return 737;
	}
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_MOUSE_MOVE;
	tEvent.fX = 70.0f;
	tEvent.fY = 20.0f;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONTINUE || tToolbar.iHover != 1 || ((xgeXuiToolbarGetState(&tToolbar) & XGE_XUI_STATE_HOVER) == 0) ) {
		xgeXuiUnit(&tXui);
		return 738;
	}
	tEvent.iType = XGE_EVENT_MOUSE_DOWN;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || tXui.pCapture != pWidget || tXui.pFocus != pWidget || tToolbar.iActive != 1 ) {
		xgeXuiUnit(&tXui);
		return 739;
	}
	g_iXuiToolbarChanges = 0;
	g_iXuiToolbarLastSelected = -1;
	tEvent.iType = XGE_EVENT_MOUSE_UP;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || xgeXuiToolbarGetItemChecked(&tToolbar, 1) != 1 || g_iXuiToolbarChanges != 1 || g_iXuiToolbarLastSelected != 1 || tToolbar.iSelectCount != 1 || tXui.pCapture != NULL ) {
		xgeXuiUnit(&tXui);
		return 740;
	}
	xgeXuiToolbarSetItemEnabled(&tToolbar, 3, 0);
	tEvent.iType = XGE_EVENT_MOUSE_DOWN;
	tEvent.fX = 125.0f;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONTINUE || g_iXuiToolbarChanges != 1 ) {
		xgeXuiUnit(&tXui);
		return 741;
	}
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_KEY_DOWN;
	tEvent.iParam1 = XGE_KEY_SPACE;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || xgeXuiToolbarGetItemChecked(&tToolbar, 1) != 0 || g_iXuiToolbarChanges != 2 || g_iXuiToolbarLastSelected != 1 ) {
		xgeXuiUnit(&tXui);
		return 742;
	}
	xgeXuiToolbarSetOrientation(&tToolbar, XGE_XUI_SEPARATOR_VERTICAL);
	if ( tToolbar.iOrientation != XGE_XUI_SEPARATOR_VERTICAL ) {
		xgeXuiUnit(&tXui);
		return 743;
	}
	xgeXuiToolbarUnit(&tToolbar);
	if ( pWidget->procEvent != NULL || pWidget->procPaint != NULL || pWidget->pUser != NULL ) {
		xgeXuiUnit(&tXui);
		return 744;
	}
	xgeXuiUnit(&tXui);
	return 0;
}

static int g_iXuiStatusBarChanges;
static int g_iXuiStatusBarLastSelected;

static void __testXuiStatusBarSelect(xge_xui_widget pWidget, int iIndex, void* pUser)
{
	(void)pWidget;
	if ( pUser == (void*)0x5678 ) {
		g_iXuiStatusBarChanges++;
		g_iXuiStatusBarLastSelected = iIndex;
	}
}

static int __testXuiStatusBar(void)
{
	xge_xui_context_t tXui;
	xge_xui_status_bar_t tStatusBar;
	xge_xui_widget pRoot;
	xge_xui_widget pWidget;
	xge_event_t tEvent;
	int iLeft;
	int iProgress;
	int iCenter;
	int iRight;

	memset(&tXui, 0, sizeof(tXui));
	memset(&tStatusBar, 0, sizeof(tStatusBar));
	if ( xgeXuiInit(&tXui) != XGE_OK ) {
		return 745;
	}
	pRoot = xgeXuiRoot(&tXui);
	pWidget = xgeXuiWidgetCreate();
	if ( (pRoot == NULL) || (pWidget == NULL) ) {
		xgeXuiWidgetFree(pWidget);
		xgeXuiUnit(&tXui);
		return 746;
	}
	xgeXuiWidgetSetRect(pWidget, (xge_rect_t){ 0.0f, 0.0f, 360.0f, 24.0f });
	xgeXuiWidgetSetPaddingPx(pWidget, 0.0f, 0.0f, 0.0f, 0.0f);
	xgeXuiWidgetAdd(pRoot, pWidget);
	xgeXuiUpdate(&tXui, 0.0f);
	if ( xgeXuiStatusBarInit(&tStatusBar, &tXui, pWidget) != XGE_OK ) {
		xgeXuiUnit(&tXui);
		return 747;
	}
	xgeXuiStatusBarSetSelect(&tStatusBar, __testXuiStatusBarSelect, (void*)0x5678);
	xgeXuiStatusBarSetMetrics(&tStatusBar, 24.0f, 4.0f, 6.0f);
	xgeXuiStatusBarSetColors(&tStatusBar, XGE_COLOR_RGBA(1, 2, 3, 255), XGE_COLOR_RGBA(4, 5, 6, 255), XGE_COLOR_RGBA(7, 8, 9, 255), XGE_COLOR_RGBA(10, 11, 12, 255), XGE_COLOR_RGBA(13, 14, 15, 255), XGE_COLOR_RGBA(16, 17, 18, 255), XGE_COLOR_RGBA(19, 20, 21, 255), XGE_COLOR_RGBA(22, 23, 24, 255), XGE_COLOR_RGBA(25, 26, 27, 255));
	iLeft = xgeXuiStatusBarAddText(&tStatusBar, XGE_XUI_STATUS_BAR_SECTION_LEFT, "Ready", 70.0f, 1);
	iProgress = xgeXuiStatusBarAddProgress(&tStatusBar, XGE_XUI_STATUS_BAR_SECTION_LEFT, 0.0f, 100.0f, 40.0f, 80.0f);
	iCenter = xgeXuiStatusBarAddText(&tStatusBar, XGE_XUI_STATUS_BAR_SECTION_CENTER, "Line 12", 76.0f, 0);
	iRight = xgeXuiStatusBarAddText(&tStatusBar, XGE_XUI_STATUS_BAR_SECTION_RIGHT, "UTF-8", 64.0f, 1);
	if ( pWidget->procEvent != xgeXuiStatusBarEventProc || pWidget->procPaint != xgeXuiStatusBarPaintProc || xgeXuiWidgetIsFocusable(pWidget) == 0 || iLeft != 0 || iProgress != 1 || iCenter != 2 || iRight != 3 ) {
		xgeXuiUnit(&tXui);
		return 748;
	}
	if ( xgeXuiPaint(&tXui) != 1 || tStatusBar.arrItems[iLeft].tRect.fX != 0.0f || tStatusBar.arrItems[iProgress].tRect.fX <= tStatusBar.arrItems[iLeft].tRect.fX || tStatusBar.arrItems[iRight].tRect.fX <= tStatusBar.arrItems[iCenter].tRect.fX ) {
		xgeXuiUnit(&tXui);
		return 749;
	}
	xgeXuiStatusBarSetProgress(&tStatusBar, iProgress, 140.0f);
	if ( tStatusBar.arrItems[iProgress].fValue != 100.0f ) {
		xgeXuiUnit(&tXui);
		return 750;
	}
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_MOUSE_MOVE;
	tEvent.fX = 20.0f;
	tEvent.fY = 12.0f;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONTINUE || tStatusBar.iHover != iLeft || ((xgeXuiStatusBarGetState(&tStatusBar) & XGE_XUI_STATE_HOVER) == 0) ) {
		xgeXuiUnit(&tXui);
		return 751;
	}
	tEvent.iType = XGE_EVENT_MOUSE_DOWN;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || tXui.pCapture != pWidget || tStatusBar.iActive != iLeft ) {
		xgeXuiUnit(&tXui);
		return 752;
	}
	g_iXuiStatusBarChanges = 0;
	g_iXuiStatusBarLastSelected = -1;
	tEvent.iType = XGE_EVENT_MOUSE_UP;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || g_iXuiStatusBarChanges != 1 || g_iXuiStatusBarLastSelected != iLeft || tStatusBar.iSelectCount != 1 || tXui.pCapture != NULL ) {
		xgeXuiUnit(&tXui);
		return 753;
	}
	xgeXuiStatusBarSetItemEnabled(&tStatusBar, iRight, 0);
	tEvent.iType = XGE_EVENT_MOUSE_DOWN;
	tEvent.fX = tStatusBar.arrItems[iRight].tRect.fX + 2.0f;
	tEvent.fY = 12.0f;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONTINUE || g_iXuiStatusBarChanges != 1 ) {
		xgeXuiUnit(&tXui);
		return 754;
	}
	xgeXuiStatusBarClear(&tStatusBar);
	if ( tStatusBar.iItemCount != 0 || xgeXuiStatusBarAddSpacer(&tStatusBar, XGE_XUI_STATUS_BAR_SECTION_LEFT, 10.0f) != 0 ) {
		xgeXuiUnit(&tXui);
		return 755;
	}
	xgeXuiStatusBarUnit(&tStatusBar);
	if ( pWidget->procEvent != NULL || pWidget->procPaint != NULL || pWidget->pUser != NULL ) {
		xgeXuiUnit(&tXui);
		return 756;
	}
	xgeXuiUnit(&tXui);
	return 0;
}

static int g_iXuiTreeSelected;
static int g_iXuiTreeSelectCount;

typedef struct __test_xui_tree_adapter_node_t {
	int iId;
	int iParent;
	const char* sText;
	int bExpanded;
	int bIconReserved;
	int bCheckReserved;
} __test_xui_tree_adapter_node_t;

typedef struct __test_xui_tree_adapter_t {
	const __test_xui_tree_adapter_node_t* arrNode;
	int iCount;
	int iBindCount;
} __test_xui_tree_adapter_t;

static void __testXuiTreeViewSelect(xge_xui_widget pWidget, int iIndex, void* pUser)
{
	(void)pWidget;
	if ( pUser == (void*)0x6789 ) {
		g_iXuiTreeSelected = iIndex;
		g_iXuiTreeSelectCount++;
	}
}

static int __testXuiTreeViewAdapterCount(xge_xui_widget pWidget, void* pUser)
{
	__test_xui_tree_adapter_t* pAdapter;

	(void)pWidget;
	pAdapter = (__test_xui_tree_adapter_t*)pUser;
	return (pAdapter != NULL) ? pAdapter->iCount : 0;
}

static int __testXuiTreeViewAdapterNode(xge_xui_widget pWidget, int iIndex, xge_xui_tree_view_node_t* pNode, void* pUser)
{
	__test_xui_tree_adapter_t* pAdapter;
	const __test_xui_tree_adapter_node_t* pSource;

	(void)pWidget;
	pAdapter = (__test_xui_tree_adapter_t*)pUser;
	if ( (pAdapter == NULL) || (pNode == NULL) || (iIndex < 0) || (iIndex >= pAdapter->iCount) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pSource = &pAdapter->arrNode[iIndex];
	pNode->iId = pSource->iId;
	pNode->iParent = pSource->iParent;
	pNode->sText = pSource->sText;
	pNode->bExpanded = pSource->bExpanded;
	pNode->bIconReserved = pSource->bIconReserved;
	pNode->bCheckReserved = pSource->bCheckReserved;
	pAdapter->iBindCount++;
	return XGE_OK;
}

static int __testXuiTreeView(void)
{
	static const __test_xui_tree_adapter_node_t arrAdapterNode[] = {
		{ 100, -1, "Adapter Root", 1, 1, 0 },
		{ 110, 100, "Adapter Child", 1, 0, 1 },
		{ 120, 110, "Adapter Leaf", 0, 1, 1 }
	};
	xge_xui_context_t tXui;
	xge_xui_tree_view_t tTree;
	__test_xui_tree_adapter_t tAdapter;
	xge_xui_widget pRoot;
	xge_xui_widget pWidget;
	xge_event_t tEvent;
	int iLoop;

	memset(&tXui, 0, sizeof(tXui));
	memset(&tTree, 0, sizeof(tTree));
	memset(&tAdapter, 0, sizeof(tAdapter));
	if ( xgeXuiInit(&tXui) != XGE_OK ) {
		return 760;
	}
	pRoot = xgeXuiRoot(&tXui);
	pWidget = xgeXuiWidgetCreate();
	if ( (pRoot == NULL) || (pWidget == NULL) ) {
		xgeXuiWidgetFree(pWidget);
		xgeXuiUnit(&tXui);
		return 761;
	}
	xgeXuiWidgetSetRect(pWidget, (xge_rect_t){ 0.0f, 0.0f, 200.0f, 82.0f });
	xgeXuiWidgetSetPaddingPx(pWidget, 0.0f, 0.0f, 0.0f, 0.0f);
	xgeXuiWidgetAdd(pRoot, pWidget);
	xgeXuiUpdate(&tXui, 0.0f);
	if ( xgeXuiTreeViewInit(&tTree, &tXui, pWidget) != XGE_OK ) {
		xgeXuiUnit(&tXui);
		return 762;
	}
	xgeXuiTreeViewSetSelect(&tTree, __testXuiTreeViewSelect, (void*)0x6789);
	xgeXuiTreeViewSetMetrics(&tTree, 20.0f, 14.0f);
	xgeXuiTreeViewSetColors(&tTree, XGE_COLOR_RGBA(1, 2, 3, 255), XGE_COLOR_RGBA(4, 5, 6, 255), XGE_COLOR_RGBA(7, 8, 9, 255), XGE_COLOR_RGBA(10, 11, 12, 255), XGE_COLOR_RGBA(13, 14, 15, 255), XGE_COLOR_RGBA(16, 17, 18, 255));
	if ( pWidget->procEvent != xgeXuiTreeViewEventProc || pWidget->procPaint != xgeXuiTreeViewPaintProc || xgeXuiWidgetIsFocusable(pWidget) == 0 || (xgeXuiWidgetGetFlags(pWidget) & XGE_XUI_WIDGET_CLIP) == 0 ) {
		xgeXuiUnit(&tXui);
		return 763;
	}
	if ( xgeXuiTreeViewAddNode(&tTree, 10, -1, "Root") != 0 || xgeXuiTreeViewAddNode(&tTree, 20, 10, "Child A") != 1 || xgeXuiTreeViewAddNode(&tTree, 30, 10, "Child B") != 2 || xgeXuiTreeViewAddNode(&tTree, 40, 20, "Leaf") != 3 || xgeXuiTreeViewAddNode(&tTree, 50, -1, "Other") != 4 ) {
		xgeXuiUnit(&tXui);
		return 764;
	}
	if ( xgeXuiTreeViewGetVisibleCount(&tTree) != 2 || xgeXuiTreeViewGetVisibleNodeId(&tTree, 0) != 10 || xgeXuiTreeViewGetVisibleNodeId(&tTree, 1) != 50 ) {
		xgeXuiUnit(&tXui);
		return 765;
	}
	xgeXuiTreeViewSetNodeExpanded(&tTree, 10, 1);
	if ( xgeXuiTreeViewGetNodeExpanded(&tTree, 10) == 0 || xgeXuiTreeViewGetVisibleCount(&tTree) != 4 || xgeXuiTreeViewGetVisibleNodeId(&tTree, 1) != 20 ) {
		xgeXuiUnit(&tXui);
		return 766;
	}
	if ( xgeXuiPaint(&tXui) != 1 || tTree.arrNodes[0].tRect.fH != 20.0f ) {
		xgeXuiUnit(&tXui);
		return 767;
	}
	g_iXuiTreeSelected = -1;
	g_iXuiTreeSelectCount = 0;
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_MOUSE_DOWN;
	tEvent.fX = 60.0f;
	tEvent.fY = 30.0f;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || tXui.pCapture != pWidget ) {
		xgeXuiUnit(&tXui);
		return 768;
	}
	tEvent.iType = XGE_EVENT_MOUSE_UP;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || xgeXuiTreeViewGetSelected(&tTree) != 20 || g_iXuiTreeSelected != 20 || g_iXuiTreeSelectCount != 1 || tTree.iSelectCount != 1 ) {
		xgeXuiUnit(&tXui);
		return 769;
	}
	tEvent.iType = XGE_EVENT_KEY_DOWN;
	tEvent.iParam1 = XGE_KEY_RIGHT;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || xgeXuiTreeViewGetNodeExpanded(&tTree, 20) == 0 || xgeXuiTreeViewGetVisibleCount(&tTree) != 5 ) {
		xgeXuiUnit(&tXui);
		return 770;
	}
	tEvent.iParam1 = XGE_KEY_DOWN;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || xgeXuiTreeViewGetSelected(&tTree) != 40 ) {
		xgeXuiUnit(&tXui);
		return 771;
	}
	tEvent.iParam1 = XGE_KEY_LEFT;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || xgeXuiTreeViewGetSelected(&tTree) != 20 ) {
		xgeXuiUnit(&tXui);
		return 772;
	}
	tEvent.iType = XGE_EVENT_MOUSE_DOWN;
	tEvent.iParam1 = 0;
	tEvent.fX = 8.0f;
	tEvent.fY = 10.0f;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED ) {
		xgeXuiUnit(&tXui);
		return 773;
	}
	tEvent.iType = XGE_EVENT_MOUSE_UP;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || xgeXuiTreeViewGetNodeExpanded(&tTree, 10) != 0 || xgeXuiTreeViewGetVisibleCount(&tTree) != 2 || xgeXuiTreeViewGetSelected(&tTree) != -1 ) {
		xgeXuiUnit(&tXui);
		return 774;
	}
	xgeXuiTreeViewClear(&tTree);
	if ( xgeXuiTreeViewGetVisibleCount(&tTree) != 0 || xgeXuiTreeViewGetSelected(&tTree) != -1 ) {
		xgeXuiUnit(&tXui);
		return 775;
	}
	tAdapter.arrNode = arrAdapterNode;
	tAdapter.iCount = (int)(sizeof(arrAdapterNode) / sizeof(arrAdapterNode[0]));
	xgeXuiTreeViewSetAdapter(&tTree, __testXuiTreeViewAdapterCount, __testXuiTreeViewAdapterNode, &tAdapter);
	if ( tAdapter.iBindCount != 3 || tTree.iNodeCount != 3 || xgeXuiTreeViewGetVisibleCount(&tTree) != 3 || xgeXuiTreeViewGetVisibleNodeId(&tTree, 2) != 120 || tTree.arrNodes[1].bCheckReserved != 1 || tTree.arrNodes[1].bIconReserved != 0 ) {
		xgeXuiUnit(&tXui);
		return 777;
	}
	xgeXuiTreeViewSetSelected(&tTree, 120);
	tAdapter.iBindCount = 0;
	if ( xgeXuiTreeViewRefreshAdapter(&tTree) != XGE_OK || tAdapter.iBindCount != 3 || xgeXuiTreeViewGetSelected(&tTree) != 120 ) {
		xgeXuiUnit(&tXui);
		return 778;
	}
	xgeXuiTreeViewClear(&tTree);
	for ( iLoop = 0; iLoop < 20; iLoop++ ) {
		if ( xgeXuiTreeViewAddNode(&tTree, 1000 + iLoop, -1, "Row") < 0 ) {
			xgeXuiUnit(&tXui);
			return 907;
		}
	}
	xgeXuiTreeViewSetScroll(&tTree, 60.0f);
	if ( xgeXuiPaint(&tXui) != 1 || xgeXuiTreeViewGetVisibleCount(&tTree) != 20 || xgeXuiTreeViewGetFirstVisible(&tTree) != 3 || xgeXuiTreeViewGetPaintVisibleCount(&tTree) != 6 ) {
		xgeXuiUnit(&tXui);
		return 908;
	}
	xgeXuiTreeViewUnit(&tTree);
	if ( pWidget->procEvent != NULL || pWidget->procPaint != NULL || pWidget->pUser != NULL ) {
		xgeXuiUnit(&tXui);
		return 776;
	}
	xgeXuiUnit(&tXui);
	return 0;
}

typedef struct __test_xui_table_adapter_t {
	int iRowCount;
	int iCellCalls;
	int iSortColumn;
	int bSortDescending;
	int iSortCount;
	int iSelected;
	int iSelectCount;
} __test_xui_table_adapter_t;

static int __testXuiTableViewCount(xge_xui_widget pWidget, void* pUser)
{
	__test_xui_table_adapter_t* pAdapter;

	(void)pWidget;
	pAdapter = (__test_xui_table_adapter_t*)pUser;
	return (pAdapter != NULL) ? pAdapter->iRowCount : 0;
}

static int __testXuiTableViewCell(xge_xui_widget pWidget, int iRow, int iColumn, char* sBuffer, int iBufferSize, void* pUser)
{
	__test_xui_table_adapter_t* pAdapter;

	(void)pWidget;
	pAdapter = (__test_xui_table_adapter_t*)pUser;
	if ( (sBuffer == NULL) || (iBufferSize <= 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( pAdapter != NULL ) {
		pAdapter->iCellCalls++;
	}
	snprintf(sBuffer, (size_t)iBufferSize, "R%dC%d", iRow, iColumn);
	return XGE_OK;
}

static void __testXuiTableViewSort(xge_xui_widget pWidget, int iColumn, int bDescending, void* pUser)
{
	__test_xui_table_adapter_t* pAdapter;

	(void)pWidget;
	pAdapter = (__test_xui_table_adapter_t*)pUser;
	if ( pAdapter != NULL ) {
		pAdapter->iSortColumn = iColumn;
		pAdapter->bSortDescending = bDescending;
		pAdapter->iSortCount++;
	}
}

static void __testXuiTableViewSelect(xge_xui_widget pWidget, int iIndex, void* pUser)
{
	__test_xui_table_adapter_t* pAdapter;

	(void)pWidget;
	pAdapter = (__test_xui_table_adapter_t*)pUser;
	if ( pAdapter != NULL ) {
		pAdapter->iSelected = iIndex;
		pAdapter->iSelectCount++;
	}
}

static int __testXuiTableView(void)
{
	xge_xui_context_t tXui;
	xge_xui_table_view_t tTable;
	xge_xui_table_view_column_t arrColumns[3];
	__test_xui_table_adapter_t tAdapter;
	xge_xui_widget pRoot;
	xge_xui_widget pWidget;
	xge_event_t tEvent;

	memset(&tXui, 0, sizeof(tXui));
	memset(&tTable, 0, sizeof(tTable));
	memset(&tAdapter, 0, sizeof(tAdapter));
	memset(arrColumns, 0, sizeof(arrColumns));
	tAdapter.iRowCount = 20;
	tAdapter.iSelected = -1;
	tAdapter.iSortColumn = -1;
	if ( xgeXuiInit(&tXui) != XGE_OK ) {
		return 909;
	}
	pRoot = xgeXuiRoot(&tXui);
	pWidget = xgeXuiWidgetCreate();
	if ( (pRoot == NULL) || (pWidget == NULL) ) {
		xgeXuiWidgetFree(pWidget);
		xgeXuiUnit(&tXui);
		return 910;
	}
	xgeXuiWidgetSetRect(pWidget, (xge_rect_t){ 0.0f, 0.0f, 240.0f, 104.0f });
	xgeXuiWidgetSetPaddingPx(pWidget, 0.0f, 0.0f, 0.0f, 0.0f);
	xgeXuiWidgetAdd(pRoot, pWidget);
	xgeXuiUpdate(&tXui, 0.0f);
	if ( xgeXuiTableViewInit(&tTable, &tXui, pWidget) != XGE_OK ) {
		xgeXuiUnit(&tXui);
		return 911;
	}
	arrColumns[0].iId = 10;
	arrColumns[0].sTitle = "Name";
	arrColumns[0].fWidth = 80.0f;
	arrColumns[0].fMinWidth = 40.0f;
	arrColumns[0].iAlign = XGE_TEXT_ALIGN_LEFT;
	arrColumns[1].iId = 20;
	arrColumns[1].sTitle = "Value";
	arrColumns[1].fWidth = 70.0f;
	arrColumns[1].fMinWidth = 50.0f;
	arrColumns[1].iAlign = XGE_TEXT_ALIGN_RIGHT;
	arrColumns[2].iId = 30;
	arrColumns[2].sTitle = "State";
	arrColumns[2].fWidth = 60.0f;
	arrColumns[2].fMinWidth = 45.0f;
	arrColumns[2].iAlign = XGE_TEXT_ALIGN_CENTER;
	xgeXuiTableViewSetColumns(&tTable, arrColumns, 3);
	xgeXuiTableViewSetAdapter(&tTable, __testXuiTableViewCount, __testXuiTableViewCell, &tAdapter);
	xgeXuiTableViewSetSort(&tTable, __testXuiTableViewSort, &tAdapter);
	xgeXuiTableViewSetSelect(&tTable, __testXuiTableViewSelect, &tAdapter);
	xgeXuiTableViewSetMetrics(&tTable, 24.0f, 20.0f);
	xgeXuiTableViewSetColors(&tTable, XGE_COLOR_RGBA(1, 2, 3, 255), XGE_COLOR_RGBA(4, 5, 6, 255), XGE_COLOR_RGBA(7, 8, 9, 255), XGE_COLOR_RGBA(10, 11, 12, 255), XGE_COLOR_RGBA(13, 14, 15, 255), XGE_COLOR_RGBA(16, 17, 18, 255));
	if ( pWidget->procEvent != xgeXuiTableViewEventProc || pWidget->procPaint != xgeXuiTableViewPaintProc || xgeXuiWidgetIsFocusable(pWidget) == 0 || (xgeXuiWidgetGetFlags(pWidget) & XGE_XUI_WIDGET_CLIP) == 0 ) {
		xgeXuiUnit(&tXui);
		return 912;
	}
	xgeXuiTableViewSetScroll(&tTable, 40.0f);
	tAdapter.iCellCalls = 0;
	if ( xgeXuiPaint(&tXui) != 1 || xgeXuiTableViewGetRowCount(&tTable) != 20 || xgeXuiTableViewGetFirstVisible(&tTable) != 2 || xgeXuiTableViewGetPaintVisibleCount(&tTable) != 6 || tAdapter.iCellCalls != 18 || tTable.arrColumns[0].tRect.fH != 24.0f ) {
		xgeXuiUnit(&tXui);
		return 913;
	}
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_MOUSE_DOWN;
	tEvent.fX = 90.0f;
	tEvent.fY = 50.0f;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || tXui.pCapture != pWidget ) {
		xgeXuiUnit(&tXui);
		return 914;
	}
	tEvent.iType = XGE_EVENT_MOUSE_UP;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || xgeXuiTableViewGetSelected(&tTable) != 3 || tAdapter.iSelected != 3 || tAdapter.iSelectCount != 1 ) {
		xgeXuiUnit(&tXui);
		return 915;
	}
	tEvent.iType = XGE_EVENT_MOUSE_DOWN;
	tEvent.fX = 20.0f;
	tEvent.fY = 8.0f;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || tAdapter.iSortColumn != 0 || tAdapter.bSortDescending != 0 || tAdapter.iSortCount != 1 ) {
		xgeXuiUnit(&tXui);
		return 916;
	}
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || tAdapter.iSortColumn != 0 || tAdapter.bSortDescending != 1 || tAdapter.iSortCount != 2 ) {
		xgeXuiUnit(&tXui);
		return 917;
	}
	tEvent.iType = XGE_EVENT_MOUSE_DOWN;
	tEvent.fX = 80.0f;
	tEvent.fY = 8.0f;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || tTable.iResizeColumn != 0 ) {
		xgeXuiUnit(&tXui);
		return 918;
	}
	tEvent.iType = XGE_EVENT_MOUSE_MOVE;
	tEvent.fX = 110.0f;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || tTable.arrColumns[0].fWidth != 110.0f ) {
		xgeXuiUnit(&tXui);
		return 919;
	}
	tEvent.iType = XGE_EVENT_MOUSE_UP;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || tTable.iResizeColumn != -1 ) {
		xgeXuiUnit(&tXui);
		return 920;
	}
	xgeXuiTableViewUnit(&tTable);
	if ( pWidget->procEvent != NULL || pWidget->procPaint != NULL || pWidget->pUser != NULL ) {
		xgeXuiUnit(&tXui);
		return 921;
	}
	xgeXuiUnit(&tXui);
	return 0;
}

static int g_iXuiPropertyGridSelected;
static int g_iXuiPropertyGridSelectCount;

static void __testXuiPropertyGridSelect(xge_xui_widget pWidget, int iIndex, void* pUser)
{
	(void)pWidget;
	if ( pUser == (void*)0x7890 ) {
		g_iXuiPropertyGridSelected = iIndex;
		g_iXuiPropertyGridSelectCount++;
	}
}

static int __testXuiPropertyGrid(void)
{
	xge_xui_context_t tXui;
	xge_xui_property_grid_t tGrid;
	xge_xui_widget pRoot;
	xge_xui_widget pWidget;
	xge_event_t tEvent;
	static const char* arrModes[] = { "Auto", "Manual", "Script" };
	int iGeneral;
	int iAdvanced;
	int iName;
	int iCount;
	int iEnabled;
	int iMode;
	int iColor;

	memset(&tXui, 0, sizeof(tXui));
	memset(&tGrid, 0, sizeof(tGrid));
	if ( xgeXuiInit(&tXui) != XGE_OK ) {
		return 922;
	}
	pRoot = xgeXuiRoot(&tXui);
	pWidget = xgeXuiWidgetCreate();
	if ( (pRoot == NULL) || (pWidget == NULL) ) {
		xgeXuiWidgetFree(pWidget);
		xgeXuiUnit(&tXui);
		return 923;
	}
	xgeXuiWidgetSetRect(pWidget, (xge_rect_t){ 0.0f, 0.0f, 260.0f, 160.0f });
	xgeXuiWidgetSetPaddingPx(pWidget, 0.0f, 0.0f, 0.0f, 0.0f);
	xgeXuiWidgetAdd(pRoot, pWidget);
	xgeXuiUpdate(&tXui, 0.0f);
	if ( xgeXuiPropertyGridInit(&tGrid, &tXui, pWidget) != XGE_OK ) {
		xgeXuiUnit(&tXui);
		return 924;
	}
	xgeXuiPropertyGridSetSelect(&tGrid, __testXuiPropertyGridSelect, (void*)0x7890);
	xgeXuiPropertyGridSetMetrics(&tGrid, 20.0f, 110.0f);
	xgeXuiPropertyGridSetColors(&tGrid, XGE_COLOR_RGBA(1, 2, 3, 255), XGE_COLOR_RGBA(4, 5, 6, 255), XGE_COLOR_RGBA(7, 8, 9, 255), XGE_COLOR_RGBA(10, 11, 12, 255), XGE_COLOR_RGBA(13, 14, 15, 255), XGE_COLOR_RGBA(16, 17, 18, 255));
	iGeneral = xgeXuiPropertyGridAddCategory(&tGrid, "General", 1);
	iName = xgeXuiPropertyGridAddProperty(&tGrid, iGeneral, "Name", "Player", XGE_XUI_PROPERTY_GRID_EDITOR_TEXT);
	iCount = xgeXuiPropertyGridAddProperty(&tGrid, iGeneral, "Count", "12", XGE_XUI_PROPERTY_GRID_EDITOR_NUMBER);
	iEnabled = xgeXuiPropertyGridAddProperty(&tGrid, iGeneral, "Enabled", "true", XGE_XUI_PROPERTY_GRID_EDITOR_BOOL);
	iAdvanced = xgeXuiPropertyGridAddCategory(&tGrid, "Advanced", 0);
	iMode = xgeXuiPropertyGridAddProperty(&tGrid, iAdvanced, "Mode", "Auto", XGE_XUI_PROPERTY_GRID_EDITOR_ENUM);
	xgeXuiPropertyGridSetEnumItems(&tGrid, iMode, arrModes, (int)(sizeof(arrModes) / sizeof(arrModes[0])));
	iColor = xgeXuiPropertyGridAddProperty(&tGrid, iAdvanced, "Tint", "#168AC2", XGE_XUI_PROPERTY_GRID_EDITOR_COLOR);
	if ( pWidget->procEvent != xgeXuiPropertyGridEventProc || pWidget->procPaint != xgeXuiPropertyGridPaintProc || xgeXuiWidgetIsFocusable(pWidget) == 0 || (xgeXuiWidgetGetFlags(pWidget) & XGE_XUI_WIDGET_CLIP) == 0 ) {
		xgeXuiUnit(&tXui);
		return 925;
	}
	if ( iGeneral != 0 || iName != 1 || iCount != 2 || iEnabled != 3 || iAdvanced != 4 || iMode != 5 || iColor != 6 || xgeXuiPropertyGridGetVisibleCount(&tGrid) != 5 ) {
		xgeXuiUnit(&tXui);
		return 926;
	}
	xgeXuiPropertyGridSetPropertyFlags(&tGrid, iName, 1, 0, 0);
	xgeXuiPropertyGridSetPropertyFlags(&tGrid, iCount, 0, 1, 0);
	xgeXuiPropertyGridSetPropertyFlags(&tGrid, iEnabled, 0, 0, 1);
	if ( tGrid.arrItems[iName].bReadonly != 1 || tGrid.arrItems[iCount].bDefaultChanged != 1 || tGrid.arrItems[iEnabled].bError != 1 || tGrid.arrItems[iMode].iEditor != XGE_XUI_PROPERTY_GRID_EDITOR_ENUM || tGrid.arrItems[iMode].iEnumItemCount != 3 || tGrid.arrItems[iColor].iEditor != XGE_XUI_PROPERTY_GRID_EDITOR_COLOR ) {
		xgeXuiUnit(&tXui);
		return 927;
	}
	if ( xgeXuiPaint(&tXui) != 1 || tGrid.arrItems[iName].tRect.fH != 20.0f ) {
		xgeXuiUnit(&tXui);
		return 928;
	}
	g_iXuiPropertyGridSelected = -1;
	g_iXuiPropertyGridSelectCount = 0;
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_MOUSE_DOWN;
	tEvent.fX = 120.0f;
	tEvent.fY = 30.0f;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || xgeXuiPropertyGridGetSelected(&tGrid) != iName || g_iXuiPropertyGridSelected != iName || g_iXuiPropertyGridSelectCount != 1 ) {
		xgeXuiUnit(&tXui);
		return 929;
	}
	tEvent.fY = 84.0f;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || tGrid.arrItems[iAdvanced].bExpanded != 1 || xgeXuiPropertyGridGetVisibleCount(&tGrid) != 7 ) {
		xgeXuiUnit(&tXui);
		return 930;
	}
	xgeXuiPropertyGridBeginEdit(&tGrid, iMode);
	if ( tGrid.iEnumEditing != iMode || tGrid.iEditing != -1 || tGrid.pEnumPopup == NULL || xgeXuiPopupIsOpen(tGrid.pEnumPopup) == 0 || tGrid.pEnumList == NULL || xgeXuiListViewGetSelected(tGrid.pEnumList) != 0 ) {
		xgeXuiUnit(&tXui);
		return 934;
	}
	tEvent.fX = tGrid.pEnumListWidget->tRect.fX + 8.0f;
	tEvent.fY = tGrid.pEnumListWidget->tRect.fY + tGrid.pEnumList->fItemHeight * 2.0f + 5.0f;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || strcmp(xgeXuiPropertyGridGetValue(&tGrid, iMode), "Script") != 0 || tGrid.iEnumEditing != -1 || xgeXuiPopupIsOpen(tGrid.pEnumPopup) != 0 ) {
		xgeXuiUnit(&tXui);
		return 935;
	}
	tEvent.fY = 84.0f;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || tGrid.arrItems[iAdvanced].bExpanded != 0 || xgeXuiPropertyGridGetVisibleCount(&tGrid) != 5 ) {
		xgeXuiUnit(&tXui);
		return 931;
	}
	xgeXuiPropertyGridClear(&tGrid);
	if ( xgeXuiPropertyGridGetVisibleCount(&tGrid) != 0 || xgeXuiPropertyGridGetSelected(&tGrid) != -1 ) {
		xgeXuiUnit(&tXui);
		return 932;
	}
	xgeXuiPropertyGridUnit(&tGrid);
	if ( pWidget->procEvent != NULL || pWidget->procPaint != NULL || pWidget->pUser != NULL ) {
		xgeXuiUnit(&tXui);
		return 933;
	}
	xgeXuiUnit(&tXui);
	return 0;
}

static int g_iXuiBreadcrumbSelected;
static int g_iXuiBreadcrumbSelectCount;

static void __testXuiBreadcrumbSelect(xge_xui_widget pWidget, int iIndex, void* pUser)
{
	(void)pWidget;
	if ( pUser == (void*)0x2468 ) {
		g_iXuiBreadcrumbSelected = iIndex;
		g_iXuiBreadcrumbSelectCount++;
	}
}

static int __testXuiBreadcrumb(void)
{
	xge_xui_context_t tXui;
	xge_xui_breadcrumb_t tBreadcrumb;
	xge_xui_widget pRoot;
	xge_xui_widget pWidget;
	xge_event_t tEvent;
	xge_rect_t tLast;
	int iLast;

	memset(&tXui, 0, sizeof(tXui));
	memset(&tBreadcrumb, 0, sizeof(tBreadcrumb));
	g_iXuiBreadcrumbSelected = -1;
	g_iXuiBreadcrumbSelectCount = 0;
	if ( xgeXuiInit(&tXui) != XGE_OK ) {
		return 934;
	}
	pRoot = xgeXuiRoot(&tXui);
	pWidget = xgeXuiWidgetCreate();
	if ( pWidget == NULL ) {
		xgeXuiUnit(&tXui);
		return 935;
	}
	xgeXuiWidgetSetRect(pWidget, (xge_rect_t){ 0.0f, 0.0f, 160.0f, 28.0f });
	xgeXuiWidgetAdd(pRoot, pWidget);
	if ( xgeXuiBreadcrumbInit(&tBreadcrumb, &tXui, pWidget) != XGE_OK ) {
		xgeXuiUnit(&tXui);
		return 936;
	}
	xgeXuiBreadcrumbSetSelect(&tBreadcrumb, __testXuiBreadcrumbSelect, (void*)0x2468);
	xgeXuiBreadcrumbSetMetrics(&tBreadcrumb, 8.0f, 14.0f);
	xgeXuiBreadcrumbSetColors(&tBreadcrumb, XGE_COLOR_RGBA(1, 2, 3, 255), XGE_COLOR_RGBA(4, 5, 6, 255), XGE_COLOR_RGBA(7, 8, 9, 255), XGE_COLOR_RGBA(10, 11, 12, 255), XGE_COLOR_RGBA(13, 14, 15, 255), XGE_COLOR_RGBA(16, 17, 18, 255));
	xgeXuiBreadcrumbAddSegment(&tBreadcrumb, "Home", 10);
	xgeXuiBreadcrumbAddSegment(&tBreadcrumb, "Project", 20);
	xgeXuiBreadcrumbAddSegment(&tBreadcrumb, "Assets", 30);
	xgeXuiBreadcrumbAddSegment(&tBreadcrumb, "Textures", 40);
	iLast = xgeXuiBreadcrumbAddSegment(&tBreadcrumb, "Sky", 50);
	if ( pWidget->procEvent != xgeXuiBreadcrumbEventProc || pWidget->procPaint != xgeXuiBreadcrumbPaintProc || xgeXuiWidgetIsFocusable(pWidget) == 0 || (xgeXuiWidgetGetFlags(pWidget) & XGE_XUI_WIDGET_CLIP) == 0 ) {
		xgeXuiUnit(&tXui);
		return 937;
	}
	if ( xgeXuiBreadcrumbGetSegmentCount(&tBreadcrumb) != 5 || iLast != 4 || xgeXuiBreadcrumbGetCollapsedCount(&tBreadcrumb) <= 0 || xgeXuiBreadcrumbGetFirstVisible(&tBreadcrumb) <= 0 ) {
		xgeXuiUnit(&tXui);
		return 938;
	}
	if ( xgeXuiPaint(&tXui) != 1 || tXui.iPaintCommandCount != 1 ) {
		xgeXuiUnit(&tXui);
		return 939;
	}
	tLast = tBreadcrumb.arrSegments[iLast].tRect;
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_MOUSE_MOVE;
	tEvent.fX = tLast.fX + 4.0f;
	tEvent.fY = tLast.fY + 8.0f;
	xgeXuiDispatchEvent(&tXui, &tEvent);
	if ( tBreadcrumb.iHover != iLast ) {
		xgeXuiUnit(&tXui);
		return 940;
	}
	tEvent.iType = XGE_EVENT_MOUSE_DOWN;
	tEvent.iParam1 = XGE_MOUSE_LEFT;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || xgeXuiBreadcrumbGetSelected(&tBreadcrumb) != iLast || g_iXuiBreadcrumbSelected != iLast || g_iXuiBreadcrumbSelectCount != 1 ) {
		xgeXuiUnit(&tXui);
		return 941;
	}
	xgeXuiBreadcrumbSetSelected(&tBreadcrumb, 1);
	if ( xgeXuiBreadcrumbGetSelected(&tBreadcrumb) != 1 ) {
		xgeXuiUnit(&tXui);
		return 942;
	}
	xgeXuiBreadcrumbClear(&tBreadcrumb);
	if ( xgeXuiBreadcrumbGetSegmentCount(&tBreadcrumb) != 0 || xgeXuiBreadcrumbGetSelected(&tBreadcrumb) != -1 || xgeXuiBreadcrumbGetCollapsedCount(&tBreadcrumb) != 0 ) {
		xgeXuiUnit(&tXui);
		return 943;
	}
	xgeXuiBreadcrumbUnit(&tBreadcrumb);
	if ( pWidget->procEvent != NULL || pWidget->procPaint != NULL || pWidget->pUser != NULL ) {
		xgeXuiUnit(&tXui);
		return 944;
	}
	xgeXuiUnit(&tXui);
	return 0;
}

static int g_iXuiAccordionSelected;
static int g_iXuiAccordionSelectCount;

static void __testXuiAccordionSelect(xge_xui_widget pWidget, int iIndex, void* pUser)
{
	(void)pWidget;
	if ( pUser == (void*)0x1357 ) {
		g_iXuiAccordionSelected = iIndex;
		g_iXuiAccordionSelectCount++;
	}
}

static int __testXuiAccordion(void)
{
	xge_xui_context_t tXui;
	xge_xui_accordion_t tAccordion;
	xge_xui_widget pRoot;
	xge_xui_widget pWidget;
	xge_event_t tEvent;
	float fHeight;
	int iA;
	int iB;
	int iC;

	memset(&tXui, 0, sizeof(tXui));
	memset(&tAccordion, 0, sizeof(tAccordion));
	g_iXuiAccordionSelected = -1;
	g_iXuiAccordionSelectCount = 0;
	if ( xgeXuiInit(&tXui) != XGE_OK ) {
		return 945;
	}
	pRoot = xgeXuiRoot(&tXui);
	pWidget = xgeXuiWidgetCreate();
	if ( pWidget == NULL ) {
		xgeXuiUnit(&tXui);
		return 946;
	}
	xgeXuiWidgetSetRect(pWidget, (xge_rect_t){ 0.0f, 0.0f, 220.0f, 170.0f });
	xgeXuiWidgetAdd(pRoot, pWidget);
	if ( xgeXuiAccordionInit(&tAccordion, &tXui, pWidget) != XGE_OK ) {
		xgeXuiUnit(&tXui);
		return 947;
	}
	xgeXuiAccordionSetSelect(&tAccordion, __testXuiAccordionSelect, (void*)0x1357);
	xgeXuiAccordionSetMetrics(&tAccordion, 24.0f, 3.0f, 6.0f);
	xgeXuiAccordionSetColors(&tAccordion, XGE_COLOR_RGBA(1, 2, 3, 255), XGE_COLOR_RGBA(4, 5, 6, 255), XGE_COLOR_RGBA(7, 8, 9, 255), XGE_COLOR_RGBA(10, 11, 12, 255), XGE_COLOR_RGBA(13, 14, 15, 255), XGE_COLOR_RGBA(16, 17, 18, 255));
	iA = xgeXuiAccordionAddSection(&tAccordion, "A", "Alpha", 30.0f, 1, 10);
	iB = xgeXuiAccordionAddSection(&tAccordion, "B", "Beta", 30.0f, 0, 20);
	iC = xgeXuiAccordionAddSection(&tAccordion, "C", "Gamma", 30.0f, 0, 30);
	if ( pWidget->procEvent != xgeXuiAccordionEventProc || pWidget->procPaint != xgeXuiAccordionPaintProc || xgeXuiWidgetIsFocusable(pWidget) == 0 || (xgeXuiWidgetGetFlags(pWidget) & XGE_XUI_WIDGET_CLIP) == 0 ) {
		xgeXuiUnit(&tXui);
		return 948;
	}
	if ( xgeXuiAccordionGetSectionCount(&tAccordion) != 3 || iA != 0 || iB != 1 || iC != 2 || xgeXuiAccordionIsExpanded(&tAccordion, iA) == 0 || xgeXuiAccordionIsExpanded(&tAccordion, iB) != 0 ) {
		xgeXuiUnit(&tXui);
		return 949;
	}
	fHeight = xgeXuiAccordionGetContentHeight(&tAccordion);
	if ( fHeight != 111.0f ) {
		xgeXuiUnit(&tXui);
		return 950;
	}
	if ( xgeXuiPaint(&tXui) != 1 || tXui.iPaintCommandCount != 1 ) {
		xgeXuiUnit(&tXui);
		return 951;
	}
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_MOUSE_MOVE;
	tEvent.fX = 12.0f;
	tEvent.fY = tAccordion.arrSections[iB].tHeaderRect.fY + 8.0f;
	xgeXuiDispatchEvent(&tXui, &tEvent);
	if ( tAccordion.iHover != iB ) {
		xgeXuiUnit(&tXui);
		return 952;
	}
	tEvent.iType = XGE_EVENT_MOUSE_DOWN;
	tEvent.iParam1 = XGE_MOUSE_LEFT;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || xgeXuiAccordionIsExpanded(&tAccordion, iB) == 0 || g_iXuiAccordionSelected != iB || g_iXuiAccordionSelectCount != 1 ) {
		xgeXuiUnit(&tXui);
		return 953;
	}
	xgeXuiAccordionSetMode(&tAccordion, XGE_XUI_ACCORDION_MODE_SINGLE);
	xgeXuiAccordionSetExpanded(&tAccordion, iC, 1);
	if ( xgeXuiAccordionIsExpanded(&tAccordion, iA) != 0 || xgeXuiAccordionIsExpanded(&tAccordion, iB) != 0 || xgeXuiAccordionIsExpanded(&tAccordion, iC) == 0 ) {
		xgeXuiUnit(&tXui);
		return 954;
	}
	xgeXuiAccordionClear(&tAccordion);
	if ( xgeXuiAccordionGetSectionCount(&tAccordion) != 0 || xgeXuiAccordionGetContentHeight(&tAccordion) != 0.0f ) {
		xgeXuiUnit(&tXui);
		return 955;
	}
	xgeXuiAccordionUnit(&tAccordion);
	if ( pWidget->procEvent != NULL || pWidget->procPaint != NULL || pWidget->pUser != NULL ) {
		xgeXuiUnit(&tXui);
		return 956;
	}
	xgeXuiUnit(&tXui);
	return 0;
}

static int __testXuiToast(void)
{
	xge_xui_context_t tXui;
	xge_xui_toast_t tToast;
	xge_xui_widget pRoot;
	xge_xui_widget pWidget;
	xge_event_t tEvent;
	int i;

	memset(&tXui, 0, sizeof(tXui));
	memset(&tToast, 0, sizeof(tToast));
	if ( xgeXuiInit(&tXui) != XGE_OK ) {
		return 980;
	}
	pRoot = xgeXuiRoot(&tXui);
	pWidget = xgeXuiWidgetCreate();
	if ( (pRoot == NULL) || (pWidget == NULL) ) {
		xgeXuiWidgetFree(pWidget);
		xgeXuiUnit(&tXui);
		return 981;
	}
	xgeXuiWidgetSetRect(pWidget, (xge_rect_t){ 0.0f, 0.0f, 320.0f, 220.0f });
	xgeXuiWidgetAdd(pRoot, pWidget);
	if ( xgeXuiToastInit(&tToast, &tXui, pWidget, NULL) != XGE_OK ) {
		xgeXuiUnit(&tXui);
		return 982;
	}
	xgeXuiToastSetMetrics(&tToast, 180.0f, 46.0f, 5.0f);
	xgeXuiToastSetPlacement(&tToast, XGE_XUI_TOAST_PLACEMENT_BOTTOM_CENTER);
	xgeXuiToastSetColors(&tToast, XGE_COLOR_RGBA(1, 2, 3, 255), XGE_COLOR_RGBA(4, 5, 6, 255), XGE_COLOR_RGBA(7, 8, 9, 255), XGE_COLOR_RGBA(10, 11, 12, 255), XGE_COLOR_RGBA(13, 14, 15, 255), XGE_COLOR_RGBA(16, 17, 18, 255), XGE_COLOR_RGBA(19, 20, 21, 255), XGE_COLOR_RGBA(22, 23, 24, 255));
	if ( pWidget->procEvent != xgeXuiToastEventProc || pWidget->procUpdate != xgeXuiToastUpdateProc || pWidget->procPaint != xgeXuiToastPaintProc || (xgeXuiWidgetGetFlags(pWidget) & XGE_XUI_WIDGET_CLIP) == 0 ) {
		xgeXuiUnit(&tXui);
		return 983;
	}
	if ( xgeXuiToastShow(&tToast, XGE_XUI_TOAST_TYPE_INFO, "Info", "Queued", 10.0f) != 0 || xgeXuiToastShow(&tToast, XGE_XUI_TOAST_TYPE_SUCCESS, "OK", "Saved", 10.0f) != 1 || xgeXuiToastGetCount(&tToast) != 2 || tToast.iShowCount != 2 ) {
		xgeXuiUnit(&tXui);
		return 984;
	}
	if ( xgeXuiPaint(&tXui) != 1 || tToast.arrItems[0].tRect.fX != 70.0f || tToast.arrItems[0].tRect.fY != 123.0f ) {
		xgeXuiUnit(&tXui);
		return 985;
	}
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_MOUSE_MOVE;
	tEvent.fX = tToast.arrItems[0].tCloseRect.fX + 2.0f;
	tEvent.fY = tToast.arrItems[0].tCloseRect.fY + 2.0f;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || tToast.iHoverClose != 0 ) {
		xgeXuiUnit(&tXui);
		return 986;
	}
	tEvent.iType = XGE_EVENT_MOUSE_DOWN;
	tEvent.iParam1 = XGE_MOUSE_LEFT;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || xgeXuiToastGetCount(&tToast) != 1 || tToast.iCloseCount != 1 ) {
		xgeXuiUnit(&tXui);
		return 987;
	}
	xgeXuiToastClear(&tToast);
	for ( i = 0; i < XGE_XUI_TOAST_CAPACITY + 2; i++ ) {
		xgeXuiToastShow(&tToast, XGE_XUI_TOAST_TYPE_WARNING, "W", "Capacity", 10.0f);
	}
	if ( xgeXuiToastGetCount(&tToast) != XGE_XUI_TOAST_CAPACITY || tToast.iShowCount != XGE_XUI_TOAST_CAPACITY + 4 ) {
		xgeXuiUnit(&tXui);
		return 988;
	}
	xgeXuiToastClear(&tToast);
	xgeXuiToastShow(&tToast, XGE_XUI_TOAST_TYPE_ERROR, "Error", "Expired", 0.25f);
	xgeXuiToastUpdateProc(pWidget, 0.30f, &tToast);
	if ( xgeXuiToastGetCount(&tToast) != 0 || tToast.iExpireCount != 1 ) {
		xgeXuiUnit(&tXui);
		return 989;
	}
	xgeXuiToastUnit(&tToast);
	if ( pWidget->procEvent != NULL || pWidget->procUpdate != NULL || pWidget->procPaint != NULL || pWidget->pUser != NULL ) {
		xgeXuiUnit(&tXui);
		return 990;
	}
	xgeXuiUnit(&tXui);
	return 0;
}

static int g_iXuiScrollBarChanges;
static float g_fXuiScrollBarLastValue;

static void __testXuiScrollBarChange(xge_xui_widget pWidget, float fValue, void* pUser)
{
	(void)pWidget;
	(void)pUser;
	g_iXuiScrollBarChanges++;
	g_fXuiScrollBarLastValue = fValue;
}

static int __testXuiScrollBar(void)
{
	xge_xui_context_t tXui;
	xge_xui_scrollbar_t tBar;
	xge_xui_widget pRoot;
	xge_xui_widget pWidget;
	xge_event_t tEvent;

	memset(&tXui, 0, sizeof(tXui));
	memset(&tBar, 0, sizeof(tBar));
	if ( xgeXuiInit(&tXui) != XGE_OK ) {
		return 570;
	}
	pRoot = xgeXuiRoot(&tXui);
	pWidget = xgeXuiWidgetCreate();
	if ( (pRoot == NULL) || (pWidget == NULL) ) {
		xgeXuiWidgetFree(pWidget);
		xgeXuiUnit(&tXui);
		return 571;
	}
	xgeXuiWidgetSetRect(pWidget, (xge_rect_t){ 10.0f, 10.0f, 10.0f, 100.0f });
	xgeXuiWidgetAdd(pRoot, pWidget);
	xgeXuiUpdate(&tXui, 0.0f);
	if ( xgeXuiScrollBarInit(&tBar, &tXui, pWidget) != XGE_OK ) {
		xgeXuiUnit(&tXui);
		return 572;
	}
	xgeXuiScrollBarSetChange(&tBar, __testXuiScrollBarChange, NULL);
	xgeXuiScrollBarSetRange(&tBar, 0.0f, 100.0f, 25.0f);
	xgeXuiScrollBarSetValue(&tBar, 50.0f);
	xgeXuiScrollBarSetColors(&tBar, XGE_COLOR_RGBA(1, 2, 3, 255), XGE_COLOR_RGBA(4, 5, 6, 255), XGE_COLOR_RGBA(7, 8, 9, 255), XGE_COLOR_RGBA(10, 11, 12, 255), XGE_COLOR_RGBA(13, 14, 15, 160), XGE_COLOR_RGBA(16, 17, 18, 120));
	if ( pWidget->procEvent != xgeXuiScrollBarEventProc || pWidget->procPaint != xgeXuiScrollBarPaintProc || xgeXuiWidgetIsFocusable(pWidget) == 0 || xgeXuiScrollBarGetValue(&tBar) != 50.0f ) {
		xgeXuiUnit(&tXui);
		return 573;
	}
	if ( xgeXuiPaint(&tXui) != 1 || tXui.iPaintCommandCount != 1 ) {
		xgeXuiUnit(&tXui);
		return 574;
	}
	g_iXuiScrollBarChanges = 0;
	g_fXuiScrollBarLastValue = 0.0f;
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_MOUSE_DOWN;
	tEvent.fX = 15.0f;
	tEvent.fY = 92.0f;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || xgeXuiScrollBarGetValue(&tBar) != 75.0f || g_iXuiScrollBarChanges != 1 || g_fXuiScrollBarLastValue != 75.0f ) {
		xgeXuiUnit(&tXui);
		return 575;
	}
	xgeXuiScrollBarSetValue(&tBar, 0.0f);
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_MOUSE_DOWN;
	tEvent.fX = 15.0f;
	tEvent.fY = 4.0f;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONTINUE ) {
		xgeXuiUnit(&tXui);
		return 576;
	}
	tEvent.fY = 24.0f;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || tXui.pCapture != pWidget || tBar.bDraggingThumb == 0 ) {
		xgeXuiUnit(&tXui);
		return 577;
	}
	tEvent.iType = XGE_EVENT_MOUSE_MOVE;
	tEvent.fY = 56.0f;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || xgeXuiScrollBarGetValue(&tBar) != 50.0f ) {
		xgeXuiUnit(&tXui);
		return 578;
	}
	tEvent.iType = XGE_EVENT_MOUSE_UP;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || tXui.pCapture != NULL || tBar.bDraggingThumb != 0 ) {
		xgeXuiUnit(&tXui);
		return 579;
	}
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_MOUSE_WHEEL;
	tEvent.fX = 15.0f;
	tEvent.fY = 20.0f;
	tEvent.fDY = 1.0f;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || xgeXuiScrollBarGetValue(&tBar) != 25.0f ) {
		xgeXuiUnit(&tXui);
		return 580;
	}
	xgeXuiScrollBarSetOrientation(&tBar, XGE_XUI_SEPARATOR_HORIZONTAL);
	xgeXuiWidgetSetRect(pWidget, (xge_rect_t){ 10.0f, 10.0f, 100.0f, 10.0f });
	xgeXuiUpdate(&tXui, 0.0f);
	xgeXuiScrollBarSetValue(&tBar, 0.0f);
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_MOUSE_DOWN;
	tEvent.fX = 24.0f;
	tEvent.fY = 15.0f;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED ) {
		xgeXuiUnit(&tXui);
		return 581;
	}
	tEvent.iType = XGE_EVENT_MOUSE_MOVE;
	tEvent.fX = 56.0f;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || xgeXuiScrollBarGetValue(&tBar) != 50.0f ) {
		xgeXuiUnit(&tXui);
		return 582;
	}
	xgeXuiScrollBarSetRange(&tBar, 100.0f, 0.0f, -10.0f);
	xgeXuiScrollBarSetValue(&tBar, 200.0f);
	if ( tBar.fMin != 0.0f || tBar.fMax != 100.0f || tBar.fPage != 0.0f || xgeXuiScrollBarGetValue(&tBar) != 100.0f ) {
		xgeXuiUnit(&tXui);
		return 583;
	}
	xgeXuiScrollBarUnit(&tBar);
	if ( pWidget->procEvent != NULL || pWidget->procPaint != NULL || pWidget->pUser != NULL ) {
		xgeXuiUnit(&tXui);
		return 584;
	}
	xgeXuiUnit(&tXui);
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
	xge_xui_widget pChild;
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
	pChild = xgeXuiWidgetCreate();
	if ( (pRoot == NULL) || (pWidget == NULL) || (pChild == NULL) ) {
		xgeXuiWidgetFree(pWidget);
		xgeXuiWidgetFree(pChild);
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
	tRect.fX = 0.0f;
	tRect.fY = 100.0f;
	tRect.fW = 20.0f;
	tRect.fH = 20.0f;
	xgeXuiWidgetSetRect(pChild, tRect);
	xgeXuiWidgetAdd(pWidget, pChild);
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
	xgeXuiScrollViewSetOffset(&tScroll, 0.0f, 50.0f);
	xgeXuiUpdate(&tXui, 0.0f);
	tRect = xgeXuiWidgetGetRect(pChild);
	if ( tRect.fX != 15.0f || tRect.fY != 65.0f || xgeXuiHitTest(&tXui, 20.0f, 70.0f) != pChild ) {
		xgeXuiUnit(&tXui);
		return 661;
	}
	xgeXuiScrollViewSetOffset(&tScroll, 0.0f, 0.0f);
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_MOUSE_WHEEL;
	tEvent.fX = 12.0f;
	tEvent.fY = 12.0f;
	tEvent.fDY = -1.0f;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) == XGE_XUI_EVENT_CONSUMED ) {
		xgeXuiUnit(&tXui);
		return 662;
	}
	xgeXuiScrollViewGetOffset(&tScroll, &fX, &fY);
	if ( fX != 0.0f || fY != 0.0f ) {
		xgeXuiUnit(&tXui);
		return 663;
	}
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
	xgeXuiScrollViewSetOffset(&tScroll, 0.0f, 0.0f);
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_MOUSE_DOWN;
	tEvent.fX = 103.0f;
	tEvent.fY = 80.0f;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || tXui.pCapture != NULL ) {
		xgeXuiUnit(&tXui);
		return 653;
	}
	xgeXuiScrollViewGetOffset(&tScroll, &fX, &fY);
	if ( fX != 0.0f || fY != 70.0f ) {
		xgeXuiUnit(&tXui);
		return 654;
	}
	xgeXuiScrollViewSetOffset(&tScroll, 0.0f, 0.0f);
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_MOUSE_DOWN;
	tEvent.fX = 103.0f;
	tEvent.fY = 20.0f;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || tScroll.bDragging != 2 || tXui.pCapture != pWidget ) {
		xgeXuiUnit(&tXui);
		return 655;
	}
	tEvent.iType = XGE_EVENT_MOUSE_MOVE;
	tEvent.fY = 43.33333f;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED ) {
		xgeXuiUnit(&tXui);
		return 656;
	}
	xgeXuiScrollViewGetOffset(&tScroll, &fX, &fY);
	if ( fX != 0.0f || fY < 69.9f || fY > 70.1f ) {
		xgeXuiUnit(&tXui);
		return 657;
	}
	tEvent.iType = XGE_EVENT_MOUSE_UP;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || tXui.pCapture != NULL || tScroll.bDragging != 0 ) {
		xgeXuiUnit(&tXui);
		return 658;
	}
	xgeXuiScrollViewSetOffset(&tScroll, 0.0f, 0.0f);
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_MOUSE_DOWN;
	tEvent.fX = 100.0f;
	tEvent.fY = 83.0f;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || tXui.pCapture != NULL ) {
		xgeXuiUnit(&tXui);
		return 659;
	}
	xgeXuiScrollViewGetOffset(&tScroll, &fX, &fY);
	if ( fX != 90.0f || fY != 0.0f ) {
		xgeXuiUnit(&tXui);
		return 660;
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

static int g_iXuiComboSelected;
static int g_iXuiComboSelectCount;

static void __testXuiComboBoxSelect(xge_xui_widget pWidget, int iIndex, void* pUser)
{
	(void)pWidget;
	(void)pUser;
	g_iXuiComboSelected = iIndex;
	g_iXuiComboSelectCount++;
}

static int g_iXuiTooltipOldCapture;

static int __testXuiTooltipOldCapture(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser)
{
	(void)pWidget;
	if ( pUser == (void*)0x1234 && pEvent != NULL && pEvent->iType == XGE_EVENT_MOUSE_MOVE ) {
		g_iXuiTooltipOldCapture++;
	}
	return XGE_XUI_EVENT_CONTINUE;
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
	xgeXuiListViewSetScroll(&tList, 0.0f);
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_MOUSE_MOVE;
	tEvent.fX = 20.0f;
	tEvent.fY = 54.0f;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONTINUE || tList.iHover != 2 ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 671;
	}
	tEvent.fX = 1.0f;
	tEvent.fY = 1.0f;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONTINUE || tList.iHover != -1 ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 672;
	}
	xgeXuiListViewSetScroll(&tList, 0.0f);
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_MOUSE_DOWN;
	tEvent.fX = 126.0f;
	tEvent.fY = 60.0f;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || xgeXuiListViewGetScroll(&tList) != 56.0f || tXui.pCapture != NULL ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 673;
	}
	xgeXuiListViewSetScroll(&tList, 0.0f);
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_MOUSE_DOWN;
	tEvent.fX = 126.0f;
	tEvent.fY = 20.0f;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || tList.bDraggingThumb == 0 || tXui.pCapture != pWidget ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 674;
	}
	tEvent.iType = XGE_EVENT_MOUSE_MOVE;
	tEvent.fY = 34.93333f;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || xgeXuiListViewGetScroll(&tList) < 31.9f || xgeXuiListViewGetScroll(&tList) > 32.1f ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 675;
	}
	tEvent.iType = XGE_EVENT_MOUSE_UP;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || tList.bDraggingThumb != 0 || tXui.pCapture != NULL ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 676;
	}
	xgeXuiListViewSetSelected(&tList, 100);
	if ( xgeXuiListViewGetSelected(&tList) != -1 ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 368;
	}
	g_iXuiListSelected = -1;
	g_iXuiListSelectCount = 0;
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_KEY_DOWN;
	tEvent.iParam1 = XGE_KEY_DOWN;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || xgeXuiListViewGetSelected(&tList) != 0 || g_iXuiListSelected != 0 || g_iXuiListSelectCount != 1 ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 677;
	}
	tEvent.iParam1 = XGE_KEY_PAGE_DOWN;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || xgeXuiListViewGetSelected(&tList) != 2 ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 678;
	}
	tEvent.iParam1 = XGE_KEY_PAGE_UP;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || xgeXuiListViewGetSelected(&tList) != 0 ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 679;
	}
	tEvent.iParam1 = XGE_KEY_END;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || xgeXuiListViewGetSelected(&tList) != 5 || xgeXuiListViewGetScroll(&tList) != 64.0f ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 680;
	}
	tEvent.iParam1 = XGE_KEY_HOME;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || xgeXuiListViewGetSelected(&tList) != 0 || xgeXuiListViewGetScroll(&tList) != 0.0f ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 681;
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

typedef struct __test_xui_virtual_list_adapter_t {
	int iCount;
	int iCreateCount;
	int iBindCount;
	int iSelected;
	int iSelectCount;
} __test_xui_virtual_list_adapter_t;

static int __testXuiVirtualListCount(xge_xui_widget pWidget, void* pUser)
{
	__test_xui_virtual_list_adapter_t* pAdapter;

	(void)pWidget;
	pAdapter = (__test_xui_virtual_list_adapter_t*)pUser;
	return (pAdapter != NULL) ? pAdapter->iCount : 0;
}

static xge_xui_widget __testXuiVirtualListCreate(xge_xui_widget pWidget, int iSlot, void* pUser)
{
	__test_xui_virtual_list_adapter_t* pAdapter;
	xge_xui_widget pItem;

	(void)pWidget;
	pAdapter = (__test_xui_virtual_list_adapter_t*)pUser;
	pItem = xgeXuiWidgetCreate();
	if ( pItem != NULL ) {
		xgeXuiWidgetSetId(pItem, 1000 + iSlot);
	}
	if ( pAdapter != NULL ) {
		pAdapter->iCreateCount++;
	}
	return pItem;
}

static void __testXuiVirtualListBind(xge_xui_widget pWidget, int iIndex, void* pUser)
{
	__test_xui_virtual_list_adapter_t* pAdapter;

	pAdapter = (__test_xui_virtual_list_adapter_t*)pUser;
	if ( pWidget != NULL ) {
		xgeXuiWidgetSetId(pWidget, 2000 + iIndex);
	}
	if ( pAdapter != NULL ) {
		pAdapter->iBindCount++;
	}
}

static void __testXuiVirtualListSelect(xge_xui_widget pWidget, int iIndex, void* pUser)
{
	__test_xui_virtual_list_adapter_t* pAdapter;

	(void)pWidget;
	pAdapter = (__test_xui_virtual_list_adapter_t*)pUser;
	if ( pAdapter != NULL ) {
		pAdapter->iSelected = iIndex;
		pAdapter->iSelectCount++;
	}
}

static int __testXuiVirtualList(void)
{
	xge_xui_context_t tXui;
	xge_xui_virtual_list_t tList;
	__test_xui_virtual_list_adapter_t tAdapter;
	xge_xui_widget pRoot;
	xge_xui_widget pWidget;
	xge_xui_widget pSlot;
	xge_event_t tEvent;
	xge_rect_t tRect;
	int iPaintCount;

	memset(&tXui, 0, sizeof(tXui));
	memset(&tList, 0, sizeof(tList));
	memset(&tAdapter, 0, sizeof(tAdapter));
	tAdapter.iCount = 100;
	tAdapter.iSelected = -1;
	if ( xgeXuiInit(&tXui) != XGE_OK ) {
		return 690;
	}
	pRoot = xgeXuiRoot(&tXui);
	pWidget = xgeXuiWidgetCreate();
	if ( (pRoot == NULL) || (pWidget == NULL) ) {
		xgeXuiWidgetFree(pWidget);
		xgeXuiUnit(&tXui);
		return 691;
	}
	tRect.fX = 10.0f;
	tRect.fY = 10.0f;
	tRect.fW = 100.0f;
	tRect.fH = 80.0f;
	xgeXuiWidgetSetRect(pWidget, tRect);
	xgeXuiWidgetSetPaddingPx(pWidget, 5.0f, 5.0f, 5.0f, 5.0f);
	xgeXuiWidgetAdd(pRoot, pWidget);
	xgeXuiUpdate(&tXui, 0.0f);
	if ( xgeXuiVirtualListInit(&tList, &tXui, pWidget) != XGE_OK ) {
		xgeXuiUnit(&tXui);
		return 692;
	}
	xgeXuiVirtualListSetAdapter(&tList, __testXuiVirtualListCount, __testXuiVirtualListCreate, __testXuiVirtualListBind, &tAdapter);
	xgeXuiVirtualListSetItemHeight(&tList, 20.0f);
	xgeXuiVirtualListSetSelect(&tList, __testXuiVirtualListSelect, &tAdapter);
	if ( (xgeXuiWidgetGetFlags(pWidget) & XGE_XUI_WIDGET_CLIP) == 0 || xgeXuiWidgetIsFocusable(pWidget) == 0 ) {
		xgeXuiUnit(&tXui);
		return 693;
	}
	xgeXuiUpdate(&tXui, 0.0f);
	if ( xgeXuiVirtualListGetFirstVisible(&tList) != 0 || xgeXuiVirtualListGetVisibleCount(&tList) != 5 || tAdapter.iCreateCount != 5 || tAdapter.iBindCount != 5 || tList.iSlotCount != 5 ) {
		xgeXuiUnit(&tXui);
		return 694;
	}
	pSlot = xgeXuiVirtualListGetSlotWidget(&tList, 4);
	tRect = xgeXuiWidgetGetRect(pSlot);
	if ( (pSlot == NULL) || (pSlot->iId != 2004) || (tRect.fX != 15.0f) || (tRect.fY != 95.0f) || (tRect.fW != 90.0f) || (tRect.fH != 20.0f) ) {
		xgeXuiUnit(&tXui);
		return 695;
	}
	xgeXuiVirtualListSetScroll(&tList, 45.0f);
	xgeXuiUpdate(&tXui, 0.0f);
	pSlot = xgeXuiVirtualListGetSlotWidget(&tList, 0);
	tRect = xgeXuiWidgetGetRect(pSlot);
	if ( xgeXuiVirtualListGetFirstVisible(&tList) != 2 || xgeXuiVirtualListGetVisibleCount(&tList) != 5 || tList.iSlotCount != 5 || tAdapter.iCreateCount != 5 || pSlot->iId != 2002 || tRect.fY != 10.0f ) {
		xgeXuiUnit(&tXui);
		return 696;
	}
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_MOUSE_WHEEL;
	tEvent.fX = 12.0f;
	tEvent.fY = 12.0f;
	tEvent.fDY = -1.0f;
	xgeXuiVirtualListSetScroll(&tList, 0.0f);
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) == XGE_XUI_EVENT_CONSUMED || xgeXuiVirtualListGetScroll(&tList) != 0.0f ) {
		xgeXuiUnit(&tXui);
		return 697;
	}
	tEvent.fX = 20.0f;
	tEvent.fY = 20.0f;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || xgeXuiVirtualListGetScroll(&tList) != 20.0f ) {
		xgeXuiUnit(&tXui);
		return 698;
	}
	xgeXuiVirtualListSetScroll(&tList, 0.0f);
	xgeXuiUpdate(&tXui, 0.0f);
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_MOUSE_DOWN;
	tEvent.fX = 20.0f;
	tEvent.fY = 36.0f;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || xgeXuiVirtualListGetSelected(&tList) != 1 || tAdapter.iSelected != 1 || tAdapter.iSelectCount != 1 || tXui.pFocus != pWidget ) {
		xgeXuiUnit(&tXui);
		return 699;
	}
	tAdapter.iCount = 3;
	xgeXuiVirtualListRefresh(&tList);
	xgeXuiUpdate(&tXui, 0.0f);
	if ( xgeXuiVirtualListGetFirstVisible(&tList) != 0 || xgeXuiVirtualListGetVisibleCount(&tList) != 3 || xgeXuiWidgetIsVisible(xgeXuiVirtualListGetSlotWidget(&tList, 3)) != 0 || xgeXuiWidgetIsVisible(xgeXuiVirtualListGetSlotWidget(&tList, 4)) != 0 ) {
		xgeXuiUnit(&tXui);
		return 700;
	}
	iPaintCount = xgeXuiPaint(&tXui);
	if ( iPaintCount != 1 || tXui.iPaintCommandCount < 1 || pWidget->procPaint != xgeXuiVirtualListPaintProc || pWidget->procEvent != xgeXuiVirtualListEventProc || pWidget->procLayout != xgeXuiVirtualListLayoutProc ) {
		xgeXuiUnit(&tXui);
		return 701;
	}
	xgeXuiVirtualListUnit(&tList);
	if ( pWidget->procEvent != NULL || pWidget->procPaint != NULL || pWidget->procLayout != NULL || pWidget->pUser != NULL || pWidget->pLayoutUser != NULL || pWidget->pFirstChild != NULL ) {
		xgeXuiUnit(&tXui);
		return 702;
	}
	xgeXuiUnit(&tXui);
	return 0;
}

static int __testXuiTooltip(void)
{
	xrf_test_blob2_t tBlob;
	xge_font_t tFont;
	xge_xui_context_t tXui;
	xge_xui_tooltip_t tTooltip;
	xge_xui_widget pRoot;
	xge_xui_widget pOwner;
	xge_event_t tEvent;
	xge_rect_t tRect;

	__testXrfBlobMake(&tBlob);
	memset(&tFont, 0, sizeof(tFont));
	memset(&tXui, 0, sizeof(tXui));
	memset(&tTooltip, 0, sizeof(tTooltip));
	if ( xgeFontLoadXRFMemory(&tFont, &tBlob, sizeof(tBlob)) != XGE_OK ) {
		return 710;
	}
	if ( xgeXuiInit(&tXui) != XGE_OK ) {
		xgeFontFree(&tFont);
		return 711;
	}
	pRoot = xgeXuiRoot(&tXui);
	pOwner = xgeXuiWidgetCreate();
	if ( (pRoot == NULL) || (pOwner == NULL) ) {
		xgeXuiWidgetFree(pOwner);
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 712;
	}
	xgeXuiWidgetSetRect(pOwner, (xge_rect_t){ 10.0f, 10.0f, 80.0f, 24.0f });
	xgeXuiWidgetSetBackground(pOwner, XGE_COLOR_RGBA(1, 2, 3, 255));
	xgeXuiWidgetSetCaptureEventUser(pOwner, __testXuiTooltipOldCapture, (void*)0x1234);
	xgeXuiWidgetAdd(pRoot, pOwner);
	xgeXuiUpdate(&tXui, 0.0f);
	if ( xgeXuiTooltipInit(&tTooltip, &tXui, pOwner) != XGE_OK ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 713;
	}
	xgeXuiTooltipSetText(&tTooltip, &tFont, "Tip");
	xgeXuiTooltipSetColors(&tTooltip, XGE_COLOR_RGBA(4, 5, 6, 255), XGE_COLOR_RGBA(7, 8, 9, 255));
	xgeXuiTooltipSetOffset(&tTooltip, 5.0f, 3.0f);
	if ( pOwner->procCaptureEvent != xgeXuiTooltipOwnerEventProc || pOwner->pCaptureUser != &tTooltip || xgeXuiTooltipIsOpen(&tTooltip) != 0 ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 714;
	}
	g_iXuiTooltipOldCapture = 0;
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_MOUSE_MOVE;
	tEvent.fX = 20.0f;
	tEvent.fY = 20.0f;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONTINUE || xgeXuiTooltipIsOpen(&tTooltip) == 0 || g_iXuiTooltipOldCapture != 1 ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 715;
	}
	xgeXuiUpdate(&tXui, 0.0f);
	tRect = xgeXuiWidgetGetRect(tTooltip.pPopupWidget);
	if ( tRect.fX != 15.0f || tRect.fY != 37.0f || xgeXuiWidgetIsVisible(tTooltip.pPopupWidget) == 0 ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 716;
	}
	tEvent.fX = 300.0f;
	tEvent.fY = 300.0f;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONTINUE || xgeXuiTooltipIsOpen(&tTooltip) != 0 ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 717;
	}
	xgeXuiTooltipSetOpen(&tTooltip, 1);
	xgeXuiTooltipSetEnabled(&tTooltip, 0);
	if ( xgeXuiTooltipIsOpen(&tTooltip) != 0 ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 718;
	}
	xgeXuiTooltipUnit(&tTooltip);
	if ( pOwner->procCaptureEvent != __testXuiTooltipOldCapture || pOwner->pCaptureUser != (void*)0x1234 ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 719;
	}
	xgeXuiUnit(&tXui);
	xgeFontFree(&tFont);
	return 0;
}

static int __testXuiComboBox(void)
{
	static const char* arrItems[] = {"One", "Two", "Three", "Four"};
	xrf_test_blob2_t tBlob;
	xge_font_t tFont;
	xge_xui_context_t tXui;
	xge_xui_combo_box_t tCombo;
	xge_xui_widget pRoot;
	xge_xui_widget pWidget;
	xge_event_t tEvent;
	xge_rect_t tRect;

	__testXrfBlobMake(&tBlob);
	memset(&tFont, 0, sizeof(tFont));
	memset(&tXui, 0, sizeof(tXui));
	memset(&tCombo, 0, sizeof(tCombo));
	if ( xgeFontLoadXRFMemory(&tFont, &tBlob, sizeof(tBlob)) != XGE_OK ) {
		return 690;
	}
	if ( xgeXuiInit(&tXui) != XGE_OK ) {
		xgeFontFree(&tFont);
		return 691;
	}
	pRoot = xgeXuiRoot(&tXui);
	pWidget = xgeXuiWidgetCreate();
	if ( (pRoot == NULL) || (pWidget == NULL) ) {
		xgeXuiWidgetFree(pWidget);
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 692;
	}
	tRect.fX = 10.0f;
	tRect.fY = 10.0f;
	tRect.fW = 120.0f;
	tRect.fH = 28.0f;
	xgeXuiWidgetSetRect(pWidget, tRect);
	xgeXuiWidgetSetPaddingPx(pWidget, 2.0f, 2.0f, 2.0f, 2.0f);
	xgeXuiWidgetAdd(pRoot, pWidget);
	xgeXuiUpdate(&tXui, 0.0f);
	if ( xgeXuiComboBoxInit(&tCombo, &tXui, pWidget) != XGE_OK ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 693;
	}
	xgeXuiComboBoxSetFont(&tCombo, &tFont);
	xgeXuiComboBoxSetItems(&tCombo, arrItems, 4);
	xgeXuiComboBoxSetSelected(&tCombo, 1);
	xgeXuiComboBoxSetDropDownHeight(&tCombo, 72.0f);
	xgeXuiComboBoxSetSelect(&tCombo, __testXuiComboBoxSelect, NULL);
	xgeXuiComboBoxSetColors(&tCombo, XGE_COLOR_RGBA(1, 2, 3, 255), XGE_COLOR_RGBA(4, 5, 6, 255), XGE_COLOR_RGBA(7, 8, 9, 255), XGE_COLOR_RGBA(10, 11, 12, 255), XGE_COLOR_RGBA(13, 14, 15, 255), XGE_COLOR_RGBA(16, 17, 18, 255), XGE_COLOR_RGBA(19, 20, 21, 255));
	if ( pWidget->procEvent != xgeXuiComboBoxEventProc || pWidget->procPaint != xgeXuiComboBoxPaintProc || xgeXuiWidgetIsFocusable(pWidget) == 0 || xgeXuiComboBoxGetSelected(&tCombo) != 1 || xgeXuiComboBoxIsOpen(&tCombo) != 0 ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 694;
	}
	if ( xgeXuiPaint(&tXui) < 1 || tXui.iPaintCommandCount < 1 ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 695;
	}
	g_iXuiComboSelected = -1;
	g_iXuiComboSelectCount = 0;
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_MOUSE_DOWN;
	tEvent.fX = 20.0f;
	tEvent.fY = 20.0f;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || xgeXuiComboBoxIsOpen(&tCombo) == 0 || tXui.pFocus != tCombo.pListWidget ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 696;
	}
	xgeXuiUpdate(&tXui, 0.0f);
	tEvent.fX = tCombo.pListWidget->tContentRect.fX + 10.0f;
	tEvent.fY = tCombo.pListWidget->tContentRect.fY + 24.0f * 2.0f + 4.0f;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 697;
	}
	if ( xgeXuiComboBoxGetSelected(&tCombo) != 2 ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 704;
	}
	if ( xgeXuiComboBoxIsOpen(&tCombo) != 0 ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 705;
	}
	if ( g_iXuiComboSelected != 2 || g_iXuiComboSelectCount != 1 ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 706;
	}
	if ( tXui.pFocus != pWidget ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 707;
	}
	tEvent.fX = 20.0f;
	tEvent.fY = 20.0f;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || xgeXuiComboBoxIsOpen(&tCombo) == 0 ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 698;
	}
	xgeXuiUpdate(&tXui, 0.0f);
	tEvent.fX = 400.0f;
	tEvent.fY = 400.0f;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || xgeXuiComboBoxIsOpen(&tCombo) != 0 ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 699;
	}
	xgeXuiSetFocus(&tXui, pWidget);
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_KEY_DOWN;
	tEvent.iParam1 = XGE_KEY_DOWN;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || xgeXuiComboBoxIsOpen(&tCombo) == 0 || tXui.pFocus != tCombo.pListWidget ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 700;
	}
	tEvent.iParam1 = XGE_KEY_ESCAPE;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || xgeXuiComboBoxIsOpen(&tCombo) != 0 ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 701;
	}
	xgeXuiComboBoxSetItems(&tCombo, arrItems, 1);
	if ( xgeXuiComboBoxGetSelected(&tCombo) != -1 ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 702;
	}
	xgeXuiComboBoxUnit(&tCombo);
	if ( pWidget->procEvent != NULL || pWidget->procPaint != NULL || pWidget->pUser != NULL ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 703;
	}
	xgeXuiUnit(&tXui);
	xgeFontFree(&tFont);
	return 0;
}

static int g_iXuiDialogCloseCount;
static int g_iXuiPopupCloseCount;
static int g_iXuiMessageBoxResultCount;
static int g_iXuiMessageBoxLastResult;

static void __testXuiDialogClose(xge_xui_widget pWidget, void* pUser)
{
	(void)pWidget;
	(void)pUser;
	g_iXuiDialogCloseCount++;
}

static void __testXuiPopupClose(xge_xui_widget pWidget, void* pUser)
{
	(void)pWidget;
	(void)pUser;
	g_iXuiPopupCloseCount++;
}

static void __testXuiMessageBoxResult(xge_xui_widget pWidget, int iResult, void* pUser)
{
	(void)pWidget;
	if ( pUser == (void*)0x4567 ) {
		g_iXuiMessageBoxResultCount++;
		g_iXuiMessageBoxLastResult = iResult;
	}
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
	if ( tDialog.bModal != 1 || tDialog.bCloseOnEscape != 1 || tDialog.bShowClose != 1 ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 720;
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
	g_iXuiDialogCloseCount = 0;
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_KEY_DOWN;
	tEvent.iParam1 = XGE_KEY_ESCAPE;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || xgeXuiDialogIsOpen(&tDialog) != 0 || g_iXuiDialogCloseCount != 1 || tDialog.iCloseCount != 2 ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 721;
	}
	xgeXuiDialogSetOpen(&tDialog, 1);
	xgeXuiDialogSetCloseOnEscape(&tDialog, 0);
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || xgeXuiDialogIsOpen(&tDialog) != 1 || tDialog.iCloseCount != 2 ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 722;
	}
	xgeXuiDialogSetModal(&tDialog, 0);
	tEvent.iType = XGE_EVENT_MOUSE_DOWN;
	tEvent.fX = 400.0f;
	tEvent.fY = 400.0f;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONTINUE ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 723;
	}
	xgeXuiDialogSetModal(&tDialog, 1);
	xgeXuiDialogSetShowClose(&tDialog, 0);
	xgeXuiPaint(&tXui);
	tEvent.fX = pWidget->tContentRect.fX + pWidget->tContentRect.fW - 8.0f;
	tEvent.fY = pWidget->tContentRect.fY + 8.0f;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || xgeXuiDialogIsOpen(&tDialog) != 1 ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 724;
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

static int __testXuiMessageBox(void)
{
	xge_xui_context_t tXui;
	xge_xui_message_box_t tBox;
	xge_xui_widget pRoot;
	xge_xui_widget pWidget;
	xge_event_t tEvent;

	memset(&tXui, 0, sizeof(tXui));
	memset(&tBox, 0, sizeof(tBox));
	if ( xgeXuiInit(&tXui) != XGE_OK ) {
		return 725;
	}
	pRoot = xgeXuiRoot(&tXui);
	pWidget = xgeXuiWidgetCreate();
	if ( (pRoot == NULL) || (pWidget == NULL) ) {
		xgeXuiWidgetFree(pWidget);
		xgeXuiUnit(&tXui);
		return 726;
	}
	xgeXuiWidgetSetRect(pWidget, (xge_rect_t){ 10.0f, 10.0f, 220.0f, 120.0f });
	xgeXuiWidgetAdd(pRoot, pWidget);
	if ( xgeXuiMessageBoxInit(&tBox, &tXui, pWidget) != XGE_OK ) {
		xgeXuiUnit(&tXui);
		return 727;
	}
	xgeXuiMessageBoxSetButtons(&tBox, XGE_XUI_MESSAGE_BOX_YES_NO_CANCEL);
	xgeXuiMessageBoxSetResult(&tBox, __testXuiMessageBoxResult, (void*)0x4567);
	g_iXuiMessageBoxResultCount = 0;
	g_iXuiMessageBoxLastResult = 0;
	xgeXuiPaint(&tXui);
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_MOUSE_DOWN;
	tEvent.fX = tBox.arrButtonRect[1].fX + 2.0f;
	tEvent.fY = tBox.arrButtonRect[1].fY + 2.0f;
	if ( xgeXuiMessageBoxEvent(&tBox, &tEvent) != XGE_XUI_EVENT_CONSUMED || xgeXuiMessageBoxIsOpen(&tBox) != 0 || xgeXuiMessageBoxGetResult(&tBox) != XGE_XUI_MESSAGE_BOX_RESULT_NO || g_iXuiMessageBoxResultCount != 1 || g_iXuiMessageBoxLastResult != XGE_XUI_MESSAGE_BOX_RESULT_NO ) {
		xgeXuiUnit(&tXui);
		return 728;
	}
	if ( xgeXuiMessageBoxEvent(&tBox, &tEvent) != XGE_XUI_EVENT_CONTINUE || g_iXuiMessageBoxResultCount != 1 ) {
		xgeXuiUnit(&tXui);
		return 729;
	}
	xgeXuiMessageBoxSetOpen(&tBox, 1);
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_KEY_DOWN;
	tEvent.iParam1 = XGE_KEY_ESCAPE;
	if ( xgeXuiMessageBoxEvent(&tBox, &tEvent) != XGE_XUI_EVENT_CONSUMED || xgeXuiMessageBoxGetResult(&tBox) != XGE_XUI_MESSAGE_BOX_RESULT_CANCEL || g_iXuiMessageBoxResultCount != 2 || g_iXuiMessageBoxLastResult != XGE_XUI_MESSAGE_BOX_RESULT_CANCEL ) {
		xgeXuiUnit(&tXui);
		return 730;
	}
	xgeXuiMessageBoxSetOpen(&tBox, 1);
	tEvent.iParam1 = XGE_KEY_ENTER;
	if ( xgeXuiMessageBoxEvent(&tBox, &tEvent) != XGE_XUI_EVENT_CONSUMED || xgeXuiMessageBoxGetResult(&tBox) != XGE_XUI_MESSAGE_BOX_RESULT_YES || g_iXuiMessageBoxResultCount != 3 || g_iXuiMessageBoxLastResult != XGE_XUI_MESSAGE_BOX_RESULT_YES ) {
		xgeXuiUnit(&tXui);
		return 731;
	}
	xgeXuiMessageBoxUnit(&tBox);
	if ( pWidget->procEvent != NULL || pWidget->procPaint != NULL || pWidget->pUser != NULL ) {
		xgeXuiUnit(&tXui);
		return 732;
	}
	xgeXuiUnit(&tXui);
	return 0;
}

static int __testXuiPopup(void)
{
	xge_xui_context_t tXui;
	xge_xui_popup_t tPopup;
	xge_xui_widget pRoot;
	xge_xui_widget pOverlayRoot;
	xge_xui_widget pOwner;
	xge_xui_widget pPopup;
	xge_event_t tEvent;
	xge_rect_t tRect;
	int iPaintCount;

	memset(&tXui, 0, sizeof(tXui));
	memset(&tPopup, 0, sizeof(tPopup));
	if ( xgeXuiInit(&tXui) != XGE_OK ) {
		return 420;
	}
	pRoot = xgeXuiRoot(&tXui);
	pOverlayRoot = xgeXuiOverlayRoot(&tXui);
	pOwner = xgeXuiWidgetCreate();
	pPopup = xgeXuiWidgetCreate();
	if ( (pRoot == NULL) || (pOverlayRoot == NULL) || (pOwner == NULL) || (pPopup == NULL) ) {
		xgeXuiWidgetFree(pOwner);
		xgeXuiWidgetFree(pPopup);
		xgeXuiUnit(&tXui);
		return 421;
	}
	tRect.fX = 10.0f;
	tRect.fY = 10.0f;
	tRect.fW = 80.0f;
	tRect.fH = 30.0f;
	xgeXuiWidgetSetRect(pOwner, tRect);
	xgeXuiWidgetSetBackground(pOwner, XGE_COLOR_RGBA(1, 2, 3, 255));
	xgeXuiWidgetAdd(pRoot, pOwner);
	tRect.fX = 20.0f;
	tRect.fY = 20.0f;
	tRect.fW = 100.0f;
	tRect.fH = 70.0f;
	xgeXuiWidgetSetRect(pPopup, tRect);
	xgeXuiWidgetAdd(pOverlayRoot, pPopup);
	if ( xgeXuiPopupInit(&tPopup, &tXui, pPopup) != XGE_OK ) {
		xgeXuiUnit(&tXui);
		return 422;
	}
	xgeXuiPopupSetOwner(&tPopup, pOwner);
	xgeXuiPopupSetClose(&tPopup, __testXuiPopupClose, NULL);
	xgeXuiPopupSetBackground(&tPopup, XGE_COLOR_RGBA(10, 20, 30, 255));
	xgeXuiPopupSetOpen(&tPopup, 1);
	xgeXuiUpdate(&tXui, 0.0f);
	if ( xgeXuiPopupIsOpen(&tPopup) != 1 || xgeXuiWidgetIsVisible(pPopup) == 0 || xgeXuiWidgetIsFocusable(pPopup) == 0 || (xgeXuiWidgetGetFlags(pPopup) & XGE_XUI_WIDGET_CLIP) == 0 ) {
		xgeXuiUnit(&tXui);
		return 423;
	}
	if ( xgeXuiHitTest(&tXui, 25.0f, 45.0f) != pPopup ) {
		xgeXuiUnit(&tXui);
		return 424;
	}
	iPaintCount = xgeXuiPaint(&tXui);
	if ( iPaintCount != 2 || tXui.iPaintCommandCount != 2 || pPopup->procEvent != xgeXuiPopupEventProc || pPopup->procPaint != xgeXuiPopupPaintProc ) {
		xgeXuiUnit(&tXui);
		return 425;
	}
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_MOUSE_DOWN;
	tEvent.fX = 12.0f;
	tEvent.fY = 12.0f;
	g_iXuiPopupCloseCount = 0;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONTINUE || xgeXuiPopupIsOpen(&tPopup) != 1 || g_iXuiPopupCloseCount != 0 ) {
		xgeXuiUnit(&tXui);
		return 426;
	}
	tEvent.fX = 300.0f;
	tEvent.fY = 300.0f;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || xgeXuiPopupIsOpen(&tPopup) != 0 || g_iXuiPopupCloseCount != 1 || tPopup.iCloseCount != 1 ) {
		xgeXuiUnit(&tXui);
		return 427;
	}
	xgeXuiPopupSetOpen(&tPopup, 1);
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_KEY_DOWN;
	tEvent.iParam1 = XGE_KEY_ESCAPE;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || xgeXuiPopupIsOpen(&tPopup) != 0 || g_iXuiPopupCloseCount != 2 || tPopup.iCloseCount != 2 ) {
		xgeXuiUnit(&tXui);
		return 428;
	}
	xgeXuiPopupUnit(&tPopup);
	if ( pPopup->procEvent != NULL || pPopup->procPaint != NULL || pPopup->pUser != NULL ) {
		xgeXuiUnit(&tXui);
		return 429;
	}
	xgeXuiUnit(&tXui);
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
	xui_host_test_t tHostState;
	xge_xui_host_t tHost;
	xge_xui_context_t tXui;
	xge_xui_image_t tImage;
	xge_xui_widget pRoot;
	xge_xui_widget pWidget;
	xge_texture_t tTexture;
	xge_rect_t tRect;
	int iPaintCount;

	memset(&tHostState, 0, sizeof(tHostState));
	memset(&tHost, 0, sizeof(tHost));
	memset(&tXui, 0, sizeof(tXui));
	memset(&tImage, 0, sizeof(tImage));
	memset(&tTexture, 0, sizeof(tTexture));
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
	tTexture.iBackendId = 0;
	if ( xgeXuiInit(&tXui) != XGE_OK ) {
		return 230;
	}
	xgeXuiSetHost(&tXui, &tHost);
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
	if ( iPaintCount != 1 || tXui.iPaintCommandCount != 1 || pWidget->procPaint != xgeXuiInputPaintProc || pWidget->procEvent != xgeXuiInputEventProc || pWidget->procUpdate != xgeXuiInputUpdateProc ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 244;
	}
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_MOUSE_DOWN;
	tEvent.iParam1 = XGE_MOUSE_LEFT;
	tEvent.fX = 12.0f;
	tEvent.fY = 14.0f;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || tXui.pFocus != pWidget ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 245;
	}
	tEvent.iType = XGE_EVENT_MOUSE_UP;
	tEvent.iParam1 = XGE_MOUSE_LEFT;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || tInput.bSelecting != 0 ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 285;
	}
	if ( tInput.bCursorVisible == 0 ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 282;
	}
	xgeXuiUpdate(&tXui, 0.51f);
	if ( tInput.bCursorVisible != 0 ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 283;
	}
	xgeXuiUpdate(&tXui, 0.51f);
	if ( tInput.bCursorVisible == 0 ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 284;
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
	xgeXuiInputSetText(&tInput, "hello world");
	xgeXuiTextSetCursor(&tInput.tText, 11);
	tEvent.iParam1 = XGE_KEY_LEFT;
	tEvent.iParam2 = XGE_KEY_MOD_CTRL;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || xgeXuiTextGetCursor(&tInput.tText) != 6 ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 272;
	}
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || xgeXuiTextGetCursor(&tInput.tText) != 0 ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 273;
	}
	tEvent.iParam1 = XGE_KEY_RIGHT;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || xgeXuiTextGetCursor(&tInput.tText) != 6 ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 274;
	}
	xgeXuiInputSetText(&tInput, "ABCD");
	tEvent.iParam1 = XGE_KEY_HOME;
	tEvent.iParam2 = 0;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || xgeXuiTextGetCursor(&tInput.tText) != 0 ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 264;
	}
	tEvent.iParam1 = XGE_KEY_END;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || xgeXuiTextGetCursor(&tInput.tText) != 4 ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 265;
	}
	tEvent.iParam1 = 'A';
	tEvent.iParam2 = XGE_KEY_MOD_CTRL;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 266;
	}
	xgeXuiInputGetSelection(&tInput, &iStart, &iEnd);
	if ( iStart != 0 || iEnd != 4 ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 267;
	}
	xgeXuiInputSetSelection(&tInput, 1, 3);
	tEvent.iParam1 = 'C';
	tEvent.iParam2 = XGE_KEY_MOD_CTRL;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || strcmp(xgeXuiInputGetText(&tInput), "ABCD") != 0 ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 275;
	}
	tEvent.iParam1 = 'X';
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || strcmp(xgeXuiInputGetText(&tInput), "AD") != 0 ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 276;
	}
	tEvent.iParam1 = 'V';
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 277;
	}
	xgeXuiInputSetText(&tInput, "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz");
	tInput.fScrollX = 12.0f;
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_KEY_DOWN;
	tEvent.iParam1 = XGE_KEY_HOME;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || tInput.fScrollX != 0.0f ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 279;
	}
	xgeXuiInputSetPlaceholder(&tInput, "placeholder");
	xgeXuiInputSetPassword(&tInput, 1);
	if ( (tInput.sPlaceholder == NULL) || (strcmp(tInput.sPlaceholder, "placeholder") != 0) || (tInput.bPassword == 0) ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 268;
	}
	xgeXuiInputSetReadonly(&tInput, 1);
	xgeXuiInputSetText(&tInput, "RO");
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_TEXT;
	tEvent.iCodepoint = 'X';
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || strcmp(xgeXuiInputGetText(&tInput), "RO") != 0 ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 269;
	}
	xgeXuiInputSetReadonly(&tInput, 0);
	xgeXuiInputSetPassword(&tInput, 0);
	xgeXuiInputSetDisabled(&tInput, 1);
	if ( ((pWidget->iFlags & XGE_XUI_WIDGET_ENABLED) != 0) || (xgeXuiInputEvent(&tInput, &tEvent) != XGE_XUI_EVENT_CONTINUE) ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 270;
	}
	xgeXuiInputSetDisabled(&tInput, 0);
	if ( (pWidget->iFlags & XGE_XUI_WIDGET_ENABLED) == 0 ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 271;
	}
	xgeXuiInputSetText(&tInput, "AA");
	tInput.fLastClickTime = xgeTimer() - 1.0;
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_MOUSE_DOWN;
	tEvent.iParam1 = XGE_MOUSE_LEFT;
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
	tEvent.iParam1 = XGE_MOUSE_LEFT;
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
	xgeXuiInputSetText(&tInput, "hello world");
	tInput.fLastClickTime = xgeTimer();
	tInput.fLastClickX = pWidget->tContentRect.fX + 1.0f;
	tInput.fLastClickY = pWidget->tContentRect.fY + 1.0f;
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_MOUSE_DOWN;
	tEvent.iParam1 = XGE_MOUSE_LEFT;
	tEvent.fX = pWidget->tContentRect.fX + 1.0f;
	tEvent.fY = pWidget->tContentRect.fY + 1.0f;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 280;
	}
	xgeXuiInputGetSelection(&tInput, &iStart, &iEnd);
	if ( iStart == iEnd ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 281;
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
	if ( pWidget->procPaint != NULL || pWidget->procEvent != NULL || pWidget->procUpdate != NULL || pWidget->pUser != NULL ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 250;
	}
	xgeXuiUnit(&tXui);
	xgeFontFree(&tFont);
	return 0;
}

static int g_iXuiSearchBoxSubmitCount;
static int g_iXuiSearchBoxClearCount;
static char g_sXuiSearchBoxSubmitText[64];

static void __testXuiSearchBoxSubmit(xge_xui_widget pWidget, const char* sText, void* pUser)
{
	(void)pWidget;
	if ( pUser == (void*)0x3579 ) {
		g_iXuiSearchBoxSubmitCount++;
		snprintf(g_sXuiSearchBoxSubmitText, sizeof(g_sXuiSearchBoxSubmitText), "%s", (sText != NULL) ? sText : "");
		g_sXuiSearchBoxSubmitText[sizeof(g_sXuiSearchBoxSubmitText) - 1] = 0;
	}
}

static void __testXuiSearchBoxClear(xge_xui_widget pWidget, const char* sText, void* pUser)
{
	(void)pWidget;
	(void)sText;
	if ( pUser == (void*)0x4680 ) {
		g_iXuiSearchBoxClearCount++;
	}
}

static int __testXuiSearchBox(void)
{
	xge_xui_context_t tXui;
	xge_xui_search_box_t tSearch;
	xge_xui_widget pRoot;
	xge_xui_widget pWidget;
	xge_event_t tEvent;

	memset(&tXui, 0, sizeof(tXui));
	memset(&tSearch, 0, sizeof(tSearch));
	memset(g_sXuiSearchBoxSubmitText, 0, sizeof(g_sXuiSearchBoxSubmitText));
	g_iXuiSearchBoxSubmitCount = 0;
	g_iXuiSearchBoxClearCount = 0;
	if ( xgeXuiInit(&tXui) != XGE_OK ) {
		return 957;
	}
	pRoot = xgeXuiRoot(&tXui);
	pWidget = xgeXuiWidgetCreate();
	if ( (pRoot == NULL) || (pWidget == NULL) ) {
		xgeXuiWidgetFree(pWidget);
		xgeXuiUnit(&tXui);
		return 958;
	}
	xgeXuiWidgetSetRect(pWidget, (xge_rect_t){ 10.0f, 10.0f, 180.0f, 28.0f });
	xgeXuiWidgetAdd(pRoot, pWidget);
	if ( xgeXuiSearchBoxInit(&tSearch, &tXui, pWidget, NULL) != XGE_OK ) {
		xgeXuiUnit(&tXui);
		return 959;
	}
	xgeXuiSearchBoxSetSubmit(&tSearch, __testXuiSearchBoxSubmit, (void*)0x3579);
	xgeXuiSearchBoxSetClear(&tSearch, __testXuiSearchBoxClear, (void*)0x4680);
	xgeXuiSearchBoxSetPlaceholder(&tSearch, "Find");
	xgeXuiSearchBoxSetColors(&tSearch, XGE_COLOR_RGBA(1, 2, 3, 255), XGE_COLOR_RGBA(4, 5, 6, 255), XGE_COLOR_RGBA(7, 8, 9, 255), XGE_COLOR_RGBA(10, 11, 12, 255), XGE_COLOR_RGBA(13, 14, 15, 255), XGE_COLOR_RGBA(16, 17, 18, 255));
	if ( pWidget->procEvent != xgeXuiSearchBoxEventProc || pWidget->procUpdate != xgeXuiSearchBoxUpdateProc || pWidget->procPaint != xgeXuiSearchBoxPaintProc || tSearch.tInput.sPlaceholder == NULL || strcmp(tSearch.tInput.sPlaceholder, "Find") != 0 || tSearch.bSuggestionsReserved == 0 ) {
		xgeXuiUnit(&tXui);
		return 960;
	}
	xgeXuiSearchBoxSetText(&tSearch, "asset");
	if ( strcmp(xgeXuiSearchBoxGetText(&tSearch), "asset") != 0 ) {
		xgeXuiUnit(&tXui);
		return 961;
	}
	xgeXuiSetFocus(&tXui, pWidget);
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_KEY_DOWN;
	tEvent.iParam1 = XGE_KEY_ENTER;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || g_iXuiSearchBoxSubmitCount != 1 || strcmp(g_sXuiSearchBoxSubmitText, "asset") != 0 || tSearch.iSubmitCount != 1 ) {
		xgeXuiUnit(&tXui);
		return 962;
	}
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_MOUSE_MOVE;
	tEvent.fX = tSearch.tClearRect.fX + 3.0f;
	tEvent.fY = tSearch.tClearRect.fY + 3.0f;
	xgeXuiDispatchEvent(&tXui, &tEvent);
	if ( tSearch.bClearHover == 0 ) {
		xgeXuiUnit(&tXui);
		return 963;
	}
	tEvent.iType = XGE_EVENT_MOUSE_DOWN;
	tEvent.iParam1 = XGE_MOUSE_LEFT;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || strcmp(xgeXuiSearchBoxGetText(&tSearch), "") != 0 || g_iXuiSearchBoxClearCount != 1 || tSearch.iClearCount != 1 ) {
		xgeXuiUnit(&tXui);
		return 964;
	}
	xgeXuiSearchBoxSetSuggestionsReserved(&tSearch, 0);
	if ( tSearch.bSuggestionsReserved != 0 ) {
		xgeXuiUnit(&tXui);
		return 965;
	}
	if ( xgeXuiPaint(&tXui) != 1 ) {
		xgeXuiUnit(&tXui);
		return 966;
	}
	xgeXuiSearchBoxUnit(&tSearch);
	if ( pWidget->procEvent != NULL || pWidget->procUpdate != NULL || pWidget->procPaint != NULL || pWidget->pUser != NULL ) {
		xgeXuiUnit(&tXui);
		return 967;
	}
	xgeXuiUnit(&tXui);
	return 0;
}

static int g_iXuiColorPickerChangeCount;
static uint32_t g_iXuiColorPickerLastColor;

static void __testXuiColorPickerChange(xge_xui_widget pWidget, uint32_t iColor, void* pUser)
{
	(void)pWidget;
	if ( pUser == (void*)0x5791 ) {
		g_iXuiColorPickerChangeCount++;
		g_iXuiColorPickerLastColor = iColor;
	}
}

static int __testXuiColorPicker(void)
{
	xge_xui_context_t tXui;
	xge_xui_color_picker_t tPicker;
	xge_xui_widget pRoot;
	xge_xui_widget pWidget;
	xge_event_t tEvent;
	uint32_t arrPalette[3];
	int iR;
	int iG;
	int iB;
	int iA;

	memset(&tXui, 0, sizeof(tXui));
	memset(&tPicker, 0, sizeof(tPicker));
	g_iXuiColorPickerChangeCount = 0;
	g_iXuiColorPickerLastColor = 0;
	if ( xgeXuiInit(&tXui) != XGE_OK ) {
		return 968;
	}
	pRoot = xgeXuiRoot(&tXui);
	pWidget = xgeXuiWidgetCreate();
	if ( (pRoot == NULL) || (pWidget == NULL) ) {
		xgeXuiWidgetFree(pWidget);
		xgeXuiUnit(&tXui);
		return 969;
	}
	xgeXuiWidgetSetRect(pWidget, (xge_rect_t){ 10.0f, 10.0f, 260.0f, 132.0f });
	xgeXuiWidgetAdd(pRoot, pWidget);
	if ( xgeXuiColorPickerInit(&tPicker, &tXui, pWidget, NULL) != XGE_OK ) {
		xgeXuiUnit(&tXui);
		return 970;
	}
	xgeXuiColorPickerSetChange(&tPicker, __testXuiColorPickerChange, (void*)0x5791);
	xgeXuiColorPickerSetColors(&tPicker, XGE_COLOR_RGBA(1, 2, 3, 255), XGE_COLOR_RGBA(4, 5, 6, 255), XGE_COLOR_RGBA(7, 8, 9, 255), XGE_COLOR_RGBA(10, 11, 12, 255), XGE_COLOR_RGBA(13, 14, 15, 255), XGE_COLOR_RGBA(16, 17, 18, 255));
	if ( pWidget->procEvent != xgeXuiColorPickerEventProc || pWidget->procPaint != xgeXuiColorPickerPaintProc || xgeXuiWidgetIsFocusable(pWidget) == 0 || (xgeXuiWidgetGetFlags(pWidget) & XGE_XUI_WIDGET_CLIP) == 0 || xgeXuiColorPickerGetPaletteCount(&tPicker) != 8 ) {
		xgeXuiUnit(&tXui);
		return 971;
	}
	xgeXuiColorPickerSetRGBA(&tPicker, 1, 2, 3, 4);
	xgeXuiColorPickerGetRGBA(&tPicker, &iR, &iG, &iB, &iA);
	if ( xgeXuiColorPickerGetColor(&tPicker) != XGE_COLOR_RGBA(1, 2, 3, 4) || iR != 1 || iG != 2 || iB != 3 || iA != 4 || strcmp(xgeXuiColorPickerGetHex(&tPicker), "#01020304") != 0 ) {
		xgeXuiUnit(&tXui);
		return 972;
	}
	if ( xgeXuiColorPickerSetHex(&tPicker, "#AABBCC") != XGE_OK || xgeXuiColorPickerGetColor(&tPicker) != XGE_COLOR_RGBA(0xAA, 0xBB, 0xCC, 0xFF) || xgeXuiColorPickerSetHex(&tPicker, "bad") == XGE_OK ) {
		xgeXuiUnit(&tXui);
		return 973;
	}
	arrPalette[0] = XGE_COLOR_RGBA(10, 20, 30, 255);
	arrPalette[1] = XGE_COLOR_RGBA(40, 50, 60, 255);
	arrPalette[2] = XGE_COLOR_RGBA(70, 80, 90, 255);
	xgeXuiColorPickerSetPalette(&tPicker, arrPalette, 3);
	if ( xgeXuiColorPickerGetPaletteCount(&tPicker) != 3 ) {
		xgeXuiUnit(&tXui);
		return 974;
	}
	if ( xgeXuiPaint(&tXui) != 1 ) {
		xgeXuiUnit(&tXui);
		return 975;
	}
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_MOUSE_MOVE;
	tEvent.fX = tPicker.arrPaletteRect[1].fX + 2.0f;
	tEvent.fY = tPicker.arrPaletteRect[1].fY + 2.0f;
	xgeXuiDispatchEvent(&tXui, &tEvent);
	if ( tPicker.iHoverPalette != 1 ) {
		xgeXuiUnit(&tXui);
		return 976;
	}
	tEvent.iType = XGE_EVENT_MOUSE_DOWN;
	tEvent.iParam1 = XGE_MOUSE_LEFT;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || xgeXuiColorPickerGetColor(&tPicker) != arrPalette[1] || g_iXuiColorPickerChangeCount != 1 || g_iXuiColorPickerLastColor != arrPalette[1] || tPicker.iSelectedPalette != 1 ) {
		xgeXuiUnit(&tXui);
		return 977;
	}
	tEvent.fX = tPicker.arrFieldRect[2].fX + 2.0f;
	tEvent.fY = tPicker.arrFieldRect[2].fY + 2.0f;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || tPicker.iActiveField != 2 ) {
		xgeXuiUnit(&tXui);
		return 978;
	}
	xgeXuiColorPickerUnit(&tPicker);
	if ( pWidget->procEvent != NULL || pWidget->procPaint != NULL || pWidget->pUser != NULL ) {
		xgeXuiUnit(&tXui);
		return 979;
	}
	xgeXuiUnit(&tXui);
	return 0;
}

static int g_iXuiNumericInputChanges;
static float g_fXuiNumericInputLastValue;

static void __testXuiNumericInputChange(xge_xui_widget pWidget, float fValue, void* pUser)
{
	(void)pWidget;
	(void)pUser;
	g_iXuiNumericInputChanges++;
	g_fXuiNumericInputLastValue = fValue;
}

static int __testXuiNumericInputFormat(float fValue, char* sBuffer, int iSize, void* pUser)
{
	(void)pUser;
	if ( (sBuffer == NULL) || (iSize <= 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	snprintf(sBuffer, (size_t)iSize, "v=%.1f", fValue);
	sBuffer[iSize - 1] = 0;
	return XGE_OK;
}

static int __testXuiNumericInput(void)
{
	xge_xui_context_t tXui;
	xge_xui_numeric_input_t tNumeric;
	xge_xui_widget pRoot;
	xge_xui_widget pWidget;
	xge_event_t tEvent;

	memset(&tXui, 0, sizeof(tXui));
	memset(&tNumeric, 0, sizeof(tNumeric));
	if ( xgeXuiInit(&tXui) != XGE_OK ) {
		return 780;
	}
	pRoot = xgeXuiRoot(&tXui);
	pWidget = xgeXuiWidgetCreate();
	if ( (pRoot == NULL) || (pWidget == NULL) ) {
		xgeXuiWidgetFree(pWidget);
		xgeXuiUnit(&tXui);
		return 781;
	}
	xgeXuiWidgetSetRect(pWidget, (xge_rect_t){ 10.0f, 10.0f, 100.0f, 24.0f });
	xgeXuiWidgetSetPaddingPx(pWidget, 4.0f, 3.0f, 4.0f, 3.0f);
	xgeXuiWidgetAdd(pRoot, pWidget);
	xgeXuiUpdate(&tXui, 0.0f);
	if ( xgeXuiNumericInputInit(&tNumeric, &tXui, pWidget, NULL) != XGE_OK ) {
		xgeXuiUnit(&tXui);
		return 782;
	}
	xgeXuiNumericInputSetChange(&tNumeric, __testXuiNumericInputChange, NULL);
	xgeXuiNumericInputSetRange(&tNumeric, -10.0f, 10.0f);
	xgeXuiNumericInputSetStep(&tNumeric, 2.0f);
	xgeXuiNumericInputSetValue(&tNumeric, 4.0f);
	if ( pWidget->procEvent != xgeXuiNumericInputEventProc || pWidget->procUpdate != xgeXuiNumericInputUpdateProc || pWidget->procPaint != xgeXuiNumericInputPaintProc || xgeXuiNumericInputGetValue(&tNumeric) != 4.0f || strcmp(xgeXuiInputGetText(&tNumeric.tInput), "4.000") != 0 ) {
		xgeXuiUnit(&tXui);
		return 783;
	}
	xgeXuiNumericInputSetInteger(&tNumeric, 1);
	if ( strcmp(xgeXuiInputGetText(&tNumeric.tInput), "4") != 0 ) {
		xgeXuiUnit(&tXui);
		return 784;
	}
	xgeXuiSetFocus(&tXui, pWidget);
	g_iXuiNumericInputChanges = 0;
	g_fXuiNumericInputLastValue = 0.0f;
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_KEY_DOWN;
	tEvent.iParam1 = XGE_KEY_UP;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || xgeXuiNumericInputGetValue(&tNumeric) != 6.0f || g_iXuiNumericInputChanges != 1 || g_fXuiNumericInputLastValue != 6.0f ) {
		xgeXuiUnit(&tXui);
		return 785;
	}
	tEvent.iParam1 = XGE_KEY_DOWN;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || xgeXuiNumericInputGetValue(&tNumeric) != 4.0f || g_iXuiNumericInputChanges != 2 || g_fXuiNumericInputLastValue != 4.0f ) {
		xgeXuiUnit(&tXui);
		return 786;
	}
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_MOUSE_DOWN;
	tEvent.fX = 102.0f;
	tEvent.fY = 16.0f;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || xgeXuiNumericInputGetValue(&tNumeric) != 6.0f ) {
		xgeXuiUnit(&tXui);
		return 787;
	}
	tEvent.fY = 29.0f;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || xgeXuiNumericInputGetValue(&tNumeric) != 4.0f ) {
		xgeXuiUnit(&tXui);
		return 788;
	}
	xgeXuiInputSetText(&tNumeric.tInput, "9");
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_KEY_DOWN;
	tEvent.iParam1 = XGE_KEY_ENTER;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || xgeXuiNumericInputGetValue(&tNumeric) != 9.0f || strcmp(xgeXuiInputGetText(&tNumeric.tInput), "9") != 0 ) {
		xgeXuiUnit(&tXui);
		return 789;
	}
	xgeXuiInputSetText(&tNumeric.tInput, "bad");
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_KEY_DOWN;
	tEvent.iParam1 = XGE_KEY_ENTER;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || xgeXuiNumericInputGetValue(&tNumeric) != 9.0f || tNumeric.bError == 0 || strcmp(xgeXuiInputGetText(&tNumeric.tInput), "bad") != 0 ) {
		xgeXuiUnit(&tXui);
		return 793;
	}
	tEvent.iParam1 = XGE_KEY_UP;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || xgeXuiNumericInputGetValue(&tNumeric) != 9.0f || tNumeric.bError == 0 ) {
		xgeXuiUnit(&tXui);
		return 794;
	}
	xgeXuiInputSetText(&tNumeric.tInput, "7");
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_XUI_FOCUS_OUT;
	if ( xgeXuiNumericInputEvent(&tNumeric, &tEvent) != XGE_XUI_EVENT_CONTINUE || xgeXuiNumericInputGetValue(&tNumeric) != 7.0f || tNumeric.bError != 0 ) {
		xgeXuiUnit(&tXui);
		return 795;
	}
	xgeXuiNumericInputSetValue(&tNumeric, 99.0f);
	if ( xgeXuiNumericInputGetValue(&tNumeric) != 10.0f ) {
		xgeXuiUnit(&tXui);
		return 790;
	}
	xgeXuiNumericInputSetFormatter(&tNumeric, __testXuiNumericInputFormat, NULL);
	if ( strcmp(xgeXuiInputGetText(&tNumeric.tInput), "v=10.0") != 0 ) {
		xgeXuiUnit(&tXui);
		return 796;
	}
	xgeXuiNumericInputSetFormatter(&tNumeric, NULL, NULL);
	if ( strcmp(xgeXuiInputGetText(&tNumeric.tInput), "10") != 0 ) {
		xgeXuiUnit(&tXui);
		return 797;
	}
	if ( xgeXuiPaint(&tXui) != 1 ) {
		xgeXuiUnit(&tXui);
		return 791;
	}
	xgeXuiNumericInputUnit(&tNumeric);
	if ( pWidget->procEvent != NULL || pWidget->procUpdate != NULL || pWidget->procPaint != NULL || pWidget->pUser != NULL ) {
		xgeXuiUnit(&tXui);
		return 792;
	}
	xgeXuiUnit(&tXui);
	return 0;
}

static int __testXuiTextEdit(void)
{
	xrf_test_blob2_t tBlob;
	xge_font_t tFont;
	xge_xui_context_t tXui;
	xge_xui_text_edit_t tEdit;
	xge_xui_widget pRoot;
	xge_xui_widget pWidget;
	xge_event_t tEvent;
	xge_rect_t tRect;
	xge_rect_t tCandidate;
	int iStart;
	int iEnd;

	__testXrfBlobMake(&tBlob);
	memset(&tFont, 0, sizeof(tFont));
	memset(&tXui, 0, sizeof(tXui));
	memset(&tEdit, 0, sizeof(tEdit));
	if ( xgeFontLoadXRFMemory(&tFont, &tBlob, sizeof(tBlob)) != XGE_OK ) {
		return 720;
	}
	if ( xgeXuiInit(&tXui) != XGE_OK ) {
		xgeFontFree(&tFont);
		return 721;
	}
	pRoot = xgeXuiRoot(&tXui);
	pWidget = xgeXuiWidgetCreate();
	if ( (pRoot == NULL) || (pWidget == NULL) ) {
		xgeXuiWidgetFree(pWidget);
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 722;
	}
	xgeXuiWidgetSetRect(pRoot, (xge_rect_t){ 0.0f, 0.0f, 240.0f, 180.0f });
	xgeXuiWidgetSetRect(pWidget, (xge_rect_t){ 10.0f, 12.0f, 120.0f, 72.0f });
	xgeXuiWidgetSetPaddingPx(pWidget, 4.0f, 4.0f, 4.0f, 4.0f);
	xgeXuiWidgetAdd(pRoot, pWidget);
	if ( xgeXuiTextEditInit(&tEdit, &tXui, pWidget, &tFont) != XGE_OK ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 723;
	}
	xgeXuiTextEditSetColors(&tEdit, XGE_COLOR_RGBA(1, 2, 3, 255), XGE_COLOR_RGBA(4, 5, 6, 255), XGE_COLOR_RGBA(7, 8, 9, 255), XGE_COLOR_RGBA(10, 11, 12, 255));
	xgeXuiUpdate(&tXui, 0.0f);
	if ( pWidget->procEvent != xgeXuiTextEditEventProc || pWidget->procUpdate != xgeXuiTextEditUpdateProc || pWidget->procPaint != xgeXuiTextEditPaintProc || xgeXuiPaint(&tXui) != 1 ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 724;
	}
	xgeXuiTextEditSetText(&tEdit, "ab\ncd");
	if ( strcmp(xgeXuiTextEditGetText(&tEdit), "ab\ncd") != 0 ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 725;
	}
	if ( xgeXuiPaint(&tXui) != 1 || tEdit.iLineCount != 2 || tEdit.arrLineStarts == NULL || tEdit.arrLineStarts[0] != 0 || tEdit.arrLineStarts[1] != 3 || tEdit.bLineCacheDirty != 0 ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 752;
	}
	xgeXuiSetFocus(&tXui, pWidget);
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_KEY_DOWN;
	tEvent.iParam1 = XGE_KEY_ENTER;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || strcmp(xgeXuiTextEditGetText(&tEdit), "ab\ncd\n") != 0 ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 726;
	}
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_TEXT;
	tEvent.iCodepoint = 'e';
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || strcmp(xgeXuiTextEditGetText(&tEdit), "ab\ncd\ne") != 0 ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 727;
	}
	if ( tEdit.iLineCount != 3 || tEdit.arrLineStarts == NULL || tEdit.arrLineStarts[2] != 6 || tEdit.bLineCacheDirty != 0 ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 753;
	}
	if ( xgeXuiTextEditUndo(&tEdit) != XGE_OK || strcmp(xgeXuiTextEditGetText(&tEdit), "ab\ncd\n") != 0 ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 754;
	}
	if ( xgeXuiTextEditRedo(&tEdit) != XGE_OK || strcmp(xgeXuiTextEditGetText(&tEdit), "ab\ncd\ne") != 0 ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 755;
	}
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_KEY_DOWN;
	tEvent.iParam1 = 'Z';
	tEvent.iParam2 = XGE_KEY_MOD_CTRL;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || strcmp(xgeXuiTextEditGetText(&tEdit), "ab\ncd\n") != 0 ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 756;
	}
	tEvent.iParam1 = 'Y';
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || strcmp(xgeXuiTextEditGetText(&tEdit), "ab\ncd\ne") != 0 ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 757;
	}
	xgeXuiTextEditSetText(&tEdit, "abcdefghijklmnopqrstuvwxyz abcdefghijklmnopqrstuvwxyz abcdefghijklmnopqrstuvwxyz");
	xgeXuiWidgetSetRect(pWidget, (xge_rect_t){ 10.0f, 12.0f, 48.0f, 72.0f });
	xgeXuiUpdate(&tXui, 0.0f);
	xgeXuiTextEditSetWordWrap(&tEdit, 1);
	if ( xgeXuiPaint(&tXui) != 1 || tEdit.bWordWrap == 0 || tEdit.arrVisualLines == NULL || tEdit.iVisualLineCount < tEdit.iLineCount || tEdit.bVisualCacheDirty != 0 || tEdit.fScrollX != 0.0f ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 758;
	}
	xgeXuiTextEditSetWordWrap(&tEdit, 0);
	xgeXuiWidgetSetRect(pWidget, (xge_rect_t){ 10.0f, 12.0f, 120.0f, 72.0f });
	xgeXuiUpdate(&tXui, 0.0f);
	xgeXuiTextEditSetText(&tEdit, "ab\ncd\ne");
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_KEY_DOWN;
	tEvent.iParam1 = XGE_KEY_HOME;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || xgeXuiTextGetCursor(&tEdit.tText) != 6 ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 728;
	}
	tEvent.iParam1 = XGE_KEY_UP;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 729;
	}
	tEvent.iParam1 = XGE_KEY_END;
	tEvent.iParam2 = XGE_KEY_MOD_CTRL;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || xgeXuiTextGetCursor(&tEdit.tText) != tEdit.tText.iSize ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 737;
	}
	tEvent.iParam1 = XGE_KEY_HOME;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || xgeXuiTextGetCursor(&tEdit.tText) != 0 ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 738;
	}
	tEvent.iParam1 = XGE_KEY_PAGE_DOWN;
	tEvent.iParam2 = 0;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || xgeXuiTextGetCursor(&tEdit.tText) == 0 ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 739;
	}
	tEvent.iParam1 = 'A';
	tEvent.iParam2 = XGE_KEY_MOD_CTRL;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 740;
	}
	xgeXuiTextGetSelection(&tEdit.tText, &iStart, &iEnd);
	if ( iStart != 0 || iEnd != tEdit.tText.iSize ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 741;
	}
	tEvent.iParam1 = 'X';
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || xgeXuiTextEditGetText(&tEdit)[0] != 0 ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 742;
	}
	tEvent.iParam1 = 'V';
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 743;
	}
	xgeXuiTextEditSetText(&tEdit, "ab\ncd\nef");
	tEvent.iType = XGE_EVENT_KEY_DOWN;
	tEvent.iParam1 = XGE_KEY_HOME;
	tEvent.iParam2 = XGE_KEY_MOD_CTRL;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 744;
	}
	tEvent.iParam1 = XGE_KEY_RIGHT;
	tEvent.iParam2 = XGE_KEY_MOD_SHIFT;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 745;
	}
	xgeXuiTextGetSelection(&tEdit.tText, &iStart, &iEnd);
	if ( iStart == iEnd ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 746;
	}
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_MOUSE_DOWN;
	tEvent.iParam1 = XGE_MOUSE_LEFT;
	tEvent.fX = pWidget->tContentRect.fX + 1.0f;
	tEvent.fY = pWidget->tContentRect.fY + 1.0f;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 748;
	}
	tEvent.iType = XGE_EVENT_MOUSE_MOVE;
	tEvent.fX = pWidget->tContentRect.fX + 30.0f;
	tEvent.fY = pWidget->tContentRect.fY + 30.0f;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 749;
	}
	xgeXuiTextGetSelection(&tEdit.tText, &iStart, &iEnd);
	if ( iStart == iEnd ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 750;
	}
	tEvent.iType = XGE_EVENT_MOUSE_UP;
	tEvent.iParam1 = XGE_MOUSE_LEFT;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 751;
	}
	tCandidate = xgeXuiTextEditGetCandidateRect(&tEdit);
	if ( tCandidate.fH <= 0.0f || tCandidate.fY < pWidget->tContentRect.fY ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 730;
	}
	xgeXuiTextEditSetText(&tEdit, "a\nb\nc\nd\ne\nf\ng\nh");
	if ( tEdit.fScrollY <= 0.0f ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 731;
	}
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_KEY_DOWN;
	tEvent.iParam1 = XGE_KEY_BACKSPACE;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 732;
	}
	xgeXuiTextEditSetReadonly(&tEdit, 1);
	xgeXuiTextEditSetText(&tEdit, "readonly");
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_TEXT;
	tEvent.iCodepoint = 'x';
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || strcmp(xgeXuiTextEditGetText(&tEdit), "readonly") != 0 ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 733;
	}
	xgeXuiTextEditSetScroll(&tEdit, 4.0f, 5.0f);
	if ( tEdit.fScrollX != 4.0f || tEdit.fScrollY != 5.0f ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 734;
	}
	tRect = xgeXuiTextEditGetCandidateRect(&tEdit);
	if ( tRect.fW <= 0.0f || tRect.fH <= 0.0f ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 735;
	}
	xgeXuiTextEditUnit(&tEdit);
	if ( pWidget->procEvent != NULL || pWidget->procUpdate != NULL || pWidget->procPaint != NULL || pWidget->pUser != NULL ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 736;
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

static int __testXuiPageApi(void)
{
	xge_xui_context_t tXui;
	xge_xui_binder_t tBinder;
	xge_xui_model_t tModel;
	xge_xui_page_t tPage;
	xge_xui_widget pRoot;
	xge_xui_widget pChild;
	xge_xui_widget pGrid;
	xge_xui_button pPageButton;
	xge_xui_image pPageImage;
	xge_xui_input pPageInput;
	xge_xui_numeric_input pPageNumeric;
	xge_xui_checkbox pPageCheckBox;
	xge_xui_radio pPageRadio;
	xge_xui_switch pPageSwitch;
	xge_xui_slider pPageSlider;
	xge_xui_progress pPageProgress;
	xge_xui_tabs pPageTabs;
	xge_xui_toolbar pPageToolbar;
	xge_xui_status_bar pPageStatusBar;
	xge_xui_combo_box pPageCombo;
	xge_xui_popup pPagePopup;
	xge_xui_tooltip pPageTooltip;
	xge_xui_menu pPageMenu;
	xge_xui_dialog pPageDialog;
	xge_xui_message_box pPageMessageBox;
	xge_xui_label pPageLabel;
	xge_xui_separator pPageSeparator;
	xge_xui_scroll_view pPageScroll;
	xge_xui_tree_view pPageTree;
	xge_xui_table_view pPageTable;
	xge_xui_property_grid pPagePropertyGrid;
	xge_xui_breadcrumb pPageBreadcrumb;
	xge_xui_accordion pPageAccordion;
	xge_xui_toast pPageToast;
	xge_xui_virtual_list pPageVirtualList;
	xge_resource_provider_t tProvider;
	xge_font_t tFont;
	xge_texture_t tTexture;
	xge_event_t tEvent;
	xge_rect_t tRect;
	const char* sError;
	static const char sXson[] = "{ \"xui\": 1, \"tokens\": { \"colors\": { \"panel\": \"#112233\" }, \"spacing\": { \"baseWidth\": 120, \"gap\": 8, \"margin\": 4, \"padX\": 2, \"padY\": 3, \"childHeight\": 32, \"gridColumnGap\": 5 } }, \"styles\": { \"base\": { \"width\": \"@spacing.baseWidth\", \"height\": \"grow\", \"alignX\": \"stretch\", \"background\": \"@colors.panel\", \"padding\": [\"@spacing.padX\", \"@spacing.padY\"] }, \"panel\": { \"@parent\": \"base\", \"layout\": \"row\", \"gap\": \"@spacing.gap\", \"margin\": \"@spacing.margin\" } }, \"tree\": { \"type\": \"column\", \"id\": \"root\", \"style\": \"panel\", \"height\": 200, \"onClick\": \"ok\", \"children\": [ { \"type\": \"row\", \"name\": \"child\", \"style\": \"base\", \"width\": \"50%\", \"height\": \"@spacing.childHeight\", \"margin\": [1, 2, 3, 4], \"anchor\": { \"left\": 10, \"right\": \"5%\" }, \"clip\": true }, { \"type\": \"grid\", \"name\": \"grid-child\", \"grid\": { \"columns\": 3, \"rowHeight\": 12, \"columnGap\": \"@spacing.gridColumnGap\", \"rowGap\": 6 }, \"columnSpan\": 2, \"width\": { \"unit\": \"grow\", \"value\": 2 } } ] } }";
	static const char sStyleCycleXson[] = "{ \"xui\": 1, \"styles\": { \"a\": { \"@parent\": \"b\", \"width\": 10 }, \"b\": { \"@parent\": \"a\", \"height\": 10 } }, \"tree\": { \"id\": \"root\", \"style\": \"a\" } }";
	static const char sMissingTokenXson[] = "{ \"xui\": 1, \"styles\": { \"root\": { \"background\": \"@colors.missing\" } }, \"tree\": { \"id\": \"root\", \"style\": \"root\" } }";
	static const char sContextTokenXson[] = "{ \"xui\": 1, \"styles\": { \"root\": { \"width\": \"@spacing.ctxWidth\", \"background\": \"@colors.ctxPanel\" } }, \"tree\": { \"id\": \"root\", \"style\": \"root\" } }";
	static const char sDockXson[] = "{ \"xui\": 1, \"tree\": { \"type\": \"dock\", \"id\": \"dock-root\", \"padding\": 10, \"children\": [ { \"type\": \"panel\", \"id\": \"dock-top\", \"dock\": \"top\", \"height\": 20 }, { \"type\": \"panel\", \"id\": \"dock-left\", \"dock\": \"left\", \"width\": 40 }, { \"type\": \"panel\", \"id\": \"dock-fill\", \"dock\": \"fill\", \"width\": \"grow\", \"height\": \"grow\" } ] } }";
	static const char sSafeAreaXson[] = "{ \"xui\": 1, \"tokens\": { \"spacing\": { \"safeX\": 11, \"safeY\": 22 } }, \"safeArea\": [\"@spacing.safeX\", \"@spacing.safeY\"], \"tree\": { \"id\": \"safe-root\" } }";
	static const char sScrollViewXson[] = "{ \"xui\": 1, \"tokens\": { \"spacing\": { \"cw\": 160, \"ch\": 170, \"oy\": 25 } }, \"tree\": { \"type\": \"scrollView\", \"id\": \"scroll-root\", \"width\": 100, \"height\": 80, \"padding\": 5, \"contentSize\": [\"@spacing.cw\", \"@spacing.ch\"], \"offset\": { \"x\": 10, \"y\": \"@spacing.oy\" }, \"backgroundColor\": \"#01020304\", \"barColor\": \"#11121314\", \"thumbColor\": \"#21222324\", \"children\": [ { \"type\": \"panel\", \"id\": \"scroll-child\", \"width\": 20, \"height\": 20, \"anchor\": { \"left\": 0, \"top\": 40 } } ] } }";
	static const char sTreeViewXson[] = "{ \"xui\": 1, \"tokens\": { \"spacing\": { \"itemH\": 20, \"indent\": 14 } }, \"styles\": { \"treeStyle\": { \"font\": \"@fonts.body\", \"itemHeight\": \"@spacing.itemH\", \"indent\": \"@spacing.indent\", \"backgroundColor\": \"#01020304\", \"rowColor\": \"#11121314\", \"selectedColor\": \"#21222324\", \"textColor\": \"#313233FF\", \"barColor\": \"#41424344\", \"thumbColor\": \"#51525354\", \"expanderColor\": \"#61626364\", \"disabledTextColor\": \"#71727374\" } }, \"tree\": { \"type\": \"treeView\", \"id\": \"tree-root\", \"style\": \"treeStyle\", \"width\": 160, \"height\": 82, \"selected\": 30, \"nodes\": [ { \"id\": 10, \"text\": \"Root\", \"expanded\": true }, { \"id\": 20, \"parent\": 10, \"text\": \"Child\" }, { \"id\": 30, \"parent\": 10, \"text\": \"Leaf\" }, { \"id\": 40, \"text\": \"Other\" } ] } }";
	static const char sTreeViewSelectXson[] = "{ \"xui\": 1, \"tree\": { \"type\": \"treeView\", \"id\": \"tree-root\", \"onSelect\": \"changed\" } }";
	static const char sTableViewXson[] = "{ \"xui\": 1, \"tokens\": { \"spacing\": { \"head\": 24, \"row\": 20 }, \"colors\": { \"bg\": \"#01020304\", \"head\": \"#11121314\", \"headText\": \"#21222324\", \"row\": \"#31323334\", \"sel\": \"#41424344\", \"grid\": \"#51525354\", \"text\": \"#61626364\", \"bar\": \"#71727374\", \"thumb\": \"#81828384\" } }, \"styles\": { \"table\": { \"font\": \"@fonts.body\", \"headerHeight\": \"@spacing.head\", \"rowHeight\": \"@spacing.row\", \"backgroundColor\": \"@colors.bg\", \"headerColor\": \"@colors.head\", \"headerTextColor\": \"@colors.headText\", \"rowColor\": \"@colors.row\", \"selectedColor\": \"@colors.sel\", \"gridColor\": \"@colors.grid\", \"textColor\": \"@colors.text\", \"barColor\": \"@colors.bar\", \"thumbColor\": \"@colors.thumb\" } }, \"tree\": { \"type\": \"tableView\", \"id\": \"table\", \"style\": \"table\", \"width\": 260, \"height\": 120, \"selected\": 2, \"columns\": [ { \"id\": 10, \"title\": \"Name\", \"width\": 120, \"minWidth\": 60, \"align\": \"left\" }, { \"id\": 20, \"title\": \"Value\", \"width\": 70, \"minWidth\": 40, \"align\": \"right\" }, { \"id\": 30, \"title\": \"State\", \"width\": 80, \"minWidth\": 50, \"align\": \"center\" } ], \"rows\": [ [ \"Item 0\", 0, \"Ready\" ], [ \"Item 1\", 7, \"Idle\" ], [ \"Item 2\", 14, \"Ready\" ], [ \"Item 3\", 21, \"Idle\" ] ] } }";
	static const char sTableViewSelectXson[] = "{ \"xui\": 1, \"tree\": { \"type\": \"tableView\", \"id\": \"table\", \"columns\": [\"Name\"], \"rows\": [], \"onSelect\": \"changed\" } }";
	static const char sTableViewSortXson[] = "{ \"xui\": 1, \"tree\": { \"type\": \"tableView\", \"id\": \"table\", \"columns\": [\"Name\"], \"rows\": [], \"onSort\": \"changed\" } }";
	static const char sPropertyGridXson[] = "{ \"xui\": 1, \"tokens\": { \"spacing\": { \"row\": 20, \"name\": 110 }, \"colors\": { \"bg\": \"#01020304\", \"cat\": \"#11121314\", \"row\": \"#21222324\", \"sel\": \"#31323334\", \"grid\": \"#41424344\", \"text\": \"#51525354\", \"value\": \"#61626364\", \"readonly\": \"#71727374\", \"changed\": \"#81828384\", \"error\": \"#91929394\" } }, \"styles\": { \"props\": { \"font\": \"@fonts.body\", \"rowHeight\": \"@spacing.row\", \"nameWidth\": \"@spacing.name\", \"backgroundColor\": \"@colors.bg\", \"categoryColor\": \"@colors.cat\", \"rowColor\": \"@colors.row\", \"selectedColor\": \"@colors.sel\", \"gridColor\": \"@colors.grid\", \"textColor\": \"@colors.text\", \"valueColor\": \"@colors.value\", \"readonlyColor\": \"@colors.readonly\", \"changedColor\": \"@colors.changed\", \"errorColor\": \"@colors.error\" } }, \"tree\": { \"type\": \"propertyGrid\", \"id\": \"props\", \"style\": \"props\", \"width\": 220, \"height\": 120, \"selected\": 1, \"categories\": [ { \"name\": \"General\", \"expanded\": true, \"properties\": [ { \"name\": \"Name\", \"value\": \"Player\", \"editor\": \"text\", \"readonly\": true }, { \"name\": \"Count\", \"value\": \"12\", \"editor\": \"number\", \"changed\": true }, { \"name\": \"Enabled\", \"value\": \"true\", \"editor\": \"bool\", \"error\": true } ] }, { \"name\": \"Advanced\", \"expanded\": false, \"properties\": [ { \"name\": \"Mode\", \"value\": \"Auto\", \"editor\": \"enum\", \"items\": [\"Auto\", \"Manual\", \"Script\"] }, { \"name\": \"Tint\", \"value\": \"#168AC2\", \"editor\": \"color\" } ] } ] } }";
	static const char sPropertyGridSelectXson[] = "{ \"xui\": 1, \"tree\": { \"type\": \"propertyGrid\", \"id\": \"props\", \"categories\": [], \"onSelect\": \"changed\" } }";
	static const char sBreadcrumbXson[] = "{ \"xui\": 1, \"tokens\": { \"spacing\": { \"pad\": 8, \"sep\": 14 }, \"colors\": { \"bg\": \"#01020304\", \"seg\": \"#11121314\", \"sel\": \"#21222324\", \"border\": \"#31323334\", \"text\": \"#41424344\", \"sepColor\": \"#51525354\" } }, \"styles\": { \"crumb\": { \"font\": \"@fonts.body\", \"paddingX\": \"@spacing.pad\", \"separatorWidth\": \"@spacing.sep\", \"backgroundColor\": \"@colors.bg\", \"segmentColor\": \"@colors.seg\", \"selectedColor\": \"@colors.sel\", \"borderColor\": \"@colors.border\", \"textColor\": \"@colors.text\", \"separatorColor\": \"@colors.sepColor\" } }, \"tree\": { \"type\": \"breadcrumb\", \"id\": \"path\", \"style\": \"crumb\", \"width\": 210, \"height\": 28, \"selected\": 2, \"segments\": [ \"Home\", { \"text\": \"Project\", \"id\": 20 }, { \"text\": \"Assets\", \"id\": 30 } ] } }";
	static const char sBreadcrumbSelectXson[] = "{ \"xui\": 1, \"tree\": { \"type\": \"breadcrumb\", \"id\": \"path\", \"segments\": [\"Home\"], \"onSelect\": \"changed\" } }";
	static const char sAccordionXson[] = "{ \"xui\": 1, \"tokens\": { \"spacing\": { \"hh\": 24, \"gap\": 3, \"pad\": 6, \"ch\": 32 }, \"colors\": { \"bg\": \"#01020304\", \"head\": \"#11121314\", \"exp\": \"#21222324\", \"content\": \"#31323334\", \"border\": \"#41424344\", \"text\": \"#51525354\", \"ctext\": \"#61626364\" } }, \"styles\": { \"acc\": { \"font\": \"@fonts.body\", \"mode\": \"single\", \"headerHeight\": \"@spacing.hh\", \"spacing\": \"@spacing.gap\", \"contentPadding\": \"@spacing.pad\", \"backgroundColor\": \"@colors.bg\", \"headerColor\": \"@colors.head\", \"expandedColor\": \"@colors.exp\", \"contentColor\": \"@colors.content\", \"borderColor\": \"@colors.border\", \"textColor\": \"@colors.text\", \"contentTextColor\": \"@colors.ctext\" } }, \"tree\": { \"type\": \"accordion\", \"id\": \"settings\", \"style\": \"acc\", \"width\": 200, \"height\": 120, \"selected\": 1, \"sections\": [ { \"title\": \"General\", \"text\": \"Alpha\", \"contentHeight\": \"@spacing.ch\", \"expanded\": true, \"id\": 10 }, { \"title\": \"Advanced\", \"text\": \"Beta\", \"contentHeight\": 36, \"expanded\": true, \"id\": 20 }, \"About\" ] } }";
	static const char sAccordionSelectXson[] = "{ \"xui\": 1, \"tree\": { \"type\": \"accordion\", \"id\": \"settings\", \"sections\": [\"General\"], \"onSelect\": \"changed\" } }";
	static const char sToastXson[] = "{ \"xui\": 1, \"tokens\": { \"spacing\": { \"tw\": 180, \"th\": 46, \"gap\": 5, \"dur\": 9 }, \"colors\": { \"bg\": \"#01020304\", \"border\": \"#11121314\", \"text\": \"#21222324\", \"muted\": \"#31323334\", \"info\": \"#41424344\", \"success\": \"#51525354\", \"warning\": \"#61626364\", \"error\": \"#71727374\", \"close\": \"#81828384\", \"closeHover\": \"#91929394\" } }, \"styles\": { \"toastStyle\": { \"font\": \"@fonts.body\", \"placement\": \"bottomCenter\", \"toastWidth\": \"@spacing.tw\", \"toastHeight\": \"@spacing.th\", \"spacing\": \"@spacing.gap\", \"backgroundColor\": \"@colors.bg\", \"borderColor\": \"@colors.border\", \"textColor\": \"@colors.text\", \"mutedTextColor\": \"@colors.muted\", \"infoColor\": \"@colors.info\", \"successColor\": \"@colors.success\", \"warningColor\": \"@colors.warning\", \"errorColor\": \"@colors.error\", \"closeColor\": \"@colors.close\", \"closeHoverColor\": \"@colors.closeHover\" } }, \"tree\": { \"type\": \"toast\", \"id\": \"notify\", \"style\": \"toastStyle\", \"width\": 320, \"height\": 160, \"items\": [ { \"type\": \"info\", \"title\": \"Info\", \"message\": \"Queued\", \"duration\": \"@spacing.dur\" }, { \"type\": \"success\", \"title\": \"Saved\", \"text\": \"Done\", \"duration\": 8 }, \"Plain\" ] } }";
	static const char sToastCloseXson[] = "{ \"xui\": 1, \"tree\": { \"type\": \"toast\", \"id\": \"notify\", \"items\": [\"Info\"], \"onClose\": \"changed\" } }";
	static const char sVirtualListXson[] = "{ \"xui\": 1, \"tokens\": { \"spacing\": { \"rows\": 10, \"itemH\": 20, \"sy\": 20 } }, \"templates\": { \"rowItem\": { \"type\": \"row\", \"id\": \"row-template\", \"height\": \"@spacing.itemH\", \"children\": [ { \"type\": \"label\", \"name\": \"item-label\", \"text\": \"Item\" } ] } }, \"tree\": { \"type\": \"virtualList\", \"id\": \"virtual-root\", \"width\": 100, \"height\": 80, \"padding\": 5, \"itemCount\": \"@spacing.rows\", \"itemHeight\": \"@spacing.itemH\", \"scrollY\": \"@spacing.sy\", \"backgroundColor\": \"#01020304\", \"barColor\": \"#11121314\", \"thumbColor\": \"#21222324\", \"itemTemplate\": \"rowItem\" } }";
	static const char sModelXson[] = "{ \"xui\": 1, \"tree\": { \"type\": \"column\", \"id\": \"model-root\", \"children\": [ { \"type\": \"label\", \"id\": \"model-title\", \"text\": \"${player.name}\" }, { \"type\": \"input\", \"id\": \"model-field\", \"value\": \"${player.name}\" }, { \"type\": \"image\", \"id\": \"model-icon\", \"src\": \"${icon.path}\" } ] } }";
	static const char sModelMissingXson[] = "{ \"xui\": 1, \"tree\": { \"type\": \"label\", \"id\": \"missing-model\", \"text\": \"${missing.key}\" } }";
	static const char sMissingClickXson[] = "{ \"xui\": 1, \"tree\": { \"id\": \"root\", \"onClick\": \"missing\" } }";
	static const char sScriptXson[] = "{ \"xui\": 1, \"tree\": { \"id\": \"root\", \"script\": \"return 1\" } }";
	static const char sUnknownTypeXson[] = "{ \"xui\": 1, \"tree\": { \"type\": \"unknownControl\", \"id\": \"root\" } }";
	static const char sBadTypeXson[] = "{ \"xui\": 1, \"tree\": { \"type\": 7, \"id\": \"root\" } }";
	static const char sControlXson[] = "{ \"xui\": 1, \"tokens\": { \"colors\": { \"text\": \"#010203\", \"button\": \"#203040\" }, \"spacing\": { \"rule\": 3 } }, \"styles\": { \"title\": { \"font\": \"@fonts.body\", \"textColor\": \"@colors.text\", \"textAlign\": \"center\", \"textVAlign\": \"middle\" }, \"action\": { \"font\": \"@fonts.body\", \"textColor\": \"@colors.text\", \"color\": \"@colors.button\", \"hoverColor\": \"#304050\", \"activeColor\": \"#405060\", \"focusColor\": \"#506070\", \"disabledColor\": \"#607080\", \"textAlign\": \"right\", \"textVAlign\": \"bottom\" }, \"icon\": { \"texture\": \"@textures.icon\", \"source\": [1, 2, 3, 4], \"color\": \"#AABBCCDD\", \"mode\": \"fit\" }, \"field\": { \"font\": \"@fonts.body\", \"textColor\": \"#111213\", \"background\": \"#212223\", \"focusColor\": \"#313233\", \"cursorColor\": \"#414243\", \"placeholderColor\": \"#515253\", \"selectionColor\": \"#616263\", \"disabledTextColor\": \"#717273\", \"disabledBackgroundColor\": \"#818283\", \"selection\": [1, 2] }, \"rule\": { \"orientation\": \"vertical\", \"thickness\": \"@spacing.rule\", \"color\": \"#112233\" } }, \"tree\": { \"type\": \"column\", \"id\": \"root\", \"children\": [ { \"type\": \"label\", \"id\": \"title\", \"style\": \"title\", \"text\": \"Hello\" }, { \"type\": \"button\", \"id\": \"action\", \"style\": \"action\", \"text\": \"Run\", \"onClick\": \"ok\" }, { \"type\": \"image\", \"id\": \"icon\", \"style\": \"icon\" }, { \"type\": \"input\", \"id\": \"field\", \"style\": \"field\", \"value\": \"abc\", \"placeholder\": \"Name\", \"password\": true, \"readonly\": true, \"disabled\": false }, { \"type\": \"separator\", \"id\": \"rule\", \"style\": \"rule\" } ] } }";
	static const char sInputChangeXson[] = "{ \"xui\": 1, \"tree\": { \"type\": \"input\", \"id\": \"field\", \"onChange\": \"changed\" } }";
	static const char sNumericInputXson[] = "{ \"xui\": 1, \"tokens\": { \"spacing\": { \"lo\": -5, \"hi\": 8, \"step\": 2 }, \"colors\": { \"text\": \"#010203\", \"bg\": \"#F4FAFF\" } }, \"styles\": { \"num\": { \"font\": \"@fonts.body\", \"textColor\": \"@colors.text\", \"backgroundColor\": \"@colors.bg\", \"focusColor\": \"#B8DFF5\", \"cursorColor\": \"#0F6EA8\" } }, \"tree\": { \"type\": \"numericInput\", \"id\": \"num\", \"style\": \"num\", \"min\": \"@spacing.lo\", \"max\": \"@spacing.hi\", \"step\": \"@spacing.step\", \"integer\": true, \"spinner\": false, \"value\": 6, \"placeholder\": \"Qty\" } }";
	static const char sNumericInputChangeXson[] = "{ \"xui\": 1, \"tree\": { \"type\": \"numericInput\", \"id\": \"num\", \"onChange\": \"changed\" } }";
	static const char sToggleXson[] = "{ \"xui\": 1, \"tokens\": { \"colors\": { \"text\": \"#010203\", \"on\": \"#168AC2\", \"box\": \"#D8ECF8\" } }, \"styles\": { \"toggle\": { \"font\": \"@fonts.body\", \"textColor\": \"@colors.text\", \"checkedColor\": \"@colors.on\", \"focusColor\": \"#B8DFF5\" } }, \"tree\": { \"type\": \"column\", \"id\": \"toggles\", \"children\": [ { \"type\": \"checkbox\", \"id\": \"agree\", \"style\": \"toggle\", \"text\": \"Agree\", \"checked\": true, \"boxColor\": \"@colors.box\" }, { \"type\": \"radio\", \"id\": \"choice\", \"style\": \"toggle\", \"text\": \"Choice\", \"value\": 7, \"checked\": true, \"ringColor\": \"@colors.box\" }, { \"type\": \"switch\", \"id\": \"wifi\", \"style\": \"toggle\", \"text\": \"WiFi\", \"checked\": false, \"trackColor\": \"@colors.box\", \"knobColor\": \"#FFFFFF\" } ] } }";
	static const char sToggleChangeXson[] = "{ \"xui\": 1, \"tree\": { \"type\": \"checkbox\", \"id\": \"agree\", \"onChange\": \"changed\" } }";
	static const char sRangeXson[] = "{ \"xui\": 1, \"tokens\": { \"spacing\": { \"min\": 0, \"max\": 100, \"value\": 25 }, \"colors\": { \"track\": \"#D8ECF8\", \"fill\": \"#168AC2\", \"knob\": \"#FFFFFF\", \"text\": \"#010203\" } }, \"styles\": { \"range\": { \"trackColor\": \"@colors.track\", \"fillColor\": \"@colors.fill\", \"knobColor\": \"@colors.knob\", \"focusColor\": \"#B8DFF5\", \"disabledColor\": \"#EEF6FB\" } }, \"tree\": { \"type\": \"column\", \"id\": \"ranges\", \"children\": [ { \"type\": \"slider\", \"id\": \"volume\", \"style\": \"range\", \"min\": \"@spacing.min\", \"max\": \"@spacing.max\", \"value\": \"@spacing.value\" }, { \"type\": \"progress\", \"id\": \"load\", \"style\": \"range\", \"min\": 0, \"max\": 10, \"value\": 6, \"font\": \"@fonts.body\", \"text\": \"60%\", \"textColor\": \"@colors.text\" } ] } }";
	static const char sSliderChangeXson[] = "{ \"xui\": 1, \"tree\": { \"type\": \"slider\", \"id\": \"volume\", \"onChange\": \"changed\" } }";
	static const char sTabsXson[] = "{ \"xui\": 1, \"tokens\": { \"spacing\": { \"tabW\": 54, \"tabH\": 22 }, \"colors\": { \"bg\": \"#F4FAFF\", \"tab\": \"#EAF6FD\", \"hover\": \"#D8ECF8\", \"active\": \"#C7E8F8\", \"text\": \"#010203\", \"activeText\": \"#0F6EA8\" } }, \"styles\": { \"tabs\": { \"font\": \"@fonts.body\", \"backgroundColor\": \"@colors.bg\", \"tabColor\": \"@colors.tab\", \"hoverColor\": \"@colors.hover\", \"activeColor\": \"@colors.active\", \"focusColor\": \"#B8DFF5\", \"disabledColor\": \"#EEF6FB\", \"textColor\": \"@colors.text\", \"activeTextColor\": \"@colors.activeText\" } }, \"tree\": { \"type\": \"tabs\", \"id\": \"main-tabs\", \"style\": \"tabs\", \"items\": [\"Home\", \"Edit\", \"View\"], \"selected\": 1, \"tabWidth\": \"@spacing.tabW\", \"tabHeight\": \"@spacing.tabH\" } }";
	static const char sTabsSelectXson[] = "{ \"xui\": 1, \"tree\": { \"type\": \"tabs\", \"id\": \"main-tabs\", \"items\": [\"A\"], \"onSelect\": \"changed\" } }";
	static const char sToolbarXson[] = "{ \"xui\": 1, \"tokens\": { \"spacing\": { \"iw\": 50, \"ih\": 24, \"sep\": 8 }, \"colors\": { \"bg\": \"#F4FAFF\", \"item\": \"#EAF6FD\", \"hover\": \"#D8ECF8\", \"active\": \"#C7E8F8\", \"checked\": \"#B8DFF5\", \"text\": \"#010203\", \"disabled\": \"#8090A0\" } }, \"styles\": { \"bar\": { \"font\": \"@fonts.body\", \"backgroundColor\": \"@colors.bg\", \"itemColor\": \"@colors.item\", \"hoverColor\": \"@colors.hover\", \"activeColor\": \"@colors.active\", \"checkedColor\": \"@colors.checked\", \"focusColor\": \"#7FC4E5\", \"disabledColor\": \"#EEF6FB\", \"separatorColor\": \"#7FC4E5\", \"textColor\": \"@colors.text\", \"disabledTextColor\": \"@colors.disabled\" } }, \"tree\": { \"type\": \"toolbar\", \"id\": \"main-toolbar\", \"style\": \"bar\", \"items\": [ { \"text\": \"New\", \"type\": \"button\" }, { \"text\": \"Pin\", \"type\": \"toggle\", \"checked\": true }, { \"type\": \"separator\" }, { \"text\": \"Run\", \"enabled\": false } ], \"itemWidth\": \"@spacing.iw\", \"itemHeight\": \"@spacing.ih\", \"separatorSize\": \"@spacing.sep\" } }";
	static const char sToolbarSelectXson[] = "{ \"xui\": 1, \"tree\": { \"type\": \"toolbar\", \"id\": \"main-toolbar\", \"items\": [\"A\"], \"onSelect\": \"changed\" } }";
	static const char sStatusBarXson[] = "{ \"xui\": 1, \"tokens\": { \"spacing\": { \"h\": 24, \"gap\": 4, \"pad\": 6, \"progressW\": 80 }, \"colors\": { \"bg\": \"#F4FAFF\", \"border\": \"#7FC4E5\", \"item\": \"#EAF6FD\", \"hover\": \"#D8ECF8\", \"active\": \"#C7E8F8\", \"text\": \"#010203\", \"disabled\": \"#8090A0\", \"track\": \"#D8ECF8\", \"fill\": \"#168AC2\" } }, \"styles\": { \"bar\": { \"font\": \"@fonts.body\", \"barHeight\": \"@spacing.h\", \"itemGap\": \"@spacing.gap\", \"itemPadding\": \"@spacing.pad\", \"backgroundColor\": \"@colors.bg\", \"borderColor\": \"@colors.border\", \"itemColor\": \"@colors.item\", \"hoverColor\": \"@colors.hover\", \"activeColor\": \"@colors.active\", \"textColor\": \"@colors.text\", \"disabledTextColor\": \"@colors.disabled\", \"progressTrackColor\": \"@colors.track\", \"progressFillColor\": \"@colors.fill\" } }, \"tree\": { \"type\": \"statusBar\", \"id\": \"main-status\", \"style\": \"bar\", \"items\": [ { \"text\": \"Ready\", \"section\": \"left\", \"width\": 70, \"clickable\": true }, { \"type\": \"progress\", \"section\": \"left\", \"min\": 0, \"max\": 100, \"value\": 45, \"width\": \"@spacing.progressW\" }, { \"text\": \"Ln 12\", \"section\": \"center\", \"width\": 72 }, { \"text\": \"UTF-8\", \"section\": \"right\", \"width\": 64, \"clickable\": true, \"enabled\": false } ] } }";
	static const char sStatusBarSelectXson[] = "{ \"xui\": 1, \"tree\": { \"type\": \"statusBar\", \"id\": \"main-status\", \"items\": [\"Ready\"], \"onSelect\": \"changed\" } }";
	static const char sComboBoxXson[] = "{ \"xui\": 1, \"tokens\": { \"spacing\": { \"drop\": 72 }, \"colors\": { \"bg\": \"#F4FAFF\", \"hover\": \"#D8ECF8\", \"active\": \"#C7E8F8\", \"text\": \"#010203\", \"popup\": \"#FFFFFF\" } }, \"styles\": { \"combo\": { \"font\": \"@fonts.body\", \"background\": \"@colors.bg\", \"hoverColor\": \"@colors.hover\", \"activeColor\": \"@colors.active\", \"focusColor\": \"#B8DFF5\", \"disabledColor\": \"#EEF6FB\", \"textColor\": \"@colors.text\", \"popupColor\": \"@colors.popup\" } }, \"tree\": { \"type\": \"comboBox\", \"id\": \"mode\", \"style\": \"combo\", \"items\": [\"One\", \"Two\", \"Three\"], \"selected\": 1, \"dropDownHeight\": \"@spacing.drop\" } }";
	static const char sComboBoxSelectXson[] = "{ \"xui\": 1, \"tree\": { \"type\": \"comboBox\", \"id\": \"mode\", \"items\": [\"A\"], \"onSelect\": \"changed\" } }";
	static const char sPopupXson[] = "{ \"xui\": 1, \"tokens\": { \"colors\": { \"pop\": \"#F4FAFF\" } }, \"tree\": { \"type\": \"column\", \"id\": \"popup-root\", \"children\": [ { \"type\": \"panel\", \"id\": \"owner\", \"width\": 80, \"height\": 24 }, { \"type\": \"popup\", \"id\": \"popup\", \"owner\": \"owner\", \"width\": 100, \"height\": 70, \"backgroundColor\": \"@colors.pop\", \"open\": true, \"closeOnOutside\": false, \"closeOnEscape\": true } ] } }";
	static const char sPopupCloseXson[] = "{ \"xui\": 1, \"tree\": { \"type\": \"popup\", \"id\": \"popup\", \"onClose\": \"changed\" } }";
	static const char sTooltipXson[] = "{ \"xui\": 1, \"tokens\": { \"spacing\": { \"ox\": 5, \"oy\": 3 }, \"colors\": { \"tip\": \"#040506\", \"text\": \"#070809\" } }, \"styles\": { \"tip\": { \"font\": \"@fonts.body\", \"backgroundColor\": \"@colors.tip\", \"textColor\": \"@colors.text\", \"offsetX\": \"@spacing.ox\", \"offsetY\": \"@spacing.oy\" } }, \"tree\": { \"type\": \"column\", \"id\": \"tip-root\", \"children\": [ { \"type\": \"panel\", \"id\": \"tip-owner\", \"width\": 80, \"height\": 24 }, { \"type\": \"tooltip\", \"id\": \"tip\", \"owner\": \"tip-owner\", \"style\": \"tip\", \"text\": \"Tip\", \"open\": true } ] } }";
	static const char sTooltipCloseXson[] = "{ \"xui\": 1, \"tree\": { \"type\": \"column\", \"id\": \"tip-root\", \"children\": [ { \"type\": \"panel\", \"id\": \"tip-owner\" }, { \"type\": \"tooltip\", \"id\": \"tip\", \"owner\": \"tip-owner\", \"text\": \"Tip\", \"onClose\": \"changed\" } ] } }";
	static const char sMenuXson[] = "{ \"xui\": 1, \"tokens\": { \"spacing\": { \"mw\": 120, \"mh\": 80, \"ih\": 20, \"mx\": 10, \"my\": 12 }, \"colors\": { \"bg\": \"#F4FAFF\", \"row\": \"#EAF6FD\", \"sel\": \"#C7E8F8\", \"text\": \"#010203\", \"disabled\": \"#8090A0\" } }, \"styles\": { \"menu\": { \"font\": \"@fonts.body\", \"menuWidth\": \"@spacing.mw\", \"maxHeight\": \"@spacing.mh\", \"itemHeight\": \"@spacing.ih\", \"backgroundColor\": \"@colors.bg\", \"rowColor\": \"@colors.row\", \"selectedColor\": \"@colors.sel\", \"textColor\": \"@colors.text\", \"disabledTextColor\": \"@colors.disabled\" } }, \"tree\": { \"type\": \"column\", \"id\": \"menu-root\", \"children\": [ { \"type\": \"panel\", \"id\": \"menu-owner\", \"width\": 80, \"height\": 24 }, { \"type\": \"menu\", \"id\": \"file-menu\", \"owner\": \"menu-owner\", \"style\": \"menu\", \"items\": [\"Open\", \"Save\", \"Exit\"], \"enabledItems\": [true, false, true], \"open\": true, \"x\": \"@spacing.mx\", \"y\": \"@spacing.my\" } ] } }";
	static const char sMenuSelectXson[] = "{ \"xui\": 1, \"tree\": { \"type\": \"column\", \"id\": \"menu-root\", \"children\": [ { \"type\": \"panel\", \"id\": \"menu-owner\" }, { \"type\": \"menu\", \"id\": \"file-menu\", \"owner\": \"menu-owner\", \"items\": [\"Open\"], \"onSelect\": \"changed\" } ] } }";
	static const char sDialogXson[] = "{ \"xui\": 1, \"tokens\": { \"colors\": { \"back\": \"#01020378\", \"bg\": \"#F4FAFF\", \"title\": \"#070809\", \"close\": \"#0A0B0C\" } }, \"styles\": { \"dlg\": { \"font\": \"@fonts.body\", \"backdropColor\": \"@colors.back\", \"backgroundColor\": \"@colors.bg\", \"titleColor\": \"@colors.title\", \"closeColor\": \"@colors.close\" } }, \"tree\": { \"type\": \"dialog\", \"id\": \"dialog\", \"style\": \"dlg\", \"title\": \"Settings\", \"width\": 160, \"height\": 90, \"open\": true, \"modal\": false, \"closeOnEscape\": true, \"showClose\": true } }";
	static const char sDialogCloseXson[] = "{ \"xui\": 1, \"tree\": { \"type\": \"dialog\", \"id\": \"dialog\", \"onClose\": \"changed\" } }";
	static const char sMessageBoxXson[] = "{ \"xui\": 1, \"tokens\": { \"colors\": { \"back\": \"#01020378\", \"bg\": \"#F4FAFF\", \"title\": \"#070809\", \"close\": \"#0A0B0C\", \"msg\": \"#111213\", \"btn\": \"#EAF6FD\", \"hover\": \"#C7E8F8\", \"btnText\": \"#010203\" } }, \"styles\": { \"msg\": { \"font\": \"@fonts.body\", \"backdropColor\": \"@colors.back\", \"backgroundColor\": \"@colors.bg\", \"titleColor\": \"@colors.title\", \"closeColor\": \"@colors.close\", \"messageColor\": \"@colors.msg\", \"buttonColor\": \"@colors.btn\", \"buttonHoverColor\": \"@colors.hover\", \"buttonTextColor\": \"@colors.btnText\" } }, \"tree\": { \"type\": \"messageBox\", \"id\": \"msg\", \"style\": \"msg\", \"title\": \"Confirm\", \"message\": \"Save changes?\", \"kind\": \"question\", \"buttons\": \"yesNoCancel\", \"width\": 220, \"height\": 120, \"open\": true } }";
	static const char sMessageBoxResultXson[] = "{ \"xui\": 1, \"tree\": { \"type\": \"messageBox\", \"id\": \"msg\", \"message\": \"Save?\", \"onResult\": \"changed\" } }";

	memset(&tXui, 0, sizeof(tXui));
	memset(&tPage, 0, sizeof(tPage));
	xgeXuiModelInit(&tModel);
	memset(&tFont, 0, sizeof(tFont));
	memset(&tTexture, 0, sizeof(tTexture));
	memset(&tEvent, 0, sizeof(tEvent));
	memset(&tRect, 0, sizeof(tRect));
	if ( xgeXuiInit(&tXui) != XGE_OK ) {
		return 500;
	}
	tTexture.iWidth = 16;
	tTexture.iHeight = 12;
	if ( xgeXuiTokenSetColor(&tXui, "ctxPanel", XGE_COLOR_RGBA(9, 8, 7, 255)) != XGE_OK || xgeXuiTokenSetSpacing(&tXui, "ctxWidth", 77.0f) != XGE_OK || xgeXuiTokenSetFont(&tXui, "body", &tFont) != XGE_OK || xgeXuiTokenSetTexture(&tXui, "icon", &tTexture) != XGE_OK || xgeXuiGetThemeVersion(&tXui) != 5 ) {
		xgeXuiUnit(&tXui);
		return 527;
	}
	xgeXuiBinderInit(&tBinder);
	if ( tBinder.iClickCount != 0 || xgeXuiBinderSetClick(&tBinder, "ok", __testXuiButtonClick, NULL) != XGE_OK || tBinder.iClickCount != 1 ) {
		xgeXuiUnit(&tXui);
		return 501;
	}
	if ( xgeXuiBinderSetClick(&tBinder, "ok", __testXuiButtonClick, &tBinder) != XGE_OK || tBinder.iClickCount != 1 || tBinder.arrClick[0].pUser != &tBinder ) {
		xgeXuiUnit(&tXui);
		return 502;
	}
	if ( xgeXuiPageLoadMemory(NULL, sXson, (int)strlen(sXson), &tBinder, &tPage) != XGE_ERROR_INVALID_ARGUMENT ) {
		xgeXuiUnit(&tXui);
		return 503;
	}
	if ( xgeXuiPageLoadMemory(&tXui, sContextTokenXson, (int)strlen(sContextTokenXson), &tBinder, &tPage) != XGE_OK ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 528;
	}
	pRoot = xgeXuiPageRoot(&tPage);
	if ( pRoot == NULL || pRoot->tStyle.tWidth.fValue != 77.0f || pRoot->tStyle.iBackgroundColor != XGE_COLOR_RGBA(9, 8, 7, 255) || tPage.iThemeVersion != xgeXuiGetThemeVersion(&tXui) ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 529;
	}
	if ( xgeXuiTokenSetColor(&tXui, "ctxPanel", XGE_COLOR_RGBA(10, 11, 12, 255)) != XGE_OK || xgeXuiTokenSetSpacing(&tXui, "ctxWidth", 88.0f) != XGE_OK || xgeXuiPageSyncStyle(&tPage) != XGE_OK ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 554;
	}
	if ( xgeXuiPageStyleVersion(&tPage) != 2 || tPage.iThemeVersion != xgeXuiGetThemeVersion(&tXui) || pRoot->tStyle.tWidth.fValue != 88.0f || pRoot->tStyle.iBackgroundColor != XGE_COLOR_RGBA(10, 11, 12, 255) || ((xgeXuiWidgetGetFlags(pRoot) & XGE_XUI_WIDGET_DIRTY_STYLE) != 0) || ((xgeXuiWidgetGetFlags(pRoot) & XGE_XUI_WIDGET_DIRTY_LAYOUT) == 0) || ((xgeXuiWidgetGetFlags(pRoot) & XGE_XUI_WIDGET_DIRTY_PAINT) == 0) ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 555;
	}
	xgeXuiPageUnload(&tPage);
	if ( xgeXuiPageLoadMemory(&tXui, sXson, (int)strlen(sXson), &tBinder, &tPage) != XGE_OK || xgeXuiPageRoot(&tPage) == NULL || xgeXuiPageFind(&tPage, "root") != xgeXuiPageRoot(&tPage) || xgeXuiPageFind(&tPage, "child") == NULL ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 505;
	}
	if ( tPage.iIndexCount < 3 || tPage.bIndexOverflow != 0 || xgeXuiPageStyleVersion(&tPage) != 1 ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 504;
	}
	pRoot = xgeXuiPageRoot(&tPage);
	pChild = xgeXuiPageFind(&tPage, "child");
	pGrid = xgeXuiPageFind(&tPage, "grid-child");
	if ( pRoot->tStyle.iLayout != XGE_XUI_LAYOUT_ROW || pRoot->tStyle.tWidth.iUnit != XGE_XUI_SIZE_PX || pRoot->tStyle.tWidth.fValue != 120.0f || pRoot->tStyle.tHeight.iUnit != XGE_XUI_SIZE_PX || pRoot->tStyle.tHeight.fValue != 200.0f || pRoot->tStyle.fGap != 8.0f || pRoot->tStyle.iAlignX != XGE_XUI_ALIGN_STRETCH || pRoot->tStyle.iBackgroundColor != XGE_COLOR_RGBA(0x11, 0x22, 0x33, 0xFF) || pRoot->tStyle.tMargin.tLeft.fValue != 4.0f || pRoot->tStyle.tPadding.tLeft.fValue != 2.0f || pRoot->tStyle.tPadding.tTop.fValue != 3.0f ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 511;
	}
	if ( pRoot->iStyleVersion != xgeXuiPageStyleVersion(&tPage) || pChild->iStyleVersion != xgeXuiPageStyleVersion(&tPage) || ((xgeXuiWidgetGetFlags(pRoot) & XGE_XUI_WIDGET_DIRTY_STYLE) != 0) ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 524;
	}
	g_iXuiButtonClicks = 0;
	g_pXuiLastClickWidget = NULL;
	g_pXuiLastClickUser = NULL;
	tEvent.iType = XGE_EVENT_MOUSE_UP;
	tEvent.iParam1 = XGE_MOUSE_LEFT;
	if ( pRoot->procEvent == NULL || pRoot->procEvent(pRoot, &tEvent, pRoot->pUser) != XGE_XUI_EVENT_CONSUMED || g_iXuiButtonClicks != 1 || g_pXuiLastClickWidget != pRoot || g_pXuiLastClickUser != &tBinder ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 530;
	}
	if ( pChild->tStyle.tWidth.iUnit != XGE_XUI_SIZE_PERCENT || pChild->tStyle.tWidth.fValue != 50.0f || pChild->tStyle.tHeight.iUnit != XGE_XUI_SIZE_PX || pChild->tStyle.tHeight.fValue != 32.0f || pChild->tStyle.tMargin.tLeft.fValue != 1.0f || pChild->tStyle.tMargin.tBottom.fValue != 4.0f || pChild->tStyle.iAnchor != (XGE_XUI_ANCHOR_LEFT | XGE_XUI_ANCHOR_RIGHT) || pChild->tStyle.tAnchor.tRight.iUnit != XGE_XUI_SIZE_PERCENT || (xgeXuiWidgetGetFlags(pChild) & XGE_XUI_WIDGET_CLIP) == 0 ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 512;
	}
	if ( pGrid == NULL || pGrid->tStyle.iGridColumns != 3 || pGrid->tStyle.fGridRowHeight != 12.0f || pGrid->tStyle.fGridColumnGap != 5.0f || pGrid->tStyle.fGridRowGap != 6.0f || pGrid->tStyle.iGridColumnSpan != 2 || pGrid->tStyle.tWidth.iUnit != XGE_XUI_SIZE_GROW || pGrid->tStyle.tWidth.fValue != 2.0f ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 519;
	}
	xgeXuiPageUnload(&tPage);
	if ( xgeXuiPageLoadMemory(&tXui, sDockXson, (int)strlen(sDockXson), &tBinder, &tPage) != XGE_OK ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 556;
	}
	pRoot = xgeXuiPageRoot(&tPage);
	pChild = xgeXuiPageFind(&tPage, "dock-top");
	pGrid = xgeXuiPageFind(&tPage, "dock-left");
	if ( pRoot == NULL || pRoot->tStyle.iLayout != XGE_XUI_LAYOUT_DOCK || pChild == NULL || xgeXuiWidgetGetDock(pChild) != XGE_XUI_DOCK_TOP || pGrid == NULL || xgeXuiWidgetGetDock(pGrid) != XGE_XUI_DOCK_LEFT ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 557;
	}
	tRect.fX = 0.0f;
	tRect.fY = 0.0f;
	tRect.fW = 300.0f;
	tRect.fH = 200.0f;
	xgeXuiWidgetSetRect(pRoot, tRect);
	xgeXuiUpdate(&tXui, 0.0f);
	if ( xgeXuiWidgetGetRect(pChild).fW != 280.0f || xgeXuiWidgetGetRect(pGrid).fH != 160.0f || xgeXuiWidgetGetRect(xgeXuiPageFind(&tPage, "dock-fill")).fW != 240.0f ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 558;
	}
	xgeXuiPageUnload(&tPage);
	xgeXuiSetSafeAreaPx(&tXui, 1.0f, 2.0f, 3.0f, 4.0f);
	if ( xgeXuiPageLoadMemory(&tXui, sSafeAreaXson, (int)strlen(sSafeAreaXson), &tBinder, &tPage) != XGE_OK ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 559;
	}
	pRoot = xgeXuiRoot(&tXui);
	if ( pRoot == NULL || pRoot->tStyle.tPadding.tLeft.fValue != 11.0f || pRoot->tStyle.tPadding.tTop.fValue != 22.0f || pRoot->tStyle.tPadding.tRight.fValue != 11.0f || pRoot->tStyle.tPadding.tBottom.fValue != 22.0f ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 560;
	}
	xgeXuiPageUnload(&tPage);
	if ( pRoot->tStyle.tPadding.tLeft.fValue != 1.0f || pRoot->tStyle.tPadding.tTop.fValue != 2.0f || pRoot->tStyle.tPadding.tRight.fValue != 3.0f || pRoot->tStyle.tPadding.tBottom.fValue != 4.0f ) {
		xgeXuiUnit(&tXui);
		return 561;
	}
	xgeXuiSetSafeAreaPx(&tXui, 0.0f, 0.0f, 0.0f, 0.0f);
	if ( xgeXuiPageLoadMemory(&tXui, sScrollViewXson, (int)strlen(sScrollViewXson), &tBinder, &tPage) != XGE_OK ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 562;
	}
	pRoot = xgeXuiPageRoot(&tPage);
	pChild = xgeXuiPageFind(&tPage, "scroll-child");
	if ( pRoot == NULL || pChild == NULL || pRoot->procEvent != xgeXuiScrollViewEventProc || pRoot->procPaint != xgeXuiScrollViewPaintProc || tPage.iScrollViewCount != 1 ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 563;
	}
	pPageScroll = (xge_xui_scroll_view)pRoot->pUser;
	if ( pPageScroll != &tPage.arrScrollView[0] || pPageScroll->fContentW != 160.0f || pPageScroll->fContentH != 170.0f || pPageScroll->fScrollX != 10.0f || pPageScroll->fScrollY != 25.0f || pPageScroll->iBackgroundColor != 0x01020304u || pPageScroll->iBarColor != 0x11121314u || pPageScroll->iThumbColor != 0x21222324u || (xgeXuiWidgetGetFlags(pRoot) & XGE_XUI_WIDGET_CLIP) == 0 ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 564;
	}
	tRect.fX = 0.0f;
	tRect.fY = 0.0f;
	tRect.fW = 100.0f;
	tRect.fH = 80.0f;
	xgeXuiWidgetSetRect(pRoot, tRect);
	xgeXuiUpdate(&tXui, 0.0f);
	tRect = xgeXuiWidgetGetRect(pChild);
	if ( tRect.fX != -5.0f || tRect.fY != 20.0f ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 565;
	}
	xgeXuiPageUnload(&tPage);
	if ( xgeXuiPageLoadMemory(&tXui, sTreeViewXson, (int)strlen(sTreeViewXson), &tBinder, &tPage) != XGE_OK ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 901;
	}
	pRoot = xgeXuiPageRoot(&tPage);
	if ( pRoot == NULL || pRoot->procEvent != xgeXuiTreeViewEventProc || pRoot->procPaint != xgeXuiTreeViewPaintProc || tPage.iTreeViewCount != 1 ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 902;
	}
	pPageTree = (xge_xui_tree_view)pRoot->pUser;
	if ( pPageTree != &tPage.arrTreeView[0] || pPageTree->pFont != &tFont || pPageTree->iNodeCount != 4 || xgeXuiTreeViewGetVisibleCount(pPageTree) != 4 || xgeXuiTreeViewGetSelected(pPageTree) != 30 || pPageTree->fItemHeight != 20.0f || pPageTree->fIndent != 14.0f ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 903;
	}
	if ( pPageTree->iBackgroundColor != 0x01020304u || pPageTree->iRowColor != 0x11121314u || pPageTree->iSelectedColor != 0x21222324u || pPageTree->iTextColor != XGE_COLOR_RGBA(0x31, 0x32, 0x33, 0xFF) || pPageTree->iBarColor != 0x41424344u || pPageTree->iThumbColor != 0x51525354u || pPageTree->iExpanderColor != 0x61626364u || pPageTree->iDisabledTextColor != 0x71727374u ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 904;
	}
	xgeXuiPageUnload(&tPage);
	if ( xgeXuiPageLoadMemory(&tXui, sTreeViewSelectXson, (int)strlen(sTreeViewSelectXson), &tBinder, &tPage) == XGE_OK ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 905;
	}
	sError = xgeXuiPageGetError(&tPage);
	if ( (strstr(sError, "onSelect is not supported yet") == NULL) || (strstr(sError, "onSelect") == NULL) ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 906;
	}
	xgeXuiPageUnload(&tPage);
	if ( xgeXuiPageLoadMemory(&tXui, sTableViewXson, (int)strlen(sTableViewXson), &tBinder, &tPage) != XGE_OK ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 1018;
	}
	pRoot = xgeXuiPageRoot(&tPage);
	if ( pRoot == NULL || pRoot->procEvent != xgeXuiTableViewEventProc || pRoot->procPaint != xgeXuiTableViewPaintProc || tPage.iTableViewCount != 1 ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 1019;
	}
	pPageTable = (xge_xui_table_view)pRoot->pUser;
	if ( pPageTable != &tPage.arrTableView[0] || pPageTable->pFont != &tFont || pPageTable->iColumnCount != 3 || xgeXuiTableViewGetRowCount(pPageTable) != 4 || xgeXuiTableViewGetSelected(pPageTable) != 2 || pPageTable->fHeaderHeight != 24.0f || pPageTable->fRowHeight != 20.0f ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 1020;
	}
	if ( pPageTable->arrColumns[0].iId != 10 || strcmp(pPageTable->arrColumns[0].sTitle, "Name") != 0 || pPageTable->arrColumns[1].iAlign != XGE_TEXT_ALIGN_RIGHT || pPageTable->arrColumns[2].iAlign != XGE_TEXT_ALIGN_CENTER || tPage.arrTableViewAdapter[0] == NULL || strcmp(tPage.arrTableViewAdapter[0]->arrCell[2][1], "14") != 0 ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 1021;
	}
	if ( pPageTable->iBackgroundColor != 0x01020304u || pPageTable->iHeaderColor != 0x11121314u || pPageTable->iHeaderTextColor != 0x21222324u || pPageTable->iRowColor != 0x31323334u || pPageTable->iSelectedColor != 0x41424344u || pPageTable->iGridColor != 0x51525354u || pPageTable->iTextColor != 0x61626364u || pPageTable->iBarColor != 0x71727374u || pPageTable->iThumbColor != 0x81828384u ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 1022;
	}
	xgeXuiPageUnload(&tPage);
	if ( xgeXuiPageLoadMemory(&tXui, sTableViewSelectXson, (int)strlen(sTableViewSelectXson), &tBinder, &tPage) == XGE_OK ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 1023;
	}
	sError = xgeXuiPageGetError(&tPage);
	if ( (strstr(sError, "onSelect is not supported yet") == NULL) || (strstr(sError, "onSelect") == NULL) ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 1024;
	}
	xgeXuiPageUnload(&tPage);
	if ( xgeXuiPageLoadMemory(&tXui, sTableViewSortXson, (int)strlen(sTableViewSortXson), &tBinder, &tPage) == XGE_OK ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 1025;
	}
	sError = xgeXuiPageGetError(&tPage);
	if ( (strstr(sError, "onSort is not supported yet") == NULL) || (strstr(sError, "onSort") == NULL) ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 1026;
	}
	xgeXuiPageUnload(&tPage);
	if ( xgeXuiPageLoadMemory(&tXui, sPropertyGridXson, (int)strlen(sPropertyGridXson), &tBinder, &tPage) != XGE_OK ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 1011;
	}
	pRoot = xgeXuiPageRoot(&tPage);
	if ( pRoot == NULL || pRoot->procEvent != xgeXuiPropertyGridEventProc || pRoot->procPaint != xgeXuiPropertyGridPaintProc || tPage.iPropertyGridCount != 1 ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 1012;
	}
	pPagePropertyGrid = (xge_xui_property_grid)pRoot->pUser;
	if ( pPagePropertyGrid != &tPage.arrPropertyGrid[0] || pPagePropertyGrid->pFont != &tFont || pPagePropertyGrid->iItemCount != 7 || xgeXuiPropertyGridGetVisibleCount(pPagePropertyGrid) != 5 || xgeXuiPropertyGridGetSelected(pPagePropertyGrid) != 1 || pPagePropertyGrid->fRowHeight != 20.0f || pPagePropertyGrid->fNameWidth != 110.0f ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 1013;
	}
	if ( strcmp(pPagePropertyGrid->arrItems[0].sName, "General") != 0 || pPagePropertyGrid->arrItems[1].bReadonly != 1 || pPagePropertyGrid->arrItems[2].bDefaultChanged != 1 || pPagePropertyGrid->arrItems[3].bError != 1 || pPagePropertyGrid->arrItems[5].iEditor != XGE_XUI_PROPERTY_GRID_EDITOR_ENUM || pPagePropertyGrid->arrItems[5].iEnumItemCount != 3 || pPagePropertyGrid->arrItems[6].iEditor != XGE_XUI_PROPERTY_GRID_EDITOR_COLOR ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 1014;
	}
	if ( pPagePropertyGrid->iBackgroundColor != 0x01020304u || pPagePropertyGrid->iCategoryColor != 0x11121314u || pPagePropertyGrid->iRowColor != 0x21222324u || pPagePropertyGrid->iSelectedColor != 0x31323334u || pPagePropertyGrid->iGridColor != 0x41424344u || pPagePropertyGrid->iTextColor != 0x51525354u || pPagePropertyGrid->iValueColor != 0x61626364u || pPagePropertyGrid->iReadonlyColor != 0x71727374u || pPagePropertyGrid->iChangedColor != 0x81828384u || pPagePropertyGrid->iErrorColor != 0x91929394u ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 1015;
	}
	xgeXuiPageUnload(&tPage);
	if ( xgeXuiPageLoadMemory(&tXui, sPropertyGridSelectXson, (int)strlen(sPropertyGridSelectXson), &tBinder, &tPage) == XGE_OK ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 1016;
	}
	sError = xgeXuiPageGetError(&tPage);
	if ( (strstr(sError, "onSelect is not supported yet") == NULL) || (strstr(sError, "onSelect") == NULL) ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 1017;
	}
	xgeXuiPageUnload(&tPage);
	if ( xgeXuiPageLoadMemory(&tXui, sBreadcrumbXson, (int)strlen(sBreadcrumbXson), &tBinder, &tPage) != XGE_OK ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 991;
	}
	pRoot = xgeXuiPageRoot(&tPage);
	if ( pRoot == NULL || pRoot->procEvent != xgeXuiBreadcrumbEventProc || pRoot->procPaint != xgeXuiBreadcrumbPaintProc || tPage.iBreadcrumbCount != 1 ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 992;
	}
	pPageBreadcrumb = (xge_xui_breadcrumb)pRoot->pUser;
	if ( pPageBreadcrumb != &tPage.arrBreadcrumb[0] || pPageBreadcrumb->pFont != &tFont || xgeXuiBreadcrumbGetSegmentCount(pPageBreadcrumb) != 3 || pPageBreadcrumb->arrSegments[1].iId != 20 || strcmp(pPageBreadcrumb->arrSegments[2].sText, "Assets") != 0 || xgeXuiBreadcrumbGetSelected(pPageBreadcrumb) != 2 || pPageBreadcrumb->fSegmentPaddingX != 8.0f || pPageBreadcrumb->fSeparatorWidth != 14.0f ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 993;
	}
	if ( pPageBreadcrumb->iBackgroundColor != 0x01020304u || pPageBreadcrumb->iSegmentColor != 0x11121314u || pPageBreadcrumb->iSelectedColor != 0x21222324u || pPageBreadcrumb->iBorderColor != 0x31323334u || pPageBreadcrumb->iTextColor != 0x41424344u || pPageBreadcrumb->iSeparatorColor != 0x51525354u ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 994;
	}
	xgeXuiPageUnload(&tPage);
	if ( xgeXuiPageLoadMemory(&tXui, sBreadcrumbSelectXson, (int)strlen(sBreadcrumbSelectXson), &tBinder, &tPage) == XGE_OK ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 995;
	}
	sError = xgeXuiPageGetError(&tPage);
	if ( (strstr(sError, "onSelect is not supported yet") == NULL) || (strstr(sError, "onSelect") == NULL) ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 996;
	}
	xgeXuiPageUnload(&tPage);
	if ( xgeXuiPageLoadMemory(&tXui, sAccordionXson, (int)strlen(sAccordionXson), &tBinder, &tPage) != XGE_OK ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 997;
	}
	pRoot = xgeXuiPageRoot(&tPage);
	if ( pRoot == NULL || pRoot->procEvent != xgeXuiAccordionEventProc || pRoot->procPaint != xgeXuiAccordionPaintProc || tPage.iAccordionCount != 1 ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 998;
	}
	pPageAccordion = (xge_xui_accordion)pRoot->pUser;
	if ( pPageAccordion != &tPage.arrAccordion[0] || pPageAccordion->pFont != &tFont || xgeXuiAccordionGetSectionCount(pPageAccordion) != 3 || pPageAccordion->iMode != XGE_XUI_ACCORDION_MODE_SINGLE || pPageAccordion->iSelected != 1 || pPageAccordion->fHeaderHeight != 24.0f || pPageAccordion->fSpacing != 3.0f || pPageAccordion->fContentPadding != 6.0f ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 999;
	}
	if ( pPageAccordion->arrSections[0].iId != 10 || pPageAccordion->arrSections[0].bExpanded != 0 || pPageAccordion->arrSections[1].iId != 20 || pPageAccordion->arrSections[1].bExpanded == 0 || strcmp(pPageAccordion->arrSections[1].sText, "Beta") != 0 || strcmp(pPageAccordion->arrSections[2].sTitle, "About") != 0 ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 1000;
	}
	if ( pPageAccordion->iBackgroundColor != 0x01020304u || pPageAccordion->iHeaderColor != 0x11121314u || pPageAccordion->iExpandedColor != 0x21222324u || pPageAccordion->iContentColor != 0x31323334u || pPageAccordion->iBorderColor != 0x41424344u || pPageAccordion->iTextColor != 0x51525354u || pPageAccordion->iContentTextColor != 0x61626364u ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 1001;
	}
	xgeXuiPageUnload(&tPage);
	if ( xgeXuiPageLoadMemory(&tXui, sAccordionSelectXson, (int)strlen(sAccordionSelectXson), &tBinder, &tPage) == XGE_OK ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 1002;
	}
	sError = xgeXuiPageGetError(&tPage);
	if ( (strstr(sError, "onSelect is not supported yet") == NULL) || (strstr(sError, "onSelect") == NULL) ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 1003;
	}
	xgeXuiPageUnload(&tPage);
	if ( xgeXuiPageLoadMemory(&tXui, sToastXson, (int)strlen(sToastXson), &tBinder, &tPage) != XGE_OK ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 1004;
	}
	pRoot = xgeXuiPageRoot(&tPage);
	if ( pRoot == NULL || pRoot->procEvent != xgeXuiToastEventProc || pRoot->procUpdate != xgeXuiToastUpdateProc || pRoot->procPaint != xgeXuiToastPaintProc || tPage.iToastCount != 1 ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 1005;
	}
	pPageToast = (xge_xui_toast)pRoot->pUser;
	if ( pPageToast != &tPage.arrToast[0] || pPageToast->pFont != &tFont || xgeXuiToastGetCount(pPageToast) != 3 || pPageToast->iPlacement != XGE_XUI_TOAST_PLACEMENT_BOTTOM_CENTER || pPageToast->fToastWidth != 180.0f || pPageToast->fToastHeight != 46.0f || pPageToast->fSpacing != 5.0f ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 1006;
	}
	if ( pPageToast->arrItems[0].iType != XGE_XUI_TOAST_TYPE_INFO || strcmp(pPageToast->arrItems[0].sMessage, "Queued") != 0 || pPageToast->arrItems[0].fDuration != 9.0f || pPageToast->arrItems[1].iType != XGE_XUI_TOAST_TYPE_SUCCESS || strcmp(pPageToast->arrItems[1].sMessage, "Done") != 0 || strcmp(pPageToast->arrItems[2].sTitle, "Plain") != 0 ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 1007;
	}
	if ( pPageToast->iBackgroundColor != 0x01020304u || pPageToast->iBorderColor != 0x11121314u || pPageToast->iTextColor != 0x21222324u || pPageToast->iMutedTextColor != 0x31323334u || pPageToast->iInfoColor != 0x41424344u || pPageToast->iSuccessColor != 0x51525354u || pPageToast->iWarningColor != 0x61626364u || pPageToast->iErrorColor != 0x71727374u || pPageToast->iCloseColor != 0x81828384u || pPageToast->iCloseHoverColor != 0x91929394u ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 1008;
	}
	xgeXuiPageUnload(&tPage);
	if ( xgeXuiPageLoadMemory(&tXui, sToastCloseXson, (int)strlen(sToastCloseXson), &tBinder, &tPage) == XGE_OK ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 1009;
	}
	sError = xgeXuiPageGetError(&tPage);
	if ( (strstr(sError, "onClose is not supported yet") == NULL) || (strstr(sError, "onClose") == NULL) ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 1010;
	}
	xgeXuiPageUnload(&tPage);
	if ( xgeXuiPageLoadMemory(&tXui, sVirtualListXson, (int)strlen(sVirtualListXson), &tBinder, &tPage) != XGE_OK ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 566;
	}
	pRoot = xgeXuiPageRoot(&tPage);
	if ( pRoot == NULL || pRoot->procEvent != xgeXuiVirtualListEventProc || pRoot->procPaint != xgeXuiVirtualListPaintProc || pRoot->procLayout != xgeXuiVirtualListLayoutProc || tPage.iVirtualListCount != 1 ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 567;
	}
	pPageVirtualList = (xge_xui_virtual_list)pRoot->pUser;
	if ( pPageVirtualList != &tPage.arrVirtualList[0] || pPageVirtualList->iItemCount != 10 || pPageVirtualList->fItemHeight != 20.0f || pPageVirtualList->fScrollY != 20.0f || pPageVirtualList->iBackgroundColor != 0x01020304u || pPageVirtualList->iBarColor != 0x11121314u || pPageVirtualList->iThumbColor != 0x21222324u ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 568;
	}
	tRect.fX = 0.0f;
	tRect.fY = 0.0f;
	tRect.fW = 100.0f;
	tRect.fH = 80.0f;
	xgeXuiWidgetSetRect(pRoot, tRect);
	xgeXuiUpdate(&tXui, 0.0f);
	pChild = xgeXuiVirtualListGetSlotWidget(pPageVirtualList, 0);
	pGrid = xgeXuiPageFind(&tPage, "item-label");
	if ( xgeXuiVirtualListGetFirstVisible(pPageVirtualList) != 1 || xgeXuiVirtualListGetVisibleCount(pPageVirtualList) != 5 || pChild == NULL || pChild->iId != 0 || pChild->sName == NULL || strcmp(pChild->sName, "row-template") != 0 || pChild->pInternal != (void*)(intptr_t)2 || pGrid == NULL || pGrid->pUser == NULL || pGrid->procPaint != xgeXuiLabelPaintProc || tPage.iLabelCount != 5 ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 569;
	}
	tRect = xgeXuiWidgetGetRect(pChild);
	if ( tRect.fX != 5.0f || tRect.fY != 5.0f || tRect.fW != 90.0f || tRect.fH != 20.0f ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 570;
	}
	xgeXuiVirtualListSetScroll(pPageVirtualList, 40.0f);
	xgeXuiUpdate(&tXui, 0.0f);
	pChild = xgeXuiVirtualListGetSlotWidget(pPageVirtualList, 0);
	if ( xgeXuiVirtualListGetFirstVisible(pPageVirtualList) != 2 || pChild == NULL || pChild->pInternal != (void*)(intptr_t)3 || tPage.iLabelCount != 5 ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 571;
	}
	if ( xgeXuiPageRefreshStyle(&tPage) != XGE_OK ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 572;
	}
	xgeXuiPageUnload(&tPage);
	if ( xgeXuiPageLoadMemory(&tXui, sModelXson, (int)strlen(sModelXson), &tBinder, &tPage) != XGE_OK ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 573;
	}
	pChild = xgeXuiPageFind(&tPage, "model-title");
	pRoot = xgeXuiPageFind(&tPage, "model-field");
	pGrid = xgeXuiPageFind(&tPage, "model-icon");
	if ( pChild == NULL || pRoot == NULL || pGrid == NULL ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 574;
	}
	pPageLabel = (xge_xui_label)pChild->pUser;
	pPageInput = (xge_xui_input)pRoot->pUser;
	pPageImage = (xge_xui_image)pGrid->pUser;
	if ( pPageLabel == NULL || pPageInput == NULL || pPageImage == NULL || tPage.iModelBindingCount != 3 || strcmp(pPageLabel->sText, "") != 0 || strcmp(xgeXuiInputGetText(pPageInput), "") != 0 || pPageImage->pTexture != NULL ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 574;
	}
	if ( xgeXuiModelSetText(&tModel, "player.name", "Ada") != XGE_OK || xgeXuiModelSetText(&tModel, "icon.path", "") != XGE_OK || xgeXuiPageApplyModel(&tPage, &tModel) != XGE_OK ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 575;
	}
	if ( tPage.iModelVersion != xgeXuiModelVersion(&tModel) || strcmp(pPageLabel->sText, "Ada") != 0 || strcmp(xgeXuiInputGetText(pPageInput), "Ada") != 0 || pPageImage->pTexture != NULL || ((xgeXuiWidgetGetFlags(pChild) & XGE_XUI_WIDGET_DIRTY_PAINT) == 0) ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 576;
	}
	if ( xgeXuiModelSetText(&tModel, "player.name", "Lin") != XGE_OK || xgeXuiPageApplyModel(&tPage, &tModel) != XGE_OK || strcmp(pPageLabel->sText, "Lin") != 0 || strcmp(xgeXuiInputGetText(pPageInput), "Lin") != 0 ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 577;
	}
	xgeXuiPageUnload(&tPage);
	xgeXuiModelInit(&tModel);
	if ( xgeXuiPageLoadMemory(&tXui, sModelMissingXson, (int)strlen(sModelMissingXson), &tBinder, &tPage) != XGE_OK || xgeXuiPageApplyModel(&tPage, &tModel) != XGE_ERROR_INVALID_ARGUMENT ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 578;
	}
	xgeXuiPageUnload(&tPage);
	if ( xgeXuiPageLoadMemory(&tXui, sControlXson, (int)strlen(sControlXson), &tBinder, &tPage) != XGE_OK ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 539;
	}
	pChild = xgeXuiPageFind(&tPage, "title");
	pGrid = xgeXuiPageFind(&tPage, "rule");
	if ( pChild == NULL || pChild->pUser == NULL || pChild->procMeasure != xgeXuiLabelMeasureProc || pChild->procPaint != xgeXuiLabelPaintProc || tPage.iLabelCount != 1 || tPage.iButtonCount != 1 || tPage.iImageCount != 1 || tPage.iInputCount != 1 ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 540;
	}
	pPageLabel = (xge_xui_label)pChild->pUser;
	if ( strcmp(pPageLabel->sText, "Hello") != 0 || pPageLabel->pFont != &tFont || pPageLabel->iColor != XGE_COLOR_RGBA(1, 2, 3, 255) || (pPageLabel->iTextFlags & XGE_TEXT_ALIGN_CENTER) == 0 || (pPageLabel->iTextFlags & XGE_TEXT_ALIGN_MIDDLE) == 0 ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 541;
	}
	pRoot = xgeXuiPageFind(&tPage, "action");
	if ( pRoot == NULL || pRoot->pUser == NULL || pRoot->procEvent != xgeXuiButtonEventProc || pRoot->procPaint != xgeXuiButtonPaintProc ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 544;
	}
	pPageButton = (xge_xui_button)pRoot->pUser;
	if ( strcmp(pPageButton->sText, "Run") != 0 || pPageButton->pFont != &tFont || pPageButton->procClick != __testXuiButtonClick || pPageButton->pUser != &tBinder || pPageButton->iTextColor != XGE_COLOR_RGBA(1, 2, 3, 255) || pPageButton->iColorNormal != XGE_COLOR_RGBA(0x20, 0x30, 0x40, 0xFF) || pPageButton->iColorHover != XGE_COLOR_RGBA(0x30, 0x40, 0x50, 0xFF) || pPageButton->iColorActive != XGE_COLOR_RGBA(0x40, 0x50, 0x60, 0xFF) || pPageButton->iColorFocus != XGE_COLOR_RGBA(0x50, 0x60, 0x70, 0xFF) || pPageButton->iColorDisabled != XGE_COLOR_RGBA(0x60, 0x70, 0x80, 0xFF) || (pPageButton->iTextFlags & XGE_TEXT_ALIGN_RIGHT) == 0 || (pPageButton->iTextFlags & XGE_TEXT_ALIGN_BOTTOM) == 0 ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 545;
	}
	g_iXuiButtonClicks = 0;
	g_pXuiLastClickWidget = NULL;
	g_pXuiLastClickUser = NULL;
	pRoot->tRect = (xge_rect_t){ 0.0f, 0.0f, 20.0f, 20.0f };
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_MOUSE_DOWN;
	tEvent.iParam1 = XGE_MOUSE_LEFT;
	tEvent.fX = 1.0f;
	tEvent.fY = 1.0f;
	if ( pRoot->procEvent(pRoot, &tEvent, pRoot->pUser) != XGE_XUI_EVENT_CONSUMED ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 546;
	}
	tEvent.iType = XGE_EVENT_MOUSE_UP;
	if ( pRoot->procEvent(pRoot, &tEvent, pRoot->pUser) != XGE_XUI_EVENT_CONSUMED || g_iXuiButtonClicks != 1 || g_pXuiLastClickWidget != pRoot || g_pXuiLastClickUser != &tBinder || pPageButton->iClickCount != 1 ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 547;
	}
	pRoot = xgeXuiPageFind(&tPage, "icon");
	if ( pRoot == NULL || pRoot->pUser == NULL || pRoot->procMeasure != xgeXuiImageMeasureProc || pRoot->procPaint != xgeXuiImagePaintProc ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 548;
	}
	pPageImage = (xge_xui_image)pRoot->pUser;
	if ( pPageImage->pTexture != &tTexture || pPageImage->tSrc.fX != 1.0f || pPageImage->tSrc.fY != 2.0f || pPageImage->tSrc.fW != 3.0f || pPageImage->tSrc.fH != 4.0f || pPageImage->iColor != 0xAABBCCDDu || pPageImage->iMode != XGE_XUI_IMAGE_FIT || tPage.arrImageTextureOwned[0] != 0 ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 549;
	}
	pRoot = xgeXuiPageFind(&tPage, "field");
	if ( pRoot == NULL || pRoot->pUser == NULL || pRoot->procEvent != xgeXuiInputEventProc || pRoot->procUpdate != xgeXuiInputUpdateProc || pRoot->procPaint != xgeXuiInputPaintProc || xgeXuiWidgetIsEnabled(pRoot) == 0 ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 550;
	}
	pPageInput = (xge_xui_input)pRoot->pUser;
	if ( strcmp(xgeXuiInputGetText(pPageInput), "abc") != 0 || strcmp(pPageInput->sPlaceholder, "Name") != 0 || pPageInput->pFont != &tFont || pPageInput->iTextColor != XGE_COLOR_RGBA(0x11, 0x12, 0x13, 0xFF) || pPageInput->iBackgroundColor != XGE_COLOR_RGBA(0x21, 0x22, 0x23, 0xFF) || pPageInput->iFocusColor != XGE_COLOR_RGBA(0x31, 0x32, 0x33, 0xFF) || pPageInput->iCursorColor != XGE_COLOR_RGBA(0x41, 0x42, 0x43, 0xFF) || pPageInput->iPlaceholderColor != XGE_COLOR_RGBA(0x51, 0x52, 0x53, 0xFF) || pPageInput->iSelectionColor != XGE_COLOR_RGBA(0x61, 0x62, 0x63, 0xFF) || pPageInput->iDisabledTextColor != XGE_COLOR_RGBA(0x71, 0x72, 0x73, 0xFF) || pPageInput->iDisabledBackgroundColor != XGE_COLOR_RGBA(0x81, 0x82, 0x83, 0xFF) || pPageInput->bPassword == 0 || pPageInput->bReadonly == 0 || pPageInput->bDisabled != 0 || pPageInput->tText.iSelectStart != 1 || pPageInput->tText.iSelectEnd != 2 ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 551;
	}
	if ( pGrid == NULL || pGrid->pUser == NULL || pGrid->procPaint != xgeXuiSeparatorPaintProc || tPage.iSeparatorCount != 1 ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 542;
	}
	pPageSeparator = (xge_xui_separator)pGrid->pUser;
	if ( pPageSeparator->iOrientation != XGE_XUI_SEPARATOR_VERTICAL || pPageSeparator->fThickness != 3.0f || pPageSeparator->iColor != XGE_COLOR_RGBA(0x11, 0x22, 0x33, 0xFF) ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 543;
	}
	xgeXuiPageUnload(&tPage);
	if ( xgeXuiPageLoadMemory(&tXui, sNumericInputXson, (int)strlen(sNumericInputXson), &tBinder, &tPage) != XGE_OK ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 585;
	}
	pRoot = xgeXuiPageFind(&tPage, "num");
	if ( pRoot == NULL || pRoot->pUser == NULL || pRoot->procEvent != xgeXuiNumericInputEventProc || pRoot->procUpdate != xgeXuiNumericInputUpdateProc || pRoot->procPaint != xgeXuiNumericInputPaintProc || tPage.iNumericInputCount != 1 ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 586;
	}
	pPageNumeric = (xge_xui_numeric_input)pRoot->pUser;
	if ( pPageNumeric != &tPage.arrNumericInput[0] || pPageNumeric->tInput.pFont != &tFont || pPageNumeric->fMin != -5.0f || pPageNumeric->fMax != 8.0f || pPageNumeric->fStep != 2.0f || xgeXuiNumericInputGetValue(pPageNumeric) != 6.0f || pPageNumeric->bInteger == 0 || pPageNumeric->bShowSpinner != 0 || strcmp(pPageNumeric->tInput.sPlaceholder, "Qty") != 0 || pPageNumeric->tInput.iTextColor != XGE_COLOR_RGBA(1, 2, 3, 255) || pPageNumeric->tInput.iBackgroundColor != XGE_COLOR_RGBA(0xF4, 0xFA, 0xFF, 0xFF) || pPageNumeric->tInput.iFocusColor != XGE_COLOR_RGBA(0xB8, 0xDF, 0xF5, 0xFF) || pPageNumeric->tInput.iCursorColor != XGE_COLOR_RGBA(0x0F, 0x6E, 0xA8, 0xFF) ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 587;
	}
	xgeXuiSetFocus(&tXui, pRoot);
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_KEY_DOWN;
	tEvent.iParam1 = XGE_KEY_UP;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || xgeXuiNumericInputGetValue(pPageNumeric) != 8.0f ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 588;
	}
	xgeXuiPageUnload(&tPage);
	if ( xgeXuiPageLoadMemory(&tXui, sNumericInputChangeXson, (int)strlen(sNumericInputChangeXson), &tBinder, &tPage) == XGE_OK ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 589;
	}
	sError = xgeXuiPageGetError(&tPage);
	if ( (strstr(sError, "onChange is not supported yet") == NULL) || (strstr(sError, "onChange") == NULL) ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 590;
	}
	xgeXuiPageUnload(&tPage);
	if ( xgeXuiPageLoadMemory(&tXui, sToggleXson, (int)strlen(sToggleXson), &tBinder, &tPage) != XGE_OK ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 591;
	}
	pChild = xgeXuiPageFind(&tPage, "agree");
	pGrid = xgeXuiPageFind(&tPage, "choice");
	pRoot = xgeXuiPageFind(&tPage, "wifi");
	if ( pChild == NULL || pGrid == NULL || pRoot == NULL || pChild->procEvent != xgeXuiCheckBoxEventProc || pGrid->procEvent != xgeXuiRadioEventProc || pRoot->procEvent != xgeXuiSwitchEventProc || tPage.iCheckBoxCount != 1 || tPage.iRadioCount != 1 || tPage.iSwitchCount != 1 ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 592;
	}
	pPageCheckBox = (xge_xui_checkbox)pChild->pUser;
	pPageRadio = (xge_xui_radio)pGrid->pUser;
	pPageSwitch = (xge_xui_switch)pRoot->pUser;
	if ( pPageCheckBox != &tPage.arrCheckBox[0] || pPageRadio != &tPage.arrRadio[0] || pPageSwitch != &tPage.arrSwitch[0] || strcmp(pPageCheckBox->sText, "Agree") != 0 || strcmp(pPageRadio->sText, "Choice") != 0 || strcmp(pPageSwitch->sText, "WiFi") != 0 || pPageCheckBox->pFont != &tFont || pPageRadio->pFont != &tFont || pPageSwitch->pFont != &tFont || xgeXuiCheckBoxGetChecked(pPageCheckBox) != 1 || xgeXuiRadioGetChecked(pPageRadio) != 1 || pPageRadio->iValue != 7 || xgeXuiSwitchGetChecked(pPageSwitch) != 0 ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 593;
	}
	if ( pPageCheckBox->iTextColor != XGE_COLOR_RGBA(1, 2, 3, 255) || pPageCheckBox->iColorBox != XGE_COLOR_RGBA(0xD8, 0xEC, 0xF8, 0xFF) || pPageCheckBox->iColorChecked != XGE_COLOR_RGBA(0x16, 0x8A, 0xC2, 0xFF) || pPageRadio->iColorRing != XGE_COLOR_RGBA(0xD8, 0xEC, 0xF8, 0xFF) || pPageRadio->iColorChecked != XGE_COLOR_RGBA(0x16, 0x8A, 0xC2, 0xFF) || pPageSwitch->iColorTrack != XGE_COLOR_RGBA(0xD8, 0xEC, 0xF8, 0xFF) || pPageSwitch->iColorKnob != XGE_COLOR_RGBA(0xFF, 0xFF, 0xFF, 0xFF) || pPageSwitch->iColorChecked != XGE_COLOR_RGBA(0x16, 0x8A, 0xC2, 0xFF) ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 594;
	}
	xgeXuiWidgetSetRect(pChild, (xge_rect_t){ 0.0f, 0.0f, 80.0f, 22.0f });
	xgeXuiWidgetSetRect(pRoot, (xge_rect_t){ 0.0f, 24.0f, 90.0f, 22.0f });
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_MOUSE_DOWN;
	tEvent.iParam1 = XGE_MOUSE_LEFT;
	tEvent.fX = 5.0f;
	tEvent.fY = 5.0f;
	if ( pChild->procEvent(pChild, &tEvent, pChild->pUser) != XGE_XUI_EVENT_CONSUMED ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 595;
	}
	tEvent.iType = XGE_EVENT_MOUSE_UP;
	if ( pChild->procEvent(pChild, &tEvent, pChild->pUser) != XGE_XUI_EVENT_CONSUMED || xgeXuiCheckBoxGetChecked(pPageCheckBox) != 0 ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 596;
	}
	tEvent.iType = XGE_EVENT_MOUSE_DOWN;
	tEvent.fY = 29.0f;
	if ( pRoot->procEvent(pRoot, &tEvent, pRoot->pUser) != XGE_XUI_EVENT_CONSUMED ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 597;
	}
	tEvent.iType = XGE_EVENT_MOUSE_UP;
	if ( pRoot->procEvent(pRoot, &tEvent, pRoot->pUser) != XGE_XUI_EVENT_CONSUMED || xgeXuiSwitchGetChecked(pPageSwitch) != 1 ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 598;
	}
	xgeXuiPageUnload(&tPage);
	if ( xgeXuiPageLoadMemory(&tXui, sToggleChangeXson, (int)strlen(sToggleChangeXson), &tBinder, &tPage) == XGE_OK ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 599;
	}
	sError = xgeXuiPageGetError(&tPage);
	if ( (strstr(sError, "onChange is not supported yet") == NULL) || (strstr(sError, "onChange") == NULL) ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 600;
	}
	xgeXuiPageUnload(&tPage);
	if ( xgeXuiPageLoadMemory(&tXui, sRangeXson, (int)strlen(sRangeXson), &tBinder, &tPage) != XGE_OK ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 601;
	}
	pChild = xgeXuiPageFind(&tPage, "volume");
	pRoot = xgeXuiPageFind(&tPage, "load");
	if ( pChild == NULL || pRoot == NULL || pChild->procEvent != xgeXuiSliderEventProc || pChild->procPaint != xgeXuiSliderPaintProc || pRoot->procEvent != NULL || pRoot->procPaint != xgeXuiProgressPaintProc || tPage.iSliderCount != 1 || tPage.iProgressCount != 1 ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 602;
	}
	pPageSlider = (xge_xui_slider)pChild->pUser;
	pPageProgress = (xge_xui_progress)pRoot->pUser;
	if ( pPageSlider != &tPage.arrSlider[0] || pPageProgress != &tPage.arrProgress[0] || pPageSlider->fMin != 0.0f || pPageSlider->fMax != 100.0f || xgeXuiSliderGetValue(pPageSlider) != 25.0f || pPageProgress->fMin != 0.0f || pPageProgress->fMax != 10.0f || xgeXuiProgressGetValue(pPageProgress) != 6.0f || pPageProgress->pFont != &tFont || strcmp(pPageProgress->sText, "60%") != 0 ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 603;
	}
	if ( pPageSlider->iColorTrack != XGE_COLOR_RGBA(0xD8, 0xEC, 0xF8, 0xFF) || pPageSlider->iColorFill != XGE_COLOR_RGBA(0x16, 0x8A, 0xC2, 0xFF) || pPageSlider->iColorKnob != XGE_COLOR_RGBA(0xFF, 0xFF, 0xFF, 0xFF) || pPageSlider->iColorFocus != XGE_COLOR_RGBA(0xB8, 0xDF, 0xF5, 0xFF) || pPageProgress->iColorTrack != XGE_COLOR_RGBA(0xD8, 0xEC, 0xF8, 0xFF) || pPageProgress->iColorFill != XGE_COLOR_RGBA(0x16, 0x8A, 0xC2, 0xFF) || pPageProgress->iTextColor != XGE_COLOR_RGBA(1, 2, 3, 255) ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 604;
	}
	xgeXuiWidgetSetRect(pChild, (xge_rect_t){ 0.0f, 0.0f, 100.0f, 20.0f });
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_MOUSE_DOWN;
	tEvent.iParam1 = XGE_MOUSE_LEFT;
	tEvent.fX = 50.0f;
	tEvent.fY = 10.0f;
	if ( pChild->procEvent(pChild, &tEvent, pChild->pUser) != XGE_XUI_EVENT_CONSUMED || xgeXuiSliderGetValue(pPageSlider) != 50.0f ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 605;
	}
	xgeXuiPageUnload(&tPage);
	if ( xgeXuiPageLoadMemory(&tXui, sSliderChangeXson, (int)strlen(sSliderChangeXson), &tBinder, &tPage) == XGE_OK ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 606;
	}
	sError = xgeXuiPageGetError(&tPage);
	if ( (strstr(sError, "onChange is not supported yet") == NULL) || (strstr(sError, "onChange") == NULL) ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 607;
	}
	xgeXuiPageUnload(&tPage);
	if ( xgeXuiPageLoadMemory(&tXui, sTabsXson, (int)strlen(sTabsXson), &tBinder, &tPage) != XGE_OK ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 608;
	}
	pRoot = xgeXuiPageFind(&tPage, "main-tabs");
	if ( pRoot == NULL || pRoot->procEvent != xgeXuiTabsEventProc || pRoot->procPaint != xgeXuiTabsPaintProc || tPage.iTabsCount != 1 ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 609;
	}
	pPageTabs = (xge_xui_tabs)pRoot->pUser;
	if ( pPageTabs != &tPage.arrTabs[0] || pPageTabs->pFont != &tFont || pPageTabs->iItemCount != 3 || strcmp(pPageTabs->arrItems[0], "Home") != 0 || strcmp(pPageTabs->arrItems[1], "Edit") != 0 || strcmp(pPageTabs->arrItems[2], "View") != 0 || xgeXuiTabsGetSelected(pPageTabs) != 1 || pPageTabs->fTabWidth != 54.0f || pPageTabs->fTabHeight != 22.0f ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 610;
	}
	if ( pPageTabs->iBackgroundColor != XGE_COLOR_RGBA(0xF4, 0xFA, 0xFF, 0xFF) || pPageTabs->iTabColor != XGE_COLOR_RGBA(0xEA, 0xF6, 0xFD, 0xFF) || pPageTabs->iHoverColor != XGE_COLOR_RGBA(0xD8, 0xEC, 0xF8, 0xFF) || pPageTabs->iActiveColor != XGE_COLOR_RGBA(0xC7, 0xE8, 0xF8, 0xFF) || pPageTabs->iTextColor != XGE_COLOR_RGBA(1, 2, 3, 255) || pPageTabs->iActiveTextColor != XGE_COLOR_RGBA(0x0F, 0x6E, 0xA8, 0xFF) ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 611;
	}
	xgeXuiWidgetSetRect(pRoot, (xge_rect_t){ 0.0f, 0.0f, 180.0f, 28.0f });
	xgeXuiUpdate(&tXui, 0.0f);
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_MOUSE_DOWN;
	tEvent.iParam1 = XGE_MOUSE_LEFT;
	tEvent.fX = 120.0f;
	tEvent.fY = 10.0f;
	if ( pRoot->procEvent(pRoot, &tEvent, pRoot->pUser) != XGE_XUI_EVENT_CONSUMED ) {
		printf("tabs-event layout=%p layoutUser=%p tabs=%p scroll=%.1f contentW=%.1f tabW=%.1f count=%d\n",
			(void*)pRoot->procLayout, pRoot->pLayoutUser, (void*)pPageTabs, pPageTabs->fScrollX, pRoot->tContentRect.fW, pPageTabs->fTabWidth, pPageTabs->iItemCount);
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 612;
	}
	tEvent.iType = XGE_EVENT_MOUSE_UP;
	if ( pRoot->procEvent(pRoot, &tEvent, pRoot->pUser) != XGE_XUI_EVENT_CONSUMED || xgeXuiTabsGetSelected(pPageTabs) != 2 ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 613;
	}
	xgeXuiPageUnload(&tPage);
	if ( xgeXuiPageLoadMemory(&tXui, sToolbarXson, (int)strlen(sToolbarXson), &tBinder, &tPage) != XGE_OK ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 657;
	}
	pRoot = xgeXuiPageFind(&tPage, "main-toolbar");
	if ( pRoot == NULL || pRoot->procEvent != xgeXuiToolbarEventProc || pRoot->procPaint != xgeXuiToolbarPaintProc || tPage.iToolbarCount != 1 ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 658;
	}
	pPageToolbar = (xge_xui_toolbar)pRoot->pUser;
	if ( pPageToolbar != &tPage.arrToolbar[0] || pPageToolbar->pFont != &tFont || pPageToolbar->iItemCount != 4 || strcmp(pPageToolbar->arrItems[0].sText, "New") != 0 || strcmp(pPageToolbar->arrItems[1].sText, "Pin") != 0 || pPageToolbar->arrItems[1].iType != XGE_XUI_TOOLBAR_ITEM_TOGGLE || pPageToolbar->arrItems[1].bChecked != 1 || pPageToolbar->arrItems[2].iType != XGE_XUI_TOOLBAR_ITEM_SEPARATOR || pPageToolbar->arrItems[3].bEnabled != 0 ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 659;
	}
	if ( pPageToolbar->fItemWidth != 50.0f || pPageToolbar->fItemHeight != 24.0f || pPageToolbar->fSeparatorSize != 8.0f || pPageToolbar->iBackgroundColor != XGE_COLOR_RGBA(0xF4, 0xFA, 0xFF, 0xFF) || pPageToolbar->iItemColor != XGE_COLOR_RGBA(0xEA, 0xF6, 0xFD, 0xFF) || pPageToolbar->iCheckedColor != XGE_COLOR_RGBA(0xB8, 0xDF, 0xF5, 0xFF) || pPageToolbar->iTextColor != XGE_COLOR_RGBA(1, 2, 3, 255) ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 660;
	}
	xgeXuiWidgetSetRect(pRoot, (xge_rect_t){ 0.0f, 0.0f, 220.0f, 30.0f });
	xgeXuiUpdate(&tXui, 0.0f);
	xgeXuiPaint(&tXui);
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_MOUSE_DOWN;
	tEvent.iParam1 = XGE_MOUSE_LEFT;
	tEvent.fX = 60.0f;
	tEvent.fY = 10.0f;
	if ( pRoot->procEvent(pRoot, &tEvent, pRoot->pUser) != XGE_XUI_EVENT_CONSUMED ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 661;
	}
	tEvent.iType = XGE_EVENT_MOUSE_UP;
	if ( pRoot->procEvent(pRoot, &tEvent, pRoot->pUser) != XGE_XUI_EVENT_CONSUMED || xgeXuiToolbarGetItemChecked(pPageToolbar, 1) != 0 ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 662;
	}
	xgeXuiPageUnload(&tPage);
	if ( xgeXuiPageLoadMemory(&tXui, sToolbarSelectXson, (int)strlen(sToolbarSelectXson), &tBinder, &tPage) == XGE_OK ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 663;
	}
	sError = xgeXuiPageGetError(&tPage);
	if ( sError == NULL || strstr(sError, "onSelect") == NULL ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 664;
	}
	xgeXuiPageUnload(&tPage);
	if ( xgeXuiPageLoadMemory(&tXui, sStatusBarXson, (int)strlen(sStatusBarXson), &tBinder, &tPage) != XGE_OK ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 665;
	}
	pRoot = xgeXuiPageFind(&tPage, "main-status");
	if ( pRoot == NULL || pRoot->procEvent != xgeXuiStatusBarEventProc || pRoot->procPaint != xgeXuiStatusBarPaintProc || tPage.iStatusBarCount != 1 ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 666;
	}
	pPageStatusBar = (xge_xui_status_bar)pRoot->pUser;
	if ( pPageStatusBar != &tPage.arrStatusBar[0] || pPageStatusBar->pFont != &tFont || pPageStatusBar->iItemCount != 4 || strcmp(pPageStatusBar->arrItems[0].sText, "Ready") != 0 || pPageStatusBar->arrItems[0].bClickable != 1 || pPageStatusBar->arrItems[1].iType != XGE_XUI_STATUS_BAR_ITEM_PROGRESS || pPageStatusBar->arrItems[1].fValue != 45.0f || pPageStatusBar->arrItems[2].iSection != XGE_XUI_STATUS_BAR_SECTION_CENTER || pPageStatusBar->arrItems[3].iSection != XGE_XUI_STATUS_BAR_SECTION_RIGHT || pPageStatusBar->arrItems[3].bEnabled != 0 ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 667;
	}
	if ( pPageStatusBar->fHeight != 24.0f || pPageStatusBar->fGap != 4.0f || pPageStatusBar->fItemPadding != 6.0f || pPageStatusBar->iBackgroundColor != XGE_COLOR_RGBA(0xF4, 0xFA, 0xFF, 0xFF) || pPageStatusBar->iBorderColor != XGE_COLOR_RGBA(0x7F, 0xC4, 0xE5, 0xFF) || pPageStatusBar->iProgressFillColor != XGE_COLOR_RGBA(0x16, 0x8A, 0xC2, 0xFF) ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 668;
	}
	xgeXuiWidgetSetRect(pRoot, (xge_rect_t){ 0.0f, 0.0f, 320.0f, 26.0f });
	xgeXuiUpdate(&tXui, 0.0f);
	xgeXuiPaint(&tXui);
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_MOUSE_DOWN;
	tEvent.iParam1 = XGE_MOUSE_LEFT;
	tEvent.fX = 10.0f;
	tEvent.fY = 10.0f;
	if ( pRoot->procEvent(pRoot, &tEvent, pRoot->pUser) != XGE_XUI_EVENT_CONSUMED ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 669;
	}
	tEvent.iType = XGE_EVENT_MOUSE_UP;
	if ( pRoot->procEvent(pRoot, &tEvent, pRoot->pUser) != XGE_XUI_EVENT_CONSUMED || pPageStatusBar->iSelectCount != 1 ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 670;
	}
	xgeXuiPageUnload(&tPage);
	if ( xgeXuiPageLoadMemory(&tXui, sStatusBarSelectXson, (int)strlen(sStatusBarSelectXson), &tBinder, &tPage) == XGE_OK ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 671;
	}
	sError = xgeXuiPageGetError(&tPage);
	if ( sError == NULL || strstr(sError, "onSelect") == NULL ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 672;
	}
	xgeXuiPageUnload(&tPage);
	if ( xgeXuiPageLoadMemory(&tXui, sTabsSelectXson, (int)strlen(sTabsSelectXson), &tBinder, &tPage) == XGE_OK ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 614;
	}
	sError = xgeXuiPageGetError(&tPage);
	if ( (strstr(sError, "onSelect is not supported yet") == NULL) || (strstr(sError, "onSelect") == NULL) ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 615;
	}
	xgeXuiPageUnload(&tPage);
	if ( xgeXuiPageLoadMemory(&tXui, sComboBoxXson, (int)strlen(sComboBoxXson), &tBinder, &tPage) != XGE_OK ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 616;
	}
	pRoot = xgeXuiPageFind(&tPage, "mode");
	if ( pRoot == NULL || pRoot->procEvent != xgeXuiComboBoxEventProc || pRoot->procPaint != xgeXuiComboBoxPaintProc || tPage.iComboBoxCount != 1 ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 617;
	}
	pPageCombo = (xge_xui_combo_box)pRoot->pUser;
	if ( pPageCombo == NULL || pPageCombo != tPage.arrComboBox[0] || pPageCombo->pFont != &tFont || pPageCombo->iItemCount != 3 || strcmp(pPageCombo->arrItems[0], "One") != 0 || strcmp(pPageCombo->arrItems[1], "Two") != 0 || strcmp(pPageCombo->arrItems[2], "Three") != 0 || xgeXuiComboBoxGetSelected(pPageCombo) != 1 || pPageCombo->fDropDownHeight != 72.0f ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 618;
	}
	if ( pPageCombo->iColorNormal != XGE_COLOR_RGBA(0xF4, 0xFA, 0xFF, 0xFF) || pPageCombo->iColorHover != XGE_COLOR_RGBA(0xD8, 0xEC, 0xF8, 0xFF) || pPageCombo->iColorActive != XGE_COLOR_RGBA(0xC7, 0xE8, 0xF8, 0xFF) || pPageCombo->iColorFocus != XGE_COLOR_RGBA(0xB8, 0xDF, 0xF5, 0xFF) || pPageCombo->iColorDisabled != XGE_COLOR_RGBA(0xEE, 0xF6, 0xFB, 0xFF) || pPageCombo->iTextColor != XGE_COLOR_RGBA(1, 2, 3, 255) || pPageCombo->iPopupColor != XGE_COLOR_RGBA(0xFF, 0xFF, 0xFF, 0xFF) ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 619;
	}
	xgeXuiWidgetSetRect(pRoot, (xge_rect_t){ 0.0f, 0.0f, 120.0f, 24.0f });
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_MOUSE_DOWN;
	tEvent.iParam1 = XGE_MOUSE_LEFT;
	tEvent.fX = 8.0f;
	tEvent.fY = 8.0f;
	if ( pRoot->procEvent(pRoot, &tEvent, pRoot->pUser) != XGE_XUI_EVENT_CONSUMED || xgeXuiComboBoxIsOpen(pPageCombo) == 0 ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 620;
	}
	xgeXuiPageUnload(&tPage);
	if ( xgeXuiPageLoadMemory(&tXui, sComboBoxSelectXson, (int)strlen(sComboBoxSelectXson), &tBinder, &tPage) == XGE_OK ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 621;
	}
	sError = xgeXuiPageGetError(&tPage);
	if ( (strstr(sError, "onSelect is not supported yet") == NULL) || (strstr(sError, "onSelect") == NULL) ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 622;
	}
	xgeXuiPageUnload(&tPage);
	if ( xgeXuiPageLoadMemory(&tXui, sPopupXson, (int)strlen(sPopupXson), &tBinder, &tPage) != XGE_OK ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 623;
	}
	pChild = xgeXuiPageFind(&tPage, "owner");
	pRoot = xgeXuiPageFind(&tPage, "popup");
	if ( pChild == NULL || pRoot == NULL || pRoot->procEvent != xgeXuiPopupEventProc || pRoot->procPaint != xgeXuiPopupPaintProc || tPage.iPopupCount != 1 ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 624;
	}
	pPagePopup = (xge_xui_popup)pRoot->pUser;
	if ( pPagePopup == NULL || pPagePopup != tPage.arrPopup[0] || pPagePopup->pOwner != pChild || xgeXuiPopupIsOpen(pPagePopup) != 1 || xgeXuiWidgetIsVisible(pRoot) == 0 || pPagePopup->iBackgroundColor != XGE_COLOR_RGBA(0xF4, 0xFA, 0xFF, 0xFF) || pPagePopup->bCloseOnOutside != 0 || pPagePopup->bCloseOnEscape != 1 ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 625;
	}
	xgeXuiWidgetSetRect(pChild, (xge_rect_t){ 0.0f, 0.0f, 80.0f, 24.0f });
	xgeXuiWidgetSetRect(pRoot, (xge_rect_t){ 20.0f, 20.0f, 100.0f, 70.0f });
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_MOUSE_DOWN;
	tEvent.fX = 300.0f;
	tEvent.fY = 300.0f;
	if ( pRoot->procEvent(pRoot, &tEvent, pRoot->pUser) != XGE_XUI_EVENT_CONTINUE || xgeXuiPopupIsOpen(pPagePopup) != 1 ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 626;
	}
	tEvent.iType = XGE_EVENT_KEY_DOWN;
	tEvent.iParam1 = XGE_KEY_ESCAPE;
	if ( pRoot->procEvent(pRoot, &tEvent, pRoot->pUser) != XGE_XUI_EVENT_CONSUMED || xgeXuiPopupIsOpen(pPagePopup) != 0 ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 627;
	}
	xgeXuiPageUnload(&tPage);
	if ( xgeXuiPageLoadMemory(&tXui, sPopupCloseXson, (int)strlen(sPopupCloseXson), &tBinder, &tPage) == XGE_OK ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 628;
	}
	sError = xgeXuiPageGetError(&tPage);
	if ( (strstr(sError, "onClose is not supported yet") == NULL) || (strstr(sError, "onClose") == NULL) ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 629;
	}
	xgeXuiPageUnload(&tPage);
	if ( xgeXuiPageLoadMemory(&tXui, sTooltipXson, (int)strlen(sTooltipXson), &tBinder, &tPage) != XGE_OK ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 630;
	}
	pChild = xgeXuiPageFind(&tPage, "tip-owner");
	pRoot = xgeXuiPageFind(&tPage, "tip");
	if ( pChild == NULL || pRoot == NULL || tPage.iTooltipCount != 1 || pChild->procCaptureEvent != xgeXuiTooltipOwnerEventProc ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 631;
	}
	pPageTooltip = (xge_xui_tooltip)pChild->pCaptureUser;
	if ( pPageTooltip == NULL || pPageTooltip != tPage.arrTooltip[0] || pPageTooltip->pOwner != pChild || pPageTooltip->pFont != &tFont || strcmp(pPageTooltip->sText, "Tip") != 0 || xgeXuiTooltipIsOpen(pPageTooltip) != 1 || pPageTooltip->fOffsetX != 5.0f || pPageTooltip->fOffsetY != 3.0f || pPageTooltip->iBackgroundColor != XGE_COLOR_RGBA(4, 5, 6, 255) || pPageTooltip->iTextColor != XGE_COLOR_RGBA(7, 8, 9, 255) ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 632;
	}
	xgeXuiWidgetSetRect(pChild, (xge_rect_t){ 10.0f, 10.0f, 80.0f, 24.0f });
	xgeXuiTooltipSetOpen(pPageTooltip, 1);
	xgeXuiUpdate(&tXui, 0.0f);
	tRect = xgeXuiWidgetGetRect(pPageTooltip->pPopupWidget);
	if ( tRect.fX != 15.0f || tRect.fY != 37.0f || xgeXuiWidgetIsVisible(pPageTooltip->pPopupWidget) == 0 ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 633;
	}
	xgeXuiPageUnload(&tPage);
	if ( xgeXuiPageLoadMemory(&tXui, sTooltipCloseXson, (int)strlen(sTooltipCloseXson), &tBinder, &tPage) == XGE_OK ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 635;
	}
	sError = xgeXuiPageGetError(&tPage);
	if ( (strstr(sError, "onClose is not supported yet") == NULL) || (strstr(sError, "onClose") == NULL) ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 636;
	}
	xgeXuiPageUnload(&tPage);
	if ( xgeXuiPageLoadMemory(&tXui, sMenuXson, (int)strlen(sMenuXson), &tBinder, &tPage) != XGE_OK ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 637;
	}
	pChild = xgeXuiPageFind(&tPage, "menu-owner");
	pRoot = xgeXuiPageFind(&tPage, "file-menu");
	if ( pChild == NULL || pRoot == NULL || tPage.iMenuCount != 1 ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 638;
	}
	pPageMenu = tPage.arrMenu[0];
	if ( pPageMenu == NULL || tPage.arrMenuWidget[0] != pRoot || pPageMenu->pOwner != pChild || pPageMenu->pFont != &tFont || pPageMenu->iItemCount != 3 || strcmp(pPageMenu->arrItems[0], "Open") != 0 || strcmp(pPageMenu->arrItems[1], "Save") != 0 || strcmp(pPageMenu->arrItems[2], "Exit") != 0 || pPageMenu->iEnabledCount != 3 || pPageMenu->arrEnabled[0] != 1 || pPageMenu->arrEnabled[1] != 0 || pPageMenu->arrEnabled[2] != 1 ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 639;
	}
	if ( pPageMenu->fWidth != 120.0f || pPageMenu->fMaxHeight != 80.0f || pPageMenu->fItemHeight != 20.0f || pPageMenu->iBackgroundColor != XGE_COLOR_RGBA(0xF4, 0xFA, 0xFF, 0xFF) || pPageMenu->iRowColor != XGE_COLOR_RGBA(0xEA, 0xF6, 0xFD, 0xFF) || pPageMenu->iSelectedColor != XGE_COLOR_RGBA(0xC7, 0xE8, 0xF8, 0xFF) || pPageMenu->iTextColor != XGE_COLOR_RGBA(1, 2, 3, 255) || pPageMenu->iDisabledTextColor != XGE_COLOR_RGBA(0x80, 0x90, 0xA0, 0xFF) ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 640;
	}
	tRect = xgeXuiWidgetGetRect(pPageMenu->pPopupWidget);
	if ( xgeXuiMenuIsOpen(pPageMenu) != 1 || xgeXuiWidgetIsVisible(pPageMenu->pPopupWidget) == 0 || tXui.pFocus != pPageMenu->pListWidget || tRect.fX != 10.0f || tRect.fY != 12.0f || tRect.fW != 120.0f || tRect.fH != 60.0f ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 641;
	}
	xgeXuiPageUnload(&tPage);
	if ( xgeXuiPageLoadMemory(&tXui, sMenuSelectXson, (int)strlen(sMenuSelectXson), &tBinder, &tPage) == XGE_OK ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 642;
	}
	sError = xgeXuiPageGetError(&tPage);
	if ( (strstr(sError, "onSelect is not supported yet") == NULL) || (strstr(sError, "onSelect") == NULL) ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 643;
	}
	xgeXuiPageUnload(&tPage);
	if ( xgeXuiPageLoadMemory(&tXui, sDialogXson, (int)strlen(sDialogXson), &tBinder, &tPage) != XGE_OK ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 644;
	}
	pRoot = xgeXuiPageFind(&tPage, "dialog");
	if ( pRoot == NULL || pRoot->procEvent != xgeXuiDialogEventProc || pRoot->procPaint != xgeXuiDialogPaintProc || tPage.iDialogCount != 1 ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 645;
	}
	pPageDialog = (xge_xui_dialog)pRoot->pUser;
	if ( pPageDialog == NULL || pPageDialog != tPage.arrDialog[0] || pPageDialog->pFont != &tFont || strcmp(pPageDialog->sTitle, "Settings") != 0 || xgeXuiDialogIsOpen(pPageDialog) != 1 || pPageDialog->bModal != 0 || pPageDialog->bCloseOnEscape != 1 || pPageDialog->bShowClose != 1 ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 646;
	}
	if ( pPageDialog->iBackdropColor != 0x01020378u || pPageDialog->iBackgroundColor != XGE_COLOR_RGBA(0xF4, 0xFA, 0xFF, 0xFF) || pPageDialog->iTitleColor != XGE_COLOR_RGBA(7, 8, 9, 255) || pPageDialog->iCloseColor != XGE_COLOR_RGBA(10, 11, 12, 255) ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 647;
	}
	xgeXuiWidgetSetRect(pRoot, (xge_rect_t){ 20.0f, 20.0f, 160.0f, 90.0f });
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_MOUSE_DOWN;
	tEvent.fX = 400.0f;
	tEvent.fY = 400.0f;
	if ( pRoot->procEvent(pRoot, &tEvent, pRoot->pUser) != XGE_XUI_EVENT_CONTINUE || xgeXuiDialogIsOpen(pPageDialog) != 1 ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 648;
	}
	tEvent.iType = XGE_EVENT_KEY_DOWN;
	tEvent.iParam1 = XGE_KEY_ESCAPE;
	if ( pRoot->procEvent(pRoot, &tEvent, pRoot->pUser) != XGE_XUI_EVENT_CONSUMED || xgeXuiDialogIsOpen(pPageDialog) != 0 ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 649;
	}
	xgeXuiPageUnload(&tPage);
	if ( xgeXuiPageLoadMemory(&tXui, sDialogCloseXson, (int)strlen(sDialogCloseXson), &tBinder, &tPage) == XGE_OK ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 650;
	}
	sError = xgeXuiPageGetError(&tPage);
	if ( (strstr(sError, "onClose is not supported yet") == NULL) || (strstr(sError, "onClose") == NULL) ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 651;
	}
	xgeXuiPageUnload(&tPage);
	if ( xgeXuiPageLoadMemory(&tXui, sMessageBoxXson, (int)strlen(sMessageBoxXson), &tBinder, &tPage) != XGE_OK ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 652;
	}
	pRoot = xgeXuiPageFind(&tPage, "msg");
	if ( pRoot == NULL || pRoot->procEvent != xgeXuiMessageBoxEventProc || pRoot->procPaint != xgeXuiMessageBoxPaintProc || tPage.iMessageBoxCount != 1 ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 653;
	}
	pPageMessageBox = (xge_xui_message_box)pRoot->pUser;
	if ( pPageMessageBox == NULL || pPageMessageBox != tPage.arrMessageBox[0] || pPageMessageBox->pFont != &tFont || strcmp(pPageMessageBox->tDialog.sTitle, "Confirm") != 0 || strcmp(pPageMessageBox->sMessage, "Save changes?") != 0 || pPageMessageBox->iType != XGE_XUI_MESSAGE_BOX_QUESTION || pPageMessageBox->iButtons != XGE_XUI_MESSAGE_BOX_YES_NO_CANCEL || pPageMessageBox->iButtonCount != 3 || xgeXuiMessageBoxIsOpen(pPageMessageBox) != 1 ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 654;
	}
	if ( pPageMessageBox->tDialog.iBackdropColor != 0x01020378u || pPageMessageBox->tDialog.iBackgroundColor != XGE_COLOR_RGBA(0xF4, 0xFA, 0xFF, 0xFF) || pPageMessageBox->iMessageColor != XGE_COLOR_RGBA(0x11, 0x12, 0x13, 0xFF) || pPageMessageBox->iButtonColor != XGE_COLOR_RGBA(0xEA, 0xF6, 0xFD, 0xFF) || pPageMessageBox->iButtonHoverColor != XGE_COLOR_RGBA(0xC7, 0xE8, 0xF8, 0xFF) || pPageMessageBox->iButtonTextColor != XGE_COLOR_RGBA(1, 2, 3, 255) ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 655;
	}
	xgeXuiWidgetSetRect(pRoot, (xge_rect_t){ 20.0f, 20.0f, 220.0f, 120.0f });
	xgeXuiPaint(&tXui);
	tEvent.iType = XGE_EVENT_MOUSE_DOWN;
	tEvent.fX = pPageMessageBox->arrButtonRect[1].fX + 2.0f;
	tEvent.fY = pPageMessageBox->arrButtonRect[1].fY + 2.0f;
	if ( pRoot->procEvent(pRoot, &tEvent, pRoot->pUser) != XGE_XUI_EVENT_CONSUMED || xgeXuiMessageBoxIsOpen(pPageMessageBox) != 0 || xgeXuiMessageBoxGetResult(pPageMessageBox) != XGE_XUI_MESSAGE_BOX_RESULT_NO ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 656;
	}
	xgeXuiPageUnload(&tPage);
	if ( xgeXuiPageLoadMemory(&tXui, sMessageBoxResultXson, (int)strlen(sMessageBoxResultXson), &tBinder, &tPage) == XGE_OK ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 657;
	}
	sError = xgeXuiPageGetError(&tPage);
	if ( (strstr(sError, "onResult is not supported yet") == NULL) || (strstr(sError, "onResult") == NULL) ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 658;
	}
	xgeXuiPageUnload(&tPage);
	if ( xgeXuiPageLoadMemory(&tXui, sStyleCycleXson, (int)strlen(sStyleCycleXson), &tBinder, &tPage) == XGE_OK ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 520;
	}
	sError = xgeXuiPageGetError(&tPage);
	if ( (strstr(sError, "style parent cycle") == NULL) || (strstr(sError, "styles") == NULL) ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 521;
	}
	if ( tPage.pRoot != NULL || tPage.pDocument != NULL || tPage.tLoader.tResource.pData != NULL || tPage.iIndexCount != 0 ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 522;
	}
	xgeXuiPageUnload(&tPage);
	if ( xgeXuiPageLoadMemory(&tXui, sMissingTokenXson, (int)strlen(sMissingTokenXson), &tBinder, &tPage) == XGE_OK ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 525;
	}
	sError = xgeXuiPageGetError(&tPage);
	if ( (strstr(sError, "missing token") == NULL) || (strstr(sError, "background") == NULL) ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 526;
	}
	xgeXuiPageUnload(&tPage);
	if ( xgeXuiPageLoadMemory(&tXui, sMissingClickXson, (int)strlen(sMissingClickXson), &tBinder, &tPage) == XGE_OK ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 531;
	}
	sError = xgeXuiPageGetError(&tPage);
	if ( (strstr(sError, "unregistered click event") == NULL) || (strstr(sError, "onClick") == NULL) ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 532;
	}
	xgeXuiPageUnload(&tPage);
	if ( xgeXuiPageLoadMemory(&tXui, sScriptXson, (int)strlen(sScriptXson), &tBinder, &tPage) == XGE_OK ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 533;
	}
	sError = xgeXuiPageGetError(&tPage);
	if ( (strstr(sError, "scripts are not supported") == NULL) || (strstr(sError, "script") == NULL) ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 534;
	}
	xgeXuiPageUnload(&tPage);
	if ( xgeXuiPageLoadMemory(&tXui, sInputChangeXson, (int)strlen(sInputChangeXson), &tBinder, &tPage) == XGE_OK ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 552;
	}
	sError = xgeXuiPageGetError(&tPage);
	if ( (strstr(sError, "onChange is not supported yet") == NULL) || (strstr(sError, "onChange") == NULL) ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 553;
	}
	xgeXuiPageUnload(&tPage);
	if ( xgeXuiPageLoadMemory(&tXui, sUnknownTypeXson, (int)strlen(sUnknownTypeXson), &tBinder, &tPage) == XGE_OK ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 535;
	}
	sError = xgeXuiPageGetError(&tPage);
	if ( (strstr(sError, "unknown widget type") == NULL) || (strstr(sError, "tree") == NULL) ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 536;
	}
	xgeXuiPageUnload(&tPage);
	if ( xgeXuiPageLoadMemory(&tXui, sBadTypeXson, (int)strlen(sBadTypeXson), &tBinder, &tPage) == XGE_OK ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 537;
	}
	sError = xgeXuiPageGetError(&tPage);
	if ( (strstr(sError, "type must be string") == NULL) || (strstr(sError, "tree") == NULL) ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 538;
	}
	xgeXuiPageUnload(&tPage);
	memset(&tProvider, 0, sizeof(tProvider));
	tProvider.sScheme = "xui";
	tProvider.load = __testXuiPageProviderLoad;
	tProvider.free = __testProviderFree;
	if ( xgeResourceProviderAdd(&tProvider) != XGE_OK ) {
		xgeXuiUnit(&tXui);
		return 506;
	}
	g_iProviderFreeCount = 0;
	if ( xgeXuiPageLoad(&tXui, "xui://page", &tBinder, &tPage) != XGE_OK || tPage.tLoader.tResource.pData == NULL || xgeXuiPageFind(&tPage, "7") != xgeXuiPageRoot(&tPage) || xgeXuiPageFind(&tPage, "remote-child") == NULL ) {
		xgeResourceProviderClear();
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 507;
	}
	xgeXuiPageUnload(&tPage);
	if ( xgeXuiPageLoad(&tXui, "xui://bad", &tBinder, &tPage) == XGE_OK ) {
		xgeResourceProviderClear();
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 513;
	}
	sError = xgeXuiPageGetError(&tPage);
	if ( (strstr(sError, "xui://bad") == NULL) || (strstr(sError, "tree.children") == NULL) ) {
		xgeResourceProviderClear();
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 514;
	}
	if ( tPage.pRoot != NULL || tPage.pDocument != NULL || tPage.tLoader.tResource.pData != NULL || tPage.iIndexCount != 0 ) {
		xgeResourceProviderClear();
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 523;
	}
	xgeXuiPageUnload(&tPage);
	if ( xgeXuiPageLoad(&tXui, "xui://ui/page", &tBinder, &tPage) != XGE_OK ) {
		xgeResourceProviderClear();
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 515;
	}
	pRoot = xgeXuiPageRoot(&tPage);
	if ( pRoot == NULL || pRoot->tStyle.iLayout != XGE_XUI_LAYOUT_ROW || pRoot->tStyle.tWidth.fValue != 140.0f || pRoot->tStyle.tHeight.fValue != 64.0f || xgeXuiPageFind(&tPage, "ignored-import-tree") != NULL ) {
		xgeResourceProviderClear();
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 516;
	}
	xgeXuiPageUnload(&tPage);
	if ( xgeXuiPageLoad(&tXui, "xui://cycle/a", &tBinder, &tPage) == XGE_OK ) {
		xgeResourceProviderClear();
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 517;
	}
	sError = xgeXuiPageGetError(&tPage);
	if ( (strstr(sError, "xui://cycle/a") == NULL) || (strstr(sError, "import cycle") == NULL) ) {
		xgeResourceProviderClear();
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 518;
	}
	xgeXuiPageUnload(&tPage);
	xgeResourceProviderClear();
	if ( g_iProviderFreeCount != 7 ) {
		xgeXuiUnit(&tXui);
		return 508;
	}
	pRoot = xgeXuiWidgetCreate();
	pChild = xgeXuiWidgetCreate();
	if ( (pRoot == NULL) || (pChild == NULL) ) {
		xgeXuiWidgetFree(pRoot);
		xgeXuiWidgetFree(pChild);
		xgeXuiUnit(&tXui);
		return 509;
	}
	xgeXuiWidgetSetId(pRoot, 42);
	xgeXuiWidgetSetName(pRoot, "root");
	xgeXuiWidgetSetName(pChild, "child");
	xgeXuiWidgetAdd(pRoot, pChild);
	memset(&tPage, 0, sizeof(tPage));
	tPage.pRoot = pRoot;
	if ( xgeXuiPageFind(&tPage, "42") != pRoot || xgeXuiPageFind(&tPage, "child") != pChild || xgeXuiPageFind(&tPage, "missing") != NULL ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 510;
	}
	xgeXuiPageUnload(&tPage);
	xgeXuiUnit(&tXui);
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

	iRet = __testAsyncResources();
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

	iRet = __testXuiSearchBox();
	if ( iRet != 0 ) {
		xgeUnit();
		return iRet;
	}

	iRet = __testXuiColorPicker();
	if ( iRet != 0 ) {
		xgeUnit();
		return iRet;
	}

	iRet = __testXuiNumericInput();
	if ( iRet != 0 ) {
		xgeUnit();
		return iRet;
	}

	iRet = __testXuiPageApi();
	if ( iRet != 0 ) {
		xgeUnit();
		return iRet;
	}

	iRet = __testXuiButton();
	if ( iRet != 0 ) {
		xgeUnit();
		return iRet;
	}

	iRet = __testXuiIconButton();
	if ( iRet != 0 ) {
		xgeUnit();
		return iRet;
	}

	iRet = __testXuiToggle();
	if ( iRet != 0 ) {
		xgeUnit();
		return iRet;
	}

	iRet = __testXuiCheckBox();
	if ( iRet != 0 ) {
		xgeUnit();
		return iRet;
	}

	iRet = __testXuiRadio();
	if ( iRet != 0 ) {
		xgeUnit();
		return iRet;
	}

	iRet = __testXuiSwitch();
	if ( iRet != 0 ) {
		xgeUnit();
		return iRet;
	}

	iRet = __testXuiSeparator();
	if ( iRet != 0 ) {
		xgeUnit();
		return iRet;
	}

	iRet = __testXuiSplitter();
	if ( iRet != 0 ) {
		xgeUnit();
		return iRet;
	}

	iRet = __testXuiTabs();
	if ( iRet != 0 ) {
		xgeUnit();
		return iRet;
	}

	iRet = __testXuiToolbar();
	if ( iRet != 0 ) {
		xgeUnit();
		return iRet;
	}

	iRet = __testXuiStatusBar();
	if ( iRet != 0 ) {
		xgeUnit();
		return iRet;
	}

	iRet = __testXuiTreeView();
	if ( iRet != 0 ) {
		xgeUnit();
		return iRet;
	}

	iRet = __testXuiTableView();
	if ( iRet != 0 ) {
		xgeUnit();
		return iRet;
	}

	iRet = __testXuiPropertyGrid();
	if ( iRet != 0 ) {
		xgeUnit();
		return iRet;
	}

	iRet = __testXuiBreadcrumb();
	if ( iRet != 0 ) {
		xgeUnit();
		return iRet;
	}

	iRet = __testXuiAccordion();
	if ( iRet != 0 ) {
		xgeUnit();
		return iRet;
	}

	iRet = __testXuiToast();
	if ( iRet != 0 ) {
		xgeUnit();
		return iRet;
	}

	iRet = __testXuiScrollBar();
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

	iRet = __testXuiVirtualList();
	if ( iRet != 0 ) {
		xgeUnit();
		return iRet;
	}

	iRet = __testXuiComboBox();
	if ( iRet != 0 ) {
		xgeUnit();
		return iRet;
	}

	iRet = __testXuiMessageBox();
	if ( iRet != 0 ) {
		xgeUnit();
		return iRet;
	}

	iRet = __testXuiTooltip();
	if ( iRet != 0 ) {
		xgeUnit();
		return iRet;
	}

	iRet = __testXuiDialog();
	if ( iRet != 0 ) {
		xgeUnit();
		return iRet;
	}

	iRet = __testXuiPopup();
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

	iRet = __testXuiTextEdit();
	if ( iRet != 0 ) {
		xgeUnit();
		return iRet;
	}

	xgeUnit();
	return 0;
}

