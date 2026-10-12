/* ch178 — Terminal 终端基础 */
#include "tut_capture_xui.h"

static int create_ui(tut_xui_ctx_t* ctx)
{
	xui_terminal_desc_t desc;
	xui_widget pTerm;
	int ret;

	memset(&desc, 0, sizeof(desc));
	desc.iSize = sizeof(desc);
	desc.pFont = ctx->pFont;
	desc.iColumns = 80;
	desc.iRows = 24;
	desc.iScrollbackLimit = 1000;
	ret = xuiTerminalCreate(ctx->pContext, &pTerm, &desc);
	if (ret != XUI_OK) return ret;
	xuiWidgetSetMargin(pTerm, (xui_thickness_t){10.0f, 10.0f, 10.0f, 10.0f});
	xuiWidgetSetSizeMode(pTerm, XUI_SIZE_FILL, XUI_SIZE_FILL);
	return xuiWidgetAddChild(ctx->pRoot, pTerm);
}

int main(int argc, char** argv) { return tut_xui_run(create_ui, "ch178", argc, argv); }
