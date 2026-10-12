/* ch143 — Panel 面板 */
#include "tut_capture_xui.h"

static int create_ui(tut_xui_ctx_t* ctx)
{
	xui_panel_desc_t desc;
	xui_label_desc_t ldesc;
	xui_widget pPanel, pLabel;
	int ret;

	memset(&desc, 0, sizeof(desc));
	desc.iSize = sizeof(desc);
	desc.sTitle = "Panel Title";
	desc.pFont = ctx->pFont;
	ret = xuiPanelCreate(ctx->pContext, &pPanel, &desc);
	if (ret != XUI_OK) return ret;
	xuiWidgetSetMargin(pPanel, (xui_thickness_t){20.0f, 20.0f, 0.0f, 0.0f});
	xuiWidgetSetPreferredSize(pPanel, (xui_vec2_t){300.0f, 180.0f});
	xuiWidgetSetLayoutType(pPanel, XUI_LAYOUT_COLUMN);
	xuiWidgetAddChild(ctx->pRoot, pPanel);

	memset(&ldesc, 0, sizeof(ldesc));
	ldesc.iSize = sizeof(ldesc);
	ldesc.sText = "Panel content area";
	ldesc.pFont = ctx->pFont;
	ldesc.iTextColor = XUI_COLOR_RGBA(60, 70, 90, 255);
	ret = xuiLabelCreate(ctx->pContext, &pLabel, &ldesc);
	if (ret != XUI_OK) return ret;
	xuiWidgetSetMargin(pLabel, (xui_thickness_t){12.0f, 12.0f, 0.0f, 0.0f});
	return xuiWidgetAddChild(pPanel, pLabel);
}

int main(int argc, char** argv) { return tut_xui_run(create_ui, "ch143", argc, argv); }
