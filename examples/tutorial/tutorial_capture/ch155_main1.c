/* ch155 — Menu 菜单 */
#include "tut_capture_xui.h"

static int create_ui(tut_xui_ctx_t* ctx)
{
	xui_menu_desc_t desc;
	xui_widget pMenu;
	int ret;

	memset(&desc, 0, sizeof(desc));
	desc.iSize = sizeof(desc);
	desc.pFont = ctx->pFont;
	ret = xuiMenuCreate(ctx->pContext, &pMenu, &desc);
	if (ret != XUI_OK) return ret;
	xuiWidgetSetPreferredSize(pMenu, (xui_vec2_t){180.0f, 200.0f});

	/* 添加菜单项 */
	{
		xui_menu_item_t tItem;
		memset(&tItem, 0, sizeof(tItem));
		tItem.sText = "New"; xuiMenuAddItem(pMenu, &tItem);
		tItem.sText = "Open"; xuiMenuAddItem(pMenu, &tItem);
		tItem.sText = "Save"; xuiMenuAddItem(pMenu, &tItem);
		xuiMenuAddSeparator(pMenu);
		tItem.sText = "Exit"; xuiMenuAddItem(pMenu, &tItem);
	}

	/* 显示菜单 */
	xuiMenuOpenAt(pMenu, NULL, 50.0f, 50.0f);
	return XUI_OK;
}

int main(int argc, char** argv) { return tut_xui_run(create_ui, "ch155", argc, argv); }
