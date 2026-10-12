/* ch124 — 脏标记系统 */
#include "tut_capture_xui.h"

static int create_ui(tut_xui_ctx_t* ctx)
{
	xui_label_desc_t desc;
	xui_widget pLabel;
	int ret;

	memset(&desc, 0, sizeof(desc));
	desc.iSize = sizeof(desc);
	desc.sText = "Dirty Flag: layout invalidated on change";
	desc.pFont = ctx->pFont;
	desc.iTextColor = XUI_COLOR_RGBA(30, 40, 60, 255);
	ret = xuiLabelCreate(ctx->pContext, &pLabel, &desc);
	if (ret != XUI_OK) return ret;
	xuiWidgetSetMargin(pLabel, (xui_thickness_t){20.0f, 20.0f, 0.0f, 0.0f});
	xuiWidgetAddChild(ctx->pRoot, pLabel);

	/* 修改属性触发脏标记 */
	xuiLabelSetText(pLabel, "Dirty Flag: re-layout triggered");
	return XUI_OK;
}

int main(int argc, char** argv) { return tut_xui_run(create_ui, "ch124", argc, argv); }
