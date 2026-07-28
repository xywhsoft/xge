/* ch191 — 输入分发 */
#include "tut_capture_xui.h"

static int create_ui(tut_xui_ctx_t* ctx)
{
	xui_panel_desc_t pdesc;
	xui_button_desc_t bdesc;
	xui_widget pPanel, pBtn;
	int ret;

	/* 外层面板 */
	memset(&pdesc, 0, sizeof(pdesc));
	pdesc.iSize = sizeof(pdesc);
	ret = xuiPanelCreate(ctx->pContext, &pPanel, &pdesc);
	if (ret != XUI_OK) return ret;
	xuiWidgetSetMargin(pPanel, (xui_thickness_t){30.0f, 30.0f, 30.0f, 30.0f});
	xuiWidgetSetSizeMode(pPanel, XUI_SIZE_FILL, XUI_SIZE_FILL);
	xuiWidgetSetLayoutType(pPanel, XUI_LAYOUT_COLUMN);
	xuiWidgetAddChild(ctx->pRoot, pPanel);

	/* 按钮 — 接收输入事件 */
	memset(&bdesc, 0, sizeof(bdesc));
	bdesc.iSize = sizeof(bdesc);
	bdesc.pFont = ctx->pFont;
	bdesc.sText = "Click / Key / Scroll";
	ret = xuiButtonCreate(ctx->pContext, &pBtn, &bdesc);
	if (ret != XUI_OK) return ret;
	xuiWidgetSetMargin(pBtn, (xui_thickness_t){20.0f, 20.0f, 0.0f, 0.0f});
	xuiWidgetSetPreferredSize(pBtn, (xui_vec2_t){200.0f, 40.0f});
	return xuiWidgetAddChild(pPanel, pBtn);
}

int main(int argc, char** argv) { return tut_xui_run(create_ui, "ch191", argc, argv); }
