/* ch190 — Canvas 画布控件 */
#include "tut_capture_xui.h"

static int create_ui(tut_xui_ctx_t* ctx)
{
	xui_canvas_desc_t desc;
	xui_widget pCanvas;
	int ret;

	memset(&desc, 0, sizeof(desc));
	desc.iSize = sizeof(desc);
	desc.fCanvasWidth = 800.0f;
	desc.fCanvasHeight = 600.0f;
	desc.bContentDragEnabled = 1;
	ret = xuiCanvasCreate(ctx->pContext, &pCanvas, &desc);
	if (ret != XUI_OK) return ret;
	xuiWidgetSetSizeMode(pCanvas, XUI_SIZE_FILL, XUI_SIZE_FILL);
	return xuiWidgetAddChild(ctx->pRoot, pCanvas);
}

int main(int argc, char** argv) { return tut_xui_run(create_ui, "ch190", argc, argv); }
