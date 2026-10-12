/* ch122 — 上下文创建 */
#include "tut_capture_xui.h"

static int create_ui(tut_xui_ctx_t* ctx)
{
	xui_label_desc_t desc;
	xui_widget pLabel1, pLabel2;
	int ret;

	memset(&desc, 0, sizeof(desc));
	desc.iSize = sizeof(desc);
	desc.pFont = ctx->pFont;

	desc.sText = "XUI Context Created";
	desc.iTextColor = XUI_COLOR_RGBA(30, 40, 60, 255);
	ret = xuiLabelCreate(ctx->pContext, &pLabel1, &desc);
	if (ret != XUI_OK) return ret;
	xuiWidgetSetMargin(pLabel1, (xui_thickness_t){20.0f, 20.0f, 0.0f, 0.0f});
	xuiWidgetAddChild(ctx->pRoot, pLabel1);

	desc.sText = "Proxy + Surface + Font initialized";
	desc.iTextColor = XUI_COLOR_RGBA(80, 100, 140, 255);
	ret = xuiLabelCreate(ctx->pContext, &pLabel2, &desc);
	if (ret != XUI_OK) return ret;
	xuiWidgetSetMargin(pLabel2, (xui_thickness_t){20.0f, 8.0f, 0.0f, 0.0f});
	return xuiWidgetAddChild(ctx->pRoot, pLabel2);
}

int main(int argc, char** argv) { return tut_xui_run(create_ui, "ch122", argc, argv); }
