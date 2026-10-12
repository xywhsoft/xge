/* ch177 — CodeEdit 折叠与标记 */
#include "tut_capture_xui.h"

static int create_ui(tut_xui_ctx_t* ctx)
{
	xui_code_edit_desc_t desc;
	xui_widget pEdit;
	int ret;

	memset(&desc, 0, sizeof(desc));
	desc.iSize = sizeof(desc);
	desc.pFont = ctx->pFont;
	desc.sText = "class Widget {\npublic:\n    Widget();\n    ~Widget();\n    void render();\nprivate:\n    int m_x, m_y;\n};\n";
	desc.sLanguage = "cpp";
	desc.bShowLineNumbers = 1;
	desc.bShowFoldMargin = 1;
	desc.bShowMarkerMargin = 1;
	desc.iTabColumns = 4;
	ret = xuiCodeEditCreate(ctx->pContext, &pEdit, &desc);
	if (ret != XUI_OK) return ret;
	xuiWidgetSetMargin(pEdit, (xui_thickness_t){20.0f, 20.0f, 20.0f, 20.0f});
	xuiWidgetSetSizeMode(pEdit, XUI_SIZE_FILL, XUI_SIZE_FILL);
	return xuiWidgetAddChild(ctx->pRoot, pEdit);
}

int main(int argc, char** argv) { return tut_xui_run(create_ui, "ch177", argc, argv); }
