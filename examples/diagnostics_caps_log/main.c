#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../xge.h"

typedef struct diagnostics_state_t {
	int iFrameLimit;
	double fSecondLimit;
	int iFrameCount;
	int bCapsDumped;
} diagnostics_state_t;

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

static const char* SafeName(const char* sText)
{
	return (sText != NULL) ? sText : "";
}

static void FlattenText(char* sText)
{
	int i;

	if ( sText == NULL ) {
		return;
	}
	for ( i = 0; sText[i] != 0; i++ ) {
		if ( (sText[i] == '\r') || (sText[i] == '\n') || (sText[i] == '\t') ) {
			sText[i] = ' ';
		}
	}
}

static void PrintBackendSummary(const char* sPrefix)
{
	xge_platform_backend_t tPlatform;
	xge_graphics_backend_t tGraphics;
	xge_platform_caps_t tPlatformCaps;
	xge_graphics_mapping_t tMapping;
	char arrHeader[128];
	char arrLibrary[128];

	tPlatform = xgePlatformBackendGet();
	tGraphics = xgeGraphicsBackendGet();
	memset(&tPlatformCaps, 0, sizeof(tPlatformCaps));
	memset(&tMapping, 0, sizeof(tMapping));
	memset(arrHeader, 0, sizeof(arrHeader));
	memset(arrLibrary, 0, sizeof(arrLibrary));
	(void)xgePlatformCapsGet(&tPlatformCaps);
	(void)xgeGraphicsMappingGet(XGE_GPU_BACKEND_NONE, &tMapping);
	(void)xgeGraphicsShaderHeaderGet(XGE_GPU_BACKEND_NONE, arrHeader, (int)sizeof(arrHeader));
	(void)xgeGraphicsLibraryNameGet(XGE_GPU_BACKEND_NONE, 0, arrLibrary, (int)sizeof(arrLibrary));
	FlattenText(arrHeader);
	FlattenText(arrLibrary);
	printf("%s platform=%s(%d) graphics=%s(%d) caps_platform=%s caps_graphics=%s target=%s window=%d offscreen=%d rt=%d audio=%d resource=%d highdpi=%d shader_header=%s library0=%s mapping(core=%d gles=%d webgl=%d vao=%d fbo=%d rgba8=0x%X)\n",
		sPrefix,
		SafeName(tPlatform.sName),
		tPlatform.iType,
		SafeName(tGraphics.sName),
		tGraphics.iType,
		tPlatformCaps.sPlatformName,
		tPlatformCaps.sGraphicsName,
		tPlatformCaps.sSokolTargetName,
		tPlatformCaps.bWindow,
		tPlatformCaps.bOffscreen,
		tPlatformCaps.bRenderTarget,
		tPlatformCaps.bAudio,
		tPlatformCaps.bResourceProvider,
		tPlatformCaps.bHighDPI,
		arrHeader,
		arrLibrary,
		tMapping.bOpenGLCore,
		tMapping.bGLES,
		tMapping.bWebGL,
		tMapping.bVAO,
		tMapping.bFramebufferObject,
		tMapping.iRGBA8InternalFormat);
}

static void PrintRuntimeSummary(const char* sPrefix)
{
	xge_platform_runtime_t tRuntime;
	xge_gpu_caps_t tGpu;
	xge_debug_stats_t tDebug;

	memset(&tRuntime, 0, sizeof(tRuntime));
	memset(&tGpu, 0, sizeof(tGpu));
	memset(&tDebug, 0, sizeof(tDebug));
	(void)xgePlatformRuntimeGet(&tRuntime);
	(void)xgeGpuCapsGet(&tGpu);
	(void)xgeDebugGetStats(&tDebug);
	printf("%s running=%d window=%dx%d framebuffer=%dx%d dpi=%.2f key=%d text=%d mouse=%d touch=%d gamepad=%d resize=%d quit=%d gpu=%d.%d max_texture=%d vendor=%s renderer=%s version=%s glsl=%s textures=%d texmem=%llu fonts=%d audio=%d draw=%d batch=%d dirty=%d\n",
		sPrefix,
		tRuntime.bRunning,
		tRuntime.iWindowWidth,
		tRuntime.iWindowHeight,
		tRuntime.iFramebufferWidth,
		tRuntime.iFramebufferHeight,
		tRuntime.fDpiScale,
		tRuntime.iKeyEventCount,
		tRuntime.iTextEventCount,
		tRuntime.iMouseEventCount,
		tRuntime.iTouchEventCount,
		tRuntime.iGamepadEventCount,
		tRuntime.iResizeEventCount,
		tRuntime.iQuitEventCount,
		tGpu.iMajorVersion,
		tGpu.iMinorVersion,
		tGpu.iMaxTextureSize,
		tGpu.sVendor,
		tGpu.sRenderer,
		tGpu.sVersion,
		tGpu.sShadingLanguage,
		tDebug.iTextureCount,
		(unsigned long long)tDebug.iTextureMemoryBytes,
		tDebug.iFontCount,
		tDebug.iAudioCount,
		tDebug.tFrame.iDrawCallCount,
		tDebug.tFrame.iBatchCount,
		tDebug.tFrame.iDirtyRectCount);
}

static int DiagnosticsFrame(void* pUser)
{
	diagnostics_state_t* pState;
	xge_rect_t tRect;
	char arrCaps[2048];

	pState = (diagnostics_state_t*)pUser;
	pState->iFrameCount++;
	if ( pState->bCapsDumped == 0 ) {
		memset(arrCaps, 0, sizeof(arrCaps));
		(void)xgeDebugDumpCaps(arrCaps, (int)sizeof(arrCaps));
		printf("%s\n", arrCaps);
		PrintBackendSummary("diagnostics backend");
		pState->bCapsDumped = 1;
	}
	if ( (pState->iFrameCount == 1) || ((pState->iFrameCount % 60) == 0) ) {
		PrintRuntimeSummary("diagnostics runtime");
	}
	if ( xgeKeyDown(XGE_KEY_ESCAPE) ) {
		PrintRuntimeSummary("diagnostics esc-summary");
		xgeQuit();
		return 1;
	}
	xgeClear(XGE_COLOR_RGBA(22, 30, 44, 255));
	tRect.fX = 28.0f;
	tRect.fY = 28.0f;
	tRect.fW = 260.0f;
	tRect.fH = 96.0f;
	xgeShapeRectFill(tRect, XGE_COLOR_RGBA(54, 120, 200, 255));
	xgeShapeCircleFill(360.0f, 76.0f, 42.0f, XGE_COLOR_RGBA(240, 180, 64, 235));
	if ( ((pState->iFrameLimit > 0) && (pState->iFrameCount >= pState->iFrameLimit)) || ((pState->fSecondLimit > 0.0) && (xgeTimer() >= pState->fSecondLimit)) ) {
		PrintRuntimeSummary("diagnostics final-summary");
		xgeLogWrite(XGE_LOG_INFO, "diagnostics", "final summary emitted");
		xgeLogFlush();
		xgeQuit();
		return 1;
	}
	return 0;
}

static void ProbeBackendSetGet(void)
{
	xge_platform_backend_t tPlatform;
	xge_graphics_backend_t tGraphics;

	tPlatform = xgePlatformBackendDefault();
	tGraphics = xgeGraphicsBackendDefault();
	(void)xgePlatformBackendSet(&tPlatform);
	(void)xgeGraphicsBackendSet(&tGraphics);
	PrintBackendSummary("diagnostics pre-init");
}

int main(int argc, char** argv)
{
	diagnostics_state_t tState;
	xge_desc_t tDesc;
	int i;
	int iRet;
	int iOldLogLevel;

	memset(&tState, 0, sizeof(tState));
	tState.iFrameLimit = ArgInt(getenv("XGE_DIAGNOSTICS_FRAMES"), 0);
	tState.fSecondLimit = ArgDouble(getenv("XGE_DIAGNOSTICS_SECONDS"), 0.0);
	for ( i = 1; i < argc; i++ ) {
		if ( (strcmp(argv[i], "--frames") == 0) && ((i + 1) < argc) ) {
			tState.iFrameLimit = ArgInt(argv[++i], tState.iFrameLimit);
		} else if ( (strcmp(argv[i], "--seconds") == 0) && ((i + 1) < argc) ) {
			tState.fSecondLimit = ArgDouble(argv[++i], tState.fSecondLimit);
		}
	}
	iOldLogLevel = xgeLogGetLevel();
	(void)xgeLogSetLevel(XGE_LOG_INFO);
	xgeLogWrite(XGE_LOG_INFO, "diagnostics", "starting diagnostics caps/log example");
	ProbeBackendSetGet();
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iWidth = 760;
	tDesc.iHeight = 420;
	tDesc.sTitle = "XGE Diagnostics Caps Log";
	tDesc.iFlags = XGE_INIT_WINDOW | XGE_INIT_VSYNC;
	tDesc.iRunMode = XGE_RUN_GAME_LOOP;
	tDesc.iTargetFPS = 60;
	iRet = xgeInit(&tDesc);
	if ( iRet != XGE_OK ) {
		fprintf(stderr, "diagnostics init failed: %d\n", iRet);
		(void)xgeLogSetLevel(iOldLogLevel);
		return 1;
	}
	xgeFrameStatsReset();
	iRet = xgeRun(DiagnosticsFrame, &tState);
	(void)xgeLogSetLevel(iOldLogLevel);
	xgeUnit();
	printf("diagnostics summary frames=%d log_level_restored=%d\n", tState.iFrameCount, iOldLogLevel);
	return (iRet == XGE_OK) ? 0 : 2;
}
