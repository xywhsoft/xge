#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../xge.h"

typedef struct render_target_lab_t {
	xge_render_target_t tWindowTarget;
	xge_render_target_t tTarget;
	int iFrameLimit;
	double fSecondLimit;
	int iFrameCount;
	int bReady;
	int bDone;
	int bWindowOK;
	int bCreateOK;
	int bResizeOK;
	int bTextureOK;
	int bPassOK;
	int bReadbackOK;
	int iReadR;
	int iReadG;
	int iReadB;
	int iReadA;
} render_target_lab_t;

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

static int CheckPixelNear(int iValue, int iTarget)
{
	int iDelta;

	iDelta = iValue - iTarget;
	if ( iDelta < 0 ) {
		iDelta = -iDelta;
	}
	return iDelta <= 4;
}

static void DrawTargetTexture(render_target_lab_t* pLab)
{
	xge_draw_t tDraw;
	xge_texture pTexture;

	pTexture = xgeRenderTargetTexture(&pLab->tTarget);
	if ( pTexture == NULL ) {
		return;
	}
	memset(&tDraw, 0, sizeof(tDraw));
	tDraw.pTexture = pTexture;
	tDraw.tSrc.fX = 0.0f;
	tDraw.tSrc.fY = 0.0f;
	tDraw.tSrc.fW = (float)pTexture->iWidth;
	tDraw.tSrc.fH = (float)pTexture->iHeight;
	tDraw.tDst.fX = 188.0f;
	tDraw.tDst.fY = 108.0f;
	tDraw.tDst.fW = 256.0f;
	tDraw.tDst.fH = 192.0f;
	tDraw.iColor = XGE_COLOR_RGBA(255, 255, 255, 255);
	tDraw.iFlags = XGE_DRAW_SCREEN_SPACE;
	xgeDrawEx(&tDraw);
}

static int InitAndRenderTarget(render_target_lab_t* pLab)
{
	xge_pass_t tPass;
	unsigned char arrPixels[128 * 96 * 4];
	xge_texture pTexture;
	int iRet;

	if ( pLab->bReady ) {
		return XGE_OK;
	}
	iRet = xgeRenderTargetWindow(&pLab->tWindowTarget);
	pLab->bWindowOK = (iRet == XGE_OK) && ((pLab->tWindowTarget.iFlags & XGE_RENDER_TARGET_WINDOW) != 0) && (pLab->tWindowTarget.iWidth == xgeGetWidth());
	if ( !pLab->bWindowOK ) {
		fprintf(stderr, "render-target-lab stage failed: window target ret=%d size=%dx%d flags=0x%X\n", iRet, pLab->tWindowTarget.iWidth, pLab->tWindowTarget.iHeight, pLab->tWindowTarget.iFlags);
		return XGE_ERROR_GPU_FAILED;
	}
	iRet = xgeRenderTargetCreate(&pLab->tTarget, 96, 64);
	pLab->bCreateOK = (iRet == XGE_OK) && ((pLab->tTarget.iFlags & XGE_RENDER_TARGET_TEXTURE) != 0);
	if ( !pLab->bCreateOK ) {
		fprintf(stderr, "render-target-lab stage failed: create ret=%d\n", iRet);
		return XGE_ERROR_GPU_FAILED;
	}
	iRet = xgeRenderTargetResize(&pLab->tTarget, 128, 96);
	pLab->bResizeOK = (iRet == XGE_OK) && (pLab->tTarget.iWidth == 128) && (pLab->tTarget.iHeight == 96);
	if ( !pLab->bResizeOK ) {
		fprintf(stderr, "render-target-lab stage failed: resize ret=%d size=%dx%d\n", iRet, pLab->tTarget.iWidth, pLab->tTarget.iHeight);
		return XGE_ERROR_GPU_FAILED;
	}
	pTexture = xgeRenderTargetTexture(&pLab->tTarget);
	pLab->bTextureOK = (pTexture != NULL) && (pTexture->iWidth == 128) && (pTexture->iHeight == 96);
	if ( !pLab->bTextureOK ) {
		fprintf(stderr, "render-target-lab stage failed: texture=%p\n", (void*)pTexture);
		return XGE_ERROR_GPU_FAILED;
	}
	xgePassInit(&tPass, &pLab->tTarget, XGE_PASS_CLEAR_COLOR, XGE_COLOR_RGBA(32, 64, 96, 255));
	if ( xgePassBegin(&tPass) != XGE_OK ) {
		fprintf(stderr, "render-target-lab stage failed: pass begin\n");
		return XGE_ERROR_GPU_FAILED;
	}
	xgeShapeRectFillPx((xge_rect_t){ 48.0f, 32.0f, 48.0f, 32.0f }, XGE_COLOR_RGBA(220, 120, 60, 255));
	xgeShapeCircleFillPx(92.0f, 70.0f, 14.0f, XGE_COLOR_RGBA(88, 220, 150, 255));
	if ( xgePassEnd(&tPass) != XGE_OK ) {
		fprintf(stderr, "render-target-lab stage failed: pass end\n");
		return XGE_ERROR_GPU_FAILED;
	}
	pLab->bPassOK = 1;
	memset(arrPixels, 0, sizeof(arrPixels));
	iRet = xgeRenderTargetReadPixels(&pLab->tTarget, arrPixels, 128 * 4);
	if ( iRet != XGE_OK ) {
		fprintf(stderr, "render-target-lab stage failed: readback ret=%d\n", iRet);
		return XGE_ERROR_GPU_FAILED;
	}
	pLab->iReadR = arrPixels[0];
	pLab->iReadG = arrPixels[1];
	pLab->iReadB = arrPixels[2];
	pLab->iReadA = arrPixels[3];
	pLab->bReadbackOK = CheckPixelNear(pLab->iReadR, 32) && CheckPixelNear(pLab->iReadG, 64) && CheckPixelNear(pLab->iReadB, 96) && CheckPixelNear(pLab->iReadA, 255);
	if ( !pLab->bReadbackOK ) {
		fprintf(stderr, "render-target-lab stage failed: readback rgba=%d,%d,%d,%d\n", pLab->iReadR, pLab->iReadG, pLab->iReadB, pLab->iReadA);
		return XGE_ERROR_GPU_FAILED;
	}
	pLab->bReady = 1;
	printf("render-target-lab init window=%d create=%d resize=%d texture=%d pass=%d readback=%d rgba=%d,%d,%d,%d target=%dx%d\n",
		pLab->bWindowOK,
		pLab->bCreateOK,
		pLab->bResizeOK,
		pLab->bTextureOK,
		pLab->bPassOK,
		pLab->bReadbackOK,
		pLab->iReadR,
		pLab->iReadG,
		pLab->iReadB,
		pLab->iReadA,
		pLab->tTarget.iWidth,
		pLab->tTarget.iHeight);
	return XGE_OK;
}

static void CleanupLab(render_target_lab_t* pLab)
{
	xgeRenderTargetFree(&pLab->tTarget);
	xgeRenderTargetFree(&pLab->tWindowTarget);
	pLab->bReady = 0;
}

static int RenderTargetLabFrame(void* pUser)
{
	render_target_lab_t* pLab;
	int iRet;

	pLab = (render_target_lab_t*)pUser;
	pLab->iFrameCount++;
	iRet = InitAndRenderTarget(pLab);
	if ( iRet != XGE_OK ) {
		xgeQuit();
		return 1;
	}
	if ( xgeKeyDown(XGE_KEY_ESCAPE) ) {
		printf("render-target-lab esc-summary frames=%d readback=%d\n", pLab->iFrameCount, pLab->bReadbackOK);
		CleanupLab(pLab);
		xgeQuit();
		return 1;
	}
	xgeClear(XGE_COLOR_RGBA(12, 16, 22, 255));
	xgeShapeRectFillPx((xge_rect_t){ 46.0f, 42.0f, 548.0f, 324.0f }, XGE_COLOR_RGBA(28, 34, 44, 255));
	xgeShapeRectStrokePx((xge_rect_t){ 46.0f, 42.0f, 548.0f, 324.0f }, 2.0f, XGE_COLOR_RGBA(120, 148, 174, 255));
	DrawTargetTexture(pLab);
	xgeShapeRectStrokePx((xge_rect_t){ 188.0f, 108.0f, 256.0f, 192.0f }, 3.0f, XGE_COLOR_RGBA(255, 245, 210, 255));
	if ( ((pLab->iFrameLimit > 0) && (pLab->iFrameCount >= pLab->iFrameLimit)) || ((pLab->fSecondLimit > 0.0) && (xgeTimer() >= pLab->fSecondLimit)) ) {
		printf("render-target-lab final-summary frames=%d window=%d create=%d resize=%d texture=%d pass=%d readback=%d rgba=%d,%d,%d,%d target=%dx%d\n",
			pLab->iFrameCount,
			pLab->bWindowOK,
			pLab->bCreateOK,
			pLab->bResizeOK,
			pLab->bTextureOK,
			pLab->bPassOK,
			pLab->bReadbackOK,
			pLab->iReadR,
			pLab->iReadG,
			pLab->iReadB,
			pLab->iReadA,
			pLab->tTarget.iWidth,
			pLab->tTarget.iHeight);
		CleanupLab(pLab);
		xgeQuit();
		return 1;
	}
	return 0;
}

int main(int argc, char** argv)
{
	xge_desc_t tDesc;
	render_target_lab_t tLab;
	int i;

	memset(&tLab, 0, sizeof(tLab));
	tLab.iFrameLimit = ArgInt(getenv("XGE_RENDER_TARGET_FRAMES"), 0);
	tLab.fSecondLimit = ArgDouble(getenv("XGE_RENDER_TARGET_SECONDS"), 0.0);
	for ( i = 1; i < argc; i++ ) {
		if ( (strcmp(argv[i], "--frames") == 0) && ((i + 1) < argc) ) {
			tLab.iFrameLimit = ArgInt(argv[++i], tLab.iFrameLimit);
		} else if ( (strcmp(argv[i], "--seconds") == 0) && ((i + 1) < argc) ) {
			tLab.fSecondLimit = ArgDouble(argv[++i], tLab.fSecondLimit);
		}
	}
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iWidth = 640;
	tDesc.iHeight = 420;
	tDesc.sTitle = "XGE Render Target Lab";
	tDesc.iFlags = XGE_INIT_WINDOW | XGE_INIT_VSYNC;
	tDesc.iRunMode = XGE_RUN_GAME_LOOP;
	tDesc.iTargetFPS = 60;
	if ( xgeInit(&tDesc) != XGE_OK ) {
		return 1;
	}
	xgeRun(RenderTargetLabFrame, &tLab);
	CleanupLab(&tLab);
	xgeUnit();
	printf("render-target-lab summary frames=%d\n", tLab.iFrameCount);
	return 0;
}
