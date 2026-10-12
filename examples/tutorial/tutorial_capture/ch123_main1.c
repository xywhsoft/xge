/* ch123 — 控件树与生命周期 */
#include "tut_capture_xui.h"

static int create_ui(tut_xui_ctx_t* ctx)
{
	xui_widget pParent, pChild1, pChild2;
	xui_label_desc_t desc;
	int ret;

	/* 创建父容器 */
	ret = xuiWidgetCreate(ctx->pContext, &pParent);
	if (ret != XUI_OK) return ret;
	xuiWidgetSetLayoutType(pParent, XUI_LAYOUT_COLUMN);
	xuiWidgetSetSizeMode(pParent, XUI_SIZE_FILL, XUI_SIZE_CONTENT);
	xuiWidgetSetPadding(pParent, (xui_thickness_t){16.0f, 16.0f, 16.0f, 16.0f});
	xuiWidgetAddChild(ctx->pRoot, pParent);

	/* 子控件 1 */
	memset(&desc, 0, sizeof(desc));
	desc.iSize = sizeof(desc);
	desc.sText = "Parent Widget";
	desc.pFont = ctx->pFont;
	desc.iTextColor = XUI_COLOR_RGBA(30, 40, 60, 255);
	ret = xuiLabelCreate(ctx->pContext, &pChild1, &desc);
	if (ret != XUI_OK) return ret;
	xuiWidgetAddChild(pParent, pChild1);

	/* 子控件 2 */
	desc.sText = "  -> Child Widget";
	desc.iTextColor = XUI_COLOR_RGBA(80, 120, 180, 255);
	ret = xuiLabelCreate(ctx->pContext, &pChild2, &desc);
	if (ret != XUI_OK) return ret;
	xuiWidgetSetMargin(pChild2, (xui_thickness_t){0.0f, 8.0f, 0.0f, 0.0f});
	return xuiWidgetAddChild(pParent, pChild2);
}

int main(int argc, char** argv) { return tut_xui_run(create_ui, "ch123", argc, argv); }
