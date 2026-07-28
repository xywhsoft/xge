/* ch205 — 实战：数据仪表盘 */
#include "tut_capture_xui.h"

static int create_ui(tut_xui_ctx_t* ctx)
{
	xui_chart_desc_t cdesc;
	xui_label_desc_t ldesc;
	xui_progress_desc_t pdesc;
	xui_widget pChart, pLabel, pProg;
	int ret;

	/* 标题 */
	memset(&ldesc, 0, sizeof(ldesc));
	ldesc.iSize = sizeof(ldesc);
	ldesc.pFont = ctx->pFont;
	ldesc.sText = "System Dashboard";
	ret = xuiLabelCreate(ctx->pContext, &pLabel, &ldesc);
	if (ret != XUI_OK) return ret;
	xuiWidgetSetMargin(pLabel, (xui_thickness_t){20.0f, 16.0f, 0.0f, 0.0f});
	xuiWidgetAddChild(ctx->pRoot, pLabel);

	/* 图表 */
	memset(&cdesc, 0, sizeof(cdesc));
	cdesc.iSize = sizeof(cdesc);
	cdesc.pFont = ctx->pFont;
	cdesc.sTitle = "CPU Usage";
	ret = xuiChartCreate(ctx->pContext, &pChart, &cdesc);
	if (ret != XUI_OK) return ret;
	xuiWidgetSetMargin(pChart, (xui_thickness_t){20.0f, 10.0f, 20.0f, 0.0f});
	xuiWidgetSetPreferredSize(pChart, (xui_vec2_t){400.0f, 200.0f});
	xuiWidgetAddChild(ctx->pRoot, pChart);

	/* 内存进度 */
	memset(&pdesc, 0, sizeof(pdesc));
	pdesc.iSize = sizeof(pdesc);
	pdesc.pFont = ctx->pFont;
	pdesc.fMin = 0.0f;
	pdesc.fMax = 100.0f;
	pdesc.fValue = 62.0f;
	pdesc.sText = "Memory: 62%";
	ret = xuiProgressCreate(ctx->pContext, &pProg, &pdesc);
	if (ret != XUI_OK) return ret;
	xuiWidgetSetMargin(pProg, (xui_thickness_t){20.0f, 10.0f, 20.0f, 0.0f});
	xuiWidgetSetPreferredSize(pProg, (xui_vec2_t){400.0f, 22.0f});
	return xuiWidgetAddChild(ctx->pRoot, pProg);
}

int main(int argc, char** argv) { return tut_xui_run(create_ui, "ch205", argc, argv); }
