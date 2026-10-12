/* ch158 — Tooltip 工具提示 */
#include "tut_capture_xui.h"

static int create_ui(tut_xui_ctx_t* ctx)
{
	xui_button_desc_t desc;
	xui_widget pBtn;
	int ret;

	memset(&desc, 0, sizeof(desc));
	desc.iSize = sizeof(desc);
	desc.pFont = ctx->pFont;

	desc.sText = "Hover for Tooltip";
	ret = xuiButtonCreate(ctx->pContext, &pBtn, &desc);
	if (ret != XUI_OK) return ret;
	xuiWidgetSetMargin(pBtn, (xui_thickness_t){20.0f, 20.0f, 0.0f, 0.0f});
	xuiWidgetSetPreferredSize(pBtn, (xui_vec2_t){180.0f, 40.0f});
	xuiWidgetSetTooltipText(pBtn, "This is a tooltip message");
	xuiWidgetAddChild(ctx->pRoot, pBtn);

	desc.sText = "Another Button";
	ret = xuiButtonCreate(ctx->pContext, &pBtn, &desc);
	if (ret != XUI_OK) return ret;
	xuiWidgetSetMargin(pBtn, (xui_thickness_t){20.0f, 16.0f, 0.0f, 0.0f});
	xuiWidgetSetPreferredSize(pBtn, (xui_vec2_t){180.0f, 40.0f});
	xuiWidgetSetTooltipText(pBtn, "Second tooltip");
	return xuiWidgetAddChild(ctx->pRoot, pBtn);
}

int main(int argc, char** argv) { return tut_xui_run(create_ui, "ch158", argc, argv); }
