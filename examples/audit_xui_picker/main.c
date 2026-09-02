#include "../audit_xui_common.h"
static audit_xui_ctx_t g_ctx;
static int create_ui(audit_xui_ctx_t* ctx) {
	xui_widget w; xui_label_desc_t ld; xui_button_desc_t bd; xui_slider_desc_t sd;
	memset(&ld,0,sizeof(ld)); ld.iSize=sizeof(ld); ld.pFont=ctx->pFont;
	ld.sText="Picker Controls:"; xuiLabelCreate(ctx->pContext,&w,&ld); xuiWidgetAddChild(ctx->pRoot,w);
	memset(&bd,0,sizeof(bd)); bd.iSize=sizeof(bd); bd.pFont=ctx->pFont;
	bd.sText="ColorPicker trigger"; xuiButtonCreate(ctx->pContext,&w,&bd); xuiWidgetAddChild(ctx->pRoot,w);
	bd.sText="DatePicker trigger"; xuiButtonCreate(ctx->pContext,&w,&bd); xuiWidgetAddChild(ctx->pRoot,w);
	bd.sText="ComboBox trigger"; xuiButtonCreate(ctx->pContext,&w,&bd); xuiWidgetAddChild(ctx->pRoot,w);
	ld.sText="Cascader placeholder"; xuiLabelCreate(ctx->pContext,&w,&ld); xuiWidgetAddChild(ctx->pRoot,w);
	memset(&sd,0,sizeof(sd)); sd.iSize=sizeof(sd); sd.fMin=0; sd.fMax=360; sd.fValue=180;
	xuiSliderCreate(ctx->pContext,&w,&sd); xuiWidgetAddChild(ctx->pRoot,w);
	return XUI_OK;
}
static int frame(void* user) {
	return audit_xui_frame((audit_xui_ctx_t*)user);
}
int main(int argc, char** argv) {
	int ret; setbuf(stdout, NULL);
	memset(&g_ctx, 0, sizeof(g_ctx));
	audit_xui_parse(&g_ctx, argc, argv);
	ret = audit_xui_init(&g_ctx, "audit_xui_picker");
	if (ret != XGE_OK) { printf("init failed: %d\n", ret); return 1; }
	ret = create_ui(&g_ctx);
	if (ret != XUI_OK) { printf("create_ui failed: %d\n", ret); audit_xui_shutdown(&g_ctx); return 1; }
	ret = xgeRun(frame, &g_ctx);
	audit_xui_shutdown(&g_ctx);
	return (ret == XGE_OK) ? 0 : 1;
}
