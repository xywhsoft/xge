/* ch175 — CodeEdit 编辑操作 */
#include "tut_capture_xui.h"

static int create_ui(tut_xui_ctx_t* ctx)
{
	xui_code_edit_desc_t desc;
	xui_widget pEdit;
	int ret;

	memset(&desc, 0, sizeof(desc));
	desc.iSize = sizeof(desc);
	desc.pFont = ctx->pFont;
	desc.sText = "import os\nimport sys\n\ndef process(data):\n    result = [x * 2 for x in data]\n    return sorted(result)\n\nif __name__ == '__main__':\n    print(process([3, 1, 4, 1, 5]))\n";
	desc.sLanguage = "python";
	desc.bShowLineNumbers = 1;
	desc.bWordWrap = 1;
	desc.iTabColumns = 4;
	ret = xuiCodeEditCreate(ctx->pContext, &pEdit, &desc);
	if (ret != XUI_OK) return ret;
	xuiWidgetSetMargin(pEdit, (xui_thickness_t){20.0f, 20.0f, 20.0f, 20.0f});
	xuiWidgetSetSizeMode(pEdit, XUI_SIZE_FILL, XUI_SIZE_FILL);
	return xuiWidgetAddChild(ctx->pRoot, pEdit);
}

int main(int argc, char** argv) { return tut_xui_run(create_ui, "ch175", argc, argv); }
