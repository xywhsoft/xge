#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../xge.h"

typedef struct offscreen_egl_lab_t {
	xge_offscreen_t tOffscreen;
	xge_egl_caps_t tCaps;
	xge_egl_context_t tProbeEGL;
	xge_texture pPreview;
	int iFrameLimit;
	double fSecondLimit;
	int iFrameCount;
	int bReady;
	int bFreed;
	int bCapsOK;
	int bEGLInitOK;
	int bEGLMakeCurrentOK;
	int bEGLUnitOK;
	int bEGLSkipped;
	int bOffscreenInitOK;
	int bOffscreenTargetOK;
	int bOffscreenReadbackOK;
	int bOffscreenUnitOK;
	int bFallbackOK;
	unsigned char arrReadback[64 * 64 * 4];
	char sSkipReason[128];
} offscreen_egl_lab_t;

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

static void ProbeEGL(offscreen_egl_lab_t* pLab)
{
	xge_egl_desc_t tDesc;
	int iRet;

	memset(&pLab->tCaps, 0, sizeof(pLab->tCaps));
	memset(&pLab->tProbeEGL, 0, sizeof(pLab->tProbeEGL));
	pLab->bCapsOK = (xgeEGLCapsGet(&pLab->tCaps) == XGE_OK);
	if ( !pLab->bCapsOK ) {
		snprintf(pLab->sSkipReason, sizeof(pLab->sSkipReason), "%s", "caps-failed");
		pLab->bEGLSkipped = 1;
		pLab->bEGLUnitOK = 1;
		return;
	}
	if ( (pLab->tCaps.bCompiled == 0) || (pLab->tCaps.bPBuffer == 0) ) {
		snprintf(pLab->sSkipReason, sizeof(pLab->sSkipReason), "%s", (pLab->tCaps.bCompiled == 0) ? "egl-not-compiled" : "pbuffer-not-available");
		pLab->bEGLSkipped = 1;
		xgeEGLUnit(&pLab->tProbeEGL);
		pLab->bEGLUnitOK = 1;
		return;
	}
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iWidth = 32;
	tDesc.iHeight = 32;
	tDesc.bPBuffer = 1;
	iRet = xgeEGLInit(&pLab->tProbeEGL, &tDesc);
	pLab->bEGLInitOK = (iRet == XGE_OK);
	if ( !pLab->bEGLInitOK ) {
		snprintf(pLab->sSkipReason, sizeof(pLab->sSkipReason), "eglinit-%s", pLab->tProbeEGL.sLastStage);
		xgeEGLUnit(&pLab->tProbeEGL);
		pLab->bEGLUnitOK = 1;
		return;
	}
	iRet = xgeEGLMakeCurrent(&pLab->tProbeEGL);
	pLab->bEGLMakeCurrentOK = (iRet == XGE_OK);
	if ( !pLab->bEGLMakeCurrentOK ) {
		snprintf(pLab->sSkipReason, sizeof(pLab->sSkipReason), "makecurrent-%s", pLab->tProbeEGL.sLastStage);
	}
	xgeEGLUnit(&pLab->tProbeEGL);
	pLab->bEGLUnitOK = 1;
	if ( pLab->bEGLMakeCurrentOK ) {
		snprintf(pLab->sSkipReason, sizeof(pLab->sSkipReason), "%s", "none");
	}
}

static int InitOffscreen(offscreen_egl_lab_t* pLab)
{
	xge_pass_t tPass;
	xge_render_target pTarget;
	int iRet;
	int iPos;

	if ( pLab->bReady ) {
		return XGE_OK;
	}
	iRet = xgeOffscreenInit(&pLab->tOffscreen, 64, 64);
	pLab->bOffscreenInitOK = (iRet == XGE_OK);
	if ( !pLab->bOffscreenInitOK ) {
		fprintf(stderr, "offscreen-egl-lab stage failed: offscreen init ret=%d\n", iRet);
		return iRet;
	}
	pLab->bFallbackOK = pLab->tOffscreen.bFallbackRenderTarget ? 1 : 0;
	pTarget = xgeOffscreenRenderTarget(&pLab->tOffscreen);
	pLab->bOffscreenTargetOK = (pTarget != NULL);
	if ( !pLab->bOffscreenTargetOK ) {
		fprintf(stderr, "offscreen-egl-lab stage failed: target null\n");
		return XGE_ERROR_RESOURCE_FAILED;
	}
	xgePassInit(&tPass, pTarget, XGE_PASS_CLEAR_COLOR, XGE_COLOR_RGBA(34, 68, 102, 255));
	iRet = xgePassBegin(&tPass);
	if ( iRet != XGE_OK ) {
		fprintf(stderr, "offscreen-egl-lab stage failed: pass begin ret=%d\n", iRet);
		return iRet;
	}
	xgePassEnd(&tPass);
	memset(pLab->arrReadback, 0, sizeof(pLab->arrReadback));
	iRet = xgeOffscreenReadPixels(&pLab->tOffscreen, pLab->arrReadback, 64 * 4);
	pLab->bOffscreenReadbackOK = (iRet == XGE_OK);
	if ( !pLab->bOffscreenReadbackOK ) {
		fprintf(stderr, "offscreen-egl-lab stage failed: readback ret=%d\n", iRet);
		return iRet;
	}
	iPos = ((0 * 64) + 0) * 4;
	pLab->bOffscreenReadbackOK = (pLab->arrReadback[iPos + 0] == 34) &&
		(pLab->arrReadback[iPos + 1] == 68) &&
		(pLab->arrReadback[iPos + 2] == 102) &&
		(pLab->arrReadback[iPos + 3] == 255);
	pLab->pPreview = xgeRenderTargetTexture(pTarget);
	pLab->bReady = 1;
	printf("offscreen-egl-lab init caps=%d egl=%d/%d/%d skip=%d reason=%s offscreen=%d target=%d fallback=%d readback=%d\n",
		pLab->bCapsOK,
		pLab->bEGLInitOK,
		pLab->bEGLMakeCurrentOK,
		pLab->bEGLUnitOK,
		pLab->bEGLSkipped,
		pLab->sSkipReason[0] ? pLab->sSkipReason : "none",
		pLab->bOffscreenInitOK,
		pLab->bOffscreenTargetOK,
		pLab->bFallbackOK,
		pLab->bOffscreenReadbackOK);
	return XGE_OK;
}

static void CleanupLab(offscreen_egl_lab_t* pLab)
{
	if ( pLab->bFreed ) {
		return;
	}
	xgeOffscreenUnit(&pLab->tOffscreen);
	pLab->bOffscreenUnitOK = 1;
	pLab->bFreed = 1;
	pLab->bReady = 0;
}

static void PrintFinalSummary(offscreen_egl_lab_t* pLab)
{
	printf("offscreen-egl-lab final-summary frames=%d caps=%d egl=%d/%d/%d skip=%d reason=%s offscreen=%d target=%d readback=%d fallback=%d rgba=%u,%u,%u,%u unit=%d\n",
		pLab->iFrameCount,
		pLab->bCapsOK,
		pLab->bEGLInitOK,
		pLab->bEGLMakeCurrentOK,
		pLab->bEGLUnitOK,
		pLab->bEGLSkipped,
		pLab->sSkipReason[0] ? pLab->sSkipReason : "none",
		pLab->bOffscreenInitOK,
		pLab->bOffscreenTargetOK,
		pLab->bOffscreenReadbackOK,
		pLab->bFallbackOK,
		(unsigned int)pLab->arrReadback[0],
		(unsigned int)pLab->arrReadback[1],
		(unsigned int)pLab->arrReadback[2],
		(unsigned int)pLab->arrReadback[3],
		pLab->bOffscreenUnitOK);
}

static int OffscreenEGLFrame(void* pUser)
{
	offscreen_egl_lab_t* pLab;
	xge_draw_t tDraw;
	int iRet;

	pLab = (offscreen_egl_lab_t*)pUser;
	pLab->iFrameCount++;
	iRet = InitOffscreen(pLab);
	if ( iRet != XGE_OK ) {
		xgeQuit();
		return 1;
	}
	if ( xgeKeyDown(XGE_KEY_ESCAPE) ) {
		CleanupLab(pLab);
		PrintFinalSummary(pLab);
		xgeQuit();
		return 1;
	}
	xgeClear(XGE_COLOR_RGBA(18, 22, 28, 255));
	xgeShapeRectFillPx((xge_rect_t){ 40.0f, 42.0f, 280.0f, 220.0f }, XGE_COLOR_RGBA(30, 36, 44, 255));
	xgeShapeRectStrokePx((xge_rect_t){ 40.0f, 42.0f, 280.0f, 220.0f }, 2.0f, XGE_COLOR_RGBA(110, 126, 144, 255));
	if ( pLab->pPreview != NULL ) {
		memset(&tDraw, 0, sizeof(tDraw));
		tDraw.pTexture = pLab->pPreview;
		tDraw.tSrc.fW = (float)pLab->pPreview->iWidth;
		tDraw.tSrc.fH = (float)pLab->pPreview->iHeight;
		tDraw.tDst.fX = 110.0f;
		tDraw.tDst.fY = 86.0f;
		tDraw.tDst.fW = 144.0f;
		tDraw.tDst.fH = 144.0f;
		tDraw.iColor = XGE_COLOR_RGBA(255, 255, 255, 255);
		tDraw.iFlags = XGE_DRAW_SCREEN_SPACE;
		xgeDrawEx(&tDraw);
	}
	if ( ((pLab->iFrameLimit > 0) && (pLab->iFrameCount >= pLab->iFrameLimit)) || ((pLab->fSecondLimit > 0.0) && (xgeTimer() >= pLab->fSecondLimit)) ) {
		CleanupLab(pLab);
		PrintFinalSummary(pLab);
		xgeQuit();
		return 1;
	}
	return 0;
}

int main(int argc, char** argv)
{
	xge_desc_t tDesc;
	offscreen_egl_lab_t tLab;
	int i;

	memset(&tLab, 0, sizeof(tLab));
	tLab.iFrameLimit = ArgInt(getenv("XGE_OFFSCREEN_EGL_FRAMES"), 0);
	tLab.fSecondLimit = ArgDouble(getenv("XGE_OFFSCREEN_EGL_SECONDS"), 0.0);
	for ( i = 1; i < argc; i++ ) {
		if ( (strcmp(argv[i], "--frames") == 0) && ((i + 1) < argc) ) {
			tLab.iFrameLimit = ArgInt(argv[++i], tLab.iFrameLimit);
		} else if ( (strcmp(argv[i], "--seconds") == 0) && ((i + 1) < argc) ) {
			tLab.fSecondLimit = ArgDouble(argv[++i], tLab.fSecondLimit);
		}
	}
	ProbeEGL(&tLab);
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iWidth = 360;
	tDesc.iHeight = 300;
	tDesc.sTitle = "XGE Offscreen EGL Lab";
	tDesc.iFlags = XGE_INIT_WINDOW | XGE_INIT_VSYNC;
	tDesc.iRunMode = XGE_RUN_GAME_LOOP;
	tDesc.iTargetFPS = 60;
	if ( xgeInit(&tDesc) != XGE_OK ) {
		return 1;
	}
	xgeRun(OffscreenEGLFrame, &tLab);
	CleanupLab(&tLab);
	xgeUnit();
	printf("offscreen-egl-lab summary frames=%d\n", tLab.iFrameCount);
	return 0;
}
