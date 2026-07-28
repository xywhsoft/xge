/* ch133 — Button 按钮 */
#include "tut_capture_xui.h"

static int create_ui(tut_xui_ctx_t* ctx)
{
	xui_button_desc_t desc;
	xui_widget pBtn;
	int ret;

	memset(&desc, 0, sizeof(desc));
	desc.iSize = sizeof(desc);
	desc.pFont = ctx->pFont;

	/* 默认按钮 */
	desc.sText = "Default Button";
	ret = xuiButtonCreate(ctx->pContext, &pBtn, &desc);
	if (ret != XUI_OK) return ret;
	xuiWidgetSetMargin(pBtn, (xui_thickness_t){20.0f, 20.0f, 0.0f, 0.0f});
	xuiWidgetSetPreferredSize(pBtn, (xui_vec2_t){160.0f, 36.0f});
	xuiWidgetAddChild(ctx->pRoot, pBtn);

	/* 可选中按钮 */
	desc.sText = "Selectable";
	ret = xuiButtonCreate(ctx->pContext, &pBtn, &desc);
	if (ret != XUI_OK) return ret;
	xuiWidgetSetMargin(pBtn, (xui_thickness_t){20.0f, 12.0f, 0.0f, 0.0f});
	xuiWidgetSetPreferredSize(pBtn, (xui_vec2_t){160.0f, 36.0f});
	xuiButtonSetSelectable(pBtn, 1);
	xuiButtonSetSelected(pBtn, 1);
	xuiWidgetAddChild(ctx->pRoot, pBtn);

	/* 禁用按钮 */
	desc.sText = "Disabled";
	ret = xuiButtonCreate(ctx->pContext, &pBtn, &desc);
	if (ret != XUI_OK) return ret;
	xuiWidgetSetMargin(pBtn, (xui_thickness_t){20.0f, 12.0f, 0.0f, 0.0f});
	xuiWidgetSetPreferredSize(pBtn, (xui_vec2_t){160.0f, 36.0f});
	xuiWidgetSetEnabled(pBtn, 0);
	return xuiWidgetAddChild(ctx->pRoot, pBtn);
}

int main(int argc, char** argv) { return tut_xui_run(create_ui, "ch133", argc, argv); }
