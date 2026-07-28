/* ch159 — MessageBox 消息框 */
#include "tut_capture_xui.h"

static int create_ui(tut_xui_ctx_t* ctx)
{
	xui_msgbox_desc_t desc;
	xui_msgbox pBox;
	int ret;

	memset(&desc, 0, sizeof(desc));
	desc.iSize = sizeof(desc);
	desc.sTitle = "Confirm";
	desc.sMessage = "Are you sure you want to delete this file?";
	desc.pFont = ctx->pFont;
	desc.iType = 1; /* warning */
	desc.iButtons = 3; /* Yes + No */
	desc.bModal = 1;
	ret = xuiMsgBoxCreate(ctx->pContext, &pBox, &desc);
	if (ret != XUI_OK) return ret;
	xuiMsgBoxSetOpen(pBox, 1);
	return XUI_OK;
}

int main(int argc, char** argv) { return tut_xui_run(create_ui, "ch159", argc, argv); }
