#include "../xge.h"
#include <stdio.h>
#include <string.h>

int main(void)
{
	FILE* fp;
	xge_desc_t tDesc;
	xge_skeleton_asset_t tAsset;
	xge_skeleton_t tSkeleton;
	xge_skeleton_pose_t tPose;
	unsigned char arrPixels[16 * 8 * 4];
	int iRet;
	int i;

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iWidth = 320;
	tDesc.iHeight = 200;
	tDesc.sTitle = "XGE Skeleton XSKEL Smoke";
	tDesc.iFlags = XGE_INIT_WINDOW;
	tDesc.iRunMode = XGE_RUN_MANUAL;
	if ( xgeInit(&tDesc) != XGE_OK ) {
		return 1;
	}
	for ( i = 0; i < 16 * 8; i++ ) {
		arrPixels[(i * 4) + 0] = 180;
		arrPixels[(i * 4) + 1] = 220;
		arrPixels[(i * 4) + 2] = 255;
		arrPixels[(i * 4) + 3] = 255;
	}
	if ( xgeImageSavePNG("build/test_skeleton_xskel.png", 16, 8, arrPixels, 16 * 4) != XGE_OK ) {
		xgeUnit();
		return 2;
	}
	fp = fopen("build/test_skeleton_xskel.xskel", "wb");
	if ( fp == NULL ) {
		xgeUnit();
		return 3;
	}
	fprintf(fp, "XGE_SKEL_TEXT\t1\n");
	fprintf(fp, "document\tloader_smoke\t24\t0\t1\t0\t111\t0\t-1\t0\t3\t2\t2\t2\t2\n");
	fprintf(fp, "bones\t2\n");
	fprintf(fp, "bone\t1\t-1\troot\t10\t20\t0\t40\t0\t1\n");
	fprintf(fp, "bone\t2\t0\tchild\t0\t0\t0\t30\t1\t1\n");
	fprintf(fp, "slots\t1\n");
	fprintf(fp, "slot\t1\t1\tchild.slot\t0\t0\t4294967295\t1\n");
	fprintf(fp, "attachments\t1\n");
	fprintf(fp, "attachment\t1\t1\tchild.region\ttest_skeleton_xskel.png\t0\t0\t0\t1\t1\t8\t4\t0\n");
	fprintf(fp, "animations\t1\n");
	fprintf(fp, "animation\t1\tidle\t0\t24\t1\n");
	fprintf(fp, "keyframes\t1\n");
	fprintf(fp, "keyframe\t1\t0\t1\t12\t0\t0\t0.5\t45\n");
	fprintf(fp, "end\n");
	fclose(fp);

	memset(&tAsset, 0, sizeof(tAsset));
	memset(&tSkeleton, 0, sizeof(tSkeleton));
	iRet = xgeSkeletonAssetLoadXskel(&tAsset, "build/test_skeleton_xskel.xskel");
	if ( iRet != XGE_OK ) {
		printf("xgeSkeletonAssetLoadXskel failed: %d\n", iRet);
		xgeUnit();
		return 4;
	}
	if ( tAsset.iBoneCount != 2 || tAsset.iSlotCount != 1 || tAsset.iRegionCount != 1 || tAsset.iAnimationCount != 1 || tAsset.iOwnedTextureCount != 1 ) {
		xgeSkeletonAssetFree(&tAsset);
		xgeUnit();
		return 5;
	}
	if ( tAsset.arrBones[1].fX < 39.99f || tAsset.arrBones[1].fX > 40.01f ) {
		xgeSkeletonAssetFree(&tAsset);
		xgeUnit();
		return 6;
	}
	if ( tAsset.arrRegions[0].tSize.fX != 16.0f || tAsset.arrRegions[0].tSize.fY != 8.0f ) {
		xgeSkeletonAssetFree(&tAsset);
		xgeUnit();
		return 7;
	}
	if ( xgeSkeletonInit(&tSkeleton, &tAsset) != XGE_OK ) {
		xgeSkeletonAssetFree(&tAsset);
		xgeUnit();
		return 8;
	}
	if ( xgeSkeletonPoseGet(&tSkeleton, 1, &tPose) != XGE_OK ) {
		xgeSkeletonFree(&tSkeleton);
		xgeSkeletonAssetFree(&tAsset);
		xgeUnit();
		return 9;
	}
	if ( tPose.fWorldX < 49.99f || tPose.fWorldX > 50.01f || tPose.fWorldY < 19.99f || tPose.fWorldY > 20.01f ) {
		xgeSkeletonFree(&tSkeleton);
		xgeSkeletonAssetFree(&tAsset);
		xgeUnit();
		return 10;
	}
	printf("skeleton_xskel_load_smoke ok bones=%d slots=%d regions=%d animations=%d\n",
		tAsset.iBoneCount,
		tAsset.iSlotCount,
		tAsset.iRegionCount,
		tAsset.iAnimationCount);
	xgeSkeletonFree(&tSkeleton);
	xgeSkeletonAssetFree(&tAsset);
	xgeUnit();
	return 0;
}
