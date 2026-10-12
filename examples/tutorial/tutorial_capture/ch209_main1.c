/* ch209 — 跨平台适配要点 */
#include "tut_capture_xui.h"

static int create_ui(tut_xui_ctx_t* ctx)
{
	xui_label_desc_t ldesc;
	xui_widget pLabel;
	int ret;

	memset(&ldesc, 0, sizeof(ldesc));
	ldesc.iSize = sizeof(ldesc);
	ldesc.pFont = ctx->pFont;
	ldesc.sText = "Platform: Windows / Linux / macOS\nDPI Scale: 1.0x\nBackend: OpenGL ES 3.0\nFont: System TTF";
	ret = xuiLabelCreate(ctx->pContext, &pLabel, &ldesc);
	if (ret != XUI_OK) return ret;
	xuiWidgetSetMargin(pLabel, (xui_thickness_t){30.0f, 30.0f, 0.0f, 0.0f});
	xuiWidgetSetPreferredSize(pLabel, (xui_vec2_t){400.0f, 120.0f});
	return xuiWidgetAddChild(ctx->pRoot, pLabel);
}

int main(int argc, char** argv) { return tut_xui_run(create_ui, "ch209", argc, argv); }
