/* ch192 — 缓存策略 */
#include "tut_capture_xui.h"

static int create_ui(tut_xui_ctx_t* ctx)
{
	xui_label_desc_t desc;
	xui_widget pLabel;
	int ret;

	memset(&desc, 0, sizeof(desc));
	desc.iSize = sizeof(desc);
	desc.pFont = ctx->pFont;
	desc.sText = "Cache Strategy: LRU + Dirty Rect\nGlyph cache: 512x512 atlas\nTexture pool: 64 slots\nRender cache: enabled";
	ret = xuiLabelCreate(ctx->pContext, &pLabel, &desc);
	if (ret != XUI_OK) return ret;
	xuiWidgetSetMargin(pLabel, (xui_thickness_t){30.0f, 30.0f, 0.0f, 0.0f});
	xuiWidgetSetPreferredSize(pLabel, (xui_vec2_t){400.0f, 120.0f});
	return xuiWidgetAddChild(ctx->pRoot, pLabel);
}

int main(int argc, char** argv) { return tut_xui_run(create_ui, "ch192", argc, argv); }
