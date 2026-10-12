/* ch132 — Hyperlink 超链接 */
#include "tut_capture_xui.h"

static int create_ui(tut_xui_ctx_t* ctx)
{
	xui_hyperlink_desc_t desc;
	xui_widget pLink;
	int ret;

	memset(&desc, 0, sizeof(desc));
	desc.iSize = sizeof(desc);
	desc.pFont = ctx->pFont;

	desc.sText = "https://xge.dev";
	desc.iTextColor = XUI_COLOR_RGBA(30, 100, 200, 255);
	desc.iHoverTextColor = XUI_COLOR_RGBA(60, 130, 230, 255);
	desc.bUnderline = 0;
	desc.bHoverUnderline = 1;
	ret = xuiHyperlinkCreate(ctx->pContext, &pLink, &desc);
	if (ret != XUI_OK) return ret;
	xuiWidgetSetMargin(pLink, (xui_thickness_t){20.0f, 20.0f, 0.0f, 0.0f});
	xuiWidgetAddChild(ctx->pRoot, pLink);

	desc.sText = "Documentation Link";
	desc.iTextColor = XUI_COLOR_RGBA(150, 50, 200, 255);
	desc.bUnderline = 1;
	ret = xuiHyperlinkCreate(ctx->pContext, &pLink, &desc);
	if (ret != XUI_OK) return ret;
	xuiWidgetSetMargin(pLink, (xui_thickness_t){20.0f, 12.0f, 0.0f, 0.0f});
	return xuiWidgetAddChild(ctx->pRoot, pLink);
}

int main(int argc, char** argv) { return tut_xui_run(create_ui, "ch132", argc, argv); }
