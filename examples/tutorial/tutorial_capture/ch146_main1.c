/* ch146 — ListView 列表 */
#include "tut_capture_xui.h"

static int create_ui(tut_xui_ctx_t* ctx)
{
	xui_list_view_desc_t desc;
	xui_widget pList;
	int ret;

	memset(&desc, 0, sizeof(desc));
	desc.iSize = sizeof(desc);
	desc.pFont = ctx->pFont;
	ret = xuiListViewCreate(ctx->pContext, &pList, &desc);
	if (ret != XUI_OK) return ret;
	xuiWidgetSetMargin(pList, (xui_thickness_t){20.0f, 20.0f, 0.0f, 0.0f});
	xuiWidgetSetPreferredSize(pList, (xui_vec2_t){280.0f, 250.0f});
	xuiWidgetAddChild(ctx->pRoot, pList);

	/* 添加列表项 */
	{
		static const char* arrItems[] = {
			"Item 1 - Apple", "Item 2 - Banana", "Item 3 - Cherry",
			"Item 4 - Date", "Item 5 - Elderberry"
		};
		xuiListViewSetItems(pList, arrItems, 5);
	}
	return XUI_OK;
}

int main(int argc, char** argv) { return tut_xui_run(create_ui, "ch146", argc, argv); }
