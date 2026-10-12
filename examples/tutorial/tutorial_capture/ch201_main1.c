/* ch201 — XGE + XUI 代理桥接 */
#include "tut_capture_xui.h"

static int create_ui(tut_xui_ctx_t* ctx)
{
	xui_label_desc_t ldesc;
	xui_button_desc_t bdesc;
	xui_widget pLabel, pBtn;
	int ret;

	/* 标题 */
	memset(&ldesc, 0, sizeof(ldesc));
	ldesc.iSize = sizeof(ldesc);
	ldesc.pFont = ctx->pFont;
	ldesc.sText = "XGE Scene + XUI Overlay";
	ret = xuiLabelCreate(ctx->pContext, &pLabel, &ldesc);
	if (ret != XUI_OK) return ret;
	xuiWidgetSetMargin(pLabel, (xui_thickness_t){30.0f, 20.0f, 0.0f, 0.0f});
	xuiWidgetAddChild(ctx->pRoot, pLabel);

	/* UI 按钮覆盖在场景上 */
	memset(&bdesc, 0, sizeof(bdesc));
	bdesc.iSize = sizeof(bdesc);
	bdesc.pFont = ctx->pFont;
	bdesc.sText = "UI Button";
	ret = xuiButtonCreate(ctx->pContext, &pBtn, &bdesc);
	if (ret != XUI_OK) return ret;
	xuiWidgetSetMargin(pBtn, (xui_thickness_t){30.0f, 16.0f, 0.0f, 0.0f});
	xuiWidgetSetPreferredSize(pBtn, (xui_vec2_t){160.0f, 40.0f});
	return xuiWidgetAddChild(ctx->pRoot, pBtn);
}

int main(int argc, char** argv) { return tut_xui_run(create_ui, "ch201", argc, argv); }
