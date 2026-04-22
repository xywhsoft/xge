#include "../../xge.h"
#include <string.h>

#define XGE_EXAMPLE_KEY_ESCAPE	256

typedef struct render_target_demo_t {
	xge_render_target_t tTarget;
	int bReady;
	float fTime;
} render_target_demo_t;

static int __renderTargetDemoInit(render_target_demo_t* pDemo)
{
	if ( pDemo->bReady ) {
		return XGE_OK;
	}
	if ( xgeRenderTargetCreate(&pDemo->tTarget, 256, 192) != XGE_OK ) {
		return XGE_ERROR_GPU_FAILED;
	}
	pDemo->bReady = 1;
	return XGE_OK;
}

static int RenderTargetFrame(void* pUser)
{
	render_target_demo_t* pDemo;
	xge_pass_t tPass;
	xge_draw_t tDraw;
	xge_texture pTexture;

	pDemo = (render_target_demo_t*)pUser;
	if ( xgeKeyDown(XGE_EXAMPLE_KEY_ESCAPE) ) {
		xgeQuit();
		return 1;
	}
	if ( __renderTargetDemoInit(pDemo) != XGE_OK ) {
		xgeQuit();
		return 2;
	}
	pDemo->fTime += xgeGetDelta();

	xgePassInit(&tPass, &pDemo->tTarget, XGE_PASS_CLEAR_COLOR, XGE_COLOR_RGBA(18, 26, 38, 255));
	if ( xgePassBegin(&tPass) != XGE_OK ) {
		xgeQuit();
		return 3;
	}
	xgeShapeRectFillPx((xge_rect_t){ 20.0f, 20.0f, 216.0f, 152.0f }, XGE_COLOR_RGBA(46, 64, 86, 255));
	xgeShapeCircleFillPx(128.0f, 96.0f, 42.0f + (pDemo->fTime * 8.0f), XGE_COLOR_RGBA(120, 210, 255, 230));
	xgeShapeLinePx(32.0f, 160.0f, 224.0f, 42.0f, 5.0f, XGE_COLOR_RGBA(255, 220, 120, 255));
	xgePassEnd(&tPass);

	xgeClear(XGE_COLOR_RGBA(12, 16, 22, 255));
	xgeShapeRectFillPx((xge_rect_t){ 32.0f, 32.0f, 576.0f, 384.0f }, XGE_COLOR_RGBA(28, 34, 44, 255));
	xgeShapeRectStrokePx((xge_rect_t){ 32.0f, 32.0f, 576.0f, 384.0f }, 2.0f, XGE_COLOR_RGBA(110, 128, 150, 255));

	pTexture = xgeRenderTargetTexture(&pDemo->tTarget);
	memset(&tDraw, 0, sizeof(tDraw));
	tDraw.pTexture = pTexture;
	tDraw.tDst.fX = 320.0f;
	tDraw.tDst.fY = 224.0f;
	tDraw.tDst.fW = 384.0f;
	tDraw.tDst.fH = 288.0f;
	tDraw.tOrigin.fX = 192.0f;
	tDraw.tOrigin.fY = 144.0f;
	tDraw.fRotation = pDemo->fTime * 0.4f;
	tDraw.iColor = XGE_COLOR_RGBA(255, 255, 255, 255);
	tDraw.iFlags = XGE_DRAW_SCREEN_SPACE;
	xgeDrawEx(&tDraw);

	xgeShapeRectStrokePx((xge_rect_t){ 128.0f, 80.0f, 384.0f, 288.0f }, 3.0f, XGE_COLOR_RGBA(255, 255, 255, 190));
	return 0;
}

int main(int argc, char** argv)
{
	xge_desc_t tDesc;
	render_target_demo_t tDemo;

	(void)argc;
	(void)argv;
	memset(&tDesc, 0, sizeof(tDesc));
	memset(&tDemo, 0, sizeof(tDemo));
	tDesc.iWidth = 640;
	tDesc.iHeight = 448;
	tDesc.sTitle = "XGE Render Target";
	tDesc.iFlags = XGE_INIT_WINDOW | XGE_INIT_VSYNC;
	tDesc.iRunMode = XGE_RUN_GAME_LOOP;
	tDesc.iTargetFPS = 60;
	if ( xgeInit(&tDesc) != XGE_OK ) {
		return 1;
	}
	xgeRun(RenderTargetFrame, &tDemo);
	xgeRenderTargetFree(&tDemo.tTarget);
	xgeUnit();
	return 0;
}
