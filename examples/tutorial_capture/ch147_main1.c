/* ch147 — TreeView 树形 */
#include "tut_capture_xui.h"

static int create_ui(tut_xui_ctx_t* ctx)
{
	xui_tree_view_desc_t desc;
	xui_widget pTree;
	int ret;

	memset(&desc, 0, sizeof(desc));
	desc.iSize = sizeof(desc);
	desc.pFont = ctx->pFont;
	ret = xuiTreeViewCreate(ctx->pContext, &pTree, &desc);
	if (ret != XUI_OK) return ret;
	xuiWidgetSetMargin(pTree, (xui_thickness_t){20.0f, 20.0f, 0.0f, 0.0f});
	xuiWidgetSetPreferredSize(pTree, (xui_vec2_t){300.0f, 280.0f});
	xuiWidgetAddChild(ctx->pRoot, pTree);

	/* 添加树节点 */
	xuiTreeViewAddNode(pTree, 1, 0, "Root");
	xuiTreeViewAddNode(pTree, 2, 1, "Child 1");
	xuiTreeViewAddNode(pTree, 3, 1, "Child 2");
	xuiTreeViewAddNode(pTree, 4, 2, "Grandchild 1.1");
	xuiTreeViewAddNode(pTree, 5, 3, "Grandchild 2.1");
	return XUI_OK;
}

int main(int argc, char** argv) { return tut_xui_run(create_ui, "ch147", argc, argv); }
