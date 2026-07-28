/* ch148 — TableView 表格 */
#include "tut_capture_xui.h"

static int create_ui(tut_xui_ctx_t* ctx)
{
	xui_table_view_desc_t desc;
	xui_widget pTable;
	int ret;

	memset(&desc, 0, sizeof(desc));
	desc.iSize = sizeof(desc);
	desc.pFont = ctx->pFont;
	ret = xuiTableViewCreate(ctx->pContext, &pTable, &desc);
	if (ret != XUI_OK) return ret;
	xuiWidgetSetMargin(pTable, (xui_thickness_t){20.0f, 20.0f, 0.0f, 0.0f});
	xuiWidgetSetPreferredSize(pTable, (xui_vec2_t){400.0f, 250.0f});
	xuiWidgetAddChild(ctx->pRoot, pTable);

	/* 添加列 */
	{
		xui_table_view_column_t arrCols[3];
		memset(arrCols, 0, sizeof(arrCols));
		arrCols[0].sTitle = "Name";  arrCols[0].fWidth = 120.0f; arrCols[0].bVisible = 1;
		arrCols[1].sTitle = "Value"; arrCols[1].fWidth = 100.0f; arrCols[1].bVisible = 1;
		arrCols[2].sTitle = "Status"; arrCols[2].fWidth = 80.0f;  arrCols[2].bVisible = 1;
		xuiTableViewSetColumns(pTable, arrCols, 3);
	}
	return XUI_OK;
}

int main(int argc, char** argv) { return tut_xui_run(create_ui, "ch148", argc, argv); }
