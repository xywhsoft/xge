#include "../../xge.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

#define XGE_EXAMPLE_KEY_ESCAPE	256

typedef struct skeleton_basic_demo_t {
	xge_texture_t tAtlas;
	xge_skeleton_asset_t tAsset;
	xge_skeleton_t tSkeleton;
	int bReady;
} skeleton_basic_demo_t;

static void __skeletonMakeAtlas(unsigned char* pPixels, int iWidth, int iHeight)
{
	int iX;
	int iY;

	for ( iY = 0; iY < iHeight; iY++ ) {
		for ( iX = 0; iX < iWidth; iX++ ) {
			int iPos = ((iY * iWidth) + iX) * 4;
			unsigned char iR = 30;
			unsigned char iG = 40;
			unsigned char iB = 54;
			if ( iX < 64 ) {
				iR = 88;
				iG = 168;
				iB = 245;
			} else if ( iX < 128 ) {
				iR = 241;
				iG = 196;
				iB = 83;
			} else {
				iR = 235;
				iG = 97;
				iB = 83;
			}
			if ( (iX % 64) < 3 || iY < 3 || iY > iHeight - 4 ) {
				iR = 255;
				iG = 255;
				iB = 255;
			}
			pPixels[iPos + 0] = iR;
			pPixels[iPos + 1] = iG;
			pPixels[iPos + 2] = iB;
			pPixels[iPos + 3] = 255;
		}
	}
}

static int __skeletonBasicInit(skeleton_basic_demo_t* pDemo)
{
	unsigned char arrPixels[192 * 64 * 4];
	xge_skeleton_bone_desc_t arrBones[3];
	xge_skeleton_region_desc_t arrRegions[3];
	xge_skeleton_slot_desc_t arrSlots[3];
	xge_skeleton_bone_key_t arrBodyKeys[3];
	xge_skeleton_bone_key_t arrArmKeys[3];
	xge_skeleton_bone_key_t arrHeadKeys[3];
	xge_skeleton_bone_track_desc_t arrTracks[3];
	xge_skeleton_animation_desc_t arrAnimations[1];
	xge_skeleton_asset_desc_t tDesc;

	if ( pDemo->bReady ) {
		return XGE_OK;
	}
	__skeletonMakeAtlas(arrPixels, 192, 64);
	if ( xgeTextureCreateRGBA(&pDemo->tAtlas, 192, 64, arrPixels) != XGE_OK ) {
		return XGE_ERROR_RESOURCE_FAILED;
	}

	memset(arrBones, 0, sizeof(arrBones));
	arrBones[0].sName = "body";
	arrBones[0].iParent = -1;
	arrBones[0].fScaleX = 1.0f;
	arrBones[0].fScaleY = 1.0f;
	arrBones[1].sName = "arm";
	arrBones[1].iParent = 0;
	arrBones[1].fX = 0.0f;
	arrBones[1].fY = -36.0f;
	arrBones[1].fScaleX = 1.0f;
	arrBones[1].fScaleY = 1.0f;
	arrBones[2].sName = "head";
	arrBones[2].iParent = 0;
	arrBones[2].fX = 0.0f;
	arrBones[2].fY = -56.0f;
	arrBones[2].fScaleX = 1.0f;
	arrBones[2].fScaleY = 1.0f;

	memset(arrRegions, 0, sizeof(arrRegions));
	arrRegions[0].sName = "body_region";
	arrRegions[0].pTexture = &pDemo->tAtlas;
	arrRegions[0].tSrc = (xge_rect_t){ 0.0f, 0.0f, 64.0f, 64.0f };
	arrRegions[0].tSize = (xge_vec2_t){ 46.0f, 76.0f };
	arrRegions[0].tPivot = (xge_vec2_t){ 23.0f, 38.0f };
	arrRegions[0].fScaleX = 1.0f;
	arrRegions[0].fScaleY = 1.0f;
	arrRegions[1].sName = "arm_region";
	arrRegions[1].pTexture = &pDemo->tAtlas;
	arrRegions[1].tSrc = (xge_rect_t){ 64.0f, 0.0f, 64.0f, 64.0f };
	arrRegions[1].tSize = (xge_vec2_t){ 18.0f, 72.0f };
	arrRegions[1].tPivot = (xge_vec2_t){ 9.0f, 8.0f };
	arrRegions[1].tOffset = (xge_vec2_t){ 0.0f, 0.0f };
	arrRegions[1].fScaleX = 1.0f;
	arrRegions[1].fScaleY = 1.0f;
	arrRegions[2].sName = "head_region";
	arrRegions[2].pTexture = &pDemo->tAtlas;
	arrRegions[2].tSrc = (xge_rect_t){ 128.0f, 0.0f, 64.0f, 64.0f };
	arrRegions[2].tSize = (xge_vec2_t){ 38.0f, 38.0f };
	arrRegions[2].tPivot = (xge_vec2_t){ 19.0f, 30.0f };
	arrRegions[2].fScaleX = 1.0f;
	arrRegions[2].fScaleY = 1.0f;

	memset(arrSlots, 0, sizeof(arrSlots));
	arrSlots[0].sName = "body_slot";
	arrSlots[0].iBone = 0;
	arrSlots[0].iAttachment = 0;
	arrSlots[0].iColor = XGE_COLOR_RGBA(255, 255, 255, 255);
	arrSlots[1].sName = "arm_slot";
	arrSlots[1].iBone = 1;
	arrSlots[1].iAttachment = 1;
	arrSlots[1].iColor = XGE_COLOR_RGBA(255, 255, 255, 255);
	arrSlots[2].sName = "head_slot";
	arrSlots[2].iBone = 2;
	arrSlots[2].iAttachment = 2;
	arrSlots[2].iColor = XGE_COLOR_RGBA(255, 255, 255, 255);

	memset(arrBodyKeys, 0, sizeof(arrBodyKeys));
	arrBodyKeys[0] = (xge_skeleton_bone_key_t){ 0.0f, XGE_SKELETON_KEY_Y | XGE_SKELETON_KEY_SCALE_X | XGE_SKELETON_KEY_SCALE_Y, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f };
	arrBodyKeys[1] = (xge_skeleton_bone_key_t){ 0.5f, XGE_SKELETON_KEY_Y | XGE_SKELETON_KEY_SCALE_X | XGE_SKELETON_KEY_SCALE_Y, 0.0f, -8.0f, 0.0f, 1.0f, 1.0f };
	arrBodyKeys[2] = arrBodyKeys[0];
	arrBodyKeys[2].fTime = 1.0f;
	memset(arrArmKeys, 0, sizeof(arrArmKeys));
	arrArmKeys[0] = (xge_skeleton_bone_key_t){ 0.0f, XGE_SKELETON_KEY_ROTATION | XGE_SKELETON_KEY_SCALE_X | XGE_SKELETON_KEY_SCALE_Y, 0.0f, 0.0f, -0.65f, 1.0f, 1.0f };
	arrArmKeys[1] = (xge_skeleton_bone_key_t){ 0.5f, XGE_SKELETON_KEY_ROTATION | XGE_SKELETON_KEY_SCALE_X | XGE_SKELETON_KEY_SCALE_Y, 0.0f, 0.0f, 0.65f, 1.0f, 1.0f };
	arrArmKeys[2] = arrArmKeys[0];
	arrArmKeys[2].fTime = 1.0f;
	memset(arrHeadKeys, 0, sizeof(arrHeadKeys));
	arrHeadKeys[0] = (xge_skeleton_bone_key_t){ 0.0f, XGE_SKELETON_KEY_ROTATION | XGE_SKELETON_KEY_SCALE_X | XGE_SKELETON_KEY_SCALE_Y, 0.0f, 0.0f, 0.15f, 1.0f, 1.0f };
	arrHeadKeys[1] = (xge_skeleton_bone_key_t){ 0.5f, XGE_SKELETON_KEY_ROTATION | XGE_SKELETON_KEY_SCALE_X | XGE_SKELETON_KEY_SCALE_Y, 0.0f, 0.0f, -0.15f, 1.0f, 1.0f };
	arrHeadKeys[2] = arrHeadKeys[0];
	arrHeadKeys[2].fTime = 1.0f;

	arrTracks[0] = (xge_skeleton_bone_track_desc_t){ 0, arrBodyKeys, 3 };
	arrTracks[1] = (xge_skeleton_bone_track_desc_t){ 1, arrArmKeys, 3 };
	arrTracks[2] = (xge_skeleton_bone_track_desc_t){ 2, arrHeadKeys, 3 };
	arrAnimations[0].sName = "idle";
	arrAnimations[0].fDuration = 1.0f;
	arrAnimations[0].arrBoneTracks = arrTracks;
	arrAnimations[0].iBoneTrackCount = 3;

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.arrBones = arrBones;
	tDesc.iBoneCount = 3;
	tDesc.arrSlots = arrSlots;
	tDesc.iSlotCount = 3;
	tDesc.arrRegions = arrRegions;
	tDesc.iRegionCount = 3;
	tDesc.arrAnimations = arrAnimations;
	tDesc.iAnimationCount = 1;
	if ( xgeSkeletonAssetInit(&pDemo->tAsset, &tDesc) != XGE_OK ) {
		xgeTextureFree(&pDemo->tAtlas);
		return XGE_ERROR_RESOURCE_FAILED;
	}
	if ( xgeSkeletonInit(&pDemo->tSkeleton, &pDemo->tAsset) != XGE_OK ) {
		xgeSkeletonAssetFree(&pDemo->tAsset);
		xgeTextureFree(&pDemo->tAtlas);
		return XGE_ERROR_RESOURCE_FAILED;
	}
	xgeSkeletonSetAnimation(&pDemo->tSkeleton, "idle", 1);
	pDemo->bReady = 1;
	return XGE_OK;
}

static int SkeletonBasicFrame(void* pUser)
{
	skeleton_basic_demo_t* pDemo;
	xge_skeleton_draw_desc_t tDraw;
	xge_frame_stats_t tStats;

	pDemo = (skeleton_basic_demo_t*)pUser;
	if ( xgeKeyDown(XGE_EXAMPLE_KEY_ESCAPE) ) {
		xgeQuit();
		return 1;
	}
	if ( __skeletonBasicInit(pDemo) != XGE_OK ) {
		xgeQuit();
		return 2;
	}
	xgeSkeletonUpdate(&pDemo->tSkeleton, xgeGetDelta());
	xgeFrameStatsReset();
	xgeClear(XGE_COLOR_RGBA(16, 20, 28, 255));
	xgeShapeRectFillPx((xge_rect_t){ 96.0f, 42.0f, 448.0f, 344.0f }, XGE_COLOR_RGBA(28, 34, 44, 255));
	xgeShapeRectStrokePx((xge_rect_t){ 96.0f, 42.0f, 448.0f, 344.0f }, 2.0f, XGE_COLOR_RGBA(76, 94, 116, 255));
	xgeShapeLinePx(128.0f, 306.0f, 512.0f, 306.0f, 2.0f, XGE_COLOR_RGBA(80, 96, 116, 255));
	memset(&tDraw, 0, sizeof(tDraw));
	tDraw.fX = 320.0f;
	tDraw.fY = 300.0f;
	tDraw.fScale = 1.0f;
	tDraw.iColor = XGE_COLOR_RGBA(255, 255, 255, 255);
	tDraw.iFlags = XGE_DRAW_SCREEN_SPACE;
	xgeSkeletonDraw(&pDemo->tSkeleton, &tDraw);
	tStats = xgeFrameStatsGet();
	printf("\rSkeleton slots: %d  DrawCalls: %d   ", pDemo->tAsset.iSlotCount, tStats.iDrawCallCount);
	return 0;
}

int main(int argc, char** argv)
{
	xge_desc_t tDesc;
	skeleton_basic_demo_t tDemo;

	(void)argc;
	(void)argv;
	memset(&tDemo, 0, sizeof(tDemo));
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iWidth = 640;
	tDesc.iHeight = 448;
	tDesc.sTitle = "XGE Skeleton Basic";
	tDesc.iFlags = XGE_INIT_WINDOW | XGE_INIT_VSYNC;
	tDesc.iRunMode = XGE_RUN_GAME_LOOP;
	tDesc.iTargetFPS = 60;
	if ( xgeInit(&tDesc) != XGE_OK ) {
		return 1;
	}
	xgeRun(SkeletonBasicFrame, &tDemo);
	xgeSkeletonFree(&tDemo.tSkeleton);
	xgeSkeletonAssetFree(&tDemo.tAsset);
	xgeTextureFree(&tDemo.tAtlas);
	xgeUnit();
	return 0;
}
