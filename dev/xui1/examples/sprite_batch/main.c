#include "../../xge.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

#define XGE_EXAMPLE_KEY_ESCAPE	256
#define XGE_SPRITE_COUNT	96

typedef struct sprite_batch_demo_t {
	xge_texture_t tAtlas;
	xge_sprite_batch_t tBatch;
	int bReady;
	float fTime;
} sprite_batch_demo_t;

static void __spriteBatchMakeAtlas(unsigned char* pPixels, int iWidth, int iHeight)
{
	int iX;
	int iY;
	int iTileX;
	int iTileY;
	int iPos;
	unsigned char iR;
	unsigned char iG;
	unsigned char iB;

	for ( iY = 0; iY < iHeight; iY++ ) {
		for ( iX = 0; iX < iWidth; iX++ ) {
			iTileX = iX / 32;
			iTileY = iY / 32;
			iPos = ((iY * iWidth) + iX) * 4;
			iR = (unsigned char)(70 + iTileX * 50);
			iG = (unsigned char)(80 + iTileY * 55);
			iB = (unsigned char)(190 + ((iTileX + iTileY) & 1) * 45);
			if ( (iX % 32) < 3 || (iY % 32) < 3 ) {
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

static int __spriteBatchInit(sprite_batch_demo_t* pDemo)
{
	unsigned char arrPixels[128 * 128 * 4];

	if ( pDemo->bReady ) {
		return XGE_OK;
	}
	__spriteBatchMakeAtlas(arrPixels, 128, 128);
	if ( xgeTextureCreateRGBA(&pDemo->tAtlas, 128, 128, arrPixels) != XGE_OK ) {
		return XGE_ERROR_RESOURCE_FAILED;
	}
	if ( xgeSpriteBatchInit(&pDemo->tBatch, &pDemo->tAtlas, XGE_SPRITE_COUNT, 0) != XGE_OK ) {
		xgeTextureFree(&pDemo->tAtlas);
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pDemo->bReady = 1;
	return XGE_OK;
}

static int SpriteBatchFrame(void* pUser)
{
	sprite_batch_demo_t* pDemo;
	xge_draw_t tDraw;
	xge_frame_stats_t tStats;
	int i;
	int iTile;
	float fX;
	float fY;

	pDemo = (sprite_batch_demo_t*)pUser;
	if ( xgeKeyDown(XGE_EXAMPLE_KEY_ESCAPE) ) {
		xgeQuit();
		return 1;
	}
	if ( __spriteBatchInit(pDemo) != XGE_OK ) {
		xgeQuit();
		return 2;
	}
	pDemo->fTime += xgeGetDelta();
	xgeFrameStatsReset();

	xgeClear(XGE_COLOR_RGBA(14, 18, 26, 255));
	xgeShapeRectFillPx((xge_rect_t){ 30.0f, 32.0f, 580.0f, 382.0f }, XGE_COLOR_RGBA(28, 34, 44, 255));
	xgeShapeRectStrokePx((xge_rect_t){ 30.0f, 32.0f, 580.0f, 382.0f }, 2.0f, XGE_COLOR_RGBA(90, 120, 142, 255));

	xgeSpriteBatchClear(&pDemo->tBatch);
	for ( i = 0; i < XGE_SPRITE_COUNT; i++ ) {
		iTile = i & 15;
		fX = 56.0f + (float)(i % 12) * 44.0f;
		fY = 64.0f + (float)(i / 12) * 38.0f + sinf(pDemo->fTime * 2.0f + (float)i * 0.19f) * 6.0f;
		memset(&tDraw, 0, sizeof(tDraw));
		tDraw.pTexture = &pDemo->tAtlas;
		tDraw.tSrc.fX = (float)((iTile % 4) * 32);
		tDraw.tSrc.fY = (float)((iTile / 4) * 32);
		tDraw.tSrc.fW = 32.0f;
		tDraw.tSrc.fH = 32.0f;
		tDraw.tDst.fX = fX;
		tDraw.tDst.fY = fY;
		tDraw.tDst.fW = 28.0f;
		tDraw.tDst.fH = 28.0f;
		tDraw.tOrigin.fX = 14.0f;
		tDraw.tOrigin.fY = 14.0f;
		tDraw.fRotation = sinf(pDemo->fTime + (float)i * 0.11f) * 0.45f;
		tDraw.iColor = XGE_COLOR_RGBA(255, 255, 255, 230);
		tDraw.iFlags = XGE_DRAW_SCREEN_SPACE;
		xgeSpriteBatchAdd(&pDemo->tBatch, &tDraw);
	}
	xgeSpriteBatchFlush(&pDemo->tBatch);

	tStats = xgeFrameStatsGet();
	printf("\rSprites: %d  DrawCalls: %d  Batches: %d   ", XGE_SPRITE_COUNT, tStats.iDrawCallCount, tStats.iBatchCount);
	return 0;
}

int main(int argc, char** argv)
{
	xge_desc_t tDesc;
	sprite_batch_demo_t tDemo;

	(void)argc;
	(void)argv;
	memset(&tDemo, 0, sizeof(tDemo));
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iWidth = 640;
	tDesc.iHeight = 448;
	tDesc.sTitle = "XGE Sprite Batch";
	tDesc.iFlags = XGE_INIT_WINDOW | XGE_INIT_VSYNC;
	tDesc.iRunMode = XGE_RUN_GAME_LOOP;
	tDesc.iTargetFPS = 60;
	if ( xgeInit(&tDesc) != XGE_OK ) {
		return 1;
	}
	xgeRun(SpriteBatchFrame, &tDemo);
	xgeSpriteBatchFree(&tDemo.tBatch);
	xgeTextureFree(&tDemo.tAtlas);
	xgeUnit();
	return 0;
}
