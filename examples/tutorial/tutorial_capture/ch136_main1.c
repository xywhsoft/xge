/* ch136 — Toggle 开关 */
#include "tut_capture_xui.h"

static int create_ui(tut_xui_ctx_t* ctx)
{
	xui_toggle_desc_t desc;
	xui_widget pToggle;
	int ret;

	memset(&desc, 0, sizeof(desc));
	desc.iSize = sizeof(desc);
	desc.pFont = ctx->pFont;

	/* 关闭状态 */
	desc.sText = "Wi-Fi";
	desc.bChecked = 0;
	ret = xuiToggleCreate(ctx->pContext, &pToggle, &desc);
	if (ret != XUI_OK) return ret;
	xuiWidgetSetMargin(pToggle, (xui_thickness_t){20.0f, 20.0f, 0.0f, 0.0f});
	xuiWidgetAddChild(ctx->pRoot, pToggle);

	/* 开启状态 */
	desc.sText = "Bluetooth";
	desc.bChecked = 1;
	ret = xuiToggleCreate(ctx->pContext, &pToggle, &desc);
	if (ret != XUI_OK) return ret;
	xuiWidgetSetMargin(pToggle, (xui_thickness_t){20.0f, 12.0f, 0.0f, 0.0f});
	xuiWidgetAddChild(ctx->pRoot, pToggle);

	/* 带文字 */
	desc.sText = "Notifications";
	desc.bChecked = 1;
	desc.sCheckedText = "ON";
	desc.sUncheckedText = "OFF";
	ret = xuiToggleCreate(ctx->pContext, &pToggle, &desc);
	if (ret != XUI_OK) return ret;
	xuiWidgetSetMargin(pToggle, (xui_thickness_t){20.0f, 12.0f, 0.0f, 0.0f});
	return xuiWidgetAddChild(ctx->pRoot, pToggle);
}

int main(int argc, char** argv) { return tut_xui_run(create_ui, "ch136", argc, argv); }
