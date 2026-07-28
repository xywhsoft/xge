/* ch193 — 资源管理 */
#include "tut_capture_xui.h"

static int create_ui(tut_xui_ctx_t* ctx)
{
	xui_label_desc_t ldesc;
	xui_progress_desc_t pdesc;
	xui_widget pLabel, pProg;
	int ret;

	/* 标题 */
	memset(&ldesc, 0, sizeof(ldesc));
	ldesc.iSize = sizeof(ldesc);
	ldesc.pFont = ctx->pFont;
	ldesc.sText = "Resource Manager";
	ret = xuiLabelCreate(ctx->pContext, &pLabel, &ldesc);
	if (ret != XUI_OK) return ret;
	xuiWidgetSetMargin(pLabel, (xui_thickness_t){30.0f, 20.0f, 0.0f, 0.0f});
	xuiWidgetAddChild(ctx->pRoot, pLabel);

	/* 加载进度 */
	memset(&pdesc, 0, sizeof(pdesc));
	pdesc.iSize = sizeof(pdesc);
	pdesc.pFont = ctx->pFont;
	pdesc.fMin = 0.0f;
	pdesc.fMax = 100.0f;
	pdesc.fValue = 75.0f;
	pdesc.sText = "Loading assets... 75%";
	ret = xuiProgressCreate(ctx->pContext, &pProg, &pdesc);
	if (ret != XUI_OK) return ret;
	xuiWidgetSetMargin(pProg, (xui_thickness_t){30.0f, 16.0f, 30.0f, 0.0f});
	xuiWidgetSetPreferredSize(pProg, (xui_vec2_t){400.0f, 24.0f});
	return xuiWidgetAddChild(ctx->pRoot, pProg);
}

int main(int argc, char** argv) { return tut_xui_run(create_ui, "ch193", argc, argv); }
