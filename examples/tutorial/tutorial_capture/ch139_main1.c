/* ch139 — NumericInput 数字输入 */
#include "tut_capture_xui.h"

static int create_ui(tut_xui_ctx_t* ctx)
{
	xui_numeric_input_desc_t desc;
	xui_widget pInput;
	int ret;

	memset(&desc, 0, sizeof(desc));
	desc.iSize = sizeof(desc);
	desc.pFont = ctx->pFont;

	/* 整数输入 */
	desc.fMin = 0.0f;
	desc.fMax = 100.0f;
	desc.fStep = 1.0f;
	desc.fValue = 42.0f;
	desc.bInteger = 1;
	desc.bSpinnerVisible = 1;
	ret = xuiNumericInputCreate(ctx->pContext, &pInput, &desc);
	if (ret != XUI_OK) return ret;
	xuiWidgetSetMargin(pInput, (xui_thickness_t){20.0f, 20.0f, 0.0f, 0.0f});
	xuiWidgetSetPreferredSize(pInput, (xui_vec2_t){160.0f, 32.0f});
	xuiWidgetAddChild(ctx->pRoot, pInput);

	/* 浮点输入 */
	desc.fMin = 0.0f;
	desc.fMax = 1.0f;
	desc.fStep = 0.01f;
	desc.fValue = 0.75f;
	desc.bInteger = 0;
	desc.iPrecision = 2;
	ret = xuiNumericInputCreate(ctx->pContext, &pInput, &desc);
	if (ret != XUI_OK) return ret;
	xuiWidgetSetMargin(pInput, (xui_thickness_t){20.0f, 12.0f, 0.0f, 0.0f});
	xuiWidgetSetPreferredSize(pInput, (xui_vec2_t){160.0f, 32.0f});
	return xuiWidgetAddChild(ctx->pRoot, pInput);
}

int main(int argc, char** argv) { return tut_xui_run(create_ui, "ch139", argc, argv); }
