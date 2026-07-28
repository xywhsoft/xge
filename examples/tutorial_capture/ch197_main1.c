/* ch197 — 热键系统 */
#include "tut_capture_xui.h"

static int create_ui(tut_xui_ctx_t* ctx)
{
	xui_button_desc_t bdesc;
	xui_widget pBtn;
	int ret;

	memset(&bdesc, 0, sizeof(bdesc));
	bdesc.iSize = sizeof(bdesc);
	bdesc.pFont = ctx->pFont;

	/* Ctrl+S */
	bdesc.sText = "Save (Ctrl+S)";
	ret = xuiButtonCreate(ctx->pContext, &pBtn, &bdesc);
	if (ret != XUI_OK) return ret;
	xuiWidgetSetMargin(pBtn, (xui_thickness_t){30.0f, 30.0f, 0.0f, 0.0f});
	xuiWidgetSetPreferredSize(pBtn, (xui_vec2_t){200.0f, 36.0f});
	xuiWidgetAddChild(ctx->pRoot, pBtn);

	/* Ctrl+Z */
	bdesc.sText = "Undo (Ctrl+Z)";
	ret = xuiButtonCreate(ctx->pContext, &pBtn, &bdesc);
	if (ret != XUI_OK) return ret;
	xuiWidgetSetMargin(pBtn, (xui_thickness_t){30.0f, 12.0f, 0.0f, 0.0f});
	xuiWidgetSetPreferredSize(pBtn, (xui_vec2_t){200.0f, 36.0f});
	xuiWidgetAddChild(ctx->pRoot, pBtn);

	/* F5 */
	bdesc.sText = "Refresh (F5)";
	ret = xuiButtonCreate(ctx->pContext, &pBtn, &bdesc);
	if (ret != XUI_OK) return ret;
	xuiWidgetSetMargin(pBtn, (xui_thickness_t){30.0f, 12.0f, 0.0f, 0.0f});
	xuiWidgetSetPreferredSize(pBtn, (xui_vec2_t){200.0f, 36.0f});
	return xuiWidgetAddChild(ctx->pRoot, pBtn);
}

int main(int argc, char** argv) { return tut_xui_run(create_ui, "ch197", argc, argv); }
