/* ch171 — Progress 进度条 */
#include "tut_capture_xui.h"

static int create_ui(tut_xui_ctx_t* ctx)
{
	xui_progress_desc_t desc;
	xui_widget pProg;
	int ret;

	memset(&desc, 0, sizeof(desc));
	desc.iSize = sizeof(desc);
	desc.pFont = ctx->pFont;
	desc.fMin = 0.0f;
	desc.fMax = 100.0f;
	desc.sText = "Downloading... 65%";

	/* 进度条 1: 65% */
	desc.fValue = 65.0f;
	ret = xuiProgressCreate(ctx->pContext, &pProg, &desc);
	if (ret != XUI_OK) return ret;
	xuiWidgetSetMargin(pProg, (xui_thickness_t){40.0f, 40.0f, 40.0f, 0.0f});
	xuiWidgetSetPreferredSize(pProg, (xui_vec2_t){400.0f, 24.0f});
	xuiWidgetAddChild(ctx->pRoot, pProg);

	/* 进度条 2: 30% */
	desc.fValue = 30.0f;
	desc.sText = "Uploading... 30%";
	ret = xuiProgressCreate(ctx->pContext, &pProg, &desc);
	if (ret != XUI_OK) return ret;
	xuiWidgetSetMargin(pProg, (xui_thickness_t){40.0f, 16.0f, 40.0f, 0.0f});
	xuiWidgetSetPreferredSize(pProg, (xui_vec2_t){400.0f, 24.0f});
	return xuiWidgetAddChild(ctx->pRoot, pProg);
}

int main(int argc, char** argv) { return tut_xui_run(create_ui, "ch171", argc, argv); }
