/* ch161 — Toast 轻提示 */
#include "tut_capture_xui.h"

static int create_ui(tut_xui_ctx_t* ctx)
{
	xui_toast_desc_t desc;
	xui_toast pToast;
	int ret;

	memset(&desc, 0, sizeof(desc));
	desc.iSize = sizeof(desc);
	desc.pFont = ctx->pFont;
	desc.iPlacement = 1; /* top-right */
	desc.iDirection = 0; /* vertical */
	ret = xuiToastCreate(ctx->pContext, &pToast, &desc);
	if (ret != XUI_OK) return ret;
	xuiToastShow(pToast, 0, "Success", "File saved.", 3.0f, NULL, NULL);
	xuiToastShow(pToast, 1, "Info", "Syncing data...", 3.0f, NULL, NULL);
	return XUI_OK;
}

int main(int argc, char** argv) { return tut_xui_run(create_ui, "ch161", argc, argv); }
