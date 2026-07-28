/* ch187 — InventoryGrid 手柄导航 */
#include "tut_capture_xui.h"

static int create_ui(tut_xui_ctx_t* ctx)
{
	xui_inventory_grid_desc_t desc;
	xui_widget pGrid;
	int ret;

	memset(&desc, 0, sizeof(desc));
	desc.iSize = sizeof(desc);
	desc.pFont = ctx->pFont;
	desc.iSlotCount = 16;
	ret = xuiInventoryGridCreate(ctx->pContext, &pGrid, &desc);
	if (ret != XUI_OK) return ret;
	xuiWidgetSetMargin(pGrid, (xui_thickness_t){60.0f, 60.0f, 60.0f, 60.0f});
	xuiWidgetSetPreferredSize(pGrid, (xui_vec2_t){360.0f, 360.0f});
	return xuiWidgetAddChild(ctx->pRoot, pGrid);
}

int main(int argc, char** argv) { return tut_xui_run(create_ui, "ch187", argc, argv); }
