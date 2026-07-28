/* ch157 — Toolbar 工具栏 */
#include "tut_capture_xui.h"

static int create_ui(tut_xui_ctx_t* ctx)
{
	xui_toolbar_desc_t desc;
	xui_widget pToolbar;
	int ret;

	memset(&desc, 0, sizeof(desc));
	desc.iSize = sizeof(desc);
	desc.pFont = ctx->pFont;
	ret = xuiToolbarCreate(ctx->pContext, &pToolbar, &desc);
	if (ret != XUI_OK) return ret;
	xuiWidgetSetSizeMode(pToolbar, XUI_SIZE_FILL, XUI_SIZE_CONTENT);
	xuiWidgetAddChild(ctx->pRoot, pToolbar);

	/* 添加工具栏按钮 */
	xuiToolbarAddItem(pToolbar, "New", 0, 0);
	xuiToolbarAddItem(pToolbar, "Open", 0, 1);
	xuiToolbarAddItem(pToolbar, "Save", 0, 2);
	xuiToolbarAddSeparator(pToolbar);
	xuiToolbarAddItem(pToolbar, "Cut", 0, 3);
	xuiToolbarAddItem(pToolbar, "Copy", 0, 4);
	xuiToolbarAddItem(pToolbar, "Paste", 0, 5);
	return XUI_OK;
}

int main(int argc, char** argv) { return tut_xui_run(create_ui, "ch157", argc, argv); }
