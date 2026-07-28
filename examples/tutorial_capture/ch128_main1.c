/* ch128 — 布局系统：Table / Dock / Grid */
#include "tut_capture_xui.h"

static int create_ui(tut_xui_ctx_t* ctx)
{
	xui_button_desc_t desc;
	xui_widget pTable, pBtn;
	int i, ret;

	memset(&desc, 0, sizeof(desc));
	desc.iSize = sizeof(desc);
	desc.pFont = ctx->pFont;

	/* Table 布局：2行3列 */
	ret = xuiWidgetCreate(ctx->pContext, &pTable);
	if (ret != XUI_OK) return ret;
	xuiWidgetSetLayoutType(pTable, XUI_LAYOUT_TABLE);
	xuiWidgetSetTableSize(pTable, 2, 3);
	xuiWidgetSetSizeMode(pTable, XUI_SIZE_CONTENT, XUI_SIZE_CONTENT);
	xuiWidgetSetPadding(pTable, (xui_thickness_t){10.0f, 10.0f, 10.0f, 10.0f});
	xuiWidgetSetMargin(pTable, (xui_thickness_t){20.0f, 20.0f, 0.0f, 0.0f});
	xuiWidgetAddChild(ctx->pRoot, pTable);

	for (i = 0; i < 6; i++) {
		char buf[32];
		snprintf(buf, sizeof(buf), "Cell %d", i + 1);
		desc.sText = buf;
		ret = xuiButtonCreate(ctx->pContext, &pBtn, &desc);
		if (ret != XUI_OK) return ret;
		xuiWidgetSetPreferredSize(pBtn, (xui_vec2_t){100.0f, 32.0f});
		xuiWidgetSetMargin(pBtn, (xui_thickness_t){4.0f, 4.0f, 4.0f, 4.0f});
		xuiWidgetAddChild(pTable, pBtn);
	}
	return XUI_OK;
}

int main(int argc, char** argv) { return tut_xui_run(create_ui, "ch128", argc, argv); }
