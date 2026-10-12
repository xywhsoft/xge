/* ch145 — Scrollbar 滚动条 */
#include "tut_capture_xui.h"

static int create_ui(tut_xui_ctx_t* ctx)
{
	xui_scroll_view_desc_t desc;
	xui_label_desc_t ldesc;
	xui_widget pScroll, pContent, pLabel;
	int i, ret;

	/* ScrollView 自带滚动条 */
	memset(&desc, 0, sizeof(desc));
	desc.iSize = sizeof(desc);
	ret = xuiScrollViewCreate(ctx->pContext, &pScroll, &desc);
	if (ret != XUI_OK) return ret;
	xuiWidgetSetMargin(pScroll, (xui_thickness_t){20.0f, 20.0f, 0.0f, 0.0f});
	xuiWidgetSetPreferredSize(pScroll, (xui_vec2_t){250.0f, 250.0f});
	xuiWidgetAddChild(ctx->pRoot, pScroll);

	ret = xuiWidgetCreate(ctx->pContext, &pContent);
	if (ret != XUI_OK) return ret;
	xuiWidgetSetLayoutType(pContent, XUI_LAYOUT_COLUMN);
	xuiWidgetSetSizeMode(pContent, XUI_SIZE_FILL, XUI_SIZE_CONTENT);
	xuiWidgetAddChild(pScroll, pContent);

	memset(&ldesc, 0, sizeof(ldesc));
	ldesc.iSize = sizeof(ldesc);
	ldesc.pFont = ctx->pFont;
	ldesc.iTextColor = XUI_COLOR_RGBA(40, 50, 70, 255);

	for (i = 0; i < 20; i++) {
		char buf[64];
		snprintf(buf, sizeof(buf), "Item %d - scrollbar visible", i + 1);
		ldesc.sText = buf;
		ret = xuiLabelCreate(ctx->pContext, &pLabel, &ldesc);
		if (ret != XUI_OK) return ret;
		xuiWidgetSetMargin(pLabel, (xui_thickness_t){8.0f, 3.0f, 0.0f, 3.0f});
		xuiWidgetAddChild(pContent, pLabel);
	}
	return XUI_OK;
}

int main(int argc, char** argv) { return tut_xui_run(create_ui, "ch145", argc, argv); }
