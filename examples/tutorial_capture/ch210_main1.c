/* ch210 — 发布与打包 */
#include "tut_capture_xui.h"

static int create_ui(tut_xui_ctx_t* ctx)
{
	xui_label_desc_t ldesc;
	xui_progress_desc_t pdesc;
	xui_widget pLabel, pProg;
	int ret;

	/* 打包信息 */
	memset(&ldesc, 0, sizeof(ldesc));
	ldesc.iSize = sizeof(ldesc);
	ldesc.pFont = ctx->pFont;
	ldesc.sText = "Build: Release x64\nOutput: app.exe (2.4 MB)\nAssets: packed (1.1 MB)";
	ret = xuiLabelCreate(ctx->pContext, &pLabel, &ldesc);
	if (ret != XUI_OK) return ret;
	xuiWidgetSetMargin(pLabel, (xui_thickness_t){30.0f, 20.0f, 0.0f, 0.0f});
	xuiWidgetSetPreferredSize(pLabel, (xui_vec2_t){350.0f, 90.0f});
	xuiWidgetAddChild(ctx->pRoot, pLabel);

	/* 打包进度 */
	memset(&pdesc, 0, sizeof(pdesc));
	pdesc.iSize = sizeof(pdesc);
	pdesc.pFont = ctx->pFont;
	pdesc.fMin = 0.0f;
	pdesc.fMax = 100.0f;
	pdesc.fValue = 100.0f;
	pdesc.sText = "Package complete!";
	ret = xuiProgressCreate(ctx->pContext, &pProg, &pdesc);
	if (ret != XUI_OK) return ret;
	xuiWidgetSetMargin(pProg, (xui_thickness_t){30.0f, 16.0f, 30.0f, 0.0f});
	xuiWidgetSetPreferredSize(pProg, (xui_vec2_t){350.0f, 24.0f});
	return xuiWidgetAddChild(ctx->pRoot, pProg);
}

int main(int argc, char** argv) { return tut_xui_run(create_ui, "ch210", argc, argv); }
