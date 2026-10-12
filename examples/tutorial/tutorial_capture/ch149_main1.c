/* ch149 — TableGrid 数据网格 */
#include "tut_capture_xui.h"

static int create_ui(tut_xui_ctx_t* ctx)
{
	xui_table_grid_desc_t desc;
	xui_widget pGrid;
	int ret;

	memset(&desc, 0, sizeof(desc));
	desc.iSize = sizeof(desc);
	desc.pFont = ctx->pFont;
	ret = xuiTableGridCreate(ctx->pContext, &pGrid, &desc);
	if (ret != XUI_OK) return ret;
	xuiWidgetSetMargin(pGrid, (xui_thickness_t){20.0f, 20.0f, 0.0f, 0.0f});
	xuiWidgetSetPreferredSize(pGrid, (xui_vec2_t){450.0f, 280.0f});
	return xuiWidgetAddChild(ctx->pRoot, pGrid);
}

int main(int argc, char** argv) { return tut_xui_run(create_ui, "ch149", argc, argv); }
