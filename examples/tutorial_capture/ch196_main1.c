/* ch196 — 拖拽系统 */
#include "tut_capture_xui.h"

static int create_ui(tut_xui_ctx_t* ctx)
{
	xui_panel_desc_t pdesc;
	xui_label_desc_t ldesc;
	xui_widget pPanel, pLabel;
	int ret;

	/* 可拖拽面板 */
	memset(&pdesc, 0, sizeof(pdesc));
	pdesc.iSize = sizeof(pdesc);
	ret = xuiPanelCreate(ctx->pContext, &pPanel, &pdesc);
	if (ret != XUI_OK) return ret;
	xuiWidgetSetRect(pPanel, (xui_rect_t){60.0f, 60.0f, 200.0f, 120.0f});
	xuiWidgetAddChild(ctx->pRoot, pPanel);

	memset(&ldesc, 0, sizeof(ldesc));
	ldesc.iSize = sizeof(ldesc);
	ldesc.pFont = ctx->pFont;
	ldesc.sText = "Drag Me!";
	ret = xuiLabelCreate(ctx->pContext, &pLabel, &ldesc);
	if (ret != XUI_OK) return ret;
	xuiWidgetSetMargin(pLabel, (xui_thickness_t){10.0f, 10.0f, 0.0f, 0.0f});
	return xuiWidgetAddChild(pPanel, pLabel);
}

int main(int argc, char** argv) { return tut_xui_run(create_ui, "ch196", argc, argv); }
