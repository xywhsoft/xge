/* ch207 — 实战：文件管理器 */
#include "tut_capture_xui.h"

static int create_ui(tut_xui_ctx_t* ctx)
{
	xui_toolbar_desc_t tdesc;
	xui_tree_view_desc_t trdesc;
	xui_list_view_desc_t ldesc;
	xui_widget pToolbar, pTree, pList;
	int ret;

	/* 工具栏 */
	memset(&tdesc, 0, sizeof(tdesc));
	tdesc.iSize = sizeof(tdesc);
	tdesc.pFont = ctx->pFont;
	ret = xuiToolbarCreate(ctx->pContext, &pToolbar, &tdesc);
	if (ret != XUI_OK) return ret;
	xuiWidgetSetSizeMode(pToolbar, XUI_SIZE_FILL, XUI_SIZE_CONTENT);
	xuiWidgetAddChild(ctx->pRoot, pToolbar);
	xuiToolbarAddItem(pToolbar, "Back", 0, 0);
	xuiToolbarAddItem(pToolbar, "Up", 0, 1);
	xuiToolbarAddSeparator(pToolbar);
	xuiToolbarAddItem(pToolbar, "New Folder", 0, 2);

	/* 左侧目录树 */
	memset(&trdesc, 0, sizeof(trdesc));
	trdesc.iSize = sizeof(trdesc);
	trdesc.pFont = ctx->pFont;
	ret = xuiTreeViewCreate(ctx->pContext, &pTree, &trdesc);
	if (ret != XUI_OK) return ret;
	xuiWidgetSetPreferredSize(pTree, (xui_vec2_t){200.0f, 0.0f});
	xuiWidgetSetSizeMode(pTree, XUI_SIZE_FIXED, XUI_SIZE_FILL);
	xuiWidgetAddChild(ctx->pRoot, pTree);

	/* 右侧文件列表 */
	memset(&ldesc, 0, sizeof(ldesc));
	ldesc.iSize = sizeof(ldesc);
	ldesc.pFont = ctx->pFont;
	ret = xuiListViewCreate(ctx->pContext, &pList, &ldesc);
	if (ret != XUI_OK) return ret;
	xuiWidgetSetSizeMode(pList, XUI_SIZE_FILL, XUI_SIZE_FILL);
	return xuiWidgetAddChild(ctx->pRoot, pList);
}

int main(int argc, char** argv) { return tut_xui_run(create_ui, "ch207", argc, argv); }
