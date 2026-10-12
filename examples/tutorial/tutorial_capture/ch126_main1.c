/* ch126 — 布局系统：Manual / Overlay */
#include "tut_capture_xui.h"

static int create_ui(tut_xui_ctx_t* ctx)
{
	xui_label_desc_t desc;
	xui_widget pLabel1, pLabel2, pLabel3;
	int ret;

	/* Manual 布局：使用绝对定位 */
	xuiWidgetSetLayoutType(ctx->pRoot, XUI_LAYOUT_MANUAL);

	memset(&desc, 0, sizeof(desc));
	desc.iSize = sizeof(desc);
	desc.pFont = ctx->pFont;
	desc.iTextColor = XUI_COLOR_RGBA(30, 40, 60, 255);

	desc.sText = "Manual: (30, 30)";
	ret = xuiLabelCreate(ctx->pContext, &pLabel1, &desc);
	if (ret != XUI_OK) return ret;
	xuiWidgetSetRect(pLabel1, (xui_rect_t){30.0f, 30.0f, 150.0f, 24.0f});
	xuiWidgetAddChild(ctx->pRoot, pLabel1);

	desc.sText = "Manual: (200, 100)";
	ret = xuiLabelCreate(ctx->pContext, &pLabel2, &desc);
	if (ret != XUI_OK) return ret;
	xuiWidgetSetRect(pLabel2, (xui_rect_t){200.0f, 100.0f, 160.0f, 24.0f});
	xuiWidgetAddChild(ctx->pRoot, pLabel2);

	/* Overlay 布局：重叠 */
	desc.sText = "Overlay: stacked";
	desc.iTextColor = XUI_COLOR_RGBA(200, 80, 80, 255);
	ret = xuiLabelCreate(ctx->pContext, &pLabel3, &desc);
	if (ret != XUI_OK) return ret;
	xuiWidgetSetRect(pLabel3, (xui_rect_t){210.0f, 110.0f, 160.0f, 24.0f});
	return xuiWidgetAddChild(ctx->pRoot, pLabel3);
}

int main(int argc, char** argv) { return tut_xui_run(create_ui, "ch126", argc, argv); }
