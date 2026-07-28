/* ch160 — MsgTip 消息提示 */
#include "tut_capture_xui.h"

static int create_ui(tut_xui_ctx_t* ctx)
{
	xui_msgtip_desc_t desc;
	xui_msgtip pTip;
	int ret;

	memset(&desc, 0, sizeof(desc));
	desc.iSize = sizeof(desc);
	desc.sText = "Operation completed successfully!";
	desc.pFont = ctx->pFont;
	desc.iType = 0; /* success */
	desc.fDuration = 3.0f;
	ret = xuiMsgTipCreate(ctx->pContext, &pTip, &desc);
	if (ret != XUI_OK) return ret;
	xuiMsgTipShow(pTip, 0, "Operation completed successfully!", 3.0f);
	return XUI_OK;
}

int main(int argc, char** argv) { return tut_xui_run(create_ui, "ch160", argc, argv); }
