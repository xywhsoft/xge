/* ch200 — 性能优化 */
#include "tut_capture_xui.h"

static int create_ui(tut_xui_ctx_t* ctx)
{
	xui_label_desc_t ldesc;
	xui_progress_desc_t pdesc;
	xui_widget pLabel, pProg;
	int ret;

	/* FPS 标签 */
	memset(&ldesc, 0, sizeof(ldesc));
	ldesc.iSize = sizeof(ldesc);
	ldesc.pFont = ctx->pFont;
	ldesc.sText = "FPS: 60 | Draw Calls: 12 | Batches: 4";
	ret = xuiLabelCreate(ctx->pContext, &pLabel, &ldesc);
	if (ret != XUI_OK) return ret;
	xuiWidgetSetMargin(pLabel, (xui_thickness_t){30.0f, 20.0f, 0.0f, 0.0f});
	xuiWidgetAddChild(ctx->pRoot, pLabel);

	/* GPU 使用率 */
	memset(&pdesc, 0, sizeof(pdesc));
	pdesc.iSize = sizeof(pdesc);
	pdesc.pFont = ctx->pFont;
	pdesc.fMin = 0.0f;
	pdesc.fMax = 100.0f;
	pdesc.fValue = 35.0f;
	pdesc.sText = "GPU: 35%";
	ret = xuiProgressCreate(ctx->pContext, &pProg, &pdesc);
	if (ret != XUI_OK) return ret;
	xuiWidgetSetMargin(pProg, (xui_thickness_t){30.0f, 16.0f, 30.0f, 0.0f});
	xuiWidgetSetPreferredSize(pProg, (xui_vec2_t){350.0f, 22.0f});
	return xuiWidgetAddChild(ctx->pRoot, pProg);
}

int main(int argc, char** argv) { return tut_xui_run(create_ui, "ch200", argc, argv); }
