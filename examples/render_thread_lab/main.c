#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../xge.h"

typedef struct render_thread_lab_t {
	xge_font_t tFont;
	xge_render_thread_caps_t tCapsBefore;
	xge_render_thread_caps_t tCapsEnabled;
	xge_render_thread_caps_t tWindowCaps;
	xge_render_thread_caps_t tEGLCapsEnabled;
	xge_egl_caps_t tEGLCaps;
	int iFrameLimit;
	double fSecondLimit;
	int iFrameCount;
	int bWindowReady;
	int bWindowFreed;
	int bFontOK;
	int bManualInitOK;
	int bCapsBeforeOK;
	int bWorkerCapOK;
	int bTextureCreateOK;
	int bQueueRefOK;
	int bEnableOK;
	int bEnabledCapsOK;
	int bFlushOK;
	int bDisableOK;
	int bEGLCapsOK;
	int bEGLConfigOK;
	int bEGLEnableOK;
	int bEGLContextOwnedOK;
	int bEGLDisableOK;
	int bEGLClearOK;
	int bEGLSkipped;
	int bWindowCapsOK;
	int bWindowRejectOK;
	int iRenderThreadGetAfterEnable;
	int iRenderThreadGetAfterDisable;
	int iTextureRefQueued;
	int iTextureRefFlushed;
	int iWindowEnableRet;
	char sFontPath[260];
	char sEGLSkipReason[128];
} render_thread_lab_t;

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

static int ChooseFontPath(char* sPath, int iSize)
{
#if defined(_WIN32)
	static const char* arrCandidates[] = {
		"C:/Windows/Fonts/msyh.ttc",
		"C:/Windows/Fonts/segoeui.ttf",
		"C:/Windows/Fonts/arial.ttf"
	};
#else
	static const char* arrCandidates[] = {
		"/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
		"/usr/share/fonts/truetype/liberation2/LiberationSans-Regular.ttf"
	};
#endif
	FILE* pFile;
	int i;

	if ( (sPath == NULL) || (iSize <= 0) ) {
		return 0;
	}
	for ( i = 0; i < (int)(sizeof(arrCandidates) / sizeof(arrCandidates[0])); i++ ) {
		pFile = fopen(arrCandidates[i], "rb");
		if ( pFile != NULL ) {
			fclose(pFile);
			snprintf(sPath, iSize, "%s", arrCandidates[i]);
			return 1;
		}
	}
	sPath[0] = 0;
	return 0;
}

static void DrawTextBlock(xge_font pFont, const char* sText, xge_rect_t tRect)
{
	if ( (pFont == NULL) || (sText == NULL) ) {
		return;
	}
	xgeTextDrawRect(pFont, sText, tRect, XGE_COLOR_RGBA(232, 240, 246, 255), XGE_TEXT_ALIGN_LEFT | XGE_TEXT_ALIGN_TOP);
}

static void CleanupManualProbe(void)
{
	(void)xgeRenderThreadSet(0);
	(void)xgeRenderThreadEGLSet(NULL);
	xgeUnit();
}

static void DescribeEGLSkip(render_thread_lab_t* pLab, int iSetRet)
{
	if ( !pLab->bEGLCapsOK ) {
		snprintf(pLab->sEGLSkipReason, sizeof(pLab->sEGLSkipReason), "%s", "caps-failed");
		return;
	}
	if ( pLab->tEGLCaps.bCompiled == 0 ) {
		snprintf(pLab->sEGLSkipReason, sizeof(pLab->sEGLSkipReason), "%s", "egl-not-compiled");
		return;
	}
	if ( pLab->tEGLCaps.bPBuffer == 0 ) {
		snprintf(pLab->sEGLSkipReason, sizeof(pLab->sEGLSkipReason), "%s", "pbuffer-not-available");
		return;
	}
	snprintf(pLab->sEGLSkipReason, sizeof(pLab->sEGLSkipReason), "eglset-ret-%d", iSetRet);
}

static int RunManualProbe(render_thread_lab_t* pLab)
{
	xge_desc_t tDesc;
	xge_texture_t tTexture;
	xge_egl_desc_t tEGLDesc;
	uint32_t iPixel;
	int iRet;

	memset(&tDesc, 0, sizeof(tDesc));
	memset(&tTexture, 0, sizeof(tTexture));
	memset(&tEGLDesc, 0, sizeof(tEGLDesc));
	tDesc.iWidth = 320;
	tDesc.iHeight = 180;
	tDesc.sTitle = "XGE Render Thread Lab Manual Probe";
	tDesc.iFlags = 0;
	tDesc.iRunMode = XGE_RUN_MANUAL;
	tDesc.iTargetFPS = 60;
	iRet = xgeInit(&tDesc);
	pLab->bManualInitOK = (iRet == XGE_OK);
	if ( !pLab->bManualInitOK ) {
		return iRet;
	}

	pLab->bCapsBeforeOK = (xgeRenderThreadCapsGet(&pLab->tCapsBefore) == XGE_OK);
	pLab->bWorkerCapOK = pLab->bCapsBeforeOK &&
		pLab->tCapsBefore.bSupported &&
		pLab->tCapsBefore.bWorkerDrain &&
		pLab->tCapsBefore.bCanUseWithCurrentContext;
	if ( !pLab->bWorkerCapOK ) {
		CleanupManualProbe();
		return XGE_ERROR_UNSUPPORTED;
	}

	iPixel = XGE_COLOR_RGBA(255, 255, 255, 255);
	pLab->bTextureCreateOK = (xgeTextureCreateRGBA(&tTexture, 1, 1, &iPixel) == XGE_OK);
	if ( !pLab->bTextureCreateOK ) {
		CleanupManualProbe();
		return XGE_ERROR_RESOURCE_FAILED;
	}

	xgeDraw(&tTexture, 12.0f, 18.0f);
	pLab->iTextureRefQueued = tTexture.iRefCount;
	pLab->bQueueRefOK = (pLab->iTextureRefQueued == 2);

	pLab->bEnableOK = (xgeRenderThreadSet(1) == XGE_OK);
	pLab->iRenderThreadGetAfterEnable = xgeRenderThreadGet();
	pLab->bEnableOK = pLab->bEnableOK && (pLab->iRenderThreadGetAfterEnable != 0);
	pLab->bEnabledCapsOK = pLab->bEnableOK &&
		(xgeRenderThreadCapsGet(&pLab->tCapsEnabled) == XGE_OK) &&
		pLab->tCapsEnabled.bEnabled &&
		pLab->tCapsEnabled.bCanUseWithCurrentContext &&
		(pLab->tCapsEnabled.bAsyncFlush == 0);

	iRet = xgeFlush();
	pLab->iTextureRefFlushed = tTexture.iRefCount;
	pLab->bFlushOK = (iRet == XGE_OK) && (pLab->iTextureRefFlushed == 1);

	pLab->bDisableOK = (xgeRenderThreadSet(0) == XGE_OK);
	pLab->iRenderThreadGetAfterDisable = xgeRenderThreadGet();
	pLab->bDisableOK = pLab->bDisableOK && (pLab->iRenderThreadGetAfterDisable == 0);
	xgeTextureFree(&tTexture);

	pLab->bEGLCapsOK = (xgeEGLCapsGet(&pLab->tEGLCaps) == XGE_OK);
	tEGLDesc.iWidth = 32;
	tEGLDesc.iHeight = 32;
	tEGLDesc.bPBuffer = 1;
	iRet = xgeRenderThreadEGLSet(&tEGLDesc);
	if ( iRet == XGE_OK ) {
		pLab->bEGLConfigOK = 1;
		pLab->bEGLEnableOK = (xgeRenderThreadSet(1) == XGE_OK);
		pLab->bEGLContextOwnedOK = pLab->bEGLEnableOK &&
			(xgeRenderThreadCapsGet(&pLab->tEGLCapsEnabled) == XGE_OK) &&
			pLab->tEGLCapsEnabled.bEnabled &&
			pLab->tEGLCapsEnabled.bGLContextOwned;
		pLab->bEGLDisableOK = (xgeRenderThreadSet(0) == XGE_OK);
		pLab->bEGLClearOK = (xgeRenderThreadEGLSet(NULL) == XGE_OK);
		snprintf(pLab->sEGLSkipReason, sizeof(pLab->sEGLSkipReason), "%s", "none");
	} else if ( iRet == XGE_ERROR_UNSUPPORTED ) {
		pLab->bEGLSkipped = 1;
		DescribeEGLSkip(pLab, iRet);
		pLab->bEGLClearOK = (xgeRenderThreadEGLSet(NULL) == XGE_OK);
	} else {
		snprintf(pLab->sEGLSkipReason, sizeof(pLab->sEGLSkipReason), "eglset-ret-%d", iRet);
		CleanupManualProbe();
		return iRet;
	}

	printf("render-thread-lab init caps=%d worker=%d queue=%d enable=%d flush=%d disable=%d egl_caps=%d egl=%d/%d/%d/%d skip=%d reason=%s refs=%d/%d\n",
		pLab->bCapsBeforeOK,
		pLab->bWorkerCapOK,
		pLab->bQueueRefOK,
		pLab->bEnableOK && pLab->bEnabledCapsOK,
		pLab->bFlushOK,
		pLab->bDisableOK,
		pLab->bEGLCapsOK,
		pLab->bEGLConfigOK,
		pLab->bEGLEnableOK,
		pLab->bEGLContextOwnedOK,
		pLab->bEGLClearOK,
		pLab->bEGLSkipped,
		pLab->sEGLSkipReason[0] ? pLab->sEGLSkipReason : "none",
		pLab->iTextureRefQueued,
		pLab->iTextureRefFlushed);
	CleanupManualProbe();
	return XGE_OK;
}

static int InitWindow(render_thread_lab_t* pLab)
{
	if ( pLab->bWindowReady ) {
		return XGE_OK;
	}
	pLab->bWindowCapsOK = (xgeRenderThreadCapsGet(&pLab->tWindowCaps) == XGE_OK);
	if ( ChooseFontPath(pLab->sFontPath, (int)sizeof(pLab->sFontPath)) ) {
		pLab->bFontOK = (xgeFontLoad(&pLab->tFont, pLab->sFontPath, 18.0f) == XGE_OK);
	}
	pLab->iWindowEnableRet = xgeRenderThreadSet(1);
	pLab->bWindowRejectOK = (pLab->iWindowEnableRet == XGE_ERROR_UNSUPPORTED) &&
		pLab->bWindowCapsOK &&
		(pLab->tWindowCaps.bCanUseWithCurrentContext == 0);
	if ( (pLab->iWindowEnableRet == XGE_OK) && xgeRenderThreadGet() ) {
		(void)xgeRenderThreadSet(0);
	}
	pLab->bWindowReady = 1;
	printf("render-thread-lab window caps=%d current=%d enabled=%d reject=%d ret=%d\n",
		pLab->bWindowCapsOK,
		pLab->bWindowCapsOK ? pLab->tWindowCaps.bCanUseWithCurrentContext : 0,
		pLab->bWindowCapsOK ? pLab->tWindowCaps.bEnabled : 0,
		pLab->bWindowRejectOK,
		pLab->iWindowEnableRet);
	return XGE_OK;
}

static void CleanupWindow(render_thread_lab_t* pLab)
{
	if ( pLab->bWindowFreed ) {
		return;
	}
	if ( pLab->bFontOK ) {
		xgeFontFree(&pLab->tFont);
	}
	pLab->bWindowFreed = 1;
	pLab->bWindowReady = 0;
}

static void DrawPanels(render_thread_lab_t* pLab)
{
	char sWorker[512];
	char sEGL[512];
	char sWindow[512];

	snprintf(sWorker, sizeof(sWorker),
		"manual caps=%d\nworker=%d\nenable=%d\nflush=%d\ndisable=%d\nrefs=%d -> %d",
		pLab->bCapsBeforeOK,
		pLab->bWorkerCapOK,
		pLab->bEnableOK && pLab->bEnabledCapsOK,
		pLab->bFlushOK,
		pLab->bDisableOK,
		pLab->iTextureRefQueued,
		pLab->iTextureRefFlushed);
	snprintf(sEGL, sizeof(sEGL),
		"egl caps=%d\nconfig=%d\nenable=%d\nowned=%d\nclear=%d\nskip=%d\n%s",
		pLab->bEGLCapsOK,
		pLab->bEGLConfigOK,
		pLab->bEGLEnableOK,
		pLab->bEGLContextOwnedOK,
		pLab->bEGLClearOK,
		pLab->bEGLSkipped,
		pLab->sEGLSkipReason[0] ? pLab->sEGLSkipReason : "none");
	snprintf(sWindow, sizeof(sWindow),
		"window caps=%d\ncurrent=%d\nenabled=%d\nreject=%d\nret=%d\nfont=%d",
		pLab->bWindowCapsOK,
		pLab->bWindowCapsOK ? pLab->tWindowCaps.bCanUseWithCurrentContext : 0,
		pLab->bWindowCapsOK ? pLab->tWindowCaps.bEnabled : 0,
		pLab->bWindowRejectOK,
		pLab->iWindowEnableRet,
		pLab->bFontOK);

	xgeShapeRectFillPx((xge_rect_t){ 24.0f, 28.0f, 212.0f, 210.0f }, XGE_COLOR_RGBA(30, 36, 42, 255));
	xgeShapeRectFillPx((xge_rect_t){ 252.0f, 28.0f, 212.0f, 210.0f }, XGE_COLOR_RGBA(30, 36, 42, 255));
	xgeShapeRectFillPx((xge_rect_t){ 480.0f, 28.0f, 216.0f, 210.0f }, XGE_COLOR_RGBA(30, 36, 42, 255));
	xgeShapeRectStrokePx((xge_rect_t){ 24.0f, 28.0f, 212.0f, 210.0f }, 2.0f, XGE_COLOR_RGBA(106, 124, 140, 255));
	xgeShapeRectStrokePx((xge_rect_t){ 252.0f, 28.0f, 212.0f, 210.0f }, 2.0f, XGE_COLOR_RGBA(106, 124, 140, 255));
	xgeShapeRectStrokePx((xge_rect_t){ 480.0f, 28.0f, 216.0f, 210.0f }, 2.0f, XGE_COLOR_RGBA(106, 124, 140, 255));

	xgeShapeRectFillPx((xge_rect_t){ 46.0f, 196.0f, pLab->bFlushOK ? 168.0f : 36.0f, 14.0f }, XGE_COLOR_RGBA(86, 176, 255, 255));
	xgeShapeRectFillPx((xge_rect_t){ 274.0f, 196.0f, pLab->bEGLConfigOK ? 168.0f : (pLab->bEGLSkipped ? 84.0f : 36.0f), 14.0f }, XGE_COLOR_RGBA(126, 218, 132, 255));
	xgeShapeRectFillPx((xge_rect_t){ 504.0f, 196.0f, pLab->bWindowRejectOK ? 168.0f : 36.0f, 14.0f }, XGE_COLOR_RGBA(255, 168, 92, 255));

	xgeShapeCircleFillPx(88.0f, 258.0f, 12.0f, (pLab->bWorkerCapOK && pLab->bFlushOK) ? XGE_COLOR_RGBA(86, 176, 255, 255) : XGE_COLOR_RGBA(180, 74, 74, 255));
	xgeShapeCircleFillPx(360.0f, 258.0f, 12.0f, (pLab->bEGLConfigOK || pLab->bEGLSkipped) ? XGE_COLOR_RGBA(126, 218, 132, 255) : XGE_COLOR_RGBA(180, 74, 74, 255));
	xgeShapeCircleFillPx(632.0f, 258.0f, 12.0f, pLab->bWindowRejectOK ? XGE_COLOR_RGBA(255, 168, 92, 255) : XGE_COLOR_RGBA(180, 74, 74, 255));

	if ( pLab->bFontOK ) {
		DrawTextBlock(&pLab->tFont, sWorker, (xge_rect_t){ 38.0f, 42.0f, 184.0f, 140.0f });
		DrawTextBlock(&pLab->tFont, sEGL, (xge_rect_t){ 266.0f, 42.0f, 184.0f, 140.0f });
		DrawTextBlock(&pLab->tFont, sWindow, (xge_rect_t){ 494.0f, 42.0f, 188.0f, 140.0f });
		DrawTextBlock(&pLab->tFont,
			"left: worker drain on manual context\ncenter: EGL-owned GL context path\nright: current Sokol window context should reject render thread",
			(xge_rect_t){ 28.0f, 286.0f, 668.0f, 54.0f });
	}
}

static void PrintFinalSummary(render_thread_lab_t* pLab)
{
	printf("render-thread-lab final-summary frames=%d worker=%d/%d/%d/%d/%d egl=%d/%d/%d/%d/%d skip=%d reason=%s window=%d/%d current=%d ret=%d refs=%d/%d\n",
		pLab->iFrameCount,
		pLab->bCapsBeforeOK,
		pLab->bEnableOK,
		pLab->bEnabledCapsOK,
		pLab->bFlushOK,
		pLab->bDisableOK,
		pLab->bEGLCapsOK,
		pLab->bEGLConfigOK,
		pLab->bEGLEnableOK,
		pLab->bEGLContextOwnedOK,
		pLab->bEGLClearOK,
		pLab->bEGLSkipped,
		pLab->sEGLSkipReason[0] ? pLab->sEGLSkipReason : "none",
		pLab->bWindowCapsOK,
		pLab->bWindowRejectOK,
		pLab->bWindowCapsOK ? pLab->tWindowCaps.bCanUseWithCurrentContext : 0,
		pLab->iWindowEnableRet,
		pLab->iTextureRefQueued,
		pLab->iTextureRefFlushed);
}

static int RenderThreadFrame(void* pUser)
{
	render_thread_lab_t* pLab;
	int iRet;

	pLab = (render_thread_lab_t*)pUser;
	pLab->iFrameCount++;
	iRet = InitWindow(pLab);
	if ( iRet != XGE_OK ) {
		xgeQuit();
		return 1;
	}
	if ( xgeKeyDown(XGE_KEY_ESCAPE) ) {
		CleanupWindow(pLab);
		PrintFinalSummary(pLab);
		xgeQuit();
		return 1;
	}

	xgeClear(XGE_COLOR_RGBA(18, 22, 28, 255));
	DrawPanels(pLab);

	if ( ((pLab->iFrameLimit > 0) && (pLab->iFrameCount >= pLab->iFrameLimit)) || ((pLab->fSecondLimit > 0.0) && (xgeTimer() >= pLab->fSecondLimit)) ) {
		CleanupWindow(pLab);
		PrintFinalSummary(pLab);
		xgeQuit();
		return 1;
	}
	return 0;
}

int main(int argc, char** argv)
{
	xge_desc_t tDesc;
	render_thread_lab_t tLab;
	int i;
	int iRet;

	memset(&tLab, 0, sizeof(tLab));
	tLab.iFrameLimit = ArgInt(getenv("XGE_RENDER_THREAD_FRAMES"), 0);
	tLab.fSecondLimit = ArgDouble(getenv("XGE_RENDER_THREAD_SECONDS"), 0.0);
	for ( i = 1; i < argc; i++ ) {
		if ( (strcmp(argv[i], "--frames") == 0) && ((i + 1) < argc) ) {
			tLab.iFrameLimit = ArgInt(argv[++i], tLab.iFrameLimit);
		} else if ( (strcmp(argv[i], "--seconds") == 0) && ((i + 1) < argc) ) {
			tLab.fSecondLimit = ArgDouble(argv[++i], tLab.fSecondLimit);
		}
	}

	iRet = RunManualProbe(&tLab);
	if ( iRet != XGE_OK ) {
		fprintf(stderr, "render-thread-lab manual probe failed: %d\n", iRet);
		return 1;
	}

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iWidth = 720;
	tDesc.iHeight = 360;
	tDesc.sTitle = "XGE Render Thread Lab";
	tDesc.iFlags = XGE_INIT_WINDOW | XGE_INIT_VSYNC;
	tDesc.iRunMode = XGE_RUN_GAME_LOOP;
	tDesc.iTargetFPS = 60;
	iRet = xgeInit(&tDesc);
	if ( iRet != XGE_OK ) {
		fprintf(stderr, "render-thread-lab window init failed: %d\n", iRet);
		return 2;
	}
	iRet = xgeRun(RenderThreadFrame, &tLab);
	CleanupWindow(&tLab);
	xgeUnit();
	if ( iRet != XGE_OK ) {
		fprintf(stderr, "render-thread-lab window loop failed: %d\n", iRet);
		return 3;
	}
	printf("render-thread-lab summary frames=%d\n", tLab.iFrameCount);
	return 0;
}
