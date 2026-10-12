/* ch165 — Cascader 级联选择 */
#include "tut_capture_xui.h"

static xui_cascader_item_t s_items[] = {
	{ "Asia",    1, -1, 0, NULL },
	{ "China",   2,  0, 0, NULL },
	{ "Beijing", 3,  1, 0, NULL },
	{ "Shanghai",4,  1, 0, NULL },
	{ "Japan",   5,  0, 0, NULL },
	{ "Tokyo",   6,  4, 0, NULL },
	{ "Europe",  7, -1, 0, NULL },
	{ "France",  8,  6, 0, NULL },
	{ "Paris",   9,  7, 0, NULL },
};

static int create_ui(tut_xui_ctx_t* ctx)
{
	xui_cascader_desc_t desc;
	xui_widget pCascader;
	int ret;

	memset(&desc, 0, sizeof(desc));
	desc.iSize = sizeof(desc);
	desc.arrItems = s_items;
	desc.iItemCount = 9;
	desc.pFont = ctx->pFont;
	desc.sPlaceholder = "Select region...";
	desc.sSeparator = " / ";
	ret = xuiCascaderCreate(ctx->pContext, &pCascader, &desc);
	if (ret != XUI_OK) return ret;
	xuiWidgetSetMargin(pCascader, (xui_thickness_t){40.0f, 40.0f, 0.0f, 0.0f});
	xuiWidgetSetPreferredSize(pCascader, (xui_vec2_t){260.0f, 36.0f});
	return xuiWidgetAddChild(ctx->pRoot, pCascader);
}

int main(int argc, char** argv) { return tut_xui_run(create_ui, "ch165", argc, argv); }
