/* ch172 — StepBar 步骤条 */
#include "tut_capture_xui.h"

static const char* s_titles[] = { "Cart", "Shipping", "Payment", "Done" };

static int create_ui(tut_xui_ctx_t* ctx)
{
	xui_step_bar_desc_t desc;
	xui_widget pStepBar;
	int ret;

	memset(&desc, 0, sizeof(desc));
	desc.iSize = sizeof(desc);
	desc.ppTitles = s_titles;
	desc.iStepCount = 4;
	desc.iCurrent = 1; /* 当前在第2步 */
	desc.iStyle = 0;
	desc.pFont = ctx->pFont;
	ret = xuiStepBarCreate(ctx->pContext, &pStepBar, &desc);
	if (ret != XUI_OK) return ret;
	xuiWidgetSetMargin(pStepBar, (xui_thickness_t){40.0f, 60.0f, 40.0f, 0.0f});
	xuiWidgetSetPreferredSize(pStepBar, (xui_vec2_t){600.0f, 60.0f});
	return xuiWidgetAddChild(ctx->pRoot, pStepBar);
}

int main(int argc, char** argv) { return tut_xui_run(create_ui, "ch172", argc, argv); }
