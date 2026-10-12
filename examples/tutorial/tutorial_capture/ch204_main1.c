/* ch204 — 实战：代码编辑器 */
#include "tut_capture_xui.h"

static int create_ui(tut_xui_ctx_t* ctx)
{
	xui_menubar_desc_t mdesc;
	xui_code_edit_desc_t cdesc;
	xui_statusbar_desc_t sdesc;
	xui_widget pMenu, pEdit, pStatus;
	int ret;

	/* 菜单栏 */
	memset(&mdesc, 0, sizeof(mdesc));
	mdesc.iSize = sizeof(mdesc);
	mdesc.pFont = ctx->pFont;
	ret = xuiMenuBarCreate(ctx->pContext, &pMenu, &mdesc);
	if (ret != XUI_OK) return ret;
	xuiWidgetSetSizeMode(pMenu, XUI_SIZE_FILL, XUI_SIZE_CONTENT);
	xuiWidgetAddChild(ctx->pRoot, pMenu);
	xuiMenuBarAddItem(pMenu, "File", NULL, 0);
	xuiMenuBarAddItem(pMenu, "Edit", NULL, 1);
	xuiMenuBarAddItem(pMenu, "Build", NULL, 2);

	/* 代码编辑区 */
	memset(&cdesc, 0, sizeof(cdesc));
	cdesc.iSize = sizeof(cdesc);
	cdesc.pFont = ctx->pFont;
	cdesc.sText = "int main(void)\n{\n    xgeInit();\n    xgeRun(frame, NULL);\n    return 0;\n}\n";
	cdesc.sLanguage = "c";
	cdesc.bShowLineNumbers = 1;
	cdesc.bShowFoldMargin = 1;
	ret = xuiCodeEditCreate(ctx->pContext, &pEdit, &cdesc);
	if (ret != XUI_OK) return ret;
	xuiWidgetSetSizeMode(pEdit, XUI_SIZE_FILL, XUI_SIZE_FILL);
	xuiWidgetAddChild(ctx->pRoot, pEdit);

	/* 状态栏 */
	memset(&sdesc, 0, sizeof(sdesc));
	sdesc.iSize = sizeof(sdesc);
	sdesc.pFont = ctx->pFont;
	ret = xuiStatusBarCreate(ctx->pContext, &pStatus, &sdesc);
	if (ret != XUI_OK) return ret;
	xuiWidgetSetSizeMode(pStatus, XUI_SIZE_FILL, XUI_SIZE_CONTENT);
	xuiWidgetSetAlign(pStatus, XUI_ALIGN_STRETCH, XUI_ALIGN_END);
	xuiWidgetAddChild(ctx->pRoot, pStatus);
	xuiStatusBarAddText(pStatus, 0, "C", 60.0f, 0, 0);
	xuiStatusBarAddText(pStatus, 1, "Ln 3, Col 12", 120.0f, 0, 0);
	return XUI_OK;
}

int main(int argc, char** argv) { return tut_xui_run(create_ui, "ch204", argc, argv); }
