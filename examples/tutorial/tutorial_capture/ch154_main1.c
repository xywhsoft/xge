/* ch154 — Popup 弹出层 */
#include "tut_capture_xui.h"

static int create_ui(tut_xui_ctx_t* ctx)
{
	xui_popup_desc_t desc;
	xui_button_desc_t bdesc;
	xui_label_desc_t ldesc;
	xui_widget pPopup, pBtn, pLabel;
	int ret;

	/* 触发按钮 */
	memset(&bdesc, 0, sizeof(bdesc));
	bdesc.iSize = sizeof(bdesc);
	bdesc.sText = "Show Popup";
	bdesc.pFont = ctx->pFont;
	ret = xuiButtonCreate(ctx->pContext, &pBtn, &bdesc);
	if (ret != XUI_OK) return ret;
	xuiWidgetSetMargin(pBtn, (xui_thickness_t){20.0f, 20.0f, 0.0f, 0.0f});
	xuiWidgetSetPreferredSize(pBtn, (xui_vec2_t){140.0f, 36.0f});
	xuiWidgetAddChild(ctx->pRoot, pBtn);

	/* 弹出层 */
	memset(&desc, 0, sizeof(desc));
	desc.iSize = sizeof(desc);
	ret = xuiPopupCreate(ctx->pContext, &pPopup, &desc);
	if (ret != XUI_OK) return ret;
	xuiWidgetSetPreferredSize(pPopup, (xui_vec2_t){200.0f, 120.0f});

	memset(&ldesc, 0, sizeof(ldesc));
	ldesc.iSize = sizeof(ldesc);
	ldesc.sText = "Popup content";
	ldesc.pFont = ctx->pFont;
	ldesc.iTextColor = XUI_COLOR_RGBA(40, 50, 70, 255);
	ret = xuiLabelCreate(ctx->pContext, &pLabel, &ldesc);
	if (ret != XUI_OK) return ret;
	xuiWidgetSetMargin(pLabel, (xui_thickness_t){12.0f, 12.0f, 0.0f, 0.0f});
	xuiWidgetAddChild(pPopup, pLabel);

	/* 显示弹出层 */
	xuiPopupSetOwner(pPopup, pBtn);
	xuiPopupSetOpen(pPopup, 1);
	return XUI_OK;
}

int main(int argc, char** argv) { return tut_xui_run(create_ui, "ch154", argc, argv); }
