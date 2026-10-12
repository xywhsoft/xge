#include "../audit_xui_common.h"
static audit_xui_ctx_t g_ctx;
static int create_ui(audit_xui_ctx_t* ctx) {
	xui_widget w, panel; xui_label_desc_t ld;
	memset(&ld,0,sizeof(ld)); ld.iSize=sizeof(ld); ld.pFont=ctx->pFont;
	ld.sText="Complex Controls:"; xuiLabelCreate(ctx->pContext,&w,&ld); xuiWidgetAddChild(ctx->pRoot,w);
	xuiWidgetCreate(ctx->pContext,&panel); xuiWidgetSetLayoutType(panel, XUI_LAYOUT_ROW); xuiWidgetAddChild(ctx->pRoot,panel);
	ld.sText="Dock Left"; xuiLabelCreate(ctx->pContext,&w,&ld); xuiWidgetAddChild(panel,w);
	ld.sText="Dock Center"; xuiLabelCreate(ctx->pContext,&w,&ld); xuiWidgetAddChild(panel,w);
	ld.sText="Dock Right"; xuiLabelCreate(ctx->pContext,&w,&ld); xuiWidgetAddChild(panel,w);
	ld.sText="Chart/Timeline placeholder"; xuiLabelCreate(ctx->pContext,&w,&ld); xuiWidgetAddChild(ctx->pRoot,w);
	ld.sText="Terminal placeholder"; xuiLabelCreate(ctx->pContext,&w,&ld); xuiWidgetAddChild(ctx->pRoot,w);
	return XUI_OK;
}
static int frame(void* user) {
	return audit_xui_frame((audit_xui_ctx_t*)user);
}
int main(int argc, char** argv) {
	int ret; setbuf(stdout, NULL);
	memset(&g_ctx, 0, sizeof(g_ctx));
	audit_xui_parse(&g_ctx, argc, argv);
	ret = audit_xui_init(&g_ctx, "audit_xui_complex");
	if (ret != XGE_OK) { printf("init failed: %d\n", ret); return 1; }
	ret = create_ui(&g_ctx);
	if (ret != XUI_OK) { printf("create_ui failed: %d\n", ret); audit_xui_shutdown(&g_ctx); return 1; }
	ret = xgeRun(frame, &g_ctx);
	audit_xui_shutdown(&g_ctx);
	return (ret == XGE_OK) ? 0 : 1;
}
