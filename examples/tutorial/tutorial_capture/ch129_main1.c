/* ch129 — 样式系统 */
#include "tut_capture_xui.h"

static int create_ui(tut_xui_ctx_t* ctx)
{
	xui_button_desc_t desc;
	xui_widget pBtn1, pBtn2;
	int ret;

	memset(&desc, 0, sizeof(desc));
	desc.iSize = sizeof(desc);
	desc.pFont = ctx->pFont;

	/* 自定义样式按钮 1 */
	desc.sText = "Styled Button";
	desc.iTextColor = XUI_COLOR_RGBA(255, 255, 255, 255);
	desc.iNormalColor = XUI_COLOR_RGBA(50, 120, 200, 255);
	desc.iHoverColor = XUI_COLOR_RGBA(70, 140, 220, 255);
	desc.iActiveColor = XUI_COLOR_RGBA(30, 100, 180, 255);
	desc.fBorderWidth = 2.0f;
	desc.iBorderColor = XUI_COLOR_RGBA(30, 80, 150, 255);
	ret = xuiButtonCreate(ctx->pContext, &pBtn1, &desc);
	if (ret != XUI_OK) return ret;
	xuiWidgetSetMargin(pBtn1, (xui_thickness_t){20.0f, 20.0f, 0.0f, 0.0f});
	xuiWidgetSetPreferredSize(pBtn1, (xui_vec2_t){180.0f, 40.0f});
	xuiWidgetAddChild(ctx->pRoot, pBtn1);

	/* 自定义样式按钮 2 */
	desc.sText = "Green Theme";
	desc.iNormalColor = XUI_COLOR_RGBA(40, 160, 100, 255);
	desc.iHoverColor = XUI_COLOR_RGBA(60, 180, 120, 255);
	desc.iActiveColor = XUI_COLOR_RGBA(20, 140, 80, 255);
	desc.iBorderColor = XUI_COLOR_RGBA(20, 120, 70, 255);
	ret = xuiButtonCreate(ctx->pContext, &pBtn2, &desc);
	if (ret != XUI_OK) return ret;
	xuiWidgetSetMargin(pBtn2, (xui_thickness_t){20.0f, 16.0f, 0.0f, 0.0f});
	xuiWidgetSetPreferredSize(pBtn2, (xui_vec2_t){180.0f, 40.0f});
	return xuiWidgetAddChild(ctx->pRoot, pBtn2);
}

int main(int argc, char** argv) { return tut_xui_run(create_ui, "ch129", argc, argv); }
