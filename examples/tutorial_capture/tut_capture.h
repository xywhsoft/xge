/* tut_capture.h — 教程截图公共框架（XGE 章节用）
 * 提供初始化、渲染循环、截图保存的完整样板。
 * 用法：#include "tut_capture.h"，实现 draw_scene()，调用 tut_run()。
 */
#ifndef TUT_CAPTURE_H
#define TUT_CAPTURE_H

#include "../../xge.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef TUT_W
#define TUT_W 800
#endif
#ifndef TUT_H
#define TUT_H 600
#endif

typedef struct tut_ctx_t {
	xge_render_target_t tTarget;
	char sCapturePath[260];
	int bCaptureDone;
	int iFrame;
} tut_ctx_t;

static int tut_parse(tut_ctx_t* ctx, int argc, char** argv)
{
	int i;
	for ( i = 1; i < argc; i++ ) {
		if ( strcmp(argv[i], "--capture") == 0 ) {
			if ( i + 1 >= argc ) return XGE_ERROR_INVALID_ARGUMENT;
			snprintf(ctx->sCapturePath, sizeof(ctx->sCapturePath), "%s", argv[++i]);
			ctx->sCapturePath[sizeof(ctx->sCapturePath) - 1] = '\0';
		} else if ( strncmp(argv[i], "--capture=", 10) == 0 ) {
			snprintf(ctx->sCapturePath, sizeof(ctx->sCapturePath), "%s", argv[i] + 10);
			ctx->sCapturePath[sizeof(ctx->sCapturePath) - 1] = '\0';
		} else if ( strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0 ) {
			printf("usage: tut_chXX [--capture PATH.png]\n");
			return 1;
		}
	}
	return XGE_OK;
}

static int tut_save(tut_ctx_t* ctx)
{
	unsigned char* pPixels;
	int iStride;
	int iRet;

	if ( ctx->sCapturePath[0] == '\0' || ctx->bCaptureDone ) {
		return XGE_OK;
	}
	iStride = TUT_W * 4;
	pPixels = (unsigned char*)malloc((size_t)iStride * TUT_H);
	if ( pPixels == NULL ) return XGE_ERROR_OUT_OF_MEMORY;
	iRet = xgeRenderTargetReadPixels(&ctx->tTarget, pPixels, iStride);
	if ( iRet == XGE_OK ) {
		iRet = xgeImageSavePNG(ctx->sCapturePath, TUT_W, TUT_H, pPixels, iStride);
	}
	free(pPixels);
	if ( iRet == XGE_OK ) {
		ctx->bCaptureDone = 1;
		printf("capture saved: %s\n", ctx->sCapturePath);
	}
	return iRet;
}

static int tut_init(tut_ctx_t* ctx, const char* sTitle)
{
	xge_desc_t desc;
	int iRet;

	memset(ctx, 0, sizeof(*ctx));
	memset(&desc, 0, sizeof(desc));
	desc.iWidth = TUT_W + 40;
	desc.iHeight = TUT_H + 40;
	desc.sTitle = sTitle;
	desc.iFlags = XGE_INIT_WINDOW | XGE_INIT_VSYNC;
	desc.iRunMode = XGE_RUN_GAME_LOOP;
	iRet = xgeInit(&desc);
	if ( iRet != XGE_OK ) {
		printf("%s: xgeInit failed: %d\n", sTitle, iRet);
		return iRet;
	}
	iRet = xgeRenderTargetCreate(&ctx->tTarget, TUT_W, TUT_H);
	if ( iRet != XGE_OK ) {
		printf("%s: render target create failed: %d\n", sTitle, iRet);
		xgeUnit();
		return iRet;
	}
	return XGE_OK;
}

static void tut_shutdown(tut_ctx_t* ctx)
{
	xgeRenderTargetFree(&ctx->tTarget);
	xgeUnit();
}

/* 标准帧回调：渲染到 target → 截图 → 显示到窗口 → 退出 */
typedef void (*tut_draw_fn)(void);

static tut_draw_fn tut__draw_fn;

static int tut__frame(void* pUser)
{
	tut_ctx_t* ctx = (tut_ctx_t*)pUser;
	xge_pass_t pass;
	xge_draw_t draw;
	xge_texture texture;
	int ret;

	ret = xgeBegin();
	if ( ret != XGE_OK ) return ret;
	xgePassInit(&pass, &ctx->tTarget, XGE_PASS_CLEAR_COLOR, XGE_COLOR_RGBA(13, 17, 22, 255));
	ret = xgePassBegin(&pass);
	if ( ret == XGE_OK ) {
		if ( tut__draw_fn ) tut__draw_fn();
		ret = xgePassEnd(&pass);
	}
	if ( ret == XGE_OK ) ret = tut_save(ctx);
	/* 显示到窗口 */
	xgeClear(XGE_COLOR_RGBA(8, 10, 12, 255));
	memset(&draw, 0, sizeof(draw));
	texture = xgeRenderTargetTexture(&ctx->tTarget);
	draw.pTexture = texture;
	draw.tSrc.fX = 0.0f; draw.tSrc.fY = 0.0f;
	draw.tSrc.fW = (float)TUT_W; draw.tSrc.fH = (float)TUT_H;
	draw.tDst.fX = 20.0f; draw.tDst.fY = 20.0f;
	draw.tDst.fW = (float)TUT_W; draw.tDst.fH = (float)TUT_H;
	draw.iColor = XGE_COLOR_RGBA(255, 255, 255, 255);
	draw.iFlags = XGE_DRAW_SCREEN_SPACE;
	xgeDrawEx(&draw);
	ret = xgeEnd();
	if ( ret != XGE_OK ) return ret;
	ctx->iFrame++;
	if ( ctx->bCaptureDone ) xgeQuit();
	return XGE_OK;
}

/* 一键运行：初始化 → 解析参数 → 运行帧循环 → 清理 */
static int tut_run(tut_draw_fn fn, const char* sTitle, int argc, char** argv)
{
	tut_ctx_t ctx;
	int ret;

	ret = tut_init(&ctx, sTitle);
	if ( ret != XGE_OK ) return 1;
	ret = tut_parse(&ctx, argc, argv);
	if ( ret == 1 ) { tut_shutdown(&ctx); return 0; }
	if ( ret != XGE_OK ) { tut_shutdown(&ctx); return 1; }
	tut__draw_fn = fn;
	ret = xgeRun(tut__frame, &ctx);
	tut_shutdown(&ctx);
	return (ret == XGE_OK) ? 0 : 1;
}

#endif /* TUT_CAPTURE_H */
