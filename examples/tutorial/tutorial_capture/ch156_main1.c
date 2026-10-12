/* ch156 — MenuBar 菜单栏 */
#include "tut_capture_xui.h"

static int create_ui(tut_xui_ctx_t* ctx)
{
	xui_menubar_desc_t desc;
	xui_widget pMenuBar;
	int ret;

	memset(&desc, 0, sizeof(desc));
	desc.iSize = sizeof(desc);
	desc.pFont = ctx->pFont;
	ret = xuiMenuBarCreate(ctx->pContext, &pMenuBar, &desc);
	if (ret != XUI_OK) return ret;
	xuiWidgetSetSizeMode(pMenuBar, XUI_SIZE_FILL, XUI_SIZE_CONTENT);
	xuiWidgetAddChild(ctx->pRoot, pMenuBar);

	/* 添加菜单 */
	xuiMenuBarAddItem(pMenuBar, "File", NULL, 0);
	xuiMenuBarAddItem(pMenuBar, "Edit", NULL, 1);
	xuiMenuBarAddItem(pMenuBar, "View", NULL, 2);
	xuiMenuBarAddItem(pMenuBar, "Help", NULL, 3);
	return XUI_OK;
}

int main(int argc, char** argv) { return tut_xui_run(create_ui, "ch156", argc, argv); }
