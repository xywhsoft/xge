/* ch127 — 布局系统：Row / Column / Flow */
#include "tut_capture_xui.h"

static int create_ui(tut_xui_ctx_t* ctx)
{
	xui_button_desc_t desc;
	xui_widget pRow, pCol, pBtn;
	int i, ret;

	memset(&desc, 0, sizeof(desc));
	desc.iSize = sizeof(desc);
	desc.pFont = ctx->pFont;

	/* Row 布局 */
	ret = xuiWidgetCreate(ctx->pContext, &pRow);
	if (ret != XUI_OK) return ret;
	xuiWidgetSetLayoutType(pRow, XUI_LAYOUT_ROW);
	xuiWidgetSetSizeMode(pRow, XUI_SIZE_FILL, XUI_SIZE_CONTENT);
	xuiWidgetSetPadding(pRow, (xui_thickness_t){10.0f, 10.0f, 10.0f, 10.0f});
	xuiWidgetAddChild(ctx->pRoot, pRow);

	for (i = 0; i < 3; i++) {
		char buf[32];
		snprintf(buf, sizeof(buf), "Row %d", i + 1);
		desc.sText = buf;
		ret = xuiButtonCreate(ctx->pContext, &pBtn, &desc);
		if (ret != XUI_OK) return ret;
		xuiWidgetSetPreferredSize(pBtn, (xui_vec2_t){100.0f, 32.0f});
		xuiWidgetSetMargin(pBtn, (xui_thickness_t){0.0f, 0.0f, 8.0f, 0.0f});
		xuiWidgetAddChild(pRow, pBtn);
	}

	/* Column 布局 */
	ret = xuiWidgetCreate(ctx->pContext, &pCol);
	if (ret != XUI_OK) return ret;
	xuiWidgetSetLayoutType(pCol, XUI_LAYOUT_COLUMN);
	xuiWidgetSetSizeMode(pCol, XUI_SIZE_CONTENT, XUI_SIZE_CONTENT);
	xuiWidgetSetPadding(pCol, (xui_thickness_t){10.0f, 10.0f, 10.0f, 10.0f});
	xuiWidgetSetMargin(pCol, (xui_thickness_t){0.0f, 16.0f, 0.0f, 0.0f});
	xuiWidgetAddChild(ctx->pRoot, pCol);

	for (i = 0; i < 3; i++) {
		char buf[32];
		snprintf(buf, sizeof(buf), "Col %d", i + 1);
		desc.sText = buf;
		ret = xuiButtonCreate(ctx->pContext, &pBtn, &desc);
		if (ret != XUI_OK) return ret;
		xuiWidgetSetPreferredSize(pBtn, (xui_vec2_t){100.0f, 32.0f});
		xuiWidgetSetMargin(pBtn, (xui_thickness_t){0.0f, 0.0f, 0.0f, 6.0f});
		xuiWidgetAddChild(pCol, pBtn);
	}
	return XUI_OK;
}

int main(int argc, char** argv) { return tut_xui_run(create_ui, "ch127", argc, argv); }
