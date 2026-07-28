/* ch184 — PropertyGrid 属性网格 */
#include "tut_capture_xui.h"

static int create_ui(tut_xui_ctx_t* ctx)
{
	xui_property_grid_desc_t desc;
	xui_widget pGrid;
	int ret;

	memset(&desc, 0, sizeof(desc));
	desc.iSize = sizeof(desc);
	desc.pFont = ctx->pFont;
	desc.fNameWidth = 160.0f;
	desc.fRowHeight = 28.0f;
	desc.fCategoryHeight = 24.0f;
	ret = xuiPropertyGridCreate(ctx->pContext, &pGrid, &desc);
	if (ret != XUI_OK) return ret;
	xuiWidgetSetMargin(pGrid, (xui_thickness_t){20.0f, 20.0f, 20.0f, 20.0f});
	xuiWidgetSetSizeMode(pGrid, XUI_SIZE_FILL, XUI_SIZE_FILL);
	return xuiWidgetAddChild(ctx->pRoot, pGrid);
}

int main(int argc, char** argv) { return tut_xui_run(create_ui, "ch184", argc, argv); }
