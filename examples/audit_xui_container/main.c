#include "../audit_xui_common.h"
static audit_xui_ctx_t g_ctx;
static int create_ui(audit_xui_ctx_t* ctx) {
	xui_widget w, panel;
	xui_label_desc_t ld;
	memset(&ld,0,sizeof(ld)); ld.iSize=sizeof(ld); ld.pFont=ctx->pFont;
	/* Panel */
	ld.sText="Panel (column layout):"; xuiLabelCreate(ctx->pContext,&w,&ld); xuiWidgetAddChild(ctx->pRoot,w);
	xuiWidgetCreate(ctx->pContext,&panel);
	xuiWidgetSetLayoutType(panel, XUI_LAYOUT_ROW);
	xuiWidgetAddChild(ctx->pRoot, panel);
	ld.sText="Child A"; xuiLabelCreate(ctx->pContext,&w,&ld); xuiWidgetAddChild(panel,w);
	ld.sText="Child B"; xuiLabelCreate(ctx->pContext,&w,&ld); xuiWidgetAddChild(panel,w);
	ld.sText="Child C"; xuiLabelCreate(ctx->pContext,&w,&ld); xuiWidgetAddChild(panel,w);
	/* Separator */
	ld.sText="Separator:"; xuiLabelCreate(ctx->pContext,&w,&ld); xuiWidgetAddChild(ctx->pRoot,w);
	xuiWidgetCreate(ctx->pContext,&w); xuiWidgetAddChild(ctx->pRoot,w);
	/* ScrollView placeholder */
	ld.sText="ScrollView (nested):"; xuiLabelCreate(ctx->pContext,&w,&ld); xuiWidgetAddChild(ctx->pRoot,w);
	xuiWidgetCreate(ctx->pContext,&panel);
	xuiWidgetSetLayoutType(panel, XUI_LAYOUT_COLUMN);
	xuiWidgetAddChild(ctx->pRoot, panel);
	{ int i; char buf[32];
	  for(i=0;i<5;i++){snprintf(buf,32,"Scroll item %d",i+1); ld.sText=buf; xuiLabelCreate(ctx->pContext,&w,&ld); xuiWidgetAddChild(panel,w);}
	}
	return XUI_OK;
}
static int frame(void* user) {
	audit_xui_ctx_t* ctx = (audit_xui_ctx_t*)user;
	xui_rect_i_t tFull; int ret;
	ret = xgeBegin(); if (ret != XGE_OK) return ret;
	(void)xuiDispatchPendingEvents(ctx->pContext);
	(void)xuiLayout(ctx->pContext);
	(void)xuiUpdate(ctx->pContext, xgeGetDelta());
	(void)ctx->tProxy.surfaceClear(&ctx->tProxy, ctx->pTargetSurface, XUI_COLOR_RGBA(40,45,55,255));
	tFull = (xui_rect_i_t){0,0,AUDIT_XUI_W,AUDIT_XUI_H};
	(void)xuiRender(ctx->pContext, ctx->pTargetSurface, &tFull, 1);
	ret = audit_xui_capture(ctx);
	if (ret == XGE_OK) ret = xgeEnd();
	if (ret != XGE_OK) return ret;
	ctx->iFrame++;
	if (ctx->bCaptureDone || ((ctx->iMaxFrames > 0) && (ctx->iFrame >= ctx->iMaxFrames))) xgeQuit();
	return XGE_OK;
}
int main(int argc, char** argv) {
	int ret; setbuf(stdout, NULL);
	memset(&g_ctx, 0, sizeof(g_ctx));
	audit_xui_parse(&g_ctx, argc, argv);
	ret = audit_xui_init(&g_ctx, "audit_xui_container");
	if (ret != XGE_OK) { printf("init failed: %d\n", ret); return 1; }
	ret = create_ui(&g_ctx);
	if (ret != XUI_OK) { printf("create_ui failed: %d\n", ret); audit_xui_shutdown(&g_ctx); return 1; }
	ret = xgeRun(frame, &g_ctx);
	audit_xui_shutdown(&g_ctx);
	return (ret == XGE_OK) ? 0 : 1;
}
