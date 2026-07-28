/* ch150 — Window 窗口 */
#include "tut_capture_xui.h"

static int create_ui(tut_xui_ctx_t* ctx)
{
	xui_window_desc_t desc;
	xui_label_desc_t ldesc;
	xui_widget pWindow, pLabel;
	int ret;

	memset(&desc, 0, sizeof(desc));
	desc.iSize = sizeof(desc);
	desc.sTitle = "Window Title";
	desc.pFont = ctx->pFont;
	ret = xuiWindowCreate(ctx->pContext, &pWindow, &desc);
	if (ret != XUI_OK) return ret;
	xuiWidgetSetRect(pWindow, (xui_rect_t){100.0f, 60.0f, 350.0f, 250.0f});
	xuiWidgetAddChild(ctx->pRoot, pWindow);

	memset(&ldesc, 0, sizeof(ldesc));
	ldesc.iSize = sizeof(ldesc);
	ldesc.sText = "Window content area";
	ldesc.pFont = ctx->pFont;
	ldesc.iTextColor = XUI_COLOR_RGBA(50, 60, 80, 255);
	ret = xuiLabelCreate(ctx->pContext, &pLabel, &ldesc);
	if (ret != XUI_OK) return ret;
	xuiWidgetSetMargin(pLabel, (xui_thickness_t){16.0f, 16.0f, 0.0f, 0.0f});
	return xuiWidgetAddChild(pWindow, pLabel);
}

int main(int argc, char** argv) { return tut_xui_run(create_ui, "ch150", argc, argv); }
