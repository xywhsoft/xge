/* tut_capture_xui.h — 教程截图公共框架（XUI 章节用）
 * 提供 XUI proxy/context/font/root 初始化 + 渲染截图样板。
 * 用法：#include "tut_capture_xui.h"，实现 create_ui()，调用 tut_xui_run()。
 */
#ifndef TUT_CAPTURE_XUI_H
#define TUT_CAPTURE_XUI_H

#include "tut_capture.h"
#include "../../xui.h"

#ifndef TUT_XUI_W
#define TUT_XUI_W 940
#endif
#ifndef TUT_XUI_H
#define TUT_XUI_H 560
#endif

/* 覆盖 tut_capture.h 的默认尺寸 */
#undef TUT_W
#undef TUT_H
#define TUT_W TUT_XUI_W
#define TUT_H TUT_XUI_H

typedef struct tut_xui_ctx_t {
	tut_ctx_t tBase;
	xui_proxy_t tProxy;
	xui_context pContext;
	xui_surface pTargetSurface;
	xui_font pFont;
	xui_widget pRoot;
} tut_xui_ctx_t;

static const char* tut_xui_find_ttf(void)
{
	static const char* arrPaths[] = {
		"C:\\Windows\\Fonts\\msyh.ttc",
		"C:\\Windows\\Fonts\\segoeui.ttf",
		"C:\\Windows\\Fonts\\arial.ttf",
		"C:\\Windows\\Fonts\\calibri.ttf",
		"C:\\Windows\\Fonts\\simhei.ttf"
	};
	int i;
	for ( i = 0; i < (int)(sizeof(arrPaths) / sizeof(arrPaths[0])); i++ ) {
		FILE* f = fopen(arrPaths[i], "rb");
		if ( f ) { fclose(f); return arrPaths[i]; }
	}
	return NULL;
}

static int tut_xui_init(tut_xui_ctx_t* ctx, const char* sTitle)
{
	xui_surface_desc_t tSurfDesc;
	const char* sFontPath;
	int iRet;

	memset(ctx, 0, sizeof(*ctx));
	/* 初始化 XGE 窗口 + render target */
	iRet = tut_init(&ctx->tBase, sTitle);
	if ( iRet != XGE_OK ) return iRet;

	/* XUI proxy + context */
	ctx->tProxy = xuiProxyXge();
	iRet = xuiCreate(&ctx->pContext);
	if ( iRet != XUI_OK ) { printf("%s: xuiCreate failed: %d\n", sTitle, iRet); return iRet; }
	iRet = xuiSetProxy(ctx->pContext, &ctx->tProxy);
	if ( iRet != XUI_OK ) { printf("%s: xuiSetProxy failed: %d\n", sTitle, iRet); return iRet; }
	iRet = xuiInputViewport(ctx->pContext, (float)TUT_XUI_W, (float)TUT_XUI_H);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiSetViewportSize(ctx->pContext, (float)TUT_XUI_W, (float)TUT_XUI_H);

	/* 目标 surface */
	memset(&tSurfDesc, 0, sizeof(tSurfDesc));
	tSurfDesc.iKind = XUI_SURFACE_KIND_TEXTURE;
	tSurfDesc.iFormat = XUI_SURFACE_FORMAT_RGBA8;
	tSurfDesc.iWidth = TUT_XUI_W;
	tSurfDesc.iHeight = TUT_XUI_H;
	tSurfDesc.iFlags = XUI_SURFACE_ALPHA_PREMULTIPLIED | XUI_SURFACE_USAGE_TARGET;
	iRet = ctx->tProxy.surfaceCreate(&ctx->tProxy, &ctx->pTargetSurface, &tSurfDesc);
	if ( iRet != XUI_OK ) { printf("%s: surfaceCreate failed: %d\n", sTitle, iRet); return iRet; }

	/* 字体 */
	sFontPath = tut_xui_find_ttf();
	if ( sFontPath == NULL ) { printf("%s: no system TTF font found\n", sTitle); return XUI_ERROR_FILE_NOT_FOUND; }
	iRet = ctx->tProxy.fontLoadFile(&ctx->tProxy, &ctx->pFont, sFontPath, 16.0f, XUI_FONT_FORMAT_TTF);
	if ( iRet != XUI_OK ) { printf("%s: fontLoadFile failed: %d\n", sTitle, iRet); return iRet; }
	(void)xuiSetDefaultFont(ctx->pContext, ctx->pFont);

	/* 根控件 */
	iRet = xuiWidgetCreate(ctx->pContext, &ctx->pRoot);
	if ( iRet != XUI_OK ) return iRet;
	{
		xui_cache_policy_t policy;
		memset(&policy, 0, sizeof(policy));
		policy.iSize = sizeof(policy);
		policy.iPolicy = XUI_CACHE_POLICY_SELF;
		policy.iFlags = XUI_CACHE_CLEAR_ON_UPDATE;
		(void)xuiWidgetSetCachePolicy(ctx->pRoot, &policy);
	}
	(void)xuiWidgetSetRect(ctx->pRoot, (xui_rect_t){0.0f, 0.0f, (float)TUT_XUI_W, (float)TUT_XUI_H});
	(void)xuiWidgetSetLayoutType(ctx->pRoot, XUI_LAYOUT_COLUMN);
	(void)xuiSetRootWidget(ctx->pContext, ctx->pRoot);

	return XGE_OK;
}

static int tut_xui_save(tut_xui_ctx_t* ctx)
{
	unsigned char* pPixels;
	int iStride;
	int iRet;

	if ( ctx->tBase.sCapturePath[0] == '\0' || ctx->tBase.bCaptureDone ) {
		return XGE_OK;
	}
	iStride = TUT_XUI_W * 4;
	pPixels = (unsigned char*)malloc((size_t)iStride * TUT_XUI_H);
	if ( pPixels == NULL ) return XGE_ERROR_OUT_OF_MEMORY;
	iRet = ctx->tProxy.surfaceReadRGBA(&ctx->tProxy, ctx->pTargetSurface, pPixels, iStride);
	if ( iRet == XGE_OK ) {
		iRet = xgeImageSavePNG(ctx->tBase.sCapturePath, TUT_XUI_W, TUT_XUI_H, pPixels, iStride);
	}
	free(pPixels);
	if ( iRet == XGE_OK ) {
		ctx->tBase.bCaptureDone = 1;
		printf("capture saved: %s\n", ctx->tBase.sCapturePath);
	}
	return iRet;
}

static void tut_xui_shutdown(tut_xui_ctx_t* ctx)
{
	if ( ctx->pContext ) xuiDestroy(ctx->pContext);
	if ( ctx->pFont ) ctx->tProxy.fontDestroy(&ctx->tProxy, ctx->pFont);
	if ( ctx->pTargetSurface ) ctx->tProxy.surfaceDestroy(&ctx->tProxy, ctx->pTargetSurface);
	tut_shutdown(&ctx->tBase);
}

/* 标准 XUI 帧回调 */
typedef int (*tut_xui_create_fn)(tut_xui_ctx_t* ctx);

static tut_xui_ctx_t* tut_xui__ctx;

static int tut_xui__frame(void* pUser)
{
	tut_xui_ctx_t* ctx = (tut_xui_ctx_t*)pUser;
	xui_rect_i_t tFull;
	int ret;

	ret = xgeBegin();
	if ( ret != XGE_OK ) return ret;
	(void)xuiDispatchPendingEvents(ctx->pContext);
	(void)xuiLayout(ctx->pContext);
	(void)xuiUpdate(ctx->pContext, xgeGetDelta());
	(void)ctx->tProxy.surfaceClear(&ctx->tProxy, ctx->pTargetSurface, XUI_COLOR_RGBA(229, 235, 244, 255));
	tFull = (xui_rect_i_t){0, 0, TUT_XUI_W, TUT_XUI_H};
	(void)xuiRender(ctx->pContext, ctx->pTargetSurface, &tFull, 1);
	ret = tut_xui_save(ctx);
	if ( ret == XGE_OK ) ret = xgeEnd();
	if ( ret != XGE_OK ) return ret;
	ctx->tBase.iFrame++;
	if ( ctx->tBase.bCaptureDone ||
		(ctx->tBase.iMaxFrames > 0 && ctx->tBase.iFrame >= ctx->tBase.iMaxFrames) ) xgeQuit();
	return XGE_OK;
}

/* 一键运行 XUI 截图 */
static int tut_xui_run(tut_xui_create_fn fn, const char* sTitle, int argc, char** argv)
{
	tut_xui_ctx_t ctx;
	int ret;

	ret = tut_xui_init(&ctx, sTitle);
	if ( ret != XGE_OK ) return 1;
	ret = tut_parse(&ctx.tBase, argc, argv);
	if ( ret == 1 ) { tut_xui_shutdown(&ctx); return 0; }
	if ( ret != XGE_OK ) { tut_xui_shutdown(&ctx); return 1; }
	ret = fn(&ctx);
	if ( ret != XGE_OK ) { printf("%s: create_ui failed: %d\n", sTitle, ret); tut_xui_shutdown(&ctx); return 1; }
	tut_xui__ctx = &ctx;
	ret = xgeRun(tut_xui__frame, &ctx);
	tut_xui_shutdown(&ctx);
	return (ret == XGE_OK) ? 0 : 1;
}

#endif /* TUT_CAPTURE_XUI_H */
