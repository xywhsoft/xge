/* ch174 — CodeEdit 词法高亮 */
#include "tut_capture_xui.h"

static int create_ui(tut_xui_ctx_t* ctx)
{
	xui_code_edit_desc_t desc;
	xui_widget pEdit;
	int ret;

	memset(&desc, 0, sizeof(desc));
	desc.iSize = sizeof(desc);
	desc.pFont = ctx->pFont;
	desc.sText = "function greet(name) {\n    const msg = `Hello, ${name}!`;\n    console.log(msg);\n    return msg.length > 0;\n}\n";
	desc.sLanguage = "javascript";
	desc.bShowLineNumbers = 1;
	desc.iTabColumns = 4;
	ret = xuiCodeEditCreate(ctx->pContext, &pEdit, &desc);
	if (ret != XUI_OK) return ret;
	xuiWidgetSetMargin(pEdit, (xui_thickness_t){20.0f, 20.0f, 20.0f, 20.0f});
	xuiWidgetSetSizeMode(pEdit, XUI_SIZE_FILL, XUI_SIZE_FILL);
	return xuiWidgetAddChild(ctx->pRoot, pEdit);
}

int main(int argc, char** argv) { return tut_xui_run(create_ui, "ch174", argc, argv); }
