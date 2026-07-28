/* ch125 — 控件状态 */
#include "tut_capture_xui.h"

static int create_ui(tut_xui_ctx_t* ctx)
{
	xui_button_desc_t desc;
	xui_widget pBtn1, pBtn2;
	int ret;

	memset(&desc, 0, sizeof(desc));
	desc.iSize = sizeof(desc);
	desc.pFont = ctx->pFont;

	/* 正常状态按钮 */
	desc.sText = "Normal State";
	ret = xuiButtonCreate(ctx->pContext, &pBtn1, &desc);
	if (ret != XUI_OK) return ret;
	xuiWidgetSetMargin(pBtn1, (xui_thickness_t){20.0f, 20.0f, 0.0f, 0.0f});
	xuiWidgetSetPreferredSize(pBtn1, (xui_vec2_t){160.0f, 36.0f});
	xuiWidgetAddChild(ctx->pRoot, pBtn1);

	/* 禁用状态按钮 */
	desc.sText = "Disabled State";
	ret = xuiButtonCreate(ctx->pContext, &pBtn2, &desc);
	if (ret != XUI_OK) return ret;
	xuiWidgetSetMargin(pBtn2, (xui_thickness_t){20.0f, 12.0f, 0.0f, 0.0f});
	xuiWidgetSetPreferredSize(pBtn2, (xui_vec2_t){160.0f, 36.0f});
	xuiWidgetSetEnabled(pBtn2, 0);
	return xuiWidgetAddChild(ctx->pRoot, pBtn2);
}

int main(int argc, char** argv) { return tut_xui_run(create_ui, "ch125", argc, argv); }
