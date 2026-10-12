/* ch199 — 自定义控件开发 */
#include "tut_capture_xui.h"

static int create_ui(tut_xui_ctx_t* ctx)
{
	xui_panel_desc_t pdesc;
	xui_label_desc_t ldesc;
	xui_widget pPanel, pLabel;
	int ret;

	/* 自定义控件容器 */
	memset(&pdesc, 0, sizeof(pdesc));
	pdesc.iSize = sizeof(pdesc);
	ret = xuiPanelCreate(ctx->pContext, &pPanel, &pdesc);
	if (ret != XUI_OK) return ret;
	xuiWidgetSetMargin(pPanel, (xui_thickness_t){40.0f, 40.0f, 40.0f, 40.0f});
	xuiWidgetSetPreferredSize(pPanel, (xui_vec2_t){300.0f, 200.0f});
	xuiWidgetSetLayoutType(pPanel, XUI_LAYOUT_COLUMN);
	xuiWidgetAddChild(ctx->pRoot, pPanel);

	/* 自定义控件内容 */
	memset(&ldesc, 0, sizeof(ldesc));
	ldesc.iSize = sizeof(ldesc);
	ldesc.pFont = ctx->pFont;
	ldesc.sText = "Custom Widget\n(override paint/hitTest)";
	ret = xuiLabelCreate(ctx->pContext, &pLabel, &ldesc);
	if (ret != XUI_OK) return ret;
	xuiWidgetSetMargin(pLabel, (xui_thickness_t){20.0f, 20.0f, 0.0f, 0.0f});
	return xuiWidgetAddChild(pPanel, pLabel);
}

int main(int argc, char** argv) { return tut_xui_run(create_ui, "ch199", argc, argv); }
