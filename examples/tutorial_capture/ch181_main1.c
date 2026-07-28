/* ch181 — DockPanel 停靠面板 */
#include "tut_capture_xui.h"

static int create_ui(tut_xui_ctx_t* ctx)
{
	xui_dock_panel_desc_t desc;
	xui_widget pDock;
	int ret;

	memset(&desc, 0, sizeof(desc));
	desc.iSize = sizeof(desc);
	desc.pFont = ctx->pFont;
	ret = xuiDockPanelCreate(ctx->pContext, &pDock, &desc);
	if (ret != XUI_OK) return ret;
	xuiWidgetSetSizeMode(pDock, XUI_SIZE_FILL, XUI_SIZE_FILL);
	return xuiWidgetAddChild(ctx->pRoot, pDock);
}

int main(int argc, char** argv) { return tut_xui_run(create_ui, "ch181", argc, argv); }
