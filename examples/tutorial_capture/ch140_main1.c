/* ch140 — TextEdit 多行编辑 */
#include "tut_capture_xui.h"

static int create_ui(tut_xui_ctx_t* ctx)
{
	xui_text_edit_desc_t desc;
	xui_widget pEdit;
	int ret;

	memset(&desc, 0, sizeof(desc));
	desc.iSize = sizeof(desc);
	desc.pFont = ctx->pFont;
	desc.sText = "Line 1\nLine 2\nLine 3 - Multi-line text editing";
	desc.bWordWrap = 1;
	desc.bLineNumbers = 1;
	ret = xuiTextEditCreate(ctx->pContext, &pEdit, &desc);
	if (ret != XUI_OK) return ret;
	xuiWidgetSetMargin(pEdit, (xui_thickness_t){20.0f, 20.0f, 0.0f, 0.0f});
	xuiWidgetSetPreferredSize(pEdit, (xui_vec2_t){400.0f, 160.0f});
	return xuiWidgetAddChild(ctx->pRoot, pEdit);
}

int main(int argc, char** argv) { return tut_xui_run(create_ui, "ch140", argc, argv); }
