#include "../../xge.h"
#include <string.h>

#define XGE_EXAMPLE_KEY_ESCAPE	256

typedef struct async_demo_t {
	xge_async_request_t tRequest;
	xge_texture_t tTexture;
	int bStarted;
	int bComplete;
	float fPulse;
} async_demo_t;

static void AsyncComplete(xge_async_request pRequest, void* pUser)
{
	async_demo_t* pDemo;

	pDemo = (async_demo_t*)pUser;
	pDemo->bComplete = 1;
	pDemo->fPulse = 1.0f;
	(void)pRequest;
}

static void AsyncStart(async_demo_t* pDemo)
{
	unsigned char arrFallback[16];

	arrFallback[0] = 255; arrFallback[1] = 120; arrFallback[2] = 150; arrFallback[3] = 255;
	arrFallback[4] = 255; arrFallback[5] = 220; arrFallback[6] = 96; arrFallback[7] = 255;
	arrFallback[8] = 120; arrFallback[9] = 200; arrFallback[10] = 255; arrFallback[11] = 255;
	arrFallback[12] = 96; arrFallback[13] = 220; arrFallback[14] = 150; arrFallback[15] = 255;
	xgeTextureFallbackSetRGBA(2, 2, arrFallback);
	xgeAsyncTextureLoad(&pDemo->tRequest, &pDemo->tTexture, "missing_async_demo_texture.png", XGE_IMAGE_PREMULTIPLIED, AsyncComplete, pDemo);
	pDemo->bStarted = 1;
}

static int AsyncFrame(void* pUser)
{
	async_demo_t* pDemo;
	xge_draw_t tDraw;
	xge_rect_t tRect;
	float fSize;

	pDemo = (async_demo_t*)pUser;
	if ( xgeKeyDown(XGE_EXAMPLE_KEY_ESCAPE) ) {
		xgeQuit();
		return 1;
	}
	if ( pDemo->bStarted == 0 ) {
		AsyncStart(pDemo);
	}

	xgeClear(XGE_COLOR_RGBA(18, 24, 30, 255));
	tRect.fX = 56.0f;
	tRect.fY = 52.0f;
	tRect.fW = 220.0f;
	tRect.fH = 44.0f;
	xgeShapeRectFillPx(tRect, pDemo->bComplete ? XGE_COLOR_RGBA(42, 160, 110, 255) : XGE_COLOR_RGBA(160, 120, 42, 255));
	xgeShapeRectStrokePx(tRect, 3.0f, XGE_COLOR_RGBA(210, 235, 255, 255));

	fSize = 128.0f + (pDemo->fPulse * 36.0f);
	memset(&tDraw, 0, sizeof(tDraw));
	tDraw.pTexture = &pDemo->tTexture;
	tDraw.tSrc.fW = (float)pDemo->tTexture.iWidth;
	tDraw.tSrc.fH = (float)pDemo->tTexture.iHeight;
	tDraw.tDst.fX = 256.0f - (fSize * 0.5f);
	tDraw.tDst.fY = 168.0f - (fSize * 0.5f);
	tDraw.tDst.fW = fSize;
	tDraw.tDst.fH = fSize;
	tDraw.iColor = XGE_COLOR_RGBA(255, 255, 255, 255);
	tDraw.iFlags = XGE_DRAW_SCREEN_SPACE;
	xgeDrawEx(&tDraw);

	xgeShapeCircleStrokePx(256.0f, 168.0f, fSize * 0.62f, 4.0f, (pDemo->tTexture.iFlags & XGE_TEXTURE_FALLBACK) ? XGE_COLOR_RGBA(255, 220, 96, 255) : XGE_COLOR_RGBA(120, 200, 255, 255));
	xgeShapeLinePx(64.0f, 282.0f, 448.0f, 282.0f, 3.0f, XGE_COLOR_RGBA(80, 120, 150, 255));
	xgeShapeRectFillPx((xge_rect_t){ 64.0f, 276.0f, 384.0f * pDemo->tRequest.fProgress, 12.0f }, XGE_COLOR_RGBA(120, 200, 255, 255));

	if ( pDemo->fPulse > 0.0f ) {
		pDemo->fPulse -= xgeGetDelta() * 2.0f;
		if ( pDemo->fPulse < 0.0f ) {
			pDemo->fPulse = 0.0f;
		}
	}
	return 0;
}

int main(int argc, char** argv)
{
	xge_desc_t tDesc;
	async_demo_t tDemo;

	(void)argc;
	(void)argv;
	memset(&tDemo, 0, sizeof(tDemo));
	xgeAsyncRequestInit(&tDemo.tRequest);
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iWidth = 512;
	tDesc.iHeight = 320;
	tDesc.sTitle = "XGE Async";
	tDesc.iFlags = XGE_INIT_WINDOW | XGE_INIT_VSYNC;
	tDesc.iRunMode = XGE_RUN_GAME_LOOP;
	tDesc.iTargetFPS = 60;
	if ( xgeInit(&tDesc) != XGE_OK ) {
		return 1;
	}
	xgeRun(AsyncFrame, &tDemo);
	xgeTextureFree(&tDemo.tTexture);
	xgeTextureFallbackClear();
	xgeAsyncRequestFree(&tDemo.tRequest);
	xgeUnit();
	return 0;
}
