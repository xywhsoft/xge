#include "../audit_xui_common.h"
static audit_xui_ctx_t g_ctx;
static int create_ui(audit_xui_ctx_t* ctx) {
	xui_widget w; xui_label_desc_t ld; xui_progress_desc_t pd;
	memset(&ld,0,sizeof(ld)); ld.iSize=sizeof(ld); ld.pFont=ctx->pFont;
	ld.sText="Feedback Controls:"; xuiLabelCreate(ctx->pContext,&w,&ld); xuiWidgetAddChild(ctx->pRoot,w);
	memset(&pd,0,sizeof(pd)); pd.iSize=sizeof(pd);
	pd.fMin=0; pd.fMax=100; pd.fValue=0; xuiProgressCreate(ctx->pContext,&w,&pd); xuiWidgetAddChild(ctx->pRoot,w);
	pd.fValue=50; xuiProgressCreate(ctx->pContext,&w,&pd); xuiWidgetAddChild(ctx->pRoot,w);
	pd.fValue=100; xuiProgressCreate(ctx->pContext,&w,&pd); xuiWidgetAddChild(ctx->pRoot,w);
	ld.sText="Toast/MsgBox shown on interaction"; xuiLabelCreate(ctx->pContext,&w,&ld); xuiWidgetAddChild(ctx->pRoot,w);
	ld.sText="Progress indeterminate (visual check)"; xuiLabelCreate(ctx->pContext,&w,&ld); xuiWidgetAddChild(ctx->pRoot,w);
	return XUI_OK;
}
static int frame(void* user) {
	return audit_xui_frame((audit_xui_ctx_t*)user);
}
int main(int argc, char** argv) {
	int ret; setbuf(stdout, NULL);
	memset(&g_ctx, 0, sizeof(g_ctx));
	audit_xui_parse(&g_ctx, argc, argv);
	ret = audit_xui_init(&g_ctx, "audit_xui_feedback");
	if (ret != XGE_OK) { printf("init failed: %d\n", ret); return 1; }
	ret = create_ui(&g_ctx);
	if (ret != XUI_OK) { printf("create_ui failed: %d\n", ret); audit_xui_shutdown(&g_ctx); return 1; }
	ret = xgeRun(frame, &g_ctx);
	audit_xui_shutdown(&g_ctx);
	return (ret == XGE_OK) ? 0 : 1;
}
