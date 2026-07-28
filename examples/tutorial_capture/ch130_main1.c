/* ch130 — 事件系统 */
#include "tut_capture_xui.h"

static int create_ui(tut_xui_ctx_t* ctx)
{
	xui_button_desc_t desc;
	xui_label_desc_t ldesc;
	xui_widget pBtn, pLabel;
	int ret;

	/* 按钮（可接收点击事件） */
	memset(&desc, 0, sizeof(desc));
	desc.iSize = sizeof(desc);
	desc.sText = "Click Me (Event)";
	desc.pFont = ctx->pFont;
	ret = xuiButtonCreate(ctx->pContext, &pBtn, &desc);
	if (ret != XUI_OK) return ret;
	xuiWidgetSetMargin(pBtn, (xui_thickness_t){20.0f, 20.0f, 0.0f, 0.0f});
	xuiWidgetSetPreferredSize(pBtn, (xui_vec2_t){180.0f, 36.0f});
	xuiWidgetAddChild(ctx->pRoot, pBtn);

	/* 标签显示事件说明 */
	memset(&ldesc, 0, sizeof(ldesc));
	ldesc.iSize = sizeof(ldesc);
	ldesc.sText = "Events: click, hover, focus, key";
	ldesc.pFont = ctx->pFont;
	ldesc.iTextColor = XUI_COLOR_RGBA(80, 100, 140, 255);
	ret = xuiLabelCreate(ctx->pContext, &pLabel, &ldesc);
	if (ret != XUI_OK) return ret;
	xuiWidgetSetMargin(pLabel, (xui_thickness_t){20.0f, 16.0f, 0.0f, 0.0f});
	return xuiWidgetAddChild(ctx->pRoot, pLabel);
}

int main(int argc, char** argv) { return tut_xui_run(create_ui, "ch130", argc, argv); }
