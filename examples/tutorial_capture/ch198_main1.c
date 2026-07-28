/* ch198 — 命中测试 */
#include "tut_capture_xui.h"

static int create_ui(tut_xui_ctx_t* ctx)
{
	xui_button_desc_t bdesc;
	xui_widget pBtn;
	int ret;

	memset(&bdesc, 0, sizeof(bdesc));
	bdesc.iSize = sizeof(bdesc);
	bdesc.pFont = ctx->pFont;

	/* 多个按钮用于命中测试演示 */
	bdesc.sText = "Target A";
	ret = xuiButtonCreate(ctx->pContext, &pBtn, &bdesc);
	if (ret != XUI_OK) return ret;
	xuiWidgetSetRect(pBtn, (xui_rect_t){50.0f, 50.0f, 150.0f, 50.0f});
	xuiWidgetAddChild(ctx->pRoot, pBtn);

	bdesc.sText = "Target B";
	ret = xuiButtonCreate(ctx->pContext, &pBtn, &bdesc);
	if (ret != XUI_OK) return ret;
	xuiWidgetSetRect(pBtn, (xui_rect_t){250.0f, 50.0f, 150.0f, 50.0f});
	xuiWidgetAddChild(ctx->pRoot, pBtn);

	bdesc.sText = "Target C";
	ret = xuiButtonCreate(ctx->pContext, &pBtn, &bdesc);
	if (ret != XUI_OK) return ret;
	xuiWidgetSetRect(pBtn, (xui_rect_t){150.0f, 150.0f, 150.0f, 50.0f});
	return xuiWidgetAddChild(ctx->pRoot, pBtn);
}

int main(int argc, char** argv) { return tut_xui_run(create_ui, "ch198", argc, argv); }
