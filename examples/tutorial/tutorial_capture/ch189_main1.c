/* ch189 — QRCode 二维码 */
#include "tut_capture_xui.h"

static int create_ui(tut_xui_ctx_t* ctx)
{
	xui_qrcode_desc_t desc;
	xui_widget pQR;
	int ret;

	memset(&desc, 0, sizeof(desc));
	desc.iSize = sizeof(desc);
	desc.sValue = "https://xge.dev/tutorial";
	desc.iForegroundColor = 0xFF000000;
	desc.iBackgroundColor = 0xFFFFFFFF;
	desc.fPadding = 8.0f;
	ret = xuiQrCodeCreate(ctx->pContext, &pQR, &desc);
	if (ret != XUI_OK) return ret;
	xuiWidgetSetMargin(pQR, (xui_thickness_t){100.0f, 60.0f, 0.0f, 0.0f});
	xuiWidgetSetPreferredSize(pQR, (xui_vec2_t){200.0f, 200.0f});
	return xuiWidgetAddChild(ctx->pRoot, pQR);
}

int main(int argc, char** argv) { return tut_xui_run(create_ui, "ch189", argc, argv); }
