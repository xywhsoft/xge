/* ch195 — 层系统 */
#include "tut_capture_xui.h"

static int create_ui(tut_xui_ctx_t* ctx)
{
	xui_panel_desc_t pdesc;
	xui_label_desc_t ldesc;
	xui_widget pBack, pFront, pLabel;
	int ret;

	/* 背景层 */
	memset(&pdesc, 0, sizeof(pdesc));
	pdesc.iSize = sizeof(pdesc);
	ret = xuiPanelCreate(ctx->pContext, &pBack, &pdesc);
	if (ret != XUI_OK) return ret;
	xuiWidgetSetRect(pBack, (xui_rect_t){50.0f, 50.0f, 300.0f, 200.0f});
	xuiWidgetAddChild(ctx->pRoot, pBack);

	/* 前景层 — 覆盖 */
	ret = xuiPanelCreate(ctx->pContext, &pFront, &pdesc);
	if (ret != XUI_OK) return ret;
	xuiWidgetSetRect(pFront, (xui_rect_t){150.0f, 120.0f, 300.0f, 200.0f});
	xuiWidgetAddChild(ctx->pRoot, pFront);

	/* 标签 */
	memset(&ldesc, 0, sizeof(ldesc));
	ldesc.iSize = sizeof(ldesc);
	ldesc.pFont = ctx->pFont;
	ldesc.sText = "Layer 2 (top)";
	ret = xuiLabelCreate(ctx->pContext, &pLabel, &ldesc);
	if (ret != XUI_OK) return ret;
	xuiWidgetSetMargin(pLabel, (xui_thickness_t){10.0f, 10.0f, 0.0f, 0.0f});
	return xuiWidgetAddChild(pFront, pLabel);
}

int main(int argc, char** argv) { return tut_xui_run(create_ui, "ch195", argc, argv); }
