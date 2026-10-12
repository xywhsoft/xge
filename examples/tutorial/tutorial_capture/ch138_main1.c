/* ch138 — Input 输入框 */
#include "tut_capture_xui.h"

static int create_ui(tut_xui_ctx_t* ctx)
{
	xui_input_desc_t desc;
	xui_widget pInput;
	int ret;

	memset(&desc, 0, sizeof(desc));
	desc.iSize = sizeof(desc);
	desc.pFont = ctx->pFont;

	/* 普通输入框 */
	desc.sText = "";
	desc.sPlaceholder = "Enter text...";
	ret = xuiInputCreate(ctx->pContext, &pInput, &desc);
	if (ret != XUI_OK) return ret;
	xuiWidgetSetMargin(pInput, (xui_thickness_t){20.0f, 20.0f, 0.0f, 0.0f});
	xuiWidgetSetPreferredSize(pInput, (xui_vec2_t){280.0f, 32.0f});
	xuiWidgetAddChild(ctx->pRoot, pInput);

	/* 带初始文本 */
	desc.sText = "Hello XUI";
	desc.sPlaceholder = NULL;
	ret = xuiInputCreate(ctx->pContext, &pInput, &desc);
	if (ret != XUI_OK) return ret;
	xuiWidgetSetMargin(pInput, (xui_thickness_t){20.0f, 12.0f, 0.0f, 0.0f});
	xuiWidgetSetPreferredSize(pInput, (xui_vec2_t){280.0f, 32.0f});
	xuiWidgetAddChild(ctx->pRoot, pInput);

	/* 密码输入框 */
	desc.sText = "secret123";
	desc.bPassword = 1;
	ret = xuiInputCreate(ctx->pContext, &pInput, &desc);
	if (ret != XUI_OK) return ret;
	xuiWidgetSetMargin(pInput, (xui_thickness_t){20.0f, 12.0f, 0.0f, 0.0f});
	xuiWidgetSetPreferredSize(pInput, (xui_vec2_t){280.0f, 32.0f});
	return xuiWidgetAddChild(ctx->pRoot, pInput);
}

int main(int argc, char** argv) { return tut_xui_run(create_ui, "ch138", argc, argv); }
