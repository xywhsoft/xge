/* ch134 — Checkbox 复选框 */
#include "tut_capture_xui.h"

static int create_ui(tut_xui_ctx_t* ctx)
{
	xui_checkbox_desc_t desc;
	xui_widget pCheck;
	int ret;

	memset(&desc, 0, sizeof(desc));
	desc.iSize = sizeof(desc);
	desc.pFont = ctx->pFont;

	/* 未选中 */
	desc.sText = "Unchecked";
	desc.bChecked = 0;
	ret = xuiCheckBoxCreate(ctx->pContext, &pCheck, &desc);
	if (ret != XUI_OK) return ret;
	xuiWidgetSetMargin(pCheck, (xui_thickness_t){20.0f, 20.0f, 0.0f, 0.0f});
	xuiWidgetAddChild(ctx->pRoot, pCheck);

	/* 已选中 */
	desc.sText = "Checked";
	desc.bChecked = 1;
	ret = xuiCheckBoxCreate(ctx->pContext, &pCheck, &desc);
	if (ret != XUI_OK) return ret;
	xuiWidgetSetMargin(pCheck, (xui_thickness_t){20.0f, 12.0f, 0.0f, 0.0f});
	xuiWidgetAddChild(ctx->pRoot, pCheck);

	/* 禁用 */
	desc.sText = "Disabled";
	desc.bChecked = 1;
	ret = xuiCheckBoxCreate(ctx->pContext, &pCheck, &desc);
	if (ret != XUI_OK) return ret;
	xuiWidgetSetMargin(pCheck, (xui_thickness_t){20.0f, 12.0f, 0.0f, 0.0f});
	xuiWidgetSetEnabled(pCheck, 0);
	return xuiWidgetAddChild(ctx->pRoot, pCheck);
}

int main(int argc, char** argv) { return tut_xui_run(create_ui, "ch134", argc, argv); }
