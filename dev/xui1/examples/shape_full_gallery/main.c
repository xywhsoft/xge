#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../xge.h"

typedef struct shape_full_gallery_t {
	xge_shape_batch_t tBatch;
	int iFrameLimit;
	double fSecondLimit;
	int iFrameCount;
	int bReady;
	int bFreed;
	int bBatchInitOK;
	int bBatchClearOK;
	int bBatchRectOK;
	int bBatchTriangleOK;
	int bBatchFlushOK;
	int bPixelOK;
	int bWorldOK;
	int bPointOK;
	int bLineOK;
	int bRectOK;
	int bCircleOK;
	int bArcOK;
	int bTriangleOK;
	int bPolygonOK;
	int bFillStrokeOK;
	int bAutoBatchOK;
	int bBatchPathOK;
	int iLastBatchCount;
	int iLastDrawCount;
} shape_full_gallery_t;

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

static int InitLab(shape_full_gallery_t* pLab)
{
	int iRet;

	if ( pLab->bReady ) {
		return XGE_OK;
	}
	iRet = xgeShapeBatchInit(&pLab->tBatch, XGE_COLOR_RGBA(120, 214, 255, 208), 64, XGE_DRAW_SCREEN_SPACE);
	pLab->bBatchInitOK = (iRet == XGE_OK);
	if ( !pLab->bBatchInitOK ) {
		fprintf(stderr, "shape-full-gallery stage failed: batch init\n");
		return iRet;
	}
	pLab->bReady = 1;
	printf("shape-full-gallery init batch_init=%d\n", pLab->bBatchInitOK);
	return XGE_OK;
}

static void DrawPanelFrames(void)
{
	xgeShapeRectFillPx((xge_rect_t){ 24.0f, 24.0f, 348.0f, 206.0f }, XGE_COLOR_RGBA(30, 36, 42, 255));
	xgeShapeRectFillPx((xge_rect_t){ 388.0f, 24.0f, 348.0f, 206.0f }, XGE_COLOR_RGBA(30, 36, 42, 255));
	xgeShapeRectFillPx((xge_rect_t){ 24.0f, 250.0f, 712.0f, 246.0f }, XGE_COLOR_RGBA(24, 30, 36, 255));
	xgeShapeRectStrokePx((xge_rect_t){ 24.0f, 24.0f, 348.0f, 206.0f }, 2.0f, XGE_COLOR_RGBA(104, 120, 128, 255));
	xgeShapeRectStrokePx((xge_rect_t){ 388.0f, 24.0f, 348.0f, 206.0f }, 2.0f, XGE_COLOR_RGBA(104, 120, 128, 255));
	xgeShapeRectStrokePx((xge_rect_t){ 24.0f, 250.0f, 712.0f, 246.0f }, 2.0f, XGE_COLOR_RGBA(104, 120, 128, 255));
}

static void DrawPixelGallery(shape_full_gallery_t* pLab)
{
	xge_vec2_t tA;
	xge_vec2_t tB;
	xge_vec2_t tC;
	xge_vec2_t arrPoly[5];

	xgeShapePointPx(56.0f, 58.0f, 10.0f, XGE_COLOR_RGBA(255, 255, 255, 255));
	xgeShapeLinePx(74.0f, 72.0f, 168.0f, 112.0f, 6.0f, XGE_COLOR_RGBA(120, 206, 255, 255));
	xgeShapeRectFillPx((xge_rect_t){ 196.0f, 52.0f, 78.0f, 46.0f }, XGE_COLOR_RGBA(80, 148, 230, 230));
	xgeShapeRectStrokePx((xge_rect_t){ 192.0f, 48.0f, 86.0f, 54.0f }, 3.0f, XGE_COLOR_RGBA(220, 240, 255, 255));
	xgeShapeCircleFillPx(96.0f, 168.0f, 28.0f, XGE_COLOR_RGBA(86, 200, 118, 228));
	xgeShapeCircleStrokePx(186.0f, 168.0f, 30.0f, 5.0f, XGE_COLOR_RGBA(255, 224, 118, 255));
	xgeShapeArcPx(284.0f, 162.0f, 34.0f, 0.25f, 4.90f, 6.0f, XGE_COLOR_RGBA(255, 150, 92, 255));
	tA.fX = 256.0f; tA.fY = 102.0f;
	tB.fX = 312.0f; tB.fY = 134.0f;
	tC.fX = 232.0f; tC.fY = 148.0f;
	xgeShapeTriangleFillPx(tA, tB, tC, XGE_COLOR_RGBA(255, 122, 164, 214));
	arrPoly[0].fX = 302.0f; arrPoly[0].fY = 56.0f;
	arrPoly[1].fX = 340.0f; arrPoly[1].fY = 78.0f;
	arrPoly[2].fX = 334.0f; arrPoly[2].fY = 124.0f;
	arrPoly[3].fX = 294.0f; arrPoly[3].fY = 134.0f;
	arrPoly[4].fX = 274.0f; arrPoly[4].fY = 90.0f;
	xgeShapePolygonFillPx(arrPoly, 5, XGE_COLOR_RGBA(156, 114, 232, 204));

	pLab->bPixelOK = 1;
	pLab->bPointOK = 1;
	pLab->bLineOK = 1;
	pLab->bRectOK = 1;
	pLab->bCircleOK = 1;
	pLab->bArcOK = 1;
	pLab->bTriangleOK = 1;
	pLab->bPolygonOK = 1;
	pLab->bFillStrokeOK = 1;
}

static void DrawBatchGallery(shape_full_gallery_t* pLab)
{
	xge_vec2_t tA;
	xge_vec2_t tB;
	xge_vec2_t tC;

	xgeShapeBatchClear(&pLab->tBatch);
	pLab->bBatchClearOK = (pLab->tBatch.iTriangleCount == 0);
	pLab->bBatchRectOK = (xgeShapeBatchRectFill(&pLab->tBatch, (xge_rect_t){ 414.0f, 52.0f, 58.0f, 44.0f }) == XGE_OK);
	pLab->bBatchRectOK = pLab->bBatchRectOK && (xgeShapeBatchRectFill(&pLab->tBatch, (xge_rect_t){ 486.0f, 80.0f, 46.0f, 64.0f }) == XGE_OK);
	pLab->bBatchRectOK = pLab->bBatchRectOK && (xgeShapeBatchRectFill(&pLab->tBatch, (xge_rect_t){ 548.0f, 54.0f, 68.0f, 36.0f }) == XGE_OK);
	tA.fX = 446.0f; tA.fY = 154.0f;
	tB.fX = 516.0f; tB.fY = 116.0f;
	tC.fX = 522.0f; tC.fY = 188.0f;
	pLab->bBatchTriangleOK = (xgeShapeBatchTriangleFill(&pLab->tBatch, tA, tB, tC) == XGE_OK);
	tA.fX = 596.0f; tA.fY = 114.0f;
	tB.fX = 678.0f; tB.fY = 126.0f;
	tC.fX = 632.0f; tC.fY = 196.0f;
	pLab->bBatchTriangleOK = pLab->bBatchTriangleOK && (xgeShapeBatchTriangleFill(&pLab->tBatch, tA, tB, tC) == XGE_OK);
	pLab->bBatchFlushOK = (xgeShapeBatchFlush(&pLab->tBatch) == XGE_OK);
}

static void DrawWorldGallery(shape_full_gallery_t* pLab)
{
	xge_camera_t tCamera;
	xge_vec2_t tA;
	xge_vec2_t tB;
	xge_vec2_t tC;
	xge_vec2_t arrPoly[5];

	tCamera = xgeCameraDefault((float)xgeGetWidth(), (float)xgeGetHeight());
	tCamera.iCoordinateMode = XGE_COORD_CENTER;
	tCamera.tViewport.fX = 24.0f;
	tCamera.tViewport.fY = 250.0f;
	tCamera.tViewport.fW = 712.0f;
	tCamera.tViewport.fH = 246.0f;
	tCamera.tScale.fX = 1.18f;
	tCamera.tScale.fY = 1.18f;
	xgeCameraSet(&tCamera);

	xgeShapePoint(-272.0f, -70.0f, 10.0f, XGE_COLOR_RGBA(255, 255, 255, 255));
	xgeShapeLine(-252.0f, -48.0f, -176.0f, -12.0f, 7.0f, XGE_COLOR_RGBA(118, 206, 255, 255));
	xgeShapeRectFill((xge_rect_t){ -126.0f, -72.0f, 70.0f, 42.0f }, XGE_COLOR_RGBA(84, 150, 232, 228));
	xgeShapeRectStroke((xge_rect_t){ -134.0f, -80.0f, 86.0f, 58.0f }, 3.0f, XGE_COLOR_RGBA(220, 240, 255, 255));
	xgeShapeCircleFill(-216.0f, 48.0f, 24.0f, XGE_COLOR_RGBA(86, 200, 118, 226));
	xgeShapeCircleStroke(-126.0f, 48.0f, 28.0f, 5.0f, XGE_COLOR_RGBA(255, 224, 118, 255));
	xgeShapeArc(-28.0f, 42.0f, 34.0f, 0.35f, 5.15f, 6.0f, XGE_COLOR_RGBA(255, 150, 92, 255));
	tA.fX = 8.0f; tA.fY = -54.0f;
	tB.fX = 62.0f; tB.fY = -18.0f;
	tC.fX = -18.0f; tC.fY = -4.0f;
	xgeShapeTriangleFill(tA, tB, tC, XGE_COLOR_RGBA(255, 122, 164, 214));
	arrPoly[0].fX = 82.0f; arrPoly[0].fY = -70.0f;
	arrPoly[1].fX = 116.0f; arrPoly[1].fY = -44.0f;
	arrPoly[2].fX = 106.0f; arrPoly[2].fY = 4.0f;
	arrPoly[3].fX = 56.0f; arrPoly[3].fY = 14.0f;
	arrPoly[4].fX = 34.0f; arrPoly[4].fY = -30.0f;
	xgeShapePolygonFill(arrPoly, 5, XGE_COLOR_RGBA(156, 114, 232, 204));

	pLab->bWorldOK = 1;
}

static void RestoreDefaultCamera(void)
{
	xge_camera_t tCamera;

	tCamera = xgeCameraDefault((float)xgeGetWidth(), (float)xgeGetHeight());
	xgeCameraSet(&tCamera);
}

static void CleanupLab(shape_full_gallery_t* pLab)
{
	if ( pLab->bFreed ) {
		return;
	}
	if ( pLab->bBatchInitOK ) {
		xgeShapeBatchFree(&pLab->tBatch);
	}
	pLab->bFreed = 1;
	pLab->bReady = 0;
}

static void PrintFinalSummary(shape_full_gallery_t* pLab)
{
	printf("shape-full-gallery final-summary frames=%d pixel=%d world=%d point=%d line=%d rect=%d circle=%d arc=%d triangle=%d polygon=%d fillstroke=%d batch=%d/%d/%d/%d/%d auto=%d batch_path=%d stats(batch=%d draw=%d)\n",
		pLab->iFrameCount,
		pLab->bPixelOK,
		pLab->bWorldOK,
		pLab->bPointOK,
		pLab->bLineOK,
		pLab->bRectOK,
		pLab->bCircleOK,
		pLab->bArcOK,
		pLab->bTriangleOK,
		pLab->bPolygonOK,
		pLab->bFillStrokeOK,
		pLab->bBatchInitOK,
		pLab->bBatchClearOK,
		pLab->bBatchRectOK,
		pLab->bBatchTriangleOK,
		pLab->bBatchFlushOK,
		pLab->bAutoBatchOK,
		pLab->bBatchPathOK,
		pLab->iLastBatchCount,
		pLab->iLastDrawCount);
}

static int ShapeFullGalleryFrame(void* pUser)
{
	shape_full_gallery_t* pLab;
	xge_frame_stats_t tStats;
	int iRet;

	pLab = (shape_full_gallery_t*)pUser;
	pLab->iFrameCount++;
	iRet = InitLab(pLab);
	if ( iRet != XGE_OK ) {
		xgeQuit();
		return 1;
	}
	if ( xgeKeyDown(XGE_KEY_ESCAPE) ) {
		PrintFinalSummary(pLab);
		CleanupLab(pLab);
		xgeQuit();
		return 1;
	}

	xgeClear(XGE_COLOR_RGBA(16, 20, 24, 255));
	DrawPanelFrames();
	DrawPixelGallery(pLab);
	DrawBatchGallery(pLab);
	DrawWorldGallery(pLab);
	RestoreDefaultCamera();

	tStats = xgeFrameStatsGet();
	pLab->iLastBatchCount = tStats.iBatchCount;
	pLab->iLastDrawCount = tStats.iDrawCallCount;
	pLab->bAutoBatchOK = (tStats.iBatchCount >= 1);
	pLab->bBatchPathOK = (tStats.iBatchCount >= 2);

	if ( ((pLab->iFrameLimit > 0) && (pLab->iFrameCount >= pLab->iFrameLimit)) || ((pLab->fSecondLimit > 0.0) && (xgeTimer() >= pLab->fSecondLimit)) ) {
		PrintFinalSummary(pLab);
		CleanupLab(pLab);
		xgeQuit();
		return 1;
	}
	return 0;
}

int main(int argc, char** argv)
{
	xge_desc_t tDesc;
	shape_full_gallery_t tLab;
	int i;

	memset(&tLab, 0, sizeof(tLab));
	tLab.iFrameLimit = ArgInt(getenv("XGE_SHAPE_FULL_GALLERY_FRAMES"), 0);
	tLab.fSecondLimit = ArgDouble(getenv("XGE_SHAPE_FULL_GALLERY_SECONDS"), 0.0);
	for ( i = 1; i < argc; i++ ) {
		if ( (strcmp(argv[i], "--frames") == 0) && ((i + 1) < argc) ) {
			tLab.iFrameLimit = ArgInt(argv[++i], tLab.iFrameLimit);
		} else if ( (strcmp(argv[i], "--seconds") == 0) && ((i + 1) < argc) ) {
			tLab.fSecondLimit = ArgDouble(argv[++i], tLab.fSecondLimit);
		}
	}
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iWidth = 760;
	tDesc.iHeight = 520;
	tDesc.sTitle = "XGE Shape Full Gallery";
	tDesc.iFlags = XGE_INIT_WINDOW | XGE_INIT_VSYNC;
	tDesc.iRunMode = XGE_RUN_GAME_LOOP;
	tDesc.iTargetFPS = 60;
	if ( xgeInit(&tDesc) != XGE_OK ) {
		return 1;
	}
	xgeRun(ShapeFullGalleryFrame, &tLab);
	CleanupLab(&tLab);
	xgeUnit();
	printf("shape-full-gallery summary frames=%d freed=%d\n", tLab.iFrameCount, tLab.bFreed);
	return 0;
}
