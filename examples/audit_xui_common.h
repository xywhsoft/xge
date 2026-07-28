/* audit_xui_common.h — Shared framework for XUI visual audit programs */
#ifndef AUDIT_XUI_COMMON_H
#define AUDIT_XUI_COMMON_H

#include "../xge.h"
#include "../xui.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef AUDIT_XUI_W
#define AUDIT_XUI_W 940
#endif
#ifndef AUDIT_XUI_H
#define AUDIT_XUI_H 560
#endif

typedef struct audit_xui_ctx_t {
	xge_render_target_t tTarget;
	xui_proxy_t tProxy;
	xui_context pContext;
	xui_surface pTargetSurface;
	xui_font pFont;
	xui_widget pRoot;
	char sCapturePath[260];
	int bCaptureDone;
	int iFrame;
	int iMaxFrames;
} audit_xui_ctx_t;

static const char* audit_xui_find_ttf(void)
{
	static const char* paths[] = {
		"C:\\Windows\\Fonts\\msyh.ttc",
		"C:\\Windows\\Fonts\\segoeui.ttf",
		"C:\\Windows\\Fonts\\arial.ttf",
		"C:\\Windows\\Fonts\\calibri.ttf",
		"C:\\Windows\\Fonts\\simhei.ttf"
	};
	int i;
	for ( i = 0; i < 5; i++ ) {
		FILE* f = fopen(paths[i], "rb");
		if ( f ) { fclose(f); return paths[i]; }
	}
	return NULL;
}

static int audit_xui_parse(audit_xui_ctx_t* ctx, int argc, char** argv)
{
	int i;
	for ( i = 1; i < argc; i++ ) {
		if ( strcmp(argv[i], "--capture") == 0 && i+1 < argc )
			snprintf(ctx->sCapturePath, 260, "%s", argv[++i]);
		else if ( strncmp(argv[i], "--capture=", 10) == 0 )
			snprintf(ctx->sCapturePath, 260, "%s", argv[i]+10);
		else if ( strcmp(argv[i], "--frames") == 0 && i+1 < argc )
			ctx->iMaxFrames = atoi(argv[++i]);
		else if ( strncmp(argv[i], "--frames=", 9) == 0 )
			ctx->iMaxFrames = atoi(argv[i]+9);
	}
	return XGE_OK;
}

static int audit_xui_init(audit_xui_ctx_t* ctx, const char* title)
{
	xui_surface_desc_t sd;
	xge_desc_t desc;
	const char* fontPath;
	int ret;
	char savedPath[260];
	int savedMaxFrames = ctx->iMaxFrames;
	memcpy(savedPath, ctx->sCapturePath, 260);

	memset(ctx, 0, sizeof(*ctx));
	memcpy(ctx->sCapturePath, savedPath, 260);
	ctx->iMaxFrames = savedMaxFrames;
	memset(&desc, 0, sizeof(desc));
	desc.iWidth = AUDIT_XUI_W;
	desc.iHeight = AUDIT_XUI_H;
	desc.sTitle = title;
	desc.iFlags = XGE_INIT_WINDOW;
	desc.iRunMode = XGE_RUN_GAME_LOOP;
	ret = xgeInit(&desc);
	if ( ret != XGE_OK ) return ret;

	ret = xgeRenderTargetCreate(&ctx->tTarget, AUDIT_XUI_W, AUDIT_XUI_H);
	if ( ret != XGE_OK ) { xgeUnit(); return ret; }

	ctx->tProxy = xuiProxyXge();
	ret = xuiCreate(&ctx->pContext);
	if ( ret != XUI_OK ) return ret;
	ret = xuiSetProxy(ctx->pContext, &ctx->tProxy);
	if ( ret != XUI_OK ) return ret;
	(void)xuiInputViewport(ctx->pContext, (float)AUDIT_XUI_W, (float)AUDIT_XUI_H);
	(void)xuiSetViewportSize(ctx->pContext, (float)AUDIT_XUI_W, (float)AUDIT_XUI_H);

	memset(&sd, 0, sizeof(sd));
	sd.iKind = XUI_SURFACE_KIND_TEXTURE;
	sd.iFormat = XUI_SURFACE_FORMAT_RGBA8;
	sd.iWidth = AUDIT_XUI_W;
	sd.iHeight = AUDIT_XUI_H;
	sd.iFlags = XUI_SURFACE_ALPHA_PREMULTIPLIED | XUI_SURFACE_USAGE_TARGET;
	ret = ctx->tProxy.surfaceCreate(&ctx->tProxy, &ctx->pTargetSurface, &sd);
	if ( ret != XUI_OK ) return ret;

	fontPath = audit_xui_find_ttf();
	if ( !fontPath ) return XUI_ERROR_FILE_NOT_FOUND;
	ret = ctx->tProxy.fontLoadFile(&ctx->tProxy, &ctx->pFont, fontPath, 16.0f, XUI_FONT_FORMAT_TTF);
	if ( ret != XUI_OK ) return ret;
	(void)xuiSetDefaultFont(ctx->pContext, ctx->pFont);

	ret = xuiWidgetCreate(ctx->pContext, &ctx->pRoot);
	if ( ret != XUI_OK ) return ret;
	(void)xuiWidgetSetRect(ctx->pRoot, (xui_rect_t){0,0,(float)AUDIT_XUI_W,(float)AUDIT_XUI_H});
	(void)xuiWidgetSetLayoutType(ctx->pRoot, XUI_LAYOUT_COLUMN);
	(void)xuiSetRootWidget(ctx->pContext, ctx->pRoot);

	return XGE_OK;
}

static int audit_xui_capture(audit_xui_ctx_t* ctx)
{
	unsigned char* px; int stride, ret;
	if ( !ctx->sCapturePath[0] || ctx->bCaptureDone ) return XGE_OK;
	stride = AUDIT_XUI_W * 4;
	px = (unsigned char*)malloc((size_t)stride * AUDIT_XUI_H);
	if ( !px ) return XGE_ERROR_OUT_OF_MEMORY;
	ret = ctx->tProxy.surfaceReadRGBA(&ctx->tProxy, ctx->pTargetSurface, px, stride);
	if ( ret == XGE_OK ) ret = xgeImageSavePNG(ctx->sCapturePath, AUDIT_XUI_W, AUDIT_XUI_H, px, stride);
	free(px);
	if ( ret == XGE_OK ) { ctx->bCaptureDone = 1; printf("capture saved: %s\n", ctx->sCapturePath); }
	return ret;
}

static void audit_xui_shutdown(audit_xui_ctx_t* ctx)
{
	if ( ctx->pContext ) xuiDestroy(ctx->pContext);
	if ( ctx->pFont ) ctx->tProxy.fontDestroy(&ctx->tProxy, ctx->pFont);
	if ( ctx->pTargetSurface ) ctx->tProxy.surfaceDestroy(&ctx->tProxy, ctx->pTargetSurface);
	xgeRenderTargetFree(&ctx->tTarget);
	xgeUnit();
}

#endif /* AUDIT_XUI_COMMON_H */
