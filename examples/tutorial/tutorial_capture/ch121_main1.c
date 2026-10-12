/* ch121 — 渲染代理 xui_proxy_t */
#include "tut_capture_xui.h"

static int create_ui(tut_xui_ctx_t* ctx)
{
	xui_label_desc_t desc;
	xui_widget pLabel;
	int ret;

	/* 通过 proxy 创建 surface 并绘制 */
	memset(&desc, 0, sizeof(desc));
	desc.iSize = sizeof(desc);
	desc.sText = "XUI Proxy: XGE Backend";
	desc.pFont = ctx->pFont;
	desc.iTextColor = XUI_COLOR_RGBA(30, 40, 60, 255);
	ret = xuiLabelCreate(ctx->pContext, &pLabel, &desc);
	if (ret != XUI_OK) return ret;
	xuiWidgetSetMargin(pLabel, (xui_thickness_t){20.0f, 20.0f, 0.0f, 0.0f});
	return xuiWidgetAddChild(ctx->pRoot, pLabel);
}

int main(int argc, char** argv) { return tut_xui_run(create_ui, "ch121", argc, argv); }
