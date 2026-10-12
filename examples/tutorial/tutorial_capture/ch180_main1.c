/* ch180 — Chart 图表 */
#include "tut_capture_xui.h"

static int create_ui(tut_xui_ctx_t* ctx)
{
	xui_chart_desc_t desc;
	xui_widget pChart;
	int ret;

	memset(&desc, 0, sizeof(desc));
	desc.iSize = sizeof(desc);
	desc.pFont = ctx->pFont;
	desc.sTitle = "Monthly Revenue";
	ret = xuiChartCreate(ctx->pContext, &pChart, &desc);
	if (ret != XUI_OK) return ret;
	xuiWidgetSetMargin(pChart, (xui_thickness_t){30.0f, 30.0f, 30.0f, 30.0f});
	xuiWidgetSetSizeMode(pChart, XUI_SIZE_FILL, XUI_SIZE_FILL);
	return xuiWidgetAddChild(ctx->pRoot, pChart);
}

int main(int argc, char** argv) { return tut_xui_run(create_ui, "ch180", argc, argv); }
