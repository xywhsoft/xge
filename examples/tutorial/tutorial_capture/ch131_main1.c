/* ch131 — Label 标签 */
#include "tut_capture_xui.h"

static int create_ui(tut_xui_ctx_t* ctx)
{
	xui_label_desc_t desc;
	xui_widget pLabel;
	int ret;

	memset(&desc, 0, sizeof(desc));
	desc.iSize = sizeof(desc);
	desc.pFont = ctx->pFont;

	/* 普通标签 */
	desc.sText = "Normal Label";
	desc.iTextColor = XUI_COLOR_RGBA(30, 40, 60, 255);
	ret = xuiLabelCreate(ctx->pContext, &pLabel, &desc);
	if (ret != XUI_OK) return ret;
	xuiWidgetSetMargin(pLabel, (xui_thickness_t){20.0f, 20.0f, 0.0f, 0.0f});
	xuiWidgetAddChild(ctx->pRoot, pLabel);

	/* 彩色标签 */
	desc.sText = "Colored Label";
	desc.iTextColor = XUI_COLOR_RGBA(200, 60, 60, 255);
	ret = xuiLabelCreate(ctx->pContext, &pLabel, &desc);
	if (ret != XUI_OK) return ret;
	xuiWidgetSetMargin(pLabel, (xui_thickness_t){20.0f, 12.0f, 0.0f, 0.0f});
	xuiWidgetAddChild(ctx->pRoot, pLabel);

	/* 下划线标签 */
	desc.sText = "Underlined Label";
	desc.iTextColor = XUI_COLOR_RGBA(30, 40, 60, 255);
	desc.bUnderline = 1;
	ret = xuiLabelCreate(ctx->pContext, &pLabel, &desc);
	if (ret != XUI_OK) return ret;
	xuiWidgetSetMargin(pLabel, (xui_thickness_t){20.0f, 12.0f, 0.0f, 0.0f});
	return xuiWidgetAddChild(ctx->pRoot, pLabel);
}

int main(int argc, char** argv) { return tut_xui_run(create_ui, "ch131", argc, argv); }
