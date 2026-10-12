/* ch173 — CodeEdit 架构 */
#include "tut_capture_xui.h"

static int create_ui(tut_xui_ctx_t* ctx)
{
	xui_code_edit_desc_t desc;
	xui_widget pEdit;
	int ret;

	memset(&desc, 0, sizeof(desc));
	desc.iSize = sizeof(desc);
	desc.pFont = ctx->pFont;
	desc.sText = "#include <stdio.h>\n\nint main(void)\n{\n    printf(\"Hello XGE!\\n\");\n    return 0;\n}\n";
	desc.sLanguage = "c";
	desc.bShowLineNumbers = 1;
	desc.bShowFoldMargin = 1;
	desc.iTabColumns = 4;
	ret = xuiCodeEditCreate(ctx->pContext, &pEdit, &desc);
	if (ret != XUI_OK) return ret;
	xuiWidgetSetMargin(pEdit, (xui_thickness_t){20.0f, 20.0f, 20.0f, 20.0f});
	xuiWidgetSetSizeMode(pEdit, XUI_SIZE_FILL, XUI_SIZE_FILL);
	return xuiWidgetAddChild(ctx->pRoot, pEdit);
}

int main(int argc, char** argv) { return tut_xui_run(create_ui, "ch173", argc, argv); }
