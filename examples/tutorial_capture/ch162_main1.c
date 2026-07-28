/* ch162 — StatusBar 状态栏 */
#include "tut_capture_xui.h"

static int create_ui(tut_xui_ctx_t* ctx)
{
	xui_statusbar_desc_t desc;
	xui_widget pBar;
	int ret;

	memset(&desc, 0, sizeof(desc));
	desc.iSize = sizeof(desc);
	desc.pFont = ctx->pFont;
	ret = xuiStatusBarCreate(ctx->pContext, &pBar, &desc);
	if (ret != XUI_OK) return ret;
	xuiWidgetSetSizeMode(pBar, XUI_SIZE_FILL, XUI_SIZE_CONTENT);
	xuiWidgetSetAlign(pBar, XUI_ALIGN_STRETCH, XUI_ALIGN_END);
	xuiWidgetAddChild(ctx->pRoot, pBar);

	/* 添加状态栏项 */
	xuiStatusBarAddText(pBar, 0, "Ready", 120.0f, 0, 0);
	xuiStatusBarAddText(pBar, 1, "Line 12, Col 8", 140.0f, 0, 0);
	xuiStatusBarAddProgress(pBar, 2, 0.0f, 100.0f, 65.0f, 100.0f);
	xuiStatusBarAddText(pBar, 3, "UTF-8", 80.0f, 0, 0);
	return XUI_OK;
}

int main(int argc, char** argv) { return tut_xui_run(create_ui, "ch162", argc, argv); }
