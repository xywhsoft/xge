#include "../../xge.h"
#include <stdio.h>
#include <string.h>

#define XGE_EXAMPLE_KEY_ESCAPE 256

typedef struct skeleton_xskel_viewer_t {
	const char* sPath;
	xge_skeleton_asset_t tAsset;
	xge_skeleton_t tSkeleton;
	int bReady;
	int bFailed;
} skeleton_xskel_viewer_t;

static int __skeletonXskelViewerInit(skeleton_xskel_viewer_t* pViewer)
{
	if ( pViewer->bReady ) {
		return XGE_OK;
	}
	if ( pViewer->bFailed ) {
		return XGE_ERROR;
	}
	if ( xgeSkeletonAssetLoadXskel(&pViewer->tAsset, pViewer->sPath) != XGE_OK ) {
		printf("xskel load failed: %s\n", pViewer->sPath);
		pViewer->bFailed = 1;
		return XGE_ERROR_RESOURCE_FAILED;
	}
	if ( xgeSkeletonInit(&pViewer->tSkeleton, &pViewer->tAsset) != XGE_OK ) {
		printf("skeleton init failed: %s\n", pViewer->sPath);
		xgeSkeletonAssetFree(&pViewer->tAsset);
		pViewer->bFailed = 1;
		return XGE_ERROR_RESOURCE_FAILED;
	}
	if ( pViewer->tAsset.iAnimationCount > 0 ) {
		xgeSkeletonSetAnimation(&pViewer->tSkeleton, pViewer->tAsset.arrAnimations[0].sName, 1);
	}
	printf("xskel loaded: %s bones=%d slots=%d regions=%d animations=%d\n",
		pViewer->sPath,
		pViewer->tAsset.iBoneCount,
		pViewer->tAsset.iSlotCount,
		pViewer->tAsset.iRegionCount,
		pViewer->tAsset.iAnimationCount);
	pViewer->bReady = 1;
	return XGE_OK;
}

static int SkeletonXskelViewerFrame(void* pUser)
{
	skeleton_xskel_viewer_t* pViewer;
	xge_skeleton_draw_desc_t tDraw;

	pViewer = (skeleton_xskel_viewer_t*)pUser;
	if ( xgeKeyDown(XGE_EXAMPLE_KEY_ESCAPE) ) {
		xgeQuit();
		return 1;
	}
	if ( __skeletonXskelViewerInit(pViewer) != XGE_OK ) {
		xgeQuit();
		return 2;
	}
	if ( pViewer->tAsset.iAnimationCount > 0 ) {
		xgeSkeletonUpdate(&pViewer->tSkeleton, xgeGetDelta());
	}
	xgeClear(XGE_COLOR_RGBA(18, 22, 28, 255));
	xgeShapeRectFillPx((xge_rect_t){ 64.0f, 40.0f, 672.0f, 500.0f }, XGE_COLOR_RGBA(27, 33, 42, 255));
	xgeShapeRectStrokePx((xge_rect_t){ 64.0f, 40.0f, 672.0f, 500.0f }, 2.0f, XGE_COLOR_RGBA(82, 96, 112, 255));
	xgeShapeLinePx(96.0f, 300.0f, 704.0f, 300.0f, 1.0f, XGE_COLOR_RGBA(72, 84, 98, 255));
	xgeShapeLinePx(384.0f, 72.0f, 384.0f, 512.0f, 1.0f, XGE_COLOR_RGBA(72, 84, 98, 255));
	memset(&tDraw, 0, sizeof(tDraw));
	tDraw.fX = 384.0f;
	tDraw.fY = 300.0f;
	tDraw.fScale = 1.0f;
	tDraw.iColor = XGE_COLOR_RGBA(255, 255, 255, 255);
	tDraw.iFlags = XGE_DRAW_SCREEN_SPACE;
	xgeSkeletonDraw(&pViewer->tSkeleton, &tDraw);
	return 0;
}

int main(int argc, char** argv)
{
	xge_desc_t tDesc;
	skeleton_xskel_viewer_t tViewer;

	memset(&tViewer, 0, sizeof(tViewer));
	tViewer.sPath = (argc > 1 && argv[1] != NULL) ? argv[1] : "build/test_skeleton_xskel.xskel";
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iWidth = 768;
	tDesc.iHeight = 576;
	tDesc.sTitle = "XGE Skeleton XSKEL Viewer";
	tDesc.iFlags = XGE_INIT_WINDOW | XGE_INIT_VSYNC;
	tDesc.iRunMode = XGE_RUN_GAME_LOOP;
	tDesc.iTargetFPS = 60;
	if ( xgeInit(&tDesc) != XGE_OK ) {
		return 1;
	}
	xgeRun(SkeletonXskelViewerFrame, &tViewer);
	xgeSkeletonFree(&tViewer.tSkeleton);
	xgeSkeletonAssetFree(&tViewer.tAsset);
	xgeUnit();
	return 0;
}
