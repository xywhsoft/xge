/* ch186 — InventoryGrid 物品栏 */
#include "tut_capture_xui.h"

static int create_ui(tut_xui_ctx_t* ctx)
{
	xui_inventory_grid_desc_t desc;
	xui_widget pGrid;
	int ret;

	memset(&desc, 0, sizeof(desc));
	desc.iSize = sizeof(desc);
	desc.pFont = ctx->pFont;
	desc.iSlotCount = 20;
	ret = xuiInventoryGridCreate(ctx->pContext, &pGrid, &desc);
	if (ret != XUI_OK) return ret;
	xuiWidgetSetMargin(pGrid, (xui_thickness_t){40.0f, 40.0f, 40.0f, 40.0f});
	xuiWidgetSetPreferredSize(pGrid, (xui_vec2_t){400.0f, 300.0f});
	return xuiWidgetAddChild(ctx->pRoot, pGrid);
}

int main(int argc, char** argv) { return tut_xui_run(create_ui, "ch186", argc, argv); }
