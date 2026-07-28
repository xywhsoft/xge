#include "../audit_xui_common.h"
static audit_xui_ctx_t g_ctx;
static int create_ui(audit_xui_ctx_t* ctx) {
	xui_widget w;
	xui_label_desc_t ld;
	xui_input_desc_t inDesc;
	xui_text_edit_desc_t teDesc;
	/* Labels */
	memset(&ld,0,sizeof(ld)); ld.iSize=sizeof(ld); ld.pFont=ctx->pFont;
	ld.sText="Label Normal"; xuiLabelCreate(ctx->pContext,&w,&ld); xuiWidgetAddChild(ctx->pRoot,w);
	ld.sText="Label Disabled"; xuiLabelCreate(ctx->pContext,&w,&ld); xuiWidgetSetEnabled(w,0); xuiWidgetAddChild(ctx->pRoot,w);
	ld.sText="Label with long text that should wrap or truncate depending on layout policy settings"; xuiLabelCreate(ctx->pContext,&w,&ld); xuiWidgetAddChild(ctx->pRoot,w);
	/* Inputs */
	memset(&inDesc,0,sizeof(inDesc)); inDesc.iSize=sizeof(inDesc); inDesc.pFont=ctx->pFont;
	inDesc.sText="Input text"; inDesc.sPlaceholder="placeholder...";
	xuiInputCreate(ctx->pContext,&w,&inDesc); xuiWidgetAddChild(ctx->pRoot,w);
	inDesc.sText=""; inDesc.sPlaceholder="Empty input";
	xuiInputCreate(ctx->pContext,&w,&inDesc); xuiWidgetAddChild(ctx->pRoot,w);
	inDesc.sText="Disabled";
	xuiInputCreate(ctx->pContext,&w,&inDesc); xuiWidgetSetEnabled(w,0); xuiWidgetAddChild(ctx->pRoot,w);
	/* TextEdit */
	memset(&teDesc,0,sizeof(teDesc)); teDesc.iSize=sizeof(teDesc); teDesc.pFont=ctx->pFont;
	teDesc.sText="Multi-line\ntext edit\ncontent";
	xuiTextEditCreate(ctx->pContext,&w,&teDesc); xuiWidgetAddChild(ctx->pRoot,w);
	teDesc.sText=""; teDesc.sPlaceholder="Empty text edit";
	xuiTextEditCreate(ctx->pContext,&w,&teDesc); xuiWidgetAddChild(ctx->pRoot,w);
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
	ret = audit_xui_init(&g_ctx, "audit_xui_text");
	if (ret != XGE_OK) { printf("init failed: %d\n", ret); return 1; }
	ret = create_ui(&g_ctx);
	if (ret != XUI_OK) { printf("create_ui failed: %d\n", ret); audit_xui_shutdown(&g_ctx); return 1; }
	ret = xgeRun(frame, &g_ctx);
	audit_xui_shutdown(&g_ctx);
	return (ret == XGE_OK) ? 0 : 1;
}
